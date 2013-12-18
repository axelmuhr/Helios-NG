/*
 * File:	wdether.c
 * Subsystem:	VLSI PID Western Digital ethernet device driver.
 * Author:	P.A.Beskeen
 * Date:	April '93
 *
 * Description: Provides an ethernet device driver for the Helios TCP/IP
 *		server. It supports SMC 8/16 bit compatible ethernet cards
 *		plugged into the VLSI PID's option bus (PC ISA compatible bus).
 *		This includes cards such as the SMC EtherCard Plus Elite16
 *		Combo Card, in fact most cards that are implemented using the
 *		western digital wd83c584/wd83c690 chip set.
 *
 *		The mapping of PC addresses, ports and interrupts on the VLSI
 *		PID are as follows:
 *
 *			PC addresses are mapped at 0x2c0 0000 16 bit reads are
 *			achieved by issuing a 32 bit word read and throwing
 *			away the 16 MSB's.
 *
 *			PC port addresses are mapped at 0x280 0000. The PC port
 *			number should be multiplied by four and added to this
 *			address.
 *
 *			PC interrupts are mapped to the ARM IRQ interrupt
 *			through the PID's INTC interface chip.
 *
 *		Note that whenever you see a '* 4' multiply by four, it is
 *		because of this odd address mapping.
 *
 *
 * RcsId: $Id: wdether.c,v 1.4 1994/06/07 12:50:37 nickc Exp $
 *
 * (C) Copyright 1993 Perihelion Software Ltd.
 * 
 * RcsLog: $Log: wdether.c,v $
 * Revision 1.4  1994/06/07  12:50:37  nickc
 * hmm....change reason unknown
 *
 * Revision 1.3  1994/03/17  13:01:16  nickc
 * updated to work with new RT kernel
 *
 * Revision 1.2  1994/03/17  12:48:26  paul
 * lots of unknown changes (checkin forced by NC)
 *
 * Revision 1.1  1993/04/27  16:25:22  paul
 * Initial revision
 *
 */
 
/* Generic device and Helios manifests */

#include <device.h>
#include <syslib.h>
#include <nonansi.h>
#include <queue.h>
#include <sem.h>
#include <codes.h>
#include <string.h>

#include <arm.h>		/* Interrupt handler bits */
#include <event.h>
#include <root.h>

/* Hardware specific manifests */

#include <ARM/vy86pid.h>	/* VLSI PID manifests */
#include "wd83c584.h"		/* Bus interface */
#include "wd83c690.h"		/* Ethernet LAN controller */


#if 0
#define DEBUG 			/* Generate debugging messages */
#endif

#if 1
#define LAN16			/* 16 bit memory between LAN and shared mem */
#endif
#define MAXETHERPKT 	1514	/* Maximum ethernet packet size, sent or */
				/* received by TCP/IP server. */

#define BUFFSIZE	256	/* Size of individual buffers in buffer ring */

/* Yuk! Fixed address configuration for the ethernet card. - This could and */
/* should be changed to a more flexible approach. */
#define PCPORTADDR	0x280		/* PC ISA bus port number. */
#define PCMEMADDR	0xd0000		/* PC ISA bus shared mem address. */
#define PCIRQNUM	INTR_IRQ_3	/* PC ISA bus interrupt level */
					/* PID intr mask ISA bus intr. bit */
#define PCIRQBIT	hw_intc_enableirq_expslot3
#define PCIRQSELECT	IRR_IR0		/* wd83c584 interrupt level select */

/*
 * Extend the DCB structure to contain device specific data. Since
 * devices cannot have static data this is the ONLY place that persistent
 * data may be stored.
 */
typedef struct {
	/* Common part of DCB structure: */
	DCB		dcb;

	/* Device's Private entries: */
	Semaphore	lock;		/* Concurrent access lock */
	List		readq;		/* List of pending read requests */
	Semaphore	nreq;		/* Size of readq */
	Semaphore	CanReadPacket;	/* Packets available to read */
	Semaphore	CanWritePacket;	/* Packets can be written */
#ifdef DEBUG
	Semaphore	PanicHit;	/* Debug semaphore */
	Event		DebugHandle;	/* Debug event handler */
	word		IntrCount;	/* Count of number of interrupts */
	word		IntrLastStat;	/* Last interrupt status */
#endif
	/* Hardware specific entries: */
	Event		EventHandle;	/* Interrupt handler structure */
	wd83c584 *	bic;		/* Bus Interface Controller H/W	*/
	wd83c690 *	lic;		/* LAN Interface Controller H/W	*/
	ubyte		StationAddr[8];	/* Physical ethernet address */
	char *		EtherMem;	/* Start of Shared memory on board */
	char *		BufferRingStart;/* Start of buffer ring */
	char *		BufferRingEnd;	/* End of ring (first unused byte) */
	ubyte		StartIndex;	/* buffer ring start index */
	ubyte		StopIndex;	/* buffer ring stop index */
	ubyte		NextPktIndex;	/* Next valid packet starts here */
} NetDCB;


