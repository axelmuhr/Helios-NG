/*
 * cfe/syn.c: syntax analysis phase of C compiler, version 157
 * Copyright (C) Codemist Ltd., 1988
 * Copyright (C) Acorn Computers Ltd., 1988
 */

/*
 * RCS $Revision: 1.5 $
 * Checkin $Date: 1993/03/18 16:23:22 $
 * Revising $Author: nickc $
 */

#ifndef NO_VERSION_STRINGS
extern char syn_version[];
char syn_version[] = "\ncfe/syn.c $Revision: 1.5 $ 157\n";
#endif

/* AM Memo: TypeFnAux (e.g. oldstyle) may enable us to kill the        */
/*          dreadful uses of 999 and 1999 in the following.            */
/* AM Memo: reconsider the uses of FileLine fl, in particular the      */
/* possibility of having a FileLine field in DeclRhsList.              */
/* AM Jan 90: rework 'declflag' contexts; fix register syntax,         */
/*            fn-type parsing in formals/casts and 'void' formals.     */
/* AM Memo: uses of bind_level are a nasty hack (see bind.c).          */
/* AM Sep 89: Rework 'incomplete type' code.  Kill some Parkes hacks.  */
/* AM Sep 89: re-work s_typestartsym so that it may only occur in      */
/*            contexts in which macros may reasonably use it.          */
/* AM, Mar 89: rework rd_declarator now that ANSI have ruled on [] and */
/*             () combinations in declarators.  Fix bugs whereby       */
/*             bad combinations of () and [] via typedef were missed.  */

#include "globals.h"
#include "syn.h"
#include "pp.h"        /* for pp_inhashif */
#include "lex.h"
#include "simplify.h"
#include "bind.h"
#include "sem.h"
#include "aetree.h"
#include "builtin.h"
#include "vargen.h"
#include "mcdep.h"     /* for dbg_xxx */
#include "store.h"
#include "errors.h"
#include "aeops.h"

/* The next line is in flux, but flags code which is invented, and     */
/* does not correspond to a user written action.  E.g. narrowing in    */
/* f(x) float x; {...}.  Note that x = 1 in { int x = 1; is not so.    */
static FileLine syn_invented_fl = { 0, 0, 0 };
/* Code may test for invented FileLines by testing (fl.f == 0)!!!      */

#define syn_err     cc_err
#define syn_rerr    cc_rerr
#define syn_warn    cc_warn
#define syn_pccwarn cc_pccwarn

/* forward references within this file - reorganise to reduce */
static void ensure_formals_typed(DeclRhsList *d,bool proto);
static TypeExpr *rd_typename(void);
static Expr *rd_expr(int n);
static Expr *rd_prefixexp(int n);
#ifdef EXTENSION_VALOF
static Cmd *rd_block(bool inner);
#endif
static Cmd *rd_command(void);
static DeclRhsList *rd_decllist(int declflag);
static TagMemList *rd_strdecl(TagBinder *);
static TagMemList *rd_enumdecl(TypeExpr *t);

/* newer comments...
 * 2. (7-apr-86 further change):
 *      Extra field (a BindList) of formal params added to s_fndef
 *      (think of as like lambda).  It has s_binder entries.
 * 3. Diads (incl fnap) and monads have a type field type_() at offset 1.
 *    Use arg1_ and arg2_ to get args.
 *  1. declarators are now turned into type expressions a la Algol68.
 *  3. sizeof is turned into an integer constant by the parser.
 *     s_integer has therefore a intorig_() node showing the original
 *     expression for error messages (e.g. "sizeof(int) = 9;")
 */

/* some general comments...
   0.  AE tree nodes have an h0_() field specifying which AEop (small int)
       they contain and then several pointers representing subtrees.
   1.  list types in general (like actual parameters, block declarations
       block commands) are stored without tag fields, in the usual lisp
       form.  Use cdr_() = h0_() for CDR.  0 denotes the end.
       They are read using tail pointers to avoid excess recursion.
   2.  Missing, or empty expressions or commands are stored as 0.
       E.g. "for (;;)" or "if e then c;".
       Note that the empty statement ";" is also stored as 0 and
       hence "if e then c else;" is indistiguishable from the above.
   3.  Note the curious format of string constants - for ANSI
       implicit string constant concatenation - we have
       a string node has a pointer to a LIST of string elements.
   4.  Note that labelled statements require 1 symbol of lookahead
       to distinguish them from expression statements.
       See POSSLABEL uses below.
   5.  The allocators list1, ..., list6 create the parse tree.
       Moreover no permanent structure is allocated in them
       so that the allocator free pointer can be reset after reading
       and code-generating each top-level phrase.
*/

bool implicit_return_ok;

#define B_OMITTYPE b_synbit1
#define B_DECLMADE b_synbit2

/* values for 'declflag' below - may be or'ed in sensible combinations.
   NB.  I (AM) regard these as entirely private to parsing */
/* Jan 90: values in 'declflag' being re-organised to be C 'contexts'.  */
/* The values are 'sparse' to allow quick testing for membership of     */
/* sets like CONC_DECLARATOR.                                           */

/* #define DUPL_OK            0x001 */      /* defined in bind.h */
/* #define TOPLEVEL           0x002 */      /* defined in bind.h */
#define POINTERTYPES       0x004
#define LENGTH_OK          0x008  /* i.e. in struct/union */
#define BLOCKHEAD          0x010
#define ARG_TYPES          0x020  /* declaring formals, after e.g. 'f(x)'  */
#define FORMAL             0x040  /* inside ()'s: both (int a) and (a) OK. */
#define ABS_DECLARATOR     0x080  /* i.e. typename for case/sizeof.        */
/* contexts in which 'storage classes' are allowed (vestigial):         */
#define STGCLASS_OK        (TOPLEVEL|BLOCKHEAD|FORMAL|ARG_TYPES)
/* contexts in which a type is REQUIRED (typename and struct member):   */
#define TYPE_NEEDED        (ABS_DECLARATOR|LENGTH_OK)
/* contexts in which declarators must be CONCRETE.                      */
#define CONC_DECLARATOR    (TOPLEVEL|BLOCKHEAD|ARG_TYPES|LENGTH_OK)

#define istypedefname_(sv) (symdata_(sv)!=NULL &&  \
             (bindstg_(symdata_(sv))&bitofstg_(s_typedef)))

/* parsing table initialisation... */
static int32 illtypecombination[NUM_OF_TYPES];

/* operator priority for rd_expr() ... */
static char lpriovec[s_NUMSYMS];
#define lprio_(op) lpriovec[op]
#define rprio_(op) (lpriovec[op] | 1)

/* excuse low cunning! */

#define UPTOCOMMA 11
#define PASTCOMMA 10

/* AM: I intend to put other things in lpriovec - such as a bit
   to indicate arg_() must be an lvalue.  E.g.
*/
static void initpriovec(void)
{   AEop s,t;
    AEop i;
    for (s = s_char; istypestarter_(s); s++)
        illtypecombination[shiftoftype_(s)] = ~0;
    illtypecombination[shiftoftype_(s_signed)] =
    illtypecombination[shiftoftype_(s_unsigned)] =
        ~(bitoftype_(s_int) | bitoftype_(s_char) |
          bitoftype_(s_long) | bitoftype_(s_short) |
          bitoftype_(s_const) | bitoftype_(s_volatile));
    illtypecombination[shiftoftype_(s_long)] =
        ~(bitoftype_(s_int) | bitoftype_(s_signed) | bitoftype_(s_unsigned) |
          bitoftype_(s_double) | bitoftype_(s_float) |
          bitoftype_(s_const) | bitoftype_(s_volatile));
    illtypecombination[shiftoftype_(s_short)] =
        ~(bitoftype_(s_int) | bitoftype_(s_signed) | bitoftype_(s_unsigned) |
          bitoftype_(s_const) | bitoftype_(s_volatile));
    illtypecombination[shiftoftype_(s_const)] = bitoftype_(s_const);
    illtypecombination[shiftoftype_(s_volatile)] = bitoftype_(s_volatile);
    /* now symmetrise: */
    for (s = s_char; istypestarter_(s); s++)
      for (t = s_char; istypestarter_(t); t++)
        if (!(illtypecombination[shiftoftype_(s)] & bitoftype_(t)))
          illtypecombination[shiftoftype_(t)] &= ~bitoftype_(s);

    for (i=0; i<s_NUMSYMS; i++)
        lpriovec[i] = (isassignop_(i) ? 13 : 0);
    lpriovec[s_comma] = 10;
    lpriovec[s_assign] = 13;
    lpriovec[s_cond] = 15;
    lpriovec[s_oror] = 16;
    lpriovec[s_andand] = 18;
    lpriovec[s_or] = 20;
    lpriovec[s_xor] = 22;
    lpriovec[s_and] = 24;
    lpriovec[s_equalequal] =
    lpriovec[s_notequal] = 26;
    lpriovec[s_less] =
    lpriovec[s_greater] =
    lpriovec[s_lessequal] =
    lpriovec[s_greaterequal] = 28;
    lpriovec[s_leftshift] =
    lpriovec[s_rightshift] = 30;
    lpriovec[s_plus] =
    lpriovec[s_minus] = 32;
    lpriovec[s_times] =
    lpriovec[s_div] =
    lpriovec[s_rem] = 34;
}

/* check_bitsize and check_bittype get prunetype'd types.               */
static Expr *check_bitsize(Expr *e, TypeExpr *t, Symstr *sv)
{   unsigned32 n;
    unsigned32 size = MAXBITSIZE;
    n = evaluate(e);
    /*
     * In PCC mode bit fields may be 'char', 'short', 'long' or 'enum'.
     * In ANSI mode bit fields must (else undefined) be 'int'.
     */
    if (h0_(t) == s_typespec)
    {   if (typespecmap_(t) & bitoftype_(s_char)) size = 8;
        else if (typespecmap_(t) & bitoftype_(s_short)) size = 8*sizeof_short;
        else if (typespecmap_(t) & bitoftype_(s_long)) size = 8*sizeof_long;
        /* else treat enum as 'int'.                                    */
    }
    if (n > size /* unsigned! */) cc_err(syn_err_bitsize, (long)n);
    else if (n==0 && sv!=NULL) n = -1, cc_err(syn_err_zerobitsize);
    if (n > size || h0_(e) != s_integer) e = globalize_int(1);
    return e;
}

static TypeExpr *check_bittype(TypeExpr *t)
{   /*
     * In PCC mode, it's important NOT to translate char and short
     * types to int. This causes a fatal loss of information.
     * Types 'enum', and 'long' ARE so translated.
     * @@@ beware this if sizeof_int != sizeof_long.
     */
#define PCC_extrabittypes  (bitoftype_(s_char)|bitoftype_(s_short)| \
                            bitoftype_(s_enum)|bitoftype_(s_long))
    if ((feature & FEATURE_PCC) &&
        (h0_(t) == s_typespec)  &&
        (typespecmap_(t) & PCC_extrabittypes))
    {
        t = primtype_((typespecmap_(t) | bitoftype_(s_int)) &
                     ~(bitoftype_(s_enum)|bitoftype_(s_long)));
    }
#undef PCC_extrabittypes
#define ANSI_OKbittypes  (bitoftype_(s_int)| \
                          bitoftype_(s_unsigned)|bitoftype_(s_signed)| \
                          bitoftype_(s_const)|bitoftype_(s_volatile))
    else if (h0_(t) != s_typespec)
    {
        syn_err(syn_rerr_bitfield, t);
        t = te_int;
    }
    else if (typespecmap_(t) & ~ANSI_OKbittypes)
    {
        syn_rerr(syn_rerr_bitfield,
        primtype_(typespecmap_(t) & ~ANSI_OKbittypes));
        t = te_int;
    }
