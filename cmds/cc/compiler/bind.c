
/* bind.c: various binding routines for C compiler */
/* Copyright (C) A.Mycroft and A.C.Norman          */
/* version 0.15 */
/* $Id: bind.c,v 1.1 1990/09/13 17:09:33 nick Exp $ */

/* AM Jul-87: experiment with memo-ising globalize_typeexpr(). */
/* bind_level rather vestigial - hack soon. */
/* exports globalize_int(), globalize_typeexpr() */

#include "AEops.h"
#include "cchdr.h"

/* comments re globalisation:
   Binders are globalized or not on creation, the only need for
   globalize routines is for the type expressions (see globalize_typeexpr()
   below) which hang from them.
*/

Expr *globalize_int(n)
int n;
{   stuse_const += 16;
    return global_list4(s_integer,te_int,n,0);
}

static Symstr *globalize_declaree1(d)
Symstr *d;
{
    if (d==(Symstr *)0) return d;
    if (h0_(d) == s_identifier)
         /* declaree within formal parameter within type.        */
         /* struct's have their own code in syn.c (to change?).  */
         return d;
     syserr("globalize_declaree1(%d,%d)", d, h0_(d));
     return d;
}

static FormTypeList *globalize_formals(d)
FormTypeList *d;
{   FormTypeList *d1;
    if (d == NULL) return NULL;
    d1 = global_list3(0,
                globalize_declaree1(d->ftname),
                globalize_typeexpr(d->fttype));
    stuse_type += sizeof(FormTypeList);
    d1->ftcdr = globalize_formals(d->ftcdr);
    return d1;
}

static struct te_memo { struct te_memo *cdr; TypeExpr te; }
    *glob_typeexpr_memo;

static TypeExpr *globalize_memo(t)
TypeExpr *t;
{   struct te_memo *p;
    for (p = glob_typeexpr_memo; p != 0; p = p->cdr)
        if (EQtype_(t, &(p->te))) break;
    if (p == 0)
        stuse_type += 16,
        p = glob_typeexpr_memo =
            global_list4(glob_typeexpr_memo,
                         h0_(t), typespecmap_(t), typespecbind_(t));
    return &(p->te);
}

