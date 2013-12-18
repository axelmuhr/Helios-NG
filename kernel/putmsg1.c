/*------------------------------------------------------------------------
--                                                                      --
--                      H E L I O S   K E R N E L                       --
--                      -------------------------                       --
--                                                                      --
--             Copyright (C) 1988, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- putmsg.c								--
--                                                                      --
--	Message transmission						--
--                                                                      --
--	Author:  NHG 8/8/88						--
--                                                                      --
------------------------------------------------------------------------*/
/* $Id: putmsg1.c,v 1.29 1993/08/11 09:48:13 nickc Exp $ */

#define __in_putmsg 1	/* flag that we are in this module */

#include "kernel.h"
#include <message.h>

/*--------------------------------------------------------
-- PutMsg						--
--							--
-- Transmit a message.					--
--							--
--------------------------------------------------------*/

#ifndef TRANSPUTER
Code _PutMsg(MCB *mcb);

Code PutMsg(MCB *mcb)
{
	return (Code)System(_PutMsg,(mcb));
}
#endif

Code _PutMsg(MCB *mcb)
{
	Id	id;
	PTE	*pte;
	word	csize = (word)mcb->MsgHdr.ContSize * sizeof(word);
#ifdef __BIGENDIAN
	word	dsize = mcb->MsgHdr.DataSize;
#else
	word	dsize = (*(word *)&mcb->MsgHdr)&0xffff;
#endif
	Port	dest = mcb->MsgHdr.Dest;
	RootStruct *root = GetRoot();

#if defined(__C40) && defined(SYSDEB)
	/* @@@ this is probably being a little too paranoid! */
	if ((word)mcb & 3)
		/* MsgHdr must be word aligned */
		return Err_BadPort;
#endif

	if( dest == NullPort ) return Err_BadPort;

	pte = GetPTE(dest,root);

	if( NotSameCycle_(dest,pte) ) return Err_BadPort;

	pte->Age = 1;

	id.rc = Err_Null;
	id.mcb = mcb;

	if( pte->Type == T_Local )
	{
		root->LocalMsgs += sizeof(MsgHdr) + csize + dsize;
		/* The order of the next two tests is important.	*/
		if( pte->TxId != NULL )
		{
			if( id.mcb->Timeout == 0 ) return Err_NotReady;
			id.next = NULL; id.tail = &id;
			if( Forever(id.mcb->Timeout) ) id.endtime = -1;
			else id.endtime = AddTimes(id.mcb->Timeout,Timer());

			/* here we know that pte->TxId != NULL 		*/
			pte->TxId->tail->next = &id;
			pte->TxId->tail = &id;
			Suspend(&id.state, THREAD_MSGWRITE);

			/* when we wake up the receiver will have taken	*/
			/* the message.					*/
		}
		elif( pte->RxId == NULL )
		{
			if( id.mcb->Timeout == 0 ) return Err_NotReady;
			id.next = NULL; id.tail = &id;
			if( Forever(id.mcb->Timeout) ) id.endtime = -1;
			else id.endtime = AddTimes(id.mcb->Timeout,Timer());

			/* the previous test means we can assume	*/
			/* pte->TxId == NULL here.			*/
			pte->TxId = &id;
			Suspend(&id.state, THREAD_MSGWRITE2);

			/* when we wake up the receiver will have taken	*/
			/* the message.					*/
		}
		else 
		{
			/* The receiver is ready and waiting, copy the	*/
			/* message over and restart him.		*/
			Id *rxid = pte->RxId;
			MCB *rxmcb = rxid->mcb;
			
			pte->RxId = rxid->next;
			if( rxid->next != NULL ) rxid->next->tail = rxid->tail;

			/* we do not touch the PTE after this point */
#ifndef __TRAN
			/* to reduce System priority latencies */
			/* dont copy at highpri */
			SetPhysPri(HIGHPRI+1);
#endif
#ifdef __TRAN
			/* MoveBlock is an instruction i.e. its fast on tranies */
			MoveBlock(&rxmcb->MsgHdr,&id.mcb->MsgHdr,sizeof(MsgHdr));
#else
			/* fast copy by assignment for other processors */

			/* next statement is equivalent to:
			   rxmcb->MsgHdr.DataSize = id.mcb->MsgHdr.DataSize;
			   rxmcb->MsgHdr.ContSize = id.mcb->MsgHdr.ContSize;
			   rxmcb->MsgHdr.Flags    = id.mcb->MsgHdr.Flags;
			*/

			*((word *)rxmcb) = *((word *)id.mcb);

			rxmcb->MsgHdr.Dest    = id.mcb->MsgHdr.Dest;
			rxmcb->MsgHdr.Reply   = id.mcb->MsgHdr.Reply;
			rxmcb->MsgHdr.FnRc    = id.mcb->MsgHdr.FnRc;
#endif

			if( csize != 0 )
				MoveBlock(rxmcb->Control,id.mcb->Control,csize);

			if( dsize != 0 )
#ifdef __C40
			{
				/* if we are thru routed msg hitting its dest */
				/* then take byte align into account */
				MoveBlock(rxmcb->Data, id.mcb->Data +
					(id.mcb->MsgHdr.Flags &
					MsgHdr_Flags_bytealign), dsize);

				/* zero flags in case msghdr used again */
				rxmcb->MsgHdr.Flags &= ~MsgHdr_Flags_bytealign;
			}
#else
				MoveBlock(rxmcb->Data,id.mcb->Data,dsize);
#endif

#ifndef __TRAN
			/* to reduce System priority latencies */
			/* dont copy at highpri - now return to System pri */
			SetPhysPri(HIGHPRI);
#endif
			if( rxid->rc == MultiWaiting )
			{
					SaveState **statep = (SaveState **)rxid->state;

					if( *statep != NULL )
					{
						Resume(*statep);
						*statep = NULL;
						rxid->rc = Err_Null;
					}
			}
			else Resume(rxid->state);
		}
	}
	else	/* surrogate */
	{
#ifdef LINKIO
		ProtoPrefix protohdr = Proto_Msg;
		LinkInfo *link = root->Links[pte->Link];
#ifdef __C40
		word bytealign = 0;
#endif

		if( (link->Mode == Link_Mode_Dumb) ||
		    (link->State != Link_State_Running) ) return Err_BadLink;

		id.mcb->MsgHdr.Dest = (Port)pte->TxId;

		if( (id.mcb->MsgHdr.Flags & MsgHdr_Flags_preserve) == 0 &&
		     pte->Type != T_Permanent ) _FreePort(dest);

		/* we do not touch the PTE after this point */
		
		id.next = NULL; id.tail = &id;

		if( Forever(id.mcb->Timeout) ) id.endtime = -1;
		else id.endtime = AddTimes(id.mcb->Timeout,Timer());

		/* if the link is in use, queue for it		*/
		if( link->TxUser != NULL )
		{
			if( link->TxQueue == NULL ) link->TxQueue = &id;
			else {
				link->TxQueue->tail->next = &id;
				link->TxQueue->tail = &id;
			}
			Suspend(&id.state, THREAD_LINKWRITEQ);

			if( id.rc != Err_Null ) goto done;
			/* wakeup having reached head of queue	*/
		}
#ifdef __TRAN
		else link->TxUser = &id;
#else
		else {
			link->TxUser = &id;
			/* note our savestate so that we can ensure it */
			/* is removed by EvictLinks() and not EvictRunQs() */
			/* if its task is killed */
			id.state = GetExecRoot()->CurrentSaveArea;
		}
#endif
		/* we now have control over the Tx channel */

		id.endtime = AddTimes(link->Timeout,Timer());

		/* send protocol type byte */
		LinkTx(sizeof(ProtoPrefix), link, &protohdr);

		if( id.rc == Err_Timeout ) goto linkdone;

#if defined(__C40) || defined(__ARM)
		/* This fix allows us to send messages down a half duplex */
		/* link without having to worry about blocking the link */
		/* while the IO Server is trying to send us some info. */
		/* The full story is very involved... (you dont want to know) */
		/* In the case of the ARM, it stops the serial receive buffer */
		/* from overflowing, allowing the I/O Server to read each */
		/* message before the next arrives. */

		if (link->Flags & Link_Flags_HalfDuplex) {
			/* Wait until timed out or signaled. If we are */
			/* signaled, it is because the link guardian has */
			/* received the Proto_Go message from the I/O Server */
			if (!_TimedWait(&link->HalfDuplex, link->Timeout) ||
			    id.rc == Err_Timeout ) {
					id.rc = Err_Timeout;
					goto linkdone;
			}
		}
#endif
		/* now message header 					*/
		/* from here on, we must ensure we complete the tfr	*/
		/* so only abandon it if we see a timeout.		*/

#ifdef __C40
		if (id.mcb->MsgHdr.Flags & MsgHdr_Flags_bytealign) {
			/* If byte align flags are already set then this must */
			/* be a through routed message. In this case we */
			/* should preserve the existing byte alignment info. */
			/* *BEWARE* As a small optimisation, we know that */
			/* the control and data vectors are contigous in this */
			/* case and therefore send them out in one write. */

			word 	msgsize = (word)id.mcb->MsgHdr.ContSize * sizeof(word) +
			(((word)id.mcb->MsgHdr.Flags & MsgHdr_Flags_bytealign) +
			(word)id.mcb->MsgHdr.DataSize + 3) & ~3;

			LinkTx(sizeof(MsgHdr),link,&id.mcb->MsgHdr);
			if( id.rc == Err_Timeout ) goto linkdone;

			if (msgsize) {
				LinkTx(msgsize, link, id.mcb->Control);	
				if( id.rc == Err_Timeout ) goto linkdone;
			}

			/* the output appears to have worked, set state to */
			/* running */
			link->State = Link_State_Running;
			link->MsgsOut += sizeof(MsgHdr) + msgsize;

			goto linkdone;
		}

		/* If we reach this point, then the message is either not */
		/* through routed, or through routed but word aligned */
		if (dsize) {
			/* Byte align and MsgHdr byte align flags note the */
			/* alignment of the start of data - the whole first */
			/* word is transferred */

			bytealign = (word)id.mcb->Data & MsgHdr_Flags_bytealign;
			id.mcb->MsgHdr.Flags |= (byte)bytealign;
		}
#endif

		LinkTx(sizeof(MsgHdr),link,&id.mcb->MsgHdr);	
		if( id.rc == Err_Timeout ) goto linkdone;

		if (csize) {
			LinkTx(csize,link,id.mcb->Control);
			if( id.rc == Err_Timeout ) goto linkdone;
		}

		if (dsize) {
#ifdef __C40
			/* round up to a 4 byte multiple */
			dsize = (bytealign + dsize + 3) & ~3;

			/* transfer a word multiple from a word boundary */
			LinkTx(dsize, link, id.mcb->Data - bytealign);
#else
			LinkTx(dsize, link, id.mcb->Data);
#endif
			if( id.rc == Err_Timeout ) goto linkdone;
		}

		/* the output appears to have worked, set state to running */
		link->State = Link_State_Running;
		link->MsgsOut += sizeof(MsgHdr)+csize+dsize;

	linkdone:
		id.mcb->MsgHdr.Dest = dest;
 
		/* start next process waiting for link */
		if( link->TxQueue != NULL )
		{
			Id *succ = link->TxQueue;
			link->TxQueue = succ->next;
			if( succ->next != NULL ) succ->next->tail = succ->tail;
			Resume(succ->state);
			link->TxUser = succ;
		}
		else link->TxUser = NULL;
#endif
		if( id.rc == Err_Kill ) StopProcess();				
	}

done:
	return id.rc;
}

