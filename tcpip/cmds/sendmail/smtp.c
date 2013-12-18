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

#ifdef __HELIOS
#define _DEFINE
#include <syslib.h>
#endif

#include "sendmail.h"
#include <errno.h>
#include <signal.h>

#ifndef __HELIOS
#ifndef lint
# ifdef SMTP
static char sccsid[] = "@(#)srvrsmtp.c	5.28 (Berkeley) 6/1/90 (with SMTP)";
# else /* !SMTP */
static char sccsid[] = "@(#)srvrsmtp.c	5.28 (Berkeley) 6/1/90 (without SMTP)";
# endif /* SMTP */
#endif /* not lint */
#else
#ifdef __TRAN
static char *rcsid = "$Header: /users/nickc/RTNucleus/tcpip/cmds/sendmail/RCS/smtp.c,v 1.5 1994/03/17 16:43:22 nickc Exp $";
#endif
#endif

#ifdef SMTP

#ifdef __STDC__
static char * skipword(char *, const char *);
static void help(char *);
#ifndef __HELIOS
static runinchild(const char *);
#endif
#else /* !__STDC__ */
static char * skipword();
static void help();
static runinchild();
#endif /* __STDC__ */

/*
**  SMTP -- run the SMTP protocol.
**
**	Parameters:
**		none.
**
**	Returns:
**		never.
**
**	Side Effects:
**		Reads commands from the input channel and processes
**			them.
*/

struct cmd
{
	char	*cmdname;	/* command name */
	int	cmdcode;	/* internal code, see below */
};

/* values for cmdcode */
# define CMDERROR	0	/* bad command */
# define CMDMAIL	1	/* mail -- designate sender */
# define CMDRCPT	2	/* rcpt -- designate recipient */
# define CMDDATA	3	/* data -- send message text */
# define CMDRSET	4	/* rset -- reset state */
# define CMDVRFY	5	/* vrfy -- verify address */
# define CMDHELP	6	/* help -- give usage info */
# define CMDNOOP	7	/* noop -- do nothing */
# define CMDQUIT	8	/* quit -- close connection and die */
# define CMDHELO	9	/* helo -- be polite */
#ifndef __HELIOS
# define CMDONEX	10	/* onex -- sending one transaction only */
#endif
# define CMDVERB	11	/* verb -- go into verbose mode */
#ifndef __HELIOS
# define CMDTICK	12	/* tick -- batch SMTP sync command */
/* debugging-only commands, only enabled if SMTPDEBUG is defined */
# define CMDDBGQSHOW	13	/* showq -- show send queue */
# define CMDDBGDEBUG	14	/* debug -- set debug mode */
#endif

static struct cmd	CmdTab[] =
{
	"mail",		CMDMAIL,
	"rcpt",		CMDRCPT,
	"data",		CMDDATA,
	"rset",		CMDRSET,
	"vrfy",		CMDVRFY,
	"expn",		CMDVRFY,
	"help",		CMDHELP,
	"noop",		CMDNOOP,
	"quit",		CMDQUIT,
	"helo",		CMDHELO,
	"verb",		CMDVERB,
#ifndef __HELIOS
	"onex",		CMDONEX,
	"tick",		CMDTICK,
	/*
	 * remaining commands are here only
	 * to trap and log attempts to use them
	 */
	"showq",	CMDDBGQSHOW,
	"debug",	CMDDBGDEBUG,
#endif	
	NULL,		CMDERROR,
};

#ifndef __HELIOS
bool	InChild = FALSE;		/* true if running in a subprocess */
bool	OneXact = FALSE;		/* one xaction only this run */
#endif

# define EX_QUIT	22		/* special code for QUIT command */

#ifndef __HELIOS
void
smtp(batched)
	bool batched;			/* running non-interactively? */
