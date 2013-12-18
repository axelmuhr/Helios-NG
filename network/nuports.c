/*------------------------------------------------------------------------
--                                                                      --
--           H E L I O S   N E T W O R K I N G   S O F T W A R E	--
--           ---------------------------------------------------	--
--                                                                      --
--             Copyright (C) 1990, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- netutils : nuports.c							--
--									--
--	Author:  BLV 10/4/92						--
--                                                                      --
------------------------------------------------------------------------*/
/* RcsId: $Header: /hsrc/network/RCS/nuports.c,v 1.2 1993/08/11 10:39:51 bart Exp $*/

#include <helios.h>
#include <task.h>
#include <syslib.h>
#include <queue.h>
#include <memory.h>
#include <codes.h>

/**
*** Code similar to that in nuheap.c, but for keeping track of message
*** ports rather than memory.
**/

#ifdef __TRAN
static	WordFnPtr	real_NewPort;
static	WordFnPtr	real_FreePort;

static word	my_NewPort()
{ word result = ((real_NewPort)());
  IOdebug("NewPort : %x", result);
  return(result);
}

static word	my_FreePort(int x)
{ if (x != NullPort)
   { IOdebug("FreePort : %x", x);
     return( ((real_FreePort)(x)) );
   }
  else
   return(Err_Null);
}

static void PatchPorts_aux(int x)
{ WORD	**modtab;
  WORD	 *kernel_slot;

  modtab = (WORD **) (*(WORD *)(*(WORD *)(&x - 1)));
  kernel_slot		= modtab[1];
  real_NewPort		= (WordFnPtr) kernel_slot[8];
  real_FreePort		= (WordFnPtr) kernel_slot[9];
  kernel_slot[8]	= (WORD) &my_NewPort;
  kernel_slot[9]	= (WORD) &my_FreePort;
}

void PatchPorts(void)
{
  IOdebug("Installing own versions of NewPort and FreePort");
  PatchPorts_aux(0);
}
#else
void PatchPorts(void)
{
}
#endif

