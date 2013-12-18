/*------------------------------------------------------------------------
--                                                                      --
--                      H E L I O S   K E R N E L                       --
--                      -------------------------                       --
--                                                                      --
--             Copyright (C) 1988, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- getmsg.c								--
--                                                                      --
--	Message reception routines.					--
--                                                                      --
--	Author:  NHG 8/8/88						--
--                                                                      --
------------------------------------------------------------------------*/
/* $Id: getmsg1.c,v 1.12 1993/09/03 16:56:22 paul Exp $ */

#define __in_getmsg 1	/* flag that we are in this module */

#include "kernel.h"

/* #define _Trace(a,b,c) */

/*--------------------------------------------------------
-- GetMsg						--
--							--
-- Receive a message.					--
--							--
--------------------------------------------------------*/

#ifndef TRANSPUTER
Code _GetMsg(MCB *mcb);

Code GetMsg(MCB *mcb)
{
	return (Code)System(_GetMsg,mcb);
}
#endif

Code _GetMsg(MCB *mcb)
{
	Id	id;
	PTE	*pte;
	Port	dest = mcb->MsgHdr.Dest;
	word	csize;
	word	dsize;
	RootStruct *root = GetRoot();
	
	if( (dest & (Port_Flags_Tx|Port_Flags_Remote)) != 0 ) 
		return Err_BadPort;
		
	pte = GetPTE(dest,root);

	if( NotSameCycle_(dest,pte) || (pte->Type != T_Local) )
		return Err_BadPort;

	pte->Age = 1;

	id.rc = Err_Null;
	id.mcb = mcb;
	
	if( pte->TxId == NULL || pte->RxId != NULL )
	{
		if( id.mcb->Timeout == 0 ) return Err_NotReady;
		id.next = NULL; id.tail = &id;
		if( Forever(id.mcb->Timeout) ) id.endtime = -1;
		else id.endtime = AddTimes(id.mcb->Timeout,Timer());

		if( pte->RxId == NULL ) pte->RxId = &id;
		else {
			pte->RxId->tail->next = &id;
			pte->RxId->tail = &id;
		}

		Suspend(&id.state, THREAD_MSGREAD);

		/* when we wake up the message will have been copied into */
		/* the MCB.						  */
	}
	else
	{
		/* There is a waiting transmitter, get the message.	*/
		Id *txid = pte->TxId;
		MCB *txmcb = txid->mcb;
		
		pte->TxId = txid->next;
		if( txid->next != NULL ) txid->next->tail = txid->tail;

		/* get header	*/
#ifndef __TRAN
		/* to reduce System priority latencies */
		/* dont copy at highpri */
		SetPhysPri(HIGHPRI+1);
#endif
#ifdef __TRAN
		MoveBlock(&id.mcb->MsgHdr,&txmcb->MsgHdr,sizeof(MsgHdr));
#else
		/* optimised fast copy by assignment */
		/* - for processors without block move instruction */

		/* following statement is equivalent to:
			id.mcb->MsgHdr.DataSize = txmcb->MsgHdr.DataSize;
			id.mcb->MsgHdr.ContSize = txmcb->MsgHdr.ContSize;
			id.mcb->MsgHdr.Flags    = txmcb->MsgHdr.Flags;
		*/
		*((word *)id.mcb) = *((word *)txmcb);

		id.mcb->MsgHdr.Dest  = txmcb->MsgHdr.Dest;
		id.mcb->MsgHdr.Reply = txmcb->MsgHdr.Reply;
		id.mcb->MsgHdr.FnRc  = txmcb->MsgHdr.FnRc;
#endif

		csize = (word)id.mcb->MsgHdr.ContSize * sizeof(word);
#ifdef __BIGENDIAN
		dsize = (word)id.mcb->MsgHdr.DataSize
#else
		dsize = (*(word *)&id.mcb->MsgHdr) & 0xffff;
#endif
		if( csize != 0 )
			MoveBlock(id.mcb->Control, txmcb->Control, csize);

		if( dsize != 0 )
#ifdef __C40
			/* if we are thru routed msg hitting its dest */
			/* then take byte align into account */
			MoveBlock(id.mcb->Data, txmcb->Data +
				(txmcb->MsgHdr.Flags &
				MsgHdr_Flags_bytealign), dsize);

			/* zero flags in case msghdr used again */
			id.mcb->MsgHdr.Flags &= ~MsgHdr_Flags_bytealign;
#else
			MoveBlock(id.mcb->Data, txmcb->Data, dsize);
#endif

#ifndef __TRAN
		/* to reduce System priority latencies we dont copy at hipri */
		/* now return back to System pri */
		SetPhysPri(HIGHPRI);
#endif

		Resume( txid->state );
	}

	if( id.mcb->MsgHdr.Reply != NullPort ) id.mcb->MsgHdr.Reply |= Port_Flags_Tx;

	id.mcb->MsgHdr.Dest = dest;

	if (id.rc == Err_Null) 
		return id.mcb->MsgHdr.FnRc;
	else 	return id.rc;
}


