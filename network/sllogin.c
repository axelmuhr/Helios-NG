/*------------------------------------------------------------------------
--                                                                      --
--           H E L I O S   N E T W O R K I N G   S O F T W A R E	--
--           ---------------------------------------------------	--
--                                                                      --
--             Copyright (C) 1990, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- login.c								--
--									--
--	Code for creating new sessions.					--
--                                                                      --
--	Author:  BLV 9/7/90						--
--                                                                      --
------------------------------------------------------------------------*/
/* RcsId: $Header: /hsrc/network/RCS/sllogin.c,v 1.5 1993/08/12 11:39:14 nickc Exp $*/

#include <stdio.h>
#include <string.h>
#include <syslib.h>
#include <gsp.h>
#include <nonansi.h>
#include "exports.h"
#include "session.h"

#ifdef Malloc		/* courtesy of servlib.h */
#undef Malloc
#endif

#ifndef eq
#define eq ==
#define ne !=
#endif

/**
*** RmCreateSession(), verify that the name and dest seem reasonable,
*** and check that there is a Session Manager in the system. Work
*** out how big the RmLoginInfo structure should be (it contains the
*** window name which is variable length), allocate it and fill it in.
*** Try to create the Session, and return whatever is the result.
**/

Object	*RmCreateSession(char *name, char *password, Stream *dest, WORD *error)
{ RmLoginInfo	*info = Null(RmLoginInfo);
  int		size;
  Object	*result = Null(Object);
  Object	*SessionManager = Null(Object);
  WORD		junk;
  
  if (error eq Null(WORD)) error = &junk;
    
  if (dest eq Null(Stream))
   { *error = EC_Error + EG_Invalid + EO_Stream; return(Null(Object)); }
  if (name eq Null(char))
   { *error = EC_Error + EG_Invalid + EO_Name; return(Null(Object)); }
 
  SessionManager = Locate(Null(Object), "/sm");
  if (SessionManager eq Null(Object))
   { *error = EC_Error + EG_Unknown + EO_Server; return(Null(Object)); }
   
  size = sizeof(RmLoginInfo) + strlen(dest->Name);
  info = (RmLoginInfo *) Malloc(size);
  if (info eq Null(RmLoginInfo)) 
   { *error = EC_Error + EG_NoMemory + EO_Message; goto done; }
  strncpy(info->UserName, name, NameMax - 1);
  info->UserName[NameMax - 1] = '\0';
  if (password eq Null(char))
   info->Password[0] = '\0';
  else
   { strncpy(info->Password, password, NameMax - 1);
     info->Password[NameMax - 1] = '\0';
   }
  info->Cap = dest->Access;
  strcpy(info->WindowName, dest->Name);

  result = Create(SessionManager, name, Type_Session, size, (BYTE *) info);

  if (result eq Null(Object))
   *error = Result2(SessionManager);
  else
   *error = Err_Null;
   
done:
  if (info ne Null(RmLoginInfo)) Free(info);
  if (SessionManager ne Null(Object)) Close(SessionManager);
  return(result);
}

/**
*** Once a Session has been created it is necessary to get hold of all the
*** boring bits of information such as capabilities for the Session's
*** Taskforce Manager and current directory. This is done by Open()'ing
*** the Session in private mode and reading an appropriate amount of data.
*** A few objects have to be initialised.
**/

bool	RmGetCapabilities(Object *session, RmSessionInfo *info, WORD *error)
{ Stream	*SessionStream = Null(Stream);
  int		size;
  RmSessionAux	*buffer = Null(RmSessionAux);
  bool		result = FALSE;
  char		*name;
  WORD		junk;

  if (error eq Null(WORD)) error = &junk;
  
  if (session eq Null(Object))
   { *error = EC_Error + EG_Invalid + EO_Session; return(FALSE); }
  if (session->Type ne Type_Session)
   { *error = EC_Error + EG_Invalid + EO_Session; return(FALSE); }
  if (info eq Null(RmSessionInfo)) 
   { *error = EC_Error + EG_Invalid + EO_Memory; return(FALSE); }
      
  info->TFM = info->CurrentDirectory = info->Program = Null(Object);

  SessionStream = Open(session, Null(char), O_Private);  
  if (SessionStream eq Null(Stream))
   { *error = Result2(session); goto done; }
  
  size = (int) GetFileSize(SessionStream);

  if ((size <= 0) || (size > (sizeof(RmSessionAux) + IOCDataMax)))
   { *error = EC_Error + EG_WrongSize + EO_Session; goto done; }
  
  buffer = (RmSessionAux *) Malloc(size);
  if (buffer eq Null(RmSessionAux)) 
   { *error = EC_Error + EG_NoMemory + EO_Message; goto done; }

  if (Read(SessionStream, (BYTE *) buffer, size, -1) < size) 
   { *error = EC_Error + EG_Broken + EO_Stream; goto done; }
  
  strcpy(info->UserName, buffer->UserName);
  info->Uid		= buffer->Uid;
  info->Gid		= buffer->Gid;
  strcpy(info->Comment, buffer->Comment);

	/* There may not be a taskforce manager, depending on the	*/
	/* nsrc option							*/
  if (buffer->TFMName ne 0)
   { name = (char *) (buffer->TFMName + (int) ((BYTE *) &(buffer->TFMName)));
     info->TFM = NewObject(name, &(buffer->TFMCap));
     if (info->TFM eq Null(Object))
      { *error = EC_Error + EG_Unknown + EO_TFM; goto done; }
   }
  else
   info->TFM = Null(Object);
   
  name = (char *)
         (buffer->CurrentDirName + (int) ((BYTE *) &(buffer->CurrentDirName)));
  info->CurrentDirectory = NewObject(name, &(buffer->CurrentDirCap));
  if (info->CurrentDirectory eq Null(Object))
   { *error = EC_Error + EG_Unknown + EO_Directory; goto done; }
   
  name = (char *) (buffer->ProgramName + (int) ((BYTE *) &(buffer->ProgramName)));
  info->Program = NewObject(name, &(buffer->ProgramCap));
  if (info->Program eq Null(Object))
   { *error = EC_Error + EG_Unknown + EO_Program; goto done; }

  result = TRUE;
  *error = Err_Null;   
   
done:
  if (SessionStream ne Null(Stream)) Close(SessionStream);
  if (buffer ne Null(RmSessionAux)) Free(buffer);
  unless(result)
   { if (info->TFM ne Null(Object)) Close(info->TFM);
     if (info->CurrentDirectory ne Null(Object)) Close(info->CurrentDirectory);
     if (info->Program ne Null(Object)) Close(info->Program);
   }

  return(result); 
}

/**
*** AbortSession is equivalent to Delete()
**/
void RmAbortSession(Object *Session)
{ Delete(Session, Null(char));
}

