#ifndef __HELIOS
#ifndef lint
static char sccsid[] = "@(#)delivermail.c	4.36 (Berkeley) 4/21/89";
#endif
#else
#ifdef __TRAN
static char *rcsid = "$Header: /users/nickc/RTNucleus/tcpip/cmds/binmail/RCS/binmail.c,v 1.13 1994/03/17 16:38:25 nickc Exp $";
#endif
#endif

#include <sys/param.h>
#include <sys/stat.h>
#include <sys/file.h>

#include <ctype.h>
#include <stdio.h>
#include <pwd.h>
#ifndef __HELIOS
#include <utmp.h>
#endif
#include <signal.h>
#include <setjmp.h>
#include <sysexits.h>
#include "pathnames.h"

#ifdef __HELIOS
#include <netdb.h>
#include "f_lock.h"
#include <stdarg.h>
#include <nonansi.h> 
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <syslog.h>
#include <attrib.h>
#endif
	/* copylet flags */
#define REMOTE		1		/* remote mail, add rmtmsg */
#define ORDINARY	2
#define ZAP		3		/* zap header and trailing empty line */
#ifndef __HELIOS
#define	FORWARD		4
#else
#define	FOR_WARD	4
#endif

#define	LSIZE		256
#define	MAXLET		300		/* maximum number of letters */
#define	MAILMODE	0600		/* mode of created mail */

char	line[LSIZE];
char	resp[LSIZE];
struct let {
	long	adr;
	char	change;
} let[MAXLET];
int	nlet	= 0;
char	lfil[50];
#ifndef __HELIOS
long	iop, time();
char	*getenv();
char	*index();
#else
time_t	iop, time(time_t *);
char	*getenv(const char *);
char	*index(char *, char);
#endif
char	lettmp[] = _PATH_TMP;
char	maildir[] = _PATH_MAILDIR;
#ifndef __HELIOS
char	mailfile[] = "/usr/spool/mail/xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";
#else
char	mailfile[] = "/helios/local/spool/mail/xxxxxxxxxxxxxxxxxxxxxxxxxxxx";
#endif
#ifndef __HELIOS
char	dead[] = "dead.letter";
#else
char	dead[256] ;
#endif
char	forwmsg[] = " forwarded\n";
FILE	*tmpf;
FILE	*malf;
char	my_name[60];
#ifndef __HELIOS
char	*getlogin();
#else
char	*getlogin(void);
#endif
int	error;
int	changed;
int	forward;
char	from[] = "From ";
#ifndef __HELIOS
long	ftell();
int	delex();
char	*ctime();
#else
long	ftell(FILE *);
void	delex(int);
char	*ctime(const time_t *);
#endif
int	flgf;
int	flgp;
#ifndef __HELIOS
int	delflg = 1;
#endif
int	hseqno;
jmp_buf	sjbuf;
#ifndef __HELIOS
int	rmail;
#endif

#ifdef __HELIOS
void done(void) ;
void panic (char *, ...) ;
int any(int, char *) ;
void setsig(int, void (*)(int));
void cat(char *, char *, char *) ;
void copylet(int, FILE *, int) ;
int sendmail(int, char *, char *) ;
int isfrom(char *);
int safefile(char *);
void debugf (char *, ...) ;

extern int f_lock (int, int) ;
extern int sigsetmask(int);
extern int openlog (char *, int, int) ;
extern int syslog (int, char *, ... ) ;

extern char **environ ;
char *name;
#endif

