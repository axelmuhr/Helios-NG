/*
 * File:	hdr_C40.h
 * Author:	P.A.Beskeen
 * Date:	Sept '91
 *
 * Description: TMS320C40 specific header file for generic Helios assembler
 *
 * RcsId: $Id: hdr_C40.h,v 1.5 1993/06/22 16:58:53 paul Exp $
 *
 * (C) Copyright 1991 Perihelion Software Ltd.
 * 
 * $RcsLog$
 *
 */

#ifndef __hdr_C40_h
#define __hdr_C40_h


#ifndef __C40TARGET
# error Target processor architecture is not set correctly
#endif

#include "binary.h"


/* ********************************************************************	*/
/* *** 'C40 Processor Specific Structures *****************************	*/


/* ********************************************************************	*/
/* Mnemonic Structure							*/
/*
 * The Mnemonic structure is attached to all Symbol structures in the hash
 * table that correspond to cpu specific mneumonic keywords
 * (type = HT_MNEMONIC)
 *
 * The decoding of Condition codes is done the C40 lexer, not the parser.
 * This is because they are entered as part of the mnemonic keyword and it is
 * therfore easier to decode them at this point. The condcode item in the
 * Mnemonic structure is used to hold the 'C40 condition field value
 * for any conditional instruction's token.
 *
 * The condcode field is also used to hold the byte/half-word selectors/shifts
 * for the: LBb, LUBb, LHw, LHUw, LWLct, LWRct, MBct and MHct instructions.
 *
 * The condcode field can also be used to store the register selection field
 * for all recognised 'C40 registers: R0-11, AR0-7, DP, IR0-1, BK, SP, ST,
 * DIE, IIE, IIF, RS, RE, RC, IVTP and TVTP.
 *
 */

typedef struct Mnemonic {
	char		*name;		/* mneumonics text name		*/
	int		token;		/* parser token number 		*/
	unsigned	diadic;		/* diadic instruction template	*/
	unsigned	triadic;	/* triadic instruction template	*/
	unsigned	par_st;		/* parallel store template	*/
} Mnemonic;


/* ********************************************************************	*/
/* Instruction Structure						*/
/*									*/
/* Instruction is a Target CPU dependent structure that defines the	*/
/* machine opcode to output. It may also contain expressions that need	*/
/* to be evaluated and combined with the instruction before it is output*/


typedef struct Instruction {
	int			opcode;		/* The opcode to output	    */
	struct Expression	*optexpr;	/* Optionally insert	    */
						/* expr into the opcode	    */
	int			combine;	/* How to combine the       */
						/* expr with the opcode	    */
	struct Expression	*optexpr2;	/* Second src operand	    */
	int			combine2;	/* How to combine 2nd src   */
} Instruction;


/* ********************************************************************	*/
/* Combine Commands							*/


/* *** Diadic Combine Commands ****************************************	*/

/* Most these commands will only use combine, NOT combine2 */

/* Register addressing doesn't need to be checked */
#define COM_DIA_REG		0

/* Check immediate <= 16 bits and insert int bits 0-15 */
#define COM_DIA_IMM		1

/* Check immediate <= 16 bits and insert int bits 0-15 */
#define COM_DIA_IMM_UNSIGNED	2

/* Shift immediate >> 16 and insert int bits 0-15 */
#define COM_DIA_IMM_LDP		3

/* Check disp <= 8 bits and insert into bits 0-8 */
#define COM_DIA_IND		4

/* Check direct offset <= 16 bits and insert into bits 0-15 */
#define COM_DIA_DIR		5

/* Check immediate is <= 5 bits and insert into bits 16-20 */
#define COM_STIK_IMM		6


/* *** Triadic Combine Commands ***************************************	*/

/* Notes arg position by: _A1 = bits 0-7, _A2 = bits 8-15 */

/* Triadic type 1 indirect addressing, args 1 and 2 */
/* Check displacement is only 1 or 0. */
/* If displacement is 0, then convert mod to 0b11000 (*+ARn(0) -> *ARn) */
#define COM_TRI_IND1_A1		7	/* bits 0-7 */
#define COM_TRI_IND1_A2		8	/* bits 8-15 */

/* Triadic type 2 indirect addressing, args 1 and 2 */
/* Check displacement is <= 5 bits and insert disp. into bits 11-15 or 3-7 */
#define COM_TRI_IND2_A1		9	/* bits 3-7 */
#define COM_TRI_IND2_A2		10	/* bits 11-15 */

/* Triadic type 2 immediate addressing, arg 1 only */
/* Check immediate <= eight bits and insert into bits 0-7 of instruction */
#define COM_TRI_IMM_A1		11
#define COM_TRI_IMM_A1_UNSIGNED	12


