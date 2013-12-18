/*
 * File:	gexec.c
 * Subsystem:	Generic Helios executive
 * Author:	P.A.Beskeen
 * Date:	Nov '91
 *
 * Description: Generic Helios executive.
 *
 *		This file implements miscellaneous executive support functions.
 *
 * RcsId: $Id: gexec.c,v 1.25 1993/10/04 12:05:09 paul Exp $
 *
 * (C) Copyright 1991 Perihelion Software Ltd.
 * 
 * RcsLog: $Log: gexec.c,v $
 * Revision 1.25  1993/10/04  12:05:09  paul
 * Added dispatch latency test
 *
 * Revision 1.24  1993/09/17  16:06:42  paul
 * interrupts are now only enabled after the event handling system has been
 * initialised.
 *
 * Revision 1.23  1993/08/11  09:46:24  nickc
 * added ARM support
 *
 * Revision 1.21  1993/06/18  08:54:59  paul
 * added GetNucleusBase() and GetRootBase() from defunct romsupp.c
 *
 * Revision 1.20  1992/11/18  14:23:20  paul
 * changed to support non 0 IR0 on C40
 *
 * Revision 1.19  1992/11/12  17:11:49  paul
 * updated to remove uncessary Save/RestoreCPUState wrappers for C40
 *
 * Revision 1.14  1992/09/25  09:39:34  paul
 * removed SystemStackSize and stopped setting SystemStack as bootstrap now
 * does this.
 *
 * Revision 1.13  1992/09/22  13:13:34  paul
 * changed name of ExecRoot to GetExecRoot()
 *
 * Revision 1.12  1992/09/21  10:37:11  paul
 * moved the generic executive from the C40 directory to the main kernel
 * dir, so that it can be shared with the ARM implementation
 *
 * Revision 1.11  1992/09/17  16:09:24  paul
 * fixed StoreSize for C40 so that it returns the highest available block
 * on local bus
 *
 * Revision 1.10  1992/08/04  18:04:18  paul
 * removed dbg
 *
 * Revision 1.9  1992/07/30  18:35:50  paul
 * added system hipri overstay debug
 *
 * Revision 1.8  1992/06/30  19:29:48  paul
 * changed proto for SaveCPUState()
 *
 * Revision 1.7  1992/06/26  17:59:50  paul
 * added GetIDROM/SliceState/SliceQuantum functions
 * and variable quantum rather than fixed for timeslice
 *
 * Revision 1.6  1992/06/23  19:20:18  paul
 * default timeslicing to on
 *
 * Revision 1.5  1992/06/23  19:12:13  paul
 * overly safe cast of sign to unsign for timer calc
 *
 * Revision 1.4  1992/06/19  18:18:42  paul
 * added global buffer trace functions and made System() note changed
 * priority in savestate
 *
 * Revision 1.3  1992/05/14  10:46:10  paul
 * added a few savestate checks
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
#include <stdarg.h>
#include <process.h>


#if defined(SYSDEB) && defined(__C40)
# define GNEXT	0x80000000
# define GEND	GNEXT + 4 * 1024 * 1024
#endif


/*
 * ExecInit
 *
 * Initialise executive	structures, setup current thread as a
 * high priority process and start timeslicer.
 *
 * Any processor specific initialisation and the setting up a kosha
 * 'C' runtime environment is expected to have been done by the
 * initialisation code in kmodule.a and/or the bootstrap (c40boot.a, etc)
 *
 */

