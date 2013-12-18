/*
 * Copyright (c) INMOS Limited 1990
 *
 * Description	: Interface to Read 10 for the WITCH IMS B422 SCSI TRAM.
 *
 * Date		: 16-Nov-90
 *
 * Filename	: b422r10.c
 *
 * Project	: WITCH
 *
 * Author	: Mike Burrow
 *
 * Version	: 2.0
 *
 * Division	: CAD
 *
 * Limitations	: None
 *
 * Amendments	:
 *
 */


#include <helios.h>
#include "b422def.h"
#include "b422fns.h"


void scsi_read_10(
	BYTE	link_to_scsi,
	BYTE	link_from_scsi,
	BYTE	target_id,
	BYTE	lun,
	INT32	logical_block_address,
	INT32	number_of_blocks,
	INT32	block_size,
	BYTE	control_byte,
	BYTE	*rx_data,
	BYTE	*message,
	BYTE	*msg_length,
	BYTE	*scsi_status,
	INT16	*execution_status)
/*
 * The Read 10 command requests that the target transfer data to the
 * initiator.
 */
{
	SCSI_CMND_10	scsi_command;
	BYTE		command_length;
	BYTE		data_transfer_mode;
	NULL_BUFFER	tx_data;
	INT32		rx_transfer_length;
	
	rx_transfer_length = block_size * number_of_blocks;
	
	compile_read_10(
		lun, 
		logical_block_address, 
		number_of_blocks, 
		control_byte,
		scsi_command, 
		&command_length, 
		&data_transfer_mode);
			 
	scsi_hadd_if(
		link_to_scsi,
	    	link_from_scsi, 
	    	target_id,
	    	lun,
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

#ifdef DEBUG_b422r10	    	
	IOdebug("[R10] lba: %d,\tnum: %d,\tscsi: %d,\texe: %d\tlen: %d,\tmsg:<%s>",
		logical_block_address, 
		number_of_blocks, 
	    	*scsi_status,
	    	*execution_status,
	    	*msg_length,
	    	message);
#endif
}



void compile_read_10(
	BYTE 	lun, 
	INT32	logical_block_address, 
	INT32	transfer_count, 
	BYTE	control_byte,
	BYTE	*scsi_command, 
	BYTE	*scsi_command_length, 
	BYTE	*data_transfer)
/*
 * Build up the scsi command block from the input parameters. Return
 * the command, command length and data transfer direction.
 */
{
	scsi_command[0] = READ_10;
	scsi_command[1] = lun << 5;
	scsi_command[2] = getbyte((INT32)logical_block_address, 4);
	scsi_command[3] = getbyte((INT32)logical_block_address, 3);
	scsi_command[4] = getbyte((INT32)logical_block_address, 2);
	scsi_command[5] = getbyte((INT32)logical_block_address, 1);
	scsi_command[6] = 0;
	scsi_command[7] = getbyte(transfer_count, 2);
	scsi_command[8] = getbyte(transfer_count, 1);
	scsi_command[9] = control_byte;
	
	*scsi_command_length = SCSI_10;
	*data_transfer	     = SCSI_TO_HOST;
}

