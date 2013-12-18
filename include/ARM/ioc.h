/*
 * File:	ioc.h
 * Subsystem:	Helios/ARM implementation
 * Author:	P.A.Beskeen
 * Date:	Oct '92
 *
 * Description: IOC (VL86C410) ARM I/O Controller manifests
 *		Includes timers, serial keyboard, interrupt control,
 *		periperal access and programmable I/O pins.
 *
 *
 * RcsId: $Id: ioc.h,v 1.2 1993/12/10 14:15:14 paul Exp $
 *
 * (C) Copyright 1992 Perihelion Software Ltd.
 *     All Rights Reserved.
 * 
 */

#ifndef	__ioc_h
#define	__ioc_h

/* Std IOC address
 */
#ifndef IOC_base
# define IOC_base	0x03200000
#endif

/* Use this macro to access IOC registers directly from C
 */
#define IOC		((ioc_regs *)IOC_base)


/* IOC register block definitions.
 */
typedef struct intr_block {
	volatile unsigned char
		status,		pad0a, pad0b, pad0c,
		request,	pad1a, pad1b, pad1c,
		mask,		pad2a, pad2b, pad2c,
				pad3a, pad3b, pad3c, pad3d;
} intr_block;

typedef struct timer_block {
	volatile unsigned char
		count_lo,	pad0a, pad0b, pad0c,	/* RO */
#		define latch_lo	count_lo		/* WO */
		count_hi,	pad1a, pad1b, pad1c,	/* RO */
#		define latch_hi	count_hi		/* WO */
		go_cmd,		pad2a, pad2b, pad2c,	/* WO */
		latch_cmd,	pad3a, pad3b, pad3c;	/* WO */
} timer_block;
         
typedef struct ioc_regs {
	volatile unsigned char
		control,	pad0a, pad0b, pad0c,	/* 00..03 R/W */
		kart_data,	pad1a, pad1b, pad1c;	/* 04..07 R=Rx, w=Tx */
		int		pad2, pad3;		/* 08..0F */

	intr_block
		irq_a,					/* 10..1F */
#		define irq_clear irq_a.request		/* WO */
		irq_b,					/* 20..2F */
		fiq;					/* 30..3F */

	timer_block
		timer_0,				/* 40..4F */
		timer_1,				/* 50..5F */
		timer_baud,				/* 60..6F */
		timer_kart;				/* 70..7F */
} ioc_regs;


/* Internal IOC interrupt bits */

/* Std IRQ status register A bits.
 * Mostly latched events, cleared via IRQ clear reg.
 */
#define IRQA_POR	(1 << 4)	/* Power On Reset */
#define IRQA_TM0	(1 << 5)	/* Timer 0 expiry */
#define IRQA_TM1	(1 << 6)	/* Timer 1 expiry */
#define IRQA_FORCE	(1 << 7)	/* Force IRQ int bit (permanently 1) */


/* Std IRQ status register B bits.
 */
#define IRQB_KSTx	(1 << 6)	/* KART transmit done */
#define IRQB_KSRx	(1 << 7)	/* KART receiver data ready */


/* Other interrupt bits in IOC registers are defined in hw_machine.h machine
 * specific header files.
 */

/* Delay required after KSRx set in IRQ B status before reading KART data reg,
 * with KART serial line on maximum speed.
 */
#define IOC_KART_DELAY	16		/* in microseconds: 1/2 bit time */


/* Number of nanoseconds for each timer tick.
 */
#if 0
/* Number of nanoseconds for each timer tick. Number arrived at impirically
 * as documentation doesn't seem to be correct!
 */
# define IOC_Timer_Resolution	125
#else
/* Number of nanoseconds for each timer tick (assumes 8Mhz IOC)
 * As documented in the IOC manual!?
 */
# define IOC_Timer_Resolution	500
#endif

/* One microseconds worth of timer ticks (1 tick = 125/500nS)
 */
#define IOC_Timer_1us	(1000 / IOC_Timer_Resolution)

/* One milliseconds worth of timer ticks (1 tick = 125/500nS)
 */
#define IOC_Timer_1ms	((1000 / IOC_Timer_Resolution) * 1000)

/* Macro returns number of microseconds since last timer tick .
 * Assumes timer cycles at one millisecond intervals. Counting down to zero.
 */
#define AddUSecs()	(IOC->timer_0.latch_cmd = 0, \
			((IOC_Timer_1ms - (IOC->timer_0.count_hi << 8 \
			| IOC->timer_0.count_lo)) \
			* IOC_Timer_Resolution) / 1000)

/* Macro returns number of microseconds before next timer tick 
 */
#define ToGoUSecs()	(IOC->timer_0.latch_cmd = 0, \
			((IOC->timer_0.count_hi << 8 \
			| IOC->timer_0.count_lo) \
			* IOC_Timer_Resolution) / 1000 )


#endif /*__ioc_h*/



/* End of ioc.h */
