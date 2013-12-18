/*	SC	A Spreadsheet Calculator
 *		Expression interpreter and assorted support routines.
 *
 *		original by James Gosling, September 1982
 *		modified by Mark Weiser and Bruce Israel, 
 *			University of Maryland
 *
 *              More mods Robert Bond, 12/86
 *		More mods by Alan Silverstein, 3-4/88, see list of changes.
 *		$Revision: 1.6 $
 */

#include <math.h>
#include <signal.h>
#include <setjmp.h>
#include <stdio.h>

extern int errno;		/* set by math functions */
#ifdef BSD42
#include <strings.h>
#include <sys/time.h>
#ifndef strchr
#define strchr rindex
#endif
#else
#include <time.h>
#ifndef SYSIII
#include <string.h>
#endif
#endif

#include <curses.h>
#include "sc.h"

#ifndef __HELIOS
# if defined(BSD42) || defined(BSD43)
char *re_comp();
# endif
# if defined(SYSV2) || defined(SYSV3)
char *regcmp();
char *regex();
# endif
#endif

#ifdef SIGVOID
    void quit( int );
#else
    int quit();
#endif

void decompile( register struct enode *e, int priority );
static void decompile_list(struct enode *p);


  /* Use this structure to save the the last 'g' command */

struct go_save {
	int g_type;
	double g_n;
	char *g_s;
	int  g_row;
	int  g_col;
} gs;

/* g_type can be: */

#define G_NONE 0			/* Starting value - must be 0*/
#define G_NUM 1
#define G_STR 2
#define G_CELL 3

jmp_buf fpe_save;
int	exprerr;	   /* Set by eval() and seval() if expression errors */
double  prescale = 1.0;    /* Prescale for constants in let() */
int	extfunc  = 0;	   /* Enable/disable external functions */
int     loading = 0;       /* Set when readfile() is active */

#ifdef SIGVOID
void
#endif
eval_fpe(int dummy) /* Trap for FPE errors in eval */
{
	longjmp(fpe_save, 1);
}

double
fn1_eval(
	 double (*fn)(),
	 double arg )
{
	double res;
	errno = 0;
	res = (*fn)(arg);
	if(errno)
	  eval_fpe(0);

	return res;
}

double
fn2_eval(
	 double (*fn)(),
	 double arg1,
	 double arg2 )
{
	double res;
	errno = 0;
	res = (*fn)(arg1, arg2);
	if(errno) 
	    eval_fpe(0);

	return res;
}

#define PI (double)3.14159265358979323846
#define dtr(x) ((x)*(PI/(double)180.0))
#define rtd(x) ((x)*(180.0/(double)PI))

#ifndef __STDC__ /*__HELIOS*/
# define time_t long
#endif

double
finfunc(
	int 	fun,
	double	v1,
	double	v2,
	double	v3 )
{
 	double answer = 0.0,p;
 
 	p = fn2_eval(pow, 1 + v2, v3);
 
 	switch(fun)
 	{
 	case PV:
 		answer = v1 * (1 - 1/p) / v2;
 		break;
 	case FV:
 		answer = v1 * (p - 1) / v2;
 		break;
 	case PMT:
 		answer = v1 * v2 / (1 - 1/p);
 		break;
	}
	return(answer);
}

char *
dostindex( 
	  double val,
	  int minr,
	  int minc,
	  int maxr,
	  int maxc )
{
    register r,c;
    register struct ent *p;
    char *pr;
    int x;

    x = (int) val;
    r = minr; c = minc;
    p = 0;
    if ( minr == maxr ) { /* look along the row */
	c = minc + x - 1;
	if (c <= maxc && c >=minc)
	    p = tbl[r][c];
    } else if ( minc == maxc ) { /* look down the column */
	r = minr + x - 1;
	if (r <= maxr && r >=minr)
	    p = tbl[r][c];
    } else {
	error ("range specified to @stindex");
	return(0);
    }
    if (p && p->label) {
	pr = xmalloc((unsigned)(strlen(p->label)+1));
	(void)strcpy(pr, p->label);
	return (pr);
     } else
	return(0);
}

double
doindex( 
	double val,
	int minr,
	int minc,
	int maxr,
	int maxc )
{
    double v;
    register r,c;
    register struct ent *p;
    int x;

    x = (int) val;
    v = 0;
    r = minr; c = minc;
    if ( minr == maxr ) { /* look along the row */
	c = minc + x - 1;
	if (c <= maxc && c >=minc 
		&& (p = tbl[r][c]) != NULL && p->flags&is_valid )
					return p->v;
	}
    else if ( minc == maxc ){ /* look down the column */
	r = minr + x - 1;
	if (r <= maxr && r >=minr 
		&& (p = tbl[r][c]) != NULL && p->flags&is_valid )
					return p->v;
	}
    else error(" range specified to @index");
    return v;
}

double
dolookupn( 
	  double val,
	  int minr,
	  int minc,
	  int maxr,
	  int maxc )
{
    double v;
    register r,c;
    register struct ent *p;

    v = 0;
    r = minr; c = minc;
    if ( minr == maxr ) { /* look along the row */
	for ( c = minc; c <= maxc; c++) {
		if ( (p = tbl[r][c]) != NULL && p->flags&is_valid ) {
			if(p->v <= val) {
				p = tbl[r+1][c];
				if ( p && p->flags&is_valid)
					v = p->v;
				}
			else return v;
			}
		}
	}
    else if ( minc == maxc ){ /* look down the column */
	for ( r = minr; r <= maxr; r++) {
		if ( (p = tbl[r][c]) != NULL && p->flags&is_valid ) {
			if(p->v <= val) {
				p = tbl[r][c+1];
				if ( p && p->flags&is_valid)
					v = p->v;
				}
			else return v;
			}
		}
	}
    else error(" range specified to @lookup");
    return v;
}

double
dolookups(
	  char *s,
	  int minr,
	  int minc,
	  int maxr,
	  int maxc )
{
    double v;
    register r,c;
    register struct ent *p;

    v = 0;
    r = minr; c = minc;
    if ( minr == maxr ) { /* look along the row */
	for ( c = minc; c <= maxc; c++) {
	    if ( (p = tbl[r][c]) != NULL && p->label) {
		if(strcmp(s,p->label) == 0) {
		    p = tbl[r+1][c];
		    xfree(s);
		    if ( p && p->flags & is_valid)
			return(p->v);
		}
	    }
	}
    } else if ( minc == maxc ) { /* look down the column */
	for ( r = minr; r <= maxr; r++) {
	    if ( (p = tbl[r][c]) != NULL && p->label) {
		if(strcmp(s,p->label) == 0) {
		    p = tbl[r][c+1];
		    xfree(s);
		    if ( p && p->flags & is_valid)
			return(p->v);
		}
	    }
	}
    } else error(" range specified to @lookup");
    xfree(s);
    return v;
}

