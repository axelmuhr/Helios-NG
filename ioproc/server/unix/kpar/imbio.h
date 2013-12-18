/* @(#)imbio.h	1.3	89/10/18
/*#############################################################################
 *
 *	Copyright (C) 1989 K-par Systems  Ltd.  All rights reserved
 *
 * Program/Library:	imb Sun 386i B008 driver - imbio.h
 *
 * Purpose: 		imb driver user definitions file
 *
 * Author:		Chris Farey 27-Apr-1989
 *
 *---------------------------------------------------------------------------*/

#ifndef _IMBIO_H

#define _IMBIO_H

#include <sys/ioccom.h>

#define	IMB_DMA_OFF		0
#define	IMB_DMA_READ		1
#define	IMB_DMA_WRITE		2
#define	IMB_DMA_READWRITE	(IMB_DMA_READ|IMB_DMA_WRITE)	
/*
 * I/O controls
 */
#define	IMB_RESET		_IO(k,0)	/* Reset site */
#define	IMB_ANALYSE		_IO(k,1)	/* Analyse site */
#define	IMB_ENABLE_ERRORS	_IO(k,2)	/* Abort i/o on error */
#define	IMB_DISABLE_ERRORS	_IO(k,3)	/* Ignore errors */
#define	IMB_ERROR		_IOR(k,4,int)	/* Is error flag set? */
#define	IMB_INPUT_PENDING	_IOR(k,5,int)	/* Is input pending */
#define IMB_DMA			_IOW(k,6,int)	/* DMA setup (read/write/off */
#define IMB_TIMEOUT		_IOW(k,8,int)	/* Set timeout */
#define IMB_OUTPUT_READY	_IOR(k,9,int)	/* Ready to output */

#endif
