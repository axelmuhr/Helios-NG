/*
 * sem.c: semantic analysis phase of the C/C++ compiler
 * Copyright (C) Codemist Ltd, 1988-1992
 * Copyright (C) Acorn Computers Ltd., 1988-1990.
 * Copyright (C) Advanced RISC Machines Limited, 1991-1992.
 */

/*
 * RCS $Revision: 1.3 $ Codemist 153
 * Checkin $Date: 1993/07/27 09:37:31 $
 * Revising $Author: nickc $
 */

/* @@@ worry about the type of: const struct { int a[10]; } x; &x.a;    */
/*     in pcc mode.                                                     */
/* New routine 'princtype' which acts like old prunetype for users who  */
/* just wish to the 'shape' of a type and not its qualifiers.           */

/* AM Aug 88: Overflow during constant reduction (ANSI Dec 88, section  */
/* 3.4) is now trapped.  It currently gives only a cc_warn message in   */
/* spite of it being a 'constraint violation'.  This is because AM is   */
/* a little fearful of faulting (e.g.) 1<<31, (char)257, -1u, etc.      */
/* It may be that these messages are a little aggressive, let's see.    */
/* Currently, explicit casts give no warning.                           */
/* Find all strings of the form '_overflow' to study.                   */
/* There is also the problem of 'static a = 1 || 1000000*1000000;'.     */
/* AM believe the draft spec to be ambiguous here.                      */
/* Hence the use of cc_ansi_warn not cc_ansi_rerr which sadly deletes   */
/* the object file.                                                     */

/* Nov 88: Rework FEATURE_SIGNED_CHAR so it is ansi-conformant too.     */
/* Nov 88: Rework optional double alignment, see TARGET_ALIGNS_DOUBLES. */
/* Memo: AM to check sizeoftype/findfield/gensubstatic are consistent.  */
/*       doesn't dbx/asd do this too?                                   */

#include <string.h>                            /* for memset and strcmp */
#include "globals.h"
#include "sem.h"
#include "bind.h"
#include "aetree.h"
#include "builtin.h"
#include "aeops.h"
#include "store.h"
#include "errors.h"
#include "util.h"     /* for padsize */

#ifdef PASCAL /*ECN*/
#include "syn.h"      /* No comments please, its already under the axe */
#include "lib.h"
#include "synbind.h"
#include "passem.h"

#else

#define te_boolean te_int

#endif

/* Discussion on "int" and "signed int".  In most cases these are      */
/* identical, hence rather tortuous code in equivtype (find all        */
/* occurrences of s_signed below).  However, they differ in bitfield   */
/* context ("signed" selects signed bitfield, plain implementation     */
/* dependent signed/unsigned) and hence typedef context.  Hence syn.c  */
/* cannot simply normalise so that all int's are signed.               */
/* Note similarly "signed short" == short and "signed long" == long.   */
/* #define'ing SIGNEDNESS_MATTERS can cause this not to happen.        */
/* However, "char", "signed char" and "unsigned char" all differ.      */

#define addrsignmap_ (TARGET_ADDRESSES_UNSIGNED ? \
                        bitoftype_(s_int)|bitoftype_(s_unsigned) : \
                        bitoftype_(s_int)|bitoftype_(s_signed))

/* The following macro is used to default the signedness for 'plain'   */
/* char and 'plain' int bit fields.                                    */
#define issignedchar_(m) ((m) & bitoftype_(s_signed) || \
          (feature & FEATURE_SIGNED_CHAR && !((m) & bitoftype_(s_unsigned))))

Expr *errornode;
#ifdef CPLUSPLUS
SynBindList *sem_reftemps;
#endif

/* memo: re-insert check on fn returning fn!!!                         */
/* AM aug-88: Allow expansion of offsetof to be compile-time constant. */
/* BIG thing to do soon: get correct op in error message for things
               like 3[4], !(struct ...).                               */
/* AM 6-2-87:  check args to printf/scanf!                             */
/* AM 24-6-86: Treat 'float' as 'short double' internally.             */

/* forward references */
static Expr *mkaddr(Expr *a);
static Expr *mkincdec(AEop op, Expr *a);
static Expr *bitfieldvalue(Expr *ebf, SET_BITMAP m, Expr *ewd);
#ifdef CPLUSPLUS
static Symstr *class_has_conversion(TagBinder *b, TypeExpr *t);
static bool islvalue(Expr *x);         /* isaddressablelvalue??        */
#define exprdotmemfn_(p) arg2_(p)
static Binder *ovld_resolve_addr(Expr *e, BindList *bl)
{   Binder *bspecific;
/* Taking the address of an ordinary or (possibly static) member fn.   */
/* For fn 'f', we treat &A::f, &a->f (and, in value ctxt) A::f, a->f   */
/* identically, returning type 't (A::*)(f())'.                        */
/* @@@ Well, at least we intend to!                                    */
    if (h0_(e) == s_dot) e = exprdotmemfn_(e);
    if (bl == NULL || h0_(e) != s_binder) syserr("sem(odd ovld)");
    bspecific = bl->bindlistcar;
    if (bl->bindlistcdr != NULL)
        cc_rerr("non-call site overload (using $b for $e)",
                bspecific, e);
    binduses_(bspecific) |= u_referenced;
    return bspecific;
}
/* the following routine is a hack until we have a better way to get    */
/* from a memfn binder to its class.                                    */
static TagBinder *parentclass(Binder *b)
{   TypeExpr *t = bindtype_(b);
    FormTypeList *l;
    if (h0_(t) != t_fnap) syserr("parentclass(1)");
    l = typefnargs_(t);
    t = l->fttype;
    if (h0_(t) != t_content) syserr("parentclass(2)");
    t = princtype(typearg_(t));
    if (!(h0_(t) == s_typespec && typespecmap_(t) & CLASSBITS))
        syserr("parentclass(3)");
    return typespectagbind_(t);
}
#endif

#define orig_(p)  arg1_(p)
#define compl_(p) arg2_(p)

/* Note, in contexts where default promotions have been done, that      */
/* the s_char/s_enum bits in the following tests are irrelevant.        */
/* Apr 92: see the now finer grain coerceunary() for C++ (and C).       */
#define ARITHTYPEBITS (bitoftype_(s_char)|bitoftype_(s_int)| \
                       bitoftype_(s_enum)|bitoftype_(s_double))
#define INTEGRALTYPEBITS (bitoftype_(s_char)|bitoftype_(s_int)| \
                          bitoftype_(s_enum))

/* type-like things... */

extern void typeclash(AEop op)
{    cc_err(sem_err_typeclash, op);
}

/* Type manipulation -- these are becoming more of an abstract type.    */
/* Exact routine partition in flux.                                     */

#define QUALIFIER_MASK (bitoftype_(s_const)|bitoftype_(s_volatile))

TypeExpr *princtype(TypeExpr *t)
/* Remove 'typedef's from top of type.  Ignore any qualifiers on the    */
/* typedef to the type so qualified.                                    */
{   Binder *b;
    while (isprimtype_(t,s_typedefname))
    {   if (debugging(DEBUG_TYPE)) cc_msg("Pruning typedef...\n");
        b = typespecbind_(t);
        t = bindtype_(b);
    }
    return t;
}

static SET_BITMAP typedef_qualifiers(TypeExpr *x)
{   SET_BITMAP q = 0;
    while (isprimtype_(x, s_typedefname))
    {   Binder *b = typespecbind_(x);
        q |= typespecmap_(x) & QUALIFIER_MASK;
        x = bindtype_(b);
    }
    return q;
}

SET_BITMAP qualifiersoftype(TypeExpr *x)
{   SET_BITMAP q;
    while (h0_(x) == t_subscript) x = typearg_(x);
    q = typedef_qualifiers(x);
    x = princtype(x);
    while (h0_(x) == t_subscript) x = typearg_(x);
/* AM would like to put typespecmap_ and typeptrmap_ in same place.     */
    return q | QUALIFIER_MASK &
        (h0_(x) == s_typespec ? typespecmap_(x) :
         h0_(x) == t_content || h0_(x) == t_ref ? typeptrmap_(x) : 0);
}

extern TypeExpr *mkqualifiedtype(TypeExpr *t, SET_BITMAP qualifiers)
{   TypeExpr *p, *q, *r;
    if (qualifiers == 0) return t;
    p = r = NULL;
    for (;;)
    {   q = mk_typeexpr1(h0_(t), typearg_(t), (Expr *)typespecbind_(t));
        if (r == NULL)
            r = q;
        else
            typearg_(p) = q;
        p = q;
        switch (h0_(q))
        {
default:    /* includes t_fnap, needs faulting to avoid syserr()        */
            syserr(syserr_mkqualifiedtype, h0_(t));
case s_typespec:
            /* NB. this may place the type on another typedef           */
            typespecmap_(q) |= qualifiers; return r;
case t_content:
case t_ref:
            typeptrmap_(q) |= qualifiers;  return r;
case t_subscript:
            /* this next line probably indicates that space for a       */
            /* qualifier map field in an array type would be useful.    */
            t = typearg_(t); break;
        }
    }
}

TypeExpr *prunetype(TypeExpr *t)
/* Remove 'typedef's from top of type.  Add any qualifiers on the       */
/* typedef to the type so qualified -- these are assumed to be          */
/* relatively rare (maybe one day re-implement).                        */
{
    return mkqualifiedtype(princtype(t), typedef_qualifiers(t));
}

bool isvolatile_type(TypeExpr *x)
{   return (qualifiersoftype(x) & bitoftype_(s_volatile)) != 0;
}

bool pointerfree_type(TypeExpr *t) {
    t = princtype(t);
    switch (h0_(t))
    {
case t_content:
case t_ref:       return NO;
case t_subscript: return pointerfree_type(typearg_(t));
case s_typespec:  if (typespecmap_(t) & CLASSBITS)
                  {   ClassMember *l = tagbindmems_(typespectagbind_(t));
                      for (; l != 0; l = memcdr_(l))
                          if (!pointerfree_type(l->memtype)) return NO;
                  }
default:          return YES;
    }
}

bool isbitfield_type(TypeExpr *t)
{   t = princtype(t);
/* Beware: BITFIELD should never qualify a typedef (syn.c ensures)      */
/* (BITFIELD can only occur at the top level in types taken from        */
/* struct/unions).  Else princtype() risks skipping a 'BITFIELD'.       */
    return (h0_(t) == s_typespec && (typespecmap_(t) & BITFIELD));
}

/* The call to unbitfield_type() replaces a use of 'te_int'.            */
TypeExpr *unbitfield_type(TypeExpr *t)
{   /* Return the corresponding simple type, for a pseudo-cast in       */
    /* assignment and initialisation, for e.g. implicit cast err/warn.  */
    if (h0_(t) == s_typespec)
    {   SET_BITMAP m = typespecmap_(t);
        if ((m & (bitoftype_(s_typedef)|BITFIELD)) == BITFIELD)
#ifdef ONE_DAY_SOON /* including C++? */
            return primtype2_(m & ~BITFIELD, typespectagbind_(t));
#endif
            return m & bitoftype_(s_enum) ?
                primtype2_(m & ~BITFIELD, typespectagbind_(t)) : te_int;
    }
    syserr(syserr_unbitfield, t);
    return te_int;
}

/* Merge unbitfield_type() and bf_promotedtype()?                       */
static TypeExpr *bf_promotedtype(TypeExpr *t, int32 size)
{   /* caller ensures that 't' is a s_typespec of BITFIELD.             */
    /* promote: enum bit fields to enum, else int/unsigned int.         */
    /* Maybe long bit fields (in C++) should promote to long.           */
    if (h0_(t) == s_typespec)
    {   SET_BITMAP m = typespecmap_(t);
        if ((m & (bitoftype_(s_typedef)|BITFIELD)) == BITFIELD)
            return (m & bitoftype_(s_enum)) ?
                primtype2_(m & ~BITFIELD, typespectagbind_(t)) :
                   (issignedchar_(m) ||
                    size != MAXBITSIZE && !(feature & FEATURE_PCC)) ?
                te_int : te_uint;
    }
    syserr(syserr_bf_promote, t);
    return te_int;
}

bool isclassorref_type(TypeExpr *t)
{   t = princtype(t);
    if (h0_(t) == t_ref) t = princtype(typearg_(t));
    return isclasstype_(t);
}

static bool isvoidtype(TypeExpr *t)
{   t = princtype(t);
    return (h0_(t) == s_typespec && (typespecmap_(t) & bitoftype_(s_void)));
}

static bool indexable(TypeExpr *t)
{   t = princtype(t);           /* can ignore typedef qualifiers.       */
    return (h0_(t) == t_content
#ifdef CPLUSPLUSxxx
            /* but not x after 'int A::*x' etc.                         */
            && h0_(princtype(typearg_(t))) != t_coloncolon
#endif
           );
}

/* For the two following functions/macros, t must satisfy indexable()   */
#define indexee(t) (typearg_(princtype(t)))
#define strideofindexee(t) (sizeoftype(indexee(t)))

static bool indexer(TypeExpr *t)
{   /* the type reading routine 'or's the 'int' bit
       in if we read unsigned/signed/short/long and no honest type.
       Moreover, we assume 'unary coercions' to be done - so no char/enum.
    */
    t = princtype(t);           /* can ignore typedef qualifiers.       */
    return isprimtype_(t,s_int);
}

static bool sameenum(TypeExpr *a, TypeExpr *b)
{   a = princtype(a), b = princtype(b);
    return isprimtype_(a,s_enum) && isprimtype_(b,s_enum) &&
        typespectagbind_(a) == typespectagbind_(b);
}

/* AE-tree oriented things... */

TypeExpr *typeofexpr(Expr *x)
{   AEop op;
    TypeExpr *t;
    switch (op = h0_(x))
    {   case s_binder:
            t = bindtype_((Binder *)x);
            break;
        case s_member:
            t = memtype2_((ClassMember *)x);
            break;
        case s_floatcon:
/* perhaps slave all three combinations - or put a type_ field in s_floatcon */
            t = ((FloatCon *)x)->floatlen == bitoftype_(s_double) ?
                te_double : primtype_(((FloatCon *)x)->floatlen);
            break;
        case s_string:
          { int32 n = 0; StringSegList *p;
            for (p = ((String *)x)->strseg; p != 0; p = p->strsegcdr)
                n += p->strseglen;
#ifdef PASCAL /*ECN*/
            t = mkArrayType(te_char, mkintconst(te_int, n+1, 0),
                  mkOrdType(bitoftype_(s_int), s_ellipsis, te_int,
                    mkintconst(te_int, 1, 0),
                    mkintconst(te_int, n, 0)), b_packed);
#else
            t = mk_typeexpr1(t_subscript, primtype_(bitoftype_(s_char)),
                             mkintconst(te_int,n+1,0));
#endif
          } break;
#ifdef EXTENSION_UNSIGNED_STRINGS
        case s_ustring:
          { int32 n = 0; StringSegList *p;
            for (p = ((String *)x)->strseg; p != 0; p = p->strsegcdr)
                n += p->strseglen;
            t = mk_typeexpr1(t_subscript, primtype_(bitoftype_(s_char)|bitoftype_(s_unsigned)),
                             mkintconst(te_int,n+1,0));
          } break;
#endif
        case s_wstring:
          { int32 n = 0; StringSegList *p;
            for (p = ((String *)x)->strseg; p != 0; p = p->strsegcdr)
                n += p->strseglen;
            t = mk_typeexpr1(t_subscript, te_wchar,
                             mkintconst(te_int, n/sizeof_wchar + 1, 0));
          } break;
        case s_error:            /* @@@ remove soon? */
            t = te_int; break;   /* for sizeof() or sizeof(1.0%2.0) */
#ifdef PASCAL /*ECN*/
        case s_set:
#endif
        case s_integer:
            t = type_(x); break;
        default:
            if (hastypefield_(op)) { t = type_(x); break; }
            syserr(syserr_typeof, (long)op);
            t = te_int;
            break;
    }
/* The clever macro for debugging() means that the next line generates  */
/* no code if ENABLE_TYPE is not #defined.                              */
    if (debugging(DEBUG_TYPE))
    {   eprintf("Type of "); pr_expr(x);
        eprintf(" is "); pr_typeexpr(t, 0); eprintf("\n");
    }
    return t;
}

/* @@@ contemplate a routine 'TypeExpr -> AEop' to avoid
   2-level switches and make code more representation independent
*/

/* we use 'short long int' internally to represent 'long long int'.     */
#define int_isshort_(m) \
    ((m & (bitoftype_(s_long) | bitoftype_(s_short))) == bitoftype_(s_short))
#define int_decodelength_(m, xs, xi, xl, xll) \
    (m & bitoftype_(s_short)) ? \
        ((m & bitoftype_(s_long)) ? xll : xs) : \
        ((m & bitoftype_(s_long)) ? xl : xi)


/* Beware alignoftype(t_ref) == alignoftype(t_content).                 */
int32 alignoftype(TypeExpr *x)
/* Gives mininum alignment (power of 2 ) for object of type. The current view
   is given in mip/defaults.h. Arrays get aligned as members (ANSI require).
   "alignof_double" may be set to 4 or 8 in target.h.
   If alignof_double > alignof_struct then internal contents of structs
   are examined so that alignof(struct { double d; }) and
     alignof(struct { int a,b;}) can differ.
   Note that the code currently (for efficiency on non-aligning machines)
   assumes that at most alignof_double exceed alignof_struct.  Fix?
   Bit fields are treated as requiring an 'int' in which several fields
   may fit.  Structs/unions are alignof_struct aligned, which should
   currently be 4, so "sizeof(struct {char x}[6])" = 24.
   Contemplate changing this.
*/
{   SET_BITMAP m;
    Binder *b;
    switch (h0_(x))
    {
case s_typespec:
        m = typespecmap_(x);
        switch (m & -m)    /* LSB - unsigned/long etc. are higher */
        {
    case bitoftype_(s_typedefname):
            b = typespecbind_(x);
            return alignoftype(bindtype_(b));
    case bitoftype_(s_char):
            return 1;
    case bitoftype_(s_int):
            return int_decodelength_(m, alignof_short, alignof_int,
                                        alignof_long, alignof_longlong);
    case bitoftype_(s_enum):
            return alignof_int;
    case bitoftype_(s_double):
            return m & bitoftype_(s_short) ? alignof_float :
                   m & bitoftype_(s_long) ? alignof_ldble : alignof_double;
    case bitoftype_(s_struct):
    case bitoftype_(s_class):
    case bitoftype_(s_union):
          { int32 n = alignof_struct;
            /* Short circuit the code if it can never update 'n':         */
            if (alignof_max > alignof_struct)
            {   TagBinder *b = typespectagbind_(x);
                ClassMember *l = tagbindmems_(b);
/* This code should never be called when the struct/union is not defined, */
/* but, even if it is no harm will occur as then l==0.  (!TB_DEFD)        */
/* Note we can safely treat bit field members as non-bit field members.   */
                for (; l != 0; l = memcdr_(l))
#ifdef CPLUSPLUS
                  if (!isfntype(l->memtype))
#endif
                    n = max(n, alignoftype(l->memtype));
            }
            return n;
          }
    case bitoftype_(s_void):            /* pure cowardice */
            cc_rerr(sem_rerr_sizeof_void);
            return alignof_int;
    default:
            break;
        }
default:
        syserr(syserr_alignoftype, (long)h0_(x), (long)typespecmap_(x));
case t_content:
case t_ref:
        return alignof_ptr;
case t_subscript:
        return alignoftype(typearg_(x));
    }
}

