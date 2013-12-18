/*------------------------------------------------------------------------
--                                                                      --
--                      H E L I O S   K E R N E L                       --
--                      -------------------------                       --
--                                                                      --
--             Copyright (C) 1988 - 1993, Perihelion Software Ltd.      --
--                        All Rights Reserved.                          --
--                                                                      --
-- sml.c								--
--                                                                      --
--	Shared memory link stuff. This file is included	into		--
--	link1.c if SML is required. This allows it to integrate		--
--	properly with the rest of the system.				--
--                                                                      --
--	Author:  NHG 18/1/93						--
--                                                                      --
------------------------------------------------------------------------*/
/* $Id: sml.c,v 1.10 1994/06/29 14:57:07 tony Exp $ */

/*{{{  Includes */

#include "kernel.h"
#include <config.h>
#include <task.h>
#include <process.h>
#include <sem.h>

/*}}}*/
/*{{{  Configuration */

#ifdef __C40
#include <c40.h>
#endif

#define	POLL_LOOPS	1000

#define	USE_ABORT	1

#define	USE_INTERRUPT	1

#define RX_IN_INTERRUPT	0

#define TX_IN_INTERRUPT	0

#ifdef __C40

#define SML_INTDISABLE(iv) ((0xFL)<<(((iv)-1))*4)	/* Interrupt disable mask */
#define SML_INTFUNC(iv)	   ((0x1L)<<(((iv)-1))*4)	/* Interrupt function bit */
#define SML_INTTYPE(iv)	   ((0x2L)<<(((iv)-1))*4)	/* Interrupt type bit	  */
#define	SML_INTFLAG(iv)	   ((0x4L)<<(((iv)-1))*4)	/* Interrupt pending flag */
#define SML_INTENABLE(iv)  ((0x8L)<<(((iv)-1))*4)	/* Interrupt enable mask  */

#define SML_NMIVEC	INTR_NMI	/* NMI interrupt */

#define S40_INTVEC	INTR_IIOF1	/* Incoming interrupt		*/

#define HYDRAI_INTVEC	INTR_IIOF2	/* Incoming interrupt		*/

#define HYDRAII_INTVEC	INTR_IIOF2	/* Incoming interrupt		*/

#endif /* __C40 */

#define SML_WORKSPACE (2*sizeof(SMLChannel)+sizeof(Event))

/*}}}*/
/*{{{  Types */

#define	MPSize	4

typedef WORD	Atomic;


typedef	MPtr	SMPtr;

#define SMWord_(m,o)		MWord_(m,o)
#define SMData_(d,m,o,s)	MData_(d,m,o,s)
#define SMInc_(m,o)		MInc_(m,o)
#define SetSMWord_(m,o,v)	SetMWord_(m,o,v)
#define SetSMData_(m,o,d,s)	SetMData_(m,o,d,s)

#define	SMSize			sizeof(MPtr)


#define Halt(a,b,c) JTAGHalt(0,a,b,c)

extern void SetIIFBits(word bits);
extern void ClrIIFBits(word bits);
extern Code _SetEvent( Event * );

static WORD SMLInterruptNMI(LinkInfo *link, WORD vector);

void writeVIC( unsigned long add, unsigned long data );
void IACK( unsigned long ipl );

void ResetDSP(WORD i);

/*}}}*/
/*{{{  Debugging */

#if 1

#define TBUF		0x80050000
#define TBUFSIZE	0x00001000

static void TInit()
{
	SetMWord_(TBUF,0,1);
	SetMWord_(TBUF,4,0xdeadbeef);
}

void T(WORD x)
{
	MPtr o = MWord_(TBUF,0);
	
	SetMWord_(TBUF,o*4,x);

	o++;

	if( o == TBUFSIZE ) o = 1;

	SetMWord_(TBUF,o*4,0xdeadbeef);
	
	SetMWord_(TBUF,0,o);
}

#else

#define TInit()

#define T(x)

#endif

/*}}}*/

#include "sml.h"

/*{{{  InitSMLLink */

#define HYDRAI_BOARD	1
#define HYDRAII_BOARD	2
#define SPIRIT40_BOARD	4

