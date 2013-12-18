/*
 * The routines in this file provide support for the Data General Model 10
 * Microcomputer.
 *
 * $Header: /usr/perihelion/Helios/cmds/emacs/RCS/dg10.c,v 1.1 90/08/23 15:13:36 jon Exp $
 *
 */

#define	termdef	1			/* don't define "term" external */

#include        <stdio.h>
#include	"estruct.h"
#include        "edef.h"

#if     DG10

#define NROW    24                      /* Screen size.                 */
#define NCOL    80                      /* Edit if you want to.         */
#define	NPAUSE	100			/* # times thru update to pause */
#define	MARGIN	8			/* size of minimim margin and	*/
#define	SCRSIZ	64			/* scroll size for extended lines */
#define BEL     0x07                    /* BEL character.               */
#define ESC     30                      /* DG10 ESC character.          */

extern  int     ttopen();               /* Forward references.          */
extern  int     ttgetc();
extern  int     ttputc();
extern  int     ttflush();
extern  int     ttclose();
extern	int	dg10kopen();
extern	int	dg10kclose();
extern  int     dg10move();
extern  int     dg10eeol();
extern  int     dg10eeop();
extern  int     dg10beep();
extern  int     dg10open();
extern	int	dg10rev();
extern	int	dg10close();
extern	int	dg10cres();

#if	COLOR
extern	int	dg10fcol();
extern	int	dg10bcol();

int	cfcolor = -1;		/* current forground color */
int	cbcolor = -1;		/* current background color */
int	ctrans[] = {		/* emacs -> DG10 color translation table */
	0, 4, 2, 6, 1, 5, 3, 7};
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
        dg10open,
        dg10close,
	dg10kopen,
	dg10kclose,
        ttgetc,
        ttputc,
        ttflush,
        dg10move,
        dg10eeol,
        dg10eeop,
        dg10beep,
	dg10rev,
	dg10cres
#if	COLOR
	, dg10fcol,
	dg10bcol
#endif
};

#if	COLOR
dg10fcol(color)		/* set the current output color */

int color;	/* color to set */

{
	if (color == cfcolor)
		return;
	ttputc(ESC);
	ttputc(0101);
	ttputc(ctrans[color]);
	cfcolor = color;
}

dg10bcol(color)		/* set the current background color */

int color;	/* color to set */

{
	if (color == cbcolor)
		return;
	ttputc(ESC);
	ttputc(0102);
	ttputc(ctrans[color]);
        cbcolor = color;
}
#endif

dg10move(row, col)
{
	ttputc(16);
        ttputc(col);
	ttputc(row);
}

dg10eeol()
{
        ttputc(11);
}

dg10eeop()
{
#if	COLOR
	dg10fcol(gfcolor);
	dg10bcol(gbcolor);
#endif
        ttputc(ESC);
        ttputc(0106);
        ttputc(0106);
}

dg10rev(state)		/* change reverse video state */

int state;	/* TRUE = reverse, FALSE = normal */

{
#if	COLOR
	if (state == TRUE) {
		dg10fcol(0);
		dg10bcol(7);
	}
#else
	ttputc(ESC);
	ttputc(state ? 0104: 0105);
#endif
}

dg10cres()	/* change screen resolution */

{
	return(TRUE);
}

spal()		/* change palette string */

{
	/*	Does nothing here	*/
}

dg10beep()
{
        ttputc(BEL);
        ttflush();
}

dg10open()
{
	strcpy(sres, "NORMAL");
	revexist = TRUE;
        ttopen();
}

dg10close()

{
#if	COLOR
	dg10fcol(7);
	dg10bcol(0);
#endif
	ttclose();
}

dg10kopen()

{
}

dg10kclose()

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
dg10hello()
{
}
#endif
