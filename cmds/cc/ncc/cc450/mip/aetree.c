/*
 * C compiler file aetree.c, version 38
 * Copyright (C) Codemist Ltd., 1987.
 * Copyright (C) Acorn Computers Ltd., 1988
 */

/*
 * RCS $Revision: 1.1 $
 * Checkin $Date: 1993/07/14 14:07:18 $
 * Revising $Author: nickc $
 */

#ifdef __STDC__
#  include <stdarg.h>
#else
#  include <varargs.h>
#endif
#include <stddef.h>
#include <ctype.h>

#include "globals.h"
#include "aetree.h"
#include "defs.h"
#include "store.h"
#include "aeops.h"
#include "builtin.h"

AEop tagbindsort(TagBinder *b)
{   return attributes_(b) & bitoftype_(s_enum) ? s_enum :
           attributes_(b) & bitoftype_(s_struct) ? s_struct :
           attributes_(b) & bitoftype_(s_class) ? s_class :
           attributes_(b) & bitoftype_(s_union) ? s_union : s_nothing;
}

/* Expr parse tree constructors */
Expr *mk_expr1(AEop op, TypeExpr *t, Expr *a1)
{
    return (Expr *) syn_list4(op, t, 0, a1);
}

Expr *mk_expr2(AEop op, TypeExpr *t, Expr *a1, Expr *a2)
{
    return (Expr *) syn_list5(op, t, 0, a1, a2);
}

Expr *mk_exprlet(AEop op, TypeExpr *t, SynBindList *a1, Expr *a2)
{
    return (Expr *) syn_list5(op, t, 0, a1, a2);
}

Expr *mk_expr3(AEop op, TypeExpr *t, Expr *a1, Expr *a2, Expr *a3)
{
    return (Expr *) syn_list6(op, t, 0, a1, a2, a3);
}

#ifdef EXTENSION_VALOF
Expr *mk_expr_valof(AEop op, TypeExpr *t, Cmd *c)
{
    return (Expr *) syn_list4(op, t, 0, c);
}
#endif

Expr *mk_exprwdot(AEop op, TypeExpr *t, Expr *a1, int32 a2)
{
    return (Expr *) syn_list5(op, t, 0, a1, a2);
}

Expr *mk_exprbdot(AEop op, TypeExpr *t, Expr *a1, int32 a2, int32 a3,
                  int32 a4)
{
    return (Expr *) syn_list7(op, t, 0, a1, a2, a3, a4);
}

DeclRhsList *mkDeclRhsList(Symstr *sv, TypeExpr *t, SET_BITMAP s)
{   /* slowly becoming local to syn.c */
    DeclRhsList *p = (DeclRhsList *) SynAlloc(sizeof(DeclRhsList));
    p->declcdr  = 0;  p->declname = sv;  p->decltype = t;
    declinit_(p) = 0; /* p->decbits = 0; */
    p->declstg  = s;  p->declbind = 0;
    p->fileline.f = NULL; p->fileline.l = 0;
#ifdef PASCAL /*ECN*/
    p->synflags = 0;
#endif
    return(p);
}

/* Hmm.  The last arg to this varies wildly in type */
TypeExpr *mk_typeexpr1(AEop op, TypeExpr *t, Expr *a1)
{
    return (TypeExpr *) syn_list4(op, (int32)t, (int32)a1, 0);
}

TopDecl *mkTopDeclFnDef(AEop a, Binder *b, SynBindList *c, Cmd *d, bool e)
{
    return (TopDecl *)syn_list5(a, (int32)b, (int32)c, (int32)d, (int32)e);
}

