/*
 * Copyright (c) 1983, 1988 The Regents of the University of California.
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

#ifdef lint
char copyright[] =
"@(#) Copyright (c) 1983, 1988 The Regents of the University of California.\n\
 All rights reserved.\n";
#endif /* not lint */

#ifdef lint
static char sccsid[] = "@(#)rshd.c	5.17.1.2 (Berkeley) 2/7/89";
#endif /* not lint */

/*
 * remote shell server:
 *	[port]\0
 *	remuser\0
 *	locuser\0
 *	command\0
 *	data
 */
#include <sys/param.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/file.h>
#include <sys/time.h>

#include <netinet/in.h>

#include <arpa/inet.h>

#include <stdio.h>
#include <errno.h>
#include <pwd.h>
#include <signal.h>
#include <netdb.h>
#include <syslog.h>
#include <stdarg.h>
#include <strings.h>
#include <stdlib.h>
#include <sys/wait.h>
  
#ifdef __HELIOS
#include <nonansi.h>
#include <sem.h>
#include <getopt.h>
#ifdef NEW_SYSTEM
  #include <bsd.h>
#endif
#endif

extern int	errno;
int	keepalive = 1;

/*VARARGS1*/
void	error(char * fmt, ...);

#ifdef __HELIOS
int pid;
#if 0
void do_stderr(int pv0, int s);
#else
/*
-- crf: 16/03/93
-- Pass synchronization channel
*/
void do_stderr(int pv0, int s, int synch0);
Semaphore sem ;	/* synchronize pipe closure */
#endif
#endif

char * ProgName;



char	username[20] = "USER=";
char	homedir[64] = "HOME=";
char	shell[64] = "SHELL=";
#ifdef __HELIOS
char	uid[] = "_UID=00000000";
char	gid[] = "_GID=00000000";
char	*envinit[] =
	    {uid, gid, homedir, shell, "PATH=/helios/bin:/helios/local/bin:", username, 0};
#else
char	*envinit[] =
	    {homedir, shell, "PATH=/usr/ucb:/bin:/usr/bin:", username, 0};
#endif
extern char	**environ;

#ifdef __HELIOS

static struct stub
{
	char	*stub;
	int	len;
} stubs[] =
{	
	{ "/bin",	4 },
	{ "/usr/ucb",	8 },
	{ 0, 0 }
};

/* This routine translates Unix command directory names into helios	*/
/* directory names. This is so that rcp (which uses direct UNIX names)	*/
/* will work between Unix and Helios machines.				*/
void xlatecmd(char *cmd)
{
	static char buf[NCARGS+1];
	struct stub *s = stubs;

	while( s->stub )
	{	
		if( strncmp(s->stub,cmd,s->len) == 0 ) break;
		s++;
	}
	
	if( s->stub == 0 ) return;
	
	strcpy(buf,"/helios/bin");
	strcat(buf,cmd+s->len);
	strcpy(cmd,buf);
}

#endif

void
getstr(
       char *	buf,
       int	cnt,
       char *	err )
{
	char c;

	do {
		if (read(0, &c, 1) != 1)
			exit(1);
		*buf++ = c;
		if (--cnt == 0) {
			error("%s too long\n", err,0,0);
			exit(1);
		}
	} while (c != 0);
}

/*
 * Check whether host h is in our local domain,
 * as determined by the part of the name following
 * the first '.' in its name and in ours.
 * If either name is unqualified (contains no '.'),
 * assume that the host is local, as it will be
 * interpreted as such.
 */
int
local_domain( char * h )
{
	char localhost[MAXHOSTNAMELEN];
	char *p1, *p2 = index(h, '.');

	(void) gethostname(localhost, sizeof(localhost));
	p1 = index(localhost, '.');
	if (p1 == NULL || p2 == NULL || !strcasecmp(p1, p2))
		return(1);
	return(0);
}

