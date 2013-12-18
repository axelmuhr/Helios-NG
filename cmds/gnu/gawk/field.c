/*
 * field.c - routines for dealing with fields and record parsing
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


static int (*parse_field) P((int, char **, int, char *,
			     Regexp *, void (*)(), NODE *));
static void rebuild_record P((void));
static int re_parse_field P((int, char **, int, char *,
			     Regexp *, void (*)(), NODE *));
static int def_parse_field P((int, char **, int, char *,
			      Regexp *, void (*)(), NODE *));
static int sc_parse_field P((int, char **, int, char *,
			     Regexp *, void (*)(), NODE *));
static int fw_parse_field P((int, char **, int, char *,
			     Regexp *, void (*)(), NODE *));
static void set_element P((int, char *, int, NODE *));

static Regexp *FS_regexp = NULL;
static char *parse_extent;	/* marks where to restart parse of record */
static int parse_high_water=0;	/* field number that we have parsed so far */
static int nf_high_water = 0;	/* size of fields_arr */
static char f_empty[] = "\0";
static int resave_fs;
static NODE *save_FS;
static char *save_fs;		/* save current value of FS when line is read,
				 * to be used in deferred parsing
				 */

NODE **fields_arr;		/* array of pointers to the field nodes */
int field0_valid = 1;		/* $(>0) has not been changed yet */
NODE *field0;
static NODE **nodes;		/* permanent repository of field nodes */
static int *FIELDWIDTHS = NULL;

void
init_fields()
{
	emalloc(fields_arr, NODE **, sizeof(NODE *), "init_fields");
	emalloc(nodes, NODE **, sizeof(NODE *), "init_fields");
	emalloc(field0, NODE *, sizeof(NODE), "init_fields");
	field0->type = Node_val;
	field0->stref = 0;
	field0->stptr = "";
	field0->flags = (STRING|STR|PERM);	/* never free buf */
	fields_arr[0] = field0;
	save_FS = dupnode(FS_node->var_value);
	save_fs = save_FS->stptr;
}

static void
grow_fields_arr(num)
int num;
{
	register int t;
	register NODE *n;

	erealloc(fields_arr, NODE **, (num + 1) * sizeof(NODE *), "set_field");
	erealloc(nodes, NODE **, (num+1) * sizeof(NODE *), "set_field");
	for (t = nf_high_water+1; t <= num; t++) {
		getnode(n);
		n->type = Node_val;
		nodes[t] = n;
		fields_arr[t] = nodes[t];
	}
	nf_high_water = num;
}

/*ARGSUSED*/
static void
set_field(num, str, len, dummy)
int num;
char *str;
int len;
NODE *dummy;	/* not used -- just to make interface same as set_element */
{
	register NODE *n;
	register int t;

	if (num > nf_high_water)
		grow_fields_arr(num);
	n = nodes[num];
	n->stptr = str;
	n->stlen = len;
	n->flags = (PERM|STR|STRING|MAYBE_NUM);
	fields_arr[num] = n;
}

/* Someone assigned a value to $(something).  Fix up $0 to be right */
static void
rebuild_record()
{
	register int tlen;
	register NODE *tmp;
	NODE *ofs;
	char *ops;
	register char *cops;
	register NODE **ptr;
	register int ofslen;

	tlen = 0;
	ofs = force_string(OFS_node->var_value);
	ofslen = ofs->stlen;
	ptr = &fields_arr[NF];
	while (ptr > &fields_arr[0]) {
		tmp = force_string(*ptr);
		tlen += tmp->stlen;
		ptr--;
	}
	tlen += (NF - 1) * ofslen;
	emalloc(ops, char *, tlen + 2, "fix_fields");
	cops = ops;
	ops[0] = '\0';
	for (ptr = &fields_arr[1]; ptr <= &fields_arr[NF]; ptr++) {
		tmp = *ptr;
		if (tmp->stlen == 1)
			*cops++ = tmp->stptr[0];
		else if (tmp->stlen != 0) {
			memcpy(cops, tmp->stptr, tmp->stlen);
			cops += tmp->stlen;
		}
		if (ptr != &fields_arr[NF]) {
			if (ofslen == 1)
				*cops++ = ofs->stptr[0];
			else if (ofslen != 0) {
				memcpy(cops, ofs->stptr, ofslen);
				cops += ofslen;
			}
		}
	}
	tmp = make_str_node(ops, tlen, ALREADY_MALLOCED);
	unref(fields_arr[0]);
	fields_arr[0] = tmp;
	field0_valid = 1;
}

