#ifdef USE_NORCROFT_PRAGMAS
#pragma force_top_level
#pragma include_only_once
#endif
/*
 * file mip/jopcode.h, version 29
 * Copyright (C) Codemist Ltd., 1989.
 */

/*
 * RCS $Revision: 1.1 $
 * Checkin $Date: 1992/03/23 15:02:05 $
 * Revising $Author: nickc $
 */

/*
 * Some bits of information now live in the joptable[] array, while things
 * like J_DEAD_R1 still live in the main code, since they need to be set
 * on a per-instruction basis.
 */

#ifndef _jopcode_LOADED
#define _jopcode_LOADED 1

#ifndef _defs_LOADED
#  include "defs.h"
#endif

/* The following are in for compatibility with old back ends only.      */
#define J_MOVFR1 J_MOVIFR
#define J_MOVDR1 J_MOVIDR
#define J_double(op) J_fltisdouble(op)

/* Opcodes in the intermediate code that I generate                       */
/* Idea -- the things beginning with '_' are local to this file, but...     */
/* ... strictly speaking this contravenes ANSI's rules on '_' use.          */
#define _J_FLOATING   0x001L
#define _J_DOUBLE     0x002L
#define _J_REGMOVE    0x004L
#define _J_ASYMDIAD   0x008L
#define _J_READ_R1    0x010L
#define _J_SET_R1     0x020L
#define _J_READ_R2    0x040L
#define _J_READ_R3    0x080L      /* r3 field is a register rather than literal */
/* the next few bits are regarded JOP adressing modes.                      */
/* N.B. read the BEWARE on LDRK before using                                */
#define _J_MEM        0x100L      /* distinguishes mem ref from op            */
#define _J_MEMA       0x300L      /* address of memory - really _J_MEM+0x200    */
/* *** N.B. but for LDRV1 we could arrange that _J_MEMA was 0x104 ***         */
#define _J_STACKREF   0x400L
#define _J_IVD        0x800L      /* indexed var + disp (only with _J_STACKREF) */
#define _J_OPTYPE     0xf000L     /* Used just for LDR/STR opcodes */
#define _J_OPTYPE_SHIFT 12L

/*
 * The above bits are for joptable[], those which follow are part of the main
 * opcode, but must not be used when indexing into joptable[].
 */

#define J_TABLE_BITS 0x7ff  /* Room for 2000 distinct jopcodes */
#define _joptable(o) (joptable[(o) & J_TABLE_BITS].bits)

#define J_VOLATILE 0x0800L      /* replacing J_USE/J_VSTORE, only LDRx/STRx */
/* N.B. The compiler currently does not represent 'volatile' on J_MOVC      */
/* because large structs are not subject to dataflow analysis.              */
/* Maybe it should, and hence maybe there should be two J_VOLATILE bits.    */
#define J_DEAD_R1  0x1000L      /* for dataflow analysis                    */
#define J_DEAD_R2  0x2000L      /* for dataflow analysis                    */
#define J_DEAD_R3  0x4000L      /* for dataflow analysis                    */
#define J_DEADBITS (J_DEAD_R1+J_DEAD_R2+J_DEAD_R3)
#ifdef TARGET_HAS_SCALED_ADDRESSING
#  define J_NEGINDEX      0x8000L /* currently or-able with load/store only */
#  define J_SHIFTMASK   0xff0000L
#  define J_SHIFTPOS          16
#    define SHIFT_MASK      0x3fL
#    define SHIFT_RIGHT     0x80L
#    define SHIFT_ARITH     0x40L
#endif
#define J_SIGNED   0x04000000L  /* orable with LDRBx, LDRWx, also fix/float */
#define J_UNSIGNED 0x08000000L  /* ditto */
#define _J_GAP1    0x10000000L  /* r1 field always unused */
#define _J_GAP2    0x20000000L  /* r2 field always unused */

/* type bits (reallocate?) ... n.b. Q_UBIT shares with J_UNSIGNED */

/*
 * The next few macros represent all the places where arithmetic is done
 * on jopcodes to map between them, and hence these lines imply all the
 * constraints that there are on the ordering of jopcodes within the
 * enumeration.
 */
#define J_RTOK(a) ((a) - 1)
#define J_KTOR(a) ((a) + 1)
/*
 * When an instruction occurs in both R and K formats, the codes for the two
 * must be adjacent.  E.g. ADDK then ADDR.
 */
#define J_addvk(a) ((a)+(J_LDRVK-J_LDRK))
#define J_subvk(a) ((a)-(J_LDRVK-J_LDRK))
/*
 * the LDRVK set of codes must be at a constant offset from the LDRK base
 * set.
 */
#define tailify_(a)   ((a) + (J_TAILCALLK-J_CALLK))
/*
 * TAILCALL & CALL separation does not impose any extra constraints here.
 */
/* auxiliary bits in the r2 field of CALLK/R...                         */
#define K_ARGWORDMASK   0x00ffffffL
#define K_VACALL        0x01000000L /* only if TARGET_FLAGS_VA_CALLS    */
#define K_PURE          0x02000000L /* replaces J_CALLP                 */
#define K_STRUCTRET     0x04000000L /* only if TARGET_STRUCT_RESULT_REGISTER */
#define K_FPARG_SHIFT   28          /* only if TARGET_IS_MIPS so far.   */

/*
 * Now macros which test characteristics of codes.  In some cases this is
 * currently implemented as sets of tests where allocating extra bits in
 * joptable may allow conversion to simple bit testing.
 */
