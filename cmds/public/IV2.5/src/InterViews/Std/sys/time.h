/*
 * C++ interface to Unix system time calls
 */

#ifndef sys_time_h

#define gettimeofday sys_time_h_gettimeofday
#define settimeofday sys_time_h_settimeofday

/*
 * Some systems (e.g., Ultrix) replicate <time.h> functions in <sys/time.h>.
 */

#define time time_h_time
#define ctime time_h_ctime
#define localtime time_h_localtime
#define gmtime time_h_gmtime
#define asctime time_h_asctime
#define tzset time_h_tzset

#include "//usr/include/sys/time.h"

#undef gettimeofday
#undef settimeofday

#undef time
#undef ctime
#undef localtime
#undef gmtime
#undef asctime
#undef tzset

/* just in case standard header didn't */
#ifndef sys_time_h
#define sys_time_h
#endif

/*
 * BSD-style get/set date and time
 */

extern int gettimeofday(struct timeval*, struct timezone*);
extern int settimeofday(struct timeval*, struct timezone*);

#endif
