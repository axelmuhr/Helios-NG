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
# define IN_SCCS_ID
char copyright[] =
"@(#) Copyright (c) 1988 Regents of the University of California.\n\
 All rights reserved.\n";
static char sccsid[] = "@(#)main.c	5.31 (Berkeley) 7/20/90";
# ifdef	__GNUC__
static	char	compiled[] = "@(#)compiled by gcc version "__VERSION__;
# endif	/* __GNUC__ */
#endif /* not lint */
#else
#ifdef __TRAN
static char *rcsid = "$Header: /users/nickc/RTNucleus/tcpip/cmds/sendmail/RCS/main.c,v 1.8 1994/03/17 16:42:07 nickc Exp $";
#endif
#endif

#define _DEFINE

#ifdef __HELIOS
#include <nonansi.h>
#include <sys/types.h>
#include <sys/wait.h>
#endif

#include "sendmail.h"
#include <signal.h>
#ifndef __HELIOS
#include <sgtty.h>
#ifdef NAMED_BIND
# include <arpa/nameser.h>
# include <resolv.h>
#endif /* NAMED_BIND */
#ifndef MAXHOSTNAMELEN
# define MAXHOSTNAMELEN	64
#endif /* !MAXHOSTNAMELEN */

# ifdef lint
char	edata, end;
# endif /* lint */

#ifdef __STDC__
static void intsig();
static void initmacros();
static void freeze(const char *);
static thaw(const char *);
#else /* !__STDC__ */
static void intsig();
static void initmacros();
static void freeze();
static thaw();
#endif /* __STDC__ */
#endif

/*
**  SENDMAIL -- Post mail to a set of destinations.
**
**	This is the basic mail router.  All user mail programs should
**	call this routine to actually deliver mail.  Sendmail in
**	turn calls a bunch of mail servers that do the real work of
**	delivering the mail.
**
**	Sendmail is driven by tables read in from /usr/lib/sendmail.cf
**	(read by readcf.c).  Some more static configuration info,
**	including some code that you may want to tailor for your
**	installation, is in conf.c.  You may also want to touch
**	daemon.c (if you have some other IPC mechanism), acct.c
**	(to change your accounting), names.c (to adjust the name
**	server mechanism).
**
**	Usage:
**		/usr/lib/sendmail [flags] addr ...
**
**		See the associated documentation for details.
**
**	Author:
**		Eric Allman, UCB/INGRES (until 10/81)
**			     Britton-Lee, Inc., purveyors of fine
**				database computers (from 11/81)
**		The support of the INGRES Project and Britton-Lee is
**			gratefully acknowledged.  Britton-Lee in
**			particular had absolutely nothing to gain from
**			my involvement in this project.
*/

#ifndef __HELIOS
int		NextMailer;	/* "free" index into Mailer struct */
char		*FullName;	/* sender's full name */
ENVELOPE	BlankEnvelope;	/* a "blank" envelope */
ENVELOPE	MainEnvelope;	/* the envelope around the basic letter */
ADDRESS		NullAddress =	/* a null address */
		{ "", "", NULL, "" };

/*
**  Pointers for setproctitle.
**	This allows "ps" listings to give more useful information.
**	These must be kept out of BSS for frozen configuration files
**		to work.
*/

# ifdef SETPROCTITLE
char		**Argv = NULL;		/* pointer to argument vector */
char		*LastArgv = NULL;	/* end of argv */
# endif /* SETPROCTITLE */
#else
char Channel_fd_str[9] ;
char Verbose_str[2] =
{ '0', (char) NULL } ;
char LogRequests_str[2] =
{ '0', (char) NULL } ;
/*
-- crf: not elegant, but the most straight forward way of doing things ...
*/
char *smtp_argv [7] =
{
	SMTP_NAME,
	Channel_fd_str,
	MyHostName,
	RealHostName,
	Verbose_str,
	LogRequests_str,
	NULL
} ;
#endif

#ifdef DAEMON
#ifndef SMTP
ERROR %%%%   Cannot have daemon mode without SMTP   %%%% ERROR
#endif /* SMTP */
#else
#error DAEMON must be defined
#endif /* DAEMON */

#ifndef __HELIOS
main(argc, argv, envp)
	int argc;
	char **argv;
	char **envp;
