/*
 * Copyright (c) 1987 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef __HELIOS
#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)mktemp.c	5.7 (Berkeley) 6/27/88";
#endif /* LIBC_SCCS and not lint */
#else
static char *rcsid = "$Header: /hsrc/tcpip/cmds/binmail/RCS/mktemp.c,v 1.1 1992/01/17 10:43:10 craig Exp $";
#endif

#include <sys/types.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdio.h>
#include <ctype.h>

#define	YES	1
#define	NO	0

#ifdef __HELIOS
static int _gettemp(char *, int	*) ;
#endif

#ifdef __HELIOS
int
#endif
mkstemp(as)
	char	*as;
{
	int	fd;

	return (_gettemp(as, &fd) ? fd : -1);
}

#ifndef __HELIOS
char *
mktemp(as)
	char	*as;
{
	return(_gettemp(as, (int *)NULL) ? as : (char *)NULL);
}
#endif

static
_gettemp(as, doopen)
	char	*as;
	register int	*doopen;
{
	extern int	errno;
	register char	*start, *trv;
	struct stat	sbuf;
#ifndef __HELIOS
	u_int	pid;
	pid = getpid();
#else
	static int pid = -1 ;
	static int rnd_pid = -1 ;
	int get_tmp (void) ;

	if (rnd_pid == -1)
		rnd_pid = get_tmp () ;
	if (pid != rnd_pid)
		pid = rnd_pid;
#endif

	/* extra X's get set to 0's */
	for (trv = as; *trv; ++trv);
	while (*--trv == 'X') {
		*trv = (pid % 10) + '0';
		pid /= 10;
	}

	/*
	 * check for write permission on target directory; if you have
	 * six X's and you can't write the directory, this will run for
	 * a *very* long time.
	 */
	for (start = ++trv; trv > as && *trv != '/'; --trv);
	if (*trv == '/') {
		*trv = '\0';
		if (stat(as, &sbuf) || !(sbuf.st_mode & S_IFDIR))
			return(NO);
		*trv = '/';
	}
	else if (stat(".", &sbuf) == -1)
		return(NO);

	for (;;) {
		if (doopen) {
		    if ((*doopen = open(as, O_CREAT|O_EXCL|O_RDWR, 0600)) >= 0)
			return(YES);
		    if (errno != EEXIST)
			return(NO);
		}
		else if (stat(as, &sbuf))
			return(errno == ENOENT ? YES : NO);

		/* tricky little algorithm for backward compatibility */
		for (trv = start;;) {
			if (!*trv)
				return(NO);
			if (*trv == 'z')
				*trv++ = 'a';
			else {
				if (isdigit(*trv))
					*trv = 'a';
				else
					++*trv;
				break;
			}
		}
	}
	/*NOTREACHED*/
}

#ifdef __HELIOS
static int _tmp_file_ser = 0;

int get_tmp (void)
{
/*
-- crf: I've pinched this from tmpnam (stdio.c) 
*/
	int signature = ((int)time(NULL) << 8) | (_tmp_file_ser++ & 0xff);
	signature &= 0x0FFFFFFF;
	return signature % 99999 ;
}
#endif
