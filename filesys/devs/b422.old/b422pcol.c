 

	/****************************************************************/
	/*								*/
	/* b422pcol.c	Link protocol routines for B422 INMOS SCSI 	*/
	/*		TRAM. Copyright (C) INMOS Limited 1990.		*/
	/*								*/
	/* Date		Ver	Author		Comment			*/
	/* ----		---	------		-------			*/
	/* 22-Mar-90	1.0	Mike Burrow	Original.		*/
	/*					Intended to be simple,  */
	/*                                      later speed ups may be  */
	/* 					possible.		*/
	/*								*/
	/****************************************************************/


#include <helios.h>
#include <fcntl.h>
#include <posix.h>
#include <syslib.h>
#include <link.h>
#include <stdio.h>
#include <stdarg.h>
#include "b422cons.h"
#include "b422err.h"
#include "b422pcol.h"
#include "b422fns.h"



void output_pcol(BYTE link, ...)
{
	va_list	ap;
	BYTE	tag, *byteval;
	INT16	*int16val;
	INT32	int32val;


	va_start(ap, link);
	
	tag = va_arg(ap, BYTE);
	
	LinkOut(sizeof(tag), link, &tag, LINKTIMEOUT);
	
	switch (tag) {
	case SCSI_OPEN_CONNECTION:
	case SCSI_CLOSE_CONNECTION:
	case SCSI_BECOME_AN_INITIATOR:
	case SCSI_BECOME_A_TARGET:
	case SCSI_S_RESET_SCSI_BUS:
	case SCSI_S_RESET_SCSI_ADAPTOR:
	case SCSI_SS_RESET_SUBSYSTEM:
	case SCSI_SS_ANALYSE_SUBSYSTEM:
	case SCSI_ALWAYS_ACKNOWLEDGE:
	case SCSI_ACKNOWLEDGE_ONLY_ON_ERROR:
	case SCSI_PERFORM_SELF_TEST:
	case SCSI_LS_ENABLE_HEART_BEAT:
	case SCSI_LS_DISABLE_HEART_BEAT:
	case SCSI_SID_REPORT_VERSION_NUMBER:
	case SCSI_SID_REPORT_VENDOR:
		break;
		
	case SCSI_I_TARGET_ID:
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
		
	case SCSI_I_TX_LENGTH:
	case SCSI_I_RX_LENGTH:
		LinkOut(sizeof(INT16), link, va_arg(ap, INT16 *), LINKTIMEOUT);
		LinkOut(sizeof(INT64), link, va_arg(ap, INT64 *), LINKTIMEOUT);
		/* 
		 *	Since INT64 is really an INT32 in this implementation
		 *	we need to send 4 zero padding bytes.
		 */
		 int32val = 0;
		 LinkOut(sizeof(INT32), link, &int32val, LINKTIMEOUT);
		break;
		
	case SCSI_I_TXBUFF_1_OF_2:
		LinkOut(sizeof(INT16), link, va_arg(ap, INT16 *), LINKTIMEOUT);
		LinkOut(PART_1_OF_2, link, va_arg(ap, BYTE *), LINKTIMEOUT);
		break;
		
	case SCSI_I_TXBUFF_2_OF_2:
		LinkOut(sizeof(INT16), link, va_arg(ap, INT16 *), LINKTIMEOUT);
		LinkOut(PART_2_OF_2, link, va_arg(ap, BYTE *), LINKTIMEOUT);
		break;
		
	case SCSI_I_TXBUFF_1_OF_3:
		LinkOut(sizeof(INT16), link, va_arg(ap, INT16 *), LINKTIMEOUT);
		LinkOut(PART_1_OF_3, link, va_arg(ap, BYTE *), LINKTIMEOUT);
		break;
		
	case SCSI_I_TXBUFF_2_OF_3:
		LinkOut(sizeof(INT16), link, va_arg(ap, INT16 *), LINKTIMEOUT);
		LinkOut(PART_2_OF_3, link, va_arg(ap, BYTE *), LINKTIMEOUT);
		break;
		
	case SCSI_I_TXBUFF_3_OF_3:
		LinkOut(sizeof(INT16), link, va_arg(ap, INT16 *), LINKTIMEOUT);
		LinkOut(PART_3_OF_3, link, va_arg(ap, BYTE *), LINKTIMEOUT);
		break;
		
	case SCSI_I_TXBUFF_1_OF_4:
		LinkOut(sizeof(INT16), link, va_arg(ap, INT16 *), LINKTIMEOUT);
		LinkOut(PART_1_OF_4, link, va_arg(ap, BYTE *), LINKTIMEOUT);
		break;
		
	case SCSI_I_TXBUFF_2_OF_4:
		LinkOut(sizeof(INT16), link, va_arg(ap, INT16 *), LINKTIMEOUT);
		LinkOut(PART_2_OF_4, link, va_arg(ap, BYTE *), LINKTIMEOUT);
		break;
		
	case SCSI_I_TXBUFF_3_OF_4:
		LinkOut(sizeof(INT16), link, va_arg(ap, INT16 *), LINKTIMEOUT);
		LinkOut(PART_3_OF_4, link, va_arg(ap, BYTE *), LINKTIMEOUT);
		break;
		
	case SCSI_I_TXBUFF_4_OF_4:
		LinkOut(sizeof(INT16), link, va_arg(ap, INT16 *), LINKTIMEOUT);
		LinkOut(PART_4_OF_4, link, va_arg(ap, BYTE *), LINKTIMEOUT);
		break;
		
	case SCSI_I_BLOCK_SIZE:
		LinkOut(sizeof(BYTE), link, va_arg(ap, BYTE *), LINKTIMEOUT);
		LinkOut(sizeof(BYTE), link, va_arg(ap, BYTE *), LINKTIMEOUT);
		LinkOut(sizeof(INT16), link, va_arg(ap, INT16 *), LINKTIMEOUT);
		break;
		
	case SCSI_I_ATTRIBUTES:
	case SCSI_MESSAGES_TO_OTHER_LINKS:
		LinkOut(sizeof(BYTE), link, va_arg(ap, BYTE *), LINKTIMEOUT);
		int16val = va_arg(ap, INT16 *);
		LinkOut(sizeof(INT16), link, int16val, LINKTIMEOUT);
		LinkOut(*int16val, link, va_arg(ap, BYTE *), LINKTIMEOUT);
		break;
		
	case SCSI_T_INITIATOR_ID:
	case SCSI_T_STATUS:
	case SCSI_S_DEFINE_ID:
	case SCSI_LINK_MANAGEMENT:
		LinkOut(sizeof(BYTE), link, va_arg(ap, BYTE *), LINKTIMEOUT);
		break;
		
	case SCSI_T_DISCONNECT:
		LinkOut(sizeof(BOOL), link, va_arg(ap, BOOL *), LINKTIMEOUT);
		break;
		
	case SCSI_T_MESSAGE_BUFFER_1:
		byteval = va_arg(ap, BYTE *);
		LinkOut(sizeof(BYTE), link, byteval, LINKTIMEOUT);
		LinkOut(*byteval, link, va_arg(ap, BYTE *), LINKTIMEOUT);
		break;
		
	case SCSI_T_TXBUFF_VARIABLE:
	case SCSI_OTSP_REQUESTED_DATA:
		int16val = va_arg(ap, INT16 *);
		LinkOut(sizeof(INT16), link, int16val, LINKTIMEOUT);
		LinkOut(*int16val, link, va_arg(ap, BYTE *), LINKTIMEOUT);
		break;
		
	case SCSI_T_TXBUFF_1_OF_2:
		LinkOut(PART_1_OF_2, link, va_arg(ap, BYTE *), LINKTIMEOUT);
		break;
		
	case SCSI_T_TXBUFF_2_OF_2:
		LinkOut(PART_2_OF_2, link, va_arg(ap, BYTE *), LINKTIMEOUT);
		break;
		
	case SCSI_T_TXBUFF_1_OF_3:
		LinkOut(PART_1_OF_3, link, va_arg(ap, BYTE *), LINKTIMEOUT);
		break;
		
	case SCSI_T_TXBUFF_2_OF_3:
		LinkOut(PART_2_OF_3, link, va_arg(ap, BYTE *), LINKTIMEOUT);
		break;
		
	case SCSI_T_TXBUFF_3_OF_3:
		LinkOut(PART_3_OF_3, link, va_arg(ap, BYTE *), LINKTIMEOUT);
		break;
		
	case SCSI_T_TXBUFF_1_OF_4:
		LinkOut(PART_1_OF_4, link, va_arg(ap, BYTE *), LINKTIMEOUT);
		break;
		
	case SCSI_T_TXBUFF_2_OF_4:
		LinkOut(PART_2_OF_4, link, va_arg(ap, BYTE *), LINKTIMEOUT);
		break;
		
	case SCSI_T_TXBUFF_3_OF_4:
		LinkOut(PART_3_OF_4, link, va_arg(ap, BYTE *), LINKTIMEOUT);
		break;
		
	case SCSI_T_TXBUFF_4_OF_4:
		LinkOut(PART_4_OF_4, link, va_arg(ap, BYTE *), LINKTIMEOUT);
		break;
		
	case SCSI_I_START_IO_PROCESS:
	case SCSI_PM_REQUEST_RX_PACKET_SIZE:
	case SCSI_PM_REQUEST_TX_PACKET_SIZE:
	case SCSI_LS_DEFINE_HEART_BEAT:
		LinkOut(sizeof(INT16), link, va_arg(ap, INT16 *), LINKTIMEOUT);
		break;
		
	case SCSI_I_TXBUFF_VARIABLE:
		LinkOut(sizeof(INT16), link, va_arg(ap, INT16 *), LINKTIMEOUT);
		int16val = va_arg(ap, INT16 *);
		LinkOut(sizeof(INT16), link, int16val, LINKTIMEOUT);
		LinkOut(*int16val, link, va_arg(ap, BYTE *), LINKTIMEOUT);
		break;
		
	default:
		IOdebug("output_pcol: ** ERROR ** found default case");
		break;
	}
	va_end(ap);
}



