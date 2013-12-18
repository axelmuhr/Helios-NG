/* $Id: sem.c,v 1.1 1990/09/13 17:10:20 nick Exp $ */

/* c.sem: semantic analysis phase of C compiler */
/* Copyright (C) A.Mycroft and A.C.Norman       */
/* version 0.99a */

#include "cchdr.h"
#include "AEops.h"

Expr *errornode;

/* memo: re-insert check on fn returning fn!!!                         */
/* BIG thing to do soon: get correct op in error message for things
               like 3[4], !(struct ...).                               */
/* AM 6-2-87:  check args to printf/scanf!                             */
/* AM 24-6-86: Treat 'float' as 'short double' internally.             */

/* forward references */
static Expr *mkaddrop();
static Expr *bitfieldvalue();

int lsbitfirst;   /* ordering for bitfields within word                */

#define orig_ arg1_
#define compl_ arg2_

/* the next line is the RUNTIME size of int - i.e. sizeoftype(te_int) */
#define sizeof_int 4
#define padsize(n,align) (-((-(n)) & (-(align))))

#define ARITHTYPEBITS (bitoftype_(s_char)|bitoftype_(s_int)| \
                       bitoftype_(s_enum)|bitoftype_(s_double))

/* type-like things... */

static void typeclash(op)
AEop op;
{    sem_err("Illegal types for operands: $s", op);
}

/* type manipulation ... */

TypeExpr *prunetype(t)
TypeExpr *t;
{   Binder *b;
    while (h0_(t) == s_typespec &&
           (typespecmap_(t) & bitoftype_(s_typedefname)))
    {   if (debugging(DEBUG_TYPE)) eprintf("Pruning typedef...\n");
        b = typespecbind_(t);
        t = bindtype_(b);
    }
    return t;
}

bool isvolatile_type(x)
TypeExpr *x;
{
    x = prunetype(x);
    if (h0_(x) == t_content) return (typeptrmap_(x) & bitoftype_(s_volatile));
    return isprimtype_(x, s_volatile);
}


TypeExpr *typeofexpr(x)
Expr *x;
{   AEop op;
    TypeExpr *t;
    switch (op = h0_(x))
    {   case s_binder:
            t = bindtype_((Binder *)x);
            break;
        case s_floatcon:
/* perhaps slave all three combinations - or put a type_ field in s_floatcon */
            t = ((FloatCon *)x)->floatlen == bitoftype_(s_double) ?
                te_double : primtype_(((FloatCon *)x)->floatlen);
            break;
        case s_string:
          { int n = 1; StringSegList *p;
            for (p = ((String *)x)->strseg; p != 0; p = p->strsegcdr)
                n += p->strseglen;
            t = mk_typeexpr1(t_subscript, primtype_(bitoftype_(s_char)),
                                   mkintconst(te_int,n,0));
          } break;
        case s_error:            /* @@@ remove soon? */
            t = te_int; break;   /* for sizeof() or sizeof(1.0%2.0) */
        case s_integer:
            t = type_(x); break;
        default:
            if (hastypefield_(op)) { t = type_(x); break; }
            syserr("typeof(%d)", op);
            t = te_int;
            break;
    }
    if (debugging(DEBUG_TYPE))
    {   eprintf("Type of "); pr_expr(x);
        eprintf(" is "); pr_typeexpr(t, 0); eprintf("\n");
    }
    return t;
}

/* @@@ contemplate a routine 'princtype: TypeExpr -> AEop' to avoid
   2-level switches and make code more representation independent
*/

int alignoftype(x)
TypeExpr *x;
/* gives mininum alignment (power of 2 ) for object of type.  Current view
   epitomised here is char/char array 1, short/short array 2, else 4.
   If TARGET_ALIGNS_DOUBLES then doubles and their structs are 8.
   Bit fields are treated as requiring an 'int' in which several fields
   may fit.  Structs/unions int aligned, so "sizeof(struct {char x}[6])" = 24.
   Contemplate changing this.
*/
{   SET_BITMAP m;
    Binder *b;
    switch (h0_(x))
    {   case s_typespec:
            m = typespecmap_(x);
            switch (m & -m)    /* LSB - unsigned/long etc. are higher */
            {   case bitoftype_(s_char): return 1;
                case bitoftype_(s_int):  if (m & bitoftype_(s_short)) return 2;
                default:                 return 4;
#ifdef TARGET_ALIGNS_DOUBLES
                case bitoftype_(s_double): return 8;
/* #error incomplete double alignment code. */
/* e.g. structs have with double elements. */
#endif
                case bitoftype_(s_typedefname):
                    b = typespecbind_(x);
                    return alignoftype(bindtype_(b));
            }
        case t_subscript:
            return alignoftype(typearg_(x));
        default:
            return 4;
    }
}

/* NB. this MUST be kept in step with initstaticvar and findfield (q.v.) */
int sizeoftype(x)
TypeExpr *x;
{   SET_BITMAP m;
    int n, bitoff;
    TagMemList *l;
    TagBinder *b;
    switch (h0_(x))
    {   case s_typespec:
            m = typespecmap_(x);
            switch (m & -m)    /* LSB - unsigned/long etc. are higher */
            {   case bitoftype_(s_char):
                    return 1;
                case bitoftype_(s_int):
                    if (m & BITFIELD)
                      sem_rerr("sizeof <bit field> illegal - sizeof(int) assumed");
                    if (m & bitoftype_(s_short)) return 2;
                    /* drop through */
                case bitoftype_(s_enum):
                    return 4;
                case bitoftype_(s_double):
                    if (m & bitoftype_(s_short)) return 4;
                    return 8;
                case bitoftype_(s_struct):
                    b = typespectagbind_(x);
                    if ((l = tagbindmems_(b)) == 0)
                      sem_err("size of struct $b needed but not yet defined",b);
                    for (bitoff=n=0; l != 0; l = l->memcdr)
                    {   if (l->membits)
                        {   int k = evaluate(l->membits);
                            n = padsize(n, alignoftype(te_int));
                            if (k == 0) k = 32-bitoff;  /* rest of int */
                            if (k+bitoff > 32) n +=sizeof_int, bitoff = 0; /* ovflow */
                            bitoff += k;
                        }
                        else
                        {   if (bitoff != 0) n += sizeof_int, bitoff = 0;
                            n = padsize(n, alignoftype(l->memtype));
                            n += sizeoftype(l->memtype);
                        }
                    }
                    if (bitoff != 0) n += sizeof_int, bitoff = 0;
                    return padsize(n,4);
                case bitoftype_(s_union):
                    b = typespectagbind_(x);
                    if ((l = tagbindmems_(b)) == 0)
                      sem_err("size of union $b needed but not yet defined",b);
                    for (n=0; l != 0; l = l->memcdr)
                        n = max(n, l->membits ? sizeof_int : sizeoftype(l->memtype));
                    return padsize(n,sizeof_int);
                case bitoftype_(s_typedefname):
                    return sizeoftype(bindtype_(typespecbind_(x)));
                case bitoftype_(s_void):
                    sem_rerr("size of 'void' required - treated as 1");
                    return 1;
                default: break;
            }
            /* drop through for now */
        default:
            syserr("sizeoftype(%d,0x%x)", h0_(x), typespecmap_(x));
        case t_subscript:
            n = sizeoftype(typearg_(x));
            if (typesubsize_(x)) n *= evaluate(typesubsize_(x));
            else typesubsize_(x) = globalize_int(1),
                 sem_rerr("size of a [] array required, treated as [1]");
            return n;
        case t_fnap:
            sem_rerr("size of function required - treated as size of pointer");
            /* drop through */
        case t_content:
            return 4;
    }
}

/* the C rules on type equivalence mean that equivtype does not have
   to worry about circularity */
/* equivtype notionally returns a boolean, but the true case is separated */
/* into result 2, identical (LISP equal) and result 1, merely equivalent. */
int equivtype(t1,t2)
TypeExpr *t1;
TypeExpr *t2;
{   int sofar = 2;
    for (;; (t1 = typearg_(t1), t2 = typearg_(t2)))
    {   t1 = prunetype(t1);
        t2 = prunetype(t2);
        if (h0_(t1) != h0_(t2)) return 0;
        switch (h0_(t1))
        {
    case s_typespec:
                /* this next line does a lot:
                   1) it checks same basic type (struct/unsigned int/etc)
                      via set equality on type specifiers.
                   2) it tests pointer equality on tag binders.
                   3) it relies on typespecbind_() being 0 for simple types */
                return (typespecmap_(t1) == typespecmap_(t2) &&
                        typespecbind_(t1) == typespecbind_(t2)) ? sofar:0;
    case t_content:                  /* does (int *const) == (int *) ?      */
                if (typeptrmap_(t1) != typeptrmap_(t2)) return 0;
                continue;
    case t_subscript:                /* should check sizes (if any) match   */
            {   Expr *e1 = typesubsize_(t1), *e2 = typesubsize_(t2);
                if (e1 != 0 && e2 != 0)
                {   if (evaluate(e1) != evaluate(e2)) return 0;
                }
                else
                {   if (e1 != e2) sofar = 1;
                }
            }
            continue;    /* now check results */
    case t_fnap:
                /* ANSI draft (May 86) is remarkably imprecise re type
                   equivalence.  Here we take a robust view that two
                   function types are only equivalent (and hence can
                   be used as prototype then definition) if they have
                   identical parameter lists, or one is (), as distinct
                   from (void).  Allowing () to be equivalent to non-()
                   makes 'equivtype' not an equivalence relation!!!
                */
            {   /* check arg types match... */
                FormTypeList *d1 = typefnargs_(t1), *d2 = typefnargs_(t2);
                if (maxargs_(t1) != maxargs_(t2) ||
                    minargs_(t1) != minargs_(t2) ||
                    t1->variad != t2->variad) sofar = 1;
                if (d1 == 0 && maxargs_(t1) >= 999);       /* t1 is f() */
                else if (d2 == 0 && maxargs_(t2) >= 999);  /* t2 is f() */
                else for (;;)
                  if (d1 && d2) switch (equivtype(d1->fttype,d2->fttype))
                  { default: return 0;
                    case 1: sofar = 1;
                    case 2: d1 = d1->ftcdr, d2 = d2->ftcdr;
                  }
                  else if (d1==d2) break;
                  else return 0;
            }
            continue;    /* now check results */
    default:
            syserr("equivtype(%d)", h0_(t1));
            return 0;
        }
    }
}

static bool indexable(t)
TypeExpr *t;
{   t = prunetype(t);
    return (h0_(t) == t_content);
}

static int strideofindexee(t)
TypeExpr *t;
{   t = prunetype(t);
    return sizeoftype(typearg_(t));
}

#define strideexprofindexee(t) mkintconst(te_int,strideofindexee(t),0)

static int indexer(t)
TypeExpr *t;
{   /* the type reading routine 'or's the 'int' bit
       in if we read unsigned/signed/short/long and no honest type.
       Moreover, we assume 'unary coercions' to be done - so no char.
    */
    t = prunetype(t);
    return isprimtype_(t,s_int);
}

