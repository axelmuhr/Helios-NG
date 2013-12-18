#pragma force_top_level
#pragma include_only_once
/*
 * cfe/vargen.h:
 * Copyright (C) Acorn Computers Ltd., 1988
 * Copyright (C) Codemist Ltd., 1988.
 */

/*
 * RCS $Revision: 1.1 $ Codemist 2
 * Checkin $Date: 1995/05/19 11:38:17 $
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

/* The following routine generates statics, which MUST have been instated
   with instate_declaration().  Dynamic initialistions are turned into
   assignments for rd_block() by returning the expression tree; NULL means
   no dynamic initialization. Top-level dynamic initialization code (for C++)
   is also generated in the module initialization function.

   Ensure type errors are noticed here (for line numbers etc.)
*/
extern Expr *genstaticparts(DeclRhsList * const d, bool topflag,
                            bool dummy_call);
/* @@@ since the 'const' isn't part of the function type in the line    */
/* @@@ above, AM wonders why it has been added.                         */

#ifdef CPLUSPLUS
extern TopDecl *vg_dynamic_init(void);
extern void vg_ref_dynamic_init(void);
extern void vargen_init(void);
#endif

#endif

/* end of cfe/vargen.h */
