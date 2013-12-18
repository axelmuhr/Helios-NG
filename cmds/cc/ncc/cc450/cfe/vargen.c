/*
 * vargen.c: static initialised variable generator for C compiler
 * Copyright (C) Codemist Ltd, 1988-1992.
 * Copyright (C) Acorn Computers Ltd., 1988-1990.
 * Copyright (C) Advanced RISC Machines Limited, 1991-1992.
 */

/*
 * RCS $Revision: 1.2 $  Codemist 41
 * Checkin $Date: 1993/07/20 08:21:02 $
 * Revising $Author: nickc $
 */

/* AM memo: I do not see how genpointer gets called with a string arg       */
/*   for WR_STR_LITS since it s_string is filtered off by initsubstatic().  */
/*   Bug in WR_STR_LITS in ANSI mode?                                       */
/*   I think the two calls to genpointer need merging -- read ANSI carefully.*/
/* AM memo: see obj_symref @@@ below for more general use?                  */
/* Warning: the vg_copyobj/zeroobj probably depend on structs/arrays being  */
/*   aligned as the compiler currently does to 4 word boundaries.           */
/* AM memo: void values/vars need faulting earlier -- see initsubstatic()   */
/* WGD 28-9-87 ACW support added, 27-3-88 updated                           */
/* 30-Nov-86: vargen now assembles bytes/halfword data initialisations      */
/* into bytes and adds LIT_XXX flags for xxxasm.c, xxxobj.c                 */

/* Discussion:  Observe that generating code in full generality for
   arbitrary open arrays requires two passes - consider (e.g.)
   "char [][] = { "a", "bc",... }".  Moreover ANSI forbid such
   possibly useful initialisations.  So do we, but for reasons of
   producing (type and size) error messages as soon as possible rather
   than after a reading phase.  Accordingly we adopt a coroutine-like
   linkage to syn.c.
*/

#include "globals.h"
#include "vargen.h"
#include "lex.h"               /* for curlex */
#include "syn.h"
#include "sem.h"
#include "simplify.h"
#include "bind.h"
#include "builtin.h"
#include "aetree.h"
#include "codebuf.h"
#ifndef NON_CODEMIST_MIDDLE_END
#include "regalloc.h"  /* to handle global register variables */
#endif
#include "mcdep.h"     /* for dbg_xxx */
#include "store.h"
#include "aeops.h"
#include "util.h"      /* for padsize */
#include "xrefs.h"
#include "errors.h"

static Expr *rdinit(TypeExpr *t, int32 flag)
{   Expr *e = syn_rdinit(t,flag);
    return e ? optimise0(e) : 0;
}

/* @@@ check this routine takes care with padding for wide strings.    */
static int32 genstring(String *s, int32 size)
{   /* the efficiency of this is abysmal. */
    StringSegList *p = s->strseg;
    AEop sort = h0_(s);
    int32 length = stringlength(p)/(sort == s_wstring ? 4 : 1);
    if (size == 0xffffff)
        size = length + 1;
    else if (length > size)
        cc_rerr(vargen_err_long_string, sort==s_wstring ? "wchar_t":"char",
                (long)size);
    else if (length == size)
        cc_warn(vargen_warn_nonull, sort==s_wstring ? "wchar_t":"char",
                (long)size);
    vg_genstring(p, size*(sort == s_wstring ? 4:1), 0);
    return size;
}

static int32 int_of_init0(Expr *init, bool *ok)
{   int32 ival = 0;
    if (init != 0 && *ok)
    {   int32 op = h0_(init);
        if (op == s_integer)
            ival = intval_(init);
        else
        {
#ifndef PROPER_CODE_TO_SUPPORT_OFFSETOF_IN_STATIC_INITIALISERS
/* AM: It isn't just a case of being sensibly centralised, an attempt    */
/* was made to encourage sem.c only to reduce 'honest' constant exprs,   */
/* like 3.4*2, but not to do algebraic re-arrangement (which simplify.c  */
/* or cg.c are responsible for).  This is partly to enable that correct  */
/* diagnostics are produced (e.g. we do not want (x+0)=3 to become x=3). */
/* The offsetof rules changed and so I believe that sem.c should NOW     */
/* do a little more about these things.  Note the requirement to         */
/* diagnose "static int x = 1 || f();" comes in here!                    */
            /* @@@ LDS 27-Mar-90. The following makes offsetof() work in */
            /* static initializer contexts. Oh woe, expression reduction */
            /* isn't sensibly centralised. This must be regarded as an   */
            /* interim bodge of the worst sort, but I don't have time to */
            /* rstructure the tree-reduction part of the compiler.       */
            if (op == s_cast || op == s_plus || op == s_minus)
            {   Expr *a1, *a2;
                if (op == s_cast)
                    a1 = init, a2 = arg1_(init);
                else
                    a1 = arg1_(init), a2 = arg2_(init);
                if ((mcrepofexpr(a1) & ~0x01000000) == sizeof_int &&
                    (mcrepofexpr(a2) & ~0x01000000) == sizeof_int)
                {   ival = int_of_init0(a2, ok);
                    if (op == s_plus)
                        ival = int_of_init0(a1, ok) + ival;
                    if (op == s_minus)
                        ival = int_of_init0(a1, ok) - ival;
                    return ival;
                }
            }
#endif
            *ok = 0;
        }
    }
    return ival;
}

