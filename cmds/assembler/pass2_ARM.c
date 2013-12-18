/*
 * File:	pass2_ARM.c
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
 * RcsId: $Id: pass2_ARM.c,v 1.5 1993/06/22 16:58:53 paul Exp $
 *
 * (C) Copyright 1992 Paul Beskeen
 * 
 * $RcsLog$
 *
 */

/* Include Files: */

#include "gasm.h"
#include "ghof.h"
#include <stdio.h>


/****************************************************************/
/* Pass2							*/
/*								*/
/* Execute ARM second pass on an instruction.			*/
/* The Instruction structure was created by the first pass ARM	*/
/* specific parser. This parser noted where instructions had	*/
/* to have delayed evaluation expression operands evaluated,	*/
/* what limit checks to perform and where to insert the		*/
/* resulting value into the binary instruction.			*/
/*								*/
/****************************************************************/

void Pass2(Instruction *instr, int pc)
{
	Expression	*expr = instr->optexpr;
	int		opcode = instr->opcode;
	int		result = 0;


	/* if instruction has a coprocessor number associated with it */
	if (instr->optcoprocnum != NULL ) {
		if ((result = Eval(instr->optcoprocnum, pc)) >= 16 ||
		     result < 0 ) {
			char Err[80];

			sprintf(Err, "coprocessor number of %#x is not valid (must be 0-15)", result);
			Error(Err);
		} else
			opcode |= CP_CPNUM(result);
	}

	/* if instruction has a coprocessor aux field expr associated with it */
	if (instr->optcoprocaux != NULL ) {
		if ((result = Eval(instr->optcoprocaux, pc)) >= 8 ||
		     result < 0 ) {
			char Err[80];

			sprintf(Err, "coprocessor aux field of %#x is not valid (must be 0-7)", result);
			Error(Err);
		} else
			opcode |= CP_AUX(result);
	}

	switch (instr->combine) {

	/* Just register addressing so do nothing */
	case COM_REG:
		break;

	/* Check 24bit pc-relative and insert int bits 0-23		*/
	case COM_PCREL_24:
		if (expr->what == E_Str) { /* possible imported label ? */
			Symbol *s = FindSymb(expr->type.name);

			if (s != NULL && s->what == HT_IMPORT) {
				/* Output instruction with codestub/labelref */
				/* patch */

				FlushCodeBuffer();

				/* Assume word sized instruction */
				GHOFEncode(GHOF_WORD);

				/* Output the machine specific patch */
				GHOFEncode(PATCH_ARM_JP);

				/* Output the assembled instruction as the */
				/* patches data */
				GHOFEncode(opcode);

				/* The ARM JP patch automatically adjusts for */
				/* the pipeline and shifts to a word offset */
				/* value to a word pointer */

#if 0	/* @@@ implement codestubs for ARM in linker */
				/* Other machines may use labelrefs if they */
				/* have not implemented linker codestubs. */
				GHOFEncode(GHOF_CODESTUB);
				ObjWrite(s->name, strlen(s->name)+1);
#else
				/* ARM does not currently have codestubs */
				/* implemented in the linker. */
				GHOFEncode(GHOF_LABELREF);
				ObjWrite(s->name, strlen(s->name)+1);
#endif
				return;
			}
		}

		/* Normal label/expression */

		if ((result = Eval(expr, pc)) & 3)
			Error("branch to non word aligned address requested");

		/* Adjust for pipeline */
		result = (result >> 2) - 2;

		if (!fits_in_24_bits(result)) {
			char Err[80];

			sprintf(Err, "pc-relative branch of %#x will not fit into 24 bits", result);
			Error(Err);
		}

		opcode |= result & 0xffffff;
		break;

	/* Check immediate can be encoded into 8 bits with a 4*2 bit	*/
	/* rotate and insert into bits 0-7 + 8-11			*/
	case COM_ROTIMM_8:
	{
		int	rot = 0, rotmask = 0xff;
		unsigned val = Eval(expr, pc);

		if (val == 0)
			/* immediate of zero = 0 rotate with immediate of 0 */
			break;

		for (rot = 0; rot < 32; rot += 2) {
			if ((val & ~rotmask) == 0) {
				opcode |= DP_IMMROT(rot / 2) | \
					ROTATER(val, 32 - rot);
				break;
			}
			rotmask = ROTATER(rotmask, 2);
		}

		if (rot >= 32) {
			char Err[128];
	
			sprintf(Err, "immediate value of %d will not decompose "
			"into an eight bit value rotated by up to 30 bits "
			"(evens only)", val);
			Error(Err);
		}

		break;
	}

	/* After evaluation the constant should have eight subtracted from it */
	/* (to take account of the pipeline). Check this value can be encoded */
	/* into 8 bits with a 4*2 bit rotate and insert into bits 0-7 + 8-11 */
	case COM_LEAROTIMM_8:
	{
		int	rot = 0, rotmask = 0xff;
		int	val;
		unsigned uval;

		val = Eval(expr, pc) - 8;

		if (val == 0)
			/* immediate of zero = 0 rotate with immediate of 0 */
			break;

		if (val < 0) {
			/* convert add to a sub */
			opcode &= ~(B_1111 << 21);	/* mask out DP opcode */
			opcode |= OP_SUB;

			uval = (unsigned) -val;	/* abs val */
		} else
			uval = (unsigned) val;


		for (rot = 0; rot < 32; rot += 2) {
			if ((uval & ~rotmask) == 0) {
				opcode |= DP_IMMROT(rot / 2) | \
					ROTATER(uval, 32 - rot);
				break;
			}
			rotmask = ROTATER(rotmask, 2);
		}

		if (rot >= 32) {
			char Err[128];
	
			sprintf(Err, 
			 "The effective address (pcrel offset %#x) requested "
			 "is not accessable by a single add or sub opcode",
			 val);
			Error(Err);
		}

		break;
	}

	/* Check unsigned shift value < 31/32 (converting shift type	*/
	/* if required) and insert into bits 7-11			*/
	case COM_SHIFT:
		if ((result = Eval(expr, pc)) > 32 || result < 0) {
			char Err[80];

			sprintf(Err, "shift immediate value of %d is out of range", result);
			Error(Err);
		}

		if ((result == 32) &&
			( ((opcode & SHIFT_TYPE_MASK) == OP_LSL) ||
			  ((opcode & SHIFT_TYPE_MASK) == OP_ROR) ) ) {
			char Err[80];

			sprintf(Err, "LSL/ROR shifts cannot be shifted by 32");
			Error(Err);
		}

		if (result == 0)
			/* all rotates of 0 are encoded as LSL #0 */
			/* RRX doesn't set COM_SHIFT so wont be converted */
			opcode &= SHIFT_TYPE_MASK;	/* code for LSL = 0 */

		if (result == 32 && (
			((opcode & SHIFT_TYPE_MASK) == OP_ASR) ||
			((opcode & SHIFT_TYPE_MASK) == OP_LSR) )
		)
			/* ASR/LSR #32 encoded as ASR/LSR #0 */
			result = 0;

		opcode |= SHIFT_IMM(result);
		break;

	/* Check if fits into 12 bits unsigned and insert into bits 0-11 */
	/* If positive bit 23 U should be set, if neg unset and abs result */
	/* If base reg is PC Adjust for pipeline as value is pc relative */
	/* Used in data processing instructions for immediate offsets */
	case COM_INDEX_12:
		if ( EXTRACT_BASEREG(opcode) == R_R15) {
			if ((result = (Eval(expr, pc) - 8)) > 4095 || result < -4095) {
				char Err[80];

				sprintf(Err, "12 bit PC relative offset of %#x is out of range", result);
				Error(Err);
			}
		} else {
			if ((result = Eval(expr, pc)) > 4095 || result < -4095) {
				char Err[80];

				sprintf(Err, "offset of %d is out of range", result);
				Error(Err);
			}
		}

		if (result < 0)
			result = -result;	/* sub offset from base */
		else
			opcode |= INDEXUP;	/* add offset to base */
	
		opcode |= result;
		break;

	/* Check if fits into 8 bits unsigned and insert into bits 0-7 */
	/* If positive bit 23 U should be set, if neg unset and abs result */
	/* If base reg is pc adjust for pipelined pc relative value	*/
	case COM_INDEX_8:
		if ( EXTRACT_BASEREG(opcode) == R_R15) {
			if ((result = (Eval(expr, pc) - 8)) > 1023 || result < -1023) {
				char Err[80];

				sprintf(Err, "coprocessor PC relative offset of %#x is out of range (+/- 1023)", result);
				Error(Err);
			}
		} else {
			if ((result = Eval(expr, pc)) > 1023 || result < -1023) {
				char Err[90];

				sprintf(Err, "coprocessor indirect addressing offset of %d is out of range (+/- 1023)", result);
				Error(Err);
			}
		}

		if (result & 3) {
			char Err[90];

			sprintf(Err, "coprocessor indirect addressing offset must be a word multiple");
			Error(Err);
		}

		if (result < 0)
			result = -result;	/* sub offset from base */
		else
			opcode |= INDEXUP;	/* add offset to base */
	
		opcode |= (result >> 2);
		break;

	/* Check expression fits in 24 bits and insert into bits 0-23	*/
	case COM_SWI:
		result = Eval(expr, pc);
		if (!fits_in_24_bits(result)) {
			char Err[80];

			sprintf(Err, "SWI value of %#x will not fit into 24 bits", result);
			Error(Err);
		}
		opcode |= result & 0xffffff;
		break;


	/* Check that coprocessor opcode will fit in 4 bits.		*/
	/* Insert into bits 20-23.					*/
	case COM_CP_OPc:
		if ((result = Eval(expr, pc)) >= 16 || result < 0 ) {
			char Err[80];

			sprintf(Err, "coprocessor data operation of %#x is not valid (0-15)", result);
			Error(Err);
		}
		opcode |= result << 20;
		break;

	/* Check that coprocessor opcode will fit in 4 bits.		*/
	/* Insert into bits 21-23.					*/
	case COM_CP_OPc2:
		if ((result = Eval(expr, pc)) >= 8 || result < 0 ) {
			char Err[80];

			sprintf(Err, "coprocessor register transfer opcode of %#x is not valid (0-8)", result);
			Error(Err);
		}
		opcode |= result << 21;
		break;

	default:
		{
			char Err[80];

			sprintf(Err, "(internal) Unknown combine command (%d) in second pass", \
				instr->combine);
			Fatal(Err);
			break;
		}
	}

	/* pass opcode on to the object formatter */
	WriteCodeWord(opcode);
}



/* end of pass2_ARM.c */

