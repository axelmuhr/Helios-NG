/*
 * Copyright (c) INMOS Limited 1990
 *
 * Description	: Link handling for the WITCH IMS B422 SCSI TRAM
 *                driver.
 *
 * Date		: 06-Nov-90
 *
 * Filename	: b422lh.c
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
#include <fcntl.h>
#include <posix.h>
#include <syslib.h>
#include <link.h>
#include <stdio.h>
#include <stdarg.h>
#include "b422def.h"
#include "b422fns.h"


void output_pcol(BYTE link, ...)
/*
 * Output a B422 protocol to the specified link. The format of the 
 * input parameters is:
 * output_link, protocol_tag, [*parameter_1,] [*parameter_2,] ...
 * The tag is used to extract the parameters from the argument list.
 */
{
	va_list	ap;
	BYTE	tag, *byteval;
	INT16	*int16val;

	va_start(ap, link);
	
	tag = va_arg(ap, BYTE);
	
	LinkOut(sizeof(tag), link, &tag, LINKTIMEOUT);
	
	switch (tag) {
	case SCSI_BECOME_AN_INITIATOR:
	case SCSI_BECOME_TARGET:
	case SCSI_T_AWAIT_SELECTION:
	case SCSI_T_DISCONNECT:		
	case SCSI_T_MESSAGE_OUT:
	case SCSI_S_RESET_SCSI_BUS:
	case SCSI_SS_RESET_SUBSYSTEM:
	case SCSI_SS_ANALYSE_SUBSYSTEM:
	case SCSI_SS_READ_SUBSYSTEM_NOTERROR:
	case SCSI_PERFORM_SELF_TEST:
	case SCSI_SID_REPORT_VERSION_NUMBER:
	case SCSI_SID_REPORT_VENDOR:
		break;

	case SCSI_I_TARGET_ID:
	case SCSI_I_LUN:
	case SCSI_I_DIRECTION:
		LinkOut(sizeof(INT16), link, va_arg(ap, INT16 *), LINKTIMEOUT);
		LinkOut(sizeof(BYTE), link, va_arg(ap, BYTE *), LINKTIMEOUT);
		break;

	case SCSI_I_MESSAGE_BUFFER:
	case SCSI_I_COMMAND_BUFFER:
		LinkOut(sizeof(INT16), link, va_arg(ap, INT16 *), LINKTIMEOUT);
		byteval = va_arg(ap, BYTE *);
		LinkOut(sizeof(BYTE), link, byteval, LINKTIMEOUT);
		LinkOut(*byteval, link, va_arg(ap, BYTE *), LINKTIMEOUT);
		break;

	case SCSI_I_START_IO_PROCESS:
	case SCSI_DATA_TRANSFER_MODE:
	case SCSI_PM_REQUEST_PACKET_SIZE:
	case SCSI_E_REQUEST_EXECUTION_STATUS:
	case SCSI_WD_DATA_PHASE_TIME_OUT:
	case SCSI_WD_INTERRUPT_TIME_OUT:
		LinkOut(sizeof(INT16), link, va_arg(ap, INT16 *), LINKTIMEOUT);
		break;

	case SCSI_I_TX_LENGTH:
	case SCSI_I_RX_LENGTH:
		LinkOut(sizeof(INT16), link, va_arg(ap, INT16 *), LINKTIMEOUT);
		LinkOut(sizeof(INT32), link, va_arg(ap, INT32 *), LINKTIMEOUT);
		break;

	case SCSI_I_TX_DATA:
		LinkOut(sizeof(INT16), link, va_arg(ap, INT16 *), LINKTIMEOUT);
		int16val = va_arg(ap, INT16 *);
		LinkOut(sizeof(INT16), link, int16val, LINKTIMEOUT);
		LinkOut(*int16val, link, va_arg(ap, BYTE *), LINKTIMEOUT);
		break;

	case SCSI_T_RECONNECT:
	case SCSI_T_STATUS:
	case SCSI_S_DEFINE_ID:
	case SCSI_LINK_MANAGEMENT:
		LinkOut(sizeof(BYTE), link, va_arg(ap, BYTE *), LINKTIMEOUT);
		break;
		
	case SCSI_T_DATA_IN:
	case SCSI_T_DATA_OUT:
		LinkOut(sizeof(INT32), link, va_arg(ap, INT32 *), LINKTIMEOUT);
		break;
	
	case SCSI_T_TX_DATA:
	case GENERIC_INPUT:
		int16val = va_arg(ap, INT16 *);
		LinkOut(sizeof(INT16), link, int16val, LINKTIMEOUT);
		LinkOut(*int16val, link, va_arg(ap, BYTE *), LINKTIMEOUT);
		break;

	case SCSI_T_MESSAGE_IN:
		byteval = va_arg(ap, BYTE *);
		LinkOut(sizeof(BYTE), link, byteval, LINKTIMEOUT);
		LinkOut(*byteval, link, va_arg(ap, BYTE *), LINKTIMEOUT);
		break;

	case SCSI_T_STATUS_AND_MESSAGE_IN:
		LinkOut(sizeof(BYTE), link, va_arg(ap, BYTE *), LINKTIMEOUT);
		LinkOut(sizeof(BYTE), link, va_arg(ap, BYTE *), LINKTIMEOUT);
		byteval = va_arg(ap, BYTE *);
		LinkOut(sizeof(BYTE), link, byteval, LINKTIMEOUT);
		LinkOut(*byteval, link, va_arg(ap, BYTE *), LINKTIMEOUT);
		break;

	case SCSI_S_RESET_SCSI_ADAPTOR:
		LinkOut(sizeof(BYTE), link, va_arg(ap, BYTE *), LINKTIMEOUT);
		LinkOut(sizeof(BYTE), link, va_arg(ap, BYTE *), LINKTIMEOUT);
		LinkOut(sizeof(BYTE), link, va_arg(ap, BYTE *), LINKTIMEOUT);
		break;

	case SCSI_MESSAGE_TO_OTHER_LINK:
		LinkOut(sizeof(BYTE), link, va_arg(ap, BYTE *), LINKTIMEOUT);
		int16val = va_arg(ap, INT16 *);
		LinkOut(sizeof(INT16), link, int16val, LINKTIMEOUT);
		LinkOut(*int16val, link, va_arg(ap, BYTE *), LINKTIMEOUT);
		break;

	default:
		IOdebug("output_pcol: **ERROR** Unknown protocol tag");
		break;
	}
	va_end(ap);
}



