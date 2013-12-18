

	/****************************************************************/
	/*								*/
	/* b422r6.c	Interface to read 6 for B422 INMOS SCSI TRAM.	*/
	/*		Copyright (C) INMOS Limited 1990.		*/
	/*								*/
	/* Date		Ver	Author		Comment			*/
	/* ----		---	------		-------			*/
	/* 04-Apr-90	1.0	Mike Burrow	Original		*/
	/*								*/
	/****************************************************************/


#include <helios.h>
#include "b422cons.h"
#include "b422err.h"
#include "b422pcol.h"
#include "b422fns.h"


void scsi_read_6(
	BYTE	link_to_scsi,
	BYTE	link_from_scsi,
	BYTE	target_id,
	BYTE	lun,
	INT32	logical_block_address,
	BYTE	number_of_blocks,
	INT32	block_size,
	BYTE	control_byte,
	BYTE	*rx_data,
	BYTE	*message,
	BYTE	*msg_length,
	BYTE	*scsi_status,
	INT16	*execution_status)
{
	SCSI_CMND_6	scsi_command;
	BYTE		command_length;
	BYTE		data_transfer_mode;
	NULL_BUFFER	tx_data;
	INT64		rx_transfer_length;
	
	rx_transfer_length = block_size * number_of_blocks;
	
	compile_read_6(lun, 
		       logical_block_address, 
		       number_of_blocks, 
		       control_byte,
		       scsi_command, 
		       &command_length, 
		       &data_transfer_mode);
			 
	xpt(link_to_scsi,
	    link_from_scsi, 
	    target_id,
	    command_length,
	    scsi_command,
	    data_transfer_mode,
	    rx_transfer_length,
	    rx_data,
	    NULL_LENGTH,
	    tx_data,
	    msg_length,
	    message,
	    scsi_status,
	    execution_status);
}



void compile_read_6(
	BYTE 	lun, 
	INT32	logical_block_address, 
	BYTE	transfer_count, 
	BYTE	control_byte,
	BYTE	*scsi_command, 
	BYTE	*scsi_command_length, 
	BYTE	*data_transfer)
{
	scsi_command[0] = READ;
	scsi_command[1] = (lun << 5) |
			  (getbyte(logical_block_address, 3) & 0x1f);
	scsi_command[2] = getbyte(logical_block_address, 2);
	scsi_command[3] = getbyte(logical_block_address, 1);
	scsi_command[4] = transfer_count;
	scsi_command[5] = control_byte;
	
	*scsi_command_length = SCSI_6;
	*data_transfer	     = SCSI_TO_HOST_VARIABLE;
}