int main(int argc, char **argv)
{
	register int i;
	struct passwd *pwent;
#ifndef __HELIOS
	char *name;
#else
	char *user_name;
	extern int mkstemp (char *) ;
	void printmail(int, char **) ;
	void bulkmail(int, char **) ;
	name = argv[0] ;
#endif	

	openlog("mail", LOG_PID, LOG_MAIL);
	
#ifdef MEM_CHECK
	IOdebug ("entering binmail: Bytes free : %d  Heap size : %d", 
		 Malloc(-1), Malloc(-3));
#endif

#ifndef __HELIOS
	if (!(name = getlogin()) || !*name || !(pwent = getpwnam(name)) ||
#else	
	if ((user_name = getlogin()) == NULL || !*user_name || 
	    (pwent = getpwnam(user_name)) == NULL ||
#endif	
	    getuid() != pwent->pw_uid) 
		pwent = getpwuid(getuid());
	strncpy(my_name, pwent ? pwent->pw_name : "???", sizeof(my_name)-1);
	if (setjmp(sjbuf))
		done();
#ifndef __HELIOS
	for (i=SIGHUP; i<=SIGTERM; i++)
		setsig(i, delex);
#else
	{
		struct sigaction act;
		act.sa_handler = delex;
		act.sa_mask = 0;
		act.sa_flags = SA_ASYNC;
		(void) sigaction(SIGINT, &act, NULL);
		(void) sigaction(SIGHUP, &act, NULL);
		(void) sigaction(SIGQUIT, &act, NULL);
		(void) sigaction(SIGTERM, &act, NULL);
/*
-- crf: problems with pclose ...
*/
		signal (SIGPIPE, SIG_IGN) ;
	}
#endif
	i = mkstemp(lettmp);
	debugf ("lettmp = %s", lettmp) ;
#ifndef __HELIOS
	tmpf = fdopen(i, "r+");
#else
	tmpf = fdopen(i, "rb+");
#endif	
	if (i < 0 || tmpf == NULL)
		panic("%s: cannot open for writing", lettmp);
	/*
	 * This protects against others reading mail from temp file and
	 * if we exit, the file will be deleted already.
	 */
#ifndef __HELIOS
	unlink(lettmp);
#else
	debugf ("NOT unlinking ...") ;
#endif	

	if (argv[0][0] == 'r')
#ifndef __HELIOS
		rmail++;
#else
		debugf ("no rmail ...") ;
#endif
	if (argv[0][0] != 'r' &&	/* no favors for rmail*/
#ifndef __HELIOS
	   (argc == 1 || argv[1][0] == '-' && !any(argv[1][1], "rhd")))
#else
	   (argc == 1 || argv[1][0] == '-' && !any(argv[1][1], "rhdv")))
#endif

#ifndef __HELIOS
		printmail(argc, argv);
#else
		{
			printmail(argc, argv);
			(void) fclose (tmpf) ;
		}
#endif
	else
		bulkmail(argc, argv);
	done();
}

void setsig(i, f)
int i;
#ifndef __HELIOS
int (*f)();
#else
void (*f)(int);
#endif
{
	if (signal(i, SIG_IGN) != SIG_IGN)
		signal(i, f);
}

int any(c, str)
	register int c;
	register char *str;
{

	while (*str)
		if (c == *str++)
			return(1);
	return(0);
}