#ifdef CPLUSPLUS
static bool complain_non_aggregate;
static TypeExpr *inittype;
static int32 initbsize, initboffset;
static FileLine init_fl;
static void dynamic_init(Expr *e);
#endif

static int32 int_of_init(Expr *init)
{   bool ok = YES;
    int32 ival = int_of_init0(init, &ok);
    if (!ok)
    {
#ifdef CPLUSPLUS
        dynamic_init(init);
#else
        moan_nonconst(init, moan_static_int_type);
#endif
    }
    return ival;
}

static FloatCon *float_of_init(Expr *init)
{   FloatCon *fval = fc_zero.d;
    if (init != 0)
    {   if (h0_(init) == s_floatcon) fval = (FloatCon *)init;
        else
#ifdef CPLUSPLUS
            dynamic_init(init);
#else
            moan_nonconst(init, moan_floating_type);
#endif
    }
    return fval;
}

static int32 rd_bitinit(TypeExpr *t, int32 size)
{  t = unbitfield_type(t);
/* Apr 92: for type-safe enums, now use 't' instead of prev. te_int     */
/* The current version of unbitbields returns te_int/enum type.         */
    return int_of_init(rdinit(t, 0))  &  (((unsigned32)1 << size)-1);
/* One day it might be nice to compare the value read with size...      */
}

/**************************************************************************
      oo          oo     Problem: This code is tailored to putting string
      o\\  ____  //o     literals in the read only code area, thus it handles
        \\/    \//       initialised char *s without additional buffering and
         | O  O |        can generate the address constants on the fly. In
          : oo :         order to put the literals in the data area, we have
           \__/          to postpone generating them until after we've made
           //\\          the adcons pointing to them. Clever use of existing
         o//  \\o        data structures makes this almost painless... but
         oo    oo        BEWARE if you aspire to grok the code...
**************************************************************************/

    struct StrLit {
        struct StrLit *next;
        DataInit *d;
        String *s;
    };
    static struct StrLit *str_lits;

static bool init_adcons;

static void genpointer(Expr *x)
{   int32 offset = 0;
    AEop op;
    for (;;) switch (op = h0_(x))
    {   case s_addrof:
        {   Expr *y = arg1_(x); Binder *b;
            if (h0_(y) != s_binder)
                syserr(syserr_genpointer, (long)h0_(y));
            b = (Binder *)y;
            if ((b->bindstg & (bitofstg_(s_static) | u_loctype))
                 == bitofstg_(s_static))
            {   /* now statics may be local (and hence use datasegment
                 * offsets -- see initstaticvar()) or functions (possibly
                 * not yet defined -- "static f(), (*g)() = f;".
                 */
                if (b->bindstg & b_fnconst) gendcF(bindsym_(b), offset, 0);
                else {
                    gendcA(bindsym_(datasegment), bindaddr_(b)+offset, 0);
                }
            }
#ifdef TARGET_HAS_BSS
            else if ((bindstg_(b) & bitofstg_(s_static)) &&
                     bindaddr_(b) != BINDADDR_UNSET)
                gendcA((bindstg_(b) & u_constdata) ? bindsym_(constdatasegment) :
                                                     bindsym_(bsssegment),
                       bindaddr_(b)+offset, 0);
#endif
            else if (b->bindstg & (bitofstg_(s_extern) | u_loctype))
            {   int xr = (bindstg_(b) & bitofstg_(s_weak)) ? xr_weak : 0;
                if (b->bindstg & b_fnconst)
                    gendcF(bindsym_(b), offset, xr);
                else
                    gendcA(bindsym_(b), offset, xr);
            }
            else
            {   cc_err(vargen_err_nonstatic_addr, b);
                gendcI(sizeof_ptr, TARGET_NULL_BITPATTERN);
            }
            init_adcons = YES;
            return;
        }
        case_s_any_string
            if ((feature & FEATURE_WR_STR_LITS) ||
                (config & CONFIG_REENTRANT_CODE)) {
              /*
               * Standard pcc mode: first dump a reference to the
               * literal then record that the literal must be generated
               * and the reference updated when the recursion terminates.
               * I.e. the offset value stashed away in the next value
               *  will be sought out again and updated.
               */
              gendcA(bindsym_(datasegment), offset, 0);  /* dataseg relative */
              init_adcons = YES;
              str_lits = (struct StrLit *) syn_list3(
                                   (int32)str_lits,
                                   (int32)data.tail,
                                   (int32)((String *)x)
                                                    );
            } else {
              /*
               * Standard ANSI mode: put the strings in code space,
               * nonmodifiable. Rely on the fact that CG thinks it is
               * between routines (even for local static inits).
               * codeloc() is a routine temporarily for forward ref
               */
              gendcF(bindsym_(codesegment),codeloc()+offset, 0);
              codeseg_stringsegs(((String *)x)->strseg, NO);
            }
            return;
        case s_integer:
            /*
             * A New case to deal with things like '(&(struct tag*)0)->field'
             * Such expressions turn into '(int) + (int)' in the AE tree.
             * So here I calculate the value and assign it to the pointer.
             */
            /* @@@AM: new pointer reduction code in sem.c (for offsetof)   */
            /* *probably* means can never happen.  Review.                 */
            gendcI(sizeof_ptr, evaluate(x) + offset);
            return;
        case s_monplus:
        case s_cast:
            x = arg1_(x); break;
        case s_plus:
            if (h0_(arg1_(x)) == s_integer)
              { offset += evaluate(arg1_(x)); x = arg2_(x); break; }
            /* drop through */
        case s_minus:
            if (h0_(arg2_(x)) == s_integer)
              { offset += (op == s_plus ? evaluate(arg2_(x)) :
                                          -evaluate(arg2_(x)));
                x = arg1_(x); break; }
            /* drop through */
        default:
            /* I wonder if the type system allows the next error to occur! */
            /* ',' operator probably does */
#ifdef CPLUSPLUS
            dynamic_init(x);
#else
            cc_err(vargen_err_bad_ptr, op);
#endif
            gendcI(sizeof_ptr, TARGET_NULL_BITPATTERN);
            return;
    }
}

