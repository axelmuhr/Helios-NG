/*	UNIX.C:	Operating specific I/O and Spawning functions
		under UNIX V7, BSD4.2/3, System V, SUN OS and SCO XENIX
		for MicroEMACS 3.10
		(C)opyright 1988 by Daniel M. Lawrence

*/

#include        <stdio.h>
#include	"estruct.h"
#include	"etype.h"
#if	V7 | BSD | USG | HPUX | SUN | XENIX
#include        "edef.h"
#include	"elang.h"

#if	USG | HPUX | XENIX			/* System V */
#include	<signal.h>
#include	<termio.h>
#include	<fcntl.h>
#include	<ndir.h>

int kbdflgs;			/* saved keyboard fd flags	*/
int kbdpoll;			/* in O_NDELAY mode			*/
int kbdqp;			/* there is a char in kbdq	*/
char kbdq;			/* char we've already read	*/

struct	termio	otermio;	/* original terminal characteristics */
struct	termio	ntermio;	/* charactoristics to use inside */
#endif

#if V7 | BSD | SUN
/* I hit a system name here... we have to define it back to what
   emacs expacts */
#undef	CTRL
#include        <sgtty.h>        /* for stty/gtty functions */
#include	<signal.h>
#include	<sys/types.h>
#include	<sys/dir.h>
#include	<sys/stat.h>
#include	<fcntl.h>

int kbdflgs;			/* saved keyboard fd flags	*/
int kbdpoll;			/* in O_NDELAY mode			*/
int kbdqp;			/* there is a char in kbdq	*/
char kbdq;			/* char we've already read	*/
struct  sgttyb  ostate;          /* saved tty state */
struct  sgttyb  nstate;          /* values for editor mode */
struct tchars	otchars;	/* Saved terminal special character set */
struct tchars	ntchars = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
#undef	CTRL
#define CTRL	0x0100		/* Control flag, or'ed in		*/

#if BSD
#include <sys/ioctl.h>		/* to get at the typeahead */
extern	int rtfrmshell();	/* return from suspended shell */
#define	TBUFSIZ	128
char tobuf[TBUFSIZ];		/* terminal output buffer */
#endif
#endif

#if     V7 | USG | HPUX | SUN | XENIX | BSD
#include        <signal.h>
extern int vttidy();
#endif

/*
 * This function is called once to set up the terminal device streams.
 * On VMS, it translates TT until it finds the terminal, then assigns
 * a channel to it and sets it raw. On CPM it is a no-op.
 */
ttopen()

{
#if	USG | HPUX | XENIX
	ioctl(0, TCGETA, &otermio);	/* save old settings */
	ntermio.c_iflag = 0;		/* setup new settings */
	ntermio.c_oflag = 0;
	ntermio.c_cflag = otermio.c_cflag;
	ntermio.c_lflag = 0;
	ntermio.c_line = otermio.c_line;
	ntermio.c_cc[VMIN] = 1;
	ntermio.c_cc[VTIME] = 0;
	ioctl(0, TCSETAW, &ntermio);	/* and activate them */
	kbdflgs = fcntl( 0, F_GETFL, 0 );
	kbdpoll = FALSE;
#endif

#if     V7 | BSD | SUN
        gtty(0, &ostate);                       /* save old state */
        gtty(0, &nstate);                       /* get base of new state */
        nstate.sg_flags |= RAW;
        nstate.sg_flags &= ~(ECHO|CRMOD);       /* no echo for now... */
        stty(0, &nstate);                       /* set mode */
	ioctl(0, TIOCGETC, &otchars);		/* Save old characters */
	ioctl(0, TIOCSETC, &ntchars);		/* Place new character into K */
#if	BSD
	/* provide a smaller terminal output buffer so that
	   the type ahead detection works better (more often) */
	setbuffer(stdout, &tobuf[0], TBUFSIZ); 
	signal(SIGTSTP,SIG_DFL);	/* set signals so that we can */
	signal(SIGCONT,rtfrmshell);	/* suspend & restart emacs */
#endif
#endif
	/* on all screens we are not sure of the initial position
	   of the cursor					*/
	ttrow = 999;
	ttcol = 999;
}

/*
 * This function gets called just before we go back home to the command
 * interpreter.
 */

ttclose()