void InitSMLLink(LinkInfo *link)
{
	RootStruct *root = GetRoot();
	IDROM *idrom = &GetExecRoot()->ID_ROM;
	SMLChannel *sc;
	Event *event;
	word lstat = link->State;
	word wssize = SML_WORKSPACE;
	word usenmi = FALSE;
	word intvec = 0;
	word *intinfo = NULL;
	MPtr srambase;
	word sramsize;

	int	board_type = -1;

	/* This kernel assumes that it is loaded onto an Ariel Hydra II */

	switch(lstat & Link_State_SML_type)
	{
	case 0x10:	/* Ariel's VC40 Hydra II board */
	{
		/* hard coded values for now */
/*
		word	ndsp = 4;
		word	ip = 3;
		word	iv = 0xd0;

		ndsp = MWord_(0xc0000001, 0);
		ip   = MWord_(0xc0000002, 0);
		iv   = MWord_(0xc0000003, 0);
*/

		intvec = HYDRAII_INTVEC;
		/* intinfo is -ve to distinguish it between Hydra I interrupts */
		intinfo = (word *)(-1);

		board_type = HYDRAII_BOARD;

		/* set up outgoing interrupts */
		ClrIIFBits(SML_INTDISABLE(INTR_IIOF3));
		ClrIIFBits(SML_INTDISABLE(INTR_IIOF3));
/*
		SetIIFBits(SML_INTFUNC(INTR_IIOF3));
		SetIIFBits(SML_INTENABLE(INTR_IIOF3));
		ClrIIFBits(SML_INTFLAG(INTR_IIOF3));
*/
		SetIIFBits(SML_INTTYPE(INTR_IIOF3));

		break;
	}
	case 0:		/* Ariel's VC40 Hydra I board */
	{
		int 	i;
		word 	ndsp = MWord_(0x8d000001,0);
		word	ip   = MWord_(0x8d000002,0);
		word	iv   = MWord_(0x8d000003,0);

		/* Set VME interrupt vector for selected priority */

		/* writeVIC( 0x87 + (ip-1) * 4, iv ); --- nickg version */
		writeVIC ((0x20 + ip) * 4, iv);		/* tony version */

		intvec = HYDRAI_INTVEC;
		intinfo = (word *)ip;

		/* reset other DSPs */

		for( i = 2; i <= ndsp; i++ ) ResetDSP(i);

		board_type = HYDRAI_BOARD;

		break;
	}
	case 0x08:
		intvec = S40_INTVEC;

		board_type = SPIRIT40_BOARD;

		break;

	default:
		return;
	}

#ifdef __C40

	if (board_type == HYDRAII_BOARD)
	{
		/* HYDRA II board */
		/* The communications RAM always extends from 0x80000000 */
		srambase = 0x80000000;
	}
	else
	{
		/* HYDRA I board or SPIRIT40 */

		/* The communications RAM is beyond the IDROM defined end of	*/
		/* either global strobe 0 or 1.					*/
		if( lstat & Link_State_SML_strobe )
		{
			srambase = SMInc_(idrom->GBASE1,idrom->GSIZE1*sizeof(MPtr));
		}
		else
		{
			srambase = SMInc_(idrom->GBASE0,idrom->GSIZE0*sizeof(MPtr));
		}
	}
#endif	/* __C40 */

	/* Set sramsize to the size of the buffer for each direction	*/
	sramsize = 0x1000L << (lstat & (word) Link_State_SML_size);

#ifdef __C40	
	if( usenmi ) wssize += sizeof(Event);
#endif

	sc = Allocate(wssize,root->FreePool,&root->SysPool);

	/* Set up Rx channel					*/
	link->RxFunction	= SMLRx;
	link->RxChan		= (Channel)sc;
	InitSMLChan(sc,srambase,sramsize);
	sc->IntInfo 		= intinfo;
	sc->IntVec		= intvec;

	/* And Tx channel					*/
	link->TxFunction	= SMLTx;
	link->TxChan		= (Channel)(sc+1);	
	InitSMLChan(sc+1,SMInc_(srambase,sramsize),sramsize);
	sc->IntInfo 		= intinfo;
	sc->IntVec		= intvec;
	
	/* Attach Interrupt					*/

	event = (Event *)(sc+2);

	event->Vector	= intvec;
	event->Pri	= 0;
	event->Code	= (WordFnPtr)SMLInterrupt;
	event->Data	= link;
	
	_SetEvent(event);

#ifdef __C40

	if( usenmi )
	{

		Event *nmievent = event+1;

		nmievent->Vector= SML_NMIVEC;
		nmievent->Pri	= 0;
		nmievent->Code	= (WordFnPtr)SMLInterruptNMI;
		nmievent->Data	= link;

		_SetEvent(nmievent);

	}

	ClrIIFBits(SML_INTDISABLE(intvec));	/* clear all bits	*/
		
	ClrIIFBits(SML_INTDISABLE(intvec));	/* clear all bits	*/

	SetIIFBits(SML_INTFUNC(intvec));	/* Set to Int, edge trigger */

	SetIIFBits(SML_INTENABLE(intvec));	/* Enable the interrupt	*/

	ClrIIFBits(SML_INTFLAG(intvec));	/* clear any pending int */

	if (board_type == HYDRAI_BOARD)
	{
		/* writeVIC( 0x5F, 0 );	 clear ICMS switches (nickg version) */
		writeVIC (0x17 * 4, 0);/*  clear ICMS switches (tony version) */

		IACK( 0x20 );		/* ensure VIC interrupts acked */
		IACK( 0x40 );
	}
	else if (board_type == HYDRAII_BOARD)
	{	/* If a Hydra II board, tell IO Server to start up interrupts */

		SMLChannel *	hydraii_sc = (SMLChannel *)(link -> TxChan);
		CBPtr		hydraii_cb = hydraii_sc -> Cb;

		SetCBWord_(hydraii_cb, Protocol, 0x00000010l);

		while (CBWord_(hydraii_cb, Protocol) == 0x00000010l)
		{
			;
		}
	}


#else

#error	"SML not supported for anything other than a C40"

#endif		/* __C40 */
}

