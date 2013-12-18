/*
* huntcom.c
*
*	Driver code for HE1000 and Helios File Server
*
*	Copyright HUNT ENGINEERING 1990
*	TEL (0278) 784769, FAX ( 0278) 792688
*
* REV 1.1	 Date 2-11-90	  P.W.
* REV 1.2	 Date 15-2-91	  P.W.	 addition of quick read for verify
*
*/

#include <helios.h>
#include <stdio.h>
#include <stdlib.h>

#include "huntcom.h"
#include "scsi.h"


/*
 * HE1000_init()
 *
 * Set controlling link to dumb, allocatea table for filesystem use,
 * set initaiator and target ids.
 *
 * Return allocated table number of result less than 0 on error.
 */

int HE1000_init( WORD link, WORD target_id , WORD tram_id )
{
	int	state;
	WORD	result;

	/* configure link to dumb mode */
	if((state = link_init(link)) < 0)
		return(ERROR);

	/* set the controlling link to initiator */
	if((state = init_link_initiator( link , TRUE )) < 0)
		return(ERROR);

	/* set the scsi id of this initiator */
	if((state = set_scsi_id( link , tram_id )) < 0)
		return(ERROR);

	/* get a table for file system use */
	if((state = alloc_table( link , &result )) < 0)
		return(ERROR);

	/* set the target id of the table */
	if((state = set_target_id( link , result , target_id )) < 0)
		return(ERROR);

	return(result);
}

/*
 * HE1000_release()
 *
 * free the allocated table back to system use, free the controlling link
 *
 * Return 0 on sucess or less than 0 on error
 */
int HE1000_release(WORD link, WORD table)
{

	/* free the table */
	free_table(link,table);

	/* set link to unused */
	init_link_initiator( link , FALSE );

	/* release the link */
	link_reset(link);

	return(OK);
}

int scsi_read(
	WORD link,
	WORD table,
	WORD target_id,
	WORD lun,
	WORD position,
	WORD size,
	WORD sector_size,
	BYTE *data,
	WORD *command_status)
{
	BYTE scsi_command[10];
	int  state;
	WORD blocks = size / sector_size;

	target_id = target_id;	/* assume its already set */
	scsi_command[0] = 0x28; /* read 10 byte command */
	scsi_command[1] = lun << 5;
	scsi_command[2] = getbyte(position,3);
	scsi_command[3] = getbyte(position,2);
	scsi_command[4] = getbyte(position,1);
	scsi_command[5] = getbyte(position,0);
	scsi_command[6] = 0;
	scsi_command[7] = getbyte(blocks,1);
	scsi_command[8] = getbyte(blocks,0);
	scsi_command[9] = 0;

	/* put the command into a buffer */
	if((state = put_command( link , table , 10 , scsi_command )) < 0)
		return(ERROR);

	*command_status = send_table( link , table);

	if( *command_status eq 0 )	/* get the data from module */
	{
		state = get_data( link , table , size , data );
		return(state);	/* error or return size */
	}
	return(ERROR);
}

	
int scsi_read_quick(
	WORD link,
	WORD table,
	WORD target_id,
	WORD lun,
	WORD position,
	WORD *command_status)
{
	BYTE scsi_command[10];
	int  state;

	target_id = target_id;	/* assume its already set */
	scsi_command[0] = 0x28; /* read 10 byte command */
	scsi_command[1] = lun << 5;
	scsi_command[2] = getbyte(position,3);
	scsi_command[3] = getbyte(position,2);
	scsi_command[4] = getbyte(position,1);
	scsi_command[5] = getbyte(position,0);
	scsi_command[6] = 0;
	scsi_command[7] = 0;
	scsi_command[8] = 0x1;
	scsi_command[9] = 0;

	/* put the command into a buffer */
	if((state = put_command( link , table , 10 , scsi_command )) < 0)
		return(ERROR);

	*command_status = send_table( link , table);

	if( *command_status eq 0 )	/* get the data from module */
		return(0x0);  /* good  */
	return(ERROR);
}

int scsi_write(
	WORD link,
	WORD table,
	WORD target_id,
	WORD lun,
	WORD position,
	WORD size,
	WORD sector_size,
	BYTE *data,
	WORD *command_status)
{
	BYTE scsi_command[10];
	int  state;
	WORD blocks = size / sector_size;

	target_id = target_id;		/* assume its already set */
	scsi_command[0] = 0x2A; 	/* write 10 byte command */
	scsi_command[1] = lun << 5;
	scsi_command[2] = getbyte(position,3);
	scsi_command[3] = getbyte(position,2);
	scsi_command[4] = getbyte(position,1);
	scsi_command[5] = getbyte(position,0);
	scsi_command[6] = 0;
	scsi_command[7] = getbyte(blocks,1);
	scsi_command[8] = getbyte(blocks,0);
	scsi_command[9] = 0;

	/* put the command into a buffer */
	if((state = put_command( link , table , 10 , scsi_command )) < 0)
		return(state);

	if((state = put_data( link , table , size , data )) < 0)
		return(state);

	/* send the command on the into bus */

	*command_status = send_table( link , table);
	if (*command_status eq 0)
		return(OK);
	else
		return(ERROR);
}