TypeExpr *mkTypeExprfn(AEop a, TypeExpr *b, FormTypeList *c,
                       const TypeExprFnAux *d)
{
/*
 * The fnaux field in a TypeExpr contains a RealRegSet, the size of which
 * depends on the number of registers that our target has. For this reason,
 * we allocate the TypeExpr in Binder store rather than syntax store, so that
 * regalloc can locate and use the RealRegSet value.
 */
    TypeExpr *t = (TypeExpr *)BindAlloc(sizeof(TypeExpr));
    t->h0 = a;
    t->typearg = b;
    t->typespecbind = (Binder *)c;      /* should be union soon, not cast. */
    t->dbglanginfo = 0;
    t->fnaux = *d;  /* At least 2 words, more if > 32 real registers exist */
#ifdef PASCAL /*ECN*/
    t->pun.type = syn_list3(t_fnap, 0, 0);
#endif
    return t;
}

TypeExpr *g_mkTypeExprfn(AEop a, TypeExpr *b, FormTypeList *c,
                         const TypeExprFnAux *d)
{
    TypeExpr *t = (TypeExpr *)GlobAlloc(SU_Type, sizeof(TypeExpr));
    t->h0 = a;
    t->typearg = b;
    t->typespecbind = (Binder *)c;      /* should be union soon, not cast. */
    t->dbglanginfo = 0;
    t->fnaux = *d;
#ifdef PASCAL /*ECN*/
    t->pun.type = global_list3(SU_Type, t_fnap, 0, 0);
#endif
    return t;
}

/* command nodes... */
Cmd *mk_cmd_0(AEop op, FileLine x)   /* op = s_break,s_endcase,s_continue */
{
    Cmd *p = (Cmd *) SynAlloc(offsetof(Cmd,cmd1));
    p->fileline = x;
    h0_(p) = op;
    return p;
}

Cmd *mk_cmd_e(AEop op, FileLine x, Expr *e)   /* op = s_return,s_semicolon */
{
    Cmd *p = (Cmd *) SynAlloc(offsetof(Cmd,cmd2));
    p->fileline = x;
    h0_(p) = op, cmd1e_(p) = e;
    return p;
}

Cmd *mk_cmd_default(FileLine x, Cmd *c)
{
    Cmd *p = (Cmd *) SynAlloc(offsetof(Cmd,cmd2));
    p->fileline = x;
    h0_(p) = s_default, cmd1c_(p) = c;
    return p;
}

Cmd *mk_cmd_lab(AEop op, FileLine x, LabBind *b, Cmd *c)
{   /* op = s_colon,s_goto */
    Cmd *p = (Cmd *) SynAlloc(offsetof(Cmd,cmd3));
    p->fileline = x;
    h0_(p) = op, cmd1c_(p) = (Cmd *)b, cmd2c_(p) = c;
    return p;
}

Cmd *mk_cmd_block(FileLine x, SynBindList *bl, CmdList *cl)
{
    Cmd *p = (Cmd *) SynAlloc(offsetof(Cmd,cmd3));
    p->fileline = x;
    h0_(p) = s_block, cmd1c_(p) = (Cmd *)bl, cmd2c_(p) = (Cmd *)cl;
    return p;
}

Cmd *mk_cmd_do(FileLine x, Cmd *c, Expr *e)
{
    Cmd *p = (Cmd *) SynAlloc(offsetof(Cmd,cmd3));
    p->fileline = x;
    h0_(p) = s_do, cmd1c_(p) = c, cmd2e_(p) = e;
    return p;
}

Cmd *mk_cmd_if(FileLine x, Expr *e, Cmd *c1, Cmd *c2)
{
    Cmd *p = (Cmd *) SynAlloc(offsetof(Cmd,cmd4));
    p->fileline = x;
    h0_(p) = s_if, cmd1e_(p) = e, cmd2c_(p) = c1, cmd3c_(p) = c2;
    return p;
}

Cmd *mk_cmd_switch(FileLine x, Expr *e, Cmd *c1, Cmd *c2, Cmd *c3)
{
    Cmd *p = (Cmd *) SynAlloc(sizeof(Cmd));
    p->fileline = x;
    h0_(p) = s_switch, cmd1e_(p) = e, cmd2c_(p) = c1,
                       cmd3c_(p) = c2, cmd4c_(p) = c3;
    return p;
}