double
dosum(
      int minr,
      int minc,
      int maxr,
      int maxc )
{
    double v;
    register r,c;
    register struct ent *p;

    v = 0;
    for (r = minr; r<=maxr; r++)
	for (c = minc; c<=maxc; c++)
	    if ((p = tbl[r][c]) != NULL && p->flags&is_valid)
		v += p->v;
    return v;
}

double
doprod(
       int minr,
       int minc,
       int maxr,
       int maxc )
{
    double v;
    register r,c;
    register struct ent *p;

    v = 1;
    for (r = minr; r<=maxr; r++)
	for (c = minc; c<=maxc; c++)
	    if ((p = tbl[r][c]) != NULL && p->flags&is_valid)
		v *= p->v;
    return v;
}

double
doavg(
      int minr,
      int minc,
      int maxr,
      int maxc )
{
    double v;
    register r,c,count;
    register struct ent *p;

    v = 0;
    count = 0;
    for (r = minr; r<=maxr; r++)
	for (c = minc; c<=maxc; c++)
	    if ((p = tbl[r][c]) != NULL && p->flags&is_valid) {
		v += p->v;
		count++;
	    }

    if (count == 0) 
	return ((double) 0);

    return (v / (double)count);
}

double
dostddev(
	 int minr,
	 int minc,
	 int maxr,
	 int maxc )
{
    double lp, rp, v, nd;
    register r,c,n;
    register struct ent *p;

    n = 0;
    lp = 0;
    rp = 0;
    for (r = minr; r<=maxr; r++)
	for (c = minc; c<=maxc; c++)
	    if ((p = tbl[r][c]) != NULL && p->flags&is_valid) {
		v = p->v;
		lp += v*v;
		rp += v;
		n++;
	    }

    if ((n == 0) || (n == 1)) 
	return ((double) 0);
    nd = (double)n;
    return (sqrt((nd*lp-rp*rp)/(nd*(nd-1))));
}

double
domax(
      int minr,
      int minc,
      int maxr,
      int maxc )
{
    double v = 0.0;
    register r,c,count;
    register struct ent *p;

    count = 0;
    for (r = minr; r<=maxr; r++)
	for (c = minc; c<=maxc; c++)
	    if ((p = tbl[r][c])!= NULL && p->flags&is_valid) {
		if (!count) {
		    v = p->v;
		    count++;
		} else if (p->v > v)
		    v = p->v;
	    }

    if (count == 0) 
	return ((double) 0);

    return (v);
}

double
domin(
      int minr,
      int minc,
      int maxr,
      int maxc )
{
    double v = 0.0;
    register r,c,count;
    register struct ent *p;

    count = 0;
    for (r = minr; r<=maxr; r++)
	for (c = minc; c<=maxc; c++)
	    if ((p = tbl[r][c])!= NULL  && p->flags&is_valid) {
		if (!count) {
		    v = p->v;
		    count++;
		} else if (p->v < v)
		    v = p->v;
	    }

    if (count == 0) 
	return ((double) 0);

    return (v);
}

double
dotime(
       int which,
       double when )
{
#ifndef __STDC__ /* __HELIOS*/
	long time();
#endif
	static long t_cache;
	static struct tm *tp;
	time_t tloc;

	if (which == NOW) 
	    return (double)time(NULL);

	tloc = (time_t)when;

	if (tloc != t_cache) {
	    tp = localtime(&tloc);
	    tp->tm_mon += 1;
	    tp->tm_year += 1900;
	    t_cache = tloc;
	}

	switch (which) {
		case HOUR: return((double)(tp->tm_hour));
		case MINUTE: return((double)(tp->tm_min));
		case SECOND: return((double)(tp->tm_sec));
		case MONTH: return((double)(tp->tm_mon));
		case DAY: return((double)(tp->tm_mday));
		case YEAR: return((double)(tp->tm_year));
	}
	/* Safety net */
	return (0.0);
}

double
doston(char *s)
{
    double v;

    if (!s)
	return((double)0.0);

    (void)strtof(s, &v);
    xfree(s);
    return(v);
}

double
doeqs(
      char *s1,
      char *s2 )
{
    double v;

    if (!s1 && !s2)
	return(1.0);

    if (!s1 || !s2)
	v = 0.0;
    else if (strcmp(s1, s2) == 0)
	v = 1.0;
    else
	v = 0.0;

    if (s1)
    	xfree(s1);

    if (s2)
    	xfree(s2);

    return(v);
}


/*
 * Given a string representing a column name and a value which is a column
 * number, return a pointer to the selected cell's entry, if any, else 0.  Use
 * only the integer part of the column number.  Always free the string.
 */

struct ent *
getent (
    char *colstr,
    double rowdoub)
{
    int collen;		/* length of string */
    int row, col;	/* integer values   */
    struct ent *ep = 0;	/* selected entry   */

    if (((row = (int) floor (rowdoub)) >= 0)
     && (row < MAXROWS)				/* in range */
     && ((collen = strlen (colstr)) <= 2)	/* not too long */
     && ((col = atocol (colstr, collen)) >= 0)
     && (col < MAXCOLS))			/* in range */
    {
	ep = tbl [row] [col];
    }

    xfree (colstr);
    return (ep);
}


/*
 * Given a string representing a column name and a value which is a column
 * number, return the selected cell's numeric value, if any.
 */

double
donval (
    char *colstr,
    double rowdoub )
{
    struct ent *ep;

    return (((ep = getent (colstr, rowdoub)) != NULL && ((ep -> flags) & is_valid)) ?
	    (ep -> v) : 0);
}


/*
 *	The list routines (e.g. dolmax) are called with an LMAX enode.
 *	The left pointer is a chain of ELIST nodes, the right pointer
 *	is a value.
 */
double
dolmax(struct enode *ep)
{
	register int count = 0;
	register double maxval = 0; /* Assignment to shut up lint */
	register struct enode *p;
	register double v;

	for (p = ep; p; p = p->e.o.left) {
		v = eval(p->e.o.right);
		if (!count || v > maxval) {
			maxval = v; count++;
		}
	}
	if (count) return maxval;
	else return 0.0;
}

