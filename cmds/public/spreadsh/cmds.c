/*	SC	A Spreadsheet Calculator
 *		Command routines
 *
 *		original by James Gosling, September 1982
 *		modifications by Mark Weiser and Bruce Israel,
 *			University of Maryland
 *
 *              More mods Robert Bond, 12/86
 *
 *		$Revision: 1.2 $
 */

#include <curses.h>
#include "sc.h"
#include <signal.h>
#include <stdlib.h>
#include <posix.h>
#include <sys/wait.h>

#ifdef BSD42
#include <strings.h>
#else
#ifndef SYSIII
#include <string.h>
#endif
#endif

#ifdef SYSV3
extern void exit();
#else
#include <stdlib.h>
#endif

#define DEFCOLDELIM ':'

void
duprow()
{
    if (currow >= MAXROWS - 1 || maxrow >= MAXROWS - 1) {
	error ("The table can't be any bigger");
	return;
    }
    modflg++;
    currow++;
    openrow (currow);
    for (curcol = 0; curcol <= maxcol; curcol++) {
	register struct ent *p = tbl[currow - 1][curcol];
	if (p) {
	    register struct ent *n;
	    n = lookat (currow, curcol);
	    copyent ( n, p, 1, 0);
	}
    }
    for (curcol = 0; curcol <= maxcol; curcol++) {
	register struct ent *p = tbl[currow][curcol];
	if (p && (p -> flags & is_valid) && !p -> expr)
	    break;
    }
    if (curcol > maxcol)
	curcol = 0;
}

void
dupcol() 
{
    if (curcol >= MAXCOLS - 1 || maxcol >= MAXCOLS - 1) {
	error ("The table can't be any wider");
	return;
    }
    modflg++;
    curcol++;
    opencol (curcol);
    for (currow = 0; currow <= maxrow; currow++) {
	register struct ent *p = tbl[currow][curcol - 1];
	if (p) {
	    register struct ent *n;
	    n = lookat (currow, curcol);
	    copyent ( n, p, 0, 1);
	}
    }
    for (currow = 0; currow <= maxrow; currow++) {
	register struct ent *p = tbl[currow][curcol];
	if (p && (p -> flags & is_valid) && !p -> expr)
	    break;
    }
    if (currow > maxrow)
	currow = 0;
}

void
insertrow(arg)
register int arg;
{
    while (--arg>=0) openrow (currow);
}

void
deleterow(arg)
register int arg;
{
    flush_saved();
    erase_area(currow, 0, currow + arg - 1, maxcol);
    currow += arg;
    while (--arg>=0) closerow (--currow);
    sync_refs();
}

void
insertcol(arg)
register int arg;
{
    while (--arg>=0) opencol(curcol);
}

void
deletecol(arg)
register int arg;
{
    flush_saved();
    erase_area(0, curcol, maxrow, curcol + arg - 1);
    curcol += arg;
    while (--arg>=0) closecol (--curcol);
    sync_refs();
}

void
rowvalueize(arg)
register int arg;
{
    valueize_area(currow, 0, currow + arg - 1, maxcol);
}

void
colvalueize(arg)
register int arg;
{
    valueize_area(0, curcol, maxrow, curcol + arg - 1);
}

void
erase_area(sr, sc, er, ec)
int sr, sc, er, ec;
{
    register int r, c;
    register struct ent **p;

    if (sr > er) {
	r = sr; sr = er; er= r;	
    }

    if (sc > ec) {
	c = sc; sc = ec; ec= c;	
    }

    if (sr < 0)
	sr = 0; 
    if (sc < 0)
	sc = 0;
    if (er >= MAXROWS)
	er = MAXROWS-1;
    if (ec >= MAXCOLS)
	ec = MAXCOLS-1;

    for (r = sr; r <= er; r++) {
	for (c = sc; c <= ec; c++) {
	    p = &tbl[r][c];
	    if (*p) {
		free_ent(*p);
		*p = 0;
	    }
	}
    }

}

void
valueize_area(sr, sc, er, ec)
int sr, sc, er, ec;
{
    register int r, c;
    register struct ent *p;

    if (sr > er) {
	r = sr; sr = er; er= r;	
    }

    if (sc > ec) {
	c = sc; sc = ec; ec= c;	
    }

    if (sr < 0)
	sr = 0; 
    if (sc < 0)
	sc = 0;
    if (er >= MAXROWS)
	er = MAXROWS-1;
    if (ec >= MAXCOLS)
	ec = MAXCOLS-1;

    for (r = sr; r <= er; r++) {
	for (c = sc; c <= ec; c++) {
	    p = tbl[r][c];
	    if (p && p->expr) {
		efree(p->expr);
		p->expr = 0;
		p->flags &= ~is_strexpr;
	    }
	}
    }

}

