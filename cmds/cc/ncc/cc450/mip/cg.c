/*
 * C compiler file cg.c
 * Copyright (C) Codemist Ltd, 1988.
 * Copyright (C) Acorn Computers Ltd., 1988.
 */

/*
 * RCS $Revision: 1.6 $ Codemist 116
 * Checkin $Date: 1993/07/27 09:46:25 $
 * Revising $Author: nickc $
 */

/* AM memo: remove smashes to tree for long-term safety (e.g. find s_neg)  */
/* AM memo: bug in cse/deadcode kills fp voided div.                       */
/* AM Mar 89: Add s_wstring (treated here as s_string).  Memo: later add   */
/*            J_WSTRING similar to J_STRING for cross sex compilation.     */
/* AM Nov 88: Remove some (only!!!) magic numbers on mcreps for optional   */
/* double alignment.                                                       */
/* Memo: tidy up the current_/greatest_stackdepth edifice.                 */

/* see cg_exprreg() for half developed code to optimise the JOP            */
/* code to save space (and possibly time in regalloc).                     */

/* Note to reader:  the order of routines in here is extremely unnatural */
/* (hence the large number of inane 'static' forward references below).  */
/* It is so that the largest routine gets compiled first to save         */
/* (compile-time) space.  Yuk.  Undoes rationalisation of 4/12/86.       */

/*
 * ACN Feb 90: rework struct args support and '...' for 88000 etc.
 * AM aug 87:  improve the code for x.a++ etc, following ACN's LDRVK.
 * ACN aug 87  add 'valof' block extension.  Buggy for structs still.
 * AM 20-jun-87 fix bug in cg_cond on structs. fix fp/int reg confusion
 *              if registers active at cg_fnap.  Redo cg_fnargs removing
 *              50 limit on arg count, and improving code.
 * AM 26-may-87 redo CASEBRANCH.  Re-use store later.
 * AM 22-apr-87 fix bug in loadzero resulting in f()*0 doing nothing.
 * AM 27-dec-86 require just 2 free fpregs in cg_expr (why can't one work?)
 * AM 4-dec-86 put struct/unions of size 4 to regs, fix bug in f().a.
 * AM 4-dec-86 rationalisation(!) of order - i.e. Cmd stuff here.
 * AM 3-dec-86 separate 'C' case into s_break and s_endcase.
 */

#include "globals.h"
#include "cg.h"
#include "store.h"
#include "codebuf.h"
#include "aeops.h"
#include "util.h"
#include "xrefs.h"
#include "jopcode.h"
#include "regalloc.h"
#include "regsets.h"
#include "cse.h"
#include "flowgraf.h"
#include "mcdep.h"
#include "aetree.h"
#include "builtin.h"
#include "sem.h"       /* typeofexpr */
#include "simplify.h"  /* mcrepofexpr */
#include "bind.h"
#include "errors.h"

/* The next routine is a nasty hack -- see its uses.   */
/* It copies a SynAlloc BindList into a BindAlloc one. */
static SynBindList *binderise(SynBindList *l)
{   SynBindList *f1 = NULL;
    for (; l != NULL; l = l->bindlistcdr)
        f1 = (SynBindList *)mkBindList(f1, l->bindlistcar);
    return (SynBindList *)dreverse((List *)f1);
}

#define NOTALABEL   ((LabelNumber *)DUFF_ADDR)
#define NOTINLOOP   NOTALABEL
#define NOTINSWITCH NOTALABEL

static int32 max_icode, max_block;          /* statistics (lies, damn lies) */

static Cmd *cg_current_cmd;
static int32 current_stackdepth;
int32 greatest_stackdepth;                  /* needed for stack check code */
int32 max_argsize;                          /* for regalloc.c              */
int32 procflags;                            /* see jopcode.h               */
int32 procauxflags;
int32 cg_fnname_offset_in_codeseg;          /* for xxx/gen.c               */
                                            /* (actually used by s370/gen  */
static BindList *local_binders, *regvar_binders;
static Binder *integer_binder, *double_pad_binder;
bool has_main;
static bool defining_main;
static bool cg_infobodyflag;

static struct SwitchInfo {
    BindList *binders;            /* 'endcase' may be non-local goto */
    LabelNumber *defaultlab, *endcaselab;
} switchinfo;

static struct LoopInfo {
    BindList *binders;  /* 'break', 'continue' may be non-local goto */
    LabelNumber *breaklab, *contlab;
} loopinfo;

#ifdef EXTENSION_VALOF
static struct ValofInfo {
    BindList *binders;
    LabelNumber *lab;
    VRegnum r;
} valofinfo;
#endif

typedef struct CasePair { int32 caseval; LabelNumber *caselab;} CasePair;

static VRegnum cg_expr1(Expr *x,bool valneeded);
#ifndef ADDRESS_REG_STUFF
#define ensure_regtype(r,rsort) (r)
#endif
static VRegnum cg_stind(VRegnum r, Expr *x, const int32 flag, bool topalign,
                    const int32 mcmode, const int32 mclength,
                    bool address, bool volatilep);
static VRegnum cg_var(VRegnum r, Binder *b, AEop flag,
                      int32 mcmode, int32 mclength, bool address);
static void bfreeregister(VRegnum r);
static VRegnum fgetregister(RegSort rsort);
static VRegnum reserveregister(RegSort precision);
static VRegnum getreservedreg(VRegnum r);
static void cg_bindlist(SynBindList *x,bool initflag);
static Binder *is_local_adcon(Expr *a);
static bool is_same(Expr *a,Expr *b);
static Expr *take_address(Expr *e);
static VRegnum open_compilable(Expr **xp,RegSort rsort);
static VRegnum cg_fnap(Expr *x,VRegnum resreg,bool valneeded);
static VRegnum cg_cond(Expr *c,bool valneeded,VRegnum targetreg,
                       LabelNumber *l3,bool structload);
static VRegnum cg_cast1(Expr *x1,int32 mclength,int32 mcmode);
static VRegnum cg_addr(Expr *sv,bool valneeded);
static VRegnum cg_storein(VRegnum r,Expr *e,AEop flag);
#ifdef EXTENSION_VALOF
static void cg_cmd(Cmd *x);
#endif
static void structure_assign(Expr *lhs,Expr *rhs,int32 length);
static void verify_integer(Expr *x);
static bool structure_function_value(Expr *x);
static VRegnum cg_binary(J_OPCODE op,Expr *a1,Expr *a2,bool commutesp,
                         RegSort fpp);
static VRegnum cg_binary_or_fn(J_OPCODE op,TypeExpr *type,Expr *fname,
                               Expr *a1,Expr *a2,bool commutesp);
static bool iszero(Expr *x);
static bool isone(Expr *x);
static bool isminusone(Expr *x);
static int32 ispoweroftwo(Expr *x);
static VRegnum cg_loadconst(int32 n,Expr *e);
static int nastiness(Expr *x);
static void cg_count(FileLine fl);
static void cg_return(Expr *x, bool implicitinvaluefn);
static void cg_loop(Expr *init, Expr *pretest, Expr *step, Cmd *body,
                    Expr *posttest);
static void cg_test(Expr *x, bool branchtrue, LabelNumber *dest);
static void casebranch(VRegnum r, CasePair *v, int32 ncases,
                       LabelNumber *defaultlab);
static void cg_case_or_default(LabelNumber *l1);
static void cg_condjump(J_OPCODE op,Expr *a1,Expr *a2,RegSort rsort,J_OPCODE cond,LabelNumber *dest);
#if defined TARGET_HAS_SCALED_ADDRESSING || defined TARGET_HAS_SCALED_OPS
  static int32 is_shifted(Expr *x, int32 mclength);
  static Expr *shift_operand(Expr *x);
  static int32 shift_amount(Expr *x);
#endif
static void emituse(VRegnum r,RegSort rsort);
static VRegnum load_integer_structure(Expr *e);

/* result_variable is an extra first arg. for use with structure returning  */
/* functions - private to cg_return and cg_topdecl                          */
static Binder *result_variable, *result_temporary;
static LabelNumber *structretlab;

#define lowerbits(n) (((int32)1<<(n))-1)

/*************************************************************************/
/*                 Start of codegeneration proper.                       */
/*************************************************************************/

#define unsigned_expression_(x) \
        ((mcrepofexpr(x) >> MCR_SORT_SHIFT) == 1)

static RegList *usedregs, *usedfpregs;
static int32 nusedregs, nusedfpregs, spareregs, sparefpregs, nreservedregs;

static BindList *datasegbinders;

#define NOT_OPEN_COMPILABLE ((VRegnum)(-2))
/* Temp - real distinguished non-GAP VRegnum wanted */
#define cg_loadzero(e) cg_loadconst(0,e)

#ifdef TARGET_HAS_DIVIDE
#define cg_divrem(op,type,fname,a1,a2) \
        cg_binary(op,a1,a2,0,INTREG)
#else
#define cg_divrem(op,type,fname,a1,a2) \
        cg_binary_or_fn(op,type,fname,a1,a2,0)
#endif

/* values for nastiness() */
#define ISCONST  0x100   /* assumed minimum for nastiness()              */
#define ISXCONST 0x101   /* floating, string, (addrof one day -- ask AM) */
#define ISBIND   0x102
#define ISEXPR   0x103
/* ISHARD *MUST BE* the maximum nastiness() can return:                  */
/* such expressions have function applications in them -- allows 2^10    */
/* terms in an expression (without fn calls) before we have to truncate  */
/* to avoid hitting the ISHARD ceiling.   See nastiness().               */
#define ISHARD   (ISEXPR+10)   /* assumed maximum for nastiness()        */

static int32 result2;

static bool integer_constant(Expr *x)
{
/* Test if x is an integer constant, and if it is leave its value in result2 */
    if (h0_(x)==s_integer)
    {   result2 = intval_(x);
        return YES;
    }
    return NO;
}

static Expr *cg_content_for_dot(Expr *x)
{
/* Of course, mcrepofexpr(e) = 4 implies that selections from e will have */
/* offset = 0, but simplify may produce a selection outside e (in turning */
/* *(a+n).k into *a.(n+k)).                                               */
    if (mcrepofexpr(arg1_(x)) == 0x0000004 && exprdotoff_(x) == 0 &&
        !isvolatile_expr(x))
        return arg1_(x);
    else
        return mk_expr1(s_content, type_(x), take_address(x));
}


static VRegnum cg_exprvoid(Expr *x)
{
    return cg_expr1(x, NO);
}

static VRegnum cg_expr(Expr *x)
{
    return cg_expr1(x, YES);
}

static VRegnum cg_diadvoid(Expr *x)
{   cg_expr1(arg1_(x), NO);
    return cg_expr1(arg2_(x), NO);
}

static VRegnum cg_multiply(TypeExpr *type, Expr *a1, Expr *a2)
{
  if (config & CONFIG_HAS_MULTIPLY) {
    return cg_binary(J_MULR, a1, a2, 1, INTREG);
  } else {
    return cg_binary_or_fn(J_MULR, type, sim.mulfn, a1, a2, 1);
  }
}

static VRegnum cg_unary_i(J_OPCODE op, RegSort rsort, Expr *x)
{
    VRegnum r = fgetregister(rsort);
    VRegnum r1;
#ifdef TARGET_HAS_SCALED_OPS
    if (op == J_NOTR && is_shifted(x, 0)) {
        r1 = cg_expr(shift_operand(x));
        emit5(op, r, GAP, r1, shift_amount(x));
    } else
#endif
    {   r1 = cg_expr(x);
        emitreg(op, r, GAP, r1);
    }
    bfreeregister(r1);
    return r;
}

static J_OPCODE floatyop(RegSort rsort, J_OPCODE j_i, J_OPCODE j_f, J_OPCODE j_d) {
    return rsort == DBLREG ? j_d :
           rsort == FLTREG ? j_f :
                             j_i;
}

/* Maybe the following routine will subsume cg_expr2 one day.          */
static VRegnum cg_exprreg(Expr *x, VRegnum r)
/* Used to save (virtual) registers for simple expressions which must  */
/* then be moved to a specified register (e.g. cond/fn_arg/fn_result). */
        /* Note that targetreg here is only 'reserved' not 'got'.  The  */
        /* caller of cg_cond will getreservedreg() on return to avoid   */
        /* overestimating temporary register use.                       */
{   switch (h0_(x))
    {
case s_integer:
        if (usrdbg(DBG_LINE) && x->fileline != 0)
            emitfl(J_INFOLINE, *x->fileline);
        emit(J_MOVK, r, GAP, intval_(x));
        break;
case_s_any_string
        emitstring(J_STRING, r, ((String *)x)->strseg);
        break;
#ifdef EVEN_FINER_DAY
case s_binder: ...
        break;
#endif
case s_cond:
        {   LabelNumber *l3 = nextlabel();
/* Previous phases of the compiler must have arranged that the two arms  */
/* of the condition each have the same mode. In particular they must     */
/* either be both integer or both floating point values. This has to     */
/* include the possibility of them being voided. Structure values are    */
/* not legal here.                                                       */
            (void)cg_cond(x, 1, r, l3, 0);
            start_new_basic_block(l3);
        }
        break;
/* n.b. do not use with with s_addrof due to loadadcon() slaving */
default:
        {   VRegnum r2 = cg_expr1(x, 1);
            RegSort rsort = vregsort(r);
            emitreg(floatyop(rsort, J_MOVR, J_MOVFR, J_MOVDR), r, GAP, r2);
            bfreeregister(r2);
        }
        break;
    }
    return r;   /* may be useful */
}

#ifdef TARGET_HAS_DIVIDE
#ifdef TARGET_LACKS_REMAINDER

/* Use this if the target has divide but not remainder opcodes */
static VRegnum simulate_remainder(TypeExpr *t, Expr *a1, Expr *a2)
{
    Binder *gen1 = gentempbinder(t);
    Binder *gen2 = gentempbinder(t);
/* { int a1= arg1, a2 = arg2;
     a1 - (a1/a2)*a2;
   }
*/
    Expr *x = mk_exprlet(s_let, t,
        mkSynBindList(mkSynBindList(0, gen1), gen2),
        mk_expr2(s_comma, t,
            mk_expr2(s_assign, t,
                (Expr *)gen1,
                a1),
            mk_expr2(s_comma, t,
                mk_expr2(s_assign, t,
                    (Expr *)gen2,
                    a2),
                mk_expr2(s_minus, t,
                    (Expr *)gen1,
                    mk_expr2(s_times, t,
                        mk_expr2(s_div, t, (Expr *)gen1, (Expr *)gen2),
                        (Expr *)gen2)))));
    return cg_expr(x);
}

#endif
#endif

#ifdef ADDRESS_REG_STUFF
static VRegnum ensure_regtype(VRegnum r, RegSort rsort)
{
   if (vregsort(r) != rsort)
   {  if (rsort == ADDRREG)
      {  VRegnum r1 = fgetregister(rsort);
         emitreg(J_MOVR, r1, GAP, r);
         bfreeregister(r);
         r = r1;
      }
      else if (rsort == INTREG);
         /* do nothing -- AM presumes that this is because the          */
         /* back end can always use a D as an A but not vice-versa.     */
         /* Or is it the other way round?                               */
      else
         syserr(syserr_regtype, (long)rsort);
   }
   return r;
}
#endif

static VRegnum cg_loadfpzero(RegSort rsort, Expr *e)
{   /* void e if !=NULL and return 0 - used for things like f()*0     */
    VRegnum r;
    if (e) (void)cg_exprvoid(e);
    r = fgetregister(rsort);
    if (rsort == FLTREG)
        emitfloat(J_MOVFK, r, GAP, fc_zero.s);
    else
        emitfloat(J_MOVDK, r, GAP, fc_zero.d);
    return r;
}

#ifdef RANGECHECK_SUPPORTED
/* It would seem that this routine is a special case of cg_binary_1().  */
static void boundcheck(J_OPCODE op, VRegnum r, Expr *x)
{
    if (x == NULL)
        /* nothing */;
    else if (integer_constant(x))
        emit(J_RTOK(op), GAP, r, result2);
    else {
        VRegnum r1 = cg_expr(x);
        emitreg(op, GAP, r, r1);
        bfreeregister(r1);
    }
}
#endif

static bool returnsstructinregs(Expr *fn) {
    TypeExpr *t = prunetype(typeofexpr(fn));
    if (h0_(t) != s_content) return NO; /* syserr will follow */
    t = typearg_(t);
    if (typefnaux_(t).flags & bitoffnaux_(s_structreg)) {
        int32 resultwords = sizeoftype(typearg_(t)) / sizeof_long;
        return (resultwords > 1 && resultwords <= NARGREGS);
    }
    return NO;
}

