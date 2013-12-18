/*
 * The functions in this file are a general set of line management utilities.
 * They are the only routines that touch the text. They also touch the buffer
 * and window structures, to make sure that the necessary updating gets done.
 * There are routines in this file that handle the kill buffer too. It isn't
 * here for any good reason.
 *
 * Note that this code only updates the dot and mark values in the window list.
 * Since all the code acts on the current window, the buffer that we are
 * editing must be being displayed, which means that "b_nwnd" is non zero,
 * which means that the dot and mark values in the buffer headers are nonsense.
 */

/*      Modifications:
        11-Sep-89       Mike Burrow (INMOS)     Added folding.
*/

#include	<stdio.h>
#include	"estruct.h"
#include	"eproto.h"
#include	"edef.h"
#include	"elang.h"

#define	BSIZE(a)	(a + NBLOCK - 1) & (~(NBLOCK - 1))
/* variables to count recursion */
int intoindentfold = 0;
int intoundentfold = 0;

/* Do something, but not much! if we detect an error.
 * MJB: 20-Sep-89.
 */
PASCAL NEAR int beep()
{
        TTputc(BELL);
        return(FALSE);
}


/*
 * Insert spaces so no line is shorter than the left margin value.
 */
PASCAL NEAR int marginchk(
LINE *lp,
int  oset )
{
        LINE    *olddotp;
        int     olddoto, i;

        /*if ((oset = loffset(lp)) < lp->l_lmargin) {*/
        if (oset < lp->l_lmargin) {

                /* save old values and setup for insert */
                if (lp != curwp->w_dotp){
                        olddotp = curwp->w_dotp;
                        curwp->w_dotp = lp;
                }
                else
                        olddotp = (LINE *) NULL;

                olddoto = curwp->w_doto;
                curwp->w_doto = oset;

                for (i = oset; i < lp->l_lmargin; i++)
                        linsert(1, lp->l_bp->l_text[i], FALSE);

                /* reset values */
                if (olddotp != (LINE *)NULL)
                        curwp->w_dotp = olddotp;
                if (olddoto < curwp->w_dotp->l_lmargin)
                        curwp->w_doto = curwp->w_dotp->l_lmargin;
                else
                        curwp->w_doto = olddoto;
        }
        return(TRUE);
}

/*
 * This routine allocates a block of memory large enough to hold a LINE
 * containing "used" characters. Return a pointer to the new block, or
 * NULL if there isn't any memory left. Print a message in the message
 * line if no space.
 */

LINE *PASCAL NEAR lalloc(register int used)

{
	register LINE	*lp;

	if ((lp = (LINE *)malloc(sizeof(LINE)+used)) == NULL) {
		mlwrite(TEXT94);
/*                      "%%Out of memory" */
		return(NULL);
	}
	lp->l_size = used;
	lp->l_used = used;
        lp->l_lmargin = 0;
	lp->l_omargin = 0;
        lp->l_type = LNORMAL;
        lp->l_foldp = (LINE *)NULL;
	return(lp);
}

/*
 * Delete line "lp". Fix all of the links that might point at it (they are
 * moved to offset 0 of the next line. Unlink the line from whatever buffer it
 * might be in. Release the memory. The buffers are updated too; the magic
 * conditions described in the above comments don't hold here.
 */
PASCAL NEAR void lfree(register LINE	*lp)
{
	register BUFFER *bp;
	SCREEN *scrp;		/* screen to fix pointers in */
	register WINDOW *wp;
	register int cmark;		/* current mark */

	/* in all screens.... */
	scrp = first_screen;
	while (scrp) {
		wp = scrp->s_first_window;
		while (wp != NULL) {
			if (wp->w_linep == lp)
				wp->w_linep = lp->l_fp;
			if (wp->w_dotp	== lp) {
				wp->w_dotp  = lp->l_fp;
				wp->w_doto  = 0;
			}
			for (cmark = 0; cmark < NMARKS; cmark++) {
				if (wp->w_markp[cmark] == lp) {
					wp->w_markp[cmark] = lp->l_fp;
					wp->w_marko[cmark] = 0;
				}
			}
			wp = wp->w_wndp;
		}

		/* next screen! */
		scrp = scrp->s_next_screen;
	}

	bp = bheadp;
	while (bp != NULL) {
		if (bp->b_nwnd == 0) {
			if (bp->b_dotp	== lp) {
				bp->b_dotp = lp->l_fp;
				bp->b_doto = 0;
			}
			for (cmark = 0; cmark < NMARKS; cmark++) {
				if (bp->b_markp[cmark] == lp) {
					bp->b_markp[cmark] = lp->l_fp;
					bp->b_marko[cmark] = 0;
				}
			}
		}
		bp = bp->b_bufp;
	}
	lp->l_bp->l_fp = lp->l_fp;
	lp->l_fp->l_bp = lp->l_bp;
	free((char *) lp);
}

/* Given a line, return the index of the first non whitespace
 * character. MJB: 25-Sep-89. 
 */
int PASCAL NEAR loffset(LINE * lp)
{
        int i;

        i = 0;
        while ((i < lp->l_used) &&
               ((lp->l_text[i] == ' ') ||
                (lp->l_text[i] == '\t')))
                i++;
        return(i);
}


/* Given a line pointer, find the next one to use.
 * This is only called by lforw. MJB: 14-Sep-89.
 * Changed for open fold symbols. MJB 28-Sep-89.
 */
LINE *PASCAL NEAR nxtfwd(register LINE   *lp)
{
        if ((lp->l_type == LEOFOLD))
                return(nxtfwd(lp->l_fp));
        else
                return(lp);
}
 

/* Get the next line, this replaces the previous macro,
 * required with the introduction of folding and line
 * types. MJB: 14-Sep-89.
 */
LINE *PASCAL NEAR lforw(register LINE   *lp)
{
        if (lp->l_type == LSOFOLD)
                return(nxtfwd(lp->l_foldp));
        else 
                return(nxtfwd(lp->l_fp));
}


/* Given a line pointer, find the previous one to use.
 * This is only called by lback. MJB: 14-Sep-89.
 */
LINE *PASCAL NEAR nxtbwd(register LINE   *lp)
{
        if (lp->l_type == LEOFOLD)
                return(nxtbwd(lp->l_foldp));
        else 
                return(lp);
}
 

/* Get the previous line, this replaces the previous macro,
 * required with the introduction of folding and line
 * types. MJB: 14-Sep-89.
 */
