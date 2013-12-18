/* limits.h: ANSI draft (X3J11 Oct 86) library header, section 2.2.4.2 */
/* Copyright (C) A.C. Norman and A. Mycroft */
/* version 0.01 - SccsId: %W% %G% */
/* $Id: limits.h,v 1.2 91/02/06 13:47:40 martyn Exp $ */

/* Expanded to incorporate POSIX limits; NHG 7/5/88 */

#ifndef __limits_h
#define __limits_h

#define CHAR_BIT 8
#define SCHAR_MIN (-128)
#define SCHAR_MAX 127
#define UCHAR_MAX 255
#define CHAR_MIN 0
#define CHAR_MAX 255

#define SHRT_MIN  (-0x8000)
#define SHRT_MAX  0x7fff
#define USHRT_MAX 65535U
#define INT_MIN   (~0x7fffffff)  /* -2147483648 and 0x80000000 are unsigned */
#define INT_MAX   0x7fffffff
#define UINT_MAX  0xffffffff
#define LONG_MIN  (~0x7fffffff)
#define LONG_MAX  0x7fffffff
#define ULONG_MAX 0xffffffffU

/* The following are for POSIX */

/* Note: some of these values are irrelevant to Helios, they are */
/* provided for completeness only and given their minimum values */

/* POSIX defined minimum values 				*/
#define _POSIX_ARG_MAX		4096
#define _POSIX_CHILD_MAX	6
#define _POSIX_LINK_MAX		8
#define _POSIX_MAX_CANON	255
#define _POSIX_MAX_INPUT	255
#define _POSIX_NAME_MAX		14
#define _POSIX_NGROUPS_MAX	0
#define _POSIX_OPEN_MAX		16
#define _POSIX_PATH_MAX		255
#define _POSIX_PIPE_BUF		512

/* Run-Time Increasable Values 					*/
/* Actual values may be obtained from sysconf()			*/
#define NGROUPS_MAX	_POSIX_NGROUPS_MAX

/* Runtime Invariant Values					*/
/* Actual values may be obtained from sysconf()			*/
#ifndef CLK_TCK
#define CLK_TCK		100
#endif
#define ARG_MAX		65535
#ifndef _POSIX_SOURCE
/* POSIX says that if any of these is greater than the minimum	*/
/* but of indeterminate magnitude, it shall be omitted. Only do	*/
/* this for explicitly notified POSIX sources.			*/
#define CHILD_MAX	LONG_MAX
#define OPEN_MAX	LONG_MAX
#endif

/* Pathname Variable Values					*/
/* These may vary with the filesystem or device being used, the	*/
/* actual values should be obtained from pathconf().		*/
#define LINK_MAX	_POSIX_LINK_MAX
#define MAX_CANON	_POSIX_MAX_CANON
#define MAX_INPUT	_POSIX_MAX_INPUT
#define NAME_MAX	31
#define PATH_MAX	512
#define	PIPE_BUF	_POSIX_PIPE_BUF

#endif

/* end of limits.h */
