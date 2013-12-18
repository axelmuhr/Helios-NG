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
 */

#include "sendmail.h"
#include <signal.h>

#ifndef __HELIOS
#ifndef lint
# ifdef QUEUE
static char sccsid[] = "@(#)queue.c	5.30 (Berkeley) 6/1/90 (with queueing)";
# else /* !QUEUE */
static char sccsid[] = "@(#)queue.c	5.30 (Berkeley) 6/1/90 (without queueing)";
# endif /* QUEUE */
#endif /* not lint */
#else
static char *rcsid = "$Header: /hsrc/tcpip/cmds/sendmail/RCS/queue.c,v 1.3 1993/02/26 14:55:28 paul Exp $";
#endif

#include <sys/stat.h>
#ifndef direct
# include <sys/dir.h>
#endif /* !direct */
#include <errno.h>
#include <pwd.h>

/* 
-- crf: The only routine that I need from this file is queuename(). I have 
-- clobbered everything else ...
*/

/*
**  QUEUENAME -- build a file name in the queue directory for this envelope.
**
**	Assigns an id code if one does not already exist.
**	This code is very careful to avoid trashing existing files
**	under any circumstances.
**
**	Parameters:
**		e -- envelope to build it in/from.
**		type -- the file type, used as the first character
**			of the file name.
**
**	Returns:
**		a pointer to the new file name (in a static buffer).
**
**	Side Effects:
**		Will create the qf file if no id code is
**		already assigned.  This will cause the envelope
**		to be modified.
*/

#if defined __C40 || defined __ARM
char *
queuename(
	  register ENVELOPE *	e,
	  char		    	type )
#else
char *
queuename(e, type)
	register ENVELOPE *e;
	char type;
#endif
{
	static char buf[MAXNAME];
	static int pid = -1;
	char c1 = 'A';
	char c2 = 'A';

#ifdef __HELIOS
	static int rnd_pid = -1 ;
	int get_tmp (void) ;
	if (rnd_pid == -1)
		rnd_pid = get_tmp () ;
#endif

#ifndef __HELIOS
	if (e->e_id == NULL)
#else
	if (e->e_id[0] == (char) NULL)
#endif
	{
		char qf[20];

		/* find a unique id */ /* under Helios ? yeah ... */

#ifndef __HELIOS
		if (pid != getpid())
#else		
		if (pid != rnd_pid)
#endif		
		{
			/* new process -- start back at "AA" */
#ifndef __HELIOS
			pid = getpid();
#else			
			pid = rnd_pid;
#endif			
			c1 = 'A';
			c2 = 'A' - 1;
		}
#ifndef __HELIOS
		(void) sprintf(qf, "qfAA%05d", pid);
#else
		(void) sprintf(qf, "qfAA%04d", pid);
#endif
		while (c1 < '~' || c2 < 'Z')
		{
			int i;

			if (c2 >= 'Z')
			{
				c1++;
				c2 = 'A' - 1;
			}
			qf[2] = c1;
			qf[3] = ++c2;
#ifndef __HELIOS
			if (tTd(7, 20))
				printf("queuename: trying \"%s\"\n", qf);
#endif

			i = open(qf, O_WRONLY|O_CREAT|O_EXCL, FileMode);

			if (i < 0) {
				if (errno != EEXIST) {
					syserr("queuename: Cannot create \"%s\" in \"%s\"",
						qf, QueueDir);
					exit(EX_UNAVAILABLE);
				}
			} else {
				(void) close(i);
				break;
			}
		}
		if (c1 >= '~' && c2 >= 'Z')
		{
			syserr("queuename: Cannot create \"%s\" in \"%s\"",
				qf, QueueDir);
			exit(EX_OSERR);
		}
#ifndef __HELIOS
		e->e_id = newstr(&qf[2]);
#else
		strncpy (e->e_id, qf+2, 6) ;
		debugf ("e_id = %s", e->e_id) ; 
#endif

#ifndef __HELIOS
		define('i', e->e_id, e);
		if (tTd(7, 1))
			printf("queuename: assigned id %s, env=%x\n", e->e_id, e);
#ifdef LOG
		if (LogLevel > 16)
			syslog(LOG_DEBUG, "%s: assigned id", e->e_id);
#endif /* LOG */
#endif			
	}

	if (type == '\0')
		return (NULL);
#ifndef __HELIOS
	(void) sprintf(buf, "%cf%s", type, e->e_id);
#else
	(void) sprintf(buf, "%cf%s", type, e->e_id);
#endif

#ifndef __HELIOS
	if (tTd(7, 2))
		printf("queuename: %s\n", buf);
#endif		
	return (buf);
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
	return signature % 9999 ;
}
#endif
