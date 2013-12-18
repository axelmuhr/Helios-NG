/*
 * File:	gexec.h
 * Subsystem:	Generic Helios executive
 * Author:	P.A.Beskeen
 * Date:	Nov '91
 *
 * Description: Generic Helios executive manifests
 *
 *		This file defines the generic executive manifests and
 *		function prototypes.
 *
 * RcsId: $Id: gexec.h,v 1.29 1993/10/04 12:11:18 paul Exp $
 *
 * (C) Copyright 1991 Perihelion Software Ltd.
 * 
 * RcsLog: $Log: gexec.h,v $
 * Revision 1.29  1993/10/04  12:11:18  paul
 * Dispatch latency test added (use -DLATENCYTEST)
 *
 * Revision 1.28  1993/08/11  09:56:07  nickc
 * added ARM support
 *
 * Revision 1.27  1993/06/18  08:55:35  paul
 * fixed prototype for GetRootBase()
 *
 * Revision 1.26  1993/01/21  09:59:44  paul
 * Bootstraps now record HWConfig word in ExecRoot. This is used by the
 * network server as a default when booting other processors. The GetHWConfig()
 * function returns this word
 *
 * Revision 1.25  1992/12/17  11:44:18  paul
 * added fixup for lost end of Tx DMA interrupt. - changed AbortLinkTx
 * proto
 *
 * Revision 1.24  1992/12/03  16:48:09  paul
 * changed names of all WP_ functions to MP_
 *
 * Revision 1.23  1992/11/23  10:11:49  paul
 * converted arguments in linkTx/Rx functions to take word addresses and
 * word sizes for C40 derivative
 *
 * Revision 1.22  1992/11/20  15:33:25  paul
 * chnaged execroot, removing SystemStack and adding CAddressBase
 * altered return parameters from GetSysBase and GetNucleusBase to MPtr
 *
 * Revision 1.20  1992/11/18  14:24:05  paul
 * converted Nucleus pointer to WPTR for C40 and updated CheckState for
 * non 0 IR0
 *
 * Revision 1.19  1992/11/12  17:10:41  paul
 * updated to rationalise Save/RestoreCPUState on C40
 *
 * Revision 1.16  1992/09/25  17:56:06  paul
 * added some new prototypes
 *
 * Revision 1.15  1992/09/25  14:47:33  paul
 * re-ordered function prototypes to show which are processor specific
 *
 * Revision 1.14  1992/09/25  09:40:34  paul
 * removed SystemStackSize, and moved DispatchStack around
 *
 * Revision 1.13  1992/09/23  17:23:11  paul
 * changed defn. of ExecRoot() to GetExecRoot()
 *
 * Revision 1.12  1992/09/21  10:37:11  paul
 * moved the generic executive from the C40 directory to the main kernel
 * dir, so that it can be shared with the ARM implementation
 *
 * Revision 1.11  1992/08/18  09:55:34  paul
 * fixed savestate checks to allow on-chip ram
 *
 * Revision 1.10  1992/07/30  18:31:35  paul
 * fixes for idle monitor, check state not to check upper bound and
 * difftimes to note parameters
 *
 * Revision 1.9  1992/06/30  19:30:20  paul
 * moved KDebug proto from C40.h to here
 *
 * Revision 1.8  1992/06/26  18:00:52  paul
 * added idrom struct to execroot
 *
 * Revision 1.7  1992/06/23  19:12:13  paul
 * fixup After() macro to work, even when compiler doesn't
 *
 * Revision 1.6  1992/06/16  08:44:21  paul
 * added manifest for Kernel Debug buffer and defined the internal version
 * of _SaveCPUState()
 *
 * Revision 1.5  1992/05/14  13:31:05  paul
 *  updated for inline opcode macros in C40 version
 *
 * Revision 1.4  1992/05/14  08:22:30  paul
 * changes to the execroot struct
 *
 * Revision 1.3  1992/04/28  09:23:38  paul
 * added SliceEnabled to execroot
 *
 * Revision 1.2  1992/04/21  09:54:56  paul
 * alpha version
 *
 * Revision 1.1  1991/12/03  11:49:44  paul
 * Initial revision
 *
 *
 */

#ifndef  __gexec_h
#define  __gexec_h


#ifndef in_kernel
# define in_kernel 1
#endif