#undef ANSI_OKbittypes
/* sem.c is responsible for the interpretation of 'plain' int bitfields     */
/* as signed/unsigned (q.v. under FEATURE_SIGNED_CHAR).                     */
    return primtype_(typespecmap_(t) | BITFIELD);
}

static Expr *check_arraysize(Expr *e)
{   unsigned32 n;
    n = evaluate(e);
    if (n == 0 && !(suppress & D_ZEROARRAY)) syn_pccwarn(syn_rerr_array_0);
/* The limit imposed here on array sizes is rather ARBITRARY, but char */
/* arrays that consume over 16Mbytes seem silly at least in 1988/89!   */
    if (n > 0xffffff) syn_err(syn_err_arraysize, (long)n);
    if (n > 0xffffff || h0_(e) != s_integer) e = globalize_int(1);
    return e;
}

static char *ctxtofdeclflag(int f)
{  if (f & POINTERTYPES)   return errname_pointertypes;
   if (f & TOPLEVEL)       return errname_toplevel;
   if (f & LENGTH_OK)      return errname_structelement;
   if (f & FORMAL)         return errname_formalarg;
   if (f & ARG_TYPES)      return errname_formaltype;
   if (f & BLOCKHEAD)      return errname_blockhead;
   if (f & ABS_DECLARATOR) return errname_typename;
                           return errname_unknown;
}

#ifdef never
bool issymb(AEop s)
{
    if (curlex.sym == s) { nextsym(); return 1; }
    return 0;
}
#endif

static void checkfor_ket(AEop s)
{
/* The next lines are a long-stop against loops in error recovery */
    if (++errs_on_this_sym > 4)
    {   syn_err(syn_err_expecteda, s);
        nextsym();
    }
    else if (curlex.sym == s) nextsym();
    else syn_err(syn_err_expected, s);
}

static void checkfor_delimiter_ket(AEop s, char *more)
                            /* as checkfor_ket but less read-ahead */
{
/* The next lines are a long-stop against loops in error recovery */
    if (++errs_on_this_sym > 4)
    {   syn_err(syn_err_expected1a, s, more);
        nextsym();
    }
    else if (curlex.sym == s) curlex.sym = s_nothing;
    else syn_err(syn_err_expected1, s, more);
}

static void checkfor_2ket(AEop s, AEop t)
{
/* The next lines are a long-stop against loops in error recovery */
    if (++errs_on_this_sym > 4)
    {   syn_err(syn_err_expected2a, s, t);
        nextsym();
    }
    else if (curlex.sym == s) nextsym();
    else syn_rerr(syn_err_expected2, s, t, s);
}

static void checkfor_delimiter_2ket(AEop s, AEop t)
{
    if (curlex.sym == s) curlex.sym = s_nothing;
    else checkfor_2ket(s, t);
}

/* now to get on with it. */

static Expr *rd_ANSIstring(void)
{
    AEop op = curlex.sym;              /* s_string or s_wstring */
    StringSegList *p,*q;
/* Note that the list of string segments must last longer than most     */
/* other parts of the parse tree, hence use binder_list3() here.        */
    q = p = (StringSegList *) binder_list3(0, curlex.a1.s, curlex.a2.len);
    while (nextsym() == s_string || curlex.sym == s_wstring)
        if (curlex.sym == op)
            q = q->strsegcdr =
                (StringSegList *) binder_list3(0, curlex.a1.s, curlex.a2.len);
        else
            cc_err(syn_err_mix_strings);
    return (Expr *)syn_list2(op, p);
}

static ExprList *rd_exprlist(void)
{
    ExprList *p,*q;
    p = q = (ExprList *) mkExprList(0, rd_expr(UPTOCOMMA));
    while (curlex.sym == s_comma)
    {   nextsym();
        q = cdr_(q) = (ExprList *) mkExprList(0, rd_expr(UPTOCOMMA));
    }
    return p;
}

/* Notionally, rd_primary and rd_prefixexp should take no argument.
   However, it is most convenient to give it an argument 'labelhack'
   which we set to POSSLABEL to indicate statement contexts.
   In such contexts the possible expressions are 'label:' or <expression>.
   This is neccessary to enable lookahead past s_identifier's in statement
   context to see if there is a colon, with represents a label, or an
   operator like '='.  Consider "a:b=c;" or "a:b++;".
   Note that this must be done by rd_primary since labels and variables
   have different environments.
   Doing this re-entrantly is important since "#if..." happens rather
   asynchronously.
*/

#define NOLABEL   PASTCOMMA        /* for clarity */
#define POSSLABEL (PASTCOMMA-2)    /* lower (= same) priority as PASTCOMMA */

static int elselast;  /* private to rd_command(), cleared by valof */
static Cmd *cur_switch, *cur_loop;
static AEop cur_break;   /* what 'break' means i.e. s_break or s_endcase */
static TypeExpr *cur_restype, *cur_switchtype;
#ifdef EXTENSION_VALOF
static TypeExpr *valof_block_result_type;
#endif

static Expr *rd_primaryexp(int labelhack)
{
    Expr *a;
    AEop op;
    switch (op = curlex.sym)
    {
default:
        syn_err(syn_err_expected_expr, op);
        return errornode;
case s_lpar:
        nextsym();
/* The next little bit supports a new keyword "__type" which is treated */
/* almost as whitespace here, but which is used by a few macros as a    */
/* clue to the parser that certain parenthesised expressions are casts  */
/* and not just arithmetic.  This (only) aids error recovery.           */
        if (isdeclstarter_(curlex.sym) ||   /* moan if stgclass in rd_declspec */
            (curlex.sym == s_identifier && !pp_inhashif
                                        && istypedefname_(curlex.a1.sv)))
        {   TypeExpr *t;
            if (curlex.sym == s_typestartsym) nextsym();
            t = rd_typename();     /* TYPE_NEEDED aided by typestartsym */
            if (t==0) syserr(syserr_rd_typename);
            checkfor_ket(s_rpar);
#ifdef EXTENSION_VALOF
            /* invalidate syntactically top level "int a = (int){...};" */
            if (curlex.sym == s_lbrace && cur_restype)
            {   Cmd *c;
                TypeExpr *saver = valof_block_result_type;
                if ((suppress & D_VALOFBLOCKS) == 0)
                   syn_err(syn_err_valof_block);
/* Set a flag so that 'resultis' is recognized as such */
                if (equivtype(t, te_void))
                {   syn_rerr(syn_rerr_void_valof);
                    t = te_int;     /* Fixup to be type int */
                }
                inside_valof_block++;
                valof_block_result_type = t;
                c = rd_block(1);
                elselast = 0;
                inside_valof_block--;
                valof_block_result_type = saver;
                nextsym();
                return mk_expr_valof(s_valof, t, c);
            }
            else
#endif
            return mkcast(s_cast, rd_prefixexp(NOLABEL), t);
        }
        a = rd_expr(PASTCOMMA);
        checkfor_ket(s_rpar);
        break;
case s_string:                  /* cope with ANSI justaposed concatenation */
case s_wstring:
        a = rd_ANSIstring();
        break;
case s_identifier:
    {   Symstr *sv = curlex.a1.sv;
        bool advanced = 0;  /* read-ahead as little as possible for ... */
        if (pp_inhashif)    /* ... error message line numbers.          */
        {   /* the following warning is a good idea - consider:
               enum foo { a,b }; #if a==b ... */
            if ((pp_inhashif == 1) || (feature & FEATURE_FUSSY))
                syn_warn(syn_warn_hashif_undef, symname_(sv));
            a = mkintconst(te_int,0,0);
        }
        else
        {   Binder *b;
            if (labelhack == POSSLABEL)
            {   nextsym(), advanced = 1;
                if (curlex.sym == s_colon)
                    /* note we do NOT nextsym() here so no postfix forms */
                    return (Expr *) syn_list2(s_colon,sv);
            }
            if ((b = symdata_(sv)) == NULL)
            {   if (!advanced) nextsym(), advanced = 1;
                if (curlex.sym==s_lpar)
                {   if (warn_implicit_fns)
                        syn_warn(syn_warn_invent_extern, symname_(sv));
                    else xwarncount++;
                }
                else
                  syn_rerr(syn_rerr_undeclared, symname_(sv));
                implicit_decl(sv, curlex.sym==s_lpar);
                a = (Expr *)(b = symdata_(sv));
            }
            else if (istypedefname_(sv)) /* macro retests b==NULL! */
            {   syn_err(syn_err_typedef, sv);
                a = errornode;
            }
            else if (isenumconst_(b))
                a = mkintconst(te_int,bindaddr_(b),(Expr *)b);
            /* the last line probably should have enum type - see reducediad */
            else a = (Expr *)b;           /* use 'variable' current binding */
            binduses_(b) |= u_referenced;
        }
        if (!advanced) nextsym();
        break;
    }
case s_floatcon:
        a = (Expr *)curlex.a1.fc;
        nextsym();
        break;
case s_integer:            /* invent some more te_xxx for the next line? */
        a = mkintconst((curlex.a2.flag == bitoftype_(s_int) ? te_int :
                                          primtype_(curlex.a2.flag)),
                       curlex.a1.i, 0);
        nextsym();
        break;
    }
    return a;
}

static Expr *mkfnaporbuiltin(Expr *e, ExprList *l)
/* Here I have special processing for a pseudo-function-call to
 * ___assert(bool, string) which (at this very early stage) is turned into
 * (void)0, but if the boolean is not a compile time nonzero constant
 * the string is used to generate a diagnostic.
 */
{   if (h0_(e) == s_binder && bindsym_((Binder *)e) == assertsym)
    {   Expr *a1, *a2;
        ExprList *ll = l;
        if (ll != NULL)
        {   a1 = optimise0(exprcar_(ll));
            ll = cdr_(ll);
            if (ll != NULL) a2 = optimise0(exprcar_(ll));
            else a2 = 0;
        }
        else a1 = a2 = 0;
        if (a1 == 0 || h0_(a1) != s_integer || intval_(a1) == 0)
            cc_err(syn_err_assertion, a2);
/* map the assert() into (void)0 */
        return mkcast(s_cast, mkintconst(te_int, 0, e), te_void);
    }
    return mkfnap(e,l);
}

static Expr *rd_postfix(Expr *a)
{   AEop op;
        for (;;) switch (op = curlex.sym)
        {   case s_plusplus:
            case s_minusminus:
                nextsym();
                a = mkunary(postop_(op), a);
                break;
            case s_lpar:
                nextsym();
                a = mkfnaporbuiltin(a,
                        ((curlex.sym == s_rpar) ? 0 : rd_exprlist()));
                checkfor_2ket(s_rpar,s_comma);
                break;
            case s_lbracket:
                nextsym();
                a = mkbinary(s_subscript, a, rd_expr(PASTCOMMA));
                checkfor_ket(s_rbracket);
                break;
            case s_dot:
            case s_arrow:
                nextsym();
                if (curlex.sym != s_identifier)
                    syn_err(syn_err_expected_id, op),
                    a = errornode;
                else a = mkfieldselector(op, a, curlex.a1.sv);
                nextsym();
                break;
            default:
                return a;
        }
}

static int32 codeoftype(TypeExpr *x)
/*
 * This concocts an integer to return as the value of ___typeof(xx)
 * where ___typeof can be used wherever sizeof can. The objective is to make
 * it possible to (mostly) stdarg.h to make more checks and tests to
 * spot errors and generate code that depends in fine ways on the types
 * of objects.  The coding used here at present is NO GOOD, and needs to be
 * thought out...  I rather suspect that the best fixup here is to hand
 * back a type descriptor as defined for use with coff.
 */
{
    x = prunetype(x);
    switch (h0_(x))
    {
case s_typespec:
        return typespecmap_(x);
case t_subscript:
case t_content:
        return 0x10000000;
case t_fnap:
        return 0x20000000;
default:
        syserr(syserr_codeoftype);
        return 0x40000000;
    }
}

