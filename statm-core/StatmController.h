/*
 * StatmController.h
 *
 *  Created on: 28. okt. 2015
 *      Author: miran
 */

#pragma once

#include "StatmRunningContext.h"
#include "StatmCommon.h"
#include "StatmMessages.h"

namespace statm_daemon
{

/*! @brief XML DB entity class
 *
 *  this class represents XML DB entity being it ordinary
 *  XML DB client or XML DB controller. Of all concurrently started
 *  entities, at most one will become controlling entity, all
 *  others will become ordinary clients. Controller will become
 *  an entity which will successfully invoke start administrative
 *  request
 *
 */
class StatmController: public StatmRunningContext
{
public:
	StatmController (int n, char**p);
	virtual ~StatmController ();

private:
	ctx_starter (StartController, StatmController)
	ctx_finisher (StopController, StatmController)
	ctx_timehook (TimeHookController, StatmController)

	timer_handler (AccessSyncPath, StatmController)
	timer_handler (DetermineStatmEntityRole, StatmController)
	timer_handler (DriveStatClientEntity, StatmController)
	timer_handler (DriveStatControllerEntity, StatmController)
	timer_handler (WaitWorkingEntity, StatmController)

	fd_handler (HandleMasterClient, StatmController)
	fd_handler (HandleLocalClient, StatmController)

	sig_handler (SigcldSignalHandler, StatmController)
	sig_handler (HangupSignalHandler, StatmController)

	timer_handler (LocalClientIOTimer, StatmController)
	timer_handler (MasterClientIOTimer, StatmController)

	int	LockControllingPath (void);
	int	UnlockControllingPath (void);
	int	CreateClientEntity (void);
	int	CreateControllerEntity (void);
	int	CreateLocalListener();
	int	CreateLocalClient();

	int	PrepareLocalClientMessage ();
	int	PostRequest (StatRequest* req);
	void	CreateMessageHeader (StatMessageHeader* header);

	void	BecomeDaemon ();
	int	ForkWorker ();

	void	Release ();
private:
	static	const	string	g_localSocketPath;	//!< UNIX domain listening socket path
	static	const	int	g_localListener;	//!< redirected UNIX domain listening FD (3)
	static	int	g_messageIndex;	//!< protocol message number
private:
	int	m_argc;	//!< number of arguments provided by main() function
	char**	m_argv;	//!< program arguments provided by main() function

	StatmWordCode	m_commandType;	//!< administrative command type
	StatmWordCode	m_commandCode;	//!< administrative command code: stop, start, restart
	int	m_sync;		//!< synchronization path semaphore

	int	m_localListenFd;	//!< local socket listening fd - working entity controller
	int	m_localClientFd;	//!< local socket client fd - controlling client
	int	m_masterClientReaderFd;	//!< local socket client fd - controlling entity master client reader pipe
	int	m_masterClientWriterFd;	//!< controlling entity master client writer pipe

	pid_t	m_worker;	//!< working entity PID

	ctx_fddes_t	m_localClientHandler;	//!< UNIX domain client FD I/O handler
	ctx_fddes_t	m_masterClientReaderHandler;	//!< input pipe I/O handler
	ctx_fddes_t	m_masterClientWriterHandler;	//!< output pipe I/O handler
	ctx_timer_t	m_localClientIOTimer;	//!< UNIX domain client FD I/O timer
	ctx_timer_t	m_masterClientIOTimer;	//!< master client pipe I/O timer

	ctx_sig_t	m_sigcldSignalHandler;	//!< SIGCLD signal handler
	ctx_sig_t	m_hangupSignalHandler;	//!< SIGHUP signal handler

	u_char*	m_inputBuffer;	//!< internal input buffer for either ordinary client or controlling entity
	u_char*	m_inputPtr;	//!< internal input buffer read head
	u_char*	m_inputEnd;	//!< internal input buffer far end point

	u_char*	m_outputBuffer;	//!< internal output buffer for either ordinary client or controlling entity
	u_char*	m_outputPtr;	//!< internal output buffer write head
	u_char*	m_outputEnd;	//!< internal output buffer far end point

	int	m_intSize;	//!< size of XDR integer representation
	bool	m_isController;	//!< indicates controlling entity: false - ordinary client, true - controller
	bool	m_deamonized;	//!< controller daemonized: true - does not need to be daemonized again when restarting
};

} /* namespace statm_daemon */
