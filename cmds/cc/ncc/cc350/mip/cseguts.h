#ifdef USE_NORCROFT_PRAGMAS
#pragma force_top_level
#pragma include_only_once
#endif
 /*
 * cseguts.h: CSE: internal interfaces
 * Copyright (C) Acorn Computers Ltd., 1988.
 */

/*
 * RCS $Revision: 1.1 $
 * Checkin $Date: 1992/03/23 15:01:24 $
 * Revising $Author: nickc $
 */

#ifndef _cseguts_h
#define _cseguts_h 1

#include "regsets.h"

#define E_UNARYK  0  /* unaryop const */
#define E_UNARY   1  /* unaryop Exprn */
#define E_BINARYK 2  /* Exprn binaryop const */
#define E_BINARY  3  /* Exprn binaryop Exprn */
#define E_LOAD    4  /* load value of vreg/binder/[Exprn, #offset] */
#define E_MISC    5  /* nasties.  should be none? */
#define E_CALL    6

typedef struct ExprnUse {
    struct ExprnUse *cdr;
    BlockHead *block;
    struct {
        int valno: 2,
            flags: 5,
            icoden: 25;
    } ix;
} ExprnUse;

#define U_NOTREF 0x10
#define U_NOTDEF 0x08
#define U_PEEK   0x04
#define U_STORE  0x02
#define U_LOCALCSE 0x01

#define flags_(x) ((x)->ix.flags)
#define icoden_(x) ((x)->ix.icoden)
#define valno_(x) ((x)->ix.valno)
#define useicode_(a) (blkcode_((a)->block)[icoden_(a)])

typedef struct LocList {
    struct LocList *cdr;
    struct Location *loc;
} LocList;

#define mkLocList(a, b) ((LocList *) syn_cons2(a, b))

typedef struct Exprn {
    struct Exprn *cdr;          /* hash bucket chain */
    int32  op;                  /* a jopcode */
    int32  nodeid; /* (nodeno << 5) | alias | type */
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
            struct Exprn *e1;
        } unary;
        struct {
            int32  m;           /* immediate const (or binder for ADCON) */
        } unaryk;
        struct {                /* load or store some location */
            struct Location *loc;
        } loc;
        struct {
            struct Exprn *e1;
            int32  m;
        } binaryk;
        struct {
            struct Exprn *e1;
            struct Exprn *e2;
        } binary;
        struct {
            Binder *primary;
            int32  nargs;       /* and result type */
            struct Exprn *arg[1];
        } call;
    } u;
} Exprn;

#define exid_(e) ((e)->nodeid >> 5)
#define extype_(e) ((e)->nodeid & 0xf)
#define EX_ALIAS 0x10
#define exalias_(e) ((e)->nodeid & EX_ALIAS)
#define e1_(e) ((e)->u.binary.e1)
#define e1k_(e) ((e)->u.unaryk.m)
#define e1b_(e) ((Binder *)(e)->u.unaryk.m)
#define e2_(e) ((e)->u.binary.e2)
#define e2k_(e) ((e)->u.binaryk.m)
#define exloc_(e) ((e)->u.loc.loc)
#define exfn_(e) ((e)->u.call.primary)
#define exfntype_(e) (regtype_((e)->u.call.nargs))
#define exnargs_(e) (regname_((e)->u.call.nargs))
#define exarg_(e, i) ((e)->u.call.arg[i])
#define exlocs_(e) ((e)->v.a.locs)
#define exwaslive_(e) ((e)->v.a.waslive)
#define exnewid_(e) ((e)->v.newid)
#define exuses_(e) ((e)->uses)
#define exop_(e) ((e)->op)

typedef int32 LocType;

#define LOC_VAR 1
#define LOC_PVAR 2    /* non-local or address taken */
#define LOC_MEM 3
#define LOC_MEMB 4
#define LOC_MEMW 5
#define LOC_MEMD 6
#define LOC_MEMF 7
#define LOC_REALBASE 8

typedef struct Location {
    struct Location *cdr;
    Exprn  *curvalue; /* the current value, or NULL if we don't know it */
    int32  idandtype; /* id<<4 | (LOC_xxxx) | LOC_REALBASE */
    VRegSetP users;   /* the set of Exprns killed if this location is stored to.
                       * (those with this as a leaf)
                       */
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
} Location;

#define SizeOfLocVar (sizeof(Location) - sizeof(Exprn *))

