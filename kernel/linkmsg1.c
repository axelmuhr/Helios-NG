/*------------------------------------------------------------------------
--                                                                      --
--                      H E L I O S   K E R N E L                       --
--                      -------------------------                       --
--                                                                      --
--             Copyright (C) 1988, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- linkmsg.c								--
--                                                                      --
--	Handle delivery of a message from a link to its destination	--
--                                                                      --
--	Author:  NHG 8/8/88						--
--                                                                      --
------------------------------------------------------------------------*/
/* $Id: linkmsg1.c,v 1.34 1993/08/11 09:51:53 nickc Exp $ */

#define __in_linkmsg 1	/* flag that we are in this module */

#include "kernel.h"
#include <message.h>

#ifdef LINKIO

#ifdef __C40

#define DOUBLE_BUFFER		0	/* link -> link double buffering */

#define DOUBLE_BUFFER_PROCESS	0	/* use permanently allocated process for above */

#else

#define DOUBLE_BUFFER		1	/* link -> link double buffering */

#define DOUBLE_BUFFER_PROCESS	1	/* use permanently allocated process for above */

#endif


#ifdef __C40
#define SINGLE_BUFFER           0  	/* for now,  because it do'nt seem to work  */
#else
#define SINGLE_BUFFER		1	/* do small link->link msgs directly in LG */
#endif

#define SIMPLE_BUFFER		1	/* optimised memory usage for buffered msgs */


/* The following buffer sizes have important relationships which are	*/
/* not obvious from their values.					*/
/* For C40 systems, these values MUST always be word multiples.		*/
#define DoubleBuffSize		256
#define DoubleBuffMin		256
#define DoubleBuffTfrMin	64
#define DoubleBuffTfrMid	128
#define DoubleBuffTfrMax	DoubleBuffSize

#define SingleBuffSize		256
#define SingleBuffTfr		64

#define SimpleBuffSize		512

#if DOUBLE_BUFFER

typedef struct DBInfo {
	word		status;		/* status word			*/
	word		size;		/* data remaining in source link*/
	word		send;		/* data expected by dest link	*/
	word		tfr;		/* transfer size		*/
	LinkInfo	*source;	/* source link struct		*/
	LinkInfo	*dest;		/* dest link struct		*/
	Id		*sid;		/* source Id == source->RxUser	*/
	Id		*did;		/* dest Id == dest->TxUser	*/
	SaveState	*sync;		/* rendezvous point		*/
} DBInfo;

#define DBTxError	1
#define DBRxError	2
#define	DBError		(DBTxError|DBRxError)

void DoubleBuffer(DBInfo *bi);

#define DBRendezvous(bi)						\
	if( (bi)->sync ) { Resume((bi)->sync); (bi)->sync = NULL; }	\
	else { Suspend(&(bi)->sync, THREAD_LINKTHRU2); }
			

#endif

#if SINGLE_BUFFER

word SingleBuffer(LinkInfo *srclink, LinkInfo *destlink, MsgHdr *hdr, word msgsize);

#endif

#if SIMPLE_BUFFER

static void SimpleBuffer(LinkInfo *link, MsgHdr *hdr);

#endif

static void DeliverMsg(MsgBuf *msgbuf, LinkInfo *link);

#ifdef __C40
static void LeftShuffle(uword *src, word bytesleft, word numwords);
#endif

/* FlowControl level tests. We xoff our links when the amount of memory	*/
/* used by message buffering is more than half the available free	*/
/* memory. We switch back on again when it falls to below a quarter.	*/

#define FlowControlHiWater (root->BufferCount > root->FreePool->Size)

#define FlowControlLoWater (3*root->BufferCount < root->FreePool->Size)


/*--------------------------------------------------------
-- LinkMsg						--
--							--
-- Get a message from the link and deliver it to its	--
-- destination, or queue it.				--
--							--
--------------------------------------------------------*/

