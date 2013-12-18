#ifdef USE_NORCROFT_PRAGMAS
#pragma force_top_level
#pragma include_only_once
#endif
/*
 * mip/flowgraf.h
 * Copyright (C) Acorn Computers Ltd., 1988.
 * Copyright (C)  Codemist Ltd., 1988.
 */

/*
 * RCS $Revision: 1.1 $
 * Checkin $Date: 1992/03/23 15:01:46 $
 * Revising $Author: nickc $
 */

#ifndef _flowgraf_LOADED
#define _flowgraf_LOADED 1

#ifndef _defs_LOADED
#  include "defs.h"
#endif
#ifndef _cgdefs_LOADED
#  include "cgdefs.h"
#endif
#ifndef _jopcode_LOADED
#  include "jopcode.h"
#endif

extern BindListList *current_env;

/* statistics */
extern int32 icode_cur, block_cur;

extern BlockHead *top_block, *bottom_block;   /* exported to cg/regalloc */

extern BindList *active_binders;

extern Binder *juststored;

extern VRegnum justregister;

extern int32 sizeofbinders(BindList *l, bool countall);

extern void finishblock(void);

extern void end_emit(void);

extern Icode *newicodeblock(int32 size);

extern void freeicodeblock(Icode *p, int32 size);

extern void reopen_block(BlockHead *p);

#define start_new_basic_block(l) \
    start_basic_block_at_level(l, active_binders)

extern BlockHead *insertblockbetween(BlockHead *before, BlockHead *after);

extern BlockHead *start_basic_block_at_level(LabelNumber *l, BindList *active_on_entry);

extern bool is_exit_label(LabelNumber *ll);  /* exported to jopprint.c (lab_xname_); */

extern void emitfl(J_OPCODE op, FileLine fl);

extern void emit5(J_OPCODE op, VRegnum r1, VRegnum r2, VRegnum r3, int32 m);

extern void emitstring(J_OPCODE op, VRegnum r1, StringSegList *m);

extern void emitbranch(J_OPCODE op, LabelNumber *m);

extern void emitbinder(J_OPCODE op, VRegnum r1, Binder *m);

extern void emitvk(J_OPCODE op, VRegnum r1, int32 n, Binder *m);

extern void emitreg(J_OPCODE op, VRegnum r1, VRegnum r2, VRegnum m);

extern void emitfloat(J_OPCODE op, VRegnum r1, VRegnum r2, FloatCon *m);

extern void emitpush(J_OPCODE op, VRegnum r1, VRegnum r2, RegList *m);

extern void emitsetsp(J_OPCODE op, BindList *b2);

extern void emitsetspandjump(BindList *b2, LabelNumber *l);

extern void emitcall(J_OPCODE op, VRegnum r1, int32 argwords, Binder *m);

extern void emitcallreg(J_OPCODE op, VRegnum r1, int32 argwords, VRegnum m);

extern void emitsetspgoto(BindList *r2, LabelNumber *m);

extern void emitsetspenv(BindList *r2, BindList *m);

extern void emitcasebranch(VRegnum r1, LabelNumber **r2, int32 m);

extern void emit(J_OPCODE op, VRegnum r1, VRegnum r2, int32 m);

extern void lose_dead_code(void);

extern void linearize_code(void);

extern void flowgraph_reinit(void);

#endif

/* end of mip/flowgraf.h */
