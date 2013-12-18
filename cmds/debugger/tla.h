/**
*
* Title:  Helios Debugger Header File
*
* Author: Andy England (original header files)
* Date:   March 1989
*
* Author: Carsten Rietbrock (integration of header files)
* Date:   Dezember 1990
*
* -- crf : August 1991 - some modifications
*
* Author: Nick Clifton (ported to C40)
* Date:   October 1992
*
*         (c) Copyright 1988 - 1993, Perihelion Software Ltd.
*
*         All Rights Reserved.
*
* included headers: debug.h module.h monitor.h display.h key.h line.h
*		    source.h thread.h interp.h system.h table.h symbol.h
*		    expr.h eval.h cmd.h util.h info.h
*
* $Header: /hsrc/cmds/debugger/RCS/tla.h,v 1.11 1993/07/06 14:09:34 nickc Exp $
*
**/

#include <stdio.h>
#include <ctype.h>
#include <stdarg.h>
#include <signal.h>
#include <posix.h>
#include <helios.h>
#include <syslib.h>
#include <codes.h>

#include <errno.h> /* -- crf : 16/07/91 - Bug 706 */

#include <gsp.h>
#include <ioevents.h>
#include <attrib.h>
#include <fcntl.h>
#include <fault.h>
#include <stdlib.h>
#include <module.h>
#include <servlib.h>
#include <task.h>
#ifdef OLDCODE
#include <stdlib.h>
#endif
#include <string.h>
#include <nonansi.h>
#include <queue.h>
#include <setjmp.h>
#include <stddef.h>
#include "./lib/dmsg.h"
#undef print
#undef STRING


/*debug.h************************************************/


typedef long 			LONG;
typedef unsigned long		ULONG;
typedef int			BOOL;
typedef Node			NODE;
typedef List			LIST;

#define AND			&&
#define OR			||

#define strequ(  s, t )		(strcmp(  s, t )    == 0)
#define strnequ( s, t, n )	(strncmp( s, t, n ) == 0)
#define NEW( t )		(t *)newmem( sizeof (t) )

#define PathMax			512

typedef enum
  {
    Default, Ascii, Binary, Decimal, Error, Float, Hexadecimal, Octal, Unsigned, STring /* String is already defined */
  }
FORMAT;

typedef struct
  {
    struct module *	module;
    int			line;
  }
LOCATION;

#define HASH_MAX 	211  /* CR: guess this def is better here */

typedef struct chain
  {
    struct link *	head;
  }
CHAIN;

typedef struct link
  {
    struct link *	next;
  }
LINK;

typedef struct symbol
  {
    LINK 		link;
    CHAIN		entrylist;
    char		name[ 1 ];
  }
SYMBOL;

typedef CHAIN 		TABLE[ HASH_MAX ]; /* CR: was in table.h */

typedef struct debug
  {
    char 		name[ 256 ];
    LIST		modulelist;
    LIST		breakpointlist;
    LIST		watchlist;
    LIST		watchpointlist;
    LIST		threadlist;
    Port		port;
    Port		reply;
    struct thread *	thread;
    struct display *	display;
    struct interp *	interp;
    Environ		env;
    struct line *	line;
    struct eval *	eval;
    struct chain *	table; 		/* CR: was 'struct table' but there is no struct table */
    int			ifwatchpoint;	/* CR: im sorry for this doggy solution of the varwindow */
    void *		tempwatchpoint;	/*   refreshing problem */
  }
DEBUG;

PUBLIC void *		newmem(		int );
PUBLIC void		freemem(	void * );
PUBLIC void		putmem(		void );

PUBLIC void		initdebug(	int, char *[] );
PUBLIC void		debugf(		char *, ... );
PUBLIC DEBUG *		newdebug(	char * );
PUBLIC void		startdebug(	DEBUG *, Port, Port );
PUBLIC void		remdebug(	DEBUG * );

#ifdef V1_1
#define _MYSYS_OPEN	64

extern Semaphore	loadlock;
extern FILE		my_iob[ _MYSYS_OPEN ];
#endif


/*module.h*******************************************/


typedef struct module
  {
    NODE 		node;
    char *		name;
    int			modnum;
    DEBUG *		debug;
    struct source *	source;
    struct block *	outerblock;
  }
MODULE;

PUBLIC BOOL 		addmodule(	DEBUG *, char * name, int modnum );
PUBLIC void		remmodule(	MODULE * );
PUBLIC MODULE *		getmodule(	DEBUG *, int modnum );
PUBLIC MODULE *		findmodule(	DEBUG *, char * name );
PUBLIC struct source *	getsource(	MODULE * );


/*monitor.h***************************************************/


typedef struct breakpoint
  {
    NODE 		node;
    LOCATION		loc;
    int			count;
    char *		docmd;
  }
