/* assert.h: ANSI draft (X3J11 Oct 86) library header, section 4.2 */
/* Copyright (C) A.C. Norman and A. Mycroft */
/* version 0.01 - SccsId: %W% %G% */
/* $Id: assert.h,v 1.4 1993/05/25 18:56:13 bart Exp $ */

/*
 * The assert macro puts diagnostics into programs. When it is executed,
 * if its argument expression is false, it writes information about the
 * call that failed (including the text of the argument, the name of the
 * source file, and the source line number - the latter are respectively
 * the values of the preprocessing macros __FILE__ and __LINE__) on the
 * standard error stream. It then calls the abort function.
 * If its argument expression is true, the assert macro returns no value.
 */

/*
 * Note that <assert.h> may be included more that once in a program with
 * different setting of NDEBUG. Hence the slightly unusual first-time
 * only flag.
 */

#ifndef __assert_h
# define __assert_h
  extern void _assert_fail( char *, char *, int );
#else
# undef assert
#endif /* __assert_h */

#ifdef NDEBUG
/* ANSI require the following silly expansion:
 *  (bans (f() ? assert(e) : assert(e')) for no good reason).
 */
# ifdef assert
# undef assert
# endif

# define assert( ignore ) ((void)0)
#else
/* Syntactially MUST be done with a conditional expression -
 *  consider "if(1) assert(1); else ..."
 */
# ifdef assert
# undef assert
# endif

# define assert( e ) ((e) ? (void)0 : _assert_fail( #e, __FILE__, __LINE__ ))
#endif /* NDEBUG */


/* end of assert.h */
