/*
 * scsi.c
 *
 * HE1000 support routines
 * Copyright Hunt Engineering 1990
 * TEL: (0278) 784769,  FAX : (0278) 792688.
 *
 * Revision 1.0		Date 29-06-90	M.E
 * Revision 1.1         Date 2-11-90 P.W. get rid of flaky helios linkin
 * Revision 1.2         Date 15-2-91 P.W. fix bugs
 *
 * $Id: scsi.c,v 1.1 1992/09/16 10:19:30 al Exp $
 * $Log: scsi.c,v $
 * Revision 1.1  1992/09/16  10:19:30  al
 * Initial revision
 *
 */
 
#include <helios.h>
#include <link.h>
#include <syslib.h>
#include <asm.h>
#include <codes.h>
#include "scsi.h"

#define LinkVector ((Channel *) MinInt)
#define Out_Link(n) (&LinkVector [(n)])
#define In_Link(n) (&LinkVector [(n + 4)])
#define Link_RX(s,l,b) in_(s,In_Link(l),b)
#define Link_TX(s,l,b) out_(s,Out_Link(l),b)

/*
 * handle_return()
 * 
 * Receives return protocol codes from HE1000 hardware.
 * Supported return codes include:
 * Acknowledge      :- protocol code 80h returns 0 
 * Error            :- protocol code A1h returns < 0
 * Command Complete :- protocol code A2h returns scsi status
 * 
 */

int     handle_return ( WORD link )
{
	BYTE buffer[2];
	
        Link_RX(2,link,buffer);
	
	/* check for acknowledge */
	if ( buffer[1] eq 0x80 ) 
		return(OK);
	
	/* check for command complete */
	if ( buffer[1] eq 0xA2 ) 
	{
		/* get the scsi status of the command */
                Link_RX(1,link,buffer);
		return((int)buffer[0]);
	}
		
	/* check for error return code */
	if ( buffer[1] eq 0xA1 ) 
	{
		Link_RX(1,link,buffer);
		return(ERROR);
	}
	
	return(ERROR);
}
	

	 
/*
 * set_scsi_id()
 *
 * Sets the scsi ID of the scsi module. The ID is a value between
 * 0 and 7. It must not clash with other scsi module i.d's
 *
 * returns < 0 on error
 */
 
int 	set_scsi_id( WORD link, BYTE scsi_id )
{
	BYTE buffer[3];
	
	if ( scsi_id > 7 ) return(ERROR);
	buffer[0] = 0xC0;
	buffer[1] = 0xA3;
	buffer[2] = scsi_id;
	Link_TX(3,link,buffer);

	return(handle_return(link));
}

/*
 * sets the target scsi of a transaction table
 *
 */

int set_target_id(WORD link, WORD table, BYTE scsi_id)
{
	BYTE buffer[3];
	
	buffer[0] = 0xC0 | table;
	buffer[1] = 0xB0;
	
	if ( scsi_id > 7 ) return(ERROR);
	
	buffer[2] = scsi_id;
	Link_TX(3,link,buffer);

	return(handle_return(link));
}

/*
 * Reqests module to perform a bus reset.
 *
 */
		
int 	assert_bus_reset( WORD link)
{
	BYTE buffer[2];

	buffer[0] = 0xC0;
	buffer[1] = 0x84;	
	Link_TX(2,link,buffer);

	return(handle_return(link));
}

/*
 * sets a link up to accept/reject initiator request protocols
 *
 */
 
int 	init_link_initiator ( WORD link, bool state )
{
	BYTE buffer[3];
	INT result;
	
	buffer[0] = 0xC0;
	buffer[1] = 0xA5;
	buffer[2] = 0x01;
	
	if (!state) buffer[2] = 0x00;

	/* use LinkOut here as if there's no module then we need a timeout */
	result = LinkOut(3,link,buffer,LINKTIMEOUT);
        if (result <0)
	{
		return(ERROR);
	}

	return(handle_return(link));
}

/*
 * enables/disables scsi bus arbitration
 *
 */
int 	set_arbitration( WORD link, bool state )
{
	BYTE buffer[3];
	
	buffer[0] = 0xC0;
	buffer[1] = 0xA6;
	buffer[2] = 0x01;
	
	if (!state) buffer[2] = 0x00;
	
	Link_TX(3,link,buffer);

	return(handle_return(link));
}

int     request_ptrs( WORD link, WORD table , scsi_ptrs *ptable )
{
	BYTE  buffer[2];
	BYTE  data[2];
	SHORT size;
	
        buffer[0] = 0xC0 | table;
	buffer[1] = 0x87;

	Link_TX(2,link,buffer);

	Link_RX(2,link,data);
	
	if ((buffer[0] ne data[0]) || (data[1] ne 0xC7)) return(ERROR);

	Link_RX(2,link,&size);	
	if  (size ne 0x0C ) return(ERROR);	

	Link_RX(size,link,ptable);
	
	return(OK);	

}

