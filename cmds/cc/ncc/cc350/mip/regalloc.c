/*
 * C compiler file mip/regalloc.c, version 64
 * Copyright (C) Codemist Ltd., 1988.
 * Copyright (C) Acorn Computers Ltd., 1988.
 */

/*
 * RCS $Revision: 1.1 $
 * Checkin $Date: 1993/11/23 16:41:52 $
 * Revising $Author: nickc $
 */

/* AM memo: tidy up the use of 'curstats'.                              */

/* AM whinge: Recently I changed the type of RealRegSet to allow        */
/* machines with more than 32 registers.  I found this very hard to do  */
/* reliably due to the vast number of VoidStar formal parameters        */
/* meaning it is impossible to find all occurrences.  Please can we     */
/* try to get rid of some?  Unions would be MUCH safer.  My view is     */
/* that VoidStar should really only be used where it is essential       */
/* and not to reduce type-checking (which indirectly increases the cost */
/* of modifications and the chances of buggy releases.                  */
/* Key suspicion: mapping fns are defined VoidStar for pseudo-          */
/* polymorphism and then only used at one type.                         */

#ifndef NO_VERSION_STRINGS
extern char regalloc_version[];
char regalloc_version[] = "\nregalloc.c $Revision: 1.1 $ 64\n";
#endif

/* The following line is initially here for documentation, however, observe */
/* that more store is used calculating deadbits (liveness info cannot be    */
/* discarded as soon).  MEMO: find how much store may be consumed between   */
/* the two TARGET_NEEDS_DEADBITS in allocate_registers().                   */
/* As a test, setting TARGET_NEEDS_DEADBITS on the ARM took 8 secs longer   */
/* out of 400 (increase in regalloc) and required 7K more local store       */
/* (although no new segments) when compiling the compiler.                  */

#define TARGET_NEEDS_DEADBITS 1

/* WGD,AM 7-1-88 Op deadbits bug corrected - when several live virtual regs
   are allocated to the same real register (this is encouraged by the
   voiding of copy instructions) - then only the final occurrence
   of the set should be recorded as dead, not the final occurrence of each
   member separately.  New routine update_deadflags handles this.
*/
/* The simplest code AM can produce to exhibit this is:             */
/*   extern int z; void f(x) { while (x) z=1; while (x) z=1; }      */

/* Experimental test of using SynAlloc/syn_list2 here wasted space.      */

/* Following Nov 87 comment referenced below.                            */
/* Nov 87: Dataflow determination of dead registers introduced to allow  */
/* store-to-store ops to be peepholed in (e.g. x = y; vs. x = y = z;)    */
/* and use of ibm 370 RX arithmetic operations.                          */
/* Dead regs are indicated by the J_DEAD_Rx bits.  Two points:           */
/* 1. We do not claim that *ALL* dead regs are detected -- it seems      */
/*    overkill to detect it for (e.g. CALLR).  However, we do guarantee  */
/*    that J_DEAD_Rx guarantees that Rx fields may be corrupted, but...  */
/* 2. A (real) register may be marked dead, but become alive again by    */
/*    being set in the R1 field.  As to why this is so, and why nothing  */
/*    can sensibly be done about it, observe that                        */
/*    f(int x) { register int a = x; ... } will produce something like   */
/*    MOVR rt, rx; MOVR ra, rt.  Note that (if x is not used further)    */
/*    then the first MOVR will mark R3 (rx) as dead and the second MOVR  */
/*    also R3 (now rt).  Now either or both of these MOVR's may be       */
/*    killed by the mapping of virt. regs to (possibly same) phys. regs. */

/* The use of RetImplLab is nasty here (equivalent to RetVoidLab).       */

/* move towards this code being machine independent apart from a few
   parameters, such as 'number of registers'.
*/

#include <limits.h>
#include <time.h>
#ifdef __STDC__
#  include <string.h>
#else
#  include <strings.h>
#endif
#include "globals.h"
#include "regalloc.h"
#include "errors.h"
#include "sem.h"      /* princtype -- @@@ yuk, lets use abstract interface */
#include "jopcode.h"
#include "regsets.h"
#include "store.h"
#include "cg.h"
#include "mcdep.h"    /* immed_cmp, usrdbg(xxxx) */
#include "flowgraf.h" /* top_block, bottom_block */
#include "builtin.h"  /* sim */
#include "aeops.h"    /* bitofstg_(), s_register - sigh */


#define ClashAllocType AT_Syn
#define ListAllocType AT_Syn
#define CopyAllocType AT_Syn

/* extra value for the r->realreg field... */
#define R_UNSCHEDULED (-1L)   /* not scheduled for allocation yet         */
#define R_SCHEDULED   (-2L)   /* now scheduled for allocation             */
#define R_WILLFIT     (-3L)   /* allocatable, but not yet assigned        */
                              /* (now defunct: future spilling considered */
                              /*  harmful)                                */
#define R_SPILT       (-4L)   /* need to spill onto stack                 */
#define R_BOGUS       (-5L)   /* used transiently: != SPILT, != WILLFIT   */

typedef struct VRegister
{
    VRegnum rname;
    VRegSetP clash2;
    union { int32 nclashes; Binder *spillbinder;} u;
    RealRegister realreg;
    int32 heapaddr;                /* effectively the inverse permutation */
    struct VRegister *perm;
    int32 ncopies;
    int32 refcount;
    VRegnum slave;
} VRegister;

#define vregname_(r) regname_((r)->rname)
#define vregtype_(r) regtype_((r)->rname)

RealRegSet 	regmaskvec;	/* registers used or corrupted in this proc */
#ifdef TARGET_IS_C40
RealRegSet	usedmaskvec;	/* registers just used by this proc */
#endif

static RealRegSet m_intregs, m_notpreserved;
#if !defined TARGET_SHARES_INTEGER_AND_FP_REGISTERS || defined TARGET_IS_C40
static RealRegSet m_fltregs;
#endif
#ifdef ADDRESS_REG_STUFF
static RealRegSet m_addrregs;
#endif

static int32 n_real_spills, n_cse_spills,        /* these seem to be real.. */
             tot_real_spills, tot_cse_spills,    /* while these are just for. */
             spill_cost, tot_cost, choose_count; /* the trace option.   */


/* We could speed up the following routines by #if (NMAGICREGS<=32)?    */
/* The following routines take pointers to RealRegSets as args even     */
/* though conceptually RealRegSets should be passed by value.           */

static bool member_RealRegSet(const RealRegSet *a, unsigned32 r)
{   return ((a->map)[r/32] & regbit(r % 32)) != 0;
}

void augment_RealRegSet(RealRegSet *a, unsigned32 r)
{   (a->map)[r/32] |= regbit(r % 32);
}

static void delete_RealRegSet(RealRegSet *a, unsigned32 r)
{   (a->map)[r/32] &= ~regbit(r % 32);
}

static bool intersect_RealRegSet(RealRegSet *a, const RealRegSet *b,
                                                const RealRegSet *c)
{   int32 i; bool res = 0;
    for (i = 0; i < (NMAGICREGS+31)/32; i++)
        if (((a->map)[i] = (b->map)[i] & (c->map)[i]) != 0) res = 1;
    return res;
}

#ifdef TARGET_SHARES_INTEGER_AND_FP_REGISTERS

static bool two_bits(unsigned32 w)
{   /* Used to test if an even/odd register pair is available */
    int i;
    for (i=0; i<31; i++)
    {   if ((w & 3) == 3) return YES;
        else w = w >> 2;
    }
    return NO;
}

static bool nonempty_RealRegSet(const RealRegSet *a, int32 type)
{   int32 i; bool res = 0;
/* I demand an even/odd pair for float regs as well as double ones     */
/* because doing so (slightly) simplifies allocation and because float */
/* registers will not often arise in C anyway.  Fix up later maybe.    */
    if (type == FLTREG || type == DBLREG)
    {
      for (i = 0; i < (NMAGICREGS+31)/32; i++)
	if (two_bits((a->map)[i]))
	  {
	    res = 1;
	  }
    }
    else
    {   for (i = 0; i < (NMAGICREGS+31)/32; i++)
            if ((a->map)[i] != 0) res = 1;
    }
    return res;
}

#else /* TARGET_SHARES_INTEGER_AND_FP_REGISTERS */
static bool nonempty_RealRegSet(const RealRegSet *a)
{   int32 i; bool res = 0;
    for (i = 0; i < (NMAGICREGS+31)/32; i++)
        if ((a->map)[i] != 0) res = 1;
    return res;
}

#endif /* TARGET_SHARES_INTEGER_AND_FP_REGISTERS */
#ifdef ADDRESS_REG_STUFF
static bool spillpanic;     /* forced to demote A to D register? */
#endif

static VRegSetP globalregvarset;
static RealRegSet globalregvarvec;

static void regalloc_changephase(void);

typedef union vreg_type { VRegister *vreg; VRegnum type; } vreg_type;

static vreg_type (*(vregheap[REGHEAPSEGMAX]))[REGHEAPSEGSIZE];

#define vreg_(n) (*vregheap[((n) & 0xffffffL)>>REGHEAPSEGBITS]) \
                           [(n)&(REGHEAPSEGSIZE-1)].vreg

static unsigned32 vregistername;
#define vregtypetab_(n) (*vregheap[(n)>>REGHEAPSEGBITS]) \
                                  [(n)&(REGHEAPSEGSIZE-1L)].type
#define vregtable_(n)   (*vregheap[(n)>>REGHEAPSEGBITS]) \
                                  [(n)&(REGHEAPSEGSIZE-1L)].vreg
/* Note that vregtable_() is set and then never updated -- permregheap_() is */
#define permregheap_(n) (vregtable_((n)+NMAGICREGS)->perm)

/*
 * Statistics-gathering stuff.
 */

typedef struct RegStats {
    unsigned32  dataflow_iterations;
    unsigned32  ncopies;
    unsigned32  copysquares;
    unsigned32  copysquarebytes;
    unsigned32  nlists;
    unsigned32  newlists;
    unsigned32  listbytes;
    unsigned32  nvregs;
    unsigned32  vregbytes;
    unsigned32  nsquares;
    unsigned32  squarebytes;
    unsigned32  nregsets;
    unsigned32  newregsets;
    unsigned32  regsetbytes;
    unsigned32  clashbytes; /* total of clashmatrix/regset stuff */
} RegStats;

static RegStats curstats, maxstats;

static clock_t regalloc_clock1, regalloc_clock2, dataflow_clock;

extern RealRegister register_number(VRegnum a)
{
   return vreg_(regname_(a))->realreg;
}

/* TARGET_IS_NULL is defined when the system being built is a Lint-like   */
/* checker or some source-to-source conversion utility. It is desirable   */
/* to keep enough of the register allocation code that data-flow oddities */
/* such as unused variables can be reported on, but much of the rest can  */
/* (and should) be skipped as unnecessary.                                */

#ifndef TARGET_IS_NULL

/* first routines to keep the register heap in order of number of clashes */
/* so allocation can proceed from the most(?) clashing register first.    */
/* Note that these permute the vregheap table.                            */

/* Comparison routine for deciding heap-orderedness property.             */
/* Use ->nclashes as primary key, but experimentally use number of real   */
/* register copies for secondary one in lexicographic order.              */
/* To achieve this in a way which does not change during allocation       */
/* (which would upset the ordering property (but not chaotically))        */
/* we eliminate registers, 1. by least clashes and then 2. by least       */
/* register copies, counting a physical (magic) register many times.      */

static int32 reg_cmp(VRegister *v, VRegister *w)
{   int32 d;
    if ((d = v->u.nclashes - w->u.nclashes) != 0) return d;
    return v->ncopies - w->ncopies;
}

static void downheap(int32 k, int32 n)
{
/* fix up heap property at position k in the register heap               */
/* see Sedgewick's book on algorithms for commentary on this.            */
    VRegister *v = permregheap_(k), *w;
    while (k <= (n/2L))
    {   int32 j = k + k;
        if (j < n)
        {   if (reg_cmp(permregheap_(j),permregheap_(j+1)) > 0) j++;
        }
        if (reg_cmp(v, permregheap_(j)) <= 0) break;
        w = permregheap_(k) = permregheap_(j);
        w->heapaddr = k;
        k = j;
    }
    permregheap_(k) = v;
    v->heapaddr = k;
}

