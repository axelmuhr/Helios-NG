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

RcsId: $Id: esupport.c,v 1.3 1992/05/04 19:24:04 craig Exp $

*****************************************************************
* Author: Alan Cosslett                              April 1990 *
****************************************************************/

#ifdef OLD_ETHER
#include "defs8390.h"
#include "defs583.h"

PRIVATE unsigned int nicbase;		/* actual addres of 8360 chip */
#define eb nicbase			/* alias of above	      */

#define BT_UNKNOWN 0
#define BT_WESTERN_DIGITAL 1
#define BT_DLINK 2
#endif /* OLD_ETHER */

#ifndef OLD_ETHER

/*
-- ===========================================================================
-- Support for Clarkson Packet Drivers
-- crf: May 1992
-- ===========================================================================
*/

#include "etherdef.h"

/*
-- packet driver s/w interrupt
*/
PRIVATE SHORT pkt_int_no = -1 ;	
/*
-- associates access to packet type
*/
PRIVATE	SHORT drvr_handle [NUM_HANDLES] = { -1, -1 } ;
/*
-- table of packet buffers
*/
PRIVATE BYTE ether_pkt_table [MAX_PKT_TABLE] [MAXETHERPKT] ;
/*
-- packet length vector
*/
PRIVATE SHORT ether_pkt_len [MAX_PKT_TABLE] ;
/*
-- index into packet buffer table
*/
PRIVATE SHORT pkt_index = 0 ;

/*
-- Data used by receiver routine 
-- packet received - incremented by receiver routine, decremented when packet consumed
*/
PUBLIC SHORT pkt_rcvd = 0 ;
/*
-- pointer to packet buffer table
*/
PUBLIC BYTE* pkt_table = ether_pkt_table [0] ;
/*
-- pointer to packet length vector
*/
PUBLIC SHORT *pkt_len = ether_pkt_len ;
/*
-- rx errors (set by receiver routine)
*/
PUBLIC SHORT pkt_too_long = 0 ;
#ifdef ETHER_DEBUG
PUBLIC SHORT pkt_overflow = 0 ;
#endif /* ETHER_DEBUG */

/* 
-- Errors returned from packet driver 
*/
PRIVATE SHORT drvr_err = 0 ;
PRIVATE BYTE *Drvr_Err_Code [NUM_ERR_CODES] =
{
"No error",							/* NO_ERROR		0 */
"Invalid handle number",					/* BAD_HANDLE		1 */
"No interfaces of specified class found",			/* NO_CLASS		2 */
"No interfaces of specified type found",			/* NO_TYPE		3 */
"No interfaces of specified number found",			/* NO_NUMBER		4 */
"Bad packet type specified",					/* BAD_TYPE		5 */
"Interface does not support multicast",				/* NO_MULTICAST		6 */
"Packet driver cannot terminate",				/* CANT_TERMINATE	7 */
"Invalid receiver mode was specified",				/* BAD_MODE		8 */
"Operation failed - insufficient space",			/* NO_SPACE		9 */
"Type previously accessed and not released",			/* TYPE_INUSE		10 */
"Command out of range or not implemented",			/* BAD_COMMAND		11 */
"Packet couldn't be sent (usually hardware error)",		/* CANT_SEND		12 */
"Hardware address couldn't be changed (> 1 handle open)",	/* CANT_SET		13 */
"Hardware address has bad length or format",			/* BAD_ADDRESS		14 */
"Couldn't reset interface (> 1 handle open)"			/* CANT_RESET		15 */
} ;

/*
-- Ethertypes
*/
PRIVATE USHORT ether_type [NUM_HANDLES] = { ETHERTYPE_ARP , ETHERTYPE_IP } ;

#ifdef ETHER_STATS
typedef struct Ether_Stats
{
	UWORD pkts_in ;
	UWORD pkts_out ;
	UWORD bytes_in ;
	UWORD bytes_out ;
	UWORD errors_in ;
	UWORD errors_out ;
	UWORD pkts_lost ;
} Ether_Stats ;
#endif /* ETHER_STATS */

#define TEST_ERROR			\
	if (regs.x.cflag)		\
	{				\
		drvr_err = regs.h.dh ;	\
		return -1 ;		\
	}

/*
-- ===========================================================================
-- Routines that interact directly with the packet drivers
-- ===========================================================================
*/