/* note that 'valneeded' and 'RegSort rsort' really tell similar stories */
/* Maybe a VOIDREG version of RegSort subsumes both                      */
static VRegnum cg_expr2(Expr *x, bool valneeded)
{
    AEop op;
    VRegnum r, r1;
    int32 mclength = mcrepofexpr(x);
    int32 mcmode = mclength >> MCR_SORT_SHIFT;
    RegSort rsort = (mclength &= MCR_SIZE_MASK,
         (mcmode!=2) ? INTREG : (mclength==4 ? FLTREG : DBLREG));
#ifdef ADDRESS_REG_STUFF
    if( rsort == INTREG )
    {
/* Work out further if the result is to be used as an address            */
/* If it is the specialise further to an ADDRREG                         */
      TypeExpr *te = princtype(typeofexpr(x));
      /* The t_subscript case is to include strings which       */
      /* have had their implicit '&' removed by simplify().     */
      if (h0_(te) == t_content || h0_(te) == t_subscript)
         rsort = ADDRREG;
    }
#endif

/* The next line will not catch cases where loading a one-word struct    */
/* occurs improperly if that gets treated as an integer. I have to let   */
/* this case slip through since in places I really want to treat such    */
/* structs as ints. Anyway all I lose is an internal check that should   */
/* never really fail anyway.                                             */
    if (mcmode==3 && valneeded)
        syserr(syserr_struct_val);
    if (x == 0) { syserr(syserr_missing_expr); return GAP; }

    op = h0_(x);
    if (usrdbg(DBG_LINE) && hasfileline_(op) && x->fileline != 0)
        emitfl(J_INFOLINE, *x->fileline);

    switch (op)
    {
case s_binder:
            if (!valneeded && !isvolatile_expr(x)) return GAP;
            r = ensure_regtype(cg_var(GAP, (Binder *)x, s_content,
                                      mcmode, mclength, rsort==ADDRREG),
                               rsort);
            if (!valneeded) bfreeregister(r), r = GAP;
            return r;

case s_integer:
            if (!valneeded) return GAP;
#ifdef TARGET_R0_ALWAYS_ZERO
            if (intval_(x) == 0) return virtreg(0,INTREG);
#endif
            emit(J_MOVK, r = fgetregister(rsort), GAP, intval_(x));
            return r;

case s_floatcon:
            if (!valneeded) return GAP;
#ifdef SOFTWARE_FLOATING_POINT
            if (rsort != DBLREG) /* single precision values treated as ints */
            {   emit(J_MOVK, r = fgetregister(INTREG), GAP,
                             ((FloatCon *)x)->floatbin.irep[0]);
                return r;
            }
#endif
            r = fgetregister(rsort);
            emitfloat(rsort==DBLREG ? J_MOVDK:J_MOVFK, r, GAP, (FloatCon *)x);
            return r;

case_s_any_string
            if (!valneeded) return GAP;
            emitstring(J_STRING, r=fgetregister(ADDRREG),
                                 ((String *)x)->strseg);
            return r;

#ifdef EXTENSION_VALOF
case s_valof:
            {   struct ValofInfo saver;

                saver = valofinfo;

                valofinfo.binders = active_binders;
                valofinfo.lab = nextlabel();
                valofinfo.r = reserveregister(rsort);
            /* Valof blocks will not work (a syserr will occur via cg_addr */
            /* or cg_exprreg) for structure results and would need code    */
            /* here and at s_valof (and cg_addr &c) to make them work.     */
                cg_cmd(expr1c_(x));
                r = getreservedreg(valofinfo.r);
                /* The next line does nothing much but waste space usually */
                /* but is needed in case the valof may (or appear to) have */
                /* a route through without a resultis.                     */
                /* See the other uses of J_INIT.                           */
                emitbinder(floatyop(rsort, J_INIT, J_INITF, J_INITD),
                           r, gentempvar(type_(x), r));
                start_new_basic_block(valofinfo.lab);
                valofinfo = saver;
                return r;
            }
#endif

#ifdef RANGECHECK_SUPPORTED
case s_rangecheck:
            /* rangecheck  i, l, u: check that l<=i<=u, taking
               target-dependent action if not.
               Either l or u may be NULL, meaning don't check.
               value is i.
             */
            {   r = cg_expr2(arg1_(x), YES);
                boundcheck(J_CHKLR, r, arg2_(x));
                boundcheck(J_CHKUR, r, arg3_(x));
                return r;
            }

case s_checknot:
            /* check i, k: check that the value of i is not k,
               taking target-dependent action if not.
             */
            {   r = cg_expr2(arg1_(x), YES);
                if (integer_constant(arg2_(x)))
                    emit(J_CHKNEK, GAP, r, result2);
                else if (isintregtype_(rsort))
                    syserr(syserr_checknot);
                else {
                    VRegnum r2 = cg_expr2(arg2_(x), YES);
                    emitreg(rsort==FLTREG ? J_CHKNEFR: J_CHKNEDR,
                             GAP, r, r2);
                    bfreeregister(r2);
                }
                return r;
            }
#endif

case s_let:
            {   BindList *sl = active_binders;
                int32 d = current_stackdepth;
                cg_bindlist(exprletbind_(x), 0);
                r = cg_expr1(arg2_(x), valneeded);
                emitsetsp(J_SETSPENV, sl);
                current_stackdepth = d;
            }
            return r;
case s_fnap:
            if (mcmode == 3)
/* In a void context I am allowed to call a function that returns a      */
/* structure value. Here I have to make room on that stack for that      */
/* value to get dumped.                                                  */
            {   if (!valneeded && returnsstructinregs(arg1_(x)))
                    x = mk_expr2(
                            s_fnapstructvoid,
                            te_void,
                            arg1_(x),
                            (Expr *)mkExprList(
                                exprfnargs_(x),
                                NULL));
                else {
                    TypeExpr *t = type_(x);
                    Binder *gen = gentempbinder(t);
                    bindstg_(gen) |= b_addrof;
                    x = mk_exprlet(
                            s_let,
                            t,
                            mkSynBindList(0, gen),
                            mk_expr2(
                                s_comma,
                                t,
                                mk_expr2(
                                    valneeded ? s_fnapstruct : s_fnapstructvoid,
                                    te_void,
                                    arg1_(x),
                                    (Expr *)mkExprList(
                                        exprfnargs_(x),
                                        take_address((Expr *)gen))),
/* This last item in the comma expression is only useful if the value of */
/* this (structure) function call is required. This would not normally   */
/* be a possibility, since structure-values can only occur in rather     */
/* special places and in general cg_expr() is not allowed to load one.   */
/* But one-word structure returning functions being called to provide    */
/* arguments for other functions can drop me into here on account of the */
/* punning between these structures and integer values.                  */
                                (Expr *)gen));
                }
                if (debugging(DEBUG_CG))
                {   eprintf("Structure fn call -> ");
                    pr_expr(x);
                }
                return cg_expr1(x, valneeded);
            }
            /* drop through */
/*
 * s_fnapstruct is an artefact introduced when a structure returning
 * function call is mapped from
 *        v = f(a,b);
 * onto   (void)f(&v,a,b);
 * so that the converted function application is marked as having been
 * subjected to this transformation.  This is so that cg_fnap can (for
 * some machines) pass the implicit extra address in some way other than
 * making it a new first argument.  Corresponding special treatment
 * is needed in function definitions - here that is achieved by inspection
 * of result_variable (NULL if not used) with funny treatment only
 * activated if TARGET_STRUCT_RESULT_REGISTER is defined.
 */
case s_fnapstructvoid:
case s_fnapstruct:
/* Some functions have special rules re compilation. open_compilable()   */
/* returns -2 if it is handed anything other than one of these.          */
/* NB open_compilable is now handed a pointer to the fnap expression,    */
/* for which it may generate a replacement to be given to cg_fnap        */
            r = open_compilable(&x, rsort);
            return (r != NOT_OPEN_COMPILABLE) ? r :
                    cg_fnap(x, V_resultreg(rsort), valneeded);
case s_cond:
            {   LabelNumber *l3 = nextlabel();
/* Previous phases of the compiler must have arranged that the two arms  */
/* of the condition each have the same mode. In particular they must     */
/* either be both integer or both floating point values. This has to     */
/* include the possibility of them being voided. Structure values are    */
/* only legal here if the conditional expression as a whole is voided.   */
                r = cg_cond(x, valneeded, reserveregister(rsort), l3, 0);
                start_new_basic_block(l3);
                return getreservedreg(r);
            }

case s_cast:
            return cg_cast1(arg1_(x), mclength, mcmode);
case s_addrof:
            return cg_addr(arg1_(x), valneeded);

case s_equalequal:          /* (a==b)  -->  ((a==b) ? 1 : 0)             */
case s_notequal:            /* and similarly for these others.           */
case s_greater:
case s_greaterequal:
case s_less:
case s_lessequal:
            if (!valneeded) return cg_diadvoid(x);

            /* boolnot is unary, but is treated the same */
case s_boolnot:
            if (!valneeded) return cg_exprvoid(arg1_(x));

case s_andand:
case s_oror:
            x = mk_expr3(s_cond, te_int,
                                 x,
                                 mkintconst(te_int, 1, 0),
                                 mkintconst(te_int, 0, 0));
            return cg_expr(x);

case s_comma:
            cg_exprvoid(arg1_(x));
            return cg_expr1(arg2_(x), valneeded);

case s_assign:
            if (mcmode == 3)
            {
                if (valneeded)
                    syserr(syserr_structassign_val);
                else
                {   structure_assign(arg1_(x), arg2_(x), mclength);
                    return GAP;
                }
            }
            goto ass_disp;
case s_displace:
            {   Expr *v = arg1_(x), *x3 = arg2_(x);
                if (valneeded && isintregtype_(rsort) && h0_(x3) == s_plus &&
                    is_same(arg1_(x3),v) && integer_constant(arg2_(x3)))
                {   /* a little optimisation... */
                    int32 n = result2;
                    VRegnum rx = cg_expr(v);
                    VRegnum r1 = fgetregister(ADDRREG);
                    emit(J_ADDK, r1, rx, n);
                    cg_storein(r1, v, s_assign);
                    bfreeregister(r1);
                    return rx;
                }
            }
ass_disp:
            return cg_storein(cg_expr(arg2_(x)), arg1_(x),
                              valneeded ? op : s_assign);

case s_dot:
        {   Expr *arg1 = arg1_(x);
            x = cg_content_for_dot(x);
/* Assert: x == arg1 => integerlikestruct. cg_content_for_dot () generates  */
/*         the correct code either way.                                     */
/* NOTE: ugly implicit collusion with simplify.c and cg_content_for_dot().  */
            if (x != arg1 && structure_function_value(arg1))
            {   syserr(syserr_structdot);
                return GAP;
            }
            return cg_expr1(x, valneeded);
        }

case s_content:
        {   bool volatilep = isvolatile_expr(x);
            if (!valneeded && !volatilep) return cg_exprvoid(arg1_(x));
            x = arg1_(x);
            verify_integer(x);
            if (memory_access_checks)
            {   Expr *fname =
                     mclength==1 ? sim.readcheck1 :
                     mclength==2 ? sim.readcheck2 :
                                   sim.readcheck4;
                x = mk_expr2(s_fnap, typeofexpr(x), fname,
                                                    (Expr *)mkExprList(0, x));
            }
            r = ensure_regtype(cg_stind(GAP, x, s_content, NO, mcmode, mclength,
                                        rsort==ADDRREG, volatilep),
                               rsort);
            if (!valneeded) bfreeregister(r), r = GAP;
            return r;
        }

case s_monplus:
/* Monadic plus does not have to generate any code                       */
            return cg_expr1(arg1_(x), valneeded);

case s_neg:
            if (!valneeded) return cg_exprvoid(arg1_(x));
            if (isintregtype_(rsort))
                return cg_unary_i(J_NEGR, rsort, arg1_(x));
            r1 = cg_expr(arg1_(x));
            r = fgetregister(rsort);
            emitreg((rsort==FLTREG ? J_NEGFR : J_NEGDR), r, GAP, r1);
            bfreeregister(r1);
            return r;

case s_bitnot:
            if (!valneeded) return cg_exprvoid(arg1_(x));
            verify_integer(x);
            return cg_unary_i(J_NOTR, rsort, arg1_(x));

case s_times:
        if (!valneeded) return cg_diadvoid(x);
        else if (!isintregtype_(rsort)) {
            if (is_fpzero(arg1_(x))) return cg_loadfpzero(rsort, arg2_(x));
            if (is_fpzero(arg2_(x))) return cg_loadfpzero(rsort, arg1_(x));
            return(cg_binary(rsort==FLTREG ? J_MULFR : J_MULDR,
                             arg1_(x), arg2_(x), 1, rsort));
        }
        if (iszero(arg1_(x))) return cg_loadzero(arg2_(x));
        if (iszero(arg2_(x))) return cg_loadzero(arg1_(x));
        else if (isone(arg1_(x))) return(cg_expr(arg2_(x)));
        else if (isone(arg2_(x))) return(cg_expr(arg1_(x)));
        else if (isminusone(arg1_(x)))
        {   arg1_(x) = arg2_(x);
            h0_(x) = s_neg;
            return(cg_expr(x));
        }
        else if (isminusone(arg2_(x)))
        {   h0_(x) = s_neg;
            return(cg_expr(x));
        }
        else
        {   int32 p;
            if ((p = ispoweroftwo(arg2_(x))) != 0)
                /* change to SIGNED shift if mcmode=0 and overflow checked */
                return cg_binary(J_SHLR+J_UNSIGNED, arg1_(x),
                                 mkintconst(te_int,p,0), 0, rsort);
            if ((p = ispoweroftwo(arg1_(x))) != 0)
                /* change to SIGNED shift if mcmode=0 and overflow checked */
                return cg_binary(J_SHLR+J_UNSIGNED, arg2_(x),
                                 mkintconst(te_int,p,0), 0, rsort);
            return cg_multiply(type_(x), arg1_(x), arg2_(x));
        }

case s_plus:
        if (!valneeded) return cg_diadvoid(x);
        {   Expr *a1 = arg1_(x), *a2 = arg2_(x);
            /* Code that used to be here to turn a+-b into a-b etc now    */
            /* resides in simplify.c (where it also gets argument of      */
            /* s_content                                                  */
            if (!isintregtype_(rsort))
                return(cg_binary(rsort==FLTREG ? J_ADDFR : J_ADDDR,
                                 a1, a2, 1, rsort));
            if (iszero(a1)) return cg_expr(a2);
            else if (iszero(a2)) return cg_expr(a1);
            else return(cg_binary(J_ADDR, a1, a2, 1, rsort));
        }

case s_minus:
        if (!valneeded) return cg_diadvoid(x);
        {   Expr *a1 = arg1_(x), *a2 = arg2_(x);
            if (!isintregtype_(rsort))
                return(cg_binary(rsort==FLTREG ? J_SUBFR : J_SUBDR,
                                 a1, a2, 0, rsort));
            if (iszero(a1))
            {   arg1_(x) = a2;
                h0_(x) = s_neg;
                return(cg_expr(x));
            }
            else if (iszero(a2)) return(cg_expr(a1));
            else return(cg_binary(J_SUBR, a1, a2, 0, rsort));
        }

case s_div:
/* Even if voiding this I calculate it in case there is a division error */
/* that ought to be reported.                                            */
/* But I arrange that the numerator is voided in this odd case, since    */
/* that can save me some effort.                                         */
        if (!valneeded)
        {   cg_exprvoid(arg1_(x));
            x = mk_expr2(s_div, type_(x), mkintconst(te_int, 1, 0), arg2_(x));
        }
        if (iszero(arg1_(x))) r = cg_loadzero(arg2_(x));
        else if (!isintregtype_(rsort))
/* @@@ AM: tests like the following can probably be hit with a feature  */
/* bit within the JOPCODE property table when AM re-arranges them.      */
#ifdef TARGET_LACKS_FP_DIVIDE
/* N.B. in this version I make (p/q) turn into divide(q,p) since that    */
/* seems to make register usage behave better (see cg_binary_or_fn)      */
            r = cg_expr(mk_expr2(s_fnap, type_(x),
		 /*
		  * XXX - NC - 29/8/91 - BUG FIX
		  *
		  * changed the expression below to read
		  *   'sim.fdivfn' and 'sim.ddivfn'
		  * rather than
		  *   'sim.fdiv'   and 'sim.ddiv'
		  * as these field names appear to have changed
		  */
                        rsort==FLTREG ? sim.fdivfn : sim.ddivfn,
                        (Expr *)mkExprList(mkExprList(0, arg1_(x)), arg2_(x)));
#else
            r = cg_binary(rsort==FLTREG ? J_DIVFR : J_DIVDR,
                             arg1_(x), arg2_(x), 0, rsort);
#endif
        else if (isone(arg2_(x))) r = cg_expr(arg1_(x));
/* can't the unsignedness property get in rsort? */
        else if (mcmode==1)
        {   int32 p;
            r = ((p = ispoweroftwo(arg2_(x))) != 0) ?
                cg_binary(J_SHRR+J_UNSIGNED, arg1_(x),
                          mkintconst(te_int,p,0), 0, rsort) :
                cg_divrem(J_DIVR+J_UNSIGNED, type_(x), sim.udivfn,
                          arg1_(x), arg2_(x));
        }
        else
        {
#if !defined(TARGET_HAS_DIVIDE) && !defined(TARGET_HAS_NONFORTRAN_DIVIDE)
            int32 p;
            if ((p = ispoweroftwo(arg2_(x))) != 0)
            {   /* e.g. (signed)  z/8 == (z>=0 ? z:z+7) >> 3 (even MIN_INT) */
                /* Do not forge such an expression since (a) we cannot      */
                /* re-use VRegs as we do below (this saves a resource AND   */
                /* forces and MOVR first which can often be combined with   */
                /* the zero test) and (b) profile counting is unwanted here.*/
                /* This code may be better as a procedure.                  */
                r = cg_expr(arg1_(x));
#ifdef TARGET_HAS_SCALED_OPS         /* even more cunning trick for n/2 ... */
                if (p == 1)
                    emit5(J_ADDR, r, r, r, SHIFT_RIGHT | 31);   /* unsigned */
                else
#endif
                {   LabelNumber *l = nextlabel();
                    blkflags_(bottom_block) |= BLKREXPORTED;
                    emit(J_CMPK+Q_GE, GAP, r, 0);
                    emitbranch(J_B+Q_GE, l);
                    emit(J_ADDK, r, r, lowerbits(p));
                    start_new_basic_block(l);
                }
                emit(J_SHRK+J_SIGNED, r, r, p);
            }
            else
#endif
            if (isminusone(arg2_(x)))  /* if !TARGET_HAS_NONFORTRAN_DIVIDE? */
            {   h0_(x) = s_neg;
                r = cg_expr(x);
            }
            else r = cg_divrem(J_DIVR+J_SIGNED, type_(x), sim.divfn,
                               arg1_(x), arg2_(x));
        }
        if (!valneeded) bfreeregister(r), r = GAP;
        return r;

case s_rem:
        verify_integer(x);
        if (iszero(arg1_(x))) return cg_loadzero(arg2_(x));
        else if (isone(arg2_(x))) return cg_loadzero(arg1_(x));
/* can't the unsignedness property get in rsort? */
        else if (mcmode==1)
        {   int32 p;
            if ((p = ispoweroftwo(arg2_(x))) != 0)
                return cg_binary(J_ANDR, arg1_(x),
                                 mkintconst(te_int,lowerbits(p),0),
                                 0, rsort);
#ifdef TARGET_LACKS_REMAINDER
            return simulate_remainder(type_(x), arg1_(x), arg2_(x));
#else
            return(cg_divrem(J_REMR+J_UNSIGNED, type_(x), sim.uremfn,
                             arg1_(x), arg2_(x)));
#endif
        }
        else
        {
#if !defined(TARGET_HAS_DIVIDE) && !defined(TARGET_HAS_NONFORTRAN_DIVIDE)
            int32 p;
            if ((p = ispoweroftwo(arg2_(x))) != 0)
            {   /* see above code and comments for s_div too                */
                /* e.g. (signed)  z%8 == (z>=0 ? z&7 : -((-z)&7))           */
                VRegnum r = cg_expr(arg1_(x));
                LabelNumber *l = nextlabel(), *m = nextlabel();
                blkflags_(bottom_block) |= BLKREXPORTED;
                emit(J_CMPK+Q_GE, GAP, r, 0);
                emitbranch(J_B+Q_GE, l);
                if (p != 1) /* marginally better code for % 2 */
                    emitreg(J_NEGR, r, GAP, r);
                emit(J_ANDK, r, r, lowerbits(p));
                emitreg(J_NEGR, r, GAP, r);
                emitbranch(J_B+Q_AL, m);
                start_new_basic_block(l);
                emit(J_ANDK, r, r, lowerbits(p));
                start_new_basic_block(m);
                return r;
            }
#endif
            if (isminusone(arg2_(x)))
                return cg_loadzero(arg1_(x));   /* required by s_div defn */
#ifdef TARGET_LACKS_REMAINDER
            return simulate_remainder(type_(x), arg1_(x), arg2_(x));
#else
            return(cg_divrem(J_REMR+J_SIGNED, type_(x), sim.remfn,
                             arg1_(x), arg2_(x)));
#endif
        }

case s_leftshift:
        if (!valneeded) return cg_diadvoid(x);
        verify_integer(x);
        if (iszero(arg2_(x))) return(cg_expr(arg1_(x)));
        else if (iszero(arg1_(x))) return cg_loadzero(arg2_(x));
        else return(cg_binary(mcmode==1 ? J_SHLR+J_UNSIGNED : J_SHLR+J_SIGNED,
                              arg1_(x), arg2_(x), 0, rsort));

case s_rightshift:
        if (!valneeded) return cg_diadvoid(x);
        verify_integer(x);
        if (iszero(arg2_(x))) return(cg_expr(arg1_(x)));
        else if (iszero(arg1_(x))) return cg_loadzero(arg2_(x));
#ifdef TARGET_LACKS_RIGHTSHIFT   /* vax, clipper */
        else if (!integer_constant(arg2_(x)))
        {   Expr *a2 = arg2_(x);
/* The difference between signed and unsigned left shifts shows here */
            return cg_binary(mcmode==1 ? J_SHLR+J_UNSIGNED : J_SHLR+J_SIGNED,
                             arg1_(x), mk_expr1(s_neg, typeofexpr(a2), a2),
                             0, rsort);
        }
#endif /* TARGET_LACKS_RIGHTSHIFT */
/* Note that for right shifts I need to generate different code for      */
/* signed and unsigned operations.                                       */
        else return cg_binary(mcmode==1 ? J_SHRR+J_UNSIGNED : J_SHRR+J_SIGNED,
                              arg1_(x), arg2_(x), 0, rsort);

case s_and:
        if (!valneeded) return cg_diadvoid(x);
        verify_integer(x);
        if (iszero(arg1_(x))) return cg_loadzero(arg2_(x));
        if (iszero(arg2_(x))) return cg_loadzero(arg1_(x));
        else if (isminusone(arg1_(x))) return cg_expr(arg2_(x));
        else if (isminusone(arg2_(x))) return cg_expr(arg1_(x));
        else return cg_binary(J_ANDR, arg1_(x), arg2_(x), 1, rsort);

case s_or:
        if (!valneeded) return cg_diadvoid(x);
        verify_integer(x);
        if (iszero(arg1_(x))) return(cg_expr(arg2_(x)));
        if (iszero(arg2_(x))) return(cg_expr(arg1_(x)));
        if (isminusone(arg1_(x))) return cg_loadconst(-1, arg2_(x));
        if (isminusone(arg2_(x))) return cg_loadconst(-1, arg1_(x));
        return(cg_binary(J_ORRR, arg1_(x), arg2_(x), 1, rsort));

case s_xor:
        if (!valneeded) return cg_diadvoid(x);
        verify_integer(x);
        if (iszero(arg1_(x))) return(cg_expr(arg2_(x)));
        else if (iszero(arg2_(x))) return(cg_expr(arg1_(x)));
        else if (isminusone(arg1_(x)))
        {   arg1_(x) = arg2_(x);
            h0_(x) = s_bitnot;
            return(cg_expr(x));
        }
        else if (isminusone(arg2_(x)))
        {   h0_(x) = s_bitnot;
            return(cg_expr(x));
        }
        else return(cg_binary(J_EORR, arg1_(x), arg2_(x), 1, rsort));

default:
        syserr(syserr_cg_expr, (long)op, op);
        return GAP;
    }
}

/* things for swapping (virtual-) register contexts ...                    */

static SynBindList *bindlist_for_temps(RegList *regstosave,
                                       RegList *fpregstosave)
{   RegList *p1;
    SynBindList *things_to_bind = NULL;
    for (p1=regstosave; p1!=NULL; p1 = p1->rlcdr)
    {   Binder *bb = gentempvar(te_int, GAP);
        things_to_bind = mkSynBindList(things_to_bind, bb);
    }
    for (p1=fpregstosave; p1!=NULL; p1 = p1->rlcdr)
    {   RegSort tt = vregsort(p1->rlcar);
        Binder *bb = gentempvar(tt==FLTREG ? te_float : te_double, GAP);
        things_to_bind = mkSynBindList(things_to_bind, bb);
    }
    return (SynBindList *)dreverse((List *)things_to_bind);
}

static void stash_temps(RegList *regstosave, RegList *fpregstosave,
                        SynBindList *p2, J_OPCODE j_i, J_OPCODE j_f,
                        J_OPCODE j_d)
{   RegList *p1;
    for (p1=regstosave; p1!=NULL; p1 = p1->rlcdr)
    {   emitbinder(j_i, p1->rlcar, p2->bindlistcar);
        p2 = p2->bindlistcdr;
    }
    for (p1=fpregstosave; p1!=NULL; p1 = p1->rlcdr)
    {   VRegnum r = p1->rlcar;
        emitbinder((vregsort(r)==FLTREG ? j_f : j_d),
                   r, p2->bindlistcar);
        p2 = p2->bindlistcdr;
    }
}

/* The effect of calling flush_arg_usedregs is to set usedregs to 0     */
/* and either dispose of its pointed to list or to pass it on to        */
/* someone else who will.   @@@ REVIEW?                                 */
static void flush_arg_usedregs(int32 argaddr)
{   /* 'argaddr' is the stack offset (counting arg1 as 0) of where      */
    /* the first register (if any) in usedregs is to go.                */
    argaddr += alignof_toplevel * length((List *)usedregs);
    for (usedregs = (RegList *)dreverse((List *)usedregs);
             usedregs != NULL;
             usedregs = rldiscard(usedregs))
    {   VRegnum r = usedregs->rlcar;
        emit(J_PUSHR, r, GAP, argaddr -= alignof_toplevel);
        if
#if (NARGREGS==0)
#  if 1         /* #ifdef EXPERIMENTAL_68000  <<<<<<<<<<<<<< experiment */
                (usedregs->rlcdr != NULL)
#  else
                (r != R_A1)               /* hmm.         */
#  endif
#else
                (!isarg_regnum_(r))
#endif
                syserr(syserr_bad_reg, (long)r);
        active_binders = mkBindList(active_binders, integer_binder);
        current_stackdepth += alignof_toplevel;
    }
    if (usedfpregs!=NULL) syserr(syserr_bad_fp_reg);
    nusedregs = nusedfpregs = 0;
}

static void flush_fparg(VRegnum r, int32 rep, int32 argaddr)
{   if (rep == (2<<MCR_SORT_SHIFT | 4))
    {   emit(J_PUSHF, r, GAP, argaddr);
        active_binders = mkBindList(active_binders, integer_binder);
    }
    else
    {   emit(J_PUSHD, r, GAP, argaddr);
#ifdef NEVER    /* The following preferable code is beaten by cg_fnargs.  */
>>>     active_binders = mkBindList(active_binders, double_binder);
#else  /* ALWAYS */
        active_binders = mkBindList(active_binders, integer_binder);
#ifndef TARGET_IS_ADENART
        active_binders = mkBindList(active_binders, integer_binder);
#endif
#endif
    }
    current_stackdepth = padtomcrep(current_stackdepth, rep) +
                          padsize(rep & MCR_SIZE_MASK, alignof_toplevel);
}

/* shouldn't be in this file.                                           */
#ifdef TARGET_IS_ADENART
#define MEMCPYREG DBLREG
#define J_memcpy(op) ((op) - J_LDRK + J_LDRDK)
#define MEMCPYQUANTUM 8
#else
#define MEMCPYREG INTREG
#define J_memcpy(op) (op)
#define MEMCPYQUANTUM 4
#endif

#define NARGREGS1 (NARGREGS==0 ? 1 : NARGREGS)

/* The following typedef is local to cg_fnargs() and cg_fnap() */
typedef struct ArgInfo { Expr *expr; int info;
                         int32 regoff, rep, addr; } ArgInfo;

static void cg_fnargs_stack(ArgInfo arg[], int32 regargs, int32 n)
#ifdef TARGET_HAS_RISING_STACK
/* Now compile last arguments onto the stack, starting with the bottom one,
   and leaving a gap for those that are passed in registers. */
#else
/* Now compile last arguments onto the stack, starting with the top one. */
#endif
/* However, ensure the stack is always correctly aligned if necessary.   */
{   int32 narg;
#ifdef TARGET_HAS_RISING_STACK
    for (narg = regargs; narg!=n; narg++ )
    {   int32 rep = arg[narg].rep;
#else
    for (narg = n; narg!=regargs; )
    {   int32 rep = arg[--narg].rep;
#if (alignof_double > alignof_toplevel)
        if (arg[narg].addr + padsize(rep & MCR_SIZE_MASK, alignof_toplevel) !=
              arg[narg+1].addr)
        {   BindList *nb = active_binders;
            /* @@@ note that the next line of code assumes that         */
            /* (alignof_double/alignof_int) == 1 or 2.                  */
            nb = mkBindList(nb, integer_binder);
            current_stackdepth += alignof_toplevel;
            emitsetsp(J_SETSPENV, nb);
        }
#endif
#endif /* TARGET_HAS_RISING_STACK */
/* Major fun is called for here in case an actual argument is something  */
/* other than a 32-bit integer-like quantity.                            */
        switch (rep)
        {
    case 0x00000001: case 0x00000002:   /* @@@ no longer needed?         */
    case 0x00000004: case 0x00000008:
    case 0x01000001: case 0x01000002:   /* @@@ no longer needed?         */
    case 0x01000004: case 0x01000008:
/* Integer-like values of all sensible lengths : signed and unsigned.    */
/* Maybe the next use of 'nusedregs' is nasty.                           */
          if (nusedregs == NARGREGS1 || arg[narg].info == ISHARD)
              /* The following line avoids silly (n**2) code on fn call. */
#ifdef TARGET_HAS_RISING_STACK
              flush_arg_usedregs(arg[narg-1].addr);
#else
              flush_arg_usedregs(arg[narg+1].addr);
#endif
#if (NARGREGS==0)   /* #ifdef EXPERIMENTAL_68000 <<<<<<<<<<<<<< experiment */
          {   Expr *a = arg[narg].expr;
              TypeExpr *t = princtype(typeofexpr(a));
              VRegnum argr = reserveregister(
#  ifdef ADDRESS_REG_STUFF
                                h0_(t) == t_content || h0_(t) == t_subscript ?
                                    ADDRREG :
#  endif
                                    INTREG);
              (void)cg_exprreg(a, argr);
              getreservedreg(argr);
          }
/* cg_expr1(arg[narg].expr, 1); should work, but compiles wrong code (why?) */
#else
          {   Expr *a = arg[narg].expr;
#ifdef TARGET_HAS_RISING_STACK
              VRegnum argr = virtreg(R_A1 + nusedregs, INTREG);
#else
              VRegnum argr = virtreg(R_A1 + NARGREGS1-1 - nusedregs, INTREG);
#endif
              (void)cg_exprreg(a, argr);
              usedregs = mkRegList(usedregs, argr);
              nusedregs++;
          }
#endif
          break;
    case 0x02000004:    /* n.b. single float only occur with fn prototypes */
    case 0x02000008:    /* double floating */
    case 0x02000008+MCR_ALIGN_DOUBLE:         /* nasty temp hack. */
#ifdef TARGET_HAS_RISING_STACK
          flush_arg_usedregs(arg[narg-1].addr);
#else
          flush_arg_usedregs(arg[narg+1].addr);
#endif
          { VRegnum r = cg_expr(arg[narg].expr);
            int32 argaddr = arg[narg].addr;
            flush_fparg(r, rep, argaddr);
            bfreeregister(r);
          }
          break;
    default:
/* Structure valued arguments come here (except maybe the one-word case  */
/* which went through the integer code.                                  */
          if ((rep & (0xff000000+alignof_struct-1)) != 0x03000000)
              /* @@@ will this line trip up acorn's APRM etc?            */
              /* Before you change this, note the need to round up       */
              /* structsize to a word below, and to cope with MOVC in    */
              /* case the arg. is non-aligned.                           */
              syserr(syserr_cg_fnarg, (long)rep);
          if (nusedregs == NARGREGS1 || arg[narg].info == ISHARD
                                     || MEMCPYREG == DBLREG)
#ifdef TARGET_HAS_RISING_STACK
              flush_arg_usedregs(arg[narg-1].addr);
#else
              flush_arg_usedregs(arg[narg+1].addr);
#endif
          { int32 structsize = rep & MCR_SIZE_MASK, i;
/*
 * We now pretend that the ARM has a BLOCKMOVE instruction because that
 * makes it possible for gen.c to expand blockmoves into sequences of
 * loads or stores, or maybe procedure calls, exploiting knowledge of
 * how many registers are free.
 */
#ifndef TARGET_HAS_BLOCKMOVE
/* The following code needs to know struct alignment as well as size:    */
            if (alignof_struct >= MEMCPYQUANTUM && structsize < 5*MEMCPYQUANTUM)
/* The case of a (small) structure argument (now guaranteed by simplify  */
/* not to involve a function call) can be compiled using a sequence of   */
/* PUSH (or PUSHD) operations.                                           */
/* We expect that to be cheaper than the synthetic assignment used in    */
/* the more general case below.                                          */
/* This special case code is inhibited if size is not a multiple of 4.   */
            {   Expr *e = take_address(arg[narg].expr);
                Binder *b = is_local_adcon(e);
                VRegnum r = GAP;
                if (!b)
                {   r = cg_expr(e);
/* Hmm: I have to do a bfreeregister on r before any flush_arg_usedregs() */
/* This is not dangerous as flow analysis will regard r as alive up to    */
/* the last member.                                                       */
                    bfreeregister(r);
                }
#ifdef TARGET_HAS_RISING_STACK
                structsize = 0;
                while (structsize != (rep & MCR_SIZE_MASK))
#else
                while (structsize != 0)
#endif
                {   VRegnum argr;
                    if (nusedregs == NARGREGS1)
                        flush_arg_usedregs(arg[narg].addr + structsize);
#ifdef TARGET_HAS_RISING_STACK
                    argr = virtreg(R_A1 + nusedregs, INTREG);
#else
                    argr = MEMCPYREG==DBLREG ? virtreg(R_FA1, DBLREG) :
                       virtreg(R_A1 + NARGREGS1-1 - nusedregs, INTREG);
                    structsize -= MEMCPYQUANTUM;
#endif
                    if (b) emitvk(J_memcpy(J_LDRVK), argr, structsize, b);
                    else emit(J_memcpy(J_LDRK), argr, r, structsize);
#ifdef TARGET_HAS_RISING_STACK
                    structsize += MEMCPYQUANTUM;
#endif
                    if (MEMCPYREG==DBLREG)
                        flush_fparg(argr, 0x02000008,
                                    arg[narg].addr+structsize);
                    else
                        usedregs = mkRegList(usedregs, argr), nusedregs++;
                }
/* The bfreeregister(r) above notionally belongs here.                   */
            }
            else
#endif
/* The general case of an argument that can involve a function call      */
/* returning a structure is dealt with here... (Oct 92: fn can't happen) */
            {
#ifdef TARGET_HAS_RISING_STACK
              flush_arg_usedregs(arg[narg-1].addr);
#else
              flush_arg_usedregs(arg[narg+1].addr);
#endif
              { TypeExpr *t = typeofexpr(arg[narg].expr);
#ifdef NEVER
/* The following preferable code is beaten by cg_fnargs.                */
                Binder *gen = gentempbinder(t);
                bindstg_(gen) |= b_addrof;
/* Forge an address-taken struct variable to fit on the stack in the    */
/* usual position for the actual parameter.                             */
                cg_bindlist(mkSynBindList(0, gen), 0);
#else /* ALWAYS */
                Binder *gen = gentempvar(t, GAP);
                BindList *nb = active_binders;
                current_stackdepth += padsize(structsize,alignof_toplevel);
                for (i=padsize(structsize,alignof_toplevel); i != 0;
                     i-=alignof_toplevel)   /* n.b. TARGET_IS_ADENART */
                    nb = mkBindList(nb, integer_binder);
#ifdef TARGET_STACK_MOVES_ONCE
                bindstg_(gen) |= b_addrof;
/* BEWARE: the next line is in flux (Nov89) and is only OK because      */
/* 'gen' never gets to be part of 'active_binders'.                     */
                bindaddr_(gen) = arg[narg].addr | BINDADDR_NEWARG;
#else
                bindstg_(gen) |= b_addrof|b_bindaddrlist;
#  ifdef TARGET_IS_SPARC
/* The following line is an extra hack to make the structure_assign()   */
/* code work on the SPARC (there is an implicit aligning INT hole       */
/* at the end of an arg list).  Re-work this code.                      */
                gen->bindaddr.bl = mkBindList(nb, integer_binder);
#  else
                gen->bindaddr.bl = nb;
#endif
#endif
                emitsetsp(J_SETSPENV, nb);
/* I hope that fools things into believing that the stack is in the      */
/* state that I have just put it in.                                     */
#endif
                structure_assign((Expr *)gen, arg[narg].expr, structsize);
                if (usedregs != 0) syserr(syserr_fnarg_struct);
              }
            }
          }
          break;
        }                 /* end of switch */
#if defined(TARGET_HAS_RISING_STACK) && (alignof_double > alignof_toplevel)
        if (arg[narg].addr + padsize(rep & MCR_SIZE_MASK, alignof_toplevel) !=
              arg[narg+1].addr)
        {   BindList *nb = active_binders;
            /* @@@ note that the next line of code assumes that         */
            /* (alignof_double/alignof_int) == 1 or 2.                  */
            nb = mkBindList(nb, integer_binder);
            current_stackdepth += alignof_toplevel;
            emitsetsp(J_SETSPENV, nb);
        }
#endif
    }                     /* end of for */
    flush_arg_usedregs(arg[regargs].addr);
}

static void cg_fnargs_regs(ArgInfo arg[], int32 intregargs, int32 fltregargs,
                           VRegnum specialreg)
/* Finally do the first NARGREGS (or less) 'int' args to registers:    */
/* Do the hardest first to avoid moving easier things around.          */
/* Also, save and restore explicitly all function results except the   */
/* last to avoid recursive cg_fnap making a quadratic mess of          */
/* f() { g(a(),b(),c(),...); }                                         */
/* Oct 92: this code has been extended so that 'intregargs' can also   */
/* include struct/fp regs, which are loaded directly (instead of       */
/* going through cg_fnargs_stack() and J_POP).  Hence (addressable)    */
/* float values can be loaded to int regs directly.    ***SOON***      */
/* The code behaves as before unless cg_fnargs() takes the opportunity */
/* to increase 'intregargs' from before.                               */
{   BindList *save_binders = active_binders;
    int32 d = current_stackdepth;
    int32 regargs = intregargs+fltregargs;
/* the +1 in the next lines is for the possible hidden argument.       */
    VRegnum argregs[NFLTARGREGS+NARGREGS+1];        /* arg virt regs   */
    SynBindList *argsaves[NFLTARGREGS+NARGREGS+1];  /* for previous fn call results */
    int hardness;
    int32 firsthard = -1;
    {   int32 i;
        for (i = regargs; i != 0; )
        {   if (arg[--i].info == ISHARD) firsthard = i;
            argsaves[i] = (SynBindList *) DUFF_ADDR, argregs[i] = GAP;
        }
    }
    for (hardness = ISHARD; hardness >= ISCONST; hardness--)
    {   int32 i;
/* There are currently not any FLTREG args, only double...              */
#define callregsort(n, f) ((n)<(f) ? DBLREG : INTREG)
#define callreg(n, f) ((n)<(f) ? virtreg(R_FA1+(n), callregsort(n, f)) : \
                        specialreg != GAP && (n)==(f) ? specialreg : \
                        virtreg(R_A1+arg[n].regoff,INTREG))
        for (i = regargs; i != 0; )
            if (arg[--i].info == hardness)
            {   Expr *a = arg[i].expr;
                int32 repsort = arg[i].rep >> MCR_SORT_SHIFT;
                if (hardness == ISCONST)
                    (void)cg_exprreg(a, callreg(i, fltregargs));
#if alignof_struct >= MEMCPYQUANTUM
                else if (repsort == 3)
                    argregs[i] = cg_expr(take_address(a));
#endif /* but note the next line will syserr() for structs!             */
                else
                    argregs[i] = cg_expr(a);
                if (hardness == ISHARD)
                {   SynBindList *b = i == firsthard ? 0 : /* first is easy */
                                  bindlist_for_temps(usedregs, usedfpregs);
                    /* b should have at most 1 elt */
                    cg_bindlist(b, 1);
                    if (b != 0)
                    {   J_OPCODE j_saveop =
                          (repsort == 3) ? J_STRV :
                          (callregsort(i, fltregargs) == DBLREG) ? J_STRDV :
                          (repsort == 2) ? J_STRDV : J_STRV;
                        if (b->bindlistcdr != NULL)
                            syserr(syserr_cg_fnarg1);
                        emitbinder(j_saveop, argregs[i], b->bindlistcar);
                        usedregs = 0; nusedregs = 0;
                        usedfpregs = 0; nusedfpregs = 0;
                    }
                    argsaves[i] = b;
                }
            }
    }
    {   int32 i;
        for (i = 0; i < regargs; i++)
        {   SynBindList *b;
            bool done = arg[i].info == ISCONST; /* we did these above.  */
            int32 repsort = arg[i].rep >> MCR_SORT_SHIFT;
            VRegnum argr = callreg(i, fltregargs);
/* @@@ This can be usedfpregs?    Yes, fix!!!                           */
/* Unclear the nusedregs/fregs do anything at all here.                 */
            usedregs = mkRegList(usedregs, argr);
            nusedregs++;
            if (arg[i].info == ISHARD && (b = argsaves[i]) != 0)
            {   if (b->bindlistcdr != NULL)
                    syserr(syserr_cg_fnarg1);
/* Logically, the following code just reloads argregs[i], but we        */
/* try to reload from spill directly to arg regs: OK for int->intreg    */
/* dble->dblereg, but dble/struct->intreg needs careful code...         */
/* The order of tests below follows that in the (!done) case.           */
                if (repsort == 3)
                    emitbinder(J_LDRV, argregs[i], b->bindlistcar);
                else if (callregsort(i, fltregargs) == DBLREG)
                    emitbinder(J_LDRDV, argr, b->bindlistcar),
                    done = 1;
                else if (repsort == 2)
                    emitbinder(J_LDRDV, argregs[i], b->bindlistcar);
                else
                {   emitbinder(J_LDRV, argr, b->bindlistcar);
                    done = 1;
                }
                (void)discard2((List *)b);
            }
            if (!done)
            {   VRegnum r = argregs[i];
                if (repsort == 3)       /* [am] */
                {   int32 j, limit = ((arg[i].rep & MCR_SIZE_MASK) + 3)/4;
/* The usedregs/nusedregs code needs a better interface!               */
                    usedregs = usedregs->rlcdr; nusedregs--;
                    for (j = 0; j<limit; j++)
                    {   usedregs = mkRegList(usedregs, argr + j);
                        nusedregs++;
                        emit(J_LDRK, argr + j, r, 4*j);
                    }
                }
                else if (callregsort(i, fltregargs) == DBLREG)
                    emitreg(J_MOVDR, argr, GAP, r);
                else if (repsort == 2)   /* [am] */
                {   usedregs = mkRegList(usedregs, argr+1);
                    nusedregs++;
                    emitreg(J_MOVDIR, argr, argr+1, r);
                }
                else
                    emitreg(J_MOVR, argr, GAP, r);
                bfreeregister(r);
            }
        }
        emitsetsp(J_SETSPENV, save_binders);
        current_stackdepth = d;
    }
}

#ifdef TARGET_IS_ARM            /* experimental!                        */
#  define TARGET_HAS_MOVDIR 1   /* BUT NOT FOR ADENART.                 */
/* we do not currently encourage setting TARGET_HAS_MOVDIR in target.h  */
/* since J_MOVDIR becomes compulsory-ish when J_POP goes...             */
#endif

#ifdef TARGET_HAS_MOVDIR
# if alignof_struct >= 4
#  define intregable(repsort) 1
# else  /* work to be done! */
#  define intregable(repsort) ((repsort) < 3)
# endif
#else
/* ADENART must be this case since no flt/struct args in int regs.      */
#  define intregable(repsort) ((repsort) < 2)
#endif

static int32 cg_fnargs(ExprList *a, ArgInfo arg[], int32 n, int32 resultregs,
                       VRegnum specialarg, int32 fnflags)
/* specialarg is non-GAP if there is an implicit extra (first,integer)  */
/* arg (e.g. for swi_indirect or 88000 struct return value pointer).    */
{
    int32 intargwords, intregargs = 0, intregwords = 0, fltregargs = 0;
#ifdef TARGET_HAS_RISING_STACK
    Binder *arg0;
#endif
    {   int32 argoff = 0, narg = 0;
#ifdef TARGET_FP_ARGS_IN_FP_REGS
        if ((config & CONFIG_FPREGARGS) &&
            !(fnflags & f_nofpregargs))
        {   ExprList *p, *q = 0;
	    
/* Reorder the argument list, by destructively extracting the first     */
/* up to NFLTARGREGS float args, and then dropping through to handle    */
/* int args.  This code needs better TARGET parameterisation.           */
/* On the MIPS we only allow LEADING fp args in fp regs.                */
/* Moreover such fp args inhibit the corresponding 2 int reg args!      */
            for (p = a; p != NULL; p = cdr_(p))
            {   Expr *ae = exprcar_(p);
                int32 repsort = mcrepofexpr(ae) >> MCR_SORT_SHIFT;
                if (repsort == 2 && narg < NFLTARGREGS
#ifdef TARGET_IS_MIPS           /* only LEADING flt args in flt regs.   */
                                 && q == NULL
#endif
                   )
                {   arg[narg++].expr = ae;
#ifdef TARGET_IS_MIPS
                    intregwords += 2;   /* i.e. two int junk regs       */
#endif
                    fltregargs++;
                    if (q == 0) a = cdr_(p); else cdr_(q) = cdr_(p);
                }
                else q = p;
            }
        }
#endif
/* The following code determines in which (int)reg an arg is going to   */
/* be passed.  In general this is the same as determining its address   */
/* (see below '.addr') since register alignment of doubles passed in    */
/* int regs usually matches that of doubles passed in storage.          */
/* It could probably be merged into to the 'argoff' loop.               */
        if (specialarg != GAP) intregwords--;
/* intregargs can include specialarg, intregwords never does.           */
        for (; a != NULL; a = cdr_(a), narg++)
        {   Expr *ae = exprcar_(a);
            int32 rep = mcrepofexpr(ae);
#if alignof_double > alignof_int && !defined(TARGET_IS_SPARC)
            int32 rbase = padsize(intregwords, rep & MCR_ALIGN_DOUBLE ? 2:1L);
#else
            int32 rbase = intregwords;
#endif
            int32 rlimit = rbase + padsize((rep & MCR_SIZE_MASK),
                                      alignof_toplevel)/alignof_toplevel;
            arg[narg].expr = ae;
            if (rlimit <= NARGREGS && intregable(rep >> MCR_SORT_SHIFT))
                intregargs++, intregwords = rlimit, arg[narg].regoff = rbase;
            else break;
        }
        arg[narg].regoff = intregwords;   /* useful sentinel below?     */
        for (; a != NULL; a = cdr_(a), narg++)
            arg[narg].expr = exprcar_(a);
        if (narg != n) syserr(syserr_cg_argcount);

        for (narg = 0; narg < n; narg++)
        {   Expr *ae = arg[narg].expr;
            int32 rep = mcrepofexpr(ae);
            if ((rep >> MCR_SORT_SHIFT) == 3 && structure_function_value(ae))
                /* i.e. optimise1(simplify.c) failed.               */
                syserr("cg_fnargs(struct fn)");
#ifdef TARGET_IS_SPARC
/* The (de facto, but poorly documented) SPARC calling standard         */
/* requires double args to be stored on the stack (or in int regs) in   */
/* a possibly unaligned manner by avoiding leaving gaps...              */
            argoff = padtomcrep(argoff, rep & ~MCR_ALIGN_DOUBLE);
#else
            argoff = padtomcrep(argoff, rep);
#endif
            arg[narg].info = nastiness(ae);
            arg[narg].rep = rep;
            arg[narg].addr = argoff;
/* Do not increment 'argoff' for specialarg (hidden argument).          */
            if (!(narg == fltregargs && specialarg != GAP))
                argoff += padsize(rep & MCR_SIZE_MASK, alignof_toplevel);
        }
#if !defined(TARGET_HAS_RISING_STACK) && (alignof_double > alignof_toplevel)
/* The next line pads the offset of the last arg exactly in the case    */
/* it will be handled by cg_fnargs_stack.  If all the args are int-like */
/* and go directly to regs (i.e. <NARGREGS) then we do not have to pad  */
/* after the last of an odd number of args.                             */
        if (n != fltregargs+intregargs)
            argoff = padtomcrep(argoff, double_pad_binder->bindmcrep);
#endif
        arg[narg].addr = argoff;        /* sentinel for cg_fnargs_stack */
        intargwords = (argoff - arg[fltregargs].addr)/alignof_toplevel;
        /* Note that any hidden arg is not counted in intargwords.      */
    }
#if (alignof_double > alignof_toplevel)
    /* Pre-align stack to double alignment -- maybe one day         */
    /* suppress the alignment if there are no double args and the   */
    /* called proc can be seen to be floating-free and leaf-like.   */
    /* Note, by considering "f() { int x; g(0.0, &x); }", that this */
    /* may waste stack space -- i.e.                                */
    /*   [ half of 0.0, half of 0.0, &x, pad, pad, x, f-linkage..]  */
    /* perhaps double_antipad_binder would help.                    */
    {   BindList *nb = active_binders;
        nb = mkBindList(nb, double_pad_binder);
        current_stackdepth = padtomcrep(current_stackdepth,
                                        double_pad_binder->bindmcrep);
        emitsetsp(J_SETSPENV, nb);
    }
#endif
#ifdef TARGET_HAS_RISING_STACK
    /* Leave gap on stack for those args which are passed in regs, but */
    /* only if some are also (possibly unnecessarily) on the stack.    */
    if (intargwords > intregwords) {
      BindList *nb = active_binders;
      int32 i;
      for (i=0; i<intregargs; i++) {
        current_stackdepth += alignof_toplevel;
        if (i==0) {
          nb = mkBindList(nb, gentempvar(te_int, GAP));
          arg0 = nb->bindlistcar; /* needed later!! */
          bindstg_(arg0) |= b_addrof;
#  ifdef TARGET_STACK_MOVES_ONCE
          /* BEWARE: the next line is in flux (Nov89) and is only OK because */
          /* 'arg0' never gets to be part of 'active_binders'.               */
          bindaddr_(arg0) = arg[0].addr | BINDADDR_NEWARG;
#  else
          bindaddr_(arg0) = (current_stackdepth + arg[0].addr) | BINDADDR_LOC;
#  endif
        }
        else nb = mkBindList(nb, integer_binder);
      }
      emitsetsp(J_SETSPENV, nb);
    }
#endif
#ifdef TARGET_IS_88000
/*
 * For the 88000 structure args must go on the stack in the place where they
 * would have been if ALL args were stacked.  To achieve this I pad the
 * stack before preparing args. Only needed if some struct args present.
 * Something like this needs to be done if TARGET_IS_SPARC for Sun's
 * proc. call std. as opposed to gnu's.
 */
/* probably inadequate wrt fltargregs */
    {   BindList *save_binders = active_binders;
        int32 d = current_stackdepth;
        int32 padcount = NARGREGS - intargwords;
        if (n == intregargs) padcount = 0;
        if (padcount > 0)
        {   int32 j;
            BindList *nb = active_binders;
            for (j=0; j<padcount; j++)
            {   nb = mkBindList(nb, integer_binder);
                current_stackdepth += alignof_toplevel;
            }
            emitsetsp(J_SETSPENV, nb);
        }
#endif

    cg_fnargs_stack(arg, intregargs+fltregargs, n);
    cg_fnargs_regs(arg, intregargs, fltregargs, specialarg);

#if (alignof_double > alignof_toplevel)
/* The next few lines deal with the trickiness of handling a call like  */
/* f(1,2.3) which, if alignof_double==8 (excepting SPARC) can load      */
/* regs a1, a3, a4.  Keep regalloc happy by undefining a2 explicitly.   */
/* Code above ensures we only pad at end of regs if also stack args.    */
/* Beware: not tested with alignment AND specialarg != GAP.             */
/* @@@ we should probably have done intregargs-- here if specialarg...  */
/* @@@ (or used .regoff since very similar to .addr.)                   */
    {   int32 i = fltregargs + (specialarg==GAP ? 0:1), r = 0;
        int32 ibase = arg[i].addr;
        while (intregwords < NARGREGS &&
                 ibase + 4*intregwords < arg[fltregargs+intregargs].addr)
            intregwords++;
        while (r < intregwords && i < fltregargs+intregargs)
          if (ibase + 4*r >= arg[i+1].addr) i++;
          else
          {   if (ibase + 4*r >= arg[i].addr +
                      padsize(arg[i].rep & MCR_SIZE_MASK, alignof_toplevel))
                emitbinder(J_INIT, R_A1 + r, 0);
              r++;
          }
    }
#endif

    if (current_stackdepth > greatest_stackdepth)
        greatest_stackdepth = current_stackdepth;

/* Now if some of the first few args were floating there are values on   */
/* the stack that I want to have in registers. Ditto structure values.   */
/* [am] this code is now dying, as POP is about to be removed...         */

    {   RegList *q = NULL;
        int32 i;
        BindList *nb = active_binders;
#ifndef TARGET_IS_ADENART
        for (i=intregwords; i<intargwords && i<NARGREGS; i++) {
#ifdef TARGET_HAS_RISING_STACK
          if (intargwords > NARGREGS)
            emitvk(J_LDRVK, virtreg(R_A1+i, INTREG), 4*i, arg0);
          else
#endif
          { q = mkRegList(q, virtreg(R_A1+i, INTREG));
            /* @@@ The next line explains the contorted alternative */
            /* to the #ifdef NEVER code in cg_fnargs_stack.         */
            if (nb->bindlistcar != integer_binder)
                syserr(syserr_padbinder, nb->bindlistcar);
            current_stackdepth -= alignof_toplevel;
            intregwords++;
          }
          nb = nb->bindlistcdr;
        }
        if (q!=NULL)
        {   emitpush(J_POP, GAP, GAP, q);
            active_binders = nb;
        }
#endif /* !TARGET_IS_ADENART */
        /* Recreate the list of used registers to keep life safe */
        /* @@@ Redo soon? */
        while (usedregs) usedregs = rldiscard(usedregs);
        while (usedfpregs) usedfpregs = rldiscard(usedfpregs);
        nusedregs = nusedfpregs = 0;

        if (specialarg != GAP) usedregs = mkRegList(usedregs, specialarg);
        for (i=0; i<intregwords; i++)
        {   usedregs = mkRegList(usedregs, virtreg(R_A1+i, INTREG));
            nusedregs++;
        }
#ifdef TARGET_FP_ARGS_IN_FP_REGS
        for (i=0; i<fltregargs; i++)
        {   usedregs = mkRegList(usedregs, virtreg(R_FA1+i, DBLREG));
/* @@@ AM: why on earth does this code not use 'usedfpregs?'.           */
/* answer: only one call of cg_expr for odd machine before discarded.   */
            nusedregs++;
        }
#endif
    }
#ifdef TARGET_IS_88000
        emitsetsp(J_SETSPENV, save_binders);
        current_stackdepth = d;
    }
#endif
/* Marker in argwords if a special register was used for a struct result */
    return k_argdesc_(intregwords, fltregargs, resultregs,
                      arg[n].addr/alignof_toplevel,
                      specialarg != GAP ? K_SPECIAL_ARG : 0L);
}

#ifdef TARGET_IS_ADENART
#include <string.h>
                                /* Matsushita standard requires adetran_caller_
                                 * or adetran_callee_ but we cannot see how it
                                 * matters much, as once a function uses that
                                 * argument passing mechanism it must be used
                                 * internally in the C the same way.  Hence
                                 * here we check all three formats! */
#define FIRST_ADETRAN_ARG 0x101
static bool isadetran(Symstr *sv)
{   return (strncmp(symname_(sv), "adetran_caller_", 15) == 0 ||
            strncmp(symname_(sv), "adetran_callee_", 15) == 0 ||
            strncmp(symname_(sv), "adetran_", 8) == 0);
}
#endif

/* For functions of 10 args or less, C stack is used, not SynAlloc space: */
#define STACKFNARGS 10  /* n.b. this is *not* a hard limit on no. of args */
/* NB also that I need STACKFNARGS to be at least as large as the no of   */
/* registers for passing args if I have an 88000 as target.               */
static void cg_fnap_1(AEop op, Expr *fn, ExprList *a, VRegnum resreg)
{
    int32 argdesc;
    VRegnum structresultp = GAP;
    Expr *structresult = NULL;
    Binder *structresultbinder = NULL;
    int32 resultwords = 0, resultregs = 0;
    TypeExpr *t = princtype(typeofexpr(fn));
    int32 fnflags;
    if (!(h0_(t) == t_content &&
           (h0_(t = princtype(typearg_(t))) == t_fnap ||
             (h0_(t) == t_coloncolon &&         /* C++ ptr-to-mem fns */
              h0_(t = princtype(typearg_(t))) == t_fnap))))
    {   pr_expr(fn); cc_msg(" of type ");
        pr_typeexpr(princtype(typeofexpr(fn)), 0);
        syserr(syserr_cg_fnap);
    }
    fnflags = typefnaux_(t).flags;

    if ((op == s_fnapstruct || op == s_fnapstructvoid) &&
        (fnflags & bitoffnaux_(s_structreg)))
    {   resultwords = sizeoftype(typearg_(t)) / sizeof_long;
        if (resultwords > 1 && resultwords <= NARGREGS) {
            structresult = exprcar_(a);
            a = cdr_(a);
            resultregs = resultwords;
        }
    }

    {   ArgInfo v[STACKFNARGS];
        int32 n = length(a);
/* The n+1 on the next line gives a sentinel to cg_fnargs.              */
        ArgInfo *p = (n+1) > STACKFNARGS ?
                       (ArgInfo *)SynAlloc((n+1)*sizeof(ArgInfo)) : v;
        VRegnum specialarg =
#ifdef TARGET_SPECIAL_ARG_REG
                            (fnflags & f_specialargreg
#  ifdef TARGET_STRUCT_RESULT_REGISTER
                             || op == s_fnapstruct
                             || op == s_fnapstructvoid
#  endif
                             ) ? virtreg(TARGET_SPECIAL_ARG_REG, INTREG) :
#endif
                            GAP;
        argdesc = cg_fnargs(a, p, n, resultregs, specialarg, fnflags);
    }

/* Now the arguments are all in the right places - call the function     */

    if (structresult != NULL && op != s_fnapstructvoid)
    {   if (h0_(structresult) == s_addrof &&
            h0_(arg1_(structresult)) == s_binder)
            structresultbinder = (Binder *)arg1_(structresult);
        else
            structresultp = cg_expr(structresult);
/* this call to cg_expr() is wrong if specialarg or there may be a fn? */
/* it should probably go via the specialreg-like interface?               */
    }

#ifdef TARGET_FLAGS_VA_CALLS
/* We only put the K_VACALL flag on if TARGET_FLAGS_VA_CALLS as it      */
/* suppresses tail-recursion optimisation.                              */
    if (fntypeisvariadic(t)) argdesc |= K_VACALL;
#endif

    {   Expr *temp;
        if (h0_(fn)==s_addrof &&
            (temp = arg1_(fn))!=0 &&
            h0_(temp)==s_binder)
        {   if (princtype(bindtype_((Binder *)temp)) != t)
                /* sanity check after tidy of old code.                 */
                syserr("cg_fnap_1 overzealous sanity check");
            if (fnflags & bitoffnaux_(s_pure)) argdesc |= K_PURE;
/* We could imagine code which emits several jopcodes here (saved in    */
/* in-linable form in typefnaux_(t)) for in-line JOP expansion.         */
            if (fnflags & bitoffnaux_(s_swi))
                emitcall(J_OPSYSK, resreg, argdesc,
                          (Binder *)(typefnaux_(t).inlinecode));
            else
                emitcall(J_CALLK, resreg, argdesc, (Binder *)temp);
        }
        else switch (mcrepofexpr(fn))
        {
        case 0x00000004: case 0x01000004:
        case 0x00000008: case 0x01000008:
            {   VRegnum r = cg_expr(fn);
                emitcallreg(J_CALLR, resreg, argdesc, r);
                bfreeregister(r);
                break;
            }
        default:
                syserr(syserr_cg_fnap);
        }
    }
    if (structresult != NULL && op != s_fnapstructvoid)
    {   int32 i;
        if (structresultbinder != NULL &&
            !(bindstg_(structresultbinder) & bitofstg_(s_auto)))
            structresultp = cg_expr(structresult);
        for (i = 0; i < resultwords; i++) {
            if (structresultp != GAP)
                emit(J_STRK, virtreg(R_A1+i, INTREG),
                     structresultp, i * sizeof_long);
            else
                emitvk(J_STRVK, virtreg(R_A1+i, INTREG),
                       i * sizeof_long, structresultbinder);
        }
        bfreeregister(structresultp);
    }
}

static VRegnum cg_fnap(Expr *x, VRegnum resreg, bool valneeded)
{
#ifdef TARGET_IS_ADENART
  { ExprList *a = exprfnargs_(x);
    Expr *fna = arg1_(x), *fn;
/* A call to adetran has args in 0x101,0x102... with pointers           */
/* encoded as WORD pointers.  Convert from C byte pointers.             */
    if (a && h0_(fna) == s_addrof && h0_(fn = arg1_(fna)) == s_binder
          && isadetran(bindsym_((Binder *)fn)))
    {   /* At least one arg in a call to adetran_xxx(...).              */
        /* We should really put ALL the args in temps before storing    */
        /* them in low memory, but for now this is fine...              */
        int32 n = FIRST_ADETRAN_ARG;
        exprfnargs_(x) = 0;             /* modify the parse tree(!!)    */
#define te_ullint primtype_(bitoftype_(s_int)|bitoftype_(s_unsigned)| \
                            bitoftype_(s_long)|bitoftype_(s_short))
        for (; a != NULL; a = a->cdr)
        {   Expr *ae = exprcar_(a);
            TypeExpr *at = typeofexpr(ae);
            cg_exprvoid(
              mkassign(s_assign,
                mkunary(s_content,
                  mkintconst(ptrtotype_(at), 8*n++, 0)),
                h0_(princtype(at)) == t_content ?
                  mk_expr1(s_cast, at,
                    mk_expr2(s_rightshift, te_ullint,   /* hack to unsigned */
                             ae, mkintconst(te_int,3,0))) :
                  ae));
        }
    }
  }
#endif

#ifdef TARGET_STACK_MOVES_ONCE
/* If TARGET_STACK_MOVES_ONCE then we have to put all arglists in the   */
/* same place.  To avoid overwriting in f(g(1,2),g(3,4)) we transform   */
/* this to "let (t0,t1) in (t0=g(1,2), t1=g(3,4), f(t0,t1))".           */
/* We only do this transformation for ISHARD (= function containing)    */
/* terms.  This transformation is sometimes pessimistic (e.g. last      */
/* arg to a procedure being a fn), but regalloc should be able to       */
/* produce the code we first thought of.                                */
/* Before 'improving' the last arg. treatment, consider the case of     */
/* it being a structure returning function, and thus overlap occurring. */
/* Note that this code destructively updates the aetree.                */
  { ExprList *a = exprfnargs_(x);
    SynBindList *bl = 0;
    for (; a != NULL; a = cdr_(a))
      {   Expr *ae = exprcar_(a);
          if (nastiness(ae) == ISHARD)
          {   TypeExpr *t = typeofexpr(ae);
              Binder *gen = gentempbinder(t);
              exprcar_(a) = (Expr *)gen;
              bl = mkSynBindList(bl, gen);
              x = mk_expr2(s_comma, type_(x),     /* === typeofexpr(x)    */
                           mk_expr2(s_assign, t, (Expr *)gen, ae),
                           x);
          }
      }
    if (bl)
          /* We had fn call(s) inside fn args, so remove and retry.        */
          return cg_expr2(mk_exprlet(s_let, type_(x), bl, x), valneeded);
  }
#endif
  {
/* Compile a call to a function - being tidied!                          */
    RegSort rsort = vregsort(resreg);
    int32 spint = spareregs, spfp = sparefpregs;
    RegList *regstosave = usedregs, *fpregstosave = usedfpregs;
    int32 savebits = nusedregs, savefpbits = nusedfpregs;
    BindList *save_binders = active_binders;
    int32 d = current_stackdepth;
    SynBindList *things_to_bind = bindlist_for_temps(regstosave, fpregstosave);
/* Calling a function must preserve all registers - here I push any that */
/* are in use onto the stack.                                            */
    cg_bindlist(things_to_bind, 1);
    stash_temps(regstosave, fpregstosave, things_to_bind,
                J_STRV, J_STRFV, J_STRDV);

    nusedregs = nusedfpregs = 0;        /* new 20-6-87 */
    usedregs = usedfpregs = NULL;
    spareregs = sparefpregs = 0;
    cg_fnap_1(h0_(x), arg1_(x), exprfnargs_(x), resreg);

/* All registers are now free - work out what the args to the fn are      */
/* Switch back to the outer context of registers.                        */
    while (usedregs) usedregs = rldiscard(usedregs);
    while (usedfpregs) usedfpregs = rldiscard(usedfpregs);
    usedregs = regstosave, usedfpregs = fpregstosave;
    nusedregs = savebits,  nusedfpregs = savefpbits;
    spareregs = spint, sparefpregs = spfp;
    {   VRegnum resultr = GAP;
        if (valneeded)
        {   resultr = fgetregister(rsort);
            emitreg(floatyop(rsort, J_MOVR, J_MOVFR, J_MOVDR),
                    resultr, GAP, resreg);
        }
        stash_temps(regstosave, fpregstosave, things_to_bind,
                    J_LDRV, J_LDRFV, J_LDRDV);
        while (things_to_bind)
            things_to_bind = (SynBindList *)discard2((List *)things_to_bind);
        emitsetsp(J_SETSPENV, save_binders);
        current_stackdepth = d;
        return resultr;
    }
  }
}

