/****************************************************************
*                                                               *
*                           HELIOS                              *
*                                                               *
*       Copyright (C) 1990, Perihelion Software Limited         *
*                       All Rights Reserved                     *
*                                                               *
*****************************************************************

File        : esupport.c
Description : support routines for driving nic chip
              #included in ether.c

RcsId: $Id: esupport.c,v 1.9 1993/01/21 14:40:37 paul Exp $

*****************************************************************
* Author: Alan Cosslett                              April 1990 *
****************************************************************/

#include "defs8390.h"
#include "defs583.h"

PRIVATE unsigned int nicbase;		/* actual addres of 8360 chip */
#define eb nicbase			/* alias of above	      */

PRIVATE UBYTE   StartIndex;
PRIVATE UBYTE   StopIndex;
PRIVATE UBYTE   NextPkt;
PRIVATE UBYTE*  StartPtr;
PRIVATE UBYTE*  StopPtr;
PRIVATE UBYTE   TxTimeout;
PRIVATE UBYTE   TxPktNo;
PRIVATE UBYTE*  TxPktPtr;
PRIVATE bool    TxAvail;
PRIVATE bool    Overflow;
PRIVATE bool    Overrun;
PRIVATE bool    Underrun;
PRIVATE bool    ResetMe;

/* the statistics */


PRIVATE USHORT prx_no = 0;      /* pkts received                */
PRIVATE USHORT ptx_no = 0;      /* pkts transmitted             */
PRIVATE USHORT rxe_no = 0;      /* pkts received with errors    */
PRIVATE USHORT txe_no = 0;      /* pkts transmitted with errors */
PRIVATE USHORT ovw_no = 0;      /* number of buffer overflows   */
PRIVATE USHORT cnt_no = 0;      /* number of counter overflows  */
PRIVATE USHORT col_no = 0;      /* number of collisions         */
PRIVATE USHORT abt_no = 0;      /* number of aborts             */
PRIVATE USHORT crs_no = 0;      /* number of carrier sense lost */
PRIVATE USHORT fu_no = 0;       /* number of fifo underruns     */
PRIVATE USHORT cdh_no = 0;      /* number of cd heartbeats      */
PRIVATE USHORT owc_no = 0;      /* number of out of window errs */
PRIVATE USHORT crc_no = 0;      /* number of crc errors         */
PRIVATE USHORT fae_no = 0;      /* number of frame alignment errs */
PRIVATE USHORT fo_no = 0;       /* number of fifo overruns      */
PRIVATE USHORT mpa_no = 0;      /* number of missed pkts        */
PRIVATE USHORT tap_no = 0;      /* number of thrown away pkts   */
PRIVATE USHORT int_no = 0;      /* number of times interrupted  */

/* function prototypes */

PRIVATE bool SendTxPkt();
PRIVATE SHORT GeEtRxPkt();

#ifdef ETHERINTS
PRIVATE void init_int_vector();
PRIVATE void restore_int_vector();
#endif

PRIVATE void HandleError();
PRIVATE void rxerror();
PRIVATE void txerror();
PRIVATE void resetboard();
PRIVATE bool RxAvail();

PUBLIC void ether_something();

#ifdef ETHERINTS
extern void set_einterrupts();
extern void restore_einterrupts();
#endif

typedef struct PktStatus
{
  UBYTE RStatus;
  UBYTE psNextPkt;
  UBYTE psRBCR0;
  UBYTE psRBCR1;
} PktStatus;

/* routine to check the board to see if its there       */
/* and if it is initialise the word                     */

