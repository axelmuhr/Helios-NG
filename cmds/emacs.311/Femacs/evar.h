/*	EVAR.H: Environment and user variable definitions
		for MicroEMACS

		written 1986 by Daniel Lawrence
*/

/*	structure to hold user variables and their definitions	*/

typedef struct UVAR {
	char u_name[NVSIZE + 1];	       /* name of user variable */
	char *u_value;				/* value (string) */
} UVAR;

/*	current user variables (This structure will probably change)	*/

#define MAXVARS 	512

UVAR NOSHARE uv[MAXVARS + 1];	/* user variables */

/*	list of recognized environment variables	*/

NOSHARE char *envars[] = {
	"acount",		/* # of chars until next auto-save */
	"asave",		/* # of chars between auto-saves */
	"bufhook",		/* enter buffer switch hook */
	"cbflags",		/* current buffer flags */
	"cbufname",		/* current buffer name */
	"cfname",		/* current file name */
	"cmdhook",		/* command loop hook */
	"cmode",		/* mode of current buffer */
	"curchar",		/* current character under the cursor */
	"curcol",		/* current column pos of cursor */
	"curline",		/* current line in file */
	"curwidth",		/* current screen width */
	"cwline",		/* current screen line in window */
	"debug",		/* macro debugging */
	"diagflag",		/* diagonal mouse movements enabled? */
	"discmd",		/* display commands on command line */
	"disinp",		/* display command line input characters */
	"exbhook",		/* exit buffer switch hook */
	"fcol",			/* first displayed column in curent window */
	"fillcol",		/* current fill column */
	"flicker",		/* flicker supression */
	"gflags",		/* global internal emacs flags */
	"gmode",		/* global modes */
	"hardtab",		/* current hard tab size */
	"hjump",		/* horizontal screen jump size */
	"hscroll",		/* horizontal scrolling flag */
	"kill", 		/* kill buffer (read only) */
	"language",		/* language of text messages */
	"lastkey",		/* last keyboard char struck */
	"lastmesg",		/* last string mlwrite()ed */
	"line", 		/* text of current line */
	"lmargin",		/* left margin (enclosing fold) */
	"ltype",		/* line type, normal, start of fold etc */
	"lwidth",		/* width of current line */
	"match",		/* last matched magic pattern */
	"modeflag",		/* Modelines displayed flag */
	"msflag",		/* activate mouse? */
	"oldcrypt",		/* use old(broken) encryption */
	"pagelen",		/* number of lines used by editor */
	"palette",		/* current palette string */
	"pending",		/* type ahead pending flag */
	"progname",		/* returns current prog name - "MicroEMACS" */
	"ram",			/* ram in use by malloc */
	"readhook",		/* read file execution hook */
	"region",		/* current region (read only) */
	"replace",		/* replacement pattern */
	"rval", 		/* child process return value */
	"search",		/* search pattern */
	"seed", 		/* current random number seed */
	"softtab",		/* current soft tab size */
	"sres", 		/* current screen resolution */
	"ssave",		/* safe save flag */
	"sscroll",		/* smooth scrolling flag */
	"status",		/* returns the status of the last command */
	"sterm",		/* search terminator character */
	"target",		/* target for line moves */
	"time",			/* date and time */
	"tpause",		/* length to pause for paren matching */
	"version",		/* current version number */
	"wline",		/* # of lines in current window */
	"wraphook",		/* wrap word execution hook */
	"writehook",		/* write file hook */
	"xpos", 		/* current mouse X position */
	"ypos",	 		/* current mouse Y position */
	"zpos"			/* current entered fold depth */
};

#define NEVARS	sizeof(envars) / sizeof(char *)

/*	and its preprocesor definitions 	*/

#define EVACOUNT	0
#define EVASAVE 	1
#define	EVBUFHOOK	2
#define EVCBFLAGS	3
#define EVCBUFNAME	4
#define EVCFNAME	5
#define EVCMDHK 	6
#define EVCMODE 	7
#define EVCURCHAR	8
#define EVCURCOL	9
#define EVCURLINE	10
#define EVCURWIDTH	11
#define EVCWLINE	12
#define EVDEBUG         13
#define EVDIAGFLAG      14
#define EVDISCMD        15
#define EVDISINP        16
#define EVEXBHOOK       17
#define EVFCOL		18
#define EVFILLCOL	19
#define EVFLICKER	20
#define EVGFLAGS	21
#define EVGMODE 	22
#define	EVHARDTAB	23
#define EVHJUMP		24
#define EVHSCROLL	25
#define EVKILL          26
#define EVLANG          27
#define EVLASTKEY       28
#define EVLASTMESG      29
#define EVLINE          30
#define EVLMARGIN	31
#define EVLTYPE		32
#define EVLWIDTH        33
#define EVMATCH         34
#define EVMODEFLAG      35
#define EVMSFLAG        36
#define	EVOCRYPT	37
#define EVPAGELEN       38
#define EVPALETTE       39
#define EVPENDING       40
#define EVPROGNAME      41
#define EVRAM           42
#define EVREADHK        43
#define	EVREGION	44
#define EVREPLACE       45
#define EVRVAL          46
#define EVSEARCH        47
#define EVSEED          48
#define EVSOFTTAB	49
#define EVSRES          50
#define EVSSAVE         51
#define EVSSCROLL	52
#define EVSTATUS	53
#define EVSTERM 	54
#define EVTARGET	55
#define EVTIME		56
#define EVTPAUSE	57
#define EVVERSION	58
#define EVWLINE 	59
#define EVWRAPHK	60
#define	EVWRITEHK	61
#define EVXPOS		62
#define EVYPOS		63
#define EVZPOS		64

