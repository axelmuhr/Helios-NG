/*
 * bind.c: various binding and lexing routines for C compiler
 * Copyright (C) Codemist Ltd., 1987-1992.
 * Copyright (C) Acorn Computers Ltd., 1988-1990.
 * Copyright (C) Advanced RISC Machines Limited, 1991-1992.
 */

/*
 * RCS $Revision: 1.4 $  Codemist 44
 * Checkin $Date: 1993/11/23 17:04:33 $
 * Revising $Author: nickc $
 */

/* AM memo: @@@ the use of assert(0) in the development of PASCAL is    */
/* untidy but only temporary.                                           */

/* #define DEBUG_TENTATIVE_DEFS 1        -- BUT ONLY during development */

/* AM Dec-90: the BSS code was broken in several interesting ways.      */
/*   Mend it, and move PCC mode nearer to being ANSI code with only     */
/*   local differences.  u_tentative is now dead.                       */
/*   BSS is now merged with tentatives both in ansi and pcc mode.       */
/* AM Jul-87: experiment with memo-ising globalize_typeexpr(). */
/* exports globalize_int(), globalize_typeexpr() */

#ifndef __GNUC__
#include <stddef.h>         /* for offsetof() */
#endif
#include <string.h>
#ifdef __GNUC__
#include <stddef.h>
#endif
#include <ctype.h>
#include "globals.h"
#include "defs.h"
#include "aetree.h"
#include "util.h"           /* for padstrlen()... */
#include "codebuf.h"        /* for padstatic()... */
#include "cgdefs.h"         /* @@@ just for GAP */
#include "bind.h"
#include "builtin.h"
#include "lex.h"            /* for curlex... */
#include "sem.h"            /* for prunetype, equivtype... */
#include "store.h"
#include "vargen.h"         /* for initstaticvar()... */
#include "xrefs.h"          /* for LIT_LABEL */
#include "errors.h"
#include "aeops.h"
#ifdef TARGET_IS_HELIOS
#include "mcdep.h"
#endif

#ifdef PASCAL
/* for production qualify compilers prefer syserr() over assert().      */
#  include <assert.h>
#endif

#define EQtype_(t1,t2)   ((t1)->h0==(t2)->h0 && (t1)->typearg==(t2)->typearg \
                          && (t1)->typespecbind==(t2)->typespecbind)

static int gensymline, gensymgen;         /* For generating unique syms   */
static Symstr *(*hashvec)[BIND_HASHSIZE]; /* Symbol table buckets         */
char *sym_name_table[s_NUMSYMS];          /* translation back to strings  */

#define NO_CHAIN     256

#ifdef PASCAL
/* the following parameterisation allows PASCAL systems to use case-    */
/* insensitive name matching while preserving the original case.        */
#define lang_hashofchar(x) safe_tolower(x)
static int lang_namecmp(char *s, char *t)
{   for (;;)
    {   if (safe_tolower(*s) != safe_tolower(*t)) return 1;
        if (*s == 0) return 0;
        s++, t++;
    }
}
#else
#  define lang_hashofchar(x) (x)
#  define lang_namecmp(a, b) strcmp(a, b)
#endif

Symstr *sym_lookup(char *name, int glo)
{   int32 wsize;
    unsigned32 hash, temp;
    char *s;
    Symstr *next, **lvptr = NULL;
  /*
   * 'glo' ==  SYM_LOCAL  => allocate in Binder store
   *       ==  SYM_GLOBAL => allocate in Global store
   *  glo'  &  NO_CHAIN   => don't chain to symtab buckets
   */
    if (glo & NO_CHAIN)
        glo &= ~NO_CHAIN;
    else
    {   hash = 1;
        for (s = name; *s != 0; ++s)
        {   temp = (hash << 7);
            hash = ((hash >> 25)^(temp >> 1)^(temp >> 4) ^
                                             lang_hashofchar(*s)) & 0x7fffffff;
        }
        lvptr = &(*hashvec)[hash % BIND_HASHSIZE];
        while ((next = *lvptr) != NULL)
        {   if (lang_namecmp(symname_(next), name) == 0) return(next);
            lvptr = &symchain_(next);
        }
    }
    wsize = offsetof(Symstr, symname[0]) + padstrlen(strlen(name));
    next = glo != SYM_LOCAL ? (Symstr *) GlobAlloc(SU_Sym, wsize)
                            : (Symstr *) BindAlloc(wsize);
    memclr(next, (size_t)wsize);
    if (lvptr != NULL)
        *lvptr = next;
    else
        symchain_(next) = (Symstr *)DUFF_ADDR;
        /* @@@ nasty use of DUFF_ADDR here, not guaranteed to be non-NULL */
    symtype_(next) = s_identifier;
    strcpy(symname_(next), name);
    return(next);
}

Symstr *sym_insert(char *name, AEop type)
{   Symstr *p = sym_lookup(name, SYM_GLOBAL);
    symtype_(p) = type;
    return(p);
}

Symstr *sym_insert_id(char *name)
{   return sym_lookup(name, SYM_GLOBAL);
}

static void check_extern(Symstr *s)
{   Symstr *p;
/* First an option to check ansi conformance ... */
/* (The return below ensures only one is executed.  Because of room for  */
/* the back pointer only one can be active at once, unfortunately).      */
    if (feature & FEATURE_6CHARMONOCASE)
    {   char ch, v[6+1];
        int n = 0;
        while ((ch = symname_(s)[n]) != 0 && n < 6) v[n++] = safe_tolower(ch);
        v[n] = 0;
        p = sym_lookup(v, SYM_GLOBAL);
        if (symfold_(p) == 0)
            symfold_(p) = s;
        if (symfold_(p) != s)
            cc_warn(bind_warn_extern_clash, s, symfold_(p));
        return;
    }
/* ... now a compiled-in form for things like 370 linkers */
#ifdef TARGET_HAS_LINKER_NAME_LIMIT    /* e.g. 370 */
    {   char ch, v[LINKER_NAME_MAX+1];
        int n = 0;
        while ((ch = symname_(s)[n]) != 0 && n < LINKER_NAME_MAX)
            v[n++] = LINKER_NAME_MONOCASE ? safe_tolower(ch) : ch;
        v[n] = 0;
        p = sym_lookup(v, SYM_GLOBAL);
        if (symfold_(p) == 0)
            symfold_(p) = s;
        if (symfold_(p) != s)
            cc_err(bind_err_extern_clash,
                   s, symfold_(p), (long)LINKER_NAME_MAX,
                   LINKER_NAME_MONOCASE ? " monocase" : "");
    }
#endif /* TARGET_HAS_LINKER_NAME_LIMIT */
}

Symstr *gensymval(bool glo)
{
    /* Generates a new symbol with a unique name.                    */
    /* (Not quite unique in that line 1 may occur in 2 files, but    */
    /*  we RELY on NO_CHAIN below to treat as unique).               */
    char name[30];
#ifdef TARGET_IS_CLIPPER
    /* the next line is a hack to ensure these gensyms are assemblable */
    /* Note that this HOPEs that the user has no names Intsym_nnn      */
    sprintf(name, "Intsym_%d", ++gensymgen);
#else
    if (gensymline != curlex.fl.l)
        gensymline = curlex.fl.l, gensymgen = 1;
    else
        ++gensymgen;
    sprintf(name, "<Anon%d_at_line_%d>", gensymgen, gensymline);
#endif
    glo = (glo ? SYM_GLOBAL + NO_CHAIN : SYM_LOCAL + NO_CHAIN);
    return(sym_lookup(name, glo));
}

bool isgensym(Symstr *sym)
{   return symchain_(sym) == (Symstr *)DUFF_ADDR;
}

/*
 * a temporary home before its demise...
 */

int32 evaluate(Expr *a)
{
/* evaluate the compile-time expression a to yield an integer result     */
    switch (h0_(a))
    {
case s_integer:
        return(intval_(a));
default:
        moan_nonconst(a, bind_msg_const);
        return 1;
    }
}

Binder *global_mk_binder(Binder *b, Symstr *c, SET_BITMAP d, TypeExpr *e)
{
    int32 size = d & (bitofstg_(s_virtual)|b_globalregvar) ?
                     sizeof(Binder) : SIZEOF_NONAUTO_BINDER;
    Binder *p = (Binder*) GlobAlloc(SU_Bind, size);
 /*
  * This consistency check is removed so that front-ends for languages
  * other than C can create binders for auto variables in global store.
>>> if (d & bitofstg_(s_auto)) syserr("Odd global binder(%lx)", (long)d); <<<
  */
    if (d & bitofstg_(s_extern)) check_extern(c);
    h0_(p) = s_binder;
    bindcdr_(p)=b;
    bindsym_(p)=c;
    attributes_(p) = 0;
    p->bindstg=d;
    p->bindtype=e;
    p->bindaddr.i = 0;  /* soon BINDADDR_UNSET - remember 'datasegment' */
#ifdef PASCAL /*ECN*/
    p->bindlevel = 0;
    p->synflags = 0;
#endif
    if (size == sizeof(Binder))
        bindxx_(p) = GAP;
    return p;
}

