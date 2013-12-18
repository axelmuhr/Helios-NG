/*
 * The routines in this file deal with folding.
 */

/*	Modifications:
 	11-Sep-89	Mike Burrow (INMOS)	Original.
 */

#include        <stdio.h>
#include	"estruct.h"
#include	"eproto.h"
#include        "edef.h"
#include	"elang.h"

/*	openfold:	Open a fold at the current dot position.
*/

PASCAL NEAR openfold(
  int f,
  int n )
{
	BUFFER *bp;		 /* buffer being folded*/
	WINDOW *wp;		 /* windows to fix up pointers in as well */
	int    i, j;
	char   tmpstr[NSTRING];

	/* find the proper buffer */
	bp = curwp->w_bufp;		

	if (curwp->w_dotp->l_type == LSOFOLD) {
		/* Change line types */
		curwp->w_dotp->l_type = LSOEFOLD;
		curwp->w_dotp->l_foldp->l_type = LEOEFOLD;

		i = indx(curwp->w_dotp->l_text, FOLDSYMBOL);
		
		strcpy(tmpstr, BEGINFOLD);
		for (j = 0; j < strlen(FOLDSYMBOL); j ++)
			curwp->w_dotp->l_text[i + j] = tmpstr[j];

		/* Move cursor to the beginning of next line */
		curwp->w_dotp = lforw(curwp->w_dotp);
		curwp->w_doto = curwp->w_dotp->l_lmargin;

		/* let all the proper windows be updated */
		wp = wheadp;
		while (wp) {
			if (wp->w_bufp == bp)
				wp->w_flag |= (WFHARD|WFMODE|WFFORCE);
			wp = wp->w_wndp;
		}
		mlwrite(TEXT228);
		/* "[Fold Opened]" */
		return(TRUE);
	}
	else {
		mlwrite(TEXT235);
		/* "%%Line is not a fold" */
		return(FALSE);
	}
}


/*	closefold:	Close the fold at the current dot position.
*/

PASCAL NEAR closefold(
  int f,
  int n )
{
	BUFFER *bp;		 /* buffer being folded */
	WINDOW *wp;		 /* windows to fix up pointers in as well */
	LINE   *lp, *lp2;	 /* lines to search for fold markers */
	int    i, j, foldcnt;
	char   tmpstr[NSTRING];

	/* find the proper buffer */
	bp = curwp->w_bufp;		

	foldcnt = 0; /* number of nested folds */

	/* search for the start of the fold, skipping nested folds */
	if (curwp->w_dotp->l_type == LEOEFOLD)
		lp = curwp->w_dotp->l_foldp;
	else {
		lp = curwp->w_dotp;
		while (((lp->l_type != LSOEFOLD) || (foldcnt > 0)) && 
		        (lp != bp->b_linep)) {
			if ((lp->l_type == LEOFOLD) || (lp->l_type == LEOEFOLD))
				foldcnt++;
			else if (((lp->l_type == LSOFOLD) || 
				  (lp->l_type == LSOEFOLD)) && (foldcnt > 0))
				foldcnt--;
			lp = lp->l_bp;
		}
	}

	/* close the fold */
	if ((lp->l_type == LSOEFOLD) && (foldcnt == 0)) {
		lp->l_type = LSOFOLD;
		lp->l_foldp->l_type = LEOFOLD;

		i = indx(lp->l_text, BEGINFOLD);

		if (i == -1)	     	/* XXX - NC - 24/6/92 */
		  {
		    mlwrite(TEXT236);
		    /* "%%Line is not within a fold" */
		    return(FALSE);
		  }
		
		strcpy(tmpstr, FOLDSYMBOL);

		for (j = 0; j < strlen(FOLDSYMBOL); j ++)
			lp->l_text[i + j] = tmpstr[j];

		/* close any inner folds */
		lp2 = lp->l_foldp;
		while (lp2 != lp) {
			if (lp2->l_type == LSOEFOLD){
				lp2->l_type = LSOFOLD;
				i = indx(lp2->l_text, BEGINFOLD);
				for (j = 0; j < strlen(FOLDSYMBOL); j ++)
					lp2->l_text[i + j] = tmpstr[j];
			}
			else if (lp2->l_type == LEOEFOLD)
				lp2->l_type = LEOFOLD;
			lp2 = lp2->l_bp;
		}

		/* Move cursor to the beginning of fold line */
		curwp->w_dotp = lp;
		curwp->w_doto = loffset(curwp->w_dotp);

		/* let all the proper windows be updated */
		wp = wheadp;
		while (wp) {
			if (wp->w_bufp == bp) {
				wp->w_flag |= (WFHARD|WFMODE|WFFORCE);
				/* enter all windows containing fold */
				wp->w_linep = curwp->w_linep;
				wp->w_dotp = curwp->w_dotp;
				wp->w_doto = curwp->w_doto;
			}
			wp = wp->w_wndp;
		}
		mlwrite(TEXT229);
		/* "[Fold Closed]" */
		return(TRUE);
	}
	else {
		mlwrite(TEXT236);
		/* "%%Line is not within a fold" */
		return(FALSE);
	}
}