/* Include Files: */

#include <helios.h>
#include <memory.h>
#include <cpustate.h>	/* for SaveState and CPUContext structures */
#include <string.h>	/* for memset, etc */
#include <link.h>
#include <root.h>


#ifdef __C40
# include "C40/c40exec.h"
#endif

#ifdef __ARM
# include "ARM/armexec.h"
#endif


/*****************************************************************************
 * Misc Structure definitions:
 */

/* The ThreadQ structure holds the runnable threads for each priority's */
/* run Q */
typedef struct ThreadQ {
	SaveState *head;
	SaveState *tail;
} ThreadQ;


/* LinkReq struct hold status of link read/write over interrupts */
typedef struct LinkReq {
	/* @@@ maybe add mask for this links IOCRDY/IOCFULL intr. here? */
	word	Count;	/* number of bytes to read (or words on C40) */
#ifdef __C40
	word	Buf;	/* word address of buffer */
#else
	char *Buf;	
#endif
} LinkReq;


/*****************************************************************************
 * Misc Manifests:
 */

/* Highest (run to completion) priority */
#define HIGHPRI		0

/* Lowest priority - not even user threads can get this low */
#define REALIDLEPRI	PRIORITYLEVELS

#define KDSIZE	64		/* size of kdebug buffer */


/*****************************************************************************
 * The ExecRoot Structure
 *
 * The ExecRoot structure is the only fixed address in Helios all other
 * structures are relative to, or chained off this in some way
 * It holds the executives static data, such as timer and run queues
 *
 * N.B. For C40 version (BPTR) = C byte pointer, (WPTR) = C40 word address
 * Other CPU versions hold std machine pointers for both WPTR and BPTR.
 */

typedef struct ExecRoot {
	SaveState	*CurrentSaveArea; /* Points to current threads */
					/* Save area (BPTR) - *MUST BE* first */
					/* element in ExecRoot */
	word		CurrentPri;	/* Priority of current thread */
					/* The run Q's */
	word		HighestAvailPri;/* Highest priority runnable thread */

					/* **Warning** position of KernelRoot */
					/* must not change unless you update */
					/* root.m GetRoot macro */
	RootStruct	*KernelRoot;	/* kernel RootStruct address (BPTR) */
#ifdef __C40
	word		Nucleus;	/* nucleus load address (WPTR) */
#else
	word		*Nucleus;	/* nucleus load address (BPTR) */
#endif
	word		IdleTime;	/* Amount of time spent in idle */
	uword		Timer;		/* Current Microsecond clock value */
	uword		SliceTime;	/* Clock ticks until next time slice */
	word		TicksPerSlice;	/* quantum given to each slice */
	word		SliceEnabled;	/* TRUE if we can time slice */
	SaveState	*TimerQ;	/* Timer queue for Sleep() (BPTR) */

	ThreadQ		Queues[PRIORITYLEVELS];	/* Run Q's for different */
					/* priorities */

	SaveState	*KnownThreads;	/* List of all known current threads */
					/* chained off SaveState->nextknown */
#ifdef __C40
	word		CAddressBase;	/* Standard system C address base */
	word		HWConfig;	/* default config word for bootstrap */
#else
	VoidFnPtr	CriticalErrorVector;
#endif
					/* Call this function (registered via */
					/* DefineExecErrorHandler()) whenever */
					/* a processor exception occurs */

					/* used by Dispatch() as its own */
					/* temporary stack */
	word		DispatchStack[DISPATCHSTACKSIZE];	/* WPTR */


#if defined(LINKINTR) && ( defined(__C40) || defined(__ARM) )
	/* structures hold count and pointers for that links current */
	/* transaction - set up by LinkRx/Tx and used by interrupt routines */
	LinkReq		LinkInStat[COMMSLINKS];		/* input status */
	LinkReq		LinkOutStat[COMMSLINKS];	/* output status */
#endif

#ifdef __C40
	IDROM		ID_ROM;		/* TIM-40 IDROM contents */

	word		StartupStack;	/* Initial startup stack */
	word		Latency;	/* last timer latency seen */
	word		MaxLatency;	/* max timer latency seen */
	word		MaxLatencyPC;	/* PC of high latency thread */
#endif
#ifdef LATENCYTEST
	word		DispatchLatMs;	/* execroot timer at dispatch */
	Semaphore	DispatchLatCheck;/* kick to high pri thread. */
#endif

#ifdef __ARM
	/* Platform specific static data */
# ifdef __VY86PID
	/* Write only serial interrupt enable register is always shadowed */
	/* here. */
	word		Serial1_IER_softcopy;
# else
	word		spare0;
# endif
	word		spare1;
	word		spare2;
	word		spare3;
	word		spare4;
	word		spare5;
#ifndef LATENCYTEST
/* Space for Dispatch latency test variables. */
	word		spare6;
	word		spare7;
	word		spare8;
	word		spare9;
# endif
#endif

	byte		KDBuffer[KDSIZE]; /* static buffer for KDebug()s	*/
	word		KDBufPos;	/* Position in buffer		*/
	word		KDebugEnable;	/* TRUE = can output KDebugs */
					/* KERNELDEBUG3 support: */
	char		*KD3StartB;	/* start of msg dump buffer */
	char		*KD3EndB;	/* end of msg dump buffer */
	char		*KD3StartM;	/* pointer to start of msgs */
	char		*KD3EndM;	/* pointer to end of msgs */
	Semaphore	KD3Sem;		/* Kick sem to initiate msg send */
#ifdef __ARM
					/* Initial kstart threads stack */
	word		StartupStack[STARTUPSTACKSIZE];
#endif

} ExecRoot;

