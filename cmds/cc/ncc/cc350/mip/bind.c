/*
 * bind.c: various binding and lexing routines for C compiler, version 32
 * Copyright (C) Codemist Ltd., 1987.
 * Copyright (C) Acorn Computers Ltd., 1988.
 */

/*
 * RCS $Revision: 1.2 $
 * Checkin $Date: 1993/11/23 17:01:51 $
 * Revising $Author: nickc $
 */

#ifndef NO_VERSION_STRINGS
extern char bind_version[];
char bind_version[] = "\nbind.c $Revision: 1.2 $ 32\n";
#endif

/* AM memo: @@@ the use of assert(0) in the development of PASCAL is    */
/* untidy but only temporary.                                           */

  #define DEBUG_TENTATIVE_DEFS 1  -- BUT ONLY during development  /* */

/* AM Dec-90: the BSS code was broken in several interesting ways.      */
/*   Mend it, and move PCC mode nearer to being ANSI code with only     */
/*   local differences.  u_tentative is now dead.                       */
/*   BSS is now merged with tentatives both in ansi and pcc mode.       */
/* AM Jul-87: experiment with memo-ising globalize_typeexpr(). */
/* bind_level rather vestigial - hack soon. */
/* exports globalize_int(), globalize_typeexpr() */

#include <stddef.h>         /* for offsetof() */
#include <string.h>
#include <ctype.h>
#include "globals.h"
#include "defs.h"
#include "aetree.h"
#include "util.h"           /* for padstrlen()... */
#include "codebuf.h"        /* for padstatic()... */
#include "cgdefs.h"         /* @@@ just for GAP */
#include "bind.h"
#include "builtin.h"
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

static int32 gensymline, gensymgen;       /* For generating unique syms   */
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
        symchain_(next) = (Symstr *)(int)DUFF_ADDR;
    symtype_(next) = s_identifier;
    strcpy(symname_(next), name);
    return(next);
}

Symstr *sym_insert(char *name, AEop type)
{
    Symstr *p = sym_lookup(name, SYM_GLOBAL);
    symtype_(p) = type;
    return(p);
}

Symstr *sym_insert_id(char *name)
{
    return sym_lookup(name, SYM_GLOBAL);
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
    sprintf(name, "Intsym_%ld", (long)++gensymgen);
#else
    if (gensymline != pp_linect)
        gensymline = pp_linect, gensymgen = 1;
    else  ++gensymgen;
    sprintf(name, "<Anon%ld_at_line_%ld>", gensymgen, gensymline);
#endif
    glo = (glo ? SYM_GLOBAL + NO_CHAIN : SYM_LOCAL + NO_CHAIN);
    return(sym_lookup(name, glo));
}