PRIVATE SHORT drvr_info (SHORT, 
#ifdef ETHER_DEBUG
			SHORT, SHORT *, SHORT *, BYTE *,
#endif /* ETHER_DEBUG */
			SHORT *, SHORT *, SHORT *) ;

/*
-- assembler receiver routine
-- activated by packet driver when packet comes in
*/
extern void receiver (void) ;
PRIVATE SHORT access_type (SHORT, SHORT, SHORT, SHORT, BYTE *, USHORT,
			SHORT (*) (void)) ;

PRIVATE SHORT get_address (SHORT, SHORT, BYTE *, SHORT) ;
PRIVATE SHORT release_type (SHORT, SHORT) ;
PRIVATE SHORT send_pkt (SHORT, BYTE *, USHORT) ;

#ifdef ETHER_STATS
PRIVATE SHORT get_stats (SHORT, SHORT, Ether_Stats *) ;
#endif /* ETHER_STATS */

/*
-- ===========================================================================
-- Support routines
-- ===========================================================================
*/
PRIVATE SHORT locate_pkt_drvr (USHORT) ;
PRIVATE bool clarksonp (void) ;
PRIVATE bool fail (BYTE *, ...) ;
PRIVATE void notify (BYTE *, ...) ;
PRIVATE void notify_rx_error (void) ;
PRIVATE void release_all (SHORT, SHORT *) ;
#ifdef ETHER_STATS
PRIVATE void show_ether_statistics (SHORT, SHORT) ;
#endif /* ETHER_STATS */

/*
-- ===========================================================================
-- Routines that interact directly with the packet drivers
-- ===========================================================================
*/

PRIVATE SHORT drvr_info (SHORT int_no, 
#ifdef ETHER_DEBUG
			SHORT handle, SHORT *version, SHORT *drvr_funct,
			BYTE *if_name, 
#endif /* ETHER_DEBUG */
			SHORT *if_class, SHORT *if_type, SHORT *if_number)
{
	union REGS regs;
	struct SREGS sregs ;
#ifdef ETHER_DEBUG
	BYTE *name_ptr ;
#endif /* ETHER_DEBUG */

	segread (&sregs) ;
#ifdef ETHER_DEBUG
	regs.x.bx = handle ;
#endif /* ETHER_DEBUG */
	regs.h.ah = DRIVER_INFO ;
	regs.h.al = 0xff ;
	int86x (int_no, &regs, &regs, &sregs) ;
	TEST_ERROR
	*if_class   = regs.h.ch ;
	*if_type    = regs.x.dx ;
	*if_number  = regs.h.cl ;
#ifdef ETHER_DEBUG
	*version  = regs.x.bx ;
	*drvr_funct = regs.h.al ;
	FP_SEG (name_ptr) = sregs.ds ;
	FP_OFF (name_ptr) = regs.x.si ;
	if (strlen (name_ptr) >= IFACE_NAME_LEN)
		name_ptr [IFACE_NAME_LEN] = '\0' ;
	strncpy (if_name, name_ptr, strlen (name_ptr) + 1) ;
#endif /* ETHER_DEBUG */
	return 0 ;
}

/*
------------------------------------------------------------------------------
*/

PRIVATE SHORT access_type (SHORT int_no,
			SHORT if_class, SHORT if_type, SHORT if_number,
			BYTE *buffer, USHORT buf_len,
			SHORT (*receiver) (void))
{
	union REGS regs ;
	struct SREGS sregs ;

	segread (&sregs) ;
	regs.h.dl = (UBYTE) if_number ;
	sregs.ds  = FP_SEG (buffer) ;
	regs.x.si = FP_OFF (buffer) ;
	regs.x.cx = buf_len ;
	sregs.es  = FP_SEG (receiver) ;
	regs.x.di = FP_OFF (receiver) ;
	regs.x.bx = if_type ;
	regs.h.ah = ACCESS_TYPE ;
	regs.h.al = (UBYTE) if_class ;
	int86x (int_no, &regs, &regs, &sregs) ;
	TEST_ERROR
	return regs.x.ax ;
}

/*
------------------------------------------------------------------------------
*/

PRIVATE SHORT get_address (SHORT int_no, SHORT handle, 
			BYTE *buffer, SHORT buf_len)
{
	union REGS regs ;
	struct SREGS sregs ;

	segread (&sregs) ;
	sregs.es  = FP_SEG (buffer) ;
	regs.x.di = FP_OFF (buffer) ;
	regs.x.cx = buf_len ;
	regs.x.bx = handle ;
	regs.h.ah = GET_ADDRESS ;
	int86x (int_no, &regs, &regs, &sregs) ;
	TEST_ERROR
	return (regs.x.cx) ;
}

