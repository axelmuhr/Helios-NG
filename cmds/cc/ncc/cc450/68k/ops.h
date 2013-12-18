
/* C compiler file m68k/ops.h :  Copyright (C) Codemist Ltd., 1988.     */
/* version 3a */

#ifndef _m68ops_loaded
#define _m68ops_loaded 1

#ifdef COMPILING_ON_TRIPOS
extern int callglb(int g, int n, ...);
#define g_abort 38
#define trap(n) callglb(g_abort, 1, n)
#endif

/* M68000 opcodes... */

/* Condition fields are defined relative to the default C_ALWAYS value   */

/* N.B. the next line relies on the Q_ values in jopcode.h               */
#define C_FROMQ(q)  to_m68c[(((q) & 0xf0000000L) >> 28)&0xf]
#define C_ALWAYS     0x0L
#define C_CC        (0x4L)
#define C_HS        (0x4L)
#define C_CS        (0x5L)
#define C_LO        (0x5L)
#define C_EQ        (0x7L)
#define C_GE        (0xCL)
#define C_GT        (0xEL)
#define C_HI        (0x2L)
#define C_LE        (0xFL)
#define C_LS        (0x3L)
#define C_LT        (0xDL)
#define C_MI        (0xBL)
#define C_NE        (0x6L)
#define C_NEVER     (0x1L)   /* DBRA only */
#define C_PL        (0xAL)
#define C_VC        (0x8L)
#define C_VS        (0x9L)

#ifdef TARGET_IS_68020
#define FC_FROMQ(q) to_fpc[(((q) & 0xf0000000L) >> 28)&0xf]
#define FC_EQ       (0x01L)
#define FC_NE       (0x0eL)

#define FC_GT       (0x12L)
#define FC_NGT      (0x1dL)
#define FC_GE       (0x13L)
#define FC_NGE      (0x1cL)
#define FC_LT       (0x14L)
#define FC_NLT      (0x1bL)
#define FC_LE       (0x15L)
#define FC_NLE      (0x1aL)
#define FC_GL       (0x16L)
#define FC_NGL      (0x19L)
#define FC_GLE      (0x17L)
#define FC_NGLE     (0x18L)

#define FC_OGT      (0x02L)
#define FC_ULE      (0x0dL)
#define FC_OGE      (0x03L)
#define FC_ULT      (0x0cL)
#define FC_OLT      (0x04L)
#define FC_UGE      (0x0bL)
#define FC_OLE      (0x05L)
#define FC_UGT      (0x0aL)
#define FC_OGL      (0x06L)
#define FC_UEQ      (0x09L)
#define FC_OR       (0x07L)
#define FC_UN       (0x08L)
#endif

typedef enum {
OP_ORI=0, OP_ANDI,   OP_SUBI,   OP_ADDI,  OP_EORI,
OP_CMPI,  OP_BTSTS,  OP_BCHGS,  OP_BCLRS, OP_BSETS,
OP_NEGX,  OP_CLR,    OP_NEG,    OP_NOT,   OP_TST,
OP_PEA,   OP_JSR,    OP_JMP,    OP_MOVEM, OP_CHK,
OP_LEA,   OP_MOVEAL, OP_MOVEAW, OP_BTSTD, OP_BCHGD,
OP_BCLRD, OP_BSETD,  OP_SCC,    OP_OR,    OP_SUB,
OP_SUBAW, OP_SUBAL,  OP_CMP,    OP_CMPAW, OP_CMPAL,
OP_EOR,   OP_ADD,    OP_ADDAW,  OP_ADDAL, OP_AND,
OP_DIVU,  OP_DIVS,   OP_MULU,   OP_MULS,  OP_ADDQ,
OP_SUBQ,  OP_ASLM,   OP_ASRM,   OP_LSLM,  OP_LSRM,
OP_ROLM,  OP_RORM,   OP_ROXLM,  OP_ROXRM, OP_MOVE,
OP_MOVEQ, OP_BCC,    OP_BSR,    OP_RTS,   OP_DBCC,
OP_SWAP,  OP_EXTW,   OP_EXTL,   OP_LINK,  OP_UNLK,
OP_EXG,   OP_ADDX,   OP_SUBX,   OP_ASLR,  OP_ASRR,
OP_LSLR,  OP_LSRR,   OP_ROLR,   OP_RORR,  OP_ROXLR,
OP_ROXRR,  OP_NOOP,
#ifdef TARGET_IS_68020
OP_BFCHG, OP_BFCLR,  OP_BFEXTS, OP_BFEXTU,OP_BFFFO,
OP_BFINS, OP_BFSET,  OP_BFTST,  OP_DIVL,  OP_EXTBL,
OP_LINKL, OP_MULL,   OP_RTD,
#ifndef SOFTWARE_FLOATING_POINT
OP_FADD,        OP_FSUB,    OP_FMUL,    OP_FDIV, OP_FMOVETOFP,
OP_FMOVEFROMFP, OP_FSGLMUL, OP_FSGLDIV, OP_FNEG, OP_FCMP,
OP_FBCC,        OP_FSCC,    OP_FMOVEM,
#endif
#endif
OP_SIZE
} instrtype ;