TagBinder *global_mk_tagbinder(TagBinder *b, Symstr *c, AEop d)
{
    TagBinder *p = (TagBinder*) GlobAlloc(SU_Bind, sizeof(TagBinder));
    h0_(p) = s_tagbind;
    tagbindcdr_(p)=b;
    tagbindsym_(p)=c;
    attributes_(p) = bitoftype_(d);
    tagbindmems_(p) = 0;
    tagbindtype_(p) = (TypeExpr *)DUFF_ADDR;
#ifdef CPLUSPLUS
    p->friends = NULL;
    p->tagparent = current_member_scope();
#endif
#ifdef TARGET_HAS_DEBUGGER
    p->tagbinddbg = 0;
#endif
    return p;
}

Binder *mk_binder(Symstr *c, SET_BITMAP d, TypeExpr *e)
{
    Binder *p = (Binder*) BindAlloc(
        (d & (bitofstg_(s_auto)|bitofstg_(s_virtual))) ?
            sizeof(Binder) : SIZEOF_NONAUTO_BINDER);
    if (d & bitofstg_(s_extern)) check_extern(c);
    h0_(p) = s_binder;
    bindcdr_(p)=0;
    bindsym_(p)=c;
    attributes_(p) = 0;
    p->bindstg = d;
    p->bindtype=e;
    p->bindaddr.i = BINDADDR_UNSET;
#ifdef PASCAL /*ECN*/
    p->bindlevel = 0;
    p->synflags = 0;
#endif
    if (d & bitofstg_(s_auto))
    {   p->bindxx=GAP;
        p->bindmcrep = NOMCREPCACHE;
    }
    return p;
}

static TagBinder *mk_tagbinder(Symstr *c, AEop d)
{
    TagBinder *p = (TagBinder*) SynAlloc(sizeof(TagBinder));
    h0_(p) = s_tagbind;
    tagbindcdr_(p)=0;
    tagbindsym_(p)=c;
    attributes_(p) = bitoftype_(d);
    tagbindmems_(p) = 0;
    tagbindtype_(p) = (TypeExpr *)DUFF_ADDR;
#ifdef CPLUSPLUS
    p->friends = NULL;
    p->tagparent = current_member_scope();
#endif
#ifdef TARGET_HAS_DEBUGGER
    p->tagbinddbg = 0;
#endif
    return p;
}

extern LabBind *mk_labbind(LabBind *b, Symstr *c)
{
    LabBind *p = (LabBind*) SynAlloc(sizeof(LabBind));
    p->labcdr = b;
    p->labsym = c;
    p->labinternlab = 0;
    p->labuses = 0;
    return p;
}

/* comments re globalisation:
   Binders are globalized or not on creation, the only need for
   globalize routines is for the type expressions (see globalize_typeexpr()
   below) which hang from them.
*/

Expr *globalize_int(int32 n)
{   return (Expr*)global_list5(SU_Const, s_integer,te_int,0,n,0);
}

static Symstr *globalize_declaree1(Symstr *d)
/* @@@ dying */
{
    if (d==0) return d;
    if (h0_(d) == s_identifier)
         /* declaree within formal parameter within type.        */
         /* struct's have their own code in syn.c (to change?).  */
         return d;
     syserr(syserr_globalize, (VoidStar)d, (long)h0_(d));
     return d;
}

static FormTypeList *globalize_formals(FormTypeList *d)
{   FormTypeList *d1;
    if (d == NULL) return NULL;
    d1 = (FormTypeList*) global_list3(SU_Type, 0,
                globalize_declaree1(d->ftname),
                globalize_typeexpr(d->fttype));
    d1->ftcdr = globalize_formals(d->ftcdr);
    return d1;
}

static struct te_memo { struct te_memo *cdr; TypeExpr te; }
    *glob_typeexpr_memo;

static TypeExpr *globalize_memo(TypeExpr *t)
{   struct te_memo *p;
    for (p = glob_typeexpr_memo; p != 0; p = p->cdr)
        if (EQtype_(t, &p->te)) break;
    if (p == 0)
        p = glob_typeexpr_memo =
            (struct te_memo*) global_list5(SU_Type, glob_typeexpr_memo,
                         h0_(t), typespecmap_(t), typespecbind_(t), 0);
    return &p->te;
}

/* globalize_typeexpr caches only basic types (including structs/typedefs) */
/* and pointers/refs to things already cached.  Tough ched arrays/fns.     */
/* N.B. we should never cache empty arrays as size may get updated.        */

TypeExpr *globalize_typeexpr(TypeExpr *t)
{   static bool glob_incache;
    TypeExpr *ans;
#ifdef PASCAL /*ECN*/
    assert(0);
#else
    switch (h0_(t))
    {
case t_content:
case t_ref:
        {   TypeExpr *gt = globalize_typeexpr(typearg_(t));
            if (glob_incache)
            {   TypeExpr temp;
                h0_(&temp) = h0_(t), typearg_(&temp) = gt,
                typeptrmap_(&temp) = typeptrmap_(t);
                /* dbglanginfo field?  Doesn't matter for C! */
                return globalize_memo(&temp);
            }
            return (TypeExpr *)global_list4(SU_Type,
                                            h0_(t), gt, typeptrmap_(t), 0);
            /* note that glob_incache is set correctly */
        }
case t_subscript:
            ans = (TypeExpr*) global_list4(SU_Type, t_subscript,
                                globalize_typeexpr(typearg_(t)),
                                typesubsize_(t)==0 ? 0 :
                                    globalize_int(evaluate(typesubsize_(t))),
                                0);
            glob_incache = 0;
            return ans;
case t_fnap:
            /* the DeclRhsList of formals could well become a ClassMember */
            ans = g_mkTypeExprfn(t_fnap,
                               globalize_typeexpr(typearg_(t)),
                               globalize_formals(typefnargs_(t)),
                               &typefnaux_(t));
            glob_incache = 0;
            return ans;
case t_coloncolon:
            ans = (TypeExpr *)global_list4(SU_Type, t_coloncolon,
                                           globalize_typeexpr(typearg_(t)),
                                           typespectagbind_(t),
                                           0);
            glob_incache = 0;
            return ans;
case s_typespec:
            /* N.B. any binder in typespecbind_(t) is assumed globalised */
            glob_incache = 1;
            return typespecmap_(t) == typespecmap_(te_void) ? te_void :
                   typespecmap_(t) == typespecmap_(te_int) ? te_int :
                   typespecmap_(t) == typespecmap_(te_lint) ? te_lint :
                   typespecmap_(t) == typespecmap_(te_uint) ? te_uint :
                   typespecmap_(t) == typespecmap_(te_ulint) ? te_ulint :
                   typespecmap_(t) == typespecmap_(te_float) ? te_float :
                   typespecmap_(t) == typespecmap_(te_double) ? te_double :
                   typespecmap_(t) == typespecmap_(te_ldble) ? te_ldble :
                   globalize_memo(t);
default:
            syserr(syserr_globalize1, (VoidStar)t, (long)h0_(t));
            return t;
    }
#endif
}

/* Binding:
   There are 5 overloading classes, of which 3 (labels, vars, struct tags)
   are bindings in the traditional sense.  All code concerning binding
   and unbinding is in this file.  Access routes are the procedures below:
     Labels:  label_xxx;
     Vars:    instate_declaration, findbinding.
     Tags:    instate_tagbinding, findtagbinding, settagmems.
     Scopes:  push_scope, pop_scope, clear_stacked_scopes.

   Note that typedef names share the same binding space with variables.

   Labels have function scope and function scopes do not nest. Function
   scopes and the global (file) scope for variables and tags is implemented
   using a hash table of Symstrs with separate Binder, TagBinder and LabBind
   pointer fields.

   Local scopes for Binders and TagBinders are implemented using the 'deep
   binding' strategy, as are C++ class scopes. For each scope there is a
   list of Binders and a list of TagBinders which can be searched for the
   matching Symstr. If the search fails in each local scope then the global
   binding (if any) is found in O(1) time.

   In local scopes, Binders and TagBinders are segregated on separate lists
   for faster searching. In a C++ class scope, class members, class-scope
   Binders and class-scope TagBinders exist on a single list, discrimiated
   by the leading AEop field of each entry (s_binder, s_tagbind, s_member).

   APOLOGY: This assumes that Binder, TagBinder and ClassMember all begin:
   {AEop sort;  SelfType *cdr;  Symstr *sv; ...} so we can pun. It can be
   made cleaner with a common initial structure and casts, but this spreads
   the filth over several modules rather than localising it here in bind.c.

   NOTE: binding information persists only for the duration of parsing -
   consequently it makes sense for the parse tree to contain references to
   binding records rather than the main symbol table entries. Toplevel
   binding are allocated in 'global' store which is not reclaimed after each
   top-level phrase.

   NOTE: This also means that local Binders/TagBinders may not be inspected
   during/after register allocation (which reuses syntax store).

*/

/* AM: create a globalized Binder.  For use in rd_decl and
   implicit_decl.  Beware: its components still need globalizing.
   Possible optimisation: overwrite if already there on re-definition.
   Precondition to call: loc must not represent a local binding.
*/
#define topbind2(sv, stg, typ) \
   (bind_global_(sv) = topbindingchain = \
        global_mk_binder(topbindingchain, sv, stg, typ))

static Binder *topbindingchain;                                 /* vars */
static TagBinder *toptagbindchain;                              /* tags */
static LabBind *labelchain;                                     /* labels */

