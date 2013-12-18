 /*
 * Copyright (c) 1983, 1986 Regents of the University of California.
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
"@(#) Copyright (c) 1983, 1986 Regents of the University of California.\n\
 All rights reserved.\n";
#endif /* not lint */

#if !defined lint && !defined __HELIOS
static char sccsid[] = "@(#)telnetd.c	5.31 (Berkeley) 2/23/89";
#endif /* not lint */

/*
 * Telnet server.
 */
#ifdef __HELIOS
#include <helios.h>
#include <syslib.h>
#include <unistd.h>
#include <fcntl.h>
#include <limits.h>
#include <stdlib.h>
#endif

#include <sys/param.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/time.h>

#include <netinet/in.h>

#include <arpa/telnet.h>
#include <arpa/inet.h>

#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <sgtty.h>
#include <netdb.h>
#include <syslog.h>
#include <ctype.h>

#define	OPT_NO			0		/* won't do this option */
#define	OPT_YES			1		/* will do this option */
#define	OPT_YES_BUT_ALWAYS_LOOK	2
#define	OPT_NO_BUT_ALWAYS_LOOK	3

char	hisopts[256];
char	myopts[256];

char	doopt[] = { IAC, DO, '%', 'c', 0 };
char	dont[] = { IAC, DONT, '%', 'c', 0 };
char	will[] = { IAC, WILL, '%', 'c', 0 };
char	wont[] = { IAC, WONT, '%', 'c', 0 };

/*
 * I/O data buffers, pointers, and counters.
 */
char	ptyibuf[BUFSIZ], *ptyip = ptyibuf;

char	ptyobuf[BUFSIZ], *pfrontp = ptyobuf, *pbackp = ptyobuf;

char	netibuf[BUFSIZ], *netip = netibuf;
#define	NIACCUM(c)	{   *netip++ = c; \
			    ncc++; \
			}

char	netobuf[BUFSIZ], *nfrontp = netobuf, *nbackp = netobuf;
char	*neturg = 0;		/* one past last bye of urgent data */
	/* the remote system seems to NOT be an old 4.2 */
int	not42 = 1;

#if 0
#if __HELIOS
static char *BANNER = "\r\n\r\r\n\r	        Helios Telnet Daemon     Rel.900919 V1.0\r\n\r                (C) Copyright 1990 Parsytec / Perihelion\r\n\r\r\n\r";
#else
#define	BANNER	"\r\n\r\n4.3 BSD UNIX (%s)\r\n\r\r\n\r"
#endif
#endif
  
/* buffer for sub-options */
char	subbuffer[100], *subpointer= subbuffer, *subend= subbuffer;
#define	SB_CLEAR()	subpointer = subbuffer;
#define	SB_TERM()	{ subend = subpointer; SB_CLEAR(); }
#define	SB_ACCUM(c)	if (subpointer < (subbuffer+sizeof subbuffer)) { \
				*subpointer++ = (c); \
			}
#define	SB_GET()	((*subpointer++)&0xff)
#define	SB_EOF()	(subpointer >= subend)

int	pcc = 0, ncc = 0;

int	pty = -1, net = -1;
#if __HELIOS
char	pty_path[_POSIX_PATH_MAX+1] = "";
char	tty_path[_POSIX_PATH_MAX+1] = "";
int	server_id = 0;
int	login_id = 0;
Environ *my_environ;
#endif

int	inter;
extern	char **environ;
extern	int errno;
char	*line = NULL;
int	SYNCHing = 0;		/* we are in TELNET SYNCH mode */

extern int setpgrp(int, int);

/*
 * The following are some clocks used to decide how to interpret
 * the relationship between various variables.
 */

struct {
    int
	system,			/* what the current time is */
	echotoggle,		/* last time user entered echo character */
	modenegotiated,		/* last time operating mode negotiated */
	didnetreceive,		/* last time we read data from network */
	ttypeopt,		/* ttype will/won't received */
	ttypesubopt,		/* ttype subopt is received */
	getterminal,		/* time started to get terminal information */
	gotDM;			/* when did we last see a data mark */
} clocks;

#define	settimer(x)	(clocks.x = ++clocks.system)
#define	sequenceIs(x,y)	(clocks.x < clocks.y)

void	ttloop(void);
void	getterminaltype(void);
void	doit(int, struct sockaddr_in *);
void	fatal(int, char *);
void	fatalperror(int, char *);
int	stilloob(int);
void	telnet(int, int);
void	telrcv(void);
void	willoption(int);
void	wontoption(int);
void	dooption(int);
void	dontoption(int);
void	suboption(void);
void	mode(int, int);
void	interrupt(void);
void	sendbrk(void);
void	ptyflush(void);
char 	*nextitem(char *);
void	netclear(void);
void	netflush(void);
void	cleanup(void);
void	edithost(char *, char *);
void	tputs(char *);
void	putchr(int);
void	putf(char *, char *);
#ifdef __HELIOS
void	get_date(char *);
#endif

int 
main(
     int argc,
     char *argv[] )
{
	struct sockaddr_in from;
	int on = 1, fromlen;

#if __HELIOS
	my_environ = (Environ *)getenviron();

#if 0
{
#define ne !=
#define eq ==
  int	i;
  for (i = 0; my_environ->Argv[i] ne Null(char); i++)
   IOdebug("argument %d : %s", i, my_environ->Argv[i]);
  for (i = 0; my_environ->Envv[i] ne Null(char); i++)
   IOdebug("envstring %d : %s", i, my_environ->Envv[i]);
  for (i = 0; my_environ->Strv[i] ne Null(Stream); i++)
   if (my_environ->Strv[i] ne (Stream *) MinInt)
    IOdebug("stream %d : %s", i, my_environ->Strv[i]->Name);
  for (i = 0; my_environ->Objv[i] ne Null(Object); i++)
   if (my_environ->Objv[i] ne (Object *) MinInt)
    IOdebug("object %d : %s", i, my_environ->Objv[i]->Name);
}
#endif
	memset(hisopts, 0x0, 256);
	memset(myopts,  0x0, 256);
#endif

#if	defined(DEBUG)
	{
	    int e, s, ns, foo;
	    struct servent *sp;
	    static struct sockaddr_in sin = { AF_INET };

	    sp = getservbyname("telnet", "tcp");

	    /* IOdebug( "telnetd: getservbyname %x", sp); */
	    
	    if (sp == 0)
	      {
		IOdebug("telnetd: tcp/telnet: unknown service");
		fprintf(stderr, "telnetd: tcp/telnet: unknown service\n");
		exit(1);
	      }
	    sin.sin_port = sp->s_port;
	    argc--, argv++;
	    if (argc > 0)
	      {
		/* IOdebug( "telnetd: portnumber %s", *argv); */
		    sin.sin_port = atoi(*argv);
		    sin.sin_port = htons((u_short)sin.sin_port);
	    }

	    s = socket(AF_INET, SOCK_STREAM, 0);

	    /* IOdebug( "telnetd: socket %d %d", s, errno); */
	    
	    if (s < 0) {
		    perror("telnetd: socket");;
		    exit(1);
	    }
	    e = bind(s, &sin, sizeof(sin));

	    /* IOdebug( "telnetd: bind %d %d", e, errno); */
	    
	    if (e < 0) {
		perror("bind");
		exit(1);
	    }
	    e = listen(s, 1);

	    /* IOdebug( "telnetd: listen %d %d", e, errno); */
	    
	    if (e < 0) {
		perror("listen");
		exit(1);
	    }
	    foo = sizeof sin;
	    ns = accept(s, &sin, &foo);

	    /* IOdebug( "telnetd: accept %d %d", ns, errno); */
	    
	    if (ns < 0) {
		perror("accept");
		exit(1);
	    }
	    dup2(ns, 0);
	    close(s);
	}
#endif	/* defined(DEBUG) */

	openlog("telnetd", LOG_PID | LOG_ODELAY, LOG_DAEMON);
	fromlen = sizeof (from);
	if (getpeername(0, (struct sockaddr *)&from, &fromlen) < 0) {
		syslog(LOG_ERR, "getpeername: %m");
		_exit(1);
	}
	if (setsockopt(0, SOL_SOCKET, SO_KEEPALIVE, (char *)&on, sizeof (on)) < 0) {
		syslog(LOG_WARNING, "setsockopt (SO_KEEPALIVE): %m");
	}
	doit(0, &from);
}