/*
 * setup $0, but defer parsing rest of line until reference is made to $(>0)
 * or to NF.  At that point, parse only as much as necessary.
 */
void
set_record(buf, cnt, freeold)
char *buf;
int cnt;
int freeold;
{
	register int i;

	NF = -1;
	for (i = 1; i <= parse_high_water; i++) {
		unref(fields_arr[i]);
	}
	parse_high_water = 0;
	if (freeold) {
		unref(fields_arr[0]);
		if (resave_fs) {
			resave_fs = 0;
			unref(save_FS);
			save_FS = dupnode(FS_node->var_value);
			save_fs = save_FS->stptr;
		}
		field0->stptr = buf;
		field0->stlen = cnt;
		field0->stref = 1;
		field0->flags = (STRING|STR|PERM|MAYBE_NUM);
		fields_arr[0] = field0;
	}
	fields_arr[0]->flags |= MAYBE_NUM;
	field0_valid = 1;
}

void
reset_record()
{
	(void) force_string(fields_arr[0]);
	set_record(fields_arr[0]->stptr, fields_arr[0]->stlen, 0);
}

void
set_NF()
{
	NF = (int) force_number(NF_node->var_value);
	field0_valid = 0;
}

/*
 * this is called both from get_field() and from do_split()
 * via (*parse_field)().  This variation is for when FS is a regular
 * expression -- either user-defined or because RS=="" and FS==" "
 */
static int
re_parse_field(up_to, buf, len, fs, rp, set, n)
int up_to;	/* parse only up to this field number */
char **buf;	/* on input: string to parse; on output: point to start next */
int len;
register char *fs;
Regexp *rp;
void (*set) ();	/* routine to set the value of the parsed field */
NODE *n;
{
	register char *scan = *buf;
	register int nf = parse_high_water;
	register char *field;
	register char *end = scan + len;
	char *cp;

	if (up_to == HUGE)
		nf = 0;
	if (len == 0)
		return nf;

	cp = FS_node->var_value->stptr;
	if (*RS == 0 && *cp == ' ' && *(cp+1) == '\0') {
		while (scan < end
		       && (*scan == '\n' || *scan == ' ' || *scan == '\t'))
			scan++;
	}
	field = scan;
	while (scan < end
	       && research(rp, scan, (int)(end - scan), 1) != -1
	       && nf < up_to) {
		if (REEND(rp, scan) == RESTART(rp, scan)) {	/* null match */
			scan++;
			if (scan == end) {
				(*set)(++nf, field, scan - field, n);
				up_to = nf;
				break;
			}
			continue;
		}
		(*set)(++nf, field, RESTART(rp, scan), n);
		scan += REEND(rp, scan);
		field = scan;
	}
	if (nf != up_to && *RS != 0 && scan < end) {
		(*set)(++nf, scan, (int)(end - scan), n);
		scan = end;
	}
	*buf = scan;
	return (nf);
}

/*
 * this is called both from get_field() and from do_split()
 * via (*parse_field)().  This variation is for when FS is a single space
 * character.
 */
static int
def_parse_field(up_to, buf, len, fs, rp, set, n)
int up_to;	/* parse only up to this field number */
char **buf;	/* on input: string to parse; on output: point to start next */
int len;
register char *fs;
Regexp *rp;
void (*set) ();	/* routine to set the value of the parsed field */
NODE *n;
{
	register char *scan = *buf;
	register int nf = parse_high_water;
	register char *field;
	register char *end = scan + len;

	if (up_to == HUGE)
		nf = 0;
	if (len == 0)
		return nf;

	*end = ' ';	/* sentinel character */
	for (; nf < up_to; scan++) {
		/*
		 * special case:  fs is single space, strip leading whitespace 
		 */
		while (scan < end && (*scan == ' ' || *scan == '\t'))
			scan++;
		if (scan >= end)
			break;
		field = scan;
		while (*scan != ' ' && *scan != '\t')
			scan++;
		(*set)(++nf, field, (int)(scan - field), n);
		if (scan == end)
			break;
	}
	*buf = scan;
	return nf;
}