static void upheap(int32 k)
{
    VRegister *v = permregheap_(k), *w;
    int32 k1;
/* NB that there is a sentinel register with nclashes=-1 in the 0        */
/* position of the heap - that simplifies the end-test here.             */
    while (k1 = k/2L, w = permregheap_(k1), reg_cmp(w,v) > 0)
    {   permregheap_(k) = w;
        w->heapaddr = k;
        k = k1;
    }
    permregheap_(k) = v;
    v->heapaddr = k;
}

#define printvregclash(a) print_clashes(a)
#define printvregclash2(a) vregset_print((a)->clash2)

static Relation clashmatrix;

static Relation copymatrix;

static RelationAllocRec clashrallocrec;
/* = {ClashAllocType, &curstats.nsquares, &curstats.squarebytes}; */

static RelationAllocRec copyallocrec;
/* = {CopyAllocType, &curstats.copysquares, &curstats.copysquarebytes}; */

static void clash_reinit(int32 nregs)
{
    curstats.nvregs = nregs;
    clashmatrix = relation_init(&clashrallocrec, nregs, &curstats.vregbytes);
    copymatrix = relation_init(&copyallocrec, nregs, &curstats.vregbytes);
    vregset_init();
}

static void add_clash(VRegnum a, VRegnum b)
{
    int32 ar, br;
#ifndef TARGET_SHARES_INTEGER_AND_FP_REGISTERS
    {   int32 atype = regtype_(a);
        int32 btype = regtype_(b);
#  ifdef ADDRESS_REG_STUFF
        if (atype == ADDRREG) atype = INTREG;
        if (btype == ADDRREG) btype = INTREG;
#  endif
        if (atype != btype)
        {
           if ((atype == INTREG) || (btype == INTREG)) return;
        }
    }
#endif
    ar = regname_(a); br = regname_(b);
    if (ar < br) { int32 tr; VRegnum t = a; a = b; b = t;
                   tr = ar; ar = br; br = tr; }
    if ((br < 0) || (ar >= vregistername)) {
        syserr(syserr_addclash, (long)ar, (long)br);
    }
    if (relation_add(ar, br, clashmatrix, &clashrallocrec))
    {   vreg_(ar)->u.nclashes++;
        vreg_(br)->u.nclashes++;
    }
}

/* clashkillbits_cb is passed as parameter and so needs silly type. */

static void
clashkillbits_cb(
		 int32 		k,
		 VoidStar	arg )
{
  RealRegSet *	m = (RealRegSet *)(int) arg;
  RealRegister	r = vregtable_( regname_( (VRegnum) k ) )->realreg;

  
  if (r < (unsigned32)NMAGICREGS)
#ifdef TARGET_SHARES_INTEGER_AND_FP_REGISTERS
    {
      /* I certainly need to do more here */

      delete_RealRegSet( m, r );
    }
#else
  delete_RealRegSet( m, r );
#endif
}

static void clashkillbits(RealRegSet *m, VRegister *reg)
{
    vregset_map(reg->clash2, clashkillbits_cb, (VoidStar)m);
}

static VRegSetAllocRec clashvallocrec;
/* = {
 *    ClashAllocType,
 *    &curstats.nregsets,
 *    &curstats.newregsets,
 *    &curstats.regsetbytes };
 */

static void removeclashes_rcb(int32 vr, VoidStar arg)
{
    VRegSetP *residual = (VRegSetP *)(int) arg;
    VRegister *clashee;
    clashee = vregtable_(vr);
    clashee->u.nclashes--;
    *residual = vregset_insert(vr, *residual, NULL, &clashvallocrec);
    if (clashee->realreg == R_UNSCHEDULED)
        upheap(clashee->heapaddr);
}

static void removeclashes(VRegister *vreg)
{
    int32 reg = vregname_(vreg);
    VRegSetP residual = NULL;
    relation_mapanddelete(reg, clashmatrix, removeclashes_rcb,
                                            (VoidStar)&residual);
    vreg->clash2 = residual;
    vreg->u.nclashes = 0;
}

static void printvreg(VRegnum vr)
{
    int32 type = regtype_(vr);
    cc_msg(" %c%ld", (type == FLTREG ? 'f' :
                      type == DBLREG ? 'd' :
#ifdef ADDRESS_REG_STUFF
                      type == ADDRREG ? 'a' :
#endif
                      'r'),
                     (long)regname_(vr));
}

static void print_clashes(VRegister *vreg)
{   /* print the set of VRegisters which clash with  vreg */
    int32 reg = vregname_(vreg);
    relation_map1(reg, clashmatrix, printvreg);
}

static void add_copy(VRegnum a, VRegnum b)
{
    int32 anum, bnum;
#ifndef TARGET_SHARES_INTEGER_AND_FP_REGISTERS
    {   int32 atype = regtype_(a);
        int32 btype = regtype_(b);
#ifdef ADDRESS_REG_STUFF
        if (atype == ADDRREG) atype = INTREG;
        if (btype == ADDRREG) btype = INTREG;
#endif
        if (atype != btype)
        {  /* AM believes that atype and btype will always be the same  */
           /* with the possible exception of MOVDFR.  Hence the RETURN  */
           /* below can never be executed.                              */
           if ((atype == INTREG) || (btype == INTREG)) return;
        }
    }
#endif
    anum = regname_(a); bnum = regname_(b);

    if (relation_add(anum, bnum, copymatrix, &copyallocrec))
    {   VRegister *av = vreg_(anum), *bv = vreg_(bnum);
        curstats.ncopies++;
        av->ncopies += bnum<NMAGICREGS ? 0x10000 : 1;
        bv->ncopies += anum<NMAGICREGS ? 0x10000 : 1;
    }
}

typedef void RProc1(VRegnum, RealRegSet *);

static void map_copies(VRegister *vr, RProc1 *proc, RealRegSet *arg)
{
    relation_map(vregname_(vr), copymatrix, (RProc *)proc, (VoidStar)arg);
}


static void vregset_print(VRegSetP set)
{
    vregset_map2(set, printvreg);
}

/*
 * End of abstract data type for reg clash matrix.
 */

#else /* TARGET_IS_NULL */
static void add_clash(VRegnum r1n, VRegnum r2n)
{
    IGNORE(r1n); IGNORE(r2n);
}

static VRegSetP set_register_copy(VRegnum r, VRegSetP s, VRegnum rsource)
{
    IGNORE(r); IGNORE(s); IGNORE(rsource); return NULL;
}

static VRegSetP set_register_slave(VRegnum r, VRegSetP s, VRegnum rmaster)
{
    IGNORE(r); IGNORE(s); IGNORE(rmaster); return NULL;
}

#endif /* TARGET_IS_NULL */

static void stats_print(RegStats *p)
{
    cc_msg("-- passes %2li: copies =%4ld, lists = %4ld, vregs = %4ld\n",
           p->dataflow_iterations, p->ncopies, p->nlists, p->nvregs);
    cc_msg("%19s %6ld %13ld %13ld\n", "space",
           p->copysquarebytes, p->listbytes, p->vregbytes);
    cc_msg("--            squares =%4ld, regsets =%4ld, bytes =%6ld\n",
           p->nsquares, p->nregsets, p->clashbytes);
    cc_msg("%19s %6ld %13ld  total=%6ld\n", "space",
           p->squarebytes, p->regsetbytes, p->clashbytes);
}

static void stats_endproc(void)
{   unsigned32  x, j, *cur, *max;

    curstats.clashbytes = curstats.vregbytes +
                          curstats.squarebytes + curstats.regsetbytes;

    if (curstats.nvregs >= 128) {
        cc_msg("regalloc space stats for big procedure:\n");
        stats_print(&curstats);
    }

    cur = (unsigned32 *)&curstats;
    max = (unsigned32 *)&maxstats;
    for (j = 0; j < sizeof(curstats)/sizeof(unsigned32); ++j)
    {   x = *cur++; if (x > *max) *max = x;
        ++max;
    }
}

/* Consider (p() ? 0:1).  If we generate <test> ... MOVK v,0 ... MOVK v,1  */
/* and 0,1 are put in master regs i0,i1 then we find that v is a slave of  */
/* both i0 and i1. The code in regalloc then causes v not to clash with    */
/* either i0 nor i1 which can allow i0 to share with v in spite of the fact*/
/* that v can  be updated to 1 when i0 is still live.                      */
/* HACK.  Note that things are OK when slave_list is a (reverse) forest    */
/* Hence delete all entries (v,i0), (v,i1) when i0 != i1.                  */
/* Memo to AM: the idea of slave_list would better be replaced with a     */
/* general algorithm which stops x & y clashing in:                       */
/*  f(x) { int y = x; return x+y;}                                        */

#ifndef TARGET_IS_NULL

typedef struct ReadonlyCopy     /* loopopt.c and regalloc.c interface   */
{
  struct ReadonlyCopy *next;
  VRegnum r1;
  VRegnum r2;
} ReadonlyCopy;

static ReadonlyCopy *slave_list;

void note_slave(VRegnum slave, VRegnum master)
{   ReadonlyCopy *p;
    for (p = slave_list; p != NULL; p = p->next)
        if (p->r1 == slave)
        {   if (p->r2 != master) p->r2 = GAP;   /* see above note re trees */
            return;
        }
    /* allocate BindAlloc store to survive into regalloc.c */
    p = (ReadonlyCopy *) BindAlloc(sizeof(ReadonlyCopy));
    p->next = slave_list, p->r1 = slave, p->r2 = master;
    slave_list = p;
}

void forget_slave(VRegnum slave, VRegnum master)
{   ReadonlyCopy *p, **prev;
    for (prev = &slave_list; (p = *prev) != NULL; prev = &p->next)
        if (p->r1 == slave)
        {   if (p->r2 == master) *prev = p->next;
            else if (p->r2 != GAP) syserr(syserr_forget_slave, slave, master, p->r2);
            return;
        }
}

static bool reg_overlord(VRegnum r1, VRegnum r2)
/* true iff (r1,r2) is in the transitive closure of slave_list */
{   for (;;)
    {   VRegnum r = vregtable_(regname_(r1))->slave;
        if (r == r2) return YES;
        else if (r == GAP) return NO;
        r1 = r;
    }
}

#else
void note_slave(VRegnum slave, VRegnum master)
{
    IGNORE(slave);
    IGNORE(master);
}

#endif

static VRegSetAllocRec listallocrec;
/* = {ListAllocType, &curstats.nlists, &curstats.newlists,
 *                   &curstats.listbytes};
 */

static VRegSetP reference_register(VRegnum r, VRegSetP s)
/* s is a list of registers whose value is needed. We have encountered   */
/* (in a backwards scan over a basic block) an instruction that uses the */
/* value of register r.  r gets added to the list of registers that need */
/* to be given a value.                                                  */
/* BEWARE: the result is compared with s to determine if s became live.  */
{
    if (r==GAP) syserr(syserr_GAP);
    else s = vregset_insert(r, s, NULL, &listallocrec);
    return s;
}

/* things for pass2 (the exact clash info) only ... */

static BindList *thisBlocksBindList;

static void makebindersclash(VRegnum r, BindList *bl, VRegnum rx)
{   BindList *ab = argument_bindlist;
    for ( ; ab!=NULL ; ab = ab->bindlistcdr ) {
        VRegnum r1 = bindxx_(ab->bindlistcar);
        if (r1!=GAP && r1!=r && r1!=rx) add_clash(r1, r);
    }

    for ( ; bl!=NULL ; bl = bl->bindlistcdr ) {
        VRegnum r1 = bindxx_(bl->bindlistcar);
        if (r1!=GAP && r1!=r && r1!=rx) add_clash(r1, r);
    }
}

static void setregister_cb(int32 r, VoidStar r1)
{
    add_clash((VRegnum)r, (VRegnum)r1);
}

