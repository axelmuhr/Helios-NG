/*
 * File:	gslice.c
 * Subsystem:	Generic Helios executive
 * Author:	P.A.Beskeen
 * Date:	Aug '91
 *
 * Description: Generic Helios executive timeslicer.
 *		Its so simple you could hardly call it a scheduler.
 *
 *
 * RcsId: $Id: gslice.c,v 1.22 1993/10/11 10:03:53 richardp Exp $
 *
 * (C) Copyright 1991 Perihelion Software Ltd.
 * 
 * RcsLog: $Log: gslice.c,v $
 * Revision 1.22  1993/10/11  10:03:53  richardp
 * Call resume at high priority
 *
 * Revision 1.21  1993/09/13  13:57:11  nickc
 * fixed potential vulnerability in accessing KnownThread Q
 *
 * Revision 1.20  1993/08/11  09:55:25  nickc
 * added ARM support
 *
 * Revision 1.19  1993/02/17  11:13:51  paul
 * removed dbg and re-ordered if then else to remove invarient code
 *
 * Revision 1.18  1992/11/20  15:42:46  paul
 * thread createion now grabs standard system address base from execroot
 *
 * Revision 1.17  1992/11/18  14:26:52  paul
 * fixed for non 0 IR0 on C40
 *
 * Revision 1.16  1992/11/12  17:12:57  paul
 * updated for rationalised Save/RestoreCPUState on C40,
 * permanent KDebugs(),
 * and new parameter types for C40WordAddress
 *
 * Revision 1.15  1992/09/22  13:13:34  paul
 * changed name of ExecRoot to GetExecRoot()
 *
 * Revision 1.14  1992/09/21  10:37:56  paul
 * moved the generic executive from the C40 directory to the main kernel
 * dir, so that it can be shared with the ARM implementation
 *
 * Revision 1.13  1992/09/17  16:10:56  paul
 * rmeoved some dbg and fixedup a more sensible version of TimedWait support
 *
 * Revision 1.12  1992/09/03  10:03:58  nickc
 * fixed calculation of initial stack end pointer
 *
 * Revision 1.11  1992/08/18  09:56:36  paul
 * updated to allow memory addresses in on-chip ram for .pc/.lr
 *
 * Revision 1.10  1992/08/04  18:04:25  paul
 * removed dbg
 *
 * Revision 1.9  1992/07/30  18:35:02  paul
 * fix stack extension support, add thread timing and idlemon support
 *
 * Revision 1.8  1992/07/21  13:02:57  nickc
 * now saves stack chunk header in task's Task structure
 *
 * Revision 1.7  1992/07/21  09:46:12  nickc
 * added code to build a stack chunk header
 *
 * Revision 1.6  1992/07/21  09:08:45  paul
 * removed upper bounds checking from savestates and inserted new idle counter
 *
 * Revision 1.5  1992/06/26  18:01:16  paul
 * added sysdeb check for correct cache handling
 * allows configurable cache enable from config
 *
 * Revision 1.4  1992/06/23  19:14:17  paul
 * fixed up debug test
 *
 * Revision 1.3  1992/06/22  08:27:36  paul
 * Added new Thread_startup status, added debufg to check particular threads
 * activations/deactivations and fixed TimedSemaphore
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

#include "kernel.h"


/* No stack checking should be done - this means we don't have to setup the */
/* stack limit in interrupt handlers */

#pragma no_check_stack


/*
 * Dispatch
 *
 * Save the current thread into the area supplied and run another
 * thread. The caller will already have linked the save state area onto
 * some queue for later reference. If the SaveState is NULL then no state
 * should be saved - this is due to the current thread either being killed
 * or being called from the timeslice interrupt handler.
 *
 * Dispatch can be called with interrupts already turned off.
 * Callers should be aware that interrupts will be re-enabled as soon as
 * their thread is resumed.
 *
 * The flavour of sheduler is an implementation defined decision. The
 * algorithm used here is a simple round robin within priority levels.
 * Higher priority levels having absolute priority over lower ones i.e.
 * while there are threads of a higher priority, no lower ones will be
 * scheduled. At the highest priority (HIGHPRI == 0) threads always
 * run until completion or they block, they are never sliced.
 *
 * @@@ A simple addition to this would be to shedule a thread on a lower Q
 * every time the Q cycles.
 *
 * Dispatch can be called at any thread priority.
 */

