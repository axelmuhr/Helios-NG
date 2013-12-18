/*
 * huntdev.c
 *
 * Hunt Engineering Helios File System Driver
 * Copyright Hunt Engineering 1990
 * TEL: (0278) 784769,	FAX : (0278) 792688.
 *
 * Revision 1.0 	Date 29-06-90	M.E
 * Revision 1.1 	Date 2-11-90	P.W
 * Revision 1.2 	Date 15-2-91	P.W  better verify and handle busy status
 * Revision 2.0         Date 7-9-92     A.S  Multiple Disk Support
 *
 * $Id: huntdev.c,v 1.1 1992/09/16 10:19:30 al Exp $
 * $Log: huntdev.c,v $
 * Revision 1.1  1992/09/16  10:19:30  al
 * Initial revision
 *
 */

#include <helios.h>
#include <syslib.h>
#include <stdlib.h>
#include <device.h>
#include <link.h>
#include <config.h>
#include <codes.h>

#include "huntcom.h"
#include "scsi.h"

#ifndef FG_Format
#define  FG_Format 0x0000a010
#endif

#define loop_retry(status) for(status=8, retry=0; \
			      (status == 8) && (retry < MAX_RETRIES); \
			      retry++) 

#define DiscOpen	0x100
#define DiscMask	0x0FF
#define DiscFixed	0x000
#define DiscRemovable	0x001
#define DiscTape	0x002
#define DiscCDRom	0x003
#define DiscFloppy	0x004

#define MAX_RETRIES	0x7FFFFFFF
#define MAX_DISCS	8
#define MAX_TFR 4096	/* max transfer length of he1000 */

/* DevDCB and DiscParm definition */
typedef struct DiscParm {
	WORD		Status;		/* Open, closed etc */
	WORD		SectorSize;	/* Size of disc sectors    */
	WORD		DeviceId;	/* SCSI ID of target disc  */
	WORD		Table;		/* HE1000 transaction table*/
	WORD		TracksPerCyl;
	WORD		SectorsPerTrack;
	WORD		Cylinders;
} DiscParm;
typedef struct DevDCB {
	DCB		DCB;		/* Generic DCB definition  */
	WORD		Link;		/* HE1000 controlling link */
	Semaphore	Lock;		/* Serializing lock	   */
	WORD		Mode;		/* SCSI id of TRAM */
	DiscParm	Disc[MAX_DISCS];/* 8 (actually 7) possibilities */
}DevDCB;


/* Helios device drivers are based on three functions */
DevDCB         *DevOpen(Device *, DiscDevInfo *);
void		DevOperate(DevDCB *, DiscReq *);
WORD		DevClose(DevDCB *);


DevDCB *DevOpen(Device *dev, DiscDevInfo * info)
{
	DevDCB	*dcb;
	int	state;

	/* Create a DevDCB for File System Server */
	if ((dcb = Malloc(sizeof(DevDCB))) == NULL) {
		IOdebug("File System driver failed to allocate work space");
		return(NULL);
	}

	/*
         * Initialise controlling link to the HE1000 
         */
	/* configure link to dumb mode */
	if ((state = link_init(info->Controller)) < 0)
		goto error;

	/* set the controlling link to initiator */
	if ((state = init_link_initiator(info->Controller, TRUE)) < 0)
		goto error;

	/* set the scsi id of this initiator */
	if ((state = set_scsi_id(info->Controller, info->Mode)) < 0)
		goto error;

	/* Initialise DevDCB with controller information */
	dcb->DCB.Device  = dev;
	dcb->DCB.Operate = DevOperate;
	dcb->DCB.Close	 = DevClose;
	dcb->Link	 = info->Controller;
	dcb->Mode        = info->Mode;
	InitSemaphore(&dcb->Lock, 1);
	for (state=0; state<MAX_DISCS; state++) 
		dcb->Disc[state].Status = 0;

#ifdef DEBUG
	IOdebug("HE1000: Devuce driver opened");
#endif
	return(dcb);

error:
	Free(dcb);
	IOdebug("File System driver failed to initialise HE1000");
	return(NULL);
}

/*
 * Normal disk operations, as well as open, close and format other disks.
 */
