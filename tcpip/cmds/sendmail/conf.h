/*
 * Copyright (c) 1983 Eric P. Allman
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
 *	@(#)conf.h	5.17 (Berkeley) 6/1/90
 */

/*
**  CONF.H -- All user-configurable parameters for sendmail
*/

/*
static char *rcsid = "$Header: /hsrc/tcpip/cmds/sendmail/RCS/conf.h,v 1.1 1992/01/20 14:46:28 craig Exp $";
*/

/*
**  Table sizes, etc....
**	There shouldn't be much need to change these....
*/

# define MAXLINE	1024		/* max line length */
# define MAXNAME	256		/* max length of a name */
# define MAXFIELD	4096		/* max total length of a hdr field */
# define MAXPV		40		/* max # of parms to mailers */
# define MAXHOP		17		/* max value of HopCount */
# define MAXATOM	200		/* max atoms per address */
# define MAXMAILERS	25		/* maximum mailers known to system */
# define MAXRWSETS	31		/* max # of sets of rewriting rules */
# define MAXPRIORITIES	25		/* max values for Precedence: field */
# define MAXTRUST	30		/* maximum number of trusted users */
# define MAXUSERENVIRON	40		/* max # of items in user environ */
# define QUEUESIZE	600		/* max # of jobs per queue run */
# define MAXMXHOSTS	10		/* max # of MX records */

#ifdef __HELIOS
/*
-- crf: maximum number of recipients
*/
#define MAX_RCPT	1024
#endif

/*
**  Compilation options.
**
**	#define these if they are available; comment them out otherwise.
*/

#ifndef __HELIOS
# define VMUNIX		1	/* running on systems w. 4.2/4.3 networking */
# define DBM		1	/* use DBM library (may require -ldbm) */
/* define only 1 of the various {N,G,S,M}DBM libraries */
# define NDBM		1	/* new DBM library available (requires DBM) */
/*# define GDBM		1*/	/* gnu DBM library available (requires DBM) */
/*# define SDBM		1*/	/* Ozan Yigit's PD ndbm (requires DBM) */
/*# define MDBM		1*/	/* UMaryland's ndbm variant (requires DBM) */
/*# define YP		1*/	/* enable Yellow Pages code */
/*# define FUZZY	1*/	/* enable fuzzy matching of local user names */
# define LOG		1	/* enable logging */
# define SMTP		1	/* enable user and server SMTP */
# define QUEUE		1	/* enable queueing */
# define QUEUE_MACVALUE	'$'	/* save the $r and $s macros in queue file */
# define UGLYUUCP	1	/* output ugly UUCP From lines */
# define DAEMON		1	/* include the daemon (requires IPC & SMTP) */
/*# define MAIL11V3	1*/	/* enable non-standard SMTP mods for DECnet */
# define SETPROCTITLE	1	/* munge argv to display current status */
# define NAMED_BIND	1	/* use Berkeley Internet Domain Server */
# define SIG_TYPE	void	/* SUN's signal() returns void type */
# define VSPRINTF	1	/* have vsprintf() in /lib/libc.a */
/*# define SHARE	1*/	/* Convex share scheduler */
/*# define SYSV		1*/	/* running on a system 5 system */

#else
# define VMUNIX		1	/* running on systems w. 4.2/4.3 networking */
# define LOG		1	/* enable logging */
# define SMTP		1	/* enable user and server SMTP */
# define DAEMON		1	/* include the daemon (requires IPC & SMTP) */
# define SIG_TYPE	void	/* SUN's signal() returns void type */
# define VSPRINTF	1	/* have vsprintf() in /lib/libc.a */

#ifdef NOT_WANTED
# define DBM		1	/* use DBM library (may require -ldbm) */
/* define only 1 of the various {N,G,S,M}DBM libraries */
# define NDBM		1	/* new DBM library available (requires DBM) */
# define GDBM		1	/* gnu DBM library available (requires DBM) */
# define SDBM		1	/* Ozan Yigit's PD ndbm (requires DBM) */
# define MDBM		1	/* UMaryland's ndbm variant (requires DBM) */
# define FUZZY		1	/* enable fuzzy matching of local user names */
# define YP		1	/* enable Yellow Pages code */
# define NAMED_BIND	1	/* use Berkeley Internet Domain Server */
# define UGLYUUCP	1	/* output ugly UUCP From lines */
# define MAIL11V3	1	/* enable non-standard SMTP mods for DECnet */
# define QUEUE		1	/* enable queueing */
# define QUEUE_MACVALUE	'$'	/* save the $r and $s macros in queue file */
# define SETPROCTITLE	1	/* munge argv to display current status */
# define SHARE		1	/* Convex share scheduler */
# define SYSV		1	/* running on a system 5 system */
#endif
#endif

#ifndef __HELIOS
/*
** Use query type of ANY if possible (NO_WILDCARD_MX), which will
** find types CNAME, A, and MX, and will cause all existing records
** to be cached by our local server.  If there is (might be) a
** wildcard MX record in the local domain or its parents that are
** searched, we can't use ANY; it would cause fully-qualified names
** to match as names in a local domain.
*/
# define NO_WILDCARD_MX	1

/*
** Change this to the location where sendmail should read its configuration
** file.  Older systems used /usr/lib/sendmail.cf, some newer systems move
** this file to /etc/sendmail.cf.
*/
# define	_PATH_SENDMAILCF	"/etc/sendmail.cf"

/*
** Comment out the following line if freeze files don't work on your platform.
** Freeze files will not work on the Next, nor under AIX 3.1 unless both
** malloc.c and getpwent.c are compiled with the sendmail source.
*/
# if !defined(NeXT) && !defined(_AIX) && !defined(apollo)
#  define	_PATH_SENDMAILFC	"/etc/sendmail.fc"
# endif /* !NeXT && !_AIX && !apollo */

#endif

/*
** Comment out this line if you don't want sendmail to write a file with
** the daemon's pid.
*/
#ifndef __HELIOS
# define	_PATH_SENDMAILPID   	"/var/run/sendmail.pid"
#else
# define	_PATH_SENDMAILPID   	"/helios/etc/sendmail.pid"
#endif
