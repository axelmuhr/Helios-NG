/* $Header: /giga/HeliosRoot/Helios/include/RCS/fproto.h,v 1.2 90/10/30 11:29:15 paul Exp $ */
/* $Source: /giga/HeliosRoot/Helios/include/RCS/fproto.h,v $ */
/************************************************************************/ 
/* fproto.h - Definitions for Active Book Functional Prototype		*/
/*									*/
/* Copyright 1990 Active Book Company Ltd., Cambridge, England		*/
/*									*/
/* Author: Brian Knight, 14th April 1990				*/
/************************************************************************/

/*
 * $Log:	fproto.h,v $
 * Revision 1.2  90/10/30  11:29:15  paul
 * defines FIQ mask
 * 
 * Revision 1.1  90/09/05  11:06:32  nick
 * Initial revision
 * 
 */

#ifdef __HELIOSARM

#ifndef  _FPROTO_H_
#define  _FPROTO_H_

/* Addresses of memory-mapped device registers */

#define CODEC_DATA	0x00400060	/* Codec data register		*/
#define CODEC_CONSTAT	0x00400071	/* Codec control/status		*/
#define LINK0_BASE	0x00480000	/* Transputer link 0		*/
#define LINK1_BASE	0x00500000	/* Transputer link 1 (has reset line)*/
#define FLOPPY_BASE	0x00600000	/* 72068 floppy disc controller	*/
#define SERIAL_BASE	0x00680000	/* 72001 serial interface	*/
#define FLASH_BASE	0x00700000	/* Flash EPROM			*/

#define FIQ_MASK	0x00400021	/* FIQ mask register		*/

/* Bit positions in the interrupt mask registers */

#define INT_SERIAL	0x01	/* Serial line interface		*/
#define INT_FLOPPY	0x02	/* Floppy disc				*/
#define INT_CODECRX	0x04	/* Codec reception			*/
#define INT_CODECTX	0x08	/* Codec transmission			*/
#define INT_TIMER0	0x10	/* Timer counter 0 (used by executive)	*/
#define INT_PODFRAME	0x20	/* Podule or LCD frame start		*/
#define INT_LINK0	0x40	/* Transputer link 0			*/
#define INT_LINK1	0x80	/* Transputer link 1 (used by executive)*/

/* Function prototypes */
/* These functions are in the kernel library */

word DisableIRQ(word mask); /* Disable IRQ sources with bits set in mask */
word EnableIRQ(word mask);  /* Enable IRQ sources with bits set in mask  */

#endif /* _FPROTO_H_ */
#endif /* __HELIOSARM */

/* End of fproto.h */
