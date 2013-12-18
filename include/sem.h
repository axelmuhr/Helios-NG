/*------------------------------------------------------------------------
--                                                                      --
--                     H E L I O S   N U C L E U S                      --
--                     ---------------------------                      --
--                                                                      --
--             Copyright (C) 1987, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- sem.h								--
--                                                                      --
--	Kernel semaphore support					--
--                                                                      --
--	Author:  NHG 16/8/87						--
--                                                                      --
------------------------------------------------------------------------*/
/* SccsId: %W%	%G% Copyright (C) 1987, Perihelion Software Ltd.	*/
/* $Id: sem.h,v 1.4 1992/06/01 09:47:18 paul Exp $ */

#ifndef __sem_h
#define __sem_h

#ifndef __helios_h
#include <helios.h>
#endif

/* Semaphore structure */

#ifndef _ID_
#define _ID_
struct Id { word secret; };	/* secret to kernel			*/
#endif

typedef struct Semaphore {
        word		Count;		/* semaphore counter		*/
        struct Id	*Head;		/* head of process list		*/
        struct Id	*Tail;		/* tail of process list		*/
} Semaphore;


/* Kernel support routines */

PUBLIC void InitSemaphore(Semaphore *, word);
PUBLIC void Wait(Semaphore *);
PUBLIC bool TestWait(Semaphore *);
PUBLIC void Signal(Semaphore *);
PUBLIC void SignalStop(Semaphore *);
PUBLIC word TestSemaphore(Semaphore *);

#ifndef __TRAN
 PUBLIC void HardenedWait(Semaphore *);
 PUBLIC void HardenedSignal(Semaphore *);
 PUBLIC bool TimedWait(Semaphore *sem, word timeout);
#endif

#endif


/* -- End of sem.h */

