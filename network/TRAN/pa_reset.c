/*------------------------------------------------------------------------
--                                                                      --
--           H E L I O S   N E T W O R K I N G   S O F T W A R E	--
--           ---------------------------------------------------	--
--                                                                      --
--             Copyright (C) 1990, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- pa_reset.c								--
--                                                                      --
--	Activate a Parsytec reset					--
--                                                                      --
--	Author:  BLV 10/8/90						--
--                                                                      --
------------------------------------------------------------------------*/

static char *rcsid = "$Header: /users/bart/hsrc/network/TRAN/RCS/pa_reset.c,v 1.2 1991/03/01 17:22:05 bart Exp $";

#include <stdio.h>
#include <syslib.h>
#include <task.h>
#include <codes.h>
#include <nonansi.h>
#include <string.h>
#include <root.h>

#pragma -s1		/* disable stack checking */
#pragma -f0		/* and vector stack */
#pragma -g0		/* and do not put the names into the code */

#ifndef eq
#define eq ==
#define ne !=
#endif

/**
*** Activating the Parsytec reset scheme is fun.
**/

#define	Reset_Address	0x000000C0

static void usage(Stream *);

int main(void)
{ Environ	env;
  uword		*reg;
  char		*number;
  int		link;
      
  if (GetEnv(MyTask->Port, &env) < Err_Null)
   { IOdebug("pa_reset: failed to receive environment");
     Exit(0x100);
   }

  if ((env.Strv[0] eq Null(Stream)) ||
      (env.Strv[1] eq Null(Stream)) ||
      (env.Strv[2] eq Null(Stream)))
   { IOdebug("pa_reset: failed to get error stream in environment");
     Exit(0x100);
   }
   
  if ((env.Argv[0] eq Null(char)) ||
      (env.Argv[1] eq Null(char)) ||
      (env.Argv[2] ne Null(char)))
   usage(env.Strv[2]);

  link = 0;
  for (number = env.Argv[1]; ; number++)
   { if (('0' <= *number) && (*number <= '9'))
      link = (10 * link) + (*number - '0');
     else
      break;
   }
  if (*number ne '\0') usage(env.Strv[2]);

  { RootStruct	*root = GetRoot();
    LinkInfo	**info = root->Links;
    int		i;
    for (i = 0; i <= link; i++)
     if (*info++ eq Null(LinkInfo))
      usage(env.Strv[2]);
  }      
  
  reg  = (uword *) Reset_Address;
  *reg = 0;
  *reg = 1;
  *reg = 2;
  *reg = 3;
  *reg = 1 << link;
  Delay(10000);	/* 10 Msec */
  *reg = 0;
  Exit(0);
}  

static void usage(Stream *s)
{ static char *message = "pa_reset: usage, pa_reset <0 | 1 | ...>\n";
  (void) Write(s, (BYTE *) message, strlen(message), -1);
  Exit(0x100);
}

