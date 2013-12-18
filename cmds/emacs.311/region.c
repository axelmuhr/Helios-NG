/*
 * The routines in this file
 * deal with the region, that magic space
 * between "." and mark. Some functions are
 * commands. Some functions are just for
 * internal use.
 */
#include        <stdio.h>
#include	"estruct.h"
#include	"eproto.h"
#include        "edef.h"
#include	"elang.h"

/*      regundent:      undent a region.
*/

PASCAL NEAR regundent(
int n )  /* number of spaces to undent */

{
        REGION region;
        LINE *olddotp, *oldmarkp;
        int  olddoto, oldmarko, infolds;

        /* check for a valid region first */
        if (getregion(&region) != TRUE)
                return(FALSE);

        olddotp = curwp->w_dotp;
        olddoto = curwp->w_doto;
	oldmarkp = curwp->w_markp[0];
	oldmarko = curwp->w_marko[0];

        /* start at the top of the region.... */
        curwp->w_dotp = region.r_linep;
        curwp->w_doto = region.r_offset;

        region.r_size += region.r_offset;

        infolds = 0;

        /* scan the region... undenting */
        while (region.r_size > 0L) {
                region.r_size -= (long) llength(curwp->w_dotp) + 1;
                if ((curwp->w_doto == 0) && (infolds == 0)) {
                        ldelete((long)n, FALSE, FALSE, FALSE);
                        curwp->w_dotp->l_lmargin -= n;
                }
                if (infolds > 0) /* line too short for calculation */
                        region.r_size -=n;
                if (curwp->w_dotp->l_type == LSOEFOLD)
                        infolds++;
                else if (curwp->w_dotp->l_type == LEOEFOLD)
                        infolds--;
                curwp->w_dotp = lforw(curwp->w_dotp);
                curwp->w_doto = 0;
        }

        /* place us at the start position */
        curwp->w_dotp = olddotp;
        curwp->w_markp[0] = oldmarkp;
	if (olddotp == region.r_linep) {
		curwp->w_doto = olddoto;
	        curwp->w_marko[0] = (oldmarko < n) ? 0 : oldmarko - n;
	}
	else {
		curwp->w_marko[0] = oldmarko;
        	curwp->w_doto = (olddoto < n) ? 0 : olddoto - n;
	}

        return(TRUE);
}

/*	reglines:	how many lines in the current region
			used by the trim/entab/detab-region commands
*/

int PASCAL NEAR reglines()

{
        register LINE *linep;	/* position while scanning */
	register int n;		/* number of lines in this current region */
        REGION region;

	/* check for a valid region first */
        if (getrawregion(&region) != TRUE)
                return(0);

	/* start at the top of the region.... */
        linep = region.r_linep;
	region.r_size += region.r_offset;
        n = 0;

        /* scan the region... counting lines */
        while (region.r_size > 0L) {
		region.r_size -= (long) llength(linep) + 1;
		linep = lforw(linep);
		n++;
	}

	/* place us at the beginning of the region */
        curwp->w_dotp = region.r_linep;
        curwp->w_doto = region.r_offset;

        return(n);
}

/*
 * Kill the region. Ask "getregion"
 * to figure out the bounds of the region.
 * Move "." to the start, and kill the characters.
 * Bound to "C-W".
 * If necessary remove any fold indentation.
 */
PASCAL NEAR killregion(
  int f,
  int n )	/* prefix flag and argument */