void
doit( struct sockaddr_in *fromp )
{
	static char cmdbuf[NCARGS+1];
	char *cp;
	char locuser[16], remuser[16];
	struct passwd *pwd;
	int s = 0;
	struct hostent *hp;
	char *hostname;
	short port;
	int pv[2], cc;
	int synch [2] ;
#ifndef __HELIOS
	int nfd;
	fd_set ready, readfrom;
	static char buf[BUFSIZ];
	char sig;
	int one = 1;
#endif
	static char remotehost[2 * MAXHOSTNAMELEN + 1];

	(void) signal(SIGINT, SIG_DFL);
	(void) signal(SIGQUIT, SIG_DFL);
	(void) signal(SIGTERM, SIG_DFL);
#ifdef DEBUG
	{ int t = open("/dev/tty", 2);
	  if (t >= 0) {
		ioctl(t, TIOCNOTTY, (char *)0);
		(void) close(t);
	  }
	}
#endif
	fromp->sin_port = ntohs((u_short)fromp->sin_port);
	if (fromp->sin_family != AF_INET) {
		syslog(LOG_ERR, "malformed from address\n");
		exit(1);
	}
#ifdef IP_OPTIONS
      {
	static u_char optbuf[BUFSIZ/3];
	char *cp;
	static char lbuf[BUFSIZ];
	char *lp;
	int optsize = sizeof(optbuf), ipproto;
	struct protoent *ip;

	if ((ip = getprotobyname("ip")) != NULL)
		ipproto = ip->p_proto;
	else
		ipproto = IPPROTO_IP;
	if (getsockopt(0, ipproto, IP_OPTIONS, (char *)optbuf, &optsize) == 0 &&
	    optsize != 0) {
		lp = lbuf;
		for (cp = optbuf; optsize > 0; cp++, optsize--, lp += 3)
			sprintf(lp, " %2.2x", *cp);
		syslog(LOG_NOTICE,
		    "Connection received using IP options (ignored):%s", lbuf);
		if (setsockopt(0, ipproto, IP_OPTIONS,
		    (char *)NULL, &optsize) != 0) {
			syslog(LOG_ERR, "setsockopt IP_OPTIONS NULL: %m");
			exit(1);
		}
	}
      }
#endif

	if (fromp->sin_port >= IPPORT_RESERVED ||
	    fromp->sin_port < IPPORT_RESERVED/2) {
		syslog(LOG_NOTICE, "Connection from %s on illegal port",
			inet_ntoa(fromp->sin_addr));
		exit(1);
	}

	(void) alarm(60);
	port = 0;
	for (;;) {
		char c;
		if ((cc = read(0, &c, 1)) != 1) {
			if (cc < 0)
				syslog(LOG_NOTICE, "read: %m");
			shutdown(0, 1+1);
			exit(1);
		}
		if (c == 0)
			break;
		port = port * 10 + c - '0';
	}

	(void) alarm(0);
	if (port != 0) {
		int lport = IPPORT_RESERVED - 1;
		s = rresvport(&lport);
		if (s < 0) {
			syslog(LOG_ERR, "can't get stderr port: %m");
			exit(1);
		}
		if (port >= IPPORT_RESERVED) {
			syslog(LOG_ERR, "2nd port not reserved\n");
			exit(1);
		}
		fromp->sin_port = htons((u_short)port);
		if (connect(s, (struct sockaddr *)fromp, sizeof (*fromp)) < 0) {
			syslog(LOG_INFO, "connect second port: %m");
			exit(1);
		}
	}

#ifdef notdef
	/* from inetd, socket is already on 0, 1, 2 */
	dup2(f, 0);
	dup2(f, 1);
	dup2(f, 2);
#endif
	hp = gethostbyaddr((char *)&fromp->sin_addr, sizeof (struct in_addr),
		fromp->sin_family);
	if (hp) {
		/*
		 * If name returned by gethostbyaddr is in our domain,
		 * attempt to verify that we haven't been fooled by someone
		 * in a remote net; look up the name and check that this
		 * address corresponds to the name.
		 */
		if (local_domain(hp->h_name)) {
			strncpy(remotehost, hp->h_name, sizeof(remotehost) - 1);
			remotehost[sizeof(remotehost) - 1] = 0;
			hp = gethostbyname(remotehost);
			if (hp == NULL) {
				syslog(LOG_INFO,
				    "Couldn't look up address for %s",
				    remotehost);
				error("Couldn't look up address for your host");
				exit(1);
			} else for (; ; hp->h_addr_list++) {
				if (!bcmp(hp->h_addr_list[0],
				    (caddr_t)&fromp->sin_addr,
				    sizeof(fromp->sin_addr)))
					break;
				if (hp->h_addr_list[0] == NULL) {
					syslog(LOG_NOTICE,
					  "Host addr %s not listed for host %s",
					    inet_ntoa(fromp->sin_addr),
					    hp->h_name);
					error("Host address mismatch");
					exit(1);
				}
			}
		}
		hostname = hp->h_name;
	} else
		hostname = inet_ntoa(fromp->sin_addr);

	getstr(remuser, sizeof(remuser), "remuser");
	getstr(locuser, sizeof(locuser), "locuser");
	getstr(cmdbuf, sizeof(cmdbuf), "command");
	setpwent();
	pwd = getpwnam(locuser);
	if (pwd == NULL) {
		error("Login incorrect.\n");
		exit(1);
	}
	endpwent();
	if (chdir(pwd->pw_dir) < 0) {
		(void) chdir("/");
#ifdef notdef
		error("No remote directory.\n");
		exit(1);
#endif
	}

#ifdef __HELIOS
	if (ruserok(hostname, 0, remuser, locuser) < 0) {
		error("Permission denied.\n");
		exit(1);
	}
	if ( !access("/helios/etc/nologin", F_OK)) {
		error("Logins currently disabled.\n");
		exit(1);
	}
#else
	if (pwd->pw_passwd != 0 && *pwd->pw_passwd != '\0' &&
	    ruserok(hostname, pwd->pw_uid == 0, remuser, locuser) < 0) {
		error("Permission denied.\n");
		exit(1);
	}
	if (pwd->pw_uid && !access("/etc/nologin", F_OK)) {
		error("Logins currently disabled.\n");
		exit(1);
	}
#endif

	(void) write(2, "\0", 1);

	if (port) {
		if (pipe(pv) < 0) {
			error("Can't make pipe.\n");
			exit(1);
		}
#ifdef __HELIOS
#if 0
/*
-- crf: 16/03/93
-- Timing problems, move Fork to after exec
*/
		Fork(3000,do_stderr,2*sizeof(int),pv[0],s);
#endif
		dup2(pv[1], 2);		
#else
		pid = fork();
		if (pid == -1)  {
			error("Try again.\n");
			exit(1);
		}
		if (pv[0] > s)
			nfd = pv[0];
		else
			nfd = s;
		nfd++;
		if (pid) {
			(void) close(0); (void) close(1); (void) close(2);
			(void) close(pv[1]);
			FD_ZERO(&readfrom);
			FD_SET(s, &readfrom);
			FD_SET(pv[0], &readfrom);
			ioctl(pv[0], FIONBIO, (char *)&one);
			/* should set s nbio! */
			do {
				ready = readfrom;
				if (select(nfd, &ready, (fd_set *)0,
				    (fd_set *)0, (struct timeval *)0) < 0)
					break;
				if (FD_ISSET(s, &ready)) {
					if (read(s, &sig, 1) <= 0)
						FD_CLR(s, &readfrom);
					else
						kill(-pid, sig);
				}
				if (FD_ISSET(pv[0], &ready)) {
					errno = 0;
					cc = read(pv[0], buf, sizeof (buf));
					if (cc <= 0) {
						shutdown(s, 1+1);
						FD_CLR(pv[0], &readfrom);
					} else
						(void) write(s, buf, cc);
				}
			} while (FD_ISSET(s, &readfrom) ||
			    FD_ISSET(pv[0], &readfrom));
			exit(0);
		}
		setpgid(0, getpid());
		(void) close(s); (void) close(pv[0]);
		dup2(pv[1], 2);
#endif
	}

	if (*pwd->pw_shell == '\0')
		pwd->pw_shell = "/bin/shell";
	(void) setgid((gid_t)pwd->pw_gid);
	initgroups(pwd->pw_name, pwd->pw_gid);
	(void) setuid((uid_t)pwd->pw_uid);
#ifndef __HELIOS
	environ = envinit;
#endif
	strncat(homedir, pwd->pw_dir, sizeof(homedir)-6);
	strncat(shell, pwd->pw_shell, sizeof(shell)-7);
	strncat(username, pwd->pw_name, sizeof(username)-6);

	cp = rindex(pwd->pw_shell, '/');
	if (cp)
		cp++;
	else
		cp = pwd->pw_shell;
#ifdef __HELIOS
/*
-- crf: 16/03/93
-- synchronization channel
*/
	if (pipe (synch) < 0)
	{
		error("Can't make pipe.\n");
		exit(1);
	}

	xlatecmd(cmdbuf);
	if((pid=vfork()) == 0 )
	{
		close(pv[0]);
		close(pv[1]);
		close(3);
		close (synch[0]) ;
		close (synch[1]) ;
		execle(pwd->pw_shell, cp, "-c", cmdbuf, 0, envinit);
		perror(pwd->pw_shell);
		exit(1);
	}

	Fork (3000,do_stderr,3*sizeof(int),pv[0],s,synch[0]) ;

	wait (NULL) ;

/*
-- crf: 16/03/93
-- Synchronize shut down with do_stderr
-- Comments:
-- I have experienced immense difficulty with the rsh daemon on the C40. The
-- problems are related (yet again) to the clean termination of the daemon
-- (either hangs or turns into a zombie - refer comments pertaining to 
-- revision 1.6). I have attempted to synchronize the termination of the 
-- threads, and, although this appears to have been successful, I believe 
-- this is due more to luck than judgement.
-- Notice that it is essential to explicitly close both ends of the pipes. More
-- importantly, the order in which this is done is crucial - I have spent many
-- happy hours experimenting with this.
-- The cause (and "solution") of the zombie problem is still not clear to me,
-- and I suspect that this problem will resurface in time.
*/
	(void) write (synch[1], "\0", 1) ;
	Wait (&sem) ;
	(void) close (pv[1]) ;
	(void) close (synch[1]) ;
	exit (0) ;
#else
	execl(pwd->pw_shell, cp, "-c", cmdbuf, 0);
	perror(pwd->pw_shell);
	exit(1);
#endif
}