void printmail(int argc, char **argv)
{
	int flg, i, j, print;
#ifndef __HELIOS
	char *p, *getarg();
	struct stat statb;
#else
	char *p, *getarg(char *, char *);
	void copymt(FILE *, FILE *) ;
	void copyback(void) ;
#endif
#if defined(MH_LCK_BEL)
	int lk_fd;
	extern int lkopen(), lkclose();
#endif /* MH_LCK_BEL */

	setuid(getuid());
	cat(mailfile, maildir, my_name);
#ifdef notdef
	if (stat(mailfile, &statb) >= 0
	    && (statb.st_mode & S_IFMT) == S_IFDIR) {
		strcat(mailfile, "/");
		strcat(mailfile, my_name);
	}
#endif
	for (; argc > 1; argv++, argc--) {
		if (argv[1][0] != '-')
			break;
		switch (argv[1][1]) {

		case 'p':
			flgp++;
			/* fall thru... */
#ifndef __HELIOS
		case 'q':
			delflg = 0;
			break;
#endif

		case 'f':
			if (argc >= 3) {
				strcpy(mailfile, argv[2]);
				argv++, argc--;
			}
			break;

		case 'b':
			forward = 1;
			break;

		default:
			panic("unknown option %c", argv[1][1]);
			/*NOTREACHED*/
		}
	}
#if defined(LOCKF_FLOCK)
       /*
	* We can't get a lock on a file that
	* isn't opened for writing - sigh!
	*/
	malf = fopen(mailfile, "r+");
#else
#ifndef __HELIOS
	malf = fopen(mailfile, "r");
#else
	malf = fopen(mailfile, "rb");
#endif	
#endif /* LOCKF_FLOCK */
	if (malf == NULL) {
		printf("No mail.\n");
		return;
	}
#if defined(MH_LCK_BEL)
	lk_fd = lkopen(mailfile, O_RDONLY);
#endif /* MH_LCK_BEL */
#ifndef __HELIOS
	flock(fileno(malf), LOCK_SH);
#else
	{
		int malf_fd = fileno (malf) ;
		if (f_lock(malf_fd, LOCK_SH))
			panic ("can't lock %s", malf) ;
#endif	
		copymt(malf, tmpf);
		fclose(malf);			/* implicit unlock */
#ifdef __HELIOS
		if (f_lock (malf_fd, LOCK_UN))
			debugf ("can't unlock") ;
	}
#endif	
#if defined(MH_LCK_BEL)
	(void) lkclose(lk_fd, mailfile);
#endif /* MH_LCK_BEL */
	fseek(tmpf, 0L, L_SET);

	changed = 0;
	print = 1;
	for (i = 0; i < nlet; ) {
		j = forward ? i : nlet - i - 1;
		if (setjmp(sjbuf)) {
			print = 0;
		} else {
			if (print)
#ifndef __HELIOS
				copylet(j, stdout, ORDINARY);
#else
			{
				Attributes attr;
				FILE *p_str = popen (_PATH_MORE, "w") ;
				if (p_str == (FILE *) NULL)
					panic ("failed to popen %s - errno = %d", _PATH_MORE, errno) ;
				copylet(j, p_str, ORDINARY);
				(void) pclose (p_str) ;
/*
-- crf: I have to reset the attributes 'cos more (currently) does not
*/
				if (GetAttributes (fdstream (fileno (stdin)), &attr) < 0)
					panic ("failed to GetAttributes - errno = %d", errno) ;
				RemoveAttribute (&attr, ConsoleRawInput);
				AddAttribute (&attr, ConsoleEcho);
				if (SetAttributes (fdstream (fileno (stdin)), &attr) < 0)
					panic ("failed to SetAttributes - errno = %d", errno) ;
			}
#endif
			print = 1;
		}
		if (flgp) {
			i++;
			continue;
		}
		setjmp(sjbuf);
		fputs("? ", stdout);
		fflush(stdout);
		if (fgets(resp, LSIZE, stdin) == NULL)
			break;
		switch (resp[0]) {

		default:
			printf("usage\n");
		case '?':
			print = 0;
			printf("q\tquit\n");
			printf("x\texit without changing mail\n");
			printf("p\tprint\n");
			printf("s[file]\tsave (default mbox)\n");
			printf("w[file]\tsame without header\n");
			printf("-\tprint previous\n");
			printf("d\tdelete\n");
			printf("+\tnext (no delete)\n");
			printf("m user\tmail to user\n");
			printf("! cmd\texecute cmd\n");
			break;

		case '+':
		case 'n':
		case '\n':
			i++;
			break;
		case 'x':
			changed = 0;
		case 'q':
			goto donep;
		case 'p':
			break;
		case '^':
		case '-':
			if (--i < 0)
				i = 0;
			break;
		case 'y':
		case 'w':
		case 's':
			flg = 0;
			if (resp[1] != '\n' && resp[1] != ' ') {
				printf("illegal\n");
				flg++;
				print = 0;
				continue;
			}
			if (resp[1] == '\n' || resp[1] == '\0') {
				p = getenv("HOME");
				if (p != 0)
					cat(resp+1, p, "/mbox");
				else
					cat(resp+1, "", "mbox");
			}
			for (p = resp+1; (p = getarg(lfil, p)) != NULL; ) {
#ifndef __HELIOS
				malf = fopen(lfil, "a");
#else
				malf = fopen(lfil, "ab");
#endif				
				if (malf == NULL) {
					printf("mail: %s: cannot append\n",
						lfil);
					flg++;
					continue;
				}
				copylet(j, malf, resp[0]=='w'? ZAP: ORDINARY);
				debugf ("need to unlock ?") ;
				fclose(malf);
			}
			if (flg)
				print = 0;
			else {
				let[j].change = 'd';
				changed++;
				i++;
			}
			break;
		case 'm':
			flg = 0;
			if (resp[1] == '\n' || resp[1] == '\0') {
				i++;
				continue;
			}
			if (resp[1] != ' ') {
				printf("invalid command\n");
				flg++;
				print = 0;
				continue;
			}
			for (p = resp+1; (p = getarg(lfil, p)) != NULL; )
#ifndef __HELIOS
				if (!sendmail(j, lfil, my_name))
#else
				if (sendmail(j, lfil, my_name))
#endif
					flg++;
			if (flg)
				print = 0;
			else {
				let[j].change = 'd';
				changed++;
				i++;
			}
			break;
		case '!':
			system(resp+1);
			printf("!\n");
			print = 0;
			break;
		case 'd':
			let[j].change = 'd';
			changed++;
			i++;
			if (resp[1] == 'q')
				goto donep;
			break;
		}
	}
	donep:
	if (changed)
		copyback();
}