BREAKPOINT;

#ifdef OLDCODE
typedef struct watch
  {
    NODE 		node;
    union expr *	expr;
  }
WATCH;

typedef struct watchpoint
  {
    NODE 		node;
    union expr *	expr;
    LIST		watchelementlist;
    BOOL		recalc;
  }
WATCHPOINT;

typedef struct watchelement
  {
    NODE 		node;
    void *		addr;
    int			size;
  }
WATCHELEMENT;

#else /* not OLDCODE */

typedef enum
{
  C_Auto, C_Common, C_Display, C_Enum, C_Extern, C_Member, C_Param, C_Register,
  C_Static, C_Tag, C_Typedef
} CLASS;

typedef struct block *	blockptr;
typedef struct thread *	threadptr;
typedef struct eval *	evalptr;

typedef struct watchpoint
  {
    NODE 		node;
    char *		expr;
    void *		addr;
    int			size;
    char *		docmd;
    FORMAT		format;
    BOOL		silent;
    int			scope; 		/* CR: I need this to indicate the level of def */
    blockptr		block;		/* CR: Saved context for evaluation */
    threadptr		thread;		/* CR: Saved thread for evaluation */
    evalptr		eval;

/*
-- crf : 14/08/91 - related to Bug 708
-- The creation of watch windows must take into account the number of elements
-- in a watchpointed structure. 
*/
    int 		num_elements;

    /* XXX - NC - 26/4/93 - we need to save the class of variable neing watched */
    
    CLASS		Class;
  }
WATCHPOINT;

#endif /* not OLDCODE */


typedef struct
  {
    void *		addr;
    int			size;
  }
MEM_LOCATION;

PUBLIC void 		stopped(	 DEBUG *, int, int, int );
PUBLIC void		traced(		 DEBUG *, int, int, int );
PUBLIC void		entered(	 DEBUG *, int, int, int );
PUBLIC void		returned(	 DEBUG *, int, int, int );
PUBLIC void		endthread(	 DEBUG *, int );
PUBLIC void		addbreakpoint(	 DEBUG *, LOCATION, int, char * );
PUBLIC void		rembreakpoint(	 DEBUG *, LOCATION );
PUBLIC void		freebreakpoint(	 BREAKPOINT * );
PUBLIC BREAKPOINT *	findbreakpoint(	 DEBUG *, LOCATION );
PUBLIC void		listbreakpoints( DEBUG * );
PUBLIC void		remwatchpoint(	 DEBUG *, WATCHPOINT * );
PUBLIC void		freewatchpoint(	 WATCHPOINT * );
PUBLIC void		listwatchpoints( DEBUG * );


/* display.h *******************************************************/


#define DisplayBorder	6

typedef struct
  {
    int row;
    int col;
    int size;
  }
CURSOR;

typedef struct window
  {
    NODE 		node;
    struct display *	display;
    int			pos;
    int			size;
    LOCATION		loc;
    LOCATION		progloc;
    CURSOR		cur;
    FILE *		traceout;
  }
WINDOW;

typedef struct display
  {
    DEBUG *		 debug;
    FILE *		 filein;
    FILE *		 fileout;
    LIST		 windowlist;
    WINDOW *		 topwin;
    Semaphore		 lock;
    int			 height;
    int			 width;
    int			 row;
    int			 col;
    int			 varsize;

    /*
     * -- crf : 14/08/91 - related to Bug 708
     * -- Keep track of last line of watch window
     */
    int 		 varline;

    struct watchpoint ** varvec;
    Port		 breakport;
    BOOL		 breakflag;
    BOOL		 stop_display;
  }
DISPLAY;

PUBLIC DISPLAY *	dopen(		DEBUG *, char *, char * );
PUBLIC BOOL		testbreak(	DISPLAY * );
PUBLIC void		dclose(		DISPLAY * );
PUBLIC void		drefresh(	DISPLAY * );
PUBLIC void		dcursor(	DISPLAY *, int, int );
PUBLIC void		dstart(		DISPLAY * );
PUBLIC void		dend(		DISPLAY *, BOOL );
PUBLIC void		dclear(		DISPLAY * );
PUBLIC void		dinverse(	DISPLAY * );
PUBLIC void		dnormal(	DISPLAY * );
PUBLIC void		deol(		DISPLAY * );
PUBLIC void		dlock(		DISPLAY * );
PUBLIC void		dunlock(	DISPLAY * );
PUBLIC void		dprintf(	DISPLAY *, char *, ... );
PUBLIC void		xdputc(		DISPLAY *, int );
PUBLIC void		dputc(		DISPLAY *, int );
PUBLIC int		dgetc(		DISPLAY * );
PUBLIC void		raw(		DISPLAY * );
PUBLIC void		cooked(		DISPLAY * );