/* NB the size of the 'C40 execroot structure must NEVER exceed 512 bytes */


/*****************************************************************************
 * The Generic executive provides the following C based Functions:
 *
 * All hardware/processor specific functions required to implement the
 * generic executive are detailed after these functions.
 */

/* Initialisation functions: */

/* Return Id for the type of processor we are running on. */
/* Some processors may implement this as a function that can distinguish */
/* between different variants. */
#if defined(__ARM)
/* Add code to distinguish between arm2/2aS/250/3/6/60/600 */
# define GetCPUType()	0xA2
#elif defined(__C40)
# define GetCPUType()	0x320C40
#elif defined(__i860)
# define GetCPUType()	860
#else
# error "Processor unknown - GetCPUType not defined"
#endif

/* Initialise the ExecRoot data structure. */
void ExecInit(void);

/* Reset all on-chip links, canceling any current transfers */
void ResetLinks(void);

/* Ready on-chip communication links for data transfer */
void InitCommsLinks(void);


/* Access to executive and kernel structures: */

/* Access to executive root structure: */

/*
 * Return the address of the Executive ExecRoot data structure.
 *
 * This data structure contains global Executive information plus private
 * data describing the currently active thread.
 *
 * On the C40 GetExecRoot() returns a C byte address. *boggle warning* The
 * GetExecRoot() call gets secretly converted by the compiler into in-line
 * assembler 'LDEP tvtp, reg'.
 *
 * The ARM uses a fixed low RAM address for the ExecRoot. armexec.h defines
 * ExecRoot_base.
 *
 * Quietly exported from the kernel for use by programs like threadps and
 * slice.
 *
 * GetExecRoot defined early, incase its a #define _word() that can be used in
 * other macros
 */

/*
 * BEWARE - on the C40 there is a conspiracy between the compiler and the
 * kernel.  Any function calls to GetExecRoot() in the kernel (-zr) are
 * silently replaced by the compiler with an in-line version of the code!
 */

ExecRoot * GetExecRoot(void);

#ifdef __ARM
# define GetExecRoot()	((ExecRoot *)ExecRoot_base)
#endif

/* Returns pointer to load point of nucleus.
 *
 * Nucleus may be loaded in global or local bus on 'C40.
 * So pointer to its load address is held in the execroot.
 * Also implemented as a macro for in kernel use (GetSysBase())
 * the two can be used interchangeably.
 *
 * The C40 version returns a word pointer, not a C byte pointer.
 * Exported from the kernel.
 */

MPtr GetNucleusBase(void);

# define GetSysBase() ((MPtr)(GetExecRoot()->Nucleus))

/* Returns pointer to start of kernel root structure.
 * In RAM based systems, this will be directly after the end of the nucleus.
 * In ROM systems the root will be located directly after the executive
 * ExecRoot structure.
 * Also implemented as a macro for in kernel use (GetRoot())
 * the two can be used interchangeably.
 * Exported from the kernel.
 */