void
pullcells(to_insert)
int to_insert;
{
    register struct ent *p, *n;
    register int deltar, deltac;
    int minrow, mincol;
    int mxrow, mxcol;
    int numrows, numcols;

    if (! to_fix)
    {
	error ("No data to pull");
	return;
    }

    minrow = MAXROWS; 
    mincol = MAXCOLS;
    mxrow = 0;
    mxcol = 0;

    for (p = to_fix; p; p = p->next) {
	if (p->row < minrow)
	    minrow = p->row;
	if (p->row > mxrow)
	    mxrow = p->row;
	if (p->col < mincol)
	    mincol = p->col;
	if (p->col > mxcol)
	    mxcol = p->col;
    }

    numrows = mxrow - minrow + 1;
    numcols = mxcol - mincol + 1;
    deltar = currow - minrow;
    deltac = curcol - mincol;

    if (to_insert == 'r') {
	insertrow(numrows);
	deltac = 0;
    } else if (to_insert == 'c') {
	insertcol(numcols);
	deltar = 0;
    }

    FullUpdate++;
    modflg++;

    for (p = to_fix; p; p = p->next) {
	n = lookat (p->row + deltar, p->col + deltac);
	(void) clearent(n);
	copyent( n, p, deltar, deltac);
	n -> flags = p -> flags & ~is_deleted;
    }
}

void
colshow_op()
{
    register int i,j;
    for (i=0; i<MAXCOLS; i++)
	if (col_hidden[i]) 
	    break;
    for(j=i; j<MAXCOLS; j++)
	if (!col_hidden[j])
	    break;
    j--;
    if (i>=MAXCOLS)
	error ("No hidden columns to show");
    else {
	(void) sprintf(line,"show %s:", coltoa(i));
	(void) sprintf(line + strlen(line),"%s",coltoa(j));
	linelim = strlen (line);
    }
}

void
rowshow_op()
{
    register int i,j;
    for (i=0; i<MAXROWS; i++)
	if (row_hidden[i]) 
	    break;
    for(j=i; j<MAXROWS; j++)
	if (!row_hidden[j]) {
	    break;
	}
    j--;

    if (i>=MAXROWS)
	error ("No hidden rows to show");
    else {
	(void) sprintf(line,"show %d:%d", i, j);
        linelim = strlen (line);
    }
}

/*
 * Given a row/column command letter, emit a small menu, then read a qualifier
 * character for a row/column command and convert it to 'r' (row), 'c'
 * (column), or 0 (unknown).  If ch is 'p', an extra qualifier 'm' is allowed.
 */

int
get_rcqual (int ch )
{
    error ("%sow/column:  r: row  c: column%s",

	    (ch == 'i') ? "Insert r" :
	    (ch == 'a') ? "Append r" :
	    (ch == 'd') ? "Delete r" :
	    (ch == 'p') ? "Pull r" :
	    (ch == 'v') ? "Values r" :
	    (ch == 'z') ? "Zap r" :
	    (ch == 's') ? "Show r" : "R",

	    (ch == 'p') ? "  m: merge" : "");

    (void) refresh();

    switch (nmgetch())
    {
	case 'r':
	case 'l':
	case 'h':
	case ctl('f'):
	case ctl('b'):	return ('r');

	case 'c':
	case 'j':
	case 'k':
	case ctl('p'):
	case ctl('n'):	return ('c');

	case 'm':	return ((ch == 'p') ? 'm' : 0);

#ifdef __HELIOS
	case CSI:
#endif
	case ESC:
	case ctl ('g'):	return (ESC);

	default:	return (0);
    }
    /*NOTREACHED*/
}

void
openrow (int rs )
{
    register    r;
    register struct ent **p;
    register    c;
    register	i;

    if (rs > maxrow) maxrow = rs;
    if (maxrow >= MAXROWS - 1 || rs > MAXROWS - 1) {
	error ("The table can't be any longer");
	return;
    }
    for (i = maxrow+1; i > rs; i--) {
	row_hidden[i] = row_hidden[i-1];
    }
    for (r = ++maxrow; r > rs; r--)
	for (c = maxcol + 1, p = &tbl[r][0]; --c >= 0; p++)
	    if ((p[0] = p[-MAXCOLS]) != NULL)
		p[0] -> row++;
    p = &tbl[rs][0];
    for (c = maxcol + 1; --c >= 0;)
	*p++ = 0;
    FullUpdate++;
    modflg++;
}

