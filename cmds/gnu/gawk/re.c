/*
 * re.c - compile regular expressions.
 */

/* 
 * Copyright (C) 1991 the Free Software Foundation, Inc.
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

/* Generate compiled regular expressions */
Regexp *
make_regexp(s, ignorecase, dfa)
NODE *s;
int ignorecase;
int dfa;
{
	Regexp *rp;
	char *err;

	emalloc(rp, Regexp *, sizeof(*rp), "make_regexp");
	memset((char *) rp, 0, sizeof(*rp));
	emalloc(rp->pat.buffer, char *, 16, "make_regexp");
	rp->pat.allocated = 16;
	emalloc(rp->pat.fastmap, char *, 256, "make_regexp");

	if (ignorecase)
		rp->pat.translate = casetable;
	else
		rp->pat.translate = NULL;
	if ((err = re_compile_pattern(s->stptr, (size_t) s->stlen, &(rp->pat))) != NULL)
		fatal("%s: /%s/", err, s->stptr);
	if (dfa && !ignorecase) {
		regcompile(s->stptr, s->stlen, &(rp->dfareg), 1);
		rp->dfa = 1;
	} else
		rp->dfa = 0;
	free_temp(s);
	return rp;
}

int
research(rp, str, len, need_start)
Regexp *rp;
register char *str;
register int len;
int need_start;
{
	int count;
	int try_backref;
	char save1;
	char save2;
	char *ret = &save2;

	if (rp->dfa) {
		save1 = str[len];
		str[len] = '\n';
		save2 = str[len+1];
		ret = regexecute(&(rp->dfareg), str, str+len+1, 0, &count,
					&try_backref);
		str[len] = save1;
		str[len+1] = save2;
	}
	if (ret) {
		if (need_start || rp->dfa == 0)
			return re_search(&(rp->pat), str, len, 0, len, &(rp->regs));
		else
			return 1;
	 } else
		return -1;
}

void
refree(rp)
Regexp *rp;
{
	free(rp->pat.buffer);
	free(rp->pat.fastmap);
	if (rp->dfa)
		regfree(&(rp->dfareg));
	free(rp);
}

void
regerror(s)
const char *s;
{
	fatal((char *)s);
}

Regexp *
re_update(t)
NODE *t;
{
	NODE *t1;

#	define	CASE	1
	if ((t->re_flags & CASE) == IGNORECASE) {
		if (t->re_flags & CONST)
			return t->re_reg;
		t1 = force_string(tree_eval(t->re_exp));
		if (t->re_text) {
			if (cmp_nodes(t->re_text, t1) == 0) {
				free_temp(t1);
				return t->re_reg;
			}
			unref(t->re_text);
		}
		t->re_text = dupnode(t1);
		free_temp(t1);
	}
	if (t->re_reg)
		refree(t->re_reg);
	if (t->re_cnt)
		t->re_cnt++;
	if (t->re_cnt > 10)
		t->re_cnt = 0;
	if (!t->re_text) {
		t1 = force_string(tree_eval(t->re_exp));
		t->re_text = dupnode(t1);
		free_temp(t1);
	}
	t->re_reg = make_regexp(t->re_text, IGNORECASE, t->re_cnt);
	t->re_flags &= ~CASE;
	t->re_flags |= IGNORECASE;
	return t->re_reg;
}