int scsi_write_quick(
	WORD link,
	WORD table,
	WORD target_id,
	WORD lun,
	WORD position,
	WORD *command_status)
{
	BYTE scsi_command[6];
	int  state;

	target_id = target_id;		/* assume its already set */
	scsi_command[0] = 0x2A; 	/* write 10 byte command */
	scsi_command[1] = lun << 5;
	scsi_command[2] = getbyte(position,3);
	scsi_command[3] = getbyte(position,2);
	scsi_command[4] = getbyte(position,1);
	scsi_command[5] = getbyte(position,0);

	/* put the command into a buffer */
	if((state = put_command( link , table , 6 , scsi_command )) < 0)
		return(state);

	*command_status = send_table( link , table);
	if (*command_status eq 0)
		return(OK);
	else
		return(ERROR);
}

int scsi_format(
	WORD link,
	WORD table,
	WORD target_id,
	WORD lun,
	WORD interleave,
	WORD *command_status)
{
	BYTE scsi_command[6];
	int  state;

	target_id = target_id;		/* assume its already set */
	scsi_command[0] = 0x04; 	/* write 6 byte command */
	scsi_command[1] = (lun << 5) || 0x18;
	scsi_command[2] = 0x00;
	scsi_command[3] = getbyte(interleave,1);
	scsi_command[4] = getbyte(interleave,0);
	scsi_command[5] = 0;

	/* put the command into a buffer */
	if((state = put_command( link , table , 6 , scsi_command )) < 0)
		return(state);

	/* send the command on the into bus */
	*command_status = send_table( link , table);

	return(OK);
}

int scsi_start_stop(
	WORD link,
	WORD table,
	WORD target_id,
	WORD lun,
	WORD start_stop,
	WORD *command_status)
{
	BYTE scsi_command[6];
	int  state;

	target_id = target_id;		/* assume its already set */
	scsi_command[0] = 0x1B; 	/* write 6 byte command */
	scsi_command[1] = lun << 5;
	scsi_command[2] = 0x00;
	scsi_command[3] = 0x00;
	scsi_command[4] = getbyte(start_stop,0);
	scsi_command[5] = 0;

	/* put the command into a buffer */
	if((state = put_command( link , table , 6 , scsi_command )) < 0)
		return(state);

	/* send the command on the into bus */
	*command_status = send_table( link , table);

	return(OK);
}

int scsi_read_capacity(
	WORD link,
	WORD table,
	WORD target_id,
	WORD lun,
	WORD length,
	BYTE *data,
	WORD *command_status)
{
	BYTE scsi_command[10];
	int  state;

	target_id = target_id;		/* assume its already set */
	scsi_command[0] = 0x25; 	/* read capacity 10 byte command */
	scsi_command[1] = lun << 5;
	scsi_command[2] = 0;
	scsi_command[3] = 0;
	scsi_command[4] = 0;
	scsi_command[5] = 0;
	scsi_command[6] = 0;
	scsi_command[7] = 0;
	scsi_command[8] = 0;
	scsi_command[9] = 0;

	/* put the command into a buffer */
	if((state = put_command( link , table , 10 , scsi_command )) < 0)
		return(state);

	/* send the command on the into bus */
	*command_status = send_table( link , table);
	if( *command_status eq 0 )	/* get the data from module */
	{
		if((state = get_data( link , table , length , data )) < 0)
			return(state);
		else
			return(state);	/* return data size back */
	}
	else
	  return(ERROR);		/* scsi return bad status back */
}

int scsi_request_sense(
	WORD link,
	WORD table,
	WORD target_id,
	WORD lun,
	WORD length,
	BYTE *data,
	WORD *command_status)
{
	BYTE scsi_command[6];
	int  state;

	target_id = target_id;		/* assume its already set */
	scsi_command[0] = 0x03; 	/* read capacity 10 byte command */
	scsi_command[1] = lun << 5;
	scsi_command[2] = 0;
	scsi_command[3] = 0;
	scsi_command[4] = length;
	scsi_command[5] = 0;

	/* put the command into a buffer */
	if((state = put_command( link , table , 6 , scsi_command )) < 0)
		return(state);

	/* send the command on the into bus */
	*command_status = send_table( link , table);
	if( *command_status eq 0 )	/* get the data from module */
	{
		state = get_data( link , table , length , data );
		return(state);
	}
	else
	  return(ERROR);		/* scsi return bad status back */
	if ( *command_status < 0 )
	  return(ERROR);		/* scsi command failed */
	else
	  return(OK);
}

