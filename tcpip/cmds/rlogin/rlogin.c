
/*
 * rlogin - remote login
 */
#include <sys/param.h>
#include <sys/errno.h>
#include <sys/file.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/wait.h>

#include <netinet/in.h>

#include <stdio.h>
#include <sgtty.h>
#include <errno.h>
#include <pwd.h>
#include <signal.h>
#include <setjmp.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>

#ifdef NEW_SYSTEM
#include <bsd.h>
#endif

# ifndef TIOCPKT_WINDOW
# define TIOCPKT_WINDOW 0x80
# endif /* TIOCPKT_WINDOW */

/* concession to sun */
# ifndef SIGUSR1
# define SIGUSR1 30
# endif /* SIGUSR1 */

char	*name;
int	rem;
char	cmdchar = '~';
int	eight;
int	litout;
char	*speeds[] =
    { "0", "50", "75", "110", "134", "150", "200", "300",
      "600", "1200", "1800", "2400", "4800", "9600", "19200", "38400" };
char	term[256] = "network";
extern	int errno;

int	dosigwinch = 0;
#ifndef sigmask
#define sigmask(m)	(1 << ((m)-1))
#endif
#ifdef sun
struct winsize {
	unsigned short ws_row, ws_col;
	unsigned short ws_xpixel, ws_ypixel;
};
#endif /* sun */
struct	winsize winsize;

/*
 * The following routine provides compatibility (such as it is)
 * between 4.2BSD Suns and others.  Suns have only a `ttysize',
 * so we convert it to a winsize.
 */
#ifdef sun
int
get_window_size(fd, wp)
	int fd;
	struct winsize *wp;
{
	struct ttysize ts;
	int error;

	if ((error = ioctl(0, TIOCGSIZE, (char *)&ts)) != 0)
		return (error);
	wp->ws_row = ts.ts_lines;
	wp->ws_col = ts.ts_cols;
	wp->ws_xpixel = 0;
	wp->ws_ypixel = 0;
	return (0);
}
#else /* sun */
#define get_window_size(fd, wp)	ioctl(fd, TIOCGWINSZ, (char *)wp)
#endif /* sun */
#define bufmax 256

char	rcvbuf[bufmax];
int 	rcvcnt;
int	send_window;

int	defflags, tabflag;
int	deflflags;
char	deferase, defkill;
struct	tchars deftc;
struct	ltchars defltc;
struct	tchars notc =	{ (char) -1, (char) -1, (char) -1, (char) -1, (char) -1, (char) -1 };
struct	ltchars noltc =	{ (char) -1, (char) -1, (char) -1, (char) -1, (char) -1, (char) -1 };


static char waste[BUFSIZ];

void
oob()
{
	int out = FWRITE, atmark, n;
	int rcvd = 0;
	char mark;
	struct sgttyb sb;

	while (recv(rem, &mark, 1, MSG_OOB) < 0)
	{
		switch (errno) {
		
		case EWOULDBLOCK:
			/*
			 * Urgent data not here yet.
			 * It may not be possible to send it yet
			 * if we are blocked for output
			 * and our input buffer is full.
			 */
			if (rcvcnt < sizeof(rcvbuf)) {
				n = read(rem, rcvbuf + rcvcnt,
					sizeof(rcvbuf) - rcvcnt);
				if (n <= 0)
					return;
				rcvd += n;
			} 
			else 
			{
				n = read(rem, waste, sizeof(waste));
				if (n <= 0)
					return;
			}
			continue;
				
		default:
			return;
		}
	}

	if (mark & TIOCPKT_WINDOW) {
		/*
		 * Let server know about window size changes
		 */
		 send_window = 1;		 
	}
	if (!eight && (mark & TIOCPKT_NOSTOP)) {
		(void) ioctl(0, TIOCGETP, (char *)&sb);
		sb.sg_flags &= ~CBREAK;
		sb.sg_flags |= RAW;
		(void) ioctl(0, TIOCSETN, (char *)&sb);
		notc.t_stopc  = (char) -1;
		notc.t_startc = (char) -1;
		(void) ioctl(0, TIOCSETC, (char *)&notc);
	}
	if (!eight && (mark & TIOCPKT_DOSTOP)) {
		(void) ioctl(0, TIOCGETP, (char *)&sb);
#if 0
		sb.sg_flags &= ~RAW;
#endif
		sb.sg_flags |= CBREAK;
		(void) ioctl(0, TIOCSETN, (char *)&sb);
		notc.t_stopc = deftc.t_stopc;
		notc.t_startc = deftc.t_startc;
		(void) ioctl(0, TIOCSETC, (char *)&notc);
	}
	if (mark & TIOCPKT_FLUSHWRITE) {
		(void) ioctl(1, TIOCFLUSH, (char *)&out);
		for (;;) {
			if (ioctl(rem, SIOCATMARK, (char *)&atmark) < 0) {
				perror("ioctl");
				break;
			}
			if (atmark)
				break;
			n = read(rem, waste, sizeof (waste));
			if (n <= 0)
				break;
		}
	}
}

