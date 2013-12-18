/*
 * Copyright (c) INMOS Limited 1990
 *
 * Description	: Definitions for the WITCH B422
 * 		  device driver functions.
 *
 * Date		: 07-Nov-90
 *
 * Filename	: b422fns.h
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


void scsi_hadd_if(
	WORD	/* to_scsi_link			*/,
	WORD	/* from_scsi_link		*/,
	BYTE	/* target_id			*/,
	BYTE	/* lun				*/,
	BYTE	/* command_length		*/,
	BYTE*	/* scsi_command			*/,
	BYTE	/* direction			*/,
	INT32	/* rx_transfer_length		*/,
	BYTE*	/* rx_data			*/,
	INT32	/* tx_transfer_length		*/,
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
	INT32	/* logical_block_address	*/,
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
	INT32	/* logical_block_address	*/, 
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
	INT32	/* logical_block_address	*/,
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
	INT32	/* logical_block_address	*/, 
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


void output_pcol(
	BYTE	/* link				*/, 
	...	/* tag, p1, p2...		*/);
	

void input_tag(
	BYTE	/* link				*/, 
	BYTE*	/* tag				*/);

	
void input_pcol(
	BYTE	/* link				*/, 
	BYTE	/* tag				*/, 
	...	/* p1, p2...			*/);


int  b422_init(
	WORD	/* link				*/,
	BOOL	/* download			*/);
	

void b422_reset(
	WORD	/* link				*/,
	BYTE	/* parity_generation		*/,
	BYTE	/* parity_checking		*/,
	BYTE	/* cable_mode			*/,
	INT16	/* data_phase_time_out		*/,
	INT16	/* interrupt_time_out		*/,
	BYTE	/* initiator_id			*/,
	INT16	/* data_transfer_mode		*/);
	

void b422_term(
	WORD	/* link				*/);


WORD squirt(
	BYTE*	/* filename			*/, 
	WORD	/* link				*/);

BYTE getbyte(
	INT32	/* i				*/, 
	BYTE	/* b				*/);