bool isgensym(Symstr *sym)
{
    return symchain_(sym) == (Symstr *)(int)DUFF_ADDR;
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

/* Allocators for use below: */

#define xxlist3(a,b,c) syn_list3((int32)(a), (int32)(b), (int32)(c))

Binder *global_mk_binder(Binder *b, Symstr *c, SET_BITMAP d,
                         TypeExpr *e)
{
    int32 size = (global_intreg_var > 0 || global_floatreg_var > 0) ?
                     sizeof(Binder) :
                     SIZEOF_NONAUTO_BINDER;
    Binder *p = (Binder*) GlobAlloc(SU_Bind, size);
 /*
  * This consistency check is removed so that front-ends for languages
  * other than C can create binders for auto variables in global store.
>>> if (d & bitofstg_(s_auto)) syserr("Odd global binder(%lx)", (long)d); <<<
  */
    if (d & bitofstg_(s_extern)) check_extern(c);
    p->h0 = s_binder;
    p->bindcdr=b;
    p->bindsym=c;
    p->bindstg=d;
    p->bindtype=e;
    p->bindaddr.i = 0;  /* soon BINDADDR_UNSET - remember 'datasegment' */
#ifdef PASCAL /*ECN*/
    p->bindlevel = 0;
    p->synflags = 0;
#endif
    return p;
}

TagBinder *global_mk_tagbinder(TagBinder *b, Symstr *c, AEop d)
{
    TagBinder *p = (TagBinder*) GlobAlloc(SU_Bind, sizeof(TagBinder));
    p->h0 = s_tagbind;
    p->bindcdr=b;
    p->bindsym=c;
    p->tagbindsort=(int)d; p->tagbindstate=0;   /* re-merge?            */
    p->tagbindmems=0;
#ifdef TARGET_HAS_DEBUGGER
    p->tagbinddbg = 0;
#endif
    return p;
}

Binder *mk_binder(Symstr *c, SET_BITMAP d, TypeExpr *e)
{
    Binder *p = (Binder*) BindAlloc((d & bitofstg_(s_auto)) ? sizeof(Binder) :
                                                    SIZEOF_NONAUTO_BINDER);
    if (d & bitofstg_(s_extern)) check_extern(c);
    p->h0 = s_binder;
    p->bindcdr=0;
    p->bindsym=c;
    p->bindstg = d | b_localstg;
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

TagBinder *mk_tagbinder(Symstr *c, AEop d)
{
    TagBinder *p = (TagBinder*) SynAlloc(sizeof(TagBinder));
    p->h0 = s_tagbind;
    p->bindcdr=0;
    p->bindsym=c;
    p->tagbindsort=(int)d; p->tagbindstate=0;   /* re-merge?            */
    p->tagbindmems=0;
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
{
    return (Expr*) global_list4(SU_Const, s_integer,te_int,n,0);
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
        if (EQtype_(t, &(p->te))) break;
    if (p == 0)
        p = glob_typeexpr_memo =
            (struct te_memo*) global_list5(SU_Type, glob_typeexpr_memo,
                         h0_(t), typespecmap_(t), typespecbind_(t), 0);
    return &(p->te);
}

/* globalize_typeexpr caches only basic types (including structs/typedefs) */
/* and pointers to things which are already cached.  Tough ched arrays/fns */
TypeExpr *globalize_typeexpr(TypeExpr *t)
{   static bool glob_incache;
    TypeExpr *ans;
#ifdef PASCAL /*ECN*/
  assert(0);
  if (0) {
#endif
    switch (h0_(t))
    {
case t_content:
        {   TypeExpr *gt = globalize_typeexpr(typearg_(t));
            if (glob_incache)
            {   TypeExpr temp;
                h0_(&temp) = s_content, typearg_(&temp) = gt,
                typeptrmap_(&temp) = typeptrmap_(t);
                /* dbglanginfo field?  Doesn't matter for C! */
                return globalize_memo(&temp);
            }
            return (TypeExpr*) global_list4(SU_Type, s_content, gt, typeptrmap_(t), 0);
            /* note that glob_incache is set correctly */
        }
case t_subscript:
            ans = (TypeExpr*) global_list4(SU_Type, s_subscript,
                                globalize_typeexpr(typearg_(t)),
                                typesubsize_(t)==0 ? 0 :
                                    globalize_int(evaluate(typesubsize_(t))),
                                0);
            glob_incache = 0;
            return ans;
case t_fnap:
            /* the DeclRhsList of formals could well become a TagMemList */
            ans = g_mkTypeExprfn(t_fnap,
                               globalize_typeexpr(typearg_(t)),
                               globalize_formals(typefnargs_(t)),
                               &typefnaux_(t));
            glob_incache = 0;
            return ans;
case s_typespec:
            /* N.B. any binder in typespecbind_(t) is assumed globalised */
            glob_incache = 1;
            return typespecmap_(t) == typespecmap_(te_int) ? te_int :
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
#ifdef PASCAL /*ECN*/
  }
#endif
}

/* @@@ This routine is a Parkes' abberation: structs cannot have        */
/* incomplete (and hence size-effectable by init) types in ANSI or PCC. */
/* Check "typedef struct { int a[]; } t; t x = {{1,2}};" first.         */
static TagMemList *copy_tagbinder(TagMemList *l)
{
#ifdef PASCAL /*ECN*/
  assert(0);
  if (0) {
#endif
    if (l!=0)
      {
        return (TagMemList*) global_list4(SU_Type, copy_tagbinder( l->memcdr ),
                              l->memsv,
                              copy_typeexpr( l->memtype ),
                              l->membits );
      }
    else
      { return 0; }
#ifdef PASCAL /*ECN*/
  }
#endif
}

/*
 *  Copy a type expression.  This is needed in vargen when types such
 *  as 'char a[];' are typedef'd.  These types must be copied because
 *  vargen side effects the array size.
 *  Nb.  Okay, I confess, I know that this routine does not copy
 *  all of the type expression.  However, it does enough for me
 *  so I am afraid if you want more you will have to hack it.  MABP.
 */
TypeExpr *copy_typeexpr(TypeExpr *t)
{
#ifdef PASCAL /*ECN*/
  assert(0);
  if (0) {
#endif
    switch (h0_(t))
    {
case t_content:
        {   TypeExpr *gt = globalize_typeexpr(typearg_(t));
            return (TypeExpr*) global_list4(SU_Type, s_content, gt, typeptrmap_(t), 0);
        }
case t_subscript:
            return (TypeExpr*) global_list4(SU_Type, s_subscript,
                                globalize_typeexpr(typearg_(t)),
                                typesubsize_(t)==0 ? 0 :
                                    globalize_int(evaluate(typesubsize_(t))),
                                0);
case t_fnap:
            /* the DeclRhsList of formals could well become a TagMemList */
            return g_mkTypeExprfn(t_fnap,
                               globalize_typeexpr(typearg_(t)),
                               globalize_formals(typefnargs_(t)),
                               &typefnaux_(t));
case s_typespec:
        {   TagBinder *b=0;
/* surely the next line should look at lsb in typespecmap_ only */
            switch( typespecmap_(t) )
              {
                case bitoftype_(s_struct):
                case bitoftype_(s_union):
                  b = gentagbinding(       /* @@@ why is this case needed? */
                        (typespecmap_(t) & bitoftype_(s_struct))
                            ? s_struct : s_union );

                  settagmems( b,
                    copy_tagbinder(
                       tagbindmems_(typespectagbind_(t)) ) );

                default:
                  return (TypeExpr*) global_list4(SU_Type, s_typespec,typespecmap_(t),b, 0);

                case bitoftype_(s_typedefname):
                  return copy_typeexpr( prunetype(t) );
              }
        }
default:
            syserr(syserr_copy_typeexpr, (VoidStar)t, (long)h0_(t));
            return t;
    }
#ifdef PASCAL /*ECN*/
  }
#endif
}

/* Binding:
   There are 5 overloading classes, of which 3 (labels, vars, struct tags)
   are bindings in the traditional sense.  All code concerning binding
   and unbinding is in this file.  Access routes are the procedures below:
     Labels:  label_xxx;
     Vars:  instate_declaration, push_varenv, pop_varenv.
     Tags:  findtagbinding, gentagbinding, settagmems, push_tagenv, pop_tagenv.
   Note that typedef names share the same binding space with variables.
   The strategy used is 'shallow binding' which associates with a hash
   table 'Symstr' a field for each overloading class which holds the
   current binding record for that identifier.  This means that finding
   the binding information status of an identifier takes unit cost.
   Previous bindings are held in one of the 'Unbinder' chains below
   which are manipulated by push_ and pop_ below.

       NOTE: this means that binding information exists only for so
       long as I am in the process of parsing - consequently it makes
       sense for the parse tree to contain references to binding records
       rather than the main symbol table entries.  Toplevel binding are
       allocated in 'global' store which is not reclaimed after each
       top-level phrase.
*/

/* AM: create a globalized Binder.  For use in rd_decl and
   implicit_decl.  Beware: its components still need globalizing.
   Possible optimisation: overwrite if already there on re-definition.
   Precondition to call: loc must not represent a local binding.
*/
#define topbind2(sv, stg, typ) \
   (symdata_(sv) = topbindingchain = \
        global_mk_binder(topbindingchain, sv, stg, typ))

static Binder *topbindingchain;                                 /* vars */
static TagBinder *toptagbindchain;                              /* tags */
static LabBind *labelchain;                                     /* labels */
static Unbinder *bind_locvars;
static Untagbinder *bind_loctags;
int32 bind_level;


/* struct/union/enum tag bindings ... */

/* these next two probably should have an arg to say whether store is locally
   allocated.  For now we will use bind_level.  Note that syn.c's TOPLEVEL
   bit is not quite right - consider "struct a { struct b {...}}".
*/
TagBinder *findtagbinding(Symstr *sv, AEop s, bool defining)
{   TagBinder *b;
    if ((b = symtag_(sv)) == 0 || defining)
    {   if (bind_level == 0)
        {   if (debugging(DEBUG_BIND))
                cc_msg("top level struct $r@%p\n", sv, (VoidStar)b);
            if (b == 0)
                /* introduction of new tag */
                symtag_(sv) = toptagbindchain =
                    global_mk_tagbinder(toptagbindchain,sv,s);
            else if (tagbindmems_(b) != 0 ||        /* re-definition */
                     tagbindstate_(b) & TB_BEINGDEFD)
                cc_err(bind_err_duplicate_tag,tagbindsort_(b),b);
        }
        else
        {   Untagbinder *u;
            if (debugging(DEBUG_BIND))
                cc_msg("local struct $r@%p\n", sv, (VoidStar)b);
            /* as in instate_decl we could avoid this loop */
            for (u = bind_loctags; u != 0; u = u->unbindcdr)
                if (u->unbindsym == sv)
                {   if (tagbindmems_(b) != 0 ||        /* re-definition */
                        tagbindstate_(b) & TB_BEINGDEFD)
                        cc_err(bind_err_duplicate_tag,tagbindsort_(b),b);
                    break;
                }
            if (u == 0 || (tagbindmems_(b) != 0 ||
                           tagbindstate_(b) & TB_BEINGDEFD))
            {   bind_loctags=(Untagbinder*)xxlist3(bind_loctags,sv,symtag_(sv));
/* bind_level == 1 refers to tags in formals: these need careful        */
/* treatment in that they are somewhat visible.  e.g. equivtype needs   */
/* to see "f(struct a { int b,c;})" differing from g of similar type.   */
                symtag_(sv) = bind_level == 1 ?
                    global_mk_tagbinder(0, sv, s) : mk_tagbinder(sv, s);
            }
        }
    }
    b = symtag_(sv);
    if (tagbindsort_(b) != s)
        cc_err(bind_err_reuse_tag, tagbindsort_(b), b, s);
    if (defining) tagbindstate_(b) |= TB_BEINGDEFD;
    return b;
}

TagBinder *gentagbinding(AEop s)
{   return findtagbinding(gensymval(1), s, 1);
}

void settagmems(TagBinder *b, TagMemList *l)
/* dying...? */
{   /* We should have already given an error on redefinition.           */
    /* The next line fixes nasties like:                                */
    /* "struct d { struct d { int a; } c; } x;"  by inhibiting the      */
    /* outer setting and so recovering to "struct d { int a; }.         */
    if (tagbindstate_(b) & TB_BEINGDEFD)
    {   tagbindstate_(b) &= ~TB_BEINGDEFD;
        tagbindmems_(b) = l;
    }
}

Untagbinder *push_tagenv(void)
{   Untagbinder *p = bind_loctags;
    if (debugging(DEBUG_BIND))
        for (; bind_loctags; bind_loctags = bind_loctags->unbindcdr)
            cc_msg("Pushing tag $r\n", bind_loctags->unbindsym);
    bind_loctags = 0;
    return p;
}

void pop_tagenv(Untagbinder *old)
/* @@@ really we want the messages to come out forward, but
   we MUST unbind backwards in case of duplicates.  Leave for now.
*/
{   Untagbinder *x = bind_loctags;
    while (x != 0)
    {   Symstr *sv = x->unbindsym;
        TagBinder *p = symtag_(sv);
        if (debugging(DEBUG_BIND))
            cc_msg("local struct unbind $r from %p to %p\n",
                   sv, (VoidStar)p, (VoidStar)x->unbindold);
        symtag_(sv) = x->unbindold;  /* restore previous binding */
/* warning on undefined struct/union now removed */
/*      if (tagbindmems_(p) == 0 && !(suppress & D_STRUCTWARN))
            { cc_warn(bind_rerr_undefined_tag, tagbindsort_(p), p); }
 */
       x = x->unbindcdr;            /* does not reclaim store   */
    }
    bind_loctags = old;
}


/* variable and typedef bindings... */

void implicit_decl(Symstr *a, int32 fn)
{
/* implicit declaration of 'extern int x' or 'extern int f()'  */
/* N.B. the information has to be generated in the global heap */
    TypeExpr *t = te_int;
    TypeExprFnAux s;
    if (fn)
        t = g_mkTypeExprfn(t_fnap, t, 0,
                packTypeExprFnAux(s, 0, 999, 0, 0, 0)); /* minargs_ */
    topbind2(a, (fn ? bitofstg_(s_extern)|b_undef|b_fnconst :
                bitofstg_(s_extern)|b_undef), t);
    binduses_(topbindingchain) |= u_implicitdef;
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
    DataInit   *datainitp;
    DataInit   *datainitq;
    int32      dataloc;
    int32      size, align;
/* When TARGET_HAS_BSS is NOT set, maybebss=1 <==> size=0.              */
    bool       maybebss;
    bool       statik;
    Symstr     *sv;
} TentativeDefn;

static TentativeDefn *tentative_defs = 0;     /* also init'd by bind_init */
static DataInit *datahead = 0,                 /* These 3 don't need to be */
                *datasplice = 0,               /* init'd - done to avoid   */
                *datatail = 0;                 /* the scourge of tentative */
static TentativeDefn saved_vg_state = {0,0,0,0,0,0};


static void save_vargen_state(TentativeDefn *td)
{
    td->datainitp = datainitp;
    td->datainitq = datainitq;
    td->dataloc   = dataloc;
}

static void restore_vargen_state(TentativeDefn *td)
{
    datainitp = td->datainitp;
    datainitq = td->datainitq;
    dataloc   = td->dataloc;
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
        td->datainitp = td->datainitq = (DataInit *)(int)DUFF_ADDR;
        td->dataloc   = 0;
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
"addTentativeDefn %lx next %lx (%s) p %lx q %lx loc %ld name %s size %ld\n",
    (int32) td, (int32) td->next, (td->next) ? symname_(td->next->sv) : "",
    (int32)datainitp, (int32)datainitq, (int32)dataloc, symname_(sv), size );
#endif
    return td->maybebss;
}

#ifdef DEBUG_TENTATIVE_DEFS

static void show_vargen_state(char *when)
{
    if (debugging(DEBUG_BIND))
    {   DataInit *tmpdataq;
        cc_msg("vargen state %s restoration:-\n", when);
        for(tmpdataq = datainitp; tmpdataq != 0; tmpdataq = tmpdataq->datacdr)
        {
            cc_msg(
                "DataInit add %lx cdr %lx rpt %ld sort %lx len %ld val %ld\n",
                (int32) tmpdataq, (int32) tmpdataq->datacdr, tmpdataq->rpt,
                tmpdataq->sort, tmpdataq->len, tmpdataq->val);
        }
        cc_msg("datainitq = %lx datainitp = %lx dataloc = %ld\n\n",
            (int32)datainitq, (int32)datainitp, (int32)dataloc);
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
                datahead = td->datainitp;
#ifdef DEBUG_TENTATIVE_DEFS
if (debugging(DEBUG_BIND)) show_vargen_state("after");
#endif
                /*
                 *  Throw away old tentative (zero) initialiser ready for
                 *  replacement by genstaticparts().
                 */
                if (datainitp == 0)
                    tmpdataq = saved_vg_state.datainitp;
                else
                {
                    tmpdataq = datainitq->datacdr;
                    datainitq->datacdr = 0;
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
        if (td->datainitq == datasplice) td->datainitq = datainitq;

    /* link new initialiser to the rest of the chain */
    datainitq->datacdr = datatail;

    /*
     * Reset all the pointers so that vargen does not realise that its
     * internal lists have been modified.
     */
    if (datahead != 0) datainitp = saved_vg_state.datainitp;

    if (datatail != 0) datainitq = saved_vg_state.datainitq;

    dataloc = saved_vg_state.dataloc;

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
    if ((bindstg_(b) ^ d->declstg) & PRINCSTGBITS)
    {   /* Linkage clash, but do not moan about stray 'extern type name's */
        if (d->declstg & (b_omitextern | bitoftype_(s_static)))
        {   /* Oldest linkage wins... */
            cc_rerr(bind_rerr_linkage_disagreement,
                    d->declname, bindstg_(b) & PRINCSTGBITS);
            /* patch d->declstg to a compatible tentative type... */
            d->declstg = (d->declstg &~ b_omitextern) ^
                         (bitoftype_(s_static) | bitoftype_(s_extern));
        }
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
 * Make the symbol a an alias for b by letting it share the same binder.
 * This curious facility is used to provide extra pseudo-local variables
 * ___first_arg and ___last_arg which share binders with the first and last
 * args in a function's definition, and which are sometimes useful when
 * implementing va_args on awkward machines.  Note that pop_varenv (qv)
 * takes a special case so that it will not moan if the symbols I will
 * actually use as aliases are unused.
 */
{
    bind_locvars = (Unbinder *) xxlist3(bind_locvars, a, symdata_(a));
    symdata_(a) = b;
    if (debugging(DEBUG_BIND)) cc_msg("Make alias $r for $b\n", a, b);
}

Binder *instate_declaration(DeclRhsList *d, int declflag)
/* only the TOPLEVEL and DUPL_OK bits of declflag are examined */
{
/* I have just parsed a declarator, and in that identifiers are held    */
/* as Symstr *'s which contain an h0 of s_identifier.  Instate the      */
/* declaration, returning the Binder record hung off the symdata_ entry.*/
    Symstr *sv = d->declname;
    Binder *b;
    int32 olduses = 0;
    bool maybebss = 0;
#ifdef PASCAL /*ECN*/
    int level = declflag >> 2;
#endif

#ifdef PASCAL /*ECN*/
    declflag &= 3;
#endif
    if (sv == 0 || h0_(sv) != s_identifier)
         { syserr(syserr_instate_decl, (long)(sv==0 ? 0 : h0_(sv)));
           return 0; }  /* check/remove*/
    if (debugging(DEBUG_BIND))
        cc_msg("instate_declaration(%x): $r\n",(int)declflag, sv);
    if (declflag&TOPLEVEL)
/* Top level declarations may only surplant previous top level extern decls */
/* Really we should also unify 'local' extern decls.  @@@ not done yet.     */
    {   TypeExpr *glotype = 0;
        b = symdata_(sv);
#ifdef PASCAL /*ECN*/
        if (b && (bindstg_(b) & b_synbit1))
            b = 0;
#endif
        if (b != 0)
        {   olduses = binduses_(b);
            /* check the types match */
            if (h0_(b->bindtype) == t_fnap && h0_(d->decltype) == t_fnap)
            {   /* propagate #pragma -v and -y info from decl to defn */
                if (typefnaux_(d->decltype).variad == 0)
                    typefnaux_(d->decltype).variad =
                        typefnaux_(b->bindtype).variad;
                if (typefnaux_(d->decltype).sideeffects == 0)
                    typefnaux_(d->decltype).sideeffects =
                        typefnaux_(b->bindtype).sideeffects;
/* @@@ old-style bit here. */
            }
            switch (equivtype(b->bindtype, d->decltype))
            {   default: cc_err(bind_err_type_disagreement, sv);
                         break;
                case 2:  glotype = b->bindtype;   /* IDENTICAL */
                case 1:  break;
            }

            /* Check for duplicate and conflicting definitions */
            if ((bindstg_(b) | d->declstg) & bitoftype_(s_typedef))
                /* can't duplicate a typedef... */
                cc_err(bind_err_duplicate_definition, sv);
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
                  (bitoftype_(s_static)|b_fnconst|bitoftype_(s_typedef))
                 ) == bitoftype_(s_static) && (d->declstg & b_undef))
            {   maybebss = addTentativeDefn(sv, sizeoftype(d->decltype),
                                   alignoftype(d->decltype),
                                   (d->declstg&bitofstg_(s_static)) != 0);
                if (!maybebss) d->declstg &= ~b_undef;
            }
        }
        else
        {   if ((b == 0 || bindstg_(b) & b_undef)                  &&
                 !(d->declstg & (b_fnconst|bitoftype_(s_typedef))) &&
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
                                   (d->declstg&bitofstg_(s_static)) != 0);
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
#ifdef TARGET_HAS_BSS
        if (maybebss)
        {   b = symdata_(sv);
            /* addbsssym() now called when we know for certain. */
            bindaddr_(b) = BINDADDR_UNSET;
        }
#endif
    }
    else
    {   /* NOT a top-level declaration */
        Unbinder *u = bind_locvars;
        Binder *bnew;
        b = symdata_(sv);               /* current binding, if any.     */
        /* a 'depth' field in each binder could avoid the next loop */
        for (; u != 0; u = u->unbindcdr)
            if (u->unbindsym == sv)
            {   /* The test for NULL on next line is just for safety,    */
                /* logically b must be bound if sv is on an unbinder!    */
                if (b == NULL ||
                    !(declflag & DUPL_OK ||
                        bindstg_(b) & d->declstg & bitoftype_(s_extern) &&
                        equivtype(d->decltype,bindtype_(b))))
                    cc_err(bind_err_duplicate_definition, sv);
                /* flag old one as referenced to avoid spurious warns:  */
                if (b != NULL) binduses_(b) |= u_referenced;
                break;
            }
        bind_locvars = (Unbinder*) xxlist3(bind_locvars, sv, symdata_(sv));
/* AM: at some later point we may wish to check or export 'local'       */
/* extern decls for checking purposes.  At this point we must ensure    */
/* that d->decltype is globalize()d.                                    */
        bnew = symdata_(sv) = mk_binder(sv, d->declstg, d->decltype);
/* If a local extern is already bound, try find on topbindingchain:     */
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
            for (t = topbindingchain; t != NULL; t = t->bindcdr)
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
                    bnew->bindstg = bnew->bindstg & ~(STGBITS | u_bss) |
                                    btop->bindstg & (STGBITS | u_bss);
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
            if ((d->declstg & b_undef) &&
                 sizeoftype(d->decltype) > BSS_THRESHOLD)
                maybebss = YES;
            else
#endif
                d->declstg &= ~b_undef, bnew->bindstg &= ~b_undef;
        }
#ifdef TARGET_HAS_BSS
        b = symdata_(sv);
        if (maybebss)
        {   TypeExpr *t = bindtype_(b);
            bindaddr_(b) = addbsssym(sv, sizeoftype(t), alignoftype(t),
                                (bindstg_(b) & bitofstg_(s_static)) != 0, YES);
        }
#endif
    }
    /*
     * Make sure information about old definitions and previous references
     * gets carried over from the old binder to the new binder.
     * (ie. '{f();}; f(){}').
     */
    b = symdata_(sv);
#ifdef PASCAL /*ECN*/
    b->bindlevel = level;
    b->synflags = d->synflags;
#endif
    binduses_(b) |= olduses & u_referenced
#ifdef TARGET_HAS_BSS
/* @@@ Dec 90: when does the u_bss get removed if later init'ed?        */
                                          | (maybebss ? u_bss : 0)
#endif
                                          ;
    return b;
}

Unbinder *push_varenv(void)
{   Unbinder *p = bind_locvars;
#ifndef PASCAL /*ECN*/
    if (debugging(DEBUG_BIND))
        for (; bind_locvars; bind_locvars = bind_locvars->unbindcdr)
            cc_msg("Pushing var $r\n", bind_locvars->unbindsym);
#endif
    bind_locvars = 0;
    return p;
}

void pop_varenv(Unbinder *old)
/* @@@ really we want the messages to come out forward, but
   we MUST unbind backwards in case of duplicates.  Leave for now.
   We could change the parameter to be a BindList - but note that
   we want to unbind enum constants which are not on BindList's.
   Also 'typedef's.
*/
{   Unbinder *x = bind_locvars;
    while (x != 0)
    {   Symstr *sv = x->unbindsym;
        Binder *p = symdata_(sv);
        symdata_(sv) = x->unbindold; /* restore previous binding */
        /* do a bit more in the next line for used/set */
        /* suppress warning for gensym'd vars, which patch up user errs */
        if ((binduses_(p) & u_referenced) == 0 && (symname_(sv)[0] != '<') &&
            sv != first_arg_sym &&
	    sv != last_arg_sym)
            cc_warn(bind_warn_not_used(bindstg_(p) & bitoftype_(s_typedef),
                                       bindstg_(p) & b_fnconst, p));
        x = x->unbindcdr;            /* does not reclaim store   */
    }
    bind_locvars = old;
}


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

    for (p = toptagbindchain; p != 0; p = p->bindcdr)
    {   Symstr *sv = bindsym_(p);
        if (debugging(DEBUG_BIND))
            cc_msg("top struct unbind $r of %p\n", sv, (VoidStar)symtag_(sv));
        symtag_(sv) = 0;             /* restore previous binding */
        /* warning on undefined struct/union now removed */
        /*if (tagbindmems_(p) == 0 && !(suppress & D_STRUCTWARN))
            { cc_warn(bind_rerr_undefined_tag, tagbindsort_(p), p); }
         */
    }
    toptagbindchain = 0;             /* just for tidyness */

    check_for_incomplete_tentative_defs(
        (TentativeDefn *) dreverse((List *)tentative_defs));

    for (b = topbindingchain; b != 0; b = b->bindcdr)
    {   Symstr *sv = bindsym_(b);
        symdata_(sv) = 0;            /* restore previous binding */
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
    bind_locvars = 0, bind_loctags = 0, bind_level = 0;
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
