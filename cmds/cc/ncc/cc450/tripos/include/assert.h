
/* assert.h: ANSI draft (X3J11 Oct 86) library header, section 4.2 */
/* Copyright (C) A.C. Norman and A. Mycroft */
/* version 0.01 */

#ifndef __assert_h
#define __assert_h

#ifdef NDEBUG
/* ANSI require the following silly expansion:
   (bans (f() ? assert(e) : assert(e')) for no good reason).
*/
#  define assert(ignore)
#else
/* Syntactially MUST be done with a conditional expression -
   consider "if(1) assert(1); else ..."
*/
extern void _assert_fail(char *, char *, int);
#  define assert(e) \
     ((e) ? (void)0 : _assert_fail(#e, __FILE__, __LINE__))
#endif

#endif

/* end of assert.h */