void ExecInit(void)
{
	ExecRoot	*xroot = GetExecRoot();	/* find addr of root */
	word		pri;

	/* initialise the ExecRoot structure */

	xroot->CurrentPri = HIGHPRI;	  /* the current thread is high pri */
	xroot->CurrentSaveArea = NULL;	  /* it has no savearea */
	xroot->HighestAvailPri = HIGHPRI; /* and you can't get any higher */
	xroot->IdleTime = 0;		  /* not hit idle yet! */

#if !defined(__ARM) && (defined(KERNELDEBUG2) || defined(KERNELDEBUG1))
	/* ARM VLSI PID board debugging needs to be delayed until speed is */
	/* set. Normally KERNELDEBUG2 & KERNELDEBUG1 debugging can be done */
	/* immediately. */
	xroot->KDebugEnable = TRUE;	  /* can debug straight away */
#else
	/* message type debugging needs to be delayed */
	/* simpler debugging strategies don't (see gdebug.c) */
	xroot->KDebugEnable = FALSE;	  /* No debugging yet */
#endif
#if defined(__C40) && defined(SYSDEB) && defined(GLOBDBG)
	/* init Global memory trace */
	GWrite(GNEXT, GNEXT+3);
#endif
	xroot->TimerQ = NULL;		/* no threads on timer Q */

	xroot->Timer = 25;		/* init microsecond counter */

	/* set default number of clock ticks to first slice */
	xroot->SliceTime = xroot->TicksPerSlice = DEFAULTTICKSPERSLICE; 

#if 0
	xroot->SliceEnabled = FALSE;	/* initially disable timesliceing */
#else
	xroot->SliceEnabled = TRUE;	/* start with slicing enabled */
#endif
	/* Initialise the ready queues to empty */
	for (pri = 0; pri < PRIORITYLEVELS; pri++) {
		xroot->Queues[pri].head = NULL;
		xroot->Queues[pri].tail = \
				(SaveState *) &xroot->Queues[pri].head;
	}

	/* Initialise the known thread list */
	xroot->KnownThreads = NULL;	  /* no known threads */

#ifdef LATENCYTEST
	xroot->Latency = 0;
	xroot->MaxLatency = 0;
	xroot->MaxLatencyPC = 0;
	xroot->DispatchLatMs = 0;
	InitSemaphore(&xroot->DispatchLatCheck, 0);
#endif

#ifdef __ARM
	/* ARM CPU and hardware specific initialisation */
	ARMExecInit();
#endif

	/* Initialise the on-chip communication links */
	InitCommsLinks();

	/* Initialise and start the time slicer */
	StartTimeSlicer();

	/* Wait until interrupt handler is initialised an we have a */
	/* valid savestate for the initial thread (in xroot->CurrentSaveArea) */
	/* before enabling interrupts. This is a good idea generally. */
}



/*
 * StoreSize
 *
 * Calculate the amount of available memory..
 * StoreSize is passed the base of free memory, it should return the address
 * of the first unusable byte (word) of RAM.
 *
 * On the C40 This means that the highest valid strobe should be used.
 * If there is a gap between the two strobes it will be 'fixed' by the memory
 * code to look like a pre-allocated block.
 * @@@ + Might have to be updated to cope with both local and global buses,
 * @@@ + Ignores mem_start argument (end of config vector).
 *
 * The ARM port rightly moves the store size function to the platform specific
 * source files. i.e. Different platforms may require different ways of
 * determining how much memory they contain.
 */

#ifndef __ARM
byte *StoreSize(byte *mem_start) {
#ifdef __C40
	ExecRoot	*xroot = GetExecRoot();

	/* This code assumes one strobe has to be valid! */
	if (xroot->ID_ROM.LBASE1 == 0xffffffff || xroot->ID_ROM.LSIZE1 == 0)
		/* only strobe zero valid */
		return ((byte *) C40CAddress(xroot->ID_ROM.LBASE0) + xroot->ID_ROM.LSIZE0 * sizeof(word));
	elif (xroot->ID_ROM.LBASE0 == 0xffffffff || xroot->ID_ROM.LSIZE0 == 0)
		/* only strobe one valid */
		return ((byte *) C40CAddress(xroot->ID_ROM.LBASE1) + xroot->ID_ROM.LSIZE1 * sizeof(word));
	else
		/* both strobes are valid, use highest one */
		if (xroot->ID_ROM.LBASE1 > xroot->ID_ROM.LBASE0)
			return ((byte *) C40CAddress(xroot->ID_ROM.LBASE1) + xroot->ID_ROM.LSIZE1 * sizeof(word));
		else
			return ((byte *) C40CAddress(xroot->ID_ROM.LBASE0) + xroot->ID_ROM.LSIZE0 * sizeof(word));
#else
# error no way of calculating memory size for this CPU type.
#endif
}
#endif


