#ifdef USE_NORCROFT_PRAGMAS
#pragma force_top_level
#pragma include_only_once
#endif
/*
 * cfe/sem.h: semantic analysis phase of C compiler
 * Copyright (C) Acorn Computers Ltd., 1988
 * Copyright (C) Codemist Ltd., 1988
 */

/*
 * RCS $Revision: 1.1 $
 * Checkin $Date: 1992/03/23 15:10:26 $
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
#define primtype_(m)  mk_typeexpr1(s_typespec, (void *)(m), 0)
#define isprimtype_(x,s) (h0_(x) == s_typespec && \
                          (typespecmap_(x) & bitoftype_(s)))
#define isstructuniontype_(x)  (h0_(x) == s_typespec && \
               (typespecmap_(x) & (bitoftype_(s_struct)|bitoftype_(s_union))))
#define isfntype(x)   (h0_(princtype(x)) == t_fnap)

extern Expr *errornode;

extern bool lsbitfirst;   /* ordering for bitfields within word */

extern void typeclash(AEop op);

extern bool issimplevalue(Expr *e);
extern bool issimplelvalue(Expr *e);

extern TypeExpr *prunetype(TypeExpr *t), *princtype(TypeExpr *t);
extern SET_BITMAP qualifiersoftype(TypeExpr *t);

extern bool isvolatile_type(TypeExpr *x);
extern bool isbitfield_type(TypeExpr *x);

extern TypeExpr *typeofexpr(Expr *x);

typedef struct {
    /* Return values from structfield ... */
    int32 woffset,   /* offset (bytes) of word containing current field */
          boffset,   /* offset (bits) of start of field in word */
          bsize,     /* size (bits) of field - 0 if not a bitfield */
          typesize;  /* in which case this is the size (bytes) */
    /* Internal fields for structfield's use - caller of structfield should
       set them to zero before the first call for a structure.
     */
    int32 n,
          bitoff,
          padbits;
} StructPos;

extern void structfield(TagMemList *l, AEop sort, StructPos *p);
/* Used to iterate over the fields of a structure (union).  Returns fields
   in p describing the head field in l, and updates the internal fields
   in p to point past that element.
 */

extern int32 alignoftype(TypeExpr *x);

extern int32 sizeoftype(TypeExpr *x);

extern bool equivtype(TypeExpr *t1, TypeExpr *t2);

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

extern Expr *mkassign(AEop op, Expr *a, Expr *b);

extern Expr *mkbinary(AEop op, Expr *a, Expr *b);

extern Expr *mkfnap(Expr *e, ExprList *l);

extern Expr *mkcast(AEop op, Expr *e, TypeExpr *tr);

extern Expr *mkfieldselector(AEop op, Expr *e, Symstr *sv);

extern Expr *mkcond(Expr *a, Expr *b, Expr *c);

extern void sem_init(void);

#endif

/* end of cfe/sem.h */
