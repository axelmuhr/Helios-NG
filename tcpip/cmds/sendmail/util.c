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
static char sccsid[] = "@(#)util.c	5.18 (Berkeley) 6/1/90";
#endif /* not lint */
#else
static char *rcsid = "$Header: /hsrc/tcpip/cmds/sendmail/RCS/util.c,v 1.1 1992/01/20 14:41:08 craig Exp $";
#include <syslib.h>
#endif

#include "sendmail.h"
#include <stdio.h>
#include <sys/stat.h>
#ifndef __HELIOS
#include <sysexits.h>
#else
#include <sys/wait.h>
extern int isascii (int) ;
#endif
#include <errno.h>

#ifndef __HELIOS
static void readtimeout();

bool catPrint = FALSE;		/* xputs: print strings for catenation */

/*
**  STRIPQUOTES -- Strip quotes & quote bits from a string.
**
**	Runs through a string and strips off unquoted quote
**	characters and quote bits.  This is done in place.
**
**	Parameters:
**		s -- the string to strip.
**		qf -- if set, remove actual `` " '' characters
**			as well as the quote bits.
**
**	Returns:
**		none.
**
**	Side Effects:
**		none.
**
**	Called By:
**		deliver
*/

void
stripquotes(s, qf)
	char *s;
	bool qf;
{
	register char *p;
	register char *q;
	register char c;

	if (s == NULL)
		return;

	for (p = q = s; (c = *p++) != '\0'; )
	{
		if (c != '"' || !qf)
			*q++ = c & 0177;
	}
	*q = '\0';
}
/* new page */
/*
**  QSTRLEN -- give me the string length assuming 0200 bits add a char
**
**	Parameters:
**		s -- the string to measure.
**
**	Returns:
**		The length of s, including space for backslash escapes.
**
**	Side Effects:
**		none.
*/

qstrlen(s)
	register const char *s;
{
	register int l = 0;
	register char c;

	while ((c = *s++) != '\0')
	{
		if (bitset(0200, c))
			l++;
		l++;
	}
	return (l);
}
/* new page */
/*
**  CAPITALIZE -- return a copy of a string, properly capitalized.
**
**	Parameters:
**		s -- the string to capitalize.
**
**	Returns:
**		a pointer to a properly capitalized string.
**
**	Side Effects:
**		none.
*/

char *
capitalize(s)
	register const char *s;
{
	static char buf[50];
	register char *p;

	p = buf;

	for (;;)
	{
		while (!isalpha(*s) && *s != '\0')
			*p++ = *s++;
		if (*s == '\0')
			break;
		*p++ = toupper(*s);
		s++;
		while (isalpha(*s))
			*p++ = *s++;
	}

	*p = '\0';
	return (buf);
}
/* new page */
/*
**  XALLOC -- Allocate memory and bitch wildly on failure.
**
**	THIS IS A CLUDGE.  This should be made to give a proper
**	error -- but after all, what can we do?
**
**	Parameters:
**		sz -- size of area to allocate.
**
**	Returns:
**		pointer to data region.
**
**	Side Effects:
**		Memory is allocated.
*/

char *
xalloc(sz)
	register int sz;
{
	register char *p;

	p = malloc((unsigned) sz);
	if (p == NULL)
	{
		syserr("Out of memory!!");
		abort();
		/* exit(EX_UNAVAILABLE); */
	}
	return (p);
}
/* new page */
/*
**  COPYPLIST -- copy list of pointers.
**
**	This routine is the equivalent of newstr for lists of
**	pointers.
**
**	Parameters:
**		list -- list of pointers to copy.
**			Must be NULL terminated.
**		copycont -- if TRUE, copy the contents of the vector
**			(which must be a string) also.
**
**	Returns:
**		a copy of 'list'.
**
**	Side Effects:
**		none.
*/