static void corrupt_register(VRegnum r, VRegSetP s)
/* register r has its value altered by this instruction, but not in a    */
/* useful or predictable way. Ensure that r is not unified with any      */
/* other register currently active                                       */
{
/* AM has thought and sees no real difference between corrupt_register */
/* and set_register */
    if (vregset_member(r, s))
        syserr(syserr_corrupt_register, (long)regname_(r), s);  /* insert to check */
    vregset_map(s, setregister_cb, (VoidStar)r);
#ifdef TARGET_SHARES_INTEGER_AND_FP_REGISTERS
    if (is_physical_double_reg_(r))
        vregset_map(s, setregister_cb, (VoidStar)virtreg(r+1,INTREG));
#endif
#ifdef TARGET_HAS_DEBUGGER
    if (usrdbg(DBG_VAR))
        makebindersclash(r, thisBlocksBindList, GAP);
#endif
}

static void corrupt_physical_register(VRegnum r, VRegSetP s)
{   /* 'r' must be a physical register, corrupt it and add it to        */
    /* regmaskvec.                                                      */
    /* @@@ Shouldn't corrupt_register use regname_(r) to mask?          */
    if (!isany_realreg_(regname_(r))) syserr(syserr_regalloc);
    
    augment_RealRegSet(&regmaskvec, regname_(r));
#ifdef TARGET_SHARES_INTEGER_AND_FP_REGISTERS
    if (is_physical_double_reg_(r))
        augment_RealRegSet(&regmaskvec, regname_(r)+1);
#endif
    corrupt_register(r, s);
}


static VRegSetP set_register(VRegnum r, VRegSetP s)
/* This is called when an instruction that sets r is found (and when the */
/* instruction is not a direct copy instruction, and when then value of  */
/* the register is (expected to be) needed.                              */
{
    /* In the next line, if !member(r,s) we have the dataflow anomaly */
    /* 'register set but value not used'.  In general this will be    */
    /* optimised away, but it also occurs for calls to void functions */
    /* and so is better reported elsewhere (see J_STRV code below).   */
    s = vregset_delete(r, s, NULL);
#ifdef TARGET_SHARES_INTEGER_AND_FP_REGISTERS
    if (is_physical_double_reg_(regname_(r)))
        s = vregset_delete(r+1, s, NULL);
#endif
    vregset_map(s, setregister_cb, (VoidStar)r);
#ifdef TARGET_SHARES_INTEGER_AND_FP_REGISTERS
    if (is_physical_double_reg_(r))
        vregset_map(s, setregister_cb, (VoidStar)(r+1));
#endif
#ifdef TARGET_HAS_DEBUGGER
    if (usrdbg(DBG_VAR))
        makebindersclash(r, thisBlocksBindList, GAP);
#endif
    return s;
}

#ifndef TARGET_IS_NULL

static void record_copy(VRegnum r1n, VRegnum r2n)
/*  somebody has issued a 'MOVR r1, r2' or maybe a 'MOV r2, r1'. Life    */
/* will be distinctly better if I can arrange that r1 and r2 get put in  */
/* the same real register. Make a table of associations to help me to    */
/* achieve this at least some of the time.                               */
{
    if (regname_(r1n)!=regname_(r2n)) add_copy(r1n, r2n);
}

typedef struct {
   VRegnum r;
   VRegnum rsource;
} SRCRecord;

static void setregistercopy_cb(int32 ar, VoidStar arg)
{
    SRCRecord *p = (SRCRecord *)(int)arg;
    VRegnum r = (VRegnum)ar;
    if (r != p->rsource) add_clash(p->r, r);
}

static VRegSetP set_register_copy(VRegnum r, VRegSetP s, VRegnum rsource)
/* This is called when an instruction that copies rsource into r is seen */
{
    SRCRecord sc;
    if (!vregset_member(r, globalregvarset))
        s = vregset_delete(r, s, NULL);
    sc.r = r; sc.rsource = rsource;
/* r conflicts with all live registers EXCEPT rsource.                   */
    vregset_map(s, setregistercopy_cb, (VoidStar) &sc);
#ifdef TARGET_HAS_DEBUGGER
    if (usrdbg(DBG_VAR))
        makebindersclash(r, thisBlocksBindList, rsource);
#endif
    if (debugging(DEBUG_REGS))
        cc_msg("Record copy %ld %ld\n", (long)regname_(r), (long)regname_(rsource));
    record_copy(r, rsource);
    return s;
}

static void setregisterslave_cb(int32 r, VoidStar r1)
{
    if (!reg_overlord((VRegnum)r1, (VRegnum)r))
        add_clash((VRegnum)r, (VRegnum)r1);
}

static VRegSetP set_register_slave(VRegnum r, VRegSetP s, VRegnum rmaster)
/* This is a re-working of slave_list.  Not yet final because it builds  */
/* in the knowledge that code in other parts of the compiler did it.     */
/* r is a slave target register being set.  Arrange that it clashes with */
/* all live registers except its masters.                                */
{
    s = vregset_delete(r, s, NULL);
/* r conflicts with all live registers EXCEPT its masters.               */
    vregset_map(s, setregisterslave_cb, (VoidStar) r);
#ifdef TARGET_HAS_DEBUGGER
    if (usrdbg(DBG_VAR)) {
        BindList *bl = thisBlocksBindList;
        for ( ; bl!=NULL ; bl = bl->bindlistcdr ) {
            VRegnum r1 = bindxx_(bl->bindlistcar);
            if (r1!=GAP && regname_(r1)!=regname_(r) && !reg_overlord(r, r1))
                add_clash(r1, r);
        }
    }
#endif /* TARGET_HAS_DEBUGGER */

    if (debugging(DEBUG_REGS))
        cc_msg("Record slave copy %ld %ld\n", (long)regname_(r), (long)regname_(rmaster));
    record_copy(r, rmaster);
    return s;
}

#endif /* TARGET_IS_NULL */

static VRegSetP instruction_ref_info(
        VRegSetP s1, int32 op, VRegInt r1, VRegInt r2, VRegInt m, int32 *exact)
/* 'exact' is non-0 for the final pass when exact dataflow info is required */
/* It is convenient to arrange it to be a pointer to where J_DEAD_Rx is put */
{   switch (op)
    {
#ifdef TARGET_HAS_DIVREM_FUNCTION
case J_CALL2KP:
#endif
case J_CALLK:
case J_CALLR:
        s1 = vregset_union(s1, globalregvarset, &listallocrec);
        if (!exact)
        {   if (reads_r3(op)) s1 = reference_register(m.r, s1);
        }
        else
        {   int32 i;
#ifndef DO_NOT_EXPLOIT_REGISTERS_PRESERVED_BY_CALLEE
            RealRegSet *u;
            if (!vregset_member(m.r, s1)) *exact |= J_DEAD_R3;
/*
 * At the end of each procedure compiled (in cg.c) a record is made of the
 * set of registers actually corrupted by each procedure.  When later on
 * in the same compilation unit such functions are called the register
 * allocation code can sometimes exploit this.  Note that the information
 * stored is only useful wrt argument and temporary registers - all calls
 * preserve register variables anyway, and that the special registers
 * R_IP and R_LR are not reliably dealt with by the mask.
 * They are getting to be, via use of corrupt_physical_register().
 * Indications are that this optimisation only makes a very small change
 * to code density, but is is not very hard to implement so it nevertheless
 * feels worthwhile.
 */
            if (op == J_CALLR) u = NULL;
#if defined TARGET_IS_HELIOS && defined TARGET_HAS_DEBUGGER
	    /*
	     * XXX - NC - 23/4/93
	     *
	     * We cannot use this optimisation if the code is being compiled
	     * for debugging, as the code generator will (silently) insert
	     * calls to debug functions, even into leaf nodes, and these
	     * functions can and do corrupt argument and temporary registers.
	     */
	    
	    else if (usrdbg( DBG_ANY ))
	      u = NULL;		     
#endif
            else
            {   TypeExpr *t = princtype(bindtype_(m.b));
                if (h0_(t) != t_fnap) syserr(syserr_regalloc_typefnaux);
                u = &(typefnaux_(t).usedregs);
            }
#endif
/* AM: we could argue that the following calls to corrupt_register      */
/* should use a bit map like m_intregs, but this would mean different   */
/* targets might call in varying order, giving minor reg. re-orderings. */
            for (i=0; i<NTEMPREGS; i++)
#ifndef DO_NOT_EXPLOIT_REGISTERS_PRESERVED_BY_CALLEE
                if (u==NULL || R_T1+i == R_IP || member_RealRegSet(u, R_T1+i))
#endif
                    corrupt_physical_register(virtreg(R_T1+i, INTREG), s1);
/* The following test should be micro-efficiency hack.  Check that      */
/* corrupt_register is idempotent.                                      */
/* Note that I have not tested the mask (u) for R_IP since I fear it    */
/* may not always be mentioned there when it needs to be - I need to    */
/* check up somewhat.  AM: improving...                                 */
            if (!(R_T1 <= R_IP && R_IP < R_T1+NTEMPREGS))
                corrupt_physical_register(virtreg(R_IP, INTREG), s1);

            for (i=0; i<NARGREGS; i++)
#ifndef DO_NOT_EXPLOIT_REGISTERS_PRESERVED_BY_CALLEE
                if (u==NULL || member_RealRegSet(u, R_A1+i))
/*
 * I argue here that the result register will get processed here because
 * the callee will be marked as clobbering it.  I think that it may turn out
 * that the best consequence of all of this will be that calling functions
 * which do only integer arithmetic will be seen not to corrupt the
 * floating point registers.
 */
#endif
                    corrupt_physical_register(virtreg(R_A1+i, INTREG), s1);
#ifndef TARGET_SHARES_INTEGER_AND_FP_REGISTERS
            for (i=0; i<NFLTARGREGS; i++)
#ifndef DO_NOT_EXPLOIT_REGISTERS_PRESERVED_BY_CALLEE
                if (u==NULL || member_RealRegSet(u, R_FA1+i))
#endif
                    corrupt_physical_register(virtreg(R_FA1+i, DBLREG), s1);
            for (i=0; i<NFLTTEMPREGS; i++)
#ifndef DO_NOT_EXPLOIT_REGISTERS_PRESERVED_BY_CALLEE
                if (u==NULL || member_RealRegSet(u, R_FT1+i))
#endif
                    corrupt_physical_register(virtreg(R_FT1+i, DBLREG), s1);
#endif
/* If the call is not done using a BL then I have to allow for the fact  */
/* that the target of the call is in a register (m). This register is    */
/* (at least on the ARM) used after LR gets established by the CALLR     */
/* opcode but before the function call mangles any other registers. Thus */
/* the reference to m must be recorded exactly here.                     */
/* Similarly on the MIPS R2000 hardware.                                 */
            if (reads_r3(op)) s1 = reference_register(m.r, s1);
#ifndef TARGET_STACKS_LINK
/* If corrupt_physical_register here, functions containing any call (even */
/* if it later beomes a tailcall) will seem to need to stack LR.  But     */
/* beware regmaskvec not quite correct.                                   */
            corrupt_register(virtreg(R_LR, INTREG), s1);
#endif
        }
/* If there is at least 1 arg, R_A1 is required. similarly A2 to A4.     */
        {   int32 nargs = r2.i & K_ARGWORDMASK;
            if (nargs > NARGREGS) nargs = NARGREGS;
            while (nargs-- > 0)
            {   VRegnum rarg =
#ifdef TARGET_IS_MIPS
                    nargs < 2*(r2.i >> K_FPARG_SHIFT) ?
                        virtreg(R_FA1 + (--nargs)/2,DBLREG) :
#endif
                        virtreg(R_A1+nargs,INTREG);
                s1 = reference_register(rarg, s1);
                if (exact && usrdbg(DBG_VAR))
                   makebindersclash(rarg, thisBlocksBindList, GAP);
            }
#ifdef TARGET_STRUCT_RESULT_REGISTER
/*
 * This is in case there was an implicit extra arg that had been introduced
 * to handle a structure result, and that arg is NOT passed as one of the
 * normal arguments but in its own special register (which must be one of
 * the registers otherwise used as scratch workspace).
 */
            if (r2.i & K_STRUCTRET)
            {   s1 = reference_register(
                       virtreg(TARGET_STRUCT_RESULT_REGISTER,INTREG),
                       s1);
                if (exact && usrdbg(DBG_VAR))
                   makebindersclash(
                       virtreg(TARGET_STRUCT_RESULT_REGISTER, INTREG),
                       thisBlocksBindList, GAP);
            }
#endif
        }
        break;
    case J_COUNT:       /* for the profile option */
        if (exact)
        {
#ifndef TARGET_STACKS_LINK
            corrupt_physical_register(virtreg(R_LR,INTREG), s1);
#endif
            corrupt_physical_register(virtreg(R_IP,INTREG), s1);
        }
        break;
    case J_ADCON:
#ifdef TARGET_IS_RISC_OS
        if (exact && arthur_module) {
            corrupt_physical_register(virtreg(R_IP,INTREG), s1);
        }
#endif
        break;
    case J_ENTER:       /* AM, Jan 91: does this case do anything now?  */
            {   int32 i;
                if (m.i > NARGREGS) m.i = NARGREGS;
                for (i=0; i<m.i; i++)
#ifdef TARGET_IS_MIPS
                  if (i/2 < procinitfltargs)
                    s1 = reference_register(virtreg(R_FP1+i++/2,DBLREG),s1);
                  else
#endif                
                    s1 = reference_register(virtreg(R_P1+i,INTREG), s1);
            }
            break;
    case J_POP:
            {   RegList *p;		
                for (p=m.rl; p!=NULL; p=p->rlcdr)
		  {
                  if (!exact)
		    s1 = vregset_delete(p->rlcar, s1, NULL);
                  else
/* This seems to rely on INTREG==0 and thus regname_(r)==r for int regs. */
                    if (vregset_member(p->rlcar, s1))
                        s1 = set_register(p->rlcar, s1);
/* If I pop a register but am not then going to use it (well really it   */
/* was silly to have pushed it in the first case) I must at least record */
/* the fact that this register gets corrupted by the POP so that I do    */
/* not identify it with some other more useful value-holding register.   */
                    else corrupt_register(p->rlcar, s1)
#if !defined TARGET_IS_C40
		  
		      /*
		       * XXX - NC - 7/2/92
		       *
		       * Horrible hack time.  It seems that the following
		       * code can cause this syserr to be called.  I do
		       * not have the time or ability to track this down
		       * properly, so I am just going to ignore it for now!
		       *
		       * short
		       * func( double x )
		       * {
		       *   int	a;
		       *
		       * 
		       *   return (a / 2.0) * (x / a) + (x / a);
		       * }
		       *
		       * What happens is that the compiler detects the
		       * common sub expression '(x /a)', but it fails
		       * to eliminate the pushing and poping of
		       * registers on to the stack that would normally
		       * be required if the second division were going
		       * to be performed, and hence this error occurs.
		       */
		      
		      , syserr(syserr_regalloc_POP)
#endif
			;
		}
            }
            break;
    default:
            break;
    }
    if (reads_r2(op) ||
        (pseudo_reads_r2(op) && r2.r!=GAP))    /* MOVK/ADCON loopopt */
            {   if (exact && !vregset_member(r2.r, s1)) *exact |= J_DEAD_R2;
                s1 = reference_register(r2.r, s1);
            }
    if (reads_r3(op) && op != J_CALLR)     /* see above */
            {   if (exact && !vregset_member(m.r, s1))
                    *exact |= J_DEAD_R3;
                s1 = reference_register(m.r, s1);
            }
    if (reads_r1(op) ||
        (pseudo_reads_r1(op) && r1.r!=GAP))    /* CMPK loopopt */
            {   if (exact && !vregset_member(r1.r, s1)) *exact |= J_DEAD_R1;
                s1 = reference_register(r1.r, s1);
            }
    return s1;
}

