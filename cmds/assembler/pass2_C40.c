/*
 * File:	pass2_C40.c
 * Author:	P.A.Beskeen
 * Date:	Sept '91
 *
 * Description: This module implements pass two of the 'C40 assembler. The
 *		Pass2 function is passed the instruction structure that the
 *		first pass of the 'C40 assembler produced. It uses the
 *		WriteCodeByte/Short/Word() functions to pass the fully
 *		assembled instruction binary on to the object code specific
 *		formatter. This allows the second pass to be used with
 *		different object code formats.
 *
 * RcsId: $Id: pass2_C40.c,v 1.7 1993/06/22 16:58:53 paul Exp $
 *
 * (C) Copyright 1991 Perihelion Software Ltd.
 * 
 * $RcsLog$
 *
 */

/* Include Files: */

#include "gasm.h"
#include "ghof.h"
#include <stdio.h>


/* print fatal error and exit */

void FatalNullExpr(void)
{
	Fatal("NULL indirect immediate displacement value");
}


/****************************************************************/
/* Pass2							*/
/*								*/
/* Execute 'C40 second pass on an instruction.			*/
/* The Instruction structure was created by the first pass 'C40	*/
/* specific parser. This parser noted where instructions had	*/
/* to have delayed evaluation expression operands evaluated,	*/
/* what limit checks to perform and where to insert the		*/
/* resulting value into the binary instruction.			*/
/* optexpr and combine define the first operand that the parser	*/
/* saw, and optexpr2 and combine2 hold any second operand.	*/
/*								*/
/****************************************************************/

/* @@@ can probably coalesce the processing of the indirect 1/0	*/
/* style combine commands */

