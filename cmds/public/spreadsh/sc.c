/*	SC	A Spreadsheet Calculator
 *		Main driver
 *
 *		original by James Gosling, September 1982
 *		modifications by Mark Weiser and Bruce Israel,
 *			University of Maryland
 *
 *              More mods Robert Bond, 12/86
 *		More mods by Alan Silverstein, 3-4/88, see list of changes.
 *
 */

#include <stdlib.h>
#include <signal.h>
#include <curses.h>
#include <posix.h>
#include <sys/wait.h>

#ifdef BSD42
#include <strings.h>
#else
#ifndef SYSIII
#include <string.h>
#endif
#endif

#include <stdio.h>
#include "sc.h"

    

#ifdef SYSV3
void exit();
#endif

/*
 * CODE REVISION NUMBER:
 *
 * The part after the first colon, except the last char, appears on the screen.
 */

char *rev = "$Revision: 1.4 $";

#ifndef DFLT_PAGER
#define	DFLT_PAGER "more"	/* more is probably more widespread than less */
#endif /* DFLT_PAGER */

#define MAXCMD 160	/* for ! command below */

/* Globals defined in sc.h */

struct ent *tbl[MAXROWS][MAXCOLS];
int strow, stcol;
int currow, curcol;
int savedrow, savedcol;
int FullUpdate;
int maxrow, maxcol;
int fwidth[MAXCOLS];
int precision[MAXCOLS];
char col_hidden[MAXCOLS];
char row_hidden[MAXROWS];
char line[1000];
int changed;
struct ent *to_fix;
int modflg;
int numeric;
char *mdir;
int showsc, showsr;	/* Starting cell for highlighted range */

char curfile[1024];
char    revmsg[80];

int  linelim = -1;

int  showtop   = 1;	/* Causes current cell value display in top line  */
int  showcell  = 1;	/* Causes current cell to be highlighted	  */
int  showrange = 0;	/* Causes ranges to be highlighted		  */
int  showneed  = 0;	/* Causes cells needing values to be highlighted  */
int  showexpr  = 0;	/* Causes cell exprs to be displayed, highlighted */

int  autocalc = 1 ;	/* 1 to calculate after each update */
int  calc_order = BYROWS;
int  tbl_style = 0;	/* headers for T command output */

int  lastmx, lastmy;	/* Screen address of the cursor */
int  lastcol;		/* Spreadsheet Column the cursor was in last */
char *under_cursor = " "; /* Data under the < cursor */

#ifdef VMS
int VMS_read_raw = 0;
#endif

int seenerr;

void
yyerror (err)
char *err; {
    if (seenerr) return;
    seenerr++;
    (void) move (1,0);
    (void) clrtoeol ();
    (void) printw ("%s: %.*s<=%s",err,linelim,line,line+linelim);
}

struct ent *
lookat(int row, int col)
{
    register struct ent **p;
    if (row < 0)
	row = 0;
    else if (row > MAXROWS-1) 
	row = MAXROWS-1;
    if (col < 0) 
	col = 0;
    else if (col > MAXCOLS-1)
	col = MAXCOLS-1;
    p = &tbl[row][col];
    if (*p==0) {
	*p = (struct ent *) xmalloc ((unsigned)sizeof (struct ent));
	if (row>maxrow) maxrow = row;
	if (col>maxcol) maxcol = col;
	(*p)->label = 0;
	(*p)->flags = 0;
	(*p)->row = row;
	(*p)->col = col;
	(*p)->expr = 0;
	(*p)->v = (double) 0.0;
    }
    return *p;
}

/*
 * This structure is used to keep ent structs around before they
 * are deleted to allow the sync_refs routine a chance to fix the
 * variable references.
 * We also use it as a last-deleted buffer for the 'p' command.
 */

void
free_ent(register struct ent *p)
{
    p->next = to_fix;
    to_fix = p;
    p->flags |= is_deleted;
}

void
flush_saved()
{
    register struct ent *p;
    register struct ent *q;
    
    if ((p = to_fix) == NULL)
	return;
    while (p) {
	(void) clearent(p);
	q = p->next;
	xfree((char *)p);
	p = q;
    }
    to_fix = 0;
}

void
repaint(x, y, len)
int x, y, len;
{
    char *buf;

    buf = " ";

    while(len-- > 0) {
	(void) move(y,x);
#ifndef INTERNATIONAL
	*buf = inch() & 0x7f;
#else
	*buf = inch() & 0xff;
#endif /* INTERNATIONAL */
	(void) addstr(buf);
	x++;
    }
}


