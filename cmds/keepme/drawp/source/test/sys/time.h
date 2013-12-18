/*
 * $Header: time.h,v 1.1 90/01/13 20:14:44 charles Locked $
 * $Source: /server/usr/users/charles/world/drawp/RCS/source/test/sys/time.h,v $
 *
 * Copyright (c) 1988 Acorn Computers Ltd., Cambridge, England
 *
 * $Desc$
 * $Log:	time.h,v $
 * Revision 1.1  90/01/13  20:14:44  charles
 * Initial revision
 * 
 * Revision 1.10  89/12/19  17:22:12  john
 * File unchanged
 * 
 * Revision 1.9  89/10/26  16:30:27  charles
 * *** empty log message ***
 * 
 * Revision 1.8  89/10/18  14:50:11  john
 * *** empty log message ***
 * 
 * Revision 1.7  89/10/17  10:05:47  charles
 * File unchanged
 * 
 * Revision 1.6  89/08/14  18:24:55  charles
 * File unchanged
 * 
 * Revision 1.5  89/08/10  21:04:57  charles
 * File unchanged
 * 
 * Revision 1.4  89/08/10  19:10:29  charles
 * File unchanged
 * 
 * Revision 1.3  89/08/10  15:35:37  charles
 * File unchanged
 * 
 * Revision 1.2  89/08/10  15:31:32  charles
 * File unchanged
 * 
 * Revision 1.1  89/08/10  14:59:21  charles
 * Initial revision
 * 
 * Revision 1.4  88/10/11  10:26:26  keith
 * Protect <sys/time.h> against multiple inclusion.
 * 
 * Revision 1.3  88/06/17  20:21:51  beta
 * Acorn Unix initial beta version
 * 
 */
/* @(#)time.h	1.2 87/05/15 3.2/4.3NFSSRC */
/*
 * Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)time.h	7.1 (Berkeley) 6/4/86
 */

#ifndef __SYS_TIME__
#define __SYS_TIME__
/*
 * Structure returned by gettimeofday(2) system call,
 * and used in other calls.
 */
struct timeval {
	long	tv_sec;		/* seconds */
	long	tv_usec;	/* and microseconds */
};

struct timezone {
	int	tz_minuteswest;	/* minutes west of Greenwich */
	int	tz_dsttime;	/* type of dst correction */
};
#define	DST_NONE	0	/* not on dst */
#define	DST_USA		1	/* USA style dst */
#define	DST_AUST	2	/* Australian style dst */
#define	DST_WET		3	/* Western European dst */
#define	DST_MET		4	/* Middle European dst */
#define	DST_EET		5	/* Eastern European dst */
#define	DST_CAN		6	/* Canada */

/*
 * Operations on timevals.
 *
 * NB: timercmp does not work for >= or <=.
 */
#define	timerisset(tvp)		((tvp)->tv_sec || (tvp)->tv_usec)
#define	timercmp(tvp, uvp, cmp)	\
	((tvp)->tv_sec cmp (uvp)->tv_sec || \
	 (tvp)->tv_sec == (uvp)->tv_sec && (tvp)->tv_usec cmp (uvp)->tv_usec)
#define	timerclear(tvp)		(tvp)->tv_sec = (tvp)->tv_usec = 0

/*
 * Names of the interval timers, and structure
 * defining a timer setting.
 */
#define	ITIMER_REAL	0
#define	ITIMER_VIRTUAL	1
#define	ITIMER_PROF	2

struct	itimerval {
	struct	timeval it_interval;	/* timer interval */
	struct	timeval it_value;	/* current value */
};

#ifndef KERNEL
#include <time.h>
#endif
#endif /* Junk removed for ANSI precompiler */