Cmd *mk_cmd_for(FileLine x, Expr *e1, Expr *e2, Expr *e3, Cmd *c)
{
    Cmd *p = (Cmd *) SynAlloc(sizeof(Cmd));
    p->fileline = x;
    h0_(p) = s_for, cmd1e_(p) = e1, cmd2e_(p) = e2,
                    cmd3e_(p) = e3, cmd4c_(p) = c;
    return p;
}

/* for 'case' labels of a switch */
Cmd *mk_cmd_case(FileLine x, Expr *e, Cmd *c1, Cmd *c2)
{
    Cmd *p = (Cmd *) SynAlloc(sizeof(Cmd));
    p->fileline = x;
    h0_(p) = s_case, cmd1e_(p) = e, cmd2c_(p) = c1,
                     cmd3c_(p) = c2, cmd4c_(p) = 0; /* cmd4c_ = LabelNumber */
    return p;
}


static bool is_fpval(Expr *e, FPConst *fc)
{
    while (h0_(e) == s_invisible) e = arg2_(e);
    if (h0_(e) == s_floatcon)
    {   FloatCon *f = (FloatCon *)e;
        if (f->floatlen == (bitoftype_(s_short) | bitoftype_(s_double)))
            return (f->floatbin.fb.val == fc->s->floatbin.fb.val);
        else if (f->floatlen == bitoftype_(s_double))
            return (f->floatbin.db.msd == fc->d->floatbin.db.msd &&
                    f->floatbin.db.lsd == fc->d->floatbin.db.lsd);
    }
    return NO;
}

bool is_fpzero(Expr *e)
{
    return is_fpval(e, &fc_zero);
}

bool is_fpone(Expr *e)
{
    return is_fpval(e, &fc_one);
}

bool is_fpminusone(Expr *e)
{
    return is_fpval(e, &fc_minusone);
}

#ifdef ENABLE_AETREE
#define ANY_ENABLED ENABLE_AETREE
#else
#ifdef ENABLE_BIND
#define ANY_ENABLED ENABLE_BIND
#else
#ifdef ENABLE_CG
#define ANY_ENABLED ENABLE_CG
#else
#ifdef ENABLE_REGS
#define ANY_ENABLED ENABLE_REGS
#else
#ifdef ENABLE_TYPE
#define ANY_ENABLED ENABLE_TYPE
#endif
#endif
#endif
#endif
#endif

#define PR_CMD          1
#define PR_BIND         3
#define PR_FORMTYPE     4

#define symbol_name_(s) ((sym_name_table[s]))

static int32 position = 0;

void eprintf(char *s, ...)

{   va_list ap;
    va_start(ap,s);
    if (s[0] == '\n' && s[1] == 0) {
        fputc('\n', stderr); position = 0;
    } else
        position += _vfprintf(stderr,s,ap);
    va_end(ap);
}

void pr_stringsegs(StringSegList *z)
/* only used here and in jopprint.c                                     */
{   putc('"', stderr);
    for (; z!=NULL; z = z->strsegcdr)
    {   char *s = z->strsegbase;
        int32 len = z->strseglen, i;
        for (i=0; i<len; i++)
        {   int ch = ((unsigned char *)s)[i];   /* for isprint */
            if (isprint(ch)) putc(ch, stderr);
            else if (ch=='\n') eprintf("\\n");
            else eprintf("\\%lo", (long)ch);
        }
    }
    putc('"', stderr);
}

#ifdef ANY_ENABLED
static void elinebreak(void)
{
    if (position>64)
    {   putc('\n', stderr);
        position = 0;
    }
}

static void pr_id(Symstr *sv)
{
    if (sv == 0) eprintf("<NULL-ID>");
    else if (h0_(sv) != s_identifier) eprintf("<odd id %p/%lx>", sv, (long)h0_(sv));
    else eprintf("[id:%s]",symname_(sv));
    elinebreak();
}

