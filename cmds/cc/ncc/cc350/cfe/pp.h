#ifdef USE_NORCROFT_PRAGMAS
#pragma force_top_level
#pragma include_only_once
#endif
/*
 * C pre-processor, cfe/pp.h:
 * Copyright (C) Acorn Computers Ltd., 1988
 * Copyright (C) Codemist Ltd., 1988.
 */

/*
 * RCS $Revision: 1.1 $
 * Checkin $Date: 1992/03/23 15:10:26 $
 * Revising $Author: nickc $
 */

#ifndef _pp_h
#define _pp_h

/*
 * Joint C/Pascal/F77 pre-processor.
 * The pre-processor is essentially to ANSI Draft Standard for C, but
 * with pcc (cpp) compatibility which can be turned on at run-time.
 * Language specifics are configured in pp.c and are in flux. Note that
 * in pcc-C, Pascal, and Fortran, pre-processing directives are recognised
 * only if the '#' is in column 1. Currently, no macro-replacement is
 * done in Pascal or Fortran.
 */

/* @@@ L_QUOT is not used in C and it is far from clear that pp.h ought */
/* to be exporting it anyway!                                           */
#ifdef PASCAL /*ECN*/
#define L_QUOT '\''
#else
#define L_QUOT '\"'
#endif

extern int pp_inhashif;      /* 0 => not evaluating a #if <const-expr> */
                             /* 1 => evaluating such 'for real'        */
                             /* 2 => evaluating such in skipped text   */
extern FILE *pp_cis;

#ifndef NO_LISTING_OUTPUT
/*
 * @@@ LDS. Currently this stuff defies explanation. HELP please!
 */
extern bool list_this_file;
extern bool map_init(FILE *mapstream);
#endif

extern int pp_nextchar(void);
/*
 * Return the next character from the input, after processing #includes,
 * #ifxxxs, macro-replacements and (other than for Fortran) comment removal.
 */

extern void pp_predefine(char *s);
/*
 * Define the pre-processor symbol 's' to be 1.
 */

extern void pp_preundefine(char *s);
/*
 * Remove any built-in definition of pre-processor symbol 's'.
 */

extern void pp_tidyup(void);
/*
 * A finalisation procedure for the pre-processor.
 * If feature PP_NOUSE is enabled then unused pre-processor macros are
 * reported on. If listing output is enabled and there is an open listing
 * file then the listing file is finalised (but not closed). If pre-processor
 * debugging is enabled (ENABLE_PP) then summary statistics are output.
 */

extern void pp_init(void);
/*
 * Initialise the preprocessor, which has to be done AFTER initialising the
 * storage manager. The builtinn pre-defines are manufactured at this instant
 * too (e.g. __DATE__ and __TIME__). Effectively, this is called once for each
 * file named on the compiler's command line.
 */

extern void pp_notesource(char *filename, bool is_preinclude);
/*
 * @@@ LDS. Not clear that this is in the right place. It's here because the
 * support for profile listings is built in to the pre-processor, but it leads
 * to cyclic dependencies with driver.c, which contains the host-specific file
 * name mapping code.
 * @@@ AM. The 2nd argument is only temporary -- apologies if this upsets.
 */

extern void pp_push_include(char *fname, int lquote);
/*
 * Support for other languages such as F77. 'fname' is the name of a file
 * to continue reading from; lquote is assumed to be one of <, " or '.
 * The matching rquote is computes as lquote == '<' ? '>' : lquote.
 */

#ifdef FORTRAN
extern void pp_pop_include(void);
#endif

#endif

/* end of cfe/pp.h */
