
/* fpprintf.c: ANSI draft (X3J11 Oct 86) part of section 4.9 code */
/* Copyright (C) Codemist Ltd, 1988 */

/* version 0.01c */

/* Full entrypoints (that support floating point conversions) live in this */
/* file so that the cutdown code can be used when only integer formats are */
/* being used.                                                             */

#define __system_io 1      /* makes stdio.h declare more */

#include "hostsys.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>

/* fpprintf wants to know about the system part of the FILE descriptor, but
   it doesn't need to know about _extradata */
typedef struct __extradata {void *dummy;} __extradata;

/* Beware the following type and extern must match the one in printf.c     */
typedef int (*fp_print)(int ch, double *d, char buff[], int flags,
                        char **lvprefix, int *lvprecision, int *lvbefore_dot,
                        int *lvafter_dot);

extern int __vfprintf(FILE *fp, const char *fmt, va_list args, fp_print fn);

/* The following macros must match those in printf.c:                    */
#define _LJUSTIFY         01
#define _SIGNED           02
#define _BLANKER          04
#define _VARIANT         010
#define _PRECGIVEN       020
#define _LONGSPECIFIER   040
#define _SHORTSPEC      0100
#define _PADZERO        0200    /* *** DEPRECATED FEATURE *** */
#define _FPCONV         0400

#ifndef NO_FLOATING_POINT

#ifdef IEEE
#define FLOATING_WIDTH 17
#else
#define FLOATING_WIDTH 18       /* upper bound for sensible precision    */
#endif

static int fp_round(char buff[], int len)
/* round (char form of) FP number - return 1 if carry, 0 otherwise       */
/* Note that 'len' should be <= 20 - see fp_digits()                     */
/* The caller ensures that buff[0] is always '0' so that carry is simple */
/* However, beware that this routine does not re-ensure this if carry!!  */
{   int ch;
    char *p = &buff[len];
    if ((ch = *p)==0) return 0;                      /* at end of string */
    if (ch < '5') return 0;                          /* round downwards  */
    if (ch == '5')                                   /* the dodgy case   */
    {   char *p1;
        for (p1 = p; (ch = *++p1)=='0';);
        if (ch==0) return 0;                         /* .5 ulp exactly   */
    }
    for (;;)
    {   ch = *--p;
        if (ch=='9') *p = '0';
        else
        {   *p = ch + 1;
            break;
        }
    }
    if (buff[0]!='0')           /* caused by rounding                    */
    {   int w;                  /* renormalize the number                */
        for (w=len; w>=0; w--) buff[w+1] = buff[w];
        return 1;
    }
    return 0;
}

#ifdef HOST_HAS_BCD_FLT

static int fp_digits(char *buff, double d)
/* This routine turns a 'double' into a character string representation of  */
/* its mantissa and returns the exponent after converting to base 10.       */
/* It guarantees that buff[0] = '0' to ease problems connected with         */
/* rounding and the like.  See also comment at first call.                  */
/* Use FPE2 convert-to-packed-decimal feature to do most of the work        */
/* The sign of d is returned in the LSB of x, and x has to be halved to     */
/* obtain the 'proper' value it needs.                                      */
{
    unsigned int a[3], w, d0, d1, d2, d3;
    int x, i;
    _stfp(d, a);
    w = a[0];
/* I allow for a four digit exponent even though sensible values can     */
/* only extend to 3 digits. I call this caution!                         */
    if ((w & 0x0ffff000) == 0x0ffff000)
    {   x = 999;    /* Infinity will print as 1.0e999 here */
                    /* as will NaNs                        */
        for (i = 0; i<20; i++) buff[i] = '0';
        buff[1] = '1';
    }
    else
    {   d0 = (w>>24) & 0xf;
        d1 = (w>>20) & 0xf;
        d2 = (w>>16) & 0xf;
        d3 = (w>>12) & 0xf;
        x = ((d0*10 + d1)*10 + d2)*10 + d3;
        if (w & 0x40000000) x = -x;
        buff[0] = '0';
        for (i = 1; i<4; i++) buff[i] = '0' + ((w>>(12-4*i)) & 0xf);
        w = a[1];
        for (i = 4; i<12; i++) buff[i] = '0' + ((w>>(44-4*i)) & 0xf);
        w = a[2];
        for (i = 12; i<20; i++) buff[i] = '0' + ((w>>(76-4*i)) & 0xf);
    }
    buff[20] = 0;
    x = x<<1;
    if (a[0] & 0x80000000) x |= 1;
    return x;
}

