/*------------------------------------------------------------------------
--                                                                      --
--           H E L I O S   N E T W O R K I N G   S O F T W A R E	--
--           ---------------------------------------------------	--
--                                                                      --
--             Copyright (C) 1990, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- logname.c								--
--                                                                      --
--	Print out the result of the Posix getlogin() call.		--
--                                                                      --
--	Author:  BLV 21/8/90						--
--                                                                      --
------------------------------------------------------------------------*/
/* RcsId: $Header: /hsrc/network/RCS/logname.c,v 1.3 1993/08/11 10:31:20 bart Exp $*/

#include <stdio.h>
#include <posix.h>
#include <stdlib.h>

static void usage(void);

int main(int argc, char **argv)
{ char	*result;

  if (argc != 1) usage();
  argv = argv;
  
  result = getlogin();
  if (result == (char *) NULL)
   { fputs("logname: failed to determine login name.\n", stderr);
     exit(EXIT_FAILURE);
   }
  puts(result);
  return(EXIT_SUCCESS);
}

static void usage(void)
{ fputs("logname: usage, logname\n", stderr);
  exit(EXIT_FAILURE);
}

