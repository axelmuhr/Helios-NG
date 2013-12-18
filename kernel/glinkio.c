/*
 * File:	glinkio.c
 * Subsystem:	Generic Helios executive
 * Author:	P.A.Beskeen
 * Date:	Nov '91
 *
 * Description: Implements the basic IPC link interface functions
 *
 * RcsId: $Id: glinkio.c,v 1.13 1993/08/11 09:55:39 nickc Exp $
 *
 * (C) Copyright 1991 Perihelion Software Ltd.
 * 
 * RcsLog: $Log: glinkio.c,v $
 * Revision 1.13  1993/08/11  09:55:39  nickc
 * added ARM support
 *
 * Revision 1.12  1992/12/17  11:43:27  paul
 *  added fixup for lost end of Tx DMA interrupt.
 * If AbortLinkTx detects completed DMA then it donesn't time it out, it
 * simply re-schedules the thread
 *
 * Revision 1.11  1992/12/03  16:48:09  paul
 * changed names of all WP_ functions to MP_
 *
 * Revision 1.10  1992/11/23  10:12:37  paul
 * converted all link tx/rx call to take word addresses and word size arguments
 *
 * Revision 1.9  1992/11/12  17:04:21  paul
 * rm global bus C40 debugging
 *
 * Revision 1.8  1992/09/21  10:37:11  paul
 * moved the generic executive from the C40 directory to the main kernel
 * dir, so that it can be shared with the ARM implementation
 *
 * Revision 1.7  1992/09/18  18:10:31  paul
 * removed some code if not debug version
 *
 * Revision 1.6  1992/09/17  16:10:28  paul
 * fixed stupid error for special link functions
 *
 * Revision 1.5  1992/08/04  18:02:39  paul
 * updated dbg a bit
 *
 * Revision 1.4  1992/06/24  19:01:02  paul
 * remove cc warning
 *
 * Revision 1.3  1992/06/22  08:13:41  paul
 * minor tidy's
 *
 * Revision 1.2  1992/05/14  10:47:12  paul
 * fixed for correct C40 fifo sizes
 *
 * Revision 1.1  1992/04/21  09:54:56  paul
 * Initial revision
 *
 *
 * N.B. The ALLOCDMA code could be deleted to simplify the source.
 */

/* Include Files: */

#include "kernel.h"
#include <link.h>

#if defined(__C40) && defined(SYSDEB) && defined(GLOBDBG)
#define GNEXT	0x80000000
#define GEND	GNEXT + 4 * 1024 * 1024

void DumpComms(word size, LinkInfo *link, void *buf) {
	word gbuf;

	gbuf = GRead(GNEXT);

	if (((word)buf & 3) != 0) {
		_Trace(0xdedded99, (word)buf, 0);
		JTAGHalt();
	}

	if ((size & 3) != 0) {
		_Trace(0xdedded88, size, 0);
		JTAGHalt();
	}

	size >>= 2;

	if ((gbuf + size + 3) >= GEND)
		gbuf = GNEXT + 2;

	GWrite(gbuf++, 0xdeaddead);
	GWrite(gbuf++, link->Id);
	GWrite(gbuf++, size << 2);
	GWrite(gbuf++, (word)buf);
	GWriteB(gbuf, size, (word *)buf);

	GWrite(gbuf+size, (word)gbuf);	/* back pointer */

	gbuf += size + 1;

#if 0
	GWrite(gbuf++, 0x0badbad0);
	GWriteB(gbuf, 3, ((word *)buf)+size);
	gbuf += 3;
#endif

	GWrite(GNEXT, gbuf);
}
#endif

/* InitCommsLinks()
 *
 * Ready on-chip communication links for initial data transfers via
 * _LinkTx/Rx and LinkTx/Rx() functions. This includes setting up interrupt
 * handlers for the links.
 */
void InitCommsLinks(void) {
#ifdef __C40
	int i;
# ifdef LINKDMA
	for (i = 0; i < COMMSLINKS; i++)
		/* init DMA engines, processor control regs and */
		/* end of DMA interrupt vectors */
		InitLinkDMA(0x01000a0 + ((word)i * 16));
# else
	for (i = 0; i < COMMSLINKS; i++)
		/* init interrupt vectors */
		InitLink(i);
# endif
#elif __ARM
	int i;

	if (COMMSLINKS == 0)
		return;

	for (i = 0; i < COMMSLINKS; i++)
		/* init interrupt vectors */
		InitLink(i);
#else
# error	Implementation specific code required for InitCommsLinks()
#endif

	return;
}