#else /* HOST_HAS_BCD_FLT */

static void pr_dec(int d, char *p, int n)
                                /* print d in decimal, field width n     */
{                               /* store result at p. arg small & +ve.   */
    while ((n--)>0)
    {   int dDiv10 = _kernel_sdiv10(d);
        *p-- = '0' + d - dDiv10 * 10;
        d = dDiv10;
    }
}

static int fp_digits(char *buff, double d)
/* This routine turns a 'double' into a character string representation of  */
/* its mantissa and returns the exponent after converting to base 10.       */
/* For this we use one-and-a-half precision done by steam                   */
/* It guarantees that buff[0] = '0' to ease problems connected with         */
/* rounding and the like.  See also comment at first call.                  */
{   int hi, mid, lo, dx, sign = 0;
    if (d < 0.0) d = -d, sign = 1;
    if (d==0.0) { hi = mid = lo = 0; dx = -5; }
    else
    {   double d1, d2, d2low, d3, d3low, scale;
        int w, bx;
        d1 = frexp(d, &bx);     /* exponent & mantissa   */
        /* fraction d1 is in range 0.5 to 1.0            */
        /* remember log_10(2) = 0.3010!                  */
        dx = (301*bx - 5500)/1000;   /* decimal exponent */
        scale = ldexp(1.0, dx-bx);
        w = dx;
        if (w < 0) { w = -w; d3 = 0.2; }
        else d3 = 5.0;

        if (w!=0) for (;;)      /* scale *= 5**dx        */
        {   if((w&1)!=0)
            {   scale *= d3;
                if (w==1) break;
            }
            d3 *= d3;
            w = w >> 1;
        }
        d2 = d1/scale;

/* the initial value selected for dx was computed on the basis of the    */
/* binary exponent in the argument value - now I refine dx. If the value */
/* produced to start with was accurate enough I will hardly have to do   */
/* any work here.                                                        */
        while (d2 < 100000.0)
        {   d2 *= 10.0;
            dx -= 1;
            scale /= 10.0;
        }
        while (d2 >= 1000000.0)
        {   d2 /= 10.0;
            dx += 1;
            scale *= 10.0;
        }
        hi = (int) d2;
        for (;;)               /* loop to get hi correct                 */
        {   d2 = ldexp((double) hi, dx-bx);
            /* at worst 24 bits in d2 here                               */
            /* even with IBM fp numbers there is no accuracy lost        */
            d2low = 0.0;
            w = dx;
            if (w<0)
            {   w = -w;
/* the code here needs to set (d3, d3low) to a one-and-a-half precision  */
/* version of the constant 0.2.                                          */
                d3 = 0.2;
                d3low = 0.0;
                _fp_normalize(d3, d3low);
                d3low = (1.0 - 5.0*d3)/5.0;
            }
            else
            {   d3 = 5.0;
                d3low = 0.0;
            }
/* Now I want to compute d2 = d2 * d3**dx in extra precision arithmetic  */
            if (w!=0) for (;;)
            {   if ((w&1)!=0)
                {   d2low = d2*d3low + d2low*(d3 + d3low);
                    d2 *= d3;
                    _fp_normalize(d2, d2low);
                    if (w==1) break;
                }
                d3low *= (2.0*d3 + d3low);
                d3 *= d3;
                _fp_normalize(d3, d3low);
                w = w>>1;
            }
            if (d2<=d1) break;
            hi -= 1;          /* hardly ever happens */
        }

        d1 -= d2;
              /* for this to be accurate d2 MUST be less */
              /* than d1 so that d1 does not get shifted */
              /* prior to the subtraction.               */
        d1 -= d2low;
        d1 /= scale;
/* Now d1 is a respectably accurate approximation for (d - (double)hi)   */
/* scaled by 10**dx                                                      */

        d1 *= 1000000.0;
        mid = (int) d1;
        d1 = 1000000.0 * (d1 - (double) mid);
        lo = (int) d1;

/* Now some postnormalization on the integer results                     */
/* If I do things this way the code will work if (int) d rounds or       */
/* truncates.                                                            */
        while (lo<0) { lo += 1000000; mid -= 1; }
        while (lo>=1000000) { lo -= 1000000; mid += 1; }
        while (mid<0) { mid += 1000000; hi -= 1; }
        while (mid>=1000000) { mid -= 1000000; hi += 1; }
        if (hi<100000)
        {   int loDiv100000; int midDiv100000 = _kernel_sdiv(100000, mid);
            hi = 10*hi + midDiv100000;
            loDiv100000 = _kernel_sdiv(100000, lo);
            mid = 10*(mid - midDiv100000 * 100000) + loDiv100000;
            lo = 10*(lo - loDiv100000 * 100000);
            dx -= 1;
        }
        else if (hi >= 1000000)
        {   int midDiv10; int hiDiv10 = _kernel_sdiv10(hi);
            mid += 1000000*(hi - hiDiv10 * 10);
            hi = hiDiv10;
            midDiv10 = _kernel_sdiv10(mid);
            lo += 1000000*(mid - midDiv10 * 10);
            mid = midDiv10;
            lo = _kernel_sdiv10(lo + 5);    /* pretence at rounding */
            dx += 1;
        }
    }

/* Now my result is in three 6-digit chunks (hi, mid, lo)                */
/* The number of characters put in the buffer here MUST agree with       */
/* FLOATING_PRECISION. This version is for FLOATING_PRECISION = 18.      */
    buff[0] = '0';
    pr_dec(hi,  &buff[6], 6);
    pr_dec(mid, &buff[12], 6);
    pr_dec(lo,  &buff[18], 6);
    buff[19] = '0';
    buff[20] = 0;
    return ((dx+5)<<1) | sign;
}

