/*	MOUSE.C:	Mouse functionality commands
			for MicroEMACS 3.10
			originally written by Dave G. Conroy
			modified by Jeff Lomicka and Daniel Lawrence
*/

#include	<stdio.h>
#include	"estruct.h"
#include	"etype.h"
#include        "edef.h"
#include	"elang.h"

#define	MNONE	0			/* Mouse commands.		*/
#define	MMOVE	1
#define	MREG	2

#if	MOUSE
NOSHARE int	lastypos = HUGE;	/* Last mouse event row.	*/
NOSHARE int	lastxpos = HUGE;	/* Last mouse event column.	*/
NOSHARE int	lastmcmd = MNONE;	/* Last mouse command.		*/

/*
 * Move mouse button, down. The window that the
 * mouse is in is always selected (this lets you select a
 * window by clicking anyplace in it, even off the end
 * of the text). If the mouse points at text then dot is
 * moved to that location.
 */
PASCAL NEAR movemd(f, n)
{
	register WINDOW	*wp;
	register WINDOW	*lastwp;
	register LINE	*lp;

	/* if anything has changed, reset the click count */
	if (lastmcmd != MMOVE || lastypos!=ypos || lastxpos!=xpos)
		nclicks = 0;
	++nclicks;
	lastwp = mousewindow(lastypos);		/* remember least window */

	/* reset the last position */
	lastypos = ypos;
	lastxpos = xpos;
	lastmcmd = MMOVE;

	/* if we move the mouse off the windows, don't move anything */
	if ((wp=mousewindow(ypos)) == NULL)
		return(FALSE);

	/* if we are on the line with the point, adjust for extended lines */
	if (wp == curwp && (lp = mouseline(wp, ypos)) == curwp->w_dotp)
		xpos += lbound;

	/* make the window the mouse points to current */
	curwp = wp;
	curbp = wp->w_bufp;

	/* if we changed windows, update the modelines */
	if (wp != lastwp)
		upmode();

	/* if we aren't off the end of the text, move the point to the mouse */
	if ((lp=mouseline(wp, ypos)) != NULL) {
		curwp->w_dotp = lp;
		curwp->w_doto = mouseoffset(wp, lp, xpos);
	}

	return(TRUE);
}


/*	mouse-region-down:	mouse region operations

	nclicks = 0:	move cursor to mouse
			set-mark

		  1:	move cursor to mouse
			kill-region
*/

PASCAL NEAR mregdown(f, n)
{
	register WINDOW	*wp;
	register WINDOW	*lastwp;
	register LINE	*lp;

	/* if anything has changed, reset the click count */
	if (lastmcmd != MREG || lastypos != ypos || lastxpos != xpos)
		nclicks = 0;
	++nclicks;
	lastwp = mousewindow(lastypos);		/* remember least window */

	/* reset the last position */
	lastypos = ypos;
	lastxpos = xpos;
	lastmcmd = MREG;

	/* if we move the mouse off the windows, don't move anything */
	if ((wp=mousewindow(ypos)) == NULL)
		return(FALSE);

	/* if we are on the line with the point, adjust for extended lines */
	if (wp == curwp && (lp = mouseline(wp, ypos)) == curwp->w_dotp)
		xpos += lbound;

	/* make the window the mouse points to current */
	curwp = wp;
	curbp = wp->w_bufp;

	/* if we changed windows, update the modelines */
	if (wp != lastwp)
		upmode();

	/* if we aren't off the end of the text, move the point to the mouse */
	if ((lp=mouseline(wp, ypos)) != NULL) {
		curwp->w_dotp = lp;
		curwp->w_doto = mouseoffset(wp, lp, xpos);
	}

	/* perform the region function */
	if (nclicks == 1) {
		return(setmark(FALSE, 0));
	} else {
		lastflag &= ~CFKILL;
		return(killregion(FALSE, 0));
	}
}

