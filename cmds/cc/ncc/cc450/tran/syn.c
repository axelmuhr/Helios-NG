/*
 * cfe/syn.c: syntax analysis phase of C/C++ compiler.
 * Copyright (C) Codemist Ltd., 1988-1993
 * Copyright (C) Acorn Computers Ltd., 1988-1990.
 * Copyright (C) Advanced RISC Machines Limited, 1992.
 */

/*
 * RCS $Revision: 1.1 $ Codemist 175
 * Checkin $Date: 1995/05/19 11:38:17 $
 * Revising $Author: nickc $
 */

/* AM memo: Jan 93: need to fix lookahead w.r.t. '#if' (re-entrancy).   */
#define s_qualified 512         /* hack */
#define s_pseudoid s_record     /* ditto, stolen */
#define s_init s_with           /* ditto, stolen */

/* AM Memo: TypeFnAux (e.g. oldstyle) may enable us to kill the        */
/*          dreadful uses of 999 and 1999 in the following.            */
/* AM Memo: reconsider the uses of FileLine fl, in particular the      */
/* possibility of having a FileLine field in DeclRhsList.              */
/* AM Jan 90: rework 'declflag' contexts; fix register syntax,         */
/*            fn-type parsing in formals/casts and 'void' formals.     */
/* AM Sep 89: Rework 'incomplete type' code.  Kill some Parkes hacks.  */
/* AM Sep 89: re-work s_typestartsym so that it may only occur in      */
/*            contexts in which macros may reasonably use it.          */
/* AM, Mar 89: rework rd_declarator now that ANSI have ruled on [] and */
/*             () combinations in declarators.  Fix bugs whereby       */
/*             bad combinations of () and [] via typedef were missed.  */

#include <string.h>    /* for memset */
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
#include "codebuf.h"

enum blk_flavour { blk_BODY,            /* for rd_block(function body)  */
                   blk_INNER,           /* for rd_block(inner block)    */
                   blk_IMPLICIT };      /* for rd_block(C++ decl)       */

/* The next line is in flux, but flags code which is invented, and     */
/* does not correspond to a user written action.  E.g. narrowing in    */
/* f(x) float x; {...}.  Note that x = 1 in { int x = 1; is not so.    */
static FileLine syn_invented_fl = { 0, 0, 0 };
/* Code may test for invented FileLines by testing (fl.f == 0)!!!      */

/* forward references within this file - reorganise to reduce */
static void ensure_formals_typed(DeclRhsList *d,bool proto);
static TypeExpr *rd_typename(int declflag);
static Expr *rd_expr(int n);
static Expr *rd_prefixexp(int n);
#ifdef EXTENSION_VALOF
static Cmd *rd_block(enum blk_flavour f);
#endif
static Cmd *rd_command(bool declposs);
static DeclRhsList *rd_decllist(int declflag);
static ClassMember *rd_enumdecl(TypeExpr *t);
static ClassMember *rd_classdecl(TagBinder *, bool);
#ifdef CPLUSPLUS
static Cmd *rd_meminit(TagBinder *ctorclass);
static Expr *rd_cppcast(void);
static void syn_classname_typedef(TypeSpec *const tt, TagBinder *tb,
                                  const int declflag);
static Symstr *rd_operator(void);
static Expr *cpp_mkbinaryorop(AEop, Expr *, Expr *);
static Expr *cpp_mkunaryorop(AEop, Expr *);
static Expr *cpp_mkfnaporop(Expr *, ExprList *);
static Expr *syn_binderofname(Symstr *);
static Expr *rd_newordelete(AEop);
static Cmd *rd_cplusplus_for(FileLine);
static TypeExpr *fixup_special_member(Symstr *,
    TypeExpr *, TypeExpr *, TagBinder *);
static void memfn_typefix(DeclRhsList *, TagBinder *);
static void rd_asm_decl(void);
#endif
#ifdef CPLUSPLUSxxx
typedef struct CppName { Binder *bind; TagBinder *scope; } CppName;
static CppName *rd_cpp_name(CppName *, bool);
#endif

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

/* The values for 'declflag' below (with the exception of DUPL_OK which */
/* may be or'ed in) are private to parsing.  They represent CONTEXTS    */
/* (e.g. FORMAL) for declarators, abstract declarators, typenames, etc. */
/* The values are 'sparse' to allow quick testing for membership of     */
/* sets like CONC_DECLARATOR.                                           */

/* #define DUPL_OK            0x001 */      /* defined in bind.h */
/* #define TOPLEVEL           0x002 */      /* defined in bind.h */
#define POINTERTYPES       0x004
#define MEMBER             0x008  /* i.e. in struct/union */
#define BLOCKHEAD          0x010
#define ARG_TYPES          0x020  /* declaring formals, after e.g. 'f(x)'  */
#define FORMAL             0x040  /* inside ()'s: both (int a) and (a) OK. */
#define TYPENAME           0x080  /* i.e. typename for case/sizeof.        */
#define SIMPLETYPE         0x100  /* C++ casts like "int(3)" etc.          */
#define CONVERSIONTYPE     0x200  /* C++ "operator int * ();"              */
#define NEW_TYPENAME       0x400  /* C++ new-type-name [ES].               */
#define FLEX_TYPENAME      0x800  /* C++ 'new' TYPENAME, [var] ok [ES].    */
/* contexts in which 'storage classes' are allowed (vestigial):         */
#define STGCLASS_OK        (TOPLEVEL|BLOCKHEAD|FORMAL|ARG_TYPES)
/* contexts in which a type is REQUIRED (typename and struct member):   */
#ifdef CPLUSPLUS
#define TYPE_NEEDED        (TYPENAME|FLEX_TYPENAME|NEW_TYPENAME)
#else
#define TYPE_NEEDED        (TYPENAME|FLEX_TYPENAME|NEW_TYPENAME|MEMBER)
#endif
/* contexts in which declarators must be ABSTRACT or CONCRETE.          */
#define CONC_DECLARATOR    (TOPLEVEL|BLOCKHEAD|ARG_TYPES|MEMBER)
#define ABS_DECLARATOR     (TYPENAME|FLEX_TYPENAME|NEW_TYPENAME)

/* parsing table initialisation... */
static int32 illtypecombination[NUM_OF_TYPES];

static int bind_scope;           /* TOPLEVEL, GLOBALSCOPE, or LOCALSCOPE */

/* Operator priority for rd_expr(): we use even priorities for           */
/* left-associative operators and odd priorities for right-association.  */
/* This means that we can get right-priority simply by or-ing one into   */
/* the left-priority.                                                    */
/* Other things could added to lpriovec, such as 'needs lvalue' etc.     */
static char lpriovec[s_NUMSYMS];
#define lprio_(op) lpriovec[op]
#define rprio_(op) (lpriovec[op] | 1)

#define UPTOCOMMA 11
#define PASTCOMMA 10

static void initpriovec(void)
{   AEop s,t;
    AEop i;
    for (s = s_char; istypestarter_(s); s++)
        illtypecombination[shiftoftype_(s)] = ~0;
    illtypecombination[shiftoftype_(s_signed)] =
    illtypecombination[shiftoftype_(s_unsigned)] =
        ~(bitoftype_(s_int) | bitoftype_(s_char) | bitoftype_(s_longlong) |
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
#ifdef CPLUSPLUS
    lpriovec[s_dotstar] =
    lpriovec[s_arrowstar] = 36;
#endif
}

/* check_bitsize and check_bittype get prunetype'd types.               */
static Expr *check_bitsize(Expr *e, TypeExpr *t, Symstr *sv)
{   unsigned32 n = evaluate(e);
    unsigned32 size = MAXBITSIZE;
    /*
     * In C++/PCC mode bit fields may be 'char', 'short', 'long' or 'enum'.
     * In ANSI mode bit fields must (else undefined) be 'int'.
     */
    if (h0_(t) == s_typespec)
    {   if (typespecmap_(t) & bitoftype_(s_char)) size = 8;
        else if (typespecmap_(t) & bitoftype_(s_short)) size = 8*sizeof_short;
        else if (typespecmap_(t) & bitoftype_(s_long)) size = 8*sizeof_long;
        /* else treat enum as 'int'.                                    */
        /* @@@ Sept 91: we should check enum range against bit size,    */
        /* and even add signedness info to ensure happyness!            */
    }
    if (n > size /* unsigned! */) cc_err(syn_err_bitsize, (long)n);
    else if (n==0 && sv!=NULL) n = -1, cc_err(syn_err_zerobitsize);
    if (n > size || h0_(e) != s_integer) e = globalize_int(1);
    return e;
}

/* check_bitsize and check_bittype get prunetype'd types.               */
static TypeExpr *check_bittype(TypeExpr *t)
{   /* In ANSI C, only int bitfields are allowed, C++ also allows
     * char/short/long/enum (same as PCC mode).
     * In PCC mode, it's important NOT to translate char and short
     * types to int, because they are packed differently.
     * Also C++ typechecking requires 'enum' not to be translated to int.
     */
#define OKbittypes       (bitoftype_(s_int)| \
                          bitoftype_(s_char)|bitoftype_(s_short)| \
                          bitoftype_(s_enum)|bitoftype_(s_long)|  \
                          bitoftype_(s_unsigned)|bitoftype_(s_signed)| \
                          bitoftype_(s_const)|bitoftype_(s_volatile))
    if (h0_(t) != s_typespec || typespecmap_(t) & ~OKbittypes)
    {   cc_err(syn_rerr_bitfield, h0_(t)!=s_typespec ? t :
                        /* ensure nice message (probably OK anyway) */
                        primtype_(typespecmap_(t) & ~OKbittypes));
        t = te_int;
    }
    if (!(feature & FEATURE_PCC) && h0_(t) == s_typespec &&
          typespecmap_(t) & (bitoftype_(s_char)|bitoftype_(s_short)|
                             bitoftype_(s_enum)|bitoftype_(s_long)))
    {
#ifdef CPLUSPLUS
        /* @@@ warn for any non-ANSI C bitfield type, but leave alone!  */
        cc_warn(syn_rerr_ANSIbitfield,
                primtype_(typespecmap_(t) & ~bitoftype_(s_int)));
#else
        cc_rerr(syn_rerr_ANSIbitfield,
                primtype_(typespecmap_(t) & ~bitoftype_(s_int)));
        t = te_int;
#endif
    }
/* sem.c is responsible for the interpretation of 'plain' int bitfields     */
/* as signed/unsigned (q.v. under FEATURE_SIGNED_CHAR).                     */
/* It is vital that 'BITFIELD' on the next line does not qualify a typedef. */
    return primtype2_(typespecmap_(t) | BITFIELD, typespecbind_(t));
}

static Expr *check_arraysize(Expr *e)
{   unsigned32 n = evaluate(e);
    if (n == 0 && !(suppress & D_ZEROARRAY)) cc_pccwarn(syn_rerr_array_0);
/* The limit imposed here on array sizes is rather ARBITRARY, but char */
/* arrays that consume over 16Mbytes seem silly at least in 1988/89!   */
    if (n > 0xffffff) cc_err(syn_err_arraysize, (long)n);
    if (n > 0xffffff || h0_(e) != s_integer) e = globalize_int(1);
    return e;
}

static char *ctxtofdeclflag(int f)
{  if (f & POINTERTYPES)   return errname_pointertypes;
   if (f & TOPLEVEL)       return errname_toplevel;
   if (f & MEMBER)         return errname_structelement;
   if (f & FORMAL)         return errname_formalarg;
   if (f & ARG_TYPES)      return errname_formaltype;
   if (f & BLOCKHEAD)      return errname_blockhead;
   if (f & TYPENAME)       return errname_typename;
#ifdef CPLUSPLUS
   if (f & SIMPLETYPE)     return "<simple type>";         /* */
   if (f & CONVERSIONTYPE) return "<conversion type>";
   if (f & (FLEX_TYPENAME|NEW_TYPENAME))   return "<new-type-name>";
#endif
                           return errname_unknown;
}

static AEop peepsym()
{   AEop s;
    nextsym();
    s = curlex.sym;
    ungetsym();
    return s;
}

static void checkfor_ket(AEop s)
{
/* The next lines are a long-stop against loops in error recovery */
    if (++errs_on_this_sym > 4)
    {   cc_err(syn_err_expecteda, s);
        nextsym();
    }
    else if (curlex.sym == s) nextsym();
    else cc_err(syn_err_expected, s);
}

static void checkfor_delimiter_ket(AEop s, char *more)
                            /* as checkfor_ket but less read-ahead */
{
/* The next lines are a long-stop against loops in error recovery */
    if (++errs_on_this_sym > 4)
    {   cc_err(syn_err_expected1a, s, more);
        nextsym();
    }
    else if (curlex.sym == s) curlex.sym = s_nothing;
    else cc_err(syn_err_expected1, s, more);
}

static void checkfor_2ket(AEop s, AEop t)
{
/* The next lines are a long-stop against loops in error recovery */
    if (++errs_on_this_sym > 4)
    {   cc_err(syn_err_expected2a, s, t);
        nextsym();
    }
    else if (curlex.sym == s) nextsym();
    else cc_rerr(syn_err_expected2, s, t, s);
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
    while (nextsym(), isstring_(curlex.sym))
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
    p = q = mkExprList(0, rd_expr(UPTOCOMMA));
    while (curlex.sym == s_comma)
    {   nextsym();
        q = cdr_(q) = mkExprList(0, rd_expr(UPTOCOMMA));
    }
    return p;
}

/* Notionally, rd_primary and rd_prefixexp should take no argument.
   However, it is most convenient to give it an argument 'labelhack'
   which we set to POSSLABEL to indicate statement contexts.
   In such contexts the possible expressions are 'label:' or <expression>.
   This is necessary to enable lookahead past s_identifier's in statement
   context to see if there is a colon, with represents a label, or an
   operator like '='.  Consider "a:b=c;" or "a:b++;".
   Note that this must be done by rd_primary since labels and variables
   have different environments.
   Doing this re-entrantly is important since "#if..." happens rather
   asynchronously.
*/

#define NOLABEL   PASTCOMMA        /* for clarity */
#define POSSLABEL (PASTCOMMA-2)    /* lower (= same) priority as PASTCOMMA */

static bool elselast;   /* private to rd_command(), cleared by valof.   */
static Cmd *cur_switch, *cur_loop;
static AEop cur_break;   /* what 'break' means i.e. s_break or s_endcase */
static TypeExpr *cur_restype, *cur_switchtype;
#ifdef EXTENSION_VALOF
static TypeExpr *valof_block_result_type;
#endif

/* When we have a s_identifier in C++, we sometimes (but e.g. not after */
/* symbols like 'goto') read a qualified name.  Note we should be       */
/* careful about ungetsym() as these aren't part of curlex (but all     */
/* is currently OK).                                                    */
#ifdef CPLUSPLUS
static TagBinder *curlex_scope;         /* if curlex.sym & s_qualified. */
static ClassMember *curlex_qbind;       /* ditto (may be s_binder!)     */
static TypeExpr *curlex_optype;         /* ditto.                       */
#endif
static Binder *curlex_typedef;          /* always set, even for C       */
/* S::times (S::*), S::operator, S::identifier, S::bitnot are  used.    */
/* S::new, S::delete?                                                   */

