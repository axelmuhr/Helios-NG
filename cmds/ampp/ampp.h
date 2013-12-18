/* RcsId: $Id: ampp.h,v 1.6 1993/12/09 17:43:08 nickc Exp $ Copyright (C) Perihelion Software Ltd.	*/
#include <stdio.h>
#ifdef __HELIOS
# include <helios.h>
# include <queue.h>
#else
# include "ttypes.h"
# include "queue.h"
#endif
#include <setjmp.h>
#ifdef __STDC__
# include <stdlib.h>
# include <stdarg.h>
#endif

#ifdef MWC
# define VERBOSE 1
#endif
#ifdef IBMPC
# define VERBOSE 1
#endif
#ifdef __HELIOS
# define VERBOSE 1
#endif

extern FILE *outfd;
#define debug(zz) dbg("zz = %d[%8x]",zz,zz);

#ifdef New
#undef New
#endif

#ifdef MWC
WORD *lmalloc();
#define New(_type) (_type *)lmalloc((WORD)sizeof(_type))
#define locase(c) (isupper(c)?_tolower(c):c)
#else
#ifdef IBMPC
WORD *malloc();
#define New(_type) (_type *)malloc(sizeof(_type))
#define locase(c) (isupper(c)?tolower(c):c)
#endif
#define New(_type) (_type *)alloc((WORD)sizeof(_type))
#define locase(c) (isupper(c)?tolower(c):c)
#endif

extern WORD traceflags;

#define db_lex          0x01
#define db_parse        0x02
#define db_builtin      0x04
#define db_sym          0x08
#define db_putback	0x10
#define db_fileop	0x20

#define DEFAULT_INCLUDE_PATH	"/helios/include/ampp/"
#define MAX_INCLUDE_PATHS	16

#define HASHSIZE 257

struct Symbol {
   struct Node    node;
   struct Def     *definition;
   char           name[32];
};


struct Def {
   struct Def     *prev;
   INT            type;
   struct Symbol  *sym;
   union {
      void         (*builtin)();
      struct Macro *macro;
      INT          value;
   } Value;
};

struct Macro {
   INT            nargs;
   struct List    arglist;
   struct List    def;
};

struct Arg {
   struct Node    node;
   INT            quoted;
   struct Symbol  *sym;
};

#define charbuf_max  256

struct Charbuf {
   struct Node    node;
   INT            size;
   BYTE           text[charbuf_max];
};


#define err_memory 1
#define err_maxdef 2
#define err_infile 3
#define err_outfile 4


/* lexical types */

#define s_builtin       1
#define s_macro         2
#define s_var           3
#define s_hexvar	4
#define s_token         10  /* type < token are in symbol table */
                            /* types >= token are not */
#define s_lbra          11
#define s_rbra          12
#define s_space         13
#define s_tab           14
#define s_nl            15
#define s_other         16
#define s_comma         17
#define s_concat        18
#define s_eof           19
#define s_quote         20

#define s_plus          21
#define s_minus         22
#define s_mul           23
#define s_div           24
#define s_mod           25

#define s_number        100	/* numerics are all s_number+radix */

/* special characters */

#define c_lbra          '['
#define c_rbra          ']'
#define c_space         ' '
#define c_tab           '\t'
#define c_nl            '\n'
#define c_comma         ','
#define c_quote         '\''
#define c_plus          '+'
#define c_minus         '-'
#define c_mul           '*'
#define c_div           '/'
#define c_concat        '$'
#define c_hash          '#'
#define c_dot           '.'
#define c_backslash	'\\'

#define rdch() ((pbspos >= pbsbase) ? get_char( infd ) : *pbspos++)

extern FILE *		verfd;
extern jmp_buf		error_level;
extern BYTE *		incpaths[ MAX_INCLUDE_PATHS ];
extern BYTE *		infile;
extern BYTE *		outfile;
extern INT		traceflags;
extern INT		symb;
extern BYTE		token[ 128 ];
extern struct Def *	tokdef;
extern struct Symbol *	toksym;
extern INT		toksize;
extern INT		tokval;
extern jmp_buf		reclev;
extern FILE *		infd;
extern INT		in_line;
extern BYTE *		pbstack;
extern BYTE *		pbspos;
extern BYTE *		pbsbase;

#ifdef __STDC__
extern void 		error(BYTE *, ...);
extern void		report(BYTE  *,  ...);
extern void		warn(BYTE *,  ...);
extern void		_trace(char *, ...);
extern void		recover(BYTE *str,...);
#else
extern 			error();
extern			warn();
extern			_trace();
extern			report();
extern void		recover();
#endif

#ifdef __STDC__
extern INT		get_char( FILE * );
#else
extern INT		get_char();
#endif

/* @@@ the following prototypes should also be converted to STDC */
extern			main( int, char ** );
extern INT		max( INT, INT );
extern char *		alloc( INT );
extern void		initbuiltin( void );
extern void		builtin( void );
extern void		getdef( struct List * );
extern void 		initcs( void );
extern void		addch( struct List *, BYTE );
extern INT		popch( struct List * );
extern void		freebuf( struct List * );
extern void		adddef( INT, struct Symbol *, INT );
extern void		unwind( struct Symbol * );
extern void		nextsym( void );
extern void		skipspaces( void );
extern void		pbskipspaces( void );
extern void		macro( void );
extern WORD		parse( INT );
extern struct List *	outbuf;
extern FILE *		outfd;
extern void		wrch( UBYTE );
extern void		wrstr( BYTE * );
extern void		wrnum( INT );
extern void		wrhex( INT );
extern void		initpb( void );
extern void		pbchar( UBYTE );
extern void		pbstr( BYTE * );
extern BYTE *		savepb( void );
extern void		setpb( BYTE * );
extern INT		pbrdch( void );
extern void		pbdef( struct List * );
extern void		pbnum( INT );
extern void		pbhex( INT );
extern void		initsym( void );
extern struct Symbol *	lookup( BYTE * );
extern struct Symbol *	insert( BYTE * );
extern INT		eqs( BYTE *, BYTE * );
extern void		InitList( struct List * );
extern void		AddTail( struct List *, struct Node * );
extern void		AddHead( struct List *, struct Node * );
extern struct Node *	RemHead( struct List * );
extern struct Node *	RemTail( struct List * );
extern void		PreInsert( struct Node *, struct Node * );
extern void		PostInsert( struct Node *, struct Node * );
extern struct Node *	Remove( struct Node * );

