/*------------------------------------------------------------------------
--                                                                      --
--                     H E L I O S   K E R N E L                        --
--                     -------------------------                        --
--                                                                      --
--             Copyright (C) 1987, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- mcdep.h								--
--                                                                      --
--	Processor dependant definitions for kernel.			--
--									--
--	The system's interface to the hardware is defined here. It	--
--	is based on the Transputer hardware, but some simple		--
--	software should be capable of emulating this.			--
--	The calls defined here can be implemented either as macros,	--
--	assembler procedures, or a mixture of both.			--
--                                                                      --
--	Author:  NHG 8/8/88						--
--                                                                      --
------------------------------------------------------------------------*/
/* RcsId: $Id: mcdep.h,v 1.11 1992/09/21 10:37:11 paul Exp paul $ */
/* Copyright (C) 1987, Perihelion Software Ltd.	*/

#ifndef __mcdep_h
#define __mcdep_h

#ifndef __helios_h
#include <helios.h>
#endif

#include <cpustate.h>		/* for THREAD_status definitions */


#ifdef __ARM

# define	LINKIO	1		/* processor has links		*/
# define	PERFMON 1		/* enable performance monitor	*/
# define	IDLEMON 1		/* enable Idle monitor		*/
# define	ONCHIPRAM 1		/* processor has on-chip RAM	*/

/* The ThreadQ structure holds the runnable threads for each priority's */
/* run Q */
typedef struct ThreadQ {
	SaveState *head;
	SaveState *tail;
} ThreadQ;


/*---------------------------------------------------------------------------*/
/* lo-level functions */

void ResetCPU(void);
/* Reset the processor
 */

void ResetLink(void);
/* Reset the link adapter hardware
 */
 
word ExecRoot(void);
/* Return the address of the Executive ExecRoot data structure. This data
 * structure contains global Executive information plus private data describing
 * the currently active process.
 */

void ExecInit(void);
/* Initialise the ExecRoot data structure and the server IO device. In all
 * current implementations this is a Transputer link adaptor. 
 */

void InitTerminal(void);
/* Initialise and start the system timer and display terminal. The current
 * display is always the Transputer link adaptor. The current system clock
 * provides a centi-second interrupt.
 * This function must be called AFTER "ExecInit".
 */

void IntsOff(void);
/* Disable processor IRQs. This does NOT disable hardware interrupt generation,
 * only the processor mask.
 */

void IntsOn(void);
/* Enable processor IRQs. This does NOT enable hardware interrupt generation,
 * only the processor mask.
 */

int DefineExecErrorHandler(VoidFnPtr handler) ;
/* Attach the function called via the system error vector. The function
 * called should form the root of the run-time signal handler.
 */

#ifndef __ARM
void Dispatch(SaveState *p);
/* Dispatch the referenced process through the Scheduler.
 * Note: Only the current ExecRoot priority is made high. The referenced
 *       SaveState priority is preserved.
 */
#else
/* Helios/ARM specific */
void SchedulerDispatch(SaveState *p);
/* Dispatch the referenced process through the Scheduler. */
#endif

void ExecHalt(void);
/* Terminate Executive execution. This call generates a software abort within
 * the Executive. The currently defined system error handler provides any
 * user output.
 * This call should NOT be returned from.
 */

void MoveBlock(void *dst, void *src, word len);
/* Move a block of data (byte addressed). Blocks will not overlap.
 */

word ExchangeModTab(word dp);
/* Swap current and new module table addresses. IRQs need NOT be disabled since
 * the module table pointer is held in a register and any IRQ process will have
 * its own copy (the register forming part of the process SaveState).
 */

byte TerminalIn(void);
/* Read a character (byte) from the terminal. This function will probably
 * NOT be required by the Server/Executive.
 */

void TerminalOut(byte character);
/* Transmit the single character (byte) to the Server. The data sent does NOT
 * include a terminating NULL.
 */

void Output(char *);
/* Pass the NULL terminated ASCII string to the Server. The data sent WILL
 * include the terminating NULL.
 */

void WriteHex8(word);
/* Debbugging function to Write a eight digit hex number on the I/O Server
 * console.
 */

/* word Divide(word quotient,word divisor);  Acorn lib used at present */
/* Provide 32bit signed division. Returns quotient.
 */

/* word Remainder(word quotient,word divisor); Acorn lib used at present */
/* Provide 32bit signed division. Returns remainder.
 */


