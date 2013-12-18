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
static char sccsid[] = "@(#)rlogind.c	5.22.1.6 (Berkeley) 2/7/89";
#endif /* not lint */

/*
 * remote login server:
 *	\0
 *	remuser\0
 *	locuser\0
 *	terminal_type/speed\0
 *	data
 *
 * Automatic login protocol is done here, using login -f upon success,
 * unless OLD_LOGIN is defined (then done in login, ala 4.2/4.3BSD).
 */

#if defined __HELIOS 
#include <syslib.h>
#include <unistd.h>
#include <limits.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <getopt.h>
#ifdef NEW_SYSTEM
  #include <bsd.h>
#endif
#endif

#include <stdio.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/file.h>

#include <netinet/in.h>

#include <errno.h>
#include <pwd.h>
#include <signal.h>
#include <sgtty.h>
#include <stdio.h>
#include <netdb.h>
#include <syslog.h>
#include <strings.h>


#ifndef TIOCPKT_WINDOW
#define TIOCPKT_WINDOW 0x80
#endif

extern	char **environ;

char	*env[2];
#define	NMAX 30
char	lusername[NMAX+1], rusername[NMAX+1];
static	char term[64] = "TERM=";
#define	ENVSIZE	(sizeof("TERM=")-1)	/* skip null for concatenation */
int	keepalive = 1;

#define	SUPERUSER(pwd)	((pwd)->pw_uid == 0)

extern	int errno;
struct	passwd *pwd;


int	child;
void	cleanup( void );
int	netf;
char	*line;

struct winsize win = { 0, 0, 0, 0 };


void
fatal(
	int f,
	char *msg )
{
	char buf[BUFSIZ];

	buf[0] = '\01';		/* error indicator */
	(void) sprintf(buf + 1, "rlogind: %s.\r\n", msg);
	(void) write(f, buf, strlen(buf));
	exit(1);
}

void
fatalperror(
	int f,
	char *msg )
{
	char buf[BUFSIZ];
	extern int sys_nerr;
	extern char *sys_errlist[];

	if ((unsigned)errno < sys_nerr)
		(void) sprintf(buf, "%s: %s", msg, sys_errlist[errno]);
	else
		(void) sprintf(buf, "%s: Error %d", msg, errno);
	fatal(f, buf);
}

char	magic[2] = { 0377, 0377 };
char	oobdata[] = {TIOCPKT_WINDOW};

/*
 * Handle a "control" request (signaled by magic being present)
 * in the data stream.  For now, we are only willing to handle
 * window size changes.
 */
int
control(pty, cp, n)
	int pty;
	char *cp;
	int n;
{
	struct winsize w;

	if (n < 4+sizeof (w) || cp[2] != 's' || cp[3] != 's')
		return (0);
	oobdata[0] &= ~TIOCPKT_WINDOW;	/* we know he heard */
	bcopy(cp+4, (char *)&w, sizeof(w));
	w.ws_row = ntohs(w.ws_row);
	w.ws_col = ntohs(w.ws_col);
	w.ws_xpixel = ntohs(w.ws_xpixel);
	w.ws_ypixel = ntohs(w.ws_ypixel);
	(void)ioctl(pty, TIOCSWINSZ, (char *)&w);
	return (4+sizeof (w));
}

/*
 * rlogin "protocol" machine.
 */