/* jopcode generation for commands ...                                      */

static void cg_cmd(Cmd *x)
{
    Cmd *oldcmd = cg_current_cmd;
    AEop op;
    while (x != 0)
    {
        cg_current_cmd = x;
        if (x->fileline.f != 0)
        {   /* This (outer) 'if' block could be a proc as appears below too */
            if (!cg_infobodyflag)
            {
#ifdef TARGET_HAS_PROFILE
                if (profile_option) emitfl(J_COUNT, x->fileline);
#endif
                if (usrdbg(DBG_PROC)) emit(J_INFOBODY, GAP, GAP, 0);
                cg_infobodyflag = 1;
            }
            if (usrdbg(DBG_LINE)) emitfl(J_INFOLINE, x->fileline);
        }
        switch (op = h0_(x))
        {

default:    syserr(syserr_cg_cmd, (long)op, op);
            break;

case s_endcase:    /* those 'break's meaning "exit switch" */
            if (switchinfo.endcaselab==NOTINSWITCH) syserr(syserr_cg_endcase);
            else
            {   emitsetspandjump(switchinfo.binders, switchinfo.endcaselab);
            }
            break;

case s_break:      /* those 'break's meaning "exit loop" */
            if (loopinfo.breaklab==NOTINLOOP) syserr(syserr_cg_break);
            else
            {   if (loopinfo.breaklab == 0)
                    loopinfo.breaklab = nextlabel();
                emitsetspandjump(loopinfo.binders, loopinfo.breaklab);
            }
            break;

case s_continue:
            if (loopinfo.contlab==NOTINLOOP) syserr(syserr_cg_cont);
            else
            {   if (loopinfo.contlab == 0)
                    loopinfo.contlab = nextlabel();
                emitsetspandjump(loopinfo.binders, loopinfo.contlab);
            }
            break;

#ifdef EXTENSION_VALOF
case s_resultis:
            /* Valof blocks will not work (a syserr will occur via cg_addr */
            /* or cg_exprreg) for structure results and would need code    */
            /* here and at s_resultis (and cg_addr &c) to make them work.  */
            {   Expr *val = cmd1e_(x);
                if (val != 0) /* In case of 'resultis <errornode>' */
                    (void)cg_exprreg(val, valofinfo.r);
            }
            emitsetspandjump(valofinfo.binders, valofinfo.lab);
            break;

#endif

case s_return:
            cg_count(x->fileline);
            cg_return(cmd1e_(x), NO);
            break;

case s_do:
            cg_loop(0, 0, 0, cmd1c_(x), cmd2e_(x));
            break;

case s_for: cg_loop(cmd1e_(x), cmd2e_(x), cmd3e_(x), cmd4c_(x), 0);
            break;

case s_if:  {   LabelNumber *l1 = nextlabel();
                Cmd *cc;
                cg_test(cmd1e_(x), 0, l1);
                cc = cmd2c_(x);
                if (cc != NULL)
/* This mess is because we can properly have a null consequence for a */
/* condition, but then it does not have a count field to pass down to */
/* cg_count!                                                          */
                {   cg_count(cc->fileline);
                    cg_cmd(cc);
                }
                if ((x = cmd3c_(x))==0) start_new_basic_block(l1);
                else
                {   LabelNumber *l2 = nextlabel();
                    emitbranch(J_B, l2);
                    start_new_basic_block(l1);
                    cg_cmd(x);
                    start_new_basic_block(l2);
                    cg_count(x->fileline);
                }
            }
            break;

case s_switch:
            if ((mcrepofexpr(cmd1e_(x)) >> MCR_SORT_SHIFT) > 1)
                syserr(syserr_cg_switch);
            cg_count(x->fileline);
            {   VRegnum r = cg_expr(cmd1e_(x));
                struct SwitchInfo oswitchinfo;
                int32 ncases = 0, i;
                CasePair *casevec;
                Cmd *c;
                oswitchinfo = switchinfo;
                switchinfo.endcaselab = nextlabel();
                switchinfo.binders = active_binders;
                switchinfo.defaultlab =
                    switch_default_(x) ? nextlabel() : switchinfo.endcaselab;
                for (c = switch_caselist_(x); c != 0; c = case_next_(c))
                    ncases++;                /* count cases */
/* n.b. SynAlloc is used in the next line -- the store will die very soon */
/* we should use a 'C' auto vector for small amounts here                 */
                casevec = (CasePair *) SynAlloc(ncases*sizeof(casevec[0]));
                i = ncases;
                for (c = switch_caselist_(x); c != 0; c = case_next_(c))
                {   LabelNumber *ln = nextlabel();
                    if (h0_(c) != s_case) syserr(syserr_cg_caselist);
                    i--;    /* syn sorts them backwards */
                    casevec[i].caseval = evaluate(cmd1e_(c));
                    casevec[i].caselab = ln;
                    /* case_lab_(c) */ cmd4c_(c) = (Cmd *)ln;
                }
                /* previous phases guarantee the cases are sorted by now */
                casebranch(r, casevec, ncases, switchinfo.defaultlab);
                bfreeregister(r);
                cg_cmd(cmd2c_(x));
                start_new_basic_block(switchinfo.endcaselab);
                switchinfo = oswitchinfo;
            }
            break;

case s_case:
            if (switchinfo.defaultlab==NOTINSWITCH) syserr(syserr_cg_case);
            {   LabelNumber *l1 = case_lab_(x);
                if (l1 == NULL) syserr(syserr_unset_case);
                cg_case_or_default(l1);
            }
            x = cmd2c_(x);
            continue;

case s_default:
            if (switchinfo.defaultlab==NOTINSWITCH) syserr(syserr_cg_default);
            cg_case_or_default(switchinfo.defaultlab);
            x = cmd1c_(x);
            continue;

case s_colon:
            {   LabBind *labbinder = cmd1lab_(x);
                LabelNumber *ln = labbinder->labinternlab;
                if (ln == NULL) ln = labbinder->labinternlab = nextlabel();
                start_new_basic_block(ln);
            }
            cg_count(x->fileline);
            x = cmd2c_(x);
            continue;

case s_goto:
            {   LabBind *labbinder = cmd1lab_(x);
                LabelNumber *l = labbinder->labinternlab;
                if (l == NULL) l = labbinder->labinternlab = nextlabel();
                if (labbinder->labuses & l_defined)  /* else bind.c err msg */
                { emitsetspgoto(active_binders, l);
                  emitbranch(J_B, l);
                }
            }
            break;

case s_semicolon:
            cg_exprvoid(cmd1e_(x));
            break;

case s_block:
            {   BindList *sl = active_binders;
                int32 d = current_stackdepth;
                CmdList *cl = cmdblk_cl_(x);
                SynBindList *bl = cmdblk_bl_(x);
                cg_bindlist(bl, 0);
                if (usrdbg(DBG_VAR))
                {   current_env = (BindListList *)
                          binder_cons2(current_env, binderise(bl));
                    /* Hmm, this code is in flux pro tem. but the idea is   */
                    /* that we have to put debug scope info into block      */
                    /* heads so that is cannot get deadcoded away (discuss) */
                    (void)start_new_basic_block(nextlabel());
                }
                while (cl!=NULL)
                {   cg_cmd(cmdcar_(cl));
                    cl = cdr_(cl);
                }
                if (usrdbg(DBG_VAR))
                {   current_env = current_env -> bllcdr;
                    (void)start_new_basic_block(nextlabel());
                }
                emitsetsp(J_SETSPENV, sl);
                current_stackdepth = d;
            }
            break;     /* from switch */
        }
        break;         /* from loop */
    }
    cg_current_cmd = oldcmd;
}

