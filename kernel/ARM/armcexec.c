/*
 * File:	armcexec.c
 * Subsystem:	Generic Helios-ARM executive
 * Author:	P.A.Beskeen
 * Date:	Nov '91
 *
 * Description: Various ARM executive functions that are easier to implement
 *		in C.
 *
 *
 * RcsId: $Id: armcexec.c,v 1.1 1993/08/24 08:42:19 paul Exp $
 *
 * (C) Copyright 1992 Perihelion Software Ltd.
 * 
 * RcsLog: $Log: armcexec.c,v $
 * Revision 1.1  1993/08/24  08:42:19  paul
 * Initial revision
 *
 *
 *
 */

/* Include Files: */

#include "../kernel.h"


void ARMExecInit(void)
{
#ifdef SYSDEB
	/* tmp sanity check */
	if (((word)GetExecRoot()) + sizeof(ExecRoot) > (word)GetExecRoot()->Nucleus)
		KDebug("ARGGH ExecRoot is corrupting nucleus!");
#endif
	/* Initialise the trap table to point to the relevent functions */
	InitTrapTable();

	/* Initialise Exception vector to point at handlers (includes */
	/* IRQ and FIQ vectors) */
	InitExceptionVectors();

	/* Preload FIQ and IRQ banked registers */
	InitExceptionStacks();

	/* Do any Hardware specific initialisation */
	HWSpecificInit();
}



/* end of armcexec.c */
