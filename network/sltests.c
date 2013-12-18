/*------------------------------------------------------------------------
--                                                                      --
--           H E L I O S   N E T W O R K I N G   S O F T W A R E	--
--           ---------------------------------------------------	--
--                                                                      --
--             Copyright (C) 1990, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- tests.c								--
--									--
--	Code for the RmTestSessionManager()  etc. routines		--
--                                                                      --
--	Author:  BLV 9/7/90						--
--                                                                      --
------------------------------------------------------------------------*/
/* RcsId: $Header: /hsrc/network/RCS/sltests.c,v 1.5 1993/08/12 11:39:23 nickc Exp $*/

#include <stdio.h>
#include <string.h>
#include <syslib.h>
#include <gsp.h>
#include <nonansi.h>
#include <posix.h>

#include "session.h"

#ifdef Malloc		/* courtesy of servlib.h */
#undef Malloc
#endif

#ifndef eq
#define eq ==
#define ne !=
#endif

bool	RmTestSessionManager(void)
{ Object	*SM = Locate(Null(Object), "/sm");
  if (SM eq Null(Object)) return(FALSE);
  Close(SM);
  return(TRUE);
}

/**
*** Currently this simply locates the Session Manager. Eventually it
*** may try to get a Session Manager capability from the environment
*** to gain special access.
**/
Object	*RmGetSessionManager(void)
{ Object *result = Locate(Null(Object), "/sm");
  return(result);
}

bool	RmTestPasswordsRequired(void)
{ Object	*SM = Locate(Null(Object), "/sm");
  word		flags;
  
  if (SM eq Null(Object))
   return(FALSE);	/* Cannot verify passwords without a user database */
  flags = SM->Flags;
  Close(SM);
  return((flags & RmFlags_PasswordChecking) ne 0);
} 


