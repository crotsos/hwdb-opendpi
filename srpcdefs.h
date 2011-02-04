/*
 * srpcdefs.h - global #defines used in the simple RPC system implementation
 *
 * change these to tune the behavior of the system
 */

#ifndef _SRPCDEFS_H_DEFINED_
#define _SRPCDEFS_H_DEFINED_

#include "logdefs.h"

/*
 * the following definitions control the exponential backoff retry
 * mechanism used in the protocol - these may also be changed using
 * -D<symbol>=value in CFLAGS in the Makefile
 */
#define ATTEMPTS 7   /* number of attempts before setting state to TIMEDOUT */
#define TICKS 2      /* initial number of 20ms ticks before first retry
			number of ticks is doubled for each successive retry */

/*
 * the following specifies the max payload size before fragmentation kicks
 * in - may again be changed using -DFR_SIZE=value within CFLAGS
 */
#define FR_SIZE 1024

#endif /* _SRPCDEFS_H_DEFINED_ */
