/*> patchlib/h <*/
/*----------------------------------------------------------------------*/
/*				patchlib.h				*/
/*				----------				*/
/* Copyright (c) 1990, Active Book Company, Cambridge, United Kingdom.	*/
/*									*/
/* The "PatchLIB" provides a scheme for future-proofing any function	*/
/* with a small performance hit. Every function that you may want to	*/
/* replace is wrapped in special MACROs that will generate suitable	*/
/* "PatchLIB" calls.							*/
/*----------------------------------------------------------------------*/

#ifndef patchlib_h
#define patchlib_h

/*----------------------------------------------------------------------*/

#include <helios.h>	/* for Fn typedefs and standard types */

/*----------------------------------------------------------------------*/
/* In the following "p" is an ASCII string program name. "f" is an ASCII
 * string function name.
 */

/* start function:
 * This should be the first code after the "main" entry in all programs.
 * Currently this will be a NULL function. Eventually a more sophisticated
 * scheme for allowing the module table to be directly manipulated will
 * be provided.
 */
#define PATCHSTART(p)	PL_##p##_start(void) ;

/* void functions:
 * These functions can be called directly.
 */
#define PATCHVOID(f,p)	PL_##p##_##f((VoidFnPtr)f)

/* non-void functions:
 * These require return value casts on all calls.
 */
#define PATCHWORD(f,p)	PL_##p##_##f((WordFnPtr)f)

/*----------------------------------------------------------------------*/

#endif /* patchlib_h */

/*----------------------------------------------------------------------*/
/*> EOF patchlib/h <*/
