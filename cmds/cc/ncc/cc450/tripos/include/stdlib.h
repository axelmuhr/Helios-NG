
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

double atof(const char *nptr);
int atoi(const char *nptr);
long int atol(const char *nptr);

double strtod(const char *nptr, char **endptr);
long int strtol(const char *nptr, char **endptr, int base);
unsigned long int strtoul(const char *nptr, char **endptr, int base);

int rand(void);
void srand(unsigned int seed);

void *calloc(size_t nmemb, size_t size);
void free(void *ptr);
void *malloc(size_t size);
void *realloc(void *ptr, size_t size);

void abort(void);
int  atexit(void (*func)(void));
void exit(int status);
char *getenv(const char *name);
int  system(const char *string);

void *bsearch(const void *key, const void *base,
              size_t nmemb, size_t size,
              int (*compar)(const void *, const void *));
void qsort(void *base, size_t nmemb, size_t size,
           int (*compar)(const void *, const void *));

int abs(int j);
div_t div(int numer, int denom);
long int labs(long int j);
ldiv_t ldiv(long int numer, long int denom);

#endif

/* end of stdlib.h */