void Dispatch(SaveState *oldss)
{
	ExecRoot	*xroot = GetExecRoot();
	ThreadQ		*q = xroot->Queues;
	SaveState	*newss;
	word		i;

	/* Have to protect ourselves from a timeslice or any */
	/* interrupt that may result in a thread being added to */
	/* the run queues at this point */
	IntsOff();

	if( oldss != NULL ) {
		/* Add CPU time this thread has been running to its total */
		/* For threads that are under the resolution add at least */
		/* so they show some increment */
		oldss->CPUTimeTotal += (i = DiffTimes(xroot->Timer, oldss->LastTimeStamp)) > 0 ? i : 1;

		/* Call assembler code to actually save the CPU's context */
		/* this will seed the C return register with TRUE, but */
		/* return FALSE itself, it will also set the global */
		/* interrupt enable in the SaveState. When resumed, the */
		/* thread will run with interrupts enabled. */

		/* If SaveCPUState() is actually returning as a result of */
		/* a RestoreCPUState() then let it continue. */
#ifdef __C40
		/* C40 version automatically enables/disables interrupts to */
		/* guard against the SP corruption silicon bug. */
		if (_SaveCPUState(&oldss->CPUcontext) == TRUE)
			return;
#else
		if (SaveCPUState(&oldss->CPUcontext) == TRUE) {
			IntsOn();
			return;
		}
#endif
		/* Otherwise we have a valid CPU context in the save state. */
		/* The pointer to which has already been saved by the caller */
		/* for future resheduling (link comms, semaphores, etc). */
	}

	forever {
		/* walk down the queues running the first available thread */
		for (i = xroot->HighestAvailPri; i < PRIORITYLEVELS; i++ ) {
			if( q[i].head ) {
				/* resume head process of q[i]	*/

				/* remove save state from ready queue */
				newss = q[i].head;

				if ((q[i].head = newss->next) == NULL)
					/* fix tail if last state on queue */
					q[i].tail = (SaveState *)&q[i].head;

				/* setup root struct info for new process */
				xroot->CurrentPri = i;
				xroot->HighestAvailPri = i;
				xroot->CurrentSaveArea = newss;
				xroot->SliceTime = xroot->TicksPerSlice;
				newss->LastTimeStamp = xroot->Timer;

				/* Restart the suspended process. */
				if (newss->status <= THREAD_SLICED) {
					/* We are restoring a state saved */
					/* by the timeslicer, or starting a */
					/* new thread (THREAD_STARTUP). */
					/* We require an RTI style return. */

					newss->status = THREAD_RUNNING;
					RestoreSlicedState(&newss->CPUcontext);
				} else {
					/* We are restoring a state saved */
					/* by SaveCPUState(). */

					newss->status = THREAD_RUNNING;
					RestoreCPUState(&newss->CPUcontext);
				}
			}
		}

		/* If we reach this point then No threads are available */
		/* to run. We must wait (could possibly do some mundane */
		/* task) until one becomes available. */

		/* As we are being run at HIGHPRI, we do not have to worry */
		/* about being sliced and our stack being corrupted too badly */

		/* Force dispatcher to look at all run queues */ 
		xroot->HighestAvailPri = xroot->CurrentPri = HIGHPRI;

		i = xroot->Timer;

		/* Two types of Idle routines are allowed, the first as
		 * implemented on the C40, simply waits until an interrupt
 		 * occurs (courtesy of the IDLE instruction). The second as
		 * implemented on the ARM, simply re-enables interrupts for a
		 * short period of time, and then returns allowing the
		 * dispatcher to re-examine its queues to see if any new
		 * threads are available to run, if not the idle handler is
		 * entered again...
		 */

#ifdef __C40
		/* Interrupts are enabled and the processor now idles */
		/* until an interrupt occurs, at which point the interrupt */
		/* is serviced, we disable interrupts again and then return */

		IdleUntilInterrupt();
#else
		/* 
		 * Simple case - Disable interrupts for long enough for one
		 * to be accepted. Then busy loop around again to check if
		 * any threads have been re-scheduled. Note that as the
		 * CurrentPri is set to HIGHPRI, that no interrupt handler
		 * will directly call the dispatcher.
		 *
		 * Caution - there may be a minimum number of cycles
		 * that ints. must be enabled for an interrupt to occur.
		 * Also be aware that DISPATCHSTACKSIZE must be at least large
		 * enough to hold a SaveState.
		 */
		{
			SaveState BogusSaveState;

			/* Provide Area for intermediate state to be */
			/* saved in during interrupts */
			xroot->CurrentSaveArea = &BogusSaveState;

			IntsOn();
			/* Allow interrupts to occur */
			xroot->SliceTime++;
			xroot->SliceTime++;
			xroot->SliceTime++;
			IntsOff();
		}
#endif

		/* Keep track of amount of time spent idleing */
		xroot->IdleTime += xroot->Timer - i;
	}
}



