/*
 * Name:	MG 2a
 *		Commodore Amiga mouse handling 
 * Created:	Distant past
 * Last edit:	28-Nov-87  mic@emx.cc.utexas.edu
 */

#include <exec/types.h>
#include <intuition/intuition.h>
#undef	TRUE
#undef	FALSE
#include "def.h"
#ifndef	NO_MACRO
#include "macro.h"
#endif

extern	int	ttmouse();
extern	int	forwline();
extern	int	forwchar();
extern	int	setmark();
extern	int	isetmark();

/* stuff for go-to-window-and-do-it functions */
extern	int	reposition();
extern	int	delfword();
extern	int	killline();
extern	int	forwdel();
extern	int	justone();
extern	int	killregion();
extern	int	yank();
extern	int	forwpage();
extern	int	backpage();
extern	int	splitwind();
extern	int	delwind();
extern	int	gotobob();
extern	int	gotoeob();
extern	int	enlargewind();
extern	int	shrinkwind();

/*
 * Handle the mouse click that's been passed by ttgetc() and position
 * dot where the user pointed at.  If this is the same position
 * where the user pointed the last time, set the mark, whether or
 * not this is a true double-click. This isn't a true double-click,
 * but it does most of what we want.
 */

static USHORT	oldrow = HUGE, oldcol = HUGE;	/* last mouse click	*/
static USHORT	newrow, newcol;			/* next mouse click	*/

amigamouse(f, n)
{
	if (!dottomouse())			/* sets newrow, newcol	*/
		return (FALSE);
	if ((newrow == oldrow) && (newcol == oldcol))
		setmark(FFRAND, 1);		/* double-click		*/
	oldrow = newrow;		    	/* save state		*/
	oldcol = newcol;
	return (TRUE);
}

/*
 * Recenter on selected line
 */
mreposition(f, n)
{
	if (!dottomouse())
		return (FALSE);
	return (reposition(f, n));
}

/*
 * Delete word after selected char
 */
mdelfword(f, n)
{
	if (!dottomouse())
		return (FALSE);
	return (delfword(f, n));
}

/*
 * Move to selection, kill line
 */
mkillline(f, n)
{
	if (!dottomouse())
		return (FALSE);
	return (killline(f, n));
}

/*
 * Move to selection, kill word
 */
mforwdel(f, n)
{
	if (!dottomouse())
		return (FALSE);
	return (forwdel(f, n));
}

/*
 * Move to selection, kill line
 */
mdelwhite(f, n)
{
	if (!dottomouse())
		return (FALSE);
	return (justone(f, n));
}

/*
 * Set mark, move to selection, kill region.
 */
mkillregion(f, n)
{
	register struct LINE *p_old;
	register short	o_old;

	p_old = curwp->w_markp;		/* Save old mark */
	o_old = curwp->w_marko;
	isetmark();				/* and set current one */
	if (!dottomouse()) {
		curwp->w_markp = p_old;	/* Oops - put mark back */
		curwp->w_marko = o_old;
		return (FALSE);
		}
	return (killregion(f, n));
}

/*
 * Move to selection, yank kill buffer
 */
myank(f, n)
{
	if (!dottomouse())
		return (FALSE);
	return (yank(f, n));
}

/*
 * Select window pointed to by mouse, then scroll down
 */

mforwpage(f, n)
{
	if (!dottomouse())
		return (FALSE);
	return (forwpage(f, n));
}

/*
 * Select buffer, scroll page down
 */
mbackpage(f, n)
{
	if (!dottomouse())
		return (FALSE);
	return (backpage(f, n));
}

/*
 * Select the window, split it.
 */
msplitwind(f, n)
{
	if (!dottomouse())
		return (FALSE);
	return (splitwind(f, n));
}

/*
 * Select the buffer, delete it.
 */
mdelwind(f, n)
{
	if (!dottomouse())
		return (FALSE);
	return (delwind(f, n));
}

/*
 * Select window, goto beginning of buffer
 */
mgotobob(f, n)
{
	if (!dottomouse())
		return (FALSE);
	return (gotobob(f, n));
}

/*
 * Select window, go to end of buffer
 */
mgotoeob(f, n)
{
	if (!dottomouse())
		return (FALSE);
	return (gotoeob(f, n));
}

/*
 * Select window, enlarge it.
 */
menlargewind(f, n)
{
	if (!dottomouse())
		return (FALSE);
	return (enlargewind(f, n));
}
	
/*
 * Select window, shrink it.
 */
mshrinkwind(f, n)
{
	if (!dottomouse())
		return (FALSE);
	return (shrinkwind(f, n));
}

/*
 * Utility routine to move dot to where the user clicked.  If in
 * mode line, chooses that buffer as the one to work on.
 */

static dottomouse()
{
	register WINDOW *wp;
	register int	dot;
	register int	col;
	register int	c;
	int		getkey();

#ifndef	NO_MACRO
	if (inmacro)
		return FALSE;	/* can't record mouse clicks */
#endif

	/* read the next 2 characters to get the col, row info, using
	 * getkey() to record them (or re-read them if in a macro).
	 */
	newcol = getkey(FALSE) - M_X_ZERO;
	newrow = getkey(FALSE) - M_Y_ZERO;

#ifndef	NO_MACRO
	if (macrodef) {		/* menu picks can't be practically recorded */
		ewprintf("Can't record mouse clicks");
		return (FALSE);
	}
#endif

	/* find out which window was clicked in				*/
	for (wp = wheadp; wp != NULL; wp = wp->w_wndp)
		if ((newrow >= wp->w_toprow) && 
			(newrow <= (wp->w_toprow + wp->w_ntrows)))
			break;

	if (wp == NULL)				/* echo line		*/
		return (ABORT);
	else if (newrow == wp->w_toprow + wp->w_ntrows) {/* mode line */
		curwp = wp;			/* just change buffer	 */
		curbp = wp->w_bufp;
	} else {
		/* move to selected window, move dot to top left	*/
		curwp = wp;
		curbp = wp->w_bufp;
		curwp->w_dotp = wp->w_linep;
		curwp->w_doto = 0;

		/* go forward the correct # of lines 		*/
		forwline(FFRAND, newrow - curwp->w_toprow);
	
		/* go forward the correct # of characters	*/
		/* need to count them out because of tabs	*/
		col = dot = 0;
		while ((col < newcol) && (dot < llength(curwp->w_dotp))) {
			c = lgetc(curwp->w_dotp, dot++);
			if (c == CCHR('I'))
				col |= 0x07;
			else if (ISCTRL(c) != FALSE)
				++col;
			++col;
		}
		if (col > newcol) dot--;	/* back up to tab/ctrl char */
		forwchar(FFRAND, dot);
	}
	return (TRUE);
}
