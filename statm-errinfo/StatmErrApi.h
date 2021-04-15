/*
 * StatmErrApi.h
 *
 *  Created on: 7. sep. 2016
 *      Author: miran
 */

#pragma once

#include "StatmErrConst.h"	// error info constants

#if defined(__cplusplus)
extern	"C"
{
#endif

void	statmErrReport (int module, int severity, const char* format, ...);

#if defined(__cplusplus)
}
#endif