#ifdef CPLUSPLUS
static TagBinder *rd_scope()
{   int msg = 0;
    TagBinder *p = 0;
    while (curlex.sym == s_identifier && peepsym() == s_coloncolon)
    {   Symstr *sv = curlex.a1.sv;
        p = findtagbinding(sv);
        if (msg != 0)
            p = 0;
        else if (p == 0)
            msg = 1, cc_err("class $r not found", sv);
        else if (attributes_(p) & bitoftype_(s_enum))
            msg = 1, cc_err("$c is not a class name", p), p = 0;
        else
            push_scope(p);              /* @@@ is this right?           */
        nextsym();                      /* read the '::'...             */
        nextsym();                      /* ... and the following symbol */
    }
    return p;
}

static void rd_pseudo(TagBinder *scope)
{   /* maybe $l should be able to undo s_pseudoid for msgs?             */
    switch (curlex.sym)
    {
case s_bitnot:
        if (scope)
        {   nextsym();
/* Can we have "->~A::A" ?  No.  Check also p->A::~A(); etc.            */
            if (!(curlex.sym == s_identifier &&
                  findtagbinding(curlex.a1.sv) == scope))
                cc_err("illegal destructor ~$l");
            if (curlex.sym != s_identifier) ungetsym();
            curlex.sym = s_pseudoid;
        }
/* If scope=0 the following helps recovery (and is otherwise harmless). */
        curlex_optype = 0;
        curlex.a1.sv = dtorsym;
        break;
case s_operator:
        {   Symstr *sv = rd_operator();         /* sets curlex_optype   */
            ungetsym();
            curlex.a1.sv = sv;
            curlex.sym = s_pseudoid;
        }
        break;
    }
}
#endif

/* change so that curlex.sym is arg+result?                             */
/* xscope is a bit of a hack -- it is always 0 except when used         */
/* after a C++ '->' or '.' (to read constructors/destructors).          */
/* (or in member scope too).  defer the A => __ctor map?                */
static void rd_scope_or_typedef(TagBinder *xscope)
{   switch (curlex.sym)
    {
#ifdef CPLUSPLUS
case s_bitnot:
case s_operator:
        curlex_qbind = 0;
        rd_pseudo(xscope);
        return;
case s_coloncolon:
        if (pp_inhashif) return;
        curlex_scope = 0;
        curlex_typedef = 0;
        nextsym();
        switch (curlex.sym)
        {
    default:    cc_err("$l cannot follow unary '::'"); return;
    case s_operator:    rd_pseudo(0);
    case s_new:
    case s_delete:      curlex.sym |= s_qualified; return;
    case s_identifier:
                cc_warn("::identifier ignored (unimplemented)");
        }
        xscope = 0;
        /* drop through */
#endif
case s_identifier:      /* not s_qualified+s_identifer (already done).  */
        curlex_typedef = 0;
/* don't recognise typedefs in #if even if extension "keywords in #if"! */
        if (pp_inhashif) return;
#ifdef CPLUSPLUS
        curlex_scope = 0;
        curlex_qbind = 0;
        if (peepsym() != s_coloncolon)
        {   Symstr *sv = curlex.a1.sv;
/* The next look-ahead isn't quite precise enough.                      */
            if (xscope && findtagbinding(sv) == xscope && peepsym() == s_lpar)
            {   curlex_optype = 0;
                curlex.a1.sv = ctorsym;
                curlex.sym = s_pseudoid;
            }
        }
        else
        {   TagBinder *tb = rd_scope();
            switch (curlex.sym)
            {
    default:    cc_err("$l cannot follow binary '::'"); return;
    case s_times:
                if (tb)
                {   curlex_scope = tb;
                    curlex.sym |= s_qualified;          /* s_qtimes?    */
                }
                return;
    case s_bitnot:
    case s_operator:
                rd_pseudo(tb);
                /* drop through */
    case s_identifier:
                if (tb)
                {   curlex_scope = tb;
                    if (findtagbinding(curlex.a1.sv) == tb)
                    {   curlex_optype = 0;
                        curlex.a1.sv = ctorsym;
                        curlex.sym = s_pseudoid;
                    }
/* @@@ next line is a hack, but becoming correct!                       */
/* findmember() isn't quite right -- we need to see through inheritance */
/* but NOT up to top level (as would findbinding).                      */
                    curlex_qbind = findmember(curlex.a1.sv);
                    if (curlex_qbind == 0)
                        cc_err("$c has no $r member", tb, curlex.a1.sv);
                    curlex.sym |= s_qualified;
                }
                if ((curlex.sym & ~s_qualified) != s_identifier) return;
            }
            /* drop though if maybe typedef */
        }
#else
        IGNORE(xscope);
#endif
        {   Symstr *sv = curlex.a1.sv;
            Binder *b = findbinding(sv);
            if (b && h0_(b) == s_binder &&
                (bindstg_(b) & bitofstg_(s_typedef)))
                    curlex_typedef = b;
        }
    }
}

/* Calling rd_scope_or_typedef() must always precede isdeclstarter2_(). */
#define isdeclstarter2_(curlex) \
    (isdeclstarter_(curlex.sym) ||  \
        ((curlex.sym & ~s_qualified) == s_identifier && curlex_typedef != 0))

/* We (temp) need the following for olde-style C within C++.  Consider  */
/* int x = 3; int a(x); int f(x) int x; { return x; }.                  */
/* Really we need to read ahead to decide on seeing ';' (or ',' etc).   */
#define isdeclstarter2x_(curlex,declflag) \
    (isdeclstarter2_(curlex) || \
     (declflag & TOPLEVEL && curlex.sym == s_identifier))

static Symstr *rd_member_name(TagBinder *scope)
/* Read a member name (id/~id/operator type/operator op) and return     */
/* the (Symstr *) for it, or 0 if an error given.                       */
/* Anomalously leave curlex at last symbol read, hence ungetsym()'s...  */
/* @@@ we are moving to having rd_member_name return a ClassMember *    */
{   if (debugging(DEBUG_SYN)) cc_msg("rd_member $l in scope $b\n", scope);
    rd_scope_or_typedef(scope);
    switch (curlex.sym & ~s_qualified)
    {
case s_identifier:
case s_pseudoid:
        return curlex.a1.sv;
default:
        cc_err(syn_err_expected_member);
        ungetsym();
        return 0;
    }
}

static Expr *mkunaryorop(AEop op, Expr *a)
{   if (h0_(a) == s_error) return errornode;    /* @@@ correct placing? */
#ifdef CPLUSPLUS
    return cpp_mkunaryorop(op, a);
#else
    return mkunary(op, a);
#endif
}

static Expr *mkbinaryorop(AEop op, Expr *a, Expr *b)
{   if (h0_(a) == s_error || h0_(b) == s_error) return errornode;
#ifdef CPLUSPLUS
    return cpp_mkbinaryorop(op, a, b);
#else
    return mkbinary(op, a, b);
#endif
}

static Expr *mkfnaporop(Expr *e, ExprList *l)
/* Here I have special processing for a pseudo-function-call to
 * ___assert(bool, string) which (at this very early stage) is turned into
 * (void)0, but if the boolean is not a compile time nonzero constant
 * the string is used to generate a diagnostic.
 * Feb 93: for va_arg a ___cond(a,b,c) and ___fail(string) could be more
 * useful separated.
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
#ifdef CPLUSPLUS
    if (h0_(e) == s_error) return errornode;
    return cpp_mkfnaporop(e, l);
#else
    return mkfnap(e,l);
#endif
}

static Expr *rd_idexpr(int labelhack)
/* @@@ merge in operator/::operator?                                    */
{   Expr *a;
    Binder *b;
    Symstr *sv;
/* @@@ surely we can do better than the following!  stop S::* here too. */
    if ((curlex.sym & ~s_qualified) != s_identifier) syserr("rd_idexpr");
    sv = curlex.a1.sv;
    if (labelhack == POSSLABEL && curlex.sym == s_identifier
                               && peepsym() == s_colon)
    {   nextsym();
/* Here curlex.sym==s_colon so postfix forms won't be considered.       */
        return (Expr *)syn_list2(s_colon, sv);
    }
#ifdef CPLUSPLUSxxx
    /* for the member case! */
    {   b = findbinding(thissym);
        binduses_(b) |= u_referenced;
        a = rd_cpp_name(typearg_(bindtype_(b)), NO);
        if (h0_(a) != s_error)
            a = rooted_path(a, mkdotable(s_arrow, (Expr *)b));
    }
    /* 'a' is the member access expression computed above  */
    /* by calling rd_cpp_name(), mkdotable() and rooted_path(). */
#endif
    if ((b = findbinding(sv)) == NULL)
    {   AEop peek = peepsym();
        if (peek == s_lpar)
        {   if (warn_implicit_fns)
                cc_warn(syn_warn_invent_extern, symname_(sv));
            else xwarncount++;
        }
        else
            cc_rerr(syn_rerr_undeclared, symname_(sv));
        a = (Expr *) (b = implicit_decl(sv, peek==s_lpar));
    }
#ifdef CPLUSPLUS
    else if (h0_(b) == s_member)
    {   /* note &A::a may be valid even when x = A::a isn't, hence      */
        /* for defer any test for 'this' until we know the context,     */
        /* i.e. coerceunary() etc.  For non-qualified names there       */
        /* is no such problem.  See [ES, p55].                          */
        if (curlex.sym & s_qualified)
        {   /* We ignore the class prefix because of [ES,p55,note1].    */
/* @@@ note the potential user disaster in that &D::i has type          */
/* (int B::*), not (int D::*).  This will cause pain as unchecked       */
/* parameters, else implicit casts will save us.                        */
            a = (Expr *)b;
        }
        else    /* @@@ share this code with coerceunary(s_qualified)!   */
        {   Binder *thisb = findbinding(thissym);
            if (thisb && h0_(thisb) == s_binder)
               binduses_(thisb) |= u_referenced,
               a = mkfieldselector(s_arrow, (Expr *)thisb, sv, 0);
            else
               cc_err("No 'this' pointer to access member $r", sv),
               a = errornode;
        }
    }
#endif
    else if (bindstg_(b) & bitoftype_(s_typedef))
    {
#ifdef CPLUSPLUS
        if ((a = rd_cppcast()) != 0) return a;
#endif
        cc_err(syn_err_typedef, sv);
        a = errornode;
    }
    else if (isenumconst_(b))
        /* Map enum const binders to values. */
        a = mkintconst(bindtype_(b),bindaddr_(b),(Expr *)b);
    else
        /* use current 'variable' binding */
        a = (Expr *)b;
    if (b && h0_(b) == s_binder) binduses_(b) |= u_referenced;
    nextsym();
    return a;
}