#ifdef OLDCODE
PUBLIC void 		vgrow(		DISPLAY * );
PUBLIC void		vshrink(	DISPLAY * );
#else
/* -- crf : 24/07/91 - Bug 708 */
PUBLIC void		vgrow( 		DISPLAY *, int );
PUBLIC void		vshrink(	DISPLAY *, int );
#endif

PUBLIC void 		vinsert(	DISPLAY *, struct watchpoint * );
PUBLIC void		vdelete(	DISPLAY *, int );
PUBLIC void		vupdate(	DISPLAY *, struct watchpoint * );
PUBLIC WINDOW *		wopen(		DISPLAY * );
PUBLIC void		wclose(		WINDOW * );
PUBLIC void		wclear(		WINDOW * );
PUBLIC void		wselect(	WINDOW * );
PUBLIC void		wgrow(		WINDOW * );
PUBLIC void		wshrink(	WINDOW * );
PUBLIC void		lowlight(	WINDOW * );
PUBLIC void		wgoto(		WINDOW *, LOCATION );
PUBLIC void		view(		WINDOW *, LOCATION );
PUBLIC void		scrollup(	WINDOW *, int );
PUBLIC void		scrolldown(	WINDOW *, int );
PUBLIC void		pageup(		WINDOW * );
PUBLIC void		pagedown(	WINDOW * );
PUBLIC void		pagefirst(	WINDOW * );
PUBLIC void		pagelast(	WINDOW * );
PUBLIC void		cursorup(	WINDOW * );
PUBLIC void		cursordown(	WINDOW * );
PUBLIC void		cursorleft(	WINDOW * );
PUBLIC void		cursorright(	WINDOW * );
PUBLIC void		cursorgrow(	WINDOW * );
PUBLIC void		cursorshrink(	WINDOW * );
PUBLIC char *		getcurtext(	WINDOW *, char * );

/*key.h******************************************/


#define MAX_KEY 	255

typedef char *		KEYMAP[ MAX_KEY + 1 ];

PUBLIC void		initkeymap(	KEYMAP );
PUBLIC void		freekeymap(	KEYMAP );
PUBLIC void		addkey(		KEYMAP, int, char * );
PUBLIC void		remkey(		KEYMAP, int );
PUBLIC char *		getkey(		KEYMAP, int );
PUBLIC void		listkeys(	KEYMAP, struct display * );

/*line.h*********************************************/


#define Shift		0xdf
#define VControl	0x9f
#define CtrlD		0x04
#define Bell		0x07
#define Backspace	0x08
#define Tab		0x09
#define Return		0x0d
#define Escape		0x1b
#define Space		0x20
#define VDelete		0x7f
#define FunctionKeys	0x80
#define PageKeys	0xc0
#define Undo		(PageKeys + 1)
#define End		(PageKeys + 2)
#define PageUp		(PageKeys + 3)
#define PageDown	(PageKeys + 4)
#define UpArrow		0xd0
#define DownArrow	0xd1
#define LeftArrow	0xd2
#define RightArrow	0xd3
#ifdef NO_LONGER_WORKS
#define ShiftUpArrow	0xd4
#define ShiftDownArrow	0xd5
#define ShiftLeftArrow	0xd6
#define ShiftRightArrow	0xd7
#endif
#define Help		0xd8
#define Home		0xd9
#define VInsert		0xda

#define CSI		0x9b

#define LineMax		255
#define SaveMax		20
#define prevslot( s )	if (--(s) < 0) (s) = SaveMax - 1
#define nextslot( s )	if (++(s) == SaveMax) (s) = 0

#define iscst( c )	((c) >= 0x20 AND (c) <= 0x7f)

typedef struct line
  {
    int 		index;
    int			length;
    BOOL		update;
    struct display *	display;
    char		buffer[ LineMax + 1 ];
    char		savebuffer[ LineMax + 1 ];
    int			firstslot;
    int			lastslot;
    int			currentslot;
    char *		vec[ SaveMax ];
    KEYMAP		keymap;
  }
LINE;

PUBLIC LINE *		newline(	struct display * );
PUBLIC void		remline(	LINE * );
PUBLIC char *		getinput(	LINE *, char *, char * );
PUBLIC char *		getline(	LINE * );
PUBLIC int		dgetkey(	DISPLAY * display );


/*source.h************************************/


#define MAX_LINEVEC 	100

typedef struct source
  {
    NODE 		node;
    char *		name;
    char **		linevec;
    int			lastline;
    int			usage;
  }
SOURCE;

PUBLIC void 		initsource(	void );
PUBLIC SOURCE *		loadsource(	DEBUG *, char * );
PUBLIC void		unloadsource(	SOURCE * );
PUBLIC SOURCE *		findsource(	char * );
PUBLIC void		list(		struct display *, SOURCE *, int, int );
PUBLIC int		search(		SOURCE *, char *, int, BOOL, BOOL );
PUBLIC char *		getword(	char *, SOURCE *, int, int, int );
PUBLIC char *		gettext(	char *, SOURCE *, int, int, int );


