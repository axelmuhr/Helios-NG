
/* file jopcode.h:  Copyright (C) A. Mycroft and A.C. Norman */
/* version 23b */

/* AM memo: removing some of the global properties bits (like _READ_R1 or */
/* _ASYMDIAD) from the instruction, and placing them in a table like      */
/* _ctype[] would free some bits for an per-instruction property like     */
/* "r1 is dead after here and so can be corrupted".                       */
/* AM has now got the bits by re-organisation, but the idea is still good */

#ifndef _JOPCODE_LOADED

#define _JOPCODE_LOADED 1

/* Opcodes in the intermediate code that I generate                       */
/* Idea -- the things beginning with '_' are local to this file, but...     */
/* ... strictly speaking this contravenes ANSI's rules on '_' use.          */
#define _ASYMDIAD   0x010L
#define _READ_R1    0x020L      /* see J_CONDEXEC comment for this number   */
#define _SET_R1     0x040L
#define _READ_R2    0x080L
#define _READ_R3    0x100L      /* r3 field is a register rather than literal */
/* the next few bits are regarded JOP adressing modes.                      */
/* N.B. read the BEWARE on LDRK before using                                */
#define _MEM        0x200L      /* distinguishes mem ref from op            */
#define _MEMA       0x600L      /* address of memory - really _MEM+0x400    */
/* *** N.B. but for LDRV1 we could arrange that _MEMA was 0x204 ***         */
/* The following two bits overlay bits otherwise used for non-_MEM things   */
#define _STACKREF   0x008L
#define _IVD        0x010L      /* indexed var + disp (only with _STACKREF) */
/*  spare           0x800L                                                  */
#define J_DEAD_R1  0x1000L      /* for dataflow analysis                    */
#define J_DEAD_R2  0x2000L      /* for dataflow analysis                    */
#define J_DEAD_R3  0x4000L      /* for dataflow analysis                    */
#define J_DEADBITS (J_DEAD_R1+J_DEAD_R2+J_DEAD_R3)
#ifdef TARGET_HAS_SCALED_ADDRESSING
#  define J_NEGINDEX      0x8000L /* currently or-able with load/store only */
#  define J_SHIFTMASK   0xff0000L
#  define J_SHIFTPOS          16
#    define SHIFT_MASK  0x3fL
#    define SHIFT_RIGHT 0x80L
#    define SHIFT_ARITH 0x40L
#endif

/* type bits (reallocate?) ... n.b. Q_UBIT shares with J_UNSIGNED */
#define _FLOATING  0x01000000L
#define _DOUBLE    0x02000000L
#define J_SIGNED   0x04000000L  /* orable with LDRBx, LDRWx, also fix/float */
#define J_UNSIGNED 0x08000000L  /* ditto */

#define J_MASK     (~(Q_MASK | J_DEADBITS))
/* N.B. Q_MASK defines some J_xxx bits from the top end of the word         */
/*  #define Q_MASK 0xf8000000L                                              */

#define J_RTOK(a) ((a) - _READ_R3)
#define J_addflt(a,f) ((a) & ~(_DOUBLE+_FLOATING) | (f))
#define floatiness_(a) ((a) &  _DOUBLE+_FLOATING)
#define J_addvk(a) ((a) -_READ_R2+_STACKREF+_IVD)
#define J_subvk(a) ((a) +_READ_R2-_STACKREF-_IVD)
#define vkformat(a) (((a) & _MEM+_IVD) == _MEM+_IVD)
#define loads_r1(a) ((a) & _SET_R1)
#define reads_r1(a) ((a) & _READ_R1)
#define uses_r1(a)  ((a) & (_SET_R1 | _READ_R1))
#define reads_r2(a) ((a) & _READ_R2)
#define uses_r2(a)  reads_r2(a)
#define reads_r3(a) ((a) & _READ_R3)
#define uses_r3(a)  reads_r3(a)
#define uses_stack(a) (((a) & _STACKREF+_MEM) == _STACKREF+_MEM)
#define tailify_(a)   ((a) + (J_TAILCALLK-J_CALLK))
#ifdef TARGET_COUNT_IS_PROC
#  define isproccall_(op) ((op) == J_CALLK || (op) == J_CALLR \
                                           || (op) == J_COUNT)
