
/* time.c: ANSI draft (X3J11 Oct 86) section 4.12 code */
/* Copyright (C) A.C. Norman and A. Mycroft */
/* version 0.02a */
/* $Id: time.c,v 1.2 1992/08/25 17:15:58 craig Exp $ */

#include <time.h>
#include <stdio.h>
#include <string.h>

/* NB strftime() is in locale.c since it seems VERY locale-dependent      */

/* In NorCroft C time() yields the unix result of an unsigned int holding */
/* seconds since 1 Jan 1970.                                              */
/* clock() returns an unsigned int with ticks of cpu time.                */

/* N.B. clock() and time() are defined in opsys.c                         */

static int monlen[13] = { 31,29,31,30,31,30,31,31,30,31,30,31,0x40000000 };

                          /* BLV - added conditional compilation */
                          /* I wonder where it gets called from */
#if 0
/* now defined in fpprintf.c */
#ifndef NO_FLOATING_POINT
double difftime(time_t time1, time_t time0)
{   return (double)time1 - (double)time0;
}
#endif
#endif


static int tm_carry(int *a, int b, int q)
{   /* *a = (*a + b) % q, return (*a + b)/q.  Care with overflow.          */
    int aa = *a;
    int hi = (aa >> 16) + (b >> 16);    /* NB signed shift arithmetic here */
    int lo = (aa & 0xffff) + (b & 0xffff);
    lo += (hi % q) << 16;
    hi = hi / q;
    aa = lo % q;
    lo = lo / q;
    while (aa < 0)
    {   aa += q;
        lo -= 1;
    }
    *a = aa;        /* remainder is positive here */
    return (hi << 16) + lo;
}

time_t mktime(struct tm *timeptr)
{   /* the Oct 1986 ANSI draft spec allows ANY values for the contents     */
    /* of timeptr.  This leave the question - what is month -9 or +123?    */
    /* the code below resolves it in one way:                              */
    /* Also note that struct tm is allowed to have signed values in it for */
    /* the purposes of this function even though normalized times all have */
    /* just positive entries.                                              */
    time_t t;
    int w, v, yday;
    int sec = timeptr->tm_sec;
    int min = timeptr->tm_min;
    int hour = timeptr->tm_hour;
    int mday = timeptr->tm_mday;
    int mon = timeptr->tm_mon;
    int year = timeptr->tm_year;
    int quadyear = 0;
/* The next line applies a simple test that detects some gross overflows */
    if (year > 0x40000000 || year < -0x40000000) return (time_t)-1;

    /* we really do have to propagate carries up it seems                  */
    /* careful about overflow for divide, but not carry add.               */

    w = tm_carry(&sec,0,60);    /* leaves 0 <= sec < 60  */
    w = tm_carry(&min,w,60);    /* leaves 0 <= min < 60  */
    w = tm_carry(&hour,w,24);   /* leaves 0 <= hour < 24 */
    quadyear = tm_carry(&mday,w - 1,(4*365+1));  /* 0 <= mday < 4 years    */
/* The next line can not possibly result in year overflowing since the     */
/* initial values was checked earlier and the month can only cause a       */
/* carry of size up to MAXINT/12 with quadyear limited to MAXINT/365.      */
    year += quadyear*4 + tm_carry(&mon,0,12);
    /* at last the mday is in 0..4*365 and the mon in 0..11                */

#define notleapyear(year) (((year) & 3)!=0)
/* Note that 1900 is not in the range of valid dates and so I will fudge   */
/* the issue about it not being a leap year.                               */

    while (mday >= monlen[mon])
    {   mday -= monlen[mon++];
        if (mon==2 && notleapyear(year)) mday++;
        else if (mon == 12) mon = 0, year++;
    }
    if (mon==1 && mday==28 && notleapyear(year)) mon++, mday=0;

#define YEARS (0x7fffffff/60/60/24/365 + 1)
    if (year < 70 || year > 70+2*YEARS) return (time_t)-1;
#undef YEARS

    yday = mday;
    {   int i;
        for (i = 0; i<mon; i++) yday += monlen[i];
    }
    if (mon > 1 && notleapyear(year)) yday--;

    v = (365*4+1)*(year/4) + 365*(year & 3) + yday;
    if (!notleapyear(year)) v--;
/* v is now the number of days since 1 Jan 1900, and I have subtracted a   */
/* sly 1 to adjust for 1900 not being a leap year.                         */

#undef notleapyear

/* Adjust for a base at 1 Jan 1970                                       */

#define DAYS (17*(365*4+1)+2*365)
    t = min + 60*(hour + 24*(v - DAYS));
#undef DAYS
    {   int thi = ((int)t >> 16)*60;
        int tlo = ((int)t & 0xffff)*60 + sec;
        thi += (tlo >> 16) & 0xffff;
        t = (time_t)((thi << 16) | (tlo & 0xffff));
        if ((thi & 0xffff0000) != 0) return (time_t)-1;
    }

    timeptr->tm_sec = sec;
    timeptr->tm_min = min;
    timeptr->tm_hour = hour;
    timeptr->tm_mday = mday + 1;
    timeptr->tm_mon = mon;
    timeptr->tm_year = year;
    timeptr->tm_wday = (v + 1) % 7;
    timeptr->tm_yday = yday;
    timeptr->tm_isdst = -1;                  /* unavailable */

    return t;
    /* Now I know why Unix didn't have this                              */
}