void Pass2(Instruction *instr, int pc)
{
	Expression	*expr = instr->optexpr;
	int		opcode = instr->opcode;
	int		result = 0;

	if (instr->combine == 0) {
		/* no post processesing required */
		/* pass opcode straight to the object formatter */
		WriteCodeWord(opcode);
		return;
	}

	switch (instr->combine) {

	/* diadic combine commands					*/
	/* All these commands will only use combine, NOT combine2	*/

	/* Register addressing doesn't need to be checked		*/
	case COM_DIA_REG:
		/* do nothing */
		break;

	/* Check signed immediate <= 16 bits and insert int bits 0-15	*/
	case COM_DIA_IMM:
		result = Eval(expr, pc);
		if (!fits_in_16_bits(result)) {
			char Err[80];

			sprintf(Err, "signed immediate value (%d) will not fit in 16 bits", result);
			Error(Err);
		}
		opcode |= result & 0xffff;
		break;

	/* Check UNsigned immediate <= 16 bits and insert int bits 0-15	*/
	case COM_DIA_IMM_UNSIGNED:
		result = Eval(expr, pc);
		if (!fits_in_16_bits_unsigned(result)) {
			char Err[80];

			sprintf(Err, "unsigned immediate value (%#x) will not fit in 16 bits", result);
			Error(Err);
		}
		opcode |= result;
		break;

	/* Shift immediate >> 16 and insert int bits 0-15		*/
	case COM_DIA_IMM_LDP:
		opcode |= (Eval(expr, pc) >> 16) & 0xffff;
		break;

	/* Check unsigned disp <= 8 bits and insert into bits 0-7	*/
	case COM_DIA_IND:
		result = Eval(expr, pc);
		if (!fits_in_8_bits_unsigned(result)) {
			char Err[80];

			sprintf(Err, "unsigned indirect displacement (%d) is too large to be held in 8 bits", result);
			Error(Err);
		}

		opcode |= result;
		break;

	/* Check direct unsigned offset <= 16 bits and insert into bits 0-15 */
	case COM_DIA_DIR:
		result = Eval(expr, pc);
		if (!fits_in_16_bits_unsigned(result)) {
			char Err[80];

			sprintf(Err, "unsigned direct offset of (%d) cannot be held in 16 bits", result);
			Error(Err);
		}
		opcode |= result;
		break;

	/* Check signed immediate is <= 5 bits and insert into bits 16-20 */
	case COM_STIK_IMM:
		result = Eval(expr, pc);
		if (!fits_in_5_bits(result)) {
			char Err[80];

			sprintf(Err, "STIK signed immediate value (%d) cannot be held in 5 bits", result);
			Error(Err);
		}
		opcode |= (result & B_11111) << 16;
		break;


	/* triadic combine commands					*/
	/* Notes arg position by: _A1 = bits 0-7, _A2 = bits 8-15	*/

	/* Triadic type 1 indirect addressing, args 1 and 2		*/
	/* If instr->optexpr is NULL then there is no need to check the	*/
	/* displacement as it will be an index reg.			*/
	/* Check displacement is only 1 or 0.				*/
	/* If displacement is 0, then convert mod to 0b11000:		*/
	/*	(*+ARn(0) -> *ARn)					*/

	case COM_TRI_IND1_A1:		/* mod = bits 3-7 */
		if (expr != NULL) {
			if((result = Eval(expr, pc)) > 1 || result < 0)
				Error("indirect displacement value should only be 1 or 0");

			if (result == 0)
				/* convert to a *ARn type indirection preserving ARn */
				opcode = (opcode & ~B_11111000) | B_11000000;
		}
		break;

	case COM_TRI_IND1_A2:		/* mod = bits 11-15 */
		if (expr != NULL) {
			if((result = Eval(expr, pc)) > 1 || result < 0)
				Error("indirect displacement value should only be 1 or 0");

			if (result == 0)
				/* convert to a *ARn type indirection preserving ARn */
				opcode = (opcode & ~(B_11111000 << 8)) | (B_11000000 << 8);
		}
		break;

	/* Triadic type 2 indirect addressing, args 1 and 2		*/
	/* Check unsigned displacement is <= 5 bits and insert disp. 	*/
	/* into bits 11-15 or 3-7					*/

	case COM_TRI_IND2_A1:		/* bits 3-7 */
		if (expr != NULL) {
			result = Eval(expr, pc);
			if (!fits_in_5_bits_unsigned(result)) {
				char Err[80];

				sprintf(Err, "indirect displacement value (%d) cannot be held in 5 bits", result);
				Error(Err);
			}

#if 0	/* @@@ opt syntax #4 - would also have to convert T field */
			if (result == 0) {
				/* convert to a type 1, *ARn type indirection */
				/* preserving ARn - opt syntax #4 */
				opcode = ((opcode & ~TRI_TYPE_2) & ~B_11111000) | B_11000000;
			}
			else
#endif
				/* insert 5bit displacement into mod field */
				opcode = (opcode & ~B_11111000) | (result << 3);
		}
		else
			FatalNullExpr();
		break;
	
	case COM_TRI_IND2_A2:		/* bits 11-15 */
		if (expr != NULL) {
			result = Eval(expr, pc);
			if (!fits_in_5_bits_unsigned(result)) {
				char Err[80];

				sprintf(Err, "unsigned indirect displacement value (%d) cannot be held in 5 bits", result);
				Error(Err);
			}

#if 0	/* @@@ opt syntax #4 - would also have to convert T field */
			if (result == 0) {
				/* convert to a type 1, *ARn type indirection */
				/* preserving ARn - opt syntax #4 */
				opcode = ((opcode & ~TRI_TYPE_2) & ~(B_11111000 << 8)) | (B_11000000 << 8);
			}
			else
#endif
				opcode = (opcode & ~(B_11111000 << 8)) | (result << 11);
		}
		else
			FatalNullExpr();
		break;
	
	/* Triadic type 2 immediate addressing, arg 1 only		*/
	/* Check immediate <= eight bits and insert into bits 0-7 of	*/
	/* instruction.							*/

	case COM_TRI_IMM_A1:
		if (expr != NULL) {
			result = Eval(expr, pc);
			if (!fits_in_8_bits(result)) {
				char Err[80];

				sprintf(Err, "signed immediate value (%d) cannot be held in 8 bits", result);
				Error(Err);
			}
			opcode |= (result & 0xff);
		}
		break;

	case COM_TRI_IMM_A1_UNSIGNED:
		if (expr != NULL) {
			result = Eval(expr, pc);
			if (!fits_in_8_bits_unsigned(result)) {
				char Err[80];

				sprintf(Err, "unsigned immediate value (%d) cannot be held in 8 bits", result);
				Error(Err);
			}
			opcode |= result;
		}
		break;
	
	/* parallel store combine commands				*/
	/* Check displacement is only 1 or 0, If displacement is 0,	*/
	/* then convert mod to 0b11000 (*+ARn(0) -> *ARn)		*/

	case COM_PST_IND_S1:	/* 1st src operand: mod = bits 3-7	*/
		if (expr != NULL) {
			if((result = Eval(expr, pc)) > 1 || result < 0)
				Error("indirect displacement value should only be 1 or 0");

			if (result == 0)
				/* convert to a *ARn type indirection preserving ARn */
				opcode = (opcode & ~B_11111000) | B_11000000;
		}
		break;

	case COM_PST_IND_D2:	/* Store dest operand: mod = bits 11-15	*/
		if (expr != NULL) {
			if((result = Eval(expr, pc)) > 1 || result < 0)
				Error("indirect displacement value should only be 1 or 0");

			if (result == 0)
				/* convert to a *ARn type indirection preserving ARn */
				opcode = (opcode & ~(B_11111000 << 8)) | (B_11000000 << 8);
		}
		break;

	/* parallel multiply add/sub combine commands			*/
	/* Check displacement is only 1 or 0, If displacement is 0, 	*/
	/* then convert mod to 0b11000 (*+ARn(0) -> *ARn)		*/

	case COM_PMPY_IND1:	/* mod bits 11-15 */
		if (expr != NULL) {
			if((result = Eval(expr, pc)) > 1 || result < 0)
				Error("indirect displacement value should only be 1 or 0");

			if (result == 0)
				/* convert to a *ARn type indirection preserving ARn */
				opcode = (opcode & ~(B_11111000 << 8)) | (B_11000000 << 8);
		}
		break;

	case COM_PMPY_IND2:	/* mod bits 3-7 */
		if (expr != NULL) {
			if((result = Eval(expr, pc)) > 1 || result < 0)
				Error("indirect displacement value should only be 1 or 0");

			if (result == 0)
				/* convert to a *ARn type indirection preserving ARn */
				opcode = (opcode & ~B_11111000) | B_11000000;
		}
		break;


	/* branch combine commands					*/

	/* If operation is a delayed branch then subtract 3 from the	*/
	/* offset before inserting it into the instruction, otherwise	*/
	/* subtract 1. These combine commands will only ever occur in	*/
	/* the first ->combine						*/

	/* Check PC relative offset <= 16 bits and insert into 		*/
	/* bits 0-15. Used in conditional branch instructions.		*/
	/* Bit 21 specifies if it is a delayed branch.			*/

	case COM_CBR_PCREL:	/* default for pcrel mode */
		/* pipeline adjustment for normal or delayed branch */
		if ( opcode & (1 << 21))
			result = -3;
		else
			result = -1;

		if (expr->what == E_Str) { /* possible imported label ? */
			Symbol *s = FindSymb(expr->type.name);

			if (s != NULL && s->what == HT_IMPORT) {
				/* output instruction with codestub patch */

				FlushCodeBuffer();

				/* assume word sized instruction */
				GHOFEncode(GHOF_WORD);

				/* output the machine specific patch */
				GHOFEncode(PATCH_C40MASK16ADD);

				/* output the assembled instruction as the */
				/* patches data */
				opcode |= result & 0xffff;
				GHOFEncode(opcode);

				/* For the C40 we have to shift the labelref */
				/* value to a word pointer */
				GHOFEncode(PATCH_SHIFT);
				GHOFEncode(-2);

				/* Other machines may use labelrefs if they */
				/* have not implemented linker codestubs */
				GHOFEncode(GHOF_CODESTUB);
				ObjWrite(s->name, strlen(s->name)+1);

				return;
			}
		}

		result += Eval(expr, pc);

		if (!fits_in_16_bits(result)) {
			char Err[80];

			sprintf(Err,"pc relative offset (%#x) is larger than can be held in 16 bits", result);
			Error(Err);
		}

		opcode |= result & 0xffff;
		break;

	/* Check PC relative offset < 24 bits and insert into bits 0-23 */
	/* Used in UNconditional branch instructions and RPTB(D) */
	/* Bit 24 specifies if it is a delayed branch */
	case COM_BR_PCREL:
		if ( opcode & (1 << 24))
			result = -3;
		else
			result = -1;

		if (expr->what == E_Str) { /* possible imported label ? */
		/* pipeline adjustment for normal or delayed branch */
			Symbol *s = FindSymb(expr->type.name);

			if (s != NULL && s->what == HT_IMPORT) {
				/* output instruction with codestub patch */

				FlushCodeBuffer();

				/* assume word sized instruction */
				GHOFEncode(GHOF_WORD);

				/* output the machine specific patch */
				GHOFEncode(PATCH_C40MASK24ADD);

				/* output the assembled instruction as the */
				/* patches data */
				opcode |= result & 0xffffff;
				GHOFEncode(opcode);

				/* For the C40 we have to shift the labelref */
				/* value to a word pointer */
				GHOFEncode(PATCH_SHIFT);
				GHOFEncode(-2);

				/* Other machines may use labelrefs if they */
				/* have not implemented linker codestubs */
				GHOFEncode(GHOF_CODESTUB);
				ObjWrite(s->name, strlen(s->name)+1);

				return;
			}
		}

		result += Eval(expr, pc);

		if (!fits_in_24_bits(result)) {
			char Err[80];

			sprintf(Err, "pc relative offset (%#x) is larger than can be held in 24 bits", result);
			Error(Err);
		}

		opcode |= result & 0x00ffffff;
		break;

	/* Check immediate value is <= 9 bits and insert into bits 0-8 */
	case COM_TRAP:
		if ((result = Eval(expr, pc)) & ~(B_111111111))
			Error("cannot have a trap vector higher than 511");

		opcode |= result;
		break;

	default:
		{
			char Err[80];

			sprintf(Err, "Unknown combine 1 command (%d) in second pass", \
				instr->combine);
			Fatal(Err);
			break;
		}
	}


	if (instr->combine2 == 0) {
		/* no second operand to post process */
		/* pass opcode on to the object formatter */
		WriteCodeWord(opcode);
		return;
	}

	/* now check for combine commands on a second operand */
	expr = instr->optexpr2;

	switch (instr->combine2) {

	/* triadic combine commands					*/
	/* Notes arg position by: _A1 = bits 0-7, _A2 = bits 8-15	*/

	/* Triadic type 1 indirect addressing, args 1 and 2		*/
	/* Check displacement is only 1 or 0.				*/
	/* If displacement is 0, then convert mod to 0b11000:		*/
	/*	(*+ARn(0) -> *ARn)					*/

	case COM_TRI_IND1_A1:		/* mod = bits 3-7 */
		if (expr != NULL) {
			if((result = Eval(expr, pc)) > 1 || result < 0)
				Error("indirect displacement value should only be 1 or 0");

			if (result == 0)
				/* convert to a *ARn type indirection preserving ARn */
				opcode = (opcode & ~B_11111000) | B_11000000;
		}
		break;

	case COM_TRI_IND1_A2:		/* mod = bits 11-15 */
		if (expr != NULL) {
			if ((result = Eval(expr, pc)) > 1 || result < 0)
				Error("indirect displacement value should only be 1 or 0");

			if (result == 0)
				/* convert to a *ARn type indirection preserving ARn */
				opcode = (opcode & ~(B_11111000 << 8)) | (B_11000000 << 8);
		}
		break;

	/* Triadic type 2 indirect addressing, args 1 and 2		*/
	/* Check displacement is <= 5 bits and insert disp. into	*/
	/* bits 11-15 or 3-7						*/

	case COM_TRI_IND2_A1:		/* bits 3-7 */
		if (expr != NULL) {
			result = Eval(expr, pc);
			if (!fits_in_5_bits_unsigned(result)) {
				char Err[80];

				sprintf(Err, "indirect displacement value (%d) cannot be held in 5 bits", result);
				Error(Err);
			}

#if 0	/* would also have to convert T field */
			if (result == 0) {
				/* convert to a type 1, *ARn type indirection */
				/* preserving ARn - opt syntax #4 */
				opcode = ((opcode & ~TRI_TYPE_2) & ~B_11111000) | B_11000000;
			}
			else
#endif
				opcode = (opcode & ~B_11111000) | (result << 3);
		}
		else
			FatalNullExpr();
		break;

	case COM_TRI_IND2_A2:		/* bits 11-15 */
		if (expr != NULL) {
			result = Eval(expr, pc);
			if (!fits_in_5_bits_unsigned(result)) {
				char Err[80];

				sprintf(Err, "indirect displacement value (%d) will not fit into 5 bits", result);
				Error(Err);
			}

#if 0	/* would also have to convert T field */
			if (result == 0) {
				/* convert to a type 1, *ARn type indirection */
				/* preserving ARn - opt syntax #4 */
				opcode = ((opcode & ~TRI_TYPE_2) & ~(B_11111000 << 8)) | (B_11000000 << 8);
			}
			else
#endif
				opcode = (opcode & ~(B_11111000 << 8)) | (result << 11);
		}
		else
			FatalNullExpr();
		break;


	/* parallel store combine commands				*/
	/* Check displacement is only 1 or 0, If displacement is 0,	*/
	/* then convert mod to 0b11000 (*+ARn(0) -> *ARn)		*/

	case COM_PST_IND_S1:	/* 1st src operand: mod = bits 3-7	*/
		if (expr != NULL) {
			if((result = Eval(expr, pc)) > 1 || result < 0)
				Error("indirect displacement value should only be 1 or 0");

			if (result == 0)
				/* convert to a *ARn type indirection preserving ARn */
				opcode = (opcode & ~B_11111000) | B_11000000;
		}
		break;

	case COM_PST_IND_D2:	/* Store dest operand: mod = bits 11-15	*/
		if (expr != NULL) {
			if((result = Eval(expr, pc)) > 1 || result < 0)
				Error("indirect displacement value should only be 1 or 0");

			if (result == 0)
				/* convert to a *ARn type indirection preserving ARn */
				opcode = (opcode & ~(B_11111000 << 8)) | (B_11000000 << 8);
		}
		break;

	/* parallel multiply add/sub combine commands			*/
	/* Check displacement is only 1 or 0, If displacement is 0, 	*/
	/* then convert mod to 0b11000 (*+ARn(0) -> *ARn)		*/

	case COM_PMPY_IND1:	/* mod bits 11-15 */
		if (expr != NULL) {
			if((result = Eval(expr, pc)) > 1 || result < 0)
				Error("indirect displacement value should only be 1 or 0");

			if (result == 0)
				/* convert to a *ARn type indirection preserving ARn */
				opcode = (opcode & ~B_11111000) | B_11000000;
		}
		break;

	case COM_PMPY_IND2:	/* mod bits 3-7 */
		if (expr != NULL) {
			if((result = Eval(expr, pc)) > 1 || result < 0)
				Error("indirect displacement value should only be 1 or 0");

			if (result == 0)
				/* convert to a *ARn type indirection preserving ARn */
				opcode = (opcode & ~(B_11111000 << 8)) | (B_11000000 << 8);
		}
		break;

	/* Following two commands should only get into second combine */
	/* if used with a STIK instruction */
	/* Check unsigned disp <= 8 bits and insert into bits 0-7	*/
	case COM_DIA_IND:
		result = Eval(expr, pc);
		if (!fits_in_8_bits_unsigned(result)) {
			char Err[80];

			sprintf(Err, "indirect displacement of (%d) will not fit into 8 bits", result);
			Error(Err);
		}

		opcode |= result;
		break;


	/* Check direct unsigned offset <= 16 bits and insert into bits 0-15 */
	case COM_DIA_DIR:
		result = Eval(expr, pc);
		if (!fits_in_16_bits_unsigned(result)) {
			char Err[128];
			sprintf(Err, "direct offset of (%d) cannot be held in 16 bits", result);
			Error(Err);
		}
		opcode |= result;
		break;

	default:
		{
			char Err[80];

			sprintf(Err, "Unknown combine 2 command (%d) in second pass", \
				instr->combine2);
			Fatal(Err);
			break;
		}
	}

	/* pass opcode on to the object formatter */
	WriteCodeWord(opcode);
}


