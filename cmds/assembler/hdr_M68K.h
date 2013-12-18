/*
 * File:	header_M68K.h
 * Subsystem:	Generic 68000 Assembler
 * Author:	P.A.Beskeen
 * Date:	Sept '91
 *
 * Description: M68000 specific header file for generic Helios assembler
 *
 * RcsId: $Id: hdr_M68K.h,v 1.1 1993/06/25 12:09:11 paul Exp $
 *
 * (C) Copyright 1992 Paul Beskeen
 * 
 * RcsLog: $Log: hdr_M68K.h,v $
 * Revision 1.1  1993/06/25  12:09:11  paul
 * Initial revision
 *
 *
 */

#ifndef __header_M68K_h
#define __header_M68K_h


#ifndef __M68KTARGET
# error Target processor architecture is not set correctly
#endif


#include "binary.h"


/* ********************************************************************	*/
/* *** M68K Processor Specific Structures ******************************	*/


/* ********************************************************************	*/
/* Mnemonic Structure							*/
/*									*/
/* The Mnemonic structure is attached to all Symbol structures in the	*/
/* hash table that correspond to CPU specific mneumonic keywords.	*/
/* (type = HT_MNEMONIC)							*/

typedef struct Mnemonic {
	char		*name;		/* mneumonics text name		*/
	int		token;		/* parser token number		*/
	unsigned	template;	/* binary instruction template	*/
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
} Instruction;


/* ********************************************************************	*/
/* Combine Commands							*/

/* default - no checking */
#define COM_REG		0

#if 0 /* No combines yet defined. */
/* check 24 bit pcrel value, left shift by two and subtract 2 for pipeline */
#define COM_PCREL_24	1
#endif /* 0 */


/* ********************************************************************	*/
/* Register values							*/

#if 0 /* No registers yet defined. */
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
#endif

/* ********************************************************************	*/
/* Condition codes 							*/

/*
 * For all possible conditions, define the binary code to entered into the
 * cond field of the instruction.
 */

#if 0 	/* No condition codes yet defined */
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
#endif


/* ********************************************************************	*/
/* Common Instruction Definitions					*/



/* ********************************************************************	*/
/* Individual Instruction Opcode Definitions				*/
/* ********************************************************************	*/


#if 0
/* ********************************************************************	*/
/* Branch op-codes 							*/



/* ********************************************************************	*/
/* Data Processing op-codes 						*/
#endif


#endif /*  __header_M68K_h */