{
        register int    s;
        REGION          region;
        int             indent;
	int		setfoldbound;

	if (curbp->b_mode&MDVIEW)	/* don't allow this command if	*/
		return(rdonly());	/* we are in read only mode	*/
	killedcr = TRUE;
	setfoldbound = FALSE;

        if (curwp->w_markp[0] == (LINE *)NULL) {
                mlwrite(TEXT76);
/*                      "No mark set in this window" */
                return(FALSE);
	}

	/* Start & finish at left margin if at < the left margin */
	if (curwp->w_doto < curwp->w_dotp->l_lmargin)
		curwp->w_doto = curwp->w_dotp->l_lmargin;

	if ((curwp->w_markp[0] != (LINE *)NULL) &&
	    (curwp->w_marko[0] < curwp->w_markp[0]->l_lmargin))
		curwp->w_marko[0] = curwp->w_markp[0]->l_lmargin;

        /* check if limits are fold lines, if so set offset to 0 */
        if (curwp->w_dotp->l_type != LNORMAL) {
		curwp->w_doto = 0; 
		setfoldbound = TRUE;
	}
        if ((curwp->w_markp[0] != (LINE*)NULL) &&
            (curwp->w_markp[0]->l_type != LNORMAL)) {
               	curwp->w_marko[0] = 0;
		setfoldbound = TRUE;
	}

	/* Don't allow the mark or dot to be on a fold marker if
         * we aren't killing whole lines.
        */
	if (curwp->w_dotp->l_type != LNORMAL) {
        	if (curwp->w_marko[0] > curwp->w_markp[0]->l_lmargin)
			return(beep());
                else
			curwp->w_marko[0] = 0;
	}
            
	if (curwp->w_markp[0]->l_type != LNORMAL) {
        	if (curwp->w_doto > curwp->w_dotp->l_lmargin)
			return(beep());
		else
			curwp->w_doto = 0;
	}
            
	/* call getrawregion first, cos this will check if we cross crease */
        if ((s = getrawregion(&region)) != TRUE)
                return(s);

	/* Check if prefix removal required */
		indent = ((curwp->w_dotp == curwp->w_markp[0]) &&
   			  (curwp->w_doto >= curwp->w_dotp->l_lmargin) &&
			  (curwp->w_marko[0] >= curwp->w_markp[0]->l_lmargin)) ? 
			 0 : minleftmarg(curwp->w_dotp); 
		
        if (indent > 0) {
                regundent(indent);
		/* recalculate the region */
	        if ((s = getrawregion(&region)) != TRUE)
        	        return(s);
	}
        if ((lastflag&CFKILL) == 0)             /* This is a kill type  */
                kdelete();                      /* command, so do magic */
        thisflag |= CFKILL;                     /* kill buffer stuff.   */
        curwp->w_dotp = region.r_linep;
        curwp->w_doto = region.r_offset;
        s = ldelete(region.r_size, TRUE, TRUE, FALSE);
	curwp->w_dotp->l_lmargin = minleftmarg(curwp->w_dotp);
        if (curwp->w_dotp->l_type == LNORMAL)
		marginchk(curwp->w_dotp, curwp->w_doto); /* may have killed indent */
	if (curwp->w_doto < curwp->w_dotp->l_lmargin)
		curwp->w_doto = curwp->w_dotp->l_lmargin;
	killfoldbound = setfoldbound; /* kdelete changes flag */

        return(s);
}

/*
 * Copy all of the characters in the
 * region to the kill buffer. Don't move dot
 * at all. This is a bit like a kill region followed
 * by a yank. Bound to "M-W".
 * We have to remove any indentation due to folds, copy and
 * then put indentation back.
 */
PASCAL NEAR copyregion(
  int f,
  int n )	/* prefix flag and argument */