RootStruct *GetRootBase(void);

/* pre-calculated at boot time and held in ExecRoot */
#define GetRoot()		(GetExecRoot()->KernelRoot)

/* Returns a pointer to the head pointer of the timer Q.
 * SaveState **TimerQAddr(void); - implemented as a macro.
 */
# define TimerQAddr()		(&(GetExecRoot()->TimerQ))

/* Returns the save state of the first thread on the timer queue.
 * SaveState *TimerQHead(void); - implemented as a macro
 */
# define TimerQHead()		(GetExecRoot()->TimerQ)

/* Returns the ready queue address for the specified priority.
 * ThreadQ  *ReadyQBase(word pri); - implemented as a macro.
 */
# define ReadyQBase(pri)	(&(GetExecRoot()->Queues[pri]))
 
/* Returns the head and tail pointers to the save states on the
 * given priority's run queue.
 */
void RunqPtrs(SaveState **p, word pri);


/* Thread creation functions: */

/* Constructs a machine specific stack environment for the thread. This
 * environment contains "argsize" bytes of data area that can be written to
 * by the owner of the returned pointer. The data area pointer is also used
 * by the "EnterProcess" function to access the process description.
 */
word *CreateProcess(word *stack,VoidFnPtr entry,VoidFnPtr exit,
			word *descript,word argsize);

/* Expects a data structure identical to that returned by the "CreateProcess"
 * call. This structure is machine specific and the format should NOT be
 * published.
 */
void EnterProcess(word *stack, word pri);


/* Scheduler functions: */

/* Dispatch the referenced thread through the Scheduler.
 * Note: Only the current ExecRoot priority is made high. The referenced
 * SaveState priority is preserved.
 */
void Dispatch(SaveState *p);

/* Temporarily suspend a thread (remove it from the run Qs).
 * The reason code is used by `ps' type commands to note the status of
 * executing threads
 */
void Suspend(SaveState **pp, word reason);

/* Suspend a thread with a timeout.
 * This is used to implement TimedWait()
 */
word TimedSuspend(SaveState **p, word timeout);

/* Resume a suspended thread. */
void Resume(SaveState *ss);

/* Resume a suspended thread without enabling interrupts
 * Same as Resume, but doesn't re-enable interrupts - only used by
 * HardenedSignal()
 */
void IntrResume(SaveState *ss);

/* Irrevocable halt of a thread. */
void Stop(void);

/* remove thread from known thread Q. */
void KnownThreadRm(SaveState *rmss);

/* Yield processor to other threads of equal or higher priority.
 * Stops a lengthy high priority operation from hogging the CPU bandwidth
 */
void Yield(void);


/* Time related functions: */

/* Make thread sleep until microsecond clock = endtime. */
void Sleep(word endtime);

/* Return current microsecond timer value.
 *
 * word    Timer(void); - implemented as a macro.
 */
# define Timer()	(GetExecRoot()->Timer)

/* Returns centisecond representation of current microsecond clock.
 * The kernel exports this function. 
 */
word _cputime(void);

/* Return the current microsecond timer value
 * The kernel exports this function. 
 */
word _ldtimer(word pri);


/* Priority related functions: */

/* Temporarily boost a threads priority to HIPRI. */
word System(WordFnPtr func,...);	  /* upto 3 "word" args */

/* Returns the threads original priority after changing it to `newpri'.
 */
word SetPhysPri(word newpri);

/* Returns the current thread priority.
 * word GetPhysPri(void); - Implemented at a macro.
 */
#define GetPhysPri() 		(GetExecRoot()->CurrentPri)

/* Returns the range of available priorities, from 0 (high) to n (low).
 * 'n' being the lowest priority available.
 * Exported from the kernel.
 */
word GetPhysPriRange(void);


/* Module table access: */

/* Call a function while using an alien module table. */
word CallWithModTab(word arg0, word arg1, WordFnPtr fn, word *modtab);

/* Inter-processor communication functions: */

/* Send 'size' bytes pointed to by 'buf' down 'link'. */
/* Non blocking version. */
void LinkTx(word size, LinkInfo *link, void *buf);
#ifdef __C40
void MP_LinkTx(word size, LinkInfo *link, MPtr buf);
#endif