#define locvalue_(p) ((p)->curvalue)
#define loctype_(p) ((p)->idandtype & 0x7)
#define locrealbase_(p) ((p)->idandtype & LOC_REALBASE)
#define locid_(p)   ((p)->idandtype >> 4)
#define locreg_(p)  ((p)->u.reg.regno)
#define locbind_(p) ((p)->u.var.binder)
#define locbase_(p) ((p)->u.mem.base)
#define locoff_(p)  ((p)->u.mem.offset)
#define locpublic_(p) (loctype_(p) == LOC_PVAR)

#define publicvar_(p) (locpublic_(p))

#define CALLLOC -1 /* a fake locid to say 'has calls' */
#define ispublic(p) (loctype_(p) >= LOC_PVAR)

typedef struct CSEBlockHead {
   VRegSetP wanted, available;
   VRegSetP wantedlater;
   VRegSetP wantedonallpaths;
   VRegSetP killed;
   VRegSetP dominators;
   union {
       BlockList *predecessors;
       struct CSEDef *defs;
   } d;
   char reached, killedinverted, spare1, spare2; } CSEBlockHead;

#define blk_defs_(p) ((p)->cse->d.defs)
#define blk_wanted_(p) ((p)->cse->wanted)
#define blk_wantedlater_(p) ((p)->cse->wantedlater)
#define blk_wantedonallpaths_(p) ((p)->cse->wantedonallpaths)
#define blk_available_(p) ((p)->cse->available)
#define blk_killed_(p) ((p)->cse->killed)
#define blk_dominators_(p) ((p)->cse->dominators)
#define blk_reached_(p) ((p)->cse->reached)
#define blk_pred_(p) ((p)->cse->d.predecessors)
#define blk_killedinverted_(p) ((p)->cse->killedinverted)


#define blockkills(n, b) (!cseset_member(n, blk_killed_(b)) == (blk_killedinverted_(b)))

#define blklabname_(p) lab_name_(blklab_(p))

#define HASHSIZE 1024
#define hash(op,a,b) ( ((op) + (((int32)(a)) * 7)) & (HASHSIZE-1) )

#define EXPRNSEGSIZE 512
#define EXPRNINDEXSIZE 64
#define EXPRNSEGBITS 9

extern Exprn **exprnindex[EXPRNINDEXSIZE];
#define exprn_(id) (exprnindex[(id)>>EXPRNSEGBITS])[(id)&(EXPRNSEGSIZE-1)]

#define LOCHASHSIZE 512
#define lochash(type,a,b) ( ((type) + (int32)(a)) & (LOCHASHSIZE-1) )

#define LOCSEGSIZE 512
#define LOCINDEXSIZE 64
#define LOCSEGBITS 9

extern Location **locindex[LOCINDEXSIZE];
#define loc_(id) (locindex[(id)>>LOCSEGBITS])[(id)&(LOCSEGSIZE-1)]

#define CSEAlloc SynAlloc
#define CSEAllocType AT_Syn
#define CSEList3 syn_list3
#define CSEList2 syn_list2

#define Allocate(a) ((a *)CSEAlloc(sizeof(a)))

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
#define cseset_size(s) (length((List *)(int)(s)))

extern VRegSetAllocRec cseallocrec;

typedef struct SetSPList {
    struct SetSPList *cdr;
    BlockHead *block;
    Icode *icode;
} SetSPList;

extern SetSPList *setsplist;

#ifdef ENABLE_CSE

extern void cse_print_loc(Location *x);

extern void cse_print_node(Exprn *p);

extern void cse_printexits(int32 flags, LabelNumber *exit, LabelNumber *exit1);

#endif

extern Icode *trytokillargs(Icode *p, Icode *blockstart, bool nextinblock);

extern bool addlocalcse(Exprn *node, int valno, BlockHead *b);
/* A use of node has occurred, with a previous evaluation in the same
   basic block (b) still alive.  No decision has been made as to the
   desirability of making it into a CSE.  Return value indicates whether
   a CSE has been made.
 */

extern void cse_scanblocks(BlockHead *top);

extern Exprn *adconbase(Exprn *ex, bool allowvaroffsets);

extern ExprnUse *mkExprnUse(ExprnUse *old, int flags, int valno);

extern bool killedinblock(int32 expid);

/* These things belong to the private interface between CSE and LoopOpt,
 * and are unused elsewhere.
 */

typedef struct LoopList
{   struct LoopList *llcdr;           /* must be first for dreverse pun    */
    BlockList *llblklist;             /* list of basic blocks inside       */
    BlockHead *llblkhd;               /* where to put found invariants     */
} LoopList;

#endif
