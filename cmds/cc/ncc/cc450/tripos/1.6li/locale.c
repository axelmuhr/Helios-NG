
/* locale.c: ANSI draft (X3J11 Oct 86) library header, section 4.3 */
/* Copyright (C) A. Mycroft and A.C. Norman */
/* version 0.01 */

#include <locale.h>
#include <stddef.h>
#include <stdio.h>
#include <time.h>

/* #define LC_COLLATE  1
   #define LC_CTYPE    2
   #define LC_NUMBERIC 4
   #define LC_TIME     8
   #define LC_ALL     15
*/

static char Clocale[] = "C";

/* Tables used by strftime()                                             */

static char *abbrweek[]  = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
static char *fullweek[]  = { "Sunday", "Monday", "Tuesday", "Wednesday",
                             "Thursday", "Friday", "Saturday" };
static char *abbrmonth[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun",
                             "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
static char *fullmonth[] = { "January", "February", "March", "April",
                             "May", "June", "July", "August",
                             "September", "October", "November", "December" };
static char *ampmname[]  = { "AM", "PM" };

char *setlocale(int category, const char *locale)
{
    /* I expect the category to be a bit-map - complain if out of range  */
    if ((unsigned)category > LC_ALL) return NULL;
    if (locale == 0) return Clocale;
    if (locale[0] == 0) return Clocale;
    /* The system default is Clocale */
    if (locale[0] == 'C' && locale[1] == 0) return Clocale;
    /* currently "C" is the only acceptable locale */
    return NULL;
}

static int findweek(int yday, int startday, int today)
{
    int days_into_this_week = today - startday;
    int last_weekstart;
    if (days_into_this_week < 0) days_into_this_week += 7;
    last_weekstart = yday - days_into_this_week;
    if (last_weekstart <= 0) return 1;
    return last_weekstart/7 + 1;
}

size_t strftime(char *s, size_t maxsize, const char *fmt, const struct tm *tt)
{
    int p = 0, c;
    char *ss, buff[24];
    if (maxsize==0) return 0;
#define push(ch) { s[p++]=(ch); if (p>=maxsize) return 0; }
    for (;;)
    {   switch (c = *fmt++)
        {
    case 0: s[p] = 0;
            return p;
    default:
            push(c);
            continue;
    case '%':
            ss = buff;
            switch (c = *fmt++)
            {
        default:            /* Unknown directive - leave uninterpreted   */
                push('%');  /* NB undefined behaviour according to ANSI  */
                fmt--;
                continue;
        case 'a':
                ss = abbrweek[tt->tm_wday];
                break;
        case 'A':
                ss = fullweek[tt->tm_wday];
                break;
        case 'b':
                ss = abbrmonth[tt->tm_mon];
                break;
        case 'B':
                ss = fullmonth[tt->tm_mon];
                break;
        case 'c':
/* Is this the locale-specific date & time format we want?               */
                sprintf(ss, "%d.%d.%d %d:%d",
                    tt->tm_mday, tt->tm_mon, tt->tm_year,
                    tt->tm_hour, tt->tm_min);
                break;
        case 'd':
                sprintf(ss, "%.2d", tt->tm_mday);
                break;
        case 'H':
                sprintf(ss, "%.2d", tt->tm_hour);
                break;
        case 'I':
                sprintf(ss, "%.2d", (tt->tm_hour + 11)%12 + 1);
                break;
        case 'j':
                sprintf(ss, "%.3d", tt->tm_yday + 1);
                break;
        case 'm':
                sprintf(ss, "%.2d", tt->tm_mon + 1);
                break;
        case 'M':
                sprintf(ss, "%.2d", tt->tm_min);
                break;
        case 'p':
/* I am worried here re 12.00 AM/PM and times near same.                 */
                if (tt->tm_hour < 12) ss = ampmname[0];
                else ss = ampmname[1];
                break;
        case 'S':
                sprintf(ss, "%.2d", tt->tm_sec);
                break;
        case 'U':
                sprintf(ss, "%.2d", findweek(tt->tm_yday, 0, tt->tm_wday));
                break;
        case 'w':
                sprintf(ss, "%.1d", tt->tm_wday);
                break;
        case 'W':
                sprintf(ss, "%.2d", findweek(tt->tm_yday, 1, tt->tm_wday));
                break;
        case 'x':
/* The next two had better agree with %c conversions                     */
                sprintf(ss, "%d.%d.%d",
                    tt->tm_mday, tt->tm_mon, tt->tm_year);
                break;
        case 'X':
                sprintf(ss, "%d:%d", tt->tm_hour, tt->tm_min);
                break;
        case 'y':
                sprintf(ss, "%.2d", tt->tm_year % 100);
                break;
        case 'Y':
                sprintf(ss, "%d", 1900 + tt->tm_year);
                break;
        case 'Z':
                /* No timezone exists here */
                continue;
        case '%':
                push('%');
                continue;
            }
            while ((c = *ss++) != 0) push(c);
            continue;
        }
#undef push
    }
}

/* end of locale.c */
