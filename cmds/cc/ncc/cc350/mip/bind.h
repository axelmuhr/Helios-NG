#ifdef USE_NORCROFT_PRAGMAS
#pragma force_top_level
#pragma include_only_once
#endif
/*
 * mip/bind.h:
 * Copyright (C) Acorn Computers Ltd., 1988.
 * Copyright (C) Codemist Ltd., 1987.
 */

/*
 * RCS $Revision: 1.1 $
 * Checkin $Date: 1992/03/23 15:00:43 $
 * Revising $Author: nickc $
 */

#ifndef _bind_h
#define _bind_h

#ifndef _defs_LOADED
#  include "defs.h"
#endif

/* The next two lines interface syn.c and bind.c: see syn.c for more such */
#define DUPL_OK            0x001
#define TOPLEVEL           0x002

extern char *sym_name_table[];  /* translation back to strings  */

#define SYM_LOCAL      0
#define SYM_GLOBAL     1

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

extern TagBinder *mk_tagbinder(Symstr *c, AEop d);

extern LabBind *mk_labbind(LabBind *b, Symstr *c);

#define gentempbinder(typ) \
  mk_binder(gensymval(0), bitofstg_(s_auto), typ)

#define genglobinder(typ) \
  global_mk_binder(0, gensymval(1), bitofstg_(s_static), typ)

extern Expr *globalize_int(int32 n);

/* globalize_typeexpr caches only basic types (including structs/typedefs) */
/* and pointers to things which are already cached.  Tough ched arrays/fns */
extern TypeExpr *globalize_typeexpr(TypeExpr *t);

extern TypeExpr *copy_typeexpr(TypeExpr *t);

extern int32 bind_level;

extern TagBinder *findtagbinding(Symstr *sv, AEop s, bool defining);

extern TagBinder *gentagbinding(AEop s);

extern void settagmems(TagBinder *b, TagMemList *l);

extern Untagbinder *push_tagenv(void);

extern void pop_tagenv(Untagbinder *old);

extern void implicit_decl(Symstr *a, int32 fn);

extern void reset_vg_after_init_of_tentative_defn(void);

extern void instate_alias(Symstr *a, Binder *b);

extern Binder *instate_declaration(DeclRhsList *d, int declflag);

extern Unbinder *push_varenv(void);

extern void pop_varenv(Unbinder *old);

extern LabBind *label_define(Symstr *id);

extern LabBind *label_reference(Symstr *id);

extern void label_resolve(void);

extern void bind_cleanup(void);

extern void bind_init(void);

#endif

/* end of mip/bind.h */