{
        register LINE   *linep;
        register int    loffs;
        register int    s;
        REGION          region;
	int		setfoldbound;

	setfoldbound = FALSE;
	killedcr = TRUE;

        /* check if limits are fold lines, if so set offset to 0 */

        if (curwp->w_dotp->l_type != LNORMAL) {
                curwp->w_doto = 0;
		setfoldbound = TRUE;
	}
        if ((curwp->w_markp[0] != (LINE*)NULL) &&
            (curwp->w_markp[0]->l_type != LNORMAL)) {
               	curwp->w_marko[0] = 0;
		setfoldbound = TRUE;
	}
        if ((s=getrawregion(&region)) != TRUE)
                return(s);
        if ((lastflag&CFKILL) == 0)             /* Kill type command.   */
                kdelete();
        thisflag |= CFKILL;
        linep = region.r_linep;                 /* Current line.        */
        if (region.r_offset < linep->l_lmargin) { /* move to margin */
                loffs = linep->l_lmargin;
                region.r_size -= ((long) linep->l_lmargin - (long) region.r_offset);
        }
        else
	        loffs = region.r_offset;                /* Current offset.      */

        while (region.r_size-- > 0) {
                if (loffs == llength(linep)) {  /* End of line.         */
                        if ((s=kinsert(FORWARD, '\r')) != TRUE)
                                return(s);
                        linep = linep->l_fp;
                        loffs = region.r_linep->l_lmargin;
                        region.r_size -= loffs;
                } else {                        /* Middle of line.      */
                        if ((s=kinsert(FORWARD, lgetc(linep, loffs))) != TRUE)
                                return(s);
                        ++loffs;
                }
        }
	killfoldbound = setfoldbound; /* kdelete changes flag */
	mlwrite(TEXT70);
/*              "[region copied]" */
        return(TRUE);
}

/*
 * Lower case region. Zap all of the upper
 * case characters in the region to lower case. Use
 * the region code to set the limits. Scan the buffer,
 * doing the changes. Call "lchange" to ensure that
 * redisplay is done in all buffers. Bound to
 * "C-X C-L".
 */
PASCAL NEAR lowerregion(
  int f,
  int n )	/* prefix flag and argument */

{
        register LINE   *linep;
        register int    loffs;
        register int    s;
        int c;
        REGION          region;

	if (curbp->b_mode&MDVIEW)	/* don't allow this command if	*/
		return(rdonly());	/* we are in read only mode	*/
        if ((s=getregion(&region)) != TRUE)
                return(s);
        lchange(WFHARD);
        linep = region.r_linep;
        loffs = region.r_offset;
        while (region.r_size--) {
                if (loffs == llength(linep)) {
                        linep = lforw(linep);
                        loffs = 0;
                } else {
                        c = lgetc(linep, loffs);
			c = lowerc(c);
                        lputc(linep, loffs, c);
                        ++loffs;
                }
        }
        return(TRUE);
}

/*
 * Upper case region. Zap all of the lower
 * case characters in the region to upper case. Use
 * the region code to set the limits. Scan the buffer,
 * doing the changes. Call "lchange" to ensure that
 * redisplay is done in all buffers. Bound to
 * "C-X C-L".
 */
PASCAL NEAR upperregion(
  int f,
  int n )	/* prefix flag and argument */

{
        register LINE   *linep;
        register int    loffs;
        register int    s;
        int c;
        REGION          region;

	if (curbp->b_mode&MDVIEW)	/* don't allow this command if	*/
		return(rdonly());	/* we are in read only mode	*/
        if ((s=getregion(&region)) != TRUE)
                return(s);
        lchange(WFHARD);
        linep = region.r_linep;
        loffs = region.r_offset;
        while (region.r_size--) {
                if (loffs == llength(linep)) {
                        linep = lforw(linep);
                        loffs = 0;
                } else {
                        c = lgetc(linep, loffs);
			c = upperc(c);
                        lputc(linep, loffs, c);
                        ++loffs;
                }
        }
        return(TRUE);
}

/*	Narrow-to-region (^X-<) makes all but the current region in
	the current buffer invisable and unchangable
*/

PASCAL NEAR narrow(
  int f,
  int n )	/* prefix flag and argument */

