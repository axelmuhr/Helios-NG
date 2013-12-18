/*------------------------------------------------------------------------
--                                                                      --
--                     H E L I O S			                --
--                     -----------                                      --
--                                                                      --
--             Copyright (C) 1990, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- locklib.c								--
--                                                                      --
--	Library to interact with the lock server.			--
--                                                                      --
--	Author:  BLV 21.2.90						--
--                                                                      --
------------------------------------------------------------------------*/

/**
*** Note: this library only supports locks at the top level of the
*** /lock server. It does not attempt to use the subdirectory
*** facilities.
**/

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "locklib.h"

Lock	GetLock(char *name)
{ Object	*lock_server;
  Lock		result;

  if ((name == Null(char))		||
      (strlen(name) >= NameMax)		||
      (strchr(name, '/') != NULL) )
   { fputs("locklib: GetLock, invalid lock name passed as argument.\n", stderr);
     exit(EXIT_FAILURE);
   }

  lock_server = Locate(Null(Object), "/lock");
  if (lock_server == Null(Object))
   { fputs("locklib: GetLock, lock server is not running.\n", stderr);
     exit(EXIT_FAILURE);
   }

  result = Create(lock_server, name, Type_Stream, 0, Null(BYTE));
  Close(lock_server);
  return(result);
}

void	FreeLock(Lock lock)
{ (void) Delete(lock, Null(char));
  Close(lock);
}

  
