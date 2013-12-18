/*
 * simplify.c: tree optimisation phase of C compiler
 * Copyright (C) Codemist Ltd, 1988-1992.
 * Copyright (C) Acorn Computers Ltd., 1988-1990.
 * Copyright (C) Advanced RISC Machines Limited, 1990-1992.
 */

/*
 * RCS $Revision: 1.4 $  Codemist 11
 * Checkin $Date: 1993/08/02 09:32:27 $
 * Revising $Author: nickc $
 */

#include "globals.h"
#include "simplify.h"
#include "sem.h"
#include "bind.h"
#include "aetree.h"
#include "builtin.h"
#include "store.h"
#include "aeops.h"
#include "errors.h"

/* AM nov-88: add code to allow mcrep fns to return a flag if the     */
/* object requires double alignment (e.g. some structs)               */
/* As a transition this is only enabled if alignof_double > alignof_int. */
/* AM 24-sep-87: created from sem.c, but the idea is soon to put tree */
/* replacement bits of cg.c in here too.                              */
/* Fix (unreleased) bug in *(int *)&x caused by over-keen optimisation */
/* Re-fix to kill bug in (LDS's) extern int a[]; return *a;            */

/* optimise0() is called after parsing an expression (generally by syn.c) */
/* which has been type-checked on the way by sem.c.  It does tree-like    */
/* optimisations, such as removing casts between objects which have the   */
/* same run-time representation (e.g. char* -> int* -> long)              */

/* expression optimiser:
 * Current aims for optimise():
 *  0) Remove s_invisible nodes introduced while parsing for error messages.
 *  1) reduce constant operations.  Most (e.g. 1+2) is done when
 *     building tree nodes, however, this is a good place to spot
 *     (x+1)+2, or (x.a).b.  Unfortunately this is not done yet so
 *     genpointer() in vargen.c and the CG both do similar things with the
 *     result of optimise().  Also turn x+0 to x soon.
 *  2) Transform &*x to x, and &x.a to &x + a.
 *  3) Regroup certain arithmetic expressions so as to allow more
 *     constant folding to occur.
 * The resultant tree is normalised to some extent.  E.g. & is only
 * applied to s_binder's.
 */

/* forward references... */
static void optimiselist(ExprList *);

/* The following routine fixes LDS's bug (in 1.57) alluded to above.  */
/* It is not the final word, since we can clearly sometimes do better */
/* at detecting same pointers, but is intended to keep LDS happy with */
/* 'minimal change to code'.                                          */
static bool same_pointedto_types(Expr *e, Expr *e1)
{   TypeExpr *x = prunetype(typeofexpr(e));
    TypeExpr *x1 = prunetype(typeofexpr(e1));
    /* Careful not to pass arrays/structs/fns to mcrepofexpr()      */
    /* (and thence sizeoftype which can issue error messages).      */
    /* The next line duplicates previous behaviour on t_fnap types. */
    if (h0_(x) == t_fnap && h0_(x1) == t_fnap)
        return 1;
    if (h0_(x) == t_subscript || h0_(x) == t_fnap || isclasstype_(x))
        return 0;
    if (h0_(x1) == t_subscript || h0_(x1) == t_fnap || isclasstype_(x1))
        return 0;
    return mcrepofexpr(e) == mcrepofexpr(e1);
}

#ifdef SIMPLIFY_OPTIMISE_CHAR_AND_SHORT_ARITHMETIC
/*
 * The code here exhibits obscure failures, and is (maybe) being superseded
 * by code in regalloc.c, so is now included only if explicitly requested.
 */

static int32 mc_int_width(Expr *e)
{
    /* A small integer iff e has integer type of width 1, 2 or 4.        */
    /* Otherwise somewhat bigger than 1 << MCR_SORT_SHIFT (> 2**24).     */
    return mcrepofexpr(e) & ~(5L << MCR_SORT_SHIFT);
}

/* @@@ LDS 14Aug89 - making this ARM-dependent is provisionsal - other   */
/* targets could benefit from the manipulations done here. However, I    */
/* just haven't managed to bend my brain round the relevant abstractions.*/
/* AM/ACN should tidy up here for other back ends and maybe consider     */
/* integrating optimise2() with optimise1()... I found this too daunting.*/
/* I apologise for the plethora of magic numbers, but I seem to be       */
/* following a grand tradition set by cg and simplify via mcrepofexpr(). */

static TypeExpr *te_plain(int32 w)
{   int32 m = bitoftype_(s_signed) | bitoftype_(s_unsigned);
/* w == 1 denotes te_signedchar; w == 2 denotes te_short.                */
    if (w == 2)
        m |= bitoftype_(s_short) | bitoftype_(s_int);
    else if (w == 1)
        m |= bitoftype_(s_char);
    else
        syserr(syserr_te_plain, w);
    return primtype_(m);
}

static Expr *clone_node(Expr *e, TypeExpr *t)
{   int32 h0 = h0_(e);
    switch (h0)
    {
case s_cond:
case s_rangecheck:
        return mk_expr3(h0, t, arg1_(e), arg2_(e), arg3_(e));
case s_dot:
        if (isbitfield_type(typeofexpr(e)))
            return mk_exprbdot(h0, t, arg1_(e), exprdotoff_(e),
                               exprbsize_(e), exprmsboff_(e));
        else
            return mk_exprwdot(h0, t, arg1_(e), exprdotoff_(e));
default:
        if (ismonad_(h0) || h0 == s_cast)
            return mk_expr1(h0, t, arg1_(e));
        else if (isdiad_(h0) ||
                 h0 == s_fnap || h0 == s_let || h0 == s_checknot)
            return mk_expr2(h0, t, arg1_(e), arg2_(e));
        syserr(syserr_clone_node, h0);
    }
    return e;
}