static void cg_test1(Expr *x, bool branchtrue, LabelNumber *dest)
{
    VRegnum r;
    int32 bc, bcl;
    if (x == 0) { syserr(syserr_missing_expr); return; }
    verify_integer(x);
    if (usrdbg(DBG_LINE) && hasfileline_(h0_(x)) && x->fileline != 0)
        emitfl(J_INFOLINE, *x->fileline);
    if (integer_constant(x))
    {   if ((result2==0 && !branchtrue) ||
            (result2!=0 && branchtrue)) emitbranch(J_B, dest);
        return;
    }
    else switch (h0_(x))
    {

default:    r = cg_expr(x);
            emit(J_CMPK+ (branchtrue ? Q_NE : Q_EQ), GAP, r, 0);
            bfreeregister(r);
            emitbranch(J_B+ (branchtrue ? Q_NE : Q_EQ), dest);
            return;

case_s_any_string
            if (branchtrue) emitbranch(J_B, dest);
            return;

case s_andand:
            if (branchtrue)
            {   LabelNumber *l = nextlabel();
                cg_test1(arg1_(x), 0, l);
                cg_count(cg_current_cmd->fileline);
                cg_test1(arg2_(x), 1, dest);
                start_new_basic_block(l);
                return;
            }
            else
            {   cg_test1(arg1_(x), 0, dest);
                cg_count(cg_current_cmd->fileline);
                cg_test1(arg2_(x), 0, dest);
                return;
            }

case s_oror:
            if (!branchtrue)
            {   LabelNumber *l = nextlabel();
                cg_test1(arg1_(x), 1, l);
                cg_count(cg_current_cmd->fileline);
                cg_test1(arg2_(x), 0, dest);
                start_new_basic_block(l);
                return;
            }
            else
            {   cg_test1(arg1_(x), 1, dest);
                cg_count(cg_current_cmd->fileline);
                cg_test1(arg2_(x), 1, dest);
                return;
            }

case s_boolnot:
            cg_test1(arg1_(x), !branchtrue, dest);
            return;

case s_comma:
            cg_exprvoid(arg1_(x));
            cg_test1(arg2_(x), branchtrue, dest);
            return;

case s_monplus:
/* Monadic plus does not have to generate any code                       */
            cg_test1(arg1_(x), branchtrue, dest);
            return;

/*
case s_and:    may turn into J_BIT
*/

case s_equalequal:
            bc = Q_NE, bcl = Q_UNE;
            break;

case s_notequal:
            bc = Q_EQ, bcl = Q_UEQ;
            break;

case s_greater:
            bc = Q_LE, bcl = Q_LS;
            break;

case s_greaterequal:
            bc = Q_LT, bcl = Q_LO;
            break;

case s_less:
            bc = Q_GE, bcl = Q_HS;
            break;

case s_lessequal:
            bc = Q_GT, bcl = Q_HI;
            break;

    }
    {   int32 rep = mcrepofexpr(arg1_(x));
        if (rep==0x02000008 || rep==0x02000008+MCR_ALIGN_DOUBLE)
            cg_condjump(J_CMPDR, arg1_(x), arg2_(x), DBLREG,
                        (branchtrue ? Q_NEGATE(bc) : bc), dest);
        else if (rep==0x02000004)
            cg_condjump(J_CMPFR, arg1_(x), arg2_(x), FLTREG,
                        (branchtrue ? Q_NEGATE(bc) : bc), dest);
        else if ((rep >> MCR_SORT_SHIFT) == 1 ||    /* unsigned */
                  rep == 0x04000001)                /* plain char */
            cg_condjump(J_CMPR, arg1_(x), arg2_(x), INTREG,
                        (branchtrue ? Q_NEGATE(bcl) : bcl), dest);
        else if ((rep >> MCR_SORT_SHIFT) == 0)      /* signed */
            cg_condjump(J_CMPR, arg1_(x), arg2_(x), INTREG,
                        (branchtrue ? Q_NEGATE(bc) : bc), dest);
        else syserr(syserr_cg_badrep, (long)rep);
    }
    return;
}

static void cg_cast2(VRegnum r1, VRegnum r, int32 mcmode, int32 mclength)
{
    if (mcmode == 0) emit(J_EXTEND, r1, r, mclength);
                else emit(J_ANDK, r1, r, lowerbits(8*mclength));
}

static VRegnum cg_cast1(Expr *x1, int32 mclength, int32 mcmode)
{
    int32 arglength = mcrepofexpr(x1);
    int32 argmode = arglength >> MCR_SORT_SHIFT;
    VRegnum r, r1;
    RegSort rsort = (mcmode!=2) ? INTREG : mclength==4 ? FLTREG : DBLREG;
    arglength &= MCR_SIZE_MASK;
    if (mclength==0) return cg_exprvoid(x1);  /* cast to void */
#ifdef SIMPLIFY_OPTIMISE_CHAR_AND_SHORT_ARITHMETIC
    if (mcmode == 4)
    {   /* @@@ LDS 13Aug89 (non-cast) to 'plain' type - i.e. load narrow */
        /* integer in most efficient manner, irrespective of real type.  */
        /* Used to suppress s/u bits on J_LDR[B|W]Xx jopcodes.           */
        /* NOTE: guaranteed that *x1 IS a Binder (by simplify.optimise0) */
        /* so the call to cg_var IS appropriate.                         */
        return cg_var(GAP, (Binder *)x1, s_content, mcmode, arglength, 0);
    }
#endif
    r = cg_expr(x1);
    if (mcmode==3 || argmode==3)
    {   if (mcmode==argmode && mclength==arglength) return r;
        else syserr(syserr_cg_cast);
    }

    if (mcmode==argmode) switch(mcmode)
    {
case 0:     /* change width of integer */
/* The test mclength==4 on the next and subsequent lines is a           */
/* temporary hack for TARGET_IS_ADENART on the way to full 64-bit ops.  */
        if (mclength>=arglength || mclength==4) return r;
        emit(J_EXTEND, r1=fgetregister(INTREG), r,
/* Yukky coding here 0 is EXTENDBW, 1 is EXTENDBL, 2 is EXTENDWL...
 * this oddity should be changed sometime later - or at the very least
 * lifted into some enumeration or set of #define constants.
 */
             mclength == 1 ?
               (arglength == 2 ? 0 : 1) : 2);
        bfreeregister(r);
        return r1;
case 1:     /* change width of (unsigned) */
/* The test mclength==4 on the next and subsequent lines is a           */
/* temporary hack for TARGET_IS_ADENART on the way to full 64-bit ops.  */
        if (mclength>=arglength || mclength==4) return r;
        emit(J_ANDK, r1=fgetregister(INTREG), r, lowerbits(8*mclength));
        bfreeregister(r);
        return r1;
case 2:     /* change width of float */
        if (mclength==arglength) return r;
        r1 = fgetregister(rsort);
        if (mclength==4 && arglength==8)
            emitreg(J_MOVDFR, r1, GAP, r);
        else if (mclength==8 && arglength==4)
            emitreg(J_MOVFDR, r1, GAP, r);
        else syserr(syserr_cg_fpsize, (long)arglength, (long)mclength);
        bfreeregister(r);
        return r1;
default:
        if (mclength==arglength) return r;
        syserr(syserr_cg_cast1, (long)mcmode);
        return GAP;
    }
    else if (mcmode==2)
    {   /* floating something */
/* @@@ LDS 23Aug89 - This comment used to say:				 */
/* "Earlier parts of the compiler ensure that it is only necessary to    */
/* cope with full 32-bit integral types here. Such things as (float) on  */
/* a character are dealt with as (float)(int)<char> with the inner cast  */
/* explicit in the parse tree."                                          */
/* This is no longer true (move of cast optimisation to optimise1()) and */
/* is clearly nonesense, as this function throws away integer widening   */
/* casts in cases 0 and 1 (signed and unsigned) of the if () above.      */
/*      if (arglength!=4) syserr(syserr_cg_cast3, (long)arglength); */
#ifdef TARGET_LACKS_UNSIGNED_FIX
        if (argmode != 0)    /* unsigned float - simulate with signed */
        {   VRegnum r2;
            emit(J_EORK, r2 = fgetregister(INTREG), r, 0x80000000);
            emitreg(J_FLTDR+J_SIGNED, r1 = fgetregister(DBLREG), GAP, r2);
            bfreeregister(r2);
/* N.B. fc_two_31 is double - but this trick won't work with single      */
/*      so I do it in double and then convert down to single precision   */
            emitfloat(J_MOVDK, r2 = fgetregister(DBLREG), GAP, fc_two_31);
            emitreg(J_ADDDR, r1, r1, r2);
            bfreeregister(r2);
/* the next line should recursively call cg_cast() or cg_expr1() so we   */
/* check that we are not running out of regs.                            */
            if (mclength==4)
            {   VRegnum r3 = fgetregister(FLTREG);
                emitreg(J_MOVDFR, r3, GAP, r1); /* shorten */
                bfreeregister(r1);
                r1 = r3;
            }
        }
        else
#endif /* TARGET_LACKS_UNSIGNED_FIX */
        {   int32 w = (argmode==0) ? J_SIGNED : J_UNSIGNED;
            r1 = fgetregister(rsort);
            emitreg((rsort==FLTREG ? J_FLTFR : J_FLTDR) + w, r1, GAP, r);
        }
        bfreeregister(r);
        return r1;
    }
    else if (argmode==2)
    {   /* fixing something */
#ifdef TARGET_LACKS_UNSIGNED_FIX
/* N.B. the mclength==4 test in the next line is to produce shorter code */
/* for (unsigned short)(double)x.  It implies that this is calculated as */
/* (unsigned short)(int)(double)x.                                       */
        if (mcmode != 0 && mclength == 4)
        {
/* Fixing to an unsigned result is HORRIBLE, and is done using lots of   */
/* instructions here. The idea is to subtract 2**31 and fix, and then    */
/* to add back 2**31 to the resulting integer. Rounding toward 0 means   */
/* that we must test the sign first and calculate:                       */
/*     fixu(x) = x<2^31 ? fixs(x) : fixs(x-2^31) + 2^31                  */
/* If you want an overflow check then change the condition to            */
/*     0<=x<2^31.                                                        */
/* The trick only works if I use double precision, so for                */
/* float I start off by widening the input.                              */
/* THIS MUST BE DONE BY THE BACK-END SOON                                */
            {   VRegnum r2;
                LabelNumber *l1, *l2;
                if (arglength==4)
                {   VRegnum rx = fgetregister(DBLREG);
                    emitreg(J_MOVFDR, rx, GAP, r);
                    bfreeregister(r);
                    r = rx;             /* Now treat as double case      */
                }
                blkflags_(bottom_block) |= BLKREXPORTED;
                emitfloat(J_MOVDK, r2 = fgetregister(DBLREG), GAP, fc_two_31);
                emitreg(J_CMPDR + Q_GE, GAP, r, r2);
                emitbranch(J_B + Q_GE, l1 = nextlabel());
                emitreg(J_FIXDR+J_SIGNED, r1 = fgetregister(INTREG), GAP, r);
                emitbranch(J_B, l2 = nextlabel());
                start_new_basic_block(l1);
                emitreg(J_SUBDR, r, r, r2);   /* use 3-addr arm op one day? */
                emitreg(J_FIXDR+J_SIGNED, r1, GAP, r);
                emit(J_EORK, r1, r1, 0x80000000);
                start_new_basic_block(l2);
                bfreeregister(r2);
            }
        }
        else
            emitreg((arglength==4 ? J_FIXFR : J_FIXDR) + J_SIGNED,
                    r1 = fgetregister(INTREG), GAP, r);  /* see N.B. above */
#else  /* TARGET_LACKS_UNSIGNED_FIX */
#ifdef TARGET_IS_370          /* rationalise soon */
        {   VRegnum rx = fgetregister(DBLREG);
            /* move to new register as J_FIX corrupts its f.p. arg   */
            emitreg(arglength==4 ? J_MOVFDR : J_MOVDR, rx, GAP, r);
            emitreg(J_FIXDR + (mcmode==0 ? J_SIGNED : J_UNSIGNED),
                    r1 = fgetregister(INTREG), GAP, rx);
            bfreeregister(rx);
        }
#else  /* TARGET_IS_370 */
        {   int32 w = (mcmode == 0) ? J_SIGNED : J_UNSIGNED;
            emitreg((arglength==4 ? J_FIXFR : J_FIXDR) + w,
                    r1 = fgetregister(INTREG), GAP, r);
        }
#endif /* TARGET_IS_370 */
#endif /* TARGET_LACKS_UNSIGNED_FIX */
/* If I do something like (short)<some floating expression> I need to    */
/* squash the result down to 16 bits.                                    */
        if (mclength < 4)
            cg_cast2(r1, r1, mcmode, mclength);
        bfreeregister(r);
        return r1;
    }
    else if (arglength==4 && mclength==4) return r;
    else if (mcmode==0 || mcmode==1)
    {   if (mclength>=4) return r;
        cg_cast2(r1=fgetregister(INTREG), r, mcmode, mclength);
        bfreeregister(r);
        return r1;
    }
    else
    {   syserr(syserr_cg_cast2, (long)mcmode, (long)mclength,
                                (long)argmode, (long)arglength);
        return GAP;
    }
}

#ifdef TARGET_LDRK_MAX          /* not all machines have */
/* At some point we might be willing for a xxx/target.h to specify a    */
/* macro/fn for (a renamed) cg_limit_displacement to exploit machines   */
/* like ARM which do not have a proper notion of quantum/min/max.       */
static int32 cg_limit_displacement(int32 n, bool flt, int32 mclength,
                                   int32 flag)    /* nasty arg!         */
{
        int32 mink = TARGET_LDRK_MIN, maxk = TARGET_LDRK_MAX, span;
        int32 quantum = 1;
#ifdef TARGET_LDRFK_MAX
        if (flt)    /* Test if floating/double */
            mink = TARGET_LDRFK_MIN, maxk = TARGET_LDRFK_MAX;
#endif
#ifdef TARGET_LDRK_QUANTUM
        /* @@@ sadly this code misses the later determined ldrvk's!!!  */
        /* **MAYBE** these are alright?                                */
        quantum = target_ldrk_quantum(mclength, flt);
#endif
#ifdef TARGET_LACKS_HALFWORD_STORE
/* The prohibition on n=maxk when a store is to take place is to allow */
/* STRW r1,r2,n to map onto STRB r1,r2,n; STRB r1,r2,n+1.              */
        if (flag != s_content && mclength == 2 && !flt) maxk--;
#endif
        span = maxk - mink + 1;

/* I insist that span be a power of two, so that the inserted offset   */
/* in the code given below has a neat machine representation.  On many */
/* machines all is OK already, but on the ARM span as defined above is */
/* one less than a power of two.  Fix it up...                         */
/* AM: I think 'neat' above means 'ARM-neat(8 bits)'.                  */

        {   int32 w = 0;
            do
            {   span -= w;
                w = span & (-span);   /* least significant bit */
            } while (span != w);      /* stop when span has just 1 bit */
        }

/* If n is between mink and maxk (and quantum accessible) we want to    */
/* return it directly, else generate a 'least bits' additive fixup.     */
/* The following expression is believed algebraically equivalent to     */
/* ACN's in the case quantum=1.  @@@ Unify with take_neat_address.      */
/* Note the assumption that (mink & -quantum) == mink!                  */
        return (-quantum) &
            ((n > maxk) ? mink + (n - mink) % span :
             (n < mink) ? maxk - (maxk - n) % span : n);
}
#endif /* TARGET_LDRK_MAX */

static VRegnum cg_stind(VRegnum r, Expr *x, const int32 flag, bool topalign,
                        const int32 mcmode, const int32 mclength,
                        bool address, bool volatilep)
/* now combines effect of old cg_content.                                */
/* calculates:   *x       if flag==s_content                             */
/*               *x = r   if flag==s_assign                               */
/*      prog1(*x,*x = r)  if flag==s_displace                            */
/* The above values are the ONLY valid values for flag                   */
{   J_OPCODE ld_op;
    Expr *x1, *x2 = NULL;
    int32 n = 0, shift = 0, postinc = 0, down = 0;
    /* n.b. could add down to addrmode sometime */
    enum Addr_Mode { AD_RD, AD_RR, AD_VD } addrmode = AD_RD;
    RegSort rsort = mcmode!=2 ? INTREG : (mclength==4 ? FLTREG : DBLREG);
#ifdef ADDRESS_REG_STUFF
    if (rsort==INTREG && address) rsort = ADDRREG;
#else
    IGNORE(address);
#endif
    switch (h0_(x))
    {
case s_plus:
        x1 = arg1_(x), x2 = arg2_(x);
#ifdef TARGET_HAS_SCALED_ADDRESSING
/* one might wonder whether the test for is_shifted or s_integer should  */
/* come first.  Observe that either is correct, and the sensible code    */
/* can never have both on an indirection.  E.g.  *(int *)(3 + (x<<7)).   */
/* Moreover, if the machine cannot support all these modes then some are */
/* killed below.                                                         */
        if (is_shifted(x1, mclength))
        {   Expr *x3 = x1;
            x1 = x2;
            x2 = shift_operand(x3);
            shift = shift_amount(x3);
            addrmode = AD_RR;
        }
        else if (is_shifted(x2, mclength))
        {   shift = shift_amount(x2);
            x2 = shift_operand(x2);
            addrmode = AD_RR;
        }
        else
#endif /* TARGET_HAS_SCALED_ADDRESSING */
             if (h0_(x1)==s_integer) n = intval_(x1), x1 = x2;
        else if (h0_(x2)==s_integer) n = intval_(x2);
        else addrmode = AD_RR;
        break;
case s_minus:
        x1 = arg1_(x), x2 = arg2_(x);
#ifdef TARGET_HAS_SCALED_ADDRESSING
        if (is_shifted(x1, mclength))
        {   Expr *x3 = x1;
            x1 = mk_expr1(s_neg, type_(x), x2);
            x2 = shift_operand(x3);
            shift = shift_amount(x3);
            addrmode = AD_RR;
        }
/* why on earth is it interesting if x2 is shifted if we don't have */
/* negative-indexing?                                                  */
/* ACN to check and move is_shifted test into conditional code?        */
        else if (is_shifted(x2, -mclength))  /* Mark operand as subtracted */
#ifdef TARGET_HAS_NEGATIVE_INDEXING
        {   shift = shift_amount(x2);
            if ((shift & (SHIFT_RIGHT|SHIFT_ARITH)) == (SHIFT_RIGHT|SHIFT_ARITH))
/* error message -> miperrs.h soon, and re-word! */
                cc_rerr("iffy arithmetic shift\n");
            x2 = shift_operand(x2);
            down = J_NEGINDEX, addrmode = AD_RR;
        }
        else if (h0_(x2)==s_integer) n = -intval_(x2);
        else down = J_NEGINDEX, addrmode = AD_RR;
#else /* !TARGET_HAS_NEGATIVE_INDEXING */
        {   shift = shift_amount(x2);
            x2 = shift_operand(x2);
            x2 = mk_expr1(s_neg, typeofexpr(x2), x2);
            addrmode = AD_RR;
        }
        else if (h0_(x2)==s_integer) n = -intval_(x2);
        else x1 = x, x2 = 0;
#endif /* TARGET_HAS_NEGATIVE_INDEXING */
#else /* TARGET_HAS_SCALED_ADDRESSING */
        if (h0_(x2)==s_integer) n = -intval_(x2);
        else x1 = x, x2 = 0;
#endif /* TARGET_HAS_SCALED_ADDRESSING */
        break;
case s_displace:
        {   Expr *v = arg1_(x), *x3 = arg2_(x);
            if (h0_(x3) == s_plus &&
                is_same(arg1_(x3),v) && integer_constant(arg2_(x3)))
            {   postinc = result2;
                x = v;
            }
        }
        /* drop through */
default:
        x1 = x, x2 = 0;
        break;
    }
/* This code was, and may be better a procedure.                            */
/* Sample observations: x2 is valid only if addrmode == AD_RR (2 reg addr)  */

/* Now make allowance for the limited offsets available with LDRK/STRK      */
#ifdef TARGET_LDRK_MAX
    if (addrmode == AD_RD)
    {   int32 n1 = cg_limit_displacement(n, mcmode==2, mclength, flag);
        if (n1 != n)
            x1 = mk_expr2(s_plus, type_(x), x1, mkintconst(te_int, n-n1, 0)),
            n = n1;
    }
#endif /* TARGET_LDRK_MAX */

/* The following line needs regularising w.r.t. machines, like ARM       */
/* where fp/halfword addressing differs from int/byte.                   */
#ifdef TARGET_IS_ARM
    if (addrmode != AD_RD && (mcmode==2 || (flag!=s_content && mclength==2)))
/* Floating point can not support register indexed address modes         */
/* so I convert back to something more simple.                           */
/* Similarly the way I cope with (storing) halfwords can only cope with  */
/* reg+disp address modes, so I map onto the easier case here.           */
/* If what we have is *(a+(b+k)), it's a good idea to rewrite as *((a+b)+k) */
    {   if ((shift & SHIFT_RIGHT) == 0 && (h0_(x2) == s_plus || h0_(x2) == s_minus)) {
            Expr *x3 = arg1_(x2), *x4 = arg2_(x2);
            if (h0_(x3) == s_integer && h0_(x2) == s_plus) {
                Expr *t = x4; x4 = x3, x3 = t;
            }
            if (h0_(x4) == s_integer) {
                int32 n1;
                int32 d = (h0_(x2) == s_plus) ? down : down ^ J_NEGINDEX;
                n = intval_(x4);
                if (shift != 0) n = n << (shift & SHIFT_MASK);
                if (d) n = -n;
                n1 = cg_limit_displacement(n, mcmode==2, mclength, flag);
                if (n1 == n) {
                    addrmode = AD_RD;
                    x1 = mk_expr2((down != 0 ? s_minus : s_plus), type_(x), x1,
                                    mk_expr2(s_leftshift, type_(x3), x3, mkintconst(te_int, shift, 0)));
                }
            }
        }
        if (addrmode != AD_RD) {
            x1 = x;
            n = 0;                 /* just to make sure! */
            addrmode = AD_RD;
            down = 0;
        }
    }
#endif
#ifdef TARGET_LACKS_RR_STORE
    if (addrmode == AD_RR && flag != s_content && mcmode != 2)
/* i860 integer stores (@@@floating ones?) do not have RR forms, so      */
/* undo the above optimisations:                                         */
    {   x1 = x;
        n = 0;                 /* just to make sure! */
        addrmode = AD_RD;
        down = 0;
    }
#endif

  { VRegnum r1,r2,r99;
    Binder *x1b = 0;
/* The following test is probably in the wrong place.  Note that the      */
/* reduction to 4k boundaries above does not help if AD_VD then updates.  */
/* Further, use of AD_VD mode probably does not help at all if the binder */
/* is a long way from SP or FP.                                           */
/* @@@ For keeping IP sane, AM thinks we should mark IP as not-available  */
/* if stack frame size (plus largest offset in AD_VD) exceeds single      */
/* register addressability range.   Views?                                */
    if (addrmode == AD_RD && postinc == 0)
    {   x1b = is_local_adcon(x1);
/*            *(&<local> + n)   will use VD addressing mode                */
        if (x1b) addrmode = AD_VD;
    }
    if (addrmode != AD_VD)
    {   if (addrmode == AD_RD) r1 = cg_expr(x1), r2 = GAP;
/* is_same(x1,x2) is not worth testing for addresses                     */
        else if (nastiness(x1) < nastiness(x2))
            r2 = cg_expr(x2), r1 = cg_expr(x1);
        else
            r1 = cg_expr(x1), r2 = cg_expr(x2);
    }
    else r1 = r2 = GAP; /* To avoid dataflow anomalies */
    r99 = (flag == s_assign) ? GAP : fgetregister(rsort);
    switch (mcmode)
    {
case 0: /* signed */
case 1: /* unsigned */
#ifdef SIMPLIFY_OPTIMISE_CHAR_AND_SHORT_ARITHMETIC
case 4: /* plain... i.e. neither signed nor unsigned */
#endif
        switch (mclength)
        {
    case 1: ld_op = J_LDRBK; break;
    case 2: ld_op = J_LDRWK; break;
    case 4: ld_op = J_LDRK; break;
#ifdef TARGET_IS_ADENART
    case 8: ld_op = J_LDRK|J_ALIGN8; break;
#else
    case 8: ld_op = J_LDRLK; break;
#endif
    default: syserr(syserr_cg_bad_width, (long)mclength); ld_op = J_NOOP;
        }
        break;
case 2: ld_op = rsort==FLTREG ? J_LDRFK:J_LDRDK;
        break;
default:
        syserr(syserr_cg_bad_mode, (long)mcmode); ld_op = J_NOOP;
        break;
    }
#ifdef NEW_J_ALIGN_CODE             /* was TARGET_IS_ADENART */
/* Although this code is just for a special machine (which encourages   */
/* all 64-bit aligned loads to be done with J_LDRLK), the idea should   */
/* be generalisable (e.g. to improve the TARGET_LACKS_HALFWORD_STORE    */
/* code below).                                                         */
/* We convert ld/st 8/16/32 bits into 64-bit rd/wr, sign-/zero-         */
/* extending on load 8/16 bits and write beyond the item into           */
/* space which alignof_toplevel guarantees is junk.                     */
    if (topalign)
        ld_op |= alignof_toplevel == 8 ? J_ALIGN8 :
                 alignof_toplevel == 4 ? J_ALIGN4 :
                 J_ALIGN4;
#endif
#ifdef TARGET_IS_SPARC
    if (ld_op == J_LDRDK) ld_op |= topalign ? J_ALIGN8 : J_ALIGN4;
#endif
    if (flag != s_assign)
    {   int32 ld_op_su = mclength >= 4 ? ld_op :
#ifdef SIMPLIFY_OPTIMISE_CHAR_AND_SHORT_ARITHMETIC
                         mcmode == 4 ? ld_op :
#endif
                         mcmode == 0 ? ld_op+J_SIGNED : ld_op+J_UNSIGNED;
        if (addrmode == AD_RD) emit(ld_op_su, r99, r1, n);
        else if (addrmode == AD_VD) emitvk(J_addvk(ld_op_su), r99, n, x1b);
        else emit5(J_KTOR(ld_op_su) + down, r99, r1, r2, shift);
#ifdef never /* TARGET_IS_ADENART */
        if (ld_op == J_LDRLK && mclength < 4)
            cg_cast2(r99, r99, mcmode, mclength);
#endif
    }
    if (flag != s_content)
    {
#ifdef TARGET_LACKS_HALFWORD_STORE
/* It would be nice it we could make the back-end do this fabrication.     */
/* We do not do so yet as it is unclear how the work register 'rx' could   */
/* could be passed.                                                        */
        if (ld_op == J_LDRWK)
        {   VRegnum rx = fgetregister(INTREG);
            int32 a_msb, a_lsb;
            if (addrmode == AD_RR)
                syserr(syserr_cg_indexword);
            if (target_lsbytefirst)
            {   a_lsb = n;  a_msb = n+1;}
            else
            {   a_msb = n;  a_lsb = n+1;}
            if (addrmode == AD_VD) emitvk(J_STRBVK, r, a_lsb, x1b);
            else emit(J_STRBK, r, r1, a_lsb);
            emit(J_SHRK+J_SIGNED, rx, r, 8);
            if (addrmode == AD_VD) emitvk(J_STRBVK, rx, a_msb, x1b);
            else emit(J_STRBK, rx, r1, a_msb);
            bfreeregister(rx);
        }
        else
#endif
        {   J_OPCODE st_op = J_LDtoST(ld_op);
            if (addrmode == AD_RD) emit(st_op, r, r1, n);
            else if (addrmode == AD_VD) emitvk(J_addvk(st_op), r, n, x1b);
            else emit5(J_KTOR(st_op) + down, r, r1, r2, shift);
        }
    }
/* Here there had been a s_displace within the indirection, and I do the */
/* update part of it.                                                    */
    if (addrmode != AD_VD)
    {   if (postinc != 0)
        {   emit(J_ADDK, r1, r1, postinc);
            cg_storein(r1, x, s_assign); /* really a call to cg_var so far */
        }
        bfreeregister(r1);
    }
    if (addrmode == AD_RR) bfreeregister(r2);
    if (flag == s_content)
    {   if (volatilep) emituse(r99, rsort);
        return r99;
    }
    if (flag == s_displace)
    {   bfreeregister(r);
        if (volatilep) emituse(r99, rsort);
        return r99;
    }
    return r;
  }
}