#ifdef DEBUG
word PanicHandler(Semaphore *, word);
void InitDebug(NetDCB *dcb);
void DebugPause(NetDCB *dcb);
#endif

/* Hardware specific function prototypes */
static bool InitEtherHW(NetDCB * dcb);
static void HaltEtherHW(NetDCB * dcb);
static word ReadPacket(NetDCB * dcb, char * buf);
static word WritePacket(NetDCB * dcb, char * buf, word size);
static word EtherIntrHandler(NetDCB *dcb, word vector);

/***************************************************************************/
/* Generic Device Driver Functions */

/*
 * DevOperate is called by the server each time a device operation is
 * requested.
 */
static void DevOperate(	NetDCB * dcb, NetDevReq * req )
{
	NetInfoReq *	ireq = (NetInfoReq *)req;
  
	switch (req->DevReq.Request) {

		case FG_Read:
		/* Queue read requests for Reader thread. The server	*/
		/* currently sends us several of these when it starts.	*/
		/* Note that we must lock the DCB against concurrent 	*/
		/* access.					 	*/

		Wait( &dcb->lock );

		AddTail( &dcb->readq, &req->DevReq.Node );
		Signal( &dcb->nreq );

		Signal( &dcb->lock );
#ifdef DEBUG
		IOdebug("edriver: FG_Read: added req to list");
#endif
		return;


		case FG_Write:
		/* Write packet onto the ethernet. We first have to	*/
		/* wait for permission to write to the chip. This is	*/
		/* signaled by the interrupt handler whenever a TxAvail */
		/* Interrupt occurs.					*/
#ifdef DEBUG
		{
		  int 		i;
		  char *	buf = (char *)req->Buf;
	  
		  IOdebug( "edriver: FG_Write: %d buf = %x [%", req->Size, buf );
		  for (i = 0;i < 24; i++)
		    IOdebug( "%x %", buf[ i ] );
		  IOdebug( "] Waiting for permission" );
		}
#endif
#ifdef NEW_SYSTEM
		Wait(&dcb->CanWritePacket);
#else
		HardenedWait(&dcb->CanWritePacket);
#endif
#ifdef DEBUG
		IOdebug( "edriver: FG_Write: Got Write permission" );
#endif
		req->Actual = WritePacket(dcb, (char *) req->Buf, req->Size);
#ifdef DEBUG
		IOdebug( "edriver: FG_Write: Sent Packet" );
#endif
		req->DevReq.Result = 0;
		break;


		case FG_SetInfo:
		/* Set options/address in ethernet device		*/
		/* The tcpip server will attempt to set the ethernet	*/
		/* address on startup.					*/

		/* SetInfo does nothing in this driver.			*/
		req->DevReq.Result = 0;
		break;

      
		case FG_GetInfo:
		/* Get options/address from /ether device. Only the	*/
		/* ethernet address is used by tcpip server.		*/
      		{
			NetInfo *  info = &ireq->NetInfo;
			info->Mask = NetInfo_Mask_Addr | \
					NetInfo_Mask_Mode | NetInfo_Mask_State;
			info->Mode = info->State = 0;
			memcpy(info->Addr, dcb->StationAddr, 8);
			req->DevReq.Result = 0;
      
#ifdef DEBUG
			{
			  int i;
	  
			  IOdebug( "GetInfo Ethernet address requested: Addr [%" );
			  for (i = 0; i < 6; i++ )
			    IOdebug( "%d %", info->Addr[ i ] );
			  IOdebug( "]" );
			}
#endif
			break;
		}
	}

	/* Return request to server by calling the Action routine in	*/
	/* the request. Note that the tcpip server will re-call 	*/
	/* DevOperate before returning from this routine when the	*/
	/* request was a Read.						*/
  
	(*req->DevReq.Action)( dcb, req );

	return;

} /* DevOperate */


