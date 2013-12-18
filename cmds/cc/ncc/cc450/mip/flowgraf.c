/*
 * C compiler file mip/flowgraf.c
 * Copyright (C) Codemist Ltd., 1988.
 * Copyright (C) Acorn Computers Ltd., 1988.
 * Copyright (C) Advanced Risc Machines Ltd., 1991
 */

/*
 * RCS $Revision: 1.6 $ Codemist 70
 * Checkin $Date: 1994/02/27 12:58:19 $
 * Revising $Author: nickc $
 */

/* AM hack in progress: fixup J_CMP to have the condition code which      */
/* matches the J_B which follows.  This probably makes Q_XXX obsolete.    */
/* Moreover, this can now remove CMP's at end of basic blocks which       */
/* have been make redundant by cross-jumping.                             */

/* Memo: it may be that tail-recursion optimisation should be done AFTER  */
/* cross_jumping.  This would allow common tails to be zipped.  However   */
/* it could lead to branches to branches if the tailcall expanded to just */
/* one branch instruction.                                                */
/* Memo: move SETSP amalgamation here from armgen.c etc.                  */

#ifdef __STDC__
#include <string.h>
#else
#include <strings.h>
#include <stddef.h>
#endif

#include "globals.h"
#include "flowgraf.h"
#include "store.h"
#include "cg.h"
#include "codebuf.h"
#include "regalloc.h"
#include "regsets.h"
#include "aeops.h"
#include "util.h"
#include "jopcode.h"
#include "mcdep.h"
#include "builtin.h"
#include "simplify.h"   /* for mcrep fields/macros */
#include "xrefs.h"      /* for xr_code/data */
#include "errors.h"

/* AM Sep 88: Use obj_symref before J_ADCON (may kill J_FNCON on day)    */
/* WGD  8- 3-88 Deadflag allowed for in expand_jop_macros */
/* WGD 26- 2-88 J_USEx passed to xxxgen as peepholer must also handle */
/*              volatiles carefully */
/* WGD 15- 2-88 Corrected treatement of deadflags for pseudo_reads_rx ops */
/* WGD 19-10-87 Tailcall conversion blocked for assembler_call */
/* WGD 16-10-87 J_FNCON now passes static/extern flag in r2 */
                /*@@@ AM:this shows that xr_xxx are not quite enough.    */
/* AM 26-may-87: redo CASEBRANCH tables                                  */

/* Memo: beware the use of rldiscard() in expand_jop_macro for 2 passes  */

/* AM has changed RETURN so that it passes the xxxgen.c file a
   (possibly conditional) branch to RETLAB.  This works nicely.
   However, we now have 5 special labels - RETLAB, RetIntLab, RetVoidLab,
   RetFltLab, NOTALAB.  The last is notionally NULL, but changed for
   testing.  Tidy sometime?
*/

/* The peepholer now effects the ARM elision of J_SETSP before RETURN.      */
/* This means that remove_noops() & branch_chain() are not perfect. Think.  */

/* @@@ The following lines are in flux:  we need an environment of          */
/* *user-declared* vars for debug info.  This probably can be amalgamated   */
/* with blkstack_ (discuss).  There are some importance differences.        */

BindListList *current_env;

#ifndef TARGET_IS_NULL

static BindListList *current_env2;

/* procedural interface to machine dependent code generator is via: */
/* show_instruction(), local_base(), local_address(), plus ...      */
static void show_inst_2(J_OPCODE op, VRegInt r1, VRegInt r2, VRegInt m)
{
    if (uses_r1(op))
    {   RealRegister r1r = register_number(r1.r);
        if (r1r >= (unsigned32)NMAGICREGS) syserr(syserr_r1r, (long)r1r);
        r1.rr = r1r;
    }
    if (uses_r2(op))
    {   RealRegister r2r = register_number(r2.r);
        if (r2r >= (unsigned32)NMAGICREGS) syserr(syserr_r2r, (long)r2r);
        r2.rr = r2r;
    }
    if (uses_r3(op))
    {   RealRegister mr = register_number(m.r);
        if (mr >= (unsigned32)NMAGICREGS) syserr(syserr_mr, (long)mr);
        m.rr = mr;
    }
#ifdef TARGET_HAS_2ADDRESS_CODE
    if (jop_asymdiadr_(op))
    {   /* code in regalloc has ensured that r1 & r3 clash if r1 != r2 */
        if (r1.rr != r2.rr && r1.rr == m.rr) syserr(syserr_expand_jop);
    }
    /* Maybe turn all 3-address codes to 2-address + MOVR here, but    */
    /* think more w.r.t use of load-address target opcodes.            */
#endif
    show_instruction(op, r1, r2, m);
}

static void show_inst_3K(J_OPCODE op, VRegnum r1, Binder *b, int32 k)
{
    VRegInt vr1, vr2, m;
    if (uses_r1(op))
    {   RealRegister r1r = register_number(r1);
        if (r1r >= (unsigned32)NMAGICREGS) syserr(syserr_r1r, (long)r1r);
        vr1.rr = r1r;
    }
    else vr1.r = r1;
    vr2.rr = local_base(b);
    m.i = local_address(b) + k;
#ifdef TARGET_HAS_RISING_STACK
    if ((bindaddr_(b) & BINDADDR_MASK) == BINDADDR_LOC)
      m.i += sizeof_int - ((b->bindmcrep) & MCR_SIZE_MASK);
#endif
    show_instruction(op, vr1, vr2, m);
}

static void show_inst_3(J_OPCODE op, VRegnum r1, Binder *b)
{   show_inst_3K(op, r1, b, 0);
}

#endif /* TARGET_IS_NULL */

/* statistics */
int32 icode_cur, block_cur;

/* the next 4 private vars control start_basic_block() and emit() */
static Icode *icodetop, *currentblock;
static int32 icoden;
static bool deadcode;  /* says to lose code = no current block */

typedef struct FreeIcodeChunk {
    struct FreeIcodeChunk *next;
    int32 size;
} FreeIcodeChunk;

static FreeIcodeChunk *holes;

/* A list of the gaps at the top of Icode segments, for the use of CSE
 * and loop optimisations (NOT used while emitting code).  The chaining
 * is in the Icode blocks themselves.
 */

static BlockHead *block_header;
BlockHead *top_block, *bottom_block;   /* exported to cg/regalloc */
/* beware: way_out is used for two different purposes.                  */
static LabelNumber *way_out;

BindList *active_binders;

Binder *juststored;
VRegnum justregister;

#define size_of_binders(l) sizeofbinders(l, NO)

int32 sizeofbinders(BindList *l, bool countall)
/* return total size of non-slaved binders in the given list             */
/* Note that the BindList is in 'most recently bound first' order.       */
/* Thus, if alignof_double>alignof_int we must be careful to pad         */
/* appropriately (a la padsize).  But normal padding algorithms pad      */
/* from the zero origin end.  Hence the special one here.                */
{
    int32 m = 0, m1 = 0;
    bool dbleseen = 0;
    for (; l!=NULL; l = l->bindlistcdr)
    {   Binder *b = l->bindlistcar;
        if (!(bindstg_(b) & bitofstg_(s_auto)))
            syserr(syserr_nonauto_active);
        if (bindxx_(b) == GAP || countall)
/* It is important that Binders processed here have had mcrepofexpr()      */
/* called on them before so that a cached rep is present. This is because  */
/* the TypeExpr may have been thrown away.                                 */
        {   int32 rep = b->bindmcrep;
            if (rep == NOMCREPCACHE) syserr(syserr_size_of_binder);
/* The next line fixes up the backward scan of offsets.                  */
            if (rep & MCR_ALIGN_DOUBLE && !dbleseen)
                (dbleseen = 1, m1 = m, m = 0);
            /* check the next line */
            m = padtomcrep(m, rep) +
                padsize(rep & MCR_SIZE_MASK, alignof_toplevel);
        }
    }
    if (dbleseen) m = padsize(m, alignof_double);
    return m+m1;
}

static BlockHead *newblock2(LabelNumber *lab, BindList *active_on_entry)
{   /* one day it may be nice to make the 2nd arg a union { int; BindList *} */
    BlockHead *p = (BlockHead *) BindAlloc(sizeof(BlockHead));
    block_cur += sizeof(BlockHead);
    blkcode_(p) = (Icode *) DUFF_ADDR;
    blkusedfrom_(p) = NULL;
    blklab_(p)  = lab;
    blklength_(p) = 0;
    blknext_(p) = (LabelNumber *) DUFF_ADDR;
    blknext1_(p) = (LabelNumber *) DUFF_ADDR;
    blkflags_(p) = 0;
    blkuse_(p) = 0;
    blkstack_(p) = active_on_entry;
    blknest_(p) = 0;
    return(p);
}

static BlockHead *newblock(LabelNumber *lab, BindList *active_on_entry)
{
    BlockHead *p = newblock2(lab, active_on_entry);
    if (bottom_block!=0) blkdown_(bottom_block) = p;
    blkup_(p) = bottom_block;
    blkdown_(p) = NULL;
     blkdebenv_(p) = current_env;
    bottom_block = block_header = p;
    return(p);
}

BlockHead *insertblockbetween(BlockHead *before, BlockHead *after)
{
    LabelNumber *newlab = nextlabel();
    BlockHead *newbh = newblock2(newlab, blkstack_(after));
    newlab->block = newbh;
    blknext_(newbh) = blklab_(after);
    blkdown_(newbh) = blkdown_(before);
    blkup_(newbh) = before;
    blkup_(blkdown_(before)) = newbh;
    blkdown_(before) = newbh;
    blkdebenv_(newbh) = blkdebenv_(after);
    changesuccessors(before, newlab, blklab_(after));
    return newbh;
}

void changesuccessors(BlockHead *b, LabelNumber *newl, LabelNumber *old)
{
    if (blknext_(b) == old)
        blknext_(b) = newl;

    else if ((blkflags_(b) & BLK2EXIT) &&
             blknext1_(b) == old)
        blknext1_(b) = newl;

    else {
        bool replaced = NO;
        if (blkflags_(b) & BLKSWITCH) {
            LabelNumber **table = blktable_(b);
            int32 i, n = blktabsize_(b);
            for (i=0; i<n; i++)
                if (table[i] == old)
                    replaced = YES, table[i] = newl;
        }
        if (!replaced)
            syserr(syserr_insertblockbetween, (int32)lab_name_(blklab_(b)),
                                              (int32)lab_name_(old));
    }
}

void finishblock(void)
{
    blkcode_(block_header) = currentblock;
    blklength_(block_header) = icoden;
    currentblock = &currentblock[icoden];
    icoden = 0;
}

void end_emit(void)
{
    if (currentblock < icodetop)
        freeicodeblock(currentblock, icodetop - currentblock);
}

Icode *newicodeblock(int32 size)
{
    FreeIcodeChunk *p = holes, *prev = NULL;
    while (p != NULL) {
        int32 left = p->size - size;
        if (left >= 0) {
            if (left == 0) {
                if (prev == NULL)
                    holes = p->next;
                else
                    prev->next = p->next;
                return (Icode *)p;
            } else {
                p->size = left;
                return &((Icode *)p)[left];
            }
        }
        prev = p; p = p->next;
    }
    return (Icode *) BindAlloc(size * sizeof(Icode));
}

void freeicodeblock(Icode *p, int32 size)
{
    FreeIcodeChunk *q = (FreeIcodeChunk *) p;
    q->next = holes; q->size = size;
    holes = q;
}

void reopen_block(BlockHead *p)
{
/* This is required during loop optimisation. *p is a block that has no  */
/* code in it but which is otherwise complete. Set it up so that I can   */
/* emit() code into the block.                                           */
/* Note that I will need to call finishblock() again when I am done.     */
    IGNORE(p);
    syserr(syserr_reopen_block);
}

BlockHead *start_basic_block_at_level(LabelNumber *l, BindList *active_on_entry)
{
    BlockHead *b;
    if (!deadcode) emitbranch(J_B, l);   /* round off the previous block    */
    l->block = b = newblock(l, active_on_entry);    /* set the label        */
    deadcode = 0;                        /* assume label will be referenced */
    juststored = NULL;
    if (debugging(DEBUG_CG)) cc_msg("L%ld:\n", (long)lab_name_(l));
    return b;
}

bool is_exit_label(LabelNumber *ll)  /* exported to jopprint.c/cse.c. (lab_xname_)  */
{
    if (ll == RetIntLab || ll == RetFloatLab || ll == RetDbleLab ||
        ll == RetVoidLab || ll == RetImplLab ||
/* RETLAB is present here so that print_jopcode may safely be used on the
 * arguments to show_instruction (when all the above have turned into RETLAB).
 */
        ll == RETLAB) return YES;
    else return NO;
}

/* emit() really takes a union mode as its last arg, so I provide a      */
/* number of entrypoints here so that I can preserve some type security  */
/* despite this mess.                                                    */