#define register_movep(a) (_joptable(a) & _J_REGMOVE)
#define floatiness_(a) (_joptable(a) & (_J_DOUBLE+_J_FLOATING))
#define J_fltisdouble(a) (_joptable(a) & _J_DOUBLE)
#define vkformat(a) ((_joptable(a) & _J_MEM+_J_IVD) == _J_MEM+_J_IVD)
#define gap_r1(a) (_joptable(a) & _J_GAP1)
#define loads_r1(a) (_joptable(a) & _J_SET_R1)
#define reads_r1(a) (_joptable(a) & _J_READ_R1)
#define uses_r1(a)  (_joptable(a) & (_J_SET_R1 | _J_READ_R1))
/* The following test could easily be indicated by SET_R1+READ_R1 but   */
/* currently various parts of the compiler assumes they are disjoint.   */
/* Merging in certain J_MOVC cases seems indicated here too.            */
#ifdef TARGET_CORRUPTS_SWITCH_REGISTER
#  define corrupts_r1(a) ((a) == J_CASEBRANCH)
#else
#  define corrupts_r1(a) 0
#endif
#define stores_r1(a) ((_joptable(a) & (_J_MEM | _J_READ_R1)) == (_J_MEM | _J_READ_R1))
#define gap_r2(a) (_joptable(a) & _J_GAP2)
#define reads_r2(a) (_joptable(a) & _J_READ_R2)
#define uses_r2(a)  reads_r2(a)
#define reads_r3(a) (_joptable(a) & _J_READ_R3)
#define uses_r3(a)  reads_r3(a)
#define uses_stack(a) ((_joptable(a) & _J_STACKREF+_J_MEM) == _J_STACKREF+_J_MEM)
#define uses_mem(a) (_joptable(a) & _J_MEM)
#ifdef TARGET_COUNT_IS_PROC
#  define isproccall_(op) ((op) == J_CALLK  || (op) == J_CALLR || \
                           (op) == J_CALL2KP || (op) == J_COUNT)
#else
#  define isproccall_(op) ((op) == J_CALLK || (op) == J_CALLR || \
                           (op) == J_CALL2KP)
#endif
#ifdef TARGET_HAS_2ADDRESS_CODE
#  define jop_asymdiadr_(op) \
   ((_joptable(op) & _J_ASYMDIAD+_J_READ_R3+_J_MEM) == _J_ASYMDIAD+_J_READ_R3)
/* maybe insert an is_anydiadr() here, but this think w.r.t. ADDK/ADDR and  */
/* possible uses of load address target instructions.                       */
#endif
#define j_is_adcon(a) ((_joptable(a) & _J_MEMA) == _J_MEMA)
#define j_is_diadr(a) ((_joptable(a) & (_J_MEM | _J_READ_R3 | _J_READ_R2 | _J_READ_R1)) ==\
                       (_J_READ_R3 | _J_READ_R2))
#define j_is_diadk(a) ((_joptable(a) & (_J_MEM | _J_READ_R3 | _J_READ_R2 | _J_READ_R1)) ==\
                       _J_READ_R2)

/* The following two macros are only used by jopprint.c and regalloc.c --   */
/* this is important since they are not overkeen about masking Q_MASK or    */
/* J_DEADBITS.  Note that they must correspond to the loop optimisation     */
/* code in loopopt.c and its friends in flowgraf.c                          */
/* Note that J_FNCON does not need considering as only this is only used    */
/* in the interface to xxxgen.c from flowgraf.c (q.v.)                      */
#ifdef RANGECHECK_SUPPORTED
#  define pseudo_reads_r1(a) (((a) & ~Q_MASK) == J_CMPK ||\
                              (a)==J_CHKLK || (a)==J_CHKUK || (a)==J_CHKNEK)
#else
#  define pseudo_reads_r1(a) (((a) & ~Q_MASK) == J_CMPK)
#endif
#define pseudo_reads_r2(a) ((a)==J_MOVK || (a)==J_ADCON || (a)==J_ADCONV)
#define j_is_check(op) \
    ((J_CHKLK<=(op) && (op)<=J_CHKUR) || (J_CHKNEFK<=(op) && (op)<=J_CHKNER))

/*
 * For CSE, to find the type of the operand of LD/STxx.
 */
#define    MEM_B 0L     /* values must match _J_LDRK_w[] table in cse.c */
#define    MEM_W 1L
#define    MEM_L 2L
#define    MEM_F 3L
#define    MEM_D 4L

#define j_memsize(a) ((_joptable(a) & _J_OPTYPE) >> _J_OPTYPE_SHIFT)

#define    _T_B   (MEM_B << _J_OPTYPE_SHIFT)
#define    _T_W   (MEM_W << _J_OPTYPE_SHIFT)
#define    _T_L   (MEM_L << _J_OPTYPE_SHIFT)
#define    _T_F   (MEM_F << _J_OPTYPE_SHIFT)
#define    _T_D   (MEM_D << _J_OPTYPE_SHIFT)


#ifdef DEFINE_JOPTABLE
/*
 * Although it is generally a nasty thing to do, I have a definition of
 * the initialised array joptable here in a header file, in such a way that
 * it is only activated if the macro DEFINE_JOPTABLE is defined.  This must
 * be so for exactly one place in the source where jopcode.h gets included.
 * Doing things this way allows me to keep the enumeration of codes in step
 * with associated entries in the array.
 * NB: conditional compilation within this table might cause things to
 *     get out of step - so beware.
 */