/*--------------------------------------------------------
-- PutReady						--
--							--
-- Test a port for readiness. Returns Err_Null if a	--
-- PutMsg on this port would complete immediately.	--
--							--
--------------------------------------------------------*/

Code _PutReady(Port port)
{
	RootStruct *root = GetRoot();
	PTE *pte = GetPTE( port, root );
	
	if( pte->Type == T_Free || NotSameCycle_(port,pte) ) return Err_BadPort;
	
	pte->Age = 1;
	
	if( pte->Type == T_Local )
	{
		/* if there is already a transmitter, or no	*/
		/* receiver: fail				*/
		if( pte->TxId != NULL || pte->RxId == NULL ) return Err_NotReady;
	}
	
	/* in all other cases, ok				*/
	return Err_Null;
}

Code PutReady(Port port)
{
	return (Code)System(_PutReady,port);
}

/*--------------------------------------------------------
-- Exception						--
--							--
-- Send an exception message to a given port.		--
-- This is invoked both as a straight procedure and as	--
-- a kernel worker process.				--
-- SendException is the external interface.		--
--							--
--------------------------------------------------------*/

void Exception(Code code, Port port)
{
	MCB mcb;
	word e;

	if( port == NullPort ) return;
	
	*(word *)&mcb = 0;		/* zero all of first word */
	
	mcb.MsgHdr.Flags = MsgHdr_Flags_exception;
	mcb.MsgHdr.Dest = port;
	mcb.MsgHdr.Reply = NullPort;
	mcb.MsgHdr.FnRc = code;
	mcb.Timeout = -1;		/* wait for ever !! */
	
	e = _PutMsg(&mcb);
}

