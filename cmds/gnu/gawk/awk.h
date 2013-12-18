/*
 * awk.h -- Definitions for gawk. 
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

/* ------------------------------ Includes ------------------------------ */
#include <stdio.h>
#include <ctype.h>
#include <setjmp.h>
#include <stdarg.h>
#include <time.h>
#include <errno.h>
#include <signal.h>

/* ----------------- System dependencies (with more includes) -----------*/

#ifndef VAXC
#include <sys/types.h>
#include <sys/stat.h>
#else	/* VMS w/ Digital's "VAX C" compiler */
#include <types.h>
#include <stat.h>
#include <file.h>	/* avoid <fcntl.h> in io.c */
#endif	/*VAXC*/

#include "config.h"

#ifdef __STDC__
#define	P(s)	s
#define MALLOC_ARG_T size_t
#else
#define	P(s)	()
#define MALLOC_ARG_T unsigned
#define volatile
#define const
#endif

#ifndef SIGTYPE
#define SIGTYPE	void
#endif

#ifdef SIZE_T_MISSING
typedef unsigned int size_t;
#endif

#ifndef SZTC
#define SZTC
#define INTC
#endif

#ifdef STDC_HEADERS
#include <stdlib.h>
#include <string.h>
#ifdef NeXT
#include <libc.h>
#undef atof
#else
#if defined(atarist) || defined(VMS)
#include <unixlib.h>
#else
#include <unistd.h>
#endif	/* atarist || VMS */
#endif	/* Next */
#else	/* STDC_HEADERS */
#include "protos.h"
#endif	/* STDC_HEADERS */

#if defined(ultrix) && !defined(Ultrix41)
extern char * getenv P((char *name));
extern double atof P((char *s));
#endif

#ifdef sparc
/* nasty nasty SunOS-ism */
#include <alloca.h>
#ifdef lint
extern char *alloca();
#endif
#else /* not sparc */
#if (!defined(atarist)) && (!defined(NeXT)) && (!defined(alloca))
extern char *alloca();
#endif /* atarist */
#endif /* sparc */

#ifdef HAVE_UNDERSCORE_SETJMP
/* nasty nasty berkelixm */
#define setjmp	_setjmp
#define longjmp	_longjmp
#endif

/*
 * if you don't have vprintf, but you are BSD, the version defined in
 * vprintf.c should do the trick.  Otherwise, try this and cross your fingers.
 */
#if defined(VPRINTF_MISSING) && !defined(DOPRNT_MISSING) && !defined(BSDSTDIO)
#define vfprintf(fp,fmt,arg)	_doprnt((fmt), (arg), (fp))
#endif

#ifdef VMS
/* some macros to redirect to code in vms/vms_misc.c */
#define exit		vms_exit
#define strerror	vms_strerror
#define strdup		vms_strdup
extern void  exit P((int));
extern char *strerror P((int));
extern char *strdup P((const char *str));
# ifndef NO_TTY_FWRITE
#define fwrite		tty_fwrite
#define fclose		tty_fclose
extern size_t fwrite P((const void *,size_t,size_t,FILE *));
extern int    fclose P((FILE *));
# endif
extern void vms_arg_fixup P((int *,char ***));
#endif  /*VMS*/

#ifndef _MSC_VER
extern int errno;	/* not necessary on many systems, but it can't hurt */
#endif

#define	GNU_REGEX
#ifdef GNU_REGEX
#include "regex.h"
#include "dfa.h"
typedef struct Regexp {
	struct re_pattern_buffer pat;
	struct re_registers regs;
	struct regexp dfareg;
	int dfa;
} Regexp;
#define	RESTART(rp,s)	(rp)->regs.start[0]
#define	REEND(rp,s)	(rp)->regs.end[0]
#else	/* GNU_REGEX */
#endif	/* GNU_REGEX */

#ifdef atarist
#define read _text_read /* we do not want all these CR's to mess our input */
extern int _text_read (int, char *, int);
#endif

#ifndef DEFPATH
#define DEFPATH	".:/usr/local/lib/awk:/usr/lib/awk"
#endif

#ifndef ENVSEP
#define ENVSEP	':'
#endif

/* ------------------ Constants, Structures, Typedefs  ------------------ */
#define AWKNUM	double

