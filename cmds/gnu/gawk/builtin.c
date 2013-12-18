/*
 * builtin.c - Builtin functions and various utility procedures 
 */

/* 
 * Copyright (C) 1986, 1988, 1989, 1991 the Free Software Foundation, Inc.
 * 
 * This file is part of GAWK, the GNU implementation of the
 * AWK Progamming Language.
 * 
 * GAWK is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 1, or (at your option)
 * any later version.
 * 
 * GAWK is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with GAWK; see the file COPYING.  If not, write to
 * the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "awk.h"

#ifndef atarist
extern void srandom P((int seed));
#endif
extern char *initstate P((unsigned seed, char *state, int n));
extern char *setstate P((char *state));
extern long random P((void));

extern NODE **fields_arr;
extern int output_is_tty;

static NODE *sub_common P((NODE *tree, int global));

#ifdef GFMT_WORKAROUND
char *gfmt P((double g, int prec, char *buf));
#endif

#ifdef _CRAY
/* Work around a problem in conversion of doubles to exact integers. */
#include <float.h>
#define Floor(n) floor((n) * (1.0 + DBL_EPSILON))
#define Ceil(n) ceil((n) * (1.0 + DBL_EPSILON))

/* Force the standard C compiler to use the library math functions. */
extern double exp(double);
double (*Exp)() = exp;
#define exp(x) (*Exp)(x)
extern double log(double);
double (*Log)() = log;
#define log(x) (*Log)(x)
#else
#define Floor(n) floor(n)
#define Ceil(n) ceil(n)
#endif

/* Builtin functions */
NODE *
do_exp(tree)
NODE *tree;
{
	NODE *tmp;
	double d, res;
#ifndef exp
	double exp();
#endif

	tmp= tree_eval(tree->lnode);
	d = force_number(tmp);
	free_temp(tmp);
	errno = 0;
	res = exp(d);
	if (errno == ERANGE)
		warning("exp argument %g is out of range", d);
	return tmp_number((AWKNUM) res);
}

NODE *
do_index(tree)
NODE *tree;
{
	NODE *s1, *s2;
	register char *p1, *p2;
	register int l1, l2;
	long ret;


	s1 = tree_eval(tree->lnode);
	s2 = tree_eval(tree->rnode->lnode);
	force_string(s1);
	force_string(s2);
	p1 = s1->stptr;
	p2 = s2->stptr;
	l1 = s1->stlen;
	l2 = s2->stlen;
	ret = 0;
	if (IGNORECASE) {
		while (l1) {
			if (l2 > l1)
				break;
			if (casetable[*p1] == casetable[*p2]
			    && strncasecmp(p1, p2, l2) == 0) {
				ret = 1 + s1->stlen - l1;
				break;
			}
			l1--;
			p1++;
		}
	} else {
		while (l1) {
			if (l2 > l1)
				break;
			if (STREQN(p1, p2, l2)) {
				ret = 1 + s1->stlen - l1;
				break;
			}
			l1--;
			p1++;
		}
	}
	free_temp(s1);
	free_temp(s2);
	return tmp_number((AWKNUM) ret);
}

NODE *
do_int(tree)
NODE *tree;
{
	NODE *tmp;
	double floor();
	double ceil();
	double d;

	tmp = tree_eval(tree->lnode);
	d = force_number(tmp);
	if (d >= 0)
		d = Floor(d);
	else
		d = Ceil(d);
	free_temp(tmp);
	return tmp_number((AWKNUM) d);
}

NODE *
do_length(tree)
NODE *tree;
{
	NODE *tmp;
	int len;

	tmp = tree_eval(tree->lnode);
	len = force_string(tmp)->stlen;
	free_temp(tmp);
	return tmp_number((AWKNUM) len);
}

NODE *
do_log(tree)
NODE *tree;
{
	NODE *tmp;
#ifndef log
	double log();
#endif
	double d, arg;

	tmp = tree_eval(tree->lnode);
	arg = (double) force_number(tmp);
	if (arg < 0.0)
		warning("log called with negative argument %g", arg);
	d = log(arg);
	free_temp(tmp);
	return tmp_number((AWKNUM) d);
}

/* %e and %f formats are not properly implemented.  Someone should fix them */
/* Actually, this whole thing should be reimplemented. */