static Expr *rd_prefixexp(int labelhack)
{
    AEop op;
    Expr *a;
    switch (op = curlex.sym)
    {
case s_plus:                            /* new ANSI feature */
                if (feature & FEATURE_PCC)
                   syn_warn(syn_warn_unary_plus);
case s_and:
case s_times:
case s_minus:   op = unaryop_(op);      /* drop through */
case s_plusplus:
case s_minusminus:
case s_bitnot:
case s_boolnot: nextsym();
                return mkunary(op, rd_prefixexp(NOLABEL));
case s_typeof:  /* ncc extension keyword */
case s_sizeof:
        nextsym();
/* N.B. Ansi require sizeof to return an unsigned integer type */
        if (curlex.sym == s_lpar)
        {   nextsym();
            if (isdeclstarter_(curlex.sym) || /* moan if stgclass in rd_declspec */
                (curlex.sym == s_identifier && istypedefname_(curlex.a1.sv)))
            {   TypeExpr *t;
                if (curlex.sym == s_typestartsym) nextsym();
                t = rd_typename(); /* TYPE_NEEDED aided by typestartsym */
                if (t==0) syserr(syserr_rd_typename);
                checkfor_ket(s_rpar);
                return mkintconst(
                    !(feature & FEATURE_PCC) ? te_uint : te_int,
                        op == s_sizeof ? sizeoftype(t) : codeoftype(t),
                        syn_list2(op == s_sizeof ? s_sizeoftype : s_typeoftype,
                                  t));
            }
            a = rd_expr(PASTCOMMA);
            checkfor_ket(s_rpar);
            a = rd_postfix(a);   /* for sizeof (f)() etc */
        }
        else a = rd_prefixexp(NOLABEL);
        return mkintconst(
            !(feature & FEATURE_PCC) ? te_uint : te_int,
                op == s_sizeof ? sizeoftype(typeofexpr(a)) :
                                 codeoftype(typeofexpr(a)),
                syn_list2(op == s_sizeof ? s_sizeofexpr : s_typeofexpr, a));
default:
        return rd_postfix(rd_primaryexp(labelhack));
    }
}

static Expr *rd_expr(int n)
{   AEop op;
    Expr *a = rd_prefixexp(n);    /* see POSSLABEL */
    /* note that the loop does not go round if op == s_colon after a label */
    while (lprio_(op = curlex.sym) >= n)
    {   nextsym();
        if (op == s_cond)
        {   Expr *b = rd_expr(PASTCOMMA);
            checkfor_ket(s_colon);
            a = mkcond(a, b, rd_expr(rprio_(op)));
        }
        else a = mkbinary(op, a, rd_expr(rprio_(op)));
    }
    return a;
}

/* The next routine is used by the preprocessor to parse #if and #elif. */
/* It relies on the caller (pp) to set pp_inhashif (see cfe/pp.h).      */
bool syn_hashif()
{    /* note that we read the largest possible expression (rather
        that the ANSI "constant expression" syntax which excludes
        (top level only!!!) '+=', ',' etc.  Then we can moan better
        at the semantic level.
     */
     Expr *e;
/* the next few lines are somewhat yukky.  Think about how to improve    */
     nextsym_for_hashif();  /* precondition for rd_expr() is that curlex.sym is valid */
     e = optimise0(rd_expr(PASTCOMMA));
     if (curlex.sym != s_eol)
     {   syn_err(curlex.sym == s_eof ? syn_err_hashif_eof : syn_err_hashif_junk);
         while (curlex.sym != s_eof && curlex.sym != s_eol) nextsym();
     }
     if (e == 0 || h0_(e) != s_integer)
     {   if (e != 0) moan_nonconst(e, syn_moan_hashif);
         return 0;
     }
     return intval_(e) != 0;   /* != 0 essential if int != long */
}

static Expr *rd_condition(AEop op)
{
    Expr *e;
    if (curlex.sym == s_lpar)
    {   nextsym();
        e = rd_expr(PASTCOMMA);
        checkfor_ket(s_rpar);
    }
    else
    {   syn_rerr(syn_rerr_insert_parens, op);
        e = rd_expr(PASTCOMMA);
    }
    return (op == s_switch ? mkswitch(e) : mktest(op,e));
}

/* the next routine rd_init() has a coroutine-like linkage with vargen.c */
/* it would otherwise be a nice simple recursive routine! */

static int32 syn_initdepth;  /* only used by these fns and rd_declrhslist */
static Expr *syn_initpeek;
bool syn_undohack;           /* exported to vargen.c                      */

/* these are clearly stubs of nice recursive calls! */

int32 syn_begin_agg(void)
{   if (syn_initdepth < 0) return 0;
    if (curlex.sym == s_lbrace)
    {   nextsym();
        syn_initdepth++;
        return 1;
    }
    return 0;
}

void syn_end_agg(int32 beganbrace)
{   if (beganbrace)
    {   if (curlex.sym != s_rbrace)
        {   Expr *result;
            switch (beganbrace)
            {   case 1: syn_err(syn_err_initialisers);
                        break;
                case 2: syn_err(syn_err_initialisers1);
                        break;
            }
            /*
             * Skip the rest quitely !
             * However, beware because if a symbol like ';' if found
             * we end up in a never ending loop.
             * Thus, if an error is found bail out !!!
             */
            for
            (
                result=syn_rdinit(0,4);
                result && result != errornode;
                result=syn_rdinit(0,4)
            );
        }
        else if (beganbrace == 2)
            syn_warn(syn_warn_spurious_braces);
        nextsym();
        if (--syn_initdepth > 0 && curlex.sym != s_rbrace)
        {   checkfor_2ket(s_comma, s_rbrace);  /* @@@ improve w.r.t e.g int */
            if (curlex.sym == s_semicolon)     /* probably badly lost... */
                syn_initdepth = -1;
        }
        if (syn_initdepth == 0) syn_initdepth = -1;
    }
}

/* flag values: 0 normal, 1 read string or unread, 4 skip silently */
Expr *syn_rdinit(TypeExpr *t, int32 flag)
/* t is a non-aggregate type - read its initialiser */
/* t being a subscript type is treated specially for 'char x[] = "abc"' */
{   Expr *e;
    if (syn_undohack)
    {   syn_undohack = 0;
        if (syn_initpeek == 0) return 0;
        if (t) e = mkcast(s_assign,syn_initpeek,t);
        else { syserr(syserr_rdinit); return errornode; }
        return e;
    }
    if (syn_initdepth < 0) return 0;
    if (curlex.sym == s_rbrace) return 0;
    if (curlex.sym == s_lbrace)        /* nasty ANSI optional "int x = {1}" */
    {   if (flag == 1) return 0;   /* char x[] = {"abc"} illegal I presume */
        (void) syn_begin_agg();    /* always succeeds */
        e = rd_expr(UPTOCOMMA);
        if (t) e = mkcast(s_assign,e,t);
        if (curlex.sym != s_rbrace)
            checkfor_2ket(s_comma, s_rbrace);  /* @@@ improve w.r.t e.g int */
        syn_end_agg(flag == 4 ? 3 : 2);            /* special flags */
        return e;
    }
    syn_initpeek = e = rd_expr(UPTOCOMMA);
    if (t) e = mkcast(s_assign,e,t);
    if (syn_initdepth > 0 && curlex.sym != s_rbrace)
        checkfor_2ket(s_comma, s_rbrace);  /* @@@ improve w.r.t e.g int */
    if (syn_initdepth == 0) syn_initdepth = -1;  /* one only */
    return e;
}

bool syn_canrdinit(void)
{  if (syn_undohack) return 1;
   if (syn_initdepth < 0 || curlex.sym == s_rbrace ||
/*
 * The next test is intended to make recovery from
 *    int x[] = { int y;
 * and similar cases at least a little better.  In initialisers I
 * expect that the only things legal are lists of expressions sometimes
 * enclosed in braces, so things that can start declarators (such as type
 * names, and words like 'unsigned' ought to be illegal.
 * Putting this in stops some infinite loops in error recovery, but
 * the parser still gets pretty well messed up by the example shown above.
 */
       isdeclstarter_(curlex.sym)) return 0;
   return 1;
}

/* command reading routines... */

static Cmd *rd_block(bool inner)
{
    DeclRhsList *d,*dp;
    CmdList  *c,*cq = NULL,*ct;
    Unbinder *oldvars = NULL;
    Untagbinder *oldtags = NULL;
    FileLine fl; fl.f = pp_cisname, fl.l = pp_linect;
    fl.p = dbg_notefileline(fl);
    nextsym();
    if (inner) (oldvars = push_varenv(), oldtags = push_tagenv());
    d = rd_decllist(BLOCKHEAD);
    c = 0;
    for (dp = d; dp != 0; dp = dp->declcdr)  /* add dynamic inits    */
    {   Expr *e = dp->declinit;              /* see genstaticparts() */
        if (e != 0)
        {   if (debugging(DEBUG_SYN)) cc_msg("[Init]");
            /*
             * NB field dp->fileline introduced by RCC 25-Mar-88.
             */
            ct = (CmdList*)mkCmdList(0, mk_cmd_e(s_semicolon, dp->fileline, e));
            if (c == 0) c = cq = ct;
            else { cdr_(cq) = ct; cq = ct; }
        }
        ((SynBindList *)dp)->bindlistcar = dp->declbind;    /* re-use store */
    }
    while (curlex.sym != s_rbrace && curlex.sym != s_eof)
    {   ct = (CmdList *) mkCmdList(0, rd_command());
        if (curlex.sym == s_nothing) nextsym();
        if (c == 0) c = cq = ct;
        else { cdr_(cq) = ct; cq = ct; }
    }
    checkfor_delimiter_ket(s_rbrace, "");
/* the next line MUST be executed, even if a syntax error occurs to
   avoid global/local storage inconsistency after error recovery.     */
    if (inner) (pop_varenv(oldvars), pop_tagenv(oldtags));
/* note the BindList/DeclRhsList pun */
    return mk_cmd_block(fl, (SynBindList *)d, c);
}

static bool isexprstarter(AEop op)
/* implement as macro using bool array? */
{ switch (op)
    {   default: return 0;
        case s_lpar:    /* primary expression starters */
        case s_string: case s_wstring: case s_integer: case s_floatcon:
        case s_identifier:
        case s_and:     /* prefix expression starters */
        case s_times: case s_plus: case s_minus:
        case s_plusplus: case s_minusminus:
        case s_bitnot: case s_boolnot: case s_sizeof:
                 return 1;
    }
}

/*
   NB - many commands are terminated by a semicolon - here (rd_command())
   when I find the semicolon I set curlex.sym to s_nothing rather than reading
   ahead further. This reduces spurious read-ahead.
*/

/* Beware: the code for associating case labels with their enclosing    */
/* 'switch' which follows assumes that no such labelled command is      */
/* later thrown away by error recovery (else cg etc. will be unhappy).  */
static Cmd *adddefault(FileLine fl)
{    if (cur_switch == 0)
     {   syn_err(syn_err_default); return 0; }
     if (switch_default_(cur_switch))
     {   syn_err(syn_err_default1); return 0; }
     return (switch_default_(cur_switch) = mk_cmd_default(fl, 0));
}

