/*
 * File:	c40dma.c
 * Subsystem:	'C40 Helios executive
 * Author:	P.A.Beskeen
 * Date:	Feb '92
 *
 * Description: Implements the DMA initialisation, allocation and free fuctions.
 *
 * RcsId: $Id: c40dma.c,v 1.1 1992/04/21 09:54:56 paul Exp $
 *
 * (C) Copyright 1992 Perihelion Software Ltd.
 * 
 * RcsLog: $Log: c40dma.c,v $
 * Revision 1.1  1992/04/21  09:54:56  paul
 * Initial revision
 *
 *
 */

/* Include Files: */

#include "../kernel.h"
#include <root.h>

/* InitDMA()
 * Initialise DMA engine tables and pointers in the root structure.
 *
 */

void InitDMA(void) {

#ifdef ALLOCDMA
	word i;
	DMAFree	*prev = NULL;
	RootStruct *root = GetRoot();

	for (i=0; i < DMAENGS; i++) {
		/* pre-compute the DMA control reg. address for this channel */
		root->DMAFreeQ[i].DMAEng = 0x1000a0 + (i * 16);

		/* add to the allocation chain */
		root->DMAFreeQ[i].next = prev;
		prev = &root->DMAFreeQ[i];
	}
	root->DMAFreeQhead = prev;	/* head of alloc list */

	/* initialise empty DMA request Q */
	root->DMAReqQhead = NULL;
	root->DMAReqQtail = (DMAReq *)&root->DMAReqQhead;
#endif

}


#ifdef ALLOCDMA
/* _AllocDMA()
 *
 * Allocate a DMA engine to the caller. The caller is expected to free up the
 * engine immediately after they have finished using it via a FreeDMA() call.
 * The timeout can be either 0, meaning only allocate engine if there is one
 * available NOW, -1, meaning block until DMA engine becomes available, or any
 * positive number of microseconds to wait for until an engine to become free.
 * The resolution of the timeout is however the same as that of Put/GetMsg()
 * i.e. it is re-evaluated every second in the port TimeoutHandler(). The link
 * and flags parameters are used for split mode Rx/Tx DMA channel allocations.
 * We assume that there will only be one Tx and one Rx DMA allocation request
 * for the same channel at one time and allocate the same engine to both.
 * The DMA_Tx/Rx flag and link parameters are also used by AbortTx/Rx() to
 * determine which request to abort. Note that these parameters are only passed
 * by the LinkTx/Rx() routines, not by calls from user code.
 * _AllocDMA() should always be called at high priority to ensure that the
 * integrity of the DMA structures is maintained.
 *
 * Returns a WPTR to a DMA control register or -1 for a failed request.
 */

word _AllocDMA(RootStruct *root, word timeout, word link, word flags) {

	if (root->DMAFreeQhead != NULL) {
		/* allocate from the free Q */
		word DMAEng = root->DMAFreeQhead->DMAEng;

		/* remove engine from free Q */
		root->DMAFreeQhead = root->DMAFreeQhead->next;

		return DMAEng;
	}

	if (timeout == 0)
		return -1;	/* failed to allocate engine */

	/* request cannot be immediatly satisfied, so Queue it up */
	{
		DMAReq	DMARequest;

		DMARequest.next = NULL;
		DMARequest.endtime = AddTimes(timeout, Timer());
		DMARequest.link = link;
		DMARequest.flags = flags;

		/* add to tail of Q */
		root->DMAReqQtail = root->DMAReqQtail->next = &DMARequest;

		/* Suspend until we either timeout or get an engine allocated */
		Suspend(&DMARequest.state, THREAD_DMAREQ);

		return DMARequest.rc;
	}
}


/* user callable DMA engine allocator */

word AllocDMA(word timeout) {
	return System(_AllocDMA, GetRoot(), timeout, -1, 0);
}


/* _FreeDMA()
 *
 * Free a previously allocated DMA engine.
 *
 * If there are outstanding requests queued up for the free'd engine, give
 * the top most request the engine. Otherwise add to the DMA engine free Q.
 * Split mode DMA users MUST only free up their engine after both users have
 * finished using the engine.
 */

void _FreeDMA(RootStruct *root, word DMAEng) {

	/* if we have an outstanding DMA engine request, fulfill it */
	if (root->DMAReqQhead != NULL) {
		DMAReq	*DMARequest = root->DMAReqQhead;

		/* Unlink first request from DMA request Q */
		if ((root->DMAReqQhead = DMARequest->next) == NULL)
			root->DMAReqQtail = (DMAReq *)&root->DMAReqQhead;

		/* Give the request this engine */
		DMARequest->rc = DMAEng;
		Resume(DMARequest->state);

		/* Look for Tx/Rx partner requiring DMA for the same link */
		/* as both can share engine in split mode */
		if (DMARequest->next != NULL && DMARequest->link != -1) {
			word reqlink = DMARequest->link;

			/* Get/PutMsg() constrain link ownership to one Tx */
			/* and one Rx at a time. Therefore if we find another */
			/* request for the same link we should grant it also. */

			do {
				DMARequest = DMARequest->next;
				if (DMARequest->link == reqlink) {
					Resume(DMARequest->state);
					return;
				}
			} while (DMARequest->next != NULL);
		}
		return;
	}

	/* The free Q is a table of DMAFree structures linked via a singly */
	/* linked list for fast allocation. */

	/* Return the DMA engine to the free list */
	{
		/* convert DMA engine control reg address to channel number */
		word	index = (DMAEng - 0x1000a0) >> 4;

		/* add at head of Q */
		root->DMAFreeQ[index].next = root->DMAFreeQhead;
		root->DMAFreeQhead = &root->DMAFreeQ[index];
	}
}


/* user callable FreeDMA() function */

void FreeDMA(word DMAEng) {
	System((WordFnPtr)_FreeDMA, GetRoot(), DMAEng);
}

#endif /* ALLOCDMA */


/* end of c40dma.c */