typedef enum {
	/* illegal entry == 0 */
	Node_illegal,

	/* binary operators  lnode and rnode are the expressions to work on */
	Node_times,
	Node_quotient,
	Node_mod,
	Node_plus,
	Node_minus,
	Node_cond_pair,		/* conditional pair (see Node_line_range) */
	Node_subscript,
	Node_concat,
	Node_exp,

	/* unary operators   subnode is the expression to work on */
/*10*/	Node_preincrement,
	Node_predecrement,
	Node_postincrement,
	Node_postdecrement,
	Node_unary_minus,
	Node_field_spec,

	/* assignments   lnode is the var to assign to, rnode is the exp */
	Node_assign,
	Node_assign_times,
	Node_assign_quotient,
	Node_assign_mod,
/*20*/	Node_assign_plus,
	Node_assign_minus,
	Node_assign_exp,

	/* boolean binaries   lnode and rnode are expressions */
	Node_and,
	Node_or,

	/* binary relationals   compares lnode and rnode */
	Node_equal,
	Node_notequal,
	Node_less,
	Node_greater,
	Node_leq,
/*30*/	Node_geq,
	Node_match,
	Node_nomatch,

	/* unary relationals   works on subnode */
	Node_not,

	/* program structures */
	Node_rule_list,		/* lnode is a rule, rnode is rest of list */
	Node_rule_node,		/* lnode is pattern, rnode is statement */
	Node_statement_list,	/* lnode is statement, rnode is more list */
	Node_if_branches,	/* lnode is to run on true, rnode on false */
	Node_expression_list,	/* lnode is an exp, rnode is more list */
	Node_param_list,	/* lnode is a variable, rnode is more list */

	/* keywords */
/*40*/	Node_K_if,		/* lnode is conditonal, rnode is if_branches */
	Node_K_while,		/* lnode is condtional, rnode is stuff to run */
	Node_K_for,		/* lnode is for_struct, rnode is stuff to run */
	Node_K_arrayfor,	/* lnode is for_struct, rnode is stuff to run */
	Node_K_break,		/* no subs */
	Node_K_continue,	/* no stuff */
	Node_K_print,		/* lnode is exp_list, rnode is redirect */
	Node_K_printf,		/* lnode is exp_list, rnode is redirect */
	Node_K_next,		/* no subs */
	Node_K_exit,		/* subnode is return value, or NULL */
/*50*/	Node_K_do,		/* lnode is conditional, rnode stuff to run */
	Node_K_return,
	Node_K_delete,
	Node_K_getline,
	Node_K_function,	/* lnode is statement list, rnode is params */

	/* I/O redirection for print statements */
	Node_redirect_output,	/* subnode is where to redirect */
	Node_redirect_append,	/* subnode is where to redirect */
	Node_redirect_pipe,	/* subnode is where to redirect */
	Node_redirect_pipein,	/* subnode is where to redirect */
	Node_redirect_input,	/* subnode is where to redirect */

	/* Variables */
/*60*/	Node_var,		/* rnode is value, lnode is array stuff */
	Node_var_array,		/* array is ptr to elements, asize num of
				 * eles */
	Node_val,		/* node is a value - type in flags */

	/* Builtins   subnode is explist to work on, proc is func to call */
	Node_builtin,

	/*
	 * pattern: conditional ',' conditional ;  lnode of Node_line_range
	 * is the two conditionals (Node_cond_pair), other word (rnode place)
	 * is a flag indicating whether or not this range has been entered.
	 */
	Node_line_range,

	/*
	 * boolean test of membership in array lnode is string-valued
	 * expression rnode is array name 
	 */
	Node_in_array,

	Node_func,		/* lnode is param. list, rnode is body */
	Node_func_call,		/* lnode is name, rnode is argument list */

	Node_cond_exp,		/* lnode is conditonal, rnode is if_branches */
	Node_regex,
/*70*/	Node_hashnode,
	Node_ahash,
	Node_NF,
	Node_NR,
	Node_FNR,
	Node_FS,
	Node_RS,
	Node_FIELDWIDTHS,
	Node_IGNORECASE,
	Node_OFS,
	Node_ORS,
	Node_OFMT,
	Node_CONVFMT
} NODETYPE;

/*
 * NOTE - this struct is a rather kludgey -- it is packed to minimize
 * space usage, at the expense of cleanliness.  Alter at own risk.
 */
