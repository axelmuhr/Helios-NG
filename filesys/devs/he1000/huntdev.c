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
 */

#include <helios.h>
#include <syslib.h>
#include <stdlib.h>
#include <device.h>
#include <link.h>
#include <config.h>
#include <codes.h>

#include "huntcom.h"
#define  eq ==
#define  ne !=
#ifndef FG_Format
#define  FG_Format 0x0000a010
#endif

#define MAX_TFR 4096	/* max transfer length of he1000 */

/* DiscDCB definition */
typedef struct DiscDCB {
	DCB		DCB;		/* Generic DCB definition  */
	WORD		Link;		/* HE1000 controlling link */
	Semaphore	Lock;		/* Serializing lock	   */
	WORD		BlockSize;	/* Size of disc block	   */
	WORD		SectorSize;	/* Size of disc sectors    */
	WORD		DeviceId;	/* SCSI ID of target disc  */
	WORD		Table;		/* HE1000 transaction table*/
	WORD		TracksPerCyl;
	WORD		SectorsPerTrack;
	WORD		Cylinders;
	WORD		Mode;		/* SCSI id of TRAM */
}DiscDCB;


/* Helios device drivers are based on three functions */
DiscDCB        *DevOpen(Device *, DiscDevInfo *);
void		DevOperate(DiscDCB *, DiscReq *);
WORD		DevClose(DiscDCB *);


DiscDCB *DevOpen(Device *dev, DiscDevInfo * info)
{
	DiscDCB 	*dcb;
	DriveInfo	*dvi;
	WORD		command_status;
	BYTE sense_data[SENSE_LENGTH];

	/* Create a DiscDCB for File System Server */
	if ((dcb = Malloc(sizeof(DiscDCB))) == NULL)
	{
		IOdebug("File System driver failed to allocate work space");
		return(NULL);
	}

	dvi = (DriveInfo *)RTOA(info->Drives);

	/* Initialise controlling link to the HE1000 */
	if(( dcb->Table = HE1000_init( info->Controller, dvi->DriveId , info->Mode)) < 0)
	{
		IOdebug("File System driver failed to initialise HE1000");
		return(NULL);
	}


	/* Initialise DiscDCB with drive information */
	dcb->DeviceId	 = dvi->DriveId;
	dcb->SectorSize  = dvi->SectorSize;
	dcb->DCB.Device  = dev;
	dcb->DCB.Operate = DevOperate;
	dcb->DCB.Close	 = DevClose;
	dcb->Link	 = info->Controller;
	dcb->BlockSize	 = info->Addressing;
	dcb->SectorsPerTrack	= dvi->SectorsPerTrack;
	dcb->TracksPerCyl	= dvi->TracksPerCyl;
	dcb->Cylinders		= dvi->Cylinders;
	dcb->Mode		= info->Mode;

	scsi_start_stop(
		dcb->Link,
		dcb->Table,
		dcb->DeviceId,
		0,
		1,		/* place head on track 0 */
		&command_status);

	/* Test disc for ready */
	do
	{
		scsi_test_unit_ready(
				dcb->Link,
				dcb->Table,
				dcb->DeviceId,
				0,
				&command_status);

		if( command_status eq CHECK_SENSE )
		{
			scsi_request_sense(
				dcb->Link,
				dcb->Table,
				dcb->DeviceId,
				0,
				SENSE_LENGTH,
				sense_data,
				&command_status);
		}
	} while ( command_status ne 0 );

	/* Initialise the access locking semaphore */
	InitSemaphore(&dcb->Lock, 1);
	/* return DiscDCB */
	return(dcb);
}