static void inner_emit(J_OPCODE, VRegInt, VRegInt, VRegInt);

void emitfl(J_OPCODE op, FileLine fl)
{
    VRegInt r1, r2, m;
    r1.p = fl.p;
    r2.str = fl.f;
    m.i = fl.l;
    inner_emit(op, r1, r2, m);
}

void emit5(J_OPCODE op, VRegnum r1, VRegnum r2, VRegnum r3, int32 m)
{
    VRegInt vr1, vr2, vm;
    vr1.r = r1;
    vr2.r = r2;
    vm.r = r3;
    if (m == 0) inner_emit(op, vr1, vr2, vm);
#ifdef TARGET_HAS_SCALED_ADDRESSING
    else inner_emit(op | (m << J_SHIFTPOS) & J_SHIFTMASK, vr1, vr2, vm);
#else
    else syserr(syserr_scaled_address);
#endif
}

void emitstring(J_OPCODE op, VRegnum r1, StringSegList *m)
{
    VRegInt vr1, vr2, vm;
    vr1.r = r1;
    vr2.r = GAP;
    vm.s = m;
    inner_emit(op, vr1, vr2, vm);
}

void emitbranch(J_OPCODE op, LabelNumber *m)
{
    VRegInt vr1, vr2, vm;
    vr1.r = GAP;
    vr2.r = GAP;
    vm.l = m;
    inner_emit(op, vr1, vr2, vm);
}

void emitbinder(J_OPCODE op, VRegnum r1, Binder *m)
{
    VRegInt vr1, vr2, vm;
    vr1.r = r1;
    vr2.r = GAP;
    vm.b = m;
    inner_emit(op, vr1, vr2, vm);
}

void emitvk(J_OPCODE op, VRegnum r1, int32 n, Binder *m)
{
    VRegInt vr1, vr2, vm;
    vr1.r = r1;
    vr2.i = n;
    vm.b = m;
    inner_emit(op, vr1, vr2, vm);
}

void emitreg(J_OPCODE op, VRegnum r1, VRegnum r2, VRegnum m)
{
    VRegInt vr1, vr2, vm;
    vr1.r = r1;
    vr2.r = r2;
    vm.r = m;
    inner_emit(op, vr1, vr2, vm);
}

void emitfloat(J_OPCODE op, VRegnum r1, VRegnum r2, FloatCon *m)
{
    VRegInt vr1, vr2, vm;
    vr1.r = r1;
    vr2.r = r2;
    vm.f = m;
    inner_emit(op, vr1, vr2, vm);
}

void emitpush(J_OPCODE op, VRegnum r1, VRegnum r2, RegList *m)
{
    VRegInt vr1, vr2, vm;
    vr1.r = r1;
    vr2.r = r2;
    vm.rl = m;
    inner_emit(op, vr1, vr2, vm);
}

void emitsetsp(J_OPCODE op, BindList *b2)
{
/* Note optimization performed here.                                     */
    if (active_binders != b2)
    {   VRegInt vr1, vr2, vm;
        vr1.r = GAP;
        vr2.bl = active_binders;
        vm.bl = b2;
        inner_emit(op, vr1, vr2, vm);
        active_binders = b2;
    }
}

void emitsetspandjump(BindList *b2, LabelNumber *l)
{
/* Note optimization performed here.                                     */
    if (active_binders != b2)
    {   VRegInt vr1, vr2, vm;
        vr1.r = GAP;
        vr2.bl = active_binders;
        vm.bl = b2;
        inner_emit(J_SETSPENV, vr1, vr2, vm);
    }
    {   VRegInt r1, r2, m;
        r1.r = GAP;
        r2.r = GAP;
        m.l = l;
        inner_emit(J_B, r1, r2, m);
    }
}

void emitsetspenv(BindList *b1, BindList *b2)
{
    emit(J_SETSPENV, GAP, (VRegnum)b1, (int32)b2);
}

void emitsetspgoto(BindList *b1, LabelNumber *l)
{
    emit(J_SETSPGOTO, GAP, (VRegnum)b1, (int32)l);
}

void emitcall(J_OPCODE op, VRegnum resreg, int32 nargs, Binder *fn)
{
    emit(op, resreg, (VRegnum)nargs, (int32)fn);
}

void emitcallreg(J_OPCODE op, VRegnum resreg, int32 nargs, VRegnum fn)
{
    emit(op, resreg, (VRegnum)nargs, (int32)fn);
}

void emitcasebranch(VRegnum r1, LabelNumber **tab, int32 size)
{
    emit(J_CASEBRANCH, r1, (VRegnum)tab, size);
}

void emit(J_OPCODE op, VRegnum r1, VRegnum r2, int32 m)
{
    VRegInt vr1, vr2, vm;
    vr1.r = r1;
    vr2.r = r2;
    vm.i = m;
    inner_emit(op, vr1, vr2, vm);
}

static void inner_emit(J_OPCODE op, VRegInt r1, VRegInt r2, VRegInt m)
{
    if (deadcode) {
        if (!usrdbg(DBG_LINE)) return;
        start_new_basic_block(nextlabel()); }

    if (op != J_MOVR ||
        r1.r==justregister) juststored = NULL;
    if (op==J_B)
    {   deadcode = 1;
        if (debugging(DEBUG_CG)) print_jopcode(op,r1,r2,m);
        finishblock();
        blknext_(block_header) = m.l;
        return;
    }
/* Conditional branches terminate a basic block, so force a new start    */
/* if one has just been appended to the block                            */
    if ((op & ~Q_MASK)==J_B)
    {   blknext1_(block_header) = m.l;
        blkflags_(block_header) |= BLK2EXIT;
        blkflags_(block_header) |= op & Q_MASK;
        if (debugging(DEBUG_CG)) print_jopcode(op,r1,r2,m);
        start_new_basic_block(nextlabel());
        return;
    }
/* Some peephole transformations appear here (they are machine indep)    */
/* N.B. soon swap the new LDRxK with the ADCONV if possible --           */
/* this is good for loading structs into reg. args.                      */
/* Better is to make sure CSE can map ADCONV/LDRK to LDRVK and then      */
/* remove this code (as it can get in the way with struct args).         */
    if (icoden != 0) switch (op & ~(J_SIGNED|J_UNSIGNED|J_ALIGNMENT))
    {   Binder *b;
default:        break;
case J_LDRK:
case J_STRK:
case J_LDRBK:   /* NB signed & unsigned version exist */
case J_STRBK:
case J_LDRWK:   /* NB signed & unsigned version exist */
case J_STRWK:
case J_LDRFK:
case J_STRFK:
case J_LDRDK:
case J_STRDK:
                if (currentblock[icoden-1].op == J_ADCONV &&
                    currentblock[icoden-1].r1.r == r2.r &&
                    (b = currentblock[icoden-1].m.b,
                     (bindstg_(b) & PRINCSTGBITS) == bitofstg_(s_auto)))
/* Convert LDRK into LDRVK - the ADCONV is then often dead & will get      */
/* removed later, but note that in the expansion of _memcpy it can still   */
/* be needed.  AM: Is this a lie?  (I have just fixed cg_fnargs wrt this.) */
                {   op = J_addvk(op & ~J_ALIGNMENT);
                    r2 = m;
                    m = currentblock[icoden-1].m;
                }
    }
/* The next line fixes to recover when I fill up a segment of store      */
    if (&currentblock[icoden] >= icodetop)
    {   if (icoden >= ICODESEGSIZE/2)
        {   if (debugging(DEBUG_CG | DEBUG_STORE))
                cc_msg("Force new block (ICODE segment overflow)\n");
            start_new_basic_block(nextlabel());
        }
/* The above arranges that icoden will be zero (following the call to      */
/* start_new_basic_block() -> emitbranch() -> finishcode()) if it had      */
/* originally been huge, and thus that I will not attempt an embarassingly */
/* large copy up into the new segment allocated here.                      */
        {   Icode *p = (Icode *) BindAlloc(ICODESEGSIZE*sizeof(Icode));
            if (icoden != 0)
            {   memcpy(p, currentblock, (size_t)(icoden*sizeof(Icode)));
                freeicodeblock(currentblock, icoden);
            }
            currentblock = p;
            icodetop = &p[ICODESEGSIZE];
            if (debugging(DEBUG_CG))
                cc_msg("New ICODE segment allocated\n");
            icode_cur += icoden * sizeof(Icode);   /* wasted */
        }
    }
    if (debugging(DEBUG_CG)) print_jopcode(op,r1,r2,m);
    currentblock[icoden].op = op;
    currentblock[icoden].r1 = r1;
    currentblock[icoden].r2 = r2;
    currentblock[icoden++].m = m;
    icode_cur += sizeof(Icode);
    if (isproccall_(op) && op != J_OPSYSK)
    {   if (blkflags_(block_header) & BLKCALL)
            blkflags_(block_header) |= BLK2CALL;
        blkflags_(block_header) |= BLKCALL;
        /* see tail recursion optimisation comment below */
        if ((op == J_CALLR && (config & CONFIG_INDIRECT_SETJMP)) ||
            (op == J_CALLK && (bindsym_(m.b) == setjmpsym
#ifdef TARGET_IS_C40
			       || bindsym_(m.b) == SaveCPUStatesym 
#endif
			       )))
        {   blkflags_(block_header) |= BLKSETJMP;
            if (feature & FEATURE_UNIX_STYLE_LONGJMP)
            /* We need this information early (to be able to turn off CSE),
             * but can't do branch_chain (where it is otherwise set) before
             * loop_optimise, because that turns some empty blocks into
             * non-empty ones.
             */
                procflags |= BLKSETJMP;
        }

    }
    if (op==J_CASEBRANCH)   /* special way to end a block */
    {   deadcode = 1;
        finishblock();
        blkflags_(block_header) |= BLKSWITCH;
        blktable_(block_header) = r2.lnn;
        blktabsize_(block_header) = m.i;
    }
}

/* AM believes that remove_noops logically fits here now. */

#ifndef TARGET_IS_NULL

bool is_compare(J_OPCODE op) {
#ifdef TARGET_HAS_SCALED_ADDRESSING
/* AM is about to invent J_OPMASK which is around 1023 to avoid this!   */
    J_OPCODE realop = op & ~(Q_MASK | J_SHIFTMASK | J_DEADBITS);
#else
    J_OPCODE realop = op & ~(Q_MASK | J_DEADBITS);
#endif
    return (realop == J_CMPK || realop == J_CMPR ||
            realop == J_CMPFR || realop == J_CMPDR ||
            realop == J_CMPFK || realop == J_CMPDK ||
            j_is_check(realop));
}

static Icode fg_pending;

