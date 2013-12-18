/* $Header: /dsl/HeliosARM/nick/RCS/fdcdev.h,v 1.1 1991/03/03 22:15:28 paul Exp $ */
/* $Source: /dsl/HeliosARM/nick/RCS/fdcdev.h,v $ */
/************************************************************************/ 
/* fdcdev.h - Header for Helios/ARM device driver for floppy disc on AB	*/
/*	      Functional Prototype					*/
/*									*/
/* Copyright 1990 Active Book Company Ltd., Cambridge, England		*/
/*									*/
/* Author: Brian Knight, September 1990					*/
/************************************************************************/


/************************************************************************/
/* This driver is for the NEC uPD72068 floppy disc controller. 		*/
/************************************************************************/

/*
 * $Log: fdcdev.h,v $
 * Revision 1.1  1991/03/03  22:15:28  paul
 * Initial revision
 *
 * Revision 1.1  90/10/12  17:14:31  brian
 * Initial revision
 * 
 */


#ifndef __FDCDEV_H
#define __FDCDEV_H

#ifndef __device_h
#include <device.h>
#endif

/*----------------------------------------------------------------------*/

#define MAXDRIVES	 4 /* Maximum number of drives on one controller */
#define MAXPARTS	10 /* Maximum number of partitions on one drive	*/

/*----------------------------------------------------------------------*/
/* Flags in DiscType field of DiscDevInfo				*/
/*----------------------------------------------------------------------*/

#define DT_MFM		0x01	/* MFM rather than FM recording		*/
#define DT_IBM		0x02	/* IBM rather than ISO format		*/
#define DT_HIGHDEN	0x04	/* High density (1.44/1.6 MB)		*/

/*----------------------------------------------------------------------*/
/* Device-specific Function Codes (most are for debugging only)		*/
/*----------------------------------------------------------------------*/
#define FCP FC_Private

#define FF_EnableMotor	 (FCP|FG_SetInfo|0x1) /* Enable drive motor  	*/
#define FF_DisableMotor	 (FCP|FG_SetInfo|0x2) /* Disable drive motor 	*/
#define FF_Seek		 (FCP|FG_SetInfo|0x3) /* Seek only	     	*/
#define FF_Recalibrate	 (FCP|FG_SetInfo|0x4) /* Recalibrate head posn	*/

#define FF_ReadId	 (FCP|FG_GetInfo|0x1) /* Read sector id	     	*/
#define FF_SimpleCommand (FCP|FG_GetInfo|0x2) /* For debugging FDC interface */
#define FF_ReadErrCounts (FCP|FG_GetInfo|0x3) /* Read error counts	*/

/*----------------------------------------------------------------------*/
/* Structure used as buffer for FF_ReadErrCounts.			*/
/*----------------------------------------------------------------------*/

typedef struct ErrorCounts
{
  int	softErrors;	/* Total number of soft errors			*/
  int	hardErrors;	/* Total number of hard errors			*/
  /* Separate counts for each error type */
  int	notReady;	
  int	equipmentCheck;	
  int   dataError;
  int	overrun;
  int	noData;
  int	notWritable;
  int	missingAddressMark;
  int	controlMark;
  int	crcError;
  int	wrongCylinder;
  int	badCylinder;
  int	missingAddrMarkInData;
  int	seekNotComplete;
  int	seekToWrongCylinder;
  int	unknown;	/* Unknown reason for failure */
} ErrorCounts;

/************************************************************************/

#endif /* __FDCDEV_H */

/* End of fdcdev.h */







