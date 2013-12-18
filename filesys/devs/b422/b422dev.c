/*
 * Copyright (c) INMOS Limited 1990
 *
 * Description	: Helios File system driver for the WITCH IMS B422 SCSI TRAM.
 *
 * Date		: 16-Nov-90
 *
 * Filename	: b422dev.c
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
#include <stdlib.h>
#include <device.h>
#include <link.h>
#include <config.h>
#include <codes.h>
#include "b422def.h"
#include "b422fns.h"


/* DiscDCB definition */
typedef	struct DiscDCB {
	DCB		DCB;		/* Standard DCB			*/
	word		Link;		/* B422 link			*/
	Semaphore	Lock;		/* Serializing lock		*/
	word		BlockSize;	/* Unit for read/write requests */
	word		SectorSize;	/* Size of disc sectors		*/
	word		DeviceId;	/* Disc SCSI id			*/
} DiscDCB;



/* Function definitions */
DiscDCB	*DevOpen(Device *, DiscDevInfo *);
void	DevOperate(DiscDCB *, DiscReq *);
word 	DevClose(DiscDCB *);



DiscDCB *DevOpen(Device *dev, DiscDevInfo * info)

{
	WORD		res;	
	DiscDCB		*dcb;	
	DriveInfo	*dvi;	
	BYTE		message[MAX_SCSI_MESSAGE_SIZE];
	BYTE		sense_data[MAX_SCSI_MESSAGE_SIZE];
	BYTE		msglen, result;
	INT16		exestat;


	/* Allocate space for DiscDCB - if none return NULL */
	if ((dcb = Malloc(sizeof(DiscDCB))) == NULL) {
		IOdebug("devopen: ** ERROR ** Cannot Malloc(DiscDCB)");
		return(NULL);
	}

	/* Initialise link to the SCSI TRAM */
	if ((res = b422_init(info->Controller, TRUE)) != TRUE)
		return(NULL);

	/* Get a pointer to the drive information */
	dvi = (DriveInfo *)RTOA(info->Drives);
	
	/* Set up the dcb */
	dcb->DeviceId    = dvi->DriveId;
	dcb->SectorSize  = dvi->SectorSize;
	dcb->DCB.Device  = dev;
	dcb->DCB.Operate = DevOperate;
	dcb->DCB.Close   = DevClose;
	dcb->Link        = info->Controller;
	dcb->BlockSize   = info->Addressing;

	b422_reset(
		dcb->Link, 
		ENABLE_PARITY_CHECKING,
		ENABLE_PARITY_GENERATION,
		/* FAST_CABLE_MODE, */
		SLOW_CABLE_MODE,
		DEFAULT_DATA_PHASE_TIME_OUT,
		DEFAULT_INTERRUPT_TIME_OUT,
		B422_SCSI_ID,
		/* SCSI_FAST_DMA_MODE); */
		SCSI_SLOW_DMA_MODE);
	
	/* Wait until the unit is ready */
	do { 
		msglen = 0;
		scsi_request_sense_6(
			dcb->Link, 
			dcb->Link,
			dcb->DeviceId,
			0,
			REQUEST_SENSE_DATA_LENGTH,
			0,
			sense_data,
			message,
			&msglen,
			&result,
			&exestat);

		msglen = 0;
		scsi_test_unit_ready_6(
			dcb->Link, 
			dcb->Link,
			dcb->DeviceId,
			0,
			0,
			message,
			&msglen,
			&result,
			&exestat);

	} while (exestat != 0);
	
	/* Initialise the semaphore to ensure controlled access*/
	InitSemaphore(&dcb->Lock, 1);
	
	/* Return the dcb */
	return(dcb);
}




void DevOperate(DiscDCB * dcb, DiscReq *req)

{
	BYTE		message[MAX_SCSI_MESSAGE_SIZE];
	BYTE		capdata[CAPACITY_DATA_LENGTH];
	BYTE		msglen, result;
	INT16		exestat;
	INT32		logical_block_address, block_length;
	FormatReq	*freq;

	/* Wait here until unlocked */
	Wait(&dcb->Lock);
		
	/* Select command */
	switch (req->DevReq.Request & FG_Mask) {
		
	case FG_Read:
	
		msglen = 0;
		scsi_read_10(
			dcb->Link,
			dcb->Link,
			dcb->DeviceId,
			0,
			req->Pos / dcb->SectorSize,
			req->Size / dcb->SectorSize,
			dcb->SectorSize,
			0,
			req->Buf,
			message,
			&msglen,
			&result,
			&exestat);

		req->DevReq.Result = exestat;

		if (req->DevReq.Result == 0)
			req->Actual = req->Size;
		else
			req->Actual = 0;
			
		break;
			
	case FG_Write:

		msglen = 0;
		scsi_write_10(
			dcb->Link,
			dcb->Link,
			dcb->DeviceId,
			0,
			req->Pos / dcb->SectorSize,
			req->Size / dcb->SectorSize,
			dcb->SectorSize,
			0,
			req->Buf,
			message,
			&msglen,
			&result,
			&exestat);

		req->DevReq.Result = exestat;

		if (req->DevReq.Result == 0)
			req->Actual = req->Size;
		else
			req->Actual = 0;
			
		break;

	case FG_GetSize:
	
		msglen = 0;
		scsi_read_capacity_10(
			dcb->Link,
			dcb->Link,
			dcb->DeviceId,
			0,
			0,
			FALSE,
			0,
			0,
			0,
			0,
			0,
			0,
			capdata,
			message,
			&msglen,
			&result,
			&exestat);
		
		if (exestat == 0) {
			logical_block_address = capdata[0] * 0x1000000 +
			      			capdata[1] * 0x10000   +
			      			capdata[2] * 0x100     +
			      			capdata[3];
			      
			block_length  = capdata[4] * 0x1000000 +
			      		capdata[5] * 0x10000   +
			      		capdata[6] * 0x100     +
			      		capdata[7];
			      
			req->DevReq.Result = logical_block_address * block_length;
		}

		break;
		

	case FG_Format:

		freq = (FormatReq *)req;
		
		msglen = 0;
		scsi_format_6(
			dcb->Link,
			dcb->Link,
			dcb->DeviceId,
			0,
			0,
			0,
			0,
			0,
			freq->Interleave,
			0,
			0,
			0,
			message,
			&msglen,
			&result,
			&exestat);
					
		req->DevReq.Result = exestat;

		break;

 
	default:

		/* Just to be safe trap anything else */
		req->DevReq.Result = (word) SS_Device | EC_Error | EG_WrongFn | EO_Medium;
		break;
	} 
	
	/* Indicate that we have finished */
	Signal(&dcb->Lock);
	
	/* Client action */
	(*req->DevReq.Action)(req);
	
	return;
	
}

	   


WORD DevClose(DiscDCB *dcb)
{

	/* Wait here until unlocked */
	Wait(&dcb->Lock);
	
	/* Deallocate the link to the TRAM and tidy up */
	b422_term(dcb->Link);
	
	/* Return success */

	return(Err_Null);
}