static void pr_memb(ClassMember *m)
{
    Symstr *sv = memsv_(m);
    eprintf("[mem:%s]", sv ? symname_(sv) : "*NULL*");
}

static void pr_label(LabBind *x)
{   pr_id(x->labsym);
}

static void pr_bind0(Binder *b)
{
    Symstr *sv = bindsym_(b);
    eprintf("%s",symname_(sv));
}

static void pr_bind1(Binder *b)
{   Symstr *sv;
    eprintf("[Bind%p:", (VoidStar )b);
    eprintf("s=%lx ", (long)bindstg_(b));
    eprintf("t=");
    pr_typeexpr(bindtype_(b), 0);
    elinebreak();
    eprintf("addr=%ld", (long)bindaddr_(b));
    sv = bindsym_(b);
    eprintf(" name=%s]",symname_(sv));
    elinebreak();
}

static void pr_tagbind(TagBinder *b)
{   Symstr *sv;
    eprintf("[TagBind%p:", (VoidStar )b);
    elinebreak();
    eprintf("sort=%s ", symbol_name_(tagbindsort(b)));
    /* do not print member list in case circular type! */
    sv = bindsym_(b);
    eprintf(" name=%s]",symname_(sv));
    elinebreak();
}

static void pr_optexpr(Expr *x, char *s)
{
        if (x!=0) pr_expr(x);
        eprintf("%s", s);
        elinebreak();
}

static void pr_condition(Expr *x)
{
    eprintf("(");
    pr_expr(x);
    eprintf(")");
    elinebreak();
}

static void pr_list(int32 frep, VoidStar x)
{
        while (x != 0)
        {       switch (frep)
                {
        default:        eprintf("?");
                        break;
        case PR_CMD:    pr_cmd(cmdcar_((CmdList *)x));
                        break;
        case PR_FORMTYPE:
                        pr_id(((FormTypeList *)x)->ftname);
                        break;
        case PR_BIND:   pr_bind1(((BindList *)x)->bindlistcar);
                        break;
                }
                eprintf(" ");
                elinebreak();
                x = (VoidStar) cdr_((List *)x);
        }
}

static void pr_typespec(TypeExpr *x)
{
    SET_BITMAP m = typespecmap_(x);
    if(h0_(x)!=s_typespec)
        {       eprintf("<bad typespec %ld>", (long)h0_(x));
                return;
        }
    eprintf(" %lx ", (long)m);
    elinebreak();
    if (m & (CLASSBITS | bitoftype_(s_enum)))
        pr_tagbind(typespectagbind_(x));
    else if (m & bitoftype_(s_typedefname))
        pr_bind1(typespecbind_(x));
    else if (typespecbind_(x) != 0)
        eprintf("Odd type binder %p", (VoidStar )typespecbind_(x));
}

/* this SHOULD print out the typeexpr backwards as a declarator with
   string s (possibly 0) as the declaree innermost.  Some fine day... */
void pr_typeexpr(TypeExpr *x, Symstr *s)
/* s is currently unused */
{
        switch (h0_(x))
        {
case s_typespec:eprintf("<TYPE:%p ", (VoidStar )x);
                pr_typespec(x);
                eprintf("> ");
                break;
case t_coloncolon: eprintf("<"); pr_tagbind(typespectagbind_(x));
                eprintf("::"); pr_typeexpr(typearg_(x),s);
                eprintf("> ");
                break;
case t_content: eprintf("*%lx ", (long)typeptrmap_(x));
                pr_typeexpr(typearg_(x),s);
                break;
case t_ref:     eprintf("&*%lx ", (long)typeptrmap_(x));
                pr_typeexpr(typearg_(x),s);
                break;
case t_subscript:
                pr_typeexpr(typearg_(x),s);
                eprintf("[");
                pr_optexpr(typesubsize_(x)," ");
                eprintf("]");
                break;
case t_fnap:    pr_typeexpr(typearg_(x),s);
                eprintf("(");
                pr_list(PR_FORMTYPE, typefnargs_(x));
                eprintf(")");
                break;
case t_ovld:    eprintf("<ovld:");
                pr_list(PR_BIND, typeovldlist_(x));
                eprintf(">");
                break;
default:        eprintf("[unrecognized typeexpr %p:%ld]",
                        (void *)x, (long)h0_(x));
                break;
        }
        elinebreak();
}

