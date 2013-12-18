/*------------------------------------------------------------------------
--                                                                      --
--                     H E L I O S   N U C L E U S                      --
--                     ---------------------------                      --
--                                                                      --
--             Copyright (C) 1987, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- cpustate.h								--
--                                                                      --
--	CPU save state data structure and fn prototypes			--
--                                                                      --
--	Author:  PAB/JGS 24/10/89					--
--                                                                      --
------------------------------------------------------------------------*/
/* SccsId: %W% %G% Copyright (C) 1987, Perihelion Software Ltd.	*/
/* RcsId: $Id: cpustate.h,v 1.14 1992/09/17 15:47:54 paul Exp $ */

#ifndef __cpustate_h
#define __cpustate_h

#ifndef __TRAN	/* non of this is applicable to the transputer */

#ifndef __helios_h
# include <helios.h>
#endif


#ifdef __ARM
/* These should be be kept in-line with the TRUE definitions in
 * "/helios/include/abcARM/asm/arm.s"
 */
# define sp_offset	(0x00000080)
# if 1 /* see the comments in "include/abcARM/asm/arm.s" */
#  define sl_offset	(0x00000200 + sp_offset)
# else
#  define sl_offset	(0x00000300 + sp_offset)
# endif

typedef struct SaveState {
	struct SaveState *next;	/* NULL (used by Scheduler) */
	word	endtime;	/* NULL (used by Scheduler) */
	word	timeslice;	/* NULL (used by Scheduler) */
	word	pri;		/* current process priority */
        word	fparea;		/* fp work area */
	word	initial_dp;	/* dp given to CreateProcess */
	word	memmap;		/* process MEMMAP state */
	word	flags;		/* SaveState state flags */
	word	a1;		/* argument variables */
	word	a2;
	word	a3;
	word	a4;
	word	v1;		/* register variables */
	word	v2;
	word	v3;
	word	v4;
	word	v5;
	word	dp;		/* module table pointer */
	word	sl;		/* stack limit */
	word	fp;		/* frame pointer */
	word	ip;		/* temporary register */
	word	sp;		/* stack pointer */
	word	lk;		/* link register */
	word	pc;		/* program counter */
	word	usr_r13;	/* explicit USR mode r13 */
	word	usr_r14;	/* explicit USR mode r14 */
} SaveState;

#else

# ifdef __C40
typedef struct CPURegs {
		     /* C PCS BINDING NAME: */
				/* 'C40 REGISTER NAME: */
				        /* FUNCTION: */

	word	PC;		/*	   PC of sliced thread */
	word	R_ST;		/* st :	   status reg */

						/* C ADDRESS REGS */
	word	R_ADDR1;	/* ar0: */
	word	R_ADDR2;	/* ar1: */
	word	R_ADDR3;	/* ar2: */
	word	R_ADDR4;	/* ar3: */

							/* MISC ADDRESS REGS */
	word	R_MT;		/* ar4:    module table pointer */
	word	R_ATMP;		/* ar5:    temporary address reg */
	word	R_USP;		/* ar6:    user stack pointer */
	word	R_FP;		/* ar7:    frame pointer */

							/* REGISTER ARGUMENTS */
	word	R_A1;		/* r0 :    first arg and result reg */
	word	R_A1f;		/* r0 :    fp extension to 32bit reg */

	word	R_A2;		/* r1 :    32bits */
	word	R_A2f;		/* r1 :    fpext */
	word	R_A3;		/* r2 :    32bits */
	word	R_A3f;		/* r2 :    fpext */
	word	R_A4;		/* r3 :    32bits */
	word	R_A4f;		/* r3 :    fpext */

							/* REGISTER VARIABLES */
	word	R_FV1;		/* r4 :    32bits */
	word	R_FV1f;		/* r4 :    fpext */
	word	R_FV2;		/* r5 :    32bits */
	word	R_FV2f;		/* r5 :    fpext */
	word	R_FT1;		/* r6 :    32bits */
	word	R_FT1f;		/* r6 :    fpext */
	word	R_FT2;		/* r7 :    32bits */
	word	R_FT2f;		/* r7 :    fpext */

							/* TEMP REGISTERS */
	word	R_V3;		/* r8 :    32bits */
	word	R_V3f;		/* r8 :    fpext */
	word	R_V4;		/* r9 :    32bits */
	word	R_V4f;		/* r9 :    fpext */
	word	R_T1;		/* r10:    32bits */
	word	R_T1f;		/* r10:    fpext */

							/* MISC REGISTERS */
	word	R_LR;		/* r11:   link register */
	word	R_LRf;		/* r11:   fpext */
	word	R_V1;		/* dp :   data page pointer */
	word	R_BASE;		/* ir0:   byte address base */
	word	R_USE;		/* ir1:   user stack end pointer */
	word	R_V2;		/* bk :   temporary register */

							/* TEMP BACK-END REGS */
	word	R_TMP1; 	/* rs : */
	word	R_TMP2; 	/* re : */
	word	R_TMP3; 	/* rc : */

	/* Note that iie, iif, die and system stack pointer (sp) are never */
	/* saved by SaveCPUState. */
} CPURegs;