void input_tag(BYTE link, BYTE *tag)
{
	LinkIn(sizeof(BYTE), link, tag, LINKTIMEOUT);
}

	

void input_pcol(BYTE link, BYTE tag, ...)
{
	va_list	ap;
	BYTE	*byteval;
	INT16	*int16val;


	va_start(ap, tag);
	
	switch(tag) {
	 	
	case SCSI_HEART_BEAT:
	case SCSI_SID_VERSION_NUMBER:
	case SCSI_SID_VENDOR:
		break;

	case SCSI_I_STATUS:
		LinkIn(sizeof(INT16), link, va_arg(ap, INT16 *), LINKTIMEOUT);
		LinkIn(sizeof(BYTE), link, va_arg(ap, BYTE *), LINKTIMEOUT);
		break;

	case SCSI_I_MSGBUFF:
		LinkIn(sizeof(INT16), link, va_arg(ap, INT16 *), LINKTIMEOUT);
		byteval = va_arg(ap, BYTE *);
		LinkIn(sizeof(BYTE), link, byteval, LINKTIMEOUT);
		LinkIn(*byteval, link, va_arg(ap, BYTE *), LINKTIMEOUT);
		break;

	case SCSI_I_RXBUFF_1_OF_2:
		LinkIn(sizeof(INT16), link, va_arg(ap, INT16 *), LINKTIMEOUT);
		LinkIn(PART_1_OF_2, link, va_arg(ap, BYTE *), LINKTIMEOUT);
		break;

	case SCSI_I_RXBUFF_2_OF_2:
		LinkIn(sizeof(INT16), link, va_arg(ap, INT16 *), LINKTIMEOUT);
		LinkIn(PART_2_OF_2, link, va_arg(ap, BYTE *), LINKTIMEOUT);
		break;

	case SCSI_I_RXBUFF_1_OF_3:
		LinkIn(sizeof(INT16), link, va_arg(ap, INT16 *), LINKTIMEOUT);
		LinkIn(PART_1_OF_3, link, va_arg(ap, BYTE *), LINKTIMEOUT);
		break;

	case SCSI_I_RXBUFF_2_OF_3:
		LinkIn(sizeof(INT16), link, va_arg(ap, INT16 *), LINKTIMEOUT);
		LinkIn(PART_2_OF_3, link, va_arg(ap, BYTE *), LINKTIMEOUT);
		break;

	case SCSI_I_RXBUFF_3_OF_3:
		LinkIn(sizeof(INT16), link, va_arg(ap, INT16 *), LINKTIMEOUT);
		LinkIn(PART_3_OF_3, link, va_arg(ap, BYTE *), LINKTIMEOUT);
		break;

	case SCSI_I_RXBUFF_1_OF_4:
		LinkIn(sizeof(INT16), link, va_arg(ap, INT16 *), LINKTIMEOUT);
		LinkIn(PART_1_OF_4, link, va_arg(ap, BYTE *), LINKTIMEOUT);
		break;

	case SCSI_I_RXBUFF_2_OF_4:
		LinkIn(sizeof(INT16), link, va_arg(ap, INT16 *), LINKTIMEOUT);
		LinkIn(PART_2_OF_4, link, va_arg(ap, BYTE *), LINKTIMEOUT);
		break;

	case SCSI_I_RXBUFF_3_OF_4:
		LinkIn(sizeof(INT16), link, va_arg(ap, INT16 *), LINKTIMEOUT);
		LinkIn(PART_3_OF_4, link, va_arg(ap, BYTE *), LINKTIMEOUT);
		break;

	case SCSI_I_RXBUFF_4_OF_4:
		LinkIn(sizeof(INT16), link, va_arg(ap, INT16 *), LINKTIMEOUT);
		LinkIn(PART_4_OF_4, link, va_arg(ap, BYTE *), LINKTIMEOUT);
		break;

	case SCSI_MESSAGE_FROM_OTHER_LINK:
		LinkIn(sizeof(BYTE), link, va_arg(ap, BYTE *), LINKTIMEOUT);
		int16val = va_arg(ap, INT16 *);
		LinkIn(sizeof(INT16), link, int16val, LINKTIMEOUT);
		LinkIn(*int16val, link, va_arg(ap, BYTE *), LINKTIMEOUT);
		break;

	case SCSI_T_TARGET_ID:
	case THE_SCSI_PHASE:
		LinkIn(sizeof(BYTE), link, va_arg(ap, BYTE *), LINKTIMEOUT);
		break;

	case SCSI_T_MESSAGE_BUFFER_2:
	case SCSI_T_COMMAND_BUFFER:
		byteval = va_arg(ap, BYTE *);
		LinkIn(sizeof(BYTE), link, byteval, LINKTIMEOUT);
		LinkIn(*byteval, link, va_arg(ap, BYTE *), LINKTIMEOUT);
		break;

	case SCSI_T_RXBUFF_VARIABLE:
		int16val = va_arg(ap, INT16 *);
		LinkIn(sizeof(INT16), link, int16val, LINKTIMEOUT);
		LinkIn(*int16val, link, va_arg(ap, BYTE *), LINKTIMEOUT);
		break;

	case SCSI_T_RXBUFF_1_OF_2:
		LinkIn(PART_1_OF_2, link, va_arg(ap, BYTE *), LINKTIMEOUT);
		break;

	case SCSI_T_RXBUFF_2_OF_2:
		LinkIn(PART_2_OF_2, link, va_arg(ap, BYTE *), LINKTIMEOUT);
		break;

	case SCSI_T_RXBUFF_1_OF_3:
		LinkIn(PART_1_OF_3, link, va_arg(ap, BYTE *), LINKTIMEOUT);
		break;

	case SCSI_T_RXBUFF_2_OF_3:
		LinkIn(PART_2_OF_3, link, va_arg(ap, BYTE *), LINKTIMEOUT);
		break;

	case SCSI_T_RXBUFF_3_OF_3:
		LinkIn(PART_3_OF_3, link, va_arg(ap, BYTE *), LINKTIMEOUT);
		break;

	case SCSI_T_RXBUFF_1_OF_4:
		LinkIn(PART_1_OF_4, link, va_arg(ap, BYTE *), LINKTIMEOUT);
		break;

	case SCSI_T_RXBUFF_2_OF_4:
		LinkIn(PART_2_OF_4, link, va_arg(ap, BYTE *), LINKTIMEOUT);
		break;

	case SCSI_T_RXBUFF_3_OF_4:
		LinkIn(PART_3_OF_4, link, va_arg(ap, BYTE *), LINKTIMEOUT);
		break;

	case SCSI_T_RXBUFF_4_OF_4:
		LinkIn(PART_4_OF_4, link, va_arg(ap, BYTE *), LINKTIMEOUT);
		break;

	case SCSI_PM_ALLOCATED_RX_PACKET_SIZE:
	case SCSI_PM_ALLOCATED_TX_PACKET_SIZE:
		LinkIn(sizeof(INT16), link, va_arg(ap, INT16 *), LINKTIMEOUT);
		break;

	case SCSI_OTSP_WRITE:
		LinkIn(sizeof(INT32), link, va_arg(ap, INT32 *), LINKTIMEOUT);
		int16val = va_arg(ap, INT16 *);
		LinkIn(sizeof(INT16), link, int16val, LINKTIMEOUT);
		LinkIn(*int16val, link, va_arg(ap, BYTE *), LINKTIMEOUT);
		break;

	case SCSI_OTSP_READ:
		LinkIn(sizeof(INT32), link, va_arg(ap, INT32 *), LINKTIMEOUT);
		LinkIn(sizeof(INT16), link, va_arg(ap, INT16 *), LINKTIMEOUT);
		break;

	case SCSI_I_TX_SEND_PACKET:
	case SCSI_I_EXECUTION_STATUS:
	case SCSI_I_CURRENT_EXECUTION_STATUS:
		LinkIn(sizeof(INT16), link, va_arg(ap, INT16 *), LINKTIMEOUT);
		LinkIn(sizeof(INT16), link, va_arg(ap, INT16 *), LINKTIMEOUT);
		break;

	case SCSI_I_RXBUFF_VARIABLE:
		LinkIn(sizeof(INT16), link, va_arg(ap, INT16 *), LINKTIMEOUT);
		int16val = va_arg(ap, INT16 *);
		LinkIn(sizeof(INT16), link, int16val, LINKTIMEOUT);
		LinkIn(*int16val, link, va_arg(ap, BYTE *), LINKTIMEOUT);
		break;

	default:
		IOdebug("input_pcol: ** ERROR ** found default case");
		break;
	}
	va_end(ap);
}


int  b422_init(WORD link)
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
	return(squirt("/helios/lib/b422.b2u", link));
}


void b422_reset(WORD link, BYTE b422id)
{
	output_pcol(link, SCSI_BECOME_AN_INITIATOR);
	output_pcol(link, SCSI_S_RESET_SCSI_ADAPTOR);
	output_pcol(link, SCSI_S_DEFINE_ID, &b422id);
	output_pcol(link, SCSI_S_RESET_SCSI_BUS);
}


void b422_term(WORD link)
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
{
	return((i >> (8 * (b - 1))) & 0xff);
}



