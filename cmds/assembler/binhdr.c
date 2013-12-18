/*
 * File:	binhdr.c
 * Author:	Author P.A.Beskeen (idea stolen from NickC)
 * Date:	Sept '91
 *
 * Description: Automatically generate a Handy set of binary definitions
 *
 *		Helps to get around C's lack of binary constants
 *
 * RcsId: $Id: binhdr.c,v 1.1 1992/03/12 21:16:01 paul Exp $
 *
 * (C) Copyright 1991 Perihelion Software Ltd.
 * 
 * $RcsLog$
 *
 */

#include <stdio.h>
#include <stdlib.h>

#include "itoabin.c"	/* stops confusion over host and native .o's */

char *preamble =
"/*\n * File:	binary.h\n"
" * Author:	A. Program (*Author P.A.Beskeen (idea stolen from NickC!))\n"
" * Date:	Originally Sept '91\n"
" *\n"
" * Description: Handy set of binary definitions.\n"
" *\n"
" *		Helps to get around C's lack of binary constants.\n"
" *		This file was automatically generated - so believe it!\n"
" *\n"
" * RcsId: $Id: binhdr.c,v 1.1 1992/03/12 21:16:01 paul Exp $\n"
" *\n"
" * (C) Copyright 1991 Perihelion Software Ltd.\n"
" * \n"
" * $RcsLog$\n"
" *\n"
" */\n"
"\n"
"\n"
"#ifndef __binary_h\n"
"#define __binary_h\n"
"\n"
"\n"
"/* Binary Constants: */\n";

static char *postamble =
"\n"
"#endif\n"
"\n"
"/* binary.h */";


int main(void)
{
	int i, bits, n = 0;
	int cols_todo[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, /*10, */ 11, /* 16 */ 0};

	puts(preamble);

	while ((bits = cols_todo[n++]) != 0) {
		if (bits == 1)
			printf("\n/* Bit 0: */\n");
		else
			printf("\n/* Bits %d-0: */\n", bits-1);

		for (i = 0; i < (1 << bits); i++)
			printf("#define B_%s	%#x\n", \
				itoabin(NULL, bits, i), i);
	}

	puts(postamble);

	return 0;
}


/* end of binhdr.c */
