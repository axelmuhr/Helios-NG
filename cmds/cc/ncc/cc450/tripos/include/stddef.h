
/* stddef.h: ANSI draft (X3J11 Oct 86) library header, section 4.1.4 */
/* Copyright (C) A.C. Norman and A. Mycroft */
/* version 0.02 */

#ifndef __stddef_h
#define __stddef_h

#define ptrdiff_t int
#ifndef size_t
#  define size_t unsigned int   /* others (e.g. <stdio.h>) define */
#endif
#ifndef NULL  /* this hack is so that <stdio.h> can also define it */
#  define NULL 0
#endif
#define offsetof(type, member) \
    ((size_t)((char *)&(((type *)0)->member) - (char *)0))
#ifndef errno
extern volatile int _errno;
#  define errno _errno
#endif
#endif

/* end of stddef.h */
