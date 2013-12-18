/*
 * C++ interface to Unix process timing system calls
 */

#ifndef sys_times_h

#include <sys/types.h>
#include "//usr/include/sys/times.h"

/* just in case standard header didn't */
#ifndef sys_times_h
#define sys_times_h
#endif

extern int times(struct tms*);

#endif
