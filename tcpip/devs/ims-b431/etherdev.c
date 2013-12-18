/*
-- Copyright INMOS Limited 1991
--
-- **************************************************************************
--
-- Group:	Iq software
--
-- Product:	
--
-- File:	etherdev.c
--
-- Version:	1.0
--
-- **************************************************************************
--
--
-- **************************************************************************
--
-- History:	
--
-- (17-SEP-91) Bob Wipfel, Created file.
--
-- **************************************************************************
--
*/

#include <device.h>	/* Generic device */
#include <syslib.h>	/* Helios system library */
#include <fcntl.h>	/* Posix file handling */
#include <nonansi.h>	/* Helios ANSI extensions */
#include <queue.h>	/* Queue library */
#include <sem.h>	/* Semaphore library */
#include <codes.h>	/* Helios error codes */
#include <string.h>	/* ANSI string library */
#include <link.h>	/* Link handling */

#include "b431io.h"

/* Network device control block */

typedef struct
{
	DCB		dcb;	
	Semaphore	lock;
	List		readq;
	Semaphore	nreq;
	Semaphore	outlink;
	long		Link;
} NetDCB;

#ifdef STATS

PRIVATE void IOdebug_ethernet_stats( ETHER_STATS ethernet_stats )
{
	IOdebug("tx_packets = %d", ethernet_stats.tx_packets);
	IOdebug("rx_packets = %d", ethernet_stats.rx_packets);
	IOdebug("framing_errors = %d", ethernet_stats.framing_errors);
	IOdebug("crc_errors = %d", ethernet_stats.crc_errors);
	IOdebug("packets_dropped = %d", ethernet_stats.packets_dropped);
	IOdebug("packets_missed = %d", ethernet_stats.packets_missed);
	IOdebug("deferred_transmits = %d", ethernet_stats.deferred_transmits);
	IOdebug("late_collisions = %d", ethernet_stats.late_collisions);
	IOdebug("carrier_lost = %d", ethernet_stats.carrier_lost);
	IOdebug("failed_retries = %d", ethernet_stats.no_more_retries);
	IOdebug("single_retries = %d", ethernet_stats.single_retries);
	IOdebug("many_retries = %d", ethernet_stats.multiple_retries);
	IOdebug("average_tdr_value = %d", ethernet_stats.average_tdr_value);
}


PRIVATE void Stats_Update( NetDCB *dcb )
{
	IOdebug("ETHER: Stats update running");

	forever
 	{
 		Delay( 20 * OneSec );
		Wait( &dcb->outlink );
		B431_Ether_Stats( dcb->Link );
		Signal( &dcb->outlink );
	}
}

#endif /* STATS */

PRIVATE void Packet_Reader( NetDCB *dcb )
{
	NetDevReq	*req;
	ETHER_STATS	ethernet_stats;
	char		failed_packet[FAILED_PACKET_LENGTH];
	int		error_code, event_code, packet_length;

#ifdef DEBUG
	IOdebug("ETHER: Packet reader running");
#endif /* DEBUG */

	forever
	{		
		Wait( &dcb->nreq );	/* Wait for a buffer */
		
		Wait( &dcb->lock );	/* Lock buffer queue */
		req = (NetDevReq *) RemHead( &dcb->readq );
		Signal( &dcb->lock );	/* Release buffer queue */
		
		if ( req == NULL ) continue;	/* Null request */

		event_code = B431_Waitfor_Event( dcb->Link, &ethernet_stats,
			req->Buf, &packet_length, &error_code, failed_packet );

		while ( event_code != B431_RX_PACKET )
		{
			switch ( event_code )
			{
#ifdef STATS
				case B431_ETHER_STATS:
					IOdebug_ethernet_stats( ethernet_stats );
					break;
#endif /* STATS */
				case B431_ERROR_REPORT:
					IOdebug("ETHER: B431 error %d", error_code); 
					break;
				
				case B431_STOP_ETHER:
					IOdebug("ETHER: B431 hardware has stopped");
					break;
					
				case B431_TERMINATE:
					IOdebug("ETHER: B431 device driver terminated");
					return;
					
				default:
					IOdebug("ETHER: Serious, unknown B431 event type");
					break;
			}
				
			event_code = B431_Waitfor_Event( dcb->Link, &ethernet_stats,
				req->Buf, &packet_length, &error_code, failed_packet );
		}

		req->Actual = packet_length;
		req->DevReq.Result = Err_Null;
#ifdef DEBUG
		{	
			int	i;
			char	*buf = (char *) req->Buf;

			IOdebug( "ETHER: rx (%d) [ %", packet_length );
			for ( i = 0; i < 24; i++ ) 
				IOdebug( "%x %", buf[i] );
			IOdebug( " ]" );
		}
#endif /* DEBUG */
		( *req->DevReq.Action )( dcb, req );
	}
}


