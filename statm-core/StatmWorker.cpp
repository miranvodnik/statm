/*
 * StatmWorker.cpp
 *
 *  Created on: 30. okt. 2015
 *      Author: miran
 */

#define	_GNU_SOURCE
#include <sched.h>
#include <fcntl.h>
#include <sys/sysinfo.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <uuid/uuid.h>
#include "StatmWorker.h"
#include "StatmStatDriver.h"
#include <iostream>
using namespace std;
using namespace statm_daemon;

namespace statm_daemon
{

//	file descriptors 3, 4, 5, 6 are inherited (!!!) by controlling entity
const	int	StatmWorker::g_localListener = ::g_localListener;	// worker's unix domain listener
const	int	StatmWorker::g_localClientReader = ::g_localWrkReader;	// worker's unix domain master client
const	int	StatmWorker::g_localClientWriter = ::g_localWrkWriter;	// worker's unix domain master client

StatRequest	StatmWorker::g_stopRequest;
StatmWorker*	StatmWorker::g_genWorker = new StatmWorker (0);

void sigint (int signo)
{
	// signo /= 0;
}

/*! @brief private constructor
 *
 *  this constructor cannot be used. Its only purpose is
 *  to create g_stopRequest by c++ running environment
 *
 */
StatmWorker::StatmWorker(int n) : StatmRunningContext (0, 0, 0, 0, 0)
{
	g_stopRequest.m_requestType = AdminRequestCode;

	StatAdminRequest*	p_StatAdminRequest = &g_stopRequest.StatRequest_u.m_adminRequest;
	StatMessageHeader*	header = (StatMessageHeader*) p_StatAdminRequest;
	header->m_version.m_major = 1;
	header->m_version.m_minor = 0;
	header->m_index = -1;
	uuid_generate (header->m_hash);
	header->m_error = 0;

	p_StatAdminRequest->m_command = (u_int) StatmAdminStopWord;
	p_StatAdminRequest->m_parameters.m_parameters_len = 0;
	p_StatAdminRequest->m_parameters.m_parameters_val = 0;
}

/*! @brief working entity constructor
 *
 *  initializes internal data structures and provides its superclass
 *  multiplexer with start, stop and time-hook routines
 *
 *  @param n number of program parameters
 *  @param p program parameters
 *
 */
StatmWorker::StatmWorker(int n, char* p[]) : StatmRunningContext (StartWorker, StopWorker, TimeHookWorker, this, "statm-worker")
{
#if defined(DEBUG)
	if (g_debug) statmErrReport (SC_STATM, SC_ERR, "CREATE StatmWorker %ld", this);
#endif

	m_localClientReaderHandler = 0;
	m_localClientWriterHandler = 0;
	m_localListenerHandler = 0;

	m_outputBuffer = 0;
	m_outputPtr = 0;
	m_outputEnd = 0;

	m_hangupSignalHandler = 0;
	m_abortSignalHandler = 0;

	int	dummy = 0;
	m_intSize = xdr_sizeof ((xdrproc_t)xdr_int, &dummy);

	signal (SIGINT, sigint);
#if defined (STATM_HAM)
	m_hamThread = 0;
#endif
}

/*! @brief working entity destructor
 *
 *  releases all allocated resources
 *
 */
StatmWorker::~StatmWorker()
{
#if defined(DEBUG)
	if (g_debug) statmErrReport (SC_STATM, SC_ERR, "DELETE StatmWorker %ld", this);
#endif
	Release ();
}

/*! release allocated resources
 *
 *  removes all I/O handlers for listening sockets and controlling pipes.
 *  Terminates working and client driver threads and unloads database.
 *  Finally it terminates internal message queue.
 *
 */
void	StatmWorker::Release ()
{
	if (m_localClientReaderHandler != 0)
		RemoveDescriptor(m_localClientReaderHandler);
	m_localClientReaderHandler = 0;

	if (m_localClientWriterHandler != 0)
		RemoveDescriptor(m_localClientWriterHandler);
	m_localClientWriterHandler = 0;

	if (m_localListenerHandler != 0)
		RemoveDescriptor(m_localListenerHandler);
	m_localListenerHandler = 0;

	if (m_statTimer != 0)
		DisableTimer (m_statTimer);
	m_statTimer = 0;

	if (m_hangupSignalHandler != 0)
		RemoveSignal(m_hangupSignalHandler);
	m_hangupSignalHandler = 0;

	if (m_abortSignalHandler != 0)
		RemoveSignal(m_abortSignalHandler);
	m_abortSignalHandler = 0;

	if (m_outputBuffer != 0)
		free (m_outputBuffer);
	m_outputBuffer = 0;
	m_outputPtr = 0;
	m_outputEnd = 0;

	StatmWorkingThread::StopWorkingThreads ();
	StatmClientDriver::StopClientDrivers ();
	StatmStatDriver::ReleaseStatDriver();

	#if defined (STATM_HAM)
	if (m_hamThread != 0)
	{
		m_hamThread->Terminate ();
		delete m_hamThread;
		m_hamThread = 0;
	}
#endif
	Stop ();
}

/*! @brief SIGHUP signal handler
 *
 *  when SIGHUP is detected by working environment (one of I/O multiplexing
 *  functionalities) this handler is called: it logs message and posts stop
 *  administrative message to controlling entity through output pipe controller
 *  connection
 *
 */
void	StatmWorker::HangupSignalHandler (StatmRunningContext *ctx, ctx_fddes_t handler, siginfo_t* info)
{
	statmErrReport (SC_STATM, SC_ERR, err_info("HANGUP detected: statm worker, PID = %d"), getpid());
	statmErrReport (SC_STATM, SC_ERR, err_info("post stop request to statm controller"));
	PostAdminRequest (&g_stopRequest);
}

/*! @brief ABORT signal handler
 *
 *  abort signal is reported
 *
 */
void	StatmWorker::AbortSignalHandler (StatmRunningContext *ctx, ctx_fddes_t handler, siginfo_t* info)
{
	statmErrReport (SC_STATM, SC_ERR, err_info("ABORT detected: statm worker, PID = %d"), getpid());
}

/*! @brief start routine
 *
 *  this routine is invoked by I/O multiplexing working environment. It makes all necessary steps
 *  to start working entity:
 *
 *  - first it checks controller connections making simple fcntl() calls
 *
 *  - signal handlers for SIGHUP and SIGABRT are installed next
 *
 *  - next step loads database files
 *
 *  - internal message queue is created and initialized
 *
 *  - client driver threads are started
 *
 *  - working threads are started
 *
 *  - handlers to I/O redirections are installed
 *
 *  - FD 3 redirection: UNIX domain listening socket
 *  - FD 4 redirection: network TCP/IP listening socket
 *  - FD 5 redirection: local message queue listening socket
 *  - FD 6 redirection: input pipe controller connection
 *  - FD 7 redirection: output pipe controller connection
 *
 *  If all these steps succeed than working entity will be started otherwise it will throw
 *  exception causing it to cease.
 *
 *  @param ctx reference to superclass part of this object (I/O multiplexer core)
 *
 */
void	StatmWorker::StartWorker (StatmRunningContext *ctx)
{
	if ((fcntl (g_localClientReader, F_GETFD, 0) < 0) || (fcntl (g_localClientWriter, F_GETFD, 0) < 0))
		throw	2;

	StatmCommon::entity (StatWorkerEntity);
	m_hangupSignalHandler = RegisterSignal(SIGHUP, HangupSignalHandler, this);
	m_abortSignalHandler = RegisterSignal(SIGABRT, AbortSignalHandler, this);
//	signal (SIGABRT, SIG_IGN);

#if defined (STATM_HAM)
	m_hamThread = new StatmHamThread ();
	m_hamThread->Start ();
	while (m_hamThread->useHam())
	{
		switch (m_hamThread->Registration())
		{
		case	EV_START:
			sleep (1);
			continue;
		case	EV_REG_OK:
			break;
		default:
			throw	3;
		}
		break;
	}
#endif

	if (StatmStatDriver::InitStatDriver(ctx) < 0)
	{
		statmErrReport (SC_STATM, SC_ERR, err_info("cannot initialize statistical driver"));
#if defined (STATM_HAM)
		if (m_hamThread->useHam())
			m_hamThread->WorkerReady (false);
#endif
		throw	1;
	}

	StatmMessageQueue::Initialize ();
	StatmClientDriver::StartClientDrivers ();
	StatmWorkingThread::StartWorkingThreads ();
	StatmStatDriver::InitStatAdapter ();

	m_localListenerHandler = ctx->RegisterDescriptor (EPOLLIN, g_localListener, HandleLocalListener, this, ctx_info);
	m_localClientReaderHandler = ctx->RegisterDescriptor (EPOLLIN, g_localClientReader, HandleMainLocalClientReader, this, ctx_info);
	m_localClientWriterHandler = ctx->RegisterDescriptor (EPOLLIN, g_localClientWriter, HandleMainLocalClientWriter, this, ctx_info);

	{	// start DB data refresh in the middle of data generation period
		struct timespec t = ctx->realTime();
		time_t now = time (0);
		struct tm tnow;
		localtime_r (&now, &tnow);
		int diff = (900 + 450) - ((tnow.tm_min % 15) * 60 + tnow.tm_sec);
		if (diff > 960)
			diff -= 900;
		statmErrReport (SC_STATM, SC_ERR, err_info ("stat DB data refreshed after %d seconds"), diff);

		t.tv_sec += diff;
		t.tv_nsec = 0;
		m_statTimer = ctx->RegisterTimer (t, RestartStatisticalAdapter, this, ctx_info);
	}

#if defined (STATM_HAM)
	if (m_hamThread->useHam())
		m_hamThread->WorkerReady (true);
#endif
}

/*! @brief stop routine
 *
 *  this routine is invoked by working environment of I/O multiplexer
 *  it reports termination message
 *
 */
void	StatmWorker::StopWorker (StatmRunningContext *ctx)
{
#if defined (STATM_HAM)
	if (m_hamThread != 0)
		m_hamThread->Terminate ();
#endif
	StatmStatDriver::ReleaseStatAdapter();
	StatmStatDriver::ReleaseStatDriver();
	statmErrReport (SC_STATM, SC_ERR, "statm worker terminated, PID = %d", getpid());
}

/*! @brief time-hook routine
 *
 *  this routine is invoked by working environment of I/O multiplexer
 *  if system time changes; useless routine
 *
 */
void	StatmWorker::TimeHookWorker (StatmRunningContext *ctx, struct timespec oldTime, struct timespec newTime, long long timeDiff)
{

}

/*! @brief controller pipes handler
 *
 *  this function is registered with I/O multiplexing mechanism to handle
 *  I/O activity between working and controlling entities through pipes
 *  connecting them:
 *
 *  - input pipe: when this pipe is closed, working entity will report
 *  appropriate message and release all allocated resources causing it
 *  to cease
 *
 *  - output pipe: output pipe activity is needed to communicate messages
 *  to controlling entity. These messages can be created by working entity
 *  itself: detecting hangup causes stop request to be sent to controller.
 *  Administrative requests created by ordinary clients willing to restart
 *  or stop server, will also be transfered to controlling entity.
 *
 *  - exceptions: all I/O exceptions will cause working entity to release
 *  its resources causing it to cease
 *
 */
void	StatmWorker::HandleMainLocalClientReader (StatmRunningContext *ctx, uint flags, ctx_fddes_t handler, int fd)
{
	if (flags & EPOLLIN)
	{
		int	count;
		char	buff[1024];
		if ((count = read (fd, buff, 1024)) <= 0)
		{
			if (count == 0)
				statmErrReport (SC_STATM, SC_ERR, err_info ("statm controller connection closed"));
			else
				statmErrReport (SC_STATM, SC_ERR, err_info ("recv() failed, errno = %d"), errno);
			statmErrReport (SC_STATM, SC_ERR, err_info ("statm worker terminating, PID = %d"), getpid());
			Release ();
			return;
		}
	}
	else	if (flags & EPOLLOUT)
	{
		size_t	size = m_outputPtr - m_outputBuffer;
		ssize_t	count = write (fd, m_outputBuffer, size);
		if (count <= 0)
		{
			statmErrReport (SC_STATM, SC_ERR, err_info ("send() failed, errno = %d"), errno);
			Release ();
			return;
		}
		if ((size_t)count < size)
			memcpy (m_outputBuffer, m_outputBuffer + count, size - count);
		else
			DisableDescriptor(handler, EPOLLOUT);
		m_outputPtr -= count;
	}
	else
	{
		statmErrReport (SC_STATM, SC_ERR, err_info ("statm worker terminating, PID = %d"), getpid());
		Release ();
	}
}

/*! @brief controller pipes handler
 *
 *  this function is registered with I/O multiplexing mechanism to handle
 *  I/O activity between working and controlling entities through pipes
 *  connecting them:
 *
 *  - input pipe: when this pipe is closed, working entity will report
 *  appropriate message and release all allocated resources causing it
 *  to cease
 *
 *  - output pipe: output pipe activity is needed to communicate messages
 *  to controlling entity. These messages can be created by working entity
 *  itself: detecting hangup causes stop request to be sent to controller.
 *  Administrative requests created by ordinary clients willing to restart
 *  or stop server, will also be transfered to controlling entity.
 *
 *  - exceptions: all I/O exceptions will cause working entity to release
 *  its resources causing it to cease
 *
 */
void	StatmWorker::HandleMainLocalClientWriter (StatmRunningContext *ctx, uint flags, ctx_fddes_t handler, int fd)
{
	if (flags & EPOLLIN)
	{
		int	count;
		char	buff[1024];
		if ((count = read (fd, buff, 1024)) <= 0)
		{
			if (count == 0)
				statmErrReport (SC_STATM, SC_ERR, err_info ("statm controller connection closed"));
			else
				statmErrReport (SC_STATM, SC_ERR, err_info ("recv() failed, errno = %d"), errno);
			statmErrReport (SC_STATM, SC_ERR, err_info ("statm worker terminating, PID = %d"), getpid());
			Release ();
			return;
		}
	}
	else	if (flags & EPOLLOUT)
	{
		size_t	size = m_outputPtr - m_outputBuffer;
		ssize_t	count = write (fd, m_outputBuffer, size);
		if (count <= 0)
		{
			statmErrReport (SC_STATM, SC_ERR, err_info ("send() failed, errno = %d"), errno);
			Release ();
			return;
		}
		if ((size_t)count < size)
			memcpy (m_outputBuffer, m_outputBuffer + count, size - count);
		else
			DisableDescriptor(handler, EPOLLOUT);
		m_outputPtr -= count;
	}
	else
	{
		statmErrReport (SC_STATM, SC_ERR, err_info ("statm worker terminating, PID = %d"), getpid());
		Release ();
	}
}

/*! @brief UNIX domain listening socket handler
 *
 *  this function accepts local client connections (administrative and SQL requests)
 *  Every client is set in nonblocking I/O mode. After that its FD is sent to
 *  selected client driver thread (current client driver). If any of these steps fail,
 *  client FD is closed and appropriate error message is reported.
 *
 *  @param ctx I/O multiplexer reference (this object's superclass part)
 *  @param flags I/O flags (EPOLLIN, EPOLLOUT, ...) only EPOLLIN is valid for listening socket
 *  @param handler I/O multiplexing handler associated with this function
 *  @param fd UNIX domain listening socket
 *
 */
void	StatmWorker::HandleLocalListener (StatmRunningContext *ctx, uint flags, ctx_fddes_t handler, int fd)
{
	int	localClientSocket;
	int	gflags;
	struct sockaddr_un addr;
	socklen_t slen = sizeof(struct sockaddr_un);

	memset (&addr, 0, sizeof(struct sockaddr_un));
	if ((localClientSocket = accept (fd, (struct sockaddr*) &addr, &slen)) < 0)
	{
		statmErrReport (SC_STATM, SC_ERR, err_info ("accept() failed, errno = %d"), errno);
		return;
	}
	if ((gflags = fcntl (localClientSocket, F_GETFL, 0)) < 0)
	{
		statmErrReport (SC_STATM, SC_ERR, err_info ("fcntl(F_GETFL) failed, errno = %d"), errno);
		close (localClientSocket);
		return;
	}
	if (fcntl (localClientSocket, F_SETFL, gflags | O_NONBLOCK) < 0)
	{
		statmErrReport (SC_STATM, SC_ERR, err_info ("fcntl(F_SETFL) failed, errno = %d"), errno);
		close (localClientSocket);
		return;
	}
	if (StatmClientDriver::currentClientDriver()->SendLocalClientHandlerActivation(localClientSocket) < 0)
	{
		statmErrReport (SC_STATM, SC_ERR, err_info ("cannot activate local client"));
		close (localClientSocket);
		return;
	}
}

/*! @brief post administrative request
 *
 *  when HANGUP signal is detected, stop message (g_stopRequest) need to
 *  be sent to controlling entity. And that is exactly what this function
 *  do: it allocates sufficient space, serializes message and enables
 *  output processing on output pipe connection with controlling entity
 *
 *  @param cln unused reference to local client connection, since client
 *  is actually known: controlling entity is nothing but special client
 *  obeying same communication rules as any other client
 *  @param req request reference
 *
 */
void	StatmWorker::HandleAdminRequest (StatmLocalClient* cln, StatRequest* req)
{
	u_int	msgSize = xdr_sizeof ((xdrproc_t)xdr_StatRequest, req);
	u_int	needSpace = msgSize + m_intSize;
	u_int	usedSpace = m_outputPtr - m_outputBuffer;
	u_int	freeSpace = m_outputEnd - m_outputPtr;

	if (freeSpace < needSpace)
	{
		needSpace += usedSpace;
		needSpace >>= 10;
		needSpace++;
		needSpace <<= 10;

		u_char*	buffer = (u_char*) malloc (needSpace);
		if (buffer == 0)
		{
			statmErrReport (SC_STATM, SC_ERR, err_info ("malloc() failed, errno = %d"), errno);
			return;
		}

		if (m_outputBuffer != 0)
		{
			memcpy (buffer, m_outputBuffer, usedSpace);
			free (m_outputBuffer);
		}
		m_outputBuffer = buffer;
		m_outputPtr = buffer + usedSpace;
		m_outputEnd = buffer + needSpace;
		freeSpace = needSpace - usedSpace;
	}

	XDR	xdr;

	xdrmem_create(&xdr, (char*) m_outputPtr, freeSpace, XDR_ENCODE);
	if (xdr_int(&xdr, (int*) &msgSize) != TRUE)
	{
		statmErrReport (SC_STATM, SC_ERR, err_info ("xdr_int() failed"));
		xdr_destroy (&xdr);
		return;
	}
	xdr_destroy (&xdr);

	xdrmem_create(&xdr, (char*) (m_outputPtr + m_intSize), freeSpace - m_intSize, XDR_ENCODE);
	if (xdr_StatRequest (&xdr, req) != TRUE)
	{
		statmErrReport (SC_STATM, SC_ERR, err_info ("xdr_StatRequest() failed"));
		xdr_destroy (&xdr);
		return;
	}
	xdr_destroy (&xdr);

	u_int	pos = xdr_getpos (&xdr);
	m_outputPtr += pos + m_intSize;

	EnableDescriptor (m_localClientWriterHandler, EPOLLOUT);
}

void	StatmWorker::RestartStatisticalAdapter (StatmRunningContext *ctx, ctx_timer_t handler, struct timespec t)
{
	statmErrReport (SC_STATM, SC_ERR, err_info ("Reload statistical DB data"));

	StatmStatDriver::ReleaseStatAdapter();
	StatmStatDriver::InitStatAdapter();

	t.tv_sec += 900;
	m_statTimer = ctx->RegisterTimer (t, RestartStatisticalAdapter, this, ctx_info);
}

} /* namespace statm_daemon */
