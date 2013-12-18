#ifdef USE_NORCROFT_PRAGMAS
#pragma force_top_level
#pragma include_only_once
#endif
/*
 * mip/defaults.h - default sizes of things, debugging options etc...
 * Copyright (C) Codemist Ltd., 1989.
 * version 6.
 */

/* This file is included after host.h, options.h and target.h and       */
/* defaults any TARGET/LANGUAGE details left undefined by these files.  */
/* Hence DO NOT MODIFY THIS FILE, but modify (e.g.) target.h instead.   */
/* Possibilities currently supported are in comments.                   */
/* Not all possibilities are supported on all targets.                  */

#ifndef _defaults_LOADED
#define _defaults_LOADED 1

#ifndef TARGET_MACHINE
  #error target.h did not specify TARGET_MACHINE
#endif
#ifndef TARGET_SYSTEM
  #error options.h and target.h did not specify TARGET_SYSTEM (Unix assumed)
  #define TARGET_SYSTEM "Unix"
  #define TARGET_IS_UNIX 1
#endif

/* The following number determines the size (in bytes) threshold above  */
/* which tentative (top level) definitions are provisionally allocated  */
/* to BSS.  This is only used if TARGET_HAS_BSS is set.  Note that it   */
/* may be set to 0, but this produces worse code for:                   */
/*    int a,b; f() { return a+b; }                                      */
/* which then requires two address constants for (since just one        */
/* may LATER find its way to the data segment by (e.g.) "int a=1;"      */
/* Allocating both a,b to datasegment initially allows one address      */
/* constant to suffice.                                                 */
#ifndef BSS_THRESHOLD
#  define BSS_THRESHOLD 100
#endif

#ifndef NTEMPREGS                       /* gentle upwards compatibility */
#  define NTEMPREGS 0
#  define R_T1 R_A1                     /* not used if NTEMPREGS=0.     */
#endif
#ifndef NFLTTEMPREGS                    /* gentle upwards compatibility */
#  define NFLTTEMPREGS 0
#  define R_FT1 R_FA1                   /* not used if NFLTTEMPREGS=0.  */
#endif
#ifndef NFLTREGS                        /* gentle upwards compatibility */
#  define NFLTREGS (NFLTARGREGS+NFLTVARREGS+NFLTTEMPREGS)
#endif

/* R_P1 is the first argument register seen from the callee side.       */
/* Unless on a register-windows machine R_P1 = R_A1.                    */
#ifndef R_P1
#  define R_P1 R_A1
#endif
#ifndef R_FA1
#  define R_FA1 R_F0
#endif
#ifndef R_FP1
#  define R_FP1 R_FA1
#endif
/* R_A1result and R_P1result similarly give names for the result reg.   */
#ifndef R_A1result
#  define R_A1result R_A1
#endif
#ifndef R_P1result
#  define R_P1result R_A1result
#endif
#ifndef R_FA1result
#  define R_FA1result R_FA1
#endif
#ifndef R_FP1result
#  define R_FP1result R_FA1result
#endif

#ifndef sizeof_short
#  define sizeof_short 2                /* 2, 4 */
#endif
#ifndef sizeof_int
#  define sizeof_int 4                  /* 2, 4 */
#endif
#ifndef sizeof_long
#  define sizeof_long 4                 /* 4 */
#endif
#ifndef sizeof_ptr
#  define sizeof_ptr sizeof_int         /* sizeof_int, 4 */
#endif
#ifndef sizeof_float
#  define sizeof_float 4                /* 4 */
#endif
#ifndef sizeof_double
#  define sizeof_double 8               /* 8 */
#endif
#ifndef sizeof_ldble
#  define sizeof_ldble sizeof_double    /* 8 */
#endif

#ifndef alignof_short
#  define alignof_short sizeof_short    /* 2, 4 */
#endif
#ifndef alignof_int
#  define alignof_int sizeof_int        /* 2, 4 */
#endif
#ifndef alignof_long
#  define alignof_long sizeof_long      /* 4 */
#endif
#ifndef alignof_ptr
#  define alignof_ptr alignof_int       /* alignof_int, 4 */
#endif
#ifndef alignof_float
#  define alignof_float sizeof_float    /* 4 */
#endif
#ifndef alignof_double
#  define alignof_double 4              /* 2, 4, 8 */
#endif
#ifndef alignof_ldble
#  define alignof_ldble alignof_double  /* 8 */
#endif
#ifndef alignof_struct
#  define alignof_struct 4      /* 4, but stricter member alignment wins */
#endif
#ifndef alignof_toplevel        /* somewhat a hack.             */
#  define alignof_toplevel 4    /* min alignment for named things */
#endif
#ifndef alignof_max             /* somewhat a hack.             */
#  define alignof_max alignof_double   /* a form of quantum_max */
#endif