LINE *PASCAL NEAR lback(register LINE   *lp)
{
        if (lp->l_type == LEOFOLD)
                return(nxtbwd(lp->l_foldp));
        else
                return(nxtbwd(lp->l_bp));
}

/*
 * This routine gets called when a character is changed in place in the current
 * buffer. It updates all of the required flags in the buffer and window
 * system. The flag used is passed as an argument; if the buffer is being
 * displayed in more than 1 window we change EDIT t HARD. Set MODE if the
 * mode line needs to be updated (the "*" has to be set).
 */
PASCAL NEAR void lchange(register int	flag )
{
	register WINDOW *wp;
	SCREEN *scrp;		/* screen to fix pointers in */

	if (curbp->b_nwnd != 1) 		/* Ensure hard. 	*/
		flag = WFHARD;
	if ((curbp->b_flag&BFCHG) == 0) {	/* First change, so	*/
		flag |= WFMODE; 		/* update mode lines.	*/
		curbp->b_flag |= BFCHG;
	}

	/* in all screens.... */
	scrp = first_screen;
	while (scrp) {
		/* make sure all the needed windows get this flag */
		wp = scrp->s_first_window;
		while (wp != NULL) {
			if (wp->w_bufp == curbp)
				wp->w_flag |= flag;
			wp = wp->w_wndp;
		}

		/* next screen! */
		scrp = scrp->s_next_screen;
	}
}

PASCAL NEAR void insspace(	/* insert spaces forward into term */
  int f,
  int n )	/* default flag and numeric argument */

{
	linsert(n, ' ', TRUE);
	backchar(f, n);
}

/*
 * linstr -- Insert a string at the current point
 */

PASCAL NEAR int linstr(char	*instr)
{
	register int status;

	status = TRUE;
	if (instr != NULL)
		while (*instr) {
			status = ((*instr == '\r') ? lnewline(): linsert(1, *instr,TRUE));

			/* Insertion error? */
			if (status != TRUE) {
				mlwrite(TEXT168);
/*                                      "%%Can not insert string" */
				break;
			}
			instr++;
		}
	return(status);
}

 /*
 * indent the contents of a fold
 */
PASCAL NEAR void indentfold(
int     n,
char    c,
int     margmode )
{
        LINE    *olddotp;
        int     olddoto;

	intoindentfold++;
        olddotp = curwp->w_dotp; 
        olddoto = curwp->w_doto;
        curwp->w_dotp = curwp->w_dotp->l_fp; /* jump round fold */
        while (curwp->w_dotp != olddotp->l_foldp->l_fp) {
                /* set offset & let recursion take over  */
                curwp->w_doto = olddoto;
                if (curwp->w_dotp->l_type == LEOFOLD){
                        curwp->w_dotp->l_type = LNORMAL;
                        linsert(n, c, FALSE); /* bodge to allow prefix */
                        curwp->w_dotp->l_type = LEOFOLD;
                }
                else if (curwp->w_dotp->l_type == LEOEFOLD){
                        curwp->w_dotp->l_type = LNORMAL;
                        linsert(n, c, FALSE); /* bodge to allow prefix */
                        curwp->w_dotp->l_type = LEOEFOLD;
                }
                else
                        linsert(n, c, FALSE);

                /* move left margin */
                curwp->w_dotp->l_lmargin += n;

                lchange(WFHARD);
                /* get next line - end of folds are valid */
                if ((curwp->w_dotp->l_type == LSOFOLD) ||
                    (curwp->w_dotp->l_type == LSOEFOLD))
                        curwp->w_dotp = curwp->w_dotp->l_foldp;
                curwp->w_dotp = curwp->w_dotp->l_fp;
        }
        curwp->w_dotp = olddotp;
        curwp->w_doto = olddoto;

	if (!--intoindentfold)
		/* we have moved the left margin of close-fold, so move it back */
		curwp->w_dotp->l_foldp->l_lmargin -= n;
}

/*
 * Insert "n" copies of the character "c" at the current location of dot. In
 * the easy case all that happens is the text is stored in the line. In the
 * hard case, the line has to be reallocated. When the window list is updated,
 * take special care; I screwed it up once. You always update dot in the
 * current window. You update mark, and a dot in another window, if it is
 * greater than the place where you did the insert. Return TRUE if all is
 * well, and FALSE on errors.
 */


#if	PROTO
PASCAL NEAR int linsert(int n, char c, int margmode)
#else
PASCAL NEAR linsert(n, c, margmode)
int	n;
char	c;
int	margmode; /* if true use left margin, else ignore */
#endif

