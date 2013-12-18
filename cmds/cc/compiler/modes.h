/* modes.h:  Copyright (C) A. Mycroft and A.C. Norman */
/* version 0.29b */
/* $Id: modes.h,v 1.1 1990/09/13 17:11:11 nick Exp $ */

#ifndef _MODES_LOADED

#define _MODES_LOADED 1

/* C has no way to say "either a 't' or nothing" - i.e.
   union { int t; struct {}; } is illegal.  The following macro
   defines an optional field of a struct.  We #undef opt at the
   end of this file in case of clashes.
*/
#define opt
#define bool int

/* DUFF_ADDR is used to initialise pointers which may not be dereferenced. */
/* The address chosen traps on dereference on the machines we use.         */
/* No use is made of its value, save that is assumed to compare equal to   */
/* itself when used to initialise allocators.  It could functionally       */
/* equally well be NULL, or for the queasy, a pointer to one static byte.  */
#define DUFF_ADDR (void *)0xbadbadad

/* the following lines ease bootstrapping problems: "\v\f\r" respectively */
#define CHAR_VT 11
#define CHAR_FF 12
#define CHAR_CR 13

/* We put xx!=0 for switches that default to ON, and xx>0 for default off */

#define warn_implicit_fns       (pp_pragmavec['a'-'a'] != 0)
#define warn_implicit_casts     (pp_pragmavec['b'-'a'] != 0)
#define memory_access_checks    (pp_pragmavec['c'-'a'] > 0)
#define warn_deprecated         (pp_pragmavec['d'-'a'] != 0)
/* beware that #pragma -e is temporarily used for #error behaviour */
#define profile_option          (pp_pragmavec['p'-'a'] > 0)
#define full_profile_option     (pp_pragmavec['p'-'a'] > 1)
#define tweak_densemap          pp_pragmavec['r'-'a']
#define no_stack_checks         (pp_pragmavec['s'-'a'] > 0)
#define special_variad          pp_pragmavec['v'-'a']
/* see also cg.h for transputer-specific pragmas		*/

#define var_warn_implicit_fns       pp_pragmavec['a'-'a']
#define var_warn_implicit_casts     pp_pragmavec['b'-'a']
#define var_memory_access_checks    pp_pragmavec['c'-'a']
#define var_warn_deprecated         pp_pragmavec['d'-'a']
#define var_profile_option          pp_pragmavec['p'-'a']
#define var_no_stack_checks         pp_pragmavec['s'-'a']

/* new types are defined by structures... */

typedef struct List { struct List *cdr; int car; } List;
#define cdr_(p) ((p)->cdr)
#define car_(p) ((p)->car)

typedef int AEop;
#define SET_BITMAP int

typedef struct TypeExpr         /* make into a better union type */
{ AEop h0;
  struct TypeExpr *typearg;     /* or SET_BITMAP for s_typespec */
  struct Binder *typespecbind;  /* or TagBinder for s_typedefname */
  opt int minargs, maxargs, variad;
} TypeExpr;
#define typearg_(p) ((p)->typearg)
#define typespecmap_(p) (*(SET_BITMAP *)&(p)->typearg)
#define typespecbind_(p) ((p)->typespecbind)
#define typespectagbind_(p) ((TagBinder *)((p)->typespecbind))
#define typefnargs_(p)   ((FormTypeList *)((p)->typespecbind))
#define typefnargs1_(p)   ((DeclRhsList *)((p)->typespecbind))
#define typeptrmap_(p)   (*(SET_BITMAP *)&((p)->typespecbind))
#define typesubsize_(p)  (*(Expr **)&((p)->typespecbind))
/* the next routine for SEM.C ONLY */
#define EQtype_(t1,t2)   ((t1)->h0==(t2)->h0 && (t1)->typearg==(t2)->typearg \
                          && (t1)->typespecbind==(t2)->typespecbind)
/* for function types */
#define minargs_(p) ((p)->minargs)
#define maxargs_(p) ((p)->maxargs)

#define TypeSpec TypeExpr   /* used when known to be a s_typespec TypeExpr */

