/*#######################################################################*/
/*#                                                                     #*/
/*#            Copyright (c) 2003 IskraTEL                              #*/
/*#                                                                     #*/
/*# Name        : Runnable.h                                            #*/
/*#                                                                     #*/
/*# Description : Object running in its own thread                      #*/
/*#                                                                     #*/
/*# Code        : BJFW - XAB0898                                        #*/
/*#                                                                     #*/
/*# Date        : Dec 2005                                              #*/
/*#                                                                     #*/
/*# Author      : Miran Vodnik                                          #*/
/*#                                                                     #*/
/*# Translation : FtpDBDll.mk                                           #*/
/*#                                                                     #*/
/*# Remarks     :                                                       #*/
/*#                                                                     #*/
/*#######################################################################*/

#pragma once

#include <unistd.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <errno.h>

/****************************************************************/
/*          UNIX - SCCS  VERSION DESCRIPTION                   	*/
/****************************************************************/
/*static char  unixid_BJFW[] = "%W%	%D%	Runnable.h";*/

/*! @brief class representing single thread of execution
 *
 *  Interface StatmRunnable defines and partially implements class,
 *  which is capable to run and control its own program thread.
 *  Every derived class must implement at least function Run() of
 *  this interface. This function must implement the running logic
 *  of program thread. Other virtual functions should not be
 *  implemented by derived class, thus doing nothing
 */

class StatmRunnable
{
public:
	/*! @brief instance initialization
	 *
	 *  initialize (clear) members. mutex p_mutex is set to be recursive,
	 *  recursive locking from same thread will not block
	 *
	 */
	StatmRunnable (void)
	{
		pthread_mutexattr_t attr;

		pthread_mutexattr_init (&attr);
		pthread_mutexattr_settype (&attr, PTHREAD_MUTEX_RECURSIVE);
		pthread_mutex_init (&m_mutex, &attr);
		m_threadId = 0;
		m_handle = (pthread_t) 0;
		m_exitCode = -1;
	}

	/*! @brief delete an instance of this object
	 *
	 *  deleting this object when associated thread is still running
	 *  is not recommended. Thread should be stopped, awaited and only
	 *  after then deleted
	 *
	 */
	virtual ~StatmRunnable (void)
	{
		pthread_mutex_destroy (&m_mutex);
	}

	/*! @brief start associated thread
	 *
	 * function tries to create and start program thread associated
	 * with this instance. Before creating new thread it calls
	 * InitInstance() giving to this object instance a chance to do
	 * an initialization step just prior to thread start. If
	 * InitInstance() returns error code (everything but S_OK) program
	 * thread is not created. Object instance should be deleted
	 * immediately in this case. It should be noted that InitInstance
	 * is executed in the context of calling thread and not in the
	 * context of the newly created thread. If there are any entities
	 * concerning new thread which are 'context sensitive' it may be
	 * better to perform initialization step in the context of newly
	 * created thread from the function Run()
	 *
	 * @param flags flags which are used (as is) in pthread_create().
	 * Default value of this parameter is 0
	 *
	 * @return 0 program thread has been created
	 * @return other thread creation error, or error returned by InitInstance ()
	 *
	 */
	int Start (int flags = 0)
	{
		int hRes;

		switch (hRes = Lock ())
		{
		case 0:
			if (m_handle != (pthread_t) 0)
			{
				Unlock ();
				return 0;
			}
			break;
		default:
			perror ("");
			return hRes;
		}
		if ((hRes = InitInstance ()) != 0)
		{
			Unlock ();
			return hRes;
		}
		pthread_attr_t attr;
		pthread_attr_init (&attr);
		pthread_attr_setdetachstate (&attr, PTHREAD_CREATE_JOINABLE);
		hRes = pthread_create (&m_handle, &attr, _Run, (void*) this);
		Unlock ();
		return hRes;
	}

	/*! @brief wait associated thread to terminate
	 *
	 *  function waits until associated program thread terminates or
	 *  until timeout expires, whichever comes first
	 *
	 *  @param timeOut time to wait the associated thread to terminate.
	 *  Default value is infinite, thus until program thread terminates
	 *
	 *  @return 0 program thread has not terminated
	 *  @return other exit code of associated thread
	 *
	 */
	int WaitForCompletion (int timeOut = -1)
	{
		int hRes;

		switch (hRes = Lock ())
		{
		case 0:
			if (m_handle == (pthread_t) 0)
			{
				Unlock ();
				return 0;
			}
			break;
		default:
			return hRes;
		}
		Unlock ();
		if ((hRes = pthread_join (m_handle, NULL)) != 0)
			return hRes;
		return m_exitCode;
	}