static void initbitfield(unsigned32 bfval, int32 bfsize, bool pad_to_int)
{
    int32 j;
    /* one day AM will extend this code to deal with long bit fields,   */
    /* e.g. a la mips.                                                  */
    if (!(feature & FEATURE_PCC))
    {   padstatic(alignof_int);
        gendcI(sizeof_int, bfval);
        return;
    }
    bfsize = (bfsize + 7) & ~7;
    if (debugging(DEBUG_DATA))
        cc_msg("initbitfield(%.8lx, %lu, %i)\n", bfval, bfsize, pad_to_int);
    for (j = 0;  j < bfsize;  j += 8)
        if (target_lsbytefirst)
            gendcI(1, bfval & 255), bfval >>= 8;
        else
            gendcI(1, bfval >> 24), bfval <<= 8;
    if (pad_to_int) padstatic(alignof_int);
}

/* NB. this MUST be kept in step with sizeoftype and findfield (q.v.) */
static void initsubstatic(TypeExpr *t)
{   SET_BITMAP m;
#ifdef CPLUSPLUS
    inittype = t;    /* for possible dynamic initialization */
#endif
    switch (h0_(t))
    {   case s_typespec:
            m = typespecmap_(t);
            switch (m & -m)    /* LSB - unsigned/long etc. are higher */
            {   case bitoftype_(s_void):
                    /*
                     * This Guy is trying to initalise a 'void' !!!
                     * @@@ Can this happen any more?
                     */
                    cc_err(vargen_err_init_void);
                    (void)int_of_init(rdinit(te_int,0));
                    break;
                case bitoftype_(s_char):
                case bitoftype_(s_enum):    /* maybe do something more later */
                case bitoftype_(s_int):
                    if (m & BITFIELD) syserr(syserr_initsubstatic);
                        /* these are all supposedly done as part of the
                         * enclosing struct or union.
                         */
#ifndef REVIEW_AND_REMOVE     /* in by default */
/*    This code to be reviewed now that the front end reduces some
      such expressions to integers.  AM and ??? to discuss.
      Do we need support for:   int x = (int)"abc";?
      Check the exact wording of last ANSI draft.
      If nothing else review the cc_err() message in genpointer.
*/
                    /*
                     * Okay, I confess, I have used the pointer initialisation
                     * code here to initialise 'int' values in PCC mode,
                     * But I can explain !!!.  Well, it goes like this, some
                     * PCC code contains expressions like :
                     *   'int x = (int) (&(struct tag*)0)->field;'
                     * Well, this is really a pointer initialisation so why
                     * not use the code.  (See simple, isn't it).
                     **** AM cringes at this.  Tidy soon.
                     *@@@ LDS too - BUT DON'T BREAK IT;
                     *              Unix won't compile if you do.
                     * AM dec90: this code probably is redundant since
                     *  changes elsewhere means it works in ansi mode.
                     *  but it does allow:  int x = (int)"abc";
                     */
                    if ((feature & FEATURE_PCC) && (sizeoftype(t) == 4))
                      { Expr *init = rdinit(t,0);
                        while (init && h0_(init) == s_cast) /* Remove casts */
                          { init = arg1_(init); }
                        if (init == 0) gendcI(4,0);
                        else if (h0_(init) == s_integer)
                            gendcI(4,intval_(init));
                        else genpointer(init);
                      }
                    else
#endif /* REVIEW_AND_REMOVE */
                      { gendcI(sizeoftype(t), int_of_init(rdinit(t,0))); }
                    break;
                case bitoftype_(s_double):
                    gendcE(sizeoftype(t), float_of_init(rdinit(t,0)));
                    break;
                case bitoftype_(s_struct):
                case bitoftype_(s_class):
                case bitoftype_(s_union):
/* ANSI 3rd public review draft says that                            */
/*   union { int a[2][2] a, ... } x = {{1,2}} initialises like       */
/*   int a[2][2] = {1,2}   ( = {{1,0},{2,0}}).                       */
                {   int32 note = syn_begin_agg();  /* skips and notes if '{' */
                    TagBinder *b = typespectagbind_(t);
                    ClassMember *l;
                    int32 bfsize, bfval, k, woffset;
                    bool is_union = ((m & -m) == bitoftype_(s_union));
                    StructPos p;
                    p.n = p.bitoff = 0;
                    if (!(attributes_(b) & TB_DEFD))
                        cc_err(vargen_err_undefined_struct, b);
                    bfsize = bfval = woffset = 0;
                    for (l = tagbindmems_(b); l != 0; l = memcdr_(l))
                    {
                        if (!structfield(l, bitoftype_(s_struct), &p)) continue;
#ifdef CPLUSPLUS
                        inittype = l->memtype;
                        initbsize = p.bsize;
                        initboffset = p.boffset;
#endif
                        if (isbitfield_type(l->memtype))
                        {   k = p.bsize;
                            if (bfsize == 0)
                            {   while (woffset < p.woffset)
                                {   gendcI(1, 0);
                                    ++woffset;
                                }
                            }
                            if (woffset < p.woffset)
                            {   initbitfield(bfval, bfsize, 1);
                                bfsize = bfval = 0;
                                woffset = p.woffset;
                            }
                            /* accumulate bitfield in bfval */
                            if (memsv_(l) != 0)
                            {   /* ANSI 3rd draft says unnamed bitfields */
                                /* never consume initialisers.           */
                                int32 leftshift = p.boffset;
                                if (!target_lsbitfirst)
                                    leftshift = MAXBITSIZE - k - leftshift;
                                if (woffset != p.woffset)
                                {   if (target_lsbytefirst)
                                        leftshift -= (woffset-p.woffset)*8;
                                    else
                                        leftshift += (woffset-p.woffset)*8;
                                }
                                if ((leftshift + k) > bfsize)
                                    bfsize = leftshift + k;
                                bfval |= rd_bitinit(l->memtype, k)
                                            << leftshift;
                            }
                        }
                        else
                        {   if (bfsize)
                            {   initbitfield(bfval, bfsize, 0);
                                bfsize = bfval = 0;
                            }
                            padstatic(alignof_member);
                            padstatic(alignoftype(l->memtype));
#ifdef CPLUSPLUS
                            if (attributes_(l) & CB_MASK &&
                                complain_non_aggregate)
                            {   cc_warn(vargen_warn_init_non_aggregate);
                                complain_non_aggregate = 0;
                            }
#endif
                            initsubstatic(l->memtype);
                            woffset = p.woffset + p.typesize;
                        }
                        /* only the 1st field of a union can be initialised */
                        if (is_union) break;
                    }
                    if (bfsize) initbitfield(bfval, bfsize, is_union);
                    if (is_union)
                        gendc0(sizeoftype(t) - (l==0 ? 0 : /* empty union!! */
                             bfsize ? sizeof_int : sizeoftype(l->memtype)));
#ifdef CPLUSPLUS        /* See sem.c(sizeoftype) -- check this agrees   */
                    else
                        if (bfsize == 0 && p.n == 0)
                            gendc0(1); /* empty C++ struct.   */
#endif
                    syn_end_agg(note);
                    padstatic(alignoftype(t));  /* often, alignof_struct */
                    break;
                }
                case bitoftype_(s_typedefname):
                    initsubstatic(bindtype_(typespecbind_(t)));
                    break;
                default:
                    syserr(syserr_initstatic, (long)h0_(t), (long)m);
                    break;
            }
            break;
        case t_fnap:  /* spotted earlier */
        default:
            syserr(syserr_initstatic1, (long)h0_(t));
        case t_subscript:
        {   int32 note = syn_begin_agg();          /* skips and notes if '{' */
            int32 i, m = typesubsize_(t) ? evaluate(typesubsize_(t)):0xffffff;
            TypeExpr *t2;
            if (m == 0)
            {   syn_end_agg(note);
/* Be careful with struct { char x; int y[0]; char y; } even if ANSI illegal */
                break;
            }
/* N.B. the code here updates the size of an initialised [] array          */
/* with the size of its initialiser.  initstaticvar()                      */
/* ensures (by copying types if necessary) that this does not clobber      */
/* a typedef in things like: typedef int a[]; a b = {1,2};                 */
            if (!syn_canrdinit())
            {   if (typesubsize_(t) == 0)
                    cc_err(vargen_err_open_array);
            }
            else if (t2 = princtype(typearg_(t)), isprimtype_(t2,s_char))
            {   Expr *e = rdinit(0,1);
                if (e != 0 && (isstring_(h0_(e))) && h0_(e) != s_wstring)
                {   int32 k = genstring(((String *)e),m);
                    if (typesubsize_(t) == 0)
                        typesubsize_(t) = globalize_int(k);
                    syn_end_agg(note);
                    break;
                }
                syn_undohack = 1;
                /* we know that m > 0 so the syn_rdinit() is executed */
            }
            else if (t2 = princtype(typearg_(t)),
/* t2 is maybe const/volatile 'signed int' or 'int':                  */
                       h0_(t2) == s_typespec && (typespecmap_(t2) &
                              (bitoftype_(s_int)|bitoftype_(s_long)|
                               bitoftype_(s_short)|bitoftype_(s_unsigned))) ==
                           bitoftype_(s_int))
            {   Expr *e = rdinit(0,1);
                if (e != 0 && h0_(e) == s_wstring)
                {   int32 k = genstring(((String *)e),m);
                    if (typesubsize_(t) == 0)
                        typesubsize_(t) = globalize_int(k);
                    syn_end_agg(note);
                    break;
                }
                syn_undohack = 1;
                /* we know that m > 0 so the syn_rdinit() is executed */
            }
/* Maybe generalise this one day:                                       */
#define vg_init_to_null(t) (TARGET_NULL_BITPATTERN == 0)
            for (i = 0; i < m; i++)
            {   if (!syn_canrdinit())
                {   if (typesubsize_(t) == 0)
                    {   typesubsize_(t) = globalize_int(i);
                        break;  /* set size to number of elements read. */
                    }
                    if (vg_init_to_null(typearg_(t)))
                    {   gendc0((m-i)*sizeoftype(typearg_(t)));
                        break;  /* optimise multi-zero initialisation.  */
                    }
                }
                initsubstatic(typearg_(t));
            }
            syn_end_agg(note);
            break;
        }
        case t_content:
        case t_ref:
        {   Expr *init = rdinit(t,0);
            while (init && h0_(init) == s_cast) init = arg1_(init);
            if (init == 0)
                gendcI(sizeof_ptr, TARGET_NULL_BITPATTERN);
/*@@@ AM: WR_STR_LITS code needs to be available here, but we also have a */
/* a rats nest of re-targetting code.  AM working on this.                */
            else if (isstring_(h0_(init)))
            {   /* put the strings in code space, nonmodifiable -
                   rely on the fact that CG thinks it is between routines
                   (even for local static inits).                      */
                /* codeloc() is a routine temporarily for forward ref */
                genpointer(init);  /* maybe in data segment, if WR_STR_LITS */
            }
            else if (h0_(init) == s_integer)
                gendcI(sizeof_ptr, intval_(init)); /* casted int to pointer */
            else genpointer(init);
            break;
        }
    }
}

