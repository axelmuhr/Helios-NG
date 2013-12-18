/*
 *	HP110:	Hewlett Packard 110 Screen Driver
 *
 * $Header: /usr/perihelion/Helios/cmds/emacs/RCS/hp110.c,v 1.1 90/08/23 15:14:51 jon Exp $
 *
 */

#define	termdef	1			/* don't define "term" external */

#include        <stdio.h>
#include	"estruct.h"
#include        "edef.h"

#if     HP110

#define NROW    16                      /* Screen size.                 */
#define NCOL    80                      /* Edit if you want to.         */
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
extern  int     h110move();
extern  int     h110eeol();
extern  int     h110eeop();
extern  int     h110beep();
extern  int     h110open();
extern	int	h110rev();
extern	int	h110cres();
extern	int	h110close();
extern	int	h110kopen();
extern	int	h110kclose();

#if	COLOR
extern	int	h110fcol();
extern	int	h110bcol();

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
        h110open,
        h110close,
	h110kopen,
	h110kclose,
        ttgetc,
        ttputc,
        ttflush,
        h110move,
        h110eeol,
        h110eeop,
        h110beep,
	h110rev,
	h110cres
#if	COLOR
	, h110fcol,
	h110bcol
#endif
};

#if	COLOR
h110fcol(color)		/* set the current output color */

int color;	/* color to set */

{
	if (color == cfcolor)
		return;
	ttputc(ESC);
	ttputc('[');
	h110parm(color+30);
	ttputc('m');
	cfcolor = color;
}

h110bcol(color)		/* set the current background color */

int color;	/* color to set */

{
	if (color == cbcolor)
		return;
	ttputc(ESC);
	ttputc('[');
	h110parm(color+40);
	ttputc('m');
        cbcolor = color;
}
#endif

h110move(row, col)
{
        ttputc(ESC);
        ttputc('[');
        h110parm(row+1);
        ttputc(';');
        h110parm(col+1);
        ttputc('H');
}

h110eeol()
{
        ttputc(ESC);
        ttputc('[');
	ttputc('0');
        ttputc('K');
}

h110eeop()
{
#if	COLOR
	h110fcol(gfcolor);
	h110bcol(gbcolor);
#endif
        ttputc(ESC);
        ttputc('[');
	ttputc('0');
        ttputc('J');
}

h110rev(state)		/* change reverse video state */

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
		h110fcol(ftmp);
		h110bcol(btmp);
	}
#endif
}

h110cres()	/* change screen resolution */

{
	return(TRUE);
}

spal()		/* change pallette register */

{
	/*   not here */
}

h110beep()
{
        ttputc(BEL);
        ttflush();
}

h110parm(n)
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

h110open()
{
	strcpy(sres, "15LINE");
	revexist = TRUE;
        ttopen();
}

h110close()

{
#if	COLOR
	h110fcol(7);
	h110bcol(0);
#endif
	ttclose();
}

h110kopen()

{
}

h110kclose()

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
h110hello()
{
}
#endif
