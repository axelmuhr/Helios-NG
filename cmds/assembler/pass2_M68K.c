/*
 * File:	pass2_M68K.c
 * Author:	P.A.Beskeen
 * Date:	June '93
 *
 * Description: This module implements pass two of the M68K assembler. The
 *		Pass2 function is passed the instruction structure that the
 *		first pass of the M68K assembler produced. It uses the
 *		WriteCodeByte/Short/Word() functions to pass the fully
 *		assembled instruction binary on to the object code specific
 *		formatter. This allows the second pass to be used with
 *		different object code formats.
 *
 * RCSId: $Id: pass2_M68K.c,v 1.1 1993/06/25 12:09:11 paul Exp $
 *
 * (C) Copyright 1993 Perihelion Software Ltd.
 * 
 * RCSLog: $Log: pass2_M68K.c,v $
 * Revision 1.1  1993/06/25  12:09:11  paul
 * Initial revision
 *
 *
 */

/* Include Files: */

#include "gasm.h"
#include "ghof.h"
#include <stdio.h>


/****************************************************************/
/* Pass2							*/
/*								*/
/* Execute M68K second pass on an instruction.			*/
/* The Instruction structure was created by the first pass M68K	*/
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


	switch (instr->combine) {

	/* Just register addressing so do nothing */
	case COM_REG:
		break;

#if 0
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
				GHOFEncode(PATCH_M68K_JP);

				/* Output the assembled instruction as the */
				/* patches data */
				GHOFEncode(opcode);

				/* The M68K JP patch automatically adjusts for */
				/* the pipeline and shifts to a word offset */
				/* value to a word pointer */

#if 0	/* @@@ implement codestubs for M68K in linker */
				/* Other machines may use labelrefs if they */
				/* have not implemented linker codestubs. */
				GHOFEncode(GHOF_CODESTUB);
				ObjWrite(s->name, strlen(s->name)+1);
#else
				/* M68K does not currently have codestubs */
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
#endif

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



/* end of pass2_M68K.c */
