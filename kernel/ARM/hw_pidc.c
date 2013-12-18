/*
 * File:	hw_pidc.c
 * Subsystem:	Helios-ARM executive
 * Author:	P.A.Beskeen
 * Date:	March '93
 *
 * Description: VLSI VY86PID specific hardware functions.
 *
 *
 * RcsId: $Id: hw_pidc.c,v 1.1 1993/08/24 08:45:33 paul Exp $
 *
 * (C) Copyright 1992 Perihelion Software Ltd.
 * All Rights Reserved.
 *
 * RcsLog: $Log: hw_pidc.c,v $
 * Revision 1.1  1993/08/24  08:45:33  paul
 * Initial revision
 *
 *
 *
 */

#include "../kernel.h"

#include <ARM/vy86pid.h>


#ifndef __VY86PID
# error "This source file should only be used for VLSI PID systems"
#endif


/* Function prototypes for PID specific functions */

void IO_Init(void);
void MEM_Init(void);


/* Initialise any system hardware specific details.
 */
void HWSpecificInit(void)
{
	/* Initialise soft copy of interrupt mask. */
	/* Panic button interrupt enable is always set. */
	GetRoot()->IRQM_softcopy = hw_intc_irq_panic;

	/* Force I/O and interrupt world into an aquescent state. */
	/* Functions provided in hw_pida.a */
	IO_Init();
	MEM_Init();
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
	/* @@@ cheat! - assume 4Mb system. MUST BE UPDATED. */
	return	(byte *) (1024 * 1024 * 4);
}


/* Enable and disable system timer interrupts.
 */
void ClockIntsOn(void) {
	RootStruct *kr = GetRoot();

	if (IntsAreEnabled()) {
		IntsOff();
		/* enable timer interrupts */
		hw_INTC->IRQM = kr->IRQM_softcopy |= hw_intc_enableirq_timer;
		IntsOn();
	} else {
		/* enable timer interrupts */
		hw_INTC->IRQM = kr->IRQM_softcopy |= hw_intc_enableirq_timer;
	}
}

void ClockIntsOff(void) {
	RootStruct *kr = GetRoot();

	if (IntsAreEnabled()) {
		IntsOff();
		/* disable timer interrupts */
		hw_INTC->IRQM = kr->IRQM_softcopy &= ~hw_intc_enableirq_timer;
		IntsOn();
	} else {
		/* disable timer interrupts */
		hw_INTC->IRQM = kr->IRQM_softcopy &= ~hw_intc_enableirq_timer;
	}
}


/* void StartTimeSlicer(void)
 *
 * Starts the time slicer.
 * 
 * Initialise time slicer clock to call `TimerIntrHandler' every
 * 10 milliseconds. Once set up, the clock is reset and slicer
 * interrupts are enabled.
 *
 * Assumes interrupt vector is already initialised.
 *
 */
void StartTimeSlicer(void) {


	/* Timer is simply a fixed 10 millsecond periodic timer interrupt */
	/* Enable timer interrupts by changing interrupt mask. */
	ClockIntsOn();

	/* Clear any outstanding interrupt. */
	hw_INTC->IRQRST = hw_intc_resetirq_timer;
}



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


/* PID specific Function to set the LED's. */
/* This is coded as a function rather than a simple macro for object code */
/* compatibility with other platforms. */

void hw_setLED(int leds) {
	hw_pid_setLED(leds);
}


/* end of hw_pidc.c */
