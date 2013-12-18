/* $Id: syn.c,v 1.1 1990/09/13 17:10:34 nick Exp $ */

/* c.syn: syntax analysis phase of C compiler */
/* Copyright (C) A.Mycroft and A.C.Norman     */
/* version 126 */

/***********************************************************************/
/*                                                                     */
/*  Parsing is done by recursive descent, building a tree on the way.  */
/*                                                                     */
/***********************************************************************/

#include "cchdr.h"
#include "AEops.h"

#define syn_err cc_err
#define syn_rerr cc_rerr
#define syn_warn cc_warn

/* forward references within this file - reorganise to reduce */
static void ensure_formals_typed();
static TypeExpr *rd_typename();
static Expr *rd_expr();
static Expr *rd_prefixexp();
static Cmd *rd_block();
static Cmd *rd_command();
static DeclRhsList *rd_decllist();
static TagMemList *rd_strdecl();
static TagMemList *rd_enumdecl();
static DeclRhsList *rd_formals();

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

int implicit_return_ok;

#define B_OMITTYPE b_synbit1
#define B_DECLMADE b_synbit2

#define ncons2_(x)   list2(0,x)
#define ncons6_(x,y,z,t,a) list6(0,x,y,z,t,a)

/* values for 'declflag' below - may be or'ed in sensible combinations.
   NB.  I (AM) regard these as entirely private to parsing */

#define POINTERTYPES       0x001
#define LENGTH_OK          0x002  /* i.e. in struct/union */
#define STGCLASS_OK        0x004
/* #define DUPL_OK            0x008 */
/* #define TOPLEVEL           0x010 */
#define BLOCKHEAD          0x020
#define TYPE_NEEDED        0x040
#define ARG_TYPES          0x100  /* declaring formal parameters      */
/* the following bits are exclusive and one must be set for rd_decl() */
/* Note (as ANSI want) FORMAL satisfies both needed_ and banned_      */
#define FORMAL             0x200
#define needed_DECLARATOR  0x400
#define banned_DECLARATOR  0x800
/* readability synonyms (ACN: remove ?) */
#define ABS_DECLARATOR     banned_DECLARATOR
#define CONC_DECLARATOR    needed_DECLARATOR

#define istypedefname_(sv) (_symdata(sv)!=NULL &&  \
             (bindstg_(_symdata(sv))&bitofstg_(s_typedef)))