/* get past Helios/ARM compiler bug - no statics in fns */
static char _timebuf[26+(8+3*9+7)];  /* slop in case illegal args */

char *asctime(const struct tm *timeptr)
{
    sprintf(_timebuf, "%.3s %.3s%3d %.2d:%.2d:%.2d %d\n",
       "SunMonTueWedThuFriSat" + (timeptr -> tm_wday)*3,
       "JanFebMarAprMayJunJulAugSepOctNovDecBad" + (timeptr -> tm_mon)*3,
       timeptr -> tm_mday,
       timeptr -> tm_hour, timeptr -> tm_min, timeptr -> tm_sec,
       timeptr -> tm_year + 1900);
    return _timebuf;
}

char *ctime(const time_t *timer)
{   return asctime(localtime(timer));
}

struct tm *gmtime(const time_t *timer)
/*                           ========== GMT not available ==========     */
{
#if 0
/* Artificial code so that *timer is referenced (to avoid compiler moan) */
    if (*timer) return 0;
    else return 0;
#else
/*
-- crf: 25/08/92 - Bug 829
-- Decided (after discussion with MJT and BLV) that gmtime() should return
-- localtime()
*/
	return (localtime(timer)) ;
#endif
}

/* get past Helios/ARM compiler bug - no statics in fns */
static struct tm _tms;

struct tm *localtime(const time_t *timer)
{   time_t t = *timer;
    int i = 0, yr;
/* treat unset dates as 1-Jan-1900 - any better ideas? */
    if (t == (time_t)-1) memset(&_tms, 0, sizeof(_tms)), _tms.tm_mday = 1;
    else
    {   /* unix time already in seconds (since 1-Jan-1970) ... */
        _tms.tm_sec = t % 60; t /= 60;
        _tms.tm_min = t % 60; t /= 60;
        _tms.tm_hour = t % 24; t /= 24;
/* The next line converts *timer arg into days since 1-Jan-1900 from t which
   now holds days since 1-Jan-1970.  Now there are really only 17 leap years
   in this range 04,08,...,68 but we use 18 so that we do not have to do
   special case code for 1900 which was not a leap year.  Of course this
   cannot give problems as pre-1970 times are not representable in *timer. */
        t += 70*365 + 18;
        _tms.tm_wday = t % 7;               /* it just happens to be so */
        yr = 4 * (t / (365*4+1)); t %= (365*4+1);
        if (t >= 366) yr += (t-1) / 365, t = (t-1) % 365;
        _tms.tm_year = yr;
        _tms.tm_yday = t;
        if ((yr & 3) != 0 && t >= 31+28) t++;
        while (t >= monlen[i]) t -= monlen[i++];
        _tms.tm_mday = t+1;
        _tms.tm_mon = i;
        _tms.tm_isdst = -1;                  /* unavailable */
    }
    return &_tms;
}

/* end of time.c */
