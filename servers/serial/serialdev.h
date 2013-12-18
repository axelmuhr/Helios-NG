/* $Header: serialdev.h,v 1.3 90/10/07 09:41:13 brian Exp $ */
/* $Source: /server/usr/users/a/brian/world/helios/dev/serial/RCS/serialdev.h,v $ */
/************************************************************************/ 
/* serialdev.h - Header for ARM Helios serial line device driver for 	*/
/*	         AB Functional Prototype board				*/
/*									*/
/* Copyright 1990 Active Book Company Ltd., Cambridge, England		*/
/*									*/
/* Author: Brian Knight, 14th February 1990				*/
/************************************************************************/

/*
 * $Log:	serialdev.h,v $
 * Revision 1.3  90/10/07  09:41:13  brian
 * Checkpoint before moving to SMT
 * 
 * Revision 1.2  90/07/05  16:18:08  brian
 * Checkpoint before fiddling with semaphore handling
 * 
 * Revision 1.1  90/06/12  10:00:46  brian
 * Initial revision
 * 
 * Revision 1.2  90/05/01  09:27:03  brian
 * Checkpoint before moving to functional prototype board.
 * 
 * Revision 1.1  90/03/08  12:22:35  brian
 * Initial revision
 * 
 */

#ifndef __SERIALDEV_H
#define __SERIALDEV_H

#ifndef __device_h
#include <device.h>
#endif

#include <dev/result.h>

#define SERIAL_ID RESULT_SOFTUNIT(3)
#define MAXSERIALDEVICES (1) /* Maximum number of SERIAL devices */

#define DEFAULTTXBAUDRATE 9600
#define DEFAULTRXBAUDRATE 9600

/*----------------------------------------------------------------------*/
/* Device-specific Function Codes (most are for debugging only)		*/
/*----------------------------------------------------------------------*/
#define FCP FC_Private

#define FS_UseLights	 (FCP|FG_SetInfo|0x1) /* Use debugging lights */
#define FS_NoLights	 (FCP|FG_SetInfo|0x2) /* Don't use lights     */
#define FS_SetReg	 (FCP|FG_SetInfo|0x3) /* Set control register */
#define FS_OpenChannel	 (FCP|FG_SetInfo|0x4) /* Open one channel of device */
#define FS_CloseChannel	 (FCP|FG_SetInfo|0x5) /* Close one channel of device */

#define FS_GetNumLines	 (FCP|FG_GetInfo|0x1) /* Read no. of serial lines */
#define FS_GetStatusReg	 (FCP|FG_GetInfo|0x2) /* Read status register */
#define FS_GetControlReg (FCP|FG_GetInfo|0x3) /* Read ctrl reg soft copy */

/************************************************************************/

/* Info parameter to DevOpen() */

typedef struct SerialDevInfo
{
  int logDevNum; /* Logical no. of device to open (0..MAXSERIALDEVICES-1) */
} SerialDevInfo;
        
/*----------------------------------------------------------------------*/

/* Specific Errors */

/* Overrun on reading */
#define SERIAL_SERROROVERRUN	SERROR(SERIAL, 255, Error)

/* Attempt to open channel twice or do two reads or writes in parallel */
#define SERIAL_SERRORINUSE	SERROR(SERIAL, 254, Error)

/* Sub-device number out of range */
#define SERIAL_SERRORBADSUBDEVICE SERROR(SERIAL, 253, Error)

/* Attempt to close channel which was not open */
#define SERIAL_SERRORNOTOPEN SERROR(SERIAL, 252, Error)

/* Function not implemented (yet) */
#define SERIAL_SERRORNOTIMPLEMENTED SERROR(SERIAL, 200, Error)

/************************************************************************/

#endif /* __SERIALDEV_H */

/* End of serialdev.h */
