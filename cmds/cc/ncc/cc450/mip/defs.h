/*
 * defs.h: front-end to cg interface.
 * Copyright (C) Acorn Computers Ltd., 1988-1990.
 * Copyright (C) Codemist Ltd., 1987-1992.
 * Copyright (C) Advanced RISC Machines Limited, 1991-1992.
 */

/*
 * RCS $Revision: 1.4 $ Codemist 123
 * Checkin $Date: 1995/05/19 13:50:07 $
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

#ifndef _host_LOADED
#include "host.h"
#endif

/* The following lines allow the precise details of C++ scope rules     */
/* to be ignored.  Consider  'struct A { struct B *c; }'                */
/* where B is undefined or 'struct A { struct C{} *c; };'.  IF C++      */
/* really means that structs are SCOPE's then struct B (and its typedef */
/* should not be exported from scope A.  gcc seems to export B/C!       */
/* By pre-declaring we create the struct at top level, not inside A.    */
/* These are not conditional on __cplusplus because they slightly       */
/* improve life when debugging the compiler with a debugger that        */
/* doesn't understand struct names.                                     */

typedef struct Binder Binder;
typedef struct LabBind LabBind;
typedef struct TagBinder TagBinder;
typedef struct ExtRef ExtRef;
typedef struct Cmd Cmd;
typedef struct SynBindList SynBindList;
typedef struct LabelNumber LabelNumber;
typedef struct BindList BindList;
typedef struct ClassMember ClassMember;

/*
 * List is a generic list type of general utility.
 */
typedef struct List { struct List *cdr; int32 car; } List;
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
{  unsigned16 minargs, maxargs;
#ifdef TARGET_IS_HELIOS
   int8 variad;	/* XXX - NC - 12/7/93 must be signed as default value is -1, and value is tested as ">0" */
#else
   unsigned8 variad;
#endif
   unsigned8 oldstyle;      /* notionally bool */
   unsigned8 flags;
   int32 inlinecode;
#ifndef NON_CODEMIST_MIDDLE_END
   RealRegSet usedregs;
#endif
} TypeExprFnAux;

/* flags bits are mostly bitoffnaux_() bits, together with ... */
#define f_specialargreg 0x80L
#define f_nofpregargs 0x40L

#define fntypeisvariadic(t) (maxargs_(t)==1999) /* could do with improvement */

/*
 * packTypeExprFnAux also sets the usedregs field to describe that ALL
 * registers are clobbered.  A more refined value is set when/if the
 * function gets defined.  Codemist + LDS agree this must be re-organised.
 */

#ifdef NON_CODEMIST_MIDDLE_END
#define packTypeExprFnAux1(s,mina,maxa,var,old,fl,il) \
  (s.minargs=mina, s.maxargs=maxa, s.variad=((var)<0 ? 0:(var)), \
   s.oldstyle=old, s.flags=(unsigned8)fl, s.inlinecode = il, \
   &s)
#else
#define packTypeExprFnAux1(s,mina,maxa,var,old,fl,il) \
  (s.minargs=mina, s.maxargs=maxa, s.variad=((var)<0 ? 0:(var)), \
   s.oldstyle=old, s.flags=(unsigned8)fl, s.inlinecode = il, \
   reg_setallused(&s.usedregs), &s)
#endif
#define packTypeExprFnAux(s,mina,maxa,var,old,fl) \
        packTypeExprFnAux1(s,mina,maxa,var,old,fl,0)

typedef struct TypeExpr TypeExpr;
typedef struct Expr Expr;

struct TypeExpr                 /* make into a better union type */
{ AEop h0;
  TypeExpr *typearg;            /* or SET_BITMAP for s_typespec */
  Binder *typespecbind;         /* or TagBinder for struct/union */
  int32 dbglanginfo;            /* dbx support requires f77 type info */
#ifdef PASCAL /*ECN*/
  union {
    TypeExpr *type;
    Expr *e;
  } pun;
#endif
  TypeExprFnAux fnaux;          /* only if t_fnap                */
};
/*
 * TypeExpr access functions
 */