void input_tag(BYTE link, BYTE *tag)
/*
 * Input a B422 protocol tag from the specified link.
 */
{
	LinkIn(sizeof(BYTE), link, tag, LINKTIMEOUT);
}



void input_pcol(BYTE link, BYTE tag, ...)
/*
 * Input the B422 protocol associated with the given tag from the specified
 * link. The format of the input parameters is:
 * output_link, protocol_tag, [*parameter_1,] [*parameter_2,] ...
 * The tag is used to extract the parameters from the argument list.
 */
{
	va_list	ap;
	BYTE	*byteval;
	INT16	*int16val;

	va_start(ap, tag);
	
	switch(tag) {
	 	
	case SCSI_I_TX_SEND_PACKET:
		LinkIn(sizeof(INT16), link, va_arg(ap, INT16 *), LINKTIMEOUT);
		LinkIn(sizeof(INT32), link, va_arg(ap, INT32 *), LINKTIMEOUT);
		LinkIn(sizeof(INT16), link, va_arg(ap, INT16 *), LINKTIMEOUT);
		break;

	case SCSI_I_RX_DATA:
		LinkIn(sizeof(INT16), link, va_arg(ap, INT16 *), LINKTIMEOUT);
		LinkIn(sizeof(INT32), link, va_arg(ap, INT32 *), LINKTIMEOUT);
		int16val = va_arg(ap, INT16 *);
		LinkIn(sizeof(INT16), link, int16val, LINKTIMEOUT);
		LinkIn(*int16val, link, va_arg(ap, BYTE *), LINKTIMEOUT);
		break;

	case SCSI_I_MSGBUFF:
		LinkIn(sizeof(INT16), link, va_arg(ap, INT16 *), LINKTIMEOUT);
		byteval = va_arg(ap, BYTE *);
		LinkIn(sizeof(BYTE), link, byteval, LINKTIMEOUT);
		LinkIn(*byteval, link, va_arg(ap, BYTE *), LINKTIMEOUT);
		break;

	case SCSI_I_STATUS:
		LinkIn(sizeof(INT16), link, va_arg(ap, INT16 *), LINKTIMEOUT);
		LinkIn(sizeof(BYTE), link, va_arg(ap, BYTE *), LINKTIMEOUT);
		break;

	case SCSI_T_COMMAND_BUFFER:
		LinkIn(sizeof(BYTE), link, va_arg(ap, BYTE *), LINKTIMEOUT);
		LinkIn(sizeof(BYTE), link, va_arg(ap, BYTE *), LINKTIMEOUT);
		byteval = va_arg(ap, BYTE *);
		LinkIn(sizeof(BYTE), link, byteval, LINKTIMEOUT);
  		LinkIn(*byteval, link, va_arg(ap, BYTE *), LINKTIMEOUT);
 		break;

	case SCSI_T_MSG_OUT_DATA:
	case SCSI_SID_VENDOR:
		byteval = va_arg(ap, BYTE *);
		LinkIn(sizeof(BYTE), link, byteval, LINKTIMEOUT);
		LinkIn(*byteval, link, va_arg(ap, BYTE *), LINKTIMEOUT);
		break;
	
	case SCSI_T_RX_DATA:
	case SCSI_GENERIC_OUTPUT:
		int16val = va_arg(ap, INT16 *);
		LinkIn(sizeof(INT16), link, int16val, LINKTIMEOUT);
		LinkIn(*int16val, link, va_arg(ap, BYTE *), LINKTIMEOUT);
		break;
			
	case SCSI_PM_ALLOCATED_PACKET_SIZE:
	case SCSI_DL_END_DOUBLE_LINK:
	case SCSI_SID_VERSION_NUMBER:
		LinkIn(sizeof(INT16), link, va_arg(ap, INT16 *), LINKTIMEOUT);
		break;
		
	case SCSI_MESSAGE_FROM_OTHER_LINK:
		LinkIn(sizeof(BYTE), link, va_arg(ap, BYTE *), LINKTIMEOUT);
		int16val = va_arg(ap, INT16 *);
		LinkIn(sizeof(INT16), link, int16val, LINKTIMEOUT);
		LinkIn(*int16val, link, va_arg(ap, BYTE *), LINKTIMEOUT);
		break;

	case SCSI_E_EXECUTION_STATUS:
		LinkIn(sizeof(INT16), link, va_arg(ap, INT16 *), LINKTIMEOUT);
		LinkIn(sizeof(INT16), link, va_arg(ap, INT16 *), LINKTIMEOUT);
		break;

	case SCSI_SS_SUBSYSTEM_NOTERROR:
		LinkIn(sizeof(BYTE), link, va_arg(ap, BYTE *), LINKTIMEOUT);
		break;

	default:
		IOdebug("input_pcol: **ERROR** Unknown protocol tag");
		break;
	}
	va_end(ap);
}