/* lubtype() and friends return a type if OK, or give an error message
   and return 0.  Worry that they are still rather ragged.
   Note that 'the usual unary coercions' omit 'char' and (possibly) float.
   *** BEWARE *** result type is ASSUMED to be prunetype'd ***
*/
static TypeExpr *lubtype(op,t1,t2)
AEop op;
TypeExpr *t1;
TypeExpr *t2;
/* Take care of 'the usual coercions' for e.g. s_times.
   Relations are done separately, as are the pointer cases of s_plus */
{   TypeExpr *x1 = prunetype(t1), *x2 = prunetype(t2);
    SET_BITMAP m1 = typespecmap_(x1), m2 = typespecmap_(x2);
    if (h0_(x1) == s_typespec && h0_(x2) == s_typespec &&
        (m1 & ARITHTYPEBITS) != 0 && (m2 & ARITHTYPEBITS) != 0)
    {   if ((m1 | m2) & bitoftype_(s_double))
        {   if (!floatableop_(op))  /* % << >> etc. */
            {   typeclash(op); return 0; }
            if ((m1 | m2) & bitoftype_(s_long)) return te_ldble;
            if ((m1 & (bitoftype_(s_double)|bitoftype_(s_short))) ==
                bitoftype_(s_double)) return te_double;
            if ((m2 & (bitoftype_(s_double)|bitoftype_(s_short))) ==
                bitoftype_(s_double)) return te_double;
            /* the next line is an ANSI draft (May 86) feature.
               In the K&R case coerceunary would have ensured that any
               such arguments would have already been coerced to double */
            return te_float;
        }
        /* the next line is an ANSI special case */
        if (op == s_leftshift || op == s_rightshift) return x1;
        if ((m1 | m2) & bitoftype_(s_long))
        {   if ((m1 & (bitoftype_(s_unsigned)|bitoftype_(s_long))) ==
                      (bitoftype_(s_unsigned)|bitoftype_(s_long)) ||
                (m2 & (bitoftype_(s_unsigned)|bitoftype_(s_long))) ==
                      (bitoftype_(s_unsigned)|bitoftype_(s_long)))
                return te_ulint;
            return te_lint;
        }
        if ((m1 | m2) & bitoftype_(s_unsigned)) return te_uint;
        return te_int;  /* only case left */
    }
    /* now follows code to warn and fix up things allowed by tiny-C */
    if (h0_(x1) == s_typespec && (m1 & bitoftype_(s_int))
                              && h0_(x2) == t_content)
    {   sem_rerr("<int> %s <pointer> treated as <int> %s (int)<pointer>",
             symbol_name_(op), symbol_name_(op));
        return x1;  /* may be unsigned */
    }
    if (h0_(x2) == s_typespec && (m2 & bitoftype_(s_int))
                              && h0_(x1) == t_content)
    {   sem_rerr("<pointer> %s <int> treated as (int)<pointer> %s <int>",
             symbol_name_(op), symbol_name_(op));
        return x2;  /* may be unsigned */
    }
    typeclash(op);
    return 0;
}

/* patch up for special treatment of types of formal parameters at defn */
TypeExpr *modify_formaltype(t)
TypeExpr *t;
{   TypeExpr *t1 = prunetype(t);
    switch (h0_(t1))
    {   case t_fnap:       /* turn f() to (*f)() */
            return ptrtotype_(t);
        case t_subscript:  /* turn a[] to *a     */
            return ptrtotype_(typearg_(t1));
        default:
            return t;
    }
}

/* For safety w.r.t. prototype and non-prototype forms, we pass all
   short ints/chars and floats as repectively ints and doubles and
   do callee narrowing.  Wonder what modula-2 thinks of this!!!!
*/
TypeExpr *widen_formaltype(t)
TypeExpr *t;
{   TypeExpr *t1 = prunetype(t);
    if (h0_(t1) == s_typespec)
    {   if (typespecmap_(t1) & bitoftype_(s_double))
            /* turn 'float' = 'short double' to 'double'. */
            return typespecmap_(t1) & bitoftype_(s_short) ? te_double : t;
        if (typespecmap_(t1) & bitoftype_(s_char))
            /* turn 'char' to 'int'. */
            return te_int;
        if (typespecmap_(t1) & bitoftype_(s_int))
            /* turn 'short' to 'int'. */
            return typespecmap_(t1) & bitoftype_(s_short) ? te_int : t;
        return t;
    }
    return t;
}

/* printf/scanf format checker... (belongs in a separate file?) */

enum fmtsort { FMT_INT, FMT_FLT, FMT_PTR };

static void fmt1chk(c,arg,sort)
char c;
Expr *arg;
enum fmtsort sort;
{   TypeExpr *t = prunetype(typeofexpr(arg));
    if (h0_(t) == t_content && sort == FMT_PTR) return;
    if (h0_(t) == s_typespec &&
         ((typespecmap_(t) & bitoftype_(s_int) && sort == FMT_INT) ||
          (typespecmap_(t) & bitoftype_(s_double) && sort == FMT_FLT)))
              return;
    sem_warn("actual type $t mismatches format '%%%c'", t, c);
}

static int fmtchk(fmt,l,class)
String *fmt;
ExprList *l;
int class;
{   /* class=1 => s/f/printf, class = 2 => s/f/scanf - see stdio.h pragma */
    /* returns TRUE if no floating point will be used & printf case */
    StringSegList *z;
    int state = 0, nformals = 0, nactuals = 0,
        no_fp_conversions = (class == 1);
    char buf[20]; int bufp;
    for (z = fmt->strseg; z!=NULL; z = z->strsegcdr)
    {   char *s = z->strsegbase;
        int len = z->strseglen;
        while (len-- > 0)
        {   int ch = *s++;
            if (ch == '%') bufp=1, buf[0]=ch, (state = state==0 ? 1 : 0);
            else if (state != 0) switch ((bufp<20 ? (buf[bufp++] = ch,0):0), ch)
            {   case '*':
                    if (class==2) { state = 9; break; }
                    nformals++;
                    if (l)
                    {   fmt1chk(ch, exprcar_(l), FMT_INT);
                        nactuals++;
                        l = cdr_(l);
                    }
                    break;
                case '.':
                    break;
                case '+': case '-': case ' ': case '#':
                    state = state==1 ? 2 : 99; break;
                case 'h': case 'l': case 'L':
                    /* not treated properly yet */
                    break;
                case 'd': case 'i': case 'o': case 'u': case 'x': case 'X':
                case 'c':
                    if (state == 9) { state = 0; break; } /* scanf suppressed */
                    nformals++;
                    if (l)
                    {   fmt1chk(ch, exprcar_(l), class==2 ? FMT_PTR:FMT_INT);
                        nactuals++;
                        l = cdr_(l);
                    }
                    state = 0;
                    break;
                case 'f': case 'e': case 'E': case 'g': case 'G':
                    no_fp_conversions = 0;
                    if (state == 9) { state = 0; break; } /* scanf suppressed */
                    nformals++;
                    if (l)
                    {   fmt1chk(ch, exprcar_(l), class==2 ? FMT_PTR:FMT_FLT);
                        nactuals++;
                        l = cdr_(l);
                    }
                    state = 0;
                    break;
                case 's': case 'p': case 'n':
                    if (state == 9) { state = 0; break; } /* scanf suppressed */
                    nformals++;
                    if (l)
                    {   fmt1chk(ch, exprcar_(l), FMT_PTR);
                        nactuals++;
                        l = cdr_(l);
                    }
                    state = 0;
                    break;
                case '[':
                    if (state == 9) { state = 0; break; } /* scanf suppressed */
                    if (class==2) /* scanf only */
                    {   /* check well-formed one day */
                        nformals++;
                        if (l)
                        {   fmt1chk(ch, exprcar_(l), FMT_PTR);
                            nactuals++;
                            l = cdr_(l);
                        }
                        state = 0;
                        break;
                    }
                    /* else drop through */
                default:
                    if ('0'<=ch && ch<='9') break;
                    sem_warn("Illegal format conversion '%%%c'", ch);
                    state = 0;
                    break;
            }
        }
    }
    if (state != 0) sem_warn("Incomplete format string");
    nactuals += length(l);
    if (nactuals != nformals)
        sem_warn("Format requires %d parameter%s, but %d given",
                 nformals, nformals==1 ? "":"s", nactuals);
    return no_fp_conversions;
}

/* l-value like stuff ... */

#ifdef TARGET_IS_XPUTER
/* The definition of what constitutes a simple lvalue differ between	*/
/* the ARM and the Transputer						*/
/* The definition of a simple lvalue is something which can be		*/
/* re-calculated cheaply (i.e. using just 1 register) without side fx.	*/
static int issimplelvalue(e)
Expr *e;
{
	for(;;) switch( h0_(e) )
	{
	case s_content:				/* *simple is simple	*/
	case s_dot:
		e = arg1_(e); continue;		/* simple.a.b is simple */
	case s_binder:
		return 1;
	case s_plus:
	case s_minus:				/* simple +- const is simple  */
						/* (should be address expr)   */
		if( h0_(arg2_(e)) == s_integer ) { e = arg1_(e); continue; }
		if( h0_(arg1_(e)) == s_integer ) { e = arg2_(e); continue; }

	default: return 0;		/* everything else is difficult */
	}
}
#else
static int issimplelvalue(e)
Expr *e;
{   while (h0_(e) == s_dot) e = arg1_(e);  /* x.a.b is simple if x is ...   */
    return h0_(e) == s_binder;             /* and lvalues do not have casts */
}
#endif

static Expr *ensurelvalue(e,op)
Expr *e;
AEop op;
{   Expr *x; TypeExpr *t;
    if (h0_(e) == s_error) return errornode;
/* this code catches things like &(a.b) where "struct {int b[4];} a" */
/* but, oh misfortune, that is just what happens inside the macro    */
/* for offsetof(). Ah well.                                          */
    switch (t = prunetype(typeofexpr(e)), h0_(t))
    {   case t_fnap:
        case t_subscript:
            if (op == s_addrof)
            {
#ifdef WARN_ON_ADDRESS_OF_ARRAY
                sem_warn("'&' unnecessary for function or array $e", e);
#endif
/* worse than this, there is an ambiguity in the draft ANSI spec:
 * consider 'char v[10]'.  It is clear that v and &v[0] have type 'char *'
 * but it would seem that '&v' has type 'char *[10]' by careful reading.
 * We do that even though it seems silly.
 */
                return e;  /* this is OK to skip next switch */
            }
            sem_err("Illegal in lvalue: function or array $e", e);
            return errornode;
        case t_content:
            if ((typeptrmap_(t) & bitoftype_(s_const)) && op != s_addrof)
                sem_rerr("assignment to 'const' object $e", e);
            break;
        case s_typespec:
            if ((typespecmap_(t) & bitoftype_(s_const)) && op != s_addrof)
                sem_rerr("assignment to 'const' object $e", e);
            if ((typespecmap_(t) & BITFIELD) && op == s_addrof)
            {   sem_err("bit fields do not have addresses");
                return errornode;
            }
            break;
        default:
            break;
    }
    for (x = e;;) switch (h0_(x))
    {   case s_binder:
        {   Binder *b = (Binder *)x;
            if (isenumconst_(b))
            /* will always be seen via a s_integer nodes */
            {   sem_err("Illegal in l-value: 'enum' constant $b", b);
                return errornode;
            }
            if (op == s_addrof)
            {   if (bindstg_(b) & bitofstg_(s_register))
                {    sem_rerr("'register' attribute for $b ignored when address taken", b);
                     bindstg_(b) &= ~bitofstg_(s_register);
                }
                bindstg_(b) |= b_addrof;
            }
            return e;
        }
        case s_subscript:
        case s_arrow:
        case s_content:
            return e;
        case s_dot:
            x = arg1_(x);
            break;
        case s_invisible:  /* a more general version of s_integer ... */
            x = orig_(x);  /* ... to replace that idea.               */
            break;
        case s_integer:
            if (intorig_(x) != 0) /* i.e. if previously simplified */
            {   x = intorig_(x);
                break;            /* only to give correct error message */
            }
            /* drop through */
        default:
            sem_err("Illegal in l-value: $s", h0_(x));
            return errornode;
    }
}

