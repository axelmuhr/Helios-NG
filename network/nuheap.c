/*------------------------------------------------------------------------
--                                                                      --
--           H E L I O S   N E T W O R K I N G   S O F T W A R E	--
--           ---------------------------------------------------	--
--                                                                      --
--             Copyright (C) 1990, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- netutils : nuheap.c							--
--									--
--	Author:  BLV 3/8/90						--
--                                                                      --
------------------------------------------------------------------------*/
/* RcsId: $Header: /hsrc/network/RCS/nuheap.c,v 1.6 1993/08/11 10:38:11 bart Exp $*/

#include <helios.h>
#include <task.h>
#include <syslib.h>
#include <queue.h>
#include <memory.h>

/**
*** This unbelievably gruesome code zaps the module table entries for
*** Malloc and Free, installing my own routines which call the system ones
*** and produce some debugging. It only works on the transputer version.
**/

#ifdef __TRAN
static	WordFnPtr	real_Malloc;
static	WordFnPtr	real_Free;

static word	my_Malloc(int x)
{ word result = ((real_Malloc)(x));

  IOdebug("Malloc(%d) : %x", x, result);
  return(result);
}

static word	my_Free(int x)
{ word	result = ((real_Free)(x));

  IOdebug("Free(%x)", x);
  return(result);
}

void PatchMalloc(void)
{ int	*table = (int *) &MyTask;

  IOdebug("Installing own versions of Malloc and Free");
  real_Malloc = (WordFnPtr) table[26];
  real_Free   = (WordFnPtr) table[27];
  table[26]   = (int) &my_Malloc;
  table[27]   = (int) &my_Free;
}
#else
void PatchMalloc(void)
{
}
#endif

