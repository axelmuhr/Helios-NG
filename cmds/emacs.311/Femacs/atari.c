/*	ATARI.C:	Operating specific I/O and Spawning functions
			for the ATARI ST operating system (GEMDOS)
			for MicroEMACS 3.10
			(C)opyright 1988 by Daniel M. Lawrence
*/

#include        <stdio.h>
#include	"estruct.h"
#include	"etype.h"
#if	ATARI
#include        "edef.h"
#include	"elang.h"
#include	"osbind.h"
#include	"stat.h"	/* DMABUFFER is here */
#include	"errno.h"

/****	ST Internals definitions		*****/

/*	BIOS calls */

#define	BCONSTAT	1	/* return input device status */
#define	CONIN		2	/* read character from device */
#define	BCONOUT		3	/* write character to device */

/*	XBIOS calls */

#define	INITMOUS	0	/* initialize the mouse */
#define	GETREZ		4	/* get current resolution */
#define	SETSCREEN	5	/* set screen resolution */
#define	SETPALETTE	6	/* set the color pallette */
#define	SETCOLOR	7	/* set or read a color */
#define	CURSCONF	21	/* set cursor configuration */
#define	IKBDWS		25	/* intelligent keyboard send command */
#define	KBDVBASE	34	/* get keyboard table base */

/*	GEMDOS calls */

#define	EXEC		0x4b	/* Exec off a process */

#define	CON		2	/* CON: Keyboard and screen device */

/*
 * This function is called once to set up the terminal device streams.
 * On VMS, it translates TT until it finds the terminal, then assigns
 * a channel to it and sets it raw. On CPM it is a no-op.
 */
ttopen()
{
	/* on all screens we are not sure of the initial position
	   of the cursor					*/
	ttrow = 999;
	ttcol = 999;
}

/*
 * This function gets called just before we go back home to the command
 * interpreter. On VMS it puts the terminal back in a reasonable state.
 * Another no-operation on CPM.
 */
ttclose()
{
}

/*
 * Write a character to the display. On VMS, terminal output is buffered, and
 * we just put the characters in the big array, after checking for overflow.
 * On CPM terminal I/O unbuffered, so we just write the byte out. Ditto on
 * MS-DOS (use the very very raw console output routine).
 */
ttputc(c)

char c;

{
}

/*
 * Flush terminal buffer. Does real work where the terminal output is buffered
 * up. A no-operation on systems where byte at a time terminal I/O is done.
 */
ttflush()
{
}

/*
 * Read a character from the terminal, performing no editing and doing no echo
 * at all. More complex in VMS that almost anyplace else, which figures. Very
 * simple on CPM, because the system can do exactly what you want.
 */
ttgetc()
{
}

#if	TYPEAH
typahead()

{
	int rval;	/* return value from BIOS call */

	/* get the status of the console */
	rval = bios(BCONSTAT, CON);

	/* end return the results */
	if (rval == 0)
		return(FALSE);
	else
		return(TRUE);
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
	/* don't allow this command if restricted */
	if (restflag)
		return(resterr());

#if     MWC
	mlerase();	/* clear the message line */
        TTflush();
	TTkclose();
	system("msh.prg");
	TTkopen();
        sgarbf = TRUE;
        return(TRUE);
#endif
}

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

#if     MWC
        if ((s=mlreply("!", line, NLINE)) != TRUE)
                return(s);
	mlerase();
	TTkclose();
        system(line);
	TTkopen();
	/* if we are interactive, pause here */
	if (clexec == FALSE) {
	        mlputs(TEXT6);
/*                     "\r\n\n[End]" */
        	tgetc();
        }
        sgarbf = TRUE;
        return (TRUE);
#endif
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

#if     MWC
        if ((s=mlreply("!", line, NLINE)) != TRUE)
                return(s);
	mlerase();
	TTkclose();
        system(line);
	TTkopen();
	/* if we are interactive, pause here */
	if (clexec == FALSE) {
	        mlputs(TEXT6);
/*                     "\r\n\n[End]" */
        	tgetc();
        }
        sgarbf = TRUE;
        return (TRUE);
#endif
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
	FILE *fp;

	static char filnam[NSTRING] = "command";

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

#if     MWC
	strcat(line," >>");
	strcat(line,filnam);
	movecursor(term.t_nrow - 1, 0);
	TTkclose();
	system(line);
	TTkopen();
        sgarbf = TRUE;
	if ((fp = fopen(filnam, "r")) == NULL) {
		s = FALSE;
	} else {
		fclose(fp);
		s = TRUE;
	}
#endif

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

#if     MWC
	strcat(line," <fltinp >fltout");
	movecursor(term.t_nrow - 1, 0);
	TTkclose();
	system(line);
	TTkopen();
        sgarbf = TRUE;
	s = TRUE;
#endif

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

rename(oldname, newname)	/* rename a file */

char *oldname;		/* original file name */
char *newname;		/* new file name */

{
	Frename(0, oldname, newname);
}

/* return a system dependant string with the current time */

char *PASCAL NEAR timeset()

{
	register char *sp;	/* temp string pointer */
	char buf[16];		/* time data buffer */

	time(buf);
	sp = ctime(buf);
	sp[strlen(sp)-1] = 0;
	return(sp);
}