#else
int main(int argc, char **argv)
#endif	
{
	register char *p;
	char **av;
	register int i;
#ifndef __HELIOS
	extern char Version[];
	char *from;
	typedef int (*fnptr)();
	STAB *st;
	bool readconfig = TRUE;
	bool queuemode = FALSE;		/* process queue requests */
	bool NoName = FALSE;
	bool nothaw;
	static bool reenter = FALSE;
	char jbuf[MAXHOSTNAMELEN+1];	/* holds MyHostName */
	extern char **environ;
#endif

#ifdef __HELIOS
	extern int getdtablesize (void) ;
	int Channel_fd = 0 ;
	ENVELOPE Current_Envelope ;
	
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
	memset( &Current_Envelope, 0, sizeof (ENVELOPE) );   /* XXX - added by NC */
	CurEnv = &Current_Envelope ;
	
	LogRequests = 0 ;
#ifdef MEM_CHECK
	IOdebug ("entering sendmail: Bytes free : %d  Heap size : %d", 
		Malloc(-1), Malloc(-3));
#endif
#endif

#ifndef __HELIOS
	/*
	**  Check to see if we reentered.
	**	This would normally happen if e_putheader or e_putbody
	**	were NULL when invoked.
	*/

	if (reenter)
	{
		syserr("main: reentered!");
		abort();
	}
	reenter = TRUE;

#if !defined(hpux)
	/* Enforce use of local time */
	(void) unsetenv("TZ");
#endif /* hpux */

	/* Make mail act uniformly (resolver recursion disabled elsewhere) */
	(void) unsetenv("HOSTALIASES");
	(void) unsetenv("LOCALDOMAIN");

	/*
	**  Be sure we have enough file descriptors.
	**	But also be sure that 0, 1, & 2 are open.
	*/

	i = open("/dev/null", O_RDWR);
#else
	i = open("/null", O_RDWR);
#endif	
	while (i >= 0 && i < 2)
		i = dup(i);
#if defined(XPG3)
	for (i = (int) sysconf (_SC_OPEN_MAX); i > 2; --i)
#else
#ifndef __HELIOS
	for (i = getdtablesize(); i > 2; --i)
#else
	for (i = getdtablesize(); i > 3; --i)
#endif	
#endif /* XPG3 */
		(void) close(i);
	errno = 0;

#ifdef LOG
#ifdef LOG_MAIL
	openlog("sendmail", LOG_PID, LOG_MAIL);
#else 
	openlog("sendmail", LOG_PID);
#endif /* LOG_MAIL */
#endif /* LOG */

#ifndef __HELIOS
	/*
	**  Set default values for variables.
	**	These cannot be in initialized data space.
	*/

	setdefaults();

	/* set up the blank envelope */
	BlankEnvelope.e_puthdr = putheader;
	BlankEnvelope.e_putbody = putbody;
	BlankEnvelope.e_xfp = NULL;
	STRUCTCOPY(NullAddress, BlankEnvelope.e_from);
	CurEnv = &BlankEnvelope;
	STRUCTCOPY(NullAddress, MainEnvelope.e_from);

	/*
	**  Do a quick prescan of the argument list.
	**	We do this to find out if we can potentially thaw the
	**	configuration file.  If not, we do the thaw now so that
	**	the argument processing applies to this run rather than
	**	to the run that froze the configuration.
	*/

	argv[argc] = NULL;
	av = argv;
	nothaw = FALSE;
	while ((p = *++av) != NULL)
	{
		if (strncmp(p, "-C", 2) == 0)
		{
			ConfFile = &p[2];
			if (ConfFile[0] == '\0')
				ConfFile = "sendmail.cf";
			(void) setgid(getrgid());
			(void) setuid(getruid());
			nothaw = TRUE;
		}
		else if (strncmp(p, "-bz", 3) == 0)
			nothaw = TRUE;
		else if (strncmp(p, "-bd", 3) == 0 || strncmp(p, "-q", 2) == 0)
			NoName = TRUE;
		else if (strncmp(p, "-Z", 2) == 0)
		{
#ifdef _PATH_SENDMAILFC
			FreezeFile = &p[2];
			if (FreezeFile[0] == '\0')
				FreezeFile = "sendmail.fc";
			(void) setgid(getrgid());
			(void) setuid(getruid());
#else /* !_PATH_SENDMAILFC */
			/* Use printf since OutChannel isn't assigned yet */
			printf("Frozen configuration files not available\n");
#endif /* _PATH_SENDMAILFC */
		}
		else if (strncmp(p, "-d", 2) == 0)
		{
			tTsetup(tTdvect, sizeof tTdvect, "0-99.1");
			tTflag(&p[2]);
#if defined(XPG3)
			setvbuf(stdout, (char *) NULL, _IOLBF, BUFSIZ);
#else /* !XPG3 */
# if defined(unixpc)
			setbuf(stdout, (char *)NULL);
# else /* !unixpc */
			setlinebuf(stdout);
# endif /* unixpc */
#endif /* XPG3 */
			printf("Version %s\n", Version);
		}
	}

	if (tTd(3, 1))
		(void) getla();	/* prints load average in getla() */
#endif		
	InChannel = stdin;
	OutChannel = stdout;

#ifndef __HELIOS
#ifdef _PATH_SENDMAILFC

	/* Reset the environment only after a successful thaw() */
	if (!nothaw && (readconfig = !thaw(FreezeFile)) == FALSE)
	{
		for (i = 0; i < MAXUSERENVIRON && envp[i] != NULL; i++)
			UserEnviron[i] = newstr(envp[i]);
		UserEnviron[i] = NULL;
		environ = UserEnviron;
	}
	else
		for (i = 0; i < MAXUSERENVIRON && envp[i] != NULL; i++)
			;
#endif /* _PATH_SENDMAILFC */

#ifdef NAMED_BIND
	/*
	** Make sure the resolver library is initialized and that enough time
	** is allowed for non-local servers.
	*/
	res_init();
	_res.retrans = 30;
#endif /* NAMED_BIND */

# ifdef SETPROCTITLE
	/*
	**  Save start and extent of argv for setproctitle.
	*/

	Argv = argv;
	if (i > 0)
		LastArgv = envp[i - 1] + strlen(envp[i - 1]);
	else
		LastArgv = argv[argc - 1] + strlen(argv[argc - 1]);
# endif /* SETPROCTITLE */
#endif

#ifndef __HELIOS
	if (signal(SIGINT, SIG_IGN) != (SIG_TYPE (*)()) SIG_IGN)
		(void) signal(SIGINT, intsig);
	if (signal(SIGHUP, SIG_IGN) != (SIG_TYPE (*)()) SIG_IGN)
		(void) signal(SIGHUP, intsig);
	(void) signal(SIGTERM, intsig);
	(void) signal(SIGPIPE, SIG_IGN);
#else
	SETUP_SIG_HANDLER	
#endif

#ifndef __HELIOS
	OpMode = MD_DELIVER;
	OldUmask = umask(0);
	MotherPid = getpid();
	srand(MotherPid);
	FullName = (NoName) ? NULL : getenv("NAME");

	errno = 0;
	from = NULL;

	if (readconfig)
	{
		/* initialize some macros, etc. */
		initmacros();

		/* hostname */
		av = myhostname(jbuf, sizeof jbuf);
		if (jbuf[0] != '\0')
		{
			if (tTd(0, 4))
				printf("canonical name: %s\n", jbuf);
			p = newstr(jbuf);
			define('w', p, CurEnv);
			setclass('w', p);
			if ((p = index(jbuf, '.')) != NULL)
				*p = '\0';
			makelower(jbuf);
			define('k', newstr(jbuf), CurEnv);
		}
		while (av != NULL && *av != NULL)
		{
			if (tTd(0, 4))
				printf("\ta.k.a.: %s\n", *av);
			setclass('w', *av++);
		}

		/* version */
		define('v', Version, CurEnv);
	}

	/* current time */
	define('b', arpadate((char *) NULL), CurEnv);

#endif /* not __HELIOS */

	/*
	** Crack argv.
	*/

	av = argv;
	p = rindex(*av, '/');
	if (p++ == NULL)
		p = *av;
#ifndef __HELIOS		
	if (strcmp(p, "newaliases") == 0)
		OpMode = MD_INITALIAS;
	else if (strcmp(p, "mailq") == 0)
		OpMode = MD_PRINT;
	else if (strcmp(p, "smtpd") == 0)
		OpMode = MD_DAEMON;
	else if (strcmp(p, "bsmtp") == 0)
		OpMode = MD_BSMTP;
#endif	/* not __HELIOS */
	
	while ((p = *++av) != NULL && p[0] == '-')
	{
		switch (p[1])
		{
		  case 'b':	/* operations mode */
			switch (p[2])
			{
			  case MD_DAEMON:
# ifdef DAEMON
#ifndef __HELIOS
				if (getuid() != 0) {
					usrerr("Permission denied");
					exit (EX_USAGE);
				}
#else
				debugf ("Not testing for permission") ;
				(void) WritePid(_PATH_SENDMAILPID);
#endif				
# else
				usrerr("Daemon mode not implemented");
				ExitStat = EX_USAGE;
				break;
# endif /* DAEMON */
			  case MD_SMTP:
#ifndef __HELIOS		  
			  case MD_BSMTP:
#endif			  
# ifndef SMTP
				usrerr("I don't speak SMTP");
				ExitStat = EX_USAGE;
				break;
# endif /* SMTP */
#ifndef __HELIOS
			  case MD_ARPAFTP:
			  case MD_DELIVER:
			  case MD_VERIFY:
			  case MD_TEST:
			  case MD_INITALIAS:
			  case MD_PRINT:
			  case MD_FREEZE:
#endif
				OpMode = p[2];
				break;

			  default:
				usrerr("Invalid operation mode %c", p[2]);
				ExitStat = EX_USAGE;
				break;
			}
			break;

#ifndef __HELIOS
		  case 'C':	/* select configuration file (already done) */
			break;

		  case 'Z':	/* select frozen config file (already done) */
			break;

		  case 'd':	/* debugging -- redo in case frozen */
			tTsetup(tTdvect, sizeof tTdvect, "0-99.1");
			tTflag(&p[2]);
			tTflag(&p[2]);
#if defined(XPG3)
			setvbuf(stdout, (char *) NULL, _IOLBF, BUFSIZ);
#else /* !XPG3 */
# if defined(unixpc)
			setbuf(stdout, (char *)NULL);
# else /* !unixpc */
			setlinebuf(stdout);
# endif /* unixpc */
#endif /* XPG3 */
#ifdef NAMED_BIND
			if (tTd(8, 8))
			_res.options |= RES_DEBUG;
#endif /* NAMED_BIND */
			break;

		  case 'f':	/* from address */
		  case 'r':	/* obsolete -f flag */
			p += 2;
			if (*p == '\0' && ((p = *++av) == NULL || *p == '-'))
			{
				p = *++av;
				if (p == NULL || *p == '-')
				{
					usrerr("No \"from\" person");
					ExitStat = EX_USAGE;
					av--;
					break;
				}
			}
			if (from != NULL)
			{
				usrerr("More than one \"from\" person");
				ExitStat = EX_USAGE;
				break;
			}
			from = newstr(p);
			break;

		  case 'F':	/* set full name */
			p += 2;
			if (*p == '\0' && ((p = *++av) == NULL || *p == '-'))
			{
				usrerr("Bad -F flag");
				ExitStat = EX_USAGE;
				av--;
				break;
			}
			FullName = newstr(p);
			break;

		  case 'h':	/* hop count */
			p += 2;
			if (*p == '\0' && ((p = *++av) == NULL || !isdigit(*p)))
			{
				usrerr("Bad hop count (%s)", p);
				ExitStat = EX_USAGE;
				av--;
				break;
			}
			CurEnv->e_hopcount = atoi(p);
			break;
		
		  case 'n':	/* don't alias */
			NoAlias = TRUE;
			break;

		  case 'o':	/* set option */
			setoption(p[2], &p[3], FALSE, TRUE);
			break;

		  case 'q':	/* run queue files at intervals */
# ifdef QUEUE
			if (getuid() != 0) {
				usrerr("Permission denied");
				exit (EX_USAGE);
			}
			queuemode = TRUE;
			QueueIntvl = convtime(&p[2]);
# else /* !QUEUE */
			usrerr("I don't know about queues");
			ExitStat = EX_USAGE;
# endif /* QUEUE */
			break;

		  case 't':	/* read recipients from message */
			GrabTo = TRUE;
			break;

			/* compatibility flags */
		  case 'c':	/* connect to non-local mailers */
		  case 'e':	/* error message disposition */
		  case 'i':	/* don't let dot stop me */
		  case 'm':	/* send to me too */
		  case 'T':	/* set timeout interval */
#endif		  
		  case 'v':	/* give blow-by-blow description */
#ifndef __HELIOS		  
			setoption(p[1], &p[2], FALSE, TRUE);
#else
			Verbose = TRUE ;
#endif			
			break;

#ifdef __HELIOS
		  case 'l':	/* log mail requests */
			LogRequests ++ ;
			break;
#endif

#ifndef __HELIOS
		  case 's':	/* save From lines in headers */
			setoption('f', &p[2], FALSE, TRUE);
			break;

# ifdef DBM
		  case 'I':	/* initialize alias DBM file */
			OpMode = MD_INITALIAS;
			break;
# endif /* DBM */
#endif
		}
	}

	/*
	**  Do basic initialization.
	**	Read system control file.
	**	Extract special fields for local use.
	*/

#ifndef __HELIOS
	if (OpMode == MD_FREEZE || readconfig)
		readcf(ConfFile);

	switch (OpMode)
	{
	  case MD_FREEZE:
#ifdef _PATH_SENDMAILFC
		/* this is critical to avoid forgeries of the frozen config */
		(void) setgid(getgid());
		(void) setuid(getuid());

		/* freeze the configuration */
		freeze(FreezeFile);
		exit(EX_OK);
#else /* !_PATH_SENDMAILFC */
		usrerr("Frozen configuration files not available");
		exit(EX_USAGE);
#endif /* _PATH_SENDMAILFC */

	  case MD_INITALIAS:
		Verbose = TRUE;
		break;
	}

	/* do heuristic mode adjustment */
	if (Verbose)
	{
		/* turn off noconnect option */
		setoption('c', "F", TRUE, FALSE);

		/* turn on interactive delivery */
		setoption('d', "", TRUE, FALSE);
	}

	/* our name for SMTP codes */
	expand("\001j", jbuf, &jbuf[sizeof jbuf - 1], CurEnv);
	MyHostName = jbuf;
#endif

#ifdef __HELIOS
	(void) myhostname(MyHostName, sizeof MyHostName);
#endif /* __HELIOS */

#ifndef __HELIOS
	/* the indices of local and program mailers */
	st = stab("local", ST_MAILER, ST_FIND);
	if (st == NULL)
		syserr("No local mailer defined");
	else
		LocalMailer = st->s_mailer;
	st = stab("prog", ST_MAILER, ST_FIND);
	if (st == NULL)
		syserr("No prog mailer defined");
	else
		ProgMailer = st->s_mailer;
#endif

	/* operate in queue directory */
	
	if (chdir(QueueDir) < 0)
	{
	  syserr("cannot chdir(%s)", QueueDir);

	  exit(EX_SOFTWARE);
	}

#ifndef __HELIOS
	/*
	**  Do operation-mode-dependent initialization.
	*/

	switch (OpMode)
	{
	  case MD_PRINT:
		/* print the queue */
#ifdef QUEUE
		dropenvelope(CurEnv);
		printqueue();
		exit(EX_OK);
#else /* !QUEUE */
		usrerr("No queue to print");
		finis();
#endif /* QUEUE */

	  case MD_INITALIAS:
		/* initialize alias database */
		initaliases(TRUE);
		exit(EX_OK);

	  case MD_DAEMON:
		/* don't open alias database -- done in srvrsmtp */
		break;

	  default:
		/* open the alias database */
		initaliases(FALSE);
		break;
	}

	if (tTd(0, 15))
	{
		/* print configuration table (or at least part of it) */
		printrules();
		for (i = 0; i < MAXMAILERS; i++)
		{
			register struct mailer *m = Mailer[i];
			int j;

			if (m == NULL)
				continue;
			printf("mailer %d (%s): P=%s S=%d/%d R=%d/%d M=%ld F=",
				i, m->m_name, m->m_mailer,
				m->m_se_rwset, m->m_sh_rwset,
				m->m_re_rwset, m->m_rh_rwset, m->m_maxsize);
			for (j = '\0'; j <= '\177'; j++)
				if (bitnset(j, m->m_flags))
					(void) putchar(j);
			printf(" E=");
			xputs(m->m_eol);
			printf("\n");
		}
	}

	/*
	**  Switch to the main envelope.
	*/

	CurEnv = newenvelope(&MainEnvelope);
	MainEnvelope.e_flags = BlankEnvelope.e_flags;

	/*
	**  If test mode, read addresses from stdin and process.
	*/

	if (OpMode == MD_TEST)
	{
		char buf[MAXLINE];

		printf("ADDRESS TEST MODE\nEnter <ruleset> <address>\n");
		printf("[Note: No initial ruleset 3 call]\n");
		for (;;)
		{
			register char **pvp;
			char *q;
			extern char *DelimChar;

			printf("> ");
			(void) fflush(stdout);
			if (fgets(buf, sizeof buf, stdin) == NULL)
				finis();
			for (p = buf; isspace(*p); p++)
				continue;
			q = p;
			while (*p != '\0' && !isspace(*p))
				p++;
			if (*p == '\0')
				continue;
			*p = '\0';
			if (invalidaddr(p+1))
				continue;
			do
			{
				char pvpbuf[PSBUFSIZE];

				pvp = prescan(++p, '\0', pvpbuf);
				if (pvp == NULL)
					continue;
				/* rewrite(pvp, 3); */
				p = q;
				while (*p != '\0')
				{
					rewrite(pvp, atoi(p));
					while (*p != '\0' && *p++ != ',')
						continue;
				}
			} while (*(p = DelimChar) != '\0');
		}
	}

# ifdef QUEUE
	/*
	**  If collecting stuff from the queue, go start doing that.
	*/

	if (queuemode && OpMode != MD_DAEMON && QueueIntvl == 0)
	{
		runqueue(FALSE);
		finis();
	}
# endif /* QUEUE */
#endif /* not __HELIOS */

	/*
	**  If a daemon, wait for a request.
	**	getrequests will always return in a child.
	**	If we should also be processing the queue, start
	**		doing it in background.
	**	We check for any errors that might have happened
	**		during startup.
	*/

#ifndef __HELIOS
	if (OpMode == MD_DAEMON || QueueIntvl != 0)
#else
	if (OpMode == MD_DAEMON)
#endif	
	{
#ifndef __HELIOS
		if (!tTd(0, 1))
		{
			/* put us in background */
			i = fork();
			if (i < 0)
				syserr("daemon: cannot fork");
			if (i != 0)
				exit(0);

			/* get our pid right */
			MotherPid = getpid();

			/* disconnect from our controlling tty */
			disconnect(TRUE);
		}
#else
		debugf ("disconnecting ...") ;
/*
-- crf: necessary ?
*/
		disconnect() ;
#endif

# ifdef QUEUE
		if (queuemode)
		{
			runqueue(TRUE);
			if (OpMode != MD_DAEMON)
				for (;;)
					pause();
		}
# endif /* QUEUE */

#ifndef __HELIOS
		dropenvelope(CurEnv);
#endif

#ifdef DAEMON
#ifndef __HELIOS
		getrequests();

		/* at this point we are in a child: reset state */
		OpMode = MD_SMTP;
		(void) newenvelope(CurEnv);
		openxscript(CurEnv);
#endif		
#endif /* DAEMON */
	}

# ifdef SMTP
#ifndef __HELIOS
	/*
	**  If running SMTP protocol, start collecting and executing
	**  commands.  This will never return.
	*/

	if (OpMode == MD_SMTP || OpMode == MD_BSMTP) {
		bool batched = (OpMode == MD_BSMTP);
		OpMode = MD_SMTP;
#ifdef notdef
/* test without this first */
		/* have to run unbuffered or else will lose synchronization */
		if (batched)
			setbuf(InChannel, (char *) NULL);
#endif /* notdef */
		smtp(batched);
	}
#else
	if ((OpMode == MD_SMTP) || (OpMode == MD_DAEMON))
	{
		register int pid;
		int status ;
		for (;;)
		{
			if (OpMode == MD_DAEMON)
				Channel_fd = getrequests();

			strcpy (Channel_fd_str, itoa (Channel_fd)) ;
			Verbose_str[0] = (char) Verbose + '0' ;
			LogRequests_str[0] = LogRequests + '0' ;

			if ((pid = vfork()) < 0)
				syslog (LOG_ERR, "cannot vfork: %m") ;
			if (!pid)
			{
				if (execve (SMTP_PATHNAME, smtp_argv, environ) < 0)
					syslog (LOG_ERR, "%s: %m", SMTP_PATHNAME) ;
				_exit(0) ;
			}
			waitpid (pid, &status, 0) ;
			if (OpMode == MD_SMTP)
				finis() ;
			else
				(void) close (Channel_fd) ;
		}
	}
#endif	
# endif /* SMTP */

#ifndef __HELIOS
	/*
	**  Do basic system initialization and set the sender
	*/

	initsys();
	setsender(from);

	if (OpMode != MD_ARPAFTP && *av == NULL && !GrabTo)
	{
		usrerr("Recipient names must be specified");

		/* collect body for UUCP return */
		if (OpMode != MD_VERIFY)
			collect(FALSE);
		finis();
	}
	if (OpMode == MD_VERIFY)
		SendMode = SM_VERIFY;
	/*
	**  Scan argv and deliver the message to everyone.
	*/

	sendtoargv(av);

	/* if we have had errors sofar, arrange a meaningful exit stat */
	if (Errors > 0 && ExitStat == EX_OK)
		ExitStat = EX_USAGE;
#endif		

#ifndef __HELIOS
	/*
	**  Read the input mail.
	*/

	CurEnv->e_to = NULL;
	if (OpMode != MD_VERIFY || GrabTo)
		collect(FALSE);
#else
	Init_Env (CurEnv) ;
	if (*av)
		collect(FALSE);
	else
		usrerr("Recipient name(s) must be specified");
#endif

	errno = 0;

#ifndef __HELIOS
	if (tTd(1, 1))
		printf("From person = \"%s\"\n", CurEnv->e_from.q_paddr);
#endif

	/*
	**  Actually send everything.
	**	If verifying, just ack.
	*/

#ifdef __HELIOS
	strcpy (CurEnv->e_from, getlogin()) ;
	debugf ("e_from = %s", CurEnv->e_from) ;
	{
		int rcode = 0 ;
		while (*av)
		{
			strcpy (CurEnv->e_to, *av) ;
			rcode += deliver (CurEnv) ;
			av ++ ;
		}
		if (!rcode)
			unlink_temps (CurEnv) ;
	}
#else
	CurEnv->e_from.q_flags |= QDONTSEND;
	CurEnv->e_to = NULL;
	sendall(CurEnv, SM_DEFAULT);

	/* collect statistics (done after sendall, since sendall may fork) */
	if (OpMode != MD_VERIFY)
		markstats(CurEnv, (ADDRESS *) NULL);
#endif	

	/*
	** All done.
	*/

	finis();
}