/* Should be static except for initstaticvar(datasegment) in binder.c   */
void initstaticvar(Binder *b, bool topflag)
{   if (debugging(DEBUG_DATA))
        cc_msg("%.6lx: %s%s\n", (long)data.size, topflag ? "":"; ",
               symname_(bindsym_(b)));
    bindaddr_(b) = datap->size;
    if (topflag) /* note: names of local statics may clash but cannot be
                    forward refs (except for fns which don't come here) */
    {   labeldata(bindsym_(b));
        (void)obj_symref(bindsym_(b),
                   (bindstg_(b) & bitofstg_(s_extern) ? datap->xrarea+xr_defext :
                                                        datap->xrarea+xr_defloc),
                   datap->size);
    }
    if (b == datasegment) return;      /* not really tidy */
    /*
     * A decl such as 'typedef char MSG[];' gets side effected by
     * 'MSG name = "A name";'.  Therefore copy type before initialiser
     * is read ... Here we go ...
     */
    if (isprimtype_(bindtype_(b), s_typedefname))
    {   TypeExpr *t = prunetype(bindtype_(b));
        if (h0_(t)==t_subscript && typesubsize_(t) == 0)
            /* the next line is idle, since case is rare...             */
            /* (We must alloc. glob store since b maybe top level       */
            /*  and hence its type will already be globalised, but      */
            /*  prunetype() may alloc. local store -- globalise all.)   */
            /* Note that the next line relies on globalize_typeexpr     */
            /* NOT caching empty arrays, hence we get a fresh copy.     */
            bindtype_(b) = globalize_typeexpr(t);
    }
    {
#ifdef TARGET_IS_ACW
/* The following (hackish) lines force FEATURE_WR_STR_LITS (which puts   */
/* strings in data segment) for the Acorn 32000 machine) which, due to   */
/* TARGET_CALL_USES_DESCRIPTOR, are unhappy abount code segment adcons   */
/* (as opposed to descriptors) in a data segment.                        */
        int32 f = feature; feature |= FEATURE_WR_STR_LITS;
#endif
        initsubstatic(bindtype_(b));
/* Note that the following padstatic(alignof_toplevel) helps alignment   */
/* of (e.g.) strings (which often speeds memcpy etc.).  However it is    */
/* also assumed to happen by the code for initialising auto arrays.      */
/* See the call to trydeletezerodata().                                  */
        padstatic(alignof_toplevel);
#ifdef TARGET_IS_ACW
        feature = f;
#endif
    }
    codeseg_flush(0);           /* Aug 90: arg is ALWAYS 0 -- check.     */
}

