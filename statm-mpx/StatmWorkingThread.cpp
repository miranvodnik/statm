/*
 * StatmWorkingThread.cpp
 *
 *  Created on: 3. nov. 2015
 *      Author: miran
 */

#include "StatmWorkingThread.h"
#include <iostream>
using namespace std;

namespace statm_daemon
{

StatmCommon*	StatmCommon::g_commonInitializer = new StatmCommon();
StatmWorkingThread*	StatmWorkingThread::g_currentWorkingThread = 0;
bool	StatmWorkingThread::g_started = false;	//!< started indicator (true - threads are started)

/*! @brief start working threads
 *
 *  if working threads are started, this function does nothing
 *
 *  otherwise number of CPU threads and number of working threads
 *  is determined
 *
 *  function then cycles creating new working thread in every
 *  cycle. Working threads are evenly distributed between CPU
 *  threads by using this formula to select CPU thread for newly
 *  created working thread: 'cycle number' modulo 'number of
 *  CPU threads'.
 *
 */
void	StatmWorkingThread::StartWorkingThreads ()
{
	if (g_started)
		return;
	g_started = true;

	StatmWorkingThread*	g_currentWorkingThread = new StatmWorkingThread (0);
	g_currentWorkingThread->Start(0);
}

/*! @brief stop working threads
 *
 *  cycle between working threads and cancel it's execution calling
 *  pthread_cancel(). Every thread is then joined and deleted
 *
 */
void	StatmWorkingThread::StopWorkingThreads ()
{
	delete	g_currentWorkingThread;
	g_currentWorkingThread = 0;
}

/*! @brief replace working threads
 *
 *  threads executing lengthy jobs with too short execution watch-dog timer
 *  should be canceled by watch-dog timer mechanisms. Canceled working
 *  threads are usually replaced by new one. This function does it. It creates
 *  new working thread setting it's CPU thread affinity to the same thread as
 *  it was canceled one.
 *
 *  @param wth reference to canceled working thread used to retrieve it's
 *  CPU thread affinity
 */
bool	StatmWorkingThread::ReplaceWorkingThread (StatmWorkingThread* wth)
{
	g_currentWorkingThread = new StatmWorkingThread (0);
	g_currentWorkingThread->Start(0);
	return	true;
}

/*! @brief create working thread
 *
 *  function initializes:
 *  - thread CPU affinity
 *  - internal watch-dog timer
 *
 *  @param index index of CPU thread which will be used to set working
 *  thread's CPU affinity
 *
 */
StatmWorkingThread::StatmWorkingThread (u_int index)
{
#if defined(DEBUG)
	if (g_debug) statmErrReport (SC_STATM, SC_ERR, "CREATE StatmWorkingThread  %d", this);
#endif
	m_index = index;
	m_tid = 0;
}

/*! @brief delete working thread
 *
 *  function deletes internal watch-dog timer if it has been created
 *
 */
StatmWorkingThread::~StatmWorkingThread ()
{
#if defined(DEBUG)
	if (g_debug) statmErrReport (SC_STATM, SC_ERR, "DELETE StatmWorkingThread  %d", this);
#endif
	if (m_tid)
		timer_delete (m_tid);
	m_tid = 0;
}

/*! @brief main function of working thread
 *
 *  main function does the following:
 *
 *  - installs cleanup handler
 *
 *  - loops forever reading message from working thread message queue
 *  and executing them
 *
 *  - execution time watch-dog timer is started for every job which
 *  provide finite job execution time. This timer is turned off when
 *  job terminates. If it takes too long, watch-dog is fired causing
 *  working thread to be canceled
 *
 *  @return 0 function Run() never terminates normally and it never
 *  returns any value, because it is running in an infinite loop, which
 *  is broken by running environment by canceling it, either because of
 *  watch-dog timer or because of process termination request. But since
 *  function Run() is in fact implementation of superclass's Run() it must
 *  return an integer value.
 *
 */
int StatmWorkingThread::Run (void)
{
	pthread_cleanup_push (ThreadCleanup, this);
	statmErrReport (SC_STATM, SC_ERR, "Working thread started, TID = %d", getTid ());
	while (true)
	{
		StatmBaseJob* job = (StatmBaseJob*) StatmMessageQueue::GetInt ();
		if (job == 0)
			continue;
		u_long	execTimer = job->execTimer();
		if (execTimer == StatmBaseJob::Infinite)
		{
			pthread_cleanup_push (JobCleanup, job);
			job->Execute (this);
			pthread_cleanup_pop(0);
		}
		else
		{
			StartSentinel (execTimer);
			pthread_cleanup_push (JobCleanup, job);
			job->Execute (this);
			pthread_cleanup_pop(0);
			StopSentinel ();
		}
	}
	pthread_cleanup_pop(0);
	return 0;
}

/*! @brief start watch-dog timer
 *
 *  function starts watch-dog timer using number of nanoseconds
 *  to determine it's expiration time. During execution of this
 *  function thread cancellation is disabled
 *
 *  @param tvalue number of nanoseconds
 *
 *  @return 0 successfully started watch-dog timer
 *  @return other failed to start watch-dog timer
 */
int	StatmWorkingThread::StartSentinel (u_long tvalue)
{
	return	0;

	int	cancelstate, dummy;

	pthread_setcancelstate (PTHREAD_CANCEL_DISABLE, &cancelstate);
	struct	itimerspec	its;
	memset (&its, 0, sizeof (its));
	its.it_value.tv_sec  = tvalue / (1000 * 1000 * 1000);
	its.it_value.tv_nsec = tvalue % (1000 * 1000 * 1000);

	int	ret;
	while ((ret = timer_settime (m_tid, 0, &its, NULL)) < 0)
	{
		if (errno == EINTR)
			continue;
		statmErrReport (SC_STATM, SC_ERR, "Working thread %d: cannot start sentinel", getTid());
		break;
	}

	pthread_setcancelstate (cancelstate, &dummy);
	pthread_testcancel ();
	return	ret;
}

/*! @brief stop watch-dog timer
 *
 *  watch-dog timer previously started is turned off
 *
 *  @return 0 watch-dog timer turned off
 *  @return other watch-dog timer cannot be turned off
 *
 */
int	StatmWorkingThread::StopSentinel (void)
{
	return	0;

	int	cancelstate, dummy;

	pthread_setcancelstate (PTHREAD_CANCEL_DISABLE, &cancelstate);
	struct	itimerspec	its;
	memset (&its, 0, sizeof (its));

	int	ret;
	while ((ret = timer_settime (m_tid, 0, &its, NULL)) < 0)
	{
		if (errno == EINTR)
			continue;
		statmErrReport (SC_STATM, SC_ERR, "Working thread %d: cannot stop sentinel", getTid());
		break;
	}

	pthread_setcancelstate (cancelstate, &dummy);
	pthread_testcancel ();
	return	ret;
}

/*! @brief restart watch-dog timer
 *
 *  function stops and then starts watch-dog timer
 *
 *  @param value new value of watch-dog timer
 *
 *  @return 0 watch-dog timer restarted
 *  @return other watch-dog timer cannot be restarted
 *
 */
int	StatmWorkingThread::RestartSentinel (u_long tvalue)
{
	int	ret;
	if ((ret = StopSentinel ()) == 0)
		ret = StartSentinel (tvalue);
	return	ret;
}

/*! @brief initialization of working thread
 *
 *  implementation of superclass's InitInstance(). It installs
 *  signal handler for SIGALARM, signal fired when watch-dog
 *  timer expires and creates timer which should actually
 *  fire it. There is small detail which should be remembered.
 *  sigval structure, used to define SIGALRM signal structure,
 *  hold reference to working thread object in it's sival_ptr.
 *  This reference can be retrieved later on when SIGALRM
 *  actually fires and used to determine working thread instance
 *  which should be canceled.
 *
 *  @return 0 watch-dog timer created successfully
 *  @return other creation of watch-dog timer failed
 *
 */
int StatmWorkingThread::InitInstance (void)
{
	clockid_t	cid = CLOCK_REALTIME;
	sigevent_t	sigev;
	sigval_t	sigval;

	sigval.sival_ptr = this;

	memset (&sigev, 0, sizeof (sigev));
	sigev.sigev_signo = SIGALRM;
	sigev.sigev_notify = SIGEV_THREAD;
	sigev.sigev_notify_function = g_ptf;
	sigev.sigev_value = sigval;

	while (timer_create (cid, &sigev, &m_tid) < 0)
	{
		if (errno == EINTR)
			continue;
		statmErrReport (SC_STATM, SC_ERR, "Working thread %d: cannot create watchdog timer", getTid());
		return	-1;
	}
	return 0;
}

/*! @brief finalization of working thread
 *
 *  implementation of superclass's ExitInstance()
 *
 *  it reports message stating that working thread terminated
 *
 *  @return 0 always
 */
int StatmWorkingThread::ExitInstance (void)
{
	statmErrReport (SC_STATM, SC_ERR, "Working thread terminated, TID = %d", getTid ());
	return 0;
}

/*! @brief watch-dog's SIGALRM signal handler
 *
 *  function is global as required by OS's signal handling environment
 *  and it thus does nothing but dereferences working thread instance
 *  reference saved when creating watch-dog timer and reported by OS
 *  as sival_ptr member of sigval_t structure when SIGALRM fired. This
 *  reference is then used to call the member part function.
 *
 *  @param val sigval_t structure reported by OS. sival_ptr member of
 *  this structure holds working thread reference provided at watch-dog
 *  timer creation.
 *
 */
void	StatmWorkingThread::g_ptf (sigval_t val)
{
	StatmWorkingThread	*_this = (StatmWorkingThread*) val.sival_ptr;
	if (_this == NULL)
		return;

	_this->_g_ptf();
	delete	_this;
}

/*! @brief member part function of g_ptf
 *
 *  Function does the following:
 *
 *  - reports working thread cancellation message
 *  - it replaces working thread with new one
 *  - it cancels original thread and joined it
 *
 */
void	StatmWorkingThread::_g_ptf ()
{
	statmErrReport (SC_STATM, SC_ERR, "Working thread %d: Cancellation timeout fired", getTid());
	if (!ReplaceWorkingThread (this))
	{
		statmErrReport (SC_STATM, SC_ERR, "Cannot replace working thread %d", getTid());
		return;
	}
	pthread_cancel (getHandle());
	WaitForCompletion ();
}

/*! @brief thread cleanup function
 *
 *  this function is called due to process termination request. It cannot
 *  be called due to watch-dog timer expiration
 *
 */
void StatmWorkingThread::ThreadCleanup ()
{
	statmErrReport (SC_STATM, SC_ERR, "Working thread canceled, TID = %d", getTid ());
}

/*! @brief job cleanup function
 *
 *  this function is called due to watch-dog timer expiration and it can also
 *  be called due to process termination request if it comes in the middle of
 *  job execution. It calls job's cleanup function.
 *
 *  @param data generalized reference to an instance of job
 *
 */
void StatmWorkingThread::JobCleanup (void* data)
{
	StatmBaseJob* job = (StatmBaseJob*) data;
	statmErrReport (SC_STATM, SC_ERR, "Working job canceled, JOBID = %d", job->jobCounter ());
	job->Cleanup ();
}

} /* namespace statm_daemon */
