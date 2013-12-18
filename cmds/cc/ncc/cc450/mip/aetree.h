#pragma force_top_level
#pragma include_only_once
/*
 * aetree.h - AE Tree constructor and print functions.
 * Copyright (C) Acorn Computers Ltd., 1988-1990.
 * Copyright (C) Codemist Ltd., 1987-1992.
 * Copyright (C) Advanced RISC Machines Limited, 1991-1992.
 */

/*
 * RCS $Revision: 1.1 $
 * Checkin $Date: 1993/07/14 14:07:18 $
 * Revising $Author: nickc $
 */

#ifndef _aetree_h
#define _aetree_h

#ifndef _defs_LOADED
#  include "defs.h"
#endif

/* NB: it is unclear that the TypeExpr creation fns should be in this   */
/* mip file -- TypeExpr's are moving to being a cfe local (opaque)      */
/* whose interface to mip is via mcrepoftype.                           */

extern AEop tagbindsort(TagBinder *b);

extern Expr *mk_expr1(AEop op, TypeExpr *t, Expr *a1);

extern Expr *mk_expr2(AEop op, TypeExpr *t, Expr *a1, Expr *a2);

extern Expr *mk_exprlet(AEop op, TypeExpr *t, SynBindList *a1, Expr *a2);

extern Expr *mk_expr3(AEop op, TypeExpr *t, Expr *a1, Expr *a2, Expr *a3);

#ifdef EXTENSION_VALOF
extern Expr *mk_expr_valof(AEop op, TypeExpr *t, Cmd *c);
#endif

extern Expr *mk_exprwdot(AEop op, TypeExpr *t, Expr *a1, int32 a2);

extern Expr *mk_exprbdot(AEop op, TypeExpr *t, Expr *a1, int32 a2, int32 a3,
                         int32 a4);

/* mkDeclRhsList is slowly dying in C++ (syntax aid only).              */
extern DeclRhsList *mkDeclRhsList(Symstr *sv, TypeExpr *t, SET_BITMAP s);

extern TopDecl *mkTopDeclFnDef(AEop a, Binder *b, SynBindList *c,
                               Cmd *d, bool e);

extern TypeExpr *mk_typeexpr1(AEop op, TypeExpr *t, Expr *a1);
extern TypeExpr *mkTypeExprfn(AEop a, TypeExpr *b, FormTypeList *c,
                              const TypeExprFnAux *d);
extern TypeExpr *g_mkTypeExprfn(AEop a, TypeExpr *b, FormTypeList *c,
                                const TypeExprFnAux *d);

extern Cmd *mk_cmd_0(AEop op, FileLine x);
  /* op = s_break,s_endcase,s_continue */

extern Cmd *mk_cmd_e(AEop op, FileLine x, Expr *e);
  /* op = s_return,s_semicolon */

extern Cmd *mk_cmd_default(FileLine x, Cmd *c);

extern Cmd *mk_cmd_lab(AEop op, FileLine x, LabBind *b, Cmd *c);

extern Cmd *mk_cmd_block(FileLine x, SynBindList *bl, CmdList *cl);

extern Cmd *mk_cmd_do(FileLine x, Cmd *c, Expr *e);

extern Cmd *mk_cmd_if(FileLine x, Expr *e, Cmd *c1, Cmd *c2);

extern Cmd *mk_cmd_switch(FileLine x, Expr *e, Cmd *c1, Cmd *c2, Cmd *c3);

extern Cmd *mk_cmd_for(FileLine x, Expr *e1, Expr *e2, Expr *e3, Cmd *c);

extern Cmd *mk_cmd_case(FileLine x, Expr *e, Cmd *c1, Cmd *c2);

extern bool is_fpzero(Expr *e);

extern bool is_fpone(Expr *e);

extern bool is_fpminusone(Expr *e);

extern void eprintf(char *s, ...);

extern void pr_typeexpr(TypeExpr *x, Symstr *s);

extern void pr_stringsegs(StringSegList *z);

extern void pr_expr(Expr *x);

extern void pr_cmd(Cmd *c);

extern void pr_topdecl(TopDecl *x);

#endif

/* end of aetree.h */