/*
 * The Reader thread simply rendezvous packet requests with arriving packets,
 * reads the packet and returns it to the requestor.
 *
 * Packets are returned to the requestor by calling the Action routine,
 * this threads stack size must therefore be adequate for the tcpip server's
 * packet arrival processing.
 */
static void Reader( NetDCB * dcb )
{
	NetDevReq *	req;
  
	for (;;) {		
#ifdef DEBUG
		IOdebug( "edriver: Reader: Waiting for Read request");
#endif	  
		/* Wait for a request from server. */
		Wait( &dcb->nreq );

		/* Remove it from list. Note that we must lock the DCB	*/
		/* before doing this. */
		Wait( &dcb->lock );
		req = (NetDevReq *)RemHead( &dcb->readq );
		Signal( &dcb->lock );
      
		if (req == NULL)
			continue;

GetAnotherPacket:
#ifdef DEBUG
		IOdebug( "edriver: Reader: Waiting for Packet arrival");
#endif

#if 1
		/* Wait for a packet arrival signal from interrupt handler. */

#ifdef NEW_SYSTEM
		Wait(&dcb->CanReadPacket);
#else
		HardenedWait(&dcb->CanReadPacket);
#endif
		
#else	/* @@@ DELETE ME */
		while (!TimedWait(&dcb->CanReadPacket, 10 * OneSec)) {
			/* If the reason we have not recieved a packet */
			/* is because the ring overflow is set... */
			if (dcb->lic->p0r.INTSTAT & INTSTAT_OVW){
#ifdef DEBUG
				IOdebug("***Detected overflow and recovered (missed %x)...",
					dcb->lic->p0r.MPCNT);
#endif
				/* Reset overflow signal. */
				dcb->lic->p0w.INTSTAT = INTSTAT_OVW;
				/* Reset by re-writing boundary register... */
				dcb->lic->p0w.BOUND = dcb->lic->p0r.BOUND;
				/* and continue waiting for packets. */
			}
		}
#endif

#ifdef DEBUG
		IOdebug( "edriver: Reader: Reading Packet...");
		IOdebug( "edriver: Reader: status irqs %x, irqm %x"
			" INT STAT %x, IIR %x",
			hw_INTC->IRQS, GetRoot()->IRQM_softcopy,
			dcb->lic->p0r.INTSTAT, dcb->bic->IRR );

#endif	  

		if (dcb->lic->p0r.INTSTAT & INTSTAT_OVW) {
#ifdef DEBUG
			IOdebug("***Detected overflow and recovering (missed %x)...",
				dcb->lic->p0r.MPCNT);
#endif
			/* Reset overflow signal. */
			dcb->lic->p0w.INTSTAT = INTSTAT_OVW;
			/* We complete the recovery by re-writing the BOUND */
			/* register in ReadPacket(). */
		}

		/* Read the packet from the ethernet. */
		/* Note that the requested size is ignored, you get whatever */
		/* packet is available. */
		if((req->Actual = ReadPacket(dcb, (char *)req->Buf)) == 0) {
#ifdef DEBUG
			IOdebug("***Detected Packet Read error (0)");
#endif
			goto GetAnotherPacket;
		}

#ifdef DEBUG
		{
		  int 		i;
		  char *	buf = (char *)req->Buf;
	  
		  IOdebug( "edriver: Reader: Got Packet: %x [%", req->Actual);
		  for (i = 0;i < 24; i++)
		    IOdebug( "%x %", buf[ i ] );
		  IOdebug( "]" );
		}
#endif

		req->DevReq.Result = 0;
      
		/* Return request to server. */
		(*req->DevReq.Action)( dcb, req );	
	}

} /* Reader */


/*
 * DevClose - this is never called by the current tcpip server
 */
static word DevClose( NetDCB * dcb )
{

	Wait( &dcb->lock );

	/* Stop all ethernet activity */
	HaltEtherHW( dcb );

	Free( dcb );

#ifdef DEBUG
	IOdebug( "Ether: closed down ethernet device" );
#endif
  
	return 0;
}


/*
 * DevOpen - this is called to initialize the device. It must allocate
 * the DCB, inititiaize it and initialize the hardware.
 * The info parameter points to a structure which has been initialized from
 * a devinfo file netdevice entry.
 */