char	*terminaltype = 0;
char	*envinit[2];

/*
 * ttloop
 *
 *	A small subroutine to flush the network output buffer, get some data
 * from the network, and pass it through the telnet state machine.  We
 * also flush the pty input buffer (by dropping its data) if it becomes
 * too full.
 */

void
ttloop()
{
    if (nfrontp-nbackp) {
	netflush();
    }
#ifdef NET_RW
    /* IOdebug( "telnetd: read(net,%x,%d)", netibuf, sizeof netibuf); */
    ncc = read(net, netibuf, sizeof netibuf);
    /* IOdebug( "telnetd: result %d %d", ncc, ncc < 0 ? errno : 0); */
#else
    ncc = read(net, netibuf, sizeof netibuf);
#endif
    if (ncc < 0)
      {
	syslog(LOG_INFO, "ttloop:  read: %m");
#if __HELIOS
	/* IOdebug( "read failed, ncc = %d, errno = %d, shutting down", ncc, errno ); */
	
	cleanup();
#else
	exit(1);
#endif
    }
    else if (ncc == 0)
      {
	syslog(LOG_INFO, "ttloop:  peer died: %m");
#if __HELIOS
	/* IOdebug( "read timed out, errno = %d, shutting down", errno ); */
	
	cleanup();
#else
	exit(1);
#endif
    }
    netip = netibuf;
    telrcv();			/* state machine */
    if (ncc > 0) {
	pfrontp = pbackp = ptyobuf;
	telrcv();
    }
}

/*
 * getterminaltype
 *
 *	Ask the other end to send along its terminal type.
 * Output is the variable terminaltype filled in.
 */

void
getterminaltype()
{
    static char sbuf[] = { IAC, DO, TELOPT_TTYPE };

    settimer(getterminal);
    bcopy(sbuf, nfrontp, sizeof sbuf);
    nfrontp += sizeof sbuf;
    hisopts[TELOPT_TTYPE] = OPT_YES_BUT_ALWAYS_LOOK;
    while (sequenceIs(ttypeopt, getterminal)) {
	ttloop();
    }
    if (hisopts[TELOPT_TTYPE] == OPT_YES) {
	static char sbbuf[] = { IAC, SB, TELOPT_TTYPE, TELQUAL_SEND, IAC, SE };

	bcopy(sbbuf, nfrontp, sizeof sbbuf);
	nfrontp += sizeof sbbuf;
	while (sequenceIs(ttypesubopt, getterminal)) {
	    ttloop();
	}
    }
}

/*
 * Get a pty, scan input lines.
 */

#if 0
#define IODEBUG
#endif

void
doit(
	int f,
	struct sockaddr_in *who )
{
	char *host;
	int p, t;
	struct hostent *hp;
#if !defined __HELIOS	
	int i;
	struct sgttyb b;
	int c;
#endif

#ifdef __HELIOS
	extern int SendTerminalType ( char *path, char *terminaltype );
	
	Object *OV_Co, *OV_Cs;
	{
	    int fda[2];
	    int e;
	    
	    /* create pipe */
	    /* fda[0] for reading */
	    /* fda[1] for writing */

	    if (pipe(fda)) 
		fatalperror(f, "/pipe");

	    if (chdir("/") < 0)
		fatalperror(f, "/");

	    if ((server_id = vfork()) < 0)
		fatalperror(f, "vfork");
	    
	    if (server_id == 0)
	      {
		/* child */
		close(f);
		close(fda[0]);
		dup2(fda[1], 0); /* input for pty, just a dummy */
		dup2(fda[1], 1); /* output for pty */
		dup2(fda[1], 2); /* error for pty */
		close(fda[1]);

		/* our arguments for tty:		*/
		/*					*/
		/* -p        use pseudo terminal driver	*/
		/*					*/
		execl( "/helios/lib/ttyserv", "ttyserv", "-p", NULL);
		
		syslog(LOG_ERR, "/helios/lib/ttyserv: %m\r\n");
		fatalperror(2, "/helios/lib/ttyserv");
		/*NOTREACHED*/
	    }
	    /* parent */
#ifdef IODEBUG
	    IOdebug("telnetd: tty started, in child now");
#endif
	    close(fda[1]);

	    /* wait for tty name table entry */ 
	    e = read(fda[0], tty_path, _POSIX_PATH_MAX);
	    if (e <= 0)
	      {
	    	kill(server_id, SIGTERM);
		fatalperror( f, "no tty_path");
	    }
	    tty_path[e] = '\0';
	
#ifdef IODEBUG 
	    IOdebug("telnetd: tty name table entry is: '%s'", tty_path);	
#endif

	    OV_Cs = Locate(NULL, tty_path);
	    if (OV_Cs == NULL) {
	    	kill(server_id, SIGTERM);
		fatalperror(f, "cannot locate tty");
	    }
	    
	    my_environ->Objv[OV_CServer] = OV_Cs;

	    /* wait for pty name table entry */
	    e = read(fda[0], pty_path, _POSIX_PATH_MAX);
	    if (e <= 0) {
	    	kill(server_id, SIGTERM);
		fatalperror(f, "no pty_path");
	    }
	    pty_path[e] = '\0';
#ifdef IODEBUG
	    IOdebug( "telnetd: pty name table entry is: '%s'", pty_path);	
#endif

	    /* open pty */
	    strcat(pty_path, "/entry");
	    p = open(pty_path, O_RDWR|O_NONBLOCK);
	    if (p < 0)
	      {
	    	kill(server_id, SIGTERM);
		fatalperror(f, pty_path);
	    }
	    pty = p;

#ifdef IODEBUG 
   	    IOdebug("telnetd: pty is %s", pty_path);
#endif
	}
	
	dup2(f, 0);
	net = f;
		
	/* create tty/console */
	strcat(tty_path, "/console");
	t = open(tty_path, O_RDWR|O_CREAT|O_NONBLOCK);
	if (t < 0) {
		atexit(cleanup);
		fatalperror(f, tty_path);
	}
#ifdef IODEBUG
	IOdebug("telnetd: tty is %s", tty_path);
#endif

	OV_Co = Locate(OV_Cs, "console");
	if (OV_Co == NULL)
		fatalperror(f, "cannot locate console");
	    
