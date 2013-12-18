#ifdef XXX_HEADER
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

#ifndef __HELIOS
#ifndef lint
static char sccsid[] = "@(#)headers.c	5.15 (Berkeley) 6/1/90";
#endif /* not lint */
#else
static char *rcsid = "$Header: /hsrc/tcpip/cmds/sendmail/RCS/headers.c,v 1.1 1992/01/20 14:41:08 craig Exp $";
#endif

#include "sendmail.h"

/* new page */
/*
**  PUTHEADER -- put the header part of a message from the in-core copy
**
**	Parameters:
**		fp -- file to put it on.
**		m -- mailer to use.
**		e -- envelope to use.
**
**	Returns:
**		none.
**
**	Side Effects:
**		none.
*/

#ifndef __HELIOS
/*
 * Macro for fast max (not available in e.g. DG/UX, 386/ix).
 */
#ifndef MAX
# define MAX(a,b) (((a)>(b))?(a):(b))
#endif

void
putheader(fp, m, e)
	register FILE *fp;
	register MAILER *m;
	register ENVELOPE *e;
{
	char buf[MAX(MAXFIELD,BUFSIZ)];
	register HDR *h;
	char obuf[MAX(MAXFIELD,MAXLINE)];

	for (h = e->e_header; h != NULL; h = h->h_link)
	{
		register char *p;

		if (bitset(H_CHECK|H_ACHECK, h->h_flags) &&
		    !bitintersect(h->h_mflags, m->m_flags))
			continue;

		/* handle Resent-... headers specially */
		if (bitset(H_RESENT, h->h_flags) && !bitset(EF_RESENT, e->e_flags))
			continue;

		p = h->h_value;
		if (bitset(H_DEFAULT, h->h_flags))
		{
			/* macro expand value if generated internally */
			expand(p, buf, &buf[(sizeof(buf)-1)], e);
			p = buf;
			if (p == NULL || *p == '\0')
				continue;
		}

		if (bitset(H_FROM|H_RCPT, h->h_flags))
		{
			/* address field */
			bool oldstyle = bitset(EF_OLDSTYLE, e->e_flags);

			if (bitset(H_FROM, h->h_flags))
				oldstyle = FALSE;
			commaize(h, p, fp, oldstyle, m);
		}
		else
		{
			/* vanilla header line */
			register char *nlp;

			(void) sprintf(obuf, "%s: ", capitalize(h->h_field));
			while ((nlp = index(p, '\n')) != NULL)
			{
				*nlp = '\0';
				(void) strcat(obuf, p);
				*nlp = '\n';
				putline(obuf, fp, m);
				p = ++nlp;
				obuf[0] = '\0';
			}
			(void) strcat(obuf, p);
			putline(obuf, fp, m);
		}
	}
}
#else
/*
-- crf: **highly** simplified version of putheader
*/

#include <time.h>
#include <pwd.h>

#define MSG_ID_LEN	20

void
putheader(fp, m, e, full)
	register FILE *fp;
	register MAILER *m;
	register ENVELOPE *e;
	bool full ;
{
	char buf[MAXLINE] ;
	struct tm *calender ;
	time_t t ;
	char msg_id_date [MSG_ID_LEN+1] ;
	extern char App_To [MAXNAME] ;

	time (&t) ;
	calender = localtime (&t) ;
	(void) strftime (msg_id_date, MSG_ID_LEN, "%y%m%d%H%M", calender) ;

	if (full)
	{
		(void) sprintf(buf, "From %s %s", 
			       e->e_from, ctime (&t)) ;
		putline(buf, fp, m);
		(void) sprintf(buf, "Return-Path: <%s>", e->e_from) ;
		putline(buf, fp, m);
	}
	(void) sprintf(buf, "Received: by %s (%s)\n\tid %s; %s", 
			     MyHostName, Version_ID(), e->e_id, arpadate()) ;
	putline(buf, fp, m);
	(void) sprintf(buf, "Date: %s", arpadate()) ;
	putline(buf, fp, m);
	{
		struct passwd *passwd_entry ;
		if ((passwd_entry = getpwnam (getlogin())) == NULL)
			syslog (LOG_ERR, "failed to get passwd name: %m") ;
		(void) sprintf(buf, "From: %s (%s)", 
		       e->e_from, passwd_entry->pw_gecos) ;
		putline(buf, fp, m);
	}
	(void) sprintf(buf, "Message-Id: <%s.%s@%s>", 
			     msg_id_date, e->e_id, MyHostName) ;
	putline(buf, fp, m);
	(void) sprintf(buf, "Apparently-To: %s", App_To) ; 
	putline(buf, fp, m);

	(void) fflush(fp);
	if (ferror(fp))
	{
		syserr("putheader: write error");
		ExitStat = EX_IOERR;
	}
	errno = 0;
}
#endif
#endif
