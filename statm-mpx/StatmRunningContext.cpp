/*#######################################################################*/
/*#                                                                     #*/
/*#             Copyright (c) 2009 IskraTEL                             #*/
/*#                                                                     #*/
/*# Name        : StatmRunningContext.cpp                                #*/
/*#                                                                     #*/
/*# Description	: running context implementation                        #*/
/*#                                                                     #*/
/*# Code        : QQFB - XAK9281                                        #*/
/*#                                                                     #*/
/*# Date        : Mar 2009                                              #*/
/*#                                                                     #*/
/*# Author      : Miran Vodnik                                          #*/
/*#                                                                     #*/
/*# Translation : FtpDBDll.mk                                           #*/
/*#                                                                     #*/
/*# Remarks     :                                                       #*/
/*#                                                                     #*/
/*#######################################################################*/

/****************************************************************/
/*          UNIX - SCCS  VERSION DESCRIPTION                    */
/****************************************************************/
/*static char  unixid_QQFB[] = "%W%	%D%	StatmRunningContext.cpp";*/

/****************************************************************/
/*      Header files                                            */
/****************************************************************/

#include "StatmRunningContext.h"	// running context declarations

pthread_mutex_t	StatmRunningContext::g_lock;	//!< mutex used in signal manipulation routines
sigset_t	StatmRunningContext::g_sigset;	//!< signals handled by all instances of StatmRunningContext
StatmRunningContext::CSignalContext*	StatmRunningContext::g_sigActions[_NSIG];	//!< signal actions of all signals
SigInfoMsg*	SigInfoMsg::g_sigMessages = 0;	//!< linked list of signal descriptors
SigInfoMsg*	SigInfoMsg::g_sigsMessages = 0;	//!< temporary linked list of signal descriptors
StatmRunningContext*	StatmRunningContext::g_initializer = new StatmRunningContext (true);	//!< signals stuff initializer

/*! @brief private constructor for signal stuff initializer
 *
 *  this constructor is private and cannot be used to create object instances
 *  Its purpose is to create single signal stuff initializer which initializes
 *  some data sets associated with signal handling functionality prior to
 *  process start. It initializes:
 *  - g_lock mutex which will synchronize assess to signal handling data structures
 *  - g_sigset signal set which will hold identities of all signals handled by any
 *  instance of StatmRunningContext
 *  - g_sigactions which hold signal actions for all possible signal from 1 to _NSIG
 *  - g_sigsMessages temporary linked list of signal descriptors
 *
 */
StatmRunningContext::StatmRunningContext (bool initialize)
{
	if (!initialize)
		return;

	pthread_mutexattr_t attr;

	pthread_mutexattr_init (&attr);
	pthread_mutexattr_settype (&attr, PTHREAD_MUTEX_RECURSIVE);
	pthread_mutex_init (&g_lock, &attr);

	sigemptyset (&g_sigset);
	memset (g_sigActions, 0, sizeof (g_sigActions));

	u_int	sigsSize = SigInfoMsg::g_sigsSize;
	char*	env;
	if ((env = getenv ("STATM_SIGS_SIZE")) != 0)
	{
		if ((sigsSize = (u_int) atoi (env)) < SigInfoMsg::g_sigsSizeMin)
		{
			sigsSize = SigInfoMsg::g_sigsSizeMin;
		}
		else	if (sigsSize > SigInfoMsg::g_sigsSizeMax)
		{
			sigsSize = SigInfoMsg::g_sigsSizeMax;
		}
	}

	for (u_int i = 0; i < sigsSize; ++i)
	{
		SigInfoMsg*	msg = new SigInfoMsg (0, 0);
		SigInfoMsg::slink (msg);
	}
}

/*! brief create an instance of I/O multiplexer
 *
 *  initializes internal structures of I/O multiplexer, especially:
 *
 *  - initfunc f: initialization function which will be called
 *  by calling environment (MainLoop() function). Function is
 *  used as one shot initialization provided by caller. This
 *  function should initialize whatever is supposed to be used
 *  by I/O multiplexer and functionality encapsulated into it by
 *  the caller through different FD, timer and signal handling
 *  call-back functions.
 *
 *  - exitfunc g: function called at the end of I/O multiplexer main
 *  loop. This function should release all resources allocated
 *  by initializer and any functionality encapsulated in I/O multiplexer
 *  by caller
 *
 *  - timehookfunc h: function triggered whenever system time changes
 *
 *  - ctx_appdt_t appdata: application data provided by caller. It
 *  should be anything which seems to be useful for the caller. This
 *  data will be later used as last parameter in calls of above
 *  functions (initfunc, exitfunc, timehookfunc) and is thus accessed
 *  by caller and meaningful only to the caller.
 *
 */
StatmRunningContext::StatmRunningContext (initfunc f, exitfunc g, timehookfunc h, ctx_appdt_t appdata, const char* name)
{
	m_initfunc = f;
	m_exitfunc = g;
	m_timehookfunc = h;
	m_appldata = appdata;
	m_nfds = -1;
	m_round = 0;
	m_quit = false;
	m_mqReadHandler = 0;
	m_mqWriteHandler = 0;
	m_mqBuffer = 0;
	m_mqBufferWritePtr = 0;
	m_mqBufferReadPtr = 0;
	m_mqBufferEndPtr = 0;
	m_mqid[0] = m_mqid[1] = 0;
}

/*! @brief I/O multiplexer destructor
 *
 *  stop internal message queue and release all internal
 *  data structures
 *
 */
