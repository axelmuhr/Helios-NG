#ifdef USE_NORCROFT_PRAGMAS
#pragma force_top_level
#pragma include_only_once
#endif
/*
 * defs.h: front-end to cg interface.
 * Copyright (C) Acorn Computers Ltd., 1988.
 * Copyright (C) Codemist Ltd., 1987.
 */

/*
 * RCS $Revision: 1.2 $ Codemist 116
 * Checkin $Date: 1992/06/25 13:10:55 $
 * Revising $Author: nickc $
 */

/*
 * This file defines the set of data structures which constitute the
 * interface between a language-specific front-end and the code generator.
 * The set of structures is not yet minimal - some embedded comments point
 * to possible improvements.
 *
 * A similar file - cgdefs.h - defines structures which are global to the
 * code generator but hidden from front-ends. More private structures are
 * defined, perhaps opaquely, in individual module headers.
 *
 * Our intentions are good, but the execution of them is not yet perfect.
 *
 * General Convention: macros of the form foo_(x) are understood to be
 * access functions. Names with trailing underscores are used for no other
 * purpose. The names of access functions always end with an '_'.
 *
 * Some structures - e.g. Symstr and FloatCon - have trailing string fields
 * which are described as being of length [1]. These are actually allocated
 * as large as required to hold the relevant null-terminated value.
 */

#ifndef _defs_LOADED
#define _defs_LOADED 1

/*
 * C has no way to say "either a 't' or nothing" - i.e.
 * union { int t; struct {}; } is illegal. The following macro
 * defines an optional field of a struct. We #undef opt at the
 * end of this file in case of clashes.
 * NOTE: It is not clear how several opt fields are related, if at all.
 *       For example opt int a; opt int b; - which of the following sets
 *       of fields can appear: {}, {a}, {b}, {a,b} ?
 */
#define opt

/*
 * List is a generic list type of general utility.
 */
#ifndef __queue_h
typedef struct List { struct List *cdr; int32 car; } List;
#endif

#define cdr_(p) ((p)->cdr)
#define car_(p) ((p)->car)

/*
 * List3 is used in some back-ends as an extended version of the existing
 * List2 datastructure but where extra informaton is needed - the only
 * case at the tim this note was written was for long-format forward
 * references where the extra field is used to cope with some ugly
 * issues about relocating 32-bit forward references where too many
 * offsets seem needed for the original structures. (ACN Feb '90)
 */
typedef struct List3 { struct List *cdr; int32 car; int32 csr; } List3;

typedef int32      AEop;        /* An AEop is implemented as an integer */
#define SET_BITMAP int32        /* ... as are small sets... */

/*
 * Sometime the type VRegnum should be made into a union or some such that
 * can not accidentally be punned with an int32.
 */
typedef int32 VRegnum;          /* a pity this has to be here, but... */

#ifndef NON_CODEMIST_MIDDLE_END
/*
 * NMAGICREGS is defined here as whatever length integer comes out
 * of the arithmetic shows.  This is intended to allow it to be a
 * preprocessor-available constant, whereas putting in a cast to (int32)
 * would mean that #if NMAGICREGS < ..  etc would be illegal.
 */
/*
 * XXX - NC - 29/8/91
 *
 * ifdef added because I want to override definition of NMAGICREGS in target.h
 */
#ifndef NMAGICREGS
#ifdef TARGET_SHARES_INTEGER_AND_FP_REGISTERS
#define NMAGICREGS (NINTREGS)
#else
#define NMAGICREGS (NINTREGS+NFLTREGS)
#endif
#endif /* NMAGICREGS */

typedef struct RealRegSet
{
    unsigned32 map[(NMAGICREGS+31)/32];
} RealRegSet;

#endif

/*
 * A TypeExpr is the basic, generic node of a type expression tree.
 * The first field - called h0 for historical reasons - is a discriminator
 * which determines the fine structure of the node.
 * @@@ AM would like to swap typespecmap_ and typespecbind_ one day.
 */