static Cmd *addcase(Expr *e, FileLine fl)
{    Cmd *p,*q; int32 n;
     if (cur_switch == 0)
     {   syn_err(syn_err_case);
         return 0;
     }
     if (h0_(e) != s_integer)
     {   moan_nonconst(e, syn_moan_case);
         return 0;
     }
     n = intval_(e);
     /* the insertion sort code which follows is linear for increasing
        case expressions - it sorts into reverse (int) order (reverse later)?
        Anyway, ACN's CG code like them this way.
     */
     for (p = switch_caselist_(cur_switch), q=0; p!=0; q=p, p=case_next_(p))
     {   Expr *t = cmd1e_(p);
         if (debugging(DEBUG_SYN))
             cc_msg("Comparing cases %ld %ld\n", (long)n, (long)intval_(t));
         if (n >= intval_(t))
         {   if (n > intval_(t)) break;
             syn_err(syn_err_case1, (long)n);
             return 0;
         }
     }
     {   Cmd *r = mk_cmd_case(fl, e, 0, p);
         if (q == 0)
             return switch_caselist_(cur_switch) = r;
         else
             return case_next_(q) = r;
     }
}


static Cmd *rd_command(void)
{
    AEop op;
    Cmd *c;
    Expr *e;
    FileLine fl; fl.f = pp_cisname, fl.l = pp_linect;
    fl.p = dbg_notefileline(fl);
    switch (op = curlex.sym)
    {
case s_default:
        nextsym();
        checkfor_ket(s_colon);
        if ((c = adddefault(fl)) == 0) return rd_command(); /* error */
        cmd1c_(c) = rd_command();
        return c;
case s_case:
        nextsym();
        e = optimise0(mkcast(s_case, rd_expr(PASTCOMMA), cur_switchtype));
        checkfor_ket(s_colon);
        if (e==0 || (c = addcase(e,fl)) == 0) return rd_command(); /* error */
        cmd2c_(c) = rd_command();
        return c;
default:
        if (!isexprstarter(op))
        {   syn_err(syn_err_expected_cmd, op);
            while (curlex.sym!=s_lbrace && curlex.sym!=s_rbrace &&
                   curlex.sym!=s_semicolon && curlex.sym!=s_eof) nextsym();
            return 0;
        }
        e = rd_expr(POSSLABEL);
        if (h0_(e) == s_colon)
        {   Symstr *a = (Symstr *)type_(e);   /* also in curlex.a1.sv */
            LabBind *lab = label_define(a);
            nextsym();                      /* not previously done */
            if (lab == 0) return rd_command();    /* duplicate     */
            return mk_cmd_lab(s_colon, fl, lab, rd_command());
        }
        /* @@@ perhaps we should check the ';' first */
        e = optimise0(mkcast(s_semicolon, e, te_void));
        c = (e==0) ? 0 : mk_cmd_e(s_semicolon, fl, e);
        break;
case s_semicolon:
        c = 0;
        break;
case s_lbrace:
        c = rd_block(1);
        elselast = 0;
        return c;
case s_do:
        nextsym();
        {   Cmd *oldloop = cur_loop; AEop oldbreak = cur_break;
            cur_loop = c = mk_cmd_do(fl, 0, 0);   /* amalgamate with for */
            cur_break = s_break;
            cmd1c_(c) = rd_command();
            cur_loop = oldloop; cur_break = oldbreak;
        }
        if (curlex.sym == s_nothing) nextsym();
        if (curlex.sym == s_while)
        {   nextsym();
            e = optimise0(rd_condition(s_while));
        }
        else
        {   syn_err(syn_err_expected_while);
            e = 0;      /* treat like "do c while;" (missing e).      */
        }
        if (e == 0) e = mkintconst(te_int,0,0); /* error => while(0). */
        cmd2e_(c) = e;
        break;
case s_while:
        nextsym();
        e = optimise0(rd_condition(op));
        {   Cmd *oldloop = cur_loop; AEop oldbreak = cur_break;
            cur_loop = c = mk_cmd_for(fl, 0, e, 0, 0);
            cur_break = s_break;
            cmd4c_(c) = rd_command();
            cur_loop = oldloop; cur_break = oldbreak;
            return c;
        }
case s_for:
        nextsym();
        checkfor_ket(s_lpar);
        {   Expr *e2, *e3;
            e = ((curlex.sym == s_semicolon) ? 0 :
                    optimise0(mkcast(s_for,rd_expr(PASTCOMMA),te_void)));
            checkfor_ket(s_semicolon);
            e2 = ((curlex.sym == s_semicolon) ? 0 :
                    optimise0(mktest(s_for, rd_expr(PASTCOMMA))));
            checkfor_ket(s_semicolon);
            e3 = ((curlex.sym == s_rpar) ? 0 :
                    optimise0(mkcast(s_for,rd_expr(PASTCOMMA),te_void)));
            checkfor_ket(s_rpar);
            {   Cmd *oldloop = cur_loop; AEop oldbreak = cur_break;
                cur_loop = c = mk_cmd_for(fl, e,e2,e3,0);
                cur_break = s_break;
                cmd4c_(c) = rd_command();
                cur_loop = oldloop; cur_break = oldbreak;
            }
            return c;
        }
case s_if:
        nextsym();
        e = optimise0(rd_condition(op));
        if (e == 0) e = mkintconst(te_int,0,0); /* error => if(0).    */
        c = rd_command();
        if (curlex.sym == s_nothing) nextsym();
        {   Cmd *c2;
            static int32 elseline = 0;
            if (curlex.sym == s_else)
            {   elseline = pp_linect;
                nextsym();
                c2 = rd_command();
                elselast = 1;
            }
            else
            {   c2 = 0;
                if (elselast) /* in 'c' above */
                {   int32 temp = pp_linect;
                    pp_linect = elseline;
/* The construction applied here seems a little bit nasty - it would    */
/* certainly give trouble if diagnostics were displayed with an echo of */
/* part of the surrounding source. Still, it seems a good idea!         */
                    syn_warn(syn_warn_dangling_else);
                    pp_linect = temp;
                }
            }
            return mk_cmd_if(fl, e, c, c2);
        }
case s_else:
        syn_err(syn_err_else);
        nextsym();
        return rd_command();
case s_switch:
        nextsym();
        e = rd_condition(op);
        {   Cmd *oldswitch = cur_switch; AEop oldbreak = cur_break;
            TypeExpr *oldswitchtype = cur_switchtype;
            Expr *ee = optimise0(e);
            if (ee==0) e = ee = mkintconst(te_int,0,0); /* error=>switch(0). */
            cur_switch = c = mk_cmd_switch(fl, ee,0,0,0);
            cur_break = s_endcase;
            cur_switchtype = typeofexpr(e);             /* not optimise0(e)! */
            cmd2c_(c) = rd_command();
            cur_switch = oldswitch; cur_break = oldbreak;
            cur_switchtype = oldswitchtype;
            return c;
        }
case s_return:
        nextsym();
#ifdef INMOSC
        /* The following code is natural, but some versions of optimise */
        /* like having 'return' as an AE *expression*.  See below.      */
        e = ((curlex.sym == s_semicolon) ? 0 : optimise0(
                mkcast(s_return, rd_expr(PASTCOMMA), cur_restype)));
#else
        e = ((curlex.sym == s_semicolon) ? 0 : (
                mkcast(s_return, rd_expr(PASTCOMMA), cur_restype)));
        e = (e == 0 || h0_(e) == s_error) ? 0 :
            optimise0(mk_expr1(s_return, cur_restype, e));
#endif
        if (e != 0) implicit_return_ok = 0;
        if (e != 0 && equivtype(cur_restype, te_void))
            syn_rerr(syn_rerr_return);
        if (e == 0 && !implicit_return_ok && !equivtype(cur_restype, te_void))
            syn_warn(syn_warn_void_return);
        c = mk_cmd_e(op, fl, e);
        break;
#ifdef EXTENSION_VALOF
case s_resultis:
        nextsym();
        e = optimise0(
               mkcast(s_return, rd_expr(PASTCOMMA), valof_block_result_type));
        c = mk_cmd_e(op, fl, e);
        break;
#endif
case s_continue:
        if (cur_loop != 0) c = mk_cmd_0(op, fl);  /* syn_list2(op,cur_loop)? */
        else { c=0; syn_err(syn_err_continue); }
        nextsym();
        break;
case s_break:
        if (cur_loop != 0 || cur_switch != 0) c = mk_cmd_0(cur_break, fl);
        else { c=0; syn_err(syn_err_break); }
        nextsym();
        break;
case s_goto:
        nextsym();
        if (curlex.sym != s_identifier)
        {   syn_err(syn_err_no_label);
            return rd_command();
        }
        /* the 0 in the next command is never used */
        c = mk_cmd_lab(op, fl, label_reference(curlex.a1.sv), 0);
        nextsym();
        break;
    }
    elselast = 0;
    checkfor_delimiter_ket(s_semicolon, errname_aftercommand);
    return c;
}

static Cmd *rd_body(TypeExpr *t)
{   cur_switch = 0;
    cur_loop = 0;
    cur_break = s_error;
    cur_restype = t;
    cur_switchtype = te_lint;
    if (curlex.sym == s_lbrace)
        return rd_block(0);
    else
    {   syn_err(syn_err_no_brace);
        return 0;
    }
}