static void expand_jop_macro(J_OPCODE op, VRegInt r1, VRegInt r2, VRegInt m)
{
#ifndef TARGET_IS_ADENART
#ifndef TARGET_IS_SPARC
    op &= ~J_ALIGNMENT;         /* forthcoming general feature.         */
#endif
#endif
        if (debugging(DEBUG_CG)) print_jopcode(op&~J_DEADBITS,r1,r2,m);
#ifdef TARGET_STACK_MOVES_ONCE
/*
 * TARGET_STACK_MOVES_ONCE is believed to be OK when used in combination
 * with NARGREGS==0, and in that case it drops SP once at the start of a
 * procedure and lifts it on exit.  If NARGREGS>0 all goes well when only
 * integer and pointer args are used.  However double and structure args
 * are handled by temporarily pushing them onto the stack and then popping
 * them into the argument registers.  While this is going on SP is not
 * where the MOVES_ONCE code expects it to be.  Again these temporary
 * movements are normally local and can cause no hassle - but it seems
 * possible that CSE optimisation and cross-jumping might sometimes
 * rearrange code in a way that in effect distributes the local stack
 * movement across several basic blocks.  At present this would break
 * the compiler, so with MOVES_ONCE and NARGREGS>0 it is suggested that
 * CSE and crossjumping be disabled for the while.  Work is in hand to
 * fix this problem by moving the offending args into registers directly
 * rather than via the stack.  Meanwhile a warning is generated if any
 * such local stack motion gets generated, and if the warning is not seen
 * all is well.
 */
    /* @@@ BEWARE: assumption here that fg_pending==J_NOOP.             */
    /* Turn a PUSHx into a STRK, but only if the push did not           */
    /* involve dropping the stack when NARGREGS>0.                      */
    /* E.g. struct { double d[3]; } x; ... f(x) ... with NARGREGS=4.    */
    switch (op & ~J_DEADBITS)
    {
case J_PUSHR: case J_PUSHD: case J_PUSHF:
        if ((unsigned32)m.i < 4*NARGREGS)
            cc_warn(warn_untrustable, currentfunction.symstr); /* syserr() */
        else
/* @@@ (AM) BEWARE: do not trust this code if NARGREGS>0 since          */
/* it makes assumptions easily invalidated by crossjump/cse.            */
/* NB PUSHx codes are only used in function calls.                      */
        {   /* Forge a Binder sufficient for local_base/address.        */
            static Binder forgery;
            bindaddr_(&forgery) = BINDADDR_LOC |
                                  (greatest_stackdepth - (m.i - 4*NARGREGS));
/* @@@ "op & ~J_DEADBITS" was innocent effect of old code.  Check?!!    */
            show_inst_3(J_XtoY(op&~J_DEADBITS, J_PUSHR, J_STRK) | op&J_DEAD_R1,
                        r1.r, &forgery);
            return;
        }
/* @@@ (AM) BEWARE: do not trust this code if NARGREGS>0 since          */
/* it makes assumptions easily invalidated by crossjump/cse.            */
/* NB PUSHx codes are only used in function calls.                      */
        cc_warn(warn_untrustable, currentfunction.symstr); /* syserr() */
    }
#endif
#ifndef EXPERIMENTAL_68000
    if ((op & ~J_DEADBITS) == J_PUSHR)
        op = J_PUSHM, m.i = regbit(register_number(r1.r)),
                      r1.r = r2.r = GAP;
#endif
/* Now, a (mini-)jopcode peepholer...   ************************        */
/* Currently this just amalgamates J_PUSHR's, but J_SETSP's are next.   */
/* See the BEWARE for TARGET_STACK_MOVES_ONCE above.                    */
    if (op == J_PUSHM)  /* && (pending==J_NOOP || pending==J_PUSHM)     */
    {
#ifdef TARGET_HAS_RISING_STACK
        /* The following check is that regs are ascending (this         */
        /* relies on the current code in cg.c).                         */
        if (fg_pending.m.i & -m.i) syserr(syserr_expand_pushr);
#else
        /* The following check is that regs are descending (this        */
        /* relies on the current code in cg.c).                         */
        if (m.i & -fg_pending.m.i) syserr(syserr_expand_pushr);
#endif
        fg_pending.op = J_PUSHM, fg_pending.m.i |= m.i;
        return;
    }
    if (fg_pending.op == J_PUSHM)
    {   VRegInt vr1, vr2;
        vr1.r = GAP;
        vr2.r = GAP;
        show_inst_2(J_PUSHM, vr1, vr2, fg_pending.m);
        fg_pending.op = J_NOOP, fg_pending.m.i = 0;
    }
/* end of peepholer                     ************************        */

    switch (op & ~(J_SIGNED|J_UNSIGNED|J_DEADBITS|J_ALIGNMENT))
    {
#ifdef TARGET_STACK_MOVES_ONCE
case J_SETSP: return; /* Ignore.  @@@ temporary placing here.          */
#endif
#ifndef TARGET_HAS_SIGN_EXTEND
case J_EXTEND:
#  ifdef TARGET_LACKS_SIGNED_SHIFT
/* This trick could also be useful for loading signed chars on machines */
/* (like ARM) with load-byte with zero-extend, since the ANDK could     */
/* be lost in the load-byte and the SUBK might fold with another const. */
        {   int32 msb = (m.i == 2 ? (int32)0x8000 : 0x80);
            m.i = (msb << 1) - 1;
            show_inst_2(J_ANDK, r1, r2, m);
            m.i = msb;
            show_inst_2(J_EORK, r1, r1, m);
            show_inst_2(J_SUBK, r1, r1, m);
        }
#  else
        m.i = m.i == 2 ? 16: 24;
        show_inst_2(J_SHLK, r1, r2, m);
        show_inst_2(J_SHRK+J_SIGNED, r1, r1, m);
# endif
        return;
#endif
/* The rest of this 'switch' deals with JOPCODE operators which are  */
/* macro-expanded to others on every conceivable machine.            */
/* N.B. the code below is slowly moving into remove_noops()          */
/* The next few groups of cases should not be here - see remove_noops */
case J_LDRV:case J_STRV:case J_LDRFV:case J_STRFV:
case J_LDRDV:case J_STRDV:
        {   Binder *bb = m.b;
            if (bindxx_(bb) != GAP)
                syserr(syserr_remove_noop_failed);
/* WGD 8-3-88 deadflag allowed for below */
/* @@@ "op & ~J_DEADBITS" was innocent effect of old code.  Check?!!    */
            op = loads_r1(op) ?
                J_XtoY(op&~J_DEADBITS, J_LDRV, J_LDRK) :
                J_XtoY(op&~J_DEADBITS, J_STRV, J_STRK) | (op&J_DEAD_R1),
#ifdef NEW_J_ALIGN_CODE             /* was TARGET_IS_ADENART */
/* It is arguably better that J_ALIGNMENT should be put on LDRV earlier.*/
            op |= alignof_toplevel == 8 ? J_ALIGN8 :
                  alignof_toplevel == 4 ? J_ALIGN4 : J_ALIGN4;
#endif
            show_inst_3(op, r1.r, bb);
        }
        return;
case J_LDRV1:case J_LDRFV1:case J_LDRDV1:
        {   Binder *bb = m.b;
            if (!isany_realreg_(register_number(r1.r)))
                syserr(syserr_remove_noop_failed2);
            if ((bindaddr_(bb) & BINDADDR_MASK)!=BINDADDR_ARG) /* DEAD? */
                syserr(syserr_bad_bindaddr);
/* @@@ "op & ~J_DEADBITS" was innocent effect of old code.  Check?!!    */
            op = J_XtoY(op&~J_DEADBITS, J_LDRV1, J_LDRK),
#ifdef NEW_J_ALIGN_CODE             /* was TARGET_IS_ADENART */
            op |= alignof_toplevel == 8 ? J_ALIGN8 :
                  alignof_toplevel == 4 ? J_ALIGN4 : J_ALIGN4;
#endif
            show_inst_3(op, r1.r, bb);
        }
        return;
case J_LDRBVK:case J_STRBVK:
case J_LDRWVK:case J_STRWVK:
case J_LDRVK: case J_STRVK:
case J_LDRFVK:case J_STRFVK:
case J_LDRDVK:case J_STRDVK:
case J_LDRLVK:case J_STRLVK:
        show_inst_3K(J_subvk(op), r1.r, m.b, r2.i);
        return;
case J_ADCONV:
        show_inst_3(J_ADDK, r1.r, m.b);
        return;

/* Here (in the case that string literals are to be writable, we remove  */
/* J_STRING opcodes in favour of J_ADCON.                                */
case J_STRING:
        if (feature & FEATURE_WR_STR_LITS)
          /*
           * Pcc-mode - string lits writable in the data segment.
           * Handle this by generating the string lit then falling
           * through to J_ADCON to generate a suitable adcon to address it.
           * J_ADCON expects a <offset, symbol> pair in <r2, m>.
           */
        {   int32 offset = data.size;   /* The literal's about to be */
                                        /* put at this offset ...  */
            VRegInt vr2, vm;
            vg_genstring(m.s, stringlength(m.s)+1, 0);
            /* The following call is unexpectedly important...            */
            /* vg_genstring() does not post-align, and other genXXX() fns */
            /* do not pre-align, so death is inevitable if we omit it.    */
/* It also plays its part in optimising string constants to word          */
/* boundaries which improves performance on many machines (not just RISC) */
            padstatic(alignof_toplevel);
            /* since datasegment is defined, it will have been obj_symref'd */
            vr2.i = offset;
            vm.sym = bindsym_(datasegment);
            show_inst_2(J_ADCON, r1, vr2, vm);
            return;
        }
        r2.i = 0;
        break;

/* normalise some Binder's to Symstr's for xxxgen.c */
case J_ADCON:
        {   Binder *bb = m.b;
            Symstr *name = bindsym_(bb);
            int32 offset = 0;
#ifdef TARGET_IS_HELIOS
            extern int suppress_module;
            if (suppress_module != 1)   /* See diatribe against this in cg.c */
#endif
            {
#ifdef TARGET_HAS_BSS
                if ((bindstg_(bb) & u_bss) && bindaddr_(bb) != BINDADDR_UNSET)
                {   name = bindsym_(bsssegment);
                    offset = bindaddr_(bb);
                } else
#endif
#ifdef CONST_DATA_IN_CODE
                if (bindstg_(bb) & u_constdata)
                {   name = bindsym_(constdatasegment);
                    offset = bindaddr_(bb);
                } else
#endif
                if ((bindstg_(bb) & b_fnconst+bitofstg_(s_static)+u_loctype)
                        == bitofstg_(s_static))
/* static variables get referenced relative to a static base             */
/* extdef variables can be referenced that way, but here it seems nicer  */
/* (and certainly it is no more expensive) to use the external symbol.   */
                {   name = bindsym_(datasegment);
                    offset = bindaddr_(bb);
                }
            }
/* Announce to the linker the flavour (code/data) of the symbol.         */
            (void)obj_symref(name, (bindstg_(bb) & b_fnconst ?
                                             xr_code : xr_data) |
                                   (bindstg_(bb) & bitofstg_(s_weak) ?
                                             xr_weak : 0),
                             0);
/* The next line of code is probably dying given the obj_symref() above. */
            {
#ifdef TARGET_CALL_USES_DESCRIPTOR
                if (bindstg_(bb) & b_fnconst)
                {
                    /* WGD: pass static/extern flag in r2 */
                    r2.i = bindstg_(bb) & bitofstg_(s_static);
                    m.sym = name;
                    op = J_FNCON;
                }
                else
#endif
                {   r2.i = offset;
                    m.sym = name;
                }
            }
        }
        break;
#ifndef TARGET_FP_ARGS_IN_FP_REGS
case J_ENTER:
        m.i = k_argwords_(m.i);
        break;
#endif
case J_TAILCALLK:
case J_CALLK:
/* @@@ BUG here:  1. if m is a local static data binder, then it does   */
/* not get defined in cfe.c.vargen (to avoid clashes and make sure      */
/* store does not vanish under out feet).  Thus this code should        */
/* resolve to codesegment+nnn.  However, unlike the ADCON case above    */
/* there is no room for the nnn.                                        */
/* 2. Moreover, the current (Dec 88) a.out formatter does not treat     */
/* X_PCreloc branches to data segment correctly -- see AM comments.     */
/* (This explains acorn's curious code in arm/mcdep.c).                 */
/* One fix is just to syserr(), or to fix into a CALLR.  But beware     */
/* that back ends do not support TAILCALLR yet (ANSI/register use).     */
        {
#ifdef TARGET_FP_ARGS_IN_FP_REGS
#define ArgWordMask (~K_FLAGS)
#else
#define ArgWordMask K_ARGWORDMASK
#endif
#ifdef TARGET_FLAGS_VA_CALLS
            r2.i &= ArgWordMask | K_VACALL;
#else
            r2.i &= ArgWordMask;
#endif
            m.sym = bindsym_(m.b);
/* The next line simplifies CALLK to match the ADCON case by declaring  */
/* the branch target as code.   Back ends now needn't do this too.      */
            obj_symref(m.sym, xr_code, 0);
        }
        break;
#ifdef TARGET_IS_ARM
case J_MOVDIR:                          /* to move to arm/gen.c...      */
        {   VRegInt gap, zero; gap.r = GAP; zero.i = 0;
            show_inst_2(J_PUSHD, m, gap, zero);
            /* let's check the two regs are in correct order!           */
            m.i = regbit(register_number(r1.r)) |
                  regbit(register_number(r2.r));
            show_inst_2(J_POPM, gap, gap, m);
        }
        return;
#endif
case J_POP:                             /* dying.                       */
        {   RegList *p = m.rl;
            m.i = 0;
            for (; p!=NULL; p = rldiscard(p))
                m.i |= regbit(register_number(p->rlcar));
            op = J_POPM;
        }
        break;
#ifdef TARGET_LACKS_RIGHTSHIFT
/* then map constant right shifts here to leftright, expr ones done in cg.c */
case J_SHRK:
        m.i = -m.i;
        op ^= J_SHRK^J_SHLK;
        break;
#endif /* TARGET_LACKS_RIGHTSHIFT */
case J_NOOP:
        syserr(syserr_remove_noop_failed);
        return;
#ifdef TARGET_LDRK_MAX
/* The following cases are there to ensure that STRxK does not use IP */
/* when regalloc says it doesn't.                                     */
case J_LDRFK: case J_STRFK:
case J_LDRDK: case J_STRDK:
#ifdef TARGET_LDRFK_MAX
        if (m.i < TARGET_LDRFK_MIN || m.i > TARGET_LDRFK_MAX ||
            m.i & (15 & ~(int32)TARGET_LDRFK_MAX)) /* TARGET_LDRK_QUANTUM */
                syserr(syserr_ldrfk, (long)m.i);
        break;
#endif
case J_LDRK: case J_STRK:
case J_LDRBK: case J_STRBK:
case J_LDRWK: case J_STRWK:
        if (m.i < TARGET_LDRK_MIN || m.i > TARGET_LDRK_MAX ||
            m.i & (15 & ~(int32)TARGET_LDRK_MAX)) /* TARGET_LDRK_QUANTUM */
                syserr(syserr_ldrk, (long)m.i);
        /* drop through */
#endif
    }
    show_inst_2(op, r1, r2, m);
}