/*	mouse-region-up:	mouse region operations

	If the corrosponding downclick was on a modeline, then we
	wish to delete the indicated window. Otherwise we are using
	this button to copy/paste.

	nclicks = 0:	move cursor to mouse
			copy-region

		  1:	move cursor to mouse
			yank

		  3:	reset nclicks to 0
*/

PASCAL NEAR mregup(f, n)

{
	register WINDOW	*wp;
	register WINDOW	*lastwp;
	register LINE *lp;
	register int lastmodeline;	/* was the dowbclick on a modeline? */

	/* if anything has changed, reset the click count */
	if (lastmcmd != MREG || lastypos != ypos || lastxpos != xpos)
		nclicks = 0;
	++nclicks;
	lastwp = mousewindow(lastypos);		/* remember least window */

	/* did we down click on a modeline? */
	lastmodeline = ismodeline(lastwp, lastypos);

	/* reset the last position */
	lastypos = ypos;
	lastxpos = xpos;
	lastmcmd = MREG;

	/* if we started on a modeline.... */
	if (lastmodeline)
		return(delwind(TRUE, 0));

	/* if we move the mouse off the windows, don't move anything */
	if ((wp=mousewindow(ypos)) == NULL)
		return(FALSE);

	/* if we are on the line with the point, adjust for extended lines */
	if (wp == curwp && (lp = mouseline(wp, ypos)) == curwp->w_dotp)
		xpos += lbound;

	/* make the window the mouse points to current */
	curwp = wp;
	curbp = wp->w_bufp;

	/* if we aren't off the end of the text, move the point to the mouse */
	if ((lp=mouseline(wp, ypos)) != NULL && nclicks < 3) {
		curwp->w_dotp = lp;
		curwp->w_doto = mouseoffset(wp, lp, xpos);
	}

	/* if we changed windows, update the modelines, abort the new op */
	if (wp != lastwp) {
		upmode();
		return(TRUE);
	}

	/* perform the region function */
	if (nclicks == 1) {
		return(copyregion(FALSE, 0));
	} else if (nclicks == 2) {
		return(yank(FALSE, 1));
	} else {
		nclicks = 0;
		return(TRUE);
	}
}

/*
 * Move mouse button, up. The up click must be
 * in the text region of a window. If the old click was in a
 * mode line then the mode line moves to the row of the
 * up click. If the old click is not in a mode line then the
 * window scrolls. The code in this function is just
 * too complex!
 */
PASCAL NEAR movemu(f, n)
{
	register WINDOW	*lastwp;
	register WINDOW	*wp;
	register int	lastmodeline;	/* was the dowbclick on a modeline? */
	register int	deltay;
	register int	deltax;

	/* no movement... fail the command */
	if (lastypos==ypos && lastxpos==xpos)
		return(FALSE);

	/* if the down click was in the bottom right corner...
	   then we are resizing */
	if (lastypos == term.t_nrow && lastxpos + 1 == term.t_ncol) {
		(*term.t_eeop)();
		newwidth(TRUE, xpos + 1);
		newsize(TRUE, ypos + 1);
		return(TRUE);
	}

	/* if the down click was not in a window.. fail the command
	   (for example, if we click on the command line) */
	if ((lastwp=mousewindow(lastypos)) == NULL)
		return(FALSE);

	/* did we down click on a modeline? */
	lastmodeline = ismodeline(lastwp, lastypos);

	/* are we not in a window? fail it then */
	if ((wp=mousewindow(ypos)) == NULL)
		return(FALSE);

	/* how far did we move? */
	deltay = lastypos-ypos;
	deltax = lastxpos-xpos;
	lastypos = ypos;
	lastxpos = xpos;

	/* if we started on a modeline.... */
	if (lastmodeline) {

		/* move the window horizontally */
		if (deltax != 0 && (diagflag || deltay == 0)) {
			lastwp->w_fcol += deltax;
			if (lastwp->w_fcol < 0)
				lastwp->w_fcol = 0;
			lastwp->w_flag |= WFMODE|WFHARD;
			if (deltay == 0)
				return(TRUE);
		}

		/* don't allow the bottom modeline to move */
		if (lastwp->w_wndp == NULL)
			return(FALSE);

		/* shrink the current window */
		if (deltay > 0) {
			if (lastwp != wp)
				return(FALSE);
			curwp = wp;
			curbp = wp->w_bufp;
			return(shrinkwind(TRUE, deltay));
		}

		/* or grow it */
		if (deltay < 0) {
			if (wp != lastwp->w_wndp)
				return(FALSE);
			curwp = lastwp;
			curbp = lastwp->w_bufp;
			return(enlargewind(TRUE, -deltay));
		}
	}

	/* did we up click in a modeline? fail it them */
	if (ismodeline(wp, ypos) != FALSE)
		return(FALSE);

	/* we can not move outside the current window */
	if (lastwp != wp)
		return(FALSE);

	/* move horizontally as well? */
	if (deltax != 0 && (diagflag || deltay == 0)) {
		wp->w_fcol += deltax;
		if (wp->w_fcol < 0)
			wp->w_fcol = 0;
		wp->w_flag |= WFMODE;
	}

	/* and move the screen */
	return(mvdnwind(TRUE, deltay));
}