static VRegSetP exitregset(VRegnum r, VRegSetP s)
{
    s = vregset_union(s, globalregvarset, &listallocrec);
    return (r == GAP) ? s : reference_register(r, s);
}

static VRegSetP extra_regs(VRegSetP s, LabelNumber *q)
/* update the list s to reflect things needed by the block with label q. */
/* Note that q can have one of the special values RetIntLab, RetFloatLab */
/* or RetVoidLab (for exit labels).   New RetImplLab.                    */
/* This procedure is applied repeatedly to all blocks in the flowgraph   */
/* to find definitive information about what registers are used by which */
/* blocks.                                                               */
{   /* local to successor_regs() */
    if (q == RetIntLab)
        return exitregset(V_Presultreg(INTREG), s);
    else if (q == RetDbleLab)
        return exitregset(V_Presultreg(DBLREG), s);
    else if (q == RetFloatLab)
        return exitregset(V_Presultreg(FLTREG), s);
    else if (q == RetVoidLab || q == RetImplLab)
        return exitregset(GAP, s);
    else
        return vregset_union(s, blkuse_(q->block), &listallocrec);
}

static VRegSetP successor_regs(BlockHead *p)
{   /* local to collect_register_clashes and update_block_use_info */
/* Produce a list of the registers required at the end of block p        */
/* This is done by merging the blkuse_() information from all succesor   */
/* blocks.                                                               */
    VRegSetP s1 = NULL;
    if (blkflags_(p) & BLKSWITCH)
    {   LabelNumber **v = blktable_(p);
        int32 i, n = blktabsize_(p);
        for (i=0; i<n; i++) s1 = extra_regs(s1, v[i]);
    }
    else
    {   s1 = extra_regs(s1, blknext_(p));
        if (blkflags_(p) & BLK2EXIT) s1 = extra_regs(s1, blknext1_(p));
    }
    return s1;
}

#ifdef TARGET_HAS_DIVREM_FUNCTION
static VRegnum othercall2reg(VRegnum r)
{
    if (r == R_P1) return R_P1+1;
    else if (r == R_P1+1) return R_P1;
    else { syserr(syserr_call2, (long)r); return 0; }
}
#endif

static VRegSetP add_instruction_info(
        VRegSetP s1, int32 op, VRegInt r1, VRegInt r2, VRegInt m, bool removed)
/* Note that add_instruction_info() gets called during live variable    */
/* iteration, and ALSO AFTERWARDS, by update_deadflags().  In this case */
/* the caller has masked off the J_DEADBITS from 'op', so that naive    */
/* tests like 'op == J_ADCON' below work fine.                          */
{
        if (loads_r1(op))
        {   bool old;
#ifdef TARGET_HAS_DIVREM_FUNCTION
            bool old2;
#endif
            if (removed);/*nothing*/
#ifdef TARGET_HAS_DIVREM_FUNCTION
            else if ( op == J_CALL2KP &&
                      (s1 = vregset_delete(r1.r, s1, &old),
                       s1 = vregset_delete(othercall2reg(r1.r), s1, &old2),
                       old | old2));
#endif
            else
#ifdef TARGET_SHARES_INTEGER_AND_FP_REGISTERS
                 if ((J_fltisdouble(op) && isany_realreg_(regname_(r1.r)) ?
                          s1 = vregset_delete(virtreg(r1.r+1, INTREG), s1, NULL) : NULL),
                     s1 = vregset_delete(r1.r, s1, &old),
                     old);
#else
              if (s1 = vregset_delete(r1.r, s1, &old), old);
#endif
#ifdef TARGET_HAS_DEBUGGER
            else if (usrdbg(DBG_VAR+DBG_LINE));
                 /* leave unused code alone if debugging */
#endif
            else if (op == J_CALLK
#ifdef TARGET_HAS_DIVREM_FUNCTION
                     || op == J_CALL2KP
#endif
                    )
               /* I suppose this should really be just CALL{2}KP */
            {   if (m.e == arg1_(sim.mulfn))
                    /* ignore mult if result seems (so far) unwanted */
                    op = J_NOOP, r1.r = r2.r = GAP, m.i = 0;
                else if (m.e == arg1_(sim.divfn) ||
                         m.e == arg1_(sim.udivfn) ||
                         m.e == arg1_(sim.remfn) ||
                         m.e == arg1_(sim.uremfn))
/* a division where the result is not needed can be treated as a one-arg */
/* function call.                                                        */
                    r2.i = 1;
            }
            else if (op != J_CALLR)
            {   /* we had better not treat a voided fn as dead code */
                if (debugging(DEBUG_REGS))
                    print_xjopcode(op, r1, r2, m, "-> NOOP loads r1 (a)");
                op = J_NOOP, r1.r = r2.r = GAP, m.i = 0;
            }
        }
        if (uses_stack(op) && op!=J_ADCONV)
        {   VRegnum m1 = bindxx_(m.b);
/* usage information is accumulated for the virtual register that I will */
/* use if I manage to map this stack location onto a register.           */
            if (m1 != GAP)
            {   bool old;
                if (loads_r1(op)) s1 = reference_register(m1, s1);
                /* this else case is really J_STRV/STRDV/STRFV */
                else if (vregset_member(m1, globalregvarset) ||
                         (s1 = vregset_delete(m1, s1, &old), old)) /*NULL*/;
                else if ((bindstg_(m.b) & b_addrof)==0 &&
                         !usrdbg(DBG_VAR+DBG_LINE))
                {
                    if (debugging(DEBUG_REGS))
                        print_xjopcode(op, r1, r2, m,
                            "-> NOOP store %ld (a)", (long)regname_(m1));
                    op = J_NOOP, r1.r = r2.r = GAP, m.i = 0;
                }
            }
        }
        return instruction_ref_info(s1, op, r1, r2, m, 0);
}

static bool update_block_use_info(BlockHead *p)
/* scan a basic block backwards recording information about which        */
/* virtual registers will be needed at the start of the block.           */
{
    VRegSetP s1 = successor_regs(p);
    Icode *q = blkcode_(p);
    int32 w;
/* Now scan this block backwards to see what is needed at its head.      */
    for (w=blklength_(p)-1; w>=0; w--)
    {   int32 op=q[w].op;
        VRegInt r1=q[w].r1, r2=q[w].r2, m=q[w].m;
        s1 = add_instruction_info(s1, op, r1, r2, m, 0);
    }
    {   VRegSetP s2 = blkuse_(p);
        bool same = vregset_equal(s1, s2);
        vregset_discard(s2);
        blkuse_(p) = s1;
#ifndef TARGET_IS_NULL
        if (debugging(DEBUG_REGS))
        {   cc_msg("Block %ld uses: ", (long)lab_name_(blklab_(p)));
            vregset_print(s1);
            cc_msg("\n");
        }
#endif
        return !same;
    }
}

static void increment_refcount(VRegnum n, BlockHead *p)
{
    if (n != GAP) vreg_(n)->refcount += (8L << blknest_(p));
}

#if defined(TARGET_IS_ARM) && defined(TARGET_HAS_BLOCKMOVE)

/* work registers used in expanding J_MOVC (short & long cases) */
#define R_ENDFLAG (NMAGICREGS+1)        /* just a terminator    */
static int32 movc_regs1[] = { R_IP, R_A1+3, R_ENDFLAG };
static int32 movc_regs2[] = { R_IP, R_A1+3, R_A1+2, R_A1+1, R_ENDFLAG };

#endif

