/*
 * StatmWorkingClient.cpp
 *
 *  Created on: 3. nov. 2015
 *      Author: miran
 */

#include "StatmWorkingClient.h"
#include <iostream>
using namespace std;

namespace statm_daemon
{

/*! @brief working client constructor
 *
 *  it remembers thread references in which it is running
 *
 *  @param mainThread thread reference
 *
 */
StatmWorkingClient::StatmWorkingClient(StatmRunnable* mainThread)
{
#if defined(DEBUG)
	if (g_debug) statmErrReport (SC_STATM, SC_ERR, "CREATE StatmWorkingClient %ld", this);
#endif
	m_mainThread = mainThread;
}

/*! @brief working client destructor
 *
 *  it does nothin special
 *
 */
StatmWorkingClient::~StatmWorkingClient()
{
#if defined(DEBUG)
	if (g_debug) statmErrReport (SC_STATM, SC_ERR, "DELETE StatmWorkingClient %ld", this);
#endif
}

} /* namespace statm_daemon */