typedef struct Symstr {
  AEop h0;                /* keyword or s_identifier.  Must be offset 0 */
  struct Symstr *symchain;      /* linear list constituting hash bucket */
                                /* AM: the 5 overloading classes... */
/*  void  *sympp;                 / * preprocessor information            */
  struct LabBind *symlab;       /* status as a label                   */
  struct TagBinder *symtag;     /* structure, union, enumeration tag   */
  int /* ExtRef */    *symext; /* stolen by AM (was _symsel)          */
  struct Binder *symdata;       /* variable, function etc.             */
  char  symname[1];             /* name of this symbol                 */
} Symstr;
#define _symchain(sym) ((sym)->symchain)
#define _symtype(sym)  ((sym)->h0)
#define _sympp(sym)    ((sym)->sympp)
#define _symlab(sym)   ((sym)->symlab)
#define _symtag(sym)   ((sym)->symtag)
#define _symext(sym)   ((sym)->symext)
#define _symdata(sym)  ((sym)->symdata)
#define _symname(sym)  ((sym)->symname)

typedef struct StringSegList
{ struct StringSegList *strsegcdr;
  char *strsegbase;
  int  strseglen;
} StringSegList;

typedef struct String
{ AEop h0;
  StringSegList *strseg;
} String;

/* The order in this type is exploited for in xxxasm, xxxobj only */
typedef struct DbleBin {
#ifdef TARGET_HAS_OTHER_IEEE_ORDER
  int lsd,msd;     /* e.g. clipper, transputer */
#else
  int msd,lsd;     /* e.g. arm, (not really ieee) 370 */
#endif
} DbleBin;

typedef struct FloatBin {
  int val;
} FloatBin;

typedef struct FloatCon {
  AEop h0;
  SET_BITMAP floatlen;
  union { /* float f; double d; long double ld;  */
          int irep[2];     /* for xxxasm.c to print as int(s) */
          DbleBin db; FloatBin fb; } floatbin;
  char floatstr[1];   /* in practice longer */
} FloatCon;

/* a type for lex.c inspected by misc.c */
typedef struct SymInfo {
    AEop sym;
    union { char *s; int i; Symstr *sv; FloatCon *fc; } a1;
    union { int len, flag; } a2;
} SymInfo;

typedef struct Expr {
  AEop h0;
  TypeExpr *etype;
  struct Expr *arg1;       /* or Cmd for valof                    */
  opt struct Expr *arg2;   /* or int dotwoff, or ExprList *fnargs */
  opt struct Expr *arg3;   /* or int bitsize */
  opt int msboff;
} Expr;
#define type_(p) ((p)->etype)
/* N.B. leave arg1,2,3 as macros as will need soon to expand into  */
/*      p->field3.i (selecting union from struct)                  */
#define arg1_(p) ((p)->arg1)
#define arg2_(p) ((p)->arg2)
#define arg3_(p) ((p)->arg3)
#define exprfnargs_(p)  ((ExprList *)(p)->arg2)
#define exprdotoff_(p)  ((int)(p)->arg2)
#define exprbsize_(p)   ((int)(p)->arg3)
#define exprmsboff_(p)  ((p)->msboff)
#define expr1c_(p)      ((Cmd *)(p)->arg1)
/* for integer constants */
#define intval_(p)      ((int)(p)->arg1)
#define intorig_        arg2_

typedef struct FileLine { char *f; int l; } FileLine;

typedef struct Cmd {
  AEop h0;
  FileLine fileline;
  union { struct Cmd *c; Expr *e; } cmd1,cmd2,cmd3,cmd4;
} Cmd;

#define cmd1c_(x) ((x)->cmd1.c)
#define cmd2c_(x) ((x)->cmd2.c)
#define cmd3c_(x) ((x)->cmd3.c)
#define cmd4c_(x) ((x)->cmd4.c)
#define cmd1e_(x) ((x)->cmd1.e)
#define cmd2e_(x) ((x)->cmd2.e)
#define cmd3e_(x) ((x)->cmd3.e)
#define cmd4e_(x) ((x)->cmd4.e)

#define cmd1lab_(c)         ((LabBind *)cmd1c_(c))
#define switch_caselist_(c) cmd3c_(c)
#define switch_default_(c)  cmd4c_(c)
#define case_next_(c)       cmd3c_(c)
#define case_lab_(c)        ((LabelNumber *)cmd4c_(c))
#define cmdblk_bl_(c)       ((BindList *)cmd1c_(c))
#define cmdblk_cl_(c)       ((CmdList *)cmd2c_(c))


/* compatibility */

