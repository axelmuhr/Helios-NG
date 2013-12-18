/*------------------------------------------------------------------------
--                                                                      --
--                      H E L I O S   K E R N E L                       --
--                      -------------------------                       --
--                                                                      --
--             Copyright (C) 1988, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- sem.c								--
--                                                                      --
--	Semaphore functions.						--
--	Future improvements here would be to allow timeouts on		--
--	semaphores, also a WaitIfFree function.				--
--                                                                      --
--	Author:  NHG 12/8/88						--
--                                                                      --
------------------------------------------------------------------------*/
/* SccsId:	 %W%	%G% Copyright (C) 1987, Perihelion Software Ltd.*/


#define __in_sem 1	/* flag that we are in this module */

#include "kernel.h"

/*--------------------------------------------------------
-- InitSemaphore					--
--							--
-- Initialise a sempahore 				--
--							--
--------------------------------------------------------*/

void InitSemaphore(Semaphore *sem, word count)
{
	sem->Count = count;
	sem->Head = NULL;
	sem->Tail = (Id *)sem;
}

/*--------------------------------------------------------
-- _Wait						--
--							--
-- Internal Wait routine.				--
-- MUST be called at high priority.			--
--							--
--------------------------------------------------------*/

Code _Wait(Semaphore *sem)
{
	Id w;
			
	sem->Count--;
	w.rc = 1;
	if( sem->Count < 0 )
	{
		w.next = NULL;
		sem->Tail->next = &w;
		sem->Tail = &w;
		Suspend(&w.state, 0);
	}
	return w.rc;
}

/*--------------------------------------------------------
-- _Signal						--
--							--
-- Internal Signal routine.				--
-- MUST be called at high priority.			--
--							--
--------------------------------------------------------*/

void _Signal( Semaphore *sem )
{
	sem->Count++;
	if( sem->Count < 1 )
	{
		Id *w = sem->Head;
		sem->Head = w->next;
		if( sem->Head == NULL ) sem->Tail = (Id *)sem;
		w->rc = 1;
		Resume(w->state);
	}
}

void _SignalStop(Semaphore *sem)
{
	_Signal(sem);
	Stop();
}

/*--------------------------------------------------------
-- AbortSem						--
--							--
-- Restart all processes on sempahore with Aborted Rc.	--
-- Not presently available outside the kernel.		--
-- MUST be called at high priority.			--
--							--
--------------------------------------------------------*/

void AbortSem(Semaphore *sem)
{
	sem->Count++;
	while( sem->Count < 1 )
	{
		Id *w = sem->Head;
		sem->Head = w->next;
		if( sem->Head == NULL ) sem->Tail = (Id *)sem;
		w->rc = 0;
		Resume(w->state);
		sem->Count++;
	}
}

/*--------------------------------------------------------
-- _TestSemaphore					--
--							--
-- Test the value of a semaphore and return it.		--
-- MUST be called at high priority.			--
--							--
--------------------------------------------------------*/

word _TestSemaphore(Semaphore *sem)
{
	return sem->Count;
}

/*--------------------------------------------------------
-- User Routines					--
--							--
-- Callable at any priority level.			--
--							--
--------------------------------------------------------*/

void Wait(Semaphore *sem)
{
	System(_Wait,sem);
}

void Signal(Semaphore *sem)
{
	System((WordFnPtr)_Signal,sem);
}

word TestSemaphore(Semaphore *sem)
{
	return (word)System(_TestSemaphore,sem);
}

void SignalStop(Semaphore *sem)
{
	System((WordFnPtr)_SignalStop,sem);

}

/* -- End of sem.c */