#define typearg_(p)         ((p)->typearg)
#define typespecmap_(p)     (*(SET_BITMAP *)&(p)->typearg)
#define typespecbind_(p)    ((p)->typespecbind)
#define typespectagbind_(p) ((TagBinder *)((p)->typespecbind))
#define typefnargs_(p)      ((FormTypeList *)((p)->typespecbind))
#define typefnargs1_(p)     (*(DeclRhsList **)&((p)->typespecbind))
#define typeovldlist_(p)    (*(BindList **)&((p)->typespecbind))
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
typedef struct Symstr Symstr;
struct Symstr {
  AEop h0;              /* keyword or s_identifier. Must be first field */
  struct Symstr *symchain;      /* linear list constituting hash bucket */
  /* The 4 overloading classes... */
  Binder  *symbind;             /* variable name, function name etc.   */
  LabBind *symlab;              /* definition as a label               */
  TagBinder *symtag;            /* structure, union, enumeration tag   */
  ExtRef *symext;               /* Data about use as an external sym   */
  Symstr *symfold;              /* for 6 char monocase extern check    */
             /* may one day allocate within *symext to save space here */
  char  symname[1];             /* Text name... allocated as needed    */
};
/*
 * Symstr access functions
 */
#define symchain_(sym)      ((sym)->symchain)
#define symtype_(sym)       ((sym)->h0)
#define sympp_(sym)         ((sym)->sympp)
#define symlab_(sym)        ((sym)->symlab)
/* #define symtag_(sym)        ((sym)->symtag)        @@@ under threat */
#define symext_(sym)        ((sym)->symext)
#define symfold_(sym)       ((sym)->symfold)
/* #define symdata_(sym)       ((sym)->bindglobal)    @@@ under threat */
#define symname_(sym)       ((sym)->symname)
#define bind_global_(sym)   ((sym)->symbind)
#define tag_global_(sym)    ((sym)->symtag)

/* Backwards compatibility macro(s), for transputer code.  Tony 3/1/95 */
#define symdata_(sym)	    ((sym)->symbind)

/*
 * String literals in ANSI-C are naturally segmented, so the code
 * generator is prepared to handle segmented string values.
 */
typedef struct StringSegList StringSegList;
struct StringSegList
{ StringSegList *strsegcdr;
  char *strsegbase;
  int32 strseglen;
};

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
 * FileLine structures are used to describe source file positions and
 * in support of debugger table generation. Essentially, a FileLine can
 * associate a file name and position with any object.
 * Notionally, p should be of type struct dbg_... *.
 */
typedef struct FileLine {
  char *f;                      /* The source file name */
#if 0
  unsigned16 l;                 /* Source file line number */
#else
  unsigned32 l;			/* Fix for warnings in cg.c/showcode.c.  Tony 3/1/95 */
#endif
  unsigned16 column;            /* position within line */
  int32 filepos;                /* source file byte offset */
  VoidStar p;
} FileLine;

typedef List ExprList;          /* a list of Exprs is a List... */
/*
 * exprcar: ExprList -> Expr    -- extract an Expr from an Expr list element
 * mkExprList: ExprList x Expr -> ExprList -- add an Expr to an ExprList
 */
#define exprcar_(l)         (*(Expr **)&(l)->car)
#define mkExprList(a,b)     ((ExprList *)syn_cons2(a,b))

typedef List CmdList;           /* a list of commands is a List... */
/*
 * cmdcar: CmdList -> Cmd       -- extract a Cmd from a Cmd list element
 * mkCmdList: CmdList x Cmd -> CmdList -- add a Cmdr to a CmdList
 */
#define mkCmdList(a,b)      ((CmdList *)syn_cons2(a,b))
#define cmdcar_(l)          ((Cmd *)(l)->car)

/*
 * A node of an expression tree is an Expr...
 * (or a FloatCon or String or Binder)
 */
