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
--	semaphores.							--
--                                                                      --
--	Author:  NHG 12/8/88						--
--		 JGS 910404	Timeouts on semaphores			--
--                                                                      --
------------------------------------------------------------------------*/
/* $Id: sem1.c,v 1.12 1993/09/17 08:30:45 paul Exp $ */


#define __in_sem 1	/* flag that we are in this module */

#define Waiter Id
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
-- The definition is that Wait return as soon as the	--
-- count may be decremented to yield a non-negative	--
-- value.						--
--							--
-- MUST be called at high priority.			--
--							--
--------------------------------------------------------*/

Code _Wait(Semaphore *sem)
{
	Id w;

	w.rc = 1;
	if( sem->Count <= 0 )
	{
		w.next = NULL;
		sem->Tail->next = &w;
		sem->Tail = &w;
		Suspend(&w.state, THREAD_SEMAPHORE);
		/* Resumed by Signal() */
	}
	else	sem->Count--;

	return w.rc;
}

/*--------------------------------------------------------
-- _TestWait						--
--							--
-- Test the value of the semaphore. If the process would--
-- have to suspend in Wait return false. Otherwise do a	--
-- Wait and return True.				--
-- MUST be called at high priority.			--
--							--
--------------------------------------------------------*/

bool _TestWait(Semaphore *sem)
{
	if( sem->Count <= 0 ) return false;
	_Wait(sem);
	return true;
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
	if( sem->Count >= 0 && sem->Head != NULL )
	{
		Id *w = sem->Head;

		sem->Head = w->next;
		if( sem->Head == NULL ) sem->Tail = (Id *)sem;
		w->rc = 1;
		Resume(w->state);
	}
	else sem->Count++;
}

#ifndef __TRAN
/*--------------------------------------------------------
-- _HardenedWait					--
--							--
-- Internal HardenedWait routine.			--
-- No need to be called at high priority as it turns	--
-- interrupts off for its duration and cannot be	--
-- sliced.						--
--							--
-- This fn is interrupt resilient, so that signals	--
-- received via HardenedSignal will not cause 		--
-- problems.						--
--							--
-- Semaphores should not be shared between hardened and --
-- Non hardened versions of wait and signal.		--
--							--
--------------------------------------------------------*/

void HardenedWait(Semaphore *sem)
{
	Id w;

	IntsOff();

	sem->Count--;
	if( sem->Count < 0 ) {
		w.next = NULL;
		sem->Tail->next = &w;
		sem->Tail = &w;

		Suspend(&w.state, THREAD_SEMAPHORE);
	}
	IntsOn();
	return;
}

#endif /* !__TRAN */


/*--------------------------------------------------------
-- _SignalStop						--
--							--
-- Signal the semaphore and halt the process atomically	--
-- MUST be called at high priority.			--
--							--
--------------------------------------------------------*/

void _SignalStop(Semaphore *sem)
{
	_Signal(sem);
	Stop();
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
	return sem->Head!=NULL ? -1 : sem->Count;
}