StatmRunningContext::~StatmRunningContext ()
{
	StopMQ ();

	{
		cblist::iterator	i;
		for (i = m_cblist.begin(); i != m_cblist.end(); ++i)
		{
			cbset::iterator	ci;
			for (ci = m_cbset.begin(); (ci != m_cbset.end()) && (ci->second != *i); ++ci);
			if (ci != m_cbset.end())
				m_cbset.erase(ci);
			delete	*i;
		}
		m_cblist.clear();
	}

	{
		cbset::iterator	i;
		for (i = m_cbset.begin(); i != m_cbset.end(); ++i)
			delete	i->second;
		m_cbset.clear();
	}

	{
		tmrlist::iterator	i;
		for (i = m_tmrlist.begin(); i != m_tmrlist.end(); ++i)
		{
			tmrset::iterator	ti;
			for (ti = m_tmrset.begin(); (ti != m_tmrset.end()) && (ti->second != *i); ++ti);
			if (ti != m_tmrset.end())
				m_tmrset.erase (ti);
			delete	*i;
		}
		m_tmrlist.clear();
	}

	{
		tmrset::iterator	i;
		for (i = m_tmrset.begin(); i != m_tmrset.end(); ++i)
			delete	i->second;
		m_tmrset.clear();
	}
}

/*! @brief create internal message queue
 *
 *  function creates internal message queue in form of pipe
 *  performing in nonblocking packet mode. Calls to read()
 *  and write() will return immediately no matter if there
 *  are data to be read or write. Data are sent or received in
 *  packet mode: all data or none at all. I/O operations
 *  performed on this message queue are governed by this
 *  I/O multiplexer using call-back function ReadInternalMessageQueue()
 *
 *  @return 0 message queue created
 *  @return -1 message queue has not been created
 *
 */
int	StatmRunningContext::CreateInternalMQ ()
{
	if (pipe2 (m_mqid, O_CLOEXEC | O_DIRECT | O_NONBLOCK) < 0)
		return	-1;

	m_mqReadHandler = RegisterDescriptor (EPOLLIN, m_mqid[0], ReadInternalMessageQueue, this, ctx_info);
	m_mqWriteHandler = RegisterDescriptor (EPOLLOUT, m_mqid[1], WriteInternalMessageQueue, this, ctx_info);
	DisableDescriptor (m_mqWriteHandler, EPOLLOUT);

	return	0;
}

/*! @brief I/O multiplexer main loop
 *
 *  main loop of I/O multiplexer. It handles virtually unlimited number
 *  of file descriptors of any kind, timers and signal handlers
 *  simultaneously. The main logic of this function is very simple:
 *  it iterates indefinitely until something happens on one or more
 *  file descriptors, timers or signals registered within it. It then
 *  invokes appropriate actions associated with these file descriptors,
 *  timers and signals. This actions are call-back functions registered
 *  to handle I/O events on file descriptors, timers and signals. After
 *  that loop is repeated until there are any registered call-back
 *  functions. New one can be registered or unregistered at any
 *  time. Function performs as follows:
 *
 *  - it remembers time stamps for monotonic and real system time. This
 *  time is initial time stamp to compute timers.
 *
 *  - it creates internal message queue
 *
 *  - it calls initialization function provided by user when creating
 *  this object. This function is appropriate space where first I/O
 *  call-back functions, timers and signals should be registered. They
 *  will be executed in the main loop of I/O multiplexer
 *
 *  - it blocks all signals registered by all instances of StatmRunningContext
 *
 *  - it finally creates polling mechanism for I/O multiplexer
 *
 *  These were initialization steps of I/O multiplexer. This phase is
 *  followed by this iteration. Each step of iteration is composed of these
 *  steps:
 *
 *  - call-back functions for all timers expired in this iteration are
 *  invoked. After that time stamp for the next timer (one or more if they
 *  expire at the same time) is calculated.
 *
 *  - call-back functions for I/O handlers changed in previous iteration
 *  are updated. New one are added, unneeded are removed.
 *
 *  - if immediate termination is requested in previous step or if there
 *  are no I/O activity or no timer then loop will be broken
 *
 *  - next it is calculated maximal time interval to wait before next
 *  iteration: if there are no timers, loop will wait indefinitely until
 *  some I/O activity happens, otherwise it will pause execution until
 *  next timer expiration or until next I/O activity whichever happens
 *  first
 *
 *  - before waiting for new I/O activity or timer expiration (system call
 *  epoll_wait()) signals are allowed to trigger. This is the only place
 *  in the whole iteration loop where registered signals should trigger
 *
 *  - if epoll_wait() fails, error code is checked. If this code indicates
 *  that epoll_wait() was interrupted by signal, signal call-backs are
 *  executed and loop begins next iteration
 *
 *  - if epoll_wait() succeeds, I/O call-back functions for file descriptors
 *  which require I/O activity are invoked. This call-back functions should
 *  register new one or deactivate existing one, they should activate new
 *  timers or deactivate existing one, and finally they can activate new
 *  signal handling call-back functions or deactivate existing one. The
 *  same is true also for timer and signal handling call-back functions.
 *  After call-back functions for file descriptors activated in current
 *  iteration have been invoked I/O multiplexer starts execution of next
 *  iteration
 *
 *  - whenever epoll_wait() terminates, system time is checked to see if
 *  it agrees with application time.
 *
 *  When loop terminates exit function is called. This function is provided
 *  by caller and should contain code which will
 *
 */