/*
------------------------------------------------------------------------------
*/

PRIVATE SHORT release_type (SHORT int_no, SHORT handle)
{
	union REGS regs ;
	regs.x.bx = handle ;
	regs.h.ah = RELEASE_TYPE ;
	int86 (int_no, &regs, &regs) ;
	TEST_ERROR
	return 0 ;
}

/*
------------------------------------------------------------------------------
*/

PRIVATE SHORT send_pkt (SHORT int_no, BYTE *buffer, USHORT buf_len)
{
	union REGS regs ;
	struct SREGS sregs ;

	segread (&sregs) ;
	sregs.ds = FP_SEG (buffer) ;
#ifdef DRIVER_BUG
	sregs.es = FP_SEG (buffer) ;
#endif
	regs.x.si = FP_OFF (buffer) ;
	regs.x.cx = buf_len ;
	regs.h.ah = SEND_PKT ;
	int86x (int_no, &regs, &regs, &sregs) ;
	TEST_ERROR
	return 0 ;
}

/*
------------------------------------------------------------------------------
*/

#ifdef ETHER_STATS
PRIVATE SHORT get_stats (SHORT int_no, SHORT handle, Ether_Stats *stats)
{
	Ether_Stats *stats_ptr ;
	union REGS regs ;
	struct SREGS sregs ;

	segread (&sregs) ;
	regs.h.ah = GET_STATISTICS ;
	regs.x.bx = handle ;
	
	int86x (int_no, &regs, &regs, &sregs) ;

	FP_SEG (stats_ptr) = sregs.ds ;
	FP_OFF (stats_ptr) = regs.x.si ;
	memcpy (stats, stats_ptr, sizeof (Ether_Stats)) ;
	
	TEST_ERROR
	return regs.x.ax ;
}
#endif /* ETHER_STATS */

/*
-- ===========================================================================
-- Support routines
-- ===========================================================================
*/

PRIVATE SHORT locate_pkt_drvr (USHORT int_no)
{
	WORD drvr_vec ;

	drvr_vec = (WORD) _dos_getvect (int_no) ;
	return (!strncmp ((BYTE *) (drvr_vec + 3) , DRVR_SIG, strlen (DRVR_SIG))) ;
}

/*
------------------------------------------------------------------------------
*/

