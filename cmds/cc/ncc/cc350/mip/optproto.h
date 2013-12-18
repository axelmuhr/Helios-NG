/*
 * mip/optproto.h -- compiler configuration options set at compile time.
 * Copyright (C) Acorn Computers Ltd., 1988
 * Copyright (C) Codemist Ltd., 1989
 * Version 1.
 */

/*
 * This is a prototype file documenting (some of) the available
 * things which might go in target.h or options.h.
 * Insert all such flags in here.
 */

#error attempt to compiler optproto.h           /* ensure still sane */

#ifndef _options_LOADED
#define _options_LOADED

/*
 * Firstly set TARGET_MACHINE and TARGET_SYSTEM.  Note that you should
 * follow the general line below.
 */
#define TARGET_MACHINE          "Wombat-500"
#define TARGET_IS_WOMBAT        1
/*
 * Since host.h is #included previously we can simply make the TARGET
 * OS depend on host, e.g.
 */
#ifdef COMPILING_ON_UNIX
#    define TARGET_SYSTEM           "Unix"
#    define TARGET_IS_UNIX          1
#else
#    define TARGET_SYSTEM           "Wombat-OS"
#    define TARGET_IS_WOMBAT_OS     1
#endif

/* Now here, or more likely in target.h you specify machine properties */
/* #define TARGET_IS_BIG_ENDIAN    1 -- if your target is big endian...  */
/* #define TARGET_IS_LITTLE_ENDIAN 1 -- if your target is little endian  */
/* #define TARGET_LACKS_HALFWORD_STORE 1                                */

/* Now things defining the desired target representations.              */
/* #define alignof_int     4 -- else accept the default set in defaults.h  */
/* #define sizeof_int      4 -- else accept the default set in defaults.h  */
/* #define alignof_struct  4 -- else accept the default set in defaults.h  */
/* #define DRIVER_OPTIONS        { "-zps1", NULL } -- e.g. no stack checks */

/*
 * Now parameterisations as to facilities of the compiler you wish,
 * note that you are unlikely to need to specify many of these.
 */

/* #define EXTENSION_VALOF     1 -- to build a compiler with ACN's extension */
/* #define NO_ASSEMBLER_OUTPUT 1 -- to build a compiler sans ... capability */
/* #define NO_OBJECT_OUTPUT    1 -- to build a compiler sans ... capability */
/* #define NO_LISTING_OUTPUT   1 -- to build a compiler sans ... capability */
/* #define NO_INSTORE_FILES    1 -- to build a compiler sans in-store hdrs */
/* #define NO_DEBUGGER         1 -- to build a compiler sans debugger suppt */
/* #define NO_SELF_CHECKS      1 -- to avoid building checks aimed at itself */
#define NO_VERSION_STRINGS  1 /* -- to avoid module version strings */


#define TARGET_HAS_DIVREM_FUNCTION 1 /* divide function also returns remainder.*/
                                     /* would be in target.h, but is OS- */
                                     /* dependent too. */

/* #define REVERSE_OBJECT_BYTE_ORDER 1 -- for 'improper' cross-compilation */
/*                           -- (every word byte reversed after linking) */
/*  -- do NOT use this unless you have been condemned to a very curious  */
/* (and brain-damaged) form of cross compilation.                        */

/* ARM only things -- specify target register convention variant.        */
/* #define APCS_BINDING_A  'X' -- for the X binding of APCS (X = A,U,R,M) */
/*                           -- else accept the default set in mcdep.c */

/*
 * The following rather lengthy set of tests and definitions allows
 * various options debugging in the compiler to be selected. Selected
 * options can be controlled at run-time from the comand line.
 */
#define ENABLE_ALL          1 /* -- to enable all debugging options */
/* #define ENABLE_LEX       1 -- trace lexer token stream */
/* #define ENABLE_SYN       1 -- hint at parser operation (mostly unused) */
/* #define ENABLE_CG        1 -- trace code generation */
/* #define ENABLE_BIND      1 -- trace generation of binders */
/* #define ENABLE_TYPE      1 -- trace manipulation of type expressions */
/* #define ENABLE_REGS      1 -- debug register allocation */
/* #define ENABLE_OBJ       1 -- debug object-code formatter */
/* #define ENABLE_FNAMES    1 -- to note start of parsing each top-level fn */
/* #define ENABLE_FILES     1 -- trace finding/inclusion of source files */
/* #define ENABLE_LOOP      1 -- debug loop optimisation */
/* #define ENABLE_Q         1 -- debug generation of debugger tables */
/* #define ENABLE_STORE     1 -- report on use of store by code generator */
/* #define ENABLE_2STORE    1 -- debug compiler's storage manager */
/* #define ENABLE_SPILL     1 -- help debug register spilling */
/* #define ENABLE_MAPSTORE  1 -- enable compiler profiling */
/* #define ENABLE_AETREE    1 -- allow abstract syntax tree to be printed */
/* #define ENABLE_PP        1 -- trace pre-processor operation */
/* #define ENABLE_DATA      1 -- trace generation of static/global data */
/* #define ENABLE_CSE       1 -- debug common subexprssion elimination */
/* #define ENABLE_LOCALCG   1 -- debug local code generation (xxxgen) */

/*
 * ****** TEMPORARY HACKS ******
 */
/* #define ENABLE_X         1 -- to enable the desperate codetrace feture */

#endif /* _options_LOADED */

/* end of mip/optproto.h */
