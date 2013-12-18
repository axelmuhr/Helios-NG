

	/****************************************************************/
	/*								*/
	/* b422inq.c	Interface to inquiry command for B422 INMOS 	*/
	/*		SCSI TRAM. Copyright (C) INMOS Limited 1990.	*/
	/*								*/
	/* Date		Ver	Author		Comment			*/
	/* ----		---	------		-------			*/
	/* 03-Apr-90	1.0	Mike Burrow	Original		*/
	/*								*/
	/****************************************************************/


#include <helios.h>
#include "b422cons.h"
#include "b422err.h"
#include "b422pcol.h"
#include "b422fns.h"


void scsi_inquiry_6(
	BYTE	link_to_scsi,
	BYTE	link_from_scsi,
	BYTE	target_id,
	BYTE	lun,
	BYTE	reserved_0,
	BYTE	reserved_1,
	BYTE	reserved_2,
	BYTE	allocation_length,
	BYTE	control_byte,
	BYTE	*inquiry_data,
	BYTE	*message,
	BYTE	*msg_length,
	BYTE	*scsi_status,
	INT16	*execution_status)
{
	SCSI_CMND_6	scsi_command;
	BYTE		command_length;
	BYTE		data_transfer_mode;
	NULL_BUFFER	null_buffer;


	compile_inquiry_6(lun, 
			  reserved_0,
		 	  reserved_1,
			  reserved_2,
			  allocation_length,
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
	    (INT64)allocation_length,
	    inquiry_data,
	    NULL_LENGTH,
	    null_buffer,
	    msg_length,
	    message,
	    scsi_status,
	    execution_status);
}



void compile_inquiry_6(
	BYTE 	lun, 
	BYTE	reserved_0,
	BYTE	reserved_1,
	BYTE	reserved_2,
	BYTE	allocation_length,
	BYTE	control_byte,
	BYTE	*scsi_command, 
	BYTE	*scsi_command_length, 
	BYTE	*data_transfer)
{
	scsi_command[0] = INQUIRY;
	scsi_command[1] = ((lun << 5) & 0xe0) | (reserved_0 & 0x1f);
	scsi_command[2] = reserved_1;
	scsi_command[3] = reserved_2;
	scsi_command[4] = allocation_length;
	scsi_command[5] = control_byte;
	
	*scsi_command_length = SCSI_6;
	*data_transfer	     = SCSI_TO_HOST_VARIABLE;
}

