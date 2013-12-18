/*
-- Copyright INMOS Limited 1991
--
-- **************************************************************************
--
-- Group:	Iq software
--
-- Product:	IMS-F006 Ethernet TRAM support software
--
-- File:	b431io.c
--
-- Version:	1.0
--
-- **************************************************************************
--
-- B431 device driver interface library. This file contains procedures for
-- interacting with an IMS-B431 or IMS-B407 ethernet TRAM when executing the
-- ethernet TRAM device driver (b431drvr).
--
-- **************************************************************************
--
-- History:	
--
-- (24-SEP-91) Bob Wipfel, Created file.
-- (19-NOV-91) Bob Wipfel, Variant for Helios.
--
-- **************************************************************************
*/

#include <string.h>
#include <chanio.h>

#include "b431io.h"
#include "b431drvr.h"


int B431_Init_Normal(
	long int	link,
	const char	physical_address[PHYSICAL_ADDRESS_SIZE],
	const char	logical_address_filter[LOGICAL_ADDRESS_FILTER_SIZE],
	const long	mode_flags )
{
	char		message[ PHYSICAL_ADDRESS_SIZE + 
				LOGICAL_ADDRESS_FILTER_SIZE + sizeof(long) ];
	MESSAGE_HDR	header;
	char		*m = message;

	INIT_MESSAGE_HDR( header, sizeof(message), FN_INIT_NORMAL, 0 );

	memcpy( m, physical_address, PHYSICAL_ADDRESS_SIZE );
	m += PHYSICAL_ADDRESS_SIZE;
	memcpy( m, logical_address_filter, LOGICAL_ADDRESS_FILTER_SIZE );
	m += LOGICAL_ADDRESS_FILTER_SIZE;
	memcpy( m, &mode_flags, sizeof(long) );

	link_out_struct( link, header );
	link_out_data( link, message, header.length );

	link_in_struct( link, header );

	return( header.result );
}


int B431_Start_Ether( long int link )
{
	MESSAGE_HDR header;

	INIT_MESSAGE_HDR( header, 0, FN_START_ETHER, 0 );

	link_out_struct( link, header );
	link_in_struct( link, header );

	return( header.result );
}


void B431_Tx_Packet1(
	long int	link,
	const char	*ethernet_packet,
	const int	ethernet_packet_length )
{
	if ( ethernet_packet_length != 0 )
	{
		MESSAGE_HDR	header;

		INIT_MESSAGE_HDR( header, ethernet_packet_length,
			FN_TX_PACKET1, 0 );

		link_out_struct( link, header );
		link_out_data( link, ethernet_packet, header.length );
	}
}		


void B431_Ether_Stats( long int link )
{
	MESSAGE_HDR header;

	INIT_MESSAGE_HDR( header, 0, FN_ETHER_STATS, 0 );

	link_out_struct( link, header );
}


void B431_Terminate( long int link )
{
	MESSAGE_HDR header;

	INIT_MESSAGE_HDR( header, 0, FN_TERMINATE, 0 );

	link_out_struct( link, header );
}


int B431_Waitfor_Event( 
	long int	link,
	ETHER_STATS	*ethernet_stats,
	char		*ethernet_packet,
	int		*ethernet_packet_length,
	int		*error_code,
	char		*failed_packet_data )
{
	MESSAGE_HDR	header;

	link_in_struct( link, header );

	switch ( header.fn_code )
	{
		case B431_TERMINATE:
		case B431_STOP_ETHER:
			break;

		case B431_ETHER_STATS:
			link_in_data( link, ethernet_stats, header.length );
			break;

		case B431_ERROR_REPORT:
			*error_code = header.result;
			if ( *error_code == ERROR_TX_PACKET_FAILED )
				link_in_data( link, failed_packet_data, header.length );
			break;

		case B431_RX_PACKET:
			link_in_data( link, ethernet_packet, header.length );
			*ethernet_packet_length = header.length;
			break;
	}

	return( header.fn_code );
}
