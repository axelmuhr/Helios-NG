/*
 * cse.c: Common sub-expression elimination
 * Copyright (C) Acorn Computers Ltd., 1988.
 */

/*
 * RCS $Revision: 1.1 $
 * Checkin $Date: 1993/11/23 16:41:52 $
 * Revising $Author: nickc $
 */

#ifndef NO_VERSION_STRINGS
extern char cse_version[];
char cse_version[] = "\ncse.c $Revision: 1.1 $ 123\n";
#endif

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
#include "aeops.h"
#include "jopcode.h"
#include "store.h"
#include "regalloc.h"
#include "cg.h"
#include "flowgraf.h"
#include "mcdep.h"     /* usrdbg, DBG_xxx */
#include "builtin.h"   /* for te_xxx */
#include "errors.h"

#ifndef TARGET_HAS_SCALED_ADDRESSING
/* This seems an easy way of avoiding more optional compilation */
/* No -- surely the correct thing is to define something              */
/* else -- not do silly things like this.                                */
#  define J_SHIFTMASK 0L
#  define J_NEGINDEX  0L
#endif

typedef struct CSERef {
    struct CSERef *cdr;
    union {
        BlockHead *b;
        ExprnUse *ex;
    } ref;
} CSERef;

#define refuse_(p) ((p)->ref.ex)
#define refblock_(p) ((p)->ref.b)

typedef struct CSEDef {
    struct CSEDef *cdr;
    BlockHead *block;
    Exprn *exprn;
    CSERef *refs;
    Icode *icode;
    VRegSetP uses;
    Binder *binder;
    Binder *binder2;  /* only present for CALL2KP (fn returning 2 results) */
} CSEDef;

typedef struct CSE {
    struct CSE *cdr;
    Exprn *exprn;
    CSEDef *def;
} CSE;

static CSE *cselist;
static CSEDef *localcsedefs;

static BindList *csespillbinders;

static LoopList *all_loops;

static unsigned32 nsets, newsets, setbytes;
static unsigned32 maxsets, maxbytes;
static unsigned32 cse_count, cse_refs;

Exprn **exprnindex[EXPRNINDEXSIZE];

Location **locindex[LOCINDEXSIZE];

VRegSetAllocRec cseallocrec; /* = {CSEAllocType, &nsets, &newsets, &setbytes}; */

SetSPList *setsplist;

#ifdef ENABLE_CSE

#define printset(s) cseset_map(s, ps, NULL)

static void ps(int32 n, VoidStar arg)
{
    IGNORE(arg);
    cc_msg(" %ld", (long)n);
}

#else

#define printset(s) /* nothing */

#endif

static int32 floatinessofreg(VRegnum r, int32 plain, int32 flt, int32 dbl)
{   switch (regtype_(r))
    {   case FLTREG: return flt;
        case DBLREG: return dbl;
        default:     return plain;
    }
}

