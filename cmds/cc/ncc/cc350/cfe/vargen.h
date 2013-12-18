#ifdef USE_NORCROFT_PRAGMAS
#pragma force_top_level
#pragma include_only_once
#endif
/*
 * cfe/vargen.h:
 * Copyright (C) Acorn Computers Ltd., 1988
 * Copyright (C) Codemist Ltd., 1988.
 */

/*
 * RCS $Revision: 1.1 $
 * Checkin $Date: 1992/03/23 15:10:26 $
 * Revising $Author: nickc $
 */

#ifndef _vargen_h
#define _vargen_h

#ifndef _defs_LOADED
#  include "defs.h"
#endif

/*
 * ****** NASTY EXPORT - RECONSIDER ******
 * Should be static except for initstaticvar(datasegment) in compiler.c
 */
extern void initstaticvar(Binder *b, bool topflag);

/* The following routine removes generates statics, which MUST have been
   instated with instate_declaration().  Dynamic initialistions are turned
   into assignments for rd_block(), by return'ing.  0 means no init.
   Ensure type errors are noticed here (for line numbers etc.) */

/*
 * XXX - NC - 22/8/91
 *
 * (This change reported by NHG)
 *
 * genstaticparts() now takes a 'DeclRhsList * const' rather than a
 * 'DeclRhsList *' as previously decalred here ....
 */

extern Expr *genstaticparts(DeclRhsList * const d, bool topflag, FileLine fl);

#endif

/* end of cfe/vargen.h */