#ifndef NUM_BITS_PER_BYTE
#define NUM_BITS_PER_BYTE	8
#endif

#ifndef MAXBITSIZE
#define MAXBITSIZE 	(NUM_BITS_PER_BYTE * sizeof_int) /* (was 32) sizeof_long too maybe one day! */
#endif

#ifndef TARGET_ADDRESSES_UNSIGNED
#  define TARGET_ADDRESSES_UNSIGNED 0   /* 0, 1 */
#endif

#ifndef TARGET_RIGHTSHIFT
#  define TARGET_RIGHTSHIFT(a,b) ((a)>>(b))     /* i.e. same as host. */
#endif

#ifndef signed_rightshift_
#  define signed_rightshift_(a,b) \
         ((int32)((int32)(a)>=0 ? (a)>>(b) : ~((~(unsigned32)(a)) >> (b))))
#endif

#ifndef TARGET_NULL_BITPATTERN
#  define TARGET_NULL_BITPATTERN 0      /* bit pattern for (void *)0. */
#endif

#ifndef TARGET_LACKS_MULDIV_LITERALS
#if defined(TARGET_LACKS_MULTIPLY_LITERALS) && defined(TARGET_LACKS_DIVIDE_LITERALS)
#define TARGET_LACKS_MULDIV_LITERALS 1
#endif
#endif

#ifdef TARGET_LACKS_MULDIV_LITERALS
#undef TARGET_LACKS_MULTIPLY_LITERALS
#define TARGET_LACKS_MULTIPLY_LITERALS 1
#undef TARGET_LACKS_DIVIDE_LITERALS
#define TARGET_LACKS_DIVIDE_LITERALS 1
#endif


#ifndef sizeof_wchar            /* wide string element type.            */
/* The following lines serve to make the compiler use wchar_t == int.   */
/* This happens works with both 16 and 32 bit target ints.              */
#  define sizeof_wchar sizeof_int
#  define te_wchar     te_int   /* for sem.c */
#  define NUM_WCHAR    NUM_INT  /* for lex.c */
#endif

#ifndef LANGUAGE
#  ifdef PASCAL
#    define LANGUAGE "Pascal"
#    define NO_INSTORE_FILES 1
#    define PASCAL_OR_FORTRAN 1
#  endif
#  ifdef FORTRAN
#    define LANGUAGE "Fortran"
#    define NO_INSTORE_FILES 1
#    define PASCAL_OR_FORTRAN 1
#  endif
#endif
#ifndef LANGUAGE
#  define LANGUAGE "C"
#endif

#ifndef DRIVER_OPTIONS
  #error options.h did not specify DRIVER_OPTIONS ('none' assumed)
/* possible example setting: { "__unix", "__manufacturer" }             */
#    define DRIVER_OPTIONS { NULL }
#endif

/*
 * Note that ENABLE_ALL does not imply ENABLE_MAPSTORE any more, since
 * the mapstore option is but rarely implemented, and not very useful
 * at that!  If needed it must be explicitly set in options.h
 */

#ifdef ENABLE_ALL
#  ifndef ENABLE_LEX
#     define ENABLE_LEX       1
#  endif
#  ifndef ENABLE_SYN
#     define ENABLE_SYN       1
#  endif
#  ifndef ENABLE_CG
#     define ENABLE_CG        1
#  endif
#  ifndef ENABLE_BIND
#     define ENABLE_BIND      1
#  endif
#  ifndef ENABLE_TYPE
#     define ENABLE_TYPE      1
#  endif
#  ifndef ENABLE_REGS
#     define ENABLE_REGS      1
#  endif
#  ifndef ENABLE_OBJ
#     define ENABLE_OBJ       1
#  endif
#  ifndef ENABLE_FNAMES
#     define ENABLE_FNAMES    1
#  endif
#  ifndef ENABLE_FILES
#     define ENABLE_FILES     1
#  endif
#  ifndef ENABLE_LOOP
#     define ENABLE_LOOP      1
#  endif
#  ifndef ENABLE_Q
#     define ENABLE_Q         1
#  endif
#  ifndef ENABLE_STORE
#     define ENABLE_STORE     1
#  endif
#  ifndef ENABLE_2STORE
#     define ENABLE_2STORE    1
#  endif
#  ifndef ENABLE_SPILL
#     define ENABLE_SPILL     1
#  endif
#  ifndef ENABLE_AETREE
#     define ENABLE_AETREE    1
#  endif
#  ifndef ENABLE_PP
#     define ENABLE_PP        1
#  endif
#  ifndef ENABLE_DATA
#     define ENABLE_DATA      1
#  endif
#  ifndef ENABLE_CSE
#     define ENABLE_CSE       1
#  endif
#  ifndef ENABLE_LOCALCG
#     define ENABLE_LOCALCG   1
#  endif
#endif

