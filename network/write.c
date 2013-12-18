/*------------------------------------------------------------------------
--                                                                      --
--           H E L I O S   N E T W O R K I N G   S O F T W A R E	--
--           ---------------------------------------------------	--
--                                                                      --
--             Copyright (C) 1990, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- write.c								--
--                                                                      --
--	Send a message to a particular user.				--
--                                                                      --
--	Author:  BLV 13/7/90						--
--                                                                      --
------------------------------------------------------------------------*/
/* RcsId: $Header: /hsrc/network/RCS/write.c,v 1.4 1993/08/11 10:57:13 bart Exp $*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "session.h"

#define eq ==
#define ne !=

static void usage(void);

int main(int argc, char **argv)
{ Object	*SessionManager = Null(Object);
  Object	*Session = Null(Object);
  Stream	*Window = Null(Stream);
  int		rc = EXIT_FAILURE;
  char		buffer[256];
  char		login_id[NameMax];    

  if (argc ne 2) usage();

  SessionManager = RmGetSessionManager();
  if (SessionManager eq Null(Object))
   { fprintf(stderr, "write : failed to locate session manager.\n");
     goto done;
   }

  unless(RmGetNames(Null(char), login_id))
   { fprintf(stderr, "write : unable to validate user name.\n");
     goto done;
   }
   
  Session = Locate(SessionManager, argv[1]);
  if (Session eq Null(Object))
   { fprintf(stderr, "write : user %s not logged in.\n", argv[1]);
     goto done;
   }
  Window = RmGetWindow(Session, Null(WORD));
  if (Window eq Null(Stream))
   { fprintf(stderr, "write : failed to access window for user %s\n", argv[1]);
     goto done;
   }
  if ((Window->Flags & Flags_Interactive) eq 0)
   { fprintf(stderr, "write : session for user %s is not interactive.\n",
   		argv[1]);
     goto done;
   }

  { char	*timebuf;
    time_t	now;
    now		= time(Null(time_t));
    timebuf	= ctime(&now);
    sprintf(buffer, "\r\nMessage from %s at %.5s ...\n", login_id, 
    		&(timebuf[11]));
    (void) Write(Window, buffer, strlen(buffer), -1);
  }

  while (fgets(buffer, 255, stdin) ne Null(char))
   { if (buffer[0] eq '!')
      system(&(buffer[1]));
     else
      (void) Write(Window, buffer, strlen(buffer), -1);
   }

  (void) Write(Window, "EOF\r\n", 5, -1);
  rc = EXIT_SUCCESS;
  
done:
  if (SessionManager ne Null(Object)) Close(SessionManager);
  if (Session ne Null(Object)) Close(Session);
  if (Window ne Null(Stream)) Close(Window);
  return(rc);
}

static void usage(void)
{ fprintf(stderr, "write : usage, write <username>.\n");
  exit(EXIT_FAILURE);
}