typedef struct Scope {
    struct Scope *prev;
    union
    {   Binder *binding;              /* A scope either contains Binders */
        TagBinder *tagbinding;        /* and TagBinders (a local scope)  */
        ClassMember *member;          /* and members too (class scope).  */
    } u;
    union
    {   Binder **binding;
        TagBinder **tagbinding;
        ClassMember **member;
    } ptr;                            /* used by find{tag}binding()      */
    TagBinder *class_tag;
} Scope;

static Scope *local_scope, *freeScopes;
static bool tag_found_in_local_scope;
        /* used to avoid searching twice in instate_xxx... */
static int scope_level;

#define bindings_(scope)     ((scope)->u.binding)
#define tagbindings_(scope)  ((scope)->u.tagbinding)
#define members_(scope)      ((scope)->u.member)

int push_scope(TagBinder *class_tag)
{   Scope *scope = freeScopes;
    if (scope != NULL)
        freeScopes = freeScopes->prev;
    else
        scope = (Scope *) GlobAlloc(SU_Bind, sizeof(Scope));
    if (debugging(DEBUG_BIND))
        cc_msg("push_scope($b) -> %d\n", class_tag, scope_level);
    scope->u.binding = NULL;
    scope->prev = local_scope;
    local_scope = scope;
    scope->class_tag = class_tag;
    members_(scope) = (class_tag != NULL) ? tagbindmems_(class_tag) : NULL;
    return scope_level++;
}

static void pop_scope_1(int level, bool check_unrefd)
{   Scope *scope = local_scope;
    Binder *p;
    if (level > scope_level)
        syserr("pop_scope: level=%d >= scope_level=%d", level, scope_level);
  while (scope_level > level)
  { if (debugging(DEBUG_BIND)) cc_msg("pop_scope(%d)\n", scope_level);
    if (scope == NULL) syserr("pop_scope: NULL scope pointer");
    if (scope->class_tag == NULL && check_unrefd)
    {   /* Check for unreferenced names in local scopes */
        Symstr *sv;
        for (p = bindings_(scope);  p != 0;  p = bindcdr_(p))
        {   if (h0_(p) != s_binder) continue;
            sv = bindsym_(p);
            /* do a bit more in the next line for used/set */
            /* suppress warning for gensym'd vars, which patch up user errs */
            if ((binduses_(p) & u_referenced) == 0 &&
                (symname_(sv)[0] != '<') &&
                (bindstg_(p) & b_pseudonym) == 0)
                cc_warn(bind_warn_not_used(bindstg_(p) & bitoftype_(s_typedef),
                                       bindstg_(p) & b_fnconst, p));
        }
    }
    else
    /* The next line fixes nasties like:                                */
    /* "struct d { struct d { int a; } c; } x;"  by inhibiting the      */
    /* outer setting and so recovering to "struct d { int a; }.         */
    {   TagBinder *b = scope->class_tag;
        scope->class_tag = NULL;                /* for future safety... */
        if (b != NULL && (attributes_(b) & TB_BEINGDEFD))
        {   attributes_(b) &= ~TB_BEINGDEFD;
            attributes_(b) |= TB_DEFD;
            tagbindmems_(b) = members_(scope);
        }
    }
    local_scope = scope->prev;
    scope->prev = freeScopes;
    freeScopes = scope;
    scope = local_scope;
    --scope_level;
  }
}

void pop_scope(int level)
{   pop_scope_1(level, YES);
}

void pop_scope_no_check(int level)
{   pop_scope_1(level, NO);
}

int unpop_scope(void)
/* unpop the last scope popped... called at start of function defn */
{   Scope *scope = freeScopes;
    if (debugging(DEBUG_BIND)) cc_msg("unpop_scope(%d)\n", scope_level);
    if (scope == NULL)
    {   syserr("unpop_scope");
        return scope_level;
    }
/* The next line isn't really a syserr, just a temp AM check.           */
    freeScopes = scope->prev;
    scope->prev = local_scope;
    local_scope = scope;
    return scope_level ++;
}

#ifdef CPLUSPLUS
static ClassMember **find_localmember(Symstr *sv, ClassMember **p);
/* Note that in C++ find_localmember and find_localbinding are identical! */
#endif

static Binder **find_localbinding(Symstr *sv, Binder **p)
{   Binder *b;
    for (b = *p;  b != NULL;   b = *(p = &bindcdr_(b)))
    {   if (debugging(DEBUG_BIND))
            cc_msg("findbinder try $r %lx\n", bindsym_(b), (long)attributes_(b));
        if (bindsym_(b) == sv)
            if (h0_(b) == s_binder
#ifdef CPLUSPLUS
             || h0_(b) == s_member
#endif
                ) break;
#ifdef CPLUSPLUS
        if (attributes_(b) & CB_CORE)
        {   TypeExpr *t = bindtype_(b);
            if (h0_(t) == s_typespec && typespecmap_(t) & CLASSBITS)
            {   TagBinder *tb = typespectagbind_(t);
                ClassMember **pp = find_localmember(sv, &tagbindmems_(tb));
                if (*pp) { p = (Binder **)pp; break; }
            }
            else syserr("findbinding");
        }
#endif
    }
    return p;
}

static Binder *findbinding_2(Symstr *sv, Scope *scope)
{   Binder **p = find_localbinding(sv, &bindings_(scope)), *b;

    scope->ptr.binding = p;
    b = *p;
    if (b != NULL && h0_(b) == s_binder && (bindstg_(b) & b_pseudonym))
        b = realbinder_(b);
    return b;
}

Binder *findbinding(Symstr *sv)
{   Scope *scope = local_scope;
    for (; scope != NULL; scope = scope->prev)
    {   Binder *b = findbinding_2(sv, scope);
        if (b != NULL) return b;
    }
    return bind_global_(sv);
}

static TagBinder *find_localtagbinding(Symstr *sv, Scope *scope)
{   TagBinder *b, **p;
    p = &tagbindings_(scope);
    for (b = *p;  b != NULL;  b = *(p = &tagbindcdr_(b)))
        if (bindsym_(b) == sv && h0_(b) == s_tagbind) break;
    scope->ptr.tagbinding = p;
    return b;
}

TagBinder *findtagbinding(Symstr *sv)
{   Scope *scope = local_scope;
    TagBinder *b;
    tag_found_in_local_scope = (scope != NULL);
    while (scope != NULL)
    {
#ifndef CPLUSPLUS
        if (scope->class_tag == NULL)    /* C class scopes don't nest */
#endif
        {   if ((b = find_localtagbinding(sv, scope)) != NULL) return b;
            tag_found_in_local_scope = NO;
        }
        scope = scope->prev;
    }
    return tag_global_(sv);
}

static ClassMember **find_localmember(Symstr *sv, ClassMember **p)
{   ClassMember *l;
    for (l = *p;  l != NULL;  l = *(p = &memcdr_(l)))
    {   if (debugging(DEBUG_BIND))
            cc_msg("findmember try $r %lx\n", memsv_(l), (long)attributes_(l));
        if (memsv_(l) == sv)
            if (h0_(l) == s_member
#ifdef CPLUSPLUS
             || h0_(l) == s_binder
#endif
                ) break;
#ifdef CPLUSPLUS
        if (attributes_(l) & CB_CORE)
        {   TypeExpr *t = memtype_(l);
            if (h0_(t) == s_typespec && typespecmap_(t) & CLASSBITS)
            {   TagBinder *tb = typespectagbind_(t);
                ClassMember **pp = find_localmember(sv, &tagbindmems_(tb));
                if (*pp) { p = pp; break; }
            }
            else syserr("findmember");
        }
#endif
    }
    return p;
}

#ifdef CPLUSPLUS
ClassMember *findclassmember(Symstr *sv, TagBinder *b, bool inherit)
{
/* Soon inherit=1 will indicate that we need to look via inheritance.   */
/* We need to move towards more explicit scope args for routines like   */
/* this (used to find the __ctor of a class etc).                       */
    return *find_localmember(sv, &tagbindmems_(b));
}
#endif

ClassMember *findmember(Symstr *sv)
{   ClassMember **p = find_localmember(sv, &members_(local_scope));
    local_scope->ptr.member = p;
    return *p;
}

#ifdef CPLUSPLUS
static Expr *path_to_base_member(TagBinder *b, SET_BITMAP qualifiers,
                        Symstr *sv, TagBinder *start_scope);
static bool derived_from(TagBinder *, TagBinder *);
static bool accessor_is_friendof(TagBinder *ofclass);

static bool accessOK;           /* local to path_to_member... */
TagBinder *found_in_class;      /* now local to path_to_member... */
static TagBinder *accessing_class;
#endif