void
closerow (register r )
{
    register struct ent **p;
    register c;
    register int i;

    if (r > maxrow) return;

    p = &tbl[r][0];
    for (c=maxcol+1; --c>=0; ) {
	if (*p)
	    free_ent(*p);
	*p++ = 0;
    }

    for (i = r; i < MAXROWS - 1; i++) {
	row_hidden[i] = row_hidden[i+1];
    }

    while (r<maxrow) {
	for (c = maxcol+1, p = &tbl[r][0]; --c>=0; p++)
	    if ((p[0] = p[MAXCOLS]) != NULL)
		p[0]->row--;
	r++;
    }

    p = &tbl[maxrow][0];
    for (c=maxcol+1; --c>=0; ) *p++ = 0;
    maxrow--;
    FullUpdate++;
    modflg++;
}

void
opencol (int cs )
{
    register r;
    register struct ent **p;
    register c;
    register lim = maxcol-cs+1;
    int i;

    if (cs > maxcol) maxcol = cs;
    if (maxcol >= MAXCOLS - 1 || cs > MAXCOLS - 1) {
	error ("The table can't be any wider");
	return;
    }
    for (i = maxcol+1; i > cs; i--) {
	fwidth[i] = fwidth[i-1];
	precision[i] = precision[i-1];
	col_hidden[i] = col_hidden[i-1];
    }
    /* fwidth[cs] = DEFWIDTH;
    precision[i] =  DEFPREC;  */

    for (r=0; r<=maxrow; r++) {
	p = &tbl[r][maxcol+1];
	for (c=lim; --c>=0; p--)
	    if ((p[0] = p[-1]) != NULL)
		p[0]->col++;
	p[0] = 0;
    }
    maxcol++;
    FullUpdate++;
    modflg++;
}

void
closecol (int cs )
{
    register r;
    register struct ent **p;
    register struct ent *q;
    register c;
    register lim = maxcol-cs;
    int i;

    if (lim < 0) return;

    for (r=0; r<=maxrow; r++)
	if ((q = tbl[r][cs]) != NULL) {
	    free_ent(q);
	}

    for (r=0; r<=maxrow; r++) {
	p = &tbl[r][cs];
	for (c=lim; --c>=0; p++)
	    if ((p[0] = p[1]) != NULL)
		p[0]->col--;
	p[0] = 0;
    }

    for (i = cs; i < MAXCOLS - 1; i++) {
	fwidth[i] = fwidth[i+1];
	precision[i] = precision[i+1];
	col_hidden[i] = col_hidden[i+1];
    }

    maxcol--;
    FullUpdate++;
    modflg++;
}

void
doend(rowinc, colinc)
int rowinc, colinc;
{
    register struct ent *p;
    int r, c;

    if (VALID_CELL(p, currow, curcol)) {
	r = currow + rowinc;
	c = curcol + colinc;
	if (r >= 0 && r < MAXROWS && 
	    c >= 0 && c < MAXCOLS &&
	    !VALID_CELL(p, r, c)) {
		currow = r;
		curcol = c;
	}
    }

    if (!VALID_CELL(p, currow, curcol)) {
        switch (rowinc) {
        case -1:
	    while (!VALID_CELL(p, currow, curcol) && currow > 0)
		currow--;
	    break;
        case  1:
	    while (!VALID_CELL(p, currow, curcol) && currow < MAXROWS-1)
		currow++;
	    break;
        case  0:
            switch (colinc) {
 	    case -1:
	        while (!VALID_CELL(p, currow, curcol) && curcol > 0)
		    curcol--;
	        break;
 	    case  1:
	        while (!VALID_CELL(p, currow, curcol) && curcol < MAXCOLS-1)
		    curcol++;
	        break;
	    }
            break;
        }

	error ("");	/* clear line */
	return;
    }

    switch (rowinc) {
    case -1:
	while (VALID_CELL(p, currow, curcol) && currow > 0)
	    currow--;
	break;
    case  1:
	while (VALID_CELL(p, currow, curcol) && currow < MAXROWS-1)
	    currow++;
	break;
    case  0:
	switch (colinc) {
	case -1:
	    while (VALID_CELL(p, currow, curcol) && curcol > 0)
		curcol--;
	    break;
	case  1:
	    while (VALID_CELL(p, currow, curcol) && curcol < MAXCOLS-1)
		curcol++;
	    break;
	}
	break;
    }
    if (!VALID_CELL(p, currow, curcol)) {
        currow -= rowinc;
        curcol -= colinc;
    }
}