	my_environ->Objv[OV_Console] = OV_Co;

#else	/* ifdef __HELIOS */

	for (c = 'p'; c <= 's'; c++) {
		struct stat stb;

		line = "/dev/ptyXX";
		line[strlen("/dev/pty")] = c;
		line[strlen("/dev/ptyp")] = '0';
		if (stat(line, &stb) < 0)
			break;
		for (i = 0; i < 16; i++) {
			line[sizeof("/dev/ptyp") - 1] = "0123456789abcdef"[i];
			p = open(line, O_RDWR);
			if (p > 0)
				goto gotpty;
		}
	}
	fatal(f, "All network ports in use");
	/*NOTREACHED*/
gotpty:
	dup2(f, 0);
	line[strlen("/dev/")] = 't';
	t = open("/dev/tty", O_RDWR);
	if (t >= 0) {
		ioctl(t, TIOCNOTTY, 0);
		close(t);
	}
	t = open(line, O_RDWR);
	if (t < 0)
		fatalperror(f, line);
	if (fchmod(t, 0))
		fatalperror(f, line);
	(void)signal(SIGHUP, SIG_IGN);
	vhangup();
	(void)signal(SIGHUP, SIG_DFL);
	t = open(line, O_RDWR);
	if (t < 0)
		fatalperror(f, line);
	ioctl(t, TIOCGETP, &b);
	b.sg_flags = CRMOD|XTABS|ANYP;
	ioctl(t, TIOCSETP, &b);
	ioctl(p, TIOCGETP, &b);
	b.sg_flags &= ~ECHO;
	ioctl(p, TIOCSETP, &b);

#endif	/* ifdef __HELIOS */

	hp = gethostbyaddr(&who->sin_addr, sizeof (struct in_addr),
		who->sin_family);
	if (hp)
		host = hp->h_name;
	else
		host = inet_ntoa(who->sin_addr);

	net = f;
	pty = p;

	/*
	 * get terminal type.
	 */
	getterminaltype();
#ifdef __HELIOS

	if (terminaltype && SendTerminalType(tty_path, terminaltype) < 0)
	{
	    char buf[BUFSIZ];

	    (void)sprintf(buf, "telnetd: failed to set %s: %s\r\n",
	    	terminaltype, sys_errlist[errno]);
	    (void) write(f, buf, strlen(buf));
	    terminaltype = NULL;
	    Delay(OneSec*4);
	}

#ifdef IODEBUG
	IOdebug("telnetd: %s", terminaltype);
#endif

	mode(CRMOD|XTABS|ANYP, ECHO);

	if (!myopts[TELOPT_ECHO]) {
	    dooption(TELOPT_ECHO);
	}
	if (!myopts[TELOPT_SGA]) {
	    dooption(TELOPT_SGA);
	}
	ttloop();

	if ((login_id = vfork()) < 0) {
		atexit(cleanup);
		fatalperror(f, "vfork");
	}
	if (login_id) 
#else
	if ((i = fork()) < 0)
		fatalperror(f, "fork");
	if (i)
#endif
		telnet(f, p);

	close(f);
	close(p);
	dup2(t, 0);
	dup2(t, 1);
	dup2(t, 2);
	close(t);

	envinit[0] = terminaltype;
	envinit[1] = 0;
	environ = envinit;
	/*
	 * -h : pass on name of host.
	 *		WARNING:  -h is accepted by login if and only if
	 *			getuid() == 0.
	 * -p : don't clobber the environment (so terminal type stays set).
	 */
#if __HELIOS
	execl("/helios/bin/login", "-", 0);
	syslog(LOG_ERR, "/helios/bin/login: %m\r\n");
	fatalperror(2, "/helios/bin/login");
	/*NOTREACHED*/
#else
	execl("/bin/login", "login", "-h", host,
					terminaltype ? "-p" : 0, 0);
	syslog(LOG_ERR, "/bin/login: %m\n");
	fatalperror(2, "/bin/login");
	/*NOTREACHED*/
#endif
}

void
fatal(
	int f,
	char *msg )
{
	char buf[BUFSIZ];

	(void) sprintf(buf, "telnetd: %s.\r\n", msg);
	(void) write(f, buf, strlen(buf));
	exit(1);
}

void fatalperror(
	int f,
	char *msg )
{
	char buf[BUFSIZ];

	(void) sprintf(buf, "%s: %s\r\n", msg, sys_errlist[errno]);
	fatal(f, buf);
}


/*
 * Check a descriptor to see if out of band data exists on it.
 */
int stilloob(
	int s )		/* socket number */
{
    static struct timeval timeout = { 0 };
    fd_set	excepts;
    int value;

#ifdef __HELIOS
    timeout.tv_sec = 0;
    timeout.tv_usec = 0;
#endif
    do {
	FD_ZERO(&excepts);
	FD_SET(s, &excepts);
	value = select(s+1, (int *)0, (int *)0, (int *)&excepts, &timeout);

#ifdef SELECTDEBUG
	IOdebug( "telnetd: %x = select(%d, e(net), timeout = 0)", value, s+1);
#endif

    } while ((value == -1) && (errno == EINTR));

    if (value < 0) {
	fatalperror(pty, "select");
    }
    if (FD_ISSET(s, &excepts)) {
	return 1;
    } else {
	return 0;
    }
}


/*
 * Main loop.  Select from pty and network, and
 * hand data to telnet receiver finite state machine.
 */