/*
 * Suspend
 *
 * This is called by Kernel functions running at high priority,
 * or with interrupts disabled to suspend the current thread.
 * It is called with the address of a location into which a pointer to the
 * thread's SaveState structure will be saved.
 * The thread will only return from Suspend when it has been Resume()'ed.
 */

void Suspend(SaveState **ssp, word reason)
{
	SaveState	*oldss = GetExecRoot()->CurrentSaveArea;

#ifdef SYSDEB
	CheckState(oldss);
#endif

	/* give caller reference to save state */
	*ssp = oldss;

	/* note why thread is suspended */
	oldss->status = reason;

	Dispatch(oldss);
}


/*
 * TimedSuspend
 *
 * Similar to normal Suspend(), but can be rescheduled by either a call
 * to Resume(), or automatically at the end of its timeout. It is used to
 * implement the TimedWait() function.
 *
 * This function will always be called at high priority.
 *
 * The thread resumes with a result of TRUE if the thread was Resume()ed, or
 * FALSE if the thread returns as a result of a timeout.
 */

word TimedSuspend(SaveState **ssp, word timeout)
{
	SaveState	*ss = GetExecRoot()->CurrentSaveArea;

	*ssp = ss;	/* give caller reference to save state  */
	
	/* The special status value is used by Resume() to note that */
	/* the thread must also be removed from the timer Q */
	ss->status = THREAD_TIMEDWAIT;

	/* default to timedout type return, normal resume will set to TRUE */
	ss->TimedWaitUtil = FALSE;

	Sleep(timeout);

	return	ss->TimedWaitUtil;
}


/*
 * Resume
 *
 * Add a thread to the scheduling list, enabling the scheduler
 * to execute the thread again.
 *
 * This function is always called at HighPri.
 *
 * Note that any changes to this functions should also be added to the
 * companion IntrResume() function.
 */

