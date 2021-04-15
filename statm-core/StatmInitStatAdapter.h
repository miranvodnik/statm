/*
 * StatmInitStatAdapter.h
 *
 *  Created on: Feb 6, 2019
 *      Author: miran
 */

#pragma once

#include <time.h>
#include <sys/inotify.h>
#include <StatmBaseJob.h>
#include <string>
using namespace std;

namespace statm_daemon
{

class StatmInitStatAdapter: public StatmBaseJob
{
public:
	StatmInitStatAdapter();
	virtual ~StatmInitStatAdapter();
	virtual int Execute (StatmWorkingThread* wt);
	virtual int Cleanup ();
	virtual void Report ();
	virtual int	Invoke (StatmRunningContext* ctx) { return Execute ((StatmWorkingThread*)ctx); }
};

} /* namespace statm_daemon */