struct Expr {
  AEop h0;                      /* node type, discriminates unions */
  TypeExpr *etype;
  FileLine *fileline;           /* only a small subset of Exprs want this */
  union Expr_1 {
      Expr *e;                  /* as in expr :: unop exprr...   */
      Cmd *c;                   /* for valof expressions...      */
      SynBindList *bl;          /* for expr :: (let t; f(t,...)) */
      int32 i;                  /* h0==s_integer; expr = intval  */
  } arg1;
  /* Fields from here on are optional and may not even be allocated.      */
  union Expr_2 {
      Expr *e;                  /* e.g. expr :: expr binop expr, */
                                /* & const expr yielding arg1.i  */
      ExprList *l;              /* as in expr :: f(expr,expr...) */
      int32 i;                  /* as in expr :: expr dot intval */
  } arg2;
  union Expr_3 {
      Expr *e;                  /* eg ?-expr :: expr expr expr...*/
                                /*   (as in (e1 ? e2 : e3))      */
      int32 i;                  /* for bitfields, intval bitsize */
  } arg3;
  int32 msboff;                 /* for bitfields, ms bit offset  */
};
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
struct Cmd {
  AEop h0;                      /* The node type and union discriminator */
  FileLine fileline;            /* File, line, and code address hook...  */
  union {                       /* Then up to four component Cmd/Exprs...*/
      Cmd *c;                   /* Consider, for example, the C 'for':-  */
      Expr *e;                  /* for-cmd :: expr expr expr cmd         */
  } cmd1,cmd2,cmd3,cmd4;        /* (for (e1; e2; e3) stmnt;)             */
};
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
#define case_lab_(c)        ((LabelNumber *)cmd4c_(c))
#define cmdblk_bl_(c)       ((SynBindList *)cmd1c_(c))
#define cmdblk_cl_(c)       ((CmdList *)cmd2c_(c))

/*
 * LabBinds describe source-level labels (e.g. 'out' in goto out;... out:...)
 * and addres the corresponding internal LabelNumber.
 */
struct LabBind {
  LabBind *labcdr;              /* rev. list of binders at this scope level */
  Symstr *labsym;               /* the label's name */
  LabelNumber *labinternlab;    /* and its internal representation, */
                                /* (opaque to back end).            */
  int32 labuses;                /* flags, further elucidated below */
#ifdef PASCAL /*ECN*/
  union {
    Binder *jbuf;
    Binder *proc;
  } lun;
  unsigned8 bindlevel;
#endif
};

/*
 * Flags for labuses in LabBind
 */
#define l_referenced        1L
#define l_defined           2L

/*
 * In C++, Binders, TagBinders and ClassMembers can all occur in name scopes,
 * on the same list of named entities, discriminated by their h0 fields.
 * Pro tem, and prior to more radical rationalisation, we ensure that each
 * structure has the same common head:
 *    AEop h0;
 *    SelfType *cdr;
 *    Symstr *name;
 *    TypeExpr *type;
 *    SET_BITMAP attributes;
 * Attributes sweeps up C++ access specifiers (via bitofaccess_(), base
 * class flags and other miscellaneous flag values. Eventually, we could
 * unify attributes with h0.
 */

#define  attributes_(p)  ((p)->attributes)
/* Fields of attributes...        */
/* bitoftype_(s_enum, s_struct, s_class, s_union) = 0x0e4 */
/* bitofaccess_(s_private, s_protected, s_public) = 0x700 */
/* ... bits 0x0f000 reserved to TagBinder state flags...  */
/* ... bits 0xf0000 reserved to ClassMember attributes.   */

/*
 * Binders describe variables and functions (named objects).
 * The code generator also introduces Binders for some anonymous abjects.
 */
struct Binder {
  AEop h0;                      /* needed as Binder is a subtype of Expr */
  Binder *bindcdr;              /* reverse chain of binders at this level */
  Symstr *bindsym;              /* pointer to Symstr (for name and restore) */
  TypeExpr *bindtype;           /* the object's type... but disappears */
                                /* for auto objects - cached in bindmcrep */
  SET_BITMAP attributes;        /* this scope entity's attributes... */

