/*
 * File:	header_ARM.h
 * Subsystem:	Generic (ARM) Assembler
 * Author:	P.A.Beskeen
 * Date:	Sept '91
 *
 * Description: ARM specific header file for generic Helios assembler
 *
 * RcsId: $Id: hdr_ARM.h,v 1.6 1994/03/08 12:50:56 nickc Exp $
 *
 * (C) Copyright 1992 Paul Beskeen
 * 
 * $RcsLog$
 *
 */

#ifndef __header_ARM_h
#define __header_ARM_h


#ifndef __ARMTARGET
# error Target processor architecture is not set correctly
#endif


#include "binary.h"


/* ********************************************************************	*/
/* *** ARM Processor Specific Structures ******************************	*/


/* ********************************************************************	*/
/* Mnemonic Structure							*/
/*									*/
/* The Mnemonic structure is attached to all Symbol structures in the	*/
/* hash table that correspond to CPU specific mneumonic keywords.	*/
/* (type = HT_MNEMONIC)							*/
/*									*/
/* The decoding of Condition codes is done in the ARM lexer, not the	*/
/* parser. This is because they are entered as part of the mnemonic	*/
/* keyword and it is therfore easier to decode them at this point.	*/


typedef struct Mnemonic {
	char		*name;		/* mneumonics text name		*/
	int		token;		/* parser token number		*/
	unsigned	Template;	/* binary instruction template	*/
} Mnemonic;


/* ********************************************************************	*/
/* Instruction Structure						*/
/*									*/
/* Instruction is a Target CPU dependent structure that defines the	*/
/* machine opcode to output. It may also contain expressions that need	*/
/* to be evaluated and combined with the instruction before it is	*/
/* This structure is held in the parse tree.				*/


typedef struct Instruction {
	int			opcode;		/* The opcode to output	    */
	struct Expression	*optexpr;	/* Optionally insert this   */
						/* expr into the opcode     */
	int			combine;	/* How to combine the	    */
						/* optexpr with the opcode  */
	/* optexpr 2 is only used for the CDO, LDC/STC and MRC/MCR instr's. */
	/* and is always used to store the coprocessor number (bits 8-11) */
	struct Expression	*optcoprocnum;	/* Optionally insert this   */
						/* expr into the opcode     */
	/* optexpr 3 is only used for the CDO and MRC/MCR instructions */
	/* and is always used to store the optional aux field (bits 5-7) */
	struct Expression	*optcoprocaux;	/* Optionally insert this   */
						/* expr into the opcode     */
} Instruction;


/* ********************************************************************	*/
/* Combine Commands							*/

/* default - no checking */
#define COM_REG		0

/* check 24 bit pcrel value, left shift by two and subtract 2 for pipeline */
#define COM_PCREL_24	1

/* Check immediate can be encoded into 8 bits with a 4*2 bit rotate and */
/* insert into bits 0-7 + 8-11 */
#define COM_ROTIMM_8	2

/* Check unsigned shift value < 31/32 (converting shift type if required) */
/* and insert into bits 7-11 */
#define COM_SHIFT	3

/* Check immediate <= 8 bits and insert into bits 0-7 */
#define COM_IMM8	4

/* Check if fits into 12 bits unsigned - if positive bit 23 U should be set */
/* insert into bits 0-11 */
#define COM_INDEX_12	5	/* data transfer instr's index addressing */

/* Check if fits into 8 bits unsigned - if positive bit 23 U should be set */
/* insert into bits 0-7 */
#define COM_INDEX_8	6	/* restricted coproc index addressing */

/* Check if fits into 12 bits unsigned and insert into bits 0-11 */
/* If negative, ABS value and reset bit 23 U (sign bit). */
/* If positive bit 23 U should be set. */
/* Sub 8 to adjust for pipeline */
#define COM_PCREL_12	7	/* ARM data transfer expr */

/* Check if fits into 8 bits unsigned and insert into bits 0-7 */
/* If negative, ABS value and reset bit 23 U (sign bit). */
/* If positive bit 23 U should be set. */
/* Sub 8 to adjust for pipeline */
#define COM_PCREL_8	8	/* coprocessor data transfer expr */

/* Check expression will fit in 24 bits and insert into bits 0-23 */
#define COM_SWI		9	/* user defined operation */

/* Check expression will fit in 4 bits and insert into bits 20-23 */
#define COM_CP_OPc	10	/* coprocessor data operations */

/* Check expression will fit in 3 bits and insert into bits 21-23 */
#define COM_CP_OPc2	11	/* coprocessor register transfers */

/* After evaluation the constant should have eight subtracted from it */
/* (to take account of the pipeline). Check this value can be encoded */
/* into 8 bits with a 4*2 bit rotate and insert into bits 0-7 + 8-11 */
#define COM_LEAROTIMM_8	12


/* ********************************************************************	*/
/* Register values							*/

/* Standard register file regs: */
#define	R_R0		0x0
#define	R_R1		0x1
#define	R_R2		0x2
#define	R_R3		0x3
#define	R_R4		0x4
#define	R_R5		0x5
#define	R_R6		0x6
#define	R_R7		0x7
#define	R_R8		0x8
#define	R_R9		0x9
#define	R_R10		0xA
#define	R_R11		0xB
#define	R_R12		0xC
#define	R_R13		0xD
#define	R_R14		0xE
#define	R_R15		0xF

