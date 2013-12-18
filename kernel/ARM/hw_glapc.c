/*
 * File:	hw_glapc.c
 * Subsystem:	Helios-ARM executive
 * Author:	P.A.Beskeen
 * Date:	Nov '92
 *
 * Description: C based Gnome link adapter functions. The comms link numbers
 *		used by Helios are used by this implementation to refer to a
 *		podule slot, each of which might contain a Gnome link adapter.
 *
 *
 * RcsId: $Id: hw_glapc.c,v 1.1 1993/08/12 08:19:47 nickc Exp $
 *
 * (C) Copyright 1992 Perihelion Software Ltd.
 * All Rights Reserved.
 *
 * RcsLog: $Log: hw_glapc.c,v $
 * Revision 1.1  1993/08/12  08:19:47  nickc
 * fixed compile time warnings
 *
 *
 *
 */

#include "../kernel.h"

#include "hw_glap.h"


#ifndef __ARCHIMEDES
# error "This source file should only be used for Archimedes systems"
#endif


/*
 * void InitLink(word channel)
 *
 * Sets up input and output interrupt handlers for given link channel. If a
 * podule for this link number is not present, then the function simply
 * returns.
 *
 * channel == channel number == podule number.
 */

void InitLink(int ln) {
	xcb_id		*slotid = XCB_ID(ln);
	ioc_regs	*ioc = IOC;
	glap_regs	*linka;
	int i;

	if (slotid->product_lo != XCB_PRODUCT_TRANSPUTERLINKADAPTER)
		return;

	/* Link adapter reset - this disables link tx and rx interrupts. */
	/* When de-asserting reset, set the link speed */
	linka = GLAP_LinkAdapter(ln);

	/* Assert reset for at least 8 * 5Mhz = 8 * 0.2uS = 1.6uS */
	for (i = 0; i < 1000; i++)
		linka->control_status = GLAP_ChipReset;

#if 1
	/* 0 = set speed at 10Mhz */
	linka->control_status = 0; /* GLAP_LinkSpeed10Mhz;*/
#else
	linka->control_status = GLAP_LinkSpeed20Mhz;
#endif

	/* Enable expansion bus interrupts. */
	ioc->irq_b.mask |= IRQB_XCB;
}


/*
 * void InitLink2(LinkInfo l)
 *
 * This function is called after the link hardware initialisation function
 * InitLink(), during the kernel link guardian initialisation.
 *
 * It is used to set the Tx/Rx channel numbers in link structure that are
 * passed to _/__LinkTx/Rx functions to identify the comms hardware for that
 * particular operation.
 *
 * If one of the channels is found to have no link comms hardware attached,
 * then the link is set to an unconnected state.
 *
 */

void InitLink2(LinkInfo *l) {
	int id = l->Id;

	if (l->Id >= 4 || \
	    XCB_ID(id)->product_lo != XCB_PRODUCT_TRANSPUTERLINKADAPTER) {
		/* Unknown podule type so set link to unconnected. */
		l->Mode = Link_Mode_Null;
		l->State = Link_State_Null;

		l->RxChan = l->TxChan = 0;

		return;
	}

	l->RxChan = l->TxChan = (Channel)GLAP_LinkAdapter(id);
}


/*
 * void ResetLinkHardware(void);
 *
 * This is called if another processor requests this one to terminate.
 * It is only useful on multiprocessor systems.
 *
 * Reset all on-chip links, canceling any current transfers
 * AbortLinkTx/Rx has already been called to cancel any outstanding transfers.
 *
 */

void ResetLinkHardware(void) {
	xcb_id		*slotid;
	glap_regs	*linka;
	int i, j;

	for (j = 0; j < COMMSLINKS; j++) {
		slotid = XCB_ID(j);

		if (slotid->product_lo != XCB_PRODUCT_TRANSPUTERLINKADAPTER)
			return;

		/* Link adapter reset - this disables link tx and rx */
		/* interrupts. When de-asserting reset, sets the link speed. */
		linka = GLAP_LinkAdapter(j);

		/* Assert reset for at least 8 * 5Mhz = 8 * 0.2uS = 1.6uS */
		for (i = 0; i < 1000; i++)
			linka->control_status = GLAP_ChipReset;
#if 1
		linka->control_status = 0; /* GLAP_LinkSpeed10Mhz*/
#else
		linka->control_status = GLAP_LinkSpeed20Mhz;
#endif
	}
}


/*
 * _LinkTx()
 *
 * Simple blocking link reception function. This is used to send a bogus Info
 * message when debugging early versions of the kernel and can be used for
 * initial system debugging in concert with KDebug() and -D KERNELDEBUG2.
 *
 * size: number of bytes to send
 * link: ptr the link hardware
 * buf: ptr to buffer holding data
 *
 */

void _LinkTx(word size, Channel link, void *buf)
{
	char *buff = (char *)buf;
	glap_regs *linka = (glap_regs *)link;

	while (size) {
		if (linka->output_status & GLAP_OutputReady) {
			linka->write_data = *buff++;
			size--;
		}
	}
}


