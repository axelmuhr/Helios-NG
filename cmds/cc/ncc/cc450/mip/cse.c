
/*
 * cse.c: Common sub-expression elimination
 * Copyright (C) Acorn Computers Ltd., 1988.
 * Copyright (C) Advanced Risc Machines Ltd., 1991
 */

/*
 * RCS $Revision: 1.5 $ Codemist 133
 * Checkin $Date: 1993/10/06 09:04:52 $
 * Revising $Author: nickc $
 */

#ifdef __STDC__
#  include <string.h>
#  include <time.h>
#  include <stddef.h>
#else
#  include <strings.h>
#  include "time.h"
#  include "stddef.h"
#endif

#include "globals.h"
#include "cse.h"
#include "cseguts.h"
#include "jopcode.h"
#include "store.h"
#include "regalloc.h"
#include "cg.h"
#include "flowgraf.h"
#include "mcdep.h"     /* usrdbg, DBG_xxx */
#include "builtin.h"   /* for te_xxx */
#include "errors.h"

typedef struct CSE CSE;
typedef struct CSERef CSERef;

struct CSERef {
    CSERef *cdr;
    union {
        BlockHead *b;
        ExprnUse *ex;
    } ref;
};

#define refuse_(p) ((p)->ref.ex)
#define refblock_(p) ((p)->ref.b)

struct CSEDef {
    CSEDef *cdr;
    BlockHead *block;
    Exprn *exprn;
    CSERef *refs;
    Icode *icode;
    VRegSetP uses;
    Binder *binder;
    CSEDef *subdefs, *nextsub;
    CSEDef *super;
    union {
        Binder *binder2;  /* only present for CALLK (returning 2 results) */
                          /* (> 2 results not currently cseable) */
        int32 mask;       /* only for compares */
    } extra;
};

#define CSEBind_(x) ((x)->binder)
#define CSEBind2_(x) ((x)->extra.binder2)
#define CSEDefMask_(x) ((x)->extra.mask)

struct CSE {
    CSE *cdr;
    Exprn *exprn;
    CSEDef *def;
};

static CSE *cselist;
static CSEDef *localcsedefs;

static BindList *csespillbinders;

typedef struct LoopList LoopList;
struct LoopList
{   struct LoopList *cdr;
    BlockList *members;             /* list of basic blocks inside       */
    BlockHead *preheader;           /* where to put found invariants     */
    BlockHead *header;
    BlockHead *tail;
};

static LoopList *all_loops;
static VRegSetP loopmembers;

#define mkCSEBlockList(a, b) ((BlockList *)CSEList2(a, b))

static unsigned32 nsets, newsets, setbytes;
static unsigned32 maxsets, maxbytes;
static unsigned32 cse_count, cse_refs;

VRegSetAllocRec cseallocrec; /* = {CSEAllocType, &nsets, &newsets, &setbytes};*/

SetSPList *setsplist;

#ifdef ENABLE_CSE

#define printset(s) cseset_map(s, ps, NULL)

static void ps(int32 n, VoidStar arg)
{
    IGNORE(arg);
    cc_msg(" %ld", (long)n);
}

void cse_printset(VRegSetP s) { cseset_map(s, ps, NULL); }

#else

#define cse_printset(s) /* nothing */

#endif

static int32 floatinessofreg(VRegnum r, int32 plain, int32 flt, int32 dbl)
{   switch (vregsort(r))
    {   case FLTREG: return flt;
        case DBLREG: return dbl;
        default:     return plain;
    }
}

static VRegnum newdestreg(J_OPCODE op)
{   /* Given 'op' return a new register of type suitable for the 'r1'   */
    /* field of 'op'.  If 'op' is a j_ischeck() (or J_CMP?) then        */
    /* 'GAP' is returned.                                               */
/* @@@ Note that often the old 'r1' field is available, in which case   */
/* (the slower) vregister(vregsort(r1)) is used.  Unify?                */
    /* This routine is the only user of deprecated 'floatiness_()'.     */
    /* It suggests that a new 'J_destregtype() could be in jopcode.h    */
    if (j_is_check(op))
        return GAP;
    else {
        J_OPCODE x = op & ~(J_SIGNED | J_UNSIGNED);
        switch (floatiness_(op))
        {   case _J_FLOATING: return vregister(x==J_FIXFR ? INTREG:FLTREG);
            case _J_DOUBLE:   return vregister(x==J_FIXDR ? INTREG:DBLREG);
            default:          return vregister(INTREG);
        }
    }
}

Icode *trytokillargs(Icode *p, Icode *blockstart, bool nextinblock)
{
/* Mostly, when an expression is being replaced we don't bother to do
 * anything its parts: since now they're unused register allocation
 * will kill them.  This doesn't appear to be true for function arguments,
 * though (i.e. where the destination is a physical register), and
 * particularly for floating point arguments it's a good idea to kill
 * them.  We won't necessarily be able to, since the search only goes
 * back to the head of the containing basic block.
 */
    int32 intregs = k_intregs_(p->r2.i),
          fltregs = k_fltregs_(p->r2.i),
          count = intregs+fltregs;
    VRegSetP args = NULL;
    int32 i;
    Icode *res = p;


#ifdef TARGET_IS_C40
    /*
     * XXX - NC - 29/01/92
     *
     * I have added the next line because somehow I have introduced
     * a bug into the compiler whereby this function gets called with
     * an icode whoes r2.i field has not had the K_ bits masked out,
     * (eg K_PURE, cf jopcode.h). Why this should be I do not know, but
     * this line fixes the problem for the moment.
     *
     * MEMO - this must be cleaned up
     */
    
    count &= K_ARGWORDMASK;
#endif    
    
    if (nextinblock) {
        Icode *next = p + 1;
        if ( register_movep(next->op) &&
             next->m.i == p->r1.i ) {
            p->op = J_NOOP;
            res = next;
        }
    }

    for ( i = 0 ; i < intregs; i++ )
        cseset_insert(virtreg(R_A1+i, INTREG), args, NULL);
    for ( i = 0 ; i < fltregs; i++ )
        cseset_insert(virtreg(R_FA1+i, DBLREG), args, NULL);

    while (count > 0 && (--p) >= blockstart) {
        int32 op = p->op;
        if (loads_r1(op)) {
            VRegnum r = p->r1.r;
            if (cseset_member(r, args)) {
                count--;
               /* p->op = J_NOOP;*/
                p->r1.r = vregister(vregsort(r));
                /* Register allocation should now manage to kill this, and
                   this way we needn't worry about lifting CSE definitions
                   too */
                cseset_delete(r, args, NULL);
            }
        } else if (op == J_POP) {
            RegList *rp = p->m.rl;
            int32 n = 0;
            bool killable = YES;
            for ( ; rp != NULL ; rp = rp->rlcdr)
                if (cseset_member(rp->rlcar, args)) {
                    n++; count--;
                    cseset_delete(rp->rlcar, args, NULL);
                } else
                    killable = NO;
            if (killable) {
                Icode *q = p;
                while (n > 0 && (--p) > blockstart)
                    if (p->op == J_PUSHR || p->op == J_PUSHF)
                        n--;
                    else if (p->op == J_PUSHD)
                        n -= 2;
                    else
                        break;
                /* This seems a bit excessive.  Wouldn't it be better
                   just to kill the pushes? */
                if (n == 0)
                    while (q >= p) { q->op = J_NOOP; q--; }
                else
                    p = q;
            }
        }
    }
    return res;
}

static void replacewithload(Icode *target, VRegnum r1, Binder *binder)
{
#ifdef RANGECHECK_SUPPORTED
    if (binder == NULL)
        target->op = J_NOOP;
    else
#endif
    {   target->op = floatinessofreg(r1, J_LDRV, J_LDRFV, J_LDRDV);
        target->r2.r = GAP;
        target->m.b  = binder;
    }
}

static Icode *replaceicode(ExprnUse *ref, CSEDef *def)
{
    BlockHead *b = ref->block;
    Icode *target = &useicode_(ref);
    J_OPCODE op = target->op;
    VRegnum r1 = target->r1.r;
    if (debugging(DEBUG_CSE))  {
        print_jopcode_1(op, target->r1, target->r2, target->m);
        cc_msg("     %ld/%ld\n", (long)blklabname_(b), (long)icoden_(ref));
    }
    if (op == J_CALLK && (target->r2.i & K_PURE)) { /* @@@ pure OPSYSK */
        Icode *blockend = blkcode_(b)+blklength_(b);
        target = trytokillargs(target, blkcode_(b), (target+1) < blockend);
    }
    if (def != NULL) {
        Exprn *ex = def->exprn;
#ifdef TARGET_ALLOWS_COMPARE_CSES
        if (is_compare(exop_(ex))) {
            if ((target->op & Q_MASK) != CSEDefMask_(def))
                CSEDefMask_(def) = (exop_(def->exprn) & Q_MASK) | Q_XXX;
            target->op = J_NOOP;
        } else
#endif
        if (pseudo_reads_r2(exop_(ex))) {
            VRegnum r = bindxx_(CSEBind_(def));
            if (pseudo_reads_r1(op))
                target->r1.r = r;
            else {
                target->op = exop_(ex);
                target->r2.r = r;
                note_slave(target->r1.r, r);
            }
            target->m.i = e1k_(ex);
        } else if (valno_(ref) == 0)
            replacewithload(target, r1, CSEBind_(def));
        else
            replacewithload(target, r1, CSEBind2_(def));
    }
    cse_refs++;
    return target;
}

static Binder *addcsebinder(J_OPCODE op, BindList **bl, VRegnum r)
{
    Binder *bnew;
/* The following line is a hack to fix things up now that vregsort() */
/* faults r == GAP.  Fix properly soon.                                 */
/* It is manifested by f(n) { if (n>1023) g(); if (n>1023) h(); }       */
    switch (r == GAP ? INTREG : vregsort(r)) {
        case FLTREG: bnew = gentempvar(te_float, vregister(FLTREG)); break;
        case DBLREG: bnew = gentempvar(te_double, vregister(DBLREG)); break;
#ifdef ADDRESS_REG_STUFF
        case ADDRREG:bnew = gentempvar(te_int, vregister(ADDRREG)); break;
#endif
        default:     bnew = gentempvar(te_int, vregister(INTREG)); break;
    }
    /* Two lists of binders introduced by CSE are required: one for computing
       stack frame offsets (to add to lists present before CSE in block heads
       and SETSPENVs), and one to determine binder spill order.  These are not
       the same: binders introduced for ADCONs and the like need to be in the
       second list but not the first (since they are simply discarded if they
       spill: if they were in the first list, they would consume stack to no
       purpose).
     */
    csespillbinders = mkBindList(csespillbinders, bnew);
    if (!pseudo_reads_r2(op) && !is_compare(op)) {
        *bl = mkBindList(*bl, bnew);
        bnew->bindaddr.bl = *bl;
    } else
        bnew->bindaddr.bl = NULL;
    bindstg_(bnew) |= b_bindaddrlist;
    return bnew;
}

