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



PRIVATE void tidyboard(void);		
PRIVATE bool SendTxPkt(BYTE*, USHORT) ;
PRIVATE SHORT GetRxPkt (UBYTE*, WORD) ;


PRIVATE void tidyboard()
{
#ifdef ETHER_STATS
  show_ether_statistics (pkt_int_no, drvr_handle [0]) ;
#endif /* ETHER_STATS */
  release_all (pkt_int_no, drvr_handle) ;
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
#ifdef ETHER_DEBUG
  notify ("tx: pkt len = %d", len) ;
#endif /* ETHER_DEBUG */
  if (send_pkt (pkt_int_no, buff, len) < 0)
    return (fail ("Failed to send packet")) ;
  return (TRUE) ;
}


/* GetRxPkt - gets a packet from the ring buffer 
   must only be called if there is one there 
   check with RxAvail */

PRIVATE SHORT GetRxPkt(buff,maxsize)
UBYTE* buff;
WORD maxsize;
{ 

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

  Server_errno = 0 ;				/* all ok */
  return len ;					/* and home */
}


/****************************************************************
*                                                               *
*                          End Of File                          *
*                                                               *
*       Copyright (C) 1990, Perihelion Software Limited         *
*                       All Rights Reserved                     *
*                                                               *
****************************************************************/