/*VARARGS1*/
void
error( char * fmt, ... )
{
  char buf[BUFSIZ];
  va_list	args;

  va_start( args, fmt );
  
  buf[0] = 1;
  (void) vsprintf( buf + 1, fmt, args );
  (void) write(2, buf, strlen(buf));
  va_end( args );
}

#ifdef __HELIOS

void do_stderr(int pv0, int s, int synch0)
{
	fd_set readfrom, ready;	
	int nfd;
	char sig;
	char buf[100];
	int one = 1;
	int cc;

	if (pv0 > s)
		nfd = pv0;
	else
		nfd = s;
	if (synch0 > nfd)
		nfd = synch0 ;
	nfd++;
	
	FD_ZERO(&readfrom);
	FD_SET(s, &readfrom);
	FD_SET(pv0, &readfrom);
	FD_SET(synch0, &readfrom);
	ioctl(pv0, FIONBIO, (char *)&one);
	/* should set s nbio! */
	do {
		ready = readfrom;
		if (select(nfd, (int *)&ready, (int *)0,
		    (int *)0, (struct timeval *)0) < 0)
			break;
		if (FD_ISSET(s, &ready)) {
			if (read(s, &sig, 1) <= 0)
				FD_CLR(s, &readfrom);
			else
			{
				kill(-pid, sig);
			}
		}
		if (FD_ISSET(pv0, &ready)) {
			errno = 0;
			cc = read(pv0, buf, sizeof (buf));
			if (cc <= 0) {
				shutdown(s, 1+1);
				FD_CLR(pv0, &readfrom);
			} else
				(void) write(s, buf, cc);
/*
-- crf: 16/03/93
-- This is UGLY. Want to ignore synch0 if pv0 and synch0 are both ready ...
*/
			continue ; /* XXX */
		}
/*
-- crf: 16/03/93
-- Anything on synchronization channel ? If so, terminate ...
*/
		if (FD_ISSET(synch0, &ready)) {
			char c ;
			(void) read (synch0, &c, 1) ;
			break ;
		}
	} while (FD_ISSET(s, &readfrom) ||
	    FD_ISSET(pv0, &readfrom));

/*
-- crf: 16/03/93
-- Synchronize pipe closure - Note the ORDER in which the pipes are closed
*/
	Signal (&sem) ;
	(void) close(synch0);
	(void) close(pv0);
}