void DevOperate(DiscDCB *dcb, DiscReq *req)
{
	BYTE	  capacity_data[CAPACITY_LENGTH];
	WORD	  command_status,second_status;
	WORD	  block_addr, block_len;
	WORD	  done = 0;
	WORD	  size;
	BYTE	  *buf = req->Buf;
	INT	  res;
	BYTE sense_data[SENSE_LENGTH];

	FormatReq *freq;

	Wait(&dcb->Lock);

	/* Select the appropriate command */
	switch( req->DevReq.Request & FG_Mask)
	{
		case FG_Read:
#ifdef DEBUG
		IOdebug("read pos = %d size = %d",req->Pos / dcb->SectorSize,req->Size / dcb->SectorSize);
#endif
			size = req->Size;
			while( done < size )
			{
				WORD tfr = size - done;
				WORD position = (req->Pos / dcb->SectorSize) + (done/dcb->SectorSize);
				command_status = 8;/* make loop happen once*/
				if ( tfr > MAX_TFR )
				{
					tfr = MAX_TFR;
				}
				while (command_status == 8)
				  {
				  res = scsi_read(dcb->Link,dcb->Table,dcb->DeviceId,0,position,tfr,dcb->SectorSize,buf,&command_status);
				  }

				if ( ( res < 0 ) || ( command_status ne 0 ))
					{
					 command_status = 8;
					 while (command_status == 8)
					   {
					   res = scsi_read(dcb->Link,dcb->Table,dcb->DeviceId,0,position,tfr,dcb->SectorSize,buf,&command_status);
					   }
				}

				done += tfr;
				buf  += tfr;
			}

			req->DevReq.Result = command_status;
			if ( req->DevReq.Result eq 0 )
				req->Actual = req->Size;
			else
				req->Actual = 0;
			break;

		case FG_Write:
#ifdef DEBUG
			IOdebug("write pos = %d size = %d",req->Pos / dcb->SectorSize,req->Size / dcb->SectorSize);
#endif
			size = req->Size;
			while( done < size )
			{
				WORD tfr = size - done;
				WORD position = (req->Pos / dcb->SectorSize) + (done/dcb->SectorSize);
				command_status = 8; /* make loop happen once*/
				if ( tfr > MAX_TFR )
				{
					tfr = MAX_TFR;
				}
				while (command_status == 8)
				{
				  res = scsi_write(dcb->Link,dcb->Table,dcb->DeviceId,0,position,tfr,dcb->SectorSize,buf,&command_status);
				}
				if ( ( res < 0 ) || ( command_status ne 0 ))
				{
					command_status = 8;
					while (command_status == 8)
					{
					 res = scsi_write(dcb->Link,dcb->Table,dcb->DeviceId,0,position,tfr,dcb->SectorSize,buf,&command_status);
					}
				}
				done += tfr;
				buf  += tfr;
			}


			req->DevReq.Result = command_status;
			if ( req->DevReq.Result eq 0 )
				req->Actual = req->Size;
			else
				req->Actual = 0;
			break;

		case FG_GetSize:
#ifdef DEBUG
			IOdebug("read capacity request");
#endif
			command_status = 8;
			while (command_status == 8)
			{
			scsi_read_capacity(
				dcb->Link,
				dcb->Table,
				dcb->DeviceId,
				0,		/* lun		   */
				8,		/* capacity length */
				capacity_data,
				&command_status);
			}

			if ( command_status eq 0 )
			{
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


			command_status = 8;/* so the loop happens once*/
			while (command_status == 8)
			{
			scsi_mode_select(
				dcb->Link,
				dcb->Table,
				dcb->DeviceId,
				0,
				0,			/* format whole disk */
				dcb->SectorSize,
				&command_status);
			}
			command_status = 8;/* so the loop happens once*/
			while (command_status == 8)
			{
			scsi_format(
				dcb->Link,
				dcb->Table,
				dcb->DeviceId,
				0,
				freq->Interleave,
				&command_status);
			}
/* If the disk supports the busy status we need to wait until busy goes away
	before giving the message that we are verifying */

			command_status = 8;
			while (command_status == 8)
			{
			  scsi_test_unit_ready(
				dcb->Link,
				dcb->Table,
				dcb->DeviceId,
				0,
				&command_status);
			}
/* some disks will queue one command so we need to wait twice to cope with this
	*/
			command_status = 8;
			while (command_status != 0)
			{
			  IOdebug("status = %d",command_status);
			  scsi_test_unit_ready(
				dcb->Link,
				dcb->Table,
				dcb->DeviceId,
				0,
				&command_status);

			scsi_request_sense(
				dcb->Link,
				dcb->Table,
				dcb->DeviceId,
				0,
				SENSE_LENGTH,
				sense_data,
				&command_status);
			IOdebug("sense =%x",sense_data[2]);
			}


			{
			WORD	total_blocks;
			WORD	i,j;
			WORD	ten_cent,done = 0;
			BYTE	*data;

			IOdebug("\rVerifying disk please wait ....");
			data = Malloc(dcb->SectorSize);
			command_status = 8; /* do at least once*/
			while (command_status == 8 )
			{
				res = scsi_write(dcb->Link,dcb->Table,dcb->DeviceId,0,1,dcb->SectorSize,dcb->SectorSize,data,&command_status);
			}
			total_blocks = (dcb->SectorsPerTrack * dcb->TracksPerCyl * dcb->Cylinders);
/*			IOdebug("\rdisk size is %d blocks",total_blocks);*/
			ten_cent =  total_blocks  / 10;
			for ( i = 1; i < total_blocks; i++)
			{
			 command_status = 8; /* you know by now*/
			 while (command_status == 8)
			 {
				 res = scsi_write_quick(dcb->Link,dcb->Table,dcb->DeviceId,0,i,&command_status);
			}
			second_status = 8;
			while (second_status == 8)
			{
			 res = scsi_read_quick(dcb->Link,dcb->Table,dcb->DeviceId,0,i,&second_status);
			}
					 if (( i % ten_cent )  eq 0)
				{
					done += 10;
					IOdebug("\rVerified %d percent of disk\v",done);
				}
				if ((command_status ne 0) || (second_status ne 0))
				{
					command_status = 8;
					while (command_status == 8 )
					{
					scsi_write_quick(dcb->Link,dcb->Table,dcb->DeviceId,0,i,&command_status);
					}
					second_status = 8;
					while (second_status == 8)
					{
					scsi_read_quick(dcb->Link,dcb->Table,dcb->DeviceId,0,i,&second_status);
					}
					if (( command_status  ne 0 ) || (second_status ne 0))
					{
						IOdebug("Verifier found bad block at %d",i);
						for( j = 0; j  < 10; j++)
						{
							command_status = 8;
							while (command_status == 8)
							{
							scsi_reassign_block(dcb->Link,dcb->Table,dcb->DeviceId,0,i,&command_status);
							}
							command_status = 8;
							while (command_status == 8)
							{
							scsi_write_quick(dcb->Link,dcb->Table,dcb->DeviceId,0,i,&command_status);
							}
							second_status = 8;
							while (second_status == 8)
							{
							scsi_read_quick(dcb->Link,dcb->Table,dcb->DeviceId,0,i,&second_status);
							}
							if ((command_status eq 0) && (second_status eq 0))break;
						}
						if  (( command_status eq 0 ) && (second_status eq 0))
							IOdebug("Block %d reassigned OK",i);
						else
							IOdebug("Failed to reassign block %d",i);
					}
				}
			}

			}
			IOdebug("Verification complete                         ");
			req->DevReq.Result = 0;
			break;
		default:
			break;
		}
		/* Unlock the driver */
		Signal(&dcb->Lock);
		/* Client action */
		(*req->DevReq.Action)(req);
		return;
}

WORD DevClose(DiscDCB *dcb)
{
	WORD	command_status;
	Wait(&dcb->Lock);

	scsi_start_stop(
		dcb->Link,
		dcb->Table,
		dcb->DeviceId,
		0,
		0,		/* park drive in safe area */
		&command_status);
#ifdef DEBUGGING
	if ( command_status ne 0) IOdebug("Failed to park drive");
#endif
	HE1000_release( dcb->Link , dcb->Table );
	return(Err_Null);
}