PRIVATE bool clarksonp ()
{
	bool located = FALSE ;

	if (pkt_int_no == -1)
	{
#ifdef ETHER_DEBUG
		notify ("searching for pkt drvr") ;
#endif /* ETHER_DEBUG */
		pkt_int_no = MIN_PKT_INT ;
		while (pkt_int_no <= MAX_PKT_INT)
		{		
			located = locate_pkt_drvr (pkt_int_no) ;
			if (located)
				break ;
			pkt_int_no ++ ;
		}
		if (!located)
			return (fail ("%s in range 0x%x - 0x%x", CANT_LOCATE, MIN_PKT_INT, MAX_PKT_INT)) ;
	}
	else
	{
		if ((pkt_int_no < MIN_PKT_INT) || (pkt_int_no > MAX_PKT_INT))
			return (fail ("Invalid %s (0x%x) - must be in range 0x%x-0x%x", HOST_PKT_INT, pkt_int_no, MIN_PKT_INT, MAX_PKT_INT)) ;
		located = locate_pkt_drvr (pkt_int_no) ;
		if (!located)
			return (fail ("%s at 0x%x (%d)", CANT_LOCATE, pkt_int_no, pkt_int_no)) ;
	}
/*#ifdef ETHER_DEBUG*/
	notify ("Located packet driver at 0x%x (%d)", pkt_int_no, pkt_int_no) ;
/*#endif*/ /* ETHER_DEBUG */

	{
		SHORT iface_class, iface_type, iface_number ;
#ifdef ETHER_DEBUG
		SHORT version ;
		BYTE iface_name [IFACE_NAME_LEN] ;
		SHORT drvr_funct ;
#endif /* ETHER_DEBUG */

		if (drvr_info (pkt_int_no, 
#ifdef ETHER_DEBUG
				drvr_handle[0], &version, &drvr_funct,
				(BYTE *) &iface_name,
#endif /* ETHER_DEBUG */
				&iface_class, &iface_type, &iface_number) < 0)
			return (fail ("Failed to get interface information")) ;

#ifdef ETHER_DEBUG
		notify ("version = 0x%x (%d)", version, version) ;
		notify ("iface_class = 0x%x (%d)", iface_class, iface_class) ;
		notify ("iface_type = 0x%x (%d)", iface_type, iface_type) ;
		notify ("iface_number = 0x%x (%d)", iface_number, iface_number) ;
		notify ("drvr_funct = 0x%x (%d)", drvr_funct, drvr_funct) ;
		notify ("iface_name = %s", iface_name) ;			
#endif /* ETHER_DEBUG */

		{
			SHORT i ;
			for (i = 0 ; i < NUM_HANDLES ; i ++)
			{	
				USHORT pkt_type = ether_type [i] ;
				drvr_handle[i] = access_type (pkt_int_no, 
						iface_class, iface_type, iface_number,
						(BYTE *) &pkt_type, PKT_TYPE_LEN, 
						&receiver) ;
				if (drvr_handle[i] < 0)
					return (fail ("Failed to initiate access to packet type")) ;
#ifdef ETHER_DEBUG
				notify ("drvr_handle [%d] = 0x%x (%d)", i, drvr_handle[i], drvr_handle[i]) ;
#endif /* ETHER_DEBUG */
			}
		}
	}

	{
		SHORT addr_len ;
		addr_len = get_address (pkt_int_no, drvr_handle[0], etheraddr, sizeof (etheraddr)) ;
		if (addr_len < 0)
			return (fail ("Failed to get Ethernet address")) ;
/*#ifdef ETHER_DEBUG*/
		notify ("Ethernet Address: %02x:%02x:%02x:%02x:%02x:%02x",
			etheraddr[0],etheraddr[1],etheraddr[2],etheraddr[3],etheraddr[4],etheraddr[5]) ;
/*#endif*/ /* ETHER_DEBUG */
	}
	return (TRUE) ;
}

/*
------------------------------------------------------------------------------
*/

#include <stdarg.h>

PRIVATE bool fail (BYTE *format, ...)
{
	va_list args;
	BYTE msg [256] ;

	va_start (args, format);
	vsprintf (msg, format, args) ;    
	va_end (args);
	notify (msg) ;
	if (drvr_err > 0)
	{
		if (drvr_err < NUM_ERR_CODES)
			notify ("Error %d: %s", drvr_err, Drvr_Err_Code [drvr_err]) ;
		else
			notify ("Unknown error code: 0x%x", drvr_err) ;
		drvr_err = 0 ;
	}
	release_all (pkt_int_no, drvr_handle) ;
	return (FALSE) ;
}

/*
------------------------------------------------------------------------------
*/

PRIVATE void notify (BYTE *format, ...)
{
	va_list args;
	BYTE msg [256] ;
	va_start (args, format);
	vsprintf (msg, format, args) ;    
	ServerDebug ("/ether: %s", msg) ;
	va_end (args);
}

/*
------------------------------------------------------------------------------
*/

PRIVATE void notify_rx_error ()
{
	if (pkt_too_long)
	{
		notify ("RX error - bad packet size (%d bytes) - packet discarded", pkt_too_long) ;
		pkt_too_long = 0 ;
	}
#ifdef ETHER_DEBUG
	if (pkt_overflow)
	{
		notify ("Buffer overflow - packet discarded") ; 
		pkt_overflow = 0 ;
	}
#endif /* ETHER_DEBUG */
}

/*
------------------------------------------------------------------------------
*/

PRIVATE void release_all (SHORT int_no, SHORT *handle)
{
	SHORT i ;
	for (i = 0 ; i < NUM_HANDLES ; i ++)
	{
		SHORT curr_handle = handle [i] ;
		if (curr_handle != -1)
		{
			if (release_type (int_no, curr_handle) < 0)
				(void) fail ("Failed to release access to packet type") ;
#ifdef ETHER_DEBUG
			else
				notify ("released handle: 0x%x (%d)", curr_handle, curr_handle) ;
#endif /* ETHER_DEBUG */
		}
	}
}

/*
------------------------------------------------------------------------------
*/