/* coprocessor registers: */
#define	R_CR0		0x0
#define	R_CR1		0x1
#define	R_CR2		0x2
#define	R_CR3		0x3
#define	R_CR4		0x4
#define	R_CR5		0x5
#define	R_CR6		0x6
#define	R_CR7		0x7
#define	R_CR8		0x8
#define	R_CR9		0x9
#define	R_CR10		0xA
#define	R_CR11		0xB
#define	R_CR12		0xC
#define	R_CR13		0xD
#define	R_CR14		0xE
#define	R_CR15		0xF


/* ********************************************************************	*/
/* Condition codes [bits 31 - 28]					*/

/*
 * For all possible conditions, define the binary code to entered into the
 * cond field of the instruction.
 */

#define CC(cond)	((unsigned)(cond) << 28)

#define	C_EQ	CC(B_0000)
#define	C_NE	CC(B_0001)
#define	C_CS	CC(B_0010)
#define	C_CC	CC(B_0011)
#define	C_MI	CC(B_0100)
#define	C_PL	CC(B_0101)
#define	C_VS	CC(B_0110)
#define	C_VC	CC(B_0111)
#define	C_HI	CC(B_1000)
#define	C_LS	CC(B_1001)
#define	C_GE	CC(B_1010)
#define C_LT	CC(B_1011)
#define C_GT	CC(B_1100)
#define C_LE	CC(B_1101)
#define C_AL	CC(B_1110)
#define	C_NV	CC(B_1111)



/* ********************************************************************	*/
/* SHIFT TYPES used in data processing and transfer instructions (bits 5/6) */

#define SHIFT_TYPE(x)	((x) << 5)	/* shift type field */
#define SHIFT_TYPE_MASK	(B_11 << 5)	/* shift type field */
#define SHIFT_IMM(x)	((x) << 7)	/* shift amount field */

#define	OP_LSL		SHIFT_TYPE(B_00)
#define	OP_ASL		SHIFT_TYPE(B_00)	/* synoynm for OP_LSL */
#define	OP_LSR		SHIFT_TYPE(B_01)
#define	OP_ASR		SHIFT_TYPE(B_10)
#define	OP_ROR		SHIFT_TYPE(B_11)
#define	OP_RRX		SHIFT_TYPE(B_11)	/* synoynm for OP_ROR of 0 */


/* ********************************************************************	*/
/* Common Instruction Definitions					*/

#define WRITEBACK	(1 << 21)	/* index addressing write back '!' */
#define WTRANS		(1 << 21)	/* synonym for -TRANS pin in non user */
#define INDEXUP		(1 << 23)	/* up, add offset to base, default dn */
#define PREINDEX	(1 << 24)	/* pre-index, default post */

/* index base used in SWP, single, multiple and coproc data transfer instr's */
#define INDEX_BASEREG(x)	((x) << 16)
#define EXTRACT_BASEREG(opcode)	((opcode & (0xf << 16)) >> 16)


/* ********************************************************************	*/
/* Individual Instruction Opcode Definitions				*/
/* ********************************************************************	*/


/* ********************************************************************	*/
/* Branch op-codes (bits 27 - 25 => 101)				*/

#define OP_B	(B_101 << 25)
#define OP_BL	((B_101 << 25) | 1 << 24)


/* ********************************************************************	*/
/* Data Processing op-codes (bits 27 - 26 => 00)			*/

#define DP(op)		((B_00 << 26) | ((op) << 21))

#define OP_AND	DP(B_0000)
#define OP_EOR	DP(B_0001)
#define OP_SUB	DP(B_0010)
#define OP_RSB	DP(B_0011)
#define OP_ADD	DP(B_0100)
#define OP_ADC	DP(B_0101)
#define OP_SBC	DP(B_0110)
#define OP_RSC	DP(B_0111)
#define OP_TST	DP(B_1000)
#define OP_TEQ	DP(B_1001)
#define OP_CMP	DP(B_1010)
#define OP_CMN	DP(B_1011)
#define OP_ORR	DP(B_1100)
#define OP_MOV	DP(B_1101)
#define OP_BIC	DP(B_1110)
#define OP_MVN	DP(B_1111)

/* Flags */
#define	DP_S		(1 << 20)	/* set cond codes */
#define DP_I		(1 << 25)	/* immediate operand, */
					/* default reg/shift */
#define DP_REGSHIFT	(1 << 4)	/* shift type is reg, default is imm */

/* opcode fields */

#define DP_DSTREG(r)	((r) << 12)	/* destination reg */
#define DP_OPREG1(r)	((r) << 16)	/* first operand reg */
#define DP_SHIFTREG(r)	((r) << 8)	/* register defining shift amount */
#define DP_IMMROT(x)	((x) << 8)	/* rotate field for immediate op */


/* ********************************************************************	*/
/* New ARM6 PSR TRANSFER op-codes (subset of existing dataproc instr.s)	*/

