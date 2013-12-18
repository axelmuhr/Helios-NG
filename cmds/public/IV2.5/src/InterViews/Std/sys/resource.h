/*
 * C++ interface to Unix system resource information
 */

#ifndef sys_resource_h

#include "//usr/include/sys/resource.h"

/* just in case standard header didn't */
#ifndef sys_resource_h
#define sys_resource_h
#endif

extern int getrlimit(int resource, struct rlimit*);
extern int setrlimit(int resource, struct rlimit*);

#endif
