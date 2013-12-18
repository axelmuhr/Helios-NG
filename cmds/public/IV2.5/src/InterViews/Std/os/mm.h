/*
 * C++ interface to OS memory management functions
 */

#ifndef os_mm_h
#define os_mm_h

extern int brk(char*);
extern char* sbrk(int);

extern char* malloc(unsigned);
extern char* calloc(unsigned, unsigned);

#endif
