/* sys/param.h: BSD compatibility header				*/
/* SccsId: %W% %G% (C) Copyright 1990, Perihelion Software Ltd.		*/
/* RcsId: $Id: param.h,v 1.5 91/03/11 15:53:29 nickc Exp $ */

#ifdef _BSD

#ifndef __param_h
#define __param_h

#define	BSD	43	/* pretend we are BSD4.3			*/

#include <sys/types.h>

#include <signal.h>

#define MAXPATHLEN 512

#define NCARGS	10240

#define NBBY 8

#define NGROUPS 1

#ifndef NULL
#define NULL	(0)
#endif

#define NOFILE	64

#endif /* __param_h */

#else
#error sys/param.h included without _BSD set
#endif /* _BSD */