/*---------------------------------------------------------------------------*/
/* hi-level functions */

word       System(WordFnPtr func,...);      /* upto 3 "word" args */
/* Boost a low priority process to hipri */

void       Suspend(SaveState **pp, word reason);
/* Temporarily suspend a process (remove it from the scheduling lists) */

word	   TimedSuspend(SaveState **p, word timeout);
/* Temporarily suspend a process, but with a timeout */

void       Resume(SaveState *p);
/* Resume a suspended process (places it back on scheduling list) */

void       Stop(void);
/* Irrevocable halt of a process */

void       Yield(void);
/* Yield hipri process to let any other hipri processes have a go */

void       Sleep(word endtime);
/* Make process sleep until endtime when it is resheduled */

word       Timer(void);
/* Return current timer value */

word CallWithModTab(word arg0,word arg1,WordFnPtr fn,word *modtab);
/* Call a function while using an alien module table */

word      *CreateProcess(word *stack,VoidFnPtr entry,VoidFnPtr exit,
						word *descript,word argsize) ;
/* Constructs a machine specific stack environment for the process. This
 * environment contains "argsize" bytes of data area that can be written to
 * by the owner of the returned pointer. The data area pointer is also used
 * by the "EnterProcess" function to access the process description.
 */

void       EnterProcess(word *stack,word pri);
/* Expects a data structure identical to that returned by the "CreateProcess"
 * call. This structure is machine specific and the format should NOT be
 * published.
 */

SaveState *TimerQHead(void) ;
/* Returns the process description for the process at the front of the timer
 * queue.
 *
 * in:  void
 * out: A "SaveState *" for the referenced process
 */

SaveState **TimerQAddr(void) ;
/* Returns a pointer to the head pointer of the timer Q.
 *
 * in:  void
 * out: A "SaveState **" to the head process on the timerq
 */

void RunqPtrs(SaveState **p, word pri) ;
/* Returns the head and tail process descriptions for the processes on the
 * given priority process queue.
 *
 * in:  A pointer to an array of "SaveState *" entries
 * out: void
 */

ThreadQ *ReadyQBase(word pri) ;
/* Returns the ExecRoot structure entry address for the specified ready queue.
 *
 * in:  The desired priority queue.
 * out:	"ProcessQ" structure pointer.
 */
 
word SetPhysPri(word newpri);
/* Returns the old priority of the process, after changing the processes
 * priority to 'newpri'.
 */

word GetPhysPri(void);
/* Returns the current process priority
 */

word GetPhysPriRange(void);
/* Returns the range of available priorities, from 0 (high) to n (low).
 * 'n' being the lowest priority available.
 */

word *GetNucleusBase(void);
/* Returns pointer to load point of nucleus. */

word *GetRootBase(void);
/* Returns pointer to start of kernel root structure.
 * In RAM based systems, this will be directly after the end of the nucleus.
 * In ROM systems the root will be located at the start of RAM.
 */

#define _LinkTx(x,y,z) LinkTx(x,y,z)
void LinkTx(word size,struct LinkInfo *link,void *buf);
/* Send data down link */

SaveState *AbortLinkTx(struct LinkInfo *link);
/* Abort link tx transfer */

#define _LinkRx(x,y,z) LinkRx(x,y,z)
void LinkRx(word size,struct LinkInfo *link,void *buf);
/* Get data down link */

SaveState *AbortLinkRx(struct LinkInfo *link);
/* Abort link rx transfer */

word _cputime(void);
/* returns centisecond ticks since we started */

word _ldtimer(word pri);
/* pri = 0, returns microsecond timer, else pri = 1 returns centisecond timer */

void InitEventHandler(VoidFnPtr handler) ;
/* Initialise the root event handler */

/* The following are in "asm.a" and NOT in "armexec/hiexec-a" */
word *_GetModTab(void) ;
/* Return the module table pointer for the current task */

#if 1 /* JGS 910404 */
void SetBlock(void *address,word value,word length) ;
#endif

/*---------------------------------------------------------------------------*/
/* pre-processor macro functions */

/* Miscellaneous */

#define AddTimes(x,y)  ((unsigned)(x) + (unsigned)(y))
#define DiffTimes(x,y) ((unsigned)(x) - (unsigned)(y))
#define After(x,y)     ( ((word)((unsigned)(y) - (unsigned)(x))) < 0)
/*#define After(x,y)     (((y) - (x)) < 0)*/
#define Forever(x) ((x) == -1)