#ifndef __HELIOS
/*
-- crf: this is now in extras.c
*/

/* new page */
/*
**  INTSIG -- clean up on interrupt
**
**	This just arranges to exit.  It pessimises in that it
**	may resend a message.
**
**	Parameters:
**		none.
**
**	Returns:
**		none.
**
**	Side Effects:
**		Unlocks the current job.
*/

static void
intsig()
{
	FileName = NULL;
	unlockqueue(CurEnv);
	exit(EX_OK);
}

/*
-- crf: this is now in extras.c
*/

/* new page */
/*
**  FINIS -- Clean up and exit.
**
**	Parameters:
**		none
**
**	Returns:
**		never
**
**	Side Effects:
**		exits sendmail
*/

void
finis()
{
	if (tTd(2, 1))
		printf("\n====finis: stat %d e_flags %o\n", ExitStat, CurEnv->e_flags);

	/* clean up temp files */
	CurEnv->e_to = NULL;
	dropenvelope(CurEnv);

	/* post statistics */
	poststats(StatFile);

	/* and exit */
# ifdef LOG
	if (LogLevel > 11)
		syslog(LOG_DEBUG, "finis, pid=%d", getpid());
# endif /* LOG */
	if (ExitStat == EX_TEMPFAIL)
		ExitStat = EX_OK;
	exit(ExitStat);
}