int StatmRunningContext::MainLoop (void)
{
	int status = 0;

	clock_gettime (CLOCK_MONOTONIC, &m_cpuTime);
	m_cpuTime.tv_nsec /= SEC_TO_NSEC / 1000;
	m_cpuTime.tv_nsec *= SEC_TO_NSEC / 1000;
	clock_gettime (CLOCK_REALTIME, &m_realTime);
	m_realTime.tv_nsec /= SEC_TO_NSEC / 1000;
	m_realTime.tv_nsec *= SEC_TO_NSEC / 1000;

	if (CreateInternalMQ() < 0)
		return	-1;

	if (m_initfunc != NULL)
		m_initfunc (this, m_appldata);

	pthread_sigmask (SIG_BLOCK, &g_sigset, 0);

	if ((m_epollSetId = epoll_create (g_epollSize)) > 0)
		while (true)
		{
			int n, epollTimer;
			struct timespec *tp;

			++m_round;

			tp = HandleTimers ();
			ActivateDescriptors ();

			if (m_quit)
			{
				stat ("QUIT");
				break;
			}

			if ((m_nfds <= 0) && (tp == NULL))
				break;

			if (tp == NULL)
				epollTimer = -1;
			else
				epollTimer = (tp->tv_sec * 1000) + ((tp->tv_nsec / (SEC_TO_NSEC / 1000)));

			pthread_sigmask (SIG_UNBLOCK, &g_sigset, 0);
			if ((n = epoll_wait (m_epollSetId, m_epollSet, g_epollSize, epollTimer)) < 0)
			{
				int	err = errno;
				pthread_sigmask (SIG_BLOCK, &g_sigset, 0);
				if (err == EINTR)
				{
					HandleAllSignals (this);
					CheckTimeValidity ();
					continue;
				}
				char msg [32];
				sprintf (msg, "epoll_wait (%d)", getpid ());
				perror (msg);
				stat (msg);
				status = -1;
				break;
			}

			pthread_sigmask (SIG_BLOCK, &g_sigset, 0);
			CheckTimeValidity ();
			HandleDescriptors (n);
		}

	StopMQ();

	if (m_exitfunc != NULL)
		m_exitfunc (this, m_appldata);

	pthread_sigmask (SIG_UNBLOCK, &g_sigset, 0);

	return status;
}

/*! @brief check validity of time
 *
 *  function checks difference between CPU and real time. If CPU
 *  and real times elapsed from last iteration of I/O multiplexer
 *  loop differ for more than one second all timers are recalculated
 *  to meet the new value of real time. Time hook function provided
 *  by user is called with old and new time stamps.
 *
 */
void StatmRunningContext::CheckTimeValidity ()
{
	struct timespec cpuTimeStop;
	struct timespec realTimeStop;
	long long longCpuTime;
	long long longCpuTimeStop;
	long long longCpuTimeDiff;
	long long longRealTime;
	long long longRealTimeStop;
	long long longRealTimeDiff;
	long long diff;

	clock_gettime (CLOCK_MONOTONIC, &cpuTimeStop);
	cpuTimeStop.tv_nsec /= SEC_TO_NSEC / 1000;
	cpuTimeStop.tv_nsec *= SEC_TO_NSEC / 1000;
	clock_gettime (CLOCK_REALTIME, &realTimeStop);
	realTimeStop.tv_nsec /= SEC_TO_NSEC / 1000;
	realTimeStop.tv_nsec *= SEC_TO_NSEC / 1000;

	longCpuTime = m_cpuTime.tv_sec;
	longCpuTime *= SEC_TO_NSEC;
	longCpuTime += m_cpuTime.tv_nsec;

	longCpuTimeStop = cpuTimeStop.tv_sec;
	longCpuTimeStop *= SEC_TO_NSEC;
	longCpuTimeStop += cpuTimeStop.tv_nsec;

	longCpuTimeDiff = longCpuTimeStop - longCpuTime;

	longRealTime = m_realTime.tv_sec;
	longRealTime *= SEC_TO_NSEC;
	longRealTime += m_realTime.tv_nsec;

	longRealTimeStop = realTimeStop.tv_sec;
	longRealTimeStop *= SEC_TO_NSEC;
	longRealTimeStop += realTimeStop.tv_nsec;

	longRealTimeDiff = longRealTimeStop - longRealTime;

	m_cpuTime = cpuTimeStop;
	m_realTime = realTimeStop;

	diff = longRealTimeDiff - longCpuTimeDiff;
	if ((diff > SEC_TO_NSEC) || (diff < -SEC_TO_NSEC))
	{
		CompensateTimers (diff);
		if (m_timehookfunc != NULL)
		{
			long long longOldTime = longRealTimeStop + diff;
			struct timespec oldTime;

			oldTime.tv_sec = longOldTime / SEC_TO_NSEC;
			oldTime.tv_nsec = longOldTime % SEC_TO_NSEC;
			m_timehookfunc (this, oldTime, realTimeStop, diff, m_appldata);
		}
	}
}

/*! @brief compensate timers
 *
 *  functions adjusts expiration times for all timers registered
 *  by an instance of I/O multiplexer. All timers are restarted:
 *  they are removed from active timer list and put into list of
 *  prepared timers
 *
 *  @param timeDiff real-time time stamp difference between two
 *  consecutive iterations of I/O multiplexer loop
 *
 */
void StatmRunningContext::CompensateTimers (long long timeDiff)
{
	bool advance = (timeDiff >= 0);
	struct timespec td;

	if (!advance)
		timeDiff *= -1;
	td.tv_sec = timeDiff / SEC_TO_NSEC;
	td.tv_nsec = timeDiff % SEC_TO_NSEC;

	tmrlist::iterator tli;
	for (tli = m_tmrlist.begin (); tli != m_tmrlist.end (); ++tli)
	{
		CTimer* tmr = *tli;
		tmr->compensate (advance, td);
	}

	tmrset::iterator tsi;
	for (tsi = m_tmrset.begin (); tsi != m_tmrset.end (); ++tsi)
	{
		CTimer* tmr = tsi->second;
		tmr->compensate (advance, td);
		m_tmrlist.push_back (tmr);
	}
	m_tmrset.clear ();
}

