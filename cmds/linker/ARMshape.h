/*> ARMshape.h <*/
/*---------------------------------------------------------------------------*/
/* RcsId: $Id: ARMshape.h,v 1.9 1994/01/26 15:01:46 vlsi Exp $ */

extern int pcsregs;		/* TRUE == display PCS register aliases */

#define TRUE    (1)
#define FALSE   (0)

#define wordsize     (4)        /* ARM word size in bytes */

/* align upto the next word boundary */
#define wordalign(v) ((v + (wordsize - 1)) & -wordsize)

#if !defined __sys_types_h && !defined RS6000 /* NASTY : we have a clash of uint definitions otherwise */
typedef unsigned int uint ;     /* general abbreviation */
#if 0
typedef unsigned int word ;
#endif
#endif

/*---------------------------------------------------------------------------*/
/* ARM processor description */

/* The bits in the PSR */
#define Nbit            (1 << 31)
#define Zbit            (1 << 30)
#define Cbit            (1 << 29)
#define Vbit            (1 << 28)
#define Ibit            (1 << 27)
#define Fbit            (1 << 26)
#define r1bit           (1 <<  1)
#define r0bit           (1 <<  0)
#define MODEmask        (0x00000003)
#define FLAGbits        (0xFC000000)
#define PSRbits         (0xFC000003)

#define USRmode         (0x00000000)
#define FIQmode         (0x00000001)
#define IRQmode         (0x00000002)
#define SVCmode         (0x00000003)

/* hardware vectors */
#define vec_reset       (0x00000000)
#define vec_undefins    (0x00000004)
#define vec_SWI         (0x00000008)
#define vec_prefetch    (0x0000000C)
#define vec_dataabort   (0x00000010)
#define vec_address     (0x00000014)
#define vec_IRQ         (0x00000018)
#define vec_FIQ         (0x0000001C)

/*---------------------------------------------------------------------------*/
/* ARM instruction format */

/* conditional execution values */
#define c_EQ    (0x00000000)    /* Z   */
#define c_NE    (0x10000000)    /* Z   */
#define c_CS    (0x20000000)    /* C   */
#define c_HS    (0x20000000)    /* C   */ /* synonym for CS */
#define c_CC    (0x30000000)    /* C   */
#define c_LO    (0x30000000)    /* C   */ /* synonym for CC */
#define c_MI    (0x40000000)    /* N   */
#define c_PL    (0x50000000)    /* N   */
#define c_VS    (0x60000000)    /* V   */
#define c_VC    (0x70000000)    /* V   */
#define c_HI    (0x80000000)    /* CZ  */
#define c_LS    (0x90000000)    /* CZ  */
#define c_GE    (0xA0000000)    /* NV  */
#define c_LT    (0xB0000000)    /* NV  */
#define c_GT    (0xC0000000)    /* NVZ */
#define c_LE    (0xD0000000)    /* NVZ */
#define c_AL    (0xE0000000)    /*     */ /* always */
#define c_NV    (0xF0000000)    /*     */ /* never */

/* condition execution masks */
#define c_execute       (0x10000000)    /* decide if instruction is executed */
#define c_flags         (0xE0000000)    /* which flags are used */
#define c_flags_shift   (29)            /* where the flags live in a word */

/* major opcode groups */
#define op_regxreg1     (0x00000000)
#define op_regxreg2     (0x01000000)
#define op_regximm1     (0x02000000)
#define op_regximm2     (0x03000000)
#define op_postimm      (0x04000000)    /* load/store immediate post offset */
#define op_preimm       (0x05000000)    /* load/store immediate pre offset */
#define op_postreg      (0x06000000)    /* load/store register post offset */
#define op_prereg       (0x07000000)    /* load/store register pre offset */
#define op_ldmstm1      (0x08000000)
#define op_ldmstm2      (0x09000000)
#define op_b            (0x0A000000)
#define op_bl           (0x0B000000)
#define op_cppost       (0x0C000000)    /* co-processor data post transfer */
#define op_cppre        (0x0D000000)    /* co-processor data pre transfer */
#define op_cpop         (0x0E000000)    /* co-processor data op or reg trans */
#define op_swi          (0x0F000000)

