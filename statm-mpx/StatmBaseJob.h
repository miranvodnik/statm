/*
 * StatmBaseJob.h
 *
 *  Created on: 19. feb. 2016
 *      Author: miran
 */

#ifndef STATMBASEJOB_H_
#define STATMBASEJOB_H_

#include "StatmRunningContext.h"
#include <time.h>
#include <string>
#include <functional>
using namespace std;

namespace statm_daemon
{

class	StatmWorkingThread;

/*! @brief base job superclass for all jobs
 *
 *  every subclass must implement:
 *  - Execute() job execution routine
 *  - Cleanup() job cleanup routine (thread cancellation routine)
 *  - Report() job report routine
 *
 */
class StatmBaseJob
{
public:
	StatmBaseJob (u_long execTimer);
	virtual ~StatmBaseJob ();
	virtual int Execute (StatmWorkingThread* wt) = 0;
	virtual int Cleanup () = 0;
	virtual void Report () = 0;

	inline int returnStatus () { return m_returnStatus; }	//!< get job return status
	inline unsigned int jobCounter () { return m_jobCounter; }	//!< get job number
	inline int errorCode () { return m_errorCode; }	//!< get job error code
	inline string errorMessage () { return m_errorMessage; }	//!< get job error message
	inline u_long	execTimer() { return m_execTimer; }	//!< get job execution timer
private:
	static unsigned int	g_jobCounter;	//!< application wide job number
	unsigned int	m_jobCounter;	//!< job number
protected:
	u_long	m_execTimer;	//!< job execution timer
	int m_returnStatus;	//!< job return status
	int	m_errorCode;	//!< job error code
	string	m_errorMessage;	//!< job error message
public:
	static const u_long	Infinite = (u_long) -1;	//!< infinite job execution time
	static const u_long	NsecPerSec = (u_long) (1000 * 1000 * 1000);	//!< number of nanoseconds per second
};

} /* namespace statm_daemon */

#endif /* STATMBASEJOB_H_ */
