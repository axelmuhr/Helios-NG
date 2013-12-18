/* stddef.h: ANSI draft (X3J11 Oct 86) library header, section 4.1.4 */
/* Copyright (C) A.C. Norman and A. Mycroft */
/* version 0.01 - SccsId: %W% %G% */
/* $Id: stddef.h,v 1.6 1992/11/20 18:10:29 nick Exp $ */

#ifndef __stddef_h
# define __stddef_h

typedef int ptrdiff_t;		/* the signed integral type of the result of subtracting two pointers. */
#ifndef __STDC__
#   define ptrdiff_t int   	/* ANSI bans this -- delete unless pcc wants.   */
#endif

# if !defined size_t && !defined __size_t
   typedef unsigned int size_t;	/* the unsigned integral type of the result of the sizeof operator. */
#  define __size_t 1		/* cf stdlib.h */
# endif

#ifndef __wchar_t
# define __wchar_t 1
  typedef int wchar_t;                         /* also in <stdlib.h> */
   /*
    * An integral type whose range of values can represent distinct codes for
    * all members of the largest extended character set specified among the
    * supported locales; the null character shall have the code value zero and
    * each member of the basic character set shall have a code value when used
    * as the lone character in an integer character constant.
    */
#endif

#ifndef NULL  /* this hack is so that <stdio.h> can also define it */
#  define NULL 0
# endif

#ifdef offsetof /* replace any current offsetof() with the one defined here */
#undef offsetof
#endif

/* for compatibility - should really read: ifndef __TRAN */
# if defined(__ARM) || defined(__C40)
#  define offsetof(type, member) \
    ((size_t)((char *)&(((___type type *)0)->member) - (char *)0))
   /*
    * expands to an integral constant expression that has type size_t, the
    * value of which is the offset in bytes, from the beginning of a structure
    * designated by type, of the member designated by the identifier (if the
    * specified member is a bit-field, the behaviour is undefined).
    */
# else
#  define offsetof(type, member) ((char *)&(((type *)0)->member) - (char *)0)
# endif

#endif

/* end of stddef.h */