#else
#  define isproccall_(op) ((op) == J_CALLK || (op) == J_CALLR)
#endif
#ifdef TARGET_HAS_2ADDRESS_CODE
#  define jop_asymdiadr_(op) \
   (((op) & _ASYMDIAD+_READ_R3+_MEM) == _ASYMDIAD+_READ_R3)
/* maybe insert an is_anydiadr() here, but this think w.r.t. ADDK/ADDR and  */
/* possible uses of load address target instructions.                       */
#endif

/* The following two macros are only used by jopprint.c and regalloc.c --   */
/* this is important since they are not overkeen about masking Q_MASK or    */
/* J_DEADBITS.  Note that they must correspond to the loop optimisation     */
/* code in loopopt.c and its friends in flowgraf.c                          */
/* Note that J_FNCON does not need considering as only this is only used    */
/* in the interface to xxxgen.c from flowgraf.c (q.v.)                      */
#define pseudo_reads_r1(a) (((a) & ~Q_MASK) == J_CMPK)
#define pseudo_reads_r2(a) ((a)==J_MOVK || (a)==J_ADCON || (a)==J_ADCONV)

#define    J_NOOP0 0L
#define    J_NOOP (J_NOOP0+1L)
#define    J_BXX (J_NOOP0+2L)      /* as J_B but within branch table    */
#define    J_B (J_NOOP0+3L)          
#define    J_ENTER (J_NOOP0+4L)    /* special effect on registers */
#define    J_PUSH (J_NOOP0+5L)     /* special effect on registers */
#define    J_POP (J_NOOP0+6L)      /* special effect on registers */
#define    J_PUSHM (J_NOOP0+7L)    /* for the back end, reg map   */
#define    J_POPM (J_NOOP0+8L)     /* ditto                       */
#define    J_LABEL (J_NOOP0+9L)
#define    J_STACK (J_NOOP0+10L)
#define    J_SETSPENV (J_NOOP0+11L)
#define    J_SETSPGOTO (J_NOOP0+12L)
#define    J_SETSP (J_NOOP0+13L)
#define    J_COUNT (J_NOOP0+14L)
#define    J_INFOLINE (J_NOOP0+15L)
#define    J_INFOSCOPE (J_NOOP0+16L)
#define    J_INFOBODY (J_NOOP0+17L)
#define    J_ENDPROC (J_NOOP0+18L)
#ifdef TARGET_HAS_COND_EXEC
#define    J_CONDEXEC (J_NOOP0+19L)
#endif
/* that was the largest block of codes, and is why 5 bits are reserved     */
/* for selection within a class.                                           */

#define    J_NOOP1 _READ_R1
#define    J_CASEBRANCH (J_NOOP1+1L) /* dispatch into branch table          */
#define    J_USE (J_NOOP1+2L)        /* uses R1 (for support of 'volatile') */

#define    J_NOOP2 _SET_R1
#define    J_MOVK (J_NOOP2+1L)       /* see pseudo_reads_r1() (loop opt) */
#define    J_INIT (J_NOOP2+2L)
#define    J_STRING (J_NOOP2+3L)
#define    J_CALLK (J_NOOP2+4L)
#ifdef TARGET_HAS_TAILCALL
#define    J_TAILCALLK (J_NOOP2+5L)            /* see tailify_() */
#endif
#define    J_ADCONF (J_NOOP2+6L)     /* only if SOFTWARE_FLOATING_POINT */
#define    J_ADCOND (J_NOOP2+7L)     /* only if SOFTWARE_FLOATING_POINT */

#define    J_NOOP3 _READ_R1+_SET_R1

