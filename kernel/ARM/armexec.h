/*
 * File:	armexec.h
 * Subsystem:	ARM Helios executive
 * Author:	P.A.Beskeen
 * Date:	Sept '92
 *
 * Description: ARM specific Helios executive manifests. These allow you
 * 		to change the basic parameters of the ARM executive.
 *
 * WARNING:	Changes to these values should be reflected in "armexec.m"
 *
 * RcsId: $Id: armexec.h,v 1.1 1992/09/25 15:49:45 paul Exp paul $
 *
 * (C) Copyright 1992 Perihelion Software Ltd.
 * 
 * RcsLog: $Log: armexec.h,v $
 * Revision 1.1  1992/09/25  15:49:45  paul
 * Initial revision
 *
 *
 */

#ifndef __armexec_h
#define __armexec_h

#include <arm.h>


#if defined(__ARCHIMEDES) && !defined(__IOC)
# define __IOC			/* Archi. uses ARM IOC chip.		*/
#endif

#define DBG(x) {char y = x | 0x80; _LinkTx(1,(Channel)0x03342000,&y);}
#define DBGW(x) {int y = x | 0x80808080; _LinkTx(4,(Channel)0x03342000,&y);}

/* Define executive/kernel options: */

#define	LINKIO		1	/* Processor has links.			*/

#ifdef LINKIO
# define LINKINTR	1	/* Needs LinkReq structs in ExecRoot.	*/

# define COMMSLINKS	4	/* Maximum Number of comms links */
#endif

#define	PERFMON		1	/* Enable performance monitor.		*/

#ifdef __ABC
# define ONCHIPRAM	1	/* If processor has on-chip RAM.	*/
#endif

#define PRIORITYLEVELS	8	/* Number of priority levels.		*/


/* Define executive parameters: */

/*
 * DEFAULTTICKSPERSLICE defines the default number of clock ticks for each
 * thread before it is sliced - currently the clock ticks at one millisecond
 * intervals.
 */
#define DEFAULTTICKSPERSLICE	10	/* 1/100 of a second slice */


/*
 * DISPATCHSTACKSIZE is the maximum size of stack required for a call to
 * Dispatch(). It is use to allow a thread to stay in the dispatchers idle
 * loop until a new thread is sheduled, but still allow interrupts to occur
 * Must include the size of a SaveState structure.
 */
#define DISPATCHSTACKSIZE	48	/* words of stack space */


/*
 * STARTUPSTACKSIZE is the size of the stack required for the first
 * thread used to initialise the system. @@@ This could well be smaller.
 */
#define STARTUPSTACKSIZE	384	/* words of stack space */


/* Define ExecRoot position in low RAM. *WARNING* If this manifest is changed,
 * then update ExecRoot_base in armexec.h.
 */
#if 0
# define ExecRoot_base		0x2800
#else
/* Leave first 32k free for bootstrap/monitor system */
/* *WARNING* If this constant is changed then armexec.m must also be updated */
# define ExecRoot_base		0x9800
#endif

/* Set interrupt level, if in user mode, use trap interface. */

#ifdef __ARM6
#if 1
/* Continue to use standard functions at the moment. */
/* The macros currently have side-effects so cannot safely be used. */
#else
/* The macros will take (3N + 4S + 1I) to execute. If we provided functions
 * like:
 *      IntsOff
 *          MRS a1,cpsr
 *          ORR a1,a1,#&80
 *          MSR cpsr_all,a1
 *          MOV pc,lr
 *
 * they would take (2N + 7S) to execute, including the entry BL. The
 * functions rely on APCS defining that a1-a4, ip and lr do not need to be
 * preserved over a function call. It is the fact that the compiler
 * does not "lose" these registers when using the in-line code that will
 * make most of the difference.
 *
 * The code is further complicated by the fact that we cannot
 * manipulate the IRQ control flag directly from USR mode. The code
 * checks if the thread is executing in USR mode, and executes a suitable
 * SWI instruction instead.
 *
 * We use "_word" to introduce all of the code, since the compiler
 * only has global register allocations and not local ones. If we used a
 * global register allocation (__global_reg(n)) then that register would
 * be lost to the rest of the world.
 */

