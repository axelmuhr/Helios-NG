/*
 * C++ interface to C library errors
 */

#ifndef errno_h

#include "//usr/include/errno.h"

/* just in case standard header didn't */
#ifndef errno_h
#define errno_h
#endif

extern int errno;

extern void perror(const char*);

extern char* sys_errlist[];
extern int sys_nerr;

#endif