static VRegnum chroma_check(VRegnum v)
/* This code goes to some trouble to see if there would be a real register */
/* available for use, even though register allocation happens later. This  */
/* behaviour is so that I have something on which to base an heuristic     */
/* relating to spilling things to the stack.                               */
{   if (isintregtype_(vregsort(v)))
    {   usedregs = mkRegList(usedregs, v);
        if (++nusedregs <= NTEMPREGS+NARGREGS+NVARREGS) return v;
    }
    else
    {   usedfpregs = mkRegList(usedfpregs, v);
#ifdef TARGET_SHARES_INTEGER_AND_FP_REGISTERS
        if ((nusedregs+=2) <= NTEMPREGS+NARGREGS+NVARREGS) return v;
#else
        if (++nusedfpregs <= NFLTTEMPREGS+NFLTARGREGS+NFLTVARREGS) return v;
#endif
    }
    /* The next line can happen either if:
     *  1. target.h gives NTEMPREGS=NARGREGS=NVARREGS=0  or
     *  2. an op like unsigned fix takes a number of temporaries larger
     *     that the 4 allowed for below (this is delicate, see cg_expr1()).
     */
    syserr(syserr_chroma);
    return v;
}

static VRegnum reserveregister(RegSort precision)
{   nreservedregs++;
    return vregister(precision);
}

static VRegnum getreservedreg(VRegnum r)
{   nreservedregs--;
    return chroma_check(r);
}

static VRegnum fgetregister(RegSort rtype)
{
    return chroma_check(vregister(rtype));
}

static void bfreeregister(VRegnum r)
{
    if (r == GAP) return;
    if (member(r, usedregs))
    {   nusedregs--;
        usedregs = ndelete(r, usedregs);
    }
    else if (member(r, usedfpregs))
    {
#ifdef TARGET_SHARES_INTEGER_AND_FP_REGISTERS
        nusedregs -= 2;
#else
        nusedfpregs--;
#endif
        usedfpregs = ndelete(r, usedfpregs);
    }
/* There used to be a syserr() trap here that checked that registers were  */
/* discarded tidily - under the newer order of things it has gone away.    */
}

#define jop_iscmp_(op) (((op)&~Q_MASK)==J_CMPR || \
                        ((op)&~Q_MASK)==J_CMPFR || \
                        ((op)&~Q_MASK)==J_CMPDR)

static J_OPCODE Q_swap(J_OPCODE op)
{   /* If the bit patterns for the codes were more sensible then this could */
    /* be cheaper.  As it stands more cases coalesce than the code admits.  */
    switch (op & Q_MASK)
    {   default: syserr(syserr_Q_swap, (long)op);
        case Q_UEQ: case Q_UNE:
        case Q_EQ: case Q_NE: return op;
        case Q_LT: case Q_GT: return op ^ (Q_LT ^ Q_GT);
        case Q_LE: case Q_GE: return op ^ (Q_LE ^ Q_GE);
        case Q_LO: case Q_HI: return op ^ (Q_LO ^ Q_HI);
        case Q_LS: case Q_HS: return op ^ (Q_LS ^ Q_HS);
    }
}

static bool is_same(Expr *a, Expr *b)
/* Expressions are considered the same if they compute the same value    */
/* and do not have any side-effects.                                     */
{
    AEop op;
    for (;;)
    {   if ((op=h0_(a))!=h0_(b)) return 0;
        if (isvolatile_expr(a) || isvolatile_expr(b)) return 0;
        switch (op)
        {
    case s_binder:
            {   if (a==b) return 1;  /* volatile attribute already checked */
                else return 0;
            }
    case s_integer:
            if (intval_(a)==intval_(b)) return 1;
            else return 0;
    case_s_any_string
    case s_floatcon:
            if (a==b) return 1;    /* improve? */
            else return 0;
    case s_dot:
            if (exprdotoff_(a) != exprdotoff_(b)) return 0;
            a = arg1_(a), b = arg1_(b);
            continue;
    case s_cast:
/* equivtype is probably too strong on the next line (we should         */
/* probably use a more machine-oriented test on types) but, before      */
/* changing it to the logical mcrepofexpr()=mcrepofexpr(), note that    */
/* casts on empty arrays to pointers can cause mcrepofexpr() to cc_err. */
/* @@@ Then again, perhaps is_same should be elided by cse.c now?       */
            if (!equivtype(type_(a), type_(b))) return 0;
    case s_addrof:
    case s_bitnot:
    case s_boolnot:
    case s_neg:
    case s_content:
    case s_monplus:
            a = arg1_(a);
            b = arg1_(b);
            continue;
    case s_andand:
    case s_oror:
    case s_equalequal:
    case s_notequal:
    case s_greater:
    case s_greaterequal:
    case s_less:
    case s_lessequal:
    case s_comma:
    case s_and:
    case s_times:
    case s_plus:
    case s_minus:
    case s_div:
    case s_leftshift:
    case s_or:
    case s_rem:
    case s_rightshift:
    case s_xor:
            if (!is_same(arg1_(a), arg1_(b))) return 0;
            a = arg2_(a);
            b = arg2_(b);
            continue;
    default:
            return 0;
        }
    }
}

static Binder *is_local_adcon(Expr *a)
{   if (h0_(a) == s_addrof)
    {   Expr *w = arg1_(a);
        if (h0_(w) == s_binder)
        {   Binder *b = (Binder *)w;
            if ((bindstg_(b) & PRINCSTGBITS) == bitofstg_(s_auto))
                return b;
        }
    }
    return 0;
}

static Expr *take_address(Expr *e)
{
    if (h0_(e)==s_content) return arg1_(e);     /*   & * x   --->  x     */
    else if (h0_(e)==s_dot)                     /*   &(x.y)  ---> (&x)+y */
        return mk_expr2(s_plus,
                        ptrtotype_(type_(e)),
                        take_address(arg1_(e)),
                        mkintconst(te_int, exprdotoff_(e), 0));
    else if (h0_(e)==s_cast) return take_address(arg1_(e));  /* ignore casts */
    else return optimise0(mk_expr1(s_addrof,
                                   ptrtotype_(typeofexpr(e)),
                                   e));
}

/* This routine forges 'Binder's and so MUST be done in one place.       */
/* It is arguable that is should be more opportunistic about re-using    */
/* pre-existing adcons (e.g. use FLT ones for INTs on the ARM).          */
static Expr *take_neat_address(Binder *b, RegSort rsort)
/* b is a static or extdef binder                                        */
        {   int32 off = bindaddr_(b);
            BindList *bb = datasegbinders;
            Binder *bb_elt = NULL;
            int32 base = 0;
            int32 loctype = binduses_(b) & u_loctype;
#ifdef TARGET_LDRK_MAX
            int32 mink = TARGET_LDRK_MIN, maxk = TARGET_LDRK_MAX, span;

#ifdef TARGET_LDRFK_MAX
            if (!isintregtype_(rsort))        /* floating/double */
                mink = TARGET_LDRFK_MIN, maxk = TARGET_LDRFK_MAX;
#endif
#ifdef TARGET_IS_HELIOS
            {   extern int suppress_module;  /* Boo Hiss... */
/*
 * This hack has been put in at the express request of Perihelion for the
 * benefit of their Helios port for the ARM - it ensures that when the
 * magic -zr option is enabled (which itself has bizarre effects) then
 * addresses of static variables are all done separately rather than using
 * a single dataseg binder to mark the base of the current area of store.
 * When -zr is enabled the data segment will not get generated by the
 * compiler (!) so the fact that the labels referenced here will never seem
 * to get set is not a problem - their definitions are created by hand and
 * merged in with the code while building a shared kernel library.
 */
                if (suppress_module == 1) return take_address((Expr *)b);	/* XXX - NC */
            }
#endif
#ifdef TARGET_LACKS_HALFWORD_STORE
/* See cg_limit_displacement(), for this slight paranoia.               */
/* We could envisage targets where all of an object must be addressable */
            if (isintregtype_(rsort)) maxk--;
#endif
            /* The alignof_max is needed on machines like the i860.     */
            /* alignof_max acts like quantum in cg_limit_displacement.  */
            span = (maxk+1-mink) & -(int32)4 & -(int32)alignof_max;

/* I find a binder for a variable at (&datasegment+nnn) where nnn is a   */
/* neat number and the variable I want now is close to it.               */
/* The following code should use xxx % span as in cg_limit_displacement. */
/* @@@ In fact this whole code probably should be a call to              */
/* cg_limit_displacement with possibly a flag arg (they do same job!).   */

            while (off > maxk)
                base += span, off -= span;
            while (off < mink)
                base -= span, off += span;

#else
            IGNORE(rsort);
#endif /* TARGET_LDRK_MAX */

/* Find a binder for the address involved...                             */
            while (bb != NULL)
            {   bb_elt = bb->bindlistcar;
                if (bindaddr_(bb_elt) == base &&
                   (binduses_(bb_elt) & u_loctype) == loctype) break;
                bb = bb->bindlistcdr;
            }
            if (bb == NULL)
            {   bb_elt = genglobinder(te_int);  /* MUST be global */
                bindaddr_(bb_elt) = base;
                binduses_(bb_elt) |= loctype;
                datasegbinders =
                  (BindList *) global_cons2(SU_Other, datasegbinders, bb_elt);
            }
            if (off == 0) return take_address((Expr *)bb_elt);
            else return mk_expr2(s_plus, te_int,
                                         take_address((Expr *)bb_elt),
                                         mkintconst(te_int, off, 0));
        }

static VRegnum loadadcon(Binder *b)
/* only used once from cg_addr()                                         */
{
    VRegnum r = fgetregister(ADDRREG);
    emitbinder(J_ADCON, r, b);
    return r;
}

static void emituse(VRegnum r, RegSort rsort)
{
    if (isintregtype_(rsort)) emit(J_USE, r, GAP, 0);
    else if (rsort == FLTREG) emit(J_USEF, r, GAP, 0);
    else emit(J_USED, r, GAP, 0);
}

static VRegnum cg_var(VRegnum r, Binder *b, AEop flag,
                      int32 mcmode, int32 mclength,
                      bool address)
{
/* AM 22-10-86: New code amalgamating cg_var and (part of) cg_storein.    */
/*    flag is a member of {s_content,s_displace,s_assign}, see cg_stind(). */
/*    fixes bugs in &-locals and extern value fetch.                      */
/* variables can be of several flavours :                                */
/*    (a) local          reference offset from sp                        */
/*    (b) global         reference offset from &datasegment              */
/*    (c) constant       codegenerate associated value (done elsewhere)  */
/*    (d) external       indirect access necessary                       */
/* local vars (even char/short) are treated as possible register vars    */
/* (unless address taken - see below) and are treated as 4 byte values   */
/* with the top bits set up so that widening is done on STORE not on     */
/* load.  This seems a good strategy.                                    */
/* If they have address taken then this strategy does not work (consider */
/* signed short/char variables).  Hence they MUST be put on the stack,   */
/* and be properly accessed (this code fixed a bug in version 1.10)).    */
/* A related bug from extern loads loading (possibly incorrect) top bits */
/* is fixed at the same time.                                            */
/* Note that we must be extra careful with short/char vars whose address */
/* is taken on 370/68000 sex machines.                                   */

/* note that 'b' didn't ought to be a struct/union value.                */
    RegSort rsort = mcmode!=2 ? INTREG : (mclength==4 ? FLTREG : DBLREG);
    VRegnum r99 = GAP;  /* Never used but this saves dataflow anomaly */
    bool volatilep = isvolatile_expr((Expr *)b);
#ifdef ADDRESS_REG_STUFF
    if (rsort==INTREG && address) rsort = ADDRREG;
#else
    IGNORE(address);
#endif

/* Here I peephole out the sequence (STR x; LDR x) where x is a variable */
/* Experimentally skip for (potential) register variable for dataflow    */
/* see FEATURE_ANOMALY in regalloc.c                                     */
    if (flag == s_content && b == juststored && !volatilep
#ifdef EXPERIMENTAL_DATAFLOW
        && (bindstg_(b) & (bitofstg_(s_auto)|b_addrof)) != bitofstg_(s_auto)
#endif
       )
        return justregister;

    switch (bindstg_(b) & PRINCSTGBITS)
    {
default:
        syserr(syserr_cg_stgclass, (long)bindstg_(b));
case b_globalregvar:
case bitofstg_(s_auto):
        if (!((bindstg_(b) & b_addrof) && mclength < 4))
        {   /* We use LDRxV/STRxV for variables either whose address is  */
            /* not taken or which occupy 1 or 2 whole stack word(s).     */
            /* Avoid this for short/char &-taken locals, consider        */
            /*    extern short x=0; *(&x+0) = -1; f(x);                  */
            /* The indirect assignment to x cannot affect the top bits   */
            /* (e.g. maybe in procedure) and so LDRV cannot be used to   */
            /* load a pre-(sign/zero) padded version of x as it would be */
            /* if x is in a register.  Also mutter about machine sex.    */
            if (flag != s_assign)
            {   r99 = fgetregister(rsort);
                emitbinder(floatyop(rsort, J_LDRV, J_LDRFV, J_LDRDV), r99, b);
            }
/* N.B. local integer variables are ALWAYS stored in a 32-bit word even  */
/* if only 8 or 16 bits is needed. Hence STRV does not need length data  */
            if (flag != s_content)
                emitbinder(floatyop(rsort, J_STRV, J_STRFV, J_STRDV), r, b);
            break;
        }
        /* else drop through to use J_ADCONV via s_extern case.          */
        /* This is certainly NOT optimal code, but better than bugs.     */
/* The previous code here was buggy, but on the other hand, I like the
 * idea of using STRV/STRK (32 bits) for storing simple short/char
 * vars made feasible by the padding performed on simple variables by
 * vargen.c.  Of course this is not possible in general in cg_stind()
 * because of array elements.  Worry a bit too about oddsex machines.
 */
jolly:
        if (flag == s_content)    /* amalgamate */
            return cg_stind(r, take_address((Expr *)b), flag, YES,
                            mcmode, mclength, NO, volatilep);
        r = cg_stind(r, take_address((Expr *)b), flag, YES,
                          mcmode, mclength, NO, volatilep);
        flag = s_assign;
        break;
case bitofstg_(s_extern):
        if (bindstg_(b) & b_undef) goto jolly;
case bitofstg_(s_static):
        if (binduses_(b) & u_bss) goto jolly;
/* v1   ->    *(&datasegment + nnn)    when v1 is a static               */
/* See comment for s_extern above for possible improvement too.          */
/* AMALGAMATE the next two lines                                         */
        if (flag == s_content)
             return cg_stind(r, take_neat_address(b, rsort), flag, YES,
                             mcmode, mclength, NO, volatilep);
        r = cg_stind(r, take_neat_address(b, rsort), flag, YES,
                     mcmode, mclength, NO, volatilep);
        flag = s_assign;
        break;
    }
    if (flag == s_content)
    {   if (volatilep) emituse(r99, rsort);
        return r99;
    }
    if (flag != s_displace && !volatilep)
    {   juststored = b;
        justregister = r;
    }
    if (flag == s_displace)
    {   bfreeregister(r);
        if (volatilep) emituse(r99, rsort);
        return r99;
    }
    return r;
}

static VRegnum cg_storein(VRegnum r, Expr *e, AEop flag)
/* if flag is s_displace return old value,
   else flag is s_assign and return stored value (already coerced). */
{
    int32 mcrep = mcrepofexpr(e);
    int32 mcmode = mcrep >> MCR_SORT_SHIFT;
    int32 mclength = mcrep & MCR_SIZE_MASK;
    bool volatilep = isvolatile_expr(e);
    VRegnum res = GAP;

#ifdef SIMPLIFY_OPTIMISE_CHAR_AND_SHORT_ARITHMETIC
    if (mcmode == 4 && h0_(e) == s_cast)
/* @@@ LDS 28-Nov-89. Fix to <short-or-char>++ => syserr in cg_storein.  */
/* Cast to plain narrow integer. optimise2() guarantees that *arg1_(e)   */
/* IS a binder and that mclength is the same as for *arg1_(e).           */
    {   e = arg1_(e);
        volatilep = isvolatile_expr(e);
    }
#endif

    switch (h0_(e))
    {
case s_binder:
        res = cg_var(r, (Binder *)e, flag, mcmode, mclength, 0);
        break;

case s_dot:
        e = cg_content_for_dot(e);
        return cg_storein(r, e, flag);

case s_content:
        e = arg1_(e);
        if (memory_access_checks)
        {   Expr *fname = mclength==1 ? sim.writecheck1 :
                         mclength==2 ? sim.writecheck2 :
                         sim.writecheck4;
            e = mk_expr2(s_fnap, typeofexpr(e), fname,
                                                (Expr *)mkExprList(0, e));
        }
        res = cg_stind(r, e, flag, NO, mcmode, mclength, NO, volatilep);
        break;

/* The following case is caused by 's.m = 42; where s is a one-word     */
/* struct.  The problem is that optimise0 changes s.m to (typeof_m)s.   */
/* The solution below is rather nasty and doesn't even try to fix the   */
/* way that { (char)a = 1; } warns and then syserrs in pcc mode!        */
case s_cast:
        if ((mcrep & ~0x01000000) == (mcrepofexpr(arg1_(e)) & ~0x01000000))
            return cg_storein(r, arg1_(e), flag);
        /* drop through */
default:
        syserr(syserr_cg_storein, (long)h0_(e));
    }
    if (volatilep) emit(J_VSTORE, GAP, GAP, 0);
    return res;

}

static VRegnum cg_addr(Expr *sv, bool valneeded)
{
    VRegnum r;
    Expr *e;
    for (;;) switch (h0_(sv))
    {
case s_comma:   /* can occur in structure manipulation code */
        cg_exprvoid(arg1_(sv));
        sv = arg2_(sv);
        continue;

case s_cast:
        /*  & ((cast)x)    --->   & x               */
        sv = arg1_(sv);
        continue;

case s_cond:
        /*  & (p ? x : y)  --->   p ? (&x) : (&y)   */
        arg2_(sv) = take_address(arg2_(sv));
        arg3_(sv) = take_address(arg3_(sv));
        type_(sv) = typeofexpr(arg2_(sv));
        return cg_expr1(sv, valneeded);

case s_assign:
/* this can happen with structure assignments - it is essentially an     */
/* artefect of transformations made here in the codegenerator but seems  */
/* fairly convenient.                                                    */
/* In particular this is used in the case of                             */
/*      (a = b) . c                                                      */
/* which in effect turns into                                            */
/*      ((a = b), a . c)                                                 */
/* but with need for special care if a is an expression that might have  */
/* side effects when being evaluated.                                    */
        e = arg1_(sv);
ell:    switch (h0_(e))
        {
    default:
            syserr(syserr_cg_addr);
    case s_dot:
/*  &((p.q) =r )   =>   &(*(&p+q') = r)                                  */
            e = cg_content_for_dot(e);
            goto ell;
    case s_content:
           {    TypeExpr *t = typeofexpr(arg1_(e));
                Binder *gen = gentempbinder(t);
/*     & (*p=q)                                                          */
/*                turns into                                             */
/*     (let g;  g = p,  *g = q,  g)                                      */
/*  where the temp var is to ensure that p gets evaluated just once.     */
                sv = mk_exprlet(s_let,
/* @@@ The semantic analyser should export such a function ... */
                              t,
                              mkSynBindList(0, gen),
                              mk_expr2(s_comma,
                                       t,
                                       mk_expr2(s_assign,
                                                t,
                                                (Expr *)gen,
                                                arg1_(e)),
                                       mk_expr2(s_comma,
                                                t,
                                                mk_expr2(s_assign,
                                                         typeofexpr(arg2_(sv)),
                                                         mk_expr1(s_content,
                                                                  t,
                                                                  (Expr *)gen),
                                                arg2_(sv)),
                                       (Expr *)gen)));
                if (debugging(DEBUG_CG))
                {   eprintf("& p = q  ---> ");
                    pr_expr(sv);
                    eprintf("\n");
                }
            }
            return cg_expr1(sv, valneeded);
    case s_binder:
            cg_exprvoid(sv);  /* get the assignment done, voiding the result */
            sv = e;           /* then take address of the lhs variable       */
        }
        /* drop through */
case s_binder:
      { Binder *b = (Binder *)sv;
        switch (bindstg_(b) & PRINCSTGBITS)
        {
    default:
            syserr(syserr_cg_stgclass, (long)bindstg_(b));
    case bitofstg_(s_auto):
            if (valneeded)
            {   emitbinder(J_ADCONV, r = fgetregister(ADDRREG), b);
                return (r);
            }
            else return GAP;
    case bitofstg_(s_static):
    case bitofstg_(s_extern):
            if (valneeded) return loadadcon(b);
            else return GAP;
        }
      }
#ifdef SOFTWARE_FLOATING_POINT
case s_floatcon:
        if (!valneeded) return GAP;
        emitfloat((((FloatCon *)sv)->floatlen) & bitoftype_(s_double)
              ? J_ADCOND : J_ADCONF, r = fgetregister(ADDRREG), GAP,
                                     (FloatCon *)sv);
        return r;
#endif
default:
        syserr(syserr_cg_addr1, (long)h0_(sv));
    }
}


#if defined TARGET_HAS_SCALED_ADDRESSING || defined TARGET_HAS_SCALED_OPS
static int32 is_shifted(Expr *x, int32 mclength)
/* Predicate to test if an expression is of the form something shifted.  */
/* 11-Nov-87: changed to consult target_scalable() for 32016/vax.        */
/* Used to decide if scaled indexed addressing is wanted.                */
/*
 * mclength is zero for scaled-op investigation, or 1, 2, 4 or 8 for byte,
 * short, long or double memory references.  It is -1, -2, -4 or -8 for
 * memory references where the scaled index must be subtracted from the
 * base value rather than added. mclength is just handed down to a
 * target specific test for validity.
 */
{
    int32 n;
    Expr *arg;
    switch (h0_(x))
    {
case s_rightshift:
case s_leftshift:
        arg = arg2_(x);
        if (h0_(arg)!=s_integer) return 0;
        n = intval_(arg);
        if (n == 0) return is_shifted(arg1_(x), mclength);
        if (n>=0 && n<=31) return target_scalable(shift_amount(x), mclength);
        else return 0;
case s_times:                   /* detect multiplication by 2, 4, 6, ...  */
        if ((arg = arg1_(x), h0_(arg)) == s_integer ||
            (arg = arg2_(x), h0_(arg)) == s_integer)
        {   n = intval_(arg);
            if (n!=0 && (n & 1)==0)
                return target_scalable(shift_amount(x), mclength);
        }
        return 0;
default:
        return 0;
    }
}

static Expr *shift_op1(Expr *x, int32 n)
{
    if (n==0) syserr(syserr_cg_shift0);
    while ((n & 1)==0) n >>= 1;
    if (n==1) return x;
    return mk_expr2(s_times, te_int, x,
                    mkintconst(te_int, n, 0));
}

