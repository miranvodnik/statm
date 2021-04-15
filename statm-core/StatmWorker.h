/*
 * StatmWorker.h
 *
 *  Created on: 30. okt. 2015
 *      Author: miran
 */

#pragma once

#include "StatmRunningContext.h"
#include "StatmWorkingThread.h"
#include "StatmClientDriver.h"
#include "StatmLocalClient.h"
#include "StatmCommon.h"
#include "StatmMessages.h"
#include "StatmMessageQueue.h"
#include "StatmHamThread.h"

#include <set>
#include <map>
#include <deque>
#include <list>

using namespace std;

namespace statm_daemon
{

/*! @brief working entity
 *
 *  this class represents working entity I/O multiplexer and runs
 *  in main thread of working entity process. It is XML DB server
 *  which enables virtually unlimited number of client to connect
 *  to it simultaneously
 *
 */
class StatmWorker: public StatmRunningContext
{
private:
	StatmWorker (int);
public:
	StatmWorker (int n, char* p[]);
	virtual ~StatmWorker();

	void	HandleAdminRequest (StatmLocalClient* cln, StatRequest* req);
	inline void	PostAdminRequest (StatRequest* req) { HandleAdminRequest (0, req); }

private:
	// starting routine of cdcp worker proxy
	ctx_starter (StartWorker, StatmWorker)
	ctx_finisher (StopWorker, StatmWorker)
	ctx_timehook (TimeHookWorker, StatmWorker)

	fd_handler (HandleMainLocalClient, StatmWorker)
	fd_handler (HandleMainLocalClientReader, StatmWorker)
	fd_handler (HandleMainLocalClientWriter, StatmWorker)
	fd_handler (HandleLocalListener, StatmWorker)

	timer_handler (RestartStatisticalAdapter, StatmWorker)

	sig_handler (HangupSignalHandler, StatmWorker)
	sig_handler (AbortSignalHandler, StatmWorker)

	void	Release ();
private:
	static const int	g_localListener;	//!< UNIX domain listener fd (3)
	static const int	g_localClientReader;	//!< input controller pipe fd (6)
	static const int	g_localClientWriter;	//!< output controller pipe fd (7)

	static	StatmWorker*	g_genWorker;	//!< helper instance to make runtime initialization
	static	StatRequest	g_stopRequest;	//!< stop request message reference made by runtime initialization

	int	m_intSize;	//!< size of XDR integer serialization
	u_char*	m_outputBuffer;	//!< output buffer for output controller pipe
	u_char*	m_outputPtr;	//!< write head of output buffer
	u_char*	m_outputEnd;	//!< par end point of output buffer

	ctx_fddes_t	m_localClientReaderHandler;	//!< input controller pipe handler
	ctx_fddes_t	m_localClientWriterHandler;	//!< output controller pipe handler

	ctx_fddes_t	m_localListenerHandler;	//!< UNIX domain listener handler

	ctx_timer_t m_statTimer;	//!< timer to periodically reload statistical DB data

	ctx_sig_t	m_hangupSignalHandler;	//!< HANGUP handler
	ctx_sig_t	m_abortSignalHandler;	//!< ABORT handler

	#if defined (STATM_HAM)
	StatmHamThread*	m_hamThread;
#endif
};

} /* namespace statm_daemon */
