/* $Header$ */
/* $Source$ */
/************************************************************************/ 
/* ab1.h - Definitions for Active Book One				*/
/*									*/
/* Copyright 1991 Active Book Company Ltd., Cambridge, England		*/
/*									*/
/* Author: Brian Knight, April 1991					*/
/************************************************************************/

/*
 * $Log$
 */

#ifdef __HELIOSARM

#ifndef  __AB1_H_
#define  __AB1_H_

/************************************************************************/
/* HD81900 fax modem interface						*/
/************************************************************************/

#define AB1_FAX_BASE	0x00D80000	/* Base of (sparse!) registers	*/
#define AB1_FAX_TXD	0x00DD0000	/* Tx data reg (write only)	*/
#define AB1_FAX_RXD	0x00DE0000	/* Rx data reg (read only)	*/

/* DMA channels */

#define AB1_FAX_DMA_CHAN	   2	/* DMA channel used for fax	*/

#define AB1_FAX_RX_ROUTE  DMA_CHAN2_EXTC /* Rx DMA is on REQC		*/
#define AB1_FAX_TX_ROUTE  DMA_CHAN2_EXTC /* Tx DMA is on REQC		*/

/************************************************************************/
/* uPD72001 serial interface						*/
/************************************************************************/

#define AB1_SERIAL_BASE 0x00C00000	/* Base of serial registers	*/

#define AB1_INT_SERIAL  INT_EXA		/* Serial ints on channel A	*/

#define AB1_SERIAL_CLOCK_FREQ (25804800/7) /* Freq of clock to serial chip */

/************************************************************************/

#endif /* __AB1_H_ */
#endif /* __HELIOSARM */

/* End of ab1.h */