/* Kernel access to executive structures */

#if 0 /* implemented as a macros */
/* Return pointer to first save state on timer queue */
SaveState *TimerQHead(void)
{
	return (GetExecRoot()->TimerQ);
}

/* Return address of timer queue */
SaveState **TimerQAddr(void)
{
	return &(GetExecRoot()->TimerQ);
}

/* Returns address of the ready queue for a particular priority */
ProcessQ *ReadyQBase(word pri)
{
	return &(GetExecRoot()->Queues[pri]);
}
#endif

/* Provides pointers to the first and last save states on the ready queue */
/* For a particular priority */
void RunqPtrs(SaveState **p, word pri)
{
	struct ExecRoot *xroot = GetExecRoot();

	p[0] = xroot->Queues[pri].head;
	p[1] = xroot->Queues[pri].tail;
}


/* Thread priority control */

/*
 * System
 *
 * This is called by Kernel procedures to effect a temporary
 * transition to high priority.
 * 
 * The function takes as arguments a pointer to a function and up to three
 * arguments to be passed to it.
 * The original priority of the calling thread is saved and the procedure
 * argument called at high priority. When it returns the old priority is
 * restored and the thread continues at it original priority.
 * Before the call to System returns, all pending higher priority threads
 * must have completed. Note that System doesn't set HighestAvailPri, only
 * CurrentPri.
 */

word System(WordFnPtr func, ... )
{
	va_list		ap;
	word		a, b, c;
	word		result;
	word		oldpri;
#if 0
	word		starttime;	/* check length of time at hipri */
#endif
	ExecRoot	*xroot = GetExecRoot();

	/* grab arguments */
	va_start(ap, func);
		a = va_arg(ap, word);
		b = va_arg(ap, word);
		c = va_arg(ap, word);
	va_end(ap);

#if 0
	KDebug("System (fn %x, %x %x %x)\n", func, a, b, c);
#endif

	oldpri = xroot->CurrentPri;	/* save old priority */
	xroot->CurrentPri = HIGHPRI;	/* goto high priority */

#if 0
        starttime = Timer();
#endif

	if (xroot->CurrentSaveArea != NULL)
		xroot->CurrentSaveArea->priority = HIGHPRI;

        result = func(a, b, c);         /* call function at high pri */

#if 0
	/* if we havent been descheduled at highpri and we were in it for */
	/* longer than is decent, then complain */
        if (xroot->CurrentSaveArea->LastTimeStamp <= starttime &&
	    (starttime = DiffTimes(Timer(), starttime)) > OneSec/100)
		KDebug("System: too long @ HIPRI %xuS fn %x\n",
			starttime, func);
#endif

	/* return to original priority */
	if (xroot->CurrentSaveArea != NULL)
		xroot->CurrentSaveArea->priority = oldpri;

	xroot->CurrentPri = oldpri;

 	/* Try to keep dispatch latency low by allow any new higher priority */
 	/* threads to be executed immediately. */
 	if (xroot->HighestAvailPri < oldpri)
 		Yield();

	return result;
}


/*
 * SetPhysPri
 *
 * Returns the threads original priority after changing it to `newpri'.
 * The Execroot->CurrentPri field is used to determine which ready
 * queue to place sliced processes on. The SaveState->priority field is
 * only used as a reference as the Dispatch()er always sets the savestates
 * pri to be the priority it was suspended at.
 */

word SetPhysPri(word pri)
{
	ExecRoot	*xroot = GetExecRoot();
	word		old = xroot->CurrentPri;

	if ((xroot->CurrentPri = pri) < xroot->HighestAvailPri)
		xroot->HighestAvailPri = pri;

	if (xroot->CurrentSaveArea != NULL)
		xroot->CurrentSaveArea->priority = pri;

	/* If we are lowering our priority then Yield() in case there */
	/* are higher priority processes ready to run. */
	if (old < pri)
		Yield();

	return old;
}

#if 0	/* actually implemented as a macro */
/*
 * GetPhysPri
 *
 * Returns the current thread's priority.
 */
word GetPhysPri(void)
{
	return GetExecRoot()->CurrentPri;
}
#endif


