/* $Id: synprint.c,v 1.4 1994/08/03 09:52:46 al Exp $ */

/* C compiler file synprint.c :  Copyright (C) A.Mycroft and A.C.Norman */
/* version 32 */

#ifdef __STDC__
#include <stdarg.h>
#define start_args( a, b )	va_start( a, b )
#else
#include <varargs.h>
#define start_args( a, b )	va_start( a )
#endif

#include <ctype.h>
#include "cchdr.h"
#include "AEops.h"

#ifdef COMPILING_ON_XPUTER
#undef va_alist
#define va_alist ...
#define va_dcl
#endif

#ifdef COMPILING_ON_DOS
#define va_alist ...
#define va_dcl
#endif

/* forward references... */
static void pr_list();

#define PR_CMD          1
#define PR_BIND         3
#define PR_FORMTYPE     4

static int position;

void eprintf(s, va_alist)
char *s;
va_dcl
{   va_list ap;
    start_args(ap, s);
    position += _vfprintf(stderr,s,ap);
    va_end(ap);
}

#define ANY_ENABLED (defined ENABLE_AETREE || defined ENABLE_BIND || \
  defined ENABLE_CG || defined ENABLE_REGS || defined ENABLE_TYPE)


#ifdef ANY_ENABLED
static void elinebreak()
{
    if (position>64)
    {   putc('\n', stderr);
        position = 0;
    }
}

void pr_id(sv)
Symstr *sv;
{
    if (sv == 0) eprintf("<NULL-ID>");
    else if (h0_(sv) != s_identifier) eprintf("<odd id %p/%x>", sv, h0_(sv));
    else eprintf("[id:%s]",_symname(sv));
    elinebreak();
}
#else
void pr_id(sv)
Symstr *sv;
{
    sv = sv;
}
#endif

#ifdef ANY_ENABLED
static void pr_label(x)
LabBind *x;
{   pr_id(x->labsym);
}

void pr_bind0(b)
Binder *b;
{
    Symstr *sv = bindsym_(b);
    eprintf("%s",_symname(sv));
}
#else
void pr_bind0(b)
Binder *b;
{
    b = b;
}
#endif

#ifdef ANY_ENABLED
static void pr_bind1(b)
Binder *b;
{   Symstr *sv;
    eprintf("[Bind%x:", b);
    eprintf("s=%x ", bindstg_(b));
    eprintf("t=");
    pr_typeexpr(bindtype_(b), 0);
    elinebreak();
    eprintf("addr=%d", bindaddr_(b));
    sv = bindsym_(b);
    eprintf(" name=%s]",_symname(sv));
    elinebreak();
}
#else
static void pr_bind1(b)
Binder *b;
{
    b = b;
}
#endif

#ifdef ANY_ENABLED
static void pr_tagbind(b)
TagBinder *b;
{   Symstr *sv;
    AEop s = tagbindsort_(b);
    eprintf("[TagBind%x:", b);
    elinebreak();
    eprintf("sort=%s ", symbol_name_(s));
    /* do not print member list in case circular type! */
    sv = bindsym_(b);
    eprintf(" name=%s]",_symname(sv));
    elinebreak(); 
}

void pr_bindlist(p)
BindList *p;
{
    int ch = '{';
    while (p!=NULL)
    {   putc(ch, stderr);
        ch = ',';
        pr_bind0(p->bindlistcar);
        p = p->bindlistcdr;
    }
    if (ch=='{') putc('{', stderr);
    putc('}', stderr);
}
#else
void pr_bindlist(p)
BindList *p;
{
    p = p;
}
#endif

#ifdef ANY_ENABLED
static void pr_optexpr(x,s)
Expr *x;
char *s;
{
        if (x!=0) pr_expr(x);
        eprintf(s);
        elinebreak();
}

static void pr_condition(x)
Expr *x;
{
    eprintf("(");
    pr_expr(x);
    eprintf(")");
    elinebreak();
}

static void pr_typespec(x)
TypeExpr *x;
{
    SET_BITMAP m = typespecmap_(x);
    if(h0_(x)!=s_typespec)
        {       eprintf("<bad typespec %d>", h0_(x));
                return;
        }
    eprintf(" %x ", m);
    elinebreak();
    if (m & (bitoftype_(s_struct) | bitoftype_(s_union) | bitoftype_(s_enum)))
        pr_tagbind(typespectagbind_(x));
    else if (m & bitoftype_(s_typedefname))
        pr_bind1(typespecbind_(x));
    else if (typespecbind_(x) != 0)
        eprintf("Odd type binder %x", typespecbind_(x));
}

