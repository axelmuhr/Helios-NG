
/* stdlib.h: ANSI draft (X3J11 Oct 86) library header, section 4.10 */
/* Copyright (C) A.C. Norman and A. Mycroft */
/* version 0.01 */

#ifndef __stdlib_h
#define __stdlib_h

/* it is far from clear that <stdlib.h> should define size_t      */
#ifndef size_t
#  define size_t unsigned int                  /* from <stddef.h> */
#endif

typedef struct div_t { int quot, rem; } div_t;
typedef struct ldiv_t { long int quot, rem; } ldiv_t;

/* The following two macros also appear in <math.h> */
#ifndef ERANGE
#define ERANGE 2
#endif
#ifndef HUGE_VAL
extern const double _huge_val;
#define HUGE_VAL _huge_val
#endif
#define RAND_MAX 0x7fff

double atof();
int atoi();
long int atol();

double strtod();
long int strtol();
unsigned long int strtoul();

int rand();
void srand();

void *calloc();
void free();
void *malloc();
void *realloc();

void abort();
int  atexit();
void exit();
char *getenv();
int  system();

void *bsearch();
void qsort();

#ifndef COMPILING_ON_ST
int abs();
#endif
div_t div();
long int labs();
ldiv_t ldiv();

#endif

/* end of stdlib.h */
