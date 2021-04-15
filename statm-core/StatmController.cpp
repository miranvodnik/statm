/*
 * StatmController.cpp
 *
 *  Created on: 28. okt. 2015
 *      Author: miran
 */

#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/wait.h>
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
#include "StatmController.h"
#include "StatmMessages.h"
#include "StatmSemval.h"
using namespace statm_daemon;

namespace statm_daemon
{

const	string	StatmController::g_localSocketPath = StatmCommon::localSocketPath();	//!< UNIX domain socket name
const	int	StatmController::g_localListener = ::g_localListener;	//!< working process's UNIX domain socket redirection
int	StatmController::g_messageIndex = 0;

/*! @brief controlling entity object constructor
 *
 *  It provides start, stop and time hook routines to its superclass;
 *  It fetches command parameters
 *  It initializes internal data structures
 *
 */
StatmController::StatmController (int n, char**p) : StatmRunningContext (StartController, StopController, TimeHookController, this, "statm-ctl")
{
#if defined(DEBUG)
	if (g_debug) statmErrReport (SC_STATM, SC_ERR, "CREATE StatmController %ld", this);
#endif
	if (n < 2)
	{
		m_commandType = StatmAdminWord;
		m_commandCode = StatmAdminStatusWord;
	}
	else
	{
		m_commandType = StatmCommon::findWord (p[1]);
		if (n < 3)
			m_commandCode = StatmAdminStatusWord;
		else
			m_commandCode = StatmCommon::findWord (p[2]);
	}

	m_argc = n;
	m_argv = p;

	m_sync = -1;

	m_localListenFd = -1;
	m_localClientFd = -1;
	m_masterClientReaderFd = -1;
	m_masterClientWriterFd = -1;

	m_worker = (pid_t) -1;

	m_localClientHandler = 0;
	m_masterClientReaderHandler = 0;
	m_masterClientWriterHandler = 0;

	m_localClientIOTimer = 0;
	m_masterClientIOTimer = 0;
	m_sigcldSignalHandler = 0;
	m_hangupSignalHandler = 0;

	m_inputBuffer = 0;
	m_inputPtr = 0;
	m_inputEnd = 0;

	m_outputBuffer = 0;
	m_outputPtr = 0;
	m_outputEnd = 0;

	int	dummy = 0;
	m_intSize = xdr_sizeof ((xdrproc_t)xdr_int, &dummy);
	m_isController = false;
	m_deamonized = false;
}

/*! brief controlling entity object destructor
 *
 *  it releases all internal data structures
 *
 */
StatmController::~StatmController()
{
#if defined(DEBUG)
	if (g_debug) statmErrReport (SC_STATM, SC_ERR, "DELETE StatmController %ld", this);
#endif
	Release ();
}

/*! brief start routine
 *
 *  Implementation of start routine required by StatmRunningContext interface.
 *  If program has been given meaningful command parameters then
 *  this routine makes first step of quite simple procedure which
 *  will enable correct invocation of program regardless of how many
 *  instances of program are started simultaneously. Main purpose of
 *  this procedure is, that mostly one of these instances execute
 *  successfully as process controller. All other instances will simply
 *  cease or execute as client
 *
 *  First step of program invocation is simple: if program parameters
 *  are meaningful then the second step is prepared with registering
 *  timer handler for AccessSyncPath(), which will be invoked as soon
 *  as possible
 *
 *  @param ctx reference to superclass part of this object
 *
 *  @see StatmRunningContext
 *
 */
void	StatmController::StartController (StatmRunningContext *ctx)
{
	if (m_commandCode != StatmFirstWord)
		ctx->RegisterTimer (ctx->realTime(), AccessSyncPath, this, ctx_info);
}

/*! brief stop routine
 *
 *  Implementation of stop routine required by StatmRunningContext interface.
 *  It reports message
 *
 *  @param ctx reference to superclass part of this object
 *
 *  @see StatmRunningContext
 *
 */
void	StatmController::StopController (StatmRunningContext *ctx)
{
	if (m_isController)
		statmErrReport (SC_STATM, SC_ERR, "STATM controller terminated, PID = %d", getpid());
}

/*! brief time hook routine
 *
 *  Implementation of time hook routine required by StatmRunningContext interface.
 *  It reports time change message
 *
 *  @param ctx reference to superclass part of this object
 *  @param oldTime time stamp before time change
 *  @param new Time time stamp after time change
 *  @param timeDiff difference between time stamps expressed in nanoseconds
 *
 *  @see StatmRunningContext
 *
 */
void	StatmController::TimeHookController (StatmRunningContext *ctx, struct timespec oldTime, struct timespec newTime, long long timeDiff)
{
	statmErrReport (SC_STATM, SC_ERR, "STATM controller time correction: old time = %d,%06d, new time = %d,%06d",
		oldTime.tv_sec, oldTime.tv_nsec, newTime.tv_sec, newTime.tv_nsec);
}

/*! @brief release procedure
 *
 *  releases all internal data structures
 *
 */
void	StatmController::Release ()
{
	m_sync = -1;

	if (m_localListenFd > 0)
		close (m_localListenFd);
	m_localListenFd = -1;

	if (m_localClientFd > 0)
		close (m_localClientFd);
	m_localClientFd = -1;

	if (m_masterClientReaderFd > 0)
		close (m_masterClientReaderFd);
	m_masterClientReaderFd = -1;

	if (m_masterClientWriterFd > 0)
		close (m_masterClientWriterFd);
	m_masterClientWriterFd = -1;

	m_worker = (pid_t) -1;

	if (m_localClientHandler != 0)
		RemoveDescriptor(m_localClientHandler);
	m_localClientHandler = 0;

	if (m_masterClientReaderHandler != 0)
		RemoveDescriptor(m_masterClientReaderHandler);
	m_masterClientReaderHandler = 0;

	if (m_masterClientWriterHandler != 0)
		RemoveDescriptor(m_masterClientWriterHandler);
	m_masterClientWriterHandler = 0;

	if (m_localClientIOTimer != 0)
		DisableTimer(m_localClientIOTimer);
	m_localClientIOTimer = 0;

	if (m_masterClientIOTimer != 0)
		DisableTimer(m_masterClientIOTimer);
	m_masterClientIOTimer = 0;

	if (m_sigcldSignalHandler != 0)
		RemoveSignal(m_sigcldSignalHandler);
	m_sigcldSignalHandler = 0;

	if (m_hangupSignalHandler != 0)
		RemoveSignal(m_hangupSignalHandler);
	m_hangupSignalHandler = 0;

	if (m_inputBuffer != 0)
		free (m_inputBuffer);
	m_inputBuffer = 0;
	m_inputPtr = 0;
	m_inputEnd = 0;

	if (m_outputBuffer != 0)
		free (m_outputBuffer);
	m_outputBuffer = 0;
	m_outputPtr = 0;
	m_outputEnd = 0;

	Stop ();
}

/*! @brief lock controlling path
 *
 *  This function locks synchronization semaphore owned by this object.
 *
 *  @return 0 success - controlling path has been secured
 *  @return -1 failure - controlling path has not been secured
 */
int	StatmController::LockControllingPath (void)
{
	struct	sembuf	sb;

	sb.sem_num = 0;
	sb.sem_op = -1;
	sb.sem_flg = SEM_UNDO;
	if (semop (m_sync, &sb, 1) < 0)
	{
		statmErrReport (SC_STATM, SC_ERR, err_info("cannot lock controlling path, semop() failed, errno = %d"), errno);
		return	-1;
	}
	return	0;
}

/*! @brief unlock controlling path
 *
 *  This function unlocks synchronization semaphore owned by this object.
 *
 *  @return 0 success - controlling path has been unsecured
 *  @return -1 failure - controlling path has not been unsecured
 */
int	StatmController::UnlockControllingPath (void)
{
	struct	sembuf	sb;

	sb.sem_num = 0;
	sb.sem_op = 1;
	sb.sem_flg = SEM_UNDO;
	if (semop (m_sync, &sb, 1) < 0)
	{
		statmErrReport (SC_STATM, SC_ERR, err_info("cannot unlock controlling path, semop() failed, errno = %d"), errno);
		return	-1;
	}
	return	0;
}

/*! @brief access synchronization path
 *
 *  Second step of program invocation: This
 *  function provides access to semaphore with key STATM_SEM_KEY
 *  either creating or simply accessing it. If that can be done,
 *  function prepares next step by registering timer handler
 *  implemented by DetermineStatmEntityRole(), which will be
 *  invoked as soon as possible.
 *
 *  @param ctx reference to superclass part of this object
 *  @param handler reference to registration handler of this routine
 *  @param t time stamp of invocation
 *
 */
void	StatmController::AccessSyncPath (StatmRunningContext *ctx, void *handler, struct timespec t)
{
	if ((m_sync = semget (STATM_SEM_KEY, 0, 0)) < 0)
	{
		if (errno == EINTR)
		{
			statmErrReport (SC_STATM, SC_WRN, err_info("cannot access synchronization path, semget() failed, errno = %d"), errno);
			ctx->RegisterTimer (t, AccessSyncPath, this, ctx_info);
			return;
		}
		if ((m_sync = semget (STATM_SEM_KEY, 1, 0666 | IPC_CREAT | IPC_EXCL)) < 0)
		{
			if (errno == EINTR)
			{
				statmErrReport (SC_STATM, SC_WRN, err_info("cannot access synchronization path, semget() failed, errno = %d"), errno);
				ctx->RegisterTimer (t, AccessSyncPath, this, ctx_info);
				return;
			}
			t.tv_sec += 1;
			statmErrReport (SC_STATM, SC_ERR, err_info("cannot access synchronization path, semget() failed, errno = %d"), errno);
			ctx->RegisterTimer (t, AccessSyncPath, this, ctx_info);
			return;
		}
		{
			union semval sv;
			sv.value = 1;
			semctl (m_sync, 0, SETVAL, sv);
		}
	}
	ctx->RegisterTimer(t, DetermineStatmEntityRole, this, ctx_info);
}

/*! brief determine process role
 *
 *  Third step of program invocation: this step is secured by semaphore
 *  which ensures that almost one instance of program executes this code
 *  at once.
 *
 *  Program instance, which successfully enter secured section will first
 *  try to create obvious client entity. If that client can be created it is
 *  due to the fact that controller and working entity already exist. In this
 *  case client will be prepared to make next step by registering
 *  DriveStatClientEntity() timer, which will be executed as soon as possible
 *
 *  If client cannot be created (probably because controller an working entity
 *  are not running) command codes are checked to see if it is meaningful
 *  to start controlling entity. Only 'start' and 'restart' commands are
 *  allowed to start controller. If that is not the case, program will simply cease.
 *
 *  In case of appropriate command codes ('start' or 'restart') controlling
 *  entity is created. If successful controller will be prepared to make next step
 *  by registering DriveStatControllerEntity() timer, which will be executed
 *  as soon as possible.
 *
 *  If even controlling entity cannot be created the whole step is repeated
 *  as soon as possible.
 *
 *  @param ctx reference to superclass part of this object
 *  @param handler reference to registration handler of this routine
 *  @param t time stamp of invocation
 *
 */
void	StatmController::DetermineStatmEntityRole (StatmRunningContext *ctx, void *handler, struct timespec t)
{
	if (LockControllingPath () < 0)
	{
		t.tv_sec += 1;
		statmErrReport (SC_STATM, SC_ERR, err_info("cannot determine statm entity role"));
		ctx->RegisterTimer(t, DetermineStatmEntityRole, this, ctx_info);
		return;
	}

	if (CreateClientEntity () == 0)
	{
		UnlockControllingPath ();
		ctx->RegisterTimer (t, DriveStatClientEntity, this, ctx_info);
		return;
	}

	if ((m_commandCode != StatmAdminStartWord) && (m_commandCode != StatmAdminRestartWord))
	{
		statmErrReport (SC_STATM, SC_ERR, err_info("illegal command code for non-client role, code = %d"), m_commandCode);
		UnlockControllingPath ();
		Release ();
		return;
	}

	if (CreateControllerEntity () == 0)
	{
		UnlockControllingPath ();
		ctx->RegisterTimer (t, DriveStatControllerEntity, this, ctx_info);
		return;
	}

	statmErrReport (SC_STATM, SC_WRN, err_info("undetermined statm entity role, trying again"));
	UnlockControllingPath ();
	t.tv_sec += 1;
	ctx->RegisterTimer (t, DetermineStatmEntityRole, this, ctx_info);
}

/*! @brief create client entity
 *
 *  Creates client entity: only local UNIX domain client can be created
 *
 *  @return 0 client entity has been created
 *  @return -1 client entity has not been created, error has been logged
 */
int	StatmController::CreateClientEntity (void)
{
	if (CreateLocalClient() == 0)
		return	0;
	statmErrReport (SC_STATM, SC_WRN, err_info("cannot create client entity role"));
	return	-1;
}

/*! @brief create local UNIX domain client
 *
 *  Function returns immediately if local client already exists. Otherwise
 *  it will create UNIX domain socket, connect to UNIX domain server and
 *  sets nonblocking I/O behavior.
 *
 *  @return 0 all steps mentioned above succeed
 *  @return -1 any step mention above failed; error is logged in that case
 */
int	StatmController::CreateLocalClient()
{
	if (m_localClientFd > 0)
	{
		statmErrReport (SC_STATM, SC_WRN, err_info("statm entity canot become local client because it is already client"));
		return	0;
	}

	struct	sockaddr_un	clnaddr;

	if ((m_localClientFd = socket (AF_LOCAL, SOCK_STREAM, 0)) < 0)
	{
		statmErrReport (SC_STATM, SC_ERR, err_info("cannot create local client socket, socket() failed, errno = %d"), errno);
		return	-1;
	}

	bzero (&clnaddr, sizeof (clnaddr));
	clnaddr.sun_family = AF_LOCAL;
	strncpy (clnaddr.sun_path, g_localSocketPath.c_str(), sizeof (clnaddr.sun_path) - 1);
	if (connect (m_localClientFd, (sockaddr*) &clnaddr, sizeof (clnaddr)) < 0)
	{
		statmErrReport (SC_STATM, SC_ERR, err_info("cannot connect local client socket, connect() failed, errno = %d"), errno);
		close (m_localClientFd);
		m_localClientFd = -1;
		return	-1;
	}

	int	gflags;

	if ((gflags = fcntl (m_localClientFd, F_GETFL, 0)) < 0)
	{
		statmErrReport (SC_STATM, SC_ERR, err_info("cannot retrieve local client socket flags, fcntl() failed, errno = %d"), errno);
		close (m_localClientFd);
		m_localClientFd = -1;
		return	-1;
	}

	if (fcntl (m_localClientFd, F_SETFL, gflags | O_NONBLOCK) < 0)
	{
		statmErrReport (SC_STATM, SC_ERR, err_info("cannot update local client socket flags, fcntl() failed, errno = %d"), errno);
		close (m_localClientFd);
		m_localClientFd = -1;
		return	-1;
	}

	return	0;
}

/*! @brief create controlling entity
 *
 *  function will create all listening entities: local UNIX domain server,
 *  remote BSD TCP/IP server and message queue server
 *
 *  @return 0 all servers created
 *  @return -1 any server failed to create
 *
 */
int	StatmController::CreateControllerEntity (void)
{
	if (CreateLocalListener() != 0)
	{
		statmErrReport (SC_STATM, SC_ERR, err_info("controlling entity creation failed"));
		return	-1;
	}
	return	0;
}

/*! @brief create local UNIX domain server
 *
 *  Function will fail if process already represent existing
 *  client or controller entity
 *
 *  If that is not the case, function will create socket, bind
 *  it to UNIX domain path, make it listening socket and sets
 *  nonblocking I/O behavior
 *
 *  @return 0 all steps mentioned above succeed
 *  @return -1 any step mentioned above fail
 *
 */
int	StatmController::CreateLocalListener()
{
	if (m_localClientFd > 0)
	{
		statmErrReport (SC_STATM, SC_WRN, err_info("cannot create local listener: local listener already exists"));
		return	-1;
	}
	if (m_localListenFd > 0)
	{
		statmErrReport (SC_STATM, SC_WRN, err_info("cannot create local listener: local listener already exists"));
		return	-1;
	}

	if ((m_localListenFd = socket (AF_LOCAL, SOCK_STREAM, 0)) < 0)
	{
		statmErrReport (SC_STATM, SC_ERR, err_info("cannot create local listener: socket() failed, errno = %d"), errno);
		return	-1;
	}

	unlink (g_localSocketPath.c_str());

	struct	sockaddr_un	srvaddr;

	bzero (&srvaddr, sizeof (srvaddr));
	srvaddr.sun_family = AF_LOCAL;
	strncpy (srvaddr.sun_path, g_localSocketPath.c_str(), sizeof (srvaddr.sun_path) - 1);
	if (bind (m_localListenFd, (sockaddr*) &srvaddr, sizeof (srvaddr)) < 0)
	{
		statmErrReport (SC_STATM, SC_ERR, err_info("cannot create local listener: bind(%s) failed, errno = %d"), g_localSocketPath.c_str(), errno);
		close (m_localListenFd);
		m_localListenFd = -1;
		return	-1;
	}

	if (listen (m_localListenFd, SOMAXCONN) < 0)
	{
		statmErrReport (SC_STATM, SC_ERR, err_info("cannot create local listener: listen() failed, errno = %d"), errno);
		close (m_localListenFd);
		m_localListenFd = -1;
		return	-1;
	}
	m_isController = true;

	int	gflags;

	if ((gflags = fcntl (m_localListenFd, F_GETFL, 0)) < 0)
	{
		statmErrReport (SC_STATM, SC_ERR, err_info("cannot create local listener: fcntl() failed, errno = %d"), errno);
		close (m_localListenFd);
		m_localListenFd = -1;
		return	-1;
	}

	if (fcntl (m_localListenFd, F_SETFL, gflags | O_NONBLOCK) < 0)
	{
		statmErrReport (SC_STATM, SC_ERR, err_info("cannot create local listener: fcntl() failed, errno = %d"), errno);
		close (m_localListenFd);
		m_localListenFd = -1;
		return	-1;
	}

	return	0;
}

/*! @brief drive client entity
 *
 *  function prepares message, which should be sent to
 *  controlling entity
 *
 *  @param ctx reference to superclass part of this object
 *  @param handler reference to registration handler of this routine
 *  @param t time stamp of invocation
 *
 */
void	StatmController::DriveStatClientEntity (StatmRunningContext *ctx, void *handler, struct timespec t)
{
	StatmCommon::entity (StatClientEntity);
	m_localClientHandler = RegisterDescriptor(EPOLLIN, m_localClientFd, HandleLocalClient, this, ctx_info);
	if (PrepareLocalClientMessage() < 0)
	{
		statmErrReport (SC_STATM, SC_ERR, err_info("client entity creation failed: cannot prepare client message"));
		Release();
		return;
	}
	t.tv_sec += 10;
	m_localClientIOTimer = RegisterTimer(t, LocalClientIOTimer, this, ctx_info);
}

/*! @brief drive controlling entity
 *
 *  Function performs two tasks: it daemonizes process
 *  and starts working entity
 *
 */
void	StatmController::DriveStatControllerEntity (StatmRunningContext *ctx, void *handler, struct timespec t)
{
	BecomeDaemon();
	ForkWorker();
}

/*! @brief daemonize process
 *
 *  function clones itself, making clone to run in background
 *  and then ceases. Controlling entity now runs as a
 *  background process
 */
void	StatmController::BecomeDaemon ()
{
	if (m_deamonized)
		return;

	pid_t	pid;
	pid_t	spid;

	switch (pid = fork ())
	{
	case	-1:
		statmErrReport (SC_STATM, SC_ERR, err_info("statm-controller entity cannot become daemon, fork() failed, errno = %d"), errno);
		return;
	case	0:
		m_deamonized = true;
		break;
	default:
		exit (0);
		break;
	}

	if ((spid = setsid ()) == (pid_t) -1)
		statmErrReport (SC_STATM, SC_ERR, err_info("statm-controller entity cannot become session leader, setsid() failed, errno = %d"), errno);
	else
		statmErrReport (SC_STATM, SC_ERR, err_info("statm-controller starting new session - PID = %d"), spid);

	umask (027);
}

/*! @brief fork working entity
 *
 *  Function follows these steps:
 *
 *  two pipes are created connecting controlling and working
 *  entity. This connections are called master client connections
 *  Controller entity is thus called master client. All other
 *  clients (client entites) never connect to controlling entity
 *  since this entity redirects all I/O activity to working
 *  entity.
 *
 *  next step creates working entity by cloning controlling entity
 *  and running the same process with argument 'worker'. Prior to
 *  running new process cloned controller must redirect its listening
 *  file descriptors to predefined values used by working entity.
 *  Working entity assumes that local listening socket number, network
 *  listening socket number, message queue server FD, pipe flowing from
 *  controller to worker and pipe flowing from worker to controller
 *  have the following numbers: 3, 4, 5, 6 and 7. To achieve this
 *  both cloned and original controller makes appropriate redirections
 *  using dup() and close() system calls. When redirections are established
 *  it starts new working entity and ceases, living alive original controller
 *  entity
 *
 *  Since controlling entity does not need listening sockets any more,
 *  it closes them. It then creates I/O mechanisms to handle communication
 *  with working entity: it registers I/O handlers on open ends of pipes
 *  created at the beginning of this function using function HandleMasterClient()
 *
 *  @return 0 all steps done without errors
 *  @return -1 any step failed
 */
int	StatmController::ForkWorker ()
{
	int	p1[2];
	int	p2[2];

	if (pipe (p1) < 0)
	{
		statmErrReport (SC_STATM, SC_ERR, err_info("statm-controller entity cannot create working entity, pipe() failed, errno = %d"), errno);
		return	-1;
	}

	if (pipe (p2) < 0)
	{
		statmErrReport (SC_STATM, SC_ERR, err_info("statm-controller entity cannot create working entity, pipe() failed, errno = %d"), errno);
		return	-1;
	}

	switch (m_worker = fork())
	{
	case	-1:
		statmErrReport (SC_STATM, SC_ERR, err_info("statm-controller entity cannot create working entity, fork() failed, errno = %d"), errno);
		return	-1;
	case	0:
		//
		//	file descriptors created by controller and used by worker:
		//	0, 1, 2 are closed and not used
		//
		//	fd 3 - unix domain listening socket
		if (m_localListenFd != g_localListener)
		{
			close (g_localListener);
			dup2 (m_localListenFd, g_localListener);
			close (m_localListenFd);
		}
		//	fd 6 - pipe: controller --> worker
		close (p1[1]);
		if (p1[0] != g_localWrkReader)
		{
			close (g_localWrkReader);
			dup2 (p1[0], g_localWrkReader);
			close (p1[0]);
		}
		//	fd 7 - pipe: controller <-- worker
		close (p2[0]);
		if (p2[1] != g_localWrkWriter)
		{
			close (g_localWrkWriter);
			dup2 (p2[1], g_localWrkWriter);
			close (p2[1]);
		}
		for (int i = g_localWrkWriter + 1; i < 1024; ++i)
			close (i);
		{
			const char* const	cmd[] = {"statm-worker", "worker", 0};
			execvp (cmd[0], (char*const*) cmd);
			statmErrReport (SC_STATM, SC_ERR, err_info("statm-controller entity cannot run working entity, execvp () failed, errno = %d"), errno);
			exit (0);
		}
		return	-1;
	default:
		//	disambiguate controller <--> worker pipes
		close (p1[0]);
		close (p2[1]);
		break;
	}

	StatmCommon::entity (StatControllerEntity);

	statmErrReport (SC_STATM, SC_APL, err_info("statm-working entity created - PID = %d"), m_worker);
	m_sigcldSignalHandler = RegisterSignal(SIGCLD, SigcldSignalHandler, this);
	m_hangupSignalHandler = RegisterSignal(SIGHUP, HangupSignalHandler, this);

	close (m_localListenFd);
	m_localListenFd = -1;

	m_masterClientReaderFd = p2[0];
	m_masterClientWriterFd = p1[1];

	m_masterClientReaderHandler = RegisterDescriptor(EPOLLIN, m_masterClientReaderFd, HandleMasterClient, this, ctx_info);
	m_masterClientWriterHandler = RegisterDescriptor(EPOLLIN, m_masterClientWriterFd, HandleMasterClient, this, ctx_info);
	return	0;
}

/*! @brief SIGCLD signal handler
 *
 *  SIGCLD is generated by system when working entity dies. In that case
 *  timer is created by registering function WaitWorkingEntity(), which
 *  will handle died process
 *
 *  @param ctx reference to superclass part of this object
 *  @param handler reference to registration handler of this routine
 *  @param info original signal information as caught by infrastructure
 */
void	StatmController::SigcldSignalHandler (StatmRunningContext *ctx, ctx_fddes_t handler, siginfo_t* info)
{
	m_commandCode = StatmAdminStopWord;
	RegisterTimer (ctx->realTime(), WaitWorkingEntity, this, ctx_info);
}

/*! @brief SIGHUP signal handler
 *
 *  SIGHUP can be generated by system (shutdown, ...) or by working
 *  entity. In any case communication pipes with working entity  are
 *  close which will cause working entity to cease. That is why timer
 *  is created by registering WaitWorkingEntity(), which will handle
 *  died working process
 *
 *  @param ctx reference to superclass part of this object
 *  @param handler reference to registration handler of this routine
 *  @param info original signal information as caught by infrastructure
 */
void	StatmController::HangupSignalHandler (StatmRunningContext *ctx, ctx_fddes_t handler, siginfo_t* info)
{
	statmErrReport (SC_STATM, SC_ERR, err_info("HANGUP detected: statm controller, PID = %d"), getpid());
	statmErrReport (SC_STATM, SC_ERR, err_info("statm controller: close statm worker connection"));
	close (m_masterClientReaderFd);
	close (m_masterClientWriterFd);
	m_masterClientReaderFd = -1;
	m_masterClientWriterFd = -1;
	if (m_masterClientReaderHandler != 0)
		RemoveDescriptor(m_masterClientReaderHandler);
	m_masterClientReaderHandler = 0;
	if (m_masterClientWriterHandler != 0)
		RemoveDescriptor(m_masterClientWriterHandler);
	m_masterClientWriterHandler = 0;
	m_commandCode = StatmAdminStopWord;
	RegisterTimer (ctx->realTime(), WaitWorkingEntity, this, ctx_info);
}

/*! @brief create header common to all protocol messages
 *
 *  function fills version, message number, UUID,
 *  entity type and PID of sending process
 *
 *  @param header message header reference
 *
 */
void	StatmController::CreateMessageHeader (StatMessageHeader* header)
{
	header->m_version.m_major = 1;
	header->m_version.m_minor = 0;
	header->m_index = ++g_messageIndex;
	uuid_generate (header->m_hash);
	header->m_error = 0;
	header->m_entity = StatmCommon::entity();
	header->m_pid = StatmCommon::pid();
}

/*! @brief prepare local client message for sending
 *
 *  prepares message which should be sent to
 *  working entity in accordance with program
 *  parameters: message type (administrative or
 *  SQL request) command code (start, stop for
 *  admin requests, fetch, execute, etc. for
 *  SQL requests. Finally it calls function
 *  PostRequest() which ensures that it will
 *  be sent as soon as possible
 *
 *  @return 0 message successfully posted
 *  @return -1 message not posted, incorrect command
 */
int	StatmController::PrepareLocalClientMessage ()
{
	StatRequest*	req = new StatRequest;
	memset (req, 0, sizeof (StatRequest));

	switch (m_commandType)
	{
	case	StatmAdminWord:
		{
			req->m_requestType = AdminRequestCode;

			StatAdminRequest*	p_StatAdminRequest = &req->StatRequest_u.m_adminRequest;
			p_StatAdminRequest->m_command = (u_int) m_commandCode;
			p_StatAdminRequest->m_parameters.m_parameters_len = 0;
			p_StatAdminRequest->m_parameters.m_parameters_val = 0;
		}
		break;
	default:
		statmErrReport (SC_STATM, SC_ERR, err_info("cannot create local client message, incorrect message type = %d"), m_commandType);
		return	-1;
	}

	int	status = PostRequest (req);
	xdr_free ((xdrproc_t)xdr_StatRequest, (char*) req);
	delete	req;
	return	status;
}

/*! @post request to server (working entity)
 *
 *  function tries to serializes message, allocates
 *  sufficient space in internal buffers and puts
 *  serialized message into this buffer. If allocation
 *  and serialization succeed it enables output
 *  processing of associated I/O channel (local UNIX
 *  domain socket, network socket or message queue)
 *  thus enabling message to be sent as soon as possible
 *
 *  @return 0 all steps (allocation, serialization and
 *  activation) succeed
 *  @return -1 some step failed, error logged
 *
 */
int	StatmController::PostRequest (StatRequest* req)
{
	CreateMessageHeader (&req->StatRequest_u.m_header);
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
			statmErrReport (SC_STATM, SC_ERR, err_info("cannot post client request, malloc() failed, errno = %d"), errno);
			return	-1;
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
		statmErrReport (SC_STATM, SC_ERR, err_info("cannot post client request, cannot encode message size, xdr_int() failed"));
		return	-1;
	}

