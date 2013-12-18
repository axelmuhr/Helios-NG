

	/****************************************************************/
	/*								*/
	/* b422fmt.c	Interface to format for B422 INMOS SCSI TRAM.	*/
	/*		Copyright (C) INMOS Limited 1990.		*/
	/*								*/
	/* Date		Ver	Author		Comment			*/
	/* ----		---	------		-------			*/
	/* 27-Mar-90	1.0	Mike Burrow	Original		*/
	/*								*/
	/****************************************************************/


#include <helios.h>
#include "b422cons.h"
#include "b422err.h"
#include "b422pcol.h"
#include "b422fns.h"


void scsi_format_6(
	BYTE	link_to_scsi,
	BYTE	link_from_scsi,
	BYTE	target_id,
	BYTE	lun,
	BYTE	fmtdata,
	BYTE	cmplst,
	BYTE	defect_list_format,
	BYTE	vendor_specific,
	INT32	interleave,
	BYTE	control_byte,
	INT32	defect_list_length,
	BYTE	*defect_list,
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
	
	compile_format_6(lun,
			 fmtdata,
			 cmplst,
			 defect_list_format,
			 vendor_specific,
			 interleave, 
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
	    (INT64)defect_list_length,
	    defect_list,
	    msg_length,
	    message,
	    scsi_status,
	    execution_status);
}



void compile_format_6(
	BYTE 	lun, 
	BYTE	fmtdata,
	BYTE	cmplst,
	BYTE	defect_list_format,
	BYTE	vendor_specific,
	INT32	interleave,
	BYTE	control_byte,
	BYTE	*scsi_command, 
	BYTE	*scsi_command_length, 
	BYTE	*data_transfer)
{
	scsi_command[0] = FORMAT_UNIT;
	scsi_command[1] = (((lun & 0x07) << 5) | 
			   ((fmtdata & 0x01) << 4)) |
			  (((cmplst & 0x01) << 3) |
			   (defect_list_format & 0x07));
	scsi_command[2] = vendor_specific;
	scsi_command[3] = getbyte(interleave, 2);
	scsi_command[4] = getbyte(interleave, 1);
	scsi_command[5] = control_byte;
	
	*scsi_command_length = SCSI_6;
	*data_transfer	     = HOST_TO_SCSI_VARIABLE;
}