/* parsing table initialisation... */
static int illtypecombination[NUM_OF_TYPES];

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
static void initpriovec()
{   AEop s,t;
    int i;
    for (s = s_char; istypestarter_(s); s++)
        illtypecombination[shiftoftype_(s)] = ~0;
    illtypecombination[shiftoftype_(s_signed)] =
    illtypecombination[shiftoftype_(s_unsigned)] =
        ~(bitoftype_(s_int) | bitoftype_(s_char) |
          bitoftype_(s_long) | bitoftype_(s_short) |
          bitoftype_(s_const) | bitoftype_(s_volatile));
    illtypecombination[shiftoftype_(s_long)] =
    illtypecombination[shiftoftype_(s_short)] =
        ~(bitoftype_(s_int) | bitoftype_(s_signed) | bitoftype_(s_unsigned) |
          bitoftype_(s_double) |
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


static Expr *check_bitsize(e)
Expr *e;
{   unsigned int n = evaluate(e);
    if (n>32) syn_err("Bit size %d illegal - 1 assumed", n),
              e = globalize_int(1);
    return e;
}

static Expr *check_arraysize(e)
Expr *e;
{   unsigned int n = evaluate(e);
    if (n == 0)
    {   if (suppress & D_ZEROARRAY) xrecovercount++;
        else syn_rerr("Array [0] found");
    }
    if (n>0xffffff) syn_err("Array size %d illegal - 1 assumed", n),
                    e = globalize_int(1);
    return e;
}

static char *ctxtofdeclflag(f)
int f;
{  if (f & POINTERTYPES) return "<after * in declarator>";
   if (f & TOPLEVEL) return "<top level>";
   if (f & LENGTH_OK) return "<structure component>";
   if (f & FORMAL) return "<formal parameter>";
   if (f & ARG_TYPES) return "<formal parameter type declaration>";
   if (f & BLOCKHEAD) return "<head of block>";
   if (f & ABS_DECLARATOR) return "<type-name>";
   return "<unknown context>";
}

#ifdef never
int issymb(s)
AEop s;
{
        if (curlex.sym == s) { nextsym(); return 1; }
        return 0;
}
#endif

static void checkfor_ket(s)
AEop s;
{
        if (curlex.sym == s) nextsym();
        else syn_err("expected $s - inserted before $l", s);
}

static void checkfor_delimiter_ket(s,more)
AEop s;
char *more;
                            /* as checkfor_ket but less read-ahead */
{
        if (curlex.sym == s) curlex.sym = s_nothing;
        else syn_err("expected $s%s - inserted before $l", s, more);
}

static void checkfor_2ket(s,t)
AEop s;
AEop t;
{
        if (curlex.sym == s) nextsym();
        else syn_err("expected $s or $s - inserted $s before $l", s, t, s);
}

static void checkfor_delimiter_2ket(s,t)
AEop s;
AEop t;
{
    if (curlex.sym == s) curlex.sym = s_nothing;
    else checkfor_2ket(s, t);
}

/* now to get on with it. */

static Expr *rd_ANSIstring()
{
        StringSegList *p,*q;
/* Note that the list of string segments must last longer than most
   other parts of the parse tree, hence I use binder_list3() here */
        q = p = binder_list3(0, curlex.a1.s, curlex.a2.len);
        while (nextsym() == s_string)
            q = q->strsegcdr = binder_list3(0, curlex.a1.s, curlex.a2.len);
        return list2(s_string, p);
}

static ExprList *rd_exprlist()
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
static TypeExpr *cur_restype;
#ifndef NO_VALOF_BLOCKS
static TypeExpr *valof_block_result_type;
#endif

static Expr *rd_primaryexp(labelhack)
int labelhack;
{
    Expr *a;
    AEop op;
    switch (op = curlex.sym)
    {
default:
        syn_err("<expression> expected but found $s", op);
        return errornode;
case s_lpar:
        nextsym();
        if (isdeclstarter_(curlex.sym) ||   /* moan if stgclass in rd_declspec */
            (curlex.sym == s_identifier && istypedefname_(curlex.a1.sv)))
        {   TypeExpr *t = rd_typename();
            if (t==0) syserr("rd_typename()=0");
            checkfor_ket(s_rpar);
#ifndef NO_VALOF_BLOCKS
            if (curlex.sym == s_lbrace)
            {   Cmd *c;
                TypeExpr *saver = valof_block_result_type;
                if ((suppress & D_VALOFBLOCKS) == 0)
                   syn_err("{ following a cast will be treated as VALOF block");
/* Set a flag so that 'resultis' is recognized as such */
                if (equivtype(t, te_void))
                {   syn_rerr("void valof blocks are not permitted");
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
        a = rd_ANSIstring();
        break;
case s_identifier:
    {   Symstr *sv = curlex.a1.sv;
        int advanced = 0;   /* read-ahead as little as possible for ... */
        if (pp_inhashif)    /* ... error message line numbers.          */
        {   /* the following warning is a good idea - consider:
               enum foo { a,b }; #if a==b ... */
            syn_warn("Undefined macro $r in #if - treated as 0", sv);
            a = mkintconst(te_int,0,0);
        }
        else
        {   Binder *b;
            if (labelhack == POSSLABEL)
            {   nextsym(), advanced = 1;
                if (curlex.sym == s_colon)
                    /* note we do NOT nextsym() here so no postfix forms */
                    return list2(s_colon,sv);
            }
            if ((b = _symdata(sv)) == NULL)
            {   if (!advanced) nextsym(), advanced = 1;
                if (curlex.sym==s_lpar)
                {   if (warn_implicit_fns)
                        syn_warn("inventing 'extern int %s();'", _symname(sv));
                    else xwarncount++;
                }
                else
                  syn_rerr("Undeclared name, inventing 'extern int %s'",
                          _symname(sv));
                implicit_decl(sv, curlex.sym==s_lpar);
                a = (Expr *)(b = _symdata(sv));
            }
            else if (istypedefname_(sv)) /* macro retests b==NULL! */
            {   syn_err("typedef name $r used in expression context", sv);
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

static Expr *rd_postfix(a)
Expr *a;
{   AEop op;
        for (;;) switch (op = curlex.sym)
        {   case s_plusplus:
            case s_minusminus:
                nextsym();
                a = mkunary(postop_(op), a);
                break;
            case s_lpar:
                nextsym();
                a = mkfnap(a, ((curlex.sym == s_rpar) ? 0 : rd_exprlist()));
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
                    syn_err("Expected <identifier> after $s but found $l", op),
                    a = errornode;
                else a = mkfieldselector(op, a, curlex.a1.sv);
                nextsym();
                break;
            default:
                return a;
        }
}

static Expr *rd_prefixexp(labelhack)
int labelhack;
{
    AEop op;
    Expr *a;
    switch (op = curlex.sym)
    {
case s_and:
case s_times:
case s_plus:                            /* new ANSI feature */
case s_minus:   op = unaryop_(op);      /* drop through */
case s_plusplus:
case s_minusminus:
case s_bitnot:
case s_boolnot: nextsym();
                return mkunary(op, rd_prefixexp(NOLABEL));
case s_sizeof:
        nextsym();
/* N.B. Ansi require sizeof to return an unsigned integer type */
        if (curlex.sym == s_lpar)
        {   nextsym();
            if (isdeclstarter_(curlex.sym) || /* moan if stgclass in rd_declspec */
                (curlex.sym == s_identifier && istypedefname_(curlex.a1.sv)))
            {   TypeExpr *t = rd_typename();
                if (t==0) syserr("rd_typename()=0");
                checkfor_ket(s_rpar);
                return mkintconst(te_uint, sizeoftype(t),
                                           list2(s_sizeoftype,t));
            }
            a = rd_expr(PASTCOMMA);
            checkfor_ket(s_rpar);
            a = rd_postfix(a);   /* for sizeof (f)() etc */
        }
        else a = rd_prefixexp(NOLABEL);
        return mkintconst(te_uint, sizeoftype(typeofexpr(a)),
                                   list2(s_sizeofexpr,a));
default:
        return rd_postfix(rd_primaryexp(labelhack));
    }
}

static Expr *rd_expr(n)
int n;
{   AEop op;
    Expr *a = rd_prefixexp(n);    /* see POSSLABEL */
    /* note that the loop does not go round if op == s_colon after a label */                                  
    while (lprio_(op = curlex.sym) >= n)
    {   nextsym();
        if (op == s_cond)
        {   Expr *b = rd_expr(PASTCOMMA);
      /* this should at least evoke a warning although unambiguous */
            checkfor_ket(s_colon);
            a = mkcond(a, b, rd_expr(rprio_(op)));
        }
        else a = mkbinary(op, a, rd_expr(rprio_(op)));
    } 
    return a;  
}

/* the next routine is used by the preprocessor to parse #if and #elif */
long int syn_hashif()
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
     {   syn_err(curlex.sym == s_eof ? "EOF not newline after #if ..." :
                 "Junk after #if <expression>");
         while (curlex.sym != s_eof && curlex.sym != s_eol) nextsym();
     }
     if (e == 0 || h0_(e) != s_integer)
     {   if (e != 0) moan_nonconst(e, "#if <expression>");
         return 0;
     }
     return intval_(e);
}

static Expr *rd_condition(op)
AEop op;
{
    Expr *e;
    if (curlex.sym == s_lpar)
    {   nextsym();
        e = rd_expr(PASTCOMMA);
        checkfor_ket(s_rpar);
    }
    else
    { syn_rerr("parentheses (..) inserted around expression following $s", op);
      e = rd_expr(PASTCOMMA);
    }
    /* check more in case of switch */
    e = (op == s_switch ? mkcast(s_switch, e, te_int) : mktest(op,e));
    e = optimise0(e);
    return (e != 0) ? e : mkintconst(te_int,0,0);
}

/* the next routine rd_init() has a coroutine-like linkage with vargen.c */
/* it would otherwise be a nice simple recursive routine! */

static int syn_initdepth;  /* only used by these routines and rd_declrhslist */
static Expr *syn_initpeek;
int syn_undohack;          /* exported to vargen.c                           */

/* these are clearly stubs of nice recursive calls! */

int syn_begin_agg()
{   if (syn_initdepth < 0) return 0;
    if (curlex.sym == s_lbrace)
    {   nextsym();
        syn_initdepth++;
        return 1;
    }
    return 0;
}

void syn_end_agg(beganbrace)
int beganbrace;
{   if (beganbrace)
    {   if (curlex.sym != s_rbrace)
        {   switch (beganbrace)
            {   case 1: syn_err("too many initialisers in {} for aggregate");
                        break;
                case 2: syn_err("{} must have 1 element to initialise scalar or auto");
                        break;
            }
            while (syn_rdinit(0,4));     /* skip rest quietly */
        }
        else if (beganbrace == 2)
            syn_warn("spurious {} around scalar initialiser");
        nextsym();
        if (--syn_initdepth > 0 && curlex.sym != s_rbrace)
            checkfor_2ket(s_comma, s_rbrace);  /* @@@ improve w.r.t e.g int */
        if (syn_initdepth == 0) syn_initdepth = -1;
    }
}

/* flag values: 0 normal, 1 read string or unread, 4 skip silently */
Expr *syn_rdinit(t,flag)
TypeExpr *t;
int flag;
/* t is a non-aggregate type - read its initialiser */
/* t being a subscript type is treated specially for 'char x[] = "abc"' */
{   Expr *e;
    if (syn_undohack)
    {   syn_undohack = 0;
        if (syn_initpeek == 0) return 0;
        if (t) e = mkcast(s_assign,syn_initpeek,t);
        else { syserr("syn_rdinit"); return 0; }
        return optimise0(e);
    }
    if (syn_initdepth < 0) return 0;
    if (curlex.sym == s_rbrace) return 0;
    if (curlex.sym == s_lbrace)        /* nasty ANSI optional "int x = {1}" */
    {   if (flag == 1) return 0;   /* char x[] = {"abc"} illegal I presume */
        (void) syn_begin_agg();    /* always succeeds */
        e = rd_expr(UPTOCOMMA);
        if (t) e = mkcast(s_assign,e,t);
        e = optimise0(e);
        if (curlex.sym != s_rbrace)
            checkfor_2ket(s_comma, s_rbrace);  /* @@@ improve w.r.t e.g int */
        syn_end_agg(flag == 4 ? 3 : 2);            /* special flags */
        return e;
    }
    syn_initpeek = e = rd_expr(UPTOCOMMA);
    if (t) e = mkcast(s_assign,e,t);
    e = optimise0(e);
    if (syn_initdepth > 0 && curlex.sym != s_rbrace)
        checkfor_2ket(s_comma, s_rbrace);  /* @@@ improve w.r.t e.g int */
    if (syn_initdepth == 0) syn_initdepth = -1;  /* one only */
    return e;
}

bool syn_canrdinit()
{  if (syn_undohack) return 1;
   if (syn_initdepth < 0 || curlex.sym == s_rbrace) return 0;
   return 1;
}

/* command reading routines... */

static Cmd *rd_block(inner)
bool inner;
{
    DeclRhsList *d,*dp;
    CmdList  *c,*cq,*ct;
    Unbinder *oldvars;
    Untagbinder *oldtags;
    FileLine fl; fl.f = pp_cisname, fl.l = pp_linect;
    nextsym();
    if (inner) (oldvars = push_varenv(), oldtags = push_tagenv());
    d = rd_decllist(BLOCKHEAD|STGCLASS_OK|CONC_DECLARATOR);
    c = 0;
    for (dp = d; dp != 0; dp = dp->declcdr)  /* add dynamic inits    */
    {   Expr *e = dp->declinit;              /* see genstaticparts() */
        if (e != 0)
        {   if (debugging(DEBUG_SYN)) eprintf("[Init]");
            /* @@@ N.B. the line number may not be right in the next line */
            ct = ncons2_(mk_cmd_e(s_semicolon, fl, e));
            if (c == 0) c = cq = ct;
            else { cdr_(cq) = ct; cq = ct; }
        }
        ((BindList *)dp)->bindlistcar = dp->declbind;    /* re-use store */
    }
    while (curlex.sym != s_rbrace && curlex.sym != s_eof)
    {   ct = ncons2_(rd_command());
        if (curlex.sym == s_nothing) nextsym();
        if (c == 0) c = cq = ct;
        else { cdr_(cq) = ct; cq = ct; }
    }
    checkfor_delimiter_ket(s_rbrace, "");
/* the next line MUST be executed, even if a syntax error occurs to
   avoid global/local storage inconsistency after error recovery.     */
    if (inner) (pop_varenv(oldvars), pop_tagenv(oldtags));
/* note the BindList/DeclRhsList pun */
    return mk_cmd_block(fl, (BindList *)d, c);
}

static int isexprstarter(op)
AEop op;
{ switch (op)
    {   default: return 0;
        case s_lpar:    /* primary expression starters */
        case s_string: case s_integer: case s_floatcon:
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
   ahead further. This reduces spurious read-ahead.  AM changes.
*/

static Cmd *adddefault(fl)
FileLine fl;
{    if (cur_switch == 0)
     {   syn_err("'default' not in switch - ignored"); return 0; }
     if (switch_default_(cur_switch))
     {   syn_err("duplicate 'default' case ignored"); return 0; }
     return (switch_default_(cur_switch) = mk_cmd_default(fl, 0));
}

static Cmd *addcase(e,fl)
Expr *e;
FileLine fl;
{    Cmd *p,*q; int n;
     if (cur_switch == 0)
     {   syn_err("'case' not in switch - ignored");
         return 0;
     }
     if (h0_(e) != s_integer)
     {   moan_nonconst(e, "case expression (ignored)");
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
             eprintf("Comparing cases %d %d\n", n, intval_(t));
         if (n >= intval_(t)) 
         {   if (n > intval_(t)) break;
             syn_err("duplicated case constant: %d", n);
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


static Cmd *rd_command()
{
    AEop op;
    Cmd *c;
    Expr *e;
    FileLine fl; fl.f = pp_cisname, fl.l = pp_linect;
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
        e = optimise0(rd_expr(PASTCOMMA));
        checkfor_ket(s_colon);
        if (e==0 || (c = addcase(e,fl)) == 0) return rd_command(); /* error */
        cmd2c_(c) = rd_command();
        return c;
default:
        if (!isexprstarter(op))
        {   syn_err("<command> expected but found a $s", op);
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
        if (curlex.sym != s_while)
        {   syn_err("'while' expected after 'do' - found $l");
            return cmd1c_(c);
        }
        nextsym();
        cmd2e_(c) = rd_condition(s_while);
        break;
case s_while:
        nextsym();
        e = rd_condition(op);
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
        e = rd_condition(op);
        c = rd_command();
        if (curlex.sym == s_nothing) nextsym();
        {   Cmd *c2;
            if (curlex.sym == s_else)
            {   nextsym();
                c2 = rd_command();
                elselast = 1;
            }
            else
            {   c2 = 0;
                if (elselast) /* in 'c' above */
                    syn_warn("Dangling 'else' indicates possible error");
            }
            return mk_cmd_if(fl, e, c, c2);
        }
case s_else:
        syn_err("Misplaced 'else' ignored");
        nextsym();
        return rd_command();
case s_switch:
        nextsym();
        e = rd_condition(op);
        {   Cmd *oldswitch = cur_switch; AEop oldbreak = cur_break;
            cur_switch = c = mk_cmd_switch(fl, e,0,0,0);
            cur_break = s_endcase;
            cmd2c_(c) = rd_command();
            cur_switch = oldswitch; cur_break = oldbreak;
            return c;
        }
case s_return:
        nextsym();
        e = ((curlex.sym == s_semicolon) ? 0 : optimise0(
                mkcast(s_return, rd_expr(PASTCOMMA), cur_restype)));
        if (e != 0) implicit_return_ok = 0;
        if (e != 0 && equivtype(cur_restype, te_void))
            syn_rerr("return <expr> illegal for void function");
        if (e == 0 && !equivtype(cur_restype, te_void))
            syn_warn("non-value return in non-void function");
        c = mk_cmd_e(op, fl, e);
        break;
#ifndef NO_VALOF_BLOCKS
case s_resultis:
        nextsym();
        e = optimise0(
               mkcast(s_return, rd_expr(PASTCOMMA), valof_block_result_type));
        c = mk_cmd_e(op, fl, e);
        break;
#endif
case s_continue:
        if (cur_loop != 0) c = mk_cmd_0(op, fl);  /* list2(op,cur_loop)? */
        else { c=0; syn_err("'continue' not in loop - ignored"); }
        nextsym();
        break;
case s_break:
        if (cur_loop != 0 || cur_switch != 0) c = mk_cmd_0(cur_break, fl);
        else { c=0; syn_err("'break' not in loop or switch - ignored"); }
        nextsym();
        break;
case s_goto:
        nextsym();
        if (curlex.sym != s_identifier)
        {   syn_err("'goto' not followed by label - ignored");
            return rd_command();
        }
        /* the 0 in the next command is never used */
        c = mk_cmd_lab(op, fl, label_reference(curlex.a1.sv), 0);
        nextsym();
        break;
    }
    elselast = 0;
    checkfor_delimiter_ket(s_semicolon, " after command");
    return c;
}
      
static Cmd *rd_body(t)
TypeExpr *t;
{   cur_switch = 0;
    cur_loop = 0;
    cur_break = s_error;
    cur_restype = t;
    if (curlex.sym == s_lbrace)
        return rd_block(0);
    else
    {   syn_err("'{' of function body expected - found $l");
        return 0;
    }
}


/* rd_declspec() reads a possibly optional list of declaration-specifiers
   (May 86 ANSI draft section 3.5 page 49).  It can also read
   type-specifiers via bits in 'declflag'.
   NB. The following two comments need updating after merge of
   previous routines rd_typespec() and rd_stgclass().
*/
/* @@@ lie: It returns a bit map of storage classes due the
   fact that some of the ANSI storage classes
   seem merely to be adjectives - volatile/const.  Register naturally
   fits this classification too (we allow 'register auto x').
   Might we not later allow 'register static x;' to indicate slaving?
   Note that defaulting of storage class depends not only on context, but
   also on type - e.g. { int f(); ...} so it is done later.
*/
/* @@@ lie: declflag is a subset of declflag: only the TYPE_NEEDED and
   FORMAL bits arg inspected.  TOPLEVEL too now (AM check further) */
static TypeSpec *rd_declspec(declflag)
int declflag;
{
    SET_BITMAP illtype = 0, typesseen = 0;
    Binder *b = 0;  /* typedef or struct/union/enum tag binder record */
    SET_BITMAP illstg = 0, stgseen = 0;
    if (declflag & POINTERTYPES)
        illtype = TYPEDEFINHIBITORS;   /* leaves just const/volatile */
    if (!(declflag & (BLOCKHEAD|FORMAL|ARG_TYPES)))
        illstg |= bitofstg_(s_register) | bitofstg_(s_auto);
    if (declflag & (FORMAL|ARG_TYPES))
        illstg |= PRINCSTGBITS;
    for (;;)
    {   AEop s = curlex.sym;
        if (s == s_identifier && !(illtype & TYPEDEFINHIBITORS)
                              && istypedefname_(curlex.a1.sv))
            s = s_typedefname, b = _symdata(curlex.a1.sv),
            binduses_(b) |= u_referenced;
        else if (!isdeclstarter_(s)) break;
/* merge the next if-then-else sometime via table driven code. */
        if (isstorageclass_(s))
        {   if (!(declflag & STGCLASS_OK) || bitofstg_(s)&illstg)
                syn_err("storage class $s not permitted in context %s - ignored",
                        s, ctxtofdeclflag(declflag));
            else if (bitofstg_(s) &
                     (stgseen&PRINCSTGBITS ? stgseen|PRINCSTGBITS : stgseen))
                syn_err("storage class $s incompatible with $m - ignored",
                         s, stgseen);
            else
                stgseen |= bitofstg_(s);
            nextsym();
        }
        else
        {   SET_BITMAP typebit = bitoftype_(s);
            int trouble = illtype & typebit;
            if (trouble)
                 /* this code relies that for type symbols x,y, "x y" is
                    legal iff "y x" is. */
                 syn_err("type $s inconsistent with $m", s,
                         typesseen & illtypecombination[shiftoftype_(s)]);
            else
            {   
#ifdef TARGET_IS_ARM
                if (s == s_short && !(suppress & D_SHORTWARN))
                {   syn_warn("'short' slower than 'int' on this machine"
                             " (see manual)");
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
                int sawid = 0;
                /* Now, after "struct id" a ';' or '{' indicates a new
                   definition at the current nesting level */
                if (curlex.sym == s_identifier)
                {   Symstr *sv = curlex.a1.sv;
                    bool defining;
                    sawid = 1;
                    nextsym();
                    defining = (curlex.sym==s_semicolon || curlex.sym==s_lbrace);
                    b2 = findtagbinding(sv,s,defining);
                    if (defining)
                        typesseen |= B_DECLMADE;  /* police 'int;' error */
                }
                else b2 = gentagbinding(s);    /* anonymous */
                if (curlex.sym == s_lbrace)
                {   nextsym();
                    if (s==s_enum) 
                        typesseen |= B_DECLMADE; /* else other syntax error */
                    settagmems(b2, (s==s_enum ?
/* beware the next line.  We forge a type for recovery in the case
   of "short enum {...}".  However, this means that possible
   const/volatile are not put on enumeration constants.  This
   doesn't really matter since constants are constant anyway!!!
*/
                        rd_enumdecl(list3(s_typespec,typebit,b2), declflag)
                      : rd_strdecl()));
                    checkfor_ket(s_rbrace);
                }
                else if (!sawid)
                {   syn_err("'{' or <identifier> expected after $s, but found $l", s);
                    /* recovers for 'struct *a' or suchlike error. */
                }
                if (!trouble) b = (Binder *)b2;
            }
        }
    }
    if ((declflag & TYPE_NEEDED) && typesseen == 0)
        syn_rerr("Missing type specification - 'int' assumed");
    else if (!(declflag & FORMAL) && (typesseen|stgseen)==0)
        typesseen |= B_OMITTYPE; /* see rd_declrhslist for why we need this */
    if ((typesseen & NONQUALTYPEBITS) == 0)
    {   if ((typesseen|stgseen) != 0 || !(declflag & FORMAL))
            /* consider returning 0 for untyped formals?
               changes would be need for several routines, so pend */
            typesseen |= bitoftype_(s_int);
    }
    if (typesseen & bitoftype_(s_float))    /* normalise for rest of system */
        typesseen = (typesseen & ~bitoftype_(s_float)) |
                        (bitoftype_(s_double) | bitoftype_(s_short));
    return list3(s_typespec, stgseen|typesseen, b);
}

#define isfntype_(t) (h0_(t) == t_fnap)

/* the following macro checks for an arg which has not been typed */
#define is_untyped_(x) (h0_(x) == s_typespec && typespecmap_(x) == 0)

typedef TypeExpr Declarator;   /* funny inside-out Expr */
#define sub_declarator_(x) ((x)->typearg)

/* Declarators: read these backwards (inside out) to yield a name
 * (omitted if abstract declarator) and a TypeExpression (a la Algol68).
 * I can't think of any obvious algorithm for this so I read
 * the declarator AE structure and do an in-place pointer reverse
 * (not so tricky as it sounds).
 */
/* @@@ should add code to fault (and correct with *) d()(), d()[], d[]().
   Always remembering that "typedef int a[3]; a() {}" is illegal too.
*/

static int syn_minformals, syn_maxformals;
                       /* set by rd_formals, read by rd_declarator_1() */
static Symstr *declarator_name;
                       /* set by rd_declarator: 0 or Symstr *          */

/* abs_or_conc is a subset of declflag (q.v.).  Only the
 * needed_DECLARATOR, banned_DECLARATOR fields are inspected.
 * Now also TOPLEVEL to see whether we can allow the olde 'f(a,b)' form.
 */
static Declarator *rd_declarator_1(abs_or_conc)
int abs_or_conc;
{
    Declarator *a;
    switch (curlex.sym)
    {
default:
        if (abs_or_conc & needed_DECLARATOR)
        {   syn_err("Expecting <declarator> or <type>, but found $l");
            if (curlex.sym == s_rbrace) nextsym();
            return (Declarator *)errornode;
        }
        a = list1(s_nothing);
        break;
case s_identifier:
        if (abs_or_conc & banned_DECLARATOR)
        {   syn_err("Identifier (%s) found in <abstract declarator> - ignored",
                    _symname(curlex.a1.sv));
            a = list1(s_nothing);
        }
        else a = (Declarator *)curlex.a1.sv; /* curlex.a1.sv is a Symstr with s_id */
        nextsym();
        break;
case s_lpar:
/* @@@ the next line pends on ANSI's final views (e.g. can "int (a)" be
   an abstract declarator of a fn of 1 arg?).  However, in the meantime
   we still have to follow K&R's rule that in an abstract declarator
   () means application rather than enclosing missing name - consider
   "(*(int ())foo)()" which casts foo to be "int ()" not "int".  Yuk.
*/
        nextsym();
        if (curlex.sym == s_rpar && (abs_or_conc & banned_DECLARATOR))
        {   nextsym();
            a = list6(t_fnap, list1(s_nothing), 0, 0, 999, 0); /* minargs_ */
            break;
        }
        a = rd_declarator_1(abs_or_conc);
        if (h0_(a) == s_error) return a;
        checkfor_ket(s_rpar);
        break;
case s_times:
        {   TypeSpec *p;
            nextsym();
            p = rd_declspec(POINTERTYPES);
            a = rd_declarator_1(abs_or_conc);
            if (h0_(a) == s_error) return a;
            /* the next line has to mask off b_synbits & bitoftype_(s_int) */
            a = list3(t_content, a, typespecmap_(p) & (bitoftype_(s_const) | bitoftype_(s_volatile)));
            break;
        }
    }
    for (;;) switch (curlex.sym)
    {
case s_lpar:    nextsym();
      /* NB: K&R only allows empty ()'s here.  It is then extremely
         hard to predict a fn definition (unlimited lookahead
         according to the K&R grammar).   Anyway ANSI allow
         parameters anytime (even in abstract declarators?) */
            { DeclRhsList *f = rd_formals();
              /* extra results for minargs_(), maxargs_()... */
              a = list6(t_fnap, a, f, syn_minformals, syn_maxformals,
                                      special_variad);
            }
            checkfor_ket(s_rpar);
            break;
case s_lbracket:
            nextsym();
            a = list3(s_subscript, a,
                  ((curlex.sym==s_rbracket) ? 0 : check_arraysize(rd_expr(PASTCOMMA))));
            checkfor_ket(s_rbracket);
            break;
default:    return a;
    }
}


/* rd_declarator() returns 0 in the event that it failed to find a
   declarator.  Note that this can only happen if 'CONC_DECLARATOR' is set.
*/
static TypeExpr *rd_declarator(abs_or_conc,basictype)
int abs_or_conc;
TypeExpr *basictype;
{   Declarator *x = rd_declarator_1(abs_or_conc);
    for (;;) switch (h0_(x))
    {   case s_error: return 0;
        default: syserr("rd_declarator(%d)", h0_(x));
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
                {   syn_rerr("Omitted <type> before formal declarator - 'int' assumed");
                    basictype = te_int;
                }
                sub_declarator_(x) = (Declarator *)basictype;
                basictype = (TypeExpr *)x;
                x = temp;
                break;
            }
    }
}

static TypeExpr *rd_typename()
{
    TypeExpr *t = rd_declspec(TYPE_NEEDED | ABS_DECLARATOR);
                             /*      & ~STGCLASS_OK       */
    typespecmap_(t) &= ~b_synbits;   /* allow empty declarations */
    return rd_declarator(ABS_DECLARATOR, t);
    /* ignore the value in declarator_name as abstract declarator. */
}

/* note that in general we cannot default storageclasses on parsing them
   due to the differing default classes in "{ int a,f();...}".
   On the other hand nonsensical combinations SHOULD be detected by
   rd_typename().
*/
static void defaultstorageclass(d,declflag)
DeclRhsList *d;
int declflag;
{   TypeExpr *t;
    if ((declflag & STGCLASS_OK) && (d->declstg & PRINCSTGBITS) == 0)
    {   t = d->decltype;
        if (declflag & BLOCKHEAD)
            /* note on the next line we do not prune typedef's with
               prunetype().  This means that (@@@ check ANSI)
               "typedef int a(); a b;" is not counted as a function
               and so will be faulted in when defaulted to 'auto' say.
            */
            d->declstg |= isfntype_(t) ? bitofstg_(s_extern) : bitofstg_(s_auto);
        else if (declflag & TOPLEVEL)
            d->declstg |= isfntype_(t) ? bitofstg_(s_extern)
                                      : bitofstg_(s_extern)|b_omitextern;
        else if (declflag & (FORMAL|ARG_TYPES))
            d->declstg |= bitofstg_(s_auto);   /* was s_argclass */
        else syserr("defaultstorageclass(0x%x)", declflag);
    }
}

static void ensure_formals_typed(d,proto)
DeclRhsList *d;
int proto;
{   for (; d; d = d->declcdr)
    {   TypeExpr *t = d->decltype;
        if (is_untyped_(t))
        {   if (proto)
                /* @@@ ensure that f(,) error has not got this far */
                syn_rerr("function prototype formal $r needs type or class - 'int' assumed", d->declname);
            else
                /* god knows why ANSI do not consider this an error */
                syn_warn("formal parameter $r not declared - 'int' assumed", d->declname);
            d->decltype = te_int;
        }
    }
}

static void merge_formal_decl(d,dnew)
DeclRhsList *d;
DeclRhsList *dnew;
{
/* Here I have a formal parameter (d) which is now being given an        */
/* explicit declaration (dnew).  Merge new information in with the old.  */
    TypeExpr *t = d->decltype;
    if (!is_untyped_(t))  /* previous type info */
        syn_err("duplicate type specification of formal parameter $r",
                d->declname);
    d->declstg = dnew->declstg;
    d->decltype = dnew->decltype;
}

/* but for its size rd_declrhslist() would be part of rd_decl()
   maybe it will become so anyway!  It reads any of K&R's
  "(last part of undefined category)type-decl-list(see function-body)",
  "init-declarator-list" or "struct-declarator-list"
*/

static int topfnflag;  /* for use of rd_declrhslist() and rd_decl() ONLY! */
static DeclRhsList *syn_formals;  /* ditto - now also rd_fndef() */

static DeclRhsList *rd_declrhslist(ss,tt,declflag)
SET_BITMAP ss;
TypeSpec *tt;
int declflag;
/* NB: possibility of allowing both length and init as extension. */
{
    DeclRhsList *p,*q;
    /* note - do NOT change ss, tt or declflag since used in loop below */
    for (p = 0; ;)                           /* 3 exits - all 'return's */
    {   DeclRhsList *const temp = ncons6_(0, 0, 0, ss, 0);
        if ((declflag & LENGTH_OK) && curlex.sym == s_colon)
            temp->decltype = tt;             /* stash away type, 0 declaree */
        else
        {   if ((temp->decltype = rd_declarator(declflag,tt)) == 0)
            {   /* error in rd_declarator already reported */
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
        if (isfntype_(temp->decltype))           /* but NOT via typedef */
        {   /* see if the fn declaration is a fn definition, we have
               already (in rd_formals()) changed f(void) to f()         */
            TypeExpr *fntype = temp->decltype;
            DeclRhsList *fnpars = typefnargs1_(fntype);
            temp->declstg |= b_fnconst;
            if ((declflag & TOPLEVEL) &&      /* top level */
                p == 0 &&                     /* first in list */
                !(temp->declstg & bitofstg_(s_typedef)) &&
                (  isdeclstarter_(curlex.sym) ||  /* body or param starter */
                   (curlex.sym == s_identifier && istypedefname_(curlex.a1.sv)) ||
                   curlex.sym == s_lbrace))
            {   if (fnpars == 0) maxargs_(fntype) = 0;
                    /* i.e. treat f() {} as f(void) {} */
                topfnflag = 1;
                return temp;
            }
            temp->declstg |= b_undef;
            /* The ANSI draft disallows 'extern f(a,b);' (types needed) */
            ensure_formals_typed(fnpars, 1);
        }
        if (isfntype_(prunetype(temp->decltype))) /* possibly via typedef */
        {   if (!(temp->declstg & b_fnconst) ||
                !(temp->declstg & (bitofstg_(s_extern)|
                     bitofstg_(s_typedef)|bitofstg_(s_static))))
            {   syn_rerr("%s $r may not be function - assuming function pointer",
                        temp->declstg&PRINCSTGBITS ?
                           "variable" : "struct component",
                        temp->declname);
                temp->decltype = ptrtotype_(temp->decltype);
                temp->declstg &= ~(b_fnconst | b_undef);
                /* treat typedef int a(); extern a x; as extern a *x; */
            }
            else if (maxargs_(prunetype(temp->decltype)) == 999)
            { if (warn_deprecated)
              {
                /* I can see I will have to justify this warning message
                   very carefully.  Firstly, system builders need a way
                   to ensure that (e.g. CLIB) has no olde-style
                   function specs.  Moreover, AM has just accidentally
                   discovered an uncheckable bug in the compiler of the
                   form "extern f(); g() { f(1); } f(int x,int y) {...}"
                   I want to know there are no more.  Others may too.
                */
                syn_warn("Deprecated declaration %s() - give arg types",
                         _symname(temp->declname));
              }
              else xwarncount++;
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
            if (p==0) syn_err("Non-formal $r in parameter-type-specifier", sv);
        }
        if (declflag & (TOPLEVEL|BLOCKHEAD))    /* was INIT_OK */
        {   int initflag = 0;
            if (temp->declstg & bitofstg_(s_extern) &&
                !(temp->declstg & b_omitextern)) temp->declstg |= b_undef;
            if (curlex.sym == s_assign)
            {   if (temp->declstg & bitofstg_(s_extern) &&
                    declflag & TOPLEVEL)  /* extern init only at top level */
                        temp->declstg &= ~b_undef;
                if (temp->declstg & b_fnconst)
                {   syn_rerr("function $r may not be initialised - assuming function pointer",
                            temp->declname);
                    temp->decltype = ptrtotype_(temp->decltype);
                    temp->declstg &= ~(b_fnconst | b_undef);
                }
            }
            /* do the next line AFTER above type patching but before
               reading possible initialiser.  Save Binder in ->declbind */
            temp->declbind = instate_declaration(temp, declflag);
            if (curlex.sym == s_assign)
            {   if (temp->declstg & (bitofstg_(s_auto)|bitofstg_(s_static)) ||
                     (temp->declstg & (bitofstg_(s_extern)|b_undef)) ==
                                      (bitofstg_(s_extern)))
                    initflag = 1;
                else 
                    syn_err("$m variables may not be initialised",
                            temp->declstg),
                    initflag = 2;
                nextsym();
            }
            if (initflag == 2) (void)syn_rdinit(0,4);
            syn_initdepth = (initflag == 1) ? 0 : -1;
            syn_undohack = 0;
            temp->declinit = genstaticparts(temp, declflag & TOPLEVEL);
        }
        /* do not instate formals (or in casts "(int ()(int a,b))foo")
           nor structure elements. */
        if (declflag & LENGTH_OK)
        {   TypeExpr *t = prunetype(temp->decltype);
            if (curlex.sym == s_colon)
            {   nextsym();
                /* ->declinit also holds the bit size expr temporarily */
                temp->declinit = check_bitsize(rd_expr(UPTOCOMMA));
                if (h0_(t) != s_typespec || (typespecmap_(t) & ~(bitoftype_(s_int)|
                                  bitoftype_(s_unsigned)|bitoftype_(s_signed))))
                {   syn_err("illegal bit field type $t - 'int' assumed",
                        h0_(t) != s_typespec ? t :
                            primtype_(typespecmap_(t) & ~(bitoftype_(s_int)|bitoftype_(s_unsigned)|bitoftype_(s_signed))));
                    t = te_int;
                }
                temp->decltype = primtype_(typespecmap_(t) | BITFIELD);
            }
            else if (h0_(t) == s_typespec && (typespecmap_(t) &
                                (bitoftype_(s_struct)|bitoftype_(s_union))))
            {   TagBinder *b = typespectagbind_(t);
                if (!tagbindmems_(b))
                {   syn_err("undefined struct/union $b cannot be member",b);
                    /* This is what ANSI suggest and disallows circular types
                        however, the rest of our code could allow more
                        generous treatment.  Fix up as pointer to same. */
                    temp->decltype = ptrtotype_(temp->decltype);
                }
            }
        }
        if (p == 0) p = q = temp;
        else { q->declcdr = temp; q = temp; }
        if ((declflag & FORMAL) || curlex.sym != s_comma)
        {   topfnflag = 0;
            return p;
        }
        nextsym();
    }
}

static TopDecl *rd_fndef(d,declflag)
DeclRhsList *d;
int declflag;
/* d has exactly 1 item */
        {   TypeExpr *fntype = d->decltype;        /* known to be fn type */
            Binder *fnbind; Cmd *body;
            DeclRhsList *fpdecllist = typefnargs1_(fntype), *fpe;
            BindList *lambdap = 0, *lambdaq;
            BindList *narrowformals = 0; Expr *arginit = 0;
            /* syn_formals is examined by rd_declrhslist if ARG_TYPE is set */
            syn_formals = fpdecllist;
            currentfunction = _symname(d->declname);
            if (debugging(DEBUG_FNAMES))
                eprintf("Start of function %s\n", currentfunction);
/* when ARG_TYPES is set in declflag any identifiers declared will have  */
/* been seen before (as formal parameters) and rd_decllist works by      */
/* updating these existing declaration records in a side-effect-full way */
            (void) rd_decllist(CONC_DECLARATOR|STGCLASS_OK|ARG_TYPES);
/* do some checking and defaulting on the arguments and types BEFORE
   instate_decl ... */
            for (fpe = fpdecllist; fpe != 0; fpe = fpe->declcdr)
            {   if (fpe->declname == 0)
                {   syn_rerr("formal name missing in function DEFINITION");
                    /* fixup so we can continue ... */
                    fpe->declname = gensymval(1);
                }
            }
            ensure_formals_typed(fpdecllist, 0);
/* do the declaration of the function - this will copy the types just
 * updated in fpdecllist as part of globalize_ing the type of d.
 */
            fnbind = instate_declaration(d, declflag);
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
                Binder *wb;
                if (wt == at)            /* pointer equality is fine here */
                    wb = instate_declaration(fpe, FORMAL);
                else
#ifndef TARGET_IS_XPUTER
                {   Binder *ab; Expr *e;
                    DeclRhsList *narrowedformal = ncons6_(fpe->declname, at,
                                          fpe->declinit,   /* should be 0 */
                     /* was s_argclass */ fpe->declstg, 0);
                    DeclRhsList *wideformal = ncons6_(fpe->declname, wt,
                                          fpe->declinit,   /* should be 0 */
                                          fpe->declstg, 0);
                    /* do the original binder second so it gets seen first */
                    wb = instate_declaration(wideformal, FORMAL);
                    ab = instate_declaration(narrowedformal, FORMAL|DUPL_OK);
                    binduses_(wb) |= u_referenced;
                    e = mkassign(s_assign, (Expr *)ab, (Expr *)wb);
                        /* do the narrowing */
                    narrowformals = list2(narrowformals, ab);
                    arginit = arginit ? mkbinary(s_comma, arginit, e) : e;
                }
#else
		{
                    DeclRhsList *wideformal = ncons6_(fpe->declname, wt,
                                          fpe->declinit,   /* should be 0 */
                                          fpe->declstg, 0);
                    wb = instate_declaration(wideformal, FORMAL);
		}
#endif
                if (lambdap == 0) lambdap = lambdaq = mkBindList(0,wb);
                else lambdaq = lambdaq->bindlistcdr = mkBindList(0,wb);
            }
            {   Cmd *argstuff = 0;
                FileLine fl; fl.f = pp_cisname, fl.l = pp_linect;
                if (narrowformals)
                    argstuff = mk_cmd_e(s_semicolon, fl, optimise0(arginit));
   bind_level = 1;                               /* @@@ NASTY - fix soon */
                body = rd_body(typearg_(fntype));
   bind_level = 0;                               /* @@@ NASTY - fix soon */
                if (argstuff)
                    body = mk_cmd_block(fl, narrowformals,
                                        list2(list2(0,body), argstuff));
            }
            label_resolve();
            pop_varenv(0);    /*  see comment at other call */
            pop_tagenv(0);
/* h4_(result) is a flag to indicate the presence of an ellipsis in      */
/* the list of formal arguments for this function.                       */
            return list5(s_fndef, fnbind, lambdap, body,
                         maxargs_(fntype)==1999);
        }


/* rd_decl reads a possibly top-level decl, see also rd_decl2()       */
static TopDecl *rd_decl(declflag)
int declflag;
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
    int declxtramap = typespecmap_(t) & b_synbits;
/* we are required to complain at top level for "*x;", but not "f();"   */
    typespecmap_(t) &= ~ (STGBITS|b_synbits);           /* not necessary? */
    if (curlex.sym == s_semicolon && !(declflag & FORMAL))
    {   d = 0;  /* empty */
        if (!(declxtramap & B_DECLMADE))
            syn_rerr("declaration with no effect");
    }
    else
    {   d = rd_declrhslist(s, t, declflag);
        if (topfnflag)
        {   /* the next line allows static f(); - need type soon */
            if (declxtramap & B_OMITTYPE)
            {   if (suppress & D_IMPLICITVOID)
                    /* The next line allows us also to avoid the       */
                    /* 'implicit return' warning in f(){}.             */
                    xwarncount++, implicit_return_ok = 1;
                else
                    syn_warn("'int %s()' assumed - 'void' intended?",
                             _symname(d->declname));
            }
            return rd_fndef(d, declflag);        /* d != 0 by fiat */
        }
        /* this message is a little late, but cannot occur 'on time' */
        if (declxtramap & B_OMITTYPE && d != 0)
            /* the case d==0 has already been reported by          */
            /* rd_declarator() via rd_declrhslist() (q.v.).        */
            syn_rerr("type or class needed (except in function DEFINITION) - 'int' assumed");
    }
    if ((declflag & FORMAL) == 0) checkfor_delimiter_2ket(s_semicolon, s_comma);
    return list2(s_decl, d);
}

static DeclRhsList *rd_decl2(declflag)
int declflag;
{   TopDecl *d = rd_decl(declflag);
    if (d == 0 || h0_(d) != s_decl)
         syserr("rd_decl2(%d,%d)", d, d==0 ? 0 : h0_(d));
    return d->v_f.var;
}

/* rd_formals() is only used once in rd_declarator_1() */
/* remember we have to cope with the new ANSI format fns like:
     "f(register int a,b) float b; {...}".
   Modulo concrete syntax it is identical to rd_decllist().  unify??? */
static DeclRhsList *rd_formals()
{   DeclRhsList *p,*q,*temp;
    int minformals = -1;
    if (curlex.sym == s_rpar)
    {   syn_minformals = 0, syn_maxformals = 999;
        return 0;
    }
    for (p = 0;;)
    {   if (curlex.sym == s_ellipsis)
        {   nextsym();
            syn_minformals = length((void *)p), syn_maxformals = 1999;
/* @@@ the last line is a gross hack (abetted by AM) */
            return p;    /* to checkfor_ket(s_rpar) */
        }
#ifdef EXTENSION /* allow optional, but type-checked parameters. */
        if (curlex.sym == s_cond) nextsym(), minformals = length((void *)p);
#endif
        temp = rd_decl2(FORMAL|STGCLASS_OK);
        if (curlex.sym == s_nothing) nextsym();
        for (; temp != 0; temp = temp->declcdr)
        {   if (debugging(DEBUG_BIND)) eprintf(" Formal: ");
            if (p == 0) p = q = temp; else q->declcdr = temp, q = temp;
            if (debugging(DEBUG_BIND)) pr_id(q->declname);
        }
        /* all that should legally appear here are ',' and ')', but fix    */
        /* ';' as ',' a la pascal and treat all other symbols as ')' to be */
        /* faulted by the caller (rd_declarator_1()).                      */
        if (curlex.sym == s_semicolon)   /* error recovery */
            syn_rerr("',' (not ';') separates formal parameters");
        else if (curlex.sym != s_comma)
        {   /* arg list end, but first check for ANSI  "(void)" argument list
             * and remove it.  Note that "f(void) {}" is also OK by ANSI
             * if somewhat curious.
             */
            if (p != 0 && p->declcdr == 0 &&                /* 1 parameter */
                p->declname == 0 &&                         /* no name     */
                equivtype(p->decltype, te_void))      /* void (or typedef) */
                    p = 0;                            /* then clear arglist */
            syn_maxformals = length((void *)p);
            syn_minformals = minformals >= 0 ? minformals : syn_maxformals;
            return p;
        }
        nextsym();
    }
}


static DeclRhsList *rd_decllist(declflag)
int declflag;
/* reads a list of Decls and NCONC's them togther.  Used for struct/union
   elements, type specs after formal lists, rd_block().  NB:  It should not
   be used when TOPLEVEL is set as it does not look for functions.
   Identical to rd_formals modulo concrete syntax - unify???
   Note the Symstr's may be 0 (abstract), or s_id.
*/
{   DeclRhsList *p,*q,*temp;
    p = 0;
    while (isdeclstarter_(curlex.sym) ||
           (curlex.sym == s_identifier && istypedefname_(curlex.a1.sv)))
    {   temp = rd_decl2(declflag);
        if (curlex.sym == s_nothing) nextsym();
        for (; temp != 0; temp = temp->declcdr)
        {   if (debugging(DEBUG_BIND)) eprintf(" Identifier: ");
            if (p == 0) p = q = temp; else q->declcdr = temp, q = temp;
            if (debugging(DEBUG_BIND)) pr_id(q->declname);
        }
    }
    return p;
}

static TagMemList *rd_strdecl()
/* Currently this just mungs the structure returned by rd_decllist().
   AM Confession: extreme cowardice and the knowledge of further change
   means that I use global_list store everywhere.  Grovel.
   *** BEWARE TagMemList/DeclRhsList conversion *** Tally ho!
*/
{   DeclRhsList *l = rd_decllist(CONC_DECLARATOR|TYPE_NEEDED|LENGTH_OK);
    Symstr *sv;
    TagMemList *p = 0, *q, *temp;
/* TagMemList's are like BindList's except that curlex.a1's are stored direct */
    for (; l != 0; l = l->declcdr)
        {   if (debugging(DEBUG_TYPE)) eprintf(" Member: ");
            sv = l->declname;
            if (sv != 0 && h0_(sv) != s_identifier)
                /* sv would be 0 for padding (:0) bit fields */
                syserr("rd_strdecl");
/* invent a globalize_tagmemlist()? */
            temp = global_list4(0, sv,
                                globalize_typeexpr(l->decltype),
                                /* BITFIELD size or zero: */
                l->declinit == 0 ? 0 : globalize_int(evaluate(l->declinit)));
            stuse_type += 16;
            if (p == 0) p = q = temp; else q->memcdr = temp, q = temp;
            if (debugging(DEBUG_TYPE)) pr_id(q->memsv);
        }
    return p;
}

/* rd_enumdecl() works very much like rd_decllist() or rd_strdecl() but the
   different surface syntax leads me to implement it separately.
   Syntax is taken from Harbison&Steele.
   ANSI draft means 'int' implementation of 'enum's.
*/

static TagMemList *rd_enumdecl(t,istop)
TypeExpr *t;
int istop;
/* istop is a subset of declflag - only TOPLEVEL inspected */
{   DeclRhsList *p,*q,*temp;
    int nextenumval = 0;
    for (p = 0;;)
    {   if (curlex.sym == s_identifier)
        {   Symstr *sv = curlex.a1.sv; Expr *e = 0; Binder *b;
            temp = ncons6_(sv, t, 0, b_enumconst, 0);
            nextsym();
            if (curlex.sym == s_assign)
            {   nextsym();
                e = optimise0(rd_expr(UPTOCOMMA));
            }
            if (e != 0) nextenumval = evaluate(e);
            b = instate_declaration(temp, istop);
            bindaddr_(b) = nextenumval++;
            if (p == 0) p = q = temp; else q->declcdr = temp, q = temp;
        }
        else
        {   syn_err("<identifier> expected but found $l in 'enum' definition");
            while (curlex.sym != s_rbrace && curlex.sym != s_eof) nextsym();
        }
        if (curlex.sym != s_comma) break;
        nextsym();
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

TopDecl *rd_topdecl()
{
    implicit_return_ok = 0;      /* for junky old C programs         */
#ifndef NO_VALOF_BLOCKS
    inside_valof_block = 0;
    valof_block_result_type = DUFF_ADDR;
#endif
    if (curlex.sym == s_nothing) nextsym();
    while (curlex.sym == s_lbrace)  /* temp for ACN - rethink general case */
    {   syn_err("Misplaced '{' at top level - ignoring block");
        (void)rd_body(te_int);   /* this will also skip initialisers */
        label_resolve();         /* tidy up in case declarations */
        pop_varenv(0);
        pop_tagenv(0);
        if (curlex.sym == s_nothing) nextsym();
    }
    return rd_decl(CONC_DECLARATOR|TOPLEVEL|STGCLASS_OK);
}

void syn_init()
{   initpriovec();
}

/* End of syn.c */