void telnet(int f, int p)
{
	int on = 1;
	char hostname[MAXHOSTNAMELEN];
#ifndef __HELIOS
#define	TABBUFSIZ	512
	char	defent[TABBUFSIZ];
	char	defstrs[TABBUFSIZ];	
#undef	TABBUFSIZ
	char *HE;
	char *HN;
	char *IM;
#endif
	
#ifdef __HELIOS
	struct timeval timeout;

	timeout.tv_sec = 3;
	timeout.tv_usec = 0;
#endif

#ifdef IODEBUG
	IOdebug( "telnetd: ioctl(f, FIONBIO, 1) = %d)", ioctl(f, FIONBIO, &on) );
	IOdebug( "telnetd: ioctl(p, FIONBIO, 1) = %d)", ioctl(p, FIONBIO, &on) );
	IOdebug( "telnetd: ioctl(p, TIOCPKT, 1) = %d)", ioctl(p, TIOCPKT, &on) );
#else
	ioctl(f, FIONBIO, (char *)&on);
	ioctl(p, FIONBIO, (char *)&on);
	ioctl(p, TIOCPKT, (char *)&on);
#endif

#if	defined(SO_OOBINLINE)
	setsockopt(net, SOL_SOCKET, SO_OOBINLINE, (char *)&on, sizeof on);
#endif	/* defined(SO_OOBINLINE) */
	signal(SIGTSTP, SIG_IGN);
	/*
	 * Ignoring SIGTTOU keeps the kernel from blocking us
	 * in ttioctl() in /sys/tty.c.
	 */
	signal(SIGTTOU, SIG_IGN);
#if 0 /* XXX */
	signal(SIGCHLD, (void(*)())cleanup);
#endif
	setpgrp(0, 0);

	/*
	 * Request to do remote echo and to suppress go ahead.
	 */
#ifndef __HELIOS
	if (!myopts[TELOPT_ECHO]) {
	    dooption(TELOPT_ECHO);
	}
	if (!myopts[TELOPT_SGA]) {
	    dooption(TELOPT_SGA);
	}
#endif
	/*
	 * Is the client side a 4.2 (NOT 4.3) system?  We need to know this
	 * because 4.2 clients are unable to deal with TCP urgent data.
	 *
	 * To find out, we send out a "DO ECHO".  If the remote system
	 * answers "WILL ECHO" it is probably a 4.2 client, and we note
	 * that fact ("WILL ECHO" ==> that the client will echo what
	 * WE, the server, sends it; it does NOT mean that the client will
	 * echo the terminal input).
	 */
	(void) sprintf(nfrontp, doopt, TELOPT_ECHO);
	nfrontp += sizeof doopt-2;
	hisopts[TELOPT_ECHO] = OPT_YES_BUT_ALWAYS_LOOK;


	/*
	 * Show banner that getty never gave.
	 *
	 * We put the banner in the pty input buffer.  This way, it
	 * gets carriage return null processing, etc., just like all
	 * other pty --> client data.
	 */
	gethostname(hostname, sizeof (hostname));

#ifndef __HELIOS
	if (getent(defent, "default") == 1)
	  {
		char *p=defstrs;

		HE = getstr("he", &p);
		HN = getstr("hn", &p);
		IM = getstr("im", &p);
		if (HN && *HN)
			strcpy(hostname, HN);
		edithost(HE, hostname);
		if (IM && *IM)
			putf(IM, ptyibuf+1);
	} else 
		sprintf(ptyibuf+1, BANNER, hostname);
	
	ptyip = ptyibuf+1;		/* Prime the pump */
	pcc = strlen(ptyip);		/* ditto */

#else
#if 0
	write(net, BANNER, strlen(BANNER));
	Delay(OneSec*4);
#endif
#endif

	/* Clear ptybuf[0] - where the packet information is received */
	ptyibuf[0] = 0;

	/*
	 * Call telrcv() once to pick up anything received during
	 * terminal type negotiation.
	 */
	telrcv();

	for (;;) {
		fd_set ibits, obits, xbits;
		register int c;

#if __HELIOS
		if (ncc < 0 || pcc < 0) {
#ifdef IODEBUG
			IOdebug("telnetd: ncc < 0 || pcc < 0");
#endif
			break;
		}
#else
		if (ncc < 0 && pcc < 0)
			break;
#endif

		FD_ZERO(&ibits);
		FD_ZERO(&obits);
		FD_ZERO(&xbits);
		/*
		 * Never look for input if there's still
		 * stuff in the corresponding output buffer
		 */
		if (nfrontp - nbackp || pcc > 0) {
			FD_SET(f, &obits);
			FD_SET(p, &xbits);
		} else {
			FD_SET(p, &ibits);
		}
		if (pfrontp - pbackp || ncc > 0) {
			FD_SET(p, &obits);
		} else {
			FD_SET(f, &ibits);
		}
		if (!SYNCHing) {
			FD_SET(f, &xbits);
		}

#ifdef SELECTDEBUG
		IOdebug("telnetd: select(16, i(%s %s), o(%s %s), e(%s %s), 0)",
			FD_ISSET(f, &ibits) ? "net" : "",
			FD_ISSET(p, &ibits) ? "pty" : "",
			FD_ISSET(f, &obits) ? "net" : "",
			FD_ISSET(p, &obits) ? "pty" : "",
			FD_ISSET(f, &xbits) ? "net" : "",
			FD_ISSET(p, &xbits) ? "pty" : "");
#endif

#ifdef TRACE
_Trace(0,0,1);
_Trace(((int*)&ibits)[0], ((int*)&obits)[0], ((int*)&xbits)[0]);
#endif

#ifdef __HELIOS
		c = select(16 , (int *)&ibits, (int *)&obits, (int *)&xbits,	&timeout );
#ifdef TRACE
_Trace(c,errno,2);
#endif
		if (c < 1)
		  {
			if (c == -1)
			  {
				if (errno == EINTR || errno == EAGAIN)
				  {
#if defined(SELECTDEBUG) || defined(IODEBUG)
					IOdebug( "telnetd: select: EINTR | EAGAIN");
#endif
					continue;
				}
			}
#if defined(SELECTDEBUG) || defined(IODEBUG)
			IOdebug("telnetd: select: 0x%x, errno 0x%x (%x %x %x)",
				c, errno, ibits, obits, xbits);
#endif
#ifdef TRACE
_Trace(0,0,3);
#endif

#ifdef OLDCODE
/*
-- crf : 02/10/91 - Bug 730 (Major response delay ...)
-- The problem is related to the timeout associated with the select. When the 
-- select times out (after 3 seconds), this Delay is encountered ... if you
-- catch it at the wrong time, you'll end up with a 5+ second wait for a
-- response. There are two ways to sort this out :
--   1. give the select an infinite timeout
--   2. remove the Delay().
-- I have chosen to do the latter to remain consistent with the original 
-- decision to force the select to time out.
*/
			Delay(OneSec*5);
#endif

			continue;
		}
		
#else /* ifdef __HELIOS */

		c = select(16 , &ibits, &obits, &xbits,	(struct timeval *)0 );
		if (c < 1) {
			if (c == -1) {
				if (errno == EINTR)
					continue;
			}
			sleep(5);
			continue;
		}
#endif /* ifdef __HELIOS */

#ifdef SELECTDEBUG
		IOdebug( "telnetd: %x = select(16, i(%s %s), o(%s %s), e(%s %s), 0)", c,
			FD_ISSET(f, &ibits) ? "net" : "",
			FD_ISSET(p, &ibits) ? "pty" : "",
			FD_ISSET(f, &obits) ? "net" : "",
			FD_ISSET(p, &obits) ? "pty" : "",
			FD_ISSET(f, &xbits) ? "net" : "",
			FD_ISSET(p, &xbits) ? "pty" : "");
#endif

		/*
		 * Any urgent data?
		 */
		if (FD_ISSET(net, &xbits))
		    SYNCHing = 1;

		/*
		 * Something to read from the network...
		 */
		if (FD_ISSET(net, &ibits)) {
#if	!defined(SO_OOBINLINE)
			/*
			 * In 4.2 (and 4.3 beta) systems, the
			 * OOB indication and data handling in the kernel
			 * is such that if two separate TCP Urgent requests
			 * come in, one byte of TCP data will be overlaid.
			 * This is fatal for Telnet, but we try to live
			 * with it.
			 *
			 * In addition, in 4.2 (and...), a special protocol
			 * is needed to pick up the TCP Urgent data in
			 * the correct sequence.
			 *
			 * What we do is:  if we think we are in urgent
			 * mode, we look to see if we are "at the mark".
			 * If we are, we do an OOB receive.  If we run
			 * this twice, we will do the OOB receive twice,
			 * but the second will fail, since the second
			 * time we were "at the mark", but there wasn't
			 * any data there (the kernel doesn't reset
			 * "at the mark" until we do a normal read).
			 * Once we've read the OOB data, we go ahead
			 * and do normal reads.
			 *
			 * There is also another problem, which is that
			 * since the OOB byte we read doesn't put us
			 * out of OOB state, and since that byte is most
			 * likely the TELNET DM (data mark), we would
			 * stay in the TELNET SYNCH (SYNCHing) state.
			 * So, clocks to the rescue.  If we've "just"
			 * received a DM, then we test for the
			 * presence of OOB data when the receive OOB
			 * fails (and AFTER we did the normal mode read
			 * to clear "at the mark").
			 */
		    if (SYNCHing) {
			int atmark = 0;

#ifdef IODEBUG
			if (ioctl(net, SIOCATMARK, (char *)&atmark) < 0)
 			    IOdebug( "telnetd: ioctl(net, SIOCATMARK, &) errno %d)", errno);
#else
			ioctl(net, SIOCATMARK, (char *)&atmark);
#endif

			if (atmark) {
#ifdef NET_RW
			    /* IOdebug("recv(net,%x,%d,MSG_OOB)", netibuf, sizeof (netibuf));*/
			    ncc = recv(net, netibuf, sizeof (netibuf), MSG_OOB);
			    /* IOdebug("result %d %d", ncc, ncc < 0 ? errno : 0); */
#else
			    ncc = recv(net, netibuf, sizeof (netibuf), MSG_OOB);
#endif
			    if ((ncc == -1) && (errno == EINVAL)) {
#ifdef NET_RW
				/* IOdebug("read(net,%x,%d)", netibuf, sizeof (netibuf)); */
				ncc = read(net, netibuf, sizeof (netibuf));
				/* IOdebug("result %d %d", ncc, ncc < 0 ? errno : 0); */
#else
				ncc = read(net, netibuf, sizeof (netibuf));
#endif
				if (sequenceIs(didnetreceive, gotDM)) {
				    SYNCHing = stilloob(net);
				}
			    }
			} else {
#ifdef NET_RW
			    /* IOdebug("read(net,%x,%d)", netibuf, sizeof (netibuf)); */
			    ncc = read(net, netibuf, sizeof (netibuf));
			    /* IOdebug("result %d %d", ncc, ncc < 0 ? errno : 0); */
#else
			    ncc = read(net, netibuf, sizeof (netibuf));
#endif
			}
		    } else {
#ifdef NET_RW
			/* IOdebug("read(net,%x,%d)", netibuf, sizeof (netibuf)); */
			ncc = read(net, netibuf, sizeof (netibuf));
			/* IOdebug("result %d %d", ncc, ncc < 0 ? errno : 0); */
#else
			ncc = read(net, netibuf, sizeof (netibuf));
#endif
		    }
		    settimer(didnetreceive);
#else	/* !defined(SO_OOBINLINE)) */
#ifdef NET_RW
		    /* IOdebug("read(net,%x,%d)", netibuf, sizeof (netibuf)); */
		    ncc = read(net, netibuf, sizeof (netibuf));
		    /* IOdebug("result %d %d", ncc, ncc < 0 ? errno : 0);*/
#else
		    ncc = read(net, netibuf, sizeof (netibuf));
#endif
#endif	/* !defined(SO_OOBINLINE)) */
		    if (ncc < 0 && errno == EWOULDBLOCK)
			ncc = 0;
		    else {
			if (ncc <= 0) {
#ifdef IODEBUG
			    IOdebug("telnetd: ncc <= 0 errno %d", errno);
#endif
			    break;
			}
			netip = netibuf;
		    }
		}

		/*
		 * Something to read from the pty...
		 */
		if (FD_ISSET(p, &xbits)) {
#ifdef PTY_RW
		    /* IOdebug( "telnetd: read(pty,%x,%d)", ptyibuf, 1); */
		    if (read(p, ptyibuf, 1) != 1) {
#else
		    if (read(p, ptyibuf, 1) != 1) {
#endif
#if defined(IODEBUG) || defined(PTY_RW)
			IOdebug("telnetd: read(pty,1) != 1 errno %d", errno);
#endif
			break;
		    }
		}
		if (FD_ISSET(p, &ibits)) {
#ifdef PTY_RW
		    /* IOdebug("read(pty,%x,%d)", ptyibuf, BUFSIZ); */
			pcc = read(p, ptyibuf, BUFSIZ);
			/* IOdebug("result %d %d", pcc, pcc < 0 ? errno : 0);*/
#else
			pcc = read(p, ptyibuf, BUFSIZ);
#endif
			if (pcc < 0 && errno == EWOULDBLOCK)
				pcc = 0;
			else {
			    if (pcc <= 0) {
#ifdef IODEBUG
				IOdebug("telnetd: pcc <= 0 errno %d", errno);
#endif
				break;
			    }
			    /* Skip past "packet" */
			    pcc--;
			    ptyip = ptyibuf+1;
			}
		}
		if (ptyibuf[0] & TIOCPKT_FLUSHWRITE) {
#ifdef IODEBUG
			IOdebug("TIOCPK_FLUSHWRITE from pty");
#endif
			netclear();	/* clear buffer back */
			*nfrontp++ = IAC;
			*nfrontp++ = DM;
			neturg = nfrontp-1;  /* off by one XXX */
			ptyibuf[0] = 0;
		}

		while (pcc > 0) {
			if ((&netobuf[BUFSIZ] - nfrontp) < 2)
				break;
			c = *ptyip++ & 0377, pcc--;
			if (c == IAC)
				*nfrontp++ = c;
			*nfrontp++ = c;
			/* Don't do CR-NUL if we are in binary mode */
			if ((c == '\r') && (myopts[TELOPT_BINARY] == OPT_NO)) {
				if (pcc > 0 && ((*ptyip & 0377) == '\n')) {
					*nfrontp++ = *ptyip++ & 0377;
					pcc--;
				} else
					*nfrontp++ = '\0';
			}
#if 0
			if ((c == '\n') && (myopts[TELOPT_BINARY] == OPT_NO)) {
				if (pcc > 0 && ((*ptyip & 0377) == '\r')) {
					*nfrontp++ = *ptyip++ & 0377;
					pcc--;
				} else
					*nfrontp++ = '\r';
#ifdef IODEBUG
				IOdebug("telnetd: '\\n' maped to '\\n\\r'");
#endif
			}
#endif
		}
		if (FD_ISSET(f, &obits) && (nfrontp - nbackp) > 0)
			netflush();
		if (ncc > 0)
			telrcv();
		if (FD_ISSET(p, &obits) && (pfrontp - pbackp) > 0)
			ptyflush();
	}
		
	cleanup();
}
	
/*
 * State for recv fsm
 */
#define	TS_DATA		0	/* base state */
#define	TS_IAC		1	/* look for double IAC's */
#define	TS_CR		2	/* CR-LF ->'s CR */
#define	TS_SB		3	/* throw away begin's... */
#define	TS_SE		4	/* ...end's (suboption negotiation) */
#define	TS_WILL		5	/* will option negotiation */
#define	TS_WONT		6	/* wont " */
#define	TS_DO		7	/* do " */
#define	TS_DONT		8	/* dont " */

void telrcv()
{
	register int c;
	static int state = TS_DATA;

	while (ncc > 0) {
		if ((&ptyobuf[BUFSIZ] - pfrontp) < 2)
			return;
		c = *netip++ & 0377, ncc--;
		switch (state) {

		case TS_CR:
			state = TS_DATA;
			/* Strip off \n or \0 after a \r */
			if ((c == 0) || (c == '\n')) {
				break;
			}
			/* FALL THROUGH */

		case TS_DATA:
			if (c == IAC) {
				state = TS_IAC;
				break;
			}
			if (inter > 0)
				break;
			/*
			 * We now map \r\n ==> \r for pragmatic reasons.
			 * Many client implementations send \r\n when
			 * the user hits the CarriageReturn key.
			 *
			 * We USED to map \r\n ==> \n, since \r\n says
			 * that we want to be in column 1 of the next
			 * printable line, and \n is the standard
			 * unix way of saying that (\r is only good
			 * if CRMOD is set, which it normally is).
			 */
			if ((c == '\r') && (hisopts[TELOPT_BINARY] == OPT_NO)) {
				state = TS_CR;
			}
			*pfrontp++ = c;
			break;

		case TS_IAC:
			switch (c) {

			/*
			 * Send the process on the pty side an
			 * interrupt.  Do this with a NULL or
			 * interrupt char; depending on the tty mode.
			 */
			case IP:
				interrupt();
				break;

			case BREAK:
				sendbrk();
				break;

			/*
			 * Are You There?
			 */
			case AYT:
				strcpy(nfrontp, "\r\n[Yes]\r\n");
				nfrontp += 9;
				break;

			/*
			 * Abort Output
			 */
			case AO: {
					struct ltchars tmpltc;

					ptyflush();	/* half-hearted */

#ifdef IODEBUG
					if (ioctl(pty, TIOCGLTC, (char *)&tmpltc) < 0)
 					    IOdebug("telnetd: ioctl(pty, TIOCGLTC, &) errno %d", errno);
#else
					ioctl(pty, TIOCGLTC, (char *)&tmpltc);
#endif

					if (tmpltc.t_flushc != '\377') {
						*pfrontp++ = tmpltc.t_flushc;
					}
#ifdef IODEBUG
					IOdebug("send abort output: char is %x", pfrontp[-1]);
#endif
					netclear();	/* clear buffer back */
					*nfrontp++ = IAC;
					*nfrontp++ = DM;
					neturg = nfrontp-1; /* off by one XXX */
					break;
				}

			/*
			 * Erase Character and
			 * Erase Line
			 */
			case EC:
			case EL: {
					struct sgttyb b;
					char ch;

					ptyflush();	/* half-hearted */
#ifdef IODEBUG
					if (ioctl(pty, TIOCGETP, (char *)&b) < 0)
 					    IOdebug("ioctl(pty, TIOCGETP, &b) errno %d", errno);
#else
					ioctl(pty, TIOCGETP, (char *)&b);
#endif

					ch = (c == EC) ?
						b.sg_erase : b.sg_kill;
					if (ch != '\377') {
						*pfrontp++ = ch;
					}
#ifdef IODEBUG
					if (ch == EC)
					    IOdebug("send erase char: char is %x", pfrontp[-1]);
					else
					    IOdebug("send erase line: char is %x", pfrontp[-1]);
#endif
					break;
				}

			/*
			 * Check for urgent data...
			 */
			case DM:
				SYNCHing = stilloob(net);
				settimer(gotDM);
				break;


			/*
			 * Begin option subnegotiation...
			 */
			case SB:
				state = TS_SB;
				continue;

			case WILL:
				state = TS_WILL;
				continue;

			case WONT:
				state = TS_WONT;
				continue;

			case DO:
				state = TS_DO;
				continue;

			case DONT:
				state = TS_DONT;
				continue;

			case IAC:
				*pfrontp++ = c;
				break;
			}
			state = TS_DATA;
			break;

		case TS_SB:
			if (c == IAC) {
				state = TS_SE;
			} else {
				SB_ACCUM(c);
			}
			break;

		case TS_SE:
			if (c != SE) {
				if (c != IAC) {
					SB_ACCUM(IAC);
				}
				SB_ACCUM(c);
				state = TS_SB;
			} else {
				SB_TERM();
				suboption();	/* handle sub-option */
				state = TS_DATA;
			}
			break;

		case TS_WILL:
			if (hisopts[c] != OPT_YES)
				willoption(c);
			state = TS_DATA;
			continue;

		case TS_WONT:
			if (hisopts[c] != OPT_NO)
				wontoption(c);
			state = TS_DATA;
			continue;

		case TS_DO:
			if (myopts[c] != OPT_YES)
				dooption(c);
			state = TS_DATA;
			continue;

		case TS_DONT:
			if (myopts[c] != OPT_NO) {
				dontoption(c);
			}
			state = TS_DATA;
			continue;

		default:
			syslog(LOG_ERR, "telnetd: panic state=%d\r\n", state);
			printf("telnetd: panic state=%d\n", state);
#if __HELIOS
			cleanup();
#else
			exit(1);
#endif
		}
	}
}

void willoption(
	int option )
{
	char *fmt;

	switch (option) {

	case TELOPT_BINARY:
		mode(RAW, 0);
		fmt = doopt;
		break;

	case TELOPT_ECHO:
		not42 = 0;		/* looks like a 4.2 system */
		/*
		 * Now, in a 4.2 system, to break them out of ECHOing
		 * (to the terminal) mode, we need to send a "WILL ECHO".
		 * Kludge upon kludge!
		 */
		if (myopts[TELOPT_ECHO] == OPT_YES) {
		    dooption(TELOPT_ECHO);
		}
		fmt = dont;
		break;

	case TELOPT_TTYPE:
		settimer(ttypeopt);
		if (hisopts[TELOPT_TTYPE] == OPT_YES_BUT_ALWAYS_LOOK) {
		    hisopts[TELOPT_TTYPE] = OPT_YES;
		    return;
		}
		fmt = doopt;
		break;

	case TELOPT_SGA:
		fmt = doopt;
		break;

	case TELOPT_TM:
		fmt = dont;
		break;

	default:
		fmt = dont;
		break;
	}
	if (fmt == doopt) {
		hisopts[option] = OPT_YES;
	} else {
		hisopts[option] = OPT_NO;
	}
	(void) sprintf(nfrontp, fmt, option);
	nfrontp += sizeof (dont) - 2;
}

void wontoption(
	int option )
{
	char *fmt;

	switch (option) {
	case TELOPT_ECHO:
		not42 = 1;		/* doesn't seem to be a 4.2 system */
		break;

	case TELOPT_BINARY:
		mode(0, RAW);
		break;

	case TELOPT_TTYPE:
	    settimer(ttypeopt);
	    break;
	}

	fmt = dont;
	hisopts[option] = OPT_NO;
	(void) sprintf(nfrontp, fmt, option);
	nfrontp += sizeof (doopt) - 2;
}

void dooption(
	int option )
{
	char *fmt;

	switch (option) {

	case TELOPT_TM:
		fmt = wont;
		break;

	case TELOPT_ECHO:
		mode(ECHO|CRMOD, 0);
		fmt = will;
  		break;

	case TELOPT_BINARY:
		mode(RAW, 0);
		fmt = will;
		break;

	case TELOPT_SGA:
		fmt = will;
		break;

	default:
		fmt = wont;
		break;
	}
	if (fmt == will) {
	    myopts[option] = OPT_YES;
	} else {
	    myopts[option] = OPT_NO;
	}
	(void) sprintf(nfrontp, fmt, option);
	nfrontp += sizeof (doopt) - 2;
}


void dontoption(
int option)
{
    char *fmt;

    switch (option) {
    case TELOPT_ECHO:		/* we should stop echoing */
	mode(0, ECHO);
	fmt = wont;
	break;

    default:
	fmt = wont;
	break;
    }

    if ((fmt = wont) != NULL) {
	myopts[option] = OPT_NO;
    } else {
	myopts[option] = OPT_YES;
    }
    (void) sprintf(nfrontp, fmt, option);
    nfrontp += sizeof (wont) - 2;
}

/*
 * suboption()
 *
 *	Look at the sub-option buffer, and try to be helpful to the other
 * side.
 *
 *	Currently we recognize:
 *
 *	Terminal type is
 */

void suboption()
{
    switch (SB_GET()) {
    case TELOPT_TTYPE: {		/* Yaaaay! */
	static char terminalname[5+41] = "TERM=";

	settimer(ttypesubopt);

	if (SB_GET() != TELQUAL_IS) {
	    return;		/* ??? XXX but, this is the most robust */
	}

	terminaltype = terminalname+strlen(terminalname);

	while ((terminaltype < (terminalname + sizeof terminalname-1)) &&
								    !SB_EOF()) {
	    register int c;

	    c = SB_GET();
	    if (isupper(c)) {
		c = tolower(c);
	    }
	    *terminaltype++ = c;    /* accumulate name */
	}
	*terminaltype = 0;
	terminaltype = terminalname;
	break;
    }

    default:
	;
    }
}

void mode(
	int on, int off)
{
	struct sgttyb b;

#ifdef IODEBUG
	IOdebug("mode(on %s%s%s%s%s %x, off %s%s%s%s%s %x)", 
	on & CRMOD ? " CRMOD" : "",
	on & XTABS ? " XTABS" : "",
	on & ANYP ? " ANYP" : "",
	on & ECHO ? " ECHO" : "",
	on & RAW ? " RAW" : "",
	on,
	off & CRMOD ? " CRMOD" : "",
	off & XTABS ? " XTABS" : "",
	off & ANYP ? " ANYP" : "",
	off & ECHO ? " ECHO" : "",
	off & RAW ? " RAW" : "", 
	off);
#endif

	ptyflush();

#ifdef IODEBUG
	if (ioctl(pty, TIOCGETP, (char *)&b) < 0)
	    IOdebug("ioctl(pty, TIOCGETP, &b) errno %d", errno);
#else
	ioctl(pty, TIOCGETP, (char *)&b);
#endif

#ifdef IODEBUG
	IOdebug("mode: got flags %s%s%s%s%s %x", 
	b.sg_flags & CRMOD ? " CRMOD" : "",
	b.sg_flags & XTABS ? " XTABS" : "",
	b.sg_flags & ANYP ? " ANYP" : "",
	b.sg_flags & ECHO ? " ECHO" : "",
	b.sg_flags & RAW ? " RAW" : "",
	b.sg_flags);
#endif

	b.sg_flags |= on;
	b.sg_flags &= ~(long)off;

#ifdef IODEBUG
	if (ioctl(pty, TIOCSETP, (char *)&b) < 0)
	    IOdebug("ioctl(pty, TIOCSETP, &b) errno %d", errno);
#else
	ioctl(pty, TIOCSETP, (char *)&b);
#endif
}

/*
 * Send interrupt to process on other side of pty.
 * If it is in raw mode, just write NULL;
 * otherwise, write intr char.
 */
void interrupt()
{
	struct sgttyb b;
	struct tchars tchars;

	ptyflush();	/* half-hearted */

#ifdef IODEBUG
	if (ioctl(pty, TIOCGETP, (char *)&b) < 0)
	    IOdebug("ioctl(pty, TIOCGETP, &) errno %d", errno);
#else
	ioctl(pty, TIOCGETP, (char *)&b);
#endif

	if (b.sg_flags & RAW) {
#ifdef IODEBUG
		IOdebug("send intr: tty is in RAW mode");
#endif
		*pfrontp++ = '\0';
		return;
	}
#ifdef IODEBUG
	if (ioctl(pty, TIOCGETC, (char *)&tchars) < 0) {
	    IOdebug("ioctl(pty, TIOCGETC, &) errno %d)", errno); 
	    *pfrontp++ = '\177';
	}
	else
	    *pfrontp++ = tchars.t_intrc;
#else
	*pfrontp++ = ioctl(pty, TIOCGETC, (char *)&tchars) < 0 ?
		'\177' : tchars.t_intrc;
#endif

#ifdef IODEBUG
	IOdebug("send intr: char is %x", pfrontp[-1]);
#endif
}

/*
 * Send quit to process on other side of pty.
 * If it is in raw mode, just write NULL;
 * otherwise, write quit char.
 */
void sendbrk()
{
	struct sgttyb b;
	struct tchars tchars;

	ptyflush();	/* half-hearted */

#ifdef IODEBUG
	if (ioctl(pty, TIOCGETP, (char *)&b) < 0)
	    IOdebug("ioctl(pty, TIOCGETP, &) errno %d", errno);
#else
	ioctl(pty, TIOCGETP, (char *)&b);
#endif

	if (b.sg_flags & RAW) {
		*pfrontp++ = '\0';
		return;
	}
#ifdef IODEBUG
	if (ioctl(pty, TIOCGETC, (char *)&tchars) < 0) {
	    IOdebug("ioctl(pty, TIOCGETC, &) errno %d)", errno); 
	    *pfrontp++ = '\034';
	}
	else
	    *pfrontp++ = tchars.t_quitc;
#else
	*pfrontp++ = ioctl(pty, TIOCGETC, (char *)&tchars) < 0 ?
		'\034' : tchars.t_quitc;
#endif

#ifdef IODEBUG
	IOdebug("send quit: char is %x", pfrontp[-1]);
#endif
}

void ptyflush()
{
	int n;

	if ((n = pfrontp - pbackp) > 0) {
#ifdef PTY_RW
	    /* IOdebug("write(pty,%x,%d)", pbackp, n); */
		n = write(pty, pbackp, n);
		/* IOdebug("result %d %d", n, n < 0 ? errno : 0); */
#else
		n = write(pty, pbackp, n);
#endif
	}
	if (n < 0)
		return;
	pbackp += n;
	if (pbackp == pfrontp)
		pbackp = pfrontp = ptyobuf;
}


/*
 * nextitem()
 *
 *	Return the address of the next "item" in the TELNET data
 * stream.  This will be the address of the next character if
 * the current address is a user data character, or it will
 * be the address of the character following the TELNET command
 * if the current address is a TELNET IAC ("I Am a Command")
 * character.
 */

char *
nextitem(
char	*current )
{
    if ((*current&0xff) != IAC) {
	return current+1;
    }
    switch (*(current+1)&0xff) {
    case DO:
    case DONT:
    case WILL:
    case WONT:
	return current+3;
    case SB:		/* loop forever looking for the SE */
	{
	    register char *look = current+2;

	    for (;;) {
		if ((*look++&0xff) == IAC) {
		    if ((*look++&0xff) == SE) {
			return look;
		    }
		}
	    }
	}
    default:
	return current+2;
    }
}


/*
 * netclear()
 *
 *	We are about to do a TELNET SYNCH operation.  Clear
 * the path to the network.
 *
 *	Things are a bit tricky since we may have sent the first
 * byte or so of a previous TELNET command into the network.
 * So, we have to scan the network buffer from the beginning
 * until we are up to where we want to be.
 *
 *	A side effect of what we do, just to keep things
 * simple, is to clear the urgent data pointer.  The principal
 * caller should be setting the urgent data pointer AFTER calling
 * us in any case.
 */

void netclear()
{
    register char *thisitem, *next;
    char *good;
#define	wewant(p)	((nfrontp > p) && ((*p&0xff) == IAC) && \
				((*(p+1)&0xff) != EC) && ((*(p+1)&0xff) != EL))

    thisitem = netobuf;

    while ((next = nextitem(thisitem)) <= nbackp) {
	thisitem = next;
    }

    /* Now, thisitem is first before/at boundary. */

    good = netobuf;	/* where the good bytes go */

    while (nfrontp > thisitem) {
	if (wewant(thisitem)) {
	    int length;

	    next = thisitem;
	    do {
		next = nextitem(next);
	    } while (wewant(next) && (nfrontp > next));
	    length = next-thisitem;
	    bcopy(thisitem, good, length);
	    good += length;
	    thisitem = next;
	} else {
	    thisitem = nextitem(thisitem);
	}
    }

    nbackp = netobuf;
    nfrontp = good;		/* next byte to be sent */
    neturg = 0;
}

