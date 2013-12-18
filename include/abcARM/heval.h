/* $Header: heval.h,v 1.1 91/01/02 13:40:59 brian Exp $ */
/* $Source: /server/usr/users/a/brian/world/helios/dev/fax/RCS/heval.h,v $ */
/************************************************************************/ 
/* heval.h - Definitions for Active Book Hercules Evaluation Board	*/
/*									*/
/* Copyright 1990 Active Book Company Ltd., Cambridge, England		*/
/*									*/
/* Author: Brian Knight, November 1990					*/
/************************************************************************/

/*
 * $Log:	heval.h,v $
 * Revision 1.1  91/01/02  13:40:59  brian
 * Initial revision
 * 
 */

#ifdef __HELIOSARM

#ifndef  __HEVAL_H_
#define  __HEVAL_H_

/************************************************************************/
/* HD81900 fax modem interface						*/
/************************************************************************/

#define HEVAL_FAX_BASE	0x00FC0000	/* Base of (sparse!) registers	*/
#define HEVAL_FAX_TXD	0x00FF0000	/* Tx data reg (write only)	*/
#define HEVAL_FAX_RXD	0x00FE0000	/* Rx data reg (read only)	*/

/* DMA channels */

#define HEVAL_FAX_DMA_CHAN	   3	/* DMA channel used for fax	*/

#define HEVAL_FAX_RX_ROUTE  DMA_CHAN3_EXTB /* Rx DMA is on REQB		*/
#define HEVAL_FAX_TX_ROUTE  DMA_CHAN3_EXTC /* Tx DMA is on REQC		*/

/************************************************************************/
/* uPD72001 serial interface						*/
/************************************************************************/

#define HEVAL_SERIAL_BASE 0x00A00000	/* Base of serial registers	*/

#define HEVAL_INT_SERIAL  VINT_EXB	/* Serial ints on channel B	*/

#define HEVAL_SERIAL_CLOCK_FREQ (29491200/4) /* Freq of clock to serial chip */

/************************************************************************/

#endif /* __HEVAL_H_ */
#endif /* __HELIOSARM */

/* End of heval.h */
