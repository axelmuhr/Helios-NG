/*
 * The routines in this file provide support for ANSI style terminals
 * over a serial line. The serial I/O services are provided by routines in
 * "termio.c". It compiles into nothing if not an ANSI device.
 *
 * RcsId: $Id: ansi.c,v 1.3 1992/04/30 07:56:03 nickc Exp $
 */

#ifdef HELIOS
#include <attrib.h>			/* helios only			*/
#endif

#define	termdef	1			/* don't define "term" external */

#include        <stdio.h>
#include	"estruct.h"
#include        "edef.h"

#ifdef TRUE
#undef TRUE
#undef FALSE
#define TRUE 1
#define FALSE 0
#endif

#if     ANSI

#ifdef __HELIOSARM
extern Attributes ostate;
#endif

#if	AMIGA
#define NROW    23                      /* Screen size.                 */
#define NCOL    77                      /* Edit if you want to.         */
#else
#define NROW    25                      /* Screen size.                 */
#define NCOL    80                      /* Edit if you want to.         */
#endif
#define	NPAUSE	100			/* # times thru update to pause */
#define	MARGIN	8			/* size of minimim margin and	*/
#define	SCRSIZ	64			/* scroll size for extended lines */
#define BEL     0x07                    /* BEL character.               */
#define ESC     0x1B                    /* ESC character.               */

extern  int     ttopen();               /* Forward references.          */
extern  int     ttgetc();
extern  int     ttputc();
extern  int     ttflush();
extern  int     ttclose();
extern  int     ansimove();
extern  int     ansieeol();
extern  int     ansieeop();
extern  int     ansibeep();
extern  int     ansiopen();
extern	int	ansirev();
extern	int	ansiclose();
extern	int	ansikopen();
extern	int	ansikclose();
extern	int	ansicres();

#if	COLOR
extern	int	ansifcol();
extern	int	ansibcol();

int	cfcolor = -1;		/* current forground color */
int	cbcolor = -1;		/* current background color */
#endif

/*
 * Standard terminal interface dispatch table. Most of the fields point into
 * "termio" code.
 */
TERM    term    = {
	NROW-1,
        NROW-1,
        NCOL,
        NCOL,
	MARGIN,
	SCRSIZ,
	NPAUSE,
        ansiopen,
        ansiclose,
	ansikopen,
	ansikclose,
        ttgetc,
        ttputc,
        ttflush,
        ansimove,
        ansieeol,
        ansieeop,
        ansibeep,
	ansirev,
	ansicres
#if	COLOR
	, ansifcol,
	ansibcol
#endif
};

int
ansiparm(n)
register int    n;
{
        register int q,r;

        q = n/10;
        if (q != 0) {
		r = q/10;
		if (r != 0) {
			ttputc((r%10)+'0');
		}
		ttputc((q%10) + '0');
        }
        ttputc((n%10) + '0');

	return 0;
}

#if	COLOR
int
ansifcol(color)		/* set the current output color */

int color;	/* color to set */

{
	if (color == cfcolor)
		return;
	ttflush();				/* XXX */
	ttputc(ESC);
	ttputc('[');
	ansiparm(color+30);
	ttputc('m');
	cfcolor = color;

	return 0;	
}

int
ansibcol(color)		/* set the current background color */

int color;	/* color to set */

{
	if (color == cbcolor)
		return;
	ttflush();				/* XXX */
	ttputc(ESC);
	ttputc('[');
	ansiparm(color+40);
	ttputc('m');
        cbcolor = color;

	return 0;
}
#endif

int
ansimove(row, col)
int row,col;     
{
  /* IOdebug( "me: move to %d,%d", row, col ); */
  
	ttflush();				/* XXX */
        ttputc(ESC);
        ttputc('[');
        ansiparm(row+1);
        ttputc(';');
        ansiparm(col+1);
        ttputc('H');

	return 0;
}

int
ansieeol()
{
	ttflush();				/* XXX */
        ttputc(ESC);
        ttputc('[');
        ttputc('K');

	return 0;
}

int
ansieeop()
{
#if	COLOR
	ansifcol(gfcolor);
	ansibcol(gbcolor);
#endif
	ttflush();				/* XXX */
        ttputc(ESC);
        ttputc('[');
        ttputc('J');

	return 0;
}

int
ansirev(state)		/* change reverse video state */
int state;	/* TRUE = reverse, FALSE = normal */
{
#if	COLOR
	int ftmp, btmp;		/* temporaries for colors */
#endif
        static int oldstate = FALSE;	/* BLV addition */
					/* Remember the old state, so that */
					/* emacs does not switch back to   */
	if (state == oldstate)		/* normal video for every character */
         return(0);
	oldstate = state;

	ttflush();				/* XXX */
	ttputc(ESC);
	ttputc('[');
	ttputc(state ? '7': '0');
	ttputc('m');
#if	COLOR
	if (state == FALSE) {
		ftmp = cfcolor;
		btmp = cbcolor;
		cfcolor = -1;
		cbcolor = -1;
		ansifcol(ftmp);
		ansibcol(btmp);
	}
#endif
	return 0;
}

int
ansicres()	/* change screen resolution */
{
	return(TRUE);
}

int
spal(dummy)		/* change pallette settings */
int dummy;
{
  return 0;
	/* none for now */
  dummy = dummy;
}

int
ansibeep()
{
        ttputc(BEL);
        ttflush();

	return 0;
}

int
ansiopen()
{
#if     V7 | USG | BSD
        register char *cp;
        char *getenv();

        if ((cp = getenv("TERM")) == NULL) {
                puts("Shell variable TERM not defined!");
                exit(1);
        }
        if (strcmp(cp, "vt100") != 0) {
                puts("Terminal type not 'vt100'!");
                exit(1);
        }
#endif

        ttopen();

#ifdef __HELIOS
	setvbuf(stdin, NULL, _IONBF, 1);
	{
/**
*** BLV - mrow appears to be the number of screen rows - 1, whereas mcol is
*** the real number of columns. Do not change these lines again! If it does not
*** work, the problem is in the window manager which gets the attributes wrong.
**/
#ifndef __HELIOSARM
		extern Attributes ostate;
#endif
		term.t_mrow = term.t_nrow = ostate.Min - 1;
		term.t_mcol = term.t_ncol = ostate.Time;

		/* IOdebug( "me: size = %d rows by %d cols", term.t_mrow + 1, term.t_mcol ); */
	}
#endif
	strcpy(sres, "NORMAL");
	revexist = TRUE;

	return 0;
}

int
ansiclose()

{
#if	COLOR
	ansifcol(7);
	ansibcol(0);
#endif
	ttclose();

	return 0;
}

int
ansikopen()	/* open the keyboard (a noop here) */
{
  return 0;
}

int
ansikclose()	/* close the keyboard (a noop here) */
{
  return 0;
}

#if	FLABEL
int
fnclabel(f, n)		/* label a function key */

int f,n;	/* default flag, numeric argument [unused] */

{
	/* on machines with no function keys...don't bother */
	return(TRUE);
}
#endif
#else
int
ansihello()
{
  return 0;
}
#endif