bool LinkMsg(LinkInfo *link)
{
	RootStruct	*root = GetRoot();
	MsgHdr		hdr;
	Port 		dest;
	PTE 		*pte;
	Id 		id;
	word 		result = FALSE;			/* assume failure */
	word 		e = Err_Congestion;		/* default error */
	word		msgsize;
#ifdef SYSDEB
	word reason = 0;
#define setreason(x) (reason = x);
#else
#define setreason(x)
#endif

	id.rc = Err_Null;
	id.endtime = AddTimes(link->Timeout,Timer());
	link->RxUser = &id;
	
	LinkRx(sizeof(MsgHdr),link,&hdr);

	if( id.rc != Err_Null ) goto rxfailed;

	dest = hdr.Dest;

	pte = GetPTE( dest, root );

	/* if there is a reply port, replace it with a surrogate	*/
	if( hdr.Reply != NullPort )
	{
		Port reply = _NewPort();
		PTE *rpte = GetPTE( reply, root );

		rpte->TxId = (Id *)hdr.Reply;
		rpte->Owner = pte->Owner;
		rpte->RxId = (Id *)pte->Link;

		rpte->Type = (pte->Type==T_Local)?T_Surrogate:T_Trail;
		rpte->Link = link->Id;
		reply |= (Port_Flags_Tx|Port_Flags_Remote);

		hdr.Reply = reply;
	}

	if( pte->Type == T_Free || NotSameCycle_(dest,pte) )
	{
		e = Err_BadPort; 
		setreason(1);
		goto sink; 
	}

#ifdef __C40
	/* round up to the word multiple size that will have been sent, */
	/* regardless of  the data size specified in the message header */
	msgsize = (word)hdr.ContSize * sizeof(word) +
			(((word)hdr.Flags & MsgHdr_Flags_bytealign) +
			(word)hdr.DataSize + 3) & ~3;
#else
	msgsize = (word)hdr.ContSize * sizeof(word) + hdr.DataSize;
#endif

	link->MsgsIn += sizeof(MsgHdr)+msgsize;

	pte->Age = 1;			/* prevent garbage collection */

	switch( pte->Type )
	{
	case T_Local:

		/* if there is already a transmitter on the port, or	*/
		/* there is no receiver waiting for a message, buffer it*/
		if( (pte->TxId != NULL) || (pte->RxId == NULL) ) 
		{
			/* if the message has the sacrifice bit set, 	*/
			/* don't buffer it since protocol doesn't care.	*/
			if( hdr.Flags & MsgHdr_Flags_sacrifice ) 
			{ setreason(2); goto sink; }
			else goto buffermsg;
		}
		else
		{
			Id *rxid = pte->RxId;
			MCB *rxmcb = rxid->mcb;
			word csize = (word)hdr.ContSize * sizeof(word);
#ifdef __BIGENDIAN
			word dsize = hdr.DataSize;
#else
			word dsize = (*(word *)&hdr)&0xffff;
#endif

			pte->RxId = rxid->next;
			if( rxid->next != NULL ) rxid->next->tail = rxid->tail;

			MoveBlock(&rxmcb->MsgHdr,&hdr,sizeof(MsgHdr));

#ifdef __C40
			/* clear byte align flags in case msghdr used again */
			rxmcb->MsgHdr.Flags &= ~MsgHdr_Flags_bytealign;
#endif

			link->RxId = rxid;
			
			if( csize != 0 )
			{
				LinkRx(csize,link,rxmcb->Control);
				/* we must complete tfr on all errors execpt timeout */
				if( id.rc == Err_Timeout ) goto linkdone;
			}

#ifdef __C40
			/* Helios messages may contain a non word multiple */
			/* amount of data and their src and dst do not have */
			/* to be word aligned. Unfortunately the 'C40 has no */
			/* natural support for bytes and non word multiple */
			/* tranfers. The following code works around these */
			/* limitations to support the standard Helios message */
			/* specification. */

			/* All word alignment is done at the messages */
			/* destination and only whole words are transferred. */
			/* If the front or rear of the message is not word */
			/* aligned, then the entire word the non-aligned data */
			/* is contained in is sent. This is achieved by */
			/* passing the 'bytealign' flag in the message hdr */
			/* that notes which are valid bytes in the first word */
			/* The size of data is then rounded up to a word */
			/* multiple to give the true size of the message sent */
			/* The last word may again contain some invalid bytes */
			/* padding the message to a word boundary. The true */
			/* size in the message hdr enables you to calc. which */
			/* bytes in the tail are valid. */


			/* bytealign: (X = valid byte, 0 = padding)

				0	[X,X,X,X]
				1	[0,X,X,X]
				2	[0,0,X,X]
				3	[0,0,0,X]
			*/

			if( dsize != 0 ) {
				word dummy;
				byte *dst = rxmcb->Data;
				word bytealign = ((word)hdr.Flags & MsgHdr_Flags_bytealign);
				word frontbytes;
				word worddata;
				word tailbytes;

				/* valid bytes in first word */
				if ((frontbytes = (bytealign ? 4 - bytealign : 0)) > dsize)
					frontbytes = dsize;

				/* amount of src data that is full words */
				worddata = (dsize - frontbytes) & ~3;

				/* last word has this many valid bytes */
				tailbytes = (dsize - frontbytes) & 3;

				if (bytealign) {
					/* get initial padded word */
					LinkRx(4, link, &dummy);

					/* transfer non word aligned bytes */
					/* bytes to the data */
# ifdef __BIGENDIAN
					*dst++ = (byte)(dummy >> 16);
					if (--frontbytes) {
						*dst++ = (byte)(dummy >> 8);
						if (--frontbytes)
							*dst++ = (byte)dummy;
					}
# else /* C40 */
					dummy >>= 8 * bytealign;
					while(frontbytes--) {
						*dst++ = (byte)(dummy);
						dummy >>= 8;
					}
# endif
				}


				/* Get word mult. portion of src msg into dst */
				if (worddata) {
					if ((word)dst & 3) {
						/* dst is not word aligned, */
						/* but src is now. Transfer */
						/* and shuffle data to */
						/* correct alignment. */
						word safe = worddata - 4;
						byte *aligndst = (byte *)(((word)dst + 3) & ~3);

						if (safe > 0) {
							LinkRx(safe, link, aligndst);
							/* left shuffle:    src,      # bytes to left,     numwords */
							LeftShuffle((uword *)aligndst, 4 - ((word)dst & 3), safe/4);
							dst += safe;
						}

						/* Xfer last word of word data. */
						/* We insert each byte by hand */
						/* to make sure we can't overshoot */
						/* the msg data boundary. */
						
						LinkRx(4, link, &dummy);
# ifdef __BIGENDIAN
						*dst++ = (byte)(dummy >> 24);
						*dst++ = (byte)(dummy >> 16);
						*dst++ = (byte)(dummy >> 8);
						*dst++ = (byte)dummy;
# else /* C40 */
						*dst++ = (byte)dummy;
						*dst++ = (byte)(dummy >> 8);
						*dst++ = (byte)(dummy >> 16);
						*dst++ = (byte)(dummy >> 24);
# endif
					} else  {
						/* Dst & src are word aligned */
						LinkRx(worddata, link, dst);
						dst += worddata;
					}
				}

				if (tailbytes) {
					/* get any trailing bytes that were */
					/* not part of a word multiple */

					/* get full padding word */
					LinkRx(4, link, &dummy);

					/* extract and add the valid bytes */
					/* to the data */
# ifdef __BIGENDIAN
					*dst++ = (byte)(dummy >> 24);
					if (--tailbytes) {
						*dst++ = (byte)(dummy >> 16);
						if (--tailbytes)
							*dst++ = (byte)(dummy >> 8);
					}
# else /* C40 */
					*dst++ = (byte)dummy;
					if (--tailbytes) {
						*dst++ = (byte)(dummy >> 8);
						if (--tailbytes)
							*dst++ = (byte)(dummy >> 16);
					}
# endif
				}
			}
#else /* !__C40 */
			/* links can transfer byte multiples */
			if( dsize != 0 )
				LinkRx(dsize,link,rxmcb->Data);
#endif
		linkdone:
			if( rxid->rc != Err_Kill ) 
			{
				if( rxid->rc == MultiWaiting )
				{
					SaveState **statep = (SaveState **)rxid->state;
					if( *statep != NULL )
					{
						rxid->rc = Err_Null;
						Resume(*statep);
						*statep = NULL;
					}
				}
				else Resume(rxid->state);
			}
			
			if( id.rc != Err_Null ) 
			{ e = id.rc; setreason(3); goto msglost; }
			result = TRUE; goto done;
		}

	case T_Surrogate:
	case T_Trail:
	case T_Permanent:
	{
		LinkInfo *destlink = root->Links[pte->Link];
		
		/* If the link is not in required state, sink the message */
		/* and report a warning, this should eventually cause a	  */
		/* re-route search to happen.				  */
		if( destlink->Mode != Link_Mode_Intelligent ||
		    destlink->State != Link_State_Running)
		{ 
			e = Err_BadRoute; 
			setreason(4);
			goto sink; 
		}
#if DOUBLE_BUFFER || SINGLE_BUFFER
		/* Only if the link is free, can we transfer data directly,*/
		/* only do it if there is enough data to make it pay.	   */
		/* We seem to have problems here if either link is an IO   */
		/* processor. (Probably PC only)			   */
		
		if( destlink->TxUser != NULL 		||
		    destlink->TxQueue != NULL 		||
		    link->Flags & Link_Flags_ioproc 	||
		    destlink->Flags & Link_Flags_ioproc  ) goto buffermsg;
#endif

#if SINGLE_BUFFER
		/* if the message is < SingleBuffSize we can buffer it on our	   */
		/* stack and transfer it simply.			   */
		if( msgsize <= SingleBuffSize )
		{
			Id did;

			/* set did up as current TxUser of outward link	*/
			did.rc = Err_Null;
			destlink->TxUser = &did;
			link->RxId = &id;
			
			/* install port we are surrogate for		*/
			hdr.Dest = (Port)pte->TxId;
			
			result = SingleBuffer(link,destlink,&hdr,msgsize);

			if( (hdr.Flags & MsgHdr_Flags_preserve) == 0 &&
			    pte->Type != T_Permanent )
				_FreePort(dest);

			_SignalLink(destlink);	/* wake up any waiters */

			destlink->MsgsOut += sizeof(MsgHdr)+msgsize;

			goto done;
		}
#endif /* SINGLE_BUFFER */

#if DOUBLE_BUFFER
		if( msgsize > DoubleBuffMin )
		{
#if DOUBLE_BUFFER_PROCESS
			DBInfo *bi = destlink->DBInfo;
			Id *did = bi->did;
			word protohdr = Proto_Msg;

			destlink->TxUser = did;
			link->RxId = &id;

			bi->size = msgsize;
			bi->send = msgsize;
			bi->source = link;
			bi->sid = &id;
			
			/* Set transfer size to maximise the parallelism*/
			/* in the transfer.				*/
			if( msgsize <= DoubleBuffTfrMin*8 ) bi->tfr = DoubleBuffTfrMin;
			elif( msgsize <= DoubleBuffTfrMid*8 ) bi->tfr = DoubleBuffTfrMid;
			else bi->tfr = DoubleBuffTfrMax;
			
			/* Rendezvous with other process to get it going */
			DBRendezvous(bi);

			/* Rendezvous with process and let it go ahead	*/
			/* and read the first chunk from the source link*/
			DBRendezvous(bi);

			/* install port we are surrogate for		*/
			hdr.Dest = (Port)pte->TxId;

			did->endtime = AddTimes(destlink->Timeout,Timer());
			
			/* send link protocol header byte		*/
			LinkTx(sizeof(ProtoPrefix),destlink,&protohdr);
			if( did->rc != Err_Null ) goto dberror;

#if defined(__C40) || defined(__ARM)
			/* Wait until timed out or signaled. If we are */
			/* signaled, it is because the link guardian has */
			/* received the Proto_Go message from the I/O Server */
			if (destlink->Flags & Link_Flags_HalfDuplex) {
				if (!TimedWait(&destlink->HalfDuplex, destlink->Timeout) ||
					did->rc == Err_Timeout ) {
						did->rc = Err_Timeout;
						goto dberror;
				}
			}
#endif

			/* send message header				*/
			LinkTx(sizeof(MsgHdr),destlink,&hdr);
			if( did->rc != Err_Null ) goto dberror;

			/* call double buffer to partner other invocation */
			DoubleBuffer(bi);

			if( (hdr.Flags & MsgHdr_Flags_preserve) == 0 &&
			    pte->Type != T_Permanent )
				_FreePort(dest);

			result = TRUE;
		dbdone:
			_SignalLink(destlink);	/* wake up any waiters */

			destlink->MsgsOut += sizeof(MsgHdr)+msgsize;

			goto done;

			/* if we failed to forward the header, come here*/
			/* to wake up double buffer process		*/
		dberror:
			bi->status |= DBTxError;
			DBRendezvous(bi);
			result = FALSE; goto dbdone;

#else /* DOUBLE_BUFFER_PROCESS */
			Id did;
			word protohdr = Proto_Msg;
			DBInfo bi;

			/* set did up as current TxUser of outward link	*/
			did.rc = Err_Null;
			did.endtime = -1;
			destlink->TxUser = &did;

			bi.status = 0;
			bi.source = link;
			bi.dest = destlink;
			bi.size = msgsize;
			bi.send = msgsize;
			bi.sid = &id;
			bi.did = &did;
			bi.sync = NULL;

			/* Set transfer size to maximise the parallelism*/
			/* in the transfer.				*/
			if( msgsize <= DoubleBuffTfrMin*8 ) bi.tfr = DoubleBuffTfrMin;
			elif( msgsize <= DoubleBuffTfrMid*8 ) bi.tfr = DoubleBuffTfrMid;
			else bi.tfr = DoubleBuffTfrMax;
			
			/* get other process receiving message while we	*/
			/* send header					*/
			if( !NewWorker(DoubleBuffer,&bi) ) goto buffermsg;
			
			/* Rendezvous with process and let it go ahead	*/
			/* and read the first chunk from the source link*/
			DBRendezvous(&bi);

			/* install port we are surrogate for		*/
			hdr.Dest = (Port)pte->TxId;

			did.endtime = AddTimes(destlink->Timeout,Timer());
			
			/* send link protocol header byte		*/
			LinkTx(sizeof(ProtoPrefix),destlink,&protohdr);

			if( did.rc == Err_Timeout ) goto dberror;

#if defined(__C40) || defined(__ARM)
			/* Wait until timed out or signaled. If we are */
			/* signaled, it is because the link guardian has */
			/* received the Proto_Go message from the I/O Server */
			if (destlink->Flags & Link_Flags_HalfDuplex) {
				if (!TimedWait(destlink->HalfDuplex, destlink->Timeout) ||
					did.rc == Err_Timeout ) {
						did.rc = Err_Timeout;
						goto dberror;
				}
			}
#endif
			/* send message header				*/
			LinkTx(sizeof(MsgHdr),destlink,&hdr);

			if( did.rc != Err_Null ) goto dberror;

			/* call double buffer to partner other invocation */
			DoubleBuffer(&bi);

			if( (hdr.Flags & MsgHdr_Flags_preserve) == 0 &&
			    pte->Type != T_Permanent )
				_FreePort(dest);

			result = TRUE;
		dbdone:
			_SignalLink(destlink);	/* wake up any waiters */

			destlink->MsgsOut += sizeof(MsgHdr)+msgsize;
			
			goto done;

			/* if we failed to forward the header, come here*/
			/* to wake up double buffer process		*/
		dberror:
			bi.status |= DBTxError;
			DBRendezvous(&bi);
			result = FALSE; goto dbdone;

#endif /* DOUBLE_BUFFER_PROCESS */

		}
		
#endif /* DOUBLE_BUFFER */
	}

		/* this is a general destination for when we must buffer*/
		/* the message for later delivery.			*/
	buffermsg:
		{
#if SIMPLE_BUFFER
			if( msgsize <= SimpleBuffSize )
			{
				/* start buffer process	*/
				if( !NewWorker(SimpleBuffer,link,&hdr) )
				{ setreason(5); goto sink; }
					
				/* wait for process to collect msg &	*/
				/* restart us.				*/

				Suspend(&id.state, THREAD_LINKTHRU1);

				result = TRUE;			
			}
			else
#endif /* SIMPLE_BUFFER */
			{
				MsgBuf *msgbuf = (MsgBuf *)GetBuf(msgsize+MsgBuf_Overhead);

				if( msgbuf == NULL ) { setreason(6); goto sink;}

				msgbuf->MCB.MsgHdr = hdr;
			
				if( msgsize != 0 )
				{
					LinkRx(msgsize,link,msgbuf->Msg);
					msgbuf->MCB.Control = msgbuf->Msg;
					msgbuf->MCB.Data = (byte *)&(msgbuf->Msg[hdr.ContSize]);
					if( id.rc != Err_Null ) {
						FreeBuf((Buffer *)msgbuf);
						e=id.rc; setreason(7);
						goto msglost;
					}
				}

				if( !NewWorker(DeliverMsg,msgbuf,link) ) 
				{
					FreeBuf((Buffer *)msgbuf);
					link->MsgsLost++;
#ifdef SYSDEB
					_Trace(0xaaaa0011,*(word *)&hdr,hdr.Dest);
					_Trace(hdr.Reply,hdr.FnRc,e);
#endif
				}
				else root->BufferedMsgs += sizeof(MsgHdr)+msgsize;
			}

#ifdef FLOWCONTROL
			if( FlowControlHiWater ) 
					NewWorker(FlowControl,Proto_Xoff);
#endif	
			result = TRUE; goto done;
		}
	}

sink:
	/* we want to dispose of the remainder of this message	*/
	/* do this carefully because we may not have much memory*/
	/* in which to do this.					*/
	{
		word i;
		word junk;

		for( i = hdr.ContSize; i != 0 ; i-- )
		{
			LinkRx(4,link,&junk);
			if( id.rc == Err_Timeout ) { setreason(8); goto msglost; }
		}
#ifdef __C40
		/* read in word chunks */
		i = (((uword)hdr.DataSize) +
			((word)hdr.Flags & MsgHdr_Flags_bytealign) + 3) & ~3;

		for( ; i != 0 ; i -= 4 )
		{
			LinkRx(4,link,&junk);
			if( id.rc == Err_Timeout ) { setreason(9); goto msglost; }
		}
#else
		for( i = hdr.DataSize; i != 0 ; i-- )
		{
			LinkRx(1,link,&junk);
			if( id.rc == Err_Timeout ) { setreason(9); goto msglost; }
		}
#endif
	}

	result = TRUE;	/* result indicates state of link, which is ok here */
	
msglost:
	/* come here if the message has been lost, generate an 	*/
	/* exception message if necessary.			*/
	if(hdr.Reply != NullPort && (hdr.Flags & MsgHdr_Flags_exception) == 0)
		NewWorker(Exception,e,hdr.Reply);

	/* count lost messages, but not badly routed ones */
	if( e != Err_BadPort ) link->MsgsLost++;
#ifdef SYSDEB
	if( e != Err_BadPort )
	{
	_Trace(0xaaaa2200|reason,*(word *)&hdr,hdr.Dest);
	_Trace(hdr.Reply,hdr.FnRc,e);
	}
#endif
rxfailed:
done:
	link->RxUser = NULL;

	if( link->Sync != NULL ) 
	{
		Resume(link->Sync->state);
		link->Sync = NULL;
	}

	link->RxId = NULL;

	return result;
}

