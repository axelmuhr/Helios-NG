/*------------------------------------------------------------------------
--                                                                      --
--           H E L I O S   N E T W O R K I N G   S O F T W A R E	--
--           ---------------------------------------------------	--
--                                                                      --
--             Copyright (C) 1990, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- tty.c								--
--                                                                      --
--	Print details of the current standard input stream, if that	--
--	is a tty.							--
--                                                                      --
--	Author:  BLV 21/8/90						--
--                                                                      --
------------------------------------------------------------------------*/
/* RcsId: $Header: /hsrc/network/RCS/tty.c,v 1.3 1993/08/11 10:55:35 bart Exp $*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <helios.h>
#include <syslib.h>
#include <nonansi.h>

static void usage(void);

int main(int argc, char **argv)
{ Stream	*Stdin;
  bool		silent = FALSE;
  
  if ((argc < 1) || (argc > 2)) usage();
  if (argc == 2) 
   { if (strcmp(argv[1], "-s")) usage();
     silent = TRUE;
   }
   
  Stdin = Heliosno(stdin);
  if (Stdin->Flags & Flags_Interactive)
   { unless(silent) puts(Stdin->Name);
     return(0);
   }
  else
   { unless(silent) fputs("tty: stdin is not an interactive stream.\n", stderr);
     return(1);
   }
}

static void usage(void)
{ fputs("tty: usage, tty [-s]\n", stderr);
  exit(2);
}

