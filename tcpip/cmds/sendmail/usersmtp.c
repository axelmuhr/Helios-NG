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

# include "sendmail.h"

#ifndef __HELIOS
#ifndef lint
# ifdef SMTP
static char sccsid[] = "@(#)usersmtp.c	5.15 (Berkeley) 6/1/90 (with SMTP)";
# else /* ! SMTP */
static char sccsid[] = "@(#)usersmtp.c	5.15 (Berkeley) 6/1/90 (without SMTP)";
# endif /* SMTP */
#endif /* ! lint */
#else
static char *rcsid = "$Header: /hsrc/tcpip/cmds/sendmail/RCS/usersmtp.c,v 1.2 1992/06/22 15:22:30 nickc Exp $";
#endif

#ifndef __HELIOS
#include <sysexits.h>
#endif
#include <errno.h>

#ifdef __HELIOS
#include <stdarg.h>
#endif

#ifdef SMTP

#ifndef __HELIOS
# ifdef __STDC__
static reply(MAILER *);
#  ifdef VSPRINTF
/*
 * The following doesn't work as gcc transforms va_alist for some reason.
 *
 * static void smtpmessage(const char *, MAILER *, va_alist);
 */
static void smtpmessage();
#  else /* !VSPRINTF*/
static void smtpmessage();
#  endif /* VSPRINTF*/
# else /* !__STDC__ */
static reply();
static void smtpmessage();
# endif /* __STDC__  */
#else
static reply(MAILER *);
static void smtpmessage(MAILER *, const char *, ...) ;
#endif

/*
**  USERSMTP -- run SMTP protocol from the user end.
**
**	This protocol is described in RFC821.
*/

# define REPLYTYPE(r)	((r) / 100)		/* first digit of reply code */
# define REPLYCLASS(r)	(((r) / 10) % 10)	/* second digit of reply code */
# define SMTPGOODREPLY	250			/* positive SMTP response */
# define SMTPCLOSING	421			/* "Service Shutting Down" */

static char	SmtpMsgBuffer[MAXLINE];	/* buffer for commands */
static char	SmtpReplyBuffer[MAXLINE]; /* buffer for replies */
char		SmtpError[MAXLINE] = ""; /* save failure error messages */
static bool	SmtpNeedIntro;		/* set before first error */
static FILE	*SmtpOut;		/* output file */
static FILE	*SmtpIn;		/* input file */
static int	SmtpPid;		/* pid of mailer */

/* following represents the state of the SMTP connection */
static int	SmtpState;		/* connection state, see below */

# define SMTP_CLOSED	0		/* connection is closed */
# define SMTP_OPEN	1		/* connection is open for business */
# define SMTP_SSD	2		/* service shutting down */
/* new page */
/*
**  SMTPINIT -- initialize SMTP.
**
**	Opens the connection and sends the initial protocol.
**
**	Parameters:
**		m -- mailer to create connection to.
**		pvp -- pointer to parameter vector to pass to
**			the mailer.
**		e -- the envelope to deliver (#ifdef MAIL11V3)
**
**	Returns:
**		appropriate exit status -- EX_OK on success.
**		If not EX_OK, it should close the connection.
**
**	Side Effects:
**		creates connection and sends initial protocol.
*/

#ifndef __HELIOS
# ifdef MAIL11V3
smtpinit(m, pvp, e)
	register ENVELOPE *e;
# else /* ! MAIL11V3 */
smtpinit(m, pvp)
# endif /* MAIL11V3 */
	MAILER *m;
	char **pvp;