/* rd_declspec() reads a possibly optional list (as controlled by       */
/* 'declflag') of declaration-specifiers.  (Dec 88 ANSI draft section   */
/* 3.5).   It returns an object of TypeSpec (subtype of TypeExpr)       */
/* giving a bit map of types/storage classes read (plus Tagbinder for   */
/* struct/union or Binder for 'typedef') plus (local) B_OMITTYPE if the */
/* type info was omitted -- consider extern fn definition syntax.       */
/* It also performs normalisation of 'float' to 'short double',         */
/* 'register' to 'auto register' to simplify later stages.              */
/* Note that defaulting of storage class depends not only on context,   */
/* but also on type - e.g. { int f(); ...} so it is done later.         */
static TypeSpec *rd_declspec(int declflag)
{
    SET_BITMAP illtype = 0, typesseen = 0, typedefquals = 0;
    Binder *b = 0;  /* typedef or struct/union/enum tag binder record */
    SET_BITMAP illstg, stgseen = 0;
    if (declflag & POINTERTYPES)
        illtype = TYPEDEFINHIBITORS;   /* leaves just const/volatile */
    illstg = declflag & TOPLEVEL ? (bitofstg_(s_register)|bitofstg_(s_auto)) :
             declflag & BLOCKHEAD ? (SET_BITMAP)0 :
             declflag & (FORMAL|ARG_TYPES) ? PRINCSTGBITS : STGBITS;
    for (;;)
    {   AEop s = curlex.sym;
        if (s == s_identifier && !(illtype & TYPEDEFINHIBITORS)
                              && istypedefname_(curlex.a1.sv))
            s = s_typedefname, b = symdata_(curlex.a1.sv),
            typedefquals = qualifiersoftype(bindtype_(b)),
            binduses_(b) |= u_referenced;
        else if (!isdeclstarter_(s)) break;
/* merge the next if-then-else sometime via table driven code. */
        if (isstorageclass_(s))
        {   if (bitofstg_(s) & illstg)
                syn_err(syn_err_stgclass, /* storage class illegal here */
                        s, ctxtofdeclflag(declflag));
            else if (stgseen & STGBITS)
                syn_err(syn_err_stgclass1, /* storage class incompatible */
                         s, stgseen);
            else
                stgseen |= bitofstg_(s);
            nextsym();
        }
        else
        {   SET_BITMAP typebit = bitoftype_(s);
            bool trouble = (illtype & typebit) != 0;
            if (trouble)
                 /* this code relies that for type symbols x,y, "x y" is
                    legal iff "y x" is. */
                 syn_err(syn_err_typeclash, s,
                         typesseen & illtypecombination[shiftoftype_(s)]);
            else
            {
#ifdef TARGET_IS_ARM
                if (s == s_short && !(suppress & D_SHORTWARN))
                {   syn_warn(syn_warn_use_of_short);
                    suppress |= D_SHORTWARN;
                }
#endif
                typesseen = typesseen | typebit,
                illtype |= illtypecombination[shiftoftype_(s)];
            }
            nextsym();
            if (typebit & (bitoftype_(s_union)|bitoftype_(s_struct)
                                              |bitoftype_(s_enum)))
            {   TagBinder *b2;
                bool sawid = 0;
                /* Now, after "struct id" a ';' or '{' indicates a new
                   definition at the current nesting level */
                if (curlex.sym == s_identifier)
                {   Symstr *sv = curlex.a1.sv;
                    bool defining;
                    sawid = 1;
                    nextsym();
                    defining = (curlex.sym == s_semicolon ||
                                curlex.sym == s_lbrace);
                    b2 = findtagbinding(sv,s,defining);
                    if (curlex.sym == s_semicolon)
                        tagbindstate_(b2) &= ~TB_BEINGDEFD;
                    if (defining)
                        typesseen |= B_DECLMADE;  /* police 'int;' error */
                }
                else b2 = gentagbinding(s);    /* anonymous */
                if (curlex.sym == s_lbrace)
                {   nextsym();
                    if (s==s_enum)
                      { typesseen |= B_DECLMADE; /* else other syntax error */

                        if (!(suppress & D_ENUM))
                          { syn_warn(syn_warn_enum_unchecked);
                            suppress |= D_ENUM;
                          }
                      }

                    settagmems(b2, (s==s_enum ?
/* beware the next line.  We forge a type for recovery in the case
   of "short enum {...}".  However, this means that possible
   const/volatile are not put on enumeration constants.  This
   doesn't really matter since constants are constant anyway!!!
*/
                        rd_enumdecl(mk_typeexpr1(s_typespec,(VoidStar)typebit,
                                                 (VoidStar)b2))
                      : rd_strdecl(b2)));
                    checkfor_ket(s_rbrace);
                }
                else if (!sawid)
                {   syn_err(syn_err_tag_brace, s);
                    /* recovers for 'struct *a' or suchlike error. */
                }
                if (!trouble) b = (Binder *)b2;
            }
        }
    }
    if (typedefquals & typesseen)
        syn_rerr(syn_rerr_qualified_typedef(b, typedefquals & typesseen));
    if ((declflag & TYPE_NEEDED) && typesseen == 0)
        syn_rerr(syn_rerr_missing_type);
    else if (!(declflag & FORMAL) && (typesseen|stgseen)==0)
        typesseen |= B_OMITTYPE; /* see rd_declrhslist for why we need this */

    if ((typesseen & NONQUALTYPEBITS) == 0)
    {   if ((typesseen|stgseen) != 0 || !(declflag & FORMAL))
            /* consider returning 0 for untyped formals?
               changes would be need for several routines, so pend */
            typesseen |= bitoftype_(s_int);
    }

    if (typesseen & bitoftype_(s_float))    /* normalise for rest of system */
      { if (typesseen & bitoftype_(s_long) && !(suppress & D_LONGFLOAT)
                  && !(feature & FEATURE_PCC))
             { suppress |= D_LONGFLOAT;
               syn_rerr(syn_rerr_long_float);
             }

        typesseen = (typesseen & ~bitoftype_(s_float)) | bitoftype_(s_double);
        typesseen = (typesseen & bitoftype_(s_long)) ?
            (typesseen & ~bitoftype_(s_long)) :
                (typesseen | bitoftype_(s_short));
       }
    if (stgseen & bitofstg_(s_register))   /* normalise for rest of system */
        stgseen |= bitofstg_(s_auto);
    return mk_typeexpr1(s_typespec,(VoidStar)(stgseen|typesseen), (VoidStar)b);
}

/* the following macro checks for an arg which has not been typed */
#define is_untyped_(x) (h0_(x) == s_typespec && typespecmap_(x) == 0)

typedef TypeExpr Declarator;   /* funny inside-out Expr */
#define sub_declarator_(x) ((x)->typearg)

/* Declarators: read these backwards (inside out) to yield a name
 * (omitted if abstract declarator) and a TypeExpression (a la Algol68).
 * The the declarator AE structure is read and an in-place pointer reverse
 * is done to turn a 'basictype' (possibly typedef) and a declarator into
 * a declaree (identifier or empty) in 'declarator_name' and TypeExpr.
 */

static Declarator *rd_formals_1(Declarator *a);
/* AM: next edit the following becomes a TypeFnAux* arg to rd_formals. */
static int32 syn_minformals, syn_maxformals;
static bool syn_oldeformals;
                       /* set by rd_formals, read by rd_declarator_1() */
static Symstr *declarator_name;
                       /* set by rd_declarator: 0 or Symstr *          */

/* rd_declarator reads a C declarator (q.v.).  'abs_or_conc' is now     */
/* functionally a context (like declflag (q.v.)).  We test it to see    */
/* whether CONC_DECLARATOR or ABS_DECLARATOR's (or both) are            */
/* acceptable, also TOPLEVEL (which allows old 'f(a,b)' non-prototype   */
/* form).                                                               */
static Declarator *rd_declarator_1(int abs_or_conc)
{
    static Declarator emptydeclarator = { s_nothing };
    Declarator *a;
    switch (curlex.sym)
    {
default:
        if (abs_or_conc & CONC_DECLARATOR)
        {   syn_err(syn_err_expected3);
            if (curlex.sym == s_rbrace) nextsym();
            return (Declarator *)errornode;
        }
        a = &emptydeclarator;
        break;
case s_identifier:
        if (abs_or_conc & ABS_DECLARATOR)
        {   syn_err(syn_err_unneeded_id, symname_(curlex.a1.sv));
            a = &emptydeclarator;
        }
        else a = (Declarator *)curlex.a1.sv; /* curlex.a1.sv is a Symstr with s_id */
        nextsym();
        break;
case s_lpar:
/* Note that "int ()" means an abstract declarator (or nameless formal) */
/* of the type of "int x()", not "int (x)".                             */
/* Similarly "int (void)" or "int (typedefname)" represent fn-typenames */
/* @@@ However, the ANSI draft (Dec 88) has an ambiguity in             */
/* "typedef int t; void f(int (t));" as to whether the formal to f is   */
/* of type "int" and name "t" or nameless and of type "int (int)".      */
/* We choose to select the latter (this seems to be the ANSI ctte's     */
/* intent from the example "t f(t (t))" in section 3.5.6 (dec 88)).     */
        nextsym();
        if ((abs_or_conc & (ABS_DECLARATOR|FORMAL)) &&
            (curlex.sym == s_rpar || isdeclstarter_(curlex.sym) ||
              (curlex.sym == s_identifier && istypedefname_(curlex.a1.sv))))
/* @@@ Add conjunct && !(abs_or_conc & FORMAL) to istypedefname_() for  */
/* the other interpretation of ANSI draft ambiguity.                    */
        {   a = rd_formals_1(&emptydeclarator);
        }
        else
        {   a = rd_declarator_1(abs_or_conc);
            if (h0_(a) == s_error) return a;
        }
        checkfor_ket(s_rpar);
        break;
case s_times:
        {   TypeSpec *p;
            nextsym();
            p = rd_declspec(POINTERTYPES);
            a = rd_declarator_1(abs_or_conc);
            if (h0_(a) == s_error) return a;
            /* the next line has to mask off b_synbits & bitoftype_(s_int) */
            a = mk_typeexpr1(t_content, a, (VoidStar)(typespecmap_(p) &
                         (bitoftype_(s_const) | bitoftype_(s_volatile))));
            break;
        }
    }
    for (;;) switch (curlex.sym)
    {
case s_lpar:
            nextsym();
            a = rd_formals_1(a);
            checkfor_ket(s_rpar);
            break;
case s_lbracket:
            nextsym();
            a = mk_typeexpr1(t_subscript, a,
                             ((curlex.sym==s_rbracket) ? 0 :
                                 check_arraysize(rd_expr(PASTCOMMA))));
            checkfor_ket(s_rbracket);
            break;
default:    return a;
    }
}

static TypeExpr *fault_incomplete_type_object(TypeExpr *tt, Symstr *name,
                                              bool member, SET_BITMAP stg)
{   /* Fault attempts to declare an object (or member) of               */
    /* 'incomplete type' or function type.  Members are constraint      */
    /* violations, objects are unclear, maybe just undefined.           */
    /* Note particularly, that ncc(pre-3.32) and PCC allow              */
    /*   "extern struct a x;" where 'struct a' is not defined.          */
    /* Note also that member <==> (stg & PRINCSTGBITS) == 0.            */
    TypeExpr *t = princtype(tt);
    if (h0_(t) == s_typespec)
    {   SET_BITMAP m = typespecmap_(t);
        if (m & (bitoftype_(s_struct)|bitoftype_(s_union)))
        {   TagBinder *b = typespectagbind_(t);
            /* Much of the compiler can support undefined structs until */
            /* first essential use, but ANSI ban.                       */
            if (tagbindstate_(b) & TB_BEINGDEFD)
            {   syn_err(syn_err_selfdef_struct(member,b,name));
                goto fixup;
            }
            if (tagbindmems_(b) == 0 && !(stg & bitoftype_(s_extern)))
            {   syn_err(syn_err_undef_struct(member,b,name));
                goto fixup;
            }
        }
        if (m & bitoftype_(s_void))
        {   syn_err(syn_err_void_object(member,name));
            goto fixup;
        }
    }
    /* @@@ pick up more [] cases (like auto, but not extern) below?     */
    if (h0_(t) == t_subscript && typesubsize_(t) == 0 && member)
    {   cc_pccwarn(syn_rerr_open_member, name);
        /* ANSI ban open array members, pcc treats as [0].              */
        typesubsize_(t) = globalize_int(0);
    }
    if (h0_(t) == t_fnap &&
           (member || !(stg & (bitofstg_(s_extern)|bitofstg_(s_static)))))
    {   /* Beware: next line was syn_pccwarn, but AM sees PCC error.    */
        syn_rerr(syn_rerr_fn_ptr(member,name));
        /* Check non-typedef formals have types before fixup.           */
        if (h0_(tt) == t_fnap) ensure_formals_typed(typefnargs1_(tt), 1);
        goto fixup;
    }
    return tt;
fixup:
    return ptrtotype_(tt);
}


/* @@@ AM Jan 90: The two calls to fault_incomplete_formals() make      */
/* me nervous: it is ANSI illegal to have a fn prototype DEFINITION     */
/* with an incomplete type (e.g. void or 'struct t' where tag t is not  */
/* yet defined), but in a DECLARATION this is merely ANSI undefined.    */
/* Now, to avoid syserr()s (and to be helpful) in later calls to        */
/* functions with void parameters it is necessary to fault them at the  */
/* time of declaration.  The line below has the effect of faulting      */
/*          "struct a; extern void f(struct a);" too,                   */
/* (but of course not "extern void f(struct a *);").                    */
/* Unless anyone complains I intend to treat this undefined as error.   */
/* If you wish to re-allow the above example this check should be       */
/* inhibited unless isprimtype_(princtype(p->decltype),s_void).         */
/* This only concerns FORMAL's (prototype) and so is PCC-irrelevant.    */
static void fault_incomplete_formals(DeclRhsList *p)
{   for (; p != NULL; p = p->declcdr)
        p->decltype = fault_incomplete_type_object(
                        p->decltype, p->declname, 0, p->declstg);
}