void pr_expr(Expr *x)
{
    AEop op;
    if (x == 0) eprintf("<missing expr>");
    else switch (op = h0_(x))
    {
case s_error:   eprintf("previous_error");
                return;
case s_identifier:
                pr_id((Symstr *)x);
                return;
case s_member:
                pr_memb((ClassMember *)x);
                return;
case s_binder:
#ifdef DETAILED
                pr_bind1((Binder *)x);
#else
                pr_bind0((Binder *)x);
#endif
                return;
case s_integer: eprintf("%ld", (long)intval_(x));
                return;
case s_floatcon:eprintf("<float %s>", ((FloatCon *)x)->floatstr);
                return;
case s_string:  elinebreak();
                eprintf(" ");
                pr_stringsegs(((String *)x) -> strseg);
                eprintf(" ");
                elinebreak();
                return;
case s_fnapstructvoid:
case s_fnapstruct:
case s_fnap:    pr_expr(arg1_(x));
                eprintf("(");
                {   ExprList *y = exprfnargs_(x);
                    while (y != NULL)
                    {   pr_expr(exprcar_(y));
                        y = cdr_(y);
                        if (y != NULL) { eprintf(", "); elinebreak(); }
                    }
                }
                eprintf(")");
                return;
case s_cond:    eprintf("(");
                pr_expr(arg1_(x));
                eprintf(" ? ");
                elinebreak();
                pr_expr(arg2_(x));
                eprintf(" : ");
                elinebreak();
                pr_expr(arg3_(x));
                eprintf(")");
                elinebreak();
                return;
#ifdef RANGECHECK_SUPPORTED
case s_checknot: eprintf("(");
                pr_expr(arg1_(x));
                eprintf(" ne ");
                elinebreak();
                pr_expr(arg2_(x));
                eprintf(")");
                elinebreak();
                return;
case s_rangecheck: eprintf("(");
                pr_expr(arg1_(x));
                eprintf(" in ");
                elinebreak();
                eprintf("[");
                pr_expr(arg2_(x));
                eprintf(" : ");
                elinebreak();
                pr_expr(arg3_(x));
                eprintf("])");
                elinebreak();
                return;
#endif
case s_dot:     eprintf("(");
                pr_expr(arg1_(x));
                eprintf(" . ");
                if (h0_(type_(x)) == t_ovld)
                    pr_expr(arg2_(x));
                else
                    eprintf("%ld", (long)exprdotoff_(x));
                eprintf(")");
                return;
case s_cast:    eprintf("(CAST(");
                pr_typeexpr(type_(x),0);
                eprintf(") ");
                elinebreak();
                pr_expr(arg1_(x));
                eprintf(")");
                return;
case s_invisible: eprintf("INVISIBLE(");
                pr_expr(arg1_(x));
                eprintf(" ==> ");
                elinebreak();
                pr_expr(arg2_(x));
                eprintf(")");
                return;
case s_let:     eprintf("(LET ");
                elinebreak();
                pr_list(PR_BIND, arg1_(x));
                eprintf(" IN ");
                elinebreak();
                pr_expr(arg2_(x));
                eprintf(")");
                return;
#ifdef EXTENSION_VALOF
case s_valof:   eprintf("VALOF ");
                pr_cmd(expr1c_(x));
                return;
#endif
default:
        if (ismonad_(op) || op == s_return)
        {       eprintf("(%s ", symbol_name_(op));
                elinebreak();
                pr_expr(arg1_(x));
                eprintf(")");
        }
        else if (isdiad_(op))
        {   eprintf("(");
            pr_expr(arg1_(x));
            eprintf(" %s ", symbol_name_(op));
            elinebreak();
            pr_expr(arg2_(x));
            eprintf(")");
        }
        else
        {   eprintf("Unprintable op %ld (%s)\n", (long)op, symbol_name_(op));
            position = 0;
        }
        return;
    }
}