#else
int smtpinit(MAILER *m, char **pvp)
#endif	
{
	register int r;
#ifndef __HELIOS
	time_t SavedReadTimeout;
	char buf[MAXNAME];
#endif	

	/*
	**  Open the connection to the mailer.
	*/

#ifndef __HELIOS
	if (SmtpState == SMTP_OPEN)
		syserr("smtpinit: already open");
#endif

	SmtpIn = SmtpOut = NULL;
	SmtpState = SMTP_CLOSED;
	SmtpError[0] = '\0';
	SmtpNeedIntro = TRUE;
	SmtpPhase = "user open";
#ifndef __HELIOS
	setproctitle("%s %s: %s", CurEnv->e_id, pvp[1], SmtpPhase);
#endif

#ifndef __HELIOS	
	SmtpPid = openmailer(m, pvp, (ADDRESS *) NULL, TRUE, &SmtpOut, &SmtpIn);
#else
	SmtpPid = openmailer(m, pvp, TRUE, &SmtpOut, &SmtpIn);
#endif	

	if (SmtpPid < 0)
	{
#ifndef __HELIOS		
		if (tTd(18, 1))
			printf("smtpinit: cannot open %s: stat %d errno %d\n",
			   pvp[0], ExitStat, errno);
#endif			   
		if (CurEnv->e_xfp != NULL)
		{
			register char *p;

			if (errno == 0)
			{
				p = statstring(ExitStat);
				fprintf(CurEnv->e_xfp,
					"%.3s %s (%s)... %s\n",
					p, pvp[1], m->m_name, p);
			}
			else
			{
				r = errno;
				fprintf(CurEnv->e_xfp,
					"421 %s (%s)... Deferred: %s\n",
					pvp[1], m->m_name, errstring(errno));
				errno = r;
			}
		}
		return (ExitStat);
	}
	SmtpState = SMTP_OPEN;

	/*
	**  Get the greeting message.
	**	This should appear spontaneously.  Give it five minutes to
	**	happen.
	*/

	SmtpPhase = "greeting wait";
#ifndef __HELIOS
	setproctitle("%s %s: %s", CurEnv->e_id, CurHostName, SmtpPhase);
	SavedReadTimeout = ReadTimeout;
	if (ReadTimeout > 300)
		ReadTimeout = 300;
#endif	
	r = reply(m);
#ifndef __HELIOS
	ReadTimeout = SavedReadTimeout;
#endif	
	if (r < 0 || REPLYTYPE(r) != 2)
		goto tempfail;

	/*
	**  Send the HELO command.
	**	My mother taught me to always introduce myself.
	*/

#ifndef __HELIOS
	smtpmessage("HELO %s", m, MyHostName);
#else
	smtpmessage(m, "HELO %s", MyHostName);
#endif	
	SmtpPhase = "HELO wait";
#ifndef __HELIOS
	setproctitle("%s %s: %s", CurEnv->e_id, CurHostName, SmtpPhase);
#endif	
	r = reply(m);
	if (r < 0)
		goto tempfail;
	else if (REPLYTYPE(r) == 5)
		goto unavailable;
	else if (REPLYTYPE(r) != 2)
		goto tempfail;

#ifndef __HELIOS
	/*
	**  If this is expected to be another sendmail, send some internal
	**  commands.
	*/

	if (bitnset(M_INTERNAL, m->m_flags))
	{
		/* tell it to be verbose */
		smtpmessage("VERB", m);
		r = reply(m);
		if (r < 0)
			goto tempfail;

		/* tell it we will be sending one transaction only */
		smtpmessage("ONEX", m);
		r = reply(m);
		if (r < 0)
			goto tempfail;
	}
#endif

# ifdef MAIL11V3
	/*
	**  If this mailer can do multiple status returns after DATA command,
	**  ask if it will do so.
	*/
	if (bitnset(M_MANYSTATUS, m->m_flags))
	{
		smtpmessage("MULT", m);
		r = reply(m);
		if (r < 0)
			goto tempfail;
		else if (r == 250)
			SmtpManyStatus = TRUE;
	}
	else
		SmtpManyStatus = FALSE;

	/*
	**  If this mailer wants to see the headers and body early, ask if
	**  now is OK.
	*/

	if (bitnset(M_PREHEAD, m->m_flags))
	{
		smtpmessage("HEAD", m);
		r = reply(m);
		if (r < 0)
			goto tempfail;
		if (REPLYTYPE(r) == 2 || REPLYTYPE(r) == 3)
		{
			/* Send the header and message... */
			(*e->e_puthdr)(SmtpOut, m, CurEnv);
			putline("\n", SmtpOut, m);
			(*e->e_putbody)(SmtpOut, m, CurEnv);

			/* followed by the proper termination. */
			fprintf(SmtpOut, ".%s", m->m_eol);
			r = reply(m);
			if (r < 0)
				goto tempfail;
		}
	}
# endif /* MAIL11V3 */

	/*
	**  Send the MAIL command.
	**	Designates the sender.
	*/

#ifndef __HELIOS
	expand("\001g", buf, &buf[sizeof buf - 1], CurEnv);
	if (CurEnv->e_from.q_mailer == LocalMailer ||
	    !bitnset(M_FROMPATH, m->m_flags))
	{
		smtpmessage("MAIL From:<%s>", m, buf);
	}
	else
	{
		smtpmessage("MAIL From:<@%s%c%s>", m, MyHostName,
			buf[0] == '@' ? ',' : ':', buf);
	}
#else
		debugf ("e_from = %s", CurEnv->e_from) ;	
		smtpmessage(m, "MAIL From:<%s>", CurEnv->e_from);
#endif	
	SmtpPhase = "MAIL wait";
#ifndef __HELIOS
	setproctitle("%s %s: %s", CurEnv->e_id, CurHostName, SmtpPhase);
#endif	
	r = reply(m);
	if (r < 0 || REPLYTYPE(r) == 4)
		goto tempfail;
	else if (r == 250)
		return (EX_OK);
	else if (r == 552)
		goto unavailable;
#ifdef MAIL11V3
	else if (r == 559)
	{
		smtpquit(m);
		return (EX_NOHOST);
	}
#endif /* MAIL11V3 */

	/* protocol error -- close up */
	smtpquit(m);
	return (EX_PROTOCOL);

	/* signal a temporary failure */
  tempfail:
	smtpquit(m);
	return (EX_TEMPFAIL);

	/* signal service unavailable */
  unavailable:
	smtpquit(m);
	return (EX_UNAVAILABLE);
}

