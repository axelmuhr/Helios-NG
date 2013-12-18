/*------------------------------------------------------------------------
--                                                                      --
--                   H E L I O S   I / O   S E R V E R                  --
--                   ---------------------------------                  --
--                                                                      --
--             Copyright (C) 1993, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
--      sml.h								--
--                                                                      --
--	Share Memory Link header					--
--                                                                      --
--  Author:  NHG 19/1/93                                                --
--                                                                      --
------------------------------------------------------------------------*/
/* RcsId: $Id: sml.h,v 1.3 1994/06/29 13:42:25 tony Exp $ */
/* Copyright (C) 1993, Perihelion Software Ltd.    			*/

/*
	This file defines the data structures and functions in the
	shared memory link interface. This file and <smlgen.c> should
	be included into a source file which calls the interface and
	defines some types and procedures to complete the interface.

	Before including this file the follwing types and macros should
	be defined:

	type Atomic	32 bit type which may be read/written atomically.

	type MPtr	a Machine pointer, all references to local
			memory use these.

	macro MPSize	size of unit of MPtr addressing in bytes
	
	macros MWord_() SetMWord_() MData_() SetMData_() MInc_() MtoC_()
			Macros to access data through MPtrs.

	type SMPtr	a Shared Memory pointer. All accesses to the
			shared memory go through these.

	macro SMPSize	size of unit of SMPtr addressing in bytes.
			
	macros SMWord_() SetSMWord_() SMData_() SetSMData_() SMInc_()
			Macros to access data through SMPtrs.
			
	type SaveState	A pointer to a saved thread state. This may be a
			dummy state in non-threaded systems.

	The following configuration macros are also defined:

	USE_ABORT	Include code to allow threads waiting for a link
			to be aborted. Only useful in threaded systems.

	USE_INTERRUPT	Include the SMLInterrupt routine which should
			be attached to an inter-processor interrupt.
			IF this is defined then the macros TX_IN_INTERRUPT
			and RX_IN_INTERRUPT may also be defined. The enable
			the continuation of a transfer in the interrupt
			routine, waking the thread only when the entire
			transfer is done, rather than waking the thread
			each time.

	
*/

/*--------------------------------------------------------
-- Types and Data Structures				--
--------------------------------------------------------*/

typedef struct ChannelBuffer
{
	Atomic		DataReady;
	Atomic		Ack;
	word		Size;
	word		Protocol;
	BYTE		Buffer[Variable];
} ChannelBuffer;

typedef SMPtr		CBPtr;

#define offsetof(type, member) ((char *)&(((type *)0)->member) - (char *)0)

#define CBWord_(cb,field) SMWord_(cb,offsetof(ChannelBuffer,field))
#define CBData_(dst,cb,size) SMData_(MtoC_(dst),cb,offsetof(ChannelBuffer,Buffer),size)
#define CBDataReady_(cb) SMInc_(cb,offsetof(ChannelBuffer,DataReady))
#define CBAck_(cb) SMInc_(cb,offsetof(ChannelBuffer,Ack))
#define SetCBWord_(cb,field,val) SetSMWord_(cb,offsetof(ChannelBuffer,field),val)
#define SetCBData_(cb,src,size) SetSMData_(cb,offsetof(ChannelBuffer,Buffer),MtoC_(src),size)

#define CBOverhead offsetof(ChannelBuffer,Buffer[0])

typedef struct SMLChannel
{
	CBPtr		Cb;
	word		*IntInfo;
	word		IntVec;
	word		BufferMax;
	SaveState	*Waiter;
	word		Reason;
	word		Size;
	MPtr		Buf;
	word		Left;
	word		Timeout;
} SMLChannel;

#define	SML_Wakeup	0
#define SML_Aborted	1
#define SML_Done	2

#ifndef in_kernel

typedef struct LinkInfo
{
	word		Unit;
	SMLChannel	*TxChan;
	SMLChannel	*RxChan;
	SMLChannel	Channel[2];
} LinkInfo;

typedef struct LinkTransferInfo
{
	word		size;		/* size in WORDS		*/
	MPtr		buf;
	LinkInfo	*link;
} LinkTransferInfo;

#endif

#ifdef __STDC__
static word InitSMLChan(SMLChannel *sc, CBPtr cb, word cbsize);
static word SMLTx(LinkTransferInfo *info);
static word SMLRx(LinkTransferInfo *info);
static word SMLTxRdy(LinkInfo *link);
static word SMLRxRdy(LinkInfo *link);
static word SMLInterrupt(LinkInfo *link, word vector);
static void Await(SMLChannel *s, MPtr atom);
static void SendInterrupt(word *intinfo);
#else
static word InitSMLChan();
static word SMLTx();
static word SMLRx();
static word SMLTxRdy();
static word SMLRxRdy();
static word SMLInterrupt();
static void Await();
static void SendInterrupt();
#endif


