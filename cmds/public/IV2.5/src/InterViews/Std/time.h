/*
 * C++ interface to system time routines
 */

#ifndef time_h

#define time time_h_time
#define ctime time_h_ctime
#define localtime time_h_localtime
#define gmtime time_h_gmtime
#define asctime time_h_asctime
#define tzset time_h_tzset

#include "//usr/include/time.h"

#undef time
#undef ctime
#undef localtime
#undef gmtime
#undef asctime
#undef tzset

/* just in case standard header didn't */
#ifndef time_h
#define time_h
#endif

extern long time(long*);
extern char* ctime(long*);
extern struct tm* localtime(long*);
extern struct tm* gmtime(long*);
extern char* asctime(struct tm*);

/*
 * Can't use the timezone name here because it may have been used in <time.h>.
 * Can't redefine it like other symbols because we need the timezone struct.
 */

#if defined(SYSTEM_FIVE)

extern long time_zone;
extern int daylight;
extern char* tzname[2];
extern void tzset();

#else

extern char* time_zone(int zone, int dst);

#endif

#endif