/*}}}*/
/*{{{  Await */

static void Await(SMLChannel *s, MPtr atom)
{
#if POLL_LOOPS > 0
	WORD	poll;

	s->Reason = SML_Wakeup;

	for( poll = POLL_LOOPS ; poll > 0 ; poll-- )
	{
		if( SMWord_(atom,0) != 0 ) return;
	}
#else
	s->Reason = SML_Wakeup;
	if( SMWord_(atom,0) != 0 ) return;

#endif
	IntsOff();

	if( SMWord_(atom,0) == 0 )
	  {
	    Suspend(&s->Waiter,THREAD_LINKRX);
	  }

	IntsOn();
}
/*}}}*/
/*{{{  SendInterrupt */

void SendInterrupt(WORD *intinfo)
{
	/* A NULL intinfo means "Do not interrupt"	*/

	if( intinfo == NULL )
	{
		return;
	}

	/* A value between 1 and 7 implies a VIC interrupt at	*/
	/* that level.						*/
	if(1 <= (word)intinfo && (word)intinfo <= 7 )
	{
		/* VC40 interrupt to host */

		/* writeVIC( 0x83, 1L | (1L << (word)intinfo) ); (nickg version) */

		writeVIC (0x20 * 4, 1L | (1L << (word)intinfo) ); /* (tony version) */
	}
	/* A value less than 0 implies a Hydra II interrupt generated */
	/* by twiddling th IIOF3 pin.				      */
	else if ((word)intinfo < 0)
	{	/* HYDRA II Board twiddles the IIOF3 pin to send interrupt */
		SetIIFBits(SML_INTFLAG(INTR_IIOF3));
		ClrIIFBits(SML_INTFLAG(INTR_IIOF3));
/*
		SetIIFBits(SML_INTFLAG(INTR_IIOF3));
		ClrIIFBits(SML_INTFLAG(INTR_IIOF3));
*/
	}
}

/*}}}*/
/*{{{  AcknowledgeInterrupt */

void AcknowledgeInterrupt(LinkInfo *link, WORD vector)
{
	SMLChannel *sc = (SMLChannel *)(link->RxChan);	

	if( (word)(sc->IntInfo) > 0 )
	{
		/* writeVIC( 0x5F, 0 );	 clear ICMS switches (nickg version) */
		writeVIC (0x17 * 4, 0);/*  clear ICMS switches (tony version) */

		IACK( 0x20 );
	}
}
/*}}}*/
/*{{{  SMLInterruptNMI */
#ifdef __C40

static WORD SMLInterruptNMI(LinkInfo *link, WORD vector)
{
	word intvec = ((SMLChannel *)(link->RxChan))->IntVec;

	SetIIFBits(SML_INTFLAG(intvec));	

	return 1;
}

#endif
/*}}}*/
/*{{{  writeVIC */
#ifdef __C40

void writeVIC( unsigned long add, unsigned long data )
{
	MPtr m = (MPtr)0xBFFF0000;

	SetMWord_(m,add,data);  /* Just testing without VIC */
}

#endif
/*}}}*/
/*{{{  IACK */
#ifdef __C40

int WriteTCR( unsigned long tcr );

void IACK( unsigned long ipl )
{
	register unsigned long tcr;
	register MPtr m = (MPtr)0xbfffffc1;
	register WORD dummy;

	tcr = WriteTCR( ipl );                  /* Set LA1 high */

	dummy = MWord_(m,0);

	(void)WriteTCR( tcr );                  /* restore old TCR */
}

#endif
/*}}}*/
/*{{{  ResetDSP */
#ifdef __C40

void ResetDSP(WORD i)
{
	MPtr mcr = (MPtr)0xBF7FC008;
	word resetbit;
	word bootbits;
	word oldmcr = MWord_(mcr,0);

	i -= 2;				/* cvt to MCR bit offset	*/
		
	resetbit = 0x10000000L << i;
	bootbits = 0x00000E01L << (i*3);

	SetMWord_(mcr, 0, oldmcr | bootbits );

	SetMWord_(mcr, 0, (oldmcr | bootbits) & ~resetbit );

	SetMWord_(mcr, 0, oldmcr | bootbits );
}

#endif
/*}}}*/

#include "smlgen.c"