/* copy temp or whatever back to /usr/spool/mail */
void copyback()
{
	register int i, c;
	long oldmask;
	int fd, New = 0;
	struct stat stbuf;
#if defined(MH_LCK_BEL)
	int lk_fd;
	extern int lkopen(), lkclose();
#endif /* MH_LCK_BEL */

	oldmask = sigblock(sigmask(SIGINT)|sigmask(SIGHUP)|sigmask(SIGQUIT));
	fd = open(mailfile, O_RDWR | O_CREAT, MAILMODE);
	if (fd >= 0) {
#if defined(MH_LCK_BEL)
		lk_fd = lkopen(mailfile, O_RDWR);
#endif /* MH_LCK_BEL */
#ifndef __HELIOS
		flock(fd, LOCK_EX);
#else
		if (f_lock(fd, LOCK_EX))
			panic ("can't lock %s", mailfile) ;
#endif
#ifndef __HELIOS
		malf = fdopen(fd, "r+");
#else
		malf = fdopen(fd, "rb+");
#endif
	}
	if (fd < 0 || malf == NULL)
#ifndef __HELIOS
		panic("can't rewrite %s", lfil);
#else
	{
		if (f_lock (fd, LOCK_UN))
			debugf ("can't unlock") ;
		panic("can't rewrite %s", lfil);
	}
#endif			

	fstat(fd, &stbuf);
	if (stbuf.st_size != let[nlet].adr) {	/* new mail has arrived */
		fseek(malf, let[nlet].adr, L_SET);
		fseek(tmpf, let[nlet].adr, L_SET);
		while ((c = getc(malf)) != EOF)
			putc(c, tmpf);
		let[++nlet].adr = stbuf.st_size;
		New = 1;
		fseek(malf, 0L, L_SET);
	}
#ifndef __HELIOS
	ftruncate(fd, 0L);
#else
	debugf ("alternative ftruncate()") ;
	if (fclose (malf) != 0)
		debugf ("failed to close mail file") ;
	(void) close (fd) ;
	fd = open(mailfile, O_RDWR | O_TRUNC, MAILMODE);
	if (fd >= 0) {
		malf = fdopen(fd, "rb+");
	}
	if (fd < 0 || malf == NULL)
	{
		if (f_lock (fd, LOCK_UN))
			debugf ("can't unlock") ;
		panic("can't rewrite %s", lfil);
	}
#endif
	for (i = 0; i < nlet; i++)
		if (let[i].change != 'd')
			copylet(i, malf, ORDINARY);
	fclose(malf);		/* implict unlock */
#ifdef __HELIOS
	if (f_lock (fd, LOCK_UN))
		debugf ("can't unlock") ;
#endif	
#if defined(MH_LCK_BEL)
	(void) lkclose(lk_fd, mailfile);
#endif /* MH_LCK_BEL */
	if (New)
		printf("New mail has arrived.\n");
	sigsetmask((int)oldmask);
}

