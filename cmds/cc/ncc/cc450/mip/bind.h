/*
 * mip/bind.h:
 * Copyright (C) Acorn Computers Ltd., 1988-1990.
 * Copyright (C) Codemist Ltd., 1987-1992.
 * Copyright (C) Advanced RISC Machines Limited, 1991-1992.
 */

/*
 * RCS $Revision: 1.2 $
 * Checkin $Date: 1993/07/27 17:21:48 $
 * Revising $Author: nickc $
 */

#ifndef _bind_h
#define _bind_h

#ifndef _defs_LOADED
#  include "defs.h"
#endif

/* The next three lines interface syn.c and bind.c: see syn.c for more.  */
#define DUPL_OK            0x001
#define TOPLEVEL           0x002      /* Global lexical scope and global */
                                      /* storage scope...                */
#define GLOBALSCOPE        0x004      /* Not global lexical scope, but   */
                                      /* global storage scope for e.g.   */
                                      /* types defined in argument lists */
#define LOCALSCOPE         0x000      /* Local lexical and storage scope */

#define DECL_CTXT          0x001      /* these must avoid bitoftype_(x)  */
#define NO_CTXT            0x002      /* for x = s_const, s_volatile     */

extern char *sym_name_table[];  /* translation back to strings  */

#define SYM_LOCAL          0
#define SYM_GLOBAL         1

extern Symstr *sym_lookup(char *name, int glo);

extern Symstr *sym_insert(char *name, AEop type);

extern Symstr *sym_insert_id(char *name);

extern Symstr *gensymval(bool glo);

extern bool isgensym(Symstr *sym);

/* temporary home, pending demise... */
extern int32 evaluate(Expr *a);

extern Binder *global_mk_binder(Binder *b, Symstr *c, SET_BITMAP d,
                                TypeExpr *e);
extern Binder *mk_binder(Symstr *c, SET_BITMAP d, TypeExpr *e);
extern TagBinder *global_mk_tagbinder(TagBinder *b, Symstr *c, AEop d);
extern LabBind *mk_labbind(LabBind *b, Symstr *c);

#define gentempbinder(typ) \
  mk_binder(gensymval(0), bitofstg_(s_auto), typ)

#define genglobinder(typ) \
  global_mk_binder(0, gensymval(1), bitofstg_(s_static), typ)

extern void add_toplevel_binder(Binder *b);

extern Expr *globalize_int(int32 n);

/* globalize_typeexpr caches only basic types (including structs/typedefs) */
/* and pointers to things which are already cached.  Tough ched arrays/fns */
extern TypeExpr *globalize_typeexpr(TypeExpr *t);

extern TypeExpr *copy_typeexpr(TypeExpr *t);

typedef enum {
  TD_NotDef,
  TD_ContentDef,
  TD_Decl
} TagDefSort;

/* The distinction between TD_ContentDef and TD_Decl is to allow
     struct a { int x; }; struct a;
   but forbid
     struct a { int x; }; struct a { anything };
   @@@ AM: it may be subsumable into the TB_DEFD needed for C++ (q.v.)
 */

extern int push_scope(TagBinder *class_tag);

extern void pop_scope(int);

extern void pop_scope_no_check(int);

extern int unpop_scope(void);

extern Binder *findbinding(Symstr *sv);

extern TagBinder *findtagbinding(Symstr *sv);

extern ClassMember *findmember(Symstr *sv);

extern Expr *path_to_member(TagBinder *b, SET_BITMAP qualifiers,
                            Symstr *sv, TagBinder *start_scope);
/*
 * In the following, bindflg takes any combination of TOPLEVEL + GLOBALTAG.
 */
extern TagBinder *instate_tagbinding(Symstr *sv, AEop s, TagDefSort defining,
        int bindflg);

extern void instate_alias(Symstr *a, Binder *b);

extern Binder *instate_declaration(DeclRhsList *d, int declflag);

extern ClassMember *instate_member(DeclRhsList *d, int bindflg);
    /* BEWARE: Re-uses *d unless (bindflg & GLOBALTAG) */

extern void settagmems(TagBinder *b, ClassMember *l);

extern Binder *implicit_decl(Symstr *a, int32 fn);

extern void reset_vg_after_init_of_tentative_defn(void);

extern LabBind *label_define(Symstr *id);

extern LabBind *label_reference(Symstr *id);

extern void label_resolve(void);
extern void bind_cleanup(void);
extern void bind_init(void);

#ifdef CPLUSPLUS
extern ClassMember *findclassmember(Symstr *sv, TagBinder *b, bool inherit);
extern TypeExpr *memfn_realtype(TypeExpr *fntype, TagBinder *cl);
extern TagBinder *found_in_class;
extern Binder *instate_memfn(Symstr *fsv, TypeExpr *t);
extern Expr *globalize_expr(Expr *e);
extern Expr *find_cpp_binding(TypeExpr *context,
                            Symstr *sv, TagBinder *start_scope);
extern Expr *find_cpp_binding(TypeExpr *context,
                Symstr *sv, TagBinder *start_scope);
extern TagBinder *current_member_scope(void);
extern int current_scope_level(void);
extern int32 popped_bindings(void);
extern void restore_bindings(int32 handle);
extern TagBinder *set_access_context(TagBinder *cl, Binder *fn);
extern void mk_friend_class(TagBinder *classtag, TagBinder *ofclass);
extern void mk_friend_fn(Binder *bspecific, TagBinder *ofclass);
#endif

#endif

/* end of mip/bind.h */
