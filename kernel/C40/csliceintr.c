/*
 * File:	csliceintr.c
 * Subsystem:	Generic Helios executive
 * Author:	P.A.Beskeen
 * Date:	Nov '91
 *
 * Description: Dummy C implementation of the executives Time Slice handler.
 *		This called directly by some form of clock interrupt.
 *		It assumes that it is called every millisecond and when called
 *		that interrupts are globally disabled.
 *
 * RcsId: $Id: csliceintr.c,v 1.3 1992/09/22 13:29:26 paul Exp $
 *
 * (C) Copyright 1991 Perihelion Software Ltd.
 * 
 * RcsLog: $Log: csliceintr.c,v $
 * Revision 1.3  1992/09/22  13:29:26  paul
 * renamed ExecRoot() to GetExecRoot()
 *
 * Revision 1.2  1992/04/21  09:54:56  paul
 * alpha version
 *
 * Revision 1.1  1991/12/03  11:50:52  paul
 * Initial revision
 *
 *
 */

/* Include Files: */

#include "gexec.h"


void SliceIntrHandler(void)
{
	ExecRoot	*xroot = GetExecRoot();
	SaveState	*ss = xroot->TimerQ;

	/* inc microsecond timer by 1 millisecond */
	xroot->Timer += 1000;

	/* Remove all TimerQ threads that should be awakened at this time */
	/* from the timer Q and place them onto the correct run Q */
	while (ss != NULL && (After(xroot->Timer, ss->wakeup))) {
		ThreadQ *q = &xroot->Queues[ss->priority];

		/* Remove from timer Q */
		xroot->TimerQ = ss->next;

		/* Add thread to the tail of its priority's run Q  */
		q->tail = q->tail->next = ss;
		ss->next = NULL;

		/* check if it's higher than the current priority */
		if (ss->priority  < xroot->HighestAvailPri)
			xroot->HighestAvailPri = ss->priority;

		/* if a TimedSuspend() thread times out then it must return */
		/* a FALSE value to signify this. The status value must be */
		/* left unmolested so that concurrent Resume() can detect */
		/* that the thread is special - see Resume() */
		if (ss->status == THREAD_TIMEDWAIT)
			ss->CPUcontext.R_A1 = FALSE;
		else
			ss->status = THREAD_RUNNABLE;

		/* advance to next thread */
		ss = xroot->TimerQ;
	}

	/* Decrement timeslice counter, if counter <= 0, current thread is */
	/* due for slicing (unless it is currently at HIGHPRI). */
	xroot->SliceTime--;

	/* If current thread is HIGHPRI return and continue thread. */
	if (xroot->CurrentPri == HIGHPRI)
		return; /* RTI */

	/* If no higher priority threads have been sheduled and */
	/* if Timeslice has not expired yet, then return and continue thread. */
	if (xroot->CurrentPri <= xroot->HighestAvailPri \
	    && xroot->SliceTime > 0)
		return; /* RTI */

	/* When higher priority threads than the current one are scheduled, */
	/* we preemptively slice the current thread before its full timeslice */
	/* is completed and execute the higher priority thread(s) */

	/* Slice current thread */
	ss = xroot->CurrentSaveArea;
	if (!SaveCPUState2(&xroot->CurrentSaveArea->CPUcontext)) {
		ThreadQ *q = &(GetExecRoot()->Queues[ss->priority]);

		/* add thread to end of run Q for its priority */
		q->tail = q->tail->next = ss;
		ss->next = NULL;

		ss->status = THREAD_SLICED;

		Dispatch(NULL);
	}

	return;	/* RTI */
}