/* this SHOULD print out the typeexpr backwards as a declarator with
   string s (possibly 0) as the declaree innermost.  Some fine day... */
void pr_typeexpr(x,s)
TypeExpr *x;
Symstr *s;
{
        switch (h0_(x))
        {
case s_typespec: eprintf("[TYPE:%x ", x); pr_typespec(x); eprintf("] "); break;
case t_content: eprintf("*%x ", typeptrmap_(x));
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
default:        eprintf("[unrecognized typeexpr %d]", h0_(x));
                break;
        }
        elinebreak();
}

void pr_stringsegs(z)
StringSegList *z;
{   putc('"', stderr);
    for (; z!=NULL; z = z->strsegcdr)
    {   char *s = z->strsegbase;
        int len = z->strseglen;
        int i;
        for (i=0; i<len; i++)
        {   int ch = ((unsigned char *)s)[i];   /* for isprint */
            if (isprint(ch)) putc(ch, stderr);
            else if (ch=='\n') eprintf("\\n");
            else eprintf("\\%o", ch);
        }
    }
    putc('"', stderr);
 }
#else
void pr_stringsegs(z)
StringSegList *z;
{
    z = z;
}
#endif

#ifdef ANY_ENABLED
static void pr_cmd();

void pr_expr(x)
Expr *x;
{
    AEop op;
    if (x == 0) eprintf("<missing expr>");
    else switch (op = h0_(x))
    {
case s_error:   eprintf("previous_error");
                return;
case s_binder:
                pr_bind0((Binder *)x);
                return;
case s_integer: eprintf("%d", intval_(x));
                return;
case s_floatcon:eprintf("<float %s>", ((FloatCon *)x)->floatstr);
                return;
case s_string:  elinebreak();
                eprintf(" ");
                pr_stringsegs(((String *)x) -> strseg);
                eprintf(" ");
                elinebreak();
                return;
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
case s_dot:     eprintf("(");
                pr_expr(arg1_(x));
                eprintf(" . %d)", arg2_(x));
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
#ifndef NO_VALOF_BLOCKS
case s_valof:   eprintf("VALOF ");
                pr_cmd(expr1c_(x));
                return;
#endif
default:
        if (ismonad_(op))
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
        {   eprintf("Unprintable op %d (%s)\n", op, symbol_name_(op));
            position = 0;
        }
        return;
    }
}
#else
void pr_expr(x)
Expr *x;
{
    x = x;
}
#endif

#ifdef ANY_ENABLED
static void pr_cmd(x)
Cmd *x;
{
        AEop op;
        for (;;)
        {
                if (x!=0) switch (op = h0_(x))
                {
        default:        eprintf("<odd cmd %d = %s in pr_cmd>",op, symbol_name_(op));
                        elinebreak();
                        return;
        case s_break:
        case s_endcase:
        case s_continue:eprintf(symbol_name_(op));
                        break;
        case s_return:  eprintf("return ");
                        pr_optexpr(cmd1e_(x),";");
                        elinebreak();
                        return;
#ifndef NO_VALOF_BLOCKS
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
#else
static void pr_cmd(x)
Cmd *x;
{
    x = x;
}
#endif

#ifdef ENABLE_AETREE
void pr_topdecl(x)
TopDecl *x;
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
                position = 0;
                pr_list(PR_BIND, x->v_f.fn.formals);
                if (x->v_f.fn.ellipsis) eprintf(" (...)");
                eprintf("\n");
                position = 0;
                pr_cmd(x->v_f.fn.body);
                eprintf("\n");
                position = 0;
case s_decl:    break;
default:        eprintf("<unknown top level %d>", h0_(x));
                position = 0;
                break;
         }
}
#else
void pr_topdecl(x)
TopDecl *x;
{
    x = x;
}
#endif

#ifdef ANY_ENABLED
static void pr_list(frep,x)
int frep;
void *x;
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
                x = cdr_((List *)x);
        }
}
#else
static void pr_list(frep,x)
int frep;
void *x;
{
    frep = frep;
    x = x;
}
#endif


/* End of section synprint.c */