#if defined(__ARM) || defined(__C40)
/*--------------------------------------------------------
-- _TimedWait						--
--							--
-- The definition is that Wait must return as soon as	--
-- the count may be decremented to yield a non-negative	--
-- value.						--
--							--
-- This call provides a version of Wait that will	--
-- timeout (if required). If a "timeout" value of zero	--
-- is given then the function will operate exactly like	--
-- the TestWait function. If a "timeout" value of -1	--
-- is given then the function will operate exactly like	--
-- the Wait function.					--
-- The "timeout" value is given in microseconds.	--
--							--
-- This call will return "true" if the process was	--
-- signalled sucessfully before the timeout occured.	--
-- Alternatively "false" will be returned if the	--
-- process was awoken due to a timeout.			--
-- If the semaphore did timeout then it is not		--
-- decremented. i.e. the next wait will also block.	--
-- If a "timeout" of zero is given, the value returned	--
-- will be true if the Wait has been called, false if	--
-- the Wait would have blocked the process.		--
--							--
-- This function will place a process onto the standard	--
-- TimerQ at the position described by the "timeout".	--
-- The process will then be Suspended like a normal	--
-- Wait operation. The process on the TimerQ will then	--
-- naturally progress to the front of the TimerQ and be	--
-- re-started. This call will then return -1 to notify	--
-- the caller that the Wait has timed-out. If the	--
-- Semaphore is signalled before the process on the	--
-- TimerQ has been started, this code will be Resumed	--
-- (at hi-priority) and then scan the TimerQ, removing	--
-- the corresponding process description.		--
-- NOTE: Timer IRQs will need to be disabled whilst	--
--	 the TimerQ is being scanned and modified.	--
--							--
-- Any Signals that may occur AFTER a timed-out Wait	--
-- will simply increment the Semaphore count as		--
-- standard (like a normal Signal without a Wait).	--
--							--
-- Note that TimedWait may be used with HardendSignal().--
--							--
-- MUST be called at high priority.			--
--							--
--------------------------------------------------------*/
bool _TimedWait(Semaphore *sem,word timeout)
{
	Id w;

	if (timeout == 0)
		return(_TestWait(sem));

	if (timeout == -1) {
		_Wait(sem);
		return(true);	/* always non-timeout case */
	}

	w.rc = true;		/* assume direct return */

	if (sem->Count <= 0) {
		/* We disable IRQs here, since IRQs may call "HardenedSignal"
		 * or "Resume" which may attempt to perform operations on
		 * Semaphore thread that has just timed-out.
		 */
		IntsOff();

		/* This process will need to block */
		w.next = NULL;
		sem->Tail->next = &w;
		sem->Tail = &w;

		/* TimedSuspend() returns TRUE if Signal()'d,
		 * FALSE if timed out.
		 */
		if ((w.rc = TimedSuspend(&w.state,timeout)) == FALSE) {
			Id *lw = sem->Head;

			/* Remove our Id structure from the Semaphore's list */
			if (lw == &w)		/* is our Id at head? */
				/* YES - then next is new head */
				sem->Head = lw->next;
			else
			{
				Id *nw;

				/* remove from mid list */
				while ((nw = lw->next) != NULL) {
					/* is next Id our Id? */
					if (nw == &w) {
						/* un-link our Id, fixing tail
						   pointer if required */
						if ((lw->next = nw->next) == NULL)
							sem->Tail = lw;
						break;
					}
					lw = nw;
				}
				/* if we didn't find it in the list then we */
				/* are in BIG trouble - our thread has just */
				/* self forked! */
			}

			/* ensure a valid tail pointer if we have removed */
			/* the only list entry */
			if (sem->Head == NULL)
				sem->Tail = (Id *)sem;
		}

		IntsOn();	/* re-enable interrupts */
	}
	else	sem->Count--;

	return(w.rc);	/* "true" if Signalled, "false" if timed-out */
}
#endif /* __ARM || __C40 */


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

bool TestWait(Semaphore *sem)
{
	return (word)System(_TestWait,sem);
}

void Signal(Semaphore *sem)
{
	System((WordFnPtr)_Signal,sem);
}

#ifndef __TRAN
/*--------------------------------------------------------
-- HardenedSignal					--
--							--
-- Event handler Signal routine.			--
-- Can be used safely in event (interrupt) handlers	--
-- Must only be paired up with a HardenedWait() fn, 	--
-- not a normal Wait(), and should ONLY be called 	--
-- while interrupts are be disabled			--
--							--
--------------------------------------------------------*/

void HardenedSignal( Semaphore *sem )
{
	sem->Count++;
	if( sem->Count < 1 )
	{
		Id *w = sem->Head;
		sem->Head = w->next;
		if( sem->Head == NULL ) sem->Tail = (Id *)sem;

		IntrResume(w->state);
	}
}
#endif

word TestSemaphore(Semaphore *sem)
{
	return (word)System(_TestSemaphore,sem);
}

void SignalStop(Semaphore *sem)
{
	System((WordFnPtr)_SignalStop,sem);
}

#if defined(__ARM) || defined(__C40)
word TimedWait(Semaphore *sem,word timeout)
{
	return((word)System(_TimedWait,sem,timeout));
}
#endif



/* -- End of sem.c */
