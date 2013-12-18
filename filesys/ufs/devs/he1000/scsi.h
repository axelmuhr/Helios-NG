/*
 * Header file for scsi
 *
 * $Id: scsi.h,v 1.1 1992/09/16 10:19:30 al Exp $
 * $Log: scsi.h,v $
 * Revision 1.1  1992/09/16  10:19:30  al
 * Initial revision
 * 
 */
#define ERROR -1
#define OK     0
#define ne     !=
#define eq     ==
#define LINKTIMEOUT ( OneSec * 5 )

typedef struct scsi_ptrs
{
	short current_command;
	short current_data;
	short current_status;
	short save_command;
	short save_data;
	short save_status;
}scsi_ptrs;

int 	set_scsi_id( WORD link, BYTE scsi_id );
int	set_target_id ( WORD link, WORD table, BYTE scsi_id );
int 	assert_bus_reset( WORD link);
int 	init_link_initiator( WORD link, bool state );
int 	set_arbitration( WORD link, bool state );
int     request_ptrs( WORD link , WORD table , scsi_ptrs *ptable );
int     alloc_table( WORD link, WORD *table );
int	put_command( WORD link , WORD table , WORD size , BYTE *data);
int	put_data( WORD link, WORD table , WORD size , BYTE *data );
int     get_data( WORD link, WORD table , WORD size , BYTE *data );
int	free_table ( WORD link, WORD table );
int	send_table ( WORD link, WORD table );

int     handle_return ( WORD link );
int  	link_init(WORD);
void 	link_reset(WORD);
void 	reset_subsys(void);
void    resync(WORD link);