NetDCB * DevOpen( MPtr dev, NetDevInfo * info)
{
	NetDCB *	dcb;

	/* Allocate the DCB. */
	dcb = (NetDCB *) Malloc( sizeof (NetDCB) );
  
	if (dcb == NULL) {
#ifdef DEBUG
	      IOdebug( "Ether: Failed to allocate memory for DCB structure" );
#endif      
		return NULL;
	}
  
	/* Initialize the common DCB fields. */
  	dcb->dcb.Device  = dev;
	dcb->dcb.Operate = DevOperate;
	dcb->dcb.Close   = DevClose;
  
	/* Initialize our private fields. */
  	InitSemaphore( &dcb->lock, 1 );
  	InitList( &dcb->readq );
  	InitSemaphore( &dcb->nreq, 0 );		
	InitSemaphore( &dcb->CanReadPacket, 0 );		
	InitSemaphore( &dcb->CanWritePacket, 1 );		

#ifdef DEBUG
	InitDebug(dcb);
#endif
	/* Initialise ethernet hardware and hardware specific field of dcb. */
	if (!InitEtherHW(dcb))
		return NULL;

	/* Start the Reader thread. */
	if (Fork( 2000, Reader, 4, dcb ) == 0) {
		IOdebug( "Ether: failed to Fork Reader process" );
		return NULL;
	}

#ifdef DEBUG
	IOdebug( "Ether: successfully Opened ether driver" );
#endif

	return dcb;

	info=info; /* not used, this stops compiler moaning. */

} /* DevOpen */


/***************************************************************************/
/* Hardware Specific Functions */

/*
 * Initialise the ethernet hardware and hardware specific dcb entries.
 */
