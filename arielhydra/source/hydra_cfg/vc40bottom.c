/*
 * V-C40 driver bottom half - interrupt handling
 */
#include "vc40drv.h"

/*
 * SunOS Weirdness: if the driver is loadable, then the interrupt routine
 * is called with the actual vector number.  But if it is configured into the
 * kernel, then it is called with the unit number
 */

/****************************************************************************
 * vectored interrupt routine.  The kernel calls this routine directly
 */
#ifdef MODLOAD	/* driver is loadable */
vc_intr(vctr)
int vctr;
{
	register struct vc40_units_struct *vc40dsp;
	register int	i;

	/*
	 * find the DSP associated with this vector
	 */
	for (i=1; i<=num_dsp; i++) {
		vc40dsp = vc40_units[i];
		if (vc40dsp && vc40dsp->intvec == vctr) {
			if (vc40dsp->uproc && vc40dsp->usignum) {
				psignal(vc40dsp->uproc, vc40dsp->usignum);
				return;
			}
		}
	}
	ERROR_MSG(logit, 
		"vc40dsp: got unrecognized interrupt on vector 0x%x\n", vctr);
}

#else	/* driver is configured */

vc_intr(unit)
int unit;
{
	register struct vc40_units_struct *vc40dsp;

	vc40dsp = vc40_units[unit];
	if (vc40dsp->uproc && vc40dsp->usignum) {
		psignal(vc40dsp->uproc, vc40dsp->usignum);
		return;
	}
	ERROR_MSG(logit, 
		"vc40dsp: got unrecognized interrupt from unit %d\n", unit);
}
#endif /* MODLOAD */