#define M_ORI    0x0000
#define M_ANDI   0x0200
#define M_SUBI   0x0400
#define M_ADDI   0x0600
#define M_EORI   0x0a00
#define M_CMPI   0x0c00
#define M_BTSTS  0x0800
#define M_BCHGS  0x0840
#define M_BCLRS  0x0880
#define M_BSETS  0x08c0
#define M_NEGX   0x4000
#define M_CLR    0x4200
#define M_NEG    0x4400
#define M_NOT    0x4600
#define M_TST    0x4a00
#define M_PEA    0x4840
#define M_JSR    0x4e80
#define M_JMP    0x4ec0
#define M_MOVEM  0x4880
#define TOREG  1
#define TOMEM  0
#define M_CHK    0x4180
#define M_LEA    0x41c0
#define M_MOVEAL 0x2040
#define M_MOVEAW 0x3040
#define M_BTSTD  0x0100
#define M_BCHGD  0x0140
#define M_BCLRD  0x0180
#define M_BSETD  0x01c0
#define M_SCC    0x50c0
#define M_OR     0x8000
#define M_SUB    0x9000
#define M_SUBAW  0x90c0
#define M_SUBAL  0x91c0
#define M_CMP    0xb000
#define M_CMPAW  0xb0c0
#define M_CMPAL  0xb1c0
#define M_EOR    0xb000
#define M_ADD    0xd000
#define M_ADDAW  0xd0c0
#define M_ADDAL  0xd1c0
#define M_AND    0xc000
#define M_DIVU   0x80c0
#define M_DIVS   0x81c0
#define M_MULU   0xc0c0
#define M_MULS   0xc1c0
#define M_ADDQ   0x5000
#define M_SUBQ   0x5100
#define M_ASLM   0xe1c0
#define M_ASRM   0xe0c0
#define M_LSLM   0xe3c0
#define M_LSRM   0xe2c0
#define M_ROLM   0xe7c0
#define M_RORM   0xe6c0
#define M_ROXLM  0xe5c0
#define M_ROXRM  0xe4c0
#define M_MOVE   0x0000
#define M_MOVEQ  0x7000
#define M_BCC    0x6000
#define M_BSR    0x6100
#define M_RTS    0x4e75
#define M_DBCC   0x50c8
#define M_SWAP   0x4840
#define M_EXTW   0x4880
#define M_EXTL   0x48c0
#define M_LINK   0x4e50
#define M_UNLK   0x4e58
#define M_EXG    0xc100
#define M_ADDX   0xd100
#define M_SUBX   0x9100
#define M_ASLR   0xe100
#define M_ASRR   0xe000
#define M_LSLR   0xe108
#define M_LSRR   0xe008
#define M_ROLR   0xe118
#define M_RORR   0xe018
#define M_ROXLR  0xe110
#define M_ROXRR  0xe010
#define M_NOOP   0x4e71

