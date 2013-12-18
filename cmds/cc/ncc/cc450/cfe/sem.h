#pragma force_top_level
#pragma include_only_once
/*
 * sem.c: semantic analysis phase of C compiler
 * Copyright (C) Codemist Ltd, 1988-1992.
 * Copyright (C) Acorn Computers Ltd., 1988-1990.
 * Copyright (C) Advanced RISC Machines Ltd., 1990-1992.
 */

/*
 * RCS $Revision: 1.1 $
 * Checkin $Date: 1993/07/14 14:06:32 $
 * Revising $Author: nickc $
 */

#ifndef _sem_h
#define _sem_h

#ifndef _defs_LOADED
#  include "defs.h"
#endif

#define isvolatile_expr(e) (isvolatile_type(typeofexpr(e)))
#ifdef PASCAL /*ECN*/
#define ptrtotype_(t) ((TypeExpr *)syn_list5(t_content, (t), 0, 0, \
                                                    syn_list2(s_content, 0)))
#else
#define ptrtotype_(t) mk_typeexpr1(t_content, t, 0)
#endif
/* The next line is a type-check cheat -- casts of int to (TypeExpr *) !! */
#define primtype_(m)  mk_typeexpr1(s_typespec, (TypeExpr *)(m), 0)
#define primtype2_(m,b) mk_typeexpr1(s_typespec, (TypeExpr *)(m), (Expr *)(b))
#define isprimtype_(x,s) (h0_(x) == s_typespec && \
                          (typespecmap_(x) & bitoftype_(s)))
#define isprimtypein_(x,set) (h0_(x) == s_typespec && \
                              (typespecmap_(x) & (set)))
#define isclasstype_(x)  isprimtypein_(x, CLASSBITS)
#define isfntype(x)   (h0_(princtype(x)) == t_fnap)

extern Expr *errornode;

extern void typeclash(AEop op);

extern bool issimplevalue(Expr *e);
extern bool issimplelvalue(Expr *e);

extern TypeExpr *prunetype(TypeExpr *t), *princtype(TypeExpr *t);
extern SET_BITMAP qualifiersoftype(TypeExpr *t);
extern TypeExpr *mkqualifiedtype(TypeExpr *t, SET_BITMAP qualifiers);

extern bool isvolatile_type(TypeExpr *x);
extern bool isbitfield_type(TypeExpr *x);
extern TypeExpr *unbitfield_type(TypeExpr *x);
extern bool isclassorref_type(TypeExpr *x);
extern bool pointerfree_type(TypeExpr *t);

extern TypeExpr *typeofexpr(Expr *x);

extern bool structfield(ClassMember *l, int32 sort, StructPos *p);
/* Used to iterate over the fields of a structure (union).  Returns fields
   in p describing the head field in l, and updates the internal fields
   in p to point past that element.
 */

extern int32 sizeoftypenotepadding(TypeExpr *x, bool *padded);

#define sizeoftype(t) sizeoftypenotepadding(t, NULL)

extern bool qualfree_equivtype(TypeExpr *t1, TypeExpr *t2);

extern int32 alignoftype(TypeExpr *x);

extern bool equivtype(TypeExpr *t1, TypeExpr *t2);
extern bool widened_equivtype(TypeExpr *t1, TypeExpr *t2);

extern TypeExpr *modify_formaltype(TypeExpr *t);
extern TypeExpr *widen_formaltype(TypeExpr *t);

extern Expr *mkintconst(TypeExpr *te, int32 n, Expr *e);

#ifdef SOFTWARE_FLOATING_POINT
extern Expr *fixflt(Expr *e);
#endif

extern void moan_nonconst(Expr *e, char *s);

extern Expr *mkswitch(Expr *a);
extern Expr *mktest(AEop opreason, Expr *a);
extern Expr *mkunary(AEop op, Expr *a);

/* @@@ Maybe I want to get rid of mkassign entry point in favour of     */
/* s_init which is a non-checking, void returning s_assign?             */
extern Expr *mkassign(AEop op, Expr *a, Expr *b);
extern Expr *mkbinary(AEop op, Expr *a, Expr *b);
extern Expr *mkfnap(Expr *e, ExprList *l);
extern Expr *mkcast(AEop op, Expr *e, TypeExpr *tr);

extern Expr *findfield(TypeExpr *te, Symstr *sv, TagBinder *start_scope);
extern Expr *mkdotable(AEop op, Expr *e);
extern Expr *rooted_path(Expr *path, Expr *root);
extern Expr *mkfieldselector(AEop op, Expr *e, Symstr *sv, TagBinder *scope);

extern Expr *mkcond(Expr *a, Expr *b, Expr *c);

extern void sem_init(void);

#ifdef CPLUSPLUS
Expr *mk_cppcast(TypeExpr *t, ExprList *l);
extern Expr *mkctor(Expr *nw, TypeExpr *pt, ExprList *init);
extern Expr *mkdtor(Expr *e);
extern SynBindList *sem_reftemps;
/* nasty: routines for overload.c */
extern Symstr *ovld_add_memclass(Symstr *sv, TagBinder *scope, bool staticfn);
extern Symstr *ovld_instance_name(Symstr *sv, TypeExpr *t);
extern Binder *ovld_resolve(Binder *b, BindList *alternatives,
                            ExprList *l, ExprList *ll);
extern Symstr *operator_name(AEop op);
extern Symstr *conversion_name(TypeExpr *t);
#endif

#endif

/* end of cfe/sem.h */
