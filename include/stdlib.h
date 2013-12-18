/* stdlib.h: ANSI draft (X3J11 Oct 86) library header, section 4.10 */
/* Copyright (C) A.C. Norman and A. Mycroft */
/* version 0.02 */
/* $Id: stdlib.h,v 1.6 1993/06/24 11:46:56 nickc Exp $ */

#ifndef __stdlib_h
#define __stdlib_h

/* it is far from clear that <stdlib.h> should define size_t      */
#if !defined size_t && !defined __size_t
#  define __size_t 1
   typedef unsigned int size_t;   /* from <stddef.h> */
#endif

#if !defined __wchar_t && !defined __sys_stdtypes_h	/* __sys_stdtype_h is defined by SUN's sys/stdtype.h */
   typedef int wchar_t;           /* from <stddef.h> */
#  define __wchar_t 1
#endif

#ifndef NULL
#  define NULL 0                  /* from <stddef.h> */
#endif

typedef struct div_t  {      int quot, rem; } div_t;	  /* type of the value returned by the div function. */
typedef struct ldiv_t { long int quot, rem; } ldiv_t;	  /* type of the value returned by the ldiv function. */


/* The following two macros also appear in <math.h> */

#ifndef ERANGE
#define ERANGE 32
#endif

#ifndef HUGE_VAL
extern const double _huge_val;
#define HUGE_VAL _huge_val
#endif

   /*
    * an integral expression which may be used as an argument to the exit
    * function to return successful termination status to the host
    * environment.
    */
#define EXIT_SUCCESS	0

   /*
    * an integral expression which may be used as an argument to the exit
    * function to return unsuccessful termination status to the host
    * environment.
    */
#ifdef __EXIT_FAILURE
#  define EXIT_FAILURE __EXIT_FAILURE
#else
#  define EXIT_FAILURE 1  /* unixoid */
#endif

   /*
    * an integral constant expression, the value of which is the maximum value
    * returned by the __rand function.
    */
#define __RAND_MAX 0x7fffffff

   /*
    * an integral constant expression, the value of which is the maximum value
    * returned by the rand function.
    */
#define RAND_MAX 0x7fff /* for the so-called portable version rand() */

   /*
    * a positive integer expression whose value is the maximum number of bytes
    * in a multibyte character for the extended character set specified by the
    * current locale (category LC_CTYPE), and whose value is never greater
    * than MB_LEN_MAX.
    */
#define MB_CUR_MAX 1


double 			atof( const char * nptr );
int 			atoi( const char * nptr );
long int		atol( const char * nptr );

double 			strtod(  const char * nptr, char ** endptr );
long int 		strtol(  const char * nptr, char ** endptr, int base );
unsigned long int	strtoul( const char * nptr, char ** endptr, int base );

int 			rand( void );
void 			srand( unsigned int seed );
extern int		__rand( void );
extern void		__srand( unsigned int seed );

void *			calloc( size_t nmemb, size_t size );
void 			free( void * ptr );
void *			malloc( size_t size );
void *			realloc( void * ptr, size_t size );

void 			abort( void );
int  			atexit( void (* func )( void ) );
void 			exit( int status );
char *			getenv( const char * name );
int  			system( const char * string );

void *			bsearch( const void * key, const void * base, size_t nmemb, size_t size,
				int (* compar )(const void *, const void *) );
void 			qsort( void * base, size_t nmemb, size_t size,
			      int (* compar )(const void *, const void *) );

#ifndef abs
int 			abs( int j );
#endif

div_t 			div( int numer, int denom );
long int 		labs( long int j );
ldiv_t 			ldiv( long int numer, long int denom );

/*
 * Multibyte Character Functions.
 * The behaviour of the multibyte character functions is affected by the
 * LC_CTYPE category of the current locale. For a state-dependent encoding,
 * each function is placed into its initial state by a call for which its
 * character pointer argument, s, is a null pointer. Subsequent calls with s
 * as other than a null pointer cause the internal state of the function to be
 * altered as necessary. A call with s as a null pointer causes these functions
 * to return a nonzero value if encodings have state dependency, and a zero
 * otherwise. After the LC_CTYPE category is changed, the shift state of these
 * functions is indeterminate.
 */

   /*
    * If s is not a null pointer, the mblen function determines the number of
    * bytes compromising the multibyte character pointed to by s. Except that
    * the shift state of the mbtowc function is not affected, it is equivalent
    * to   mbtowc((wchar_t *)0, s, n);
    * Returns: If s is a null pointer, the mblen function returns a nonzero or
    *          zero value, if multibyte character encodings, resepectively, do
    *          or do not have state-dependent encodings. If s is not a null
    *          pointer, the mblen function either returns a 0 (if s points to a
    *          null character), or returns the number of bytes that compromise
    *          the multibyte character (if the next n of fewer bytes form a
    *          valid multibyte character), or returns -1 (they do not form a
    *          valid multibyte character).
    */