/* Expression AE tree node creators and associated functions */

Expr *mkintconst(te,n,e)
TypeExpr *te;
int n;
Expr *e;
{    return mk_expr2(s_integer, te, (Expr *)n, e);
}

#ifdef SOFTWARE_FLOATING_POINT
static Expr *mk1fnap(fn,args)
Expr *fn;
ExprList *args;
{
   TypeExpr *t = prunetype(typeofexpr(fn));
   fn = mk_expr2(s_invisible, t, fn,
                 mk_expr1(s_addrof, ptrtotype_(t), fn));
   return mk_expr2(s_fnap, typearg_(t), fn, (Expr *)args);
}
#endif

#ifdef AM_WANTS_TO_THINK_MORE  /* about constant reduction */
static Expr *trydiadreduce1(op,a,b,flag)
AEop op;
Expr *a;
Expr *b;
SET_BITMAP flag;
{
  if (h0_(a) == s_integer)
  { int n = intval_(a);
/* In the following reductions I need to be careful about                  */
/*   (a) side-effects in expressions such as 0*f()                         */
/*   (b) signed vs unsigned quantifies                                     */
/* The only reductions I insist on performing here are ones that will      */
/* load to a constant result even when variable quantities are in the      */
/* input expression, and the main object of this test is so that boolean   */
/* conditions such as (0 & xxx) are reduced to 0 early enough that code    */
/* guarded by tests on them will be recognized as dead. Clearly not all    */
/* possible cases will be spotted, but at least I can cope with some of    */
/* the more important ones.                                                */
    switch (op)
    {
    case s_and:
    case s_andand:
    case s_times:
        if (n == 0) break;
        return NULL;
    case s_or:
        if (n == -1) break;
        return NULL;
    case s_oror:
        if (n == 0) return NULL;
        if (n != 1) a = mkintconst(te_int, 1, 0);
        break;
    default:
        return NULL;
    }
/* Here I have to verify that b does not have any side-effects and that    */
/* the type of a is as I require.                                          */
/* I take a very cautious view about what expressions might have side      */
/* effects - simple variable references will be all that I allow.          */
    if (h0_(b) != s_binder || isvolatile_expr(b)) return NULL;
    {   SET_BITMAP flag1 = typespecmap_(type_(a));
        if ((flag ^ flag1) & (bitoftype_(s_int) | bitoftype_(s_short) |
                bitoftype_(s_long) | bitoftype_(s_unsigned))) return NULL;
    }
    return a;
  }
  return NULL;
}
#endif

static Expr *trydiadreduce(c,flag)
Expr *c;
SET_BITMAP flag;
/* Some code rationalisation so that x>>33 gives correct line error.    */
{ AEop op = h0_(c); Expr *a = arg1_(c), *b = arg2_(c);
  if ((op == s_leftshift || op == s_rightshift) && h0_(b) == s_integer)
  {  int n = intval_(b);
     if ((unsigned)n >= 32)
     {  /* see ANSI spec, valid shifts are 0..31 */
        int nn = mcdep_fixupshift(n);
        if (nn == n)
            sem_warn("Shift by %d illegal in ANSI C", n);
        else
        {   sem_warn("Shift by %d illegal - here treated as %d", n, nn);
            n = nn, arg2_(c) = b = mkintconst(typeofexpr(b), nn, 0);
        }
     }
     if (n == 0 && h0_(a) != s_integer)  /* int case reduced below */
       /* the s_invisible node allows detection of "(x<<0)=1" lvalue error */
       return mk_expr2(s_invisible, type_(c), c, a);
  }
  if (h0_(a) == s_integer && h0_(b) == s_integer &&
                             (flag & bitoftype_(s_unsigned)))
  { unsigned int m = intval_(a), n = intval_(b), r;
    switch (op)
    {   case s_plus:        r = m+n;  break;
        case s_minus:       r = m-n;  break;
        case s_times:       r = m*n;  break;
        case s_div:         if (n==0) return c; r = m/n; break;
        case s_rem:         if (n==0) return c; r = m%n; break;
        case s_and:         r = m&n;  break;
        case s_or:          r = m|n;  break;
        case s_xor:         r = m^n;  break;
        case s_andand:      r = m&&n; break;
        case s_oror:        r = m||n; break;
        case s_leftshift:   r = m<<n; break;
        case s_rightshift:  r = m>>n; break;
        case s_equalequal:  r = m==n; break;
        case s_notequal:    r = m!=n; break;
        case s_less:        r = m<n;  break;
        case s_lessequal:   r = m<=n; break;
        case s_greater:     r = m>n;  break;
        case s_greaterequal:r = m>=n; break;
        default: syserr("Unknown op %d on unsigned integer constants", op);
                 return c;
    }
    return mkintconst(type_(c),r,c);  /* always has a type_ field */
  }
  if (h0_(a) == s_integer && h0_(b) == s_integer)
  { int m = intval_(a), n = intval_(b), r;
    switch (op)
    {   case s_plus:        r = m+n;  break;
        case s_minus:       r = m-n;  break;
        case s_times:       r = m*n;  break;
        case s_div:         if (n==0) return c; r = m/n; break;
        case s_rem:         if (n==0) return c; r = m%n; break;
        case s_and:         r = m&n;  break;
        case s_or:          r = m|n;  break;
        case s_xor:         r = m^n;  break;
        case s_andand:      r = m&&n; break;
        case s_oror:        r = m||n; break;
        case s_leftshift:   r = m<<n; break;
        case s_rightshift:  r = m>>n; break;
        case s_equalequal:  r = m==n; break;
        case s_notequal:    r = m!=n; break;
        case s_less:        r = m<n;  break;
        case s_lessequal:   r = m<=n; break;
        case s_greater:     r = m>n;  break;
        case s_greaterequal:r = m>=n; break;
        default: syserr("Unknown op %d on integer constants", op);
                 return c;
    }
    return mkintconst(type_(c),r,c);  /* always has a type_ field */
  }

  while (h0_(a) == s_invisible) a = compl_(a);
  while (h0_(b) == s_invisible) b = compl_(b);

#ifdef AM_WANTS_TO_THINK_MORE  /* about constant reduction */
  {   Expr *d;
      if ((d = trydiadreduce1(op, a, b, flag)) != NULL) return d;
      if ((d = trydiadreduce1(op, b, a, flag)) != NULL) return d;
  }
#endif

  if (h0_(a) == s_floatcon && h0_(b) == s_floatcon)
  { FloatCon *m = (FloatCon *)a, *n = (FloatCon *)b;
    DbleBin d,e,r;
    int ok;
/* The selector m->floatbin.i is used in place of m->floatbin.f because  */
/* fltrep_widen() wants an (int *) argument. This type pun is a mess.    */
    if (m->floatlen & bitoftype_(s_short))
         fltrep_widen(&m->floatbin.fb, &d);
    else flt_move(&d, &m->floatbin.db);
    if (n->floatlen & bitoftype_(s_short))
         fltrep_widen(&n->floatbin.fb, &e);
    else flt_move(&e, &n->floatbin.db);
    switch (op)
    {   case s_plus:  ok = flt_add(&r,&d,&e); break;
        case s_minus: ok = flt_subtract(&r,&d,&e); break;
        case s_times: ok = flt_multiply(&r,&d,&e); break;
        case s_div:   ok = flt_divide(&r,&d,&e); break;
#define SEM_FLTCMP(op) mkintconst(type_(c),flt_compare(&d,&e) op 0,c);
        case s_equalequal:   return SEM_FLTCMP(==);
        case s_notequal:     return SEM_FLTCMP(!=);
        case s_less:         return SEM_FLTCMP(<);
        case s_lessequal:    return SEM_FLTCMP(<=);
        case s_greater:      return SEM_FLTCMP(>);
        case s_greaterequal: return SEM_FLTCMP(>=);
#undef SEM_FLTCMP
        default: syserr("Unknown op %d on FP constants", op);
                 return c;
    }
    if (!ok)
    {   sem_warn("floating point overflow when folding");
        /* improve */
        return c;
    }
    m = real_of_string("<expr>", 0);             /* nasty */
    m->floatlen = flag & 
          (bitoftype_(s_double)|bitoftype_(s_short)|bitoftype_(s_long));
    if (flag & bitoftype_(s_short))
         fltrep_narrow(&r, &m->floatbin.fb);
    else flt_move(&m->floatbin.db, &r);
    return mk_expr2(s_invisible, type_(c), c, (Expr *)m);
  }
#ifdef SOFTWARE_FLOATING_POINT
  if (flag & bitoftype_(s_double))
  {  Expr *fname;
     if (flag & bitoftype_(s_short)) switch(op)
     {
        case s_plus:        fname = sim.fadd;           break;
        case s_minus:       fname = sim.fsubtract;      break;
        case s_times:       fname = sim.fmultiply;      break;
        case s_div:         fname = sim.fdivide;        break;
        case s_equalequal:  fname = sim.fequal;         break;
        case s_notequal:    fname = sim.fneq;           break;
        case s_less:        fname = sim.fless;          break;
        case s_lessequal:   fname = sim.fleq;           break;
        case s_greater:     fname = sim.fgreater;       break;
        case s_greaterequal:fname = sim.fgeq;           break;
        default:            syserr("FP op %d unknown", op);
     }
     else switch(op)
     {
        case s_plus:        fname = sim.dadd;           break;
        case s_minus:       fname = sim.dsubtract;      break;
        case s_times:       fname = sim.dmultiply;      break;
        case s_div:         fname = sim.ddivide;        break;
        case s_equalequal:  fname = sim.dequal;         break;
        case s_notequal:    fname = sim.dneq;           break;
        case s_less:        fname = sim.dless;          break;
        case s_lessequal:   fname = sim.dleq;           break;
        case s_greater:     fname = sim.dgreater;       break;
        case s_greaterequal:fname = sim.dgeq;           break;
        default:            syserr("FP op %d unknown", op);
     }
    return mk1fnap(fname, list2(list2(0, b), a));
  }
#endif
  return c;
}

