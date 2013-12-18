

	/****************************************************************/
	/*								*/
	/* b422err.h	Error messages transmitted from the B422 INMOS  */
	/*		SCSI TRAM. Copyright (C) INMOS Limited 1990.	*/
	/*								*/
	/* Date		Ver	Author		Comment			*/
	/* ----		---	------		-------			*/
	/* 19-Mar-90	1.0	Mike Burrow	Original		*/
	/*								*/
	/****************************************************************/


#define	SCSI_E_GOOD						0x0000
#define	SCSI_E_BAD_SCSI_STATUS					0x0001
#define	SCSI_E_COMMAND_NOT_COMPLETE				0x0002
#define	SCSI_E_LINKS_HAVE_ALREADY_BEEN_DEFINED			0x0003
#define	SCSI_E_INCORRECT_IO_PROCESS_NUMBER			0x0004
#define	SCSI_E_SCSI_BUS_PARITY_ERROR				0x0005
#define	SCSI_E_DATA_IN_FAILURE					0x0006
#define	SCSI_E_BAD_DATA_IN_RETURNED_PACKET_SIZE			0x0007
#define	SCSI_E_INCORRECT_IO_PNO_DURING_DATA_IN			0x0008
#define	SCSI_E_DATA_OUT_FAILURE					0x0009
#define	SCSI_E_BAD_DATA_OUT_RETURNED_PACKET_SIZE		0x000a
#define	SCSI_E_INCORRECT_IO_PNO_DURING_DATA_OUT			0x000b
#define	SCSI_E_DATA_IN_TIME_OUT					0x000c
#define	SCSI_E_DATA_OUT_TIME_OUT				0x000d
#define	SCSI_E_COMMAND_TIME_OUT					0x000e
#define	SCSI_E_STATUS_TIME_OUT					0x000f
#define	SCSI_E_CONTROLLER_RESET_TIME_OUT			0x0010
#define	SCSI_E_BUS_RESET_TIME_OUT				0x0011
#define	SCSI_E_RESELLECTED					0x0012
#define	SCSI_E_FUNCTION_NOT_COMPLETE				0x0013
#define	SCSI_E_DISCONNECTED					0x0014
#define	SCSI_E_NOT_BUS_SERVICE					0x0015
#define	SCSI_E_SEQUENCE_NOT_COMPLETE				0x0016
#define	SCSI_E_PREMATURE_PHASE_CHANGE				0x0017
#define	SCSI_E_TARGET_DID_NOT_ASSERT_ATN			0x0018
#define	SCSI_E_TARGET_DID_NOT_ASSERT_MESSAGE_OUT		0x0019
#define	SCSI_E_SELECTION_TIME_OUT				0x001a
#define	SCSI_E_UNDOCUMENTED_ERROR				0x001b
#define	SCSI_E_DATA_IN_ABBORT					0x001c
#define	SCSI_E_DATA_OUT_ABBORT					0x001d
#define	SCSI_E_UNEXPECTED_PHASE_CHANGE				0x001e