static Expr *quietaddrof(Binder *b)
/* This function exists only to keep PCC mode happy. ensurelvalue() in */
/* sem is what's really needed, but it isn't exported and whinges in   */
/* pcc mode about &<array>. In the long-term, something like this is   */
/* needed for export from simplify (a quiet force-address-of-tree opn  */
/* for trusted callers) so until then LDS leves this bodge here.       */
{   bindstg_(b) |= b_addrof;
    return mk_expr1(s_addrof, ptrtotype_(bindtype_(b)), (Expr *)b);
}

/* Auxiliary routine for initialising auto array/struct/unions */
static Expr *vg_copyobj(Binder *sb, Binder *b, int32 size)
{   /* Note that the same code suffices for array and structs as it is */
    /* quite legal to take the address of an array, implicitly or      */
    /* explicitly.  Note that b/sb both have array/struct/union type.  */
    return mk_expr2(s_fnap, primtype_(bitoftype_(s_void)), sim.memcpyfn,
              (Expr *)mkExprList(
                mkExprList(
                  mkExprList(0, mkintconst(te_int, size, 0)),
                  quietaddrof(sb)),
                quietaddrof(b)));
}

static Expr *vg_zeroobj(Binder *b, int32 offset, int32 zeros)
{   Expr *addrb = quietaddrof(b);
    return mk_expr2(s_fnap, primtype_(bitoftype_(s_void)), sim.memsetfn,
              (Expr *)mkExprList(
                mkExprList(
                  mkExprList(0, mkintconst(te_int,zeros,0)),
                  mkintconst(te_int,0,0)),
                mk_expr2(s_plus, typeofexpr(addrb), addrb,
                         mkintconst(te_int, offset, 0))));
}