/* *** Parallel Store Combine Commands ********************************	*/

/* Check displacement is only 1 or 0 */
/* If displacement is 0, then convert mod to 0b11000 (*+ARn(0) -> *ARn) */
		/* 1st src operand: mod = bits 3-7	*/
#define COM_PST_IND_S1		13
		/* Store dest operand: mod = bits 11-15	*/
#define COM_PST_IND_D2		14


/* *** Parallel Multiply Add/Sub Combine Commands *********************	*/

/* Check displacement is only 1 or 0 */
/* If displacement is 0, then convert mod to 0b11000 (*+ARn(0) -> *ARn) */
#define COM_PMPY_IND1		15	/* mod bits 11-15 */
#define COM_PMPY_IND2		16	/* mod bits 3-7 */


/* *** Branch Combine Commands ****************************************	*/

/* If operation is a delayed branch then subtract 3 from the offset */
/* before inserting it into the instruction, otherwise subtract 1 */
/* These combine commands will only ever occur in the first ->combine */

/* Check PC relative offset < 16 bits and insert into bits 0-15 */
/* Used in conditional branch instructions */
/* Bit 21 specifies if it is a delayed branch */
#define COM_CBR_PCREL	17	/* default for pcrel mode */

/* Check PC relative offset < 24 bits and insert into bits 0-23 */
/* Used in UNconditional branch instructions and RPTB(D) */
/* Bit 24 specifies if it is a delayed branch */
#define COM_BR_PCREL	18

/* Check immediate value is <= 9 bits and insert into bits 0-8 */
#define COM_TRAP	19


/* ********************************************************************	*/
/* Register values							*/

/* Standard register file regs: */
#define	R_R0		0x00
#define	R_R1		0x01
#define	R_R2		0x02
#define	R_R3		0x03
#define	R_R4		0x04
#define	R_R5		0x05
#define	R_R6		0x06
#define	R_R7		0x07
#define	R_R8		0x1C
#define	R_R9		0x1D
#define	R_R10		0x1E
#define	R_R11		0x1F
#define	R_AR0		0x08
#define	R_AR1		0x09
#define	R_AR2		0x0A
#define	R_AR3		0x0B
#define	R_AR4		0x0C
#define	R_AR5		0x0D
#define	R_AR6		0x0E
#define	R_AR7		0x0F
#define	R_DP		0x10
#define	R_IR0		0x11
#define	R_IR1		0x12
#define	R_BK		0x13
#define	R_SP		0x14
#define	R_ST		0x15
#define	R_DIE		0x16
#define	R_IIE		0x17
#define	R_IIF		0x18
#define	R_RS		0x19
#define	R_RE		0x1A
#define	R_RC		0x1B

/* Expansion register file regs: */
#define R_IVTP		0x00
#define R_TVTP		0x01


/* ********************************************************************	*/
/* Macros to check that the correct registers are being used.		*/
/* They raise a parser error if this is not the case.			*/

#define CHK_Dreg(reg)		if (reg > 11) { \
					Error("Must be register R0-R11");\
				}

#define CHK_Dreg0_7(reg)	if (reg > 7) { \
					Error("Must be register R0-R7"); \
				}

#define CHK_Dreg0_1(reg)	if (reg > 1) { \
					Error("Must be register R0 or R1"); \
				}

#define CHK_Dreg2_3(reg)	if (reg < 2 || reg > 3) { \
					Error("Must be Register R2 or R3"); \
				}



/* ********************************************************************	*/
/* Condition codes [usually bits 16 - 21]				*/

/*
 * For all possible conditions, define the binary code to entered into the
 * cond field of conditional instructions.
 */

#define CC(cond)	((cond) << 16)

#define C_U		CC(B_00000)	/* unconditional		*/
#define C_LO		CC(B_00001)	/* less than (unsigned)		*/
#define C_LS		CC(B_00010)	/* less than or equal to (unsigned)*/
#define C_HI		CC(B_00011)	/* greater than (unsigned)	*/
#define C_HS		CC(B_00100)	/* greater than or equal to (unsigned)*/
#define C_EQ		CC(B_00101)	/* equal to			*/
#define C_NE		CC(B_00110)	/* not equal to			*/
#define C_LT		CC(B_00111)	/* less than (signed)		*/
#define C_LE		CC(B_01000)	/* less than or equal to (signed)*/
#define C_GT		CC(B_01001)	/* greater than (signed)	*/
#define C_GE		CC(B_01010)	/* greater than or equal to (signed)*/
/* #define C_		CC(B_01011)	 */ /* UNSPECIFIED		*/
#define C_NV		CC(B_01100)	/* no overflow 			*/
#define C_V		CC(B_01101)	/*    overflow 			*/
#define C_NUF		CC(B_01110)	/* no underflow			*/
#define C_UF		CC(B_01111)	/*    underflow			*/
#define C_NLV		CC(B_10000)	/* no latched overflow		*/
#define C_LV		CC(B_10001)	/*    latched overflow		*/
#define C_NLUF		CC(B_10010)	/* no latched fp underflow	*/
#define C_LUF		CC(B_10011)	/*    latched fp underflow	*/
#define C_ZUF		CC(B_10100)	/* zero or fp underflow		*/

