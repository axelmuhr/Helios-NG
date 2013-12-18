#pragma force_top_level
#pragma include_only_once
/*
 * cseguts.h: CSE: internal interfaces
 * Copyright (C) Acorn Computers Ltd., 1988.
 * Copyright (C) Advanced Risc Machines Ltd., 1991
 */

/*
 * RCS $Revision: 1.1 $
 * Checkin $Date: 1993/07/14 14:07:18 $
 * Revising $Author: nickc $
 */

#ifndef _cseguts_h
#define _cseguts_h 1

#include "regsets.h"

#define CSEDebugLevel(n) (cse_debugcount > (n))

#define E_UNARYK  0  /* unaryop const */
#define E_UNARY   1  /* unaryop Exprn */
#define E_BINARYK 2  /* Exprn binaryop const */
#define E_BINARY  3  /* Exprn binaryop Exprn */
#define E_LOAD    4  /* load value of vreg/binder/[Exprn, #offset] */
#define E_MISC    5  /* nasties.  should be none? */
#define E_CALL    6

typedef struct ExprnUse ExprnUse;
struct ExprnUse {
    ExprnUse *cdr;
    BlockHead *block;
    unsigned32 val_flags_icoden;
    /* was (but can't make that 16-bit int safe)
      struct {
        int valno: 2,
            flags: 5,
            icoden: 25;
      } ix;
    */
};

#define U_NOTDEF2 0x20
#define U_NOTREF 0x10
#define U_NOTDEF 0x08
#define U_PEEK   0x04
#define U_STORE  0x02
#define U_LOCALCSE 0x01

#define flags_(x) ((((x)->val_flags_icoden) >> 2) & 0x1f)
#define icoden_(x) (((x)->val_flags_icoden) >> 7)
#define valno_(x) (((x)->val_flags_icoden) & 0x3)
#define vfi_(v, f, i) ((((unsigned32)(i)) << 7) | ((unsigned32)(f) << 2) | (v))
#define setflag_(x, f) ((x)->val_flags_icoden |= (f) << 2)
#define setvalno_(x, v) ((x)->val_flags_icoden = ((x)->val_flags_icoden & ~3L) | (v))

#define useicode_(a) (blkcode_((a)->block)[icoden_(a)])

typedef struct LocList LocList;
typedef struct Exprn Exprn;
typedef struct Location Location;
struct Exprn {
    Exprn *cdr;                 /* hash bucket chain */
    int32  op;                  /* a jopcode */
    int32  nodeid;      /* nodeno<<5 | alias | type, see mknodeid_().   */
    ExprnUse *uses;
    union {
        struct {
            bool waslive;
            LocList *locs;      /* locations with this value */
        } a;         /* during available expression analysis */
        int32 newid; /* during renumbering */
    } v;
    union {
        struct {                /* unary operand */
            Exprn *e1;
        } unary;
        struct {
            int32  m;           /* immediate const (or binder for ADCON) */
        } unaryk;
        struct {                /* load or store some location */
            Location *loc;
        } loc;
        struct {
            Exprn *e1;
            int32  m;
        } binaryk;
        struct {
            Exprn *e1;
            Exprn *e2;
        } binary;
        struct {
            Binder *primary;
            int32  argres;      /* a jopcode argdesc, with the function result
                                   type replacing the K_FLAGS */
            Exprn *arg[1];
        } call;
    } u;
};

#define argres_setrestype_(argd, t) ((t) | (argd))
#define argres_nargs_(n) (k_intregs_(n)+k_fltregs_(n))

#define mknodeid_(nodeno, aliasandtype)   ((nodeno)<<5 | (aliasandtype))
#define exid_(e) ((e)->nodeid >> 5)
#define extype_(e) ((e)->nodeid & 0xf)
#define EX_ALIASandTYPE 0x1f
#define EX_ALIAS 0x10
#define exalias_(e) ((e)->nodeid & EX_ALIAS)
#define e1_(e) ((e)->u.binary.e1)
#define e1k_(e) ((e)->u.unaryk.m)
#define e1b_(e) ((Binder *)(e)->u.unaryk.m)
#define e2_(e) ((e)->u.binary.e2)
#define e2k_(e) ((e)->u.binaryk.m)
#define exloc_(e) ((e)->u.loc.loc)
#define exfn_(e) ((e)->u.call.primary)
#define exfntype_(e) ((e)->u.call.argres & REGSORTMASK)
#define exargdesc_(e) ((e)->u.call.argres & ~REGSORTMASK)
#define exnargs_(e) argres_nargs_((e)->u.call.argres)
#define exfargs_(e) k_fltregs_((e)->u.call.argres)
#define exiargs_(e) k_intregs_((e)->u.call.argres)
#define exnres_(e) k_resultregs_((e)->u.call.argres)
#define exarg_(e, i) ((e)->u.call.arg[i])
#define exlocs_(e) ((e)->v.a.locs)
#define exwaslive_(e) ((e)->v.a.waslive)
#define exnewid_(e) ((e)->v.newid)
#define exuses_(e) ((e)->uses)
#define exop_(e) ((e)->op)


#define is_call2(ex) ((exop_(ex) == J_CALLK || exop_(ex) == J_OPSYSK) && \
                      exnres_(ex) == 2)

typedef struct ExSet ExSet;

typedef int32 LocType;

/* LOC_xxx values can be store references (0..7) or LOC_VAR/LOC_PVAR    */
#define LOC_(MEM_x) (MEM_x)
#define LOC_VAR  8
#define LOC_PVAR 9    /* non-local or address taken */
#define LOC_anyVAR 8  /* used in masks to detect LOC_VAR/LOC_VAR.       */
#define LOC_REALBASE 16