char **
copyplist(list, copycont)
	char **list;
	bool copycont;
{
	register char **vp;
	register char **newvp;

	for (vp = list; *vp != NULL; vp++)
		continue;

	vp++;

	newvp = (char **) xalloc((int) (vp - list) * sizeof *vp);
	bcopy((char *) list, (char *) newvp, (int) (vp - list) * sizeof *vp);

	if (copycont)
	{
		for (vp = newvp; *vp != NULL; vp++)
			*vp = newstr(*vp);
	}

	return (newvp);
}
/* new page */
/*
**  PRINTAV -- print argument vector.
**
**	Parameters:
**		av -- argument vector.
**
**	Returns:
**		none.
**
**	Side Effects:
**		prints av.
*/

void
printav(av)
	register char **av;
{
	while (*av != NULL)
	{
		if (tTd(0, 44))
			printf("\n\t%08x=", *av);
		else
			(void) putchar(' ');
		xputs(*av++);
	}
	(void) putchar('\n');
}
/* new page */
/*
**  LOWER -- turn letter into lower case.
**
**	Parameters:
**		c -- character to turn into lower case.
**
**	Returns:
**		c, in lower case.
**
**	Side Effects:
**		none.
*/

char
lower(c)
	register char c;
{
	return(isascii(c) && isupper(c) ? tolower(c) : c);
}
/* new page */
/*
**  XPUTS -- put string doing control escapes.
**
**	Parameters:
**		s -- string to put.
**
**	Returns:
**		none.
**
**	Side Effects:
**		output to stdout
*/

void
xputs(s)
	register const char *s;
{
	register char c;
	register struct metamac *m;

	if (s == MACNULL)
	{
		printf("<macnull>");
		return;
	}
	if (s == NULL)
	{
		printf("<null>");
		return;
	}

	if (s[0] == MATCHREPL && isdigit(s[1]) && s[2] == '\0')
	{
		printf("$%c", s[1]);
		return;
	}
	else
		for (m = MetaMacros; m->metaname != '\0'; m++)
			if (m->metaval == *s)
			{
				printf("$%c%s", m->metaname, &s[1]);
				return;
			}

	if (!catPrint)
		(void) putchar('"');
	while ((c = *s++) != '\0')
	{
		if (!isascii(c))
		{
			(void) putchar('\\');
			c &= 0177;
		}
		if (c < 040 || c >= 0177)
		{
			(void) putchar('^');
			c ^= 0100;
		}
		(void) putchar(c);
	}
	if (!catPrint)
		(void) putchar('"');
	(void) fflush(stdout);
}
#endif

/* new page */
/*
**  MAKELOWER -- Translate a line into lower case
**
**	Parameters:
**		p -- the string to translate.  If NULL, return is
**			immediate.
**
**	Returns:
**		none.
**
**	Side Effects:
**		String pointed to by p is translated to lower case.
**
**	Called By:
**		parse
*/

void
makelower(p)
	register char *p;
{
	register char c;
	register bool quoted_string = FALSE;

	if (p == NULL)
		return;
	for (; (c = *p) != '\0'; p++)
		if (c == '"')
			quoted_string = !quoted_string;
		else if (!quoted_string && isascii(c) && isupper(c))
			*p = tolower(c);
}

#ifndef __HELIOS
/* new page */
/*
**  BUILDFNAME -- build full name from gecos style entry.
**
**	This routine interprets the strange entry that would appear
**	in the GECOS field of the password file.
**
**	Parameters:
**		p -- name to build.
**		login -- the login name of this user (for &).
**		buf -- place to put the result.
**
**	Returns:
**		none.
**
**	Side Effects:
**		none.
*/

void
buildfname(p, login, buf)
	register const char *p;
	const char *login;
	char *buf;
{
	register char *bp = buf;

	if (*p == '*')
		p++;
	while (*p != '\0' && *p != ',' && *p != ';' && *p != '%')
	{
		if (*p == '&')
		{
			(void) strcpy(bp, login);
			*bp = toupper(*bp);
			while (*bp != '\0')
				bp++;
			p++;
		}
		else
			*bp++ = *p++;
	}
	*bp = '\0';
}
/* new page */
/*
**  SAFEFILE -- return true if a file exists and is safe for a user.
**
**	Parameters:
**		fn -- filename to check.
**		uid -- uid to compare against.
**		mode -- mode bits that must match.
**
**	Returns:
**		TRUE if fn exists, is owned by uid, and matches mode.
**		FALSE otherwise.
**
**	Side Effects:
**		none.
*/

