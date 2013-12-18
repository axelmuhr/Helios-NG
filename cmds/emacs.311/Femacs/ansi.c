/*
 * The routines in this file provide support for ANSI style terminals
 * over a serial line. The serial I/O services are provided by routines in
 * "termio.c". It compiles into nothing if not an ANSI device.
 */

#define	termdef	1			/* don't define "term" external */

#include        <stdio.h>
#include	"estruct.h"
#include	"etype.h"
#include	"etype.h"
#include        "edef.h"
#include	"elang.h"

#if     ANSI

#define NROW    25                      /* Screen size.                 */
#define NCOL    80                      /* Edit if you want to.         */
#define	NPAUSE	100			/* # times thru update to pause */
#define	MARGIN	8			/* size of minimim margin and	*/
#define	SCRSIZ	64			/* scroll size for extended lines */
#define BEL     0x07                    /* BEL character.               */
#define ESC     0x1B                    /* ESC character.               */

/* Forward references.          */
extern int PASCAL NEAR ansimove();
extern int PASCAL NEAR ansieeol();
extern int PASCAL NEAR ansieeop();
extern int PASCAL NEAR ansibeep();
extern int PASCAL NEAR ansiopen();
extern int PASCAL NEAR ansirev();
extern int PASCAL NEAR ansiclose();
extern int PASCAL NEAR ansikopen();
extern int PASCAL NEAR ansikclose();
extern int PASCAL NEAR ansicres();
extern int PASCAL NEAR ansiparm();

#if	COLOR
extern int PASCAL NEAR ansifcol();
extern int PASCAL NEAR ansibcol();

static int cfcolor = -1;	/* current forground color */
static int cbcolor = -1;	/* current background color */

#if	AMIGA
/* apperently the AMIGA does not follow the ANSI standards as
   regards to colors....maybe because of the default pallette
   settings?
*/

/* color translation table */

int coltran[16] = {2, 3, 5, 7, 0, 4, 6, 1,
	 8, 12, 10, 14, 9, 13, 11, 15};
#endif
#endif

/*
 * Standard terminal interface dispatch table. Most of the fields point into
 * "termio" code.
 */
NOSHARE TERM term    = {
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

#if	COLOR
PASCAL NEAR ansifcol(color)		/* set the current output color */

int color;	/* color to set */

{
	if (color == cfcolor)
		return;
	ttputc(ESC);
	ttputc('[');
#if	AMIGA
	ansiparm(coltran[color]+30);
#else
	ansiparm(color+30);
#endif
	ttputc('m');
	cfcolor = color;
}

PASCAL NEAR ansibcol(color)		/* set the current background color */

int color;	/* color to set */

{
	if (color == cbcolor)
		return;
	ttputc(ESC);
	ttputc('[');
#if	AMIGA
	ansiparm(coltran[color]+40);
#else
	ansiparm(color+40);
#endif
	ttputc('m');
        cbcolor = color;
}
#endif

PASCAL NEAR ansimove(row, col)
{
        ttputc(ESC);
        ttputc('[');
        ansiparm(row+1);
        ttputc(';');
        ansiparm(col+1);
        ttputc('H');
}

PASCAL NEAR ansieeol()
{
        ttputc(ESC);
        ttputc('[');
        ttputc('K');
}

PASCAL NEAR ansieeop()
{
#if	COLOR
	ansifcol(gfcolor);
	ansibcol(gbcolor);
#endif
        ttputc(ESC);
        ttputc('[');
        ttputc('J');
}

PASCAL NEAR ansirev(state)		/* change reverse video state */

int state;	/* TRUE = reverse, FALSE = normal */

{
#if	COLOR
	int ftmp, btmp;		/* temporaries for colors */
#endif

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
}

PASCAL NEAR ansicres()	/* change screen resolution */

{
	return(TRUE);
}

PASCAL NEAR spal(dummy)		/* change pallette settings */

{
	/* none for now */
}

PASCAL NEAR ansibeep()
{
        ttputc(BEL);
        ttflush();
}

PASCAL NEAR ansiparm(n)
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
}

PASCAL NEAR ansiopen()
{
#if     V7 | USG | HPUX | BSD | SUN | XENIX
        register char *cp;
        char *getenv();

        if ((cp = getenv("TERM")) == NULL) {
                puts(TEXT4);
/*                   "Shell variable TERM not defined!" */
                meexit(1);
        }
        if (strcmp(cp, "vt100") != 0) {
                puts(TEXT5);
/*                   "Terminal type not 'vt100'!" */
                meexit(1);
        }
#endif
	strcpy(sres, "NORMAL");
	revexist = TRUE;
        ttopen();
}

PASCAL NEAR ansiclose()

{
#if	COLOR
	ansifcol(7);
	ansibcol(0);
#endif
	ttclose();
}

PASCAL NEAR ansikopen()	/* open the keyboard (a noop here) */

{
}

PASCAL NEAR ansikclose()	/* close the keyboard (a noop here) */

{
}

#if	FLABEL
fnclabel(f, n)		/* label a function key */

int f,n;	/* default flag, numeric argument [unused] */

{
	/* on machines with no function keys...don't bother */
	return(TRUE);
}
#endif
#else
ansihello()
{
}
#endif