void
update ()
{
    register    row,
                col;
    register struct ent **p;
    int     mxcol;
    int     mxrow;
    int     rows;
    int     cols;
    int     minsr, minsc, maxsr, maxsc;
    register r;
    register i;

    while (row_hidden[currow])   /* You can't hide the last row or col */
	currow++;
    while (col_hidden[curcol])
	curcol++;
    /* First see if the last display still covers curcol */
    if (stcol <= curcol) { 
	for (i = stcol, cols = 0, col = RESCOL;
			(col + fwidth[i]) < COLS-1 && i < MAXCOLS; i++) {
	    cols++;
	    if (col_hidden[i])
		continue;
	    col += fwidth[i];
	}
    }
    while (stcol + cols - 1 < curcol || curcol < stcol) {
	FullUpdate++;
	if (stcol - 1 == curcol) {    /* How about back one? */
	    stcol--;
	} else if (stcol + cols == curcol) {   /* Forward one? */
	    stcol++;
	} else {
	    /* Try to put the cursor in the center of the screen */
	    col = (COLS - RESCOL - fwidth[curcol]) / 2 + RESCOL; 
	    stcol = curcol;
	    for (i=curcol-1; i >= 0 && col-fwidth[i] > RESCOL; i--) {
		stcol--;
		if (col_hidden[i])
		    continue;
		col -= fwidth[i];
	    }
	}
	/* Now pick up the counts again */
	for (i = stcol, cols = 0, col = RESCOL;
			(col + fwidth[i]) < COLS-1 && i < MAXCOLS; i++) {
	    cols++;
	    if (col_hidden[i])
		continue;
	    col += fwidth[i];
	}
    }
    /* Now - same process on the rows */
    if (strow <= currow) { 
	for (i = strow, rows = 0, row=RESROW; row<LINES && i<MAXROWS; i++) {
	    rows++;
	    if (row_hidden[i])
		continue;
	    row++;
	}
    }
    while (strow + rows - 1 < currow || currow < strow) {
	FullUpdate++;
	if (strow - 1 == currow) {    /* How about up one? */
	    strow--;
	} else if (strow + rows == currow) {   /* Down one? */
	    strow++;
	} else {
	    /* Try to put the cursor in the center of the screen */
	    row = (LINES - RESROW) / 2 + RESROW; 
	    strow = currow;
	    for (i=currow-1; i >= 0 && row-1 > RESROW; i--) {
		strow--;
		if (row_hidden[i])
		    continue;
		row--;
	    }
	}
	/* Now pick up the counts again */
	for (i = strow, rows = 0, row=RESROW; row<LINES && i<MAXROWS; i++) {
	    rows++;
	    if (row_hidden[i])
		continue;
	    row++;
	}
    }
    mxcol = stcol + cols - 1;
    mxrow = strow + rows - 1;
    if (FullUpdate) {
	(void) move (2, 0);
	(void) clrtobot ();
	(void) standout();
	for (row=RESROW, i=strow; i <= mxrow; i++) {
	    if (row_hidden[i]) 
		continue;
	    (void) move(row,0);
#if MAXROWS < 1000
	    (void) printw("%-*d", RESCOL-1, i);
#else
	    (void) printw("%-*d", RESCOL, i);
#endif
	    row++;
	}
	(void) move (2,0);
	(void) printw("%*s", RESCOL, " ");
	for (col=RESCOL, i = stcol; i <= mxcol; i++) {
	    register int k;
	    if (col_hidden[i])
		continue;
	    (void) move(2, col);
	    k = fwidth[i]/2;
	    if (k == 0)
		(void) printw("%1s", coltoa(i));
	    else
	        (void) printw("%*s%-*s", k, " ", fwidth[i]-k, coltoa(i));
	    col += fwidth[i];
	}
	(void) standend();
    }

    /* Get rid of cursor standout on the cell at previous cursor position */
    (void) move(lastmx, lastmy);
    if (showcell)
        repaint(lastmx, lastmy, fwidth[lastcol]);

    if (showrange) {
	minsr = showsr < currow ? showsr : currow;
	minsc = showsc < curcol ? showsc : curcol;
	maxsr = showsr > currow ? showsr : currow;
	maxsc = showsc > curcol ? showsc : curcol;

	if (showtop) {
	    (void) move(1,0);
	    (void) clrtoeol();
	    (void) printw("Default range:  %s",
			    r_name(minsr, minsc, maxsr, maxsc));
	}
    }

    /* Repaint the visible screen */
    for (row = strow, r = RESROW; row <= mxrow; row++) {
	register c = RESCOL;
	int do_stand = 0;
	int fieldlen;
	int nextcol;

	if (row_hidden[row])
	    continue;
	for (p = &tbl[row][col = stcol]; col <= mxcol;
	         p += nextcol - col,  col = nextcol, c += fieldlen) {

	    nextcol = col+1;
	    if (col_hidden[col]) {
		fieldlen = 0;
		continue;
	    }

	    fieldlen = fwidth[col];

	    /*
	     * Set standout if:
	     *
	     * - showing ranges, and not showing cells which need to be filled
	     *   in, and not showing cell expressions, and in a range, OR
	     *
	     * - if showing cells which need to be filled in and this one is
	     *   of that type (has a value and doesn't have an expression, or
	     *   it is a string expression), OR
	     *
	     * - if showing cells which have expressions and this one does.
	     */

	    if ((showrange && (! showneed) && (! showexpr)
			   && (row >= minsr) && (row <= maxsr)
			   && (col >= minsc) && (col <= maxsc))

	     || (showneed && (*p) && ((*p) -> flags & is_valid)
	      && (((*p) -> flags & is_strexpr) || ! ((*p) -> expr)))

	     || (showexpr && (*p) && ((*p) -> expr)))
	    {
		do_stand = 1;
	    }

	    if (*p && ((*p) -> flags & is_changed || FullUpdate) || do_stand) {
		(void) move (r, c);
		if (!*p)
		    *p = lookat(row, col);
		if (do_stand) {
		    (void) standout();
		    (*p) -> flags |= is_changed; 
		} else {
		    (*p) -> flags &= ~is_changed;
		}

		/*
		 * Show expression; takes priority over other displays:
		 */

		if (showexpr && ((*p) -> expr)) {
		    linelim = 0;
		    editexp (row, col);		/* set line to expr */
		    linelim = -1;
		    showstring (line, /* leftflush = */ 1, /* hasvalue = */ 0,
				row, col, & nextcol, mxcol, & fieldlen, r, c);
		}
		else {

		    /*
		     * Show cell's numeric value:
		     */

		    if ((*p) -> flags & is_valid) {
			char field[1024];
			(void)sprintf(field,"%*.*f", fwidth[col], precision[col], (*p)->v);
			if(strlen(field) > fwidth[col]) {
			    for(i = 0; i<fwidth[col]; i++)
				(void)addch('*');
			} else {
			    (void)addstr(field);
			}
		    }

		    /*
		     * Show cell's label string:
		     */

		    if ((*p) -> label) {
			showstring ((*p) -> label,
				    (*p) -> flags & is_leftflush,
				    (*p) -> flags & is_valid,
				    row, col, & nextcol, mxcol,
				    & fieldlen, r, c);
		    }

		    /*
		     * repaint a blank cell:
		     */

		    if (!((*p)->flags & is_valid) && !(*p)->label) {
			(void) printw ("%*s", fwidth[col], " ");
		    }
		} /* else */

		if (do_stand) {
		    (void) standend();
		    do_stand = 0;
		}
	    }
	}
	r++;
    }
	    
    (void) move(lastmy, lastmx+fwidth[lastcol]);
#ifndef INTERNATIONAL
    if((inch() & 0x7f) == '<')
#else
    if((inch() & 0xff) == '<')
#endif /* INTERNATIONAL */
        (void) addstr(under_cursor);
    lastmy =  RESROW;
    for (row = strow; row < currow; row++)
	if (!row_hidden[row])
		lastmy += 1;
    lastmx = RESCOL;
    for (col = stcol; col < curcol; col++)
	if (!col_hidden[col])
		lastmx += fwidth[col];
    lastcol = curcol;
    (void) move(lastmx, lastmy);
    if (showcell && (! showneed) && (! showexpr)) {
        (void) standout();
        repaint(lastmx, lastmy, fwidth[lastcol]);
        (void) standend();
    }
    (void) move(lastmy, lastmx+fwidth[lastcol]);
#ifndef INTERNATIONAL /*changed to make consistent with all other uses of INTERNATIONAL */
    *under_cursor = (inch() & 0x7f);
#else
    *under_cursor = (inch() & 0xff);
#endif /* INTERNATIONAL */
    (void) addstr("<");

    (void) move (0, 0);
    (void) clrtoeol ();
    if (linelim >= 0) {
	(void) addstr (">> ");
	(void) addstr (line);
    } else {
	if (showtop) {			/* show top line */
	    register struct ent *p1;
	    int printed = 0;		/* printed something? */

            (void) printw ("%s%d ", coltoa (curcol), currow);

	    if ((p1 = tbl [currow] [curcol]) != NULL)
	    {
		if (p1 -> expr)		/* has expr of some type */
		{
		    linelim = 0;
		    editexp (currow, curcol);	/* set line to expr */
		    linelim = -1;
		}

		/*
		 * Display string part of cell:
		 */

		if ((p1 -> expr) && (p1 -> flags & is_strexpr))
		{
		    (void) addstr ((p1 -> flags & is_leftflush) ? "<{" : ">{");
		    (void) addstr (line);
		    (void) addstr ("} ");	/* and this '}' is for vi % */
		    printed = 1;
		}
		else if (p1 -> label)		/* has constant label only */
		{
		    (void) addstr ((p1 -> flags & is_leftflush) ? "<\"" : ">\"");
		    (void) addstr (p1 -> label);
		    (void) addstr ("\" ");
		    printed = 1;
		}

		/*
		 * Display value part of cell:
		 */

		if (p1 -> flags & is_valid)	/* has value or num expr */
		{
		    if ((! (p1 -> expr)) || (p1 -> flags & is_strexpr))
			(void) sprintf (line, "%.15g", p1 -> v);

		    (void) addstr ("[");
		    (void) addstr (line);
		    (void) addstr ("]");
		    printed = 1;
		}
	    }

	    if (! printed)
		(void) addstr ("[]");
	}
	(void) move (lastmy, lastmx + fwidth[lastcol]);
    }
    if (revmsg[0]) {
	(void) move(0, 0);
	(void) clrtoeol ();	/* get rid of topline display */
	(void) printw(revmsg);
	revmsg[0] = 0;		/* don't show it again */
	(void) move (lastmy, lastmx + fwidth[lastcol]);
    }
    FullUpdate = 0;
}