/*	enterfold:	Enter a fold at the current dot position.
*/

PASCAL NEAR enterfold(
  int f,
  int n )
{
	BUFFER *bp;		 /* buffer being entered */
	WINDOW *wp;		 /* windows to fix up pointers in as well */
	int    wscnt;		 /* whitespace count */
	LINE   *lp;		 /* line loop value */
	int    chgflag;		 /* record previous value of change flag */
	int    vewflag;		 /* record previous value of view flag */
	int    i;

	/* find the proper buffer */
	bp = curwp->w_bufp;		

	if (curwp->w_dotp->l_type == LSOFOLD) {

		/* remove indentation and stack previous buffer values */
		if (bp->b_nfolds < NFOLDS) {

			/* remove indentation from fold */

			/* 1st find out how much to remove */
			wscnt = loffset(curwp->w_dotp);

			/* and then remove them */
			if (wscnt > 0) {

				/* record if buffer changed yet since this will */
				chgflag = bp->b_flag & BFCHG;
				vewflag = bp->b_mode & MDVIEW; /* no good in view mode */
				bp->b_mode &= ~MDVIEW;

				lp = curwp->w_dotp; /* save old value */

				curwp->w_dotp = lp->l_fp;
				while (curwp->w_dotp != lp->l_foldp) {
					curwp->w_doto = 0;
					curwp->w_dotp->l_omargin = curwp->w_dotp->l_lmargin;
					curwp->w_dotp->l_lmargin = 0;
					/* safety 1st - but shouldn't be required */
					if (loffset(curwp->w_dotp) >= wscnt)
						ldelete((long)wscnt, FALSE,FALSE,FALSE);
						
					if (curwp->w_dotp->l_type == LSOFOLD)
						curwp->w_dotp = curwp->w_dotp->l_foldp;
					curwp->w_dotp = curwp->w_dotp->l_fp;
				}
				curwp->w_dotp = lp;

				/* reset change flag? */
				if (!chgflag)
					bp->b_flag &= ~BFCHG;
				if (vewflag) /* reset view flag */
					bp->b_mode |= MDVIEW;
			}
			else { /* just set old margin value to zero - or problem with exit */
				lp = curwp->w_dotp; /* save old value */
				curwp->w_dotp = lp->l_fp;
				while (curwp->w_dotp != lp->l_foldp) {
					curwp->w_dotp->l_omargin = 0;
					curwp->w_dotp->l_lmargin = 0;
					if (curwp->w_dotp->l_type == LSOFOLD)
						curwp->w_dotp = curwp->w_dotp->l_foldp;
					curwp->w_dotp = curwp->w_dotp->l_fp;
				}
				curwp->w_dotp = lp;
			}

			/* stack values */
			bp->b_folds[bp->b_nfolds].f_topline = bp->b_linep->l_fp;
			bp->b_folds[bp->b_nfolds].f_botline = curwp->w_dotp->l_foldp;
			bp->b_linep->l_fp = curwp->w_dotp->l_fp;
			curwp->w_dotp->l_fp->l_bp = bp->b_linep;
			curwp->w_dotp->l_fp = (LINE *)NULL;
			bp->b_folds[bp->b_nfolds].f_botline->l_bp->l_fp = bp->b_linep;
			bp->b_linep->l_bp->l_fp = (LINE *)NULL;
			bp->b_linep->l_bp = bp->b_folds[bp->b_nfolds].f_botline->l_bp;

			/* Move cursor to the beginning of next line */
			curwp->w_dotp = bp->b_linep->l_fp;
			curwp->w_doto = 0;

			/* increment fold count */
			bp->b_nfolds++;

			/* let all the proper windows be updated */
			wp = wheadp;
			while (wp) {
				if (wp->w_bufp == bp) {
					wp->w_flag |= (WFHARD|WFMODE|WFFORCE);
					/* have to enter in all windows */
					/* since buffer only valid for fold */
					wp->w_linep = bp->b_linep->l_fp;
					wp->w_dotp = wp->w_linep;
					wp->w_doto = 0;
					/* clear out marks */
					for (i = 0; i < NMARKS; i++)
						wp->w_markp[i] = (LINE *)NULL;

				}
				wp = wp->w_wndp;
			}
			mlwrite(TEXT230);
			/* "[Fold Entered]" */
			if (bp->b_mode & MDASAVE)
			mlwrite(TEXT248);
			/* "%%Cannot AUTOSAVE when within a fold" */
			return(TRUE);
		}
		else {
			mlwrite(TEXT237);
			/* "%% Folds too deep" */
			return(FALSE);
		}
	}
	else {
		mlwrite(TEXT235);
		/* "%%Line is not a fold" */
		return(FALSE);
	}
}