PRIVATE bool etherboardp()
{
  int i;

  /* initialise ethernet board */
  InitBoard();

  StartIndex = (UBYTE)8+TxPktNo;
  StopIndex  = (UBYTE)0x20+TxPktNo;
  TxTimeout  = 0;
  StartPtr   = &((UBYTE*)ethermem)[StartIndex<<8];  
  StopPtr    = &((UBYTE*)ethermem)[StopIndex<<8];
  TxAvail    = true;
  
  ResetMe = false;
  Overrun = false;
  Underrun = false;
  Overflow = false;

  /* make sure the board is stopped */
  /* probably do not need           */
  
  outp(eb+CR,0x21);
  outp(eb+IMR,0);

 /* initialise the board just as it says in the docs */

  outp(eb+CR,0x21);             /* Stage 1 - Stop The Board             */
  outp(eb+DCR,0x48);            /* Stage 2 - Set Data Config Register   */
  outp(eb+RBCR0,0);             /* Stage 3 - Clear Remote Byte Counts   */
  outp(eb+RBCR1,0);             /* Stage 3 - Clear Remote Byte Counts   */

  outp(eb+RCR,etherrcr);        /* Stage 4 - Set the receive confi reg  */
  outp(eb+TCR,4);               /* Stage 5 - put nic in loopback mode 2 */
  outp(eb+BNRY,StartIndex);     /* Stage 6 - init boundary ptr          */
  outp(eb+PSTART,StartIndex);   /* Stage 6 - init page start ptr        */
  outp(eb+PSTOP,StopIndex);     /* Stage 6 - init page stop ptr         */
  outp(eb+ISR,0xFF);            /* Stage 7 - clear interrupt status     */
  outp(eb+IMR,0);               /* Stage 8 - no interrupts              */
  outp(eb+CR,0x61);             /* Stage 9 - set to page 1              */

  for (i = 0; i < 6; i++)
  {
    outp(eb+PAR0+i,etheraddr[i]); /* Stage 9 - set ether net address    */      
    outp(eb+MAR0+i,ethermult[i]); /* Stage 9 - set multicast address    */
  }

  outp(eb+MAR6,ethermult[6]);   /* Stage 9 - set multicast address    */
  outp(eb+MAR7,ethermult[7]);   /* Stage 9 - set multicast address    */

  outp(eb+CURR,StartIndex+1);   /* Stage 9 initialise current pointer */
  
  NextPkt = StartIndex+1;       /* Stage 9 initialise next packet     */

  StopIndex--;                  /* make life easier for later         */

  outp(eb+CR,0x22);             /* Stage 10 - start the nic           */
  outp(eb+TCR,0);               /* Stage 11 - out of loopback mode    */   

  return true;
 
}

/* SendTxPkt - send a pkt to the net - if we can  */
/* N.B. the chip SHOULD interrupt when the pkt has */
/* gone causing the interrupt routine to set TxAvail */
/* However this cannot be relied on so we also set */
/* up TxTimeout as a failsafe and assume the packet has */
/* gone if this ever reaches zero. I use two variable TxTimeout */
/* and TxAvail so we do not have problems of the interrupt routine */
/* changing things half way through this routine and hence TxTimeout */
/* getting set to -1. This could be worked around. */

PRIVATE bool SendTxPkt(buff,len)
byte*  buff;
USHORT len;
{
#ifdef ETHERDEBUG	
  ServerDebug("/ether - ptx %d prx %d int %d %2x",ptx_no,prx_no,int_no,inp(eb+ISR));
  ServerDebug("/ether - chucked %d overruns %d  underruns %d overflows %d",
               tap_no,fo_no,fu_no,ovw_no);
#endif

  if (TxAvail || (TxTimeout == 0))
  {
    memcpy(TxPktPtr,buff,len);

    if (len <= 64) len = 64;
     
    outp(eb+TPSR,TxPktNo);  /* set up transmit page start */

    outp(eb+TBCR0,len % 0xFF);  /* set lower byte of length  */
    outp(eb+TBCR1,len / 0xFF);  /* set higher byte of length */
    outp(eb+CR,0x26);		/* transmit it		     */
    TxTimeout = 0xFF;		/* Kludge -- Set a timeout   */
    return true;		
  }
  else
  { 
    TxTimeout--;		/* dec the timeout           */
    return false;
  }

}

