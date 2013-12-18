/*------------------------------------------------------------------------
--                                                                      --
--           H E L I O S   N E T W O R K I N G   S O F T W A R E	--
--           ---------------------------------------------------	--
--                                                                      --
--             Copyright (C) 1990, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- tr_reset.c								--
--                                                                      --
--	Activate a tram subsystem reset					--
--                                                                      --
--	Author:  BLV 10/8/90						--
--                                                                      --
------------------------------------------------------------------------*/

static char *rcsid = "$Header: /users/bart/hsrc/network/TRAN/RCS/tr_reset.c,v 1.2 1991/03/01 17:22:28 bart Exp $";

#include <stdio.h>
#include <syslib.h>
#include <task.h>
#include <nonansi.h>

#pragma -s1		/* disable stack checking */
#pragma -f0		/* and vector stack */
#pragma -g0		/* and do not put the names into the code */

/**
*** The reset performed is actually a full analyse rather than a simple
*** reset, just in case somebody wants to debug the target processor.
**/
#define		Subsystem_Reset		0x00000000L
#define		Subsystem_Analyse 	0x00000004L
#define		Subsystem_Error		0x00000000L

int main(void)
{ Environ	env;
  uword		*reg;
      
  (void) GetEnv(MyTask->Port, &env);
  
	/* Step 1 : force analyse low, to get into a known state */
  reg = (uword *) Subsystem_Analyse;
  *reg = 0;
  Delay(10000);	/* 10 Msec */
  
 	/* Step 2 : force analyse high, to start the reset */
  *reg = 1;
  Delay(10000);
  
 	/* Step 3 : assert the reset */
  reg = (uword *) Subsystem_Reset;
  *reg = 1;
  Delay(10000);
  
  	/* Step 4 : release the reset */
  *reg = 0;
  Delay(10000);
  
 	/* Step 5 : release the analyse */
  reg = (uword *) Subsystem_Analyse;
  *reg = 0;

  Exit(0);
}  
