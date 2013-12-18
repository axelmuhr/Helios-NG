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
static char sccsid[] = "@(#)err.c	5.10 (Berkeley) 6/1/90";
#endif /* not lint */
#else
static char *rcsid = "$Header: /hsrc/tcpip/cmds/sendmail/RCS/err.c,v 1.1 1992/01/20 14:41:08 craig Exp $";
#endif

#include "sendmail.h"
#include <errno.h>
#ifndef __HELIOS
#include <netdb.h>
#else
#include <stdarg.h>
#endif

#ifndef __HELIOS
#ifdef __STDC__
static void putmsg(const char *, int);		/* bool -> int */
static void puterrmsg(const char *);
# ifndef VSPRINTF
static void fmtmsg();
# else /* VSPRINTF */
static void fmtmsg(char *, const char *, const char *, int, const char *, va_list);
# endif /* !VSPRINTF */
#else /* !__STDC__ */
static void putmsg();
static void puterrmsg();
static void fmtmsg();
#endif /* __STDC__ */
#else
static void putmsg(const char *, int);		/* bool -> int */
static void puterrmsg(const char *);
static void fmtmsg(char *, const char *, const char *, int, const char *, 
                   va_list);
#endif

/*
**  SYSERR -- Print error message.
**
**	Prints an error message via printf to the diagnostic
**	output.  If LOG is defined, it logs it also.
**
**	Parameters:
**		fmt -- the format string
**		a, b, c, d, e -- parameters
**
**	Returns:
**		none
**		Through TopFrame if QuickAbort is set.
**
**	Side Effects:
**		increments Errors.
**		sets ExitStat.
*/

#ifdef lint
int	sys_nerr;
char	*sys_errlist[];
#endif /* lint */
char	MsgBuf[BUFSIZ*2];	/* text of most recent message */

#ifndef __HELIOS
#ifdef VSPRINTF
/*VARARGS*/
void
syserr(fmt, va_alist)
	const char *fmt;