#ifdef TARGET_IS_68020
#define M_BFCHG  0xeac0
#define M_BFCLR  0xecc0
#define M_BFEXTS 0xebc0
#define M_BFEXTU 0xe9c0
#define M_BFFFO  0xedc0
#define M_BFINS  0xefc0
#define M_BFSET  0xeec0
#define M_BFTST  0xe8c0
#define M_DIVL   0x4c40
#define M_EXTBL  0x49c0
#define M_LINKL  0x4808
#define M_MULL   0x4c00
#define M_RTD    0x4e74
#ifndef SOFTWARE_FLOATING_POINT
#define M_FADD   0x0022       /* used with outfpinstr */
#define M_FSUB   0x0028       /*   "   "       "      */
#define M_FMUL   0x0023       /*   "   "        "     */
#define M_FDIV   0x0020       /*   "   "        "     */
#define M_FMOVETOFP   0x0     /*   "   "       "      */
#define M_FMOVEFROMFP 0x6000  /*   "   "       "      */
#define M_FSGLMUL 0x0027      /*   "   "       "      */
#define M_FSGLDIV 0x0024      /*   "   "       "      */
#define M_FNEG    0x001a      /*   "   "       "      */
#define M_FCMP    0x0038      /*   "   "       "      */
#define M_FMOVEM  0xc000      /*   "   "       "      */
#define M_FBCC    0xF280      /* Used with outinstr */
#define M_FSCC    0xF240      /* Used with outinstr */
#endif
#endif

#define EM_D 0x0000l
#define EM_A 0x8000l
#define EM_W 0x0000l
#define EM_L 0x0800l

#ifdef TARGET_IS_68020
#define FPF_L  0
#define FPF_S  1
#define FPF_X  2
#define FPF_P  3
#define FPF_W  4
#define FPF_D  5
#define FPF_B  6

#define M 1
#define R 0

#define EM_TIMES1 0x0000l
#define EM_TIMES2 0x0200l
#define EM_TIMES4 0x0400l
#define EM_TIMES8 0x0600l
#define EM_SHORT  0x0000l
#define EM_FULL   0x0100l
#define EM_BS     0x0080l
#define EM_IS     0x0040l
#define EM_IS0    0x0l
#define EM_IS1    0x1l
#define EM_IS5    0x5l
#endif

typedef struct fieldstr {
   unsigned int mask;
            int shift;
} fieldstr;

typedef struct instr {
   int      baseval;
   int32    extwords;
   int      nfields;
   fieldstr field[4];
} instr;

#ifdef ARGS_ON_STACK            /* temp name */
# ifdef TARGET_IS_ATARI
/* This enables the calling conventions used by LATTICE C5:             */
/* regs D0,D1,A0,A1 are trashable; regs D2-D7, A2,A3,A5 are for         */
/* register variables. A4 must not be used.                             */ 
/* #define R_A1    0x0L     / * main result register  (D0)           */
#define R_AS2   0x1L     /* regalloc'd scratch A-reg          (A0)       */
#define R_DS    0x2L     /* non-regalloc'd R_IP and scratch   (D1)       */
#define R_AS    0x3L     /* Scratch A-reg                     (A1)       */
/* #define R_V1    0x4L     / * register variable 1               (D2)       */
#define R_DS2    R_V1    /* Also used as 2nd D-reg temp (saved/restored) */
/* #define R_V2    0x5L     / * register variable 2               (D3)       */
/* #define R_V3    0x6L     / * register variable 3               (D4)       */
/* #define R_V4    0x7L     / * register variable 4               (D5)       */
/* #define R_V5    0x8L     / * register variable 5               (D6)       */
/* #define R_V6    0x9L     / * register variable 6               (D7)       */
/* #define R_V7    0xaL     / * register variable 7               (A2)       */
/* #define R_V8    0xbL     / * register variable 8               (A3)       */
/* #define R_V9    0xcL     / * register variable 10              (A5)       */
/* #define R_NONO  0xdL     / * DO NOT USE                        (A4)       */
#define R_FP    0xeL     /* Frame pointer                     (A6)       */
#define R_SP    0xfL     /* main stack pointer                (A7)       */
#define R_F7    (R_F0+7) /* FP reg 7 */