typedef struct exp_node {
	union {
		struct {
			union {
				struct exp_node *lptr;
				char *param_name;
			} l;
			union {
				struct exp_node *rptr;
				struct exp_node *(*pptr) ();
				Regexp *preg;
				struct for_loop_header *hd;
				struct exp_node **av;
				int r_ent;	/* range entered */
			} r;
			union {
				char *name;
				struct exp_node *extra;
			} x;
			short number;
			unsigned char reflags;
#			define	CASE	1
#			define	CONST	2
#			define	FS_DFLT	4
		} nodep;
		struct {
			AWKNUM fltnum;	/* this is here for optimal packing of
					 * the structure on many machines
					 */
			char *sp;
			short slen;
			unsigned char sref;
			char idx;
		} val;
		struct {
			struct exp_node *next;
			char *name;
			int length;
			struct exp_node *value;
		} hash;
#define	hnext	sub.hash.next
#define	hname	sub.hash.name
#define	hlength	sub.hash.length
#define	hvalue	sub.hash.value
		struct {
			struct exp_node *next;
			struct exp_node *name;
			struct exp_node *value;
		} ahash;
#define	ahnext	sub.ahash.next
#define	ahname	sub.ahash.name
#define	ahvalue	sub.ahash.value
	} sub;
	NODETYPE type;
	unsigned short flags;
#			define	MEM	0x7
#			define	MALLOC	1	/* can be free'd */
#			define	TEMP	2	/* should be free'd */
#			define	PERM	4	/* can't be free'd */
#			define	VAL	0x18
#			define	NUM	8	/* numeric value is current */
#			define	STR	16	/* string value is current */
#			define	NUMERIC	32	/* entire string is numeric */
#			define	NUMBER	64	/* assigned as number */
#ifdef STRING
#undef STRING
#endif
#			define	STRING	128	/* assigned as string */
#			define	MAYBE_NUM	256
} NODE;

#define lnode	sub.nodep.l.lptr
#define nextp	sub.nodep.l.lptr
#define rnode	sub.nodep.r.rptr
#define source_file	sub.nodep.x.name
#define	source_line	sub.nodep.number
#define	param_cnt	sub.nodep.number
#define param	sub.nodep.l.param_name

#define subnode	lnode
#define proc	sub.nodep.r.pptr

#define re_reg	sub.nodep.r.preg
#define re_flags sub.nodep.reflags
#define re_text lnode
#define re_exp	sub.nodep.x.extra
#define	re_cnt	sub.nodep.number

#define forsub	lnode
#define forloop	rnode->sub.nodep.r.hd

#define stptr	sub.val.sp
#define stlen	sub.val.slen
#define stref	sub.val.sref
#define	stfmt	sub.val.idx

#define numbr	sub.val.fltnum

#define var_value lnode
#define var_array sub.nodep.r.av

#define condpair lnode
#define triggered sub.nodep.r.r_ent

#ifdef DONTDEF
int primes[] = {31, 61, 127, 257, 509, 1021, 2053, 4099, 8191, 16381};
#endif
/* a quick profile suggests that the following is a good value */
#define	HASHSIZE	127

typedef struct for_loop_header {
	NODE *init;
	NODE *cond;
	NODE *incr;
} FOR_LOOP_HEADER;

/* for "for(iggy in foo) {" */
struct search {
	NODE **arr_ptr;
	NODE **arr_end;
	NODE *bucket;
	NODE *retval;
};

/* for faster input, bypass stdio */
typedef struct iobuf {
	int fd;
	char *buf;
	char *off;
	char *end;
	size_t size;	/* this will be determined by an fstat() call */
	int cnt;
	char *secbuf;
	size_t secsiz;
	int flag;
#	define		IOP_IS_TTY	1
} IOBUF;

typedef void (*Func_ptr)();

/*
 * structure used to dynamically maintain a linked-list of open files/pipes
 */
struct redirect {
	unsigned int flag;
#		define		RED_FILE	1
#		define		RED_PIPE	2
#		define		RED_READ	4
#		define		RED_WRITE	8
#		define		RED_APPEND	16
#		define		RED_NOBUF	32
#		define		RED_USED	64
	char *value;
	FILE *fp;
	IOBUF *iop;
	int pid;
	int status;
	struct redirect *prev;
	struct redirect *next;
};

/* longjmp return codes, must be nonzero */
/* Continue means either for loop/while continue, or next input record */
#define TAG_CONTINUE 1
/* Break means either for/while break, or stop reading input */
#define TAG_BREAK 2
/* Return means return from a function call; leave value in ret_node */
#define	TAG_RETURN 3

