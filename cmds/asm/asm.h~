
#include <stdio.h>
#include <setjmp.h>
#ifdef __STDC__
#include <helios.h>
#include <string.h>
#else
#include "ttypes.h"
#endif
#include <queue.h>

#ifdef __STDC__
#define ellipsis ...
#else
#define ellipsis
#endif

#ifdef MWC
#define ATARI 1
#endif

#ifdef __DOS386
#include <stdarg.h>
#include <time.h>
#define LITTLE_ENDIAN
#define VERBOSE 1
#undef min
#undef max
#endif

#ifdef IBMPC
#define LITTLE_ENDIAN
#define VERBOSE 1
#endif

#ifdef __TRAN
#define LITTLE_ENDIAN
#ifndef __DOS386
#define VERBOSE 0
#endif
#endif

extern FILE *outfd;
extern FILE *verfd;

#ifdef print
#undef print
#endif
#define print(zz) dbg("zz = %d[%8x]",zz,zz);

#ifdef New
#undef New
#endif
#define New(_type) (_type *)alloc((WORD)sizeof(_type))


#ifdef MWC
#define locase(c) (isupper(c)?_tolower(c):c)
#else
#define locase(c) (isupper(c)?tolower(c):c)
#endif


/* flags for trace */

extern WORD traceflags;
#define db_lex       0x001
#define db_syn       0x002
#define db_growcode  0x004
#define db_gencode   0x008
#define db_genimage  0x010
#define db_expr      0x020
#define db_eval      0x040
#define db_preasm    0x080
#define db_vm        0x100
#define db_kludge    0x200
#define db_files     0x400
#define db_mem	     0x800
#define db_sym	     0x1000
#define db_modules   0x2000
#define db_refs	     0x4000

/* warning flags */
extern WORD warnflags;
#define warn_unref	0x0001
#define warn_undef	0x0002

#ifdef VM

#include "vm.h"

#ifdef __STDC__
extern VMRef VMNew(int size);
#else
extern VMRef VMNew();
#endif

#else

typedef long	VMRef;

#define VMlock(x)

#define VMunlock(x)

#define VMAddr(type,x) ((type *)(x))

#define VMNew(_type) (_type *)alloc((WORD)sizeof(_type))

#define VMDirty(x)

#define VMalloc alloc

#endif


/* general purpose union */

typedef union Value
  {
    WORD	w;
    VMRef	v;
    UBYTE	b[ 4 ];
    char *	c;
  }
Value;

/* keyword dictionary */

#define GLOBAL_HASHSIZE 	541
#define LOCAL_HASHSIZE 		31

struct keyentry
  {
    INT     next;           /* offset of next entry in chain 	*/
    INT     type;           /* type of entry                 	*/
    INT     value;          /* value of instruction, or subtype */
    char    name[ 1 ];      /* larger in practice           	*/
  };

/* Modules */

typedef struct STEntry
  {
    VMRef	head;
    int		entries;
  }
STEntry;

typedef struct asm_Module
  {
    VMRef	next;           	/* link in modlist              */
    VMRef	start;			/* code start and end...	*/
    VMRef	end;
    char *	file;			/* ptr to file name		*/
    VMRef	refs;			/* list of references		*/
    WORD	id;			/* module table slot            */
    WORD	linked;			/* true if added to link	*/
    STEntry	symtab[ LOCAL_HASHSIZE ];/* module symbol table		*/
  }
asm_Module;

typedef struct Ref
  {
    VMRef	next;			/* next ref in list	*/
    VMRef	sym;			/* referenced symbol	*/
  }
Ref;

/* Symbol table */

#define UINT	unsigned int
#define NEW_REF

typedef struct Symbol
  {
    VMRef	next;		/* next symbol in hash chain	*/
    VMRef	prev;		/* prev symbol in hash chain	*/
    INT		type;           /* symbol type                  */
#ifdef NEVER
    WORD	global;		/* true if global, else false	*/
    WORD	referenced;	/* true if referenced		*/
#else
    UINT	global:1,	/* true if global, else false	*/
    		referenced:1;	/* true if referenced		*/
#endif
#ifdef NEW_REF
    VMRef	fwd_ref;	/* link into forward ref chain	*/
#endif
    VMRef	module;		/* defining module		*/
    Value	def;            /* its definition               */
    char	name[ 1 ];      /* symbol (actually variable)   */
  }
Symbol;

/* expression tree nodes */

