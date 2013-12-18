/*------------------------------------------------------------------------
--                                                                      --
--           H E L I O S   N E T W O R K I N G   S O F T W A R E	--
--           ---------------------------------------------------	--
--                                                                      --
--             Copyright (C) 1990, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- window.c								--
--									--
--	Given a session, find the window corresponding to that session	--
--                                                                      --
--	Author:  BLV 9/7/90						--
--                                                                      --
------------------------------------------------------------------------*/
/* RcsId: $Header: /hsrc/network/RCS/slwindow.c,v 1.5 1993/08/12 12:27:53 nickc Exp $*/

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

/**
*** Make sure that the window is interactive
**/
bool	RmTestInteractiveSession(Object *Session)
{ if (Session eq Null(Object)) return(FALSE);
  if (Session->Type ne Type_Session) return(FALSE);
  return((Session->Flags & Flags_Interactive) ne 0);
}

/**
*** This routine would be called, typically, when walking down the directory
*** of current sessions. Every entry is Locate()'ed and this routine is
*** used on the resulting object. It gives a Stream for the login window.
*** Programs like write and wall are implemented using this.
**/
Stream	*RmGetWindow(Object *Session, WORD *error)
{ Stream	*SessionStream = Null(Stream);
  int		size;
  BYTE		*buffer = Null(BYTE);
  Stream	*result = Null(Stream);
  Object	*TargetWindow = Null(Object);
  WORD		junk;
  
  if (error eq Null(WORD)) error = &junk;
  
  if (Session eq Null(Object))
   { *error = EC_Error + EG_Invalid + EO_Session; return(Null(Stream)); }
  if (Session->Type ne Type_Session)
   { *error = EC_Error + EG_Invalid + EO_Session; return(Null(Stream)); }
  
  SessionStream = Open(Session, Null(char), O_ReadOnly);
  if (SessionStream eq Null(Stream)) 
   { *error = Result2(Session); return(Null(Stream)); }

  size = (int) GetFileSize(SessionStream);
  if ( (size <= 0) || (size > IOCDataMax + sizeof(Capability)))
   { *error = EC_Error + EG_WrongSize + EO_Stream; goto done; }

  buffer = (BYTE *) Malloc(size);
  if (buffer eq Null(BYTE)) 
   { *error = EC_Error + EG_NoMemory + EO_Message; goto done; }

  if (Read(SessionStream, buffer, size, -1) ne size) 
   { *error = EC_Error + EG_Broken + EO_Stream; goto done; }
  
  TargetWindow = NewObject(&(buffer[sizeof(Capability)]),
  			(Capability *) buffer);
  if (TargetWindow eq Null(Object))
   { *error = EC_Error + EG_Unknown + EO_Window; goto done; }

  result = Open(TargetWindow, Null(char), O_ReadOnly);
  if (result eq Null(Stream))
   *error = Result2(TargetWindow);
   
done:
  if (buffer ne Null(BYTE)) Free(buffer);
  if (TargetWindow ne Null(Object)) Close(TargetWindow);
  if (SessionStream ne Null(Stream)) Close(SessionStream);
  return(result);
}