char    *progname;


void
startshow()
{
    showrange = 1;
    showsr = currow;
    showsc = curcol;
}

void
showdr()
{
    int     minsr, minsc, maxsr, maxsc;

    minsr = showsr < currow ? showsr : currow;
    minsc = showsc < curcol ? showsc : curcol;
    maxsr = showsr > currow ? showsr : currow;
    maxsc = showsc > curcol ? showsc : curcol;
    (void) sprintf (line+linelim,"%s", r_name(minsr, minsc, maxsr, maxsc));
}

void
setorder(i)
int i;
{
	if((i == BYROWS)||(i == BYCOLS))
	    calc_order = i;
	else
	    error("Not yet implemented");
}

void
setauto(i)
int i;
{
	autocalc = i;
}


#ifdef VMS

void
goraw()
{
    VMS_read_raw = 1;
    FullUpdate++;
}

void
deraw()
{
    (void) move (LINES - 1, 0);
    (void) clrtoeol();
    (void) refresh();
    VMS_read_raw = 0;
}

#else /* VMS */

void
goraw()
{
#if defined SYSV2 || defined SYSV3
    fixterm();
#else /* SYSV2 || SYSV3 */
    cbreak();
    nonl();
    noecho ();
#endif /* SYSV2 || SYSV3 */
    kbd_again();
    (void) clear();
    FullUpdate++;
}