static Expr *path_to_member_1(TagBinder *tb, SET_BITMAP qualifiers,
                        Symstr *sv, TagBinder *start_scope)
{   ClassMember *l;
    int32 sort;
    StructPos p;

if (start_scope == NULL || start_scope == tb)
{   memset(&p, 0, sizeof(p));
    sort = attributes_(tb) & CLASSBITS;
    for (l = tagbindmems_(tb);  l != NULL;  l = memcdr_(l))
    {   structfield(l, sort, &p);
        if (debugging(DEBUG_BIND))
            cc_msg("see $r %lx\n", memsv_(l), (long)attributes_(l));
        if (memsv_(l) != sv) continue;
#ifdef CPLUSPLUS
/* Note that access is maximal, so the following does just fine...      */
        if (!accessOK)
        {   if (attributes_(l) & bitofaccess_(s_public))
                accessOK = 1;
            else
            {   if ((attributes_(l) & bitofaccess_(s_private)) &&
                        tb == accessing_class ||
                    (attributes_(l) & bitofaccess_(s_protected)) &&
                        accessing_class != NULL &&
                        derived_from(tb, accessing_class) ||
                    accessor_is_friendof(tb))
                    accessOK = 1;
            }
        }
        if (l->memtype == ACCESSADJ) /* access declaration */ continue;
        found_in_class = tb;
/* This is the dreadful A::b access declaration hack... */
        if (h0_(l) == s_member && (qualifiers & DECL_CTXT)) return (Expr *)l;
        if (h0_(l) == s_member)
#endif
        {   TypeExpr *t;
#ifdef CPLUSPLUS
            if (qualifiers & NO_CTXT)
            {   cc_err("can't access $c::$r: no 'this' pointer", tb, memsv_(l));
                return errornode;
            }
#endif
            t = mkqualifiedtype(l->memtype, qualifiers);
            p.boffset = target_lsbitfirst ?
                MAXBITSIZE-p.bsize - p.boffset : p.boffset;
            return mk_exprbdot(s_dot, t, 0, p.woffset, p.bsize, p.boffset);
        }
#ifdef CPLUSPLUS
/* @@@ re-check this stuff!                                             */
        else if (h0_(l) == s_binder)
        {   Binder *b = (Binder *)l;
            if (bindstg_(b) & b_pseudonym) b = realbinder_(b);
            if (!(bindstg_(b) & (bitofstg_(s_static)|bitofstg_(s_typedef))) &&
                /* overloaded fns can't be typedefs, invent ovld-stg bit? */
                (qualifiers & DECL_CTXT) == 0 &&
                h0_(bindtype_(b)) == t_ovld)
                /* exprdotmemfn_() */
                return mk_exprwdot(s_dot, bindtype_(b), 0, (int32)(Expr *)b);
            /* presume a typedef name, static member or declarator context */
            return (Expr *)b;
        }
/* The following should never happen - paranoia during development...   */
        else if (h0_(l) != s_tagbind)
            syserr("h0_(path_to_member($r,,$r,)) == %ld",
                tagbindsym_(tb), sv, h0_(l));
#endif
    }
    start_scope = NULL;       /* from here on down search everywhere... */
}
#ifdef CPLUSPLUS
    return path_to_base_member(tb, qualifiers, sv, start_scope);
#else
    return NULL;
#endif
}

Expr *path_to_member(TagBinder *b, SET_BITMAP qualifiers,
                        Symstr *sv, TagBinder *start_scope)
{   Expr *e;
    if (b == 0) syserr("path_to_member(0,...)");
#ifdef CPLUSPLUS
    found_in_class = 0;
    accessOK = 0;
    {   ClassMember *l = tagbindmems_(b);
        if (l != NULL && (attributes_(l) & CB_CORE))
        {
/* The following section is not C++, but is useful for testing... */
            StructPos p;
            memset(&p, 0, sizeof(p));
            for (;  l != NULL;  l = memcdr_(l))
            {   if (memsv_(l) == sv)
                    return mk_exprbdot(s_dot, l->memtype, 0, p.woffset, 0, 0);
                structfield(l, attributes_(b) & CLASSBITS, &p);
            }
/* End of temporary language extension (for testing)...          */
            b = typespectagbind_(memtype_(tagbindmems_(b)));
            if (b == 0) syserr("path_to_core_member(0,...)");
        }
    }
#endif
    e = path_to_member_1(b, qualifiers, sv, start_scope);
#ifdef CPLUSPLUS
    if (e != NULL && !accessOK && !(qualifiers & DECL_CTXT))
    {   /* need to do better here... */
        cc_rerr(sem_rerr_nonpublic, sv, b);
    }
#endif
    return e;
}

void add_toplevel_binder(Binder *b)
{   bind_global_(bindsym_(b)) = b;
    bindcdr_(b) = topbindingchain;
    topbindingchain = b;
}

static void add_local_binder(Binder *b)
/* Precondition: find_localbinding() has been called to set the insertion */
/* point; usually at the end of the list, but in the case of a duplicate  */
/* entry, just before the old entry.                                      */
/* @@@ AM: do we wish to keep this behaviour?                             */
{   Scope *scope = local_scope;
    Binder **p = scope->ptr.binding;
    if (debugging(DEBUG_BIND))
    {   cc_msg("add_local_binder($b) in scope %p\n", b, scope);
        if (debugging(DEBUG_TYPE))
            cc_msg("princtype %ld\n", h0_(bindtype_(b)));
    }
    bindcdr_(b) = *p;
    *p = b;
}

static void add_local_tagbinder(TagBinder *b)
/* Precondition: findtagbinding() has been called to set the insertion    */
/* point; usually at the end of the list, but in the case of a duplicate  */
/* entry, just before the old entry.                                      */
/* Note that in C, class scopes don't nest; see also findtagbinding().    */
{   Scope *scope = local_scope;
    TagBinder **p;
#ifndef CPLUSPLUS
    while (scope->class_tag != NULL) scope = scope->prev;
    /* Note the collusion with findtagbinding() in the following line...  */
#endif
    p = scope->ptr.tagbinding;
    tagbindcdr_(b) = *p;
    *p = b;
}

static void add_member(ClassMember *m)
/* Precondition: findmember() has been called to set the insertion point. */
/* However, always add members at the end of the list, irrespective.      */
{   Scope *scope = local_scope;
    ClassMember *l, **p = scope->ptr.member;
    if (debugging(DEBUG_BIND))
        cc_msg("add_member($r) in scope %p\n", memsv_(m), scope);
    while ((l = *p) != NULL) p = &memcdr_(l);
    memcdr_(m) = NULL;
    *p = m;
}

#ifdef CPLUSPLUS
static Binder *instate_member_binder(DeclRhsList *d, int bindflg);
#endif

static ClassMember *instate_member_1(DeclRhsList *d, int bindflg)
{   ClassMember *m;
    Expr *bitsize;
    TypeExpr *t, *t2;
    Symstr *sv = d->declname;
    SET_BITMAP access = attribofstgacc_(d->declstg);
/* note: other users of d->declstg should use killstgacc_(d->declstg).  */
    if (access==0) syserr("instate_member(access==0)");

    if (sv != NULL && findmember(sv) != NULL)
    {   cc_rerr(syn_rerr_duplicate_member(sv, local_scope->class_tag));
        sv = gensymval(1);
    }
#ifdef CPLUSPLUS
    if ((d->declstg & STGBITS) || isfntype(d->decltype))
        /* catch member fns, statics, typedef, enum constant, */
        /* inline, virtual and friend.                        */
        return (ClassMember *)instate_member_binder(d, bindflg);
#endif
/* bindflg is set so that all structs are globalized except within fns. */
/* This includes structs declared in formal parameter lists whose scope */
/* is only the function.                                                */
    bitsize = declbits_(d);
    t = d->decltype;
/* @@@ don't do this if t == ACCESSADJ?                                 */
    t2 = (TypeExpr *)syn_list3(t_coloncolon, t, local_scope->class_tag);
    if (bindflg & (GLOBALSCOPE|TOPLEVEL))
    {   /* invent a globalize_tagmemlist()? */
        m = (ClassMember *) GlobAlloc(SU_Type, sizeof(ClassMember));
        t2 = globalize_typeexpr(t2);  t = typearg_(t2);
/* There must be a better way to hold size-or-zero.                     */
        bitsize = bitsize == 0 ? 0 : globalize_int(evaluate(bitsize));
    }
    else
    {   m = (ClassMember *) BindAlloc(sizeof(ClassMember));
        if (bitsize != 0 && h0_(bitsize) != s_integer)
            bitsize = mkintconst(te_int, evaluate(bitsize), 0);
    }
    h0_(m) = s_member;
    memcdr_(m) = 0,
    memsv_(m) = sv;                    /* 0 for padding (:0) bit fields */
    m->memtype = t;
    m->memtype2 = t2;
    m->u.membits = bitsize;
    attributes_(m) = access;
    add_member(m);
    return m;
}

/* struct/union/enum tag bindings ... */

