#ifndef _HEPCDEFS_H

#define _HEPCDEFS_H

#include <sys/types.h>
#include "hepcio.h"

#define u_char	unsigned char

struct hepc_reg
{
    u_char	idr;		/* input data register */
    u_char	odr;		/* output data register */
    u_char	isr;		/* input status register */
    u_char	osr;		/* output status register */
    u_char	cfg;		/* config register */
    u_char	space[1];	/* not used */
    u_char	reset;		/* reset register */
    u_char	pifsr;		/* perfomance i/f status register */
    u_char	intcr;		/* interrupt control register */
    u_char	intsr;		/* interrupt status register */
};

/*
 * Intcr interrupt enable bit masks
 */
#define INT_HIWM		4
#define INT_SEND_READY		2
#define INT_RECEIVE_READY	1

#define	LINK_READY		(u_char)0x80

/*
 * I/o controls for debugging use
 */
#define HEPC_DEBUG		(('k'<<8)|23)	/* Set driver debug messages */
#define HEPC_READ_REG		(('k'<<8)|24)	/* Read register */
#define HEPC_WRITE_REG		(('k'<<8)|25)	/* Write register */

#endif