/*! @brief invoke timer call-back functions
 *
 *  this function is called at the beginning of each iteration of
 *  I/O multiplexer main loop. Shortly before this function call
 *  new real-time time-stamp is calculated. This time-stamp is
 *  used to determine all timers which expire in current iteration
 *  of I/O multiplexer main loop. This is done very simply. Since
 *  set of active timers, namely m_tmrset, is arranged by rising
 *  expiration times it is sufficient to compute upper bound of
 *  this set given real-time time-stamp. All call-back functions
 *  for timers from the beginning to the upper bound from this set
 *  will be invoked and corresponding timers are removed from this
 *  set so that they will not be invoked again in the next iteration
 *  of I/O multiplexer main loop.
 *
 *  After that all prepared timers from the list m_tmrlist will be
 *  put into the set of active timers m_tmrset and will be executed
 *  in one of next iterations.
 *
 *  Finally it is computed new time interval which represents maximal
 *  time to wait before execution of next iteration of I/O multiplexer
 *  main loop. It is the difference between time-stamp of the earliest
 *  timer expiration an real-time time-stamp. Time-stamp of earliest
 *  timer is simply the expiration time of first active timer in the
 *  set of active timers m_tmrset. If there is one (set is not empty)
 *  its expiration time is taken and the difference between it and
 *  real-time time-stamp is calculated. If this difference has negative
 *  value it means that loop is so busy that missed expiration of some
 *  timers and so the next iteration must be executed immediately which
 *  can be achieved by setting this difference to 0. This difference
 *  is finally saved into m_timer.
 *
 *  @return null there are no active timers in this instance of I/O
 *  multiplexer. Next iteration will wait forever until some I/O activity
 *  happens or until interrupted by signal
 *  @return other maximal time to wait until next iteration of I/O
 *  multiplexer main loop
 *
 */
struct timespec* StatmRunningContext::HandleTimers (void)
{
	// execute all timers expired before current time stamp
	if (!m_tmrset.empty ())
	{
		tmrset::iterator it, ub;
		// timer expiration time stamp is bounded with upper bound of current time stamp
		for (ub = m_tmrset.upper_bound (m_realTime), it = m_tmrset.begin (); it != ub; ++it)
		{
			CTimer *tmr = it->second;
			if (tmr->active ())
			{
				tmr->active (false);
				tmr->f () (this, tmr, tmr->t (), tmr->data ());
			}
		}
		// expired timers are deleted
		for (it = m_tmrset.begin (); it != ub; ++it)
		{
			CTimer *tmr = it->second;
			delete tmr;
			it->second = NULL;
		}
		m_tmrset.erase (m_tmrset.begin (), ub);
	}

	// register new timers
	if (!m_tmrlist.empty ())
	{
		tmrlist::iterator li;
		// copy registration list in execution set
		for (li = m_tmrlist.begin (); li != m_tmrlist.end (); ++li)
		{
			CTimer *tmr = *li;
			if (tmr->active())
				m_tmrset.insert (tmrset::value_type (tmr->t (), tmr));
		}
		m_tmrlist.clear ();
	}

	// compute next timer expiration delay
	struct timespec *tp = NULL; // suppose we have no registered timers
	tmrset::iterator it; // if there is any timer it is surely at the beginning
	for (it = m_tmrset.begin (); (it != m_tmrset.end ()) && !(it->second->active ()); ++it)
		;
	if (it != m_tmrset.end ())
	// there are registered timers
	{
		timecmp cmp;
		if (cmp (m_realTime, it->first))
		// we have to wait for next time-out
		{
			m_timer.tv_sec = it->first.tv_sec - m_realTime.tv_sec;
			if ((m_timer.tv_nsec = it->first.tv_nsec - m_realTime.tv_nsec) < 0)
			{
				m_timer.tv_nsec += SEC_TO_NSEC;
				m_timer.tv_sec -= 1;
			}
		}
		// we have missed next time-out: execute it immediatelly
		else
			m_timer.tv_sec = m_timer.tv_nsec = 0;
		tp = &m_timer;
	}
	return tp;
}

/*! @brief register timer
 *
 *  create timer object and put it into list of prepared timers.
 *  This timer will be put into the set of active timers at the
 *  beginning of the next iteration of I/O multiplexer and invoked
 *  in one of next iterations
 *
 *  @param timer time-stamp of invocation
 *  @param f timer call-back function
 *  @param appdata general reference to application data. It is remembered
 *  and used as one of parameters in invocation of timer call-back function
 *  @param info additional information which will be displayed by stat()
 *
 */
ctx_timer_t StatmRunningContext::RegisterTimer (struct timespec timer, tmrfunc f, ctx_appdt_t appdata, const char* info)
{
	timer.tv_nsec /= SEC_TO_NSEC / 1000;
	timer.tv_nsec *= SEC_TO_NSEC / 1000;
	CTimer *tmr = new CTimer (timer, f, appdata, info);
	m_tmrlist.push_back (tmr);
	return (ctx_timer_t) tmr;
}

/*! @brief invoke I/O call-back functions
 *
 *  function will invoke call-back functions for all file descriptors
 *  which are ready for requested I/O operation in the current
 *  iteration of I/O multiplexer main loop. It will iterate through
 *  m_epollSet as much times as needed. At every iteration it will
 *  execute all call-back functions provided for file descriptor
 *  determined in given iteration
 *
 *  @param count number of file descriptors ready for I/O in current
 *  invocation of I/O multiplexer main loop
 *
 */