static bool InitEtherHW(NetDCB * dcb)
{
	int i, size;

	/* Initialise hardware specific dcb entries. */

	/* Get addresses of LAN H/W in PID boards I/O memory space. */
	/* Currently only supports one card address configuration. */
	/* Port 0x280, Mem 0xd0000, Int 3 */

	/* PC BUS interface address */
	dcb->bic = (wd83c584 *)(IO_base_expslot_IO + (PCPORTADDR * 4));

	/* LAN controller ports directly follow bus controller ports. */
	dcb->lic = (wd83c690 *)((int)dcb->bic + sizeof(wd83c584));

	/* Get address of shared memory between host and ethernet chipset. */
	dcb->EtherMem = (char *)(IO_base_expslot_MEM + (PCMEMADDR * 4));

	/* Read board's ethernet address and checksum it. */
	{
		ubyte checksum = 0;

#ifdef DEBUG
			IOdebug("MSR %x, IAR %x, BIO %x  IRR %x Station[0] = %x",
			&dcb->bic->MSR, &dcb->bic->IAR, &dcb->bic->BIO,
			&dcb->bic->IRR, &dcb->bic->LAR[0].valid);
#endif
		for (i = 0; i < 8; i++) {
			checksum += (dcb->StationAddr[i] =
					dcb->bic->LAR[i].valid);
#ifdef DEBUG
			IOdebug("Station[%x] = %x (cs %x)", i, 
				dcb->StationAddr[i], checksum);
#endif
		}

		/* If checksum != 0xFF then no board or board not Western D. */
		if (checksum != 0xff) {
			IOdebug("Cannot find ethernet card"
				" (checksum error 0x%x != 0xff)", checksum);
			return FALSE;
		}
#ifdef DEBUG
IOdebug("Checksum is OK - about to reset LAN controller");
#endif
	}

	/* Initialise ethernet controller hardware. */

	/* Reset the LAN controller. */
	dcb->bic->MSR = MSR_RST;
	dcb->bic->MSR = 0;

#ifdef DEBUG
IOdebug("Reset Lan Controller - about to enable shared mem");
#endif
	/* Enable the shared memory at fixed address. */
	dcb->bic->MSR = (ubyte)(MSR_MENB | ((PCMEMADDR >> 13) & 0x3f));

#ifdef DEBUG
IOdebug("Enabled shared mem");
#endif
#ifdef PCMEM16BIT
	/* Enable 16 bit LAN/PC working and fix A19 for DP8390 compatibility */
	dcb->bic->LAAR = LAAR_M16EN | LAAR_L16EN | LAAR_LA19;
#else
#ifdef LAN16
	/* Enable 16 bit LAN working and fix A19 for DP8390 compatibility */
	dcb->bic->LAAR = LAAR_L16EN | LAAR_LA19;
#endif
#endif
	/* 16 bit lan working doubles the shared memory avail. (8->16/32->64) */
	size = dcb->bic->ICR & ICR_MSZ; /* false = 8/16k / true = 32/64k */

	/* Set up the drivers private data. */

	/* Assign the first buffers in shared memory to the transmit buffer. */
	/* dcb->EtherMem points to the transmit buffer. */
	/* And the rest to the receive buffer ring. */
	/* dcb->StartIndex = start of receive buffer ring. */
	/* dcb->StopIndex = end (first unuseable buffer) in ring. */
	dcb->StartIndex = (ubyte)(MAXETHERPKT / BUFFSIZE) + 1;

#ifdef LAN16
	if (size)
		dcb->StopIndex  = (ubyte)256;	/* 256 * 256 byte buffs = 64k */
	else
		dcb->StopIndex  = (ubyte)64;	/* 64 * 256 byte buffs = 16k */
#else
	if (size)
		dcb->StopIndex  = (ubyte)128;	/* 128 * 256 byte buffs = 32k */
	else
		dcb->StopIndex  = (ubyte)32;	/* 32 * 256 byte buffs = 8k */
#endif

	dcb->BufferRingStart = dcb->EtherMem + dcb->StartIndex * (BUFFSIZE * 4);
	dcb->BufferRingEnd = dcb->EtherMem + dcb->StopIndex * (BUFFSIZE * 4);

#ifdef DEBUG
IOdebug("Sized mem ICR %x istwice %x, stopindex %x endmem %x", dcb->bic->ICR, size, dcb->StopIndex, dcb->BufferRingEnd);
#endif
#ifdef DEBUG
IOdebug("About to Set ring start/end and cleared ");
	{
		int *i = (int *)dcb->BufferRingStart;

		while (i < (int *)dcb->BufferRingEnd)
			*i++ = 0;
	}
#endif

	/* Setup Helios-ARM Interrupt handler. */
	dcb->EventHandle.Pri = 0;
	dcb->EventHandle.Vector = PCIRQNUM;
	dcb->EventHandle.Code = EtherIntrHandler;
	dcb->EventHandle.Data = dcb;
	SetEvent(&dcb->EventHandle);

	/* Enable PC ISA bus level 3 interrupts in the PIDs interrupt mask. */
	{	RootStruct *	xroot = GetRoot();
#ifdef DEBUG
IOdebug("About to  Enable ISA IRQs %x current, %x todo, IRQ status %x",
		xroot->IRQM_softcopy,
		xroot->IRQM_softcopy | PCIRQBIT,
		hw_INTC->IRQS);
#endif
		hw_INTC->IRQM = (int)(xroot->IRQM_softcopy |= PCIRQBIT);
	}

	/* Initialise wd83c690 chip */
	/* DP8390 compatibility requires that the initialisation is carried */
	/* out in the following sequence. */

#ifdef DEBUG
IOdebug("About to Stop COMMAND REG addr = %x = cmd %x",
	&(dcb->lic->p0w.COMMAND), COMMAND_STOP );
#endif
	/* Make sure the board is stopped - probably not needed as just rst. */
	/* Also forces selection of page 0 registers. */
	dcb->lic->p0w.COMMAND = COMMAND_STOP;

#ifdef DEBUG
IOdebug("About to set DMA burst size");
#endif

	/* Select internal DMA burst size and bus size. */
	/* DCON_BSIZE(0x2) = 8 byte burst mode DMA's to ether FIFO's */
#ifdef LAN16
	dcb->lic->p0w.DCON = DCON_BSIZE(0x2) | DCON_BUS16 | DCON_OLDNOLOOP;
#else
	dcb->lic->p0w.DCON = DCON_BSIZE(0x2) | DCON_OLDNOLOOP;
#endif

#ifdef DEBUG
IOdebug("About to RCON broadcasts allowed");
#endif
	/* Allow broadcast packets to be received, but not multicasts. */
	dcb->lic->p0w.RCON = RCON_BROAD;

#ifdef DEBUG
IOdebug("About to Enable Loopback internal");
#endif
	/* DP8390 compatibility requires that the next portion of the */
	/* initialisation is carried out with loopback mode set. */
	dcb->lic->p0w.TCON = TCON_LOOPBACK(TCON_LB_internal);

#ifdef DEBUG
IOdebug("About to Set BOUND %x, RSTOP %x, RSTART %x", dcb->StartIndex,
	dcb->StopIndex, dcb->StartIndex);
#endif
	/* Set boundary register to start of our receive packet buffer ring */
	dcb->lic->p0w.BOUND = dcb->StartIndex;

	/* Set start and end of our receive packet buffer ring in shared mem */
	dcb->lic->p0w.RSTART = dcb->StartIndex;
	dcb->lic->p0w.RSTOP = dcb->StopIndex;

#ifdef DEBUG
IOdebug("about to clear pending interrupts %x", dcb->lic->p0r.INTSTAT );
#endif
	/* Clear all possible pending interrupts. */
	dcb->lic->p0w.INTSTAT = INTSTAT_PTX | INTSTAT_TXE | INTSTAT_PRX;

#ifdef DEBUG
IOdebug("Cleared pending interrupts %x, about to set mask", dcb->lic->p0r.INTSTAT );
#endif
	/* Set interrupt mask to interrupt when packets are transmitted and */
	/* received, as well as transmit errors (so we know transmit buffer */
	/* is now free, even when an error occurs). */
	dcb->lic->p0w.INTMASK = INTMASK_PTXE | INTMASK_PRXE | INTMASK_TXEE;

#ifdef DEBUG
IOdebug("Set interrupt mask %x\n", INTMASK_PTXE | INTMASK_PRXE | INTMASK_TXEE);
#endif
	/* Enable IRQ level 3 interrupts from ether chip to bus. */
	{
#ifdef DEBUG
		ubyte iir = dcb->bic->IRR;
		IOdebug("IIR = %x, will %x", iir, iir | IRR_IEN | PCIRQSELECT);
		dcb->bic->IRR = iir | IRR_IEN | PCIRQSELECT;
#else
		dcb->bic->IRR |= IRR_IEN | PCIRQSELECT;
#endif

	}

#ifdef DEBUG
IOdebug("Set IRQ level IIR = %x - about to set COMMAND to page 1", dcb->bic->IRR);
#endif
	/* Select page one of register bank. */
	dcb->lic->p0w.COMMAND = COMMAND_PAGE(1);

#ifdef DEBUG
IOdebug("Set page 1, about to Set Phys addr");
#endif
	/* Set physical ethernet address */
	for (i = 0; i < 6; i++)
		dcb->lic->p1w.STA[i].valid = dcb->StationAddr[i];

#ifdef DEBUG
IOdebug("Set CURR %x",dcb->StartIndex + 1);
#endif
	/* Init current ring buffer pointer (received packets written here) */
	dcb->lic->p1w.CURR = dcb->StartIndex + 1;
  	dcb->NextPktIndex = dcb->StartIndex + 1;	/* init our own copy */

	/* Now point at last buffer for ReadPacket() use. */
	dcb->StopIndex--;

#ifdef DEBUG
IOdebug("Command start");
#endif
	/* Enable packet reception and transmission and register page 0. */
	dcb->lic->p0w.COMMAND = COMMAND_START;

#ifdef DEBUG
IOdebug("Out of loopback");
#endif
	/* Take out of loopback mode. */
	dcb->lic->p0w.TCON = 0;	

#ifdef DEBUG
IOdebug("End of Init: dcb %x, TXbuf %x, Rxbuf s %x e %x, bic, %x lic %x\n",
	dcb, dcb->EtherMem, dcb->BufferRingStart, dcb->BufferRingEnd,
	dcb->bic, dcb->lic);
IOdebug("End of Init: StartIndex %x, StopIndex %x, NextPktIndex %x",
		dcb->StartIndex, dcb->StopIndex, dcb->NextPktIndex);
#endif

	return TRUE;
}