static Expr *shift_operand(Expr *x)
/* if is_shifted() returned true this can be used to extract the operand */
{
    int32 n;
    Expr *arg;
    switch (h0_(x))
    {
case s_rightshift:
case s_leftshift:
        arg = arg2_(x);
        if (h0_(arg) == s_integer && intval_(arg)==0)
            return shift_operand(arg1_(x));
        return arg1_(x);
case s_times:
        arg = arg1_(x);
        if (h0_(arg)==s_integer)
        {   n = intval_(arg);
            if ((n & 1)==0) return shift_op1(arg2_(x), n);
        }
        else return shift_op1(arg1_(x), intval_(arg2_(x)));
default:
        syserr(syserr_not_shift);
        return x;
    }
}

static int32 logbase2(int32 n)
{
    int32 r = 0;
    while ((n & 1)==0) n >>= 1, r++;
    return r;
}

static int32 shift_amount(Expr *x)
/* if is_shifted() returned true this can be used to extract the shift   */
{
    int32 n;
    Expr *arg;
    switch (h0_(x))
    {
case s_rightshift:
        n = intval_(arg2_(x));
        if (n == 0) return shift_amount(arg1_(x));
/* This is a horrid packing scheme */
        n = (n & SHIFT_MASK) | SHIFT_RIGHT;
        if (!unsigned_expression_(arg1_(x))) n |= SHIFT_ARITH;
        return n;
case s_leftshift:
        arg = arg2_(x);
        n = intval_(arg);
        if (n == 0) return shift_amount(arg1_(x));
        return n & SHIFT_MASK;
case s_times:
        arg = arg1_(x);
        if (h0_(arg)==s_integer)
        {   n = intval_(arg);
            if ((n & 1)==0) return logbase2(n) & SHIFT_MASK;
        }
        arg = arg2_(x);
        if (h0_(arg)==s_integer)
        {   n = intval_(arg);
            if ((n & 1)==0) return logbase2(n) & SHIFT_MASK;
        }
        /* drop through */
default:
        syserr(syserr_not_shift1);
        return 0;
    }
}

#endif /* TARGET_HAS_SCALED_ADDRESSING or _OPS */

#ifdef TARGET_HAS_SCALED_OPS
static VRegnum cg_opshift(J_OPCODE jop, Expr *arg1, Expr *arg2,
                          int32 shift)
/*
 * Note that the scheme we have here can only cope with constant scale
 * quantities - the ARM can support dynamic shift-amounts, as in
 *     ORR  r1, r2, r3, LSL r4
 * but same will not fit in with the limitations os Jopcode formats.
 */
{
    bool same = 0;
    VRegnum r, r2, targetreg;
    if (is_same(arg1,arg2)) (same = 1, r = r2 = cg_expr(arg1));
    else if (nastiness(arg1) < nastiness(arg2))
        r2 = cg_expr(arg2), r = cg_expr(arg1);
    else
        r = cg_expr(arg1), r2 = cg_expr(arg2);
    targetreg = (jop_iscmp_(jop) ? GAP : fgetregister(INTREG));
    emit5(jop, targetreg, r, r2, shift);
    bfreeregister(r);
    if (!same) bfreeregister(r2);
    return targetreg;
}
#endif /* TARGET_HAS_SCALED_OPS */

static void verify_integer(Expr *x)
/* @@@ probably overcautious and underused nowadays.                    */
{
    switch(mcrepofexpr(x))
    {
case 0x00000001:    /* signed integers are OK */
case 0x00000002:
case 0x00000004:
case 0x00000008:
case 0x01000001:    /* unsigned integers are OK */
case 0x01000002:
case 0x01000004:
case 0x01000008:
case 0x03000001:    /* structures/unions up to 4 bytes long are OK */
case 0x03000002:
case 0x03000003:
case 0x03000004:
case 0x04000001:    /* plain, short integers are OK */
case 0x04000002:
        return;
default:
        syserr(syserr_integer_expected);
    }
}

/* The code that follows is too simplistic - I need a better estimate of */
/* the complexity of expressions, e.g. an estimate of the number of regs */
/* needed to evaluate something.                                         */

static int nastiness(Expr *x)
{
/* This procedure is used to provide an heuristic that helps me decide   */
/* what order to prepare arguments for functions in. It a value between  */
/* ISCONST and ISHARD to show if the expression x contains any function  */
/* calls (which clobber lots of regs) or otherwise a value estimating    */
/* the likely number of registers the expression is liable to take.      */
    AEop op;
    int n1, n2, n3, nr;
#define max_(a,b) ((a)>=(b) ? (a):(b))
    for (;;) switch (op = h0_(x))
    {
case s_integer:
        return ISCONST;
case s_floatcon:
case_s_any_string
        return ISXCONST;
case s_binder:
        return ISBIND;
/* monadic things... */
case s_dot:
case s_content:
        {   int32 m = mcrepofexpr(x);
            if ((m >> MCR_SORT_SHIFT) == 3 && (m & MCR_SIZE_MASK) > 4)
                return ISHARD;  /* big struct */
        }
        if (memory_access_checks) return(ISHARD);
case s_cast:
case s_bitnot:
case s_boolnot:
case s_monplus:
case s_neg:
case s_addrof:
        x = arg1_(x);
        if (op == s_dot && h0_(x) == s_content) x = arg1_(x);
        /* (*x . 0) is not hard just because *x would be */
        continue;
case s_cond:
        /* due to the fact that operands from s_cond are never combined */
        /* we do not need the complexity of binary operators below.     */
        nr = nastiness(arg1_(x));
        n2 = nastiness(arg2_(x));
        nr = max_(nr,n2);
        n3 = nastiness(arg3_(x));
        nr = max_(nr,n3);
        return max_(ISEXPR, nr);
case s_assign:
case s_displace:
        {   int32 m = mcrepofexpr(x);
            if ((m >> MCR_SORT_SHIFT) == 3 && (m & MCR_SIZE_MASK) > 4)
                return ISHARD;  /* big struct */
        }
        /* drop through */
case s_andand:
case s_oror:
case s_equalequal:
case s_notequal:
case s_greater:
case s_greaterequal:
case s_less:
case s_lessequal:
case s_comma:
case s_and:
case s_plus:
case s_minus:
case s_leftshift:
case s_or:
case s_rightshift:
case s_xor:
case s_times:
case s_div:
case s_rem:
        n1 = nastiness(arg1_(x));
        n2 = nastiness(arg2_(x));
/* A good estimate of the complexity (number of temp registers needed)  */
/*             n1==n2 ? n1+1 : max(n1,n2),                              */
/* but modify the middle term so that we never exceed ISHARD, nor       */
/* even attain it unless one term is ISHARD (thus ISHARD terms are only */
/* those containing function like things).  See cg_fnargs for why.      */
        nr = n1==n2 ? (n1>=ISHARD-1 ? n1 : n1+1) : max_(n1,n2);
        if (!(config & CONFIG_HAS_MULTIPLY)) {
          /* See if we will need a proc. call on the ARM and similar.      */
          /* However, this 'better' code seems to compile a slightly 0.01% */
          /* bigger compiler than if we omit the conjuncts which check for */
          /* (const) multiplies which do not require a function call!!!!   */
          /* Leave in the 'notionally correct code'.                       */
          if (op == s_times && h0_(arg1_(x)) != s_integer
                            && h0_(arg2_(x)) != s_integer) return ISHARD;
        }
#ifndef TARGET_HAS_DIVIDE
        if ((op == s_div || op == s_rem) && h0_(arg2_(x)) != s_integer)
            return ISHARD;
#endif
        return max_(ISEXPR, nr);            /* always a little bit hard. */
case s_let:
        x = arg2_(x);
        continue;
default:   /* includes s_fnap */
        return ISHARD;
    }
#undef max_
}

static VRegnum cg_binary_or_fn(J_OPCODE op, TypeExpr *type,
                               Expr *fname, Expr *a1, Expr *a2,
                               bool commutesp)
{
/* commutesp is used to control selection of special cases on integer    */
/* args as well as commutativity.                                        */
/* This code is only ever activated on integerlike things                */
    if (commutesp && nastiness(a1)<nastiness(a2))
    {   Expr *t = a1;
        a1 = a2;
        a2 = t;
    }
    if (commutesp && integer_constant(a2)
#ifndef TARGET_IS_SPARC
  && (config&CONFIG_HAS_MULTIPLY)
#endif
)   /* really just allow MULK */
        return cg_binary(op, a1, a2, commutesp, INTREG);
/* Fortunately I can somewhat fudge the creation of a function call node */
/* since it will only be looked at be this codegenerator and in the      */
/* restricted context of function calls only little bits of type info    */
/* are looked at.                                                        */

/* N.B. in this version I make (p/q) turn into divide(q,p) since that    */
/* seems to make register usage behave better. This is an incompatible   */
/* change from some earlier versions of this compiler - BEWARE           */

/* Detection of special case of divide, remainder by 10 moved to cse.c   */
    a1 = mk_expr2(s_fnap, type, fname,
                                (Expr *)mkExprList(mkExprList(0, a1), a2));
    return cg_expr(a1);
}

static VRegnum cg_binary_1(J_OPCODE op, Expr *a1, Expr *a2,
                           bool commutesp, J_OPCODE *condP,
                           RegSort fpp)
{   /* This routine has been grossly uglified by CMP's having conds */
    J_OPCODE cond = condP==0 ? 0 : *condP;
    if (is_same(a1, a2))
    {   VRegnum r, targetreg;
        r = cg_expr(a1);    /* Do this BEFORE allocating targetreg */
        targetreg = jop_iscmp_(op) ? GAP : fgetregister(fpp);
        emitreg(op+cond, targetreg, r, r);
        bfreeregister(r);
        return(targetreg);
    }
/* This first swap is entirely looking for constant arg cases,           */
/* possible evaluation order swap comes later.                           */
    if (commutesp && nastiness(a1)<nastiness(a2))
    {   Expr *t = a1;
        a1 = a2;
        a2 = t;
        if (condP) *condP = cond = Q_swap(cond);
    }
#ifdef TARGET_HAS_SCALED_OPS
/* The following code has come here as the best place for it.             */
/* One might wonder whether the test for is_shifted or s_integer should   */
/* come first.  Observe that either is correct.  Consider:                */
/*   if ((x>>7) == 27) ...                                                */
/* We can either do    MOVK r,27;        CMPR r, x LSR 7                  */
/* or                  MOVR r, x LSR 7;  CMPK r, 27                       */
/* I suppose that the former is better if loop invariants are optimised.  */
/* The former is what we do here...                                       */
    if (op == J_ADDR || op == J_SUBR ||
        op == J_ANDR || op == J_ORRR || op == J_EORR ||
        (op & ~Q_MASK) == J_CMPR)
    { if (is_shifted(a2, 0))
        return cg_opshift(op+cond, a1, shift_operand(a2), shift_amount(a2));
      if (is_shifted(a1, 0))
      { if (condP) *condP = cond = Q_swap(cond);
        return cg_opshift((op==J_SUBR ? J_RSBR : op) + cond,
                              a2, shift_operand(a1), shift_amount(a1));
      }
    }
#endif /* TARGET_HAS_SCALED_OPS */
  { VRegnum targetreg;
    op = op+cond;
    if (jop_canRTOK(op) && integer_constant(a2))     /* floating case below */
    {   int32 n = result2;
        VRegnum r1 = cg_expr(a1);
/* Compare instructions do not need a real destination.                  */
        targetreg = jop_iscmp_(op) ? GAP : fgetregister(fpp);
#define jop_isregshift_(op) \
   (((op) & ~(J_SIGNED+J_UNSIGNED)) == J_SHLR || \
    ((op) & ~(J_SIGNED+J_UNSIGNED)) == J_SHRR)
        if (jop_isregshift_(op))
        {
            if (n == 0)   /* this case should not currently happen */
            {   bfreeregister(targetreg);  /* i.e. don't use reg */
                return r1;
            }
            if ((unsigned32)n >= (unsigned32)32)
            {
/* The following optimisations of errors are overzealous (warned in sem.c)  */
/* but ensure compatible behaviour of constants and expressions.            */
                VRegnum r2 = fgetregister(INTREG);
#ifdef TARGET_LACKS_RIGHTSHIFT
                if ((op & ~(J_SIGNED+J_UNSIGNED)) == J_SHRR)
                    op ^= (J_SHRR^J_SHLR), n = -n;
#endif /* TARGET_LACKS_RIGHTSHIFT */
                emit(J_MOVK, r2, GAP, n);
                emitreg(op, targetreg, r1, r2);
                bfreeregister(r2);
                op = J_NOOP;
            }
        }
        /* @@@ do similar things for div/rem by 0? */
        if (op != J_NOOP) emit(J_RTOK(op), targetreg, r1, n);
        bfreeregister(r1);
    }
#ifdef TARGET_HAS_FP_LITERALS
    else if (h0_(a2) == s_floatcon && fpliteral((FloatCon *) a2, J_RTOK(op)))
    {   VRegnum r1 = cg_expr(a1);
        targetreg = jop_iscmp_(op) ? GAP : fgetregister(fpp);
        emitfloat(J_RTOK(op), targetreg, r1, (FloatCon *)a2);
        bfreeregister(r1);
    }
#endif /* TARGET_HAS_FP_LITERALS */
    else
    {   VRegnum r1, r2;
        if (nastiness(a1) < nastiness(a2))
            r2 = cg_expr(a2),
            (!isintregtype_(fpp) ? sparefpregs++ : spareregs++),
            r1 = cg_expr(a1),    /* Do this BEFORE allocating targetreg */
            (!isintregtype_(fpp) ? sparefpregs-- : spareregs--);
        else
            r1 = cg_expr(a1),
            (!isintregtype_(fpp) ? sparefpregs++ : spareregs++),
            r2 = cg_expr(a2),    /* Do this BEFORE allocating targetreg */
            (!isintregtype_(fpp) ? sparefpregs-- : spareregs--);
        targetreg = jop_iscmp_(op) ? GAP : fgetregister(fpp);
        emitreg(op, targetreg, r1, r2);
        bfreeregister(r1);
        bfreeregister(r2);
    }
    return(targetreg);
  }
}

static VRegnum cg_binary(J_OPCODE op, Expr *a1, Expr *a2,
                         bool commutesp, RegSort fpp)
{
    return cg_binary_1(op, a1, a2, commutesp, 0, fpp);
}

static void cg_condjump(J_OPCODE op, Expr *a1, Expr *a2, RegSort rsort,
                        J_OPCODE cond, LabelNumber *dest)
{   bfreeregister(cg_binary_1(op, a1, a2, 1, &cond, rsort));
    emitbranch(J_B+cond, dest);
}

static void cg_count(FileLine fl)
{
#ifdef TARGET_HAS_PROFILE
    if (full_profile_option && fl.f != 0)
        emitfl(J_COUNT, fl);
#else
    IGNORE(fl);
#endif
}

static void cg_test(Expr *x, bool branchtrue, LabelNumber *dest)
{
/* I wonder if the count-point that was here was really useful? ...
    cg_count(cg_current_cmd->fileline);
 ... end of commented out code */
    cg_test1(x, branchtrue, dest);
}

static bool at_least_once(Expr *init, Expr *endtest)
{
/* This should return true if the boolean formula endtest is certain to  */
/* return false if obeyed straight after the execution of init. This is  */
/* used to map 'for' loops onto 'do .. while' ones where the test is a   */
/* little cheaper.                                                       */
/* The cases I recognize at present are where the initializer is an      */
/* expression (possibly with commas) ending with a form 'var=int' for    */
/* a simple variable, and the end test is an expression 'var op int' or  */
/* 'int op var' for the same variable and a relational operator. Then    */
/* by comparing the two integers I can see if the endtest will fail on   */
/* entry to the loop & if so move it to the end. It might be worth       */
/* extending this code to identify the forms 'var', 'var++', 'var--',    */
/* '++var' and '--var' in the end test position, depending somewhat on   */
/* the perception we have of what sorts of loop are commonly written.    */
    int32 i1, i2;
    int32 unsignedp;
    Expr *var, *x1, *x2;
    AEop tst;
    if (endtest==0) return(1);  /* no endtest => this does not matter    */
    if (init==0) return(0);     /* no init for => I can't tell           */
    while (h0_(init) == s_comma) init = arg2_(init);
    while (h0_(init) == s_cast)  init = arg1_(init);
    if (!(h0_(init)==s_assign)) return(0);
    var = arg1_(init);
    if (!(h0_(var)==s_binder)) return(0);
    i1 = mcrepofexpr(var);
    if (i1!=4 && i1!=0x01000004) return(0); /* int or unsigned int OK    */
    init = arg2_(init);
    if (!integer_constant(init)) return(0);
    i1 = result2;
/* Now I know that the initializer is 'var = i1'                         */
    if (!isrelational_(tst = h0_(endtest))) return(0);
    x1 = arg1_(endtest);
    x2 = arg2_(endtest);
    {   Expr *v = 0;
        if (integer_constant(x2) &&
            (x1 == var || (h0_(x1) == s_cast && arg1_(x1) == var)))
            v = x1, i2 = result2;
        else if (integer_constant(x1) &&
                 (x2 == var || (h0_(x2) == s_cast && arg1_(x2) == var)))
            v = x2, i2 = i1, i1=result2;
        else
            return 0;

        if (h0_(v) == s_cast) {
            /* ok if both types are effectively the same, or if they are
               the same width and one is signed and the other unsigned
             */
            int32 repdiff = mcrepofexpr(var) ^ mcrepofexpr(v);
            if ((repdiff & MCR_SIZE_MASK) != 0 ||
                ((repdiff >> MCR_SORT_SHIFT) & ~1) != 0)
                return 0;
        }
        unsignedp = unsigned_expression_(v);
    }
/* NB the tests here are done in signed or unsigned mode                 */
/* depending on the character of the variable v.                         */
    switch (tst)
    {
default:
        return(0);
case s_equalequal:
        return(i1==i2);
case s_notequal:
        return(i1!=i2);
case s_greater:
        if (unsignedp) return ((unsigned32)i1>(unsigned32)i2);
        else return(i1>i2);
case s_greaterequal:
        if (unsignedp) return ((unsigned32)i1>=(unsigned32)i2);
        return(i1>=i2);
case s_less:
        if (unsignedp) return ((unsigned32)i1<(unsigned32)i2);
        return(i1<i2);
case s_lessequal:
        if (unsignedp) return ((unsigned32)i1<=(unsigned32)i2);
        return(i1<=i2);
    }
}

static int32 alignofptr(Expr *x)
{
    if (h0_(x) == s_string)
        return alignof_toplevel;        /* @@@ true for FORTRAN? */
    else if (h0_(x) == s_addrof) {
        x = arg1_(x);
        if (h0_(x) == s_binder)
            return alignof_toplevel;
    }
    return 1;
}

static int32 sizeofpointee(Expr *x, int32 *padding)
{
/* @@@ Beware: sizeofpointee() is now a constraint on how strings       */
/* are stored (and even shared!) in memory.  Hence more of the string   */
/* stuff which some back-ends (including ARM) manage needs to move to   */
/* MIP so we can see that it is consistent.                             */
    int32 len = 0, pad = 0;
    if (h0_(x) == s_string) {
        len = stringlength(((String *)x)->strseg)+1;
        pad = padsize(len, alignof_toplevel) - len;
    } else if (h0_(x) == s_addrof) {
        x = arg1_(x);
        if (h0_(x) == s_binder) {
            len = sizeoftype(bindtype_((Binder *)x));
            pad = padsize(len, alignof_toplevel) - len;
        }
    }
    *padding = pad;
    return len;
}

static VRegnum open_compilable(Expr **xp, RegSort rsort)
{
    Expr *x = *xp;
    Expr *fname = arg1_(x);
    Expr *a1 = NULL, *a2 = NULL, *a3 = NULL;
    int32 narg = 0, n;
    ExprList *a = exprfnargs_(x);

    if (h0_(fname) != s_addrof) return NOT_OPEN_COMPILABLE;
    fname = arg1_(fname);
    if (fname == NULL || h0_(fname) != s_binder) return NOT_OPEN_COMPILABLE;
/* Only consider this call if the function is a direct function name */
    if (a != NULL)
    {   a1 = exprcar_(a);
        narg++;
        a = cdr_(a);
        if (a != NULL)
        {   a2 = exprcar_(a);
            narg++;
            a = cdr_(a);
            if (a != NULL)
            {   a3 = exprcar_(a);
                narg++;
                a = cdr_(a);
            }
        }
    }
#ifdef TARGET_INLINES_MONADS    /* distinctly special case-ish          */
    if (narg == 1) {
        int32 i = target_inlinable((Binder *)fname, narg);
        if (i) {
            VRegnum r1 = cg_expr(a1);
            VRegnum targetreg = fgetregister(rsort);
            emit(floatyop(rsort, J_INLINE1, J_INLINE1F, J_INLINE1D),
                 targetreg, r1, i);
            bfreeregister(r1);
            return targetreg;
        }
    }
#endif

#ifdef TARGET_HAS_DIVREM_FUNCTION
/* The idea here is that if we are doing both divide and remainder,
   CSE can eliminate one call.  To do this, it and regalloc need to
   understand that div & udiv return two results.
 */
/* Should we invent a V_resultreg2 macro for R_A1+1?                    */
    if (fname == arg1_(sim.remfn) && narg == 2)
        return cg_fnap(mk_expr2(s_fnap, te_int, sim.divfn, (Expr *)exprfnargs_(x)),
                       virtreg(R_A1+1,INTREG), YES);
    else if (fname == arg1_(sim.uremfn) && narg == 2)
        return cg_fnap(mk_expr2(s_fnap, te_uint, sim.udivfn, (Expr *)exprfnargs_(x)),
                       virtreg(R_A1+1,INTREG), YES);
#endif

    if (bindsym_((Binder *)fname) == sim.strcpysym) {
        if (narg == 2 && h0_(a2) == s_string) {
          *xp = mk_expr2(s_fnap, type_(x), sim.realmemcpyfn,
                         (Expr *)mkExprList(
                                   mkExprList(
                                     mkExprList(NULL, mkintconst(te_int, stringlength(((String *)a2)->strseg)+1, 0)),
                                     a2),
                                   a1));
          return open_compilable(xp, rsort);
        } else
          return NOT_OPEN_COMPILABLE;
    }

#define structaligned(e) ((alignofptr(e) & (alignof_struct-1)) == 0)
#ifdef TARGET_HAS_BLOCKMOVE
#  define cg_inlineable_size(n) ((n & alignof_struct-1) == 0)
#else
/* Currently the non-J_MOVC expansion below only works for mults of 4,   */
/* and then only if guaranteed 4-byte aligned.                           */
#  define cg_inlineable_size(n) (alignof_struct >= 4 && (n & 3) == 0)
#endif

    if (bindsym_((Binder *)fname) == bindsym_((Binder *)arg1_(sim.realmemcpyfn)) &&
        h0_(a3) == s_integer &&
        structaligned(a1) && structaligned(a2)) {
        Expr *x1;
        ExprList *args = exprfnargs_(x);
        n = intval_(a3);
        if (!cg_inlineable_size(n)) {
            int32 padding;
            int32 destsize = sizeofpointee(a1, &padding);
            if (n != destsize || padding == 0 || !cg_inlineable_size(destsize+padding))
                return NOT_OPEN_COMPILABLE;
            args = mkExprList(
                     mkExprList(
                       mkExprList(NULL, mkintconst(te_int, destsize+padding, 0)),
                       a2),
                     a1);
        } else if (sim.realmemcpyfn == sim.memcpyfn)
            args = NULL;  /* bodge to avoid endless recursion */

        if (args != NULL) {
            x1 = mk_expr2(s_fnap, type_(x), sim.memcpyfn, (Expr *)args);
            return open_compilable(&x1, rsort);
        }
    }
/* At present the main thing that we do open compilation for is          */
/* _memcpy() and _memset() and then only if it has just 3 args, the      */
/* last of which is an integer with value >= 0 & a multiple of           */
/* alignof_struct                                                        */
    if (bindsym_((Binder *)fname) == bindsym_((Binder *)arg1_(sim.memcpyfn)) &&
        a == NULL && narg == 3 && h0_(a3) == s_integer &&
        (n = intval_(a3)) >= 0 && cg_inlineable_size(n))
    {   VRegnum r1, r2;
/*
 * Here we break pay the penalty for having pretended that the ARM has
 * a block move instruction - we have to deny it at this stage.
 */
#if defined (TARGET_HAS_BLOCKMOVE) && !defined(TARGET_IS_ARM)
        spareregs += 2;
        r1 = cg_expr(a1);
        r2 = cg_expr(a2);
        spareregs -= 2;
#  ifdef TARGET_IS_ACW
        if (n<=16) emit(J_MOVC, r1, r2, n);   /* must NOT corrupt r1 */
        else
        {  VRegnum r3, r4;
           r3 = fgetregister(INTREG);
           r4 = fgetregister(INTREG);
           emitreg(J_MOVR, r3, GAP, r1);
           emitreg(J_MOVR, r4, GAP, r2);
           emit(J_MOVC, r3, r4, n);
           bfreeregister(r3);
           bfreeregister(r4);
        }
#  else  /* TARGET_IS_ACW */
        emit(J_MOVC, r1, r2, n);   /* must NOT corrupt r1 */
#  endif /* TARGET_IS_ACW */
#else  /* TARGET_HAS_BLOCKMOVE */
        { VRegnum r3;
#ifdef TARGET_IS_ARM
/* ARM experiment with MOVC underway.  Copy of a single word is turned into
   LOAD+STORE, as easier for CSE to optimise away if appropriate.  Actually,
   I suppose that this is a good idea for all targets. Longer moves
   get turned into a MOVC (which destroys its argument registers if the move
   is longer than the number of registers I'm prepared to guarantee available).
 */
          if (n >= 8) {
            spareregs += 2;
            r1 = cg_expr(a1);
            r2 = cg_expr(a2);
            spareregs -= 2;
            if (n <= 12)
            /* If there are enough spare registers, the expansion of
               MOVC needn't alter r1 and r2.  (2 dedicated registers,
               plus the one holding the source address).
             */
              emit(J_MOVC, r1, r2, n);   /* must NOT corrupt r1 */
            else {
              r3 = fgetregister(INTREG);
              emitreg(J_MOVR, r3, GAP, r1);
              emit(J_MOVC, r3, r2, n);
              bfreeregister(r3);
            }
          }
#else /* TARGET_IS_ARM */
/* For very short moves I just do LOAD/STORE sequences. For longer ones  */
/* I synthesize a loop.                                                  */
          if (n >= 5*MEMCPYQUANTUM)
          {
            LabelNumber *ll = nextlabel();
            VRegnum r4;
            spareregs += 2;
            r1 = cg_expr(a1);
            r2 = cg_expr(a2);
            spareregs -= 2;
            r3 = fgetregister(MEMCPYREG);
            r4 = fgetregister(INTREG);
#ifndef TARGET_LACKS_RR_STORE
#  ifdef TARGET_IS_88000
/*
 * The following code is designed to exploit a Motorola 88000 by
 * giving scope for at least some overlap of memory transactions. On some
 * other machines it may be a DISASTER because it needs two more scratch
 * registers.  Hence at present it is guarded by the machine type.  It
 * is by now pretty clear that most targets should support blockmoves
 * at the level of gen.c.
 */
            if ((n & 4) == 0)           /* move an even number of words */
            {   VRegnum r5 = fgetregister(INTREG),
                        r6 = fgetregister(INTREG);
                emit(J_MOVK, r4, GAP, n-4);
                start_new_basic_block(ll);
                emitreg(J_LDRR, r3, r2, r4);
                emit(J_SUBK, r5, r4, 4);
                emitreg(J_LDRR, r6, r2, r5);
                emitreg(J_STRR, r3, r1, r4);
                emit(J_SUBK, r4, r5, 4);
                emitreg(J_STRR, r6, r1, r5);
                emit(J_CMPK + Q_GE, GAP, r4, 0);
                emitbranch(J_B + Q_GE, ll);
                bfreeregister(r6);
                bfreeregister(r5);
                bfreeregister(r4);
                bfreeregister(r3);
            }
            else            /* odd number of words to move */
#  endif
#endif
            {
                emit(J_MOVK, r4, GAP, n-MEMCPYQUANTUM);
#  ifdef TARGET_LACKS_RR_STORE
                    emit(J_ADDK, r1, r1, n);
#  endif
                start_new_basic_block(ll);
                emitreg(J_memcpy(J_LDRR), r3, r2, r4);
#  ifdef TARGET_LACKS_RR_STORE
                emit(J_SUBK, r1, r1, MEMCPYQUANTUM);
                emit(J_memcpy(J_STRK), r3, r1, 0);
#  else
                emitreg(J_memcpy(J_STRR), r3, r1, r4);
#  endif
                emit(J_SUBK, r4, r4, MEMCPYQUANTUM);
                emit(J_CMPK + Q_GE, GAP, r4, 0);
                emitbranch(J_B + Q_GE, ll);
                bfreeregister(r4);
                bfreeregister(r3);
            }
          }
#endif
          else
          { Binder *loadv = is_local_adcon(a2),
                   *storev = is_local_adcon(a1);
            spareregs += 2;
            r1 = (!storev) ? cg_expr(a1) : GAP;
            r2 = (!loadv) ? cg_expr(a2) : GAP;
            spareregs -= 2;
            r3 = fgetregister(MEMCPYREG);
            while ((n -= MEMCPYQUANTUM) >= 0)
            {   if (loadv) emitvk(J_memcpy(J_LDRVK), r3, n, loadv);
                else emit(J_memcpy(J_LDRK), r3, r2, n);
                if (storev) emitvk(J_memcpy(J_STRVK), r3, n, storev);
                else emit(J_memcpy(J_STRK), r3, r1, n);
            }
            bfreeregister(r3);
          }
        }
#endif  /* TARGET_HAS_BLOCKMOVE */
        bfreeregister(r2);
        return r1;
    }
/*
 * The following code does open compilation of "_memset" in the case
 * were there are 3 args and the second and third args are
 * zero and divisible by alignof_struct exactly.
 */
    if (bindsym_((Binder *)fname) == bindsym_((Binder *)arg1_(sim.memsetfn)) &&
        a == NULL && narg == 3 &&
        h0_(a2) == s_integer && intval_(a2) == 0 &&
        h0_(a3) == s_integer && (n = intval_(a3)) >= 0 && cg_inlineable_size(n))
    {   VRegnum r1;
#if defined (TARGET_HAS_BLOCKMOVE) && !defined(TARGET_IS_ARM)
        spareregs += 1;                 /* duh?                         */
        r1 = cg_expr(a1);
        spareregs -= 1;
        emit(J_CLRC, r1, GAP, n);       /* must NOT corrupt r1?         */
#else  /* TARGET_HAS_BLOCKMOVE */
/* For very short memset's I just do STORE sequences. For longer ones  */
/* I synthesize a loop.                                                */
        VRegnum r2;
#ifdef TARGET_IS_ARM
        if (n >= 8) {
          r1 = cg_expr(a1);
          if (n == 8)
            emit(J_CLRC, r1, GAP, n);   /* must NOT corrupt r1 */
          else {
            r2 = fgetregister(INTREG);
            emitreg(J_MOVR, r2, GAP, r1);
            emit(J_CLRC, r2, GAP, n);
            bfreeregister(r2);
          }
        }
#else /* TARGET_IS_ARM */
        if (n >= 10*MEMCPYQUANTUM)
        {
            LabelNumber *ll = nextlabel();
            VRegnum r3;
            spareregs += 2;
            r1 = cg_expr(a1);
            r2 = MEMCPYREG==DBLREG ? cg_loadfpzero(DBLREG,0) : cg_expr(a2);
            spareregs -= 2;
            r3 = fgetregister(INTREG);
            emit(J_MOVK, r3, GAP, n-MEMCPYQUANTUM);
#ifdef TARGET_LACKS_RR_STORE
                emit(J_ADDK, r1, r1, n);
#endif
            start_new_basic_block(ll);
#ifdef TARGET_LACKS_RR_STORE
                emit(J_SUBK, r1, r1, MEMCPYQUANTUM);
                emit(J_memcpy(J_STRK), r2, r1, 0);
#else
                emitreg(J_memcpy(J_STRR), r2, r1, r3);
#endif
            emit(J_SUBK, r3, r3, MEMCPYQUANTUM);
            emit(J_CMPK + Q_GE, GAP, r3, 0);
            emitbranch(J_B + Q_GE, ll);
            bfreeregister(r3);
            bfreeregister(r2);
        }
#endif
        else
        {   Binder *storev = is_local_adcon(a1);
            spareregs += 2;
            r1 = (!storev) ? cg_expr(a1) : GAP;
            r2 = MEMCPYREG==DBLREG ? cg_loadfpzero(DBLREG,0) : cg_expr(a2);
            spareregs -= 2;
            while ((n -= MEMCPYQUANTUM) >= 0)
            {
                if (storev) emitvk(J_memcpy(J_STRVK), r2, n, storev);
                else emit(J_memcpy(J_STRK), r2, r1, n);
            }
            bfreeregister(r2);
        }
#endif  /* TARGET_HAS_BLOCKMOVE */
        return r1;
    }
    if (bindsym_((Binder *)fname) ==
        bindsym_((Binder *)arg1_(sim.inserted_word)) &&
        a == NULL && narg == 1 && h0_(a1) == s_integer)
    {
/* _word(nnn) will plant nnn in the code - for EXPERT/lunatic use only! */
        emit(J_WORD, GAP, GAP, intval_(a1));
        return virtreg(R_A1, INTREG); /* Resultregister wanted here? */
    }
    return NOT_OPEN_COMPILABLE;
}

