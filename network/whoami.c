/*------------------------------------------------------------------------
--                                                                      --
--           H E L I O S   N E T W O R K I N G   S O F T W A R E	--
--           ---------------------------------------------------	--
--                                                                      --
--             Copyright (C) 1990, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- whoami.c								--
--                                                                      --
--	Give details of the current session				--
--                                                                      --
--	Author:  BLV 13/7/90						--
--                                                                      --
------------------------------------------------------------------------*/
/* RcsId: $Header: /hsrc/network/RCS/whoami.c,v 1.3 1993/08/11 10:56:59 bart Exp $*/

#include <stdio.h>
#include <posix.h>
#include <string.h>
#include <helios.h>
#include <stdlib.h>
#include "session.h"

#define eq ==
#define ne !=

static void use_environment(void);

int main(void)
{ char		SessionName[NameMax];
  char		UserName[NameMax];

  unless(RmTestSessionManager())
   { fprintf(stderr, "whoami : warning, failed to find Session Manager\n");
     use_environment();
     return(EXIT_SUCCESS);
   }

  unless(RmGetNames(UserName, SessionName))
   { fprintf(stderr, "whoami : warning, failed to validate names.\n");
     use_environment();
     return(EXIT_SUCCESS);
   }

	/* Both are known so I can compare the two values. If they	*/
	/* are the same only the user name is printed. Otherwise the	*/
	/* user name is printed followed by the session name in brackets*/
  if (!strcmp(UserName, SessionName))
   puts(UserName);
  else
   printf("%s (%s)\n", UserName, SessionName);
  return(EXIT_SUCCESS);
}

static void use_environment(void)
{ char *SessionName	= getenv("SESSION");
  char *UserName	= getenv("USER");
  
	/* If neither the Session name nor the user name is known then	*/
	/* I am stuck.							*/  
  if ((SessionName eq Null(char)) && (UserName eq Null(char)))
   { fprintf(stderr, "whoami : I do not know who you are.\n");
     exit(EXIT_FAILURE);
   }

	/* If only the Session Name or only the user name is known then */
	/* that entry is printed.					*/
  if (SessionName eq Null(char))
   { printf("whoami : name is probably %s\n", UserName); 
     return;
   }

  if (UserName eq Null(char))
   { printf("whoami : name is probably %s\n",SessionName);
     return;
   }

	/* If both are known then I can compare the two values. If they */
	/* are the same only the user name is printed. Otherwise the	*/
	/* user name is printed followed by the session name in brackets*/
  if (!strcmp(UserName, SessionName))
   printf("whoami : name is probably %s\n", UserName);
  else
   printf(
   	"whoami : user name is probably %s, and session name is probably %s\n",
          UserName, SessionName);
  return;
}

