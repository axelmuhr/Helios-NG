/*
 * main.c -- Expression tree constructors and main program for gawk. 
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
#include "patchlevel.h"

static void usage P((void));
static void copyleft P((void));
static void cmdline_fs P((char *str));
static void init_args P((int argc0, int argc, char *argv0, char **argv));
static void init_vars P((void));
static void pre_assign P((char *v));
SIGTYPE catchsig P((int sig, int code));
static void gawk_option P((char *optstr));
static void nostalgia P((void));

/* These nodes store all the special variables AWK uses */
NODE *FS_node, *NF_node, *RS_node, *NR_node;
NODE *FILENAME_node, *OFS_node, *ORS_node, *OFMT_node;
NODE *CONVFMT_node;
NODE *FNR_node, *RLENGTH_node, *RSTART_node, *SUBSEP_node;
NODE *ENVIRON_node, *IGNORECASE_node;
NODE *ARGC_node, *ARGV_node;
NODE *FIELDWIDTHS_node;

int NF;
int NR;
int FNR;
int IGNORECASE;
char *FS;
char *RS;
char *OFS;
char *ORS;
char *OFMT;
char *CONVFMT;

/*
 * The parse tree and field nodes are stored here.  Parse_end is a dummy item
 * used to free up unneeded fields without freeing the program being run 
 */
int errcount = 0;	/* error counter, used by yyerror() */

/* The global null string */
NODE *Nnull_string;

/* The name the program was invoked under, for error messages */
const char *myname;

/* A block of AWK code to be run before running the program */
NODE *begin_block = 0;

/* A block of AWK code to be run after the last input file */
NODE *end_block = 0;

int exiting = 0;		/* Was an "exit" statement executed? */
int exit_val = 0;		/* optional exit value */

#if defined(YYDEBUG) || defined(DEBUG)
extern int yydebug;
#endif

char **srcfiles = NULL;		/* source file name(s) */
int numfiles = -1;		/* how many source files */
char *cmdline_src = NULL;	/* if prog is on command line */

int strict = 0;			/* turn off gnu extensions */
int do_posix = 0;		/* turn off gnu extensions and \x */
int do_lint = 0;		/* provide warnings about questionable stuff */

int output_is_tty = 0;		/* control flushing of output */

extern char *version_string;	/* current version, for printing */

NODE *expression_value;

/*
 * for strict to work, legal options must be first
 *
 * Unfortunately, -a and -e are orthogonal to -c.
 *
 * Note that after 2.13, c,a,e,C,D, and V go away.
 */
#ifdef DEBUG
char awk_opts[] = "F:f:v:W:caeCVD";
#else
char awk_opts[] = "F:f:v:W:caeCV";
#endif