#ifdef ETHER_STATS
PRIVATE void show_ether_statistics (SHORT int_no, SHORT handle)
{
	Ether_Stats stats ;
	if (get_stats (int_no, handle, &stats) < 0)
	{
		(void) fail ("Failed to get statistics") ;
		return ;
	}
	notify ("Statistics") ;
	notify ("pkts in    = %d", stats.pkts_in) ;
	notify ("pkts out   = %d", stats.pkts_out) ;
	notify ("bytes in   = %d", stats.bytes_in) ;
	notify ("bytes out  = %d", stats.bytes_out) ;
	notify ("errors in  = %d", stats.errors_in) ;
	notify ("errors out = %d", stats.errors_out) ;
	notify ("pkts lost  = %d", stats.pkts_lost) ;
}
#endif /* ETHER_STATS */

/*
-- ===========================================================================
-- Unused routines
-- ===========================================================================
*/

#if 0

PRIVATE SHORT reset_interface (SHORT int_no, SHORT handle)
{
/*
-- crf: careful with this ...
*/
	union REGS regs ;
	regs.x.bx = handle ;
	regs.h.ah = RESET_INTERFACE ;
	int86 (int_no, &regs, &regs) ;
	TEST_ERROR
	return 0 ;
}

/*
------------------------------------------------------------------------------
*/

PRIVATE SHORT terminate (SHORT int_no, SHORT handle)
{
	union REGS regs ;
	regs.x.bx = handle ;
	regs.h.ah = TERMINATE ;
	int86 (int_no, &regs, &regs) ;
	TEST_ERROR
	return 0 ;
}
#endif /* 0 (unused) */

/*
-- ===========================================================================
-- End of Support for Clarkson Packet Drivers
-- ===========================================================================
*/

#endif /* !OLD_ETHER */

#ifdef OLD_ETHER
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

PRIVATE bool etherboardp();		/* do we have a board */
PRIVATE int getboardtype();		/* returns type of board */
PRIVATE bool westerndigp();		/* check for wesetrn dig board */
PRIVATE bool dlinkp();			/* check for dlink board */
PRIVATE void dlinkaddr();		/* find the board address */
#endif /* OLD_ETHER */

PRIVATE void tidyboard(void);		
#ifdef OLD_ETHER
PRIVATE bool SendTxPkt();
PRIVATE SHORT GeEtRxPkt();
#else /* !OLD_ETHER */
PRIVATE bool SendTxPkt(BYTE*, USHORT) ;
PRIVATE SHORT GetRxPkt (UBYTE*, WORD) ;
#endif /* !OLD_ETHER */

#ifdef OLD_ETHER
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

  if (getboardtype() == BT_UNKNOWN) return FALSE; 

  StartIndex = (UBYTE)8+TxPktNo;
  StopIndex  = (UBYTE)0x20+TxPktNo;
  TxTimeout  = 0;
  StartPtr   = &((UBYTE*)ethermem)[StartIndex<<8];  
  StopPtr    = &((UBYTE*)ethermem)[StopIndex<<8];
  TxAvail    = TRUE;
  
  ResetMe = FALSE;
  Overrun = FALSE;
  Underrun = FALSE;
  Overflow = FALSE;

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

  /* if we are a dlink board we can get the address now */
    
  if (ethertype == BT_DLINK) dlinkaddr(); 

  return TRUE;
 
}
#endif /* OLD_ETHER */ 

PRIVATE void tidyboard()
{
#ifdef OLD_ETHER
  if (ethertype == BT_DLINK) outp(etherbase+0x1F,0);	 
  if (ethertype == BT_WESTERN_DIGITAL) outp(etherbase,0);
#else /* !OLD_ETHER */
#ifdef ETHER_STATS
  show_ether_statistics (pkt_int_no, drvr_handle [0]) ;
#endif /* ETHER_STATS */
  release_all (pkt_int_no, drvr_handle) ;
#endif /* !OLD_ETHER */
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
BYTE*  buff;
USHORT len;
{
#ifdef OLD_ETHER

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
    return TRUE;		
  }
  else
  { 
    TxTimeout--;		/* dec the timeout           */
    return FALSE;
  }

#else /* !OLD_ETHER */
#ifdef ETHER_DEBUG
  notify ("tx: pkt len = %d", len) ;
#endif /* ETHER_DEBUG */
  if (send_pkt (pkt_int_no, buff, len) < 0)
    return (fail ("Failed to send packet")) ;
  return (TRUE) ;
#endif /* !OLD_ETHER */
}