/*	extcode:	resolve MSDOS extended character codes
			encoding the proper sequences into emacs
			printable character specifications

			Well, yes... The ATARI ST was supposed to be
			a PC clone originally... These codes are the
			same for the ST.
*/

int extcode(c)

unsigned c;	/* byte following a zero extended char byte */

{
	/* function keys 1 through 9 */
	if (c >= 59 && c < 68)
		return(SPEC | c - 58 + '0');

	/* function key 10 */
	if (c == 68)
		return(SPEC | '0');

	/* shifted function keys */
	if (c >= 84 && c < 93)
		return(SPEC | SHFT | c - 83 + '0');
	if (c == 93)
		return(SPEC | SHFT | '0');

	/* control function keys */
	if (c >= 94 && c < 103)
		return(SPEC | CTRL | c - 93 + '0');
	if (c == 103)
		return(SPEC | CTRL | '0');

	/* ALTed function keys */
	if (c >= 104 && c < 113)
		return(SPEC | ALTD | c - 103 + '0');
	if (c == 113)
		return(SPEC | ALTD | '0');

	/* ALTed number keys */
	if (c >= 120 && c < 129)
		return(ALTD | c - 119 + '0');
	if (c == 130)
		return(ALTD | '0');

	/* some others as well */
	switch (c) {
		case 3:		return(0);		/* null */
		case 15:	return(SHFT | CTRL | 'I');	/* backtab */

		case 16:	return(ALTD | 'Q');
		case 17:	return(ALTD | 'W');
		case 18:	return(ALTD | 'E');
		case 19:	return(ALTD | 'R');
		case 20:	return(ALTD | 'T');
		case 21:	return(ALTD | 'Y');
		case 22:	return(ALTD | 'U');
		case 23:	return(ALTD | 'I');
		case 24:	return(ALTD | 'O');
		case 25:	return(ALTD | 'P');

		case 30:	return(ALTD | 'A');
		case 31:	return(ALTD | 'S');
		case 32:	return(ALTD | 'D');
		case 33:	return(ALTD | 'F');
		case 34:	return(ALTD | 'G');
		case 35:	return(ALTD | 'H');
		case 36:	return(ALTD | 'J');
		case 37:	return(ALTD | 'K');
		case 38:	return(ALTD | 'L');

		case 44:	return(ALTD | 'Z');
		case 45:	return(ALTD | 'X');
		case 46:	return(ALTD | 'C');
		case 47:	return(ALTD | 'V');
		case 48:	return(ALTD | 'B');
		case 49:	return(ALTD | 'N');
		case 50:	return(ALTD | 'M');

		case 71:	return(SPEC | '<');	/* HOME */
		case 72:	return(SPEC | 'P');	/* cursor up */
		case 73:	return(SPEC | 'Z');	/* page up */
		case 75:	return(SPEC | 'B');	/* cursor left */
		case 77:	return(SPEC | 'F');	/* cursor right */
		case 79:	return(SPEC | '>');	/* end */
		case 80:	return(SPEC | 'N');	/* cursor down */
		case 81:	return(SPEC | 'V');	/* page down */
		case 82:	return(SPEC | 'C');	/* insert */
		case 83:	return(SPEC | 'D');	/* delete */
		case 115:	return(SPEC | CTRL | 'B');	/* control left */
		case 116:	return(SPEC | CTRL | 'F');	/* control right */
		case 117:	return(SPEC | CTRL | '>');	/* control END */
		case 118:	return(SPEC | CTRL | 'V');	/* control page down */
		case 119:	return(SPEC | CTRL | '<');	/* control HOME */
		case 132:	return(SPEC | CTRL | 'Z');	/* control page up */
	}

	return(ALTD | c);
}

/*	FILE Directory routines		*/

static DMABUFFER info;		/* Info about the file */
char path[NFILEN];		/* path of file to find */
char rbuf[NFILEN];		/* return file buffer */

/*	do a wild card directory search (for file name completion) */

char *PASCAL NEAR getffile(fspec)

char *fspec;	/* file to match */

{
	register int index;		/* index into various strings */
	register int point;		/* index into other strings */
	register int extflag;		/* does the file have an extention? */
	char fname[NFILEN];		/* file/path for DOS call */

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

	/* construct the composite wild card spec */
	strcpy(fname, path);
	strcat(fname, &fspec[index+1]);
	strcat(fname, "*");
	if (extflag == FALSE)
		strcat(fname, ".*");

	/* and call for the first file */

	Fsetdta(&info);		/* Initialize buffer for our search */
	if (Fsfirst(fname, 0xF7) != AE_OK)
		return(NULL);

	/* return the first file name! */
	strcpy(rbuf, path);
	strcat(rbuf, info.d_fname);
	mklower(rbuf);
	return(rbuf);
}

char *PASCAL NEAR getnfile()

{

	/* and call for the first file */
	if (Fsnext() != AE_OK)
		return(NULL);

	/* return the first file name! */
	strcpy(rbuf, path);
	strcat(rbuf, info.d_fname);
	mklower(rbuf);
	return(rbuf);
}
#else
atarihello()
{
}
#endif