int
main(argc, argv)
int argc;
char **argv;
{
	int c;
	extern int optind;
	extern char *optarg;
	int i;
	int do_nostalgia;
	int regex_mode = RE_SYNTAX_AWK;

	(void) signal(SIGFPE,  (SIGTYPE (*) P((int))) catchsig);
	(void) signal(SIGSEGV, (SIGTYPE (*) P((int))) catchsig);
#ifdef VMS
	(void) signal(SIGBUS,  (SIGTYPE (*) P((int))) catchsig);
#endif

#ifndef VMS
	myname = basename(argv[0]);
#else	/* VMS */
	myname = strdup(basename(argv[0]));
	argv[0] = (char *) myname;   /* strip path [prior to getopt()] */
	vms_arg_fixup(&argc, &argv); /* emulate redirection, expand wildcards */
#endif
	if (argc < 2)
		usage();

	/* remove sccs gunk */
	if (strncmp(version_string, "@(#)", 4) == 0)
		version_string += 4;

	/* initialize the null string */
	Nnull_string = make_string("", 0);
	Nnull_string->numbr = 0.0;
	Nnull_string->type = Node_val;
	Nnull_string->flags = (PERM|STR|STRING|NUM|NUMERIC|NUMBER);

	/* Set up the special variables */

	/*
	 * Note that this must be done BEFORE arg parsing else -F
	 * breaks horribly 
	 */
	init_vars();

	/* worst case */
	emalloc(srcfiles, char **, argc * sizeof(char *), "main");
	srcfiles[0] = NULL;

	/* undocumented feature, inspired by nostalgia, and a T-shirt */
	do_nostalgia = 0;
	for (i = 1; i < argc && argv[i][0] == '-'; i++) {
		if (argv[i][1] == '-')		/* -- */
			break;
		else if (argv[i][1] == 'c') {	/* compat not in next release */
			do_nostalgia = 0;
			break;
		} else if (STREQ(&argv[i][1], "nostalgia"))
			do_nostalgia = 1;
			/* keep looping, in case -c after -nostalgia */
	}
	if (do_nostalgia) {
		fprintf(stderr, "%s, %s\n",
		"warning: option -nostalgia will go away in the next release",
		"use -W nostalgia");
		nostalgia();
		/* NOTREACHED */
	}

	while ((c = getopt (argc, argv, awk_opts)) != EOF) {
		switch (c) {
#ifdef DEBUG
		case 'D':
			fprintf(stderr,
"warning: option -D will go away in the next release, use -W parsedebug\n");
			gawk_option("parsedebug");
			break;
#endif

		case 'c':
			fprintf(stderr,
	"warning: option -c will go away in the next release, use -W compat\n");
			gawk_option("compat");
			break;

		case 'F':
			cmdline_fs(optarg);
			break;

		case 'f':
			/*
			 * a la MKS awk, allow multiple -f options.
			 * this makes function libraries real easy.
			 * most of the magic is in the scanner.
			 */
			srcfiles[++numfiles] = optarg;
			break;

		case 'v':
			pre_assign(optarg);
			break;

		case 'V':
			warning(
		"option -V will go away in the next release, use -W version");
			gawk_option("version");
			break;

		case 'C':
			warning(
		"option -C will go away in the next release, use -W copyright");
			gawk_option("copyright");
			break;

		case 'a':	/* use old fashioned awk regexps */
			warning("option -a will go away in the next release");
			/*regex_mode = RE_SYNTAX_AWK;*/
			break;

		case 'e':	/* use Posix style regexps */
			warning("option -e will go away in the next release");
			/*regex_mode = RE_SYNTAX_POSIX_AWK;*/
			break;

		case 'W':       /* gawk specific options */
			gawk_option(optarg);
			break;

		case '?':
		default:
			/* getopt will print a message for us */
			/* S5R4 awk ignores bad options and keeps going */
			break;
		}
	}

	/* Tell the regex routines how they should work. . . */
	(void) re_set_syntax(regex_mode);
	regsyntax(regex_mode, 0);

#ifdef DEBUG
	setbuf(stdout, (char *) NULL);	/* make debugging easier */
#endif
	if (isatty(fileno(stdout)))
		output_is_tty = 1;
	/* No -f option, use next arg */
	/* write to temp file and save sourcefile name */
	if (numfiles == -1) {
		if (optind > argc - 1)	/* no args left */
			usage();
		cmdline_src = argv[optind];
		optind++;
	}
	srcfiles[++numfiles] = NULL;
	init_args(optind, argc, (char *) myname, argv);
	(void) tokexpand();

	/* Read in the program */
	if (yyparse() || errcount)
		exit(1);

	/* Set up the field variables */
	init_fields();

	if (begin_block)
		(void) interpret(begin_block);
	if (!exiting && (expression_value || end_block))
		do_input();
	if (end_block)
		(void) interpret(end_block);
	if (close_io() != 0 && exit_val == 0)
		exit_val = 1;
	exit(exit_val);		/* more portable */
	return exit_val;	/* to suppress warnings */
}

static void
usage()
{
	char *opt1 = " -f progfile [--]";
	char *opt2 = " [--] 'program'";
	char *regops = " [-F fs] [-v var=val] [-W gawk-opts]";

	fprintf(stderr, "usage: %s%s%s file ...\n       %s%s%s file ...\n",
		myname, regops, opt1, myname, regops, opt2);
	exit(11);
}

Regexp *
mk_re_parse(s, ignorecase)
char *s;
int ignorecase;
{
	char *src;
	register char *dest;
	register int c;
	int in_brack = 0;

	for (dest = src = s; *src != '\0';) {
		if (*src == '\\') {
			c = *++src;
			switch (c) {
			case '/':
			case 'a':
			case 'b':
			case 'f':
			case 'n':
			case 'r':
			case 't':
			case 'v':
			case 'x':
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
				c = parse_escape(&src);
				if (c < 0)
					cant_happen();
				*dest++ = (char)c;
				break;
			default:
				*dest++ = '\\';
				*dest++ = (char)c;
				src++;
				break;
			}
		} else if (*src == '/' && ! in_brack)
			break;
		else {
			if (*src == '[')
				in_brack = 1;
			else if (*src == ']')
				in_brack = 0;

			*dest++ = *src++;
		}
	}
	return make_regexp(tmp_string(s, dest-s), ignorecase, 1);
}