{
#if	USG | HPUX | XENIX
	ioctl(0, TCSETA, &otermio);	/* restore terminal settings */
	fcntl(0, F_SETFL, kbdflgs);
#endif

#if     V7 | BSD | SUN
        stty(0, &ostate);
	ioctl(0, TIOCSETC, &otchars);	/* Place old character into K */
#endif
}

/*
 * Write a character to the display. On VMS, terminal output is buffered, and
 * we just put the characters in the big array, after checking for overflow.
 * On CPM terminal I/O unbuffered, so we just write the byte out. Ditto on
 * MS-DOS (use the very very raw console output routine).
 */
ttputc(c)
{
#if     V7 | USG | HPUX | SUN | XENIX | BSD
        fputc(c, stdout);
#endif
}

/*
 * Flush terminal buffer. Does real work where the terminal output is buffered
 * up. A no-operation on systems where byte at a time terminal I/O is done.
 */
ttflush()
{
#if     V7 | USG | HPUX | SUN | XENIX | BSD
        fflush(stdout);
#endif
}

/*	TTGETC:	Read a character from the terminal, performing no
		editing and doing no echo at all. More complex in VMS
		that almost anyplace else, which figures. Very simple
		on CPM, because the system can do exactly what you
		want.
*/

ttgetc()

{
#if     V7
	char c;

        read(0, &c, 1);
	return(255 & (int)c);
#endif

#if	USG | HPUX | XENIX | BSD
	if (kbdqp)
		kbdqp = FALSE;
	else {
		/* we desperatly seek a character so we turn off
		   the NO_DELAY flag and simply wait for the bastard*/
		if (fcntl(0, F_SETFL, kbdflgs) < 0 && kbdpoll)
			return(FALSE);
		kbdpoll = FALSE;	/* no polling */
		read(0, &kbdq, 1);	/* wait until we get a character */
	}
	return(kbdq & 255);
#endif
}

#if	TERMCAP & (USG | HPUX | XENIX)
/* get a character with timeout */
mttgetc()

{
	struct termio tset;
	int status;
	char c,c_eof,c_eol;
	int i,count;
	long trns;

	fcntl(0,F_SETFL,O_NDELAY);
	for(count=150; count != 0 ; --count) {
		i = read(0,&c,1); 		/* get a character */
		if (i == 1)			/* got a char */
			break;
	}
	fcntl(0,F_SETFL,0);
	if (i<= 0)	/* timeout error */
		return(-1);
	i = c;
	return(i & 0xff);		/* return character */
}
#endif

#if	TYPEAH
/* typahead:	Check to see if any characters are already in the
		keyboard buffer
*/

typahead()

{
#if	BSD | SUN
	int x;	/* holds # of pending chars */

	return((ioctl(0,FIONREAD,&x) < 0) ? 0 : x);
#endif

#if	USG | HPUX | XENIX
	if (!kbdqp) {
		/* set O_NDELAY */
		if (fcntl(0, F_SETFL, kbdflgs | O_NDELAY) < 0 && kbdpoll)
			return(FALSE);
		kbdpoll = TRUE;
		kbdqp = (1 == read(0, ëbdq, 1));
	}
	return(kbdqp);
#endif
#if	V7
	return(FALSE);
#endif
}
#endif

/*
 * Create a subjob with a copy of the command intrepreter in it. When the
 * command interpreter exits, mark the screen as garbage so that you do a full
 * repaint. Bound to "^X C". The message at the start in VMS puts out a newline.
 * Under some (unknown) condition, you don't get one free when DCL starts up.
 */
spawncli(f, n)
{
        register char *cp;
        char    *getenv();

	/* don't allow this command if restricted */
	if (restflag)
		return(resterr());

        movecursor(term.t_nrow, 0);             /* Seek to last line.   */
        TTflush();
        TTclose();                              /* stty to old settings */
        if ((cp = getenv("SHELL")) != NULL && *cp != '\0')
                system(cp);
        else
#if	BSD | SUN
                system("exec /bin/csh");
#else
                system("exec /bin/sh");
#endif
        sgarbf = TRUE;
	sleep(2);
        TTopen();
        return(TRUE);
}

#if	BSD | SUN

bktoshell()		/* suspend MicroEMACS and wait to wake up */
{
	int pid;

	vttidy();
	pid = getpid();
	kill(pid,SIGTSTP);
}