#define    J_NOOP4 _READ_R2
#define    J_CMPK (J_NOOP4+1L)       /* see pseudo_reads_r2() (loop opt) */

#define    J_NOOP5 _READ_R2+_READ_R1
#ifdef TARGET_HAS_BLOCKMOVE
#define    J_MOVC (J_NOOP5+1L)
#endif

#define    J_NOOP6 _READ_R2+_SET_R1
#define    J_ADDK (J_NOOP6+1L)
#define    J_MULK (J_NOOP6+2L)
#define    J_ANDK (J_NOOP6+3L)
#define    J_ORRK (J_NOOP6+4L)
#define    J_EORK (J_NOOP6+5L)

#define    J_NOOP6b _READ_R2+_SET_R1+_ASYMDIAD
#define    J_SUBK (J_NOOP6b+1L)
#define    J_DIVK (J_NOOP6b+2L)   /* must or in J_SIGNED/J_UNSIGNED */
#define    J_RSBK (J_NOOP6b+3L)   /* should only happen as J_RSBR+J_SHIFTM */
#define    J_REMK (J_NOOP6b+4L)   /* must or in J_SIGNED/J_UNSIGNED */
#define    J_SHLK (J_NOOP6b+5L)   /* must or in J_SIGNED/J_UNSIGNED */
#define    J_SHRK (J_NOOP6b+6L)   /* must or in J_SIGNED/J_UNSIGNED */

#define    J_NOOP7 _READ_R2+_READ_R1+_SET_R1

#define    J_NOOP8 _READ_R3

#define    J_NOOP9 _READ_R3+_READ_R1

#define    J_NOOP10 _READ_R3+_SET_R1
#define    J_MOVR (J_NOOP10+1L)  /* relationship MOVR,MOVFR,MOVDR in J_addflt */
#define    J_NEGR (J_NOOP10+2L)
#define    J_NOTR (J_NOOP10+3L)
#define    J_CALLR (J_NOOP10+4L)
#ifdef TARGET_HAS_TAILCALL
#define    J_TAILCALLR (J_NOOP10+5L)            /* see tailify_() */
#endif

#define    J_NOOP11 _READ_R3+_READ_R1+_SET_R1

#define    J_NOOP12 _READ_R3+_READ_R2
#define    J_CMPR (J_NOOP12+1L)

#define    J_NOOP13 _READ_R3+_READ_R2+_READ_R1

#define    J_NOOP14 _READ_R3+_READ_R2+_SET_R1
#define    J_ADDR (J_NOOP14+1L)
#define    J_MULR (J_NOOP14+2L)
#define    J_ANDR (J_NOOP14+3L)
#define    J_ORRR (J_NOOP14+4L)
#define    J_EORR (J_NOOP14+5L)

#define    J_NOOP14b _READ_R3+_READ_R2+_SET_R1+_ASYMDIAD
#define    J_SUBR (J_NOOP14b+1L)
#define    J_DIVR (J_NOOP14b+2L)
#define    J_RSBR (J_NOOP14b+3L)   /* should only happen as J_RSBR+J_SHIFTM */
#define    J_REMR (J_NOOP14b+4L)
#define    J_SHLR (J_NOOP14b+5L)
#define    J_SHRR (J_NOOP14b+6L)

#define    J_NOOP15 _READ_R3+_READ_R2+_READ_R1+_SET_R1

#define    J_NOOP16 _FLOATING

#define    J_NOOP17 _FLOATING+_READ_R1
#define    J_PUSHF (J_NOOP17+1L)
#define    J_USEF  (J_NOOP17+2L)

#define    J_NOOP18 _FLOATING+_SET_R1
#define    J_MOVFK (J_NOOP18+1L)
#define    J_INITF (J_NOOP18+2L)

#define    J_NOOP19 _FLOATING+_READ_R1+_SET_R1

#define    J_NOOP20 _FLOATING+_READ_R2
#define    J_CMPFK (J_NOOP20+1L)

#define    J_NOOP21 _FLOATING+_READ_R2+_READ_R1