/*
-- Floating point conversion routines
--
-- crf: October 1992
--
-- @@@ 
-- Note: Conversion from higher- to lower-precision representations 
-- (e.g. C40_32 --> C40_16) does not currently perform any rounding.
-- The sequence of conversion in getting to a short (16 bit) representation
-- is :
--        (round)		      (truncate)
-- IEEE_64  --->  IEEE_32  --->  C40_32  --->  C40_16
-- The above rounding and truncating should preferably be swapped around :
--       (truncate)		       (round)
-- IEEE_64  --->  IEEE_32  --->  C40_32  --->  C40_16
-- Whether this is really necessary is debatable ...
-- @@@ 
*/

/* extract components */

#define EXTRACT_C40_32			\
	e = (val >> 24) & 0xff ;	\
	s = (val >> 23) & 1 ;		\
	f = val & ((1 << 23) - 1)

#define EXTRACT_IEEE_32			\
	e = (val >> 23) & 0xff ;	\
	s = (val >> 31) & 1 ;		\
	f = val & ((1 << 23) - 1)

/* build number from components */

#define BUILD_C40_32	\
	((e << 24) | (s << 23) | (f & ((1 << 23) - 1)))
	
#define BUILD_C40_16	\
	((e << 12) | (s << 11) | (f & ((1 << 11) - 1))) & 0x0000ffff