/*--------------------------------------------------------
-- DeliverMCB						--
--							--
-- Deliver a buffered message to its destination. Used	--
-- by both DeliverMsg to deliver a message in a MsgBuf	--
-- and by SimpleBuffer to deliver a message buffered	--
-- on its own stack.					--
--							--
--------------------------------------------------------*/

static void DeliverMCB(MCB *mcb, LinkInfo *link)
{
	Code e;
	RootStruct *root = GetRoot();
	PTE *pte = GetPTE(mcb->MsgHdr.Dest, root );

	/* Through-routed messages are kept permanently until	*/
	/* they can be sent. Messages buffered for a local port	*/
	/* are kept for DeliveryTime before being destroyed.	*/
	/* All processes should be eager-readers, so if the	*/
	/* message is not received with reasonable speed it is	*/
	/* the users own fault.					*/
	
	mcb->Timeout = pte->Type==T_Local?DeliveryTime:-1;

	e = _PutMsg(mcb);

	/* generate an exception reply on error, but only to 	*/
	/* non-exception messages				*/
	if( e != Err_Null )
	{
		if( (e&(EG_Mask|EO_Mask)) != (EG_Exception|EE_Abort) ) 
		{
				link->MsgsLost++;
#ifdef SYSDEB
				_Trace(0xaaaa0033,*(word *)&mcb->MsgHdr,mcb->MsgHdr.Dest);
				_Trace(mcb->MsgHdr.Reply,mcb->MsgHdr.FnRc,e);
#endif
		}

		if( mcb->MsgHdr.Reply != NullPort && 
		   (mcb->MsgHdr.Flags & MsgHdr_Flags_exception) == 0)
			Exception(e,mcb->MsgHdr.Reply);
	}

#ifdef FLOWCONTROL
	if( (root->Flags & Root_Flags_xoffed) && FlowControlLoWater ) 
		NewWorker(FlowControl,Proto_Xon);
#endif
}


