#ifndef _NIDIO_DEFS_H

#define _NIDIO_DEFS_H

#include <sys/types.h>
#include "nidio.h"

#define u_short	unsigned short
#define u_char	unsigned char

struct nidio_reg
{
    u_short	cfg1;		/* 0x00 config 1 register and status register */
#define STAT	cfg1
    u_short	cfg2;		/* 0x02 config 2 register */
    u_short	cfg3;		/* 0x04 config 3 register */
    u_char	porta;		/* 0x06 Port A register */
    u_char	portb;		/* 0x07 Port B register */
    u_char	portc;		/* 0x08 Port C register */
    u_char	portd;		/* 0x09 Port D register */
    u_short	cntintclr;	/* 0x0a CNTINTCLR register */
    u_short	dmaclr1;	/* 0x0c DMACLR1 register */
    u_short	dmaclr2;	/* 0x0e DMACLR2 register */
    u_short	rtsishft;	/* 0x10 RTISHFT register */
    u_short	rtsistrb;	/* 0x12 RTISTRB register */
    u_short	cfg4;		/* 0x14 config 4 register */
    u_short	space;		/* 0x16 not used */
    u_short	cntr1;		/* 0x18 CNTR 1 register */
    u_short	cntr2;		/* 0x1a CNTR 2 register */
    u_short	cntr3;		/* 0x1c CNTR 3 register */
    u_short	cntrcmd;	/* 0x1e CNTRCMD register */
};

/*
 * Register bit definitions
 */

/* CFG1 register */

#define DMAEN1	(u_short) 0x8000
#define INTEN1	(u_short) 0x4000
#define T1S2	(u_short) 0x2000
#define T1S1	(u_short) 0x1000
#define T1S0	(u_short) 0x0800
#define DIOBEN	(u_short) 0x0400
#define DIOAEN	(u_short) 0x0200
#define LRESET1	(u_short) 0x0100
#define INVRQ1	(u_short) 0x0040
#define DBLDUFA	(u_short) 0x0020
#define PULSE1	(u_short) 0x0010
#define EDGE1	(u_short) 0x0008
#define INVACK1	(u_short) 0x0004
#define SETACK1	(u_short) 0x0002
#define OUT1	(u_short) 0x0001

/* CFG2 register */

#define DMAEN2	(u_short) 0x8000
#define INTEN2	(u_short) 0x4000
#define T2S2	(u_short) 0x2000
#define T2S1	(u_short) 0x1000
#define T2S0	(u_short) 0x0800
#define DIODEN	(u_short) 0x0400
#define DIOCEN	(u_short) 0x0200
#define LRESET2	(u_short) 0x0100
#define INVRQ2	(u_short) 0x0040
#define DBLDUFC	(u_short) 0x0020
#define PULSE2	(u_short) 0x0010
#define EDGE2	(u_short) 0x0008
#define INVACK2	(u_short) 0x0004
#define SETACK2	(u_short) 0x0002
#define OUT2	(u_short) 0x0001

/* CFG3 register */

#define DBLBUFB	 (u_short) 0x8000
#define WRITED	 (u_short) 0x4000
#define WRITEB	 (u_short) 0x2000
#define TRANS32	 (u_short) 0x1000
#define WRITEC	 (u_short) 0x0800
#define WRITEA	 (u_short) 0x0400
#define CNT2SRC	 (u_short) 0x0200
#define CNT1SRC	 (u_short) 0x0100
#define DBLDMA	 (u_short) 0x0080
#define CNTINTEN (u_short) 0x0040
#define CNT2HSEN (u_short) 0x0020
#define CNT1HSEN (u_short) 0x0010
#define CNT2EN	 (u_short) 0x0008
#define CNT1EN	 (u_short) 0x0004
#define TCINTEN2 (u_short) 0x0002
#define TCINTEN1 (u_short) 0x0001

/* CFG4 register */

#define LPULSE2	(u_short) 0x0008
#define LPULSE1	(u_short) 0x0004
#define DBLBUFD	(u_short) 0x0002
#define REVC	(u_short) 0x0001

/* STAT register */

#define DMACH	(u_short) 0x8000
#define CNTINT	(u_short) 0x4000
#define TRANS32	(u_short) 0x1000
#define IN1	(u_short) 0x0800
#define DMATC2	(u_short) 0x0400
#define DMATC1	(u_short) 0x0200
#define DRDY1	(u_short) 0x0100
#define REQ1	(u_short) 0x0080
#define ACK1	(u_short) 0x0040
#define IN2	(u_short) 0x0020
#define ID	(u_short) 0x0010
#define DRDY2	(u_short) 0x0004
#define REQ2	(u_short) 0x0002
#define ACK2	(u_short) 0x0001

/*
 * I/o controls for debugging use
 */
#define NIDIO_DEBUG		(('n'<<8)|23)	/* Set driver debug messages */
#define NIDIO_READ_REG		(('n'<<8)|24)	/* Read register */
#define NIDIO_WRITE_REG		(('n'<<8)|25)	/* Write register */

#endif
