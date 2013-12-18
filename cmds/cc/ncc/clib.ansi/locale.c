
/* locale.c: ANSI draft (X3J11 Oct 86) library header, section 4.3 */
/* Copyright (C) Codemist Ltd., 1988 */
/* version 0.01 */

#include <locale.h>
#include <stddef.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>  /* multibyte characters & strings */

/* #define LC_COLLATE  1
   #define LC_CTYPE    2
   #define LC_MONETARY 4
   #define LC_NUMERIC  8
   #define LC_TIME    16
   #define LC_ALL     31
*/

#define C_LOCALE "C"

static char Clocale[] = C_LOCALE;
static char iso_8859_1[] = "ISO8859-1";
static char *locales[5] = {C_LOCALE, C_LOCALE, C_LOCALE, C_LOCALE, C_LOCALE};
static char *lc_all = C_LOCALE;
static struct lconv lc =
{".", ",", "\3", "STG", "`", ".", ",", "\3", "", "-", 2, 2, 1, 0, 1, 0, 1, 2};

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

static int index_of(bitset)
int bitset;
{
  int j = 0;
  /* return index of ls bit */
  bitset &= (-bitset);
  for (;;) {
    bitset >>= 1;
    if (bitset == 0) return j;
    ++j;
  }
}

extern void _set_ctype_8859(int yes_or_no);

char *setlocale(int category, const char *locale)
{
    /* I expect the category to be a bit-map - complain if out of range  */
    if (((unsigned)category > LC_ALL) || (category == 0))
      /* no can do... */
      return NULL;
    if ((locale == 0) || (locale[0] == 0)) {
      /* get locale */
      if (category == LC_ALL)
        return lc_all;
      else
        return locales[index_of(category)];
    } else {
      /* set locale */
      if (strcmp(locale, "C") == 0) {
        /* The system default is Clocale */
        int j = 0;
        if (category & LC_CTYPE) _set_ctype_8859(0);
        while (category) {
          if (category & 1) locales[j] = Clocale;
          category >>= 1;  ++j;
        }
        lc_all = Clocale;
        for (j = 0;  j < sizeof(locales) / sizeof(char *);  ++j) {
          if (locales[j] != Clocale) lc_all = NULL;
        }
        return Clocale;
      } else if ((strcmp(locale, iso_8859_1) == 0) && (category == LC_CTYPE)) {
        lc_all = NULL;
        _set_ctype_8859(1);
        return locales[index_of(LC_CTYPE)] = iso_8859_1;
      }
      /* currently "C" is the only acceptable locale */
    }
    return NULL;
}

struct lconv *localeconv(void)
{
  return &lc;
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
                sprintf(ss, "%02d.%02d.%02d %02d:%02d:%02d",
                    tt->tm_mday, tt->tm_mon+1, tt->tm_year,
                    tt->tm_hour, tt->tm_min, tt->tm_sec);
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
                sprintf(ss, "%02d.%02d.%02d",
                    tt->tm_mday, tt->tm_mon+1, tt->tm_year);
                break;
        case 'X':
                sprintf(ss, "%02d:%02d:%02d", tt->tm_hour, tt->tm_min, tt->tm_sec);
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

#define STATE_DEPENDENT_ENCODINGS 0

int mblen(const char *s, size_t n)
{   if (s == 0) return STATE_DEPENDENT_ENCODINGS;
/* @@@ ANSI ambiguity: if n=0 and *s=0 then return 0 or -1?                 */
/* @@@ LDS: for consistency with mbtowc, return -1                          */
    if (n == 0) return -1;
    if (*s == 0) return 0;
    return 1;
}

int mbtowc(wchar_t *pwc, const char *s, size_t n)
{   if (s == 0) return STATE_DEPENDENT_ENCODINGS;
/* @@@ ANSI ambiguity: if n=0 and *s=0 then return 0 or -1?                 */
/* @@@ LDS At most n chars of s are examined, ergo must return -1.          */
    if (n == 0) return -1;
    else
    {   wchar_t wc = *(unsigned char *)s;
        if (pwc) *pwc = wc;
        return (wc != 0);
    }
}

int wctomb(char *s, wchar_t w)
{   if (s == 0) return STATE_DEPENDENT_ENCODINGS;
/* @@@ ANSI ambiguity: what return (and setting for s) if w == 0?           */
    if ((unsigned)w > (unsigned char)-1) return -1;
    if ((*s = w) == 0) return 0;
    return 1;
}

size_t mbstowcs(wchar_t *pwcs, const char *s, size_t n)
{
/* @@@ ANSI ambiguity: if n=0 then is *s read?                              */
    size_t r = 0;
    for (; n != 0; n--)
    {   if ((pwcs[r] = ((unsigned char *)s)[r]) == 0) return r;
        r++;
    }
    return r;
}

size_t wcstombs(char *s, const wchar_t *pwcs, size_t n)
{
/* @@@ ANSI ambiguity: if n=0 then is *pwcs read?  Also invalidity check?   */
    size_t r = 0;
    for (; n != 0; n--)
    {   wchar_t w = pwcs[r];
        if ((unsigned)w > (unsigned char)-1) return (size_t)-1;
        if ((s[r] = w) == 0) return r;
        r++;
    }
    return r;
}

/* end of locale.c */