static Expr *trymonadreduce(op,a,c)
AEop op;
Expr *a;
Expr *c;
{ if (h0_(a) == s_integer)
  { int m = intval_(a);
    switch (op)   /* BEWARE: 2's complement means signed == unsigned */
    {   case s_monplus:  break;
        case s_neg:      m = -m; break;
        case s_bitnot:   m = ~m; break;
        case s_boolnot:  m = !m; break;
        default:         syserr("Unknown unary op %d on integer constant", op);
        case s_content:  return c;
    }
    return mkintconst(type_(c),m,c);  /* always has a type_ field */
  }
  while (h0_(a) == s_invisible) a = compl_(a);
  if (h0_(a) == s_floatcon)
  { FloatCon *m = (FloatCon *)a;
    int flag = m->floatlen;      /* must be same as type info */
    DbleBin d,r;
    int ok;
    if (m->floatlen & bitoftype_(s_short))
         fltrep_widen(&m->floatbin.fb, &d);
    else flt_move(&d, &m->floatbin.db);
    switch (op)
    {   case s_monplus:  ok = flt_move(&r,&d); break;
        case s_neg:      ok = flt_negate(&r,&d); break;
        default: syserr("Unknown unary op %d on FP constant", op);
                 return c;
    }
    if (!ok)
    {   sem_warn("floating point overflow when folding");
        /* improve */
        return c;
    }
    m = real_of_string("<expr>", 0);             /* nasty */
    m->floatlen = flag;
    if (flag & bitoftype_(s_short))
         fltrep_narrow(&r, &m->floatbin.fb);
    else flt_move(&m->floatbin.db, &r);
    return mk_expr2(s_invisible, type_(c), c, (Expr *)m);
  }
#ifdef SOFTWARE_FLOATING_POINT
  { TypeExpr *t = prunetype(typeofexpr(a));
    SET_BITMAP m = h0_(t)==s_typespec ? typespecmap_(t) : 0;
    if (m & bitoftype_(s_double))
    { Expr *fname;
      if (m & bitoftype_(s_short)) switch (op)
      {
        case s_monplus: return a;
        case s_neg:     fname = sim.fnegate;    break;
        default:        syserr("FP operation %d unknown", op);
      }
      else switch (op)
      {
        case s_monplus: return a;
        case s_neg:     fname = sim.dnegate;    break;
        default:        syserr("FP operation %d unknown", op);
      }
      return mk1fnap(fname, list2(0, a));
    }
  }
#endif
  return c;
}

#ifdef SOFTWARE_FLOATING_POINT
Expr *fixflt(e)
Expr *e;
{
    TypeExpr *t1, *t2;
    Expr *a = arg1_(e), *fname;
    SET_BITMAP m1, m2;
    if (h0_(e) != s_cast) return e;
    t1 = prunetype(type_(e));
    t2 = prunetype(typeofexpr(a));
    if (h0_(t1) != s_typespec || h0_(t2) != s_typespec) return e;
    m1 = typespecmap_(t1);
    if (m1 & bitoftype_(s_void)) return e; /* cast to void: leave alone  */
    m2 = typespecmap_(t2);
    if (m1 & bitoftype_(s_double))
    {   /* this converts something into a floating point value           */
        if (m2 & bitoftype_(s_double))
        {   /* maybe changes the width of a floatint point value?        */
            if (m1 & bitoftype_(s_short))
            {   if (m2 & bitoftype_(s_short)) return a;
                else return mk1fnap(fnarrow, list2(0, a));
            }
            else if (m2 & bitoftype_(s_short))
                return mk1fnap(dwiden, list2(0, a));
            else return a;
        }
        if (m1 & bitoftype_(s_short))
            fname = (m2 & bitoftype_(s_unsigned)) ? sim.ffloatu : sim.ffloat;
        else fname = (m2 & bitoftype_(s_unsigned)) sim.dfloatu : sim.dfloat;
        return mk1fnap(fname, list2(0, a));
    }
    if (m2 & bitoftype_(s_double))
    {   /* some sort of fixing here                                      */
        if (m2 & bitoftype_(s_short))
            fname = (m1 & bitoftype_(s_unsigned)) ? sim.ffixu : sim.ffix;
        else fname = (m1 & bitoftype_(s_unsigned)) ? sim.dfixu : sim.dfix;
        return mkcast(s_cast, mk1fnap(fname, list2(0, a)), type_(e));
    }
/* Otherwise the cast does not involve any FP types.                     */
    return e;
}
#endif

static Expr *trycastreduce(a,tc,c)
Expr *a;
TypeExpr *tc;
Expr *c;
/* Args somewhat redundant -- c = (s_cast,tc,a)                       */
{ if (h0_(a) == s_integer)
  { int n = intval_(a);
    SET_BITMAP m;
    TypeExpr *x = prunetype(tc);
    switch (h0_(x))          /* signed/unsigned shouldn't matter here */
    {   case s_typespec:
            m = typespecmap_(x);
            switch (m & -m)    /* LSB - unsigned/long etc. are higher */
            {   case bitoftype_(s_char):
                    n = ((m & bitoftype_(s_signed)) && (n & 0x80))
                        ? (n | ~0xff) : (n & 0xff);
                    break;
                case bitoftype_(s_int):
                    if (m & bitoftype_(s_short))
                        n = (!(m & bitoftype_(s_unsigned)) && (n & 0x8000))
                            ? (n | ~0xffff) : (n & 0xffff);
                    break;
                case bitoftype_(s_enum): break;
                case bitoftype_(s_void):
                    n = 0;
                    break;
                case bitoftype_(s_double):
                {   TypeExpr *ta = prunetype(type_(a));  /* known s_integer */
                    /* remember (double)(-1u) != (double)(-1) ... */
/* Use int_to_real() rather than flt_itod() or flt_utod() since it fills */
/* in a string with the number etc etc etc.                              */
                    if (h0_(ta) == s_typespec && (typespecmap_(ta) & ARITHTYPEBITS))
                        return mk_expr2(s_invisible, tc, c,
                            (Expr *)int_to_real(n, typespecmap_(ta)&bitoftype_(s_unsigned), m));
                }
                /* drop through */
                default: return c;     /* always harmless to return cast */
            }
        case t_content:  break;        /* cast to pointer                */
        default:         return c;     /* should not happen (prev error) */
    }
    return mkintconst(tc,n,c);
  }
  while (h0_(a) == s_invisible) a = compl_(a);
/* was Expr *tryfcastreduce(Expr *c) */
/* c = (s_cast,tc,a) and a = (s_floatcon,...) */
/* we have a cast (possibly implicit) on a floating constant value.
   produce a new FloatCon value with the new type.
   Could be later changed to cast floats to ints too.
*/
  if (h0_(a) == s_floatcon)
  { TypeExpr *tcp = prunetype(tc);
    if (h0_(tcp) == s_typespec)
    {   SET_BITMAP m = typespecmap_(tcp);
        int n, ok;
        DbleBin d;
        if (m & (bitoftype_(s_char) | bitoftype_(s_int) | bitoftype_(s_enum)))
        {   FloatCon *aa = (FloatCon *)a;
            if (aa->floatlen & bitoftype_(s_short))
                fltrep_widen(&aa->floatbin.fb, &d);
            else flt_move(&d, &aa->floatbin.db);
        }
        switch (m & -m)
        {   case bitoftype_(s_char):
                if (m & bitoftype_(s_signed)) ok = flt_dtoi(&n, &d);
                else ok = flt_dtou((unsigned *)&n, &d);
/* I do not complain about (char)258.0, but I will moan at (char)1.e11   */
/* where 1.e11 will not fit in an int. This is at least compatible with  */
/* run-time behaviour on casts.                                          */
/* NB chars are unsigned unless the signed bit is set while ints are     */
/* signed unless the unsigned bit is set!                                */
                n = ((m & bitoftype_(s_signed)) && (n & 0x80))
                    ? (n | ~0xff) : (n & 0xff);
                break;
            case bitoftype_(s_int):
                if (m & bitoftype_(s_unsigned))
                    ok = flt_dtou((unsigned *)&n, &d);
                else ok = flt_dtoi(&n, &d);
                if (m & bitoftype_(s_short))
                    n = (!(m & bitoftype_(s_unsigned)) && (n & 0x8000))
                        ? (n | ~0xffff) : (n & 0xffff);
                break;
            case bitoftype_(s_enum):
                ok = flt_dtoi(&n, &d); /* This case seems pretty dodgy! */
                break;
            case bitoftype_(s_double):
                return mk_expr2(s_invisible, tc, c,
                                (Expr *)real_to_real((FloatCon *)a, m));
            default:
                /* this includes the (void)1.3 case for which warning has */
                /* already been given.  Could do ok=1, n=0 instead.       */
                return c;
        }
        if (!ok)
        {   sem_warn("floating to integral conversion failed");
            n = 0;
        }
        return mkintconst(tc,n,c);
    }
    return c;   /* nothing doing */
  }
  return c;
}


static Expr *nonconst1(e)
Expr *e;
{   Expr *e1;
    for (;;) switch (h0_(e))
    {   default: return e;
        case s_integer: return 0;
        case s_invisible: e = orig_(e); break;
        case s_cast:
        case s_monplus:
        case s_neg:
        case s_bitnot:
        case s_boolnot: e = arg1_(e); break;
        case s_plus:
        case s_minus:
        case s_times:
        case s_div:
        case s_rem:
        case s_and:
        case s_or:
        case s_andand:
        case s_oror:
        case s_leftshift:
        case s_rightshift:
        case s_equalequal:
        case s_notequal:
        case s_less:
        case s_lessequal:
        case s_greater:
        case s_greaterequal:
        case s_comma: if ((e1 = nonconst1(arg1_(e))) != 0) return e1;
                      e = arg2_(e); break;
        case s_cond:  if ((e1 = nonconst1(arg1_(e))) != 0) return e1;
                      if ((e1 = nonconst1(arg2_(e))) != 0) return e1;
                      e = arg3_(e); break;
    }
}

void moan_nonconst(e,s)
Expr *e;
char *s;
{   e = nonconst1(e);
    if (e == 0) sem_err("illegal in %s: <unknown>", s);
    else if (h0_(e) == s_binder)
        sem_err("illegal in %s: non constant $b", s, (Binder *)e);
    else
        sem_err("illegal in %s: $s", s, h0_(e));
}

static Expr *coerce2(e,t)
Expr *e;
TypeExpr *t;
  /* does NOT check consistent - use mkcast for this */
  /* really only for implicit arithmetic casts       */
{   Expr *r;
    TypeExpr *t2 = prunetype(typeofexpr(e));
    if (EQtype_(t,t2))
        /* improve this test later */
        return e;
/* new code */
    r = mk_expr1(s_cast, t, e);
#ifdef SOFTWARE_FLOATING_POINT
    return fixflt(trycastreduce(e,t,r));
#else
    return trycastreduce(e,t,r);
#endif
/* the invisible node in the following should never be seen and
   needs re-thinking re constant reduction.                          */
/*     return mk_expr2(s_invisible, t, e, mk_expr1(s_cast, t, e));         */
}


