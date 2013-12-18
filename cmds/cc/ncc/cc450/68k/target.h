
/* C compiler file m68k/spec.h :  Copyright (C) Codemist Ltd 1988 */
/* Modified from armspec.h v 6b by C.Selwyn */
/* version 2 */

#ifndef _m68target_loaded
#define _m68target_loaded 1

#if defined TRIPOS_OBJECTS || defined COMPILING_ON_TRIPOS
/* The Tripos linker only regards 7 chars as unique so this file just
   renames all of routines which are not unique in those chars.
   This does not have to be done both on HOST and TARGET!
*/
# include "sixchar.h"
#endif

#define TARGET_IS_68000 1       /* TARGET_IS_68020 may be further defined,  */
                                /* e.g. by options.h                        */

#define TARGET_IS_HELIOS 1

#ifndef TARGET_MACHINE
#  ifdef TARGET_IS_68020
#    define TARGET_MACHINE "68020"
#  else
#    define TARGET_MACHINE "68000"
#  endif
#endif

#define TARGET_PREDEFINES { "__M68K", \
			    "__HELIOS", \
			    "__HELIOSM68K", \
                            "__CLK_TCK=100", \
                            "__JMP_BUF_SIZE=22" }

#define TARGET_HAS_AOUT	1
/* #define TARGET_HAS_BSS  1 */

#if !(defined TARGET_HAS_AOUT || defined TARGET_HAS_COFF)
#  define TARGET_HAS_COFF 1
#endif
#  define target_coff_magic 0520     /* inspected if TARGET_HAS_COFF */
#  define target_coff_prefix ""      /* inspected if TARGET_HAS_COFF */

#define TARGET_IS_BIG_ENDIAN       1
#define TARGET_HAS_SEPARATE_CODE_DATA_SEGS 1

/* #define TARGET_HAS_SCALED_ADDRESSING 1 */
/* #define TARGET_LACKS_MULDIV_LITERALS 1 */
/* #define TARGET_HAS_NONFORTRAN_DIVIDE 1 */
/* #define TARGET_LACKS_RIGHTSHIFT 1      */
/* #define TARGET_HAS_COND_EXEC 1         */
/* #define TARGET_HAS_SLOW_FP_COND_EXEC 1 */
/* #define TARGET_SHARES_INTEGER_AND_FP_REGISTERS 1 */
/* #define TARGET_HAS_OTHER_IEEE_ORDER 1  */
/* #define TARGET_COUNT_IS_PROC 1         */
#define TARGET_HAS_IEEE 1       /* both 68000 and 68020 */

#ifdef TARGET_IS_68020
#  define TARGET_HAS_SCALED_ADDRESSING 1
#  define target_scalable(n,m) ((n)<=3)
#  define TARGET_HAS_MULTIPLY 1
#  define TARGET_HAS_DIVIDE 1
#else /* MC68000 */
#  define SOFTWARE_FLOATING_POINT 1
#  define TARGET_LACKS_UNSIGNED_FIX 1
#endif

#ifdef TARGET_IS_HELIOS
#  define UNIQUE_DATASEG_NAMES
#  define GLOBAL_SOURCE_NAME            /* ??? */
#endif
#define NO_DOLLARS_IN_BUILTINS 1        /* ??? */
#define TARGET_HAS_BLOCKMOVE 1
#define TARGET_HAS_HALFWORD_STORE 1
#define TARGET_HAS_FP_LITERALS 1
#define TARGET_HAS_2ADDRESS_CODE 1
#define TARGET_HAS_SIGN_EXTEND 1
#define TARGET_HAS_TAILCALL 1
#define TARGET_HAS_SWITCH_BRANCHTABLE 1
#define TARGET_STACKS_LINK 1
#define TARGET_HAS_HALFWORD_INSTRUCTIONS 1
#define TARGET_LINKER_OMITS_DOLLAR 1
#define ADDRESS_REG_STUFF 1

#define DO_NOT_OPTIMISE_CHAR_AND_SHORT_ARITHMETIC 1
#define EXPERIMENTAL_68000

#define R_A1 0L
#define R_T1 0L
#define R_F0 16L

#ifdef ARGS_ON_STACK     /* silly temp name */
# ifdef TARGET_IS_ATARI
#  define NARGREGS 0L
#  define NTEMPREGS 2L   /* d0,a0 */
#  define R_IP 2L        /* for now we discourage regalloc from allocating */
#  define NVARREGS 9L
#  define R_V1 4L
#  define target_isaddrreg_(i) ((i) == R_T1+1 ||   \
                                R_V1+6 <= (i) && (i) < R_V1+NVARREGS)
#  define NFLTARGREGS 2L
#  define NFLTVARREGS 6L
# else
#  define NARGREGS 0L
#  define NTEMPREGS 2L   /* d0,a0 */
#  define R_IP 2L        /* for now we discourage regalloc from allocating */
#  define NVARREGS 10L
#  define R_V1 4L
#  define target_isaddrreg_(i) ((i) == R_T1+1 ||   \
                                R_V1+6 <= (i) && (i) < R_V1+NVARREGS)
#  define NFLTARGREGS 2L
#  define NFLTVARREGS 6L
# endif
#else
#  define NARGREGS 4L
#  define NTEMPREGS 0L
#  define R_IP 7          /* R_IP is A1 */  /* Shurely Shome mishtake */
#  ifdef TARGET_IS_HELIOS
#    define NVARREGS 7L
#  else
#    define NVARREGS 8L
#  endif
/* The next line has 3 since %d7 is currently not allocated.            */
#  define R_V1 4L
#  define target_isaddrreg_(i) (R_V1+3 <= (i) && (i) < R_V1+NVARREGS)
#  define NFLTARGREGS 4L
#  define NFLTVARREGS 4L
#endif

#define NINTREGS 16L     /* same as smallest fp reg, usually R_F0 */
#define MAXGLOBINTREG 7L
#define R_FV1           (R_F0+NFLTARGREGS)
#define MAXGLOBFLTREG   4L

/* The following line is pretty suspect - it defines an order for        */
/* looking at registers and is intended to interact well with the copy-  */
/* avoidance code. When the copy-avoidance code is better this may not   */
/* be needed any more.
                                                   */
/* #define ALLOCATION_ORDER { 255 } */

#ifndef sizeof_int
#  define sizeof_int        4
#endif
#ifndef alignof_int
#  define alignof_int       2       /*  COULD align to 2 not 4... */
#endif
#ifndef alignof_double
#  define alignof_double    2
#endif
#ifndef alignof_struct
#  define alignof_struct    2
#endif

#endif

/* end of m68k/target.h */
