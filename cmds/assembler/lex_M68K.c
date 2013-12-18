/*
 * File:	lex_M68K.c
 * Subsystem:	Generic (M68K) Assembler
 * Author:	P.A.Beskeen
 * Date:	June '93
 *
 * Description: M68K specific lexer functions.
 *
 * RCSId: $Id: lex_M68K.c,v 1.1 1993/06/25 12:09:11 paul Exp $
 *
 * (C) Copyright 1993 Perihelion Software Ltd.
 * 
 * RCSLog: $Log: lex_M68K.c,v $
 * Revision 1.1  1993/06/25  12:09:11  paul
 * Initial revision
 *
 *
 */

/* Include Files: */

#include <stdlib.h>

#include "gasm.h"
#include "y.tab.h"
#include "ghof.h"


/*
 * This structure is used to initialise the hash table with the M68K specific
 * mnemonics. It associates the keyword with its lexer token value,
 * the condition code and the binary instruction template for that opcode.
 * 
 * N.B. keywords must be entered in lower case.
 *  
 * This description may seem needlessly verbose, but it leads to faster
 * initialisation and far less memory fragmentation.
 *
 */

static Mnemonic TheMnemonics[] =  {

	/* ************************************************************ */
	/* M68K Machine opcode mnemonics				*/
	/* ************************************************************ */

	/* TEXT		TOKEN	OPCODE		*/

	/* Currently there are no M68K specific mnemonics */

	/* trap op-codes 			*/
	{"trap",	TRAP,	B_01001110010 << 5	},

	{NULL, 0, 0}	/* **** END MARKER **** */
} ;



	/* ************************************************************ */
	/* M68K Register Tokens						*/
	/* ************************************************************ */

static Common TheRegs[] =  {
	/* TEXT		TOKEN		VALUE	*/

#if 0 /* Currently none */
	/* standard register names (user mode) */
	{"r0",		R0,		R_R0	},
	{"r1",		R1,		R_R1	},
	{"r2",		R2,		R_R2	},
	{"r3",		R3,		R_R3	},
	{"r4",		R4,		R_R4	},
	{"r5",		R5,		R_R5	},
	{"r6",		R6,		R_R6	},
	{"r7",		R7,		R_R7	},
	{"r8",		R8,		R_R8	},
	{"r9",		R9,		R_R9	},
	{"r10",		R10,		R_R10	},
	{"r11",		R11,		R_R11	},
	{"r12",		R12,		R_R12	},
	{"r13",		R13,		R_R13	},
	{"r14",		R14,		R_R14	},
	{"r15",		R15,		R_R15	},
#endif

	{NULL, 0, 0}	/* **** END MARKER **** */
} ;



	/* ************************************************************ */
	/* M68K specific linker instruction patch tokens		*/
	/* ************************************************************ */

static Common ThePatches[] =  {
	/* TEXT			TOKEN		VALUE	*/

	/* Currently there are no M68K specific linker patches */

	{NULL, 0, 0}	/* **** END MARKER **** */
} ;



	/* ************************************************************ */
	/* Extra M68K tokens to recognise				*/
	/* ************************************************************ */

static Common TheExtras[] =  {
	/* TEXT		TOKEN		VALUE	*/

	/* Currently there are no M68K specific extras */
#if 0
	{"lsl",		LSL,		OP_LSL},
#endif

	{NULL, 0, 0}	/* **** END MARKER **** */
} ;



/************************************************************************/
/* InitMnemonicKeywords							*/
/*									*/
/* Add a set of target CPU specific mnemonic keywords to the hash table */
/*									*/
/************************************************************************/

void InitMnemonicKeywords(void)
{
	struct Mnemonic		*cm = TheMnemonics;
	struct Common		*regs = TheRegs;
	struct Common		*ip = ThePatches;
	struct Common		*xtras = TheExtras;

	/* install mneumonics in hash table */
	while (cm->name != NULL) {
		NewSymbStruct(cm->name, HT_MNEMONIC, cm);
		cm++;
	}

	/* install registers in hash table */
	while (regs->name != NULL) {
		NewSymbStruct(regs->name, HT_TOKENVAL, regs);
		regs++;
	}

	/* install instruction patches in hash table */
	while (ip->name != NULL) {
		NewSymbStruct(ip->name, HT_TOKENVAL, ip);
		ip++;
	}

	/* install extra tokens in hash table */
	while (xtras->name != NULL) {
		NewSymbStruct(xtras->name, HT_TOKENVAL, xtras);
		xtras++;
	}
}



/* end of lex_M68K.c */
