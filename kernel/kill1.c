/*------------------------------------------------------------------------
--                                                                      --
--                      H E L I O S   K E R N E L                       --
--                      -------------------------                       --
--                                                                      --
--             Copyright (C) 1988, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- kill.c								--
--                                                                      --
--	Port and task termination.					--
--	Although in theory processor-specific, if the implementation	--
--	of the scheduler on other processors is close enough to the	--
--	transputer, this should be usable.				--
--                                                                      --
--	Author:  NHG 8/8/88						--
--                                                                      --
------------------------------------------------------------------------*/
/* SccsId: %W%\t%G% Copyright (C) 1987, Perihelion Software Ltd.*/
/* RcsId: $Id: kill1.c,v 1.18 1993/10/21 13:18:51 nickc Exp $ */

#define __in_kill 1	/* flag that we are in this module */

#include "kernel.h"
#include <message.h>
#include <root.h>
#include <task.h>

#define _Trace1(a,b,c)

#ifdef __TRAN
bool EvictRunQs(Pool *pool);
bool EvictTimerQ(Pool *pool);
#else
void EvictRunQs(Pool *pool);
void EvictTimerQ(Pool *pool);
bool IsLinkIOThread(SaveState *p);
void EvictKnownThreadList(Pool *pool);
#endif
void EvictPortTable(Pool *pool, Task *task);
void EvictLinks(Pool *pool);
#if 0
/* @@@ TBD */
void EvictEvents(Pool *pool);
#endif

/*--------------------------------------------------------
-- _AbortPort						--
--							--
-- Terminate all waiters on a given port with the given	--
-- return code.						--
-- MUST be called at high priority.			--
--							--
--------------------------------------------------------*/

Code _AbortPort(Port port, Code rc)
{
	RootStruct *root = GetRoot();
	PTE *pte = GetPTE(port,root);	
	Id *id;
	
	if( port == NullPort ) return Err_BadPort;
	
	if( NotSameCycle_(port,pte) || pte->Type == T_Free ) 
		return Err_BadPort;
	
	pte->Age = 1;
	
	if( pte->Type == T_Local )
	{
		for(id = pte->TxId; id != NULL; id=id->next )
		{
			id->rc = rc;
			Resume(id->state);
		}
		pte->TxId = NULL;
				
		for(id = pte->RxId; id != NULL; id=id->next )
		{
			if( id->rc == MultiWaiting )
			{
				SaveState **statep = (SaveState **)id->state;
				id->rc = rc;
				if( *statep != NULL )
				{
					Resume(*statep);
					*statep = NULL;
				}
			}
			else { id->rc = rc; Resume(id->state); }
		}
		pte->RxId = NULL;
	}
#ifdef LINKIO
	else /* a surrogate */
	{
		LinkInfo *link = root->Links[pte->Link];
		Id *prev;
		
		/* search the TxQueue for processes transmitting to the */
		/* port this is a surrogate for.			*/
		for(id = link->TxQueue,prev=NULL; id != NULL; id=id->next )
		{
			CheckState(id->state);
			if( id->mcb->MsgHdr.Dest != (Port)pte->TxId )
			{ prev = id; continue; }

			id->rc = rc;
			dq(&link->TxQueue,prev,id);
			Resume(id->state);
		}
	}
#endif	
	return Err_Null;
}

/*--------------------------------------------------------
-- _KillTask						--
--							--
-- Terminate a task and all its sub-processes.		--
-- MUST be called at high priority.			--
--							--
--------------------------------------------------------*/

Code _KillTask(Task *task)
{
	Pool *pool = &task->MemPool;

	/* Evict processes from timer and run qs	*/
#ifdef LINKIO
	EvictLinks(pool);
#endif
#ifdef __TRAN
	while( EvictTimerQ(pool) || EvictRunQs(pool) );
#else
	EvictTimerQ(pool);
	EvictRunQs(pool);
	EvictKnownThreadList(pool);
#endif

	/* Clean up and free any ports owned by task */
	EvictPortTable(pool, task);

#if 0
	/* Remove any events serviced by this task */
	EvictEvents(pool); /* TBD */
#endif

	return Err_Null;
}