void
doformat(c1,c2,w,p)
int c1,c2,w,p;
{
    register int i;

    if (w > COLS - RESCOL - 2) {
	error("Format too large - Maximum = %d", COLS - RESCOL - 2);
	w = COLS-RESCOL-2;
    }

    if (p > w) {
	error("Precision too large");
	p = w;
    }

    for(i = c1; i<=c2; i++)
	fwidth[i] = w, precision[i] = p;

    FullUpdate++;
    modflg++;
}

void
print_options(FILE *f )
{
    if(
       autocalc &&
       propagation == 10 &&
       calc_order == BYROWS &&
       !numeric &&
       prescale == 1.0 &&
       !extfunc &&
       showcell &&
       showtop &&
       tbl_style == 0
      )
		return;		/* No reason to do this */

    (void) fprintf(f, "set");
    if(!autocalc) 
	(void) fprintf(f," !autocalc");
    if(propagation != 10)
	(void) fprintf(f, " iterations = %d", propagation);
    if(calc_order != BYROWS )
	(void) fprintf(f, " bycols");
    if (numeric)
	(void) fprintf(f, " numeric");
    if (prescale != 1.0)
	(void) fprintf(f, " prescale");
    if (extfunc)
	(void) fprintf(f, " extfun");
    if (!showcell)
	(void) fprintf(f, " !cellcur");
    if (!showtop)
	(void) fprintf(f, " !toprow");
    if (tbl_style)
	(void) fprintf(f, " tblstyle = %s", tbl_style == TBL ? "tbl" :
					tbl_style == LATEX ? "latex" :
					tbl_style == TEX ? "tex" : "0" );
    (void) fprintf(f, "\n");
}

void
printfile (fname, r0, c0, rn, cn)
char *fname;
int r0, c0, rn, cn;
{
    FILE *f;
    char pline[1000];
    int plinelim;
    int pid;
    int fieldlen, nextcol;
    register row, col;
    register struct ent **p;
    char ch, lin[100];

    if (strcmp(fname, curfile) == 0) {
	(void) move (0, 0);
	(void) clrtoeol ();
	(void) sprintf (lin,
		"Confirm that you want to destroy the data base: (y,n)");
	(void) addstr (lin);
	(void) refresh();
	ch = nmgetch();
	if (ch != 'y' && ch != 'Y') 
	    return;
    }

    f = openout(fname, &pid);

    if (f==0) {
	error ("Can't create file \"%s\"", fname);
	return;
    }
    for (row=r0;row<=rn; row++) {
	register c = 0;

	if (row_hidden[row])
	    continue;

	pline[plinelim=0] = '\0';
	for (p = &tbl[row][col=c0]; col<=cn;
	        p += nextcol-col, col = nextcol, c += fieldlen) {

	    nextcol = col+1;
	    if (col_hidden[col]) {
		fieldlen = 0;
		continue;
	    }

	    fieldlen = fwidth[col];
	    if (*p) {
		char *s;

		while (plinelim<c) pline[plinelim++] = ' ';
		plinelim = c;
		if ((*p)->flags&is_valid) {
		    (void)sprintf (pline+plinelim,"%*.*f",fwidth[col],
		                                precision[col], (*p)->v);
		    plinelim += strlen (pline+plinelim);
		}
		if ((s = (*p)->label) != NULL) {
		    int slen;
		    char *start, *last;
		    register char *fp;
		    struct ent *nc;

		    /* Figure out if the label slops over to a blank field */
		    slen = strlen(s);
		    while (slen > fieldlen && nextcol <= cn &&
			    !((nc = lookat(row,nextcol))->flags & is_valid) &&
			    !(nc->label)) {
			
	                if (!col_hidden[nextcol])
		 	    fieldlen += fwidth[nextcol];

			nextcol++;
		    }
		    if (slen > fieldlen)
			slen = fieldlen;
		    
		    /* Now justify and print */
		    start = (*p)->flags & is_leftflush ? pline + c
					: pline + c + fieldlen - slen;
		    last = pline + c + fieldlen;
		    fp = plinelim < c ? pline + plinelim : pline + c;
		    while (fp < start)
			*fp++ = ' ';
		    while (slen--)
			*fp++ = *s++;
		    if (!((*p)->flags & is_valid) || fieldlen != fwidth[col])
			while(fp < last)
			    *fp++ = ' ';
		    if (plinelim < fp - pline)
			plinelim = fp - pline;
		}
	    }
	}
	pline[plinelim++] = '\n';
	pline[plinelim] = 0;
	(void) fputs (pline, f);
    }

    closeout(f, pid);
}