/* Unspecified condition codes */
#if 0
#define C_		CC(B_10101)
#define C_		CC(B_10110)
#define C_		CC(B_10111)
#define C_		CC(B_11000)
#define C_		CC(B_11001)
#define C_		CC(B_11010)
#define C_		CC(B_11011)
#define C_		CC(B_11100)
#define C_		CC(B_11101)
#define C_		CC(B_11110)
#define C_		CC(B_11111)
#endif

/* aliases */

#define C_Z		C_EQ		/* zero				*/
#define C_NZ		C_NE		/* not zero			*/
#define C_P		C_GT		/* positive			*/
#define C_N		C_LT		/* negative			*/
#define C_NN		C_GE		/* not negative			*/
#define C_NC		C_HS		/* not carry			*/
#define C_C		C_LO		/* carry			*/


/* ********************************************************************	*/
/* Individual Instruction Opcode Definitions				*/
/* ********************************************************************	*/

/* ********************************************************************	*/
/* Diadic sequential op-codes [bits 23 - 28] (bits 29 - 31 => 000)	*/

#define DIA(op)		(/*(B_000 << 29) |*/ ((op) << 23))

#define OP_ABSF		DIA(B_000000)	/* absolute value of fp		*/
#define OP_ABSI		DIA(B_000001)	/* absolute value of int	*/
#define OP_ADDC		DIA(B_000010)	/* add int with carry		*/
#define OP_ADDF		DIA(B_000011)	/* add foating-point		*/
#define OP_ADDI		DIA(B_000100)	/* add int, no carry		*/
#define OP_AND		DIA(B_000101)	/* bitwise AND			*/
#define OP_ANDN		DIA(B_000110)	/* bitwise (NOT src) AND dst	*/
#define OP_ASH		DIA(B_000111)	/* arithmetic shift		*/
#define OP_CMPF		DIA(B_001000)	/* compare fp			*/
#define OP_CMPI		DIA(B_001001)	/* compare int			*/
#define OP_FIX		DIA(B_001010)	/* fp to int conversion		*/
#define OP_FLOAT	DIA(B_001011)	/* int to fp conversion		*/
#define OP_IDLE		DIA(B_001100)	/* idle until interrupt		*/
#define OP_LDE		DIA(B_001101)	/* load fp exponent		*/
#define OP_LDF		DIA(B_001110)	/* load fp			*/
#define OP_LDFI		DIA(B_001111)	/* load fp, interlocked		*/
#define OP_LDI		DIA(B_010000)	/* load int			*/
#define OP_LDII		DIA(B_010001)	/* load int, interlocked	*/
#define OP_LDM		DIA(B_010010)	/* load fp mantissa		*/
#define OP_LSH		DIA(B_010011)	/* logical shift		*/
#define OP_MPYF		DIA(B_010100)	/* multiply, fp			*/
#define OP_MPYI		DIA(B_010101)	/* multiply, int		*/
#define OP_NEGB		DIA(B_010110)	/* negate int with borrow	*/
#define OP_NEGF		DIA(B_010111)	/* negate fp			*/
#define OP_NEGI		DIA(B_011000)	/* negate integer, no borrow	*/
#define OP_NOP		DIA(B_011001)	/* nop 				*/
#define OP_NORM		DIA(B_011010)	/* normalise			*/
#define OP_NOT		DIA(B_011011)	/* bitwise NOT			*/
#define OP_POP		(DIA(B_011100) | G_DIR) /* pull int off stack	*/
#define OP_POPF		(DIA(B_011101) | G_DIR) /* pull fp off stack	*/
#define OP_PUSH		(DIA(B_011110) | G_DIR) /* push int onto stack	*/
#define OP_PUSHF	(DIA(B_011111) | G_DIR) /* push fp onto stack	*/
#define OP_OR		DIA(B_100000)	/* bitwise OR			*/
/* #define OP_		DIA(B_100001)	*//* UNSPECIFIED		*/
#define OP_RND		DIA(B_100010)	/* round, fp			*/
					/* rotate 1 bit left		*/
