/****************************************************************/
/*                          Ariel Corp.                         */
/*                        433 River Road                        */
/*                Highland Park, NJ 08904, U.S.A.               */
/*                     Tel:  (908) 249-2900                     */
/*                     Fax:  (908) 249-2123                     */
/*                     BBS:  (908) 249-2124                     */
/*                  E-Mail:  ariel@ariel.com                    */
/*                                                              */
/*                 Copyright (C) 1993 Ariel Corp.               */
/****************************************************************/

/* $Id: portable.h,v 1.1 1994/06/29 13:46:19 tony Exp $ */

/* portable.h -- include file. 
 *
 * Meant to be included by other include files...
 *
 * Attempts to provide a centralized location to handle K&R/ANSI
 * incompatibilities and define macros to handle compiler-specific 
 * non-portable features. (e.g., the "inline" keyword.)
 *
 */

#ifndef PORTABLE_H
#define PORTABLE_H 1

/* 
 * Note compiler-specific weird features here.
 */

#if defined (__GNUC__)		/* gnu gcc */
#   define CC_INLINE inline
#   define CC_AG_INIT 
#else
#   if defined (sun)		/* sun cc */
#      undef CC_INLINE
#      undef CC_AG_INIT
#   else
#      if defined (i860)		/* metaware hc860 */
#         define CC_INLINE _inline;
#      else
#         if defined (_TMS320C40)	/* TI cl30 */
#            define c40 1
#            define __C40__ 1
#         else
#            if defined (__BORLANDC__)	/* borland c */
#               define CC_ANSI_PROTOTYPE
#               define CC_VOID_P
#               undef CC_INLINE
#            else
#               if defined (MSDOS) /* Microsoft c */
#                  if !defined (__MSDOS__)
#                     define __MSDOS__ 1
#                  endif
#               endif
#            endif
#         endif
#      endif
#   endif
#endif

/* 
 * C++, Ansi, K&R differences. Take care of prototypes and Ansi keywords.
 */

#if defined (__cplusplus)	/* C++ */
#   define CPP_PROTOTYPE
#   define CC_CONST
#   define CC_VOID_P
#endif

#if defined (__STDC__)		/* Ansi */
#   define CC_ANSI_PROTOTYPE
#   define CC_CONST
#   define CC_VOID_P
#endif				/* C++, Ansi, K & R switches */

/* 
 * C features. Generally, I use the Capitalized name as the macro. 
 */

#if defined (CPP_PROTOTYPE)
#   define PROTOTYPE(name,arglist) extern "C" { name arglist; } ;
#elif defined (CC_ANSI_PROTOTYPE)
#   define PROTOTYPE(name,arglist) name arglist;
#else
#   define PROTOTYPE(name,arglist) name();
#endif

/* 
 * some non-ANSI compilers have const available 
 */

#if defined (CC_CONST)
#   define Const const
#else
#   define Const
#endif				/* CC_CONST */

/* 
 * some non-ANSI compilers have "void *" 
 */

#if defined (CC_VOID_P)
typedef void *    Void_p;
#else
typedef char *    Void_p;
#endif				/* CC_VOID_P */

/* 
 * some compilers can inline functions, and the syntax varies.
 */

#if defined (CC_INLINE)
#   define Inline CC_INLINE
#else
#   define Inline
#endif				/* CC_INLINE */

/*
 * sun cc, for one, doesn't allow automatic aggregate initialization yet.
 */

#if defined (CC_AG_INIT)
#  define static_ag 
#else
#  define static_ag static
#endif

/* 
 * Make "far" go away if not on a PC 
 */

#if !defined(__MSDOS__) && !defined(__OS2__)
#   if !defined( far )
#       define far
#   endif
#endif

/* 
 * Individual compiler quirks. 
 */

#if defined (sun)		/* sun's stupid non-ansi header files */
#   include <stdio.h>
#   include <time.h>
#   include <sys/time.h>

PROTOTYPE (int fprintf, 
	   (FILE * stream, Const char *format,...))
PROTOTYPE (int printf, 
	   (Const char *format,...))
PROTOTYPE (void perror, 
	   (Const char *string))
PROTOTYPE (void fflush, 
	   (FILE * stream))
PROTOTYPE (int sscanf, 
	   (Const char *s, Const char *format,...))
PROTOTYPE (time_t time, 
	   (time_t * time_buf_p))
PROTOTYPE (int fread, 
	   (Void_p ptr, size_t size, int nitems, FILE * stream))
PROTOTYPE (int fwrite, 
	   (Void_p ptr, size_t size, int nitems, FILE * stream))
PROTOTYPE (int fclose, 
	   (FILE * stream))

#   if defined (__STDC__)		/* Ansi */
#      include <stdarg.h>
PROTOTYPE (int vfprintf, 
	   (FILE * stream, Const char *format, va_list ap))
#   endif /* __STDC__ */

#endif				/* stupid sun tricks */

#ifdef i860			/* stupid mc860 tricks */
#   include <math.h>
#   define M_PI _PI
#endif				/* stupid mc860 tricks */

#endif				/* PORTABLE_H */