/* coerceunary ought to be idempotent for its uses.                       */
static Expr *coerceunary_2(e,actual)
Expr *e;
int actual;
                                          /* arg is non-error (@@@ check) */
                                          /* see Harbison&Steele p134 */
{   TypeExpr *t = prunetype(typeofexpr(e));
    SET_BITMAP m;
    if (debugging(DEBUG_TYPE))
        {   eprintf("Coerceunary called: "); pr_expr(e); eprintf("\n"); }
    switch (h0_(t))
    {   case s_typespec:
            m = typespecmap_(t);
            switch (m & -m)    /* LSB - unsigned/long etc. are higher */
            {   case bitoftype_(s_char):
/* ANSI says coerce unsigned or signed char to (signed) int */
                    return coerce2(e, te_int);
                case bitoftype_(s_enum):
                    return coerce2(e, te_int);
                case bitoftype_(s_int):
                    if (m & BITFIELD)  /* rvalue context */
                    {   if (h0_(e) != s_dot)
                        {   syserr("coerceunary(bitfieldvalue)");
                            return e;
                        }
                        return bitfieldvalue(e, m,
                                  mk_exprwdot(s_dot, te_int, arg1_(e), exprdotoff_(e)));
                    }
/* ANSI says coerce unsigned or signed short to (signed) int if shorter */
                    if (m & bitoftype_(s_short))
                        return coerce2(e, te_int);
                    return e;
                case bitoftype_(s_double):
                    if ((KRC || actual) && (m & bitoftype_(s_short)))
                        /* K&R do this widening if !actual too. */
                        return coerce2(e, te_double);
                    return e;
                case bitoftype_(s_struct):
                case bitoftype_(s_union):
                case bitoftype_(s_void):
                    return e;
                default: break;
            }
            /* drop through */
        default:
            syserr("coerceunary(%d,0x%x)", h0_(t), typespecmap_(t));
            return e;
        case t_content:
            return e;
        case t_subscript:  /* @@@ check code here for s_dot etc */
            {   TypeExpr *t1 = ptrtotype_(typearg_(t));
                TypeExpr *t2 = ptrtotype_(t);
                return mk_expr2(s_invisible, t1, e,
                           mk_expr1(s_cast, t1, mk_expr1(s_addrof,t2,e)));
            }
        case t_fnap:
            {   TypeExpr *t2 = ptrtotype_(t);
                return mk_expr2(s_invisible, t2, e, mk_expr1(s_addrof,t2,e));
            }
#ifdef never
/* ANSI draft may 86 states that formal arrays "(type a[...])" are
   indistinguishable from "(type *a)", e.g. type checking does not involve
   first dimension and sizeof yields sizeof(pointer).
   Accordingly modify_formals() above has been called so the following code
   cannot be activated.  Save though for the possibility of change in
   later versions. */
t_subscr:   if (h0_(e)==s_binder && 
                (bindstg_((Binder *)e)&bitofstg_(s_argclass))) ec = e;
t_fnap      if ();
#endif
    }
}

static Expr *coerceunary(e)
Expr *e;
{
    return coerceunary_2(e, 0);
}

/* worry about type_ field here? */
#define isliteralzero_(e) \
    (h0_(e) == s_integer && intval_(e) == 0 && intorig_(e) == 0)

static Expr *pointerofint(e,op,t)
Expr *e;
AEop op;
TypeExpr *t;
{   if (!isliteralzero_(e) || (op != s_equalequal && op != s_notequal))
        sem_rerr("Curious comparison $s of pointer and int:\n%s", op,
             "  literal 0 (for == and !=) is only legal case");
    return coerce2(e,t);
}

/* mktest() is used for exprs in conditional (0/non-0) context.
   It does two things.
   1) warns (optionally later) of '=' if '==' could be more sensible.
   2) adds a !=0 node to any non-boolean values, so result is always 0/1
      which also takes care of type-checking modulo error messages.
*/
Expr *mktest(opreason,a)
AEop opreason;
Expr *a;
{   Expr *x;
    AEop op;
    for (x=a;;) switch (op = h0_(x))
    {   case s_invisible: 
           x = orig_(x); break;
        case s_comma:
           x = arg2_(x); break;
        case s_oror:
        case s_andand:
        case s_boolnot:
            return a;   /* type must be OK.  Args already done.  */
        case s_assign:
        case s_bitnot:
            if (suppress & D_ASSIGNTEST) xwarncount++;
            else sem_warn("use of $s in condition context", op);
            /* drop through */
        default:
            opreason = opreason;        /* to reference it .... */
            if (isrelational_(op)) return a;  /* type OK */
            return mkbinary(s_notequal, a, mkintconst(te_int,0,0));
    }
}

static Expr *mkvoided(e)
Expr *e;
/* currently always returns its arg */
{   Expr *x; AEop op;
    for (x = e;;) switch (op = h0_(x))
    {   case s_invisible:
            x = orig_(x); break;
        case s_cast:  /* @@@ do more to moan at (void)(void)f(); */
            x = arg1_(x); break;
        case s_fnap:  /* we could moan for non-void fns? */
            return e;  
        case s_let:
        case s_comma:
            x = arg2_(x); break;
        case s_cond:
            (void)mkvoided(arg2_(x)); x = arg3_(x); break;
        case s_integer:
            if (intorig_(x)) { x = intorig_(x); break; } /* (void)(1+2); */
/* treat 0 specially because (e ? f() : 0) is common (indeed necessary)  */
/* in macro:s (e.g. assert) which must be an expression, not command.    */
            if (intval_(x) == 0) return e;
            /* else drop through */
        default:
            if (!(isassignop_(op) || isincdec_(op)))
                sem_warn("no side effect in void context: $s", op);
            /* drop through */
        case s_assign:
            return e;
#ifndef NO_VALOF_BLOCKS
        case s_valof:
            return e;
#endif
    }
}

static Expr *mklet(v,t,e)
Binder *v;
TypeExpr *t;
Expr *e;
{   if (h0_(e) == s_error) return errornode;
    return mk_expr2(s_let, t, mkBindList(0, v), e);
}

#ifdef OFFSETOF_FIX
static Expr *mkinvisible(t,old,new)
TypeExpr *t;
Expr *old;
Expr *new;
/* The purpose of s_invisible nodes in the tree is to compile one thing
   whilst retaining the original expression for error messages.
   Thus ++x can compile as x=x+1 but produce an error about ++ in &++x.
*/
{/*   if (h0_(new) == s_error) return errornode;		*/
	if( h0_(new) == s_integer ) return mkintconst(t,intval_(new), old);
	return mk_expr2(s_invisible, t, old, new);
}
#endif

Expr *mkunary(op,a)
AEop op;
Expr *a;
{   Expr *c;
    TypeExpr *t;
    if (monadneedslvalue_(op))    /* & ++ -- */
    {   c = mkaddrop(op,ensurelvalue(a,op));
        /* add an invisible node if expression was transformed (for errors) */
        return (h0_(c) == s_error) ? errornode :
               (h0_(c) == op) ? c :
#ifdef OFFSETOF_FIX
               (t = typeofexpr(c), mkinvisible(t, mk_expr1(op,t,a), c));
#else
               (t = typeofexpr(c), mk_expr2(s_invisible, t, mk_expr1(op,t,a), c));
#endif
    }
    if (h0_(a) == s_error) return errornode;
    a = coerceunary(a);
    t = typeofexpr(a);
    switch (op)        /*  + - ~ ! *  */
    {   case s_content:
            t = prunetype(t);
            if (indexable(t)) { t = typearg_(t); break; }
            typeclash(op);
            return errornode;
        case s_boolnot:
            a = mktest(op,a);
            if (h0_(a) == s_error) return errornode;
            t = typeofexpr(a);
            break;
        case s_monplus:
            t = prunetype(t);
            if (h0_(t) == t_content) break;    /* OK */
            /* drop through */
        default:                               /* ~ - (and + from above) */
            t = prunetype(t);
            if (h0_(t) == s_typespec && (typespecmap_(t) & ARITHTYPEBITS))
                break;
            typeclash(op);
            return errornode;
    }
    c = mk_expr1(op,t,a);
/* AM: I wonder if we should do the transformations
       +e => 0+e; -e => 0-e; !e => 0==e; ~e = (-1)^e;
   here?  This would only leave & and * as monads.
   It would certainly simplify type checking for (e.g. +x
   as it would say that int/float/pointer were ok only).
   Then cg.c could get to undo it (and catch similar cases too!).
*/
    return trymonadreduce(op,a,c);
}

static int isbitfieldtype(t)
TypeExpr *t;
{   t = prunetype(t);
    return (h0_(t) == s_typespec && (typespecmap_(t) & BITFIELD));
}

static Expr *mkshift(op,a,n)
AEop op;
Expr *a;
int n;
{
    if (n == 0) return a;
    else return mkbinary(op, a, mkintconst(te_int, n, 0));
}

static Expr *bitfieldvalue(ebf,m,efrom)
Expr *ebf;
SET_BITMAP m;
Expr *efrom;
/* memo: exprbsize_ is size, exprmsboff_ is offset from msb (efrom must be s_dot of bits) */
/* treat 'int' bit fields as 'unsigned' by analogy with 'char' */
{   return (m & bitoftype_(s_signed)) ?
        mkshift(s_rightshift,
                mkshift(s_leftshift, efrom, exprmsboff_(ebf)),
                32-exprbsize_(ebf)) :
        mkbinary(s_and,
                 mkshift(s_rightshift, efrom,
                         32-exprbsize_(ebf)-exprmsboff_(ebf)),
                 mkintconst(te_int, (1<<exprbsize_(ebf))-1, 0));
}

static Expr *bitfieldstuff(op,a,m,bitword,b)
AEop op; Expr *a; SET_BITMAP m;
Expr *bitword; Expr *b;
/* special case: op==s_postinc has b = 1 or -1 according to ++ or -- */
{   /* note that 'bitword' must not have side effects as it is duplicated */
    if (op != s_assign)
        b = mkbinary(op == s_postinc ? s_plus : unassignop_(op),
                     bitfieldvalue(a,m,bitword),
                     b);
    b = mkcast(op, b, te_int);
    return
        mkbinary(op == s_postinc ? s_displace : s_assign,
            bitword,
            mkbinary(s_or,
                mkbinary(s_and, bitword,
                    mkintconst(te_int,
                        ~((1<<exprbsize_(a))-1 << 32-exprbsize_(a)-exprmsboff_(a)), 0)),
                mkshift(s_leftshift,
                    mkbinary(s_and, b, mkintconst(te_int, (1<<exprbsize_(a))-1, 0)),
                    32-exprbsize_(a)-exprmsboff_(a))));
}

static Expr *bitfieldassign(op,a,ta,b)
AEop op;
Expr *a;
TypeExpr *ta;
Expr *b;
/* special case: op==s_postinc has b = 1 or -1 according to ++ or -- */
{   Expr *bitword, *r;
    SET_BITMAP m;
    ta = prunetype(ta);
    if (h0_(a) != s_dot || h0_(ta) != s_typespec)
    {   syserr("bitfieldassign");
        return errornode;  /* note the curious a/ta spread of info */
    }
    m = typespecmap_(ta);
    bitword = mk_exprwdot(s_dot, te_int, arg1_(a), exprdotoff_(a));   /* slight lie */
    if (issimplelvalue(a))
        r = bitfieldvalue(a, m, bitfieldstuff(op, a, m, bitword, b));
    else
    {   Binder *gen = gentempbinder(ptrtotype_(te_int));
        r = bitfieldvalue(a, m,
            mklet(gen, te_int,
                mkbinary(s_comma,
                    mkbinary(s_assign, (Expr *)gen, mkunary(s_addrof,bitword)),
                    bitfieldstuff(op, a, m,
                        mkunary(s_content, (Expr *)gen), b))));
    }
    if (h0_(r) == s_error) return errornode;
    /* for correct error messages and absence of warnings when assignment
       is voided */
    return mk_expr2(s_invisible, te_int, mk_expr2(op, ta, a, b), r);
}


/* mkaddrop and mkassign deal with monadic and diadic operators which require
   an lvalue operand (this is assumed already checked).  Many of these
   may turn into the pseudo-operator s_let to reduce CG complexity.
   Consider nasties like "int *f(); double *g(); *f() /= *g();"
*/