/*
 *  netflush
 *	Send as much data as possible to the network,
 *	handling requests for urgent data.
 */

void netflush()
{
    int n;

    if ((n = nfrontp - nbackp) > 0) {
	/*
	 * if no urgent data, or if the other side appears to be an
	 * old 4.2 client (and thus unable to survive TCP urgent data),
	 * write the entire buffer in non-OOB mode.
	 */
	if ((neturg == 0) || (not42 == 0)) {
#ifdef NET_RW
	    /* IOdebug("write(net,%x,%d)", nbackp, n); */
	    n = write(net, nbackp, n);	/* normal write */
	    /* IOdebug("result %d %d", n, n < 0 ? errno : 0); */
#else
	    n = write(net, nbackp, n);	/* normal write */
#endif
	} else {
	    n = neturg - nbackp;
	    /*
	     * In 4.2 (and 4.3) systems, there is some question about
	     * what byte in a sendOOB operation is the "OOB" data.
	     * To make ourselves compatible, we only send ONE byte
	     * out of band, the one WE THINK should be OOB (though
	     * we really have more the TCP philosophy of urgent data
	     * rather than the Unix philosophy of OOB data).
	     */
	    if (n > 1) {
#ifdef NET_RW
		/* IOdebug("write(net,%x,%d)", nbackp, n-1);*/
		n = send(net, nbackp, n-1, 0);	/* send URGENT all by itself */
		/* IOdebug("result %d %d", n, n < 0 ? errno : 0); */
#else
		n = send(net, nbackp, n-1, 0);	/* send URGENT all by itself */
#endif
	    } else {
#ifdef NET_RW
		/* IOdebug("write(net,%x,%d,MSG_OOB)", nbackp, n); */
		n = send(net, nbackp, n, MSG_OOB);	/* URGENT data */
		/* IOdebug("result %d %d", n, n < 0 ? errno : 0); */
#else
		n = send(net, nbackp, n, MSG_OOB);	/* URGENT data */
#endif
	    }
	}
    }
    if (n < 0) {
	if (errno == EWOULDBLOCK)
	    return;
	/* should blow this guy away... */
	return; 
    }
    nbackp += n;
    if (nbackp >= neturg) {
	neturg = 0;
    }
    if (nbackp == nfrontp) {
	nbackp = nfrontp = netobuf;
    }
}


