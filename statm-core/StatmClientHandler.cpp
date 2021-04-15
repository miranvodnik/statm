/*
 * StatmClientHandler.cpp
 *
 *  Created on: 11. nov. 2016
 *      Author: miran
 */

#include "StatmClientHandler.h"

namespace statm_daemon
{

/*! @brief starting routine for client driver multiplexer
 *
 *  implementation of starting routine required by StatmRunningContext
 *  interface. It does nothing
 *
 *  @see StatmRunningContext
 *
 *  @param ctx refinement of this object referring to its superclass object
 *
 */
void	StatmClientHandler::HandlerStart (StatmRunningContext *ctx) {}

/*! @brief ending routine for client driver multiplexer
 *
 *  implementation of ending routine required by StatmRunningContext
 *  interface. It does nothing
 *
 *  @see StatmRunningContext
 *
 *  @param ctx refinement of this object referring to its superclass object
 *
 */
void	StatmClientHandler::HandlerStop (StatmRunningContext *ctx) {}

/*! @brief time hook routine for client driver multiplexer
 *
 *  implementation of time hook routine required by StatmRunningContext
 *  interface. It does nothing
 *
 *  @see StatmRunningContext
 *
 *  @param ctx refinement of this object referring to its superclass object
 *  @param oldTime time stamp before time change
 *  @param new Time time stamp after time change
 *  @param timeDiff difference between time stamps expressed in nanoseconds
 *
 */
void	StatmClientHandler::HandlerTimeHook (StatmRunningContext *ctx, timespec oldTime, timespec newTime, long long int timeDiff) {}

} /* namespace statm_daemon */
