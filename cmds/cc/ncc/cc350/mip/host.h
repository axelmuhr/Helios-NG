#ifdef USE_NORCROFT_PRAGMAS
#pragma force_top_level
#pragma include_only_once
#endif
/*
 * C compiler file mip/host.h, version 10
 * Copyright (C) Codemist Ltd., 1988
 * Copyright (C) Acorn Computers Ltd., 1988.
 */

/*
 * RCS $Revision: 1.2 $
 * Checkin $Date: 1992/04/21 13:37:03 $
 * Revising $Author: nickc $
 */

/* AM memo, July 1990: in principle there should be no tests of         */
/* COMPILING_ON_<machine>, but only COMPILING_ON_<operating system> or  */
/* COMPILING_ON_<manufacturer> (for special features).                  */
/* Accordingly COMPILING_ON_<machine> is deprecated.                    */

/*
 * This file deals with peculiarities of the host system under which the
 * compiler is compiled AND peculiarities of the host system under which
 * the compiler will run (hence it might need further explication in the
 * unlikely event that we wish to cross-compile the compiler (repeatedly
 * as opposed to once-off bootstrap)).   It is now loaded first and can
 * therefore not depend on TARGET_xxx parameterisations, these are now
 * done in target.h or, if systematic, in mip/defaults.h.
 * The correct mechanism for host->target dependencies (e.g. if compiling
 * on unix then make a unix compiler, else a homebrew object file version)
 * is via the options.h file, along the lines of:
 *   #ifdef COMPILING_ON_UNIX
 *   #  define TARGET_IS_UNIX
 *   #endif
 * The intent is that most of the pecularities should be linked to
 * COMPILING_ON_machine and/or COMPILING_ON_system.  Further NO OTHER FILE
 * should refer to magic names like 'unix', '__arm' etc., but go via
 * the COMPILING_xxx flags defined here in terms of these.
 * The aim is that this file should suffice for all host dependencies
 * and thus all COMPILING_ON_xxx tests outwith are suspect.  However,
 * the #include file munger clearly needs to so depend.
 */

#ifndef _host_LOADED
#define _host_LOADED 1

#include <stdio.h>

#ifdef __STDC__
#define BELL      '\a'
typedef void                  /* newline to fool the topcc tool */
             *VoidStar;
typedef const void
             *ConstVoidStar;
#define safe_tolower(ch)      tolower(ch)  /* see comment below */
#define safe_toupper(ch)      toupper(ch)  /* see comment below */
#else
#define BELL      '\007'
typedef char *VoidStar;
#define  ConstVoidStar VoidStar
/*
 * not all C libraries define tolower() and toupper() over all character
 * values. BSD Unix, for example, defines tolower() only over UC chars.
 */
#define safe_tolower(ch)      (isupper(ch) ? tolower(ch) : ch)
#define safe_toupper(ch)      (islower(ch) ? toupper(ch) : ch)
#  ifdef bsd
#    define sprintf ansi_sprintf
#  endif
#endif

#ifdef __CC_NORCROFT
  #pragma -e1                 /* temp hack to allow #error to continue */
#endif

#ifdef unix                   /* A temporary sop to older compilers */
#  ifndef __unix              /* (good for long-term portability?)  */
#    define __unix    1
#  endif
#endif

