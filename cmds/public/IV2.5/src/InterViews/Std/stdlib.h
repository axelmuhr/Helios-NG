/*
 * C++ interface to standard library functions
 */

#ifndef stdlib_h
#define stdlib_h

extern int atoi(const char*);
extern double atof(const char*);
extern long atol(const char*);
extern double strtod(const char*, char**);
extern long strtol(const char*, char**, int);

extern char** environ;

extern int abort();
extern void exit(int);
extern char* getenv(const char*);
extern int system(const char*);

extern int rand();
extern void srand(unsigned);

#endif