{
        register int status;	/* return status */
	BUFFER *bp;		/* buffer being narrowed */
	SCREEN *scrp;		/* screen to fix pointers in */
	WINDOW *wp;		/* windows to fix up pointers in as well */
	REGION creg;		/* region boundry structure */
	int cmark;		/* current mark */

	/* find the proper buffer and make sure we aren't already narrow */
	bp = curwp->w_bufp;		/* find the right buffer */
	if (bp->b_flag&BFNAROW) {
		mlwrite(TEXT71);
/*                      "%%This buffer is already narrowed" */
		return(FALSE);
	}

	/* find the boundries of the current region */
        if ((status=getregion(&creg)) != TRUE)
                return(status);
        curwp->w_dotp = creg.r_linep;	/* only by full lines please! */
        curwp->w_doto = 0;
	creg.r_size += (long)creg.r_offset;
	if (creg.r_size <= (long)curwp->w_dotp->l_used) {
		mlwrite(TEXT72);
/*                      "%%Must narrow at least 1 full line" */
		return(FALSE);
	}

	/* archive the top fragment */
	if (bp->b_linep->l_fp != creg.r_linep) {
		bp->b_topline = bp->b_linep->l_fp;
		creg.r_linep->l_bp->l_fp = (LINE *)NULL;
		bp->b_linep->l_fp = creg.r_linep;
		creg.r_linep->l_bp = bp->b_linep;
	}

	/* move forward to the end of this region
	   (a long number of bytes perhaps) */
	while (creg.r_size > (long)32000) {
		forwchar(TRUE, 32000);
		creg.r_size -= (long)32000;
	}
	forwchar(TRUE, (int)creg.r_size);
	curwp->w_doto = 0;		/* only full lines! */

	/* archive the bottom fragment */
	if (bp->b_linep != curwp->w_dotp) {
		bp->b_botline = curwp->w_dotp;
		bp->b_botline->l_bp->l_fp = bp->b_linep;
		bp->b_linep->l_bp->l_fp = (LINE *)NULL;
		bp->b_linep->l_bp = bp->b_botline->l_bp;
	}

	/* in all screens.... */
	scrp = first_screen;
	while (scrp) {

		/* let all the proper windows be updated */
		wp = scrp->s_first_window;
		while (wp) {
			if (wp->w_bufp == bp) {
				wp->w_linep = creg.r_linep;
				wp->w_dotp = creg.r_linep;
				wp->w_doto = 0;
				for (cmark = 0; cmark < NMARKS; cmark++) {
					wp->w_markp[cmark] = creg.r_linep;
					wp->w_marko[cmark] = 0;
				}
				wp->w_flag |= (WFHARD|WFMODE);
			}
			wp = wp->w_wndp;
		}

		/* next screen! */
		scrp = scrp->s_next_screen;
	}

	/* and now remember we are narrowed */
	bp->b_flag |= BFNAROW;
	mlwrite(TEXT73);
/*              "[Buffer is narrowed]" */
        return(TRUE);
}

/*	widen-from-region (^X->) restores a narrowed region	*/

PASCAL NEAR widen(
int f,
  int n )	/* prefix flag and argument */