#define OP_ROL		(DIA(B_100011) | G_IMM | 1)
					/* rot left, thru carry		*/
#define OP_ROLC		(DIA(B_100100) | G_IMM | 1)
					/* rotate 1 bit right		*/
#define OP_ROR		(DIA(B_100101) | G_IMM | 0xffff)
					/* rot right thru carry		*/
#define OP_RORC		(DIA(B_100110) | G_IMM | 0xffff)
					/* repeat single instruction	*/
#define OP_RPTS		(DIA(B_100111) | DST_REG(R_RC))
#define OP_STF		DIA(B_101000)	/* store fp			*/
#define OP_STFI		DIA(B_101001)	/* store fp, interlocked	*/
#define OP_STI		DIA(B_101010)	/* store int			*/
#define OP_STII		DIA(B_101011)	/* store int, interlocked	*/
#define OP_SIGI		DIA(B_101100)	/* signal, interlocked		*/
#define OP_SUBB		DIA(B_101101)	/* sub int with borrow		*/
#define OP_SUBC		DIA(B_101110)	/* sub int, maybe		*/
#define OP_SUBF		DIA(B_101111)	/* sub fp			*/
#define OP_SUBI		DIA(B_110000)	/* sub int, no borrow		*/
#define OP_SUBRB	DIA(B_110001)	/* sub int with borrow, reversed*/
#define OP_SUBRF	DIA(B_110010)	/* sub fp, reversed		*/
#define OP_SUBRI	DIA(B_110011)	/* sub int, no borrow, reverse	*/
#define OP_TSTB		DIA(B_110100)	/* test bit fields		*/
#define OP_XOR		DIA(B_110101)	/* bitwise exclusive or		*/
#define OP_IACK		DIA(B_110110)	/* interrupt acknowledge	*/
#define OP_TOIEEE	DIA(B_110111)	/* convert internal fp to IEEE	*/
#define OP_FRIEEE	DIA(B_111000)	/* convert IEEE to internal fp	*/
#define OP_RSQRF	DIA(B_111001)	/* reciprocal square root fp	*/
#define OP_RCPF		DIA(B_111010)	/* reciprocal fp		*/
#define OP_MPYSHI	DIA(B_111011)	/* mult   signed int, high word	*/
#define OP_MPYUHI	DIA(B_111100)	/* mult unsigned int, high word	*/
/* @@@ NICKC got following two opcodes wrong */
#define OP_LDA		DIA(B_111101)	/* load address register	*/
					/* load sixteen bit unsigned imm.*/
#define OP_LDPK		DIA(B_111110) | (B_1110000 << 16)
#define OP_LDHI		DIA(B_111111)	/* load top 16 bits (immediate)	*/


/* ********************************************************************	*/
/* Triadic sequential op-codes [bits 23 - 27] (bits 29 - 31 => 001,	*/
/* bit 28 => 0 (type 1), or => 1 (type 2))				*/

#define TRI(op)		(((unsigned)B_10 << 28) | ((op) << 23))

#define OP_ADDC3	TRI(B_00000)	/* add integer with carry	*/
#define OP_ADDF3	TRI(B_00001)	/* add floating point		*/
#define OP_ADDI3	TRI(B_00010)	/* add integer, no carry	*/
#define OP_AND3		TRI(B_00011)	/* bitwise AND			*/
#define OP_ANDN3	TRI(B_00100)	/* (NOT src) AND dst		*/
#define OP_ASH3		TRI(B_00101)	/* arithmetic shift		*/
#define OP_CMPF3	TRI(B_00110)	/* floating point compare	*/
#define OP_CMPI3	TRI(B_00111)	/* integer compare		*/
#define OP_LSH3		TRI(B_01000)	/* logical shift		*/
#define OP_MPYF3	TRI(B_01001)	/* floating point multiply	*/
#define OP_MPYI3	TRI(B_01010)	/* integer multiply		*/
#define OP_OR3		TRI(B_01011)	/* bitwise OR			*/
#define OP_SUBB3	TRI(B_01100)	/* integer sub with borrow	*/
#define OP_SUBF3	TRI(B_01101)	/* floating point subtact	*/
#define OP_SUBI3	TRI(B_01110)	/* integer sub, no borrow	*/
#define OP_TSTB3	TRI(B_01111)	/* test bit fields		*/
#define OP_XOR3		TRI(B_10000)	/* bitwise XOR			*/
#define OP_MPYSHI3	TRI(B_10001)	/* signed integer multiply,	*/
					/* high word of result		*/