#ifndef kill_is_working_well
void reapchld(void)
{
	int status = 0;
	int pid;

	signal(SIGCHLD, (void(*)())reapchld);

	while ( (pid = wait2(&status, WNOHANG)) > 0)
	{
	    if (pid == login_id) 
	    	login_id = 0;
	    if (pid == server_id)
	    	server_id = 0;
	}
}
#endif

void cleanup(void)
{
#ifdef __HELIOS
	int pid;

#ifdef kill_is_working_well
        int status = 0;

	signal(SIGTERM, SIG_IGN);

	close(pty);
	
	kill (0, SIGTERM);
	while ( (pid = wait(&status)) > 0);
#else
	signal(SIGTERM, SIG_IGN);

	close(pty);
	
	reapchld();

    	if ((pid = login_id) > 0) {
/*	    IOdebug("telnetd: term %d: %d", pid, kill(pid, SIGTERM));*/
	    login_id = 0;
	}
    	if ((pid = server_id) > 0) {
/*	    IOdebug("telnetd: term %d: %d", pid, kill(pid,SIGTERM));*/
	    server_id = 0;
	}
#endif


#else
	char *p;

	p = line + sizeof("/dev/") - 1;
	if (logout(p))
	    logwtmp(p, "", "");
	(void)chmod(line, 0666);
	(void)chown(line, 0, 0);
	*p = 'p';
	(void)chmod(line, 0666);
	(void)chown(line, 0, 0);

	shutdown(net,2);
#endif

#ifdef IODEBUG
	{
	    int e;
	    
	    IOdebug( "telnetd: shutdown(net,2)" );
	    
	    if ((e=shutdown(net, 2)) < 0)
		IOdebug("telnetd: shutdown error %d", errno );
	    else
		IOdebug("telnetd: shutdown done");
	}
#else
	shutdown(net, 2);
#endif

	exit(1);
}


