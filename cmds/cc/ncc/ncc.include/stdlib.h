#ifndef _SIZE_T
#define _SIZE_T
typedef unsigned size_t;
#endif
#ifndef _WCHAR_T
#define _WCHAR_T
typedef int wchar_t;
#endif
#ifndef _DIV_T
#define _DIV_T
typedef struct { int quot; int rem; } div_t;
#endif
#ifndef _LDIV_T
#define _LDIV_T
typedef struct { long int quot; long int rem; } ldiv_t;
#endif
#ifndef NULL
#define NULL ((void *)0)
#endif
#define EXIT_FAILURE  1
#define EXIT_SUCCESS  0
#ifdef SYSTEM_FIVE
#define RAND_MAX  0x7FFF
#else
#define RAND_MAX  0x7FFFFFFF
#endif
#define MB_CUR_MAX  1
extern double atof(const char *nptr);
extern int atoi(const char *nptr);
extern long int atol(const char *nptr);
extern double strtod(const char *nptr, char **endptr);
extern long int strtol(const char *nptr, char **endptr, int base);
extern unsigned long int strtoul(const char *nptr, char **endptr, int base);
extern int rand(void);
extern void srand(unsigned int seed);
extern void *calloc(size_t nmemb, size_t size);
extern void free(void *ptr);
extern void *malloc(size_t size);
extern void *realloc(void *ptr, size_t size);
extern void abort(void);
extern int atexit(void (*func)(void));
extern void exit(int status);
extern char *getenv(const char *name);
extern int system(const char *string);
extern void *bsearch(
  const void *key, const void *base,
  size_t nmemb, size_t size,
  int (*compar)(const void *, const void *)
);
extern void qsort(
  void *base, size_t nmemb, size_t size,
  int (*compar)(const void *, const void *)
);
extern int abs(int j);
extern div_t div(int numer, int denom);
extern long int labs(long int j);
extern ldiv_t ldiv(long int numer, long int denom);
extern int mblen(const char *s, size_t n);
extern int mbtowc(wchar_t *pwc, const char *s, size_t n);
extern int wctomb(char *s, wchar_t wchar);
extern size_t mbstowcs(wchar_t *pwcs, const char *s, size_t n);
extern size_t wcstombs(char *s, const wchar_t *pwcs, size_t n);