bool
safefile(fn, uid, mode)
	char *fn;
	int uid;
	int mode;
{
	struct stat stbuf;

	if (stat(fn, &stbuf) >= 0 && stbuf.st_uid == uid &&
	    (stbuf.st_mode & mode) == mode)
		return (TRUE);
	errno = 0;
	return (FALSE);
}
#endif

/* new page */
/*
**  FIXCRLF -- fix <CR><LF> in line.
**
**	Looks for the <CR><LF> combination and turns it into the
**	UNIX canonical <NL> character.  It only takes one line,
**	i.e., it is assumed that the first <NL> found is the end
**	of the line.
**
**	Parameters:
**		line -- the line to fix.
**		stripnl -- if true, strip the newline also.
**
**	Returns:
**		none.
**
**	Side Effects:
**		line is changed in place.
*/

void
fixcrlf(line, stripnl)
	char *line;
	bool stripnl;
{
	register char *p;

	p = index(line, '\n');
	if (p == NULL)
		return;
	if (p > line && p[-1] == '\r')
		p--;
	if (!stripnl)
		*p++ = '\n';
	*p = '\0';
}

#ifndef __HELIOS
/* new page */
/*
**  DFOPEN -- determined file open
**
**	This routine has the semantics of fopen, except that it will
**	keep trying a few times to make this happen.  The idea is that
**	on very loaded systems, we may run out of resources (inodes,
**	whatever), so this tries to get around it.
*/

FILE *
dfopen(filename, mode)
	const char *filename;
	const char *mode;
{
	register int tries;
	register FILE *fp;

	for (tries = 0; tries < 10; tries++)
	{
		Xsleep((unsigned) (10 * tries));
		errno = 0;
		fp = fopen(filename, mode);
		if (fp != NULL)
			break;
		if (errno != ENFILE && errno != EINTR)
			break;
	}
	errno = 0;
	return (fp);
}
#endif

/* new page */
/*
**  PUTLINE -- put a line like fputs obeying SMTP conventions
**
**	This routine always guarantees outputing a newline (or CRLF,
**	as appropriate) at the end of the string.
**
**	Parameters:
**		l -- line to put.
**		fp -- file to put it onto.
**		m -- the mailer used to control output.
**
**	Returns:
**		none
**
**	Side Effects:
**		output of l to fp.
*/

#define SMTPLINELIM	990	/* maximum line length */

void
putline(l, fp, m)
	register char *l;
	FILE *fp;
	MAILER *m;
{
	register char *p;
#ifndef __HELIOS
	char svchar;
	int limitsflag, xdotflag;

	limitsflag = bitnset(M_LIMITS, m->m_flags);
	/* strip out 0200 bits -- these can look like TELNET protocol */
	if (limitsflag)
		for (p=l; (*p & ~0200)!=0; p++)
			if (*p & 0200)
				*p &= ~0200;

	xdotflag = bitnset(M_XDOT, m->m_flags);
#endif
	
	do
	{
		/* find the end of the line */
		p = index(l, '\n');
		if (p == NULL)
			p = &l[strlen(l)];

		/* check for line overflow */
#ifndef __HELIOS
		while (((p - l) > SMTPLINELIM) && limitsflag)
#else
		while ((p - l) > SMTPLINELIM)
#endif		
		{
			register char *q = &l[SMTPLINELIM - 1];

#ifndef __HELIOS
			if ((*l == '.') && xdotflag)
#else
			if (*l == '.')
#endif			
				(void) putc('.', fp);
			fprintf(fp, "%.*s!%s", SMTPLINELIM-1, l, m->m_eol);
			l = q;
		}

		/* output last part */
#ifndef __HELIOS
		if ((*l == '.') && xdotflag)
#else
		if (*l == '.')
#endif		
			(void) putc('.', fp);

		/*
		 * Some broken compilers evaluate %.0s%s as %s%s.  This
		 * doubles blank lines.  I have to remember that portability
		 * is one of my goals (*sigh*).  -pbp
		 */
		if ((p-l) == 0)
			fprintf(fp, "%s", m->m_eol);
		else
			fprintf(fp, "%.*s%s", p-l, l, m->m_eol);
		l = p;
		if (*l == '\n')
			l++;
	} while (*l != '\0');
}

