/*
 * Copyright (c) INMOS Limited 1990
 *
 * Description	: Interface to Test Unit Ready for the WITCH IMS 
 *		  B422 SCSI TRAM.
 *
 * Date		: 16-Nov-90
 *
 * Filename	: b422tur.c
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


void scsi_test_unit_ready_6(
	BYTE	link_to_scsi,
	BYTE	link_from_scsi,
	BYTE	target_id,
	BYTE	lun,
	BYTE	control_byte,
	BYTE	*message,
	BYTE	*msg_length,
	BYTE	*scsi_status,
	INT16	*execution_status)
/*
 * The Test Unit Ready command provides a means to check if the logical
 * unit is ready. If the scsi_status returned does not equal GOOD, then
 * the Request Sense interface should be called in order to ascertain 
 * the reason the unit is demanding attention.
 */
{
	SCSI_CMND_6	scsi_command;
	BYTE		command_length;
	BYTE		data_transfer_mode;
	BYTE		rx_data[1];
	INT32		rx_transfer_length;
	BYTE		tx_data[1];
	INT32		tx_transfer_length;
	
	rx_transfer_length = 0;
	tx_transfer_length = 0;
	
	compile_test_unit_ready_6(lun,
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
		tx_transfer_length,
		tx_data,
		msg_length,
		message,
		scsi_status,
		execution_status);
}



void compile_test_unit_ready_6(
	BYTE 	lun, 
	BYTE	control_byte,
	BYTE	*scsi_command, 
	BYTE	*scsi_command_length, 
	BYTE	*data_transfer)
/*
 * Build up the scsi command block from the input parameters. Return
 * the command, command length and data transfer direction.
 */
{
	scsi_command[0] = TEST_UNIT_READY;
	scsi_command[1] = lun << 5;
	scsi_command[2] = 0;
	scsi_command[3] = 0;
	scsi_command[4] = 0;
	scsi_command[5] = control_byte;
	
	*scsi_command_length = SCSI_6;
	*data_transfer	     = HOST_TO_SCSI;
}