# else
#  error "Processor register structure not defined"
# endif


/* The save state structure holds the CPU context of an unscheduled thread */
typedef struct SaveState {
	struct SaveState *next;		/* for queueing on run Q's */
					/* next MUST be first element */
	struct SaveState *nextknown;	/* for exec housekeeping */
	struct SaveState *stdsavearea;	/* permanent save area */
	word		priority;	/* thread priority */
	uword		endtime;	/* Wakeup time if Sleep()ing */
	word		status;		/* Thread status */
	void		*stack_chunk;	/* current stack chunk header */
	word		TimedWaitUtil;	/* true if OK, false if timedout */
	word		CPUTimeTotal;	/* milliseconds of CPU time used */
	word		LastTimeStamp;	/* Time stamp at last resume/slice */
	word		InitialTime;	/* Startup time of thread (1970 secs) */
	VoidFnPtr	InitialFn;	/* Root fn of thread */
	struct CPURegs	CPUcontext;	/* CPU state of this thread */
} SaveState;

#endif

/* save current thread state, returns FALSE if saved, TRUE if return */
/* is due to a RestoreCPUState() */
extern bool SaveCPUState(SaveState *threadstate);

/* restore a thread state  */
extern void RestoreCPUState(SaveState *threadstate);

# ifdef __C40
/* Save just CPU context, returns FALSE if saved, TRUE if return */
/* is due to a RestoreCPUState2() */
extern word SaveCPUState2(CPURegs *cpustate);

/* restore a CPU context */
extern void RestoreCPUState2(CPURegs *cpustate);
# endif
#endif /* __TRAN */

/* SaveState status values */
/* *Warning*: if these values are changed then update threadps */

#define THREAD_STARTUP		0	/* thread is just starting */
#define THREAD_SLICED		1	/* runnable, was sliced, needs RTI */
#define THREAD_RUNNABLE		2	/* runnable, resheduled, needs RestoreCPUState2() */
#define THREAD_RUNNING		3	/* current CPU thread */
#define THREAD_KILLED		4	/* thread has been Stop()'ed */
#define THREAD_BOGUS		5	/* illegal state of thread */
					/* THREAD_SLICED status in normal dispatch */
#define THREAD_SAVED		6	/* only use for user SaveCPUState() */
#define THREAD_SLEEP		7	/* on timer Q */ 
#define THREAD_TIMEDWAIT	8	/* on timer and semaphore Q's */
#define THREAD_SEMAPHORE	9	/* on semaphore Q */
#define THREAD_MSGREAD		10	/* blocked reading msg */
#define THREAD_MSGWRITE		11	/* blocked writing internal msg */

#define THREAD_MULTIWAIT	12	/* blocked during MultiWait */

#define THREAD_LINKRX		13	/* blocked reading external msg */
#define THREAD_LINKTX		14	/* blocked writing external msg */

#define THREAD_LINKWRITEQ	15	/* blocked on queue to write external msg */
#define THREAD_LINKWAIT		16	/* guardian waiting on dumb link */
#define THREAD_LINKEND		17	/* waiting for linktx/rx to complete */
					/* while in kernel: KillTask, */
					/* Configure, WaitLink or JumpLink */
#define THREAD_LINKXOFF		18	/* waiting for XON on link */

#define THREAD_LINKTHRU1	19	/* single buffering thru-routed msg */
#define THREAD_LINKTHRU2	20	/* double buffering thru-routed msg */

#ifdef __C40
# define THREAD_DMAREQ		21	/* waiting for a DMA engine (unused) */
#endif

#define THREAD_MSGWRITE2	22	/* blocked writing internal msg */


#endif /*__cpustate_h */



/* end of cpustate.h */