static CSEDef *mkCSEDef(Exprn *ex, BlockHead *b)
{
    CSEDef *def;
    if (is_call2(ex)
#ifdef TARGET_ALLOWS_COMPARE_CSES
        || is_compare(exop_(ex))
#endif
       )
        def = (CSEDef *)CSEAlloc(sizeof(CSEDef));
    else
        def = (CSEDef *)CSEAlloc(offsetof(CSEDef, extra));
    def->block = b; def->icode = NULL; def->exprn = ex;
    def->refs = NULL; def->uses = NULL; CSEBind_(def) = NULL;
    def->subdefs = def->nextsub = def->super = NULL;
    return def;
}

static CSEDef *addtocselist(Exprn *ex, BlockHead *b, bool mustbenew)
{
    CSE *cse;
    CSEDef *def;
    for (cse = cselist ; cse != NULL ; cse = cdr_(cse))
        if (ex == cse->exprn) break;
    if (cse == NULL)
        cse = cselist = (CSE *)CSEList3(cselist, ex, NULL);
    if (!mustbenew)
        for (def = cse->def; def != NULL; def = cdr_(def))
            if (def->block == b) return def;

    def = mkCSEDef(ex, b);
    cdr_(def) = cse->def; cse->def = def;
    return def;
}

static bool worthreplacement(Exprn *ex)
{
/* Same rules for what's worth making into a CSE whether for local
   or non-local use.
 */
/* The following line is really unnecessary (csescan doesn't create Exprns for
   these things).
 */
    if (exop_(ex) == J_MOVDIR ||
        exop_(ex) == J_MOVLIR ||
        exop_(ex) == J_MOVFIR) return NO;
#ifdef TARGET_IS_88000  /* Experimental */
    if (exop_(ex) == J_MOVK) return NO;
#endif
    if (extype_(ex) == E_LOAD) return ispublic(exloc_(ex));
    return exop_(ex) != J_STRING && exop_(ex) != J_ADCONV;
}

bool addlocalcse(Exprn *node, int valno, BlockHead *b)
{
    ExprnUse *def = exuses_(node);
    Icode *deficode;
    CSEDef *csedef;
    CSERef *ref;

    if (!worthreplacement(node)) return NO;
    ref = (CSERef *)CSEAlloc(sizeof(CSERef));

    if (def == NULL)
        syserr(syserr_addlocalcse, exid_(node));
    /* If  node  is not killed between here and the start of the block,
       then the local CSE may be later subsumed by a non-local CSE.
       Otherwise, we must take care to avoid that happening.
       In the latter case, the CSEDef is attached to the list localcsedefs;
       in the former, to a CSE in cselist
       I believe subsuming an ADCON or MOVK is not helpful, thanks to
       slave_list, and may be harmful if the non-local CSE must be spilt
       but a register could be allocated for the local one. (Hence the
       !pseudo_reads_r2() test)
     */
    deficode = &useicode_(def);
    if (!pseudo_reads_r2(deficode->op) && !killedinblock(exid_(node))) {
        csedef = addtocselist(node, b, NO);
        cseset_insert(blklabname_(b), csedef->uses, NULL);
        if (debugging(DEBUG_CSE) && CSEDebugLevel(1))
            cc_msg("(subsumable) ");
    } else {
        for (csedef = localcsedefs ; csedef != NULL ; csedef = cdr_(csedef))
            if (csedef->icode == deficode) break;
        if (csedef == NULL) {
            csedef = mkCSEDef(node, b);
            setflag_(def, U_LOCALCSE);
            cdr_(csedef) = localcsedefs;
            localcsedefs = csedef;
        }
    }
    csedef->icode = deficode;
    cdr_(ref) = csedef->refs; csedef->refs = ref;
    refuse_(ref) = ExprnUse_New(NULL, 0, valno);
    if (debugging(DEBUG_CSE) && CSEDebugLevel(1))
        cc_msg("-- local CSE reference [%ld]\n", exid_(node));
    return YES;
}

static VRegSetP exprnswantedby(LabelNumber *q, bool allpaths)
{
    if (is_exit_label(q))
        return NULL;
    else {
        CSEBlockHead *p = q->block->cse;
        VRegSetP s = cseset_copy(allpaths ? p->wantedonallpaths :
                                            p->wantedlater);
        if (p->killedinverted)
            cseset_intersection(s, p->killed);
        else
            cseset_difference(s, p->killed);
        return cseset_union(s, p->wanted);
    }
}

static void exprnsreaching(BlockHead *p)
{
    VRegSetP s1, s2;
    if (blkflags_(p) & BLKSWITCH) {
        LabelNumber **v = blktable_(p);
        int32 i, n = blktabsize_(p);
        s1 = exprnswantedby(v[0], NO);
        s2 = exprnswantedby(v[0], YES);
        for (i = 1 ; i < n ; i++) {
            VRegSetP s = exprnswantedby(v[i], NO);
            cseset_union(s1, s);
            cseset_discard(s);
            s = exprnswantedby(v[i], YES);
            cseset_intersection(s2, s);
            cseset_discard(s);
        }
    } else {
        s1 = exprnswantedby(blknext_(p), NO);
        s2 = exprnswantedby(blknext_(p), YES);
        if (blkflags_(p) & BLK2EXIT) {
            VRegSetP s = exprnswantedby(blknext1_(p), NO);
            cseset_union(s1, s);
            cseset_discard(s);
            s = exprnswantedby(blknext1_(p), YES);
            cseset_intersection(s2, s);
            cseset_discard(s);
        }
    }
    blk_wantedlater_(p) = s1;
    blk_wantedonallpaths_(p) = s2;
}

static bool containsloadr(Exprn *p)
{ /* A temporary bodge.  Expressions containing a register whose value is
   * unknown are valid local CSEs, but not valid outside their block (because
   * before the LOADR was created, earlier loads to the register would have
   * passed unknown).  The earlier treatment of registers (where each was a
   * Location) got this right, but had undesirable space & time costs.  More
   * thought needed.  Here, we simply prevent anything with a leaf which is a
   * LOADR from becoming a candidate CSE.
   */
    if (p == NULL) return NO;
    switch (extype_(p)) {
    case E_UNARYK:
        return (exop_(p) == CSE_LOADR);
    case E_BINARY:
        if (containsloadr(e2_(p))) return YES;
    case E_BINARYK:
    case E_UNARY:
        return containsloadr(e1_(p));
    case E_LOAD:
        {   Location *loc = exloc_(p);
            if (loctype_(loc) & LOC_anyVAR) return NO;
            return containsloadr(locbase_(loc));
        }
    case E_CALL:
        {   int32 i;
            for (i = 0 ; i < exnargs_(p) ; i++)
                if (containsloadr(exarg_(p, i))) return YES;
        }
        return NO;
    default:
        return NO;
    }
}

static void addcse(int32 n, VoidStar arg)
{
    Exprn *ex = exprn_(n);
    if ( !containsloadr(ex) && worthreplacement(ex))
        addtocselist(ex, (BlockHead *)arg, YES);
}

static CSEBlockHead *new_CSEBlockHead(void)
{
    CSEBlockHead *q = (CSEBlockHead *)CSEAlloc(sizeof(CSEBlockHead));
    q->available = NULL; q->wanted = NULL;
    q->wantedlater = NULL; q->wantedonallpaths = NULL;
    q->killed = NULL; q->dominators = NULL;
    q->defs = NULL;
    q->killedinverted = NO;
    q->reached = NO;
    q->loopempty = 0;
    q->scanned = NO;
    q->locvals = NULL;
    return q;
}

#define dominates(p, q) cseset_member(blklabname_(p), blk_dominators_(q))

static bool prunesuccessors(LabelNumber *lab, VRegSetP s)
{
    if (is_exit_label(lab)) return NO;

    {   BlockHead *p = lab->block;
        VRegSetP old = blk_dominators_(p);
        VRegSetP s1 = cseset_copy(s);
        bool oldreached = blk_reached_(p), same;
        cseset_intersection(s1, old);
        cseset_insert(lab_name_(lab), s1, NULL);
        same = cseset_equal(s1, old);
        cseset_discard(old);
        blk_dominators_(p) = s1;
        blk_reached_(p) = YES;
        return !same || !oldreached;
    }
}

bool cse_AddPredecessor(LabelNumber *lab, BlockHead *p)
{
    if (!is_exit_label(lab) &&
        !member((VRegnum)p, (RegList *)blk_pred_(lab->block))) {
      blk_pred_(lab->block) = mkCSEBlockList(blk_pred_(lab->block), p);
      return YES;
    } else
      return NO;
}

static void PruneDominatorSets() {
    BlockHead *p;
    bool changed;
    do {
        changed = NO;
        for (p = top_block; p != NULL; p = blkdown_(p)) {
            VRegSetP s = blk_dominators_(p);
            if (blk_reached_(p)) {
                if (blkflags_(p) & BLKSWITCH) {
                    LabelNumber **v = blktable_(p);
                    int32 i, n = blktabsize_(p);
                    for (i=0; i<n; i++)
                        changed |= prunesuccessors(v[i], s);
                } else {
                    changed |= prunesuccessors(blknext_(p), s);
                    if (blkflags_(p) & BLK2EXIT)
                        changed |= prunesuccessors(blknext1_(p), s);
                }
            }
        }
    } while (changed);
}

static void FindDominators(void)
{
    BlockHead *p;
    {   VRegSetP allblocks = NULL;
        for (p = top_block; p != NULL; p = blkdown_(p))
            cseset_insert(blklabname_(p), allblocks, NULL);
        for (p = blkdown_(top_block); p != NULL; p = blkdown_(p))
            blk_dominators_(p) = cseset_copy(allblocks);
        cseset_discard(allblocks);
    }
    blk_reached_(top_block) = YES;
    cseset_insert(blklabname_(top_block), blk_dominators_(top_block), NULL);
    PruneDominatorSets();
    for (p = top_block; p != NULL; p = blkdown_(p)) {
        if (!blk_reached_(p)) {
            cseset_discard(blk_dominators_(p));
            blk_dominators_(p) = NULL;
        } else if (blkflags_(p) & BLKSWITCH) {
            LabelNumber **v = blktable_(p);
            int32 i, n = blktabsize_(p);
            for (i=0; i<n; i++)
                cse_AddPredecessor(v[i], p);
        } else {
            cse_AddPredecessor(blknext_(p), p);
            if (blkflags_(p) & BLK2EXIT)
                cse_AddPredecessor(blknext1_(p), p);
        }
    }
}