{
	register char	*cp1;
	register char	*cp2;
	register LINE	*lp1;
	register LINE	*lp2;
	register LINE	*lp3;
	register int	doto;
	register int	i;
	register WINDOW *wp;
	SCREEN *scrp;		/* screen to fix pointers in */
	int cmark;		/* current mark */

	if (curbp->b_mode&MDVIEW)	/* don't allow this command if	*/
		return(rdonly());	/* we are in read only mode	*/

	/* a zero insert means do nothing! */
	if (n == 0)
		return(TRUE);

	/* Negative numbers of inserted characters are right out! */
	if (n < 1)
		return(FALSE);

        /* check if before left margin of an open fold */
        if ((curwp->w_dotp->l_type == LNORMAL) && margmode &&
            (curwp->w_doto < curwp->w_dotp->l_lmargin))
                return(beep());

        /* check if attempting to edit a fold symbol. MJB: 20-Sep-89 */
        if ((curwp->w_dotp->l_type == LEOFOLD) ||
            (curwp->w_dotp->l_type == LEOEFOLD)) 
                return(beep());
        if ((curwp->w_dotp->l_type == LSOFOLD) ||
            (curwp->w_dotp->l_type == LSOEFOLD)) {
                if (curwp->w_dotp->l_type == LSOFOLD)
                        i = indx(curwp->w_dotp->l_text, FOLDSYMBOL);
                else
                        i = indx(curwp->w_dotp->l_text, BEGINFOLD);
                if ((curwp->w_doto > i) && 
                    (curwp->w_doto < i + strlen(FOLDSYMBOL)))
                        return(beep()); /* in fold symbol */
                else if ((curwp->w_doto <= i) && (c != ' ') && (c != '\t'))
                        return(beep()); /* non space before fold */
                else if ((curwp->w_doto < i) && (c == '\t') &&
                         margmode)
                                return(beep()); /* tab over margin boundary? */
                else if ((curwp->w_doto <= i) && ((c == ' ') || (c == '\t')))
                                indentfold(n, c, margmode);
        }

	/* mark the current window's buffer as changed */
	lchange(WFEDIT);

	lp1 = curwp->w_dotp;			/* Current line 	*/
	if (lp1 == curbp->b_linep) {		/* At the end: special	*/
		if (curwp->w_doto != 0) {
			mlwrite(TEXT170);
/*                              "bug: linsert" */
			return(FALSE);
		}
		if ((lp2=lalloc(BSIZE(n))) == NULL)	/* Allocate new line	*/
			return(FALSE);
		lp2->l_used = n;
                lp2->l_type = LNORMAL;          /* MJB: 11-Sep-89       */
                lp2->l_foldp = (LINE *)NULL;    /* MJB: 11-Sep-89       */
                lp2->l_lmargin = lp1->l_lmargin;
		lp2->l_omargin = lp1->l_omargin;
		lp3 = lp1->l_bp;		/* Previous line	*/
		lp3->l_fp = lp2;		/* Link in		*/
		lp2->l_fp = lp1;
		lp1->l_bp = lp2;
		lp2->l_bp = lp3;
		for (i=0; i<n; ++i)
			lp2->l_text[i] = c;
		curwp->w_dotp = lp2;
		curwp->w_doto = n;
		return(TRUE);
	}
	doto = curwp->w_doto;			/* Save for later.	*/
	if (lp1->l_used+n > lp1->l_size) {	/* Hard: reallocate	*/
		if ((lp2=lalloc(BSIZE(lp1->l_used+n))) == NULL)
			return(FALSE);
		lp2->l_used    = lp1->l_used+n;
                lp2->l_type    = lp1->l_type;      /* MJB: 11-Sep-89       */
                lp2->l_foldp   = lp1->l_foldp;    /* MJB: 11-Sep-89       */
                lp2->l_lmargin = lp1->l_lmargin;  /* MJB: 16-Oct-89     */
		lp2->l_omargin = lp1->l_omargin;
                if (lp2->l_foldp != (LINE *)NULL) /* MJB: 27-Sep-89     */
                        lp2->l_foldp->l_foldp = lp2;
		cp1 = &lp1->l_text[0];
		cp2 = &lp2->l_text[0];
	    
		while (cp1 != &lp1->l_text[doto])
		  *cp2++ = *cp1++;
	    
		cp2 += n;
		while (cp1 != &lp1->l_text[lp1->l_used])
			*cp2++ = *cp1++;
	    
		lp1->l_bp->l_fp = lp2;
		lp2->l_fp = lp1->l_fp;
		lp1->l_fp->l_bp = lp2;
		lp2->l_bp = lp1->l_bp;
		free((char *) lp1);
	} else {				/* Easy: in place	*/
		lp2 = lp1;			/* Pretend new line	*/
		lp2->l_used += n;
		cp2 = &lp1->l_text[lp1->l_used];
		cp1 = cp2-n;
		while (cp1 != &lp1->l_text[doto])
			*--cp2 = *--cp1;
	}
	for (i=0; i<n; ++i)			/* Add the characters	*/
	  lp2->l_text[doto+i] = c;
	
	/* in all screens.... */
	scrp = first_screen;
	while (scrp) {

		wp = scrp->s_first_window;
		while (wp != NULL) {
			if (wp->w_linep == lp1)
				wp->w_linep = lp2;
			if (wp->w_dotp == lp1) {
				wp->w_dotp = lp2;
				if (wp==curwp || wp->w_doto>doto)
					wp->w_doto += n;
			}
			for (cmark = 0; cmark < NMARKS; cmark++) {
				if (wp->w_markp[cmark] == lp1) {
					wp->w_markp[cmark] = lp2;
					if (wp->w_marko[cmark] > doto)
						wp->w_marko[cmark] += n;
				}
			}
			wp = wp->w_wndp;
		}

		/* next screen! */
		scrp = scrp->s_next_screen;
	}
	return(TRUE);
}

/*
 * Overwrite a character into the current line at the current position
 *
 */

#if	PROTO
PASCAL NEAR int lowrite(char c)
#else
PASCAL NEAR lowrite(c)

char c;		/* character to overwrite on current position */
#endif

{
	if (curwp->w_doto < curwp->w_dotp->l_used &&
		(lgetc(curwp->w_dotp, curwp->w_doto) != '\t' ||
		 (curwp->w_doto) % 8 == 7))
			ldelete(1L, FALSE, FALSE, TRUE);
	return(linsert(1, c, TRUE));
}

/*
 * lover -- Overwrite a string at the current point
 */

PASCAL NEAR int lover(char	*ostr)

{
	register int status = TRUE;

	if (ostr != NULL)
		while (*ostr && status == TRUE) {
			status = ((*ostr == '\r') ? lnewline(): lowrite(*ostr));

			/* Insertion error? */
			if (status != TRUE) {
				mlwrite(TEXT172);
/*                                      "%%Out of memory while overwriting" */
				break;
			}
			ostr++;
		}
	return(status);
}

/*
 * Insert a newline into the buffer at the current location of dot in the
 * current window. The funny ass-backwards way it does things is not a botch;
 * it just makes the last line in the file not a special case. Return TRUE if
 * everything works out and FALSE on error (memory allocation failure). The
 * update of dot and mark is a bit easier then in the above case, because the
 * split forces more updating.
 */