static int32 regtypeofop(J_OPCODE op)
{
    switch (floatiness_(op))
    {   case _J_FLOATING: return FLTREG;
        case _J_DOUBLE:   return DBLREG;
        default:          return INTREG;
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
    int32 count = p->r2.i;
    VRegSetP args = NULL;
    int32 i;
    Icode *res = p;


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
    
    if (nextinblock)
      {
        Icode *next = p + 1;

        if ( register_movep(next->op) &&
             next->m.i == p->r1.i )
	  {
            p->op = J_NOOP;
            res = next;
        }
    }

    for (i = 0 ; i < count ; i++)
        cseset_insert(virtreg(R_P1+i, INTREG), args, NULL);

    while (count > 0 && (--p) >= blockstart)
      {
        int32 op = p->op;

	
        if (loads_r1(op))
	  {
            VRegnum r = p->r1.r;

	    
            if (cseset_member(r, args))
	      {
                count--;
		
               /* p->op = J_NOOP;*/
		
                p->r1.r = vregister(INTREG);
		
                /* Register allocation should now manage to kill this, and
                   this way we needn't worry about lifting CSE definitions
                   too */
		
                cseset_delete(r, args, NULL);
            }
        }
	else if (op == J_POP)
	  {
            RegList *rp = p->m.rl;
            int32 n = 0;
            bool killable = YES;

	    
            for ( ; rp != NULL ; rp = rp->rlcdr)
                if (cseset_member(rp->rlcar, args))
		  {
                    n++; count--;
                    cseset_delete(rp->rlcar, args, NULL);
                } else
                    killable = NO;
	    
            if (killable)
	      {
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
    if (op == J_CALLK && (target->r2.i & K_PURE) || op == J_CALL2KP) {
        Icode *blockend = blkcode_(b)+blklength_(b);
	
        target = trytokillargs(target, blkcode_(b), (target+1) < blockend);
    }
    if (def != NULL) {
        Exprn *ex = def->exprn;
        if (pseudo_reads_r2(exop_(ex))) {
            VRegnum r = bindxx_(def->binder);
            if (pseudo_reads_r1(op))
                target->r1.r = r;
            else {
                target->op = exop_(ex);
                target->r2.r = r;
                note_slave(target->r1.r, r);
            }
            target->m.i = e1k_(ex);
        } else if (valno_(ref) == 0)
            replacewithload(target, r1, def->binder);
        else
            replacewithload(target, r1, def->binder2);
    }
    cse_refs++;
    return target;
}

static Binder *addcsebinder(J_OPCODE op, BindList **bl, VRegnum r)
{
    Binder *bnew;
    switch (regtype_(r)) {
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
    if (!pseudo_reads_r2(op)) {
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
#ifdef TARGET_HAS_DIVREM_FUNCTION
/* @@@ AM does not like local routines called Allocate() without prefix */
    if (exop_(ex) == J_CALL2KP)
        def = Allocate(CSEDef);
    else
#endif
        def = (CSEDef *)CSEAlloc(offsetof(CSEDef, binder2));
    def->block = b; def->icode = NULL; def->exprn = ex;
    def->refs = NULL; def->uses = NULL; def->binder = NULL;
    return def;
}

static CSEDef *addtocselist(Exprn *ex, BlockHead *b, bool mustbenew)
{
    CSE *cse;
    CSEDef *def;
    for (cse = cselist ; cse != NULL ; cse = cdr_(cse))
        if (ex == cse->exprn) break;
    if (cse == NULL)
        cse = cselist = (CSE *)CSEList3((int32)cselist, (int32)ex, NULL);
    if (!mustbenew)
        for (def = cse->def; def != NULL; def = cdr_(def))
            if (def->block == b) return def;

    def = mkCSEDef(ex, b);
    cdr_(def) = cse->def; cse->def = def;
    return def;
}

static bool worthreplacement(Exprn *ex)
{
/* I don't see that it makes much sense to have separate rules for what's worth
   making into a CSE for local and non-local use, which is what there used to be.
 */
#ifdef TARGET_IS_88000  /* Experimental */
    if (exop_(ex) == J_MOVK) return NO;
#endif
    if (extype_(ex) == E_LOAD) return ispublic(exloc_(ex));
    return exop_(ex) != J_STRING && exop_(ex) != J_ADCONV;
#ifdef never
/* old non-local rules were */
         /*exop_(ex) != J_ADCON &&*/ exop_(ex) != J_MOVK &&
         exop_(ex) != J_ADCONV && exop_(ex) != J_STRING &&
         ( extype_(ex) != E_LOAD || ispublic(exloc_(ex))) )
/* old local rules were */
/*
 * Treatment of _FLOAT & _DOUBLE made ultra conservative while
 * jopcode.h is in state of flux.  Maybe an extra bit in joptable can help
 * things out here...
 */
    int32 opx = op & ~(J_SIGNED | J_UNSIGNED);
    if (opx == J_MOVR || op == J_MOVK) return NO;
    if (opx == J_MOVFR || op == J_MOVFK) return NO;
    if (opx == J_MOVDR || op == J_MOVDK) return NO;
    if (opx == J_LDRV || opx == J_LDRFV || opx == J_LDRDV)
        return locpublic_(loc);
    return YES;
#endif
}

bool addlocalcse(Exprn *node, int valno, BlockHead *b)
{
    ExprnUse *def = exuses_(node);
    Icode *deficode;
    CSEDef *csedef;
    CSERef *ref;

    if (!worthreplacement(node)) return NO;
    ref = Allocate(CSERef);

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
        if (debugging(DEBUG_CSE) && var_cse_enabled > 16)
            cc_msg("(subsumable) ");
    } else {
        for (csedef = localcsedefs ; csedef != NULL ; csedef = cdr_(csedef))
            if (csedef->icode == deficode) break;
        if (csedef == NULL) {
            csedef = mkCSEDef(node, b);
            flags_(def) |= U_LOCALCSE;
            cdr_(csedef) = localcsedefs;
            localcsedefs = csedef;
        }
    }
    csedef->icode = deficode;
    cdr_(ref) = csedef->refs; csedef->refs = ref;
    refuse_(ref) = mkExprnUse(NULL, 0, valno);
    if (debugging(DEBUG_CSE) && var_cse_enabled > 16)
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
            LocType type = loctype_(loc);
            switch (type) {
            case LOC_VAR:
            case LOC_PVAR:
                return NO;

            default:
                return containsloadr(locbase_(loc));
            }
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
        addtocselist(ex, (BlockHead *)(int)arg, YES);
}

static CSEBlockHead *new_CSEBlockHead(void)
{
    CSEBlockHead *q = Allocate(CSEBlockHead);
    q->available = NULL; q->wanted = NULL;
    q->wantedlater = NULL; q->wantedonallpaths = NULL;
    q->killed = NULL; q->dominators = NULL;
    q->d.defs = NULL;
    q->killedinverted = NO;
    q->reached = NO;
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

static void addpredecessor(LabelNumber *lab, BlockHead *p)
{
    if ( !is_exit_label(lab) &&
         !member((VRegnum)p, (RegList *)blk_pred_(lab->block)) )
        blk_pred_(lab->block) = (BlockList *)CSEList2(blk_pred_(lab->block), p);
}

static void finddominators(void)
{
    BlockHead *p;
    bool changed;
    {   VRegSetP allblocks = NULL;
        for (p=top_block; p!=NULL; p = blkdown_(p))
            cseset_insert(blklabname_(p), allblocks, NULL);
        for (p=blkdown_(top_block); p!=NULL; p = blkdown_(p))
            blk_dominators_(p) = cseset_copy(allblocks);
        cseset_discard(allblocks);
    }
    blk_reached_(top_block) = YES;
    cseset_insert(blklabname_(top_block), blk_dominators_(top_block), NULL);
    do {
        changed = NO;
        for (p=top_block; p!=NULL; p = blkdown_(p)) {
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
    for (p=top_block; p!=NULL; p = blkdown_(p)) {
        if (!blk_reached_(p)) {
            cseset_discard(blk_dominators_(p));
            blk_dominators_(p) = NULL;
        } else if (blkflags_(p) & BLKSWITCH) {
            LabelNumber **v = blktable_(p);
            int32 i, n = blktabsize_(p);
            for (i=0; i<n; i++)
                addpredecessor(v[i], p);
        } else {
            addpredecessor(blknext_(p), p);
            if (blkflags_(p) & BLK2EXIT)
                addpredecessor(blknext1_(p), p);
        }
    }
}

static LoopList *mkLoopList(BlockList *members, BlockHead *preheader)
{
    LoopList *l = Allocate(LoopList);
    l->llblklist = members; l->llblkhd = preheader;
    if (debugging(DEBUG_CSE) && var_cse_enabled > 17) {
        cc_msg("add loop %ld:", (long)blklabname_(preheader));
        for (; members != NULL; members = members->blklstcdr)
            cc_msg(" %ld", (long)blklabname_(members->blklstcar));
        cc_msg("\n");
    }
    return l;
}

static BlockHead *makepreheader(BlockHead *pred, BlockHead *header)
{
    BlockHead *preheader = insertblockbetween(pred, header);
    int32 pn = blklabname_(preheader);
    BlockHead *p;
    preheader->cse = new_CSEBlockHead();
    blk_reached_(preheader) = YES;
    blk_dominators_(preheader) = cseset_copy(blk_dominators_(pred));
    for (p = blkdown_(top_block); p != NULL; p = blkdown_(p))
        if (blk_reached_(p))
            cseset_insert(pn, blk_dominators_(p), NULL);
    blkflags_(preheader) |= BLKLOOP;
    return preheader;
}

static void addloop(BlockHead *p, LabelNumber *q)
{
    if (!is_exit_label(q) && dominates(q->block, p)) {
        BlockHead *header = q->block;
        BlockList *bl = (BlockList *)CSEList2(NULL, p);
        BlockList *members = mkBlockList(NULL, header);
        BlockHead *preheader = NULL;
        while (bl != NULL) {
            BlockHead *b = bl->blklstcar;
            bl = bl->blklstcdr;
            if (!member((VRegnum)b, (RegList *)members)) {
                BlockList *pred = blk_pred_(b);
                members = mkBlockList(members, b);
                for (; pred != NULL; pred = pred->blklstcdr)
                    bl = (BlockList *)CSEList2(bl, pred->blklstcar);
            }
        }
        for (bl = blk_pred_(header); bl != NULL; bl = bl->blklstcdr)
            if (dominates(bl->blklstcar, header) && bl->blklstcar != header) {
            /* (The &&... because the loop might be a single block) */
                BlockHead *pred = bl->blklstcar;
                if (blk_pred_(pred) == (BlockList *)(int)DUFF_ADDR)
                /* That is, if this is a previously generated preheader */
                    preheader = pred;
                else {
                    preheader = makepreheader(pred, header);
                    bl->blklstcar = preheader;
                    blk_pred_(preheader) = NULL;
                }
                break;
            }

        if (preheader != NULL) {
        /* Loops are presented in source (blkdown_) order of the end block. */
            LoopList *l = mkLoopList(members, preheader);
            l->llcdr = all_loops; all_loops = l;
        }
    }
}

static void findloops(void)
{
    BlockHead *p;
    BlockList *allbuttop = NULL;
    bool changed;
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
    for (p = blkdown_(top_block); p != NULL; p = blkdown_(p)) {
        BlockList *bl = blk_pred_(p);
        blk_pred_(p) = NULL;
        while (bl != NULL) bl = (BlockList *)discard2((VoidStar)bl);
        allbuttop = mkBlockList(allbuttop, p);
    }
    /* Add a spurious whole function loop */
    {   BlockHead *h = makepreheader(top_block, blkdown_(top_block));
        LoopList *l = mkLoopList(allbuttop, h);
        l->llcdr = all_loops; all_loops = l;
        blkflags_(h) |= BLKOUTER;
    }
    /* Now adjust the dominator sets to take account of the inserted
       preheader blocks. (If we kept dominated sets instead, we wouldn't
       need to bother with this).
     */
    do {
        changed = NO;
        for (p=top_block; p!=NULL; p = blkdown_(p)) {
            VRegSetP s = p->cse->dominators;
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
    if (debugging(DEBUG_CSE) && var_cse_enabled > 17)
        for (p=top_block; p!=NULL; p = blkdown_(p)) {
            cc_msg("Block %ld dominated by ", blklabname_(p));
            printset(blk_dominators_(p));
            cc_msg("\n");
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

static void findreficodes(CSEDef *def, ExprnUse *uses)
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

static void discarddef(CSEDef *discard, CSEDef *keep)
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

static void linkrefs(CSE *cse, CSEDef *def)
{
/* For the definition  def  of  cse, find the uses of it (blocks for which
 * it is wanted) which the definition must reach.
 */
    BlockHead *defblock = def->block;
    bool local;
    if (def->uses != NULL)
    /* Non-null def->uses here means a local CSE which may be subsumed */
        local = YES;
    else {
        int32 exid = exid_(cse->exprn);
        BlockHead *p;
        bool changed;
        if (debugging(DEBUG_CSE))
            cc_msg("(%ld):%ld r", (long)exid, (long)blklabname_(defblock));
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
            if ( blk_reached_(p) && p != defblock &&
                 cseset_member(exid, blk_wanted_(p))) {
                def->refs = (CSERef *) syn_cons2(def->refs, p);
                cseset_insert(blklabname_(p), def->uses, NULL);
                if (debugging(DEBUG_CSE))
                    cc_msg(" %ld", (long)blklabname_(p));
            }
        }
        local = NO;
    }

    if (def->refs != NULL) {
        CSEDef *p;
        if (!local)
            findreficodes(def, exuses_(cse->exprn));

        if ( (blkflags_(defblock) & BLKOUTER) &&
             ( def->refs == NULL || /* findreficodes may have discarded some */
               cdr_(def->refs) == NULL) ) {
            if (debugging(DEBUG_CSE)) cc_msg(": unwanted outer loop inv");
            def->refs = NULL;
        } else {
            for (p = cse->def ; p != def ; p = cdr_(p)) {
                int cf = cseset_compare(def->uses, p->uses);
                if ( cf == VR_SUBSET ||
                     (cf == VR_EQUAL && dominates(p->block, defblock))) {
                    discarddef(def, p);
                    if (debugging(DEBUG_CSE)) cc_msg(": killed (%ld)", (long)blklabname_(p->block));
                    break;
                } else if (cf == VR_SUPERSET || cf == VR_EQUAL)
                    discarddef(p, def);
                    if (debugging(DEBUG_CSE)) cc_msg(": kills %ld", (long)blklabname_(p->block));
            }
        }
    }
    if (debugging(DEBUG_CSE)) cc_msg("\n");
}

static void linkrefstodefs(void)
{
    CSE *cse;
    for ( cse = cselist ; cse != NULL ; cse = cdr_(cse) ) {
        CSEDef *def, **prevp = &cse->def;
        VRegSetP refblocks = NULL;
        if (debugging(DEBUG_CSE))
            cse_print_node(cse->exprn);
        for (def = cse->def ; def != NULL ; def = cdr_(def)) {
            BlockHead *c = def->block;
            /* Discard this CSEDef if it is a reference to one already processed */
            bool delete = cseset_member(blklabname_(c), refblocks) &&
                          !blockkills(exid_(cse->exprn), c);

            if (delete) {
                if (debugging(DEBUG_CSE))
                    cc_msg("kill %ld\n", (long)blklabname_(c));
                if (def->refs != NULL) {
                    CSEDef *other;
                    for (other = cse->def ; other != def ; other = cdr_(other))
                        if (cseset_member(blklabname_(c), other->uses))
                            break;
                    if (def == other) syserr(syserr_linkrefstodefs);
                    discarddef(def, other);
                }
            } else {
                linkrefs(cse, def);
                delete = (def->refs == NULL);
            }
            if (delete)
                *prevp = cdr_(def);
            else {
                refblocks = cseset_union(refblocks, def->uses);
                prevp = &cdr_(def);
            }
        }
        cseset_discard(refblocks);
    }
}

static bool safetolift(Exprn *ex)
{
    J_OPCODE op = exop_(ex);
    switch (extype_(ex)) {
    case E_UNARYK:
        return YES;
    case E_BINARY:
        if (op == J_CHKUR || op == J_CHKLR || op == J_CHKNEFR || op == J_CHKNEDR)
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
        if (((op & ~(J_UNSIGNED|J_SIGNED)) == J_DIVK ||
             (op & ~(J_UNSIGNED|J_SIGNED)) == J_REMK)  &&  e2k_(ex) == 0)
            return NO;
        /* drop through -- what about floating pt ? */

    case E_UNARY:
        return safetolift(e1_(ex));

    default:
    case E_MISC:
        syserr(syserr_safetolift);

    case E_CALL:
        return NO;

    case E_LOAD:
        {   Location *loc = exloc_(ex);
            if (loctype_(loc) < LOC_MEM) return YES;
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
    for (p = all_loops ; p != NULL ; p = p->llcdr) {
        BlockHead *b = p->llblkhd;
        BlockList *bl;
        VRegSetP w = cseset_copy(blk_wantedlater_(b));
        VRegSetP we = cseset_copy(blk_wantedonallpaths_(b));
        VRegSetP u = NULL;
        for (bl = p->llblklist ; bl != NULL ; bl = bl->blklstcdr) {
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
            for (bl = p->llblklist ; bl != NULL ; bl = bl->blklstcdr)
                cc_msg(" %ld", (long)blklabname_(bl->blklstcar));
            cc_msg(": safe"); printset(we);
            cc_msg("; unsafe"); printset(w);
            cc_msg("\n");
        }
        cseset_map(we, addcse, (VoidStar)b);
        cseset_map(w, addifsafe, (VoidStar)b);
    }
}

static BindList *nconcbl(BindList *l, BindList *bl)
{
/* Returns its first argument, adjusted to have as its tail the BindList bl if it
   didn't on entry (Bindlists in the places being adjusted share tails, so after
   the first has been adjusted many others will already be right).
 */
    BindList *p = l;
    BindList *prev = NULL;
    for ( ; p != NULL ; prev = p, p = p->bindlistcdr )
        if (p == bl) return l;
    if (prev == NULL) return bl;
    prev->bindlistcdr = bl;
    return l;
}

static Icode *storecse2(Icode *new, VRegnum r1, Binder *binder)
{
    new = new+1;
    new->op = floatinessofreg(r1, J_STRV, J_STRFV, J_STRDV);
    new->r1.r = r1;
    new->r2.r = GAP;
    new->m.b = binder;
    return new;
}

static Icode *storecse(Icode *new, CSEDef *def)
{ /*  ADCONs etc get treated specially here, in that rather than store
      into the CSE binder, we use an ADCON with the register for the CSE
      binder as r1.  This is so that, if the binder gets spilled, the ADCON
      is voided.  (The load of the CSE register is inserted before the
      original instruction, so that can be turned into a use of the CSE
      in case it isn't spilled).
   */
    VRegnum r1 = new->r1.r;
#ifdef TARGET_HAS_DIVREM_FUNCTION
    if (exop_(def->exprn) == J_CALL2KP) {
        VRegnum r2; Icode *call = new-1;
        if (new->op == J_CALL2KP || call->op == J_CALL2KP)
        /* First case for lifting out of loop; second normally */
            r2 = virtreg(R_P1+1,INTREG), r1 = virtreg(R_P1,INTREG);
        else
        /* Local CSE def which is a non-local ref */
        /* @@@ presumably, this should never happen now */
            r2 = call->r1.r;
        new = storecse2(new, r2, def->binder2);
    }
#endif
    if (j_is_check(exop_(def->exprn)))
        return new;

    if (def->binder == NULL)
        syserr(syserr_storecse, (long)blklabname_(def->block),
                                (long)exid_(def->exprn));
    if (pseudo_reads_r2(new->op)) {
        VRegnum b = bindxx_(def->binder);
        Icode *ic = new+1;
        *ic = *new;
        ic->r2.r = new->r1.r = b;
        if (new->r2.r != GAP) {
            forget_slave(ic->r1.r, new->r2.r);
            note_slave(b, new->r2.r);
        }
        note_slave(ic->r1.r, b);
        return ic;
    } else if (pseudo_reads_r1(new->op)) {
        VRegnum b = bindxx_(def->binder);
        Icode *ic = new+1;
        *ic = *new;
        new->op = exop_(def->exprn);
        new->r2.r = GAP;
        ic->r1.r = new->r1.r = b;
        return ic;
    } else
        return storecse2(new, r1, def->binder);
}

static int32 defsize(CSEDef *defs)
{
    int32 ndefs = 0;
    for (; defs != NULL; defs = cdr_(defs))
#ifdef TARGET_HAS_DIVREM_FUNCTION
        if (exop_(defs->exprn) == J_CALL2KP)
            ndefs += 2;
        else
#endif
#ifdef RANGECHECK_SUPPORTED
        if (!j_is_check(exop_(defs->exprn)))

#endif
            ndefs++;
    cse_count += ndefs;
    return ndefs;
}

typedef struct CopyList {
    struct CopyList *cdr;
    Icode  icode;
    struct CopyCSE *cse;
    Exprn  *exprn;
} CopyList;

typedef struct CopyListList {
    struct CopyListList *cdr;
    CopyList *p;
} CopyListList;

typedef struct CopyCSE {
    struct CopyCSE *cdr;
    CopyList *def;
    CopyListList *refs;
    CSEDef *csedef;
} CopyCSE;

static CopyCSE *addeddefs;

static CopyList *copylist(CopyList *cl, Exprn *exprn,
                          J_OPCODE op, VRegnum r1, VRegnum r2, int32 m)
{
    CopyList *q = Allocate(CopyList);
    cdr_(q) = cl; q->cse = NULL; q->exprn = exprn;
    q->icode.op = op; q->icode.r1.r = r1; q->icode.r2.r = r2; q->icode.m.i = m;
    return q;
}

static CopyList *addexprnsbelow(CopyList *cl, Exprn *exprn, CopyList **clp, int32 *callcount);

static VRegnum aeb_argumentreg(CopyList **cl, Exprn *e, int32 *callcount)
{
    CopyList *arg;
    *cl = addexprnsbelow(*cl, e, &arg, callcount);
    if (arg->cse == NULL)
        return arg->icode.r1.r;
    else {
        VRegnum oldr = arg->icode.r1.r,
                newr = vregister(regtype_(oldr));
        if (pseudo_reads_r2(exop_(e)))
            *cl = copylist(*cl, NULL,
                           arg->icode.op,
                           newr, GAP, arg->icode.m.i);
        else
            *cl = copylist(*cl, NULL,
                           floatinessofreg(oldr, J_LDRV, J_LDRFV, J_LDRDV),
                           newr, GAP, 0);
        arg->cse->refs = (CopyListList*)CSEList2(arg->cse->refs, *cl);
        return newr;
    }
}

static CopyCSE *mk_CopyCSE(CopyList *p)
{
    CopyCSE *q = Allocate(CopyCSE);
    cdr_(q) = addeddefs; q->def = p;
    q->refs = NULL; q->csedef = NULL;
    addeddefs = q;
    return q;
}

static CopyList *addexprnsbelow(CopyList *cl, Exprn *exprn, CopyList **clp, int32 *callcount)
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
        J_OPCODE x = op & ~(J_SIGNED | J_UNSIGNED);
        VRegnum r1 =
#ifdef RANGECHECK_SUPPORTED
                     j_is_check(op) ? GAP :
#endif
                     vregister((x == J_FIXFR || x == J_FIXDR) ? INTREG :
                               regtypeofop(op));
        VRegnum r2 = GAP;
        int32 m = 0;
        switch (extype_(exprn)) {
        case E_UNARYK:
            m = exprn->u.unaryk.m;
            break;
        case E_UNARY:
            m = aeb_argumentreg(&cl, e1_(exprn), callcount);
            break;
        case E_BINARYK:
            r2 = aeb_argumentreg(&cl, e1_(exprn), callcount);
            m = exprn->u.binaryk.m;
            break;
        case E_BINARY:
            r2 = aeb_argumentreg(&cl, e1_(exprn), callcount);
            m = aeb_argumentreg(&cl, e2_(exprn), callcount);
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
                switch (loctype_(loc)) {
                case LOC_VAR:
                case LOC_PVAR:
                    m = (int32) locbind_(loc);
                    op += (J_LDRV - J_LDRK);
                    break;
                default:
                    {   Exprn *base = locbase_(loc);
                        if (locrealbase_(loc)) {
                          /* an untransformed load: must be copied as it stands */
                            r2 = aeb_argumentreg(&cl, base, callcount);
                            m = locoff_(loc);
                        } else {
                          /* base must be one of ADDR, SUBR or ADCONV */
                            J_OPCODE baseop = exop_(base);
                            switch (baseop & ~J_SHIFTMASK) {
                            case J_SUBR:
                                op |= J_NEGINDEX;
                            case J_ADDR:
                                op |= (baseop & J_SHIFTMASK);
                                op = J_KTOR(op);
                                r2 = aeb_argumentreg(&cl, e1_(base), callcount);
                                m = aeb_argumentreg(&cl, e2_(base), callcount);
                                break;
                            case J_ADCONV:
                                m = (int32)e1_(base);
                                r2 = locoff_(loc);
                                op = J_addvk(op);
                                break;
                            default:
                                syserr(syserr_baseop, (long)baseop);
                            }
                        }
                    }
                }
            }
            break;
        case E_CALL:
            {   int32 n = exnargs_(exprn);
                int32 i;
                CopyList *arglist = NULL;
                (*callcount)++;
                for (i = 0 ; i < n ; i++) {
                    Exprn *thisarg = exarg_(exprn, i);
                    Exprn *nextarg;
                    VRegnum argreg = virtreg(R_P1+i, INTREG);
                    if ( exop_(thisarg) == CSE_WORD1 &&
                         (i+1) < n &&
                         exop_(nextarg = exarg_(exprn, i+1)) == CSE_WORD2 &&
                         e1_(thisarg) == e1_(nextarg) ) {
                        RegList *rl;
                        cl = addexprnsbelow(cl, e1_(thisarg), &arg, callcount);
                        cl = copylist(cl, NULL,
                                      floatinessofreg(arg->icode.r1.r,
/* These look like unclear assumptions on what cg.c does with args.     */
/* Harry assures AM that this code is just concerned with lifting       */
/* fns with fp args and no side-effects out of loops!                   */
/* Accordingly there is no J_PUSHR case.                                */
                                                      0, J_PUSHF, J_PUSHD),
                                      arg->icode.r1.r, GAP, 0);
                        rl = (RegList *) mkRegList(
                               (RegList *) mkRegList(NULL, argreg),
                               virtreg(R_P1+i+1, INTREG));
                        arglist = copylist(arglist, NULL,
                                      J_POP, GAP, GAP, (int32) rl);

                        i++;
                    } else if (exop_(thisarg) == CSE_WORD1 || exop_(thisarg) == CSE_WORD2)
                        syserr(syserr_cse_wordn);
                      else {
                        VRegnum r = aeb_argumentreg(&cl, exarg_(exprn, i), callcount);
                        arglist = copylist(arglist, NULL, J_MOVR, argreg, GAP, r);
                    }
                }
                cl = (CopyList *)nconc(dreverse((List *)arglist), (List *)cl);
                {   int32 restype = exfntype_(exprn);
                    r1 = V_Presultreg(restype);
                    cl = copylist(cl, exprn, op, r1, i, (int32) exfn_(exprn));
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

static CopyList *icodetocopy_i(CSEDef *defs, int32 *callcount)
{
    CopyList *res = NULL, *p;
    addeddefs = NULL;
    for ( ; defs != NULL ; defs = cdr_(defs))
        if (defs->icode == NULL) {
            res = addexprnsbelow(res, defs->exprn, &p, callcount);
            if (p->cse == NULL) {
                p->cse = mk_CopyCSE(p);
            }
            p->cse->csedef = defs;
        }
    return (CopyList *)dreverse((List *)res);
}

static CopyList *icodetocopy(BlockHead *b, CSEDef *defs, BindList *bl, int32 *ndefs)
{   int32 callcount = 0;
    CopyList *c = icodetocopy_i(defs, &callcount);
    int32 n = length((List *) c);
    {   BindList **bp = &bl->bindlistcdr;
        for (; addeddefs != NULL; addeddefs = cdr_(addeddefs))
            if (addeddefs->csedef == NULL) {
        /* Observe that if we're doing this, bl must be non-null - so
         * adding new binders generated here after the first element of
         * bl is safe (and saves trouble with the code for modifying
         * SETSPENVs).
         */
            VRegnum r1 = addeddefs->def->icode.r1.r;
            CSEDef *csedef = mkCSEDef(addeddefs->def->exprn, b);
            J_OPCODE op = addeddefs->def->icode.op;
            if (op == J_CALL2KP) {
                csedef->binder2 = addcsebinder(op, bp, r1);
                n++;
            }
            csedef->icode = &addeddefs->def->icode;
            if (!j_is_check(csedef->icode->op)) {
                csedef->binder = addcsebinder(op, bp, r1);
                n++;
            }
            addeddefs->csedef = csedef;
        }
    }
    if (callcount == 1 && !(blkflags_(b) & BLKCALL))
        blkflags_(b) |= BLKCALL;
    else if (callcount >= 1)
        blkflags_(b) |= BLKCALL | BLK2CALL;
    *ndefs += n;
    return c;
}

static Icode *copyicode(Icode *new, CopyList *c) {
    for ( ; c != NULL ; c = cdr_(c)) {
        *new = c->icode;
        if (c->cse != NULL) {
            CopyListList *refs = c->cse->refs;
            Binder *b = c->cse->csedef->binder;
            if (b != NULL) new = storecse(new, c->cse->csedef);
            for (; refs != NULL; refs = cdr_(refs))
#ifdef RANGECHECK_SUPPORTED
                if (b == NULL)
                    refs->p->icode.op = J_NOOP;
                else
#endif
                if (pseudo_reads_r2(c->icode.op)) {
                    refs->p->icode.r2.r = bindxx_(b);
                    note_slave(refs->p->icode.r1.r, bindxx_(b));
                } else
                    refs->p->icode.m.b = b;
        }
        new++;
    }
    return new;
}

static BindList *referencecsedefs(BindList *bl, CSEDef *def)
{
    for ( ; def != NULL ; def = cdr_(def))
        if (def->refs != NULL) {
            CSERef *ref = def->refs;
#ifdef RANGECHECK_SUPPORTED
            if (j_is_check(exop_(def->exprn))) {
                def->binder = NULL;
                if (debugging(DEBUG_CSE))
                    cc_msg("\n: ");
            } else
#endif
            {   VRegnum r1 = useicode_(refuse_(ref)).r1.r;
                J_OPCODE op = exop_(def->exprn);
                def->binder = addcsebinder(op, &bl, r1);
                if (debugging(DEBUG_CSE))
                    cc_msg("\n$b [%ld]: ", def->binder, (long)bindxx_(def->binder));
#ifdef TARGET_HAS_DIVREM_FUNCTION
                if (op == J_CALL2KP) {
                    def->binder2 = addcsebinder(op, &bl, r1);
                    if (debugging(DEBUG_CSE))
                        cc_msg("$b [%ld]: ", def->binder2, (long)bindxx_(def->binder2));
                }
#endif
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
    return bl;
}

static void addcsedefstoblock(CSEDef *def)
{
    CSEDef *next;
    for (; def != NULL; def = next) {
        next = cdr_(def);
        if (def->refs != NULL) {
            BlockHead *b = def->block;
            if (blkflags_(b) & BLKOUTER) {
            /* for expressions lifted out of the fake outer "loop", we wish
               to lift them not to the loop preheader, but to the nearest block
               which dominates all references.
             */
                CSERef *ref = def->refs;
                VRegSetP d = cseset_copy(blk_dominators_(refuse_(ref)->block));
                for (; (ref = cdr_(ref)) != NULL; )
                    cseset_intersection(d, blk_dominators_(refuse_(ref)->block));
                for (b = top_block; b != NULL; b = blkdown_(b))
                    if (cseset_member(blklabname_(b), d) &&
                        cseset_equal(d, blk_dominators_(b)))
                        break;
                if (b == NULL)
                    syserr(syserr_addcsedefs);
                {  /* if the block to which we've decided to lift the expression
                      contains a reference, it's not a lifted CSE.  There may be
                      many references in the block, thanks to an amalgamated local
                      CSE.
                    */
                    CSERef **prev = &def->refs;
                    CSERef **defpref = NULL;
                    Icode *deficode = NULL;
                    for (; (ref = *prev) != NULL; prev = &cdr_(ref))
                        if (b == refuse_(ref)->block) {
                            Icode *reficode = &useicode_(refuse_(ref));
                            if (deficode != NULL && deficode < reficode)
                                continue;
                            defpref = prev;
                            deficode = reficode;
                        }
                    if (deficode != NULL) {
                        def->icode = deficode;
                        *defpref = cdr_(*defpref);
                    }
                }
                if (debugging(DEBUG_CSE))
                    if (!(blkflags_(b) & BLKOUTER))
                        cc_msg("%ld %slifted) : %ld\n",
                            (long)exid_(def->exprn),
                            (def->icode == NULL ? "(" : "(not "),
                            (long)blklabname_(b));
            }
            {   CSEDef **prevp = &blk_defs_(b);
                CSEDef *p = blk_defs_(b);
                /* CSEDefs are ordered by the position of their definition within
                   the block, with those corresponding to lifted expressions (where
                   there is no definition in the block) coming last.
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

static BindList *modifycode(void)
{
    CSE *cse;
    BindList *bl = NULL;
    /* Hang CSE definitions off the heads of the blocks containing them
     * (sorted by the order of their occurrence in the block).
     */
    for (cse = cselist ; cse != NULL ; cse = cdr_(cse))
        addcsedefstoblock(cse->def);
    addcsedefstoblock(localcsedefs);

    {   BlockHead *b;
        for (b = top_block; b != NULL; b = blkdown_(b))
             bl = referencecsedefs(bl, blk_defs_(b));
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
                CopyList *c = icodetocopy(b, defs, bl, &ndefs);
                     /* which updates ndefs by the number of JopCodes to lift */
                int32 oldlength = blklength_(b);
                Icode *new = newicodeblock(oldlength+ndefs);
                Icode *old = blkcode_(b);
                int32 i;
                blkcode_(b) = new;
                for ( i = 0 ; i != blklength_(b) ; i++ ) {
                    *new = old[i];
                    while (defs != NULL && (&old[i]) == defs->icode) {
                    /* At the moment, there may be both a non-local and
                       a local def for the same icode (if the exprn was
                       killed earlier in the block)
                     */
                        new = storecse(new, defs);
                        defs = cdr_(defs);
                    }
                    new++;
                }
                if (defs != NULL) {
                    if ( oldlength != 0 &&
                         (blkflags_(b) & (BLKSWITCH | BLK2EXIT))) {
                        new = copyicode(new-1, c);
                        *new++ = old[i-1];
                    } else
                        new = copyicode(new, c);
                }
                if (oldlength != 0) freeicodeblock(old, oldlength);
                blklength_(b) +=ndefs;
                if ((new - blkcode_(b)) != blklength_(b))
                    syserr(syserr_modifycode, (long)blklabname_(b),
                              (long)(new - blkcode_(b)), (long)blklength_(b));
            }
        }
    }
    {   int32 i = blklength_(top_block);
        Icode *new = newicodeblock(i+1);
        Icode *old = blkcode_(top_block);
        blkcode_(top_block) = new;
        if (i != 0) memcpy(new, old, (int) i*sizeof(Icode));
        freeicodeblock(old, i);
        new[i].op = J_SETSPENV;
        new[i].r1.r = GAP;
        new[i].r2.bl = (BindList *) NULL;
        new[i].m.bl = bl;
        blklength_(top_block) = i+1;
    }
    greatest_stackdepth += sizeofbinders(bl, YES);
    return csespillbinders;
}

static clock_t csetime;

static void cse_setup()
{
    memclr(locindex, LOCINDEXSIZE * sizeof(Location **));
    memclr(exprnindex, EXPRNINDEXSIZE * sizeof(Exprn **));
    csespillbinders = NULL;
    setsplist = NULL;
    vregset_init();
    cselist = NULL; localcsedefs = NULL;
}

static BindList *cse_eliminate_i()
{   BlockHead *p;
    clock_t t0 = clock();
    clock_t ts = t0;
    int32 refs = cse_refs, count = cse_count;

    nsets = 0; newsets = 0; setbytes = 0;
    end_emit();

    phasename = "CSELoops";
    cse_setup();
    for (p = top_block; p != NULL; p = blkdown_(p))
        p->cse = new_CSEBlockHead();

    finddominators();
    findloops();

    {   LoopList *lp;
        for (lp = all_loops; lp != NULL; lp = lp->llcdr) {
            BlockList *bp = lp->llblklist;
            for (; bp != NULL; bp = bp->blklstcdr) blknest_(bp->blklstcar)++;
        }
    }

    if (debugging(DEBUG_CSE | DEBUG_STORE)) {
        cc_msg("dominators found - %d csecs\n", clock()-t0);
    }

    if (!cse_enabled ||
        usrdbg(DBG_LINE))
      /* HCM. There used to be a check for (procflags & BLKSETJMP) &&
         (feature & FEATURE_UNIX_STYLE_LONGJMP), but that seems misguided.
       */
        return NULL;

    phasename = "CSE_Available";
    cse_scanblocks(top_block);

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
                    !cseset_equal(oldwantedonallpaths, blk_wantedonallpaths_(p)))
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
     *  available    as before
     *  wantedlater  the set of expressions evaluated by some subsequent block
     *               and not killed on any path from here to the evaluating
     *               block.
     *  wantedonallpaths the set of expressions evaluated by some block on each
     *               path from the block, and not killed between here and there.
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
        if (debugging(DEBUG_CSE) && var_cse_enabled > 18) {
            cc_msg("L%li:", (long)blklabname_(p));
            if (blkflags_(p) & BLKLOOP) cc_msg("*");
            cc_msg(" w"); printset(blk_wanted_(p));
            cc_msg(" wl"); printset(blk_wantedlater_(p));
            cc_msg(" we"); printset(blk_wantedonallpaths_(p));
            cc_msg(" k"); if (blk_killedinverted_(p)) cc_msg("~"); printset(blk_killed_(p));
            cc_msg(" a"); printset(blk_available_(p)); cc_msg("\n");
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
    linkrefstodefs();
    if (debugging(DEBUG_CSE | DEBUG_STORE)) {
        clock_t now = clock();
        cc_msg("cse references linked - %d csecs\n", now-t0);
        t0 = now;
    }
    phasename = "CSEeliminate";
    {   BindList *bl = modifycode();
        if (debugging(DEBUG_CSE | DEBUG_STORE)) {
            clock_t now = clock();
            cc_msg("%ld CSEs, %ld references - %d csecs, CSE total %d\n",
                        (long)(cse_count-count), (long)(cse_refs-refs),
                        now-t0, now-ts);
            csetime += (now - ts);
        }
        drop_local_store();
        if (nsets > maxsets) maxsets = nsets;
        if (setbytes > maxbytes) maxbytes = setbytes;
        return bl;
    }
}

#ifdef CG_FINDS_LOOPS
void note_loop(BlockList *b, BlockHead *c)
{   /* use BindAlloc store, even though SynAlloc may do one day */
    LoopList *p = (LoopList *) BindAlloc(sizeof(LoopList));
    p->llcdr = all_loops, p->llblklist = b, p->llblkhd = c;
    all_loops = p;
}
#endif

extern BindList *cse_eliminate(void)
{
    BindList *bl = cse_eliminate_i();
    if (debugging(DEBUG_CSE) && var_cse_enabled > 16) {
        BlockHead *p;
        cc_msg("\n\ntransformed to:\n\n");
        for (p = top_block; p != NULL; p = blkdown_(p)) {
            Icode    *c, *limit;
            cc_msg("L%li:\n", (long)blklabname_(p));
            for (c = blkcode_(p), limit = c + blklength_(p); c < limit; ++c)
                print_jopcode(c->op, c->r1, c->r2, c->m);
            cse_printexits(blkflags_(p), blknext_(p), blknext1_(p));
        }
    }
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