#  if defined ENABLE_CG || defined ENABLE_REGS || defined ENABLE_CSE
#     define with_bits(n, b) {n, b},
      extern struct joptable { char *name; unsigned32 bits; } joptable[];
      struct joptable joptable[] = {
#  else
#     define with_bits(n, b) { b },
      extern struct joptable { unsigned32 bits; } joptable[];
      struct joptable joptable[] = {
#  endif
#else
#  define with_bits(n, b)
#  if defined ENABLE_CG || defined ENABLE_REGS || defined ENABLE_CSE
      extern struct joptable { char *name; unsigned32 bits; } joptable[];
#  else
      extern struct joptable { unsigned32 bits; } joptable[];
#  endif
#endif

/*
 * Note that whenever an operand has both an R and a K (register and literal
 * operand) form the two must appear adjacent in this table for J_RTOK()
 * Towards the start of this table there are some other numbering nasties
 * which allow rapid conversion between related codes - see the macros defined
 * above for what is done, and do not re-order this table or insert or
 * delete entries without understanding the consequences.
 * In particular, note that there must be no gaps, since the numbering
 * of jopcodes is used as an index into the table created by with_bits().
 */

#define    J_NOOP       0L
           with_bits("NOOP", _J_GAP1+_J_GAP2)
#define    J_LABEL      1L
           with_bits("LABEL", _J_GAP1+_J_GAP2)
#define    J_LDRFK      2L
           with_bits("LDRFK", _J_FLOATING+_J_MEM+_J_READ_R2+_J_SET_R1+_T_F)
#define    J_LDRFR      3L
           with_bits("LDRFR", _J_FLOATING+_J_MEM+_J_READ_R3+_J_READ_R2+_J_SET_R1+_T_F)
#define    J_LDRDK      4L
           with_bits("LDRDK", _J_DOUBLE+_J_MEM+_J_READ_R2+_J_SET_R1+_T_D)
#define    J_LDRDR      5L
           with_bits("LDRDR", _J_DOUBLE+_J_MEM+_J_READ_R3+_J_READ_R2+_J_SET_R1+_T_D)
#define    J_LDRK       6L
           with_bits("LDRK", _J_MEM+_J_READ_R2+_J_SET_R1+_T_L)
#define    J_LDRR       7L
           with_bits("LDRR", _J_MEM+_J_READ_R3+_J_READ_R2+_J_SET_R1+_T_L)
#define    J_LDRBK      8L
           with_bits("LDRBK", _J_MEM+_J_READ_R2+_J_SET_R1+_T_B)
#define    J_LDRBR      9L
           with_bits("LDRBR", _J_MEM+_J_READ_R3+_J_READ_R2+_J_SET_R1+_T_B)
#define    J_LDRWK      10L
           with_bits("LDRWK", _J_MEM+_J_READ_R2+_J_SET_R1+_T_W)
#define    J_LDRWR      11L
           with_bits("LDRWR", _J_MEM+_J_READ_R3+_J_READ_R2+_J_SET_R1+_T_W)
#define    J_BXX        12L      /* as J_B but within branch table    */
           with_bits("BXX", _J_GAP1+_J_GAP2)
#define    J_B          13L
           with_bits("B", _J_GAP1+_J_GAP2)
#define    J_BZ         14L     /* For 88000 back-end use only */
           with_bits("BZ", _J_READ_R2)
#define    J_POP        15L     /* special effect on registers */
           with_bits("POP", _J_GAP2)
#define    J_PUSHM      16L     /* for the back end, reg map   */
           with_bits("PUSHM", _J_GAP1+_J_GAP2)
#define    J_POPM       17L     /* ditto                       */
           with_bits("POPM", _J_GAP1+_J_GAP2)
#define    J_STRFK      18L
           with_bits("STRFK", _J_FLOATING+_J_MEM+_J_READ_R2+_J_READ_R1+_T_F)
#define    J_STRFR      19L
           with_bits("STRFR", _J_FLOATING+_J_MEM+_J_READ_R3+_J_READ_R2+_J_READ_R1+_T_F)
#define    J_STRDK      20L
           with_bits("STRDK", _J_DOUBLE+_J_MEM+_J_READ_R2+_J_READ_R1+_T_D)
#define    J_STRDR      21L
           with_bits("STRDR", _J_DOUBLE+_J_MEM+_J_READ_R3+_J_READ_R2+_J_READ_R1+_T_D)
#define    J_STRK       22L
           with_bits("STRK", _J_MEM+_J_READ_R2+_J_READ_R1+_T_L)
#define    J_STRR       23L
           with_bits("STRR", _J_MEM+_J_READ_R3+_J_READ_R2+_J_READ_R1+_T_L)
#define    J_STRBK      24L
           with_bits("STRBK", _J_MEM+_J_READ_R2+_J_READ_R1+_T_B)
#define    J_STRBR      25L
           with_bits("STRBR", _J_MEM+_J_READ_R3+_J_READ_R2+_J_READ_R1+_T_B)
#define    J_STRWK      26L
           with_bits("STRWK", _J_MEM+_J_READ_R2+_J_READ_R1+_T_W)
#define    J_STRWR      27L
           with_bits("STRWR", _J_MEM+_J_READ_R3+_J_READ_R2+_J_READ_R1+_T_W)
#define    J_CMPK       28L       /* see pseudo_reads_r2() (loop opt) */
           with_bits("CMPK", _J_READ_R2)
#define    J_CMPR       29L
           with_bits("CMPR", _J_READ_R3+_J_READ_R2)
#define    J_CHKLK      30L
           with_bits("CHKLK", _J_READ_R2)
#define    J_CHKLR      31L
           with_bits("CHKLR", _J_READ_R3+_J_READ_R2)
#define    J_CHKUK      32L
           with_bits("CHKUK", _J_READ_R2)
#define    J_CHKUR      33L
           with_bits("CHKUR", _J_READ_R3+_J_READ_R2)
#define    J_LDRFVK     34L
           with_bits("LDRFVK", _J_MEM+_J_STACKREF+_J_IVD+_J_SET_R1+_T_F)
#define    J_ADCONF     35L     /* only if SOFTWARE_FLOATING_POINT */
           with_bits("ADCONF", _J_SET_R1)
#define    J_LDRDVK     36L
           with_bits("LDRDVK", _J_MEM+_J_STACKREF+_J_IVD+_J_SET_R1+_T_D)
#define    J_ADCOND     37L     /* only if SOFTWARE_FLOATING_POINT */
           with_bits("ADCOND", _J_SET_R1)
#define    J_LDRVK      38L
           with_bits("LDRVK", _J_MEM+_J_STACKREF+_J_IVD+_J_SET_R1+_T_L)
#define    J_ADCON      39L   /* see pseudo_reads_r2() (loop opt) */
           with_bits("ADCON", _J_MEMA+_J_SET_R1)
#define    J_LDRBVK     40L
           with_bits("LDRBVK", _J_MEM+_J_STACKREF+_J_IVD+_J_SET_R1+_T_B)
#define    J_COUNT      41L
           with_bits("COUNT", _J_GAP1+_J_GAP2)
#define    J_LDRWVK     42L
           with_bits("LDRWVK", _J_MEM+_J_STACKREF+_J_IVD+_J_SET_R1+_T_W)
#define    J_STACK      43L
           with_bits("STACK", _J_GAP1+_J_GAP2)
#define    J_SETSPENV   44L
           with_bits("SETSPENV", _J_GAP1+_J_GAP2)
#define    J_SETSPGOTO  45L
           with_bits("SETSPGOTO", _J_GAP1+_J_GAP2)
#define    J_SETSP      46L
           with_bits("SETSP", _J_GAP1)
#define    J_INFOLINE   47L
           with_bits("INFOLINE", 0)
#define    J_INFOSCOPE  48L
           with_bits("INFOSCOPE", 0)
#define    J_INFOBODY   49L
           with_bits("INFOBODY", 0)
#define    J_STRFVK     50L
           with_bits("STRFVK", _J_MEM+_J_STACKREF+_J_IVD+_J_READ_R1+_T_F)
#define    J_VSTORE     51L
           with_bits("VSTORE", _J_GAP1+_J_GAP2)
#define    J_STRDVK     52L
           with_bits("STRDVK", _J_MEM+_J_STACKREF+_J_IVD+_J_READ_R1+_T_D)
#define    J_WORD       53L
           with_bits("WORD", _J_GAP1+_J_GAP2)
#define    J_STRVK      54L
           with_bits("STRVK", _J_MEM+_J_STACKREF+_J_IVD+_J_READ_R1+_T_L)
#define    J_CASEBRANCH 55L /* dispatch into branch table          */
           with_bits("CASEBRANCH", _J_READ_R1)
#define    J_STRBVK     56L
           with_bits("STRBVK", _J_MEM+_J_STACKREF+_J_IVD+_J_READ_R1+_T_B)
#define    J_USE        57L        /* uses R1 (for support of 'volatile') */
           with_bits("USE", _J_READ_R1+_J_GAP2)
#define    J_STRWVK     58L
           with_bits("STRWVK", _J_MEM+_J_STACKREF+_J_IVD+_J_READ_R1+_T_W)
#define    J_INIT       59L
           with_bits("INIT", _J_SET_R1+_J_GAP2)
#define    J_STRING     60L
           with_bits("STRING", _J_SET_R1+_J_GAP2)
#define    J_CALLK      61L
           with_bits("CALLK", _J_SET_R1)
#define    J_CALLR      62L
           with_bits("CALLR", _J_READ_R3+_J_SET_R1)
#define    J_spare63    63L
           with_bits("spare63", 0)
#define    J_ENTER      64L    /* special effect on registers */
           with_bits("ENTER", _J_GAP1+_J_GAP2)
#define    J_ENDPROC    65L
           with_bits("ENDPROC", _J_GAP1+_J_GAP2)
#define    J_LDRFV      66L
           with_bits("LDRFV", _J_FLOATING+_J_MEM+_J_SET_R1+_J_STACKREF+_T_F+_J_GAP2)
#define    J_LDRFV1     67L
           with_bits("LDRFV1", _J_FLOATING+_J_MEM+_J_SET_R1+_J_STACKREF+_T_F+_J_GAP2)
#define    J_LDRDV      68L
           with_bits("LDRDV", _J_DOUBLE+_J_MEM+_J_SET_R1+_J_STACKREF+_T_D+_J_GAP2)
#define    J_LDRDV1     69L
           with_bits("LDRDV1", _J_DOUBLE+_J_MEM+_J_SET_R1+_J_STACKREF+_T_D+_J_GAP2)
#define    J_LDRV       70L
           with_bits("LDRV", _J_MEM+_J_SET_R1+_J_STACKREF+_T_L+_J_GAP2)
#define    J_LDRV1      71L
           with_bits("LDRV1", _J_MEM+_J_SET_R1+_J_STACKREF+_T_L+_J_GAP2)
#define    J_ADDK       72L
           with_bits("ADDK", _J_READ_R2+_J_SET_R1)
#define    J_ADDR       73L
           with_bits("ADDR", _J_READ_R3+_J_READ_R2+_J_SET_R1)
#define    J_MULK       74L
           with_bits("MULK", _J_READ_R2+_J_SET_R1)
#define    J_MULR       75L
           with_bits("MULR", _J_READ_R3+_J_READ_R2+_J_SET_R1)
#define    J_ANDK       76L
           with_bits("ANDK", _J_READ_R2+_J_SET_R1)
#define    J_ANDR       77L
           with_bits("ANDR", _J_READ_R3+_J_READ_R2+_J_SET_R1)
#define    J_ORRK       78L
           with_bits("ORRK", _J_READ_R2+_J_SET_R1)
#define    J_ORRR       79L
           with_bits("ORRR", _J_READ_R3+_J_READ_R2+_J_SET_R1)
#define    J_EORK       80L
           with_bits("EORK", _J_READ_R2+_J_SET_R1)
#define    J_EORR       81L
           with_bits("EORR", _J_READ_R3+_J_READ_R2+_J_SET_R1)
#define    J_STRFV      82L
           with_bits("STRFV", _J_FLOATING+_J_MEM+_J_READ_R1+_J_STACKREF+_T_F+_J_GAP2)
#define    J_CONDEXEC   83L
           with_bits("CONDEXEC", _J_GAP1+_J_GAP2)
#define    J_STRDV      84L
           with_bits("STRDV", _J_DOUBLE+_J_MEM+_J_READ_R1+_J_STACKREF+_T_D+_J_GAP2)
#define    J_FNCON      85L   /* only if TARGET_CALL_USES_DESCRIPTOR */
           with_bits("FNCON", _J_MEMA+_J_SET_R1)
#define    J_STRV       86L
           with_bits("STRV", _J_MEM+_J_READ_R1+_J_STACKREF+_T_L+_J_GAP2)
#define    J_ADCONV     87L   /* special use of r2 field */
           with_bits("ADCONV", _J_MEMA+_J_SET_R1+_J_STACKREF)
#define    J_CMPFK      88L
           with_bits("CMPFK", _J_FLOATING+_J_READ_R2)
#define    J_CMPFR      89L
           with_bits("CMPFR", _J_FLOATING+_J_READ_R3+_J_READ_R2)
#define    J_CHKNEFK    90L
           with_bits("CHKNEFK", _J_FLOATING+_J_READ_R2)
#define    J_CHKNEFR    91L
           with_bits("CHKNEFR", _J_FLOATING+_J_READ_R3+_J_READ_R2)
#define    J_CMPNEDK    92L
           with_bits("CHKNEDK", _J_DOUBLE+_J_READ_R2)
#define    J_CHKNEDR    93L
           with_bits("CHKNEDR", _J_DOUBLE+_J_READ_R3+_J_READ_R2)
#define    J_CHKNEK     94L
           with_bits("CHKNEK", _J_READ_R2)
#define    J_CHKNER     95L
           with_bits("CHKNER", _J_READ_R3+_J_READ_R2)
#define    J_CALL2KP    96L     /* artefact of CSE, survives only until
                                   register allocation - call of function
                                   returning two arguments */
           with_bits("CALL2KP", _J_SET_R1)
#define    CSE_LOADR    97L    /* wanted internally by CSE */
           with_bits("CSE_LOADR", _J_SET_R1)
#define    J_MOVFK      98L
           with_bits("MOVFK", _J_FLOATING+_J_SET_R1+_J_GAP2)
#define    J_MOVFR      99L
           with_bits("MOVFR", _J_FLOATING+_J_READ_R3+_J_SET_R1+_J_REGMOVE+_J_GAP2)
#define    J_MOVDK      100L
           with_bits("MOVDK", _J_DOUBLE+_J_SET_R1+_J_GAP2)
#define    J_MOVDR      101L
           with_bits("MOVDR", _J_DOUBLE+_J_READ_R3+_J_SET_R1+_J_REGMOVE+_J_GAP2)
#define    J_MOVK       102L
           with_bits("MOVK", _J_SET_R1+_J_GAP2)
#define    J_MOVR       103L
           with_bits("MOVR", _J_READ_R3+_J_SET_R1+_J_REGMOVE+_J_GAP2)

/*
 * From here onwards the order of jopcodes is less important.
 */

#define    J_SUBK       104L
           with_bits("SUBK", _J_READ_R2+_J_SET_R1+_J_ASYMDIAD)
#define    J_SUBR       105L
           with_bits("SUBR", _J_READ_R3+_J_READ_R2+_J_SET_R1+_J_ASYMDIAD)
#define    J_DIVK       106L   /* must or in J_SIGNED/J_UNSIGNED */
           with_bits("DIVK", _J_READ_R2+_J_SET_R1+_J_ASYMDIAD)
#define    J_DIVR       107L
           with_bits("DIVR", _J_READ_R3+_J_READ_R2+_J_SET_R1+_J_ASYMDIAD)
#define    J_RSBK       108L   /* should only happen as J_RSBR+J_SHIFTM */
           with_bits("RSBK", _J_READ_R2+_J_SET_R1+_J_ASYMDIAD)
#define    J_RSBR       109L  /* should only happen as J_RSBR+J_SHIFTM */
           with_bits("RSBR", _J_READ_R3+_J_READ_R2+_J_SET_R1+_J_ASYMDIAD)
#define    J_REMK       110L   /* must or in J_SIGNED/J_UNSIGNED */
           with_bits("REMK", _J_READ_R2+_J_SET_R1+_J_ASYMDIAD)
#define    J_REMR       111L
           with_bits("REMR", _J_READ_R3+_J_READ_R2+_J_SET_R1+_J_ASYMDIAD)
#define    J_SHLK       112L   /* must or in J_SIGNED/J_UNSIGNED */
           with_bits("SHLK", _J_READ_R2+_J_SET_R1+_J_ASYMDIAD)
#define    J_SHLR       113L
           with_bits("SHLR", _J_READ_R3+_J_READ_R2+_J_SET_R1+_J_ASYMDIAD)
#define    J_SHRK       114L   /* must or in J_SIGNED/J_UNSIGNED */
           with_bits("SHRK", _J_READ_R2+_J_SET_R1+_J_ASYMDIAD)
#define    J_SHRR       115L
           with_bits("SHRR", _J_READ_R3+_J_READ_R2+_J_SET_R1+_J_ASYMDIAD)
#define    J_INLINE1    116L
           with_bits("INLINE1", _J_READ_R2+_J_SET_R1)
#define    J_NEGR       117L
           with_bits("NEGR", _J_READ_R3+_J_SET_R1+_J_GAP2)
#define    J_NOTR       118L
           with_bits("NOTR", _J_READ_R3+_J_SET_R1+_J_GAP2)
#define    J_PUSHF      119L
           with_bits("PUSHF", _J_FLOATING+_J_READ_R1+_J_GAP2)
#define    J_USEF       120L
           with_bits("USEF", _J_FLOATING+_J_READ_R1+_J_GAP2)
#define    J_INITF      121L
           with_bits("INITF", _J_FLOATING+_J_SET_R1+_J_GAP2)
#define    J_ADDFK      122L
           with_bits("ADDFK", _J_FLOATING+_J_READ_R2+_J_SET_R1)
#define    J_ADDFR      123L
           with_bits("ADDFR", _J_FLOATING+_J_READ_R3+_J_READ_R2+_J_SET_R1)
#define    J_MULFK      124L
           with_bits("MULFK", _J_FLOATING+_J_READ_R2+_J_SET_R1)
#define    J_MULFR      125L
           with_bits("MULFR", _J_FLOATING+_J_READ_R3+_J_READ_R2+_J_SET_R1)
#define    J_INLINE1F   126L
           with_bits("INLINE1F", _J_FLOATING+_J_READ_R2+_J_SET_R1)
#define    J_SUBFK      127L
           with_bits("SUBFK", _J_FLOATING+_J_READ_R2+_J_SET_R1+_J_ASYMDIAD)
#define    J_SUBFR      128L
           with_bits("SUBFR", _J_FLOATING+_J_READ_R3+_J_READ_R2+_J_SET_R1+_J_ASYMDIAD)
#define    J_DIVFK      129L
           with_bits("DIVFK", _J_FLOATING+_J_READ_R2+_J_SET_R1+_J_ASYMDIAD)
#define    J_DIVFR      130L
           with_bits("DIVFR", _J_FLOATING+_J_READ_R3+_J_READ_R2+_J_SET_R1+_J_ASYMDIAD)
#define    J_NEGFR      131L
           with_bits("NEGFR", _J_FLOATING+_J_READ_R3+_J_SET_R1)
#define    J_FLTFR      132L
           with_bits("FLTFR", _J_FLOATING+_J_READ_R3+_J_SET_R1)
#define    J_FIXFR      133L
           with_bits("FIXFR", _J_FLOATING+_J_READ_R3+_J_SET_R1)
#define    J_MOVDFR     134L
           with_bits("MOVDFR", _J_FLOATING+_J_READ_R3+_J_SET_R1)
#define    J_MOVIFR     135L
           with_bits("MOVIFR", _J_FLOATING+_J_READ_R3+_J_SET_R1)
#define    J_PUSHD      136L
           with_bits("PUSHD", _J_DOUBLE+_J_READ_R1+_J_GAP2)
#define    J_USED       137L
           with_bits("USED", _J_DOUBLE+_J_READ_R1+_J_GAP2)
#define    J_INITD      138L
           with_bits("INITD", _J_DOUBLE+_J_SET_R1+_J_GAP2)
#define    J_CMPDK      139L
           with_bits("CMPDK", _J_DOUBLE+_J_READ_R2)
#define    J_CMPDR      140L
           with_bits("CMPDR", _J_DOUBLE+_J_READ_R3+_J_READ_R2)
#define    J_ADDDK      141L
           with_bits("ADDDK", _J_DOUBLE+_J_READ_R2+_J_SET_R1)
#define    J_ADDDR      142L
           with_bits("ADDDR", _J_DOUBLE+_J_READ_R3+_J_READ_R2+_J_SET_R1)
#define    J_MULDK      143L
           with_bits("MULDK", _J_DOUBLE+_J_READ_R2+_J_SET_R1)
#define    J_MULDR      144L
           with_bits("MULDR", _J_DOUBLE+_J_READ_R3+_J_READ_R2+_J_SET_R1)
#define    J_MOVIDR     145L    /* a bit special - doesn't require r1/r3 clash. */
           with_bits("MOVIDR", _J_DOUBLE+_J_READ_R3+_J_READ_R2+_J_SET_R1)
#define    J_INLINE1D   146L
           with_bits("INLINE1D", _J_DOUBLE+_J_READ_R2+_J_SET_R1)
#define    J_SUBDK      147L
           with_bits("SUBDK", _J_DOUBLE+_J_READ_R2+_J_SET_R1+_J_ASYMDIAD)
#define    J_SUBDR      148L
           with_bits("SUBDR", _J_DOUBLE+_J_READ_R3+_J_READ_R2+_J_SET_R1+_J_ASYMDIAD)
#define    J_DIVDK      149L
           with_bits("DIVDK", _J_DOUBLE+_J_READ_R2+_J_SET_R1+_J_ASYMDIAD)
#define    J_DIVDR      150L
           with_bits("DIVDR", _J_DOUBLE+_J_READ_R3+_J_READ_R2+_J_SET_R1+_J_ASYMDIAD)
#define    J_NEGDR      151L
           with_bits("NEGDR", _J_DOUBLE+_J_READ_R3+_J_SET_R1+_J_GAP2)
#define    J_FLTDR      152L
           with_bits("FLTDR", _J_DOUBLE+_J_READ_R3+_J_SET_R1+_J_GAP2)
#define    J_FIXDR      153L
           with_bits("FIXDR", _J_DOUBLE+_J_READ_R3+_J_SET_R1+_J_GAP2)
#define    J_MOVFDR     154L
           with_bits("MOVFDR", _J_DOUBLE+_J_READ_R3+_J_SET_R1+_J_GAP2)
#define    CSE_WORD1    155L
           with_bits("CSE_WORD1", _J_DOUBLE+_J_READ_R3+_J_SET_R1)
#define    CSE_WORD2    156L
           with_bits("CSE_WORD2", _J_DOUBLE+_J_READ_R3+_J_SET_R1)
/*
 * Things from here on are not needed for all targets.
 */

#define    J_CLRC       157L
           with_bits("CLRC", _J_READ_R1)
#define    J_MOVC       158L
           with_bits("MOVC", _J_READ_R2+_J_READ_R1)
#define    J_TAILCALLK  159L            /* see tailify_() */
           with_bits("TAILCALLK", _J_SET_R1)
#define    J_TAILCALLR  160L            /* see tailify_() */
           with_bits("TAILCALLR", _J_READ_R3+_J_SET_R1)
/* The following are not used at present, but may be sometime - when they
 * are the numbering of jopcodes will probably need rearranging to fit these
 * in somewhere better than at the end. Indeed some of these MAY be used now...
 * but sort of experimentally!
 */
#define    J_LDRBV      161L
           with_bits("LDRBV", _J_MEM+_J_SET_R1+_J_STACKREF+_J_GAP2)
#define    J_LDRWV      162L
           with_bits("LDRWV", _J_MEM+_J_SET_R1+_J_STACKREF+_J_GAP2)
#define    J_STRBV      163L
           with_bits("STRBV", _J_MEM+_J_READ_R1+_J_STACKREF+_J_GAP2)
#define    J_STRWV      164L
           with_bits("STRWV", _J_MEM+_J_READ_R1+_J_STACKREF+_J_GAP2)
#define    J_RESULT2    165L
           with_bits("RESULT2", _J_READ_R3+_J_SET_R1)
#define    J_PUSHR      166L
           with_bits("PUSHR", _J_READ_R1+_J_GAP2)
/* AM reserves the next two opcodes for flt->int reg moves.             */
/* Note that MOVDIR sets r1 and r1+1.  Regalloc beware.                 */
#define    J_MOVDIR     167L
           with_bits("MOVDIR", _J_SET_R1+_J_READ_R3+_J_GAP2)
#define    J_MOVFIR     168L
           with_bits("MOVFIR", _J_SET_R1+_J_READ_R3+_J_GAP2)
#define    J_SCCK       169L    /* compare J_CONDEXEC                   */
           with_bits("SCCK", _J_SET_R1)
#define    J_EXTEND     170L    /* sign extend */
           with_bits("EXTEND", _J_SET_R1+_J_READ_R2)

#define J_LAST_JOPCODE  170L

#ifdef DEFINE_JOPTABLE
   with_bits( 0, 0 ) };

#endif

/*************************************/
/* end of emumeration of all opcodes */
/*************************************/


typedef int32 J_OPCODE; /* really ENUMERATED_OPCODE but ensure 32 bits */

/* n.b Q_UBIT is the same as J_UNSIGNED */
#define Q_MASK          0xf8000000U
#define Q_UBIT          0x08000000U   /* for test of signed/unsigned CMP   */
#define Q_NEGATE( x )	((x) ^ 0x10000000U)
      
/* N.B. individual machine translators may rely on the cond values below -  */
/* the routine/macro C_FROMQ() does this.                                   */
/* The following idea of conditions are machine independent - so any new    */
/* code (e.g. Q_CC) should have a different number even if they have the    */
/* same effect on your favorite machine - just as Q_HS and Q_GE differ even */
/* though they have the same effect on the 370 (the comparison differs).    */

#define Q_EQ    ( 0x00000000U ^ 0xe0000000U)
#define Q_NE    ( 0x10000000U ^ 0xe0000000U)
#define Q_HS    ((0x20000000U ^ 0xe0000000U) | Q_UBIT)
#define Q_LO    ((0x30000000U ^ 0xe0000000U) | Q_UBIT)
/* #define Q_MI ( 0x40000000U ^ 0xe0000000U) */
/* #define Q_PL ( 0x50000000U ^ 0xe0000000U) */
/* #define Q_VS ( 0x60000000U ^ 0xe0000000U) */
/* #define Q_VC ( 0x70000000U ^ 0xe0000000U) */
#define Q_HI    ((0x80000000U ^ 0xe0000000U) | Q_UBIT)
#define Q_LS    ((0x90000000U ^ 0xe0000000U) | Q_UBIT)
#define Q_GE    ( 0xa0000000U ^ 0xe0000000U)
#define Q_LT    ( 0xb0000000U ^ 0xe0000000U)
#define Q_GT    ( 0xc0000000U ^ 0xe0000000U)
#define Q_LE    ( 0xd0000000U ^ 0xe0000000U)
#define Q_AL    ( 0xe0000000U ^ 0xe0000000U)
#define Q_NOT   ( 0xf0000000U ^ 0xe0000000U)
      
/* The following are added by AM to check conversion to new style J_CMP */
/* Q_UEQ and Q_UNE are unsigned equality tests.  May be useful one day? */
      
#define Q_UEQ    (Q_EQ | Q_UBIT)
#define Q_UNE    (Q_NE | Q_UBIT)
#define Q_XXX    (0x60000000U ^ 0xe0000000U)  /* flag CMP use for 3 way tests */



#define blkcode_(x)     (x->code)            /* start of 3-address code  */
#define blklength_(x)   (x->length)          /* length of ditto          */
#define blkflags_(x)    (x->flags)           /* misc flag bits           */
#define blknext_(x)     (x->succ1.next)      /* successor block          */
#define blknext1_(x)    (x->succ2.next1)     /* alternative successor    */
#define blktable_(x)    (x->succ1.table)     /* table of successors      */
#define blktabsize_(x)  (x->succ2.tabsize)   /* size of aforesaid table  */
#define blkbackp_(x)    (x->succ1.backp)     /* for branch_chain() only  */
#define blkbackp1_(x)   (x->succ2.backp1)    /* for branch_chain() only  */
#define blkdown_(x)     (x->down)            /* forward chaining         */
#define blkup_(x)       (x->up)              /* backward chaining        */
#define blklab_(x)      (x->lab)             /* label for this block     */
#define blkuse_(x)      (x->use)      /* registers needed at block head  */
#define blkstack_(x)    (x->stack.l)  /* binders active at head of block */
#define blkstacki_(x)   (x->stack.i)
#define blkdebenv_(x)   (x->debenv)   /* for debugger                    */
#define blkusedfrom_(x) (x->usedfrom) /* used in cross-jump optimization */
#define blknest_(x)     (x->loopnest) /* # loops enclosing this block.   */

/* bits for use in blkflags_(x)                                          */

/* #define BLKREFMASK   3L  DEFUNCT    refcount 0,1,2,many               */
#define BLKALIVE        4L          /* used when flattening graph        */
#define BLKSWITCH       8L          /* contains 'switch' multi-exit      */
#define BLK2EXIT     0x10L          /* contains conditional exit         */
#define BLKBUSY      0x20L          /* used when flattening graph        */
#define BLKCODED     0x40L          /* ditto                             */
#define BLKEMPTY     0x80L          /* SET IF BLOCK EMPTY                */
#define BLKLOOP     0x100L          /* block is a loop head              */
/*#define BLKINNER    0x200L*/      /* loop has no other loops inside    */
                                    /* Nowhere used ?                    */
#define BLKOUTER    0x400L          /* set in function header block      */
#define BLK0EXIT    0x800L          /* no exit (tail procedure call)     */
#define BLKP2      0x1000L          /* bits used to control backpointer  */
#define BLKP3      0x2000L          /* bits used to control backpointer  */
#define BLKCALL    0x4000L          /* block contains a proc call        */
#define BLK2CALL   0x8000L          /* block contains 2 proc calls       */
#define BLKSETJMP 0x10000L          /* block may call setjmp (groan).    */
#define BLKSTACKI 0x100000L         /* blkstack is a number not a bindlist*/
/* N.B. Q_MASK values above also used when BLK2EXIT is set (condition)   */
/* Also, the following bits are are disjoint from the above but are only */
/* used in 'procflags'.  Some of them may later be BLK oriented.         */
#define PROC_ARGADDR     0x20000L   /* arg address taken                 */
#define PROC_ARGPUSH     0x40000L   /* treat args carefully (see cg.c)   */
#define PROC_BIGSTACK    0x80000L   /* stack bigger than 256             */

/* Special values to go in blknext_(), getting to be too many...         */

#define RetIntLab       ((LabelNumber *)-256L)
#define RetFloatLab     ((LabelNumber *)-252L)
#define RetDbleLab      ((LabelNumber *)-248L)
#define RetVoidLab      ((LabelNumber *)-244L)
#define RetImplLab      ((LabelNumber *)-240L)
/* and for use in flowgraph.c - change soon? */
#define RETLAB          ((LabelNumber *)-236L)
#define NOTALAB         ((LabelNumber *)-232L)

extern void print_jopcode(J_OPCODE op, VRegInt r1, VRegInt r2, VRegInt m);
extern void print_jopcode_1(J_OPCODE op, VRegInt r1, VRegInt r2, VRegInt m);
extern void jopprint_opname(J_OPCODE o);
extern void print_xjopcode(J_OPCODE op, VRegInt r1, VRegInt r2, VRegInt m,
                    char *fmt, ...);

#endif

/* End of jopcode.h */
