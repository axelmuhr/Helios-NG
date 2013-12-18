/*
 * C++ interface to Unix file system stat structure
 */

#ifndef sys_stat_h

#include "//usr/include/sys/stat.h"

/* just in case standard header didn't */
#ifndef sys_stat_h
#define sys_stat_h
#endif

extern int stat(const char* path, struct stat*);
extern int fstat(int fd, struct stat*);
extern int lstat(const char* path, struct stat*);

#endif