  SET_BITMAP bindstg;           /* flags describing sort of object - also */
                                /* used to discriminate unions and opts */
  union {                       /* discriminated by bindstg & b_bindaddrlist */
     int32 i;                   /* e.g. stack address... */
     BindList *bl;              /* e.g. list of active binders in a fn... */
     Expr *c;                   /* e.g. for a constant expression... */
     Binder *realbinder;        /* for instate_alias()... */
  } bindaddr;
#ifdef PASCAL /*ECN*/
  unsigned8 bindlevel;
  unsigned16 synflags;
#endif
#define SIZEOF_NONAUTO_BINDER offsetof(Binder,bindxx)
  VRegnum bindxx;               /* these 3 fields in s_auto Binders only */
  int32 bindmcrep;              /* in flux */
#define NOMCREPCACHE (-1L)
};
/*
 * Useful access functions...
 */
#define bindcdr_(p)         ((p)->bindcdr)
#define bindsym_(p)         ((p)->bindsym)
#define bindstg_(p)         ((p)->bindstg)
#define bindtype_(p)        ((p)->bindtype)
#define binduses_(p)        ((p)->bindstg)    /* n.b. */
#define bindaddr_(p)        ((p)->bindaddr.i)
#define constexpr_(p)       ((p)->bindaddr.c)
#define realbinder_(p)      ((p)->bindaddr.realbinder)
#define bindxx_(p)          ((p)->bindxx)
/*
 * Flag bits in bindaddr
 */
#define BINDADDR_MASK       0xf0000000L
/* Use top-bit-set values to provide a little free union checking.      */
/* The lower 28 bits give offset within class for local_address()       */
/* (q.v.) which should only get to see BINDADDR_ARG or BINDADDR_LOC.    */
#define BINDADDR_ARG        0x80000000L /* a formal parameter           */
#define BINDADDR_LOC        0xc0000000L /* a local variable             */
/* The next case is currently only used if TARGET_STACK_MOVES_ONCE and  */
/* is converted to BINDADDR_LOC by flowgraf.c  (in flux Nov89).         */
#define BINDADDR_NEWARG     0xd0000000L /* an actual parameter          */
#define BINDADDR_UNSET      0xe0000000L

/* aeops.h reserves the (lsb) bits satisfying isdeclstarter_() for a    */
/* bit map of types and storage classes (C allows their intermixing).   */
/* Nov 92: there are 17 type bits and 10 STGBITS (C++ adds 3) so the    */
/* top 5 bits plus the lower STGBITS are available for the uses listed  */
/* below (b_synbits aid the parsing of the bitmap).                     */
/* These bits are now (Apr 93) always kept separate.                    */
/* After pasrsing, the type has the additional value 'BITFIELD'. The    */
/* storage part is hel in bindstg_ and shares with the binduses_ field  */
/* of a Binder. The stg part can (and does) re-use the type part of the */
/* map below.                                                           */

#define b_synbit1           0x80000000L /* reserved to parser */
#define b_synbit2           0x40000000L /* reserved to parser */
#define b_synbits           (b_synbit1+b_synbit2)
#define b_pseudonym         0x20000000L /* for instate_alias            */
#define b_dbgbit            0x10000000L /* reserved to debugger support */
/* N.B. none of the above 4 bits were used by instate_decl/member.      */
/* (they are only tested via bindstg_(b) & b_xxx).                      */
/* So, pending rework, we have used the 0x70000000 bits for access      */
/* specs (so to remove an arg to instate_member to allow instate_decl   */
/* to be merged).  Beware.  AM Feb 92.                                  */
/* @@@ HIGHLY DEPENDENT ON aeops.h!   Rework soon.                      */
#define killstgacc_(m)      ((m) & ~0x70000000)
#define attribofstgacc_(m)  \
              ((((m) >> 28) & 7) << shiftoftype_(s_union+1))
#define stgaccof_(s)        (1L << (s)-s_private+28)
#define b_memfna            0x08000000L /* memfn (non-static)           */
#define b_memfns            0x00010000L /* memfn (static)               */