/* The purpose of the following function is to do tree-transformations   */
/* which change explicitly (un)signed load/store-B/Ws into plain ops and */
/* introduce explicit (narrowing) casts higher up the tree. On the ARM   */
/* this is a win - and well nigh essential for Unix which requires chars */
/* to be signed chars make extensive use of shorts - all anathema to ARM.*/
/* It's sad that much of optimise1 has to be copied here...              */

static Expr *optimise2(Expr *e, int32 mc_width)
{
/* mc_width is the width of the narrow integer context in which 'e' is   */
/* to be evaluated, as seen from the back-end's point of view. mc_width  */
/* == 0 if the width is unknown or the context is not narrow integer.    */
    if (debugging(DEBUG_AETREE) && syserr_behaviour > 0)
    {   cc_msg("optimise2(..., %ld): ", mc_width);
        pr_expr(e);
        cc_msg(" ...of type... ");
        pr_typeexpr(typeofexpr(e), 0);
        cc_msg("\n");
    }
    switch (h0_(e))
    {
case s_fnap:
        {   ExprList *x;
            arg1_(e) = optimise2(arg1_(e), 0);
            for (x = exprfnargs_(e);  x != 0;  x = cdr_(x))
                exprcar_(x) = optimise2(exprcar_(x), 0);
        }
        break;
case s_comma:
        arg1_(e) = optimise2(arg1_(e), -1);
        /* drop through */
case s_let:
        arg2_(e) = optimise2(arg2_(e), mc_width);
        break;
case s_assign:
case s_displace:
        {   Expr *a1 = arg1_(e), *a2 = arg2_(e);
            int32 w1 = mc_int_width(e), w2 = (w1 <= 2) ? w1 : 0;
            TypeExpr *t1 = typeofexpr(e);
/* N.B. The following call may change typeofexpr(a1) in case s_displace. */
            arg1_(e) = a1 = optimise2(a1,
                (h0_(e) == s_displace && mc_width >= 0 ? w2 : 0));
            if (h0_(a1) == s_binder)
/* @@@ LDS 07-Dec-89 - The value stored into a binder is expected by cg.c  */
/* to be 'clean', whether the binder's value will be in a register or in   */
/* memory. A cg.c peephole and cg's value loading strategy depend on this. */
                w2 = 0;
            arg2_(e) = a2 = optimise2(a2, w2);
            if (w2 == 0) break;             /* not narrow or already clean */
            w2 = mc_int_width(a2);
            if (w2 < w1) t1 = typeofexpr(a2);     /* char in short context */
            if (mc_width == 0 || w1 < mc_width)
                /* NB: t1 is type_(a1) BEFORE optimising a1 */
                e = mk_expr1(s_cast, t1, clone_node(e, te_int));
        }
        break;
case s_cond:
/* @@@ LDS - we could do better here sometimes, but it's hard work to do */
/* so I don't atempt it for the moment.                                  */
        if (mc_width > 0) mc_width = 0;
        arg2_(e) = optimise2(arg2_(e), mc_width);
        arg3_(e) = optimise2(arg3_(e), mc_width);
        arg1_(e) = optimise2(arg1_(e), 0);                /* N.B. 0 here */
        break;
case s_content:
case s_dot:
/* in the following line, 0, NOT mc_width IS correct (think about it!).  */
        arg1_(e) = optimise2(arg1_(e), 0);
        /* DROP THROUGH */
case s_binder:
        {   int32 ew;
            if (mc_width > 0 &&                   /* in a narrow context */
                (ew = mc_int_width(e)) <= 2 &&    /* and a narrow exprn  */
                ew >= mc_width &&               /* which is not narrower */
                !isvolatile_expr(e))                 /* and not volatile */
/* @@@ LDS - the following causes cg to generate a 'plain' load of a     */
/* narrow type, rather than a signed or unsigned one. See cg_cast1().    */
            {   if (h0_(e) == s_binder)
                    e = mk_expr1(s_cast, te_plain(ew), e);
                else if (h0_(e) == s_content)
                    e = mk_expr1(s_content, te_plain(ew), arg1_(e));
                else /* s_dot */
                {   if (isbitfield_type(typeofexpr(e)))
/* Surely this case is a syserr, since bitfields have now gone!.        */
                        e = mk_exprbdot(s_dot, te_plain(ew), arg1_(e),
                                exprdotoff_(e), exprbsize_(e), exprmsboff_(e));
                    else
                        e = mk_exprwdot(s_dot, te_plain(ew), arg1_(e),
                                exprdotoff_(e));
                }
            }
        }
        break;
case s_cast:
        {   Expr *a1 = arg1_(e);
            int32 ew = mc_int_width(e);
            if (mc_width < 0 ||                  /* cast in void context */
                mc_width > 0 &&             /* OR narrow integer context */
                ew <= 4 &&              /* & this cast is to an int type */
                ew >= mc_width &&               /* which is not narrower */
                mc_int_width(a1) <= 4)          /* & castee has int type */
            {   /* omit the cast */
                e = optimise2(a1, mc_width);
                break;
            }
            if (ew == 0)                                 /* cast to void */
                mc_width = -1;
            else if (ew > 4)                         /* cast to floating */
                mc_width = 0;
            else if (mc_width == 0 && ew <= 2 || ew < mc_width)
                mc_width = ew;
            arg1_(e) = a1 = optimise2(a1, mc_width);
/* The following predicate checks whether this cast has been rendered    */
/* redundant by a cast introduced by the call to optimise2(), above..    */
/* NOTE: in the following predicate, DO NOT change ... < ew to ... <= ew */
/* lest AM's problem with (int)(unsigned)d recur.                        */
            if (ew <= 4 && h0_(a1) == s_cast && mc_int_width(a1) < ew) e = a1;
        }
        break;
case s_equalequal:
case s_notequal:
        {   Expr *a1 = arg1_(e), *a2 = arg2_(e);
            mc_width = 0;                      /* int context by default */
            if (h0_(a1) != s_integer && h0_(a2) == s_integer)
            {   a1 = a2;                  /* if either arg is an integer */
                a2 = arg1_(e);            /* a1 is now an integer...     */
            }
/* Here we do a special case for signed chars, very important for Unix.  */
            if (h0_(a1) == s_integer &&
                  (unsigned32) intval_(a1) <=            /* int const... */
                    ((unsigned char)(-1) >> 1) &&     /* ...<= SCHAR_MAX */
                      mc_int_width(a2) == 1)       /* compared with char */
                mc_width = 1;               /* so set context width to 1 */
        }
        /* and drop through with mc_width set appropriately */
case s_and:
case s_times:
case s_plus:
case s_minus:
case s_or:
case s_xor:
        arg2_(e) = optimise2(arg2_(e), mc_width);
        /* drop through */
case s_neg:
case s_bitnot:
case s_monplus:
        arg1_(e) = optimise2(arg1_(e), mc_width);
        break;
#ifdef RANGECHECK_SUPPORTED
case s_rangecheck:
        if (arg3_(e) != NULL) arg3_(e) = optimise2(arg3_(e), 0);
        /* drop through */
case s_checknot:
        arg1_(e) = optimise2(arg1_(e), 0);
        if (arg2_(e) != NULL) arg2_(e) = optimise2(arg2_(e), 0);
        break;
#endif
default:
        {   AEop op = h0_(e);
            if (isdiad_(op))
                arg2_(e) = optimise2(arg2_(e), 0);
            if (isdiad_(op) || ismonad_(op))
                arg1_(e) = optimise2(arg1_(e), 0);
        }
        break;
    }
    if (debugging(DEBUG_AETREE) && syserr_behaviour > 0) {
        cc_msg(" => "); pr_expr(e); cc_msg("\n");
    }
    return e;
}