void
deraw()
{
    (void) move (LINES - 1, 0);
    (void) clrtoeol();
    (void) refresh();
#if defined SYSV2 || defined SYSV3
    resetterm();
#else
    nocbreak();
    nl();
    echo();
#endif
    resetkbd();
}

#endif /* VMS */

void
signals()
{
#ifdef SIGVOID
# ifdef __STDC__ /*__HELIOS*/
    void quit(int);
    void time_out(int);
# else
    void quit();
    void time_out();
# endif
#else
    int quit();
    int time_out();
#endif

    (void) signal(SIGINT, SIG_IGN);
    (void) signal(SIGQUIT, quit);
    (void) signal(SIGPIPE, quit);
    (void) signal(SIGTERM, quit);
    (void) signal(SIGALRM, time_out);
    (void) signal(SIGFPE, quit);
    (void) signal(SIGBUS, quit);
}

#ifdef SIGVOID
void
#endif
quit(dummy)
int dummy;
{
    deraw();
    resetkbd();
    endwin();
    exit(1);
}

int
modcheck(endstr)
char *endstr;
{
    if (modflg && curfile[0]) {
	char ch, lin[100];

	(void) move (0, 0);
	(void) clrtoeol ();
	(void) sprintf (lin,"File \"%s\" is modified, save%s? ",curfile,endstr);
	(void) addstr (lin);
	(void) refresh();
	ch = nmgetch();
	if ( ch != 'y' && ch != 'Y' && ch != 'n' && ch != 'N' ) {
	    error("y or n response required");
	    return (1);
	}
 	if (ch != 'n' && ch != 'N') {
 	    if (writefile(curfile, 0, 0, maxrow, maxcol) < 0)
 		return (1);
	} else if (ch == ctl ('g') || ch == ESC) return(1);
    } else if (modflg) {
	char ch, lin[100];

	(void) move (0, 0);
	(void) clrtoeol ();
	(void) sprintf (lin,"Do you want a chance to save the data? ");
	(void) addstr (lin);
	(void) refresh();
	ch = nmgetch();
	if ( ch != 'y' && ch != 'Y' && ch != 'n' && ch != 'N' ) {
	    error("y or n response required");
	    return (1);
	}
	if (ch == 'n' || ch == 'N') return(0);
	else return(1);
      }
    return(0);
}