static void show_branch_instruction(J_OPCODE op, LabelNumber *m)
{
    VRegInt vr1, vr2, vm;
    vr1.r = vr2.r = GAP;
    vm.l = m;
    expand_jop_macro(op, vr1, vr2, vm);
}

/* more of the debugger */
static void dbg_scope1(BindListList *newbl, BindListList *old)
{
    IGNORE(newbl); IGNORE(old); /* avoid warnings when dbg_scope is FALSE */
    if (dbg_scope(newbl, old))
    {   VRegInt vr1, vr2, vm;
        vr1.r = vr2.r = GAP;
        vm.i = 0;
        /* request code location where scope started or ended */
        expand_jop_macro(J_INFOSCOPE, vr1, vr2, vm);
    }
}

static BlockHead *prevblock;

static void show_basic_block(BlockHead *p, int32 cond)
{
    Icode *b = blkcode_(p);
    int32 len = blklength_(p);
    int32 b1;
   if (usrdbg(DBG_VAR)) dbg_scope1(blkdebenv_(p), current_env2),
                         current_env2 = blkdebenv_(p);
/* The BLKCODED bit allows me to display blocks out of their natural order */
/* @@@ but it may later cause trouble when DBG_VAR is on.                  */
/* AM: fix to suppress the spurious J_LABEL/J_STACK before J_ENTRY.     */
    blkflags_(p) |= BLKCODED;
    if (prevblock == NULL ||
        blkusedfrom_(p) == NULL ||
        blkusedfrom_(p)->blklstcdr != NULL ||
        blkusedfrom_(p)->blklstcar != prevblock ||
        (blkflags_(prevblock) & BLKSWITCH))
    {
        VRegInt vr1, vr2, vm;
        vr1.r = vr2.r = GAP;
        vm.l = blklab_(p);
        expand_jop_macro(J_LABEL, vr1, vr2, vm);    /* label of block */
#ifndef TARGET_STACK_MOVES_ONCE
        vm.i = blkstacki_(p);
        expand_jop_macro(J_STACK, vr1, vr2, vm);    /* env. of block  */
#endif
    }
    for (b1=0; b1<len; b1++)
    {   J_OPCODE op = b[b1].op;
        if (cond != Q_AL && b1 == len-1) switch (op & ~(Q_MASK|J_DEADBITS))
            case J_CMPK:  case J_CMPR:
            case J_CMPFK: case J_CMPFR:
            case J_CMPDK: case J_CMPDR:
                if ((op & Q_MASK) != Q_XXX) op = (op & ~Q_MASK) | cond;
        expand_jop_macro(op, b[b1].r1, b[b1].r2, b[b1].m);
    }
    prevblock = p;
}

static bool same_instruction(Icode *c1, Icode *c2) {
    /*Two instructions are deemed   */
    /* to match if they are identical when virtual registers have been   */
    /* converted to real register numbers wherever they are needed. The  */
    /* instructions that use pseudo-virtual registers to cope with items */
    /* lifted out of loops etc have to match even more exactly, thus the */
    /* tests here are conservative (and hence safe!).                    */
    /* For example they fail to spot equal J_POP's (which are dying).    */
    int32 op;
    if ((op = c1->op) == c2->op)
    {   VRegInt r11, r12, r21, r22, r31, r32;
        r11 = c1->r1;
        r12 = c2->r1;
        r21 = c1->r2;
        r22 = c2->r2;
        r31 = c1->m;
        r32 = c2->m;
/* The next fragment of code scares me somewhat - I need to see if the  */
/* two instructions being considered will be treated identically after  */
/* macro-expansion, where macro-expansion includes the replacement of   */
/* virtual register identifiers by real register numbers.               */
/* As more code moves from expand_jop_macro() to remove_noops this      */
/* will ease.                                                           */
/* Note that remove_noops() has removed any possible loop invariant     */
/* suggestions either by smashing to a RR operation or by               */
/* removing the (pseudo_reads_r1/2) potential invariant register field. */
        if (uses_r1(op))
        {   r11.r = (int)register_number(r11.r);
            r12.r = (int)register_number(r12.r);
        }
        if (uses_r2(op))
        {   r21.r = (int)register_number(r21.r);
            r22.r = (int)register_number(r22.r);
        }
        if (uses_r3(op))
        {   r31.r = (int)register_number(r31.r);
            r32.r = (int)register_number(r32.r);
        }
/* The test that follows is a PUN and is not guaranteed portable */
        if (r11.i == r12.i &&
            r21.i == r22.i &&
            r31.i == r32.i)
            return YES;
    }
    return NO;
}

#endif /* TARGET_IS_NULL */

#define is_empty_block(b) (blklength_(b) == 0) /* remove_noops tidied */

/* N.B. BLKEMPTY and BLKALIVE are disjoint.                             */
/* BLKEMPTY performs a similar role to BLKALIVE: it flags that the      */
/* block has been seen, BUT that it is not really alive.                */
/* Note that self referential empty blocks have BLKALIVE set instead.   */

static LabelNumber *branch_emptychain(LabelNumber *lab)
/* When we get an empty block we must flag it so (BLKEMPTY)  */
/* and change its blknext_() pointer to where it ULTIMATELY  */
/* goes.  Note that empty blocks are never BLKALIVE except   */
/* when they have a tight cycle which BLKBUSY detects.       */
{   LabelNumber *lab2 = lab, *lab3;
    /* skip to end of branch chain of empty basic blocks     */
    if (usrdbg(DBG_VAR+DBG_LINE)) return lab;
    while (!is_exit_label(lab2))
    {   BlockHead *b = lab2->block;
        /* Treat a block as empty if it does not have a conditional  */
        /* exit & it has no code (not overkill: three-way-branches). */
        if (blkflags_(b) & BLK2EXIT || !is_empty_block(b) ||
            (blkflags_(b) & (BLKBUSY|BLKALIVE|BLKEMPTY))) break;
        blkflags_(b) |= BLKBUSY;
        lab2 = blknext_(b);
    }
    /* the next line deals with the case that two empty chains have  */
    /* a common tail:  lab3 is the real end of the branch chain.     */
    lab3 = lab2;
    if (!is_exit_label(lab3))
    {   BlockHead *b = lab3->block;
        if (blkflags_(b) & BLKEMPTY) lab3 = blknext_(b);
    }
    /* now remove empty blocks (c.f. indirection nodes) */
    while (lab != lab2)
    {   BlockHead *b = lab->block;
        if (debugging(DEBUG_CG))
            cc_msg("empty block: L%ld -> L%ld\n", (long)lab_name_(lab),
                    (long)lab_xname_(lab3));
        blkflags_(b) |= BLKEMPTY;
        blkflags_(b) &= ~BLKBUSY;    /* only for tidyness       */
        lab = blknext_(b);
        blknext_(b) = lab3;          /* smash it in             */
    }
    return lab3;                     /* the real place to go to */
}

static LabelNumber *branch_chain(LabelNumber *lab, bool finalp)
/* scan the flowgraph (a) marking bit BLKALIVE in blkflags_ of blocks    */
/* that can be reached, and (b) converting any destination addresses in  */
/* block exits to avoid chains of branches.                              */
/* When called on label n, if label n references a branch instruction    */
/* this function returns the label number of the destination of the jump */
/*                                                                       */
/* This code now uses pointer-reversal since otherwise it can use up a   */
/* great deal of stack.                                                  */
{
    BlockHead *b, *back_pointer = NULL;
    for (;;)
    {   for (;;)  /* Here to descend via lab */
        {   lab = branch_emptychain(lab);
            if (is_exit_label(lab))
            {   if (lab == RetImplLab && !implicit_return_ok)
                    implicit_return_ok = 1;
#ifndef TARGET_IS_HELIOS
		/* XXX - NC - 10/8/93 - this test appears to be bogus for some unknown reason */
		cc_warn(flowgraf_warn_implicit_return);
#endif
                if (finalp) lab = way_out;
                break; /* exit: do not chain further */
            }
            b = lab->block;
            if (blkflags_(b) & BLKALIVE) break;       /* visited already */
            blkflags_(b) |= BLKALIVE;
#ifdef TARGET_HAS_TAILCALL
            /* set up the BLKSETJMP bit early enough for TAILCALL        */
            /* optimisation.  refblock() sets up the other bits.         */
            procflags |= blkflags_(b) & BLKSETJMP;
#endif
            if (blkflags_(b) & BLKSWITCH)
            {   LabelNumber **v = blktable_(b);
                int32 i, n = blktabsize_(b);
/* For the moment I will leave this next line as a recursive call        */
                for (i=0; i<n; i++) v[i] = branch_chain(v[i], finalp);
                break;
            }
            blkflags_(b) = (blkflags_(b) & ~(BLKP2|BLKP3)) | BLKP2;
            lab = blknext_(b); blkbackp_(b) = back_pointer;     /* share */
            back_pointer = b;
        }
        for (;;)        /* here to ascend */
        {   if (back_pointer==NULL) return lab;
            b = back_pointer;
            switch (blkflags_(b) & (BLKP2|BLKP3))
            {
    default:    syserr(syserr_branch_backptr);
    case BLKP2: back_pointer = blkbackp_(b); blknext_(b) = lab;  /* share */
                if (blkflags_(b) & BLK2EXIT)
                {   lab = blknext1_(b); blkbackp1_(b) = back_pointer; /* share */
                    blkflags_(b) = (blkflags_(b) & ~(BLKP2|BLKP3)) | BLKP3;
                    back_pointer = b;
                    break;
                }
                lab = blklab_(b);
                continue;
    case BLKP3: back_pointer = blkbackp1_(b); blknext1_(b) = lab; /* share */
/* Optimise away the conditional exit if both exits from this block go   */
/* to the same place.                                                    */
                if (blknext_(b) == blknext1_(b) ||
                    (is_exit_label(blknext_(b)) &&
                     is_exit_label(blknext1_(b)))) {
                    blkflags_(b) &= ~(BLK2EXIT|Q_MASK);
                    if (!(blkflags_(b) & BLKCCEXPORTED))
                        if (--blklength_(b) == 0) {
                            blkflags_(b) = (blkflags_(b) & ~BLKALIVE) | BLKEMPTY;
                            continue;
                        }
                }
                lab = blklab_(b);
                continue;
            }
            break;  /* so that break inside the switch = break in for    */
        }
    }
}

/* shouldn't the next line have BLK0EXIT? or is that done elsewhere?    */
#define block_single_entryexit(b) \
        (block_single_exit(b) && block_single_entry(b))

#define block_single_entry(b) \
        (blkusedfrom_(b) == NULL || blkusedfrom_(b)->blklstcdr == NULL)

#define block_single_exit(b) \
        (!(blkflags_(b) & (BLK2EXIT | BLKSWITCH)))

#define block_coded(b) (blkflags_(b) & BLKCODED)

static int32 adjust_stack_size(int32 stack_size, Icode *code) {
    switch (code->op & ~J_DEADBITS) {
/* This code seems distinctly grotty... */
    case J_SETSP:
        if (stack_size != code->r2.i)
            syserr(syserr_zip_blocks, stack_size, code->r2.i);
        return code->m.i;
    case J_PUSHD:
        return stack_size + 8;
/* alignof_toplevel==8 is a euphemism for TARGET_IS_ADENART, else 4.   */
    case J_PUSHR:
    case J_PUSHF:
        return stack_size + alignof_toplevel;
/* AM: use 4 (or sizeof_long?) on next lines in case sizeof_int==2.     */
/* (Oct 89) the following lines' case is under re-organisation.         */
    case J_POP:
        return stack_size - alignof_toplevel*length((List *)code->m.bl);
    default:
        return stack_size;
    }
}

static int32 adjusted_stack_size(int32 stack_size, Icode *code, int32 n) {
    int32 i;
    for (i = 0; i < n; i++)
        stack_size = adjust_stack_size(stack_size, &code[i]);
    return stack_size;
}

#ifdef TARGET_HAS_COND_EXEC

#define set_cond_execution(cond)        \
 { VRegInt vr1, vr2, vm;                \
   vr1.r = vr2.r = GAP;                 \
   vm.i = 0;                            \
   expand_jop_macro(J_CONDEXEC|(cond),vr1, vr2, vm); }

/* A block is not usable with conditional execution if if ends with a */
/* conditional branch (or switch), or if it is shared between the     */
/* path in the code where a condition code will be tested and any     */
/* other route through the code.                                      */
#define conditionalizable(b) \
        (block_single_entryexit(b) && block_preserves_cc(b, NO))

