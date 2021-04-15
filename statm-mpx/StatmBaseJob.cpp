/*
 * StatmBaseJob.cpp
 *
 *  Created on: 19. feb. 2016
 *      Author: miran
 */

#include "StatmBaseJob.h"

namespace statm_daemon
{

unsigned int	StatmBaseJob::g_jobCounter = 0;	//!< application wide unique job number

/*! @brief construct base job
 *
 *  - increments unique job number
 *  - remembers job execution timer (in nanoseconds)
 *
 */
StatmBaseJob::StatmBaseJob (u_long execTimer)
{
	m_jobCounter = ++g_jobCounter;
	m_execTimer = execTimer;
}

StatmBaseJob::~StatmBaseJob ()
{
	// TODO Auto-generated destructor stub
}

} /* namespace statm_daemon */