int scsi_test_unit_ready(
	WORD link,
	WORD table,
	WORD target_id,
	WORD lun,
	WORD *command_status)
{
	BYTE scsi_command[6];
	int  state;

	target_id = target_id;		/* assume its already set */
	scsi_command[0] = 0x00; 	/* test unit ready 6 byte command */
	scsi_command[1] = lun << 5;
	scsi_command[2] = 0;
	scsi_command[3] = 0;
	scsi_command[4] = 0;
	scsi_command[5] = 0;

	/* put the command into a buffer */
	if((state = put_command( link , table , 6 , scsi_command )) < 0)
		return(state);

	/* send the command on the into bus */
	*command_status = send_table( link , table);
	if ( *command_status < 0 )
	  return(ERROR);		/* scsi command failed */
	else
	  return(OK);
}

int scsi_mode_select(
	WORD link,
	WORD table,
	WORD target_id,
	WORD lun,
	WORD no_blocks,
	WORD block_length,
	WORD *command_status)
{
	BYTE scsi_command[6];
	BYTE scsi_data[12];
	int  state;

	target_id = target_id;		/* assume its already set */
	scsi_command[0] = 0x15; 	/* test unit ready 6 byte command */
	scsi_command[1] = lun << 5;
	scsi_command[2] = 0;
	scsi_command[3] = 0;
	scsi_command[4] = 12;		/* parameter list is 12 bytes */
	scsi_command[5] = 0;

	scsi_data[0]	= 0;
	scsi_data[1]	= 0;		/* medium type - default */
	scsi_data[2]	= 0;
	scsi_data[3]	= 8;		/* 8 bytes to follow */

	scsi_data[4]	= 0;		/* default density */
	scsi_data[5]	= getbyte(no_blocks,2);
	scsi_data[6]	= getbyte(no_blocks,1);
	scsi_data[7]	= getbyte(no_blocks,0);
	scsi_data[8]	= 0;		/* reserved */
	scsi_data[9]	= getbyte(block_length,2);
	scsi_data[10]	= getbyte(block_length,1);
	scsi_data[11]	= getbyte(block_length,0);


	/* put the command into a buffer */
	if((state = put_command( link , table , 6 , scsi_command )) < 0)
		return(state);

	if((state = put_data( link , table , 12, scsi_data )) < 0)
		return(state);

	/* send the command on the into bus */
	*command_status = send_table( link , table);
	if ( *command_status < 0 )
	  return(ERROR);		/* scsi command failed */
	else
	  return(OK);
}


BYTE getbyte(WORD i, BYTE b)
{
	return((i >> (8 * b)) & 0xFF);
}

void scsi_reset( WORD link )
{
	assert_bus_reset( link );
}

int scsi_reassign_block(
	WORD link,
	WORD table,
	WORD target_id,
	WORD lun,
	WORD block,
	WORD *command_status)
{
	BYTE scsi_command[26];
	int  state;

	target_id = target_id;		/* assume its already set */
	scsi_command[0] = 0x07; 	/* write 10 byte command */
	scsi_command[1] = lun << 5;
	scsi_command[2] = 0;
	scsi_command[3] = 0;
	scsi_command[4] = 0;
	scsi_command[5] = 0;

	scsi_command[6] = 0;
	scsi_command[7] = 0;
	scsi_command[8] = 0;
	scsi_command[9] = 4;

	scsi_command[10] = getbyte(block,3);
	scsi_command[11] = getbyte(block,2);
	scsi_command[12] = getbyte(block,1);
	scsi_command[13] = getbyte(block,0);


	/* put the command into a buffer */
	if((state = put_command( link , table , 6 , scsi_command )) < 0)
	{
#ifdef DEBUGGING
		IOdebug("reassign block: put command failed, link = %d table = %d",link,table);
#endif
			return(state);
	}

	if((state = put_data( link , table , 8 , &scsi_command[6] )) < 0)
	{
#ifdef DEBUGGING
		IOdebug("reassign block: put data failed, link = %d table = %d size = %d",link,table,8);
#endif
		return(state);
	}

	/* send the command on the into bus */

	*command_status = send_table( link , table);
	if (*command_status eq 0)
		return(OK);
	else
	{
#ifdef DEBUGGING
		IOdebug("reassign block: send table failed, link = %d table = %d command_status = %d",link,table,*command_status);
#endif
		return(ERROR);
	}
}

