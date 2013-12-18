/* @(#)bbkio.h	1.3	89/10/18
/*#############################################################################
 *
 *	Copyright (C) 1989 K-par Systems  Ltd.  All rights reserved
 *
 * Program/Library:	bbk Sun 386i B008 driver - bbkio.h
 *
 * Purpose: 		bbk driver user definitions file
 *
 * Author:		Chris Farey 27-Apr-1989
 *			Modified by Tony Cruickshank 5-July-1993
 *
 *---------------------------------------------------------------------------*/

#ifndef _BBKIO_H

#define _BBKIO_H

/*
 * I/O controls
 */
#define	BBK_RESET		(('k'<<8)|0)	/* Reset site */
#define	BBK_ANALYSE		(('k'<<8)|1)	/* Analyse site */
#define	BBK_ENABLE_ERRORS	(('k'<<8)|2)	/* Abort i/o on error */
#define	BBK_DISABLE_ERRORS	(('k'<<8)|3)	/* Ignore errors */
#define	BBK_ERROR		(('k'<<8)|4)	/* Is error flag set? */
#define	BBK_INPUT_PENDING	(('k'<<8)|5)	/* Is input pending */
#define BBK_TIMEOUT		(('k'<<8)|7)	/* Set timeout */
#define BBK_OUTPUT_READY	(('k'<<8)|8)	/* Ready to output */

#endif