int     alloc_table( WORD link, WORD *table )
{
	BYTE  buffer[2];
	BYTE  data[2];
	BYTE  btable;
	
	buffer[0] = 0xC0;
	buffer[1] = 0x88;

	Link_TX(2,link,buffer);	

	Link_RX(2,link,data);
	
	if ((data[0] ne 0xC0) || (data[1] ne 0xA8)) return(ERROR);

	Link_RX(1,link,&btable);	
	*table = btable;
	return(OK);
}

int	put_command( WORD link, WORD table , WORD size , BYTE *data )
{
	BYTE  buffer[2];
	short table_size;
	
	buffer[0] = 0xC0 | table;
	buffer[1] = 0xC9;

	Link_TX(2,link,buffer);

	table_size = size;
	Link_TX(2,link,&table_size);

	Link_TX(size,link,data);

	return(handle_return(link));
}


int	put_data( WORD link, WORD table , WORD size , BYTE *data )
{
	BYTE  buffer[2];
	short table_size;
	
	buffer[0] = 0xC0 | table;
	buffer[1] = 0xCA;

	Link_TX(2,link,buffer);	

	table_size = size;
	Link_TX(2,link,&table_size);

	Link_TX(size,link,data);

	return(handle_return(link));
}

int     get_data( WORD link, WORD table , WORD size , BYTE *data )
{
	BYTE  buffer[6];
	WORD  actual;
	
	if( size eq 0 ) return(OK);
	
	buffer[0] = 0xC0 | table;
	buffer[1] = 0xCB;
	buffer[2] = 0x02;
	buffer[3] = 0x00;
	buffer[4] = 0x00;
	buffer[5] = 0x00;

	Link_TX(6,link,buffer);	

	Link_RX(4,link,buffer);
	if ( buffer[1] ne 0xCB )
	{
		return(ERROR);
         }
	actual = buffer[2] + (buffer[3]*256);
     	Link_RX(actual,link,data);
	if (actual ne size) /* if we check here the module is still alive !*/
	{
		return(ERROR);
	}
	return(actual);
        }

int	free_table ( WORD link, WORD table )
{
	BYTE buffer[2];
	
	buffer[0] = 0xC0 | table;
	buffer[1] = 0x8C;
	Link_TX(2,link,buffer);

	return(handle_return(link));
}

int	send_table ( WORD link, WORD table )
{
	BYTE buffer[4];
	
	buffer[0] = 0xC0 | table;
	buffer[1] = 0x8E;
	Link_TX(2,link,buffer);

	Link_RX(3,link,buffer);
	
	/* check for command complete */
	if ( buffer[1] eq 0xA2 ) 
	{

		return((int)buffer[2]);
	}
		
	/* check for error return code */
	if ( buffer[1] eq 0xA1 )
		return(ERROR);
	
	return(ERROR);

}

int link_init(WORD link)
{
	word		status;
	LinkInfo	linfo;
	LinkConf	lconf;
	
	/* Get the current link state */
	if ((status = LinkData(link, &linfo)) < 0)
		return(status);
		
	/* Modify link to dumb mode */
	lconf.Flags = linfo.Flags;
	lconf.Mode  = Link_Mode_Dumb;
	lconf.State = linfo.State;
	lconf.Id    = linfo.Id;
	
	/* Now configure the link */
	if ((status = Configure(lconf)) < 0)
		return(status);
		
	/* Allocate the link for use */
	if ((status = AllocLink(link)) < 0)
		return(status);
		
	
}


void link_reset(WORD link)
{
	word		status;
	LinkInfo	linfo;
	LinkConf	lconf;
	
	/* Free the link */
	status = FreeLink(link);
	
	/* Get the current link state */
	if ((status = LinkData(link, &linfo)) >= 0)
	{
		/* Modify link to dumb mode */
		lconf.Flags = linfo.Flags;
		lconf.Mode  = Link_Mode_Intelligent;
		lconf.State = linfo.State;
		lconf.Id    = linfo.Id;
	        status = Configure(lconf);
	}		
}


#define		SUBSYSTEM_RESET		0x00000000L
#define		SUBSYSTEM_ANALYSE 	0x00000004L

void reset_subsys(void)
{
	
	uword *reg;
	
   	reg = (uword *)SUBSYSTEM_ANALYSE; *reg = 0 ;
   	    Delay(10000);
   	reg = (uword *)SUBSYSTEM_ANALYSE; *reg = 1 ;
   	    Delay(10000);
   	reg = (uword *)SUBSYSTEM_RESET; *reg = 1 ;
   	    Delay(10000);
   	reg = (uword *)SUBSYSTEM_RESET; *reg = 0 ;
	    Delay(10000);
   	reg = (uword *)SUBSYSTEM_ANALYSE; *reg = 0 ;
	    Delay(10000);
} 
