/*------------------------------------------------------------------------
--                                                                      --
--           H E L I O S   N E T W O R K I N G   S O F T W A R E	--
--           ---------------------------------------------------	--
--                                                                      --
--             Copyright (C) 1990, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- names.c								--
--									--
--	Code for getting the user and session names.			--
--                                                                      --
--	Author:  BLV 9/7/90						--
--                                                                      --
------------------------------------------------------------------------*/
/* RcsId: $Header: /hsrc/network/RCS/slnames.c,v 1.4 1993/08/11 10:50:01 bart Exp $*/

#include <string.h>
#include <syslib.h>
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
*** Get the real names for the current user and the session.
**/   
static char *myobjname(char *);

bool	RmGetNames(char *UserName, char *SessionName)
{ Environ	*Env = getenviron();
  Object	**Objv = Env->Objv;
  Object	*SupposedSession;
  Stream	*stream;
  Object	*SessionManager;
  Object	*RealSession;
  int		i;

	/* 1) Check that Objv exists */  
  if (Objv eq Null(Object *)) return(FALSE);
  
 	/* 2) validate the Object vector. It would be nice to be able	*/
 	/* to verify that the objects were real, because Env->Objv	*/
 	/* can be overwritten.						*/
  for (i = 0; i <= OV_Session; i++)
   if (Objv[i] eq Null(Object))
    return(FALSE);

	/* 3) I am now fairly confident that the Objv entry is real */
  SupposedSession = Objv[OV_Session];
  if (SupposedSession eq (Object *) MinInt) return(FALSE);
    
	/* 4) Validation can happen only if the Session Manager is running */
  SessionManager = Locate(Null(Object), "/sm");
  if (SessionManager eq Null(Object)) return(FALSE);

	/* 5) See if a suitable entry exists in the /sm directory */
  RealSession = Locate(SessionManager, myobjname(SupposedSession->Name));
  if (RealSession eq Null(Object))
   { Close(SessionManager); return(FALSE); }
  Close(SessionManager);	/* no longer needed */
     
 	/* 6) Zap the capability to be that of the supposed session */
  RealSession->Access = SupposedSession->Access;
  
	/* 7) Try to open the resulting object. This causes the */
	/* Session Manager to validate the capability		*/
  stream = Open(RealSession, Null(char), O_Private);
  Close(RealSession);
  if (stream eq Null(Stream)) return(FALSE);

	/* 8) all the validation has now been done. Extract the information */
  Close(stream);
  if (SessionName ne Null(char))
   strcpy(SessionName, myobjname(SupposedSession->Name));
  if (UserName ne Null(char))
   { char *tmp = UserName;
     strcpy(UserName, myobjname(SupposedSession->Name));
     for ( ; *tmp ne '\0'; tmp++)
      if (*tmp eq '.')
       *tmp = '\0';
   }
  return(TRUE);
/**
BLV Outstanding weaknesses are as follows:
BLV  1) the information returned by getenviron is dubious.
BLV  2) any user can produce another /sm server which does not do
BLV     do any checking, and overwrite the objv entry with /mysm/bart
BLV Unfortunately I think that this is the best I can do.
**/
}

/**
*** Internal version of the Server library's objname() routine,
*** because the program may not be linked with servlib.
**/
static char *myobjname(char *path)
{
	string p = path + strlen(path);
	
	until( *p == c_dirchar || p < path ) p--;

	return p+1;
}