#define OP_MPYUHI3	TRI(B_10010)	/* unsigned integer multiply,	*/
					/* high word of result		*/

#if 0	/* Unspecified triadic opcodes */
#define OP_		TRI(B_10011)	/* UNSPECIFIED */
#define OP_		TRI(B_10100)	/* UNSPECIFIED */
#define OP_		TRI(B_10101)	/* UNSPECIFIED */
#define OP_		TRI(B_10110)	/* UNSPECIFIED */
#define OP_		TRI(B_10111)	/* UNSPECIFIED */
#define OP_		TRI(B_11000)	/* UNSPECIFIED */
#define OP_		TRI(B_11001)	/* UNSPECIFIED */
#define OP_		TRI(B_11010)	/* UNSPECIFIED */
#define OP_		TRI(B_11011)	/* UNSPECIFIED */
#define OP_		TRI(B_11100)	/* UNSPECIFIED */
#define OP_		TRI(B_11101)	/* UNSPECIFIED */
#define OP_		TRI(B_11110)	/* UNSPECIFIED */
#define OP_		TRI(B_11111)	/* UNSPECIFIED */
#endif


/* ********************************************************************	*/
/* Parallel store op codes [bits 25 - 29] (bits 30 - 31 => 11)		*/
/* (bits 19 - 21 => 000) 						*/

#define	PARST(op)	(((unsigned)B_11 << 30) | ((op) << 25))

#define OP_STF_STF	PARST(B_00000)	/* store floating point		*/
					/* AND store floating point	*/
#define OP_STI_STI	PARST(B_00001)	/* store integer		*/
					/* AND store integer		*/
#define OP_LDF_LDF	PARST(B_00010)	/* load floating point		*/
					/* AND load  floating point	*/
#define OP_LDI_LDI	PARST(B_00011)	/* load integer			*/
					/* AND load  integer       	*/
#define OP_ABSF_STF	PARST(B_00100)	/* absolute floating point	*/
					/* AND store floating point	*/
#define OP_ABSI_STI	PARST(B_00101)	/* absolute integer		*/
					/* AND store integer       	*/
#define OP_ADDF3_STF	PARST(B_00110)	/* triadic add floating point	*/
					/* AND store floating point	*/
#define OP_ADDI3_STI	PARST(B_00111)	/* triadic add integer		*/
					/* AND store integer       	*/
#define OP_AND3_STI	PARST(B_01000)	/* triadic bitwise and		*/
					/* AND store integer       	*/
#define OP_ASH3_STI	PARST(B_01001)	/* triadic arithmetic shift	*/
					/* AND store integer       	*/
#define OP_FIX_STI	PARST(B_01010)	/* convert fp to integer	*/
					/* AND store integer       	*/
#define OP_FLOAT_STF	PARST(B_01011)	/* convert integer to fp	*/
					/* AND store floating point	*/
#define OP_LDF_STF	PARST(B_01100)	/* load floating point 		*/
					/* AND store floating point	*/
#define OP_LDI_STI	PARST(B_01101)	/* load integer			*/
					/* AND store integer       	*/
#define OP_LSH3_STI	PARST(B_01110)	/* triadic logical shift	*/
					/* AND store integer       	*/
#define OP_MPYF3_STF	PARST(B_01111)	/* triadic floating point mult	*/
					/* AND store floating point	*/
#define OP_MPYI3_STI	PARST(B_10000)	/* triadic integer multiply	*/
					/* AND store integer       	*/
#define OP_NEGF_STF	PARST(B_10001)	/* negate floating point	*/
					/* AND store floating point	*/
#define OP_NEGI_STI	PARST(B_10010)	/* negate integer		*/
					/* AND store integer       	*/
#define OP_NOT_STI	PARST(B_10011)	/* bitwise NOT			*/
					/* AND store integer       	*/
#define OP_OR3_STI	PARST(B_10100)	/* traidic bitwise OR		*/
					/* AND store integer       	*/
#define OP_SUBF3_STF	PARST(B_10101)	/* triadic floating point sub	*/
					/* AND store floating point	*/
#define OP_SUBI3_STI	PARST(B_10110)	/* triadic integer sub		*/
					/* AND store integer       	*/
#define OP_XOR3_STI	PARST(B_10111)	/* triadic bitwise XOR		*/
					/* AND store integer       	*/
#define OP_TOIEEE_STF	PARST(B_11000)	/* convert to IEEE format	*/
					/* AND store floating point	*/