/*
 * _LinkRx()
 *
 * Simple blocking link reception function. This is used to get config vector
 * in kernel startup.
 *
 * size: number of bytes to read
 * link: ptr the link hardware
 * buf: ptr to buffer to hold the data
 *
 */

void _LinkRx(word size, Channel link, void *buf)
{
	char		*buff  = (char *)buf;
	glap_regs	*linka = (glap_regs *)link;

	while (size) {
		if (linka->input_status & GLAP_InputReady) {
			*buff++ = linka->read_data;
			size--;
		}
	}
}


/*
 * LinkTx
 *
 * Sets up and starts a non-blocking transfer to a communications link.
 *
 * First try to throw some bytes down the link by polling, if this will
 * take too long, then setup and interrupt driven transfer and suspend
 * until it completes.
 *
 * @@@ This could be optimised by allowing a number of polls before deciding
 * to setup the interrupt transfer.
 *
 */

void LinkTx(word size, LinkInfo *link, void *buf)
{
	glap_regs *	linka;
	char *		buff = (char *)buf;

	/* If link is being used for add-on comms, use external fn. */
	if (link->TxFunction) {
		_ELinkTx(size, link, buff);
		return;
	}

	linka = (glap_regs *)link->TxChan;

	while (size && (linka->output_status & GLAP_OutputReady)) {
		linka->write_data = *buff++;
		size--;
	}

	if (size != 0) {
		LinkReq	*	intr_info = 
					&(GetExecRoot()->LinkOutStat[link->Id]);

		/* Set info to be passed to interrupt handler */
		intr_info->Count = size;
		intr_info->Buf = buff;

		/* Guard against link ready interrupts happening before we */
		/* are successfully Suspend()ed. */
		DisableIRQ();

		/* Enable link adapter output ready interrupts */
		linka->output_status = GLAP_WriteIntrEnable;
		
		/* Interrupt handler will restart our thread when the */
		/* transfer is complete. AbortLinkTx() may also cause us to */
		/* return. */
		Suspend(&link->TxThread, THREAD_LINKTX);

		/* We will return with interrupts enabled again */
	}
}


/*
 * LinkRx
 *
 * Sets up and starts a non-blocking transfer from a communications link.
 *
 * First try to grab some bytes from the link by polling, if this will
 * take too long, then setup and interrupt driven transfer and suspend
 * until it completes.
 *
 * @@@ This could be optimised by allowing a number of polls before deciding
 * to setup the interrupt transfer.
 *
 */

void LinkRx(word size, LinkInfo *link, void *buf)
{
	glap_regs *	linka;
	char *		buff = (char *)buf;

	/* If link is being used for add-on comms, use external fn. */
	if (link->RxFunction) {
		_ELinkRx(size, link, buff);
		return;
	}

	linka = (glap_regs *)link->RxChan;

	while (size && (linka->input_status & GLAP_InputReady)) {
		*buff++ = linka->read_data;
		size--;
	}

	if (size != 0) {
		LinkReq	*	intr_info = 
					&(GetExecRoot()->LinkInStat[link->Id]);

		/* Set info to be passed to interrupt handler */
		intr_info->Count = size;
		intr_info->Buf = buff;

		/* Guard against link ready interrupts happening before we */
		/* are successfully Suspend()ed. */
		DisableIRQ();

		/* Enable link adapter input ready interrupts */
		linka->input_status = GLAP_ReadIntrEnable;
		
		/* Interrupt handler will restart our thread when the */
		/* transfer is complete. AbortLinkRx() may also cause us to */
		/* return. */
		Suspend(&link->RxThread, THREAD_LINKRX);

		/* We will return with interrupts enabled again */
	}
}



#ifdef EXAMPLE_LINK_COMMS_INTERRUPT_HANDLER

/* Example Link based Communications Interrupt handler.
 *
 * This code interfaces with the above LinkRx/Tx functions to provide the
 * non-blocking parts of the message transmission.
 *
 * It is usually coded in assembler for maximum efficiency. Internal interrupt
 * handlers need to be in assembler as they call ExternCheckIRQ and
 * ExternContinueThread in system handler directly. The run Q's are always
 * checked after User handlers have been run, so these do not need to
 * note if they re-schedule any threads.
 *
 * User level LinkTx/Rx code should first examine the cards in each slot and
 * install handlers such as this onto any xcb pseudo interrupt vector that
 * contains a valid link adapter card. The handler will only be called if this
 * card has asserted IRQ. To improve performance, the handler data could be
 * used to hold invariant data pointers, like the link adapter register address.
 */

#ifdef InternalInterruptHandler