/* routine to set up interrupt stuff */
#ifdef ETHERINT
PRIVATE void init_int_vector()
{
  /* set up the interrupt vector */

  int level = etherlevel + 8;		/* plus 8 for some reason */
  
  byte mask = inp(0x21);		/* read old mask	 */
  outp(0x21, mask & ~(1 << etherlevel));/* set our level	 */
  set_einterrupts(level);		/* set our handler       */
  
  /* outp(eb+IMR,0x3f);  NO INTS ! turn on board ints    */
}

/* restore interrupts to how they were */

PRIVATE void restore_int_vector()
{
  byte mask = inp(0x21);		/* get int mask */
  outp(eb+CR,0x21);			/* stop the board */
  outp(eb+IMR,0);			/* stop board interrupting */
  restore_einterrupts(etherlevel+8);	/* put back old handler  */
  outp(0x21, mask | (1 << etherlevel)); /* turn of etherlevel interrupts   */
}
#endif

/* now for the interrupt routine */

void ether_something()
{
  /* ok we may or may not have been interrupted */

  byte status = (byte)inp(eb+ISR);    /* get in status */
  outp(eb+ISR,status);          /* and stop the ints */
  outp(eb+IMR,0);
  
  int_no ++;
  
/*  outp(0x20,0x20);               MAGIC to Enable lower interrups ? */

  if (status & (OVW|RXE|TXE|CNT)) HandleError(status);

  if (status & PTX) 		/* we have transmitted a pkt */
  {
    ptx_no++;
    TxAvail = true;
  }

  /*  if (status & PRX) RxAvail = true; */
  /* outp(eb+IMR,0x3f);	 */
}

/* increment some stats and set ResetMe flag if ness */

PRIVATE void HandleError(status)
byte status;
{
  if (status & RXE) rxerror(status);
  if (status & TXE) txerror(status);
  if (status & OVW) 
  { 
    Overflow = true;
    ResetMe = true;
    ovw_no ++;
  }
  if (status & CNT) cnt_no++;
}

/* handle recption errors */

PRIVATE void rxerror(status)
byte status;
{
  rxe_no ++;
  
 if (status & CRC) crc_no ++;
 if (status & FAE) fae_no ++;
 if (status & FO)  
 {
   fo_no ++;
   Overrun = true;
   ResetMe = true;
 }   

 if (status & MPA) mpa_no++;

}

/* handle transmission errors */

PRIVATE void txerror(status)
byte status;
{
  txe_no ++;
  
  TxAvail = true;		/* ok to send more pkts */

  if (status & COL) col_no++;
  if (status & ABT) abt_no++;
  if (status & CRS) crs_no++;
  if (status & FU)  
  {
    fu_no ++;
    Underrun = true;
    ResetMe  = true;
  }

  if (status & CDH) cdh_no ++;
  if (status & OWC) owc_no ++;
} 

/* reset the board - do this for Ring buffer overflows
   fifo underruns and overruns (these last two should
   not really happen) but occasionally do on the amiga */

PRIVATE void resetboard()
{
  UBYTE status;
  SHORT res = 0;

#ifdef ETHERINTS
  _disable();				/* stop ints screwing us */
#endif

  outp(eb+IMR,0);	
  outp(eb+CR,0x21);			/* start reset */
  outp(eb+RBCR0,0);
  outp(eb+RBCR1,0);

  /* wait for board to reset */

  do 	
  {
    status = (UBYTE)inp(eb+ISR);
    outp(eb+ISR,status);
  } while ((status & 0x80) == 0);

  outp(eb+TCR,2);			/* loop back mode */
  outp(eb+CR,0x22);			/* start the board */
  
  /* remove pkts from the ring */
  /* the reason we have the while loop is so we can use break */

  while (RxAvail())
  { 

    PktStatus* psp =(PktStatus*)(&((UBYTE*)ethermem)[NextPkt<<8]);
    unless (psp->RStatus & 1) break;
    tap_no ++;

    NextPkt = psp->psNextPkt;
     
    if (NextPkt == StartIndex)
    {
      outp(eb+BNRY,StopIndex);
    }
    else
    {
      outp(eb+BNRY,NextPkt-1);
    }

  }
  
  outp(eb+ISR,0xFF); 			/* clear interrupt status */

  ResetMe = Overflow = Underrun = Overrun = false;

#ifdef ETHERINTS
  _enable();
#endif
  
  outp(eb+TCR,0);
  outp(eb+IMR,0x3f);

}