rtfrmshell()
{
	TTopen();
	curwp->w_flag = WFHARD;
	sgarbf = TRUE;
}
#endif

/*
 * Run a one-liner in a subjob. When the command returns, wait for a single
 * character to be typed, then mark the screen as garbage so a full repaint is
 * done. Bound to "C-X !".
 */
spawn(f, n)
{
        register int    s;
        char            line[NLINE];

	/* don't allow this command if restricted */
	if (restflag)
		return(resterr());

        if ((s=mlreply("!", line, NLINE)) != TRUE)
                return(s);
        TTputc('\n');                /* Already have '\r'    */
        TTflush();
        TTclose();                              /* stty to old modes    */
        system(line);
        TTopen();
        TTflush();
	/* if we are interactive, pause here */
	if (clexec == FALSE) {
	        mlputs(TEXT6);
/*                     "\r\n\n[End]" */
        	tgetc();
        }
        sgarbf = TRUE;
        return(TRUE);
}

/*
 * Run an external program with arguments. When it returns, wait for a single
 * character to be typed, then mark the screen as garbage so a full repaint is
 * done. Bound to "C-X $".
 */

execprg(f, n)

{
        register int    s;
        char            line[NLINE];

	/* don't allow this command if restricted */
	if (restflag)
		return(resterr());

        if ((s=mlreply("!", line, NLINE)) != TRUE)
                return(s);
        TTputc('\n');                /* Already have '\r'    */
        TTflush();
        TTclose();                              /* stty to old modes    */
        system(line);
        TTopen();
        mlputs(TEXT188);                        /* Pause.               */
/*             "[End]" */
        TTflush();
        while ((s = tgetc()) != '\r' && s != ' ')
                ;
        sgarbf = TRUE;
        return(TRUE);
}

/*
 * Pipe a one line command into a window
 * Bound to ^X @
 */
pipecmd(f, n)
{
        register int    s;	/* return status from CLI */
	register WINDOW *wp;	/* pointer to new window */
	register BUFFER *bp;	/* pointer to buffer to zot */
        char	line[NLINE];	/* command line send to shell */
	static char bname[] = "command";

	static char filnam[NFILEN] = "command";

	/* don't allow this command if restricted */
	if (restflag)
		return(resterr());

	/* get the command to pipe in */
        if ((s=mlreply("@", line, NLINE)) != TRUE)
                return(s);

	/* get rid of the command output buffer if it exists */
        if ((bp=bfind(bname, FALSE, 0)) != FALSE) {
		/* try to make sure we are off screen */
		wp = wheadp;
		while (wp != NULL) {
			if (wp->w_bufp == bp) {
				onlywind(FALSE, 1);
				break;
			}
			wp = wp->w_wndp;
		}
		if (zotbuf(bp) != TRUE)

			return(FALSE);
	}

        TTputc('\n');                /* Already have '\r'    */
        TTflush();
        TTclose();                              /* stty to old modes    */
	strcat(line,">");
	strcat(line,filnam);
        system(line);
        TTopen();
        TTflush();
        sgarbf = TRUE;
        s = TRUE;

	if (s != TRUE)
		return(s);

	/* split the current window to make room for the command output */
	if (splitwind(FALSE, 1) == FALSE)
			return(FALSE);

	/* and read the stuff in */
	if (getfile(filnam, FALSE) == FALSE)
		return(FALSE);

	/* make this window in VIEW mode, update all mode lines */
	curwp->w_bufp->b_mode |= MDVIEW;
	wp = wheadp;
	while (wp != NULL) {
		wp->w_flag |= WFMODE;
		wp = wp->w_wndp;
	}

	/* and get rid of the temporary file */
	unlink(filnam);
	return(TRUE);
}

rename(old, new)	/* change the name of a file */

char *old;	/* original file name */
char *new;	/* new file name */

{
	link(old, new);
	unlink(old);
}

/*
 * filter a buffer through an external DOS program
 * Bound to ^X #
 */
filter(f, n)