PASCAL NEAR int lnewline()
{
	register char	*cp1;
	register char	*cp2;
	register LINE	*lp1;
	register LINE	*lp2;
        register LINE   *lpx;
	register int	doto;
	register WINDOW *wp;
	SCREEN *scrp;		/* screen to fix pointers in */
	int cmark;		/* current mark */
        int oset;
	int backup;

	if (curbp->b_mode&MDVIEW)	/* don't allow this command if	*/
		return(rdonly());	/* we are in read only mode	*/
        if (curwp->w_doto < curwp->w_dotp->l_lmargin)
                return(beep());

	backup = FALSE;

        if (curwp->w_dotp->l_type != LNORMAL) {
                if ((curwp->w_doto != loffset(curwp->w_dotp)) && 
                    (curwp->w_doto != curwp->w_dotp->l_used))
                        return(beep()); /* Only ends of fold lines. MJB: 20-Sep-89 */
                else if (curwp->w_doto == curwp->w_dotp->l_used) {
                        /* if at end of fold line, we really mean after the
                           fold!, so goto the beginning of the line after the
                           fold. MJB 22-Sep-89 */
                        curwp->w_dotp = lforw(curwp->w_dotp);
			if (curwp->w_dotp->l_type == LEOEFOLD)  /* empty fold */
				curwp->w_doto = loffset(curwp->w_dotp);
			else
                        	curwp->w_doto = curwp->w_dotp->l_lmargin;
			backup = TRUE; /* have to backup in the end */
                }
                else if ((curwp->w_dotp->l_type == LSOFOLD) ||
			 (curwp->w_dotp->l_type == LSOEFOLD)) {
                        /* must be at start of fold symbol and need to */ 
                        /* remove any prefix from WHOLE fold */
                        oset = curwp->w_doto - curwp->w_dotp->l_lmargin;
       	                if (oset) {
               	                curwp->w_doto = curwp->w_dotp->l_lmargin;
                       	        ldelete((long)oset, FALSE, FALSE, FALSE);
                       	}
                }

        }

	lchange(WFHARD);
	lp1  = curwp->w_dotp;			/* Get the address and	*/
	doto = curwp->w_doto;			/* offset of "."	*/
	if ((lp2=lalloc(doto)) == NULL) 	/* New first half line	*/
		return(FALSE);
        lp2->l_type = LNORMAL;  
        lp2->l_foldp = (LINE *)NULL;
	if (lp1->l_type == LEOEFOLD) {
		lpx = lback(lp1); /* pointer to required indentation */
		if (lpx->l_type == LSOEFOLD) /* fold open & empty */
			lp2->l_lmargin = loffset(lpx);
		else
			lp2->l_lmargin = lpx->l_lmargin;
		lp2->l_omargin = lpx->l_omargin;
	}
	else {
	        lp2->l_lmargin = lp1->l_lmargin;
	        lp2->l_omargin = lp1->l_omargin;
	}
        
	cp1 = &lp1->l_text[0];			/* Shuffle text around	*/
	cp2 = &lp2->l_text[0];
	while (cp1 != &lp1->l_text[doto])
		*cp2++ = *cp1++;

	if (lp1->l_type != LEOEFOLD) {
       	        cp2 = &lp1->l_text[lp1->l_lmargin];
	        while (cp1 != &lp1->l_text[lp1->l_used])
        	        *cp2++ = *cp1++;

                lp1->l_used = lp1->l_used - doto + lp1->l_lmargin;
	}

	lp2->l_bp = lp1->l_bp;
	lp1->l_bp = lp2;
	lp2->l_bp->l_fp = lp2;
	lp2->l_fp = lp1;

	/* in all screens.... */
	scrp = first_screen;
	while (scrp) {

		wp = scrp->s_first_window;
		while (wp != NULL) {
			if (wp->w_linep == lp1)
				wp->w_linep = lp2;
			if (wp->w_dotp == lp1) {
				if ((wp->w_doto < doto) || backup) {
					wp->w_dotp = lp2;
                                wp->w_doto = lp2->l_lmargin;
				}
                        else if (lp1->l_type != LEOEFOLD)
                                wp->w_doto -= (doto - lp1->l_lmargin);
			}
			for (cmark = 0; cmark < NMARKS; cmark++) {
				if (wp->w_markp[cmark] == lp1) {
					if (wp->w_marko[cmark] < doto)
						wp->w_markp[cmark] = lp2;
					else
						wp->w_marko[cmark] -= doto;
				}
			}
			wp = wp->w_wndp;
		}

		/* next screen! */
		scrp = scrp->s_next_screen;
	}
	return(TRUE);
}

/*
 * Undent the contents of a fold
 */
PASCAL NEAR void undentfold(
long    n,
int     i,
int     kflag,
int     rawmode,
int     margmode )
{
        LINE    *olddotp;
        int     olddoto;

	intoundentfold++;
        olddotp = curwp->w_dotp; 
        olddoto = curwp->w_doto;
        curwp->w_dotp = curwp->w_dotp->l_fp; /* jump round fold */
        while (curwp->w_dotp != olddotp->l_foldp->l_fp) {

                /* move left margin */
                curwp->w_dotp->l_lmargin -= (int)n;

                /* set offset & let recursion take over  */
                curwp->w_doto = olddoto;
                if (curwp->w_dotp->l_type == LEOFOLD) {
                        curwp->w_dotp->l_type = LNORMAL;
                        if ((i = loffset(curwp->w_dotp)) > n)
                                i = (int)n;
                        ldelete((long)i, kflag, rawmode, FALSE);
                        curwp->w_dotp->l_type = LEOFOLD;
                }
                else if (curwp->w_dotp->l_type == LEOEFOLD) {
                        curwp->w_dotp->l_type = LNORMAL;
                        if ((i = loffset(curwp->w_dotp)) > n)
                                i = (int)n;
                        ldelete((long)i, kflag, rawmode, FALSE);
                        curwp->w_dotp->l_type = LEOEFOLD;
                }
                else {
                        if ((i = loffset(curwp->w_dotp)) > n)
                                i = (int)n;
                        ldelete((long)i, kflag, rawmode, FALSE);
                }

                lchange(WFHARD);
                /* get next line - end of folds are valid */
                if ((curwp->w_dotp->l_type == LSOFOLD) ||
                    (curwp->w_dotp->l_type == LSOEFOLD))
                        curwp->w_dotp = curwp->w_dotp->l_foldp;
                curwp->w_dotp = curwp->w_dotp->l_fp;
        }
        curwp->w_dotp = olddotp;
        curwp->w_doto = olddoto;

	if (!--intoundentfold)
		/* we have moved the left margin of close-fold, so move it back */
		curwp->w_dotp->l_foldp->l_lmargin += (int)n;
}

/*

LDELETE:

	This function deletes "n" bytes, starting at dot. Positive n
deletes forward, negative n deletes backwords. It understands how to
deal with end of lines, and with two byte characters. It returns TRUE
if all of the characters were deleted, and FALSE if they were not
(because dot ran into the buffer end). The "kflag" is TRUE if the text
should be put in the kill buffer.

*/

PASCAL NEAR int ldelete(
long n, 	/* # of chars to delete */
int kflag,	/* put killed text in kill buffer flag */
int rawmode,     /* treat folds like normal lines */
int margmode )   /* don't allow deletion to left of margin */

