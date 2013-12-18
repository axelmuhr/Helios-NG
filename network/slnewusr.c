/*------------------------------------------------------------------------
--                                                                      --
--           H E L I O S   N E T W O R K I N G   S O F T W A R E	--
--           ---------------------------------------------------	--
--                                                                      --
--             Copyright (C) 1990, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- Newuser.c								--
--									--
--	Code for the RmRegisterWindow() routine				--
--                                                                      --
--	Author:  BLV 9/7/90						--
--                                                                      --
------------------------------------------------------------------------*/
/* RcsId: $Header: /hsrc/network/RCS/slnewusr.c,v 1.5 1993/08/12 11:39:33 nickc Exp $*/

#include <string.h>
#include <syslib.h>
#include <gsp.h>
#include <nonansi.h>
#include <posix.h>
#include "session.h"
#include "exports.h"

#ifdef Malloc		/* courtesy of servlib.h */
#undef Malloc
#endif

#ifndef eq
#define eq ==
#define ne !=
#endif

bool	RmRegisterWindow(Object *WindowServer, Stream *Window, char *UserName,
			 WORD *error)
{ Object	*SessionManager = Null(Object);
  RmWindowInfo	*Info		= Null(RmWindowInfo);
  Object	*NewSession	= Null(Object);  
  bool		rc		= FALSE;
  int		size;
  char		WindowName[NameMax];
  WORD		junk;
  
  if (error eq Null(WORD)) error = &junk;
  
  SessionManager	= Locate(Null(Object), "/sm/Windows");
  if (SessionManager eq Null(Object))
   { *error = EC_Error + EG_Unknown + EO_Server; return(FALSE); }

	/* Create a suitable window name, maintaining the Helios limit */
	/* of 31 characters. For example, if the window is /pc/window/console */
	/* then the directory entry created is pc.window.console */
  if (strlen(Window->Name) >= NameMax)
   { int	offset = strlen(Window->Name) - NameMax;
     char	*temp  = &(Window->Name[offset]);
     for ( ; (*temp ne '/') && (*temp ne '\0'); temp++);
     if (*temp eq '\0')
      { *error = EC_Error + EG_WrongSize + EO_Name; goto done; }
     strcpy(WindowName, ++temp);      		
   }
  else
   strcpy(WindowName, &(Window->Name[1]));
  { char *temp; 
    for (temp = WindowName; *temp ne '\0'; temp++)
     if (*temp eq '/')
      *temp = '.';
   }
   
  size = 0;
  if (UserName ne Null(char))
   size = size + strlen(UserName) + 1;
  size = size + strlen(Window->Name)  + 1;
  size = size + sizeof(RmWindowInfo);
  if (WindowServer ne (Object *) MinInt)
   size += strlen(WindowServer->Name) + 1;
   
  if ((Info = (RmWindowInfo *) Malloc(size)) eq Null(RmWindowInfo))
   { *error = EC_Error + EG_NoMemory + EO_Message; goto done; }
   
  Info->Flags		= Window->Flags;
  Info->Pos		= Window->Pos;
  Info->WindowCap	= Window->Access;
  { BYTE	*temp = (BYTE *) Info;
    temp	= &(temp[sizeof(RmWindowInfo)]);
    if (UserName eq Null(char))
     Info->UserName = -1;
    else
     { strcpy(temp, UserName);
       Info->UserName = (word) temp - (word) &(Info->UserName);
       temp = &(temp[strlen(UserName) + 1]);
     }
    Info->WindowName  = (word) temp - (word) &(Info->WindowName);
    strcpy(temp, Window->Name);

    if (WindowServer eq (Object *) MinInt)
     Info->WindowServerName = MinInt;
    else
     { temp = &(temp[strlen(Window->Name) + 1]);
       Info->WindowServerName = (word) temp - (word) &(Info->WindowServerName);
       strcpy(temp, WindowServer->Name);
       Info->WindowServerCap  = WindowServer->Access;
     }
  }

  NewSession = Create(SessionManager, WindowName, Type_Device, size,
  			(BYTE *) Info);
    			
  if (NewSession eq Null(Object))
   { *error = Result2(SessionManager); goto done; }

  rc = TRUE;
  
done:   
  if (NewSession ne Null(Object))	Close(NewSession);
  if (SessionManager ne Null(Object))	Close(SessionManager);
  if (Info ne Null(RmWindowInfo))	Free(Info);
  return(rc);    
}