/* new page */
/*
**  SMTPRCPT -- designate recipient.
**
**	Parameters:
**		to -- address of recipient.
**		m -- the mailer we are sending to.
**
**	Returns:
**		exit status corresponding to recipient status.
**
**	Side Effects:
**		Sends the mail via SMTP.
*/

#ifndef __HELIOS
smtprcpt(to, m)
	ADDRESS *to;
	register MAILER *m;
#else
int smtprcpt(MAILER *m)
#endif
{
	register int r;

#ifndef __HELIOS
	smtpmessage("RCPT To:<%s>", m, to->q_user);
#else
	debugf ("CurEnv->e_to = %s", CurEnv->e_to) ;
	smtpmessage(m, "RCPT To:<%s>", CurEnv->e_to);
#endif

	SmtpPhase = "RCPT wait";
#ifndef __HELIOS
	setproctitle("%s %s: %s", CurEnv->e_id, CurHostName, SmtpPhase);
#endif	
	r = reply(m);
	if (r < 0 || REPLYTYPE(r) == 4)
		return (EX_TEMPFAIL);
	else if (REPLYTYPE(r) == 2)
		return (EX_OK);
	else if (r == 550 || r == 551 || r == 553)
		return (EX_NOUSER);
	else if (r == 552 || r == 554)
		return (EX_UNAVAILABLE);
	return (EX_PROTOCOL);
}

/* new page */
/*
**  SMTPDATA -- send the data and clean up the transaction.
**
**	Parameters:
**		m -- mailer being sent to.
**		e -- the envelope for this message.
**
**	Returns:
**		exit status corresponding to DATA command.
**
**	Side Effects:
**		none.
*/

#ifndef __HELIOS
smtpdata(m, e)
	MAILER *m;
	register ENVELOPE *e;
#else
int smtpdata(MAILER *m)
#endif	
{
	register int r;

	/*
	**  Send the data.
	**	First send the command and check that it is ok.
	**	Then send the data.
	**	Follow it up with a dot to terminate.
	**	Finally get the results of the transaction.
	*/

	/* send the command and check ok to proceed */
#ifndef __HELIOS
	smtpmessage("DATA", m);
#else
	smtpmessage(m, "DATA");
#endif	
	SmtpPhase = "DATA wait";
#ifndef __HELIOS
	setproctitle("%s %s: %s", CurEnv->e_id, CurHostName, SmtpPhase);
#endif	
	r = reply(m);
	if (r < 0 || REPLYTYPE(r) == 4)
		return (EX_TEMPFAIL);
	else if (r == 554)
		return (EX_UNAVAILABLE);
	else if (r != 354 && r != 250)
		return (EX_PROTOCOL);

#ifndef __HELIOS
	/* now output the actual message */
	(*e->e_puthdr)(SmtpOut, m, CurEnv);
	putline("\n", SmtpOut, m);
	(*e->e_putbody)(SmtpOut, m, CurEnv);
#else
#ifdef XXX_HEADER
	putheader (SmtpOut, m, CurEnv, FALSE);
	putline("\n", SmtpOut, m);
#endif
	putbody (SmtpOut, m, CurEnv);
#endif

	/* terminate the message */
	fprintf(SmtpOut, ".%s", m->m_eol);
#ifndef __HELIOS
	if (Verbose && !HoldErrs)
#else
	if (Verbose)
#endif	
		nmessage(Arpa_Info, ">>> .");
#ifdef MAIL11V3
	/*
	** If we are running with mail11v3 support, status is collected in
	** in deliver().
	*/
	return (EX_OK);
#else /* ! MAIL11V3 */

	/* check for the results of the transaction */
	SmtpPhase = "result wait";
#ifndef __HELIOS
	setproctitle("%s %s: %s", CurEnv->e_id, CurHostName, SmtpPhase);
#endif	
	r = reply(m);
	if (r < 0 || REPLYTYPE(r) == 4)
		return (EX_TEMPFAIL);
	else if (r == 250)
		return (EX_OK);
	else if (r == 552 || r == 554)
		return (EX_UNAVAILABLE);
	return (EX_PROTOCOL);
#endif /* MAIL11V3 */
}