void Resume(SaveState *ss)
{
	ExecRoot	*xroot = GetExecRoot();
	ThreadQ		*q = &xroot->Queues[ss->priority];
	word		intsenabled;

#ifdef SYSDEB
	CheckState(ss);
#endif

	/* TimedSuspend support */
	if (ss->status == THREAD_TIMEDWAIT) {
		SaveState	*nextss;
		SaveState	*lastss = (SaveState *)&xroot->TimerQ;
 
		ClockIntsOff();

		nextss = xroot->TimerQ;

		/* Signal() is resuming a semaphore thread that was */
		/* waited on with a TimedSuspend(). We should */
		/* therefore force the thread to return a TRUE value */

		/* Check timer Q and remove this threads save state */
		/* that was Q'ed there. If the state cannot be found */
		/* then the state has already timed out. In this */
		/* case the thread will have been rescheduled, but */
		/* not yet run. The Signal() function will have */
		/* already adjusted the semaphore count, so we */
		/* should make the thread look like it was resumed */
		/* as a result of a Resume(), not a timeout. As */
		/* interrupts are disabled we shouldn't worry about */
		/* race conditions any further. */

		/* Remove timeout thread from timer Q */
		while (nextss != NULL) {
			if (nextss == ss) {
				/* remove thread */
				lastss->next = nextss->next;
				break;
			}
			lastss = nextss;
			nextss = nextss->next;
		}

		/* If not found on Q, thread has already been removed */
		/* due to a time out. If found on Q, then the */
		/* timeout has not yet occurred. either way, make */
		/* savestate return as if semaphore was signaled */
		ss->status = THREAD_RUNNABLE;
		/* Note return via Signal()/Resume() NOT timeout */
		ss->TimedWaitUtil = TRUE;

		/* If savestate was removed from timer Q due to */
		/* timing out, then it is already on the run Q */
		if (nextss == NULL) {
			ClockIntsOn();
			return;
		}

		ClockIntsOn();
		/* otherwise add it to the run Q */
	}

	/* Only shield if not already disabled. */
	if ((intsenabled = IntsAreEnabled()) != FALSE)
		IntsOff();

		/* don't change THREAD_STARTUP as this is used by */
		/* EnterProcess to start a new thread going */
		if (ss->status != THREAD_STARTUP)
			ss->status = THREAD_RUNNABLE;

		/* check if re-scheduled thread's priority is highest */
		if (ss->priority < xroot->HighestAvailPri)
			/* optimisation hint for dispatcher */
			xroot->HighestAvailPri = ss->priority;

		/* add thread to end of run Q for its priority */
		ss->next = NULL;
		q->tail = q->tail->next = ss;

	if (intsenabled)	/* only enable if we disabled them */
		IntsOn();
}


/*
 * IntrResume
 *
 * Add a thread to the scheduling list, enabling the scheduler
 * to execute the thread again.
 * This function is passed a pointer to the SaveState of a suspended thread.
 *
 * IntrResume is only called from within interrupt handlers
 * that use the HardenedSignal() kernel function. It therefore doesn't play
 * with the global interrupt mask. THIS IS THE ONLY DIFFERENCE TO THE NORMAL
 * Resume() function.
 */

void IntrResume(SaveState *ss)
{
	ExecRoot	*xroot = GetExecRoot();
	ThreadQ		*q = &xroot->Queues[ss->priority];

	/* TimedSuspend support */
	if (ss->status == THREAD_TIMEDWAIT) {
		SaveState	*nextss = xroot->TimerQ;
		SaveState	*lastss = (SaveState *)&xroot->TimerQ;
 
		/* Signal() is resuming a semaphore thread that was */
		/* waited on with a TimedSuspend(). We should */
		/* therefore force the thread to return a TRUE value */

		/* Check timer Q and remove this threads save state */
		/* that was Q'ed there. If the state cannot be found */
		/* then the state has already timed out. In this */
		/* case the thread will have been rescheduled, but */
		/* not yet run. The Signal() function will have */
		/* already adjusted the semaphore count, so we */
		/* should make the thread look like it was resumed */
		/* as a result of a Resume(), not a timeout. As */
		/* interrupts are disabled we shouldn't worry about */
		/* race conditions any further. */

		/* Remove timeout thread from timer Q */
		while (nextss != NULL) {
			if (nextss == ss) {
				/* remove thread */
				lastss->next = nextss->next;
				break;
			}
			lastss = nextss;
			nextss = nextss->next;
		}

		/* If not found on Q, thread has already been removed */
		/* due to a time out. If found on Q, then the */
		/* timeout has not yet occurred. either way, make */
		/* savestate return as if semaphore was signaled */
		ss->status = THREAD_RUNNABLE;
		/* Note return via Signal()/Resume() NOT timeout */
		ss->TimedWaitUtil = TRUE;

		/* If savestate was removed from timer Q due to */
		/* timing out, then it is already on the run Q */
		if (nextss == NULL)
			return;
		/* otherwise add it to the run Q */
	}

	/* don't change THREAD_STARTUP as this is used by EnterProcess */
	/* to start a new thread going. The runQ may only have status's*/
	/* of THREAD_STARTUP, THREAD_SLICED and THREAD_RUNNABLE on it */
	if (ss->status != THREAD_STARTUP)
		ss->status = THREAD_RUNNABLE;

	/* add thread to end of run Q for its priority */
	ss->next = NULL;
	q->tail = q->tail->next = ss;

	/* check if re-scheduled thread's priority is highest */
	if (ss->priority < xroot->HighestAvailPri)
		/* optimisation hint for dispatcher */
		xroot->HighestAvailPri = ss->priority;
}