#else
extern char **environ ;
int main (int argc, char **argv)
#endif	
{
#ifndef __HELIOS
	int la;
	auto ADDRESS *vrfyqueue;
	ADDRESS *a;
	char TickArg[20];
	extern ENVELOPE BlankEnvelope;
	extern char Version[];
#endif
	register char *p;
	register struct cmd *c;
	char *cmd;
	bool hasmail;			/* mail command received */
#ifndef __HELIOS
	char *sendinghost;
#else
	char sendinghost [MAXNAME];
#endif	
	char inp[MAXLINE];
	char cmdbuf[100];
	char hostbuf[MAXNAME];		/* for host name transformations */
	
#ifdef __HELIOS
	ENVELOPE Current_Envelope ;
	int Channel_fd = 0 ;
	int hasrcpt;			/* rcpt command received */
	char *rcpt_list [MAX_RCPT] ;
	extern int closelog (void) ;

/*
-- crf: 04/10/92
-- Oops ! Current envelope not initialized ! Init_Env() is called later to do
-- the initialization, but Init_Env() assumes that the current directory is 
-- QueueDir and creates a transcript file here. However, if, for example, an 
-- error occurs before we are in QueueDir (i.e. before the call the Init_Env()
-- has been made), a call will be made to syserr() which access the contents of
-- the (uninitialized) envelope ... nasty. Hence, must initialize the envelope 
-- without creating the transcipt file here.
*/
	memset( &Current_Envelope, 0, sizeof (ENVELOPE) );
	CurEnv = &Current_Envelope ;

	MyHostName  [0] = '\0' ;
	RealHostName[0] = '\0' ;
	LogRequests = 0 ;
	
	(void) closelog () ;
	openlog ("smtp", LOG_PID, LOG_MAIL);

	switch (argc)
	{
		case 6:
			LogRequests = atoi (	argv[5]) ;
		case 5:
			Verbose	= atoi (	argv[4]) ;
		case 4:
			strcpy (RealHostName,	argv[3]) ;
		case 3:
			strcpy (MyHostName,	argv[2]) ;
		case 2:
			Channel_fd = atoi (	argv[1]) ;
		case 1:
			break ;
		default:
			syserr ("Too many args to smtp") ;
			exit (EX_USAGE) ;
	}

	if (LogRequests)
	{
/*
-- crf: get task id
*/
		{
			char smtp_name [MAXNAME] ;
			Environ *env = getenviron () ;
			sprintf(smtp_name, "%s", env->Objv [OV_Task]->Name) ;
			syslog(LOG_INFO, "(%s) starting", smtp_name);
		}
	}

	if (MyHostName[0] == (char) NULL)
		(void) myhostname(MyHostName, sizeof MyHostName);
	OpMode = MD_SMTP ;

	SETUP_SIG_HANDLER

#ifdef MEM_CHECK
	IOdebug ("entering smtp: Bytes free : %d  Heap size : %d", 
		Malloc(-1), Malloc(-3));
#endif
        if (((InChannel  = fdopen (Channel_fd, "r"))      == (FILE *) NULL) ||
       	    ((OutChannel = fdopen (dup(Channel_fd), "w")) == (FILE *) NULL))
       	{
		syserr ("failed to open I/O channel") ;
		exit(EX_SOFTWARE);
	}
	if (chdir(QueueDir) < 0)
	{
		syserr("cannot chdir(%s)", QueueDir);
		exit(EX_SOFTWARE);
	}
	Init_Env (CurEnv) ;
	hasrcpt = 0;
#endif
	hasmail = FALSE;
	if (OutChannel != stdout)
	{
		/* arrange for debugging output to go to remote host */
		(void) close(1);
		(void) dup(fileno(OutChannel));
	}
#ifndef __HELIOS
	settime();
#endif	

#ifndef __HELIOS
	if (RealHostName != NULL)
#else
	if (RealHostName[0] != (char) NULL)
#endif	
	{
		CurHostName = RealHostName;
#ifndef __HELIOS
		setproctitle("srvrsmtp %s", CurHostName);
#endif		
	}
	else
	{
		/* this must be us!! */
		CurHostName = MyHostName;
	}

#ifndef __HELIOS
	/* see if we are rejecting connections (see daemon.c) */
	la = getla();
	if (batched && la > RefuseLA) {
		message("421", "%s too busy, try again later", MyHostName);
		exit (EX_TEMPFAIL);
	}
	expand("\001e", inp, &inp[(sizeof(inp) - 1)], CurEnv);
#else
	sprintf (inp, "%s  %s ready at %s", 
	         MyHostName, Version_ID(), arpadate());
#endif	
	message("220", inp);
	SmtpPhase = "startup";
#ifndef __HELIOS
	sendinghost = NULL;
#else
	strcpy (sendinghost, NULL) ;	
#endif	
	for (;;)
	{
#ifndef __HELIOS		
		/* arrange for backout */
		if (setjmp(TopFrame) > 0 && InChild)
			finis();
		QuickAbort = FALSE;
		HoldErrs = FALSE;

		/* setup for the read */
		CurEnv->e_to = NULL;
#endif
		Errors = 0;
		(void) fflush(stdout);

		/* read the input line */
		p = sfgets(inp, sizeof inp, InChannel);

		/* handle errors */
		if (p == NULL)
		{
			/* end of file, just die */
			message("421", "%s Lost input channel from %s",
				MyHostName, CurHostName);
			finis();
		}

		/* clean up end of line */
		fixcrlf(inp, TRUE);

		/* echo command to transcript */
		if (CurEnv->e_xfp != NULL)
			fprintf(CurEnv->e_xfp, "<<< %s\n", inp);

		/* break off command */
		for (p = inp; isspace(*p); p++)
			continue;
		for (cmd = cmdbuf; *p != '\0' && !isspace(*p); )
			*cmd++ = *p++;
		*cmd = '\0';

		/* throw away leading whitespace */
		while (isspace(*p))
			p++;

		/* decode command */
		for (c = CmdTab; c->cmdname != NULL; c++)
		{
			if (!strcasecmp(c->cmdname, cmdbuf))
				break;
		}

		/* process command */
		switch (c->cmdcode)
		{
		  case CMDHELO:		/* hello -- introduce yourself */
			SmtpPhase = "HELO";
#ifndef __HELIOS
			setproctitle("%s: %s", CurHostName, inp);
#endif			
			/* find canonical name */
			strcpy(hostbuf, p);
#ifndef __HELIOS
			maphostname(hostbuf, sizeof(hostbuf));
#endif			
			if (!strcasecmp(hostbuf, MyHostName))
			{
				/*
				 * didn't know about alias,
				 * or connected to an echo server
				 */
				message("553", "Local configuration error, hostname not recognized as local");
				break;
			}
#ifndef __HELIOS
			if (RealHostName != NULL && strcasecmp(hostbuf, RealHostName))
#else
			if ((RealHostName[0] != (char) NULL) &&
			    (strcasecmp(hostbuf, RealHostName)))
#endif			
			{
				(void) sprintf(hostbuf, "%s (%s)", p, RealHostName);
#ifndef __HELIOS
				sendinghost = newstr(hostbuf);
				message("250", "Hello %s, why do you call yourself %s?",
					RealHostName, p);
#else
				strcpy (sendinghost, hostbuf);
				if (*p)
					message("250", "Hello %s, why do you call yourself %s?",
						RealHostName, p);
				else						
					message("250", "Hello %s, why do you not introduce yourself ?",
						RealHostName);
#endif				
			}
			else
			{
#ifndef __HELIOS
				sendinghost = newstr(p);
#else				
				strcpy (sendinghost, p);
#endif				
				message("250", "Hello %s, pleased to meet you", p);
			}
			break;

		  case CMDMAIL:		/* mail -- designate sender */
			SmtpPhase = "MAIL";

			/* force a sending host even if no HELO given */
#ifndef __HELIOS
			if (RealHostName != NULL && macvalue('s', CurEnv) == NULL)
				sendinghost = RealHostName;
#else				
			if (RealHostName[0] != (char) NULL)
				strcpy (sendinghost, RealHostName);
#endif
			/* check for validity of this command */
			if (hasmail)
			{
				message("503", "Sender already specified");
				break;
			}
#ifndef __HELIOS			
			if (InChild)
			{
				errno = 0;
				syserr("Nested MAIL command");
				exit(0);
			}

			/* fork a subprocess to process this command */
			if (runinchild("SMTP-MAIL") > 0)
				break;
			define('s', sendinghost, CurEnv);
			define('r', "SMTP", CurEnv);
			initsys();

			setproctitle("%s %s: %s", CurEnv->e_id,
				CurHostName, inp);
#endif				
			/* child -- go do the processing */
			p = skipword(p, "from");
			if (p == NULL)
				break;
#ifndef __HELIOS				
			setsender(p);
#else
			strcpy (CurEnv->e_from, p) ;
#endif			
			if (Errors == 0)
			{
#ifndef __HELIOS
				message("250", "Sender ok");
#else
				message("250", "%s... Sender ok",
					CurEnv->e_from);
#endif
				hasmail = TRUE;
			}
#ifndef __HELIOS
			else if (InChild)
				finis();
#endif				
			break;

		  case CMDRCPT:		/* rcpt -- designate recipient */
			SmtpPhase = "RCPT";
#ifndef __HELIOS
			setproctitle("%s %s: %s", CurEnv->e_id,
				CurHostName, inp);

			if (setjmp(TopFrame) > 0)
			{
				if (!batched)
					CurEnv->e_flags &= ~EF_FATALERRS;
				break;
			}
			QuickAbort = TRUE;
#endif			
			p = skipword(p, "to");
#ifndef __HELIOS
			if (p == NULL)
#else
			if (*p == NULL)
#endif			
				break;
#ifndef __HELIOS				
			a = parseaddr(p, (ADDRESS *) NULL, 1, '\0');
			if (a == NULL)
				break;
			a->q_flags |= QPRIMARY;
			a = recipient(a, &CurEnv->e_sendqueue);
#endif			
			if (Errors != 0)
				break;

#ifndef __HELIOS
			/* no errors during parsing, but might be a duplicate */
			CurEnv->e_to = p;
#else
if (!valid_name (p))
{
				message("550", "%s... User unknown", p) ;
}
else
{
			if (hasrcpt >= MAX_RCPT)
			{
				char buf [MAXLINE] ;
				sprintf (buf, 
					"RCPT TO: %s ignored (RCPT limit [%d] execeeded)",
					p, MAX_RCPT) ;
				syslog (LOG_WARNING, "%s", buf) ;
				message("552", "%s", buf) ;
			}
			else
			{
				rcpt_list [hasrcpt] = (char *) Malloc ((long)strlen (p) + 1) ;
				if (!rcpt_list [hasrcpt])
				{
					message("552", "out of memory") ;
					syserr ("out of memory") ;
					exit (EX_OSERR) ;
				}	
				strcpy (rcpt_list [hasrcpt], p) ;
				debugf ("Malloc'd rcpt : %s OK", rcpt_list [hasrcpt]) ;
				hasrcpt ++ ; 
				message("250", "%s... Recipient ok", p) ;
			}
}
#endif
				
#ifndef __HELIOS			
			if (!bitset(QBADADDR, a->q_flags))
				message("250", "Recipient ok");
			else
			{
				/* punt -- should keep message in ADDRESS.... */
				message("550", "Addressee unknown");
			}
			CurEnv->e_to = NULL;
#endif				
			break;

		  case CMDDATA:		/* data -- text of mail */
			SmtpPhase = "DATA";
			if (!hasmail)
			{
				message("503", "Need valid MAIL command");
#ifndef __HELIOS
				if (batched)
					Errors++;
				else
#else
				Errors++;
#endif				
				break;
			}
#ifndef __HELIOS			
			else if (CurEnv->e_nrcpts <= 0)
#else
			else if (!hasrcpt)
#endif
			{
				message("503", "Need valid RCPT (recipient)");
#ifndef __HELIOS
				if (batched)
					Errors++;
				else
#else
				Errors++;
#endif
				break;
			}

			/* collect the text of the message */
			SmtpPhase = "collect";
#ifndef __HELIOS
			setproctitle("%s %s: %s", CurEnv->e_id,
				CurHostName, inp);
#endif
			collect(TRUE);
			if (Errors != 0)
				break;

			/*
			**  Arrange to send to everyone.
			**	If sending to multiple people, mail back
			**		errors rather than reporting directly.
			**	In any case, don't mail back errors for
			**		anything that has happened up to
			**		now (the other end will do this).
			**	Truncate our transcript -- the mail has gotten
			**		to us successfully, and if we have
			**		to mail this back, it will be easier
			**		on the reader.
			**	Then send to everyone.
			**	Finally give a reply code.  If an error has
			**		already been given, don't mail a
			**		message back.
			**	We goose error returns by clearing error bit.
			*/

			SmtpPhase = "delivery";
#ifndef __HELIOS
			if (CurEnv->e_nrcpts != 1 || batched)
			{
				HoldErrs = TRUE;
				ErrorMode = EM_MAIL;
			}
			if (!batched) {
				CurEnv->e_flags &= ~EF_FATALERRS;
				CurEnv->e_xfp = freopen(queuename(CurEnv, 'x'),
							"w", CurEnv->e_xfp);
			}

			/* send to all recipients */
			sendall(CurEnv, SM_DEFAULT);
			CurEnv->e_to = NULL;

			/* save statistics */
			markstats(CurEnv, (ADDRESS *) NULL);
#else
			{
				char xf_name [MAXNAME] ;
				int rcode = 0 ;
				int i ;

				strcpy (xf_name, queuename (CurEnv, 'x')) ;
				if ((CurEnv->e_xfp = freopen (xf_name, "w", CurEnv->e_xfp)) == NULL)
					syslog (LOG_WARNING, "failed to freopen: %s: %m", 
						xf_name);

				for (i = 0 ; i < hasrcpt ; i ++)
				{
					strcpy (CurEnv->e_to, rcpt_list [i]) ;
					debugf ("freeing rcpt : %s", rcpt_list [i]) ;
					if (Free (rcpt_list [i]) != 0)
					{
						syserr ("error freeing memory") ;
						exit (EX_OSERR) ;
					}
					rcode += deliver (CurEnv) ;
				}
				if (!rcode)
					unlink_temps (CurEnv) ;
			}
#endif

			/* issue success if appropriate and reset */
#ifndef __HELIOS
			if (Errors == 0 || HoldErrs)
#else
			if (Errors == 0)
#endif			
				message("250", "Ok");
			else
				syslog (LOG_ERR, "Error in message delivery") ;
#ifndef __HELIOS
				CurEnv->e_flags &= ~EF_FATALERRS;
				
			/* if in a child, pop back to our parent */
			if (InChild)
				finis();
#endif
			/* clean up a bit */
			hasmail = 0;
#ifdef __HELIOS
			hasrcpt = 0;
#endif
			
#ifndef __HELIOS
			dropenvelope(CurEnv);
			CurEnv = newenvelope(CurEnv);
			CurEnv->e_flags = BlankEnvelope.e_flags;
#else
			Init_Env (CurEnv) ;
#endif			
			break;

		  case CMDRSET:		/* rset -- reset state */
			message("250", "Reset state");
#ifndef __HELIOS
			if (InChild)
				finis();
#endif				
			break;

		  case CMDVRFY:		/* vrfy -- verify address */
#ifndef __HELIOS
			if (runinchild("SMTP-VRFY") > 0)
				break;
			setproctitle("%s: %s", CurHostName, inp);
			vrfyqueue = NULL;
			QuickAbort = TRUE;
			sendtolist(p, (ADDRESS *) NULL, &vrfyqueue);
			if (Errors != 0)
			{
				if (InChild)
					finis();
				break;
			}
			while (vrfyqueue != NULL)
			{
				register ADDRESS *a = vrfyqueue->q_next;
				char *code;

				while (a != NULL && bitset(QDONTSEND|QBADADDR, a->q_flags))
					a = a->q_next;

				if (!bitset(QDONTSEND|QBADADDR, vrfyqueue->q_flags))
				{
					if (a != NULL)
						code = "250-";
					else
						code = "250";
					if (vrfyqueue->q_fullname == NULL)
						message(code, "<%s>", vrfyqueue->q_paddr);
					else
						message(code, "%s <%s>",
						    vrfyqueue->q_fullname, vrfyqueue->q_paddr);
				}
				else if (a == NULL)
					message("554", "Self destructive alias loop");
				vrfyqueue = a;
			}
			if (InChild)
				finis();
#else
			message("502", "Command not implemented");
#endif				
			break;

		  case CMDHELP:		/* help -- give user info */
			if (*p == '\0')
#ifndef __HELIOS
/*
-- crf: naughty !!!
*/
				p = "SMTP";
#else
			{
				strcpy (inp, "SMTP") ;
				p = inp ;
			}
#endif
			help(p);
			break;

		  case CMDNOOP:		/* noop -- do nothing */
			message("200", "OK");
			break;

		  case CMDQUIT:		/* quit -- leave mail */
			message("221", "%s closing connection", MyHostName);
#ifndef __HELIOS			
			if (InChild)
				ExitStat = EX_QUIT;
#endif				
			finis();

		  case CMDVERB:		/* set verbose mode */
			Verbose = TRUE;
#ifndef __HELIOS
			SendMode = SM_DELIVER;
#endif			
			message("200", "Verbose mode");
			break;

#ifndef __HELIOS
		  case CMDONEX:		/* doing one transaction only */
			OneXact = TRUE;
			message("200", "Only one transaction");
			break;

		  case CMDTICK:		/* BSMTP TICK */
			(void) strncpy(TickArg, p, 20-1);
			message("250", "OK");
			break;

# ifdef SMTPDEBUG
		  case CMDDBGQSHOW:	/* show queues */
			printf("Send Queue=");
			printaddr(CurEnv->e_sendqueue, TRUE);
			break;

		  case CMDDBGDEBUG:	/* set debug mode */
			tTsetup(tTdvect, sizeof tTdvect, "0-99.1");
			tTflag(p);
			message("200", "Debug set");
			break;

# else /* not SMTPDEBUG */

		  case CMDDBGQSHOW:	/* show queues */
		  case CMDDBGDEBUG:	/* set debug mode */
#  ifdef LOG
			if (RealHostName != NULL && LogLevel > 0)
				syslog(LOG_NOTICE,
				    "\"%s\" command from %s (%s)\n",
				    c->cmdname, RealHostName,
				    inet_ntoa(RealHostAddr.sin_addr));
#  endif /* LOG */
			/* FALL THROUGH */
#endif			
# endif /* SMTPDEBUG */

		  case CMDERROR:	/* unknown command */
			message("500", "Command unrecognized");
#ifndef __HELIOS
/*
-- crf: don't want a syslog for this ...
*/
# ifdef LOG
			syslog(LOG_NOTICE, "\"%s\" command unrecognized\n", cmdbuf);
# endif /* LOG */
#endif
			break;

		  default:
			errno = 0;
			syserr("smtp: unknown code %d", c->cmdcode);
			break;
		}
		/* NOTREACHED */
	}
}