/* Clean and free any ports owned by task (inpool(pool)) in the port table */
void EvictPortTable(Pool *pool, Task *task)
{
    RootStruct	*root = GetRoot();
    PTE **basetab = root->PortTable;
    word btindex, ptindex;
	    
    /* loop over port tables */
    for( btindex = 0 ; basetab[btindex] != NULL; btindex++ )
    {
	Id *id;
	Id *prev;
	PTE *ptab = basetab[btindex];

	if( ptab == (PTE *)MinInt ) continue;

	for( ptindex = 0; ptindex < 64 ; ptindex++ )
	{
		PTE *pte = ptab + ptindex;

		/* Ignore free ports */
		if( pte->Type == T_Free ) continue;
			
		/* Free any surrogates owned by me */
		if(     (pte->Type == T_Surrogate) && 
			(pte->Owner == (word)task) )
		{
			Port port = MinInt |
				((uword)pte->Cycle << Port_Cycle_shift)
				| (btindex << 8) | ptindex;

			__FreePort(port,TRUE,Err_Abort);
			continue;
		}
			
		if( pte->Type != T_Local ) continue;

		/* here we have an allocated local port	*/
			
		/* start by checking the transmitters	*/
		for(id = pte->TxId,prev=NULL; id != NULL; id=id->next)
		{
			if( !inpool(id,pool) ) { prev = id; continue; }
			/* kill this process */	
			id->rc = Err_Kill;
#ifndef __TRAN
			KnownThreadRm(id->state);
#endif
_Trace1(0xffff1111,id,id->state);
			dq(&pte->TxId,prev,id);
		}

		/* If this task owns the port, kill all	*/
		/* receivers and abort all transmitters.*/
		/* We can also free the port.		*/
		if( pte->Owner == (word)task )
		{
			Port port;

#ifndef __TRAN
			/* kill all receivers */
			for (id = pte->RxId, prev=NULL; id != NULL; id=id->next) {
				/* kill this thread & rm from known thread Q */

			    
			    if (id->rc == MultiWaiting)
			      {
				KnownThreadRm(*((SaveState **)(id->state)));
			      }
			    else
			      {
				KnownThreadRm(id->state);
			      }
				id->rc = Err_Kill;
				dq(&pte->RxId,prev,id);
			}
			pte->RxId = NULL ;
#else
	/*****************************************************************
	 * this just detaches the Rx process... it does not kill them... *
	 * theoretically they should never be re-started (since they	 *
	 * are only referenced by this value)...			 *
	 *****************************************************************/
			pte->RxId = NULL;
#endif
			/* and abort all transmitters		*/
			for(id = pte->TxId; id != NULL; id=id->next )
			{
				id->rc = Err_Abort;
_Trace1(0xffff2222,id,id->state);
				Resume(id->state);
			}
			pte->TxId = NULL;
				
			/* now free the port */
			port = MinInt | ((uword)pte->Cycle << Port_Cycle_shift)
				| (btindex << 8) | ptindex;
			_FreePort(port);
		}
			
	} /* sub table */
		
	Yield();	/* give rest of system a chance	*/
		
    } /* basetab */
	    
} /* EvictPortsTable */