void StatmRunningContext::HandleDescriptors (int count)
{
	for (int i = 0; i < count; ++i)
	{
		epoll_event* event = m_epollSet + i;
		int fd = event->data.fd;

		pair < cbset::iterator, cbset::iterator > er = m_cbset.equal_range (fd);
		cbset::iterator it;
		for (it = er.first; it != er.second; ++it)
		{
			CDescriptor *hdlr = it->second;
			if ((hdlr->active ()) && (!hdlr->deleted ()))
				hdlr->f () (this, event->events, hdlr, fd, hdlr->data ());
		}
	}
}

/*! @brief invoke I/O handler outside of I/O multiplexer environment
 *
 *  file descriptor call-back functions can be invoked also from the
 *  application environment, for example from other call-back functions
 *
 *  @param hdlr I/O handler reference
 *  @param flags EPOLL flags
 *  @param fd file handler
 *
 */
void StatmRunningContext::ReplyDescriptor (ctx_fddes_t hdlr, unsigned int flags, int fd)
{
	if (hdlr == NULL)
		return;
	CDescriptor* d = (CDescriptor*) hdlr;
	d->f () (this, flags, d, fd, d->data ());
}

/*! @brief register call-back function for file descriptor
 *
 *  function creates new instance of CDescriptor object associated
 *  with given file descriptor and puts it into list of prepared
 *  CDescriptor objects. At the beginning of next iteration of I/O
 *  multiplexer main loop these objects will be transferred into
 *  set of active call-back objects
 *
 *  @param flags EPOLL flags, either EPOLLIN (requests input, e.g.
 *  read()), EPOLLOUT (requests output, e.g. write()) or both
 *  @param fd I/O file descriptor
 *  @param f call-back function
 *  @param appdata general reference to some data provided by user
 *  @param info any text which should be displayed by stat()
 *
 *  @return 0 cannot register file descriptor
 *  @return other address of newly created instance of CDescriptor
 *
 */
ctx_fddes_t StatmRunningContext::RegisterDescriptor (uint flags, int fd, cbfunc f, ctx_appdt_t appdata, const char* info)
{
	if (fd < 0)
		return	0;
	CDescriptor *hdlr = new CDescriptor (flags, fd, f, appdata, info);
	m_cblist.push_back (hdlr);
	return hdlr;
}

/*! @brief active file descriptors
 *
 *  at the beginning of every iteration of I/O multiplexer main loop
 *  objects associated with file descriptor call-back functions
 *  from prepared list will be put into set of active objects. This
 *  is done in the following way.
 *
 *  For every object in list of prepared objects the following steps
 *  are made:
 *
 *  - if it is one of active handlers associated with its file descriptor
 *  marked for deletion(!), it is removed from set of active objects
 *  m_cbset. This is a way how to remove active object. If application
 *  wants to remove call-back function for some file descriptor it calls
 *  RemoveDescriptor() for object associated with this call-back function.
 *  This function will mark associated object as deleted(!) and write
 *  its reference into list of prepared objects signaling to
 *  ActivateDescriptors() to actually delete this object. Reason for this
 *  complication is in the fact that generally the set m_cbset cannot be
 *  modified arbitrarily, especially when iterating through it. This set
 *  is iterated when calling call-back functions of file descriptors.
 *  This functions are only reasonable places where deletion of self or
 *  other call-back functions can happen and that is why it must be done
 *  very carefully to avoid disastrous results.
 *
 *  - if handler is not marked for deletion and if it does not exist in
 *  the set of active handlers, it is added to the set
 *
 *  - if handler is not marked for deletion but it does exist in the set
 *  of active handlers, it is modified
 *
 */
void StatmRunningContext::ActivateDescriptors (void)
{
	cblist::iterator it;
	for (it = m_cblist.begin (); it != m_cblist.end (); ++it)
	{
		CDescriptor *hdlr = (CDescriptor*) *it;
		int fd = hdlr->fd ();
		pair < cbset::iterator, cbset::iterator > er = m_cbset.equal_range (fd);
		cbset::iterator sit;
		for (sit = er.first; (sit != er.second) && (hdlr != sit->second); ++sit);
		if (hdlr->deleted ())
		{
			epoll_ctl (m_epollSetId, EPOLL_CTL_DEL, fd, 0);
			if (sit != er.second)
				m_cbset.erase (sit);
			delete	hdlr;
			continue;
		}
		if (sit == er.second)
		{
			epoll_event event;
			event.events = hdlr->flags ();
			event.data.fd = fd;
			epoll_ctl (m_epollSetId, EPOLL_CTL_ADD, fd, &event);
			m_cbset.insert (cbset::value_type (fd, hdlr));
		}
		else
		{
			epoll_event event;
			event.events = hdlr->flags ();
			event.data.fd = fd;
			epoll_ctl (m_epollSetId, EPOLL_CTL_MOD, fd, &event);
		}
	}

	m_cblist.clear ();
	m_nfds = m_cbset.size ();
}

/*! @brief enable I/O handler activity
 *
 *  modifies EPOLL activity of file descriptor associated with I/O
 *  handler
 *
 *  @param h I/O handler reference associated with file descriptor
 *  @param flags EPOLL flags. Reasonable values are EPOLLIN (enable
 *  input, e.g. read()) and EPOLLOUT (enable output, e.g. write())
 *
 */