TagBinder *instate_tagbinding(Symstr *sv, AEop s, TagDefSort defining,
            int bindflg)
{   TagBinder *b;
    if (sv == 0)
    {   sv = gensymval(1);
        defining = TD_ContentDef;
    }
    if (bindflg & TOPLEVEL)
    {   b = tag_global_(sv);
        if (b == 0 || defining != TD_NotDef)
        {   /* new tag or tag being defined... */
            if (debugging(DEBUG_BIND))
                cc_msg("top level struct $r@%p\n", sv, (VoidStar)b);
            if (b == 0)
            {   /* introduction of new tag */
                tag_global_(sv) = toptagbindchain = b =
                    global_mk_tagbinder(toptagbindchain,sv,s);
#ifdef CPLUSPLUS
/* @@@ the next line avoids 'true' scoping as a hack to make B=B in...  */
/*      class A { class B *p; }; class B { whatever };                  */
                if (defining == TD_NotDef && b->tagparent != 0)
                    b->tagparent = 0,
                    cc_warn("C++ scope may differ: $c", b);
#endif
            }
            else if (((attributes_(b) & TB_DEFD) && defining != TD_Decl) ||
                      /* re-definition */
                     (attributes_(b) & TB_BEINGDEFD))
                cc_err(bind_err_duplicate_tag, tagbindsort(b),b);
        }
    }
    else
    {   b = findtagbinding(sv);
        if (b == 0 || defining != TD_NotDef)
        {   /* new tag or tag being defined... */
            if (debugging(DEBUG_BIND))
                cc_msg("local struct $r@%p\n", sv, (VoidStar)b);
            if (b != 0 && tag_found_in_local_scope &&
                (((attributes_(b) & TB_DEFD) && defining != TD_Decl)
                 ||  /* re-definition */
                 (attributes_(b) & TB_BEINGDEFD)))
            {
                cc_err(bind_err_duplicate_tag, tagbindsort(b), b);
                b = 0;
            }
            if (b == 0 ||
                defining == TD_ContentDef && !tag_found_in_local_scope)
            {
/* bindflg & GLOBALSCOPE refers to tags in formals: these need careful  */
/* treatment in that they are somewhat visible.  e.g. equivtype needs   */
/* to see "f(struct a { int b,c;})" differing from g of similar type.   */
                b = bindflg & GLOBALSCOPE ?
                    global_mk_tagbinder(0, sv, s) : mk_tagbinder(sv, s);
#ifdef CPLUSPLUS
/* @@@ the next line avoids 'true' scoping as a hack to make B=B in...  */
/*      class A { class B *p; }; class B { whatever };                  */
                if (defining == TD_NotDef && b->tagparent != 0)
                    b->tagparent = 0,
                    cc_warn("C++ scope may differ: $c", b);
#endif
                if (local_scope == NULL)
                    syserr("instate_tagbinding - no local scope");
                add_local_tagbinder(b);
            }
        }
    }
    if ((attributes_(b) & ENUMORCLASSBITS) != bitoftype_(s))
        cc_err(bind_err_reuse_tag, tagbindsort(b), b, s);
    if (defining != TD_NotDef) attributes_(b) |= TB_BEINGDEFD;
    return b;
}

void settagmems(TagBinder *b, ClassMember *l)
/* dying...? */
{   /* We should have already given an error on redefinition.           */
    /* The next line fixes nasties like:                                */
    /* "struct d { struct d { int a; } c; } x;"  by inhibiting the      */
    /* outer setting and so recovering to "struct d { int a; }.         */
    if (attributes_(b) & TB_BEINGDEFD)
    {   attributes_(b) &= ~TB_BEINGDEFD;
        attributes_(b) |= TB_DEFD;
        tagbindmems_(b) = l;
    }
}

/* variable and typedef bindings... */

Binder *implicit_decl(Symstr *a, int32 fn)
{
/* implicit declaration of 'extern int x' or 'extern int f()'  */
/* N.B. the information has to be generated in the global heap */
    TypeExpr *t = te_int;
    TypeExprFnAux s;
    if (fn)
        t = g_mkTypeExprfn(t_fnap, t, 0,
            packTypeExprFnAux(s, 0, 999, 0, 0,
                fpregargs_disabled ? (int) f_nofpregargs : 0)); /* minargs_ */
    topbind2(a, (fn ? bitofstg_(s_extern)|b_undef|b_fnconst :
                bitofstg_(s_extern)|b_undef), t);
    binduses_(topbindingchain) |= u_implicitdef;
    return topbindingchain;
}

/*
 * Used below and in instate_declaration for tentative Ansi definitions.
 * To say that this code is NASTY is a gross understatement. In fact,
 * it is hard to describe its nastiness in mere words.
 * @@@ AM (Sep 89) wants to re-work all this structure soon.
 */
typedef struct TentativeDefn
{
    struct TentativeDefn *next;
    DataDesc   data;
    int32      size, align;
/* When TARGET_HAS_BSS is NOT set, maybebss=1 <==> size=0.              */
    bool       maybebss;
    bool       statik;
    Symstr     *sv;
} TentativeDefn;

static TentativeDefn *tentative_defs;     /* also init'd by bind_init */
static DataInit *datahead,
                *datasplice,
                *datatail;
static TentativeDefn saved_vg_state;

static void save_vargen_state(TentativeDefn *td)
{
    td->data.head = datap->head; td->data.tail = datap->tail;
    td->data.size = datap->size;
}

static void restore_vargen_state(TentativeDefn *td)
{
    datap->head = td->data.head; datap->tail = td->data.tail;
    datap->size = td->data.size;
}

/*
 * Used to make a tentative defns list for the following routines.
 * Basically, this routine is called to record information about
 * the state of vargen.c before a zero initialiser for a tentative
 * static is processed.
 * It also holds whether we hope to allocate a top-level variable in bss.
 */
static bool addTentativeDefn(Symstr *sv, int32 size, int32 align, bool statik)
{
    TentativeDefn *td = NULL;
    for (td = tentative_defs; td != NULL; td = td->next)
        if (td->sv == sv) break;
    if (td == NULL)
    {   td = (TentativeDefn *) GlobAlloc(SU_Other, sizeof(TentativeDefn));
        td->data.head = td->data.tail = (DataInit *)DUFF_ADDR;
        td->data.size = 0;
        td->next      = tentative_defs;
        td->size      = 0;
        td->align     = 0;
        td->sv        = sv;
        td->maybebss  = YES;
        td->statik    = statik;
        tentative_defs= td;
    }
/* The following test fills in the size details in:  int a[],a[100];    */
    if (size != 0)
    {   td->size = size;
        td->align = align;
#ifdef TARGET_HAS_BSS
        td->maybebss = (feature & FEATURE_PCC) || (size > BSS_THRESHOLD);
#else
        td->maybebss = 0;
#endif
        save_vargen_state(td); /* size = 0 => tentative foo[] */
    }
#ifdef DEBUG_TENTATIVE_DEFS
if (debugging(DEBUG_BIND)) cc_msg(
"addTentativeDefn %lx, %lx next %lx (%s) head %lx tail %lx loc %ld name %s size %ld\n",
    (int32)datap,
    (int32) td, (int32) td->next, (td->next) ? symname_(td->next->sv) : "",
    (int32)datap->head, (int32)datap->tail, (int32)datap->size, symname_(sv), size);
#endif
    return td->maybebss;
}

#ifdef DEBUG_TENTATIVE_DEFS

static void show_vargen_state(char *when)
{
    if (debugging(DEBUG_BIND))
    {   DataInit *tmpdataq;
        cc_msg("vargen state %lx %s restoration:-\n", (int32)datap, when);
        for (tmpdataq = datap->head; tmpdataq != 0; tmpdataq = tmpdataq->datacdr)
        {
            cc_msg(
                "DataInit %lx: cdr %lx rpt %ld sort %lx len %ld val %ld\n",
                (int32) tmpdataq, (int32) tmpdataq->datacdr, tmpdataq->rpt,
                tmpdataq->sort, tmpdataq->len, tmpdataq->val);
        }
        cc_msg("head = %lx tail = %lx size = %ld\n\n",
            (int32)datap->head, (int32)datap->tail, datap->size);
    }
}

#endif
/*
 * The following routines are the ones that do the messing around with
 * vargen pointers in order to throw away old (zero) tentative initialisers
 * and replace them with new initialisers.
 */
static bool is_tentative(Symstr *sv)
{
    TentativeDefn *td = tentative_defs, *prev;

    for (prev = NULL;  td != NULL;  prev = td, td = td->next)
    {   if (td->sv == sv)
        {   /* we are going to return TRUE at the end of this iteration.  */
            int32 count=0;
            DataInit *tmpdataq;
            /*
             * Found a tentative definition so let's reset vargen's state
             * ready for the initialiser... but only if (old) size != 0.
             * @@@ the 'size' test next is becoming subsumed by the bss test.
             */
            if (td->size != 0 && !td->maybebss)
            {   save_vargen_state(&saved_vg_state);
#ifdef DEBUG_TENTATIVE_DEFS
if (debugging(DEBUG_BIND)) show_vargen_state("before");
#endif
                /* Restore vg's state to that before reading the initialiser */
                restore_vargen_state(td);
                datahead = td->data.head;
#ifdef DEBUG_TENTATIVE_DEFS
if (debugging(DEBUG_BIND)) show_vargen_state("after");
#endif
                /*
                 *  Throw away old tentative (zero) initialiser ready for
                 *  replacement by genstaticparts().
                 */
                if (datap->head == 0)
                    tmpdataq = saved_vg_state.data.head;
                else
                {
                    tmpdataq = data.tail->datacdr;
                    datap->tail->datacdr = 0;
                }
                while (count < td->size)
                {
                    if (tmpdataq == NULL)
                        syserr(syserr_tentative);
                    /* skip labels in static init chain */
                    if (tmpdataq->sort != LIT_LABEL)
                    {
/* AM: insert check for ->len field being valid (i.e. not LABEL/ADCON   */
/* maybe this cannot happen, but AM gets very worried by this sort      */
/* of 'if not LABEL then nice' reasoning.                               */
                        if (tmpdataq->sort == LIT_ADCON)
                            syserr(syserr_tentative1);
                        count += tmpdataq->rpt * tmpdataq->len;
                        datasplice = tmpdataq;
                    }
                    tmpdataq = tmpdataq->datacdr;
                }
                if (count != padsize(td->size,alignof_toplevel))
                    syserr(syserr_tentative2);
                datatail = tmpdataq;
                /* set flag for reset_vg_after_init_of_tentative_defn() */
                saved_vg_state.size = td->size;
            }
            else
            {
if (debugging(DEBUG_BIND)) cc_msg("maybebss found\n");
            }
            /* remove entry from tentative list */
            if (prev == NULL)
                tentative_defs = td->next;
            else
                prev->next = td->next;
            return YES;
        }
    }
    return NO;
}