void DevOperate(DevDCB *dcb, DiscReq *req)
{
  BYTE			capacity_data[CAPACITY_LENGTH];
  WORD			command_status,second_status;
  WORD			block_addr, block_len;
  WORD			done = 0;
  WORD			size, retry;
  BYTE			*buf = req->Buf;
  INT			res;
  BYTE			sense_data[SENSE_LENGTH];
  FormatReq 		*freq;
  DiscParameterReq	*dpreq;
  WORD    		unit = req->DevReq.SubDevice;
  DiscParm 		*disc = &(dcb->Disc[unit]);  
  int			state;

  /* Device Driver Ready ? */
  Wait(&dcb->Lock);

#ifdef DEBUG
  IOdebug("HE1000: DevOperate 0x%x on subdevice %d",
		req->DevReq.Request & FG_Mask, unit);
#endif

  /* SubDevice must first be open, unless this is an open request */
  if ((unit < 0) || (unit > 7) || 
      (!(disc->Status & DiscOpen)) &&
       ((req->DevReq.Request & FG_Mask) != FG_Open)) {
	/* The device was not open, so an error occurs */
#ifdef DEBUG
	IOdebug("HE1000: Unit %d  Status 0x%x  Request 0x%x  EOPEN",
		unit,disc->Status,req->DevReq.Request & FG_Mask);
#endif
	goto error;
  }

  /* Select the appropriate command */
  switch (req->DevReq.Request & FG_Mask) {
  case FG_Read:
#ifdef DEBUG
	IOdebug("read sector = %d  num = %d",req->Pos / disc->SectorSize,req->Size / disc->SectorSize);
#endif
	size = req->Size;
	while (done < size) {
		WORD tfr = size - done;
		WORD position = (req->Pos / disc->SectorSize) + 
				(done/disc->SectorSize);

		if (tfr > MAX_TFR) tfr = MAX_TFR;
		loop_retry(command_status) {
#ifdef DEBUG
			  IOdebug("he1000: about to read %d");
#endif
			  res = scsi_read(dcb->Link,
					  disc->Table,
					  0,
					  position,
					  tfr,
					  disc->SectorSize,
					  buf,
					  &command_status);
#ifdef DEBUG
IOdebug("\nhe1000: read res=%d status=%d",res,command_status);
#endif
		}

		if ((res < 0) || (command_status ne 0))	{
			loop_retry(command_status) {
#ifdef DEBUG
IOdebug("he1000: about to read2");
#endif
				   res = scsi_read(dcb->Link,
						   disc->Table,
						   0,
						   position,
						   tfr,
						   disc->SectorSize,
						   buf,
						   &command_status);
#ifdef DEBUG
IOdebug("he1000: read2 res=%d status=%d",res,command_status);
#endif
			 }
		}
		done += tfr;
		buf  += tfr;
	}
	req->DevReq.Result = command_status;
	if (req->DevReq.Result eq 0)
		req->Actual = req->Size;
	else	req->Actual = 0;

	break;

  case FG_Write:
#ifdef DEBUG
	IOdebug("write   sector = %d   num = %d",req->Pos / disc->SectorSize,req->Size / disc->SectorSize);
#endif
	size = req->Size;
	while (done < size) {
		WORD tfr = size - done;
		WORD position = (req->Pos / disc->SectorSize) + 
				(done/disc->SectorSize);

		if (tfr > MAX_TFR) tfr = MAX_TFR;
		loop_retry(command_status) {
#ifdef DEBUG
IOdebug("he1000: about to write");
#endif
			  res = scsi_write(dcb->Link,
					   disc->Table,
					   0,
					   position,
					   tfr,
					   disc->SectorSize,
					   buf,
					   &command_status);
#ifdef DEBUG
IOdebug("he1000: write res=%d status=%d",res,command_status);
#endif
		}
		if ((res < 0) || (command_status ne 0)) {
			loop_retry(command_status) {
#ifdef DEBUG
IOdebug("he1000: about to write2");
#endif
				 res = scsi_write(dcb->Link,
						  disc->Table,
						  0,
						  position,
						  tfr,
						  disc->SectorSize,
						  buf,
						  &command_status);
#ifdef DEBUG
IOdebug("he1000: write2 res=%d status=%d",res,command_status);
#endif
			}
		}
		done += tfr;
		buf  += tfr;
	}

	req->DevReq.Result = command_status;
	if (req->DevReq.Result eq 0)
		req->Actual = req->Size;
	else
		req->Actual = 0;
	break;

  case FG_Open:
	/* If already open, then done */
	if (disc->Status & DiscOpen) {
		req->DevReq.Result = Err_Null;
		goto complete;
	}

	/* Allocate a table */
	if ((state = alloc_table(dcb->Link, &command_status)) < 0)
		goto error;

	/* Set the target ID of the table */
	if ((state = set_target_id(dcb->Link, command_status, unit)) < 0) {
		free_table(dcb->Link,command_status);
		goto error;
	}

	/* Initial Table Setup */
	disc->Status |= DiscOpen;
	disc->SectorSize = 512;		/* Size of disc sectors    */
	disc->DeviceId = unit;		/* SCSI ID of target disc  */
	disc->Table = command_status;	/* HE1000 transaction table*/
	disc->TracksPerCyl = 8;		/* default value */
	disc->SectorsPerTrack = 17;	/* default value */
	disc->Cylinders = 766;		/* default value */
#ifdef DEBUG
	IOdebug("HE1000: Unit %d  Status 0x%x (Open 0x%x)  Defaults set",
		unit,disc->Status,DiscOpen);
#endif

	/* Test Unit */
	scsi_start_stop(
		dcb->Link,
		disc->Table,
		0,		/* lun */
		1,		/* place head on track 0 */
		&command_status);

	/* Test disc for ready */
	loop_retry(command_status) {
#ifdef DEBUG
IOdebug("he1000: test_unit_ready");
#endif
		scsi_test_unit_ready(
				dcb->Link,
				disc->Table,
				0,			/* lun */
				&command_status);

		if (command_status eq CHECK_SENSE) {
#ifdef DEBUG
IOdebug("he1000: request_sense");
#endif
			scsi_request_sense(
				dcb->Link,
				disc->Table,
				0,
				SENSE_LENGTH,
				sense_data,
				&command_status);
		}
	}

	/* Unit is not ready */
	if (command_status ne 0) {
		free_table(dcb->Link, disc->Table);
		disc->Status &= ~DiscOpen;
		goto error;
	}

#ifdef DEBUG
IOdebug("he1000: OPEN Unit found.");
#endif

	/* Read the actual SCSI Unit size */
	loop_retry(command_status) {
		scsi_read_capacity(
			dcb->Link,
			disc->Table,
			0,		/* lun		   */
			8,		/* capacity length */
			capacity_data,
			&command_status);
	}
	if (command_status eq 0) {
		/* Got the actual data */
		disc->Cylinders = 	(capacity_data[0] * 0x1000000 +
					capacity_data[1] * 0x10000    +
					capacity_data[2] * 0x100      +
					capacity_data[3]) 
			/ (disc->SectorsPerTrack * disc->TracksPerCyl);
		
		disc->SectorSize =	capacity_data[4] * 0x1000000 +
					capacity_data[5] * 0x10000   +
					capacity_data[6] * 0x100     +
					capacity_data[7];
	}
	req->DevReq.Result = 0;
#ifdef DEBUG
	IOdebug("HE1000: Unit %d  Status 0x%x  OPEN Complete",
		unit,disc->Status);
#endif
	break;

  case FG_GetSize:
#ifdef DEBUG
  IOdebug("read capacity request");
#endif
	loop_retry(command_status) {
		scsi_read_capacity(
			dcb->Link,
			disc->Table,
			0,		/* lun		   */
			8,		/* capacity length */
			capacity_data,
			&command_status);
	  }
	  if (command_status eq 0) {
		block_addr = capacity_data[0] * 0x1000000 +
			     capacity_data[1] * 0x10000   +
			     capacity_data[2] * 0x100	  +
			     capacity_data[3];
		block_len  = capacity_data[4] * 0x1000000 +
			     capacity_data[5] * 0x10000   +
			     capacity_data[6] * 0x100	  +
			     capacity_data[7];
		req->DevReq.Result = block_addr * block_len;
	}
	break;

  case FG_Format:
	IOdebug("\rFormatting disk please wait ....");
	freq = (FormatReq *)req;

	loop_retry(command_status) {
		scsi_mode_select(
			dcb->Link,
			disc->Table,
			0,
			0,			/* format whole disk */
			disc->SectorSize,
			&command_status);
	}
	loop_retry(command_status) {
		scsi_format(
			dcb->Link,
			disc->Table,
			0,
			freq->Interleave,
			&command_status);
	}

	/*
	 * If the disk supports the busy status we need to wait until busy 
	 * goes away before giving the message that we are verifying 
	 */
	command_status = 8;
	while (command_status == 8) {
		scsi_test_unit_ready(
			dcb->Link,
			disc->Table,
			0,
			&command_status);
	}

	/*
	 * some disks will queue one command so we need to wait twice 
	 * to cope with this
	 */
	command_status = 8;
	while (command_status != 0) {
		scsi_test_unit_ready(
			dcb->Link,
			disc->Table,
			0,
			&command_status);

		scsi_request_sense(
			dcb->Link,
			disc->Table,
			0,
			SENSE_LENGTH,
			sense_data,
			&command_status);
	}

	/* And verify */
	{
		WORD	total_blocks;
		WORD	i,j;
		WORD	ten_cent,done = 0;
		BYTE	*data;

		IOdebug("\rVerifying disk please wait ....");
		data = Malloc(disc->SectorSize);
		loop_retry(command_status) {
		    res = scsi_write(dcb->Link,
				disc->Table,
				0,
				1,
				disc->SectorSize,
				disc->SectorSize,
				data,
				&command_status);
		}

		total_blocks = (disc->SectorsPerTrack * disc->TracksPerCyl * 
					disc->Cylinders);
/*			IOdebug("\rdisk size is %d blocks",total_blocks);*/
		ten_cent =  total_blocks  / 10;
		for (i = 1; i < total_blocks; i++) {
			loop_retry(command_status) {
			    res = scsi_write_quick(
					dcb->Link,
					disc->Table,
					0,
					i,
					&command_status);
			}
			loop_retry(second_status) {
				res = scsi_read_quick(
					dcb->Link,
					disc->Table,
					0,
					i,
					&second_status);
			}
			if ((i % ten_cent)  eq 0) {
				done += 10;
				IOdebug("\rVerified %d percent of disk\v",done);
			}
			if ((command_status ne 0) || (second_status ne 0)) {
				loop_retry(command_status) {
					scsi_write_quick(
						dcb->Link,
						disc->Table,
						0,
						i,
						&command_status);
				}
				loop_retry(second_status) {
					scsi_read_quick(
						dcb->Link,
						disc->Table,
						0,
						i,
						&second_status);
				}
				if ((command_status ne 0) || 
                                    (second_status ne 0)) {
					IOdebug("Verifier found bad block at %d",i);
					for (j = 0; j < 10; j++) {
						loop_retry(command_status) {
						   scsi_reassign_block(
							dcb->Link,
							disc->Table,
							0,
							i,
							&command_status);
						}
						loop_retry(command_status) {
						    scsi_write_quick(
							dcb->Link,
							disc->Table,
							0,
							i,
							&command_status);
						}
						loop_retry(second_status) {
						    scsi_read_quick(
							dcb->Link,
							disc->Table,
							0,
							i,
							&second_status);
						}
						if ((command_status eq 0) && 
						    (second_status eq 0))
							break;
						if ((command_status eq 0) && 
						    (second_status eq 0))
							IOdebug("Block %d reassigned OK",i);
						else
							IOdebug("Failed to reassign block %d",i);
					}
				}
			}
		}
		IOdebug("Verification complete                         ");
		req->DevReq.Result = 0;
	}

	break;
  case FG_Close:
#ifdef DEBUG
	IOdebug("HE1000: Closing unit %d",unit);
#endif
	/* Park the disk */
	scsi_start_stop(
		dcb->Link,
		disc->Table,
		0,
		0,		/* park drive in safe area */
		&command_status);
#ifdef DEBUG
	if (command_status ne 0)
		IOdebug("Failed to park drive");
#endif
	/* Free allocation table */
	free_table(dcb->Link,disc->Table);
	disc->Status = 0;
	req->DevReq.Result = 0;

	break;

  case FG_SetInfo:
	/* Sets the operating parameters of a disc drive */
	dpreq = (DiscParameterReq *)req;

	switch (dpreq->DriveType & DiscMask) {
	case DiscFixed:
		disc->Status = (disc->Status & ~DiscMask) | DiscFixed;
		break;
	case DiscRemovable:
		disc->Status = (disc->Status & ~DiscMask) | DiscRemovable;
		break;
	case DiscTape:
		disc->Status = (disc->Status & ~DiscMask) | DiscTape;
		break;
	case DiscFloppy:
		disc->Status = (disc->Status & ~DiscMask) | DiscFloppy;
		break;
	case DiscCDRom:
		disc->Status = (disc->Status & ~DiscMask) | DiscCDRom;
		break;
	}
	
	/* Sectorsize must be a valid log 2, 512 and above */
	for (state=512; 
	     (dpreq->SectorSize != state) && (state < 16368); /* 16K max */
	     state *=2);
	if (state == dpreq->SectorSize)
		disc->SectorSize = state;

	/* Tracks per Cylinder should be sensible (32 heads max) */
	if ((dpreq->TracksPerCyl > 0) && (dpreq->TracksPerCyl < 33))
		disc->TracksPerCyl = dpreq->TracksPerCyl;

	/* SectorsPerTrack also should be sensible */
	if ((dpreq->SectorsPerTrack > 0) && (dpreq->SectorsPerTrack < 102400))
		disc->SectorsPerTrack = dpreq->SectorsPerTrack;

	/* And cylinders ... */
	if (dpreq->Cylinders > 0)
		disc->Cylinders = dpreq->Cylinders;

	/* Adjust cylinders if SCSI says different */
	loop_retry(command_status) {
		scsi_read_capacity(
			dcb->Link,
			disc->Table,
			0,		/* lun		   */
			8,		/* capacity length */
			capacity_data,
			&command_status);
	}
	if (command_status eq 0) {
		block_addr = capacity_data[0] * 0x1000000 +
			     capacity_data[1] * 0x10000   +
			     capacity_data[2] * 0x100	  +
			     capacity_data[3] + 1; /* Include 0 block */
		state = disc->SectorsPerTrack * disc->TracksPerCyl;
		if ((state * disc->Cylinders) > block_addr)
			disc->Cylinders = block_addr / state;
	}
	req->DevReq.Result = 0;
	break;

  default:
	IOdebug("HE1000: PANIC; Unknown request 0x%x received",
		req->DevReq.Request & FG_Mask);
	req->DevReq.Result = SS_Device | EC_Error | EG_WrongFn | EO_Medium;
	break;
  }

complete:	
	Signal(&dcb->Lock);
	(*req->DevReq.Action)(req);
	return;

error:
	/* Error in open, or invalid operation */
	req->DevReq.Result = SS_Device | EC_Error | EG_Parameter | EO_Medium;
	Signal(&dcb->Lock);
	(*req->DevReq.Action)(req);
	return;
}

/*
 * Close the device and any open disc units.
 */
WORD DevClose(DevDCB *dcb)
{
	WORD	command_status;
	int i;

	Wait(&dcb->Lock);

	/* Park any open drives in a safe area */
	for (i=0; i<MAX_DISCS; i++) 
	 if (dcb->Disc[i].Status & DiscOpen) {
			/* Park the drive */
			scsi_start_stop(
				dcb->Link,
				dcb->Disc[i].Table,
				0,
				0,		/* park drive in safe area */
				&command_status);
#ifdef DEBUG
			if (command_status ne 0)
				IOdebug("Failed to park drive");
#endif
			/* Free allocation table */
			free_table(dcb->Link,dcb->Disc[i].Table);
			dcb->Disc[i].Status = 0;
	 }

	/* Set link to unused. */
	init_link_initiator(dcb->Link, FALSE);

	/* Release the link */
	link_reset(dcb->Link);

	/* Free memory */
	Free(dcb);

#ifdef DEBUG
	IOdebug("HE1000: Device Driver Closed");
#endif
	/* and ... */
	return(Err_Null);
}