static void CSEFoundLoop(BlockHead *header, BlockHead *tail, BlockList *members) {
  if (debugging(DEBUG_CSE) && CSEDebugLevel(2)) {
    if (header == NULL)
      cc_msg("fake outer loop:");
    else
      cc_msg("loop %ld<-%ld:", (long)blklabname_(header), (long)blklabname_(tail));
    for (; members != NULL; members = members->blklstcdr)
      cc_msg(" %ld", (long)blklabname_(members->blklstcar));
    cc_msg("\n");
  }
}

static void mkLoopList(
    BlockList *members, BlockHead *preheader, BlockHead *header,
    BlockHead *tail, LoopList **insert) {
  LoopList *l = (LoopList *)CSEAlloc(sizeof(LoopList));
  l->members = members; l->preheader = preheader; l->header = header; l->tail = tail;
  cdr_(l) = *insert;
  *insert = l;
}

static BlockHead *makepreheader(BlockHead *pred, BlockHead *header)
{
    BlockHead *preheader = insertblockbetween(pred, header);
    preheader->cse = new_CSEBlockHead();
    blk_dominators_(preheader) = cseset_copy(blk_dominators_(header));
    blkflags_(preheader) |= BLKLOOP;
    blk_pred_(preheader) = mkCSEBlockList(NULL, pred);
    {   BlockList *prevblks = blk_pred_(header);
        for (; prevblks != NULL; prevblks = prevblks->blklstcdr)
            if (prevblks->blklstcar == pred) {
                prevblks->blklstcar = preheader;
                break;
            }
    }
    {   /* Must add the preheader to domintor sets now, or we will fail to
           find a place to insert a preheader for another loop with the same
           header.
         */
        BlockHead *b = top_block;
        int32 labno = blklabname_(preheader);
        for (; b != NULL; b = blkdown_(b))
            if (dominates(header, b))
                cseset_insert(labno, blk_dominators_(b), NULL);
        cseset_delete(blklabname_(header), blk_dominators_(preheader), NULL);
    }
    return preheader;
}

static bool LoopEmptyBlock(BlockHead *b)
{
   /* for loop optimisation's purposes, we may ignore blocks which
      can't interfere (this means they neither reference nor kill
      any Exprn, but since loop analysis precedes cse_scanblock the simple
         !(blk_wanted_(b) == NULL && blk_killed_(b) == NULL)
      isn't available.  Instead, we use a weaker approximation.
    */
    int i;
    if (blkflags_(b) & BLK2EXIT) return NO;
    if (blk_loopempty_(b) != LOOP_EMPTY) {
        for (i = 0; i < blklength_(b); i++)
            if (blkcode_(b)[i].op != J_SETSPGOTO) return NO;
        blk_loopempty_(b) = LOOP_EMPTY;
    }
    return YES;
}

typedef struct LLL_Loop {
    struct LLL_Loop *cdr;
    BlockList *members;             /* list of basic blocks inside       */
    BlockHead *tail;
    VRegSetP memberset;
} LLL_Loop;

typedef struct LoopListList {
    struct LoopListList *cdr;
    BlockHead *header;
    LLL_Loop *loops;
} LoopListList;

static LoopListList *looplists;

static void addloop(BlockHead *p, LabelNumber *q)
{
    /* The flowgraph contains an arc from p to q: if q dominates p,
       this means there is a loop with header q
     */
    if (!is_exit_label(q) && dominates(q->block, p)) {
        BlockHead *header = q->block;
        BlockList *bl = mkCSEBlockList(NULL, p);
        BlockList *members = mkCSEBlockList(NULL, header);
        LoopListList **ll = &looplists;
        LoopListList *l;
        while (bl != NULL) {
            BlockHead *b = bl->blklstcar;
            bl = bl->blklstcdr;
            if (!member((VRegnum)b, (RegList *)members)) {
                BlockList *pred = blk_pred_(b);
                members = mkCSEBlockList(members, b);
                for (; pred != NULL; pred = pred->blklstcdr)
                    bl = mkCSEBlockList(bl, pred->blklstcar);
            }
        }
        for (; (l = *ll) != NULL; ll = &cdr_(l))
            if (l->header == header) break;

        CSEFoundLoop(header, p, members);

        if (l == NULL) *ll = l = (LoopListList *) CSEList3(NULL, header, NULL);

        {   LLL_Loop *newl = (LLL_Loop *) CSEAlloc(sizeof(*newl));
            cdr_(newl) = l->loops; newl-> members = members; newl->tail = p;
            newl->memberset = NULL;
            l->loops = newl;
        }
    }
}

static BlockHead *AddBlockBeforeHeader(BlockHead *header, char *s) {
  BlockHead *b = NULL;
  BlockList *bl, *prev = NULL;
  LabelNumber *blab = NULL;
  for (bl = blk_pred_(header); bl != NULL; prev = bl, bl = bl->blklstcdr)
    if (!dominates(header, bl->blklstcar)) {
      BlockHead *pred = bl->blklstcar;
      if (debugging(DEBUG_CSE) && CSEDebugLevel(2)) {
        cc_msg("insert %s between %ld and %ld: ",
               s,
               (long)lab_name_(blklab_(pred)),
               (long)lab_name_(blklab_(header)));
      }
      if (b == NULL) {
        b = makepreheader(pred, header);
        bl->blklstcar = b;
        blab = blklab_(b);
        if (debugging(DEBUG_CSE) && CSEDebugLevel(2))
          cc_msg("%s = %ld\n", s, (long)blklabname_(b));
      } else {
        changesuccessors(pred, blab, blklab_(header));
        prev->blklstcdr = bl->blklstcdr;
        bl = prev;
      }
      cse_AddPredecessor(blab, pred);
    }
  return b;
}

static void AddBlockToLoopsContaining(BlockHead *b, BlockHead *newbh) {
/* Add newbh to the members of any loop containing b (except those for which
   it is the header)
 */
  LoopListList *x;
  LLL_Loop *l;
  for (x = looplists; x != NULL; x = cdr_(x))
    if (x->header != b)
      for (l = x->loops; l != NULL; l = cdr_(l)) {
        BlockList *members = l->members;
        for (; members != NULL; members = members->blklstcdr)
          if (members->blklstcar == b)
            members->blklstcdr = mkCSEBlockList(members->blklstcdr, newbh);
      }
}

typedef struct LoopSet {
    struct LoopSet *cdr;
    LLL_Loop *loops;
    VRegSetP xn, un;
} LoopSet;

static void InsertPreheaders(void) {
  LoopListList *ll;
  LLL_Loop *l, *nextl;
  for (ll = looplists; ll != NULL; ll = cdr_(ll))
    if (cdr_(ll->loops) != NULL) {
      /* If there is more than one loop with the same header, convert
         the member blocklist for each loop into a VRegSet in which
         blocks which are empty as far as loop analysis is concerned
         are ignored. (We do this to make comparison easier).
       */
      LoopSet *loopsets = NULL;
      for (l = ll->loops; l != NULL; l = cdr_(l)) {
        BlockList *m  = l->members;
        for (; m != NULL; m = m->blklstcdr) {
          BlockHead *b = m->blklstcar;
          if (blk_loopempty_(b) != LOOP_NONEMPTY) {
            if (LoopEmptyBlock(b)) continue;
            blk_loopempty_(b) = LOOP_NONEMPTY;
          }
          l->memberset = cseset_insert(blklabname_(b), l->memberset, NULL);
        }
      }
      /* Now allocate the loops to sets ordered by inclusion of loop
         member sets.
       */
      for (l = ll->loops; l != NULL; l = nextl) {
        LoopSet *ls, **lsp = &loopsets;
        nextl = cdr_(l);
        for (; (ls = *lsp) != NULL; lsp = &cdr_(ls)) {
          int order = cseset_compare(l->memberset, ls->xn);
          if (order == VR_SUBSET) break;
          order = cseset_compare(l->memberset, ls->un);
          if (order != VR_SUPERSET) {
            cdr_(l) = ls->loops; ls->loops = l;
            ls->un = cseset_union(ls->un, l->memberset);
            ls->xn = cseset_intersection(ls->xn, l->memberset);
            while (order == VR_UNORDERED) {
              LoopSet *nexts = cdr_(ls);
              if (nexts == NULL ||
                  cseset_compare(l->memberset, nexts->xn) == VR_SUBSET)
                goto nextloop;
              cdr_(ls) = cdr_(nexts);
              ls->un = cseset_union(ls->un, nexts->un);
              { LLL_Loop *nxl, *xl = nexts->loops;
                for (; xl != NULL; xl = nxl) {
                  nxl = cdr_(xl);
                  cdr_(xl) = ls->loops; ls->loops = xl;
                }
              }
              order = cseset_compare(l->memberset, nexts->un);
            }
            goto nextloop;
          }
        }
        ls = (LoopSet *) CSEAlloc(sizeof(LoopSet));
        cdr_(ls) = *lsp; ls->loops = l; cdr_(l) = NULL;
        ls->un = cseset_copy(l->memberset);
        ls->xn = cseset_copy(l->memberset);
        *lsp = ls;
nextloop:;
      }
      /* for each set of loops but the first, invent a new header and insert
         it between the original header and its predecessor
       */
      if (debugging(DEBUG_CSE) && CSEDebugLevel(2)) {
        LoopSet *ls;
        cc_msg("loop sets (header %ld): ", (long)blklabname_(ll->header));
        for (ls = loopsets; ls != NULL; ls = cdr_(ls)) {
          int c = '{';
          LLL_Loop *l = ls->loops;
          for (; l != NULL; l = cdr_(l)) {
            cc_msg("%c%ld", c, (long)blklabname_(l->tail));
            c = ' ';
          }
          if (cdr_(ls) == NULL)
            cc_msg("}\n");
          else
            cc_msg("}, ");
        }
      }
      if (cdr_(loopsets) != NULL) {
        BlockHead *header = ll->header;
        /* we want to deal with the loops largest first */
        LoopSet *ls = (LoopSet *)dreverse((List *)cdr_(loopsets));
        ll->loops = loopsets->loops;
        for (; ls != NULL; ls = cdr_(ls)) {
          BlockHead *newheader = AddBlockBeforeHeader(header, "new header");
          LLL_Loop *l;
          LoopListList *n;
          blkflags_(newheader) &= ~BLKLOOP;
          for (l = ls->loops; l != NULL; l = cdr_(l))
            changesuccessors(l->tail, blklab_(newheader), blklab_(header));
          n = (LoopListList *) CSEAlloc(sizeof(*n));
          cdr_(n) = cdr_(ll);
          n->header = newheader; n->loops = ls->loops;
          cdr_(ll) = n;
          AddBlockToLoopsContaining(header, newheader);
          ll = n;
        }
      }
    }

  for (ll = looplists; ll != NULL; ll = cdr_(ll)) {
    BlockHead *header = ll->header;
    BlockHead *preheader = AddBlockBeforeHeader(header, "preheader");
    AddBlockToLoopsContaining(header, preheader);
    for (l = ll->loops; l != NULL; l = cdr_(l))
      mkLoopList(l->members, preheader, header, l->tail, &all_loops);
  }
}