void reset_vg_after_init_of_tentative_defn(void)
{
    TentativeDefn *td;
    if (saved_vg_state.size == 0) return;
    /*
     * Vargen has now inserted the new initialiser just where we want it.
     * Hum, however I might have removed an item from the list which I
     * have in my own tables.  Check for this and update any items found.
     */
    for (td = tentative_defs;  td != 0;  td = td->next)
        if (td->data.tail == datasplice) td->data.tail = datap->tail;

    /* link new initialiser to the rest of the chain */
    datap->tail->datacdr = datatail;

    /*
     * Reset all the pointers so that vargen does not realise that its
     * internal lists have been modified.
     */
    if (datahead != 0) datap->head = saved_vg_state.data.head;
    if (datatail != 0) datap->tail = saved_vg_state.data.tail;
    datap->size = saved_vg_state.data.size;

    saved_vg_state.size = 0; /* unset flag to prevent recall */
}

static void check_for_incomplete_tentative_defs(TentativeDefn *td)
{
    for (; td != NULL;  td = td->next)
        if (td->size == 0)
            cc_err(bind_err_incomplete_tentative, td->sv);
#ifdef TARGET_HAS_BSS
        else if (td->maybebss)
            addbsssym(td->sv, td->size, td->align, td->statik, 0);
#endif
}

static void check_for_fwd_static_decl(Binder *b, Symstr *sv)
{
    /* The following feature optionally enables spurious forward */
    /* static declarations to be weeded out.                     */
    if (feature & FEATURE_PREDECLARE &&
        !(binduses_(b) & u_referenced) &&
        bindstg_(b) & bitofstg_(s_static))
        cc_warn(bind_warn_unused_static_decl, sv);
}

static void check_ansi_linkage(Binder *b, DeclRhsList *d)
{
    SET_BITMAP clash = (bindstg_(b) ^ d->declstg) & PRINCSTGBITS;
    if (clash)
    {   /* Linkage clash, but do not moan about stray 'extern type name's */
        if ((d->declstg & (b_omitextern | b_globalregvar | bitoftype_(s_static))) ||
                 (bindstg_(b) & b_globalregvar))
        {   /* Oldest linkage wins... */
            cc_rerr(bind_rerr_linkage_disagreement,
                    d->declname, bindstg_(b) & PRINCSTGBITS);
            /* patch d->declstg to a compatible tentative type... */
            d->declstg = bindstg_(b) & PRINCSTGBITS;

        } else if (bindstg_(b) & bitofstg_(s_static))
            /* test is needed because of global register variables */
            /* Explicit extern here, previous was static.
                  Change d to say static.
                  Check OK for C++ */
            d->declstg ^= (bitoftype_(s_static) | bitoftype_(s_extern));
    }
    else check_for_fwd_static_decl(b, d->declname);
}

static bool is_openarray(TypeExpr *t)
{
    t = princtype(t);                          /* skip leading typedefs */
    if ((h0_(t) == t_subscript) && (typesubsize_(t) == 0)) return YES;
    return NO;
}

void instate_alias(Symstr *a, Binder *b)
/*
 * Make the symbol a an alias for b. This curious facility is used to
 * provide local pseudonyms ___first_arg and ___last_arg which share
 * binders with the first and last args in a function's definition, and
 * which are sometimes useful when implementing va_args on awkward machines.
 * Note that pop_varenv (qv) will not moan if the symbols I use as aliases
 * are unused.
 */
{   /* should take care over storage lifetimes here... */
    Binder *pseudonym = mk_binder(a, bindstg_(b)|b_pseudonym, bindtype_(b));
    realbinder_(pseudonym) =
        (bindstg_(b) & b_pseudonym) ? realbinder_(b) : b;
    add_local_binder(pseudonym);
}

