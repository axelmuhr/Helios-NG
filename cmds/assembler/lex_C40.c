/*
 * File:	lex_C40.c
 * Subsystem:	Generic (C40) Assembler
 * Author:	P.A.Beskeen
 * Date:	Sept '91
 *
 * Description: TMS320C40 specific lexer functions
 *
 *		Defines the set of C40 specific mnemonics to recognise and the
 *		results to be sent on to the parser.
 *
 * RcsId: $Id: lex_C40.c,v 1.2 1992/04/15 11:00:57 paul Exp $
 *
 * (C) Copyright 1991 Perihelion Software Ltd.
 * 
 * $RcsLog$
 *
 */


/* Include Files: */

#include <stdlib.h>

#include "gasm.h"
#include "y.tab.h"
#include "ghof.h"

/*
 * This structure is used to initialise the hash table with the C40 specific
 * mnemonics. It associates the keyword with its lexer token value,
 * the condition code if that is defined by a conditional instruction or
 * byte/shift selector if it is that sort of instruction and the binary
 * instruction template for that instruction.
 */

static Mnemonic TheC40Mnemonics[] =  {

	/* ************************************************************ */
	/* C40 Machine opcode mnemonics					*/
	/* ************************************************************ */

	/* WORD LOAD and STORE INSTRUCTIONS */
	/* ================================ */

	/* TEXT		TOKEN	OPCODE		PAR_LD		PAR_ST */

	{"lda",		LDA,	OP_LDA,		0,		0	},
	{"lde",		LDE,	OP_LDE,		0,		0	},
	{"ldep",	LDEP,	OP_LDEP,	0,		0	},
	{"ldf",		LDF,	OP_LDF,		OP_LDF_LDF,	OP_LDF_STF },
	{"ldhi",	LDHI,	OP_LDHI,	0,		0	},
	{"ldi",		LDI,	OP_LDI,		OP_LDI_LDI,	OP_LDI_STI },
	{"ldm",		LDM,	OP_LDM,		0,		0	},
	/* LDP pseudo-op if reg == DP used LDPK, else use LDIU */
	{"ldp",		LDP,	OP_LDPK,	OP_LDIc | (C_U << 7), 0	},
	{"ldpe",	LDPE,	OP_LDPE,	0,		0	},
	{"ldpk",	LDPK,	OP_LDPK,	0,		0	},
	{"stf",		STF,	OP_STF,		0,		OP_STF_STF },
	{"sti",		STI,	OP_STI,		0,		OP_STI_STI },
	/* STIK is a pseudo-op of STI with different addressing modes */
	{"stik",	STIK,	OP_STI,		0,		0	},


	/* Conditional Loads						*/
	/* The condition codes are left shifted by seven as their 	*/
	/* position in conditional loads is seven places left (bits 	*/
	/* 27-23) from	that of normal conditional instructions (bits	*/
	/* 20-16).							*/

	/* TEXT		TOKEN		OPCODE			*NOT USED* */

	/* LDFcond */
	{"ldfu",	LDFcond,	OP_LDFc | (C_U << 7),	0,	0 },
	{"ldflo",	LDFcond,	OP_LDFc | (C_LO << 7),	0,	0 },
	{"ldfls",	LDFcond,	OP_LDFc | (C_LS << 7),	0,	0 },
	{"ldfhi",	LDFcond,	OP_LDFc | (C_HI << 7),	0,	0 },
	{"ldfhs",	LDFcond,	OP_LDFc | (C_HS << 7),	0,	0 },
	{"ldfeq",	LDFcond,	OP_LDFc | (C_EQ << 7),	0,	0 },
	{"ldfne",	LDFcond,	OP_LDFc | (C_NE << 7),	0,	0 },
	{"ldflt",	LDFcond,	OP_LDFc | (C_LT << 7),	0,	0 },
	{"ldfle",	LDFcond,	OP_LDFc | (C_LE << 7),	0,	0 },
	{"ldfgt",	LDFcond,	OP_LDFc | (C_GT << 7),	0,	0 },
	{"ldfge",	LDFcond,	OP_LDFc | (C_GE << 7),	0,	0 },
	{"ldfz",	LDFcond,	OP_LDFc | (C_Z << 7),	0,	0 },
	{"ldfnz",	LDFcond,	OP_LDFc | (C_NZ << 7),	0,	0 },
	{"ldfp",	LDFcond,	OP_LDFc | (C_P << 7),	0,	0 },
	{"ldfn",	LDFcond,	OP_LDFc | (C_N << 7),	0,	0 },
	{"ldfnn",	LDFcond,	OP_LDFc | (C_NN << 7),	0,	0 },
	{"ldfnv",	LDFcond,	OP_LDFc | (C_NV << 7),	0,	0 },
	{"ldfv",	LDFcond,	OP_LDFc | (C_V << 7),	0,	0 },
	{"ldfnuf",	LDFcond,	OP_LDFc | (C_NUF << 7),	0,	0 },
	{"ldfuf",	LDFcond,	OP_LDFc | (C_UF << 7),	0,	0 },
	{"ldfnc",	LDFcond,	OP_LDFc | (C_NC << 7),	0,	0 },
	{"ldfc",	LDFcond,	OP_LDFc | (C_C << 7),	0,	0 },
	{"ldfnlv",	LDFcond,	OP_LDFc | (C_NLV << 7),	0,	0 },
	{"ldflv",	LDFcond,	OP_LDFc | (C_LV << 7),	0,	0 },
	{"ldfnluf",	LDFcond,	OP_LDFc | (C_NLUF << 7),0,	0 },
	{"ldfluf",	LDFcond,	OP_LDFc | (C_LUF << 7),	0,	0 },
	{"ldfzuf",	LDFcond,	OP_LDFc | (C_ZUF << 7),	0,	0 },

	/* LDIcond */

	{"ldiu",	LDIcond,	OP_LDIc | (C_U << 7),	0,	0 },
	{"ldilo",	LDIcond,	OP_LDIc | (C_LO << 7),	0,	0 },
	{"ldils",	LDIcond,	OP_LDIc | (C_LS << 7),	0,	0 },
	{"ldihi",	LDIcond,	OP_LDIc | (C_HI << 7),	0,	0 },
	{"ldihs",	LDIcond,	OP_LDIc | (C_HS << 7),	0,	0 },
	{"ldieq",	LDIcond,	OP_LDIc | (C_EQ << 7),	0,	0 },
	{"ldine",	LDIcond,	OP_LDIc | (C_NE << 7),	0,	0 },
	{"ldilt",	LDIcond,	OP_LDIc | (C_LT << 7),	0,	0 },
	{"ldile",	LDIcond,	OP_LDIc | (C_LE << 7),	0,	0 },
	{"ldigt",	LDIcond,	OP_LDIc | (C_GT << 7),	0,	0 },
	{"ldige",	LDIcond,	OP_LDIc | (C_GE << 7),	0,	0 },
	{"ldiz",	LDIcond,	OP_LDIc | (C_Z << 7),	0,	0 },
	{"ldinz",	LDIcond,	OP_LDIc | (C_NZ << 7),	0,	0 },
	{"ldip",	LDIcond,	OP_LDIc | (C_P << 7),	0,	0 },
	{"ldin",	LDIcond,	OP_LDIc | (C_N << 7),	0,	0 },
	{"ldinn",	LDIcond,	OP_LDIc | (C_NN << 7),	0,	0 },
	{"ldinv",	LDIcond,	OP_LDIc | (C_NV << 7),	0,	0 },
	{"ldiv",	LDIcond,	OP_LDIc | (C_V << 7),	0,	0 },
	{"ldinuf",	LDIcond,	OP_LDIc | (C_NUF << 7),	0,	0 },
	{"ldiuf",	LDIcond,	OP_LDIc | (C_UF << 7),	0,	0 },
	{"ldinc",	LDIcond,	OP_LDIc | (C_NC << 7),	0,	0 },
	{"ldic",	LDIcond,	OP_LDIc | (C_C << 7),	0,	0 },
	{"ldinlv",	LDIcond,	OP_LDIc | (C_NLV << 7),	0,	0 },
	{"ldilv",	LDIcond,	OP_LDIc | (C_LV << 7),	0,	0 },
	{"ldinluf",	LDIcond,	OP_LDIc | (C_NLUF << 7),0,	0 },
	{"ldiluf",	LDIcond,	OP_LDIc | (C_LUF << 7),	0,	0 },
	{"ldizuf",	LDIcond,	OP_LDIc | (C_ZUF << 7),	0,	0 },


	/* PART WORD LOAD and STORE INSTRUCTIONS */
	/* ===================================== */

	/*
	 * Note half word icnstructions only use bit 23, bit 24 being used to
	 * select different instrucctions. Byte instructions use bits 24 and
	 * 23 to select a particular byte.
	 */

	/* TEXT		TOKEN		OPCODE			*NOT USED* */

	/* LBb */
	{"lb0",		LBb,		OP_LB | (0 << 23),	0,	0 },
	{"lb1",		LBb,		OP_LB | (1 << 23),	0,	0 },
	{"lb2",		LBb,		OP_LB | (2 << 23),	0,	0 },
	{"lb3",		LBb,		OP_LB | (3 << 23),	0,	0 },

	/* LBUb  */
	{"lbu0",	LBUb,		OP_LBU | (0 << 23),	0,	0 },
	{"lbu1",	LBUb,		OP_LBU | (1 << 23),	0,	0 },
	{"lbu2",	LBUb,		OP_LBU | (2 << 23),	0,	0 },
	{"lbu3",	LBUb,		OP_LBU | (3 << 23),	0,	0 },

	/* MBct */
	{"mb0",		MBct,		OP_MB | (0 << 23),	0,	0 },
	{"mb1",		MBct,		OP_MB | (1 << 23),	0,	0 },
	{"mb2",		MBct,		OP_MB | (2 << 23),	0,	0 },
	{"mb3",		MBct,		OP_MB | (3 << 23),	0,	0 },

	/* LWLct */
	{"lwl0",	LWLct,		OP_LWL | (0 << 23),	0,	0 },
	{"lwl1",	LWLct,		OP_LWL | (1 << 23),	0,	0 },
	{"lwl2",	LWLct,		OP_LWL | (2 << 23),	0,	0 },
	{"lwl3",	LWLct,		OP_LWL | (3 << 23),	0,	0 },

	/* LWRct */
	{"lwr0",	LWRct,		OP_LWR | (0 << 23),	0,	0 },
	{"lwr1",	LWRct,		OP_LWR | (1 << 23),	0,	0 },
	{"lwr2",	LWRct,		OP_LWR | (2 << 23),	0,	0 },
	{"lwr3",	LWRct,		OP_LWR | (3 << 23),	0,	0 },

	/* MHct */
	{"mh0",		MHct,		OP_MH | (0 << 23),	0,	0 },
	{"mh1",		MHct,		OP_MH | (1 << 23),	0,	0 },

	/* LHw */
	{"lh0",		LHw,		OP_LH | (0 << 23),	0,	0 },
	{"lh1",		LHw,		OP_LH | (1 << 23),	0,	0 },

	/* LHUw */
	{"lhu0",	LHUw,		OP_LHU | (0 << 23),	0,	0 },
	{"lhu1",	LHUw,		OP_LHU | (1 << 23),	0,	0 },


	/* DIADIC INSTRUCTIONS */
	/* =================== */

	/* TEXT		TOKEN	DIADIC		TRIADIC		PAR_ST	*/

	{"absf",	ABSF,	OP_ABSF,	0,		OP_ABSF_STF },
	{"absi",	ABSI,	OP_ABSI,	0,		OP_ABSI_STI },
	{"addc",	ADDC,	OP_ADDC,	OP_ADDC3,	0	},
	{"addf",	ADDF,	OP_ADDF,	OP_ADDF3,	OP_ADDF3_STF },
	{"addi",	ADDI,	OP_ADDI,	OP_ADDI3,	OP_ADDI3_STI },
	{"and",		AND,	OP_AND,		OP_AND3,	OP_AND3_STI },
	{"andn",	ANDN,	OP_ANDN,	OP_ANDN3,	0	},
	{"ash",		ASH,	OP_ASH,		OP_ASH3,	OP_ASH3_STI },
	{"cmpf",	CMPF,	OP_CMPF,	OP_CMPF3,	0	},
	{"cmpi",	CMPI,	OP_CMPI,	OP_CMPI3,	0	},
	{"fix",		FIX,	OP_FIX,		0,		OP_FIX_STI },
	{"float",	FLOAT,	OP_FLOAT,	0,		OP_FLOAT_STF },
	{"frieee",	FRIEEE,	OP_FRIEEE,	0,		OP_FRIEEE_STF },
	{"lsh",		LSH,	OP_LSH,		OP_LSH3,	OP_LSH3_STI },
	{"mpyf",	MPYF,	OP_MPYF,	OP_MPYF3,	OP_MPYF3_STF },
	{"mpyi",	MPYI,	OP_MPYI,	OP_MPYI3,	OP_MPYI3_STI },
	{"mpyshi",	MPYSHI,	OP_MPYSHI,	OP_MPYSHI3,	0	},
	{"mpyuhi",	MPYUHI,	OP_MPYUHI,	OP_MPYUHI3,	0	},
	{"negb",	NEGB,	OP_NEGB,	0,		0	},
	{"negf",	NEGF,	OP_NEGF,	0,		OP_NEGF_STF },
	{"negi",	NEGI,	OP_NEGI,	0,		OP_NEGI_STI },
	{"norm",	NORM,	OP_NORM,	0,		0	},
	{"not",		NOT,	OP_NOT,		0,		OP_NOT_STI },
	{"or",		OR,	OP_OR,		OP_OR3,		OP_OR3_STI },
	{"rcpf",	RCPF,	OP_RCPF,	0,		0	},
	{"rnd",		RND,	OP_RND,		0,		0	},
	{"rol",		ROL,	OP_ROL,		0,		0	},
	{"rolc",	ROLC,	OP_ROLC,	0,		0	},
	{"ror",		ROR,	OP_ROR,		0,		0	},
	{"rorc",	RORC,	OP_RORC,	0,		0	},
	{"rsqrf",	RSQRF,	OP_RSQRF,	0,		0	},
	{"subb",	SUBB,	OP_SUBB,	OP_SUBB3,	0	},
	{"subc",	SUBC,	OP_SUBC,	0,		0	},
	{"subf",	SUBF,	OP_SUBF,	OP_SUBF3,	OP_SUBF3_STF },
	{"subi",	SUBI,	OP_SUBI,	OP_SUBI3,	OP_SUBI3_STI },
	{"subrb",	SUBRB,	OP_SUBRB,	0,		0	},
	{"subrf",	SUBRF,	OP_SUBRF,	0,		0	},
	{"subri",	SUBRI,	OP_SUBRI,	0,		0	},
	{"toieee",	TOIEEE,	OP_TOIEEE,	0,		OP_TOIEEE_STF },
	{"tstb",	TSTB,	OP_TSTB,	OP_TSTB3,	0	},
	{"xor",		XOR,	OP_XOR,		OP_XOR3,	OP_XOR3_STI },


	/* TRIADIC INSTRUCTIONS */
	/* ==================== */

	/* Parser will also accept mnemonics without the appended 3, */
	/* but should only accept xxx3's if it is a triadic instruction */

	/* TEXT 	TOKEN	DIADIC	TRIADIC 	PAR_ST	*/

	{"addc3",	ADDC3,	0,	OP_ADDC3,	0	},
	{"addf3",	ADDF3,	0,	OP_ADDF3,	OP_ADDF3_STF },
	{"addi3",	ADDI3,	0,	OP_ADDI3,	OP_ADDI3_STI },
	{"and3",	AND3,	0,	OP_AND3,	OP_AND3_STI },
	{"andn3",	ANDN3,	0,	OP_ANDN3,	0	},
	{"ash3",	ASH3,	0,	OP_ASH3,	OP_ASH3_STI },
	{"cmpf3",	CMPF3,	0,	OP_CMPF3,	0	},
	{"cmpi3",	CMPI3,	0,	OP_CMPI3,	0	},
	{"lsh3",	LSH3,	0,	OP_LSH3,	OP_LSH3_STI },
	{"mpyf3",	MPYF3,	0,	OP_MPYF3,	OP_MPYF3_STF },
	{"mpyi3",	MPYI3,	0,	OP_MPYI3,	OP_MPYI3_STI },
	{"mpyshi3",	MPYSHI3,0,	OP_MPYSHI3,	0	},
	{"mpyuhi3",	MPYUHI3,0,	OP_MPYUHI3,	0	},
	{"or3",		OR3,	0,	OP_OR3,		OP_OR3_STI },
	{"subb3",	SUBB3,	0,	OP_SUBB3,	0	},
	{"subf3",	SUBF3,	0,	OP_SUBF3,	OP_SUBF3_STF },
	{"subi3",	SUBI3,	0,	OP_SUBI3,	OP_SUBI3_STI },
	{"tstb3",	TSTB3,	0,	OP_TSTB3,	0	},
	{"xor3",	XOR3,	0,	OP_XOR3,	OP_XOR3_STI },


	/* PROGRAM CONTROL */
	/* =============== */

	/* TEXT 	TOKEN		STD OPCODE		ALT  NOT USED */

	{"br",		BR,		OP_BR,			0,	0 },
	{"brd",		BRD,		OP_BRD,			0,	0 },
	{"call",	CALL,		OP_CALL,		0,	0 },
	{"laj",		LAJ,		OP_LAJ,			0,	0 },
	{"rptb",	RPTB,		OP_RPTBp,		OP_RPTBr,  0 },
	{"rptbd",	RPTBD,		OP_RPTBDp,		OP_RPTBDr, 0 },
	{"rpts",	RPTS,		OP_RPTS,		0,	0 },

	/* Bcond */
	{"b",		Bcond,		OP_Bcond | C_U,		0,	0 },
	{"bu",		Bcond,		OP_Bcond | C_U,		0,	0 },
	{"blo",		Bcond,		OP_Bcond | C_LO,	0,	0 },
	{"bls",		Bcond,		OP_Bcond | C_LS,	0,	0 },
	{"bhi",		Bcond,		OP_Bcond | C_HI,	0,	0 },
	{"bhs",		Bcond,		OP_Bcond | C_HS,	0,	0 },
	{"beq",		Bcond,		OP_Bcond | C_EQ,	0,	0 },
	{"bne",		Bcond,		OP_Bcond | C_NE,	0,	0 },
	{"blt",		Bcond,		OP_Bcond | C_LT,	0,	0 },
	{"ble",		Bcond,		OP_Bcond | C_LE,	0,	0 },
	{"bgt",		Bcond,		OP_Bcond | C_GT,	0,	0 },
	{"bge",		Bcond,		OP_Bcond | C_GE,	0,	0 },
	{"bz",		Bcond,		OP_Bcond | C_Z,		0,	0 },
	{"bnz",		Bcond,		OP_Bcond | C_NZ,	0,	0 },
	{"bp",		Bcond,		OP_Bcond | C_P,		0,	0 },
	{"bn",		Bcond,		OP_Bcond | C_N,		0,	0 },
	{"bnn",		Bcond,		OP_Bcond | C_NN,	0,	0 },
	{"bnv",		Bcond,		OP_Bcond | C_NV,	0,	0 },
	{"bv",		Bcond,		OP_Bcond | C_V,		0,	0 },
	{"bnuf",	Bcond,		OP_Bcond | C_NUF,	0,	0 },
	{"buf",		Bcond,		OP_Bcond | C_UF,	0,	0 },
	{"bnc",		Bcond,		OP_Bcond | C_NC,	0,	0 },
	{"bc",		Bcond,		OP_Bcond | C_C,		0,	0 },
	{"bnlv",	Bcond,		OP_Bcond | C_NLV,	0,	0 },
	{"blv",		Bcond,		OP_Bcond | C_LV,	0,	0 },
	{"bnluf",	Bcond,		OP_Bcond | C_NLUF,	0,	0 },
	{"bluf",	Bcond,		OP_Bcond | C_LUF,	0,	0 },
	{"bzuf",	Bcond,		OP_Bcond | C_ZUF,	0,	0 },

	/* BcondAF */
	{"buaf",	BcondAF,	OP_BcondAF | C_U,	0,	0 },
	{"bloaf",	BcondAF,	OP_BcondAF | C_LO,	0,	0 },
	{"blsaf",	BcondAF,	OP_BcondAF | C_LS,	0,	0 },
	{"bhiaf",	BcondAF,	OP_BcondAF | C_HI,	0,	0 },
	{"bhsaf",	BcondAF,	OP_BcondAF | C_HS,	0,	0 },
	{"beqaf",	BcondAF,	OP_BcondAF | C_EQ,	0,	0 },
	{"bneaf",	BcondAF,	OP_BcondAF | C_NE,	0,	0 },
	{"bltaf",	BcondAF,	OP_BcondAF | C_LT,	0,	0 },
	{"bleaf",	BcondAF,	OP_BcondAF | C_LE,	0,	0 },
	{"bgtaf",	BcondAF,	OP_BcondAF | C_GT,	0,	0 },
	{"bgeaf",	BcondAF,	OP_BcondAF | C_GE,	0,	0 },
	{"bzaf",	BcondAF,	OP_BcondAF | C_Z,	0,	0 },
	{"bnzaf",	BcondAF,	OP_BcondAF | C_NZ,	0,	0 },
	{"bpaf",	BcondAF,	OP_BcondAF | C_P,	0,	0 },
	{"bnaf",	BcondAF,	OP_BcondAF | C_N,	0,	0 },
	{"bnnaf",	BcondAF,	OP_BcondAF | C_NN,	0,	0 },
	{"bnvaf",	BcondAF,	OP_BcondAF | C_NV,	0,	0 },
	{"bvaf",	BcondAF,	OP_BcondAF | C_V,	0,	0 },
	{"bnufaf",	BcondAF,	OP_BcondAF | C_NUF,	0,	0 },
	{"bufaf",	BcondAF,	OP_BcondAF | C_UF,	0,	0 },
	{"bncaf",	BcondAF,	OP_BcondAF | C_NC,	0,	0 },
	{"bcaf",	BcondAF,	OP_BcondAF | C_C,	0,	0 },
	{"bnlvaf",	BcondAF,	OP_BcondAF | C_NLV,	0,	0 },
	{"blvaf",	BcondAF,	OP_BcondAF | C_LV,	0,	0 },
	{"bnlufaf",	BcondAF,	OP_BcondAF | C_NLUF,	0,	0 },
	{"blufaf",	BcondAF,	OP_BcondAF | C_LUF,	0,	0 },
	{"bzufaf",	BcondAF,	OP_BcondAF | C_ZUF,	0,	0 },

	/* BcondAT */
	{"buat",	BcondAT,	OP_BcondAT | C_U,	0,	0 },
	{"bloat",	BcondAT,	OP_BcondAT | C_LO,	0,	0 },
	{"blsat",	BcondAT,	OP_BcondAT | C_LS,	0,	0 },
	{"bhiat",	BcondAT,	OP_BcondAT | C_HI,	0,	0 },
	{"bhsat",	BcondAT,	OP_BcondAT | C_HS,	0,	0 },
	{"beqat",	BcondAT,	OP_BcondAT | C_EQ,	0,	0 },
	{"bneat",	BcondAT,	OP_BcondAT | C_NE,	0,	0 },
	{"bltat",	BcondAT,	OP_BcondAT | C_LT,	0,	0 },
	{"bleat",	BcondAT,	OP_BcondAT | C_LE,	0,	0 },
	{"bgtat",	BcondAT,	OP_BcondAT | C_GT,	0,	0 },
	{"bgeat",	BcondAT,	OP_BcondAT | C_GE,	0,	0 },
	{"bzat",	BcondAT,	OP_BcondAT | C_Z,	0,	0 },
	{"bnzat",	BcondAT,	OP_BcondAT | C_NZ,	0,	0 },
	{"bpat",	BcondAT,	OP_BcondAT | C_P,	0,	0 },
	{"bnat",	BcondAT,	OP_BcondAT | C_N,	0,	0 },
	{"bnnat",	BcondAT,	OP_BcondAT | C_NN,	0,	0 },
	{"bnvat",	BcondAT,	OP_BcondAT | C_NV,	0,	0 },
	{"bvat",	BcondAT,	OP_BcondAT | C_V,	0,	0 },
	{"bnufat",	BcondAT,	OP_BcondAT | C_NUF,	0,	0 },
	{"bufat",	BcondAT,	OP_BcondAT | C_UF,	0,	0 },
	{"bncat",	BcondAT,	OP_BcondAT | C_NC,	0,	0 },
	{"bcat",	BcondAT,	OP_BcondAT | C_C,	0,	0 },
	{"bnlvat",	BcondAT,	OP_BcondAT | C_NLV,	0,	0 },
	{"blvat",	BcondAT,	OP_BcondAT | C_LV,	0,	0 },
	{"bnlufat",	BcondAT,	OP_BcondAT | C_NLUF,	0,	0 },
	{"blufat",	BcondAT,	OP_BcondAT | C_LUF,	0,	0 },
	{"bzufat",	BcondAT,	OP_BcondAT | C_ZUF,	0,	0 },

	/* BcondD */
	{"bud",		BcondD,		OP_BcondD | C_U,	0,	0 },
	{"blod",	BcondD,		OP_BcondD | C_LO,	0,	0 },
	{"blsd",	BcondD,		OP_BcondD | C_LS,	0,	0 },
	{"bhid",	BcondD,		OP_BcondD | C_HI,	0,	0 },
	{"bhsd",	BcondD,		OP_BcondD | C_HS,	0,	0 },
	{"beqd",	BcondD,		OP_BcondD | C_EQ,	0,	0 },
	{"bned",	BcondD,		OP_BcondD | C_NE,	0,	0 },
	{"bltd",	BcondD,		OP_BcondD | C_LT,	0,	0 },
	{"bled",	BcondD,		OP_BcondD | C_LE,	0,	0 },
	{"bgtd",	BcondD,		OP_BcondD | C_GT,	0,	0 },
	{"bged",	BcondD,		OP_BcondD | C_GE,	0,	0 },
	{"bzd",		BcondD,		OP_BcondD | C_Z,	0,	0 },
	{"bnzd",	BcondD,		OP_BcondD | C_NZ,	0,	0 },
	{"bpd",		BcondD,		OP_BcondD | C_P,	0,	0 },
	{"bnd",		BcondD,		OP_BcondD | C_N,	0,	0 },
	{"bnnd",	BcondD,		OP_BcondD | C_NN,	0,	0 },
	{"bnvd",	BcondD,		OP_BcondD | C_NV,	0,	0 },
	{"bvd",		BcondD,		OP_BcondD | C_V,	0,	0 },
	{"bnufd",	BcondD,		OP_BcondD | C_NUF,	0,	0 },
	{"bufd",	BcondD,		OP_BcondD | C_UF,	0,	0 },
	{"bncd",	BcondD,		OP_BcondD | C_NC,	0,	0 },
	{"bcd",		BcondD,		OP_BcondD | C_C,	0,	0 },
	{"bnlvd",	BcondD,		OP_BcondD | C_NLV,	0,	0 },
	{"blvd",	BcondD,		OP_BcondD | C_LV,	0,	0 },
	{"bnlufd",	BcondD,		OP_BcondD | C_NLUF,	0,	0 },
	{"blufd",	BcondD,		OP_BcondD | C_LUF,	0,	0 },
	{"bzufd",	BcondD,		OP_BcondD | C_ZUF,	0,	0 },

	/* CALLcond */
	{"callu",	CALLcond,	OP_CALLcond | C_U,	0,	0 },
	{"calllo",	CALLcond,	OP_CALLcond | C_LO,	0,	0 },
	{"callls",	CALLcond,	OP_CALLcond | C_LS,	0,	0 },
	{"callhi",	CALLcond,	OP_CALLcond | C_HI,	0,	0 },
	{"callhs",	CALLcond,	OP_CALLcond | C_HS,	0,	0 },
	{"calleq",	CALLcond,	OP_CALLcond | C_EQ,	0,	0 },
	{"callne",	CALLcond,	OP_CALLcond | C_NE,	0,	0 },
	{"calllt",	CALLcond,	OP_CALLcond | C_LT,	0,	0 },
	{"callle",	CALLcond,	OP_CALLcond | C_LE,	0,	0 },
	{"callgt",	CALLcond,	OP_CALLcond | C_GT,	0,	0 },
	{"callge",	CALLcond,	OP_CALLcond | C_GE,	0,	0 },
	{"callz",	CALLcond,	OP_CALLcond | C_Z,	0,	0 },
	{"callnz",	CALLcond,	OP_CALLcond | C_NZ,	0,	0 },
	{"callp",	CALLcond,	OP_CALLcond | C_P,	0,	0 },
	{"calln",	CALLcond,	OP_CALLcond | C_N,	0,	0 },
	{"callnn",	CALLcond,	OP_CALLcond | C_NN,	0,	0 },
	{"callnv",	CALLcond,	OP_CALLcond | C_NV,	0,	0 },
	{"callv",	CALLcond,	OP_CALLcond | C_V,	0,	0 },
	{"callnuf",	CALLcond,	OP_CALLcond | C_NUF,	0,	0 },
	{"calluf",	CALLcond,	OP_CALLcond | C_UF,	0,	0 },
	{"callnc",	CALLcond,	OP_CALLcond | C_NC,	0,	0 },
	{"callc",	CALLcond,	OP_CALLcond | C_C,	0,	0 },
	{"callnlv",	CALLcond,	OP_CALLcond | C_NLV,	0,	0 },
	{"calllv",	CALLcond,	OP_CALLcond | C_LV,	0,	0 },
	{"callnluf",	CALLcond,	OP_CALLcond | C_NLUF,	0,	0 },
	{"callluf",	CALLcond,	OP_CALLcond | C_LUF,	0,	0 },
	{"callzuf",	CALLcond,	OP_CALLcond | C_ZUF,	0,	0 },

	/* DBcond */
	{"dbu",		DBcond,		OP_DBcond | C_U,	0,	0 },
	{"dblo",	DBcond,		OP_DBcond | C_LO,	0,	0 },
	{"dbls",	DBcond,		OP_DBcond | C_LS,	0,	0 },
	{"dbhi",	DBcond,		OP_DBcond | C_HI,	0,	0 },
	{"dbhs",	DBcond,		OP_DBcond | C_HS,	0,	0 },
	{"dbeq",	DBcond,		OP_DBcond | C_EQ,	0,	0 },
	{"dbne",	DBcond,		OP_DBcond | C_NE,	0,	0 },
	{"dblt",	DBcond,		OP_DBcond | C_LT,	0,	0 },
	{"dble",	DBcond,		OP_DBcond | C_LE,	0,	0 },
	{"dbgt",	DBcond,		OP_DBcond | C_GT,	0,	0 },
	{"dbge",	DBcond,		OP_DBcond | C_GE,	0,	0 },
	{"dbz",		DBcond,		OP_DBcond | C_Z,	0,	0 },
	{"dbnz",	DBcond,		OP_DBcond | C_NZ,	0,	0 },
	{"dbp",		DBcond,		OP_DBcond | C_P,	0,	0 },
	{"dbn",		DBcond,		OP_DBcond | C_N,	0,	0 },
	{"dbnn",	DBcond,		OP_DBcond | C_NN,	0,	0 },
	{"dbnv",	DBcond,		OP_DBcond | C_NV,	0,	0 },
	{"dbv",		DBcond,		OP_DBcond | C_V,	0,	0 },
	{"dbnuf",	DBcond,		OP_DBcond | C_NUF,	0,	0 },
	{"dbuf",	DBcond,		OP_DBcond | C_UF,	0,	0 },
	{"dbnc",	DBcond,		OP_DBcond | C_NC,	0,	0 },
	{"dbc",		DBcond,		OP_DBcond | C_C,	0,	0 },
	{"dbnlv",	DBcond,		OP_DBcond | C_NLV,	0,	0 },
	{"dblv",	DBcond,		OP_DBcond | C_LV,	0,	0 },
	{"dbnluf",	DBcond,		OP_DBcond | C_NLUF,	0,	0 },
	{"dbluf",	DBcond,		OP_DBcond | C_LUF,	0,	0 },
	{"dbzuf",	DBcond,		OP_DBcond | C_ZUF,	0,	0 },

	/* DBcondD */
	{"dbud",	DBcondD,	OP_DBcondD | C_U,	0,	0 },
	{"dblod",	DBcondD,	OP_DBcondD | C_LO,	0,	0 },
	{"dblsd",	DBcondD,	OP_DBcondD | C_LS,	0,	0 },
	{"dbhid",	DBcondD,	OP_DBcondD | C_HI,	0,	0 },
	{"dbhsd",	DBcondD,	OP_DBcondD | C_HS,	0,	0 },
	{"dbeqd",	DBcondD,	OP_DBcondD | C_EQ,	0,	0 },
	{"dbned",	DBcondD,	OP_DBcondD | C_NE,	0,	0 },
	{"dbltd",	DBcondD,	OP_DBcondD | C_LT,	0,	0 },
	{"dbled",	DBcondD,	OP_DBcondD | C_LE,	0,	0 },
	{"dbgtd",	DBcondD,	OP_DBcondD | C_GT,	0,	0 },
	{"dbged",	DBcondD,	OP_DBcondD | C_GE,	0,	0 },
	{"dbzd",	DBcondD,	OP_DBcondD | C_Z,	0,	0 },
	{"dbnzd",	DBcondD,	OP_DBcondD | C_NZ,	0,	0 },
	{"dbpd",	DBcondD,	OP_DBcondD | C_P,	0,	0 },
	{"dbnd",	DBcondD,	OP_DBcondD | C_N,	0,	0 },
	{"dbnnd",	DBcondD,	OP_DBcondD | C_NN,	0,	0 },
	{"dbnvd",	DBcondD,	OP_DBcondD | C_NV,	0,	0 },
	{"dbvd",	DBcondD,	OP_DBcondD | C_V,	0,	0 },
	{"dbnufd",	DBcondD,	OP_DBcondD | C_NUF,	0,	0 },
	{"dbufd",	DBcondD,	OP_DBcondD | C_UF,	0,	0 },
	{"dbncd",	DBcondD,	OP_DBcondD | C_NC,	0,	0 },
	{"dbcd",	DBcondD,	OP_DBcondD | C_C,	0,	0 },
	{"dbnlvd",	DBcondD,	OP_DBcondD | C_NLV,	0,	0 },
	{"dblvd",	DBcondD,	OP_DBcondD | C_LV,	0,	0 },
	{"dbnlufd",	DBcondD,	OP_DBcondD | C_NLUF,	0,	0 },
	{"dblufd",	DBcondD,	OP_DBcondD | C_LUF,	0,	0 },
	{"dbzufd",	DBcondD,	OP_DBcondD | C_ZUF,	0,	0 },

	/* LAJcond */
	{"laju",	LAJcond,	OP_LAJcond | C_U,	0,	0 },
	{"lajlo",	LAJcond,	OP_LAJcond | C_LO,	0,	0 },
	{"lajls",	LAJcond,	OP_LAJcond | C_LS,	0,	0 },
	{"lajhi",	LAJcond,	OP_LAJcond | C_HI,	0,	0 },
	{"lajhs",	LAJcond,	OP_LAJcond | C_HS,	0,	0 },
	{"lajeq",	LAJcond,	OP_LAJcond | C_EQ,	0,	0 },
	{"lajne",	LAJcond,	OP_LAJcond | C_NE,	0,	0 },
	{"lajlt",	LAJcond,	OP_LAJcond | C_LT,	0,	0 },
	{"lajle",	LAJcond,	OP_LAJcond | C_LE,	0,	0 },
	{"lajgt",	LAJcond,	OP_LAJcond | C_GT,	0,	0 },
	{"lajge",	LAJcond,	OP_LAJcond | C_GE,	0,	0 },
	{"lajz",	LAJcond,	OP_LAJcond | C_Z,	0,	0 },
	{"lajnz",	LAJcond,	OP_LAJcond | C_NZ,	0,	0 },
	{"lajp",	LAJcond,	OP_LAJcond | C_P,	0,	0 },
	{"lajn",	LAJcond,	OP_LAJcond | C_N,	0,	0 },
	{"lajnn",	LAJcond,	OP_LAJcond | C_NN,	0,	0 },
	{"lajnv",	LAJcond,	OP_LAJcond | C_NV,	0,	0 },
	{"lajv",	LAJcond,	OP_LAJcond | C_V,	0,	0 },
	{"lajnuf",	LAJcond,	OP_LAJcond | C_NUF,	0,	0 },
	{"lajuf",	LAJcond,	OP_LAJcond | C_UF,	0,	0 },
	{"lajnc",	LAJcond,	OP_LAJcond | C_NC,	0,	0 },
	{"lajc",	LAJcond,	OP_LAJcond | C_C,	0,	0 },
	{"lajnlv",	LAJcond,	OP_LAJcond | C_NLV,	0,	0 },
	{"lajlv",	LAJcond,	OP_LAJcond | C_LV,	0,	0 },
	{"lajnluf",	LAJcond,	OP_LAJcond | C_NLUF,	0,	0 },
	{"lajluf",	LAJcond,	OP_LAJcond | C_LUF,	0,	0 },
	{"lajzuf",	LAJcond,	OP_LAJcond | C_ZUF,	0,	0 },

	/* LATcond */
	{"latu",	LATcond,	OP_LATcond | C_U,	0,	0 },
	{"latlo",	LATcond,	OP_LATcond | C_LO,	0,	0 },
	{"latls",	LATcond,	OP_LATcond | C_LS,	0,	0 },
	{"lathi",	LATcond,	OP_LATcond | C_HI,	0,	0 },
	{"laths",	LATcond,	OP_LATcond | C_HS,	0,	0 },
	{"lateq",	LATcond,	OP_LATcond | C_EQ,	0,	0 },
	{"latne",	LATcond,	OP_LATcond | C_NE,	0,	0 },
	{"latlt",	LATcond,	OP_LATcond | C_LT,	0,	0 },
	{"latle",	LATcond,	OP_LATcond | C_LE,	0,	0 },
	{"latgt",	LATcond,	OP_LATcond | C_GT,	0,	0 },
	{"latge",	LATcond,	OP_LATcond | C_GE,	0,	0 },
	{"latz",	LATcond,	OP_LATcond | C_Z,	0,	0 },
	{"latnz",	LATcond,	OP_LATcond | C_NZ,	0,	0 },
	{"latp",	LATcond,	OP_LATcond | C_P,	0,	0 },
	{"latn",	LATcond,	OP_LATcond | C_N,	0,	0 },
	{"latnn",	LATcond,	OP_LATcond | C_NN,	0,	0 },
	{"latnv",	LATcond,	OP_LATcond | C_NV,	0,	0 },
	{"latv",	LATcond,	OP_LATcond | C_V,	0,	0 },
	{"latnuf",	LATcond,	OP_LATcond | C_NUF,	0,	0 },
	{"latuf",	LATcond,	OP_LATcond | C_UF,	0,	0 },
	{"latnc",	LATcond,	OP_LATcond | C_NC,	0,	0 },
	{"latc",	LATcond,	OP_LATcond | C_C,	0,	0 },
	{"latnlv",	LATcond,	OP_LATcond | C_NLV,	0,	0 },
	{"latlv",	LATcond,	OP_LATcond | C_LV,	0,	0 },
	{"latnluf",	LATcond,	OP_LATcond | C_NLUF,	0,	0 },
	{"latluf",	LATcond,	OP_LATcond | C_LUF,	0,	0 },
	{"latzuf",	LATcond,	OP_LATcond | C_ZUF,	0,	0 },

	/* RETIcond  */
	{"retiu",	RETIcond,	OP_RETIcond | C_U,	0,	0 },
	{"retilo",	RETIcond,	OP_RETIcond | C_LO,	0,	0 },
	{"retils",	RETIcond,	OP_RETIcond | C_LS,	0,	0 },
	{"retihi",	RETIcond,	OP_RETIcond | C_HI,	0,	0 },
	{"retihs",	RETIcond,	OP_RETIcond | C_HS,	0,	0 },
	{"retieq",	RETIcond,	OP_RETIcond | C_EQ,	0,	0 },
	{"retine",	RETIcond,	OP_RETIcond | C_NE,	0,	0 },
	{"retilt",	RETIcond,	OP_RETIcond | C_LT,	0,	0 },
	{"retile",	RETIcond,	OP_RETIcond | C_LE,	0,	0 },
	{"retigt",	RETIcond,	OP_RETIcond | C_GT,	0,	0 },
	{"retige",	RETIcond,	OP_RETIcond | C_GE,	0,	0 },
	{"retiz",	RETIcond,	OP_RETIcond | C_Z,	0,	0 },
	{"retinz",	RETIcond,	OP_RETIcond | C_NZ,	0,	0 },
	{"retip",	RETIcond,	OP_RETIcond | C_P,	0,	0 },
	{"retin",	RETIcond,	OP_RETIcond | C_N,	0,	0 },
	{"retinn",	RETIcond,	OP_RETIcond | C_NN,	0,	0 },
	{"retinv",	RETIcond,	OP_RETIcond | C_NV,	0,	0 },
	{"retiv",	RETIcond,	OP_RETIcond | C_V,	0,	0 },
	{"retinuf",	RETIcond,	OP_RETIcond | C_NUF,	0,	0 },
	{"retiuf",	RETIcond,	OP_RETIcond | C_UF,	0,	0 },
	{"retinc",	RETIcond,	OP_RETIcond | C_NC,	0,	0 },
	{"retic",	RETIcond,	OP_RETIcond | C_C,	0,	0 },
	{"retinlv",	RETIcond,	OP_RETIcond | C_NLV,	0,	0 },
	{"retilv",	RETIcond,	OP_RETIcond | C_LV,	0,	0 },
	{"retinluf",	RETIcond,	OP_RETIcond | C_NLUF,	0,	0 },
	{"retiluf",	RETIcond,	OP_RETIcond | C_LUF,	0,	0 },
	{"retizuf",	RETIcond,	OP_RETIcond | C_ZUF,	0,	0 },

	/* RETIcondD */
	{"retiud",	RETIcondD,	OP_RETIcondD | C_U,	0,	0 },
	{"retilod",	RETIcondD,	OP_RETIcondD | C_LO,	0,	0 },
	{"retilsd",	RETIcondD,	OP_RETIcondD | C_LS,	0,	0 },
	{"retihid",	RETIcondD,	OP_RETIcondD | C_HI,	0,	0 },
	{"retihsd",	RETIcondD,	OP_RETIcondD | C_HS,	0,	0 },
	{"retieqd",	RETIcondD,	OP_RETIcondD | C_EQ,	0,	0 },
	{"retined",	RETIcondD,	OP_RETIcondD | C_NE,	0,	0 },
	{"retiltd",	RETIcondD,	OP_RETIcondD | C_LT,	0,	0 },
	{"retiled",	RETIcondD,	OP_RETIcondD | C_LE,	0,	0 },
	{"retigtd",	RETIcondD,	OP_RETIcondD | C_GT,	0,	0 },
	{"retiged",	RETIcondD,	OP_RETIcondD | C_GE,	0,	0 },
	{"retizd",	RETIcondD,	OP_RETIcondD | C_Z,	0,	0 },
	{"retinzd",	RETIcondD,	OP_RETIcondD | C_NZ,	0,	0 },
	{"retipd",	RETIcondD,	OP_RETIcondD | C_P,	0,	0 },
	{"retind",	RETIcondD,	OP_RETIcondD | C_N,	0,	0 },
	{"retinnd",	RETIcondD,	OP_RETIcondD | C_NN,	0,	0 },
	{"retinvd",	RETIcondD,	OP_RETIcondD | C_NV,	0,	0 },
	{"retivd",	RETIcondD,	OP_RETIcondD | C_V,	0,	0 },
	{"retinufd",	RETIcondD,	OP_RETIcondD | C_NUF,	0,	0 },
	{"retiufd",	RETIcondD,	OP_RETIcondD | C_UF,	0,	0 },
	{"retincd",	RETIcondD,	OP_RETIcondD | C_NC,	0,	0 },
	{"reticd",	RETIcondD,	OP_RETIcondD | C_C,	0,	0 },
	{"retinlvd",	RETIcondD,	OP_RETIcondD | C_NLV,	0,	0 },
	{"retilvd",	RETIcondD,	OP_RETIcondD | C_LV,	0,	0 },
	{"retinlufd",	RETIcondD,	OP_RETIcondD | C_NLUF,	0,	0 },
	{"retilufd",	RETIcondD,	OP_RETIcondD | C_LUF,	0,	0 },
	{"retizufd",	RETIcondD,	OP_RETIcondD | C_ZUF,	0,	0 },

	/* RETScond  */
	{"retsu",	RETScond,	OP_RETScond | C_U,	0,	0 },
	{"retslo",	RETScond,	OP_RETScond | C_LO,	0,	0 },
	{"retsls",	RETScond,	OP_RETScond | C_LS,	0,	0 },
	{"retshi",	RETScond,	OP_RETScond | C_HI,	0,	0 },
	{"retshs",	RETScond,	OP_RETScond | C_HS,	0,	0 },
	{"retseq",	RETScond,	OP_RETScond | C_EQ,	0,	0 },
	{"retsne",	RETScond,	OP_RETScond | C_NE,	0,	0 },
	{"retslt",	RETScond,	OP_RETScond | C_LT,	0,	0 },
	{"retsle",	RETScond,	OP_RETScond | C_LE,	0,	0 },
	{"retsgt",	RETScond,	OP_RETScond | C_GT,	0,	0 },
	{"retsge",	RETScond,	OP_RETScond | C_GE,	0,	0 },
	{"retsz",	RETScond,	OP_RETScond | C_Z,	0,	0 },
	{"retsnz",	RETScond,	OP_RETScond | C_NZ,	0,	0 },
	{"retsp",	RETScond,	OP_RETScond | C_P,	0,	0 },
	{"retsn",	RETScond,	OP_RETScond | C_N,	0,	0 },
	{"retsnn",	RETScond,	OP_RETScond | C_NN,	0,	0 },
	{"retsnv",	RETScond,	OP_RETScond | C_NV,	0,	0 },
	{"retsv",	RETScond,	OP_RETScond | C_V,	0,	0 },
	{"retsnuf",	RETScond,	OP_RETScond | C_NUF,	0,	0 },
	{"retsuf",	RETScond,	OP_RETScond | C_UF,	0,	0 },
	{"retsnc",	RETScond,	OP_RETScond | C_NC,	0,	0 },
	{"retsc",	RETScond,	OP_RETScond | C_C,	0,	0 },
	{"retsnlv",	RETScond,	OP_RETScond | C_NLV,	0,	0 },
	{"retslv",	RETScond,	OP_RETScond | C_LV,	0,	0 },
	{"retsnluf",	RETScond,	OP_RETScond | C_NLUF,	0,	0 },
	{"retsluf",	RETScond,	OP_RETScond | C_LUF,	0,	0 },
	{"retszuf",	RETScond,	OP_RETScond | C_ZUF,	0,	0 },

	/* TRAPcond */
	{"trapu",	TRAPcond,	OP_TRAPcond | C_U,	0,	0 },
	{"traplo",	TRAPcond,	OP_TRAPcond | C_LO,	0,	0 },
	{"trapls",	TRAPcond,	OP_TRAPcond | C_LS,	0,	0 },
	{"traphi",	TRAPcond,	OP_TRAPcond | C_HI,	0,	0 },
	{"traphs",	TRAPcond,	OP_TRAPcond | C_HS,	0,	0 },
	{"trapeq",	TRAPcond,	OP_TRAPcond | C_EQ,	0,	0 },
	{"trapne",	TRAPcond,	OP_TRAPcond | C_NE,	0,	0 },
	{"traplt",	TRAPcond,	OP_TRAPcond | C_LT,	0,	0 },
	{"traple",	TRAPcond,	OP_TRAPcond | C_LE,	0,	0 },
	{"trapgt",	TRAPcond,	OP_TRAPcond | C_GT,	0,	0 },
	{"trapge",	TRAPcond,	OP_TRAPcond | C_GE,	0,	0 },
	{"trapz",	TRAPcond,	OP_TRAPcond | C_Z,	0,	0 },
	{"trapnz",	TRAPcond,	OP_TRAPcond | C_NZ,	0,	0 },
	{"trapp",	TRAPcond,	OP_TRAPcond | C_P,	0,	0 },
	{"trapn",	TRAPcond,	OP_TRAPcond | C_N,	0,	0 },
	{"trapnn",	TRAPcond,	OP_TRAPcond | C_NN,	0,	0 },
	{"trapnv",	TRAPcond,	OP_TRAPcond | C_NV,	0,	0 },
	{"trapv",	TRAPcond,	OP_TRAPcond | C_V,	0,	0 },
	{"trapnuf",	TRAPcond,	OP_TRAPcond | C_NUF,	0,	0 },
	{"trapuf",	TRAPcond,	OP_TRAPcond | C_UF,	0,	0 },
	{"trapnc",	TRAPcond,	OP_TRAPcond | C_NC,	0,	0 },
	{"trapc",	TRAPcond,	OP_TRAPcond | C_C,	0,	0 },
	{"trapnlv",	TRAPcond,	OP_TRAPcond | C_NLV,	0,	0 },
	{"traplv",	TRAPcond,	OP_TRAPcond | C_LV,	0,	0 },
	{"trapnluf",	TRAPcond,	OP_TRAPcond | C_NLUF,	0,	0 },
	{"trapluf",	TRAPcond,	OP_TRAPcond | C_LUF,	0,	0 },
	{"trapzuf",	TRAPcond,	OP_TRAPcond | C_ZUF,	0,	0 },


	/* INTERLOCK INSTRUCTIONS */
	/* ====================== */

	{"ldfi",	LDFI,		OP_LDFI,		0,	0 },
	{"ldii",	LDII,		OP_LDII,		0,	0 },
	{"sigi",	SIGI,		OP_SIGI,		0,	0 },
	{"stfi",	STFI,		OP_STFI,		0,	0 },
	{"stii",	STII,		OP_STII,		0,	0 },


	/* STACK INSTRUCTIONS */
	/* ================== */

	{"popf",	POPF,		OP_POPF,		0,	0 },
	{"pop",		POP,		OP_POP,			0,	0 },
	{"push",	PUSH,		OP_PUSH,		0,	0 },
	{"pushf",	PUSHF,		OP_PUSHF,		0,	0 },


	/* MISC INSTRUCTIONS */
	/* ================= */

	{"idle",	IDLE,		OP_IDLE,		0,	0 },
	{"swi",		SWI,		OP_SWI,			0,	0 },
	{"nop",		NOP,		OP_NOP,			0,	0 },
	{"iack",	IACK,		OP_IACK,		0,	0 },


	/* PARALLEL INSTRUCTIONS */
	/* ===================== */

	/* uses standard tokens defined for two and */
	/* triadic instructions */

	/* Possible combinations (instructions may be combined in either order)
	ABSF || STF,	ABSI || STI,	ADDF3 || STF,	ADDI3 || STI,
	AND3 || STI,	ASH3 || STI,	FIX || STI,	FLOAT || STF,
	FRIEEE || STF,	LDF || STF,	LDI || STI,	LSH3 || STI,
	MPYF3 || STF,	MPYI3 || STI,	NEGF || STF,	NEGI || STI,
	NOT || STI,	OR3 || STI,	SUBF3 || STF,	TOIEEE || STF,
	SUBI3 || STI,	XOR3 || STI,	LDF || LDF,	LDI || LDI,
	MPYF3 || ADDF3, MPYF3 || SUBF3,	MPYF3 || ADDI3,	MPYF3 || SUBI3
	*/

	{NULL, 0, 0}	/* **** END MARKER **** */
} ;



	/* ************************************************************ */
	/* C40 specific linker instruction patch tokens			*/
	/* ************************************************************ */