/*--------------------------------------------------------
-- DeliverMsg						--
--							--
-- Deliver a message buffer to its dest.		--
--							--
--------------------------------------------------------*/

static void DeliverMsg(MsgBuf *msgbuf, LinkInfo *link)
{
	DeliverMCB(&msgbuf->MCB, link);

	FreeBuf((Buffer *)msgbuf);
}

#if DOUBLE_BUFFER

/*--------------------------------------------------------
-- DoubleBuffer						--
--							--
-- Cooperate with another process to double buffer a 	--
-- message from one link to another.			--
--							--
--------------------------------------------------------*/

void DoubleBuffer(DBInfo *bi1)
{
	byte buf[DoubleBuffSize];
	DBInfo *bi = bi1;		/* optimisation */
	LinkInfo *sl = bi->source;
	LinkInfo *dl = bi->dest;
	Id *sid = bi->sid;
	Id *did = bi->did;
	word size;
	word tfr = bi->tfr;

#ifdef __TRAN
	RootStruct *	root = GetRoot();	/* XXX don't ask! */
#endif
	
	do
	{
		/* rendezvous with partner */
		DBRendezvous(bi);

		if( bi->status & DBError ) break;
		
		/* receive data from link */
		size = bi->size;
		if( size == 0 ) break;
		if( size > tfr ) size = tfr;

		sid->endtime = AddTimes(sl->Timeout,Timer());
			
		LinkRx(size,sl,&buf);
		if( sid->rc == Err_Timeout ) { bi->status |= DBRxError; break; }
		bi->size -= size;

		/* if the partner has seen a Tx error already, quit here */
		if( bi->status & DBTxError ) break;

		/* rendezvous with partner again */
		DBRendezvous(bi);

		if( bi->status & DBTxError ) break;
		
		/* and send data to link */
		did->endtime = AddTimes(dl->Timeout,Timer());

		LinkTx(size,dl,&buf);
		if( did->rc != Err_Null ) { bi->status |= DBTxError; break; }
		bi->send -= size;
		
		if( bi->status & DBError ) break;

	} while( bi->send != 0 );

	/* force a final sync before exiting */
	DBRendezvous(bi);
}