#ifdef OLD_ETHER 
/* routine to set up interrupt stuff */
#ifdef ETHERINT
PRIVATE void init_int_vector()
{
  /* set up the interrupt vector */

  int level = etherlevel + 8;		/* plus 8 for some reason */
  
  BYTE mask = inp(0x21);		/* read old mask	 */
  outp(0x21, mask & ~(1 << etherlevel));/* set our level	 */
  set_einterrupts(level);		/* set our handler       */
  
  /* outp(eb+IMR,0x3f);  NO INTS ! turn on board ints    */
}

/* restore interrupts to how they were */

PRIVATE void restore_int_vector()
{
  BYTE mask = inp(0x21);		/* get int mask */
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

  BYTE status = (BYTE)inp(eb+ISR);    /* get in status */
  outp(eb+ISR,status);          /* and stop the ints */
  outp(eb+IMR,0);
  
  int_no ++;
  
/*  outp(0x20,0x20);               MAGIC to Enable lower interrups ? */

  if (status & (OVW|RXE|TXE|CNT)) HandleError(status);

  if (status & PTX) 		/* we have transmitted a pkt */
  {
    ptx_no++;
    TxAvail = TRUE;
  }

  /*  if (status & PRX) RxAvail = TRUE; */
  /* outp(eb+IMR,0x3f);	 */
}

/* increment some stats and set ResetMe flag if ness */

PRIVATE void HandleError(status)
BYTE status;
{
  if (status & RXE) rxerror(status);
  if (status & TXE) txerror(status);
  if (status & OVW) 
  { 
    Overflow = TRUE;
    ResetMe = TRUE;
    ovw_no ++;
  }
  if (status & CNT) cnt_no++;
}

/* handle recption errors */

PRIVATE void rxerror(status)
BYTE status;
{
  rxe_no ++;
  
 if (status & CRC) crc_no ++;
 if (status & FAE) fae_no ++;
 if (status & FO)  
 {
   fo_no ++;
   Overrun = TRUE;
   ResetMe = TRUE;
 }   

 if (status & MPA) mpa_no++;

}

/* handle transmission errors */

