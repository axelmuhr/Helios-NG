

	/****************************************************************/
	/*								*/
	/* b422fns.h	Function definitions for B422 INMOS SCSI TRAM.	*/
	/*		Copyright (C) INMOS Limited 1990.		*/
	/*								*/
	/* Date		Ver	Author		Comment			*/
	/* ----		---	------		-------			*/
	/* 26-Mar-90	1.0	Mike Burrow	Original		*/
	/*								*/
	/****************************************************************/


void xpt(
	WORD	/* to_scsi_link   		*/,
	WORD	/* from_scsi_link	 	*/,
	BYTE	/* target_id      		*/,
	BYTE	/* command_length 		*/,
	BYTE*	/* scsi_command  		*/,
	BYTE	/* direction    	  	*/,
	INT64	/* rx_transfer_length	 	*/,
	BYTE*	/* rx_data			*/,
	INT64	/* tx_transfer_length		*/,	
	BYTE*	/* tx_data			*/,
	BYTE*	/* msg_length			*/,
	BYTE*	/* message			*/,
	BYTE*	/* scsi_status			*/,
	INT16*	/* execution_status		*/);


void scsi_write_10(
	BYTE	/* link_to_scsi			*/,
	BYTE	/* link_from_scsi		*/,
	BYTE	/* target_id			*/,
	BYTE	/* lun				*/,
	INT64	/* logical_block_address	*/,
	INT32	/* number_of_blocks		*/,
	INT32	/* block_size			*/,
	BYTE	/* control_byte			*/,
	BYTE*	/* tx_data			*/,
	BYTE*	/* message			*/,
	BYTE*	/* msg_length			*/,
	BYTE*	/* scsi_status			*/,
	INT16*	/* execution_status		*/);


void compile_write_10(
	BYTE 	/* lun				*/, 
	INT64	/* logical_block_address	*/, 
	INT32	/* transfer_count		*/, 
	BYTE	/* control_byte			*/,
	BYTE*	/* scsi_command			*/, 
	BYTE*	/* scsi_command_length		*/, 
	BYTE*	/* data_transfer		*/);


void scsi_read_10(
	BYTE	/* link_to_scsi			*/,
	BYTE	/* link_from_scsi		*/,
	BYTE	/* target_id			*/,
	BYTE	/* lun				*/,
	INT64	/* logical_block_address	*/,
	INT32	/* number_of_blocks		*/,
	INT32	/* block_size			*/,
	BYTE	/* control_byte			*/,
	BYTE*	/* rx_data			*/,
	BYTE*	/* message			*/,
	BYTE*	/* msg_length			*/,
	BYTE*	/* scsi_status			*/,
	INT16*	/* execution_status		*/);


void compile_read_10(
	BYTE 	/* lun				*/, 
	INT64	/* logical_block_address	*/, 
	INT32	/* transfer_count		*/, 
	BYTE	/* control_byte			*/,
	BYTE*	/* scsi_command			*/, 
	BYTE*	/* scsi_command_length		*/, 
	BYTE*	/* data_transfer		*/);


void scsi_read_capacity_10(
	BYTE	/* link_to_scsi			*/,
	BYTE	/* link_from_scsi		*/,
	BYTE	/* target_id			*/,
	BYTE	/* lun				*/,
	BYTE	/* reserved_0			*/,
	BOOL	/* reladr			*/,
	INT32	/* logical_block_address	*/,
	BYTE	/* reserved_1			*/,
	BYTE	/* reserved_2			*/,
	BYTE	/* reserved_3			*/,
	BOOL	/* pmi				*/,
	BYTE	/* control_byte			*/,
	BYTE*	/* read_capacity_data		*/,
	BYTE*	/* message			*/,
	BYTE*	/* msg_length			*/,
	BYTE*	/* scsi_status			*/,
	INT16*	/* execution_status		*/);