#define op_mask         (0x0F000000)    /* opcode type mask */
#define op_shift        (24)            /* opcode type shift */

/* data-processing operations */
#define dp_mask         (0x01E00000)
#define dp_shift        (21)
#define dp_and          (0x0)   /*  Rd = Op1 AND Op2     */
#define dp_eor          (0x1)   /*  Rd = Op1 EOR Op2     */
#define dp_sub          (0x2)   /*  Rd = Op1  -  Op2     */
#define dp_rsb          (0x3)   /*  Rd = Op2  -  Op1     */
#define dp_add          (0x4)   /*  Rd = Op1  +  Op2     */
#define dp_adc          (0x5)   /*  Rd = Op1  +  Op2 + C */
#define dp_sbc          (0x6)   /*  Rd = Op1  -  Op2 + C */
#define dp_rsc          (0x7)   /*  Rd = Op2  -  Op1 + C */
#define dp_tst          (0x8)   /* PSR = Op1 AND Op2     */
#define dp_teq          (0x9)   /* PSR = Op1 EOR Op2     */
#define dp_cmp          (0xA)   /* PSR = Op1  -  Op2     */
#define dp_cmn          (0xB)   /* PSR = Op1  +  Op2     */
#define dp_orr          (0xC)   /*  Rd = Op1 OR  Op2     */
#define dp_mov          (0xD)   /*  Rd = Op2             */
#define dp_bic          (0xE)   /*  Rd = Op1 AND NOT Op2 */
#define dp_mvn          (0xF)   /*  Rd = NOT Op2         */

/* load/store multiple operations */
#define op_LDMDA        (0x08100000)
#define op_LDMDB        (0x09100000)
#define op_LDMIA        (0x08900000)
#define op_LDMIB        (0x09900000)

#define op_STMDA        (0x08000000)
#define op_STMDB        (0x09000000)
#define op_STMIA        (0x08800000)
#define op_STMIB        (0x09800000)

/* floating point operations */

#define f_single        (0x00000100)
#define f_double        (0x00400100)    /* ((1 << 22) | (1 << 8)) */

/* opcodes for CPRT group */
#define f_FLTS          (0x00000110)
#define f_FIXS          (0x00100110)
#define f_TRNS          (0x00300110)
#define f_FLTD          (0x00400110)
#define f_FIXD          (0x00500110)
#define f_TRND          (0x00700110)
#define f_FPSWR         (0x00800110)
#define f_RFPSW         (0x00900110)
#define f_RTOF          (0x00A00110)
#define f_FTOR          (0x00B00110)
#define f_RTOF1         (0x00C00110)
#define f_FTOR1         (0x00D00110)
#define f_RTOF2         (0x00E00110)
#define f_FTOR2         (0x00F00110)

/* opcodes for more floating point operations */
#define f_regop         (0x00000000)
#define f_constop       (0x00800000)

#define f_CMF           (0x0010F030)
#define f_CNF           (0x0030F030)

#define f_ADF           (0x00000000)
#define f_SUF           (0x00000020)
#define f_RSF           (0x00000040)
#define f_ASF           (0x00000060)
#define f_MUF           (0x00100000)
#define f_DIF           (0x00100020)
#define f_RDF           (0x00100040)
#define f_spare1        (0x00100060)
#define f_CVT           (0x00200000)
#define f_ABS           (0x00200020)
#define f_SQT           (0x00200040)
#define f_MVF           (0x00200060)
#define f_MNF           (0x00300000)
#define f_LOG           (0x00300020)
#define f_LGN           (0x00300040)
#define f_EXP           (0x00300060)
#define f_POW           (0x00000080)
#define f_RPW           (0x000000A0)
#define f_spare2        (0x000000C0)
#define f_spare3        (0x000000E0)
#define f_spare4        (0x00100080)
#define f_spare5        (0x001000A0)
#define f_spare6        (0x001000C0)
#define f_spare7        (0x001000E0)
#define f_SIN           (0x00200080)
#define f_COS           (0x002000A0)
#define f_TAN           (0x002000C0)
#define f_ASN           (0x002000E0)
#define f_ACS           (0x00300080)
#define f_ATN           (0x003000A0)
#define f_spare8        (0x003000C0)
#define f_spare9        (0x003000E0)