/*
 * Send the window size to the server via the magic escape
 */
void
sendwindow()
{
	int n;
	static char obuf[4 + sizeof (struct winsize)];
	struct winsize *wp = (struct winsize *)(obuf+4);
	obuf[0] = 0377;
	obuf[1] = 0377;
	obuf[2] = 's';
	obuf[3] = 's';
	wp->ws_row = htons(winsize.ws_row);
	wp->ws_col = htons(winsize.ws_col);
	wp->ws_xpixel = htons(winsize.ws_xpixel);
	wp->ws_ypixel = htons(winsize.ws_ypixel);
	n = write(rem, obuf, sizeof(obuf));
}

void
mode(int f)
{
	struct tchars *tc;
	struct ltchars *ltc;
	struct sgttyb sb;
	int	lflags;

	(void) ioctl(0, TIOCGETP, (char *)&sb);
	(void) ioctl(0, TIOCLGET, (char *)&lflags);
	switch (f) {

	case 0:
#ifdef __HELIOS
		sb.sg_flags |= ECHO|CRMOD;
		sb.sg_flags &= ~(RAW|CBREAK);
#else
		sb.sg_flags &= ~(CBREAK|RAW|TBDELAY);
		sb.sg_flags |= defflags|tabflag;
#endif
		tc = &deftc;
		ltc = &defltc;
		sb.sg_kill = defkill;
		sb.sg_erase = deferase;
		lflags = deflflags;
		break;

	case 1:
#ifdef __HELIOS
		sb.sg_flags &= ~(ECHO|CRMOD);
		sb.sg_flags |= RAW|CBREAK;
#else
		sb.sg_flags |= (eight ? RAW : CBREAK|RAW);
		sb.sg_flags &= ~defflags;
		/* preserve tab delays, but turn off XTABS */
		if ((sb.sg_flags & TBDELAY) == XTABS)
			sb.sg_flags &= ~TBDELAY;
#endif
		tc = &notc;
		ltc = &noltc;
		sb.sg_kill = sb.sg_erase = (char) -1;
		if (litout)
			lflags |= LLITOUT;
		break;

	default:
		return;
	}
#ifndef __HELIOS
	(void) ioctl(0, TIOCSLTC, (char *)ltc);
	(void) ioctl(0, TIOCSETC, (char *)tc);
	(void) ioctl(0, TIOCLSET, (char *)&lflags);
#endif
	(void) ioctl(0, TIOCSETN, (char *)&sb);
}

void
echo(register char c)
{
	char buf[8];
	register char *p = buf;

	c &= 0177;
	*p++ = cmdchar;
	if (c < ' ') {
		*p++ = '^';
		*p++ = c + '@';
	} else if (c == 0177) {
		*p++ = '^';
		*p++ = '?';
	} else
		*p++ = c;
	*p++ = '\r';
	*p++ = '\n';
	(void) write(1, buf, p - buf);
}

