/*
 * File:	hw_arch.h
 * Subsystem:	Helios-ARM implementation
 * Author:	P.A.Beskeen
 * Date:	Oct '92
 *
 * Description: Description of the ARM based Acorn Achimedies series
 *		This includes many different models and can be a little
 *		complex.
 *
 *		The Archimedies range consists of:
 *		@@@ update with full range:
 *			A440/R140 (ARM2, ST506)
 *			A500
 *			A680
 *			A3000 (ARM3, IDE)
 *			A3010 (ARM250, IDE)
 *			A3020 (ARM250, IDE)
 *			A4000
 *			A5000
 *
 *
 * RcsId: $Id: hw_arch.h,v 1.1 1993/08/03 17:11:45 paul Exp $
 *
 * (C) Copyright 1992 Perihelion Software Ltd.
 *     All Rights Reserved.
 * 
 */

#ifndef	__hw_arch_h
#define	__hw_arch_h


#include <ARM/ioc.h>	/* Most hardware is hung off an ARM IOC. */
#include <ARM/xcb.h>	/* Acorn expansion card bus (podules). */


/* IOC interfaced hardware. */

/* Control port register bits. */

#define IOC_CON_I2C_DATA     (1 << 0)	/* I2C Bus data  line - R/W */
#define IOC_CON_I2C_CLOCK    (1 << 1)	/* I2C Bus clock line - R/W */

/*
 * The sound mute line is defined on all machines, but only performs
 * its expected function on the A500 and A440.  On A680 it must be 
 * left as a 1.
 */
#define IOC_CON_SOUND_MUTE   (1 << 5)	/* Sound Mute (A500/A440) - R/W */

/*
 * The set of bits which must be 1 when writing to the control port,
 * on any machine.
 */
#define IOC_CON_WRITE_SET   (1 << 7 | 1 << 6 | 1 << 4 | 1 << 3 | 1 << 2)


/* The following bits are defined for A4x0 machines */

#define IOC_CON4_DISC_READY  (1<<2)	/* floppy drive ready - RO */
#define IOC_CON4_AUX_C4	     (1<<4)	/* aux I/O line (unused) R/W */


/* The following 3 bits are present only on A500's. */

#define IOC_CON5_RTC_MINUTES (1<<2)	/* Real Time Clock minutes - RO */
#define IOC_CON5_RTC_SECONDS (1<<3)	/* Real Time Clock seconds - RO */
#define IOC_CON5_DISC_CHANGE (1<<4)	/* Floppy disc changed - RO */


/* A680 only. */

#define IOC_CON6_DISC_CHANGE (1<<2)	/* floppy disc changed - RO */


/* Interrupt control register bits. */

/* IRQ A block - mostly latched events, cleared via IRQ clear reg. */

#define  IRQA_PBSY   (1 << 0)   /* Printer BuSY */
#define  IRQA_RII    (1 << 1)   /* Serial line RInging Indication */
#define  IRQA_PACK   (1 << 2)   /* Printer ACKnowledge event */
#define  IRQA_VFLY   (1 << 3)   /* Vertical FLYback event */


/* IRQ B block. */

#define  IRQB_XFIQ   (1 << 0)   /* XCB FIQ(!) bit - mask OFF */
#define  IRQB_SND    (1 << 1)   /* Sound buffer switch event */
#define  IRQB_SLCI   (1 << 2)   /* Serial Line Controller Int */
#define  IRQB_WINC   (1 << 3)   /* Winchester Controller int */
#define  IRQB_WIND   (1 << 4)   /* Winchester Data int */
#define  IRQB_WINCD  (1 << 3)	/* Combined Controller/Data bit (Archimedes) */
#define  IRQB_FDDC   (1 << 4)	/* Floppy Disc Disc Changed (Archimedes) */
#define	 IRQB_FDINT  (1 << 4)   /* Floppy disc intr (A680) */
#define  IRQB_XCB    (1 << 5)   /* Expansion card common IRQ */


/* FIQ block. */

#define  FIQ_FDDR    (1 << 0)   /* Floppy Disc Data Request */
#define  FIQ_FDIR    (1 << 1)   /* Floppy Disc Interrupt Request */
#define  FIQ_ECONET  (1 << 2)   /* ECOnet interrupt */
/* 3..5 not used */
#define  FIQ_XCB     (1 << 6)   /* XCB card FIQ */
#define  FIQ_FORCE   (1 << 7)   /* Force FIQ int (permanently 1) */


/* External (to IOC) I/O hardware control structures. */

#define	EXT_A_L4	((volatile unsigned char *)0x03350040)
#define	EXT_A_L5	((volatile unsigned char *)0x03360000)
#define	EXT_B_LATCH	*((volatile unsigned char *)0x03350018)
#define EXT_L6          *((volatile unsigned char *)0x03350040)


/* External Latch A bits. */

#define	UNITBITS	0xF
#define	INUSEBIT	(1 << 6)
#define	MOTORBIT	(1 << 5)
#define	SIDEBIT		(1 << 4)


/* External Latch B floppy-specific bits on A500, A4xx. */

#define	DENBITS		0x7
#define	DOUBDEN		0x5

#define	FDCRESET	0x08
#define EXTB_FDC_BITS	(DENBITS | FDCRESET)


/* Other External Latch B bits. */

/* Printer Strobe Line: bit 0 on A680, bit 4 on the others. */

#define EXTB_L45_PR_STROBE	(1 << 4)
#define EXTB_L6_PR_STROBE	(1 << 0)


/* Audio system control (A500 only). */

#define EXTB_AIN_MUTE		(1 << 5)	/* audio input mute */
#define EXTB_AOUT_MUTE		(1 << 6)	/* audio output mute */


/* Extra head select line for ST506 interface (not A680). */

#define EXTB_HS3		(1 << 7)	/* Head Select 3 for HDC */


/* FDC for A680 bits. */

#define F6_UNITBITS	((1 << 1)|(1 << 0))
#define F6_DCRST	(1 << 2)		/* Disc change reset */
#define F6_DENSITY	(1 << 3)		/* Density */
#define F6_SIDEBIT	(1 << 4)		/* Side */
#define F6_MOTORON	(1 << 5)		/* Motor on */
#define F6_INUSE	(1 << 6)		/* In use */
#define F6_RESET        (1 << 7)		/* Reset FDC */


#endif /*__hw_arch_h*/


/* end of hw_arch.h */