static void findloops(void)
{
    BlockHead *p;
    BlockList *allbuttop = NULL;
    looplists = NULL;
    for (p=top_block; p != NULL; p = blkdown_(p))
        if (blkflags_(p) & BLKSWITCH) {
            LabelNumber **v = blktable_(p);
            int32 i, n = blktabsize_(p);
            for (i=0; i<n; i++)
                addloop(p, v[i]);
        } else {
            addloop(p, blknext_(p));
            if (blkflags_(p) & BLK2EXIT)
                addloop(p, blknext1_(p));
    }
    InsertPreheaders();
    for (p = blkdown_(top_block); p != NULL; p = blkdown_(p))
        allbuttop = mkCSEBlockList(allbuttop, p);

    {   /* Since we no longer necessarily lift expressions from the fake outer
           loop into its preheader, we need to know which blocks are inside
           real loops (to avoid lifting into such blocks).
         */
        LoopList *lp;
        loopmembers = NULL;
        for (lp = all_loops; lp != NULL; lp = cdr_(lp)) {
           BlockList *m = lp->members;
           for (; m != NULL; m = m->blklstcdr) {
               cseset_insert(blklabname_(m->blklstcar), loopmembers, NULL);
           }
        }
        /* @@@ This implies that the loop preheader for a loop is not included
               in the members of a containing loop (or it's pointless) : I
               believe this no longer to be the case.
         */
        for (lp = all_loops; lp != NULL; lp = cdr_(lp))
            if (cseset_member(blklabname_(blkup_(lp->preheader)), loopmembers))
                cseset_insert(blklabname_(lp->preheader), loopmembers, NULL);
    }

    /* Add a spurious whole function loop */
    {   BlockHead *h = makepreheader(top_block, blkdown_(top_block));
        CSEFoundLoop(NULL, NULL, allbuttop);
        mkLoopList(allbuttop, h, NULL, NULL, &all_loops);
        blkflags_(h) |= BLKOUTER;
    }
}

static bool prunereached(BlockHead *defblock, LabelNumber *lab)
{
    if (is_exit_label(lab)) return NO;
    {   BlockHead *b = lab->block;
        if (!blk_reached_(b)) return NO;
        if (defblock == b) return NO;
        blk_reached_(b) = NO;
        return YES;
    }
}

static void FindRefIcodes(CSEDef *def, ExprnUse *uses)
{
    BlockHead *defblock = def->block;
    ExprnUse *defuse;
    CSERef *l, **prevp;
    /* There may be many occurrences of this expression in the block
     * defining it (because it is killed between them): we want the last,
     * which since the list is reversed we come to first
     */
    for (defuse = uses ; defuse != NULL ; defuse = cdr_(defuse))
        if (defuse->block == defblock) break;
    if (defuse == NULL)
        if (blklength_(defblock) != 0)
            syserr(syserr_cse_lost_def);
        else  /* must be an extracted loop invariant */
            def->icode = NULL;
    else
        def->icode = &useicode_(defuse);
    for (l = def->refs, prevp = &def->refs ; l != NULL ; l = cdr_(l)) {
        /* Again, there may be many references: we want the first in the block
         * (which we come to last): for all the others, the expression will
         * have a different value.
         */
        ExprnUse *first = NULL;
        ExprnUse *use;
        for (use = uses ; use != NULL ; use = cdr_(use))
            if (use->block == refblock_(l)) first = use;
        if (first == NULL)
            syserr(syserr_cse_lost_use);
        if (flags_(first) & U_NOTREF) {
            *prevp = cdr_(l);
            if (debugging(DEBUG_CSE))
                cc_msg(" (%ld notref)", (long)blklabname_(refblock_(l)));
        } else {
            refuse_(l) = first;
            prevp = &cdr_(l);
        }
    }
}

static void DiscardDef(CSEDef *discard, CSEDef *keep)
{
    cseset_discard(discard->uses);
    discard->uses = NULL;
    while (discard->refs != NULL) {
        CSERef *p;
        ExprnUse *use = refuse_(discard->refs);
        for (p = keep->refs; p != NULL; p = cdr_(p))
            if (refuse_(p) == use) break;
        if (p == NULL)
            keep->refs = (CSERef *)CSEList2(keep->refs, use);
        discard->refs = (CSERef *)discard2((List *)discard->refs);
    }
}

static void RemoveSubRefs(CSEDef *sub, CSEDef *super) {
    CSERef *subref;
    for (subref = sub->refs; subref != NULL; subref = cdr_(subref)) {
        CSERef *p, **pp;
        ExprnUse *use = refuse_(subref);
        for (pp = &super->refs; (p = *pp) != NULL; pp = &cdr_(p))
            if (refuse_(p) == use) {
                *pp = (CSERef *)discard2((List *)p);
                break;
            }
    }
}

static void MakeSubdef(CSEDef *sub, CSEDef *super)
{ /* Add sub to the loopinv chain of super.
     Remove the refs to super which are also refs to sub.
     Do not alter super->uses, or its use in ordering defs will be
     defeated (it has no other use).
   */
    if (sub->super != NULL) {
        CSEDef *p, **pp = &sub->super->subdefs;
        for (; (p = *pp) != sub; pp = &p->nextsub)
            if (p == NULL) syserr("MakeSubDef");
        *pp = sub->nextsub;
    }
    sub->super = super;
    sub->nextsub = super->subdefs; super->subdefs = sub;

    RemoveSubRefs(sub, super);
}

static void LinkRefs(CSE *cse, CSEDef *def)
{ /* For the definition  def  of  cse, find the uses of it (blocks for which
   * it is wanted) which the definition must reach.
   */
    BlockHead *defblock = def->block;
    bool local;
    if (def->uses != NULL) {
    /* Non-null def->uses here means a local CSE which may be subsumed */
        if (debugging(DEBUG_CSE))
            cc_msg("  %ld local", (long)blklabname_(defblock));
        local = YES;
    } else {
        int32 exid = exid_(cse->exprn);
        int32 defnest = blknest_(defblock);
        BlockHead *p;
        bool changed;
        if (debugging(DEBUG_CSE))
            cc_msg("  %ld r", (long)blklabname_(defblock));
        for (p = top_block; p != NULL; p = blkdown_(p))
            blk_reached_(p) = dominates(defblock, p);
        if (extype_(cse->exprn) != E_UNARYK) {
        /* Constants can't be killed, so they'll reach everything dominated
           by the block containing their definition.
         */
            do {
                changed = NO;
                for (p=top_block; p!=NULL; p = blkdown_(p)) {
                    if ( (!blk_reached_(p) && blk_dominators_(p) != NULL) ||
                                              /* (that is, not unreachable) */
                         (p != defblock && blockkills(exid, p)) ) {

                        if (blkflags_(p) & BLKSWITCH) {
                            LabelNumber **v = blktable_(p);
                            int32 i, n = blktabsize_(p);
                            for (i=0; i<n; i++)
                                changed |= prunereached(defblock, v[i]);
                        } else {
                            changed |= prunereached(defblock, blknext_(p));
                            if (blkflags_(p) & BLK2EXIT)
                                changed |= prunereached(defblock, blknext1_(p));
                        }
                    }
                }
            } while (changed);
        }
        for (p = top_block; p != NULL; p = blkdown_(p)) {
            if (blk_reached_(p) && p != defblock) {
                cseset_insert(blklabname_(p), def->uses, NULL);
                if ( cseset_member(exid, blk_wanted_(p)) &&
                     blknest_(p) >= defnest
                 /* this isn't exactly right; it should test that p is inside
                    all loops which defblock is
                  */
                    ) {
                    def->refs = (CSERef *) syn_cons2(def->refs, p);
                    if (debugging(DEBUG_CSE))
                        cc_msg(" %ld", (long)blklabname_(p));
                }
            }
        }
        local = NO;
    }

    if (def->refs != NULL) {
        /* if this isn't a local def, def->refs is a list of blocks; change it
           now to a list of ExprnUse.  (which it already is for local defs)
         */
        if (!local)
            FindRefIcodes(def, exuses_(cse->exprn));

        if ( (blkflags_(defblock) & BLKOUTER) &&
             ( def->refs == NULL || /* findreficodes may have discarded some */
               cdr_(def->refs) == NULL) ) {
            if (debugging(DEBUG_CSE)) cc_msg(": unwanted outer loop inv");
        } else if (def->refs != NULL) {
            CSEDef **pp, *p;
            bool discard = NO;
            for (pp = &cse->def ; (p = *pp) != NULL ; pp = &cdr_(p)) {
                int cf = cseset_compare(def->uses, p->uses);
                /* Equality can only happen here if one def is local */
                if (cf == VR_SUBSET ||
                           (cf == VR_EQUAL && local)) {
                    if (pseudo_reads_r2(exop_(cse->exprn))) {
                        MakeSubdef(def, p);
                        if (debugging(DEBUG_CSE))
                            cc_msg(": subdef of %ld", (long)blklabname_(p->block));
                    } else {
                        DiscardDef(def, p);
                        if (debugging(DEBUG_CSE))
                            cc_msg(": killed (%ld)", (long)blklabname_(p->block));
                        discard = YES;
                    }
                    break;
                } else if (cf == VR_SUPERSET || cf == VR_EQUAL) {
                    if (pseudo_reads_r2(exop_(cse->exprn))) {
                        if ( p->super != NULL &&
                             cseset_compare(def->uses, p->super->uses) == VR_SUPERSET)
                            RemoveSubRefs(p, def);
                        else {
                            MakeSubdef(p, def);
                            if (debugging(DEBUG_CSE))
                                cc_msg(": has subdef %ld", (long)blklabname_(p->block));
                        }
                    } else {
                        DiscardDef(p, def);
                        if (debugging(DEBUG_CSE))
                            cc_msg(": kills %ld", (long)blklabname_(p->block));
                    }
                }
            }
            if (!discard) {
                cdr_(def) = p;
                *pp = def;
            }
        }
    }
    if (debugging(DEBUG_CSE)) cc_msg("\n");
}

