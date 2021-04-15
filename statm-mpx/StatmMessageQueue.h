/*
 * StatmMessageQueue.h
 *
 *  Created on: 3. nov. 2015
 *      Author: miran
 */

#pragma once

#include "StatmBaseJob.h"
#include <string>
using namespace std;

namespace statm_daemon
{

typedef	void*	msg_addr;

/*! @brief message queue object
 *
 *  this object contains identifier of System V message queue
 *  governed by it. It provides two basic operations on that
 *  queue: one can put and get job references into or from the
 *  queue. All job references can also be removed from message
 *  queue using single function call. Queue should be initialized
 *  using Initialize(), before job references can be put or get
 *  from underlying message queue. This should be done early at
 *  the process startup.
 *
 *  All public functions are static, because we cannot directly
 *  access this object since it has no public access and is thus
 *  accessed indirectly from these functions.
 *
 */
class StatmMessageQueue
{
private:
	StatmMessageQueue (key_t key);
public:
	virtual ~StatmMessageQueue ();
public:
	/*! @brief one shot creation and initialization of this object
	 *
	 */
	inline static void	Initialize () { ((StatmMessageQueue*)0)->_Initialize(); }
	/*! @brief put job reference into message queue
	 *
	 *  @param job job reference to be saved in message queue
	 *
	 *  @return true job reference successfully saved
	 *  @return false job reference not saved
	 */
	inline static bool	Put (StatmBaseJob* job) { return g_intQueue->_Put (job); }
	/*! @brief retrieve job reference from message queue
	 *
	 *  @return 0 job not retrieved
	 *  @return other job reference
	 *
	 */
	inline static StatmBaseJob*	GetInt () { return g_intQueue->_Get (); }
private:
	void	_Initialize ();
	bool	_Put (StatmBaseJob* job);
	StatmBaseJob*	_Get ();
	void	_Clear (bool destructive);
private:
	static bool	g_initialized;	//!< indicates one shot initialization
	static StatmMessageQueue*	g_intQueue;	//!< internal message queue singleton
	long	m_mtype;	//!< message types for messages holding job references
	int	m_reqQue;	//!< System V message queue ID
};

} /* namespace statm_daemon */
