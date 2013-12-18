#ifdef USE_NORCROFT_PRAGMAS
#pragma force_top_level
#pragma include_only_once
#endif
/*
 * mip/cg.h
 * Copyright (C) Acorn Computers Ltd., 1988.
 * Copyright (C) Codemist Ltd., 1987.
 */

/*
 * RCS $Revision: 1.1 $
 * Checkin $Date: 1992/03/23 15:00:49 $
 * Revising $Author: nickc $
 */

#ifndef _cg_h
#define _cg_h 1

#ifndef _defs_LOADED
#  include "defs.h"
#endif
#ifndef _cgdefs_LOADED
#  include "cgdefs.h"
#endif

extern int32 greatest_stackdepth;   /* needed for stack check code */
extern int32 max_argsize;           /* for regalloc.c              */
extern int32 cg_fnname_offset_in_codeseg;      /* for xxx/gen.c    */

extern BindList *argument_bindlist; /* for regalloc.c              */

extern int32 procflags;             /* see jopcode.h               */
extern int32 procinitfltargs;       /* TARGET_IS_MIPS only today   */

extern bool has_main;

#ifdef CG_FINDS_LOOPS
  extern bool in_loop;                /* flowgraph.c inspects ... */

  extern BlockList *this_loop;        /* ... and chains on this   */
#endif

extern Binder *gentempvar(TypeExpr *t, VRegnum r);

extern void cg_topdecl(TopDecl *x);

extern void cg_init(void);

extern void cg_reinit(void);

extern void cg_tidy(void);

#endif

/* end of mip/cg.h */