/*	list of recognized user functions	*/

typedef struct UFUNC {
	char *f_name;  /* name of function */
	int f_type;	/* 1 = monamic, 2 = dynamic */
} UFUNC;

#define NILNAMIC	0
#define MONAMIC 	1
#define DYNAMIC 	2
#define TRINAMIC	3

NOSHARE UFUNC funcs[] = {
	"abs", MONAMIC, 	/* absolute value of a number */
	"add", DYNAMIC,        /* add two numbers together */
	"and", DYNAMIC, 	/* logical and */
	"asc", MONAMIC, 	/* char to integer conversion */
	"ban", DYNAMIC, 	/* bitwise and	 9-10-87  jwm */
	"bin", MONAMIC, 	/* loopup what function name is bound to a key */
	"bno", MONAMIC, 	/* bitwise not */
	"bor", DYNAMIC, 	/* bitwise or	 9-10-87  jwm */
	"bxo", DYNAMIC, 	/* bitwise xor	 9-10-87  jwm */
	"cat", DYNAMIC, 	/* concatinate string */
	"chr", MONAMIC, 	/* integer to char conversion */
	"div", DYNAMIC, 	/* division */
	"env", MONAMIC, 	/* retrieve a system environment var */
	"equ", DYNAMIC, 	/* logical equality check */
	"exi", MONAMIC, 	/* check if a file exists */
	"fin", MONAMIC, 	/* look for a file on the path... */
	"gre", DYNAMIC, 	/* logical greater than */
	"gtc", NILNAMIC,	/* get 1 emacs command */
	"gtk", NILNAMIC,	/* get 1 charater */
	"ind", MONAMIC, 	/* evaluate indirect value */
	"lef", DYNAMIC, 	/* left string(string, len) */
	"len", MONAMIC, 	/* string length */
	"les", DYNAMIC, 	/* logical less than */
	"low", MONAMIC, 	/* lower case string */
	"mid", TRINAMIC,	/* mid string(string, pos, len) */
	"mod", DYNAMIC, 	/* mod */
	"neg", MONAMIC, 	/* negate */
	"not", MONAMIC, 	/* logical not */
	"or",  DYNAMIC, 	/* logical or */
	"rig", DYNAMIC, 	/* right string(string, pos) */
	"rnd", MONAMIC, 	/* get a random number */
	"seq", DYNAMIC, 	/* string logical equality check */
	"sgr", DYNAMIC, 	/* string logical greater than */
	"sin", DYNAMIC, 	/* find the index of one string in another */
	"sle", DYNAMIC, 	/* string logical less than */
	"slo", DYNAMIC,		/* set lower to upper char translation */
	"sub", DYNAMIC, 	/* subtraction */
	"sup", DYNAMIC,		/* set upper to lower char translation */
	"tim", DYNAMIC, 	/* multiplication */
	"tri", MONAMIC,		/* trim whitespace off the end of a string */
	"tru", MONAMIC, 	/* Truth of the universe logical test */
	"upp", MONAMIC, 	/* uppercase string */
	"xla", TRINAMIC		/* XLATE character string translation */
};

#define NFUNCS	sizeof(funcs) / sizeof(UFUNC)

/*	and its preprocesor definitions 	*/

#define UFABS		0
#define UFADD		1
#define UFAND		2
#define UFASCII 	3
#define UFBAND		4
#define UFBIND		5
#define UFBNOT		6
#define UFBOR		7
#define UFBXOR		8
#define UFCAT		9
#define UFCHR		10
#define UFDIV		11
#define UFENV		12
#define UFEQUAL 	13
#define UFEXIST 	14
#define UFFIND		15
#define UFGREATER	16
#define UFGTCMD 	17
#define UFGTKEY 	18
#define UFIND		19
#define UFLEFT		20
#define UFLENGTH	21
#define UFLESS		22
#define UFLOWER 	23
#define UFMID		24
#define UFMOD		25
#define UFNEG		26
#define UFNOT		27
#define UFOR		28
#define UFRIGHT 	29
#define UFRND		30
#define UFSEQUAL	31
#define UFSGREAT	32
#define UFSINDEX	33
#define UFSLESS 	34
#define	UFSLOWER	35
#define UFSUB		36
#define	UFSUPPER	37
#define UFTIMES 	38
#define	UFTRIM		39
#define UFTRUTH 	40
#define UFUPPER 	41
#define UFXLATE 	42