typedef List ExprList;
#define exprcar_(l) (*(Expr **)&(l)->car)   /* l-value for c.sem */
#define mkExprList syn_list2
typedef List CmdList;
#define cmdcar_(l)  ((Cmd *)(l)->car)

typedef struct LabelNumber          /* internal labels                   */
{
#ifdef TARGET_IS_XPUTER
  struct Block	*block;		/* pointer to defining block		*/
  struct Tcode *defn;		/* defining position in code, -1 if undef*/
  int refs;                     /* forw. ref. list                   */
#else
  struct block_head *block;
/* currently the 'defn' fields is unused by using the top bit of frefs   */
/* to cram in whether the label has been defined.                        */
/*  int defn;                         / * byte offset else -1 if forw. ref. */
  List *frefs;                      /* forw. ref. list                   */
#endif

       /* note: the structure of the frefs list is defined by the */
       /* machine specific translator xxxgen.c                    */
  int name;
} LabelNumber;

typedef struct LabBind              /* real user labels - for goto       */
{
  struct LabBind *labcdr;
  Symstr *labsym;
  struct LabelNumber *labinternlab;
  int labuses;
} LabBind;

/* flags for labuses in LabBind */
#define l_referenced 1
#define l_defined 2

/* Structure for a basic block                                           */

typedef struct block_head
{
  struct Icode *code;               /* start of 3-address code           */
  int length;                       /* length of ditto                   */
  int flags;                        /* misc flag bits (incl. exit cond)  */
  union { struct LabelNumber *next;     /* successor block               */
          struct LabelNumber **table;   /* or block table for switch     */
          struct block_head *backp;     /* for branch_chain() only       */
        } succ1;
  union { struct LabelNumber *next1;    /* alternative successor         */
          int tabsize;                  /* or size of switch table       */
          struct block_head *backp1;    /* for branch_chain() only       */
        } succ2;
  struct block_head *down;          /* forward chaining                  */
  struct block_head *up;            /* backward chaining                 */
  struct LabelNumber *lab;          /* label for this block              */
  struct RegList *use;              /* registers needed at block head    */
  struct BindList *stack;           /* binders active at head of block   */
  struct BindListList *debenv;      /* @@@ temp/rationalise - see flowgraph */
} block_head;

typedef struct BlockList
{ struct BlockList *blklstcdr;
  block_head *blklstcar;
} BlockList;

#define mkBlockList list2

typedef struct LoopList             /* loop optimisation */
{ struct LoopList *llcdr;
  BlockList *llblklist;             /* list of basic blocks inside       */
  block_head *llblkhd;              /* where to put found invariants     */
} LoopList;

#define mkLoopList list3

/* The following is used when a real machine register is required.       */

typedef int RealRegister;

typedef unsigned char *ClashMap;   /* dubious in ANSI-C? */

typedef struct VRegister   /* really local to regalloc.c */
{
  unsigned int rname:30, isdense:2;
  union ClashRep { struct ClashList *sparse;
                   ClashMap dense;
                 } clash;
  struct ClashList *clash2;    /* clash residual -- see regalloc.c */
  int nclashes;
  RealRegister realreg;
  int heapaddr; struct VRegister *perm;    /* duplicate somewhat */
  struct RegList *copies;
} VRegister;

typedef int VRegnum;

#define RegSort int    /* the following 3 values ... */
#define INTREG  0x00000000
#define FLTREG  0x10000000
#define DBLREG  0x20000000
#define SENTINELREG  0x30000000   /* for clarity in regalloc */

#define regname_(r) ((r) & 0x0fffffff)
#define regtype_(r) ((r) & 0xf0000000)
#define vreg_(n) (*vregheap[((n) & 0xffffff)>>REGHEAPSEGBITS]) \
                           [(n)&(REGHEAPSEGSIZE-1)].vreg
#define register_number(a) (vreg_(a)->realreg) /* for flowgraph,regalloc */
#define isarg_regnum_(r)   ((r)-R_A1 < (unsigned)NARGREGS) /* for cg.c only */
#define regbit(n) (1<<(n))
#define NMAGICREGS (NINTREGS+NFLTARGREGS+NFLTVARREGS)
/* the next lines are horrid, but less so that the previous magic numbers */
/*   - they test for virtual regs having become real - see flowgraph.c   */
#define isint_realreg_(rr) ((rr) < (unsigned)NINTREGS)
#define isflt_realreg_(rr) \
    ((rr)-(unsigned)NINTREGS < (unsigned)(NFLTARGREGS+NFLTVARREGS))