/* rd_primary is called with rd_scope_or_typedef having been called     */
/* on curlex.sym.  (In rd_prefixexp() if not earlier.)                  */
static Expr *rd_primaryexp(int labelhack)
{
    Expr *a;
    switch (curlex.sym & ~s_qualified)
    {
default:
#ifdef CPLUSPLUS
        /* rd_idexpr() parses C++ casts/constructors fns via typedef.   */
        if (isdeclstarter_(curlex.sym) && (a = rd_cppcast()) != 0)
            return a;
#endif
        cc_err(syn_err_expected_expr);
        return errornode;
case s_lpar:
        nextsym();
        rd_scope_or_typedef(0);
        if (isdeclstarter2_(curlex))   /* rd_declspec(stgclass) moans   */
        {   TypeExpr *t;
/* The next line supports a keyword "__type" which is treated           */
/* almost as whitespace here, but which is used by a few macros as a    */
/* clue to the parser that certain parenthesised expressions are casts  */
/* and not just arithmetic.  This (only) aids error recovery.           */
            if (curlex.sym == s_typestartsym) nextsym();
            t = rd_typename(TYPENAME);
            checkfor_ket(s_rpar);
#ifdef EXTENSION_VALOF
            /* invalidate syntactically top level "int a = (int){...};" */
            if (curlex.sym == s_lbrace && cur_restype)
            {   Cmd *c;
                TypeExpr *saver = valof_block_result_type;
                if ((suppress & D_VALOFBLOCKS) == 0)
                   cc_err(syn_err_valof_block);
/* Set a flag so that 'resultis' is recognized as such */
                if (equivtype(t, te_void))
                {   cc_rerr(syn_rerr_void_valof);
                    t = te_int;     /* Fixup to be type int */
                }
                inside_valof_block++;
                valof_block_result_type = t;
                c = rd_block(blk_INNER);
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
case_s_any_string   /* cope with ANSI juxtaposed concatenation */
        a = rd_ANSIstring();
        break;
#ifdef CPLUSPLUS
#define syn_err_ill_this "legal only in member function: $l"
case s_this:
    {   Binder *b = findbinding(thissym);
        a = (b && h0_(b) == s_binder) ?
                (binduses_(b) |= u_referenced, (Expr *)b) :
                (cc_err(syn_err_ill_this), errornode);
        nextsym();
        return a;
    }
case s_pseudoid:
        return syn_binderofname(curlex.a1.sv);
#endif
case s_identifier:
        if (pp_inhashif)
        {   /* the following warning is a good idea - consider:
               enum foo { a,b }; #if a==b ... */
            if ((pp_inhashif == 1) || (feature & FEATURE_FUSSY))
                cc_warn(syn_warn_hashif_undef, symname_(curlex.a1.sv));
                    /* @@@ - LDS 11-Nov-92: why the @@@? */
            nextsym();
            a = mkintconst(te_int,0,0);
        }
        else
            a = rd_idexpr(labelhack);
        break;
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

static Expr *rd_postfix(Expr *a)
{   AEop op;
        for (;;) switch (op = curlex.sym)
        {   case s_plusplus:
            case s_minusminus:
                nextsym();
                a = mkunaryorop(postop_(op), a);
                break;
            case s_lpar:
                nextsym();
                a = mkfnaporop(a,
                        ((curlex.sym == s_rpar) ? 0 : rd_exprlist()));
                checkfor_2ket(s_rpar,s_comma);
                break;
            case s_lbracket:
                nextsym();
                a = mkbinaryorop(s_subscript, a, rd_expr(PASTCOMMA));
                checkfor_ket(s_rbracket);
                break;
            case s_dot:
            case s_arrow:
                nextsym();
                {
#ifdef CPLUSPLUS
/* @@@ the next section is a real temp. hack.                           */
                    TypeExpr *t = princtype(typeofexpr(a));
                    TypeExpr *tt = h0_(t) == t_ref ?
                                 princtype(typearg_(t)) : t;
                    TypeExpr *ttt = h0_(tt) == t_content ?
                                 princtype(typearg_(tt)) : tt;
                    TagBinder *scope = isclasstype_(ttt) ?
                         typespectagbind_(ttt) : 0;
/* @@@ end of temp. hack.                                               */
                    Symstr *sv = rd_member_name(scope);
#else
                    Symstr *sv = rd_member_name(0);
#endif
/* It is probably better to have sv an  s_member  in the C++ case.      */
                    a = (sv==0) ? errornode : mkfieldselector(op, a, sv, 0);
                    nextsym();
                }
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
        {   SET_BITMAP m = typespecmap_(x);
/* Hack round the C++ in aeops.h, map struct/class to same number,      */
/* preserving the magic numbers in <stdarg.h>                           */
            m = (m & (bitoftype_(s_class)-1)) |
                (m & -bitoftype_(s_class)) >> 1;
            m = (m & (bitoftype_(s_longlong)-1)) |
                (m & -bitoftype_(s_longlong)) >> 1;
            return m;
        }
case t_ref:
case t_content:
case t_subscript:
        return 0x10000000;
case t_fnap:
        return 0x20000000;
default:
        syserr(syserr_codeoftype);
        return 0x40000000;
    }
}

static int32 cpp_sizeoftype(TypeExpr *t)
{
#ifdef CPLUSPLUS
    t = princtype(t);   /* elide typedef's maybe losing qualifiers.     */
    /* There are no refs of refs, but the following code looks neater.. */
    while (h0_(t) == t_ref) t = princtype(typearg_(t));
#endif
    return sizeoftype(t);
}

#define te_size_t (feature & FEATURE_PCC ? te_int : te_uint)

static Expr *rd_prefixexp(int labelhack)
{   AEop op;
    Expr *a;
    rd_scope_or_typedef(0);       /* probably only really NEEDED in C++ */
/* Note ::new and ::delete are valid, but not S::* nor S::~.            */
    switch (op = curlex.sym)
    {
case s_plus:    if (feature & FEATURE_PCC)
                   cc_warn(syn_warn_unary_plus);
case s_and:
case s_times:   /* N.B. not s_qualified+s_times.                        */
case s_minus:   op = unaryop_(op);      /* drop through */
case s_plusplus:
case s_minusminus:
case s_bitnot:
case s_boolnot: nextsym();
                return mkunaryorop(op, rd_prefixexp(NOLABEL));
case s_typeof:  /* ncc extension keyword */
case s_sizeof:
        nextsym();
/* N.B. Ansi require sizeof to return an unsigned integer type */
        if (curlex.sym == s_lpar)
        {   nextsym();
            rd_scope_or_typedef(0);
            if (isdeclstarter2_(curlex)) /* rd_declspec(stgclass) moans */
            {   TypeExpr *t;
                if (curlex.sym == s_typestartsym) nextsym();
                t = rd_typename(TYPENAME);
                checkfor_ket(s_rpar);
                return mkintconst(
                    te_size_t,
                    op == s_sizeof ? cpp_sizeoftype(t) : codeoftype(t),
                    (Expr *)syn_list2(op == s_sizeof ? s_sizeoftype :
                                                       s_typeoftype,    t));
            }
            a = rd_expr(PASTCOMMA);
            checkfor_ket(s_rpar);
            a = rd_postfix(a);   /* for sizeof (f)() etc */
        }
        else a = rd_prefixexp(NOLABEL);
        return mkintconst(
            te_size_t,
            op == s_sizeof ? cpp_sizeoftype(typeofexpr(a)) :
                             codeoftype(typeofexpr(a)),
            (Expr *)syn_list2(op == s_sizeof ? s_sizeofexpr :
                                               s_typeofexpr,   a));
#ifdef CPLUSPLUS
/* @@@ following are only here temporarily...                           */
/* Maybe not, now we can lookahead.  However, we should rejig the       */
/* 'new' grammar, especially ::new.  (Jan 93: in progress.)             */
case s_new:    case s_qualified+s_new:
case s_delete: case s_qualified+s_delete:
        /* maybe s_qualified (by top-level only).                       */
        return rd_newordelete(op);
#endif
default:
        return rd_postfix(rd_primaryexp(labelhack));
    }
}

static Expr *addfilelinetoexpr_i(Expr *e, FileLine *fl)
{   if (e != NULL && hasfileline_(h0_(e))) {
        FileLine *flp = (FileLine *)SynAlloc(sizeof(FileLine));
        *flp = *fl;
        e->fileline = flp;
    }
    return e;
}

static Expr *addfilelinetoexpr(Expr *e, FileLine *fl)
{
    return addfilelinetoexpr_i(optimise0(e), fl);
}

static Expr *rd_exprwithfileline(int n, FileLine *fl) {
    Expr *e = rd_expr(n);
    e = addfilelinetoexpr_i(e, fl);
    return e;
}

static void NoteCurrentFileLine(FileLine *fl) {
    *fl = curlex.fl;
    fl->p = dbg_notefileline(*fl);
}

/* note: many calls to rd_expr() are followed by a checkfor_ket(s).     */
/* It would be nice if rd_expr_ket() could check for these.             */
/* (This would give syntactic "inserted s" messages BEFORE knock-on     */
/* semantic errs.  In the past we haven't done this because of          */
/* getting wrong line numbers for semantic errors/read-ahead.           */
/* Re-think this bit of the compiler.                                   */
static Expr *rd_expr(int n)
{   AEop op;
    Expr *a = rd_prefixexp(n);    /* see POSSLABEL */
    /* note that the loop does not go round if op == s_colon after a label */
    while (lprio_(op = curlex.sym) >= n)
    {   FileLine fl;
        nextsym();
        if (op == s_cond)
        {   Expr *b;
            NoteCurrentFileLine(&fl);
            b = rd_exprwithfileline(PASTCOMMA, &fl);
            checkfor_ket(s_colon);
            NoteCurrentFileLine(&fl);
            a = mkcond(a, b, rd_exprwithfileline(rprio_(op), &fl));
        } else if (op == s_comma || op == s_andand || op == s_oror) {
            NoteCurrentFileLine(&fl);
            a = mkbinaryorop(op, a, rd_exprwithfileline(rprio_(op), &fl));
        } else
            a = mkbinaryorop(op, a, rd_expr(rprio_(op)));
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
     {   cc_err(curlex.sym == s_eof ? syn_err_hashif_eof : syn_err_hashif_junk);
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
    {   cc_rerr(syn_rerr_insert_parens, op);
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
            {   case 1: cc_err(syn_err_initialisers);
                        break;
                case 2: cc_err(syn_err_initialisers1);
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
            cc_warn(syn_warn_spurious_braces);
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
    if (curlex.sym == s_lbrace)    /* nasty ANSI optional "int x = {1}" */
    {   if (flag == 1) return 0;   /* char x[] = {"abc"} illegal I presume */
        (void) syn_begin_agg();    /* always succeeds */
        e = rd_expr(UPTOCOMMA);
        if (t) e = mkcast(s_assign,e,t);
        if (curlex.sym != s_rbrace)
            checkfor_2ket(s_comma, s_rbrace);  /* @@@ improve w.r.t e.g int */
        syn_end_agg(flag == 4 ? 3 : 2);        /* special flags */
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
   if (syn_initdepth < 0 || curlex.sym == s_rbrace) return 0;
#ifndef CPLUSPLUS
/*
 * The next test is intended to make recovery from
 *    int x[] = { int y;
 * and similar cases at least a little better.
 * This is OK in C but not C++:  int a[] = { int(1),int(2)};
 * Putting this in stops some infinite loops in error recovery, but
 * the parser still gets pretty well messed up by the example shown above.
 * AM, Sept 91: lets have another go at initialiser error recovery.
 */
    rd_scope_or_typedef(0);
    if (isdeclstarter2_(curlex)) return 0;
#endif
   return 1;
}

/* command reading routines... */

static Cmd *rd_block(enum blk_flavour f)
{   DeclRhsList *d,*dp;
    CmdList  *c = NULL, *cq = NULL;
    int scope_level = 0;
    FileLine fl;
#ifdef CPLUSPLUS
    SynBindList *otemps = sem_reftemps;
    sem_reftemps = 0;
#endif
    NoteCurrentFileLine(&fl);
    if (f != blk_IMPLICIT) nextsym();        /* skip '{' if nec.     */
    if (f == blk_INNER) scope_level = push_scope(0);
    d = rd_decllist(BLOCKHEAD);
    for (dp = d; dp != 0; dp = dp->declcdr)  /* add dynamic inits    */
    {   Expr *e = declinit_(dp);             /* see genstaticparts() */
        if (e != 0)
        {   CmdList *ct = mkCmdList(0, mk_cmd_e(s_semicolon, dp->fileline, e));
            if (c == 0) c = cq = ct;
            else { cdr_(cq) = ct; cq = ct; }
        }
/* Note the SynBindList/DeclRhsList pun to re-use DeclRhsList store.   */
        ((SynBindList *)dp)->bindlistcar = dp->declbind;
    }
    while (curlex.sym != s_rbrace && curlex.sym != s_eof)
    {   CmdList *ct = mkCmdList(0, rd_command(1));
        if (curlex.sym == s_nothing) nextsym();
        if (c == 0) c = cq = ct;
        else { cdr_(cq) = ct; cq = ct; }
    }
    if (f != blk_IMPLICIT) checkfor_delimiter_ket(s_rbrace, "");
/* the next line MUST be executed, even if a syntax error occurs to
   avoid global/local storage inconsistency after error recovery.     */
    if (f == blk_INNER) pop_scope(scope_level);
    {   SynBindList *dd = (SynBindList *)d;
#ifdef CPLUSPLUS
        /* nconc sem_reftemps to end of decls 'd'.                    */
        if (dd == 0)
            dd = sem_reftemps;
        else
        {   SynBindList *p;
            for (p = dd; p->bindlistcdr; ) p = p->bindlistcdr;
            p->bindlistcdr = sem_reftemps;
        }
        sem_reftemps = otemps;
#endif
        return mk_cmd_block(fl, dd, c);
    }
}

static bool isexprstarter(AEop op)
/* implement as macro using bool array? */
{   switch (op & ~s_qualified)
    {   default: return 0;
        case s_lpar:    /* primary expression starters */
        case_s_any_string
        case s_integer: case s_floatcon:
        case s_identifier:
#ifdef CPLUSPLUS
        case s_pseudoid:
        /* rd_scope_or_typedef() swallowed s_coloncolon and s_operator. */
        case s_this:
        case s_new:  case s_delete:
#endif
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
     {   cc_err(syn_err_default); return 0; }
     if (switch_default_(cur_switch))
     {   cc_err(syn_err_default1); return 0; }
     return (switch_default_(cur_switch) = mk_cmd_default(fl, 0));
}

static Cmd *addcase(Expr *e, FileLine fl)
{    Cmd *p,*q; int32 n;
     if (cur_switch == 0)
     {   cc_err(syn_err_case);
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
             cc_err(syn_err_case1, (long)n);
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

/* rd_for_2() reads the last 2 parameters and the body of a for loop.   */
/* It is so factored for C++'s for(d;e;e)c and for(e;e;e)c styles.      */
static Cmd *rd_for_2(Expr *e1, FileLine fl)
{   Expr *e2, *e3; Cmd *c;
    if (curlex.sym == s_semicolon)
        e2 = 0;
    else {
         FileLine fl; NoteCurrentFileLine(&fl);
         e2 = addfilelinetoexpr(mktest(s_for, rd_expr(PASTCOMMA)), &fl);
    }
    checkfor_ket(s_semicolon);
    if (curlex.sym == s_rpar)
        e3 = 0;
    else {
         FileLine fl; NoteCurrentFileLine(&fl);
         e3 = addfilelinetoexpr(mkcast(s_for,rd_expr(PASTCOMMA),te_void), &fl);
    }
    checkfor_ket(s_rpar);
    {   Cmd *oldloop = cur_loop; AEop oldbreak = cur_break;
        cur_loop = c = mk_cmd_for(fl, e1,e2,e3,0);
        cur_break = s_break;
        cmd4c_(c) = rd_command(0);
        cur_loop = oldloop; cur_break = oldbreak;
        return c;
    }
}

static Cmd *rd_command(bool declposs)
{
#ifdef CPLUSPLUS
  rd_scope_or_typedef(0);
  if (declposs && isdeclstarter2_(curlex))
      return rd_block(blk_IMPLICIT);
  else
#endif
  {
    AEop op;
    Cmd *c;
    Expr *e;
    FileLine fl; NoteCurrentFileLine(&fl);     switch (op = curlex.sym)
    {
case s_default:
        nextsym();
        checkfor_ket(s_colon);
        if ((c = adddefault(fl)) == 0)
            return rd_command(declposs); /* error */
        cmd1c_(c) = rd_command(declposs);
        return c;
case s_case:
        nextsym();
        e = optimise0(
            mkcast(s_case, rd_expr(PASTCOMMA), cur_switchtype));
        checkfor_ket(s_colon);
        if (e==0 || (c = addcase(e,fl)) == 0)
            return rd_command(declposs); /* error */
        cmd2c_(c) = rd_command(declposs);
        return c;
default:
        if (!isexprstarter(op))
        {   cc_err(syn_err_expected_cmd);
            while (curlex.sym!=s_lbrace && curlex.sym!=s_rbrace &&
                   curlex.sym!=s_semicolon && curlex.sym!=s_eof) nextsym();
            return 0;
        }
        e = rd_expr(POSSLABEL);
        if (h0_(e) == s_colon)
        {   Symstr *a = (Symstr *)type_(e);   /* also in curlex.a1.sv */
            LabBind *lab = label_define(a);
            nextsym();                      /* not previously done */
            if (lab == 0)
                return rd_command(declposs);  /* duplicate    */
            return mk_cmd_lab(s_colon, fl, lab, rd_command(declposs));
        }
        /* @@@ perhaps we should check the ';' first */
        e = optimise0(mkcast(s_semicolon, e, te_void));
        c = (e==0) ? 0 : mk_cmd_e(s_semicolon, fl, e);
        break;
case s_semicolon:
        c = 0;
        break;
case s_lbrace:
        c = rd_block(blk_INNER);
        elselast = 0;
        return c;
case s_do:
        nextsym();
        {   Cmd *oldloop = cur_loop; AEop oldbreak = cur_break;
            cur_loop = c = mk_cmd_do(fl, 0, 0);   /* amalgamate with for */
            cur_break = s_break;
            cmd1c_(c) = rd_command(0);
            cur_loop = oldloop; cur_break = oldbreak;
        }
        if (curlex.sym == s_nothing) nextsym();
        if (curlex.sym == s_while)
        {   FileLine fl;
            nextsym();
            NoteCurrentFileLine(&fl);
            e = addfilelinetoexpr(rd_condition(s_while), &fl);
        }
        else
        {   cc_err(syn_err_expected_while);
            e = 0;      /* treat like "do c while;" (missing e).      */
        }
        if (e == 0) e = mkintconst(te_int,0,0); /* error => while(0). */
        cmd2e_(c) = e;
        break;
case s_while:
        nextsym();
        {   FileLine fl; NoteCurrentFileLine(&fl);
            e = addfilelinetoexpr(rd_condition(op), &fl);
        }
        {   Cmd *oldloop = cur_loop; AEop oldbreak = cur_break;
            cur_loop = c = mk_cmd_for(fl, 0, e, 0, 0);
            cur_break = s_break;
            cmd4c_(c) = rd_command(0);
            cur_loop = oldloop; cur_break = oldbreak;
            return c;
        }
case s_for:
        nextsym();
        checkfor_ket(s_lpar);
#ifdef CPLUSPLUS
        rd_scope_or_typedef(0);
        if (isdeclstarter2_(curlex))
            return rd_cplusplus_for(fl);
        else
#endif
        {   e = ((curlex.sym == s_semicolon) ? 0 :
                optimise0(mkcast(s_for,rd_expr(PASTCOMMA),te_void)));
            checkfor_ket(s_semicolon);
            return rd_for_2(e, fl);
        }
case s_if:
        nextsym();
        e = optimise0(rd_condition(op));
        if (e == 0) e = mkintconst(te_int,0,0); /* error => if(0).    */
        c = rd_command(0);
        if (curlex.sym == s_nothing) nextsym();
        {   Cmd *c2;
            static int elseline = 0;
            if (curlex.sym == s_else)
            {   elseline = curlex.fl.l;
                nextsym();
                c2 = rd_command(0);
                elselast = 1;
            }
            else
            {   c2 = 0;
                if (elselast) /* in 'c' above */
                {   int temp = curlex.fl.l;
                    curlex.fl.l = elseline;
/* The construction applied here seems a little bit nasty - it would    */
/* certainly give trouble if diagnostics were displayed with an echo of */
/* part of the surrounding source. Still, it seems a good idea!         */
                    cc_warn(syn_warn_dangling_else);
                    curlex.fl.l = temp;
                }
            }
            return mk_cmd_if(fl, e, c, c2);
        }
case s_else:
        cc_err(syn_err_else);
        nextsym();
        return rd_command(0);
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
            cmd2c_(c) = rd_command(0);
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
            cc_rerr(syn_rerr_return);
        if (e == 0 && !implicit_return_ok && !equivtype(cur_restype, te_void))
            cc_warn(syn_warn_void_return);
        c = mk_cmd_e(op, fl, e);
        break;
#ifdef EXTENSION_VALOF
case s_resultis:
        nextsym();
        e = optimise0(
            mkcast(s_return,
                rd_expr(PASTCOMMA), valof_block_result_type));
        c = mk_cmd_e(op, fl, e);
        break;
#endif
case s_continue:
        if (cur_loop != 0) c = mk_cmd_0(op, fl);  /* syn_list2(op,cur_loop)? */
        else { c=0; cc_err(syn_err_continue); }
        nextsym();
        break;
case s_break:
        if (cur_loop != 0 || cur_switch != 0) c = mk_cmd_0(cur_break, fl);
        else { c=0; cc_err(syn_err_break); }
        nextsym();
        break;
case s_goto:
        nextsym();
        if (curlex.sym != s_identifier)
        {   cc_err(syn_err_no_label);
            return rd_command(declposs);
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
}

static Cmd *rd_body(TypeExpr *t)
{   cur_switch = 0;
    cur_loop = 0;
    cur_break = s_error;
    cur_restype = t;
    cur_switchtype = te_lint;
    if (curlex.sym == s_lbrace)
        return rd_block(blk_BODY);  /* share scope with args...     */
    else
    {   cc_err(syn_err_no_brace);
        return 0;
    }
}

typedef struct DeclFnAux {
    int32 flags;
    int32 val;
} DeclFnAux;

typedef struct DeclSpec {
    TypeSpec *t;
    SET_BITMAP stg;
    int32 stgval;
    DeclFnAux fnaux;
/* private flags for C/C++ for 'omitted type' for non-fn etc.           */
#define B_OMITTYPE 1
#define B_DECLMADE 2
    int synflags;
} DeclSpec;

#ifdef CPLUSPLUS
enum LinkSort { LINK_C, LINK_CPP };
typedef struct Linkage {
    struct Linkage *linkcdr;
    enum LinkSort linkcar;
} Linkage;
static Linkage *syn_linkage, *syn_linkage_free;

#define syn_current_linkage() \
    (syn_linkage == 0 ? LINK_CPP : syn_linkage->linkcar)

static void syn_pop_linkage()
{   Linkage *p = syn_linkage;
    if (p == 0) syserr("pop_linkage(0)");
    syn_linkage = p->linkcdr;
    p->linkcdr = syn_linkage_free;
    syn_linkage_free = p;
}

static int cmpANSIstring(String *s, char *t)
{   /* @@@ beware: treats concatenation as mismatch currently!          */
    StringSegList *z = s->strseg;
    return (h0_(s) == s_string && z->strsegcdr == 0
                               && z->strseglen == strlen(t))
             ? memcmp(t, z->strsegbase, (size_t)z->strseglen) : 99;
}

static enum LinkSort linkage_of_string(String *s)
{   static struct { char lang[4]; enum LinkSort code; } valid[] =
      { "C++", LINK_CPP, "C", LINK_C };
    int i;
    for (i = 0; i < sizeof valid/sizeof valid[0]; i++)
        if (cmpANSIstring((String *)s, valid[i].lang) == 0)
            return valid[i].code;
    cc_rerr("unknown linkage: extern $e", s);
    return LINK_CPP;
}

/* LINKAGESPEC (private to syn.c) flags empty decl 'extern "xx" {'.     */
#define LINKAGESPEC (bitoftype_(s_struct)|bitoftype_(s_union))
#endif

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
/* Oct92: soon add *optional* "long long" ==> s_longlong.               */
static DeclSpec rd_declspec(int declflag)
{
    SET_BITMAP illtype = 0, typesseen = 0, typedefquals = 0;
    int synflags = 0;
    Binder *b = 0;  /* typedef or struct/union/enum tag binder record */
    SET_BITMAP illstg, stgseen = 0;
    int32 stgval = 0;
    int32 auxseen = 0, auxval = 0; /* fnaux->val */
    bool structspec = NO;
    if (declflag & POINTERTYPES)
        illtype = ~CVBITS;      /* only const/volatile acceptable.      */
    illstg = declflag & TOPLEVEL ?
                 (bitofstg_(s_register)|bitofstg_(s_auto)|
                  bitofstg_(s_virtual)|bitofstg_(s_friend)) :
             declflag & MEMBER ?
                 (bitofstg_(s_register)|bitofstg_(s_auto)|
                  bitofstg_(s_extern)|bitofstg_(s_weak)) :
             declflag & BLOCKHEAD ?
                 (bitofstg_(s_virtual)|bitofstg_(s_friend)) :
             declflag & (FORMAL|ARG_TYPES) ?
                 (STGBITS & ~bitoftype_(s_register)) : STGBITS;
    for (;;)
    {   AEop s = curlex.sym;
        if (!isdeclstarter_(s))
        {   /* A typedef may be possible, else break from loop...       */
            if (illtype & ~CVBITS) break;
            rd_scope_or_typedef(0);
            if (!((curlex.sym & ~s_qualified) == s_identifier &&
                  curlex_typedef != 0)) break;
            s = s_typedefname, b = curlex_typedef;
            typedefquals = qualifiersoftype(bindtype_(b)),
            binduses_(b) |= u_referenced;
        }
#ifdef CPLUSPLUSnever           /* didn't seem to work, do elsehow! */
/* @@@ s_member return? */
        else if ((b = findbinding(curlex.a1.sv)) != 0 &&
                 (bindsym_(b) == ctorsym || bindsym_(b) == dtorsym))
        {   curlex.a1.sv = bindsym_(b);        /* @@@ nasty! */
            if ((stgseen & ~bitofstg_(s_inline)) || typesseen)
                cc_rerr(
"storage class/type specifiers forbidden with constructor or destructor");
/* @@@ merge with access-spec code.                                     */
            return te_int;
        }
#endif
        if (isstorageclass_(s))
        {   SET_BITMAP stgbit = bitofstg_(s);
            if (s == s_globalreg || s == s_globalfreg) {
                nextsym();
                checkfor_ket(s_lpar);
                stgval = evaluate(rd_expr(PASTCOMMA)) << 1;
                checkfor_ket(s_rpar);
                stgbit = b_globalregvar;
                if (s == s_globalfreg) stgval |= 1;
            } else
                nextsym();
            if (stgbit & illstg)
                cc_err(syn_err_stgclass, /* storage class illegal here */
                        s, ctxtofdeclflag(declflag));
            else if (stgbit & stgseen ||
                       stgbit & (PRINCSTGBITS|bitofstg_(s_register)) &&
                       stgseen & (PRINCSTGBITS|bitofstg_(s_register)))
                cc_err(syn_err_stgclass1, /* storage class incompatible */
                         s, stgseen);
            else
            {   stgseen |= stgbit;
                if (s == s_weak) stgseen |= bitofstg_(s_extern);
/* @@@ beware -- this leads to 'extern incompatible with extern'.       */
/* we should do this defaulting later, but to disallow '__weak auto'    */
/* we need a separate illstg2 bit (working like illtype).               */
            }
        } else if (isfnaux_(s)) {
            auxseen |= bitoffnaux_(s);
            nextsym();
            if (s == s_swi || s == s_swi_i) {
                if (s == s_swi_i)
                    auxseen |= bitoffnaux_(s_swi) | f_specialargreg;
                checkfor_ket(s_lpar);
                auxval = evaluate(rd_expr(PASTCOMMA));
                checkfor_ket(s_rpar);
            }
        } else {
            SET_BITMAP typebit = bitoftype_(s);
            bool trouble = (illtype & typebit) != 0;
            if (trouble)
            {   /* this code relies that for type symbols x,y, "x y" is
                   legal iff "y x" is. */
                cc_err(syn_err_typeclash, s,
                       typesseen & illtypecombination[shiftoftype_(s)]);
/* New error recovery: ignore previous types/stgclass and retry.        */
/* This is better for things like "class T {} int f() {...}".           */
/* Note that we aren't just inserting a ';', e.g. sizeof(struct{}int)   */
/* and also that 'struct {} typedef const int' splits AFTER the const.  */
/* remove var 'trouble' if we keep this code...                         */
                return rd_declspec(declflag);
            }
            else
            {
#ifdef CPLUSPLUS /* LDS 27-Oct-92 Review here - also for C ?? */
                if (declflag & SIMPLETYPE && (typesseen || typebit &
                      (CVBITS|ENUMORCLASSBITS)))
                    cc_err("illegal <simple type>: $m", typesseen | typebit);
#endif
                typesseen |= typebit;
                illtype |= illtypecombination[shiftoftype_(s)];
            }
            nextsym();
            if (typebit & ENUMORCLASSBITS)
            {   TagBinder *b2;
                bool sawid = 0;
/* Now, after "struct id" a ';' or '{' indicates a new definition at */
/* the current nesting level, but not in type names, e.g. C++        */
/*        struct T *p = new struct T;                                */
                if (curlex.sym == s_identifier)
                {   Symstr *sv = curlex.a1.sv;
                    TagDefSort defining;
                    sawid = 1;
                    nextsym();
                    defining = (curlex.sym == s_semicolon &&
                                declflag & CONC_DECLARATOR) ? TD_Decl :
#ifdef CPLUSPLUS
                                curlex.sym == s_colon ||
#endif
                                curlex.sym == s_lbrace      ? TD_ContentDef
                                                            : TD_NotDef;
                    b2 = instate_tagbinding(sv, s, defining, bind_scope);
#ifdef CPLUSPLUS
/* @@@ check ansi re things like 'struct a; int a; struct a {}' etc.    */
                    if (defining != TD_NotDef)
                        syn_classname_typedef(primtype2_(bitoftype_(s),b2),
                            b2, bind_scope);
#endif
                    if (curlex.sym == s_semicolon)
                        /* @@@ what about above CONC_DECLARATOR?        */
                        attributes_(b2) &= ~TB_BEINGDEFD;
                    if (s == s_enum) {
                        if (defining == TD_ContentDef)
                            synflags |= B_DECLMADE;
                        else if (!(attributes_(b2) & TB_DEFD))
                            cc_rerr(syn_err_undef_enum, sv);
                    } else if (defining != TD_NotDef)
                        synflags |= B_DECLMADE;  /* police 'int;' error */
                }
                else b2 = instate_tagbinding(0, s, TD_ContentDef, bind_scope);
                        /* anonymous */
                if (curlex.sym == s_lbrace
#ifdef CPLUSPLUS
                 || curlex.sym == s_colon
#endif
                )
                {   bool has_bases = (curlex.sym == s_colon);
                    nextsym();
                    /* avoid spurious (C++?) warning for enum { x,y };  */
                    if (s==s_enum)
                    {   synflags |= B_DECLMADE;
                        settagmems(b2,
/* beware the next line.  We forge a type for recovery in the case
   of "short enum {...}".  However, this means that possible
   const/volatile are not put on enumeration constants.  This
   doesn't really matter since constants are constant anyway!!!
*/
                            rd_enumdecl(primtype2_(typebit,b2)));
                    }
                    else
                        /* @@@ but result taken from tagbindmems_()!!   */
                        tagbindmems_(b2) = rd_classdecl(b2, has_bases);
                    checkfor_ket(s_rbrace);
                    structspec = YES;
                }
                else if (!sawid)
                {   cc_err(syn_err_tag_brace, s);
                    /* recovers for 'struct *a' or suchlike error. */
                }
                if (!trouble) b = (Binder *)b2;
            }
        }
    }
#ifdef CPLUSPLUS
    if (declflag & TOPLEVEL && isstring_(curlex.sym) &&
        typesseen == 0 && stgseen == bitofstg_(s_extern) && auxseen == 0)
    {   Expr *s = rd_ANSIstring();
        Linkage *p = syn_linkage_free;
        TypeSpec *t;
        if (p == 0) p = (Linkage *)GlobAlloc(SU_Other, sizeof(Linkage));
        else syn_linkage_free = p->linkcdr;
        p->linkcar = linkage_of_string((String *)s);
        p->linkcdr = syn_linkage;
        syn_linkage = p;
        if (curlex.sym == s_lbrace) nextsym();
        /* else the 'extern "C" typedef int foo();' form.               */
        /* Alternative implementation: retry rd_declspec and then       */
        /* pop implicitly at terminating ';' or '}'?                    */
        {   DeclSpec ds;
            ds.t = primtype_(LINKAGESPEC);
            ds.stg = stgseen; ds.stgval = stgval;
            ds.fnaux.flags = 0;
            ds.synflags = 0;
            return ds;
        }
    }
#endif
    if (typedefquals & typesseen)
        cc_rerr(syn_rerr_qualified_typedef(b, typedefquals & typesseen));
    /* could warn here in C (not C++) for undefined const <fntypedef>.  */
    if ((declflag & TYPE_NEEDED) && typesseen == 0)
        cc_rerr(syn_rerr_missing_type);
    else if (!(declflag & FORMAL) && (typesseen|stgseen)==0)
        synflags |= B_OMITTYPE;

    if ((typesseen & NONQUALTYPEBITS) == 0)
    {   if ((typesseen|stgseen) != 0 || !(declflag & FORMAL))
            /* consider returning 0 for untyped formals?
               changes would be need for several routines, so pend */
            typesseen |= bitoftype_(s_int);
    }

    if (typesseen & bitoftype_(s_float))    /* normalise for rest of system */
    {   typesseen ^= (bitoftype_(s_float) ^ bitoftype_(s_double));
        if (typesseen & bitoftype_(s_long))
        {   if (!(suppress & D_LONGFLOAT) && !(feature & FEATURE_PCC))
             { suppress |= D_LONGFLOAT;
               cc_rerr(syn_rerr_long_float);
             }
            typesseen &= ~bitoftype_(s_long);
        }
        else
            typesseen |= bitoftype_(s_short);  /* float => short double */
    }
    if (typesseen & bitoftype_(s_longlong)) /* normalise for rest of system */
    {   /* map type "long long" into "long short"...                    */
        /* Storing types as bit-maps must be up for review.             */
        typesseen ^= bitoftype_(s_longlong) ^ bitoftype_(s_int) ^
                     (bitoftype_(s_long) | bitoftype_(s_short));
    }

    if (stgseen & bitofstg_(s_register))   /* normalise for rest of system */
        stgseen |= bitofstg_(s_auto);
#ifndef NON_CODEMIST_MIDDLE_END
    if (declflag & TOPLEVEL) {
        if (global_intreg_var > 0)
            stgseen &= ~(bitofstg_(s_static)|bitofstg_(s_extern)),
            stgseen |= b_globalregvar,
            stgval = global_intreg_var << 1;
        else if (global_floatreg_var > 0)
            stgseen &= ~(bitofstg_(s_static)|bitofstg_(s_extern)),
            stgseen |= b_globalregvar,
            stgval = (global_floatreg_var << 1) | 1;
    }
#endif
    {   DeclSpec ds;
        TypeSpec *t = primtype2_(typesseen, b);
        if (structspec && !(suppress & D_STRUCTPADDING)) {
            bool padded = NO;
            (void)sizeoftypenotepadding(t, &padded);
            if (padded) cc_warn(syn_warn_struct_padded, b);
        }
        ds.t = t;
        ds.stg = stgseen;
        ds.stgval = stgval;
        ds.fnaux.flags = auxseen,
        ds.fnaux.val = auxval,
        ds.synflags = synflags;
        return ds;
    }
}

/* the following macro checks for an arg which has not been typed */
#define is_untyped_(x) (h0_(x) == s_typespec && typespecmap_(x) == 0)

typedef TypeExpr Declarator;   /* funny inside-out Expr */
#define sub_declarator_(x) ((x)->typearg)

/* Declarators: read these backwards (inside out) to yield a name
 * (omitted if abstract declarator) and a TypeExpression (a la Algol68).
 * Then the declarator AE structure is read and an in-place pointer reverse
 * is done to turn a 'basictype' (possibly typedef) and a declarator into
 * a declaree (identifier or empty) in 'declarator_name' and TypeExpr.
 * For C++ we need to allow things like "int a(3);".  This is currently
 * done by an s_init Declarator node (local to rd_declarator()) and
 * declarator_init.  'inner' forbids (e.g.) "int (a(3))" etc.
 */

static Declarator *rd_formals_1(Declarator *a, const DeclFnAux *fnaux);

static Declarator *rd_declarator_postfix(Declarator *a,
            int declflag, const DeclFnAux *fnaux, bool inner)
{   for (;;) switch (curlex.sym)
    {
case s_lpar:
        if (declflag & (NEW_TYPENAME|CONVERSIONTYPE)) return a;
        nextsym();
#ifdef CPLUSPLUS
            if (bind_scope & TOPLEVEL)   /* @@@ nasty usage here... */
/* beware: found_in_class is no longer set properly!.                   */
                set_access_context(found_in_class, 0);
#else
        IGNORE(inner);
#endif
#ifdef CPLUSPLUS        /* allow int a(3) etc, see isdeclstarter2x_().  */
        rd_scope_or_typedef(0);
        if (!inner && declflag & (TOPLEVEL|BLOCKHEAD) &&
              !(curlex.sym == s_rpar || curlex.sym == s_ellipsis ||
                isdeclstarter2x_(curlex,declflag)))
            return mk_typeexpr1(s_init, a, 0);

        else
#endif
            a = rd_formals_1(a, fnaux);
        checkfor_ket(s_rpar);
        break;
case s_lbracket:
        if (declflag & CONVERSIONTYPE) return a;
        nextsym();
        {   Expr *e = 0;
            if (curlex.sym != s_rbracket)
            {   e = rd_expr(PASTCOMMA);
/* The next line allows  new(int [n][6])  with 'n' a variable for C++.  */
                if (!(declflag & (NEW_TYPENAME|FLEX_TYPENAME) &&
                      h0_(a) == s_nothing)) e = check_arraysize(e);
            }
            a = mk_typeexpr1(t_subscript, a, e);
        }
        checkfor_ket(s_rbracket);
        break;
default:
        return a;
    }
}

/* AM: next edit the following becomes a TypeFnAux* arg to rd_formals. */
static int32 syn_minformals, syn_maxformals;
static bool syn_oldeformals;
                       /* set by rd_formals, read by rd_declarator_1() */
static Symstr *declarator_name;
                       /* set by rd_declarator: 0 or Symstr *          */
static SET_BITMAP declarator_memfn;
                       /* set by rd_declarator: 0/b_memfna/b_memfns     */
                       /* (declarator_name could become a Binder *.)    */
static bool declarator_init;    /* ditto */


/* rd_declarator reads a C declarator (q.v.).  We test declflag to see  */
/* whether CONC_DECLARATOR or ABS_DECLARATOR's (or both) are            */
/* acceptable, also TOPLEVEL (which allows old 'f(a,b)' non-prototype   */
/* form).  In C++ declflag is tested more.                              */
static Declarator *rd_declarator_1(int declflag, TagBinder *mem_scope,
        const DeclFnAux *fnaux, bool inner)
{
    static Declarator emptydeclarator = { s_nothing };  /* @@@ kill? */
    static Declarator errordeclarator = { s_error };
    Declarator *a; AEop op;
    rd_scope_or_typedef(mem_scope);
    op = curlex.sym & ~s_qualified;
#ifdef CPLUSPLUS
    if (declflag & CONVERSIONTYPE && !(op == s_times || op == s_and))
        /* CONVERSIONTYPE declarators include *, &, S::* types only.    */
        op = s_nothing;
#endif
    switch (op)
    {
default:
        if (declflag & CONC_DECLARATOR)
        {   cc_err(syn_err_expected3);
            if (curlex.sym == s_rbrace && !(declflag & TOPLEVEL)) nextsym();
            return &errordeclarator;
        }
        a = &emptydeclarator;
        break;
#ifdef CPLUSPLUS
case s_pseudoid:
#endif
case s_identifier:
        if (declflag & ABS_DECLARATOR)
        {   /* @@@ improve next error message                           */
            cc_err(syn_err_unneeded_id);
            a = &emptydeclarator;
        }
        else
        {   a =
#ifdef CPLUSPLUS
                curlex_qbind ? (Declarator *) curlex_qbind :
#endif
                               (Declarator *) curlex.a1.sv;
            /* Symstr(s_id) is Declarator (or now s_member/s_binder).   */
            if (a == 0 ||
                  h0_(a) != s_identifier &&
                  h0_(a) != s_binder &&
                  h0_(a) != s_member) syserr("rd_member_name=0");
#ifdef CPLUSPLUS
            if (declflag & MEMBER && curlex_optype)
                a = (Declarator *)syn_list3(s_convfn, a, curlex_optype);
/* @@@ we should put the s_coloncolon in here instead of PUSH?          */
#endif
        }
        nextsym();
        break;
case s_lpar:
        if (declflag & NEW_TYPENAME) return &emptydeclarator;
/* Note that "int ()" means an abstract declarator (or nameless formal) */
/* of the type of "int x()", not "int (x)".                             */
/* Similarly "int (void)" or "int (typedefname)" represent fn-typenames */
/* @@@ However, the ANSI draft (Dec 88) has an ambiguity in             */
/* "typedef int t; void f(int (t));" as to whether the formal to f is   */
/* of type "int" and name "t" or nameless and of type "int (int)".      */
/* We choose to select the latter (this seems to be the ANSI ctte's     */
/* intent from the example "t f(t (t))" in section 3.5.6 (dec 88)).     */
        { DeclFnAux fnaux;
          fnaux.flags = (fpregargs_disabled ? f_nofpregargs : 0L) |
                          (no_side_effects ? bitoffnaux_(s_pure) : 0L);
          nextsym();
          rd_scope_or_typedef(mem_scope);
/* @@@ for C++ a form of ungetsym() might be an idea here so that       */
/* rd_declarator_postfix can parse the '(', or maybe we need to parse   */
/* ahead as 'either'.  See other calls to rd_formals_1.                 */
          if ((declflag & (ABS_DECLARATOR|FORMAL)) &&
              (curlex.sym == s_rpar || curlex.sym == s_ellipsis ||
               isdeclstarter2_(curlex)))
          {   a = rd_formals_1(&emptydeclarator, &fnaux);
          }
          else
          {   a = rd_declarator_1(declflag, mem_scope, &fnaux, 1);
              if (h0_(a) == s_error) return a;
          }
        }
        checkfor_ket(s_rpar);
        break;
case s_times:
#ifdef CPLUSPLUS
case s_and:
#endif
        {   AEop typeop = (curlex.sym & ~s_qualified) == s_times ?
                              t_content : t_ref;
#ifdef CPLUSPLUS
            TagBinder *tb = curlex.sym & s_qualified ? curlex_scope : 0;
/* @@@ What is curlex_scope/mem_scope relation?                         */
            Declarator *aa;
#endif
            DeclSpec ds;
            nextsym();
            ds = rd_declspec(POINTERTYPES);
            a = rd_declarator_1(declflag, mem_scope, fnaux, inner);
            if (h0_(a) == s_error) return a;
#ifdef CPLUSPLUS
/* move s_init to outermost level, suppress rd_declarator_postfix().    */
            if (h0_(aa = a) == s_init) a = sub_declarator_(a);
#endif
            /* the next line masks off bitoftype_(s_int) etc.           */
            a = mk_typeexpr1(typeop, a, (Expr *)(typespecmap_(ds.t) & CVBITS));
#ifdef CPLUSPLUS
            if (tb) a = (Declarator *)syn_list3(t_coloncolon, a, tb);
            if (h0_(aa) == s_init) { sub_declarator_(aa) = a; return aa; }
#endif
        }
        break;
    }
    return rd_declarator_postfix(a, declflag, fnaux, inner);
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
        if (m & CLASSBITS)
        {   TagBinder *b = typespectagbind_(t);
            /* Much of the compiler can support undefined structs until */
            /* first essential use, but ANSI ban.                       */
            if (attributes_(b) & TB_BEINGDEFD)
            {   cc_err(syn_err_selfdef_struct(member,b,name));
                goto fixup;
            }
            if (!(attributes_(b) & TB_DEFD) &&
                    !(stg & bitoftype_(s_extern)))
            {   cc_err(syn_err_undef_struct(member,b,name));
                goto fixup;
            }
        }
        if (m & bitoftype_(s_void))
        {   cc_err(syn_err_void_object(member,name));
            goto fixup;
        }
    }
    /* @@@ pick up more [] cases (like auto, but not extern) below?     */
    if (h0_(t) == t_subscript && typesubsize_(t) == 0 && member)
    {   if (!(suppress & D_ZEROARRAY)) {
            if (suppress & D_MPWCOMPATIBLE)
                cc_warn(syn_rerr_open_member, name);
            else
                cc_pccwarn(syn_rerr_open_member, name);
        }
        /* ANSI ban open array members, pcc treats as [0].              */
        typesubsize_(t) = globalize_int(0);
    }
    if (h0_(t) == t_fnap && (
#ifdef CPLUSPLUS
            !member &&
#else
            member ||
#endif
            !(stg & (bitofstg_(s_extern)|bitofstg_(s_static)))))
    {   /* Beware: next line was syn_pccwarn, but AM sees PCC error.    */
        cc_rerr(syn_rerr_fn_ptr(member,name));
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
#ifdef CPLUSPLUS
/* fault ref to void.                                                   */
    if (h0_(t) == t_ref && isprimtype_(pa, s_void))
    {   /* treat as ref to int (better ideas?)                          */
        cc_rerr(syn_rerr_ref_void);
        typearg_(t) = te_int;
        return t;
    }
/* fault array of ref, pointer to ref and ref to ref.                   */
    if (h0_(pa) == t_ref &&
         (h0_(t) == t_subscript || h0_(t) == t_content || h0_(t) == t_ref))
    {   /* just forget the ref.                                         */
        cc_rerr(syn_rerr_ill_ref, t);
        typearg_(t) = typearg_(pa);
        return t;
    }
#endif
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
    typearg_(t) = mk_typeexpr1(t_content, a, 0);
    return t;
}

#ifdef CPLUSPLUS
static Binder *ovld_match_def(Binder *b, TypeExpr *t, BindList *l)
{   for (; l != 0; l=l->bindlistcdr)
    {   Binder *bb = l->bindlistcar;
        if (equivtype(bindtype_(bb), t)) return bb;
    }
    cc_err("no $b declaration at this type", b);
    return 0;
}
#endif

/* rd_declarator() returns 0 in the event that it failed to find a
   declarator.  Note that this can only happen if 'CONC_DECLARATOR' is set.
   The caller is now responsible for faulting declarations of 'incomplete
   types' (see ansi).
*/
static TypeExpr *rd_declarator(int declflag, TypeExpr *basictype,
        const DeclFnAux *fnaux)
{   TagBinder *mem_scope = 0;
    Declarator *x;
    bool init = 0;
#ifdef CPLUSPLUS
    mem_scope = current_member_scope();
#endif
    x = rd_declarator_1(declflag, mem_scope, fnaux, 0);
#ifdef CPLUSPLUS
    if (h0_(x) == s_init)
    {   init = 1;
        x = sub_declarator_(x);
    }
#endif
    for (;;) switch (h0_(x))
    {   case s_error: return 0; /* error from rd_declarator_1()         */
        default: syserr(syserr_rd_declarator, (long)h0_(x));
            /* drop through */
        case s_nothing:
            declarator_memfn = 0; declarator_init = init;
            declarator_name = 0;
            return basictype;
#ifdef CPLUSPLUS
        case s_binder:
            declarator_memfn = 0; declarator_init = init;
            {   Binder *b = (Binder *)x;
                TypeExpr *t = bindtype_(b);
                Symstr *sv = bindsym_(b);
                if (sv == ctorsym || sv == dtorsym)
                    basictype = fixup_special_member(sv, basictype,
                                                     te_int, mem_scope);
                if (bindstg_(b) & b_pseudonym)
                {   declarator_name = bindsym_(realbinder_(b));
                    return basictype;
                }
                if (h0_(t) == t_ovld)
                {   b = ovld_match_def(b, basictype, typeovldlist_(t));
                    if (b == 0)
                        goto notmember; /* ovld_match_def() gave msg.   */
                    if (!(bindstg_(b) & b_pseudonym))
                        syserr("rd_declarator $b", b);
                    b = realbinder_(b);
                    declarator_memfn = bindstg_(b) & (b_memfna|b_memfns);
                    declarator_name = bindsym_(b);
                    return basictype;
                }
            }
            /* drop through */
        case s_member:
            cc_err("legal only for member function or static member: $b",
                   (Binder *)x);
        notmember:
            declarator_memfn = 0; declarator_init = init;
            declarator_name = bindsym_((Binder *)x);
            return basictype;
#endif
        case s_identifier:
            declarator_memfn = 0; declarator_init = init;
            declarator_name = (Symstr *)x;
#ifdef CPLUSPLUS
            if (declarator_name == ctorsym || declarator_name == dtorsym)
                basictype = fixup_special_member(declarator_name, basictype,
                                        te_int, mem_scope);
#endif
            return basictype;
#ifdef CPLUSPLUS
        case s_convfn:
            declarator_memfn = 0; declarator_init = init;
            declarator_name = (Symstr *)sub_declarator_(x);
            return fixup_special_member(declarator_name, basictype,
                                        (TypeExpr *)x->typespecbind, mem_scope);
#endif
        case t_fnap:
            {   Declarator *y = sub_declarator_(x);
                if (!(h0_(y) == s_identifier && (declflag & TOPLEVEL)))
                    /* all cases except outermost () of TOPLEVEL declarator */
                    ensure_formals_typed(typefnargs1_((TypeExpr *)x), 1);
            }
            /* drop through */
        case t_ref:
        case t_content:
        case t_subscript:
#ifdef CPLUSPLUS
        case t_coloncolon:
#endif
            {   Declarator *temp = sub_declarator_(x);
                if (is_untyped_(basictype))  /* e.g. f(int a,*b) */
                {   cc_rerr(syn_rerr_missing_type1);
                    basictype = te_int;
                }
                sub_declarator_(x) = (Declarator *)basictype;
                basictype = fault_odd_fn_array((TypeExpr *)x);
                x = temp;
                break;
            }
    }
}

static TypeExpr *rd_typename(int declflag)
{   DeclSpec ds;
    ds = rd_declspec(declflag);
                                 /*  TYPE_NEEDED and ~STGCLASS_OK       */
    if (!(declflag & SIMPLETYPE))
        return rd_declarator(declflag, ds.t, &ds.fnaux);
    return ds.t;
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
    {   SET_BITMAP s = d->declstg;
        TypeExpr *t = d->decltype;
#ifdef CPLUSPLUS
        if (!(declflag & MEMBER) && s & bitofstg_(s_inline))
            /* is this right for BLOCKHEAD? */
            s |= s_static;
#endif
        if (declflag & (BLOCKHEAD|FORMAL|ARG_TYPES))
            /* of course, by now there are no arg fns (mapped to ptrs). */
            s |= isfntype(t) ? bitofstg_(s_extern)
                             : bitofstg_(s_auto);
        else if (declflag & TOPLEVEL)
            s |= isfntype(t) ? bitofstg_(s_extern) :
#ifdef CPLUSPLUS
                 qualifiersoftype(t) & bitoftype_(s_const) ?
                               bitofstg_(s_static) :
#endif
                               bitofstg_(s_extern)|b_omitextern;
        else syserr(syserr_defaultstgclass, (int)declflag);
        d->declstg = s;
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
                cc_rerr(syn_rerr_missing_type2, d->declname);
            else
              if (!(feature & FEATURE_PCC))
                  /* God knows why ANSI do not consider this an error */
                  cc_warn(syn_warn_undeclared_parm, d->declname);

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
/* The caller can indicate that () is considered empty id-list with     */
/* map=0, or empty decl-list with map=1 (e.g. for ellipsis).            */
    return map & 1;
}

static void merge_formal_decl(DeclRhsList *d, DeclRhsList *dnew)
{
/* Here I have a formal parameter (d) which is now being given an        */
/* explicit declaration (dnew).  Merge new information in with the old.  */
    TypeExpr *t = d->decltype;
    if (!is_untyped_(t))  /* previous type info */
        cc_err(syn_err_duplicate_type, d->declname);
    d->declstg = dnew->declstg;
    d->decltype = dnew->decltype;
}

#ifdef CPLUSPLUS

typedef struct PendingFnList {
    struct PendingFnList *pfcdr;
    Symstr *pfname;             /* maybe a binder soon, see declarator_name */
    TypeExpr *pftype;
    SET_BITMAP pfstg;          /* b_memfna+b_memfns */
    TagBinder *pfscope;
    int32 pf_arg_scope;
    int pf_toklist_handle;
} PendingFnList;
static PendingFnList *syn_pendingfns;

#endif /* CPLUSPLUS */

static void fixup_fndef(DeclRhsList *temp, int declflag, SET_BITMAP memflags)
{   TypeExpr *fntype = temp->decltype;
    DeclRhsList *fnpars = typefnargs1_(fntype);
#ifdef CPLUSPLUS
    temp->declstg |= memflags;
    if (memflags & b_memfna)
        /* current_member_scope is uncomfortable as a static reference. */
        memfn_typefix(temp, current_member_scope());
#else
    IGNORE(declflag); IGNORE(memflags);
#endif
    if (curlex.sym != s_lbrace && curlex.sym != s_colon)
    {   if (feature & FEATURE_WARNOLDFNS)
            cc_warn(syn_warn_old_style, temp->declname);
    }
    else
    {   if ((feature & (FEATURE_PCC|FEATURE_FUSSY)) ==
                       (FEATURE_PCC|FEATURE_FUSSY))
            /* @@@ The next line considers f(){} ANSI only! */
            cc_warn(syn_warn_ANSI_decl, symname_(temp->declname));
    }
#ifdef CPLUSPLUS
/* Next additional C++ could sensibly be done for C as f(...) is        */
/* illegal in C.  The ellipsis will have set oldstyle==0 anyway.        */
/* This old PCC-compatibility code could be tidied too.                 */
    if (!fntypeisvariadic(fntype))
#endif
    {   if (fnpars == 0)
        {   maxargs_(fntype) = 0;
            if (!(feature & FEATURE_PCC))
                /* treat f() {} as f(void) {} in ANSI mode... */
                typefnaux_(fntype).oldstyle = 0;
        }
    }
    temp->declstg |= b_fnconst;
}

static void fixup_fndecl(DeclRhsList *temp, int declflag, SET_BITMAP memflags)
{
#ifdef CPLUSPLUS
    temp->declstg |= memflags;
    if (memflags & b_memfna)
        /* current_member_scope is uncomfortable as a static reference. */
        memfn_typefix(temp, current_member_scope());
    if (declflag & MEMBER); else
#else
    IGNORE(declflag); IGNORE(memflags);
#endif
    if (!(temp->declstg & (bitofstg_(s_extern)|
             bitofstg_(s_typedef)|bitofstg_(s_static))))
        syserr(syserr_rd_declrhslist);
    else
    {   temp->declstg |= b_fnconst|b_undef;
        if (maxargs_(princtype(temp->decltype)) == 999)
        { if (warn_deprecated)
            /* The follow warning enables us to root out        */
            /* insecurities/errors of the form:                 */
            /* extern f(); g() { f(1); } f(int x,int y) {...}   */
            cc_warn(syn_warn_give_args, symname_(temp->declname));
          else xwarncount++;
        }
    }
}

#ifdef CONST_DATA_IN_CODE
/* This should merge with the pointerfree_type call in cfe/vargen.c.    */
static void attempt_constdata(DeclRhsList *temp)
{   if ( (qualifiersoftype(temp->decltype) & bitoftype_(s_const)) &&
         ( !(config & CONFIG_REENTRANT_CODE) ||
           pointerfree_type(temp->decltype)) )
    {   datap = &constdata;
        temp->declstg |= u_constdata;
    }
}
#else
#define attempt_constdata(temp) /* nothing */
#endif

static void rd_declrhs_exec(DeclRhsList *temp, int declflag, bool cppinit)
{   int initflag = 0;
    if (temp->declstg & (bitofstg_(s_extern) | bitofstg_(s_static) | b_globalregvar))
        temp->declstg |= b_undef;
/* 'cppinit' holds in e.g. int a(3) and we have read the '('.           */
    if (curlex.sym == s_assign || curlex.sym == s_lbrace || cppinit)
/* Extern init only at TOPLEVEL.  Static init at AUTO scope too:        */
    {   if (((temp->declstg & bitofstg_(s_extern)) &&
             (declflag & TOPLEVEL)) ||
            (temp->declstg & bitofstg_(s_static)))
        {
            attempt_constdata(temp);
            temp->declstg &= ~b_undef;
        }
        if (temp->declstg & b_fnconst)
        {   cc_rerr(syn_rerr_fn_ptr1, temp->declname);
            temp->decltype = ptrtotype_(temp->decltype);
            temp->declstg &= ~(b_fnconst | b_undef);
        }
    }
#ifdef CONST_DATA_IN_CODE
    else if (((temp->declstg & bitofstg_(s_extern)) &&
                (temp->declstg & b_omitextern)) ||
               (temp->declstg & bitofstg_(s_static)))
    {    attempt_constdata(temp);
    }
#endif
    /* do the next line AFTER above type patching but before
       reading possible initialiser.  Save Binder in ->declbind.
       d->declstg now always has b_undef for statics & externs if there
       is no initialiser to read. instate_declaration removes for local
       statics not going in bss.
     */
    temp->declbind = instate_declaration(temp, declflag);
#ifdef CPLUSPLUS
    if (cppinit)
    {   ExprList *l = rd_exprlist();
        checkfor_ket(s_rpar);
        syn_initpeek = mk_cppcast(temp->decltype, l);
        initflag = 3;
    }
    else
#endif
    if (curlex.sym == s_assign || curlex.sym == s_lbrace)
    {   if (temp->declstg & (bitofstg_(s_auto)|bitofstg_(s_static)) ||
             (temp->declstg & (bitofstg_(s_extern)|b_undef)) ==
                              (bitofstg_(s_extern)))
            initflag = 1;
        else
            cc_err(syn_err_cant_init, temp->declstg & STGBITS),
            initflag = 2;

        if( curlex.sym == s_lbrace )
            cc_pccwarn(syn_rerr_archaic_init);
        else
            nextsym();
    }
    if (initflag == 2) (void)syn_rdinit(0,4);
    syn_initdepth = (initflag == 1) ? 0 : -1;
    syn_undohack = (initflag == 3);
    declinit_(temp) =
        genstaticparts(temp, (declflag & TOPLEVEL) != 0, initflag != 1);
    /* The positioning of the next line is subject to some debate */
    /* -- I put it here so that the line number is exact, but     */
    /* note that (a) TOPLEVEL vars are done in vargen.c,          */
    /* (b) LOCAL vars BLOCKHEAD|FORMAL are not yet complete in    */
    /* that we have not yet decided on their eventual storage;    */
    /* this is done by dbg_scope().  See other dbg_locvar() call. */
    if (usrdbg(DBG_VAR) && (declflag & BLOCKHEAD))
#ifdef TARGET_IS_C40
        dbg_locvar(temp->declbind, temp->fileline, NO);
#else
        dbg_locvar(temp->declbind, temp->fileline);
#endif
}

/* but for its size rd_declrhslist() would be part of rd_decl()
   maybe it will become so anyway!  It reads any of K&R's
  "(last part of undefined category)type-decl-list(see function-body)",
  "init-declarator-list" or "struct-declarator-list"
*/

static bool topfnflag;  /* for use of rd_declrhslist() and rd_decl() ONLY! */
static DeclRhsList *syn_formals;  /* ditto - now also rd_fndef() */

static DeclRhsList *rd_declrhslist(const SET_BITMAP ss, TypeSpec *const tt,
                const int declflag, const DeclSpec *ds)
{   DeclRhsList *p,*q;
    /* note - do NOT change ss, tt or declflag since used in loop below */
    for (p = q = 0; ;)                       /* 3 exits - all 'return's */
    {   DeclRhsList *temp = mkDeclRhsList(0, 0, ss);
        bool cppinit = 0;
        SET_BITMAP ss2 = 0;                  /* b_memfna+b_memfns       */
        NoteCurrentFileLine(&temp->fileline);
        declstgval_(temp) = ds->stgval;
        if ((declflag & MEMBER) && curlex.sym == s_colon)
            temp->decltype = tt;        /* anon. bit field: 0 declaree  */
        else
        {   if ((temp->decltype = rd_declarator(declflag, tt, &ds->fnaux)) == 0)
            {   /* error in rd_declarator already reported */
                if (declflag & TOPLEVEL)
                    bind_scope = TOPLEVEL, pop_scope(0);
                while (curlex.sym!=s_lbrace && curlex.sym != s_rbrace &&
                       curlex.sym!=s_semicolon && curlex.sym!=s_eof)
                    nextsym();
                if ((declflag & TOPLEVEL) && curlex.sym == s_rbrace) nextsym();
                topfnflag = 0;
                return p;       /* return decls so far to recover */
            }
#ifdef CPLUSPLUS                /* >>> temp t_coloncolon hack<<<<<   */
            /* do this so that fns can still be seen! */
            while (h0_(temp->decltype) == t_coloncolon)
                temp->decltype = typearg_(temp->decltype);
#endif
            cppinit = declarator_init;
            ss2 = declarator_memfn;
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
                 (declflag & MEMBER) != 0, temp->declstg);
        if (h0_(temp->decltype) == t_fnap)       /* but NOT via typedef */
        {   /* see if the fn declaration is a fn definition, we have    */
            /* already (in rd_formals()) changed f(void) to f()         */
            /* Allow also in MEMBER's -- only reach here if CPLUSPLUS   */
            if (declflag & (TOPLEVEL|MEMBER) &&   /* top level or class */
                p == 0 &&                         /* first in list      */
                !(temp->declstg & bitofstg_(s_typedef)) &&
                !cppinit &&
                /* not typedef and suitable following symbol...         */
                /* (CPLUSPLUS use of s_colon.)                          */
                (curlex.sym == s_lbrace ||
#ifdef CPLUSPLUS
                 curlex.sym == s_colon  ||
#endif
/* olde-style C "void f(x) type x; {...}":                              */
/* @@@ it would improve error recovery if we refuse extern here.        */
/* Note also that "int A::f() const;" shouldn't come here.              */
                   (declflag & TOPLEVEL &&
                      (rd_scope_or_typedef(0), isdeclstarter2_(curlex)))))
            {   fixup_fndef(temp, declflag, ss2);   /* b_memfna etc.    */
/* move the next few lines to fixup_fndef?                              */
                if (declflag & MEMBER)
                {   Binder *b;
                    ensure_formals_typed(typefnargs1_(temp->decltype), 1);
                    temp->declstg |= bitofstg_(s_inline);
                    b = (Binder *)instate_member(temp, bind_scope);
                    if (b && h0_(b) == s_binder && bindstg_(b) & b_pseudonym)
                    {   b = realbinder_(b);
                        if (!(bindstg_(b) & (b_memfna+b_memfns)))
                            syserr("rd_declrhslist(memfn $b)", b);
                        temp->declstg |= bindstg_(b) & (b_memfna|b_memfns);
                        temp->declname = bindsym_(b);
                    }
                    else
                        syserr("rd_declrhslist(inline memfn $b)", b);
                }
/* @@@@@@ Why is return (instead of fn call) a good idea here????       */
                topfnflag = 1;
                return temp;
            }
            /* ANSI C disallows 'extern f(a,b);' (types needed).:       */
            ensure_formals_typed(typefnargs1_(temp->decltype), 1);
            /* drop through into next 'if'... */
        }
        if (isfntype(temp->decltype))           /* possibly via typedef */
            fixup_fndecl(temp, declflag, ss2);  /* b_memfna etc.        */
/* The following two lines anticipate __inline for ANSI C;              */
/* otherwise they would be #ifdef CPLUSPLUS.                            */
        else if (ss & (bitofstg_(s_inline)|bitofstg_(s_virtual)))
            cc_rerr("inline ignored for non-fn $r", temp->declname);
        if (declflag & ARG_TYPES)     /* special code to save up info */
        {   DeclRhsList *p;           /* instate_decl is called later */
            Symstr *sv = temp->declname;
            for (p = syn_formals; p != 0; p = p->declcdr)
                if (sv == p->declname)
                {   merge_formal_decl(p,temp);  /* update syn_formals */
                    break;
                }
            if (p==0) cc_err(syn_err_not_a_formal, sv);
        }
        if (declflag & (TOPLEVEL|BLOCKHEAD))
            rd_declrhs_exec(temp, declflag, cppinit);
#ifdef CPLUSPLUS
        if (declflag & FORMAL && curlex.sym == s_assign)
        {   Expr *init;
            nextsym();
            cc_err("ignoring default parameter value for $r", temp->declname);
            /* @@@ check not a pcc untyped formal? */
            init = mkcast(s_fnap, rd_expr(UPTOCOMMA), temp->decltype);
            (void)optimise0(init);
}
#endif
/* @@@ the next comment is for old C implementation -- instating        */
/* formals fixes the C bug in 'void f(int a, int (*b)[sizeof a]) {...}  */
/* and instating members is invisible to C.  @@@ It is out of date.     */
        /* do not instate formals (or in casts "(int ()(int a,b))foo")
           nor structure elements. */
        if (declflag & MEMBER)
        {   if (curlex.sym == s_colon)
            {   TypeExpr *t = prunetype(temp->decltype);
/* NB. The code here is most important -- BITFIELD types have any        */
/* typedefs removed with prunetype so that later prunetype/princtype's   */
/* (e.g. isbitfield_type()) do not skip over BITFIELD.                   */
                nextsym();
                /* declbits holds bit size (= new arg to instate_mem?)   */
                declbits_(temp) = check_bitsize(
                            rd_expr(UPTOCOMMA), t, temp->declname);
                temp->decltype = check_bittype(t);
            }
            instate_member(temp, bind_scope);
        }
/* The next line loses all local vars/tags at the end of each TOPLEVEL  */
/* declarator.  This is still not quite right, but enables vars/tags    */
/* to be local to (toplevel) parameter lists.  @@@ More fixing required */
/* for parameter lists within parameter lists.                          */
        if (declflag & TOPLEVEL)
            bind_scope = TOPLEVEL, pop_scope(0);
        if (p == 0) p = q = temp;
        else { q->declcdr = temp; q = temp; }
        if ((declflag & FORMAL) || curlex.sym != s_comma)
        {   topfnflag = 0;
            return p;
        }
        nextsym();
    }
}

/* AM, Sept 91: memo: too much effort is spent mapping between          */
/* DeclRhsList and FormTypeLists, and Binders.  @@@ Think.              */

#ifdef CPLUSPLUS
static DeclRhsList *DeclRhs_of_FormType(FormTypeList *x)
{   DeclRhsList *p = 0, *q = 0, *t;
/* This routine loses any s_register on args of member fn definitions   */
/* which are specified in a class definition.  Tough.                   */
    for (; x; x = x->ftcdr)
    {   t = mkDeclRhsList(x->ftname, x->fttype, bitofstg_(s_auto));
        if (p == 0) p = q = t; else q->declcdr = t, q = t;
    }
    return p;
}

static DeclRhsList *reinvent_fn_DeclRhsList(Symstr *name, TypeExpr *t,
                                            SET_BITMAP s)
{   DeclRhsList *p = mkDeclRhsList(name, t,
        b_fnconst|bitofstg_(s_static)|bitofstg_(s_inline)
                 |s&(b_memfns|b_memfna));
    typefnargs1_(t) = DeclRhs_of_FormType(typefnargs_(t));
    return p;
}
#endif

static TopDecl *rd_fndef(DeclRhsList *d, int declflag)
/* d has exactly 1 item */
{   TypeExpr *fntype = d->decltype;        /* known to be fn type */
    Binder *fnbind; Cmd *body;
    DeclRhsList *fpdecllist = typefnargs1_(fntype), *fpe;
    SynBindList *lambdap = 0, *lambdaq = 0;
    SynBindList *narrowformals = 0; Expr *arginit = 0;
    /* syn_formals is examined by rd_declrhslist if ARG_TYPE is set */
    IGNORE(declflag);
    syn_formals = fpdecllist;
    currentfunction.symstr = d->declname;
    if (debugging(DEBUG_FNAMES))
        cc_msg("Start of function $r\n", currentfunction.symstr);
/* when ARG_TYPES is set in declflag any identifiers declared will have  */
/* been seen before (as formal parameters) and rd_decllist works by      */
/* updating these existing declaration records in a side-effect-full way */
/* Feb 93: the next line always currently pops an empty scope.          */
/* it will need to do more work when parameters have proper scopes.     */
    unpop_scope();
    bind_scope = GLOBALSCOPE;              /* local scope for any structs! */
    (void) rd_decllist(ARG_TYPES);
/* do some checking and defaulting on the arguments and types BEFORE
   instate_decl ... */
    for (fpe = fpdecllist; fpe != 0; fpe = fpe->declcdr)
    {   if (fpe->declname == 0)
        {
#ifndef CPLUSPLUS
            cc_rerr(syn_rerr_missing_formal);    /* not an error in C++ */
#endif
            /* fixup so we can continue (@@@ soon alter callers)...      */
            fpe->declname = gensymval(1);
        }
    }
    ensure_formals_typed(fpdecllist, 0);
/* do the declaration of the function - this will copy the types just
 * updated in fpdecllist as part of globalize_ing the type of d.
 */
  { FileLine fl; NoteCurrentFileLine(&fl);
    fnbind = instate_declaration(d, TOPLEVEL);
#ifdef CPLUSPLUS
/* @@@ There is a nastiness in that currentfunction was never intended  */
/* for general use (especially as it is a C 'string'), it was just      */
/* intended for internal compiler progress messages.  Unfortunately     */
/* arm/gen.c uses it to get back at what AM presumes is fnbind above.   */
/* The is needed for C++ because d->declname can be updated by the      */
/* ovld_instance_name code.  Rationalise soon?                          */
/* Sep 92: now being rationalised...                                            */
    currentfunction.symstr = d->declname;
    set_access_context(0, fnbind);
                                  /* complete the friend access context */
#endif
    if (usrdbg(DBG_PROC+DBG_VAR))
        dbg_proc(bindsym_(fnbind), bindtype_(fnbind),
                 (bindstg_(fnbind) & bitofstg_(s_extern)) != 0, fl);
/* @@@ The following comment about re-using store is now a lie.         */
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
#ifndef TARGET_IS_XPUTER
        {   Expr *e;
            DeclRhsList *narrowedformal = mkDeclRhsList(fpe->declname, at,
                                  fpe->declstg);
            DeclRhsList *wideformal = mkDeclRhsList(fpe->declname, wt,
                                  fpe->declstg);
            /* do the original binder second so it gets seen first */
            wb = instate_declaration(wideformal, FORMAL);
            ab = instate_declaration(narrowedformal, FORMAL|DUPL_OK);
            binduses_(wb) |= u_referenced;
            e = mkassign(s_assign, (Expr *)ab,  /* do the narrowing */
                                   mkcast(s_cast, (Expr *)wb, bindtype_(ab)));
            narrowformals = mkSynBindList(narrowformals, ab);
            arginit = arginit ? mkbinary(s_comma, arginit, e) : e;
        }
#else
	/* The non-transputer version of this code generates a casting	*/
	/* assignment from the narrow to the wide formal, when using the*/
	/* full compiler this is optimised away, or reduced to an in-	*/
	/* place conversion (e.g. a sign extend) for the transputer cg	*/
	/* this does not happen and we can end up with unnecessary and	*/
	/* wasteful assignments. Not forcing the cast means we rely on	*/
	/* the caller setting up his parameters properly, this does not	*/
	/* seem to have caused any problems so far...			*/
	{
                   DeclRhsList *wideformal = mkDeclRhsList(fpe->declname, wt,
                                          fpe->declstg);
                    ab = wb = instate_declaration(wideformal, FORMAL);
	}
#endif /* TARGET_IS_XPUTER */

#ifndef CPLUSPLUS
        if (!(feature & FEATURE_PREDECLARE))
            /* preclude any whinge about unused fn args */
            /* Allow whinges in C++ since formal names are omittable.   */
            binduses_(ab) |= u_referenced;
#endif
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
        if (lambdap == 0) lambdap = lambdaq = mkSynBindList(0,wb);
        else lambdaq = lambdaq->bindlistcdr = mkSynBindList(0,wb);
    }
    {   Cmd *argstuff = narrowformals==0 ? 0 :
            mk_cmd_e(s_semicolon, syn_invented_fl, optimise0(arginit));
#ifdef CPLUSPLUS
/* @@@ we should pass 0 to rd_meminit if not a ctor (bit in bindstg_).  */
        TypeExpr *arg1t = bindstg_(fnbind) & b_memfna && lambdap ?
                              bindtype_(lambdap->bindlistcar) : te_int;
        TypeExpr *arg1s = h0_(arg1t) == t_content ? typearg_(arg1t) : te_int;
        Cmd *meminit = rd_meminit(isclasstype_(arg1s) ?
                                      typespectagbind_(arg1s) : 0);
#endif
        bind_scope = LOCALSCOPE;
        body = rd_body(typearg_(fntype));
        bind_scope = TOPLEVEL;
#ifdef CPLUSPLUS
        if (meminit)
            body = mk_cmd_block(syn_invented_fl, 0,
                                mkCmdList(mkCmdList(0,body), meminit));
#endif
        if (argstuff)
            body = mk_cmd_block(syn_invented_fl, narrowformals,
                                mkCmdList(mkCmdList(0,body), argstuff));
    }
    label_resolve();
    pop_scope(0);
/* h4_(result) is a flag to indicate the presence of an ellipsis in      */
/* the list of formal arguments for this function.                       */
    return mkTopDeclFnDef(s_fndef, fnbind, lambdap, body,
        /*
         * Beware !!!. In PCC mode all functions are considered as
         * Having a trailing '...'.  This is activated by the
         * code in cg.c which checks whether the address of ANY of the
         * args has been taken.  If so all args go to stack !!!
         */
        (fntypeisvariadic(fntype) || (lambdap!=0) && (feature&FEATURE_PCC)));
  }
}


/* rd_decl reads a possibly top-level decl, see also rd_decl2()       */
static TopDecl *rd_decl(int declflag, SET_BITMAP accbits)
/* AM: Structure decls are tantalisingly close to ordinary decls (but no
   storage class) except for length specs replacing initialisation.
   Type specs are compulsory for structure members, but not for
   vars (e.g. auto a=1;).
   Read them all them with a single parameterised routine.
   Note that 'accbits' is or-able with declstg_() (non-zero if MEMBER).
   @@@ Feb 93: It is in a different bit position from in attributes_()!!!
*/
{
    DeclRhsList *d;
    DeclSpec ds;
    TypeSpec *t;
    SET_BITMAP s;
    int declxtramap;
#ifdef CPLUSPLUS
    int scope_level = current_scope_level();
#endif
    FileLine fl; NoteCurrentFileLine(&fl);
#ifdef CPLUSPLUS                    /* ... in conjunction with this... */
    while (curlex.sym == s_asm) rd_asm_decl();
    if ((declflag & MEMBER) &&
          (curlex.sym == s_bitnot ||
             curlex.sym == s_identifier &&
             findtagbinding(curlex.a1.sv) == current_member_scope() &&
             peepsym() == s_lpar))
    {   /* constructor or destructor */
        s = 0;
        t = te_int;
        ds.fnaux.flags = 0, ds.fnaux.val = 0;
        declxtramap = 0;
        syserr("rd_decl(ctor/dtor)");    /* rd_scope_or_typedef catches? */
    }
    else
#endif
    {   ds = rd_declspec(declflag);
        if (fpregargs_disabled) ds.fnaux.flags |= f_nofpregargs;
        if (no_side_effects) ds.fnaux.flags |= bitoffnaux_(s_pure);
        s = ds.stg;
        t = ds.t;
        declxtramap = ds.synflags;    /* errorchecks          */
    }
/* merge acess bits (if MEMBER) and stg bits...                         */
    s |= accbits;
/* we are required to complain at top level for "*x;", but not "f();"   */
#ifdef CPLUSPLUS
/* @@@ we could/should use the synflags bits in the following...        */
    if (typespecmap_(t) == LINKAGESPEC)
    {   if (s & bitofstg_(s_extern))
        {   /* The 'extern "C" typedef int foo(); form                  */
            TopDecl *dd = rd_decl(TOPLEVEL, 0);
            syn_pop_linkage();
            return dd;
        }
        else d = 0;             /* treat as empty decl.                  */
    }
    else
#endif
  { if (curlex.sym == s_semicolon && !(declflag & FORMAL) ||
        curlex.sym == s_rbrace    && (declflag & MEMBER))
    {   /*
         * Found an empty declaration or an empty declaration within a
         * struct or union in which the terminating ';' has been omitted.
         */
        d = 0;
        if (!(declxtramap & B_DECLMADE))
            cc_pccwarn(syn_rerr_ineffective);
        else
        {   /*
             * Here we have just got a new struct/union/enum tag
             * so lets tell the debugger about it, lest it never
             * finds out about it until its too late.
             */
            if (usrdbg(DBG_PROC)) dbg_type(gensymval(1),t);
        }
    }
    else
    {   d = rd_declrhslist(s, t, declflag, &ds);
        if (d != 0) d->fileline = fl;
        if (topfnflag)
        {   /* NB. it is vital that when topfnflag is set on return     */
            /* from rd_declrhslist we must soon pop_varenv/tagenv.      */
            /* This has to be done after reading the body due to ANSI   */
            /* scope joining of formals and body top block.             */
#ifdef CPLUSPLUS
            if (declflag & MEMBER)
            {   PendingFnList *x =
                    (PendingFnList *)GlobAlloc(SU_Other, sizeof(*x));
                /* Stash away (d,h) for use after block -- we can't     */
                /* use them straight away -- see lex_savebody().        */
                x->pfcdr = syn_pendingfns;
                x->pfname = d->declname;
                x->pftype = globalize_typeexpr(d->decltype);
                x->pfstg = d->declstg;
                x->pfscope = current_member_scope();
/* @@@ the next line gets the wrong bindings in                         */
/*   int (*f(int a))(int b)                                             */
/* and probably if 'a' had a sizeof(struct defn).                       */
/* This only affects (C++ illegal) arg-declared classes currently.      */
                x->pf_arg_scope = popped_bindings();
                x->pf_toklist_handle = lex_savebody();
                syn_pendingfns = x;
                curlex.sym = s_nothing;
                /* Just pretend all is sweetness and light 'til later:  */
                return (TopDecl *) syn_list2(s_decl, d);
            }
#endif
            if (feature & FEATURE_PCC) implicit_return_ok = syn_oldeformals;
            /* the next line allows static f(); - ansi_warn soon? */
            if (declxtramap & B_OMITTYPE)
            {   if (suppress & D_IMPLICITVOID)
                    /* The next line allows us also to avoid the       */
                    /* 'implicit return' warning in f(){}.             */
                    /* avoid (wrong MEMBER fn's) implicit_return_ok=1  */
                    xwarncount++, implicit_return_ok = 1;
                else
                    cc_warn(syn_warn_untyped_fn, symname_(d->declname));
            }
            return rd_fndef(d, declflag);        /* d != 0 by fiat */
        }
        /* this message is a little late, but cannot occur 'on time' */
        if (declxtramap & B_OMITTYPE && d != 0)
            /* the case d==0 has already been reported by          */
            /* rd_declarator() via rd_declrhslist() (q.v.).        */
            if (!(feature & FEATURE_PCC))
                cc_rerr(syn_rerr_missing_type3);
    }
    if ((declflag & FORMAL) == 0)
    {
      if (!(feature & FEATURE_PCC) || curlex.sym != s_rbrace)
        checkfor_delimiter_2ket(s_semicolon, s_comma);
    }
  }
#ifdef CPLUSPLUS
    pop_scope(scope_level);
#endif
    return (TopDecl *) syn_list2(s_decl, d);
}

static DeclRhsList *rd_decl2(int declflag, SET_BITMAP accbits)
{   TopDecl *d = rd_decl(declflag, accbits);
    if (d == 0 || h0_(d) != s_decl)
         syserr(syserr_rd_decl2, d, (long)(d==0 ? 0 : h0_(d)));
    return d->v_f.var;
}

/* rd_formals() is only used once in rd_declarator_1().  It behaves     */
/* very much like rd_decllist, but different concrete syntax.           */
/* It copes with both ANSI and olde-style formals (and now C++).        */
/* @@@ sort out the 999/1999 confusion.                                 */
static DeclRhsList *rd_formals(void)
{   DeclRhsList *p,*q,*temp;
    const int32 minformals = -1;
    if (curlex.sym == s_rpar)
    {
#ifdef CPLUSPLUS
       syn_minformals = 0, syn_maxformals = 0, syn_oldeformals = 0;
#else
       syn_minformals = 0, syn_maxformals = 999, syn_oldeformals = 1;
#endif
       return 0;
    }
    for (p = q = 0;;)
    {   if (curlex.sym == s_ellipsis)
        {
#ifndef CPLUSPLUS
            if (p == 0) cc_rerr(syn_rerr_ellipsis_first);
#endif
            fault_incomplete_formals(p);
            nextsym();
            syn_minformals = length((List *)p), syn_maxformals = 1999;
            syn_oldeformals = !is_proto_arglist(p,1);
/* @@@ the last line is a gross hack (abetted by AM) */
            return p;    /* to checkfor_ket(s_rpar) */
        }
#ifdef EXTENSION /* allow optional, but type-checked parameters. */
        if (curlex.sym == s_cond) nextsym(), minformals = length((VoidStar)p);
#endif
        temp = rd_decl2(FORMAL, 0);
        if (curlex.sym == s_nothing) nextsym();
        for (; temp != 0; temp = temp->declcdr)
        {   if (debugging(DEBUG_BIND))
                cc_msg(" Formal: $r\n", temp->declname);
            if (p == 0) p = q = temp; else q->declcdr = temp, q = temp;
        }
#ifdef CPLUSPLUS
        if (curlex.sym == s_ellipsis) continue;    /* can omit comma  */
        else
#endif
        /* all that should legally appear here are ',' and ')', but fix    */
        /* ';' as ',' a la pascal and treat all other symbols as ')' to be */
        /* faulted by the caller (rd_declarator_1()).                      */
        if (curlex.sym == s_semicolon)   /* error recovery */
            cc_rerr(syn_rerr_semicolon_in_arglist);
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
            syn_maxformals = length((List *)p);
            syn_minformals = minformals >= 0 ? minformals : syn_maxformals;
            syn_oldeformals = !b;
            return p;
        }
        nextsym();
    }
}

static Declarator *rd_formals_1(Declarator *a, const DeclFnAux *fnaux)
{   int old_bind_scope = bind_scope;
    int scope_level = push_scope(0);
    DeclRhsList *f;
    TypeExprFnAux s;
    if (bind_scope == TOPLEVEL) bind_scope = GLOBALSCOPE;    /* @@@ nasty */
    f = rd_formals();
    bind_scope = old_bind_scope;
    pop_scope_no_check(scope_level);
/* @@@ beware that ANSI disallow the use of pragma's which change         */
/* semantics -- those for printf requesting extra warnings are ok, but    */
/* not those like 'no_side_effects' which can screw code.                 */
              /* extra results for minargs_(), maxargs_()... */
    return mkTypeExprfn(t_fnap, a, (FormTypeList *)f,
                    packTypeExprFnAux1(s, (int)syn_minformals,
                                         (int)syn_maxformals,
                                         (int)special_variad,
                                         syn_oldeformals,
                                         fnaux->flags,
                                         fnaux->val));
}

static DeclRhsList *rd_decllist(int declflag)
/* reads a list of Decls and NCONC's them togther.  Used for struct/union
   elements, type specs after formal lists, rd_block().  NB:  It should not
   be used when TOPLEVEL is set as it does not look for functions.
   It should not be used either when MEMBER is set (no 'accbits' facility).
   Identical to rd_formals modulo concrete syntax - unify???
   Note the Symstr's may be 0 (abstract), or s_id.
*/
{   DeclRhsList *p,*q,*temp;
    p = q = 0;
    while (rd_scope_or_typedef(0), isdeclstarter2_(curlex))
    {   if (curlex.sym == s_typestartsym) nextsym();
        temp = rd_decl2(declflag, 0);
        if (curlex.sym == s_nothing) nextsym();
        for (; temp != 0; temp = temp->declcdr)
        {   if (p == 0) p = q = temp; else q->declcdr = temp, q = temp;
            if (debugging(DEBUG_BIND)) cc_msg(" Identifier: $r", q->declname);
        }
    }
    return p;
}

#ifdef CPLUSPLUS
static ClassMember *in_base_list(TagBinder *cl, ClassMember *l)
{   for (;  l != NULL;  l = memcdr_(l))
        if (typespectagbind_(l->memtype) == cl)
            return l;
    return 0;
}
#endif

static ClassMember *rd_strdecl(TagBinder *b, ClassMember *bb, ClassMember *vb)
{   bool nameseen = 0;
/* Use a SET_BITMAP for access soon (see uses of this local var)?       */
    AEop access = attributes_(b) & bitoftype_(s_class) ? s_private : s_public;
    int scope_level = push_scope(b);
#ifdef CPLUSPLUS
    TagBinder *old_access = set_access_context(b, 0);
        /* doesn't yet nest properly... */
#endif
    for (;;)
    {
#ifdef CPLUSPLUS
        bool access_seen = 0;
/* @@@ mumble, mumble access syntax...                                  */
        if (isaccessspec_(curlex.sym))
        {   access = curlex.sym;
            access_seen = 1;
            nextsym();
            checkfor_ket(s_colon);
        }
#else
    IGNORE(vb);  IGNORE(bb);
#endif
/* @@@ check/fix this....                        The stack              */
/* of scopes is also side-effected and later unwound by pop_scope().    */
        rd_scope_or_typedef(b);
#ifdef CPLUSPLUS
        if ((curlex.sym == s_qualified+s_identifier ||
             curlex.sym == s_qualified+s_pseudoid)
            && (peepsym()==s_semicolon || peepsym()==s_comma
                                       || peepsym()==s_rbrace))
        {   /* access adjustment member...                              */
            /* possibly is typedef/static member (but see syserr below) */
            /* @@@ what about A::operator foo?                          */
            Binder *mb = findbinding(curlex.a1.sv);
            if (mb && h0_(mb) == s_member)
            {   ClassMember *m = (ClassMember *)mb, *base;
                /* An access declaration of <current_member_scope>::m */
#define syn_rerr_not_base(mm,cc) "$b is not a base class of $b",mm,cc
#define syn_rerr_badly_placed_access \
    "access declarations only in public and protected parts"
#define syn_rerr_modify_access "base access rights cannot be altered"
#define syn_warn_modify_access "access declaration with no effect"
                TagBinder *cl = current_member_scope();
                pop_scope(scope_level+1);
/* Access declarations must refer to a base class... */
                if ((base = in_base_list(cl, bb)) == NULL &&
                    (base = in_base_list(cl, vb)) == NULL)
                    cc_rerr(syn_rerr_not_base(cl, b));
/* Access declarations may only occur in a public or protected part... */
                else if (!access_seen || access == s_private)
                    cc_rerr(syn_rerr_badly_placed_access);
/* Finally, an access declarations may not vary base access rights...  */
                else
                {   bool ok = 1;
                    if (attributes_(base) & bitofaccess_(s_public))
                    {   /* publicly derived */
/* Warn of vacuously useless declarations and fault all others...      */
                        if (attributes_(m) & bitofaccess_(access))
                            cc_warn(syn_warn_modify_access);
                        else
                            ok = 0;
                    }
                    else
                    {   /* privately derived... */
/* Fault attempts to grant access greater or less than base access...  */
                        if ((attributes_(m) & bitofaccess_(access)) == 0 &&
                            (access != s_protected ||
                              ((attributes_(m) & bitofaccess_(s_public)) == 0)))
                        ok = 0;
                    }
                    if (!ok)
                        cc_rerr(syn_rerr_modify_access);
                    else
                    {   DeclRhsList *d = mkDeclRhsList(memsv_(m), ACCESSADJ,
                                                       stgaccof_(access));
                        instate_member(d, bind_scope);
                    }
                }
                nextsym();
                checkfor_ket(s_semicolon);
            }
            else
            {   syserr("Odd access adjuster (typedef?)");
                break;
            }
        }
        else
#endif
        if (isdeclstarter2_(curlex)
#ifdef CPLUSPLUS
/* Additional C++ (odd!) ways for a member declaration to start...      */
/* ... since the 'type' info is optional (sigh):                        */
            || (curlex.sym & ~s_qualified) == s_identifier
            || (curlex.sym & ~s_qualified) == s_pseudoid
            || curlex.sym == s_lpar
            || (curlex.sym & ~s_qualified) == s_times
            || curlex.sym == s_and
#endif
            || curlex.sym == s_semicolon)
        {   DeclRhsList *d;
            if (curlex.sym == s_typestartsym) nextsym();
            d = rd_decl2(MEMBER, stgaccof_(access));
/* using the DeclRhsList from rd_decl2 is now deprecated (we should     */
/* really look in tagbindmems_(b) to make nameseen).                    */
            for (; d != 0; d = d->declcdr)
                if (d->declname != NULL) nameseen = 1;
            if (curlex.sym == s_nothing) nextsym();
        }
        else break;
    }
    pop_scope(scope_level);
#ifdef CPLUSPLUS
    set_access_context(old_access, 0);
#endif
    if (!nameseen)
    {   if (tagbindmems_(b) != 0)
            cc_warn(syn_warn_no_named_member, b);
#ifndef CPLUSPLUS
        else
            cc_rerr(syn_rerr_no_members, b);
#endif
    }
    return tagbindmems_(b);
}

#ifndef CPLUSPLUS
static ClassMember *rd_classdecl(TagBinder *b, bool has_bases)
{   IGNORE(has_bases);
    return rd_strdecl(b, NULL, NULL);
}
#endif

/* rd_enumdecl() works very much like rd_decllist() or rd_strdecl() but the
   different surface syntax leads me to implement it separately.
   Syntax is taken from Harbison&Steele.
   ANSI draft means 'int' implementation of 'enum's.
*/

static ClassMember *rd_enumdecl(TypeExpr *t)
{   ClassMember *p,*q;
    int32 nextenumval = 0;
    bool is_non_neg = YES;
#ifdef CPLUSPLUS
    if (curlex.sym == s_rbrace) return 0;
#endif
    for (p = q = 0;;)
    {   if (curlex.sym == s_identifier)
        {   Symstr *sv = curlex.a1.sv; Expr *e = 0; Binder *b;
            DeclRhsList *temp = mkDeclRhsList(sv, t, b_enumconst);
            nextsym();
            if (curlex.sym == s_assign)
            {   nextsym();
                e = optimise0(rd_expr(UPTOCOMMA));
            }
/* @@@ AM: fixup 16 bit ints here too.                                  */
            if (e != 0)
            {   nextenumval = evaluate(e);
                is_non_neg =
                   (h0_(e) == s_integer &&
                    typespecmap_(type_(e)) & bitoftype_(s_unsigned) ||
                    nextenumval >= 0);
            }
/* bind_scope is set so that all enum consts are globalized except in fns */
            b = instate_declaration(temp, bind_scope);
            if (is_non_neg && nextenumval < 0)
                is_non_neg = NO,                /* only one error.      */
                cc_rerr(sem_rerr_monad_overflow(s_enum,0,0));
            bindaddr_(b) = nextenumval++;
            /* BEWARE: reuse of DeclRhsList to hold ClassMember */
            {   ClassMember *tempc = (ClassMember *)temp;
                if (sizeof(*tempc) > sizeof(*temp)) syserr("rd_enum/sizeof");
                memcdr_(tempc) = NULL;
                memsv_(tempc) = sv;
                memtype_(tempc) = t;
                tempc->u.enumval = bindaddr_(b);
                if (p == 0) p = q = tempc; else memcdr_(q) = tempc, q = tempc;
            }
        }
        else
        {   cc_err(syn_err_enumdef);
            while (curlex.sym != s_rbrace && curlex.sym != s_eof) nextsym();
        }
        if (curlex.sym != s_comma) break;
        nextsym();
#ifdef CPLUSPLUS
        if (curlex.sym == s_rbrace)
        {    cc_rerr(syn_warn_extra_comma);
             break;
        }
#else
        if (curlex.sym == s_rbrace && (feature & FEATURE_PCC))
        {    cc_warn(syn_warn_extra_comma);
             break;
        }
#endif
    }
    return (ClassMember *)p;
/* The result is used by debug-table formatters, as well as as a 0/non-0 flag */
}

/* Exported: ONLY syn_init(), syn_eof and rd_topdecl().
   Calling conventions: call lex_init() first.  Then call rd_topdecl()
   for as long as syn_eof() is false.
*/

bool syn_eof()          /* maybe temporary, but better than before.     */
{
#ifdef CPLUSPLUS
    if (syn_pendingfns) return 0;
#endif
    if (curlex.sym == s_nothing) nextsym();
    while (curlex.sym == s_toplevel) nextsym();
#ifdef CPLUSPLUS
    if (syn_linkage != 0) switch (curlex.sym)
    {
case s_eof:     cc_err("expected <linkage-spec> '}' before $l");
                /* drop through to effect insertion... */
case s_rbrace:  syn_pop_linkage(); nextsym();
                return syn_eof();
    }
#endif
    return curlex.sym == s_eof;
}

TopDecl *rd_topdecl(void)
{   TopDecl *d;
    implicit_return_ok = 0;      /* for junky old C programs         */
#ifdef EXTENSION_VALOF
    inside_valof_block = 0;
    valof_block_result_type = (TypeExpr *) DUFF_ADDR;
    cur_restype = 0;             /* check for valof out of body      */
#endif
#ifdef CPLUSPLUS
/* @@@ AM: the next is probably not quite right for nested classes.     */
    set_access_context(0, 0);
    if (syn_pendingfns)
    {   TopDecl *fd;
        DeclRhsList *d = reinvent_fn_DeclRhsList(
                            syn_pendingfns->pfname,
                            syn_pendingfns->pftype,
                            syn_pendingfns->pfstg);
        if (syn_pendingfns->pfstg & b_memfna)
            memfn_typefix(d, syn_pendingfns->pfscope);
        push_scope(syn_pendingfns->pfscope);
/* @@@ re-read [ES] to see if pfscope should include outermore scopes!  */
        set_access_context(syn_pendingfns->pfscope, 0);
/* the next line currently only restores arglist tagbindings...         */
        pop_scope(push_scope(0));
/* the next line is really yukky...                                     */
        restore_bindings(syn_pendingfns->pf_arg_scope);
        lex_openbody(syn_pendingfns->pf_toklist_handle);
        syn_pendingfns = syn_pendingfns->pfcdr;
        fd = rd_fndef(d, TOPLEVEL);
        lex_closebody();
        pop_scope(0);
        return fd;
    }
#endif
    while (curlex.sym == s_lbrace)  /* temp for ACN - rethink general case */
                                    /* @@@ and especially for C++ linkage! */
    {   (void) push_scope(0);    /* helps recovery */
        cc_err(syn_err_misplaced_brace);
        (void)rd_body(te_int);      /* this will also skip initialisers */
        label_resolve();            /* tidy up in case declarations */
        pop_scope(0);
        if (curlex.sym == s_nothing) nextsym();
        while (curlex.sym == s_toplevel) nextsym();
    }
    d = rd_decl(TOPLEVEL, 0);
    curlex.fl.p = dbg_notefileline(curlex.fl);
    return d;
}

void syn_init(void)
{   bind_scope = TOPLEVEL;
    initpriovec();
#ifdef CPLUSPLUS
    syn_pendingfns = 0;
    syn_linkage = syn_linkage_free = 0;
#endif
}

/* End of cfe/syn.c */
