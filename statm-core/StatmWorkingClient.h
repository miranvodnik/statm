/*
 * StatmWorkingClient.h
 *
 *  Created on: 3. nov. 2015
 *      Author: miran
 */

#pragma once

#include "StatmRunnable.h"

namespace statm_daemon
{

/*! @brief working client superclass
 *
 *  superclass for StatmLocalClient, StatmNetworkClient and StatmMqClient
 *  classes. It requires implementation of first_half_callback () and
 *  second_half_callback () functions. It stores reference to client
 *  driver thread, in which clients run
 *
 */
class StatmWorkingClient
{
public:
	StatmWorkingClient(StatmRunnable* mainThread);
	virtual ~StatmWorkingClient();
	StatmRunnable*	mainThread() { return m_mainThread; }
private:
	StatmRunnable*	m_mainThread;	//!< thread reference
};

} /* namespace statm_daemon */