static Expr *mkaddrop(op,a)
AEop op;
Expr *a;
{   if (h0_(a) == s_error) return errornode;
    if (isincdec_(op))  /* ++ -- */
    { /* type check enough here for correct error msgs or extra param */
        Expr *b = mkintconst(te_int, (incdecisinc_(op) ? 1 : -1), 0);
        if (incdecispre_(op))   /* turn ++x to x+=1 */
            return mkassign(s_plusequal, a, b);
        else return mkassign(s_postinc, a, b);
             /* let mkassign() do it - note that it 'knows' about s_postinc */
    }
    /*  case s_addrof:   */
#ifdef OFFSETOF_FIX
	if( h0_(a) == s_dot && h0_(arg1_(a)) == s_content &&
			       h0_(arg1_(arg1_(a))) == s_integer )
		return trydiadreduce(
		 	mk_expr2(s_plus,ptrtotype_(typeofexpr(a)),
			 	arg1_(arg1_(a)),
			 	mkintconst(te_int, exprdotoff_(a), 0)),
		 	0);
#endif
    return mk_expr1(op, ptrtotype_(typeofexpr(a)), a);
}

static Expr *mkopassign(op,asimple,b)
AEop op;
Expr *asimple;
Expr *b;
{   /* asimple must have passed issimplelvalue() or be otherwise reevalable */
    return mkassign(op == s_postinc ? s_displace : s_assign,
       asimple,
       mkbinary(op == s_postinc ? s_plus : unassignop_(op), asimple, b));
}

Expr *mkassign(op,a,b)
AEop op;
Expr *a;
Expr *b;
                    /* for = += *= ... also s_displace and s_postinc */
{   TypeExpr *t1;
    Binder *gen;
    if (h0_(a) == s_error || h0_(b) == s_error) return errornode;
    t1 = typeofexpr(a);  /* un-coerced type */
    if (isbitfieldtype(t1))
    {   Expr *e;
        gen = gentempbinder(te_int);
/* The let statement here is so that any possible side-effects in the    */
/* rhs can not cause confusion with the expansion of the bitfield        */
/* assignment into shifts and masks. Without it x.a = x.b = nn can give  */
/* trouble as the integer holding x can be loaded for the outer          */
/* assignment before it is updated by the inner one.                     */
        e = mkbinary(s_comma,
               mkassign(s_assign, (Expr *)gen, mkcast(op, b, te_int)),
               bitfieldassign(op, a, t1, (Expr *)gen));
        return mklet(gen, typeofexpr(e), e);
    }
    if (op == s_assign || op == s_displace)
        return b = mkcast(op, b, t1),
               h0_(b) == s_error ? errornode : mk_expr2(op, t1, a, b);
    if (issimplelvalue(a)) return mkopassign(op, a, b);
    /* otherwise use a pointer in case side-effects - e.g. *f()+=1        */
    gen = gentempbinder(ptrtotype_(t1));
    return mklet(gen, t1,
        mkbinary(s_comma, mkassign(s_assign, (Expr *)gen, mkaddrop(s_addrof,a)),
                          mkopassign(op, mkunary(s_content, (Expr *)gen), b)));
}

Expr *mkbinary(op,a,b)
AEop op;
Expr *a;
Expr *b;
{   TypeExpr *t1,*t2,*t3;
    if (diadneedslvalue_(op))
    {   Expr *c = mkassign(op,ensurelvalue(a,op),b);
        /* add an invisible node if expression was transformed (for errors) */
        return (h0_(c) == s_error) ? errornode :
               (h0_(c) == op) ? c : (t1 = typeofexpr(c),
                       mk_expr2(s_invisible, t1, mk_expr2(op,t1,a,b), c));
    }
    if (h0_(a) == s_error || h0_(b) == s_error) return errornode;
    a = coerceunary(a);
#ifdef BUGGY_ACN_CODE
    if (op!=s_comma)   /* this fails to deal with (1, a.bitfield) */
#endif
    b = coerceunary(b);
    /* the next line checks and inserts !=0 in && ||.                 */
#define isboolean_(op) ((op)==s_andand || (op)==s_oror)
    if (isboolean_(op))
    {   a = mktest(op,a);
        b = mktest(op,b);
        if (h0_(a) == s_error || h0_(b) == s_error) return errornode;
    }
    t1 = typeofexpr(a);
    t2 = typeofexpr(b);
#define mk_expr2_ptr(op,t,a,b) trydiadreduce(mk_expr2(op,t,a,b),0)
    switch (op)
    /* special case type rules...  generalise to table like lexclass? */
    {   case s_comma:
            a = mkcast(s_comma, a, te_void);
            if (h0_(a) == s_error) return errornode;
            /* note: ANSI forbids reduction of ',' operators */
            return mk_expr2(op, t2, a, b);
        case s_subscript:
            if ((indexable(t1) && indexer(t2)) ||
                (indexable(t2) && indexer(t1)))
                return mkunary(s_content, mkbinary(s_plus, a, b));
            typeclash(op);
            return errornode;
        case s_plus:
            if (indexable(t1) && indexer(t2))
                return mk_expr2_ptr(op, t1, a,
                             mkbinary(s_times, b, strideexprofindexee(t1)));
            if (indexable(t2) && indexer(t1))
                return mk_expr2_ptr(op, t2,
                             mkbinary(s_times, a, strideexprofindexee(t2)),
                             b);
            break;
        case s_minus:
            if (indexable(t1) && indexer(t2))
                return mk_expr2_ptr(op, t1, a,
                             mkbinary(s_times, b, strideexprofindexee(t1)));
            if (indexable(t1) && indexable(t2))
#ifdef OFFSETOF_FIX
	    {
	    	Expr *actual = mk_expr2(s_minus, te_int, a, b);
	    	Expr *r = mkbinary(s_div,
	    			   mk_expr2_ptr(op,te_int,a,b),
	    			   strideexprofindexee(t1));
	    	if( h0_(r) == s_error ) return errornode;
	    	return mkinvisible(te_int,actual,r);
	    }
#else
            {   if (equivtype(t1,t2))
                    return mkbinary(s_div,
                         mk_expr2(op, te_int, a, b),
                         strideexprofindexee(t1));
            }
#endif
            break;
        default:
            if (isrelational_(op))
            {   /* unify code with mkcond()... */
                if (indexable(t1) && indexable(t2))
#ifdef OFFSETOF_FIX
		{
			if( !equivtype(t1,t2) )
			{
				TypeExpr *t1a = prunetype(typearg_(prunetype(t1)));
				TypeExpr *t2a = prunetype(typearg_(prunetype(t2)));
				if( !((op == s_equalequal || op == s_notequal) &&
				      (isprimtype_(t1a, s_void) ||
				      isprimtype_(t2a, s_void))))
				      sem_rerr("differing pointer types: $s", op);
			}
			return mk_expr2(op, te_int, a, b);
		}
#else
                {   if (!equivtype(t1,t2))
                        sem_rerr("differing pointer types: $s", op);
                    return mk_expr2(op, te_int, a, b);
                }
#endif
                if (indexable(t1) && indexer(t2))
                    return mk_expr2_ptr(op, te_int, a, pointerofint(b,op,t1));
                if (indexable(t2) && indexer(t1))
                    return mk_expr2_ptr(op, te_int, pointerofint(a,op,t2), b);
            }
            break;
    }
    /* all that SHOULD be left are the arithmetic types */
    t3 = lubtype(op,t1,t2);           /* BEWARE - see trydiadreduce below */
    if (t3 == 0) return errornode;
    if (op == s_leftshift || op == s_rightshift);   /* done by lubtype() */
    else a = coerce2(a, t3), b = coerce2(b, t3);
    {   Expr *c = mk_expr2(op, (isrelational_(op) ?  te_int : t3), a, b);
        /* the next line relies on the form of result of lubtype() (q.v.) */
        return trydiadreduce(c, typespecmap_(t3));
    }
}

Expr *mkfnap(e,l)
Expr *e;
ExprList *l;
{   TypeExpr *t;
    ExprList *ll;
    FormTypeList *d;
    if (h0_(e) == s_error) return errornode;
    e = coerceunary(e);
    t = prunetype(typeofexpr(e));
    if (!(h0_(t) == t_content &&
           (t = prunetype(typearg_(t)), h0_(t) == t_fnap)))
    {   sem_err("attempt to apply a non-function");
        return errornode;
    }
    for (ll = l, d = typefnargs_(t); ll != 0; ll = cdr_(ll))
    {   Expr *elt = exprcar_(ll);
        /* avoid other error messages due to wrong no of args. */
        if (h0_(elt) == s_error) return errornode;   /* e.g. syntax error */
        /*  float and char args:  consider the following.  It is quite legal
            (according to ANSI May 86 draft) to say "f(x) float x; {...}"
            in one file, and to call it from another with f(1.2).
            In such circumstances it is required to pass 1.2 (even 1.2f)
            as 'double'.  Hence for consistency we pass ALL float args as
            double, even the the prototype form "f(float x){...}" does not
            technically need it.  Discuss with AM if in doubt.
            We call this 'callee narrowing'.  Similarly for 'char' formals:
            we widen to int before the call, do not narrow (= mask) before
            call, but as first instructions of callee.
        */
        if (d)  /* prototype parameter */
        {   Expr *x = mkcast(s_fnap, elt, widen_formaltype(d->fttype));
            if (h0_(x) == s_error) return errornode; /* coercion failed */
            exprcar_(ll) = x;
            d = d->ftcdr;
        }
        else    /* unchecked parameter */
        {   if (equivtype(typeofexpr(elt), te_void))
            {   sem_err("'void' values may not be arguments");
                return errornode;
            }
            exprcar_(ll) = coerceunary_2(elt, 1); /* does float->double */
        }
    }
    if (debugging(DEBUG_TYPE))
        eprintf("fn %s(%d..%d) gets %d args\n",
            h0_(e)==s_binder ? _symname(((Binder *)e)->bindsym) : "<expr>",
            minargs_(t), maxargs_(t), length(l));
    if (!(minargs_(t) <= length(l) && length(l) <= maxargs_(t)))
        sem_err("wrong number of parameters to $e", e);
    if (maxargs_(t) == 1999 && t->variad > 0) 
    {   /* ho-hum, lets see if there's an illegal printf/scanf! */
        Expr *fmt = 0;
        ll = l, d = typefnargs_(t);
        for (; d && ll; d = d->ftcdr, ll = cdr_(ll)) fmt = exprcar_(ll);
        if (d == 0 && fmt != 0)
        {   for (;;) 
            {   switch (h0_(fmt))
                {
        case s_invisible:
                        fmt = compl_(fmt); continue;
        case s_cast:
        case s_addrof:  fmt = arg1_(fmt); continue;
        case s_string:  if (fmtchk((String *)fmt, ll, t->variad))
/* Here we had a printf, sprintf or fprintf where the format string was    */
/* a literal string that showed that no floating point conversions would   */
/* be required. Convert to a call to _printf, _sprintf or _fprintf, which  */
/* are restricted (smaller) versions of the relevant library functions.    */
/* NB that this conversion is enabled by a #pragma present in <stdio.h>    */
/* and so will not confuse users who avoid #pragma and who redefine        */
/* their own printf etc without the benefit of the standard header file.   */
                        {   if (h0_(e) == s_invisible &&
                                h0_(orig_(e)) == s_binder)
                            {   Symstr *fname = bindsym_((Binder *)orig_(e));
                                if (fname == sim.yprintf) 
                                    e = coerceunary(arg1_(sim.xprintf));
                                else if (fname == sim.yfprintf)
                                    e = coerceunary(arg1_(sim.xfprintf));
                                else if (fname == sim.ysprintf)
                                    e = coerceunary(arg1_(sim.xsprintf));
                            }
                            else
                                syserr("sem(odd va_arg fn)");
                        }
                        else
                        {
/* The setting of a default: label here is a bit despicable, but seems to  */
/* achieve what I need is as painless a way as I can see at present. The   */
/* effect here is that if -Qu (store-use debugging) is enabled then any    */
/* printf/fprintf/sprintf that can NOT be seen to involve just integer     */
/* conversions will give rise to a warning. This warning is to be taken to */
/* suggest that extra store will be consumed (in the library) by the code. */
        default:            if (debugging(DEBUG_STORE))
                                sem_warn("printf with possible floating point conversion");
                        }
                        break;
                }
                break;
            }
        }
    }
    return mk_expr2(s_fnap, typearg_(t), e, (Expr *)l);
}