void
tblprintfile (fname, r0, c0, rn, cn)
char *fname;
int r0, c0, rn, cn;
{
    FILE *f;
    int pid;
    register row, col;
    register struct ent **p;
    char coldelim = DEFCOLDELIM;
    char ch, lin[100];

    if (strcmp(fname, curfile) == 0) {
	(void) move (0, 0);
	(void) clrtoeol ();
	(void) sprintf (lin,
		"Confirm that you want to destroy the data base: (y,n)");
	(void) addstr (lin);
	(void) refresh();
	ch = nmgetch();
	if (ch != 'y' && ch != 'Y') 
	    return;
    }

    f = openout(fname, &pid);

    if (f==0) {
	error ("Can't create file \"%s\"", fname);
	return;
    }

    if ( tbl_style == TBL ) {
	fprintf(f,".\\\" ** %s spreadsheet output \n.TS\n",progname);
	fprintf(f,"tab(%c);\n",coldelim);
	for (col=c0;col<=cn; col++) fprintf(f," n");
	fprintf(f, ".\n");
	}
    else if ( tbl_style == LATEX ) {
	fprintf(f,"%% ** %s spreadsheet output\n\\begin{tabular}{",progname);
	for (col=c0;col<=cn; col++) fprintf(f,"c");
	fprintf(f, "}\n");
	coldelim = '&';
	}
    else if ( tbl_style == TEX ) {
	fprintf(f,"{\t%% ** %s spreadsheet output\n\\settabs %d \\columns\n",
		progname, cn-c0+1);
	coldelim = '&';
	}

    for (row=r0; row<=rn; row++) {
	if ( tbl_style == TEX )
	    (void) fprintf (f, "\\+");
	
	for (p = &tbl[row][col=c0]; col<=cn; col++, p++) {
	    if (*p) {
		char *s;
		if ((*p)->flags&is_valid) {
		    (void) fprintf (f,"%.*f",precision[col],
				(*p)->v);
		}
		if ((s = (*p)->label) != NULL) {
	            (void) fprintf (f,"%s",s);
		}
	    }
	    if ( col < cn )
		(void) fprintf(f,"%c",coldelim);
	}
	if ( tbl_style == LATEX ) {
	    if ( row < rn ) (void) fprintf (f, "\\\\");
	    }
	else if ( tbl_style == TEX ) {
	    (void) fprintf (f, "\\cr");
	    }
	(void) fprintf (f,"\n");
    }

    if ( tbl_style == TBL )
    (void) fprintf (f,".TE\n.\\\" ** end of %s spreadsheet output\n", progname);
    else if ( tbl_style == LATEX )
    (void) fprintf (f,"\\end{tabular}\n%% ** end of %s spreadsheet output\n", progname);
    else if ( tbl_style == TEX )
    (void) fprintf (f,"}\n%% ** end of %s spreadsheet output\n", progname);

    closeout(f, pid);
}

struct enode *
copye (e, Rdelta, Cdelta)
register struct enode *e;
int Rdelta, Cdelta;
{
    register struct enode *ret;
    if (e==0) {
        ret = 0;
    } else if (e->op & REDUCE) {
	int newrow, newcol;
	ret = (struct enode *) xmalloc ((unsigned) sizeof (struct enode));
	ret->op = e->op;
	newrow=e->e.r.left.vf & FIX_ROW ? e->e.r.left.vp->row :
					  e->e.r.left.vp->row+Rdelta;
	newcol=e->e.r.left.vf & FIX_COL ? e->e.r.left.vp->col :
					  e->e.r.left.vp->col+Cdelta;
	ret->e.r.left.vp = lookat (newrow, newcol);
	ret->e.r.left.vf = e->e.r.left.vf;
	newrow=e->e.r.right.vf & FIX_ROW ? e->e.r.right.vp->row :
					   e->e.r.right.vp->row+Rdelta;
	newcol=e->e.r.right.vf & FIX_COL ? e->e.r.right.vp->col :
					   e->e.r.right.vp->col+Cdelta;
	ret->e.r.right.vp = lookat (newrow, newcol);
	ret->e.r.right.vf = e->e.r.right.vf;
    } else {
	ret = (struct enode *) xmalloc ((unsigned) sizeof (struct enode));
	ret->op = e->op;
	switch (ret->op) {
	case 'v':
		{
		    int newrow, newcol;
		    newrow=e->e.v.vf & FIX_ROW ? e->e.v.vp->row :
						 e->e.v.vp->row+Rdelta;
		    newcol=e->e.v.vf & FIX_COL ? e->e.v.vp->col :
						 e->e.v.vp->col+Cdelta;
		    ret->e.v.vp = lookat (newrow, newcol);
		    ret->e.v.vf = e->e.v.vf;
		    break;
		}
	case 'k':
		ret->e.k = e->e.k;
		break;
	case 'f':
		ret->e.o.right = copye (e->e.o.right,0,0);
		ret->e.o.left = 0;
 		break;
	case '$':
		ret->e.s = xmalloc((unsigned) strlen(e->e.s)+1);
		(void) strcpy(ret->e.s, e->e.s);
		break;
	default:
		ret->e.o.right = copye (e->e.o.right,Rdelta,Cdelta);
		ret->e.o.left = copye (e->e.o.left,Rdelta,Cdelta);
		break;
	}
    }
    return ret;
}