#define isany_realreg_(rr) ((rr) < (unsigned)NMAGICREGS)

#define virtreg(r,sort) ((r) | (sort))
#define V_resultreg(rsort) \
  ((rsort)==INTREG ? virtreg(R_A1,INTREG) : virtreg(R_F0, rsort))

#define GAP ((VRegnum)(0xfff00000))

typedef struct RegList {
  struct RegList *rlcdr;
  VRegnum rlcar;
} RegList;

typedef struct ClashList {      /* only for regalloc.c                 */
  struct ClashList *rlcdr;
  VRegister *vrlcar;            /* like RegList but with VRegister *'s */
} ClashList;

typedef struct AvailList
{
  struct AvailList *next;
  VRegnum reg;
  struct Binder *bind;
} AvailList;

#define mkAvailList(a,b,c) syn_list3((int)(a),(int)(b),(int)c)


typedef struct ReadonlyCopy
{
  struct ReadonlyCopy *next;
  VRegnum r1;
  VRegnum r2;
} Readonly_copy;

#define mkReadonlyCopy list3

/* Structure for an abstract instruction (3-address code)                */

typedef struct Icode
{
  int op;     /* really J_OPCODE + condition bits (Q_MASK) see jopcode.h  */
  VRegnum r1, r2;
  int m;      /* this will become VRegnum r3                              */
} Icode;

typedef struct Binder
{
  AEop h0;                  /* needed as Binder is a subtype of Expr */
  struct Binder *bindcdr;   /* reverse chain of binders at this level */
  Symstr *bindsym;          /* pointer to Symstr (for name and restore) */
  SET_BITMAP bindstg;
  TypeExpr *bindtype;
  union { int i; struct BindList *bl; } bindaddr;
/* the following fields are only present for s_auto binders */
#define SIZEOF_NONAUTO_BINDER offsetof(Binder,bindxx)
  opt VRegnum bindxx;
  opt int bindrefcount;
  opt int bindmcrep;            /* in flux */
#    define NOMCREPCACHE (-1)
} Binder;

#define bindsym_(p)  ((p)->bindsym)
#define bindstg_(p)  ((p)->bindstg)
#define bindtype_(p) ((p)->bindtype)
#define binduses_(p) ((p)->bindstg)    /* n.b. */
#define bindaddr_(p) ((p)->bindaddr.i)
#define bindxx_(p)   ((p)->bindxx)

/* bits in bindaddr */
#define BINDADDR_MASK  0xf0000000
#define BINDADDR_ARG   0x80000000
#define BINDADDR_LOC   0xc0000000
#define BINDADDR_UNSET 0xe0000000

/* bits within bindstg:  note that bitofstg_() (see h.AEops) are reserved */
#define b_synbit1    0x00100000  /* reserved for parser                     */
#define b_synbit2    0x00200000  /* reserved for parser                     */
#define b_synbits    (b_synbit1+b_synbit2)
#define b_omitextern 0x00800000
#define b_undef      0x01000000  /* flag for 'forward ref' static or extern */
#define b_fnconst    0x02000000
#define b_addrof     0x40000000
#define b_enumconst  0x10000000
#define b_localstg   0x20000000  /* for memory subpool checks */
#define isenumconst_(b) (bindstg_(b) & b_enumconst)
#define BITFIELD     0x08000000   /* beware, this is a type not stg bit */

/* bit selectors within binduses_ (now duplicate type bits in bindstg_) */
#define u_implicitdef  1
#define u_referenced   2
#define u_addrof       4
#define u_assigned    16   /* not used enough */
#define u_superceded  32

/* the type TagBinder is used for struct name bindings and is as
   Binder apart from the following fields defined by macros.
   BEWARE: new type introduced 26-11-86.   (Notionally bind/syn/sem local).
   May still depend on relationship with Binder!
*/
typedef struct TagBinder
{
  AEop h0;
  struct TagBinder *bindcdr;   /* reverse chain of binders at this level */
  Symstr *bindsym;          /* pointer to Symstr (for name and restore) */
  int tagbindsort;
  struct TagMemList *tagbindmems;
} TagBinder;

#define tagbindsort_(p) ((p)->tagbindsort)
#define tagbindmems_(p) ((p)->tagbindmems)
/* other fields h0, bindcdr, bindsym, are as for variables */