/*
 * bit selectors within binduses_ (now duplicate type bits in bindstg_)
 * I.e. bits additional to STGBITS, sharing with TYPEBITS:
 * There are NUM_OF_TYPES (Oct 92: this is 17) bits available.
 * Nov 92: 2 bits remain unused.
 */
#define u_implicitdef     0x0001L
#define u_referenced      0x0002L
#define u_addrof          0x0004L
#define u_bss             0x0008L
#define u_constdata       0x0010L
#define u_loctype          (u_bss+u_constdata)
#define u_superceded      0x0020L
#define u_spare           0x0040L

/* bit selectors logically within the sharing bindstg_:                 */
#define b_addrof          0x0080L
#define b_maybeinline     0x0100L /* for class member fns until definite */
#define b_enumconst       0x0200L
#define b_omitextern      0x0400L
#define b_undef           0x0800L /* 'forward ref' static or extern */
#define b_fnconst         0x1000L
#define b_bindaddrlist    0x2000L /* discriminator for bindstg */
#define b_noalias         0x4000L /* variable p is a pointer: *(p+x)
                                     (any x) is guaranteed to have no
                                      aliases not involving p */
#define b_spare           0x8000L /* for CSE's benefit */
#define b_globalregvar    bitofstg_(s_globalreg)
/*      STGBITS       0x07fe0000L */

#define isenumconst_(b)     (bindstg_(b) & b_enumconst)

/* Bits additional to TYPEBITS, sharing with STGBITS:                   */
#define BITFIELD  bitoftype_(s_auto)    /* beware: a type not a stg bit */

#ifdef CPLUSPLUS
typedef struct Friend Friend;
struct Friend {
  Friend *friendcdr;
  union {
    TagBinder *friendclass;             /* h0_() == s_tagbind */
    Binder *friendfn;                   /* h0_() == s_binder  */
  } u;
};
#endif

/*
 * The type TagBinder is used for struct name bindings and is similar
 * to Binder. BEWARE: the relationship with Binder may be depended upon.
 */
struct TagBinder
{
  AEop h0;                      /* Must be the first field... */
  TagBinder *bindcdr;           /* reverse chain of binders at this level */
  Symstr *bindsym;              /* pointer to Symstr (for name and restore) */
  TypeExpr *tagbindtype;        /* pointer to parent type... */
  SET_BITMAP attributes;        /* discriminates struct, union, (enum?)... */
                                /* ...TB_UNDEFMSG, TB_BEINGDEFD, TB_DEFD.  */

  ClassMember *tagbindmems;     /* list of struct/union/enum members */
#ifdef CPLUSPLUS
  Friend *friends;              /* list of friends of the class... */
  TagBinder *tagparent;         /* parent class of class or 0.          */
#endif
#ifdef TARGET_HAS_DEBUGGER
  int32 tagbinddbg;             /* space reserved to debug-table writer */
#endif
};

#define tagbindcdr_(p)      ((p)->bindcdr)
#define tagbindsym_(p)      ((p)->bindsym)
#define tagbindtype_(p)     ((p)->tagbindtype)
#define tagbindmems_(p)     ((p)->tagbindmems)
/* Fields of attributes...                                              */
#define TB_BEINGDEFD   0x1000   /* police "struct d { struct d { ..."   */
#define TB_UNDEFMSG    0x2000   /* so 'size needed' msg appears once.   */
#define TB_DEFD        0x4000   /* in C++ struct a;/struct a{} differ.  */
#define TB_HASVTABLE   0x8000   /* has a virtual function table member. */

/*
 * NOTE the worrying commonality between ClassMember and DeclRhsList -
 * worse, the code exploits this by punning.  Unify?
 * FormTypeList is used in globalised types, and is a subtype of
 * DeclRhsList for both minimality and space reasons.
 */