#define    J_NOOP22 _FLOATING+_READ_R2+_SET_R1
#define    J_ADDFK (J_NOOP22+1L)
#define    J_MULFK (J_NOOP22+2L)

#define    J_NOOP22b _FLOATING+_READ_R2+_SET_R1+_ASYMDIAD
#define    J_SUBFK (J_NOOP22b+1L)
#define    J_DIVFK (J_NOOP22b+2L)

#define    J_NOOP23 _FLOATING+_READ_R2+_READ_R1+_SET_R1

#define    J_NOOP24 _FLOATING+_READ_R3

#define    J_NOOP25 _FLOATING+_READ_R3+_READ_R1

#define    J_NOOP26 _FLOATING+_READ_R3+_SET_R1
#define    J_MOVFR (J_NOOP26+1L)  /* relationship MOVR,MOVFR,MOVDR in J_addflt */
#define    J_NEGFR (J_NOOP26+2L)
#define    J_FLTFR (J_NOOP26+3L)
#define    J_FIXFR (J_NOOP26+4L)
#define    J_MOVDFR (J_NOOP26+5L)
#define    J_MOVFR1 (J_NOOP26+6L)

#define    J_NOOP27 _FLOATING+_READ_R3+_READ_R1+_SET_R1

#define    J_NOOP28 _FLOATING+_READ_R3+_READ_R2
#define    J_CMPFR (J_NOOP28+1L)

#define    J_NOOP29 _FLOATING+_READ_R3+_READ_R2+_READ_R1

#define    J_NOOP30 _FLOATING+_READ_R3+_READ_R2+_SET_R1
#define    J_ADDFR (J_NOOP30+1L)
#define    J_MULFR (J_NOOP30+2L)

#define    J_NOOP30b _FLOATING+_READ_R3+_READ_R2+_SET_R1+_ASYMDIAD
#define    J_SUBFR (J_NOOP30b+1L)
#define    J_DIVFR (J_NOOP30b+2L)

#define    J_NOOP31 _FLOATING+_READ_R3+_READ_R2+_READ_R1+_SET_R1

#define    J_NOOP32 _DOUBLE

#define    J_NOOP33 _DOUBLE+_READ_R1
#define    J_PUSHD (J_NOOP33+1L)
#define    J_USED  (J_NOOP33+2L)

#define    J_NOOP34 _DOUBLE+_SET_R1
#define    J_MOVDK (J_NOOP34+1L)
#define    J_INITD (J_NOOP35+2L)

#define    J_NOOP35 _DOUBLE+_READ_R1+_SET_R1

#define    J_NOOP36 _DOUBLE+_READ_R2
#define    J_CMPDK (J_NOOP36+1L)

#define    J_NOOP37 _DOUBLE+_READ_R2+_READ_R1

#define    J_NOOP38 _DOUBLE+_READ_R2+_SET_R1
#define    J_ADDDK (J_NOOP38+1L)
#define    J_MULDK (J_NOOP38+2L)

#define    J_NOOP38b _DOUBLE+_READ_R2+_SET_R1+_ASYMDIAD
#define    J_SUBDK (J_NOOP38b+1L)
#define    J_DIVDK (J_NOOP38b+2L)

#define    J_NOOP39 _DOUBLE+_READ_R2+_READ_R1+_SET_R1

#define    J_NOOP40 _DOUBLE+_READ_R3

#define    J_NOOP41 _DOUBLE+_READ_R3+_READ_R1

#define    J_NOOP42 _DOUBLE+_READ_R3+_SET_R1
#define    J_MOVDR (J_NOOP42+1L)  /* relationship MOVR,MOVFR,MOVDR in J_addflt */
#define    J_NEGDR (J_NOOP42+2L)
#define    J_FLTDR (J_NOOP42+3L)
#define    J_FIXDR (J_NOOP42+4L)
#define    J_MOVFDR (J_NOOP42+5L)

#define    J_NOOP43 _DOUBLE+_READ_R3+_READ_R1+_SET_R1