{
	LINE *lp;	/* temp line pointer */
	BUFFER *bp;	/* buffer being narrowed */
	SCREEN *scrp;	/* screen to fix pointers in */
	WINDOW *wp;	/* windows to fix up pointers in as well */
	int cmark;	/* current mark */

	/* find the proper buffer and make sure we are narrow */
	bp = curwp->w_bufp;		/* find the right buffer */
	if ((bp->b_flag&BFNAROW) == 0) {
		mlwrite(TEXT74);
/*                      "%%This buffer is not narrowed" */
		return(FALSE);
	}

	/* recover the top fragment */
	if (bp->b_topline != (LINE *)NULL) {
		lp = bp->b_topline;
		while (lp->l_fp != (LINE *)NULL)
			lp = lp->l_fp;
		lp->l_fp = bp->b_linep->l_fp;
		lp->l_fp->l_bp = lp;
		bp->b_linep->l_fp = bp->b_topline;
		bp->b_topline->l_bp = bp->b_linep;
		bp->b_topline = (LINE *)NULL;
	}

	/* recover the bottom fragment */
	if (bp->b_botline != (LINE *)NULL) {

		/* if the point is at EOF, move it to
		   the beginning of the bottom fragment */
		if (curwp->w_dotp == bp->b_linep) {
			curwp->w_dotp = bp->b_botline;
			curwp->w_doto = 0;		/* this should be redundent */
		}
	
		/* if any marks are at EOF, move them to
		   the beginning of the bottom fragment */
		for (cmark = 0; cmark < NMARKS; cmark++) {
			if (curwp->w_markp[cmark] == bp->b_linep) {
				curwp->w_markp[cmark] = bp->b_botline;
				curwp->w_marko[cmark] = 0;
			}
		}

		/* connect the bottom fragment */
		lp = bp->b_botline;
		while (lp->l_fp != (LINE *)NULL)
			lp = lp->l_fp;
		lp->l_fp = bp->b_linep;
		bp->b_linep->l_bp->l_fp = bp->b_botline;
		bp->b_botline->l_bp = bp->b_linep->l_bp;
		bp->b_linep->l_bp = lp;
		bp->b_botline = (LINE *)NULL;
	}

	/* in all screens.... */
	scrp = first_screen;
	while (scrp) {

		/* let all the proper windows be updated */
		wp = scrp->s_first_window;
		while (wp) {
			if (wp->w_bufp == bp)
				wp->w_flag |= (WFHARD|WFMODE);
			wp = wp->w_wndp;
		}

		/* next screen! */
		scrp = scrp->s_next_screen;
	}

	/* and now remember we are not narrowed */
	bp->b_flag &= (~BFNAROW);
	mlwrite(TEXT75);
/*              "[Buffer is widened]" */
        return(TRUE);
}

/*
 * This routine figures out the bounds of the region in the current
 * window, and fills in the fields of the "REGION" structure pointed to by
 * "rp". Because the dot and mark are usually very close together, we scan
 * outward from dot looking for mark. This should save time. Return a
 * standard code. Callers of this routine should be prepared to get an
 * "ABORT" status; we might make this have the confirm thing later.
 */

PASCAL NEAR getregion(
register REGION *rp )

{
        register LINE   *flp;
        register LINE   *blp;
        long fsize;
        long bsize;

        if (curwp->w_markp[0] == (LINE *)NULL) {
                mlwrite(TEXT76);
/*                      "No mark set in this window" */
                return(FALSE);
        }
        if (curwp->w_dotp == curwp->w_markp[0]) {
                rp->r_linep = curwp->w_dotp;
                if (curwp->w_doto < curwp->w_marko[0]) {
                        rp->r_offset = curwp->w_doto;
                        rp->r_size = ((long)curwp->w_marko[0] - curwp->w_doto);
                } else {
                        rp->r_offset = curwp->w_marko[0];
                        rp->r_size   = ((long)curwp->w_doto - curwp->w_marko[0]);
                }
                return(TRUE);
        }
        blp = curwp->w_dotp;
        bsize = (long)curwp->w_doto;
        flp = curwp->w_dotp;
        fsize = (llength(flp) - (long) curwp->w_doto + 1);
        while (flp!=curbp->b_linep || lback(blp)!=curbp->b_linep) {
                if (flp != curbp->b_linep) {
                        flp = lforw(flp);
                        if (flp == curwp->w_markp[0]) {
                                rp->r_linep = curwp->w_dotp;
                                rp->r_offset = curwp->w_doto;
                                rp->r_size = fsize+curwp->w_marko[0];
                                return(TRUE);
                        }
                        fsize += (long) llength(flp)+1;
                }
                if (lback(blp) != curbp->b_linep) {
                        blp = lback(blp);
                        bsize += (long) llength(blp)+1;
                        if (blp == curwp->w_markp[0]) {
                                rp->r_linep = blp;
                                rp->r_offset = curwp->w_marko[0];
                                rp->r_size = bsize - curwp->w_marko[0];
                                return(TRUE);
                        }
                }
        }
        mlwrite(TEXT77);
/*              "Bug: lost mark" */
        return(FALSE);
}

