/*
 * Copyright (c) 1988 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted provided
 * that: (1) source distributions retain this entire copyright notice and
 * comment, and (2) distributions including binaries display the following
 * acknowledgement:  ``This product includes software developed by the
 * University of California, Berkeley and its contributors'' in the
 * documentation or other materials provided with the distribution and in
 * all advertising materials mentioning features or use of this software.
 * Neither the name of the University nor the names of its contributors may
 * be used to endorse or promote products derived from this software without
 * specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 *	@(#)useful.h	4.6 (Berkeley) 6/1/90
 */

#ifndef __HELIOS
#ifndef lint
# ifdef _DEFINE
static char useful_h_sccsid[] = "@(#)useful.h	4.6 (Berkeley) 6/1/9";
# endif /* _DEFINE */
#endif /* !lint */
#endif

/*
static char *rcsid = "$Header: /hsrc/tcpip/cmds/sendmail/RCS/useful.h,v 1.1 1992/01/20 14:46:28 craig Exp $";
*/

#include <sys/types.h>

/* support for bool type.  going from char to int increased BSS by 16 bytes */
#ifndef __helios_h
typedef int	bool;
#define TRUE	1
#define FALSE	0
#endif

#ifndef NULL
# define NULL	0
#endif /* NULL */

#ifndef __HELIOS
/* bit hacking */
#define bitset(bit, word)	(((word) & (bit)) != 0)

/* some simple functions */
#ifndef max
# define max(a, b)	((a) > (b) ? (a) : (b))
# define min(a, b)	((a) < (b) ? (a) : (b))
#endif /* !max */

/* assertions */
#ifndef NASSERT
# define ASSERT(expr, msg, parm)\
	if (!(expr))\
	{\
		fprintf(stderr, "assertion botch: %s:%d: ", __FILE__, __LINE__);\
		fprintf(stderr, msg, parm);\
	}
#else /* !NASSERT */
# define ASSERT(expr, msg, parm)
#endif /* NASSERT */

/* sccs id's */
#ifndef lint
# define SCCSID(arg)	static char SccsId[] = "arg";
#else /* lint */
# define SCCSID(arg)
#endif /* !lint */
#endif

/* define the types of some common functions */
#ifdef __STDC__
# include <string.h>
# include <unistd.h>
# include <stdlib.h>
#else /* !__STDC__ */
extern char	*strcpy(), *strncpy();
extern char	*strcat(), *strncat();
extern char	*malloc();
extern time_t	time();
extern char	*ctime();
extern char	*getenv();
#endif /* __STDC__ */
extern int	errno;

#ifndef SYSV
#ifndef __HELIOS
extern char	*index(), *rindex();
#else
extern char	*index(char *, char), *rindex(char *, char);
#endif
#else /* SYSV */
# include <fcntl.h>
# define	index		strchr
# define	rindex		strrchr
# define	bcopy(h,a,l)	memcpy(a,h,l)
# define	bzero(s,n)	memset(s,0,n)
# define	bcmp		memcmp
# define	vfork		fork
# ifndef NOFILE
#  include <sys/param.h>
# endif /* NOFILE */
# define	getdtablesize()	NOFILE
# ifndef SIGCHLD
#  define	SIGCHLD		SIGCLD
# endif /* !SIGCHLD */
# define	direct		dirent
# include <dirent.h>
extern	char	*strchr(),	*strrchr();
# include <grp.h>
# ifndef NGROUPS_MAX
#  define	initgroups(u,g)	kill(0,0)
# endif /* !NGROUPS_MAX */
# ifndef MAX
#  define	MAX(a,b)	((a)>(b))?(a):(b)
# endif /* !MAX */
#endif /* !SYSV */

#ifdef __HELIOS
#define SETUP_SIG_HANDLER 				\
	{ 						\
		struct sigaction act ;			\
		act.sa_handler = intsig ;		\
		act.sa_mask = 0 ;			\
		act.sa_flags = SA_ASYNC ;		\
		(void) sigaction(SIGINT, &act, NULL) ;	\
		(void) sigaction(SIGHUP, &act, NULL) ;	\
		(void) sigaction(SIGQUIT, &act, NULL) ;	\
		(void) sigaction(SIGTERM, &act, NULL) ;	\
	}
#endif