char	editedhost[32];

void edithost(
	register char *pat,
	register char *host )

{
	register char *res = editedhost;

	if (!pat)
		pat = "";
	while (*pat) {
		switch (*pat) {

		case '#':
			if (*host)
				host++;
			break;

		case '@':
			if (*host)
				*res++ = *host++;
			break;

		default:
			*res++ = *pat;
			break;

		}
		if (res == &editedhost[sizeof editedhost - 1]) {
			*res = '\0';
			return;
		}
		pat++;
	}
	if (*host)
		strncpy(res, host, sizeof editedhost - (res - editedhost) - 1);
	else
		*res = '\0';
	editedhost[sizeof editedhost - 1] = '\0';
}

static char *putlocation;

void tputs(
register char *s )
{

	while (*s)
		putchr(*s++);
}

void putchr(int cc)
{
	*putlocation++ = cc;
}

#ifdef __HELIOS
void get_date(char *buffer)
{
	time_t now;
	time(&now);
	strcpy(buffer, ctime(&now));
}
#endif


void putf(
register char *cp,
char *where )
{
	char *slash;
	char datebuffer[60];
#ifndef __HELIOS
	extern char *rindex();
#endif

	putlocation = where;

	while (*cp) {
		if (*cp != '%') {
			putchr(*cp++);
			continue;
		}
		switch (*++cp) {

		case 't':
#ifdef __HELIOS
			slash = strchr(tty_path, '/');
#else
			slash = rindex(line, '/');
#endif
			if (slash == (char *) 0)
				tputs(line);
			else
				tputs(&slash[1]);
			break;

		case 'h':
			tputs(editedhost);
			break;

		case 'd':
			get_date(datebuffer);
			tputs(datebuffer);
			break;

		case '%':
			putchr('%');
			break;
		}
		cp++;
	}
}


/* end of telnetd.c */