#define    J_NOOP44 _DOUBLE+_READ_R3+_READ_R2
#define    J_CMPDR (J_NOOP44+1L)

#define    J_NOOP45 _DOUBLE+_READ_R3+_READ_R2+_READ_R1

#define    J_NOOP46 _DOUBLE+_READ_R3+_READ_R2+_SET_R1
#define    J_ADDDR (J_NOOP46+1L)
#define    J_MULDR (J_NOOP46+2L)
#define    J_MOVDR1 (J_NOOP46+3L)    /* a bit special - doesn't require r1/r3 clash. */
#define    J_NOOP46b _DOUBLE+_READ_R3+_READ_R2+_SET_R1+_ASYMDIAD
#define    J_SUBDR (J_NOOP46b+1L)
#define    J_DIVDR (J_NOOP46b+2L)

#define    J_NOOP47 _DOUBLE+_READ_R3+_READ_R2+_READ_R1+_SET_R1

/* versions with wilder addressing modes:                                   */

#define    J_NOOP6I _MEM+_READ_R2+_SET_R1
/* BEWARE the next few things do NOT have immediate equivalents (notionally */
/* they would be things like J_MOV).                                        */
#define    J_LDRK (J_NOOP6I+1L)   /* relationship LDRK,LDRFK,LDRDK in J_addflt */
#define    J_LDRBK (J_NOOP6I+2L)
#define    J_LDRWK (J_NOOP6I+3L)
#define    J_LDRFK (J_LDRK+_FLOATING)
#define    J_LDRDK (J_LDRK+_DOUBLE)
#define    J_LDRVK (J_LDRK-_READ_R2+_STACKREF+_IVD)
#define    J_LDRBVK (J_LDRBK-_READ_R2+_STACKREF+_IVD)
#define    J_LDRWVK (J_LDRWK-_READ_R2+_STACKREF+_IVD)
#define    J_LDRFVK (J_LDRFK-_READ_R2+_STACKREF+_IVD)
#define    J_LDRDVK (J_LDRDK-_READ_R2+_STACKREF+_IVD)

#define    J_NOOP14I _MEM+_READ_R3+_READ_R2+_SET_R1
#define    J_LDRR (J_NOOP14I+1L)
#define    J_LDRBR (J_NOOP14I+2L)
#define    J_LDRWR (J_NOOP14I+3L)
#define    J_LDRFR (J_LDRR+_FLOATING)
#define    J_LDRDR (J_LDRR+_DOUBLE)

#define    J_NOOP2V _MEM+_SET_R1+_STACKREF
#define    J_LDRV (J_NOOP2V+1L)
#ifdef never /* removed (see cg_var).  However, may return soon */
#define    J_LDRBV (J_NOOP2V+2L)
#define    J_LDRWV (J_NOOP2V+3L)
#endif
#define    J_LDRV1 (J_NOOP2V+4L)
#define    J_LDRFV (J_LDRV+_FLOATING)
#define    J_LDRFV1 (J_LDRV1+_FLOATING)
#define    J_LDRDV (J_LDRV+_DOUBLE)
#define    J_LDRDV1 (J_LDRV1+_DOUBLE)

#define    J_NOOP5I _MEM+_READ_R2+_READ_R1
#define    J_STRK (J_NOOP5I+1L)   /* relationship STRK,STRFK,STRDK in J_addflt */
#define    J_STRBK (J_NOOP5I+2L)
#ifndef TARGET_LACKS_HALFWORD_STORE
# define   J_STRWK (J_NOOP5I+3L)
#endif
#define    J_STRFK (J_STRK+_FLOATING)
#define    J_STRDK (J_STRK+_DOUBLE)
#define    J_STRVK (J_STRK-_READ_R2+_STACKREF+_IVD)
#define    J_STRBVK (J_STRBK-_READ_R2+_STACKREF+_IVD)
#ifndef TARGET_LACKS_HALFWORD_STORE
# define   J_STRWVK (J_STRWK-_READ_R2+_STACKREF+_IVD)
#endif
#define    J_STRFVK (J_STRFK-_READ_R2+_STACKREF+_IVD)
#define    J_STRDVK (J_STRDK-_READ_R2+_STACKREF+_IVD)

