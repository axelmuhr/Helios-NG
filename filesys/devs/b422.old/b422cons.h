

	/****************************************************************/
	/*								*/
	/* b422cons.h	Constant definitions for B422 INMOS SCSI TRAM.	*/
	/*		Copyright (C) INMOS Limited 1990.		*/
	/*								*/
	/* Date		Ver	Author		Comment			*/
	/* ----		---	------		-------			*/
	/* 19-Mar-90	1.0	Mike Burrow	Original		*/
	/*								*/
	/****************************************************************/


/* B422 SCSI ID */
/****************/

#define	B422_SCSI_ID				0x0007


/* Link constants for Helios */
/*****************************/

#define MAX_LINK_CAPACITY			0x3000
#define LINKTIMEOUT				0x40000000


/* Those applicable to link direction */
/**************************************/

#define	NON_DATA_TRANSFER			0x0000
#define	HOST_TO_SCSI_VARIABLE			0x0005
#define	SCSI_TO_HOST_VARIABLE			0x000a
#define	TARGET_TO_SCSI_VARIABLE			0x000f
#define	SCSI_TO_TARGET_VARIABLE			0x0014



/* General definitions */
/***********************/

#define NEGATED_B				0x0000
#define	ASSERTED_B				0x00ff



/* Link management definitions */
/*******************************/

#define	CD0_CD1_CD2_CD3				0x0000
#define	CD0_D1_CD2_D3				0x0001
#define	CD0_D1_D2_CD3				0x0002
#define	CD0_D1_D2_D3				0x0003
#define	MAX_LINK_MESSAGE_SIZE			0x0100
#define	LINK_COUNT				0x0004



/* Buffer sizes */
/****************/

#define	SIZE_1					0x0200
#define	SIZE_2					0x0400
#define	SIZE_3					0x0800
#define	SIZE_4					0x1000

#define	PART_1_OF_2				0x0100
#define	PART_2_OF_2				0x0100

#define	PART_1_OF_3				0x00ac
#define	PART_2_OF_3				0x00ac
#define	PART_3_OF_3				0x00a8

#define PART_1_OF_4				0x0080
#define PART_2_OF_4				0x0080
#define PART_3_OF_4				0x0080
#define PART_4_OF_4				0x0080

#define	TX_BUFFER_ALLOCATION			0x4000
#define	RX_BUFFER_ALLOCATION			0x4000

#define MAX_BYTE				0x0100
#define MAX_INT16				0x2000

#define SCSI_6					0x0006
#define SCSI_10					0x000a

#define CAPACITY_DATA_LENGTH			0x0008
#define	REQUEST_SENSE_DATA_LENGTH		0x0012

#define	NULL_LENGTH				0x0000
#define NULL_SIZE				0x0001


/* Packet management */
/*********************/

#define MAX_SCSI_PACKET_SIZE			0x2000
#define	MIN_SCSI_PACKET_SIZE			0x0100



/* SCSI protocol sizes */
/***********************/

#define	MAX_SCSI_COMMAND_SIZE			0x0100
#define	MAX_SCSI_MESSAGE_SIZE			0x0100
#define	MAX_SCSI_IO_PROCESSES			0x0100



/* The following are constants associated with the tag fields of
 * occam variant protocols. It assumes the 1st tag = 0. The problem
 * is that variant protocols are only defined in occam.
 */


/* SCSI operating modes */
/************************/

/* To SCSI */
#define SCSI_OPEN_CONNECTION			0x0000
#define SCSI_CLOSE_CONNECTION			0x0001
#define SCSI_BECOME_AN_INITIATOR		0x0002
#define	SCSI_BECOME_A_TARGET			0x0003



/* SCSI initiators */
/*******************/