#endif /* HOST_HAS_BCD_FLT */

static int fp_addexp(char *buff, int len, int dx, int ch)
{   int dxDiv10;
    buff[len++] = ch;
    if (dx<0) { dx = -dx; buff[len++] = '-'; }
    else buff[len++] = '+';
    if (dx >= 1000)
    {   int dxDiv1000 = _kernel_sdiv(1000, dx);
        buff[len++] = '0' + dxDiv1000;
        dx = dx - dxDiv1000 * 1000;
    }
    if (dx >= 100)
    {   int dxDiv100 = _kernel_sdiv(100, dx);
        buff[len++] = '0' + dxDiv100;
        dx = dx - dxDiv100 * 100;
    }
    dxDiv10 = _kernel_sdiv10(dx);
    buff[len++] = '0' + dxDiv10;
    buff[len++] = '0' + dx - dxDiv10 * 10;
    return len;
}

#define fp_insert_(buff, pos, c)                    \
    {   int w;                                      \
        for (w=0; w<=pos; w++) buff[w] = buff[w+1]; \
        buff[pos+1] = c; }

static int fp_display(int ch, double *lvd, char buff[], int flags,
                      char **lvprefix, int *lvprecision, int *lvbefore_dot,
                      int *lvafter_dot)
{   int len = 0;
    double d = *lvd;
    switch (ch)
    {
/* The following code places characters in the buffer buff[]             */
/* to print the floating point number given as d.                        */
/* It is given flags that indicate what format is required and how       */
/* many digits precision are needed.                                     */
/* Floating point values are ALWAYS converted into 18 decimal digits     */
/* (the largest number possible reasonable) to start with, and rounding  */
/* is then performed on this character representation. This is intended  */
/* to avoid all possibility of boundary effects when numbers like .9999  */
/* are being displayed.                                                  */
    case 'f':
                {   int dx = fp_digits(buff, d);
                    if (dx & 1) *lvprefix = "-";
                    else
                        *lvprefix = (flags&_SIGNED) ? "+" :
                                 (flags&_BLANKER) ? " " : "";
                    dx = (dx & ~1)/2;
                    if (dx<0)
                    /* insert leading zeros */
                    {   dx = -dx;
                        if (dx>*lvprecision+1)
                        {   len = 0;       /* prints as zero */
                            buff[len++] = '0';
                            buff[len++] = '.';
                            *lvafter_dot = *lvprecision;
                        }
                        else
                        {   len = *lvprecision - dx + 2;
                            if (len > FLOATING_WIDTH + 1)
                            {   *lvafter_dot = len - (FLOATING_WIDTH + 2);
                                len = FLOATING_WIDTH+2;
                            }
                            if (fp_round(buff, len))
                                dx--, len++; /* dx-- because of negation */
/* unfortunately we may have dx=0 now because of the rounding            */
                            if (dx==0)
                            {   buff[0] = buff[1];
                                buff[1] = '.';
                            }
                            else if (dx==1)
                            {   int w;
                                for(w=len; w>0; w--) buff[w+1] = buff[w];
                                len += 1;
                                buff[0] = '0';
                                buff[1] = '.';
                            }
                            else
                            {   int w;
                                for(w=len; w>0; w--) buff[w+2] = buff[w];
                                len += 2;
                                buff[0] = '0';
                                buff[1] = '.';
                                buff[2] = '<';
                                *lvbefore_dot = dx - 1;
                            }
                        }
                        if (*lvafter_dot>0) buff[len++] = '>';
                    }
                    else /* dx >= 0 */
                    {   len = dx + *lvprecision + 2;
                        if (len > FLOATING_WIDTH+1)
                        {   len = FLOATING_WIDTH+2;
/* Seemingly endless fun here making sure that the number is printed     */
/* without truncation or loss even if it is very big & hence needs very  */
/* many digits. Only the first few digits will be significant, of course */
/* but the C specification forces me to print lots of insignificant ones */
/* too. Use flag characters '<' and '>' plus variables (before_dot) and  */
/* (after_dot) to keep track of what has happened.                       */
                            if (fp_round(buff, len))
                                dx++, len++;         /* number extended  */
                            if (dx<len-1)
                            {   fp_insert_(buff, dx, '.');
                                *lvafter_dot = dx + *lvprecision - FLOATING_WIDTH;
                                if (*lvafter_dot!=0) buff[len++] = '>';
                            }
                            else
                            {   int w;
                                for (w=0; w<len-1; w++) buff[w] = buff[w+1];
                                buff[len-1] = '<';
                                *lvbefore_dot = dx - len + 2;
                                buff[len++] = '.';
                                if (*lvprecision!=0)
                                {   *lvafter_dot = *lvprecision;
                                    buff[len++] = '>';
                                }
                            }
                        }
                        else
                        {   if (fp_round(buff, len))
                                dx++, len++;     /* number extended  */
                            fp_insert_(buff, dx, '.');
                        }
                    }
                    if ((*lvprecision==0) && ((flags&_VARIANT)==0)) len -= 1;
                }
                break;
    default:
/*
    case 'g': 
    case 'G':
*/
                {   int dx = fp_digits(buff, d);
                    if (dx & 1) *lvprefix = "-";
                    else
                        *lvprefix = (flags&_SIGNED) ? "+" :
                                 (flags&_BLANKER) ? " " : "";
                    dx = (dx & ~1)/2;
                    if (*lvprecision<1) *lvprecision = 1;
                    len = (*lvprecision>FLOATING_WIDTH) ? FLOATING_WIDTH+1 :
                                                       *lvprecision + 1;
                    dx += fp_round(buff, len);
/* now choose either 'e' or 'f' format, depending on which will lead to  */
/* the more compact display of the number.                               */
                    if ((dx>=*lvprecision) || (dx<-4))
                    {   buff[0] = buff[1];          /* e or E format */
                        buff[1] = '.';
                    }
                    else
                    {   ch = 'f';                   /* uses f format */
                        if (dx>=0)
/* Insert a decimal point at the correct place for 'f' format printing   */
                        {   fp_insert_(buff, dx, '.')
                        }
                        else
/* If the exponent is negative the required format will be something     */
/* like 0.xxxx, 0.0xxx or 0.00xx and I need to lengthen the buffer       */
                        {   int w;
                            dx = -dx;
                            for (w=len; w>=0; w--) buff[w+dx] = buff[w];
                            len += dx;
                            for(w=0; w<=dx; w++) buff[w] = '0';
                            buff[1] = '.';
                        }
                    }
                    if((flags&_VARIANT)==0)         /* trailing 0?   */
                    {   *lvafter_dot = -1;
                        if (buff[len]!='.') while (buff[len-1]=='0') len--;
                        if (buff[len-1]=='.') len--;
                    }
                    else
/* Allow for the fact that the specified precision may be very large in  */
/* which case I put in trailing zeros via the marker character '>' and a */
/* count (after_dot). Not applicable unless the '#' flag has been given  */
/* since without '#' trailing zeros in the fraction are killed.          */
                    {   if (*lvprecision>FLOATING_WIDTH)
                        {   *lvafter_dot = *lvprecision - FLOATING_WIDTH;
                            buff[len++] = '>';
                        }
                    }
                    if (ch!='f')    /* sets 'f' if it prints in f format */
                                    /* and 'e' or 'E' if in e format.    */
                        len = fp_addexp(buff, len, dx, ch + ('e'-'g'));
                }
                break;
    case 'e':
    case 'E':
                {   int dx = fp_digits(buff, d);
                    if (dx & 1) *lvprefix = "-";
                    else
                        *lvprefix = (flags&_SIGNED) ? "+" :
                                 (flags&_BLANKER) ? " " : "";
                    dx = (dx & ~1)/2;
                    if (*lvprecision>FLOATING_WIDTH)
                    {   *lvafter_dot = *lvprecision - FLOATING_WIDTH;
                        *lvprecision = FLOATING_WIDTH;
                    }
                    len = *lvprecision + 2;
                    dx += fp_round(buff, len);
                    buff[0] = buff[1];
                    if ((*lvprecision==0) && !(flags&_VARIANT)) len = 1;
                    else buff[1] = '.';
/* Deal with trailing zeros for excessive precision requests             */
                    if (*lvafter_dot>0) buff[len++] = '>';
                    len = fp_addexp(buff, len, dx, ch);
                }
                break;
    }
    return len;
}