struct ClassMember
{ AEop h0;                      /* must be first field...  */
  ClassMember *memcdr;          /* must be second field... */
  Symstr *memsv;                /* zero for 'padding' bit field */
  TypeExpr *memtype;
#define ACCESSADJ te_void       /* flag for [ES, p245]                  */
  SET_BITMAP attributes;        /* TML_xxx + bitofaccess_(s_public)...  */
/* >>> the above fields need to match those for Binder etc <<<          */
  TypeExpr *memtype2;   /* experimental: has (non-static) parent class! */
  union { Expr *membits;        /* only relevant if memtype is BITFIELD */
          Binder *memfname;     /* only relevant if memtype is t_fnap   */
          int32 enumval;        /* only relevant if enumeration member  */
        } u;                    /* 'isbitfield_type()' can discriminate */
#ifdef PASCAL /*ECN*/
  int32 offset;
  struct VariantList *vlist;
#endif
};

#define is_datamember_(/*(ClassMember *)*/l) \
    (h0_(l) == s_member && l->memtype != ACCESSADJ)
        /* was also ... && !isfntype(l->memtype) */

#define memcdr_(p)   ((p)->memcdr)
#define memsv_(p)    ((p)->memsv)
#define memtype_(p)  ((p)->memtype)
#define memtype2_(p)  ((p)->memtype2)
/* Fields of attributes... */
#define CB_VBASE  0x10000       /* virtual base class */
#define CB_BASE   0x20000       /* base class         */
#define CB_CORE   0x40000       /* core of class      */
#define CB_VBPTR  0x80000       /* ptr to VBASE       */
#define CB_VTAB  0x100000       /* vtable pointer...  */
#define CB_MASK  0x1f0000

typedef struct DeclRhsList DeclRhsList;
struct DeclRhsList {
  DeclRhsList *declcdr;
  Symstr *declname;
  TypeExpr *decltype;
  union {
      Expr *init;               /* temp. for init (shares with declbits). */
      Expr *bits;               /* (members never have inits).            */
      int32 stgval;             /* for global register number             */
  } u;
#ifdef PASCAL /*ECN*/
  unsigned16 synflags;
  unsigned8 section;
#endif
  SET_BITMAP declstg;
  FileLine fileline;            /* added by RCC 25-Mar-88 */
  Binder *declbind;             /* temp. working space to help ->declinit */
};

#define declinit_(d) ((d)->u.init)
#define declbits_(d) ((d)->u.bits)
#define declstgval_(d) ((d)->u.stgval)

typedef struct FormTypeList FormTypeList;
struct FormTypeList {
  FormTypeList *ftcdr;
  Symstr *ftname;
  TypeExpr *fttype;
#ifdef PASCAL /*ECN*/
  unsigned16 synflags;
  unsigned8 section;
#endif
};

/*
 * SynBindList and BindList are notionally identical, but AM wants
 * to separate concerns while re-arranging allocators.
 */
struct SynBindList {
  SynBindList *bindlistcdr;
  Binder *bindlistcar;
};

struct BindList {
  BindList *bindlistcdr;
  Binder *bindlistcar;
};

#define mkSynBindList(a,b)  ((SynBindList *)syn_cons2(a,b))
#define mkBindList(a,b)     ((BindList *)binder_cons2(a,b))

typedef struct BindListList BindListList;
struct BindListList {
  BindListList *bllcdr;
  SynBindList *bllcar;
};

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
typedef struct DataInit DataInit;
struct DataInit {
    DataInit *datacdr;
    int32 rpt, sort, len, val;
};

/*
 * To be used sparingly...
 */
#define h0_(p) ((p)->h0)

typedef struct {
    /* Return values from structfield ... */
    int32 woffset,   /* offset (bytes) of current field (for bitfields, */
                     /* offset of word containing current field.        */
          boffset,   /* bit offset of start of bitfield in word; or 0.  */
          bsize,     /* bit size of field - 0 if not a bitfield.        */
          typesize;  /* Byte size of field - 0 for bitfields.           */
    bool padded;     /* If padding occurred.                            */
    /* Internal fields for structfield's use - caller of structfield should
       set them to zero before the first call for a structure.
     */
    int32 n, bitoff;
    Expr *path;
} StructPos;

#endif

/* end of defs.h */
