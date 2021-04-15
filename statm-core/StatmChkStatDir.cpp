/*
 * StatmChkStatDir.cpp
 *
 *  Created on: Jan 29, 2019
 *      Author: miran
 */

#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <StatmChkStatDir.h>
#include <StatmStatDriver.h>
#include "StatisticalAdapterThread.h"
#include <iostream>
using namespace std;

namespace statm_daemon
{

StatmChkStatDir::StatmChkStatDir (int fd, void* data, struct inotify_event* nevent) : StatmBaseJob (StatmBaseJob::Infinite)
{
#if defined(DEBUG)
	if (g_debug) statmErrReport (SC_STATM, SC_ERR, "CREATE StatmChkStatDir (%s)", nevent->name);
#endif
	m_fd = fd;
	m_ctx = data;
	m_wd = nevent->wd;
	m_mask = nevent->mask;
	m_cookie = nevent->cookie;
	m_len = nevent->len;
	m_fname = nevent->name;
}

StatmChkStatDir::~StatmChkStatDir()
{
#if defined(DEBUG)
	if (g_debug) statmErrReport (SC_STATM, SC_ERR, "DELETE StatmChkStatDir (%s)", m_fname.c_str());
#endif
}

int StatmChkStatDir::Execute (StatmWorkingThread* wt)
{
	if ((m_mask & (IN_CLOSE_WRITE | IN_MOVED_TO)) == 0)
	{
		statmErrReport(SC_STATM, SC_ERR, "Deny Load statistical data, file = '%s'", m_fname.c_str());
		return	0;
	}

#if defined (STATM_PERFTEST)
	struct timeval	tbegin, tend;

	gettimeofday (&tbegin, 0);
#endif	// STATM_PERFTEST

	statmErrReport (SC_STATM, SC_ERR, "Load statistical data, file = '%s'", m_fname.c_str());
	StatmStatDriver::ReloadStatFile (m_ctx, (char*) m_fname.c_str());

#if defined (STATM_PERFTEST)
	gettimeofday (&tend, 0);
	tend.tv_sec -= tbegin.tv_sec;
	if ((tend.tv_usec -= tbegin.tv_usec) < 0)
	{
		tend.tv_usec += 1000 * 1000;
		tend.tv_sec--;
	}
	statmErrReport (SC_STATM, SC_ERR, err_info ("PERFTEST: loading statistical file '%s/%s', total time = %d.%06d"), m_ctx->m_dirname.c_str(), m_fname.c_str(), tend.tv_sec, tend.tv_usec);
#endif	// STATM_PERFTEST
	return	0;
}

int StatmChkStatDir::Cleanup ()
{
	statmErrReport (SC_STATM, SC_ERR, err_info ("Statistical job killed, file = %s"), m_fname.c_str());
	delete	this;
	return	0;
}

void	StatmChkStatDir::Report ()
{
	statmErrReport (SC_STATM, SC_ERR, err_info ("job - checking statistical dir: file = %s"), m_fname.c_str());
}

StatmChkConfig::StatmChkConfig (int fd, void* data, struct inotify_event* nevent) : StatmBaseJob (StatmBaseJob::Infinite)
{
#if defined(DEBUG)
	if (g_debug) statmErrReport (SC_STATM, SC_ERR, "CREATE StatmChkConfig (%s)", nevent->name);
#endif
	m_fd = fd;
	m_ctx = data;
	m_wd = nevent->wd;
	m_mask = nevent->mask;
	m_cookie = nevent->cookie;
	m_len = nevent->len;
	m_fname = nevent->name;
}

StatmChkConfig::~StatmChkConfig()
{
#if defined(DEBUG)
	if (g_debug) statmErrReport (SC_STATM, SC_ERR, "DELETE StatmChkConfig (%s)", m_fname.c_str());
#endif
}

int StatmChkConfig::Execute (StatmWorkingThread* wt)
{
	if ((m_mask & (IN_CLOSE_WRITE | IN_MOVED_TO)) == 0)
		return	0;

	string xmlCfgFileName = StatisticalAdapterThread::get_xmlCfgFileName ();
	string::size_type position = xmlCfgFileName.rfind ('/');
	if (xmlCfgFileName.compare (position + 1, xmlCfgFileName.length() - position, m_fname) != 0)
		return 0;

	statmErrReport (SC_STATM, SC_ERR, "Reload statistical configuration, file = '%s'", m_fname.c_str());
	StatmStatDriver::ReleaseStatAdapter ();
	StatmStatDriver::InitStatAdapter ();
	return	0;
}

int StatmChkConfig::Cleanup ()
{
	statmErrReport (SC_STATM, SC_ERR, err_info ("Reload statistical configuration job killed, file = %s"), m_fname.c_str());
	delete	this;
	return	0;
}

void	StatmChkConfig::Report ()
{
	statmErrReport (SC_STATM, SC_ERR, err_info ("job - reload statistical configuration: file = %s"), m_fname.c_str());
}

}
