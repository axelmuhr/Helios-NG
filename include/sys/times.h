/* sys/times.h: Posix library times structure				*/
/* SccsId: %W% %G% (C) Copyright 1990, Perihelion Software Ltd.		*/
/* RcsId: $Id: times.h,v 1.1 90/09/05 11:09:26 nick Exp $ */

#ifndef _times_h
#define _times_h


struct tms {
	clock_t		tms_utime;
	clock_t		tms_stime;
	clock_t		tms_cutime;
	clock_t		tms_cstime;
};

extern clock_t times(struct tms *t);

#endif

/* end of sys/times.h */