/*
 * sync_refs and syncref are used to remove references to
 * deleted struct ents.  Note that the deleted structure must still
 * be hanging around before the call, but not referenced by an entry
 * in tbl.  Thus the free_ent, fix_ent calls in sc.c
 */

void
sync_refs ()
{
    register i,j;
    register struct ent *p;
    sync_ranges();
    for (i=0; i<=maxrow; i++)
	for (j=0; j<=maxcol; j++)
	    if ((p = tbl[i][j]) != NULL && p->expr)
		syncref(p->expr);
}


void
syncref(e)
register struct enode *e;
{
    if (e==0)
	return;
    else if (e->op & REDUCE) {
 	e->e.r.right.vp = lookat(e->e.r.right.vp->row, e->e.r.right.vp->col);
 	e->e.r.left.vp = lookat(e->e.r.left.vp->row, e->e.r.left.vp->col);
    } else {
	switch (e->op) {
	case 'v':
		e->e.v.vp = lookat(e->e.v.vp->row, e->e.v.vp->col);
		break;
	case 'k':
		break;
	case '$':
		break;
	default:
		syncref(e->e.o.right);
		syncref(e->e.o.left);
		break;
	}
    }
}

void
hiderow(int arg )
{
    register int r1;
    register int r2;

    r1 = currow;
    r2 = r1 + arg - 1;
    if (r1 < 0 || r1 > r2) {
	error ("Invalid range");
	return;
    }
    if (r2 > MAXROWS-2) {
	error ("You can't hide the last row");
	return;
    }
    FullUpdate++;
    while (r1 <= r2)
	row_hidden[r1++] = 1;
}

void
hidecol(int arg )
{
    register int c1;
    register int c2;

    c1 = curcol;
    c2 = c1 + arg - 1;
    if (c1 < 0 || c1 > c2) {
	error ("Invalid range");
	return;
    }
    if (c2 > MAXCOLS-2) {
	error ("You can't hide the last column");
	return;
    }
    FullUpdate++;
    while (c1 <= c2)
	col_hidden[c1++] = 1;
}

void
showrow(r1, r2)
int r1, r2;
{
    if (r1 < 0 || r1 > r2) {
	error ("Invalid range");
	return;
    }
    if (r2 > MAXROWS-1) {
	r2 = MAXROWS-1;
    }
    FullUpdate++;
    while (r1 <= r2)
	row_hidden[r1++] = 0;
}

void
showcol(c1, c2)
int c1, c2;
{
    if (c1 < 0 || c1 > c2) {
	error ("Invalid range");
	return;
    }
    if (c2 > MAXCOLS-1) {
	c2 = MAXCOLS-1;
    }
    FullUpdate++;
    while (c1 <= c2)
	col_hidden[c1++] = 0;
}

/* Open the output file, setting up a pipe if needed */

FILE *
openout(fname, rpid)
char *fname;
int *rpid;
{
    int pipefd[2];
    int pid;
    FILE *f;

    while (*fname && (*fname == ' '))  /* Skip leading blanks */
	fname++;

    if (*fname != '|') {		/* Open file if not pipe */
	*rpid = 0;
	return(fopen(fname, "w"));
    }

    fname++;				/* Skip | */
    if ( pipe (pipefd) < 0) {
	error("Can't make pipe to child");
	*rpid = 0;
	return(0);
    }

    deraw();
#ifdef VMS
    fprintf(stderr, "No son tasks available yet under VMS--sorry\n");
#else /* VMS */

    if ((pid=fork()) == 0)			  /* if child  */
    {
	(void) close (0);			  /* close stdin */
	(void) close (pipefd[1]);
	(void) dup (pipefd[0]);		  /* connect to pipe input */
	(void) signal (SIGINT, SIG_DFL);	  /* reset */
	(void) execl ("/bin/sh", "sh", "-c", fname, 0);
	exit (-127);
    }
    else				  /* else parent */
    {
	*rpid = pid;
	f = fdopen (pipefd[1], "w");
	if (f == 0)
	{
	    (void) kill (pid, -9);
	    error ("Can't fdopen output");
	    (void) close (pipefd[1]);
	    *rpid = 0;
	    return(0);
	}
    }
#endif /* VMS */
    return(f);
}