/* new page */
/*
**  INITMACROS -- initialize the macro system
**
**	This just involves defining some macros that are actually
**	used internally as metasymbols to be themselves.
**
**	Parameters:
**		none.
**
**	Returns:
**		none.
**
**	Side Effects:
**		initializes several macros to be themselves.
*/

struct metamac	MetaMacros[] =
{
	/* LHS pattern matching characters */
	'*', MATCHZANY,	'+', MATCHANY,	'-', MATCHONE,	'=', MATCHCLASS,
	'~', MATCHNCLASS,

	/* these are RHS metasymbols */
	'#', CANONNET,	'@', CANONHOST,	':', CANONUSER,	'>', CALLSUBR,

	/* the conditional operations */
	'?', CONDIF,	'|', CONDELSE,	'.', CONDFI,

	/* and finally the hostname and database lookup characters */
	'[', HOSTBEGIN,	']', HOSTEND,
	'(', KEYBEGIN,	')', KEYEND,

#ifdef MACVALUE
	/* run-time macro expansion, not at freeze time */
	'&', MACVALUE,
#endif /* MACVALUE */
#ifdef QUOTE822
	'!', QUOTE822,	/* quote next macro if RFC822 requires it */
#endif /* QUOTE822 */
	'\0'
};

static void
initmacros()
{
	register struct metamac *m;
	char buf[5];
	register int c;

	for (m = MetaMacros; m->metaname != '\0'; m++)
	{
		buf[0] = m->metaval;
		buf[1] = '\0';
		define(m->metaname, newstr(buf), CurEnv);
	}
	buf[0] = MATCHREPL;
	buf[2] = '\0';
	for (c = '0'; c <= '9'; c++)
	{
		buf[1] = c;
		define(c, newstr(buf), CurEnv);
	}
}
/* new page */

