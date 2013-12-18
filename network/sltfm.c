/*------------------------------------------------------------------------
--                                                                      --
--           H E L I O S   N E T W O R K I N G   S O F T W A R E	--
--           ---------------------------------------------------	--
--                                                                      --
--             Copyright (C) 1990, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- tfm.c								--
--									--
--	Get the object corresponding to my Taskforce manager		--
--                                                                      --
--	Author:  BLV 9/7/90						--
--                                                                      --
------------------------------------------------------------------------*/
/* RcsId: $Header: /hsrc/network/RCS/sltfm.c,v 1.4 1993/08/11 10:50:43 bart Exp $*/

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

Object	*RmGetTfm(void)
{ Environ	*env	= getenviron();
  Object	**objv	= env->Objv;
  int		i;
  
  for (i = 0; i < OV_TFM; i++)
   if (objv[i] eq Null(Object))
    return(Null(Object));
  if (objv[OV_TFM] eq (Object *) MinInt)
   return(Null(Object));
  return(objv[OV_TFM]);
}