static void
copyleft ()
{
	static char blurb[] =
"Copyright (C) 1989, Free Software Foundation.\n\
GNU Awk comes with ABSOLUTELY NO WARRANTY.  This is free software, and\n\
you are welcome to distribute it under the terms of the GNU General\n\
Public License, which covers both the warranty information and the\n\
terms for redistribution.\n\n\
You should have received a copy of the GNU General Public License along\n\
with this program; if not, write to the Free Software Foundation, Inc.,\n\
675 Mass Ave, Cambridge, MA 02139, USA.\n";

	fprintf(stderr, "%s, patchlevel %d\n", version_string, PATCHLEVEL);
	fputs(blurb, stderr);
	fflush(stderr);
}

static void
cmdline_fs(str)
char *str;
{
	register NODE **tmp;
	int len = strlen(str);

	tmp = get_lhs(FS_node, (Func_ptr *) 0);
	unref(*tmp);
	/*
	 * Only if in full compatibility mode check for the stupid special
	 * case so -F\t works as documented in awk even though the shell
	 * hands us -Ft.  Bleah!
	 */
	if (strict && str[0] == 't' && str[1] == '\0')
		str[0] = '\t';
	*tmp = make_str_node(str, len, SCAN);	/* do process escapes */
	set_FS();
}

static void
init_args(argc0, argc, argv0, argv)
int argc0, argc;
char *argv0;
char **argv;
{
	int i, j;
	NODE **aptr;

	ARGV_node = install("ARGV", node(Nnull_string, Node_var, (NODE *)NULL));
	aptr = assoc_lookup(ARGV_node, tmp_number(0.0));
	*aptr = make_string(argv0, strlen(argv0));
	(*aptr)->flags |= MAYBE_NUM;
	for (i = argc0, j = 1; i < argc; i++) {
		aptr = assoc_lookup(ARGV_node, tmp_number((AWKNUM) j));
		*aptr = make_string(argv[i], strlen(argv[i]));
		(*aptr)->flags |= MAYBE_NUM;
		j++;
	}
	ARGC_node = install("ARGC",
			node(make_number((AWKNUM) j), Node_var, (NODE *) NULL));
}

/*
 * Set all the special variables to their initial values.
 */
struct varinit {
	NODE **spec;
	char *name;
	NODETYPE type;
	char *strval;
	AWKNUM numval;
	Func_ptr assign;
};
static struct varinit varinit[] = {
{&NF_node,	"NF",		Node_NF,		0,	-1, set_NF },
{&FIELDWIDTHS_node, "FIELDWIDTHS", Node_FIELDWIDTHS,	"",	0,  0 },
{&NR_node,	"NR",		Node_NR,		0,	0,  set_NR },
{&FNR_node,	"FNR",		Node_FNR,		0,	0,  set_FNR },
{&FS_node,	"FS",		Node_FS,		" ",	0,  0 },
{&RS_node,	"RS",		Node_RS,		"\n",	0,  set_RS },
{&IGNORECASE_node, "IGNORECASE", Node_IGNORECASE,	0,	0,  set_IGNORECASE },
{&FILENAME_node, "FILENAME",	Node_var,		"-",	0,  0 },
{&OFS_node,	"OFS",		Node_OFS,		" ",	0,  set_OFS },
{&ORS_node,	"ORS",		Node_ORS,		"\n",	0,  set_ORS },
{&OFMT_node,	"OFMT",		Node_OFMT,		"%.6g",	0,  set_OFMT },
{&CONVFMT_node,	"CONVFMT",	Node_CONVFMT,		"%.6g",	0,  set_CONVFMT },
{&RLENGTH_node, "RLENGTH",	Node_var,		0,	0,  0 },
{&RSTART_node,	"RSTART",	Node_var,		0,	0,  0 },
{&SUBSEP_node,	"SUBSEP",	Node_var,		"\034",	0,  0 },
{0,		0,		Node_illegal,		0,	0,  0 },
};

static void
init_vars()
{
	register struct varinit *vp;

	for (vp = varinit; vp->name; vp++) {
		*(vp->spec) = install(vp->name,
		  node(vp->strval == 0 ? make_number(vp->numval)
				: make_string(vp->strval, strlen(vp->strval)),
		       vp->type, (NODE *) NULL));
		if (vp->assign)
			(*(vp->assign))();
	}
}

void
load_environ()
{
	extern char **environ;
	register char *var, *val;
	NODE **aptr;
	register int i;

	ENVIRON_node = install("ENVIRON", 
			node(Nnull_string, Node_var, (NODE *) NULL));
	for (i = 0; environ[i]; i++) {
		static char nullstr[] = "";

		var = environ[i];
		val = strchr(var, '=');
		if (val)
			*val++ = '\0';
		else
			val = nullstr;
		aptr = assoc_lookup(ENVIRON_node, tmp_string(var, strlen (var)));
		*aptr = make_string(val, strlen (val));
		(*aptr)->flags |= MAYBE_NUM;

		/* restore '=' so that system() gets a valid environment */
		if (val != nullstr)
			*--val = '=';
	}
}