/* globalize_typeexpr caches only basic types (including structs/typedefs) */
/* and pointers to things which are already cached.  Tough ched arrays/fns */
TypeExpr *globalize_typeexpr(t)
TypeExpr *t;
{   static bool glob_incache;
    TypeExpr *ans;
    switch (h0_(t))
    {
case t_content:
        {   TypeExpr *gt = globalize_typeexpr(typearg_(t));
            if (glob_incache)
            {   TypeExpr temp;
                h0_(&temp) = s_content, typearg_(&temp) = gt,
                typeptrmap_(&temp) = typeptrmap_(t);
                return globalize_memo(&temp);
            }
            stuse_type += 12;
            return global_list3(s_content, gt, typeptrmap_(t));
            /* note that glob_incache is set correctly */
        }
case t_subscript:
            stuse_type += 12;
            ans = global_list3(s_subscript,
                                globalize_typeexpr(typearg_(t)),
                                typesubsize_(t)==0 ? 0 :globalize_int(evaluate(typesubsize_(t))));
            glob_incache = 0;
            return ans;
case t_fnap:
            /* the DeclRhsList of formals could well become a TagMemList */
            stuse_type += 24;
            ans = global_list6(t_fnap,
                               globalize_typeexpr(typearg_(t)),
                               globalize_formals(typefnargs_(t)),
                               minargs_(t),
                               maxargs_(t),
                               t->variad);
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
            syserr("globalize_typeexpr(%d,%d)", t, h0_(t));
            return t;
    }
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
   Previous bindings are help in one of the 'Unbinder' chains below
   which are manipulated by push_ and pop_ below.

       NOTE: this means that binding information exists only for so
       long as I am in the process of parsing - consequently it makes
       sense for the parse tree to contain references to binding records
       rather than the main symbol table entries.  Toplevel binding are
       allocated in 'global' store which is not reclaimed after each
       top-level phrase.
*/

#define bind2(sv, stg, typ)  \
   (bind_locvars = list3(bind_locvars, sv, _symdata(sv)),              \
    _symdata(sv) = mk_binder(0, sv, stg, typ))

/* AM: create a globalized Binder.  For use in rd_decl and
   implicit_decl.  Beware: its components still need globalizing.
   Possible optimisation: overwrite if already there on re-definition.
   Precondition to call: loc must not represent a local binding.
*/
#define topbind2(sv, stg, typ) \
   (_symdata(sv) = topbindingchain = \
        global_mk_binder(topbindingchain, sv, stg, typ))

static Binder *topbindingchain;                                 /* vars */
static TagBinder *toptagbindchain;                              /* tags */
static LabBind *labelchain;                                     /* labels */
static Unbinder *bind_locvars;
static Untagbinder *bind_loctags;
int bind_level;


/* struct/union/enum tag bindings ... */

/* these next two probably should have an arg to say whether store is locally
   allocated.  For now we will use bind_level.  Note that syn.c's TOPLEVEL
   bit is not quite right - consider "struct a { struct b {...}}".
*/
TagBinder *findtagbinding(sv,s,defining)
Symstr *sv;
AEop s;
int defining;
{   TagBinder *b;
    if ((b = _symtag(sv)) == 0 || defining)
    {   if (bind_level == 0)
        {   if (debugging(DEBUG_BIND))
                eprintf("top level struct %s@%x\n", _symname(sv), b);
            if (b == 0)
                /* introduction of new tag */
                _symtag(sv) = toptagbindchain =
                    global_mk_tagbinder(toptagbindchain,sv,s);
            else if (tagbindmems_(b) != 0)  /* re-definition */
                cc_err("duplicate definition of $s tag $b",tagbindsort_(b),b);
        }
        else
        {   Untagbinder *u;
            if (debugging(DEBUG_BIND))
                eprintf("local struct %s@%x\n", _symname(sv), b);
            /* as in instate_decl we could avoid this loop */
            for (u = bind_loctags; u != 0; u = u->unbindcdr)
                if (u->unbindsym == sv)
                {   cc_err("duplicate definition of $s tag $b",
                            tagbindsort_(b),b);
                    break;
                }
            bind_loctags = list3(bind_loctags, sv, _symtag(sv));
            _symtag(sv) = mk_tagbinder(0, sv, s);
        }
    }
    b = _symtag(sv);
    if (tagbindsort_(b) != s)
        cc_err("re-using $s tag $b as $s tag", tagbindsort_(b), b, s);
    return b;
}

TagBinder *gentagbinding(s)
AEop s;
{   return findtagbinding(gensymval(1), s, 1);
}

void settagmems(b,l)
TagBinder *b;
TagMemList *l;
{   /* we should have already given a message on redefinition */
    tagbindmems_(b) = l;
}

Untagbinder *push_tagenv()
{   Untagbinder *p = bind_loctags;
    if (debugging(DEBUG_BIND))
        for (; bind_loctags; bind_loctags = bind_loctags->unbindcdr)
            eprintf("Pushing tag %s\n", _symname(bind_loctags->unbindsym));
    bind_loctags = 0;
    return p;
}

void pop_tagenv(old)
Untagbinder *old;
/* @@@ really we want the messages to come out forward, but
   we MUST unbind backwards in case of duplicates.  Leave for now.
*/
{   Untagbinder *x = bind_loctags;
    while (x != 0)
    {   Symstr *sv = x->unbindsym;
        TagBinder *p = _symtag(sv);
        if (debugging(DEBUG_BIND))
            eprintf("local struct unbind %s from %x to %x\n",
                     _symname(sv),_symtag(sv),x->unbindold);
        _symtag(sv) = x->unbindold;  /* restore previous binding */
#ifdef NOT_NICKC_HACK
        if (tagbindmems_(p) == 0)
            cc_rerr("$s tag $b not defined", tagbindsort_(p), p);
#endif
        x = x->unbindcdr;            /* does not reclaim store   */
    }
    bind_loctags = old;
}


/* variable and typedef bindings... */

void implicit_decl(a,fn)
Symstr *a;
int fn;
{
/* implicit declaration of 'extern int x' or 'extern int f()'  */
/* N.B. the information has to be generated in the global heap */
    TypeExpr *t = te_int;
    if (fn) (stuse_type += 24,
             t = global_list6(t_fnap, t, 0, 0, 999, 0)); /* minargs_ */
    topbind2(a, bitofstg_(s_extern)|b_undef, t);
    binduses_(topbindingchain) |= u_implicitdef;
}

Binder *instate_declaration(d,declflag)
DeclRhsList *d;
int declflag;
/* only the TOPLEVEL and DUPL_OK bits of declflag are examined */
{
/* I have just parsed a declarator, and in that identifiers are held    */
/* as Symstr *'s which contain an h0 of s_identifier.  Instate the      */
/* declaration, returning the Binder record hung of the _symdata entry. */
    Symstr *sv = d->declname;
    Binder *b;
    int olduses = 0;
    if (sv == 0 || h0_(sv) != s_identifier)
         { syserr("instate_decl %d", sv==0 ? 0 : h0_(sv));
           return 0; }  /* check/remove*/
    if (debugging(DEBUG_BIND))
    {   eprintf("instate_declaration(%x)\n",declflag); pr_id(sv);
        eprintf("\n");
    }
    if (declflag&TOPLEVEL)
/* Top level declarations may only surplant previous top level extern decls */
/* Really we should also unify 'local' extern decls.  @@@ not done yet.     */
    {   TypeExpr *glotype = 0;
        if ((b=_symdata(sv)) != 0)
        {   if (bindstg_(b) & b_undef)   /* old was static f() or extern a[] */
            {   olduses = binduses_(b);
                if ((bindstg_(b) ^ d->declstg) & PRINCSTGBITS)
                    cc_rerr("linkage disagreement for $r - treated as $m",
                            sv, d->declstg);
                /* The following feature, tied with same option as below */
                /* optionally enables spurious forward static            */
                /* declarations to be weeded out.                        */
                else if (feature & FEATURE_PREDECLARE && 
                         !(olduses & u_referenced) &&
                         bindstg_(b) & bitofstg_(s_static))
                    cc_warn("unused earlier static declaration of $r", sv);
                /* check the types match */
                switch (equivtype(b->bindtype, d->decltype))
                {   default: cc_err("type disagreement for $r", sv);
                             break;
                    case 2:  glotype = b->bindtype;   /* IDENTICAL */
                    case 1:  break;
                }
            }
            else cc_err("duplicate definition of $r", sv);
            binduses_(b) |= u_superceded;
        }
        else if (feature & FEATURE_PREDECLARE)
        {   /* The following is a feature to enable policing of a software */
            /* quality policy which says "only objects previously DECLARED */
            /* as extern (presumably in a header) may be DEFINED extern".  */
            if ((d->declstg & bitofstg_(s_extern)) && !(d->declstg & b_undef)
                && sv != mainsym)
                cc_warn("extern $r not declared in header", sv);
        }
        /* Maybe we wish to turn off the following for non-hosted system.  */
        if (sv == mainsym && (d->declstg & bitofstg_(s_extern)))
        {   TypeExpr *t = prunetype(d->decltype);
            /* check args here too one day? */
            if (h0_(t) != t_fnap || !equivtype(typearg_(t), te_int))
                cc_warn("extern 'main' needs to be 'int' function");
        }
        topbind2(sv, d->declstg, 
                 glotype ? glotype : globalize_typeexpr(d->decltype));
    }
    else
    {   Unbinder *u = bind_locvars;
        /* a 'depth' field in each binder could avoid the next loop */
        for (; u != 0; u = u->unbindcdr)
            if (u->unbindsym == sv)
            {   if (!(declflag & DUPL_OK))
                    cc_err("duplicate declaration of $r", sv);
                break;   /* only give message once */
            }
        bind2(sv, d->declstg, d->decltype);
    }
    b = _symdata(sv);
    binduses_(b) |= olduses & u_referenced;  /* for {f();}; f(){} */
    return b;
}

Unbinder *push_varenv()
{   Unbinder *p = bind_locvars;
    if (debugging(DEBUG_BIND))
        for (; bind_locvars; bind_locvars = bind_locvars->unbindcdr)
            eprintf("Pushing var %s\n", _symname(bind_locvars->unbindsym));
    bind_locvars = 0;
    return p;
}

void pop_varenv(old)
Unbinder *old;
/* @@@ really we want the messages to come out forward, but
   we MUST unbind backwards in case of duplicates.  Leave for now.
   We could change the parameter to be a BindList - but note that
   we want to unbind enum constants which are not on BindList's.
   Also 'typedef's.
*/
{   Unbinder *x = bind_locvars;
    while (x != 0)
    {   Symstr *sv = x->unbindsym;
        Binder *p = _symdata(sv);
        _symdata(sv) = x->unbindold; /* restore previous binding */
        /* do a bit more in the next line for used/set */
        if ((binduses_(p) & u_referenced) == 0)
            cc_warn("variable $r declared but not used", sv);
        x = x->unbindcdr;            /* does not reclaim store   */
    }
    bind_locvars = old;
}


/* label bindings... */

static LabBind *label_create(id)
Symstr *id;
/* Called when a label is referenced - arranges for a check to be made   */
/* at the end of the block to ensure that the label is properly defined. */
{   LabBind *x = _symlab(id);
    if (x == 0) labelchain = _symlab(id) = x = list4(labelchain, id, 0, 0);
    return x;
}

LabBind *label_define(id)
Symstr *id;
/* Called when a label is defined.  NULL return iff duplicate */
{   LabBind *x = label_create(id);
    if (x->labuses & l_defined)
    {   cc_err("duplicate definition of label $r - ignored", id);
        return 0;
    }
    x->labuses |= l_defined;
    return x;
}

LabBind *label_reference(id)
Symstr *id;
/* Called when a label is referenced - arranges for a check to be made   */
/* at the end of the block to ensure that the label is properly defined. */
{   LabBind *x = label_create(id);
    x->labuses |= l_referenced;
    return x;
}

void label_resolve()
{
    LabBind *lc;
    for (lc = labelchain; lc!=NULL; lc = lc->labcdr)
    {   Symstr *id = lc->labsym;
        _symlab(id) = NULL;
        if (!(lc->labuses & l_defined))
            cc_err("label $r has not been set", id);
/* NB the CG or SEM should ignore goto's to label 0 (undef'd). */
        if (!(lc->labuses & l_referenced))
            cc_warn("label $r was defined but not used", id);
    }
    labelchain = NULL;
}

void bind_cleanup()
/* see comment on unbindlocals */
{   TagBinder *p;
    Binder *b;
    for (p = toptagbindchain; p != 0; p = p->bindcdr)
    {   Symstr *sv = bindsym_(p);
        if (debugging(DEBUG_BIND))
            eprintf("top struct unbind %s of %x\n", _symname(sv),_symtag(sv));
        _symtag(sv) = 0;             /* restore previous binding */
#ifdef NOT_NICKC_HACK
        if (tagbindmems_(p) == 0)
            cc_rerr("$s tag $b not defined", tagbindsort_(p), p);
#endif
    }
    toptagbindchain = 0;             /* just for tidyness */
    for (b = topbindingchain; b != 0; b = b->bindcdr)
    {   Symstr *sv = bindsym_(b);
        _symdata(sv) = 0;            /* restore previous binding */
        if (!(binduses_(b) & u_superceded))
        /* tidy up the next code adding detail */
        {   
            if (bindstg_(b) & bitofstg_(s_static))
            {   if (binduses_(b) & b_undef)
                    cc_err("static function $b not defined - treated as extern", b);
                else if (!(binduses_(b) & u_referenced))
                    cc_warn("static $b declared but not used", b);
            }
            else if (!(feature & FEATURE_NOUSE));
            else if (bindstg_(b) & bitofstg_(s_typedef));
            else if ((binduses_(b) & u_referenced) == 0)
                cc_warn("%s $b declared but not used",
                        b->bindstg & b_fnconst ? "function": "variable", b);
        }
    }
    topbindingchain = 0;             /* just for tidyness */
}

void bind_init()
{   topbindingchain = 0, toptagbindchain = 0, labelchain = 0;
    bind_locvars = 0, bind_loctags = 0, bind_level = 0;
    glob_typeexpr_memo = 0;
}

/* end of bind.c */