/* ARM6 ops MSR MRS (use of TST, TEQ, CMP and CMN ops without 'S' bit set) */
#define PSRTRAN(op)	((B_000100 << 22) | ((op) << 12))

#define OP_MRS	PSRTRAN(B_00011110000)
#define OP_MSR	PSRTRAN(B_01010001111)

#define MRS_DSTREG(r)	((r) << 12)	/* destination reg */
#define MSR_ALLPSR	(1 << 16)	/* alter entire PSR not just flags */

/* set PSR current or saved register selection */
#define A6CPSR		0
#define A6SPSR		1

#define A6PSR(psr)	((psr) << 22)

/* ********************************************************************	*/
/* MULTIPLY op-codes (bits 27 - 22 => 000000, bits 7 - 4 => 1001)	*/

#define OP_MUL	((B_000000 << 22) | (B_1001 << 4))
#define OP_MLA	((B_000000 << 22) | (1 << 21) | (B_1001 << 4))

/* Flags */

#define	MPY_S	(1 << 20)	/* set cond codes */

/* opcode fields */

#define MPY_DSTREG(r)	((r) << 16)	/* Rd */
#define MPY_SRC1REG(r)	(r)		/* Rm */
#define MPY_SRC2REG(r)	((r) << 8)	/* Rs */
#define MPY_SRC3REG(r)	((r) << 12)	/* Rn */


/* ********************************************************************	*/
/* Single Data Transfer op-codes (bits 27 - 26 => 01)			*/

#define	OP_LDR	((B_01 << 26) | 1 << 20)
#define	OP_STR	(B_01 << 26)

/* Flag bits */

#define SDT_B	(1 << 22)	/* byte transfer */
#define SDT_I	(1 << 25)	/* register/shift, default immediate data */

/* opcode fields */

#define SDT_REG(x)	((x) << 12)	/* src/dst reg */


/* ********************************************************************	*/
/* Block Data Transfer op-codes (bits 27 - 25 => 100)			*/

#define	OP_LDM	(B_100 << 25 | 1 << 20)
#define	OP_STM	(B_100 << 25)

/* Flag bits */

#define BDT_S	(1 << 22)	/* load psr or force user mode, '^' */
#define BDT_ERROR 0		/* note erroneous mnemonic (has no mode) */


/* ********************************************************************	*/
/* SoftWare Interrupt (TRAP) op-codes (bits 27 - 24 => 1111)		*/

#define OP_SWI	(B_1111 << 24)


/* ********************************************************************	*/
/* Single Data Swap instruction (ARM3) (bits 27 - 23 => 00010)		*/

#define OP_SWP	(B_00010 << 23 | B_1001 << 4)

#define SWP_B	(1 << 22)	/* byte / word selector */

/* opcode fields */

#define SWP_SRCREG(x)	(x)
#define SWP_DSTREG(x)	((x) << 12)


/* ********************************************************************	*/
/* Common coprocessor instruction manifests				*/

#define CP_CPNUM(x)	((x) << 8)	/* coproc number */
#define CP_AUX(x)	((x) << 5)	/* coproc aux field */


/* ********************************************************************	*/
/* Coprocessor Data Operations (bits 27 - 24 => 1110, bit 4 => 0)	*/

#define OP_CDO	(B_1110 << 24)

/* opcode fields */

#define CDO_CP_OP(x)	((x) << 20)	/* coproc opcode */
#define CDO_CP_CRn(x)	((x) << 16)	/* coproc reg m */
#define CDO_CP_CRm(x)	(x)		/* coproc reg m */
#define CDO_CP_CRDST(x)	((x) << 12)	/* coproc dest reg */


/* ********************************************************************	*/
/* Coprocessor Data Transfers (bits 27 - 25 => 110)			*/

#define OP_LDC	(B_110 << 25 | 1 << 20)
#define OP_STC	(B_110 << 25)

/* Flag bits */

#define CDT_N	(1 << 22)	/* 'N' long / short transfer */

/* opcode fields */

#define CDT_CP_REG(x)	((x) << 12)	/* coprocessor src/dst reg */
#define CDT_IMMOFF(x)	(x)		/* immediate offset */


/* ********************************************************************	*/
/* Coprocessor Register Transfers  (bits 27 - 24 => 1110, bit 4 => 1)	*/

#define OP_MRC	(B_1110 << 24 | 1 << 20 | 1 << 4)
#define OP_MCR	(B_1110 << 24 | 1 << 4)

/* opcode fields */

#define CRT_CP_OP(x)	((x) << 20)	/* coproc opcode */
#define CRT_CP_CRn(x)	((x) << 16)	/* coproc reg n */
#define CRT_CP_CRm(x)	(x)		/* coproc reg m */
#define CRT_SRCDST(x)	((x) << 12)	/* ARM core src/dst register */

/* ********************************************************************	*/
/* LEA Load Effective Address pseudo-opcode				*/
/* Creates either 'sub pc, symb + 8', or 'add pc symb - 8'		*/

#define OP_LEA	DP(B_0100)	/* defaults to ADD opcode */


#endif /*  __header_ARM_h */