/*thread.h***********************************************/


typedef struct thread
  {
    NODE 		node;
    int			id;
    LOCATION		loc;
    struct block *	block;
    struct entry *	function;
    struct window *	window;
    Semaphore		sync;
  }
THREAD;

PUBLIC THREAD *		newthread(	DEBUG *, int );
PUBLIC void		remthread(	THREAD * );
PUBLIC THREAD *		findthread(	DEBUG *, int );
PUBLIC void		nextthread(	DEBUG * );
PUBLIC void		prevthread(	DEBUG * );
PUBLIC void		resume(		THREAD * );


/*interp.h****************************************************/


#define EOL      	'\0'
#define StackMax	1024

typedef struct interp
  {
    LIST 		aliaslist;
    LIST		definelist;
    int			charindex;
    char		charstack[ StackMax ];
    jmp_buf		home;
    DEBUG *		debug;
    Semaphore		lock;
  }
INTERP;

typedef struct
  {
    NODE 		node;
    char *		name;
    char *		text;
  }
MACRO;

PUBLIC INTERP *		newinterp(		DEBUG * );
PUBLIC void		reminterp(		INTERP * );
PUBLIC void		lockinterp(		INTERP * );
PUBLIC void		unlockinterp(		INTERP * );
PUBLIC void		alias(			INTERP *, char *, char * );
PUBLIC void		define(			INTERP *, char *, char * );
PUBLIC char *		getalias(		INTERP *, char * );
PUBLIC char *		getdefine(		INTERP *, char * );
PUBLIC void		unalias(		INTERP *, char * );
PUBLIC void		undefine(		INTERP *, char * );
PUBLIC void		listaliases(		INTERP *, DISPLAY * );
PUBLIC void		listdefines(		INTERP *, DISPLAY * );
PUBLIC void		pushchar(		INTERP *, char );
PUBLIC void		pushword(		INTERP *, char * );
PUBLIC void		pushcmd(		INTERP *, char * );
PUBLIC int		popchar(		INTERP * );
PUBLIC char *		popword(		INTERP *, char *, uword );


/*system.h**************************************************/

#define DebugTimeout OneSec

PUBLIC void receiver(	      DEBUG *);
PUBLIC void syscall(	      DEBUG *, void *, int, void *, int, void *);
PUBLIC void sysfree(	      DEBUG *, int);
PUBLIC void sysfreeall(	      DEBUG *);
PUBLIC void sysgo(	      DEBUG *, int);
PUBLIC void sysgoall(	      DEBUG *);
PUBLIC void sysgoto(	      DEBUG *, int, int, int);
PUBLIC void sysgotoall(	      DEBUG *, int, int);
PUBLIC void sysgotoframe(     DEBUG *, int, int);
PUBLIC void sysgotoframeall(  DEBUG *, int);
PUBLIC void syskill(	      DEBUG *, int);
PUBLIC void syskillall(	      DEBUG *);
PUBLIC void sysprofile(	      DEBUG *, int, int, int, int);
PUBLIC void sysstep(	      DEBUG *, int);
PUBLIC void sysstepall(	      DEBUG *);
PUBLIC void sysstop(	      DEBUG *, int);
PUBLIC void sysstopall(	      DEBUG *);
PUBLIC void systrace(	      DEBUG *, int, int, int, int);
PUBLIC void systraceall(      DEBUG *);
PUBLIC void sysaddbreak(      DEBUG *, int, int, int);
PUBLIC void sysrembreak(      DEBUG *, int, int);
PUBLIC void peekmem(	      DEBUG *, void *, void *, int);
PUBLIC void peekdata(	      DEBUG *, void *, int, int, int);
PUBLIC void peekstack(	      DEBUG *, void *, int, int, int, int);
PUBLIC void pokemem(	      DEBUG *, void *, void *, int);
PUBLIC void *locatedata(      DEBUG *, int, int);
PUBLIC void *locatestack(     DEBUG *, int, int, int);
#ifdef __C40
PUBLIC void *locateframe(     DEBUG *, int, int, int);
PUBLIC void *locateregister(  DEBUG *, int, int, int);
#endif
PUBLIC void sysaddwatch(      DEBUG *, void *, int);
PUBLIC void sysremwatch(      DEBUG *, void *, int);
PUBLIC int syswhere(	      DEBUG *, int, int, LOCATION *);
PUBLIC void systimeout(	      DEBUG *, int);
PUBLIC void systimeoutall(    DEBUG *);

PUBLIC UBYTE peekbyte(	      DEBUG *, void *);
PUBLIC USHORT peekshort(      DEBUG *, void *);
PUBLIC UWORD peekword(	      DEBUG *, void *);