int  b422_init(
	WORD link, 
	BOOL download)
/*
 * Reconfigure the specified link to be dumb, allocate it, and 
 * download the B422 driver image if required.
 */
{
	word		status;
	LinkInfo	linfo;
	LinkConf	lconf;

	/* Find out existing link information */
	if ((status = LinkData(link, &linfo)) < 0) 
		return(status);
	
	/* Copy link info other than mode which is set to 'dumb' */
	lconf.Flags = linfo.Flags;
	lconf.Mode  = Link_Mode_Dumb;
	lconf.State = linfo.State;
	lconf.Id    = linfo.Id;
	
	/* Reconfigure the link in 'dumb' mode so it can be allocated */
	if ((status = Configure(lconf)) < 0)
		return(status);
	
	/* Allocate the link for use */
	if ((status = AllocLink(link)) < 0) 
		return(status);
	
	/* Squirt the B422 software down the link */
	if (download)
		return(squirt("/helios/lib/b422dvr.b2u", link));
	else
		return(TRUE);
}



void b422_reset(
	WORD	link,
	BYTE	parity_generation,
	BYTE	parity_checking,
	BYTE	cable_mode,
	INT16	data_phase_time_out,
	INT16	interrupt_time_out,
	BYTE	initiator_id,
	INT16	data_transfer_mode)