NODE *
do_sprintf(tree)
NODE *tree;
{
#define bchunk(s,l) if(l) {\
    while((l)>ofre) {\
      erealloc(obuf, char *, osiz*2, "do_sprintf");\
      ofre+=osiz;\
      osiz*=2;\
    }\
    memcpy(obuf+olen,s,(l));\
    olen+=(l);\
    ofre-=(l);\
  }

	/* Is there space for something L big in the buffer? */
#define chksize(l)  if((l)>ofre) {\
    erealloc(obuf, char *, osiz*2, "do_sprintf");\
    ofre+=osiz;\
    osiz*=2;\
  }

	/*
	 * Get the next arg to be formatted.  If we've run out of args,
	 * return "" (Null string) 
	 */
#define parse_next_arg() {\
  if(!carg) { toofew = 1; break; }\
  else {\
	arg=tree_eval(carg->lnode);\
	carg=carg->rnode;\
  }\
 }

	NODE *r;
	int toofew = 0;
	char *obuf;
	int osiz, ofre, olen;
	static char chbuf[] = "0123456789abcdef";
	static char sp[] = " ";
	char *s0, *s1;
	int n0;
	NODE *sfmt, *arg;
	register NODE *carg;
	long fw, prec, lj, alt, big;
	long *cur;
	long val;
#ifdef sun386		/* Can't cast unsigned (int/long) from ptr->value */
	long tmp_uval;	/* on 386i 4.0.1 C compiler -- it just hangs */