/* look at links and zap any link traffic this task may be	*/
/* involved in. (task = inpool(pool))				*/
/* We have to make two passes over the links, the first evicts  */
/* processes from the TxQueue of each link and sets the rc of	*/
/* any processes we own waiting on the link.			*/
/* The second pass waits for these transfers to complete.	*/
#ifdef LINKIO
void EvictLinks(Pool *pool)
{
	RootStruct *root = GetRoot();
	LinkInfo **links = root->Links;
	int i;

    	for( i = 0; links[i] != NULL ; i++ )
    	{
    		LinkInfo *l = links[i];
    		Id *lu, *id, *prev;

		/* First check link TxQueue	*/
		for( id=l->TxQueue,prev=NULL; id!=NULL; id=id->next)
		{
			CheckState(id->state);
			if( !inpool(id,pool) ) { prev = id; continue; }

			id->rc = Err_Kill;
_Trace1(0xffff3333,id,id->state);
#ifndef __TRAN
			/* killing thread so remove from known thread Q */
			KnownThreadRm(id->state);
#endif
			dq(&l->TxQueue,prev,id);
		}

		lu = l->TxUser;			
    		if( lu != NULL && inpool(lu,pool) ) {
#ifndef __TRAN
			/* killing thread so remove from known thread Q */
			KnownThreadRm(lu->state);
#endif
			lu->rc = Err_Kill;
		}

    		lu = l->RxId;
    		if( lu != NULL && inpool(lu,pool) ) {
#ifndef __TRAN
			/* killing thread so remove from known thread Q */
			KnownThreadRm(lu->state);
#endif
			lu->rc = Err_Kill;
		}
    	}

    	for( i = 0; links[i] != NULL ; i++ )
	{
    		LinkInfo *l = links[i];
    		Id *lu = l->TxUser;

    		if( lu != NULL && lu->rc == Err_Kill )
    		{
    			Id me;
    			me.next = l->TxQueue;
#if 0
			/* old erroneous code - will not work if no other */
			/* threads on Q and another thread gets added to */
			/* Q after we have inserted ourselves at the front */
    			me.tail = l->TxQueue->tail;
#else
			if (me.next != NULL)
	    			me.tail = l->TxQueue->tail;
			else
				me.tail = &me;
#endif
    			me.endtime = -1;
    			l->TxQueue = &me;
_Trace1(0xffff4444,lu,lu->state);
    			Suspend(&me.state, THREAD_LINKEND);
			_SignalLink(l);
    		}

		lu = l->RxId;
    		if( lu != NULL && lu->rc == Err_Kill )
    		{
    			Id me;
    			me.next = NULL;
    			me.tail = &me;
    			me.endtime = -1;
    			l->Sync = &me;
_Trace1(0xffff5555,lu,lu->state);    			
    			Suspend(&me.state, THREAD_LINKEND);
    		}
	}
}
#endif /* LINKIO */


void dq(Id **lvq, Id *prev, Id *id)
{
	if( prev == NULL ) 
	{
		*lvq = id->next;
		if( id->next != NULL ) id->next->tail = id->tail;
	}
	else {
		prev->next = id->next;
		if(id->next == NULL) (*lvq)->tail = prev;
	}
}

#ifdef __TRAN
void EndDummy(Buffer *stack)
{
	FreeBuf(stack);
	Stop();
}

/* dummy fn substituted for head timer entry */
void Dummy(Buffer *stack)
{
_Trace1(0xdddd3333,stack,0);
	System((WordFnPtr)EndDummy,stack);
	return;
}

/* TRANSPUTER specific version */
bool EvictTimerQ(Pool *pool)
{
	SaveState *p;
	SaveState *prev = TimerQHead();
	word *s, *w, *stack;
	bool evicted = FALSE;

	if( NullStateP(prev) ) return FALSE;

	p = P_TimerNext(prev);

	/* evict any processes from the the queue */
        /* xputer version processes head entry differently */

	while( !NullStateP(p) )
	{
		CheckState(p);

		if( inpool((SaveState *)p, pool) )
		{
_Trace1(0xdddd1111,prev,p);
			/* remove from timer Q and known thread Q */
			p = P_TimerNext(p);
			Set_P_TimerNext(prev, (SaveState *)p);

			evicted = TRUE;
		}
		else 
		{
			prev = p;
			p = P_TimerNext(p);
		}
	}

	p = TimerQHead();
	
	/* now see if the head process must be zapped */

	if( !inpool(p,pool) ) return evicted;

	/* if so, replace it with a kernel process which will wake up	*/
	/* on the timeout instead. We have to do this because the tranny*/
	/* will not allow us to change the timeout register in the	*/
	/* cpu.								*/
	
	stack = (word *)GetBuf(Worker_stacksize);
_Trace1(0xdddd2222,p,stack);

	if( stack == NULL ) { Sleep(1000); return TRUE; }

	s = &stack[Worker_stacksize/4];
	w = CreateProcess(s,Dummy,EndDummy,stack,8);
	w[0] = (word)stack;
	s = w-2;
	Set_P_RunqNext(s,P_RunqNext(p));
	Set_P_BufAddr(s,P_BufAddr(p));
	Set_P_TimerNext(s,P_TimerNext(p));
	Set_P_EndTime(s,P_EndTime(p));
	Set_TimerQHead(s);

	return TRUE;
}

#else /* Generic Executive version */

