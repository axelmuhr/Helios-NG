/*
 * C++ interface to C library string functions
 */

#ifndef string_h
#define string_h

extern char* strcpy(char*, const char*);
extern char* strncpy(char*, const char*, int);
extern char* strcat(char*, const char*);
extern char* strncat(char*, const char*, int);

extern char* strchr(const char*, char);
extern char* strrchr(const char*, char);

extern int strcmp(const char*, const char*);
extern int strncmp(const char*, const char*, int);
extern int strlen(const char*);

#endif