/* NB. this (structfield()) MUST be kept in step with initstaticvar (q.v.).
   Everyone else (sizeoftype, findfield, dbg_structrep) needing
   to know about position and size of fields in structures uses
   this function. */

static int32 pcc_bf_sizeof(TypeExpr *x)
{   SET_BITMAP m = typespecmap_(x);
    if (m & bitoftype_(s_char))  return 8;
    if (int_isshort_(m)) return sizeof_short*8;
/* @@@ what about long long? */
    return MAXBITSIZE;
}

bool structfield(ClassMember *l, int32 sort, StructPos *p)
{   /* 2nd arg should probably be a bool (struct/class vs union).       */
    int32 n = p->n, bitoff = p->bitoff;
    p->padded = NO;
#ifdef CPLUSPLUS
    if (!is_datamember_(l)) return NO;
#endif
    if (isbitfield_type(l->memtype))
    {   int32 k = evaluate(l->u.membits);
        if (!(feature & FEATURE_PCC))
        {   /* In ANSI mode, bitfields are stored in int-aligned ints. */
            int32 oldn = n;
            n = padsize(n, alignof_member);     /* min member alignment */
            n = padsize(n, alignof_int);
            if (oldn != n) p->padded = YES;
        }
        else if (bitoff == 0)
        {   /* in -pcc mode, a bitfield may start on any byte boundary */
            bitoff = (n & (alignof_int-1)) * 8;
            n &= ~(alignof_int-1);
/* the next 2 lines are very dubious here.                           */
            n = padsize(n, alignof_int);
            n = padsize(n, alignof_member);     /* min member alignment */
        }
        /* A zero-sized bifield in ANSI mode says "pack no more here". */
        /* We ignore this directive if nothing has yet been packed.    */
        if (k == 0 && bitoff > 0)
            k = MAXBITSIZE - bitoff;           /* pad rest of this int */
        /* in -pcc mode, char and short bitfields are packed into char */
        /* and short containers, respectively.                         */
        if (feature & FEATURE_PCC)
        {   int32 szmask = pcc_bf_sizeof(l->memtype) - 1;
            if (((bitoff & szmask) + k) > (szmask+1))
                bitoff = (bitoff + szmask) & ~szmask;
        }
        if ((k + bitoff) > MAXBITSIZE)         /* overflow this int    */
        {   n += sizeof_int;
            p->bitoff = bitoff = 0;          /* p->bitoff used as flag */
        }
        /* In -pcc mode, reversing the order of packing of bit fields can */
        /* be tricky: if the bit-field packing order is not the same as   */
        /* the byte order then it needs look ahead to the end of the word */
        /* to determine how much padding to insert when extracting fields */
        /* packed in the opposite order. The padding amount is the the    */
        /* number of occupied bytes (addressable units) which follow the  */
        /* current string of bit fields in the current word. In ANSI mode */
        /* this is 0, by definition. The amount of the padding is added   */
        /* to the bitfield's notional offset, boffset.                    */
        if (p->bitoff == 0 && sort != bitoftype_(s_union) &&
            target_lsbitfirst != target_lsbytefirst && feature & FEATURE_PCC)
        {   ClassMember *t;
            int32 sz, szmask, bits;
            bits = bitoff + k;
            for (t = memcdr_(l);  t != NULL;  t = memcdr_(t))
            {
#ifdef CPLUSPLUS
                if (!is_datamember_(t)) continue;
#endif
                if (!isbitfield_type(t->memtype)) break;
                sz = evaluate(t->u.membits);
                szmask = pcc_bf_sizeof(t->memtype) - 1;
                if (((bits & szmask) + sz) > (szmask+1))
                    bits = (bits + szmask) & ~szmask;
                if ((bits + sz) > MAXBITSIZE) break;
                bits +=  sz;
            }
            if (t != NULL && !isbitfield_type(t->memtype))
                /* a non-bit-field follows... */
                bitoff = (MAXBITSIZE - bits) & ~7;
            else
                bitoff = 0;
        }
        p->woffset = n, p->boffset = bitoff, p->bsize = k, p->typesize = 0;
        if (sort != bitoftype_(s_union)) bitoff += k;
    }
    else
    {   if (bitoff != 0)
        {   if (!(feature & FEATURE_PCC))
                n += sizeof_int;
            else
                n += (bitoff + 7)/8;
            bitoff = 0;
        }
#ifdef PASCAL /*ECN*/
        n = l->offset;
#else
        {   int32 oldn = n;
            n = padsize(n, alignof_member);     /* min member alignment */
            n = padsize(n, alignoftype(l->memtype));
            if (oldn != n) p->padded = YES;
        }
#endif
        p->woffset = n, p->boffset = 0, p->bsize = 0;
        p->typesize = sizeoftype(l->memtype);
        if (sort != bitoftype_(s_union)) n += p->typesize;
    }
    p->n = n, p->bitoff = bitoff;
    return YES;
}

/* Beware sizeoftype(t_ref) == sizeoftype(t_content).                   */
/* syn.c(cpp_sizeoftype) takes care of C++ rules about sizeof.          */
int32 sizeoftypenotepadding(TypeExpr *x, bool *padded)
{   SET_BITMAP m;
    ClassMember *l; TagBinder *b;
    switch (h0_(x))
    {   case s_typespec:
            m = typespecmap_(x);
            switch (m & -m)    /* LSB - unsigned/long etc. are higher */
            {   case bitoftype_(s_char):
                    return 1;
                case bitoftype_(s_int):
                    if (m & BITFIELD)
                        cc_rerr(sem_rerr_sizeof_bitfield);
                    return int_decodelength_(m, sizeof_short, sizeof_int,
                                                sizeof_long, sizeof_longlong);
                case bitoftype_(s_enum):
                    if (m & BITFIELD)
                        cc_rerr(sem_rerr_sizeof_bitfield);
                    return sizeof_int;
                case bitoftype_(s_double):
                    return (m & bitoftype_(s_short)) ? sizeof_float :
                           (m & bitoftype_(s_long)) ? sizeof_ldble :
                                                      sizeof_double;
                case bitoftype_(s_struct):
                case bitoftype_(s_class):
                    b = typespectagbind_(x);
                    if (!(attributes_(b) & (TB_DEFD|TB_UNDEFMSG)))
                    {   cc_err(sem_err_sizeof_struct, b);
                        attributes_(b) |= TB_UNDEFMSG;
                    }
                    {   StructPos p;
                        int32 m = 0;

                        p.n = p.bitoff = 0;
                        for (l = tagbindmems_(b); l != 0; l = memcdr_(l))
                        {   if (!structfield(l, bitoftype_(s_struct), &p))
                               continue;
                            if (padded != NULL) *padded = p.padded;
                            if (p.n > m) m = p.n;
                        }
#ifdef PASCAL /*ECN*/
                        p.n = m;
#endif
                        if (p.bitoff != 0)
                        {   if (!(feature & FEATURE_PCC))
                                p.n += sizeof_int;
                            else
                                p.n += (p.bitoff + 7)/8;
                        }
#ifdef CPLUSPLUS        /* Use the next line for C too?  OK by ANSI.    */
                        if (p.n == 0) p.n = 1;  /* empty C++ struct.    */
#endif
                        {   int32 n = padsize(p.n, alignoftype(x));
                                      /* often alignof_struct */
                            if (n != p.n && padded != NULL)
                                *padded = YES;
                            return n;
                        }
                    }
                case bitoftype_(s_union):
                    b = typespectagbind_(x);
                    if (!(attributes_(b) & (TB_DEFD|TB_UNDEFMSG)))
                    {   cc_err(sem_err_sizeof_struct, b);
                        attributes_(b) |= TB_UNDEFMSG;
                    }
                    {   int32 n = 0;                  /* n=0 is aligned */
                        for (l = tagbindmems_(b); l != 0; l = memcdr_(l))
/* @@@ beware sizeof_int here if alignof_struct ==1 in PCC mode packing */
/* for example "union {char a:1;} a[4];" looks buggy.                   */
                            n = max(n, isbitfield_type(l->memtype) ?
                                         sizeof_int : sizeoftype(l->memtype));
#ifdef CPLUSPLUS        /* Use the next line for C too?  OK by ANSI.    */
                        if (n == 0) n = 1;      /* empty C++ union.     */
#endif
                        return padsize(n, alignoftype(x));
                                /* often alignof_struct */
                    }
                case bitoftype_(s_typedefname):
                    return sizeoftype(bindtype_(typespecbind_(x)));
                case bitoftype_(s_void):
                    cc_rerr(sem_rerr_sizeof_void);
                    return 1;
                default: break;
            }
            /* drop through for now */
        default:
            syserr(syserr_sizeoftype, (long)h0_(x), (long)typespecmap_(x));
        case t_subscript:
            {   int32 n = sizeoftype(typearg_(x));
                if (typesubsize_(x)) n *= evaluate(typesubsize_(x));
                else typesubsize_(x) = globalize_int(1),
                     cc_rerr(sem_rerr_sizeof_array);
                return n;
            }
        case t_coloncolon:
            /* For int A::*x; ... x+1; ... and sizeof(B::b).            */
            return sizeoftype(typearg_(x));
        case t_ovld:
        case t_fnap:
            cc_rerr(sem_rerr_sizeof_function);
            /* drop through */
        case t_content:
        case t_ref:
            return sizeof_ptr;
    }
}

/* the C rules on type equivalence mean that equivtype does not have
   to worry about circularity */
/* equivtype notionally returns a boolean, but the true case is separated */
/* into result 2, identical (LISP equal) and result 1, merely equivalent. */
/* BEWARE: now (maybe temporarily) differing qualifiers on fn args can    */
/* still give result 2.                                                   */
/* AM:  @@@ In the long term, equivtype should also compute the unified   */
/*      type of t1 and t2, but beware store lifetime use, and cacheing.   */
static bool equivtype_4(TypeExpr *t1, TypeExpr *t2,
                        SET_BITMAP m1, SET_BITMAP m2,
                        bool widenformals)
{   int sofar = 2;
    for (;; (t1 = typearg_(t1), t2 = typearg_(t2)))
    {   m1 |= typedef_qualifiers(t1), m2 |= typedef_qualifiers(t2);
        t1 = princtype(t1),           t2 = princtype(t2);
        if (h0_(t1) != h0_(t2)) return 0;
        switch (h0_(t1))
        {
    case s_typespec:
                m1 |= typespecmap_(t1), m2 |= typespecmap_(t2);
                /* this next line does a lot:
                   1) it checks same basic type (struct/unsigned int/etc)
                      via set equality on type specifiers.
                   2) it tests pointer equality on tag binders.
                   3) it relies on typespecbind_() being 0 for simple types */
                return (m1 == m2 && typespecbind_(t1) == typespecbind_(t2)) ?
                              sofar :
#ifndef SIGNEDNESS_MATTERS
                       /* the 1 result here is slight paranoia re typedefs */
                       (m1 & m2 & bitoftype_(s_int) &&
                          (m1 & ~bitoftype_(s_signed)) ==
                          (m2 & ~bitoftype_(s_signed)))     ? 1 :
#endif
                              0;
    case t_coloncolon:
            if (typespecbind_(t1) != typespecbind_(t2)) return 0;
            continue;         /* now check elt types, with qualifiers */
    case t_content:
    case t_ref:
                /* two pointers match if they have the same const/volatile  */
                /* attribute and point to compatible types.  Ditto refs.    */
#ifdef PASCAL /*ECN*/
                if (t1 == t2) return 2;
#endif
                if ((m1|typeptrmap_(t1)) != (m2|typeptrmap_(t2))) return 0;
                m1 = m2 = 0;
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
            continue;         /* now check elt types, with qualifiers */
    case t_fnap:
/* Latest ANSI draft (Dec 88) is finally precise about type             */
/* equivalence.  Note that allowing () to be equivalent to non-()       */
/* things makes 'equivtype' not an equivalence relation!!!              */
            {   /* check arg types match... */
                FormTypeList *d1 = typefnargs_(t1), *d2 = typefnargs_(t2);
                bool old1 = t1->fnaux.oldstyle, old2 = t2->fnaux.oldstyle;
                if (maxargs_(t1) != maxargs_(t2) ||
                    minargs_(t1) != minargs_(t2) ||
                    t1->fnaux.variad != t2->fnaux.variad ||
                    t1->fnaux.flags != t2->fnaux.flags ||
                    t1->fnaux.oldstyle != t2->fnaux.oldstyle) sofar = 1;
/* The next 3 lines ensure "int f(); int f(x) char x; {}" is OK, but    */
/* that "int f(); int f(char x) {}" and "int f(); int f(int x,...) {}"  */
/* give the required ANSI errors.                                       */
                if (fntypeisvariadic(t1) != fntypeisvariadic(t2)) return 0;
                if (d1 == 0 && maxargs_(t1) == 999) d1 = d2, old1 = 1;
                if (d2 == 0 && maxargs_(t2) == 999) d2 = d1, old2 = 1;
                while (d1 && d2)
                {   TypeExpr *ft1 = d1->fttype, *ft2 = d2->fttype;
/* Function and array types were promoted to pointers by rd_declrhslist */
                    if (old1 || widenformals) ft1 = widen_formaltype(ft1);
                    if (old2 || widenformals) ft2 = widen_formaltype(ft2);
                    switch (qualfree_equivtype(ft1, ft2))
                    { default: return 0;
                      case 1: sofar = 1;
                      case 2: d1 = d1->ftcdr, d2 = d2->ftcdr;
                    }
                }
                /*
                 * Check either both fn's have or omit '...'
                 */
                if (d1 != d2) return 0;
            }
            m1 = m2 = 0;
            continue;    /* now check results */
    default:
            syserr(syserr_equivtype, (long)h0_(t1));
            return 0;
        }
    }
}

/* equivtype notionally returns a boolean, but the true case is separated */
/* into result 2, identical (LISP equal) and result 1, merely equivalent. */
/* BEWARE: now (maybe temporarily) differing qualifiers on fn args can    */
/* still give result 2.                                                   */
bool equivtype(TypeExpr *t1, TypeExpr *t2)
{
   return equivtype_4(t1, t2, 0, 0, NO);
}

/* @@@ I wish the use of this was documented!                           */
bool widened_equivtype(TypeExpr *t1, TypeExpr *t2)
{
   return equivtype_4(t1, t2, 0, 0, YES);
}

/* BEWARE: there is a sordid little ambiguity/accident in the Dec 88    */
/* ANSI draft (see equality ops (ignoring qualifiers) and type          */
/* qualifiers on array types) concerned with t1 (or t2) being a const   */
/* qualified array via typedef) and the other an array of const.        */
bool qualfree_equivtype(TypeExpr *t1, TypeExpr *t2)
{   /* forcing ON all qualifiers effectively ignores them.              */
    return equivtype_4(t1, t2, QUALIFIER_MASK, QUALIFIER_MASK, NO)/* != 0*/;
    /* HCM see above (case t_fnap of equivtype_4) why normalising the result
              to 0/1 may not be a good idea */
}

static FormTypeList *compositetypelist(FormTypeList *d1, FormTypeList *d2,
                                       bool old1, bool old2);

/* t3 = compositetype(t1,t2) forms the composite type of t1 and t2      */
/* into t3, if t1 and t2 are compatible(equivtype), otherwise it        */
/* returns 0.  See ANSI draft.                                          */
/* In implementation, the routine tries to return t1 if this is the     */
/* composite type to save store.                                        */
/* 'u' is set if top-level qualifiers are merely to be unioned.         */
/* 'm1' and 'm2' keep up with typedefs which have been pruned -- note   */
/* the behaviour for arrays (t_subscript).                              */
static TypeExpr *compositetype_5(TypeExpr *t1, TypeExpr *t2,
                          SET_BITMAP m1, SET_BITMAP m2, bool u)
{   TypeExpr *t3;
    if (t1 == t2 && m1 == m2) return t1;
    m1 |= typedef_qualifiers(t1), m2 |= typedef_qualifiers(t2);
    t1 = princtype(t1),           t2 = princtype(t2);
    if (h0_(t1) != h0_(t2)) return 0;
    switch (h0_(t1))
    {
case s_typespec:
            m1 |= typespecmap_(t1), m2 |= typespecmap_(t2);
            if (u) m1 |= m2 & QUALIFIER_MASK, m2 |= m1 & QUALIFIER_MASK;
            /* this next line does a lot:
               1) it checks same basic type (struct/unsigned int/etc)
                  via set equality on type specifiers.
               2) it tests pointer equality on tag binders.
               3) it relies on typespecbind_() being 0 for simple types */
            if (!(m1 == m2 && typespecbind_(t1) == typespecbind_(t2)
#ifndef SIGNEDNESS_MATTERS
                    || (m1 & m2 & bitoftype_(s_int) &&
                         ((m1^m2) & ~bitoftype_(s_signed)) == 0)
#endif
                   )) return 0;
            return
#ifdef REUSE
                typespecmap_(t1) == m1 ? t1 :
                typespecmap_(t2) == m1 ? t2 :
#endif
                primtype2_(m1, typespecbind_(t1));
    case t_content:
    case t_ref:
            m1 |= typeptrmap_(t1), m2 |= typeptrmap_(t2);
            if (u) m1 |= m2 & QUALIFIER_MASK, m2 |= m1 & QUALIFIER_MASK;
            if (m1 != m2) return 0;
            t3 = compositetype_5(typearg_(t1),typearg_(t2),0,0,0);
            return t3 == 0 ? 0 :
#ifdef REUSE
                   t3 == typearg_(t1) && m1 == typeptrmap_(t1) ? t1 :
#endif
                   mk_typeexpr1(h0_(t1), typearg_(t1), (Expr *)m1);
    case t_subscript:
          { Expr *e1 = typesubsize_(t1), *e2 = typesubsize_(t2), *e3;
            if (e1 != 0 && e2 != 0)
            {   if (evaluate(e1) != evaluate(e2)) return 0;
            }
            e3 = e1 ? e1 : e2;
            t3 = compositetype_5(typearg_(t1),typearg_(t2),m1,m2,u);
            return t3 == 0 ? 0 :
#ifdef REUSE
                   t3 == typearg_(t1) && e3 == e1 && m1 == 0 ? t1 :
#endif
                 mk_typeexpr1(t_subscript, typearg_(t1), (Expr *)e3);
          }
    case t_fnap:
          { TypeExprFnAux s;
            FormTypeList *d1 = typefnargs_(t1), *d2 = typefnargs_(t2), *d3;
                bool old1 = t1->fnaux.oldstyle, old2 = t2->fnaux.oldstyle;
            int32 max1 = maxargs_(t1), max2 = maxargs_(t2), max3 = max1;
/* The code here mirrors that in equivtype().                           */
            if (fntypeisvariadic(t1) != fntypeisvariadic(t2)) return 0;
            if (d1 == 0 && max1 == 999) d1 = d2, old1 = 1, max3 = max2;
            if (d2 == 0 && max2 == 999) d2 = d1, old2 = 1, max3 = max1;
            if (d1 == 0 && d2 == 0)
                d3 = 0;
            else
            {   d3 = compositetypelist(d1,d2,old1,old2);
                if (d3 == 0) return 0;
            }
            t3 = compositetype_5(typearg_(t1),typearg_(t2),0,0,0);
            return t3 == 0 ? 0 :
#ifdef REUSE
/*                 t3 == typearg_(t1) && d3 == d1 && ... ? t1 :         */
#endif
                mkTypeExprfn(t_fnap, t3, d3,
                    packTypeExprFnAux(s,
                                (int)length((List *)d3),     /* vestige */
                                (int)max3,
                                0,      /* variad: not special          */
                                0,      /* oldstyle: new                */
                                0       /* flags: none */
                             ));
          }
    default:
            syserr(syserr_compositetype, (long)h0_(t1));
            return 0;
    }
}