void pr_cmd(Cmd *x)
{
        AEop op;
        for (;;)
        {
                if (x!=0) switch (op = h0_(x))
                {
        default:        eprintf("<odd cmd %ld = %s>",(long)op, symbol_name_(op));
                        elinebreak();
                        return;
        case s_break:
        case s_endcase:
        case s_continue:eprintf("%s", symbol_name_(op));
                        break;
        case s_return:  eprintf("return ");
                        pr_optexpr(cmd1e_(x),";");
                        elinebreak();
                        return;
#ifdef EXTENSION_VALOF
        case s_resultis:
                        eprintf("resultis ");
                        pr_optexpr(cmd1e_(x),";");
                        elinebreak();
                        return;
#endif
        case s_case:    eprintf("case");
                        pr_condition(cmd1e_(x));
                        eprintf(":");
                        elinebreak();
                        x = cmd2c_(x);
                        continue;
        case s_default: eprintf("default:");
                        elinebreak();
                        x = cmd1c_(x);
                        continue;
        case s_do:      eprintf("do ");
                        pr_cmd(cmd1c_(x));
                        eprintf("while");
                        elinebreak();
                        pr_condition(cmd2e_(x));
                        break;
        case s_for:     eprintf("for(");
                        pr_optexpr(cmd1e_(x),";");
                        elinebreak();
                        pr_optexpr(cmd2e_(x),";");
                        elinebreak();
                        pr_optexpr(cmd3e_(x),")");
                        elinebreak();
                        x = cmd4c_(x);
                        continue;
        case s_goto:    eprintf("goto");
                        pr_label(cmd1lab_(x));
                        break;
        case s_if:      eprintf("if");
                        pr_condition(cmd1e_(x));
                        elinebreak();
                        pr_cmd(cmd2c_(x));
                        elinebreak();
                        if ((x = cmd3c_(x)) != 0) continue;
                        else return;
        case s_switch:  eprintf("switch ");
                        pr_condition(cmd1e_(x));
                        elinebreak();
                        x = cmd2c_(x);
                        continue;
        case s_colon:   pr_label(cmd1lab_(x));
                        eprintf(":");
                        elinebreak();
                        x = cmd2c_(x);
                        continue;
        case s_semicolon:
                        pr_expr(cmd1e_(x));
                        break;
        case s_block:   eprintf("{");
                        pr_list(PR_BIND, cmdblk_bl_(x));
                        eprintf(" ");
                        elinebreak();
                        pr_list(PR_CMD, cmdblk_cl_(x));
                        eprintf("}");
                        break;
                }
                eprintf(";");
                elinebreak();
                return;
        }
}

void pr_topdecl(TopDecl *x)
{
        position = 0;
        if (x==NULL)
        {   eprintf("<missing topdecl>\n");
            position = 0;
            return;
        }
        switch (h0_(x))
        {
case s_fndef:   eprintf("FNDEF ");
                pr_bind1(x->v_f.fn.name);
                eprintf("\n");
                pr_list(PR_BIND, x->v_f.fn.formals);
                if (x->v_f.fn.ellipsis) eprintf(" (...)");
                eprintf("\n");
                pr_cmd(x->v_f.fn.body);
                eprintf("\n");
case s_decl:    break;
default:        eprintf("<unknown top level %ld>", (long)h0_(x));
                position = 0;
                break;
         }
}

#else

/* non-debugging version: */
void pr_topdecl(TopDecl *x) { IGNORE(x); }
void pr_expr(Expr *x) { IGNORE(x); }
void pr_cmd(Cmd *x) { IGNORE(x); }

#endif

/* End of section aetree.c */
