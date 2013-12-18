/* Quick test of VLSI PID's interrupt handling system
 * Sets up an interrupt handler that waits for the 'panic' button
 * on the PID to be pressed and then prints a message.
 */

#include <arm.h>
#include <event.h>
#include <root.h>
#include <ARM/vy86pid.h>
#include <ARM/trap.h>

word AsmHandler(Semaphore  *, word);
word PanicHandler(Semaphore *, word);

int
  main(void) {
	Event e;
	Semaphore psem;
	RootStruct *rs;

	rs = GetRoot();

	e.Pri = 1;
#ifdef __VY86PID
	e.Vector = INTR_IRQ_PANIC;
#endif
	
	e.Code = PanicHandler;
	e.Data = &psem;

	SetEvent(&e);

/* Not required as PANIC button interrupot is always enabled. */
/*	hw_INTC->IRQM = rs->IRQM_softcopy |= hw_intc_enableirq_panic;*/

	InitSemaphore(&psem, 0);

#ifdef __VY86PID
	IOdebug("pid mask %x, status %x", rs->IRQM_softcopy , hw_INTC->IRQS);
#endif
	
	while (TRUE) {

		IOdebug("Waiting for Panic to be pressed");
#if 1
#ifdef NEW_SYSTEM
		Wait(&psem);
#else
		HardenedWait(&psem);
#endif
		IOdebug("Panic WAS pressed.");
#else
		if (TimedWait(&psem, OneSec))
			IOdebug("Panic WAS pressed.");
		else
			IOdebug("Panic was NOT pressed.");
#endif
	}
}



#pragma no_check_stack	/* MUST be used for C based interrupt handlers */

word PanicHandler(Semaphore *psem, word vec) {

	/* Reset interrupt */
	hw_INTC->IRQRST = hw_intc_resetirq_panic;

#if 0
	/* Cannot call kernel functions directly in interrupt handlers */
	/* This will re-enable interrupts and cause chaos. */
	HardenedSignal(psem);
#else
	PseudoTrap((word)psem, 0, 0, TRAP_HardenedSignal);
#endif

	return TRUE;
}
