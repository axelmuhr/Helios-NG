/*
 * C++ interface to Unix wait system call
 */

#ifndef sys_wait_h

#define wait WaitStatus

#include "//usr/include/sys/wait.h"

#undef wait

/* just in case standard header didn't */
#ifndef sys_wait_h
#define sys_wait_h
#endif

extern int wait(int*);
extern int wait3(WaitStatus*, int options, struct rusage*);

#endif