#if 0 /* JGS 910404 (see above) */
/* void SetBlock(void *block,word value,word size); */
# define SetBlock(b,v,s) *(word *)(b) = (v);                                  \
                        MoveBlock((word *)(b)+1,(b),(s)-4)
/* Set a block of memory to the given value */
#endif

/* void ZeroBlock(void *block,word size); */
#define ZeroBlock(b,s) SetBlock((b),0,(s))
/* Zero a block of memory */

#define MinProcessStack (sizeof(SaveState)+4*16)
#define CheckState(p)

#define P_NullState	((SaveState *)NULL)
#define NullStateP(p)   ((p) == (SaveState *)NULL)
/* Check if SaveState is valid. If the SaveState is NULL then no process is
 * referenced.
 */

#define P_InstPtr(p)   ((p)->pc)
/* Return the program counter (address of the next instruction) for the
 * referenced process.
 */

#define P_RunqNext(p)  ((p)->next)
/* Return the next process in the chain */

#define P_TimerNext(p) ((p)->next)
/* Return the next process in the timer chain */

#define P_PreemptNext(p)	((p)->next)
/* Return the next process in the pre-empted process queue */

#define P_EndTime(p)   ((p)->endtime)
/* Return the "endtime" for the referenced process */

/* void Set_P_InstPtr(SaveState *p,VoidFnPtr); */
#define Set_P_InstPtr(p,v)      ((p)->pc = (v))
/* Set the program counter address for the referenced process. This will be the
 * address used to fetch the next instruction when the process is re-started.
 */

/* void Set_P_RunqNext(SaveState *p,SaveState *v); */
#define Set_P_RunqNext(p,v)     ((p)->next = (v))
/* Define the next process referenced by the process chain */

/* void Set_P_TimerNext(SaveState *p,SaveState *v); */
#define Set_P_TimerNext(p,v)    ((p)->next = (v))
/* Define the next process referenced by the timer process chain */

/* void Set_P_PreemptNext(SaveState *p,SaveState *v) ; */
#define Set_P_PreemptNext(p,v)	((p)->next = (v))
/* Define the next process referenced by the pre-empted process chain */

/* void Set_P_EndTime(SaveState *p,word v); */
#define Set_P_EndTime(p,v)      ((p)->endtime = (v))
/* Set the "endtime" for the referenced process */ 

#endif /* HELIOSARM */

/*---------------------------------------------------------------------------*/


#ifdef TRANSPUTER

# define	LINKIO	1		/* processor has links		*/
# define	PERFMON 1		/* enable performance monitor	*/
# define	IDLEMON 0		/* disable Idle monitor		*/
# define	ONCHIPRAM 1		/* processor has on-chip RAM	*/

#include <asm.h>

/* Scheduler control */

#define System HiPri
extern word HiPri(WordFnPtr func,...);

typedef word SaveState;

#define P_NullState	((SaveState *)MinInt)
#define NullStateP(p)	((p) == (SaveState *)MinInt)

#define P_InstPtr(p)	((VoidFnPtr)  ((p)[-1]))
#define P_RunqNext(p)	((SaveState *)((p)[-2]))
#define P_BufAddr(p)	((byte *)     ((p)[-3]))
#define P_TimerNext(p)	((SaveState *)((p)[-4]))
#define P_EndTime(p)	((word)       ((p)[-5]))

#define Set_P_InstPtr(p,v)	(((p)[-1])=(word)v)
#define Set_P_RunqNext(p,v)	(((p)[-2])=(word)v)
#define Set_P_BufAddr(p,v)	(((p)[-3])=(word)v)
#define Set_P_TimerNext(p,v)	(((p)[-4])=(word)v)
#define Set_P_EndTime(p,v)	(((p)[-5])=(word)v)

#define	MinProcessStack		(32)

#define TimerQHead()	(*((SaveState **)MinInt+10))
#define Set_TimerQHead(v)	(*((SaveState **)MinInt+10)=(SaveState *)(v))

#define RunqPtrs(x)	savel_(x);

#define Suspend(p, r)				\
	*(p) = (SaveState *)(ldlp_(0)|ldpri_());\
	stopp_();