/* To SCSI */
#define	SCSI_I_TARGET_ID			0x0004
#define	SCSI_I_MESSAGE_BUFFER			0x0005
#define	SCSI_I_COMMAND_BUFFER			0x0006
#define	SCSI_I_DIRECTION			0x0007
#define	SCSI_I_START_IO_PROCESS			0x0008
#define	SCSI_I_TX_LENGTH			0x0009
#define	SCSI_I_RX_LENGTH			0x000a
#define	SCSI_I_TXBUFF_VARIABLE			0x000b
#define	SCSI_I_TXBUFF_1_OF_2			0x000c
#define	SCSI_I_TXBUFF_2_OF_2			0x000d
#define	SCSI_I_TXBUFF_1_OF_3			0x000e
#define	SCSI_I_TXBUFF_2_OF_3			0x000f
#define	SCSI_I_TXBUFF_3_OF_3			0x0010
#define	SCSI_I_TXBUFF_1_OF_4			0x0011
#define	SCSI_I_TXBUFF_2_OF_4			0x0012
#define	SCSI_I_TXBUFF_3_OF_4			0x0013
#define	SCSI_I_TXBUFF_4_OF_4			0x0014
#define	SCSI_I_BLOCK_SIZE			0x0015
#define	SCSI_I_ATTRIBUTES			0x0016

/* From SCSI */
#define	SCSI_I_TX_SEND_PACKET			0x0000
#define	SCSI_I_RXBUFF_VARIABLE			0x0001
#define	SCSI_I_RXBUFF_1_OF_2			0x0002
#define	SCSI_I_RXBUFF_2_OF_2			0x0003
#define	SCSI_I_RXBUFF_1_OF_3			0x0004
#define	SCSI_I_RXBUFF_2_OF_3			0x0005
#define	SCSI_I_RXBUFF_3_OF_3			0x0006
#define	SCSI_I_RXBUFF_1_OF_4			0x0007
#define	SCSI_I_RXBUFF_2_OF_4			0x0008
#define	SCSI_I_RXBUFF_3_OF_4			0x0009
#define	SCSI_I_RXBUFF_4_OF_4			0x000a
#define	SCSI_I_MSGBUFF				0x000b
#define	SCSI_I_STATUS				0x000c
#define	SCSI_I_EXECUTION_STATUS			0x0020
#define	SCSI_I_CURRENT_EXECUTION_STATUS		0x0021



/* SCSI target modes */
/*********************/

/* To SCSI */
#define	SCSI_T_INITIATOR_ID			0x0017
#define	SCSI_T_DISCONNECT			0x0018
#define	SCSI_T_MESSAGE_BUFFER_1			0x0019
#define	SCSI_T_STATUS				0x001a
#define	SCSI_T_TXBUFF_VARIABLE			0x001b
#define	SCSI_T_TXBUFF_1_OF_2			0x001c
#define	SCSI_T_TXBUFF_2_OF_2			0x001d
#define	SCSI_T_TXBUFF_1_OF_3			0x001e
#define	SCSI_T_TXBUFF_2_OF_3			0x001f
#define	SCSI_T_TXBUFF_3_OF_3			0x0020
#define	SCSI_T_TXBUFF_1_OF_4			0x0021
#define	SCSI_T_TXBUFF_2_OF_4			0x0022
#define	SCSI_T_TXBUFF_3_OF_4			0x0023
#define	SCSI_T_TXBUFF_4_OF_4			0x0024

/* From SCSI */
#define	SCSI_T_TARGET_ID			0x000d
#define	SCSI_T_MESSAGE_BUFFER_2			0x000e	/* defined twice! */
#define	SCSI_T_COMMAND_BUFFER			0x000f
#define	SCSI_T_RXBUFF_VARIABLE			0x0010
#define	SCSI_T_RXBUFF_1_OF_2			0x0011
#define	SCSI_T_RXBUFF_2_OF_2			0x0012
#define	SCSI_T_RXBUFF_1_OF_3			0x0013
#define	SCSI_T_RXBUFF_2_OF_3			0x0014
#define	SCSI_T_RXBUFF_3_OF_3			0x0015
#define	SCSI_T_RXBUFF_1_OF_4			0x0016
#define	SCSI_T_RXBUFF_2_OF_4			0x0017
#define	SCSI_T_RXBUFF_3_OF_4			0x0018
#define	SCSI_T_RXBUFF_4_OF_4			0x0019



/* SCSI general house keeping */
/******************************/

