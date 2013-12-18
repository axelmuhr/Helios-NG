
/* time.h: ANSI draft (X3J11 Oct 86 draft) library header, section 4.12 */
/* Copyright (C) A.C. Norman and A. Mycroft */
/* version 0.01 */

#ifndef __time_h
#define __time_h

#ifndef size_t
#  define size_t unsigned int
#endif

#define CLK_TCK 100              /* for the BBC                          */

typedef unsigned int clock_t;    /* cpu time type - in centisecs on bbc  */
typedef unsigned int time_t;     /* date/time in unix secs past 1-Jan-70 */

struct tm {
  int tm_sec;
  int tm_min;
  int tm_hour;
  int tm_mday;
  int tm_mon;
  int tm_year;
  int tm_wday;
  int tm_yday;
  int tm_isdst;
};

extern clock_t clock();
extern double difftime();
extern time_t mktime();
extern time_t time();

extern char *asctime();
extern char *ctime();
extern struct tm *gmtime();
extern struct tm *localtime();
extern size_t strftime();

#endif

/* end of time.h */
