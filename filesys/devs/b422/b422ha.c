/*
 * Copyright (c) INMOS Limited 1990
 *
 * Description	: Host Adaptor Device Driver Interface. Transport
 *                SCSI command to B422 TRAM.
 *
 * Date		: 06-Nov-90
 *
 * Filename	: b422ha.c
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
#include <syslib.h>
#include "b422def.h"
#include "b422fns.h"


void scsi_hadd_if(WORD	to_scsi_link,
		  WORD	from_scsi_link,
		  BYTE	target_id,
	 	  BYTE  lun,
	 	  BYTE	command_length,
		  BYTE	*scsi_command,
	 	  BYTE	direction,
		  INT32	rx_transfer_length,
		  BYTE	*rx_data,
		  INT32	tx_transfer_length,
		  BYTE	*tx_data,
		  BYTE	*msg_length,
		  BYTE	*message,
		  BYTE	*scsi_status,
		  INT16	*execution_status)
	 
	
{
	BOOL		going;
	INT16		io_pno, rx_packet_length, tx_packet_length;
	BYTE		*tx_data_pointer, *rx_data_pointer, tag, linkinfo;
	INT32		transfer_count;

	if (*msg_length > MAX_SCSI_MESSAGE_SIZE)
		*execution_status = SCSI_E_BAD_MESSAGE_LENGTH;
	else if (command_length > MAX_SCSI_COMMAND_SIZE)
		*execution_status = SCSI_E_BAD_MESSAGE_LENGTH;
	else if (lun > MAX_SCSI_LUN)
		*execution_status = SCSI_E_BAD_LUN;
	else if (target_id > MAX_SCSI_DEVICE_ID)
		*execution_status = SCSI_E_BAD_DEVICE_ID;
	else if (direction > MAX_SCSI_VALID_DIRECTION)
		*execution_status = SCSI_E_SOFTWARE_ERROR;
	else {
		*execution_status = SCSI_E_GOOD;
		going = TRUE;
		io_pno = 0;
		linkinfo = SINGLE_LINK;
	
		output_pcol(to_scsi_link, SCSI_LINK_MANAGEMENT, &linkinfo);
		output_pcol(to_scsi_link, SCSI_I_DIRECTION, &io_pno, &direction);
		output_pcol(to_scsi_link, SCSI_I_TARGET_ID, &io_pno, &target_id);
		output_pcol(to_scsi_link, SCSI_I_LUN, &io_pno, &lun);
		output_pcol(to_scsi_link, SCSI_I_MESSAGE_BUFFER, &io_pno, 
			    msg_length, message);
		output_pcol(to_scsi_link, SCSI_I_COMMAND_BUFFER, &io_pno, 
			    &command_length, scsi_command);
		
		switch (direction) {
	
		case NON_DATA_TRANSFER:
	
			output_pcol(to_scsi_link, SCSI_I_START_IO_PROCESS, 
				    &io_pno);
		
			while (going) {
			
				input_tag(from_scsi_link, &tag);
				switch (tag) {
			
				case SCSI_E_EXECUTION_STATUS:
					input_pcol(from_scsi_link, tag, 
						   &io_pno, execution_status);
					going = FALSE;
					break;
			
				case SCSI_I_MSGBUFF:
					input_pcol(from_scsi_link, tag, 
						   &io_pno, msg_length, 
						   message);
					break;
			
				case SCSI_I_STATUS:
					input_pcol(from_scsi_link, tag, 
						   &io_pno, scsi_status);
					break;
			
				default:
					break;
				}
			}
			break;

		case SCSI_TO_HOST:
				
			output_pcol(to_scsi_link, SCSI_I_RX_LENGTH, 
				    &io_pno, &rx_transfer_length);

			if (rx_transfer_length > MAX_SCSI_PACKET_SIZE)
				rx_packet_length = MAX_SCSI_PACKET_SIZE;
			else if (rx_transfer_length < MIN_SCSI_PACKET_SIZE)
				rx_packet_length = rx_transfer_length;
			else
				rx_packet_length = rx_transfer_length / 2;
			
			output_pcol(to_scsi_link, 
				    SCSI_PM_REQUEST_PACKET_SIZE,
				    &rx_packet_length);
		
			input_tag(from_scsi_link, &tag);
			
			if (tag == SCSI_PM_ALLOCATED_PACKET_SIZE)
				input_pcol(from_scsi_link, tag, 
					   &rx_packet_length);

			output_pcol(to_scsi_link, SCSI_I_START_IO_PROCESS, 
				    &io_pno);
		
			rx_data_pointer = rx_data;
		
			while (going) {
			
				input_tag(from_scsi_link, &tag);

				switch(tag) {
				
				case SCSI_E_EXECUTION_STATUS:
					input_pcol(from_scsi_link, tag, 
						   &io_pno, 
						   execution_status);
					going = FALSE;
					break;
				
				case SCSI_I_MSGBUFF:
					input_pcol(from_scsi_link, tag, 
						   &io_pno,
					           msg_length, message);
					break;
				
				case SCSI_I_STATUS:
					input_pcol(from_scsi_link, tag, 
						   &io_pno, scsi_status);
					break;
				
				case SCSI_I_RX_DATA:
					input_pcol(from_scsi_link, tag, 
						   &io_pno, &transfer_count, 
						   &rx_packet_length,
				              	   rx_data_pointer);

					rx_data_pointer += rx_packet_length;
					break;
				
				default:
					break;
				}
			}
			break;
		
		case HOST_TO_SCSI:
	
			output_pcol(to_scsi_link, SCSI_I_TX_LENGTH, &io_pno, 
				    &tx_transfer_length);
		
			if (tx_transfer_length > MAX_SCSI_PACKET_SIZE)
				tx_packet_length = MAX_SCSI_PACKET_SIZE;
			else if (tx_transfer_length < MIN_SCSI_PACKET_SIZE)
				tx_packet_length = tx_transfer_length;
			else
				tx_packet_length = tx_transfer_length / 2;

			output_pcol(to_scsi_link, SCSI_PM_REQUEST_PACKET_SIZE,
				    &tx_packet_length);
		
			input_tag(from_scsi_link, &tag);
			if (tag == SCSI_PM_ALLOCATED_PACKET_SIZE)
				input_pcol(from_scsi_link, tag, 
					   &tx_packet_length);
			
			output_pcol(to_scsi_link, SCSI_I_START_IO_PROCESS, 
				    &io_pno);
		
			tx_data_pointer = tx_data;
		
			while (going) {
			
				input_tag(from_scsi_link, &tag);
			
				switch (tag) {
			
				case SCSI_E_EXECUTION_STATUS:
					input_pcol(from_scsi_link, tag, 
						   &io_pno, 
						   execution_status);
					going = FALSE;
					break;
				
				case SCSI_I_MSGBUFF:
					input_pcol(from_scsi_link, tag, 
						   &io_pno, 
					   	   msg_length, message);
					break;
				
				case SCSI_I_STATUS:
					input_pcol(from_scsi_link, tag, 
						   &io_pno, scsi_status);
					break;
				
				case SCSI_I_TX_SEND_PACKET:
					input_pcol(from_scsi_link, tag, 
						&io_pno, &transfer_count,
						&tx_packet_length);
					output_pcol(to_scsi_link,
					            SCSI_I_TX_DATA,
					    	    &io_pno, &tx_packet_length,
						    tx_data_pointer);
					tx_data_pointer += tx_packet_length;
					break;
				
				default:
					break;
				}
			}	
			break;
			
		default:
			break;
		}
	}
}
 