void StatmRunningContext::EnableDescriptor (ctx_fddes_t h, uint flags)
{
	if (h == NULL)
		return;
	CDescriptor* hdlr = (CDescriptor*) h;
	if (hdlr->deleted ())
		return;

	uint hflags = hdlr->flags ();
	hdlr->flags (hflags |= flags);

	int fd = hdlr->fd ();
	epoll_event event;
	event.events = hflags;
	event.data.fd = fd;
	epoll_ctl (m_epollSetId, EPOLL_CTL_MOD, fd, &event);
}

/*! @brief disable I/O handler activity
 *
 *  modifies EPOLL activity of file descriptor associated with I/O
 *  handler
 *
 *  @param h I/O handler reference associated with file descriptor
 *  @param flags EPOLL flags. Reasonable values are EPOLLIN (disable
 *  input, e.g. read()) and EPOLLOUT (disable output, e.g. write())
 *
 */
void StatmRunningContext::DisableDescriptor (ctx_fddes_t h, uint flags)
{
	if (h == NULL)
		return;
	CDescriptor* hdlr = (CDescriptor*) h;
	if (hdlr->deleted ())
		return;

	uint hflags = hdlr->flags ();
	hdlr->flags (hflags &= ~flags);

	int fd = hdlr->fd ();
	epoll_event event;
	event.events = hflags;
	event.data.fd = fd;
	epoll_ctl (m_epollSetId, EPOLL_CTL_MOD, fd, &event);
}

/*! @brief remove I/O handler
 *
 *  remove I/O handler from set of active handlers. Handler is
 *  marked for deletion and its reference will be put into set
 *  of prepared file descriptors. At the beginning of the next
 *  iteration of I/O multiplexer main loop it will be actually
 *  deleted in function ActivateDescriptors()
 *
 *  @param h I/O handler reference
 *
 */
void StatmRunningContext::RemoveDescriptor (ctx_fddes_t h)
{
	if (h == NULL)
		return;

	CDescriptor* hdlr = (CDescriptor*) h;
	if (hdlr->deleted ())
		return;

	hdlr->deleted (true);

	int fd = hdlr->fd ();
	pair < cbset::iterator, cbset::iterator > er = m_cbset.equal_range (fd);
	cbset::iterator sit;
	for (sit = er.first; (sit != er.second) && (hdlr != sit->second); ++sit);
	if (sit != er.second)
		m_cblist.push_back (hdlr);
}

/*! @brief register signal handler
 *
 *  registers signal handler using sigaction paradigm and creates
 *  an instance of CSignalContext (signal handler in terms of I/O
 *  multiplexer). This instance is then pushed in list of signal
 *  handlers registered by all instances of I/O multiplexer. Access
 *  to this list must be synchronized since it is shared by all
 *  of them
 *
 *  @param sig signal number
 *  @param f call-back function associated with this signal
 *  @param appdata general reference to application data
 *
 *  @return 0 signal handler cannot be created
 *  @return other generalized reference to an instance of
 *  CSignalHandler (signal handler in terms of I/O multiplexer)
 *
 */
ctx_sig_t StatmRunningContext::RegisterSignal (int sig, sigfunc f, ctx_appdt_t appdata)
{
	if ((sig <= 0) || (sig >= _NSIG))
		return	0;

	sigLock();

	CSignalContext*	hdlr = new CSignalContext (this, sig, f, appdata);
	struct	sigaction *sigAction = hdlr->sigAction();
	struct	sigaction *oldAction = hdlr->oldAction();

	sigAction->sa_handler = NULL;
	sigAction->sa_sigaction = HandleSignal;
	sigAction->sa_flags = SA_RESTART | SA_SIGINFO;
	sigemptyset (&sigAction->sa_mask);
	sigaddset (&sigAction->sa_mask, sig);
	if (sigaction (sig, sigAction, oldAction) == -1)
	{
		delete	hdlr;
		sigUnlock();
		return	0;
	}

	g_sigActions[sig] = hdlr->link (g_sigActions[sig]);
	sigaddset (&g_sigset, sig);
	sigUnlock();
	return	hdlr;
}

/*! @brief remove signal handler from shared list
 *
 *  signal handler (reference to CSignalContext) is removed from
 *  shared list of signal handlers. Since it is shared between all
 *  instances of I/O multiplexer access to it must be synchronized
 *
 *  @param hdlr signal handler reference
 *
 */
void StatmRunningContext::RemoveSignal (ctx_sig_t hdlr)
{
	int	signo = ((CSignalContext*)hdlr)->signo();
	if ((signo <= 0) || (signo >= _NSIG))
		return;

	sigLock();
	CSignalContext::removeFromList(&(g_sigActions[signo]), (CSignalContext*) hdlr);
	sigUnlock();
}

/*! @brief OS signal handler for any signal
 *  function is sigaction form of signal handler for specified signal.
 *  Its only purpose is to remember siginfo_t information reported
 *  by OS. This job is performed in signal context and must be as
 *  short as possible. It simply moves information reported by OS
 *  into list of currently pending signal information objects and
 *  terminates. This information will be handled later when
 *  epoll_wait() will be interrupted by function HandleAllSignals()
 *
 *  @param sig signal number
 *  @param info siginfo as reported by OS
 *  @param data additional data as reported by OS
 *
 */
void StatmRunningContext::HandleSignal (int sig, siginfo_t *info, void *data)
{
	sigLock();
	SigInfoMsg*	sigInfo;
	if ((sigInfo = SigInfoMsg::slinkedmsg()) != 0)
	{
		SigInfoMsg::sunlink(sigInfo);
		*sigInfo = SigInfoMsg (sig, info);
		SigInfoMsg::link (sigInfo);
	}
	sigUnlock ();
}

