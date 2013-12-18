/*
 * C++ interface to OS hostname functions
 */

#ifndef os_host_h
#define os_host_h

extern int gethostname(char*, int);
extern int sethostname(char*, int);

extern int gethostid();
extern int sethostid(int);

#endif