#if defined(MSDOS) || (defined(atarist)) && (defined(__MSHORT__))
#define HUGE	0x7fff
#else
#define HUGE	0x7fffffff
#endif

/* -------------------------- External variables -------------------------- */
/* gawk builtin variables */
extern int NF;
extern int NR;
extern int FNR;
extern int IGNORECASE;
extern char *FS;
extern char *RS;
extern char *OFS;
extern int OFSlen;
extern char *ORS;
extern int ORSlen;
extern char *OFMT;
extern char *CONVFMT;
extern int CONVFMTidx;
extern int OFMTidx;
extern NODE *FS_node, *NF_node, *RS_node, *NR_node;
extern NODE *FILENAME_node, *OFS_node, *ORS_node, *OFMT_node;
extern NODE *CONVFMT_node;
extern NODE *FNR_node, *RLENGTH_node, *RSTART_node, *SUBSEP_node;
extern NODE *IGNORECASE_node;
extern NODE *FIELDWIDTHS_node;

extern NODE **stack_ptr;
extern NODE *Nnull_string;
extern NODE **fields_arr;
extern int sourceline;
extern char *source;
extern NODE *expression_value;

extern NODE *_t;	/* used as temporary in tree_eval */

extern const char *myname;

extern NODE *nextfree;
extern int field0_valid;
extern int strict;
extern int do_posix;
extern int do_lint;

/* ------------------------- Pseudo-functions ------------------------- */

#define is_identchar(c) (isalnum(c) || (c) == '_')


#ifndef MPROF
#define	getnode(n)	if (nextfree) n = nextfree, nextfree = nextfree->nextp;\
			else n = more_nodes()
#define	freenode(n)	((n)->nextp = nextfree, nextfree = (n))
#else
#define	getnode(n)	emalloc(n, NODE *, sizeof(NODE), "getnode")
#define	freenode(n)	free(n)
#endif

#ifdef DEBUG
#define	tree_eval(t)	r_tree_eval(t)
#else
#define	tree_eval(t)	(_t = (t),(_t) == NULL ? Nnull_string : \
			((_t)->type == Node_val ? (_t) : \
			((_t)->type == Node_var ? (_t)->var_value : \
			((_t)->type == Node_param_list ? \
			(stack_ptr[(_t)->param_cnt])->var_value : \
			r_tree_eval((_t))))))
#endif

#define	make_number(x)	mk_number((x), (MALLOC|NUM|NUMERIC|NUMBER))
#define	tmp_number(x)	mk_number((x), (MALLOC|TEMP|NUM|NUMERIC|NUMBER))

#define	free_temp(n)	if ((n)->flags&TEMP) { unref(n); } else
#define	make_string(s,l)	make_str_node((s), SZTC (l),0)
#define		SCAN			1
#define		ALREADY_MALLOCED	2

#define	cant_happen()	fatal("line %d, file: %s; bailing out", \
				__LINE__, basename(__FILE__));
#ifdef MEMDEBUG
#define memmsg(X,Y,Z,ZZ) \
	fprintf(stdout, "malloc: %s: %s: %ld 0x%08lx\n", Z, X, (long)Y, ZZ)
#if defined(__STDC__) && !defined(NO_TOKEN_PASTING)
#define free(s)	fprintf(stdout, "free: %s: 0x%08lx\n", #s, (long)s), do_free(s)
#else
#define free(s)	fprintf(stdout, "free: s: 0x%08lx\n", (long)s), do_free(s)
#endif
#else /* MEMDEBUG */
#define memmsg(x,y,z,zz)
#endif /* MEMDEBUG */

#if defined(__STDC__) && !defined(NO_TOKEN_PASTING)
#define	emalloc(var,ty,x,str)	if ((var=(ty)malloc((MALLOC_ARG_T)(x)))==NULL)\
				    fatal("%s: %s: can't allocate memory (%s)",\
					(str), #var, strerror(errno));\
				else memmsg(#var, x, str, var)
#define	erealloc(var,ty,x,str)	if((var=(ty)realloc((char *)var,\
						(MALLOC_ARG_T)(x)))==NULL)\
				    fatal("%s: %s: can't allocate memory (%s)",\
					(str), #var, strerror(errno));\
				else memmsg("re:" #var, x, str, var)
#else /* __STDC__ */
#define	emalloc(var,ty,x,str)	if ((var=(ty)malloc((MALLOC_ARG_T)(x)))==NULL)\
				    fatal("%s: %s: can't allocate memory (%s)",\
					(str), "var", strerror(errno));\
				else memmsg("var", x, str, var)
#define	erealloc(var,ty,x,str)	if((var=(ty)realloc((char *)var,\
						(MALLOC_ARG_T)(x)))==NULL)\
				    fatal("%s: %s: can't allocate memory (%s)",\
					(str), "var", strerror(errno));\
				else memmsg("re: var", x, str, var)