{
	register char	*cp1;
	register char	*cp2;
	register LINE	*dotp;
	register int	doto;
	register int	chunk;
	register WINDOW *wp;
        int i = 0;              /* MJB: 20-Sep-89 */
	int cmark;		/* current mark */

	if (curbp->b_mode&MDVIEW)	/* don't allow this command if	*/
		return(rdonly());	/* we are in read only mode	*/

        /* check if before left margin of an open fold */
        if ((curwp->w_dotp->l_type == LNORMAL) && margmode &&
            (curwp->w_doto < curwp->w_dotp->l_lmargin))
                return(beep());

        /* check if attempting to edit a fold symbol. MJB: 20-Sep-89 */
        if (((curwp->w_dotp->l_type == LEOFOLD) ||
             (curwp->w_dotp->l_type == LEOEFOLD)) && !rawmode)
                return(beep());
        if (((curwp->w_dotp->l_type == LSOFOLD) ||
             (curwp->w_dotp->l_type == LSOEFOLD)) && !rawmode) {
                if (curwp->w_dotp->l_type == LSOFOLD)
                        i = indx(curwp->w_dotp->l_text, FOLDSYMBOL);
                else if (curwp->w_dotp->l_type == LSOEFOLD)
                        i = indx(curwp->w_dotp->l_text, BEGINFOLD);
                if ((curwp->w_doto >= i) && 
                    (curwp->w_doto < i + strlen(FOLDSYMBOL)))
                        return(beep()); /* in fold symbol */
                else if (curwp->w_doto + n > curwp->w_dotp->l_used)
                        return(beep()); /* would cause line wrap */
                else if ((curwp->w_doto < i) && (curwp->w_doto + n > i))
                        n = (long)i - curwp->w_doto; /* would get into fold symbol */

                if ((n > 0) && (curwp->w_doto + n <= i)) 
                        undentfold(n, i, kflag, rawmode, margmode);
        }

	/* going Forward? */
	if (n >= 0) {

		while (n > 0) {
#if	DBCS
			/* never start forward on a 2 byte char */
			if (curwp->w_doto > 0 && is2byte(curwp->w_dotp->l_text,
			    &curwp->w_dotp->l_text[curwp->w_doto - 1])) {
				curwp->w_doto--;
				n++;
			}
#endif
			/* record the current point */
			dotp = curwp->w_dotp;
			doto = curwp->w_doto;
	
			/* can't delete past the end of the buffer */
			if (dotp == curbp->b_linep)
				return(FALSE);
	
			/* find out how many chars to delete on this line */
			chunk = dotp->l_used-doto;	/* Size of chunk.	*/
			if (chunk > n)
				chunk = (int)n;
	
			/* if at the end of a line, merge with the next */
			if (chunk == 0) {
	
				/* flag that we are making a hard change */
				lchange(WFHARD);
	                        /* last newline is special case in rawmode */
        	                if (ldelnewline(rawmode && (n > 1)) == FALSE
				    || (kflag != FALSE &&
				     kinsert(FORWARD, '\r')==FALSE))
					return(FALSE);
				--n;
				continue;
			}
	
			/* flag the fact we are changing the current line */
			lchange(WFEDIT);
	
			/* find the limits of the kill */
			cp1 = &dotp->l_text[doto];
			cp2 = cp1 + chunk;
#if	DBCS
			/* never leave half a character */
			if (is2byte(dotp->l_text, cp2 - 1)) {
				++chunk;
				++cp2;
				++n;
			}
#endif

			/* save the text to the kill buffer */
			if (kflag != FALSE) {
				while (cp1 != cp2) {
					if (kinsert(FORWARD, *cp1) == FALSE)
						return(FALSE);
					++cp1;
				}
				cp1 = &dotp->l_text[doto];
			}
	
			/* copy what is left of the line upward */
			while (cp2 != &dotp->l_text[dotp->l_used])
				*cp1++ = *cp2++;

			dotp->l_used -= chunk;
			/* fix any other windows with the same text displayed */
			wp = wheadp;
			while (wp != NULL) {
	
				/* reset the dot if needed */
				if (wp->w_dotp==dotp && wp->w_doto>=doto) {
					wp->w_doto -= chunk;
					if (wp->w_doto < doto)
						wp->w_doto = doto;
	                                if (margmode)
        	                                marginchk(wp->w_dotp,
							  loffset(wp->w_dotp));

				}
	
				/* reset any marks if needed */
				for (cmark = 0; cmark < NMARKS; cmark++) {
					if (wp->w_markp[cmark]==dotp && wp->w_marko[cmark]>=doto) {
						wp->w_marko[cmark] -= chunk;
						if (wp->w_marko[cmark] < doto)
							wp->w_marko[cmark] = doto;
					}
				}
	
				/* onward to the next window */
				wp = wp->w_wndp;
			}
	
			/* indicate we have deleted chunk characters */
			n -= chunk;
		}
	} else {
		while (n < 0) {
#if	DBCS
			/* never start backwards on the
			   1st of a 2 byte character */
			if (curwp->w_doto > 1 && is2byte(curwp->w_dotp->l_text,
			    &curwp->w_dotp->l_text[curwp->w_doto-1])) {
				curwp->w_doto++;
				n--;
			}
#endif
			/* record the current point */
			dotp = curwp->w_dotp;
			doto = curwp->w_doto;
	
			/* can't delete past the beginning of the buffer */
			if (dotp == lforw(curbp->b_linep) && (doto == 0))
				return(FALSE);
	
			/* find out how many chars to delete on this line */
			chunk = doto;		/* Size of chunk.	*/
			if (chunk > -n)
				chunk = (int)-n;
	
			/* if at the beginning of a line, merge with the last */
			if (chunk == 0) {
	
				/* flag that we are making a hard change */
				lchange(WFHARD);
				backchar(TRUE, 1);
	                        if (ldelnewline(rawmode && (n > 1)) == FALSE
				    || (kflag != FALSE &&
				     kinsert(BACKWARD, '\r')==FALSE))
					return(FALSE);
				++n;
				continue;
			}
	
			/* flag the fact we are changing the current line */
			lchange(WFEDIT);
	
			/* find the limits of the kill */
			cp1 = &dotp->l_text[doto];
			cp2 = cp1 - chunk;
#if	DBCS
			if (is2byte(dotp->l_text, cp2 - 1)) {
				++chunk;
				--cp2;
				++n;
			}
#endif
	
			/* save the text to the kill buffer */
			if (kflag != FALSE) {
				while (cp1 > cp2) {
					if (kinsert(BACKWARD, *(--cp1)) == FALSE)
						return(FALSE);
				}
				cp1 = &dotp->l_text[doto];
			}
	
			/* copy what is left of the line downward */
			while (cp1 != &dotp->l_text[dotp->l_used])
				*cp2++ = *cp1++;
			dotp->l_used -= chunk;
			curwp->w_doto -= chunk;
	
			/* fix any other windows with the same text displayed */
			wp = wheadp;
			while (wp != NULL) {
	
				/* reset the dot if needed */
				if (wp->w_dotp==dotp && wp->w_doto>=doto) {
					wp->w_doto -= chunk;
					if (wp->w_doto < doto)
						wp->w_doto = doto;
	                                if (margmode)
        	                                marginchk(wp->w_dotp,
							  loffset(wp->w_dotp));

				}
	
				/* reset any marks if needed */
				for (cmark = 0; cmark < NMARKS; cmark++) {
					if (wp->w_markp[cmark]==dotp && wp->w_marko[cmark]>=doto) {
						wp->w_marko[cmark] -= chunk;
						if (wp->w_marko[cmark] < doto)
							wp->w_marko[cmark] = doto;
					}
				}
	
				/* onward to the next window */
				wp = wp->w_wndp;
			}
	
			/* indicate we have deleted chunk characters */
			n += chunk;
		}
	}
	return(TRUE);
}