#ifdef KDEBUG	
#define BadState(p)							\
	((word)(p) <= (word)(0x80000070) || 				\
	 (word)(p) >= (word)(0x807ff000) ||				\
	 ((word)(p) >= (word)(0x80001000) &&				\
          (word)(p) <= (word)(0x80001000 + *(word *)0x80001000)))

/* BadPC to be used only when we know that process has suspended in kernel */
#define BadPC(p)							\
	(((word *)(p))[-1] <= (word)(0x80000070) || 			\
	 ((word *)(p))[-1] >= (word)(0x807ff000) ||			\
	 (((word *)(p))[-1] < (word)(0x80001000) &&			\
          ((word *)(p))[-1] > (word)(0x80001004 + *(word *)0x80001004)))

#define Resume(p) 							\
	if( BadState(p) || BadPC(p) )					\
	{ _Trace(0x77771111,&(p)); for(;;); }				\
	else runp_(p);

#define CheckState(p)							\
	if( BadState(p) || BadPC(p) )					\
	{ _Trace(0x77772222,&(p)); for(;;); }
#else
#define Resume(p) runp_(p);
#define CheckState(p)
#endif

#define Yield()					\
	runp_(ldlp_(0)|ldpri_());		\
	stopp_();

#define Timer()  ldtimer_()

#define Sleep(t) tin_(sum_(ldtimer_(),(t)))

#define CreateProcess(stack,entry,exit,modtab,args)		\
	(((word *)(stack))[-(1+(args/4))] = (word)(modtab),	\
	 ((word *)(stack))[-(2+(args/4))] = (word)(exit),	\
	 ((word *)(stack))[-(3+(args/4))] = (word)(entry),	\
	 ((word *)(stack))[-(4+(args/4))] = (word)(MinInt),	\
	 &((word *)(stack))[-(args/4)])		

#define EnterProcess(wsp,pri) runp_(((word)(wsp)-8)|pri)

#define Stop()	stopp_()

/* External message passing	*/

#if 0

#define LinkTx(size,link,buf) out_(size,link->TxChan,buf)

#define LinkRx(size,link,buf) in_(size,link->RxChan,buf)

#define AbortLinkTx(link) (SaveState *)resetch_(link->TxChan)

#define AbortLinkRx(link) (SaveState *)resetch_(link->RxChan)

#else

#define LinkTx(size,link,buf) if(link->TxFunction)link->TxFunction(size,link,buf);else out_(size,link->TxChan,buf)

#define LinkRx(size,link,buf) if(link->RxFunction)link->RxFunction(size,link,buf);else in_(size,link->RxChan,buf)

#define AbortLinkTx(link) (SaveState *)(link->TxFunction?link->TxFunction(0,link,0):resetch_(link->TxChan))

#define AbortLinkRx(link) (SaveState *)(link->RxFunction?link->RxFunction(0,link,0):resetch_(link->RxChan))

#endif

/* Inter-domain access */

void CallWithModTab(word arg0, word arg1, WordFnPtr fn, word *modtab);


/* Timer value arithmetic - to avoid overflow errors */
#define After(x,y) (diff_(x,y) < 0)

#define AddTimes(x,y) (sum_(x,y))
/*#define AddTimes(x,y) (sum_(x,y) & ~1)*/

#define DiffTimes(x,y) (diff_(y,x))

#define Forever(x) ((x) == -1)

/* Miscellaneous */
#define SetBlock(b,v,s)				\
	*(word *)(b) = v;			\
	move_(s-4,(word)(b)+4,b);

#define ZeroBlock(b,s) SetBlock(b,0,s)

#define MoveBlock(d,s,z) move_(z,d,s)

#define GetModTab(arg0) (word *)((&arg0)[-1])

#endif /* TRANSPUTER */

#ifdef TESTER

# define	LINKIO	1		/* processor has links		*/
# define	ONCHIPRAM 1		/* processor has on-chip RAM	*/

# include <asm.h>

typedef struct SaveState {
	struct SaveState	*next;		/* next process in list	*/
	word			endtime;	/* timeout or save mode	*/
	word			pri;		/* priority		*/
	VoidFnPtr		pc;		/* saved pc		*/
	VoidFnPtr		*sp;		/* saved sp		*/
} SaveState;

#define P_NullState	((SaveState *)MinInt)
#define NullStateP(p)	((p) == (SaveState *)MinInt)

