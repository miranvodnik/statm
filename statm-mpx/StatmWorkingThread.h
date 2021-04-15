/*
 * StatmWorkingThread.h
 *
 *  Created on: 3. nov. 2015
 *      Author: miran
 */

#pragma once

#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>
#include "StatmRunnable.h"
#include "StatmCommon.h"
#include "StatmBaseJob.h"
#include "StatmMessageQueue.h"

#include <list>

using namespace std;

namespace statm_daemon
{

/*! @brief working thread class
 *
 *  it is an implementation of StatmRunnable and as such it must
 *  implement Run(), InitInstance() and ExitInstance() functions
 *
 */
class	StatmWorkingThread: public StatmRunnable
{
public:
	StatmWorkingThread (u_int index);
	virtual ~StatmWorkingThread();
	static void	StartWorkingThreads ();
	static void	StopWorkingThreads ();
	static bool	ReplaceWorkingThread (StatmWorkingThread* wth);
	inline u_int	index () { return m_index; }	//!< get index of CPU thread affinity
private:
	virtual int Run (void);
	virtual int InitInstance (void);
	virtual int ExitInstance (void);
	int	StartSentinel (u_long tvalue);
	int	StopSentinel (void);
	int	RestartSentinel (u_long tvalue);
	static	void	g_ptf (sigval_t val);
	void	_g_ptf ();
	inline static void ThreadCleanup (void* data) { ((StatmWorkingThread*)data)->ThreadCleanup(); }
	void ThreadCleanup ();
	static void JobCleanup (void* data);
private:
	static bool	g_started;	//!< indicator: true - working threads started
	static StatmWorkingThread*	g_currentWorkingThread;
	u_int	m_index;	//!< CPU thread affinity for this working thread
	timer_t		m_tid;	//!< watch dog timer for this working thread
};

} /* namespace statm_daemon */