	xdrmem_create(&xdr, (char*) (m_outputPtr + m_intSize), freeSpace - m_intSize, XDR_ENCODE);
	if (xdr_StatRequest (&xdr, req) != TRUE)
	{
		statmErrReport (SC_STATM, SC_ERR, err_info("cannot post client request, cannot encode message request, xdr_StatRequest() failed"));
		return	-1;
	}

	u_int	pos = xdr_getpos (&xdr);
	m_outputPtr += pos + m_intSize;
	EnableDescriptor (m_localClientHandler, EPOLLOUT);
	return	0;
}

/*! @brief I/O handler for XML DB client
 *
 *  function handles all I/O activities for associated I/O channel (UNIX domain,
 *  network or message queue) for ordinary (non-controller) XML DB client.
 *  It handles all incoming and outgoing requests produced in communication
 *  with XML DB server working entity.
 *
 *  Incoming messages: sufficient space is allocated in internal message buffer
 *  to accomodate incoming message(s) (one or more messages should arrive). Every
 *  incoming message is deserialized and not processed at all, since they are
 *  simple replies to previously sent start and stop requests
 *
 *  Outgoing messages: these are posted serialized messages previously handled
 *  by PostRequest(). Serialized form of these messages is sent to associated FD,
 *  allocated space is freed and it should be used to post new messages which is
 *  never the case since controllers send only one message in its life time
 *
 *  @param ctx reference to superclass part of this object (I/O multiplexer)
 *  @param flags epoll events (EPOLLIN, EPOLLOUT, ...)
 *  @param handler reference to I/O multiplexing handler (useful if it needs to be released)
 *  @param fd FD associated with this handler (UNIX domain client, network client, message queue client)
 *
 */
