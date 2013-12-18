/*
 * C++ interface to OS process management functions
 */

#ifndef os_proc_h
#define os_proc_h

extern int fork();
extern int vfork();
extern int execl(const char*, const char* ...);
extern int execle(const char*, const char* ...);
extern int execlp(const char*, const char* ...);
extern int execv(const char*, const char**);
extern int execve(const char*, const char**, char**);
extern int execvp(const char*, const char**);
extern int wait(int*);
extern int kill(int, int);
extern void exit(int);
extern void _exit(int);
extern int nice(int);

extern int getpgrp();
extern int getpid();
extern int getppid();
extern int setpgrp(int pid, int pgrp);

#endif