int LinkIntrHandler(int cardsyncaddress, int cardslotnum)
{

#else

int LinkIntrHandler(void *handlerdata, int pseudovector)
{
	int cardslotnum = pseudovector - INTR_XCB_0;
#endif
	
	ExecRoot *	xroot = GetExecRoot();
	glap_regs *	linka = GLAP_LinkAdapter(cardslotnum);
	LinkReq *	linkreq;
	char *		buff;
	int		size;
#ifdef InternalInterruptHandler
	int		CheckDispatchFlag = FALSE;
#endif

	/* If this slot has a valid READ interrupt asserted. */
	if (linka->input_status == (GLAP_ReadIntrEnable | GLAP_InputReady)) {
		linkreq = &xroot->LinkInStat[cardslotnum];
		size = linkreq->Count;
		buff = linkreq->Buf;

		/* Receive as many bytes as possible. */
		do {
			*buff++ = linka->read_data;
			size--;
		} while (size && linka->input_status & GLAP_InputReady);

		if (size == 0) {
			/* End of block reception, re-schedule LinkRx thread. */
			SaveState **ssp =
				&xroot->KernelRoot->Links[cardslotnum]->TxThread;
			SaveState *	ss = *ssp;
			int		pri = ss->priority;
			ThreadQ *	tq = &xroot->Queues[pri];

			/* When Playing with run Qs we disable all */
			/* interrupts - IRQ's are already disabled. */
			DisableFIQ();

				/* Add thread to its pri's run Q. */
				tq->tail = tq->tail->next = ss;	
				ss->next = NULL;

				/* Clear thread so an AbortRx cannot */
				/* re-schedule it. */
				*ssp = NULL;

			EnableFIQ();

			/* Keep exec hint up to date. */
			if (pri < xroot->HighestAvailPri)
				xroot->HighestAvailPri = pri;

			ss->status = THREAD_RUNNABLE;

#ifdef InternalInterruptHandler
			/* Note we have a dispatch possibility. */
			CheckDispatchFlag = TRUE;
#endif

			/* Stop any further link input interrupts occuring. */
			linka->input_status = 0;
		} else {
			/* More bytes to come, so save updated buffer pointer */
			/* and size for next interrupt to use. */
			linkreq->Count = size;
			linkreq->Buf = buff;
		}
	}

	/* If this slot has a valid WRITE interrupt. */
	if (linka->output_status == (GLAP_WriteIntrEnable | GLAP_OutputReady)) {
		linkreq = &xroot->LinkOutStat[cardslotnum];
		size = linkreq->Count;
		buff = linkreq->Buf;

		/* Send as many bytes as possible. */
		do {
			linka->write_data = *buff++;
			size--;
		} while (size && linka->output_status & GLAP_OutputReady);

		if (size == 0) {
			/* End of block send, re-schedule LinkTx thread. */

			SaveState **ssp =
				&xroot->KernelRoot->Links[cardslotnum]->TxThread;
			SaveState *	ss = *ssp;
			int		pri = ss->priority;
			ThreadQ *	tq = &xroot->Queues[pri];

			/* When Playing with run Qs we disable all */
			/* interrupts - IRQ's are already disabled. */
			FIQDisable();
				/* Add thread to its pri's run Q. */
				tq->tail = tq->tail->next = ss;
				ss->next = NULL;

				/* Keep exec hint up to date. */
				if (pri < xroot->HighestAvailPri)
					xroot->HighestAvailPri = pri;
			FIQEnable();

			/* Clear thread so an AbortTx cannot */
			/* re-schedule it. */
			*ssp = NULL;

			ss->status = THREAD_RUNNABLE;

#ifdef InternalInterruptHandler
			/* Note we have a dispatch possibility. */
			CheckDispatchFlag = TRUE;
#endif

			/* Stop any further Link output interrupts occuring. */
			linka->output_status = 0;
		} else {
			/* More bytes to come, so save updated buffer pointer */
			/* and size for next interrupt to use. */
			linkreq->Count = size;
			linkreq->Buf = buff;
		}
	}
#ifdef InternalInterruptHandler
	/* Short cut Dispatch check if we have not re-scheduled any threads. */
	if (CheckDispatchFlag)
		/* As thread has been resumed, see if we need to slice to it. */
		ExternCheckIRQDispatch();
	else
		/* Otherwise, simply continue current thread. */
		ExternContinueThread();
#else
	/* Note we recognised the interrupt source. */
	return TRUE;
#endif
}
#endif /* EXAMPLE_LINK_COMMS_INTERRUPT_HANDLER */



/* _AbortLinkTx/Rx()
 *
 * These functions are usually called by the port TimeoutHandler() to abort a
 * tranfer that has not completed within its allotted timeout. The transfer
 * should be aborted immediately, and the link left in a state such that
 * another LinkTx/Rx operation will work normally (if the other side is kosha).
 *
 * A higher level AbortLinkTx/Rx() takes care of NULLing the link->Tx/RxThread
 * pointer to note that the link is no longer in use.
 */

void _AbortLinkTx(LinkInfo *link)
{
	/* Stop any further link output interrupts occuring.
	 * If they are not occuring anyway, then no harm has been done.
	 */
	((glap_regs *)link->TxChan)->output_status = 0;
}


void _AbortLinkRx(LinkInfo *link)
{
	/* Stop any further link input interrupts occuring. */
	/* If they are not occuring anyway, then no harm has been done. */
	((glap_regs *)link->RxChan)->input_status = 0;
}



/* End of hw_glapc.c */