/* getctext:	grab and return a string with the text of
		the current line
*/

char *PASCAL NEAR getctext()

{
	register LINE *lp;	/* line to copy */
	register int size;	/* length of line to return */
	register char *sp;	/* string pointer into line */
	register char *dp;	/* string pointer into returned line */
	char rline[NSTRING];	/* line to return */

	/* find the contents of the current line and its length */
	lp = curwp->w_dotp;
	sp = lp->l_text;
	size = lp->l_used;
	if (size >= NSTRING)
		size = NSTRING - 1;

	/* copy it across */
	dp = rline;
	while (size--)
		*dp++ = *sp++;
	*dp = 0;
	return(rline);
}

/* putctext:	replace the current line with the passed in text	*/

PASCAL NEAR int putctext(
char *iline )	/* contents of new line */

{
	register int status;

	/* delete the current line */
	curwp->w_doto = 0;	/* starting at the beginning of the line */
	if ((status = killtext(TRUE, 1)) != TRUE)
		return(status);

	/* insert the new line */
	if ((status = linstr(iline)) != TRUE)
		return(status);
	status = lnewline();
	backline(TRUE, 1, FALSE);
	return(status);
}

/*
 * Delete a newline. Join the current line with the next line. If the next line
 * is the magic header line always return TRUE; merging the last line with the
 * header line can be thought of as always being a successful operation, even
 * if nothing is done, and this makes the kill buffer work "right". Easy cases
 * can be done by shuffling data around. Hard cases require that lines be moved
 * about in memory. Return FALSE on error and TRUE if all looks ok. Called by
 * "ldelete" only.
 */
PASCAL NEAR int ldelnewline(
int     rawmode )        /* ignore line types if true. MJB: 29-Sep-89 */
{
	register char	*cp1;
	register char	*cp2;
	register LINE	*lp1;
	register LINE	*lp2;
	register LINE	*lp3;
	register WINDOW *wp;
	SCREEN *scrp;		/* screen to fix pointers in */
	int cmark;		/* current mark */

	if (curbp->b_mode&MDVIEW)	/* don't allow this command if	*/
		return(rdonly());	/* we are in read only mode	*/

        if ((curwp->w_dotp->l_type != LNORMAL) && !rawmode) /* don't allow for folds */
                return(beep());               /* MJB: 20-Sep-89        */

	lp1 = curwp->w_dotp;
	lp2 = lp1->l_fp;

        if ((lp2->l_type != LNORMAL) && (lp1->l_used > lp1->l_lmargin) && !rawmode)
                return(beep()); /* can't concatenate lines. MJB: 22-Sep-89 */

        if ((lp2->l_type != LNORMAL) && (lp1->l_used <= lp1->l_lmargin) && !rawmode)
                lp1->l_used = 0;

	if (lp2 == curbp->b_linep) {		/* At the buffer end.	*/
		if (lp1->l_used == 0)		/* Blank line.		*/
			lfree(lp1);
		return(TRUE);
	}
	if (lp2->l_used <= lp1->l_size-lp1->l_used) {
		cp1 = &lp1->l_text[lp1->l_used];
                cp2 = &lp2->l_text[(rawmode || (lp2->l_type != LNORMAL)) ? 
				   0 : lp2->l_lmargin];
		while (cp2 != &lp2->l_text[lp2->l_used])
		*cp1++ = *cp2++;

		/* in all screens.... */
		scrp = first_screen;
		while (scrp) {

			wp = scrp->s_first_window;
			while (wp != NULL) {
				if (wp->w_linep == lp2)
					wp->w_linep = lp1;
				if (wp->w_dotp == lp2) {
					wp->w_dotp  = lp1;
					wp->w_doto += lp1->l_used;
				}
				for (cmark = 0; cmark < NMARKS; cmark++) {
					if (wp->w_markp[cmark] == lp2) {
						wp->w_markp[cmark]  = lp1;
						wp->w_marko[cmark] += lp1->l_used;
					}
				}
				wp = wp->w_wndp;
			}

			/* next screen! */
			scrp = scrp->s_next_screen;
		}

                /* set line type, and if necessary fold pointers. MJB: 22-Sep-89 */
                if ((lp2->l_type != LNORMAL) && !rawmode) {
                        lp1->l_type = lp2->l_type;
                        lp1->l_foldp = lp2->l_foldp;
                        lp1->l_foldp->l_foldp = lp1;
/*			lp1->l_lmargin = lp2->l_lmargin;*/
			lp1->l_lmargin = minleftmarg(lp1);
                }
                else
                        lp1->l_type = LNORMAL;

                lp1->l_used += (lp2->l_used - 
				((rawmode || (lp2->l_type != LNORMAL)) ? 
				0 : lp2->l_lmargin));
                lp1->l_fp = lp2->l_fp;
                lp2->l_fp->l_bp = lp1;
                free((char *) lp2);
                return(TRUE);
	}

        if ((lp3=lalloc(lp1->l_used + lp2->l_used - 
		        ((rawmode || (lp2->l_type != LNORMAL)) ? 
			0 : lp2->l_lmargin))) == NULL)
		return(FALSE);

        /* set line type, and if necessary fold pointers. MJB: 22-Sep-89 */
        if (rawmode)
                lp3->l_type = LNORMAL;
        else {
                lp3->l_type = lp2->l_type;
                if ((lp3->l_foldp = lp2->l_foldp) != (LINE *) NULL)
                        lp3->l_foldp->l_foldp = lp3;
        }

	cp1 = &lp1->l_text[0];
	cp2 = &lp3->l_text[0];
	while (cp1 != &lp1->l_text[lp1->l_used])
		*cp2++ = *cp1++;
        cp1 = &lp2->l_text[(rawmode || (lp2->l_type != LNORMAL)) ? 
			   0 : lp2->l_lmargin];
	while (cp1 != &lp2->l_text[lp2->l_used])
		*cp2++ = *cp1++;
	lp1->l_bp->l_fp = lp3;
	lp3->l_fp = lp2->l_fp;
	lp2->l_fp->l_bp = lp3;
	lp3->l_bp = lp1->l_bp;
	if (rawmode)
		lp3->l_lmargin = 0;
	else {
	        lp3->l_lmargin = minleftmarg(lp3);
        	marginchk(lp3, loffset(lp3));
	}

	/* in all screens.... */
	scrp = first_screen;
	while (scrp) {

		wp = scrp->s_first_window;
		while (wp != NULL) {
			if (wp->w_linep==lp1 || wp->w_linep==lp2)
				wp->w_linep = lp3;
			if (wp->w_dotp == lp1)
				wp->w_dotp  = lp3;
			else if (wp->w_dotp == lp2) {
				wp->w_dotp  = lp3;
				wp->w_doto += lp1->l_used;
			}
			for (cmark = 0; cmark < NMARKS; cmark++) {
				if (wp->w_markp[cmark] == lp1)
					wp->w_markp[cmark]  = lp3;
				else if (wp->w_markp[cmark] == lp2) {
					wp->w_markp[cmark]  = lp3;
					wp->w_marko[cmark] += lp1->l_used;
				}
			}
			wp = wp->w_wndp;
		}

		/* next screen! */
		scrp = scrp->s_next_screen;
	}

	free((char *) lp1);
	free((char *) lp2);
	return(TRUE);
}