/*
 * Reset the b422 driver attached to the link.
 */
{
	BYTE	tag;
	INT16	io_pno, execution_status;
	
	output_pcol(link, SCSI_S_RESET_SCSI_BUS);
	input_tag(link, &tag);
	input_pcol(link, tag, &io_pno, &execution_status);

	output_pcol(link, SCSI_S_RESET_SCSI_ADAPTOR, &parity_generation,
		    &parity_checking, &cable_mode);
	input_tag(link, &tag);
	input_pcol(link, tag, &io_pno, &execution_status);
	
	output_pcol(link, SCSI_WD_DATA_PHASE_TIME_OUT, &data_phase_time_out);
	input_tag(link, &tag);
	input_pcol(link, tag, &io_pno, &execution_status);
	
	output_pcol(link, SCSI_WD_INTERRUPT_TIME_OUT, &interrupt_time_out);
	input_tag(link, &tag);
	input_pcol(link, tag, &io_pno, &execution_status);
	
	Delay(OneSec);
	
	output_pcol(link, SCSI_BECOME_AN_INITIATOR);
	input_tag(link, &tag);
	input_pcol(link, tag, &io_pno, &execution_status);
	
	output_pcol(link, SCSI_S_DEFINE_ID, &initiator_id);
	input_tag(link, &tag);
	input_pcol(link, tag, &io_pno, &execution_status);
	
	output_pcol(link, SCSI_DATA_TRANSFER_MODE, &data_transfer_mode);
	input_tag(link, &tag);
	input_pcol(link, tag, &io_pno, &execution_status);
}



void b422_term(WORD link)
/*
 * Terminate the driver, and release the link.
 */
{
	LinkInfo	linfo;
	LinkConf	lconf;
	word		status;

	/* Release the link and heap space */	
	status = FreeLink(link);

	/* Find out existing link information */
	if ((status = LinkData(link, &linfo)) >= 0) {

		/* Copy link info */
		lconf.Flags = linfo.Flags;
		lconf.Mode  = Link_Mode_Intelligent;
		lconf.State = linfo.State;
		lconf.Id    = linfo.Id;
	
		/* Reconfigure the link in 'intelligent' mode */
		Configure(lconf);
	}
}


WORD squirt(BYTE *filename, WORD link)
/* 
 *	'squirt' the B422 TRAM driver code down 
 *	the specified link. This will then execute
 *	and expect us to be an i-server capable of
 *	giving it's board size.
 */
{
	BYTE		data[MAX_SCSI_MESSAGE_SIZE], s[10];
	Stream		*fid;
	WORD		n;
	INT16		i16;
	Object		*context;

	context = Locate(NULL, "/");

	if ((fid = Open(context, filename, O_RDONLY)) == NULL) 
		return(FALSE);

	Close(context);

	while((n = Read(fid, data, MAX_SCSI_MESSAGE_SIZE,-1)) > 0)
		LinkOut(n, link, data, LINKTIMEOUT);

	Close(fid);
	
	/* It thinks we're an i-server, so fool it! */
	LinkIn(sizeof(i16), link, &i16, LINKTIMEOUT);
	LinkIn(i16, link, data, LINKTIMEOUT);

	/* Magic! */
	s[0] = 9; s[1] = 0; s[2] = 0; s[3] = 6;	s[4] = 0;
	s[5] = '#'; s[6] = '1'; s[7] = s[8] = s[9] = s[10] = '0';

	/* make the B422 a happy TRAM */
	LinkOut(11, link, s, LINKTIMEOUT);

	return(TRUE);

}



BYTE getbyte(INT32 i, BYTE b)
/*
 * Extract a byte from an INT32.
 */
{
	return((i >> (8 * (b - 1))) & 0xff);
}