#if DOUBLE_BUFFER_PROCESS

/*--------------------------------------------------------
-- DoubleBufferProcess					--
--							--
-- Permanently allocated double buffer process to use	--
-- during link-link transfers. Note that the process	--
-- attached to the destination link is used.		--
--							--
--------------------------------------------------------*/

void DoubleBufferProcess(LinkInfo *link)
{
	Id did;
	DBInfo bi;
	
	bi.dest = link;
	bi.did = &did;
	bi.sync = NULL;
	
	link->DBInfo = &bi;
	
	forever
	{
		did.rc = Err_Null;
		did.endtime = -1;
		bi.status = 0;

		DBRendezvous(&bi);

		DoubleBuffer(&bi);
	}		
}

void InitDoubleBufferProcess(LinkInfo *link)
{
	NewWorker(DoubleBufferProcess,link);
}

#endif /* DOUBLE_BUFFER_PROCESS */

#else

/* Called by LinkInit so must exist whatever the buffering technique */
void InitDoubleBufferProcess(LinkInfo *link)
{
	link=link;
}

#endif /* DOUBLE_BUFFER */

#if SINGLE_BUFFER

/*--------------------------------------------------------
-- SingleBuffer						--
--							--
-- Called by the link guardian when the message is less --
-- than SingleBuffSize, this function gets the message	--
-- directly into an on-stack buffer and re-transmits	--
-- it through the destination link. If the message is	--
-- large enough it is split into chunks to increase	--
-- inter-processor parallelism.				--
--							--
--------------------------------------------------------*/