/*
 * Yield
 *
 * Yield is called by threads to move themselves to the end of their
 * priority levels run queue. The dispatcher running any other available
 * threads at the same priority or higher.
 * 
 * It's main purpose is to prevent any lengthy HIGHPRI operations from
 * stealing all the CPU bandwidth from other HIGHPRI processes. But it can
 * Also be used to force an immediate swap to a higher priority if the
 * caller knows one has been re-scheduled.
 *
 * Yield can be called from any priority.
 */

void Yield(void)
{
	ExecRoot	*xroot = GetExecRoot();
	ThreadQ		*q = &xroot->Queues[xroot->CurrentPri];
	SaveState	*ss = xroot->CurrentSaveArea;

#ifdef SYSDEB
	if (ss != NULL)
		CheckState(ss);
#endif
	/* If we are yielding the initial thread (ss == NULL) simply return. */
	/* Also, as an optimisation for Yield() being called at HIGHPRI */
	/* which happens frequently, if HIGHPRI Q is empty, don't bother */
	/* to Yield. */

	if (ss == NULL || ((xroot->CurrentPri == HIGHPRI) && (q->head == NULL)))
		return;

	/* Guard against other threads rummaging about in the run Qs */
	IntsOff();
		/* add to tail its priorities run Q */
		ss->next = NULL;
		q->tail = q->tail->next = ss;

		ss->status = THREAD_RUNNABLE;

		/* Run any threads at this priority or higher. */
		/* Interrupts will be re-enabled as we return. */
		Dispatch(ss);
}


/*
 * Stop
 *
 * Halt the current thread permanently.
 *
 * The current thread is halted permanently, it will never be resumed.
 *
 * Stop can be called at any thread priority. @@@ check - may be able to
 * remove xroot->CurrentPri = high pri + if this was so, os->status setting
 * would be safer.
 */

void Stop(void)
{
	ExecRoot	*xroot = GetExecRoot();
	SaveState	*ss = xroot->CurrentSaveArea;

	if (ss != NULL) {
		ss->status = THREAD_KILLED;

		/* remove this thread from the list of active threads */
		KnownThreadRm(ss);
	}
#if 0 && defined(__C40)
	/* to trace a particular threads activations */
	GTrace(0xdeed0000);
	GTrace(THREAD_KILLED);
	GTrace(C40WordAddress((word)ss));
#endif
	/* run a more deserving thread - without saving the current one */
	Dispatch(NULL);
}


/*
 * KnownThreadRm()
 *
 * Remove reference to this threads savestate from the known thread queue.
 *
 * Called from KillTask and Stop().
 */

void KnownThreadRm(SaveState *rmss)
{
	ExecRoot  *xroot = GetExecRoot();
	SaveState *lss = NULL;				/* last ss */
	SaveState *css;					/* current ss */
	word	   pri = xroot->CurrentPri;

	/* guard the list against corruption by creation of new threads */
	xroot->CurrentPri = HIGHPRI;

	css = xroot->KnownThreads;

	/* remove from executives list of known threads */
	while (css != NULL) {
		if (css == rmss) {
			/* remove from known thread list */
			if (lss == NULL) {
				/* if head item, fix new head */
				xroot->KnownThreads = css->nextknown;
			} else {
				/* unlink from list */
				lss->nextknown = css->nextknown;
			}
			break;
		}
		lss = css;		/* remember last */
		css = css->nextknown;	/* get next */
	}

	/* return to original pri */
	xroot->CurrentPri = pri;
}



