/* time.h: ANSI draft (X3J11 Oct 86 draft) library header, section 4.12 */
/* Copyright (C) A.C. Norman and A. Mycroft */
/* version 0.01 - SccsId: %W% %G% */
/* $Id: time.h,v 1.2 91/02/06 13:47:11 martyn Exp $ */

#ifndef __time_h
#define __time_h

#ifndef size_t
#  define size_t unsigned int
#endif

#ifndef CLK_TCK
#define CLK_TCK 100              /* for the BBC                          */
#endif

#ifndef _types_h		 /* only if sys/types.h not included	 */
typedef unsigned int clock_t;    /* cpu time type - in centisecs on bbc  */
typedef unsigned int time_t;     /* date/time in unix secs past 1-Jan-70 */
#endif

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

extern clock_t clock(void);
extern double difftime(time_t time1, time_t time0);
extern time_t mktime(struct tm *timeptr);
extern time_t time(time_t *timer);

extern char *asctime(const struct tm *timeptr);
extern char *ctime(const time_t *timer);
extern struct tm *gmtime(const time_t *timer);
extern struct tm *localtime(const time_t *timer);
extern size_t strftime(char *s, size_t maxsize,
                       const char *format, const struct tm *timeptr);

#endif

/* end of time.h */