static TypeExpr *fault_odd_fn_array(TypeExpr *t)
{   /* Precondition: t is a t_fnap or t_subscript node.                 */
    /* Return t if OK, else a fixed up version -- we are sole owners.   */
/* ANSI (Oct88) oddity: function returning fn/array is a constraint     */
/* violation, whereas array of non-object type (void/fn/undef struct)   */
/* is done by weasel words (p28).  Treat similarly.                     */
    TypeExpr *a = typearg_(t), *pa = princtype(a);  /* maybe typedef!   */
/* fault function returning fn or array                                 */
    if (h0_(t) == t_fnap &&
         (h0_(pa) == t_subscript || h0_(pa) == t_fnap))
    {   /* fn returning array/fn => fn returning ptr to array elt/fn.   */
        cc_rerr(syn_rerr_fn_returntype, pa);
        if (h0_(pa) == t_subscript) a = typearg_(pa);
        goto addptr;
    }
/* fault arrays of fn, void, @@@???undef structs, @@@???[] arrays.      */
    if (h0_(t) == t_subscript &&
         (h0_(pa) == t_fnap || isprimtype_(pa, s_void)))
    {   /* array of fn/void => array of ptrs to fn/void.                */
        cc_rerr(syn_rerr_array_elttype, pa);
        goto addptr;
    }
    return t;
addptr:
    typearg_(t) = mk_typeexpr1(t_content, (VoidStar)a, (VoidStar)0);
    return t;
}

/* rd_declarator() returns 0 in the event that it failed to find a
   declarator.  Note that this can only happen if 'CONC_DECLARATOR' is set.
   The caller is now responsible for faulting declarations of 'incomplete
   types' (see ansi).
*/
static TypeExpr *rd_declarator(int abs_or_conc, TypeExpr *basictype)
{
    Declarator *x = rd_declarator_1(abs_or_conc);
    for (;;) switch (h0_(x))
    {   case s_error: return 0;
        default: syserr(syserr_rd_declarator, (long)h0_(x));
            /* drop through */
        case s_nothing:
            declarator_name = 0;
            return basictype;
        case s_identifier:
            declarator_name = (Symstr *)x;
            return basictype;
        case t_fnap:
            {   Declarator *y = sub_declarator_(x);
                if (!(h0_(y) == s_identifier && (abs_or_conc & TOPLEVEL)))
                    /* all cases except outermost () of TOPLEVEL declarator */
                    ensure_formals_typed(typefnargs1_((TypeExpr *)x), 1);
            }
            /* drop through */
        case t_content:
        case t_subscript:
            {   Declarator *temp = sub_declarator_(x);
                if (is_untyped_(basictype))  /* e.g. f(int a,*b) */
                {   syn_rerr(syn_rerr_missing_type1);
                    basictype = te_int;
                }
                sub_declarator_(x) = (Declarator *)basictype;
                basictype = fault_odd_fn_array((TypeExpr *)x);
                x = temp;
                break;
            }
    }
}

static TypeExpr *rd_typename(void)
{
    TypeExpr *t = rd_declspec(ABS_DECLARATOR);
                             /*  TYPE_NEEDED and ~STGCLASS_OK       */
    typespecmap_(t) &= ~b_synbits;   /* allow empty declarations */
    return rd_declarator(ABS_DECLARATOR, t);
    /* Ignore the value in declarator_name as abstract declarator.      */
    /* Incomplete types are valid here (or faulted later (e.g. sizeof)) */
}

/* note that in general we cannot default storageclasses on parsing them
   due to the differing default classes in "{ int a,f();...}".
   On the other hand nonsensical combinations SHOULD be detected by
   rd_typename().
*/
static void defaultstorageclass(DeclRhsList *d, int declflag)
{
    if ((declflag & STGCLASS_OK) && (d->declstg & PRINCSTGBITS) == 0)
    {   if (declflag & BLOCKHEAD)
            d->declstg |= isfntype(d->decltype) ? bitofstg_(s_extern)
                                                : bitofstg_(s_auto);
        else if (declflag & TOPLEVEL)
            d->declstg |= isfntype(d->decltype) ? bitofstg_(s_extern)
                                        : bitofstg_(s_extern)|b_omitextern;
        else if (declflag & (FORMAL|ARG_TYPES))
            d->declstg |= bitofstg_(s_auto);
        else syserr(syserr_defaultstgclass, (int)declflag);
    }
}

static void ensure_formals_typed(DeclRhsList *d, bool proto)
{   /* proto is here true if olde-style is not acceptable:              */
    /* e.g. non-top level.                                              */
    for (; d; d = d->declcdr)
    {   TypeExpr *t = d->decltype;
        if (is_untyped_(t))
        {   if (proto)
                /* @@@ ensure that f(,) error has not got this far */
                syn_rerr(syn_rerr_missing_type2, d->declname);
            else
              if (!(feature & FEATURE_PCC))
                  /* God knows why ANSI do not consider this an error */
                  syn_warn(syn_warn_undeclared_parm, d->declname);

            d->decltype = te_int;
        }
    }
}

/* (local to rd_formals).  merge this fn with previous?                 */
static bool is_proto_arglist(DeclRhsList *d, int map)
{   /* map is 1 if ellipsis seen, else 0.  Error if ansi/old-style mix  */
    for (; d; d = d->declcdr)
    {   TypeExpr *t = d->decltype;
        map |= (is_untyped_(t)) ? 2 : 1;
    }
    if (map == 3) cc_rerr(syn_rerr_mixed_formals);
    return map & 1;     /* note that () is considered empty id-list.    */
}

static void merge_formal_decl(DeclRhsList *d, DeclRhsList *dnew)
{
/* Here I have a formal parameter (d) which is now being given an        */
/* explicit declaration (dnew).  Merge new information in with the old.  */
    TypeExpr *t = d->decltype;
    if (!is_untyped_(t))  /* previous type info */
        syn_err(syn_err_duplicate_type, d->declname);
    d->declstg = dnew->declstg;
    d->decltype = dnew->decltype;
}

/* but for its size rd_declrhslist() would be part of rd_decl()
   maybe it will become so anyway!  It reads any of K&R's
  "(last part of undefined category)type-decl-list(see function-body)",
  "init-declarator-list" or "struct-declarator-list"
*/

static bool topfnflag;  /* for use of rd_declrhslist() and rd_decl() ONLY! */
static DeclRhsList *syn_formals;  /* ditto - now also rd_fndef() */

static DeclRhsList *rd_declrhslist(const SET_BITMAP ss, TypeSpec *const tt,
                                   const int declflag)
{   DeclRhsList *p,*q;
    /* note - do NOT change ss, tt or declflag since used in loop below */
    for (p = q = 0; ;)                       /* 3 exits - all 'return's */
    {   DeclRhsList *temp = mkDeclRhsList(0, 0, 0, 0, ss, 0);
        FileLine fl; fl.f = pp_cisname, fl.l = pp_linect;
        fl.p = dbg_notefileline(fl);
        if ((declflag & LENGTH_OK) && curlex.sym == s_colon)
            temp->decltype = tt;             /* stash away type, 0 declaree */
        else
        {   if ((temp->decltype = rd_declarator(declflag,tt)) == 0)
            {   /* error in rd_declarator already reported */
                if (declflag & TOPLEVEL)        /* @@@ in flux */
                    bind_level=0, pop_varenv(0), pop_tagenv(0);
                while (curlex.sym!=s_lbrace &&
                       (declflag & TOPLEVEL || curlex.sym != s_rbrace) &&
                       curlex.sym!=s_semicolon && curlex.sym!=s_eof)
                    nextsym();
                topfnflag = 0;
                return p;       /* return decls so far to recover */
            }
            temp->declname = declarator_name;          /* 2nd result */
        }
        if (declflag & (FORMAL|ARG_TYPES))
            temp->decltype = modify_formaltype(temp->decltype);
            /* transform f() -> (*f)() and a[] -> *a    */
            /* these get seen by all - even the CG.     */
        defaultstorageclass(temp,declflag);
/* AM: because f(void) is OK, but not f(void, int), checks on the       */
/* use of 'void' in parameter lists are postponed to rd_formals().      */
/* Other incomplete types (like struct t) are done there too.           */
/* However, it might be nice for early reporting to fault some cases    */
/* here (e.g. named void parameters).  See fault_imcomplete_formals().  */
        if (!(declflag & FORMAL) &&
            !(temp->declstg & bitofstg_(s_typedef)))
            temp->decltype = fault_incomplete_type_object(
                 temp->decltype, temp->declname,
                 (declflag & LENGTH_OK) != 0, temp->declstg);
        if (h0_(temp->decltype) == t_fnap)       /* but NOT via typedef */
        {   /* see if the fn declaration is a fn definition, we have
               already (in rd_formals()) changed f(void) to f()         */
            TypeExpr *fntype = temp->decltype;
            DeclRhsList *fnpars = typefnargs1_(fntype);
            if ((declflag & TOPLEVEL) &&      /* top level */
                p == 0 &&                     /* first in list */
                !(temp->declstg & bitofstg_(s_typedef)) &&
                (  isdeclstarter_(curlex.sym) ||  /* body or param starter */
                   (curlex.sym == s_identifier && istypedefname_(curlex.a1.sv))
                 || curlex.sym == s_lbrace))
            {   if (curlex.sym != s_lbrace)
                {   if (feature & FEATURE_WARNOLDFNS)
                        syn_warn(syn_warn_old_style, temp->declname);
                }
                else
                {   if ((feature & (FEATURE_PCC|FEATURE_FUSSY)) ==
                                   (FEATURE_PCC|FEATURE_FUSSY))
                        /* @@@ The next line considers f(){} ANSI only! */
                        syn_warn(syn_warn_ANSI_decl, symname_(temp->declname));
                }
                if (fnpars == 0)
                {   maxargs_(fntype) = 0;
                    if (!(feature & FEATURE_PCC))
                        /* treat f() {} as f(void) {} in ANSI mode... */
                        typefnaux_(fntype).oldstyle = 0;
                }
                temp->declstg |= b_fnconst;
                topfnflag = 1;
                return temp;
            }
            /* The ANSI draft disallows 'extern f(a,b);' (types needed) */
            ensure_formals_typed(fnpars, 1);
        }
        if (isfntype(temp->decltype))           /* possibly via typedef */
        {   if (!(temp->declstg & (bitofstg_(s_extern)|
                     bitofstg_(s_typedef)|bitofstg_(s_static))))
                syserr(syserr_rd_declrhslist);
            else
            {   temp->declstg |= b_fnconst|b_undef;
                if (maxargs_(princtype(temp->decltype)) == 999)
                { if (warn_deprecated)
                    /* The follow warning enables us to root out        */
                    /* insecurities/errors of the form:                 */
                    /* extern f(); g() { f(1); } f(int x,int y) {...}   */
                    syn_warn(syn_warn_give_args, symname_(temp->declname));
                  else xwarncount++;
                }
            }
        }
        if (declflag & ARG_TYPES)     /* special code to save up info */
        {   DeclRhsList *p;           /* instate_decl is called later */
            Symstr *sv = temp->declname;
            for (p = syn_formals; p != 0; p = p->declcdr)
                if (sv == p->declname)
                {   merge_formal_decl(p,temp);  /* update syn_formals */
                    break;
                }
            if (p==0) syn_err(syn_err_not_a_formal, sv);
        }
        if (declflag & (TOPLEVEL|BLOCKHEAD))    /* was INIT_OK */
        {   int initflag = 0;
            if (temp->declstg & (bitofstg_(s_extern) | bitofstg_(s_static)))
                temp->declstg |= b_undef;
            if (curlex.sym == s_assign || curlex.sym == s_lbrace)
            /*
             * Extern init only at top level only.  Static init at all
             * levels within a program
             */
            {   if (((temp->declstg & bitofstg_(s_extern)) &&
                     (declflag & TOPLEVEL)) ||
                    (temp->declstg & bitofstg_(s_static)))
                    temp->declstg &= ~b_undef;
                if (temp->declstg & b_fnconst)
                {   syn_rerr(syn_rerr_fn_ptr1, temp->declname);
                    temp->decltype = ptrtotype_(temp->decltype);
                    temp->declstg &= ~(b_fnconst | b_undef);
                }
            }
            /* do the next line AFTER above type patching but before
               reading possible initialiser.  Save Binder in ->declbind.
               d->declstg now always has b_undef for statics & externs if there
               is no initialiser to read. instate_declaration removes for local
               statics not going in bss.
             */
            temp->declbind = instate_declaration(temp, declflag);
            if (curlex.sym == s_assign || curlex.sym == s_lbrace)
            {   if (temp->declstg & (bitofstg_(s_auto)|bitofstg_(s_static)) ||
                     (temp->declstg & (bitofstg_(s_extern)|b_undef)) ==
                                      (bitofstg_(s_extern)))
                    initflag = 1;
                else
                    syn_err(syn_err_cant_init, temp->declstg),
                    initflag = 2;

                if( curlex.sym == s_lbrace )
                    syn_pccwarn(syn_rerr_archaic_init);
                else
                    nextsym();
            }
            if (initflag == 2) (void)syn_rdinit(0,4);
            syn_initdepth = (initflag == 1) ? 0 : -1;
            syn_undohack = 0;
            temp->declinit =
                /* the != 0 on the next line is not spurious if int!=long */
                genstaticparts(temp, (declflag & TOPLEVEL) != 0, fl);
            /* The positioning of the next line is subject to some debate */
            /* -- I put it here so that the line number is exact, but     */
            /* note that (a) TOPLEVEL vars are done in vargen.c,          */
            /* (b) LOCAL vars BLOCKHEAD|FORMAL are not yet complete in    */
            /* that we have not yet decided on their eventual storage;    */
            /* this is done by dbg_scope().  See other dbg_locvar() call. */
            if (usrdbg(DBG_VAR) && (declflag & BLOCKHEAD))
            {
#ifdef TARGET_IS_C40
	      dbg_locvar(temp->declbind, fl, NO);
#else
	      dbg_locvar(temp->declbind, fl);
#endif
                /*
                 * Field fileline in DeclRhsList added by RCC 25-Mar-88
                 * so that debugger gets a line no for initialising code.
                 */
                temp->fileline = fl;
            }
        }
        /* do not instate formals (or in casts "(int ()(int a,b))foo")
           nor structure elements. */
        if (declflag & LENGTH_OK && curlex.sym == s_colon)
        {       TypeExpr *t = prunetype(temp->decltype);
                nextsym();
                /* ->declinit also holds the bit size expr temporarily */
                temp->declinit = check_bitsize(
                            rd_expr(UPTOCOMMA), t, temp->declname);
                temp->decltype = check_bittype(t);
        }
/* The next line loses all local vars/tags at the end of each TOPLEVEL  */
/* declarator.  This is still not quite right, but enables vars/tags    */
/* to be local to (toplevel) parameter lists.  @@@ More fixing required */
/* for parameter lists within parameter lists.                          */
        if (declflag & TOPLEVEL)        /* @@@ in flux */
            bind_level=0, pop_varenv(0), pop_tagenv(0);
        if (p == 0) p = q = temp;
        else { q->declcdr = temp; q = temp; }
        if ((declflag & FORMAL) || curlex.sym != s_comma)
        {   topfnflag = 0;
            return p;
        }
        nextsym();
    }
}