/*
 * CreateProcess
 *
 * Initialise a new thread. The new thread when activated by EnterProcess()
 * will call the `entry' address as if it was a fn, any return from this fn
 * will cause the `exit' fn to be called. The result of CreateProcess()
 * is a pointer to the start of the argument area. These arguments are then
 * filled in by the caller (so must be contiguous). The argsize argument is
 * the size of the arguments passed in terms of bytes. CURRENT IMPLEMENTATIONS
 * ALWAYS EXPECT WORD ARGUMENTS. `descript[]' points to a two word
 * array, descript[0] is the module table pointer, descript[1] is the stack
 * base. The descript[ ] array is only valid for the duration of the call.
 * 
 *            Hi
 * ------------------------ <- top of stack passed to CreateProcess
 * |   Fixed SaveState    |    
 * |   (stdsavestate)     |    
 * |                      |
 * | usp = arg5/stacktop  | <- set at first stacked arg or stack top.
 * | use = descript[1] + ?| <- Optionally set up a stack end reg to base of 
 * |                      |    stack + safety value.
 * | mt = descript[0]     | <- module table pointer.
 * | lr = exit            | <- fn called to terminate process.
 * | v1 = descript[1]     | <- stack base passed to exit fn to free up stack.
 * | pc = entry           | <- process entry function.
 * |                      |
 * ------------------------ <- fixed save state area (SaveState *)
 * |        arg n         | -1    /?\
 * ------------------------        |
 * | args to be           | -2     |
 * ------------------------        |
 * | filled in            |    `argsize'
 * ------------------------        |
 * | by caller            |        |
 * ------------------------        |
 * |        arg 1         | -n    \?/
 * ------------------------ << start of args (ret by CreateProcess() & arg to
 * | saved argsize        |    EnterProcess()). 
 * ------------------------ <- used to work out what args to move from stack
 * | saved stdsavestate   |    to argument registers by EnterProcess().
 * ------------------------ <- used to work out where fixed savestate area is
 *				by EnterProcess().
 *
 * Figure 1: Possible stack configuration used by CreateProcess()
 *
 * In current implementations `argsize' is always a multiple of
 * 4 bytes (a word).
 *
 * N.B. v1 (a compiler variable register) is used to store the stack base. This
 * is safe as the first fn called should automatically stack it, restoring
 * it upon return. When the exit fn is then entered, it can access the value
 * in v1 to free up the thread stack before suspending. - See ._ProcExit
 * assembler function in "utilasm.a" - Util library.
 *
 * Space is permanently allocated for the save state at the top of
 * the `stack' that is passed to CreateProcess().
 * This is held in GetExecRoot()->CurrentSaveArea when the thread is executing.
 *
 * NB This code assumes a falling stack ...
 *
 */