extern int 		mblen( const char * s, size_t n );

   /*
    * If s is not a null pointer, the mbtowc function determines the number of
    * bytes that compromise the multibyte character pointed to by s. It then
    * determines the code for value of type wchar_t that corresponds to that
    * multibyte character. (The value of the code corresponding to the null
    * character is zero). If the multibyte character is valid and pwc is not a
    * null pointer, the mbtowc function stores the code in the object pointed
    * to by pwc. At most n bytes of the array pointed to by s will be examined.
    * Returns: If s is a null pointer, the mbtowc function returns a nonzero or
    *          zero value, if multibyte character encodings, resepectively, do
    *          or do not have state-dependent encodings. If s is not a null
    *          pointer, the mbtowc function either returns a 0 (if s points to
    *          a null character), or returns the number of bytes that
    *          compromise the converted multibyte character (if the next n of
    *          fewer bytes form a valid multibyte character), or returns -1
    *          (they do not form a valid multibyte character).
    */
extern int 		mbtowc( wchar_t * pwc, const char * s, size_t n );

   /*
    * determines the number of bytes need to represent the multibyte character
    * corresponding to the code whose value is wchar (including any change in
    * shift state). It stores the multibyte character representation in the
    * array object pointed to by s (if s is not a null pointer). At most
    * MB_CUR_MAX characters are stored. If the value of wchar is zero, the
    * wctomb function is left in the initial shift state).
    * Returns: If s is a null pointer, the wctomb function returns a nonzero or
    *          zero value, if multibyte character encodings, resepectively, do
    *          or do not have state-dependent encodings. If s is not a null
    *          pointer, the wctomb function returns a -1 if the value of wchar
    *          does not correspond to a valid multibyte character, or returns
    *          the number of bytes that compromise the multibyte character
    *          corresponding to the value of wchar.
    */
extern int 		wctomb( char * s, wchar_t wchar );

/*
 * Multibyte String Functions.
 * The behaviour of the multibyte string functions is affected by the LC_CTYPE
 * category of the current locale.
 */

   /*
    * converts a sequence of multibyte character that begins in the initial
    * shift state from the array pointed to by s into a sequence of
    * corresponding codes and stores not more than n codes into the array
    * pointed to by pwcs. No multibyte character that follow a null character
    * (which is converted into a code with value zero) will be examined or
    * converted. Each multibyte character is converted as if by a call to
    * mbtowc function, except that the shift state of the mbtowc function is
    * not affected. No more than n elements will be modified in the array
    * pointed to by pwcs. If copying takes place between objects that overlap,
    * the behaviour is undefined.
    * Returns: If an invalid multibyte character is encountered, the mbstowcs
    *          function returns (size_t)-1. Otherwise, the mbstowcs function
    *          returns the number of array elements modified, not including
    *          a terminating zero code, if any.
    */
extern size_t 		mbstowcs( wchar_t * pwcs, const char * s, size_t n );

   /*
    * converts a sequence of codes that correspond to multibyte characters
    * from the array pointed to by pwcs into a sequence of multibyte
    * characters that begins in the initial shift state and stores these
    * multibyte characters into the array pointed to by s, stopping if a
    * multibyte character would exceed the limit of n total bytes or if a
    * null character is stored. Each code is converted as if by a call to the
    * wctomb function, except that the shift state of the wctomb function is
    * not affected. No more than n elements will be modified in the array
    * pointed to by s. If copying takes place between objects that overlap,
    * the behaviour is undefined.
    * Returns: If a code is encountered that does not correspond to a valid
    *          multibyte character, the wcstombs function returns (size_t)-1.
    *          Otherwise, the wcstombs function returns the number of bytes
    *          modified, not including a terminating null character, if any.
    */
extern size_t 		wcstombs( char * s, const wchar_t * pwcs, size_t n );

#endif /* __stdlib_h */

/* end of stdlib.h */