/*
-- 32 bit C40 --> 16 bit C40
*/

int32 C40_32ToC40_16 (int32 val)
{
	int32 e ;
	int32 f ;
	int32 s ;

	EXTRACT_C40_32 ;

/*
-- If the C40_32 value passed in is zero, have to catch it here and return
-- the short (C40_16) reprentation
-- 0 (C40_32): e = -128, s = 0, f = 0
-- 0 (C40_16): e = -8, s = 0, f = 0
*/
	if (e == 0x80 && s == 0 && f == 0)
	{
		e = 0x8 ;
		return BUILD_C40_16 ;
	}

	if ((int8) e > 7)
		Fatal ("value too large to represent in short floating point format") ;

	if ((int8) e < -7)
	{
		Warn ("small floating point value converted to 0.0") ;
		e = 0x8 ; s = 0 ; f = 0 ;
		return BUILD_C40_16 ;
	}

	e = e & 0x0f ;
	f = f >> 12 ;
	return BUILD_C40_16 ;
}

/*
-- 32 bit IEEE --> 32 bit C40
*/

#ifdef HOST_SUPPORTS_IEEE
int32 IEEE_32ToC40_32 (float flt)
#else
int32 IEEE_32ToC40_32 (int32 val)
#endif
{
	int32 e ;
	int32 f ;
	int32 s ;

#ifdef HOST_SUPPORTS_IEEE
	int32 val ;
	union {
		int32 w ;
		float f ;
	} fw ;

	fw.f = flt ;
	val = fw.w ;
#endif

	EXTRACT_IEEE_32 ;

	/* decode - this algorithm is taken from the TMS320C4x User's Guide, page 4-12 */
	
	if (e == 0xff)
	{
		if (s == 0)
		{
			e = 0x7f ;
			/* s = 0 ; */
			f = 0x7fffff ;
		}
		else
		{
			e = 0x7f ;
			/* s = 1 ; */
			f = 0 ;
		}
	}
	else if (e == 0)
	{
		e = 0x80 ;
		s = 0 ;
		f = 0 ;
	}
	else
	{
		if (s == 0)
			e -= 0x7f ; /* s = 0 ; f = f ; */
		else
		{
			if (f == 0)
				e -= 0x80 ; /* s = 1 ; f = 0 ; */
			else
			{
				e -= 0x7f ;
				/* s = 1 ; */
				f = ((~f) + 1) ;
			}
		}
	}
	return BUILD_C40_32 ;
}