/*
 * this is called both from get_field() and from do_split()
 * via (*pase_field)().  This variation is for when FS is a single character
 * other than space.
 */
static int
sc_parse_field(up_to, buf, len, fs, rp, set, n)
int up_to;	/* parse only up to this field number */
char **buf;	/* on input: string to parse; on output: point to start next */
int len;
register char *fs;
Regexp *rp;
void (*set) ();	/* routine to set the value of the parsed field */
NODE *n;
{
	register char *scan = *buf;
	register char fschar = *fs;
	register int nf = parse_high_water;
	register char *field;
	register char *end = scan + len;

	if (up_to == HUGE)
		nf = 0;
	if (len == 0)
		return nf;
	*end = fschar;	/* sentinel character */
	for (; nf < up_to; scan++) {
		field = scan;
		while (*scan++ != fschar)
			;
		scan--;
		(*set)(++nf, field, (int)(scan - field), n);
		if (scan == end)
			break;
	}
	*buf = scan;
	return nf;
}

/*
 * this is called both from get_field() and from do_split()
 * via (*pase_field)().  This variation is for when FS is a single character
 * other than space.
 */
static int
fw_parse_field(up_to, buf, len, fs, rp, set, n)
int up_to;	/* parse only up to this field number */
char **buf;	/* on input: string to parse; on output: point to start next */
int len;
register char *fs;
Regexp *rp;
void (*set) ();	/* routine to set the value of the parsed field */
NODE *n;
{
	register char *scan = *buf;
	register int nf = parse_high_water;
	register char *end = scan + len;

	if (up_to == HUGE)
		nf = 0;
	if (len == 0)
		return nf;
	for (; nf < up_to && (len = FIELDWIDTHS[nf+1]) != -1; ) {
		if (len > end - scan)
			len = end - scan;
		(*set)(++nf, scan, len, n);
		scan += len;
	}
	if (len == -1)
		*buf = end;
	else
		*buf = scan;
	return nf;
}

NODE **
get_field(num, assign)
register int num;
Func_ptr *assign;	/* this field is on the LHS of an assign */
{
	int n;

	/*
	 * if requesting whole line but some other field has been altered,
	 * then the whole line must be rebuilt
	 */
	if (num == 0) {
		if (!field0_valid) {
			/* first, parse remainder of input record */
			if (NF == -1) {
				NF = (*parse_field)(HUGE-1, &parse_extent,
		    			fields_arr[0]->stlen -
					(parse_extent - fields_arr[0]->stptr),
		    			save_fs, FS_regexp, set_field,
					(NODE *)NULL);
				parse_high_water = NF;
			}
			rebuild_record();
		}
		if (assign)
			*assign = reset_record;
		return &fields_arr[0];
	}

	/* assert(num > 0); */

	if (assign)
		field0_valid = 0;
	if (num <= parse_high_water)	/* we have already parsed this field */
		return &fields_arr[num];
	if (parse_high_water == 0)	/* starting at the beginning */
		parse_extent = fields_arr[0]->stptr;
	/*
	 * parse up to num fields, calling set_field() for each, and saving
	 * in parse_extent the point where the parse left off
	 */
	n = (*parse_field)(num, &parse_extent,
		fields_arr[0]->stlen - (parse_extent-fields_arr[0]->stptr),
		save_fs, FS_regexp, set_field, (NODE *)NULL);
	parse_high_water = n;
	if (num == HUGE-1)
		num = n;
	if (n < num) {	/* requested field number beyond end of record; */
		register int i;

		if (num > nf_high_water)
			grow_fields_arr(num);

		/* fill in fields that don't exist */
		for (i = n + 1; i <= num; i++)
			fields_arr[i] = Nnull_string;

		/*
		 * if this field is onthe LHS of an assignment, then we want to
		 * set NF to this value, below
		 */
		if (assign)
			n = num;
	}
	/*
	 * if we reached the end of the record, set NF to the number of fields
	 * so far.  Note that num might actually refer to a field that
	 * is beyond the end of the record, but we won't set NF to that value at
	 * this point, since this is only a reference to the field and NF
	 * only gets set if the field is assigned to -- in this case n has
	 * been set to num above
	 */
	if (parse_extent == fields_arr[0]->stptr + fields_arr[0]->stlen)
		NF = n;

	return &fields_arr[num];
}

