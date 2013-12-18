/*------------------------------------------------------------------------
--                                                                      --
--           H E L I O S   N E T W O R K I N G   S O F T W A R E	--
--           ---------------------------------------------------	--
--                                                                      --
--             Copyright (C) 1990, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- findtfm.c								--
--	Locate the Taskforce Manager for a specific user		--
--                                                                      --
--	Author:  BLV 2/9/90						--
--                                                                      --
------------------------------------------------------------------------*/
/* RcsId: $Header: /hsrc/network/RCS/findtfm.c,v 1.3 1993/08/11 10:30:01 bart Exp $*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <helios.h>
#include <syslib.h>
#include "session.h"

#ifndef eq
#define eq ==
#define ne !=
#endif

static char TfmName[IOCDataMax];

static void usage(void);

int main(int argc, char **argv)
{ Object	*tfm;
  char		*name;

 	/* without arguments, simply print out the name of my own tfm */
  if (argc eq 1)
   { 
     tfm = RmGetTfm();     
     if (tfm eq Null(Object))
      { fputs("findtfm: failed to locate own Taskforce Manager.\n", stderr);
        exit(EXIT_FAILURE);
      }
      
     strcpy(TfmName, tfm->Name);
     name = TfmName + strlen(TfmName);
     while (*name ne '/') name--;
     *name = '\0';
     puts(TfmName);
     return(EXIT_SUCCESS);
   }
  
  if (argc > 2) usage();

  if (*(argv[1]) eq '/')
   tfm = Locate(Null(Object), argv[1]);
  else
   { strcpy(TfmName, "/");
     strcat(TfmName, argv[1]);   
     tfm = Locate(Null(Object), TfmName);
   }

  if (tfm eq Null(Object))
   { fprintf(stderr, "findtfm: failed to locate %s\n", argv[1]);
     exit(EXIT_FAILURE);
   }

	/* Make sure that the resulting object really is a Taskforce Manager */
  { Object	*domain = Locate(tfm, "domain");
    if (domain eq Null(Object))
     { fprintf(stderr, "findtfm: object %s is not a Taskforce Manager\n",
     		tfm->Name);
       exit(EXIT_FAILURE);
     }
    Close(domain);
  }

  puts(tfm->Name);
  return(EXIT_SUCCESS);
}

static void usage(void)
{ fputs("findtfm: usage, findtfm <username>\n", stderr);
  exit(EXIT_FAILURE);
}