#endif /* __STDC__ */

#ifdef DEBUG
#define	force_number	r_force_number
#define	force_string	r_force_string
#else /* not DEBUG */
#ifdef lint
extern AWKNUM force_number();
#endif
#ifdef MSDOS
extern double _msc51bug;
#define	force_number(n)	(_msc51bug=(_t = (n),(_t->flags & NUM) ? _t->numbr : r_force_number(_t)))
#else /* not MSDOS */
#define	force_number(n)	(_t = (n),(_t->flags & NUM) ? _t->numbr : r_force_number(_t))
#endif /* MSDOS */
#define	force_string(s)	(_t = (s),(_t->flags & STR) ? _t : r_force_string(_t))
#endif /* not DEBUG */

#define	STREQ(a,b)	(*(a) == *(b) && strcmp((a), (b)) == 0)
#define	STREQN(a,b,n)	((n)&& *(a)== *(b) && strncmp((a), (b), SZTC (n)) == 0)

/* ------------- Function prototypes or defs (as appropriate) ------------- */

extern void set_NF();
extern void set_FIELDWIDTHS();
extern void set_NR();
extern void set_FNR();
extern void set_FS();
extern void set_RS();
extern void set_IGNORECASE();
extern void set_OFMT();
extern void set_CONVFMT();
extern void set_OFS();
extern void set_ORS();

/* array.c */
extern NODE *concat_exp P((NODE *tree));
extern void assoc_clear P((NODE *symbol));
extern unsigned int hash P((char *s, int len));
extern int in_array P((NODE *symbol, NODE *subs));
extern NODE **assoc_lookup P((NODE *symbol, NODE *subs));
extern void do_delete P((NODE *symbol, NODE *tree));
extern void assoc_scan P((NODE *symbol, struct search *lookat));
extern void assoc_next P((struct search *lookat));
/* awk.tab.c */
extern char *tokexpand P((void));
extern char nextc P((void));
extern NODE *node P((NODE *left, NODETYPE op, NODE *right));
extern NODE *install P((char *name, NODE *value));
extern NODE *lookup P((char *name));
extern NODE *variable P((char *name, int can_free));
extern int yyparse P((void));
/* builtin.c */
extern NODE *do_exp P((NODE *tree));
extern NODE *do_index P((NODE *tree));
extern NODE *do_int P((NODE *tree));
extern NODE *do_length P((NODE *tree));
extern NODE *do_log P((NODE *tree));
extern NODE *do_sprintf P((NODE *tree));
extern void do_printf P((NODE *tree));
extern void print_simple P((NODE *tree, FILE *fp));
extern NODE *do_sqrt P((NODE *tree));
extern NODE *do_substr P((NODE *tree));
extern NODE *do_strftime P((NODE *tree));
extern NODE *do_systime P((NODE *tree));
extern NODE *do_system P((NODE *tree));
extern void do_print P((NODE *tree));
extern NODE *do_tolower P((NODE *tree));
extern NODE *do_toupper P((NODE *tree));
extern NODE *do_atan2 P((NODE *tree));
extern NODE *do_sin P((NODE *tree));
extern NODE *do_cos P((NODE *tree));
extern NODE *do_rand P((NODE *tree));
extern NODE *do_srand P((NODE *tree));
extern NODE *do_match P((NODE *tree));
extern NODE *do_gsub P((NODE *tree));
extern NODE *do_sub P((NODE *tree));
/* debug.c */
extern int ptree P((NODE *n));
extern NODE *pt P((void));
extern int print_parse_tree P((NODE *ptr));
extern int dump_vars P((void));
extern int dump_fields P((void));
extern int print_debug P((char *str, void * n));
extern int print_a_node P((NODE *ptr));
extern int print_maybe_semi P((NODE *ptr));
extern int deal_with_curls P((NODE *ptr));
extern NODE *do_prvars P((void));
extern NODE *do_bp P((void));
extern void do_free P((char *s));
/* dfa.c */
extern void regsyntax P((int bits, int fold));
extern void regparse P((const char *s, size_t len, struct regexp *r));
extern void reganalyze P((struct regexp *r, int searchflag));
extern void regstate P((int s, struct regexp *r, int trans[]));
extern char *regexecute P((struct regexp *r, char *begin,
			   char *end, int newline, int *count, int *backref));