#ifdef ENABLE_LEX
#  define DEBUG_LEX     0x1L
#else
#  define DEBUG_LEX     0
#endif
#ifdef ENABLE_SYN
#  define DEBUG_SYN     0x2L
#else
#  define DEBUG_SYN     0
#endif
#ifdef ENABLE_CG
#  define DEBUG_CG      0x4L
#else
#  define DEBUG_CG      0
#endif
#ifdef ENABLE_BIND
#  define DEBUG_BIND    0x8L
#else
#  define DEBUG_BIND    0
#endif
#ifdef ENABLE_TYPE
#  define DEBUG_TYPE    0x10L
#else
#  define DEBUG_TYPE    0
#endif
#ifdef ENABLE_REGS
#  define DEBUG_REGS    0x20L
#else
#  define DEBUG_REGS    0
#endif
#ifdef ENABLE_OBJ
#  define DEBUG_OBJ     0x40L
#else
#  define DEBUG_OBJ     0
#endif
#ifdef ENABLE_FNAMES
#  define DEBUG_FNAMES  0x100L
#else
#  define DEBUG_FNAMES  0
#endif
#ifdef ENABLE_FILES
#  define DEBUG_FILES   0x200L
#else
#  define DEBUG_FILES   0
#endif
#ifdef ENABLE_LOOP
#  define DEBUG_LOOP    0x400L
#else
#  define DEBUG_LOOP    0
#endif
#ifdef ENABLE_Q
#  define DEBUG_Q       0x800L
#else
#  define DEBUG_Q       0
#endif
#ifdef ENABLE_STORE
#  define DEBUG_STORE   0x1000L
#else
#  define DEBUG_STORE   0
#endif
#ifdef ENABLE_2STORE
#  define DEBUG_2STORE  0x2000L
#else
#  define DEBUG_2STORE  0
#endif
#ifdef ENABLE_SPILL
#  define DEBUG_SPILL   0x4000L
#else
#  define DEBUG_SPILL   0
#endif
#ifdef ENABLE_MAPSTORE
#  define DEBUG_MAPSTORE  0x8000L
#else
#  define DEBUG_MAPSTORE  0
#endif
#ifdef ENABLE_AETREE
#  define DEBUG_AETREE  0x10000L
#else
#  define DEBUG_AETREE  0
#endif
#ifdef ENABLE_PP
#  define DEBUG_PP      0x20000L
#else
#  define DEBUG_PP      0
#endif
#ifdef ENABLE_DATA
#  define DEBUG_DATA    0x40000L
#else
#  define DEBUG_DATA    0
#endif
#ifdef ENABLE_CSE
#  define DEBUG_CSE     0x80000L
#else
#  define DEBUG_CSE     0
#endif
#ifdef ENABLE_LOCALCG
#  define DEBUG_LOCALCG 0x100000L
#else
#  define DEBUG_LOCALCG 0
#endif

/* The following generates no code in if (debugging(n)) if 'n' is 0 */
extern long sysdebugmask;
#define debugging(n) ((n) && sysdebugmask & (n))

/*
 * ****** TEMPORARY HACKS ******
 */
#ifdef ENABLE_X
#  define DEBUG_X       0x80L
#else
#  define DEBUG_X       0
#endif

#ifndef TARGET_IS_LITTLE_ENDIAN
#  ifndef TARGET_IS_BIG_ENDIAN
     #error Target byte ordering within word not specified - assume little endian
#    define TARGET_IS_LITTLE_ENDIAN
#  endif
#endif

/*
 * The following is intended as a general cross compilation pattern.
 * The actual translation tables are really a host dependency in the case
 * we are cross-character-set compiling, but who in the ascii world cares?
 */
#ifdef TARGET_HAS_EBCDIC
#  define char_translation(x) ('A' == 193 ? (x) : _atoe[x])
#  define char_untranslation(x) ('A' == 193 ? (x) : _etoa[x])
   extern char _atoe[], _etoa[];
#else
#  define char_translation(x) (x)
#  define char_untranslation(x) (x)
#endif

/*
 * The following is for use while the compiler is being developed,
 * and causes warnings to appear if names are not sufficiently distinct.
 */

#ifdef POLICE_THE_SIX_CHAR_NAME_LIMIT

#define TARGET_HAS_LINKER_NAME_LIMIT 1
#  define LINKER_NAME_MAX 6       /* needed if TARGET_HAS_LINKER_NAME_LIMIT */
#  define LINKER_NAME_MONOCASE 1  /* needed if TARGET_HAS_LINKER_NAME_LIMIT */

#include "sixchar.h"

#endif /* POLICE_THE_SIX_CHAR_NAME_LIMIT */

#endif

/* end of mip/defaults.h */