static bool block_preserves_cc(BlockHead *b, bool ignorelast)
/* True if the block does not contain any instructions that would cause  */
/* trouble with conditional execution & if the block is also short       */
{
    int32 i, n = 0;
    Icode *c = blkcode_(b);
    int32 len = blklength_(b);
    if (ignorelast) len--;
    for (i = 0; i < len; i++)
    {
#ifdef TARGET_IS_C40
      extern bool can_cond_exec( Icode * );
      
      if (!can_cond_exec( &c[i] ))
            return NO;
#else
      if (alterscc(&c[i]))
            return NO;
#endif
        switch (c[i].op & ~(Q_MASK|J_DEADBITS))
        {
#ifdef TARGET_HAS_BLOCKMOVE
#ifdef TARGET_IS_ARM
    case J_MOVC:
    case J_CLRC:
    /* Some cases get expanded into a loop ... (I really should do something
       about this disgusting magic number
     */
            if (c[i].m.i > 24) return NO;
            continue;
#endif
#endif
    case J_NOOP:   /* remove_noops has normalised noops to J_NOOP */
            syserr(syserr_remove_noop_failed);
            continue;
    default:
            if ((n += 1) > 3) return NO;
        }
    }
    return YES;
}

static void show_conditional_block(int32 cond, BlockHead *p, bool showbranch)
{
/* Show a block with conditional execution enabled                       */
    set_cond_execution(cond);
    show_basic_block(p, Q_AL);
    if (!(blkflags_(p) & BLK0EXIT) && showbranch) {
        LabelNumber *l = blknext_(p);
        show_branch_instruction(J_B, l == way_out ? RETLAB : l);
    }
    prevblock = NULL;
    set_cond_execution(Q_AL);
}

#define EQ_COND(x) ((x) == Q_EQ || (x) == Q_NE || (x) == Q_UNE || (x) == Q_UEQ)

static void show_blockchain(BlockHead *b, BlockList *bl, int32 cond) {
    /* Two surprising things are done here to stop the ARM peepholer
       making incorrect transformations:
         the cond field in comparisons may be set to Q_XXX to prevent its
         being changed in one place (but not consistently)
         a CONDEXEC jopcode is output before each block to stop all but
         the last comparison being optimised out.
     */
    int32 compcond = Q_NEGATE(cond);
    cond = compcond;
    if (!EQ_COND(cond)) compcond = Q_XXX;
    show_basic_block(b, compcond);
    for (; bl != NULL; bl = bl->blklstcdr)
    {   set_cond_execution(cond);
        show_basic_block(bl->blklstcar, compcond);
    }
}

static void pr_blocklist(BlockList *bl) {
    for (; bl != NULL; bl = bl->blklstcdr)
        cc_msg(" %ld", lab_name_(blklab_(bl->blklstcar)));
}

typedef struct CommonInst CommonInst;
struct CommonInst {
    CommonInst *cdr;
    int32 n1, n2;
};

static CommonInst *common_insts(BlockHead *b1, BlockHead *b2) {
    CommonInst *c = NULL, **cp = &c;
    Icode *c1 = blkcode_(b1), *c2 = blkcode_(b2);
    int32 l1 = blklength_(b1), l2 = blklength_(b2);
    int32 n1, n2 = 0;
    for (n1 = 0; n1 < l1; n1++) {
        int32 p2;
        for (p2 = n2; p2 < l2; p2++)
            if (same_instruction(&c1[n1], &c2[p2]) &&
                !alterscc(&c1[n1]) && !is_compare(c1[n1].op)) {
                CommonInst *ci = (CommonInst *)SynAlloc(sizeof(*ci));
                cdr_(ci) = NULL; ci->n1 = n1; ci->n2 = p2;
                *cp = ci; cp = &cdr_(ci);
                n2 = p2+1;
                break;
            }
    }
    return c;
}

static void show_block_pair(int32 cond, BlockHead *b1, BlockHead *b2, CommonInst *c) {
    Icode *c1 = blkcode_(b1), *c2 = blkcode_(b2);
    int32 l1 = blklength_(b1), l2 = blklength_(b2);
    int32 s1 = blkstacki_(b1), s2 = blkstacki_(b2);
    int32 n1, n2;
    int32 p1 = 0, p2 = 0;
    do {
        if (c == NULL) {
            n1 = l1; n2 = l2;
        } else {
            n1 = c->n1; n2 = c->n2;
            c = cdr_(c);
        }
        if (p1 != n1) {
            set_cond_execution(cond);
#ifndef TARGET_STACK_MOVES_ONCE
            if (s1 != s2)  {
                VRegInt vr1, vr2, vm;
                vr1.r = vr2.r = GAP;
                vm.i = s1;
                expand_jop_macro(J_STACK, vr1, vr2, vm);
            }
#endif
            for (; p1 < n1; p1++, c1++) {
                s1 = adjust_stack_size(s1, c1);
                expand_jop_macro(c1->op, c1->r1, c1->r2, c1->m);
            }
        }
        if (p2 != n2) {
            set_cond_execution(Q_NEGATE(cond));
#ifndef TARGET_STACK_MOVES_ONCE
            if (s1 != s2)  {
                VRegInt vr1, vr2, vm;
                vr1.r = vr2.r = GAP;
                vm.i = s2;
                expand_jop_macro(J_STACK, vr1, vr2, vm);
            }
#endif
            for (; p2 < n2; p2++, c2++) {
                s2 = adjust_stack_size(s2, c2);
                expand_jop_macro(c2->op, c2->r1, c2->r2, c2->m);
            }
        }
        if (n1 < l1) {
            set_cond_execution(Q_AL|Q_UBIT);
            s1 = adjust_stack_size(s1, c1);
            s2 = adjust_stack_size(s2, c2);
            expand_jop_macro(c1->op, c1->r1, c1->r2, c1->m);
            c1++; c2++;
            p1++, p2++;
        }
    } while (p1 < l1 || p2 < l2);
    blkflags_(b1) |= BLKCODED;
    blkflags_(b2) |= BLKCODED;
    set_cond_execution(Q_AL);
}

static bool successor_inline(BlockHead *b) {
    LabelNumber *next = blknext_(b);
    BlockHead *b1;
    BlockList *bl;
    if (next == way_out) return NO;
    b1 = next->block;
    if (block_coded(b1)) return NO;
    /* b1 will be coded inline after b if its only predecessors which    */
    /* have not yet been coded are b and b1 itself                       */
    for (bl = blkusedfrom_(b1); bl != NULL; bl = bl->blklstcdr) {
        BlockHead *b2 = bl->blklstcar;
        if (b2 != b && b2 != b1 && !block_coded(b2))
            return NO;
    }
    return YES;
}

#endif

#ifndef TARGET_IS_NULL

static void show_n(int32 n, Icode *c, int32 s) {
    int32 i;
    IGNORE(s);
    for (i = 0; i < n; i++, c++)
        expand_jop_macro(c->op, c->r1, c->r2, c->m);
}

static LabelNumber *use_cond_field(BlockHead *p)
{
/* Decide if I can afford to use conditional execution. To do so I need: */
/*                                                                       */
/*             A           A           A                 A               */
/*            / \         | \         | \               | \__(return)    */
/*           B   C        |  C        |  C              |                */
/*            \ /         | /         |   \__(return)   |                */
/*             D           D           D                 D               */
/*                                                                       */
/* where B and C are short blocks that do not disturb the condition      */
/* codes which are not reached from elsewhere.                           */

/* A  here may be a single block, or a sequence of 2-exit blocks with a  */
/* common successor (reached with the same condition code), with blocks  */
/* in the sequence single-entry and not disturbing condition codes       */
/* (except by the comparison at their end).                              */

/* If either successor block for A has been displayed already I do not   */
/* need to use conditional execution.                                    */
/* Block p has now not yet been displayed.                               */
    int32 cond = blkflags_(p) & Q_MASK;
    LabelNumber *next = blknext_(p), *next1 = blknext1_(p);
    BlockHead *b = 0, *b1 = 0;
    int32 next1_arcs = 1;
#ifdef TARGET_HAS_COND_EXEC
#  define show_head(p, cond) if (next1_arcs == 1) show_basic_block(p, cond)
#else
#  define show_head(p, cond) show_basic_block(p, cond)
#endif
    if (cond == Q_AL) syserr(syserr_no_main_exit);
    if (next != way_out) b = next->block;
    if (next1 != way_out) b1 = next1->block;
/*
    if (b == 0 && b1 == 0) syserr(syserr_two_returns);
*/
    blkflags_(p) |= BLKCODED;   /* a bit of a hack -- see show_basic_block */
#ifdef TARGET_HAS_COND_EXEC
    /* Look for a sequence of 2-exit blocks with a common successor */
    if (!usrdbg(DBG_VAR+DBG_LINE))
    {   int32 commoncond = 0;
        LabelNumber *commonexit = NULL;
        BlockHead *bh = NULL;
        if (b != NULL && !block_coded(b) && block_single_entry(b) &&
                         block_preserves_cc(b, YES)) {
            if (((blkflags_(b) & Q_MASK) == cond && blknext1_(b) == next1) ||
                ((blkflags_(b) & Q_MASK) == Q_NEGATE(cond) && blknext_(b) == next1)) {
                commoncond = cond;
                commonexit = next1;
                bh = b;
            }
        } else if (b1 != NULL && !block_coded(b1) && block_single_entry(b1) &&
                                 block_preserves_cc(b1, YES)) {
            if (((blkflags_(b1) & Q_MASK) == cond && blknext_(b1) == next) ||
                ((blkflags_(b1) & Q_MASK) == Q_NEGATE(cond) && blknext1_(b1) == next)) {
                commoncond = Q_NEGATE(cond);
                commonexit = next;
                bh = b1;
            }
        }

        if (commonexit != NULL) {
            BlockList *blockchain = NULL;
            next1 = commonexit;
            cond = commoncond;
            for (;;) {
                if (block_coded(bh) || !block_single_entry(bh) ||
                    !block_preserves_cc(bh, YES))
                    break;
                if ((blkflags_(bh) & Q_MASK) == cond &&
                    blknext1_(bh) == next1)
                    next = blknext_(bh);
                else if ((blkflags_(bh) & Q_MASK) == Q_NEGATE(cond) &&
                         blknext_(bh) == next1)
                    next = blknext1_(bh);
                else
                    break;
                blockchain = mkBlockList(blockchain, bh);
                if (next == way_out) break;
                bh = next->block;
            }
            blockchain = (BlockList *)dreverse((List *)blockchain);
            next1_arcs = length((List *)blockchain)+1;

            if (debugging(DEBUG_CG)) {
                cc_msg("Conditional chain %ld", lab_name_(blklab_(p)));
                pr_blocklist(blockchain);
                cc_msg(" cond %lx common successor %ld other successor %ld\n",
                       cond, lab_name_(next1), lab_name_(next));
                if (next1 != way_out) {
                    cc_msg("Predecessors of common successor:");
                    pr_blocklist(blkusedfrom_(next1->block));
                    cc_msg("\n");
                }
            }
            show_blockchain(p, blockchain, cond);
            set_cond_execution(Q_AL);

            if (next == next1)    /* I think this should now never happen */
                return next;
            b = (next == way_out) ? 0 : next->block;
            b1 = (next1 == way_out) ? 0 : next1->block;
        }
    }
#endif
    {   Icode *commonp = NULL;
        int32 ncommon = 0;
        int32 s = 0;
        if (b != 0 && b1 != 0 && !block_coded(b) && !block_coded(b1) &&
            block_single_entry(b) &&
            length((List *)blkusedfrom_(b1)) == next1_arcs) {
            int32 l1 = blklength_(b), l2 = blklength_(b1);
            Icode *c1 = blkcode_(b), *c2 = blkcode_(b1);
            commonp = c1;
            if (l1 > l2) l1 = l2;
#ifndef STACK_MOVES_ONCE
            s = blkstacki_(b);
            if (s != blkstacki_(b1))
                /* nothing */;
            else
#endif
            for (ncommon = 0; ncommon < l1; ncommon++, c1++, c2++)
                if (!same_instruction(c1, c2) || alterscc(c1) || is_compare(c1->op))
                    break;
            blklength_(b) -= ncommon; blklength_(b1) -= ncommon;
            blkcode_(b) += ncommon; blkcode_(b1) += ncommon;
            blkstacki_(b) = blkstacki_(b1) = adjusted_stack_size(s, commonp, ncommon);
        }
        /* backward branches have priority over conditional execution */
        if (b != 0 && block_coded(b))
        {   if (b1 == 0 || !block_coded(b1) || blknest_(b) > blknest_(b1)) {
                show_head(p, Q_NEGATE(cond));
                show_branch_instruction(J_B + Q_NEGATE(cond), next);
                return (next1 == way_out) ? RETLAB : next1;
            }
        }
        if (b1 != 0 && block_coded(b1))
        {   show_head(p, cond);
            show_branch_instruction(J_B + cond, next1);
            return (next == way_out) ? RETLAB : next;
        }
#ifdef TARGET_HAS_COND_EXEC
        if (!usrdbg(DBG_VAR+DBG_LINE))
        {   bool condb = b != 0 && !block_coded(b) && conditionalizable(b),
                 condb1 = b1 != 0 && !block_coded(b1) && block_single_exit(b1) &&
                          length((List *)blkusedfrom_(b1)) == next1_arcs &&
                          block_preserves_cc(b1, NO);
            if (condb && condb1 && blknext_(b) == blknext_(b1))
            {   CommonInst *c = common_insts(b1, b);
                show_head(p, cond);
                show_n(ncommon, commonp, s);
                show_block_pair(cond, b1, b, c);
                return blknext_(b) != way_out ? blknext_(b) :
         (blkflags_(b) & blkflags_(b1) & BLK0EXIT) ? NOTALAB :
                                                     RETLAB;
            }
            if (condb && (blknext_(b) == next1 || !successor_inline(b)))
            {   show_head(p, Q_NEGATE(cond));
                show_n(ncommon, commonp, s);
                show_conditional_block(Q_NEGATE(cond), b, blknext_(b) != next1);
                return next1 == way_out ? RETLAB : next1;
            }
            if (condb1 && (blknext_(b1) == next || !successor_inline(b1)))
            {   show_head(p, cond);
                show_n(ncommon, commonp, s);
                show_conditional_block(cond, b1, blknext_(b1) != next);
                return next == way_out ? RETLAB : next;
            }
        }
#endif
        if (b == 0)
        {   show_head(p, Q_NEGATE(cond));
            show_branch_instruction(J_B + Q_NEGATE(cond), RETLAB);
            return (next1 == way_out) ? RETLAB : next1;
        }
        if (b1 == 0)
        {   show_head(p, cond);
            show_branch_instruction(J_B + cond, RETLAB);
            return (next == way_out) ? RETLAB : next;
        }
#ifdef TARGET_HAS_SCCK
/* The following code checks for two blocks which each have a single    */
/* MOVK to the same register and the same successor block.              */
/* Possible wish list: 1. SCCR to set r1 to 0 or r3, or do this by      */
/* (SCCK -1; AND) if r1!=r3.  2. A macro for target_SCCK_able(n)?.      */
        if (block_single_entryexit(b) && block_single_entryexit(b1) &&
              blknext_(b) == blknext_(b1) &&
              blklength_(b) == 1 && blklength_(b1) == 1)
        {   Icode *c = blkcode_(b), *c1 = blkcode_(b1);
/* The next test: really remove_noops() should remove J_DEADBITS for    */
/* loopopt vars which are removed.  Also note that equality is that of  */
/* virtregs, not real regs, but this is OK since both vregs are live.   */
            if ((c->op & ~J_DEAD_R2) == J_MOVK &&
                (c1->op & ~J_DEAD_R2) == J_MOVK && c->r1.r == c1->r1.r)
            {   if (c->m.i == 0)
                {   VRegInt vr2; vr2.r = GAP;
                    show_head(p, cond);
                    expand_jop_macro(J_SCCK+cond, c->r1, vr2, c1->m);
                }
                else if (c1->m.i == 0)
                {   VRegInt vr2; vr2.r = GAP;
                    show_head(p, Q_NEGATE(cond));
                    expand_jop_macro(J_SCCK+Q_NEGATE(cond), c->r1, vr2, c->m);
                }
/* This hits cases like x?4:5. It probably saves space (BC;MOVK;B;MOVK) */
/* vs (SCC;ADDK) and often time too.                                    */
                else
                {   VRegInt vr2, vm; vr2.r = GAP; vm.i = c1->m.i - c->m.i;
                    show_head(p, cond);
                    expand_jop_macro(J_SCCK+cond, c->r1, vr2, vm);
                    expand_jop_macro(J_ADDK, c->r1, c->r1, c->m);
                }
/* The next line is unfortunate: we have to do this to avoid the        */
/* block getting spuriously displayed again later.  It therefore        */
/* requires the block_single_entryexit() condition.                     */
                blkflags_(b) |= BLKCODED, blkflags_(b1) |= BLKCODED;
                return blknext_(b) == way_out ? RETLAB : blknext_(b);
            }
/* Other cases worth searching for?                                     */
        }
#endif
        /*
         * Hack by RCC 24/02/88.  If both the destinations are not yet coded,
         * and one of them is the block we would code next, it can pay to do
         * the unconditional branch to the earlier one.
         *
         * There ought to be a quicker way to find out which of the
         * destinations is likely to be coded first.
         */
        {   BlockHead *x = b; /* unconditional branch destination */
            do {
                x = blkdown_(x);
                if (x == NULL) /* conditional dest is after uncond dest: swap */
                {   show_head(p, Q_NEGATE(cond));
                    show_n(ncommon, commonp, s);
                    show_branch_instruction(J_B + Q_NEGATE(cond), next);
                    return (is_exit_label(next1)) ? RETLAB : next1;
                }
            } while (x != b1);
        }

/* Here we drop through to the ordinary simple case                      */
        show_head(p, cond);
        show_n(ncommon, commonp, s);
        show_branch_instruction(J_B + cond, next1);
        return next;
    }
}
#endif /* TARGET_IS_NULL */