/* jopcode generation for binding vars and args ...                         */

Binder *gentempvar(TypeExpr *t, VRegnum r)
{   /* like gentempbinder, but pretends set_local_vregister called */
    /* bindaddr_() is not set at this point, but will get filled   */
    /* in later (by cg_bindlist or explicit code) if it might be   */
    /* needed. There will be a syserr if an attempt is made to     */
    /* reference a spilt temporary that has not been completed     */
    /* properly.                                                   */
    Binder *bb = gentempbinder(t);
    bindxx_(bb) = r;
    bb->bindmcrep = mcrepofexpr((Expr *)bb);
    return bb;
}

static void set_local_vregister(Binder *b, int32 rep, bool isarg)
{   VRegnum r = GAP;
    int32 mode = rep >> MCR_SORT_SHIFT, len = rep & MCR_SIZE_MASK;
    if (isvolatile_expr((Expr *)b))
    {   /* treat volatile locals as 'address taken' for setjmp.             */
        bindstg_(b) = (bindstg_(b) & ~bitofstg_(s_register)) | b_addrof;
    }
    if (!(bindstg_(b) & b_addrof))
    {   /* try to put in a register */
        if (mode == 0 || mode == 1)   /* ADENART, was "&& len<=4".    */
        {
#ifdef ADDRESS_REG_STUFF
           TypeExpr *t = princtype(bindtype_(b));
           r = vregister(h0_(t) == t_content || h0_(t) == t_subscript ?
                              ADDRREG : INTREG);
#else
           r = vregister(INTREG);
#endif
/* J_INIT never generates code: it can be thought of as meaning          */
/* "load this register with an undefined value" and it helps register    */
/* allocation in the case that the user seems not to initialise the      */
/* variable properly. E.g. it controls the trouble I get in code like:   */
/*    { int x; <thing not using x>; if (tautology) x = 1; <use x>; }     */
            if (!isarg) emitbinder(J_INIT, r, b);
        }
        else if (mode == 2 && len==4)
        {   r = vregister(FLTREG);
            if (!isarg) emitbinder(J_INITF, r, b);
        }
        else if (mode == 2 && len==8)
        {   r = vregister(DBLREG);
            if (!isarg) emitbinder(J_INITD, r, b);
        }
    }
    bindxx_(b) = r;
    b->bindmcrep = mcrepofexpr((Expr *)b);
    if ((bindstg_(b) & bitofstg_(s_register)) &&
/* @@@ LDS 23Aug89 - Temporary: to be rationalised soon. Sorry */
        !(var_cc_private_flags & 1L))
    {   regvar_binders = mkBindList(regvar_binders, b);
        /* discouragement of spilling now in regalloc */
    }
    else local_binders = mkBindList(local_binders, b);
}

static void init_slave_reg(Binder *b, int32 nfltregargs, int32 nintregargs)
/* b is the binder for a potentially registered argument variable. Load  */
/* its initial value from the place where the relevant argument was      */
/* passed - register a1 to a4 for simple cases and the stack otherwise.  */
/* Oct 92: this code is in need of re-parameterisation.                  */
{
    /* bindaddr_(b) is known to be BINDADDR_ARG.                        */
    int32 addr = bindaddr_(b) & ~BINDADDR_MASK;
    int32 n = addr / alignof_toplevel;
    VRegnum v = bindxx_(b);
    if (v != GAP)
    {   int32 ni = n - (8/alignof_toplevel)*nfltregargs;
#ifdef TARGET_IS_MIPS
        int32 ir = n;   /* flt regs cause 'missing' int reg args.       */
#else
        int32 ir = ni;
#endif
        RegSort rsort = vregsort(v);
#ifdef TARGET_STRUCT_RESULT_REGISTER
        if (b == result_variable)
            emitreg(J_MOVR, v, GAP,
                    virtreg(TARGET_STRUCT_RESULT_REGISTER, INTREG));
        else
#endif
        if (isintregtype_(rsort))
        {
/* @@@ The following lines need tidying (remember adenart stops         */
/* collecting args when it sees a struct (parameterise!)).              */
/* @@@ Are the other NARGREGS's below OK too?                           */
#ifdef TARGET_IS_MIPS
            if (ir < NARGREGS)
#else
            if (0 <= ni && ni < nintregargs)
#endif
              emitreg(J_MOVR, v, GAP, virtreg(R_P1+ir, INTREG));
            else emitbinder(J_LDRV1, v, b);
        }
        else if (rsort == FLTREG)
/* This case does not currently happen in standard Norcroft compilers,  */
/* since 'float' args are passed as double and callee-narrowed.         */
/* J_MOVIFR has to copy a bit-pattern from an integer register into one */
/* of the FP registers.                                                 */
        {   if (ni < 0)
              emitreg(J_MOVDFR, v, GAP,
                      virtreg(R_FP1+addr/sizeof_double, DBLREG));
            else if (ir < NARGREGS)
              emitreg(J_MOVIFR, v, GAP, virtreg(R_P1+ir, INTREG));
            else
                emitbinder(J_LDRFV1, v, b);
        }
        else
/* If I have a double-precision arg the value is passed in two integers  */
/* and here I must be prepared to move it into a doubleprecision FP      */
/* register.                                                             */
        {   if (ni < 0)
/* The following line assumes FPREGARGS are arranged to appear first...  */
              emitreg(J_MOVDR, v, GAP,
                      virtreg(R_FP1+addr/sizeof_double, DBLREG));
            else
#ifdef TARGET_IS_ADENART
              emitbinder(J_LDRDV1, v, b);
#else
            if (ir < NARGREGS-1)
                emitreg(J_MOVIDR, v, virtreg(R_P1+ir,INTREG),
                                     virtreg(R_P1+ir+1,INTREG));
            else
/* If a floating point arg straddles the boundary between where args are */
/* passed in registers & where they come on the stack I know that there  */
/* are at least 5 words of args, so if I suppress the leaf-procedure     */
/* optimisation I know that the args will get written to the stack as    */
/* contiguous words. Then I can load the FP value easily.                */
            {   if (ir == NARGREGS-1) procflags |= PROC_ARGPUSH;
                emitbinder(J_LDRDV1, v, b);
            }
#endif
        }
    }
}

static void cg_bindlist(SynBindList *x, bool initflag)
{   /* initflag is 1 if J_INIT is not needed due to later explicit init. */
    BindList *new_binders = active_binders;
    for (; x!=NULL; x = x->bindlistcdr)
    {   Binder *b = x->bindlistcar;
        if (bindstg_(b) & bitofstg_(s_auto))        /* ignore statics    */
        /* N.B. register vars must also have auto bit set                */
        {   int32 rep = mcrepofexpr((Expr *)b);
            set_local_vregister(b, rep, initflag);
            new_binders = mkBindList(new_binders, b);
            b->bindaddr.bl = new_binders;
            bindstg_(b) |= b_bindaddrlist;
/* the next 3 line calculation should be done after regalloc, not here */
            current_stackdepth = padtomcrep(current_stackdepth, rep) +
                                 padsize(rep & MCR_SIZE_MASK, alignof_toplevel);
            if (current_stackdepth > greatest_stackdepth)
                greatest_stackdepth = current_stackdepth;
        }
    }
    emitsetsp(J_SETSPENV, new_binders);
}

static int32 cg_bindargs_size(BindList *args)
{   int32 argoff = 0;
    for (; args!=NULL; args=args->bindlistcdr)
    {   Binder *b = args->bindlistcar;
        int32 rep = mcrepofexpr((Expr *)b);
/* Things in an argument list can only have s_auto storage class     */
        if (!(bindstg_(b) & bitofstg_(s_auto))) syserr(syserr_nonauto_arg);
#ifdef TARGET_IS_SPARC
        argoff = padtomcrep(argoff, rep & ~MCR_ALIGN_DOUBLE);
#else
        argoff = padtomcrep(argoff, rep);
#endif
        set_local_vregister(b, rep, 1);
        bindaddr_(b) = argoff | BINDADDR_ARG;
        argoff += padsize(rep & MCR_SIZE_MASK, alignof_toplevel);
    }
    return argoff;
}



#ifdef TARGET_IS_ADENART
/* On this target we allow only 'int' values into int regs.             */
static int32 cg_nintregargs(BindList *args, bool ellipsis)
{   int32 n=0;
    BindList *x;
    for (x = args; x != NULL; x = x->bindlistcdr)
    {   Binder *b = x->bindlistcar;
        unsigned32 repsort = mcrepofexpr((Expr *)b) >> MCR_SORT_SHIFT;
        if (repsort >= 2) return n;
        n++;
    }
    return ellipsis ? NARGREGS : n;
}

/* this should arguably be in syn...                                    */
static Cmd *adenart_call(Cmd *c, SynBindList *formals)
{   Expr *arginit = 0;
    SynBindList *f;
    int32 n = FIRST_ADETRAN_ARG;
    for (f = formals; f != NULL; f = f->bindlistcdr)
    {   Binder *b = f->bindlistcar;
        TypeExpr *bt = bindtype_(b);
/* A call from adetran has args in 0x101,0x102... with pointers         */
/* encoded as WORD pointers.  Convert to C byte pointers.               */
        Expr *e = mkunary(s_content, mkintconst(ptrtotype_(bt), 8*n++, 0));
        e = mkassign(s_assign, (Expr *)b,
              h0_(princtype(bt)) == t_content ?
                mk_expr2(s_leftshift, bt, e, mkintconst(te_int,3,0)) : e);
        arginit = arginit ? mkbinary(s_comma, arginit, e) : e;
    }
    arginit = optimise0(arginit);
    return arginit==0 ? c : mk_cmd_block(c->fileline, formals,
        mkCmdList(mkCmdList(0, c),
                  mk_cmd_e(s_semicolon, c->fileline, arginit)));
}
#endif

/* One might worry here about functions that specify their formals with  */
/* a type such as char, short or float.  For language independence sem.c */
/* arranges that such things never occur by generating an explicit       */
/* narrowing assignment in this case.                                    */

int32 nresultregs;

static int32 cg_bindargs(BindList *args, bool ellipsis)
/* Ellipsis is true if this function was defined with a '...' at the end */
/* of the list of formal pars it should accept.                          */
/* Returns number of argwords for possible use by codeseg_function_name(). */
/* Note that on some machines (e.g. the 88000) very strange things have to */
/* be done to support va_args (i.e. if '...' is indicated).  Also in some  */
/* cases integer, floating and structure args may be passed in different   */
/* ways.                                                                   */
{
    BindList *x;
    int32 argoff, lyingargwords;
    int32 nfltregargs = 0, nintregargs = 0;

/* If ANY argument has its address taken I mark ALL arguments as having  */
/* their address taken, unless the user has declared them as registers.  */
    for (x = args; x!=NULL; x=x->bindlistcdr)
    {   Binder *b = x->bindlistcar;
        if (bindstg_(b) & b_addrof)
/* Furthermore since in that case I will always need to write arguments  */
/* in place on that stack it makes sense to use a full entry sequence.   */
/* This of course must, in general, kill tail recursion optimisation.    */
        {   procflags |= PROC_ARGADDR;
            for (x = args; x!=NULL; x=x->bindlistcdr)
            {   b = x->bindlistcar;
                if (!(bindstg_(b) & bitofstg_(s_register)))
                    bindstg_(b) |= b_addrof;
            }
            break;
        }
    }
#ifdef TARGET_FP_ARGS_IN_FP_REGS
    if ((config & CONFIG_FPREGARGS)) {
        BindList *intargs = NULL, **intargp = &intargs,
                 *fltregargs = NULL, **fltregargp = &fltregargs;
        for (x = args; x != NULL; x = x->bindlistcdr) {
            Binder *b = x->bindlistcar;
            BindList *newbl = mkBindList(NULL, b);
            int32 repsort = mcrepofexpr((Expr *)b) >> MCR_SORT_SHIFT;

            if (repsort == 2 && nfltregargs < NFLTARGREGS
#ifdef TARGET_IS_MIPS           /* only LEADING flt args in flt regs.   */
                             && intargs == NULL
#endif
               )
            {
	      nfltregargs++,
                *fltregargp = newbl, fltregargp = &newbl->bindlistcdr;
            } else
	      {
                *intargp = newbl, intargp = &newbl->bindlistcdr;
	      }
	
        }
        if (intargs != NULL) *fltregargp = intargs;
        args = fltregargs;
        argoff = cg_bindargs_size(args);
#ifdef TARGET_IS_ADENART
        nintregargs = cg_nintregargs(intargs, ellipsis);
#else
        nintregargs = intargs == NULL ? 0 :
            (argoff - (bindaddr_(intargs->bindlistcar) & ~BINDADDR_MASK))/4;
#endif
    } else
#endif /* TARGET_FP_ARGS_IN_FP_REGS */
    {   argoff = cg_bindargs_size(args),
#ifdef TARGET_IS_ADENART
        nintregargs = cg_nintregargs(args, ellipsis);
#else
        nintregargs = argoff/4;
#endif
    }

    if (nintregargs > NARGREGS) nintregargs = NARGREGS;
    max_argsize = argoff;
#ifdef TARGET_STRUCT_RESULT_REGISTER
    if (result_variable != NULL)
    {   int32 rep = mcrepofexpr((Expr *)result_variable);
        set_local_vregister(result_variable, rep, 1);
/*
 * The bindaddr field would be inspected in init_slave_reg, but that
 * takes special action on result_variable so I can put junk in here.
 */
        bindaddr_(result_variable) = BINDADDR_ARG;
    }
#endif
    /* Note that with '...' a cautious entry sequence is needed:          */
    /* it suffices to tell the back-end that we need many arg words.      */
    /* The value 0x7fff is chosen for backwards compatibility and easy    */
    /* visibility in masks.  It is subject to change and backends should  */
    /* check for a value outwith 0..255.                                  */
    lyingargwords = ellipsis ? K_ARGWORDMASK : argoff/alignof_toplevel;
    /* The operand of J_ENTER is used to determine addressing of args     */
    /* on machines such as the ARM.                                       */
    emit(J_ENTER, GAP, GAP, k_argdesc_(nintregargs, nfltregargs, nresultregs, lyingargwords, 0));
#ifdef TARGET_STRUCT_RESULT_REGISTER
    if (result_variable != NULL) init_slave_reg(result_variable, 0, 0);
#endif
    for (x=args; x!=NULL; x=x->bindlistcdr)
        init_slave_reg(x->bindlistcar, nfltregargs, nintregargs);
    active_binders = NULL;
    current_stackdepth = greatest_stackdepth = 0;
    start_new_basic_block(nextlabel());
    return lyingargwords;
}

static void loadresultsfrombinder(Binder *x) {
    int32 n;
    for (n = 0; n < nresultregs; n++)
        emitvk(J_LDRVK, V_Presultreg(INTREG)+n, n*sizeof_long, x);
}

static void cg_return(Expr *x, bool implicitinvaluefn)
{
    LabelNumber *retlab = (implicitinvaluefn ? RetImplLab : RetVoidLab);
    if (x!=0)
    {   int32 mcrep = mcrepofexpr(x);
/* Structure results are massaged here to give an assignment via a       */
/* special variable.                                                     */
        if ((mcrep >> MCR_SORT_SHIFT) == 3)
        {   TypeExpr *t = typeofexpr(x);
            if (nresultregs > 0) {
                if (h0_(x) == s_fnap && returnsstructinregs(arg1_(x)))
                    cg_expr(mk_expr2(s_fnapstructvoid, te_void, arg1_(x),
                        (Expr *)(mkExprList(exprfnargs_(x), NULL))));
                else if (h0_(x) == s_binder)
                    loadresultsfrombinder((Binder *)x);
                else {
                    BindList *sl = active_binders;
                    int32 d = current_stackdepth;
                    Binder *temp;
                    if (h0_(x) != s_fnap) {
                        t = ptrtotype_(t);
                        x = optimise0(mk_expr1(s_addrof, t, x));
                    }
                    temp = gentempbinder(t);
                    cg_bindlist(mkSynBindList(0, temp), 0);
                    x = mk_expr2(s_assign, t, (Expr *)temp, x);
                    if (h0_(t) != t_content) {
                        cg_exprvoid(x);
                        loadresultsfrombinder(temp);
                    } else {
                        VRegnum r = cg_expr(x);
                        int32 n;
                        for (n = 0; n < nresultregs; n++)
                            emit(J_LDRK, V_Presultreg(INTREG)+n, r, n*sizeof_long);
                        bfreeregister(r);
                    }
                    emitsetsp(J_SETSPENV, sl);
                    current_stackdepth = d;
                }
                retlab = RetIntLab;
            } else {
                if (result_variable==NULL) syserr(syserr_struct_result);
/* Return the result of a struct-returning fn. The result expn is a var, */
/* a fn call, or (let v in expn) if expn involves a struct-returning fn. */
                if (h0_(x) == s_fnap)
/* return f(...), so use the reult variable directly to get whizzy code. */
                    cg_exprvoid(mk_expr2(s_fnapstruct, te_void, arg1_(x),
                        (Expr *)(mkExprList(exprfnargs_(x),
                                   (Expr *)result_variable))));
                else if (h0_(x) == s_let)
/* Here we have a return (expn involving a struct-fn call). It's a pity */
/* that multiple such things can't be commoned up to share a single     */
/* result copy operation. The problem is the scope of the binder in the */
/* let, absence of return expressions, and failure to distribute the    */
/* missing return expressions through the tree appropriately (Sigh).    */
                {   arg2_(x) = mk_expr2(s_assign, t,
                        mk_expr1(s_content, t, (Expr *)result_variable),
                        arg2_(x));
                    cg_exprvoid(x);
                } else
/* There is some hope of commoning up the result copies, for example in */
/* return i ? x : y, where x and y are binders. Do this by using a ptr  */
/* to the result value until a common tail of code in which *res = *val */
                {   TypeExpr *pt = ptrtotype_(t);
                    cg_exprvoid(
                        mk_expr2(s_assign, pt, (Expr *)result_temporary,
/* @@@ LDS 22-Sep-89: use of optimise0() here is iffy, and anticipates  */
/* the evolution of simplify into a properly specified tree transformer */
                            optimise0(mk_expr1(s_addrof, pt, x))));
                    if (structretlab == NOTALABEL)
                    {   structretlab = nextlabel();
                        start_new_basic_block(structretlab);
                        cg_exprvoid(mk_expr2(s_assign, t,
                            mk_expr1(s_content, t, (Expr *)result_variable),
                            mk_expr1(s_content, t, (Expr *)result_temporary)));
                    }
                    else
                    {   emitsetspgoto(active_binders, structretlab);
                        emitbranch(J_B, structretlab);
                        return;
                    }
                }
            }
        }
        else
        {   int32 mcmode = mcrep >> MCR_SORT_SHIFT;
            RegSort rsort =
                 mcmode!=2 ? INTREG : (mcrep==0x02000004 ? FLTREG : DBLREG);
            /* The next line takes care of compiling 'correct' code for */
            /* void f() { return g();}                                  */
            if ((mcrep & MCR_SIZE_MASK) == 0) cg_exprvoid(x);
            else
            {   (void)cg_exprreg(x, V_Presultreg(rsort));
                retlab = (rsort == DBLREG ? RetDbleLab :
                          rsort == FLTREG ? RetFloatLab : RetIntLab);
            }
        }
    }
    else if (defining_main)
    {
        /* Within main() any return; is treated as return 0; - this   */
        /* is done because the value returned by main() is used as    */
        /* a success code by the library, but on some other C systems */
        /* the value of main() is irrelevant...                       */
        /* Users who go                                               */
        /*      struct foo main(int argc, char *argv[]) { ... }       */
        /* are not protected here!                                    */
        /* AM: unfortunately this stops implicit return warnings...   */
        emit(J_MOVK, V_Presultreg(INTREG), GAP, 0);
        retlab = RetIntLab;
    }
    emitsetspenv(active_binders, (BindList *)NULL);
    /* Not emitsetsp, because we don't want to allow it to be optimised out */
    /* Note also that 'active_binders = NULL;' is, in some sense, missed    */
    /* out here, but because of the emitbranch() this is NECESSARY!         */
    /* @@@ AM wonders why not. */
    /* See comments in the CSE code for the need for a SETSP just before
     * every RETURN.  function modifycode()....
     * Do not optimise this away lightly!
     */

    emitbranch(J_B, retlab);
}

static void cg_loop(Expr *init, Expr *pretest, Expr *step, Cmd *body,
                    Expr *posttest)
{
/* Here I deal with all loops. Many messy things are going on!           */
    struct LoopInfo oloopinfo;
    bool once = at_least_once(init, pretest);
/* A large amount of status belongs with loop constructs, and gets saved */
/* here so that it can be restored at the end of compiling the loop.     */
    oloopinfo = loopinfo;
    loopinfo.breaklab = loopinfo.contlab = 0;
    loopinfo.binders = active_binders;

    if (init != 0) cg_exprvoid(init);  /* the initialiser (if any)       */

    cg_count(cg_current_cmd->fileline);

/* The variable once has been set true if, on the basis if looking at    */
/* the initialiser and the pretest I can tell that the loop will be      */
/* traversed at least once. In that case I will use the pretest as a     */
/* posttest for better generated code.                                   */

    if (!usrdbg(DBG_LINE) || (pretest == 0 && step == 0))
    {   LabelNumber *bodylab = nextlabel(), *steplab = 0;
        if (!once) emitbranch(J_B, steplab = nextlabel());

        start_new_basic_block(bodylab);
        if (body != NULL)
        {   cg_count(body->fileline);
            cg_cmd(body);
        }
        if (loopinfo.contlab != 0) start_new_basic_block(loopinfo.contlab);
        if (step != 0) cg_exprvoid(step);
        if (steplab != 0) start_new_basic_block(steplab);

/* When cg_loop is called there can NEVER be both a pretest and a post-  */
/* test specified. Here I put out the (conditional) branch back up to    */
/* the top of the loop.                                                  */
        cg_count(cg_current_cmd->fileline);
        if (pretest != 0) cg_test(pretest, YES, bodylab);
        else if (posttest != 0) cg_test(posttest, YES, bodylab);
        else emitbranch(J_B, bodylab);
    } else {
        LabelNumber *steplab, *bodylab = nextlabel();
        LabelNumber *testlab = pretest == 0 ? 0 : nextlabel();
        loopinfo.contlab = steplab = (step != 0) ? nextlabel() : testlab;
        if (pretest != 0)
        {   start_new_basic_block(testlab);
            loopinfo.breaklab = nextlabel();
            if (step == 0)
                cg_test(pretest, NO, loopinfo.breaklab);
            else {
                cg_test(pretest, YES, bodylab);
                emitbranch(J_B, loopinfo.breaklab);
            }
        }
        if (step != 0) {
            if (pretest == 0) emitbranch(J_B, bodylab);
            start_new_basic_block(steplab);
            if (step->fileline != 0)
                emitfl(J_INFOLINE, *step->fileline);
            cg_exprvoid(step);
            if (testlab != 0) emitbranch(J_B, testlab);
        }
        start_new_basic_block(bodylab);
        if (body != NULL)
        {   cg_count(body->fileline);
            cg_cmd(body);
        }
        cg_count(cg_current_cmd->fileline);
        if (posttest != 0)
            cg_test(posttest, YES, steplab);
        else
            emitbranch(J_B, steplab);
    }
    if (loopinfo.breaklab != 0) start_new_basic_block(loopinfo.breaklab);
    loopinfo = oloopinfo;
}