/*! @brief handle all signals in sync with application
 *
 *  function invokes signal handler call-back functions after epoll_wait()
 *  has been interrupted and are thus invoked in sync with application.
 *  Since is synchronized with current I/O multiplexer, there is no need
 *  to use special synchronization mechanisms between call-back function
 *  used by this I/O multiplexer. Signals with lower numbers are delivered
 *  first. Order of delivery for signal handlers with same signal number
 *  is not specified. Signals which are registered by other instances of
 *  I/O multiplexer (one or more) are delivered to them using MQSend() which
 *  will sync them with these instances.
 *
 *  @param currentContext signal handler context. Actually it is a reference
 *  to this instance of I/O multiplexer and is thus used to distinguish
 *  between signal invocation and signal redirection
 *
 */
void StatmRunningContext::HandleAllSignals (StatmRunningContext* currentCtx)
{
	sigLock();

	SigInfoMsg*	msg;
	while ((msg = SigInfoMsg::linkedmsg()) != 0)
	{
		SigInfoMsg::unlink(msg);
#if	_DEBUG
		siginfo_t*	info = &msg->m_siginfo;
		cout << "SIGINFO (signo = " << info->si_signo << ", errno = " << info->si_errno << ", code = " << info->si_code;
		if (info->si_code <= 0)
		{
			cout << ", pid = " << info->si_pid << ", uid = " << info->si_uid;
		}
		cout << ")" << endl;
#endif

		for (CSignalContext* hdlr = g_sigActions[msg->signo()]; hdlr != 0; hdlr = hdlr->next())
		{
			StatmRunningContext*	ctx = hdlr->ctx();
			msg->handler (hdlr);
			if (ctx == currentCtx)
				msg->Invoke(ctx);
			else
			{
				SigInfoMsg*	sigInfo = new SigInfoMsg (*msg);
				if (sigInfo != 0)
				{
					struct { void* msg; }	m = { (void*) sigInfo };
					ctx->MQSend ((char*) &m, sizeof m);
				}
			}
		}
		delete	msg;
		msg = new SigInfoMsg (0, 0);
		SigInfoMsg::slink (msg);
	}

	sigUnlock ();
}

/*! @brief handle redirected signal
 *
 *  invoke signal handler which was triggered in another instance
 *  of I/O multiplexer
 *
 *  @param ctx referenve to this instance of I/O multiplexer
 *
 *  @return status code returned from signal handler
 *
 */
int	SigInfoMsg::Invoke (StatmRunningContext* ctx)
{
	ctx->sigf (m_handler) (ctx->sigctx(m_handler), m_handler, &m_siginfo, ctx->sigdata(m_handler));
	return	0;
}

/*! @brief stop message queue processing
 *
 *  stop processing messages received by this message queue which
 *  is achieved by calling function StopMQ()
 *
 *  @param ctx referenve to this instance of I/O multiplexer
 *
 *  @return 0 always
 *
 */
int	StatmStopMsg::Invoke (StatmRunningContext* ctx)
{
	ctx->StopMQ();
	return	0;
}

/*! @brief message queue I/O handler
 *
 *  I/O handler associated with internal message queue (implemented
 *  as bidirectional pipe) and registered as call-back function
 *  in this instance of I/O multiplexer. It reads all message packets
 *  currently queued and executes call-back functions associated
 *  with these message packets. Since pipe performs in packet mode,
 *  each read() returns one packet and we need not to be concerned
 *  with rearrangement of messages coming into input buffer
 *
 *  @param ctx reference to I/O multiplexer (this instance)
 *  @param flags EPOLL flags: EPOLLIN (input processing, e.g. read()),
 *  EPOLLOUT (output processing, e.g. write()) or both
 *  @param handler reference to I/O handler associated with this
 *  call-back function
 *  @param fd input file descriptor of bidirectional pipe associated
 *  with this message queue
 *
 */
void	StatmRunningContext::ReadInternalMessageQueue (StatmRunningContext *ctx, uint flags, ctx_fddes_t handler, int fd)
{
	ssize_t	size;
	struct { void* msg; }	m;

	while (true)
	{
		if ((size = read (fd, (char*) &m, sizeof m)) <= 0)
			break;
#if defined(_DEBUG)
		pthread_mutex_lock(&g_lock);
		cout << "read msg " << ((StatmRunningContextMsg*)m.msg)->Name() << endl;
		pthread_mutex_unlock(&g_lock);
#endif
		try
		{
			((StatmRunningContextMsg*)m.msg)->Invoke(ctx);
		}
		catch (...)
		{

		}
		delete	(StatmRunningContextMsg*)m.msg;
	}
}

/*! @brief message queue I/O handler
 *
 *  I/O handler associated with internal message queue (implemented
 *  as bidirectional pipe) and registered as call-back function
 *  in this instance of I/O multiplexer. It writes all message packets
 *  currently queued.
 *
 *  @param ctx reference to I/O multiplexer (this instance)
 *  @param flags EPOLL flags: EPOLLIN (input processing, e.g. read()),
 *  EPOLLOUT (output processing, e.g. write()) or both
 *  @param handler reference to I/O handler associated with this
 *  call-back function
 *  @param fd input file descriptor of bidirectional pipe associated
 *  with this message queue
 *
 */
void	StatmRunningContext::WriteInternalMessageQueue (StatmRunningContext *ctx, uint flags, ctx_fddes_t handler, int fd)
{
	size_t	usedSpace;
	while ((usedSpace = m_mqBufferWritePtr - m_mqBufferReadPtr) > sizeof (size_t))
	{
		size_t	msg_len;
		memcpy (&msg_len, m_mqBufferReadPtr, sizeof (size_t));
		size_t	needSpace = msg_len + sizeof (size_t);
		if (needSpace > usedSpace)
		{
			DisableDescriptor(handler, EPOLLOUT);
			break;
		}
		if (write (fd, m_mqBufferReadPtr + sizeof (size_t), msg_len) <= 0)
			break;
		m_mqBufferReadPtr += needSpace;
	}
	if (m_mqBufferWritePtr == m_mqBufferReadPtr)
		DisableDescriptor(handler, EPOLLOUT);
}