#else /* SIMPLIFY_OPTIMISE_CHAR_AND_SHORT_ARITHMETIC */

#define optimise2(e, width) (e)

#endif /* SIMPLIFY_OPTIMISE_CHAR_AND_SHORT_ARITHMETIC */

static Expr *optimise_cast(Expr *e)
{   Expr *a1 = arg1_(e);
    int32 e_mode = mcrepofexpr(e), e_len;
    int32 a_mode = mcrepofexpr(a1), a_len;
    TypeExpr *te = princtype(typeofexpr(e));
    if (h0_(te) == t_content) te = typearg_(te);
    if (isfntype(te)) return e;  /* leave in casts to fn type */

/* a cast to the same type is ineffectual                                */
    if (e_mode == a_mode) return a1;

    e_len = e_mode & MCR_SIZE_MASK;
    e_mode >>= MCR_SORT_SHIFT;
    a_len = a_mode & MCR_SIZE_MASK;
    a_mode >>= MCR_SORT_SHIFT;

/* A cast is ineffectual if it does not change the m/c representation.   */
    if (e_mode < 2 && a_mode < 2)        /* cast of integral to integral */
    {
        if (e_mode == a_mode && e_len > a_len)
            return a1;         /* vacuous signedness-preserving widening */

/* Things like (int)(unsigned char)x are NOT vacuous, even though cg     */
/* will generate no code for them, because (double)(int)(unsigned char)x */
/* (double)(unsigned char)x and (double)(signed char)x are all different.*/
/* Even (unsigned int)(int) has a role: consider:-                       */
/* double d;  (double)(unsigned)(int)d; - what can be elided? Nothing!   */
    }

    if (h0_(a1) == s_cast)
    {
/* The inner casts are irrelevant in the following cases:                */
/*     (float) (double) x                                                */
/*     (char)  (short)  x   (char) (int) x   (short) (int) x             */
/* So are the corresponding unsigned cases.                              */
        if (e_mode <  2 && a_mode <  2 && e_len < a_len ||
            e_mode == 2 && a_mode == 2 && e_len < a_len)
        {
            arg1_(e) = arg1_(a1);
        }
    }
    return e;
}
/*
   AM: expressions such as  structfn().a[i]  are odd -- the ANSI spec seems
   to be very unclear about whether they illegally require the address
   of a function result.  To ease implementation all calls to structfn()
   are mapped to  (let t; t=structfn(),t)  but this transformation is
   undone if we know where the result is going, e.g. x = structfn().
*/

static SynBindList *new_binders;

/* Beware highly: optimise currently side-effects the tree.
   Moreover, s_invisible and (more serious) nodes for ++, += etc can
   share sub-structure.  Only binders I believe in the latter case
   so that all should be OK.  Also the use of recursive use of optimise()
   in case s_addrof requires it to be idempotent.
*/

