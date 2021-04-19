/*
 * StatmChkStatDir.h
 *
 *  Created on: Jan 29, 2019
 *      Author: miran
 */

#pragma once

#include <time.h>
#include <sys/inotify.h>
#include <StatmBaseJob.h>
#include <StatmCommon.h>
#include <StatmErrApi.h>
#include <string>
using namespace std;

namespace statm_daemon
{

class StatmChkStatDir: public StatmBaseJob
{
public:
	typedef struct
	{
		string	m_dirname;
	}	StatmChkContext;
public:
	StatmChkStatDir (int fd, void* data, struct inotify_event* nevent);
	virtual ~StatmChkStatDir();
	virtual int Execute (StatmWorkingThread* wt);
	virtual int Cleanup ();
	virtual void Report ();
	virtual int	Invoke (StatmRunningContext* ctx) { return Execute ((StatmWorkingThread*)ctx); }
private:
	int	m_fd;
	void*	m_ctx;
	int	m_wd;
	uint32_t	m_mask;
	uint32_t	m_cookie;
	uint32_t	m_len;
	string	m_fname;
};

class StatmChkConfig : public StatmBaseJob
{
public:
	typedef struct
	{
		string	m_dirname;
	}	StatmChkContext;
public:
	StatmChkConfig(int fd, void* data, struct inotify_event* nevent);
	virtual ~StatmChkConfig();
	virtual int Execute(StatmWorkingThread* wt);
	virtual int Cleanup();
	virtual void Report();
	virtual int	Invoke(StatmRunningContext* ctx) { return Execute((StatmWorkingThread*)ctx); }
private:
	int	m_fd;
	void* m_ctx;
	int	m_wd;
	uint32_t	m_mask;
	uint32_t	m_cookie;
	uint32_t	m_len;
	string	m_fname;
};

class StatmChkInputCatalog : public StatmBaseJob
{
public:
	typedef struct
	{
		string	m_dirname;
	}	StatmChkContext;
public:
	StatmChkInputCatalog(int fd, void* data, struct inotify_event* nevent);
	virtual ~StatmChkInputCatalog();
	virtual int Execute(StatmWorkingThread* wt);
	virtual int Cleanup();
	virtual void Report();
	virtual int	Invoke(StatmRunningContext* ctx) { return Execute((StatmWorkingThread*)ctx); }
private:
	int	m_fd;
	void* m_ctx;
	int	m_wd;
	uint32_t	m_mask;
	uint32_t	m_cookie;
	uint32_t	m_len;
	string	m_fname;
};

}