static void LinkRefsToDefs(void)
{
    CSE *cse;
    for ( cse = cselist ; cse != NULL ; cse = cdr_(cse) ) {
        CSEDef *def = cse->def, *next;
        cse->def = NULL;
        if (debugging(DEBUG_CSE))
            cse_print_node(cse->exprn);
        for (; def != NULL ; def = next) {
        /* There used to be code here to discard this def if it was a ref of
           one previously encountered, (to save work, since LinkRefs() would
           anyway discard it).  Now we want to do something more complicated,
           the check has been retired.
           We remove def from the list, linkrefs will add it in the appropriate
           place (sorted subdefs first)
         */
            next = cdr_(def);
            LinkRefs(cse, def);
        }
        if (debugging(DEBUG_CSE)) {
            cc_msg(" sorted:\n");
            for (def = cse->def; def != NULL ; def = cdr_(def)) {
                CSERef *ref = def->refs;
                cc_msg("  %ld:", blklabname_(def->block));
                for (; ref != NULL; ref = cdr_(ref))
                    cc_msg(" %ld", blklabname_(refuse_(ref)->block));
                if (def->subdefs != NULL) {
                    CSEDef *sub = def->subdefs;
                    char *s = " <";
                    for (; sub != NULL; sub = sub->nextsub) {
                        cc_msg("%s%ld", s, blklabname_(sub->block));
                        s = " ";
                    }
                    cc_msg(">");
                }
                cc_msg("\n");
            }
        }
    }
}

static bool safetolift(Exprn *ex)
{
    J_OPCODE op = exop_(ex);
    switch (extype_(ex)) {
    case E_UNARYK:
        return YES;
    case E_BINARY:
        if (op == J_CHKUR || op == J_CHKLR ||
            op == J_CHKNEFR || op == J_CHKNEDR)
            return NO;
        if ((op & ~(J_UNSIGNED|J_SIGNED)) == J_DIVR ||
            (op & ~(J_UNSIGNED|J_SIGNED)) == J_REMR ||
            op == J_DIVFR || op == J_DIVDR)
            return NO;
        /* what about other floating pt ops ? */
        return (safetolift(e2_(ex)) && safetolift(e1_(ex)));

    case E_BINARYK:
        if (op == J_CHKUK || op == J_CHKLK || op == J_CHKNEK)
            return NO;
        if (( (op & ~(J_UNSIGNED|J_SIGNED)) == J_DIVK ||
              (op & ~(J_UNSIGNED|J_SIGNED)) == J_REMK) &&
            e2k_(ex) == 0)
            return NO;
        /* drop through - what about floating pt ? */

    case E_UNARY:
        return safetolift(e1_(ex));

    default:
    case E_MISC:
        syserr(syserr_safetolift);

    case E_CALL:
        return NO;

    case E_LOAD:
        {   Location *loc = exloc_(ex);
            if (loctype_(loc) & LOC_anyVAR) return YES;
            return (adconbase(locbase_(loc), NO) != NULL);
        }
    }
}

static void addifsafe(int32 n, VoidStar arg)
{
    if (safetolift(exprn_(n))) addcse(n, arg);
}

static void findloopinvariants(void)
{
    LoopList *p;
    for (p = all_loops ; p != NULL ; p = cdr_(p)) {
        BlockHead *b = p->preheader;
        BlockList *bl;
        VRegSetP w = cseset_copy(blk_wantedlater_(b));
        VRegSetP we = cseset_copy(blk_wantedonallpaths_(b));
        VRegSetP u = NULL;
        for (bl = p->members ; bl != NULL ; bl = bl->blklstcdr) {
            BlockHead *c = bl->blklstcar;
            cseset_union(u, blk_wanted_(c));
            if (blk_killedinverted_(c)) {
                cseset_intersection(w, blk_killed_(c));
                cseset_intersection(we, blk_killed_(c));
            } else {
                cseset_difference(w, blk_killed_(c));
                cseset_difference(we, blk_killed_(c));
            }
        }
        cseset_difference(w, we);
        cseset_intersection(w, u);
        cseset_intersection(we, u);
        cseset_discard(u);
        if (debugging(DEBUG_CSE)) {
            cc_msg("Loop L%ld:", (long)blklabname_(b));
            for (bl = p->members ; bl != NULL ; bl = bl->blklstcdr)
                cc_msg(" %ld", (long)blklabname_(bl->blklstcar));
            cc_msg(": safe"); cse_printset(we);
            cc_msg("; unsafe"); cse_printset(w);
            cc_msg("\n");
        }
        cseset_map(we, addcse, (VoidStar)b);
        cseset_map(w, addifsafe, (VoidStar)b);
    }
}

static BindList *nconcbl(BindList *l, BindList *bl)
{
/* Returns its first argument, adjusted to have as its tail the BindList bl
   if it didn't on entry (Bindlists in the places being adjusted share tails,
   so after the first has been adjusted many others will already be right).
 */
    BindList *p = l;
    BindList *prev = NULL;
    for ( ; p != NULL ; prev = p, p = p->bindlistcdr )
        if (p == bl) return l;
    if (prev == NULL) return bl;
    prev->bindlistcdr = bl;
    return l;
}

static Icode *storecse2(Icode *newic, VRegnum r1, Binder *binder)
{
    newic = newic+1;
    newic->op = floatinessofreg(r1, J_STRV, J_STRFV, J_STRDV);
    newic->r1.r = r1;
    newic->r2.r = GAP;
    newic->m.b = binder;
    return newic;
}

static Icode *storecse(Icode *newic, CSEDef *def)
{ /*  ADCONs etc get treated specially here, in that rather than store
      into the CSE binder, we use an ADCON with the register for the CSE
      binder as r1.  This is so that, if the binder gets spilled, the ADCON
      is voided.  (The load of the CSE register is inserted before the
      original instruction, so that can be turned into a use of the CSE
      in case it isn't spilled).
   */
    VRegnum r1 = newic->r1.r;
    if (is_call2(def->exprn)) {
        VRegnum r2; Icode *call = newic-1;
        if ((newic->op == J_CALLK && k_resultregs_(newic->r2.i) > 1) || /* @@@ pure OPSYSK */
            (call->op == J_CALLK && k_resultregs_(call->r2.i) > 1))
        /* First case for lifting out of loop; second normally */
            r2 = virtreg(R_A1+1, INTREG), r1 = virtreg(R_A1, INTREG);
        else
        /* Local CSE def which is a non-local ref */
        /* @@@ presumably, this should never happen now */
            r2 = call->r1.r;
        newic = storecse2(newic, r2, CSEBind2_(def));
    }
#ifdef TARGET_ALLOWS_COMPARE_CSES
    if (is_compare(exop_(def->exprn))) {
        newic->op = (newic->op & ~Q_MASK) | CSEDefMask_(def);
        blkflags_(def->block) |= BLKCCEXPORTED;
        return newic;
    }
#endif

    if (j_is_check(exop_(def->exprn)))
        return newic;

    if (CSEBind_(def) == NULL)
        syserr(syserr_storecse, (long)blklabname_(def->block),
                                (long)exid_(def->exprn));
    if (pseudo_reads_r2(newic->op)) {
        VRegnum b = bindxx_(CSEBind_(def));
        Icode *ic = newic+1;
        *ic = *newic;
        ic->r2.r = newic->r1.r = b;
        if (newic->r2.r != GAP) {
            forget_slave(ic->r1.r, newic->r2.r);
            note_slave(b, newic->r2.r);
        }
        note_slave(ic->r1.r, b);
        return ic;
    } else if (pseudo_reads_r1(newic->op)) {
        VRegnum b = bindxx_(CSEBind_(def));
        Icode *ic = newic+1;
        *ic = *newic;
        newic->op = exop_(def->exprn);
        newic->r2.r = GAP;
        ic->r1.r = newic->r1.r = b;
        return ic;
    } else
        return storecse2(newic, r1, CSEBind_(def));
}

static int32 defsize(CSEDef *defs)
{
    int32 ndefs = 0;
    for (; defs != NULL; defs = cdr_(defs))
        if (is_call2(defs->exprn))
            ndefs += 2;
        else
        if (!is_compare(exop_(defs->exprn))
            && !j_is_check(exop_(defs->exprn)))
            ndefs++;
    cse_count += ndefs;
    return ndefs;
}

typedef struct CopyList CopyList;
typedef struct CopyCSE CopyCSE;
typedef struct CopyListList CopyListList;

struct CopyList {
    CopyList *cdr;
    Icode  icode;
    CopyCSE *cse;
    Exprn  *exprn;
};

struct CopyListList {
    CopyListList *cdr;
    CopyList *p;
};

typedef struct CSEDefList CSEDefList;

struct CSEDefList {
    CSEDefList *cdr;
    CSEDef *def;
};

struct CopyCSE {
    CopyCSE *cdr;
    CopyList *def;
    CopyListList *refs;
    CSEDefList *csedefs;
};

static CopyCSE *addeddefs;

static CopyList *copylist(
    CopyList *cl, Exprn *exprn, J_OPCODE op, VRegnum r1, VRegnum r2, VRegInt m)
{
    CopyList *q = (CopyList *)CSEAlloc(sizeof(CopyList));
    cdr_(q) = cl; q->cse = NULL; q->exprn = exprn;
    q->icode.op = op; q->icode.r1.r = r1; q->icode.r2.r = r2; q->icode.m = m;
    return q;
}

static CopyList *AddExprnsBelow(
    CopyList *cl, Exprn *exprn, CopyList **clp, int32 *callcount);

static VRegnum aeb_argumentreg(CopyList **cl, Exprn *e, int32 *callcount)
{
    CopyList *arg;
    *cl = AddExprnsBelow(*cl, e, &arg, callcount);
    if (arg->cse == NULL)
        return arg->icode.r1.r;
    else {
        VRegnum oldr = arg->icode.r1.r,
                newr = vregister(vregsort(oldr));
        if (pseudo_reads_r2(exop_(e)))
            *cl = copylist(*cl, NULL,
                           arg->icode.op,
                           newr, GAP, arg->icode.m);
        else {
            VRegInt m;
            m.i = 0;
            *cl = copylist(*cl, NULL,
                           floatinessofreg(oldr, J_LDRV, J_LDRFV, J_LDRDV),
                           newr, GAP, m);
        }
        arg->cse->refs = (CopyListList*)CSEList2(arg->cse->refs, *cl);
        return newr;
    }
}