static Expr *optimise1(Expr *e, bool valneeded)
{   AEop op = h0_(e);
    Expr *e1;
    if (debugging(DEBUG_AETREE) && syserr_behaviour > 0) {
      if (op != s_return) {
        cc_msg("optimise1: "); pr_expr(e);
        cc_msg(" of type "); pr_typeexpr(typeofexpr(e), 0);
        cc_msg("\n");
      }
    }
    switch (op)
    {
#ifdef PASCAL /*ECN*/
        case s_error:
            return mkintconst(te_int, 0, 0);
#endif
        case s_integer:
        case s_floatcon:
        case_s_any_string
#ifdef EXTENSION_VALOF
        case s_valof:
#endif
        case s_binder:
            break;
        case s_invisible:
            e = optimise1(arg2_(e), valneeded);
            break;
        case s_return:
            e = arg1_(e);
            if (h0_(e) != s_fnap) { e = optimise1(e, YES); break; }
            /* drop through */
        case s_fnap:
            arg1_(e) = optimise1(arg1_(e), YES);
            optimiselist(exprfnargs_(e));
            if (op == s_return) break;
            /*
             * Does this function return a struct ?.  If so carry out a
             * transformation from 'fn()' to (let x; x=fn(),x)'.
             * Note: Temp binders are allocated in optimise0().
             */
            if ((mcrepofexpr(e) >> MCR_SORT_SHIFT) == 3 && valneeded)
            {   TypeExpr *t = typeofexpr(e);
                Binder *gen = gentempbinder(t);
                new_binders = mkSynBindList(new_binders, gen);
                e = mk_expr2(s_comma,
                             type_(e),
                             mk_expr2(s_assign, t, (Expr*)gen, e),
                             (Expr*) gen);
            }
            break;
        case s_assign:
/* Do not transform struct-returning functions whose result is directly  */
/* assigned to a binder or via s_content...                              */
            arg1_(e) = optimise1(arg1_(e), YES);
            if (h0_(arg2_(e)) == s_fnap &&
                (h0_(arg1_(e)) == s_binder  ||
                 h0_(arg1_(e)) == s_content ||
                 h0_(arg1_(e)) == s_dot))               /* s_dotstar?   */
            {   Expr *funct = arg2_(e);
                arg1_(funct) = optimise1(arg1_(funct), YES);
                optimiselist(exprfnargs_(funct));
            }
            else
                arg2_(e) = optimise1(arg2_(e), YES);
            break;
        case s_addrof:
            e1 = optimise1(arg1_(e), YES);
            if (h0_(e1) == s_comma || h0_(e1) == s_let)
            {   /* & (a, b) -> (a, & b); & (let x in e) -> (let x in & e) */
                e = mk_expr2(h0_(e1), type_(e), arg1_(e1),
                        optimise1(mk_expr1(s_addrof, type_(e), arg2_(e1)), YES));
                break;
            }
            if (h0_(e1) == s_assign)
            {   /* & a = b -> (a = b, & a) */
/* @@@ AM: this is wrong for side-effects in 'a'!!!?                    */
                e = mk_expr2(s_comma, type_(e), e1,
                    optimise1(mk_expr1(s_addrof, type_(e), arg1_(e1)), YES));
                break;
            }
/* The following line fixes a problem that shouldn't occur, as &(cast)   */
/* is illegal. However, it can occur because we recover from ++(type *)p */
/* even though ANSI disallow. Either way, a diagnostic has already been  */
/* issued (or a warning in -pcc mode) so there seems to be little excuse */
/* for generating a syserr(). Note that it also legitimises &(cast)var;  */
/* but not silently. Win some, lose some.                                */
            while (h0_(e1) == s_cast) e1 = arg1_(e1);
            arg1_(e) = e1;
            if (h0_(e1) == s_content) { e = arg1_(e1); break; }
            if (isstring_(h0_(e1))) {
                e = e1;                     /* cg thinks addr already */
                break;
            }
/* Even though s_dot is not optimised out in general it needs to be      */
/* looked after in a special way when I do something like &(a . b)       */
/* Note this can only be legal if a as an lvalue so the result (&a)+nb   */
/* will still be OK - so things like structure-returning functions are   */
/* not involved here.                                                    */
/* AM Nov 92: Hmm, struct fn results now have temps, so OK?              */
            if (h0_(e1) == s_dot) {
                e = mk_expr2(s_plus, type_(e),
/* beware - this means optimise() is invoked twice on arg1_(e) */
                      optimise1(mk_expr1(s_addrof, type_(e), arg1_(e1)), YES),
                      mkintconst(te_int, exprdotoff_(e1), 0));
                break;
            }
            if ((h0_(e1) != s_binder) && (h0_(e1) != s_comma)
#ifdef SOFTWARE_FLOATING_POINT
		&& h0_(e1) != s_floatcon
#endif		
		)
                /*
                 * Check that we have a binder for &.  However, beware we
                 * might have a structure returning function which will
                 * get transformed above.  Should test for this fully but
                 * testing for s_comma is probably ok !.
                 */
                syserr(syserr_optimise, (long)h0_(e1));
            break;
#ifdef CPLUSPLUS
        case_content:
#endif
        case s_content:  /* get rid of extra &'s and *'s introduced above */
            arg1_(e) = e1 = optimise1(arg1_(e), YES);
            if (h0_(e1) == s_addrof &&
                /* the next line ensures *(int*)&d -> d only if types match */
                same_pointedto_types(e, arg1_(e1)))
              e = arg1_(e1);
            break;
#ifndef CPLUSPLUS       /* Nov 92: leave old code for C for now.        */
        case s_dot:  /* s_arrow already removed */
            e1 = optimise1(arg1_(e), YES);
            if (h0_(e1) == s_dot) {
                exprdotoff_(e) += exprdotoff_(e1);
                e1 = arg1_(e1);
            } else if (h0_(e1) == s_content) {
                /* turn *(&foo + x) . y into &foo . (x+y) */
                Expr *e2 = arg1_(e1);
                if (h0_(e2) == s_plus && h0_(arg2_(e2)) == s_integer) {
                    exprdotoff_(e) += intval_(arg2_(e2));
                    e1 = mk_expr1(s_content, type_(e1), arg1_(e2));
                }
            }
            arg1_(e) = e1;
            break;
#else
        case s_dot:     /* replace s_dot with s_content */
/* NB. all s_dot code can be removed from cg.                           */
            if ((mcrepofexpr(arg1_(e)) >> MCR_SORT_SHIFT) != 3)
            {   /* Fetching a word from a single-word struct!.          */
                /* Probably just e (see cg_content_for_dot).            */
                /* @@@ Check 'volatile'.                                */
                /* Insert cast so types are right (e.g. for cg_fnap).   */
                /* This code wouldn't work for a byte in a union!       */
                if (mcrepofexpr(arg1_(e)) == 0x00000004 && exprdotoff_(e)==0)
                {   e = optimise1(mk_expr1(s_cast, type_(e), arg1_(e)), YES);
                    break;
                }
                else
                    syserr("optimise(dot one word)");
            }
            else
            {   TypeExpr *tp = ptrtotype_(type_(e));
                Expr *e2 = mkintconst(te_int, exprdotoff_(e), 0);
/* Beware, assumes optimise(s_fnap(struct)) is address takeable!.       */
                e = mk_expr1(s_content, type_(e),
                      mk_expr2(s_plus, tp,
                        mk_expr1(s_addrof, tp, arg1_(e)), e2));
            }
            goto case_content;
        case s_dotstar:
            if ((mcrepofexpr(arg1_(e)) >> MCR_SORT_SHIFT) != 3)
            {   /* Fetching a word from a single-word struct!.          */
                /* Replace with (e2, e1), i.e. eval e2 for side-effects */
                /* and return e1.  Implies p->*NULL == p->*(&S::s)!!    */
                /* This code wouldn't work for a byte in a union!       */
                if (mcrepofexpr(arg1_(e)) == 0x00000004)
                {   e = optimise1(
                            mk_expr2(s_comma, type_(e), arg2_(e), arg1_(e)),
                            YES);
                    break;
                }
                else
                    syserr("optimise(dotstar one word)");
            }
            else
            {   TypeExpr *tp = ptrtotype_(type_(e));
                Expr *e2 = arg2_(e);
                /* follow [ES] suggestion for ptr-to-member rep.        */
                e2 = mk_expr2(s_minus, typeofexpr(e2), e2,
                              mkintconst(te_int, 1, 0));
/* Beware, assumes optimise(s_fnap(struct)) is address takeable!.       */
                e = mk_expr1(s_content, type_(e),
                      mk_expr2(s_plus, tp,
                        mk_expr1(s_addrof, tp, arg1_(e)), e2));
            }
            goto case_content;
#endif
        case s_cond:
            e1 = e;
            arg1_(e) = optimise1(arg1_(e), YES);
            if ((mcrepofexpr(e) >> MCR_SORT_SHIFT) == 3)
            {   /* The expression is struct-valued */
                TypeExpr *te = type_(e);
                TypeExpr *pt = ptrtotype_(te);
                type_(e) = pt;
                e1 = mk_expr1(s_content, te, e);
                arg2_(e) = mk_expr1(s_addrof, pt, arg2_(e));
                arg3_(e) = mk_expr1(s_addrof, pt, arg3_(e));
            }
            arg2_(e) = optimise1(arg2_(e), valneeded);
            arg3_(e) = optimise1(arg3_(e), valneeded);
            e = e1;
            break;
        case s_let:
            arg2_(e) = optimise1(arg2_(e), valneeded);
            break;
        case s_cast:
            /* beware the 'EQ' test on the next line!                */
            arg1_(e) = optimise1(arg1_(e),
                                 valneeded && typeofexpr(e) != te_void);
            e = optimise_cast(e);
            break;
        case s_and:
        case s_times:
        case s_plus:
        case s_or:
        case s_xor:        /* these are both commutative and associative */
        case s_minus:      /* this isn't -- see goto below */
            {   Expr *a1 = optimise1(arg1_(e), YES);
                Expr *a2 = optimise1(arg2_(e), YES);
/* floating point tree optimisations moved here from cfe/sem.c          */
/* beware IEEE conformance, in that we optimise X+0.0 to X (NaN/-0).    */
                if (op == s_minus)
                {   if (is_fpzero(a1))
                        { h0_(e) = s_neg; arg1_(e) = a2; break; }
                    if (is_fpzero(a2)) { e = a1; break; }
                    goto case_minus;
                }
/* N.B. symmetric things only until case_minus...                       */
                if (h0_(a1) == s_integer || h0_(a1) == s_floatcon)
                {   Expr *w = a1;       /* move any const to arg2       */
                    a1 = a2;            /* note both can't be consts.   */
                    a2 = w;
                }
                if (op == s_plus)
                {   if (is_fpzero(a2)) { e = a1; break; }
                }
                if (op == s_times)
                {   if (is_fpone(a2)) { e = a1; break; }
                    if (is_fpminusone(a2))
                        { h0_(e) = s_neg; arg1_(e) = a1; break; }
                }
                if (h0_(a1) == op && h0_(arg2_(a1)) == s_integer &&
                    h0_(a2) == s_integer)
/* Here I transform   ((a + n) + m)  into a + (n+m)                      */
                {   a2 = mkbinary(op, arg2_(a1), a2);
                    a1 = arg1_(a1);
/* This can convert (e.g.)  (a + 1) + (-1) into  (a + 0) which does not  */
/* get simplified further here. The codegenerator will treat this as     */
/* just a, and the seemingly spurious +0 will serve to preserve the      */
/* proper type of the expression.                                        */
                }
                /* (a & ~k) | k gets transformed here to a | k, to cheer up
                 * a common bitfield-setting case
                 */
                if (op == s_or && h0_(a2) == s_integer &&
                    h0_(a1) == s_and && h0_(arg2_(a1)) == s_integer &&
                    intval_(a2) == ~intval_(arg2_(a1)))
                    a1 = arg1_(a1);

                if (op == s_times && h0_(a2) == s_integer)
                {   if (intval_(a2) == 1) { e = a1; break; }
                    if (h0_(a1) == s_leftshift && h0_(arg2_(a1)) == s_integer)
                    {   int32 shift = intval_(arg2_(a1));
                        int32 n1 = intval_(a2), n2 = ((unsigned32)1 << shift);
                        int32 n3 = n1 * n2;
                        if ((n1 ^ n3 >= 0) && (n3 / n2 == n1)) {
                            a1 = arg1_(a1);
                            a2 = mkintconst(te_int, n3, 0);
                        }
                    }
                    if (h0_(a1) == s_neg) {
                        e = mkunary(s_neg, mkbinary(s_times, arg1_(a1), a2));
                        break;
                    }
                }
                if (op == s_plus) {
                /* a + -b => a - b; -a + b => b-a */
                    if (h0_(a2) == s_neg)
                        h0_(e) = s_minus, a2 = arg1_(a2);
                    else if (h0_(a1) == s_neg) {
                        Expr *t = a2;
                        h0_(e) = s_minus, a2 = arg1_(a1);
                        a1 = t;
                    }
                }
case_minus:     if (op == s_minus)
                {   if (h0_(a2) == s_neg)
                        /* a - -b => a + b */
                        h0_(e) = s_plus, a2 = arg1_(a2);
                    /* should also do -a - b => -(a + b)? */
                }
                arg1_(e) = a1;
                arg2_(e) = a2;
            }
            break;
#ifdef RANGECHECK_SUPPORTED
        case s_rangecheck:
            if (arg3_(e) != NULL) arg3_(e) = optimise1(arg3_(e), YES);
            /* drop through */
        case s_checknot:
            arg1_(e) = optimise1(arg1_(e), YES);
            if (arg2_(e) != NULL) arg2_(e) = optimise1(arg2_(e), YES);
            break;
#endif
        case s_equalequal:
        case s_notequal:
            /* in comparison of signed char for (in)equality against positive
               constant <= SCHAR_MAX, cast the signed char to unsigned char
               (in the belief that zero extension is cheaper than sign extn).
             */
            {   Expr *a1 = optimise1(arg1_(e), YES);
                Expr *a2 = optimise1(arg2_(e), YES);
                if (h0_(a1) == s_integer) { Expr *w = a1; a1 = a2; a2 = w; }
                if (h0_(a2) == s_integer &&
                    intval_(a2) >= 0 && intval_(a2) < (1 << 7))
                {   TypeExpr *t = prunetype(typeofexpr(a1));
                    if (h0_(t) == s_typespec) {
                        SET_BITMAP tm = typespecmap_(t);
                        if ((tm & (bitoftype_(s_char)|bitoftype_(s_signed))) ==
                                  (bitoftype_(s_char)|bitoftype_(s_signed)))
                        {   tm ^= (bitofstg_(s_signed)|bitofstg_(s_unsigned));
                            a1 = optimise1(mkcast(s_cast, a1, primtype_(tm)), YES);
                        }
                    }
                }
                arg1_(e) = a1;
                arg2_(e) = a2;
                break;
            }
        default:
            if (ismonad_(op))
                arg1_(e) = optimise1(arg1_(e), YES);
            else if (isdiad_(op))
                arg1_(e) = optimise1(arg1_(e), YES),
                arg2_(e) = optimise1(arg2_(e), YES);
            else syserr(syserr_optimise1, (long)op);
            break;
    }
    if (debugging(DEBUG_AETREE) && syserr_behaviour > 0) {
      if (op != s_return) {
        cc_msg("optimise1= "); pr_expr(e);
        cc_msg(" of type "); pr_typeexpr(typeofexpr(e), 0);
        cc_msg("\n");
      }
    }
    return e;
}