/*
 * Return a pointer to the WINDOW structure
 * for the window in which "row" is located, or NULL
 * if "row" isn't in any window. The mode line is
 * considered to be part of the window.
 */

WINDOW *PASCAL NEAR mousewindow(row)

register int	row;

{
	register WINDOW	*wp;

	wp = wheadp;
	while (wp != NULL) {
		if (row < wp->w_ntrows+1)
			return(wp);
		row -= wp->w_ntrows+1;
		wp = wp->w_wndp;
	}
	return(NULL);
}

/*
 * The row "row" is a row within the window
 * whose WINDOW structure is pointed to by the "wp"
 * argument. Find the associated line, and return a pointer
 * to it. Return NULL if the mouse is on the mode line,
 * or if the mouse is pointed off the end of the
 * text in the buffer.
 */

LINE *PASCAL NEAR mouseline(wp, row)

register WINDOW	*wp;
register int	row;

{
	register LINE	*lp;

	row -= wp->w_toprow;
	if (row >= wp->w_ntrows)
		return(NULL);
	lp = wp->w_linep;
	while (row--) {
		if (lp == wp->w_bufp->b_linep)	/* Hit the end.		*/
			return(NULL);
		lp = lforw(lp);
	}
	return(lp);
}

/*
 * Return the best character offset to use
 * to describe column "col", as viewed from the line whose
 * LINE structure is pointed to by "lp".
 */

PASCAL NEAR mouseoffset(wp, lp, col)

register WINDOW *wp;
register LINE	*lp;
register int	col;

{
	register int	c;
	register int	offset;
	register int	curcol;
	register int	newcol;

	offset = 0;
	curcol = 0;
	col += wp->w_fcol;	/* adjust for extended lines */
	while (offset != llength(lp)) {
		newcol = curcol;
		if ((c=lgetc(lp, offset)) == '\t')
			newcol += -(newcol % tabsize) + (tabsize - 1);
		else if (c<32)	/* ISCTRL */
			++newcol;
		++newcol;
		if (newcol > col)
			break;
		curcol = newcol;
		++offset;
	}
	return(offset);
}

PASCAL NEAR ismodeline(wp, row)
WINDOW	*wp;
{
	if (row == wp->w_toprow+wp->w_ntrows && modeflag)
		return(TRUE);
	return(FALSE);
}

/* The mouse has been used to resize the physical window. Now we need to
   let emacs know about the newsize, and have him force a re-draw
*/

PASCAL NEAR resizm(f, n)

int f, n;	/* these are ignored... we get the new size info from
		   the mouse driver */
{
	(*term.t_eeop)();
	newwidth(TRUE, xpos);
	newsize(TRUE, ypos);
	return(TRUE);
}

#else
mousehello()
{
}
#endif