word *CreateProcess(word *stack, VoidFnPtr entry, VoidFnPtr exit, 
			word *descript, word argsize)
{
	SaveState *initss;
#ifdef __C40
	stack_chunk *pchunk;
#endif

	union {
		word w;
		VoidFnPtr vfn;
	} entryexit;

#if 0
KDebug("CreateProcess(stack %x, entryfn %x, exitfn %x \n\t" \
	"descript[0] (modtab) %a, descript[1] (stackbase) %a, argsize %x\n", \
	stack, entry, exit, descript[0], descript[1], argsize);
#endif
	/* allocate space for the permanent save state */
	stack -= sizeof(SaveState) / sizeof(word);
	initss = (SaveState *)stack;

#ifdef __C40
	/* automatic stack extension support */

	/* create a stack chunk header at the start of the stack */
	stack -= sizeof (stack_chunk) / sizeof(word);

	/* initialise the stack chunk */
	pchunk = (stack_chunk *)stack;
	pchunk->next = NULL;
	pchunk->prev = NULL;
	pchunk->size = ((int)stack - descript[1]) / sizeof (word);
	pchunk->link_register     = 0;
	pchunk->stack_pointer     = 0;
	pchunk->stack_end_pointer = 0;	    

	/* store the stack header in the threads save state structure */
	initss->stack_chunk = pchunk;
#endif

	/* build entry procedure stack frame */
	stack -= argsize / sizeof(word);	/* space for user args */
	stack[-1] = argsize / sizeof(word);	/* remember number of args */
	stack[-2] = (word)initss;		/* pass SaveState ptr to */
						/* EnterProcess() */ 

	/* Initialise CPU state. */

#ifdef __ARM
	/* 0 FP notes initial function called */
	initss->CPUcontext.R_FP = NULL;
#else
	/* frame pointer points to first usable byte of user stack */
# ifdef PCS_FULLDECENDING
#  ifdef __C40
	initss->CPUcontext.R_FP = C40WordAddress(stack);
#  else
	initss->CPUcontext.R_FP = (word)stack;
#  endif
# elif defined(PCS_EMPYTDECENDING)
#  ifdef __C40
	initss->CPUcontext.R_FP = C40WordAddress(stack - 1);
#  else
	initss->CPUcontext.R_FP = stack - 1;
#  endif
# else
#  error "Stack type not yet defined"
# endif
#endif

	/* Get machine address of entry function */
	/* The 'C40's PCS says that function ptrs are held as word addresses */
	entryexit.vfn = entry;

	/* save note of threads root function for debugging purposes */
	initss->InitialFn = entry;

	/* fake up a sliced context that starts the thread going */
#ifdef __ARM
	/* Check for bodge that indicates thread should be started in SVC */
	/* mode. This is used by NewWorker() in memory1.c. */
	/* It allows for compatibility between ARM2/3 */

	/* Currently just sets the ARM6 26bit mode from the ARM2/3 mode bits. */
	/* @@@ This will need fixing for true 32 bit world working */
	initss->CPUcontext.R_CPSR = entryexit.w & ModeMask;
#endif

#ifdef __C40
	initss->CPUcontext.PC = entryexit.w;
#else
	initss->CPUcontext.R_PC = entryexit.w;
#endif
	initss->status = THREAD_STARTUP;

#ifdef __C40
	/* set status register bits to enable interrupts and */
	/* set the SET COND bit (C PCS requirement) when a RETI */
	/* instruction is executed i.e. set COND and PGIE. */
	initss->CPUcontext.R_ST = (1 << 14) | (1 << 15);

	/* if the I/O server config doesn't disable it, enable cache */
	if (!(GetRoot()->Flags & Root_Flags_CacheOff))
		initss->CPUcontext.R_ST |= (1 << 11);
#else
# ifndef __ARM
	initss->CPUcontext.R_ST = 0;
# endif
#endif

	/* link register forces return via exit function */
	entryexit.vfn = exit;
#ifdef __ARM
	initss->CPUcontext.R_USER_LR = entryexit.w;
	/* Also set SVC LR in case we are started in SVC mode. */
	initss->CPUcontext.R_SVC_LR = entryexit.w;
#else
	initss->CPUcontext.R_LR = entryexit.w;
#endif
	/* The exit function can use the R_V1 register to find the stack */
	/* base for this thread. The PCS forces this (variable reg) to be */
	/* saved by the entry function */
	initss->CPUcontext.R_V1 = descript[1];

#ifdef __C40
	/* module table pointer */
	initss->CPUcontext.R_MT = C40WordAddress((char *)descript[0]);

	/* byte address base use by C is always the base of addressable RAM */
	/* Default address base is held in the ExecRoot and set by bootstrap */
	initss->CPUcontext.R_BASE = GetExecRoot()->CAddressBase;
#else
	/* module table pointer */
	initss->CPUcontext.R_MT = descript[0];
#endif

	/* stack end is set to the stack base minus a compiler defined safe */
	/* stack overflow area */
#ifdef __C40
	initss->CPUcontext.R_USE = C40WordAddress((char *)descript[1] + PCS_STACKGUARD);
#else
	initss->CPUcontext.R_USE = descript[1] + PCS_STACKGUARD;
#endif

	return(stack);		/* return pointer to argument area */
				/* args are then filled in by user */
}