static void optimiselist(ExprList *x)
{   for (; x != 0; x = cdr_(x))
        exprcar_(x) = optimise1(exprcar_(x), YES);
}

#define U_Read 1
#define U_Write 2

typedef struct BindUseList BindUseList;
struct BindUseList {
     BindUseList *cdr;
     Binder *b;
     int use;
};

/* Very nice, AM had wanted to do something like this for some time!    */

static void warnifusage(BindUseList *bu, Binder *b, int flag) {
    for (; bu != NULL; bu = cdr_(bu))
        if (bu->b == b && (bu->use & flag)) {
            if (bu->use & flag & U_Read)
                cc_warn(warn_usage_rw, b);
            else
                cc_warn(warn_usage_ww, b);
            return;
        }
}

static BindUseList *mergeuselists(BindUseList *b1, BindUseList *b2, BindUseList *oldb) {
    if (b1 == oldb)
        return b2;
    else if (b2 != oldb) {
        BindUseList *p = b1, *next;
        for (; (next = cdr_(p)) != oldb; p = next) /* nothing */;
        cdr_(p) = b2;
     }
     return b1;
}

static BindUseList *checkvaruse(BindUseList *b, Expr *e) {
if (debugging(DEBUG_AETREE) && syserr_behaviour > 0) {
  cc_msg("checkvaruse "); pr_expr(e); cc_msg("\n");
}
    switch (h0_(e)) {
    default: if (h0_(e) > s_binder)
               cc_warn("syserr soon: checkvar use %ld", h0_(e));
    case s_addrof:              /* not if &a[b]; though! */
             break;
/* Hmm, I'd rather do via isdiad_ ismonad_, possible? */
    case s_binder:
        warnifusage(b, (Binder *)e, U_Write);
        b = (BindUseList *)syn_list3(b, e, U_Read);
        break;

    case s_andequal: case s_orequal: case s_xorequal:
    case s_timesequal: case s_plusequal: case s_minusequal: case s_divequal:
    case s_idivequal: case s_remequal:
    case s_leftshiftequal: case s_rightshiftequal:
    case s_assign:
        {   Expr *lhs = arg1_(e);
            b = checkvaruse(b, arg2_(e));
            if (h0_(lhs) != s_binder)
                b = checkvaruse(b, lhs);
            else {
                warnifusage(b, (Binder *)lhs, U_Write);
                b = (BindUseList *)syn_list3(b, lhs, U_Write);
            }
            break;
        }
    case s_plusplus: case s_postinc: case s_minusminus: case s_postdec:
        {   Expr *a1 = arg1_(e);
            if (h0_(a1) != s_binder)
                b = checkvaruse(b, a1);
            else {
                warnifusage(b, (Binder *)a1, U_Read+U_Write);
                b = (BindUseList *)syn_list3(b, a1, U_Write);
            }
            break;
        }
    case s_subscript:
    case s_equalequal: case s_notequal:
    case s_greater: case s_greaterequal: case s_less: case s_lessequal:
    case s_plus: case s_minus: case s_times: case s_div:
    case s_ptrdiff:
    case s_idiv: case s_rem: case s_power:
    case s_and: case s_or: case s_xor: case s_leftshift: case s_rightshift:
    case s_dotstar:
        b = checkvaruse(b, arg2_(e));
        /* drop though */

    case s_cast: case s_invisible:
    case s_return:
    case s_content:
    case s_monplus: case s_neg: case s_bitnot: case s_boolnot:
    case s_dot:
        b = checkvaruse(b, arg1_(e));
    case_s_any_string
        break;

    case s_let:
        b = checkvaruse(b, arg2_(e));
        break;

    case s_andand: case s_comma: case s_oror:
        {   BindUseList *b1 = checkvaruse(b, arg1_(e));
            BindUseList *b2 = checkvaruse(b, arg2_(e));
            b = mergeuselists(b1, b2, b);
            break;
        }
    case s_fnap:
        {   ExprList *args = exprfnargs_(e);
            for (; args != NULL; args = cdr_(args))
                b = checkvaruse(b, exprcar_(args));
            b = checkvaruse(b, arg1_(e));
            break;
        }
    case s_cond:
        {   BindUseList *b1 = checkvaruse(b, arg2_(e));
            BindUseList *b2 = checkvaruse(b, arg3_(e));
            BindUseList *b3 = checkvaruse(b, arg1_(e));
            b = mergeuselists(mergeuselists(b1, b2, b), b3, b);
            break;
        }
    }

if (debugging(DEBUG_AETREE) && syserr_behaviour > 0) {
  BindUseList *p;
  cc_msg("=>"); for (p = b; p != NULL; p = cdr_(p)) { cc_msg(" $b,%s",p->b, (p->use-1)*3+"R\0\0W\0\0RW"); }
  cc_msg("\n");
}
    return b;
}