static void refblock(LabelNumber *ll, BlockHead *from, BlockList *reuse)
{
    if (!is_exit_label(ll))
    {   BlockList *bl;
        procflags |= blkflags_(ll->block);
/* I make, in the blkusedfrom_() entry in b, a list of all blocks that   */
/* reference it. This list will be used when cross-jump optimizing.      */
/* NB the top block gets a NULL in its used-from list. I will only allow */
/* an ancestor to appear once in blkusedfrom_() even if there are many   */
/* routes (e.g. because of switchon tables) from one block to another.   */
        for (bl = blkusedfrom_(ll->block); bl != NULL; bl = bl->blklstcdr)
            if (bl->blklstcar == from) return;
        if (reuse == NULL) reuse = (BlockList*) BindAlloc(sizeof(BlockList));
        reuse->blklstcar = from;
        reuse->blklstcdr = blkusedfrom_(ll->block);
        blkusedfrom_(ll->block) = reuse;
    }
/*     else syserr("refblock");  really happens  -- why? */
}

#ifndef TARGET_IS_NULL

static BlockList *unrefblock(BlockHead *ll, BlockHead *from)
/* At present block ll has from recorded as one of its ancestors - kill this */
/* Return the dead cell for refblock().                                      */
{
    BlockList **lvbl = &(blkusedfrom_(ll));
    BlockList *bl;
    while ((bl = *lvbl) != NULL)
        if (bl->blklstcar == from)
        {   *lvbl = bl->blklstcdr;
            return bl;
        }
        else lvbl = &(bl->blklstcdr);
    syserr(syserr_unrefblock);
    return 0;
}

#endif

static int32 remove_noops(Icode *c, int32 len)
{ int32 i;
  /* besides normalising noops to J_NOOP, the following code also turns */
  /* J_SETSPGOTO into J_SETSP - essentially a 2nd pass for goto/labels. */
  /* It also turns the addresses of locals to 'stack' form from         */
  /* environment form, thus completing auto and register allocation.    */
  /* Ultimately we should remove all _STACKREF jopcodes here too. ???   */
  /* J_NOOP's are now completely removed.                               */

  for (i=0;i<len;i++)
  { J_OPCODE op = c[i].op;
    if (uses_stack(op))
    {   Binder *bb = c[i].m.b;
        if (bindstg_(bb) & b_bindaddrlist)
        {   /* real stack variable -- change scope to stack offset */
            bindstg_(bb) &= ~b_bindaddrlist,
            bb->bindaddr.i = BINDADDR_LOC |
                             size_of_binders(bb->bindaddr.bl);
        }
#ifdef TARGET_STACK_MOVES_ONCE
        if ((bindaddr_(bb) & BINDADDR_MASK) == BINDADDR_NEWARG)
        {   /* Convert an 'actual in new frame' address into a local.   */
            /* AM, Nov89: this code is somewhat in flux.                */
            unsigned32 m = bindaddr_(bb) & ~BINDADDR_MASK;
/* @@@ see health warning in expand_jop_macro() if NARGREGS != 0.       */
            if (m < 4*NARGREGS)
                cc_warn(warn_untrustable, currentfunction.symstr); /* syserr() */
            bindaddr_(bb) = BINDADDR_LOC |
                                  (greatest_stackdepth - (m - 4*NARGREGS));
        }
#endif
    }
#define smashop(o,x,y,z) (c[i].op=(o), c[i].r1=(x), \
                          c[i].r2=(y), c[i].m=(z))
#define smashMOV(o,x,z) (c[i].op=(o), c[i].r1=(x), \
                         c[i].r2.r=GAP, c[i].m=(z))
    switch (op & ~(Q_MASK|J_DEADBITS)) /* crucial to ignore J_DEADBITS here */
    {
case J_OPSYSK:
case J_CALLK:   /* The 'call can be optimised out' info has now passed its
                   'use by' date */

case J_CALLR: c[i].r2.i &= ~K_PURE; break;

case J_SETSPGOTO:
        c[i].r2.i = size_of_binders(c[i].r2.bl);
        { /* The block this branch is to may have disappeared (because it was
           * empty).  If so, the scan to turn bind lists to frame sizes won't
           * have processed it.  We COULD look for SETSPGOTOs in branch_chain,
           * but that seems like overkill.  Instead, blocks which have had
           * their bind lists converted to a frame size are marked.
           */
            BlockHead *b = (c[i].m.l)->block;
            c[i].m.i = (blkflags_(b) & BLKSTACKI) ?
                          blkstacki_(b) :
                          size_of_binders(blkstack_(b));
        }
        c[i].op = J_SETSP;
        i--; break;                                         /* retry */
case J_SETSPENV:
        c[i].r2.i = size_of_binders(c[i].r2.bl);
        c[i].m.i = size_of_binders(c[i].m.bl);
        c[i].op = J_SETSP;
        i--; break;                                         /* retry */
case J_SETSP:
        if (c[i].r2.i == c[i].m.i) c[i].op = J_NOOP;
        break;
case J_LDRV: case J_LDRFV: case J_LDRDV:
case J_STRV: case J_STRFV: case J_STRDV:
        {   VRegnum r1 = c[i].r1.r;
            Binder *bb = c[i].m.b;
            VRegnum r3 = bindxx_(bb);
/* N.B. what happens if r1 does not get a real reg?  LDRV1 has this case!  */
            if (r3 != GAP && isany_realreg_(register_number(r3)))
            {   VRegInt vr1, vr3;
                vr1.r = r1;
                vr3.r = r3;
                if (register_number(r1) == register_number(r3))
                    c[i].op = J_NOOP;
                else if (loads_r1(op))
/* @@@ "op & ~J_DEADBITS" was innocent effect of old code.  Check?!!    */
                    smashMOV(J_XtoY(op&~J_DEADBITS, J_LDRV, J_MOVR),
                             vr1, vr3);
                    /* LDRxV never has J_DEAD_R3. (perhaps it should?) */
                else
/* @@@ "op & ~J_DEADBITS" was innocent effect of old code.  Check?!!    */
                    smashMOV(J_XtoY(op&~J_DEADBITS, J_STRV, J_MOVR)
                               | (op & J_DEAD_R1 ? J_DEAD_R3 : 0),
                             vr3, vr1);
                    /* but STRxV may have J_DEAD_R1, which we need to take care
                       not to discard in turning it into a MOV.
                     */
            }
            else bindxx_(bb) = GAP;
        }
        break;
case J_LDRV1:
case J_LDRFV1:
case J_LDRDV1:
        /* as LDRV but always uses the stack value. Used at head of fn   */
        /* Should only be used to load from +ve offset from FP.          */
        /* Except that leaf procedures may not have set up FP!           */
        {   VRegnum r1 = c[i].r1.r;
            if (!isany_realreg_(register_number(r1)))
                c[i].op = J_NOOP;  /* Ignore unless this var got a real reg */
        }
        break;
case J_CMPK:                       /* I.e. the pseudo_uses_r1() ops      */
/* Note that if the r1 field holds a real register then the opcode       */
/* should be treated as a comparison against that.                       */
        {   VRegnum r1 = c[i].r1.r;
            VRegnum r2 = c[i].r2.r;
            /* Fix up deadbits (WGD) */
            J_OPCODE newop = (op&~(J_DEAD_R1|J_DEAD_R3)) ^ (J_CMPK ^ J_CMPR);
            if (op&J_DEAD_R1) newop |= J_DEAD_R3;
            if (r1!=GAP && isint_realreg_(register_number(r1)))
            {   VRegInt vr1, vr2, vm;
                vr1.r = GAP;
                vr2.r = r2;
                vm.r = r1;
                smashop(newop, vr1, vr2, vm);
            }
            else c[i].r1.r = GAP;/* normalise for cross jumping */
        }
        break;
case J_STRING:
        if (feature & FEATURE_WR_STR_LITS)
            procflags |= PROC_USESADCONS;
        break;
/* the next three cases deal with lifting adcons/constants out of loops */
case J_ADCON:
        if (!(bindstg_(c[i].m.b) & u_constdata)) procflags |= PROC_USESADCONS;
        /* and fall through */
case J_ADCONV:
case J_MOVK:                       /* I.e. the pseudo_uses_r2() ops      */
/* r1 may still be a virtual register (corresponding to an optimisation  */
/* that has been spilled because of lack of real registers). In that     */
/* case I can just ignore this instruction.                              */
        {   VRegnum r1 = c[i].r1.r;
            if (isint_realreg_(register_number(r1)))
            {    VRegnum r2 = c[i].r2.r;
                 J_OPCODE newop = (op&J_DEAD_R2) ? J_MOVR|J_DEAD_R3 : J_MOVR;
/* The following funny treatment of MOVK helps loop optimisation.        */
/* If the r2 field is a real register (neither GAP nor an unallocated    */
/* virtual register) I can turn this into a MOVR r1,r2 instruction.      */
/* AM: (because loopopt.c has inserted a real register)                  */
                if (r2!=GAP && isint_realreg_(register_number(r2)))
                {   if (register_number(r1) != register_number(r2))
                    {   VRegInt vr1, vr2;
                        vr1.r = r1;
                        vr2.r = r2;
                        smashMOV(newop, vr1, vr2);
                    }
                    else c[i].op = J_NOOP;
                }
                else c[i].r2.r = GAP;/* normalise for cross jumping */
            }
            else c[i].op = J_NOOP;
        }
        break;
/* AM: there seems no rationale behind which insts have the realreg test! */
case J_MOVR:
        /* optimize away moves from a register to itself.                */
        /* Also lose moves into virtual registers (which may be a side-  */
        /* effect of the optimiser trying to allocate local vars to regs */
        if (!isint_realreg_(register_number(c[i].r1.r)) ||
            register_number(c[i].r1.r)==register_number(c[i].m.r))
                c[i].op = J_NOOP;
        break;
case J_MOVDR:
case J_MOVFR:
#ifdef TARGET_IS_ARM                         /* see regalloc.c, armgen.c */
case J_MOVFDR:                               /* see regalloc.c, armgen.c */
#endif
        if (!isflt_realreg_(register_number(c[i].r1.r)) ||
            register_number(c[i].r1.r)==register_number((VRegnum)c[i].m.r))
                c[i].op = J_NOOP;
        break;
case J_MOVIFR:
case J_MOVIDR:
        if (!isflt_realreg_(register_number(c[i].r1.r)))
	  c[i].op = J_NOOP;
        break;
case J_INIT:
case J_INITF:
case J_INITD:
/* These will be left if there is a potentially uninitialized variable   */
case J_NOOP:
/* Flatten condition mask if there was one */
#ifndef TARGET_GEN_NEEDS_VOLATILE_INFO
case J_USE:
case J_USEF:
case J_USED:
case J_VSTORE:/* @@@ AM thinks VSTORE is a bit in a JOP store, not an opcode */
#endif
/* These help ensure that references to volatile things are not          */
/* optimized out of existence                                            */
/* The local code generator needs them for similar reasons, so not       */
/* optimised out                                                         */
        c[i].op = J_NOOP;
        break;
default:
        break;
    }