/*
 * EnterProcess
 *
 * Start up a thread created by CreateProcess.
 *
 * The first argument is the result of CreateProcess, the second is the
 * priority at which this thread is to be run.
 *
 * @@@ check if we are always called at highpri - if so we dont need the
 * guard when adding the thread to the known thread Q.
 */

void EnterProcess(word *stack, word pri)
{
	/* stack[-1/-2] are set by CreateProcess() */
	/* stack[-1] = argsize in words */
	/* stack[-2] = Pointer to permanent SaveState area */
	word		argsize = stack[-1];
	SaveState	*ss = (SaveState *)stack[-2];
	word		*aregs;
	word 		i;
	ExecRoot	*xroot = GetExecRoot();

#if 0
KDebug("EnterProcess stack %a, pri %x, stack[-2] ss %a, stack[-1] argsize %d\n", \
		stack, pri, ss, argsize);

KDebug("EnterProcess stack[0] %a, stack[1] %a, stack[2] %a, stack[3] %a\n", \
		stack[0], stack[1], stack[2], stack[3]);
#endif
	/* get address of first C argument register held in save state */
	/* code assumes following PCS_ARGREGS are held contigously upwards */
	aregs = &ss->CPUcontext.R_A1;

	/* move stacked arguments to argument registers */
#ifdef __C40
	for (i = 0; i < (argsize * 2) && i < (PCS_ARGREGS * 2); i+=2) {
		aregs[i] = *stack++;
		/* SaveState has fp extension in second 32bit word per reg */
		/* this should be cleared */
		aregs[i+1] = 0;
	}
#else
	for (i = 0; i < argsize && i < PCS_ARGREGS; ) {
		aregs[i++] = *stack++;
	}
#endif

	/* any left over arguments are held on the stack */
#ifdef PCS_FULLDECENDING
# ifdef __C40
	ss->CPUcontext.R_USP = C40WordAddress(stack);
# else
	ss->CPUcontext.R_USER_SP = (word)stack;
#  ifdef __ARM
	/* Set SVC stack in case we are started in SVC mode. */
	ss->CPUcontext.R_SVC_SP = (word)stack;
#  endif
# endif
#elif defined(PCS_EMPYTDECENDING)
# ifdef __C40
	ss->CPUcontext.R_USP = C40WordAddress(stack - 1);
# else
	ss->CPUcontext.R_USER_SP = stack - 1;
# endif
#else
# error "Stack type not defined"
#endif

	/* set startup times */
	ss->InitialTime = GetRoot()->Time;	/* secs since 1970 */
	ss->CPUTimeTotal = 0;			/* no CPU time used yet */

	/* set threads initial priority */
	ss->priority = pri;

	/* go to HIGHPRI as I'm playing with execs known thread list */
	i = xroot->CurrentPri;
	xroot->CurrentPri = HIGHPRI;
		/* always add new threads to the head of list */
		ss->nextknown = xroot->KnownThreads;
		xroot->KnownThreads = ss;
	/* return to original priority */

#if 0 /*def SYSDEB*/
KDebug("EnterProc: %x, stk %x, ss %a\n", ss->CPUcontext.PC, ss->CPUcontext.R_USP, ss);
#endif

	Resume(ss);	/* add new thread to run Q */
	xroot->CurrentPri = i;
}


#if 0
/* check known thread Q for infinite loops */
int checkntq(void)
{
	int i = 0;
	SaveState *lss = GetExecRoot()->KnownThreads;

	while(lss) {
		if (i++ > 1000) {
			KDebug("checkntq: DANGER loop in knownthread Q\n");
			JTAGHalt();
			return FALSE;
		}
		lss = lss->nextknown;
	}

	return TRUE;
}
#endif



/* end of gslice.c */