Expr *optimise0(Expr *e)
/* exported - yields 0 if a 'serious' error message has already been
   printed (we know this by the s_error at the top of the tree).
   The semantic routines *SHOULD* all be strict in s_error.
*/
{   Expr *res;
    if (h0_(e) == s_error) return 0;
    checkvaruse(NULL, e);
    new_binders = 0;
    res = optimise2(optimise1(e, YES), 0);
    /*
     * If there are any structure returning functions allocate temp binders
     * here at the root of the expression tree.
     */
    return (new_binders == 0) ? res :
        mk_expr2(s_let, typeofexpr(res), (Expr*)new_binders, res);
}

/* The following routine detects when a struct/union has suitable members
   that it can be considered an integer and thus be slavable in a register.
   A first requirement is that all (union, total for struct) members are
   size 4 or less.  The total size of 4 has been checked by caller.
   However, not all such structs are suitable - consider
   struct { short a,b;}.  Moreover since C requires that the address of
   struct/union first element is the same as the address of the
   aggregate this poses problems (and that this problem extends to
   non-address-taken structs via assignment, this means that on some
   machines we cannot put  struct { char c; } in a register.
   However,  struct { int c:8;} is always OK.
   For now the rule is that every sub-object only contains int/enum/pointer.
*/
static bool integerlikestruct(TagBinder *b)
{   ClassMember *l;
    if (!integerlike_enabled) return 0;
    for (l = tagbindmems_(b); l != 0; l = memcdr_(l))
    {   TypeExpr *t;
#ifdef CPLUSPLUS
        if (!is_datamember_(l)) continue;
#endif
        t = prunetype(l->memtype);
        if (h0_(t) == t_fnap) continue;             /* C++ member func. */
        if (isbitfield_type(l->memtype))
        {   /* bits are OK (sem.c turns to int)                         */
            /* @@@ beware PCC mode/C++ char bitfields &c                */
        }
        else
        {   SET_BITMAP m;
            switch (h0_(t))
            {   default: return 0;                  /* array not OK     */
                case t_content: break;              /* pointers are OK  */
                case t_ref:     break;              /* and so are refs  */
                case t_fnap:    break;              /* so are mem fns   */
                case s_typespec:
                    m = typespecmap_(t);
                    switch (m & -m)
                    {   default: return 0;          /* includes char    */
                        case bitoftype_(s_int):
                        case bitoftype_(s_enum):
                            if (m & bitoftype_(s_short)) return 0;
                            break;                  /* 4 bit int ok     */
                        case bitoftype_(s_struct):
                        case bitoftype_(s_class):
                        case bitoftype_(s_union):
                            if (!integerlikestruct(typespectagbind_(t)))
                                return 0;
                            break;
                    }
                    break;
            }
        }
    }
    return 1;
}