/*	exitfold:	Exit the current fold.
*/

PASCAL NEAR exitfold(
  int f,
  int n )
{
	BUFFER *bp;		 /* buffer being entered */
	WINDOW *wp;		 /* windows to fix up pointers in as well */
	LINE   *lp;		 /* line limits of entered fold */
	int    i, j;
	char   tmpstr[NSTRING];
	LINE   *flp;
	int    fos;
	int    chgflag, vewflag;
	int    margval;

	/* find the proper buffer */
	bp = curwp->w_bufp;		

	if (bp->b_nfolds > 0) {

		/* need foldsymbol in an array */
		strcpy(tmpstr, FOLDSYMBOL);

		/* decrement fold count */
		bp->b_nfolds--;

		/* close any inner folds */
		lp = bp->b_linep->l_fp;
		while (lp != bp->b_linep) {
			if (lp->l_type == LSOEFOLD) {
				lp->l_foldp->l_type = LEOFOLD;
				lp->l_type = LSOFOLD;
				i = indx(lp->l_text, BEGINFOLD);
				for (j = 0; j < strlen(FOLDSYMBOL); j ++)
					lp->l_text[i + j] = tmpstr[j];
			}
			lp = lp->l_fp;
		}

		/* restore previous buffer values, for top */
		lp = bp->b_folds[bp->b_nfolds].f_topline;
		while (lp->l_fp != (LINE *)NULL)
			lp = lp->l_fp;
		lp->l_fp = bp->b_linep->l_fp;
		lp->l_fp->l_bp = lp;
		bp->b_linep->l_fp = bp->b_folds[bp->b_nfolds].f_topline;
		bp->b_folds[bp->b_nfolds].f_topline->l_bp = bp->b_linep;
		
		/* and bottom */
		lp = bp->b_folds[bp->b_nfolds].f_botline;
		while (lp->l_fp != (LINE *)NULL)
			lp = lp->l_fp;
		lp->l_fp = bp->b_linep;
		bp->b_linep->l_bp->l_fp = bp->b_folds[bp->b_nfolds].f_botline;
		bp->b_folds[bp->b_nfolds].f_botline->l_bp = bp->b_linep->l_bp;
		bp->b_linep->l_bp = lp;

		/* Move cursor to the beginning of next line */
		curwp->w_dotp = bp->b_folds[bp->b_nfolds].f_botline->l_foldp->l_fp;
		curwp->w_doto = 0;

		/* record if buffer changed yet since this will */
		chgflag = bp->b_flag & BFCHG;
		vewflag = bp->b_mode & MDVIEW; /* no good in view mode */
		bp->b_mode &= ~MDVIEW;

		/* restore prefix - copy in from fold line - maybe ' ' & '\t' */
		flp = bp->b_folds[bp->b_nfolds].f_botline->l_foldp;
		fos = loffset(flp);
		margval = curwp->w_dotp->l_omargin;
		while (curwp->w_dotp != bp->b_folds[bp->b_nfolds].f_botline) {
			curwp->w_doto = 0;
			for (i = 0; i < fos; i++)
				linsert(1, flp->l_text[i], FALSE);
			curwp->w_dotp->l_lmargin = margval;
			if (curwp->w_dotp->l_type == LSOFOLD)
				curwp->w_dotp = curwp->w_dotp->l_foldp;
			curwp->w_dotp = curwp->w_dotp->l_fp;
		}

		/* reset change flag? */
		if (!chgflag)
			bp->b_flag &= ~BFCHG;
		if (vewflag) /* reset view flag */
			bp->b_mode |= MDVIEW;

		/* Move cursor to the beginning of next line */
		curwp->w_dotp = bp->b_folds[bp->b_nfolds].f_botline->l_foldp;
		curwp->w_doto = loffset(curwp->w_dotp);

		/* let all the proper windows be updated */
		wp = wheadp;
		while (wp) {
			if (wp->w_bufp == bp) {
				wp->w_flag |= (WFHARD|WFMODE|WFFORCE);
				/* enter all windows containing fold */
				wp->w_linep = curwp->w_linep;
				wp->w_dotp = curwp->w_dotp;
				wp->w_doto = curwp->w_doto;
			}
			wp = wp->w_wndp;
		}
		mlwrite(TEXT231);
		/* "[Fold Exited]" */
		return(TRUE);
	}
	else {
		mlwrite(TEXT238);
		/* "%%Not within an entered fold" */
		return(FALSE);
	}
}


