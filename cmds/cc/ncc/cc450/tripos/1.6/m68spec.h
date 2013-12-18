/* C compiler file m68spec.h :  Copyright (C) A.Mycroft and A.C.Norman */
/* Modified from armspec.h v 6b by C.Selwyn */
/* version 1a */

#ifndef _M68SPEC_LOADED

#define _M68SPEC_LOADED 1

#if defined TRIPOS_OBJECTS || defined COMPILING_ON_TRIPOS
/* The Tripos linker only regards 7 chars as unique so this file just
   renames all of routines which are not unique in those chars.
*/
# include "aliases.h"
#endif
#ifdef HELIOS
extern int32 suppress_module;
#endif
/* sorry, yet another file.  The explanation is that I want
   M68ops.h only to be accessed by M68gen.c, M68asm.c and M68obj.c
   to ease machine independence.
*/

/* #define TARGET_HAS_SCALED_ADDRESSING 1 */
/* #define TARGET_LACKS_MULDIV_LITERALS 1 */
/* #define TARGET_HAS_NONFORTRAN_DIVIDE 1 */
/* #define TARGET_LACKS_RIGHTSHIFT 1      */
/* #define TARGET_HAS_COND_EXEC 1         */
/* #define TARGET_HAS_SLOW_FP_COND_EXEC 1 */
/* #define TARGET_ALIGNS_DOUBLES 1        */
/* #define TARGET_SHARES_INTEGER_AND_FP_REGISTERS 1 */
/* #define TARGET_HAS_OTHER_IEEE_ORDER 1  */
/* #define TARGET_COUNT_IS_PROC 1         */

#ifdef TARGET_IS_68000
#  define SOFTWARE_FLOATING_POINT 1
#  define TARGET_LACKS_UNSIGNED_FIX 1
#else /* MC68020 */
#  define TARGET_HAS_SCALED_ADDRESSING 1
#  define target_scalable(n,m) ((n)<=3)
#  define TARGET_HAS_MULTIPLY 1
#  define TARGET_HAS_IEEE 1
#  define TARGET_HAS_DIVIDE 1
#endif

#ifdef AMIGA
#define ALIGN2
#endif

#ifdef HELIOS
#  define UNIQUE_DATASEG_NAMES
#  define GLOBAL_SOURCE_NAME
#endif
#define NO_DOLLARS_IN_BUILTINS 1
#define TARGET_HAS_BLOCKMOVE 1
#define TARGET_HAS_HALFWORD_STORE 1
#define TARGET_HAS_FP_LITERALS 1
#define TARGET_HAS_2ADDRESS_CODE 1
#define TARGET_HAS_TAILCALL 1
#define TARGET_HAS_SWITCH_BRANCHTABLE 1
#define TARGET_HAS_AOUT 1
#define TARGET_STACKS_LINK 1
#define TARGET_HAS_HALFWORD_INSTRUCTIONS 1
#define TARGET_LINKER_OMITS_DOLLAR 1
#define CGS_ADDRESS_REG_STUFF 1

#define mcdep_fixupshift(n) ((n) & 0x1fL) /* what 68000 does with big/-ve shifts */

/* help please on the 0-1 controversy */
#define R_A1 0L
#define R_V1 4L
#define R_F0 16L
#define R_ADDR0 (R_V1+3L)

#define NARGREGS 4L
#ifdef HELIOS
#define NVARREGS 7L
#else
#define NVARREGS 8L
#endif
#define NINTREGS 16L         /* same as R_F0 always? */
#define NFLTARGREGS 4L
#define NFLTVARREGS 4L
#ifdef HELIOS
#define NADDRREGS 4L
#else
#define NADDRREGS 5L
#endif

#define M_ARGREGS (regbit(R_A1+NARGREGS)-regbit(R_A1))
#define M_VARREGS (regbit(R_V1+NVARREGS)-regbit(R_V1))
#define M_INTREGS (M_VARREGS | M_ARGREGS | regbit(R_IP) /* | regbit(R_LR) */ )
#define M_FARGREGS (regbit(R_F0+NFLTARGREGS)-regbit(R_F0))
#define M_FVARREGS \
        (regbit(R_F0+NFLTVARREGS+NFLTARGREGS)-regbit(R_F0+NFLTARGREGS))
#define M_FLTREGS  (regbit(R_F0+NFLTVARREGS+NFLTARGREGS)-regbit(R_F0))
#define M_ADDRREGS (regbit(R_ADDR0+NADDRREGS)-regbit(R_ADDR0))

/* The following line is pretty suspect - it defines an order for        */
/* looking at registers and is intended to interact well with the copy-  */
/* avoidance code. When the copy-avoidance code is better this may not   */
/* be needed any more.
                                                   */
/* #define ALLOCATION_ORDER { 255 } */

#define R_IP 7          /* R_IP is A1 */
#endif
/* end of armspec.h */