/*table.h***********************************************/


PUBLIC void initchain(CHAIN *);
PUBLIC void addlink(CHAIN *, LINK *);
PUBLIC void addtail(CHAIN *, LINK *);
PUBLIC void walkchain(CHAIN *, void (*)(), long);
PUBLIC LINK *searchchain(CHAIN *, int (*)(), long);
PUBLIC void inittable(TABLE );
PUBLIC SYMBOL *addsymbol(TABLE, char *);
PUBLIC SYMBOL *findsymbol(TABLE, char *);
PUBLIC void walktable(TABLE , void (*)(), long);
PUBLIC SYMBOL *searchtable(TABLE, int (*)(), long);


/*symbol.h**********************************************/


#define NAME_MAX 256

#define GetTypeEntry(t) ((ENTRY *)((word *)(t) - 2))

typedef USHORT LINENO;

typedef enum
{
  TI_Array, TI_Enum, TI_Float, TI_Function, TI_Integral, TI_Pointer, TI_ReUse,
  TI_Struct, TI_Ace, TI_Tag, TI_Typedef
} TYPEID;

typedef struct block
  {
    LINK 		link;
    struct block *	parent;
    CHAIN		blocklist;
    struct entry *	entry;
    struct module *	module;
    int			lines;
    LINENO *		linevec;
  }
BLOCK;

typedef union type *typeptr;

typedef struct reuse
  {
    TYPEID 	id;
    typeptr	type;
  }
REUSE;

typedef struct typename
  {
    TYPEID 	id;
    typeptr	type;
  }
TYPENAME;

typedef struct pointer
  {
    TYPEID 	id;
    typeptr	host;
  }
POINTER;

typedef struct array
  {
    TYPEID 	id;
    typeptr	host;
    int		size;
    int		first;
  }
ARRAY;

typedef struct structure
  {
    TYPEID 		id;
    struct entry *	tag;
    CHAIN		memberlist;
  }
STRUCTURE;

typedef struct function
  {
    TYPEID 	id;
    typeptr	host;
    CHAIN	paramlist;
  }
FUNCTION;

typedef struct enumeration
  {
    TYPEID 	id;
    CHAIN	constlist;
  }
ENUMERATION;

typedef struct basetype
  {
    TYPEID 	id;
    short	size;
    short	issigned;
  }
BASETYPE;

typedef union type
  {
    struct
      {
	TYPEID 	id;
	typeptr host;
      }
    generic;

    REUSE 	reuse;
    TYPENAME	typename;
    POINTER	pointer;
    ARRAY	array;
    STRUCTURE	structure;
    FUNCTION	function;
    ENUMERATION enumeration;
    BASETYPE	basetype;
  }
TYPE;

typedef struct 
  {
    LINK 	link;
    char *	name;
    CLASS	Class;
    TYPE *	type;
    BLOCK *	block;  /*** war als ifdef OLDCODE ***/
  }
TYPEDEF;

typedef struct
  {
    LINK 	link;
    char *	name;
    CLASS	Class;
    TYPE *	type;
    int		offset;
  }
MEMBER;

typedef struct
  {
    LINK 	link;
    char *	name;
    CLASS	Class;
    TYPE *	type;
    int		offset;
  }
ENUMCONST;

typedef struct
  {
    LINK 	link;
    char *	name;
    CLASS	Class;
    TYPE *	type;
    int		offset;
  }
PARAM;

typedef struct
  {
    LINK 	link;
    char *	name;
    CLASS	Class;
    TYPE *	type;
    int		offset;
    BLOCK *	block;
  }
LOCAL;

typedef struct entry
  {
    LINK 	link;
    char *	name;
    CLASS	Class;
    TYPE *	type;
    int		offset;
    BLOCK *	block;
  }
ENTRY;