/*	exitallfolds:	Exit all folds currently entered.
*/

PASCAL NEAR exitallfolds(
  int f,
  int n )
{
	while (curwp->w_bufp->b_nfolds > 0)
		exitfold(f, n);
	return(TRUE);
}


/*	makefold:	Make a fold of the current marked region.
*/

PASCAL NEAR makefold(
  int f,
  int n )
{
        register int status;	 /* return status */
	BUFFER *bp;		 /* buffer being folded*/
	WINDOW *wp;		 /* windows to fix up pointers in as well */
	REGION creg;		 /* region boundry structure */
	char   foldstr[NSTRING]; /* Fold string to insert into buffer/file */
	LINE   *topboundary;	 /* Line before fold */
	LINE   *botboundary;	 /* Line after fold */
	int    i;		 /* Loop */
	char   mprefix[NSTRING]; /* mark prefix */
	int    lm;		 /* left margin value */
	short  ltype;		 /* saved line type */

        if (curbp->b_mode&MDVIEW)       /* don't allow this command if  */
                return(rdonly());       /* we are in read only mode     */

	/* find the proper buffer */
	bp = curwp->w_bufp;		

	/* find the boundries of the current region */
	/* remember these could be fold lines */

	/* call getrawregion first, cos this will check if we cross crease */
        if ((status=getrawregion(&creg)) != TRUE)
                return(status);

	/* check if we have to indent the fold marker */
	i = 0;	
	lm = curwp->w_markp[0]->l_lmargin;
	if (curwp->w_marko[0] > lm) {
		while (((curwp->w_markp[0]->l_text[i + lm] == ' ') ||
		        (curwp->w_markp[0]->l_text[i + lm] == '\t')) &&
		       (i + lm < curwp->w_marko[0])) {
				mprefix[i] = curwp->w_markp[0]->l_text[i + lm];
				i++;
		}
	}
	mprefix[i] = NULL;

        if ((status=getregion(&creg)) != TRUE)
                return(status);
        curwp->w_dotp = creg.r_linep;	/* only by full lines */
        curwp->w_doto = 0;
	creg.r_size += (long)creg.r_offset;
	if (creg.r_size <= (long)curwp->w_dotp->l_used) {
		mlwrite(TEXT232);
/*                      "%%Must fold at least 1 full line" */
		return(FALSE);
	}

	/* insert the mapped fold-start line at top */
	/* have to insert and backup since it could already be a fold line */

	/* Unless line is normal cannot insert nl at left margin */
	ltype = curwp->w_dotp->l_type;
	curwp->w_dotp->l_type = LNORMAL;
	curwp->w_doto = curwp->w_dotp->l_lmargin;
	lnewline();

	/* Reset line type, backup and insert fold symbol */
	curwp->w_dotp->l_type = ltype;
	curwp->w_dotp = curwp->w_dotp->l_bp;
	strcpy(foldstr, mprefix);
	strcat(foldstr, FOLDSYMBOL);
	linstr(foldstr);
	topboundary = curwp->w_dotp;
	curwp->w_dotp = curwp->w_dotp->l_fp;
	curwp->w_doto = 0;

	/* move forward to the end of this region
	   (a long number of bytes perhaps) */
	while (creg.r_size > (long)32000) {
		forwchar(TRUE, 32000);
		creg.r_size -= (long)32000;
	}
	forwchar(TRUE, (int)creg.r_size);
	curwp->w_doto = 0;		/* only full lines! */

	/* Unless line is normal cannot insert nl at left margin */
	ltype = curwp->w_dotp->l_type;
	/* exception is end of open fold */
	if (ltype == LEOEFOLD)
		curwp->w_doto = loffset(curwp->w_dotp);
	else {
		curwp->w_dotp->l_type = LNORMAL;
		curwp->w_doto = curwp->w_dotp->l_lmargin;
	}
	lnewline();

	/* Reset line type, backup and insert end fold symbol */
	curwp->w_dotp->l_type = ltype;
	curwp->w_dotp = curwp->w_dotp->l_bp;
	strcpy(foldstr, mprefix);
	strcat(foldstr, ENDFOLD);
	linstr(foldstr);
	botboundary = curwp->w_dotp;

	/* set the fold pointers and line types */
	topboundary->l_type = LSOFOLD;
	topboundary->l_foldp = botboundary;
	botboundary->l_type = LEOFOLD;
	botboundary->l_foldp = topboundary;

	/* set left margin? */
	if (i) {
		curwp->w_dotp = topboundary->l_fp;
		i += curwp->w_dotp->l_lmargin;
		while (curwp->w_dotp != botboundary) {
			if (loffset(curwp->w_dotp) < i) {
				/* insert prefix - else problems! */
				curwp->w_doto = curwp->w_dotp->l_lmargin;
				linstr(mprefix);
			}
			if (curwp->w_dotp->l_lmargin < i) /* not inner fold */
				curwp->w_dotp->l_lmargin = i;
			curwp->w_dotp = curwp->w_dotp->l_fp; /* lforw() won't find the line */	
		}
	}

	/* move cursor to fold line */
	curwp->w_dotp = topboundary;
	curwp->w_doto = llength(curwp->w_dotp);

	/* clear out marks */
	for (i = 0; i < NMARKS; i++)
		bp->b_markp[i] = (LINE *)NULL;

	/* let all the proper windows be updated */
	wp = wheadp;
	while (wp) {
		if (wp->w_bufp == bp)
			wp->w_flag |= (WFHARD|WFMODE);
		wp = wp->w_wndp;
	}

	mlwrite(TEXT234);
/*              "[Buffer folded]" */

        return(TRUE);
}