static void
set_element(num, s, len, n)
int num;
char *s;
int len;
NODE *n;
{
	register NODE *it;

	it = make_string(s, len);
	it->flags |= MAYBE_NUM;
	*assoc_lookup(n, tmp_number((AWKNUM) (num))) = it;
}

NODE *
do_split(tree)
NODE *tree;
{
	NODE *t1, *t2, *t3, *tmp;
	register char *splitc = "";
	char *s;
	int (*parseit)();
	Regexp *rp = NULL;

	t1 = tree_eval(tree->lnode);
	t2 = tree->rnode->lnode;
	t3 = tree->rnode->rnode->lnode;

	(void) force_string(t1);

	if (t2->type == Node_param_list)
		t2 = stack_ptr[t2->param_cnt];
	if (t2->type != Node_var && t2->type != Node_var_array)
		fatal("second argument of split is not a variable");
	assoc_clear(t2);

	if (t3->re_flags & FS_DFLT) {
		parseit = parse_field;
		splitc = FS;
		rp = FS_regexp;
	} else {
		tmp = force_string(tree_eval(t3->re_exp));
		if (tmp->stlen == 1) {
			if (tmp->stptr[0] == ' ') {
				parseit = def_parse_field;
			} else {
				parseit = sc_parse_field;
				splitc = tmp->stptr;
			}
		} else {
			parseit = re_parse_field;
			rp = re_update(t3);
		}
		free_temp(tmp);
	}

	s = t1->stptr;
	tmp = tmp_number((AWKNUM) (*parseit)(HUGE, &s, t1->stlen,
					     splitc, rp, set_element, t2));
	free_temp(t1);
	return tmp;
}

void
set_FS()
{
	register NODE *tmp;
	static char buf[10];

	if (FS_regexp) {
		refree(FS_regexp);
		FS_regexp = NULL;
	}
	parse_field = def_parse_field;
	tmp = force_string(FS_node->var_value);
	FS = tmp->stptr;
	if (*RS == 0) {
		parse_field = re_parse_field;
		FS = buf;
		if (tmp->stlen == 1) {
			if (tmp->stptr[0] == ' ')
				(void) strcpy(buf, "[ 	\n]+");
			else if (tmp->stptr[0] != '\n')
				sprintf(buf, "[%c\n]", tmp->stptr[0]);
			else {
				parse_field = sc_parse_field;
				FS = tmp->stptr;
			}
		} else if (tmp->stlen == 0) {
			buf[0] = '\n';
			buf[1] = '\0';
			parse_field = sc_parse_field;
		} else
			FS = tmp->stptr;
	} else {
		if (tmp->stlen > 1)
			parse_field = re_parse_field;
		else if (*FS != ' ' && tmp->stlen == 1)
			parse_field = sc_parse_field;
	}
	if (parse_field == re_parse_field) {
		tmp = tmp_string(FS, strlen(FS));
		FS_regexp = make_regexp(tmp, 0, 1);
		free_temp(tmp);
	} else
		FS_regexp = NULL;
	resave_fs = 1;
}

void
set_RS()
{
	(void) force_string(RS_node->var_value);
	RS = RS_node->var_value->stptr;
	set_FS();
}

void
set_FIELDWIDTHS()
{
	register char *scan;
	char *end;
	register int i;
	static int fw_alloc = 1;
	static int warned = 0;

	if (do_lint && ! warned) {
		warned = 1;
		warning("use of FIELDWIDTHS is a gawk extension");
	}
	if (strict)	/* quick and dirty, does the trick */
		return;

	parse_field = fw_parse_field;
	scan = force_string(FIELDWIDTHS_node->var_value)->stptr;
	end = scan + 1;
	if (FIELDWIDTHS == NULL)
		emalloc(FIELDWIDTHS, int *, fw_alloc * sizeof(int), "set_FIELDWIDTHS");
	FIELDWIDTHS[0] = 0;
	for (i = 1; ; i++) {
		if (i >= fw_alloc) {
			fw_alloc *= 2;
			erealloc(FIELDWIDTHS, int *, fw_alloc * sizeof(int), "set_FIELDWIDTHS");
		}
		FIELDWIDTHS[i] = (int) strtol(scan, &end, 10);
		if (end == scan)
			break;
		scan = end;
	}
	FIELDWIDTHS[i] = -1;
}