#define OP_FRIEEE_STF	PARST(B_11001)	/* convert from IEEE format	*/
					/* AND store floating point	*/

#if 0	/* Unspecified parallel store opcodes */
#define OP_		PARST(B_11010)	/* UNSPECIFIED */
#define OP_		PARST(B_11011)	/* UNSPECIFIED */
#define OP_		PARST(B_11100)	/* UNSPECIFIED */
#define OP_		PARST(B_11101)	/* UNSPECIFIED */
#define OP_		PARST(B_11110)	/* UNSPECIFIED */
#define OP_		PARST(B_11111)	/* UNSPECIFIED */
#endif


/* ********************************************************************	*/
/* Parallel Multiply op codes [bits 26 - 29] (bits 30 - 31 => 10)	*/

#define PARMPY(op)	(((unsigned)B_10 << 30) | ((op) << 26))

#define OP_MPYF3_ADDF3	PARMPY(B_0000)	/* triadic floating point multiply */
					/* AND triadic floating point add  */
#define OP_MPYF3_SUBF3	PARMPY(B_0001)	/* triadic floating point multiply */
					/* AND triadic floating point sub  */
#define OP_MPYI3_ADDI3	PARMPY(B_0010)	/* triadic integer multiply	   */
					/* AND triadic integer addition    */
#define OP_MPYI3_SUBI3	PARMPY(B_0011)	/* triadic integer multiply	   */
					/* AND triadic integer sub	   */

#if 0	/* Unspecified parallel multiply opcodes */
#define OP_MPYI3_	PARMPY(B_0100)	/* UNSPECIFIED */
#define OP_MPYI3_	PARMPY(B_0101)	/* UNSPECIFIED */
#define OP_MPYI3_	PARMPY(B_0110)	/* UNSPECIFIED */
#define OP_MPYI3_	PARMPY(B_0111)	/* UNSPECIFIED */
#define OP_MPYI3_	PARMPY(B_1000)	/* UNSPECIFIED */
#define OP_MPYI3_	PARMPY(B_1001)	/* UNSPECIFIED */
#define OP_MPYI3_	PARMPY(B_1010)	/* UNSPECIFIED */
#define OP_MPYI3_	PARMPY(B_1011)	/* UNSPECIFIED */
#define OP_MPYI3_	PARMPY(B_1100)	/* UNSPECIFIED */
#define OP_MPYI3_	PARMPY(B_1101)	/* UNSPECIFIED */
#define OP_MPYI3_	PARMPY(B_1110)	/* UNSPECIFIED */
#define OP_MPYI3_	PARMPY(B_1111)	/* UNSPECIFIED */
#endif


/* ********************************************************************	*/
/* Part word data transfers [bits 25 - 27] (bits 28 - 31 => 1011)	*/
/* Note half word instructions only use bit 23, bit 24 being used to	*/
/* select a particular instruction. Byte instructions use bits 24 and	*/
/* 23 to select a particular byte.					*/

#define PWB(op)		(((unsigned)B_1011 << 28) | ((op) << 25))
#define PWHW(op)	(((unsigned)B_1011 << 28) | ((op) << 24))

/* Byte loads */
#define OP_LB		PWB(B_000)	/* load   (signed) byte */
#define OP_LBU		PWB(B_001)	/* load (unsigned) byte */
#define OP_LWL		PWB(B_010)	/* load word, left shifted */
#define OP_LWR		PWB(B_011)	/* load word, right shifted */
#define OP_MB		PWB(B_100)	/* merge byte, left shifted */

/* Half word loads */
#define OP_MH		PWHW(B_1100)	/* merge half word, left shifted */
#define OP_LH		PWHW(B_1010)	/* load half word sign extended */
#define OP_LHU		PWHW(B_1011)	/* load half word, unsigned */

#if 0	/* Unspecified part word loads */
#define OP_ 		B_PWHW(1101)	/* UNSPECIFIED */
#define OP_		B_PWHW(1110)	/* UNSPECIFIED */
#define OP_		B_PWHW(1111)	/* UNSPECIFIED */
#endif


/* ********************************************************************	*/
/* Conditional Load Instructions 					*/

#define OP_LDIc		((unsigned)B_0101 << 28)    /* conditionaly load int */
#define OP_LDFc		((unsigned)B_0100 << 28)    /* conditionaly load fp */


/* ********************************************************************	*/
/* Flow control and miscellaneous opcodes (bits 29 - 31 => 011)		*/

/* Unconditional Branches */
#define	SH_BR(op)	((op) << 24)