/*
-- 64 bit IEEE --> 32 bit C40
-- @@@
-- Note: Conversion from 64 to 32 bit representation involves rounding - this
-- may or may not be desirable (refer earlier comments on conversion from
-- IEEE 64 --> C40 16)
-- @@@
*/

#ifndef HOST_SUPPORTS_IEEE
#define IEEE_64ToIEEE_32(d,f)	\
	(void) fltrep_narrow_round (d,f)
#endif

int32 IEEE_64ToC40_32 (Dble d)
{
#ifdef HOST_SUPPORTS_IEEE
	return IEEE_32ToC40_32 ((float) d) ;
#else
	FloatBin f ;
	IEEE_64ToIEEE_32 (&d, &f) ;
	return (IEEE_32ToC40_32 (f.val)) ;
#endif
}

/*
-- 64 bit IEEE --> 16 bit C40
*/

int32 IEEE_64ToC40_16 (Dble d)
{
#ifdef HOST_SUPPORTS_IEEE
	int32 C40_32 = IEEE_32ToC40_32 ((float) d) ;
	return (C40_32ToC40_16 (C40_32)) ;
#else
	FloatBin f ;
	int32 C40_32 ;
	IEEE_64ToIEEE_32 (&d, &f) ;
	C40_32 = IEEE_32ToC40_32 (f.val) ;
	return (C40_32ToC40_16 (C40_32)) ;
#endif
}

