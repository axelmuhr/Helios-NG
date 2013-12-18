/*
 * Copyright (c) INMOS Limited 1990
 *
 * Description	: Interface to Request Sense for the WITCH IMS B422 SCSI TRAM.
 *
 * Date		: 16-Nov-90
 *
 * Filename	: b422reqs.c
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


void scsi_request_sense_6(
	BYTE	link_to_scsi,
	BYTE	link_from_scsi,
	BYTE	target_id,
	BYTE	lun,
	BYTE	allocation_length,
	BYTE	control_byte,
	BYTE	*sense_data,
	BYTE	*message,
	BYTE	*msg_length,
	BYTE	*scsi_status,
	INT16	*execution_status)
/*
 * The Request Sense command requests that the target transfer sense
 * data to the initiator. This interface should be called following a
 * returned scsi_status of SCSI_STATUS_CHECK_CONDITION from a previously
 * issued SCSI command. It permits the caller to obtain information as
 * to the cause of a failure.
 */
{
	SCSI_CMND_6	scsi_command;
	BYTE		command_length;
	BYTE		data_transfer_mode;
	BYTE		tx_data[1];
	INT64		tx_transfer_length;
	
	tx_transfer_length = 0;
	
	compile_request_sense_6(
		lun,
		allocation_length,
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
		(INT32)allocation_length,
		sense_data,
		tx_transfer_length,
		tx_data,
		msg_length,
		message,
		scsi_status,
		execution_status);
}



void compile_request_sense_6(
	BYTE 	lun, 
	BYTE	allocation_length,
	BYTE	control_byte,
	BYTE	*scsi_command, 
	BYTE	*scsi_command_length, 
	BYTE	*data_transfer)
/*
 * Build up the scsi command block from the input parameters. Return
 * the command, command length and data transfer direction.
 */
{
	scsi_command[0] = REQUEST_SENSE;
	scsi_command[1] = lun << 5;
	scsi_command[2] = 0;
	scsi_command[3] = 0;
	scsi_command[4] = allocation_length;
	scsi_command[5] = control_byte;
	
	*scsi_command_length = SCSI_6;
	*data_transfer	     = SCSI_TO_HOST;
}