double
dolmin(struct enode *ep)
{
	register int count = 0;
	register double minval = 0; /* Assignment to shut up lint */
	register struct enode *p;
	register double v;

	for (p = ep; p; p = p->e.o.left) {
		v = eval(p->e.o.right);
		if (!count || v < minval) {
			minval = v; count++;
		}
	}
	if (count) return minval;
	else return 0.0;
}

double hypot(double x, double y)
{  return sqrt(x*x + y*y);
}

double 
eval(register struct enode *e)
{

    if (e==0) return 0;
    switch (e->op) {
	case '+':	return (eval(e->e.o.left) + eval(e->e.o.right));
	case '-':	return (eval(e->e.o.left) - eval(e->e.o.right));
	case '*':	return (eval(e->e.o.left) * eval(e->e.o.right));
	case '/':     	return (eval(e->e.o.left) / eval(e->e.o.right));
	case '%':     {	double num, denom;
			num = floor(eval(e->e.o.left));
			denom = floor(eval (e->e.o.right));
			return denom ? num - floor(num/denom)*denom : 0; }
	case '^':	return (fn2_eval(pow,eval(e->e.o.left),eval(e->e.o.right)));
	case '<':	return (eval(e->e.o.left) < eval(e->e.o.right));
	case '=':	return (eval(e->e.o.left) == eval(e->e.o.right));
	case '>':	return (eval(e->e.o.left) > eval(e->e.o.right));
	case '&':	return (eval(e->e.o.left) && eval(e->e.o.right));
	case '|':	return (eval(e->e.o.left) || eval(e->e.o.right));
	case '?':	return eval(e->e.o.left) ? eval(e->e.o.right->e.o.left)
						: eval(e->e.o.right->e.o.right);
	case 'm':	return (-eval(e->e.o.right));
	case 'f':	return (eval(e->e.o.right));
	case '~':	return (eval(e->e.o.right) == 0.0);
	case 'k':	return (e->e.k);
	case 'v':	return (e->e.v.vp->v);
	case INDEX:
	case LOOKUP:
		    {	register r,c;
		register maxr, maxc;
		register minr, minc;
		maxr = e->e.o.right->e.r.right.vp -> row;
		maxc = e->e.o.right->e.r.right.vp -> col;
		minr = e->e.o.right->e.r.left.vp -> row;
		minc = e->e.o.right->e.r.left.vp -> col;
		if (minr>maxr) r = maxr, maxr = minr, minr = r;
		if (minc>maxc) c = maxc, maxc = minc, minc = c;
		switch(e->op){
		case LOOKUP:
		if (etype(e->e.o.left) == NUM)
	            return dolookupn(eval(e->e.o.left), minr, minc, maxr, maxc);
		else
	            return dolookups(seval(e->e.o.left),minr, minc, maxr, maxc);
		case INDEX:
	        return doindex(eval(e->e.o.left), minr, minc, maxr, maxc);
		}
		}
	case REDUCE | '+':
 	case REDUCE | '*':
 	case REDUCE | 'a':
 	case REDUCE | 's':
	case REDUCE | MAX:
	case REDUCE | MIN:
	    {	register r,c;
		register maxr, maxc;
		register minr, minc;
		maxr = e->e.r.right.vp -> row;
		maxc = e->e.r.right.vp -> col;
		minr = e->e.r.left.vp -> row;
		minc = e->e.r.left.vp -> col;
		if (minr>maxr) r = maxr, maxr = minr, minr = r;
		if (minc>maxc) c = maxc, maxc = minc, minc = c;
	        switch (e->op) {
	            case REDUCE | '+': return dosum(minr, minc, maxr, maxc);
 	            case REDUCE | '*': return doprod(minr, minc, maxr, maxc);
 	            case REDUCE | 'a': return doavg(minr, minc, maxr, maxc);
 	            case REDUCE | 's': return dostddev(minr, minc, maxr, maxc);
 	            case REDUCE | MAX: return domax(minr, minc, maxr, maxc);
 	            case REDUCE | MIN: return domin(minr, minc, maxr, maxc);
		}
	    }
	case ACOS:	 return (fn1_eval( acos, eval(e->e.o.right)));
	case ASIN:	 return (fn1_eval( asin, eval(e->e.o.right)));
	case ATAN:	 return (fn1_eval( atan, eval(e->e.o.right)));
	case ATAN2:	 return (fn2_eval( atan2, eval(e->e.o.left), eval(e->e.o.right)));
	case CEIL:	 return (fn1_eval( ceil, eval(e->e.o.right)));
	case COS:	 return (fn1_eval( cos, eval(e->e.o.right)));
	case EXP:	 return (fn1_eval( exp, eval(e->e.o.right)));
	case FABS:	 return (fn1_eval( fabs, eval(e->e.o.right)));
	case FLOOR:	 return (fn1_eval( floor, eval(e->e.o.right)));
	case HYPOT:	 return (fn2_eval( hypot, eval(e->e.o.left), eval(e->e.o.right)));
	case LOG:	 return (fn1_eval( log, eval(e->e.o.right)));
	case LOG10:	 return (fn1_eval( log10, eval(e->e.o.right)));
	case POW:	 return (fn2_eval( pow, eval(e->e.o.left), eval(e->e.o.right)));
	case SIN:	 return (fn1_eval( sin, eval(e->e.o.right)));
	case SQRT:	 return (fn1_eval( sqrt, eval(e->e.o.right)));
	case TAN:	 return (fn1_eval( tan, eval(e->e.o.right)));
	case DTR:	 return (dtr(eval(e->e.o.right)));
	case RTD:	 return (rtd(eval(e->e.o.right)));
	case RND:	 {
			    double temp;
			    temp = eval(e->e.o.right);
			    return(temp-floor(temp) < 0.5 ?
					     floor(temp) : ceil(temp));
			 }
	case FV:
	case PV:
	case PMT:	return(finfunc(e->op,eval(e->e.o.left),
				   eval(e->e.o.right->e.o.left),
				      eval(e->e.o.right->e.o.right)));
	case HOUR:	 return (dotime(HOUR, eval(e->e.o.right)));
	case MINUTE:	 return (dotime(MINUTE, eval(e->e.o.right)));
	case SECOND:	 return (dotime(SECOND, eval(e->e.o.right)));
	case MONTH:	 return (dotime(MONTH, eval(e->e.o.right)));
	case DAY:	 return (dotime(DAY, eval(e->e.o.right)));
	case YEAR:	 return (dotime(YEAR, eval(e->e.o.right)));
	case NOW:	 return (dotime(NOW, (double)0.0));
	case STON:	 return (doston(seval(e->e.o.right)));
	case EQS:        return (doeqs(seval(e->e.o.right),seval(e->e.o.left)));
	case LMAX:	 return dolmax(e);
	case LMIN:	 return dolmin(e);
	case NVAL:       return (donval(seval(e->e.o.left),eval(e->e.o.right)));
	default:	 error ("Illegal numeric expression");
			 exprerr = 1;
			 return((double)0.0);
    }
#ifdef sequent
    return((double)0.0);	/* Quiet a questionable compiler complaint */
#endif
}