typedef struct TypeExprFnAux
{  signed int minargs: 16, maxargs: 16, variad: 16;
   signed int sideeffects:8, oldstyle: 8;      /* notionally bool */
#ifndef NON_CODEMIST_MIDDLE_END
   RealRegSet usedregs;
#endif
} TypeExprFnAux;

/*
 * packTypeExprFnAux also sets the usedregs field to describe that ALL
 * registers are clobbered.  A more refined value is set when/if the
 * function gets defined.  Codemist + LDS agree this must be re-organised.
 */

#ifdef NON_CODEMIST_MIDDLE_END
#define packTypeExprFnAux(s,mina,maxa,var,side,old) \
  (s.minargs=mina, s.maxargs=maxa, s.variad=var,    \
   s.sideeffects=side, s.oldstyle=old,              \
   &s)
#else
#define packTypeExprFnAux(s,mina,maxa,var,side,old) \
  (s.minargs=mina, s.maxargs=maxa, s.variad=var,    \
   s.sideeffects=side, s.oldstyle=old,              \
   reg_setallused(&s.usedregs), &s)
#endif

typedef struct TypeExpr         /* make into a better union type */
{ AEop h0;
  struct TypeExpr *typearg;     /* or SET_BITMAP for s_typespec */
  struct Binder *typespecbind;  /* or TagBinder for struct/union */
  int32 dbglanginfo;            /* dbx support requires f77 type info */
#ifdef PASCAL /*ECN*/
  union {
    struct TypeExpr *type;
    struct Expr *e;
  } pun;
#endif
  TypeExprFnAux fnaux;          /* only if t_fnap                */
} TypeExpr;
/*
 * TypeExpr access functions
 */
#define typearg_(p)         ((p)->typearg)
#define typespecmap_(p)     (*(SET_BITMAP *)&(p)->typearg)
#define typespecbind_(p)    ((p)->typespecbind)
#define typespectagbind_(p) ((TagBinder *)((p)->typespecbind))
#define typefnargs_(p)      ((FormTypeList *)((p)->typespecbind))
#define typefnargs1_(p)     ((DeclRhsList *)((p)->typespecbind))
#define typeptrmap_(p)      (*(SET_BITMAP *)&((p)->typespecbind))
#define typesubsize_(p)     (*(Expr **)&((p)->typespecbind))
/* And the following apply only to function TypeExprs... */
#define typefnaux_(p)       ((p)->fnaux)
#define typedbginfo_(p)     ((p)->dbglanginfo)
#define minargs_(p)         ((p)->fnaux.minargs)
#define maxargs_(p)         ((p)->fnaux.maxargs)
#define TypeSpec TypeExpr   /* used when known to be a s_typespec TypeExpr */

/*
 * A Symstr is a symbol table entry. Symstr's can be overloaded - i.e. can
 * have several definitions - e.g. as a variable name, as a structure field,
 * as a label. The 3 'overloading classes' (pointing to Binders of one kind
 * or another) are explicitly represented.
 */
typedef struct Symstr {
  AEop h0;              /* keyword or s_identifier. Must be first field */
  struct Symstr *symchain;      /* linear list constituting hash bucket */
  /* The 4 overloading classes... */
  struct Binder  *symdata;      /* variable name, function name etc.   */
  struct LabBind *symlab;       /* definition as a label               */
  struct TagBinder *symtag;     /* structure, union, enumeration tag   */
  struct ExtRef *symext;        /* Data about use as an external sym   */
  struct Symstr *symfold;       /* for 6 char monocase extern check    */
             /* may one day allocate within *symext to save space here */
  char  symname[1];             /* Text name... allocated as needed    */
} Symstr;
/*
 * Symstr access functions
 */
#define symchain_(sym)      ((sym)->symchain)
#define symtype_(sym)       ((sym)->h0)
#define sympp_(sym)         ((sym)->sympp)
#define symlab_(sym)        ((sym)->symlab)
#define symtag_(sym)        ((sym)->symtag)
#define symext_(sym)        ((sym)->symext)
#define symfold_(sym)       ((sym)->symfold)
#define symdata_(sym)       ((sym)->symdata)
#define symname_(sym)       ((sym)->symname)