/* To SCSI */
#define	SCSI_S_DEFINE_ID			0x0025
#define	SCSI_S_RESET_SCSI_BUS			0x0026
#define	SCSI_S_RESET_SCSI_ADAPTOR		0x0027



/* SCSI link management */
/************************/

/* To SCSI */
#define	SCSI_LINK_MANAGEMENT			0x0028



/* SCSI packet management */
/**************************/

/* To SCSI */
#define	SCSI_PM_REQUEST_RX_PACKET_SIZE		0x0029
#define	SCSI_PM_REQUEST_TX_PACKET_SIZE		0x002a

/* From SCSI */
#define	SCSI_PM_ALLOCATED_RX_PACKET_SIZE	0x001a
#define	SCSI_PM_ALLOCATED_TX_PACKET_SIZE	0x001b



/* SCSI off-TRAM scratch pad */
/*****************************/

/* To SCSI */
#define	SCSI_OTSP_REQUESTED_DATA		0x002b

/* From SCSI */
#define	SCSI_OTSP_WRITE				0x001c
#define	SCSI_OTSP_READ				0x001d


/* SCSI messages to other links */
/********************************/

/* To SCSI */
#define	SCSI_MESSAGES_TO_OTHER_LINKS		0x002c

/* From SCSI */
#define	SCSI_MESSAGE_FROM_OTHER_LINK		0x001e



/* SCSI transputer subsystem control */
/*************************************/

/* To SCSI */
#define	SCSI_SS_RESET_SUBSYSTEM			0x002d
#define	SCSI_SS_ANALYSE_SUBSYSTEM		0x002e



/* SCSI acknowledge mode */
/*************************/

/* To SCSI */
#define	SCSI_ALWAYS_ACKNOWLEDGE			0x002f
#define	SCSI_ACKNOWLEDGE_ONLY_ON_ERROR		0x0030



/* SCSI subsystem control */
/**************************/

/* To SCSI */
#define	SCSI_PERFORM_SELF_TEST			0x0031



/* SCSI vital life signs of i/o subsystem on current link */
/**********************************************************/

/* To SCSI */
#define	SCSI_LS_DEFINE_HEART_BEAT		0x0032
#define	SCSI_LS_ENABLE_HEART_BEAT		0x0033
#define	SCSI_LS_DISABLE_HEART_BEAT		0x0034

/* From SCSI */
#define	SCSI_HEART_BEAT				0x001f



/* SCSI software id */
/********************/

/* To SCSI */
#define	SCSI_SID_REPORT_VERSION_NUMBER		0x0035
#define	SCSI_SID_REPORT_VENDOR			0x0036

/* From SCSI */
#define	SCSI_SID_VERSION_NUMBER			0x0022
#define	SCSI_SID_VENDOR				0x0023



/* SCSI debug */
/**************/

/* From SCSI */
#define THE_SCSI_PHASE				0x0024


/* SCSI commands							*/
/* -------------							*/

#define	TEST_UNIT_READY		0x000
#define	REZERO_UNIT		0x001
#define	REQUEST_SENSE		0x003
#define	FORMAT_UNIT		0x004
#define	REASSIGN_BLOCKS		0x007
#define	READ			0x008
#define	WRITE			0x00a
#define	SEEK			0x00b
#define	READ_USAGE_COUNTER	0x011
#define	INQUIRY			0x012
#define	MODE_SELECT		0x015
#define	RESERVE			0x016
#define	RELEASE			0x017
#define	MODE_SENSE		0x01a
#define	START_STOP		0x01b
#define	RECIEVE_DIAG_RESULTS	0x01c
#define	SEND_DIAGNOSTIC		0x01d
#define	READ_CAPACITY		0x025
#define	READ_10			0x028
#define	WRITE_10		0x02a
#define	SEEK_10			0x02b
#define	VERIFY			0x02f
#define	READ_DEFECT_DATA	0x037
#define	WRITE_BUFFER		0x03b
#define	READ_BUFFER		0x03c
#define	READ_LONG		0x0e5
#define	WRITE_LONG		0x0e6