#else /* NO_FLOATING_POINT */

static int fp_display(int ch, double *lvd, char buff[], int flags,
                      char **lvprefix, int *lvprecision, int *lvbefore_dot,
                      int *lvafter_dot)
{
#error NO_FLOATING_POINT option mangled
}

#endif /* NO_FLOATING_POINT */


int fprintf(FILE *fp, const char *fmt, ...)
{
    va_list a;
    int n;
    va_start(a, fmt);
    n = __vfprintf(fp, fmt, a, fp_display);
    va_end(a);
    return n;
}

int printf(const char *fmt, ...)
{
    va_list a;
    int n;
    va_start(a, fmt);
    n = __vfprintf(stdout, fmt, a, fp_display);
    va_end(a);
    return n;
}

int sprintf(char *buff, const char *fmt, ...)
{
    FILE hack;
    va_list a;
/*************************************************************************/
/* Note that this code interacts in a dubious way with the putc macro.   */
/*************************************************************************/
    int length;
    va_start(a, fmt);
    memclr(&hack, sizeof(FILE));
    hack.__flag = _IOSTRG+_IOWRITE;
    hack.__ptr = (unsigned char *)buff;
    hack.__ocnt = 0x7fffffff;
    length = __vfprintf(&hack, fmt, a, fp_display);
    putc(0,&hack);
    va_end(a);
    return(length);
}

int vfprintf(FILE *p, const char *fmt, va_list args)
{
    return __vfprintf(p, fmt, args, fp_display);
}

int vprintf(const char *fmt, va_list a)
{
    return __vfprintf(stdout, fmt, a, fp_display);
}

int vsprintf(char *buff, const char *fmt, va_list a)
{
    FILE hack;
/*************************************************************************/
/* Note that this code interacts in a dubious way with the putc macro.   */
/*************************************************************************/
    int length;
    memclr(&hack, sizeof(FILE));
    hack.__flag = _IOSTRG+_IOWRITE;
    hack.__ptr = (unsigned char *)buff;
    hack.__ocnt = 0x7fffffff;
    length = __vfprintf(&hack, fmt, a, fp_display);
    putc(0,&hack);
    return(length);
}


/* End of fpprintf.c */