/* NASTY ARM instruction constants */
# define ARM_STMFD_sp_a1    (0xE92D0001)             /* 2N */
# define ARM_LDMFD_sp_a1    (0xE8BD0001)             /* 1N + 1S + 1I */
# define ARM_MRS_a1_cpsr    (0xE10F0000)             /* 1S */
# define ARM_MSR_cpsr_a1(c) (0x0129F000 | (c))       /* 1S */
# define ARM_ORR_a1_IRQ(c)  (0x03800080 | (c))       /* 1S */
# define ARM_BIC_a1_IRQ(c)  (0x03C00080 | (c))       /* 1S */
# define ARM_TST_a1_USR     (0xE310000F)             /* 1S */
# define ARM_SWI(c,n)       (0x0F000000 | (c) | (n)) /* 2S + 1N */
# define ARM_EQ             (0x0)
# define ARM_NE             (0x1)

# define IntsOn()   {                                     \
                     _word(ARM_STMFD_sp_a1) ;             \
                     _word(ARM_MRS_a1_cpsr) ;             \
                     _word(ARM_TST_a1_USR) ;              \
                     _word(ARM_BIC_a1_IRQ(ARM_NE)) ;      \
                     _word(ARM_MSR_cpsr_a1(ARM_NE)) ;     \
                     _word(ARM_LDMFD_sp_a1) ;             \
                     _word(ARM_SWI(ARM_EQ,TRAP_IntsOn)) ; \
                    }
# error "IntsOn() in-line code has side-effects" /* the PSR state has been changed */
/* Since the compiler may expect the PSR to be preserved over _word() calls, we cannot do this */

# define IntsOff()  {                                      \
                     _word(ARM_STMFD_sp_a1) ;              \
                     _word(ARM_MRS_a1_cpsr) ;              \
                     _word(ARM_TST_a1_USR) ;              \
                     _word(ARM_ORR_a1_IRQ(ARM_NE)) ;       \
                     _word(ARM_MSR_cpsr_a1(ARM_NE)) ;      \
                     _word(ARM_LDMFD_sp_a1) ;              \
                     _word(ARM_SWI(ARM_EQ,TRAP_IntsOff)) ; \
                    }
# error "IntsOn() in-line code has side-effects" /* the PSR state has been changed */
#endif
#else
#if 0 /* see comments associated with ARM6 in-line code above */
# define IntsOff()	{_word(0xE33FF3C3);} /* TEQP psr,#(INTflags | SVCmode) */
# define IntsOn()	{_word(0xE33FF003);} /* TEQP psr,#(0 | SVCmode) */
#else
/* continue to use std functions for the moment */
#endif
#endif


/* ARM specific function prototypes */

/* Initialise ARM CPU specific details */

void ARMExecInit(void);


/* Initialise any system hardware specific details */

void HWSpecificInit(void);


/* Initialise trap table to point @ trap fn's */

void InitTrapTable(void);

/* Initialise Exception vector to point at handlers (includes
 * IRQ and FIQ vectors)
*/
void InitExceptionVectors(void);


/* Preload FIQ and IRQ banked registers */
void InitExceptionStacks(void);


/* Exported processor interrupt mask update */

void EnableIRQ(void);
void DisableIRQ(void);

void EnableFIQ(void);
void DisableFIQ(void);


/* Exported processor mode alteration */

void KernelEnterSVCMode(void);	/* For internal use only and must not be used */
				/* during a trap from a user thread as this */
				/* would corrupt the user mode link register .*/
void EnterSVCMode(void);	/* For export use only! */
void EnterUserMode(void);	/* For internal and export use. */


#endif /* __armexec_h */



/* End of armexec.h */