/*--------------------------------------------------------
-- GetReady						--
--							--
-- Test port for messages.				--
--							--
--------------------------------------------------------*/

Code _GetReady(Port port)
{
	RootStruct *root = GetRoot();
	PTE *pte = GetPTE( port, root );
	
	if( NotSameCycle_(port,pte) || pte->Type != T_Local )
		return Err_BadPort;
		
	pte->Age = 1;
	
	/* fail if the port already has a receiver waiting.		*/
	if( pte->RxId != NULL ) return Err_NotReady;
	
	/* fail if the port has no transmitters waiting.		*/
	if( pte->TxId == NULL ) return Err_NotReady;
	
	/* succeed in all other cases					*/
	return Err_Null;
}

Code GetReady(Port port)
{
	return (Code)System(_GetReady,port);
}


/*--------------------------------------------------------
-- MultiWait						--
--							--
-- Wait for a message on several ports simultaneously.	--
-- The message will be placed in the MCB which need only--
-- contain a timeout plus control & data vector ptrs.	--
-- If the timeout is 0 the ports are merely polled, if	--
-- it is -1 the call waits forever, otherwise it waits	--
-- for the length of the timeout.			--
-- The port list must be nports entries long, but some	--
-- entries may be NullPort.				--
-- The result is either the index of the activated port,--
-- EK_Timeout if the timeout expired, or -1 if the 	--
-- timeout was zero and no ports are ready.		--
--							--
--------------------------------------------------------*/

Code _MultiWait(MCB *mcb, word nports, Port *ports)
{
	RootStruct *root = GetRoot();
	word i;

	/* Entry in kernel modtab holding task pointer. */
	extern Task *_Task_;
	/* Allocate from tasks private memory. Makes sure that if task is */
	/* killed the memory will be free'd. */
	Id *ids = (Id *)Allocate(nports*sizeof(Id), root->FreePool, \
				&_Task_->MemPool);

	SaveState *state = NULL;
	word timeout = mcb->Timeout;
	word selected = -1;
	word e = 0;
	word queued = 0;
	
	if( ids == NULL ) return Err_NoMemory;
		
	mcb->MsgHdr.Dest = NullPort;
	mcb->MsgHdr.FnRc = Err_Null;
	
	/* scan port list for valid ports and add an id to each */
	for( i = 0; i < nports; i++ )
	{
		PTE *pte;
		Id *id = &ids[i];
		Port port = ports[i];
		
		id->rc = MultiUnused;
		
		if( port == NullPort ) continue;
		
		pte = GetPTE( port, root );
		
		if( NotSameCycle_(port,pte) || pte->Type != T_Local )
		{
			e = EC_Warn | SS_Kernel | EG_Invalid | i; 
			nports = i;
			goto cleanup;
		}
		
		pte->Age = 1;

		/* if there is a transmitter waiting, get the message 	*/
		/* and indicate a selection. This also means that there	*/
		/* is no need to carry on looking for a message.	*/
		if( pte->TxId != NULL )
		{
			mcb->MsgHdr.Dest = ports[i];
			id->rc = _GetMsg(mcb);
			nports = selected = i;
			goto cleanup;
		}

		/* A zero timeout causes us to just poll without waiting.*/
		/* Otherwise we hook an Id onto the port's receive queue */
		/* with a special rc value which indicates we are waiting*/
		if( timeout != 0 )
		{
			id->rc = MultiWaiting;
			id->next = NULL;
			id->tail = id;
			id->state = (SaveState *)&state;
			id->mcb = mcb;
			if( Forever(timeout) ) id->endtime = -1;
			else id->endtime = AddTimes(timeout,Timer());
			if( pte->RxId == NULL ) pte->RxId = id;
			else 
			{
				pte->RxId->tail->next = id;
				pte->RxId->tail = id;
			}
			queued++;
		}
	}
	
	if( queued != 0 && timeout != 0 )
	{
		/* if no port was selected during enable phase  */
		/* we must wait for one to be selected here	*/
		Suspend(&state, THREAD_MULTIWAIT);
	}

cleanup:	
	/* now go through the ports unhooking those that didnt work. */
	for( i = 0; i < nports; i++ )
	{
		Id *id = &ids[i];
		
		if( id->rc == MultiUnused ) continue;
		
		if( id->rc == MultiWaiting )
		{
			PTE *pte;
			Id *iid, *prev;

			/* unhook from port queue */
			
			pte = GetPTE( ports[i], root );
			for( iid = pte->RxId,prev=NULL; iid != id; 
					prev=iid,iid=iid->next);
			dq(&pte->RxId,prev,id);
			continue;
		}
		
		/* else it is one which caused wakeup */
		/* it will already have been unhooked */

		selected = i;
		if( id->rc != Err_Null ) e = mcb->MsgHdr.FnRc = id->rc;
		mcb->MsgHdr.Dest = ports[i];
	}

	_Free((Memory *)ids, root->FreePool);

	if( e == 0 ) e = selected;

	return e;
}

Code MultiWait(MCB *mcb, word nports, Port *ports)
{
	return (Code)System(_MultiWait,mcb,nports,ports);
}


/* -- End of getmsg.c */
