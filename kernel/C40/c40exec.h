/*
 * File:	c40exec.h
 * Subsystem:	'C40 Helios executive
 * Author:	P.A.Beskeen
 * Date:	Nov '91
 *
 * Description: 'C40 specific Helios executive manifests. These allow you
 * 		to change the basic parameters of the 'C40 executive.
 *
 * WARNING:	Changes to these values should be reflected in "c40exec.m"
 *
 * RcsId: $Id: c40exec.h,v 1.14 1993/10/04 12:12:37 paul Exp $
 *
 * (C) Copyright 1991 Perihelion Software Ltd.
 * 
 * RcsLog: $Log: c40exec.h,v $
 * Revision 1.14  1993/10/04  12:12:37  paul
 * added support for max dispatch latency test
 *
 * Revision 1.13  1993/02/05  11:12:56  paul
 * fixed timeslice to 100th of a sec rather than a tenth
 *
 * Revision 1.12  1992/11/25  13:20:15  nick
 * types of _GetAddrBase() and _SetAddrBase() chaged to agree with <c40.h>.
 *
 * Revision 1.11  1992/11/18  14:40:20  paul
 * removed CHECKSS - now use one std CheckSaveState() macro
 *
 * Revision 1.10  1992/08/18  09:53:47  paul
 * added Get/SetAddrBase protos, allowed on-chip ram to be legal position in
 * checks for .pc/.lr
 *
 * Revision 1.9  1992/08/04  18:01:47  paul
 * added proto
 *
 * Revision 1.8  1992/07/30  18:33:39  paul
 * removes idle mon and stops checking savestate address uppper bound
 *
 * Revision 1.7  1992/06/26  17:57:32  paul
 * changed ticksperslice to just a default, not fixed
 *
 * Revision 1.6  1992/06/16  08:43:58  paul
 * added GTrace prototype
 *
 * Revision 1.5  1992/05/14  13:30:36  paul
 *  updated for inline opcode macros
 * ,.
 *
 * Revision 1.4  1992/05/14  08:45:16  paul
 * changed timeslice to 1/10th of a sec
 *
 * Revision 1.3  1992/04/28  09:23:10  paul
 * added CHECKSS macro to check savestate validity
 *
 * Revision 1.2  1992/04/21  09:54:56  paul
 * alpha version
 *
 * Revision 1.1  1991/12/03  11:49:44  paul
 * Initial revision
 *
 *
 */

#ifndef __c40exec_h
#define __c40exec_h

#include <c40.h>
#include <event.h>	/* for IntsOn/Off() */

#define LINKDMA	1	/* define if we are using DMA in concert with links */

#ifndef LINKDMA
# define LINKINTR 1
#endif

#define	LINKIO	1		/* processor has links		*/
#define	PERFMON 1		/* enable performance monitor	*/
#define	ONCHIPRAM 1		/* processor has on-chip RAM	*/

/* Number of priority levels */
#define PRIORITYLEVELS		8

/* Number of on-chip comms links */
#define COMMSLINKS		6

/* number of on-chip DMA engines */
#define DMAENGS			6

/* DEFAULTTICKSPERSLICE defines the default number of clock ticks for each */
/* thread before it is sliced - currently the clock ticks at one millisecond */
/* intervals */
#define DEFAULTTICKSPERSLICE	10	/* 1/100 of a second slice */

/* DISPATCHSTACKSIZE is the maximum size of stack required for a call to */
/* Dispatch(). It is use to allow a thread to stay in the dispatchers idle */
/* loop until a new thread is sheduled, but still allow interrupts to occur */
#define DISPATCHSTACKSIZE	40	/* words of stack space */

/* Enable interrupts and idle until an interrupt occurs */
/* On the 'C40 this means Execute the IDLE instruction */
void IdleUntilInterrupt(void);


/* 'C40 specific internal DMA functions */
#ifdef ALLOCDMA /* @@@ upgrade when we re-implement these functions */
void InitDMA(void);
word _AllocDMA(RootStruct *root, word timeout, word link, word flags);
void _FreeDMA(RootStruct *root, word DMAEng);
word AvailDMA(void);	/* @@@? return number of unused DMA engines */
word MapDMA(void);	/* @@@? return bitmap of available DMA engines */
#endif


/* 'C40 specific link communication functions */
int TxFIFOSpace(Channel);
int RxFIFOSpace(Channel);
#ifdef LINKDMA
void InitLinkDMA(word DMAEng);
#else
void InitLink(word channel);
#endif


/* C40 specific C data address base functions */
MPtr _GetAddrBase(void);
void _SetAddrBase(MPtr addrbase);


/* kernel/exec debugging functions */
void WriteWord(word x);	/* tmp debug */
word ReadWord(void);	/* tmp debug */
word GRead(word addr);	/* tmp debug */
void GTrace(word data); /* tmp dbg - write to global trace buffer */
void GWrite(word addr, word data);			/* tmp debug */
void GWriteB(word addr, word size, word *buffer);	/* tmp debug */
void DumpComms(word size, LinkInfo *link, void *buf);	/* tmp dbg */

#ifdef LATENCYTEST
void DispatchLatTest(void);
#endif


#endif /* __c40exec_h */


/* end of c40exec.h */
