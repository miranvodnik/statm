/*
 * StatmStatDriver.h
 *
 *  Created on: 14. nov. 2015
 *      Author: miran
 */

#pragma once

#include <signal.h>
#include <fcntl.h>
#include <sys/inotify.h>
#include <sys/stat.h>
#include <semaphore.h>
#include "StatmChkStatDir.h"
#include "StatmInitStatAdapter.h"
#include "StatmMessageQueue.h"
#include "StatmRunningContext.h"

#include <map>
#include <vector>
#include <deque>
#include <string>
using namespace std;

namespace statm_daemon
{

class StatmStatDriver
{
	typedef	void (*chgcb) (int, void*, struct inotify_event*);
	typedef struct _chgstruct
	{
		chgcb m_f;
		void* m_data;
		struct _chgstruct* m_next;
	}	chgstruct;
	typedef pair < chgcb, void* >	chgtpr;
	typedef	map < int, chgstruct* >	chgmap;
private:
	StatmStatDriver();
	virtual ~StatmStatDriver();
public:
	inline static int	InitStatDriver (StatmRunningContext* ctx) { return (g_StatmStatDriver != 0) ? -1 : (g_StatmStatDriver = new StatmStatDriver())->_InitStatDriver(ctx); }
	inline static int	ReleaseStatDriver () { return g_StatmStatDriver->_ReleaseStatDriver(); }
	inline static int	LoadStatFiles (char* statPath = 0) { return g_StatmStatDriver->_LoadStatFiles (statPath); }
	inline static int	ReloadStatFile (void* p_adapter, char* statFname) { return g_StatmStatDriver->_ReloadStatFile (p_adapter, statFname); }
	inline static void InitStatAdapter () { StatmMessageQueue::Put (new StatmInitStatAdapter()); }
	inline static void ReleaseStatAdapter () { g_StatmStatDriver->_ReleaseStatAdapter(); }
private:
	void lock ();
	void unlock ();
	int RegChgTracker (char *dir, chgcb f, void *data);
	int	UnregChgTracker (int wd, void* data);
	inline static void HandleStatDirChanges(int fd, void* data, struct inotify_event* nevent) { cout << "new STAT request" << endl;  StatmMessageQueue::Put(new StatmChkStatDir(fd, data, nevent)); }
	inline static void HandleConfigChanges (int fd, void* data, struct inotify_event* nevent) { StatmMessageQueue::Put (new StatmChkConfig (fd, data, nevent));}
	inline static void HandleInpuCatalogChanges (int fd, void* data, struct inotify_event* nevent) { StatmMessageQueue::Put(new StatmChkConfig(fd, data, nevent)); }
	inline static chgstruct* GetChgInfo (int fd) { return g_StatmStatDriver->_GetChgInfo(fd); }
	inline chgstruct* _GetChgInfo (int fd) { return m_chgmap[fd]; }

	int	_InitStatDriver (StatmRunningContext* ctx);
	int _LoadStatFiles (char* statPath = 0);
	int	_ReloadStatFile (void* p_adapter, char* statFname);
	void _ReleaseStatAdapter ();
	int	_ReleaseStatDriver ();

	fd_handler (ReadNotifyChanges, StatmStatDriver)
	sig_handler (HandleSigPipe, StatmStatDriver)

private:

	static StatmStatDriver*	g_StatmStatDriver;

	chgmap	m_chgmap;

	StatmRunningContext*	m_ctx;
	int m_sync;
	int	m_notify;
	ctx_fddes_t	m_notifyHandler;
	int m_cfgNotify;
	int m_ctgNotify;

	u_char*	m_inputBuffer;
	u_char*	m_inputPtr;
	u_char*	m_inputEnd;
};

} /* namespace statm_daemon */
