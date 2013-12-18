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

#define	IMB_DMA_OFF		0
#define	IMB_DMA_READ		1
#define	IMB_DMA_WRITE		2
#define	IMB_DMA_READWRITE	(IMB_DMA_READ|IMB_DMA_WRITE)	
/*
 * I/O controls
 */
#define	IMB_RESET		(('k'<<8)|0)	/* Reset site */
#define	IMB_ANALYSE		(('k'<<8)|1)	/* Analyse site */
#define	IMB_ENABLE_ERRORS	(('k'<<8)|2)	/* Abort i/o on error */
#define	IMB_DISABLE_ERRORS	(('k'<<8)|3)	/* Ignore errors */
#define	IMB_ERROR		(('k'<<8)|4)	/* Is error flag set? */
#define	IMB_INPUT_PENDING	(('k'<<8)|5)	/* Is input pending */
#define IMB_DMA			(('k'<<8)|6)	/* DMA setup (read/write/off */
#define IMB_TIMEOUT		(('k'<<8)|8)	/* Set timeout */
#define IMB_OUTPUT_READY	(('k'<<8)|9)	/* Ready to output */

#endif