/* note the worrying commonality between TagMemList and DeclRhsList -
   worse, the code exploits this by punning.  Unify?
   FormTypeList is used in globalised types, and is a subtype of
   DeclRhsList for both minimality and space reasons.
*/
typedef struct TagMemList {
  struct TagMemList *memcdr;
  Symstr *memsv;      /* zero for 'padding' bit field */
  TypeExpr *memtype;
  Expr *membits;      /* only relevant if BITFIELD set in memtype */
} TagMemList;

typedef struct DeclRhsList {
  struct DeclRhsList *declcdr;
  Symstr *declname;
  TypeExpr *decltype;
  Expr *declinit;             /* temp. for init assignment/bit width    */
  SET_BITMAP declstg;
  opt Binder *declbind;       /* temp. working space to help ->declinit */
} DeclRhsList;

typedef struct FormTypeList {
  struct FormTypeList *ftcdr;
  Symstr *ftname;
  TypeExpr *fttype;
} FormTypeList;

typedef struct BindList {
  struct BindList *bindlistcdr;
  Binder *bindlistcar;
} BindList;

#define mkBindList list2

typedef struct BindListList {
  struct BindListList *bllcdr; BindList *bllcar; } BindListList;

typedef struct TopDecl {
  AEop h0;
  union {
    DeclRhsList *var;     /* h0 == s_decl  */
    struct {
      Binder *name;
      BindList *formals;
      Cmd  *body;
      bool ellipsis;
    } fn;                 /* h0 = s_fndef */
  } v_f;
} TopDecl;

/* for vargen.c, xxxobj.c and xxxasm.c */
typedef struct DataInit
{ struct DataInit *datacdr;
  int rpt, sort, len, val;
} DataInit;

/* to be used sparingly */
#define h0_(p) ((p)->h0)

/* static options for compiler */

/* NAMEMAX is the largest number of characters I am happy about        */
/* accepting in a name.                                                */

#define NAMEMAX  256
#define LEX_HASHSIZE 255        /* number of hash table entries          */

#define SEGSIZE     32768       /* (bytes) - unit of alloc of hunks */
#define ICODESEGSIZE  512       /* Icode vector now allocated in 8k hunks   */
#define CODEVECSEGBITS  9       /* 2k byte unit of allocation               */
#define CODEVECSEGSIZE (1<<CODEVECSEGBITS)
#define CODEVECSEGMAX  64       /* max segments (hence max 128K bytes/proc) */
#define REGHEAPSEGBITS  9       /* index array for segment of 512 vregs     */
#define REGHEAPSEGSIZE (1<<REGHEAPSEGBITS)
#define REGHEAPSEGMAX  64       /* max segments (hence max 32K vregs/proc)  */
/* An old comment claimed that LITPOOLSIZE was 1024 for 'max address range' */
/* I suspect that this is out of date now that litpool overflows gently.    */
#define LITPOOLSEGBITS  5       /* index array for segment of 32 lits       */
#define LITPOOLSEGSIZE (1<<LITPOOLSEGBITS)
#define LITPOOLSEGMAX  32       /* max segments, so 1024 lits ovfl gently   */

/* The following rather lengthy set of tests and definitions helps to       */
/* allow various options in the compiler to be selected.                    */
/* The single symbol ENABLE_ALL can be used to switch everything on.        */

#ifdef ENABLE_ALL

#  ifdef NO_ASSEMBLER_OUTPUT
#     undef NO_ASSEMBLER_OUTPUT
#  endif

#  ifndef ENABLE_LEX
#     define ENABLE_LEX       1
#  endif
#  ifndef ENABLE_SYN
#     define ENABLE_SYN       1
#  endif
#  ifndef ENABLE_CG
#     define ENABLE_CG        1
#  endif
#  ifndef ENABLE_BIND
#     define ENABLE_BIND      1
#  endif
#  ifndef ENABLE_TYPE
#     define ENABLE_TYPE      1
#  endif
#  ifndef ENABLE_REGS
#     define ENABLE_REGS      1
#  endif
#  ifndef ENABLE_OBJ
#     define ENABLE_OBJ       1
#  endif
#  ifndef ENABLE_X
#     define ENABLE_X         1
#  endif
#  ifndef ENABLE_FNAMES
#     define ENABLE_FNAMES    1
#  endif
#  ifndef ENABLE_FILES
#     define ENABLE_FILES     1
#  endif
#  ifndef ENABLE_LOOP
#     define ENABLE_LOOP      1
#  endif
#  ifndef ENABLE_Q
#     define ENABLE_Q         1
#  endif
#  ifndef ENABLE_STORE
#     define ENABLE_STORE     1
#  endif
#  ifndef ENABLE_2STORE
#     define ENABLE_2STORE    1
#  endif
#  ifndef ENABLE_SPILL
#     define ENABLE_SPILL     1
#  endif
#  ifndef ENABLE_MAPSTORE
#     define ENABLE_MAPSTORE  1
#  endif
#  ifndef ENABLE_AETREE
#     define ENABLE_AETREE    1
#  endif
#  ifndef ENABLE_PP
#     define ENABLE_PP        1
#  endif
#  ifndef ENABLE_DATA
#     define ENABLE_DATA      1
#  endif
#endif