/*
 * String literals in ANSI-C are naturally segmented, so the code
 * generator is prepared to handle segmented string values.
 */
typedef struct StringSegList
{ struct StringSegList *strsegcdr;
  char *strsegbase;
  int32 strseglen;
} StringSegList;

typedef struct String
{ AEop h0;
  StringSegList *strseg;
} String;

/*
 * The following types describe the representation of floating-point
 * values by the compiler in both binary and source-related forms.
 * The order of fields in DbleBin in is exploited only in object-code
 * formatters and assembly code generators.
 */
typedef struct DbleBin {
#ifdef TARGET_HAS_OTHER_IEEE_ORDER
  int32 lsd,msd;                /* e.g. clipper */
#else
  int32 msd,lsd;                /* e.g. arm, 370 (not really ieee) */
#endif
} DbleBin;

typedef struct FloatBin {
  int32 val;
} FloatBin;

typedef struct FloatCon {
  AEop h0;                      /* the type discriminator for the union */
  SET_BITMAP floatlen;
  union {
    int32 irep[2];              /* so asemblers can print hex patterns */
    DbleBin db;
    FloatBin fb;
  } floatbin;
  char floatstr[1];             /* the source representation - if there  */
} FloatCon;                     /* is one (consider x = 1.0 + 2.0, which */
                                /* has none) - allocated as needed.      */

/*
 * ****** TEMPORARY HACK ******
 * This type should be defined by lex and exported only to the front end.
 * To achieve this, we need to introduce char *curlex_name() and add support
 * to pp for the pre-process-only compiler option. Introducing symbol_name()
 * and making sym_name_table private to lex introduces further cleanliness
 * and room for manoeuvre in the fron end(s).
 */
typedef struct SymInfo {
    AEop sym;
    union { char *s; int32 i; Symstr *sv; FloatCon *fc; } a1;
    union { int32 len, flag; } a2;
} SymInfo;

/*
 * FileLine structures are used widely in support of debuggers,
 * so we make the type global here. Essentially, a FileLine can
 * associate a file name and a line number with any object.
 * @@@ p should be of type struct dbg_... * @@@
 */
typedef struct FileLine {char *f; int32 l; VoidStar p;} FileLine;

typedef List ExprList;          /* a list of Exprs is a List... */
/*
 * exprcar: ExprList -> Expr    -- extract an Expr from an Expr list element
 * mkExprList: ExprList x Expr -> ExprList -- add an Expr to an ExprList
 */
#define exprcar_(l)         (*(Expr **)&(l)->car)
#define mkExprList(a,b)     syn_cons2(a,b)

typedef List CmdList;           /* a list of commands is a List... */
/*
 * cmdcar: CmdList -> Cmd       -- extract a Cmd from a Cmd list element
 * mkCmdList: CmdList x Cmd -> CmdList -- add a Cmdr to a CmdList
 */
#define mkCmdList(a,b)      syn_cons2(a,b)
#define cmdcar_(l)          ((Cmd *)(l)->car)

/*
 * A node of an expression tree is an Expr...
 */
typedef struct Expr {
  AEop h0;                      /* node type, discriminates unions */
  TypeExpr *etype;
  union Expr_1 {
      struct Expr *e;           /* as in expr :: unop exprr...   */
      struct Cmd *c;            /* for valof expressions...      */
      struct SynBindList *bl;   /* for expr :: (let t; f(t,...)) */
      int32 i;                  /* h0==s_integer; expr = intval  */
  } arg1;
  opt union Expr_2 {
      struct Expr *e;           /* e.g. expr :: expr binop expr, */
                                /* & const expr yielding arg1.i  */
      ExprList *l;              /* as in expr :: f(expr,expr...) */
      int32 i;                  /* as in expr :: expr dot intval */
  } arg2;
  opt union Expr_3 {
      struct Expr *e;           /* eg ?-expr :: expr expr expr...*/
                                /*   (as in (e1 ? e2 : e3))      */
      int32 i;                  /* for bitfields, intval bitsize */
  } arg3;
  opt int32 msboff;             /* for bitfields, ms bit offset  */
} Expr;
/*
 * And the Expr access functions...
 */