/* 
 * Rules for string functions:
 * Take string arguments which they xfree.
 * All returned strings are assumed to be xalloced.
 */

char *
docat(
      register char *s1,
      register char *s2 )
{
    register char *p;
    char *arg1, *arg2;

    if (!s1 && !s2)
	return(0);
    arg1 = s1 ? s1 : "";
    arg2 = s2 ? s2 : "";
    p = xmalloc((unsigned)(strlen(arg1)+strlen(arg2)+1));
    (void) strcpy(p, arg1);
    (void) strcat(p, arg2);
    if (s1)
        xfree(s1);
    if (s2)
        xfree(s2);
    return(p);
}

char *
dodate(time_t tloc)
{
    char *tp;
    char *p;

    tp = ctime(&tloc);
    tp[24] = 0;
    p = xmalloc((unsigned)25);
    (void) strcpy(p, tp);
    return(p);
}


char *
dofmt(
      char *fmtstr,
      double v )
{
    char buff[1024];
    char *p;

    if (!fmtstr)
	return(0);
    (void)sprintf(buff, fmtstr, v);
    p = xmalloc((unsigned)(strlen(buff)+1));
    (void) strcpy(p, buff);
    xfree(fmtstr);
    return(p);
}


/*
 * Given a command name and a value, run the command with the given value and
 * read and return its first output line (only) as an allocated string, always
 * a copy of prevstr, which is set appropriately first unless external
 * functions are disabled, in which case the previous value is used.  The
 * handling of prevstr and freeing of command is tricky.  Returning an
 * allocated string in all cases, even if null, insures cell expressions are
 * written to files, etc.
 */

#ifdef VMS
char *
doext(command, value)
char *command;
double value;
{
    error("Warning: External functions unavailable on VMS");
    if (command)
	xfree(command);
    return (strcpy (xmalloc((unsigned) 1), "\0"));
}

#else /* VMS */

char *
doext (
       char   *command,
       double value )
{
    static char *prevstr = 0;	/* previous result */
    char buff[1024];		/* command line/return, not permanently alloc */

    if (!prevstr) {
	prevstr = xmalloc((unsigned)1);
	*prevstr = 0;
    }
    if (!extfunc)    {
	error ("Warning: external functions disabled; using %s value",
		prevstr ? "previous" : "null");

	if (command) xfree (command);
    } else {
	if (prevstr) xfree (prevstr);		/* no longer needed */
	prevstr = 0;

	if ((! command) || (! *command)) {
	    error ("Warning: external function given null command name");
	    if (command) xfree (command);
	} else {
	    FILE *pp;

	    (void) sprintf (buff, "%s %g", command, value); /* build cmd line */
	    xfree (command);

	    error ("Running external function...");
	    (void) refresh();

	    if ((pp = popen (buff, "r")) == (FILE *) NULL)	/* run it */
		error ("Warning: running \"%s\" failed", buff);
	    else {
		if (fgets (buff, 1024, pp) == NULL)	/* one line */
		    error ("Warning: external function returned nothing");
		else {
		    char *cp;

		    error ("");				/* erase notice */
		    buff[1023] = 0;

		    if ((cp = strchr (buff, '\n')) != NULL)	/* contains newline */
			*cp = 0;			/* end string there */

		    (void) strcpy (prevstr = 
			 xmalloc ((unsigned) (strlen (buff) + 1)), buff);
			 /* save alloc'd copy */
		}
		(void) pclose (pp);

	    } /* else */
	} /* else */
    } /* else */
    return (strcpy (xmalloc ((unsigned) (strlen (prevstr) + 1)), prevstr));
}

#endif /* VMS */


/*
 * Given a string representing a column name and a value which is a column
 * number, return the selected cell's string value, if any.  Even if none,
 * still allocate and return a null string so the cell has a label value so
 * the expression is saved in a file, etc.
 */

char *
dosval (
	char *colstr,
	double rowdoub )
{
    struct ent *ep;
    char *label;

    label = (ep = getent (colstr, rowdoub)) != NULL ? (ep -> label) : "";
    return (strcpy (xmalloc ((unsigned) (strlen (label) + 1)), label));
}


/*
 * Substring:  Note that v1 and v2 are one-based to users, but zero-based
 * when calling this routine.
 */

char *
dosubstr(
	 char *s,
	 register int v1,
	 register int v2 )
{
    register char *s1, *s2;
    char *p;

    if (!s)
	return(0);

    if (v2 >= strlen (s))		/* past end */
	v2 =  strlen (s) - 1;		/* to end   */

    if (v1 < 0 || v1 > v2) {		/* out of range, return null string */
	xfree(s);
	p = xmalloc((unsigned)1);
	p[0] = 0;
	return(p);
    }
    s2 = p = xmalloc((unsigned)(v2-v1+2));
    s1 = &s[v1];
    for(; v1 <= v2; s1++, s2++, v1++)
	*s2 = *s1;
    *s2 = 0;
    xfree(s);
    return(p);
}

char *
seval(register struct enode *se )
{
    register char *p;

    if (se==0) return 0;
    switch (se->op) {
	case O_SCONST: p = xmalloc((unsigned)(strlen(se->e.s)+1));
		     (void) strcpy(p, se->e.s);
		     return(p);
	case O_VAR:    {
			struct ent *ep;
			ep = se->e.v.vp;

			if (!ep->label)
			    return(0);
			p = xmalloc((unsigned)(strlen(ep->label)+1));
			(void) strcpy(p, ep->label);
			return(p);
		     }
	case '#':    return(docat(seval(se->e.o.left), seval(se->e.o.right)));
	case 'f':    return(seval(se->e.o.right));
	case '?':    return(eval(se->e.o.left) ? seval(se->e.o.right->e.o.left)
					     : seval(se->e.o.right->e.o.right));
	case DATE:   return(dodate((time_t)(eval(se->e.o.right))));
	case FMT:    return(dofmt(seval(se->e.o.left), eval(se->e.o.right)));
 	case STINDEX:
 		{	register r,c;
 		register maxr, maxc;
 		register minr, minc;
 		maxr = se->e.o.right->e.r.right.vp -> row;
 		maxc = se->e.o.right->e.r.right.vp -> col;
 		minr = se->e.o.right->e.r.left.vp -> row;
 		minc = se->e.o.right->e.r.left.vp -> col;
 		if (minr>maxr) r = maxr, maxr = minr, minr = r;
 		if (minc>maxc) c = maxc, maxc = minc, minc = c;
 	        return dostindex(eval(se->e.o.left), minr, minc, maxr, maxc);
		}
	case EXT:    return(doext(seval(se->e.o.left), eval(se->e.o.right)));
	case SVAL:   return(dosval(seval(se->e.o.left), eval(se->e.o.right)));
	case SUBSTR: return(dosubstr(seval(se->e.o.left),
			    (int)eval(se->e.o.right->e.o.left) - 1,
			    (int)eval(se->e.o.right->e.o.right) - 1));
	default:
		     error ("Illegal string expression");
		     exprerr = 1;
		     return(0);
	}
}

