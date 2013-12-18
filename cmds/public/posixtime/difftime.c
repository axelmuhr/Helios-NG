#ifndef lint
#ifndef NOID
static char	elsieid[] = "@(#)difftime.c	7.1";
#endif /* !defined NOID */
#endif /* !defined lint */

/*LINTLIBRARY*/

#include "private.h"

double
difftime(time1, time0)
const time_t	time1;
const time_t	time0;
{
	return time1 - time0;
}
