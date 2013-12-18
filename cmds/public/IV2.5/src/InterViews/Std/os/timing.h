/*
 * C++ interface to OS timing-related functions
 */

#ifndef os_timing_h
#define os_timing_h

extern unsigned alarm(unsigned);
extern void pause();
extern unsigned sleep(unsigned);

extern int stime(long*);

extern long ulimit(int cmd, long newlimit);

extern int getitimer(int, struct itimerval*);
extern int setitimer(int, struct itimerval*, struct itimerval*);

extern void profil(char*, int bufsiz, int offset, int scale);

extern int acct(const char* file);

#endif