static TopDecl *rd_fndef(DeclRhsList *d, int declflag)
/* d has exactly 1 item */
{   TypeExpr *fntype = d->decltype;        /* known to be fn type */
    Binder *fnbind; Cmd *body;
    DeclRhsList *fpdecllist = typefnargs1_(fntype), *fpe;
    SynBindList *lambdap = 0, *lambdaq = 0;
    SynBindList *narrowformals = 0; Expr *arginit = 0;
    /* syn_formals is examined by rd_declrhslist if ARG_TYPE is set */
    syn_formals = fpdecllist;
    currentfunction = symname_(d->declname);
    if (debugging(DEBUG_FNAMES))
        cc_msg("Start of function %s\n", currentfunction);
/* when ARG_TYPES is set in declflag any identifiers declared will have  */
/* been seen before (as formal parameters) and rd_decllist works by      */
/* updating these existing declaration records in a side-effect-full way */
    bind_level = 1;             /* local scope for any structs!          */
    (void) rd_decllist(ARG_TYPES);
/* do some checking and defaulting on the arguments and types BEFORE
   instate_decl ... */
    for (fpe = fpdecllist; fpe != 0; fpe = fpe->declcdr)
    {   if (fpe->declname == 0)
        {   syn_rerr(syn_rerr_missing_formal);
            /* fixup so we can continue ... */
            fpe->declname = gensymval(1);
        }
    }
    ensure_formals_typed(fpdecllist, 0);
/* do the declaration of the function - this will copy the types just
 * updated in fpdecllist as part of globalize_ing the type of d.
 */
  { FileLine fl; fl.f = pp_cisname, fl.l = pp_linect;
    fl.p = dbg_notefileline(fl);
    fnbind = instate_declaration(d, declflag);
#ifdef TARGET_HAS_DEBUGGER
    if (usrdbg(DBG_PROC))
        dbg_proc(bindsym_(fnbind), bindtype_(fnbind),
                 (bindstg_(fnbind) & bitofstg_(s_extern)) != 0, fl);
#endif
/* Now, instate_declaration above globalize_d the d->decltype field (since
 * all fns are TOPLEVEL beasties).  This leaves us free to construct a
 * BindList of instated formal parameters from the uncopied (local-store)
 * version fpdecllist above.  Start by instantiating the FORMAL's.
 * This is not so simple as it seems because we have to create new binders
 * (sharing the same name) for any narrow (char, float, short) formals
 * and a widened (int, double) binder too.
 */
    for (fpe = fpdecllist; fpe != 0; fpe = fpe->declcdr)
    {   TypeExpr *at = fpe->decltype, *wt = widen_formaltype(at);
        Binder *ab, *wb;
        if (wt == at)            /* pointer equality is fine here */
            ab = wb = instate_declaration(fpe, FORMAL);
        else
        {   Expr *e;
            DeclRhsList *narrowedformal = mkDeclRhsList(0, fpe->declname, at,
                                  fpe->declinit,   /* should be 0 */
                                  fpe->declstg, 0);
            DeclRhsList *wideformal = mkDeclRhsList(0, fpe->declname, wt,
                                  fpe->declinit,   /* should be 0 */
                                  fpe->declstg, 0);
            /* do the original binder second so it gets seen first */
            wb = instate_declaration(wideformal, FORMAL);
            ab = instate_declaration(narrowedformal, FORMAL|DUPL_OK);
            binduses_(wb) |= u_referenced;
            e = mkassign(s_assign, (Expr *)ab,  /* do the narrowing */
                                   mkcast(s_cast, (Expr *)wb, bindtype_(ab)));
            narrowformals = mkSynBindList(narrowformals, ab);
            arginit = arginit ? mkbinary(s_comma, arginit, e) : e;
        }
        if (!(feature & FEATURE_PREDECLARE))
            /* preclude any whinge about unused fn args */
            binduses_(ab) |= u_referenced;
        if (fpe == fpdecllist) instate_alias(first_arg_sym, wb);
        if (fpe->declcdr == 0) instate_alias(last_arg_sym, wb);
        /* The positioning of the next line is subject to some debate */
        /* note that (a) TOPLEVEL vars are done in vargen.c,          */
        /* (b) LOCAL vars BLOCKHEAD|FORMAL are not yet complete in    */
        /* that we have not yet decided on their eventual storage;    */
        /* this is done by dbg_scope().  See other dbg_locvar() call. */
        /* Further debate whether the user wishes to see the wide     */
        /* (i.e. entry) arg (wb), which is unchanging, or the narrow, */
        /* updatable arg (ab) which is not valid on entry.            */
#ifdef TARGET_HAS_DEBUGGER
        if (usrdbg(DBG_VAR))
        {
#ifdef TARGET_IS_C40
	  /*
	   * This fixes the problem that the debugger is asked to bind both the
	   * wide and narrow versions of this parameter (one to global
	   * scope and one to function local scope), but only the function
	   * local version is being declared to the debugger ...
	   */
	  
	  if (wt != at)
	    {
	      dbg_locvar(wb, fl, NO);
	      dbg_locvar(ab, fl, YES);
	    }
	  else
	    {
	      dbg_locvar(ab, fl, NO);
	    }
#else
	  dbg_locvar(ab, fl);
#endif
        }
#endif
        if (lambdap == 0) lambdap = lambdaq = mkSynBindList(0,wb);
        else lambdaq = lambdaq->bindlistcdr = mkSynBindList(0,wb);
    }
    {   Cmd *argstuff = 0;
        if (narrowformals)
            argstuff = mk_cmd_e(s_semicolon, syn_invented_fl,
                                optimise0(arginit));
   bind_level = 2;                               /* @@@ NASTY - fix soon */
        body = rd_body(typearg_(fntype));
   bind_level = 0;                               /* @@@ NASTY - fix soon */
        if (argstuff)
            body = mk_cmd_block(syn_invented_fl, narrowformals,
                                mkCmdList(mkCmdList(0,body), argstuff));
    }
    label_resolve();
    pop_varenv(0);    /*  see comment at other call */
    pop_tagenv(0);
/* h4_(result) is a flag to indicate the presence of an ellipsis in      */
/* the list of formal arguments for this function.                       */
    return mkTopDeclFnDef(s_fndef, fnbind, lambdap, body,
        /*
         * Beware !!!. In PCC mode all functions are considered as
         * Having a trailing '...'.  This is activated by the
         * code in cg.c which checks whether the address of ANY of the
         * args has been taken.  If so all args go to stack !!!
         */
        (maxargs_(fntype)==1999) || (lambdap!=0) && (feature&FEATURE_PCC));
  }
}


/* rd_decl reads a possibly top-level decl, see also rd_decl2()       */
static TopDecl *rd_decl(int declflag)
/* AM: Structure decls are tantalisingly close to ordinary decls (but no
   storage class) except for length specs replacing initialisation.
   Type specs are compulsory for structure members, but not for
   vars (e.g. auto a=1;).
   Read them all them with a single parameterised routine.
*/
{
    DeclRhsList *d;
    TypeSpec *t = rd_declspec(declflag);
    SET_BITMAP s = typespecmap_(t) & STGBITS;
    int32 declxtramap = typespecmap_(t) & b_synbits;
/* we are required to complain at top level for "*x;", but not "f();"   */
    typespecmap_(t) &= ~ (STGBITS|b_synbits);           /* not necessary? */
    if (curlex.sym == s_semicolon && !(declflag & FORMAL) ||
        curlex.sym == s_rbrace    && (declflag & LENGTH_OK))
    {   /*
         * Found an empty declaration or an empty declaration within a
         * struct or union in which the terminating ';' has been omitted.
         */
        d = 0;
        if (!(declxtramap & B_DECLMADE))
            syn_pccwarn(syn_rerr_ineffective);
#ifdef TARGET_HAS_DEBUGGER
        else
        {   /*
             * Here we have just got a new struct/union/enum tag
             * so lets tell the debugger about it, lest it never
             * finds out about it until its too late.
             */
            if (usrdbg(DBG_PROC)) dbg_type(gensymval(1),t);
        }
#endif
    }
    else
    {   d = rd_declrhslist(s, t, declflag);
        if (topfnflag)
        {   /* NB. it is vital that when topfnflag is set on return     */
            /* from rd_declrhslist we must soon pop_varenv/tagenv.      */
            /* This has to be done after reading the body due to ANSI   */
            /* scope joining of formals and body top block.             */
            if (feature & FEATURE_PCC) implicit_return_ok = syn_oldeformals;
            /* the next line allows static f(); - ansi_warn soon? */
            if (declxtramap & B_OMITTYPE)
            {   if (suppress & D_IMPLICITVOID)
                    /* The next line allows us also to avoid the       */
                    /* 'implicit return' warning in f(){}.             */
                    xwarncount++, implicit_return_ok = 1;
                else
                    syn_warn(syn_warn_untyped_fn, symname_(d->declname));
            }
            return rd_fndef(d, declflag);        /* d != 0 by fiat */
        }
        /* this message is a little late, but cannot occur 'on time' */
        if (declxtramap & B_OMITTYPE && d != 0)
            /* the case d==0 has already been reported by          */
            /* rd_declarator() via rd_declrhslist() (q.v.).        */
            if (!(feature & FEATURE_PCC))
                syn_rerr(syn_rerr_missing_type3);
    }
    if ((declflag & FORMAL) == 0)
    {
      if (!(feature & FEATURE_PCC) || curlex.sym != s_rbrace)
        checkfor_delimiter_2ket(s_semicolon, s_comma);
    }
    return (TopDecl *) syn_list2(s_decl, d);
}

