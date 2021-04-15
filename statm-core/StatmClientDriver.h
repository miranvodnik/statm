/*
 * StatmClientDriver.h
 *
 *  Created on: 11. nov. 2016
 *      Author: miran
 */

#pragma once

#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>
#include "StatmRunnable.h"
#include "StatmCommon.h"
#include "StatmClientHandler.h"
#include "StatmLocalClient.h"

#include <set>
#include <map>
#include <deque>
#include <list>

using namespace std;

namespace statm_daemon
{

/*! @brief client driver thread
 *
 *  this object implements StatmRunnable interface and is thus capable
 *  of running in its own working thread. Its main purpose is to handle
 *  theoretically unlimited number of communication channels between
 *  XML database server and its clients regardless if they are local or
 *  remote and regardless of their connection mode (UNIX domain, network
 *  or message queue). All these channels are multiplexed and treated
 *  with equal priority by this object
 *
 */
class StatmClientDriver : public StatmRunnable
{
private:
	typedef set < StatmLocalClient* >	lcset;	//!< type representing set of local clients connected through UNIX domain socket

public:
	StatmClientDriver ();
	virtual ~StatmClientDriver ();

	static void StartClientDrivers ();
	static void StopClientDrivers ();
	static StatmClientDriver*	currentClientDriver ();

private:
	virtual int Run (void);
	virtual int InitInstance (void);
	virtual int ExitInstance (void);

public:
	int	SendLocalClientHandlerActivation (int localClientFd);

	int	ActivateLocalClientHandler (int localClientFd);

	void	HandleAdminRequest (StatmLocalClient* cln, StatRequest* req);
	inline void	PostAdminRequest (StatRequest* req) { HandleAdminRequest (0, req); }

	void	ReleaseLocalClient (StatmLocalClient* localClient);

private:
	inline static void ThreadCleanup (void* data) { ((StatmClientDriver*)data)->ThreadCleanup(); }
	void ThreadCleanup ();
private:
	static bool	g_started;	//!< indicator set to true when client driver set is populated
	static StatmClientDriver*	g_currentClientDriver;	//!< current client driver
	StatmClientHandler*	m_clientHandler;	//!< client driver multiplexer

	lcset	m_lcset;	//!< set of local clients connected through UNIX domain socket
};

} /* namespace statm_daemon */