/*	removefold:	Remove the fold at the current dot position.
*/

PASCAL NEAR removefold(
  int f,
  int n )
{
	BUFFER *bp;		 /* buffer having fold removed */
	WINDOW *wp;		 /* windows to fix up pointers in as well */
	LINE   *lp;		 /* line loop for reset of margin */
	int    margval;		 /* value to set margin to */

        if (curbp->b_mode&MDVIEW)       /* don't allow this command if  */
                return(rdonly());       /* we are in read only mode     */

	/* find the proper buffer */
	bp = curwp->w_bufp;		

	if (curwp->w_dotp->l_type == LSOFOLD) {

		/* set line types to normal */
		curwp->w_dotp->l_type = LNORMAL;
		curwp->w_dotp->l_foldp->l_type = LNORMAL;

		/* set all margins to that of any outer fold */
		margval = minleftmarg(curwp->w_dotp);
		lp = curwp->w_dotp->l_fp;
		while (lp != curwp->w_dotp->l_foldp) {
			lp->l_lmargin = margval;
			lp = lforw(lp);
		}

		/* and remove them */
		lfree(curwp->w_dotp->l_foldp);
		lfree(curwp->w_dotp);

		/* let all the proper windows be updated */
		wp = wheadp;
		while (wp) {
			if (wp->w_bufp == bp)
				wp->w_flag |= (WFHARD|WFMODE);
			wp = wp->w_wndp;
		}
		bp->b_flag |= BFCHG; /* flag change */
		mlwrite(TEXT233);
		/* "[Fold Removed]" */
		return(TRUE);
	}
	else {
		mlwrite(TEXT235);
		/* "%% Not a fold line" */
		return(FALSE);
	}
}