PRIVATE void txerror(status)
BYTE status;
{
  txe_no ++;
  
  TxAvail = TRUE;		/* ok to send more pkts */

  if (status & COL) col_no++;
  if (status & ABT) abt_no++;
  if (status & CRS) crs_no++;
  if (status & FU)  
  {
    fu_no ++;
    Underrun = TRUE;
    ResetMe  = TRUE;
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

  ResetMe = Overflow = Underrun = Overrun = FALSE;

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
#endif /* OLD_ETHER */

/* GetRxPkt - gets a packet from the ring buffer 
   must only be called if there is one there 
   check with RxAvail */

PRIVATE SHORT GetRxPkt(buff,maxsize)
UBYTE* buff;
WORD maxsize;
{ 
#ifdef OLD_ETHER
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
 
  if (((WORD)len) > maxsize) 
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

#else /* !OLD_ETHER */

  SHORT  len  = 0;
/*
-- crf: XXX
*/
  _asm
    cli
  len = pkt_len [pkt_index]  ;

#ifdef ETHER_DEBUG
  notify ("rx: pkt len = %d  index = %d", len, pkt_index) ;
#endif /* ETHER_DEBUG */

  if ((WORD) len > maxsize) 
    len = (SHORT) maxsize;
  memcpy (buff, ether_pkt_table [pkt_index], len) ;
  pkt_index = ++pkt_index % MAX_PKT_TABLE ;
  pkt_rcvd -- ;

/*
-- crf: XXX
*/
  _asm
    sti

#endif /* !OLD_ETHER */  
  Server_errno = 0 ;				/* all ok */
  return len ;					/* and home */
}

#ifdef OLD_ETHER
/****************************************************************
*
* getboardtype - get and initialise board if pos - return type
*
****************************************************************/

PRIVATE int getboardtype()
{
  switch (ethertype) 
  {
    case BT_WESTERN_DIGITAL: 
	 if (westerndigp()) return ethertype; 
         break; 
    case BT_DLINK:
         if (dlinkp()) return ethertype;
         break;
    default:
	 if (westerndigp()) return ethertype; 
         if (dlinkp()) return ethertype;
         break;
  }
  return ethertype = BT_UNKNOWN;
}


/****************************************************************
*
* westerndigp - see if board is western digitals 
*
****************************************************************/

PRIVATE bool westerndigp()
{
  int i;
  UBYTE cs = 0;
  unsigned int etherport = etherbase; 

  /* set up default values for some variable if ness */

  if (etherbase == 0xFF) etherport = 0x280;

  nicbase = etherport + 0x10;


  for ( i = 0; i < 8; i++)
  {
    cs += (etheraddr[i] = (UBYTE) inp(etherport+i+0x08)); 
  }

  /* unless checksum is 0xFF board is not Western Digital */

  unless (cs == 0xFF) 
  {
#ifdef ETHERDEBUG
    ServerDebug("/ether - checksum %2X",cs);	
#endif  
    return FALSE;
  }

  if (etherbase == 0xFF) etherbase = 0x280;
  if (ethermem  == -1)   ethermem = 0xD0000000;
  if (etherlevel == -1) etherlevel = 3;

  /* ok we have a board lets initialise it */

  outp(etherbase,(int)( ((ethermem >> 25) & ~0x80) | 0x40));
  
  TxPktNo    = 0;
  TxPktPtr   = (UBYTE*)ethermem;

#ifdef ETHERDEBUG
  ServerDebug("/ether - I am a Western Digital Board");
#endif
  ethertype = BT_WESTERN_DIGITAL;
  return TRUE;
}

PRIVATE bool dlinkp()
{
  int i;
									   
  UBYTE * memptr;
  UBYTE * memptr1;
  unsigned int port;

  /* set up default values for some variable if ness */
  
  if (ethermem  == -1) 
  {
    memptr = (UBYTE*) 0xD0000000;
  }
  else 
  {
    memptr = (UBYTE*) ethermem;
  }

  memptr += 0x2000;
  memptr1 = memptr + (8*1024);		/* top of magic 8K  */	
  port = (memptr[0]&0xFE)*16;

  /* check that port is a valid value */

  unless ((port >= 0x100) && 
          (port <= 0x3E0) &&
	  ((port % 0x20) == 0)) 
  {
#ifdef ETHERDEBUG
  ServerDebug("/ether - port %2x invalid",port);
#endif
    return FALSE;
  }

  /* lets go and see if the magic memory has the board address in it */

  memptr += 17;

  while (memptr < memptr1)
  {
    unless (((unsigned int)(memptr[0]&0xFE)*16) == port) 
    {
#ifdef ETHERDEBUG
      ServerDebug("/ether - ports differ");
#endif
      return FALSE;
    }
    memptr += 17;  
  }

  /* seems likely that this is the board */

  /* ok we have a board lets initialise it */
  
  etherbase = port;
  if (ethermem  == -1) ethermem = 0xD0000000;
  if (etherlevel == -1) etherlevel = 3;
  nicbase = etherbase;
  
  outp(etherbase+0x1F,0xFF);  		/* enable board memory */
  

  /*  zero the address for now  */

  for ( i = 0; i < 8; i++)
  {
    etheraddr[i] = ethermult[i] = 0;
  }
 
  
  TxPktNo    = 32;
  TxPktPtr   = (UBYTE*)(ethermem+0x2000);

#ifdef ETHERDEBUG
  ServerDebug("/ether - I am a DLINK Board");
#endif  

  ethertype = BT_DLINK;

  return TRUE;
}

PRIVATE void dlinkaddr()
{
 int count;
 	 
 /* ok lets try and find that address */
 
 outp(eb+RSAR0,0);
 outp(eb+RSAR1,0);
 
 outp(eb+RBCR0,6);
 outp(eb+RBCR1,0);
 
 /* now go and start things off */
 
 outp(eb+CR,0x0A);  /* remote read */
 
 /* wait for completion */
 
 for(count = 0; count < 2000; count ++)
 {
   if (inp(eb+ISR) & 0x40) break;  	
 }
 
 unless (count <= 2000) 
 {
   ServerDebug("/ether - Cannot Get Board Address");	
 }
 
 for (count = 0; count < 6; count ++)
 {
   etheraddr[count] = (UBYTE)inp(etherbase+0x10);	
 }
 
  outp(eb,0x62);
  
  for (count = 0; count < 6; count++)
  {
    outp(eb+PAR0+count,etheraddr[count]);
  }
  
  outp(eb,0x22);

}
#endif /* OLD_ETHER */

/****************************************************************
*                                                               *
*                          End Of File                          *
*                                                               *
*       Copyright (C) 1990, Perihelion Software Limited         *
*                       All Rights Reserved                     *
*                                                               *
****************************************************************/