/* copy mail (f1) to temp (f2) */
void copymt(f1, f2)
	FILE *f1, *f2;
{
	long nextadr;

	nlet = (int)(nextadr = 0);
	let[0].adr = 0;
	while (fgets(line, LSIZE, f1) != NULL) {
		if (isfrom(line))
			let[nlet++].adr = nextadr;
		nextadr += strlen(line);
		fputs(line, f2);
	}
	let[nlet].adr = nextadr;	/* last plus 1 */
}

void copylet(int n, FILE *f, int type)
{
	int ch;
	long k;
	char hostname[MAXHOSTNAMELEN];
#ifdef __HELIOS
	extern int gethostname(char *, int);
#endif

	fseek(tmpf, let[n].adr, L_SET);
	k = let[n+1].adr - let[n].adr;
	while (k-- > 1 && (ch = getc(tmpf)) != '\n')
		if (type != ZAP)
			putc(ch, f);
	switch (type) {

	case REMOTE:
		gethostname(hostname, sizeof (hostname));
		fprintf(f, " remote from %s\n", hostname);
		break;

#ifndef __HELIOS
	case FORWARD:
#else
	case FOR_WARD:
#endif
		fprintf(f, forwmsg);
		break;

	case ORDINARY:
		putc(ch, f);
		break;

	case ZAP:
		break;

	default:
		panic("Bad letter type %d to copylet.", type);
	}
	while (k-- > 1) {
		ch = getc(tmpf);
		putc(ch, f);
	}
	if (type != ZAP || ch != '\n')
		putc(getc(tmpf), f);
}

int isfrom(lp)
register char *lp;
{
	register char *p;

	for (p = from; *p; )
		if (*lp++ != *p++)
			return(0);
	return(1);
}

