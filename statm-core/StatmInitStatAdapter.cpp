/*
 * StatmInitStatAdapter.cpp
 *
 *  Created on: Feb 6, 2019
 *      Author: miran
 */

#include "StatmInitStatAdapter.h"
#include <StatmStatDriver.h>

namespace statm_daemon
{

StatmInitStatAdapter::StatmInitStatAdapter() : StatmBaseJob (StatmBaseJob::Infinite)
{
#if defined(DEBUG)
	if (g_debug) statmErrReport (SC_STATM, SC_ERR, "CREATE StatmInitStatAdapter");
#endif
}

StatmInitStatAdapter::~StatmInitStatAdapter()
{
#if defined(DEBUG)
	if (g_debug) statmErrReport (SC_STATM, SC_ERR, "DELETE StatmInitStatAdapter");
#endif
}

int StatmInitStatAdapter::Execute (StatmWorkingThread* wt)
{
	StatmStatDriver::LoadStatFiles ();
}

int StatmInitStatAdapter::Cleanup ()
{
	delete	this;
	return	0;
}

void StatmInitStatAdapter::Report ()
{
	statmErrReport (SC_STATM, SC_ERR, err_info ("job - initialize statistical adapter"));
}

} /* namespace statm_daemon */