/*---------------------------------------------------------------------------*/
/* shift constructions */

#define reg_shiftmask   (0x00000FF0)    /* register shift mask */
#define reg_shiftshift  (4)
#define imm_shiftmask   (0x00000F00)    /* immediate value rotation */
#define imm_shiftshift  (7)             /* generates (rotator * 2) */
#define imm_valuemask   (0x000000FF)    /* immediate value */

#define shift_reg       (1 << 4)        /* immediate shift/register shift */

#define simm_mask       (0x00000F80)    /* immediate shift value mask */
#define simm_shift      (0x00000060)    /* immediate shift operation */

/* NOTE: register/register shifts have bit7 permanently unset */
/*       since bit7 set for register shifts encodes the MUL/MLA instructions */
#define sreg_mask       (0x00000F00)    /* register shift register mask */
#define sreg_shift      (0x00000060)    /* register shift operation */

#define shift_lsl       (0x00000000)    /* logical shift left */
#define shift_lsr       (0x00000020)    /* logical shift right */
#define shift_asr       (0x00000040)    /* arithmetic shift right */
#define shift_ror       (0x00000060)    /* rotate right */

/* NOTEs: "ROR #0" encodes "RRX" (Rotate Right with eXtend) */
/*        "ASR #0" encodes "ASR #32" */
/*        "LSR #0" encodes "LSR #32" */
/*        "LSR #0", "ASR #0" and "ROR #0" are converted to "LSL #0" */
/*---------------------------------------------------------------------------*/
/* instruction flags */

#define scc             (1 << 20)       /* Set Condition Codes */
#define ab              (1 << 21)       /* Accumulate Bit */
#define lsb             (1 << 20)       /* Load/Store Bit */
#define wbb             (1 << 21)       /* Write Back Bit */
#define bwb             (1 << 22)       /* Byte Word Bit */
#define udb             (1 << 23)       /* Up Down bit */
#define ppi             (1 << 24)       /* Pre Post Indexing bit */
#define immoff          (1 << 25)       /* IMMediate OFFset bit */
#define psrfu           (1 << 22)       /* PSR and Force User bit */

/* co-processor instructions */
#define regtrans        (1 << 4)        /* co-proc register transfer */
#define cpnum_mask      (0xF << 8)      /* co-proc number */
#define cpinfo_mask     (0x7 << 5)      /* co-processor information */
#define cpopcode_mask   (0xF << 20)     /* co-processor opcode (data op) */
#define cpopcode2_mask  (0x7 << 21)     /* co-processor opcode (reg trans) */
/* co-proc data transfer flags same as normal instruction flags */
#define cpnum(ins)      ((ins & cpnum_mask) >> 8)
#define cpopcode(ins)   ((ins & cpopcode_mask) >> 20)
#define cpopcode2(ins)  ((ins & cpopcode2_mask) >> 21)
#define cpinfo(ins)     ((ins & cpinfo_mask) >> 5)
#define cp_offset_mask  (0x000000FF)

#define ls_offset_mask  (0x00000FFF)    /* load store immediate offset */

#define Rn(ins)         (((ins) >> 16) & 0xF) /* get Rn from instruction */
#define Rd(ins)         (((ins) >> 12) & 0xF) /* get Rd from instruction */
#define Rs(ins)         (((ins) >>  8) & 0xF) /* get Rs from instruction */
#define Rm(ins)         (((ins) >>  0) & 0xF) /* get Rm from instruction */

#define regmask         (0x0000FFFF)    /* register bit mask in LDM/STM */
#define regbit(n)       (1 << (n))      /* register mask bit in LDM/STM */

/*---------------------------------------------------------------------------*/

int  pad(int tab,int col) ;

/*---------------------------------------------------------------------------*/
/*> EOF ARMshape.h <*/