/*
 * The graph formed by cell expressions which use other cells's values is not
 * evaluated "bottom up".  The whole table is merely re-evaluated cell by cell,
 * top to bottom, left to right, in RealEvalAll().  Each cell's expression uses
 * constants in other cells.  However, RealEvalAll() notices when a cell gets a
 * new numeric or string value, and reports if this happens for any cell.
 * EvalAll() repeats calling RealEvalAll() until there are no changes or the
 * evaluation count expires.
 */

int propagation = 10;	/* max number of times to try calculation */

void
setiterations(int i)
{
	if(i<1){
		error("iteration count must be at least 1");
		propagation = 1;
		}
	else propagation = i;
}

void
RealEvalOne(
	    register struct ent *p,
	    int i,
	    int j,
	    int *chgct )
{
	if (p->flags & is_strexpr) {
	    char *v;
	    if (setjmp(fpe_save)) {
		error("Floating point exception %s", v_name( i, j));
		v = "";
	    } else {
		v = seval(p->expr);
	    }
	    if (!v && !p->label) /* Everything's fine */
		return;
	    if (!p->label || !v || strcmp(v, p->label) != 0) {
		(*chgct)++;
		p->flags |= is_changed;
	    }
	    if(p->label)
		xfree(p->label);
	    p->label = v;
	} else {
	    double v;
	    if (setjmp(fpe_save)) {
		error("Floating point exception %s", v_name( i, j));
		v = 0.0;
	    } else {
		v = eval (p->expr);
	    }
	    if (v != p->v) {
		p->v = v; (*chgct)++;
		p->flags |= is_changed|is_valid;
	    }
	}
}

void
EvalAll ( void )
{
      int lastcnt, repct = 0;
  
     while ((lastcnt = RealEvalAll()) != NULL && (repct++ <= propagation));
     if((propagation>1)&& (lastcnt >0 ))
 	    error("Still changing after %d iterations",propagation-1);
}

/*
 * Evaluate all cells which have expressions and alter their numeric or string
 * values.  Return the number of cells which changed.
 */

int 
RealEvalAll () {
    register int i,j;
    int chgct = 0;
    register struct ent *p;

    (void) signal(SIGFPE, eval_fpe);
    if(calc_order == BYROWS ) {
    for (i=0; i<=maxrow; i++)
  	for (j=0; j<=maxcol; j++)
 	    if ((p=tbl[i][j]) != NULL && p->expr) RealEvalOne(p,i,j, &chgct);
    }
    else if ( calc_order == BYCOLS ) {
    for (j=0; j<=maxcol; j++)
 	for (i=0; i<=maxrow; i++)
 	    if ((p=tbl[i][j]) != NULL && p->expr) RealEvalOne(p,i,j, &chgct);
    }
    else error("Internal error calc_order");
 
    (void) signal(SIGFPE, quit);
    return(chgct);
}


struct enode *
new_type(
    int op,
    struct enode *a1,
    struct enode *a2 )
{
    register struct enode *p;
    p = (struct enode *) xmalloc ((unsigned)sizeof (struct enode));
    p->op = op;
    p->e.o.left = a1;
    p->e.o.right = a2;
    return p;
}

struct enode *
new_var(
	int op,
	struct ent_ptr a1 )
{
    register struct enode *p;
    p = (struct enode *) xmalloc ((unsigned)sizeof (struct enode));
    p->op = op;
    p->e.v = a1;
    return p;
}

struct enode *  
new_range(
	  int op,
	  struct range_s a1 )
{
    register struct enode *p;
    p = (struct enode *) xmalloc ((unsigned)sizeof (struct enode));
    p->op = op;
    p->e.r = a1;
    return p;
}

struct enode *
new_const(
	  int op,
	  double a1 )
{
    register struct enode *p;
    p = (struct enode *) xmalloc ((unsigned)sizeof (struct enode));
    p->op = op;
    p->e.k = a1;
    return p;
}

struct enode *
new_str(char *s)
{
    register struct enode *p;

    p = (struct enode *) xmalloc ((unsigned)sizeof(struct enode));
    p->op = O_SCONST;
    p->e.s = s;
    return(p);
}

void
efree (register struct enode *e)
{
    if (e) {
	if (e->op != O_VAR && e->op !=O_CONST && e->op != O_SCONST
		&& (e->op & REDUCE) != REDUCE) {
	    efree(e->e.o.left);
	    efree(e->e.o.right);
	}
	if (e->op == O_SCONST && e->e.s)
	    xfree(e->e.s);
	xfree ((char *)e);
    }
}

void
label (
       register struct ent *v,
       register char *s,
       int flushdir )
{
    if (v) {
	if (flushdir==0 && v->flags&is_valid) {
	    register struct ent *tv;
	    if (v->col>0 && ((tv=lookat(v->row,v->col-1))->flags&is_valid)==0)
		v = tv, flushdir = 1;
	    else if (((tv=lookat (v->row,v->col+1))->flags&is_valid)==0)
		v = tv, flushdir = -1;
	    else flushdir = -1;
	}
	if (v->label) xfree((char *)(v->label));
	if (s && s[0]) {
	    v->label = xmalloc ((unsigned)(strlen(s)+1));
	    (void) strcpy (v->label, s);
	} else
	    v->label = 0;
	if (flushdir<0) v->flags |= is_leftflush;
	else v->flags &= ~is_leftflush;
	FullUpdate++;
	modflg++;
    }
}

void
clearent (struct ent *v)
{
    if (!v)
	return;
    label(v,"",-1);
    v->v = 0;
    if (v->expr)
	efree(v->expr);
    v->expr = 0;
    v->flags |= (is_changed);
    v->flags &= ~(is_valid);
    changed++;
    modflg++;
}