/*
 * _ELinkTx() / _ELinkRx()
 *
 * Link transmission and reception for non on-chip communications hardware
 * (add-on comms).
 *								
 * If the Tx/RxFunction fields are clear, then the transfers take place as
 * normal, down the on-chip links indicated by the Tx/RxChan fields. If
 * these pointers are not null then the other fields are used thus:
 * 	     LinkInfo_DCB      (TxChan): DCB of device		
 *	     LinkInfo_ModTab   (RxChan): module table pointer	
 *
 * The transfer routines are supplied in the link structure rather than
 * being explicitly coded here so that the link functions can be 
 * dynamically updated. These functions will need to access routines
 * external to the kernel, so must be called with the supplied module
 * table pointer. The info structure is used to pass the transfer parameters
 * as CallWithModTab() cannot handle enouhgh parameters. A point to note is
 * that a tranfer size of zero indicates an AbortLinkTx/Rx operation for
 * that communications link.
 */

#ifdef __C40
void _ELinkTx(word size, LinkInfo *link, MPtr buf) {
#else
void _ELinkTx(word size, LinkInfo *link, void *buf) {
#endif
	LinkTransferInfo info;

	info.size = size;
	info.link = link;
	info.buf  = buf;

	CallWithModTab((word)&info, 0, link->TxFunction,
		(word *)link->LinkInfo_ModTab);
}


#ifdef __C40
void _ELinkRx(word size, LinkInfo *link, MPtr buf) {
#else
void _ELinkRx(word size, LinkInfo *link, void *buf) {
#endif
	LinkTransferInfo info;

	info.size = size;
	info.link = link;
	info.buf  = buf;

	CallWithModTab((word)&info, 0, link->RxFunction, 
		 (word *)link->LinkInfo_ModTab);
}



/* AbortLinkTx/Rx()
 *
 * These functions are usually called by the port TimeoutHandler() to abort a
 * tranfer that has not completed within its allotted timeout. The transfer
 * should be aborted immediately, and the link left in a state such that
 * another LinkTx/Rx operation will work normally (if the other side is kosha).
 *
#ifdef ALLOCDMA
 * A further complication for the 'C40 version is that a LinkTx/Rx() thread
 * may be waiting for a DMA engine. This waiting also needs to be aborted.
 * The responsibilty for returning resources such as DMA engines is left to
 * the original LinkTx/Rx() routines, after they have detected that they have
 * been timedout.
#endif
 *
 * If no transfer is in progress then return NULL, else halt the transfer and
 * return the blocked thread's SaveState *.
 *
 * The higher levels of the link code ensure that only one abort thread for
 * either Rx or Tx side of a link can be active at one time.
 */

SaveState *AbortLinkTx(LinkInfo *link)
{
#if 0
	KDebug(("AbortLinkTx %d\n", link->Id));
#endif
	if (link->TxFunction != 0) {
		LinkTransferInfo info;

		/* Abort add-on comms transfer. This is achieved by calling */
		/* the normal TxFunction() with a size of zero to invoke the */
		/* abort functionality. */
		info.size = 0;
		info.link = link;
		/* buf field not used */

		return (SaveState *)CallWithModTab((word)&info, 0, 
				(WordFnPtr)link->TxFunction, 
				(word *)link->LinkInfo_ModTab);
	}

	/* Abort link Tx.
  	 *
	 * If an interrupt driven transfer is underway, halt it and
	 * return the LinkTx thread.
 	 */

#ifndef __C40
	/* Stop any further link output interrupts occuring.
	 * If they are not occuring anyway, then no harm has been done.
	 */

	_AbortLinkTx(link);

	{
		SaveState *	as = link->TxThread;

		/* Note no activity now on link by NULLing Thread pointer. */
		/* If Thread was already NULL, then no harm has been done. */
		link->TxThread = NULL;

		return as;
	}

#else
	/* C40 specific code.
	 *
	 * @@@ This could probably be optimised to reduce its interrupt
	 * latency. The prototype for _AbortLinkRx/Tx should also be
	 * changed to the same as the generic case (i.e. remove Id arg.).
	 */

	/* If DMA or interrupt driven transfer is underway, halt it and */
	/* return the LinkTx thread */
	IntsOff();	/* guard against intr. occuring at this point */
	if (link->TxThread != NULL) {
		SaveState *as = link->TxThread;

		/* On the C40 halt the primary (Tx) half of the split mode */
		/* DMA engine */

		/* Fix for lost end of Tx DMA interrupts. */
		/* See comments in c40linkio.a. */
		if (_AbortLinkTx(link, link->Id) == FALSE)
			as = NULL;

		link->TxThread = NULL;
		IntsOn();
		return as;
	}
	IntsOn();

# ifdef ALLOCDMA
	/* If LinkTx is currently queued to get a DMA engine, then find it, */
	/* de-Q it and return its savestate to the caller. */
	/* @@@ could do with a way of knowing if it is queued, rather than */
	/* looking through the entire Q - However, the Q won't be very big */
	/* and this function is called very rarely. */
	{
		RootStruct *root = GetRoot();
		DMAReq *DMARequest = root->DMAReqQhead;
		DMAReq *prev = (DMAReq *)&root->DMAReqQhead;

		while (DMARequest != NULL) {
			if (link->Id == DMARequest->link && \
			    DMARequest->flags == DMA_Tx) {

				/* remove request from Q */
				if ((prev->next = DMARequest->next) == NULL)
					root->DMAReqQtail = prev;

				/* note timeout abort */
				DMARequest->rc = -1;

				/* return savestate to caller */
				return DMARequest->state;
			}
			prev = DMARequest;
			DMARequest = DMARequest->next;
		}
	}
# endif
 	return NULL;
#endif /* __C40 */
}


SaveState *AbortLinkRx(LinkInfo *link)
{
#if 0
	KDebug(("AbortLinkRx %d\n", link->Id));
#endif
	if (link->RxFunction != 0) {
		LinkTransferInfo info;

		/* Abort add-on comms transfer. This is achieved by calling */
		/* the normal RxFunction() with a size of zero to invoke the */
		/* abort functionality. */
		info.size = 0;
		info.link = link;
		/* buf field not used */

		return (SaveState *)CallWithModTab((word)&info, 0, 
				(WordFnPtr)link->RxFunction, 
				(word *)link->LinkInfo_ModTab);
	}


	/* Abort link Rx.
  	 *
	 * If an interrupt driven transfer is underway, halt it and
	 * return the LinkRx thread.
 	 */

#ifndef __C40
	/* Stop any further link input interrupts occuring.
	 * If they are not occuring anyway, then no harm has been done.
	 */

	_AbortLinkRx(link);

	{
		SaveState * as = link->RxThread;

		/* Note no activity now on link by NULLing Thread pointer. */
		/* If Thread was already NULL, then no harm has been done. */
		link->RxThread = NULL;

		return as;
	}

#else
	/* C40 specific code.
	 *
	 * @@@ This could probably be optimised to reduce its interrupt
	 * latency.
	 */

	IntsOff();	/* guard against intr. occuring at this point */
	if (link->RxThread != NULL) {
		SaveState *as = link->RxThread;

		/* on the C40 halt the primary (Rx) half of the split mode */
		/* DMA engine */
		_AbortLinkRx(link, link->Id);

		link->RxThread = NULL;
		IntsOn();
		return as;
	}
	IntsOn();

# ifdef ALLOCDMA
	/* If LinkRx is currently queued to get a DMA engine, then find it, */
	/* de-Q it and return its savestate to the caller. */
	/* @@@ could do with a way of knowing if it is queued, rather than */
	/* looking through the entire Q - However, the Q won't be very big */
	/* and this function is called very rarely. */
	{
		RootStruct *root = GetRoot();
		DMAReq *DMARequest = root->DMAReqQhead;
		DMAReq *prev = (DMAReq *)&root->DMAReqQhead;

		while (DMARequest != NULL) {
			if (link->Id == DMARequest->link && \
			    DMARequest->flags == DMA_Rx) {

				/* remove request from Q */
				if ((prev->next = DMARequest->next) == NULL)
					root->DMAReqQtail = prev;

				/* note timeout abort */
				DMARequest->rc = -1;

				/* return savestate to caller */
				return DMARequest->state;
			}
			prev = DMARequest;
			DMARequest = DMARequest->next;
		}
	}
# endif

	return NULL;
#endif
}


/* ResetLinks()
 *
 * Reset all on-chip links, canceling all current transfers.
 * This is only called in multiprocessor systems when a link guardian has
 * recieved a Terminate request. As doom is impending, we should not worry about
 * interrupt latency at this point.
 *
 * N.B. C40 variant cannot remove words already placed in its fifo's.
 */

void ResetLinks(void) {
	LinkInfo **li = GetRoot()->Links;

	IntsOff();
		/* abort all existing transfers */
		while (*li != NULL) {
			AbortLinkTx(*li);
			AbortLinkRx(*li);
		}

		/* reset any remaining parts of the link hardware */
		ResetLinkHardware();
	IntsOn();
}


#ifndef __ARM	/* ARM directly implements LinkTx/Rx() without a wrapper. */
		/* The C40 aught to do this too, to reduce interrupt latency */
		/* and improve performance. */

/* LinkTx() / LinkRx()
 *
 * Send/Receive some data down a link without blocking. The process is
 * descheduled until the transfer completes. The transfer can be aborted via
 * the port TimeoutHandler() and AbortLinkTx/Rx(), in which case the link
 * tranfer is halted at that point and any dynamically allocated resources
 * such as DMA engines are free'd up.
 *
 * The companion _LinkTx() / _LinkRx() functions are designed to implement
 * simple blocking transfers and are usually only used in the initial kernel
 * startup to get the configuration info.
 *
#ifdef ALLOCDMA
 * The C40's implementations use of the DMA engines in split mode complicates
 * the Tx/Rx routines somewhat. If the one side already has a DMA engine,
 * then we share that engine. When we complete, we should free the engine
 * unless the other side is using it. The use of a DMA engine by either
 * side is indicated by the use of the link->Tx/RxThread field. This is
 * NULLified when that side is no longer using the DMA engine. If both sides
 * are trying to allocate an engine for that link, both sides will succeed at
 * the same time, and will be given the same engine. All the associated link
 * functions are called at high priority, safeguarding the integrity of the
 * DMA and link structures.
#else
 * Each link is pre-allocated the same numbered DMA engine, so that it may
 * syncronise its transfers with that DMA engine - only same numbered link/dma
 * engines can syncronise their READY/NOTREADY signals.
#endif
 */

void LinkTx(word size, LinkInfo *link, void *buf) {
#if defined(__C40) && defined(ALLOCDMA)
	RootStruct *root = GetRoot();
	word DMAEng;
#endif

	/* if link is being used for add-on comms, use external fn */
	if (link->TxFunction) {
#ifdef __C40
		_ELinkTx(size >> 2, link, C40WordAddress(buf));
#else
		_ELinkTx(size, link, buf);
#endif
		return;
	}

#ifdef __C40
	/* if _LinkTx will not block, send data by hand */
	/* link FIFO's are 32 bytes in size */
	if (size <= 32 && size <= TxFIFOSpace(link->TxChan)) {
		_LinkTx(size >> 2, link->TxChan, C40WordAddress(buf));
		return;
	}
#endif

#if defined(__C40) && defined(ALLOCDMA)
	/* Larger transfer are accomplished by DMA. DMA engines are allocated */
	/* dynamically so we can also perform thru-route transfers and user */
	/* allocation of the engines. */

	/* DMA engines are usually used in split mode, so if we have already */
	/* allocated one for Rx, then use this one for Tx too */
	if (link->RxThread != NULL) {
		DMAEng = link->DMAEng;
	} else {
		/* Allocate a DMA engine from the free list */
		/* timeout can only be caused by the port TimeoutHandler */
		/* checking the Link->TxUser timeout */
		if ((DMAEng = _AllocDMA(root, -1, link->Id, DMA_Tx)) < 0)
			return;		/* timeout on request */

		link->DMAEng = DMAEng;
	}

	/* Disable interrupts globally. Interrupts will be re-enabled */
	/* automatically when the thread returns from Suspend() */
	/* Interrupts are disabled to stop a quick DMA ending before we */
	/* return from LinkDMATx and have noted our SaveState pointer in */
	/* the link struct */
 	IntsOff();

	/* A DMA engine is now allocated to us. Set it up for link transfer */
	/* in split mode, install end-of-DMA interrupt handler and start DMA */
	/* going. */
	LinkDMATx(DMAEng, link->Id, size, buf);

	/* Interrupt handler will restart our thread when the transfer is */
	/* complete. AbortLinkTx() may also cause us to return. */
	Suspend(&link->TxThread, THREAD_LINKTX);

	/* The interrupt handler has forced link->RxThread = NULL to note */
	/* that the link (Aux channel of split mode DMA engine) is no longer */
	/* in use */

	/* if Rx (Auxillary channel) side no longer using DMA engine, */
	/* then free it up */
	if (link->RxThread == NULL)
		_FreeDMA(root, DMAEng);

#else
 	IntsOff();

	/* Setup and start a non blocking link transfer */
	/* On the C40 this sets up and starts a transfer using the links */
	/* associated DMA engine. When the transfer is complete the thread */
	/* pointed to in the link structures TxThread field will be resumed */
	/* __LinkTx disables interrupts globally. Interrupts will be */
	/* re-enabled automatically when the thread returns from Suspend() */
	/* Interrupts are disabled to stop a quick DMA ending before we */
	/* return from __LinkTx and have noted our SaveState pointer in */
	/* the link struct */
#ifdef __C40
	__LinkTx(link, link->Id, size >> 2, C40WordAddress(buf));
#else
	__LinkTx(link, link->Id, size, buf);
#endif

	/* Interrupt handler will restart our thread when the transfer is */
	/* complete. AbortLinkTx() may also cause us to return. */
	Suspend(&link->TxThread, THREAD_LINKTX);

	/* The interrupt handler has forced link->RxThread = NULL to note */
	/* that the link (Aux channel of split mode DMA engine) is no longer */
	/* in use */
#endif
}


void LinkRx(word size, LinkInfo *link, void *buf) {
#if defined(__C40) && defined(ALLOCDMA)
	RootStruct *root = GetRoot();
	word DMAEng;
#endif

	/* If link is being used for add-on comms, use external fn. */
	if (link->RxFunction) {
#ifdef __C40
		_ELinkRx(size >> 2, link, C40WordAddress(buf));
#else
		_ELinkRx(size, link, buf);
#endif
		return;
	}

#ifdef __C40
	/* if _LinkRx will not block, get data by hand */
	/* link FIFO's are 32 bytes in size */
	if (size <= 32 && size <= RxFIFOSpace(link->RxChan)) {
		_LinkRx(size >> 2, link->RxChan, C40WordAddress(buf));
		return;
	}
#endif

#if defined(__C40) && defined(ALLOCDMA)
	/* Larger transfer are accomplished by DMA. DMA engines are allocated */
	/* dynamically so we can also perform thru-route transfers and user */
	/* allocation of the engines. */

	/* DMA engines are usually used in split mode, so if we have already */
	/* allocated one for Tx, then use this one for Rx too */
	if (link->TxThread != NULL) {
		DMAEng = link->DMAEng;
	} else {
		/* Allocate a DMA engine from the free list */
		/* timeout can only be caused by the port TimeoutHandler */
		/* checking the Link->RxUser timeout */
		if ((DMAEng = _AllocDMA(root, -1, link->Id, DMA_Rx)) < 0)
			return;		/* timeout on request */

		link->DMAEng = DMAEng;
	}

	/* Disable interrupts globally. Interrupts will be re-enabled */
	/* automatically when the thread returns from Suspend() */
	/* Interrupts are disabled to stop a quick DMA ending before we */
	/* return from LinkDMARx and have noted our SaveState pointer in */
	/* the link struct */
	IntsOff();

	/* A DMA engine is now allocated to us. Set it up for link transfer */
	/* in split mode, install end-of-DMA interrupt handler and start DMA */
	/* going. */
	LinkDMARx(DMAEng, link->Id, size, buf);

	/* Interrupt handler will restart our thread when the transfer is */
	/* complete. AbortLinkRx() may also cause us to return. */
	Suspend(&link->RxThread, THREAD_LINKRX);

	/* The interrupt handler has forced link->RxThread = NULL to note */
	/* that the link (Aux channel of split mode DMA engine) is no longer */
	/* in use */

	/* if Tx (Primary channel) side no longer using DMA engine, */
	/* then free it up */
	if (link->TxThread == NULL)
		_FreeDMA(root, DMAEng);
#else
	/* Setup and start a non blocking link transfer */
	/* On the C40 this sets up and starts a transfer using the links */
	/* associated DMA engine. When the transfer is complete the thread */
	/* pointed to in the link structures RxThread field will be resumed */
	/* __LinkRx disables interrupts globally. Interrupts will be */
	/* re-enabled automatically when the thread returns from Suspend() */
	/* Interrupts are disabled to stop a quick DMA ending before we */
	/* return from __LinkRx and have noted our SaveState pointer in */
	/* the link struct */

#ifdef __C40
	__LinkRx(link, link->Id, size >> 2, C40WordAddress(buf));
#else
	__LinkRx(link, link->Id, size, buf);
#endif

	Suspend(&link->RxThread, THREAD_LINKRX);

#if defined(__C40) && defined(SYSDEB) && defined(GLOBDBG)
	GTrace(0xdead1111);
	DumpComms(size, link, buf);
#endif
	/* The interrupt handler has forced link->RxThread = NULL to note */
	/* that the link (Aux channel of split mode DMA engine) is no longer */
	/* in use */
#endif
}
#endif /* __C40 */

#ifdef __C40
void MP_LinkTx(word size, LinkInfo *link, MPtr buf) {

	/* if link is being used for add-on comms, use external fn */
	if (link->TxFunction) {
		_ELinkTx(size, link, buf);
		return;
	}

	/* if _LinkTx will not block, send data by hand */
	/* link FIFO's are 8 words in size */
	if (size <= 8 && size <= ((word)TxFIFOSpace(link->TxChan) >> 2)) {
		_LinkTx(size, link->TxChan, buf);
		return;
	}

 	IntsOff();

	/* Setup and start a non blocking link transfer */
	/* On the C40 this sets up and starts a transfer using the links */
	/* associated DMA engine. When the transfer is complete the thread */
	/* pointed to in the link structures TxThread field will be resumed */
	/* __LinkTx disables interrupts globally. Interrupts will be */
	/* re-enabled automatically when the thread returns from Suspend() */
	/* Interrupts are disabled to stop a quick DMA ending before we */
	/* return from __LinkTx and have noted our SaveState pointer in */
	/* the link struct */

	__LinkTx(link, link->Id, size, buf);

	/* Interrupt handler will restart our thread when the transfer is */
	/* complete. AbortLinkTx() may also cause us to return. */
	Suspend(&link->TxThread, THREAD_LINKTX);

	/* The interrupt handler has forced link->RxThread = NULL to note */
	/* that the link (Aux channel of split mode DMA engine) is no longer */
	/* in use */
}


void MP_LinkRx(word size, LinkInfo *link, MPtr buf) {

	/* if link is being used for add-on comms, use external fn */
	if (link->RxFunction) {
		_ELinkRx(size, link, buf);
		return;
	}

	/* if _LinkRx will not block, get data by hand */
	/* link FIFO's are 8 words in size */
	if (size <= 8 && size <= ((word)RxFIFOSpace(link->RxChan) >> 2)) {
		_LinkRx(size, link->RxChan, buf);
		return;
	}

	/* Setup and start a non blocking link transfer */
	/* On the C40 this sets up and starts a transfer using the links */
	/* associated DMA engine. When the transfer is complete the thread */
	/* pointed to in the link structures RxThread field will be resumed */
	/* __LinkRx disables interrupts globally. Interrupts will be */
	/* re-enabled automatically when the thread returns from Suspend() */
	/* Interrupts are disabled to stop a quick DMA ending before we */
	/* return from __LinkRx and have noted our SaveState pointer in */
	/* the link struct */

	__LinkRx(link, link->Id, size, buf);

	Suspend(&link->RxThread, THREAD_LINKRX);

	/* The interrupt handler has forced link->RxThread = NULL to note */
	/* that the link (Aux channel of split mode DMA engine) is no longer */
	/* in use */
}
#endif /* C40 */

/* end of glinkio.c */
