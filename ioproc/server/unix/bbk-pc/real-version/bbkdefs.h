/* %W%	%E%
/*#############################################################################
 *
 *	Copyright (C) 1989,1990 K-par Systems  Ltd.  All rights reserved
 *
 * Program/Library:	bbk Interactive 386/ix B008 driver - bbkdefs.h
 *
 * Purpose: 		bbk driver internal definitions file
 *
 * Author:		Chris Farey 27-Apr-1989
 *			Modified by Tony Cruickshank 5-July-93
 *
 *---------------------------------------------------------------------------*/

#ifndef _BBKDEFS_H

#define _BBKDEFS_H

#include <sys/types.h>
#include "bbkio.h"

#define u_char	unsigned char

struct bbk_reg
{
    u_char	idr;		/* input data register */
    u_char	odr;		/* output data register */
    u_char	isr;		/* input status register */
    u_char	osr;		/* output status register */
    u_char	space[12];	/* not used */
    u_char	reset_error;	/* reset (write) / error (read) */
    u_char	analyse;	/* analyse (write only) */
    u_char	ior;		/* interrupt on reset - not used */
};

/*
 * Icr interrupt enable bit masks
 */
#define INT_ERROR		1
#define INT_SEND_READY		2
#define INT_RECEIVE_READY	4

#define	INT_C012		2

#define	LINK_READY		(u_char)1

/*
 * I/o controls for debugging use
 */
#define BBK_DEBUG		(('k'<<8)|23)	/* Set driver debug messages */
#define BBK_READ_REG		(('k'<<8)|24)	/* Read register */
#define BBK_WRITE_REG		(('k'<<8)|25)	/* Write register */
#define	BBK_SET_ERROR		(('k'<<8)|26)	/* Set error flag sense */

#endif