/* see if we have a packet available to read */
/* have to disable as interrupt handler might change page */

PRIVATE bool RxAvail()
{
  bool res;
 
#ifdef ETHERINTS
  _disable();				
#endif

  outp(eb+CR,0x62);
  res = (NextPkt != (UBYTE)inp(eb+CURR));
  outp(eb+CR,0x22);

#ifdef ETHERINTS  
  _enable();  
#endif

  return res;
}


/* GetRxPkt - gets a packet from the ring buffer 
   must only be called if there is one there 
   check with RxAvail */

PRIVATE SHORT GetRxPkt(buff,maxsize)
UBYTE* buff;
word maxsize;
{ 
  PktStatus * ps = (PktStatus*) (&((UBYTE*)ethermem)[NextPkt<<8]);

  UBYTE* from = ((UBYTE*)ps)+4;
  SHORT  len  = 0;

  /* check for duff packet */
  /* shoulkd never happen if RCR is set up correctly */

  unless (ps->RStatus & 1)
  {
        
#ifdef ETHERDEBUG
    ServerDebug("/ether - byte %2x ",ps->RStatus);
    ServerDebug("/ether - byte %2x ",ps->psNextPkt);
    ServerDebug("/ether - byte %2x ",ps->psRBCR0); 
    ServerDebug("/ether - byte %2x ",ps->psRBCR1);
#endif    

    Server_errno = EC_Error +SS_IOProc + EG_Broken + EO_Medium;
    return 0;
  } 

  len = *((SHORT*)(&ps->psRBCR0));		/* length of packet */
 
  if (((word)len) > maxsize) 
  {
    len = (SHORT) maxsize;
  }
  
  if (from + len  <= StopPtr)			/* check for wrap */
  {
    memcpy(buff,from,len);			/* easy case */
  }
  else
  {
    SHORT len1;					/* harder case */
    memcpy(buff,from,len1 = (SHORT) (StopPtr-from));
    memcpy(buff+len1,StartPtr,len-len1);
  }
  
  NextPkt = ps->psNextPkt;			/* on to next */
  
  if (NextPkt == StartIndex)			
  {
    outp(eb+BNRY,StopIndex);			/* special case if wrap */
  } 
  else
  {
    outp(eb+BNRY,NextPkt -1);			/* else easy */
  }
  
  prx_no++;                                     /* gather stats */

  Server_errno = 0 ;				/* all ok */
  return len ;					/* and home */
}



/* Initialise board */

static bool InitBoard()
{
	int i;
	UBYTE cs = 0;
	unsigned int etherport = etherbase; 

	/* set up default values for some variable if ness */

	if (etherbase == 0xFF) etherport = 0x280;

	nicbase = etherport + 0x10;

	for ( i = 0; i < 8; i++) {
		cs += (etheraddr[i] = (UBYTE) inp(etherport+i+0x08)); 
	}

	/* unless checksum is 0xFF board is not Western Digital */
	if (cs != 0xff) {
		IOdebug("Ethernet card checksum error 0x%x != 0xff",cs);	
		return false;
	}

	if (etherbase == 0xFF)	etherbase = 0x280;
	if (ethermem  == -1)	ethermem = 0xD0000000;
	if (etherlevel == -1)	etherlevel = 3;

	/* ok we have a board lets initialise it */
	outp(etherbase,(int)( ((ethermem >> 25) & ~0x80) | 0x40));

	TxPktNo    = 0;
	TxPktPtr   = (UBYTE*)ethermem;

	return true;
}