/* new page */
/*
**  SMTPQUIT -- close the SMTP connection.
**
**	Parameters:
**		m -- a pointer to the mailer.
**
**	Returns:
**		none.
**
**	Side Effects:
**		sends the final protocol and closes the connection.
*/

void
smtpquit(m)
	register MAILER *m;
{
	int i;

	/* if the connection is already closed, don't bother */
	if (SmtpIn == NULL)
		return;

	/* send the quit message if not a forced quit */
	if (SmtpState == SMTP_OPEN || SmtpState == SMTP_SSD)
	{
#ifndef __HELIOS
		smtpmessage("QUIT", m);
#else
		smtpmessage(m, "QUIT");
#endif		
		(void) reply(m);
		if (SmtpState == SMTP_CLOSED)
			return;
	}

	/* now actually close the connection, but without trashing errno */
	i = errno;
	(void) fclose(SmtpIn);
	(void) fclose(SmtpOut);
	errno = i;
	SmtpIn = SmtpOut = NULL;
	SmtpState = SMTP_CLOSED;

	/* and pick up the zombie */
#ifndef __HELIOS
	i = endmailer(SmtpPid, m->m_argv[0]);
	if (i != EX_OK)
		syserr("smtpquit %s: stat %d", m->m_argv[0], i);
#endif		
}

/* new page */
/*
**  REPLY -- read arpanet reply
**
**	Parameters:
**		m -- the mailer we are reading the reply from.
**
**	Returns:
**		reply code it reads.
**
**	Side Effects:
**		flushes the mail file.
*/

static
reply(m)
	MAILER *m;
{
	if (SmtpOut != NULL)
		(void) fflush(SmtpOut);

#ifndef __HELIOS
	if (tTd(18, 1))
		printf("reply\n");

	if (bitnset(M_BSMTP, m->m_flags))
		return (SMTPGOODREPLY);
#endif		

	/*
	**  Read the input line, being careful not to hang.
	*/

	for (;;)
	{
		register int r;
		register char *p;

		/* actually do the read */
		if (CurEnv->e_xfp != NULL)
			(void) fflush(CurEnv->e_xfp);	/* for debugging */

		/* if we are in the process of closing just give the code */
		if (SmtpState == SMTP_CLOSED)
			return (SMTPCLOSING);

		/* get the line from the other side */
		p = sfgets(SmtpReplyBuffer, sizeof SmtpReplyBuffer, SmtpIn);
		if (p == NULL)
		{
#ifndef __HELIOS
			extern char MsgBuf[];		/* err.c */
			extern char Arpa_TSyserr[];	/* conf.c */
#endif
			/* if the remote end closed early, fake an error */
			if (errno == 0)
# ifdef ECONNRESET
				errno = ECONNRESET;
# else /* ! ECONNRESET */
				errno = EPIPE;
# endif /* ECONNRESET */

			/* Report that connection ended prematurely */
			if (CurEnv->e_xfp != NULL)
				fprintf(CurEnv->e_xfp,
					"421 %s (%s)... Deferred: %s\n",
					CurHostName, m->m_name,
					errstring(errno));

#ifndef __HELIOS
			/* if debugging, pause so we can see state */
			if (tTd(18, 100))
				pause();
#endif

#ifndef __HELIOS				
# ifdef LOG
			syslog(LOG_INFO, "%s", &MsgBuf[4]);
# endif /* LOG */
#endif

			SmtpState = SMTP_CLOSED;
			smtpquit(m);
			return (-1);
		}
		fixcrlf(SmtpReplyBuffer, TRUE);

		if (CurEnv->e_xfp != NULL && index("45", SmtpReplyBuffer[0]) != NULL)
		{
			/* serious error -- log the previous command */
			/* also record who we were talking before first error */
			if (SmtpNeedIntro)
				fprintf(CurEnv->e_xfp,
					"While talking to %s:\n", CurHostName);
			SmtpNeedIntro = FALSE;
			if (SmtpMsgBuffer[0] != '\0')
				fprintf(CurEnv->e_xfp, ">>> %s\n", SmtpMsgBuffer);
			SmtpMsgBuffer[0] = '\0';

			/* now log the message as from the other side */
			fprintf(CurEnv->e_xfp, "<<< %s\n", SmtpReplyBuffer);
		}

		/* display the input for verbose mode */
#ifndef __HELIOS
		if (Verbose && !HoldErrs)
#else		
		if (Verbose)
#endif		
			nmessage(Arpa_Info, "%s", SmtpReplyBuffer);

		/* if continuation is required, we can go on */
		if (SmtpReplyBuffer[3] == '-' || !isdigit(SmtpReplyBuffer[0]))
			continue;

		/* decode the reply code */
		r = atoi(SmtpReplyBuffer);

		/* extra semantics: 0xx codes are "informational" */
		if (r < 100)
			continue;

		/* reply code 421 is "Service Shutting Down" */
		if (r == SMTPCLOSING && SmtpState != SMTP_SSD)
		{
			/* send the quit protocol */
			SmtpState = SMTP_SSD;
			smtpquit(m);
		}

		/* save temporary failure messages for posterity */
		if (SmtpReplyBuffer[0] == '4' && SmtpError[0] == '\0')
			(void) strcpy(SmtpError, &SmtpReplyBuffer[4]);

		return (r);
	}
}