static TypeExpr *qualunion_compositetype(TypeExpr *t1, TypeExpr *t2)
{    return compositetype_5(t1,t2,0,0,1);
}

static FormTypeList *compositetypelist(FormTypeList *d1, FormTypeList *d2,
                                       bool old1, bool old2)
{   if (d1==0 || d2==0) return 0;
    {   TypeExpr *ft1 = d1->fttype, *ft2 = d2->fttype, *tc;
        FormTypeList *d3;
/* Function and array types were promoted to pointers by rd_declrhslist */
        if (old1) ft1 = widen_formaltype(ft1);
        if (old2) ft2 = widen_formaltype(ft2);
        tc = qualunion_compositetype(ft1, ft2);
        if (tc == 0) return 0;
        d3 = (FormTypeList *)syn_list3(0, d1->ftname, tc);
        if (d1->ftcdr != 0 || d2->ftcdr != 0)
        {   FormTypeList *d4 = compositetypelist(d1->ftcdr, d2->ftcdr,
                                                 old1, old2);
            if (d4 == 0) return 0;
            d3->ftcdr = d4;
        }
        return d3;
    }
}

/* lubtype() and friends return a type if OK, or give an error message
   and return 0.  Worry that they are still rather ragged.
   Note that 'the usual unary coercions' omit 'char' and (possibly) float.
   BEWARE: result type is ASSUMED to be princtype'd by caller. Since
   result is an r-value qualifiers are irrelevant.
*/
static TypeExpr *lubtype(AEop op, TypeExpr *t1, TypeExpr *t2)
/* Take care of 'the usual coercions' for e.g. s_times.
   Relations are done separately, as are the pointer cases of s_plus */
{   TypeExpr *x1 = princtype(t1), *x2 = princtype(t2);
    SET_BITMAP m1 = typespecmap_(x1), m2 = typespecmap_(x2);
    if (h0_(x1) == s_typespec && h0_(x2) == s_typespec &&
        (m1 & ARITHTYPEBITS) != 0 && (m2 & ARITHTYPEBITS) != 0)
    {   if ((m1 | m2) & bitoftype_(s_double))
        {   if (!floatableop_(op))  /* % << >> etc. */
            {   typeclash(op); return 0; }
            /* The 2 next lines fix a bug in pre-1.60a versions whereby   */
            /* (long)op(double) was incorrectly treated as (long double). */
            if ((m1 & (bitoftype_(s_double) | bitoftype_(s_long))) ==
                (bitoftype_(s_double) | bitoftype_(s_long))) return te_ldble;
            if ((m2 & (bitoftype_(s_double) | bitoftype_(s_long))) ==
                (bitoftype_(s_double) | bitoftype_(s_long))) return te_ldble;
            if ((m1 & (bitoftype_(s_double)|bitoftype_(s_short))) ==
                bitoftype_(s_double)) return te_double;
            if ((m2 & (bitoftype_(s_double)|bitoftype_(s_short))) ==
                bitoftype_(s_double)) return te_double;
            /* the next line is an ANSI draft (May 86) feature.
               In the PCC case coerceunary would have ensured that any
               such arguments would have already been coerced to double */
            return te_float;
        }
        /* the next line is an ANSI special case */
        if (op == s_leftshift || op == s_rightshift) return x1;
        if ((m1 | m2) & bitoftype_(s_long))
        {   /* @@@ what do longlong ops do? "int_decodelength_"?        */
            if ((m1 & (bitoftype_(s_unsigned)|bitoftype_(s_long))) ==
                      (bitoftype_(s_unsigned)|bitoftype_(s_long)) ||
                (m2 & (bitoftype_(s_unsigned)|bitoftype_(s_long))) ==
                      (bitoftype_(s_unsigned)|bitoftype_(s_long)))
                /* either unsigned long gives result                     */
                return te_ulint;
#ifdef never /* @@@ save for possible later warn on <unsigned>op<signed> */
             /* as recommended to implementers by ANSI.                  */
>>>>    if ((m1^m2) & bitoftype_(s_unsigned) &&
>>>>        (op == s_div || op == s_rem ||  /* s_rightshift done above */
>>>>         op == s_less || op == s_lessequal ||
>>>>         op == s_greater || op == s_greaterequal))
>>>>        /* This warning combats AM (and others?) getting confused */
>>>>        /* that "-1L < 5U" is true.  NO longer true (Oct 88).     */
>>>>        cc_warn(sem_warn_unsigned, op);
#endif
            if ((m1^m2) & bitoftype_(s_unsigned))
                /* one unsigned, one long: Oct 88 ANSI rule (like pcc?)   */
                return sizeof_long > sizeof_int ? te_lint : te_ulint;
            return te_lint;
        }
        if ((m1 | m2) & bitoftype_(s_unsigned)) return te_uint;
        return te_int;  /* only case left */
    }
    /* Apr 92: some old error recovery retired here...                  */
    typeclash(op);
    return 0;
}

/* patch up for special treatment of types of formal parameters at defn */
TypeExpr *modify_formaltype(TypeExpr *t)
{   TypeExpr *t1 = princtype(t);
    switch (h0_(t1))
    {   case t_fnap:       /* turn f() to (*f)() */
            return ptrtotype_(t);
        case t_subscript:  /* turn a[] to *a     */
            /* We also turn a const array (only via typedef) into a     */
            /* pointer to const components.  Beware, ANSI is unclear.   */
            return mkqualifiedtype(ptrtotype_(typearg_(t1)),
                                   typedef_qualifiers(t));
        default:
            return t;
    }
}

/* For safety w.r.t. prototype and non-prototype forms, we pass all
   short ints/chars and floats as repectively ints and doubles and
   do callee narrowing.
*/
TypeExpr *widen_formaltype(TypeExpr *t)
{   TypeExpr *t1 = princtype(t);      /* can ignore typedef qualifiers? */
    if (h0_(t1) == s_typespec)
    {   SET_BITMAP m1 = typespecmap_(t1);
        if (m1 & bitoftype_(s_double))
            /* turn 'float' = 'short double' to 'double'. */
            return m1 & bitoftype_(s_short) ? te_double : t;
/* Should FEATURE_PCC do anything here re unsigned preserving?          */
        if (m1 & bitoftype_(s_char))
            /* turn 'char' to 'int'. */
            return te_int;
        if (m1 & bitoftype_(s_int))
            /* turn 'short' to 'int'. */
            return int_isshort_(m1) ?
                    (sizeof_short==sizeof_int &&
                     m1 & bitoftype_(s_unsigned) ? te_uint:te_int) : t;
        return t;
    }
    return t;
}

/* printf/scanf format checker... (belongs in a separate file?) */

enum fmtsort { FMT_INT, FMT_LINT, FMT_FLT, FMT_PTR };

static void fmt1chk(char *buf, int bufp, Expr *arg, enum fmtsort sort)
{   TypeExpr *t = princtype(typeofexpr(arg));
    switch (h0_(t))
    {
case t_content:
        if (sort == FMT_PTR) return;
        break;
case s_typespec:
        {   SET_BITMAP m = typespecmap_(t);
            if ((m & bitoftype_(s_int) &&
                   sort == (m & bitoftype_(s_long) ? FMT_LINT:FMT_INT)) ||
#ifndef CPLUSPLUS
/* In C enums coerceunary() to ints.  We suppress this, so that C++ is  */
/* easy (and so that we can warn as ANSI suggest, but we need to allow  */
/* printf("%d", enumconst) in C (although not C++? -- unclear).         */
/* See also the uses of COERCE_ARG in mkfnap()/coerceunary.             */
                (m & bitoftype_(s_enum) &&
                   sort == FMT_INT) ||
#endif
                (m & bitoftype_(s_double) &&
                   sort == FMT_FLT))
              return;
        }
        break;
    }
    cc_warn(sem_warn_format_type, t, (int)bufp, buf);
}