static void collect_register_clashes(BlockHead *p)
/* Called after block register use info has converged. (!pass1)          */
/* scan a basic block backwards recording information about which        */
/* virtual registers clash with each other.                              */
/* Essentially a souped-up version of update_block_use_info  (merge?)    */
{
    VRegSetP s1 = successor_regs(p);
    Icode *q = blkcode_(p);
    int32 w;
    thisBlocksBindList = blkstack_(p);

#ifdef TARGET_HAS_DEBUGGER
    if (usrdbg(DBG_VAR)) {
/* if we are generating debug data, cause all binders with extent that   */
/* includes this block to clash with all others.                         */
        BindList *bl = blkstack_(p);
        for ( ; bl!=NULL ; bl = bl->bindlistcdr ) {
            Binder *b = bl->bindlistcar;
            VRegnum r = bindxx_(b);
            if (r!=GAP) makebindersclash(r, bl->bindlistcdr, GAP);
        }
    }
#endif /* TARGET_HAS_DEBUGGER */
    
    for (w=blklength_(p)-1; w>=0; w--)
    {   int32 op=q[w].op;
        VRegnum r1=q[w].r1.r, r2=q[w].r2.r;
        int32 m=q[w].m.i;
        if (loads_r1(op))
/* Unreconstructed WRT sharing TARGET_SHARES_INTEGER_AND_FP_REGISTERS */
        {   if (vregset_member(r1, s1))
/* amazing things happen here because MOV r1, r2 does not want to cause  */
/* r1 and r2 to clash - indeed the very opposite is true.                */
            {   if (op==J_MOVR || op==J_MOVFR || op==J_MOVDR
#ifdef TARGET_IS_ARM
                   || op==J_MOVFDR
#endif
                   )
                    s1 = set_register_copy(r1, s1, (VRegnum)m);
                else if ((op==J_LDRV || op==J_LDRFV || op==J_LDRDV) &&
                         bindxx_((Binder *)m) != GAP)
                    s1 = set_register_copy(r1, s1, bindxx_((Binder *)m));
                else if ((op==J_ADCON || op==J_ADCONV || op == J_MOVK)
                         && r2!=GAP)
                    s1 = set_register_slave(r1, s1, r2);
                else
                {   /* it is not clear that J_INIT should set_register(r1) */
                    if (op == J_INIT || op == J_INITF || op == J_INITD)
                      if (feature & FEATURE_ANOMALY)
                        cc_warn(regalloc_warn_use_before_set, (Binder *)m);
                    s1 = set_register(r1, s1);
#ifdef TARGET_HAS_DIVREM_FUNCTION
                    if (op == J_CALL2KP)
                        s1 = set_register(othercall2reg(r1), s1);
#endif
                }
	      }
#ifdef TARGET_HAS_DEBUGGER
            else if (usrdbg(DBG_VAR))
                s1 = set_register(r1, s1);
#endif
#ifdef TARGET_HAS_DIVREM_FUNCTION
            else if (op == J_CALL2KP && vregset_member(othercall2reg(r1), s1))
            {   s1 = set_register(r1, s1);
                s1 = set_register(othercall2reg(r1), s1);

            }
#endif
            else {   /* This load seems to be unnecessary                */
/* NB loading of volatile values will be protected by the jopcodes J_USE */
/* and friends which make it seem that the value loaded is used even     */
/* if it isn't.                                                          */
                if (op == J_CALLK
#ifdef TARGET_HAS_DIVREM_FUNCTION
                    || op == J_CALL2KP
#endif
                   )
               /* I suppose this should really be just CALL{2}KP */
/* Here I discard a multiplication if its value is not used              */
                {   if ((Expr *)m == arg1_(sim.mulfn))
                    {   if (debugging(DEBUG_REGS)) cc_msg("void a multiply\n");
                        op = q[w].op = J_NOOP;
                        r1 = r2 = q[w].r1.r = q[w].r2.r = GAP;
                        m = q[w].m.i = 0;
                    }
                    else if ((Expr *)m == arg1_(sim.divfn) ||
                             (Expr *)m == arg1_(sim.udivfn) ||
                             (Expr *)m == arg1_(sim.remfn) ||
                             (Expr *)m == arg1_(sim.uremfn))
                    {   if (debugging(DEBUG_REGS)) cc_msg("void a divide\n");
                        r2 = q[w].r2.i = 1; /* Reduce to one arg  */
                        m = q[w].m.i = (int32)arg1_(sim.divtestfn);
                    }
                    else s1 = set_register(r1, s1); /* cannot remove a call */
                }
                else if (op == J_CALLR)
                    s1 = set_register(r1, s1);   /* cannot remove this call */
                else
                {   if (debugging(DEBUG_REGS))
                    {   VRegInt vr1, vr2, vm;
                        vr1.r = r1; vr2.r = r2; vm.i = m;
                        print_xjopcode(op, vr1, vr2, vm, "-> NOOP loads r1");
                    }
                    op = q[w].op = J_NOOP;
                    r1 = r2 = q[w].r1.r = q[w].r2.r = GAP;
                    m = q[w].m.i = 0;
                }
            }
        }
        if (uses_stack(op) && op!=J_ADCONV)
        {   VRegnum m1 = bindxx_((Binder *)m);
/* usage information is accumulated for the virtual register that I will */
/* use if I manage to map this stack location onto a register.           */
            if (m1 != GAP)
            {   if (loads_r1(op)) s1 = reference_register(m1, s1);
                else
                {   /* this else case is really J_STRV/STRDV/STRFV */
                    if (vregset_member(m1, s1) || usrdbg(DBG_VAR+DBG_LINE))
                        s1 = set_register_copy(m1, s1, r1);
                    else if (bindstg_((Binder *)m) & b_addrof)
                        syserr(syserr_dataflow);
                    else
                    {   /* spurious store -- var not live.  Kill to NOOP. */
#ifdef EXPERIMENTAL_DATAFLOW
/* The following code is foiled (generates warnings from sensible code)   */
/* by an action of the optimiser earlier.  (See 'justregister'). Consider */
/* f() { int x = 1; return x;}.  This generates JOPCODE like              */
/* MOVK r,1; STRV r,x; MOVR a1,r which reuses r instead of re-using x     */
/* Possible solution, use the LDRV r2 field to indicate plausible reg?    */
/* Note that I consider the warning required in f() { int x; return x=1;} */
                        if (feature & FEATURE_ANOMALY)
                            cc_warn(regalloc_warn_never_used, (Binder *)m);
#endif
                        if (debugging(DEBUG_REGS))
                        {   VRegInt vr1, vr2, vm;
                            vr1.r = r1; vr2.r = r2; vm.i = m;
                            print_xjopcode(op, vr1, vr2, vm,
                                "-> NOOP stored %ld", (long)regname_(m1));
                        }
                        op = q[w].op = J_NOOP;
                        r1 = r2 = q[w].r1.r = q[w].r2.r = GAP;
                        m = q[w].m.i = 0;
                    }
                }
            }
        }

        if (uses_r1(op)) increment_refcount(r1, p);
        if (uses_r2(op)) increment_refcount(r2, p);
        if (uses_stack(op))
        {   VRegnum n = bindxx_((Binder *)m);
            increment_refcount(n, p);
            if (n != GAP)
                vreg_(n)->refcount |= 1L; /* mark as 'real', rather than  */
        }                                 /* as a re-evaluable CSE binder.*/
        else if (uses_r3(op))
	  increment_refcount(m, p);

	/*
	 * XXX - #if added by NC to avoid compiler complaint
	 * must remove if jopcode.h is changed to redefine
	 * corrupts_r1()
	 */
	
#ifdef TARGET_CORRUPTS_SWITCH_REGISTER
	
/* AM: This is a start at rationalising the following code, based on    */
/* a wish to reduce parameterisation into explicit tests.               */
        if (corrupts_r1(op))            /* really if reads_r1() too.    */
        {   /* Archetype for this case is J_CASEBRANCH on some targets. */
            /* AM supposes that the J_MOVC case may later be tidied     */
            /* by arranging that corrupts_r1(op) is not a global        */
            /* property of op, but a per-instruction property.          */
            corrupt_register(r1, s1);
        }
#endif
	
/* let's rationalise the below someday.                                  */
#ifdef TARGET_IS_ARM
#  ifdef TARGET_IS_RISC_OS
        if ((op == J_ADCON) && arthur_module)
            add_clash(r1, virtreg(R_IP, INTREG));
#  endif
        if ((op == J_MULR)  && (config & CONFIG_HAS_MULTIPLY))
            add_clash(r1, (VRegnum)m);
#  ifdef TARGET_HAS_BLOCKMOVE
        if (op == J_MOVC || op == J_CLRC) {
            if (m != 8) {
                corrupt_register(r1, s1);
                if (op == J_MOVC) corrupt_register(r2, s1);
            }
        }
#  endif
#endif

#ifdef TARGET_IS_370
        if (op == J_MOVFDR) add_clash(r1, (VRegnum)m);
        if (op == J_FIXDR+J_UNSIGNED || op == J_FIXDR+J_SIGNED)
            corrupt_register((VRegnum)m, s1);
#endif

#ifdef TARGET_IS_ACW
        if (uses_r1(op) && !(op==J_ADCON && r2==virtreg(R_SB, INTREG)))
            add_clash(r1, virtreg(R_SB, INTREG));
        if (uses_r2(op) && !uses_mem(op)) add_clash(r2, virtreg(R_SB, INTREG));
        if (uses_r3(op) && op!=J_MOVR) add_clash(m, virtreg(R_SB, INTREG));
        if (op==J_STRV)
        {   VRegnum r3 = bindxx_((Binder *)m);
            if (r3 != GAP && r3 != virtreg(R_SB, INTREG))
                add_clash(r3, virtreg(R_SB, INTREG));
        }
        if (op==J_MOVC && m>16)
        {   int32 i;
            for (i=0; i<8; i++)
            {   if (i!=R_A1+1) add_clash(r1, virtreg(i, INTREG));
                if (i!=R_A1+2) add_clash(r2, virtreg(i, INTREG));
            }
            corrupt_physical_register(virtreg(R_A1+3,INTREG), s1);
        }
#endif

#ifdef TARGET_IS_CLIPPER
# ifdef TARGET_HAS_BLOCKMOVE
        if (op == J_MOVC || op == J_CLRC) {
	  corrupt_physical_register(virtreg(R_IP, INTREG), s1);
	  add_clash(r1, virtreg(R_IP, INTREG));
	  if (op == J_MOVC) {
	    add_clash(r2, virtreg(R_IP, INTREG));
	  }
        }
# endif
#endif

#ifdef TARGET_HAS_2ADDRESS_CODE
#  ifdef AVOID_THE_ACN_ADJUSTMENT_MADE_HERE
        if (jop_asymdiadr_(op) && r2 != (VRegnum)m) add_clash(r1, (VRegnum)m);
#  else
/* See the comment in flowgraf.c (line 99) for why I think this test
   wants to be like this and why the previous version was wrong.  Note also
   that any test on register equality here is pretty suspect since it tests
   virtual registers not real ones - thus (and especially given that SUBR
   seems to be used but rarely) I might prefer a test just on
   jop_asymdiadr(op). */
        if (jop_asymdiadr_(op) && r1 != r2) add_clash(r1, (VRegnum)m);
#  endif
#endif

        {   VRegInt vr1, vr2, vm;
            vr1.r = r1; vr2.r = r2; vm.i = m;
            s1 = instruction_ref_info(s1, op, vr1, vr2, vm, &q[w].op);
        }

/* The following things that allow for workspace registers MUST be done  */
/* here after other register-use info for the instruction has been dealt */
/* with.   Again, this needs parameterising instead of #ifdef.           */
#ifdef TARGET_IS_370
/* @@@ For the 370 this could be done further above, which would help   */
/* reduce the number of RR copies.  E.g. the corrupt_register's below   */
/* force clashes with args, which is not what we really want.           */
/* BEWARE:  use of magic register numbers below (assume R_T1==0?).      */
        switch (op & ~(J_SIGNED|J_UNSIGNED))
        {
case J_MULK:  if (-0x8000 <= m && m <= 0x7fff) break;       /* use MH   */
case J_MULR:
case J_DIVR: case J_DIVK: case J_REMR: case J_REMK:
              corrupt_physical_register(virtreg(0,INTREG), s1);
              corrupt_physical_register(virtreg(1,INTREG), s1);
              break;
case J_MOVC: case J_CLRC:
              if (m > 4096)             /* nasty magic number           */
              {   corrupt_physical_register(virtreg(0,INTREG), s1);
                  corrupt_physical_register(virtreg(1,INTREG), s1);
                  corrupt_physical_register(virtreg(2,INTREG), s1);
                  corrupt_physical_register(virtreg(3,INTREG), s1);
              }
              break;
        }
#endif

#ifdef TARGET_IS_ARM
        if ((pseudo_reads_r1(op) && !immed_cmp(m))    /* J_CMPK+friends */
            || op == J_MULK
            || (op == J_CASEBRANCH && !immed_cmp(m-2))
#  ifdef TARGET_IS_RISC_OS
            || (op == J_ADCON && arthur_module)
#  endif
           )
            corrupt_physical_register(virtreg(R_IP, INTREG), s1);
#  ifdef TARGET_HAS_BLOCKMOVE
        if (op == J_MOVC || op == J_CLRC) {
        /* The disgusting magic number here had better agree with arm/gen.c */
            int32 *regs = m <= 24 ? &movc_regs1[0] : &movc_regs2[0];
            int32 r;
            while ((r = *regs++) != R_ENDFLAG) {
                corrupt_physical_register(virtreg(r, INTREG), s1);
                add_clash(r1, virtreg(r, INTREG));
                add_clash(r2, virtreg(r, INTREG));
            }
        }
#  endif
#endif
#ifdef TARGET_LDRK_MAX
/* If I have a large stack frame I reserve register ip for helping       */
/* gain addressability to frame locations.                               */
/* The 256 slop is to allow for routine linkage/savearea space.          */
        if (uses_stack(op) && op!=J_ADCONV)
        {   int32 n = vkformat(op) ? (int32)r2 : 0;
            if (n < TARGET_LDRK_MIN
                || greatest_stackdepth+max_argsize+n > TARGET_LDRK_MAX-256
#  ifdef TARGET_LDRFK_MAX
                || n < TARGET_LDRFK_MIN
                || greatest_stackdepth+max_argsize+n > TARGET_LDRFK_MAX-256
#  endif
               )
                    corrupt_physical_register(virtreg(R_IP, INTREG), s1);
        }
#endif
    }
    vregset_discard(s1);
    if (debugging(DEBUG_REGS))
        cc_msg("Scanned block %ld\n", (long)lab_name_(blklab_(p)));
}