static int implicitpointercast(y,x)
TypeExpr *y;
TypeExpr *x;
{ return
    (isprimtype_(x,s_void) || isprimtype_(y,s_void) ||
      (h0_(x)==s_typespec && h0_(y)==s_typespec &&
        !((typespecmap_(y)&~typespecmap_(x))) &&
        !((typespecmap_(x)&~typespecmap_(y)) &
            ~(bitoftype_(s_const)|bitoftype_(s_volatile))) &&
        typespecbind_(x) == typespecbind_(y)));   /* ensure same struct */
}

/* tidy up relationship with coerce2() */
Expr *mkcast(op,e,tr)
AEop op;
Expr *e;
TypeExpr *tr;
{   TypeExpr *te, *x, *y;
    Expr *r;
    SET_BITMAP m;
    if (h0_(e) == s_error) return errornode;
    e = coerceunary(e);
    te = typeofexpr(e);
    /* check te,tr match enough ... */
    if (equivtype(te,tr))
    {   /* note that if op != s_cast and equivtype(te,tr) we needn't store the
           cast.  But wait a bit (e.g. to improve equivtype())
        */
        if (op != s_cast && EQtype_(te, tr))
            /* improve this test later */
            return e;
    }
    else switch (x = prunetype(tr), y = prunetype(te), h0_(x))
    {   case t_content:
            if (h0_(y) == t_content)
            {   x = prunetype(typearg_(x));
                y = prunetype(typearg_(y));
                if ((h0_(x) == t_fnap) != (h0_(y) == t_fnap))
                { if (warn_implicit_casts)
                  sem_rerr("$s: cast between function and object pointer", op);
                  else xrecovercount++;
                }
                else if (op != s_cast && !implicitpointercast(y,x))
                { if (warn_implicit_casts)
                  sem_rerr("$s: implicit cast of pointer to non-equal pointer",
                           op);
                  else xrecovercount++;
                }
            }
            else if (isprimtype_(y,s_int))
            {   if (op != s_cast && !isliteralzero_(e))
                { if (warn_implicit_casts)
                    sem_rerr("$s: implicit cast of non-0 int to pointer", op);
                  else xrecovercount++;
                }
            }
            else
            {   sem_err("$s: illegal cast of $t to pointer", op, y);
                return errornode;
            }
            break;
        case s_typespec:
            m = typespecmap_(x);
            switch (m & -m)    /* LSB - unsigned/long etc. are higher */
            {   case bitoftype_(s_int):
                    if (h0_(y) == t_content && !(m & bitoftype_(s_short)))
                    {   /* the only valid coercion from pointer */
                        if (op != s_cast)
                        { if (warn_implicit_casts)
                            sem_rerr("$s: implicit cast of pointer to 'int'",op);
                          else xrecovercount++;
                        }
                        break;
                    }
                    /* there may be other similar ops to moan about */
                    if (op == s_switch && isprimtype_(y,s_double))
                    {   sem_rerr("$s: implicit cast of $t to 'int'",op,y);
                        break;
                    }
                    /* drop through */
                case bitoftype_(s_char):
                case bitoftype_(s_enum):
                case bitoftype_(s_double):
                    if (h0_(y) != s_typespec || !(typespecmap_(y) & ARITHTYPEBITS))
                    {   sem_err("$s: illegal cast to $t", op, x);
                        return errornode;
                    }
                    break;
                case bitoftype_(s_struct):
                case bitoftype_(s_union):
                    sem_err("$s: cast to non-equal $t illegal", op, x);
                    return errornode; /* but cg can recover IF sizes equal */
                case bitoftype_(s_void):
                    e = mkvoided(e);
                    break;
                default:
                    syserr("mkcast(%d,0x%x)", h0_(x), typespecmap_(x));
                    return errornode;
            }
            break;
        default:
            syserr("mkcast(%d)", h0_(x));
            return errornode;
        case t_fnap:
        case t_subscript:
            sem_err("$s: cast to $t illegal", op, x);
            return errornode;    /* improve */
    }
    r = mk_expr1(s_cast, tr, e);
#ifdef SOFTWARE_FLOATING_POINT
    return fixflt(trycastreduce(e, tr, r));
#else
    return trycastreduce(e, tr, r);
#endif
}

/* private to findfield and mkfieldselector (note that findfield is NOT
   recursive).  */
static int fieldoffset, fieldbitoff, fieldbsize;

/* NB. this MUST be kept in step with sizeoftype and initstaticvar (q.v.) */
/* Worse still magic numbers are creeping in */
static TypeExpr *findfield(e,t,sv)
Expr *e;
TypeExpr *t;
Symstr *sv;
{   TagBinder *b;
    TagMemList *l;
    int n, bitoff;
    if (h0_(t) != s_typespec ||
        (typespecmap_(t) & (bitoftype_(s_union) | bitoftype_(s_struct))) == 0)
        return 0;  /* caller must provide error message */
    b = typespectagbind_(t);
    if ((l = tagbindmems_(b)) == 0)
    {   sem_err("struct/union $b not yet defined - cannot be selected from",b);
        return list1(s_error);
    }
    for (bitoff = n = 0; l != 0; l = l->memcdr)
    {   TypeExpr *tr = l->memtype;
        if (l->membits)
        {   int k = evaluate(l->membits);
            n = padsize(n, alignoftype(te_int));
            if (k == 0) k = 32 - bitoff;                   /* pad to int */
            if (k+bitoff > 32) n += sizeof_int, bitoff = 0;
            fieldoffset = n, fieldbitoff = bitoff, fieldbsize = k;
            if (tagbindsort_(b) == s_struct) bitoff += k;
        }
        else
        {   if (bitoff != 0) n += sizeof_int, bitoff = 0;
                                            /* since already int aligned */
            n = padsize(n, alignoftype(tr));
            fieldoffset = n, fieldbitoff = 0, fieldbsize = 0;
            if (tagbindsort_(b) == s_struct) n += sizeoftype(tr);
        }
        if (l->memsv == sv) return tr;
                /* note that memsv is 0 for padding bit fields */
    }
    sem_err("struct/union $b has no %s field", b, _symname(sv));
    e = e;                                         /* to reference it .... */
    return list1(s_error);
}

Expr *mkfieldselector(op,e,sv)
AEop op;
Expr *e;
Symstr *sv;
{   TypeExpr *te;
    /* the next line is there, although not mentioned in Harbison&Steele
       so that "struct { int a;}v[5]; v->a" works and becomes v[0].a.
       Also, although we never apply coerceunary to lvalues it is OK
       for '.' (whose result MAY be one) since coerceunary does not
       affect struct/union values.
    */
    if (h0_(e) == s_error) return errornode;
    e = coerceunary(e);
    te = prunetype(typeofexpr(e));
    switch (op)
    {   case s_arrow:
            if (!indexable(te)) break;
            te = prunetype(typearg_(te));
            e = mk_expr1(s_content, te, e);
            /* drop through */
        case s_dot:
            if ((te = findfield(e, te, sv)) == 0) break;
            /* the next clumsy line means an error msg has been given */
            if (h0_(te) == s_error) return errornode;
            if (lsbitfirst) fieldbitoff = 32-fieldbsize-fieldbitoff;
            return mk_exprbdot(s_dot, te, e, fieldoffset,
                               fieldbsize, fieldbitoff);
        default:
            break;
    }
    typeclash(op);
    return errornode;
}

Expr *mkcond(a,b,c)
Expr *a;
Expr *b;
Expr *c;
{   Expr *r;
    TypeExpr *t, *t1, *t2;
    if (h0_(a) == s_error) return a;
    a = coerceunary(a);
    a = mktest(s_cond, a);   /* checks type of a */
    if (h0_(a) == s_error) return a;
    if (h0_(b) == s_error) return b;
    b = coerceunary(b);
    if (h0_(c) == s_error) return c;
    c = coerceunary(c);
    t1 = typeofexpr(b), t2 = typeofexpr(c);
    t1 = prunetype(t1), t2 = prunetype(t2);
    if (equivtype(t1,t2)) t = t1;
    else if (indexable(t1) && indexable(t2))
    {   /* unify code with isrelational_() call above */
        TypeExpr *t1a = prunetype(typearg_(t1));
        TypeExpr *t2a = prunetype(typearg_(t2));
/* *** AM: this is the code I think ANSI meant, ...
        if (isprimtype_(t1a,s_void)) t = t2;
        else if (isprimtype_(t2a,s_void)) t = t1;
   *** but this is what they wrote. ********************************  */
        if (isprimtype_(t1a,s_void)) t = t1;
        else if (isprimtype_(t2a,s_void)) t = t2;
        else sem_rerr("differing pointer types: ':'"),
             t = t1;
    }
    else if (indexable(t1))
        c = mkcast(s_colon, c, t1), t = (h0_(c) == s_error ? 0 : t1);
    else if (indexable(t2))
        b = mkcast(s_colon, b, t2), t = (h0_(b) == s_error ? 0 : t2);
/* improve the next two line as pointers above */
    else if (isprimtype_(t1,s_void)) t = t1;
    else if (isprimtype_(t2,s_void)) t = t2;
    else t = lubtype(s_colon,t1,t2);  /* treat ':' as balancing diad */
    if (t == 0) return errornode;
    b = coerce2(b,t);
    c = coerce2(c,t);
    r = mk_expr3(s_cond,t,a,b,c);
    if (h0_(a) != s_integer) return r;
    /* else simplify.  However, we must not allow (1 ? x : 2) look like
       an lvalue.  Moreover, given we have already reduced b and c
       these may have divided by zero.  Think more.
       Leave an s_invisible node or an s_integer node (the later
       if further reductions may be possible)
    */
    if (intval_(a)) c = b;
    if (h0_(c) == s_integer) return mkintconst(t, intval_(c), r);
    return mk_expr2(s_invisible, t, r, c);
}

/* expression optimiser:
 *** THIS IS BEGINNING TO LOOK LIKE A SEPARATE FILE ***********************
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
static void optimiselist();

/* AM: Following a problem in which double d; (int)(unsigned)d was       */
/* trampled to (int)d I have grave doubts about whether the following    */
/* code (optimise and friends) works in all circumstances.               */

