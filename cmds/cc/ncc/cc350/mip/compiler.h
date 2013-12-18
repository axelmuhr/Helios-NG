#ifdef USE_NORCROFT_PRAGMAS
#pragma force_top_level
#pragma include_only_once
#endif
/*
 * C compiler file compiler.h
 * Copyright (C) Acorn Computers Ltd., 1988.
 */

/*
 * RCS $Revision: 1.1 $ 111
 * Checkin $Date: 1992/03/23 15:01:04 $
 * Revising $Author: nickc $
 */

#ifndef _compiler_h
#define _compiler_h

/* The filename suffixes subject to inversion under RISC OS/MVS etc.    */
#ifdef FORTRAN
#  define FNAME_INCLUDE_SUFFIXES "f F h H"
#  define FNAME_SUFFIXES "a A f F h H o O s S"
#  define LANG_EXTN 'f'
#  define LANG_UC_EXTN 'F'
#  define LANG_EXTN_STRING "f"
#else
#ifdef PASCAL
#  define FNAME_INCLUDE_SUFFIXES "p P h H"
#  define FNAME_SUFFIXES "a A p P h H o O s S"
#  define LANG_EXTN 'p'
#  define LANG_UC_EXTN 'P'
#  define LANG_EXTN_STRING "p"
#else
#ifdef BCPL
#  define FNAME_INCLUDE_SUFFIXES "b B h H"
#  define FNAME_SUFFIXES "a A b B h H o O s S"
#  define LANG_EXTN 'b'
#  define LANG_UC_EXTN 'B'
#  define LANG_EXTN_STRING "b"
#else
#  define FNAME_INCLUDE_SUFFIXES "c C h H"
#  define FNAME_SUFFIXES "a A c C h H o O s S"
#  define LANG_EXTN 'c'
#  define LANG_UC_EXTN 'C'
#  define LANG_EXTN_STRING "c"
#endif
#endif
#endif

#include <time.h>    /* for time_t */

extern time_t tmuse_front, tmuse_back;

extern int ccom(int argc, char *argv[]);  /* must match spec for main */

extern void driver_abort(char *message);

#endif

/* end of mip/compiler.h */