#endif
	unsigned long uval;
	int sgn;
	int base;
	char cpbuf[30];		/* if we have numbers bigger than 30 */
	char *cend = &cpbuf[30];/* chars, we lose, but seems unlikely */
	char *cp;
	char *fill;
	double tmpval;
	char *pr_str;
	int ucasehex = 0;
	char signchar = 0;
	int len;


	emalloc(obuf, char *, 120, "do_sprintf");
	osiz = 120;
	ofre = osiz;
	olen = 0;
	sfmt = tree_eval(tree->lnode);
	sfmt = force_string(sfmt);
	carg = tree->rnode;
	for (s0 = s1 = sfmt->stptr, n0 = sfmt->stlen; n0-- > 0;) {
		if (*s1 != '%') {
			s1++;
			continue;
		}
		bchunk(s0, s1 - s0);
		s0 = s1;
		cur = &fw;
		fw = 0;
		prec = 0;
		lj = alt = big = 0;
		fill = sp;
		cp = cend;
		s1++;

retry:
		--n0;
		switch (*s1++) {
		case '%':
			bchunk("%", 1);
			s0 = s1;
			break;

		case '0':
			if (fill != sp || lj)
				goto lose;
			if (cur == &fw)
				fill = "0";	/* FALL through */
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			if (cur == 0)
				goto lose;
			*cur = s1[-1] - '0';
			while (n0 > 0 && *s1 >= '0' && *s1 <= '9') {
				--n0;
				*cur = *cur * 10 + *s1++ - '0';
			}
			goto retry;
		case '*':
			if (cur == 0)
				goto lose;
			parse_next_arg();
			*cur = force_number(arg);
			free_temp(arg);
			goto retry;
		case ' ':		/* print ' ' or '-' */
		case '+':		/* print '+' or '-' */
			signchar = *(s1-1);
			goto retry;
		case '-':
			if (lj || fill != sp)
				goto lose;
			lj++;
			goto retry;
		case '.':
			if (cur != &fw)
				goto lose;
			cur = &prec;
			goto retry;
		case '#':
			if (alt)
				goto lose;
			alt++;
			goto retry;
		case 'l':
			if (big)
				goto lose;
			big++;
			goto retry;
		case 'c':
			parse_next_arg();
			if (arg->flags & NUMERIC) {
#ifdef sun386
				tmp_uval = arg->numbr; 
				uval= (unsigned long) tmp_uval;
#else
				uval = (unsigned long) arg->numbr;
#endif
				cpbuf[0] = uval;
				prec = 1;
				pr_str = cpbuf;
				goto dopr_string;
			}
			if (! prec)
				prec = 1;
			else if (prec > arg->stlen)
				prec = arg->stlen;
			pr_str = arg->stptr;
			goto dopr_string;
		case 's':
			parse_next_arg();
			arg = force_string(arg);
			if (!prec || prec > arg->stlen)
				prec = arg->stlen;
			pr_str = arg->stptr;

	dopr_string:
			if (fw > prec && !lj) {
				while (fw > prec) {
					bchunk(sp, 1);
					fw--;
				}
			}
			bchunk(pr_str, (int) prec);
			if (fw > prec) {
				while (fw > prec) {
					bchunk(sp, 1);
					fw--;
				}
			}
			s0 = s1;
			free_temp(arg);
			break;
		case 'd':
		case 'i':
			parse_next_arg();
			val = (long) force_number(arg);
			free_temp(arg);
			if (val < 0) {
				sgn = 1;
				val = -val;
			} else
				sgn = 0;
			do {
				*--cp = '0' + val % 10;
				val /= 10;
			} while (val);
			if (sgn)
				*--cp = '-';
			else if (signchar)
				*--cp = signchar;
			if (prec > fw)
				fw = prec;
			prec = cend - cp;
			if (fw > prec && !lj) {
				if (fill != sp && (*cp == '-' || signchar)) {
					bchunk(cp, 1);
					cp++;
					prec--;
					fw--;
				}
				while (fw > prec) {
					bchunk(fill, 1);
					fw--;
				}
			}
			bchunk(cp, (int) prec);
			if (fw > prec) {
				while (fw > prec) {
					bchunk(fill, 1);
					fw--;
				}
			}
			s0 = s1;
			break;
		case 'u':
			base = 10;
			goto pr_unsigned;
		case 'o':
			base = 8;
			goto pr_unsigned;
		case 'X':
			ucasehex = 1;
		case 'x':
			base = 16;
			goto pr_unsigned;
	pr_unsigned:
			parse_next_arg();
			uval = (unsigned long) force_number(arg);
			free_temp(arg);
			do {
				*--cp = chbuf[uval % base];
				if (ucasehex && isalpha(*cp))
					*cp = toupper(*cp);
				uval /= base;
			} while (uval);
			if (alt && (base == 8 || base == 16)) {
				if (base == 16) {
					if (ucasehex)
						*--cp = 'X';
					else
						*--cp = 'x';
				}
				*--cp = '0';
			}
			prec = cend - cp;
			if (fw > prec && !lj) {
				while (fw > prec) {
					bchunk(fill, 1);
					fw--;
				}
			}
			bchunk(cp, (int) prec);
			if (fw > prec) {
				while (fw > prec) {
					bchunk(fill, 1);
					fw--;
				}
			}
			s0 = s1;
			break;
		case 'g':
			parse_next_arg();
			tmpval = force_number(arg);
			free_temp(arg);
			chksize(fw + prec + 9);	/* 9==slop */

			cp = cpbuf;
			*cp++ = '%';
			if (lj)
				*cp++ = '-';
			if (fill != sp)
				*cp++ = '0';
#ifndef GFMT_WORKAROUND
			if (cur != &fw) {
				(void) strcpy(cp, "*.*g");
				(void) sprintf(obuf + olen, cpbuf, (int) fw, (int) prec, (double) tmpval);
			} else {
				(void) strcpy(cp, "*g");
				(void) sprintf(obuf + olen, cpbuf, (int) fw, (double) tmpval);
			}
#else	/* GFMT_WORKAROUND */
		      {
			char *gptr, gbuf[120];
#define DEFAULT_G_PRECISION 6
			if (fw + prec + 9 > sizeof gbuf) {	/* 9==slop */
				emalloc(gptr, char *, fw+prec+9, "do_sprintf(gfmt)");
			} else
				gptr = gbuf;
			(void) gfmt((double) tmpval, cur != &fw ?
				    (int) prec : DEFAULT_G_PRECISION, gptr);
			*cp++ = '*',  *cp++ = 's',  *cp = '\0';
			(void) sprintf(obuf + olen, cpbuf, (int) fw, gptr);
			if (fill != sp && *gptr == ' ') {
				char *p = gptr;
				do { *p++ = '0'; } while (*p == ' ');
			}
			if (gptr != gbuf) free(gptr);
		      }
#endif	/* GFMT_WORKAROUND */
			len = strlen(obuf + olen);
			ofre -= len;
			olen += len;
			s0 = s1;
			break;

		case 'f':
			parse_next_arg();
			tmpval = force_number(arg);
			free_temp(arg);
			chksize(fw + prec + 9);	/* 9==slop */

			cp = cpbuf;
			*cp++ = '%';
			if (lj)
				*cp++ = '-';
			if (fill != sp)
				*cp++ = '0';
			if (cur != &fw) {
				(void) strcpy(cp, "*.*f");
				(void) sprintf(obuf + olen, cpbuf, (int) fw, (int) prec, (double) tmpval);
			} else {
				(void) strcpy(cp, "*f");
				(void) sprintf(obuf + olen, cpbuf, (int) fw, (double) tmpval);
			}
			len = strlen(obuf + olen);
			ofre -= len;
			olen += len;
			s0 = s1;
			break;
		case 'e':
			parse_next_arg();
			tmpval = force_number(arg);
			free_temp(arg);
			chksize(fw + prec + 9);	/* 9==slop */
			cp = cpbuf;
			*cp++ = '%';
			if (lj)
				*cp++ = '-';
			if (fill != sp)
				*cp++ = '0';
			if (cur != &fw) {
				(void) strcpy(cp, "*.*e");
				(void) sprintf(obuf + olen, cpbuf, (int) fw, (int) prec, (double) tmpval);
			} else {
				(void) strcpy(cp, "*e");
				(void) sprintf(obuf + olen, cpbuf, (int) fw, (double) tmpval);
			}
			len = strlen(obuf + olen);
			ofre -= len;
			olen += len;
			s0 = s1;
			break;

		default:
	lose:
			break;
		}
		if (toofew)
			fatal("%s\n\t%s\n\t%*s%s",
			"not enough arguments to satisfy format string",
			sfmt->stptr, s1 - sfmt->stptr - 2, "",
			"^ ran out for this one"
			);
	}
	if (carg != NULL)
		warning("too many arguments supplied for format string");
	bchunk(s0, s1 - s0);
	free_temp(sfmt);
	r = make_str_node(obuf, olen, ALREADY_MALLOCED);
	r->flags |= TEMP;
	return r;
}