/*
 * Stop all ethernet activity.
 */
static void HaltEtherHW(NetDCB * dcb)
{
	/* Disable PC ISA bus level 3 interrupts in the PIDs interrupt mask. */
	{	RootStruct *	xroot = GetRoot();
		hw_INTC->IRQM = (int) (xroot->IRQM_softcopy &= ~PCIRQBIT);
	}
	/* Reset ethernet controller. */
	dcb->bic->MSR = MSR_RST;
	dcb->bic->MSR = 0;

	/* Make sure the board is stopped - probably not needed as just rst. */
	dcb->lic->p0w.COMMAND = COMMAND_STOP;  
}

/*
 * Read a packet from the ethernet hardware. We have already ascertained
 * that one is available. Returns the packet's size.
 *
 * As we have already set up the RCON register to only accept good packets,
 * there should be no need to check the received packet headers PS_RSTAT field.
 */
static word ReadPacket(NetDCB * dcb, char * buf)
{
	PktStatus *	PS = (PktStatus *) (dcb->EtherMem +
					dcb->NextPktIndex * (BUFFSIZE * 4) );
	int		size = (int)(PS->PS_CountLo | (PS->PS_CountHi << 8));
	int		i;

#ifdef DEBUG
	IOdebug("edriver: ReadPacket: PS_RSTAT %x size %x, PS_CountHi %x, Lo %x Next %x",
		PS->PS_RSTAT, size, PS->PS_CountHi, PS->PS_CountLo, PS->PS_NextPktIndex);
	IOdebug("edriver: ReadPacket: PS_RSTAT %x IntrCount %x IntrStat %x",
		PS->PS_RSTAT, dcb->IntrCount, dcb->IntrLastStat);
#endif

	if ((PS->PS_RSTAT & 1) != 1) {
		/* This error should never occur! - We have asked the LAN */
		/* controller to silently throw away all erroneous packets. */
		IOdebug("Packet reception error! (%x)", PS->PS_RSTAT);
		return 0;
	}

	/* Get Packet from ring buffer */
#ifdef PCMEM16BIT
	{
	int *	RxBuff = (int *)(PS + 1);
	short *	UserBuff = (short *)buf;

#ifdef DEBUG
	IOdebug( "edriver: 16bit ReadPacket: PS %x, size %x, Rxbuf %x", PS, size, RxBuff);
#endif	  

	if (((char *)RxBuff) + size <= BufferRingEnd) {
		/* If contained within one contigous area copy entire */
		/* packet to the users buffer. */
		for (i = size / 2 ; i > 0 ; i--)
			*UserBuff++ = (short )((*RxBuff++) & 0xffff);
	} else {
		/* Else copy end of ring buffer and start of ring buffer */
		/* containing the packet to the users buffer. */
		int	size2 = (int)(dcb->BufferRingEnd - RxBuff) / 4;

		for (i = size2 / 2 ; i > 0 ; i--)
			*UserBuff++ = (short )((*RxBuff++) & 0xffff);

		RxBuff = (int *)BufferRingStart;

		for (i = (size - size2) / 2 ; i > 0 ; i--)
			*UserBuff++ = (short )((*RxBuff++) & 0xffff);
	}
	}
#else
	{
	char *	RxBuff = (char *)(PS + 1);

#ifdef DEBUG
	IOdebug( "edriver: 8bit ReadPacket: ubuf %x PS %x, size %x, ring buf %x PSNxtPkt %x", buf, PS, size, RxBuff, PS->PS_NextPktIndex);
#endif	  
	if (RxBuff + size <= dcb->BufferRingEnd) {
		/* If contained within one contigous area copy entire */
		/* packet to the users buffer. */
		for (i = size ; i > 0 ; i--) {
			*buf++ = *RxBuff;
			RxBuff += 4;
		}
	} else {
		/* Else copy end of ring buffer and start of ring buffer */
		/* containing the packet to the users buffer. */
		int	size2 = (int)(dcb->BufferRingEnd - RxBuff) / 4;

		for (i = size2 ; i > 0 ; i--) {
			*buf++ = *RxBuff;
			RxBuff += 4;
		}

		RxBuff = dcb->BufferRingStart;
		for (i = size - size2 ; i > 0 ; i--) {
			*buf++ = *RxBuff;
			RxBuff += 4;
		}
	}
	}
#endif

	/* Adjust pointer to next valid packet area in ring buffer */
	dcb->NextPktIndex = PS->PS_NextPktIndex;

	/* Safeguard overwrite by updating boundary pointer. */
	/* This pointer is kept one buffer behind CURR to remain */
	/* compatible with DP4390, also there is no way full and empty */
	/* conditions can be confused with this method. */
	if (dcb->NextPktIndex == dcb->StartIndex)
		dcb->lic->p0w.BOUND = dcb->StopIndex;
	else
		dcb->lic->p0w.BOUND = dcb->NextPktIndex - 1;

	/* Make sure that packet is not excepted again. */
	PS->PS_RSTAT = 0;

	return size;
}