/* Add-on comms link non-blocking transfer. */
#ifdef __C40
void _ELinkTx(word size, LinkInfo *link, MPtr buf);
#else
void _ELinkTx(word size, LinkInfo *link, void *buf);
#endif

/* Abort link tx transfer. */
SaveState *AbortLinkTx(struct LinkInfo *link);


/* Get 'size' bytes of data from the link. */
/* Non blocking version. */
void LinkRx(word size, LinkInfo *link, void *buf);
#ifdef __C40
void MP_LinkRx(word size, LinkInfo *link, MPtr buf);
#endif
/* Add-on comms link non-blocking transfer. */
#ifdef __C40
void _ELinkRx(word size, LinkInfo *link, MPtr buf);
#else
void _ELinkRx(word size, LinkInfo *link, void *buf);
#endif

/* Abort link rx transfer. */
SaveState *AbortLinkRx(struct LinkInfo *link);


/* Executive debugging: */

/* Always available even in non SYSDEB system. */
void KDebug(const char *str, ...);

/* Prints stack backtrace. using KDebug output. */
void backtrace( void );


/*****************************************************************************
 * Each processor implementation must provide the following functions
 * (usually in assembler):
 */

/* Initialisation functions: */

/* Reset the processor and restart boot sequence */
void ResetCPU(void);

/* Low level Reset of all on-chip links */
void ResetLinkHardware(void);

/* Ready on-chip communication link for data transfer, inc. setting up
 * interrupt handler.
 */
void InitLink(int i);

#ifndef __C40
/* Set hardware address to be passed to _/__Link/Rx/Tx() for this link.
 */
void InitLink2(LinkInfo *l);
#endif

/* Initialise and start time slicer. */
void StartTimeSlicer(void);

/* Attach the function to the processors exception vector. The function
 * called should form the root of the syncronous run-time signal handler.
 * i.e. any address/bus errors, div by 0, etc exceptions.
 */
int DefineExecErrorHandler(VoidFnPtr handler);

/* Initialise each general interrupt vector to call the Event handler passed
 * in a PCS conformant fashion.
 */
void InitEventHandler(VoidFnPtr handler);


/* Inter-processor communication functions: */

/* Send 'size' bytes pointed to by 'buf' down 'link'.
 * 'Channel' is simply some form of link identification, could be an
 * address or, a number (must tally with bootstrap - bootlink).
 * Simple blocking version, used to send bogus sync for early debug.
 */
#ifdef __C40
void _LinkTx(word size, Channel link, MPtr buf);
#else
void _LinkTx(word size, Channel link, void *buf);
#endif

/* Non blocking derivative - uses either DMA or interrupt driven transfers. */
#ifdef __C40
void __LinkTx(LinkInfo *link, word linkId, word size, MPtr buf);
#else
void __LinkTx(LinkInfo *link, word linkId, word size, void *buf);
#endif

/* Abort link tx transfer. */
#ifdef __C40
int _AbortLinkTx(LinkInfo *link, word linkId);
#else
void _AbortLinkTx(LinkInfo *link);
#endif

/* Get 'size' bytes of data from the link. */
/* Simple blocking version (used to get config in kernel startup). */
#ifdef __C40
void _LinkRx(word size, Channel link, MPtr buf);
#else
void _LinkRx(word size, Channel link, void *buf);
#endif

/* Non blocking derivative - uses either DMA or interrupt driven transfers. */
#ifdef __C40
void __LinkRx(LinkInfo *link, word linkId, word size, MPtr buf);
#else
void __LinkRx(LinkInfo *link, word linkId, word size, void *buf);
#endif

/* Abort link Rx transfer. */
#ifdef __C40
void _AbortLinkRx(LinkInfo *link, word linkId);
#else
void _AbortLinkRx(LinkInfo *link);
#endif


/* Interrupt handling: */

/* Return the current state of the global interrupt mask. TRUE if interrupts
 * are enabled globally, else false.
 */
word IntsAreEnabled(void);

/*#if !(defined(__C40) || defined(__ARM)) - @@@ until _word() works */
#if !(defined(__C40))
/* ARM and C40 versions use inline opcode macros for these fn's */

/* Disable processor interrupts globally.
 * This does NOT disable hardware interrupt generation, only the processor
 * mask.
 */
