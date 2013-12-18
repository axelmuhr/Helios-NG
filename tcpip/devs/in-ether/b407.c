/*
 * File name:	b407.c
 *
 *	Functions to support the Helios interface to an IMS-B407
 *	ethernet TRAM. This package includes functions to bootstrap
 *	the ethernet TRAM with it's firmware and to then operate
 *	the B407 channel protocol. 
 *
 * Version:	1.0
 *
 * Author:	Robert Wipfel
 *
 * Copyright INMOS Limited 1991
 *
 * Revision History:
 *
 *	31-JAN-1991	RAW	Created.
 *
 */

#include "b407.h"

/*
 *	b407_open_link() - Initialise and open the B407 link.
 *
 *	Set the Helios link into `dumb' mode and allocate the link
 *	for exclusive use. If successful, the link can be used to 
 *	support the B407 channel protocol. Failure of this function 
 *	is terminal and indicates a total failure to establish the 
 *	B407 interface.
 */
 
PUBLIC WORD b407_open_link( WORD link )
{
	LinkInfo	info;
	LinkConf	conf;
	WORD		status;	

	/*
	 * Get current link info
	 */
	if ( ( status = LinkData( link, &info ) ) < 0 ) 
		return( status );
	
	/*
	 * Alter link mode to dumb
	 */
	conf.Flags = info.Flags;
	conf.Mode  = Link_Mode_Dumb;
	conf.State = info.State;
	conf.Id    = info.Id;
	
	/*
	 * Configure link in dumb mode
	 */
	if ( ( status = Configure( conf ) ) < 0 )
		return( status );
	
	/* 
	 * Allocate link for later use 
	 */ 
	if ( ( status = AllocLink( link ) ) < 0 ) 
		return( status );

	return( TRUE );	/* Success */
}

/*
 *	b407_close_link() - Close the B407 link.
 *
 *	Free the link and set it to `Intelligent' mode.
 */

PUBLIC void b407_close_link( WORD link )
{
	LinkInfo	info;
	LinkConf	conf;
	WORD		status;

	/* 
	 * Release link 
	 */
	status = FreeLink( link );

	/*
	 * Get current link info
	 */
	if ( ( status = LinkData( link, &info ) ) >= 0 )
	{ 
		/*
		 * Configure link in Helios mode
		 */
		conf.Flags = info.Flags;
		conf.Mode  = Link_Mode_Intelligent;
		conf.State = info.State;
		conf.Id    = info.Id;
	
		Configure( conf );
	}
}

/* 
 *	b407_load_firmware() - Boot the B407 with it's firmware.
 *
 *	Boot the B407 with it's firmware by loading the contents of 
 *	an INMOS single transputer binary down the link. This then
 *	emulates an ISERVER in order to satisfy the request to supply 
 *	the external memory size, which it does using SP protocol. 
 */

PUBLIC WORD b407_load_firmware( WORD link )
{
	int	n;
	Stream	*fid;
	Object	*context;
	BYTE	data[CODE_BUFFER_SIZE],	/* Code buffer */
		ipacket[I_PACKET_SIZE];	/* ISERVER packet */
	SHORT	itag;			/* ISERVER request */	

	/*
	 * Locate and open firmware file
	 */ 
	context = Locate( NULL, "/" );

	if ( ( fid = Open( context, B407_FIRMWARE, O_RDONLY ) ) == NULL ) 
		return( FALSE );

	Close( context );	/* Forget context */
	
	/*
	 * Load firmware down the link
	 */
	while ( ( n = Read( fid, data, CODE_BUFFER_SIZE, -1 ) ) > 0 )
		if ( LinkOut( n, link, data, B407_LINK_TIMEOUT ) < 0 )
		{
			Close( fid );	
			return( FALSE );
		}

	/*
	 * Close firmware file
	 */
	Close( fid );
	
	/*
	 * Satisfy ISERVER request for IBOARDSIZE
	 */
	if ( LinkIn( sizeof(itag), link, &itag, B407_LINK_TIMEOUT ) < 0 )
		return( FALSE );
		
	if ( LinkIn( itag, link, ipacket, B407_LINK_TIMEOUT ) < 0 )
		return( FALSE );

	/*
	 * Build the ISERVER response packet
	 */
	ipacket[0] = 0x9; ipacket[1] = 0x0;	/* Packet length */
 	ipacket[2] = 0x0;	/* Result byte */
	
	ipacket[3] = 0x6; ipacket[4] = 0x0;	/* ISERVER string */
	ipacket[5] = '#' ; ipacket[6] = '1' ; ipacket[7] = '0' ;
	ipacket[8] = '0' ; ipacket[9] = '0' ; ipacket[10] = '0' ;

	if ( LinkOut( 11, link, ipacket, B407_LINK_TIMEOUT ) < 0 )
		return( FALSE );
		
	return( TRUE );
}

/*
 *	b407_init_firmware() - Initialise the B407 firmware.
 *
 *	Initialise the B407 firmware by specifying a physical ethernet 
 *	address. The firmware initialises the ethernet TRAM hardware and 
 *	returns a status byte.  
 */
 
PUBLIC WORD b407_init_firmware( WORD link, char address[6] )
{
	BYTE firmware_status;

	IOdebug("Ethernet: Request init b407 firmware");

	if ( LinkOut( 6, link, address, B407_LINK_TIMEOUT ) < 0 )
		return( FALSE );

	if ( LinkIn( 1, link, &firmware_status, B407_LINK_TIMEOUT ) < 0 )
		return( FALSE );

	if ( firmware_status != B407_START_OK )
		return( FALSE );
		
	return( TRUE );
}

/*
 *	b407_write_packet() - Send an ethernet packet to the B407
 *
 *	The ethernet packet is passed to the B407 firmware where it
 *	is queued for transmission to the ethernet.
 */
 
PUBLIC void b407_write_packet( WORD link, BYTE *packet, WORD packet_length )
{
/*	IOdebug("Ethernet write req %d bytes",packet_length);	*/
	link_out_data( link, &packet_length, 2 );
		
	link_out_data( link, packet, packet_length );
/*	IOdebug("Ethernet wrote %d bytes",packet_length);	*/
}

/*
 *	b407_read_packet() - Read an ethernet packet from the B407
 *
 *	Read an ethernet packet from the B407 firmware. A packet length 
 *	of zero indicates a debug message rather than an ethernet packet, 
 *	it is copied into the packet buffer and zero terminated.
 */
 
PUBLIC void b407_read_packet( WORD link, BYTE *packet, WORD *packet_length )
{
	SHORT	length;

/*	IOdebug("Ethernet read request"); */

	link_in_data( link, &length, 2 );

	if ( length == 0 )	/* Debug message */
	{
		link_in_data( link, &length, 2 );

		link_in_data( link, packet, length );
	
		packet[length] = '\0';	/* terminate C string */

		*packet_length = 0;	/* No ethernet packet received */
	}
	else
	{
		link_in_data( link, packet, length );

		*packet_length = (WORD) length;	/* ethernet packet length */
	}
/*	IOdebug("Ethernet read %d bytes",*packet_length);   */
}


