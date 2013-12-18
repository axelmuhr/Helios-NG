#pragma force_top_level
#pragma include_only_once
/*
 * cfe/syn.h
 * Copyright (C) Acorn Computers Ltd., 1988
 * Copyright (C) Codemist Ltd., 1988-1993
 */

/*
 * RCS $Revision: 1.1 $
 * Checkin $Date: 1993/07/14 14:06:32 $
 * Revising $Author: nickc $
 */

#ifndef _syn_h
#define _syn_h

#ifndef _defs_LOADED
#  include "defs.h"
#endif

extern bool syn_hashif(void);
extern bool implicit_return_ok;
extern bool syn_undohack;

extern int32 syn_begin_agg(void);
extern void syn_end_agg(int32 beganbrace);
extern Expr *syn_rdinit(TypeExpr *t, int32 flag);
extern bool syn_canrdinit(void);

extern TopDecl *rd_topdecl(void);
extern bool syn_eof(void);
extern void syn_init(void);

#endif

/* end of cfe/syn.h */
