#pragma force_top_level
#pragma include_only_once
/*
 * cgdefs.h - structures used by the back-end
 * Copyright (C) Acorn Computers Ltd., 1988-1990.
 * Copyright (C) Codemist Ltd., 1987-1992.
 * Copyright (C) Advanced RISC Machines Limited, 1991-1992.
 */

/*
 * RCS $Revision: 1.2 $ Codemist 3
 * Checkin $Date: 1993/07/27 09:47:29 $
 * Revising $Author: nickc $
 */

#ifndef _cgdefs_LOADED
#define _cgdefs_LOADED

/* see mip/defs.h for explanation of this...                            */
typedef struct BlockHead BlockHead;
typedef struct Icode Icode;
typedef struct VRegSet *VRegSetP;
typedef struct BlockList BlockList;
typedef struct CSEBlockHead CSEBlockHead;

/*
 * LabelNumbers describe compiler-generated labels (e.g. arising from if-else
 * and looping constructs). The structure of the forward references list is
 * defined and managed by target-specific local code generators. The defn field
 * is also multiply used as a jopcode location (by the code generator) and as
 * a machine-code location (by the local code generator). Source-level labels
 * also have associated LabelNumbers addressed via label binders.
 */
struct LabelNumber {            /* compiler-generated-label descriptor */
  BlockHead *block;             /* block containing label */
  union LabRefDef {
      List *frefs;              /* forwd ref list, managed by local cg */
      int32 defn;               /* jopcode location or code location   */
  } u;
  int32 name;                   /* 'name' (internal label number) -  */
};                              /* top bit indicates 'label defined' */
/*
 * Useful access and constructor functions...
 */
#define addfref_(l,v)       ((l)->u.frefs = \
                             (List *)binder_icons2((l)->u.frefs,(v)))
#define addlongfref_(l,v,i) ((l)->u.frefs = \
                             (List *)binder_icons3((l)->u.frefs,(v),(int32)(i)))
#define lab_isset_(l)       ((l)->name < 0)
#define lab_setloc_(l,v)    ((l)->name |= 0x80000000, (l)->u.defn = (v))
#define lab_name_(l)        ((l)->name)
#define lab_xname_(l)       (is_exit_label(l) ? (int32)(l) : \
                             lab_name_(l) & ~0x80000000)

/*
 * A list of labels numbrs is a generic List... used by local code generators
 * and by disassemblers. The mk function has the obvious sort:
 * mkLabList: LabList x LabelNumber -> LabList
 */
typedef struct LabList LabList;
struct LabList {
    LabList *labcdr;
    LabelNumber *labcar;
};
#define mkLabList(a,b) ((LabList *)binder_cons2(a,b))

/*
 * Structure describing a basic block.
 */
struct BlockHead
{
  struct Icode *code;               /* start of 3-address code           */
  int32 length;                     /* length of ditto                   */
  int32 flags;                      /* misc flag bits (incl. exit cond)  */
  union { LabelNumber *next;            /* successor block               */
          LabelNumber **table;          /* or block table for switch     */
          BlockHead *backp;             /* for branch_chain() only       */
        } succ1;
  union { LabelNumber *next1;           /* alternative successor         */
          int32 tabsize;                /* or size of switch table       */
          BlockHead *backp1;            /* for branch_chain() only       */
        } succ2;
  BlockHead *down;                  /* forward chaining                  */
  BlockHead *up;                    /* backward chaining                 */
  LabelNumber *lab;                 /* label for this block              */
  VRegSetP use;                     /* registers needed at block head    */
                                    /* (private to regalloc)             */
  union { BindList *l;              /* binders active at head of block   */
          int32 i;
        } stack;
  BindListList *debenv;             /* @@@ temp/rationalise - see flowgraph */
  BlockList *usedfrom;              /* list of blocks that ref. this one */
  CSEBlockHead *cse;
  int32  loopnest;                  /* depth of loop nesting in this blk */
};

struct BlockList
{ BlockList *blklstcdr;
  BlockHead *blklstcar;
};

#define mkBlockList(a,b) ((BlockList *)binder_cons2(a,b))

/* The following is used when a real machine register is required.       */

typedef int32 RealRegister;

#define RegSort int32    /* the following 3 values ... */

#define INTREG      0x10000000L
#define FLTREG      0x20000000L
#define DBLREG      0x28000000L
#define SENTINELREG 0x30000000L   /* for clarity in regalloc */

/* The following mask is used so that cse and regalloc can pack a       */
/* RegSort value and a small integer into an int32.                     */
/* Maybe neither of these are very essential anymore.                   */
#define REGSORTMASK 0xf8000000L   /* for pack/unpack of RegSort & int   */

#ifdef ADDRESS_REG_STUFF
#define ADDRREG     0x18000000L
#define isintregtype_(rsort) ((rsort) == INTREG || (rsort) == ADDRREG)
#else
#define ADDRREG INTREG
#define isintregtype_(rsort) ((rsort) == INTREG)
#endif

#define regbit(n) (((unsigned32)1L)<<(n))

/* the next lines are horrid, but less so that the previous magic numbers */
/*   - they test for virtual regs having become real - see flowgraph.c   */
/* Their uses ought to be examined and rationalised.                     */
#define isarg_regnum_(r)  ((unsigned32)((r)-R_A1) < (unsigned32)NARGREGS)
#define isany_realreg_(r)  ((unsigned32)((r)) < (unsigned32)NMAGICREGS)
#define isint_realreg_(r)  ((unsigned32)((r)) < (unsigned32)NINTREGS)
#ifdef TARGET_SHARES_INTEGER_AND_FP_REGISTERS
#define isflt_realreg_(r) isint_realreg_(r)
#define is_physical_double_reg_(r) \
   (xxregtype_(r)==DBLREG && isany_realreg_(r))
#else
#ifdef TARGET_IS_C40
#define isflt_realreg_(r) (((r) >= R_FV1) && ((r) < R_FV1 + NFLTREGS))
#else
#define isflt_realreg_(r) \
    ((unsigned32)((r))-(unsigned32)NINTREGS < (unsigned32)NFLTREGS)
#endif
#endif

#define virtreg(r,rsort) (r)             /* 'rsort' no longer used */
#define V_resultreg(rsort) \
  (isintregtype_(rsort) ? virtreg(R_A1result,INTREG) : \
                          virtreg(R_FA1result, rsort))
/* The following line allows for the possibility of the function result   */
/* register being different in caller and callee (e.g. register windows). */
#define V_Presultreg(rsort) \
  (isintregtype_(rsort) ? virtreg(R_P1result,INTREG) : \
                          virtreg(R_FP1result, rsort))

/* 'GAP' is a non-value of type VRegnum.  The bit pattern is chosen so  */
/* that it invalidates any packing with REGSORTMASK above, and often    */
/* causes a memory trap if used as an index (hence better than -1).     */
#define GAP ((VRegnum)(0xfff00000L))

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

struct Icode
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
};

#endif

/* end of mip/cgdefs.h */