void
protocol(f, p)
	int f, p;
{
	char pibuf[1024], fibuf[1024], *pbp, *fbp;
	register pcc = 0, fcc = 0;
	int cc, nfd, pmask, fmask;
	char cntl;

	/*
	 * Must ignore SIGTTOU, otherwise we'll stop
	 * when we try and set slave pty's window shape
	 * (our controlling tty is the master pty).
	 */
	(void) signal(SIGTTOU, SIG_IGN);
	send(f, oobdata, 1, MSG_OOB);	/* indicate new rlogin */
	if (f > p)
		nfd = f + 1;
	else
		nfd = p + 1;
	fmask = 1 << f;
	pmask = 1 << p;
	for (;;) {
		int ibits, obits, ebits;

		ibits = 0;
		obits = 0;
		if (fcc)
			obits |= pmask;
		else
			ibits |= fmask;
		if (pcc >= 0)
		  {
			if (pcc)
				obits |= fmask;
			else
				ibits |= pmask;
		      }
		
		ebits = pmask;
		if (select(nfd, &ibits, obits ? &obits : (int *)NULL,
		    &ebits, 0) < 0) {
			if (errno == EINTR)
				continue;
			fatalperror(f, "select");
		}
		if (ibits == 0 && obits == 0 && ebits == 0) {
			/* shouldn't happen... */
			sleep(5);
			continue;
		}
#define	pkcontrol(c)	((c)&(TIOCPKT_FLUSHWRITE|TIOCPKT_NOSTOP|TIOCPKT_DOSTOP))
		if (ebits & pmask) {
			cc = read(p, &cntl, 1);
			if (cc == 1 && pkcontrol(cntl)) {
				cntl |= oobdata[0];
				send(f, &cntl, 1, MSG_OOB);
				if (cntl & TIOCPKT_FLUSHWRITE) {
					pcc = 0;
					ibits &= ~pmask;
				}
			}
		}
		if (ibits & fmask) {
			fcc = read(f, fibuf, sizeof(fibuf));
			if (fcc < 0 && errno == EWOULDBLOCK)
				fcc = 0;
			else {
				register char *cp;
				int left, n;

				if (fcc <= 0)
					break;
				fbp = fibuf;

			top:
				for (cp = fibuf; cp < fibuf+fcc-1; cp++)
					if (cp[0] == magic[0] &&
					    cp[1] == magic[1]) {
						left = fcc - (cp-fibuf);
						n = control(p, cp, left);
						if (n) {
							left -= n;
							if (left > 0)
								bcopy(cp+n, cp, left);
							fcc -= n;
							goto top; /* n^2 */
						}
					}
				obits |= pmask;		/* try write */
			}
		}

		if ((obits & pmask) && fcc > 0) {
			cc = write(p, fbp, fcc);
			if (cc > 0) {
				fcc -= cc;
				fbp += cc;
			}
		}

		if (ibits & pmask) {
			pcc = read(p, pibuf, sizeof (pibuf));
			pbp = pibuf;
			if (pcc < 0 && errno == EWOULDBLOCK)
				pcc = 0;
			else if (pcc <= 0)
				break;
			else if (pibuf[0] == 0) {
				pbp++, pcc--;
				obits |= fmask;	/* try a write */
			} else {
				if (pkcontrol(pibuf[0])) {
					pibuf[0] |= oobdata[0];
					send(f, &pibuf[0], 1, MSG_OOB);
				}
				pcc = 0;
			}
		}
		if ((obits & fmask) && pcc > 0) {
			cc = write(f, pbp, pcc);
			if (cc < 0 && errno == EWOULDBLOCK) {
				/* also shouldn't happen */
				sleep(5);
				continue;
			}
			if (cc > 0) {
				pcc -= cc;
				pbp += cc;
			}
		}
	}
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
local_domain(char *h)
{
	char localhost[MAXHOSTNAMELEN];
	char *p1, *p2 = index(h, '.');

	(void) gethostname(localhost, sizeof(localhost));
	p1 = index(localhost, '.');
	if (p1 == NULL || p2 == NULL || !strcasecmp(p1, p2))
		return(1);
	return(0);
}

char *speeds[] = {
	"0", "50", "75", "110", "134", "150", "200", "300", "600",
	"1200", "1800", "2400", "4800", "9600", "19200", "38400",
};
#define	NSPEEDS	(sizeof(speeds) / sizeof(speeds[0]))

void
setup_term(int fd)
{
	register char *cp = index(term, '/'), **cpp;
	struct sgttyb sgttyb;
	char *speed;

	(void)ioctl(fd, TIOCGETP, (char *)&sgttyb);
	if (cp) {
		*cp++ = '\0';
		speed = cp;
		cp = index(speed, '/');
		if (cp)
			*cp++ = '\0';
		for (cpp = speeds; cpp < &speeds[NSPEEDS]; cpp++)
		    if (strcmp(*cpp, speed) == 0) {
			sgttyb.sg_ispeed = sgttyb.sg_ospeed = cpp - speeds;
			break;
		    }
	}
	sgttyb.sg_flags = ECHO|CRMOD|ANYP|XTABS;
	(void)ioctl(fd, TIOCSETP, (char *)&sgttyb);

	env[0] = term;
	env[1] = 0;
	environ = env;
}

void
getstr(
       char *buf,
       int cnt,
       char *errmsg )
{
	char c;

	do {
		if (read(0, &c, 1) != 1)
			exit(1);
		if (--cnt < 0)
			fatal(1, errmsg);
		*buf++ = c;
	} while (c != 0);
}

int
do_rlogin(char *host)
{

	getstr(rusername, sizeof(rusername), "remuser too long");
	getstr(lusername, sizeof(lusername), "locuser too long");
	getstr(term+ENVSIZE, sizeof(term)-ENVSIZE, "Terminal type too long");

#ifndef __HELIOS
	if (getuid())
		return(-1);
#endif
	pwd = getpwnam(lusername);
	if (pwd == NULL)
		return(-1);
	return(ruserok(host, SUPERUSER(pwd), rusername, lusername));
}

void
doit(
     int f,
     struct sockaddr_in *fromp )
{
#ifndef __HELIOS
	int i;
#endif
	int p, t, pid, on = 1;
#ifndef OLD_LOGIN
	int authenticated = 0, hostok = 0;
	char remotehost[2 * MAXHOSTNAMELEN + 1];
#endif
	register struct hostent *hp;
	struct hostent hostent;
	char c;

	alarm(60);
	read(f, &c, 1);
	if (c != 0)
		exit(1);

	alarm(0);
	fromp->sin_port = ntohs((u_short)fromp->sin_port);
	hp = gethostbyaddr(&fromp->sin_addr, sizeof (struct in_addr),
		fromp->sin_family);
	if (hp == 0) {
		/*
		 * Only the name is used below.
		 */
		hp = &hostent;
		hp->h_name = inet_ntoa(fromp->sin_addr);
#ifndef OLD_LOGIN
		hostok++;
#endif
	}
#ifndef OLD_LOGIN
	else if (local_domain(hp->h_name)) {
		/*
		 * If name returned by gethostbyaddr is in our domain,
		 * attempt to verify that we haven't been fooled by someone
		 * in a remote net; look up the name and check that this
		 * address corresponds to the name.
		 */
		strncpy(remotehost, hp->h_name, sizeof(remotehost) - 1);
		remotehost[sizeof(remotehost) - 1] = 0;
		hp = gethostbyname(remotehost);
		if (hp)
		    for (; hp->h_addr_list[0]; hp->h_addr_list++)
			if (!bcmp(hp->h_addr_list[0], (caddr_t)&fromp->sin_addr,
			    sizeof(fromp->sin_addr))) {
				hostok++;
				break;
			}
	} else
		hostok++;
#endif

	if (fromp->sin_family != AF_INET ||
	    fromp->sin_port >= IPPORT_RESERVED ||
	    fromp->sin_port < IPPORT_RESERVED/2) {
		syslog(LOG_NOTICE, "Connection from %s on illegal port",
			inet_ntoa(fromp->sin_addr));
		fatal(f, "Permission denied");
	}
#ifdef IP_OPTIONS
      {
	u_char optbuf[BUFSIZ/3], *cp;
	char lbuf[BUFSIZ], *lp;
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
	write(f, "", 1);
#ifndef OLD_LOGIN
	if (do_rlogin(hp->h_name) == 0) {
		if (hostok)
		    authenticated++;
		else
		    write(f, "rlogind: Host address mismatch.\r\n",
		     sizeof("rlogind: Host address mismatch.\r\n") - 1);
	}
#endif

#ifdef __HELIOS
    {
    	Object *OV_Co, *OV_Cs;
	static char pty_path[_POSIX_PATH_MAX];
	static char tty_path[_POSIX_PATH_MAX];
	static char termtype[64];
	int server_id;
	Environ *my_environ = (Environ*)getenviron();
	int pty;

	strcpy(termtype,term+ENVSIZE);
	*index(termtype,'/') = '\0';
			    	
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
	    if (server_id == 0) {
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
		/* -t type   set terminal type		*/
		/*					*/
		execl("/helios/lib/ttyserv", "ttyserv", "-p", "-t", termtype, NULL);
		syslog(LOG_ERR, "/helios/lib/ttyserv: %m\r\n");
		fatalperror(2, "/helios/lib/ttyserv");
		/*NOTREACHED*/
	    }
	    /* parent */
#ifdef IODEBUG
	    IOdebug("tty started, in child now");
#endif
	    close(fda[1]);

	    /* wait for tty name table entry */ 
	    e = read(fda[0], tty_path, _POSIX_PATH_MAX);
	    if (e <= 0) {
	    	kill(server_id, SIGTERM);
		fatalperror(f, "no tty_path");
	    }
	    tty_path[e] = '\0';
	
#ifdef IODEBUG 
	    IOdebug("tty name table entry is: '%s'", tty_path);	
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
	    IOdebug("pty name table entry is: '%s'", pty_path);	
#endif

	    /* open pty */
	    strcat(pty_path, "/entry");
	    p = open(pty_path, O_RDWR|O_NONBLOCK);
	    if (p < 0) {
	    	kill(server_id, SIGTERM);
		fatalperror(f, pty_path);
	    }
	    pty = p;

#ifdef IODEBUG 
   	    IOdebug("telnetd: pty is %s", pty_path);
#endif
	}
	
	dup2(f, 0);
	netf = f;
		
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
    }
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
	fatal(f, "Out of ptys");
	/*NOTREACHED*/
gotpty:
	(void) ioctl(p, TIOCSWINSZ, (char *)&win);
	netf = f;
	line[strlen("/dev/")] = 't';
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
#endif /* __HELIOS */

	setup_term(t);

#ifdef DEBUG
	{
		int tt = open("/dev/tty", O_RDWR);
		if (tt > 0) {
			(void)ioctl(tt, TIOCNOTTY, 0);
			(void)close(tt);
		}
	}
#endif
	pid = fork();
	if (pid < 0)
		fatalperror(f, "");
	if (pid == 0) {
		close(f), close(p);
		dup2(t, 0), dup2(t, 1), dup2(t, 2);
		close(t);
#ifdef __HELIOS
		close(3);
		if (authenticated)
			execl("/helios/bin/login", "login", "-p", "-h", hp->h_name,
			    "-f", lusername, 0);
		else
			execl("/helios/bin/login", "login", "-p", "-h", hp->h_name,
				lusername, 0);
#else
#ifdef OLD_LOGIN
		execl("/bin/login", "login", "-r", hp->h_name, 0);
#else /* OLD_LOGIN */
		if (authenticated)
			execl("/bin/login", "login", "-p", "-h", hp->h_name,
			    "-f", lusername, 0);
		else
			execl("/bin/login", "login", "-p", "-h", hp->h_name,
			    lusername, 0);
#endif /* OLD_LOGIN */
#endif
		fatalperror(2, "/bin/login");
		/*NOTREACHED*/
	}
	close(t);

	ioctl(f, FIONBIO, (char *)&on);
	ioctl(p, FIONBIO, (char *)&on);
	ioctl(p, TIOCPKT, (char *)&on);
	signal(SIGTSTP, SIG_IGN);
	signal(SIGCHLD, (void(*)())cleanup);
#ifndef __HELIOS
	setpgrp(0, 0);
#endif
	protocol(f, p);
	signal(SIGCHLD, SIG_IGN);
	cleanup();
}


void
cleanup( void )
{
#ifndef __HELIOS
	char *p;

	p = line + sizeof("/dev/") - 1;
	if (logout(p))
		logwtmp(p, "", "");
	(void)chmod(line, 0666);
	(void)chown(line, 0, 0);
	*p = 'p';
	(void)chmod(line, 0666);
	(void)chown(line, 0, 0);
#endif
	shutdown(netf, 2);
	exit(1);
}

#ifndef OLD_LOGIN

#endif /* OLD_LOGIN */

int
main(
     int argc,
     char **argv )
{
	extern int opterr, optind, _check_rhosts_file;
	int ch;
	int on = 1, fromlen;
	struct sockaddr_in from;

	openlog("rlogind", LOG_PID | LOG_CONS, LOG_AUTH);

	opterr = 0;
	while ((ch = getopt(argc, argv, "ln")) != EOF)
		switch (ch) {
		case 'l':
			_check_rhosts_file = 0;
			break;
		case 'n':
			keepalive = 0;
			break;
		case '?':
		default:
			syslog(LOG_ERR, "usage: rlogind [-l] [-n]");
			break;
		}
	argc -= optind;
	argv += optind;

	fromlen = sizeof (from);
	if (getpeername(0, (struct sockaddr *)&from, &fromlen) < 0) {
		syslog(LOG_ERR, "Couldn't get peer name of remote host: %m");
		fatalperror(2,"Can't get peer name of host");
	}
	if (keepalive &&
	    setsockopt(0, SOL_SOCKET, SO_KEEPALIVE, (char *)&on, sizeof (on)) < 0)
		syslog(LOG_WARNING, "setsockopt (SO_KEEPALIVE): %m");
	doit(0, &from);
}