/*
 * Write a packet to the ethernet hardware. We have already ascertained
 * that this is permitted now. Returns the packet's size.
 */
static word WritePacket(NetDCB * dcb, char *buf, word size)
{
	word i;

	/* Place packet into boards memory */
#ifdef PCMEM16BIT
	int *	TxBuff = (int *) dcb->EtherMem;
	short *	UserBuff = (short *)buf;

	for (i = size / 2 ; i > 0 ; i--)
		*TxBuff++ = (int) *UserBuff++;
#else
	char *	TxBuff = dcb->EtherMem;

	for (i = size ; i > 0 ; i--) {
		*TxBuff = *buf++;
		TxBuff += 4;
	}
#endif

	/* Set Page 0 as start of transmit buffer. */
	dcb->lic->p0w.TSTART = 0;

	/* Set size of packet. */
	dcb->lic->p0w.TCNTL = (ubyte) (size & 0xff);
	dcb->lic->p0w.TCNTH = (ubyte) ((size >> 8) & 0xff);

	/* Start transmission. */
	dcb->lic->p0w.COMMAND = COMMAND_TXP;

	/* An Interrupt will now occur either when the packet has been */
	/* transmitted or a transmission error occurs. */

	return size;
}


#ifdef DEBUG
/* Initialise debug system. */
/* Driver can do a {Hardened}Wait(dcb->PanicHit) to wait for panic to be hit. */