void
copyrtv(
	int vr,
	int vc,
	int minsr,
	int minsc,
	int maxsr,
	int maxsc )
{
    register struct ent *p;
    register struct ent *n;
    register int sr, sc;
    register int dr, dc;

    for (dr=vr, sr=minsr; sr<=maxsr; sr++, dr++)
	for (dc=vc, sc=minsc; sc<=maxsc; sc++, dc++) {
	    n = lookat (dr, dc);
	    (void) clearent(n);
	    if ((p = tbl[sr][sc]) != NULL)
		copyent( n, p, dr - sr, dc - sc);
	}
}

void
copy(
     struct ent *dv1,
     struct ent *dv2,
     struct ent *v1,
     struct ent *v2 )
{
    int minsr, minsc;
    int maxsr, maxsc;
    int mindr, mindc;
    int maxdr, maxdc;
    int vr, vc;
    int r, c;

    mindr = dv1->row;
    mindc = dv1->col;
    maxdr = dv2->row;
    maxdc = dv2->col;
    if (mindr>maxdr) r = maxdr, maxdr = mindr, mindr = r;
    if (mindc>maxdc) c = maxdc, maxdc = mindc, mindc = c;
    maxsr = v2->row;
    maxsc = v2->col;
    minsr = v1->row;
    minsc = v1->col;
    if (minsr>maxsr) r = maxsr, maxsr = minsr, minsr = r;
    if (minsc>maxsc) c = maxsc, maxsc = minsc, minsc = c;
    if (maxdr >= MAXROWS  || 
           maxdc >= MAXCOLS) {
	error ("The table can't be any bigger");
	return;
    }
    erase_area(mindr, mindc, maxdr, maxdc);
    if (minsr == maxsr && minsc == maxsc) {
	/* Source is a single cell */
	for(vr = mindr; vr <= maxdr; vr++)
	    for (vc = mindc; vc <= maxdc; vc++)
		copyrtv(vr, vc, minsr, minsc, maxsr, maxsc);
    } else if (minsr == maxsr) {
	/* Source is a single row */
	for (vr = mindr; vr <= maxdr; vr++)
	    copyrtv(vr, mindc, minsr, minsc, maxsr, maxsc);
    } else if (minsc == maxsc) {
	/* Source is a single column */
	for (vc = mindc; vc <= maxdc; vc++)
	    copyrtv(mindr, vc, minsr, minsc, maxsr, maxsc);
    } else {
	/* Everything else */
	copyrtv(mindr, mindc, minsr, minsc, maxsr, maxsc);
    }
    sync_refs();
}


void
eraser(
       struct ent *v1,
       struct ent *v2 )
{
	FullUpdate++;
	flush_saved();
	erase_area(v1->row, v1->col, v2->row, v2->col);
	sync_refs();
}

/* Goto subroutines */
void
g_free()
{
    switch (gs.g_type) {
    case G_STR: xfree(gs.g_s); break;
    default: break;
    }
    gs.g_type = G_NONE;
}

void
moveto(
       int row,
       int col )
{
    currow = row;
    curcol = col;
    g_free();
    gs.g_type = G_CELL;
    gs.g_row = currow;
    gs.g_col = curcol;
}

void
num_search(double n)
{
    register struct ent *p;
    register int r,c;

    g_free();
    gs.g_type = G_NUM;
    gs.g_n = n;

    r = currow;
    c = curcol;
    do {
	if (c < maxcol)
	    c++;
	else {
	    if (r < maxrow) {
		while(++r < maxrow && row_hidden[r]) /* */;
		c = 0;
	    } else {
		r = 0;
		c = 0;
	    }
	}
	if (r == currow && c == curcol) {
	    error("Number not found");
	    return;
	}
	p = tbl[r][c];
    } while(col_hidden[c] || !p || p && (!(p->flags & is_valid) 
                                        || (p->flags&is_valid) && p->v != n));
    currow = r;
    curcol = c;
}


void
str_search(char *s)
{
    register struct ent *p;
    register int r,c;
#if defined SYSV2 || defined SYSV3
    char *tmp;
#endif
    

#ifndef __HELIOS
# if defined(BSD42) || defined(BSD43)
    if ((tmp = re_comp(s)) != (char *)0) {
	xfree(s);
	error(tmp);
	return;
    }
# endif
# if defined(SYSV2) || defined(SYSV3)
    if ((tmp = regcmp(s, (char *)0)) == (char *)0) {
	xfree(s);
	error("Invalid search string");
	return;
    }
# endif
#endif
    g_free();
    gs.g_type = G_STR;
    gs.g_s = s;
    r = currow;
    c = curcol;
    do {
	if (c < maxcol)
	    c++;
	else {
	    if (r < maxrow) {
		while(++r < maxrow && row_hidden[r]) /* */;
		c = 0;
	    } else {
		r = 0;
		c = 0;
	    }
	}
	if (r == currow && c == curcol) {
	    error("String not found");
#if defined(SYSV2) || defined(SYSV3)
	    free(tmp);
#endif
	    return;
	}
	p = tbl[r][c];
    } while(col_hidden[c] || !p || p && (!(p->label) 
#ifdef __HELIOS /* do before as __BSD defines BSD43 */
                                       || (strcmp(s, p->label) != 0)));
#else
# if defined(BSD42) || defined(BSD43)
		  			|| (re_exec(p->label) == 0)));
# else
#  if defined(SYSV2) || defined(SYSV3)
                                       || (regex(tmp, p->label) == (char *)0)));
#  else
                                       || (strcmp(s, p->label) != 0)));
#  endif
# endif
#endif
    currow = r;
    curcol = c;
#if defined(SYSV2) || defined(SYSV3)
    free(tmp);
#endif
}

void
go_last()
{
    switch (gs.g_type) {
    case G_NONE:
		error("Nothing to repeat"); break;
    case G_NUM:
		num_search(gs.g_n);
		break;
    case  G_CELL:
		moveto(gs.g_row, gs.g_col);
	    	break;
    case  G_STR: 
		gs.g_type = G_NONE;	/* Don't free the string */
   	    	str_search(gs.g_s); 
	   	break;

    default: error("go_last: internal error");
    }
}

