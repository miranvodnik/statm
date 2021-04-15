/*
 * StatmClientDriver.cpp
 *
 *  Created on: 11. nov. 2016
 *      Author: miran
 */

#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#include "StatmClientDriver.h"
#include <sstream>
using namespace std;

namespace statm_daemon
{

bool	StatmClientDriver::g_started = false;	//!< status of client driver threads
StatmClientDriver* StatmClientDriver::g_currentClientDriver = 0;	//!< current client driver thread

/*! @brief constructor of client driver instance
 *
 *  it does nothing but creates client driver multiplexer
 *  which is capable of simultaneously drive any number of
 *  client handlers
 */
StatmClientDriver::StatmClientDriver ()
{
#if defined(DEBUG)
	if (g_debug) statmErrReport (SC_STATM, SC_ERR, "CREATE StatmClientDriver %d", this);
#endif

	m_clientHandler = new StatmClientHandler ();
}

/*! @brief destructor of client driver instance
 *
 *  it deletes all client instances (local, network and message queue)
 *  collected so far together with client driver multiplexer
 */
StatmClientDriver::~StatmClientDriver ()
{
#if defined(DEBUG)
	if (g_debug) statmErrReport (SC_STATM, SC_ERR, "DELETE StatmClientDriver %d", this);
#endif

	for (lcset::iterator it = m_lcset.begin(); it != m_lcset.end(); ++it)
	{
		delete	*it;
	}
	m_lcset.clear();

	delete	m_clientHandler;
	m_clientHandler = 0;
}

/*! @brief start client drivers
 *
 *  start small number of client driver threads. This number
 *  is obviously equal to the number of CPU cores or just
 *  slightly grater if CPU has small number of cores. Every thread
 *  is associated with particular core which will execute it. First thread
 *  is declared to become current driver thread
 *
 */
void	StatmClientDriver::StartClientDrivers ()
{
	if (g_started)
		return;
	g_started = true;

	g_currentClientDriver = new StatmClientDriver ();
	g_currentClientDriver->Start(0);
}

/*! @brief stop client drivers
 *
 *  stop all client driver threads one by one. Every thread is canceled
 *  and joined before proceeding to next one. Currently executed driver
 *  handler (if any) is canceled together with driver thread
 *
 */
void StatmClientDriver::StopClientDrivers ()
{
	delete	g_currentClientDriver;
	g_currentClientDriver = 0;
}

/*! @brief select current driver thread
 *
 *  Function returns current client driver thread and selects new one
 *  in round-robin fashion by iterating client driver thread list. After
 *  reaching end of list it starts from the beginning. Every new client
 *  driver handler is associated with current client driver thread returned
 *  by this function call. Following this principle we can assume that
 *  all client handlers are equally distributed between client driver threads
 *  although there are many scenarios that this is not always true, especially
 *  when there are clients with different life living lengths
 *
 *  @return current client driver thread
 */
StatmClientDriver*	StatmClientDriver::currentClientDriver ()
{
	return	g_currentClientDriver;
}

/*! @brief Run() method of client driver thread
 *
 *  This function is thread running function. It installs thread cleaning
 *  function ThreadCleanup() and runs client handler multiplexer main loop
 *
 *  @return status status of client handler multiplexer. This status is never
 *  examined because client driver threads never terminate normally. They are
 *  always canceled.
 */
int StatmClientDriver::Run (void)
{
	int	status;
	pthread_cleanup_push (ThreadCleanup, this);
	statmErrReport (SC_STATM, SC_ERR, "Connection handling thread started, TID = %d", getTid ());
	status = m_clientHandler->MainLoop();
	pthread_cleanup_pop(0);
	return	status;
}

/*! @brief client driver thread cancellation function
 *
 *  this function is invoked by cancellation mechanism when main working thread
 *  terminates execution of client driver threads. It does nothing but reports
 *  the cancellation fact
 */
void StatmClientDriver::ThreadCleanup ()
{
	statmErrReport (SC_STATM, SC_ERR, "Connection handling thread canceled, TID = %d", getTid ());
}

/*! @brief client driver thread initialization function
 *
 *  function does nothing since there is nothing special to be initialized. Since PID of
 *  thread is not known at initialization time, starting message is reported in thread
 *  Run() function
 *
 *  @return 0 always
 *
 */
int StatmClientDriver::InitInstance (void)
{
	return	0;
}

/*! @brief client driver thread termination function
 *
 *  function reports termination message and stops client driver multiplexer
 *
 *  @return 0 always
 *
 */
int StatmClientDriver::ExitInstance (void)
{
	statmErrReport (SC_STATM, SC_ERR, "Connection handling thread terminated, TID = %d", getTid ());
	m_clientHandler->Stop();
	return	0;
}

/*! @brief handle administrative request
 *
 *  this function handles stop request. In that case it delivers SIGHUP
 *  interrupt to self process. This interrupt is intercepted by one of
 *  StatmRunningContext multiplexers active in this process (possibly to
 *  client driver multiplexer owned by this thread) and synchronized
 *  with all entities interested to handle it.
 *
 *  @see StatmWorker
 *
 */
void	StatmClientDriver::HandleAdminRequest (StatmLocalClient* cln, StatRequest* req)
{
	if ((req->m_requestType != AdminRequestCode) || (req->StatRequest_u.m_adminRequest.m_command != StatmAdminStopWord))
		return;
	if (kill (getpid(), SIGHUP) < 0)
		statmErrReport (SC_STATM, SC_ERR, err_info ("cannot seng SIGHUP, errno = %d"), errno);
}

/*! @brief release local client handler
 *
 *  exclude local client handler, connected locally through UNIX domain
 *  socket, from internal list
 *
 *  @param localClient local client driver reference
 *
 */
void	StatmClientDriver::ReleaseLocalClient (StatmLocalClient* localClient)
{
	lcset::iterator	it = m_lcset.find(localClient);
	if (it != m_lcset.end())
		m_lcset.erase(it);
}

/*! @brief send local client handler activation request
 *
 *  this function is called immediately after local client has been successfully
 *  connected through UNIX domain socket. Since connection has been made in
 *  special thread which does nothing but establishes connections (either local
 *  using UNIX domain sockets, local using message queues or remote using network
 *  sockets), info about new connection must be transfered to current client handler
 *  thread. That is exactly what it does this function: it takes local client FD,
 *  makes message from it and sends this message into internal message queue of
 *  client driver multiplexer.
 *
 *  @return status success of message sending
 *
 *  @see MQSend
 *  @see StatmLocalClientActivationMessage
 *
 */
int	StatmClientDriver::SendLocalClientHandlerActivation (int localClientFd)
{
	int	status;
	StatmLocalClientActivationMessage*	msg = new StatmLocalClientActivationMessage (localClientFd, this);
	struct { void* msg; }	m = { (void*) msg };
	if ((status = m_clientHandler->MQSend ((char*) &m, sizeof m)) < 0)
	{
		statmErrReport (SC_STATM, SC_ERR, err_info ("cannot save internal message"));
		delete	msg;
	}
	return	status;
}

/*! @brief activate local client handler
 *
 *  Once activation request has been read by client multiplexer, new instance
 *  of StatmLocalClient is created and added to internal list of this object. Newly
 *  created object registers I/O handler in client multiplexer
 *
 *  @return 0 always
 *
 *  @see StatmLocalClient
 */
int	StatmClientDriver::ActivateLocalClientHandler (int localClientFd)
{
	StatmLocalClient*	p_StatmLocalClient = new StatmLocalClient (m_clientHandler, localClientFd, this);
	m_lcset.insert (p_StatmLocalClient);
	return	0;
}

} /* namespace statm_daemon */