static CopyCSE *mk_CopyCSE(CopyList *p)
{
    CopyCSE *q = (CopyCSE *)CSEAlloc(sizeof(CopyCSE));
    cdr_(q) = addeddefs; q->def = p;
    q->refs = NULL; q->csedefs = NULL;
    addeddefs = q;
    return q;
}

static CopyList *AddExprnsBelow(
    CopyList *cl, Exprn *exprn, CopyList **clp, int32 *callcount)
{
    CopyList *p;
    for (p = cl ; p != NULL ; p = cdr_(p))
        if (p->exprn == exprn) {
            if (p->cse == NULL) p->cse = mk_CopyCSE(p);
            break;
        }

    if (p == NULL) {
        CopyList *arg;
        J_OPCODE op = exop_(exprn);
        VRegnum r1 = newdestreg(op);
        VRegnum r2 = GAP;
        VRegInt m;
        m.i = 0;
        switch (extype_(exprn)) {
        case E_UNARYK:
            m.i = e1k_(exprn);
            break;
        case E_UNARY:
            m.r = aeb_argumentreg(&cl, e1_(exprn), callcount);
            break;
        case E_BINARYK:
            r2 = aeb_argumentreg(&cl, e1_(exprn), callcount);
            m.i = e2k_(exprn);
            break;
        case E_BINARY:
            r2 = aeb_argumentreg(&cl, e1_(exprn), callcount);
            m.r = aeb_argumentreg(&cl, e2_(exprn), callcount);
            break;
        case E_LOAD:
            {   /* Loads need greater care, because they have been transformed
                   into compute address; ldrk  but transforming back is subject
                   to constraints I'd rather not know about.
                   (eg on ARM  ldrr<<2 r1, r2, r3 but
                               addr<<2 r1, r2, r3; ldrfk f1, r1, 0)
                   To sidestep this problem, transformed loads have been marked
                   as such and must be transformed back here.
                 */
                Location *loc = exloc_(exprn);
                if (loctype_(loc) & LOC_anyVAR)
                    m.b = locbind_(loc), op = J_KtoV(op);
                else
                {   Exprn *base = locbase_(loc);
                        if (locrealbase_(loc)) {
                          /* an untransformed load: must be copied as it stands */
                            r2 = aeb_argumentreg(&cl, base, callcount);
                            m.i = locoff_(loc);
                        } else {
                          /* base must be one of ADDR, SUBR or ADCONV */
                            J_OPCODE baseop = exop_(base);
#ifdef TARGET_HAS_SCALED_ADDRESSING
                            switch (baseop & ~J_SHIFTMASK) {
                            case J_SUBR:
                                op |= J_NEGINDEX;
                            case J_ADDR:
                                op |= (baseop & J_SHIFTMASK);
#else
                            switch (baseop) {
                            case J_SUBR:
                            case J_ADDR:
#endif
                                op = J_KTOR(op);
                                r2 = aeb_argumentreg(&cl, e1_(base), callcount);
                                m.r = aeb_argumentreg(&cl, e2_(base), callcount);
                                break;
                            case J_ADCONV:
                                m.b = e1b_(base);
                                r2 = locoff_(loc);
                                op = J_addvk(op);
                                break;
#if 1
				/*
				 * XXX - NC - 6/10/93
				 * Horrible hack just to get compiler working.
				 */
			      case J_LDRK:
				r2  = aeb_argumentreg(&cl, base, callcount);
				m.i = locoff_(loc);
				break;
#endif				
                            default:
                                syserr(syserr_baseop, (long)baseop);
                            }
                        }
                }
            }
            break;
        case E_CALL:
            {   int32 iargs = exiargs_(exprn);
                int32 fargs = exfargs_(exprn);
                int32 i;
                CopyList *arglist = NULL;
                (*callcount)++;
                for (i = 0 ; i < iargs+fargs ; i++) {
                    Exprn *thisarg = exarg_(exprn, i);
                    Exprn *nextarg;
                    VRegnum argreg = i < fargs ? virtreg(R_FA1+i, DBLREG) :
                                                 virtreg(R_A1+i, INTREG);
                    VRegInt m1;
                    if ( exop_(thisarg) == CSE_WORD1 &&
                         (i+1) < iargs+fargs &&
                         exop_(nextarg = exarg_(exprn, i+1)) == CSE_WORD2 &&
                         e1_(thisarg) == e1_(nextarg) ) {
                        cl = AddExprnsBelow(cl, e1_(thisarg), &arg, callcount);
                        m1.i = 0;
                        cl = copylist(cl, NULL,
                                      floatinessofreg(arg->icode.r1.r,
/* These look like unclear assumptions on what cg.c does with args.     */
/* Harry assures AM that this code is just concerned with lifting       */
/* fns with fp args and no side-effects out of loops!                   */
/* Accordingly there is no J_PUSHR case.                                */
                                                      0, J_PUSHF, J_PUSHD),
                                      arg->icode.r1.r, GAP, m1);
                        m1.rl = mkRegList(mkRegList(NULL, argreg),
                                          virtreg(R_A1+i+1, INTREG));
                        arglist = copylist(arglist, NULL, J_POP, GAP, GAP, m1);
                        i++;
                    } else if (exop_(thisarg) == CSE_WORD1 ||
                               exop_(thisarg) == CSE_WORD2)
                        syserr(syserr_cse_wordn);
                    else {
                        m1.r = aeb_argumentreg(&cl, exarg_(exprn, i),
                                               callcount);
                        arglist = copylist(arglist, NULL, i < fargs ? J_MOVDR : J_MOVR,
                                           argreg, GAP, m1);
                    }
                }
                cl = (CopyList *)nconc(dreverse((List *)arglist), (List *)cl);
                {   RegSort restype = exfntype_(exprn);
                    VRegInt m1;
                    int32 argdesc = exargdesc_(exprn) | K_PURE;
                    r1 = V_resultreg(restype);
                    m1.b = exfn_(exprn);
                    cl = copylist(cl, exprn, op, r1, argdesc, m1);
                    cl->cse = mk_CopyCSE(cl);
                    *clp = cl;
                    return cl;
                }
            }
        }
        cl = p = copylist(cl, exprn, op, r1, r2, m);
    }
    *clp = p;
    return cl;
}

static CopyList *IcodeToCopy_i(CSEDef *defs, int32 *callcount)
{
    CopyList *res = NULL, *p;
    addeddefs = NULL;
    for ( ; defs != NULL ; defs = cdr_(defs))
        if (defs->icode == NULL) {
            res = AddExprnsBelow(res, defs->exprn, &p, callcount);
            if (p->cse == NULL) {
                p->cse = mk_CopyCSE(p);
            }
            p->cse->csedefs = (CSEDefList *)CSEList2(p->cse->csedefs, defs);
        }
    return (CopyList *)dreverse((List *)res);
}

static CopyList *IcodeToCopy(
  BlockHead *b, CSEDef *defs, BindList *bl, int32 *ndefs) {
    int32 callcount = 0;
    CopyList *c = IcodeToCopy_i(defs, &callcount);
    int32 n = length((List *) c);
    {   BindList **bp = &bl->bindlistcdr;
        for (; addeddefs != NULL; addeddefs = cdr_(addeddefs))
            if (addeddefs->csedefs == NULL) {
        /* Observe that if we're doing this, bl must be non-null - so
         * adding new binders generated here after the first element of
         * bl is safe (and saves trouble with the code for modifying
         * SETSPENVs).
         */
            VRegnum r1 = addeddefs->def->icode.r1.r;
            CSEDef *csedef = mkCSEDef(addeddefs->def->exprn, b);
            J_OPCODE op = addeddefs->def->icode.op;
            if (is_call2(addeddefs->def->exprn)) {
                CSEBind2_(csedef) = addcsebinder(op, bp, r1);
                n++;
            }
            csedef->icode = &addeddefs->def->icode;
            if (!j_is_check(csedef->icode->op)) {
                CSEBind_(csedef) = addcsebinder(op, bp, r1);
                n++;
            }
            addeddefs->csedefs = (CSEDefList *)CSEList2(NULL, csedef);
        }
    }
    if (callcount == 1 && !(blkflags_(b) & BLKCALL))
        blkflags_(b) |= BLKCALL;
    else if (callcount >= 1)
        blkflags_(b) |= BLKCALL | BLK2CALL;
    *ndefs += n;
    return c;
}

static Icode *CopyIcode(Icode *newic, CopyList *c) {
    for ( ; c != NULL ; c = cdr_(c)) {
        *newic = c->icode;
        if (c->cse != NULL) {
          CopyListList *refs = c->cse->refs;
          CSEDefList *defs = c->cse->csedefs;
          for (; defs != NULL; defs = cdr_(defs)) {
            CSEDef *def = defs->def;
            Binder *b = CSEBind_(def);
            if (def->super != NULL) {
                VRegnum r = bindxx_(CSEBind_(def->super));
                newic->r2.r = r;
                note_slave(newic->r1.r, r);
            }
            if (b != NULL) newic = storecse(newic, def);
            for (; refs != NULL; refs = cdr_(refs)) {
                Icode *ic = &refs->p->icode;
#ifdef RANGECHECK_SUPPORTED
                if (b == NULL)
                    ic->op = J_NOOP;
                else
#endif
                if (pseudo_reads_r2(c->icode.op)) {
                    ic->r2.r = bindxx_(b);
                    note_slave(ic->r1.r, bindxx_(b));
                } else
                    ic->m.b = b;
            }
          }
        }
        newic++;
    }
    return newic;
}

static CSERef *FindRealRef(CSEDef *d) {
    for (; d != NULL; d = d->nextsub) {
        if (d->refs != NULL)
            return d->refs;
        if (d->subdefs != NULL) {
            CSERef *ref = FindRealRef(d->subdefs);
            if (ref != NULL) return ref;
        }
    }
    return NULL;
}

static BindList *ReferenceCSEDefs(BindList *bl, CSEDef *def)
{
    for ( ; def != NULL ; def = cdr_(def)) {
        CSERef *ref = def->refs;
#ifdef TARGET_ALLOWS_COMPARE_CSES
        if (is_compare(exop_(def->exprn)))
            CSEDefMask_(def) = def->icode->op & Q_MASK;
#endif
        if (ref != NULL || def->subdefs != NULL) {
            if (
#ifdef TARGET_ALLOWS_COMPARE_CSES
                is_compare(exop_(def->exprn)) ||
#endif
                j_is_check(exop_(def->exprn))) {
                CSEBind_(def) = NULL;
                if (debugging(DEBUG_CSE))
                    cc_msg("\n: ");
            } else {
                J_OPCODE op = exop_(def->exprn);
                VRegnum r1;
                CSERef *realref = FindRealRef(def);
                if (realref == NULL) syserr("ReferenceCSEDefs");
                r1 = useicode_(refuse_(realref)).r1.r;
                CSEBind_(def) = addcsebinder(op, &bl, r1);
                if (debugging(DEBUG_CSE))
                    cc_msg("\n$b [%ld]: ",
                           CSEBind_(def), (long)bindxx_(CSEBind_(def)));
                if (is_call2(def->exprn)) {
                    CSEBind2_(def) = addcsebinder(op, &bl, r1);
                    if (debugging(DEBUG_CSE))
                        cc_msg("$b [%ld]: ",
                               CSEBind2_(def), (long)bindxx_(CSEBind2_(def)));
                }
            }
            if (debugging(DEBUG_CSE)) {
                cc_msg("%ld/", (long)blklabname_(def->block));
                if (blklength_(def->block) == 0)
                    cc_msg("loop inv: ");
                else
                    cc_msg("%ld: ", (long)(def->icode-blkcode_(def->block)));
                cse_print_node(def->exprn);
            }
            for ( ; ref != NULL ; ref = cdr_(ref))
                replaceicode(refuse_(ref), def);
        }
    }
    return bl;
}

static void AddCSEDefsToBlock(CSEDef *def)
{
    CSEDef *next;
    
    for (; def != NULL; def = next)
      {
        next = cdr_(def);
	
        if (def->refs != NULL || def->subdefs != NULL)
	  {
            BlockHead *b = def->block;

            if (blkflags_(b) & BLKOUTER)
	      {
            /* for expressions lifted out of the fake outer "loop", we wish to
               lift them not to the loop preheader, but to the nearest block
               which dominates all references.
             */
                CSERef *ref = def->refs;
                CSEDef *sub = def->subdefs;
                VRegSetP d;
                bool discard = NO;

                if (ref != NULL)
                    b = refuse_(ref)->block, ref = cdr_(ref);
                else
                    b = sub->block, sub = sub->nextsub;
                d = cseset_copy(blk_dominators_(b));
                cseset_difference(d, loopmembers);
                for (; ref != NULL; ref = cdr_(ref))
                    cseset_intersection(d, blk_dominators_(refuse_(ref)->block));
                for (; sub != NULL; sub = sub->nextsub)
                    cseset_intersection(d, blk_dominators_(sub->block));
               for (;;)
		 {
                    bool dummy;
                    for (b = top_block; b != NULL; b = blkdown_(b))
                        if (cseset_member(blklabname_(b), d) &&
                            cseset_compare(d, blk_dominators_(b)) <= VR_EQUAL)
                            /* (equal or subset) */
                            break;
                    if (b == NULL) syserr(syserr_addcsedefs);
                    if (!(blkflags_(b) & BLKCCLIVE)
#if 0
			/*
			 * XXX - NC - 5/10/93
			 * This line cause a core dump when compiling the
			 * function 'parse_bslash()' in the file XLocalIM.c
			 * in the directory /scratch/pd/X11R5/mit/lib/X
			 * I do not know what this line is trying to achieve,
			 * nor what the consequences of omitting it will be,
			 * but at least the compiler now works.
			 */
			||			
/* what on earth should the next line do?                            */
                        !alterscc(&useicode_(refuse_(def->refs)))
#endif
			)
		      
                        break;
                    cseset_delete(blklabname_(b), d, &dummy);
                }
                {  /* if the block to which we've decided to lift the
                      expression contains a reference, it's not a lifted CSE.
                      There may be many references in the block, thanks to an
                      amalgamated local CSE.
                    */
                    CSEDef **subp = &def->subdefs;

                    /* First check for a lifted reference */
                    for (; (sub = *subp) != NULL; subp = &(sub->nextsub))
                        if (sub->block == b)
			  {
                            sub->super = NULL;
                            *subp = sub->nextsub;
                            {   CSEDef *p, *oldsubdefs = sub->subdefs;
                                sub->subdefs = def->subdefs;
                                subp = &(sub->subdefs);
                                for (; (p = *subp) != NULL; subp = &(p->nextsub))
                                    p->super = sub;
                                *subp = oldsubdefs;
                            }
                            discard = YES; break;
                        }
                    if (!discard) {
                        CSERef **prev = &def->refs;
                        CSERef **defpref = NULL;
                        Icode *deficode = NULL;
                        for (; (ref = *prev) != NULL; prev = &cdr_(ref))
                            if (b == refuse_(ref)->block) {
                                if (pseudo_reads_r2(exop_(def->exprn))) {
                                    discard = YES;
                                    break;
                                } else {
                                    Icode *reficode = &useicode_(refuse_(ref));
                                    if (deficode != NULL && deficode < reficode)
                                        continue;
                                    defpref = prev;
                                    deficode = reficode;
                                }
                            }
                        if (deficode != NULL) {
                            def->icode = deficode;
                            *defpref = cdr_(*defpref);
                        }
                    }
                }
                if (debugging(DEBUG_CSE))
                    if (!(blkflags_(b) & BLKOUTER))
                        cc_msg("%ld %s : %ld\n",
                               (long)exid_(def->exprn),
                               (discard ? "discarded" :
                                def->icode == NULL ? "(lifted)" : "(not lifted)"),
                               (long)blklabname_(b));

                if (discard) continue;
                def->block = b;
            }
            {   CSEDef **prevp = &blk_defs_(b);
                CSEDef *p = blk_defs_(b);
                /* CSEDefs are ordered by the position of their definition
                   within the block, with those corresponding to lifted
                   expressions (where there is no definition in the block)
                   coming last.
                 */
                for (; p != NULL; p = cdr_(p)) {
                    if ( def->icode != 0 &&
                         (p->icode == 0 || p->icode > def->icode))
                        break;
                    prevp = &cdr_(p);
                }
                *prevp = def;
                cdr_(def) = p;
            }
        }
    }
}

#ifdef TARGET_ALLOWS_COMPARE_CSES
static bool CantMarkCCLive(LabelNumber *from, BlockHead *to) {
    BlockHead *p;
    LabelNumber *lab;
    for (lab = from; ; lab = blknext_(p)) {
        if (is_exit_label(lab))
            return YES;
        p = lab->block;
        if (p == to)
            break;
        if (blkflags_(p) & (BLKSWITCH+BLK2EXIT))
            return YES;
    }
    for (p = from->block; ; p = blknext_(p)->block) {
        blkflags_(p) |= BLKCCLIVE;
        if (p == to) break;
    }
    return NO;
}
#endif

static BindList *modifycode(void)
{
    CSE *cse;
    BindList *bl = NULL;
    /* Hang CSE definitions off the heads of the blocks containing them
     * (sorted by the order of their occurrence in the block).
     */
#ifdef TARGET_ALLOWS_COMPARE_CSES
    /* compares as CSEs must inhibit lifting of expressions which affect the
     * condition codes into any block between the successor of the defining
     * block and the referencing block (inclusive).
     * (observe that a block can contain at most one compare, so it would be
     *  foolish to process localcsedefs here).
     */
    for (cse = cselist ; cse != NULL ; cse = cdr_(cse))
        if (is_compare(exop_(cse->exprn))) {
            CSEDef *def = cse->def;
            for (; def != NULL; def = cdr_(def)) {
                BlockHead *defblock = def->block;
                CSERef *ref = def->refs;
                for (; ref != NULL; ref = cdr_(ref)) {
                /* the blocks between the defining and referencing ones must
                 * all be single-exit (other types would destroy the condition
                 * codes).
                 */
                    BlockHead *refblock = refuse_(ref)->block;
                    /* nb MarkCCLive applied to both paths from the defining block */
                    if (CantMarkCCLive(blknext_(defblock), refblock) &
                        CantMarkCCLive(blknext1_(defblock), refblock))
                        syserr(syserr_modifycode_2, (long)blklabname_(refblock),
                                                    (long)blklabname_(defblock));
                }
            }
        }
#endif

    for (cse = cselist ; cse != NULL ; cse = cdr_(cse))
        AddCSEDefsToBlock(cse->def);
    AddCSEDefsToBlock(localcsedefs);

    {   BlockHead *b;
        for (b = top_block; b != NULL; b = blkdown_(b))
             bl = ReferenceCSEDefs(bl, blk_defs_(b));
    }
    /* Now add stores into the binders created for eliminated expressions.
     * This means lengthening the code in existing blocks: we could be clever
     * about doing this (largest first, allowing the space for most of the rest
     * to be reused).  Later, perhaps.
     * While I'm doing this, destructively alter all non-null binder chains in
     * block heads to end in the cse list, and all null ones to be the cse list.
     * (Except for the first block - the binders don't exist on function entry.
     *  We're going to create a SETSPENV to introduce them at the end of the
     *  first block).  The intent is to cause the extent of cse binders to be
     * the whole function.  Later, we can be more precise.
     * Do chains in J_SETSPxx first.  These need care to avoid changing the
     * new binder chain for a SETSPENV immediately before a return from null.
     * (Otherwise, return from a function which stores things on the stack but
     * doesn't create a frame will be compromised).
     * There can't be returns without a preceding SETSPENV: cg has been fixed
     * to ensure it.  See cg_return.
     */
    {   SetSPList *p;
        for ( p = setsplist ; p != NULL ; p = cdr_(p) ) {
            BlockHead *b = p->block;
            Icode *ic = p->icode;
            ic->r2.bl = nconcbl(ic->r2.bl, bl);
            if ( ic->op == J_SETSPENV &&
                 ( (ic+1) - blkcode_(b) != blklength_(b) ||
                   !is_exit_label(blknext_(b))))
                ic->m.bl = nconcbl(ic->m.bl, bl);
            else; /* J_SETSPGOTO is done */
        }
    }
    {   BlockHead *b;
        for ( b = blkdown_(top_block) ; b != NULL ; b = blkdown_(b) ) {
/* AM: My suspicion is that this can add cycles or corrupt arglists etc.*/
/* Moreover, AM thinks that the code consider the possible presence of  */
/* double_pad_binder and integer_binder (see cg.c) means that this      */
/* insertion (which I take to be adding to bindlists at the start of    */
/* blocks) needs to be rather more careful.  Consider conditional       */
/* expressions within arguments (e.g. f(x?y:z, 16bytestructval) on arm  */
/* and more seriously on machines where double_pad_binder really pads.  */
/* It feels like cg/flowgraf.c should export a routine to do this.      */
/* On further reading AM is getting slighly happier about this since    */
/* the binders get added first (and not within arglist construction     */
/* but remains unhappy about exactly what bindlist's get smashed via    */
/* process.                                                             */
            CSEDef *defs = blk_defs_(b);
            blkstack_(b) = nconcbl(blkstack_(b), bl);

            if (defs != NULL) {
            /* Now we can have a block containing both real CSE defs and
               lifted ones
             */
                int32 ndefs = defsize(defs);
                CopyList *c = IcodeToCopy(b, defs, bl, &ndefs);
                    /* which updates ndefs by the number of JopCodes to lift */
                int32 oldlength = blklength_(b);
                Icode *newic = newicodeblock(oldlength+ndefs);
                Icode *old = blkcode_(b);
                int32 i;
                blkcode_(b) = newic;
                for ( i = 0 ; i != blklength_(b) ; i++ ) {
                    *newic = old[i];
                    while (defs != NULL && (&old[i]) == defs->icode) {
                    /* At the moment, there may be both a non-local and
                       a local def for the same icode (if the exprn was
                       killed earlier in the block)
                     */
                        newic = storecse(newic, defs);
                        defs = cdr_(defs);
                    }
                    newic++;
                }
                if (defs != NULL) {
                    if ( oldlength != 0 &&
                         (blkflags_(b) & (BLKSWITCH | BLK2EXIT))) {
                    /* J_CASEBRANCH or J_CMP must be the last op in the block.
                       Copy from newic rather than old, because J_CMPK may have
                       had its r1 field changed from GAP.
                     */
                        Icode ic;
                        ic = *--newic;
                        newic = CopyIcode(newic, c);
                        *newic++ = ic;
                    } else
                        newic = CopyIcode(newic, c);
                }
                if (oldlength != 0) freeicodeblock(old, oldlength);
                blklength_(b) +=ndefs;
                if ((newic - blkcode_(b)) != blklength_(b)) {
                    Icode *ic = blkcode_(b);
                    for (; ic != newic; ic++)
                       print_jopcode(ic->op, ic->r1, ic->r2, ic->m);
                    syserr(syserr_modifycode, (long)blklabname_(b),
                              (long)(newic - blkcode_(b)), (long)blklength_(b));
                }
            }
        }
    }
    {   int32 i = blklength_(top_block);
        Icode *newic = newicodeblock(i+1);
        Icode *old = blkcode_(top_block);
        blkcode_(top_block) = newic;
        if (i != 0) memcpy(newic, old, (int) i*sizeof(Icode));
        freeicodeblock(old, i);
        newic[i].op = J_SETSPENV;
        newic[i].r1.r = GAP;
        newic[i].r2.bl = NULL;
        newic[i].m.bl = bl;
        blklength_(top_block) = i+1;
    }
    greatest_stackdepth += sizeofbinders(bl, YES);
    return csespillbinders;
}

static clock_t csetime;

static void cse_setup()
{   csespillbinders = NULL;
    setsplist = NULL;
    vregset_init();
    cselist = NULL; localcsedefs = NULL;
}

static BindList *cse_eliminate_i()
{   BlockHead *p;
    BindList *bl = NULL;
    clock_t t0 = clock();
    clock_t ts = t0;
    int32 refs = cse_refs, count = cse_count;

    nsets = 0; newsets = 0; setbytes = 0;
    end_emit();

    phasename = "CSELoops";
    cse_setup();
    for (p = top_block; p != NULL; p = blkdown_(p))
      p->cse = new_CSEBlockHead();

    FindDominators();
    findloops();

    {   LoopList *lp;
        for (lp = all_loops; lp != NULL; lp = cdr_(lp)) {
            BlockList *bp = lp->members;
            for (; bp != NULL; bp = bp->blklstcdr) blknest_(bp->blklstcar)++;
        }
    }

    if (debugging(DEBUG_CSE | DEBUG_STORE)) {
        cc_msg("dominators found - %d csecs\n", clock()-t0);
    }

    if (cse_enabled && !usrdbg(DBG_LINE)) {

      phasename = "CSE_Available";
      cse_scanblocks(top_block);
      /* which can alter arcs in the flowgraf, so now we recompute
         the dominator sets
       */
      {   for (p=blkdown_(top_block); p!=NULL; p = blkdown_(p))
              blk_reached_(p) = NO;

          PruneDominatorSets();
          for (p = top_block; p != NULL; p = blkdown_(p))
              if (!blk_reached_(p)) {
                  cseset_discard(blk_dominators_(p));
                  blk_dominators_(p) = NULL;
              }
          if (debugging(DEBUG_CSE) && CSEDebugLevel(2)) {
              for (p=top_block; p!=NULL; p = blkdown_(p)) {
                  cc_msg("Block %ld dominated by ", blklabname_(p));
                  cse_printset(blk_dominators_(p));
                  cc_msg("\n");
              }
              cc_msg("loop members ");
              cse_printset(loopmembers);
              cc_msg("\n");
          }
      }

      phasename = "CSEdataflow";
      t0 = clock();
      {   bool changed;
          do {
              if (debugging(DEBUG_CSE | DEBUG_STORE))
                  cc_msg("CSE dataflow iteration\n");
              changed = NO;
              for (p=bottom_block; p!=NULL; p = blkup_(p)) {
                  VRegSetP oldwantedlater = blk_wantedlater_(p);
                  VRegSetP oldwantedonallpaths = blk_wantedonallpaths_(p);
                  exprnsreaching(p);
                  if (!cseset_equal(oldwantedlater, blk_wantedlater_(p)) ||
                      !cseset_equal(oldwantedonallpaths,
                                    blk_wantedonallpaths_(p)))
                      changed = YES;
                  cseset_discard(oldwantedlater);
                  cseset_discard(oldwantedonallpaths);
              }
          } while (changed);
      }
      if (debugging(DEBUG_CSE | DEBUG_STORE)) {
          clock_t now = clock();
          cc_msg("CSE dataflow complete - %d csecs\n", now-t0);
          t0 = now;
      }
      phasename = "CSEfind";
      /* Now things have converged, and for each block we have
       *  available   as before
       *  wantedlater the set of expressions evaluated by some subsequent block
       *              and not killed on any path from here to the evaluating
       *              block.
       *  wantedonallpaths the set of expressions evaluated by some block on each
       *              path from the block, and not killed between here and there.
       * An expression is a candidate for elimination if it is in both available
       * and wantedlater.
       * For loop headers, wantedonallpaths is the set of loop-constant
       * expressions which can be pulled out of the loop with complete safety.
       */
      for (p = bottom_block; p != NULL; p = blkup_(p)) {
          /* This loop goes from the bottom upwards so that, in the list of
           * definitions for a given CSE, ones in earlier blocks come first,
           * so we have a good chance of rejecting definitions which are
           * subsumed without doing any work.
           */
          VRegSetP cses = cseset_copy(blk_wantedlater_(p));
          cseset_intersection(cses, blk_available_(p));
          cseset_map(cses, addcse, (VoidStar) p);
          cseset_discard(cses);
          if (debugging(DEBUG_CSE) && CSEDebugLevel(3)) {
              cc_msg("L%li:", (long)blklabname_(p));
              if (blkflags_(p) & BLKLOOP) cc_msg("*");
              cc_msg(" w"); cse_printset(blk_wanted_(p));
              cc_msg(" wl"); cse_printset(blk_wantedlater_(p));
              cc_msg(" we"); cse_printset(blk_wantedonallpaths_(p));
              cc_msg(" k"); if (blk_killedinverted_(p)) cc_msg("~");
                            cse_printset(blk_killed_(p));
              cc_msg(" a"); cse_printset(blk_available_(p)); cc_msg("\n");
          }
      }
      if (debugging(DEBUG_CSE | DEBUG_STORE))
          cc_msg("candidate cses found - ");
      findloopinvariants();
      if (debugging(DEBUG_CSE | DEBUG_STORE)) {
          clock_t now = clock();
          cc_msg("candidate loop invariants found - %d csecs\n", now-t0);
          t0 = now;
      }
      LinkRefsToDefs();
      if (debugging(DEBUG_CSE | DEBUG_STORE)) {
          clock_t now = clock();
          cc_msg("cse references linked - %d csecs\n", now-t0);
          t0 = now;
      }
      phasename = "CSEeliminate";
      bl = modifycode();
      if (debugging(DEBUG_CSE | DEBUG_STORE)) {
          clock_t now = clock();
          cc_msg("%ld CSEs, %ld references - %d csecs, CSE total %d\n",
                      (long)(cse_count-count), (long)(cse_refs-refs),
                      now-t0, now-ts);
          csetime += (now - ts);
      }
      if (nsets > maxsets) maxsets = nsets;
      if (setbytes > maxbytes) maxbytes = setbytes;
    }

    for (p = top_block; p != NULL; p = blkdown_(p))
        blk_pred_(p) = NULL;

    return bl;
}

extern BindList *cse_eliminate(void)
{
    BindList *bl = cse_eliminate_i();
    if (debugging(DEBUG_CG) ||
        (debugging(DEBUG_CSE) && CSEDebugLevel(1)))
        flowgraf_print("CSE transforms to:");
    return bl;
}

extern void cse_reinit(void)
{
    all_loops = NULL;
}

extern void cse_init(void)
{
    csetime = 0;
    maxsets = 0; maxbytes = 0;
    cse_count = 0; cse_refs = 0;
/*
 * The next few are done this way so that the object file for the compiler
 * will not contain any initialised statics initialised to relocatable
 * values.  Doing so may (on some systems) make it easier to build a
 * relocatable version of the compiler (e.g. as an Archimedes/RISC-OS
 * relocatable module).
 */
    cseallocrec.alloctype = CSEAllocType;
    cseallocrec.statsloc = &nsets;
    cseallocrec.statsloc1 = &newsets;
    cseallocrec.statsbytes = &setbytes;
}

extern void cse_tidy(void)
{
    if (!debugging(DEBUG_STORE | DEBUG_CSE)) return;
    cc_msg("CSE max sets: %ld (%ld bytes): time %d cs\n",
                (long)maxsets, (long)maxbytes, csetime);
    cc_msg("%ld cses, %ld references\n", (long)cse_count, (long)cse_refs);
}

/* end of mip/cse.c */