static Common TheInstrPatches[] =  {

	/* TEXT			TOKEN			VALUE	*/

	{"patchc40datamodule1",	PATCHC40DATAMODULE1,	PATCH_C40DATAMODULE1 },
	{"patchc40datamodule2",	PATCHC40DATAMODULE2,	PATCH_C40DATAMODULE2 },
	{"patchc40datamodule3",	PATCHC40DATAMODULE3,	PATCH_C40DATAMODULE3 },
	{"patchc40datamodule4",	PATCHC40DATAMODULE4,	PATCH_C40DATAMODULE4 },
	{"patchc40datamodule5",	PATCHC40DATAMODULE5,	PATCH_C40DATAMODULE5 },
	{"patchc40mask24add",	PATCHC40MASK24ADD,	PATCH_C40MASK24ADD   },
	{"patchc40mask16add",	PATCHC40MASK16ADD,	PATCH_C40MASK16ADD   },
	{"patchc40mask8add",	PATCHC40MASK8ADD,	PATCH_C40MASK8ADD    },

	{NULL, 0, 0}	/* **** END MARKER **** */
} ;



	/* ************************************************************ */
	/* C40 Register Tokens						*/
	/* ************************************************************ */

static Common TheRegs[] =  {
	/* TEXT		TOKEN		VALUE	*/

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
	{"ar0",		AR0,		R_AR0	},
	{"ar1",		AR1,		R_AR1	},
	{"ar2",		AR2,		R_AR2	},
	{"ar3",		AR3,		R_AR3	},
	{"ar4",		AR4,		R_AR4	},
	{"ar5",		AR5,		R_AR5	},
	{"ar6",		AR6,		R_AR6	},
	{"ar7",		AR7,		R_AR7	},
	{"dp",		DP,		R_DP	},
	{"ir0",		IR0,		R_IR0	},
	{"ir1",		IR1,		R_IR1	},
	{"bk",		BK,		R_BK	},
	{"sp",		SP,		R_SP	},
	{"st",		ST,		R_ST	},
	{"die",		DIE,		R_DIE	},
	{"iie",		IIE,		R_IIE	},
	{"iif",		IIF,		R_IIF	},
	{"rs",		RS,		R_RS	},
	{"re",		RE,		R_RE	},
	{"rc",		RC,		R_RC	},

	{"ivtp",	IVTP,		R_IVTP	},
	{"tvtp",	TVTP,		R_TVTP	},

	{NULL, 0, 0}	/* **** END MARKER **** */
} ;




/************************************************************************/
/* InitMnemonicKeywords							*/
/*									*/
/* Add a set of C40 specific mnemonic keywords to the symbol table	*/
/*									*/
/************************************************************************/

void InitMnemonicKeywords(void)
{
	struct Mnemonic		*cm = TheC40Mnemonics;
	struct Common		*regs = TheRegs;
	struct Common		*ip = TheInstrPatches;

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
}



/* end of lex_C40.c */