static Binder *instate_declaration_1(DeclRhsList *d, int declflag)
/* only the TOPLEVEL and DUPL_OK bits of declflag are examined */
{
/* I have just parsed a declarator, and in that identifiers are held    */
/* as Symstr *'s which contain an h0 of s_identifier.  Instate the      */
/* declaration, returning the Binder record hung off the symbind_ entry.*/
    Symstr *sv;
    Binder *b;
    int32 olduses = 0;
    bool maybebss = 0;
#ifdef PASCAL /*ECN*/
    int level = declflag >> 2;
    declflag &= 3;
#endif

    sv = d->declname;
    
    if (sv == 0 || h0_(sv) != s_identifier)     /* check/remove*/
    {   syserr(syserr_instate_decl, (long)(sv==0 ? 0 : h0_(sv)));
        return 0;
    }
    
    if (attribofstgacc_(d->declstg)) syserr("instate_decl(access!=0)");
    
    if (debugging(DEBUG_BIND))
    {   cc_msg("instate_declaration(%x): $r\n", declflag, sv);
        pr_typeexpr(d->decltype, sv); cc_msg("\n");
    }

#ifdef CPLUSPLUS
/* No need to prunetype as inner typedef would have already been used:  */
    if ((d->declstg & bitofstg_(s_typedef)) &&
            isprimtypein_(d->decltype, ENUMORCLASSBITS))
    {   TagBinder *tb = typespectagbind_(d->decltype);
        if (isgensym(tagbindsym_(tb)))
        {   tagbindsym_(tb) = sv;
            if (declflag & TOPLEVEL) tag_global_(sv) = tb;
        }
    }
#endif
    
    if (declflag&TOPLEVEL)
/* Top level declarations may only surplant previous top level extern decls */
/* Really we should also unify 'local' extern decls.  @@@ not done yet.     */
    {   TypeExpr *glotype = 0;

        b = bind_global_(sv);
#ifdef PASCAL /*ECN*/
        if (b && (bindstg_(b) & b_synbit1)) b = 0;
#endif
        if (b != 0)
        {   olduses = binduses_(b);
            /* check the types match */
            if (h0_(b->bindtype) == t_fnap && h0_(d->decltype) == t_fnap)
            {   /* propagate #pragma -v and -y info from decl to defn */
                if (typefnaux_(d->decltype).variad == 0)
                    typefnaux_(d->decltype).variad =
                        typefnaux_(b->bindtype).variad;
                typefnaux_(d->decltype).flags |=
                    typefnaux_(b->bindtype).flags &
                    ~typefnaux_(d->decltype).flags;
                if ((d->declstg & bitofstg_(s_inline)) &&
                    (bindstg_(b) & b_maybeinline))
                        bindstg_(b) = (bindstg_(b) & ~bitofstg_(s_extern)) |
                            bitofstg_(s_static);
                bindstg_(b) &= ~b_maybeinline;
/* @@@ old-style bit here. */
            }
            switch (equivtype(b->bindtype, d->decltype))
            {   default:
#ifdef TARGET_IS_ARM /* <============================================== */
/* @@@ AM is very worried about the implications of the next line. Does */
/* it allows illegal code to survive with a warning?  Bad for teaching! */
/* Yes, it allows 'extern f(char), f(int);'  Boooo.                     */
/* Not good for C++ either!                                             */
                         if (widened_equivtype(b->bindtype, d->decltype))
                             cc_warn(bind_err_type_disagreement, sv);
                         else
#endif
                             cc_err(bind_err_type_disagreement, sv);
                         break;
                case 2:  glotype = b->bindtype;   /* IDENTICAL */
                case 1:  break;
            }
/* It would be nice to merge the type/stgclass errors so we don't get   */
/* 2 errors for 'typedef int a; extern double a;'                       */
            /* Check for duplicate and conflicting definitions */
            if ((bindstg_(b) | d->declstg) & bitofstg_(s_typedef))
            {   if ((bindstg_(b) & d->declstg) & bitofstg_(s_typedef))
                {   /* can duplicate a typedef in C++, not in C */
#ifndef CPLUSPLUS
                    cc_rerr(bind_rerr_duplicate_typedef, sv);
#endif
                }
                else
                    cc_err(bind_err_duplicate_definition, sv);
            }
/* The next two tests are perhaps more natural the other way round --   */
/* if we have two *definitions* then fault (unless one is tentative),   */
/* otherwise essentially ignore one of the *declarations*.              */
/* It is arguable the PCC case should make a more careful test on u_bss */
/* but this is compatible with previous version.                        */
            else if (bindstg_(b) & b_undef  &&
                     !(bindstg_(b) & u_bss) &&
                      (feature & FEATURE_PCC ||
                        !(bindstg_(b) & b_omitextern &&
                          is_openarray(bindtype_(b)))
                      ) || d->declstg & b_undef)
            {   /* At least one of the declarations has no initialiser. */
                /* N.B. an existing static declaration will appear to   */
                /*    have an initialiser, as do ansi tentatives.       */
                /*    (Provisional) BSS tentatives have b_undef but     */
                /*    they also have u_bss, which we avoid.  Messy!     */
                /*    The delicate case: plain int [] has b_undef.      */
                if (feature & FEATURE_PCC)
                {   /* pcc/as faults b is static or initialised plain,  */
                    /* d is static or plain (whether or not init'd).    */
                    if (!(bindstg_(b) & b_undef) &&
                         (d->declstg & (bitoftype_(s_static) | b_omitextern)))
                        cc_err(bind_err_duplicate_definition, sv);
                    else
                        check_for_fwd_static_decl(b, sv);
                }
                else
                    check_ansi_linkage(b, d);
            }
            else if (!(bindstg_(b) & b_fnconst) &&
                     !(d->declstg  & b_fnconst) &&
                     !(feature & FEATURE_PCC)   &&
                     is_tentative(sv))
                check_ansi_linkage(b, d);
            else
                cc_err(bind_err_duplicate_definition, sv);
        }
        else if (feature & FEATURE_PREDECLARE)
        {   /* The following is a feature to enable policing of a software */
            /* quality policy which says "only objects previously DECLARED */
            /* as extern (presumably in a header) may be DEFINED extern".  */
            if ((d->declstg & bitofstg_(s_extern)) &&
                (!(d->declstg & b_undef) | (d->declstg & b_omitextern)) &&
                sv != mainsym)
                cc_warn(bind_warn_not_in_hdr, sv);
        }
        /* Maybe we wish to turn off the following for non-hosted system.  */
        if (sv == mainsym && (d->declstg & bitofstg_(s_extern)))
        {   TypeExpr *t = princtype(d->decltype);
            /* check args here too one day? */
            if (h0_(t) != t_fnap || !equivtype(typearg_(t), te_int))
                if (!(feature & FEATURE_PCC) || (feature & FEATURE_FUSSY))
                    cc_warn(bind_warn_main_not_int);
        }

        if (feature & FEATURE_PCC)
        {    if ((d->declstg &
                  (bitoftype_(s_static)|b_fnconst|b_globalregvar|bitoftype_(s_typedef))
                 ) == bitoftype_(s_static) && (d->declstg & b_undef))
            {   maybebss = addTentativeDefn(sv, sizeoftype(d->decltype),
                                   alignoftype(d->decltype),
                                   (d->declstg&bitofstg_(s_static)) != 0)
#ifdef CONST_DATA_IN_CODE
                        && !(d->declstg & u_constdata)
#endif
                           ;
                if (!maybebss) d->declstg &= ~b_undef;
            }
        }
        else
        {   if ((b == 0 || bindstg_(b) & b_undef)                  &&
                 !(d->declstg & (b_fnconst|b_globalregvar|bitoftype_(s_typedef))) &&
                 (d->declstg & b_undef)                            &&
                 (d->declstg & (bitoftype_(s_static)|b_omitextern)))
            {
                /* no pre-exisiting defn and not a function defn and    */
                /* no initializer and (static blah... or plain blah...) */
                /* 'static' and b_omitextern are mutually exclusive.    */
                int32 size = is_openarray(d->decltype) ? 0 :
                                                     sizeoftype(d->decltype);
                maybebss = addTentativeDefn(sv, size,
                                   alignoftype(d->decltype),
                                   (d->declstg&bitofstg_(s_static)) != 0)
#ifdef CONST_DATA_IN_CODE
                        && !(d->declstg & u_constdata)
#endif
                           ;
                if (!maybebss) d->declstg &= ~b_undef;
            }
        }

        /*
         * Decls such as 'extern int a;' may be superceded by decls such
         * as 'extern int a=1;'.  However, decls such as 'extern int a=1;'
         * may not be superceded (but useless decls such as 'extern int a;'
         * will still be accepted). BUT BEWARE: 'extern int foo[]' MUST be
         * superceded by [extern] int foo[FOOSIZE] or chaos will ensue in
         * -pcc mode (only the size distinguishes Common Def from Ext Ref).
         */
        if ((b != 0) && (glotype == 0))
        {   /*
             * Assert: glotype == 0 iff equivtype(b, d->...) == 1
             *         equivtype(b,d...) == 1 iff one of b, d is blah[].
             * No: could be (int (*b)()) and (int (*d)(int)).
             * Now check for b (the original decl) being an open array.
             */
            if (is_openarray(bindtype_(b)))
            {   binduses_(b) |= u_superceded;
                b = 0;    /* force treatment of d, below, as if new */
            }
        }
        if (b == 0 || !(d->declstg & b_undef))
#ifdef OLD_VSN
        {   /* suppress new definition if already defined.          */
            if (b != 0) binduses_(b) |= u_superceded;
#ifdef PASCAL /*ECN*/
            topbind2(sv, d->declstg,
                glotype ? glotype : d->decltype);
#else
            topbind2(sv, d->declstg,
                glotype ? glotype : globalize_typeexpr(d->decltype));
#endif
        }
        b = bind_global_(sv);
#else /* !OLD_VSN */
        {   if (b == 0)
#ifdef PASCAL /*ECN*/
                b = topbind2(sv, d->declstg,
                    glotype ? glotype : d->decltype);
#else
                b = topbind2(sv, d->declstg,
                    glotype ? glotype : globalize_typeexpr(d->decltype));
#endif
            else if (!(bindstg_(b) & b_globalregvar))
              /* Assert: !(d->declstg & b_undef) */
                bindstg_(b) = d->declstg |
                  (bindstg_(b) & (bitofstg_(s_virtual)|bitofstg_(s_inline)));
        }
#endif
#ifdef TARGET_HAS_BSS
        if (maybebss)
        {   /* addbsssym() now called when we know for certain. */
            bindaddr_(b) = BINDADDR_UNSET;
        }
#endif
    }
    else
    {   /* NOT a top-level declaration */
        Binder *bnew;

        if (local_scope == NULL)
            syserr("instate_declaration_1 - no local scope");
	
        if ((b = findbinding_2(sv, local_scope)) != NULL)
        {
	  if (!(declflag & DUPL_OK ||
                   (bindstg_(b) & d->declstg & bitoftype_(s_extern)
#ifdef CPLUSPLUS
                 || bindstg_(b) & d->declstg & bitoftype_(s_typedef)
#endif
                   ) && equivtype(d->decltype,bindtype_(b))))
                cc_err(bind_err_duplicate_definition, sv);
            /* flag old one as referenced to avoid spurious warns:  */
            binduses_(b) |= u_referenced;
        }
	
/* AM: at some later point we may wish to check or export 'local'       */
/* extern decls for checking purposes.  At this point we must ensure    */
/* that d->decltype is globalize()d.                                    */
        bnew = mk_binder(sv, d->declstg, d->decltype);
        add_local_binder(bnew);
/* If a local extern is already bound, try to find on topbindingchain:  */
        if (b && (d->declstg & b_undef) &&
            ((d->declstg & (bitofstg_(s_extern))) ||
             ((d->declstg & (bitofstg_(s_static))) &&
               (h0_(d->decltype) == t_fnap))))
        {
/* The following lines are written to cope with curios like:            */
/*    extern int i=0; void f() { auto i; { extern int i; ...            */
/* However, note ANSI ambiguities in:                                   */
/*    typedef int i; void f() { auto int i; { extern int i; ...         */
/* and                                                                  */
/*    void g(double); void f() { extern void g(); g(1); }               */
            Binder *btop = 0, *t;
            for (t = topbindingchain; t != NULL; t = bindcdr_(t))
                if (bindsym_(t) == sv && !(binduses_(t) & u_superceded))
                    btop = t;
            if (btop)
            {   if (!equivtype(bindtype_(btop), d->decltype) ||
                        bindstg_(btop) & bitofstg_(s_typedef))
                    /* warn about above ambiguities?                    */
                    cc_rerr(bind_rerr_local_extern, sv);
/* The following lines specifically please INMOS, but are otherwise OK. */
/* Update the storage class/bindaddr field, but NOT type (ambiguity):   */
/* @@@ what about local externs to register globals (extension)?        */
                else
                {   /* Inherit old storage class:                       */
                    bnew->bindstg = bnew->bindstg & ~(STGBITS | u_loctype) |
                                    btop->bindstg & (STGBITS | u_loctype);
                    if (!(btop->bindstg & b_undef))
                    {   /* change undef extern to this-module-defd.     */
                        bnew->bindaddr.i = btop->bindaddr.i;
                        bnew->bindstg &= ~b_undef;
                    }
                }
            }
        }
	
        if (h0_(d->decltype) != t_fnap &&
            (d->declstg & bitofstg_(s_static))) {
#ifdef TARGET_HAS_BSS
/* Note that that this BSS_THRESHOLD applies in PCC mode too.           */
            if ( (d->declstg & b_undef) &&
                 sizeoftype(d->decltype) > BSS_THRESHOLD &&
                 !(d->declstg & u_constdata) )
                maybebss = YES;
            else
#endif
                d->declstg &= ~b_undef, bnew->bindstg &= ~b_undef;
        }

#ifdef TARGET_IS_HELIOS
        b = bnew;
#endif
	
#ifdef TARGET_HAS_BSS
        b = bnew;
        if (maybebss)
        {   TypeExpr *t = bindtype_(b);
            bindaddr_(b) = addbsssym(sv, sizeoftype(t), alignoftype(t),
                                (bindstg_(b) & bitofstg_(s_static)) != 0, YES);
        }
#endif
    }

#ifdef TARGET_IS_HELIOS
    if (b == NULL)
      syserr( "unable to bind symbol '%s'\n", symname_( sv ) );
#endif
    
    /*
     * Make sure information about old definitions and previous references
     * gets carried over from the old binder to the new binder.
     * (ie. '{f();}; f(){}').
     */
#ifdef PASCAL /*ECN*/
    b->bindlevel = level;
    b->synflags = d->synflags;
#endif
    
    binduses_(b) |= olduses & u_referenced | (d->declstg & u_constdata)
      
#ifdef TARGET_HAS_BSS
/* @@@ Dec 90: when does the u_bss get removed if later init'ed?        */
                                          | (maybebss ? u_bss : 0)
#endif
                                          ;
    return b;
}

