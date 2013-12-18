#ifdef USE_NORCROFT_PRAGMAS
#pragma force_top_level
#pragma include_only_once
#endif
/*
 * simplify.h, version 1a
 * Copyright (C) Acorn Computers Ltd., 1988
 *
 * Copyright (C) Codemist Ltd, 1988
 */

/*
 * RCS $Revision: 1.1 $
 * Checkin $Date: 1992/03/23 15:10:26 $
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
#  define padtomcrep(a,r) padsize((a), 4)
#endif

#endif

/* end of simplify.h */
