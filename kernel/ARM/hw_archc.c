/*
 * File:	hw_archc.c
 * Subsystem:	Helios-ARM executive
 * Author:	P.A.Beskeen
 * Date:	Nov '92
 *
 * Description: Archimedes specific hardware functions.
 *
 *
 * RcsId: $Id: hw_archc.c,v 1.1 1993/08/24 08:45:33 paul Exp $
 *
 * (C) Copyright 1992 Perihelion Software Ltd.
 * All Rights Reserved.
 *
 * RcsLog: $Log: hw_archc.c,v $
 * Revision 1.1  1993/08/24  08:45:33  paul
 * Initial revision
 *
 *
 *
 */

#include "../kernel.h"

#include <ARM/ioc.h>


#ifndef __ARCHIMEDES
# error "This source file should only be used for Archimedes systems"
#endif


/* Function prototypes for Archi specific functions */

void IOCInit(void);
void MEMCInit(void);
void VIDCInit(void);


/* Initialise any system hardware specific details.
 */
void HWSpecificInit(void)
{
	IOCInit();
	MEMCInit();
	VIDCInit();
}


/*
 * StoreSize
 *
 * Calculate the amount of available memory..
 * StoreSize is passed the base of free memory, it should return the address
 * of the first unusable byte (word) of RAM.
 *
 * (mem_start argument is actually end of config vector).
 */
byte *StoreSize(byte *mem_start) {
	/* @@@ cheat! - assume 4Mb system. MUST BE UPDATED.
	 * 4Mb from address 0, minus the screen buffer that is mapped to
	 * highest 160k of memory.
	 */
	return	(byte *) ((1024 * 1024 * 4) - (160 * 1024));
}


/* Enable / Disable Clock interrupts.
 */
void ClockIntsOn(void) {
	if (IntsAreEnabled()) {
		IntsOff();
		IOC->irq_a.mask |= IRQA_TM0;	/* enable timer 0 interrupts */
		IntsOn();
	} else {
		IOC->irq_a.mask |= IRQA_TM0;	/* enable timer 0 interrupts */
	}
}

/* Disable time slicer clock interrupts.
 */
void ClockIntsOff(void) {
	if (IntsAreEnabled()) {
		IntsOff();
		IOC->irq_a.mask &= ~IRQA_TM0;	/* disable timer 0 interrupts */
		IntsOn();
	} else {
		IOC->irq_a.mask &= ~IRQA_TM0;	/* disable timer 0 interrupts */
	}
}


#if 0 /* Use Assembler version of this function */

/* void StartTimeSlicer(void)
 *
 * Starts the time slicer.
 * 
 * Initialise time slicer clock to call `TimerIntrHandler' every
 * millisecond . Once set up, the clock is reset and slicer
 * interrupts are enabled.
 *
 * Assumes interrupt vector is already initialised.
 *
 */

void StartTimeSlicer(void) {

	/* set timer to interrupt every 1ms's */
	IOC->timer_0.latch_hi = (IOC_Timer_1ms >> 8);

	/* and set timer contents from latch */
	IOC->timer_0.go_cmd = IOC->timer_0.latch_lo = (IOC_Timer_1ms && 0xff);

	/* enable timer 0 interrupts */
	IOC->irq_a.mask |= IRQA_TM0;
}
#endif


/*
 * void LinkInitSpecial(LinkInfo *link);
 *
 * Support for 'special' non-standard link types that are not part of the
 * basic comms link support.
 *
 */

void LinkInitSpecial(LinkInfo *link)
{
	switch(link->State) {
#if 0
	case Link_State_XXX :
		link->TxFunction	= (WordFnPtr) XXX_LinkTx;
		link->RxFunction	= (WordFnPtr) XXX_LinkRx;
		break;
#endif

	/* If the link type is not recognised, set the link to unusable */
	default:
		link->Mode	= Link_Mode_Null;
		link->State	= Link_State_Null;
		return;
	}

	/* If the special link type is known, change the link mode and	*/
	/* state from special to running. Flags should have been set	*/
	/* already.							*/
	link->Mode	= Link_Mode_Intelligent;
	link->State	= Link_State_Running;
}


/* For object code compatibility with VLSI PID version. */
/* Dummy function: */
void hw_setLED(int leds) {
	return;
}

/* end of hw_archc.c */
