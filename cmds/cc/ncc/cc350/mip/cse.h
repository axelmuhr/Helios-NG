#ifdef USE_NORCROFT_PRAGMAS
#pragma force_top_level
#pragma include_only_once
#endif
/*
 * cse.h: Common sub-expression elimination
 * Copyright (C) Acorn Computers Ltd., 1988.
 */

/*
 * RCS $Revision: 1.1 $
 * Checkin $Date: 1992/03/23 15:01:22 $
 * Revising $Author: nickc $
 */

/*
 * This header describes the interface provided by CSE and LoopOpt combined
 * (eventually, just CSE as LoopOpt is subsumed).
 */

#ifndef _cse_h
#define _cse_h 1

#ifndef _defs_LOADED
#  include "defs.h"
#endif
#ifndef _cgdefs_LOADED
#  include "cgdefs.h"
#endif

extern BindList *cse_eliminate(void);

extern void cse_init(void);

extern void cse_tidy(void);

#ifdef CG_FINDS_LOOPS
extern void note_loop(BlockList *b, BlockHead *c);
#endif

extern void cse_reinit(void);

#endif
