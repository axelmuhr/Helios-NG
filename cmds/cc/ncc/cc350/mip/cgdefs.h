#ifdef USE_NORCROFT_PRAGMAS
#pragma force_top_level
#pragma include_only_once
#endif
/*
 * cgdefs.h - structures used by the back-end
 * Copyright (C) Acorn Computers Ltd., 1988.
 * Copyright (C) Codemist Ltd., 1987.
 */

/*
 * RCS $Revision: 1.1 $
 * Checkin $Date: 1992/03/23 15:00:50 $
 * Revising $Author: nickc $
 */

#ifndef _cgdefs_LOADED
#define _cgdefs_LOADED

/*
 * LabelNumbers describe compiler-generated labels (e.g. arising from if-else
 * and looping constructs). The structure of the forward references list is
 * defined and managed by target-specific local code generators. The defn field
 * is also multiply used as a jopcode location (by the code generator) and as
 * a machine-code location (by the local code generator). Source-level labels
 * also have associated LabelNumbers addressed via label binders.
 */
typedef struct LabelNumber {    /* compiler-generated-label descriptor */
  struct BlockHead *block;      /* block containing label */
  union LabRefDef {
      List *frefs;              /* forwd ref list, managed by local cg */
      int32 defn;               /* jopcode location or code location   */
  } u;
  int32 name;                   /* 'name' (internal label number) -  */
} LabelNumber;                  /* top bit indicates 'label defined' */
/*
 * Useful access and constructor functions...
 */
#define addfref_(l,v)       ((l)->u.frefs = \
                             (List *)binder_icons2((l)->u.frefs,(v)))
#define addlongfref_(l,v,i) ((l)->u.frefs = \
                             (List *)binder_icons3((l)->u.frefs,(v),(int32)(i)))
#define lab_isset_(l)       ((l)->name < 0)
#define lab_setloc_(l,v)    ((l)->name |= (unsigned long) 0x80000000, (l)->u.defn = (v))
#define lab_name_(l)        ((l)->name)
#define lab_xname_(l)       (is_exit_label(l) ? (int32)(l) : \
                             lab_name_(l) & ~(unsigned long)0x80000000)

/*
 * A list of labels numbrs is a generic List... used by local code generators
 * and by disassemblers. The mk function has the obvious sort:
 * mkLabList: LabList x LabelNumber -> LabList
 */
typedef struct LabList {
    struct LabList *labcdr;
    struct LabelNumber *labcar;
} LabList;
#define mkLabList(a,b) ((LabList *)binder_cons2(a,b))

/*
 * Structure describing a basic block.
 */
typedef struct BlockHead
{
  struct Icode *code;               /* start of 3-address code           */
  int32 length;                     /* length of ditto                   */
  int32 flags;                      /* misc flag bits (incl. exit cond)  */
  union { LabelNumber *next;            /* successor block               */
          LabelNumber **table;          /* or block table for switch     */
          struct BlockHead *backp;      /* for branch_chain() only       */
        } succ1;
  union { LabelNumber *next1;           /* alternative successor         */
          int32 tabsize;                /* or size of switch table       */
          struct BlockHead *backp1;     /* for branch_chain() only       */
        } succ2;
  struct BlockHead *down;           /* forward chaining                  */
  struct BlockHead *up;             /* backward chaining                 */
  LabelNumber *lab;                 /* label for this block              */
  struct VRegSet *use;              /* registers needed at block head    */
                                    /* (private to regalloc)             */
  union { struct BindList *l;       /* binders active at head of block   */
          int32 i;
        } stack;
  struct BindListList *debenv;      /* @@@ temp/rationalise - see flowgraph */
  struct BlockList *usedfrom;       /* list of blocks that ref. this one */
  struct CSEBlockHead *cse;
  int32  loopnest;                  /* depth of loop nesting in this blk */
} BlockHead;

typedef struct BlockList
{ struct BlockList *blklstcdr;
  BlockHead *blklstcar;
} BlockList;

#define mkBlockList(a,b) ((BlockList *)binder_cons2(a,b))

/* The following is used when a real machine register is required.       */

typedef int32 RealRegister;

#define RegSort int32    /* the following 3 values ... */

#define INTREG      0x00000000L
#define FLTREG      0x10000000L
#define DBLREG      0x20000000L
#define SENTINELREG 0x30000000L   /* for clarity in regalloc */

