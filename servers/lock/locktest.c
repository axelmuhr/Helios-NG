/*------------------------------------------------------------------------
--                                                                      --
--                     H E L I O S			                --
--                     -----------                                      --
--                                                                      --
--             Copyright (C) 1990, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- locktest.c								--
--                                                                      --
--	Test program for the lock library/server.			--
--                                                                      --
--	Author:  BLV 21.2.90						--
--                                                                      --
------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include "locklib.h"

int main(void)
{ Lock	lock1, lock2, lock3;

  puts("Attempting to create first lock.");
  lock1 = GetLock("Hello");
  if (lock1 == NullLock)
   { fputs("Failure: cannot create first lock\n", stderr);
     exit(EXIT_FAILURE);
   }

  puts("OK, attempting to create first lock again. This should fail.");
  lock2 = GetLock("Hello");
  if (lock2 != NullLock)
   { fputs("Failure: managed to create a lock twice\n", stderr);
     exit(EXIT_FAILURE);
   }

  puts("OK, attempting to create another lock.");
  lock2 = GetLock("Goodbye");
  if (lock2 == NullLock)
   { fputs("Failure: cannot create another lock.\n", stderr);
     exit(EXIT_FAILURE);
   }

  puts("OK, trying first lock again. This should still fail.");
  lock3 = GetLock("Hello");
  if (lock3 != NullLock)
   { fputs("Failure: managed to create a lock twice.\n", stderr);
     exit(EXIT_FAILURE);
   }

  puts("OK, trying to create second lock again.");
  lock3 = GetLock("Goodbye");
  if (lock3 != NullLock)
   { fputs("Failure: managed to create a lock twice.\n", stderr);
     exit(EXIT_FAILURE);
   }

  puts("OK, releasing both locks.");
  FreeLock(lock1);
  FreeLock(lock2);

  puts("OK, trying to create first lock again.");
  lock1 = GetLock("Hello");
  if (lock1 == NullLock)
   { fputs("Failure: cannot recreate a lock after freeing it.\n", stderr);
     exit(EXIT_FAILURE);
   }

  puts("Basic tests passed, cleaning up and exiting.");
  FreeLock(lock1);
  return(EXIT_SUCCESS);
}