void EvictTimerQ(Pool *pool)
{
	ExecRoot *xroot = GetExecRoot();
	SaveState *p, *tqhead, *prev = NULL;

	if (xroot->TimerQ == NULL)
		/* nothing on the Q */
		return;

	/* disable clock interrupts so that Q stays in a consistent state */
	ClockIntsOff();
		/* Force Q into a quiescent state, so we don't have */
		/* to run with clock interrupts disabled */

		/* get head item on Q */
		tqhead = p = xroot->TimerQ;

		/* set Q to empty */
		xroot->TimerQ = NULL;
	ClockIntsOn();

	/* evict any of the Tasks threads from the the queue */
	while( !NullStateP(p) ) {
		CheckState(p);

#if defined(__C40) || defined(__ARM)
		if (inpool(p, pool)
#if 0 /* no longer necessary because EvictLinks() is run before EvictTimerQ() */
		    
		/* Dont remove from timer Q if it is the TimedWait() thread */
		/* used in the linkIO code to implement HalfDuplex Protocol */
		/* This allows the current message to complete, the thread */
		/* then being handled by the EvictLinks() code */
		    
		&& !(p->status == THREAD_TIMEDWAIT && IsLinkIOThread(p))
#endif
		    ) {

#else
		if (inpool(p, pool)) {
#endif
			/* remove from timer Q and known thread Q */
			KnownThreadRm(p);

			if (p == tqhead) {
				/* set new head item, removing old from Q */
				tqhead = p = P_TimerNext(p);
			} else {
				/* remove from Q */
				p = P_TimerNext(p);
				Set_P_TimerNext(prev, p);
			}
		} else {
			/* note that prev only needs to be in a consistent */
			/* state once we truck over a valid head item */
			prev = p;
			p = P_TimerNext(p);
		}
	}

	ClockIntsOff();
		/* Put the timer Q back into action */
		xroot->TimerQ = tqhead;
	ClockIntsOn();
}
#endif


#ifdef __TRAN /* TRANSPUTER specific version */
bool EvictRunQs(Pool *pool)
{
	SaveState *p, *prev = NULL;
	struct { SaveState *head, *tail; } runq;
	bool atfront = TRUE;
	bool evicted = FALSE;
		
	/* first do the low priority queue */

	savel_(&runq);

	p = runq.head;
	
	if( !NullStateP(p) ) forever
	{
		if( inpool(p,pool) )
		{
			evicted = TRUE;
			if( atfront ) 
			{
				savel_(&runq);
_Trace1(0xeeee1111,p,runq.tail);
				if( p == runq.tail ) 
				{
					stlf_(P_NullState);
					break;
				}
				p = P_RunqNext(p);
				stlf_(p);
			}
			else
			{
				savel_(&runq);
_Trace1(0xeeee2222,p,runq.tail);
				if( p == runq.tail ) 
				{
					stlb_(prev);
					break;
				}
				p = P_RunqNext(p);
				Set_P_RunqNext(prev,p);
			}
		}
		else
		{
			savel_(&runq);
			if( p == runq.tail ) break;
			prev = p;
			p = P_RunqNext(p);
			atfront = FALSE;
		}
	}

	return evicted;

#if 0
	/* now do the high priority queue 			*/
	/* this is potentially dangerous and I am not sure yet	*/
	/* whether it is necessary				*/
	
	atfront = TRUE;
	
	saveh_(&runq);

	p = runq.head;
	
	if( !NullStateP(p) ) forever
	{
		if( inpool(p,pool) )
		{
			evicted = TRUE;
			if( atfront ) 
			{
				saveh_(&runq);
_Trace1(0xeeee3333,p,runq.tail);
				if( p == runq.tail ) 
				{
					sthf_(P_NullState);
					break;
				}
				p = P_RunqNext(p);
				sthf_(p);
			}
			else
			{
				saveh_(&runq);
_Trace1(0xeeee4444,p,runq.tail);				
				if( p == runq.tail ) 
				{
					sthb_(prev);
					break;
				}
				p = P_RunqNext(p);
				Set_P_RunqNext(prev,p);
			}
		}
		else
		{
			saveh_(&runq);
			if( p == runq.tail ) break;
			prev = p;
			p = P_RunqNext(p);
			atfront = FALSE;
		}
	}

	return evicted;
#endif
}

#else

/* Generic Executive version */

void EvictRunQs(Pool *pool)
{
	volatile SaveState *p, *prev;
	word highest = GetPhysPriRange();	/* get lowest pri level avail */
	word pri;

	/* Assumes multiple process priorities are implemented */
	/* checks each priorities run Q. */

	/* Since we are the current thread, we do not exist on any process
	 * queue. We should detach all processes belonging to this task,
	 * including HIPRI. Their resources will be released later.
	 * As we are running at HighPri, the only thing that can happen to the
	 * runQ while interrupts are enabled is to have threads appended.
	 */
	for (pri = 0; pri <= highest; pri++) {
		volatile ThreadQ *runq = ReadyQBase(pri);

		/* IntsOff() is used to guard portions of the code against */
		/* interference by interrupt handlers that change the run */
		/* Q's Timer/LinkTx/Rx - dont constantly disable interrupts */
		/* as we want a low interrupt latency */
		
		IntsOff();

		prev = p = runq->head;

		while (!NullStateP(p)) {
			IntsOn();
			/* if inpool(p,pool) and not involved in link IO then remove thread from Q */
			if (inpool((SaveState *)p, pool) && !IsLinkIOThread((SaveState *)p)) {
				IntsOff();
					/* remove this thread from the run queue */
					Set_P_RunqNext(prev, P_RunqNext(p));
					if (runq->head == p)
						runq->head = P_RunqNext(p);

					if (runq->tail == p)
						runq->tail = (SaveState *)prev;
				IntsOn();

				/* note its new status */
				p->status = THREAD_KILLED;

				/* remove thread from known thread Q */
				KnownThreadRm((SaveState *)p);
				IntsOff();
					p = P_RunqNext(prev);
			}
			else {
				IntsOff();
					/* step onto the next process */
					prev = p;
					p = P_RunqNext(p);
			}
		}

		if (NullStateP(runq->head))
			runq->tail = (SaveState *)runq;

	}

	IntsOn();
}


/*--------------------------------------------------------
-- IsLinkIOThread					--
--							--
-- Return true if thread savestate passed is currently	--
-- being used for LinkTx/Rx on any of the links.	--
--							--
--------------------------------------------------------*/

bool IsLinkIOThread(SaveState * p) {
	LinkInfo **links = GetRoot()->Links;
	LinkInfo *l;
	int i;

	/* If thread is currently involved in */
	/* a link I/O  operation, then let it */
	/* remain for EvictLinks() to deal with. */
	for( i = 0; (l = links[i]) != NULL ; i++ ) {
		if ( (l->TxUser != NULL && l->TxUser->state == p)
		|| (l->RxUser != NULL && l->RxUser->state == p) ) {
			return TRUE;
		}
	}

	return FALSE;
}


/*--------------------------------------------------------
-- EvictKnownThreadList					--
--							--
-- Remove all threads held on known thread list that	--
-- are in the given pool. The following code will only	--
-- work for processors that use the generic executive.	--
--							--
--------------------------------------------------------*/

void EvictKnownThreadList(Pool *pool) {
	ExecRoot  *xroot = GetExecRoot();
	SaveState *lss = NULL;				/* last ss */
	SaveState *css;					/* current ss */
	word	   pri = xroot->CurrentPri;

	/* guard the list against corruption by creation of new threads */
	xroot->CurrentPri = HIGHPRI;

	css  = xroot->KnownThreads;
	
	/* remove from executives list of known threads */
	while (css != NULL) {
		if (inpool(css, pool)) {
			/* remove from known thread list */
			if (lss == NULL) {
				/* if head item, fix new head */
				xroot->KnownThreads = css->nextknown;
			} else {
				/* unlink from list */
				lss->nextknown = css->nextknown;
			}
			css = css->nextknown;	/* get next */
		} else {
			lss = css;		/* remember last */
			css = css->nextknown;	/* get next */
		}
	}

	/* return to original pri */
	xroot->CurrentPri = pri;
}

#endif /* Generic */


/*--------------------------------------------------------
-- AbortPort						--
--							--
-- External call interfaces.				--
--							--
--------------------------------------------------------*/

Code AbortPort(Port port, Code rc)
{
	return (Code)System(_AbortPort,port,rc);
}

Code KillTask(Task *task)
{
	Code rc = (Code)System(_KillTask,task);

	return rc;
}


/* End of kill.c */
