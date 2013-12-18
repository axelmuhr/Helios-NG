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
/* RcsId: $Id: cpustate.h,v 1.18 1993/07/27 13:59:18 paul Exp tony $ */

#ifndef __cpustate_h
#define __cpustate_h

#ifndef __TRAN	/* None of this is applicable to the transputer. */

#ifndef __helios_h
# include <helios.h>
#endif


#ifdef __ARM
/*		C REG NAME	ASM ALIASES	PCS USE:		*/
typedef struct CPURegs {
	word	R_A1;	   /*	a1/r0		argument variables	*/
	word	R_A2;	   /*	a2/r1 					*/
	word	R_A3;	   /*	a3/r2 					*/
	word	R_A4;	   /* 	a4/r3					*/
	word	R_V1;	   /* 	v1/r4		register variables	*/
	word	R_V2;	   /* 	v2/r5					*/
	word	R_V3;	   /* 	v3/r6					*/
	word	R_V4;	   /* 	v4/r7					*/
	word	R_V5;	   /*	v5/r8					*/
	word	R_MT;	   /*	mt/dp/r9	module table pointer	*/
	word	R_USE;	   /*	use/sl/r10	stack limit		*/
	word	R_FP;	   /*	fp/r11		frame pointer		*/
	word	R_TMP;	   /*	tmp/ip/r12	temporary register	*/
	word	R_SVC_SP;  /*	r13_svc		SVC stack pointer	*/
	word	R_SVC_LR;  /*	r14_svc		SVC link register	*/
	word	R_PC;	   /*	pc/st/r15	program counter		*/
	word	R_USER_SP; /*	usp/sp/r13	User mode stack pointer	*/
	word	R_USER_LR; /*	lr/lk/r14	User mode link register	*/
	word	R_CPSR;    /*	cpsr		ARM6 psr		*/
} CPURegs;
#endif /* __ARM */


#ifdef __C40
typedef struct CPURegs {
	     /* C PCS BINDING NAME: */
				/* 'C40 REGISTER NAME: */
				        /* FUNCTION: */
							/* TYPE: */

	uword	PC;		/*	   PC of sliced thread */
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
	uword	R_LR;		/* r11:   link register */
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
#endif /* __C40 */


/* The save state structure holds the CPU context of an unscheduled thread. */
typedef struct SaveState {
	struct SaveState *next;		/* For queueing on run Q's */
					/* Next MUST be first element */
	struct SaveState *nextknown;	/* For exec housekeeping */
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


/* Function prototypes: */

/* Save current CPU state, returns FALSE if saved, TRUE if return */
/* is due to a RestoreCPUState(). RestoreCPUState() might not save some */
/* of the temporary and argument PCS registers */
extern bool SaveCPUState(CPURegs *cpustate);

/* Restore a CPU state.  */
extern void RestoreCPUState(CPURegs *cpustate);
#endif /* __TRAN */


/* SaveState status values: */
/* *Warning* If these values are changed then update threadps. */

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