#if 0 /* Available, not currently used */

/* extract components */

#define EXTRACT_C40_16			\
	e = (val >> 12) & 0xf ;		\
	s = (val >> 11) & 1 ; 		\
	f = val & ((1 << 11) - 1)

/* build number from components */

#define BUILD_IEEE_32	\
	((s << 31) | ((e & 0xff) << 23) | (f & ((1 << 23) - 1))) ;


/*
-- 16 bit C40 --> 32 bit C40
*/

int32 C40_16ToC40_32 (int32 val)
{
	int32 e ;
	int32 f ;
	int32 s ;

	if (val & 0xffff0000)
		Fatal ("invalid short floating point representation") ;

	EXTRACT_C40_16 ;

/*
-- If the C40_16 value passed in is zero, have to catch it here and return
-- the short (C40_32) reprentation
-- 0 (C40_32): e = -128, s = 0, f = 0
-- 0 (C40_16): e = -8, s = 0, f = 0
*/

	if (e == 0x8 && s == 0 && f == 0)
	{
		e = 0x80 ; /* s = 0 ; f = 0 */
		return BUILD_C40_32 ;
	}

	if (e & 0x8)
		e |= 0xf0 ; /* sign extend */
	f = f << 12 ;
	return BUILD_C40_32 ;
}