#ifdef _PATH_SENDMAILFC
/*
**  FREEZE -- freeze BSS & allocated memory
**
**	This will be used to efficiently load the configuration file.
**
**	Parameters:
**		freezefile -- the name of the file to freeze to.
**
**	Returns:
**		none.
**
**	Side Effects:
**		Writes BSS and malloc'ed memory to freezefile
*/

union frz
{
	char		frzpad[BUFSIZ];	/* insure we are on a BUFSIZ boundary */
	struct
	{
		time_t	frzstamp;	/* timestamp on this freeze */
		char	*frzbrk;	/* the current break */
		char	*frzedata;	/* address of edata */
		char	*frzend;	/* address of end */
		char	frzver[252];	/* sendmail version */
		char	frzdatecompiled[64];	/* sendmail compilation date */
	} frzinfo;
};

static void
freeze(freezefile)
	const char *freezefile;
{
	int f;
	union frz fhdr;
	extern char edata, end;
	extern char *sbrk();
	extern char Version[];
	extern char datecompiled[];

	if (freezefile == NULL)
		return;
# ifdef YP
	{
		/*
		 * NIS (nee YP) state is saved in the freeze, the readback
		 * of the freeze file destroys similar information.  This
		 * causes strange results later due to the unexpected closing
		 * of a file descriptor using a stale copy.
		 */

		char *domain;
		if (yp_get_default_domain(&domain) == 0)
			yp_unbind(domain);
	}
# endif /* YP */

	/* try to open the freeze file */
	f = creat(freezefile, FileMode);
	if (f < 0)
	{
		syserr("Cannot freeze %s", freezefile);
		errno = 0;
		return;
	}

	/* build the freeze header */
	fhdr.frzinfo.frzstamp = curtime();
	fhdr.frzinfo.frzbrk = sbrk(0);
	fhdr.frzinfo.frzedata = &edata;
	fhdr.frzinfo.frzend = &end;
	(void) strcpy(fhdr.frzinfo.frzver, Version);
	(void) strcpy(fhdr.frzinfo.frzdatecompiled, datecompiled);

	/* write out the freeze header */
	if (write(f, (char *) &fhdr, sizeof fhdr) != sizeof fhdr ||
	    write(f, (char *) &edata, (int) (fhdr.frzinfo.frzbrk - &edata)) !=
					(int) (fhdr.frzinfo.frzbrk - &edata))
	{
		syserr("Cannot freeze %s", freezefile);
	}

	/* fine, clean up */
	(void) close(f);
}
/* new page */
/*
**  THAW -- read in the frozen configuration file.
**
**	Parameters:
**		freezefile -- the name of the file to thaw from.
**
**	Returns:
**		TRUE if it successfully read the freeze file.
**		FALSE otherwise.
**
**	Side Effects:
**		reads freezefile in to BSS area.
*/

