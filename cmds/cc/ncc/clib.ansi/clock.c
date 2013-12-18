#include "stdio.h"

typedef	long	time_t;

/*
 * Structure returned by times()
 */
struct tms {
	time_t	tms_utime;		/* user time */
	time_t	tms_stime;		/* system time */
	time_t	tms_cutime;		/* user time, children */
	time_t	tms_cstime;		/* system time, children */
};

void times(struct tms*);

unsigned int clock(void)
{
        struct tms              tms;

	times(&tms);
	return((unsigned int)tms.tms_utime);
}