va_dcl
{
	va_list	ap;
#else /* !VSPRINTF */
/*VARARGS1*/
void
syserr(fmt, a, b, c, d, e)
	const char *fmt;
{
#endif /* VSPRINTF */
#else
void syserr (const char *fmt, ...)
{
	va_list ap ;
#endif

	register char *p;
	int olderrno = errno;
#ifndef __HELIOS
	extern char Arpa_PSyserr[];
	extern char Arpa_TSyserr[];
#endif

	/* format and output the error message */
	if (olderrno == 0)
		p = Arpa_PSyserr;
	else
		p = Arpa_TSyserr;
		
#ifndef __HELIOS
#ifdef VSPRINTF
	va_start(ap);
	fmtmsg(MsgBuf, (char *) NULL, p, olderrno, fmt, ap);
	va_end(ap);
#else /* !VSPRINTF */
	fmtmsg(MsgBuf, (char *) NULL, p, olderrno, fmt, a, b, c, d, e);
#endif /* VSPRINTF */
#else
	va_start(ap, fmt);
	fmtmsg(MsgBuf, (char *) NULL, p, olderrno, fmt, ap);
	va_end(ap);
#endif
	puterrmsg(MsgBuf);

	/* determine exit status if not already set */
	if (ExitStat == EX_OK)
	{
		if (olderrno == 0)
			ExitStat = EX_SOFTWARE;
		else
			ExitStat = EX_OSERR;
	}

#ifdef LOG
	if (LogLevel > 0)
		syslog(LOG_CRIT, "%s: SYSERR: %s",
#ifndef __HELIOS
			CurEnv->e_id == NULL ? "NOQUEUE" : CurEnv->e_id,
#else
			CurEnv->e_id[0] == (char) NULL ? "NOQUEUE" : CurEnv->e_id,
#endif			
			&MsgBuf[4]);
#endif /* LOG */
	errno = 0;
#ifndef __HELIOS
	if (QuickAbort)
		longjmp(TopFrame, 2);
#endif		
}
/* new page */
/*
**  USRERR -- Signal user error.
**
**	This is much like syserr except it is for user errors.
**
**	Parameters:
**		fmt, a, b, c, d -- printf strings
**
**	Returns:
**		none
**		Through TopFrame if QuickAbort is set.
**
**	Side Effects:
**		increments Errors.
*/

#ifndef __HELIOS
#ifdef VSPRINTF
/*VARARGS*/
void
usrerr(fmt, va_alist)
	const char *fmt;
va_dcl
{
	va_list	ap;
#else /* !VSPRINTF */
/*VARARGS1*/
void
usrerr(fmt, a, b, c, d, e)
	const char *fmt;
{
#endif /* VSPRINTF */
#else
void usrerr (const char *fmt, ...)
{
	va_list ap ;
#endif
#ifndef __HELIOS
	extern char Arpa_Usrerr[];
#endif	
	extern int errno;

#ifndef __HELIOS
	if (SuprErrs)
		return;

#ifdef VSPRINTF
	va_start(ap);
	fmtmsg(MsgBuf, CurEnv->e_to, Arpa_Usrerr, errno, fmt, ap);
	va_end(ap);
#else /* !VSPRINTF */
	fmtmsg(MsgBuf, CurEnv->e_to, Arpa_Usrerr, errno, fmt, a, b, c, d, e);
#endif /* VSPRINTF */
#else
	va_start(ap, fmt);
	debugf ("not using CurEnv->e_to ...") ;
	fmtmsg(MsgBuf, (char *) NULL, Arpa_Usrerr, errno, fmt, ap);
	va_end(ap);
#endif
	puterrmsg(MsgBuf);

#ifndef __HELIOS
	if (QuickAbort)
		longjmp(TopFrame, 1);
#endif		
}
/* new page */
/*
**  MESSAGE -- print message (not necessarily an error)
**
**	Parameters:
**		num -- the default ARPANET error number (in ascii)
**		msg -- the message (printf fmt) -- if it begins
**			with a digit, this number overrides num.
**		a, b, c, d, e -- printf arguments
**
**	Returns:
**		none
**
**	Side Effects:
**		none.
*/

#ifndef __HELIOS
#ifdef VSPRINTF
/*VARARGS*/
void
message(num, msg, va_alist)
	register const char *num;
	register const char *msg;
va_dcl
{
	va_list	ap;

	errno = 0;
	va_start(ap);
	fmtmsg(MsgBuf, CurEnv->e_to, num, 0, msg, ap);
	va_end(ap);
	putmsg(MsgBuf, FALSE);
}
#else /* !VSPRINTF */
/*VARARGS2*/
void
message(num, msg, a, b, c, d, e)
	register const char *num;
	register const char *msg;
{
	errno = 0;
	fmtmsg(MsgBuf, CurEnv->e_to, num, 0, msg, a, b, c, d, e);
	putmsg(MsgBuf, FALSE);
}
#endif /* VSPRINTF */
#else
void message (const char *num, const char *msg, ...)
{
	va_list ap;
	errno = 0;
	va_start (ap, msg);
	debugf ("not using CurEnv->e_to ...") ;
	fmtmsg(MsgBuf, (char *) NULL, num, 0, msg, ap);
	va_end(ap);
	putmsg(MsgBuf, FALSE);
}
#endif

/* new page */
/*
**  NMESSAGE -- print message (not necessarily an error)
**
**	Just like "message" except it never puts the to... tag on.
**
**	Parameters:
**		num -- the default ARPANET error number (in ascii)
**		msg -- the message (printf fmt) -- if it begins
**			with three digits, this number overrides num.
**		a, b, c, d, e -- printf arguments
**
**	Returns:
**		none
**
**	Side Effects:
**		none.
*/

#ifndef __HELIOS
#ifdef VSPRINTF
/*VARARGS*/
void
nmessage(num, msg, va_alist)
	register const char *num;
	register const char *msg;
va_dcl
{
	va_list	ap;

	errno = 0;
	va_start(ap);
	fmtmsg(MsgBuf, (char *) NULL, num, 0, msg, ap);
	va_end(ap);
	putmsg(MsgBuf, FALSE);
}
#else /* !VSPRINTF */
/*VARARGS2*/
void
nmessage(num, msg, a, b, c, d, e)
	register const char *num;
	register const char *msg;
{
	errno = 0;
	fmtmsg(MsgBuf, (char *) NULL, num, 0, msg, a, b, c, d, e);
	putmsg(MsgBuf, FALSE);
}
#endif /* VSPRINTF */
#else
void
nmessage(const char *num, const char *msg, ...)
{
	va_list	ap;

	errno = 0;
	va_start(ap, msg);
	fmtmsg(MsgBuf, (char *) NULL, num, 0, msg, ap);
	va_end(ap);
	putmsg(MsgBuf, FALSE);
}
#endif

/* new page */
/*
**  PUTMSG -- output error message to transcript and channel
**
**	Parameters:
**		msg -- message to output (in SMTP format).
**		holdmsg -- if TRUE, don't output a copy of the message to
**			our output channel.
**
**	Returns:
**		none.
**
**	Side Effects:
**		Outputs msg to the transcript.
**		If appropriate, outputs it to the channel.
**		Deletes SMTP reply code number as appropriate.
*/

static void
putmsg(msg, holdmsg)
	const char *msg;
	bool holdmsg;
{
	/* output to transcript if serious */
	if (CurEnv->e_xfp != NULL && (msg[0] == '4' || msg[0] == '5'))
		fprintf(CurEnv->e_xfp, "%s\n", msg);

	/* output to channel if appropriate */
	if (!holdmsg && (Verbose || msg[0] != '0'))
	{
		(void) fflush(stdout);
#ifndef __HELIOS
		if (OpMode == MD_SMTP || OpMode == MD_ARPAFTP)
#else
		if (OpMode == MD_SMTP)
#endif		
			fprintf(OutChannel, "%s\r\n", msg);
		else
			fprintf(OutChannel, "%s\n", &msg[4]);
		(void) fflush(OutChannel);
	}
}
/* new page */
/*
**  PUTERRMSG -- like putmsg, but does special processing for error messages
**
**	Parameters:
**		msg -- the message to output.
**
**	Returns:
**		none.
**
**	Side Effects:
**		Sets the fatal error bit in the envelope as appropriate.
*/

static void
puterrmsg(msg)
	const char *msg;
{
	/* output the message as usual */
#ifndef __HELIOS
	putmsg(msg, HoldErrs);
#else
	putmsg(msg, FALSE);
#endif	

	/* signal the error */
	Errors++;
#ifndef __HELIOS
	if (msg[0] == '5')
		CurEnv->e_flags |= EF_FATALERRS;
#endif		
}
/* new page */
/*
**  FMTMSG -- format a message into buffer.
**
**	Parameters:
**		eb -- error buffer to get result.
**		to -- the recipient tag for this message.
**		num -- arpanet error number.
**		en -- the error number to display.
**		fmt -- format of string.
**		ap -- varargs pointer #ifdef VSPRINTF
**		a, b, c, d, e -- arguments.  #ifndef VSPRINTF
**
**	Returns:
**		none.
**
**	Side Effects:
**		none.
*/

#ifndef __HELIOS
/*VARARGS5*/
static void
#ifdef VSPRINTF
fmtmsg(eb, to, num, eno, fmt, ap)
	va_list ap;
#else /* !VSPRINTF */
fmtmsg(eb, to, num, eno, fmt, a, b, c, d, e)
#endif /* VSPRINTF */
	register char *eb;
	const char *to, *num, *fmt;
	int eno;
#else
static void
fmtmsg(char *eb, const char *to, const char *num, int eno, 
	const char *fmt, va_list ap)
#endif
{
	char del;

	/* output the reply code */
	if (isdigit(fmt[0]) && isdigit(fmt[1]) && isdigit(fmt[2]))
	{
		num = fmt;
		fmt += 4;
	}
	if (num[3] == '-')
		del = '-';
	else
		del = ' ';
	(void) sprintf(eb, "%3.3s%c", num, del);
	eb += 4;

#ifndef __HELIOS
	/* output the file name and line number */
	if (FileName != NULL)
	{
		(void) sprintf(eb, "%s: line %d: ", FileName, LineNumber);
		eb += strlen(eb);
	}
#endif

	/* output the "to" person */
	if (to != NULL && to[0] != '\0')
	{
		(void) sprintf(eb, "%s... ", to);
		while (*eb != '\0')
			*eb++ &= 0177;
	}

	/* output the message */
#ifdef VSPRINTF
	(void) vsprintf(eb, fmt, ap);
#else /* !VSPRINTF */
	(void) sprintf(eb, fmt, a, b, c, d, e);
#endif /* VSPRINTF */
	while (*eb != '\0')
		*eb++ &= 0177;

	/* output the error code, if any */
	if (eno != 0)
	{
#ifndef __HELIOS
		extern char *errstring();
#else
		extern char *errstring(int);
#endif

		(void) sprintf(eb, ": %s", errstring(eno));
		/* eb += strlen(eb); un-used  -pbp */
	}
}
/* new page */
/*
**  ERRSTRING -- return string description of error code
**
**	Parameters:
**		errno -- the error number to translate
**
**	Returns:
**		A string description of errno.
**
**	Side Effects:
**		none.
*/

char *
errstring(errno)
	int errno;
{
	extern char *sys_errlist[];
	extern int sys_nerr;
	static char buf[100];
#ifdef SMTP
	extern char *SmtpPhase;
#endif /* SMTP */

#if defined(DAEMON) && defined(VMUNIX)
	/*
	**  Handle special network error codes.
	**
	**	These are 4.2/4.3bsd specific; they should be in daemon.c.
	*/

	switch (errno)
	{
	  case ETIMEDOUT:
	  case ECONNRESET:
		(void) strcpy(buf, sys_errlist[errno]);
		if (SmtpPhase != NULL)
		{
			(void) strcat(buf, " during ");
			(void) strcat(buf, SmtpPhase);
		}
		if (CurHostName != NULL)
		{
			(void) strcat(buf, " with ");
			(void) strcat(buf, CurHostName);
		}
		return (buf);

	  case EHOSTDOWN:
		if (CurHostName == NULL)
			break;
		(void) sprintf(buf, "Host %s is down", CurHostName);
		return (buf);

	  case ECONNREFUSED:
		if (CurHostName == NULL)
			break;
		(void) sprintf(buf, "Connection refused by %s", CurHostName);
		return (buf);

# ifdef NAMED_BIND
	  case (TRY_AGAIN+MAX_ERRNO):
		(void) sprintf(buf, "Host Name Lookup Failure");
		return (buf);
# endif /* NAMED_BIND */
	}
#endif /* VMUNIX && DAEMON */

	if (errno > 0 && errno < sys_nerr)
		return (sys_errlist[errno]);

	(void) sprintf(buf, "Error %d", errno);
	return (buf);
}
