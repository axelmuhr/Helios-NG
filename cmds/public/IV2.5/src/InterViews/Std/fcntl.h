/*
 * C++ interface to system file control
 */

#ifndef fcntl_h

#include "//usr/include/fcntl.h"

/* just in case standard header didn't */
#ifndef fcntl_h
#define fcntl_h
#endif

extern int fcntl(int fd, int request, int arg);

#endif