PUBLIC void	add_params( 		TABLE, BLOCK *, TYPE * );
PUBLIC ENTRY *	declarevar(		TABLE, BLOCK *, char *, CLASS, TYPE *, int);
PUBLIC ENTRY *	declarelocal(		TABLE, BLOCK *, char *, TYPE *, int);
PUBLIC ENTRY *	declaretype(		TABLE, BLOCK *, char *, TYPE *);
PUBLIC ENTRY *	declaretag(		TABLE, BLOCK *, char *, TYPE *);
PUBLIC ENTRY *	declareparam(		TABLE, TYPE  *, char *, TYPE *, int);
PUBLIC ENTRY *	declareenum(		TABLE, TYPE  *, char *, TYPE *, int);
PUBLIC ENTRY *	declaremember(		TABLE, TYPE  *, char *, TYPE *, int);
PUBLIC ENTRY *	findvar(		TABLE, BLOCK *, char *);
PUBLIC ENTRY *	findtype(		TABLE, char *);
PUBLIC ENTRY *	findtypeid(		TABLE, BLOCK *, int, char *);
PUBLIC ENTRY *	findtag(		TABLE, char *);
PUBLIC ENTRY *	findmember(		TYPE *, char *);
PUBLIC ENTRY *	findenumconst(		TYPE *, int);
PUBLIC ENTRY *	whichentry(		BLOCK *, void *, char *);
PUBLIC ENTRY *	whichmember(		TYPE *, int);
PUBLIC TYPE *	newpointer(		TYPE *);
PUBLIC TYPE *	newarray(		TYPE *, int, int);
PUBLIC TYPE *	newstruct(		void);
PUBLIC TYPE *	newfunction(		TYPE *);
PUBLIC TYPE *	newenumeration(		void);
PUBLIC TYPE *	newtag(			ENTRY *);
PUBLIC TYPE *	newtypedef(		ENTRY *);
PUBLIC TYPE *	newbasetype(		TYPEID, int, BOOL);
PUBLIC TYPE *	newintegraltype(	int, BOOL);
PUBLIC TYPE *	newfloatingtype(	int);
PUBLIC TYPE *	reusetype(		TYPE *);
PUBLIC TYPE *	skipreuse(		TYPE *);
PUBLIC TYPE *	skiptypedef(		TYPE *);
PUBLIC TYPE *	hosttype(		TYPE *);
PUBLIC BLOCK *	newblock(		BLOCK *);
PUBLIC void	addline(		BLOCK *, int);
PUBLIC void	walkblock(		BLOCK *, void (*)(), long);
PUBLIC BLOCK *	searchblock(		BLOCK *, BOOL (*)(), long);
PUBLIC BLOCK *	findblock(		LOCATION);
PUBLIC ENTRY *	findfunction(		struct module *, int);
PUBLIC BOOL	validline(		LOCATION);
PUBLIC void	freetype(		TYPE *);
PUBLIC void	freeblock(		BLOCK *);
PUBLIC void	freesymbol(		SYMBOL *);
PUBLIC void	freeentry(		ENTRY *);
PUBLIC void	putblock(		BLOCK *, FILE *);
PUBLIC void	putentry(		ENTRY *, FILE *);
PUBLIC void	putsymbol(		SYMBOL *, FILE *);


/*expr.h***********************************************/

#define OCTAL   8
#define DECIMAL 10
#define HEX     16

#define UPTOCOMMA 3
#define PASTCOMMA 2

typedef enum
{
  T_Auto,       T_Break,     T_Case,       T_Char,        T_Const,    T_Continue, T_Default,  T_Do,
  T_Double,     T_Else,      T_Enum,       T_Extern,      T_Float,    T_For,      T_Goto,     T_If,
  T_Int,	T_Long,      T_Register,   T_Return,      T_Short,    T_Signed,   T_Sizeof,   T_STATIC,
  T_Struct,     T_Switch,    T_Typedef,    T_Union,       T_Unsigned, T_Void,     T_Volatile, T_While,
  T_Identifier, T_Constant,  T_String,     T_LBracket,    T_RBracket, T_LParen,   T_RParen,   T_Dot,
  T_Arrow,      T_PlusPlus,  T_MinusMinus, T_BitAnd,      T_Times,    T_Plus,     T_Minus,    T_BitNot,
  T_LogNot,     T_Divide,    T_Remainder,  T_LShift,      T_RShift,   T_LT,       T_GT,       T_LE,
  T_GE,		T_EQ,	     T_NE,	   T_BitXOr,	  T_BitOr,    T_LogAnd,   T_LogOr,    T_Conditional,
  T_Assign,	T_TimesEq,   T_DivideEq,   T_RemainderEq, T_PlusEq,   T_MinusEq,  T_LShiftEq, T_RShiftEq,
  T_BitAndEq,   T_BitXOrEq,  T_BitOrEq,    T_Comma,	  T_LBrace,   T_RBrace,   T_Colon,    T_Semicolon,
  T_Ellipsis,   T_Subscript, T_Call,       T_Function,    T_Pointer,  T_Array,    T_UPlus,    T_UMinus,
  T_Address,    T_Indirect,  T_PostInc,    T_PostDec,     T_Convert,  T_Cast,     T_Error,    T_Member,
  T_Parameter,  T_Tag,       T_End,        T_List
} TOKEN;

typedef union
  {
    int 	integral;
    float	floating4;
    double	floating8;
  }
VALUE;

typedef union expr * exprptr;

