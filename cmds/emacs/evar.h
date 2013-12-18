/*	EVAR.H:	Environment and user variable definitions
		for MicroEMACS

		written 1986 by Daniel Lawrence

	$Header: /hsrc/cmds/emacs/RCS/evar.h,v 1.1 1990/08/23 15:14:29 jon Exp $

*/

/*	structure to hold user variables and their definitions	*/

typedef struct UVAR {
	char u_name[NVSIZE + 1];		/* name of user variable */
	char *u_value;				/* value (string) */
} UVAR;

/*	current user variables (This structure will probably change)	*/

#define	MAXVARS		100

UVAR uv[MAXVARS];	/* user variables */

/*	list of recognized environment variables	*/

char *envars[] = {
	"fillcol",		/* current fill column */
	"pagelen",		/* number of lines used by editor */
	"curcol",		/* current column pos of cursor */
	"curline",		/* current line in file */
	"ram",			/* ram in use by malloc */
	"flicker",		/* flicker supression */
	"curwidth",		/* current screen width */
	"cbufname",		/* current buffer name */
	"cfname",		/* current file name */
	"sres",			/* current screen resolution */
	"debug",		/* macro debugging */
	"status",		/* returns the status of the last command */
	"palette",		/* current palette string */
	"asave",		/* # of chars between auto-saves */
	"acount",		/* # of chars until next auto-save */
	"lastkey",		/* last keyboard char struck */
	"curchar",		/* current character under the cursor */
	"discmd",		/* display commands on command line */
	"version",		/* current version number */
	"progname",		/* returns current prog name - "MicroEMACS" */
	"seed",			/* current random number seed */
	"disinp",		/* display command line input characters */
};

#define	NEVARS	sizeof(envars) / sizeof(char *)

/* 	and its preprocesor definitions		*/

#define	EVFILLCOL	0
#define	EVPAGELEN	1
#define	EVCURCOL	2
#define	EVCURLINE	3
#define	EVRAM		4
#define	EVFLICKER	5
#define	EVCURWIDTH	6
#define	EVCBUFNAME	7
#define	EVCFNAME	8
#define	EVSRES		9
#define	EVDEBUG		10
#define	EVSTATUS	11
#define	EVPALETTE	12
#define	EVASAVE		13
#define	EVACOUNT	14
#define	EVLASTKEY	15
#define	EVCURCHAR	16
#define	EVDISCMD	17
#define	EVVERSION	18
#define	EVPROGNAME	19
#define	EVSEED		20
#define	EVDISINP	21

/*	list of recognized user functions	*/

typedef struct UFUNC {
	char *f_name;	/* name of function */
	int f_type;	/* 1 = monamic, 2 = dynamic */
} UFUNC;

#define	NILNAMIC	0
#define	MONAMIC		1
#define	DYNAMIC		2
#define	TRINAMIC	3

UFUNC funcs[] = {
	"add", DYNAMIC,		/* add two numbers together */
	"sub", DYNAMIC,		/* subtraction */
	"tim", DYNAMIC,		/* multiplication */
	"div", DYNAMIC,		/* division */
	"mod", DYNAMIC,		/* mod */
	"neg", MONAMIC,		/* negate */
	"cat", DYNAMIC,		/* concatinate string */
	"lef", DYNAMIC,		/* left string(string, len) */
	"rig", DYNAMIC,		/* right string(string, pos) */
	"mid", TRINAMIC,	/* mid string(string, pos, len) */
	"not", MONAMIC,		/* logical not */
	"equ", DYNAMIC,		/* logical equality check */
	"les", DYNAMIC,		/* logical less than */
	"gre", DYNAMIC,		/* logical greater than */
	"seq", DYNAMIC,		/* string logical equality check */
	"sle", DYNAMIC,		/* string logical less than */
	"sgr", DYNAMIC,		/* string logical greater than */
	"ind", MONAMIC,		/* evaluate indirect value */
	"and", DYNAMIC,		/* logical and */
	"or",  DYNAMIC,		/* logical or */
	"len", MONAMIC,		/* string length */
	"upp", MONAMIC,		/* uppercase string */
	"low", MONAMIC,		/* lower case string */
	"tru", MONAMIC,		/* Truth of the universe logical test */
	"asc", MONAMIC,		/* char to integer conversion */
	"chr", MONAMIC,		/* integer to char conversion */
	"gtk", NILNAMIC,	/* get 1 charater */
	"rnd", MONAMIC,		/* get a random number */
	"abs", MONAMIC,		/* absolute value of a number */
};

#define	NFUNCS	sizeof(funcs) / sizeof(UFUNC)

/* 	and its preprocesor definitions		*/

#define	UFADD		0
#define	UFSUB		1
#define	UFTIMES		2
#define	UFDIV		3
#define	UFMOD		4
#define	UFNEG		5
#define	UFCAT		6
#define	UFLEFT		7
#define	UFRIGHT		8
#define	UFMID		9
#define	UFNOT		10
#define	UFEQUAL		11
#define	UFLESS		12
#define	UFGREATER	13
#define	UFSEQUAL	14
#define	UFSLESS		15
#define	UFSGREAT	16
#define	UFIND		17
#define	UFAND		18
#define	UFOR		19
#define	UFLENGTH	20
#define	UFUPPER		21
#define	UFLOWER		22
#define	UFTRUTH		23
#define	UFASCII		24
#define	UFCHR		25
#define	UFGTKEY		26
#define	UFRND		27
#define	UFABS		28