void
fill (
      struct ent *v1,
      struct ent *v2,
      double start,
      double inc )
{
    register r,c;
    register struct ent *n;
    int maxr, maxc;
    int minr, minc;

    maxr = v2->row;
    maxc = v2->col;
    minr = v1->row;
    minc = v1->col;
    if (minr>maxr) r = maxr, maxr = minr, minr = r;
    if (minc>maxc) c = maxc, maxc = minc, minc = c;
    if (maxr >= MAXROWS) maxr = MAXROWS-1;
    if (maxc >= MAXCOLS) maxc = MAXCOLS-1;
    if (minr < 0) minr = 0;
    if (minr < 0) minr = 0;

    FullUpdate++;
    if( calc_order == BYROWS ) {
    for (r = minr; r<=maxr; r++)
	for (c = minc; c<=maxc; c++) {
	    n = lookat (r, c);
	    (void) clearent(n);
	    n->v = start;
	    start += inc;
	    n->flags |= (is_changed|is_valid);
	}
    }
    else if ( calc_order == BYCOLS ) {
    for (c = minc; c<=maxc; c++)
	for (r = minr; r<=maxr; r++) {
	    n = lookat (r, c);
	    (void) clearent(n);
	    n->v = start;
	    start += inc;
	    n->flags |= (is_changed|is_valid);
	}
    }
    else error(" Internal error calc_order");
}

/*
 * Say if an expression is a constant (return 1) or not.
 */

int
constant (register struct enode *e)
{
    return ((e == 0)
	 || ((e -> op) == O_CONST)
	 || ((e -> op) == O_SCONST)
	 || (((e -> op) != O_VAR)
	  && (((e -> op) & REDUCE) != REDUCE)
	  && constant (e -> e.o.left)
	  && constant (e -> e.o.right)
	  && (e -> op != EXT)	 /* functions look like constants but aren't */
	  && (e -> op != NVAL)
	  && (e -> op != SVAL)
	  && (e -> op != NOW)));
}

void
let (
     struct ent *v,
     struct enode *e )
{
    double val;

    exprerr = 0;
    (void) signal(SIGFPE, eval_fpe);
    if (setjmp(fpe_save)) {
	error ("Floating point exception in cell %s", v_name(v->row, v->col));
	val = 0.0;
    } else {
	val = eval(e);
    }
    (void) signal(SIGFPE, quit);
    if (exprerr) {
	efree(e);
	return;
    }
    if (constant(e)) {
	if (!loading)
	    v->v = val * prescale;
	else
	    v->v = val;
	if (!(v->flags & is_strexpr)) {
            efree (v->expr);
	    v->expr = 0;
	}
	efree(e);
        v->flags |= (is_changed|is_valid);
        changed++;
        modflg++;
	return;
    }
    efree (v->expr);
    v->expr = e;
    v->flags |= (is_changed|is_valid);
    v->flags &= ~is_strexpr;
    changed++;
    modflg++;
}

void
slet (
      struct ent *v,
      struct enode *se,
      int flushdir )
{
    char *p;

    exprerr = 0;
    (void) signal(SIGFPE, eval_fpe);
    if (setjmp(fpe_save)) {
	error ("Floating point exception in cell %s", v_name(v->row, v->col));
	p = "";
    } else {
	p = seval(se);
    }
    (void) signal(SIGFPE, quit);
    if (exprerr) {
	efree(se);
	return;
    }
    if (constant(se)) {
	label(v, p, flushdir);
	if (p)
	    xfree(p);
	efree(se);
	if (v->flags & is_strexpr) {
            efree (v->expr);
	    v->expr = 0;
	    v->flags &= ~is_strexpr;
	}
	return;
    }
    efree (v->expr);
    v->expr = se;
    v->flags |= (is_changed|is_strexpr);
    if (flushdir<0) v->flags |= is_leftflush;
    else v->flags &= ~is_leftflush;
    FullUpdate++;
    changed++;
    modflg++;
}

void
hide_row(int arg)
{
    if (arg < 0) {
	error("Invalid Range");
	return;
    }
    if (arg > MAXROWS-2) {
	error("You can't hide the last row");
	return;
    }
    FullUpdate++;
    row_hidden[arg] = 1;
}

void
hide_col(int arg)
{
    if (arg < 0) {
	error("Invalid Range");
	return;
    }
    if (arg > MAXCOLS-2) {
	error("You can't hide the last col");
	return;
    }
    FullUpdate++;
    col_hidden[arg] = 1;
}

void
decodev (struct ent_ptr v)
{
	register struct range *r;

	if (!v.vp) (void)sprintf (line+linelim,"VAR?");
	else if ((r = find_range((char *)0, 0, v.vp, v.vp)) != NULL)
	    (void)sprintf(line+linelim, "%s", r->r_name);
	else
	    (void)sprintf (line+linelim, "%s%s%s%d",
			v.vf & FIX_COL ? "$" : "",
			coltoa(v.vp->col),
			v.vf & FIX_ROW ? "$" : "",
			v.vp->row);
	linelim += strlen (line+linelim);
}

char *
coltoa(int col)
{
    static char rname[3];
    register char *p = rname;

    if (col > 25) {
	*p++ = col/26 + 'A' - 1;
	col %= 26;
    }
    *p++ = col+'A';
    *p = 0;
    return(rname);
}

void
one_arg(
	char *s,
	struct enode *e )
{
    for (; (line[linelim++] = *s++) != 0;);
    linelim--;
    decompile (e->e.o.right, 0);
    line[linelim++] = ')';
}

void
two_arg(
	char *s,
	struct enode *e )
{
    for (; (line[linelim++] = *s++) != 0;);
    linelim--;
    decompile (e->e.o.left, 0);
    line[linelim++] = ',';
    decompile (e->e.o.right, 0);
    line[linelim++] = ')';
}

void
three_arg(
	  char *s,
	  struct enode *e )
{
    for (; (line[linelim++] = *s++) != 0;);
    linelim--;
    decompile (e->e.o.left, 0);
    line[linelim++] = ',';
    decompile (e->e.o.right->e.o.left, 0);
    line[linelim++] = ',';
    decompile (e->e.o.right->e.o.right, 0);
    line[linelim++] = ')';
}

void
range_arg(
	  char *s,
	  struct enode *e )
{
    struct range *r;

    for (; (line[linelim++] = *s++) != 0;);
    linelim--;
    if ((r = find_range((char *)0, 0, e->e.r.left.vp,
			     e->e.r.right.vp)) != NULL) {
	(void)sprintf(line+linelim, "%s", r->r_name);
	linelim += strlen(line+linelim);
    } else {
	decodev (e->e.r.left);
	line[linelim++] = ':';
	decodev (e->e.r.right);
    }
    line[linelim++] = ')';
}

