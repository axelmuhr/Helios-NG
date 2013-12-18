

	/****************************************************************/
	/*								*/
	/* b422xpc.c	Protocol transfer for B422 INMOS SCSI TRAM.	*/
	/*		Copyright (C) INMOS Limited 1990.		*/
	/*								*/
	/* Date		Ver	Author		Comment			*/
	/* ----		---	------		-------			*/
	/* 21-Mar-90	1.0	Mike Burrow	Original		*/
	/*								*/
	/****************************************************************/


#include <helios.h>
#include <syslib.h>
#include "b422cons.h"
#include "b422err.h"
#include "b422pcol.h"
#include "b422fns.h"


void xpt(WORD	to_scsi_link,
	 WORD	from_scsi_link,
	 BYTE	target_id,
	 BYTE	command_length,
	 BYTE	*scsi_command,
	 BYTE	direction,
	 INT64	rx_transfer_length,
	 BYTE	*rx_data,
	 INT64	tx_transfer_length,
	 BYTE	*tx_data,
	 BYTE	*msg_length,
	 BYTE	*message,
	 BYTE	*scsi_status,
	 INT16	*execution_status)
	 
	
{
	BOOL		going;
	INT16		io_pno, rx_packet_length, tx_packet_length;
	BYTE		*tx_data_pointer, *rx_data_pointer, tag;
	INT32		bytes_left;


	io_pno = 0;
	
	output_pcol(to_scsi_link, SCSI_I_DIRECTION, &io_pno, &direction);
	output_pcol(to_scsi_link, SCSI_I_TARGET_ID, &io_pno, &target_id);
	output_pcol(to_scsi_link, SCSI_I_MESSAGE_BUFFER, &io_pno, 
		    msg_length, message);

	output_pcol(to_scsi_link, SCSI_I_COMMAND_BUFFER, &io_pno, 
		    &command_length, scsi_command);
	
	going = TRUE;
		
	switch (direction) {

	case NON_DATA_TRANSFER:
	
		output_pcol(to_scsi_link, SCSI_I_START_IO_PROCESS, &io_pno);
		
		while (going) {
			
			input_tag(from_scsi_link, &tag);
			switch (tag) {
			
			case SCSI_I_EXECUTION_STATUS:
				input_pcol(from_scsi_link, tag, &io_pno, 
				           execution_status);
				going = FALSE;
				break;
			
			case SCSI_I_MSGBUFF:
				input_pcol(from_scsi_link, tag, &io_pno, 
					   msg_length, message);
				break;
			
			case SCSI_I_STATUS:
				input_pcol(from_scsi_link, tag, &io_pno, 
				 	   scsi_status);
				break;
			
			default:
				break;
			}
		}
		break;

	case SCSI_TO_HOST_VARIABLE:
	
		bytes_left = rx_transfer_length;
			
		output_pcol(to_scsi_link, SCSI_I_RX_LENGTH, 
			    &io_pno, &rx_transfer_length);

		if (rx_transfer_length > MAX_SCSI_PACKET_SIZE)
			rx_packet_length = MAX_SCSI_PACKET_SIZE;
		else if (rx_transfer_length < MIN_SCSI_PACKET_SIZE)
			rx_packet_length = MIN_SCSI_PACKET_SIZE;
		else
			rx_packet_length = rx_transfer_length / 2;
			
		output_pcol(to_scsi_link, SCSI_PM_REQUEST_RX_PACKET_SIZE,
			    &rx_packet_length);
		
		input_tag(from_scsi_link, &tag);
		if (tag == SCSI_PM_ALLOCATED_RX_PACKET_SIZE)
			input_pcol(from_scsi_link, tag, &rx_packet_length);

		output_pcol(to_scsi_link, SCSI_I_START_IO_PROCESS, &io_pno);
		
		rx_data_pointer = rx_data;
		
		while (going) {
			
			input_tag(from_scsi_link, &tag);

			switch(tag) {
				
			case SCSI_I_EXECUTION_STATUS:
				input_pcol(from_scsi_link, tag, &io_pno, 
					   execution_status);
				going = FALSE;
				break;
				
			case SCSI_I_MSGBUFF:
				input_pcol(from_scsi_link, tag, &io_pno,
					   msg_length, message);
				break;
				
			case SCSI_I_STATUS:
				input_pcol(from_scsi_link, tag, &io_pno, 
					   scsi_status);
				break;
				
			case SCSI_I_RXBUFF_VARIABLE:
				if (bytes_left <= 0) {
					IOdebug("xpt: ** ERROR ** too much data");
					going = FALSE;
				}
				else {
					input_pcol(from_scsi_link, tag, 
						   &io_pno, 
						   &rx_packet_length,
				              	   rx_data_pointer);

					rx_data_pointer += rx_packet_length;
					bytes_left -= rx_packet_length;
				}
				break;
				
			default:
				break;
			}
		}
		break;
		
	case HOST_TO_SCSI_VARIABLE:
	
		bytes_left = tx_transfer_length;
		
		output_pcol(to_scsi_link, SCSI_I_TX_LENGTH, &io_pno, 
			    &tx_transfer_length);
		
		if (tx_transfer_length > MAX_SCSI_PACKET_SIZE)
			tx_packet_length = MAX_SCSI_PACKET_SIZE;
		else if (tx_transfer_length < MIN_SCSI_PACKET_SIZE)
			tx_packet_length = MIN_SCSI_PACKET_SIZE;
		else
			tx_packet_length = tx_transfer_length / 2;

		output_pcol(to_scsi_link, SCSI_PM_REQUEST_TX_PACKET_SIZE,
			    &tx_packet_length);
		
		input_tag(from_scsi_link, &tag);
		if (tag == SCSI_PM_ALLOCATED_TX_PACKET_SIZE)
			input_pcol(from_scsi_link, tag, &tx_packet_length);
			
		output_pcol(to_scsi_link, SCSI_I_START_IO_PROCESS, &io_pno);
		
		tx_data_pointer = tx_data;
		
		while (going) {
			
			input_tag(from_scsi_link, &tag);
			
			switch (tag) {
			
			case SCSI_I_EXECUTION_STATUS:
				input_pcol(from_scsi_link, tag, &io_pno, 
					   execution_status);
				going = FALSE;
				break;
				
			case SCSI_I_MSGBUFF:
				input_pcol(from_scsi_link, tag, &io_pno, 
					   msg_length, message);
				break;
				
			case SCSI_I_STATUS:
				input_pcol(from_scsi_link, tag, &io_pno, 
					   scsi_status);
				break;
				
			case SCSI_I_TX_SEND_PACKET:
				input_pcol(from_scsi_link, tag, &io_pno,
					   &tx_packet_length);
				if (bytes_left < tx_packet_length) {
					IOdebug("xpt: ** ERROR ** not enough data");
					going = FALSE;
				}
				else {
					output_pcol(to_scsi_link,
					            SCSI_I_TXBUFF_VARIABLE,
						    &io_pno, &tx_packet_length,
						    tx_data_pointer);
					tx_data_pointer += tx_packet_length;
					bytes_left -= tx_packet_length;
				}
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
 
