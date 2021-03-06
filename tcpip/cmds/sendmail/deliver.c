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
static char sccsid[] = "@(#)deliver.c	5.38 (Berkeley) 6/1/90";
#endif /* not lint */
#else
static char *rcsid = "$Header: /hsrc/tcpip/cmds/sendmail/RCS/deliver.c,v 1.5 1992/02/27 15:40:35 craig Exp $";
#endif

#include "sendmail.h"
#include <sys/signal.h>
#include <sys/stat.h>
#ifndef __HELIOS
#include <netdb.h>
#endif
#include <fcntl.h>
#include <errno.h>
#ifdef NAMED_BIND
# include <arpa/nameser.h>
# include <resolv.h>
#endif  /* NAMED_BIND */
#if defined(__convex__) && defined(SHARE)
# include <shares.h>
int setupshares(int, void (*func)());
#endif /* __convex__ && SHARE */

#ifndef __HELIOS
#ifdef __STDC__
static mailfile(char *, ADDRESS *);
static void markfailure(ENVELOPE *, ADDRESS *, int);
static sendoff(ENVELOPE *, MAILER *, char **, ADDRESS *);
#else /* !__STDC__ */
static mailfile();
static void markfailure();
static sendoff();
#endif /* __STDC__ */
#else
static sendoff(ENVELOPE *, MAILER *, char **);
#ifdef XXX_HEADER
char App_To [MAXNAME] ;
bool local_sender = FALSE ;
#endif
#endif

/*
**  Status error messages
*/
#define MAXENDERR	(sizeof(Enderr) / sizeof(*Enderr))
char *Enderr[] = {
	"IMPOSSIBLE",
	/* SIGHUP */	"hangup",
	/* SIGINT */	"interrupt",
	/* SIGQUIT */	"quit",
	/* SIGILL */	"illegal instruction",
	/* SIGTRAP */	"trace trap",
	/* SIGIOT */	"IOT instruction",
	/* SIGEMT */	"EMT instruction",
	/* SIGFPE */	"floating point exception",
	/* SIGKILL */	"kill",
	/* SIGBUS */	"bus error",
	/* SIGSEGV */	"segmentation violation",
	/* SIGSYS */	"bad argument to system call",
	/* SIGPIPE */	"write on a pipe with no one to read it",
	/* SIGALRM */	"alarm clock",
	/* SIGTERM */	"software termination signal",
	/* SIGURG */	"urgent condition present on socket",
	/* SIGSTOP */	"stop",
	/* SIGTSTP */	"stop signal generated from keyboard",
	/* SIGCONT */	"continue after stop",
	/* SIGCHLD */	"child status has changed",
	/* SIGTTIN */	"background read attempted from control terminal",
	/* SIGTTOU */	"background write attempted to control terminal",
	/* SIGIO */	"I/O is possible on a descriptor",
	/* SIGXCPU */	"cpu time limit exceeded",
	/* SIGXFSZ */	"file size limit exceeded",
	/* SIGVTALRM */	"virtual time alarm",
	/* SIGPROF */	"profiling timer alarm",
	/* SIGWINCH */	"window changed",
	/* SIGLOST */	"resource lost",
	/* SIGUSR1 */	"user-defined signal 1",
	/* SIGUSR2 */	"user-defined signal 2"
};

#ifdef	NAMED_BIND
/*
**  Name server error messages
*/
# define MAXH_ERR		(sizeof(H_Errmsg) / sizeof(*H_Errmsg))
char *H_Errmsg[] = {
	/* XXX */		"[Unknown error]",
	/* HOST_NOT_FOUND */	"Authoritative answer from name server",
	/* TRY_AGAIN */		"Non-authoritiatve answer or name server failure",
	/* NO_RECOVERY */	"Non recoverable name server error",
	/* NO_DATA */		"Valid name but no data [address]"
};
#endif	/* NAMED_BIND */

/*
**  DELIVER -- Deliver a message to a list of addresses.
**
**	This routine delivers to everyone on the same host as the
**	user on the head of the list.  It is clever about mailers
**	that don't handle multiple users.  It is NOT guaranteed
**	that it will deliver to all these addresses however -- so
**	deliver should be called once for each address on the
**	list.
**
**	Parameters:
**		e -- the envelope to deliver.
**		firstto -- head of the address list to deliver to.
**
**	Returns:
**		zero -- successfully delivered.
**		else -- some failure, see ExitStat for more info.
**
**	Side Effects:
**		The standard input is passed off to someone.
*/

#ifndef __HELIOS
deliver(e, firstto)
	register ENVELOPE *e;
	ADDRESS *firstto;
