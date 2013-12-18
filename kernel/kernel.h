/*------------------------------------------------------------------------
--                                                                      --
--                     H E L I O S   K E R N E L                        --
--                     -------------------------                        --
--                                                                      --
--             Copyright (C) 1987, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- kernel.h								--
--                                                                      --
--	Machine independant kernel header.				--
--	This contains mostly definitions and structures which I do not	--
--	want the plebs to know about.					--
--                                                                      --
--	Author:  NHG 8/8/88						--
--                                                                      --
------------------------------------------------------------------------*/
/* $Id: kernel.h,v 1.21 1994/06/29 14:57:57 tony Exp $ */

#ifndef __kernel_h
#define __kernel_h

#ifndef in_kernel
# define in_kernel 1	/* some headers use this		*/
#endif

#ifndef __helios_h
#include <helios.h>
#endif

#define _ID_

#include <memory.h>
#include <root.h>
#include "mcdep.h"		/* machine dependant defs.	 	*/
#include <config.h>
#include <codes.h>
#include <task.h>
#include <message.h>


/* defines size of kernel message debug dump buffer */
#if !defined(KERNELDEBUG3) || !defined(SYSDEB)
# define KDEBUG3SIZE	0
#else
/* enough for ~10 screens worth of dumped text */
# define KDEBUG3SIZE	(30*1024)
void InitKDebug(void);
#endif

/* Extras for Ports */

#define Port_Index_mask	0xffff		/* mask for full index		*/
#define Port_Cycle_shift 16		/* shift for port cycle field	*/

/* While the first definition of PortCycle_() is faster on the 		*/
/* transputer the second is more portable.				*/
#ifdef __TRAN
# define PortCycle_(p) (((char *)&p)[2])
#else
# define PortCycle_(p) (((unsigned)p>>Port_Cycle_shift)&0xff)
#endif

/* On little-endian processors the cycle field of the PTE is in the same */
/* position in the first word as the cycle field in the port descriptor	 */
/* the following macro exploits this to produce a faster comparison.	 */
/* Actually this is not very good on the Transputer because the compiler */
/* puts the constant into the literal pool, adding 2 extra memory refs	 */
/* which loses any advantage gained by avoiding the byte loads (*SIGH*). */

#if 0 /* defined(__TRAN) */
#define NotSameCycle_(port,pte) (((port) ^ *(word *)(pte)) & 0x00FF0000)
#else
#define NotSameCycle_(port,pte) (PortCycle_(port) != pte->Cycle)
#endif

/* Transmitter/Receiver Id's	*/

typedef struct Id {
	word		rc;		/* result value			*/
	struct Id	*next;		/* pointer to next in list	*/
	struct Id	*tail;		/* last in list			*/
	SaveState	*state;		/* saved process state		*/
	word		endtime;	/* termination timeout		*/
	MCB		*mcb;		/* MCB if present		*/
} Id;


/* state values for id.rc 	*/

#define	MultiUnused	(EC_Fatal+SS_Kernel+EG_Exception+0xffff)
#define	MultiWaiting	(EC_Fatal+SS_Kernel+EG_Exception+0xfffe)


/* Port Table Entries */

typedef struct PTE {
	byte		Type;		/* entry type			*/
	byte		Link;		/* link id, for surrogates	*/
	byte		Cycle;		/* cycle value			*/
	byte		Age;		/* GCTicks since last use	*/
	word		Owner;		/* owning task			*/
	Id		*TxId;		/* current transmitter		*/
	Id		*RxId;		/* current receiver		*/	
} PTE;

#define PTBaseSize		32	/* port base table size		*/
#define PTBlockSize		1024	/* port table block size	*/

#define PortTimeout		1000000	/* in microseconds = 1 seconds	*/

/* Defaults for port garbage collector	*/
#define GCTicks			60	/* garbage collect every minute */
#define GCAge			255	/* # of GCTicks before collect	*/

#ifdef __TRAN
/* Transputer go-faster version */
# define GetPTE(p,r) (&(r->PortTable[((char *)&p)[1]][((char *)&p)[0]]))
/*# define GetPTE(p,r) (&(r->PortTable[((char *)&p)[1]][((int)p)&0xff]))*/
#else
/* Portable version */
# define GetPTE(p,r) (&(r->PortTable[((unsigned)p>>8)&0xff][(int)p&0xff]))
#endif

