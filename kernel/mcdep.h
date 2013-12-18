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
/* RcsId: $Id: mcdep.h,v 1.12 1992/09/22 08:49:18 paul Exp $ */
/* Copyright (C) 1987, Perihelion Software Ltd.	*/

#ifndef __mcdep_h
#define __mcdep_h

#ifndef __helios_h
#include <helios.h>
#endif

#include <cpustate.h>		/* for THREAD_status definitions */


#if defined(__C40) || defined(__ARM)
/* The generic executive header (gexec.h) defines all of the common functions */
/* and includes processor specific definitions from their own header file */
# include "gexec.h"
#endif /* __C40 || __ARM */


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

#endif /* __mcdep_h */



/* -- End of mcdep.h */