static bool fmtchk(String *fmt, ExprList *l, int32 flavour)
{   /* flavour=1 => s/f/printf, flavour=2 => s/f/scanf - see stdio.h pragma */
    /* returns TRUE if no floating point will be used & printf case */
    StringSegList *z;
    int state = 0; int32 nformals = 0, nactuals = 0;
    bool no_fp_conversions = (flavour == 1);
    char buf[20]; int bufp = 1;   /* 0..20 */
    enum fmtsort xint = FMT_INT;
    for (z = fmt->strseg; z!=NULL; z = z->strsegcdr)
    {   char *s = z->strsegbase;
        int32 len = z->strseglen;
        while (len-- > 0)
        {   int ch = char_untranslation(*s++);
#ifndef NO_SELF_CHECKS
/* The following lines augment printf checks to cover cc_err, syserr &c */
/* via #pragma -v3 present in mip/globals.h.                            */
/* #define'ing NO_SELF_CHECKS disables this and saves about 100 bytes.  */
            if (ch == '$' && flavour == 3 && state == 0)
            {   /* the next line is ugly because it assumes string not split */
                if (char_untranslation(*s) != 'l')   /* 'l' => use curlex */
                {   nformals++;
                    if (l) nactuals++, l = cdr_(l);
                }
            }
            else
#endif
            if (ch == '%')
                bufp=1, buf[0]=ch,
                state = state==0 ? 1 : 0,
                xint = FMT_INT;
            else if (state != 0) switch ((bufp<20 ? buf[bufp++] = ch:0), ch)
            {   case '*':
                    if (flavour==2) { state = 9; break; }
                    nformals++;
                    if (l)
                    {   fmt1chk(buf, bufp, exprcar_(l), FMT_INT);
                        nactuals++;
                        l = cdr_(l);
                    }
                    break;
                case '.':
                    break;
                case '+': case '-': case ' ': case '#':
                    state = state==1 ? 2 : 99; break;
                case 'l':
                    xint = FMT_LINT;
                    /* drop through */
                case 'h': case 'L':
                    /* not treated properly yet */
                    state = state < 5 ? 5 : 99;
                    break;
                case 'd': case 'i': case 'o': case 'u': case 'x': case 'X':
                case 'c':
                    if (state == 9) { state = 0; break; } /* scanf suppressed */
                    nformals++;
                    if (l)
                    {   fmt1chk(buf, bufp, exprcar_(l), flavour==2 ? FMT_PTR :
                                     ch=='c' ? FMT_INT:xint);
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
                    {   fmt1chk(buf, bufp, exprcar_(l),
                                flavour==2 ? FMT_PTR:FMT_FLT);
                        nactuals++;
                        l = cdr_(l);
                    }
                    state = 0;
                    break;
                case 's': case 'p': case 'n':
                    if (state == 9) { state = 0; break; } /* scanf suppressed */
                    nformals++;
                    if (l)
                    {   fmt1chk(buf, bufp, exprcar_(l), FMT_PTR);
                        nactuals++;
                        l = cdr_(l);
                    }
                    state = 0;
                    break;
                case '[':
                    if (state == 9) { state = 0; break; } /* scanf suppressed */
                    if (flavour==2) /* scanf only */
                    {   /* check well-formed one day */
                        nformals++;
                        if (l)
                        {   fmt1chk(buf, bufp, exprcar_(l), FMT_PTR);
                            nactuals++;
                            l = cdr_(l);
                        }
                        state = 0;
                        break;
                    }
                    /* else drop through */
                default:
                    if ('0'<=ch && ch<='9') break;
                    cc_warn(sem_warn_bad_format, (int)ch);
                    state = 0;
                    break;
            }
        }
    }
    if (state != 0) cc_warn(sem_warn_incomplete_format);
    nactuals += length(l);
    if (nactuals != nformals)
        cc_warn(sem_warn_format_nargs,
                 (long)nformals, nformals==1 ? "":"s", (long)nactuals);
    return no_fp_conversions;
}

static void formatcheck(Expr *e, ExprList *l, FormTypeList *d, int32 flavour)
{   Expr *fmt = 0;
    ExprList *ll = l;
    for (; d && ll; d = d->ftcdr, ll = cdr_(ll)) fmt = exprcar_(ll);
    if (d == 0 && fmt != 0)
    {   for (;;)
        {   switch (h0_(fmt))
            {
    case s_invisible:
                    fmt = compl_(fmt); continue;
    case s_cast:
    case s_addrof:  fmt = arg1_(fmt); continue;
    case s_string:  if (fmtchk((String *)fmt, ll, flavour))
/* Here we had a printf, sprintf or fprintf where the format string was    */
/* a literal string that showed that no floating point conversions would   */
/* be required. Convert to a call to _printf, _sprintf or _fprintf, which  */
/* are restricted (smaller) versions of the relevant library functions.    */
/* NB that this conversion is enabled by a #pragma present in <stdio.h>    */
/* and so will not confuse users who avoid #pragma and who redefine        */
/* their own printf etc without the benefit of the standard header file.   */
/* The following test gives false (hence no "else syserr()") for calls     */
/* like  (**printf)("hi");  we could catch this, but not really worth it.  */
                    {   if (h0_(e) == s_invisible &&
                            h0_(orig_(e)) == s_binder)
                        {   Symstr *fname = bindsym_((Binder *)orig_(e));
/* @@@ Why be paranoid to avoid sharing the sim.printf(s_addrof)??         */
/* Can simplify() corrupt otherwise?                                       */
/* Review the names 'printf' for C++ name munging too.                     */
                            if (fname == sim.yprintf)
                                e = mkaddr(arg1_(sim.xprintf));
                            else if (fname == sim.yfprintf)
                                e = mkaddr(arg1_(sim.xfprintf));
                            else if (fname == sim.ysprintf)
                                e = mkaddr(arg1_(sim.xsprintf));
                            /* this is a valid drop through place, for */
                            /* scanf, eprintf (norcroft internal) &c.  */
                        }
                    }
                    break;
/*
 * The warning message generated here will arise only if the -fussy -ansi
 * switches are set and indicates that a printf/scanf has been given where
 * the a format that is not a literal string and so which can not be checked
 * for validity by the compiler.  It is also necessary in this case to
 * link to the version of printf (etc) that supports floating point
 * conversions even in cases where store could have been saved by using
 * the integer-only version.
 */
    default:        if (1<=flavour && flavour<=3 &&
                            (feature & (FEATURE_PCC|FEATURE_FUSSY)) ==
                                FEATURE_FUSSY)
                        cc_warn(sem_warn_uncheckable_format);
                    break;
            }
            break;
        }
    }
}

/* l-value like stuff ... */

/* The need for skip_invisible is largely because of the nastiness      */
/* of storing information about the sort of field selection (bitfield   */
/* or C++ member fn) in the s_dot node, which can be protected by       */
/* an s_invisible (e.g. when 'this->a' is written just as 'a').         */
static Expr *skip_invisible(Expr *e)
{   while (h0_(e) == s_invisible) e = compl_(e);
    return e;
}

static Expr *ignore_invisible(Expr *e)          /* bad name */
{
    for (;;)
        if (h0_(e) == s_invisible)
            e = compl_(e);
        else if (h0_(e) == s_cast)
            e = arg1_(e);
        else
            return e;
}

static bool isverysimplelvalue(Expr *e, bool maybevolatile, bool allowpostinc)
{   while (h0_(e) == s_dot) e = arg1_(e);  /* x.a.b is simple if x is ...   */
                                           /* and lvalues do not have casts */
    if (h0_(e) == s_binder)
        return maybevolatile || !isvolatile_type(bindtype_((Binder *)e));
    else if (allowpostinc && h0_(e) == s_displace)
        return isverysimplelvalue(arg1_(e), NO, NO);
    else
        return NO;
}

static bool issimplevalue_i(Expr *e, bool allowpostinc)
{   AEop op;
    e = ignore_invisible(e);
    op = h0_(e);
    return op == s_integer || isverysimplelvalue(e, NO, allowpostinc) ||
           (op == s_addrof && isverysimplelvalue(ignore_invisible(arg1_(e)), NO, allowpostinc)) ||
           ( (op == s_times || op == s_plus || op == s_minus || op == s_div ||
              op == s_rem || op == s_leftshift || op == s_rightshift || op == s_and) &&
             issimplevalue_i(arg1_(e), allowpostinc) &&
             issimplevalue_i(arg2_(e), allowpostinc));
}

bool issimplevalue(Expr *e)     /* @@@ should be static! */
{   return issimplevalue_i(e, NO);
}

static bool issimplelvalue_i(Expr *e, bool allowpostinc)
{
    while (h0_(e) == s_dot) e = arg1_(e);
    if (isverysimplelvalue(e, YES, NO))
        return YES;
    else if (h0_(e) == s_content)
    {   e = ignore_invisible(arg1_(e));
        if ( isverysimplelvalue(e, NO, allowpostinc) ||
             ( (h0_(e) == s_plus || h0_(e) == s_minus) &&
               issimplevalue_i(e, allowpostinc) ) )
            return YES;
    }
    return NO;
}

bool issimplelvalue(Expr *e)    /* @@@ should be static! */
{   return issimplelvalue_i(e, NO);
}

static Expr *RemovePostincs(Expr *e, Expr **incs) {
    switch h0_(e) {
    case s_integer: case s_binder:
        return e;
    case s_displace:
      { Expr *inc = mk_expr2(s_assign, type_(e), arg1_(e), arg2_(e));
        *incs = (*incs == 0) ? inc :
                               mkbinary(s_comma, *incs, inc);
        return arg1_(e);
      }

    case s_cast: case s_content:
      { Expr *a1 = skip_invisible(arg1_(e));
        Expr *newa1 = RemovePostincs(a1, incs);
        return (a1 == newa1) ? e:
                               mk_expr1(h0_(e), type_(e), newa1);
      }

    case s_dot:         /* s_arrow gone? */
      { Expr *a1 = skip_invisible(arg1_(e));
        Expr *newa1 = RemovePostincs(a1, incs);
        return (a1 == newa1) ? e:
            mk_exprwdot(s_dot, type_(e), newa1, exprdotoff_(e));
      }

    case s_times: case s_div: case s_rem:
    case s_leftshift: case s_rightshift: case s_and:
    case s_plus: case s_minus:
      { Expr *a1 = skip_invisible(arg1_(e)),
             *a2 = skip_invisible(arg2_(e));
        Expr *newa1 = RemovePostincs(a1, incs),
             *newa2 = RemovePostincs(a2, incs);
        return (a1 == newa1 && a2 == newa2) ? e :
            mk_expr2(h0_(e), type_(e), newa1, newa2);
      }

/* @@@ AM: I am not convinced by these routines and even less convinced */
/* that they should appear here in sem.  simplify.c must be better.     */
    case s_dotstar:     /* s_arrowstar gone? */
    default:
        pr_expr(e); syserr("help");
        return e;
    }
}

/* Expression AE tree node creators and associated functions */

Expr *mkintconst(TypeExpr *te, int32 n, Expr *e)
{    return mk_expr2(s_integer, te, (Expr *)n, e);
}

static Expr *mkinvisible(TypeExpr *t, Expr *olde, Expr *newe)
/* The purpose of s_invisible nodes in the tree is to compile one thing
   whilst retaining the original expression for error messages.
   Thus ++x can compile as x=x+1 but produce an error about ++ in &++x.
*/
{   /* The next line may be useful one day...                      */
    /*           if (h0_(newe) == s_error) return errornode;       */
    /* s_integer nodes have a special invisible representation:    */
    if (h0_(newe) == s_integer) return mkintconst(t, intval_(newe), olde);
    return mk_expr2(s_invisible, t, olde, newe);
}

/* ensurelvalue() the is the lvalue checking (and coercing, for C++     */
/* t_ref's) routine analogous to coerceunary() which coerces and checks */
/* rvalues.                                                             */
/* It has two phases -- first it must check from the type info          */
/* that e is not a array, function or other 'const' type.               */
/* However, note that & can still be applied to such things.                */
/* This is the difference between lvalue and modifiable lvalue.             */
/* Note also that in PCC given 'int a[10]' then &a has type (int *), the    */
/* same as 'a' in expression context, whereas ANSI say it is 'int (*)[10]). */
/* which is more uniform (and always puts a * on the type).                 */
/* The ANSI choice reduces confusion in multi-dimensional arrays.           */
 /* The following example may help:
  *         struct { int r[7], s[7] } x;
  *         f() { printf("%d\n", &x.s - &x.r);
  *               printf("%d\n", x.s - x.r);   }
  * under ANSI prints 1 then 7, under PCC 7 then 7.
  */
/* Secondly it must check that the expression really can be an lvalue --    */
/* care must be taken for things like (e.a.b.c).                            */
/* It currently either returns its arg unchanged, or errornode.   LIE C++   */
/* In C++ (CPLUSPLUS) it converts refs to indirections...                   */
/* but not all callers are fixed to use this properly yet..              */

static Expr *ensurelvalue(Expr *e, AEop op)
{   Expr *x;
    bool OK = 0;                /* this is a gross hack !!!!!!!!!!!!    */
                                /* it stops the b_addrof bit below.     */
    if (h0_(e) == s_error) return errornode;
#ifdef CPLUSPLUS
    {   TypeExpr *t = princtype(typeofexpr(e));
        if (h0_(t) == t_ref)
        {   TypeExpr *t2 = typearg_(t);
            e = mkinvisible(t2, e, mk_expr1(s_content,t2,e));
            OK = 1;
        }
    }
#endif
    {   TypeExpr *t = typeofexpr(e);
#ifdef CPLUSPLUS
        while (h0_(t) == t_coloncolon) t = typearg_(t);
        /* @@@ how do s_member refs (see above) work?                   */
#endif
        if (op != s_addrof && (qualifiersoftype(t) & bitoftype_(s_const)))
            cc_rerr(sem_rerr_assign_const, e);
        if (op == s_addrof && isbitfield_type(t))
        {   cc_err(sem_err_bitfield_address);
            return errornode;
        }
        t = princtype(t);
        if (h0_(t) == t_fnap || h0_(t) == t_subscript)
        {   if (op != s_addrof)
            {   cc_err(sem_err_lvalue, e);
                return errornode;
            }
/* This code catches things like &(a.b) where "struct {int b[4];} a" */
/* but, oh misfortune, that is just what happens inside the macro    */
/* for offsetof(). Ah well.                                          */
            if (feature & FEATURE_PCC) cc_warn(sem_warn_addr_array, e);
            /* remember &(f().m) with m an array member is OK!       */
        }
    }
    if (OK) return e;
    for (x = e;;) switch (h0_(x))
    {   case s_member:
/* Always valid as member -- there are no enumconst s_member's          */
/* (s_binder used) or register members.                                 */
/* Maybe could fix the implicit 'this' here though.                     */
            return e;
        case s_binder:
        {   Binder *b = (Binder *)x;
            if (isenumconst_(b))
            /* will always be seen via a s_integer nodes */
            {   cc_err(sem_err_lvalue1, b);
                return errornode;
            }
            if (op == s_addrof)
            {   if (bindstg_(b) & bitofstg_(s_register))
                {    /* In spite of the real PCC giving a hard error,   */
                     /* for taking a reg var address (or at least       */
                     /* generating bad code); PCC mode gives a          */
                     /* (suppressible) warning!  Makefiles beware.      */
#ifndef CPLUSPLUS
                     cc_pccwarn(sem_rerr_addr_regvar, b);
#endif
                     bindstg_(b) &= ~bitofstg_(s_register);
                }
                if (bindstg_(b) & b_globalregvar)
                {   cc_err(
    "$b is a global register variable - can't take its address", b);
                    return errornode;
                }
                bindstg_(b) |= b_addrof;
            }
            return e;
        }
        case_s_any_string      /* &"abc" seems legal */
        case s_subscript:
        case s_arrow:
        case s_content:
            return e;
        case s_dot:
            x = arg1_(x);
            break;
#ifdef CPLUSPLUS
        case s_arrowstar:       /* don't believe [ES,p71,l-1]!          */
            return e;
        case s_dotstar:         /* don't believe [ES,p71,l-1]!          */
            x = arg1_(x);
            break;
#endif
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
            cc_err(sem_err_lvalue2, h0_(x));
            return errornode;
#ifdef CPLUSPLUS
        case s_cond:                          /* needs fixing */
            cc_err(sem_err_lvalue2, h0_(x));
            return errornode;
#endif
        case s_cast:
/* @@@ LDS: I hope this all works...                                        */
/* Casts in l-values get special error fixup code since pcc allows. The fix */
/* is ugly: If the context is ++ or -- (pre- or post) then the cast expr    */
/* will later have its address taken, so here we lie about the operator to  */
/* force b_addrof in any binders recursively encountered...                 */
/* AM: bug here: (float)x = 1; syserrs.                                     */
            cc_pccwarn(sem_rerr_lcast);
            if (monadneedslvalue_(op)) op = s_addrof;
            return ((h0_(ensurelvalue(arg1_(x), op)) == s_error)
                         ? errornode : e);
    }
}

#ifdef SOFTWARE_FLOATING_POINT
static Expr *mk1fnap(Expr *fn, ExprList *args)
{
   TypeExpr *t = princtype(typeofexpr(fn));  /* typedefs irrelevant */
/* @@@ someone is also paranoid here about sharing of &sim.fns.         */
   fn = mkinvisible(t, fn,
                 mk_expr1(s_addrof, ptrtotype_(t), fn));
   return mk_expr2(s_fnap, typearg_(t), fn, (Expr *)args);
}
#endif

/* The following routine tidies up pre-existing code, later improve for */
/* other lengths of floating point.                                     */
static void fltrep_to_widest(DbleBin *dest, FloatCon *src)
{   /* Consider extending this interface for long double > double.      */
    if (sizeof_float < sizeof_double && src->floatlen & bitoftype_(s_short))
       fltrep_widen(&src->floatbin.fb, dest);
    else
       flt_move(dest, &src->floatbin.db);
}

static FloatCon *fltrep_from_widest(
                        DbleBin *src, SET_BITMAP flag, char *floatstr)
{   /* Consider extending this interface for long double > double.      */
    /* Binary representation is definitive, but keep any text form.     */
    FloatCon *m = real_of_string(floatstr ? floatstr : "<expr>", 0);
    m->floatlen = flag;
    if (sizeof_float < sizeof_double && flag & bitoftype_(s_short))
        fltrep_narrow(src, &m->floatbin.fb);
    else
        flt_move(&m->floatbin.db, src);
    return m;
}

#define mk_expr2_ptr(op,t,a,b) trydiadreduce(mk_expr2(op,t,a,b),addrsignmap_)

static Expr *trydiadreduce(Expr *c, SET_BITMAP flag)
{ AEop op = h0_(c);
  Expr *a = arg1_(c), *b = arg2_(c);
  if ((op == s_leftshift || op == s_rightshift) && h0_(b) == s_integer)
  {  int32 n = intval_(b);
     if ((unsigned32)n >= (unsigned32)(
           (flag & bitoftype_(s_long) ? 8*sizeof_long : 8*sizeof_int)))
        /* see ANSI spec, valid shifts are 0..31 */
        cc_warn(sem_warn_bad_shift(flag, (long)n));
     if (n == 0 && h0_(a) != s_integer)  /* int case reduced below */
       /* the s_invisible node allows detection of "(x<<0)=1" lvalue error */
       return mkinvisible(type_(c), c, a);
  }
  if ((op == s_div || op == s_rem) && h0_(b) == s_integer)
  {  int32 n = intval_(b);
     if (n == 0)
        cc_warn(sem_warn_divrem_0, op);
  }
/* The rationale here is that unsigned values are in the range 0, 1, .. */
/* and so a proper discrimination for counters is either n==0 vs. n!=0  */
/* or n==0 vs. n>0.  Anything that might suggest that negative values   */
/* might exist is WRONG, this n<0, n<=0, n>=0, 0<=n, 0>n, 0>=n will all */
/* lead to diagnostics here.                                            */
  if (isinequality_(op) && (flag & bitoftype_(s_unsigned)) &&
       (h0_(b) == s_integer && intval_(b) == 0 && op != s_greater ||
        h0_(a) == s_integer && intval_(a) == 0 && op != s_less))
     cc_warn(sem_warn_ucomp_0, op);
/* The following two lines reduce (e.g.) (2 || 1/0) to 1.  Similar code */
/* exists in mkcond() (q.v.).  @@@ However, it would seem that they can */
/* contravene the ANSI draft in that they also reduce                   */
/* int x[(2 || (0,1))] to int x[1], which silently misses a constraint  */
/* violation that the comma operator shall not appear in const exprs.   */
/* Similarly x[2 || f()].  AM thinks the ANSI draft is a mess here.     */
/* Note: mktest() has transformed (e.g.) (3.4 || e) to (3.4!=0 || e).   */
  if (op == s_oror && h0_(a) == s_integer && intval_(a))
      return mkintconst(type_(c),1,c);  /* always has a type_ field */
  if (op == s_andand  && h0_(a) == s_integer && !intval_(a))
      return mkintconst(type_(c),0,c);  /* always has a type_ field */

  if (h0_(a) == s_integer && h0_(b) == s_integer &&
                             (flag & bitoftype_(s_unsigned)))
  { unsigned32 m = intval_(a), n = intval_(b), r;
    bool ok = YES;
    switch (op)
    {   case s_plus:        r = m+n;  ok = r>=m && r>=n;  break;
        case s_minus:       r = m-n;  ok = r<=m;          break;
        case s_times:       r = m*n;  ok = m==0 || r/m==n; break;
        case s_div:         if (n==0) return c; r = m/n; break;
        case s_rem:         if (n==0) return c; r = m%n; break;
        case s_power:       return c;
        case s_and:         r = m&n;  break;
        case s_or:          r = m|n;  break;
        case s_xor:         r = m^n;  break;
        case s_andand:      r = m&&n; break;
        case s_oror:        r = m||n; break;
        case s_leftshift:   r = m<<n; ok = (r>>n) == m; break;
        case s_rightshift:  r = m>>n; break;
        case s_equalequal:  r = m==n; break;
        case s_notequal:    r = m!=n; break;
        case s_less:        r = m<n;  break;
        case s_lessequal:   r = m<=n; break;
        case s_greater:     r = m>n;  break;
        case s_greaterequal:r = m>=n; break;
        default: syserr(syserr_trydiadicreduce, (long)op);
                 return c;
    }
    if (sizeof_int == 2)
    {   if (!(flag & bitoftype_(s_long)) && (r & 0xffff0000))
            r &= 0x0000ffff, ok = NO;
    }
    if (!ok)
        cc_ansi_warn(sem_rerr_udiad_overflow(op,m,n,r));
    return mkintconst(type_(c),r,c);  /* always has a type_ field */
  }
  if (h0_(a) == s_integer && h0_(b) == s_integer)
  { int32 m = intval_(a), n = intval_(b), r;
    bool ok = YES;
/* @@@ The next lines assume that host 'int' overflow wraps round silently.  */
    switch (op)
    {   case s_plus:        r = m+n;
             ok = (m^n) < 0 || (m|n|r) >= 0 || (m&n&r) < 0; break;
        case s_minus:       r = m-n;
             ok = (n>=0 ? r<=m : r>m);                      break;
        case s_times:       r = m*n;
             ok = m==0 || n==0 || (m^n^r) >= 0 && r/m == n; break;
        case s_div:         if (n==0) return c;
/* The only case where division can give overflow is when the greatest  */
/* possible negative number is divided by -1.  I.e. iff n,m,r all -ve.  */
                            r = m/n; ok = (m&n&r) >= 0;     break;
/* Interesting ANSI question: what should effect of                     */
/* (signed)0x80000000%-1 be?  Mathematically 0, but maybe the division  */
/* overflowed and so we got a trap.  See if ANSI boobed!                */
        case s_rem:         if (n==0) return c; r = m%n;    break;
        case s_power:       return c;
        case s_and:         r = m&n;  break;
        case s_or:          r = m|n;  break;
        case s_xor:         r = m^n;  break;
        case s_andand:      r = m&&n; break;
        case s_oror:        r = m||n; break;
        case s_leftshift:   r = m<<n; ok = signed_rightshift_(r,n)==m; break;
        case s_rightshift:  r = TARGET_RIGHTSHIFT(m,n); break;
        case s_equalequal:  r = m==n; break;
        case s_notequal:    r = m!=n; break;
        case s_less:        r = m<n;  break;
        case s_lessequal:   r = m<=n; break;
        case s_greater:     r = m>n;  break;
        case s_greaterequal:r = m>=n; break;
        default: syserr(syserr_trydiadicreduce1, (long)op);
                 return c;
    }
    if (sizeof_int == 2)
    {   if (!(flag & bitoftype_(s_long)) && r != (int16)r)
            r = (int16)r, ok = NO;
    }
    if (!ok)
        cc_ansi_warn(sem_rerr_diad_overflow(op,m,n,r));
    return mkintconst(type_(c),r,c);  /* always has a type_ field */
  }

  while (h0_(a) == s_invisible) a = compl_(a);
  while (h0_(b) == s_invisible) b = compl_(b);

  if (h0_(a) == s_floatcon && h0_(b) == s_floatcon)
  { DbleBin d,e,r;
    bool ok;
    fltrep_to_widest(&d, (FloatCon *)a);
    fltrep_to_widest(&e, (FloatCon *)b);
    switch (op)
    {   case s_plus:  ok = flt_add(&r,&d,&e); break;
        case s_minus: ok = flt_subtract(&r,&d,&e); break;
        case s_times: ok = flt_multiply(&r,&d,&e); break;
        case s_div:   ok = flt_divide(&r,&d,&e); break;
        case s_power: return c;
#define SEM_FLTCMP(op) mkintconst(type_(c),flt_compare(&d,&e) op 0,c);
        case s_equalequal:   return SEM_FLTCMP(==);
        case s_notequal:     return SEM_FLTCMP(!=);
        case s_less:         return SEM_FLTCMP(<);
        case s_lessequal:    return SEM_FLTCMP(<=);
        case s_greater:      return SEM_FLTCMP(>);
        case s_greaterequal: return SEM_FLTCMP(>=);
#undef SEM_FLTCMP
        default: syserr(syserr_trydiadicreduce2, (long)op);
                 return c;
    }
    if (!ok)
    {   cc_ansi_warn(sem_warn_fp_overflow(op));
        /* improve */
        return c;
    }
    /* Safety, I presume (possibly can remove things like volatile?):   */
    flag &= bitoftype_(s_double)|bitoftype_(s_short)|bitoftype_(s_long);
    {   FloatCon *m = fltrep_from_widest(&r, flag, 0);
        return mkinvisible(type_(c), c, (Expr *)m);
    }
  }
#ifdef SOFTWARE_FLOATING_POINT
  if (flag & bitoftype_(s_double))
  {  Expr *fname;
     if (flag & bitoftype_(s_short)) switch(op)
     {
/* It may be that raising to a power will be done by a library call
   that will be introduced later, and so I do not need to map it onto
   a function call here... whatever else it seems a bit ugly, but it does
   not make any difference to C, only Fortran */
        case s_plus:        fname = sim.fadd;           break;
        case s_minus:       fname = sim.fsubtract;      break;
        case s_times:       fname = sim.fmultiply;      break;
        case s_div:         fname = sim.fdivide;        break;
        case s_power:       return c;  /* probably wrong */
        case s_equalequal:  fname = sim.fequal;         break;
        case s_notequal:    fname = sim.fneq;           break;
        case s_less:        fname = sim.fless;          break;
        case s_lessequal:   fname = sim.fleq;           break;
        case s_greater:     fname = sim.fgreater;       break;
        case s_greaterequal:fname = sim.fgeq;           break;
        default:            syserr(syserr_fp_op, (long)op);
                            fname = NULL;   /* To keep compiler quiet */
     }
     else switch(op)
     {
        case s_plus:        fname = sim.dadd;           break;
        case s_minus:       fname = sim.dsubtract;      break;
        case s_times:       fname = sim.dmultiply;      break;
        case s_div:         fname = sim.ddivide;        break;
        case s_power:       return c;  /* probably wrong */
        case s_equalequal:  fname = sim.dequal;         break;
        case s_notequal:    fname = sim.dneq;           break;
        case s_less:        fname = sim.dless;          break;
        case s_lessequal:   fname = sim.dleq;           break;
        case s_greater:     fname = sim.dgreater;       break;
        case s_greaterequal:fname = sim.dgeq;           break;
        default:            syserr(syserr_fp_op, (long)op);
                            fname = NULL;   /* To keep compiler quiet */
     }
     return mk1fnap(fname, mkExprList(mkExprList(0, b), a));
  }
#endif
  return c;
}

static Expr *trymonadreduce(AEop op, Expr *a, Expr *c, SET_BITMAP flag)
{ if (h0_(a) == s_integer)
  { unsigned32 m = intval_(a), r;
    bool ok = YES;
    switch (op)   /* BEWARE: 2's complement means signed == unsigned */
    {   case s_monplus:  r = m; break;
        case s_neg:      r = -m;
                         if (!(flag & bitoftype_(s_unsigned)))
                             ok = (int32)(m&r) >= 0;
                         break;
        case s_bitnot:   r = ~m; break;
        case s_boolnot:  r = !m; break;
        default:         syserr(syserr_trymonadicreduce, (long)op);
        case s_content:  return c;
    }
    if (sizeof_int == 2)
    {   if (flag & bitoftype_(s_unsigned))
        {   if (!(flag & bitoftype_(s_long)) && (r & 0xffff0000))
                r &= 0x0000ffff, ok = NO;
        }
        else
        {   if (!(flag & bitoftype_(s_long)) && r != (int16)r)
                r = (int16)r, ok = NO;
        }
    }
    if (!ok)
    {   if (flag & bitoftype_(s_unsigned))
            cc_ansi_warn(sem_rerr_umonad_overflow(op,m,r));
        else
            cc_ansi_warn(sem_rerr_monad_overflow(op,m,r));
    }
    return mkintconst(type_(c),r,c);  /* always has a type_ field */
  }
  while (h0_(a) == s_invisible) a = compl_(a);
  if (h0_(a) == s_floatcon)
  { int32 flag = ((FloatCon *)a)->floatlen;     /* same as type info.   */
    DbleBin d,r;
    bool ok;
    fltrep_to_widest(&d, (FloatCon *)a);
    switch (op)
    {   case s_monplus:  ok = flt_move(&r,&d); break;
        case s_neg:      ok = flt_negate(&r,&d); break;
        default: syserr(syserr_trymonadicreduce1, (long)op);
                 return c;
    }
    if (!ok)
    {   cc_ansi_warn(sem_warn_fp_overflow(op));
        /* improve */
        return c;
    }
    {   FloatCon *m = fltrep_from_widest(&r, flag, 0);
        return mkinvisible(type_(c), c, (Expr *)m);
    }
  }
#ifdef SOFTWARE_FLOATING_POINT
  { TypeExpr *t = princtype(typeofexpr(a));     /* hmm */
    SET_BITMAP m = h0_(t)==s_typespec ? typespecmap_(t) : 0;
    if (m & bitoftype_(s_double))
    { Expr *fname;
      if (m & bitoftype_(s_short)) switch (op)
      {
        case s_monplus: return a;
        case s_neg:     fname = sim.fnegate;    break;
        default:        syserr(syserr_fp_op, (long)op);
                        fname = NULL;   /* To keep compiler quiet */
      }
      else switch (op)
      {
        case s_monplus: return a;
        case s_neg:     fname = sim.dnegate;    break;
        default:        syserr(syserr_fp_op, (long)op);
                        fname = NULL;   /* To keep compiler quiet */
      }
      return mk1fnap(fname, mkExprList(0, a));
    }
  }
#endif
  return c;
}

#ifdef SOFTWARE_FLOATING_POINT
Expr *fixflt(Expr *e)
{
    TypeExpr *t1, *t2;
    Expr *a = arg1_(e), *fname;
    SET_BITMAP m1, m2;
    if (h0_(e) != s_cast) return e;
    t1 = princtype(type_(e));           /* hmm (SOFTWARE_FLOATING_POINT) */
    t2 = princtype(typeofexpr(a));      /* hmm (SOFTWARE_FLOATING_POINT) */
    if (h0_(t1) != s_typespec || h0_(t2) != s_typespec) return e;
    m1 = typespecmap_(t1);
    if (m1 & bitoftype_(s_void)) return e; /* cast to void: leave alone  */
    m2 = typespecmap_(t2);
    if (m1 & bitoftype_(s_double))
    {   /* this converts something into a floating point value           */
        if (m2 & bitoftype_(s_double))
        {   /* maybe changes the width of a floating point value?        */
            if (m1 & bitoftype_(s_short))
            {   if (m2 & bitoftype_(s_short)) return a;
                else return mk1fnap(sim.fnarrow, mkExprList(0, a));
            }
            else if (m2 & bitoftype_(s_short))
                return mk1fnap(sim.dwiden, mkExprList(0, a));
            else return a;
        }
        if (m1 & bitoftype_(s_short))
            fname = (m2 & bitoftype_(s_unsigned)) ? sim.ffloatu : sim.ffloat;
        else fname = (m2 & bitoftype_(s_unsigned)) ? sim.dfloatu : sim.dfloat;
        return mk1fnap(fname, mkExprList(0, a));
    }
    if (m2 & bitoftype_(s_double))
    {   /* some sort of fixing here                                      */
        if (m2 & bitoftype_(s_short))
            fname = (m1 & bitoftype_(s_unsigned)) ? sim.ffixu : sim.ffix;
        else fname = (m1 & bitoftype_(s_unsigned)) ? sim.dfixu : sim.dfix;
        return mkcast(s_cast, mk1fnap(fname, mkExprList(0, a)), type_(e));
    }
/* Otherwise the cast does not involve any FP types.                     */
    return e;
}
#endif

static Expr *trycastreduce(Expr *a, TypeExpr *tc, Expr *c, bool explicit)
/* Args somewhat redundant -- c = (s_cast,tc,a)                       */
{ if (h0_(a) == s_integer)
  { unsigned32 n = intval_(a), r = n;
    SET_BITMAP m;
    TypeExpr *x = princtype(tc);        /* type being cast to.        */
                             /* (can ignore typedef qualifiers).      */
    switch (h0_(x))          /* signed/unsigned shouldn't matter here */
    {
case s_typespec:
        m = typespecmap_(x);
        switch (m & -m)    /* LSB - unsigned/long etc. are higher */
        {
    case bitoftype_(s_char):
            r = (issignedchar_(m) && (n & 0x80))
                ? (n | ~(int32)0xff) : (n & 0xff);
            break;
    case bitoftype_(s_enum):    /* ansi C says treat like 'int'.        */
    case bitoftype_(s_int):
            if ((int_isshort_(m) ? sizeof_short :
                 (m & bitoftype_(s_long)) ? sizeof_long : sizeof_int) == 2)
                r = (!(m & bitoftype_(s_unsigned)) && (n & 0x8000))
                    ? (n | ~(int32)0xffff) : (n & 0xffff);
            break;
    case bitoftype_(s_void):
            r = 0;
            goto omit_check;            /* mkvoided did it for us.      */
    case bitoftype_(s_double):
            {   TypeExpr *ta = princtype(type_(a));  /* 'a' known s_integer */
                /* remember (double)(-1u) != (double)(-1) ... */
/* Use int_to_real() rather than flt_itod() or flt_utod() since it fills */
/* in a string with the number etc etc etc.                              */
                if (h0_(ta) == s_typespec && (typespecmap_(ta) & ARITHTYPEBITS))
                    return mkinvisible(tc, c, (Expr *)int_to_real(
                        n, typespecmap_(ta)&bitoftype_(s_unsigned), m));
            }
            /* drop through */
            default: return c;     /* always harmless to return cast */
        }
        if (!explicit && r != n)
            cc_ansi_warn(sem_rerr_implicit_cast_overflow(x,n,r));
        break;
case t_content:
        r = n==0 ? TARGET_NULL_BITPATTERN : n;
        break;        /* cast to pointer                */
default:
        return c;     /* should not happen (prev error) */
    }
omit_check:
    return mkintconst(tc,r,c);
  }

  while (h0_(a) == s_invisible) a = compl_(a);
  if (h0_(a) == s_floatcon)
  { /* We have a cast (possibly implicit) on a floating constant value. */
    /* Produce a new FloatCon value with the new type.                  */
    TypeExpr *tcp = princtype(tc);      /* qualifiers ignored in casts. */
    if (h0_(tcp) == s_typespec)
    {   SET_BITMAP m = typespecmap_(tcp);
        int32 n; bool ok;
        DbleBin d;
        fltrep_to_widest(&d, (FloatCon *)a);
        switch (m & -m)
        {   case bitoftype_(s_char):
                if (issignedchar_(m)) ok = flt_dtoi(&n, &d);
                else ok = flt_dtou((unsigned32 *)&n, &d);
/* I do not complain about (char)258.0, but I will moan at (char)1.e11   */
/* where 1.e11 will not fit in an int. This is at least compatible with  */
/* run-time behaviour on casts.                                          */
                n = (issignedchar_(m) && (n & 0x80))
                    ? (n | ~(int32)0xff) : (n & 0xff);
                break;
            case bitoftype_(s_int):
                if (m & bitoftype_(s_unsigned))
                    ok = flt_dtou((unsigned32 *)&n, &d);
                else ok = flt_dtoi(&n, &d);
                if ((int_isshort_(m) ? sizeof_short :
                     (m & bitoftype_(s_long)) ? sizeof_long : sizeof_int) == 2)
                    n = (!(m & bitoftype_(s_unsigned)) && (n & 0x8000))
                        ? (n | ~(int32)0xffff) : (n & 0xffff);
                break;
            case bitoftype_(s_enum):
                ok = flt_dtoi(&n, &d); /* This case seems pretty dodgy! */
                break;
            case bitoftype_(s_double):
              { char *oldfloatstr = ((FloatCon *)a)->floatstr;
                FloatCon *fc = fltrep_from_widest(&d, m, oldfloatstr);
                return mkinvisible(tc, c, (Expr *)fc);
              }
            default:
                /* this includes the (void)1.3 case for which warning has */
                /* already been given.  Could do ok=1, n=0 instead.       */
                return c;
        }
        if (!ok)
        {   cc_warn(sem_warn_fix_fail);
            n = 0;
        }
        return mkintconst(tc,n,c);
    }
    return c;   /* nothing doing */
  }
  return c;
}

#ifdef TARGET_IS_C40
static Expr *ignore_invisible_but_not_casts(Expr *e)
{
    for (;;)
        if (h0_(e) == s_invisible)
            e = compl_(e);
        else if (h0_(e) == s_cast)
	  return NULL;
        else
            return e;
}
#endif /* TARGET_IS_C40 */

/* check_index_overflow checks if an address computation (possibly via  */
/* a subscript operation) indexes an array whose size, n, is manifest.  */
/* If so then it checks that a (constant) offset is in the range 0..n.  */
/* IF a dereference later occurs, then we moan at the use of offset n   */
/* too.  At this time, though, the stride will have been multiplied     */
/* into the offset, so we have to compensate.  Argument 'stride' is     */
/* a TypeExpr * for the stride type or NULL if not dereferenced (yet).  */
/* Note also that modify_formal_type makes this hard to do (and less    */
/* justifiable) in  "int f(int a[4]) { return a[-1]; }".  Hence don't.  */
static void check_index_overflow(Expr *ptr, Expr *disp, int posneg,
                                 TypeExpr *stride)
{
#ifdef TARGET_IS_C40
  /*
   * unfortantely this test does not work for the following code ...
   *
   * int a[2]; ((char *)a)[7] = 1;
   *
   * Since I have had complaints from users (and the Helios programming
   * team) about this, I am going to abort this test if a cast occurs.
   * (I do not consider myself competant enought to fix the bug properly).
   */

  if ((ptr = ignore_invisible_but_not_casts(ptr)) == NULL)
    return;
#else
  ptr = ignore_invisible(ptr);    /* @@@ skip_invisible doesn't work? */
#endif /* TARGET_IS_C40 */
  
    if (h0_(disp) == s_integer && h0_(ptr) == s_addrof)
    {   TypeExpr *t = type_(ptr);
        if (h0_(t) == t_content)
        {   TypeExpr *t2 = princtype(typearg_(t));
            /* princtype since array type may be typedef'd.             */
            if (h0_(t2) == t_subscript)
            {   Expr *bound = typesubsize_(t2);
                int32 n = posneg * intval_(disp);
                if (stride==NULL)
                {   /* always whinge for n<0 or n>limit...              */
                    if (n < 0 || bound && n > evaluate(bound))
                        cc_warn(sem_warn_index_ovfl, n);
                }
                else
                {   /* ... but also if n=limit AND dereferencing.       */
                    int32 k = sizeoftype(stride);
                    if (bound && k!=0 && n/k==evaluate(bound))
                        cc_warn(sem_warn_index_ovfl, n/k);
                }
            }
        }
    }
}

static void check_index_dereference(Expr *e)
{   if (h0_(e) == s_content)
    {   Expr *a = arg1_(e);
        TypeExpr *t = type_(e);
        if (h0_(a) == s_plus)
#ifdef OLD_POINTER_CODE
            check_index_overflow(arg2_(a), arg1_(a), 1, t),
#endif
            check_index_overflow(arg1_(a), arg2_(a), 1, t);
        if (h0_(a) == s_minus)
            check_index_overflow(arg1_(a), arg2_(a), -1, t);
    }
}

static Expr *nonconst1(Expr *e)
/* make this table driven */
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

void moan_nonconst(Expr *e, char *s)
{
    if (debugging(DEBUG_TYPE))
    { eprintf("moan_nonconst on "); pr_expr(e); }
    e = nonconst1(e);
    if (e == 0) cc_err(sem_err_nonconst, s);
    else if (h0_(e) == s_binder)
        cc_err(sem_err_nonconst1, s, (Binder *)e);
    else
        cc_err(sem_err_nonconst2, s, h0_(e));
}

static bool check_narrow_subterm(Expr *e, TypeExpr *te, TypeExpr *tt)
{   tt = princtype(tt), te = princtype(te);     /* ignore qualifiers */
    if (h0_(tt) == s_typespec && h0_(te) == s_typespec &&
        (/* moan at int/long ops in any float context */
           typespecmap_(tt) & ~typespecmap_(te) & bitoftype_(s_double) ||
         /* moan at plain ops in long context, both same floatiness. */
           !((typespecmap_(tt) ^ typespecmap_(te)) & bitoftype_(s_double)) &&
           typespecmap_(tt) & ~typespecmap_(te) & bitoftype_(s_long) ||
         /* moan at (float) short double in other float context */
           typespecmap_(tt) & bitoftype_(s_double) &&
           typespecmap_(te) & ~typespecmap_(tt) & bitoftype_(s_short)
         ))
    {
retry:  switch h0_(e)
        {
    case s_invisible:
            e = orig_(e); goto retry;
    case s_comma:
            e = arg2_(e); goto retry;
    case s_monplus:
            e = arg1_(e); goto retry;
    case s_and: case s_or: case s_xor:
    case s_leftshift: case s_rightshift:
    case s_plus: case s_minus: case s_times: case s_div:
    /* but not s_ptrdiff -- since long a = ptr1-ptr2 is always ok */
    case s_rem:
    case s_neg: case s_bitnot:
#ifdef EXTENSION_VALOF
    case s_valof:
#endif
#ifndef FORTRAN         /* re-instate something like this for f77 soon  */
            if (suppress & D_IMPLICITNARROWING) xwarncount++;
            else
                cc_warn(sem_warn_low_precision, h0_(e));
#endif
            return 1;
    case s_cond:
            /* "long x = p() ? 1 : 2;" cannot reasonably give a warning */
            /* but think more about "long x = p() ? 33000 : 1".         */
            return check_narrow_subterm(arg2_(e), te, tt) ? 1 :
                   check_narrow_subterm(arg3_(e), te, tt);
#ifdef never
    case s_integer:
            if ((e = intorig_(e)) != 0) goto retry;
            /* drop through */
    case s_andand: case s_oror:
    case s_equalequal: case s_notequal: /* + other relops */
#endif
    default:
            break;
        }
    }
    return 0;
}

#ifdef CPLUSPLUS
/* merge with similar routine in syn.c */
static Expr *thisify(Expr *e)
{   if (h0_(e) != s_member) syserr("thisify");
    else
    {   Symstr *sv = memsv_((ClassMember *)e);
        Binder *thisb = findbinding(thissym);
        if (thisb && h0_(thisb) == s_binder)
        {   binduses_(thisb) |= u_referenced;
/* mkfieldselector should take a ClassMember *sv, not a Symstr *sv.     */
            return mkfieldselector(s_arrow, (Expr *)thisb, sv, 0);
        }
        cc_err("No 'this' pointer to access member $r", sv);
    }
    return errornode;
}
#endif /* CPLUSPLUS */

static Expr *coerce2(Expr *e, TypeExpr *t)
  /* does NOT check consistent - use mkcast for this */
  /* really only for implicit arithmetic casts       */
{   Expr *r;
    TypeExpr *te = princtype(typeofexpr(e));   /* ignore cast qualifiers   */
    if (equivtype(t,te) == 2) return e;        /* identical types, no cast */
    (void)check_narrow_subterm(e, te, t);
    r = mk_expr1(s_cast, t, e);
#ifdef SOFTWARE_FLOATING_POINT
    return fixflt(trycastreduce(e,t,r,NO));
#else
    return trycastreduce(e,t,r,NO);
#endif
/* the invisible node in the following should never be seen and
   needs re-thinking re constant reduction.                          */
/*     return mkinvisible(t, e, mk_expr1(s_cast, t, e));         */
}

static Expr *bf_container(Expr *e, SET_BITMAP m)
{   if (!(h0_(e) == s_dot && m & BITFIELD)) syserr(syserr_bf_container);
    /* @@@ te_int is OK while sizeof_int == sizeof_long.                */
    /* in the longer term move to an 'unbitfield_type()' container?     */
    return mk_exprwdot(s_dot,
                       mkqualifiedtype(te_int, m & QUALIFIER_MASK),
                       arg1_(e), exprdotoff_(e));
}

/* The complications below for coercion context are due to AM's         */
/* implementation of the fact that in most contexts fns/arrays are      */
/* coerced to pointers exactly when char/short/bitfield are widened to  */
/* int.  (I.e. neither for lvalue (inlcuding sizeof) context, both for  */
/* rvalue context).  The exception is comma context, where ANSI have    */
/* (foolishly?) specified that, given char a[10], we have               */
/* sizeof(0,a)==sizeof(char *), sizeof(0,a[0])==sizeof(char).  Yuk.     */
/* The implementation chosen is further encouraged by the observation   */
/* that otherwise we would make comma-context the only non-lvalue       */
/* phrase to have non-widened types, which seems like a bug invitation. */
/* For C++ errors, and also C *warnings*, for abuse of int/enum we      */
/* now defer widening of enum to int.                                   */
enum Coerce_Context { COERCE_NORMAL, COERCE_ARG, COERCE_COMMA,
                      COERCE_ASSIGN };

/* coerceunary ought to be idempotent for its uses.                     */
/* Its caller has ensured that 'e' is not s_error (errornode).          */
static Expr *coerceunary_2(Expr *e, enum Coerce_Context c)
{   TypeExpr *qt = typeofexpr(e), *t = princtype(qt);
    /* NB. in principle coerceunary() converts lvalues to rvalues and   */
    /* hence could remove top level qualifiers.  However, this would    */
    /* turn over (a little?) more store and routines elsewhere happily  */
    /* manage to ignore such qualifiers.                                */
    /* Surprisingly, 'qt' is needed for coercing lvalues such as x in   */
    /* "typedef int t[3][4]; const t x;".  This must coerce x to        */
    /* type (const int (*)[4]).  See t_subscript case.                  */
    SET_BITMAP m;
    if (debugging(DEBUG_TYPE))
        {   eprintf("Coerceunary called: "); pr_expr(e); eprintf("\n"); }
    /* Here is just the place to check that "a[n]" is (in)valid given   */
    /* the declaration "int a[n]".                                      */
    check_index_dereference(e);
    switch (h0_(t))
    {
case s_typespec:
        m = typespecmap_(t);
        switch (m & -m)    /* LSB - unsigned/long etc. are higher */
        {
    case bitoftype_(s_char):            /* INTEGRALTYPEBITS             */
    case bitoftype_(s_int):
    case bitoftype_(s_enum):
/* For explanation of the next line see the comment for COERCE_COMMA.   */
            if (c == COERCE_COMMA) return e;
/* As part of the previous line, we now have to unravel char/bitfields  */
/* in NORMAL/ARG context, but they may be embedded inside s_comma:s.    */
/* This is needed to avoid s_dot syserr() and to preserve the useful    */
/* fact that only l-value and fn-return aetrees can have narrow values. */
            if (h0_(e) == s_comma && m & (BITFIELD|
                   bitoftype_(s_char)|bitoftype_(s_short)|bitoftype_(s_enum))
                                  && !(m & bitoftype_(s_long)))
            {   /* BEWARE: smashing of tree known to be unshared.       */
                arg2_(e) = coerceunary_2(arg2_(e), c);
                type_(e) = typeofexpr(arg2_(e));
                return e;
            }
/* bitfieldvalue can promote to signed or unsigned int.                 */
/* For C++ it can also promote to enum.   @@@ review long bit fields.   */
/* @@@ For systems in which sizeof_long>sizeof_int then maybe we should */
/* allow long bit values and promotion to (unsigned)long as an ANSI-    */
/* conformant extension.  See what the ANSI C++ committee think.        */
            if (m & BITFIELD)  /* rvalue context here */
            {   Expr *ee = skip_invisible(e);
                return bitfieldvalue(ee, m, bf_container(ee, m));
            }
/* CPLUSPLUS: it is arguable that COERCE_ARG (unchecked params) should  */
/* not promote its arg here.  Then we could warn on printf("%d", enum)  */
/* In C, of course, this is perfectly OK.                               */
            if (m & bitoftype_(s_enum) && c == COERCE_ASSIGN) return e;
/* ANSI says coerce unsigned or signed char (or short) to (signed) int  */
/* if shorter.  PCC says unsigned char (or short) coerce to unsigned.   */
/* This code now copes with sizeof_int==sizeof_short.                   */
/* It does not cope with sizeof_int==sizeof_char in ANSI mode.          */
            if (m & (bitoftype_(s_char)|bitoftype_(s_short)
                                       |bitoftype_(s_enum)) &&
                    /* next line copes with "long long" encoding.       */
                    !(m & bitoftype_(s_long)))
                e = (feature & FEATURE_PCC) ?
                    /* Someone might like to worry about the case       */
                    /* where default char is unsigned in PCC mode       */
                    /* does it then promote to signed int (as now) or   */
                    /* unsigned int?  K&R say (signed) int.             */
                    coerce2(e,
                        (m & bitoftype_(s_unsigned)) ? te_uint:te_int) :
                    coerce2(e,
                        (sizeof_short==sizeof_int && int_isshort_(m)
                            && m & bitoftype_(s_unsigned)) ? te_uint:te_int);
            return e;
    case bitoftype_(s_double):
/* The pcc mode code here may need improving (as char above) if PCC has */
/* sizeof(0,(float)0)==sizeof(float) -- this code gives sizeof(double). */
            if ((c == COERCE_ARG || feature & FEATURE_PCC) &&
                (m & bitoftype_(s_short)))
                return coerce2(e, te_double);
            return e;
    case bitoftype_(s_struct):
    case bitoftype_(s_class):
    case bitoftype_(s_union):
    case bitoftype_(s_void):
            return e;
    default:
            break;
        }
        /* drop through */
default:
        syserr(syserr_coerceunary1, (long)h0_(t), (long)typespecmap_(t));
        return e;
case t_content:
        return e;
case t_subscript:
        if (!(feature & FEATURE_PCC))   /* recovery better be good.     */
            (void)ensurelvalue(e, s_addrof);
        /* Complications here coercing a const array into a pointer to  */
        /* a const element.  Use "t a[n]" -> "(t *)&a"                  */
            {   TypeExpr *t1 = ptrtotype_(
                    mkqualifiedtype(typearg_(t), typedef_qualifiers(qt)));
                return mkinvisible(t1, e,
                           mk_expr1(s_cast, t1, mkaddr(e)));
            }
case t_ref:
        {   TypeExpr *t2 = typearg_(t);
            return mkinvisible(t2, e, mk_expr1(s_content,t2,e));
        }
#ifdef CPLUSPLUS
case t_coloncolon:
        {   e = thisify(e);
/* @@@ the next line will go when coerceunary callers expect errornode  */
            if (h0_(e) == s_error) syserr("coerceunary(::)");
            return e;
        }
case t_ovld:
        return coerceunary_2(
                   (Expr *)ovld_resolve_addr(e, typeovldlist_(t)), c);
case t_fnap:
        {   Expr *ee = skip_invisible(e); /* for implicit 'this->f'     */
            if (h0_(ee) == s_dot)         /* C++ member function        */
                /* @@@ give a type warning on the next line?            */
                return coerceunary_2(exprdotmemfn_(ee),c);
            if (h0_(ee) == s_dotstar)   /* see [ES, p71]                */
            {   cc_err("non-call site '.*' or '->*' yielding function");
                return errornode;
            }
        }
        if (h0_(e) == s_binder && bindstg_((Binder *)e) & b_pseudonym)
        {   Binder *b = realbinder_((Binder *)e);
            if (bindstg_(b) & b_memfna)
                t = (TypeExpr *)syn_list3(t_coloncolon, t,
                    parentclass((Binder *)b));
            e = (Expr *)b;
        }
#else
case t_fnap:
#endif
        {   TypeExpr *t2 = ptrtotype_(t);
            return mkinvisible(t2, e, mk_expr1(s_addrof,t2,e));
        }
    }
}

#define coerceunary(e) coerceunary_2(e, COERCE_NORMAL)

/* Null pointer constants are defined in Dec 88 draft, section 3.2.2.3. */
/* This routine accepts (void *)(int *)0 as NPC, drafts leave undef'd.  */
static bool isnullptrconst(Expr *e)
{   if (h0_(e) == s_integer)
    {   TypeExpr *t = princtype(type_(e));   /* type_(e)==typeofexpr(e) */
        if (h0_(t) == s_typespec && typespecmap_(t) & INTEGRALTYPEBITS &&
                                    intval_(e) == 0 ||
/* Note: (void *)0 is a NPC, but (const void *)0 isn't.  We assume      */
/* (void *const)0 is a NPC, because qualifiers are ignored on non-      */
/* lvalues, of which casts are an example.                              */
            h0_(t) == t_content && equivtype(typearg_(t), te_void) &&
                                   intval_(e) == TARGET_NULL_BITPATTERN)
          return 1;
    }
    return 0;
}

static Expr *pointerofint(Expr *e, AEop op, TypeExpr *t)
{   if (!isnullptrconst(e) || (op != s_equalequal && op != s_notequal))
        cc_pccwarn(sem_rerr_pointer_compare, op);
    return coerce2(e,t);
}

Expr *mkswitch(Expr *a)
{   if (h0_(a) == s_error) return errornode;
    {
/* @@@ We should check warnings (C++ errs!) for switch(enum) etc here.  */
        Expr *b = coerceunary(a);
        TypeExpr *t = princtype(typeofexpr(b));
        if (h0_(t) == s_typespec)
        {   SET_BITMAP m = typespecmap_(t);
            switch (m & -m)
            {   case bitoftype_(s_int):       /* possibly long/unsigned */
                    return b;
            }
        }
/* It is debatable whether 'int' is the right type on the next line,    */
/* but observe that only erroneous cases (flts/pointers/etc) go here.   */
        return mkcast(s_switch, b, te_int);
    }
}

/* mktest() is used for exprs in conditional (0/non-0) context.
   It does two things.
   1) warns (optionally later) of '=' if '==' could be more sensible.
   2) adds a !=0 node to any non-boolean values, so result is always 0/1
      which also takes care of type-checking modulo error messages.
*/
Expr *mktest(AEop opreason, Expr *a)
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
            else cc_warn(sem_warn_odd_condition, op);
            /* drop through */
        default:
            IGNORE(opreason);      /* re-think whether this is useful */
            if (isrelational_(op)) return a;  /* type OK */
            return mkbinary(s_notequal, a, mkintconst(te_int,0,0));
    }
}