/* Process a command-line assignment */
char *
arg_assign(arg)
char *arg;
{
	char *cp;
	Func_ptr after_assign = NULL;
	NODE *var;
	NODE *it;
	NODE **lhs;

	cp = strchr(arg, '=');
	if (cp != NULL) {
		*cp++ = '\0';
		/*
		 * Recent versions of nawk expand escapes inside assignments.
		 * This makes sense, so we do it too.
		 */
		it = make_str_node(cp, strlen(cp), SCAN);
		it->flags |= MAYBE_NUM;
		var = variable(arg, 0);
		lhs = get_lhs(var, &after_assign);
		unref(*lhs);
		*lhs = it;
		if (after_assign)
			(*after_assign)();
		*--cp = '=';	/* restore original text of ARGV */
	}
	return cp;
}

static void
pre_assign(v)
char *v;
{
	if (!arg_assign(v)) {
		fprintf (stderr,
			"%s: '%s' argument to -v not in 'var=value' form\n",
				myname, v);
		usage();
	}
}

SIGTYPE
catchsig(sig, code)
int sig, code;
{
#ifdef lint
	code = 0; sig = code; code = sig;
#endif
	if (sig == SIGFPE) {
		fatal("floating point exception");
#ifndef VMS
	} else if (sig == SIGSEGV) {
		msg("fatal error: segmentation fault");
#else
	} else if (sig == SIGSEGV || sig == SIGBUS) {
		msg("fatal error: access violation");
#endif
		/* fatal won't abort() if not compiled for debugging */
		abort();
	} else
		cant_happen();
	/* NOTREACHED */
}

/* gawk_option --- do gawk specific things */

static void
gawk_option(optstr)
char *optstr;
{
	char *cp;

	for (cp = optstr; *cp; cp++) {
		switch (*cp) {
		case ' ':
		case '\t':
		case ',':
			break;
		case 'v':
		case 'V':
			/* print version */
			if (strncasecmp(cp, "version", 7) != 0)
				goto unknown;
			else
				cp += 6;
			fprintf(stderr, "%s, patchlevel %d\n",
					version_string, PATCHLEVEL);
			break;
		case 'c':
		case 'C':
			if (strncasecmp(cp, "copyright", 9) == 0) {
				cp += 8;
				copyleft();
			} else if (strncasecmp(cp, "copyleft", 8) == 0) {
				cp += 7;
				copyleft();
			} else if (strncasecmp(cp, "compat", 6) == 0) {
				cp += 5;
				strict = 1;
			} else
				goto unknown;
			break;
		case 'n':
		case 'N':
			if (strncasecmp(cp, "nostalgia", 9) != 0)
				goto unknown;
			nostalgia();
			break;
		case 'p':
		case 'P':
#ifdef DEBUG
			if (strncasecmp(cp, "parsedebug", 10) == 0) {
				cp += 10;
				yydebug = 2;
				break;
			}
#endif
			if (strncasecmp(cp, "posix", 5) != 0)
				goto unknown;
			cp += 4;
			do_posix = 1;
			strict = 1;
			break;
		case 'l':
		case 'L':
			if (strncasecmp(cp, "lint", 4) != 0)
				goto unknown;
			cp += 3;
			do_lint = 1;
			break;
		default:
		unknown:
			fprintf(stderr, "'%c' -- unknown option, ignored\n",
				*cp);
			break;
		}
	}
}

/* nostalgia --- print the famous error message and die */

static void
nostalgia()
{
	fprintf(stderr, "awk: bailing out near line 1\n");
	abort();
}

const char *
basename(filespec)
const char *filespec;
{
#ifndef VMS	/* "path/name" -> "name" */
	char *p = strrchr(filespec, '/');

#if defined(MSDOS) || defined(atarist)
	char *q = strrchr(filespec, '\\');

	if (p == NULL || q > p)
		p = q;
#endif

	return (p == NULL ? filespec : (const char *)(p + 1));

#else		/* "device:[root.][directory.subdir]GAWK.EXE;n" -> "GAWK" */
	static char buf[255+1];
	char *p = strrchr(filespec, ']');  /* directory punctuation */
	char *q = strrchr(filespec, '>');  /* alternate <international> punct */

	if (p == NULL || q > p)
		p = q;
	(void) strcpy(buf, p == NULL ? filespec : (p + 1));
	q = strrchr(buf, '.');
	if (q != NULL)
		*q = '\0';	/* strip .type;version */

	return (const char *) buf;

#endif /*VMS*/
}
