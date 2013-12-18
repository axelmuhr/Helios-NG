/*------------------------------------------------------------------------
--                                                                      --
--      H E L I O S   P A R A L L E L   P R O G R A M M I N G		--
--	-----------------------------------------------------		--
--									--
--	  F A R M   C O M P U T A T I O N   L I B R A R Y		--
--	  -----------------------------------------------		--
--									--
--		Copyright (C) 1992, Perihelion Software Ltd.		--
--                        All Rights Reserved.                          --
--                                                                      --
-- testaux2.c								--
--		Part of the farm library test harness, together with	--
--	farmtest.c and testaux.c. This module contains a worker routine	--
--	which can be AccelerateCode'd.					--
--                                                                      --
--	Author:  BLV 28/10/92						--
--                                                                      --
------------------------------------------------------------------------*/
/* RcsId: $Id: testaux2.c,v 1.1 1992/10/30 19:00:31 bart Exp $ */

#include <stdio.h>
#include <stdlib.h>
#include <farmlib.h>

void fastcode_worker(void)
{ fprintf(stderr, "Worker %d : worker routine is at location %p\n",
		FmWorkerNumber, &fastcode_worker);
}
