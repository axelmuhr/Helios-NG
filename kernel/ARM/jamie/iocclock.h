/*----------------------------------------------------------------------*/
/*-- Clock -------------------------------------------------------------*/

#ifndef __IOCclock_h
#define __IOCclock_h

/*----------------------------------------------------------------------*/

#ifndef IOC
#define	IOC	((ioc_block *)IOC_BASE_ADDRESS)
#endif /* NOT IOC */

/*----------------------------------------------------------------------*/
/*
 * Macro to give current clock counter reading, as -(microseconds left
 * BEFORE next clock tick) (NB NOT usec since last tick).  This is what
 * mfpr(ICR) on the VAX gives.
 */
#define MICROTIME() (IOC->timer_0.latch_cmd = 0, -((IOC->timer_0.count_hi << 8 | IOC->timer_0.count_lo) >> 1))

/*----------------------------------------------------------------------*/
/*
 * Macro to decide if clock has wrapped but the interrupt has not yet
 * been serviced (because CPU is at >= PRIO_CLOCK).  This is equivalent
 * to the VAX code ((mfpr(ICCS) & ICCS_INT) != 0)
 */
#define CLOCKINTPENDING() ((IOC->irq_A.status & IRQA_TM0) != 0)

/*----------------------------------------------------------------------*/

#endif /* NOT __IOCclock_h */

/*----------------------------------------------------------------------*/