typedef struct unode
  {
    Value	expr;  		/* expression                   */
    UBYTE       ntype; 		/* node type                    */
    UBYTE       etype; 		/* expression type              */
  }
Unode;

typedef struct bnode
  {
    Value     	lexp;   	/* left expression              */
    Value     	rexp;   	/* right expression             */
    UBYTE       ntype;  	/* node type                    */
    UBYTE       ltype;  	/* left expression type         */
    UBYTE       rtype;  	/* right expression type        */
  }
Bnode;

/* code structures      */

typedef struct code
  {
    UBYTE       type;   /* see below                            */
    UBYTE       size;   /* number of bytes occupied in code     */
    UBYTE       vtype;  /* type of value                        */
    UBYTE       flags;	/* some flag bits			*/
    WORD        loc;    /* current code location                */
    Value	value;
#ifdef LINENO
    WORD        line;   /* source line which generated this code*/
#endif
  }
Code;

#define codeflags_fixed		1	/* size set by size directive	*/

#define maxtok 1024		/* max token length			*/

/* Transputer op codes  */

#define f_pfix          0x20l
#define f_nfix          0x60l

#define DefaultDataSize    4l          /* allows 512k addressability */

#define i_program       0x12345678L

#define i_helios        'h'
#define i_csc           'c'

/* lexical types        */

/* 0..15 - transputer primaries */

#define s_direct        0x80
#define s_oper          0x90
#define s_oper1         (s_oper+1)
#define s_oper2         (s_oper+2)
#define s_oper3         (s_oper+3)
#define s_oper4         (s_oper+4)

#define s_unbound      0xa0        /* unknown symbol type  */

/* the following values are chosen carefully */
/* their ordering relative to eachother and to s_number is important */

#define s_modnum       0xa1


#define s_codesymb     0xa6
#define s_coderef      0xa7
#define s_datasymb     0xa8
#define s_datadone     0xa9
#define s_commsymb     0xaa
#define s_commdone     0xab

#define islabel(x) ((s_codesymb<=(x))&&((x)<=s_commdone))
#define deftype(x) ((x)&0xfel)

#define s_directive     0xae
#define s_labdef        0xaF        /* from lex only        */
#define s_token         0xb0

/* types < labdef are in symbol table	*/
/* types >= token are not		*/

#define s_lbra          0xb1
#define s_rbra          0xb2
#define s_space         0xb3
#define s_tab           0xb4
#define s_nl            0xb5
#define s_other         0xb6
#define s_comma         0xb7
#define s_number        0xb8
#define s_eof           0xb9
#define s_quote         0xba
#define s_plus          0xbb
#define s_minus         0xbc
#define s_mul           0xbd
#define s_div           0xbe
#define s_colon         0xbf
#define s_rem           0xc0
#define s_dot           0xc1
#define s_shl           0xc2
#define s_shr           0xc3
#define s_and           0xc4
#define s_or            0xc5
#define s_xor           0xc6
#define s_not           0xc7
#define s_expr          0xc8
#define s_string        0xc9
#define s_monadic       0xcb
#define s_semic         0xcc
#define s_at            0xcd
#define s_cr		0xce

/* code and other symbols */
#define s_label         0xd2
#define s_code          0xd5
#define s_literal       0xd6
#define s_bss           0xd8
#define s_end           0xd9
#define s_newfile	0xda
#define s_newseg	0xdb

/* directives */
#define s_firstdir      0xe0
#define s_blkb          0xe1
#define s_blkw          0xe2
#define s_init          0xe3
#define s_align         0xe4
#define s_word          0xe5
#define s_byte          0xe6
#define s_module        0xe7
#define s_global        0xe8
#define s_data          0xe9
#define s_common	0xea
#define s_size		0xeb
#define s_ref		0xec

/* special characters */

#define c_lbra          '('
#define c_rbra          ')'
#define c_space         ' '
#define c_tab           '\t'
#define c_nl            '\n'
#define c_comma         ','
#define c_quote         '\"'
#define c_plus          '+'
#define c_minus         '-'
#define c_mul           '*'
#define c_div           '/'
#define c_or            '|'
#define c_hash          '#'
#define c_dot           '.'
#define c_and           '&'
#define c_xor           '^'
#define c_shl           '<'
#define c_shr           '>'
#define c_rem           '%'
#define c_not           '~'
#define c_colon         ':'
#define c_backslash     '\\'
#define c_semic         ';'
#define c_at            '@'
#define c_cr		'\r'
#define c_bell		07
#define c_bs		8
#define c_vt		11
#define c_ff		12