static
thaw(freezefile)
	const char *freezefile;
{
	int f;
	union frz fhdr;
	extern char edata, end;
	extern char Version[];
	extern char datecompiled[];
	extern caddr_t brk();

	if (freezefile == NULL)
		return (FALSE);

	/* open the freeze file */
	f = open(freezefile, 0);
	if (f < 0)
	{
# ifdef LOG
		syslog(LOG_WARNING, "Cannot open frozen config file %s: %m",
			freezefile);
# endif /* LOG */
		errno = 0;
		return (FALSE);
	}
# ifdef YP
	{
		/*
		 * NIS (nee YP) state is saved in the freeze, the readback
		 * of the freeze file destroys similar information.  This
		 * causes strange results later due to the unexpected closing
		 * of a file descriptor using a stale copy.
		 */

		char *domain;
		if (yp_get_default_domain(&domain) == 0)
			yp_unbind(domain);
	}
# endif /* YP */

	/* read in the header */
	if (read(f, (char *) &fhdr, sizeof fhdr) < sizeof fhdr)
	{
		syserr("Cannot read frozen config file");
		(void) close(f);
		return (FALSE);
	}
	if ( fhdr.frzinfo.frzedata != &edata ||
	    fhdr.frzinfo.frzend != &end ||
	    strcmp(fhdr.frzinfo.frzver, Version) != 0 ||
	    strcmp(fhdr.frzinfo.frzdatecompiled, datecompiled) != 0)
	{
# ifdef LOG
		syslog(LOG_WARNING, "Wrong version of frozen config file");
# endif /* LOG */
		(void) close(f);
		return (FALSE);
	}

	/* arrange to have enough space */
	if (brk(fhdr.frzinfo.frzbrk) == (caddr_t) -1)
	{
		syserr("Cannot break to %x", fhdr.frzinfo.frzbrk);
		(void) close(f);
		return (FALSE);
	}

	/* now read in the freeze file */
	if (read(f, (char *) &edata, (int) (fhdr.frzinfo.frzbrk - &edata)) !=
					(int) (fhdr.frzinfo.frzbrk - &edata))
	{
		syserr("Cannot read frozen config file");
		/* oops!  we have trashed memory..... */
		(void) write(2, "Cannot read freeze file\n", 24);
		_exit(EX_SOFTWARE);
	}

	(void) close(f);
	return (TRUE);
}
#endif /* _PATH_SENDMAILFC */
#endif