void bulkmail(int argc, char **argv)
{
	char *truename;
	int first;
	register char *cp;
	char *newargv[1000];
	register char **ap;
	register char **vp;
	int dflag;
#ifdef __HELIOS
	int vflag = 0 ;
	void usage(void) ;
	extern char Version[] ;
#endif

	dflag = 0;
#ifndef __HELIOS
	delflg = 0;
#endif
	if (argc < 1) {
		fprintf(stderr, "puke\n");
		return;
	}
	for (vp = argv, ap = newargv + 1; (*ap = *vp++) != 0; ap++)
		if (ap[0][0] == '-' && ap[0][1] == 'd')
			dflag++;
#ifdef __HELIOS
		else
		if (ap[0][0] == '-' && ap[0][1] == 'v')
			vflag++;
#endif

	if (!dflag) {
		/* give it to sendmail, rah rah! */
#ifdef __HELIOS 
		(void) fclose (tmpf) ;
#endif
		unlink(lettmp);
		ap = newargv+1;
#ifndef __HELIOS
		if (rmail)
			*ap-- = "-s";
#else
		if (vflag)
		{
			*ap-- = "-v";
			printf ("mail %s\n", Version) ;
		}
#endif
		*ap = "-sendmail";
		setuid(getuid());
#ifndef __HELIOS
		execv(_PATH_SENDMAIL, ap);
		perror(_PATH_SENDMAIL);
		exit(EX_UNAVAILABLE);
#else
		{
			int pid ;
			int status ;
			if ((pid = vfork()) < 0)
			{
				syslog (LOG_ERR, "cannot vfork: %m") ;
			}
			if (!pid)
			{
				if (execve (_PATH_SENDMAIL, ap, environ) < 0)
				{
					syslog (LOG_ERR,
						"%s: %m", _PATH_SENDMAIL) ;
					printf ("Error: failed to execve %s - errno = %d\n", 
						_PATH_SENDMAIL, errno) ;
				}
				_exit(0) ;
			}
			waitpid (pid, &status, 0) ;
			if (status)
			{
#if 0
				syslog (LOG_ERR, "%s: %m", _PATH_SENDMAIL);
				perror(_PATH_SENDMAIL);
#endif
/*
-- crf: assuming that sendmail will display all the relevent errors ...
*/
				exit(EX_UNAVAILABLE);
			}
			else
				done() ;
		}
#endif
	}

	truename = 0;
	line[0] = '\0';

	/*
	 * When we fall out of this, argv[1] should be first name,
	 * argc should be number of names + 1.
	 */

	while (argc > 1 && *argv[1] == '-') {
		cp = *++argv;
		argc--;
		switch (cp[1]) {
		case 'r':
			if (argc <= 1)
				usage();
			truename = argv[1];
			fgets(line, LSIZE, stdin);
			if (strncmp("From", line, 4) == 0)
				line[0] = '\0';
			argv++;
			argc--;
			break;

		case 'h':
			if (argc <= 1)
				usage();
			hseqno = atoi(argv[1]);
			argv++;
			argc--;
			break;

		case 'd':
			break;
		
		default:
			usage();
		}
	}
	if (argc <= 1)
		usage();
	if (truename == 0)
		truename = my_name;
	time(&iop);
	fprintf(tmpf, "%s%s %s", from, truename, ctime(&iop));
	iop = (int) ftell(tmpf);
	flgf = first = 1;
	for (;;) {
		if (first) {
			first = 0;
			if (*line == '\0' && fgets(line, LSIZE, stdin) == NULL)
				break;
		} else {
			if (fgets(line, LSIZE, stdin) == NULL)
				break;
		}
		if (*line == '.' && line[1] == '\n' && isatty(fileno(stdin)))
			break;
#ifndef __HELIOS
		if (isfrom(line))
			putc('>', tmpf);
#endif
#ifdef __HELIOS
/*
-- crf: not nice ...
*/
		{
			char *crlf = line + strlen (line) - 2 ;
			if (!strcmp (crlf, "\r\n"))
				strcpy (crlf, "\n") ;
		}
#endif
		fputs(line, tmpf);
		flgf = 0;
	}
	putc('\n', tmpf);
	nlet = 1;
	let[0].adr = 0;
	let[1].adr = ftell(tmpf);
	if (flgf)
		return;
	while (--argc > 0)
#ifndef __HELIOS
		if (!sendmail(0, *++argv, truename))
			error++;
#else
		if ((error = sendmail(0, *++argv, truename)) != EX_OK)
#endif
#ifdef __HELIOS
		{
			char *p = getenv("HOME");
			if (p != 0)
				sprintf (dead, "%s/%s", p, DEAD_LETTER) ;
			else
			{
				if (getcwd (dead, sizeof (dead)) == (char *) NULL)
					strcpy (dead, DEAD_LETTER);
				else
					cat (dead + strlen (dead), "/", DEAD_LETTER) ;
			}
		}
#endif
	if (error && safefile(dead)) {
		setuid(getuid());
		malf = fopen(dead, "w");
		if (malf == NULL) {
			printf("mail: cannot open %s\n", dead);
			debugf ("need to unlock ?") ;
			fclose(tmpf);
			return;
		}
		copylet(0, malf, ZAP);
		fclose(malf);
		printf("Mail saved in %s\n", dead);
	}
	debugf ("need to unlock ?") ;
	fclose(tmpf);
}

