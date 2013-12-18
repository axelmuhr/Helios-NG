

	/****************************************************************/
	/*								*/
	/* b422msel.c	Interface to mode select for B422 INMOS SCSI	*/
	/*		TRAM. Copyright (C) INMOS Limited 1990.		*/
	/*								*/
	/* Date		Ver	Author		Comment			*/
	/* ----		---	------		-------			*/
	/* 06-Apr-90	1.0	Mike Burrow	Original		*/
	/*								*/
	/****************************************************************/


#include <helios.h>
#include "b422cons.h"
#include "b422err.h"
#include "b422pcol.h"
#include "b422fns.h"


void scsi_mode_select_6(
	BYTE	link_to_scsi,
	BYTE	link_from_scsi,
	BYTE	target_id,
	BYTE	lun,
	BYTE	parameter_list_length,
	BYTE	control_byte,
	BYTE	*tx_data,
	BYTE	*message,
	BYTE	*msg_length,
	BYTE	*scsi_status,
	INT16	*execution_status)
{
	SCSI_CMND_6	scsi_command;
	BYTE		command_length;
	BYTE		data_transfer_mode;
	BYTE		rx_data[1];
	INT64		rx_transfer_length;
	
	rx_transfer_length = 0;

	compile_mode_select_6(lun, 
			      parameter_list_length,
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
	    (INT64)parameter_list_length,
	    tx_data,
	    msg_length,
	    message,
	    scsi_status,
	    execution_status);
}



void compile_mode_select_6(
	BYTE 	lun, 
	BYTE	parameter_list_length, 
	BYTE	control_byte,
	BYTE	*scsi_command, 
	BYTE	*scsi_command_length, 
	BYTE	*data_transfer)
{
	scsi_command[0] = MODE_SELECT;
	scsi_command[1] = lun << 5;
	scsi_command[2] = 0;
	scsi_command[3] = 0;
	scsi_command[4] = parameter_list_length;
	scsi_command[5] = control_byte;
	
	*scsi_command_length = SCSI_6;
	*data_transfer	     = HOST_TO_SCSI_VARIABLE;
}