void
index_arg(
	  char *s,
	  struct enode *e )
{
    for (; (line[linelim++] = *s++) != 0;);
    linelim--;
    decompile( e-> e.o.left, 0 );
    range_arg(", ", e->e.o.right);
}

void
list_arg(
	 char *s,
	 struct enode *e )
{
    for (; (line[linelim++] = *s++) != 0;);
    linelim--;

    decompile (e->e.o.right, 0);
    line[linelim++] = ',';
    decompile_list(e->e.o.left);
    line[linelim - 1] = ')';
}

void
decompile(
	  register struct enode *e,
	  int priority )
{
    register char *s;
    if (e) {
	int mypriority;
	switch (e->op) {
	default: mypriority = 99; break;
	case '?': mypriority = 1; break;
	case ':': mypriority = 2; break;
	case '|': mypriority = 3; break;
	case '&': mypriority = 4; break;
	case '<': case '=': case '>': mypriority = 6; break;
	case '+': case '-': case '#': mypriority = 8; break;
	case '*': case '/': case '%': mypriority = 10; break;
	case '^': mypriority = 12; break;
	}
	if (mypriority<priority) line[linelim++] = '(';
	switch (e->op) {
	case 'f':	for (s="fixed "; (line[linelim++] = *s++) != 0;);
			linelim--;
			decompile (e->e.o.right, 30);
			break;
	case 'm':	line[linelim++] = '-';
			decompile (e->e.o.right, 30);
			break;
	case '~':	line[linelim++] = '~';
			decompile (e->e.o.right, 30);
			break;
	case 'v':	decodev (e->e.v);
			break;
	case 'k':	(void)sprintf (line+linelim,"%.15g",e->e.k);
			linelim += strlen (line+linelim);
			break;
	case '$':	(void)sprintf (line+linelim, "\"%s\"", e->e.s);
			linelim += strlen(line+linelim);
			break;

	case REDUCE | '+': range_arg( "@sum(", e); break;
	case REDUCE | '*': range_arg( "@prod(", e); break;
	case REDUCE | 'a': range_arg( "@avg(", e); break;
	case REDUCE | 's': range_arg( "@stddev(", e); break;
	case REDUCE | MAX: range_arg( "@max(", e); break;
	case REDUCE | MIN: range_arg( "@min(", e); break;

	case ACOS:	one_arg( "@acos(", e); break;
	case ASIN:	one_arg( "@asin(", e); break;
	case ATAN:	one_arg( "@atan(", e); break;
	case ATAN2:	two_arg( "@atan2(", e); break;
	case CEIL:	one_arg( "@ceil(", e); break;
	case COS:	one_arg( "@cos(", e); break;
	case EXP:	one_arg( "@exp(", e); break;
	case FABS:	one_arg( "@fabs(", e); break;
	case FLOOR:	one_arg( "@floor(", e); break;
	case HYPOT:	two_arg( "@hypot(", e); break;
	case LOG:	one_arg( "@ln(", e); break;
	case LOG10:	one_arg( "@log(", e); break;
	case POW:	two_arg( "@pow(", e); break;
	case SIN:	one_arg( "@sin(", e); break;
	case SQRT:	one_arg( "@sqrt(", e); break;
	case TAN:	one_arg( "@tan(", e); break;
	case DTR:	one_arg( "@dtr(", e); break;
	case RTD:	one_arg( "@rtd(", e); break;
	case RND:	one_arg( "@rnd(", e); break;
	case HOUR:	one_arg( "@hour(", e); break;
	case MINUTE:	one_arg( "@minute(", e); break;
	case SECOND:	one_arg( "@second(", e); break;
	case MONTH:	one_arg( "@month(", e); break;
	case DAY:	one_arg( "@day(", e); break;
	case YEAR:	one_arg( "@year(", e); break;
	case DATE:	one_arg( "@date(", e); break;
	case STON:	one_arg( "@ston(", e); break;
	case FMT:	two_arg( "@fmt(", e); break;
	case EQS:	two_arg( "@eqs(", e); break;
	case NOW:	for ( s = "@now"; (line[linelim++] = *s++) != 0;);
			linelim--;
			break;
	case LMAX:	list_arg("@max(", e); break;
	case LMIN: 	list_arg("@min(", e); break;
	case FV:	three_arg("@fv(", e); break;
	case PV:	three_arg("@pv(", e); break;
	case PMT:	three_arg("@pmt(", e); break;
	case NVAL:	two_arg("@nval(", e); break;
	case SVAL:	two_arg("@sval(", e); break;
	case EXT:	two_arg("@ext(", e); break;
	case SUBSTR:	three_arg("@substr(", e); break;
	case STINDEX:	index_arg("@stindex(", e); break;
	case INDEX:	index_arg("@index(", e); break;
	case LOOKUP:	index_arg("@lookup(", e); break;

	default:	decompile (e->e.o.left, mypriority);
			line[linelim++] = e->op;
			decompile (e->e.o.right, mypriority+1);
			break;

	}
	if (mypriority<priority) line[linelim++] = ')';
    } else line[linelim++] = '?';
}

/*
 *	To make list elements come out in the same order
 *	they were entered, we must do a depth-first eval
 *	of the ELIST tree
 */
static void
decompile_list(struct enode *p)
{
	if (!p) return;
	decompile_list(p->e.o.left);	/* depth first */
        decompile(p->e.o.right, 0);
	line[linelim++] = ',';
}

void
editexp(
	int row,
	int col )
{
    register struct ent *p;

    p = lookat (row, col);
    decompile (p->expr, 0);
    line[linelim] = 0;
}

void
editv(
      int row,
      int col )
{
    register struct ent *p;

    p = lookat (row, col);
    (void)sprintf (line, "let %s = ", v_name(row, col));
    linelim = strlen(line);
    if (p->flags & is_strexpr || p->expr == 0) {
	(void)sprintf (line+linelim, "%.15g", p->v);
	linelim += strlen (line+linelim);
    } else {
        editexp(row,col);
    }
}

void
edits (
       int row,
       int col )
{
    register struct ent *p;

    p = lookat (row, col);
    (void)sprintf (line, "%sstring %s = ",
			((p->flags&is_leftflush) ? "left" : "right"),
			v_name(row, col));
    linelim = strlen(line);
    if (p->flags & is_strexpr && p->expr) {
	editexp(row, col);
    } else if (p->label) {
        (void)sprintf (line+linelim, "\"%s\"", p->label);
        linelim += strlen (line+linelim);
    } else {
        (void)sprintf (line+linelim, "\"");
        linelim += 1;
    }
}
