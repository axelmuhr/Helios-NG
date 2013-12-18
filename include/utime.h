/* utime.h: POSIX set file access and modification times	*/
/* %W% %G% (C) Copyright 1990, Perihelion Software Ltd.		*/
/* $Id: utime.h,v 1.1 90/09/05 11:07:37 nick Exp $ */

#ifndef __utime_h
#define __utime_h

struct utimbuf {
	time_t	ctime;
	time_t	actime;
	time_t	modtime;
};

extern int utime(char *path, struct utimbuf *times);

#endif

/* end of utime.h */