/* Link Protocol */

#ifdef __C40
typedef word	ProtoPrefix;
#else
typedef byte	ProtoPrefix;
#endif

#define Proto_Write		0	/* debug write command		*/
#define Proto_Read		1	/* debug read command		*/
#define	Proto_Msg		2	/* indicates message coming	*/
#define Proto_Null		3	/* initial value of type	*/
#define Proto_Term		4	/* terminate from neigbour	*/
#define Proto_Reconfigure	5	/* switch to dumb link		*/
#define Proto_SecurityCheck	6	/* check on security		*/
#define Proto_Reset		7	/* soft reset			*/
#define Proto_Xoff		8	/* stop message flow		*/
#define Proto_Xon		9	/* restart message flow		*/
#if defined(__C40) || defined(__ARM)
# define Proto_Go		0xa	/* Can send msg on half duplex link */
#endif
#define Proto_Info		0xf0	/* sync/info  message		*/
#define Proto_ReSync		0x7f	/* resync byte (throw away)	*/
#define Proto_Alive		0x9e	/* result of probing live link	*/
#ifdef __C40
#define Proto_AliveFull		0x9e9e9e9e /* word ret from probing live link */
#endif
#define Proto_Dead		0x61	/* result of probing dead link	*/

#define Proto_Sync	0xf0f0f0f0	/* sync word			*/

#define Probe_Value	0x61616161	/* value for probes		*/
#define Probe_Address	(MinInt+0x70)	/* magic probe address		*/

#define Proto_Reset0	0xff2f2107	/* first word of reset msg	*/
#define Proto_Reset1    0xff2f2103	/* 2nd word repeats		*/

typedef struct InfoMsg {
	word		Sync;		/* Proto_Sync			*/
	byte		TxInc;		/* transmitters incarnation	*/
	byte		RxInc;		/* tx's idea of rx's incarnation*/
	byte		Reply;		/* nonzero of reply needed	*/
	byte		Spare;		/* spare byte			*/
	Port		IOCPort;	/* Tx'x IOC port		*/
} InfoMsg;

#define IOProcIOCPort	1

#define LinkTxTimeout	(15*OneSec)	/* link tfr timeout		*/
#define LinkRxTimeout	(15*OneSec)	/* link tfr timeout		*/

#define DeliveryTime	(30*60*OneSec)	/* time msg is kept pending delivery */
					/* this could be smaller	*/

#ifdef __TRAN
# define LinkVector	((Channel *)MinInt)
#endif
#ifdef __C40
# define LinkVector	(0x00100040)	/* comms port 0 CPCR memory map */
#endif

/* work buffers */

typedef struct Buffer {
	Node		Node;		/* for free queue		*/
	word		Type;		/* see below			*/
#ifdef __TRAN
	byte		Buf[1012];	/* so whole buffer is 1024 bytes*/
#else
	byte		Buf[2048 - 12];	/* so whole buffer is 2048 bytes*/
#endif
} Buffer;

#define Buffer_Type_Cache	1	/* permanent block from BufPool	*/
#define Buffer_Type_Special	2	/* buffer block to be freed	*/

/* message buffers */

#ifdef __TRAN
#define MsgBufMax	984		/* so it maps onto a Buffer	*/
#else
#define MsgBufMax	(2048 - 12 - sizeof(MCB))
#endif

typedef struct MsgBuf {
	Node		Node;		/* for free queue		*/
	word		Type;		/* see above			*/
	MCB		MCB;		/* control block		*/
	word		Msg[MsgBufMax/4]; /* control & data		*/
} MsgBuf;

#define MsgBuf_Overhead sizeof(MCB)

/* worker processes */

#define Worker_stacksize	sizeof(((Buffer *)0)->Buf)

/* Kernel error codes */