/* The following routine removes generated statics, which MUST have been
   instated with instate_declaration().  Dynamic initialistions are turned
   into assignments for rd_block(), by return'ing.  0 means no init.
   Ensure type errors are noticed here (for line numbers etc.) */
/* AM: the 'const's below are to police the unchanging nature of 'd'    */
/* (and its subfields stg,b).  Note that 't' can be changed.            */

Expr *genstaticparts(DeclRhsList *const d, bool topflag, bool dummy_call)
{ const SET_BITMAP stg = d->declstg;
  Binder     *const b  = d->declbind;
  TypeExpr   *t  = prunetype(d->decltype);
  Expr *dyninit = 0;        /* no dynamic init by default.            */
  int32 loc = 0;
  str_lits = NULL;          /* no static string inits seen yet.       */
  init_adcons = NO;
#ifdef CPLUSPLUS
  init_fl = d->fileline;
  complain_non_aggregate = !dummy_call;
#else
  IGNORE(dummy_call);
#endif
  /* Apr 92: this case was recently lifted out -- maybe it shows a    */
  /* bug(?) whereby s_typedef and b_fnconst can both be set?          */
  if (stg & bitofstg_(s_typedef)) {
    if (usrdbg(DBG_PROC) && topflag)
      dbg_type(bindsym_(b), bindtype_(b));
  }
  else
  { if (!(stg & b_fnconst)) switch (stg & PRINCSTGBITS)
    {   case bitofstg_(s_auto):        /* includes register vars too */
            /*
             * Deal with arrays, structs and unions here
             * Treat auto a[5] = 2; consistently with static a[5] = 2;
             * by always trying to read an initialiser for an array.
             */
            if ( syn_canrdinit() &&
                 ( h0_(t) == t_subscript ||
                   ( curlex.sym == s_lbrace &&
                     h0_(t) == s_typespec &&
                     typespecmap_(t) & CLASSBITS)))
            {   /* For an initialised auto array/struct/union generate    */
                /* the whole object (ANSI 3rd draft say initialising one  */
                /* component of an object initialises it all) in static   */
                /* space and generate a run-time copy.   We treat large   */
                /* terminal zero segments specially.                      */
                /* For consistency this is a source-to-source translation. */
                int32 size, zeros;
/* NB the use of bindtype_(b) in the next line instead of t avoids        */
/* updating a possible open array typedef.                                */
                Binder *sb = mk_binder(gensymval(0), bitofstg_(s_static),
                                       bindtype_(b));
                DataInit *start;
                /* It suffices to allocate sb like any other local static. */
#ifdef TARGET_IS_HELIOS
/*
 * A strange option needed with Helios (for building a shared library) can
 * make this mechanism fall apart (the forged static may not get set up)
 * so I generate a diagnostic to warn people.  Yuk at the break of modularity.
 */
                {   extern bool suppress_module;
                    if (suppress_module)
                        cc_err(vg_err_dynamicinit);
                }
#endif
#ifdef CONST_DATA_IN_CODE
                if ( !(config & CONFIG_REENTRANT_CODE) ||
                     pointerfree_type(bindtype_(b)))
                {   datap = &constdata;
                    binduses_(sb) |= u_constdata;
                }
#endif
                start = (datap->head == NULL) ? NULL : datap->tail;
                /*
                 * Create hidden static for auto initialiser.
                 */
                padstatic(alignoftype(bindtype_(sb)));
                initstaticvar(sb, NO);
/* Update bindtype_(b) in case it was typedef to open array which       */
/* would have been copied (cloned) by initstaticvar().                  */
                t = bindtype_(b) = bindtype_(sb);
                size = sizeoftype(t);    /* size of auto [] now known.  */

/* The following line is helpful for initialising auto char arrays.     */
/* We pad the size for initialisation purposes up to a multiple of      */
/* alignof_toplevel, safe (in the assumption) that flowgraf.c has padded*/
/* the stack object and that initstaticvar() below has done the same    */
/* for the statically allocated template.                               */
/* The effect is to encourage cg.c to optimised word-oriented copies.   */
                size = padsize(size, alignof_toplevel);

/* If less than 8 words of zeros in array, struct or union then do not  */
/* remove trailing zeros.  trydeletezerodata() is a multiple of 4.      */
                zeros = trydeletezerodata(start, 32);
                if (zeros == 0)
                    dyninit = vg_copyobj(sb, b, size);
                else
                {   /*
                     * Call function to copy trailing zeros to the data
                     * structure.
                     */
                    dyninit = vg_zeroobj(b, size-zeros, zeros);
                    /*
                     * Copy hidden static to auto array, struct or union
                     * without any trailing zeros.
                     */
                    if (size>zeros)
                        dyninit = mkbinary(s_comma,
                                           vg_copyobj(sb, b, size-zeros),
                                           dyninit);
                }
                dyninit = optimise0(dyninit);
                datap = &data;
            }
            else
            {   Expr *e = syn_rdinit(d->decltype,0);   /* no optimise0 */
                if (e)
                    dyninit = optimise0(mkassign(s_assign, (Expr *)b, e));
            }
#ifdef CPLUSPLUSxxx
/* FIXME... */
            if (isclasstype_(t) && dyninit == NULL)
                dyninit = vbase_init(typespectagbind_(t), b);
#endif
            break;
#ifndef NON_CODEMIST_MIDDLE_END
        case b_globalregvar:
        {   VRegnum reg = GAP;
            int32 rno = declstgval_(d) >> 1;
            if (declstgval_(d) & 1) {
#ifndef TARGET_SHARES_INTEGER_AND_FP_REGISTERS
                if (rno > MAXGLOBFLTREG)
                    cc_err(vargen_err_overlarge_reg);
                else {
                    TypeExpr *t = princtype(bindtype_(b));
                    if ( h0_(t) == s_typespec &&
                         ( typespecmap_(t) & bitoftype_(s_double) ) )
                        reg = virtreg(R_FV1+rno-1,
                              typespecmap_(t) & bitoftype_(s_short) ?
                                      FLTREG : DBLREG);
                    else
                        cc_err(vargen_err_not_float);
                }
#else
                cc_err(vargen_err_not_int);
#endif
            } else {
                if (rno > MAXGLOBINTREG)
                    cc_err(vargen_err_overlarge_reg);
                else {
                    TypeExpr *t = prunetype(bindtype_(b));
                    reg = virtreg(R_V1+rno-1, INTREG);
                    switch (h0_(t)) {
                    case t_content:
                        break;
                    case s_typespec:
                        if (typespecmap_(t) & ( bitoftype_(s_char) |
                                                bitoftype_(s_int) |
                                                bitoftype_(s_enum) ))
                            break;
                        /* no one_word structs, unions - cg isn't up to the
                           notion that they might be in registers (?) */
                        /* @@@AM: it ought to be! */
                    default:
                        cc_err(vargen_err_not_int);
                        reg = GAP;
                    }
                }
            }
            if (reg != GAP) {
            /* We have arranged that this binder has room to record a
             * register (though normally only auto binders have).
             * We alter the storage class to s_auto, to stop cg
             * wanting to access it through ADCONs.
             * Actually, should be s_register, but that's more effort.
             */
                if (bindxx_(b) != GAP && bindxx_(b) != reg)
                    cc_rerr(bind_err_conflicting_globalreg, b);
                bindxx_(b) = reg;
                globalregistervariable(reg);
                loc = reg;
            }
            break;
        }
#endif /* NON_CODEMIST_MIDDLE_END */

        default:
            syserr(syserr_rd_decl_init, (long)stg);
            /* assume static */
        case bitofstg_(s_static):
        case bitofstg_(s_extern):
            if (!(d->declstg & b_undef))
            {   padstatic(alignoftype(bindtype_(b)));
                loc = datap->size;
                initstaticvar(b, topflag);
                /* Put out debug info AFTER initialised array size has    */
                /* been filled in by initstaticvar()                      */
                break;
            }
            else
            {
                if (feature & FEATURE_PCC)
                {   TypeExpr *t = princtype(bindtype_(b));
                    /*
                     * Found a declaration like int foo; with no initialiser.
                     * PCC regards this as common which is encoded as an
                     * undefined external data reference with a non-0 size.
                     * BUT BEWARE: int foo[]; is NOT a common variable - it
                     * is an extern decl. Thus, we have a look for undefined
                     * arrays.  (e.g. xxx[1][3][]), possibly via a leading
                     * typdef. If we find one then we exit.
                     * AM: @@@ use is_openarray() soon?  I would now, but
                     * am nervous about int a[][]; in pcc mode.
                     */
                    for (; h0_(t) == t_subscript; t = princtype(typearg_(t)))
                    {   if (typesubsize_(t) == 0)
                        {
#ifndef TARGET_IS_UNIX
                            /* Add debug info for open arrays (ASD only) */
                            if (usrdbg(DBG_PROC) && topflag)
#ifdef TARGET_HAS_BSS
                                dbg_topvar(bindsym_(b),0,bindtype_(b),DS_EXT,d->fileline);
#else
/* Not all Codemist clients unix interfaces have BSS yet (e.g. COFF)    */
                                dbg_topvar(bindsym_(b),0,bindtype_(b),1,d->fileline);
#endif
#endif
                            goto switch_break;
                        }
                    }
                    /*
                     *  Generate a special extern reference for PCC style
                     *  common variables.
                     */
/* @@@ This is a genuine feature or config which other systems may want. */
/* Provide a switch one day soon.                                       */
                    if ((stg & bitofstg_(s_extern)) && (stg & b_omitextern))
                        (void)obj_symref(bindsym_(b), xr_data+xr_comref,
                                         sizeoftype(bindtype_(b)));
                }
                loc = bindaddr_(b);
                break;
            }
    }
    /* Add debug information for common and extern variables */
    if (usrdbg(DBG_PROC) && topflag && !(stg & b_fnconst))
    {
#ifdef TARGET_HAS_BSS
        SET_BITMAP stg = bindstg_(b);
        dbg_topvar(bindsym_(b), loc, bindtype_(b),
                   stg & b_globalregvar ? DS_REG :
                   ( (stg & bitofstg_(s_extern) ? DS_EXT : 0) |
                     (stg & u_bss ? DS_BSS :
                     stg & u_constdata ? DS_CODE : 0)), d->fileline);
#else
        dbg_topvar(bindsym_(b), 0, bindtype_(b), 1, d->fileline);
#endif
    }
  }
switch_break:
/* AM Aug 90: bug fix to stop initialisation of a tentative to be a     */
/* string pointer leaving the string pointer in the wrong place in the  */
/* data segment in FEATURE_WR_STR_LITS mode.  Now that the ANSI std     */
/* has appeared, the whole tentative/bss/vargen edifice ought to be     */
/* rationally reconstructed.                                            */
   /*
    * The next call is part of a disgusting fix for tentative
    * static and extern decls.  What happens is as follows:
    * We parse along until we find a decl with no initialiser.
    * We then set 'b_undef' for this symbol and pass it to
    * 'instate_declaration()' above.  This symbol is then entered
    * into the symbol table and 'b_undef' is UNSET if it is a
    * tentative decl so that 'genstaticparts()' WILL allocate it
    * some store.
    * Now if sometime later a REAL initialiser is found for
    * this symbol, this is detected by is_tentative() in mip/bind.c
    * which removes the zeros from the 'data.head/tail' lists and then call
    * 'genstaticparts()' to read the initialiser and finally we call
    * 'reset_vg_after_init_of_...()' below to fix the tables.
    */
  reset_vg_after_init_of_tentative_defn();
  datap = &data;
    /*
     * Now generate the string literals we have delayed generating.
     * Also, we relocate the references to them by updating the
     * values in the data-generation templates that will cause those
     * references to be dumped in the data area... Note that we reverse
     * the work list so literals are generated in source order.
     * Note also that the following is a no-op unless FEATURE_WR_STR_LITS.
     */
  { struct StrLit *p = (struct StrLit *) dreverse((List *)str_lits);
    for (; p != NULL;  p = p->next)
    { p->d->val += data.size;      /* real offset of generated lit */
      genstring(p->s, 0xffffff); /* literal dumped in data area  */
      padstatic(alignof_toplevel);
    }
  }
  return dyninit;
}

/* end of vargen.c */
