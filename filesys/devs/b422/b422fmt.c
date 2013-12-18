/*
 * Copyright (c) INMOS Limited 1990
 *
 * Description	: Interface to Format for the WITCH IMS B422 SCSI TRAM.
 *
 * Date		: 16-Nov-90
 *
 * Filename	: b422fmt.c
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
/*
 * The Format command formats the medium into initiator addressable logical
 * blocks, according to the initiator defined options. In addition the 
 * medium may be certified and control structures may be created fro the
 * management of the medium and defects. There is no guarantee that the
 * medium has or has not been altered.
 */
{
	SCSI_CMND_6	scsi_command;
	BYTE		command_length;
	BYTE		data_transfer_mode;
	BYTE		rx_data[1];
	INT32		rx_transfer_length;
	
	rx_transfer_length = 0;
	
	compile_format_6(
		lun,
		fmtdata,
		cmplst,
		defect_list_format,
		vendor_specific,
		interleave, 
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
	    	defect_list_length,
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
/*
 * Build up the scsi command block from the input parameters. Return
 * the command, command length and data transfer direction.
 */
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
	*data_transfer	     = HOST_TO_SCSI;
}

