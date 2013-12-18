#pragma force_top_level
#pragma include_only_once
/*
 * simplify.h, version 2
 * Copyright (C) Codemist Ltd, 1988-1992.
 * Copyright (C) Acorn Computers Ltd., 1988-1990.
 * Copyright (C) Advanced RISC Machines Ltd., 1990-1992.
 */

/*
 * RCS $Revision: 1.1 $
 * Checkin $Date: 1993/07/14 14:06:32 $
 * Revising $Author: nickc $
 */

#ifndef _simplify_h
#define _simplify_h

#ifndef _defs_LOADED
#  include "defs.h"
#endif

extern Expr *optimise0(Expr *e);

extern int32 mcrepofexpr(Expr *e);
extern int32 mcrepoftype(TypeExpr *t);

/* fields in mcrep result: */
#define MCR_SIZE_MASK    0x007fffff
#define MCR_SORT_MASK    0x07000000
#define MCR_ALIGN_DOUBLE 0x00800000
#define MCR_SORT_SHIFT   24
/* @@@ In the next few lines 4 is still used as the minimal stack       */
/* alignment, even if alignof_int = 2.  Rework this one day?            */
/* NB the first version of padtomcrep below subsumes the second --      */
/* merge one day, but keep separate while code munging.                 */
#if (alignof_double > alignof_int)
#  define padtomcrep(a,r) \
       padsize((a),(int32)(((r) & MCR_ALIGN_DOUBLE) ? alignof_double : 4))
#else
/* The next line has the effect of aligning locals to the same boundary */
/* as top-level variables.  All this code is a little in flux.          */
#  define padtomcrep(a,r) padsize((a), alignof_toplevel)
#endif

#endif

/* end of simplify.h */