#ifndef __HELIOS
/* new page */
/*
**  XUNLINK -- unlink a file, doing logging as appropriate.
**
**	Parameters:
**		f -- name of file to unlink.
**
**	Returns:
**		none.
**
**	Side Effects:
**		f is unlinked.
*/

void
xunlink(f)
	char *f;
{
	register int i;

#ifdef LOG
	if (LogLevel > 20)
		syslog(LOG_DEBUG, "%s: unlink %s", CurEnv->e_id, f);
#endif /* LOG */

	i = unlink(f);
#ifdef LOG
	if (i < 0 && LogLevel > 21)
		syslog(LOG_DEBUG, "%s: unlink-fail %d", f, errno);
#endif /* LOG */
}
#else
/* unlink_temps */

void unlink_temps (ENVELOPE *e)
{
	char f_name [ID_LEN + 3] ;
	void f_close (FILE *, char *) ;
	void un_link (char *) ;

	if (e->e_id [0] != (char) NULL)
	{
		debugf ("Unlinking ...") ;
		sprintf (f_name, "df%s", e->e_id) ;
		if (e->e_dfp != (FILE *) NULL)
		{
			f_close (e->e_dfp, f_name) ;
			un_link (f_name) ;
		}
		sprintf (f_name, "xf%s", e->e_id) ;
		f_close (e->e_xfp, f_name) ;
		un_link (f_name) ;
		sprintf (f_name, "qf%s", e->e_id) ;
		un_link (f_name) ;

		e->e_id [0] = (char) NULL ;
	}
}

void f_close (FILE *f, char *f_name)
{
	if (fclose (f) != 0)
		syslog (LOG_WARNING, "failed to close: %s: %m", f_name) ;
}

void un_link (char *f_name)
{
	if (unlink (f_name) < 0)
		syslog (LOG_WARNING, "failed to unlink: %s: %m", f_name) ;
}
#endif

/* new page */
/*
**  SFGETS -- "safe" fgets -- times out and ignores random interrupts.
**
**	Parameters:
**		buf -- place to put the input line.
**		siz -- size of buf.
**		fp -- file to read from.
**
**	Returns:
**		NULL on error (including timeout).  This will also leave
**			buf containing a null string.
**		buf otherwise.
**
**	Side Effects:
**		none.
*/

#ifndef __HELIOS
static jmp_buf	CtxReadTimeout;
#endif

char *
sfgets(buf, siz, fp)
	char *buf;
	int siz;
	FILE *fp;
{
#ifndef __HELIOS
	register EVENT *ev = NULL;
#endif	
	register char *p;

	/* check for reasonable siz arg */
	if (siz < 1)
	{
		buf[0] = '\0';
		return (NULL);
	}

#ifndef __HELIOS
	/* set the timeout */
	if (ReadTimeout != 0)
	{
		if (setjmp(CtxReadTimeout) != 0)
		{
#ifdef LOG
			syslog(LOG_NOTICE,
			    "timeout waiting for input from %s",
			    RealHostName? RealHostName: "local");
#endif  /* LOG */
			errno = 0;
			usrerr("451 timeout waiting for input");
			buf[0] = '\0';
			return (NULL);
		}
		ev = setevent((time_t) ReadTimeout, readtimeout, 0);
	}
#endif	

	/* try to read */
	p = NULL;
	while (p == NULL && !feof(fp) && !ferror(fp))
	{
		errno = 0;
		p = fgets(buf, siz, fp);
		if (errno == EINTR)
			clearerr(fp);
	}

#ifndef __HELIOS
	/* clear the event if it has not sprung */
	clrevent(ev);

	/* clean up the books and exit */
	LineNumber++;
#endif
	
	if (p == NULL)
	{
		buf[0] = '\0';
		return (NULL);
	}
	for (p = buf; *p != '\0'; p++)
		*p &= ~0200;
	return (buf);
}

