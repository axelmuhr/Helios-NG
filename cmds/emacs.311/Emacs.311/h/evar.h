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
	"lterm",		/* current line terminator for writes */
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
	"ypos"	 		/* current mouse Y position */
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
#define	EVCURWIND	12
#define EVCWLINE	13
#define EVDEBUG         14
#define	EVDESKCLR	15
#define EVDIAGFLAG      16
#define EVDISCMD        17
#define EVDISINP        18
#define	EVDISPHIGH	19
#define EVEXBHOOK       20
#define EVFCOL		21
#define EVFILLCOL	22
#define EVFLICKER	23
#define	EVFMTLEAD	24
#define EVGFLAGS	25
#define EVGMODE 	26
#define	EVHARDTAB	27
#define EVHJUMP		28
#define EVHSCROLL	29
#define EVKILL          30
#define EVLANG          31
#define EVLASTKEY       32
#define EVLASTMESG      33
#define EVLINE          34
#define	EVLTERM		35
#define EVLWIDTH        36
#define EVMATCH         37
#define EVMODEFLAG      38
#define EVMSFLAG        39
#define	EVNUMWIND	40
#define	EVOCRYPT	41
#define	EVORGCOL	42
#define	EVORGROW	43
#define EVPAGELEN       44
#define EVPALETTE       45
#define	EVPARALEAD	46
#define EVPENDING       47
#define	EVPOPFLAG	48
#define EVPROGNAME      49
#define EVRAM           50
#define EVREADHK        51
#define	EVREGION	52
#define EVREPLACE       53
#define EVRVAL          54
#define EVSCRNAME	55
#define EVSEARCH        56
#define EVSEARCHPNT	57
#define EVSEED          58
#define EVSOFTTAB	59
#define EVSRES          60
#define EVSSAVE         61
#define EVSSCROLL	62
#define EVSTATUS	63
#define EVSTERM 	64
#define EVTARGET	65
#define EVTIME		66
#define EVTPAUSE	67
#define EVVERSION	68
#define	EVWCHARS	69
#define EVWLINE 	70
#define EVWRAPHK	71
#define	EVWRITEHK	72
#define EVXPOS		73
#define	EVYANKFLAG	74
#define EVYPOS		75

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