#ifdef CODES
#define Err_Timeout		(root->ErrorCodes[0])
#define Err_BadPort		(root->ErrorCodes[1])
#define Err_BadRoute		(root->ErrorCodes[2])
#define Err_DeadLink		(root->ErrorCodes[3])
#define Err_NoMemory		(root->ErrorCodes[4])
#define Err_Congestion		(root->ErrorCodes[5])
#define Err_Kill		(root->ErrorCodes[6])
#define Err_Abort		(root->ErrorCodes[7])
#define Err_NotReady		(root->ErrorCodes[8])
#define Err_BadLink		(root->ErrorCodes[9])
#else
#define Err_Timeout		(EC_Recover+SS_Kernel+EG_Timeout+EO_Message)
#define Err_BadPort		(EC_Warn+SS_Kernel+EG_Invalid+EO_Port)
#define Err_BadRoute		(EC_Warn+SS_Kernel+EG_Invalid+EO_Route)
#define Err_DeadLink		(EC_Error+SS_Kernel+EG_Invalid+EO_Link)
#define Err_NoMemory		(EC_Warn+SS_Kernel+EG_NoMemory)
#define Err_Congestion		(EC_Recover+SS_Kernel+EG_Congested+EO_Route)
#define Err_Kill		(EC_Fatal+SS_Kernel+EG_Exception+EE_Kill)
#define Err_Abort		(EC_Error+SS_Kernel+EG_Exception+EE_Abort)
#define Err_NotReady		(EC_Recover+SS_Kernel+EG_Congested+EO_Port)
#define Err_BadLink		(EC_Warn+SS_Kernel+EG_Invalid+EO_Link)
#endif

/* Trace vector */

#define TVSize		(4*1024)	/* 4k trace vector (must be 2^x)*/
#define	TVMask		(TVSize-4)	/* mask for offset bits		*/
#define TVEnable	1		/* trace enable bit		*/

/* Prototypes for kernel internal functions */

void MemInit(Config *config, word confsize); /* __ARM alteration */
void MemInit2(void);
void *Allocate(word size, Pool *source, Pool *dest);
void *LowAllocate(word size, Pool *source, Pool *dest);
void _Free(Memory *block, Pool *pool);

void LinkInit(Config *config, Task *procman);
void FlowControl(word code);
void EventInit(Config *config);

bool NewWorker(VoidFnPtr fn, ... );
Buffer *GetBuf(word size);
void FreeBuf(Buffer *buf);

bool _TimedWait(Semaphore *sem, word timeout);
word _Wait(Semaphore *sem);
void _Signal(Semaphore *sem);
void AbortSem(Semaphore *sem);

void PortInit(Config *config);
Code _FreePort(Port port);
Port _NewPort(void);
Code __FreePort(Port port, bool kernel, Code rc);
void ClearPorts(LinkInfo *link);
Code _AbortPort(Port port, Code rc);

Code _PutMsg(MCB *mcb);
Code _GetMsg(MCB *mcb);

void _WaitLink(LinkInfo *link, Id *id);
void _SignalLink(LinkInfo *link);

bool inpool(void *mem, Pool *pool);
void dq(Id **lvq, Id *prev, Id *id);

#ifdef __TRAN
void _Trace(...);
void _Mark(...);
#else
void _Trace(word a, word b, word c);
void _Mark(void);
#endif
void Exception(Code code, Port port);

extern Task *_Task_;

#ifdef __C40
extern void	back_trace( void );
#endif

	/* This deals with special links. The constants should be kept	*/
	/* in step with protocol.h in the I/O Server.			*/
#ifdef __C40
#define Link_State_DSP1		0x10
#define Link_State_Hydra	0x11

#define Link_State_Mask		0xe0

	/* Any link mode with the most significant 3 bits set are	*/
	/* shared memory links. The remaining 3 bits are used to	*/
	/* configure the link.						*/
#define Link_State_SML		0xe0
#define	Link_State_SML_size	0x03	/* size of shared memory	*/
					/* 0=8k, 1=16k, 2=32k,3=64k	*/
#define Link_State_SML_strobe	0x04	/* Strobe on Global Bus containing*/
					/* shared memory. 0=g0,1=g1	*/
#define Link_State_SML_type	0x18	/* Board type (defines interrupts) */
					/* 0=VC40 (hydra I), 1=SPIRIT40,   */
					/* 2=VC40 (hydra II), 3=reserved   */

#endif

/* Initialise an add-on comms link. */
extern	void	LinkInitSpecial(LinkInfo *);

#endif

/* -- End of kernel.h */