PUBLIC void DevOperate( NetDCB *dcb, NetDevReq *req )
{
	NetInfoReq	*ireq = (NetInfoReq *)req;
	NetInfo 	*info = &ireq->NetInfo;
	char		logical_address_filter[LOGICAL_ADDRESS_FILTER_SIZE];
	
	Wait( &dcb->lock );	/* Lock buffer queue */

	switch( req->DevReq.Request )
	{
		case FG_Read:
			AddTail( &dcb->readq, &req->DevReq.Node );
			Signal( &dcb->nreq );	/* Add buffer to queue */
			Signal( &dcb->lock );	/* Release buffer queue */
			return;
		
		case FG_Write:
			Wait( &dcb->outlink );
			B431_Tx_Packet1( dcb->Link, req->Buf, req->Size );
			Signal( &dcb->outlink );

			req->Actual = req->Size;
			req->DevReq.Result = Err_Null;
			break;
		
		case FG_SetInfo:
#ifdef DEBUG
		{	
			int	i;
			char	*buf = (char *) info->Addr;

			IOdebug( "ETHER: source address [ %" );
			for ( i = 0; i < 6; i++ ) 
				IOdebug( "%x %", buf[i] );
			IOdebug( " ]" );
		}
#endif /* DEBUG */
			memset( logical_address_filter, 0,
				LOGICAL_ADDRESS_FILTER_SIZE );

			if ( B431_Init_Normal( dcb->Link, info->Addr,
				logical_address_filter, MEMORY_CHECK ) == INIT_SUCCESS )
			{
#ifdef DEBUG
				IOdebug("ETHER: FG_SetInfo, B431_Init_Normal() ok");	
#endif /* DEBUG */

#ifdef STATS
				Fork( 2000, Stats_Update, 4, dcb );
#endif /* STATS */
				Fork( 2000, Packet_Reader, 4, dcb );

				B431_Start_Ether( dcb->Link );
				req->DevReq.Result = Err_Null;
			}
			else
			{
				IOdebug("ETHER: FG_SetInfo, B431_Init_Normal() failed");
				req->DevReq.Result = EG_UnknownError;
			}

			break;

		case FG_GetInfo:
			info->Mask = 0;
			info->Mode = 0;
			info->State = 0;

			req->DevReq.Result = 0;
			break;
	}
	
	Signal( &dcb->lock );
	
	( *req->DevReq.Action )( dcb, req );
}

 
PUBLIC WORD DevClose( NetDCB *dcb )
{
	WORD	status;

	Wait( &dcb->outlink );
	B431_Terminate( dcb->Link );	/* Terminate B431 driver */

	status = FreeLink( dcb->Link );	/* Free the link */

	Free( dcb );		/* Free network DCB */
}


PUBLIC NetDCB *DevOpen( Device *dev, NetDevInfo *info )
{
	NetDCB		*dcb;
	LinkInfo	link_info;
	LinkConf	link_conf;
	WORD		status;
	int		n;
	Stream		*fid;
	Object		*context;
	BYTE		data[512];

#ifdef DEBUG
	IOdebug( "ETHER: DevOpen(), controller link %d", 
		info->Controller );
#endif /* DEBUG */

	if ( ( status = LinkData(info->Controller, &link_info) ) < 0 ) 
		return( NULL );

	link_conf.Flags = link_info.Flags;
	link_conf.Mode  = Link_Mode_Dumb;
	link_conf.State = link_info.State;
	link_conf.Id    = link_info.Id;

	if ( ( status = Configure(link_conf) ) < 0 )
		return( NULL );
	
	if ( ( status = AllocLink(info->Controller) ) < 0 ) 
		return( NULL );

	context = Locate( NULL, "/" );
	if ( ( fid = Open( context,
			"/helios/lib/imsb431.b2h", O_RDONLY ) ) == NULL ) 
	{
		IOdebug("ETHER: DevOpen() failed to open /helios/lib/imsb431.b2h");
		return( NULL );
	}
	Close( context );
	
	while ( ( n = Read( fid, data, 512, -1 ) ) > 0 )
		if ( LinkOut( n, info->Controller, data, 32 ) < 0 )
		{
			Close( fid );	
			IOdebug("ETHER: DevOpen() failed to download /helios/lib/imsb431.b2h");
			return( NULL );
		}
	Close( fid );

	/* Initialise network DCB */

	dcb = Malloc( sizeof(NetDCB) );
   	if ( dcb == NULL ) return( NULL );

	dcb->dcb.Device = dev;
	dcb->dcb.Operate = DevOperate;
	dcb->dcb.Close = DevClose;

	/* Initialise read buffer queue */
	
	InitSemaphore( &dcb->lock, 1 );		/* Read queue lock */
	InitList( &dcb->readq );	
	InitSemaphore( &dcb->nreq, 0 );		/* Buffer queue empty */ 
	dcb->Link = info->Controller;
	InitSemaphore( &dcb->outlink, 1 );	/* outlink lock */

	return( dcb );
}