#undef smashop
#undef smashMOV
  }
  { int32 n;
    for (n = i = 0; i < len; i++)
/*
 * The next line exhibits a degree of extreme (unreasonable) caution - it
 * tests i!=n to prevent a structure assignment between overlapping structs
 * even in the case thet the overlap is total.  ANSI would permit the test
 * to be missed out but we are super-conservative!
 */
        if (c[i].op != J_NOOP)
        {   if (i != n) c[n] = c[i];
            n++;
        }
    return n;
  }
}

#ifndef TARGET_IS_NULL

/* Cross jumping optimisation routines:  note that cross jumping is here   */
/* implemented as a rooted (in way_out) tree isomorphism problem.          */
/* The tree is the backward chains of blocks in blkusedfrom_().            */
/* Soon change this to treat all J_TAILCALLK's (BLK0EXIT) as roots too.    */
/* (Or do TAILCALL after cross_jump.)                                      */
/* This means that (inter alia) it can never identify loops.               */
/* The full graph isomorphism problem is much more expensive and probably  */
/* does not gain significantly more in real code.  We do not intend to     */
/* change this implementation in the near future!                          */
/* Note that handling conditional exits (i.e. DAG isomorphism) is          */
/* probably not much harder -- see comments below "only merge if..."       */

static void identify_blocks(BlockHead *x1, BlockHead *x2, BlockHead *q)
{
/* blocks x1 and x2 have identical contents - redirect all references to   */
/* x2 so that they point to x1.  q is the only successor of both x1 and x2 */
/* x2 is not the top block (i.e. the place where the procedure will be     */
/* entered), and so that is all right, isn't it.                           */
    BlockList *x2src = blkusedfrom_(x2);
    LabelNumber *l1 = blklab_(x1), *l2 = blklab_(x2);
    /* first delete x2 from the ancestors of q and kill x2 then swing the  */
    /* chain of ancestors of x2 to x1.                                     */
    unrefblock(q, x2);
    blkflags_(x2) &= ~BLKALIVE;
    blkusedfrom_(x2) = (BlockList*) DUFF_ADDR; /* checks! */
    blkcode_(x2) = (Icode*) DUFF_ADDR;         /* checks! */
    while (x2src != NULL)
    {   BlockList *rplac = x2src;
        BlockHead *p = x2src->blklstcar;
        x2src = x2src->blklstcdr;
        refblock(l1, p, rplac);           /* re-use cell rplac */
        if (blkflags_(p) & BLKSWITCH)
        {   int32 i;
            LabelNumber **tab = blktable_(p);
            for (i = 0; i < blktabsize_(p); i++)
                if (tab[i] == l2) tab[i] = l1;
        }
        else
        {   /* AM: blknext_() is only valid if !BLK0EXIT & !BLKSWITCH   */
            /* NB blkflags_(p) cannot have BLK0EXIT and be an ancestor! */
            if (blknext_(p) == l2)
                blknext_(p) = l1; /* redirect main exit */
            if (blkflags_(p) & BLK2EXIT) {
                if (blknext1_(p) == l2) blknext1_(p) = l1;
                if (blknext_(p) == blknext1_(p)) {
                    blkflags_(p) &= ~(BLK2EXIT|Q_MASK);
                    if (!(blkflags_(p) & BLKCCEXPORTED)) blklength_(p)--;
                }
            }
        }
    }
}

static void truncate_block(BlockHead *x2, int32 i2, BlockHead *x1, BlockHead *p)
{
/* Here block x2 must be truncated to have new length i2, and its exit     */
/* must be rewritten as going to x1.                                       */
/* Due to the calls to truncate_block, we know that x2 has a single        */
/* successor (p) and so we only have to swing one pointer x2-->p to x1-->p */
/* However, note that p may have multiple ancestors.                       */
    blklength_(x2) = i2;
    blknext_(x2) = blklab_(x1);
    refblock(blklab_(x1), x2, unrefblock(p, x2));
}

static void zip_blocks(BlockHead *x1, int32 i1, BlockHead *x2, int32 i2, BlockHead *p)
{
    BlockHead *newbh;
    LabelNumber *lab = nextlabel();
    lab->block = newbh = newblock(lab, NULL);
    blkstacki_(newbh) = adjusted_stack_size(blkstacki_(x1), blkcode_(x1), i1);
    blklength_(newbh) = blklength_(x1) - i1;
    blkcode_(newbh) = &(blkcode_(x1)[i1]);
    blknext_(newbh) = blklab_(p);
    blkdebenv_(newbh) = blkdebenv_(x1);    /* maybe */
/* The next line is a slight lie -- e.g. not BLKCALL but does not matter */
    blkflags_(newbh) = BLKALIVE;
    blklength_(x1) = i1;
    blknext_(x1) = lab;
    blklength_(x2) = i2;
    blknext_(x2) = lab;
    refblock(lab, x1, unrefblock(p, x1));
    refblock(lab, x2, unrefblock(p, x2));
    refblock(blklab_(p), newbh, 0);
}

static void cross_jump_optimize(void)
{
    BlockHead *p;
    bool another_pass_needed;
    if (!crossjump_enabled || usrdbg(DBG_LINE+DBG_VAR)) return;
    do
    {   another_pass_needed = NO;
        for (p = top_block; p != NULL; p = blkdown_(p))
          if (blkflags_(p) & BLKALIVE)
          { BlockList *predecessors;
            BlockList *p1, *p2;
    restart_scanning_block_p:
            predecessors = blkusedfrom_(p);
            for (p1 = predecessors; p1!=NULL; p1 = p1->blklstcdr)
              for (p2 = p1->blklstcdr; p2!=NULL; p2 = p2->blklstcdr)
              { BlockHead *x1 = p1->blklstcar,
                           *x2 = p2->blklstcar;
                if (x1 != x2 &&     /* probably never happens, but be careful */
                    x1 != NULL &&   /* null block has nothing in common ... */
                    x2 != NULL &&   /* ... with anything */
                                    /* (null block is ancestor of top_block) */
                    /* only merge if parents' exits were simple */
                    (blkflags_(x1) & (BLKSWITCH | BLK2EXIT)) == 0 &&
                    (blkflags_(x2) & (BLKSWITCH | BLK2EXIT)) == 0)
                {   int32 ncommon = 0;
                    int32 i1 = blklength_(x1), i2 = blklength_(x2);
                    Icode *c1 = blkcode_(x1), *c2 = blkcode_(x2);
    /* Here I scan two ancestors of p from the tail-end looking for      */
    /* equivalent instructions. I only do this if the ancestors both     */
    /* have just simple exits (no branch tables or conditional jumps).   */
    /* J_NOOP's have already been removed.                               */
                    for (; i1 > 0 && i2 > 0; i1--, i2--, ncommon++)
                        if (!same_instruction(&c1[i1-1], &c2[i2-1])) break;

                    if (ncommon != 0)
                    {
                        if (i1 == 0)
                        {   if (i2 == 0)
/* At present the top block can not be an exact match for any other block  */
/* because it (and it alone) as an ENTER opcode in it. However I act with  */
/* caution here in case things change in the future.                       */
                            {   if (x2 != top_block)
                                     identify_blocks(x1, x2, p);
                                else identify_blocks(x2, x1, p);
                                another_pass_needed = YES;
                            }
                            else
                            {   truncate_block(x2, i2, x1, p);
                                another_pass_needed = YES;
                            }
                        }
                        else if (i2 == 0)
                        {   truncate_block(x1, i1, x2, p);
                            another_pass_needed = YES;
                        }
                        else
                        {   zip_blocks(x1, i1, x2, i2, p);
                            another_pass_needed = YES;  /* ????? needed in allcases? */
                        }
/* The above procedures may well alter the chaining in blkusedfrom_(p),    */
/* the ancestor list for p, so to                                          */
/* keep things as simple as I can (albeit with some loss of speed, but     */
/* that only when optimisations are actually being found), I leap right    */
/* out from the middle of a load of nested loops here and start scanning   */
/* the ancestors of p all over again. I think that pretty well any other   */
/* construction here will really be a stylistically ugly as the goto that  */
/* I use - maybe I could demand that identify_blocks() etc guaranteed not  */
/* to have bad interactions with the iteration over ancestor-pairs, but    */
/* that seems like the wilfull design of fragile code.                     */
                        goto restart_scanning_block_p;
                    }
                }
              }
          }
    } while (another_pass_needed);
}

#endif /* TARGET_IS_NULL */

static void kill_unreachable_blocks(void)
{
    BlockHead *b = top_block;
    blkflags_(b) &= ~BLKALIVE;  /* top_block MUST have been alive */
    while (b != bottom_block)
    {   BlockHead *next = blkdown_(b);
        int32 ff = blkflags_(next);     /* only bottom_block has empty down */
        if (ff & BLKALIVE)
        {   blkflags_(next) = ff & ~BLKALIVE;
            b = next;
        }
        else if (usrdbg(DBG_LINE+DBG_VAR))
            b = next;
        else if (next == bottom_block)  /* bottom_block was unreachable */
        {   blkdown_(b) = NULL;
            bottom_block = b;
        }
        else                            /* patch out a dead block */
        {   next = blkdown_(next);
            blkdown_(b) = next;
            blkup_(next) = b;
        }
    }
}

/* exported ... */

void lose_dead_code(void)
{   /* exported to cg.c to call before register allocation   */
/* cf branch_chain in flowgraf.c, which will be needed later */
/* AM: think about amalgamating this with branch chain()     */
    (void)branch_chain(blklab_(top_block), NO);
    kill_unreachable_blocks(); /* (just to unset BLKALIVE if usrdbg(..)) */
}

