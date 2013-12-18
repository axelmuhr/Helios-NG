/*
 * Interface to libc memory ops.
 */

#ifndef memory_h
#define memory_h

extern char* memccpy(char*, const char*, int c, int n);
extern char* memchr(const char*, int c, int n);
extern int memcmp(const char*, const char*, int);
extern char* memcpy(char*, const char*, int);
extern char* memset(char*, int c, int n);

#endif
