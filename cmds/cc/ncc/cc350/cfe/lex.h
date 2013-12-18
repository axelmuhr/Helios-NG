#ifdef USE_NORCROFT_PRAGMAS
#pragma force_top_level
#pragma include_only_once
#endif
/*
 * C compiler file cfe/lex.h:
 * Copyright (C) Acorn Computers Ltd., 1988
 * Copyright (C) Codemist Ltd., 1988
 */

/*
 * RCS $Revision: 1.1 $
 * Checkin $Date: 1992/03/23 15:10:26 $
 * Revising $Author: nickc $
 */

#ifndef _lex_h
#define _lex_h

#ifndef _defs_LOADED
#  include "defs.h"
#endif

extern SymInfo curlex;          /* Current token and aux. info. */
#ifdef EXTENSION_VALOF
extern bool inside_valof_block;
#endif

extern AEop nextsym(void);
extern int errs_on_this_sym;

extern AEop nextsym_for_hashif(void);

extern void lex_init(void);

extern void lex_beware_reinit(void);

extern void lex_reinit(void);

#endif

/* end of cfe/lex.h */