/* new page */
/*
**  DISCONNECT -- remove our connection with any foreground process
**
**	Parameters:
**		fulldrop -- if set, we should also drop the controlling
**			TTY if possible -- this should only be done when
**			setting up the daemon since otherwise UUCP can
**			leave us trying to open a dialin, and we will
**			wait for the carrier.
**
**	Returns:
**		none
**
**	Side Effects:
**		Trys to insure that we are immune to vagaries of
**		the controlling tty.
*/

void
#ifndef __HELIOS
disconnect(fulldrop)
	bool fulldrop;
#else
disconnect()
#endif	
{
	int fd;

#ifndef __HELIOS
	if (tTd(52, 1))
		printf("disconnect: In %d Out %d\n", fileno(InChannel),
						fileno(OutChannel));
	if (tTd(52, 5))
	{
		printf("don't\n");
		return;
	}

	/* be sure we don't get nasty signals */
	(void) signal(SIGHUP, SIG_IGN);
	(void) signal(SIGINT, SIG_IGN);
	(void) signal(SIGQUIT, SIG_IGN);

	/* we can't communicate with our caller, so.... */
	HoldErrs = TRUE;
	setoption('e', "m", TRUE, TRUE);
#endif	
	Verbose = FALSE;

	/* all input from /dev/null */
	if (InChannel != stdin)
	{
		(void) fclose(InChannel);
		InChannel = stdin;
	}
#ifndef __HELIOS
	(void) freopen("/dev/null", "r", stdin);
#else
	(void) freopen("/null", "r", stdin);
#endif
	/* output to the transcript */
	if (OutChannel != stdout)
	{
		(void) fclose(OutChannel);
		OutChannel = stdout;
	}
	if (CurEnv->e_xfp == NULL)
#ifndef __HELIOS
		CurEnv->e_xfp = fopen("/dev/null", "w");
#else
		CurEnv->e_xfp = fopen("/null", "w");
#endif		
	(void) fflush(stdout);
	(void) close(1);
	(void) close(2);
	while ((fd = dup(fileno(CurEnv->e_xfp))) < 2 && fd > 0)
		continue;

#ifdef __HELIOS
/*
-- crf: keep the compiler happy ...
*/
#define BSD	0
#endif

#ifndef __HELIOS
	/* drop our controlling TTY completely if possible */
	if (fulldrop)
	{
#if defined(XPG3)
	/*
	 * setsid will provide a new session w/o any tty associated at all.
	 * STANDARDS CONFORMANCE
	 *     setpgrp: SVID2, XPG2
	 *     setsid: XPG3, POSIX.1, FIPS 151-1
	 */
	(void) setsid();
#else /* !XPG3 */
# if BSD > 43
		daemon(1, 1);
# else /* BSD <= 43 */
#  ifdef TIOCNOTTY
		fd = open("/dev/tty", 2);
		if (fd >= 0)
		{
			(void) ioctl(fd, (int) TIOCNOTTY, (char *) 0);
			(void) close(fd);
		}
		(void) setpgrp(0, 0);
#  endif /* TIOCNOTTY */
# endif /* BSD */
# if !defined(BSD) && !defined(TIOCNOTTY) && defined(SYSV)
		setpgrp();
		if (fork() != 0)
			exit(0);
# endif /* !BSD && !TIOCNOTTY && SYSV */
#endif /* XPG3 */
		errno = 0;
	}
#endif

# ifdef LOG
	if (LogLevel > 11)
		syslog(LOG_DEBUG, "in background, pid=%d", getpid());
# endif /* LOG */

	errno = 0;
}
