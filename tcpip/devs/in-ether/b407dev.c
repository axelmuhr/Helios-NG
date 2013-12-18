/*
 * File name:	b407dev.c
 *
 *	Helios TCP/IP server - ethernet device driver functions for the 
 *	IMS-B407 ethernet TRAM. 
 *
 * Version:	1.0
 *
 * Author:	Robert Wipfel
 *
 * Copyright INMOS Limited 1991
 *
 * Revision History:
 *
 *	15-FEB-1991	RAW	Created from Nick Garnett's `netdev.c'.
 *	19-FEB-1991	RAW	Added a conditional compilation flag for
 *				the code to support FG_SetInfo requests.
 *
 */

#include "b407dev.h"

/*
 *	Reader() - Eagerly read ethernet packets.
 *
 *	Remove a buffer from the pending buffer queue and read an 
 *	ethernet packet into it. Return buffer if packet read ok.
 */

PRIVATE void Reader( NetDCB *dcb )
{
	WORD		length;	/* Packet length read */
	NetDevReq	*req;	
	int		alreads = 0;

	IOdebug("ETHERNET :Reader process started up");
	forever
	{		
		Wait( &dcb->nreq );	/* Wait for a buffer */
		
		Wait( &dcb->lock );	/* Lock buffer queue */
		req = (NetDevReq *) RemHead( &dcb->readq );
		Signal( &dcb->lock );	/* Release buffer queue */

	again:
		if ( req == NULL ) continue;	/* Null request */
		
		b407_read_packet( dcb->Link, req->Buf, &length );
		alreads++;
		IOdebug("Ethernet: Read %d",alreads);

		if ( length == 0 ) 
		{
#ifdef B407_DEBUG	
			IOdebug( "B407: %s", req->Buf );
#endif	
			goto again;
		}
#ifdef B407_DEBUG
		{	
			int	i;
			char	*buf = (char *)req->Buf;

			IOdebug( "B407 Read: %d [%", length );
			for ( i = 0; i < 24; i++ ) 
				IOdebug( "%x %", buf[i] );
			IOdebug( "]" );
		}
#endif
		req->Actual = length;
		req->DevReq.Result = Err_Null;

		( *req->DevReq.Action )( dcb, req );
	}
}

/*
 *	DevOperate() - Operate device, entry point
 */

word alcmp(char *b1, char *b2, word size){
	word i;
	for (i=0; (i < size); i++) {
		if ((i > 200))
		if (b1[i] != b2[i]) break;
	}
	return(i);
}

PUBLIC void DevOperate( NetDCB *dcb, NetDevReq *req )
{	word alsize2;
	char *buffer;

	NetInfoReq	*ireq = (NetInfoReq *)req;
	NetInfo 	*info = &ireq->NetInfo;
	
	Wait( &dcb->lock );	/* Lock buffer queue */

	switch( req->DevReq.Request )
	{
	case FG_Read:

#ifdef B407_DEBUG
		IOdebug( "B407 Read: %x %d", req->Buf, req->Size ); 
#endif
		AddTail( &dcb->readq, &req->DevReq.Node );
		Signal( &dcb->nreq );	/* Another buffer in queue */

		Signal( &dcb->lock );	/* Release buffer queue */

		return;
		
	case FG_Write:

#ifdef B407_DEBUG
		{
			int	i;
			char	*buf = (char *)req->Buf;

			IOdebug( "B407 Write: %d [%", req->Size );
			for( i = 0; i < 24; i++ ) 
				IOdebug( "%x %", buf[i] );
			IOdebug( "]" );
		}
#endif
		b407_write_packet( dcb->Link, req->Buf, req->Size );
		dcb->alwrites++;
		IOdebug("Ethernet: Written %d",dcb->alwrites);		

		if (dcb->alsize) {	alsize2 = 0;
			if ((req->Size != dcb->alsize) || ((alsize2 = alcmp(req->Buf,dcb->albuffer,dcb->alsize)) != dcb->alsize)) {
				buffer = (char *)req->Buf;
				if (alsize2)	IOdebug("Ethernet: Write buffer contents differ at %d old %d new %d",alsize2,dcb->albuffer[alsize2],buffer[alsize2]);
				else	 	IOdebug("Ethernet: Write differ by size");
				dcb->alsize = req->Size;
				memcpy(dcb->albuffer,req->Buf,dcb->alsize);
			}
		} else {
			dcb->alsize = req->Size;
			memcpy(dcb->albuffer,req->Buf,dcb->alsize);
		}

		req->Actual = req->Size;
		req->DevReq.Result = Err_Null;
		break;
		
	case FG_SetInfo:

#ifdef B407_DEBUG
		{
			int	i;

			IOdebug( "B407 SetInfo: %x %x %x [%",
				info->Mask, info->Mode, info->State );
			for ( i = 0; i < 6; i++ ) 
				IOdebug( "%d %", info->Addr[i] );
			IOdebug( "]" );
		}
#endif
		memcpy(dcb->Addr,info->Addr,6);

		if ( b407_init_firmware( dcb->Link, info->Addr ) )
		{
			/*
	 		 * Start the ethernet reader
			 */
			Fork( 2000, Reader, 4, dcb );

			req->DevReq.Result = Err_Null;
		}
		else
			req->DevReq.Result = EG_UnknownError;
		break;

	case FG_GetInfo:

		info->Mask = 7;
		info->Mode = 0;
		info->State = 0;

		memcpy(info->Addr,dcb->Addr,6);

#ifdef B407_DEBUG
		{
			int	i;
			
			IOdebug( "B407 GetInfo %x %x %x [%", req, ireq, info );
			for( i = 0; i < 6; i++ ) 
				IOdebug( "%d %", info->Addr[i] );
			IOdebug( "]" );
		}	
#endif
		break;

	}
	
	Signal( &dcb->lock );
	
	( *req->DevReq.Action )( dcb, req );
}

/*
 *	DevClose() - Close B407 interface and terminate
 */

PUBLIC WORD DevClose( NetDCB *dcb )
{
	Wait( &dcb->lock );	/* Block the Reader */
	
	b407_close_link( dcb->Link );	/* Release and close the link */
	
	Free( dcb );		/* Free network DCB */
}

/*
 *	DevOpen() - Initialise B407 interface.
 *
 *	Start the B407 by loading and initialising it's firmware. 
 *	Allocate a Network Device Control Block and initialise it. 
 *	Initialise the read buffer queue. Start the eager reader.
 */

PUBLIC NetDCB *DevOpen( Device *dev, NetDevInfo  *info )
{	NetDCB	*dcb;

	/*
	 * Start B407 firmware
	 */
	unless ( b407_open_link( info->Controller ) ) return( NULL );
	unless ( b407_load_firmware( info->Controller ) ) return( NULL );

	/*
	 * Initialise network DCB
	 */
	dcb = Malloc( sizeof(NetDCB) + 8 );
   	if ( dcb == NULL ) return( NULL );
	dcb->alsize = 0;
	dcb->alwrites = 0;

	dcb->dcb.Device = dev;
	dcb->dcb.Operate = DevOperate;
	dcb->dcb.Close = DevClose;

	/*
	 * Initialise read buffer queue 
	 */
	InitSemaphore( &dcb->lock, 1 );	/* Read queue lock */
	InitList( &dcb->readq );	
	InitSemaphore( &dcb->nreq, 0 );	/* Buffer queue empty */ 
	dcb->Link = info->Controller;

	return( dcb );
}