void
do_printf(tree)
register NODE *tree;
{
	struct redirect *rp = NULL;
	register FILE *fp;

	if (tree->rnode) {
		int errflg;	/* not used, sigh */

		rp = redirect(tree->rnode, &errflg);
		if (rp) {
			fp = rp->fp;
			if (!fp)
				return;
		} else
			return;
	} else
		fp = stdout;
	tree = do_sprintf(tree->lnode);
	(void) fwrite(tree->stptr, sizeof(char), tree->stlen, fp);
	free_temp(tree);
	if ((fp == stdout && output_is_tty) || (rp && (rp->flag & RED_NOBUF))) {
		fflush(fp);
		if (ferror(fp)) {
			warning("error writing output: %s", strerror(errno));
			clearerr(fp);
		}
	}
}

NODE *
do_sqrt(tree)
NODE *tree;
{
	NODE *tmp;
	double arg;
	extern double sqrt();

	tmp = tree_eval(tree->lnode);
	arg = (double) force_number(tmp);
	free_temp(tmp);
	if (arg < 0.0)
		warning("sqrt called with negative argument %g", arg);
	return tmp_number((AWKNUM) sqrt(arg));
}

NODE *
do_substr(tree)
NODE *tree;
{
	NODE *t1, *t2, *t3;
	NODE *r;
	register int indx, length;

	t1 = tree_eval(tree->lnode);
	t2 = tree_eval(tree->rnode->lnode);
	if (tree->rnode->rnode == NULL)	/* third arg. missing */
		length = t1->stlen;
	else {
		t3 = tree_eval(tree->rnode->rnode->lnode);
		length = (int) force_number(t3);
		free_temp(t3);
	}
	indx = (int) force_number(t2) - 1;
	free_temp(t2);
	t1 = force_string(t1);
	if (indx < 0)
		indx = 0;
	if (indx >= t1->stlen || length <= 0) {
		free_temp(t1);
		return Nnull_string;
	}
	if (indx + length > t1->stlen)
		length = t1->stlen - indx;
	r =  tmp_string(t1->stptr + indx, length);
	free_temp(t1);
	return r;
}