void InitDebug(NetDCB * dcb) {
	dcb->DebugHandle.Pri = 1;
	dcb->DebugHandle.Vector = INTR_IRQ_PANIC;
	dcb->DebugHandle.Code = PanicHandler;
	dcb->DebugHandle.Data = &dcb->PanicHit;
	dcb->IntrCount= dcb->IntrLastStat = 0;

	SetEvent(&dcb->DebugHandle);

/* Not required as PANIC button interrupot is always enabled. */
/*	hw_INTC->IRQM = rs->IRQM_softcopy |= hw_intc_enableirq_panic;*/

	InitSemaphore(&dcb->PanicHit, 0);
}

void DebugPause(NetDCB *dcb) {
	IOdebug("HitPanic...");
#ifdef NEW_SYSTEM
	Wait(&dcb->PanicHit);
#else
	HardenedWait(&dcb->PanicHit);
#endif
}
#endif /* DEBUG */

/*
 * This handler is called whenever the PC ISA bus IRQ3 line is raised.
 * The interrupt mask in the ethernet chip is set to only allow transmission
 * completed or error and packet reception interrupts. The rest are masked.
 *
 * The interrupt handler signals waiting processes that they can either
 * proceed with transmission of a packet, or that a new packet has been
 * received and can now be processed.
 *
 * The handler expects that register page 0 is selected.
 */

#pragma no_check_stack

static word EtherIntrHandler(NetDCB *dcb, word vector)
{
	int	status = dcb->lic->p0r.INTSTAT;

#ifdef DEBUG
	dcb->IntrCount++;
	dcb->IntrLastStat = status;
#endif

	if (status & INTSTAT_PRX) {
		/* Packet reception interrupt */
		dcb->lic->p0w.INTSTAT = INTSTAT_PRX;	/* clear interrupt */
#if 0
		/* Cannot call kernel functions via a trap interface in */
		/* interrupt handlers. */
		HardenedSignal(&dcb->CanReadPacket);	/* note packet avail */
#else
		/* note packet avail */
		PseudoTrap((word)&dcb->CanReadPacket, 0, 0, TRAP_HardenedSignal);
#endif
		return TRUE;
	}

	if (status & INTSTAT_PTX) {
		/* Packet transmitted interrupt */
		dcb->lic->p0w.INTSTAT = INTSTAT_PTX;	/* clear interrupt */
#if 0
		HardenedSignal(&dcb->CanWritePacket);
#else
		/* note tx avail */
		PseudoTrap((word)&dcb->CanWritePacket, 0, 0, TRAP_HardenedSignal);
#endif
		return TRUE;
	}

	if (status & INTSTAT_TXE) {
		/* Packet transmission error interrupt */
		dcb->lic->p0w.INTSTAT = INTSTAT_TXE;	/* clear interrupt */
#if 0
		HardenedSignal(&dcb->CanWritePacket);
#else
		/* note tx avail */
		PseudoTrap((word)&dcb->CanWritePacket, 0, 0, TRAP_HardenedSignal);
#endif
		return TRUE;
	}

#if 0
	return FALSE;
#else
	return status;
#endif
}


#ifdef DEBUG
word PanicHandler(Semaphore *psem, word vec) {

	/* Reset interrupt */
	hw_INTC->IRQRST = hw_intc_resetirq_panic;

#if 0
	/* Cannot call kernel functions directly in interrupt handlers */
	/* This will re-enable interrupts and cause chaos. */
	HardenedSignal(psem);
#else
	PseudoTrap((word)psem, 0, 0, TRAP_HardenedSignal);
#endif

	return TRUE;
}
#endif /* DEBUG */

/***************************************************************************/
/* end of wdether.c */