#define type_(p)            ((p)->etype)
#define arg1_(p)            ((p)->arg1.e)
#define arg2_(p)            ((p)->arg2.e)
#define arg3_(p)            ((p)->arg3.e)
#define exprletbind_(p)     ((p)->arg1.bl)
#define exprfnargs_(p)      ((p)->arg2.l)
#define exprdotoff_(p)      ((p)->arg2.i)
#define exprbsize_(p)       ((p)->arg3.i)
#define exprmsboff_(p)      ((p)->msboff)
#define expr1c_(p)          ((p)->arg1.c)
/* for integer constants */
#define intval_(p)          ((p)->arg1.i)
#define intorig_(p)         arg2_(p)

/*
 * The other essential object the code generator recognises is a Cmd,
 * which represents a command or statement. The structure is very flexible
 * and is designed to represent any C abstract syntax command.
 */
typedef struct Cmd {
  AEop h0;                      /* The node type and union discriminator */
  FileLine fileline;            /* File, line, and code address hook...  */
  union {                       /* Then up to four component Cmd/Exprs...*/
      struct Cmd *c;            /* Consider, for example, the C 'for':-  */
      Expr *e;                  /* for-cmd :: expr expr expr cmd         */
  } cmd1,cmd2,cmd3,cmd4;        /* (for (e1; e2; e3) stmnt;)             */
} Cmd;
/*
 * Cmd structure access functions...
 */
#define cmd1c_(x)           ((x)->cmd1.c)
#define cmd2c_(x)           ((x)->cmd2.c)
#define cmd3c_(x)           ((x)->cmd3.c)
#define cmd4c_(x)           ((x)->cmd4.c)
#define cmd1e_(x)           ((x)->cmd1.e)
#define cmd2e_(x)           ((x)->cmd2.e)
#define cmd3e_(x)           ((x)->cmd3.e)
#define cmd4e_(x)           ((x)->cmd4.e)
/*
 * And some more convenient syntactic sugaring, which hints how other
 * structures such as case labels, labels, and blocks.
 */
#define cmd1lab_(c)         ((LabBind *)cmd1c_(c))
#define switch_caselist_(c) cmd3c_(c)
#define switch_default_(c)  cmd4c_(c)
#define case_next_(c)       cmd3c_(c)
#define case_lab_(c)        ((struct LabelNumber *)cmd4c_(c))
#define cmdblk_bl_(c)       ((SynBindList *)cmd1c_(c))
#define cmdblk_cl_(c)       ((CmdList *)cmd2c_(c))

/*
 * LabBinds describe source-level labels (e.g. 'out' in goto out;... out:...)
 * and addres the corresponding internal LabelNumber.
 */
typedef struct LabBind {
  struct LabBind *labcdr;       /* rev. list of binders at this scope level */
  Symstr *labsym;               /* the label's name */
  struct LabelNumber *labinternlab;     /* and its internal representation, */
                                        /* (opaque to back end).            */
  int32 labuses;                /* flags, further elucidated below */
#ifdef PASCAL /*ECN*/
  union {
    struct Binder *jbuf;
    struct Binder *proc;
  } lun;
  int bindlevel : 8;
#endif
} LabBind;

/*
 * Flags for labuses in LabBind
 */
#define l_referenced        1L
#define l_defined           2L

/*
 * Binders describe variables and functions (named objects).
 * The code generator also introduces Binders for some anonymous abjects.
 */