#define P_InstPtr(p)	((p)->pc)
#define P_RunqNext(p)	((p)->next)
#define P_TimerNext(p)	((p)->next)
#define P_EndTime(p)	((p)->endtime)

#define Set_P_InstPtr(p,v)	((p)->pc=(v))
#define Set_P_RunqNext(p,v)	((p)->next=(v))
#define Set_P_TimerNext(p,v)	((p)->next=(v))
#define Set_P_EndTime(p,v)	((p)->endtime=(v))

#define	MinProcessStack		(sizeof(SaveState)+3*16)

void ExecInit(void);
word System(WordFnPtr func, ... );
SaveState *TimerQHead(void);
void RunqPtrs(SaveState **p);
void Suspend(SaveState **p, word reason);
void Resume(SaveState *p);
void Yield(void);
word Timer(void);
void Sleep(word time);
void EnterProcess(word *stack, word pri, VoidFnPtr entry, VoidFnPtr exit, 
			word *modtab, word arg1, word arg2);
void Stop(void);

void CallWithModTab(word arg0, word arg1, WordFnPtr fn, word *modtab);

/* further machine specific stuff	*/

#define SavedSP()	(((word **)MinInt)[11])
#define SavedPC()	(((VoidFnPtr **)MinInt)[12])
#define SavedRegs()	&(((word *)MinInt)[13])
#define Set_SavedSP(v)	(((word *)MinInt)[11] = (word)(v)|1)
#define Set_SavedPC(v)	(((word *)MinInt)[12] = (word)(v))

#define IntsOff()	_setpri(0)
#define IntsOn()	_setpri(1)

/* Miscellaneous */
/*#define After(x,y) (gt_(0,diff_(y,x)))*/
/*#define After(x,y) ((x-y)>0)*/
#define After(x,y) (diff_(x,y) < 0)

#define SetBlock(b,v,s)				\
	*(word *)(b) = v;			\
	move_(s-4,(word)(b)+4,b);

#define ZeroBlock(b,s) SetBlock(b,0,s)

#define MoveBlock(d,s,z) move_(z,d,s)

#define GetModTab(arg0) (word *)((&arg0)[-1])

void LinkTx(word size,struct LinkInfo *link,void *buf);
void LinkRx(word size,struct LinkInfo *link,void *buf);
SaveState *AbortLinkTx(struct LinkInfo *link);
SaveState *AbortLinkRx(struct LinkInfo *link);

#endif /* TESTER */

#ifdef __I860

#define LINKIO	1		/* processor has links		*/

#include "exec.h"

#define P_NullState	((SaveState *)MinInt)
#define NullStateP(p)	((p) == (SaveState *)MinInt)
#define CheckState(p)

#define P_RunqNext(p)	((p)->Next)
#define P_TimerNext(p)	((p)->Next)
#define P_EndTime(p)	((p)->EndTime)

#define Set_P_RunqNext(p,v)	((p)->Next=(v))
#define Set_P_TimerNext(p,v)	((p)->Next=(v))
#define Set_P_EndTime(p,v)	((p)->EndTime=(v))


#define AddTimes(x,y)  ((unsigned)(x) + (unsigned)(y))
#define DiffTimes(x,y) ((unsigned)(x) - (unsigned)(y))
#define After(x,y)     ( ((word)((unsigned)(y) - (unsigned)(x))) < 0)
/*#define After(x,y)     (((y) - (x)) < 0)*/
#define Forever(x) ((x) == -1)

#define SetBlock(b,v,s)	 memset(b,v,s) 

#define ZeroBlock(b,s) SetBlock(b,0,s)

#define MoveBlock(d,s,z) memmove(d,s,z)

#define IntsOff	Disable_Ints
#define IntsOn	Enable_Ints

#define SetPhysPri	_ChangePriority
void LinkTx(word size,struct LinkInfo *link,void *buf);
void LinkRx(word size,struct LinkInfo *link,void *buf);
void LinkTxP(word size,struct LinkInfo *link,void *buf);
void LinkRxP(word size,struct LinkInfo *link,void *buf);
SaveState *AbortLinkTx(struct LinkInfo *link);
SaveState *AbortLinkRx(struct LinkInfo *link);

#endif /* __I860 */

#if defined(__C40) || defined(__ARM)
# include "gexec.h"
#endif /* __C40 || __ARM */

#endif /* __mcdep_h */



/* -- End of mcdep.h */