static bool dense_case_table(int32 low_value, int32 high_value,
                             int32 ncases)
{
/* This function provides a criterion for selection of a test-and-branch */
/* or a jump-table implementation of a case statement. Note that it is   */
/* necessary to be a little careful about arithmetic overflow in this    */
/* code, and that the constants here will need tuning for the target     */
/* computer's instruction timing characteristics.                        */
    int32 halfspan = high_value/2 - low_value/2;     /* cannot overflow  */
#ifdef TARGET_SWITCH_isdense
    if (TARGET_SWITCH_isdense(ncases,halfspan)) return 1;   /* tuneable  */
#else
    if (halfspan < ncases &&
            /* The next line reflects SUB required on many targets.     */
            /* Should the test be on span, not ncases?                  */
            (ncases > 4 || ncases==4 && low_value==0))
        return 1;            /* good try? */
#endif
    return 0;
}

static void linear_casebranch(VRegnum r, CasePair *v, int32 ncases,
                              LabelNumber *defaultlab)
{
    while (ncases-- != 0)
    {   emit(J_CMPK + Q_EQ, GAP, r, v->caseval);
        emitbranch(J_B + Q_EQ, v->caselab);
        v++;
    }
    emitbranch(J_B, defaultlab);
}

static void table_casebranch(VRegnum r, CasePair *v, int32 ncases,
                             LabelNumber *defaultlab)
{
    int32 m = v[0].caseval, n = v[ncases-1].caseval, i;
    int32 size = n - m + 1;
    LabelNumber **table =
                (LabelNumber **) BindAlloc((size+1) * sizeof(LabelNumber *));
    VRegnum r1 = r;
    if (m != 0)
    {   r1 = fgetregister(INTREG);
        emit(J_SUBK, r1, r, m);
    }
    table[0] = defaultlab;
    for (i=0; i<size; i++)
       table[i+1] = (i+m == v->caseval) ? (v++)->caselab : defaultlab;
    /* It is important that literals are not generated so as to break up */
    /* the branch table that follows - J_CASEBRANCH requests this.       */
    /* Type check kluge in the next line...                              */
    emitcasebranch(r1, table, size + 1);
    if (r1 != r) bfreeregister(r1);
}

static void casebranch(VRegnum r, CasePair *v, int32 ncases,
                       LabelNumber *defaultlab)
{
    if (ncases<5) linear_casebranch(r, v, ncases, defaultlab);
    else if (dense_case_table(v[0].caseval, v[ncases-1].caseval,
                              ncases))
        table_casebranch(r, v, ncases, defaultlab);
    else
    {   int32 mid = ncases/2;
        LabelNumber *l1 = nextlabel();
#ifdef TARGET_LACKS_3WAY_COMPARE
        emit(J_CMPK + Q_GE, GAP, r, v[mid].caseval);
        emitbranch(J_B + Q_GE, l1);
        casebranch(r, v, mid, defaultlab);
        start_new_basic_block(l1);
        casebranch(r, &v[mid], ncases-mid, defaultlab);
#else
/*
 * CSE is told here not to move things which might set the condition code
 * between the two conditional branches below by setting BLKCCLIVE.
 */
        /* The following line is a nasty hack.                          */
        /* It is also not always optimal on such machines.              */
        emit(J_CMPK + Q_XXX, GAP, r, v[mid].caseval);
        blkflags_(bottom_block) |= BLKCCEXPORTED;
        emitbranch(J_B + Q_EQ, v[mid].caselab);
        blkflags_(bottom_block) |= BLKCCLIVE;
        emitbranch(J_B + Q_GT, l1);
        casebranch(r, v, mid, defaultlab);
        start_new_basic_block(l1);
        casebranch(r, &v[mid+1], ncases-mid-1, defaultlab);
#endif
    }
}

static void cg_case_or_default(LabelNumber *l1)
/* Produce a label for a case or default label in a switch.  Note that in   */
/* general we must jump round a stack adjusting jopcode, but to save jop    */
/* and block header space we test fr the common case.                       */
{   if (active_binders == switchinfo.binders)
        start_new_basic_block(l1);
/* the next few lines of code take care of 'case's within blocks with       */
/* declarations:  switch(x) { case 1: { int v[10]; case 2: foo(); }}        */
    else
    {   LabelNumber *l = nextlabel();
        BindList *bl = active_binders;
        emitbranch(J_B, l);
        start_basic_block_at_level(l1, active_binders = switchinfo.binders);
/* Amazing amount of fiddling about in case ICODE section may overflow. */
        emitsetsp(J_SETSPENV, bl);
        start_new_basic_block(l);
    }
    cg_count(cg_current_cmd->fileline);
}

static VRegnum cg_loadconst(int32 n, Expr *e)
{   /* void e if !=NULL and return n - used for things like f()*0     */
    VRegnum r;
    if (e) (void)cg_exprvoid(e);
#ifdef TARGET_R0_ALWAYS_ZERO
    if (n == 0) return virtreg(0,INTREG);
#endif
    emit(J_MOVK, r=fgetregister(INTREG), GAP, n);
    return r;
}

static bool structure_function_value(Expr *x)
{
/* x is an expression (known to be of a structure type). Return true if  */
/* the expression is the result of evaluating a structure-yielding       */
/* function. This information is needed in the case of selections such   */
/* as     f().field     etc.                                             */
/* Oct 92: this needs rationalising now optimise1() does most of this!   */
    switch (h0_(x))
    {
case s_comma:
        return structure_function_value(arg2_(x));
case s_dot:
        return structure_function_value(arg1_(x));
case s_fnap:
        return 1;
case s_cond:
        return structure_function_value(arg2_(x)) ||
               structure_function_value(arg3_(x));
default:
        return 0;
    }
}

static void cg_cond1(Expr *e, bool valneeded, VRegnum targetreg,
                     LabelNumber *l3, bool structload)
{   while (h0_(e)==s_comma)
    {   cg_exprvoid(arg1_(e));
        e = arg2_(e);
    }
    if (h0_(e)==s_cond) (void)cg_cond(e, valneeded, targetreg, l3, structload);
    else if (structload)
    {   VRegnum r;
        emitreg(J_MOVR, targetreg, GAP, r = load_integer_structure(e));
        bfreeregister(r);
    }
    else if (valneeded) (void)cg_exprreg(e, targetreg);
    else
         bfreeregister(cg_expr1(e, valneeded));
    emitbranch(J_B, l3);
}

static VRegnum cg_cond(Expr *c, bool valneeded, VRegnum targetreg,
                       LabelNumber *l3, bool structload)
{
    Expr *b = arg1_(c), *e1 = arg2_(c), *e2 = arg3_(c);
    LabelNumber *l1 = nextlabel();
    if (usrdbg(DBG_LINE) && c->fileline != 0)
        emitfl(J_INFOLINE, *c->fileline);
    cg_test(b, 0, l1);
    cg_cond1(e1, valneeded, targetreg, l3, structload);
    start_new_basic_block(l1);
    cg_cond1(e2, valneeded, targetreg, l3, structload);
    return targetreg;
}

static bool iszero(Expr *x)
{
    return (integer_constant(x) && result2==0);
}

static bool isone(Expr *x)
{
    return (integer_constant(x) && result2==1);
}

static bool isminusone(Expr *x)
{
    return (integer_constant(x) && result2==-1);
}

static int32 ispoweroftwo(Expr *x)
{
    unsigned32 n, r;
    if (!integer_constant(x)) return 0;
    n = result2;
    r = n & (-n);
    if (n == 0 || r != n) return 0;
    r = 0;
    while (n != 1) r++, n >>= 1;
    return r;
}

static void structure_assign(Expr *lhs, Expr *rhs, int32 length)
{
    Expr *e;
/* In a void context I turn a structure assignment into a call to the    */
/* library function memcpy().                                            */
/* Note that casts between structure types are not valid (and so will    */
/* have been filtered out earlier), but casts to the same structure type */
/* can be present (particularly around (a ? b : c) expressions) as an    */
/* artefact of the behaviour of sem. Prune them off here.                */
/* AM Nov 89: The present arrangement of forgeing a call to _memcpy() is */
/* not very satisfactory -- there are problems with tail recursion if    */
/* not TARGET_HAS_BLOCKMOVE and problems with stack addresses if         */
/* TARGET_STACK_MOVES_ONCE (being fixed).                                */
/* To this end, note that 'lhs' ultimately has take_address() and        */
/* typeofexpr() applied to it (but beware the s_assign ref. below).      */
    while (h0_(rhs) == s_cast) rhs = arg1_(rhs);
    switch (h0_(rhs))
    {
case s_comma:
        /* Maybe this recursive call should just be the expansion:       */
        /*  (a = (b,c)) ---> (b, a=c).                                   */
        cg_exprvoid(arg1_(rhs));
        structure_assign(lhs, arg2_(rhs), length);
        return;
case s_assign:
/* A chain of assignments as in    p = q = r   get mapped as follows:    */
/*    (  LET struct *g,                                                  */
/*       g = &q,                                                         */
/*       *g = r,                                                         */
/*       p = *g )                                                        */
/*                 thus &q gets evaluated only once                      */
        {   TypeExpr *t = typeofexpr(rhs);
            Binder *gen = gentempbinder(ptrtotype_(t));
            e = mk_exprlet(s_let,
                         t,
                         mkSynBindList(0, gen),
                         mk_expr2(s_comma,
                                  t,
                                  mk_expr2(s_assign,
                                           ptrtotype_(t),
                                           (Expr *)gen,
                                           take_address(arg1_(rhs))),
                                  mk_expr2(s_comma,
                                           t,
                                           mk_expr2(s_assign,
                                                    t,
                                                    mk_expr1(s_content,
                                                             t,
                                                             (Expr *)gen),
                                                    arg2_(rhs)),
                                           mk_expr2(s_assign,
                                                    t,
                                                    lhs,     /* @@@rework */
                                                    mk_expr1(s_content,
                                                             t,
                                                             (Expr *)gen)))));
        }
        break;
case s_fnap:
        e = mk_expr2(s_fnapstruct, te_void, arg1_(rhs),
                     (Expr *)mkExprList(exprfnargs_(rhs), take_address(lhs)));
        break;
case s_cond:
/* Convert    a = (b ? c : d)                                            */
/*    (LET struct *g,                                                    */
/*       g = &a,                                                         */
/*       b ? (*g = c) : (*g = d))                                        */
/*                                                                       */
        {   TypeExpr *t = typeofexpr(lhs);
            Binder *gen = gentempbinder(ptrtotype_(t));
            e = mk_exprlet(s_let,
                         t,
                         mkSynBindList(0, gen),
                         mk_expr2(s_comma,
                                  t,
                                  mk_expr2(s_assign,
                                           ptrtotype_(t),
                                           (Expr *)gen,
                                           take_address(lhs)),
                                  mk_expr3(s_cond,
                                           t,
                                           arg1_(rhs),
                                           mk_expr2(s_assign,
                                                    t,
                                                    mk_expr1(s_content,
                                                             t,
                                                             (Expr *)gen),
                                                    arg2_(rhs)),
                                           mk_expr2(s_assign,
                                                    t,
                                                    mk_expr1(s_content,
                                                             t,
                                                             (Expr *)gen),
                                                    arg3_(rhs)))));
        }
        break;
default:
        e = mk_expr2(s_fnap, primtype_(bitoftype_(s_void)), sim.memcpyfn,
                (Expr *)mkExprList(mkExprList(mkExprList(0,
                    mkintconst(te_int,length,0)),
                    take_address(rhs)),
                    take_address(lhs)));
        break;
    }
    if (debugging(DEBUG_CG))
    {   eprintf("Structure assignment: ");
        pr_expr(e);
        eprintf("\n");
    }
    cg_exprvoid(e);
}

static VRegnum load_integer_structure(Expr *e)
{
/* e is a structure-valued expression, but one where the value is a      */
/* one-word integer-like quantity. Must behave like cg_expr would but    */
/* with special treatment for function calls.                            */
    switch (h0_(e))
    {
case s_comma:
        cg_exprvoid(arg1_(e));
        return load_integer_structure(arg2_(e));
case s_assign:
        return cg_storein(load_integer_structure(arg2_(e)),
                          arg1_(e), s_assign);
case s_fnap:
        {   TypeExpr *t = type_(e);
            Binder *gen = gentempbinder(ptrtotype_(t));
            bindstg_(gen) |= b_addrof;
            e = mk_expr2(s_fnap, te_void, arg1_(e),     /* AM: s_fnapstruct here? */
                         (Expr *)(
                    mkExprList(exprfnargs_(e), take_address((Expr *)gen))));
            e = mk_exprlet(s_let, t, mkSynBindList(0, gen),
                    mk_expr2(s_comma, t, e, (Expr *)gen));
            return cg_expr(e);
        }
/* Since casts between structure-types are illegal the only sort of cast   */
/* that can be present here is just one re-asserting the type of the       */
/* expression being loaded. Hence I skip over the cast. This certainly     */
/* happens with a structure version of (a = b ? c : d) where the type gets */
/* inserted to give (a = (structure)(b ? c : d)).                          */
case s_cast:
            return load_integer_structure(arg1_(e));
case s_cond:
        {   LabelNumber *l3 = nextlabel();
            VRegnum r = cg_cond(e, 1, reserveregister(INTREG), l3, 1);
            start_new_basic_block(l3);
            return getreservedreg(r);
        }

default:
        return cg_expr(e);
    }
}

static VRegnum cg_expr1(Expr *x, bool valneeded)
{
    AEop op = h0_(x);
/* See the discussion in chroma_check() above.                          */
/* @@@ AM thinks that spilling probably oughtn't to start so late --    */
/* the code here allows 1 single expression to preempt many reg vars.   */
/*          *** RETHINK HERE ***                                        */
/* To avoid running out of registers I do dreadful things here:         */
    if ((nusedregs >= (NTEMPREGS+NARGREGS+NVARREGS-spareregs-4)
#ifndef TARGET_SHARES_INTEGER_AND_FP_REGISTERS
/* AM has unilaterally decided that 2 spare FP regs should always be enough */
/* here (after he has changed the unsigned FIX code).  Anyway, using 3 for  */
/* for 2 on the next line hurts the 370 code generator as EVERY integer     */
/* operation spills!  Suggestion - only spill FP regs when we have a FP     */
/* result - however beware machines like the VAX where FP=INT reg!          */
         || nusedfpregs >= (NFLTTEMPREGS+NFLTARGREGS+NFLTVARREGS-sparefpregs-2)
#endif
         ) &&
        (op!=s_binder && op!=s_integer && op!=s_floatcon &&
         !isstring_(op)))
    {
/* Here I seem a bit short on working registers, so I take drastic steps */
/* and flush all current registers to the stack, do this calculation and */
/* then reload things. Keeping everything straight is a bit of a mess    */
/* since this breaks the usual abstractions about register allocation    */
/* and usage.                                                            */
        BindList *save_binders = active_binders;
        int32 d = current_stackdepth;
        RegList *saveused = usedregs, *saveusedfp = usedfpregs;
        int32 savebits = nusedregs, savefpbits = nusedfpregs;
        int32 spint = spareregs, spfp = sparefpregs;
        VRegnum r;
        SynBindList *things_to_bind = bindlist_for_temps(saveused, saveusedfp);
#ifndef REGSTATS
/*
 * REGSTATS is defined when ACN is building a private system to investigate
 * register allocation behaviour
 */
        if (debugging(DEBUG_SPILL))
        {   cc_msg("Usedregs = %ld, spareregs = %ld\n",
                     (long)nusedregs, (long)spareregs);
#ifndef TARGET_SHARES_INTEGER_AND_FP_REGISTERS
            cc_msg("FP count = %ld, sparefpregs = %ld\n",
                     (long)nusedfpregs, (long)sparefpregs);
#endif
        }
#endif
/* First I must allocate binders on the stack & save all current         */
/* register contents away.                                               */
        cg_bindlist(things_to_bind, 1);
        stash_temps(saveused, saveusedfp, things_to_bind,
                    J_STRV, J_STRFV, J_STRDV);

        nusedregs = nusedfpregs = 0;
        usedregs = usedfpregs = NULL;
        spareregs = sparefpregs = 0;

/* Then compute the value of this expression.                            */
        if (valneeded)
        {   r = cg_expr2(x, 1);
/* I must free the register here because I am about to monkey about with */
/* the tables that show what registers are where. Ugh.                   */
            bfreeregister(r);
        }
        else
        {   r = cg_expr2(x, 0);
            bfreeregister(r);
            r = GAP;
        }

/* Switch back to the outer context of registers.                        */
        usedregs = saveused, usedfpregs = saveusedfp;
        nusedregs = savebits, nusedfpregs = savefpbits;
        spareregs = spint, sparefpregs = spfp;

/* The result of evaluating this expression must be moved to a newly     */
/* allocated register to make sure it does not clash with anything.      */
        if (r != GAP)
        {   RegSort rsort = vregsort(r);
            VRegnum r1 = fgetregister(rsort);
/* Register r is no longer formally allocated (by the chromaticity count */
/* code) but it will still exist!                                        */
            emitreg(floatyop(rsort, J_MOVR, J_MOVFR, J_MOVDR), r1, GAP, r);
            r = r1;
        }

/* Now restore register values                                           */
        stash_temps(saveused, saveusedfp, things_to_bind,
                    J_LDRV, J_LDRFV, J_LDRDV);
        while (things_to_bind)
            things_to_bind = (SynBindList *)discard2((List *)things_to_bind);

/* adjust stack pointer as necessary.                                    */
        emitsetsp(J_SETSPENV, save_binders);
        current_stackdepth = d;

        return r;
    }
    else if (valneeded) return cg_expr2(x, 1);
    else
    {   VRegnum r = cg_expr2(x, 0);
        bfreeregister(r);
        return GAP;
    }
}

static void topdec_init(void)
{
    codebuf_reinit2();
    flowgraph_reinit();
    cse_reinit();
    regalloc_reinit();

    local_binders = regvar_binders = NULL;
    integer_binder = gentempvar(te_int, GAP);
    bindstg_(integer_binder) |= b_addrof;
    double_pad_binder = gentempvar(te_double, GAP);
    bindstg_(double_pad_binder) |= b_addrof;
    double_pad_binder->bindmcrep = 3L<<MCR_SORT_SHIFT | MCR_ALIGN_DOUBLE | 0;

    active_binders = NULL;

    loopinfo.breaklab = loopinfo.contlab = NOTINLOOP;
    switchinfo.endcaselab = switchinfo.defaultlab = NOTINSWITCH;
    structretlab = NOTALABEL;
#ifdef EXTENSION_VALOF
    valofinfo.binders = NULL;
    valofinfo.lab = NULL;
    valofinfo.r = 0;
#endif
    top_block = start_new_basic_block(nextlabel());
    procflags = 0;
    cg_infobodyflag = 0;
    cg_current_cmd = (Cmd *)DUFF_ADDR;
    usedregs = usedfpregs = NULL;
    nusedregs = nusedfpregs = 0;
    spareregs = sparefpregs = 0;
    nreservedregs = 0;
}

BindList *argument_bindlist;

static void forcetostore(BindList *bl)
{   for (; bl != NULL; bl = bl->bindlistcdr) {
        Binder *b = bl->bindlistcar;
        /* Force all user-visible variables in bl to be in store,
           so that longjmp by register jamming has the pcc semantics.
         */
        if (!isgensym(bindsym_(b))) bindxx_(b) = GAP;
    }
}

void cg_topdecl(TopDecl *x, FileLine fl)
{
    int32 argwords;
    if (x==NULL) return;    /* Maybe result of previous error etc */
    switch (h0_(x))
    {

case s_fndef:
        {   Binder *b = x->v_f.fn.name;
            SynBindList *formals = x->v_f.fn.formals;
            Cmd *body = x->v_f.fn.body;
            Symstr *name = bindsym_(b);
            TypeExpr *t = prunetype(bindtype_(b)), *restype;
            int32 resrep;
            fl.p = dbg_notefileline(fl);
/* Object module generation may want to know if this module defines
   main() so that it can establish an entry point. Also while defining
   main() we should ensure that return; is mapped onto return 0;
   N.B. this latter really be done so that regalloc can warn first!! */
            if (name == mainsym &&
                (bindstg_(b) & bitofstg_(s_extern)) != 0)
                defining_main = has_main = YES;
            else defining_main = NO;
            if (h0_(t)!=t_fnap)
            {   syserr(syserr_cg_topdecl);
                return;
            }
            procauxflags = typefnaux_(t).flags;
            topdec_init();
            restype = prunetype(typearg_(t));
            resrep = mcrepoftype(restype);
            nresultregs = 0;
            result_variable = NULL;
/* If a function returns a structure result I give it a hidden extra     */
/* first argument that will point to where the result must be dumped.    */
            if ((resrep >> MCR_SORT_SHIFT) == 3)     /* structure result */
            {
                if (procauxflags & bitoffnaux_(s_structreg)) {
                    int32 n = (resrep & MCR_SIZE_MASK) / sizeof_long;
                    if (n > 1 && n <= NARGREGS)
                        nresultregs = n;
                }
                if (nresultregs == 0) {
                    TypeExpr *pt = ptrtotype_(restype);
                    Symstr *structres = sym_insert_id("__struct_result");
                    result_variable = mk_binder(structres, bitofstg_(s_auto), pt);
                    /* use a name for the result variable which will not be
                     * discarded by debugger table generators
                     */
                    if (usrdbg(DBG_VAR))
#ifdef TARGET_IS_C40
                        dbg_locvar(result_variable, body->fileline, NO);
#else
                        dbg_locvar(result_variable, body->fileline);
#endif
                    result_temporary = gentempbinder(pt);
                    set_local_vregister(result_temporary, mcrepoftype(pt), 0);
#ifndef TARGET_STRUCT_RESULT_REGISTER
                    formals = mkSynBindList(formals, result_variable);
#endif
                }
            }
#ifdef TARGET_IS_ADENART
            if (isadetran(name))
            {   body = adenart_call(body, formals);
                formals = 0;
            }
#endif
/* Here is a ugly mess - 'formals' has SynAlloc store, which is         */
/* corrupted by drop_local_store().  It needs copying for both for the  */
/* use below (PROC_ARGPUSH) and for its use in regalloc.c.              */
            argument_bindlist = (BindList *)binderise(formals);
            if (usrdbg(DBG_VAR))
            {   /* for the basic block started by J_ENTER in cg_bindargs() */
                current_env = (BindListList *) binder_cons2(0, argument_bindlist);
            }
            argwords = cg_bindargs(argument_bindlist, x->v_f.fn.ellipsis);
            cg_cmd(body);
#ifdef never
            /* the following code is maybe what AM would like */
            if (!deadcode && !isprimtype_(restype, s_void))
                cc_warn(cg_warn_implicit_return, symname_(name));
#endif
            /* we know here that fl.f != 0 */
            if (!cg_infobodyflag)
            {
#ifdef TARGET_HAS_PROFILE
                if (profile_option) emitfl(J_COUNT, fl);
#endif
                if (usrdbg(DBG_PROC)) emit(J_INFOBODY, GAP, GAP, 0);
                cg_infobodyflag = 1;
            }
            if (usrdbg(DBG_LINE)) emitfl(J_INFOLINE, fl);
            cg_return(0, !isprimtype_(restype, s_void));

            drop_local_store();
            phasename = "loopopt";
            {
                BindList *invariant_binders = cse_eliminate();
                /* Corrupt regvar_binders and local_binders to get */
                /* a spill_order list for allocate_registers()     */
                drop_local_store();   /* what loopopt used */
                lose_dead_code();     /* before regalloc   */
                if ((procflags & BLKSETJMP) &&
                    (feature & FEATURE_UNIX_STYLE_LONGJMP)) {
                    forcetostore(local_binders);
                    if (!(feature & FEATURE_LET_LONGJMP_CORRUPT_REGVARS))
                        forcetostore(regvar_binders);
                }
                allocate_registers(
                    (BindList *)nconc((List *)invariant_binders,
                        nconc((List *)local_binders, (List *)regvar_binders)));

                drop_local_store();   /* what regalloc used */
            }
            phasename = "machinecode";
/* If (after register allocation etc) an argument is left active in      */
/* memory (rather than being slaved in a register) I will do the full    */
/* entry sequence. Force this by setting PROC_ARGPUSH in that case.      */
/* Note that PROC_ARGADDR will thereby imply PROC_ARGPUSH, but they are  */
/* different in that PROC_ARGADDR suppresses tail recursion (flowgraph.c)*/
/* Also if a big stack frame is needed I tell xxxgen.c to be cautious.   */
            if (greatest_stackdepth > 256) procflags |= PROC_BIGSTACK;
            {   BindList *fb = argument_bindlist;
                while (fb != NULL) {
                    Binder *b1 = fb->bindlistcar;
                    if (bindxx_(b1) == GAP) procflags |= PROC_ARGPUSH;
                    fb = fb->bindlistcdr;
                }
            }
/* Now, also set PROC_ARGPUSH if debugging of local variables is         */
/* requested -- this will ensure that FP is set up.                      */
/* N.B. We could probably optimise this if no vars spill, i.e. the       */
/* debugger *probably* does not need FP, but why bother??                */
            if (usrdbg(DBG_VAR))
#ifdef TARGET_IS_HELIOS
		/*
		 * Helios debugger wants to call a function upon entry
		 * and exit from every function, so suppress leaf
		 * optimisiations and tail recursion
		 */
		
		procflags |= (PROC_ARGPUSH | PROC_ARGADDR);
#else
		procflags |= PROC_ARGPUSH;
#endif

/* The following lines are an attempt to reverse horrors perpetrated     */
/* in back-ends wishing to defer calls to show_entry().                  */
            currentfunction.symstr = bindsym_(b);
            currentfunction.xrflags = bindstg_(b) & bitofstg_(s_static) ?
                             xr_code+xr_defloc : xr_code+xr_defext;
/* This is the place where I put tables into the output stream for use   */
/* by debugging tools - here are the names of functions.                 */
#if !defined TARGET_IS_ARM || defined TARGET_IS_HELIOS
            cg_fnname_offset_in_codeseg =
                (feature & FEATURE_SAVENAME) ?
                    codeseg_function_name(name, argwords) : -1;
            show_entry(name, bindstg_(b) & bitofstg_(s_static) ?
                             xr_code+xr_defloc : xr_code+xr_defext);
#else
/* This work is now done in arm/gen.c so that fn names are only dumped   */
/* for functions which have stack frames.                                */
            cg_fnname_offset_in_codeseg = -1;
#endif
            linearize_code();
            dbg_xendproc(fl);

            show_code(name);

            if (block_cur > max_block) max_block = block_cur;
            if (icode_cur > max_icode) max_icode = icode_cur;

            typefnaux_(t).usedregs = regmaskvec;
        }
        break;

case s_decl:
        break;

default:syserr(syserr_cg_unknown, (long)h0_(x));
        break;
    }
}

void cg_init(void)
{
    codebuf_init();
    datasegbinders = (BindList *) global_cons2(SU_Other, 0, datasegment);
    max_icode = 0; max_block = 0;
    regalloc_init();
    cse_init();
    has_main = NO;

    obj_init();

#ifndef NO_OBJECT_OUTPUT
      if (objstream) obj_header();
#endif
#ifndef NO_ASSEMBLER_OUTPUT
      if (asmstream) asm_header();
#endif

    dbg_init();

    mcdep_init();             /* code for system dependent module header */

    show_entry(bindsym_(codesegment), xr_code+xr_defloc);  /* nasty here */
    show_code(bindsym_(codesegment));                      /* nasty here */

#ifdef TARGET_IS_ACW
    /* this split of mcdep_init() is needed for labelling disassembler */
    /* to work properly */
      mcdep_init2();
      show_entry(stackcheck, xr_code+xr_defloc);
      show_code(stackcheck);
#endif
}

void cg_reinit(void)
{
    codebuf_reinit();
}

void cg_tidy(void)
{
    codebuf_tidy();
    if (debugging(DEBUG_STORE)) {
        cc_msg("Max icode store %ld, block heads %ld bytes\n",
                (long)max_icode, (long)max_block);
    }
    regalloc_tidy();
    cse_tidy();

/* Pad data segment to at least 4 byte multiple (to flush vg_wbuff),    */
/* but pad to multiple of alignof_max if bigger.                        */
/* Do this irrespective of object module format.                        */
    padstatic(alignof_max > 4 ? alignof_max : 4);

    /* maybe do dreverse(CodeXrefs+DataXrefs here one day? */
#ifndef NO_OBJECT_OUTPUT
    if (objstream) obj_trailer();
#endif
#ifndef NO_ASSEMBLER_OUTPUT
    if (asmstream) asm_trailer();
#endif
    localcg_tidy();
}

/* End of cg.c */