static Expr *mkvoided(Expr *e)
/* currently always returns its arg */
{   Expr *x; AEop op;
    for (x = e;;) switch (op = h0_(x))
    {   case s_invisible:
            x = orig_(x); break;
        case s_cast:    /* This could otherwise moan about the wrong    */
                        /* operator since coerce2 can insert s_cast:s   */
                        /* without an s_invisible node.                 */
            x = arg1_(x); break;
        case s_fnap:    /* Only non-void results come here: moan?       */
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
/* Suppression of warnings for explicit casts is now done by caller.    */
            if (!(isassignop_(op) || isincdec_(op)))
                cc_warn(sem_warn_void_context, op);
            /* drop through */
        case s_assign:
            return e;
#ifdef EXTENSION_VALOF
        case s_valof:
            return e;
#endif
    }
}

static Expr *mklet(Binder *v, TypeExpr *t, Expr *e)
{   if (h0_(e) == s_error) return errornode;
    return mk_exprlet(s_let, t, mkSynBindList(0, v), e);
}

Expr *mkunary(AEop op, Expr *a)
{   Expr *c;
    TypeExpr *t;
    if (monadneedslvalue_(op))    /* & ++ -- */
    {   c = ensurelvalue(a, op);
        c = (op == s_addrof) ? mkaddr(c) : mkincdec(op, c);
        /* add an invisible node if expression was transformed (for errors) */
        return (h0_(c) == s_error) ? errornode :
               (h0_(c) == op) ? c :
               (t = typeofexpr(c), mkinvisible(t, mk_expr1(op,t,a), c));
    }
    if (h0_(a) == s_error) return errornode;
/* The next line treats monadic + as coercing enum->int.  Check C++.    */
    a = coerceunary(a);
    t = typeofexpr(a);
    /* the operators below all take rvalues in which qualifiers are ignored */
    switch (op)        /*  + - ~ ! *  */
    {   case s_content:
            if (indexable(t))
            {   t = indexee(t);
                /* kill all accesses via (qualified) void pointers. */
                if (isvoidtype(t))
                    {   cc_err(sem_rerr_void_indirection);
                        return errornode;
                    }
                break;
            }
            typeclash(op);
            return errornode;
        case s_boolnot:
            a = mktest(op,a);
            if (h0_(a) == s_error) return errornode;
            t = typeofexpr(a);
            break;
     /* case s_monplus: */
     /*     ANSI have ruled that +ptr is illegal, but 0+ptr is OK! ...  */
     /*     ... to make +ptr valid add:     if (indexable(t)) break;    */
     /* case s_bitnot: */
     /* case s_neg:    */
        default:               /* (s_bitnot(~), s_neg(-), s_monplus(+) */
            t = princtype(t);
            if (h0_(t) == s_typespec && (typespecmap_(t) &
                 (op == s_bitnot ? INTEGRALTYPEBITS : ARITHTYPEBITS)))
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
    return trymonadreduce(op,a,c,
         h0_(t)==s_typespec ? typespecmap_(t) : addrsignmap_);
}

static Expr *mkshift(AEop op, Expr *a, int32 n) /* for bitfields only!  */
{
    if (n == 0) return a;
/* @@@ The next line is a hack to avoid spurious error messages when    */
/* 1<<31 is used to mask off a bitfield, as in x.a=1.                   */
    else if (h0_(a) == s_integer && type_(a) == te_int && op==s_leftshift)
        return mkintconst(te_int, (unsigned32)(intval_(a)) << n, 0);
    else return mkbinary(op, a, mkintconst(te_int, n, 0));
}

static Expr *bitfieldvalue(Expr *ebf, SET_BITMAP m, Expr *ewd)
{ /*
   * MEMO: exprbsize_ is size; exprmsboff_ is offset from msb;
   * (ebf must be s_dot of bits, ewd yields the word containing bitfield).
   * ANSI require 'int' bit fields treated as 'unsigned' or 'signed' just
   * when chars would be.  E.g. that { unsigned a:7,b:32 } gives x.a type int,
   * and x.b unsigned, whereas PCC gives both unsigned (cf. unsigned char).
   * tr is type of result.  The form (s_invisible) of result is exploited.
   */
    int32 size = exprbsize_(ebf);
    TypeExpr *tr = bf_promotedtype(type_(ebf), size);
/* AM @@@ There is work to be done (e.g. in the next line) so that we   */
/* choose signed/unsigned enum bit fields depending on set of values?   */
/* Currently enum bit fields act as plain int (i.e. may/maynot extend). */
    return mkinvisible(tr, ebf, issignedchar_(m) ?
        mkshift(s_rightshift,
                mkshift(s_leftshift, ewd, exprmsboff_(ebf)),
                MAXBITSIZE-size) :
        mkbinary(s_and,
                 mkshift(s_rightshift, ewd,
                         MAXBITSIZE-size-exprmsboff_(ebf)),
                 mkintconst(te_int, lsbmask(size), 0)));
}

static Expr *bitfieldinsert(AEop op, Expr *ebf, Expr *ewd, Expr *val)
{   int32 size = exprbsize_(ebf);
    int32 lspos = MAXBITSIZE-size-exprmsboff_(ebf);
    return
        mkassign(op,    /* not mkbinary (else x.bit = 1 gives 2 errs).  */
            ewd,
            mkbinary(s_or,
                mkbinary(s_and, ewd,
                    mkintconst(te_int, ~(lsbmask(size) << lspos), 0)),
                mkshift(s_leftshift,
                    mkbinary(s_and, val,
                        mkintconst(te_int, lsbmask(size), 0)),
                    lspos)));
}

/* bitfieldstuff() has been so re-arranged to show its similarity to    */
/* to mkopassign (q.v.).   Rationalise the two one day.                 */
static Expr *bitfieldstuff(AEop op, Expr *a, SET_BITMAP m,
                           Expr *ewd, Expr *b)
/* special case: op==s_postinc has b = 1 or -1 according to ++ or -- */
{   /* note that 'ewd' must not have side effects as it is duplicated */
    Expr *bb = op == s_assign ? b :
            mkbinary(op == s_postinc ? s_plus : unassignop_(op),
                     bitfieldvalue(a, m, ewd),
                     b);
    return bitfieldinsert(op == s_postinc ? s_displace : s_assign,
                          a, ewd, mkcast(op, bb, te_int));
}

static Expr *bitfieldassign(AEop op, Expr *a, TypeExpr *ta, Expr *b)
/* special case: op==s_postinc has b = 1 or -1 according to ++ or --    */
/* NB: ta == typeofexpr(a), eliminate this arg?                         */
{   Expr *bitword, *r;
    SET_BITMAP m;
/* BEWARE: the next line can lose top-level qualifiers.  This only   */
/* really affects volatile (const done in ensurelvalue()), but more     */
/* seriously the code to access bitfields needs review for volatile.    */
    ta = prunetype(ta);
    if (h0_(a) != s_dot || h0_(ta) != s_typespec)
    {   syserr(syserr_bitfieldassign);
        return errornode;  /* note the curious a/ta spread of info */
    }
    m = typespecmap_(ta);
    bitword = bf_container(a, m);
/* @@@ AM: the expression (x.b = 1) where x.b has bitfield type here    */
/* has promoted type of x.b, maybe it shouldn't (e.g. sizeof).          */
    if (issimplelvalue(a))
        r = bitfieldvalue(a, m, bitfieldstuff(op, a, m, bitword, b));
    else
/* te_int is dubious on next line, but bitfieldvalue adds right type.   */
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
       is voided.  We rely on the form of r (and hence bitfieldvalue).  */
    return mkinvisible(type_(r), mk_expr2(op, ta, a, b), r);
}


/* mkincdec, mkaddr and mkassign deal with monadic and diadic operators which
   require an lvalue operand (this is assumed already checked).  Many of these
   may turn into the pseudo-operator s_let to reduce CG complexity.
   Consider nasties like "int *f(); double *g(); *f() /= *g();"
*/

/* mkincdec and mkaddr never insert s_invisible nodes -- they leave the */
/* caller to do it -- is this right in retrospect?                      */
static Expr *mkincdec(AEop op, Expr *a)              /* op can be ++ -- */
{   if (h0_(a) == s_error) return errornode;
    {   /* type check enough here for correct error msgs or extra param */
        Expr *b = mkintconst(te_int, (incdecisinc_(op) ? 1 : -1), 0);
/* The following test avoids warnings for "u--" where u is unsigned int */
/* and where -1 should be the bit pattern 0xffff.                       */
/* @@@ Reconsider the use of "--x" ==> "x=x+(-1) in the light of this.  */
        if (sizeof_int == 2 && !incdecisinc_(op))
        {   TypeExpr *t = princtype(typeofexpr(a));
            if (h0_(t) == s_typespec)
            {   SET_BITMAP m = typespecmap_(t);
                if ((m & (bitoftype_(s_int)|bitoftype_(s_unsigned)|
                                            bitoftype_(s_long))) ==
                         (bitoftype_(s_int)|bitoftype_(s_unsigned)) ||
                    (m & (bitoftype_(s_char)|bitoftype_(s_unsigned))) ==
                         (bitoftype_(s_char)|bitoftype_(s_unsigned)) &&
                       feature & FEATURE_PCC)
                  b = mkintconst(te_uint, 0xffff, 0);
            }
        }
        if (incdecispre_(op))   /* turn ++x to x+=1 */
            return mkassign(s_plusequal, a, b);
        else return mkassign(s_postinc, a, b);
             /* let mkassign() do it - note that it 'knows' about s_postinc */
    }
}

/* Add extra arg 'implicit' to mkaddr to unify code with coerceunary?   */
static Expr *mkaddr(Expr *a)
{   TypeExpr *t, *tp;
    if (h0_(a) == s_error) return errornode;
    t = typeofexpr(a);
    tp = princtype(t);
#ifdef CPLUSPLUS
    switch (h0_(tp))
    {
case t_coloncolon:
        {   TypeExpr *classtype = tagbindtype_(typespectagbind_(tp));
            Expr *nullptr = mkintconst(ptrtotype_(classtype), 0, 0);
            if (h0_(a) != s_member) syserr("mkaddr(::)");
/* this is really just offsetof!                                        */
/* Note that the curious def of &D::b [ES, pxxx] being &B::b            */
/* necessary because of virtual bases.                                  */
            a = mkaddr(mkfieldselector(s_arrow, nullptr,
                                       memsv_((ClassMember *)a), 0));
            return h0_(a) == s_error ? errornode :
                mk_expr2_ptr(s_plus, ptrtotype_(t), a,
                             mkintconst(te_int, 1, 0));
        }
case t_ovld:
        return mkaddr((Expr *)ovld_resolve_addr(a, typeovldlist_(tp)));
case t_fnap:
        {   Expr *aa = skip_invisible(a);
            if (h0_(aa) == s_dot)       /* CPLUSPLUS member function    */
                return mkaddr(exprdotmemfn_(aa));
            if (h0_(aa) == s_dotstar)   /* see [ES, p71]                */
            {   cc_err("non-call site '.*' or '->*' yielding function");
                return errornode;
            }
        }
        if (h0_(a) == s_binder && bindstg_((Binder *)a) & b_pseudonym)
        {   Binder *b = realbinder_((Binder *)a);
            if (bindstg_(b) & b_memfna)
                t = (TypeExpr *)syn_list3(t_coloncolon, t,
                    parentclass((Binder *)b));
            a = (Expr *)b;
        }
        /* drop through */
    }
#endif
/* The following line handles the big kludge whereby pcc treats &a as    */
/* &a[0].  Note that then we lie somewhat about the type as given, say,  */
/* int a[5]; f(&a); we have a node &a with type (int *) but whose son    */
/* is (int [5]).  Still this does not worry optimise, the only code to   */
/* look inside an & node.                                                */
    if (feature & FEATURE_PCC && h0_(tp) == t_subscript) t = typearg_(tp);

    /* The next line allows the offsetof() macro to reduce to a compile- */
    /* time constant as ANSI require.  @@@ Think a little more --        */
    /* AM would like to keep all reductions together in trydiadreduce,   */
    /* but this means that re-arrangements have to be spotted early.     */
    /* This is the classical phase-order problem recurring.              */
/* There are also ANSI-dubious things here about WHICH constant exprs.   */
/* are allowed in various contexts.   (e.g. array size, initialiser...)  */
    {   int32 n = 0;
        Expr *q, *p = a;
        for (; p != NULL && (p = ignore_invisible(p)) != NULL; p = arg1_(p))
        {   if (h0_(p) == s_dot) {
                n += exprdotoff_(p);
                if (!(h0_(arg1_(p)) == s_content &&
                     h0_(arg1_(arg1_(p))) == s_integer))
                    continue;
                p = arg1_(arg1_(p));
            } else if (h0_(p) == s_content && h0_(arg1_(p)) == s_plus &&
                       h0_(q = ignore_invisible(arg1_(arg1_(p)))) == s_integer &&
                       h0_(arg2_(arg1_(p))) == s_integer) {
                n += intval_(arg2_(arg1_(p)));
                p = q;
            } else
                break;

            return mk_expr2_ptr(s_plus, ptrtotype_(t), p,
                                        mkintconst(te_int, n, 0));
        }
    }
/* note that it has already been verified that a is an l-value if that is */
/* important.                                                             */
    return mk_expr1(s_addrof, ptrtotype_(t), a);
}

static Expr *mkopassign(AEop op, Expr *asimple, Expr *b)
{   /* asimple must have passed issimplelvalue() or be otherwise reevalable */
/* @@@ review s_postinc the light of clumsy code for sizeof_int == 2.       */
    return mkassign(op == s_postinc ? s_displace : s_assign,
       asimple,
       mkbinary(op == s_postinc ? s_plus : unassignop_(op), asimple, b));
}

Expr *mkassign(AEop op, Expr *a, Expr *b)
                    /* for = += *= ... also s_displace and s_postinc */
{   TypeExpr *t1;
    Expr *r;
    if (h0_(a) == s_error || h0_(b) == s_error) return errornode;
    check_index_dereference(a);
    t1 = typeofexpr(a);  /* un-coerced type */
#ifdef CPLUSPLUS
/* @@@ not clear this is in the right place -- ensurelvalue/mkassign  */
    if (h0_(t1) == t_coloncolon)
        {   a = thisify(a);
            t1 = typeofexpr(a);
        }
#endif
    if (isbitfield_type(t1))
    {   TypeExpr *t2 = unbitfield_type(t1);
/* @@@ Beware: the next cast to t2 may worsen code for char bitfields,  */
/* but it is more correct than using te_int (e.g. warnings/enums).      */
/* regalloc.c now optimises char casts followed by masks.               */
/* However this doesn't happen yet (see the ONE_DAY_SOON test above).   */
        Expr *bb = mkcast(op, b, t2);
        a = skip_invisible(a);
        if (h0_(bb) == s_integer)
            return bitfieldassign(op, a, t1, bb);
        else
        {   Binder *gen = gentempbinder(te_int);  /* bf_promotedtype?    */
/* The let statement here is so that any possible side-effects in the    */
/* rhs can not cause confusion with the expansion of the bitfield        */
/* assignment into shifts and masks. Without it x.a = x.b = nn can give  */
/* trouble as the integer holding x can be loaded for the outer          */
/* assignment before it is updated by the inner one.                     */
            Expr *e = mkbinary(s_comma,
                          mkassign(s_assign, (Expr *)gen, bb),
                          bitfieldassign(op, a, t1, (Expr *)gen));
            r = mklet(gen, typeofexpr(e), e);
        }
    }
    else if (op == s_assign || op == s_displace)
        b = mkcast(op, b, t1),
        r = h0_(b) == s_error ? errornode : mk_expr2(op, t1, a, b);
    else if (issimplelvalue_i(a, NO))
        r = mkopassign(op, a, b);
    else if (issimplelvalue_i(a, YES))
    {   Expr *inc = NULL;
        Binder *gen = gentempbinder(t1);
        a = RemovePostincs(a, &inc);
        r = mklet(gen, t1,
            mkbinary(s_comma,
                mkbinary(s_comma,
                    mkassign(s_assign, (Expr *)gen, mkopassign(op, a, b)),
                    inc),
                (Expr *)gen));
    }
    else
    {   /* otherwise use a pointer in case side-effects - e.g. *f()+=1. */
        Binder *gen = gentempbinder(ptrtotype_(t1));
        r = mklet(gen, t1,
            mkbinary(s_comma,
                mkassign(s_assign, (Expr *)gen, mkaddr(a)),
                mkopassign(op, mkunary(s_content, (Expr *)gen), b)));
    }
    return r == errornode ? r:
                            mkinvisible(type_(r),
                                        mk_expr2(op, type_(r), a, b),
                                        r);
}

/* mkindex copes with indexing on a machine in which sizeof int and     */
/* and sizeof long differ.  Then we need a widen/narrow before index.   */
/* The last two arguments provide for (constant) index checking.        */
static Expr *mkindex(Expr *e, int32 stride, Expr *array, int posneg)
{   /* The code is marginally perverse here so that no tests are        */
    /* generated if sizeof_ptr/int/long are equal.                      */
    if (sizeof_ptr == sizeof_int || sizeof_ptr == sizeof_long
                                 || sizeof_ptr == sizeof_longlong)
    {   int32 n = sizeoftype(typeofexpr(e));
        if (sizeof_ptr != sizeof_int && n == sizeof_int)
            e = coerce2(e, te_lint);
        if (sizeof_ptr != sizeof_long && n == sizeof_long)
            e = coerce2(e, te_int);
    }
    else
        syserr(syserr_mkindex);
    check_index_overflow(array, e, posneg, 0);
    return mkbinary(s_times, e,
        mkintconst((sizeof_ptr==sizeof_int ? te_int:te_lint),stride,0));
}


Expr *mkbinary(AEop op, Expr *a, Expr *b)
{   TypeExpr *t1,*t2,*t3;
    if (diadneedslvalue_(op))
    {   Expr *c = mkassign(op,ensurelvalue(a,op),b);
        /* add an invisible node if expression was transformed (for errors) */
        return (h0_(c) == s_error) ? errornode :
               (h0_(c) == op) ? c : (t1 = typeofexpr(c),
                       mkinvisible(t1, mk_expr2(op,t1,a,b), c));
    }
    if (h0_(a) == s_error || h0_(b) == s_error) return errornode;
    a = coerceunary(a);
    b = coerceunary_2(b, (op==s_comma ? COERCE_COMMA : COERCE_NORMAL));
    /* the next line checks and inserts !=0 in && ||.                 */
#define isboolean_(op) ((op)==s_andand || (op)==s_oror)
    if (isboolean_(op))
    {   a = mktest(op,a);
        b = mktest(op,b);
        if (h0_(a) == s_error || h0_(b) == s_error) return errornode;
    }
    t1 = typeofexpr(a);
    t2 = typeofexpr(b);
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
                /* note that mkbinary() may permute (a,b)               */
                return mkunary(s_content, mkbinary(s_plus, a, b));
            /* drop through */
            typeclash(op);
            return errornode;
#ifdef CPLUSPLUS
        case s_arrowstar:
            /* map x->*e to (*x).*e, i.e. fixup a,t1 and drop through.  */
            if (!indexable(t1)) goto clash;
            t1 = indexee(t1);
            a = mk_expr1(s_content, t1, a);
            /* drop through */
        case s_dotstar:
/* Here f().*e gives the same complications as in f().mem -- we want    */
/* to select a field from a class which does not have an lvalue.        */
/* Moreover, we don't wish to disturb the parse tree too much while     */
/* we might still give errors.  Hence leave s_dotstar for optimise()    */
/* to remove since optimise() now ensures that such structs do get an   */
/* l-value (beware 1-word ones!).  Note that nowadays optimise should   */
/* also transform s_dot to s_content too!  Finally, we have to remember */
/* to check the use of s_dotstar on a 1-word structure in simplify.c.   */
/* @@@ beware this code is (obviously) very similar to findfield UNIFY! */
            {   TypeExpr *t1c = princtype(t1), *t2e = princtype(t2);
                SET_BITMAP q = qualifiersoftype(t1);
                if (!(isclasstype_(t1c) && h0_(t2e) == t_content)) goto clash;
                t2e = typearg_(t2e);
/* Do we need prunetype(t2e) here?  Can one "typedef A::p, A::*q?;"     */
                if (h0_(t2e) == t_coloncolon &&
                        typespectagbind_(t2e) == typespectagbind_(t1c))
                {   TypeExpr *tr = mkqualifiedtype(typearg_(t2e), q);
/* If second arg is a constant we should diadreduce to s_dot??          */
                    return mk_expr2(s_dotstar, tr, a, b);
                }
            }
clash:      typeclash(op);
            return errornode;
#endif
        case s_plus:
            if (indexable(t1) && indexer(t2))
                return mk_expr2_ptr(op, t1, a,
                             mkindex(b, strideofindexee(t1), a,1));
            if (indexable(t2) && indexer(t1))
#ifdef OLD_POINTER_PLUS
                return mk_expr2_ptr(op, t2,
                             mkindex(a, strideofindexee(t2), b,1),
                             b);
#else
/* int+ptr is now treated as ptr+int, hence i[p], p[i] as *(p+i).      */
                return mk_expr2_ptr(op, t2, b,
                             mkindex(a, strideofindexee(t2), b,1));
#endif
            break;
        case s_minus:
            if (indexable(t1) && indexer(t2))
                return mk_expr2_ptr(op, t1, a,
                             mkindex(b, strideofindexee(t1), a,-1));
            if (indexable(t1) && indexable(t2))
            {   /* Ignore top-level qualifiers as vals are rvalues.     */
                /* Also ignore (by unioning) qualifiers on pointed-to   */
                /* types.  Use compositetype() to establish stride.     */
/* @@@ ANSI is ambiguous on f(int (*a)[], int (*b)[3]) { return a-b; }  */
                TypeExpr *t3 = qualunion_compositetype(indexee(t1),
                                                       indexee(t2));
                if (t3)
                {   /* The following line is the only use of s_ptrdiff   */
                    /* which is only serves to stop check_narrow_subterm */
                    /* from issuing spurious warnings on                 */
                    /*         long x = offsetof(..)                     */
                    Expr *actual = mk_expr2(s_ptrdiff, te_int, a, b);
                    Expr *r = mk_expr2_ptr(op, te_int, a, b);
                    int32 sizeof_t3 = sizeoftype(t3);
                    /* the following helps the offsetof() macro used in */
                    /* static initialisers...                           */
                    if (sizeof_t3 != 1)
                        r = mkbinary(s_div, r,
                                    mkintconst(te_int, sizeof_t3, 0));
                    if (sizeof_ptr != sizeof_int &&
                        sizeof_ptr != sizeof_longlong)
                        /* Fix this more seriously one day?             */
                        /* If so then review te_int in term for 'r'.    */
                        syserr(syserr_ptrdiff);
                    if (h0_(r) == s_error) return errornode;
                    /* The invisible node is for correct error msgs */
                    return mkinvisible(te_int, actual, r);
                }
            }
            break;
        default:
            if (isrelational_(op))
            {   /* unify code with mkcond()... */
                if (indexable(t1) && indexable(t2))
                {   /* ignore top-level qualifiers as vals are rvalues  */
                    TypeExpr *t1a = indexee(t1);
                    TypeExpr *t2a = indexee(t2);
/* Now ignore qualifiers on pointed-to types, but see                   */
/* qualfree_equivtype() for a warning.                                  */
                    if (!qualfree_equivtype(t1a,t2a))
                    {   /* looks like a type error, but first check */
                        /* if equality op and one arg is (void *)   */
/* Note that (void *) types are tricky -- the null pointer constant     */
/* (void *)0 is comparable with a fn, but no other (void *)values are.  */
                        if (!((op == s_equalequal || op == s_notequal) &&
                              (isvoidtype(t1a) &&
                                  (!isfntype(t2a) || isnullptrconst(a))) ||
                              (isvoidtype(t2a) &&
                                  (!isfntype(t1a) || isnullptrconst(b)))))
                        {   if (suppress & D_MPWCOMPATIBLE)
                                cc_warn(sem_rerr_different_pointers, op);
                            else
                                cc_pccwarn(sem_rerr_different_pointers, op);
                        }
                    }
                    return mk_expr2_ptr(op, te_boolean, a, b);
                }
/* The following lines handle the case NULL == 0 (of integral type).    */
                if (indexable(t1) && indexer(t2))
                    return mk_expr2_ptr(op, te_boolean, a, pointerofint(b,op,t1));
                if (indexable(t2) && indexer(t1))
                    return mk_expr2_ptr(op, te_boolean, pointerofint(a,op,t2), b);
            }
            break;
    }
    /* all that SHOULD be left are the arithmetic types */
    t3 = lubtype(op,t1,t2);           /* BEWARE - see trydiadreduce below */
    if (t3 == 0) return errornode;
    if (op == s_leftshift || op == s_rightshift);   /* done by lubtype() */
    else a = coerce2(a, t3), b = coerce2(b, t3);
    {
#ifdef PASCAL /*ECN*/
        Expr *c = mk_expr2(op, ((isrelational_(op) || isboolean_(op)) ?
                                                       te_boolean : t3), a, b);
#else
        Expr *c = mk_expr2(op, (isrelational_(op) ?  te_int : t3), a, b);
#endif
        /* the next line relies on the form of result of lubtype() (q.v.) */
        return trydiadreduce(c, typespecmap_(t3));
    }
}