#ifndef __HELIOS
int sendrmt(int n, char *name)
{
	FILE *rmf, *popen();
	register char *p;
	char rsys[64], cmd[64];
	register pid;
	int sts;

#ifdef notdef
	if (any('^', name)) {
		while (p = index(name, '^'))
			*p = '!';
		if (strncmp(name, "researc", 7)) {
			strcpy(rsys, "research");
			if (*name != '!')
				--name;
			goto skip;
		}
	}
#endif
	for (p=rsys; *name!='!'; *p++ = *name++)
		if (*name=='\0')
			return(0);	/* local address, no '!' */
	*p = '\0';
	if (name[1]=='\0') {
		printf("null name\n");
		return(0);
	}
skip:
	if ((pid = fork()) == -1) {
		fprintf(stderr, "mail: can't create proc for remote\n");
		return(0);
	}
	if (pid) {
		while (wait(&sts) != pid) {
			if (wait(&sts)==-1)
				return(0);
		}
		return(!sts);
	}
	setuid(getuid());
	if (any('!', name+1))
		(void)sprintf(cmd, "uux - %s!rmail \\(%s\\)", rsys, name+1);
	else
		(void)sprintf(cmd, "uux - %s!rmail %s", rsys, name+1);
	if ((rmf=popen(cmd, "w")) == NULL)
		exit(1);
	copylet(n, rmf, REMOTE);
	exit(pclose(rmf) != 0);
}
#endif

void usage()
{

	fprintf(stderr, "Usage: mail [ -f ] people . . .\n");
	error = EX_USAGE;
	done();
}

#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

void notifybiff(msg)
	char *msg;
{
	static struct sockaddr_in addr;
	static int f = -1;

	if (addr.sin_family == 0) {
		struct hostent *hp = gethostbyname("localhost");
		struct servent *sp = getservbyname("biff", "udp");

		if (hp && sp) {
			addr.sin_family = hp->h_addrtype;
#ifndef __HELIOS
			bcopy(hp->h_addr, &addr.sin_addr, hp->h_length);
#else
			bcopy(hp->h_addr, (char *) &addr.sin_addr, hp->h_length);
#endif
			addr.sin_port = sp->s_port;
		}
	}
	if (addr.sin_family) {
		if (f < 0)
			f = socket(AF_INET, SOCK_DGRAM, 0);
		if (f >= 0)
#ifndef __HELIOS
			sendto(f, msg, strlen(msg)+1, 0, &addr, sizeof (addr));
#else
			sendto(f, msg, strlen(msg)+1, 0, 
			(struct sockaddr *) &addr, sizeof (addr));
#endif
	}
}

int sendmail(n, name, fromaddr)
	int n;
	char *name, *fromaddr;
{
	char file[256];
#ifndef __HELIOS
	int mask, fd;
#else
	int fd;
#endif
	struct passwd *pw;
#ifdef notdef
	struct stat statb;
#endif
	char buf[128];
#if defined(MH_LCK_BEL)
	int lk_fd;
	extern int lkopen(), lkclose();
#endif /* MH_LCK_BEL */

#ifndef __HELIOS
	if (*name=='!')
		name++;
	if (any('!', name))
		return (sendrmt(n, name));
#else
	debugf ("not testing for '!'") ;
#endif
	if ((pw = getpwnam(name)) == NULL) {
		printf("mail: can't send to %s\n", name);
#ifndef __HELIOS
		return(0);
#else
		return(EX_NOUSER);
#endif
	}
	cat(file, maildir, name);
#ifdef notdef
	if (stat(file, &statb) >= 0 && (statb.st_mode & S_IFMT) == S_IFDIR) {
		strcat(file, "/");
		strcat(file, name);
	}
#endif
	if (!safefile(file))
#ifndef __HELIOS
		return(0);
#else
		return(EX_SOFTWARE);
#endif
	fd = open(file, O_WRONLY | O_CREAT, MAILMODE);
	if (fd >= 0) {
#if defined(MH_LCK_BEL)
		lk_fd = lkopen(mailfile, O_RDWR);
#endif /* MH_LCK_BEL */
#ifndef __HELIOS
		flock(fd, LOCK_EX);
#else
		if (f_lock(fd, LOCK_EX))
			panic ("can't lock %s", file) ;
#endif
#ifndef __HELIOS
		malf = fdopen(fd, "a");
#else
		malf = fdopen(fd, "ab");
#endif
	}
	if (fd < 0 || malf == NULL) {
#ifdef __HELIOS
		if (f_lock (fd, LOCK_UN))
			debugf ("can't unlock") ;
#endif
		close(fd);
		printf("mail: %s: cannot append\n", file);
#ifndef __HELIOS
		return(0);
#else
		return(EX_CANTCREAT);
#endif
	}
#ifndef __HELIOS
	fchown(fd, pw->pw_uid, pw->pw_gid);
#else
	debugf ("binmail: fchown()") ;
#endif	
	(void)sprintf(buf, "%s@%ld\n", name, ftell(malf));
	copylet(n, malf, ORDINARY);
	fclose(malf);
#ifdef __HELIOS
	if (f_lock (fd, LOCK_UN))
		debugf ("can't unlock") ;
#endif	
#if defined(MH_LCK_BEL)
	(void) lkclose(lk_fd, mailfile);
#endif /* MH_LCK_BEL */

	notifybiff(buf);
#ifndef __HELIOS
	return(1);
#else
	return(EX_OK);
#endif
}