int
main (argc, argv)
int argc;
char  **argv;
{
    int     inloop = 1;
    register int   c;
    int     edistate = -1;
    int     arg = 1;
    int     narg;
    int     nedistate;
    int	    running;
    char    *revi;

    /*
     * Keep command line options around until the file is read so the
     * command line overrides file options
     */

    int Mopt = 0;
    int Nopt = 0;
    int Copt = 0; 
    int Ropt = 0;

    progname = argv[0];
    while (argc > 1 && argv[1][0] == '-') {
	argv++;
	argc--;
    	switch (argv[0][1]) {
	    case 'x':
#ifdef VMS
		    (void) fprintf(stderr, "Crypt not available for VMS\n");
		    exit(1);
#else 
		    Crypt = 1;
#endif
		    break;
	    case 'm':
		    Mopt = 1;
		    break;
	    case 'n':
		    Nopt = 1;
		    break;
	    case 'c':
		    Copt = 1;
		    break;
	    case 'r':
		    Ropt = 1;
		    break;
	    default:
		    (void) fprintf(stderr,"%s: unrecognized option: \"%c\"\n",
			progname,argv[0][1]);
		    exit(1);
	}
    }

    {
	register    i;
	for (i = 0; i < MAXCOLS; i++) {
	    fwidth[i] = DEFWIDTH;
	    precision[i] = DEFPREC;
	}
    }
    curfile[0]=0;

    signals();
    (void) initscr();
    (void) clear();
#ifdef VMS
    VMS_read_raw = 1;
#else
    nonl();
    noecho ();
    cbreak();
#endif
    initkbd();


    /*
     * Build revision message for later use:
     */

    (void) strcpy (revmsg, progname);
    for (revi = rev; (*revi++) != ':'; );	/* copy after colon */
    (void) strcat (revmsg, revi);
    revmsg [strlen (revmsg) - 2] = 0;		/* erase last character */
    (void) strcat (revmsg, ":  Type '?' for help.");

    if (argc > 1) {
	(void) strcpy(curfile,argv[1]);
	readfile (argv[1], 0);
    }

    if (Mopt)
	autocalc = 0;
    if (Nopt)
	numeric = 1;
    if (Copt)
	calc_order = BYCOLS;
    if (Ropt)
	calc_order = BYROWS;

    modflg = 0;
#ifdef VENIX
    setbuf (stdin, NULL);
#endif
    FullUpdate++;
    while (inloop) { running = 1;
    while (running) {
	nedistate = -1;
	narg = 1;
	if (edistate < 0 && linelim < 0 && autocalc && (changed || FullUpdate))
	    EvalAll (), changed = 0;
	update();
#ifndef SYSV3
	(void) refresh(); /* 5.3 does a refresh in getch */ 
#endif
	c = nmgetch();
	(void) move (1, 0);
	(void) clrtoeol ();
	(void) fflush (stdout);
	seenerr = 0;
	showneed = 0;	/* reset after each update */
	showexpr = 0;

	if ((c < ' ') || ( c == DEL ))
	    switch (c) {
#ifdef SIGTSTP
		case ctl ('z'):
		    deraw();
		    (void) kill(getpid(),SIGTSTP);

		    /* the pc stops here */

		    goraw();
		    break;
#endif
		case ctl('r'):
		case ctl('l'):
		    FullUpdate++;
		    if (c == ctl ('r'))
			showneed = 1;
		    (void) clearok(stdscr,1);
		    break;
		case ctl ('x'):
		    FullUpdate++;
		    showexpr = 1;
		    (void) clearok(stdscr,1);
		    break;
		default:
		    error ("No such command (^%c)", c + 0100);
		    break;
		case ctl ('b'):
		    backcol(arg);
		    break;
		case ctl ('c'):
		    running = 0;
		    break;

		case ctl ('e'):

		    switch (nmgetch()) {
		    case ctl ('p'): case 'k':	doend (-1, 0);	break;
		    case ctl ('n'): case 'j':	doend ( 1, 0);	break;
		    case ctl ('b'): case 'h':
		    case ctl ('h'):		doend ( 0,-1);	break;
		    case ctl ('f'): case 'l':
		    case ctl ('i'): case ' ':	doend ( 0, 1);	break;

#ifdef __HELIOS
		    case CSI:
#endif
		    case ESC:
		    case ctl ('g'):
			break;

		    default:
			error("Invalid ^E command");
			break;
		    }

		    break;

		case ctl ('f'):
		    forwcol(arg);
		    break;
#ifdef __HELIOS
		case CSI:
#endif
		case ctl ('g'):
		case ESC:	/* ctl ('[') */
		    showrange = 0;
		    linelim = -1;
		    (void) move (1, 0);
		    (void) clrtoeol ();
		    break;
		case DEL:
		case ctl ('h'):
		    if (linelim <= 0) {	/* not editing line */
			backcol(arg);	/* treat like ^B    */
			break;
		    }
		    while (--arg>=0) if (linelim > 0)
			line[--linelim] = 0;
		    break;
		case ctl ('i'): 		/* tab */
		    if (linelim <= 0) {	/* not editing line */
			forwcol(arg);
			break;
		    }

		    if (!showrange) {
			startshow();
		    } else {
			showdr();
			linelim = strlen(line);
			line[linelim++] = ' ';
			line[linelim] = 0;
			showrange = 0;
		    }
		    linelim = strlen (line);
		    break;
		case ctl ('m'):
		case ctl ('j'):
		    showrange = 0;
		    if (linelim < 0)
			line[linelim = 0] = 0;
		    else {
			linelim = 0;
			(void) yyparse ();
			linelim = -1;
		    }
		    break;
		case ctl ('n'):
		    forwrow(arg);
		    break;
		case ctl ('p'):
		    backrow(arg);
		    break;
		case ctl ('q'):
		    break;	/* ignore flow control */
		case ctl ('s'):
		    break;	/* ignore flow control */
		case ctl ('t'):
		    error(
"Toggle:  a:auto  c:cell  e:ext funcs  n:numeric  t:top  x:encrypt  $:pre-scale");
		    (void) refresh();

		    switch (nmgetch()) {
			case 'a': case 'A':
			case 'm': case 'M':
			    autocalc ^= 1;
			    error("Automatic recalculation %sabled.",
				autocalc ? "en":"dis");
			    break;
			case 'n': case 'N':
			    numeric = (! numeric);
			    error ("Numeric input %sabled.",
				    numeric ? "en" : "dis");
			    break;
			case 't': case 'T':
			    showtop = (! showtop);
			    repaint(lastmx, lastmy, fwidth[lastcol]);
			    error ("Top line %sabled.", showtop ? "en" : "dis");
			    break;
			case 'c': case 'C':
			    showcell = (! showcell);
			    repaint(lastmx, lastmy, fwidth[lastcol]);
			    error ("Cell highlighting %sabled.",
				    showcell ? "en" : "dis");
			    break;
			case 'x': case 'X':
			    Crypt = (! Crypt);
			    error ("Encryption %sabled.", Crypt? "en" : "dis");
			    break;
			case '$':
			    if (prescale == 1.0) {
				error ("Prescale enabled.");
				prescale = 0.01;
			    } else {
				prescale = 1.0;
				error ("Prescale disabled.");
			    }
			    break;
			case 'e': case 'E':
			    extfunc = (! extfunc);
			    error ("External functions %sabled.",
				    extfunc? "en" : "dis");
			    break;
#ifdef __HELIOS
			case CSI:
#endif
			case ESC:
			case ctl ('g'):
			    break;
			default:
			    error ("Invalid toggle command");
		    }
		    FullUpdate++;
		    modflg++;
		    break;
		case ctl ('u'):
		    narg = arg * 4;
		    nedistate = 1;
		    break;
		case ctl ('v'):	/* insert variable name */
		    if (linelim > 0) {
		    (void) sprintf (line+linelim,"%s", v_name(currow, curcol));
			linelim = strlen (line);
		    }
		    break;
		case ctl ('w'):	/* insert variable expression */
		    if (linelim > 0) editexp(currow,curcol);
		    break;
		case ctl ('a'):	/* insert variable value */
		    if (linelim > 0) {
			struct ent *p = tbl[currow][curcol];

			if (p && p -> flags & is_valid) {
			    (void) sprintf (line + linelim, "%.*f",
					precision[curcol],p -> v);
			    linelim = strlen (line);
			}
		    }
		    break;
		}
	else
	    if ('0' <= c && c <= '9' && ( (numeric && edistate >= 0) ||
			(!numeric && (linelim < 0 || edistate >= 0))))
	    {
		if (edistate != 0) {
		    if (c == '0')      /* just a '0' goes to left col */
			curcol = 0;
		    else {
		        nedistate = 0;
		        narg = c - '0';
		    }
		} else {
		    nedistate = 0;
		    narg = arg * 10 + (c - '0');
		}
	    }
	    else
		if (linelim >= 0) {     /* Editing line */
		    switch(c) {
			case ')':
			    if (showrange) {
				showdr();
			        showrange = 0;
			        linelim = strlen (line);
			    }
			    break;
		       default:
			    break;
		    }
		    line[linelim++] = c;
		    line[linelim] = 0;
		}
		else if (!numeric && ( c == '+' || c == '-' ) )	
				/* increment/decrement ops */
			{
			    register struct ent *p = tbl[currow][curcol];
			    if (!p)
				break;
			    FullUpdate++;
			    modflg++;
			    if( c == '+' ) p -> v += (double) arg;
			    else p -> v -= (double) arg;
			    }
		else
		    switch (c) {
			case ':':
			    break;	/* Be nice to vi users */

			case '@':
			    EvalAll ();
			    changed = 0;
			    break;

			case '0': case '1': case '2': case '3': case '4':
			case '5': case '6': case '7': case '8': case '9':
			case '-': case '.': case '+':
			    (void) sprintf(line,"let %s = %c",
					v_name(currow, curcol), c);
			    linelim = strlen (line);
			    break;

			case '=':
			    (void) sprintf(line,"let %s = ",
						v_name(currow, curcol));
			    linelim = strlen (line);
			    break;

			case '!':
			    {
			    /*
			     *  "! command"  executes command
			     *  "!"	forks a shell
			     *  "!!" repeats last command
			     */
#ifdef VMS
			    error("Not implemented on VMS");
#else /* VMS */
			    char *shl;
			    int pid, temp;
			    char cmd[MAXCMD];
			    static char lastcmd[MAXCMD];

			    if ((shl = getenv("SHELL")) == NULL)
#ifdef __HELIOS
				shl = "/helios/bin/shell";
#else
				shl = "/bin/sh";
#endif
			    deraw();
			    (void) fputs("! ", stdout);
			    (void) fflush(stdout);
			    (void) fgets(cmd, MAXCMD, stdin);
			    cmd[strlen(cmd) - 1] = '\0';	/* clobber \n */
			    if(strcmp(cmd,"!") == 0)		/* repeat? */
				    (void) strcpy(cmd, lastcmd);
			    else
				    (void) strcpy(lastcmd, cmd);

			    if (modflg)
			    {
				(void) puts ("[No write since last change]");
				(void) fflush (stdout);
			    }

			    if ((pid = fork()) == 0)
			    {
				(void) signal (SIGINT, SIG_DFL);  /* reset */
			        if(strlen(cmd))
					(void)execl(shl,shl,"-c",cmd,(char *)0);
				else
					(void) execl(shl, shl, (char *)0);
				exit(-127);
			    }

			    while (pid != wait(&temp));

			    (void) printf("Press RETURN to continue ");
			    (void)nmgetch();
			    goraw();
#endif /* VMS */
			    break;
			    }

			/*
			 * Range commands:
			 */

			case '/':
			    error (
    "Range:  x:erase  v:value  c:copy  f:fill  d:define  s:show  u:undefine");
			    (void) refresh();

			    switch (nmgetch()) {
			    case 'c':
				(void) sprintf(line,"copy [dest_range src_range] ");
				linelim = strlen(line);
				startshow();
				break;
			    case 'x':
				(void) sprintf(line,"erase [range] ");
				linelim = strlen(line);
				startshow();
				break;
			    case 'v':
				(void) sprintf(line, "value [range] ");
				linelim = strlen(line);
				startshow();
				break;
			    case 'f':
				(void) sprintf(line,"fill [range start inc] ");
				linelim = strlen(line);
				startshow();
				break;
			    case 'd':
				(void) sprintf(line,"define [string range] \"");
				linelim = strlen(line);
				startshow();
				modflg++;
				break;
			    case 'u':
				(void) sprintf(line,"undefine [range] ");
				linelim = strlen(line);
				modflg++;
				break;
			    case 's':
				if(are_ranges())
				{
				FILE *f;
				int pid;
				char px[MAXCMD] ;
				char *pager;

				(void) strcpy(px, "| sort | ");
				if((pager = getenv("PAGER")) == NULL)
					pager = DFLT_PAGER;
				(void) strcat(px,pager);
				f = openout(px, &pid);
				if (!f) {
				    error("Can't open pipe to sort");
				    break;
				}
				list_range(f);
				closeout(f, pid);
				}
				else error("No ranges defined");
				break;
				
#ifdef __HELIOS
			case CSI:
#endif
			    case ESC:
			    case ctl ('g'):
				break;
			   default:
				error("Invalid region command");
				break;
			   }
			   break;

			/*
			 * Row/column commands:
			 */

			case 'i':
			case 'a':
			case 'd':
			case 'p':
			case 'v':
			case 'z':
			case 's':
			    {
				register rcqual;

				if ((rcqual = get_rcqual (c)) == '\0') {
				    error ("Invalid row/column command");
				    break;
				}

				error ("");	/* clear line */

				if ( rcqual == ESC || rcqual == ctl('g'))
				    break;

				switch (c) {

				case 'i':
				    if (rcqual == 'r')	insertrow (arg);
				    else		insertcol (arg);
				    break;

				case 'a':
				    if (rcqual == 'r')	while (arg--) duprow();
				    else		while (arg--) dupcol();
				    break;

				case 'd':
				    if (rcqual == 'r')	deleterow (arg);
				    else		deletecol (arg);
				    break;

				case 'p':
				    while (arg--)	pullcells (rcqual);
				    break;

				case 'v':
				    if (rcqual == 'r')	rowvalueize (arg);
				    else		colvalueize (arg);
				    modflg = 1;
				    break;

				case 'z':
				    if (rcqual == 'r')	hiderow (arg);
				    else		hidecol (arg);
				    modflg++;
				    break;

				case 's':
				    /* special case; no repeat count */

				    if (rcqual == 'r')	rowshow_op();
				    else		colshow_op();
				    break;
				}
				break;
			    }

			case '$':
			    {
			    register struct ent *p;

			    curcol = MAXCOLS - 1;
			    while (!VALID_CELL(p, currow, curcol) && curcol > 0)
				curcol--;
			    break;
			    }
			case '#':
			    {
			    register struct ent *p;

			    currow = MAXROWS - 1;
			    while (!VALID_CELL(p, currow, curcol) && currow > 0)
				currow--;
			    break;
			    }
			case 'w':
			    {
			    register struct ent *p;

			    while (--arg>=0) {
				do {
				    if (curcol < MAXCOLS - 1)
					curcol++;
				    else {
					if (currow < MAXROWS - 1) {
					    while(++currow < MAXROWS - 1 &&
					            row_hidden[currow]) /* */;
					    curcol = 0;
					} else {
					    error("At end of table");
					    break;
					}
				    }
				} while(col_hidden[curcol] != 0 ||
					!VALID_CELL(p, currow, curcol));
			    }
			    break;
			    }
			case 'b':
			    {
			    register struct ent *p;

			    while (--arg>=0) {
				do {
				    if (curcol) 
					curcol--;
				    else {
					if (currow) {
					    while(--currow > 0 &&
						row_hidden[currow]) /* */;
					    curcol = MAXCOLS - 1;
					} else {
					    error ("At start of table");
					    break;
					}
				    }
				} while( col_hidden[curcol] != 0 ||
					!VALID_CELL(p, currow, curcol));
			    }
			    break;
			    }
			case '^':
			    currow = 0;
			    break;
			case '?':
			    help ();
			    break;
			case '"':
			    (void) sprintf (line, "label %s = \"",
						v_name(currow, curcol));
			    linelim = strlen (line);
			    break;
			case '<':
			    (void) sprintf (line, "leftstring %s = \"",
				    v_name(currow, curcol));
			    linelim = strlen (line);
			    break;
			case '>':
			    (void) sprintf (line, "rightstring %s = \"",
				   v_name(currow, curcol));
			    linelim = strlen (line);
			    break;
			case 'e':
			    editv (currow, curcol);
			    break;
			case 'E':
			    edits (currow, curcol);
			    break;
			case 'f':
			    if (arg == 1)
			        (void) sprintf (line, "format [for column] %s ",
					coltoa(curcol));
			    else {
				(void) sprintf(line, "format [for columns] %s:",
					coltoa(curcol));
				(void) sprintf(line+strlen(line), "%s ",
					coltoa(curcol+arg-1));
			    }
			    error("Current format is %d %d",
					fwidth[curcol],precision[curcol]);
			    linelim = strlen (line);
			    break;
			case 'g':
			    (void) sprintf (line, "goto [v] ");
			    linelim = strlen (line);
			    break;
			case 'P':
			    (void) sprintf (line, "put [\"dest\" range] \"");
			    if (*curfile)
			        error ("Default path is \"%s\"",curfile);
			    linelim = strlen (line);
			    break;
			case 'M':
			    (void) sprintf (line, "merge [\"source\"] \"");
			    linelim = strlen (line);
			    break;
			case 'R':
			    (void) sprintf (line,"merge [\"macro_file\"] \"%s/", mdir);
			    linelim = strlen (line);
			    break;
			case 'D':
			    (void) sprintf (line, "mdir [\"macro_directory\"] \"");
			    linelim = strlen (line);
			    break;
			case 'G':
			    (void) sprintf (line, "get [\"source\"] \"");
			    if (*curfile)
			        error ("Default file is \"%s\"",curfile);
			    linelim = strlen (line);
			    break;
			case 'W':
			    (void) sprintf (line, "write [\"dest\" range] \"");
			    linelim = strlen (line);
			    break;
			case 'S':	/* set options */
			    (void) sprintf (line, "set ");
			    error("Options: byrows, bycols, iterations=n, tblstyle=(0|tbl|latex|tex)");
			    linelim = strlen (line);
			    break;
			case 'T':	/* tbl output */
			    (void) sprintf (line, "tbl [\"dest\" range] \"");
			    linelim = strlen (line);
			    break;
			case 'x':
			    {
			    register struct ent **p;
			    register int c1;

			    flush_saved();
			    if(calc_order == BYROWS) {
			    for (c1 = curcol; arg-- && c1 < MAXCOLS; c1++) {
				p = &tbl[currow][c1];
				if (*p) {
			            free_ent(*p);
			            *p = 0;
				}
			    }
			    }
			    else {
			    for (c1 = currow; arg-- && c1 < MAXROWS; c1++) {
				p = &tbl[c1][curcol];
				if (*p) {
			            free_ent(*p);
			            *p = 0;
				}
			    }
			    }
			    sync_refs();
			    modflg++;
			    FullUpdate++;
			    }
			    break;
			case 'Q':
			case 'q':
			    running = 0;
			    break;
			case 'h':
			    backcol(arg);
			    break;
			case 'j':
			    forwrow(arg);
			    break;
			case 'k':
			    backrow(arg);
			    break;
			case ' ':
			case 'l':
			    forwcol(arg);
			    break;
			case 'm':
			    savedrow = currow;
			    savedcol = curcol;
			    break;
			case 'c': {
			    register struct ent *p = tbl[savedrow][savedcol];
			    register c1;
			    register struct ent *n;
			    if (!p)
				break;
			    FullUpdate++;
			    modflg++;
			    for (c1 = curcol; arg-- && c1 < MAXCOLS; c1++) {
				n = lookat (currow, c1);
				(void) clearent(n);
				copyent( n, p, currow - savedrow, c1 - savedcol);
			    }
			    break;
			}
			default:
			    if ((c & 0177) != c)	/* doesn't this depend on INTERNATIONAL */
				error ("Weird character, decimal %d\n",
					(int) c);
			    else
				    error ("No such command (%c)", c);
			    break;
		    }
	edistate = nedistate;
	arg = narg;
    }				/* while (running) */
    inloop = modcheck(" before exiting");
    }				/*  while (inloop) */
    deraw();
    endwin();
#ifdef VMS	/* Unit VMS "fixes" exit we should say 1 here */
    exit(1);
#else
    exit(0);
#endif
    /*NOTREACHED*/
}