/* new page */
/*
**  SKIPWORD -- skip a fixed word.
**
**	Parameters:
**		p -- place to start looking.
**		w -- word to skip.
**
**	Returns:
**		p following w.
**		NULL on error.
**
**	Side Effects:
**		clobbers the p data area.
*/

static char *
skipword(p, w)
	register char *p;
	const char *w;
{
	register char *q;

	/* find beginning of word */
	while (isspace(*p))
		p++;
	q = p;

	/* find end of word */
	while (*p != '\0' && *p != ':' && !isspace(*p))
		p++;
	while (isspace(*p))
		*p++ = '\0';
	if (*p != ':')
	{
	  syntax:
		message("501", "Syntax error");
		Errors++;
		return (NULL);
	}
	*p++ = '\0';
	while (isspace(*p))
		p++;

	/* see if the input word matches desired word */
	if (strcasecmp(q, (char *)w))
		goto syntax;

	return (p);
}

/* new page */
/*
**  HELP -- implement the HELP command.
**
**	Parameters:
**		topic -- the topic we want help for.
**
**	Returns:
**		none.
**
**	Side Effects:
**		outputs the help file to message output.
*/

static void
help(topic)
	char *topic;
{
	register FILE *hf;
	int len;
	char buf[MAXLINE];
	bool noinfo;
#ifdef __HELIOS
	char *HelpFile = SMTP_HELP ;
#endif

	if (HelpFile == NULL || (hf = fopen(HelpFile, "r")) == NULL)
	{
		/* no help */
		errno = 0;
		message("502", "HELP not implemented");
		return;
	}

	len = strlen(topic);
	makelower(topic);
	noinfo = TRUE;

	while (fgets(buf, sizeof buf, hf) != NULL)
	{
		if (strncmp(buf, topic, len) == 0)
		{
			register char *p;

			p = index(buf, '\t');
			if (p == NULL)
				p = buf;
			else
				p++;
			fixcrlf(p, TRUE);
			message("214-", p);
			noinfo = FALSE;
		}
	}

	if (noinfo)
		message("504", "HELP topic unknown");
	else
		message("214", "End of HELP info");
	(void) fclose(hf);
}

#ifndef __HELIOS
/* new page */
/*
**  RUNINCHILD -- return twice -- once in the child, then in the parent again
**
**	Parameters:
**		label -- a string used in error messages
**
**	Returns:
**		zero in the child
**		one in the parent
**
**	Side Effects:
**		none.
*/

static
runinchild(label)
	const char *label;
{
	int childpid;

	if (!OneXact)
	{
		childpid = dofork();
		if (childpid > 0 && tTd(4, 2))
			printf("runinchild: forking (pid = %d)\n", childpid);
		if (childpid < 0)
		{
			syserr("%s: cannot fork", label);
			return (1);
		}
		if (childpid > 0)
		{
			auto int st;

			/* parent -- wait for child to complete */
			st = waitfor(childpid);
			if (st == -1)
				syserr("%s: lost child", label);

			/* if we exited on a QUIT command, complete the process */
			if (st == (EX_QUIT << 8))
				finis();

			return (1);
		}
		else
		{
			/* child */
			InChild = TRUE;
			QuickAbort = FALSE;
			clearenvelope(CurEnv, FALSE);
		}
	}

	/* open alias database */
	initaliases(FALSE);

	return (0);
}
#endif

#endif /* SMTP */
