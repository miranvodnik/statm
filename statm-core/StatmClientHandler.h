/*
 * StatmClientHandler.h
 *
 *  Created on: 11. nov. 2016
 *      Author: miran
 */

#pragma once

#include "StatmRunningContext.h"

namespace statm_daemon
{

/*! @brief client driver multiplexer
 *
 *  This class implements no new functionality. All of its functionality
 *  is implemented by its superclass StatmRunningContext which implements
 *  general mechanisms for I/O multiplexing. This class is merely a 'bag'
 *  holding theoretically unlimited number of clients which implement
 *  their own I/O logic in such a way that it can be multiplexed with
 *  other objects of this kind. In general: these objects are any open
 *  file descriptors obtained by open(), socket(), mq_open() and other
 *  system calls which return file descriptors.
 *
 *  @see StatmRunningContext
 *
 */
class StatmClientHandler : public StatmRunningContext
{
public:
	/*! @brief StatmClientHandler constructor
	 *
	 *  it provides start, stop and time hook routines to its superclass
	 *
	 */
	StatmClientHandler () : StatmRunningContext (HandlerStart, HandlerStop, HandlerTimeHook, this, "client-handler") {}
	/*! @brief StatmClientHandler destructor
	 *
	 *  enables 'chaining' of destructor calls
	 *
	 */
	virtual ~StatmClientHandler () {}

	ctx_starter (HandlerStart, StatmClientHandler)	//!< declaration of start routine
	ctx_finisher (HandlerStop, StatmClientHandler)	//!< declaration of stop routine
	ctx_timehook (HandlerTimeHook, StatmClientHandler)	//!< declaration of time hook routine
};

} /* namespace statm_daemon */