#define OP_BR		SH_BR(B_01100000)	/* branch unconditionaly */
#define OP_BRD		SH_BR(B_01100001)	/* uncond. branch, delayed */
#define OP_CALL		SH_BR(B_01100010)	/* uncond. call to subroutine */
#define OP_LAJ		SH_BR(B_01100011)	/* uncond. link to subroutine */


/* Conditional Branches */
#define SH_CBR(op)	((op) << 21)	/* shift opcode into position */

#define BR_PCREL	(1 << 25)	/* set pc relative mode */
#define BR_REG		0		/* defaults to register mode */

#define OP_Bcond	SH_CBR(B_01101000000)	/* cond. branch */
#define OP_BcondAF	SH_CBR(B_01101000101)	/* annul exec. if FALSE */
#define OP_BcondAT	SH_CBR(B_01101000011)	/* annul exec. if TRUE  */
#define OP_BcondD	SH_CBR(B_01101000001)	/* cond. branch delayed */

#define OP_CALLcond	SH_CBR(B_01110000000)	/* cond. call to subroutine */

#define OP_LAJcond	SH_CBR(B_01110000001)	/* cond. link and jump */

#define OP_RETIcond	SH_CBR(B_01111000000)	/* cond. return */
#define OP_RETIcondD	SH_CBR(B_01111000001)	/* cond. return */
#define OP_RETScond	SH_CBR(B_01111000100)	/* cond. return */

#define OP_TRAPcond	SH_CBR(B_01110100000)	/* cond. trap */
#define OP_LATcond	SH_CBR(B_01110100100)	/* cond. link and trap */

#define	DB_AREG(areg)	(((areg) - 8) << 22)	/* how to insert ARn reg into DB */
#define OP_DBcond	SH_CBR(B_01101100000)	/* cond. dec and br */
#define OP_DBcondD	SH_CBR(B_01101100001)	/* cond. dec and br, delayed */


/* Load/Store expansion regs */
#define OP_LDEP		((unsigned)B_01110110000 << 21)	/* ld reg from expansion reg */
#define OP_LDPE		((unsigned)B_01110110100 << 21)	/* ld expansion reg from reg */


/* Repeat Block */
/* Odd in that different addressing modes have completely different opcodes */
/* - so RPTBr is held in triadic position in mnemonic structure */
#define OP_RPTBr	((unsigned)B_011110010 << 23)	/* rpt block (reg)   */
#define OP_RPTBp	((unsigned)B_01100100  << 24)	/* rpt block (PCrel) */
#define OP_RPTBDr	((unsigned)B_011110011 << 23)	/* rpt (reg) delayed */
#define OP_RPTBDp	((unsigned)B_01100101  << 24)	/* rpt (PCrel) delayed */


/* Software Interrupt */
#define OP_SWI		((unsigned)B_0110011 << 25)



/* ********************************************************************	*/
/* *** Addressing Mode Macros *****************************************	*/

/* *** General addressing mode (ld/st/diadic) macros ******************	*/

/* insert into G addr. mode selection field of instruction */
#define G(am)		(am << 21)		/* [bits 21-22] */

/* G field address mode selector values */
#define G_REG		G(B_00)			/* register */
#define G_DIR		G(B_01)			/* direct */
#define G_IND		G(B_10)			/* indirect */
#define G_IMM		G(B_11)			/* immediate */

/* insert destination reg used in ld's, diadic and triadic ops into */
/* instruction template */
#define DST_REG(x)	((x) << 16)		/* [bits 16-20] */

/* insert src operand in ld's and diadic ops into instruction template */
#define SRC_REG(x)	(x)			/* [bits 0-7] */

/* insert src indirect addressing mod field into instruction template */
#define MOD_FIELD(x)	((x) << 8)		/* [bits 8-15] */

/* insert source reg used in store ops into instruction template */
#define ST_SRC_REG(x)	((x) << 16)		/* [bits 16-20] */

/* insert destination into instruction template */
#define ST_DST(x)	(x)			/* [bits 0-15] */



/* *** Triadic Addressing Macros **************************************	*/

/* Addressing mode subtype selectors */
#define TRI_TYPE_1	0			/* default */
#define TRI_TYPE_2	(1 << 28)		/* [bit 28] */

/* insert triadic addressing mode combination selection field */
#define T(x)		((x) << 21)		/* [bits 21-22] */

/* addressing mode combinations used by triadic operations */
#define T1_REGREG	(TRI_TYPE_1 | T(B_00))
#define T1_REGIND	(TRI_TYPE_1 | T(B_01))
#define T1_INDREG	(TRI_TYPE_1 | T(B_10))
#define T1_INDIND	(TRI_TYPE_1 | T(B_11))