void delex(int i)
{
#ifdef __HELIOS
	extern void f_lock_exit (void) ;
	f_lock_exit() ;
#endif
	if (i != SIGINT) {
		setsig(i, SIG_DFL);
		sigsetmask(sigblock(0) &~ sigmask(i));
	}
#ifndef __HELIOS
	putc('\n', stderr);
	if (delflg)
		longjmp(sjbuf, 1);
#endif
	if (error == 0)
		error = i;
	done();
}

void done()
{
	unlink(lettmp);
#ifdef MEM_CHECK
	IOdebug ("exiting binmail: Bytes free : %d  Heap size : %d", 
		 Malloc(-1), Malloc(-3));
#endif
	exit(error);
}

void cat(to, from1, from2)
	char *to, *from1, *from2;
{
	register char *cp, *dp;

	cp = to;
	for (dp = from1; (*cp = *dp++) != 0; cp++)
		;
	for (dp = from2; (*cp++ = *dp++) != 0; )
		;
}

/* copy p... into s, update p */
char *
getarg(s, p)
	register char *s, *p;
{
	while (*p == ' ' || *p == '\t')
		p++;
	if (*p == '\n' || *p == '\0')
		return(NULL);
	while (*p != ' ' && *p != '\t' && *p != '\n' && *p != '\0')
		*s++ = *p++;
	*s = '\0';
	return(p);
}

int safefile(f)
	char *f;
{
	struct stat statb;

	if (lstat(f, &statb) < 0)
		return (1);
#ifndef __HELIOS		
	if (statb.st_nlink != 1 || (statb.st_mode & S_IFMT) == S_IFLNK) {
		fprintf(stderr,
			"mail: %s has more than one link or is a symbolic link\n",
			f);
		return (0);
	}
#else
	debugf ("link checking not done") ;
#endif	
	return (1);
}

#ifndef __HELIOS
panic(msg, a1, a2, a3)
	char *msg;
{
	fprintf(stderr, "mail: ");
	fprintf(stderr, msg, a1, a2, a3);
	fprintf(stderr, "\n");
	done();
}
#else
void panic (char *format, ...)
{
	va_list args;
	va_start (args, format);
	fprintf(stderr, "mail: ");
	vfprintf(stderr, format, args);
	fprintf(stderr, "\n");
	va_end (args);
	done();
}
#endif

#ifdef __HELIOS

void debugf (char *format, ...)
{
#ifdef DEBUG
	char buf [256] ;
	va_list args;
	va_start (args, format);
	vsprintf (buf, format, args);
	IOdebug ("%s: %s", name, buf) ;
	va_end (args);
#endif
}

#endif
