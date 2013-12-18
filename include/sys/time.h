/* sys/time.h: BSD compatability header					*/
/* SccsId: %W% %G% (C) Copyright 1990, Perihelion Software Ltd.		*/
/* RcsId: $Id: time.h,v 1.2 90/10/02 19:02:36 nick Exp $ */

#ifndef __sys_time_h
#define __sys_time_h

#include <sys/types.h>

struct timeval {
	unsigned long 	tv_sec;
	long 		tv_usec;
};

struct timezone {
	int	tz_minuteswest;		/* minutes west of Greenwich */
	int	tz_dsttime;		/* type of DST correction */
};

extern int gettimeofday(struct timeval *tv, struct timezone *tz);

#ifdef _BSD
#include <time.h>
#endif

#endif