static int ignorable_cast(e)
Expr *e;
{
/* True if the expression e is a cast that does not really change the    */
/* machine-level representation of the thing that it represents.         */
    int m1, m2;
    if (h0_(e) != s_cast) return 0;
    m1 = cautious_mcrepofexpr(e);
    m2 = cautious_mcrepofexpr(arg1_(e));
/* I allow casts to be deemed irrelevant if they are between 4-byte      */
/* integer values even when they change the signed/unsigned status.      */
/* AM: but NOT when the inner one was some form of FIX         ????      */
    if (m1 == m2 ||
        (m1 == 0x4 && m2 == 0x01000004) ||
        (m1 == 0x01000004 && m2 == 0x4)) return 1;
    else return 0;
}

/* Beware highly: optimise currently side-effects the tree.
   Moreover, s_invisible and (more serious) nodes for ++, += etc can
   share sub-structure.  Only binders I believe in the latter case
   so that all should be OK.  Also the use of recursive use of optimise() 
   in case s_addrof requires it to be idempotent.
*/

static Expr *optimise1(e)
Expr *e;
{   AEop op;
    Expr *e1;
    switch (op = h0_(e))
    {   case s_integer:
        case s_floatcon:
        case s_string:
#ifndef NO_VALOF_BLOCKS
        case s_valof:
#endif
        case s_binder:
            return e;
        case s_invisible:
            return optimise1(compl_(e));
        case s_fnap:
            arg1_(e) = optimise1(arg1_(e));
            optimiselist(exprfnargs_(e));
            return e;
        case s_addrof:
            e1 = optimise1(arg1_(e));
            arg1_(e) = e1;
            if (h0_(e1) == s_content) return arg1_(e1);
            if (h0_(e1) == s_string) return e1;  /* cg thinks addr already */
/* Even though s_dot is not optimised out in general it needs to be      */
/* looked after in a special way when I do something like &(a . b)       */
/* Note this can only be legal if a as an lvalue so the result (&a)+nb   */
/* will still be OK - so things like structure-returning functions are   */
/* not involved here.                                                    */
            if (h0_(e1) == s_dot)
                return mk_expr2(s_plus,
                                type_(e),
/* beware - this means optimise() is invoked twice on arg1_(e) */
                                optimise1(mk_expr1(s_addrof, type_(e), arg1_(e1))),
                                mkintconst(te_int,exprdotoff_(e1),0));
            if (h0_(e1) != s_binder)
                syserr("optimise &(%d)", h0_(e1));
            return e;            
        case s_content:  /* get rid of extra &'s and *'s introduced above */
            arg1_(e) = e1 = optimise1(arg1_(e));
            if (h0_(e1) == s_addrof) return arg1_(e1);
            return e;            
        case s_dot:  /* s_arrow already removed */
            arg1_(e) = optimise1(arg1_(e));
            return e;
        case s_cond:
            arg1_(e) = optimise1(arg1_(e));
            arg2_(e) = optimise1(arg2_(e));
            arg3_(e) = optimise1(arg3_(e));
            return e;            
        case s_let:
            arg2_(e) = optimise1(arg2_(e));
            return e;
        case s_cast:
/* I think that I want ineffectual casts (between integer-like types) to */
/* be deleted here.                                                      */
/* AGAIN NOT transforming (double)(int)(unsigned)x to (double)(unsigned)x */
            arg1_(e) = optimise1(arg1_(e));
            return e;
        case s_and:
        case s_times:
        case s_plus:
        case s_or:
        case s_xor:        /* these are both commutative and associative */
            {   Expr *a1 = optimise1(arg1_(e));
                Expr *a2 = optimise1(arg2_(e));
/* This throws away some casts in (a+b), but the type of the whole       */
/* expression is repeated in the top node (which is what other things    */
/* are entitled to look at). The worst loss of information that there    */
/* could be would be that of whether the values being combined are       */
/* signed or unsigned. Thus beware further adjustment to the tree in     */
/* case the top node gets lost or displaced.                             */
                while (ignorable_cast(a1)) a1 = arg1_(a1);
                while (ignorable_cast(a2)) a2 = arg1_(a2);
                if (h0_(a1) == s_integer)
                {   Expr *w = a1; /* commute if that makes a2 an integer */
                    a1 = a2;
                    a2 = w;
                }
                if (h0_(a1) == h0_(e) && h0_(arg2_(a1)) == s_integer &&
                    h0_(a2) == s_integer)
/* Here I transform   ((a + n) + m)  into a + (n+m)                      */
                {   a2 = mkbinary(h0_(e), arg2_(a1), a2);
                    a1 = arg1_(a1);
/* This can convert (e.g.)  (a + 1) + (-1) into  (a + 0) which does not  */
/* get simplified further here. The codegenerator will treat this as     */
/* just a, and the seemingly spurious +0 will serve to preserve the      */
/* proper type of the expression.                                        */
                }
                arg1_(e) = a1;
                arg2_(e) = a2;
            }
            return e;
        default:
            if (ismonad_(op))
                arg1_(e) = optimise1(arg1_(e));
            else if (isdiad_(op))
                arg1_(e) = optimise1(arg1_(e)),
                arg2_(e) = optimise1(arg2_(e));
            else syserr("optimise(%d)\n", op);
            return e;
    }
}

static void optimiselist(x)
ExprList *x;
{   for (; x != 0; x = cdr_(x))
        exprcar_(x) = optimise1(exprcar_(x));
}

Expr *optimise0(e)
Expr *e;
/* exported - yields 0 if a 'serious' error message has already been
   printed (we know this by the s_error at the top of the tree).
   The semantic routines *SHOULD* all be strict in s_error.
*/
{   if (h0_(e) == s_error) return 0;
    return optimise1(e);
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
static bool integerlikestruct(b)
TagBinder *b;
{   TagMemList *l;
    for (l = tagbindmems_(b); l != 0; l = l->memcdr)
    {   if (l->membits == 0)        /* bits are OK (sem.c turns to int) */
        {   TypeExpr *t = prunetype(l->memtype);
            SET_BITMAP m;
            switch (h0_(t))
            {   default: return 0;                  /* array, fn not OK */
                case t_content: break;              /* pointers are OK  */
                case s_typespec:
                    m = typespecmap_(t);
                    switch (m & -m)
                    {   default: return 0;          /* includes char    */
                        case bitoftype_(s_int):
                        case bitoftype_(s_enum):
                            if (m & bitoftype_(s_short)) return 0;
                            break;                  /* 4 bit int ok     */
                        case bitoftype_(s_union):
                        case bitoftype_(s_struct):
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

static int mcrepofexpr1(e,fold)
Expr *e;
int fold;
{   TypeExpr *x = prunetype(typeofexpr(e));
    SET_BITMAP m;
    switch (h0_(x))
    {   case s_typespec:
            m = typespecmap_(x);
            switch (m & -m)    /* LSB - unsigned/long etc. are higher */
            {   case bitoftype_(s_char):
                    if (!(m & bitoftype_(s_signed)))
                        m |= bitoftype_(s_unsigned);  /* @@@ option here! */
                case bitoftype_(s_int):
                case bitoftype_(s_enum):
                    return (m & bitoftype_(s_unsigned) ?
                           0x1000000 + sizeoftype(x) :
                           0x0000000 + sizeoftype(x));
                case bitoftype_(s_double):
                    return 0x2000000 + sizeoftype(x);
                case bitoftype_(s_struct):
                case bitoftype_(s_union):
                    { int n = sizeoftype(x);
                      if (fold && n == 4 &&
                          integerlikestruct(typespectagbind_(x)))
                              return 0x0000000 + n;
                      return 0x3000000 + n;
                    }
                case bitoftype_(s_void):
                    return 0x0000000;
                default: break;
            }
            /* drop through */
        default:
            syserr("mcrepofexpr(%d,0x%x)", h0_(x),typespecmap_(x));
            return 0x0000004;
        case t_subscript:
/* The following code is like it is because ACN apparently manages to call
   mcrepoftype() on an non-binder expression with [] type. *** FIND WHY ***
*/
            if (h0_(e) != s_binder && h0_(e) != s_string)
                syserr("mcrep(array %d)", h0_(e));
            if (h0_(e) != s_binder) return 0x0000004;  /* how it was!!! */
#ifdef never
/* see the comment re ANSI draft in coerceunary() and possibilty of
   ANSI change of mind. */
            if (bindstg_((Binder *)e) & bitofstg_(s_argclass))
            {   syserr("mcrep/argclass array");
                /* Note that although exprs should never have array
                   values, we want to find the size of binders which
                   depends on whether they are args. */
                return 0x0000004; /* argument [] are treated as pointers */
            }
#endif
            return 0x3000000 + sizeoftype(x);
        case t_content:
            return 0x0000004;
    }
}

int mcrepofexpr(e)
Expr *e;
{
/* This normal entry to mcrepofexpr() asserts that certain structures of */
/* length 4 bytes are represented as integers. This makes it possible to */
/* keep them in registers and generally handle them efficiently          */
/* Caching is now not needed -- AM also worried re cautious_mcrepofexpr. */
    int r = mcrepofexpr1(e, 1);
#ifdef SOFTWARE_FLOATING_POINT
/* Fake floating point values so that they are thought of as structures. */
    if (r == 0x02000004) return 0x00000004;
    else if (r == 0x02000008) return 0x03000008;
#endif
    return r;
}

#ifndef ONE_WORD_STRUCTURE_RESULTS_ARE_INTEGERS
int cautious_mcrepofexpr(e)
Expr *e;
{
/* This version of mcrepofexpr() always calls a structure a structure,   */
/* even if it cound usefully be stored in a single machine word. This is */
/* invoked when procedure calls are being processed so that a formal     */
/* rule about how structure results are handled applies even in the      */
/* special case of one-word structures.                                  */
    int r = mcrepofexpr1(e, 0);
#ifdef SOFTWARE_FLOATING_POINT
/* Fake floating point values so that they are thought of as structures. */
    if (r == 0x02000004) return 0x00000004;
    else if (r == 0x02000008) return 0x03000008;
#endif
    return r;
}
#else
int cautious_mcrepofexpr(e)
Expr *e;
{
/* This version of mcrepofexpr() is used when processing function calls  */
/* that return structure values, and allows them to pass back one-word   */
/* structures as integer values rather than by reference.                */
    int r = mcrepofexpr1(e, 0);
#ifdef SOFTWARE_FLOATING_POINT
/* Fake floating point values so that they are thought of as structures. */
    if (r == 0x02000004) return 0x00000004;
    else if (r == 0x02000008) return 0x03000008;
#endif
    return r;
}
#endif

int cautious_mcrepoftype(t)
TypeExpr *t;
{
/* This returns the machine representation of a type. Done by forgery of */
/* an expression and a call to mcrepofexpr(). Treats structures in a     */
/* cautious way i.e. does not fold 1-word structs onto ints.             */
    return cautious_mcrepofexpr(mk_expr2(s_invisible, t, 0, 0));
}

/* a temporary home before its demise... */

int evaluate(a)
Expr *a;
{
/* evaluate the compile-time expression a to yield an integer result     */
    switch (h0_(a))
    {
case s_integer:
        return(intval_(a));
default:
        moan_nonconst(a, "constant expression");
        return 1;
    }
}

/* End of sem.c */

