/*------------------------------------------------------------------------
--                                                                      --
--           H E L I O S   N E T W O R K I N G   S O F T W A R E	--
--           ---------------------------------------------------	--
--                                                                      --
--             Copyright (C) 1991, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- netutils : count.c							--
--									--
--	Author:  BLV 19/11/91						--
--                                                                      --
------------------------------------------------------------------------*/
/* RcsId: $Header: /hsrc/network/RCS/nucount.c,v 1.3 1993/08/12 11:32:21 nickc Exp $*/

#include <helios.h>
#include <syslib.h>
#include <root.h>
#include <gsp.h>
#include <codes.h>
#include "netutils.h"

	/* Determine the number of links on a processor, using a	*/
	/* ServerInfo call.						*/
int	Util_CountLinks(Object *processor)
{ Object	*procman = Locate(processor, "tasks");
  BYTE		buffer[IOCDataMax];
  word		rc;

  if (procman == Null(Object))
   return(-1);
  rc = ServerInfo(procman, buffer);
  Close(procman);
  if (rc < Err_Null) return(-1);
  return (int)(((ProcStats *) buffer)->NLinks);
}

