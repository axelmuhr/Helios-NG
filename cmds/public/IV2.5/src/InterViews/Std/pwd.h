/*
 * C++ interface to password file entries
 */

#ifndef pwd_h
#define pwd_h

#define getpwent pwd_h_getpwent
#define getpwuid pwd_h_getpwuid
#define getpwnam pwd_h_getpwnam
#define setpwent pwd_h_setpwent
#define endpwent pwd_h_endpwent

#include "//usr/include/pwd.h"

#undef getpwent
#undef getpwuid
#undef getpwnam
#undef setpwent
#undef endpwent

/* just in case standard header didn't */
#ifndef pwd_h
#define pwd_h
#endif

extern struct passwd* getpwent();
extern struct passwd* getpwuid(int uid);
extern struct passwd* getpwnam(char* name);

extern int setpwent();
extern int endpwent();

#endif