#else
int deliver(register ENVELOPE *e)
#endif	
{
#ifndef __HELIOS
	char *user;			/* user being sent to */
	char **pvp;
	register char **mvp;
	register char *p;
	register MAILER *m;		/* mailer for this recipient */
	ADDRESS *ctladdr;
	register ADDRESS *to = firstto;
	ADDRESS *tochain = NULL;	/* chain of users in this mailer call */
	char *pv[MAXPV+1];
	char tobuf[MAXLINE-50];		/* text line of to people */
	char buf[MAXNAME];
	char tfrombuf[MAXNAME];		/* translated from person */
#else
	char *pv[6];
	MAILER New_Mailer ;		/* mailer for this recipient */
	register MAILER *m = &New_Mailer;
#endif
	char *host;			/* host being sent to */
	bool clever = FALSE;		/* running user smtp to this mailer */
	int rcode;			/* response code */

	errno = 0;

#ifdef __HELIOS
	Init_Mailer (m) ;

#define CUT_OFF 55
#define LOG_REQ(label, e_name) \
	strcpy (buf, e_name) ; \
	if (strlen (buf) > CUT_OFF) \
		strcpy (&buf[CUT_OFF], "...") ; \
	syslog (LOG_INFO, "%s:%s", label, buf) \

	if (LogRequests)
	{
		char buf [MAXNAME] ;
		LOG_REQ ("From", e->e_from) ;
		LOG_REQ ("Rcpt", e->e_to) ;
	}

#ifdef XXX_HEADER
	local_sender = local_name (e->e_from) ;
	strcpy (App_To, e->e_to) ;
#endif
	if (local_name (e->e_to))
	{
		strcpy (m->m_name,   LOCAL_M_NAME) ;
		strcpy (m->m_mailer, LOCAL_MAILER) ;
		clever = FALSE ;
		pv[0] = MAIL_NAME ; pv[1] = "-r" ;
		pv[2] = e->e_from ; pv[3] = "-d" ;
		pv[4] = e->e_to   ; pv[5] = NULL ;
	}
	else
	{
		strcpy (m->m_name,   ETHER_M_NAME) ;
		strcpy (m->m_mailer, ETHER_MAILER) ;
/*
-- crf: cludge from address if sending out
*/		
#ifdef XXX_HEADER
		if (local_sender)
#else
		if (local_name (e->e_from))
#endif
		{
			strcat (e->e_from, "@") ;
			strcat (e->e_from, MyHostName) ;
		}
		host = MAIL_HOST ;
		clever = TRUE ;
	}
#endif

#ifndef __HELIOS
	if (bitset(QDONTSEND, to->q_flags))
		return (0);

#ifdef NAMED_BIND
	/* unless interactive, try twice, over a minute */
	if (OpMode == MD_DAEMON || OpMode == MD_SMTP) {
		_res.retrans = 30;
		_res.retry = 2;
	}
#endif  /* NAMED_BIND */

	m = to->q_mailer;
	host = to->q_host;

	if (tTd(10, 1))
		printf("\n--deliver, mailer=%d, host=`%s', first user=`%s'\n",
			m->m_mno, host, to->q_user);

	/*
	**  If this mailer is expensive, and if we don't want to make
	**  connections now, just mark these addresses and return.
	**	This is useful if we want to batch connections to
	**	reduce load.  This will cause the messages to be
	**	queued up, and a daemon will come along to send the
	**	messages later.
	**		This should be on a per-mailer basis.
	*/

	if (NoConnect && !QueueRun && bitnset(M_EXPENSIVE, m->m_flags) &&
	    !Verbose)
	{
		for (; to != NULL; to = to->q_next)
		{
			if (bitset(QDONTSEND, to->q_flags) || to->q_mailer != m)
				continue;
			to->q_flags |= QQUEUEUP|QDONTSEND;
			e->e_to = to->q_paddr;
			message(Arpa_Info, "queued");
			if (LogLevel > 4)
				logdelivery("queued");
		}
		e->e_to = NULL;
		return (0);
	}

	/*
	**  Do initial argv setup.
	**	Insert the mailer name.  Notice that $x expansion is
	**	NOT done on the mailer name.  Then, if the mailer has
	**	a picky -f flag, we insert it as appropriate.  This
	**	code does not check for 'pv' overflow; this places a
	**	manifest lower limit of 4 for MAXPV.
	**		The from address rewrite is expected to make
	**		the address relative to the other end.
	*/

	/* rewrite from address, using rewriting rules */
	expand("\001f", buf, &buf[sizeof buf - 1], e);
	(void) strcpy(tfrombuf, remotename(buf, m, TRUE, TRUE, FALSE));

	define('g', tfrombuf, e);		/* translated sender address */
	define('h', host, e);			/* to host */
	Errors = 0;
	pvp = pv;
	*pvp++ = m->m_argv[0];

	/* insert -f or -r flag as appropriate */
	if (FromFlag && (bitnset(M_FOPT, m->m_flags) || bitnset(M_ROPT, m->m_flags)))
	{
		if (bitnset(M_FOPT, m->m_flags))
			*pvp++ = "-f";
		else
			*pvp++ = "-r";
		expand("\001g", buf, &buf[sizeof buf - 1], e);
		*pvp++ = newstr(buf);
	}

	/*
	**  Append the other fixed parts of the argv.  These run
	**  up to the first entry containing "$u".  There can only
	**  be one of these, and there are only a few more slots
	**  in the pv after it.
	*/

	for (mvp = m->m_argv; (p = *++mvp) != NULL; )
	{
		while ((p = index(p, '\001')) != NULL)
			if (*++p == 'u')
				break;
		if (p != NULL)
			break;

		/* this entry is safe -- go ahead and process it */
		expand(*mvp, buf, &buf[sizeof buf - 1], e);
		*pvp++ = newstr(buf);
		if (pvp >= &pv[MAXPV - 3])
		{
			syserr("Too many parameters to %s before $u", pv[0]);
			return (-1);
		}
	}

	/*
	**  If we have no substitution for the user name in the argument
	**  list, we know that we must supply the names otherwise -- and
	**  SMTP is the answer!!
	*/

	if (*mvp == NULL)
	{
		/* running SMTP */
#ifdef SMTP
		clever = TRUE;
		*pvp = NULL;
#else /* !SMTP */
		/* oops!  we don't implement SMTP */
		syserr("SMTP style mailer");
		return (EX_SOFTWARE);
#endif /* SMTP */
	}

	/*
	**  At this point *mvp points to the argument with $u.  We
	**  run through our address list and append all the addresses
	**  we can.  If we run out of space, do not fret!  We can
	**  always send another copy later.
	*/

	tobuf[0] = '\0';
	e->e_to = tobuf;
	ctladdr = NULL;
	for (; to != NULL; to = to->q_next)
	{
		/* avoid sending multiple recipients to dumb mailers */
		if (tobuf[0] != '\0' && !bitnset(M_MUSER, m->m_flags))
			break;

		/* if already sent or not for this host, don't send */
		if (bitset(QDONTSEND, to->q_flags) ||
		    strcmp(to->q_host, host) != 0 ||
		    to->q_mailer != firstto->q_mailer)
			continue;

		/* avoid overflowing tobuf */
		if (sizeof tobuf < (strlen(to->q_paddr) + strlen(tobuf) + 2))
			break;

		if (tTd(10, 1))
		{
			printf("\nsend to ");
			printaddr(to, FALSE);
		}

		/* compute effective uid/gid when sending */
		if (to->q_mailer == ProgMailer)
			ctladdr = getctladdr(to);

		user = to->q_user;
		e->e_to = to->q_paddr;
		to->q_flags |= QDONTSEND;

		/*
		**  Check to see that these people are allowed to
		**  talk to each other.
		*/

		if (m->m_maxsize != 0 && e->e_msgsize > m->m_maxsize)
		{
			NoReturn = TRUE;
			usrerr("Message is too large; %ld bytes max", m->m_maxsize);
			giveresponse(EX_UNAVAILABLE, m, e);
			continue;
		}
		if (!checkcompat(to))
		{
			giveresponse(EX_UNAVAILABLE, m, e);
			continue;
		}

		/*
		**  Strip quote bits from names if the mailer is dumb
		**	about them.
		*/

		if (bitnset(M_STRIPQ, m->m_flags))
		{
			stripquotes(user, TRUE);
			stripquotes(host, TRUE);
		}
		else
		{
			stripquotes(user, FALSE);
			stripquotes(host, FALSE);
		}

		/* hack attack -- delivermail compatibility */
		if (m == ProgMailer && *user == '|')
			user++;

		/*
		**  If an error message has already been given, don't
		**	bother to send to this address.
		**
		**	>>>>>>>>>> This clause assumes that the local mailer
		**	>> NOTE >> cannot do any further aliasing; that
		**	>>>>>>>>>> function is subsumed by sendmail.
		*/

		if (bitset(QBADADDR|QQUEUEUP, to->q_flags))
			continue;

		/* save statistics.... */
		markstats(e, to);

		/*
		**  See if this user name is "special".
		**	If the user name has a slash in it, assume that this
		**	is a file -- send it off without further ado.  Note
		**	that this type of addresses is not processed along
		**	with the others, so we fudge on the To person.
		*/

		if (m == LocalMailer)
		{
			if (user[0] == '/')
			{
				rcode = mailfile(user, getctladdr(to));
				giveresponse(rcode, m, e);
				continue;
			}
		}

		/*
		**  Address is verified -- add this user to mailer
		**  argv, and add it to the print list of recipients.
		*/

		/* link together the chain of recipients */
		to->q_tchain = tochain;
		tochain = to;

		/* create list of users for error messages */
		(void) strcat(tobuf, ",");
		(void) strcat(tobuf, to->q_paddr);
		define('u', user, e);		/* to user */
		define('z', to->q_home, e);	/* user's home */

		/*
		**  Expand out this user into argument list.
		*/

		if (!clever)
		{
			expand(*mvp, buf, &buf[sizeof buf - 1], e);
			*pvp++ = newstr(buf);
			if (pvp >= &pv[MAXPV - 2])
			{
				/* allow some space for trailing parms */
				break;
			}
		}
	}

	/* see if any addresses still exist */
	if (tobuf[0] == '\0')
	{
		define('g', (char *) NULL, e);
		return (0);
	}

	/* print out messages as full list */
	e->e_to = tobuf + 1;

	/*
	**  Fill out any parameters after the $u parameter.
	*/

	while (!clever && *++mvp != NULL)
	{
		expand(*mvp, buf, &buf[sizeof buf - 1], e);
		*pvp++ = newstr(buf);
		if (pvp >= &pv[MAXPV])
			syserr("deliver: pv overflow after $u for %s", pv[0]);
	}
	*pvp++ = NULL;

	/*
	**  Call the mailer.
	**	The argument vector gets built, pipes
	**	are created as necessary, and we fork & exec as
	**	appropriate.
	**	If we are running SMTP, we just need to clean up.
	*/

	if (ctladdr == NULL)
		ctladdr = &e->e_from;
#ifdef NAMED_BIND
	_res.options &= ~(RES_DEFNAMES | RES_DNSRCH);	/* XXX */
#endif  /* NAMED_BIND */
#endif
#ifdef SMTP
	if (clever)
	{
		rcode = EX_OK;
# ifdef NAMED_BIND
		/*
		** Don't do MX lookups on domain literals or for non-IPC
		** mailers.
		*/ 
		if (host[0] && host[0] != '[' && *m->m_mailer != '/')
		{
			/* Compare MX records against $j and not $w */
			expand("\001j", buf, &buf[sizeof(buf) - 1], e);
			Nmx = getmxrr(host, MxHosts, buf, &rcode);
		}
		else
# endif  /* NAMED_BIND */
		{
			Nmx = 1;
			MxHosts[0] = host;
		}
		if (Nmx >= 0)
		{
			message(Arpa_Info, "Connecting to %s (%s)...",
			    MxHosts[0], m->m_name);

#ifndef __HELIOS
# ifdef MAIL11V3
			if ((rcode = smtpinit(m, pv, e)) == EX_OK)
# else /* ! MAIL11V3 */
			if ((rcode = smtpinit(m, pv)) == EX_OK)
# endif /* MAIL11V3 */
			{
				register char *t = tobuf;
				register int i;

				/* send the recipient list */
				tobuf[0] = '\0';
				for (to = tochain; to; to = to->q_tchain)
				{
					e->e_to = to->q_paddr;
					if ((i = smtprcpt(to, m)) != EX_OK)
					{
						markfailure(e, to, i);
						giveresponse(i, m, e);
					}
					else
					{
						*t++ = ',';
						for (p = to->q_paddr; *p; *t++ = *p++);
					}
				}

				/* now send the data */
				if (tobuf[0] == '\0')
					e->e_to = NULL;
				else
				{
					e->e_to = tobuf + 1;
					rcode = smtpdata(m, e);
# ifdef MAIL11V3
					SmtpPhase = "result wait";
					setproctitle("%s %s: %s", CurEnv->e_id, CurHostName, SmtpPhase);
					if (rcode == EX_OK)
					{
						if (!SmtpManyStatus)
							rcode = smtpstat(m);
						else for (tobuf[0] = '\0',
						    to = tochain; to != NULL;
						    to = to->q_tchain) {
							int j;

							e->e_to = to->q_paddr;
							j = smtpstat(m);
							if (j != EX_OK)
							{
								markfailure(e, to, j);
								giveresponse(j, m, e);
							}
							else
							{
								(void) strcat(tobuf, ",");
								(void) strcat(tobuf, to->q_paddr);
							}
						}
					}
# endif /* MAIL11V3 */
				}

				/* now close the connection */
				smtpquit(m);
#else
			if ((rcode = smtpinit(m, pv)) == EX_OK)
			{
				register int i;

				if ((i = smtprcpt(m)) != EX_OK)
					giveresponse(i, e);
				else
					debugf ("rcpt OK") ;

				/* now send the data */
				if (e->e_to[0] == (char) NULL)
					syserr ("Empty destination address") ;
				else
					rcode = smtpdata(m);

				/* now close the connection */
				smtpquit(m);
#endif				
			}
		}
	}
	else
#endif /* SMTP */
	{
#ifndef __HELIOS
		message(Arpa_Info, "Connecting to %s (%s)...", host, m->m_name);
		rcode = sendoff(e, m, pv, ctladdr);
#else
		message(Arpa_Info, "Connecting ... (%s)", m->m_name);
		rcode = sendoff(e, m, pv);
#endif	
	}
#ifdef NAMED_BIND
	_res.options |= RES_DEFNAMES | RES_DNSRCH;	/* XXX */
#endif  /* NAMED_BIND */

	/*
	**  Do final status disposal.
	**	We check for something in tobuf for the SMTP case.
	**	If we got a temporary failure, arrange to queue the
	**		addressees.
	*/

#ifndef __HELIOS
	if (tobuf[0] != '\0')
		giveresponse(rcode, m, e);
	if (rcode != EX_OK)
		for (to = tochain; to != NULL; to = to->q_tchain)
			markfailure(e, to, rcode);

	errno = 0;
	define('g', (char *) NULL, e);
#endif	

#ifndef __HELIOS
	return (rcode);
#else
/*
-- crf: need to set transcript file pointer to end of file to avoid 
-- overwriting anything that binmail may have put there
*/
	if (fseek (e->e_xfp, 0L, SEEK_END) != 0)
		syslog (LOG_WARNING, "failed to fseek (transcript file)") ;
	giveresponse(rcode, e);
	return (rcode);
#endif
	
}

#ifndef __HELIOS
/* new page */
/*
**  MARKFAILURE -- mark a failure on a specific address.
**
**	Parameters:
**		e -- the envelope we are sending.
**		q -- the address to mark.
**		rcode -- the code signifying the particular failure.
**
**	Returns:
**		none.
**
**	Side Effects:
**		marks the address (and possibly the envelope) with the
**			failure so that an error will be returned or
**			the message will be queued, as appropriate.
*/

static void
markfailure(e, q, rcode)
	register ENVELOPE *e;
	register ADDRESS *q;
	int rcode;
{
	if (rcode == EX_OK)
		return;
	else if (rcode != EX_TEMPFAIL && rcode != EX_IOERR && rcode != EX_OSERR)
		q->q_flags |= QBADADDR;
	else if (curtime() > e->e_ctime + TimeOut)
	{
		char buf[MAXLINE];

		if (!bitset(EF_TIMEOUT, e->e_flags))
		{
			(void) sprintf(buf, "Cannot send message for %s",
				pintvl(TimeOut, FALSE));
			if (e->e_message != NULL)
				free(e->e_message);
			e->e_message = newstr(buf);
			message(Arpa_Info, buf);
		}
		q->q_flags |= QBADADDR;
		e->e_flags |= EF_TIMEOUT;
	}
	else
		q->q_flags |= QQUEUEUP;
}
/* new page */
/*
**  DOFORK -- do a fork, retrying a couple of times on failure.
**
**	This MUST be a macro, since after a vfork we are running
**	two processes on the same stack!!!
**
**	Parameters:
**		none.
**
**	Returns:
**		From a macro???  You've got to be kidding!
**
**	Side Effects:
**		Modifies the ==> LOCAL <== variable 'pid', leaving:
**			pid of child in parent, zero in child.
**			-1 on unrecoverable error.
**
**	Notes:
**		I'm awfully sorry this looks so awful.  That's
**		vfork for you.....
*/

#define NFORKTRIES	5
#ifdef VMUNIX
# define XFORK	vfork
#else /* !VMUNIX */
# define XFORK	fork
#endif /* VMUNIX */

#define DOFORK(fORKfN) \
{\
	register int i;\
\
	for (i = NFORKTRIES; --i >= 0; )\
	{\
		pid = fORKfN();\
		if (pid >= 0)\
			break;\
		if (i > 0)\
			Xsleep((unsigned) NFORKTRIES - i);\
	}\
}
/* new page */
/*
**  DOFORK -- simple fork interface to DOFORK.
**
**	Parameters:
**		none.
**
**	Returns:
**		pid of child in parent.
**		zero in child.
**		-1 on error.
**
**	Side Effects:
**		returns twice, once in parent and once in child.
*/

dofork()
{
	register int pid = 0;

	DOFORK(fork);
	return (pid);
}
#endif

/* new page */
/*
**  SENDOFF -- send off call to mailer & collect response.
**
**	Parameters:
**		e -- the envelope to mail.
**		m -- mailer descriptor.
**		pvp -- parameter vector to send to it.
**		ctladdr -- an address pointer controlling the
**			user/groupid etc. of the mailer.
**
**	Returns:
**		exit status of mailer.
**
**	Side Effects:
**		none.
*/
static
#ifndef __HELIOS
sendoff(e, m, pvp, ctladdr)
#else
sendoff(e, m, pvp)
#endif
	register ENVELOPE *e;
	MAILER *m;
	char **pvp;
#ifndef __HELIOS
	ADDRESS *ctladdr;
#endif	
{
	auto FILE *mfile;
	auto FILE *rfile;
	register int i;
	int pid;

	/*
	**  Create connection to mailer.
	*/

#ifndef __HELIOS
	pid = openmailer(m, pvp, ctladdr, FALSE, &mfile, &rfile);
#else
	pid = openmailer(m, pvp, FALSE, &mfile, &rfile);
#endif	
	if (pid < 0)
		return (-1);

	/*
	**  Format and send message.
	*/

#ifndef __HELIOS
	putfromline(mfile, m);
	(*e->e_puthdr)(mfile, m, e);
	putline("", mfile, m);
	(*e->e_putbody)(mfile, m, e);
#else

#ifdef XXX_HEADER
/*
-- crf: more cludging. Only want to put in a header if the sender is local
*/	
	debugf ("Cludging header ...") ;
	if (local_sender)
	{
		putheader (mfile, m, e, TRUE);
		putline("", mfile, m);
	}
#endif
	putbody (mfile, m, e);
#endif
	(void) fclose(mfile);
	if (rfile != NULL)
		(void) fclose(rfile);

	i = endmailer(pid, pvp[0]);

#ifndef __HELIOS
	/* arrange a return receipt if requested */
	if (e->e_receiptto != NULL && bitnset(M_LOCAL, m->m_flags))
	{
		e->e_flags |= EF_SENDRECEIPT;
		/* do we want to send back more info? */
	}
#endif

	return (i);
}

/* new page */
/*
**  ENDMAILER -- Wait for mailer to terminate.
**
**	We should never get fatal errors (e.g., segmentation
**	violation), so we report those specially.  For other
**	errors, we choose a status message (into statmsg),
**	and if it represents an error, we print it.
**
**	Parameters:
**		pid -- pid of mailer.
**		name -- name of mailer (for error messages).
**
**	Returns:
**		exit code of mailer.
**
**	Side Effects:
**		none.
*/

int endmailer(pid, name)
	int pid;
	const char *name;
{
	int st;

	/* in the IPC case there is nothing to wait for */
	if (pid == 0)
		return (EX_OK);

	/* wait for the mailer process to die and collect status */
	st = waitfor(pid);
	if (st == -1)
	{
		syserr("endmailer %s: wait", name);
		return (EX_SOFTWARE);
	}

	/* see if it died a horrid death */
	if ((st & 0377) != 0)
	{
		syserr("%s died because of %s (%d)--requeueing message",
		    name, ((st >= 0) && (st < MAXENDERR)) ?
		    Enderr[st] : "unknown error code", st);
		ExitStat = EX_TEMPFAIL;
		return (EX_TEMPFAIL);
	}

	/* normal death -- return status */
	st = (st >> 8) & 0377;
	return (st);
}

/* new page */
/*
**  OPENMAILER -- open connection to mailer.
**
**	Parameters:
**		m -- mailer descriptor.
**		pvp -- parameter vector to pass to mailer.
**		ctladdr -- controlling address for user.
**		clever -- create a full duplex connection.
**		pmfile -- pointer to mfile (to mailer) connection.
**		prfile -- pointer to rfile (from mailer) connection.
**
**	Returns:
**		pid of mailer ( > 0 ).
**		-1 on error.
**		zero on an IPC connection.
**
**	Side Effects:
**		creates a mailer in a subprocess.
*/

#ifndef __HELIOS
openmailer(m, pvp, ctladdr, clever, pmfile, prfile)
#else
int openmailer(m, pvp, clever, pmfile, prfile)
#endif
	MAILER *m;
	char **pvp;
#ifndef __HELIOS
	ADDRESS *ctladdr;
#endif
	bool clever;
	FILE **pmfile;
	FILE **prfile;
{
	int pid = 0;
	int mpvect[2];
	int rpvect[2];
	FILE *mfile, *rfile;
#ifdef __HELIOS
	extern int getdtablesize(void);
#endif

#ifndef __HELIOS
	if (tTd(11, 1))
	{
		printf("openmailer:");
		printav(pvp);
	}
#endif	
	errno = 0;

	CurHostName = m->m_mailer;

	/*
	**  Deal with the special case of mail handled through an IPC
	**  connection.
	**	In this case we don't actually fork.  We must be
	**	running SMTP for this to work.  We will return a
	**	zero pid to indicate that we are running IPC.
	**  We also handle a debug version that just talks to stdin/out.
	*/

#ifndef __HELIOS
	/* check for Local Person Communication -- not for mortals!!! */
	if (strcmp(m->m_mailer, "[LPC]") == 0)
	{
		*pmfile = stdout;
		*prfile = stdin;
		return (0);
	}
#endif

	if (strcmp(m->m_mailer, "[IPC]") == 0)
	{
#ifdef HOSTINFO
		register STAB *st;
#endif /* HOSTINFO */
#ifdef DAEMON
		register int i, j;
		register u_short port;

		CurHostName = pvp[1];
		if (!clever)
			syserr("non-clever IPC");
		if (pvp[2] != NULL)
			port = atoi(pvp[2]);
		else
			port = 0;
		for (j = 0; j < Nmx; j++)
		{
			CurHostName = MxHosts[j];
# ifdef HOSTINFO
		/* see if we have already determined that this host is fried */
			st = stab(MxHosts[j], ST_HOST, ST_FIND);
			if (st == NULL || st->s_host.ho_exitstat == EX_OK) {
				if (j > 0)
					message(Arpa_Info,
					    "Connecting to %s (%s)...",
					    MxHosts[j], m->m_name);
				i = makeconnection(MxHosts[j], port, pmfile, prfile);
			}
			else
			{
				i = st->s_host.ho_exitstat;
				errno = st->s_host.ho_errno;
			}
# else /* !HOSTINFO */
			i = makeconnection(MxHosts[j], port, pmfile, prfile);
# endif /* HOSTINFO */
			if (i != EX_OK)
			{
				/*
				 * Consider the case of multiple MX entries
				 * for a given host where the last entry refers
				 * to non-existent host.  On occasions when
				 * none of the hosts are reachable, the mail
				 * will bounce if the last ExitStat is
				 * EX_NOHOST.  Handle this by resetting i to
				 * EX_TEMPFAIL if it's not the primary MX entry
				 * and it's the last MX entry.  -pbp
				 */
#ifndef __HELIOS
# ifdef LOG
				if (i == EX_NOHOST)
					syslog(LOG_WARNING, "Found non-existent host %s in MX records for %s", CurHostName, pvp[1]);
# endif /* LOG */
#else
/*
-- crf: 'mailhost' alias is not set up in hosts database - can't recover from
-- this, so have to return error
*/
				if (i == EX_NOHOST)
				{
					syserr("Unable to identify %s", MAIL_HOST);
/*
-- set exit status (used by sysexits.c)
-- 78 CONFIG "554 Local configuration error",
*/
					ExitStat = 78; 
					return (-1) ;
				}
#endif
#ifndef __HELIOS
				if (j != 0 && j == (Nmx - 1))
					i = EX_TEMPFAIL;
# ifdef HOSTINFO
				/* enter status of this host */
				if (st == NULL)
					st = stab(MxHosts[j], ST_HOST, ST_ENTER);
				st->s_host.ho_exitstat = i;
				st->s_host.ho_errno = errno;
# endif /* HOSTINFO */
#endif				
				ExitStat = i;
				continue;
			}
			else
				return (0);
		}
#ifdef __HELIOS
/*
-- crf: if this happens, mailhost is down or is not running sendmail daemon. 
-- User should be told what is going on. Would prefer to use syserr(), but 
-- have problems writing to transcript file in the case when conneciton times 
-- out ...
*/
		usrerr ("Connection failed") ;
#endif
		return (-1);
#else /* !DAEMON */
		syserr("openmailer: no IPC");
		return (-1);
#endif /* DAEMON */
	}

	/* create a pipe to shove the mail through */
	if (pipe(mpvect) < 0)
	{
		syserr("openmailer: pipe (to mailer)");
		return (-1);
	}

#ifdef SMTP
	/* if this mailer speaks smtp, create a return pipe */
	if (clever && pipe(rpvect) < 0)
	{
		syserr("openmailer: pipe (from mailer)");
		(void) close(mpvect[0]);
		(void) close(mpvect[1]);
		return (-1);
	}
#endif /* SMTP */

	/*
	**  Actually fork the mailer process.
	**	DOFORK is clever about retrying.
	**
	**	Dispose of SIGCHLD signal catchers that may be laying
	**	around so that endmail will get it.
	*/

	if (CurEnv->e_xfp != NULL)
		(void) fflush(CurEnv->e_xfp);		/* for debugging */
	(void) fflush(stdout);
#ifdef SIGCHLD
	(void) signal(SIGCHLD, SIG_DFL);
#endif /* SIGCHLD */

#ifndef __HELIOS
	DOFORK(XFORK);
	/* pid is set by DOFORK */
	if (pid > 0 && tTd(4, 2))
		printf("openmailer: forking (pid = %d)\n", pid);
#else
	pid = vfork() ;
#endif
		
	if (pid < 0)
	{
		syslog (LOG_ERR, "cannot vfork: %m") ;
		/* failure */
		syserr("openmailer: cannot fork");
		(void) close(mpvect[0]);
		(void) close(mpvect[1]);
#ifdef SMTP
		if (clever)
		{
			(void) close(rpvect[0]);
			(void) close(rpvect[1]);
		}
#endif /* SMTP */
		return (-1);
	}
	else if (pid == 0)
	{
		int i;

		/* child -- set up input & exec mailer */
		/* make diagnostic output be standard output */
		(void) signal(SIGINT, SIG_IGN);
		(void) signal(SIGHUP, SIG_IGN);
		(void) signal(SIGTERM, SIG_DFL);

		/* arrange to filter standard & diag output of command */
		if (clever)
		{
			(void) close(rpvect[0]);
			(void) close(1);
			(void) dup(rpvect[1]);
			(void) close(rpvect[1]);
		}
#ifndef __HELIOS
		else if (OpMode == MD_SMTP || HoldErrs)
#else
		else if (OpMode == MD_SMTP)
#endif		
		{
			/* put mailer output in transcript */
			(void) close(1);
			(void) dup(fileno(CurEnv->e_xfp));
		}
		(void) close(2);
		(void) dup(1);

		/* arrange to get standard input */
		(void) close(mpvect[1]);
		(void) close(0);
		if (dup(mpvect[0]) < 0)
		{
			syserr("Cannot dup to zero!");
			_exit(EX_OSERR);
		}
		(void) close(mpvect[0]);
#ifndef __HELIOS
		if (!bitnset(M_RESTR, m->m_flags))
		{
			if (ctladdr == NULL || ctladdr->q_uid == 0)
			{
#if defined(__convex__) && defined(SHARE)
				if (setupshares(DefShareUid, syserr))
					syserr("Can't install shares!");
#endif /* __convex__ && SHARE */
				(void) setgid(DefGid);
				(void) initgroups(DefUser, DefGid);
				(void) setuid(DefUid);
			}
			else
			{
#if defined(__convex__) && defined(SHARE)
				if (setupshares(DefShareUid, syserr))
					syserr("Can't install shares!");
#endif /* __convex__ && SHARE */
				(void) setgid(ctladdr->q_gid);
				(void) initgroups(ctladdr->q_ruser?
					ctladdr->q_ruser: ctladdr->q_user,
					ctladdr->q_gid);
				(void) setuid(ctladdr->q_uid);
			}
		}
#endif

		/* arrange for all the files to be closed */
#if defined(XPG3)
                for (i = (int) sysconf (_SC_OPEN_MAX); i > 2; --i) {
#else
                for (i = getdtablesize(); i > 2; --i) {
#endif /* XPG3 */
			register int j;
			if ((j = fcntl(i, F_GETFD, 0)) != -1)
				(void)fcntl(i, F_SETFD, j|1);
		}

		/* try to execute the mailer */
#ifndef __HELIOS
		execve(m->m_mailer, pvp, UserEnviron);
		syserr("Cannot exec %s", m->m_mailer);
                if (m == LocalMailer || errno == EIO || errno == EAGAIN ||
#if defined(EPROCLIM)
							errno == EPROCLIM ||
#endif /* EPROCLIM */
                    errno == ENOMEM)

			_exit(EX_TEMPFAIL);
		else
			_exit(EX_UNAVAILABLE);
#else
		if (execve (m->m_mailer, pvp, environ) < 0)
		{
			syslog (LOG_ERR, "%s: %m", m->m_mailer) ;
		}
		_exit(EX_UNAVAILABLE);
#endif			
	}

/*
-- crf: not happy here - if unable to execve mailer, provision should be made
-- to clean up and close down ...
*/
	/*
	**  Set up return value.
	*/

	(void) close(mpvect[0]);
	mfile = fdopen(mpvect[1], "w");
	if (clever)
	{
		(void) close(rpvect[1]);
		rfile = fdopen(rpvect[0], "r");
	} else
		rfile = NULL;

	*pmfile = mfile;
	*prfile = rfile;

	return (pid);
}

/* new page */
/*
**  GIVERESPONSE -- Interpret an error response from a mailer
**
**	Parameters:
**		stat -- the status code from the mailer (high byte
**			only; core dumps must have been taken care of
**			already).
**		m -- the mailer descriptor for this mailer.
**
**	Returns:
**		none.
**
**	Side Effects:
**		Errors may be incremented.
**		ExitStat may be set.
*/

void
#ifndef __HELIOS
giveresponse(stat, m, e)
#else
giveresponse(stat, e)
#endif
	int stat;
#ifndef __HELIOS
	register MAILER *m;
#endif	
	ENVELOPE *e;
{
	register char *statmsg;
	extern char *SysExMsg[];
	register int i;
	extern int N_SysEx;
#ifdef NAMED_BIND
	extern int h_errno;
#endif	/* NAMED_BIND */
	char buf[MAXLINE];

#ifdef lint
	if (m == NULL)
		return;
#endif /* lint */

	/*
	**  Compute status message from code.
	*/

	i = stat - EX__BASE;
	if (stat == 0)
#ifndef __HELIOS
		statmsg = "250 Sent";
#else
	{
		sprintf (buf, "250 %s... Sent", e->e_to) ;
		statmsg = buf ;
	}
#endif
	else if (i < 0 || i > N_SysEx)
	{
		(void) sprintf(buf, "554 unknown mailer error %d", stat);
		stat = EX_UNAVAILABLE;
		statmsg = buf;
	}
#ifndef __HELIOS
	else if (stat == EX_TEMPFAIL)
	{
		(void) strcpy(buf, SysExMsg[i]);
#ifdef NAMED_BIND
		if (h_errno == TRY_AGAIN)
			statmsg = errstring(h_errno+MAX_ERRNO);
		else
#endif	/* NAMED_BIND */
		{
			if (errno != 0)
				statmsg = errstring(errno);
			else
			{
#ifdef SMTP
				extern char SmtpError[];

				statmsg = SmtpError;
#else /* !SMTP */
				statmsg = NULL;
#endif /* SMTP */
			}
		}
		if (statmsg != NULL && statmsg[0] != '\0')
		{
			(void) strcat(buf, ": ");
			(void) strcat(buf, statmsg);
		}
		statmsg = buf;
	}
#endif
	else
	{
		statmsg = SysExMsg[i];
	}

	/*
	**  Print the message as appropriate
	*/

	if (stat == EX_OK || stat == EX_TEMPFAIL)
		message(Arpa_Info, &statmsg[4]);
	else
	{
#ifndef __HELIOS
		extern char Arpa_Usrerr[];
#endif

		Errors++;
#ifdef	NAMED_BIND
		if (stat == EX_NOHOST && h_errno != 0)
			usrerr("%s (%s)", statmsg,
				H_Errmsg[h_errno > MAXH_ERR ? 0 : h_errno]);
		else
#endif	/* NAMED_BIND */
			usrerr(statmsg);
	}

#ifndef __HELIOS
	/*
	**  Final cleanup.
	**	Log a record of the transaction.  Compute the new
	**	ExitStat -- if we already had an error, stick with
	**	that.
	*/

	if (LogLevel > ((stat == 0 || stat == EX_TEMPFAIL) ? 3 : 2))
		logdelivery(&statmsg[4]);

	if (stat != EX_TEMPFAIL)
		setstat(stat);
	if (stat != EX_OK)
	{
		if (e->e_message != NULL)
			free(e->e_message);
		e->e_message = newstr(&statmsg[4]);
	}
#endif
	errno = 0;
#ifdef NAMED_BIND
	h_errno = 0;
#endif	/* NAMED_BIND */
}

#ifndef __HELIOS
/* new page */
/*
**  LOGDELIVERY -- log the delivery in the system log
**
**	Parameters:
**		stat -- the message to print for the status
**
**	Returns:
**		none
**
**	Side Effects:
**		none
*/

void
logdelivery(stat)
	const char *stat;
{
# ifdef LOG
#  define LOGSPLIT 900
	register char *p, *q;

	/*
	** Split up long To: lines, since the buffer in
	** syslog() on various systems isn't large enough.
	*/
	p = CurEnv->e_to;
	while (strlen(p) >= LOGSPLIT)
	{
		if ((q = index(p + LOGSPLIT, ',')) != NULL)
		{
			syslog(LOG_INFO, "%s: to=%.*s(cont'd), delay=%s, stat=%s",
				CurEnv->e_id, q - p + 1, p,
				pintvl(curtime() - CurEnv->e_ctime, TRUE),
				stat);
				p = q + 1;
		} else
			break;
	}
	syslog(LOG_INFO, "%s: to=%s, delay=%s, stat=%s", CurEnv->e_id,
		p, pintvl(curtime() - CurEnv->e_ctime, TRUE), stat);
# endif /* LOG */
}

/* new page */
/*
**  PUTFROMLINE -- output a UNIX-style from line (or whatever)
**
**	This can be made an arbitrary message separator by changing $l
**
**	One of the ugliest hacks seen by human eyes is contained herein:
**	UUCP wants those stupid "remote from <host>" lines.  Why oh why
**	does a well-meaning programmer such as myself have to deal with
**	this kind of antique garbage????
**
**	Parameters:
**		fp -- the file to output to.
**		m -- the mailer describing this entry.
**
**	Returns:
**		none
**
**	Side Effects:
**		outputs some text to fp.
*/

void
putfromline(fp, m)
	register FILE *fp;
	register MAILER *m;
{
	char *oldg = macvalue('g', CurEnv);
	char template[MAXLINE];
	char newg[MAXLINE];
	char buf[MAXLINE];

	(void) strcpy(template, "\001l\n");

	if (bitnset(M_NHDR, m->m_flags))
		return;

	/* construct path through us if needed */
	if (bitnset(M_FROMPATH, m->m_flags)) {
		char myname[MAXLINE];

		expand("\001k", myname, &myname[sizeof myname - 1], CurEnv);
		if (index(oldg, '!') == NULL
		    || strncmp(oldg, myname, strlen(myname)) != 0) {
			sprintf(newg, "%s!%s", myname, oldg);
			define('g', newg, CurEnv);
		}
	}

#ifdef UGLYUUCP
	if (bitnset(M_UGLYUUCP, m->m_flags))
	{
		char *bang;

		expand("\001g", buf, &buf[sizeof buf - 1], CurEnv);
		bang = index(buf, '!');
		if (bang == NULL)
			syserr("No `!' in UUCP envelope \"from\" address! (%s)",
			    buf);
		else
		{
			*bang++ = '\0';
			(void) sprintf(template,
			    "From %s  \001d remote from %s\n", bang, buf);
		}
	}
#endif /* UGLYUUCP */
	expand(template, buf, &buf[sizeof buf - 1], CurEnv);
	putline(buf, fp, m);

	/* redefine old from address */
	if (bitnset(M_FROMPATH, m->m_flags))
		define('g', oldg, CurEnv);
}
#endif

/* new page */
/*
**  PUTBODY -- put the body of a message.
**
**	Parameters:
**		fp -- file to output onto.
**		m -- a mailer descriptor to control output format.
**		e -- the envelope to put out.
**
**	Returns:
**		none.
**
**	Side Effects:
**		The message is written onto fp.
*/

void
putbody(fp, m, e)
	FILE *fp;
	MAILER *m;
	register ENVELOPE *e;
{
	char buf[MAXLINE];

	/*
	**  Output the body of the message
	*/

	if (e->e_dfp == NULL)
	{
		if (e->e_df != NULL)
		{
			e->e_dfp = fopen(e->e_df, "r");
			if (e->e_dfp == NULL)
				syserr("putbody: Cannot open %s for %s from %s",
				e->e_df, e->e_to, e->e_from);
		}
		else
		{
			(void) strcpy(buf, "<<< No Message Collected >>>");
			putline(buf, fp, m);
		}
	}
	if (e->e_dfp != NULL)
	{
		rewind(e->e_dfp);
		while (!ferror(fp) && fgets(buf, sizeof buf, e->e_dfp) != NULL)
		{
#ifndef __HELIOS
			if (buf[0] == 'F' && bitnset(M_ESCFROM, m->m_flags) &&
			    strncmp(buf, "From ", 5) == 0)
#else
			if ((buf[0] == 'F') && 
			    (strncmp(buf, "From ", 5) == 0))
#endif			    
				(void) putc('>', fp);
			putline(buf, fp, m);
		}

		if (ferror(e->e_dfp))
		{
			syserr("putbody: read error");
			ExitStat = EX_IOERR;
		}
	}

	(void) fflush(fp);
	if (ferror(fp) && errno != EPIPE)
	{
		syserr("putbody: write error");
		ExitStat = EX_IOERR;
	}
	errno = 0;
}

#ifndef __HELIOS
/* new page */
/*
**  MAILFILE -- Send a message to a file.
**
**	If the file has the setuid/setgid bits set, but NO execute
**	bits, sendmail will try to become the owner of that file
**	rather than the real user.  Obviously, this only works if
**	sendmail runs as root.
**
**	This could be done as a subordinate mailer, except that it
**	is used implicitly to save messages in ~/dead.letter.  We
**	view this as being sufficiently important as to include it
**	here.  For example, if the system is dying, we shouldn't have
**	to create another process plus some pipes to save the message.
**
**	Parameters:
**		filename -- the name of the file to send to.
**		ctladdr -- the controlling address header -- includes
**			the userid/groupid to be when sending.
**
**	Returns:
**		The exit code associated with the operation.
**
**	Side Effects:
**		none.
*/

static
mailfile(filename, ctladdr)
	char *filename;
	ADDRESS *ctladdr;
{
	register FILE *f;
	register int pid = 0;
	ENVELOPE *e = CurEnv;

	/*
	**  Fork so we can change permissions here.
	**	Note that we MUST use fork, not vfork, because of
	**	the complications of calling subroutines, etc.
	*/

	DOFORK(fork);

	if (pid > 0 && tTd(4, 2))
		printf("mailfile: forking (pid = %d)\n", pid);
	if (pid < 0)
		return (EX_OSERR);
	else if (pid == 0)
	{
		/* child -- actually write to file */
		struct stat stb;

		(void) signal(SIGINT, SIG_DFL);
		(void) signal(SIGHUP, SIG_DFL);
		(void) signal(SIGTERM, SIG_DFL);
		(void) umask(OldUmask);
		if (stat(filename, &stb) < 0)
		{
			errno = 0;
			stb.st_mode = 0666;
		}
		if (bitset(0111, stb.st_mode))
			exit(EX_CANTCREAT);
		if (ctladdr == NULL)
			ctladdr = &e->e_from;
		/* we have to open the dfile BEFORE setuid */
		if (e->e_dfp == NULL &&  e->e_df != NULL)
		{
			e->e_dfp = fopen(e->e_df, "r");
			if (e->e_dfp == NULL) {
				syserr("mailfile: Cannot open %s for %s from %s",
				e->e_df, e->e_to, e->e_from);
			}
		}

		if (!bitset(S_ISGID, stb.st_mode) || setgid(stb.st_gid) < 0)
		{
			if (ctladdr->q_uid == 0) {
				(void) setgid(DefGid);
				(void) initgroups(DefUser, DefGid);
			} else {
				(void) setgid(ctladdr->q_gid);
				(void) initgroups(ctladdr->q_ruser?
					ctladdr->q_ruser: ctladdr->q_user,
					ctladdr->q_gid);
			}
		}
		if (!bitset(S_ISUID, stb.st_mode) || setuid(stb.st_uid) < 0)
		{
			if (ctladdr->q_uid == 0)
				(void) setuid(DefUid);
			else
				(void) setuid(ctladdr->q_uid);
		}
		f = dfopen(filename, "a");
		if (f == NULL)
			exit(EX_CANTCREAT);

		putfromline(f, ProgMailer);
		(*CurEnv->e_puthdr)(f, ProgMailer, CurEnv);
		putline("", f, ProgMailer);
		(*CurEnv->e_putbody)(f, ProgMailer, CurEnv);
		putline("", f, ProgMailer);
		(void) fclose(f);
		(void) fflush(stdout);

		/* reset ISUID & ISGID bits for paranoid systems */
		(void) chmod(filename, (int) stb.st_mode);
		exit(EX_OK);
		/*NOTREACHED*/
	}
	else
	{
		/* parent -- wait for exit status */
		int st;

		st = waitfor(pid);
		if ((st & 0377) != 0)
			return (EX_UNAVAILABLE);
		else
			return ((st >> 8) & 0377);
		/*NOTREACHED*/
	}
}

/* new page */
/*
**  SENDALL -- actually send all the messages.
**
**	Parameters:
**		e -- the envelope to send.
**		mode -- the delivery mode to use.  If SM_DEFAULT, use
**			the current SendMode.
**
**	Returns:
**		none.
**
**	Side Effects:
**		Scans the send lists and sends everything it finds.
**		Delivers any appropriate error messages.
**		If we are running in a non-interactive mode, takes the
**			appropriate action.
*/

void
sendall(e, mode)
	ENVELOPE *e;
	char mode;
{
	register ADDRESS *q;
	bool oldverbose;
	int pid;
	FILE *lockfp = NULL;

	/* determine actual delivery mode */
	if (mode == SM_DEFAULT)
	{
		if (shouldqueue(e->e_msgpriority))
			mode = SM_QUEUE;
		else
			mode = SendMode;
	}

	if (tTd(13, 1))
	{
		printf("\nSENDALL: mode %c, sendqueue:\n", mode);
		printaddr(e->e_sendqueue, TRUE);
	}

	/*
	**  Do any preprocessing necessary for the mode we are running.
	**	Check to make sure the hop count is reasonable.
	**	Delete sends to the sender in mailing lists.
	*/

	CurEnv = e;

	if (e->e_hopcount > MAXHOP)
	{
		errno = 0;
		syserr("sendall: too many hops %d (%d max): from %s, to %s",
			e->e_hopcount, MAXHOP, e->e_from, e->e_to);
		return;
	}

	if (!MeToo)
	{
		e->e_from.q_flags |= QDONTSEND;
		(void) recipient(&e->e_from, &e->e_sendqueue);
	}

#ifdef QUEUE
	if ((mode == SM_QUEUE || mode == SM_FORK ||
	     (mode != SM_VERIFY && SuperSafe)) &&
	    !bitset(EF_INQUEUE, e->e_flags))
		lockfp = queueup(e, TRUE, mode == SM_QUEUE);
#endif /* QUEUE */

	oldverbose = Verbose;
	switch (mode)
	{
	  case SM_VERIFY:
		Verbose = TRUE;
		break;

	  case SM_QUEUE:
		e->e_flags |= EF_INQUEUE|EF_KEEPQUEUE;
		if (lockfp != NULL)
			(void) fclose(lockfp);
		return;

	  case SM_FORK:
		if (e->e_xfp != NULL)
			(void) fflush(e->e_xfp);

#if defined(FCNTL_FLOCK) || defined(LOCKF_FLOCK)
		/*
		** lockf()/fcntl() emulation of flock() breaks down here as
		** locks are not inherited across fork().  release lock so
		** child can re-lock.
		**/
 		if (lockfp != NULL)
 			(void) flock(fileno(lockfp), LOCK_UN);
#endif /* (FCNTL_FLOCK) || (LOCKF_FLOCK) */
		pid = fork();
		if (pid > 0 && tTd(4, 2))
			printf("sendall: forking (pid = %d)\n", pid);
		if (pid < 0)
		{
			mode = SM_DELIVER;
			break;
		}
		else if (pid > 0)
		{
			/* be sure we leave the temp files to our child */
			e->e_id = e->e_df = NULL;
			if (lockfp != NULL)
				(void) fclose(lockfp);
			return;
		}

		/* double fork to avoid zombies */
		if (fork() > 0)
			exit(EX_OK);

#if defined(FCNTL_FLOCK) || defined(LOCKF_FLOCK) 
		/*
		** re-lock tf file in child (again for flock() as lockf())
		*/
		if (lockfp != NULL && flock(fileno(lockfp), LOCK_EX|LOCK_NB) <0)
		{
			/* someone else got in before us */
			(void) fclose(lockfp);
			return;
		}
#endif /* (FCNTL_FLOCK) || (LOCKF_FLOCK) */

		/* be sure we are immune from the terminal */
		disconnect(FALSE);

		break;
	}

	/*
	**  Run through the list and send everything.
	*/

	for (q = e->e_sendqueue; q != NULL; q = q->q_next)
	{
		if (mode == SM_VERIFY)
		{
			e->e_to = q->q_paddr;
			if (!bitset(QDONTSEND|QBADADDR, q->q_flags))
				message(Arpa_Info, "deliverable");
		}
		else
			(void) deliver(e, q);
	}
	Verbose = oldverbose;

	/*
	**  Now run through and check for errors.
	*/

	if (mode == SM_VERIFY) {
		if (lockfp != NULL)
			(void) fclose(lockfp);
		return;
	}

	for (q = e->e_sendqueue; q != NULL; q = q->q_next)
	{
		register ADDRESS *qq;

		if (tTd(13, 3))
		{
			printf("Checking ");
			printaddr(q, FALSE);
		}

		/* only send errors if the message failed */
		if (!bitset(QBADADDR, q->q_flags))
			continue;

		/* we have an address that failed -- find the parent */
		for (qq = q; qq != NULL; qq = qq->q_alias)
		{
			char obuf[MAXNAME + 6];

			/* we can only have owners for local addresses */
			if (!bitnset(M_LOCAL, qq->q_mailer->m_flags))
				continue;

			/* see if the owner list exists */
			(void) strcpy(obuf, "owner-");
			if (strncmp(qq->q_user, "owner-", 6) == 0)
				(void) strcat(obuf, "owner");
			else
				(void) strcat(obuf, qq->q_user);
			makelower(obuf);
			if (aliaslookup(obuf) == NULL)
				continue;

			if (tTd(13, 4))
				printf("Errors to %s\n", obuf);

			/* owner list exists -- add it to the error queue */
			sendtolist(obuf, (ADDRESS *) NULL, &e->e_errorqueue);
			ErrorMode = EM_MAIL;
			break;
		}

		/* if we did not find an owner, send to the sender */
		if (qq == NULL && bitset(QBADADDR, q->q_flags))
			sendtolist(e->e_from.q_paddr, qq, &e->e_errorqueue);
	}

	/* this removes the lock on the file */
	if (lockfp != NULL)
		(void) fclose(lockfp);

	if (mode == SM_FORK)
		finis();
}
#endif