#ifdef ENABLE_LEX
#  define DEBUG_LEX     0x1
#else                      
#  define DEBUG_LEX     0  
#endif                     
#ifdef ENABLE_SYN          
#  define DEBUG_SYN     0x2
#else                      
#  define DEBUG_SYN     0  
#endif                       
#ifdef ENABLE_CG           
#  define DEBUG_CG      0x4 
#else                        
#  define DEBUG_CG      0      
#endif                       
#ifdef ENABLE_BIND         
#  define DEBUG_BIND    0x8
#else                      
#  define DEBUG_BIND    0  
#endif
#ifdef ENABLE_TYPE
#  define DEBUG_TYPE    0x10
#else                      
#  define DEBUG_TYPE    0  
#endif                     
#ifdef ENABLE_REGS         
#  define DEBUG_REGS    0x20
#else                      
#  define DEBUG_REGS    0  
#endif                     
#ifdef ENABLE_OBJ          
#  define DEBUG_OBJ     0x40
#else                      
#  define DEBUG_OBJ     0  
#endif                     
#ifdef ENABLE_X            
#  define DEBUG_X       0x80
#else                      
#  define DEBUG_X       0  
#endif
#ifdef ENABLE_FNAMES
#  define DEBUG_FNAMES  0x100
#else
#  define DEBUG_FNAMES  0
#endif
#ifdef ENABLE_FILES
#  define DEBUG_FILES   0x200
#else
#  define DEBUG_FILES   0
#endif
#ifdef ENABLE_LOOP
#  define DEBUG_LOOP    0x400
#else
#  define DEBUG_LOOP    0
#endif
#ifdef ENABLE_Q
#  define DEBUG_Q       0x800
#else
#  define DEBUG_Q       0
#endif
#ifdef ENABLE_STORE
#  define DEBUG_STORE   0x1000
#else
#  define DEBUG_STORE   0
#endif
#ifdef ENABLE_2STORE
#  define DEBUG_2STORE  0x2000
#else
#  define DEBUG_2STORE  0
#endif
#ifdef ENABLE_SPILL
#  define DEBUG_SPILL   0x4000
#else
#  define DEBUG_SPILL   0
#endif
#ifdef ENABLE_MAPSTORE
#  define DEBUG_MAPSTORE  0x8000
#else
#  define DEBUG_MAPSTORE  0
#endif
#ifdef ENABLE_AETREE
#  define DEBUG_AETREE  0x10000
#else
#  define DEBUG_AETREE  0
#endif
#ifdef ENABLE_PP
#  define DEBUG_PP      0x20000
#else
#  define DEBUG_PP      0
#endif
#ifdef ENABLE_DATA
#  define DEBUG_DATA    0x40000
#else
#  define DEBUG_DATA    0
#endif

#define debugging(n) ((n) && _debugging & (n))

/* disable error/warnings... */
#define D_ZEROARRAY     1
#define D_ASSIGNTEST    2
#define D_SHORTWARN     4
#define D_PPALLOWJUNK   8
#define D_IMPLICITVOID 16
#define D_VALOFBLOCKS  32

/* features */
#define FEATURE_SAVENAME 1
#define FEATURE_NOUSE    2
#define FEATURE_PPNOUSE  4
#define FEATURE_RELOCATE 8
#define FEATURE_FILEX    0x10
#define FEATURE_PREDECLARE 0x20
#define FEATURE_ANOMALY  0x40
#define FEATURE_ANNOTATE 0x80

#undef opt   /* see head of file */

#endif

/* end of modes.h */