/*	delfold:	Delete the fold at the current dot position.
*/

PASCAL NEAR delfold(
  int f,
  int n )
{
	BUFFER *bp;		 /* buffer having fold deleted */
	LINE   *lp;		 /* lines being deleted */

	/* find the proper buffer */
	bp = curwp->w_bufp;		

	if (curwp->w_dotp->l_type == LSOFOLD) {

		/* set all line types to normal */
		lp = curwp->w_dotp;
		while (lp != curwp->w_dotp->l_foldp->l_fp) {
			lp->l_type = LNORMAL; 
			lp = lp->l_fp;
		}
		curwp->w_doto = 0;
		curwp->w_markp[0] = curwp->w_dotp->l_foldp->l_fp;
		curwp->w_marko[0] = 0;
		killregion(FALSE, 0);
		mlwrite(TEXT239);
		/* "[Fold Deleted]" */
		return(TRUE);
	}
	else {
		mlwrite(TEXT235);
		/* "%% Not a fold line" */
		return(FALSE);
	}
}

/*      losequote:         remove leading quotes from a string
*/

void losequote(char *str )
{
   char  tmp[NSTRING];
   
   if (*str == '"') {
      strcpy(tmp,str+1);
      strcpy(str,tmp);
   }
}


/*      setfoldmarks:      set fold marker strings for a given file extension
*/