/*! @brief send message into internal message queue
 *
 *  write message reference into output end of bidirectional pipe
 *  representing internal message queue of this instance of I/O
 *  multiplexer. Since pipe performs in packet mode, each write()
 *  send one message reference or nothing at all and so we need not
 *  to be concerned with rearrangement of messages written into
 *  output buffer
 *
 *  @param msg_ptr reference to message to be sent
 *  @param msg_len length of message to be sent
 *
 *  @return status return status of write()
 *
 */
int StatmRunningContext::MQSend (const char *msg_ptr, size_t msg_len)
{
	size_t	freeSpace = m_mqBufferEndPtr - m_mqBufferWritePtr;
	size_t	needSpace = msg_len + sizeof (size_t);

	if (freeSpace < needSpace)
	{
		size_t	readSpace = m_mqBufferReadPtr - m_mqBuffer;
		if (readSpace > 0)
		{
			size_t	copySpace = m_mqBufferWritePtr - m_mqBufferReadPtr;
			if (false)	// memcpy () does not work on overlapping memory segments
				memcpy (m_mqBuffer, m_mqBuffer + readSpace, copySpace);
			else
			{
				// custom implementation of memcpy () for special case of overlapping memory segments:
				// destination segment precedes source segment and both of them are accessed with
				// post increment pointer operations
				char*	dstPtr;
				char*	srcPtr;
				for (dstPtr = m_mqBuffer, srcPtr = m_mqBuffer + readSpace; copySpace > 0; --copySpace, *dstPtr++ = *srcPtr++);
			}
			m_mqBufferReadPtr -= readSpace;
			m_mqBufferWritePtr -= readSpace;
			freeSpace += readSpace;
		}
	}

	if (freeSpace < needSpace)
	{
		size_t	usedSpace = m_mqBufferWritePtr - m_mqBuffer;
		size_t	alocSpace = ((usedSpace + needSpace) / 1024 + 1) * 1024;
		char*	buffer = (char*) malloc (alocSpace);
		if (buffer == 0)
			return	-1;
		if (m_mqBuffer != 0)
		{
			memcpy (buffer, m_mqBuffer, usedSpace);
			free (m_mqBuffer);
		}
		m_mqBuffer = buffer;
		m_mqBufferReadPtr = m_mqBuffer;
		m_mqBufferWritePtr = m_mqBuffer + usedSpace;
		m_mqBufferEndPtr = m_mqBuffer + alocSpace;
	}

	memcpy (m_mqBufferWritePtr, (char*) &msg_len, sizeof (size_t));
	memcpy (m_mqBufferWritePtr + sizeof (size_t), msg_ptr, msg_len);
	m_mqBufferWritePtr += needSpace;

	EnableDescriptor(m_mqWriteHandler, EPOLLOUT);
	return	0;
}

/*! @brief stop message queue processing
 *
 *  remove call-back function handler associated with message queue
 *  associated with this instance of I/O multiplexer and close both
 *  ends of pipe implementing this message queue
 *
 */
void StatmRunningContext::StopMQ ()
{
	if (m_mqReadHandler != 0)
		RemoveDescriptor(m_mqReadHandler);
	m_mqReadHandler = 0;

	if (m_mqWriteHandler != 0)
		RemoveDescriptor(m_mqWriteHandler);
	m_mqWriteHandler = 0;

	if (m_mqid[0] > 0)
		close (m_mqid[0]);
	m_mqid[0] = -1;

	if (m_mqid[1] > 0)
		close (m_mqid[1]);
	m_mqid[1] = -1;
}

/*! @brief stop message queue processing
 *
 *  stop reading message queue - function should be invoked
 *  from another thread of execution
 *
 */
void StatmRunningContext::Stop ()
{
	StatmStopMsg*	msg = new StatmStopMsg ();
	struct { void* msg; }	m = { (void*) msg };
	MQSend ((char*) &m, sizeof m);
}

/* @brief report status for all registered I/O handlers
 *
 *  write status of all I/O handlers and timers
 *
 *  @param msg additional text to be displayed
 *
 */
void StatmRunningContext::stat (const char* msg)
{
	cbset::iterator it;
	int cnt;

	for (cnt = 0, it = m_cbset.begin (); it != m_cbset.end (); ++it)
	{
		CDescriptor* des = it->second;
		const char*info = des->info();
		if (des->active () && !des->deleted ())
			++cnt;
		cout << "FDES " << des->fd ()
				<< ", FLAGS: " << des->flags ()
				<< ", ACTIVE: " << des->active()
				<< ", DELETED: " << des->deleted()
				<< ", INFO: " << ((info != 0) ? info : "")
				<< endl;
	}
	cout << msg << ": ICNT = " << cnt << endl;

	tmrset::iterator ti;

	for (cnt = 0, ti = m_tmrset.begin (); ti != m_tmrset.end (); ++ti)
	{
		CTimer *tmr = ti->second;
		if (tmr->active ())
		{
			const char*	info = tmr->info();
			if (info != 0)
				cout << "TMR " << info << endl;
			++cnt;
		}
	}
	cout << msg << ": TCNT = " << cnt << endl;
	cout << msg << ": NFDS = " << m_nfds << endl;
}