word SingleBuffer(LinkInfo *srclink, LinkInfo *destlink, MsgHdr *hdr, word msgsize)
{
	byte buf[SingleBuffSize];
	Id *sid = srclink->RxUser;
	Id *did = destlink->TxUser;
	word protohdr = Proto_Msg;
	word tfr;
	
	did->endtime = AddTimes(destlink->Timeout,Timer());

	sid->rc = Err_Null;
	sid->endtime = AddTimes(srclink->Timeout,Timer());

	/* send link protocol header byte		*/
	LinkTx(sizeof(ProtoPrefix),destlink,&protohdr);

	if( did->rc != Err_Null ) return FALSE;

#if defined(__C40) || defined(__ARM)
	/* This fix allows us to send messages down a half duplex */
	/* link without having to worry about blocking the link */
	/* while the IO Server is trying to send us some info. */
	/* The full story is very involved... (you dont want to know) */

	if (destlink->Flags & Link_Flags_HalfDuplex) {
		/* Wait until timed out or signaled. If we are */
		/* signaled, it is because the link guardian has */
		/* received the Proto_Go message from the I/O Server */
		if (!TimedWait(&destlink->HalfDuplex, destlink->Timeout) ||
			did->rc == Err_Null ) {
			return FALSE;
		}
	}
#endif

	/* send message header				*/
	LinkTx(sizeof(MsgHdr),destlink,hdr);

	if( did->rc != Err_Null ) return FALSE;

	while( msgsize != 0 )
	{
		tfr = msgsize;
		if( tfr > SingleBuffTfr ) tfr = SingleBuffTfr;
		
		/* get message from source			*/
		LinkRx(tfr,srclink,buf);

		if( sid->rc != Err_Null ) return FALSE;
			
		/* send it to dest				*/
		LinkTx(tfr,destlink,buf);
		if( did->rc != Err_Null ) return FALSE;
		
		msgsize -= tfr;
	}

	return TRUE;
}