	/*! @brief suspend associated thread
	 *
	 *  function tries to suspend program thread associated with this object
	 *
	 *  @return 0 thread is suspended
	 *  @return other thread cannot be suspended
	 *
	 */
	int Suspend (void)
	{
		int hRes;

		switch (hRes = Lock ())
		{
		case 0:
			if (m_handle == (pthread_t) 0)
			{
				Unlock ();
				return -1;
			}
			break;
		default:
			return hRes;
		}
		hRes = pthread_kill (m_handle, SIGSTOP);
		Unlock ();
		return hRes;
	}

	/*! @brief resume suspended thread
	 *
	 *  function tries to resume execution of program thread associated
	 *  with this object
	 *
	 *  @return 0 thread execution is resumed
	 *  @return other thread cannot be resumed
	 *
	 */
	int Resume (void)
	{
		int hRes;

		switch (hRes = Lock ())
		{
		case 0:
			if (m_handle == (pthread_t) 0)
			{
				Unlock ();
				return -1;
			}
			break;
		default:
			return hRes;
		}
		hRes = pthread_kill (m_handle, SIGCONT);
		Unlock ();
		return hRes;
	}

	/*! @brief cancel execution of thread
	 *
	 *  function cancels thread execution and wait it to terminate
	 *
	 *  @return code exit code of terminated thread
	 *
	 */
	int Terminate (int timeOut = -1)
	{
		int hRes;

		switch (hRes = Lock ())
		{
		case 0:
			if (m_handle == (pthread_t) 0)
			{
				Unlock ();
				return 0;
			}
			break;
		default:
			return hRes;
		}
		Unlock ();
		if (((hRes = pthread_cancel (m_handle)) != 0) && ((hRes = pthread_join (m_handle, NULL)) != 0))
			return m_exitCode;
		return hRes;
	}

	/*! @brief abort thread execution
	 *
	 *  function terminates execution of program thread associated with
	 *  this object instance immediately. If it is terminated normally
	 *  it returns forced code, otherwise it returns error code returned
	 *  by thread
	 *
	 *  @param code thread exit code forced by the calling thread
	 *
	 *  @return code forced exit code
	 *  @return other unsuccessful termination of associated thread
	 *
	 */
	int Abort (int code)
	{
		int hRes;

		return ((hRes = Terminate ()) != 0) ? hRes : (m_exitCode = code);
	}

	/*! @brief thread running loop
	 *  this function is the only function of this interface which must
	 *  be implemented by derived class. This function implements the
	 *  running logic of associated program thread although this is not
	 *  the base function of associated thread. The actual base function
	 *  of associated thread is _Run(). But nevertheless, function Run()
	 *  must be implemented as if it is the base function of associated
	 *  thread. Normally it executes an infinite loop, executing some job,
	 *  but always prepared to terminate immediately after any iteration
	 *  or even in the middle of the job, especially when executing lengthy
	 *  jobs
	 *
	 *  @return code return code of function Run() implemented by derived class
	 *
	 */
	virtual int Run (void) = 0;

	/*! @brief initialize subclass prior to running thread
	 *
	 *  Default implementation of this function does nothing and returns
	 *  0, thus telling to the calling environment that associated thread
	 *  can be created and executed. If it is implemented by derived
	 *  class we must obey these facts:
	 *  - this function is supposed to initialize some resources used
	 *  by associated thread
	 *  - return code of this function is used to determine if associated
	 *  thread will be created at all. Returning 0 signals the calling
	 *  environment to proceed with thread creation and execution. Other
	 *  return values signal to the calling environment that the normal
	 *  thread creation sequence must be terminated with error code
	 *  returned by this function
	 *  - function is not running in the context of associated thread
	 *  since it has not been created yet. Initialization which depends
	 *  on the context of the associated thread should be performed
	 *  from Run() or some function called within it.
	 *
	 *  @return 0 successful initialization. Associated program thread
	 *  will be created and executed
	 *  @return other initialization fault. Associated thread will not
	 *  be created and thus not executed
	 *
	 */
	virtual int InitInstance (void)
	{
		return 0;
	}

	/*! @brief function called when associated thread terminates
	 *
	 *  This function is provided to finish the execution of associated
	 *  thread. It is called immediately after Run() terminates in the
	 *  context of associated thread. Default implementation does nothing.
	 *  This function gives to program the last chance to perform some
	 *  actions which should be done when associated thread terminates
	 *
	 *  @return code Default implementation of this function returns
	 *  return code of Run(). If it is implemented by derived class it
	 *  can return any integer value
	 *
	 */
	virtual int ExitInstance (void)
	{
		return m_exitCode;
	}

private:

	/*! @brief running function of associated thread
	 *
	 *  Function is global because it is 3rd parameter of pthread_create()
	 *  and private so that it can be executed only by member functions
	 *  of this class (Start() function). This function is actual main
	 *  function of associated thread. Its only purpose is to start Run()
	 *  and ExitInstance() and to finish associated thread with
	 *  appropriate exit code; the code returned by function Run()
	 *
	 *  @param data void pointer to self; 4th parameter to pthread_create()
	 *  is void*
	 *
	 *  @return code return code of Run()
	 *
	 */
	static void* _Run (void* data)
	{
		return	(((StatmRunnable *) data))->_Run();
	}