int doit(int oldmask)
{
	int c, n;
	fd_set rset, xset;
	struct sgttyb sb;
	int bol = 1;
	int local = 0;
		
	FD_ZERO(&rset);	
	FD_ZERO(&xset);

	FD_SET(0,&rset);
	FD_SET(rem,&rset);
	FD_SET(rem,&xset);
	
	(void) ioctl(0, TIOCGETP, (char *)&sb);
	defflags  = (int) sb.sg_flags;
	tabflag   = defflags & TBDELAY;
	defflags &= ECHO | CRMOD;
	deferase  = sb.sg_erase;
	defkill   = sb.sg_kill;
	(void) ioctl(0, TIOCLGET, (char *)&deflflags);
	(void) ioctl(0, TIOCGETC, (char *)&deftc);
	notc.t_startc = deftc.t_startc;
	notc.t_stopc = deftc.t_stopc;
	(void) ioctl(0, TIOCGLTC, (char *)&defltc);
	(void) signal(SIGINT, SIG_IGN);
	signal(SIGHUP, (void(*)())exit);
	signal(SIGQUIT, (void(*)())exit);

	mode(1);
		
	while( rset.fds_bits[0] || xset.fds_bits[0] )
	{
		fd_set rbits, xbits;

		rbits = rset;
		xbits = xset;

		c = select(rem+1, (int *)&rbits, 0, (int *)&xbits, 0);

#if 0
IOdebug("selected c %d r %x x %x o %x ",c,rbits.fds_bits[0],xbits.fds_bits[0],oserr);
#endif

		if( c < 0 )
		{
			if( errno == EINTR ) continue;
			perror("select");
			continue;
		}
		
		if( send_window )
		{
			sendwindow();
			send_window = 0;
		}
		
		if( FD_ISSET(0,&rbits) )
		{
			char c = 0;
			/* data from terminal, sent to rem */
			n = read(0,&c,1);
			if( n == 0 )
			{
				FD_CLR(0,&rset);
				continue;
			}
			else if( n < 0 )
			{
				FD_CLR(0,&rset);
				perror("read 0");
				continue;
			}
			/*
			 * If we're at the beginning of the line
			 * and recognize a command character, then
			 * we echo locally.  Otherwise, characters
			 * are echo'd remotely.  If the command
			 * character is doubled, this acts as a 
			 * force and local echo is suppressed.
			 */
			if (bol) {
				bol = 0;
				if (c == cmdchar) {
					bol = 0;
					local = 1;
					continue;
				}
			} else if (local) {
				local = 0;
				if (c == '.' || c == deftc.t_eofc) {
					echo(c);
					break;
				}
#ifndef __HELIOS
				if (c == defltc.t_suspc || c == defltc.t_dsuspc) {
					bol = 1;
					echo(c);
					stop(c);
					continue;
				}
#endif
				if (c != cmdchar)
					(void) write(rem, &cmdchar, 1);
			}
			if (write(rem, &c, 1) == 0) {
				printf("line gone\r\n");
				break;
			}
			bol = c == defkill || c == deftc.t_eofc ||
			    c == deftc.t_intrc || c == defltc.t_suspc ||
			    c == '\r' || c == '\n';
			
		}
		
		if( FD_ISSET(rem,&rbits) )
		{
			/* data from rem, send to terminal */
			n = read(rem,rcvbuf,bufmax);

			if( n == 0 )
			{
				break;
			}
			else if( n < 0 )
			{
				FD_CLR(rem,&rset);
				FD_CLR(rem,&xset);
				perror("read rem");
				continue;
			}
			rcvcnt = n;
		}
		
		if( rcvcnt > 0 )
		{
			n = write(1,rcvbuf,rcvcnt);

			if( n <= 0 )
			{
				perror("write 1");
				continue;
			}
			rcvcnt = 0;
		}
		
		if( FD_ISSET(rem,&xbits) )
		{
			/* OOB data from socket */
			oob();
		}
	}
	mode(0);
	printf("Connection closed\n");
	exit(0);
}

int
main(argc, argv)
	int argc;
	char **argv;
{
	char *host, *cp;
	struct passwd *pwd;
	struct servent *sp;
	int uid, options = 0, oldmask;
	int on = 1;

	host = rindex(argv[0], '/');
	if (host)
		host++;
	else
		host = argv[0];
	argv++, --argc;
	if (!strcmp(host, "rlogin"))
		host = *argv++, --argc;
another:
	if (argc > 0 && !strcmp(*argv, "-d")) {
		argv++, argc--;
		options |= SO_DEBUG;
		goto another;
	}
	if (argc > 0 && !strcmp(*argv, "-l")) {
		argv++, argc--;
		if (argc == 0)
			goto usage;
		name = *argv++; argc--;
		goto another;
	}
	if (argc > 0 && !strncmp(*argv, "-e", 2)) {
		cmdchar = argv[0][2];
		argv++, argc--;
		goto another;
	}
	if (argc > 0 && !strcmp(*argv, "-8")) {
		eight = 1;
		argv++, argc--;
		goto another;
	}
	if (argc > 0 && !strcmp(*argv, "-L")) {
		litout = 1;
		argv++, argc--;
		goto another;
	}
	if (host == 0)
		goto usage;
	if (argc > 0)
		goto usage;
	pwd = getpwuid(getuid());
	if (pwd == 0) {
		fprintf(stderr, "Who are you?\n");
		exit(1);
	}
	sp = getservbyname("login", "tcp");
	if (sp == 0) {
		fprintf(stderr, "rlogin: login/tcp: unknown service\n");
		exit(2);
	}
	cp = getenv("TERM");
	if (cp)
		(void) strcpy(term, cp);

	strcat(term, "/");
	strcat(term, "9600");

	(void) get_window_size(0, &winsize);

        rem = rcmd(&host, sp->s_port, pwd->pw_name, name ? name : pwd->pw_name, term, 0);
        if (rem < 0)
                exit(1);
	if (options & SO_DEBUG &&
	    setsockopt(rem, SOL_SOCKET, SO_DEBUG, (char *)&on, sizeof (on)) < 0)
		perror("rlogin: setsockopt (SO_DEBUG)");
	uid = getuid();
	if (setuid(uid) < 0) {
		perror("rlogin: setuid");
		exit(1);
	}
	doit(oldmask);
	/*NOTREACHED*/
usage:
	fprintf(stderr,
	    "usage: rlogin host [ -ex ] [ -l username ] [ -8 ] [ -L ]\n");
	exit(1);
}