#ifndef TARGET_IS_NULL
#ifdef TARGET_NEEDS_DEADBITS

typedef struct {
    VRegnum vreg;
    RealRegister rreg;
    Icode *icode;
    int32 mask;
} UDFRec;

static void udf_cb(VRegnum r, VoidStar arg)
{
    UDFRec *udf = (UDFRec *)(int) arg;
    if (r != udf->vreg && vreg_(r)->realreg == udf->rreg)
        udf->icode->op &= udf->mask;
}

/* The reason for update_deadflags() is that the DEADBITS information    */
/* for virtual registers is a superset of that for physical registers -- */
/* consider examples like                                                */
/*   extern int z; void f(x) { while (x) z=1; while (x) z=1; }           */
/* Since the mapping from virtual register (with clashes) to physical    */
/* register graphs (with inequality) is a homomorphism one would expect  */
/* some nice theory to cover it.  Note that the property of deadness is  */
/* not preserved by the homomorphism, but it IS semi-preserved.          */
/* So the code removes the DEAD infomation from any register which       */
/* has a another live register mapped to the same physical register.     */
/* AM is not sure that the tests below of ->realreg!=0 are really        */
/* required, but they look harmless.                                     */
static void update_deadflags(BlockHead *p)
{   VRegSetP s = successor_regs(p);
    Icode *q = blkcode_(p);
    int32 w;
    UDFRec udf;
    for (w=blklength_(p)-1; w>=0; w--)
    {   int32 op=q[w].op;
        VRegnum r1=q[w].r1.r, r2=q[w].r2.r;
        int32 m=q[w].m.i;
/* The following three lines take care of the fact that we really want   */
/* to execute this code half-way through add_instruction_info().         */
/* This code ensures that we do not remove deadflags from r2 for (say)   */
/*   LDRK r1,r2,0  if r1 and r2 map to the same physical register (and   */
/* of course r2 is dead at the virtual level).                           */
/* WGD's 32000 code depends on this as he was entitled to from the       */
/* Nov87 comment above.  Whether we ought to reconsider this is another  */
/* matter.  Discuss with WGD one day.                                    */
        bool removed = 0;
        if (debugging(DEBUG_LOOP))
        {   VRegInt vr1, vr2, vm;
            vr1.r = r1; vr2.r = r2; vm.i = m;
            /* So we can look at J_DEADBITS...                              */
            vregset_print(s),
            cc_msg("(%ld)", (long)((op & J_DEADBITS) >> 12)),
            print_jopcode(op & ~J_DEADBITS, vr1, vr2, vm);
        }
        udf.icode = &q[w];
        if (loads_r1(op))
            s = vregset_delete(r1, s, &removed);
        if ((op & J_DEAD_R2) && (reads_r2(op) ||
            (pseudo_reads_r2(op & ~J_DEADBITS) && r2!=GAP)))    /* MOVK/ADCON loopopt */
        {   RealRegister rr = vreg_(r2)->realreg;
            if (debugging(DEBUG_LOOP))
                cc_msg("try fix r2 %ld %ld ", (long)r2, (long)rr);
            if (rr >= 0)
            {   udf.vreg = r2;
                udf.rreg = rr;
                udf.mask = ~J_DEAD_R2;
                vregset_map(s, udf_cb, (VoidStar)&udf);
            }
            if (debugging(DEBUG_LOOP))
                cc_msg("%s", udf.icode->op & J_DEAD_R2 ? "undead\n": "\n");
        }
        if ((op & J_DEAD_R3) && reads_r3(op) &&
            (op & ~J_DEADBITS) != J_CALLR)     /* see above */
        {   RealRegister rr = vreg_((VRegnum)m)->realreg;
            if (debugging(DEBUG_LOOP))
                cc_msg("try fix r3 %ld %ld ", (long)(VRegnum)m, (long)rr);
            if (rr >= 0)
            {   udf.vreg = (VRegnum)m;
                udf.rreg = rr;
                udf.mask = ~J_DEAD_R3;
                vregset_map(s, udf_cb, (VoidStar)&udf);
            }
            if (debugging(DEBUG_LOOP))
                cc_msg("%s", udf.icode->op & J_DEAD_R3 ? "undead\n": "\n");
        }
        if ((op & J_DEAD_R1) && (reads_r1(op) ||
            (pseudo_reads_r1(op & ~J_DEADBITS) && r1!=GAP)))    /* CMPK loopopt */
        {   RealRegister rr = vreg_(r1)->realreg;
            if (debugging(DEBUG_LOOP))
                cc_msg("try fix r1 %ld %ld ", (long)r1, (long)rr);
            if (rr >= 0)
            {   udf.vreg = r1;
                udf.rreg = rr;
                udf.mask = ~J_DEAD_R1;
                vregset_map(s, udf_cb, (VoidStar)&udf);
            }
            if (debugging(DEBUG_LOOP))
                cc_msg("%s", udf.icode->op & J_DEAD_R3 ? "undead\n": "\n");
        }
        {   VRegInt vr1, vr2, vm;
            vr1.r = r1; vr2.r = r2; vm.i = m;
            s = add_instruction_info(s, op & ~J_DEADBITS, vr1, vr2, vm, removed);
        }
    }
    vregset_discard(s);
}
#endif /* TARGET_NEEDS_DEADBITS */


/*************************************************************************/
/*       Here comes the code that allocates and assigns registers        */
/*************************************************************************/

static void MaybePrefer(VRegnum vr, RealRegSet *prefer)
{
    RealRegister r1 = register_number(vregname_(vregtable_(vr)));
    
    if (isany_realreg_(r1)) augment_RealRegSet(prefer,r1);
}

static bool choose_real_register(VRegister *r)
/* select a real register to put r into: return 1 on success, 0 on failure */
{
    RealRegSet m1, m2, prefer;
    RealRegister r1;
#ifdef TARGET_SHARES_INTEGER_AND_FP_REGISTERS
    int32 type = vregtype_(r);
#endif

#ifdef ENABLE_SPILL
    ++choose_count;
#endif

    memclr((VoidStar)&prefer, sizeof(RealRegSet));
    map_copies(r, MaybePrefer, (VoidStar)&prefer);

    m1    =         *(vregtype_(r) == INTREG ? &m_intregs :
#ifdef ADDRESS_REG_STUFF
                      vregtype_(r) == ADDRREG ?
                          (!spillpanic ? &m_addrregs : &m_intregs) :
#endif
      /*
       * XXX - NC - 2/8/91
       * The following ifdef has been added because the variable
       * 'm_fltregs'  only exists if TARGET_SHARES_INTEGER_AND_FP_REGISTERS
       * is not defined (unless we are a C40 which is doing some strange things :-)
       */
#if defined TARGET_SHARES_INTEGER_AND_FP_REGISTERS && !defined TARGET_IS_C40
                      &m_intregs);
#else
                      &m_fltregs);
#endif

    /* fprintf( stderr, "initial map = %x\n", m1.map[ 0 ] ); */

    clashkillbits( &m1, r );

#ifdef TARGET_SHARES_INTEGER_AND_FP_REGISTERS	/* XXX - added by NC */
    if (type = FLTREG || type == DBLREG)
      {
	int32		i;


	/* eliminate non-paired registers from register list */
	 
	for (i = 0; i < (NMAGICREGS + 31)/ 32; i++)
	  {
	    unsigned32	mask;
	    int32	map = m1.map[ i ];
		 
		 
	    for (mask = 1U; mask; mask <<= 1)
	      {
		if (map & mask)
		  {
		    unsigned32	upper = mask << 1;

		    
		    if ((map & upper) == 0)
		      {
			map ^= mask;
		      }

		    mask = upper;
		  }
	      }
	    
	    m1.map[ i ] = map;
	  }
      }
#endif /*TARGET_SHARES_INTEGER_AND_FP_REGISTERS */

   /* fprintf( stderr, "after eliminating clashes, map = %x\n", m1.map[ 0 ] ); */

/* Test if this register can be allocated at all.                        */
#ifdef TARGET_SHARES_INTEGER_AND_FP_REGISTERS
    if (!nonempty_RealRegSet(&m1, type))
#else
    if (!nonempty_RealRegSet(&m1))
#endif
    {   if (debugging(DEBUG_REGS) || debugging(DEBUG_SPILL))
        {   cc_msg("Unable to allocate v%ld(%ld):",
                   (long)vregname_(r), (long)r->heapaddr);
            if (debugging(DEBUG_REGS))
            {   cc_msg(" to miss ");
                printvregclash2(r);
                cc_msg("\n");
            }
        }
        r->realreg = R_BOGUS;
/* If I fail to allocate a real register to this binder I drop into the  */
/* code that spills things. This will choose a register to spill based   */
/* on a number of criteria. If possible it will spill either the binder  */
/* that failed to get allocated or one of the binders that clashes with  */
/* it directly (thus having the best chance of unblocking this particular*/
/* clash). Failing that it will spill some binder that has already been  */
/* allocated a real register (ones that have not yet been considered for */
/* allocation would not be sensible to spill, would they?). In this      */
/* context it is desirable that the binder that causes the clash should  */
/* seem to have a real register allocated to it so that it can be        */
/* selected for spilling under this heading. Priority within the above   */
/* categories is based on usage information (->refcount) and register    */
/* provided by the user. After spilling a binder all binders that have   */
/* been given registers so far get reset to a null state (thus undoing   */
/* the spurious allocation of R_BOGUS above) so that the entire process  */
/* of allocation can be restarted: with some binder marked as spilt it   */
/* will generally be possible to proceed further, until at last it       */
/* becomes possible to allocate all registers.                           */
        return NO;
    }
#ifdef ADDRESS_REG_STUFF
   spillpanic = 0;
#endif

/* If possible allocate r1 so as to remove a copy operation somewhere    */
     if (intersect_RealRegSet(&m2, &m1, &prefer))
       {
#if defined TARGET_IS_C40 && defined TARGET_SHARES_INTEGER_AND_FP_REGISTERS
	 unsigned32	set = 1;

	 
	 if (type = FLTREG || type == DBLREG)
	   {
	     int32	i;

	     
	     /* eliminate non-paired registers from prefered register list */
	     
	     set = 0;

	     for (i = 0; i < (NMAGICREGS + 31)/ 32; i++)
	       {
		 unsigned32	mask;
		 int32		map = prefer.map[ i ];
		 
		 
		 for (mask = 1U; mask; mask <<= 1)
		   {
		     if (map & mask)
		       {
			 unsigned32	upper = mask << 1;


			 if ((map & upper) == 0)
			   {
			     map ^= mask;
			   }

			 mask = upper;
		       }
		   }

		 prefer.map[ i ] = map;

		 set |= map;
	       }
	   }

	 if (set)
#endif
	   m1 = m2;
       }

/* Try to allocate avoiding V1 to V<n> (and any floating var regs).      */
/* AM (Dec 87) wonders how necessary this is in that ALLOCATION_ORDER    */
/* can do this equally as well!                                          */

    /* fprintf( stderr, "after checking preferences, map = %x\n", m1.map[ 0 ] ); */

