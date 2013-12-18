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
	"backup",		/* Create emacs.bak file	*/
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
	"curwind",		/* current window ordinal on current screen */
	"cwline",		/* current screen line in window */
	"debug",		/* macro debugging */
	"deskcolor",		/* desktop color */
	"diagflag",		/* diagonal mouse movements enabled? */
	"discmd",		/* display commands on command line */
	"disinp",		/* display command line input characters */
	"disphigh",		/* display high bit characters escaped */
	"exbhook",		/* exit buffer switch hook */
	"fcol",			/* first displayed column in curent window */
	"fillcol",		/* current fill column */
	"flicker",		/* flicker supression */
	"fmtlead",		/* format command lead characters */
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
	"lterm",		/* current line terminator for writes */
	"ltype",		/* line type, normal, start of fold etc */
	"lwidth",		/* width of current line */
	"match",		/* last matched magic pattern */
	"modeflag",		/* Modelines displayed flag */
	"msflag",		/* activate mouse? */
	"numwind",		/* number of windows on current screen */
	"oldcrypt",		/* use old(broken) encryption */
	"orgcol",		/* screen origin column */
	"orgrow",		/* screen origin row */
	"pagelen",		/* number of lines used by editor */
	"palette",		/* current palette string */
	"paralead",		/* paragraph leadin characters */
	"pending",		/* type ahead pending flag */
	"popflag",		/* pop-up windows active? */
	"progname",		/* returns current prog name - "MicroEMACS" */
	"ram",			/* ram in use by malloc */
	"readhook",		/* read file execution hook */
	"region",		/* current region (read only) */
	"replace",		/* replacement pattern */
	"rval", 		/* child process return value */
	"scrname",		/* current screen name */
	"search",		/* search pattern */
	"searchpnt",		/* differing search styles (term point) */
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
	"wchars",		/* set of characters legal in words */
	"wline",		/* # of lines in current window */
	"wraphook",		/* wrap word execution hook */
	"writehook",		/* write file hook */
	"xpos", 		/* current mouse X position */
	"yankflag",		/* point placement at yanked/included text */
	"ypos",	 		/* current mouse Y position */
	"zpos"			/* current entered fold depth */
};

#define NEVARS	sizeof(envars) / sizeof(char *)

/*	and its preprocesor definitions 	*/

#define EVACOUNT	0
#define EVASAVE 	1
#define EVBACKUP	2
#define	EVBUFHOOK	3
#define EVCBFLAGS	4
#define EVCBUFNAME	5
#define EVCFNAME	6
#define EVCMDHK 	7
#define EVCMODE 	8
#define EVCURCHAR	9
#define EVCURCOL	10
#define EVCURLINE	11
#define EVCURWIDTH	12
#define	EVCURWIND	13
#define EVCWLINE	14
#define EVDEBUG         15
#define	EVDESKCLR	16
#define EVDIAGFLAG      17
#define EVDISCMD        18
#define EVDISINP        19
#define	EVDISPHIGH	20
#define EVEXBHOOK       21
#define EVFCOL		22
#define EVFILLCOL	23
#define EVFLICKER	24
#define	EVFMTLEAD	25
#define EVGFLAGS	26
#define EVGMODE 	27
#define	EVHARDTAB	28
#define EVHJUMP		29
#define EVHSCROLL	30
#define EVKILL          31
#define EVLANG          32
#define EVLASTKEY       33
#define EVLASTMESG      34
#define EVLINE          35
#define EVLMARGIN	36
#define	EVLTERM		37
#define EVLTYPE		38
#define EVLWIDTH        39
#define EVMATCH         40
#define EVMODEFLAG      41
#define EVMSFLAG        42
#define	EVNUMWIND	43
#define	EVOCRYPT	44
#define	EVORGCOL	45
#define	EVORGROW	46
#define EVPAGELEN       47
#define EVPALETTE       48
#define	EVPARALEAD	49
#define EVPENDING       50
#define	EVPOPFLAG	51
#define EVPROGNAME      52
#define EVRAM           53
#define EVREADHK        54
#define	EVREGION	55
#define EVREPLACE       56
#define EVRVAL          57
#define EVSCRNAME	58
#define EVSEARCH        59
#define EVSEARCHPNT	60
#define EVSEED          61
#define EVSOFTTAB	62
#define EVSRES          63
#define EVSSAVE         64
#define EVSSCROLL	65
#define EVSTATUS	66
#define EVSTERM 	67
#define EVTARGET	68
#define EVTIME		69
#define EVTPAUSE	70
#define EVVERSION	71
#define	EVWCHARS	72
#define EVWLINE 	73
#define EVWRAPHK	74
#define	EVWRITEHK	75
#define EVXPOS		76
#define	EVYANKFLAG	77
#define EVYPOS		78
#define EVZPOS		79

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
	"gro", MONAMIC,		/* return group match in MAGIC mode */
	"gtc", NILNAMIC,	/* get 1 emacs command */
	"gtk", NILNAMIC,	/* get 1 charater */
	"ind", MONAMIC, 	/* evaluate indirect value */
	"isn", MONAMIC,		/* is the arg a number? */
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
#define UFGROUP		17
#define UFGTCMD 	18
#define UFGTKEY 	19
#define UFIND		20
#define	UFISNUM		21
#define UFLEFT		22
#define UFLENGTH	23
#define UFLESS		24
#define UFLOWER 	25
#define UFMID		26
#define UFMOD		27
#define UFNEG		28
#define UFNOT		29
#define UFOR		30
#define UFRIGHT 	31
#define UFRND		32
#define UFSEQUAL	33
#define UFSGREAT	34
#define UFSINDEX	35
#define UFSLESS 	36
#define	UFSLOWER	37
#define UFSUB		38
#define	UFSUPPER	39
#define UFTIMES 	40
#define	UFTRIM		41
#define UFTRUTH 	42
#define UFUPPER 	43
#define UFXLATE 	44