/*
 * This routine figures out the bounds of the region in the current
 * window, IT DESCENDS INTO FOLDS, and fills in the fields of the "REGION" 
 * structure pointed to by "rp". Because the dot and mark are usually very
 * close together, we scan outward from dot looking for mark. 
 * This should save time. Return a standard code. Callers of 
 * this routine should be prepared to get an
 * "ABORT" status; we might make this have the confirm thing later.
 * This routine will return an error if an incomplete open fold is 
 * within the region.
 */

PASCAL NEAR getrawregion(
register REGION *rp )

{
        register LINE   *flp;
        register LINE   *blp;
        long fsize;
        long bsize;
        int  ffcnt, fferr, bfcnt, bferr;

        if (curwp->w_markp[0] == (LINE *)NULL) {
                mlwrite(TEXT76);
/*                      "No mark set in this window" */
                return(FALSE);
        }
        if (curwp->w_dotp == curwp->w_markp[0]) {
                if (curwp->w_dotp->l_type != LNORMAL)
                        return(beep());
                rp->r_linep = curwp->w_dotp;
                if (curwp->w_doto < curwp->w_marko[0]) {
                        rp->r_offset = curwp->w_doto;
                        rp->r_size = ((long)curwp->w_marko[0] - curwp->w_doto);
                } else {
                        rp->r_offset = curwp->w_marko[0];
                        rp->r_size = ((long)curwp->w_doto - curwp->w_marko[0]);
                }
                return(TRUE);
        }

        /* initialise fold error counts and markers */
        /* since search is bi-directional, errors may be virtual */
        if ((curwp->w_dotp->l_type == LSOEFOLD) ||
	    (curwp->w_dotp->l_type == LSOFOLD)) {
		if (curwp->w_doto > curwp->w_dotp->l_lmargin)
			curwp->w_doto = loffset(curwp->w_dotp);
                bfcnt = 0;
                ffcnt = 1;
                bferr = fferr = FALSE;
        }
        else if (curwp->w_dotp->l_type == LEOEFOLD) {
		if (curwp->w_doto > curwp->w_dotp->l_lmargin)
			curwp->w_doto = curwp->w_dotp->l_used;
                bfcnt = 0;
                ffcnt = -1;
                bferr = FALSE;
                fferr = TRUE;
        }
        else {
                bfcnt = ffcnt = 0;
                bferr = fferr = FALSE;
        }

        blp = curwp->w_dotp;
        bsize = (long)curwp->w_doto;
        flp = curwp->w_dotp;
        fsize = (llength(flp) - (long) curwp->w_doto + 1);

        while (flp!=curbp->b_linep || blp->l_bp !=curbp->b_linep) {
                if (flp != curbp->b_linep) {
                        flp = flp->l_fp;
                        if (flp == curwp->w_markp[0]) {
                                rp->r_linep = curwp->w_dotp;
                                rp->r_offset = curwp->w_doto;
                                rp->r_size = fsize+curwp->w_marko[0];
                                if (ffcnt || fferr)
                                        mlwrite(TEXT232);
/*                                      "%%Region contains folds" */
                                return(!ffcnt && !fferr);
                        }
                        /* going forward we don't want to check last line */
                        else if ((flp->l_type == LSOEFOLD) ||
				 (flp->l_type == LSOFOLD))
                                ffcnt++;
                        else if ((flp->l_type == LEOEFOLD) ||
				 (flp->l_type == LEOFOLD))
                                fferr = fferr | (--ffcnt < 0);
                        fsize += (long) llength(flp)+1;
                }
                if (blp->l_bp != curbp->b_linep) {
                        blp = blp->l_bp;
                        if ((blp->l_type == LSOEFOLD) ||
			    (blp->l_type == LSOFOLD))
                                bferr = bferr | (--bfcnt < 0);
                        else if ((blp->l_type == LEOEFOLD) ||
				 (blp->l_type == LEOFOLD))
                                bfcnt++;
                        bsize += (long) llength(blp)+1;
                        if (blp == curwp->w_markp[0]) {
                                rp->r_linep = blp;
                                rp->r_offset = curwp->w_marko[0];
                                rp->r_size = bsize - curwp->w_marko[0];
                                if (bfcnt || bferr)
                                        mlwrite(TEXT232);
/*                                      "%%Region contains folds" */
                                return(!bfcnt && !bferr);
                        }
                }
        }
        mlwrite(TEXT77);
/*              "Bug: lost mark" */
        return(FALSE);
}