static DeclRhsList *rd_decl2(int declflag)
{   TopDecl *d = rd_decl(declflag);
    if (d == 0 || h0_(d) != s_decl)
         syserr(syserr_rd_decl2, d, (long)(d==0 ? 0 : h0_(d)));
    return d->v_f.var;
}

/* rd_formals() is only used once in rd_declarator_1().  It behaves     */
/* very much like rd_decllist, but different concrete syntax.           */
/* It copes with both ANSI and olde-style formals.                      */
static DeclRhsList *rd_formals(void)
{   DeclRhsList *p,*q,*temp;
    const int32 minformals = -1;
    if (curlex.sym == s_rpar)
    {   syn_minformals = 0, syn_maxformals = 999, syn_oldeformals = 1;
        return 0;
    }
    for (p = q = 0;;)
    {   if (curlex.sym == s_ellipsis)
        {   if (p == 0) cc_rerr(syn_rerr_ellipsis_first);
            fault_incomplete_formals(p);
            nextsym();
            syn_minformals = length((VoidStar)p), syn_maxformals = 1999;
            syn_oldeformals = !is_proto_arglist(p,1);
/* @@@ the last line is a gross hack (abetted by AM) */
            return p;    /* to checkfor_ket(s_rpar) */
        }
#ifdef EXTENSION /* allow optional, but type-checked parameters. */
        if (curlex.sym == s_cond) nextsym(), minformals = length((VoidStar)p);
#endif
        temp = rd_decl2(FORMAL);
        if (curlex.sym == s_nothing) nextsym();
        for (; temp != 0; temp = temp->declcdr)
        {   if (debugging(DEBUG_BIND))
                cc_msg(" Formal: $r\n", temp->declname);
            if (p == 0) p = q = temp; else q->declcdr = temp, q = temp;
        }
        /* all that should legally appear here are ',' and ')', but fix    */
        /* ';' as ',' a la pascal and treat all other symbols as ')' to be */
        /* faulted by the caller (rd_declarator_1()).                      */
        if (curlex.sym == s_semicolon)   /* error recovery */
            syn_rerr(syn_rerr_semicolon_in_arglist);
        else if (curlex.sym != s_comma)
        {   /* arg list end, but first check for ANSI  "(void)" argument list
             * and remove it.  Note that "f(void) {}" is also OK by ANSI
             * if somewhat curious.
             */
            bool b = is_proto_arglist(p,0);
            if (p != 0 && p->declcdr == 0 &&                /* 1 parameter */
                p->declname == 0 &&                         /* no name     */
                equivtype(p->decltype, te_void))      /* void (or typedef) */
                    p = 0;                            /* then clear arglist */
            fault_incomplete_formals(p);
            syn_maxformals = length((VoidStar)p);
            syn_minformals = minformals >= 0 ? minformals : syn_maxformals;
            syn_oldeformals = !b;
            return p;
        }
        nextsym();
    }
}

static Declarator *rd_formals_1(Declarator *a)
{   int32 oldbl = bind_level;
    Unbinder *oldvars = NULL;
    Untagbinder *oldtags = NULL;
    DeclRhsList *f;
    TypeExprFnAux s;
    if (bind_level == 0) bind_level = 1;             /* @@@ nasty */
    if (bind_level == 2) (oldvars = push_varenv(), oldtags = push_tagenv());
    f = rd_formals();
    bind_level = oldbl;
/* @@@ maybe we ought to do the checkfor(')') before unbinding so that    */
/* not-used warnings follow errors about missing ')'?  Not worth it?      */
    if (bind_level == 2) (pop_varenv(oldvars), pop_tagenv(oldtags));
/* @@@ beware that ANSI disallow the use of pragma's which change         */
/* semantics -- those for printf requesting extra warnings are ok, but    */
/* not those like 'no_side_effects' which can screw code.                 */
              /* extra results for minargs_(), maxargs_()... */
    return mkTypeExprfn(t_fnap, a, (FormTypeList *)f,
                    packTypeExprFnAux(s, (int)syn_minformals,
                                         (int)syn_maxformals,
                                         (int)special_variad,
                                         (int)var_no_side_effects,
                                         syn_oldeformals));
}

static DeclRhsList *rd_decllist(int declflag)
/* reads a list of Decls and NCONC's them togther.  Used for struct/union
   elements, type specs after formal lists, rd_block().  NB:  It should not
   be used when TOPLEVEL is set as it does not look for functions.
   Identical to rd_formals modulo concrete syntax - unify???
   Note the Symstr's may be 0 (abstract), or s_id.
*/
{   DeclRhsList *p,*q,*temp;
    p = q = 0;
    while (isdeclstarter_(curlex.sym) ||
           (curlex.sym == s_identifier && istypedefname_(curlex.a1.sv)))
    {   if (curlex.sym == s_typestartsym) nextsym();
        temp = rd_decl2(declflag);
        if (curlex.sym == s_nothing) nextsym();
        for (; temp != 0; temp = temp->declcdr)
        {   if (p == 0) p = q = temp; else q->declcdr = temp, q = temp;
            if (debugging(DEBUG_BIND)) cc_msg(" Identifier: $r", q->declname);
        }
    }
    return p;
}

static bool member_TagMemList(Symstr *s, TagMemList *p)
{   for (; p != NULL; p = p->memcdr)
        if (s == p->memsv) return 1;
    return 0;
}

static TagMemList *rd_strdecl(TagBinder *b)
/* Currently this just mungs the structure returned by rd_decllist().   */
/* BEWARE: TagMemList/DeclRhsList store re-use.                         */
{   DeclRhsList *l = rd_decllist(LENGTH_OK), x;
    TagMemList *p = 0, *q = 0, *temp;
    bool nameseen = 0;
    if (l == 0)
    {   cc_rerr(syn_rerr_no_members, b);
        return 0;
    }
    for (; l != 0; l = x.declcdr)
    {   x = *l;                        /* so we can mung l itself */
/* bind_level is used so that all structs are globalized except within fns */
/* This includes structs declared in formal parameter lists, whose scope   */
/* is only the function -- see mip/bind.c.                                 */
        if (bind_level < 2)
        {   /* invent a globalize_tagmemlist()? */
            temp = (TagMemList *) GlobAlloc(SU_Type, sizeof(TagMemList));
            x.decltype = globalize_typeexpr(x.decltype);
            x.declinit = x.declinit == 0 ? 0 :  /* BITFIELD size or zero: */
                         globalize_int(evaluate(x.declinit));
        }
        else
        {
#if defined UNNECESSARY
	  if (sizeof(TagMemList) > sizeof(DeclRhsList))
                syserr(syserr_rd_strdecl);
#endif
            temp = (TagMemList *)l;
            if (x.declinit != 0 && h0_(x.declinit) != s_integer)
                x.declinit = mkintconst(te_int, evaluate(x.declinit), 0);
        }
        temp->memcdr = 0, temp->memsv = x.declname;
             /* l->declname would be 0 for padding (:0) bit fields */
        temp->memtype = x.decltype, temp->membits = x.declinit;
        {   Symstr *sv = temp->memsv;
            if (sv)
            {   nameseen = 1;
                if (member_TagMemList(sv,p))
                    temp->memsv = gensymval(1),
                    cc_rerr(syn_rerr_duplicate_member(sv,b));
            }
        }
        if (p == 0) p = q = temp; else q->memcdr = temp, q = temp;
    }
    if (!nameseen) cc_warn(syn_warn_no_named_member, b);
    return p;
}

/* rd_enumdecl() works very much like rd_decllist() or rd_strdecl() but the
   different surface syntax leads me to implement it separately.
   Syntax is taken from Harbison&Steele.
   ANSI draft means 'int' implementation of 'enum's.
*/

static TagMemList *rd_enumdecl(TypeExpr *t)
{   DeclRhsList *p,*q,*temp;
    int32 nextenumval = 0;
    bool is_non_neg = YES;
    for (p = q = 0;;)
    {   if (curlex.sym == s_identifier)
        {   Symstr *sv = curlex.a1.sv; Expr *e = 0; Binder *b;
            temp = mkDeclRhsList(0, sv, t, 0, b_enumconst, 0);
            nextsym();
            if (curlex.sym == s_assign)
            {   nextsym();
                e = optimise0(rd_expr(UPTOCOMMA));
            }
/* @@@ AM: fixup 16 bit ints here too.                                  */
            if (e != 0)
            {
	      nextenumval = evaluate(e);
                is_non_neg =
                   (h0_(e) == s_integer &&
                    typespecmap_(type_(e)) & bitoftype_(s_unsigned) ||
                    nextenumval >= 0);
            }
/* bind_level is used so that all enum consts are globalized except in fns */
            b = instate_declaration(temp, bind_level == 0 ? TOPLEVEL : 0);
            if (is_non_neg && nextenumval < 0)
                is_non_neg = NO,                /* only one error.      */
                cc_rerr(sem_rerr_monad_overflow(s_enum,0,0));
            bindaddr_(b) = nextenumval++;
            if (p == 0) p = q = temp; else q->declcdr = temp, q = temp;
        }
        else
        {   syn_err(syn_err_enumdef);
            while (curlex.sym != s_rbrace && curlex.sym != s_eof) nextsym();
        }
        if (curlex.sym != s_comma) break;
        nextsym();
        if (curlex.sym == s_rbrace && (feature & FEATURE_PCC))
        {    syn_warn(syn_warn_extra_comma);
             break;
        }
    }
    /* @@@ note the punning between TagMemList/DeclRhsList */
    return (TagMemList *)p;
    /* @@@ I think result is only used as a 0/non-0 flag now 21-8-86 */
}

/* ONLY syn_init() and rd_topdecl() are for the world at large.
   Calling conventions: set curlex.sym to s_nothing or do nextsym() before first
   calling rd_topdecl().  This is best done by lex_init().
   Notes: K&R reckon a program is non-empty, Harbison & Steele disagree.
*/

TopDecl *rd_topdecl(void)
{
    implicit_return_ok = 0;      /* for junky old C programs         */
#ifdef EXTENSION_VALOF
    inside_valof_block = 0;
    valof_block_result_type = (TypeExpr *) DUFF_ADDR;
    cur_restype = 0;             /* check for valof out of body      */
#endif
    if (curlex.sym == s_nothing) nextsym();
    while (curlex.sym == s_toplevel) nextsym();
    while (curlex.sym == s_lbrace)  /* temp for ACN - rethink general case */
    {   syn_err(syn_err_misplaced_brace);
        (void)rd_body(te_int);   /* this will also skip initialisers */
        label_resolve();         /* tidy up in case declarations */
        pop_varenv(0);
        pop_tagenv(0);
        if (curlex.sym == s_nothing) nextsym();
        while (curlex.sym == s_toplevel) nextsym();
    }
    return rd_decl(TOPLEVEL);
}

void syn_init(void)
{   initpriovec();
}

/* End of cfe/syn.c */