int PASCAL NEAR setfoldmarks(
  int f,
  int n )
{
   register int status;
   char startf[NFOLD],afterf[NFOLD],endf[NFOLD],ext[NSTRING];
   FOLDMARKENT *fold_ent, *old_ent;

   status = TRUE; /* MJB: 19-Oct-89 */

   if (clexec == FALSE) {
      status = mlreply(TEXT243, ext, NSTRING+1);
/*                      "File extension: " */
      if (status != TRUE)
         return(status);
   }
   else {
      execstr = token(execstr, ext, NSTRING+1);
   }
   losequote(ext);

   if (clexec == FALSE) {
      status = mlreply(TEXT244, &startf[0], NFOLD+1);
/*                      "Start fold text: " */
      if (status != TRUE)
         return(status);
   }
   else {
      execstr = token(execstr, startf, NFOLD+1);
   }
   losequote(startf);
   
   if (clexec == FALSE) {
      status = mlreply(TEXT245, endf, NFOLD+1);
/*                      "End fold text: " */
      if (status != TRUE)
         return(status);
   }
   else {
      execstr = token(execstr, endf, NFOLD+1);
   }
   losequote(endf);
   
   if (clexec == FALSE) {
      status = mlreply(TEXT246, afterf, NFOLD+1);
/*                      "After fold text: " */
      if (status != TRUE)
         return(status);
   }
   else {
      execstr = token(execstr, afterf, NFOLD+1);
   }
   losequote(afterf);
   
   fold_ent = foldmarktab;
   old_ent = NULL;
   while (fold_ent != NULL) {
      if (strcmp(ext, fold_ent->fm_extension) == 0)
         old_ent = fold_ent;
      
      fold_ent = fold_ent->fm_next;
   }
   
   if (old_ent == NULL) {
      /* Build a new fold table entry */
      fold_ent = (FOLDMARKENT *)malloc(sizeof(FOLDMARKENT));
      fold_ent->fm_extension  = (char *)malloc(NSTRING);
      fold_ent->fm_startopen  = (char *)malloc(NFOLD);
      fold_ent->fm_startclose = (char *)malloc(NFOLD);
      fold_ent->fm_end        = (char *)malloc(NFOLD);

      /* link it in to the fold mark list */
      fold_ent->fm_next = foldmarktab;
      foldmarktab = fold_ent;
   }
   else {
      fold_ent = old_ent;
   }

   /* Copy in the strings */
   strcpy(fold_ent->fm_extension,  ext);
   strcpy(fold_ent->fm_startopen,  startf);
   strcpy(fold_ent->fm_startclose, afterf);
   strcpy(fold_ent->fm_end,        endf);
   
   return(status);
}

/* setfoldstring: create a fold table entry for a given extension
*/

PASCAL NEAR setfoldstring(
  char *  ext,
  char *  startf,
  char *  endf,
  char *  afterf )
{
   FOLDMARKENT *fold_ent, *old_ent;

   fold_ent = foldmarktab;
   old_ent = NULL;
   while (fold_ent != NULL) {
      if (strcmp(ext, fold_ent->fm_extension) == 0)
         old_ent = fold_ent;
      
      fold_ent = fold_ent->fm_next;
   }
   
   if (old_ent == NULL) {
      /* Build a new fold table entry */
      fold_ent = (FOLDMARKENT *)malloc(sizeof(FOLDMARKENT));
      fold_ent->fm_extension  = (char *)malloc(NSTRING);
      fold_ent->fm_startopen  = (char *)malloc(NFOLD);
      fold_ent->fm_startclose = (char *)malloc(NFOLD);
      fold_ent->fm_end        = (char *)malloc(NFOLD);

      /* link it in to the fold mark list */
      fold_ent->fm_next = foldmarktab;
      foldmarktab = fold_ent;
   }
   else {
      fold_ent = old_ent;
   }

   /* Copy in the strings */
   strcpy(fold_ent->fm_extension,  ext);
   strcpy(fold_ent->fm_startopen,  startf);
   strcpy(fold_ent->fm_startclose, afterf);
   strcpy(fold_ent->fm_end,        endf);
}

/* initfoldstrings: setup default fold mark strings
*/