/*
 * Copy all of the characters in the region to the string buffer.
 * It is assumed that the buffer size is at least one plus the
 * region size.
 */
char *PASCAL NEAR regtostr(
char *buf,
REGION *region )

{
	register LINE	*linep;
	register int	loffs;
	register long	rsize;
	register char	*ptr;

	ptr = buf;
	linep = region->r_linep;		/* Current line.	*/
	loffs = region->r_offset;		/* Current offset.	*/
	rsize = region->r_size;
	while (rsize--) {
		if (loffs == llength(linep)) {	/* End of line.		*/
			*ptr = '\r';
			linep = lforw(linep);
			loffs = 0;
		} else {			/* Middle of line.	*/
			*ptr = lgetc(linep, loffs);
			++loffs;
		}
		ptr++;
	}
	*ptr = '\0';
	return buf;
}

char *PASCAL NEAR getreg(	/* return some of the contents of the current region */

char *value )

{
	REGION region;

	/* get the region limits */
	if (getregion(&region) != TRUE)
		return(errorm);

	/* don't let the region be larger than a string can hold */
	if (region.r_size >= NSTRING)
		region.r_size = NSTRING - 1;
	return(regtostr(value, &region));
}


PASCAL NEAR indent_region(	/* indent a region n tab-stops */

  int f,
  int n )	/* default flag and numeric repeat count */

{
	register int inc;	/* increment to next line [sgn(n)] */
	int count;

	if (curbp->b_mode&MDVIEW)	/* don't allow this command if	*/
		return(rdonly());	/* we are in read only mode	*/

	if (f == FALSE)
		count = 1;
	else
		count = n;
	n = reglines();

	/* loop thru indenting n lines */
	inc = ((n > 0) ? 1 : -1);
	while (n) {
		curwp->w_doto = 0;	/* start at the beginning */

		/* shift current line using tabs */
		if (!((curbp->b_mode & MDCMOD) &&
		     (lgetc(curwp->w_dotp, curwp->w_doto) == '#')) ) {
				linsert(count, '\t', TRUE);
		}

		/* advance/or back to the next line */
		forwline(TRUE, inc, TRUE);
		n -= inc;
	}

	curwp->w_doto = 0;
	thisflag &= ~CFCPCN;	/* flag that this resets the goal column */
	lchange(WFEDIT);	/* yes, we have made at least an edit */
	return(TRUE);
}

PASCAL NEAR undent_region(	/* undent a region n tab-stops */

  int f,
  int n )	/* default flag and numeric repeat count */

{
	register int inc;	/* increment to next line [sgn(n)] */
	int i, count;

	if (curbp->b_mode&MDVIEW)	/* don't allow this command if	*/
		return(rdonly());	/* we are in read only mode	*/

	if (f == FALSE)
		count = 1;
	else
		count = n;
	n = reglines();

	/* loop thru undenting n lines */
	inc = ((n > 0) ? 1 : -1);
	while (n) {
		/* unshift current line using tabs */
		for (i = 0; i < count; i++) {
			curwp->w_doto = 0;	/* start at the beginning */
			if ((curwp->w_dotp->l_used > 0) &&
			    (lgetc(curwp->w_dotp, curwp->w_doto) == '\t')) {
				ldelete(1L, FALSE,FALSE,FALSE);
			}
		}

		/* advance/or back to the next line */
		forwline(TRUE, inc,TRUE);
		n -= inc;
	}

	thisflag &= ~CFCPCN;	/* flag that this resets the goal column */
	lchange(WFEDIT);	/* yes, we have made at least an edit */
	return(TRUE);
}