/* The next line needs to ensure that the overlap is an even/odd      */
/* pair if TARGET_SHARES_INTEGER_AND_FP_REGISTERS.                       */
    if (intersect_RealRegSet(&m2, &m1, &m_notpreserved))
        m1 = m2;

   /* fprintf( stderr, "finally map = %x\n", m1.map[ 0 ] ); */


/* Convert representation from bit-position to register number.          */
#ifdef ALLOCATION_ORDER
/* a prespecified allocation ordering:                                   */
    {
      static unsigned char o[] = ALLOCATION_ORDER;
      
        int32 i = 0;
        for (;;)
        {   r1 = o[i++];
	    
            if (r1 >= (unsigned32)NMAGICREGS)
	      {
		syserr(syserr_choose_real_reg, (long)((m1.map)[0]));
                break;
            }
#else
/* else choose in a not particularly inspired order.                     */
    for (r1=0; ; r1++)
    {
        {   /* nested blocks to match ALLOCATION_ORDER case... */
#endif
#ifdef TARGET_SHARES_INTEGER_AND_FP_REGISTERS
            if (type == FLTREG || type == DBLREG)
            {
                if (member_RealRegSet(&m1,r1) &&
                    member_RealRegSet(&m1,r1+1)) break;
            }
            else if (member_RealRegSet(&m1,r1)) break;
#else
            if (member_RealRegSet(&m1,r1)) break;
#endif
        }
    }

    if (debugging(DEBUG_REGS))
    {   cc_msg("Now allocate %ld to %ld: miss ",
               (long)vregname_(r), (long)r1);
        printvregclash2(r);
        cc_msg("\n");
    }

/* Record the allocation.                                                */
    r->realreg = r1;
/* regmaskvec gets bits set to show what registers this procedure uses.  */
    if (!member_RealRegSet(&globalregvarvec,r1))
    {
#ifdef TARGET_IS_C40
        augment_RealRegSet( &usedmaskvec, r1 ); /* register used */
#endif
        augment_RealRegSet(&regmaskvec,r1);     /* register used */
#ifdef TARGET_SHARES_INTEGER_AND_FP_REGISTERS
        if (type == FLTREG || type == DBLREG)
	  {
            augment_RealRegSet(&regmaskvec,r1+1);
#ifdef TARGET_IS_C40
            augment_RealRegSet( &usedmaskvec, r1 + 1 );
#endif
	  }
#endif
    }
    return YES;                                 /* success!      */
}

static int32 spill_binder(VRegister *rr, BindList *spill_order, VRegSetP *vr)
/* Here virtual register rr could not be allocated.  Try to pick the     */
/* best Binder in spill_order to either spill rr to the stack or to      */
/* help the process of allocating rr to a real register.                 */
{   BindList *candidates = NULL;
    {   BindList *lb;
        for (lb = spill_order; lb != NULL ; lb = lb->bindlistcdr)
        {   Binder *bb = lb->bindlistcar;
            VRegnum r1n = bindxx_(bb);
/* there is no joy in trying to spill a binder if either (a) it was not  */
/* a candidate for slaving in a register anyway or (b) the corresponding */
/* register has not yet been scheduled for allocation.                   */
            if (r1n != GAP)
            {   VRegister *r1v = vreg_(r1n);
                if (r1v->realreg != R_SCHEDULED)
                {
#ifdef ADDRESS_REG_STUFF
/* If we require an address reg and this one is not and we are not */
/* panicking yet -> don't consider this one because it wouldn't do */
/* any good any way.                                               */
/* Maybe the next line should use member(m_addrregs), but AM is    */
/* is not sure enough that r1v->realreg is in range.               */
                    if (vregtype_(rr) == ADDRREG &&
                        !target_isaddrreg_(r1v->realreg) &&
                        !spillpanic) continue;
#endif
                    candidates = (BindList *) binder_cons2(candidates, bb);
                }
            }
        }
    }
    if (candidates==NULL)
#ifdef ADDRESS_REG_STUFF
    {   if( !spillpanic )
        {   spillpanic = 1;
            return 0;
        }
        else syserr(syserr_fail_to_spill, (long)vregname_(rr));
    }
#else
    syserr(syserr_fail_to_spill, (long)vregname_(rr));
#endif
/* Now find the best binder, with ones that clash directly with the      */
/* thing that could not be allocated taking precedence                   */
    {   Binder *bb = (Binder *)(int) DUFF_ADDR;
        VRegister *r1 = (VRegister *)(int) DUFF_ADDR;
        int32 leastspillcost = INT_MAX;
        bool foundgood = NO;
        for (; candidates != NULL;
               candidates = (BindList *)discard2((List *)candidates))
        {   Binder *bb2 = candidates->bindlistcar;
            VRegnum r2n = bindxx_(bb2);
            VRegister *r2 = vreg_(r2n);
            int32 spillcost = r2->refcount;
/* This is a good candidate if it was involved in the clash that caused  */
/* me to decide that I needed to spill something, or if no such register */
/* was on the list of candidates and this is the one with fewest uses    */
            if (rr==r2 || vregset_member(regname_(r2n), rr->clash2))
            {   if (!foundgood || spillcost <= leastspillcost)
                {   bb = bb2, r1 = r2, leastspillcost = spillcost;
                    foundgood = YES;
                }
            }
            if (!foundgood && spillcost <= leastspillcost)
            {   bb = bb2, r1 = r2, leastspillcost = spillcost;
            }
        }
        if (debugging(DEBUG_REGS|DEBUG_SPILL))
            cc_msg("    spill: $b, v%lu(r%ld:%ld), cost = %lu\n",
                bb, (long)vregname_(r1), (long)r1->realreg,
                (long)r1->heapaddr, (long)leastspillcost);
        r1->realreg = R_SPILT;   /* marker for spilled register   */
        r1->u.spillbinder = bb;
        *vr = vregset_insert(vregname_(r1), *vr, NULL, &listallocrec);
        bindxx_(bb) = GAP;   /* this one has to stay on the stack */
#ifdef ENABLE_SPILL
        if (r1->refcount & 1L)
            ++n_real_spills;
        else
            ++n_cse_spills;
        spill_cost += leastspillcost;
#endif
        return r1->heapaddr;
    }
}

#endif /* TARGET_IS_NULL */

/* exported... */

static void addclashes_rcb(VRegnum n, void * arg)
{   VRegister *v = vreg_(n);
    n = (VRegnum) arg;
    v->clash2 = vregset_insert(n, v->clash2, NULL, &listallocrec);
}

void allocate_registers(BindList *spill_order)
/* spill_order is a list of all binders active in this function,         */
/* ordered with the first-mentioned register variables LAST so that they */
/* are the things least liable to be spilled out to the stack.           */
{
    clock_t t0 = clock();
    int32 i, nn;
    VRegSetP spillset = NULL;

#ifndef TARGET_IS_NULL
    ReadonlyCopy *p;
    regalloc_changephase();
    clash_reinit(vregistername);

    /* weight the reference counts of register variables to avoid spills */
    {   BindList *l = spill_order;
        Binder *b;
        while (l != NULL)
        {   b = (Binder *)(l->bindlistcar);
            if (bindstg_(b) & bitofstg_(s_register))
            {   VRegnum r = bindxx_(b);
    /* register vars may be forced to memory by setjmp, leaving r == GAP */
                if (r != GAP) vreg_(r)->refcount += 1000000;
            }
            l = l->bindlistcdr;
        }
    }

    if (debugging(DEBUG_REGS)) cc_msg("Slave list:\n");
    /* transform slave_list into a more convenient form for our use */
    for (p = slave_list; p!=NULL; p = p->next) {
        vregtable_(regname_(p->r1))->slave = p->r2;
        if (debugging(DEBUG_REGS)) {
            cc_msg("%ld : ", (long)regname_(p->r1));
            if (p->r2 == GAP)
                cc_msg("GAP\n");
            else
                cc_msg("%ld\n", (long)regname_(p->r2));
        }
    }
#endif  /* TARGET_IS_NULL */
/* First I iterate over the basic blocks to collect information about    */
/* which registers are needed at the head of each block. With structured */
/* control-flow this should cost at worst one scan of the flowgraph      */
/* (plus another to verify that there are no changes left over). With    */
/* very contorted flow of control (e.g. via goto or switch with case     */
/* labels inside embedded loops) it can take MANY iterations.            */
    phasename = "dataflow";
    {   bool changed;
        do
        {   BlockHead *p;
            changed = NO;
            curstats.dataflow_iterations++;
            if (debugging(DEBUG_REGS))
                cc_msg("Start a scan of register flow iteration\n");
            for (p=bottom_block; p!=NULL; p = blkup_(p))
                changed |= update_block_use_info(p);
        } while (changed);
    }
    if (debugging(DEBUG_REGS))
        cc_msg("Block by block register use analysis complete\n");

    dataflow_clock += clock() - t0; t0 = clock();

    phasename = "clashmap";
    {   BlockHead *p;
        for (p=top_block; p!=NULL; p=blkdown_(p)) collect_register_clashes(p);
#ifndef TARGET_NEEDS_DEADBITS
        for (p=top_block; p!=NULL; p=blkdown_(p))
            vregset_discard(blkuse_(p));
#endif /* TARGET_NEEDS_DEADBITS */
    }

    regalloc_clock1 += clock() - t0; t0 = clock();

#ifndef TARGET_IS_NULL
    if (debugging(DEBUG_REGS))
    {   int32 i;
        cc_msg("\nGlobal register clash information collected\n");
        for (i=0; i<NMAGICREGS; i++)
        {   VRegister *r = vregtable_(i);
            if (r->u.nclashes != 0)
            {   cc_msg("r%ld clashes with:", (long)vregname_(r));
                printvregclash(r);
                cc_msg("\n");
            }
        }
        for (i=1; i<vregistername-NMAGICREGS; i++)
        {   VRegister *r = permregheap_(i);
            if (r->u.nclashes != 0)
            {   if (r->realreg >= 0) cc_msg("[r%ld]: ", (long)r->realreg);
                cc_msg("v%ld clashes with:", (long)vregname_(r));
                printvregclash(r);
                cc_msg("\n");
            }
        }
    }

/* Form the register vector into a priority queue (heap)                 */
    phasename = "regalloc";
    for (i = (vregistername-NMAGICREGS-1)/2; i>=1; i--)
        downheap(i, vregistername-NMAGICREGS-1);

    for (i = vregistername-NMAGICREGS-1; i>0; i--)
    {   VRegister *s = permregheap_(1), *t = permregheap_(i);
        if (debugging(DEBUG_REGS))
            cc_msg("Register %ld clashes with %ld others\n",
                (long)vregname_(s), (long)s->u.nclashes);
        permregheap_(1) = t;
        t->heapaddr = 1;
        permregheap_(i) = s;
        s->heapaddr = i;
        downheap(1, i-1);
        s->realreg = R_SCHEDULED;
        removeclashes(s);
    }

/* Now I will try the registers in the order just selected.              */
    for (i = 1; i < vregistername-NMAGICREGS; i++)
    {   VRegister *rr = permregheap_(i);
        if (rr->realreg == R_SPILT) continue;    /* spilt register       */
        if (!choose_real_register(rr))
        {   /* Here it is necessary to spill something                   */
            int32 spilt = spill_binder(rr, spill_order, &spillset);
#ifdef ADDRESS_REG_STUFF
            if ( spilt == 0 )
            { --i;         /* spillpanic is now 1 */
              continue;
            }
#endif
/* N.B. this loop modifies the main loop control variable!!!             */
/* N.B. also (LDS) that while this loop appears to introduce quadratic   */
/* complexity, experiment shows that the total number of re-tries is     */
/* LESS than one scan through the list, ALMOST ALWAYS. Does this follow  */
/* from ordering the list by most clashes first?                         */
            while (i >= spilt)
            {   VRegister *rr = permregheap_(i);
                if (rr->realreg != R_SPILT) rr->realreg = R_SCHEDULED;
                i--;
            }
        }
    }

#ifdef ENABLE_SPILL
    if (debugging(DEBUG_SPILL))
        cc_msg("%ld calls to choose_real_register() to allocate %ld vregs\n",
               (long)choose_count, (long)vregistername-NMAGICREGS);
#endif

    nn = 0;
    if ((n_real_spills + n_cse_spills)> 1 &&        /* retrying may help */
        (var_cc_private_flags & 6L) == 0)
             /* neither compatible with original nor cleaning suppressed */
    {
/* Here, we do some cleaning up of the register colouring to trying to   */
/* the things that have already been spilt. Sometimes we'll succeed.     */
/* This heuristic reduces the number of spills in the compiler by 2%,    */
/* with significant savings in larger functions. First, then, we rebuild */
/* the complete clash lists for each spilt register.                     */
        VRegSetP x;
        clock_t us_t = clock();
        for (i = 1; i < vregistername-NMAGICREGS; ++i)
        {   VRegister *rr = permregheap_(i);
            if (rr->realreg != R_SPILT)
            {   /* the following destroys rr->clash2, saving store */
                x = vregset_intersection(rr->clash2, spillset);
                rr->clash2 = DUFF_ADDR;
            }
            else
                x = vregset_intersection(
                        vregset_copy(spillset, &listallocrec), rr->clash2);
            vregset_map(x, addclashes_rcb, (VoidStar)(vregname_(rr)));
        }

/* It isn't clear which way this loop should go - or even whether it's   */
/* best to use notional spill cost order. So far, experiments on the     */
/*compiler itself have been inconclusive.                                */
        for (i = 1;  i < vregistername-NMAGICREGS;  ++i)
        {   VRegister *rr = permregheap_(i);
            if (rr->realreg != R_SPILT) continue;
            if (choose_real_register(rr))
            {   Binder *bb = rr->u.spillbinder;
                bindxx_(bb) = vregname_(rr);
                nn += 1;
                spill_cost -= rr->refcount;
                if (rr->refcount & 1L)
                    --n_real_spills;
                else
                    --n_cse_spills;
                if (debugging(DEBUG_SPILL))
                   cc_msg("    unspill: $b, v%lu = r%lu, saving = %lu\n",
                       bb, vregname_(rr), rr->realreg, rr->refcount);
            }
        }
        if (debugging(DEBUG_SPILL))
            cc_msg("%lu vregs unspilt by cleaning pass in %ucs\n",
                   nn, clock() - us_t);
    }

#ifdef TARGET_NEEDS_DEADBITS
/* WGD Now check for spurious deadbits arising from register copies */

    {   BlockHead *p;
        for (p=top_block; p!=NULL; p=blkdown_(p)) update_deadflags(p);
        for (p=top_block; p!=NULL; p=blkdown_(p))
            vregset_discard(blkuse_(p));
    }
#endif /* TARGET_NEEDS_DEADBITS */
#else
    IGNORE(i); IGNORE(spill_order);
#endif /* TARGET_IS_NULL */
    regalloc_clock2 += clock() - t0;
    if (debugging(DEBUG_STORE | DEBUG_REGS)) stats_endproc();
#ifdef ENABLE_SPILL
    if (debugging(DEBUG_SPILL) && (n_real_spills + n_cse_spills + nn) > 0)
    {
       cc_msg("%-24s %3lu + %lu spills, cost = %6lu\n\n",
              currentfunction, n_real_spills, n_cse_spills, spill_cost);
       tot_real_spills += n_real_spills;
       tot_cse_spills += n_cse_spills;
       tot_cost += spill_cost;
       n_real_spills = n_cse_spills = spill_cost = 0;
    }
#endif
}

#ifndef TARGET_IS_NULL
/* change the union member in vregtable -- expand VRegnum's for allocation */
static void regalloc_changephase(void)
{   RealRegister i;
    for (i = 0; i < vregistername; i++)
    {   VRegnum rname = vregtypetab_(i);
        VRegister *v = (VRegister *) BindAlloc(sizeof(VRegister));
        if (regname_(rname) != i) syserr(syserr_regalloc_reinit2);
        v->heapaddr = i-NMAGICREGS;   /* -ve for real regs, 0 for sentinel */
        v->perm = v;
        v->realreg = R_UNSCHEDULED;   /* Not yet scheduled for allocation  */
        v->clash2 = (VRegSetP )(int) DUFF_ADDR;
        v->u.nclashes = 0;
        v->rname = rname;
        v->ncopies = 0;
        v->refcount = 0;
        v->slave = GAP;
        vregtable_(i) = v;
        if (i < NMAGICREGS) v->realreg = i;     /* real register */
        if (i == NMAGICREGS) v->u.nclashes = -1;  /* heap sentinel */
    }
}
#endif /* TARGET_IS_NULL */

/* exported for cg.c and loopopt.c */
VRegnum vregister(int32 type)
{   VRegnum rname = type | vregistername;
    if ((vregistername&(REGHEAPSEGSIZE-1)) == 0)
    {   int32 p = vregistername >> REGHEAPSEGBITS;
        if (p >= REGHEAPSEGMAX) syserr(syserr_regheap);
        else vregheap[p] =
          (vreg_type (*)[REGHEAPSEGSIZE]) BindAlloc(sizeof(*vregheap[0]));
    }
    vregtypetab_(vregistername) = rname;
    vregistername++;
    return rname;
}

void globalregistervariable(VRegnum r)
{
    unsigned32 dummy;
    VRegSetAllocRec a;
    a.alloctype = AT_Glob;
    a.statsloc = a.statsloc1 = a.statsbytes = &dummy;
    augment_RealRegSet(&globalregvarvec, regname_(r));
    vregset_init();
    globalregvarset = vregset_insert(r, globalregvarset, NULL, &a);
}

void avoidallocating(VRegnum r)
{
    delete_RealRegSet(&m_intregs, regname_(r));
#if !defined TARGET_SHARES_INTEGER_AND_FP_REGISTERS || TARGET_IS_C40
    delete_RealRegSet(&m_fltregs, regname_(r));
#endif
#ifdef ADDRESS_REG_STUFF
    delete_RealRegSet(&m_addrregs, regname_(r));
#endif
}

/* call before first use of vregister() */
void regalloc_reinit(void)
{   RealRegister i;   /* parallel to vregistername here */
    vregistername = 0;
    for (i = 0; i < NMAGICREGS; i++)
        (void)vregister(isint_realreg_(i) ? INTREG : DBLREG);
    (void)vregister(SENTINELREG);
    memclr((VoidStar)&curstats, sizeof(curstats));
    memclr((VoidStar)&regmaskvec, sizeof(RealRegSet));
#ifdef TARGET_IS_C40
    memclr( (VoidStar)&usedmaskvec, sizeof(RealRegSet) );
#endif
    
#ifdef TARGET_IS_MIPS
    augment_RealRegSet(&regmaskvec,R_A1result);  /* temp hack for mips */
    augment_RealRegSet(&regmaskvec,R_FA1result); /* temp hack for mips */
#endif
#ifndef TARGET_IS_NULL
    slave_list = NULL;
#endif
#ifdef ENABLE_SPILL
    n_real_spills = n_cse_spills = spill_cost = 0;
#endif
}

void regalloc_init(void)
{   VRegnum i;
    dataflow_clock = regalloc_clock1 = regalloc_clock2 = 0;
/*
 * Active initialisations so that the compiled image has a chance to
 * be absolutely relocatable (e.g. for a RISC-OS relocatable module)
 */
    clashrallocrec.alloctype = ClashAllocType;
    clashrallocrec.statloc = &curstats.nsquares;
    clashrallocrec.statbytes = &curstats.squarebytes;
    copyallocrec.alloctype = CopyAllocType;
    copyallocrec.statloc = &curstats.copysquares;
    copyallocrec.statbytes = &curstats.copysquarebytes;
    clashvallocrec.alloctype = ClashAllocType;
    clashvallocrec.statsloc = &curstats.nregsets;
    clashvallocrec.statsloc1 = &curstats.newregsets;
    clashvallocrec.statsbytes = &curstats.regsetbytes;
    listallocrec.alloctype = ListAllocType;
    listallocrec.statsloc = &curstats.nlists;
    listallocrec.statsloc1 = &curstats.newlists;
    listallocrec.statsbytes = &curstats.listbytes;
    memclr((VoidStar)(&maxstats), sizeof(maxstats));

    memclr((VoidStar)&m_notpreserved, sizeof(RealRegSet));
    for (i = 0; i<NMAGICREGS; i++)
#ifdef target_preserves                 /* better? */
        if (target_preserves(i))
#else
        if (!(R_V1 <= i && i < R_V1+NVARREGS
#ifndef TARGET_SHARES_INTEGER_AND_FP_REGISTERS
              || R_FV1 <= i && i < R_FV1+NFLTVARREGS
#endif
           ))
#endif
            augment_RealRegSet(&m_notpreserved,i);

    memclr((VoidStar)&m_intregs, sizeof(RealRegSet));
    for (i = 0; i<NMAGICREGS; i++)
        if (R_A1 <= i && i < R_A1+NARGREGS
            || R_P1 <= i && i < R_P1+NARGREGS
            || R_V1 <= i && i < R_V1+NVARREGS
/* on many machines R_IP will be one of the NTEMPREGS, but to allow it  */
/* to be non-contiguous we treat it specially.                          */
            || i == R_IP
            || R_T1 <= i && i < R_T1+NTEMPREGS
#ifndef TARGET_STACKS_LINK
# ifndef TARGET_IS_SPARC
            || i == R_LR
# endif
#endif
           ) augment_RealRegSet(&m_intregs,i);

#if !defined TARGET_SHARES_INTEGER_AND_FP_REGISTERS || TARGET_IS_C40
    memclr((VoidStar)&m_fltregs, sizeof(RealRegSet));
    
    for (i = 0; i<NMAGICREGS; i++)
        if (
#if defined TARGET_SHARES_INTEGER_AND_FP_REGISTERS      
	    R_FA1 <= i && i < R_FA1 + NFLTARGREGS  * 2 ||
            R_FP1 <= i && i < R_FP1 + NFLTARGREGS  * 2 ||
            R_FT1 <= i && i < R_FT1 + NFLTTEMPREGS * 2 ||
            R_FV1 <= i && i < R_FV1 + NFLTVARREGS  * 2
#else
	    R_FA1 <= i && i < R_FA1 + NFLTARGREGS  ||
            R_FP1 <= i && i < R_FP1 + NFLTARGREGS  ||
            R_FT1 <= i && i < R_FT1 + NFLTTEMPREGS ||
            R_FV1 <= i && i < R_FV1 + NFLTVARREGS
#endif
	    )
	  {
            augment_RealRegSet(&m_fltregs,i);
	  }
#endif

#ifdef ADDRESS_REG_STUFF
    memclr((VoidStar)&m_addrregs, sizeof(RealRegSet));
    for (i = 0; i<NMAGICREGS; i++)
        if (target_isaddrreg_(i))
            augment_RealRegSet(&m_addrregs,i);
#endif

    memclr((VoidStar)&globalregvarvec, sizeof(RealRegSet));
    globalregvarset = 0;

#ifdef ENABLE_SPILL
    tot_real_spills = tot_cse_spills = tot_cost = choose_count = 0;
#endif
}

void regalloc_tidy(void)
{
#ifdef ENABLE_SPILL
    if (debugging(DEBUG_SPILL))
        cc_msg("%lu binders spilt, %lu CSEs re-evaluated, total cost = %lu\n",
               tot_real_spills, tot_cse_spills, tot_cost);
#endif
    if (!debugging(DEBUG_STORE | DEBUG_REGS)) return;
    cc_msg("Regalloc max space stats:\n");
    stats_print(&maxstats);
    cc_msg("Dataflow time %ldcs, regalloc time %ld+%ldcs\n",
           (long)dataflow_clock, (long)regalloc_clock1, (long)regalloc_clock2);
}

/* The following routine should really be a 'const RealRegSet' extern   */
/* definition.  However, there seems to be no way to initialise it in   */
/* C.  Note that it gets called before regalloc_init() (e.g. builtin.c) */
/* and so the obvious initialisation fails.                             */

extern void reg_setallused(RealRegSet *s) { memset(s, 0xff, sizeof(*s)); }

/* end of regalloc.c */
