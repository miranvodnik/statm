/*
 * StatmStatDriver.cpp
 *
 *  Created on: 14. nov. 2015
 *      Author: miran
 */

#include <stddef.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <dirent.h>
#include <string.h>
#include <stdio.h>
#include <limits.h>
#include "StatmCommon.h"
#include "StatmSemval.h"
#include "StatmStatDriver.h"
#include "StatisticalAdapterThread.h"
#include <iostream>
#include <string>
using namespace std;

namespace statm_daemon
{

StatmStatDriver* StatmStatDriver::g_StatmStatDriver = 0;

StatmStatDriver::StatmStatDriver ()
{
	union semval sv;
	sv.value = 1;
	m_sync = semget (IPC_PRIVATE, 1, IPC_CREAT);
	semctl (m_sync, 0, SETVAL, sv);

	m_ctx = 0;
	m_notify = -1;
	m_notifyHandler = 0;
	m_inputBuffer = 0;
	m_inputPtr = 0;
	m_inputEnd = 0;
	m_cfgNotify = 0;
}

StatmStatDriver::~StatmStatDriver ()
{
	_ReleaseStatDriver ();
}

void StatmStatDriver::lock ()
{
	struct	sembuf	sb;

	sb.sem_num = 0;
	sb.sem_op = -1;
	sb.sem_flg = SEM_UNDO;
	semop (m_sync, &sb, 1);
}

void StatmStatDriver::unlock ()
{
	struct	sembuf	sb;

	sb.sem_num = 0;
	sb.sem_op = 1;
	sb.sem_flg = SEM_UNDO;
	semop (m_sync, &sb, 1);
}

int StatmStatDriver::RegChgTracker (char *dir, chgcb f, void *data)
{
	int fd;

	lock ();
	if ((fd = inotify_add_watch (m_notify, dir, IN_MODIFY | IN_CLOSE_WRITE | IN_MOVED_TO)) > 0)
	{
		statmErrReport (SC_STATM, SC_ERR, err_info ("register changes for: %s"), dir);
		chgstruct* p_chgstruct = new chgstruct;
		p_chgstruct->m_f = f;
		p_chgstruct->m_data = data;
		chgmap::iterator cit = m_chgmap.find(fd);
		if (cit != m_chgmap.end())
		{
			p_chgstruct->m_next = cit->second;
			cit->second = p_chgstruct;
		}
		else
			(m_chgmap [fd] = p_chgstruct)->m_next = 0;
		unlock ();
		return fd;
	}
	else
		statmErrReport (SC_STATM, SC_ERR, err_info ("cannot register changes for: %s, errno = %d"), dir, errno);

	unlock ();
	return -1;
}

int	StatmStatDriver::UnregChgTracker (int wd, void*data)
{
	lock ();
	chgmap::iterator cit = m_chgmap.find(wd);
	if (cit == m_chgmap.end())
		return -1;
	for (chgstruct** p_chgstruct = &(cit->second); *p_chgstruct != 0; )
	{
		if ((*p_chgstruct)->m_data != data)
		{
			p_chgstruct = &((*p_chgstruct)->m_next);
			continue;
		}
		chgstruct* q_chgstruct = *p_chgstruct;
		*p_chgstruct = q_chgstruct->m_next;
		delete q_chgstruct;
		if (cit->second == 0)
		{
			m_chgmap.erase (cit);
			inotify_rm_watch (m_notify, cit->first);
		}
		unlock ();
		return 0;
	}
	unlock ();
	return -1;
}

void StatmStatDriver::HandleSigPipe (StatmRunningContext *ctx, ctx_sig_t handler, siginfo_t* info)
{
}

void StatmStatDriver::ReadNotifyChanges (StatmRunningContext *ctx, uint flags, ctx_fddes_t handler, int fd)
{
	if ((flags & EPOLLIN) == 0)
		return;

	int needSpace = 0;
	int usedSpace = m_inputPtr - m_inputBuffer;
	int freeSpace = m_inputEnd - m_inputPtr;

	if (ioctl (fd, FIONREAD, &needSpace) < 0)
	{
		statmErrReport (SC_STATM, SC_ERR, err_info ("ioctl() failed, errno = %d"), errno);
		return;
	}

	if (needSpace == 0)
	{
		statmErrReport (SC_STATM, SC_ERR, err_info ("ioctl() failed, 0 bytes read"));
		return;
	}

	if (freeSpace < needSpace)
	{
		needSpace += usedSpace;
		needSpace >>= 10;
		needSpace++;
		needSpace <<= 10;
		u_char* buffer = (u_char*) malloc (needSpace);
		if (buffer == 0)
		{
			statmErrReport (SC_STATM, SC_ERR, err_info ("malloc(%d) failed, errno = %d"), needSpace, errno);
			return;
		}
		if (m_inputBuffer != 0)
		{
			memcpy (buffer, m_inputBuffer, usedSpace);
			free (m_inputBuffer);
		}
		m_inputBuffer = buffer;
		m_inputPtr = buffer + usedSpace;
		m_inputEnd = buffer + needSpace;
		freeSpace = needSpace - usedSpace;
	}

	int recvSize;
	if ((recvSize = read (fd, m_inputPtr, freeSpace)) <= 0)
	{
		if (errno == EWOULDBLOCK)
		{
			return;
		}
		statmErrReport (SC_STATM, SC_ERR, err_info ("read() failed, errno = %d"), errno);
		return;
	}

	usedSpace += recvSize;

	u_char* ptr = m_inputBuffer;
	while (true)
	{
		if (usedSpace < (int) sizeof(struct inotify_event))
			break;

		struct inotify_event* nevent = (struct inotify_event*) ptr;
		size_t notifySize = offsetof (struct inotify_event, name) + nevent->len;
		ptr += notifySize;
		usedSpace -= notifySize;

#if _DEBUG
		statmErrReport (SC_STATM, SC_APL, err_info ("wd     = %d"), nevent->wd);
		statmErrReport (SC_STATM, SC_APL, err_info ("mask   = %d"), nevent->mask);
		statmErrReport (SC_STATM, SC_APL, err_info ("cookie = %d"), nevent->cookie);
		statmErrReport (SC_STATM, SC_APL, err_info ("len    = %d"), nevent->len);
		statmErrReport (SC_STATM, SC_APL, err_info ("name   = %d"), nevent->name);
#endif

		if ((nevent->mask & (IN_CLOSE_WRITE | IN_MOVED_TO)) == 0)
			continue;

		for (chgstruct* chg = StatmStatDriver::GetChgInfo (nevent->wd); chg != 0; chg = chg->m_next)
			chg->m_f (nevent->wd, chg->m_data, nevent);
	}
}

int StatmStatDriver::_InitStatDriver (StatmRunningContext* ctx)
{
	m_ctx = ctx;
	if ((m_notify = inotify_init ()) < 0)
	{
		statmErrReport (SC_STATM, SC_ERR, err_info ("inotify_init() failed, errno = %d"), errno);
		return -1;
	}

	int gflags;
	gflags = fcntl (m_notify, F_GETFL, 0);
	fcntl (m_notify, F_SETFL, gflags | O_NONBLOCK);
	statmErrReport (SC_STATM, SC_APL, err_info ("NOTIFY = %d"), m_notify);
	m_ctx->RegisterSignal(SIGPIPE, HandleSigPipe, this);
	m_notifyHandler = m_ctx->RegisterDescriptor (EPOLLIN, m_notify, ReadNotifyChanges, this, ctx_info);
	return 0;
}

int StatmStatDriver::_LoadStatFiles (char* statPath)
{
	StatisticalAdapterThread::Init();

	string xmlCfgFileName = StatisticalAdapterThread::get_xmlCfgFileName ();
	if (!xmlCfgFileName.empty())
	{
		string::size_type position = xmlCfgFileName.rfind ("/");
		string dirName = xmlCfgFileName.substr (0, position);
		m_cfgNotify = RegChgTracker ((char*)dirName.c_str(), HandleConfigChanges, (void*) -1);
	}

	salist p_salist = StatisticalAdapterThread::get_salist();
	for (salist::iterator sit = p_salist.begin(); sit != p_salist.end(); ++sit)
	{
		StatisticalAdapter* p_adapter = sit->second;
		char* statPath = (char*) p_adapter->statDirName();
		p_adapter->notifyFD(RegChgTracker (statPath, HandleStatDirChanges, p_adapter));
	}
	return 0;
}

int StatmStatDriver::_ReloadStatFile (void* p_adapter, char* statFname)
{
	statmErrReport (SC_STATM, SC_ERR, err_info ("STAT FILE: %s/%s"), ((StatisticalAdapter*) p_adapter)->statDirName(), statFname);
	((StatisticalAdapter*) p_adapter)->DoCollect (statFname);
	return 0;
}

int StatmStatDriver::_ReleaseStatDriver ()
{
	union semval sv;
	sv.value = 0;
	if (m_notifyHandler != 0)
		m_ctx->RemoveDescriptor (m_notifyHandler);
	m_notifyHandler = 0;
	semctl (m_sync, 0, IPC_RMID, sv);
	return	0;
}

void StatmStatDriver::_ReleaseStatAdapter ()
{
	salist p_salist = StatisticalAdapterThread::get_salist();
	for (salist::iterator sit = p_salist.begin(); sit != p_salist.end(); ++sit)
	{
		StatisticalAdapter* p_adapter = sit->second;
		UnregChgTracker (p_adapter->notifyFD (), p_adapter);
	}
	if (m_cfgNotify > 0)
		UnregChgTracker (m_cfgNotify, (void*) -1);
	StatisticalAdapterThread::Release();
}

} /* namespace statm_daemon */