#define    J_NOOP13I _MEM+_READ_R3+_READ_R2+_READ_R1
#define    J_STRR (J_NOOP13I+1L)   /* must be first - see STRK */
#define    J_STRBR (J_NOOP13I+2L)
#ifndef TARGET_LACKS_HALFWORD_STORE
# define   J_STRWR (J_NOOP13I+3L)
#endif
#define    J_STRFR (J_STRR+_FLOATING)
#define    J_STRDR (J_STRR+_DOUBLE)

#define    J_NOOP1V _MEM+_READ_R1+_STACKREF
#define    J_STRV (J_NOOP1V+1L)
#ifdef never /* removed (see cg_var).  However, may return soon */
#define    J_STRBV (J_NOOP1V+2L)
#define    J_STRWV (J_NOOP1V+3L)
#endif
#define    J_STRFV (J_STRV+_FLOATING)
#define    J_STRDV (J_STRV+_DOUBLE)

#define    J_NOOP6x _MEMA+_SET_R1
#define    J_ADCON (J_NOOP6x+1L)   /* see pseudo_reads_r2() (loop opt) */
#define    J_FNCON (J_NOOP6x+2L)   /* only if TARGET_CALL_USES_DESCRIPTOR */

#define    J_ADCONV (J_ADCON+_STACKREF)    /* Also special use of r2 field */

/*************************************/
/* end of emumeration of all opcodes */
/*************************************/

typedef int32 J_OPCODE; /* really ENUMERATED_OPCODE but ensure 32 bits */

/* n.b Q_UBIT is the same as J_UNSIGNED */
#define Q_MASK           0xf8000000L
#define Q_UBIT           0x08000000L   /* for test of signed/unsigned CMP   */
#define Q_NEGATE(x) ((x)^0x10000000L)
/* N.B. individual machine translators may rely on the cond values below -  */
/* the routine/macro C_FROMQ() does this.                                   */
/* The following idea of conditions are machine independent - so any new    */
/* code (e.g. Q_CC) should have a different number even if they have the    */
/* same effect on your favorite machine - just as Q_HS and Q_GE differ even */
/* though they have the same effect on the 370 (the comparison differs).    */

#define Q_EQ    (0x00000000L^0xe0000000L)
#define Q_NE    (0x10000000L^0xe0000000L)
#define Q_HS    ((0x20000000L^0xe0000000L)|Q_UBIT)
#define Q_LO    ((0x30000000L^0xe0000000L)|Q_UBIT)
/* #define Q_MI    (0x40000000L^0xe0000000L) */
/* #define Q_PL    (0x50000000L^0xe0000000L) */
/* #define Q_VS    (0x60000000L^0xe0000000L) */
/* #define Q_VC    (0x70000000L^0xe0000000L) */
#define Q_HI    ((0x80000000L^0xe0000000L)|Q_UBIT)
#define Q_LS    ((0x90000000L^0xe0000000L)|Q_UBIT)
#define Q_GE    (0xa0000000L^0xe0000000L)
#define Q_LT    (0xb0000000L^0xe0000000L)
#define Q_GT    (0xc0000000L^0xe0000000L)
#define Q_LE    (0xd0000000L^0xe0000000L)
#define Q_AL    (0xe0000000L^0xe0000000L)
#define Q_NOT   (0xf0000000L^0xe0000000L)
/* The following are added by AM to check conversion to new style J_CMP */
/* Q_UEQ and Q_UNE are unsigned equality tests.  May be useful one day? */
#define Q_UEQ    (Q_EQ|Q_UBIT)
#define Q_UNE    (Q_NE|Q_UBIT)
#define Q_XXX    (0x60000000L^0xe0000000L)  /* flag CMP use for 3 way tests */