void _SendException(Port port, Code code)
{
#ifdef __TRAN
  RootStruct *	root = GetRoot();	/* XXX - don't ask! */
#endif
  
	word e = _PutReady(port);
	
	/* if the port is ready, deliver exception synchronously */
	if( e == Err_Null ) 
	{
		Exception(code,port);
		return;
	}
	
	/* if error is anything other that NotReady dont try */
	if( e != Err_NotReady ) return;

	/* as a last resort, deliver exception asynchronously */
	NewWorker(Exception,code,port);
}

void SendException(Port port, Code code)
{
	System((WordFnPtr)_SendException,port,code);
}

/*--------------------------------------------------------
-- _XchMsg						--
--							--
-- Exchange a message pair with some other process.	--
-- This routine will retry in the face of recoverable	--
-- errors but will give up on harder errors.		--
-- It may be called either with two seperate MCBs or	--
-- with both pointers the same, or with one NULL.	--
-- 							--
-- This has been added for future compatibility with	--
-- Version 2 Helios which will use this for most Kernel	--
-- operations.						--
--							--
--------------------------------------------------------*/

Code _XchMsg(MCB *txmcb, MCB *rxmcb)
{
	word e;
	Port txport;
	Port rxport;
	
	txport = txmcb->MsgHdr.Dest;

	/* If rxmcb is NULL, use txmcb for both PutMsg and GetMsg	*/
	if( rxmcb == NULL ) rxmcb = txmcb;

	/* If we only have one MCB, use the reply port for GetMsg.	*/
	/* If we have two MCBs, use the dest port of rxmcb if it is not	*/
	/* null.							*/
	if( rxmcb == txmcb || rxmcb->MsgHdr.Dest == NullPort ) 
		rxport = txmcb->MsgHdr.Reply;
	else	rxport = rxmcb->MsgHdr.Dest;

	do
	{
		txmcb->MsgHdr.Dest = txport;
		
		e = _PutMsg(txmcb);

		while( e == Err_Null ) 
		{
			rxmcb->MsgHdr.Dest = rxport;
			
			e = _GetMsg(rxmcb);
			
			/* if the return code is EG_CallBack then the	*/
			/* EO field contains a time in seconds. We must	*/
			/* wait that time and then retry. We wait with	*/
			/* GetMsg on the reply port, so any Aborts etc.	*/
			/* will be seen.				*/
			if( (e&(EC_Mask|EG_Mask)) == (EC_Recover|EG_CallBack) )
			{
				word timeout = rxmcb->Timeout;
				rxmcb->Timeout = (e&EO_Mask) * OneSec;
				e = _GetMsg(rxmcb);
				rxmcb->Timeout = timeout;
				break;
			}
			elif( (e&(EC_Mask|EG_Mask)) == (EC_Recover|EG_NewTimeout) )
			{
				rxmcb->Timeout = (e&EO_Mask) * OneSec;
				e = Err_Null; /* to satisfy while condition */
				continue;
			}
			break;
		}
		/* BLV - major change. XchMsg() must not cope with	*/
		/* timeouts. This requires action at a higher level,	*/
		/* increasing the retry count and hence decrementing	*/
		/* the confidence in the server. Otherwise if a server	*/
		/* stops responding this routine will loop forever,	*/
		/* sending one message after another, with no way out.	*/
	} while((e != EK_Timeout) && ((e&EC_Mask) == EC_Recover ));
	
	return e;
}

#ifndef __TRAN /* tran has high speed assembler jacket */
Code XchMsg(MCB *txmcb, MCB *rxmcb)
{
	return System(_XchMsg, txmcb, rxmcb);
}
#endif



/* -- End of putmsg.c */