#endif

#if SIMPLE_BUFFER

/*--------------------------------------------------------
-- SimpleBuffer						--
--							--
-- A process forked by the link guardian when a message	--
-- to be buffered is less than SimpleBuffSize. The	--
-- message is buffered in an on-stack buffer rather	--
-- than in a seperately allocated MsgBuf as is the case	--
-- with DeliverMsg.					--
--							--
--------------------------------------------------------*/

static void SimpleBuffer(LinkInfo *link, MsgHdr *hdr)
{
	byte buf[SimpleBuffSize];
	Id id;
	MCB mcb;
	Id *lgid = link->RxUser;
#ifdef __C40
	/* round up to the word multiple size that will have been sent, */
	/* regardless of  the data size specified in the message header */
	word msgsize = (word)hdr->ContSize * sizeof(word) +
			(((word)hdr->Flags & MsgHdr_Flags_bytealign) +
			(word)hdr->DataSize + 3) & ~3;
#else
	word msgsize = (word)hdr->ContSize * sizeof(word) + hdr->DataSize;
#endif
	mcb.MsgHdr = *hdr;
	id.rc = Err_Null;
		
	/* get rest of message if it exists		*/
	if( msgsize != 0 )
	{
		/* replace link guardian Id with ours		*/
		id.endtime = AddTimes(link->Timeout,Timer());
		link->RxUser = &id;

		LinkRx(msgsize,link,buf);

		link->RxUser = NULL;	

		if( id.rc == Err_Null )
		{	
			mcb.Control = (word *)buf;
			mcb.Data = &(buf[mcb.MsgHdr.ContSize*sizeof(word)]);
		}
	}
	
	/* Re-awaken Link Guardian */

	while( lgid->state == NULL ) { Yield(); }
	Resume(lgid->state);
	lgid->state = NULL;
	
	/* Finally call DeliverMCB to wait at the port	*/
	/* and transfer the message when ready		*/

	if( id.rc == Err_Null ) 
	{
		GetRoot()->BufferedMsgs += sizeof(MsgHdr)+msgsize;
		DeliverMCB(&mcb,link);
	}
}

#endif

#endif /* LINKIO */


#ifdef __C40
/* @@@ to handle non word aligned data faster, this function should be coded */
/* in assembler */

/* Shuffle the the word aligned source data, to a byte aligned destination. */
static void LeftShuffle(uword *src, word bytesleft, word numwords)
{
#ifdef __BIGENDIAN
# if 0 /* alternatively... */
  memcpy( ((byte *)src) - bytesleft, src, numwords * sizeof (long) );
# endif
	/* NICKS_SLOW_BUT_WORKING_GENERIC_VERSION */
	byte *	source = (byte *)src;	
	byte *	dest   = source - bytesleft;
	word	i;
	
	for (i = numwords * sizeof (long); i--;)
	  {
	    *dest++ = *source++;
	  }
#else
	word	rshift = bytesleft * 8;
	uword	mask   = (~0UL) >> rshift;
	word	lshift = 32 - rshift;
	uword	src1;
	uword	src2;

	src--;	/* treat src as destination */

	src1 = *src & mask;
	
	while (numwords--) {
	    src2 = *(src + 1);

	    *src = src1 | (src2 << lshift);
	    src1 = src2 >> rshift;

	    ++src;
	}

	*src = src1;
#endif
}
#endif


/* -- End of linkmsg.c */