void compile_read_capacity_10(
	BYTE 	/* lun				*/,
	BYTE	/* reserved_0			*/,
	BOOL	/* reladr			*/,
	INT32	/* logical_block_address	*/, 
	BYTE	/* reserved_1			*/,
	BYTE	/* reserved_2			*/,
	BYTE	/* reserved_3			*/,
	BOOL	/* pmi				*/,
	BYTE	/* control_byte			*/,
	BYTE*	/* scsi_command			*/,
	BYTE*	/* scsi_command_length		*/,
	BYTE*	/* data_transfer		*/);


void scsi_format_6(
	BYTE	/* link_to_scsi			*/,
	BYTE	/* link_from_scsi		*/,
	BYTE	/* target_id			*/,
	BYTE	/* lun				*/,
	BYTE	/* fmtdata			*/,
	BYTE	/* cmplst			*/,
	BYTE	/* defect_list_format		*/,
	BYTE	/* vendor_specific		*/,
	INT32	/* interleave			*/,
	BYTE	/* control_byte			*/,
	INT32	/* defect_list_length		*/,
	BYTE*	/* defect_list			*/,
	BYTE*	/* message			*/,
	BYTE*	/* msg_length			*/,
	BYTE*	/* scsi_status			*/,
	INT16*	/* execution_status		*/);



void compile_format_6(
	BYTE 	/* lun				*/, 
	BYTE	/* fmtdata			*/,
	BYTE	/* cmplst			*/,
	BYTE	/* defect_list_format		*/,
	BYTE	/* vendor_specific		*/,
	INT32	/* interleave			*/,
	BYTE	/* control_byte			*/,
	BYTE*	/* scsi_command			*/, 
	BYTE*	/* scsi_command_length		*/, 
	BYTE*	/* data_transfer		*/);


void scsi_test_unit_ready_6(
	BYTE	/* link_to_scsi			*/,
	BYTE	/* link_from_scsi		*/,
	BYTE	/* target_id			*/,
	BYTE	/* lun				*/,
	BYTE	/* control_byte			*/,
	BYTE*	/* message			*/,
	BYTE*	/* msg_length			*/,
	BYTE*	/* scsi_status			*/,
	INT16*	/* execution_status		*/);



void compile_test_unit_ready_6(
	BYTE 	/* lun				*/, 
	BYTE	/* control_byte			*/,
	BYTE*	/* scsi_command			*/, 
	BYTE*	/* scsi_command_length		*/, 
	BYTE*	/* data_transfer		*/);


void scsi_request_sense_6(
	BYTE	/* link_to_scsi			*/,
	BYTE	/* link_from_scsi		*/,
	BYTE	/* target_id			*/,
	BYTE	/* lun				*/,
	BYTE	/* allocation_length		*/,
	BYTE	/* control_byte			*/,
	BYTE*	/* sense_data			*/,
	BYTE*	/* message			*/,
	BYTE*	/* msg_length			*/,
	BYTE*	/* scsi_status			*/,
	INT16*	/* execution_status		*/);


void compile_request_sense_6(
	BYTE 	/* lun				*/, 
	BYTE	/* allocation_length		*/,
	BYTE	/* control_byte			*/,
	BYTE*	/* scsi_command			*/, 
	BYTE*	/* scsi_command_length		*/, 
	BYTE*	/* data_transfer		*/);


void scsi_inquiry_6(
	BYTE	/* link_to_scsi			*/,
	BYTE	/* link_from_scsi		*/,
	BYTE	/* target_id			*/,
	BYTE	/* lun				*/,
	BYTE	/* reserved_0			*/,
	BYTE	/* reserved_1			*/,
	BYTE	/* reserved_2			*/,
	BYTE	/* allocation_length		*/,
	BYTE	/* control_byte			*/,
	BYTE*	/* inquiry_data			*/,
	BYTE*	/* message			*/,
	BYTE*	/* msg_length			*/,
	BYTE*	/* scsi_status			*/,
	INT16*	/* execution_status		*/);