Expr *mkfnap(Expr *e, ExprList *l)
{   TypeExpr *t;
    ExprList *ll;
    FormTypeList *d;
    int32 len;
    int curried = 0;
#ifdef CPLUSPLUS
    TypeExpr *tt;
#endif
    if (h0_(e) == s_error) return errornode;
#ifdef CPLUSPLUS
/* Rationale: if we have a s_dot or s_dotstar (s_arrow/s_arrowstar      */
/* have already gone) which gives a fn value (impossible in C)          */
/* then we have to add its LHS as a *potential* first arg for overload  */
/* resolution.  Potential because it only applies to non-static memfns. */
/* We need to do this before coerceunary() can moan about unresolvable  */
/* overloadings (note this is OK because no refs to fns).               */
    ll = l;
    {   Expr *ee = skip_invisible(e);
        if (h0_(ee) == s_dot && h0_(type_(ee)) == t_ovld)
        {   /* C++ member function                                      */
            ll = mkExprList(l, mkunary(s_addrof, arg1_(ee)));
            e = exprdotmemfn_(ee);      /* s_binder of (generic) memfn. */
            /* make an s_invisible node?                                */
        }
        if (h0_(ee) == s_dotstar)       /* see [ES, p71]                */
        {   l = ll = mkExprList(l, mkunary(s_addrof, arg1_(ee)));
            e = arg2_(ee);
            curried = 1;
        }
    }
/* The placing of the next line is tentative --- we should really       */
/* do the pre-overloading-resolution coercions on args (things like     */
/* array->pointer but not char->int) before it...                       */
/* Also, given "ovldfn(1)" valid, is "(*ovldfn)(1)" or "(&ovldfn)(1)?   */
    if (h0_(e) == s_binder)
    {   Binder *b = (Binder *)e;
        TypeExpr *bt = princtype(bindtype_(b));
        if (h0_(bt) == t_ovld)
        {   b = ovld_resolve(b, typeovldlist_(bt), l, ll);
            if (b == 0) return errornode;
/* here we should check access rights to the member before indirecting. */
/* and pseudonym should not be used (so A::f() works).                  */
            if (bindstg_(b) & b_pseudonym) b = realbinder_(b);
            binduses_(b) |= u_referenced;
            if ((bindstg_(b) & STGBITS) == 0)
                syserr("mkfnap(bindstg %lx)\n", bindstg_(b));
            if (bindstg_(b) & b_memfna)
                l = ll;         /* implicit first arg for 'this' */
            e = (Expr *)b;
        }
        if (bindstg_(b) & bitofstg_(s_virtual))
        {   Expr *lv = mkdotable(s_arrow, exprcar_(l));
            TagBinder *tb = typespectagbind_(princtype(type_(lv)));
            Expr *vtab = rooted_path(path_to_member(tb, 0, vtabsym, tb), lv);
            e = mk_expr1(s_cast, ptrtotype_(bindtype_(b)),
                    mk_expr2(s_plus, type_(vtab), vtab,
                        mkintconst(te_int, bindxx_(b)*12, 0)));
            e = mkinvisible(type_(e), (Expr *)b, e);
* BEWARE: 12 is a magic number; FIXME */
        }
        bindstg_(b) &= ~b_maybeinline;
    }
#endif
    e = coerceunary(e);
    t = typeofexpr(e);
#ifdef CPLUSPLUS
    if (curried && h0_(t = princtype(t)) == t_content
                && h0_(tt = princtype(typearg_(t))) == t_coloncolon
                && h0_(t = princtype(typearg_(tt))) == t_fnap)
        d = (FormTypeList *)syn_list3(typefnargs_(t), 0,
                ptrtotype_(tagbindtype_(typespectagbind_(tt))));
    else
#endif
    if (!curried &&
        indexable(t) && (t = princtype(indexee(t)), h0_(t) == t_fnap))
        d = typefnargs_(t);
    else
    {   cc_err(sem_err_nonfunction);
        return errornode;
    }
    for (ll = l; ll != 0; ll = cdr_(ll))
    {   Expr *elt = exprcar_(ll);
        TypeExpr *formtype = 0;
        /* avoid other error messages due to wrong no of args. */
        if (h0_(elt) == s_error) return errornode;   /* e.g. syntax error */
        /*  float and char args:  consider the following.  It is quite legal
            (according to ANSI May 86 draft) to say "f(x) float x; {...}"
            in one file, and to call it from another with f(1.2).
            In such circumstances it is required to pass 1.2 (even 1.2f)
            as 'double'.  Hence for consistency we pass ALL float args as
            double, even though the prototype form "f(float x){...}" does not
            technically need it.  Discuss with AM if in doubt.
            We call this 'callee narrowing'.  Similarly for 'char' formals:
            we widen to int before the call, do not narrow (= mask) before
            call, but as first instructions of callee.
        */
        if (d)  /* type of arg visible: prototype or previous K&R defn.  */
        {   if (!typefnaux_(t).oldstyle)
            {   /* prototype parameter */
                Expr *x = elt;
                TypeExpr *t2 = princtype(d->fttype);

#ifdef PASCAL
                if ( h0_(d->fttype) != s_content ||
                     !is_conformant(typearg_(d->fttype))) {
                    if (h0_(d->fttype->pun.type) == s_var)
                        x = mkunary(s_addrof, x);
                    if (h0_(d->fttype->pun.type) == s_addrof) {
                        Binder *b;

                        block_binders = syn_list2(block_binders,
                            b = inst_decl(gensymval(1), typearg_(d->fttype),
                                                            bitofstg_(s_auto), 0));
                        x = mkbinary(s_comma,
                                mk_pas_assign(s_assign, (Expr *)b, x),
                                mkunary(s_addrof, (Expr *)b));
                    }
                }
#endif
/* The next ten lines avoid a spurious warning when (float)+(float)     */
/* is passed to a fn with a prototype 'float' parameter.                */
/* They reflect the above Codemist decision to implement 'float'        */
/* prototype parameters as 'double' to aid old code transition to ansi. */
                if (h0_(t2) == s_typespec  &&  (typespecmap_(t2) &
                        (bitoftype_(s_double)|bitoftype_(s_short))) ==
                            (bitoftype_(s_double)|bitoftype_(s_short)))
                {   TypeExpr *t1 = princtype(typeofexpr(x));
                    if (h0_(t1) == s_typespec  &&  (typespecmap_(t1) &
                        (bitoftype_(s_double)|bitoftype_(s_short))) ==
                            (bitoftype_(s_double)|bitoftype_(s_short)))
                        x = mkcast(s_cast, x, te_float);
                }
/* end of warning-avoiding code.                                        */
                x = mkcast(s_fnap, x, widen_formaltype(d->fttype));
                if (h0_(x) != s_error)
                {   exprcar_(ll) = x;
                    d = d->ftcdr;
                    continue;
                }
            }
            else /* K&R style defn in scope: check if requested below.   */
                formtype = d->fttype, d = d->ftcdr;
        }
        /* unchecked parameter or K&R (olde) style fn defn in scope.     */
        if (isvoidtype(typeofexpr(elt)))  /* includes "const void" etc. */
            {   cc_err(sem_err_void_argument);
                return errornode;
            }
        /* unchecked or olde-style paramaters get a special coercion,   */
        /* which includes converting float to double.                   */
        exprcar_(ll) = coerceunary_2(elt, COERCE_ARG);
        if (formtype &&
            (feature & (FEATURE_PCC|FEATURE_FUSSY)) != FEATURE_PCC &&
            !qualfree_equivtype(widen_formaltype(formtype),
                                typeofexpr(exprcar_(ll))))
                cc_warn(sem_warn_olde_mismatch, e);
/* if TARGET_NULL_BITPATTERN != 0 and actual is (int)0 then warn if ...  */
/* ... FEATURE_PREDECLARE or some such.                                  */
    }
    /* we have to discount the addition to 'l' if curried (only).        */
    /* perhaps all non-static memfns should be seen as curried?          */
    len = length(l) - curried;
    if (debugging(DEBUG_TYPE))
        cc_msg("fn $e(%ld..%ld) gets %ld args\n", e,
            (long)minargs_(t), (long)maxargs_(t), (long)len);
    if (!(minargs_(t) <= len && len <= maxargs_(t)))
    {
        if (typefnaux_(t).oldstyle)
        {   if ((feature & (FEATURE_PCC|FEATURE_FUSSY)) != FEATURE_PCC)
                cc_warn(sem_rerr_wrong_no_args, e);
        }
        else
            cc_rerr(sem_rerr_wrong_no_args, e);
    }
    if (!curried && fntypeisvariadic(t) && typefnaux_(t).variad > 0)
        /* ho-hum, lets see if there's an illegal printf/scanf! */
        formatcheck(e, l, typefnargs_(t), typefnaux_(t).variad);
    return mk_expr2(s_fnap, typearg_(t), e, (Expr *)l);
}