	/*! @brief non-static version of _Run()
	 *
	 *  function remembers task ID of associated thread and runs it
	 *  by calling function Run(). Thread execution can be canceled
	 *  in which case cancellation function _Cleanup() will be triggered
	 *  enabling ExitInstance() function to be called in any case,
	 *  no matter how thread terminates, normally or by cancellation
	 *
	 *  @return 0 normal termination
	 *  @return 1 abnormal termination
	 *
	 */
	void* _Run ()
	{
		int eCode;
		StatmRunnable *base = (StatmRunnable *) this;
		pthread_cleanup_push (base->_Cleanup, base);
#if	_DEBUG
			cout << "thread " << (unsigned long) base->getTid () << " started" << endl;
#endif
			m_threadId = syscall (SYS_gettid);
			eCode = base->m_exitCode = base->Run ();
#if	_DEBUG
			cout << "thread " << (unsigned long) base->getTid () << " stopped" << endl;
#endif
			pthread_cleanup_pop(0);
		base->ExitInstance ();
		pthread_exit ((eCode == 0) ? ((void*)0) : ((void*)1));
		return (eCode == 0) ? ((void*)0) : ((void*)1);
	}

	/****************************************************************/
	/* Function:    _Cleenup ()                                     */
	/* Out:         /                                               */
	/* In/Out:      /                                               */
	/* Return:      /                                               */
	/****************************************************************/

	/*! @brief cancellation trigger
	 *
	 *  Function is static as requested by pthread_cleanup() and private
	 *  to disable invocation of this function by external entities.
	 *  It is executed automatically when associated thread terminates
	 *  normally or unexpectedly due to cancellation action. Function
	 *  (in fact it's non-static version) does nothing. Its only purpose
	 *  is to enable calling ExitInstance() function no matter how
	 *  associated thread is terminated: normally or canceled
	 *
	 *  @param data void pointer to self
	 *
	 */
	static void _Cleanup (void* data)
	{
		StatmRunnable *base = (StatmRunnable *) data;
		base->Cleanup ();
	}

	/*! @brief non-static version of cancellation trigger
	 *
	 */
	void Cleanup (void)
	{
	}
public:

	/*! @brief get task ID
	 *
	 *  @return id task ID of associated thread
	 *
	 */
	inline	pid_t	getTid (void) { return m_threadId; }

	/*! @brief get thread handle
	 *
	 *  @return NULL associated thread does not exist
	 *  @return handle thread handle as returned from pthread_create()
	 *
	 */
	inline	pthread_t getHandle (void) { return m_handle; }

	/*! @brief get exit code of associated thread
	 *
	 *  @return -1 associated thread is still running
	 *  @return other return code of associated thread
	 *
	 */
	int getExitCode (void)
	{
		return m_exitCode;
	}

	/*! @brief lock mutex
	 *
	 *  function locks mutex m_mutex. If it is already locked by another
	 *  thread, this function will block until it will be unlocked by
	 *  owning thread. Function should be used to synchronize access to
	 *  resources common to many threads
	 *
	 *  @return 0 associated thread is locked
	 *  @return other error code returned by pthread_mutex_lock()
	 *
	 */
	inline int Lock ()
	{
		int ret;
		while ((ret = pthread_mutex_lock (&m_mutex)) != 0)
		{
			if (errno != EINTR)
				break;
		}
		return ret;
	}

	/****************************************************************/
	/* Function:    Unlock ()                                       */
	/* In:          /                                               */
	/* Out:         /                                               */
	/* In/Out:      /                                               */
	/* Return:      0 - associated thread is unlocked               */
	/*              < 0 - error                                     */
	/* Description: Function 'unlocks' associated thread            */
	/****************************************************************/

	/*! @brief unlock mutex
	 *
	 *  function unlocks mutex m_mutex.
	 *
	 *  @return 0 associated thread is unlocked
	 *  @return other error code returned by pthread_mutex_unlock()
	 *
	 */
	inline int Unlock ()
	{
		int ret;
		while ((ret = pthread_mutex_unlock (&m_mutex)) != 0)
		{
			if (errno != EINTR)
				break;
		}
		return ret;
	}

private:
	pid_t	m_threadId;	//!< thread id
	pthread_t m_handle;	//!< handle of program thread associated with an instance of this class.
						//!< This handle is returned by pthread_create() and should be used in
						//!< any function dealing with thread handles. This variable is private.
						//!< Its value can be accessed with function getHabdle() and cannot be changed
	pthread_mutex_t m_mutex;	//!< semaphore provided to enable synchronous access to an instance
	int m_exitCode;	//!< associated thread exit code returned by Run()
};