#ifndef __HELIOS
static void
readtimeout()
{
	longjmp(CtxReadTimeout, 1);
}

/* new page */
/*
**  FGETFOLDED -- like fgets, but know about folded lines.
**
**	Parameters:
**		buf -- place to put result.
**		n -- bytes available.
**		f -- file to read from.
**
**	Returns:
**		buf on success, NULL on error or EOF.
**
**	Side Effects:
**		buf gets lines from f, with continuation lines (lines
**		with leading white space) appended.  CRLF's are mapped
**		into single newlines.  Any trailing NL is stripped.
*/

char *
fgetfolded(buf, n, f)
	char *buf;
	register int n;
	FILE *f;
{
	register char *p = buf;
	register int i;

	n--;
	while ((i = getc(f)) != EOF)
	{
		if (i == '\r')
		{
			i = getc(f);
			if (i != '\n')
			{
				if (i != EOF)
					(void) ungetc(i, f);
				i = '\r';
			}
		}
		if (--n > 0)
			*p++ = i;
		if (i == '\n')
		{
			LineNumber++;
			i = getc(f);
			if (i != EOF)
				(void) ungetc(i, f);
			if (i != ' ' && i != '\t')
			{
				*--p = '\0';
				return (buf);
			}
		}
	}
	return (NULL);
}
/* new page */
/*
**  CURTIME -- return current time.
**
**	Parameters:
**		none.
**
**	Returns:
**		the current time.
**
**	Side Effects:
**		none.
*/

time_t
curtime()
{
	auto time_t t;

	(void) time(&t);
	return (t);
}
/* new page */
/*
**  ATOBOOL -- convert a string representation to boolean.
**
**	Defaults to "TRUE"
**
**	Parameters:
**		s -- string to convert.  Takes "tTyY" as true,
**			others as false.
**
**	Returns:
**		A boolean representation of the string.
**
**	Side Effects:
**		none.
*/

bool
atobool(s)
	register const char *s;
{
	if (*s == '\0' || index("tTyY", *s) != NULL)
		return (TRUE);
	return (FALSE);
}
/* new page */
/*
**  ATOOCT -- convert a string representation to octal.
**
**	Parameters:
**		s -- string to convert.
**
**	Returns:
**		An integer representing the string interpreted as an
**		octal number.
**
**	Side Effects:
**		none.
*/

atooct(s)
	register const char *s;
{
	register int i = 0;

	while (*s >= '0' && *s <= '7')
		i = (i << 3) | (*s++ - '0');
	return (i);
}
#endif

/* new page */
/*
**  WAITFOR -- wait for a particular process id.
**
**	Parameters:
**		pid -- process id to wait for.
**
**	Returns:
**		status of pid.
**		-1 if pid never shows up.
**
**	Side Effects:
**		none.
*/

#ifdef __HELIOS
int
#endif
waitfor(pid)
	int pid;
{
	auto int st;
	int i;

	do
	{
		errno = 0;
		i = wait(&st);
#ifndef __HELIOS
		if (i > 0 && tTd(4, 2))
			printf("waitfor: wait (pid = %d)\n", i);
#endif			
	} while ((i >= 0 || errno == EINTR) && i != pid);
	if (i < 0)
		st = -1;
	return (st);
}

#ifndef __HELIOS
/* new page */
/*
**  BITINTERSECT -- tell if two bitmaps intersect
**
**	Parameters:
**		a, b -- the bitmaps in question
**
**	Returns:
**		TRUE if they have a non-null intersection
**		FALSE otherwise
**
**	Side Effects:
**		none.
*/

bool
bitintersect(a, b)
	BITMAP a;
	BITMAP b;
{
	int i;

	for (i = BITMAPBYTES / sizeof (int); --i >= 0; )
		if ((a[i] & b[i]) != 0)
			return (TRUE);
	return (FALSE);
}
/* new page */
/*
**  BITZEROP -- tell if a bitmap is all zero
**
**	Parameters:
**		map -- the bit map to check
**
**	Returns:
**		TRUE if map is all zero.
**		FALSE if there are any bits set in map.
**
**	Side Effects:
**		none.
*/