#define T2_IMMREG	(TRI_TYPE_2 | T(B_00))
#define T2_INDREG	(TRI_TYPE_2 | T(B_01))
#define T2_IMMIND	(TRI_TYPE_2 | T(B_10))
#define T2_INDIND	(TRI_TYPE_2 | T(B_11))

/* insert argument 1 into triadic instruction template */
#define TRI_A1(x)	(x)		/* 1st arg (G)	[bits 0-7] */

/* insert argument 2 into triadic instruction template */
#define TRI_A2(x)	((x) << 8)	/* 2nd arg (G)	[bits 8-15] */


/* *** Parallel Store Addressing Macros *******************************	*/

/* insert arguments into parallel store instruction template */
#define PST_S1(x)	(x)		/* 1st src arg (ind)	[bits 0-7]   */
#define PST_S2(x)	((x) << 19)	/* 2nd src arg (reg)	[bits 19-21] */
#define PST_SS(x)	((x) << 16)	/* store src arg reg	[bits 16-18] */
#define PST_D1(x)	((x) << 22)	/* operation 1 dest  	[bits 22-24] */
#define PST_D2(x)	((x) << 8)	/* store dest (ind)	[bits 8-15]  */

/* insert arguments into parallel load instruction template */
#define PLD_SRC(x)	((x) << 8)	/* load src arg (ind)	[bits 8-15]  */
#define PLD_DST(x)	((x) << 19)	/* load dest (reg)	[bits 19-21] */

/* insert arguments into parallel st || st instruction template */
#define PSTST_SRC(x)	((x) << 22)	/* st src arg (reg)	[bits 22-24] */
#define PSTST_DST(x)	(x)		/* st dest (ind)	[bits 0-7]   */


/* *** Parallel Multiply Addressing Macros ****************************	*/

/* insert parallel multiply addressing mode combination selection field */
#define P(x)		((x) << 24)		/* [bits 24-25] */

/* addressing mode combinations used by parallel multiply operations	*/
/* I = indirect, R = register, with operand order.			*/
/* The second two operands order has been swapped so that they reflect	*/
/* the ordering of SUB operands. Other operations are commutative, so	*/
/* this is not a problem						*/

#define P_I1I2R2R1		(P(B_00))
#define P_I1R1R2I2		(P(B_01))
#define P_R1R2I2I1		(P(B_10))
#define P_I1R1I2R2		(P(B_11))

/* insert parallel multiply operands ito instruction template */
#define PM_R1(x)	((x) << 19)	/* 1st src reg [bits 19-21] */
#define PM_R2(x)	((x) << 16)	/* 2nd src reg [bits 16-18] */

#define PM_I1(x)	((x) << 8)	/* 1st indirect [bits 8-15] */
#define PM_I2(x)	(x)		/* 2nd indirect [bits 0-7] */

#define PM_D1(x)	((x) << 23)	/* dest reg0-1 [bit 23] */
#define PM_D2(x)	((x) << 22)	/* dest reg2-3 [bit 22] */


/* *** Indirect addressing macros *************************************	*/
/* Used in creating the mod and ARn fields in indirect addressing	*/

/* move indirect addr. mod field to make space for address reg field */
#define MOD(x)		((x) << 3)

/* insert indirect addr. address register field */
/* AR0 = 0 ... AR7 = 7 */
#define IND_AREG(x)	((x) - R_AR0)

/* move indirect addr. displacement TYPE selector field within modifier field */
#define MOD_DISP(x)	((x) << 3)

/* Checks if mod field is compatible with type 2 triadic addressing */
/* "*+ARn" is only mode allowed */
#define ISMODTYPE2(mod)	(((mod) & B_11111000) == B_00000000)



/* Exported variables */

/* holds current instruction being parsed */
extern	Instruction	*CurInstr;


/* Exported C40 specific functions */


/*
 * Conversion between C40 and IEEE format.
 */

extern int32 C40_32ToC40_16(int32);

#ifdef HOST_SUPPORTS_IEEE
	extern	int32 IEEE_32ToC40_32(float);
#else
	extern	int32 IEEE_32ToC40_32(int32);
#endif

extern	int32 IEEE_64ToC40_32(Dble);
extern	int32 IEEE_64ToC40_16(Dble);

#if 0 /* Available, not currently used */
extern	int32 C40_16ToC40_32(int32);

#ifdef HOST_SUPPORTS_IEEE
	extern	float C40_32ToIEEE_32(int32);
#else
	extern	int32 C40_32ToIEEE_32(int32);
#endif

#endif /* Available, not currently used */


#endif /* __hdr_C40_h */