typedef union expr
  {
    struct
      {
	TOKEN 		op;
	typeptr		type;
	exprptr		expr1;
	exprptr		expr2;
	exprptr		expr3;
      }
    generic;

    struct
      {
	TOKEN 		token;
	typeptr		type;
	struct entry *	entry;
      }
    identifier;
    
    struct
      {
	TOKEN 		token;
	typeptr		type;
	VALUE		value;
      }
    constant;
    
    struct
      {
	TOKEN 		token;
	typeptr		type;
	char *		value;
      }
    string;
    
    struct
      {
	TOKEN 		op;
	typeptr		type;
	exprptr		expr;
      }
    cast;
  }
EXPR;

typedef union
  {
    long 	number;
    EXPR *	expr;
    ENTRY *	entry;
  }
VAL;

typedef struct
  {
    TOKEN 	token;
    VAL		value;
  }
LEXICON;

extern char *	tokennames[];

PUBLIC BOOL	evalcond(	struct eval *, char *, BLOCK * );
PUBLIC EXPR *	parseexpr(	struct eval *, char *, BLOCK * );
PUBLIC void *	evaladdr(	struct eval *, EXPR * );
PUBLIC void	putexpr(	EXPR *, FILE * );


/*eval.h*******************************************************/

#define STACK_SIZE 1000
#define ALIGN(l) ((l + 3) & ~3)

typedef struct eval
  {
    int 		ch;
    char *		chptr;
    TOKEN		token;
    VAL			tokenvalue;
    int			tokenindex;
    char		tokenbuffer[ NAME_MAX + 1 ];
    BOOL		backtracked;		  	/* ACE: must initialise to FALSE */
    LEXICON		nextlexicon;
    LEXICON		prevlexicon;
    byte		stack[ STACK_SIZE ];
    byte *		stackptr;
    DEBUG *		debug;
    struct block *	block;
  }
EVAL;

PUBLIC void 	genexpr(	EVAL *, EXPR * );
PUBLIC void	genvoidexpr(	EVAL *, EXPR * );
PUBLIC void	genaddr(	EVAL *, EXPR * );
PUBLIC void	genparam(	EVAL *, PARAM *, int );
PUBLIC void	lvalue(		EVAL *, ENTRY * );
PUBLIC void	rvalue(		EVAL *, ENTRY * );


/*cmd.h*******************************************/


#define WordMax		511
#define ArgMax		80


#define CommandLevel	1
#define LoopLevel	2
#define BreakLevel	3
#define ErrorLevel	4
#define FileLevel	5
#define TopLevel	6

PUBLIC void	_do(		DEBUG *, char * );
PUBLIC void	interp(		DEBUG * );
PUBLIC int	cmdexec(	DEBUG *, char * );
PUBLIC void	cmderr(		DEBUG *, char *, ... );
PUBLIC void	cmdmsg(		DEBUG *, char *, ... );
PUBLIC void	cmdjmp(		DEBUG *, int );


/*util.h*************************************************/


#define TabSize 	8

PUBLIC char *	strdup(		char * );
PUBLIC void	tabexp(		char *, char *, int );
PUBLIC BOOL	optequ(		char *, char * );
PUBLIC char *	getvar(		char **, char * );
PUBLIC LOCATION getloc(		DEBUG *, char * );
PUBLIC char *	formloc(	char *, LOCATION );
PUBLIC char *	basename(	char * );
PUBLIC int	getkeyname(	char * );
PUBLIC void	formkeyname(	char *, int );
PUBLIC void	formvarloc(	char *, struct entry * );
PUBLIC char *	formword(	char ** );
PUBLIC void	bigbuf(		FILE * );
PUBLIC FILE *	my_fopen(	const char *, const char * );
PUBLIC FILE *	my_fdopen(	int, char * );


/*info.h**************************************************/


PUBLIC BLOCK *	loadinfo(	DEBUG *, struct module * );


/*added definitions and declarations ***********************/


PUBLIC int	sopen(		Stream * stream);
PUBLIC void	_cd(		DEBUG *  debug, char *path);
#ifdef OLDCODE
PUBLIC void 	_dump(		DEBUG * );
#else
PUBLIC void 	_dump(		DEBUG * debug, char *exprstr);
#endif
PUBLIC void 	_edit(		DEBUG * debug, LOCATION loc);
PUBLIC void	_help(		DEBUG * debug, char *topic);
PUBLIC void	_make(		DEBUG * debug);
PUBLIC void	_menu(		DEBUG * debug, char *title, int cmd, char *cmdv[], char *labv[]);
PUBLIC void	_print(		DEBUG * debug, char *exprstr, FORMAT format, int chase);
PUBLIC void	_pwd(		DEBUG * debug);
PUBLIC void	_shell(		DEBUG * debug, char *cmdline);
PUBLIC void	_watchpoint(	DEBUG * debug, char *eprstr, char *docmd, FORMAT format, BOOL silent);
PUBLIC void	_whatis(	DEBUG * debug, char *exprstr);