/*
 * GetPhysPriRange
 *
 * Returns the lowest available priority.
 * Priority levels range from 0 (HIGHPRI) to this level.
 *
 * Kernel exports this function.
 */
 
word GetPhysPriRange(void)
{
	return (PRIORITYLEVELS - 1);
}


/* Module table access */

/*
 * CallWithModTab
 *
 * Call a function using a different module table. This routine is used by
 * the kernel to call down a programs INIT chain, initialising its
 * module table's static data areas.
 *
 * Kernel exports this function.
 */

word CallWithModTab(word arg1, word arg2, WordFnPtr fn, word *modtab)
{
	word result;

	word *oldmodtab = _SetModTab(modtab);		/* asm fn */
		result = fn(arg1,arg2);
	_SetModTab(oldmodtab);				/* asm fn */

	return result;
}


/* Time related functions */

#ifndef __C40
/* Returns current value of the microsecond clock */
/* Kernel exports this function */
word _ldtimer(word pri)
{
	return Timer();
	pri = pri;	/* no compiler warnings - will be optimised out */
}
#endif /* not __C40 (C40 version is in C40/c40exec.a) */

/* Returns current value of the microsecond clock in centiseconds */
/* Kernel exports this function */
word _cputime(void)
{
	return(Timer() / 10000);
}

/* Timer() is Also implemented as a macro for in-kernel calls */
#ifdef Timer
# undef Timer
#endif

/* Returns current value of the microsecond clock */
word Timer(void)
{
	return GetExecRoot()->Timer;
}

/*
 * Sleep
 *
 * Suspend thread for the given time (in microseconds).
 * This is implemented by placing the thread on the Timer queue, in
 * time to thread wake up order.
 *
 * @@@ Should Sleep() check for times > MaxInt/2, as
 * These will not be correctly delayed?
 *
 * Sleep should be called at HIGHPRI.
 */

void Sleep(word usecs)
{
	ExecRoot	*xroot = GetExecRoot();
	SaveState	*s;


	ClockIntsOff();	/* Disable timer interrupts as we don't want anyone */
			/* else playing with the timer Q's by way of a */
			/* timeslice occuring. The reason for disabling just */
			/* the clock, rather than all interrupts is to */
			/* reduce the interrupt latency, if the TimerQ is */
			/* quite full, this could be a potentially lengthy */
			/* operation */

		s = xroot->CurrentSaveArea;

		/* TimedSuspend() support */
		if (s->status != THREAD_TIMEDWAIT)
			/* not called by TimedSuspend(), so update status */
			s->status = THREAD_SLEEP;

		/* calc thread's wakeup time */
		s->endtime = xroot->Timer + (uword)usecs;

		/* See whether queue is empty or this time is before the */
		/* first wakeup time on the Q. In either case add to Q's head */
		if (xroot->TimerQ == NULL
		    || After(xroot->TimerQ->endtime, s->endtime)) {
			/* add at head of Q */
			s->next = xroot->TimerQ;
			xroot->TimerQ = s;
		}
		else {
			/* Otherwise search queue for correct place. If we */
			/* reach this point we are guaranteed to have at */
			/* least one SaveState with an earlier time on */
			/* the queue already */
			SaveState *prev = xroot->TimerQ;
			SaveState *next = prev->next;

			while (next != NULL \
			       && After(s->endtime, next->endtime)) {
				prev = next;
				next = prev->next;
			}

			s->next = next;		/* insert at correct pos */
			prev->next = s;
		}

		IntsOff();	/* Guard our save state against changing */
				/* in a timeslice between ClockIntsOn */
				/* and the call to the Dispatcher. */
		ClockIntsOn();

		/* Deschedule current thread and run another. */
		Dispatch(s);
		/* Dispatch() will return after delaying for at least the */
		/* period required. Interrupts will be re-enabled upon return.*/

	return;
}


#ifdef __C40
/* return a pointer to the saved IDROM structure in the execroot */

IDROM *GetIDROM(void) {
	return &GetExecRoot()->ID_ROM;
}
#endif