#ifdef __unix
/* Generic unix -- hopefully a split into other variants will not be    */
/* needed.  However, beware the 'bsd' test above and safe_toupper etc.  */
/* which cope with backwards (pre-posix/X/open) unix compatility.       */
#  define COMPILING_ON_UNIX     1
#endif
#ifdef __helios
/* start improving parameterisation.  Maybe we should also set          */
/* COMPILING_ON_UNIX and use HELIOS as a special subcase?               */
#  define COMPILING_ON_HELIOS   1
#endif
#ifdef __acorn
#  define COMPILING_ON_ACORN_KIT  1
#endif
#ifdef __riscos
#  define COMPILING_ON_RISC_OS  1
#endif
#ifdef __arm
#  define COMPILING_ON_ARM      1  /* dying: unix/riscos/acorn suffice  */
#endif
#ifdef __ibm370
#  define COMPILING_ON_370      1
#  ifndef COMPILING_ON_UNIX
#     define __mvs 1               /* a hack to be removed soon        */
#  endif
#endif
#ifdef __mvs
#  define COMPILING_ON_MVS      1
#endif
#ifdef _MSDOS
#  define COMPILING_ON_HIGH_C   1
#  define COMPILING_ON_MSDOS    1
#endif
/* AM: @@@ the following machine settings are under threat of removal   */
#ifdef __clipper
#  define COMPILING_ON_CLIPPER  1
#  define COMPILING_ON_UNIX     1  /* default file munging in driver.c */
#endif
#ifdef __m88000
#  define COMPILING_ON_88000    1
#  define COMPILING_ON_UNIX     1  /* default file munging in driver.c */
#endif
#ifdef __i860
#  define COMPILING_ON_860      1
#  define COMPILING_ON_UNIX     1  /* default file munging in driver.c */
#endif
#ifdef _ACW
/* The 32000-series back-end comes from W. G. Dixon */
#  define COMPILING_ON_ACW      1
#endif
#ifdef _AMD
#  define COMPILING_ON_AMD      1
#endif
#ifdef _68000
#  define COMPILING_ON_68000    1
#endif
/* AM @@@ to here.                                                      */
/*
 * The following typedefs may need alteration for obscure host machines.
 */
typedef long int            int32;
typedef long unsigned int   unsigned32;
typedef short int           int16;
typedef short unsigned int  unsigned16;
typedef signed char         int8;
typedef unsigned char       unsigned8;
#ifndef __helios_h
typedef int                 bool;
#endif

/* The following two lines are a safety play against using ncc with a   */
/* vendor supplied library, many of which refuse to accept "wb".  POSIX */
/* and other unix standards require "wb" and "w" to have the same       */
/* effect and so the following is safe.                                  */
#if defined __HELIOS
#define FOPEN_WB	"wb"
#elif defined COMPILING_ON_UNIX
#  define FOPEN_WB "w"
/* #  define remove unlink  *//* what a nasty hack, but probably best? */
#else
#  define FOPEN_WB "wb"
#endif

#ifdef __CC_NORCROFT
#  ifndef COMPILING_ON_UNIX     /* suppress if maybe non-norcroft lib */
#    define LIBRARY_IS_NORCROFT 1
#  endif
#endif

#ifdef __m88000  /* only temporary, I do hope. */
#define _vfprintf vfprintf
#define _vsprintf vsprintf
#define _fprintf fprintf
#define _sprintf sprintf
#else

#ifdef LIBRARY_IS_NORCROFT
/*
 * Calls to all non-ansi functions are removable by macros here.
 */
extern int _vfprintf(FILE *stream, const char *format, __va_list arg);
extern int _vsprintf(char *s, const char *format, __va_list arg);
extern int _fprintf(FILE *stream, const char *format, ...);
extern int _sprintf(char *s, const char *format, ...);
#else /* LIBRARY_IS_NORCROFT */
#  define _vfprintf vfprintf
#  define _vsprintf vsprintf
#  define _fprintf fprintf
#  define _sprintf sprintf
#endif /* LIBRARY_IS_NORCROFT */
#endif /* __88000 */

#ifdef COMPILING_ON_MVS
#define HOST_USES_CCOM_INTERFACE 1
#define EXIT_warn                4
#define EXIT_error               8
#define EXIT_fatal              12
#define EXIT_syserr             16
/*
 * The following #included #define's ensure that external symbols are
 * limited to 6 chars without gratuitous changes to every file.
 */

#include "sixchar.h"

#else   /* NOT on MVS */

#define EXIT_warn                0
#define EXIT_error               1
#define EXIT_fatal               1
#ifdef  COMPILING_ON_UNIX
#  define EXIT_syserr            100
#else
#  define EXIT_syserr            1
#endif

#endif

#endif

/* end of mip/host.h */