#define Mregnum_(x) ( \
    (unsigned32)(x)<16 ? "\0\0\1\1\2\3\4\5\6\7\2\3\5\4\6\7"[x] : -1)
/* The following macro roughly does Mregnum on a bit map.               */
#define make_m68_mask(m) (((m) & 0x03f0) >> 2 | ((m) & 0xc01) |  \
			  ((m) & 0x2000) >>1 | ((m) & 0x1000) << 1 | \
                          ((m) & 2) << 7 | ((m) & 4) >> 1 | ((m) & 8) << 6)

#define datareg_(x) ((0x3f5 >> (x)) & 1)

# else
/* This enables the OCS calling conventions used by UNIX:               */
/* regs D0,D1,A0,A1 are scratch, regs D2-D7, A2-A5 are for register     */
/* variables.                                                           */ 
/* #define R_A1    0x0L     / * main result register  (D0)           */
#define R_AS2   0x1L     /* regalloc'd scratch A-reg          (A0)       */
#define R_DS    0x2L     /* non-regalloc'd R_IP and scratch   (D1)       */
#define R_AS    0x3L     /* Scratch A-reg                     (A1)       */
/* #define R_V1    0x4L     / * register variable 1               (D2)       */
#define R_DS2    R_V1    /* Also used as 2nd D-reg temp (saved/restored) */
/* #define R_V2    0x5L     / * register variable 2               (D3)       */
/* #define R_V3    0x6L     / * register variable 3               (D4)       */
/* #define R_V4    0x7L     / * register variable 4               (D5)       */
/* #define R_V5    0x8L     / * register variable 5               (D6)       */
/* #define R_V6    0x9L     / * register variable 6               (D7)       */
/* #define R_V7    0xaL     / * register variable 7               (A2)       */
/* #define R_V8    0xbL     / * register variable 8               (A3)       */
/* #define R_V9    0xcL     / * register variable 9               (A4)       */
/* #define R_V10   0xdL     / * register variable 10              (A5)       */
#define R_FP    0xeL     /* Frame pointer                     (A6)       */
#define R_SP    0xfL     /* main stack pointer                (A7)       */
#define R_F7    (R_F0+7) /* FP reg 7 */

#define Mregnum_(x) ( \
    (unsigned32)(x)<16 ? "\0\0\1\1\2\3\4\5\6\7\2\3\4\5\6\7"[x] : -1)
/* The following macro roughly does Mregnum on a bit map.               */
#define make_m68_mask(m) (((m) & 0x03f0) >> 2 | ((m) & 0x3c01) |  \
                          ((m) & 2) << 7 | ((m) & 4) >> 1 | ((m) & 8) << 6)

#define datareg_(x) ((0x3f5 >> (x)) & 1)

# endif
#else /* ARGS_ON_STACK */

/* #define R_A1    0x0L     / * arg 1 & main result register  (D0)           */
#define R_A2    0x1L     /* arg 2                             (D1)       */
/* #define R_A3    0x2L     / * arg 3                             (D2)       */
/* #define R_A4    0x3L     / * arg 4                             (D3)       */
/* #define R_V1    0x4L     / * register variable 1           (D4)           */
/* #define R_V2    0x5L     / * register variable 2               (D5)       */
/* #define R_V3    0x6L     / * register variable 3               (D6)       */
/*                                                         (D7 scratch)  */
/*                                                         (A0 scratch)  */
#define R_V4    0x7L     /* register variable 4               (A1)       */
/* #define R_V5    0x8L     / * register variable 5               (A2)       */
/* #define R_V6    0x9L     / * register variable 6               (A3)       */
/* #define R_V7    0xaL     / * register variable 7               (A4)       */
#define R_V8    0xbL     /* register variable 8               (A5)       */
#define R_FP    0xcL     /* Frame pointer                     (A6)       */
#define R_SP    0xdL     /* main stack pointer                (A7)       */
#define R_AS    0xeL     /* Reference value for A0 */
#define R_DS    0xfL     /*    "        "    "  D7 */
#define R_F7    (R_F0+7) /* FP reg 7 */

