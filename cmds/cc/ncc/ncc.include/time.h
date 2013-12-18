#ifndef NULL
#define NULL ((void *)0)
#endif

#define CLK_TCK  1000000

#ifndef _SIZE_T
#define _SIZE_T
typedef unsigned size_t;
#endif

#ifndef _CLOCK_T
#define _CLOCK_T
typedef long int clock_t;
#endif

#ifndef _TIME_T
#define _TIME_T
typedef long int time_t;
#endif

#ifndef _STRUCT_TM
#define _STRUCT_TM
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
#endif

clock_t clock(void);

double difftime(time_t time1, time_t time0);

time_t mktime(struct tm *timeptr);

time_t time(time_t *timer);

char *asctime(const struct tm *timeptr);

char *ctime(const time_t *timer);

struct tm *gmtime(const time_t *timer);

struct tm *localtime(const time_t *timer);

size_t strftime(
  char *s, size_t maxsize, const char *format, const struct tm *timeptr
);