void
closeout(f, pid)
FILE *f;
int pid;
{
    int temp;

    (void) fclose (f);
    if (pid) {
         while (pid != wait(&temp)) /**/;
	 (void) printf("Press RETURN to continue ");
	 (void) fflush(stdout);
	 (void) nmgetch();
	 goraw();
    }
}

void
copyent(n,p,dr,dc)
	    register struct ent *n, *p;
	    int dr, dc;
{
    if(!n||!p){error("internal error");return;}
    n -> v = p -> v;
    n -> flags = p -> flags;
    n -> expr = copye (p -> expr, dr, dc);
    n -> label = 0;
    if (p -> label) {
	n -> label = (char *)
		xmalloc  ((unsigned) (strlen (p -> label) + 1));
	(void) strcpy (n -> label, p -> label);
    }
}

void
write_fd (f, r0, c0, rn, cn)
register FILE *f;
int r0, c0, rn, cn;
{
    register struct ent **p;
    register r, c;

    (void) fprintf (f, "# This data file was generated by the Spreadsheet ");
    (void) fprintf (f, "Calculator.\n");
    (void) fprintf (f, "# You almost certainly shouldn't edit it.\n\n");
    print_options(f);
    for (c=0; c<MAXCOLS; c++)
	if (fwidth[c] != DEFWIDTH || precision[c] != DEFPREC)
	    (void) fprintf (f, "format %s %d %d\n",coltoa(c),fwidth[c],precision[c]);
    for (c=c0; c<cn; c++) {
        if (col_hidden[c]) {
            (void) fprintf(f, "hide %s\n", coltoa(c));
        }
    }
    for (r=r0; r<=rn; r++) {
	if (row_hidden[r]) {
	    (void) fprintf(f, "hide %d\n", r);
	}
    }

    write_range(f);

    if (mdir) 
	    (void) fprintf(f, "mdir \"%s\"\n", mdir);
    for (r=r0; r<=rn; r++) {
	p = &tbl[r][c0];
	for (c=c0; c<=cn; c++, p++)
	    if (*p) {
		if ((*p)->label) {
		    edits(r,c);
		    (void) fprintf(f, "%s\n",line);
		}
		if ((*p)->flags&is_valid) {
		    editv (r, c);
		    (void) fprintf (f, "%s\n",line);
		}
	    }
    }
}

int
writefile (fname, r0, c0, rn, cn)
char *fname;
int r0, c0, rn, cn;
{
    register FILE *f;
    char save[1024];
    int pid;

#ifndef VMS
    if (Crypt) {
	return (cwritefile(fname, r0, c0, rn, cn));
    }
#endif /* VMS */

    if (*fname == 0) fname = &curfile[0];

    (void) strcpy(save,fname);

    f = openout(fname, &pid);
    if (f == 0) {
	error ("Can't create file \"%s\"", fname);
	return (-1);
    }

    write_fd(f, r0, c0, rn, cn);
    
    closeout(f, pid);

    if (!pid) {
        (void) strcpy(curfile, save);
        modflg = 0;
        error("File \"%s\" written.",curfile);
    }

    return (0);
}

void
readfile (fname,eraseflg)
char *fname;
int eraseflg;
{
    register FILE *f;
    char save[1024];

    if (*fname == '*' && mdir) { 
       (void) strcpy(save, mdir);
       *fname = '/';
       (void) strcat(save, fname);
    } else {
        if (*fname == 0)
	    fname = &curfile[0];
        (void) strcpy(save,fname);
    }

#ifndef VMS
    if (Crypt)  {
	creadfile(save, eraseflg);
	return;
    }
#endif /* VMS */

    if (eraseflg && strcmp(fname,curfile) && modcheck(" first")) return;

    f = fopen (save, "r");
    if (f==0) {
	error ("Can't read file \"%s\"", save);
	return;
    }

    if (eraseflg) erasedb ();

    loading++;
    while (fgets(line,sizeof line,f)) {
	linelim = 0;
	if (line[0] != '#') (void) yyparse ();
    }
    --loading;
    (void) fclose (f);
    linelim = -1;
    modflg++;
    if (eraseflg) {
	(void) strcpy(curfile,save);
	modflg = 0;
    }
    EvalAll();
}

void
erasedb ()
{
    register r, c;
    for (c = 0; c<=maxcol; c++) {
	fwidth[c] = DEFWIDTH;
	precision[c] = DEFPREC;
    }

    for (r = 0; r<=maxrow; r++) {
	register struct ent **p = &tbl[r][0];
	for (c=0; c++<=maxcol; p++)
	    if (*p) {
		if ((*p)->expr) efree ((*p) -> expr);
		if ((*p)->label) xfree ((char *)((*p) -> label));
		xfree ((char *)(*p));
		*p = 0;
	    }
    }
    maxrow = 0;
    maxcol = 0;
    clean_range();
    FullUpdate++;
}