typedef struct Binder {
  AEop h0;                      /* needed as Binder is a subtype of Expr */
  struct Binder *bindcdr;       /* reverse chain of binders at this level */
  Symstr *bindsym;              /* pointer to Symstr (for name and restore) */
  SET_BITMAP bindstg;           /* flags describing sort of object - also */
                                /* used to discriminate unions and opts */
  TypeExpr *bindtype;           /* the object's type... but disappears */
                                /* for auto objects - cached in bindmcrep */
  union {                       /* discriminated by bindstg & b_bindaddrlist */
     int32 i;                   /* e.g. stack address... */
     struct BindList *bl;       /* e.g. list of active binders in a fn... */
     struct Expr *c;            /* e.g. for a constant expression... */
  } bindaddr;
#ifdef PASCAL /*ECN*/
  int bindlevel : 8;
  int synflags : 16;
#endif
#define SIZEOF_NONAUTO_BINDER offsetof(Binder,bindxx)
  opt VRegnum bindxx;           /* these 3 fields in s_auto Binders only */
  opt int32 bindmcrep;          /* in flux */
#define NOMCREPCACHE (-1L)
} Binder;
/*
 * Useful access functions...
 */
#define bindsym_(p)         ((p)->bindsym)
#define bindstg_(p)         ((p)->bindstg)
#define bindtype_(p)        ((p)->bindtype)
#define binduses_(p)        ((p)->bindstg)    /* n.b. */
#define bindaddr_(p)        ((p)->bindaddr.i)
#define constexpr_(p)       ((p)->bindaddr.c)
#define bindxx_(p)          ((p)->bindxx)

/*
 * Flag bits in bindaddr
 */

#define BINDADDR_MASK       0xf0000000U
  
/* Use top-bit-set values to provide a little free union checking.      */
/* The lower 28 bits give offset within class for local_address()       */
/* (q.v.) which should only get to see BINDADDR_ARG or BINDADDR_LOC.    */
  
#define BINDADDR_ARG        0x80000000U /* a formal parameter           */
#define BINDADDR_LOC        0xc0000000U /* a local variable             */
  
/* The next case is currently only used if TARGET_STACK_MOVES_ONCE and  */
/* is converted to BINDADDR_LOC by flowgraf.c  (in flux Nov89).         */
  
#define BINDADDR_NEWARG     0xd0000000U /* an actual parameter          */
#define BINDADDR_UNSET      0xe0000000U
/*
 * Flag bits within bindstg:  note that bitofstg_() (see AEops.h) are reserved.
 */
#define b_synbit1           0x00100000U /* reserved to parser */
#define b_synbit2           0x00200000U /* reserved to parser */
#define b_synbits           (b_synbit1+b_synbit2)
#define b_dbgbit            0x00400000U /* reserved for debugger support */
#define b_omitextern        0x00800000U
#define b_undef             0x01000000U /* 'forward ref' static or extern */
#define b_fnconst           0x02000000U
#define b_bindaddrlist      0x04000000U /* discriminator for bindstg */
#define b_addrof            0x08000000U
#define b_localstg          0x10000000U /* for memory subpool checks */
#define b_enumconst         0x20000000U
#define b_globalregvar      0x40000000U /* for CSE's benefit */
#define b_noalias           0x80000000U /* variable p is a pointer: *(p+x)
                                           (any x) is guaranteed to have no
                                           aliases not involving p */
#define isenumconst_(b)     (bindstg_(b) & b_enumconst)
#define BITFIELD            0x40000000L  /* beware: a type not a stg bit */
/*
 * bit selectors within binduses_ (now duplicate type bits in bindstg_)
 */
#define u_implicitdef       1L
#define u_referenced        2L
#define u_addrof            4L
#define u_assigned         16L  /* not used enough */
#define u_superceded       32L
#define u_tentative        64L
#define u_bss             128L

/*
 * The type TagBinder is used for struct name bindings and is similar
 * to Binder. BEWARE: the relationship with Binder may be depended upon.
 */
typedef struct TagBinder
{
  AEop h0;                      /* Must be the first field... */
  struct TagBinder *bindcdr;    /* reverse chain of binders at this level */
  Symstr *bindsym;              /* pointer to Symstr (for name and restore) */
  int  tagbindsort:16;          /* discriminates struct, union, (enum?) */
  int  tagbindstate:16;         /* TB_UNDEFMSG, TB_BEINGDEFD.           */
  struct TagMemList *tagbindmems;  /* list of struct/union/enum members */
#ifdef TARGET_HAS_DEBUGGER
  int32 tagbinddbg;             /* space reserved to debug-table writer */
#endif
} TagBinder;
#define tagbindsort_(p)     ((p)->tagbindsort)
#define tagbindstate_(p)    ((p)->tagbindstate)
#define tagbindmems_(p)     ((p)->tagbindmems)
#define TB_BEINGDEFD 1          /* police "struct d { struct d { ..."   */
#define TB_UNDEFMSG  2          /* so 'size needed' msg appears once.   */