/* new page */
/*
**  SMTPMESSAGE -- send message to server
**
**	Parameters:
**		f -- format
**		m -- the mailer to control formatting.
**		a, b, c -- parameters
**
**	Returns:
**		none.
**
**	Side Effects:
**		writes message to SmtpOut.
*/

/*VARARGS1*/
static void
#ifdef VSPRINTF
#ifndef __HELIOS
smtpmessage(f, m, va_alist)
	const char *f;
	MAILER *m;
va_dcl
#else
smtpmessage(
	    MAILER *		m,
	    const char *	f,
	    ...			)
#endif	
{
	va_list	ap;

#ifndef __HELIOS
	va_start(ap);
#else
	va_start(ap, f);
#endif	
	(void) vsprintf(SmtpMsgBuffer, f, ap);
#ifndef __HELIOS
	if (tTd(18, 1) || (Verbose && !HoldErrs))
#else
	if (Verbose)
#endif
		nmessage(Arpa_Info, ">>> %s", SmtpMsgBuffer);
		
	if (SmtpOut != NULL)
		fprintf(SmtpOut, "%s%s", SmtpMsgBuffer,
			m == 0 ? "\r\n" : m->m_eol);
	va_end(ap);
}
#else /* !VSPRINTF */
smtpmessage(f, m, a, b, c)
	const char *f;
	MAILER *m;
{
	(void) sprintf(SmtpMsgBuffer, f, a, b, c);
	if (tTd(18, 1) || (Verbose && !HoldErrs))
		nmessage(Arpa_Info, ">>> %s", SmtpMsgBuffer);
	if (SmtpOut != NULL)
		fprintf(SmtpOut, "%s%s", SmtpMsgBuffer,
			m == 0 ? "\r\n" : m->m_eol);
}
#endif /* VSPRINTF */

#ifndef __HELIOS
/* new page */
# ifdef MAIL11V3
/*
**  SMTPSTAT -- collect status from DATA command
**
**	Parameters:
**		m -- the mailer we are reading the status from.
**
**	Returns:
**		status of DATA command
**
**	Side Effects:
**		none
*/

smtpstat(m)
	MAILER *m;
{
	int r;

	/* check the status returned after DATA command */
	r = reply(m);
	if (r < 0 || REPLYTYPE(r) == 4)
		return (EX_TEMPFAIL);
	else if (r == 250)
		return (EX_OK);
	else if (r == 552 || r == 554)
		return (EX_UNAVAILABLE);
	else if (r == 550 || r == 551 || r == 553)
		return (EX_NOUSER);
	return (EX_PROTOCOL);
}
# endif /* MAIL11V3 */
#endif
#endif /* SMTP */

