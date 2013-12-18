
/* stddef.h: ANSI draft (X3J11 Oct 86) library header, section 4.1.4 */
/* Copyright (C) A.C. Norman and A. Mycroft */
/* version 0.01 */

#ifndef __stddef_h
#define __stddef_h

#ifdef COMPILING_ON_SUN4
#define void int
#define const
#define volatile 
#endif
#ifdef COMPILING_ON_ST
#define void int
#define const
#define volatile 
#endif

#ifndef COMPILING_ON_SUN4
#define ptrdiff_t int
#ifndef size_t
#  define size_t unsigned int   /* others (e.g. <stdio.h>) define */
#endif
#endif
#ifndef NULL  /* this hack is so that <stdio.h> can also define it */
#  define NULL 0
#endif
#ifdef COMPILING_ON_ST
/* Lattice C cannot handle the zeroes for some reason!! */
#define offsetof(type, member) ((char *)&(((type *)4)->member) - (char *)4)
#else
#define offsetof(type, member) ((char *)&(((type *)0)->member) - (char *)0)
#endif
#ifndef errno
extern volatile int _errno;
#  define errno _errno
#endif
#endif

/* end of stddef.h */