/*
 * NOTE the worrying commonality between TagMemList and DeclRhsList -
 * worse, the code exploits this by punning.  Unify?
 * FormTypeList is used in globalised types, and is a subtype of
 * DeclRhsList for both minimality and space reasons.
 */
typedef struct TagMemList {
  struct TagMemList *memcdr;
  Symstr *memsv;                /* zero for 'padding' bit field */
  TypeExpr *memtype;
  Expr *membits;                /* only relevant if BITFIELD set in memtype */
#ifdef PASCAL /*ECN*/
  int32 offset;
  struct VariantList *vlist;
#endif
} TagMemList;

typedef struct DeclRhsList {
  struct DeclRhsList *declcdr;
  Symstr *declname;
  TypeExpr *decltype;
  Expr *declinit;               /* temp. for init assignment/bit width    */
#ifdef PASCAL /*ECN*/
  int synflags : 16;
  int section : 8;
#endif
  SET_BITMAP declstg;
  FileLine fileline;            /* added by RCC 25-Mar-88 */
  opt Binder *declbind;         /* temp. working space to help ->declinit */
} DeclRhsList;

typedef struct FormTypeList {
  struct FormTypeList *ftcdr;
  Symstr *ftname;
  TypeExpr *fttype;
#ifdef PASCAL /*ECN*/
  int synflags : 16;
  int section : 8;
#endif
} FormTypeList;

/*
 * SynBindList and BindList are notionally identical, but AM wants
 * to separate concerns while re-arranging allocators.
 */
typedef struct SynBindList {
  struct SynBindList *bindlistcdr;
  Binder *bindlistcar;
} SynBindList;

typedef struct BindList {
  struct BindList *bindlistcdr;
  Binder *bindlistcar;
} BindList;

#define mkSynBindList(a,b)  ((SynBindList *)syn_cons2(a,b))
#define mkBindList(a,b)     ((BindList *)binder_cons2(a,b))

typedef struct BindListList {
  struct BindListList *bllcdr;
  SynBindList *bllcar;
} BindListList;

typedef struct TopDecl {        /* a top-level decalration */
    AEop h0;                    /* discriminator for union v_f... */
    union {
        DeclRhsList *var;       /* h0 == s_decl => variable  */
        struct {
          Binder *name;         /* the function's name */
          SynBindList *formals; /* its formal argument list... */
          Cmd  *body;           /* its body... */
          bool ellipsis;        /* and whether the argument list ends '...' */
        } fn;                   /* h0 = s_fndef => fn definition */
    } v_f;
} TopDecl;

/*
 * ****** TEMPORARY HACK ******
 * This structure should be private to the back-end. Making it so
 * requires splitting vargen into machine-specific and language-specific
 * parts. Also used by xxxobj.c and xxxasm.c
 * @@@ LDS: also used by bind.c for tentative definition stuff... but that
 * be improved by moving save/restore_vargen_state to vargen...
 */
typedef struct DataInit {
    struct DataInit *datacdr;
    int32 rpt, sort, len, val;
} DataInit;

/*
 * To be used sparingly...
 */
#define h0_(p) ((p)->h0)

typedef struct Unbinder {
    struct Unbinder *unbindcdr;
    Symstr *unbindsym;
    Binder *unbindold;
} Unbinder;

typedef struct Untagbinder {
    /* the field names are deliberately the same as for Unbinder */
    struct Untagbinder *unbindcdr;
    Symstr *unbindsym;
    TagBinder *unbindold;
} Untagbinder;

#undef opt                      /* see head of file */

#endif

/* end of defs.h */