void
backcol(int arg )
{
    while (--arg>=0) {
	if (curcol)
	    curcol--;
	else
	    {error ("At column A"); break;}
	while(col_hidden[curcol] && curcol)
	    curcol--;
    }
}

void
forwcol(int arg )
{
    while (--arg>=0) {
	if (curcol < MAXCOLS - 1)
	    curcol++;
	else
	    {error ("The table can't be any wider"); break;}
	while(col_hidden[curcol]&&(curcol<MAXCOLS-1))
	    curcol++;
    }
}

void
forwrow( int arg )
{
    while (--arg>=0) {
	if (currow < MAXROWS - 1)
	    currow++;
	else
	    {error ("The table can't be any longer"); break;}
	while (row_hidden[currow]&&(currow<MAXROWS-1))
	    currow++;
    }
}

void
backrow(int arg )
{
    while (--arg>=0) {
	if (currow)
	    currow--;
	else
	    {error ("At row zero"); break;}
	while (row_hidden[currow] && currow)
	    currow--;
    }
}


/*
 * Show a cell's label string or expression value.  May overwrite value if
 * there is one already displayed in the cell.  Created from old code in
 * update(), copied with minimal changes.
 */

void
showstring (string, leftflush, hasvalue, row, col, nextcolp, mxcol, fieldlenp, r, c)
    char *string;	/* to display */
    int leftflush;	/* or rightflush */
    int hasvalue;	/* is there a numeric value? */
    int row, col;	/* spreadsheet location */
    int *nextcolp;	/* value returned through it */
    int mxcol;		/* last column displayed? */
    int *fieldlenp;	/* value returned through it */
    int r, c;		/* screen row and column */
{
    register int nextcol  = *nextcolp;
    register int fieldlen = *fieldlenp;

    char field[1024];
    int  slen;
    char *start, *last;
    register char *fp;
    struct ent *nc;

    /* This figures out if the label is allowed to
       slop over into the next blank field */

    slen = strlen (string);
    while ((slen > fieldlen) && (nextcol <= mxcol) &&
	   !((nc = lookat (row, nextcol)) -> flags & is_valid) &&
	   !(nc->label)) {

	if (! col_hidden [nextcol])
	    fieldlen += fwidth [nextcol];

	nextcol++;
    }
    if (slen > fieldlen)
	slen = fieldlen;

    /* Now justify and print */
    start = leftflush ? field : field + fieldlen - slen;
    last = field+fieldlen;
    fp = field;
    while (fp < start)
	*fp++ = ' ';
    while (slen--)
	*fp++ = *string++;
    if ((! hasvalue) || fieldlen != fwidth[col]) 
	while (fp < last)
	    *fp++ = ' ';
    *fp = 0;
#ifdef VMS
    mvaddstr(r, c, field);	/* this is a macro */
#else
    (void) mvaddstr(r, c, field);
#endif

    *nextcolp  = nextcol;
    *fieldlenp = fieldlen;
}

int
etype(register struct enode *e )
{

    if (e==0) return 0;
    switch (e->op) {
    case '+': case '-': case '*': case '/': case '%': case '^':
    case '<': case '=': case '>': case '&': case '|': case 'm':
    case '~': case 'k': case INDEX:
    case REDUCE | '+': case REDUCE | '*': case REDUCE | 'a':
    case REDUCE | 's': case REDUCE | MAX: case REDUCE | MIN:
    case ACOS: case ASIN: case ATAN: case ATAN2: case CEIL:
    case COS: case EXP: case FABS: case FLOOR: case HYPOT:
    case LOG: case LOG10: case POW: case SIN: case SQRT:
    case TAN: case DTR: case RTD: case RND: case FV: case PV:
    case PMT: case HOUR: case MINUTE: case SECOND: case MONTH:
    case DAY: case YEAR: case NOW: case STON: case EQS:
    case LMAX: case LMIN: case NVAL: case LOOKUP:
        return (NUM);

    case O_SCONST: case '#': case DATE: case FMT: case STINDEX:
    case EXT: case SVAL: case SUBSTR:
        return (STR);

    case 'f':  case '?':
        return(etype(e->e.o.left));

    case O_VAR: {
	register struct ent *p;
	p = e->e.v.vp;
	if (p->expr) 
	    return(p->flags & is_strexpr ? STR : NUM);
	else if (p->label)
	    return(STR);
	else
	    return(NUM);
	}

    default:
	return(NUM);
    }
}