void compile_inquiry_6(
	BYTE 	/* lun				*/, 
	BYTE	/* reserved_0			*/,
	BYTE	/* reserved_1			*/,
	BYTE	/* reserved_2			*/,
	BYTE	/* allocation_length		*/,
	BYTE	/* control_byte			*/,
	BYTE*	/* scsi_command			*/, 
	BYTE*	/* scsi_command_length		*/, 
	BYTE*	/* data_transfer		*/);


void scsi_write_6(
	BYTE	/* link_to_scsi			*/,
	BYTE	/* link_from_scsi		*/,
	BYTE	/* target_id			*/,
	BYTE	/* lun				*/,
	INT32	/* logical_block_address	*/,
	BYTE	/* number_of_blocks		*/,
	INT32	/* block_size			*/,
	BYTE	/* control_byte			*/,
	BYTE*	/* tx_data			*/,
	BYTE*	/* message			*/,
	BYTE*	/* msg_length			*/,
	BYTE*	/* scsi_status			*/,
	INT16*	/* execution_status		*/);


void compile_write_6(
	BYTE 	/* lun				*/, 
	INT32	/* logical_block_address	*/, 
	BYTE	/* transfer_count		*/, 
	BYTE	/* control_byte			*/,
	BYTE*	/* scsi_command			*/, 
	BYTE*	/* scsi_command_length		*/, 
	BYTE*	/* data_transfer		*/);


void scsi_read_6(
	BYTE	/* link_to_scsi			*/,
	BYTE	/* link_from_scsi		*/,
	BYTE	/* target_id			*/,
	BYTE	/* lun				*/,
	INT32	/* logical_block_address	*/,
	BYTE	/* number_of_blocks		*/,
	INT32	/* block_size			*/,
	BYTE	/* control_byte			*/,
	BYTE*	/* rx_data			*/,
	BYTE*	/* message			*/,
	BYTE*	/* msg_length			*/,
	BYTE*	/* scsi_status			*/,
	INT16*	/* execution_status		*/);


void compile_read_6(
	BYTE 	/* lun				*/, 
	INT32	/* logical_block_address	*/, 
	BYTE	/* transfer_count		*/, 
	BYTE	/* control_byte			*/,
	BYTE*	/* scsi_command			*/, 
	BYTE*	/* scsi_command_length		*/, 
	BYTE*	/* data_transfer		*/);


void scsi_mode_select_6(
	BYTE	/* link_to_scsi			*/,
	BYTE	/* link_from_scsi		*/,
	BYTE	/* target_id			*/,
	BYTE	/* lun				*/,
	BYTE	/* parameter_list_length	*/,
	BYTE	/* control_byte			*/,
	BYTE*	/* tx_data			*/,
	BYTE*	/* message			*/,
	BYTE*	/* msg_length			*/,
	BYTE*	/* scsi_status			*/,
	INT16*	/* execution_status		*/);


void compile_mode_select_6(
	BYTE 	/* lun				*/, 
	BYTE	/* parameter_list_length	*/, 
	BYTE	/* control_byte			*/,
	BYTE*	/* scsi_command			*/, 
	BYTE*	/* scsi_command_length		*/, 
	BYTE*	/* data_transfer		*/);



void scsi_mode_sense_6(
	BYTE	/* link_to_scsi			*/,
	BYTE	/* link_from_scsi		*/,
	BYTE	/* target_id			*/,
	BYTE	/* lun				*/,
	BYTE	/* page_code			*/,
	BYTE	/* allocation_length		*/,
	BYTE	/* control_byte			*/,
	BYTE*	/* rx_data			*/,
	BYTE*	/* message			*/,
	BYTE*	/* msg_length			*/,
	BYTE*	/* scsi_status			*/,
	INT16*	/* execution_status		*/);


void compile_mode_sense_6(
	BYTE 	/* lun				*/, 
	BYTE	/* page_code			*/,
	BYTE	/* allocation_length		*/, 
	BYTE	/* control_byte			*/,
	BYTE*	/* scsi_command			*/, 
	BYTE*	/* scsi_command_length		*/, 
	BYTE*	/* data_transfer		*/);