#ifndef CPLUSPLUS

ClassMember *instate_member(DeclRhsList *d, int bindflg)
{   return instate_member_1(d, bindflg);
}

Binder *instate_declaration(DeclRhsList *d, int declflag)
{   return instate_declaration_1(d, declflag);
}

#endif

/* label bindings... */

static LabBind *label_create(Symstr *id)
/* Called when a label is referenced - arranges for a check to be made   */
/* at the end of the block to ensure that the label is properly defined. */
{   LabBind *x = symlab_(id);
    if (x == 0) labelchain = symlab_(id) = x = mk_labbind(labelchain, id);
    return x;
}

LabBind *label_define(Symstr *id)
/* Called when a label is defined.  NULL return iff duplicate */
{   LabBind *x = label_create(id);
    if (x->labuses & l_defined)
    {   cc_err(bind_err_duplicate_label, id);
        return 0;
    }
    x->labuses |= l_defined;
    return x;
}

LabBind *label_reference(Symstr *id)
/* Called when a label is referenced - arranges for a check to be made   */
/* at the end of the block to ensure that the label is properly defined. */
{   LabBind *x = label_create(id);
    x->labuses |= l_referenced;
    return x;
}

void label_resolve(void)
{
    LabBind *lc;
    for (lc = labelchain; lc!=NULL; lc = lc->labcdr)
    {   Symstr *id = lc->labsym;
        symlab_(id) = NULL;
        if (!(lc->labuses & l_defined))
            cc_err(bind_err_unset_label, id);
/* NB the CG or SEM should ignore goto's to label 0 (undef'd). */
        if (!(lc->labuses & l_referenced))
            cc_warn(bind_warn_label_not_used, id);
    }
    labelchain = NULL;
}

void bind_cleanup(void)
/* see comment on unbindlocals */
{   TagBinder *p;
    Binder *b;

    for (p = toptagbindchain; p != 0; p = tagbindcdr_(p))
    {   Symstr *sv = bindsym_(p);
        if (debugging(DEBUG_BIND))
            cc_msg("top struct unbind $r of %p\n",
                sv, (VoidStar)tag_global_(sv));
        tag_global_(sv) = 0;             /* restore previous binding */
        /* warning on undefined struct/union now removed */
        /*if (!(attributes_(b) & TB_DEFD) && !(suppress & D_STRUCTWARN))
            { cc_warn(bind_rerr_undefined_tag, tagbindsort(p), p); }
         */
    }
    toptagbindchain = 0;             /* just for tidyness */

    check_for_incomplete_tentative_defs(
        (TentativeDefn *) dreverse((List *)tentative_defs));

    for (b = topbindingchain; b != 0; b = bindcdr_(b))
    {   Symstr *sv = bindsym_(b);
        bind_global_(sv) = 0;            /* restore previous binding */
        if (!(binduses_(b) & u_superceded))
        /* tidy up the next code adding detail */
        {
            if (bindstg_(b) & bitofstg_(s_static))
            {   if (binduses_(b) & b_undef)
                {
                    if (bindstg_(b)&b_fnconst)
                        cc_pccwarn(bind_err_undefined_static, b);
                }
                else if (!(binduses_(b) & u_referenced)
#ifdef TARGET_IS_HELIOS
			 /* since library and kernel statics can extend across multiple files */
			 && (suppress_module == 0)
#endif
			 )
                    cc_warn(bind_warn_static_not_used, b);
            }
            else if (!(feature & FEATURE_NOUSE));
            else if (bindstg_(b) & bitofstg_(s_typedef));
            else if ((binduses_(b) & u_referenced) == 0)
                    cc_warn(bind_warn_not_used(0, b->bindstg & b_fnconst, b));
        }
    }
    topbindingchain = 0;             /* just for tidyness */
}

/* AM does not believe init_sym_name_table should be in this file. How  */
/* about its user (misc.c) or lex.c?                                    */
static void init_sym_name_table(void)
{   /* add entries for error messages for non-reserved words, e.g. block */
    /* (currently) non-table driven... */
    sym_name_table[s_error]       = errname_error;       /* <previous error> */
    sym_name_table[s_invisible]   = errname_invisible;   /* <invisible> */
    sym_name_table[s_let]         = errname_let;         /* <let> */
    sym_name_table[s_character]   = errname_character;   /* <character constant> */
    sym_name_table[s_wcharacter]  = errname_wcharacter;  /* <wide character constant> */
    sym_name_table[s_integer]     = errname_integer;     /* <integer constant> */
    sym_name_table[s_floatcon]    = errname_floatcon;    /* <floating constant> */
    sym_name_table[s_string]      = errname_string;      /* <string constant> */
    sym_name_table[s_wstring]     = errname_wstring;     /* <wide string constant> */
    sym_name_table[s_identifier]  = errname_identifier;  /* <identifier> */
    sym_name_table[s_binder]      = errname_binder;      /* <variable> */
    sym_name_table[s_tagbind]     = errname_tagbind;     /* <struct/union tag> */
    sym_name_table[s_cond]        = errname_cond;        /* _?_:_ */
    sym_name_table[s_displace]    = errname_displace;    /* ++ or -- */
    sym_name_table[s_postinc]     = errname_postinc;     /* ++ */
    sym_name_table[s_postdec]     = errname_postdec;     /* -- */
    sym_name_table[s_arrow]       = errname_arrow;       /* -> */
#ifdef CPLUSPLUS
    sym_name_table[s_arrowstar]   = "->*";
    sym_name_table[s_dotstar]     = ".*";
#endif
    sym_name_table[s_addrof]      = errname_addrof;      /* unary & */
    sym_name_table[s_content]     = errname_content;     /* unary * */
    sym_name_table[s_monplus]     = errname_monplus;     /* unary + */
    sym_name_table[s_neg]         = errname_neg;         /* unary - */
    sym_name_table[s_fnap]        = errname_fnap;        /* <function argument> */
    sym_name_table[s_subscript]   = errname_subscript;   /* <subscript> */
    sym_name_table[s_cast]        = errname_cast;        /* <cast> */
    sym_name_table[s_sizeoftype]  = errname_sizeoftype;  /* sizeof */
    sym_name_table[s_sizeofexpr]  = errname_sizeofexpr;  /* sizeof */
    sym_name_table[s_ptrdiff]     = errname_ptrdiff;     /* - */   /*  for (a-b)=c msg */
    sym_name_table[s_endcase]     = errname_endcase;     /* break */
    sym_name_table[s_block]       = errname_block;       /* <block> */
    sym_name_table[s_decl]        = errname_decl;        /* decl */
    sym_name_table[s_fndef]       = errname_fndef;       /* fndef */
    sym_name_table[s_typespec]    = errname_typespec;    /* typespec */
    sym_name_table[s_typedefname] = errname_typedefname; /* typedefname */
#ifdef EXTENSION_VALOF
    sym_name_table[s_valof]       = errname_valof;       /* valof */
#endif
    sym_name_table[s_ellipsis]    = errname_ellipsis;    /* ... */
    sym_name_table[s_eol]         = errname_eol;         /* \\n */
    sym_name_table[s_eof]         = errname_eof;         /* <eof> */
#ifdef RANGECHECK_SUPPORTED
    sym_name_table[s_rangecheck]  = errname_rangecheck;  /* <rangecheck> */
    sym_name_table[s_checknot]    = errname_checknot;    /* <check> */
#endif
}

void bind_init(void)
{   int i;
    topbindingchain = 0, toptagbindchain = 0, labelchain = 0;
    freeScopes = local_scope = NULL;
    tag_found_in_local_scope = NO;
    scope_level = 0;
    glob_typeexpr_memo = 0;
    tentative_defs = 0;
    saved_vg_state.size = 0;
    gensymline = gensymgen = 0;
    hashvec = (Symstr *((*)[BIND_HASHSIZE]))
                  GlobAlloc(SU_Other, sizeof(*hashvec));
    for (i = 0; i < BIND_HASHSIZE; i++) (*hashvec)[i] = NULL;
    /* This initialisation MUST precede the initialisation of lex... */
    for (i = 0; i < s_NUMSYMS; i++) sym_name_table[i] = errname_unset;
    init_sym_name_table();  /* language independent inits... */
}

/* end of bind.c */