/*
-- 32 bit C40 --> 32 bit IEEE
*/

#ifdef HOST_SUPPORTS_IEEE
float C40_32ToIEEE_32 (int32 val)
#else
int32 C40_32ToIEEE_32 (int32 val)
#endif
{
	int32 e ;
	int32 f ;
	int32 s ;

#ifdef HOST_SUPPORTS_IEEE
	union {
		int32 w ;
		float f ;
	} fw ;
#endif

	EXTRACT_C40_32 ;

	/* decode - this algorithm is taken from the TMS320C4x User's Guide, page 4-13 */

	switch (e)
	{
		case 0x80:		/* e = -128 */
		case 0x81:		/* e = -127 */
			e = 0 ;
			s = 0 ;
			f = 0 ;
			break ;
		case 0x7f:		/* e = 127 */
			if (s == 1 && f == 0)
			{
				e = 0xff ; /* s = 1 ; f = 0 ; */
				break ;
			}
			/* otherwise, continue ... */
		default:		/* -126 <= e <= 127 */
			if (s == 0)
				e += 0x7f ; /* s = 0 ; f = f ; */
			else
			{
				if (f != 0)
				{
					e += 0x7f ;
/*
-- Note: printing error in TMS320C4x User's Guide, page 4-13 ! (s = 0)
*/
					/* s = 1 ; */
					f = ((~f) + 1) ;
				}
				else /* s == 1, f == 0 */
/*
-- Note: printing error in TMS320C4x User's Guide, page 4-13 ! (e += 0x7e)
*/
					e += 0x80 ; /* s = 1 ; f = 0 ; */
			}
			break ;
	}
#ifdef HOST_SUPPORTS_IEEE
	fw.w = BUILD_IEEE_32 ;
	return fw.f ;
#else
	return BUILD_IEEE_32 ;
#endif
}

#endif /* Available, not currently used */



/* pass2_C40.c */