struct Location {
    Location *cdr;
    ExSet *curvalue;  /* the set of current values or Null if unknown */
    int32  idandtype; /* id<<5 | LOC_xxxx | LOC_REALBASE, see mkidandtype_() */
    VRegSetP users;   /* the set of Exprns killed if this location is   */
                      /* stored into (those with this as a leaf).       */
    VRegSetP aliasusers; /* union over possible aliases of alias->users */
    Exprn *load;      /* the Exprn which is a load from this location.
                       * (A plain load if the location is narrow)
                       */
    union {
        struct {
            Binder *binder;
        } var;
        struct {
            int32  offset;
            Exprn  *base;
        } mem;
    } u;
};

#define SizeOfLocVar (sizeof(Location) - sizeof(Exprn *))

#define mkidandtype_(id, type) ((id) << 5 | (type))
#define locvalue_(p) ((p)->curvalue)
#define loctype_(p) ((p)->idandtype & 0x0f)
#define locrealbase_(p) ((p)->idandtype & LOC_REALBASE)
#define locid_(p)   ((p)->idandtype >> 5)
#define locreg_(p)  ((p)->u.reg.regno)
#define locbind_(p) ((p)->u.var.binder)
#define locbase_(p) ((p)->u.mem.base)
#define locoff_(p)  ((p)->u.mem.offset)
#define ispublic(p) (loctype_(p) != LOC_VAR)            /* odd name? */

typedef struct CSEDef CSEDef;
typedef struct SavedLocVals SavedLocVals;

struct CSEBlockHead {
   VRegSetP wanted, available;
   VRegSetP wantedlater;
   VRegSetP wantedonallpaths;
   VRegSetP killed;
   VRegSetP dominators;
   CSEDef *defs;
   SavedLocVals *locvals;
   char reached, killedinverted, loopempty, scanned;
};

#define blk_defs_(p) ((p)->cse->defs)
#define blk_wanted_(p) ((p)->cse->wanted)
#define blk_wantedlater_(p) ((p)->cse->wantedlater)
#define blk_wantedonallpaths_(p) ((p)->cse->wantedonallpaths)
#define blk_available_(p) ((p)->cse->available)
#define blk_killed_(p) ((p)->cse->killed)
#define blk_dominators_(p) ((p)->cse->dominators)
#define blk_reached_(p) ((p)->cse->reached)
#define blk_pred_(p) (blkusedfrom_(p))
#define blk_killedinverted_(p) ((p)->cse->killedinverted)
#define blk_scanned_(p) ((p)->cse->scanned)
#define LOOP_NONEMPTY 1
#define LOOP_EMPTY 2
#define blk_loopempty_(p) ((p)->cse->loopempty)
#define blk_locvals_(p) ((p)->cse->locvals)

#define blockkills(n, b) (!cseset_member(n, blk_killed_(b)) == (blk_killedinverted_(b)))

#define blklabname_(p) lab_name_(blklab_(p))

#define EXPRNSEGSIZE 512
#define EXPRNINDEXSIZE 64
#define EXPRNSEGBITS 9

extern Exprn **exprnindex[EXPRNINDEXSIZE];
#define exprn_(id) (exprnindex[(id)>>EXPRNSEGBITS])[(id)&(EXPRNSEGSIZE-1)]

#define CSEAlloc SynAlloc
#define CSEAllocType AT_Syn
#define CSEList3 syn_list3
#define CSEList2 syn_list2

#define cseset_insert(x, s, oldp) s = vregset_insert(x, s, oldp, &cseallocrec)
#define cseset_delete(x, s, oldp) s = vregset_delete(x, s, oldp)
#define cseset_copy(s) vregset_copy(s, &cseallocrec)
#define cseset_discard(s) vregset_discard(s)
#define cseset_equal(s1, s2) vregset_equal(s1, s2)
#define cseset_compare(s1, s2) vregset_compare(s1, s2)
#define cseset_union(s1, s2) s1 = vregset_union(s1, s2, &cseallocrec)
#define cseset_intersection(s1, s2) s1 = vregset_intersection(s1, s2)
#define cseset_difference(s1, s2) s1 = vregset_difference(s1, s2)
#define cseset_member(x, s) vregset_member(x, s)
#define cseset_map(s, f, arg) vregset_map(s, f, arg)
#define cseset_size(s) (length((List *)(s)))

extern void cse_printset(VRegSetP s);

extern VRegSetAllocRec cseallocrec;

typedef struct SetSPList SetSPList;
struct SetSPList {
    SetSPList *cdr;
    BlockHead *block;
    Icode *icode;
};

extern SetSPList *setsplist;

extern void cse_print_loc(Location *x);

extern void cse_print_node(Exprn *p);

extern void cse_printexits(int32 flags, LabelNumber *exit, LabelNumber *exit1);

extern Icode *trytokillargs(Icode *p, Icode *blockstart, bool nextinblock);

extern bool addlocalcse(Exprn *node, int valno, BlockHead *b);
/* A use of node has occurred, with a previous evaluation in the same
   basic block (b) still alive.  No decision has been made as to the
   desirability of making it into a CSE.  Return value indicates whether
   a CSE has been made.
 */
extern bool cse_AddPredecessor(LabelNumber *lab, BlockHead *b);

extern void cse_scanblocks(BlockHead *top);

extern Exprn *adconbase(Exprn *ex, bool allowvaroffsets);

extern ExprnUse *ExprnUse_New(ExprnUse *old, int flags, int valno);

extern bool killedinblock(int32 expid);

#endif