#endif

/*ARGSUSED*/
int
main(
     int 	argc ,
     char **	argv )
{
	extern int opterr, optind, _check_rhosts_file;
	struct linger linger;
	int ch, on = 1, fromlen;
	struct sockaddr_in from;

	ProgName = argv[ 0 ];
	
	openlog("rsh", LOG_PID | LOG_ODELAY, LOG_DAEMON);

	opterr = 0;
	while ((ch = getopt(argc, argv, "ln")) != EOF)
		switch((char)ch) {
		case 'l':
			_check_rhosts_file = 0;
			break;
		case 'n':
			keepalive = 0;
			break;
		case '?':
		default:
			syslog(LOG_ERR, "usage: rshd [-l]");
			break;
		}

	argc -= optind;
	argv += optind;


	fromlen = sizeof (from);
	if (getpeername(0, (struct sockaddr *)&from, &fromlen) < 0) {
		fprintf(stderr, "%s: ", ProgName);
		perror("getpeername");
		_exit(1);
	}
	if (keepalive &&
	    setsockopt(0, SOL_SOCKET, SO_KEEPALIVE, (char *)&on,
	    sizeof(on)) < 0)
		syslog(LOG_WARNING, "setsockopt (SO_KEEPALIVE): %m");
	linger.l_onoff = 1;
	linger.l_linger = 60;			/* XXX */
	if (setsockopt(0, SOL_SOCKET, SO_LINGER, (char *)&linger,
	    sizeof (linger)) < 0)
		syslog(LOG_WARNING, "setsockopt (SO_LINGER): %m");

#ifdef __HELIOS
	InitSemaphore (&sem, 0) ;
#endif
	doit(&from);
}