bool
bitzerop(map)
	BITMAP map;
{
	int i;

	for (i = BITMAPBYTES / sizeof (int); --i >= 0; )
		if (map[i] != 0)
			return (FALSE);
	return (TRUE);
}

/*
**	PRINTCAV -- Print concatenated argument vector
**
**	Parameters:
**		av -- argument vector.
**
**	Returns:
**		none.
**
**	Side Effects:
**		prints av.
*/

void
printcav(av)
	register char **av;
{
	bool oldCatPrint = catPrint;

	catPrint = TRUE;
	printav(av);
	catPrint = oldCatPrint;
}
#endif

/*
**	WRITEPID -- Write process id to file
**
**	Parameters:
**		none
**
**	Returns:
**		none.
**
**	Side Effects:
**		writes pid file, creating if necessary
*/

#ifdef _PATH_SENDMAILPID
#ifndef __HELIOS
void
WritePid()
{
	extern char *PidFile;
#else	
void WritePid (char *PidFile)
{
#endif

#ifndef __HELIOS
	FILE *f;
	(void) unlink(PidFile);	/* try to be safe :-) */
	if ((f = dfopen(PidFile, "w")) != NULL)
	{
		fprintf(f, "%d\n", getpid());
		(void) chmod(PidFile, 0444);
		(void) fclose(f);
	}
# ifdef LOG
	else
		syslog(LOG_NOTICE, "Could not log daemon pid %d to file %s: %m",
		       getpid(), PidFile);
# endif /* LOG */
#else
	char daemon_name [MAXNAME] ;
	char prev_daemon_name [MAXNAME] ;
	int pid_fd ;
        extern char Version[] ;

/*
-- crf: get task id
*/
  {
    Environ *env = getenviron () ;
    sprintf(daemon_name, "%s", env->Objv [OV_Task]->Name) ;
  }
  syslog(LOG_INFO, "%s (%s) starting", Version, daemon_name);

/*
-- crf: check existing pid file
*/
    if ((pid_fd = open (PidFile, O_RDONLY)) >= 0)
    {
      debugf ("pid file exists") ;
      if (read (pid_fd, prev_daemon_name, MAXNAME) >= 0) 
      {
/*
-- crf: get rid of '\n' at end of name
*/      	
#define LAST_CHAR prev_daemon_name [strlen (prev_daemon_name) - 1] 
	if (LAST_CHAR == '\n')
	  LAST_CHAR = (char) NULL ;
        debugf ("prev. daemon name =  %s", prev_daemon_name) ;
/*
-- crf: it is possible that the current task name will be identical to the 
-- previous one. In this case, the daemon will never start up (i.e. it will 
-- Locate itself and exit) !. Therefore, the current and previous names must 
-- be compared ...
*/        
        if ((prev_daemon_name[0] == '/') && /* valid name ? */
            (strcmp (daemon_name, prev_daemon_name) != 0))
        {
          if (Locate (NULL, prev_daemon_name) != NULL)
          {
            syslog (LOG_ERR, "already exists (%s)", prev_daemon_name);
            (void) close(pid_fd);
            exit (0) ; /* daemon already present */
          }
        }
      }
    (void) close(pid_fd);
    }

  pid_fd = open(PidFile, O_WRONLY|O_CREAT|O_TRUNC, 0644);
  if (pid_fd < 0) {
    syslog(LOG_ERR, "%s: %m", PidFile);
    exit(1);
  }

  /*
   * write process id for others to know
   */
  {
    int f ;
    strcat (daemon_name, "\n") ;
    f = strlen (daemon_name);
    debugf ("daemon name = %s", daemon_name) ;    
    if (write (pid_fd, daemon_name, f) != f) {
      syslog (LOG_ERR, "%s: %m", PidFile);
      exit (1);
    }
  }

  if (close (pid_fd) < 0) /* not too concerned if this fails ... */
    debugf ("cannot close: %s", PidFile) ; 
#endif	

}
#endif	/* _PATH_SENDMAILPID */