{
        register int    s;	/* return status from CLI */
	register BUFFER *bp;	/* pointer to buffer to zot */
        char line[NLINE];	/* command line send to shell */
	char tmpnam[NFILEN];	/* place to store real file name */
	static char bname1[] = "fltinp";

	static char filnam1[] = "fltinp";
	static char filnam2[] = "fltout";

	/* don't allow this command if restricted */
	if (restflag)
		return(resterr());

	if (curbp->b_mode&MDVIEW)	/* don't allow this command if	*/
		return(rdonly());	/* we are in read only mode	*/

	/* get the filter name and its args */
        if ((s=mlreply("#", line, NLINE)) != TRUE)
                return(s);

	/* setup the proper file names */
	bp = curbp;
	strcpy(tmpnam, bp->b_fname);	/* save the original name */
	strcpy(bp->b_fname, bname1);	/* set it to our new one */

	/* write it out, checking for errors */
	if (writeout(filnam1) != TRUE) {
		mlwrite(TEXT2);
/*                      "[Cannot write filter file]" */
		strcpy(bp->b_fname, tmpnam);
		return(FALSE);
	}

        TTputc('\n');                /* Already have '\r'    */
        TTflush();
        TTclose();                              /* stty to old modes    */
	strcat(line," <fltinp >fltout");
        system(line);
        TTopen();
        TTflush();
        sgarbf = TRUE;
        s = TRUE;

	/* on failure, escape gracefully */
	if (s != TRUE || (readin(filnam2,FALSE) == FALSE)) {
		mlwrite(TEXT3);
/*                      "[Execution failed]" */
		strcpy(bp->b_fname, tmpnam);
		unlink(filnam1);
		unlink(filnam2);
		return(s);
	}

	/* reset file name */
	strcpy(bp->b_fname, tmpnam);	/* restore name */
	bp->b_flag |= BFCHG;		/* flag it as changed */

	/* and get rid of the temporary file */
	unlink(filnam1);
	unlink(filnam2);
	return(TRUE);
}

/* return a system dependant string with the current time */

char *PASCAL NEAR timeset()

{
	register char *sp;	/* temp string pointer */
	char buf[16];		/* time data buffer */
	extern char *ctime();

	time(buf);
	sp = ctime(buf);
	sp[strlen(sp)-1] = 0;
	return(sp);
}

#if	COMPLET
/*	FILE Directory routines		*/

DIR *dirptr = NULL;	/* pointer to the current directory being searched */

char path[NFILEN];	/* path of file to find */
char rbuf[NFILEN];	/* return file buffer */
char *nameptr;		/* ptr past end of path in rbuf */

/*	do a wild card directory search (for file name completion) */

char *PASCAL NEAR getffile(fspec)

char *fspec;	/* pattern to match */

{
	register int index;		/* index into various strings */
	register int point;		/* index into other strings */
	register int extflag;		/* does the file have an extention? */

	/* first parse the file path off the file spec */
	strcpy(path, fspec);
	index = strlen(path) - 1;
	while (index >= 0 && (path[index] != '/' &&
				path[index] != '\\' && path[index] != ':'))
		--index;
	path[index+1] = 0;

	/* check for an extension */
	point = strlen(fspec) - 1;
	extflag = FALSE;
	while (point >= 0) {
		if (fspec[point] == '.') {
			extflag = TRUE;
			break;
		}
		point--;
	}

	/* open the directory pointer */
	if (dirptr != NULL) {
		closedir(dirptr);
		dirptr = NULL;
	}
	dirptr = opendir(path);
	if (dirptr == NULL)
		return(NULL);

	strcpy(rbuf, path);
	nameptr = &rbuf[strlen(rbuf)];

	/* and call for the first file */
	return(getnfile());
}

char *PASCAL NEAR getnfile()

{
	register struct direct *dp;	/* directory entry pointer */
	register int index;		/* index into various strings */
	struct stat fstat;

	/* and call for the next file */
nxtdir:	dp = readdir(dirptr);
	if (dp == NULL)
		return(NULL);

	/* check to make sure we skip directory entries */
	strcpy(nameptr, dp->d_name);
	stat(rbuf, &fstat);
	if (((fstat.st_mode & S_IFMT) != S_IFREG &&
            ((fstat.st_mode & S_IFMT) != S_IFDIR)))
		goto nxtdir;
	if ((fstat.st_mode & S_IFMT) == S_IFDIR)
		strcat(nameptr, "/");

	/* return the next file name! */
	return(rbuf);
}
#else
char *PASCAL NEAR getffile(fspec)

char *fspec;	/* file to match */

{
	return(NULL);
}

char *PASCAL NEAR getnfile()

{
	return(NULL);
}
#endif
#else
unixhello()
{
}
#endif