void linearize_code(void)
{
/* before we can tidy up branches to branches we have to find out which  */
/* basic blocks are empty (this is non-trivial because e.g. register     */
/* allocation may have turned a STRV into a MOV r,r).  Ultimately we     */
/* should remove all _STACKREF jopcodes, but for now just test by        */
/* smashing to NOOPs where relevant.                                     */
    {   BlockHead *p;
        /* Now seems a convenient time to replace blkstack by blkstacki */
        for (p = top_block; p != NULL; p = blkdown_(p)) {
#ifdef TARGET_HAS_DEBUGGER /* XXX - NC */
            if (usrdbg(DBG_VAR)) {
                BindList *bl = blkstack_(p);
                for ( ; bl!=NULL ; bl = bl->bindlistcdr ) {
                    Binder *b = bl->bindlistcar;
                    if (bindstg_(b) & b_bindaddrlist)
                    {   /* The following code extends similar code in   */
                        /* remove_noops by ensuring all variables (not  */
                        /* only referenced ones) are allocated stack    */
                        /* when the debugging tables are produced.      */
                        bindstg_(b) &= ~b_bindaddrlist,
                        b->bindaddr.i = BINDADDR_LOC |
                                        size_of_binders(b->bindaddr.bl);
                    }
                }
            }
#endif /* TARGET_HAS_DEBUGGER */
            blkstacki_(p) = size_of_binders(blkstack_(p));
            blkflags_(p) |= BLKSTACKI;
        }
        for (p = top_block; p != NULL; p = blkdown_(p))
                /* NB lose_dead_code() means that all these blocks are   */
                /* really alive, however BLKALIVE has been cleared.      */
            blklength_(p) = remove_noops(blkcode_(p), blklength_(p));
    }
/* Remove branches to branches and similar oddities.                     */
/* N.B. the top_block will never be empty since it contains an ENTER     */
/* Also collect list of parents for each block.                          */
/* Furthermore turn exit-branches into jumps to a single exit block so   */
/* that cross-jumping will be able to detect shared code leading thereto */
    {   BlockHead *b;
        way_out = nextlabel();
        way_out->block = b = newblock(way_out, NULL);
        blkstacki_(b) = 0;
        blknext_(b) = RetVoidLab;  /* Hmmm - all returns are alike here */
        blkflags_(b) |= BLKALIVE;
    }
    (void)branch_chain(blklab_(top_block), YES);

#ifdef TARGET_HAS_TAILCALL
/* Now (after branch-chaining) insert J_TAILCALL instead of J_CALL ...   */
/* This transformation is valid if the CALL is immediately followed by a */
/* return, and no variables whose address are taken are in scope and     */
/* further no calls to 'setjmp' occur within the procedure at all.       */
/* (CONFIG_INDIRECT_SETJMP allows (ANSI-forbidden) J_CALLR to 'setjmp'.) */
/* This is implemented in rather a different way.  We can tell that no   */
/* non-register locals are present from the immediate return since       */
/* this can at worst have come from a squashed SETSP.  We need to check  */
/* whether any block had a 'setjmp' or an arg address taken too.         */

/* sample code that could give trouble if setjmp were called & tail-call */
/* conversion applied...                                                 */
/*   int f(int x)                                                        */
/*   {   <code that is complicated enough for x to be spilt>             */
/*       if (setjmp(buff)) p(buff);                                      */
/*       else q(x);                                                      */
/*   }                                                                   */
/* where p calls longjmp(). Then if x is in the stack-frame of f() the   */
/* call to p() can clobber it even though its value is needed for q(x).  */
/* Note that if x were a register variable its value would have been safe*/
/* in buff, and if &x were mentioned the tail call would be illegal      */
/* otherwise, so it is only if x is spilt for other reasons....          */

/* Note that the destination in a J_TAILCALLR ought not to be one in the */
/* callee-saved set (which will be restored before it is used), but      */
/* register allocation saw an ordinary J_CALLR with no such restriction. */
/* The problem is ignored here: backends with TAILCALLR must take        */
/* appropriate action themselves.                                        */

    if (!(procflags & (BLKSETJMP|PROC_ARGADDR))
#ifdef TARGET_IS_ACW       /* WGD 19-10-87 */
        && !assembler_call
#endif
#ifdef TARGET_IS_C40
	&& !backtrace_enabled	/* because we do not want to loose stack frames */
#endif
        && !usrdbg(DBG_ANY) /* No tail call if debugging - too confusing */
        && !(var_cc_private_flags & 16L)
        && !(procauxflags & bitoffnaux_(s_irq))
       )
    {   BlockHead *p; int32 len;
        for (p = top_block; p != NULL; p = blkdown_(p))
            if (!(blkflags_(p) & (BLK2EXIT|BLKSWITCH)) &&
                blknext_(p) == way_out &&
                (len = blklength_(p)) > 0)
            {   Icode *b = blkcode_(p);
                Icode *bend = &b[len-1];
/* AM: note that we do not tailify() J_CALLK/R  with K_VACALL.          */
                if (!(bend->r2.i & K_VACALL) &&
                    ( (bend->op & ~J_DEADBITS) == J_CALLK
#ifdef TARGET_HAS_TAILCALLR
                     || (b[len-1].op & ~J_DEADBITS)== J_CALLR
#endif
                    ) )
                {   bend->op = tailify_(bend->op); /* J_TAILCALLK/R */
                    /* flag block as having no exit and 1 less call ...   */
                    blkflags_(p) |= BLK0EXIT;
                    if (!(blkflags_(p) & BLK2CALL)) blkflags_(p) &= ~BLKCALL;
                }
            }
    }
#endif

/* Now establish reference counts for blocks so that I can see which     */
/* ones have more than one entry.                                        */
/* @@@ move this code into branch_chain() now that it is tidied?         */
    {   BlockHead *p;
        refblock(blklab_(top_block), NULL, 0);  /* Reference from outside proc  */
        for (p = top_block; p != NULL; p = blkdown_(p))
            if (blkflags_(p) & BLKALIVE)
            {   if (blkflags_(p) & BLKSWITCH)
                {   LabelNumber **v = blktable_(p);
                    int32 i, n = blktabsize_(p);
                    for (i=0; i<n; i++) refblock(v[i], p, 0);
                }
                else
                {   if (!(blkflags_(p) & BLK0EXIT)) refblock(blknext_(p), p, 0);
                    if (blkflags_(p) & BLK2EXIT) refblock(blknext1_(p), p, 0);
                }
            }
    }

    blkflags_(top_block) |= BLKALIVE;

#ifndef TARGET_IS_NULL
    cross_jump_optimize();                   /* uses used-from information */

#ifndef TARGET_STACKS_LINK
/* See comment in regalloc.c - mark R_LR as allocated if a non-tail call   */
/* appears.                                                                */
    if (procflags & BLKCALL) augment_RealRegSet(&regmaskvec, R_LR);
    /*
     * XXX - NC
     *
     * NB/ we do NOT add R_LR to the usedmaskvec because
     * this array only covers registers USED by the function,
     * NOT corrupted by the function.  In particular we want to
     * know if we can use R_LR as a back end temporary variable
     * (because it gets saved on the stack at function entry,
     * but then not used by the register allocator).
     */
#endif

/* Now dump out the flowgraph - note that J_ENTER implicitly               */
/* does a 'set_cond_execution(Q_AL)' if TARGET_HAS_COND_EXEC               */

/* ???Is doing the next line here what causes the syserr() in refblock()   */
    blkflags_(way_out->block) |= BLKCODED;    /* do not display this block */
/* strictly a lie, but harmless.  The more obvious &= ~BLKALIVE fails if   */
/* usrdbg(something)                                                       */

/* AM: The following interface is spurious: xxxgen.c can detect         */
/* the J_ENTER.  AM leaves it (under threat) until we can inhibit the      */
/* J_LABEL and J_STACK which preceed J_ENTER currently.                    */
    localcg_reinit();

  { LabelNumber *pending_branch_address = NOTALAB;
    BlockHead *p1;
    current_env2 = 0;
#ifndef TARGET_IS_ARM
    dbg_enterproc();
#else
    /* called from armgen (possibly after codeseg_function_name()) */
#endif
    if (debugging(DEBUG_CG))
        cc_msg("\n\nFlattened form:\n\n");

    for (p1=top_block; p1!=NULL; p1=blkdown_(p1))
    { BlockHead *p = p1;
      if ( ( (blkflags_(p) & BLKALIVE) || usrdbg(DBG_LINE+DBG_VAR) ) &&
           !(blkflags_(p) & BLKCODED))
        for (;;)
        { LabelNumber *w = blklab_(p);
          if ( (blkflags_(p) & BLKEMPTY) && !usrdbg(DBG_LINE+DBG_VAR) )
            syserr(syserr_live_empty_block, (long)lab_name_(w));
          if (pending_branch_address!=NOTALAB && pending_branch_address!=w)
            show_branch_instruction(J_B, pending_branch_address);
          if (blkflags_(p) & BLKSWITCH)
          {   LabelNumber **v = blktable_(p);
              int32 i, n = blktabsize_(p);
              /* Normalise the entries in the switch table */
              /* This is done so that the r2 field of J_CASEBRANCH is OK */
              for (i=0; i<n; i++)
                if (v[i] == way_out) v[i] = RETLAB;
          }
          if (blkflags_(p) & BLK2EXIT && blknext_(p) != blknext1_(p))
/* The seemingly odd test for distinct destinations is present here since  */
/* cross-jump optimisations can reveal new cases of vacuous tests, and if  */
/* I pass such a case down to use_cond_field() the destination block gets  */
/* displayed twice (which would be sort of OK except for the fact that     */
/* setting its label twice causes trouble).                                */
            pending_branch_address = use_cond_field(p);
          else
          {
            show_basic_block(p, Q_AL);
            if (blkflags_(p) & BLKSWITCH)
            { LabelNumber **v = blktable_(p);
              int32 i, n = blktabsize_(p);
/* drop though to execute last case directly: only if target uses a branch */
/* table of branch instructions!!!!!!!!!                                   */
#ifdef TARGET_HAS_SWITCH_BRANCHTABLE
              for (i=0; i<n-1; i++)
                show_branch_instruction(J_BXX, v[i]);
              pending_branch_address = v[n-1];
#else
              for (i=0; i<n; i++)
                show_branch_instruction(J_BXX, v[i]);
              pending_branch_address = NOTALAB;
#endif
            }
#ifdef TARGET_HAS_TAILCALL
            else if (blkflags_(p) & BLK0EXIT)
              pending_branch_address = NOTALAB;
#endif
            else if ( blknext_(p) == way_out ||
                      (!(blkflags_(p) & BLKALIVE) &&
                       is_exit_label(blknext_(p))) )
              pending_branch_address = RETLAB;
            else
              pending_branch_address = blknext_(p);
          }
          if (pending_branch_address == NOTALAB ||
              pending_branch_address == RETLAB  ||
/* @@@ LDS 21-Sep-89: the following line conspires with cg_loop and emits  */
/* better code for while/for loops, saving 1 br-not-taken/iteration.       */
            (blkflags_(p) & BLKLOOP) && !usrdbg(DBG_LINE)) break;
          { BlockList *bl;
            int flag = 1;
            BlockHead *prev = p;
            p = pending_branch_address->block;
/* p is now the destination block for the one I have just displayed.       */
/* If it has not already been coded but all its ancestors have (except     */
/* itself, if it is its own ancestor) I will display it next.              */
            if (blkflags_(p) & BLKCODED || usrdbg(DBG_VAR)) break;
            for (bl = blkusedfrom_(p); bl!=NULL; bl=bl->blklstcdr)
            { if (bl->blklstcar != p &&
                  !(blkflags_(bl->blklstcar) & BLKCODED))
              { flag = 0;
                break;
              }
            }
            if (flag) continue; /* use block p next, do not advance p1 */
            if (p == blkdown_(prev)) continue;
          }
          break;                       /* go on to next sequential block */
        }
    }
    if (pending_branch_address != NOTALAB)
      show_branch_instruction(J_B, pending_branch_address);
    if (usrdbg(DBG_VAR)) dbg_scope1(0, current_env2);
  }
  { VRegInt w;
    w.r = GAP;
    expand_jop_macro(J_ENDPROC, w, w, w);
  }
#endif /* TARGET_IS_NULL */
}

void flowgraph_reinit(void)
{
    currentblock = icodetop = (Icode *) DUFF_ADDR;
                                        /* NB. (currentblock >= icodetop) */
    icode_cur = 0, block_cur = 0;
    icoden = 0;
    bottom_block = 0, block_header = (BlockHead*) DUFF_ADDR;
    deadcode = 1;
    current_env = 0;
    holes = NULL;
    prevblock = NULL;
    fg_pending.op = J_NOOP, fg_pending.m.i = 0;
}

/* end of mip/flowgraph.c */