static bool implicitpointercast(TypeExpr *y, TypeExpr *x)
{
/* Here we have an implicit cast into a pointer to (nonpruned) type x   */
/* from a pointer to (nonpruned) type y.  Return true iff OK.           */
/* The cases where pointers to y and x are equivtype have already been  */
/* handled, leaving only the conversion to more qualified type.         */
/* However, observe also that x may be a void type and y an array type. */
/* The Oct 88 draft clarifies that void pointers are are required       */
/* to respect const/volatile qualification too.                         */
    SET_BITMAP my = qualifiersoftype(y), mx = qualifiersoftype(x);
    if (my & ~mx & (bitoftype_(s_const)|bitoftype_(s_volatile)))
        return 0;       /* forbidden implicit cast losing qualifiers.   */
    if (qualfree_equivtype(x, y)) return 1;
    if (isvoidtype(x)) return 1;
#ifndef CPLUSPLUS
    if (isvoidtype(y))
    {   /* illegal in C++, so (later optionally) warn in ANSI C         */
        cc_ansi_warn(sem_warn_narrow_voidstar);
        return 1;
    }
#endif
    return 0;
}

static void check_narrowing_cast(AEop op, SET_BITMAP from, SET_BITMAP to)
{   if (op == s_cast); /* all OK */
    else if (to & bitoftype_(s_enum))
    {   /* A cast from a type to itself cannot come here, so this is   */
        /* implicit cast from arith type to a different enum type.     */
#ifdef CPLUSPLUS
        cc_rerr(sem_rerr_casttoenum, op, from);
#else
        /* what does PCC mode want here?  Warn like most PCC's         */
        cc_warn(sem_rerr_casttoenum, op, from); /* ANSI C says OK.     */
#endif
    }
    else
      if (/* moan at long int -> int/char */
          ((to & (bitoftype_(s_long)|bitoftype_(s_int)))
               == bitoftype_(s_int) ||
           (to & bitoftype_(s_char)) != 0) &&
             (from & (bitoftype_(s_long)|bitoftype_(s_int)))
                == (bitoftype_(s_long)|bitoftype_(s_int))
         ||
          /* any floating type to any shorter floating type */
          (to & bitoftype_(s_double)) &&
            (from & bitoftype_(s_double)) &&
            (to & ~from & bitoftype_(s_short) ||
             from & ~to & bitoftype_(s_long))
         ||
          /* any floating type to any integral type */
          (to & INTEGRALTYPEBITS &&
            (from & bitoftype_(s_double)))
          /* any other ideas at what to moan at? */
         )
    {   if (suppress & D_IMPLICITNARROWING) xwarncount++;
        else cc_warn(sem_warn_narrowing, op);
    }
}

