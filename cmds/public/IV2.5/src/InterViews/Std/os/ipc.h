/*
 * C++ interface to OS IPC functions
 */

#ifndef os_ipc_h
#define os_ipc_h

extern int socket(int, int, int);
extern int bind(int, void*, int);
extern int connect(int, ...);
extern int listen(int, int);
extern int accept(int, void*, int*);
extern int select(int, int*, int*, int*, struct timeval*);

#endif
