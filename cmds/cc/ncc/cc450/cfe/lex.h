/*
 * C compiler file cfe/lex.h:
 * Copyright (C) Acorn Computers Ltd., 1988-1990.
 * Copyright (C) Codemist Ltd., 1988-1992.
 * Copyright (C) Advanced RISC Machines Limited, 1990-1992.
 */

/*
 * RCS $Revision: 1.2 $
 * Checkin $Date: 1993/07/27 09:29:48 $
 * Revising $Author: nickc $
 */

#ifndef _lex_h
#define _lex_h

#ifndef _defs_LOADED
#  include "defs.h"
#endif

typedef struct SymInfo {
    AEop sym;
    union { char *s; int32 i; Symstr *sv; FloatCon *fc; } a1;
    union { int32 len, flag; } a2;
    FileLine fl;
} SymInfo;

extern SymInfo curlex;          /* Current token and aux. info. */
#ifdef EXTENSION_VALOF
extern bool inside_valof_block;
#endif

extern AEop nextsym(void);
extern void ungetsym(void);     /* right inverse of nextsym */

extern int errs_on_this_sym;

extern AEop nextsym_for_hashif(void);

extern void lex_init(void);

extern void lex_beware_reinit(void);

extern void lex_reinit(void);

#ifdef CPLUSPLUS
extern int lex_savebody(void);
extern void lex_openbody(int h);
extern void lex_closebody(void);
#endif

#endif

/* end of cfe/lex.h */