/* tidy up relationship with coerce2() */
Expr *mkcast(AEop op, Expr *e, TypeExpr *tr)
{   TypeExpr *te, *x, *y, *xp, *yp;
    Expr *r;
    SET_BITMAP m;
    if (h0_(e) == s_error) return errornode;
#ifdef CPLUSPLUS
    if (x = princtype(tr), h0_(x) == t_ref)
    {   /* don't coerceunary yet, e.g. int a[5], (&x)[5] = a.           */
        /* are refs to fns/arrays allowed?  Unwritable!                 */
        /* but should we ref->content?                                  */
        te = typeofexpr(e); y = princtype(te);
        /* @@@think about qualifiers here                               */
/* Rationale: explicit casts to refs share an lvalue, but implicit      */
/* ones do not unless types match.  See [ES, p69, p155].  Whee(p).      */
        if (islvalue(e) &&
              (equivtype(typearg_(x), h0_(y) == t_ref ? typearg_(y) : te) ||
               op == s_cast))
            e = mkunary(s_addrof,e);
        else
            {   /* make a temp, save it on a list and use it as 'e'.    */
                Binder *b = gentempbinder(typearg_(x));
                sem_reftemps = mkSynBindList(sem_reftemps, b);
                if (!(qualifiersoftype(typearg_(y)) & bitoftype_(s_const)))
                    cc_rerr(sem_rerr_valcasttoref, op);
                /* @@@ explicit/implicit casts? */
                /* beware refs to fns/arrays next line will syserr!       */
                e = mkbinary(s_comma, mkassign(s_assign, (Expr *)b, e),
                                      mkunary(s_addrof, (Expr *)b));
            }
        return mk_expr1(s_cast, tr, e);
    }
#endif
    e = coerceunary_2(e, COERCE_ASSIGN);        /* i.e. not enum->int   */
    te = typeofexpr(e);
    /* check te,tr match enough ... */
    switch (qualfree_equivtype(te,tr))
    {
case 2: if (op != s_cast)
            /* if op != s_cast and te and tr are IDENTICAL types
               we needn't store the cast.
            */
            return e;
        /* drop through */
case 1: /* permissible cast */
        break;
case 0: /* not obviously permissible, check more */
        x = princtype(tr), y = princtype(te);   /* lose quals as rvalues */
#ifdef TARGET_IS_ARM          /* LDS: for now, until policy is agreed */
        if (op != s_cast || feature & FEATURE_ANOMALY)
#endif
            (void)check_narrow_subterm(e, y, x);
#ifdef CPLUSPLUS
        if (isclasstype_(y))
        {   Symstr *sv = class_has_conversion(typespectagbind_(y), x);
            if (sv) return mkfnap(mkfieldselector(s_dot, e, sv, 0), 0);
        }
#endif
        switch (h0_(x))
        {
    case t_content:                                 /* cast to pointer. */
            if (isnullptrconst(e));                        /* always ok */
            else if (xp = princtype(x = typearg_(x)),
                     h0_(y) == t_content || h0_(xp) == t_fnap)
            {   /* cast from ptr type or any cast to fn ptr...          */
/* AM finds the dataflow hard to follow here...                         */
                if (h0_(y) == t_content)
                    yp = princtype(y = typearg_(y));
                else
                    yp = y;
                if ((h0_(xp) == t_fnap) != (h0_(yp) == t_fnap))
                {   if (op != s_cast)
                    {   /* 2 calls to same routine again...          */
                        if (suppress & D_MPWCOMPATIBLE)
                            cc_warn(sem_warn_fn_cast, op);
                        else
                            cc_pccwarn(sem_warn_fn_cast, op);
                    }
                    else if (!(feature & (FEATURE_PCC|FEATURE_LIMITED_PCC) ||
                               suppress & D_IMPLICITCAST))
                        cc_warn(sem_warn_fn_cast, op);
/* AM: the following reflects a bug (fix) in the interface aetree->cg.     */
/* There is danger here if a local static array, or address of a local var */
/* is being cast to a fn ptr type - the Expr tree will contain a structure */
/* like (addrof (binder name)) or (binder name) with type t_subscript ...  */
/* which cg.c will interpret as a literal name of a function, fatally...   */
/* (Indeed such a thing MUST NOT become external for fear of name clashes) */
/* To fool cg.c into generating a J_CALLR with an appropriate expression,  */
/* we turn such structures into (plus (addrof (binder name)) 0). The + 0   */
/* gets optimised out, so this costs nothing in the generated code.        */
/* N.B. Doing this so calls to extern arrays still get done via  J_CALLK   */
/* is tricky... it's not clear that it's worth it...                       */
                    if (h0_(xp) == t_fnap &&
                        (h0_(e) == s_cast || h0_(princtype(te)) == t_content))
                    {   /* possible cast of dangerous expr to fn type... */
                        /* ...unless straight cast to extern array...    */
                        if (h0_(e) != s_invisible ||
                            h0_(r = arg1_(e)) != s_binder ||
                            !(bindstg_((Binder *)r) & bitoftype_(s_extern)))
                            e = mk_expr2(s_plus, te, e, globalize_int(0));
                    }
                }
                else if (op != s_cast && !implicitpointercast(y,x))
                    if (!(suppress & D_IMPLICITCAST))
                        cc_pccwarn(sem_rerr_implicit_cast1, op);
            }
            else if (isprimtype_(y,s_int))
            {   if (op != s_cast)
                    if (!(suppress & D_IMPLICITCAST))
                        cc_pccwarn(sem_rerr_implicit_cast2, op);
            }
            else
            {   cc_err(sem_err_bad_cast, op, y);
                return errornode;
            }
            break;
    case s_typespec:
            m = typespecmap_(x);
            switch (m & -m)    /* LSB - unsigned/long etc. are higher */
            {   case bitoftype_(s_int):
                    if (h0_(y) == t_content && !int_isshort_(m))
                    {   /* the only valid coercion from pointer */
                        if (op == s_cast)
                        { if (feature & FEATURE_TELL_PTRINT)
                            /* police pointer purity */
                            cc_warn(sem_warn_pointer_int);
                        }
                        else
                            if (!(suppress & D_IMPLICITCAST))
                                cc_pccwarn(sem_rerr_implicit_cast3, op);
                        break;
                    }
                    /* there may be other similar ops to moan about */
                    if (op == s_switch && isprimtype_(y,s_double))
                    {
                        cc_pccwarn(sem_rerr_implicit_cast4,op,y);
                        break;
                    }
                    /* drop through */
                case bitoftype_(s_char):
                case bitoftype_(s_enum):
                case bitoftype_(s_double):
                    if (h0_(y) != s_typespec ||
                        !(typespecmap_(y) & ARITHTYPEBITS))
                    {
/* This recovery is by and large OK for all Codemist back-ends.           */
/* @@@ AM However, it currently kills (syserr()) cg.c if just one of   */
/* x and y is an integer-like struct.  Fix.                               */
/* Also consider making a fn out of the following to remove               */
/* near duplication of code in the next case, but only if the text of     */
/* the ...cast1 and ...cast2 messages can be unified.                     */
/* @@@ relaxation guarded by FEATURE_PCC until cg.c can stop syserr()ing. */
                        if (feature & FEATURE_PCC &&
                            sizeoftype(x) == sizeoftype(y) &&
                            alignoftype(x) <= alignoftype(y))
                        {   /* same size, dest no more aligned than src */
                            cc_pccwarn(sem_err_bad_cast1, op, x);
                            break;
                        } else
                        {   cc_err(sem_err_bad_cast1, op, x);
                            return errornode;
                        }
                    }
                    check_narrowing_cast(op, typespecmap_(y), typespecmap_(x));
                    break;
                case bitoftype_(s_struct):
                case bitoftype_(s_class):
                case bitoftype_(s_union):
/* @@@ relaxation guarded by FEATURE_PCC until cg.c can stop syserr()ing. */
                    if (feature & FEATURE_PCC &&
                        sizeoftype(x) == sizeoftype(y) &&
                        alignoftype(x) <= alignoftype(y))
                        /* same size, dest no more aligned than src */
                        cc_pccwarn(sem_err_bad_cast2, op, x);
                    else
                    {   cc_err(sem_err_bad_cast2, op, x);
                        return errornode;
                    }
                    break;
                case bitoftype_(s_void):
                    /* Warn for casts unless explicit or already voided */
                    /* Macro:s like getc() or pop() in void context     */
                    /* need explicit casts to suppress warnings.        */
                    if (op != s_cast && !isprimtype_(y,s_void))
                        e = mkvoided(e);
                    break;
                default:
                    syserr(syserr_mkcast,
                           (long)h0_(x), (long)typespecmap_(x));
                    return errornode;
            }
            break;
    default:
            syserr(syserr_mkcast1, (long)h0_(x));
            return errornode;
    case t_fnap:
    case t_subscript:
            cc_err(sem_err_bad_cast1, op, x);
            return errornode;    /* improve */
        }
    }
    r = mk_expr1(s_cast, tr, e);
#ifdef SOFTWARE_FLOATING_POINT
    return fixflt(trycastreduce(e, tr, r, op==s_cast));
#else
    return trycastreduce(e, tr, r, op==s_cast);
#endif
}

Expr *findfield(TypeExpr *te, Symstr *sv, TagBinder *start_scope)
{   Expr *path;
    TagBinder *b;
    TypeExpr *t = princtype(te);
    if (!isclasstype_(t))
    {   /* typeclash(op); */
        cc_err("illegal left operand to '->' or '.'");
        return errornode;
    }
    b = typespectagbind_(t);
    if (!(attributes_(b) & TB_DEFD))
    {   cc_err(sem_err_undef_struct, b);
        return errornode;
    }
    path = path_to_member(b, qualifiersoftype(te), sv, start_scope);
    if (path == NULL)
    {   /* failed to find a real data member... */
        cc_err(sem_err_unknown_field, b, sv);
        path = errornode;
    }
    return path;
}

Expr *mkdotable(AEop op, Expr *e)
{  if (h0_(e) == s_error) return errornode;
/* The next line effectively a no-op for C but C++ can coerce values    */
/* of type (struct x &) implicitly to type (struct x).                  */
/* It needs to be done for case s_arrow anyway (e.g. array->foo).       */
    e = coerceunary(e);         /* C++ can coerce t& to t for '.' op.   */
    if (op == s_arrow)
    {   TypeExpr *te = typeofexpr(e);
        /* treat x->a as (*x).a. */
        if (indexable(te))
        {   te = indexee(te);
            return mk_expr1(s_content, te, e);
        }
    }
    else if (op == s_dot)
        return e;
    typeclash(op);
    return errornode;
}

Expr *rooted_path(Expr *path, Expr *root)
{   Expr *e = path;
    for (e = path;  (h0_(e) == s_dot || h0_(e) == s_content);  e = arg1_(e))
        if (arg1_(e) == NULL)
        {   arg1_(e) = root;
            break;
        }
    return path;
}

Expr *mkfieldselector(AEop op, Expr *e, Symstr *sv, TagBinder *start_scope)
{   e = mkdotable(op, e);
    if (h0_(e) == s_error) return errornode;
    return rooted_path(findfield(typeofexpr(e), sv, start_scope), e);
}

Expr *mkcond(Expr *a, Expr *b, Expr *c)
{   Expr *r;
    TypeExpr *t, *t1, *t2;
    if (h0_(a) == s_error) return a;
    a = coerceunary(a);
    a = mktest(s_cond, a);   /* checks type of a */
    if (h0_(a) == s_error) return a;
    if (h0_(b) == s_error) return b;
    if (h0_(c) == s_error) return c;
/* Slightly odd code for C++ cond: if b and c are same enum, then one   */
/* presumes result is enum, otherwise int.  [ES] silent, check std. @@@ */
/* ANSI C doesn't care since implicit int->enum is OK.                  */
    if (sameenum(t1 = typeofexpr(b), t2 = typeofexpr(c))) t = t1;
    else if (b = coerceunary(b), c = coerceunary(c),
             t1 = typeofexpr(b), t2 = typeofexpr(c),
/* CPLUSPLUS: the next line is a lie [ES book]. (Because Lvalue conds?) */
/* since we are in r-value context, we can ignore qualifiers:           */
             t1 = princtype(t1), t2 = princtype(t2),
      (isclasstype_(t1) && isclasstype_(t2)))
/* for C++ lvalue cond the next lines need changing too.                */
       /* Since cond's give rvalues the qualifiers on the result are    */
       /* irrelevant.  However, if you wish to support (for PCC mode)   */
       /*     "struct s { int m[10]; }; const struct s x;"              */
       /*     "volatile struct s y;  int *q = (p()?x:y).m"              */
       /* then you had better review the princtype() above and decide   */
       /* which qualifiers q needs to have.                             */
       t = qualunion_compositetype(t1,t2);
/* @@@ unify code with isrelational_() call above?                      */
    else if (indexable(t1) && indexable(t2))
    {   /* Note this includes x?(void *)0:(void *)0 which the dec 88    */
        /* ansi draft leaves (seemingly to AM) undefined.               */
        /* Note also that (x?(void *)0:(int *)z) has type (int *) but   */
        /* (x?(void *)y:(int *)z has type (void *).                     */
        /* The rationale is that nullpointer constants (0 or (void*)0)  */
        /* must behave the same, but that void * values may be wider    */
        /* that int * values (e.g. a word-addressed machine).           */
        TypeExpr *t1x = indexee(t1), *t2x = indexee(t2);
        if (isnullptrconst(b)) t = t2;
        else if (isnullptrconst(c)) t = t1;
        else if (isvoidtype(t1x) && !isfntype(t2x))
            t = ptrtotype_(mkqualifiedtype(t1x, qualifiersoftype(t2x)));
        else if (isvoidtype(t2x) && !isfntype(t1x))
            t = ptrtotype_(mkqualifiedtype(t2x, qualifiersoftype(t1x)));
        else if ((t = qualunion_compositetype(t1x,t2x)) != 0)
            t = ptrtotype_(t);
        else
            /* fix up to *some* type.                                   */
            t = t1, cc_rerr(sem_rerr_cant_balance, s_cond);
    }
    else if (indexable(t1))
        /* Similarly note x?0:(void *)0 is unclear.                     */
        c = mkcast(s_cond, c, t1), t = (h0_(c) == s_error ? 0 : t1);
    else if (indexable(t2))
        b = mkcast(s_cond, b, t2), t = (h0_(b) == s_error ? 0 : t2);
    else if (isvoidtype(t1) && isvoidtype(t2))
        /* note t1/t2 may be qualified void, but irrelevant on rvalues. */
        t = te_void;
    else t = lubtype(s_cond,t1,t2);
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
/* @@@ (See also trydiadreduce()).  It would seem that the following    */
/* code can contravene the ANSI draft in that it also reduces           */
/* int x[(1 ? 2:(0,1))] to int x[2], which silently misses a constraint */
/* violation that the comma operator shall not appear in const exprs.   */
/* Similarly x[1 ? 2 : f()].  AM thinks the ANSI draft is a mess here.  */
    return mkinvisible(t, r, intval_(a) ? b : c);
}

void sem_init(void)
{   static Expr errorexpr = { s_error };
    errornode = &errorexpr;
}

/* End of sem.c */