#ifdef TARGET_IS_HELIOS
#define R_DP R_V8   /* This is not available to the colourer under HELIOS */
#define R_ADDR1 R_DS /* Used during initialisation of static data */
#endif
#define R_AS2 R_V4      /* Also used as 2nd A-reg temp (saved/restored) */
#define R_DS2 R_A2      /* Also used as 2nd D-reg temp (saved/restored) */


/* A called function can disturb r_a1 to r_a4, but must                  */
/* preserve r_v1 to r_v8, r_fp and r_sp.                                 */

#define Mregnum_(x) ( (x)<R_AS2  ? (x) : \
                  ( (x)<=R_SP ? ((x)-6) :  \
                  ( ((x) == R_DS) ? 7 : \
                  ( ((x) == R_AS) ? 0 : -1 ))))
/* The following macro roughly does Mregnum on a bit map.               */
#define make_m68_mask(m) (((m) & 0x007f) | (((m) & 0x0f80) << 2))

#define datareg_(x) ((x) < R_AS2 || (x) == R_DS)

#endif /* ARGS_ON_STACK */

#define addrreg_(x)  (!datareg_(x))

/* Register values expressed in terms of the mask bit needed in MOVEM  */
#define LDregbit(x) (( 1L << (addrreg(x)? 8l: 0l)+Mregnum(x) ))

#define M_LDFP    LDregbit(R_FP)
#define M_LDAS    LDregbit(R_AS)
#define M_LDDS    LDregbit(R_DS)
                   
#define M_CPUREGS  (regbit(R_A1 + NINTREGS)    - regbit(R_A1))       /* duh? */
#define M_ARGREGS  (regbit(R_A1 + NARGREGS)    - regbit(R_A1))
#define M_VARREGS  (regbit(R_V1 + NVARREGS)    - regbit(R_V1))
#define M_FARGREGS (regbit(R_F0 + NFLTARGREGS) - regbit(R_F0))
#define M_FVARREGS \
        (regbit(R_F0+NFLTVARREGS+NFLTARGREGS)-regbit(R_F0+NFLTARGREGS))

/* Effective address modes */

#define EA_Word      0x100
#define EA_Long      0x200
#ifdef TARGET_IS_68020
#define EA_Single    0x300
#define EA_Double    0x400
#endif

#define EA_Dn        0x00
#define EA_An        0x08
#define EA_AnInd     0x10
#define EA_AnPost    0x18
#define EA_AnPre     0x20
#define EA_AnDisp    0x28
#define EA_AnIndx    0x30
#define EA_AbsW      0x38
#define EA_AbsL      0x39
#define EA_PCDisp    0x3a
#define EA_PCInd     0x3b
#define EA_Imm       0x3c

#define EA_ImmW      EA_Imm | EA_Word /* For information to eafield */
#define EA_ImmL      EA_Imm | EA_Long /* For information to eafield */
#ifdef TARGET_IS_68020
#define EA_ImmS      EA_Imm | EA_Single /* For information to eafield */
#define EA_ImmD      EA_Imm | EA_Double /* For information to eafield */
#endif
/* Length values */

/* CMPI, ANDI, SUBI, ADDI, EORI, CMPI, NEGX, CLR, NEG,
   NOT, TST, ADDX, SUBX, CMPM, Shifts */

#define BYTE0 0
#define WORD0 1
#define LONG0 2

/* MOVEM */
#define WORD1 0
#define LONG1 1

/* MOVE */
#define BYTE2 1
#define WORD2 3
#define LONG2 2

#ifdef TARGET_IS_68020
#  define I_FP  0x80000000L
#  define I_CPU 0x40000000L
#else
#  define I_FP  0x00000000L
#  define I_CPU 0x00000000L
#endif

#define I_MASK (I_FP|I_CPU)

#endif

/* end of m68k/ops.h */