PASCAL NEAR initfoldstrings()
{
   setfoldstring("c",   "/*{{{  ", "/*}}}*/", " */");
   setfoldstring("h",   "/*{{{  ", "/*}}}*/", " */");
   setfoldstring("pas", "(*{{{  ", "(*}}}*)", " *)");
   setfoldstring("p",   "(*{{{  ", "(*}}}*)", " *)");
   setfoldstring("bcp", "//{{{  ", "//}}}",    "");
   setfoldstring("occ", "--{{{  ", "--}}}",    "");
   setfoldstring("tex", "%%{{{  ", "%%}}}",    "");
   setfoldstring("ps",  "%%{{{  ", "%%}}}",    "");
   setfoldstring("asm", ";{{{  ",  ";}}}",     "");
   setfoldstring("inc", ";{{{  ",  ";}}}",     "");
   setfoldstring("a",	"--{{{  ", "--}}}",    "");
   setfoldstring("cc",	"//{{{  ", "//}}}",    "");
   setfoldstring("cxx",	"//{{{  ", "//}}}",    "");
   setfoldstring("hxx",	"//{{{  ", "//}}}",    "");
   setfoldstring("hh",	"//{{{  ", "//}}}",    "");
   setfoldstring("C",	"//{{{  ", "//}}}",    "");
   setfoldstring("f",	"C {{{  ", "C }}}",    "");
   setfoldstring("for", "C {{{  ", "C }}}",    "");
   setfoldstring("mod", "(*{{{  ", "(*}}}*)", " *)");
   setfoldstring("rm",  "# {{{  ", "# }}}",    "");
}


/*
 * minleftmarg -- Find the minimum left margin value for the
 *		  current line. Searches back up to enclosing
 *		  open fold. MJB: 17-Oct-89.
 */
int PASCAL NEAR minleftmarg(LINE	*lp )
{
	if (lp->l_type == LEOEFOLD)
		return(lp->l_foldp->l_lmargin);

	lp = lp->l_bp;
	while ((lp != curwp->w_bufp->b_linep) &&
	       (lp->l_type != LSOEFOLD)) {
		if ((lp->l_type == LEOFOLD) ||
		    (lp->l_type == LEOEFOLD))
			lp = lp->l_foldp;
		lp = lp->l_bp;
	}
	if (lp->l_type == LSOEFOLD)
		return(loffset(lp));
/*		return(lp->l_lmargin); */
	else
		return(0);
}


/*
 * openoutfolds -- open any nested folds above the current (moved)
 *		   dot position. MJB: 13-Oct-89.
 */
PASCAL NEAR openoutfolds()
{
	LINE *lp;
	BUFFER *bp;
	WINDOW *wp;
	int    i, j;
	char   tmpstr[NSTRING];

	/* find the proper buffer */
	bp = curwp->w_bufp;		

	lp = curwp->w_dotp; /* start at new position */
	if (lp->l_type == LEOFOLD) /* special case upon entry    */
		lp = lp->l_foldp;  /* only time this can be true */
	while (lp != curwp->w_bufp->b_linep) { /* check */
		if (lp->l_type == LSOFOLD) { /* open it */
			lp->l_type = LSOEFOLD;
			lp->l_foldp->l_type = LEOEFOLD;
			i = indx(lp->l_text, FOLDSYMBOL);
			strcpy(tmpstr, BEGINFOLD);
			for (j = 0; j < strlen(FOLDSYMBOL); j ++)
				lp->l_text[i + j] = tmpstr[j];
			}
		else if (lp->l_type == LEOFOLD) /* jump round it */
			lp = lp->l_foldp;
		lp = lp->l_bp;
	}

	/* let all the proper windows be updated */
	wp = wheadp;
	while (wp) {
		if (wp->w_bufp == bp)
			wp->w_flag |= (WFHARD|WFMODE|WFFORCE);
		wp = wp->w_wndp;
	}
	return(TRUE);
}