/*	Add a new line to the end of the indicated buffer.
	return FALSE if we run out of memory
	note that this works on non-displayed buffers as well!
*/

#if	PROTO
int PASCAL NEAR addline(BUFFER *bp, char *text)
#else
int PASCAL NEAR addline(bp, text)

BUFFER *bp;	/* buffer to add text to */
char *text;	/* line to add */
#endif
{
	register LINE	*lp;
	register int	i;
	register int	ntext;

	/* allocate the memory to hold the line */
	ntext = strlen(text);
	if ((lp=lalloc(ntext)) == NULL)
		return(FALSE);
	lp->l_type = LNORMAL;		/* MJB: 11-Sep-89 */
	lp->l_foldp = (LINE *)NULL;	/* MJB: 11-Sep-89 */
	/* copy the text into the new line */
	for (i=0; i<ntext; ++i)
		lputc(lp, i, text[i]);

	/* add the new line to the end of the buffer */
	bp->b_linep->l_bp->l_fp = lp;
	lp->l_bp = bp->b_linep->l_bp;
	bp->b_linep->l_bp = lp;
	lp->l_fp = bp->b_linep;

	/* if the point was at the end of the buffer,
	   move it to the beginning of the new line */
	if (bp->b_dotp == bp->b_linep)
		bp->b_dotp = lp;
	return(TRUE);
}

/*
 * Delete all of the text saved in the kill buffer. Called by commands when a
 * new kill context is being created. The kill buffer array is released, just
 * in case the buffer has grown to immense size. No errors.
 */
PASCAL NEAR void kdelete()
{
	KILL *kp;	/* ptr to scan kill buffer chunk list */

	if (kbufh != NULL) {

		/* first, delete all the chunks */
		kbufp = kbufh;
		while (kbufp != NULL) {
			kp = kbufp->d_next;
			free((char *)kbufp);
			kbufp = kp;
		}

		/* and reset all the kill buffer pointers */
		kbufh = kbufp = NULL;
		kskip = 0;
		kused = KBLOCK; 	        
	}

	killfoldbound = FALSE;
}

/*
 * Insert a character to the kill buffer, allocating new chunks as needed.
 * Return TRUE if all is well, and FALSE on errors.
 */

#if	PROTO
PASCAL NEAR int kinsert(int direct, char c)
#else
PASCAL NEAR kinsert(direct, c)

int direct;	/* direction (FORWARD/BACKWARD) to insert characters */
char c;		/* character to insert in the kill buffer */
#endif

{
	KILL *nchunk;	/* ptr to newly malloced chunk */

	if (direct == FORWARD) {

		/* check to see if we need a new chunk */
		if (kused >= KBLOCK) {
			if ((nchunk = (KILL *)malloc(sizeof(KILL))) == NULL)
				return(FALSE);
			if (kbufh == NULL)	/* set head ptr if first time */
				kbufh = nchunk;
			if (kbufp != NULL)	/* point the current to this new one */
				kbufp->d_next = nchunk;
			kbufp = nchunk;
			kbufp->d_next = NULL;
			kused = 0;
		}
	
		/* and now insert the character */
		kbufp->d_chunk[kused++] = c;
	} else {
		/* BACKWARDS */
		/* check to see if we need a new chunk */
		if (kskip == 0) {
			if ((nchunk = (KILL *)malloc(sizeof(KILL))) == NULL)
				return(FALSE);
			if (kbufh == NULL) {	/* set head ptr if first time */
				kbufh = nchunk;
				kbufp = nchunk;
				kskip = KBLOCK;
				kused = KBLOCK;
				nchunk->d_next = (KILL *)NULL;
			} else {
				nchunk->d_next = kbufh;
				kbufh = nchunk;
				kskip = KBLOCK;
			}
		}
	
		/* and now insert the character */
		kbufh->d_chunk[--kskip] = c;
	}
	return(TRUE);
}

/*
 * Yank text back from the kill buffer. This is really easy. All of the work
 * is done by the standard insert routines. All you do is run the loop, and
 * check for errors. Bound to "C-Y".
 * After the yank, parse the region to check for folds, and set line types
 * and pointers as required. Also add indentation if yanking into open fold.
 */

#define	Char_insert(a)	(a == '\r' ? lnewline() : linsert(1, a, FALSE))

PASCAL NEAR int yank(
  int f,
  int n )	/* prefix flag and argument */