extern void reginit P((struct regexp *r));
extern void regcompile P((const char *s, size_t len,
			  struct regexp *r, int searchflag));
extern void regfree P((struct regexp *r));
/* eval.c */
extern int interpret P((NODE *tree));
extern NODE *r_tree_eval P((NODE *tree));
extern int cmp_nodes P((NODE *t1, NODE *t2));
extern NODE **get_lhs P((NODE *ptr, Func_ptr *assign));
extern void set_IGNORECASE P((void));
/* field.c */
extern void init_fields P((void));
extern void set_record P((char *buf, int cnt, int freeold));
extern void reset_record P((void));
extern void set_NF P((void));
extern NODE **get_field P((int num, Func_ptr *assign));
extern NODE *do_split P((NODE *tree));
extern void set_FS P((void));
extern void set_RS P((void));
extern void set_FIELDWIDTHS P((void));
/* io.c */
extern void set_FNR P((void));
extern void set_NR P((void));
extern void do_input P((void));
extern struct redirect *redirect P((NODE *tree, int *errflg));
extern NODE *do_close P((NODE *tree));
extern int flush_io P((void));
extern int close_io P((void));
extern int devopen P((char *name, char *mode));
extern int pathopen P((char *file));
extern NODE *do_getline P((NODE *tree));
/* iop.c */
extern int optimal_bufsize P((int fd));
extern IOBUF *iop_alloc P((int fd));
extern int get_a_record P((char **out, IOBUF *iop, int rs));
/* main.c */
extern int main P((int argc, char **argv));
extern Regexp *mk_re_parse P((char *s, int ignorecase));
extern void load_environ P((void));
extern char *arg_assign P((char *arg));
extern SIGTYPE catchsig P((int sig, int code));
extern const char *basename P((const char *));
/* msg.c */
#if 0 /* old varargs.h stuff */
extern void msg P((int va_alist));
extern void warning P((int va_alist));
extern void fatal P((int va_alist));
#endif
void msg ( char *, ...);
void warning ( char *, ...);
void fatal ( char *, ... );
/* node.c */
extern AWKNUM r_force_number P((NODE *n));
extern NODE *r_force_string P((NODE *s));
extern NODE *dupnode P((NODE *n));
extern NODE *mk_number P((AWKNUM x, unsigned int flags));
extern NODE *make_str_node P((char *s, size_t len, int scan ));
extern NODE *tmp_string P((char *s, size_t len ));
extern NODE *more_nodes P((void));
#ifdef DEBUG
extern void freenode P((NODE *it));
#endif
extern void unref P((NODE *tmp));
extern int parse_escape P((char **string_ptr));
/* re.c */
extern Regexp *make_regexp P((NODE *s, int ignorecase, int dfa));
extern int research P((Regexp *rp, char *str, int len, int need_start));
extern void refree P((Regexp *rp));
extern void regerror P((const char *s));
extern Regexp *re_update P((NODE *t));
/* regex.c */
extern int re_set_syntax P((int syntax));
extern char *re_compile_pattern P((char *pattern,
		 size_t size,
		 struct re_pattern_buffer *bufp ));

extern int re_search P((struct re_pattern_buffer *pbufp,
		 char *string,
		 int size,
		 int startpos,
		 int range,
		 struct re_registers *regs ));
extern void re_compile_fastmap P((struct re_pattern_buffer *bufp));
/* strcase.c */
extern int strcasecmp P((const char *s1, const char *s2));
extern int strncasecmp P((const char *s1, const char *s2, register size_t n));

#ifdef atarist
/* atari/tmpnam.c */
extern char *tmpnam P((char *buf));
extern char *tempnam P((const char *path, const char *base));
#endif

/* Figure out what '\a' really is. */
#ifdef __STDC__
#define BELL	'\a'		/* sure makes life easy, don't it? */
#else
#	if 'z' - 'a' == 25	/* ascii */
#		if 'a' != 97	/* machine is dumb enough to use mark parity */
#			define BELL	'\207'
#		else
#			define BELL	'\07'
#		endif
#	else
#		define BELL	'\057'
#	endif
#endif

extern char casetable[];	/* for case-independent regexp matching */
