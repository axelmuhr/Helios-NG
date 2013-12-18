/*------------------------------------------------------------------------
--                                                                      --
--                      H E L I O S  C O M M A N D                      --
--                      --------------------------                      --
--                                                                      --
--             Copyright (C) 1992, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- idrom.c								--
--                                                                      --
-- Print information about TIM-40 modules IDROM.			--
--                                                                      --
-- The TIM-40 standards IDROM characterises the C40 module Helios is 	--
-- currently running on. If the module has no built-in IDROM a pseudo	--
-- one is constructed and sent by the I/O Server.			--
--									--
-- For more information see the TIM-40 specification.			--
--                                                                      --
--                                                                      --
-- This command will only work for the TIM-40 spec. CPU's.		--
--                                                                      --
-- Author: PAB 25/6/92							--
--                                                                      --
------------------------------------------------------------------------*/
/* RcsId: $Id: idrom.c,v 1.3 1993/07/12 11:58:44 nickc Exp $ */

#include <c40.h>
#include <stdio.h>
#include <stdlib.h>

/*---------------------------------------------------------------------------*/

int main (int argc, char **argv)
{
	IDROM	*ID_ROM = GetIDROM();

	printf("TIM-40 Module IDROM Contents\n");
	printf("----------------------------\n\n");

	printf("CPU type: %d (%s)", (int)ID_ROM->CPU_ID,
		ID_ROM->CPU_ID == 0? "TMS320C40" : "Unknown");
	printf(", Cycle time: %dns = %3.3fMhz.\n", (int)ID_ROM->CPU_CLK + 1,
		1000.0 / ((double)ID_ROM->CPU_CLK + 1) * 2.0);

	printf("Module manufacturer ID: %d", (int)ID_ROM->MAN_ID);
	printf(", module type: %d",
			ID_ROM->MODEL_NO);
	printf(", revision level: %d (%#x).\n",
			(int)ID_ROM->REV_LVL, (int)ID_ROM->REV_LVL);

	printf("\nAddress bus        :            Local                         Global\n");
	printf("Memory control reg :         %#8lx                     %#8lx\n",
		ID_ROM->LBCR, ID_ROM->GBCR);

	printf("Strobe             :   Strobe 0       Strobe 1       Strobe 0       Strobe 1\n");

	printf("Valid memory       :     %s            %s            %s            %s\n",
		ID_ROM->LBASE0 == -1 ? " No" : "Yes",
		ID_ROM->LBASE1 == -1 ? " No" : "Yes",
		ID_ROM->GBASE0 == -1 ? " No" : "Yes",
		ID_ROM->GBASE1 == -1 ? " No" : "Yes");

	printf("Size in words      : %#10lx     %#10lx     %#10lx     %#10lx\n",
	ID_ROM->LSIZE0, ID_ROM->LSIZE1, ID_ROM->GSIZE0, ID_ROM->GSIZE1);

	printf("Size in Mb's       : %10.2f     %10.2f     %10.2f     %10.2f\n",
	((float)ID_ROM->LSIZE0 * 4.0) / (1024.0 * 1024.0),
	((float)ID_ROM->LSIZE1 * 4.0) / (1024.0 * 1024.0),
	((float)ID_ROM->GSIZE0 * 4.0) / (1024.0 * 1024.0),
	((float)ID_ROM->GSIZE1 * 4.0) / (1024.0 * 1024.0));

	printf("Address base       : %#10lx     %#10lx     %#10lx     %#10lx\n",
		ID_ROM->LBASE0, ID_ROM->LBASE1, ID_ROM->GBASE0, ID_ROM->GBASE1);

	printf("Cycles within page :         %2d             %2d             %2d             %2d\n",
		(int)(ID_ROM->WAIT_L & 0xf), (int)(ID_ROM->WAIT_L >> 4 & 0xf),
		(int)(ID_ROM->WAIT_G & 0xf), (int)(ID_ROM->WAIT_G >> 4 & 0xf));

	printf("Cycles out of page :         %2d             %2d             %2d             %2d\n",
		(int)(ID_ROM->PWAIT_L & 0xf), (int)(ID_ROM->PWAIT_L >> 4 & 0xf),
		(int)(ID_ROM->PWAIT_G & 0xf), (int)(ID_ROM->PWAIT_G >> 4 & 0xf));

	printf("\nSize of fast RAM pool (includes on-chip RAM): %#lx words (%.2fKb).\n",
		ID_ROM->FSIZE, ((float)ID_ROM->FSIZE * 4.0) / 1024.0);

	printf("\nTimer 0 period value for accurate 1ms interval: %#lx, TCR value %#x.\n",
		ID_ROM->TIMER0_PERIOD, (int)ID_ROM->TIMER0_CTRL);
	printf("Timer 1 period value: %#lx, and TCR value for DRAM refresh %#x (optional).\n",
		ID_ROM->TIMER1_PERIOD, (int)ID_ROM->TIMER1_CTRL);


	printf("\nIDROM reserved byte: %x, self inclusive size of info block: %ld (should be %d).\n",
		(int)ID_ROM->RESERVED,
		ID_ROM->SIZE, sizeof(IDROM) / sizeof(word));

	printf("Total size in words of further IDROM auto-initialisation blocks: %ld.\n",
		ID_ROM->AINIT_SIZE);



}


/* end of idrom.c */