#define TargetBytesPerWord 4


extern FILE *	infd;
extern FILE *	outfd;
extern FILE *	verfd;
extern jmp_buf 	error_level;
extern WORD 	errors;
extern WORD 	traceflags;
extern WORD 	preasm;
extern WORD 	hexopt;
extern WORD 	defopt;
extern WORD 	fastopt;
extern WORD 	verbose;
extern WORD 	inlib;
extern FILE *	deffd;
extern WORD 	vmpagesize;
extern WORD 	instr_size;
extern BYTE 	infile[];
extern char *	curfile;
extern WORD 	waste;

#ifdef __DOS386
extern int 	main(int argc, char **argv);
extern void 	error(BYTE *str, ...);
extern void 	warn(BYTE *str, ...);
extern void 	report(BYTE *str, ...);
extern void 	_trace(BYTE *str, ...);
extern void 	recover(BYTE *str, ...);
extern WORD 	min(WORD a, WORD b);
extern WORD 	max(WORD a, WORD b);
extern WORD 	pfsize(WORD n);
extern void 	encode(WORD op, WORD arg, void (* pbytefn)() );
extern void 	initcode(void);
extern void 	gencode(UBYTE *code, WORD size);
extern void 	genbyte(UWORD value);
extern void 	gendirect(UWORD op, UWORD vtype, WORD value);
extern void 	copycode(void);
extern VMRef 	deflabel(void);
extern void 	genglobal(VMRef label);
extern void 	genref(VMRef label);
extern void 	genlabdef(VMRef label);
extern void 	gendata(VMRef label, WORD size);
extern void 	gencommon(VMRef label, WORD size);
extern void 	geninit(void);
extern void 	genmodule(WORD mod);
extern void 	genalign(void);
extern void 	genword(WORD vtype, WORD value);
extern void 	genblkw(WORD type, WORD size);
extern void 	genblkb(WORD type, WORD size);
extern void 	genend(void);
extern void 	gensize(int size);
extern void 	newfile(void);
extern WORD 	asize(WORD value);
extern void 	gencsc(void);
extern void 	genimage(void);
extern void 	growdata(void);
extern void 	growcode(void);
extern WORD 	eval(WORD type, WORD earg, WORD loc);
extern WORD 	dodyadic(WORD op, WORD lexp, WORD rexp);
extern WORD 	domonadic(WORD op, WORD value);
extern void 	nextsym(void);
extern UBYTE *	alloc(long n);
extern void 	initmem(void);
extern VMRef 	unary(WORD ntype, WORD etype, WORD expr);
extern VMRef 	binary(WORD ntype, WORD ltype, WORD rtype, WORD lexp, WORD rexp);
extern VMRef 	newcode(WORD type, WORD size, WORD vtype, WORD loc, WORD value);
extern VMRef 	codeptr(void);
extern void 	setmodules(void);
extern VMRef 	startmodule(WORD mod);
extern void 	endmodule(void);
extern void 	refsymbol_def(VMRef v);
extern void 	refsymbol_nondef(VMRef v);
extern void 	modstats(void);
extern void 	parsepreasm(WORD ch);
extern void 	genpreasm(void);
extern void 	initsym(void);
extern VMRef 	lookup(BYTE *name);
extern VMRef 	insert(BYTE *name, WORD local);
extern void 	movesym(VMRef sym);
extern void 	cps(void);
extern void 	initasm(void);
extern void 	assemble(void);
extern void 	objed(FILE *fd, char *name, long stacksize, long heapsize);
extern void 	InitList(List *);
extern void 	PreInsert(Node *, Node *);
extern void 	PostInsert(Node *, Node *);
extern Node 	*Remove(Node *);
extern void 	AddHead(List *, Node *);
extern void 	AddTail(List *, Node *);
extern Node 	*RemHead(List *);
extern Node 	*RemTail(List *);
extern word 	WalkList(List *,WordFnPtr,...);
extern Node 	*SearchList(List *,WordFnPtr,...);

extern time_t 	time(time_t *tloc);
extern void 	initmodules(void);
extern void 	show_unref(void);
extern void 	printtab(STEntry *tab);

#else /* !__DOS386 */