/* -- crf : 07/08/91 - "all" not used */
PUBLIC void	_where(		DEBUG * debug) ; /* , BOOL all); */

PUBLIC void	_whereis(	DEBUG * debug, char *name);
PUBLIC void	_which(		DEBUG * debug, char *name);
PUBLIC void	putvalue(	DEBUG * debug, TYPE *type, FORMAT format, int inden, int chase);
PUBLIC TYPE *	typeofexpr(	EXPR *  expr);
PUBLIC void	checkstack(	EVAL *  eval);
PUBLIC void	freeexpr(	EXPR *  expr);
PUBLIC int	sizeofexpr(	EXPR *  expr);
PUBLIC void	puttype(	TYPE *  type, FILE *file);
PUBLIC void	notifywatchpoints(DEBUG * debug, void *addr, int size, word scope);
PUBLIC int	pop(		EVAL * eval);
PUBLIC byte *	ppop(		EVAL * eval);
PUBLIC BOOL	isintegral(	TYPE * type);
PUBLIC BOOL	isfloat(	TYPE * type);
PUBLIC int	sizeoftype(	TYPE * type);
PUBLIC EXPR *	mkintconst(	EVAL * eval, int value);
PUBLIC EXPR *	mkstring(	EVAL * eval, char *value);
PUBLIC EXPR *	mkfloatconst(	EVAL * eval, double value);
PUBLIC void	recover(	EVAL * eval);
PUBLIC EXPR *	mkcond(		EVAL * eval, EXPR *expr1, EXPR *expr2, EXPR *expr3);
PUBLIC EXPR *	mkbinary(	EVAL * eval, TOKEN op, EXPR *expr1, EXPR *expr2);
PUBLIC EXPR *	mkunary(	EVAL * eval, TOKEN op, EXPR *expr);

#ifdef OLDCODE
PUBLIC EXPR *	mkexprlist(	EVAL * eval, EXPR *expr1, EXPR *expr2);
#endif
/*
-- crf : 18/08/91 - "eval" declared, not used ...
*/
PUBLIC EXPR *	mkexprlist(	EXPR * expr1, EXPR *expr2);

PUBLIC EXPR *	mkfieldref(	EVAL * eval, TOKEN op, EXPR *expr, char *name);
PUBLIC EXPR *	mkidentifier(	ENTRY * entry);
PUBLIC EXPR *	mkcast(		EVAL * eval, TYPE *type, EXPR *expr);
PUBLIC int	strideofexpr(	EXPR * expr);
PUBLIC BOOL	isfunction(	TYPE * type);
PUBLIC void	semerr(		EVAL * eval, char *format, ...);
PUBLIC BOOL	isunsigned(	TYPE * type);
PUBLIC BOOL	isstring(	TYPE * type);
PUBLIC BOOL	isShortArray(	TYPE * type );
PUBLIC BOOL	isaggregate(	TYPE * type);
PUBLIC char *	expandcmd(	INTERP * interp, char *buf, char *cmd, uword bufsiz);
PUBLIC void	tidyup(		void );
PUBLIC void	initmem(	BOOL memchecking);
PUBLIC EVAL *	neweval(	DEBUG * debug);
PUBLIC TABLE *	newtable(	void );
PUBLIC BOOL	readcode(	DEBUG * debug);
PUBLIC void	remeval(	EVAL * eval);
PUBLIC void	putwatchpoint(	WATCHPOINT * watchpoint, DEBUG *debug);

/**** new funktions *******/
PUBLIC void	tagerr(		EVAL *eval, char *format, ...);/* CR: nearly the same as semerr */

#ifdef PARSYTEC
PUBLIC int 	actualisewatchpoints(DEBUG *, word , int , word );/* CR: for notifywp */
#endif

/* -- crf : 05/08/91 - additional parameter "del_watch_id" */
PUBLIC int	actualisewatchpoints(DEBUG *, word , int , word , UWORD *);

PUBLIC WATCHPOINT *	addwatchpoint(DEBUG *, char *, void *, int, char *, FORMAT, BOOL, BLOCK *);

/* -- crf : 24/07/91 - Bug 708 */
PUBLIC int	num_watch_elements (WATCHPOINT *, DEBUG *) ;

/********** new defines **************************/
#define NEW_Watchpoint		2
#define OLD_Watchpoint		3


/********* new variables ************************/

/*
-- crf : 26/09/91 - Bug 677 
-- concerning running client programs from a directory other than that where
-- the sources are located - changes made to :
--   1. loadsource() (source.c) - loads source files
--   2. loadinfo() (info.c) - loads .dbg files
--   3. wrefill() (display.c) - tells the user if sources can't be located
-- (if the required files cannot be located in the current directory, these
-- routines look in the directory indicated by the environment variable
-- defined below).
*/
#define DBGSRC_VARNAME "DBGSRC"