NODE *
do_strftime(tree)
NODE *tree;
{
	NODE *t1, *t2;
	struct tm *tm;
	time_t clock;
	char buf[100];
	int ret;

	t1 = force_string(tree_eval(tree->lnode));

	if (tree->rnode == NULL)	/* second arg. missing, default */
		(void) time(&clock);
	else {
		t2 = tree_eval(tree->rnode->lnode);
		clock = (long) force_number(t2);
		free_temp(t2);
	}
	tm = localtime(&clock);

	ret = strftime(buf, 100, t1->stptr, tm);

	return tmp_string(buf, ret);
}

NODE *
do_systime(tree)
NODE *tree;
{
	time_t clock;

	(void) time(&clock);
	return tmp_number((AWKNUM) clock);
}

NODE *
do_system(tree)
NODE *tree;
{
	NODE *tmp;
	int ret;

	(void) flush_io ();	/* so output is synchronous with gawk's */
	tmp = tree_eval(tree->lnode);
	ret = system(force_string(tmp)->stptr);
	ret = (ret >> 8) & 0xff;
	free_temp(tmp);
	return tmp_number((AWKNUM) ret);
}

void 
do_print(tree)
register NODE *tree;
{
	register NODE *t1;
	struct redirect *rp = NULL;
	register FILE *fp;
	register char *s;

	if (tree->rnode) {
		int errflg;		/* not used, sigh */

		rp = redirect(tree->rnode, &errflg);
		if (rp) {
			fp = rp->fp;
			if (!fp)
				return;
		} else
			return;
	} else
		fp = stdout;
	tree = tree->lnode;
	while (tree) {
		t1 = tree_eval(tree->lnode);
		if (t1->flags & NUMBER) {
			if (OFMTidx == CONVFMTidx)
				(void) force_string(t1);
			else {
				char buf[100];

				sprintf(buf, OFMT, t1->numbr);
				t1 = tmp_string(buf, strlen(buf));
			}
		}
		(void) fwrite(t1->stptr, sizeof(char), t1->stlen, fp);
		free_temp(t1);
		tree = tree->rnode;
		if (tree) {
			s = OFS;
#if (!defined(VMS)) || defined(NO_TTY_FWRITE)
			while (*s)
				putc(*s++, fp);
#else
			if (OFSlen)
				fwrite(s, sizeof(char), OFSlen, fp);
#endif	/* VMS && !NO_TTY_FWRITE */
		}
	}
	s = ORS;
#if (!defined(VMS)) || defined(NO_TTY_FWRITE)
	while (*s)
		putc(*s++, fp);
	if ((fp == stdout && output_is_tty) || (rp && (rp->flag & RED_NOBUF))) {
#else
	if (ORSlen)
		fwrite(s, sizeof(char), ORSlen, fp);
	if ((rp && (rp->flag & RED_NOBUF))) {
#endif	/* VMS && !NO_TTY_FWRITE */
		fflush(fp);
		if (ferror(fp)) {
			warning("error writing output: %s", strerror(errno));
			clearerr(fp);
		}
	}
}

NODE *
do_tolower(tree)
NODE *tree;
{
	NODE *t1, *t2;
	register char *cp, *cp2;

	t1 = tree_eval(tree->lnode);
	t1 = force_string(t1);
	t2 = tmp_string(t1->stptr, t1->stlen);
	for (cp = t2->stptr, cp2 = t2->stptr + t2->stlen; cp < cp2; cp++)
		if (isupper(*cp))
			*cp = tolower(*cp);
	free_temp(t1);
	return t2;
}

NODE *
do_toupper(tree)
NODE *tree;
{
	NODE *t1, *t2;
	register char *cp;

	t1 = tree_eval(tree->lnode);
	t1 = force_string(t1);
	t2 = tmp_string(t1->stptr, t1->stlen);
	for (cp = t2->stptr; cp < t2->stptr + t2->stlen; cp++)
		if (islower(*cp))
			*cp = toupper(*cp);
	free_temp(t1);
	return t2;
}

NODE *
do_atan2(tree)
NODE *tree;
{
	NODE *t1, *t2;
	extern double atan2();
	double d1, d2;

	t1 = tree_eval(tree->lnode);
	t2 = tree_eval(tree->rnode->lnode);
	d1 = force_number(t1);
	d2 = force_number(t2);
	free_temp(t1);
	free_temp(t2);
	return tmp_number((AWKNUM) atan2(d1, d2));
}

NODE *
do_sin(tree)
NODE *tree;
{
	NODE *tmp;
	extern double sin();
	double d;

	tmp = tree_eval(tree->lnode);
	d = sin((double)force_number(tmp));
	free_temp(tmp);
	return tmp_number((AWKNUM) d);
}

NODE *
do_cos(tree)
NODE *tree;
{
	NODE *tmp;
	extern double cos();
	double d;

	tmp = tree_eval(tree->lnode);
	d = cos((double)force_number(tmp));
	free_temp(tmp);
	return tmp_number((AWKNUM) d);
}

static int firstrand = 1;
static char state[256];

#define	MAXLONG	2147483647	/* maximum value for long int */

/* ARGSUSED */
NODE *
do_rand(tree)
NODE *tree;
{
	if (firstrand) {
		(void) initstate((unsigned) 1, state, sizeof state);
		srandom(1);
		firstrand = 0;
	}
	return tmp_number((AWKNUM) random() / MAXLONG);
}

NODE *
do_srand(tree)
NODE *tree;
{
	NODE *tmp;
	static long save_seed = 0;
	long ret = save_seed;	/* SVR4 awk srand returns previous seed */

	if (firstrand)
		(void) initstate((unsigned) 1, state, sizeof state);
	else
		(void) setstate(state);

	if (!tree)
		srandom((int) (save_seed = (long) time((time_t *) 0)));
	else {
		tmp = tree_eval(tree->lnode);
		srandom((int) (save_seed = (long) force_number(tmp)));
		free_temp(tmp);
	}
	firstrand = 0;
	return tmp_number((AWKNUM) ret);
}

NODE *
do_match(tree)
NODE *tree;
{
	NODE *t1;
	int rstart;
	AWKNUM rlength;
	Regexp *rp;

	t1 = force_string(tree_eval(tree->lnode));
	tree = tree->rnode->lnode;
	rp = re_update(tree);
	rstart = research(rp, t1->stptr, t1->stlen, 1);
	if (rstart >= 0) {	/* match succeded */
		rstart++;	/* 1-based indexing */
		rlength = REEND(rp, t1->stptr) - RESTART(rp, t1->stptr);
	} else {		/* match failed */
		rstart = 0;
		rlength = -1.0;
	}
	free_temp(t1);
	unref(RSTART_node->var_value);
	RSTART_node->var_value = make_number((AWKNUM) rstart);
	unref(RLENGTH_node->var_value);
	RLENGTH_node->var_value = make_number(rlength);
	return tmp_number((AWKNUM) rstart);
}

static NODE *
sub_common(tree, global)
NODE *tree;
int global;
{
	register char *scan;
	register char *bp, *cp;
	char *buf;
	int buflen;
	register char *matchend;
	register int len;
	char *matchstart;
	char *text;
	int textlen;
	char *repl;
	char *replend;
	int repllen;
	int sofar;
	int ampersands;
	int inplace = 0;
	int matches = 0;
	Regexp *rp;
	NODE *s;		/* subst. pattern */
	NODE *t;		/* string to make sub. in; $0 if none given */
	NODE *tmp;
	NODE **lhs = &tree;	/* value not used -- just different from NULL */
	int priv = 0;
	Func_ptr after_assign = NULL;

	tmp = tree->lnode;
	rp = re_update(tmp);

	tree = tree->rnode;
	s = tree->lnode;

	tree = tree->rnode;
	tmp = tree->lnode;
	if (tmp->type == Node_val)
		lhs = NULL;
	t = force_string(tree_eval(tmp));

	/* do the search early to avoid work on non-match */
	if (research(rp, t->stptr, t->stlen, 1) == -1)
		return tmp_number((AWKNUM) 0);

	if (lhs != NULL)
		lhs = get_lhs(tmp, &after_assign);
	t->flags |= STRING;
	/*
	 * create a private copy of the string
	 */
	if (t->stref > 1 || (t->flags & PERM)) {
		unsigned int saveflags;

		saveflags = t->flags;
		t->flags &= ~MALLOC;
		tmp = dupnode(t);
		t->flags = saveflags;
		t = tmp;
		priv = 1;
	}
	text = t->stptr;
	textlen = t->stlen;

	s = force_string(tree_eval(s));
	repl = s->stptr;
	replend = repl + s->stlen;
	repllen = replend - repl;
	if (repllen == 0) {		/* replacement is null string */
		buflen = textlen;
		buf = text;		/* so do subs. in place */
		inplace = 1;
	} else {
		buflen = textlen * 2;	/* initial guess -- adjusted later */
		emalloc(buf, char *, buflen, "do_sub");
	}
	ampersands = 0;
	for (scan = repl; scan < replend; scan++) {
		if (*scan == '&') {
			repllen--;
			ampersands++;
		} else if (*scan == '\\' && *(scan+1) == '&')
			repllen--;
	}

	bp = buf;
	for (;;) {
		matches++;
		matchstart = text + RESTART(rp, t->stptr);
		matchend = text + REEND(rp, t->stptr);

		/*
		 * create the result, copying in parts of the original
		 * string 
		 */
		len = matchstart - text + repllen
		      + ampersands * (matchend - matchstart);
		sofar = bp - buf;
		while (buflen - sofar - len - 1 < 0) {
			buflen *= 2;
			erealloc(buf, char *, buflen, "do_sub");
			bp = buf + sofar;
		}
		for (scan = text; scan < matchstart; scan++)
			*bp++ = *scan;
		for (scan = repl; scan < replend; scan++)
			if (*scan == '&')
				for (cp = matchstart; cp < matchend; cp++)
					*bp++ = *cp;
			else if (*scan == '\\' && *(scan+1) == '&') {
				scan++;
				*bp++ = *scan;
			} else
				*bp++ = *scan;
		if (global && matchstart == matchend) {
			*bp++ = *text;
			matchend++;
		}
		textlen = text + textlen - matchend;
		text = matchend;
		if (!global || research(rp, text, textlen, 1) == -1)
			break;
	}
	sofar = bp - buf;
	if (!inplace && buflen - sofar - textlen - 1) {
		buflen = sofar + textlen + 2;
		erealloc(buf, char *, buflen, "do_sub");
		bp = buf + sofar;
	}
	for (scan = matchend; scan < text + textlen; scan++)
		*bp++ = *scan;
	textlen = bp - buf;
	if (inplace)
		erealloc(buf, char *, textlen + 2, "do_sub");
	else
		free(t->stptr);
	t->stptr = buf;
	t->stlen = textlen;

	free_temp(s);
	if (matches > 0 && lhs) {
		if (priv) {
			unref(*lhs);
			*lhs = t;
		}
		if (after_assign)
			(*after_assign)();
		t->flags &= ~(NUM|NUMERIC);
	}
	return tmp_number((AWKNUM) matches);
}

NODE *
do_gsub(tree)
NODE *tree;
{
	return sub_common(tree, 1);
}

NODE *
do_sub(tree)
NODE *tree;
{
	return sub_common(tree, 0);
}

#ifdef GFMT_WORKAROUND
	/*
	 *	printf's %g format [can't rely on gcvt()]
	 *		caveat: don't use as argument to *printf()!
	 */
char *
gfmt(g, prec, buf)
double g;	/* value to format */
int prec;	/* indicates desired significant digits, not decimal places */
char *buf;	/* return buffer; assumed big enough to hold result */
{
	if (g == 0.0) {
		(void) strcpy(buf, "0");	/* easy special case */
	} else {
		register char *d, *e, *p;

		/* start with 'e' format (it'll provide nice exponent) */
		if (prec < 1) prec = 1;	    /* at least 1 significant digit */
		(void) sprintf(buf, "%.*e", prec - 1, g);
		if ((e = strchr(buf, 'e')) != 0) {	/* find exponent  */
			int exp = atoi(e+1);		/* fetch exponent */
			if (exp >= -4 && exp < prec) {	/* per K&R2, B1.2 */
				/* switch to 'f' format and re-do */
				prec -= (exp + 1);	/* decimal precision */
				(void) sprintf(buf, "%.*f", prec, g);
				e = buf + strlen(buf);
			}
			if ((d = strchr(buf, '.')) != 0) {
				/* remove trailing zeroes and decimal point */
				for (p = e; p > d && *--p == '0'; ) continue;
				if (*p == '.') --p;
				if (++p < e)	/* copy exponent and NUL */
					while ((*p++ = *e++) != '\0') continue;
			}
		}
	}
	return buf;
}
#endif	/* GFMT_WORKAROUND */