#ifdef ADDRESS_REG_STUFF
/* Stealing a bit for address reg. type looks alright to JPFF */
#define ADDRREG 0x08000000L
#define regname_(r) ((r) & 0x07ffffffL)
#define regtype_(r) ((r) & 0x38000000L) /* NB 29 bit register id here */
#define isintregtype_(rsort) ((rsort) == INTREG || (rsort) == ADDRREG)
#else
#define ADDRREG INTREG
#define regname_(r) ((r) & 0x0fffffffL)
/*
 * Actually when he looks at it ACN wonders if the type field was intended
 * to be 0xc0000000, or the comment should claim a 28 bit register id??
 * It would seem prudent to leave space for a few more register types in
 * case we need them in the future.
 */
#define regtype_(r) ((r) & 0x30000000L) /* NB 30 bit register id here */
#define isintregtype_(rsort) ((rsort) == INTREG)
#endif

/* The next line is convoluted so it works with R_A1 long int and        */
/* both with unsigned32 as unsigned int or unsigned long.                */
#define isarg_regnum_(r) \
    ((unsigned32)(regname_(r)-R_A1) < (unsigned32)NARGREGS) /* for cg.c only */
#define regbit(n) ((unsigned32)1<<(n))
/*
 * #define NMAGICREGS (NINTREGS+NFLTREGS)
 * (in defs.h now)
 */
/* the next lines are horrid, but less so that the previous magic numbers */
/*   - they test for virtual regs having become real - see flowgraph.c   */
#define isint_realreg_(rr) ((rr) < (unsigned32)NINTREGS)
#ifdef TARGET_SHARES_INTEGER_AND_FP_REGISTERS
#ifdef TARGET_IS_C40
#define isflt_realreg_(rr) ((rr) < (unsigned32)NFLTREGS * 2)
#define is_physical_double_reg_(r) \
   (regtype_(r)==DBLREG && (unsigned32)regname_(r) < (unsigned32)NFLTREGS * 2)
#else
#define isflt_realreg_(rr) isint_realreg_(rr)
#define is_physical_double_reg_(r) \
   (regtype_(r)==DBLREG && (unsigned32)regname_(r) < (unsigned32)NMAGICREGS)
#endif
#else /* ! TARGET_SHARES_INTEGER_AND_FP_REGISTERS */
#if NMAGICREGS == NINTREGS	/* XXX - NC 12/12/91 */
#define isflt_realreg_(rr) isint_realreg_(rr)
#else
#define isflt_realreg_(rr) \
  ((rr)-(unsigned32)NINTREGS < (unsigned32)(NMAGICREGS-NINTREGS)) /*NFLTREGS*/
#endif
#endif
#define isany_realreg_(rr) ((rr) < (unsigned32)NMAGICREGS)

#define virtreg(r,sort) ((r) | (sort))
#define V_resultreg(rsort) \
  (isintregtype_(rsort) ? virtreg(R_A1result,INTREG) : \
                          virtreg(R_FA1result, rsort))
/* The following line allows for the possibility of the function result   */
/* register being different in caller and callee (e.g. register windows). */
#ifdef __hp9000s700

/*
 * XXX - put here by NC because there appears to be a bug in the HP 9000
 * series 700 ansi C pre-processor,, whereby the INTREG argument to
 * virtreg() is NOT being substituted for its defined value.  This causes
 * the compiler to complain about an undefined variable called INTREG in
 * line 4237 of cg.c.
 *
 * ---> Does this bug affect other nested macros ???? <----
 *
 */
#define V_Presultreg(rsort) \
  (isintregtype_(rsort) ? (R_P1result  | INTREG) : \
                          (R_FP1result | rsort))
#else
#define V_Presultreg(rsort) \
  (isintregtype_(rsort) ? virtreg(R_P1result, INTREG) : \
                          virtreg(R_FP1result, rsort))
#endif

#define GAP ((VRegnum)((unsigned long)0xfff00000L))

typedef struct RegList {
    struct RegList *rlcdr;
    VRegnum rlcar;
} RegList;

#define mkRegList(a,b) ((RegList *)binder_icons2(a,b))

/* Structure for an abstract instruction (3-address code)                */

typedef union VRegInt
{
    VRegnum r;
    RealRegister rr;
    int32 i;
    char *str;
    StringSegList *s;
    LabelNumber *l;
    LabelNumber **lnn;
    Binder *b;
    Expr *e;
    FloatCon *f;
    RegList *rl;
    BindList *bl;
    Symstr *sym;
    void *p;            /* should only be used for debugger support */
} VRegInt;

typedef struct Icode
{
/*
 * The jopcode field here really contains a J_OPCODE plus contition bits
 * (Q_MASK), see jopcode.h.   Maybe there should be packed in using bitfields
 * rather than the unchecked arithmetic coding currently used?  This is not
 * the highest priority clean-up for me to worry about
 */
  int32 op;
  VRegInt r1, r2;
  VRegInt m;
} Icode;

#endif

/* end of mip/cgdefs.h */