void IntsOff(void);

/* Enable processor interrupts globally.
 * This does NOT enable hardware interrupt generation, only the processor mask.
 */
void IntsOn(void);

/* Disable time slicer clock interrupts. */
void ClockIntsOff(void);

/* Enable time slicer clock interrupts. */
void ClockIntsOn(void);
#endif


/* Module table access: */

/* Return the module table pointer for the current task. */
#ifdef __ARM
# if 0
/* @@@ re-install this when _word() works */
/*			    mov a1, mt */
# define _GetModTab() _word(0xe1a00009)
# else
word * _GetModTab(void);
# endif
#else
word * _GetModTab(void);
#endif

/* Set a new module table pointer and return the original. */
word * _SetModTab(word *modtab);


/* CPU context handling functions: */

#ifdef __C40
/* Save the current CPU context into the structure passed as the first
 * argument. This function should also seed the C return register with a 1,
 * but return 0 itself.
 *
 * The standard exported SaveCPUState() and RestoreCPUState() functions are
 * defined in <cpustate.h/m>. All Save/Restore CPU state functions are always
 * coded in assembler.
 *
 * This C40 specific version is different from the standard version in that
 * it doesn't explicitly disable interrupts as it expect this to be done
 * by the caller. Interrupts have to be disabled in the C40 version to get
 * around a slicon bug where updating the SP must be done with interrupts
 * disabled.
 */
word _SaveCPUState(CPURegs *cpusave);
#endif

/* Restore the CPU context from the structure passed as the first
 * argument, and then return as if from an interrupt. The CPU state
 * passed is from a thread that was time sliced.
 * This function is always coded in assembler.
 */
void RestoreSlicedState(CPURegs *cpusave);

/* Debugging functions:
 *
 * Used by _Trace() et al, they simply return PCS register contents
 */
word _spreg(void);
word _fpreg(void);
word _linkreg(void);

/* Puts name of function whose frame pointer is 'frame' into 'name',
 * and returns the frame pointer of the parent of that function.
 */
int _backtrace( char * name, int frame );



/*****************************************************************************
 * Standard executive macros - also used by kernel.
 * All 'p' arguments are save state pointers.
 */

#define P_NullState		NULL
#define NullStateP(p)		((p) == NULL)

#if defined(__C40) && defined(SYSDEB)
# define CheckState(x)	if((x) < (SaveState *)(GetExecRoot())) \
			{_Trace(0xdeaddddd, (word)x, 0); JTAGHalt();}
#elif defined(__ARM) && defined(SYSDEB)
# define CheckState(x)	if((x) < (SaveState *)GetExecRoot() \
			|| (x) > (SaveState *)(GetRoot()->TraceVec)) \
			{_Trace(0xdeaddddd, (word)x, 0); \
			KDebug("%s @ %x: CheckState(%x) FAIL between %x <> %x\n"\
			,__FILE__, __LINE__,x,GetExecRoot(), GetRoot()->TraceVec);}
#else
# define CheckState(p)
#endif

#define P_RunqNext(p)		((p)->next)
#define P_TimerNext(p)		((p)->next)
#define P_EndTime(p)		((p)->endtime)

#define Set_P_RunqNext(p,v)	((p)->next=(v))
#define Set_P_TimerNext(p,v)	((p)->next=(v))
#define Set_P_EndTime(p,v)	((p)->endtime=(v))

#define AddTimes(x,y)		((unsigned long)(x) + (unsigned long)(y))
#define DiffTimes(new, old)	((unsigned long)(new) - (unsigned long)(old))

/* is time x after time y? */
#if 1
/* @@@ should be a temporary fix - depends on ANSI C spec. being followed */
#define After(x,y)		(((word)((unsigned long)(y) - (unsigned long)(x))) & MinInt)
#else
#define After(x,y)		(((word)((unsigned long)(y) - (unsigned long)(x))) < 0)
#endif
#define Forever(x)		((x) == -1)

/* use standard functions from norcroft */
#define SetBlock(b,v,s)		memset(b, (int)v, (size_t)s) 
#define ZeroBlock(b,s)		SetBlock(b, 0, s)
/* move block never encounters overlapping areas */
#define MoveBlock(d,s,z)	memcpy(d, s, (size_t)z)



#endif /* end of gexec.h */
