#pragma force_top_level
#pragma include_only_once
/*
 * C pre-processor, cfe/pp.h:
 * Copyright (C) Acorn Computers Ltd., 1988-1990.
 * Copyright (C) Codemist Ltd., 1988-1992.
 * Copyright (C) Advanced RISC Machines Limited, 1990-1992.
 */

/*
 * RCS $Revision: 1.1 $
 * Checkin $Date: 1993/07/14 14:06:32 $
 * Revising $Author: nickc $
 */

#ifndef _pp_h
#define _pp_h

#ifndef _defs_LOADED
#  include "defs.h"
#endif

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
#ifndef NO_LISTING_OUTPUT
/*
 * @@@ LDS. Currently this stuff defies explanation. HELP please!
 */
extern bool list_this_file;
extern bool map_init(FILE *mapstream);
#endif

#define PP_EOF  (-256)  /* Can't be confused with a signed/unsigned char */

extern int pp_nextchar(void);
/*
 * Return the next character from the input, after processing #includes,
 * #ifxxxs, macro-replacements and (other than for Fortran) comment removal.
 * Return PP_EOF at end of file.
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

extern void pp_init(FileLine *fl);
/*
 * Initialise the preprocessor, which has to be done AFTER initialising the
 * storage manager. The builtinn pre-defines are manufactured at this instant
 * too (e.g. __DATE__ and __TIME__). Effectively, this is called once for each
 * file named on the compiler's command line.
 */

extern void pp_notesource(char *filename, FILE *stream);
/*
 * Called before processing a pre-included file (stream != stdin) or
 * before each top-level file (stream == stdin).
 */

extern void pp_push_include(char *fname, int lquote);
/*
 * Support for other languages such as F77. 'fname' is the name of a file
 * to continue reading from; lquote is assumed to be one of <, " or '.
 * The matching rquote is computes as lquote == '<' ? '>' : lquote.
 */

#ifndef NO_INSTORE_FILES

extern FILE *open_builtin_header(char *name, bool *sf);
/*
 * Return a stream to a builtin header file and whether the file was opened
 * as a system file or not.
 */
#endif

/*
 * The following functions are defined by pp but are implemented elsewhere -
 * currently in mip/compiler.c. Thus they are part of pp's interface.
 */

extern FILE *pp_inclopen(char *name,
    bool is_system, bool *system, char **filename);
/*
 * Open the file called 'name'; is_system is true if the source form is
 * <file> rather than "file". 
 * Returns: the opened file;
 *          system = is_system && file found on the system search path;
 *          filename = the host name of the opened file.
 */

extern void pp_inclclose(FileLine fl);
/*
 * Close and adjust the search path.
 */

#ifdef FORTRAN
extern void pp_pop_include(void);
#endif

#endif

/* end of cfe/pp.h */