{
	register int counter;	/* counter into kill buffer data */
	register char *sp;	/* pointer into string to insert */
	KILL *kptr;		/* pointer into kill buffer */
	LINE *curline;
        LINE *lp1;              /* pointers within region */
	int  margin, marg;      /* margins within yanked region */
	int curoff;		/* storage for line before yanking */

	if (curbp->b_mode&MDVIEW)	/* don't allow this command if	*/
		return(rdonly());	/* we are in read only mode	*/

        if ((curwp->w_dotp->l_type != LNORMAL) && killedcr) /* don't yank onto fold lines */
                return(beep());               		    /* MJB: 20-Sep-89             */

	if (n < 0)
		return(FALSE);
	/* make sure there is something to yank */
	if (kbufh == NULL)
		return(TRUE);		/* not an error, just nothing */

        if ((curwp->w_doto < curwp->w_dotp->l_lmargin) || killfoldbound)
                curwp->w_doto = curwp->w_dotp->l_lmargin;

	/*
	 * Save the local pointers to hold global ".".
	 */
	/* Find the *previous* line, since the line we are on
	 * may disappear due to re-allocation.  This works even
	 * if we are on the first line of the file.
	 */
	curline = curwp->w_dotp->l_bp;
	curoff = curwp->w_doto;

	/* for each time.... */
	while (n--) {
		if (kskip > 0) {
			kptr = kbufh;
			sp = &(kptr->d_chunk[kskip]);
			counter = kskip;
			while (counter++ < KBLOCK) {
				Char_insert(*sp);
				++sp;
			}
			kptr = kptr->d_next;
		} else {
			kptr = kbufh;
		}
		if (kptr != (KILL *)NULL) {
			while (kptr != kbufp) {
				sp = kptr->d_chunk;
				for(counter = 0; counter < KBLOCK; counter++) {
					Char_insert(*sp);
					++sp;
				}
				kptr = kptr->d_next;
			}
			counter = kused;
			sp = kptr->d_chunk;
			while (counter--) {
				Char_insert(*sp);
				++sp;
			}
		}
	}

        /* parse region to see if we yanked any folds. MJB: 26-Sep-89 */
        lp1 = curline->l_fp;
	if (curline->l_type == LSOEFOLD)
		margin = loffset(curline);
	else
		margin = curline->l_lmargin;

        while (lp1 != curwp->w_dotp) {
		lp1->l_lmargin = margin;
		if (lp1->l_used != 0)
		{
	                if ((marg = tindx(lp1->l_text, FOLDSYMBOL, lp1->l_used)) != -1) {
        	                lp1->l_type = LSOFOLD;
                	        pushline(lp1); 
				margin = marg;
	                }
        	        else if ((marg = tindx(lp1->l_text, BEGINFOLD, lp1->l_used)) != -1) {
                	        lp1->l_type = LSOEFOLD;
                        	pushline(lp1); 
				margin = marg;
        	        }
                	else if ((marg = tindx(lp1->l_text, ENDFOLD, lp1->l_used)) != -1) {
				if (linelist->fll_bp->fll_line != (LINE *)NULL) {
	                        	lp1->l_foldp = popline();
	        	                lp1->l_foldp->l_foldp = lp1;
					margin = lp1->l_foldp->l_lmargin;
					lp1->l_lmargin = margin; /* was too right */
                		        if (lp1->l_foldp->l_type == LSOFOLD)
                        		        lp1->l_type = LEOFOLD;
		                        else
       	 		                        lp1->l_type = LEOEFOLD;
				}
				else { /* Just to be safe! */
					lp1->l_type = LNORMAL;
					lp1->l_foldp = (LINE *)NULL;
					mlwrite(TEXT241);
/*					"Missing start-fold Marker - */
				}
	                }
		}
                lp1 = lp1->l_fp;
        }

	/* check to see if matching number of start/end fold markers */
	/* if not try to tidy up a bit. MJB: 13-Nov-89 */
	while (linelist->fll_bp->fll_line != (LINE *)NULL) {
		lp1 = popline();
		lp1->l_type = LNORMAL;
		mlwrite(TEXT240);
/*			    "Missing end-fold Marker" */
        }

	/* If requested, set global "." back to the
	 * beginning of the yanked text.
	 */
	if (yankflag) {
		curwp->w_dotp = curline->l_fp;
		curwp->w_doto = curoff;
	}
	return(TRUE);
}

#if	0
dispkill()

{
	KILL *kptr;
	int index;
	char *sp;
	int counter;

	if (kbufh == (KILL *)NULL) {
		printf("<EMPTY>\n");
		return;
	}

	index = 1;
	if (kskip > 0) {
		kptr = kbufh;
		printf("kskip = %d\nBLOCK %d <", kskip, index++);
		sp = &(kptr->d_chunk[kskip]);
		counter = kskip;
		while (counter++ < KBLOCK) {
			putchar(*sp++);
		}
		printf(">\n");
		kptr = kptr->d_next;
	} else {
		kptr = kbufh;
	}

	if (kptr != (KILL *)NULL) {
		while (kptr != kbufp) {
			printf("BLOCK %d <%255s>\n", index++, kptr->d_chunk);
			kptr = kptr->d_next;
		}
		printf("BLOCK %d <", index++);
		counter = kused;
		sp = kptr->d_chunk;
		while (counter--) {
			putchar(*sp++);
		}
		printf(">\nkused = %d\n", kused);
	}

}
#endif


/* Initialise the linelist to hold start-fold lines.
 * MJB: 15-Sep-89.
 */
PASCAL NEAR int initlinelist()
{
        if ((linelist = (FOLDLINELIST *)malloc(sizeof(FOLDLINELIST))) == NULL) {
                mlwrite(TEXT99);
                /* "[OUT OF MEMORY]" */
                return(NULL);
        }
        linelist->fll_fp = linelist;
        linelist->fll_bp = linelist;
        linelist->fll_line = (LINE *)NULL;
        return(TRUE);
}


/* Add a line to the list of start-fold lines.
 * MJB: 15-Sep-89.
 */
PASCAL NEAR int pushline(LINE *lp)
{
        struct FOLDLINELIST *lele;
        struct FOLDLINELIST *lele2;

        if ((lele = (FOLDLINELIST *)malloc(sizeof(FOLDLINELIST))) == NULL) {
                mlwrite(TEXT99);
                /* "[OUT OF MEMORY]" */
                return(NULL);
        }
        lele->fll_line = lp;
        lele2 = linelist->fll_bp;
        lele2->fll_fp = lele;
        lele->fll_fp = linelist;
        lele->fll_bp = lele2;
        linelist->fll_bp = lele;
        lele->fll_line = lp;
        return(TRUE);
}


LINE *PASCAL NEAR popline()
{
        struct FOLDLINELIST     *lele;
        struct LINE             *lp;

        lele = linelist->fll_bp;
        lele->fll_bp->fll_fp = lele->fll_fp;
        lele->fll_fp->fll_bp = lele->fll_bp;
        lp = lele->fll_line;
        free((FOLDLINELIST *) lele);
        return(lp);
}