void	StatmController::HandleLocalClient (StatmRunningContext *ctx, uint flags, ctx_fddes_t handler, int fd)
{
	DisableTimer(m_localClientIOTimer);
	m_localClientIOTimer = 0;

	if (flags & EPOLLIN)
	{
		int	needSpace = 0;
		int	usedSpace = m_inputPtr - m_inputBuffer;
		int	freeSpace = m_inputEnd - m_inputPtr;

		if (ioctl (fd, FIONREAD, &needSpace) < 0)
		{
			statmErrReport (SC_STATM, SC_ERR, err_info("cannot receive client reply, ioctl() failed, errno = %d"), errno);
			Release();
			return;
		}

		if (needSpace == 0)
		{
			statmErrReport (SC_STATM, SC_APL, err_info("cannot receive client reply, empty message, ioctl() failed, errno = %d"), errno);
			Release();
			return;
		}

		if (freeSpace < needSpace)
		{
			needSpace += usedSpace;
			needSpace >>= 10;
			needSpace++;
			needSpace <<= 10;
			u_char*	buffer = (u_char*) malloc (needSpace);
			if (buffer == 0)
			{
				statmErrReport (SC_STATM, SC_ERR, err_info("cannot receive client reply, malloc(%d) failed, errno = %d"), needSpace, errno);
				Release();
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

		int	recvSize;
		if ((recvSize = recv (fd, m_inputPtr, freeSpace, 0)) <= 0)
		{
			if (errno == EWOULDBLOCK)
			{
				return;
			}
			statmErrReport (SC_STATM, SC_ERR, err_info("cannot receive client reply, recv() failed, errno = %d"), errno);
			Release();
			return;
		}

		usedSpace += recvSize;

		u_char*	ptr = m_inputBuffer;
		while (true)
		{
			if (usedSpace < (int) m_intSize)
			{
				break;
			}

			XDR	xdr;

			int	msgSize;
			xdrmem_create(&xdr, (char*) ptr, m_intSize, XDR_DECODE);
			if (xdr_int (&xdr, &msgSize) != TRUE)
			{
				statmErrReport (SC_STATM, SC_ERR, err_info("cannot receive client reply, cannot decode message size, xdr_int() failed"));
				Release ();
				return;
			}

			if (usedSpace < msgSize + (int) m_intSize)
			{
				break;
			}

			StatReply*	req = new StatReply;
			memset (req, 0, sizeof (StatReply));
			xdrmem_create(&xdr, (char*) (ptr + m_intSize), msgSize, XDR_DECODE);
			if (xdr_StatReply(&xdr, req) != TRUE)
			{
				statmErrReport (SC_STATM, SC_ERR, err_info("cannot receive client reply, cannot decode message body, xdr_StatReply() failed"));
				Release ();
				return;
			}

			ptr += msgSize + m_intSize;
			usedSpace -= msgSize + m_intSize;

			break;
		}
		if (usedSpace > 0)
		{
			timespec	t = ctx->realTime();
			t.tv_sec += 10;
			m_localClientIOTimer = RegisterTimer(t, LocalClientIOTimer, this, ctx_info);
		}
	}
	else	if (flags & EPOLLOUT)
	{
		size_t	size = m_outputPtr - m_outputBuffer;
		ssize_t	count = send (fd, m_outputBuffer, size, 0);
		if (count <= 0)
		{
			statmErrReport (SC_STATM, SC_ERR, err_info("cannot send client request, send() failed, errno = %d"), errno);
			Release ();
			return;
		}
		if ((size_t) count < size)
		{
			timespec	t = ctx->realTime();
			t.tv_sec += 10;
			memcpy (m_outputBuffer, m_outputBuffer + count, size - count);
			m_localClientIOTimer = RegisterTimer(t, LocalClientIOTimer, this, ctx_info);
		}
		else
			DisableDescriptor(handler, EPOLLOUT);
		m_outputPtr -= count;
	}
	else
	{
		statmErrReport (SC_STATM, SC_ERR, err_info("cannot determine controller data type"));
		Release ();
	}
}

/*! @brief timer for ordinary client I/O activity
 *
 *  every time certain I/O activity is carried out, previous time out control is switched
 *  off and new one is switched on by registering this function as time out handler which
 *  will fire it. If it fires (lazy communication) it releases I/O connections causing
 *  process to cease possibly not carrying out its job
 *
 *  @param ctx reference to superclass part of this object
 *  @param handler time out handler reference
 *  @param t time stamp of fire event
 *
 */
void	StatmController::LocalClientIOTimer (StatmRunningContext *ctx, void *handler, struct timespec t)
{
	m_localClientIOTimer = 0;
	Release ();
}

/*! @brief I/O handler for XML DB controller
 *
 *  function handles all I/O activities for both pipes connecting controlling and
 *  working entities
 *
 *  Input pipe: sufficient space is allocated in internal message buffer
 *  to accomodate incoming message(s) (one or more messages should arrive). Every
 *  incoming message is deserialized and processed appropriately. Only administrative
 *  requests are processed. Start, stop or restart requests release pipes connecting
 *  controlling and working entities causing both of them to cease.
 *
 *  Outgoing pipe: normal (EPOLLOUT) or exceptional output processing releases pipes
 *  connecting controlling and working entities causing both of them to cease.
 *
 *  @param ctx reference to superclass part of this object (I/O multiplexer)
 *  @param flags epoll events (EPOLLIN, EPOLLOUT, ...)
 *  @param handler reference to I/O multiplexing handler (useful if it needs to be released)
 *  @param fd FD associated with this handler (UNIX domain client, network client, message queue client)
 *
 */
void	StatmController::HandleMasterClient (StatmRunningContext *ctx, uint flags, ctx_fddes_t handler, int fd)
{
	DisableTimer(m_masterClientIOTimer);
	m_masterClientIOTimer = 0;

	if (flags & EPOLLIN)
	{
		int	needSpace = 0;
		int	usedSpace = m_inputPtr - m_inputBuffer;
		int	freeSpace = m_inputEnd - m_inputPtr;

		if (ioctl (fd, FIONREAD, &needSpace) < 0)
		{
			statmErrReport (SC_STATM, SC_ERR, err_info("cannot receive master client reply, ioctl() failed, errno = %d"), errno);
			Release();
			RegisterTimer (ctx->realTime(), WaitWorkingEntity, this, ctx_info);
			return;
		}

		if (needSpace == 0)
		{
			statmErrReport (SC_STATM, SC_ERR, err_info("cannot receive master client reply, empty message, ioctl() failed, errno = %d"), errno);
			Release();
			RegisterTimer (ctx->realTime(), WaitWorkingEntity, this, ctx_info);
			return;
		}

		if (freeSpace < needSpace)
		{
			needSpace += usedSpace;
			needSpace >>= 10;
			needSpace++;
			needSpace <<= 10;
			u_char*	buffer = (u_char*) malloc (needSpace);
			if (buffer == 0)
			{
				statmErrReport (SC_STATM, SC_ERR, err_info("cannot receive master client reply, malloc(%d) failed, errno = %d"), needSpace, errno);
				Release();
				RegisterTimer (ctx->realTime(), WaitWorkingEntity, this, ctx_info);
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

		int	recvSize;
		if ((recvSize = read (fd, m_inputPtr, freeSpace)) <= 0)
		{
			if (errno == EWOULDBLOCK)
			{
				return;
			}
			statmErrReport (SC_STATM, SC_ERR, err_info("cannot receive master client reply, recv() failed, errno = %d"), errno);
			Release();
			RegisterTimer (ctx->realTime(), WaitWorkingEntity, this, ctx_info);
			return;
		}

		usedSpace += recvSize;

		u_char*	ptr = m_inputBuffer;
		while (true)
		{
			if (usedSpace < (int) m_intSize)
			{
				break;
			}

			XDR	xdr;

			int	msgSize;
			xdrmem_create(&xdr, (char*) ptr, m_intSize, XDR_DECODE);
			if (xdr_int (&xdr, &msgSize) != TRUE)
			{
				statmErrReport (SC_STATM, SC_ERR, err_info("cannot receive master client reply, cannot decode message size, xdr_int() failed"));
				return;
			}

			if (usedSpace < msgSize + (int) m_intSize)
			{
				break;
			}

			StatRequest*	req = new StatRequest;
			memset (req, 0, sizeof (StatRequest));
			xdrmem_create(&xdr, (char*) (ptr + m_intSize), msgSize, XDR_DECODE);
			if (xdr_StatRequest(&xdr, req) != TRUE)
			{
				statmErrReport (SC_STATM, SC_ERR, err_info("cannot receive master client reply, cannot decode message body, xdr_StatReply() failed"));
				return;
			}

			ptr += msgSize + m_intSize;
			usedSpace -= msgSize + m_intSize;

			if (req->m_requestType != AdminRequestCode)
			{
				statmErrReport (SC_STATM, SC_ERR, err_info("incorrect master client reply code: %d"), req->m_requestType);
				return;
			}

			switch (m_commandCode = (StatmWordCode) req->StatRequest_u.m_adminRequest.m_command)
			{
			case	StatmAdminStartWord:
			case	StatmAdminRestartWord:
			case	StatmAdminStopWord:
				{
					Release();
					RegisterTimer (ctx->realTime(), WaitWorkingEntity, this, ctx_info);
					return;
				}
				break;
			case	StatmAdminStatusWord:
				{
				}
				break;
			default:
				{
					statmErrReport (SC_STATM, SC_ERR, err_info("incorrect master client reply command code: %d"), m_commandCode);
				}
				break;
			}
		}
		if (usedSpace > 0)
		{
			timespec	t = ctx->realTime();
			t.tv_sec += 10;
			m_masterClientIOTimer = RegisterTimer(t, MasterClientIOTimer, this, ctx_info);
		}
	}
	else	if (flags & EPOLLOUT)
	{
		statmErrReport (SC_STATM, SC_ERR, err_info("incorrect master client I/O type: %d"), flags);
		Release ();
		RegisterTimer (ctx->realTime(), WaitWorkingEntity, this, ctx_info);
	}
	else
	{
		statmErrReport (SC_STATM, SC_ERR, err_info("unknown master client I/O type: %d"), flags);
		Release ();
		RegisterTimer (ctx->realTime(), WaitWorkingEntity, this, ctx_info);
	}
}

/*! @brief timer for controlling entity I/O activity
 *
 *  every time certain I/O activity is carried out, previous time out control is switched
 *  off and new one is switched on for all incompletely received requests. This is achieved
 *  by registering this function as time out handler which will fire it. If it fires (lazy
 *  communication) it releases I/O connections causing controller and working entities to
 *  cease
 *
 *  @param ctx reference to superclass part of this object
 *  @param handler time out handler reference
 *  @param t time stamp of fire event
 *
 */
void	StatmController::MasterClientIOTimer (StatmRunningContext *ctx, void *handler, struct timespec t)
{
	m_masterClientIOTimer = 0;
	Release ();
	RegisterTimer (ctx->realTime(), WaitWorkingEntity, this, ctx_info);
}

/*! @brief wait working entity to cease
 *
 *  every time controlling entity releases pipes connecting it to working entity it
 *  switches on timer (by registering this function as time out handler) which will
 *  indefinitely wait for working entity to cease. It will repeatedly wait it as
 *  many times as needed. When awaited it will fetch working entity exit status and
 *  report it into log queue. After that it will examine administrative code obtained
 *  from input pipe. In case of stop request, controller will cease otherwise it will
 *  repeat starting processing again by registering AccessSyncPath() time out function.
 *
 *  @param ctx reference to superclass part of this object
 *  @param handler time out handler reference
 *  @param t time stamp of beginning of wait event
 *
 */
void	StatmController::WaitWorkingEntity (StatmRunningContext *ctx, void *handler, struct timespec t)
{
	int	status;
	pid_t	pid;

	errno = 0;
	switch (pid = waitpid ((pid_t) -1, &status, WNOHANG))
	{
	case	0:
	case	(pid_t) -1:
		if (errno == ECHILD)
			break;
		t.tv_sec += 1;
		RegisterTimer (t, WaitWorkingEntity, this, ctx_info);
		return;
	default:
		break;
	}

	if (pid == (pid_t) -1)
		;
	else	if (WIFEXITED (status))
	{
		statmErrReport (SC_STATM, SC_ERR, err_info("statm worker terminated, exit status = %d, PID = %d"), WEXITSTATUS (status), pid);
	}
	else	if (WIFSIGNALED (status))
	{
		statmErrReport (SC_STATM, SC_ERR, err_info("statm worker terminated, killed by signal = %d, PID = %d"), WTERMSIG (status), pid);
	}
	else
	{
		statmErrReport (SC_STATM, SC_ERR, err_info("statm worker terminated, status = %d, PID = %d"), status, pid);
	}

	if (m_commandCode == StatmAdminStopWord)
	{
		Release();
		return;
	}
	RegisterTimer (t, AccessSyncPath, this, ctx_info);
}

} /* namespace statm_daemon */
