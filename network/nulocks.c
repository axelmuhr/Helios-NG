/*------------------------------------------------------------------------
--                                                                      --
--           H E L I O S   N E T W O R K I N G   S O F T W A R E	--
--           ---------------------------------------------------	--
--                                                                      --
--             Copyright (C) 1991, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- netutils : locks.c							--
--									--
--	Author:  BLV 12/11/91						--
--                                                                      --
------------------------------------------------------------------------*/
/* RcsId: $Header: /hsrc/network/RCS/nulocks.c,v 1.2 1993/08/11 10:38:53 bart Exp $*/

/**
*** This module implements a multiple-read single-write locking mechanism
*** that is useful for the networking software. The code has been adapted
*** from Jean Bacon's lecture notes on "Operating Systems: Concurrency and
*** Synchronisation", Oct 1985, page CS10, with SwitchWrite() and
*** SwitchRead() added.
**/
#if 0
#define Debug(a) IOdebug(a)
#else
#define Debug(a)
#endif

#include <helios.h>
#include <sem.h>
#include <syslib.h>
#include <stdlib.h>
#include "netutils.h"

static	Semaphore	G, B;	/* Guard semaphores		*/
static	Semaphore	R, W;	/* Counting semaphores		*/
static	int		ar, aw; /* Active readers and writers	*/
static	int		rr, rw; /* Running readers and writers	*/
static	int		sw;	/* switching readers->writers	*/

void	MRSW_Init(void)
{
  Debug("MRSW_Init");
  InitSemaphore(&G, 1);
  InitSemaphore(&B, 1);
  InitSemaphore(&R, 0);
  InitSemaphore(&W, 0);
  ar	= 0;
  aw	= 0;
  rr	= 0;
  rw	= 0;
  sw	= 0;
  Debug("MRSW_Init done");
}

void	MRSW_GetRead(void)
{
  Debug("MRSW_GetRead");
  Wait(&B);
  ar++;
  if (aw == 0)
   { rr++; Signal(&R); }
  Signal(&B);

  Wait(&R);
  Debug("MRSW_GetRead done");
}

void	MRSW_GetWrite(void)
{
  Debug("MRSW_GetWrite");
  Wait(&B);
  aw++;
  if (rr == 0)
   { rw++; Signal(&W); }

  Signal(&B);
  Wait(&W);
  Wait(&G);
  Debug("MRSW_GetWrite done");
}

	/* Problem: when switching from a read lock to a write lock	*/
	/* it must be impossible for other threads to get a write lock	*/
	/* in between. Now, since we have a read lock there must be no	*/
	/* running writers and the W semaphore must be set to 0. Also,	*/
	/* G must be set to 1 since there are no writers.		*/
	/* Hence: this thread gets the G semaphore before activating	*/
	/* all the other writers. The other writers will restart but	*/
	/* will be blocked. This must be done outside the B block.	*/
	/* There is an outstanding problem if two threads try to switch	*/
	/* from a read to a write lock, this is guaranteed to fail.	*/
void	MRSW_SwitchWrite(void)
{ 
  Debug("MRSW_SwitchWrite");
  Wait(&G);
  Wait(&B);
  sw++;
  rr--;
  ar--;
  aw++;
  if (sw > 1)
   { IOdebug("MRSW_SwitchWrite: internal error, two calls");
     Exit(EXIT_FAILURE << 8);
   }
  if (rr == 0)
   while (rw < aw)
    { rw++; Signal(&W); }
  Signal(&B);
  Wait(&W);
  Debug("MRSW_SwitchWrite done");
  Wait(&B);
  sw--;
  Signal(&B);
}

void	MRSW_SwitchRead(void)
{
  Debug("MRSW_SwitchRead");
  Signal(&G);
  Wait(&B);
  rw--;
  aw--;
  ar++;
  if (aw == 0)
   while (rr < ar)
    { rr++; Signal(&R); }
  Signal(&B);
  Wait(&R);
  Debug("MRSW_SwitchRead done"); 
}

void	MRSW_FreeRead(void)
{ 
  Debug("MRSW_FreeRead");
  Wait(&B);
  rr--;
  ar--;
  if (rr == 0)
   while (rw < aw)
    { rw++; Signal(&W); }
  Signal(&B);
}

void	MRSW_FreeWrite(void)
{
  Debug("MRSW_FreeWrite");
  Signal(&G);
  Wait(&B);
  rw--;
  aw--;
  if (aw == 0)
   while (rr < ar)
    { rr++; Signal(&R); }
  Signal(&B);
}

void MRSW_GetInfo(MRSW_Info *buffer)
{ buffer->ar = ar;
  buffer->rr = rr;
  buffer->aw = aw;
  buffer->rw = rw;
  buffer->sw = sw;
}

