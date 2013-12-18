#include "pythag.h"
#include <math.h>
#include <stdlib.h>

/*
 * some handy macros for controlling the C40's timer
 */
#define ELAPSED_TIME( start, end )	(((end) - (start))*0.0000001)

#define GET_TIMER	(*(unsigned long *)0x00100024)
#define RESET_TIMER (*(unsigned long*)0x00100020 |= 960)

#define SET_PERIOD(X)	(*(unsigned long *)0x00100028 = (unsigned long) X)

/*
 * this is a temporary kludge since the HostInterrupt function of HyHoMon
 * is not working at the moment
 */
int intvec, intpri;
unsigned long *VIC_virsr = (unsigned long *) 0xbfff0020;
#define	HOST_INTERRUPT()			\
	*(VIC_virsr + intpri) = intvec;		\
	*(VIC_virsr) = ((1 << intpri) + 1);	\

/*
 * for speed, keep the input data in RAMBLK0 and output data in RAMBLK1
 */
float *in_addr = (float *)0x2ff800;
complex *out_addr = (complex *)0x2ffc00;

/*
 * Host will set the start flag when there is data ready to process
 */
int start_flag=0;

/*
 * Host will set the FFTSize so I know how big an FFT to compute
 */
int FFTSize;

/*
 * Host will read the elapsed time when I'm done
 */
float elapsed_time=0;

main()
{
    int i;
    unsigned long timerStart,timerEnd;

    GIEOn();   /* enable interrupts so that HyHoMon can work */

    SET_PERIOD(0xFFFFFFFF);    /* set timer period */

    /*
     * Process an FFT whenever the start_flag is set
     */
    while( 1 ) {
	/*
	 * wait for start flag to be set
	 */
	while( !start_flag );

	RESET_TIMER;
	timerStart = GET_TIMER;

	/*
	 * do the real FFT
	 */
	rfft(in_addr, out_addr, FFTSize, 1);

	timerEnd = GET_TIMER;   /* get timer value */

	elapsed_time = ELAPSED_TIME(timerStart,timerEnd);

	start_flag = 0;	    /* clear for next time */
	
        HOST_INTERRUPT();   /* send interrupt to host to signal completion */
    }
}
