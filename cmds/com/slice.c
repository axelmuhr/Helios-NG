/*{{{  Header */

/*------------------------------------------------------------------------
--                                                                      --
--                      H E L I O S  C O M M A N D                      --
--                      --------------------------                      --
--                                                                      --
--             Copyright (C) 1992 - 1994, Perihelion Software Ltd.      --
--                        All Rights Reserved.                          --
--                                                                      --
-- slice.c								--
--									--
-- Enable/Disable/Report timeslice state.				--
--									--
--                                                                      --
-- This command will only work for processors with the generic		--
-- executive.								--
--                                                                      --
-- Author: PAB 25/4/92							--
--                                                                      --
------------------------------------------------------------------------*/
/* RcsId: $Id: slice.c,v 1.6 1994/03/10 10:03:41 nickc Exp $ */

/*}}}*/
/*{{{  Includes */

#include "../../kernel/gexec.h"
#include <stdio.h>
#include <process.h>
#include <stdlib.h>

/*}}}*/
/*{{{  Code */

int
main (int argc, char **argv)
{
  while (*(++argv) != NULL)
    {
      if ((*argv)[0] == '-' && (*argv)[1] == 'h')
	{
	  printf( "use: slice [quantum <value>] [on/off]\n" );
	  return EXIT_SUCCESS;
	}
      
      if (strcmp(*argv, "quantum") == 0)
	{
	  int quan;
  
	  quan = atoi(*(++ argv));
	  SliceQuantum((int)OneSec/1000*quan);
	  return 0;
	}
      
      if (strcmp(*argv, "on") == 0)
	{
	  GetExecRoot()->SliceEnabled = TRUE;
	  printf("time slicing has been enabled\n");
	  return 0;
	}
      
      if (strcmp(*argv, "off") == 0)
	{
	  GetExecRoot()->SliceEnabled = FALSE;
	  printf("time slicing has been disabled\n");
	  return 0;
	}

      printf( "Unknown command line argument '%s' - ignored\n", *argv );
    }
  
  printf( "time slicing is %s with quantum %d\n",
	 (GetExecRoot()->SliceEnabled == TRUE) ? "enabled" : "disabled",
	 SliceQuantum(0)  );
  
  return 0;
}

/*}}}*/

/* end of slice.c */
