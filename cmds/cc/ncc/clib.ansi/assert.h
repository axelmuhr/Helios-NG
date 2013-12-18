#pragma force_top_level
#pragma include_only_once

/* assert.h: ANSI 'C' (X3J11 Oct 88) library header, section 4.2 */
/* Copyright (C) Codemist Ltd. */
/* version 0.02 */

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
#  define __assert_h
   extern void __assert(char *, char *, int);
#else
#  undef assert
#endif

#ifdef NDEBUG
#  define assert(ignore) ((void)0)
#else
#  define assert(e) ((e) ? (void)0 : __assert(#e, __FILE__, __LINE__))
#endif

/* end of assert.h */