/* Set, Reset or Report state of timeslicing */
int SliceState(int x) {
	ExecRoot *xroot = GetExecRoot();

	if (x != SLICE_REPORT)
		xroot->SliceEnabled = (word)x;

	return (int)xroot->SliceEnabled;
}


/* Set the period of time each thread receives to run in a timeslice */
/* The C40 implementation is only accurate to the nearest millisecond */
/* If 'usecs' are zero, then the quantum is not set, and the current */
/* quantum is returned */
int SliceQuantum(int usecs) {
	ExecRoot *xroot = GetExecRoot();

	if (usecs != 0)
		xroot->TicksPerSlice = (word)usecs / 1000L;

	return 	(int)xroot->TicksPerSlice;
}


/*
 * The root.h defined macros GetSysBase() and GetRoot() use the following
 * functions to provide public access to the nucleus load address and
 * root structure position. The kernel (gexec.h) redefines these macros for
 * its own private use. This allows it to directly access its private data
 * structure, rather than take the overhead of calling these functions.
 */

/* Returns the base address of the nucleus */

MPtr GetNucleusBase(void)
{
	return ( GetSysBase() );
}

/* Returns the address of the kernel's root structure. */

RootStruct *GetRootBase(void)
{
	return ( GetRoot() );
}


#ifdef LATENCYTEST
/* Dispatch latency test thread. This is only run in a debugging test system
 * to determine what the maximum dispatch latency is. This thread should be
 * run at HighestAvailPri. The results are held in the kernel root structure.
 * The "maxlat" program can be used to print out these values. This MUST not
 * be run in a production system as its semaphore is kicked every timeslice!
 * This code is generic and could be used to test other processors dispatch
 * latencies - code would have to updated where Dispatch() is called from
 * the interrupt handler. For this code to give the standard systems maximum
 * dispatch latency (the one we want to quote), this should be the highest
 * priority thread in the system, and no other threads should be at this
 * priority (in a Mutex based kernel).
 */
void DispatchLatTest(void)
{
	ExecRoot *	xr = GetExecRoot();
	RootStruct *	root = GetRoot();
	word		lat;

	forever {
		HardenedWait(&xr->DispatchLatCheck);

		lat = _ldtimer(0) - xr->DispatchLatMs;

		if (lat > root->MaxLatency)
			root->MaxLatency = lat;

		root->Latency = lat;
	}
}
#endif

/* tmp debug functions */
#if defined(__C40) && defined(SYSDEB) && defined(GLOBDBG)

/* Global bus trace buffer functions */
/* generally used for large amounts of trace info for particularly */
/* intractable BUGS */

/* find a word in the global trace block */
/* to used, set r0 to word to find and jump via ?pc = GTraceFind addr */
void GTraceFind(word find) {
	word addr = GRead(GNEXT);

	GWrite(GNEXT+1, GNEXT+2);

	while (addr > GNEXT+3) {
		if (GRead(--addr) == find) {
			word note = GRead(GNEXT+1);
			GWrite(note, addr);
			GWrite(GNEXT+1, note+1);
		}
	}
	JTAGHalt();
}

/* Write block of word data to global bus */
void GWriteB(word addr, word size, word *buffer) {
	word *w = (word *)buffer;

	while(size--)
		GWrite(addr++, *w++);
}

void GTraceB(word size, word *data) {
	word gbuf;

	gbuf = GRead(GNEXT);

	if (((word)data & 3) != 0) {
		_Trace(0xdedded99, (word)data, 0);
		JTAGHalt();
	}

	if ((size & 3) != 0) {
		_Trace(0xdedded88, size, 0);
		JTAGHalt();
	}

	if ((gbuf + size + 3) >= GEND)
		gbuf = GNEXT + 2;

	GWrite(gbuf++, 0x1baaaad1);
	GWriteB(gbuf, size, data);

	gbuf += size + 1;

	GWrite(GNEXT, gbuf);
}

void GTrace(word data) {
	word gbuf;

	gbuf = GRead(GNEXT);

	if ((gbuf + 1 + 3) >= GEND)
		gbuf = GNEXT + 2;

	GWrite(gbuf++, data);
	GWrite(GNEXT, gbuf);
}
#endif


/* end of gexec.c */