extern int 	main(ellipsis);
extern void 	error(ellipsis);
extern void 	warn(ellipsis);
extern void 	report(ellipsis);
extern void 	_trace(ellipsis);
extern void 	recover(ellipsis);
extern WORD 	min(ellipsis);
extern WORD 	max(ellipsis);
extern WORD 	pfsize(ellipsis);
extern void 	encode(ellipsis);
extern void 	initcode(ellipsis);
extern void 	gencode(ellipsis);
extern void 	genbyte(ellipsis);
extern void 	gendirect(ellipsis);
extern void 	copycode(ellipsis);
extern VMRef 	deflabel(ellipsis);
extern void 	genglobal(ellipsis);
extern void 	genref(ellipsis);
extern void 	genlabdef(ellipsis);
extern void 	gendata(ellipsis);
extern void 	gencommon(ellipsis);
extern void 	geninit(ellipsis);
extern void 	genmodule(ellipsis);
extern void 	genalign(ellipsis);
extern void 	genword(ellipsis);
extern void 	genblkw(ellipsis);
extern void 	genblkb(ellipsis);
extern void 	genend(ellipsis);
extern void 	gensize(ellipsis);
extern void 	newfile(ellipsis);
extern WORD 	asize(ellipsis);
extern void 	gencsc(ellipsis);
extern void 	genimage(ellipsis);
extern void 	growdata(ellipsis);
extern void 	growcode(ellipsis);
extern WORD 	eval(ellipsis);
extern WORD 	dodyadic(ellipsis);
extern WORD 	domonadic(ellipsis);
extern void 	nextsym(ellipsis);
extern UBYTE *	alloc(ellipsis);
extern void 	initmem(ellipsis);
extern UBYTE *	alloc(ellipsis);
extern VMRef 	unary(ellipsis);
extern VMRef 	binary(ellipsis);
extern VMRef 	newcode(ellipsis);
extern VMRef 	codeptr(ellipsis);
extern void 	setmodules(ellipsis);
extern VMRef 	startmodule(ellipsis);
extern void 	endmodule(ellipsis);
extern void 	refsymbol_def(ellipsis);
extern void 	refsymbol_nondef(ellipsis);
extern void 	modstats(ellipsis);
extern void 	parsepreasm(ellipsis);
extern void 	genpreasm(ellipsis);
extern void 	initsym(ellipsis);
extern VMRef 	lookup(ellipsis);
extern VMRef 	insert(ellipsis);
extern void 	movesym(ellipsis);
extern void 	cps(ellipsis);
extern void 	initasm(ellipsis);
extern void 	assemble(ellipsis);
extern void 	objed(ellipsis);
extern void 	InitList(ellipsis);
extern void 	AddTail(ellipsis);
extern void 	AddHead(ellipsis);
extern Node *	RemHead(ellipsis);
extern Node *	RemTail(ellipsis);

/* extern int 	PreInsert(ellipsis);
extern int 	PostInsert(ellipsis); */

extern Node *	Remove(ellipsis);

extern int 	time(ellipsis);
extern void 	initmodules(ellipsis);
extern void 	show_unref(ellipsis);
extern void 	printtab(ellipsis);
extern void 	report(ellipsis);

#endif /* __DOS386 */

extern UBYTE *	codevec;
extern WORD 	codepos;
extern WORD 	simPc;
extern Code *	code;
extern WORD 	iteration;
extern WORD 	oldloc;
extern INT 	symb;
extern BYTE 	token[ maxtok + 16 ];
extern Symbol *	toksym;
extern VMRef 	toksymref;
extern struct keyentry *	tokkey;
extern INT 	toksize;
extern Value 	tokval;
extern WORD 	tempval;
extern UBYTE 	keytab[];
extern WORD 	dicttab[];
extern WORD 	codesize;
extern WORD 	heapsize;
extern STEntry 	Symtab[ GLOBAL_HASHSIZE ];
extern WORD 	symsize;
extern WORD 	exprsize;
extern WORD 	codeflags;
extern List 	modlist;
extern VMRef 	module0;
extern VMRef 	curmod;
extern VMRef 	firstmodule;
extern VMRef 	tailmodule;
extern word 	nmodules;
extern WORD 	lineno;
extern WORD 	etype;
extern WORD filepos;

#ifdef RDCH_FN
extern WORD asm_rdch(ellipsis);
extern WORD unrdch(ellipsis);
#else
/* macro versions of asm_rdch & unrdch	*/
extern WORD uch;
extern WORD unreadready;
#define asm_rdch() (unreadready?(unreadready=FALSE,uch):getc(infd))
#define unrdch(c) (unreadready=TRUE,uch=c)
#endif

#define eqs(a,b) (strcmp(a,b)==0)
