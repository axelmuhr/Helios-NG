/* utsname.h: Posix library O/S info structure				*/
/* SccsId: %W% %G% (C) Copyright 1990, Perihelion Software Ltd.		*/
/* RcsId: $Id: utsname.h,v 1.2 90/10/02 19:01:44 nick Exp $ */

#ifndef _utsname_h
#define _utsname_h

struct utsname {
	char	sysname[12];
	char	nodename[32];
	char	release[12];
	char	version[12];
	char	machine[12];
};

extern int uname(struct utsname *name);

#endif

/* end of utsname.h */