#define blkcode_(x)     (x->code)   /* start of 3-address code           */
#define blklength_(x)   (x->length) /* length of ditto                   */
#define blkflags_(x)    (x->flags)  /* misc flag bits                    */
#define blknext_(x)     (x->succ1.next)    /* successor block          */
#define blknext1_(x)    (x->succ2.next1)   /* alternative successor    */
#define blktable_(x)    (x->succ1.table)   /* table of successors      */
#define blktabsize_(x)  (x->succ2.tabsize) /* size of aforesaid table  */
#define blkbackp_(x)    (x->succ1.backp)   /* for branch_chain() only  */
#define blkbackp1_(x)   (x->succ2.backp1)  /* for branch_chain() only  */
#define blkdown_(x)     (x->down)   /* forward chaining                  */
#define blkup_(x)       (x->up)     /* backward chaining                 */
#define blklab_(x)      (x->lab)    /* label for this block              */
#define blkuse_(x)      (x->use)    /* registers needed at block head    */
#define blkstack_(x)    (x->stack)  /* binders active at head of block   */
#define blkdebenv_(x)   (x->debenv) /* for debugger                    */
#define BLKSIZE         ((int32)sizeof(block_head))

/* bits for use in blkflags_(x)                                          */

#define BLKREFMASK      3L          /* refcount 0,1,2,many               */
#define BLKALIVE        4L          /* used when flattening graph        */
#define BLKSWITCH       8L          /* contains 'switch' multi-exit      */
#define BLK2EXIT     0x10L          /* contains conditional exit         */
#define BLKBUSY      0x20L          /* used when flattening graph        */
#define BLKCODED     0x40L          /* ditto                             */
#define BLKEMPTY     0x80L          /* SET IF BLOCK EMPTY                */
#define BLKLOOP     0x100L          /* block is a loop head              */
#define BLKINNER    0x200L          /* loop has no other loops inside    */
#define BLKOUTER    0x400L          /* set in function header block      */
#define BLK0EXIT    0x800L          /* no exit (tail procedure call)     */
#define BLKP2      0x1000L          /* bits used to control backpointer  */
#define BLKP3      0x2000L          /* bits used to control backpointer  */
#define BLKCALL    0x4000L          /* block contains a proc call        */
#define BLK2CALL   0x8000L          /* block contains 2 proc calls       */
#define BLKSETJMP 0x10000L          /* block may call setjmp (groan).    */
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

extern void print_jopcode(J_OPCODE op, VRegnum r1, VRegnum r2, int32 m);
extern void emitfl(J_OPCODE op, FileLine fl);
extern void emit5(J_OPCODE op, VRegnum r1, VRegnum r2, VRegnum r3, int32 m);
extern void emitstring(J_OPCODE op, VRegnum r1, StringSegList *m);
extern void emitbranch(J_OPCODE op, LabelNumber *m);
extern void emitbinder(J_OPCODE op, VRegnum r1, Binder *m);
extern void emitvk(J_OPCODE op, VRegnum r1, int32 n, Binder *m);
extern void emitreg(J_OPCODE op, VRegnum r1, VRegnum r2, VRegnum m);
extern void emitfloat(J_OPCODE op, VRegnum r1, VRegnum r2, FloatCon *m);
extern void emitpush(J_OPCODE op, VRegnum r1, VRegnum r2, RegList *m);
extern void emitsetsp(J_OPCODE op, BindList *b1, BindList *b2);
extern void emit(J_OPCODE op, VRegnum r1, VRegnum r2, int32 m);
extern void show_instruction(J_OPCODE op, RealRegister r1, RealRegister r2,
                             int32 m);
extern void print_xjopcode(J_OPCODE op, VRegnum r1, VRegnum r2, int32 m,
                    char *fmt, ...);

extern int32 procflags;
extern int32 greatest_stackdepth;
extern int32 max_argsize;

#endif

/* End of jopcode.h */