static int32 mcrepofexpr1(Expr *e)
/* keep in step with sizeoftype */
{   TypeExpr *x = prunetype(typeofexpr(e));
    SET_BITMAP m;
    switch (h0_(x))
    {
case s_typespec:
        m = typespecmap_(x);
        switch (m & -m)    /* LSB - unsigned/long etc. are higher */
        {
    case bitoftype_(s_char):
            if ((m & (bitoftype_(s_signed)|bitoftype_(s_unsigned))) == 0)
                m |= (feature & FEATURE_SIGNED_CHAR) ?
                         bitoftype_(s_signed) : bitoftype_(s_unsigned);
            /* drop through */
    case bitoftype_(s_int):
    case bitoftype_(s_enum):
            {
                return sizeoftype(x) +
                    (m & bitoftype_(s_unsigned) ?
                     m & bitoftype_(s_signed) ? 0x4000000 : 0x1000000 : 0);
            }
    case bitoftype_(s_double):
            {   int32 n = sizeoftype(x);
/* The tests here generate no code if alignof_double==alignof_int.      */
                return (alignof_double > alignof_int && n == sizeof_double) ?
                       0x2000000 + MCR_ALIGN_DOUBLE + n : 0x2000000 + n;
            }
    case bitoftype_(s_struct):
    case bitoftype_(s_class):
    case bitoftype_(s_union):
            {   int32 n = sizeoftype(x);
                if (n == 4 && integerlikestruct(typespectagbind_(x)))
                    return 0x0000000 + n;
                else
                    return (alignof_double > alignof_int &&
                            alignoftype(x) == alignof_double) ?
                           0x3000000 + MCR_ALIGN_DOUBLE + n : 0x3000000 + n;
            }
    case bitoftype_(s_void):
                /*
                 * BEWARE: Other parts of the compiler use mcrepoftype==0
                 * as a test for 'void' types.
                 */
                return 0x0000000;
    default: break;
            }
            /* drop through */
default:
/*      case t_fnap: */
            syserr(syserr_mcrepofexpr, (long)h0_(x),(long)typespecmap_(x));
            return 0x0000000 + sizeof_ptr;
case t_subscript:
/* The following checks that restriction that mcrepofexpr() spiritually     */
/* should never be applied to array typed expressions.  Two exceptions:     */
/* s_binder's (for flowgraf.c sizing) and s_strings (optimise removes their */
/* implicit '&').                                                           */
            if (h0_(e)!=s_binder && !isstring_(h0_(e)))
                syserr(syserr_mcreparray, (long)h0_(e));
            if (h0_(e) == s_binder)
            {   int32 n = sizeoftype(x);
                return (alignof_double > alignof_int &&
                        alignoftype(x) == alignof_double) ?
                       0x3000000 + MCR_ALIGN_DOUBLE + n : 0x3000000 + n;
            }
            /* s_string falls into pointer code */
case t_content:
case t_ref:
            return TARGET_ADDRESSES_UNSIGNED ? 0x1000000 + sizeof_ptr :
                                               0x0000000 + sizeof_ptr;
    }
}

int32 mcrepofexpr(Expr *e)
{
#ifdef CPLUSPLUS
    /* temp. nasty hack for optimise(s_dot/s_dotstar) and cppfe/vargen  */
    int32 r = e == (Expr *)datasegment ? 0x03000000 : mcrepofexpr1(e);
#else
    int32 r = mcrepofexpr1(e);
#endif
#ifdef SOFTWARE_FLOATING_POINT
/* Fake floating point values so that they are thought of as structures. */
    if (r == 0x02000004)
      return 0x00000004; 
    else if (r == 0x02000008 || r == 0x02000008+MCR_ALIGN_DOUBLE)
      return 0x03000008;
#endif
    return r;
}

int32 mcrepoftype(TypeExpr *t)
{
/* This returns the machine representation of a type. Done by forgery of */
/* an expression and a call to mcrepofexpr().                            */
    return mcrepofexpr(mk_expr2(s_invisible, t, 0, 0));
}

/* end of simplify.c */
