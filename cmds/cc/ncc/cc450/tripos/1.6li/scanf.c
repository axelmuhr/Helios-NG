
/* scanf.c: ANSI draft (X3J11 Oct 86) part of section 4.9 code */
/* Copyright (C) A.C. Norman and A. Mycroft */

/* version 4a */
/* BEWARE: there are quite a few ambiguities/oddities in the Oct 86 ANSI   */
/* draft definition of scanf().                                            */
/* Memo: consider using ftell() (or rather fgetpos() for big file worries) */
/* one day instead of charcount below.  See also 'countgetc()'.            */
/* Memo 2: the code below always reads one char beyond the end of the      */
/* item to be converted.  The exception is '%c' (q.v.).                    */
/* The last char is then ungetc'd.  This is                                */
/* necessary if no field width is given(!) but maybe should be altered for */
/* given or default maximum field width.  However, this only matters for   */
/* interactive streams so we await any further ANSI change (AM to comment) */
/* Beware further ANSI possibility of insisting that scanf() does not      */
/* use up the 1 char ungetc() guaranteed at other times.                   */

#include "hostsys.h"
#include <stdio.h>       /* we define scanf for this        */
#include <stdlib.h>      /* and strtol/strtoul etc for this */
#include <math.h>
#include <string.h>
#include <ctype.h>
#include <stddef.h>
#include <stdarg.h>
#include <limits.h>

#define NOSTORE      01
#define LONG         02
#define SHORT        04
#define FIELDGIVEN  010
#define LONGDOUBLE  020
#define ALLOWSIGN   040    /* + or - acceptable to rd_int  */
#define DOTSEEN    0100    /* internal to rd_real */
#define NEGEXP     0200    /* internal to rd_real */
#define NUMOK      0400    /* ditto + rd_int */
#define NUMNEG    01000    /* ditto + rd_int */

#define countgetc(p) (charcount++, getc(p))


/* The next macros, with the help of the compiler, ensures that we can */
/* test for LONG and SHORT properly, but not general extra code.       */
#define isLONG_(flag) ((flag) & LONG && sizeof(int) != sizeof(long))
#define isSHORT_(flag) ((flag) & SHORT)
#define isLONGDOUBLE_(flag) \
    ((flag) & LONGDOUBLE && sizeof(double) != sizeof(long double))

#define CVTEOF     (-1)   /* used for eof return (!= any number of chars)   */
#define CVTFAIL    (-2)   /* used for error return (!= any number of chars) */

#define scanf_intofdigit(c) ((c) - '0')

static int ch_val(int ch, int radix)
{
    int val;
    /* Memo: beware that ANSI currently say a-z act as 10-35 for strtol() */
    /* etc.  The test below is isalpha() etc.  This means that this test  */
    /* may not work in a non-C locale where isalpha('{') may be true      */
    /* (e.g. Swedish ASCII).                                              */
    if ('A' == 193)  /* ebcdic */
        val = (isdigit(ch) ? (ch) - '0' :
               isalpha(ch) ? (ch |= 0x40,         /* quick ebcdic toupper */
                   ch <= 'I' ? ch - 'A' + 10 :
                   ch <= 'R' ? ch - 'J' + 19 :
                               ch - 'S' + 28) :
               -1);
    else
        val = (isdigit(ch) ? (ch) - '0' :
               islower(ch) ? (ch) - 'a' + 10 :
               isupper(ch) ? (ch) - 'A' + 10 :
               -1);
    return (val < radix ? val : -1);
}

static long int rd_int(FILE *p, va_list res, int flag, int radix, int field)
{   long int charcount = -1;                     /* allow for always ungetc */
    unsigned long int n = 0;
    int ch;
    while (isspace(ch = countgetc(p)));     /* leading whitespace */
    if (ch == EOF) return CVTEOF;
    flag &= ~(NUMOK+NUMNEG);
    if (field > 0 && flag & ALLOWSIGN) switch (ch)
    {
case '-':   flag |= NUMNEG;
case '+':   ch = countgetc(p);
            field--;
            break;
    }
    if (field > 0 && ch=='0')
    {   flag |= NUMOK, field--;  /* starts with 0 - maybe octal or hex mode */
        ch = countgetc(p);
        if (field > 0 && (ch=='x' || ch=='X') && (radix==0 || radix==16))
        {   flag &= ~NUMOK, field--;
            ch = countgetc(p);
            radix = 16;
        }
        else if (radix == 0) radix = 8;
    }
    if (radix == 0) radix = 10;
    {   int digit;
        while (field > 0 && (digit = ch_val(ch, radix)) >= 0)
        {   flag |= NUMOK, field--;
            n = n*radix + digit;
            ch = countgetc(p);
        }
    }
    ungetc(ch, p);
    if (!(flag & NUMOK)) return CVTFAIL;
    if (!(flag & NOSTORE))
    { /* This code is pretty specious on a 2's complement machine         */
      if (flag & ALLOWSIGN)
      { long int m = flag & NUMNEG ? -n : n;
        int *p = va_arg(res, int *);  /* rely on sizeof(int*)=sizeof(short*) */
        if isSHORT_(flag) *(short *)p = (short)m;
        else if isLONG_(flag) *(long *)p = m;
        else *(int *)p = (int)m;
      }
      else  /* pointer case comes here too - with quite some type pun!  */
      { unsigned int *p = va_arg(res, unsigned int *);
            /* rely on sizeof(unsigned int *)==sizeof(unsigned short *) */
        if isSHORT_(flag) *(unsigned short *)p = (unsigned short)n;
        else if isLONG_(flag) *(unsigned long *)p = n;
        else *(unsigned int *)p = (unsigned int)n;
      }
    }
    return charcount;
}

#ifndef NO_FLOATING_POINT

#ifdef IBMFLOAT
static float carefully_narrow(double l)
{
/* Maybe in scanf I should ROUND rather than TRUNCATE here...            */
    static const int amax = 0x7fffffff, amin = 0xffffffff;
    if (l >= (double)*(float *)&amax ||
        l <= (double)*(float *)&amin) return (float)l;
    else                                  /* above line avoids overflow  */
    {   float f = (float) l;              /* narrower version            */
        double lost = l - (double)f;      /* amount that was chopped off */
        return (float)(l + +(lost + lost));
    }
}
#else /* IBMFLOAT */

static float carefully_narrow(double l)
/* All the trouble I went to to detect potential overflow has to be re-  */
/* done here so that underflow and overflow do not occur during the      */
/* narrowing operation that is about to be done.                         */
/* *********** machine dependent code ************* (should not go here) */
    {   int w;
        (void)frexp(l, &w);     /* extract exponent */
        w = w + 0x7e;           /* exponent for single precision version */
        if (w<=0 && l!=0.0)
        {   errno = ERANGE;
            return 0.0f;
        }
        else if (w >= 0xff)
/* Overflow of single-precision values - fudge single precision infinity */
/* *********** machine dependent code ************* (should not go here) */
        {   static int posinf = 0x7f800000, neginf = 0xff800000;
            errno = ERANGE;
            return *(float *)(l >= 0.0 ? &posinf : &neginf);
        }
        else return (float)l;  /* what we really wanted to do */
    }
#endif /* IEEE */

#ifdef HOST_HAS_BCD_FLT

static long int rd_real(FILE *p, va_list res, int flag, int field)
{
    long int charcount = -1;                     /* allow for always ungetc */
    int ch, x = 0;
    unsigned int a[3];            /* IEEE 'packed' format as per ACORN FPE2 */
    double l = 0.0;
    a[0] = a[1] = a[2] = 0;
    while (isspace(ch = countgetc(p)));  /* not counted towards field width */
    if (ch == EOF) return CVTEOF;
    flag &= ~(NUMOK+DOTSEEN+NUMNEG);
    if (field > 0) switch (ch)
    {
case '-':   flag |= NUMNEG;
case '+':   ch = countgetc(p);
            field--;
            break;
    }
    while (field > 0)
    {   if (ch=='.' && !(flag & DOTSEEN))
            flag |= DOTSEEN, field--;
        else if (isdigit(ch))
        {   flag |= NUMOK, field--;
            if ((a[0] & 0xf00) == 0)
            {   a[0] = (a[0]<<4) | (a[1]>>28);
                a[1] = (a[1]<<4) | (a[2]>>28);
                a[2] = (a[2]<<4) | scanf_intofdigit(ch);
                if (flag & DOTSEEN) x -= 1;
            }
            else if (!(flag & DOTSEEN)) x += 1;
        }
        else break;
        ch = countgetc(p);
    }
    /* we must unread the 'e' in (say) "+.e" as cannot be valid */
    if (field > 0 && (ch == 'e' || ch == 'E') && (flag & NUMOK))
    {   int x2 = 0;
        flag &= ~(NUMOK+NEGEXP), field--;
        switch (ch = countgetc(p))
        {
    case '-':   flag |= NEGEXP;
    case '+':   ch = countgetc(p);
                field--;
    default:    break;
        }
        while (field > 0 && isdigit(ch))
        {   flag |= NUMOK, field--;
            x2 = 10*x2 + scanf_intofdigit(ch);
            ch = countgetc(p);
        }
        if (flag & NEGEXP) x -= x2; else x += x2;
    }
    ungetc(ch, p);
    if (a[0]==0 && a[1]==0 && a[2]==0) l = 0.0;
    else
    {   if (a[0]==0 && a[1]==0)
        {   a[1] = a[2];
            a[2] = 0;
            x -= 8;
        }
        while ((a[0] & 0xf00)==0)
        {   a[0] = (a[0]<<4) | (a[1]>>28);
            a[1] = (a[1]<<4) | (a[2]>>28);
            a[2] = a[2]<<4;
            x -= 1;
        }
        x += 18;    /* allow for position of decimal point in packed format */
        if (x < -999) l = 0.0, errno = ERANGE;
        else if (x > 999)
        {   if (flag & NUMNEG) l = -HUGE_VAL;
            else l = HUGE_VAL;
            errno = ERANGE;
        }
        else
        {   if (x < 0) a[0] |= 0x40000000, x = -x;
            a[0] |= (x % 10) << 12;
            x /= 10;
            a[0] |= (x % 10) << 16;
            x /= 10;
            a[0] |= (x % 10) << 20;
            if (flag & NUMNEG) a[0] |= 0x80000000;
            l = _ldfp(a);       /* sets errno if necessary */
        }
    }
    if (!(flag & NUMOK)) return CVTFAIL;
    if (flag & LONG) 
    {   if (!(flag & NOSTORE))
        {   if (isLONGDOUBLE_(flag))
                *va_arg(res, long double *) = l;  /* not fully done */
            else
                *va_arg(res, double *) = l;
        }
    }
    else
    {   float f = carefully_narrow(l);
        /* treat overflow consistently whether or not stored */
        if (!(flag & NOSTORE)) *va_arg(res, float *) = f;
    }
    return charcount;
}

#else /* HOST_HAS_BCD_FLT */

static long int rd_real(FILE *p, va_list res, int flag, int field)
{
    long int charcount = -1;                     /* allow for always ungetc */
    int ch, x = 0, w;
    int i = 0;
    double l = 0.0, pten = 0.1;
    while (isspace(ch = countgetc(p)));  /* not counted towards field width */
    if (ch == EOF) return CVTEOF;
    flag &= ~(NUMOK+DOTSEEN+NUMNEG);
    if (field > 0) switch (ch)
    {
case '-':   flag |= NUMNEG;
case '+':   ch = countgetc(p);
            field--;
            break;
    }
/* I accumulate up to 6 (decimal) significant figures in the integer     */
/* variable i, and remaining digits in the floating point variable l.    */
    while (field > 0)
    {   if (ch=='.' && !(flag & DOTSEEN))
            flag |= DOTSEEN, field--;
        else if (isdigit(ch))
        {   flag |= NUMOK, field--;
            if (i < 100000)
            {   i = 10*i + scanf_intofdigit(ch);
                if (flag & DOTSEEN) x -= 1;
            }
            else
            {   l += pten * scanf_intofdigit(ch);
                pten /= 10.0;
                if (!(flag & DOTSEEN)) x += 1;
            }
        }
        else break;
        ch = countgetc(p);
    }
    /* we must unread the 'e' in (say) "+.e" as cannot be valid */
    if (field > 0 && (ch == 'e' || ch == 'E') && (flag & NUMOK))
    {   int x2 = 0;
        flag &= ~(NUMOK+NEGEXP), field--;
        switch (ch = countgetc(p))
        {
    case '-':   flag |= NEGEXP;
    case '+':   ch = countgetc(p);
                field--;
    default:    break;
        }
        while (field > 0 && isdigit(ch))
        {   flag |= NUMOK, field--;
            x2 = 10*x2 + scanf_intofdigit(ch);
            ch = countgetc(p);
        }
        if (flag & NEGEXP) x -= x2; else x += x2;
    }
    ungetc(ch, p);
/* The code that follows multiplies (i.l) by 10^x using one-and-a-half   */
/* precision arithmetic, with relevant scaling so that any over or under */
/* flow is deferred to the very last minute.                             */
    {   double d, dlow, d3, d3low;
        int bx = (10*x)/3, w1;
        l = ldexp(l, x-bx);
        d = ldexp((double)i, x-bx);
        dlow = 0.0;
        if (x < 0)
        {   w1 = -x;
            d3 = 0.2;
            d3low = 0.0;
            _fp_normalize(d3, d3low);
            d3low = (1.0 - 5.0*d3)/5.0;
        }
        else
        {   w1 = x;
            d3 = 5.0;
            d3low = 0.0;
        }
        if (w1!=0) for(;;)
        {   if ((w1&1)!=0)
            {   l *= (d3 + d3low);
                dlow = d*d3low + dlow*(d3 + d3low);
                d *= d3;
                _fp_normalize(d, dlow);
                if (w1==1) break;
            }
            d3low *= (2.0*d3 + d3low);
            d3 *= d3;
            _fp_normalize(d3, d3low);
            w1 = w1 >> 1;
        }
#ifdef IBMFLOAT
        l = l + dlow;
/* I want to set l to (l+d)*2^bx here.                                   */
/* At present d has lots of low zero bits, and l is much smaller.        */
        switch (bx & 3)
        {
    case 3:     l *= 8.0;   d *= 8.0;   break;
    case 2:     l *= 4.0;   d *= 4.0;   break;
    case 1:     l *= 2.0;   d *= 2.0;   break;
    case 0:                             break;
        }
        {   double ltemp = l + d;
            double err = +(d - ltemp) + l;
            ltemp = ltemp + +(err + err);   /* Achieves rounding         */
            l = frexp(ltemp, &w);
            w += (bx & ~3);
        }
#else
        l = l + dlow;
        l = l + d;
        l = frexp(l, &w);
        w += bx;
#endif
/* Now I check to see if the number would give a floating point overflow */
/* and if so I return HUGE_VAL, and set errno to ERANGE.                 */
/* ********* machine dependent integers ***********                      */
#ifdef IBMFLOAT
        if (w > (0x3f)*4)
#else
        if (w >= 0x7ff-0x3fe)
#endif
        {   if (flag & NUMNEG) l = -HUGE_VAL;
            else l = HUGE_VAL;
            errno = ERANGE;
        }
/* Underflows yield a zero result but set errno to ERANGE                */
#ifdef IBMFLOAT
        else if (w <= -(0x41*4) && l!=0.0)
#else
        else if (w <= -0x3fe && l!=0.0)
#endif
        {   l = 0.0;
            errno = ERANGE;
        }
        else
        {   if (flag & NUMNEG) l = -ldexp(l, w);
            else l = ldexp(l, w);
        }
    }
    if (!(flag & NUMOK)) return CVTFAIL;
    if (flag & LONG) 
    {   if (!(flag & NOSTORE))
        {   if (isLONGDOUBLE_(flag))
                *va_arg(res, long double *) = l;  /* not fully done */
            else
                *va_arg(res, double *) = l;
        }
    }
    else
    {   float f = carefully_narrow(l);
        /* treat overflow consistently whether or not stored */
        if (!(flag & NOSTORE)) *va_arg(res, float *) = f;
    }
    return charcount;
}



#endif /* HOST_HAS_BCD_FLT */

#endif /* NO_FLOATING_POINT */

/* Amalgamate the next two routines? */
static long int rd_string(FILE *p, va_list res, int flag, int field)
{   long int charcount = -1;                     /* allow for always ungetc */
    int ch; char *s;
    while (isspace(ch = countgetc(p)));  /* not counted towards field width */
    if (ch == EOF) return CVTEOF;                /* fail if EOF occurs here */
    if (!(flag & NOSTORE)) s = va_arg(res, char *);
    while (field > 0 && ch!=EOF && !isspace(ch))
    {   field--;
        if (!(flag & NOSTORE)) *s++ = ch;
        ch = countgetc(p);
    }
    ungetc(ch, p);                               /* OK if ch == EOF         */
    if (!(flag & NOSTORE)) *s = 0;
    return charcount;
}

/* Ambiguity in Oct 86 ANSI draft: can "%[x]" match a zero-length string?  */
/* p119 line 19 suggests no, p121 example suggests yes.  Treat as yes here */
static long int rd_string_map(FILE *p, va_list res, int flag, int field,
                              int charmap[])
{   long int charcount = -1;                     /* allow for always ungetc */
    int ch; char *s;
    if (!(flag & NOSTORE)) s = va_arg(res, char *);
    ch = countgetc(p);
    if (ch == EOF) return CVTEOF;
    while (field > 0 && ch != EOF && (charmap[ch/32] & (1<<(ch%32))) != 0)
    {   field--;
        if (!(flag & NOSTORE)) *s++ = ch;
        ch = countgetc(p);
    }
    ungetc(ch, p);                               /* OK if ch == EOF         */
    if (!(flag & NOSTORE)) *s = 0;
    return charcount;
}

/* It seems amazing that vfscanf is not available externally in ANSI */
static int vfscanf(FILE *p, const char *sfmt, va_list argv)
{
/* The next line is essential (see isspace() ANSI doc. and also use of
 * charmap[] below) if char is signed by default.
 * Our char is unsigned, but the following line should
 * just use the same register for fmt/sfmt and so cost nothing!
 */
    const unsigned char *fmt = (const unsigned char *)sfmt;
    int cnt = 0;
    long charcount = 0;
    for (;;)
    {   int fch;
        switch (fch = *fmt++)
        {
case 0:     return cnt;                        /* end of format string   */

default:    {   int ch;
                if (isspace(fch))              /* consolidate whitespace */
                {   int seen = 0;
                    while (isspace(fch = *fmt++));
                    fmt--;
/* N.B. isspace() must return 0 if its arg is '\0' or EOF.               */
                    while (isspace(ch = getc(p))) charcount++, seen = 1;
                    ungetc(ch, p);
#ifdef never
/* The next line requires non empty whitespace to match format whilespace. */
/* Removed as incompatible with bsd unix (and other prior practice?).      */
                    if (!seen) return cnt;     /* require at least 1     */
#endif
                    continue;
                }
                else if ((ch = getc(p)) == fch)
                {   charcount++;
                    continue;
                }
                ungetc(ch, p);  /* offending char is left unread         */
                if (ch == EOF && cnt == 0) return EOF;
                return cnt;     /* unmatched literal                     */
            }

case '%':   {   int field = 0, flag = 0;
                long int worked;
                if (*fmt == '*') fmt++, flag |= NOSTORE;
                while (isdigit(fch = *fmt++))
                {   if (field > INT_MAX/10) return cnt; /* overflow check */
                    field = field*10 + fch - '0';
                    if (field < 0) return cnt;         /* overflow check */
                    flag |= FIELDGIVEN;
                }
                if (!(flag & FIELDGIVEN)) field = INT_MAX;
                if (fch == 'l') fch = *fmt++, flag |= LONG;
                else if (fch == 'L') fch = *fmt++, flag |= LONG | LONGDOUBLE;
                else if (fch == 'h') fch = *fmt++, flag |= SHORT;
                switch (fch)
                {
        default:    return cnt;         /* illegal conversion code       */
        case '%':   {   int ch = getc(p);
/* treat as fatuous the omission of '%' from non-skipping white space list */
                        if (ch == '%')
                        {   charcount++;
                            continue;
                        }
                        ungetc(ch, p);  /* offending char is left unread */
                        if (ch == EOF && cnt == 0) return EOF;
                        return cnt;     /* unmatched literal '%'         */
                    }
        case 'c':   if (!(flag & FIELDGIVEN)) field = 1;
                    {   char *cp; int ch;
                        if (!(flag & NOSTORE)) cp = va_arg(argv, char *);
/* ANSI say no chars match -> failure.  Hence 0 width must always fail.     */
                        if (field == 0) return cnt;
                        for (; field > 0; field--)
/* The next line reflects the ANSI wording suggesting EXACTLY 'field' chars */
/* should be read.                                                          */
                        {   if ((ch = countgetc(p)) == EOF)
                                return cnt == 0 ? EOF : cnt;
                            if (!(flag & NOSTORE)) *cp++ = ch;
                        }
                    }
                    if (!(flag & NOSTORE)) cnt++; /* If conversion succeeds */
                    continue;
        case 'd':   worked = rd_int(p, argv, flag | ALLOWSIGN, 10, field);
                    break;
        case 'e':
        case 'E':
        case 'f':
        case 'g':
        case 'G':
#ifndef NO_FLOATING_POINT
                    worked = rd_real(p, argv, flag, field);
#else
                    return(cnt);    /* Floating point not implemented    */
#endif
                    break;
        case 'i':   worked = rd_int(p, argv, flag | ALLOWSIGN, 0, field);
                    break;
/* %n assigns the number of characters read from the input so far - NOTE */
/* that this assignment is NOT influenced by the * flag and does NOT     */
/* count towards the value returned by scanf.  Note that h and l apply.  */
        case 'n':   if isSHORT_(flag) *va_arg(argv, short *) = (short)charcount;
                    else if isLONG_(flag) *va_arg(argv, long *) = charcount;
                    else *va_arg(argv, int *) = (int)charcount;
                    continue;
        case 'o':   worked = rd_int(p, argv, flag | ALLOWSIGN, 8, field);
                    break;
                    /* pointers are displayed in hex, but h,l,L ignored */
        case 'p':   worked = rd_int(p, argv, flag & ~(LONG|SHORT), 16, field);
                    break;
        case 's':   worked = rd_string(p, argv, flag, field);
                    break;
        case 'u':   worked = rd_int(p, argv, flag, 10, field);
                    break;
        case 'x':
        case 'X':   worked = rd_int(p, argv, flag | ALLOWSIGN, 16, field);
                    break;
        case '[':   {   int negated = 0, i, charmap[8];
                        if ((fch = *fmt++) == '^') negated = 1, fch = *fmt++;
                        for (i=0; i<8; i++) charmap[i] = 0;
                        /* the 'do' next allows special treatment of %[]})] */
                        do { if (fch==0) return cnt;  /* %[... unterminated */
                             charmap[fch/32] |= 1<<(fch%32);
                        } while ((fch = *fmt++) != ']');
                        if (negated) for (i=0; i<8; i++)
                            charmap[i] = ~charmap[i];
                        worked = rd_string_map(p, argv, flag, field, charmap);
                    }
                    break;
                }
                if (worked < 0)                  /* conversion failed       */
                     return worked == CVTEOF && cnt == 0 ? EOF : cnt;
                if (!(flag & NOSTORE)) cnt++;    /* another assignment made */
                charcount += worked;             /* chars were read anyway  */
                continue;
            }
        }
    }
}

int fscanf(FILE *fp, const char *fmt, ...)
{
    va_list a;
    int n;
    va_start(a, fmt);
    n = vfscanf(fp, fmt, a);
    va_end(a);
    return n;
}

int scanf(const char *fmt, ...)
{
    va_list a;
    int n;
    va_start(a, fmt);
    n = vfscanf(stdin, fmt, a);
    va_end(a);
    return n;
}

int sscanf(const char *buff, const char *fmt, ...)
{
/*************************************************************************/
/* Note that this code interacts in a dubious way with the getc macro.   */
/*************************************************************************/
    va_list a;
    FILE hack;
    int n;
    va_start(a, fmt);
    memclr(&hack, sizeof(FILE));
    hack._flag = _IOSTRG+_IOREAD;
    hack._ptr = hack._base = (unsigned char *)buff;
    hack._icnt = strlen(buff);
    n = vfscanf(&hack, fmt, a);
    va_end(a);
    return n;
}

#ifndef NO_FLOATING_POINT
double strtod(const char *nptr, char **endptr)
{
    double d;
    int nchars, res;
/* Here I rely on scanf to set errno to ERANGE if the converted value is */
/* too big or too small.                                                 */
    res = sscanf(nptr, "%lf%n", &d, &nchars);
/* If the conversion failed that must be because there were no digits at */
/* all in the input.                                                     */
    if (res==EOF || res<1)
    {   if (endptr!=NULL) *endptr = (char *)nptr;
        /* the cast is needed in prev. line.  See May 86 ANSI draft 3.2.2.4 */
        return 0.0;
    }
    if (endptr!=NULL) *endptr = (char *)nptr + nchars;
    return d;
}
#endif

long int strtol(const char *nsptr, char **endptr, int base)
{
/* The specification in the ANSI information bulletin upsets me here:    */
/* strtol is of type long int, and 'if the correct value would cause     */
/* overflow LONG_MAX or LONG_MIN is returned'. Thus for hex input the    */
/* string 0x80000000 will be considered to have overflowed, and so will  */
/* be returned as LONG_MAX.                                              */
/* These days one should use strtoul for unsigned values, so some of     */
/* my worries go away.                                                   */

/* This code is NOT shared with the %i conversion in scanf for several   */
/* reasons: (a) here I deal with overflow in a silly way as noted above, */
/* (b) in scanf I have to deal with field width limitations, which does  */
/* not fit in neatly here (c) this functions scans an array of char,     */
/* while scanf reads from a stream - fudging these together seems too    */
/* much work, (d) here I have the option of specifying the radix, while  */
/* in scanf there seems to be no provision for that. Ah well!            */

    const unsigned char *nptr = (const unsigned char *)nsptr;  /* see scanf */
    int flag = 0, c;
    while ((c = *nptr++)!=0 && isspace(c));
    switch (c)
    {
case '-': flag |= NUMNEG;
          /* drop through */
case '+': break;
default:  nptr--;
          break;
    }
    {   char *endp;
        unsigned long ud = strtoul((char *)nptr, &endp, base);
        if (endptr) *endptr = endp==(char *)nptr ? (char *)nsptr : endp;
/* The following lines depend on the fact that unsigned->int casts and   */
/* unary '-' cannot cause arithmetic traps.  Recode to avoid this?       */
        if (flag & NUMNEG)
            return (-(long)ud <= 0) ? -(long)ud : (errno = ERANGE, LONG_MAX);
        else
            return (+(long)ud >= 0) ? +(long)ud : (errno = ERANGE, LONG_MIN);
    }
}

unsigned long int strtoul(const char *nsptr, char **endptr, int base)
{
    const unsigned char *nptr = (const unsigned char *)nsptr;  /* see scanf */
    int c, ok = 0, overflowed = 0;
    while ((c = *nptr++)!=0 && isspace(c));
    if (c=='0')
    {   ok = 1;
        c = *nptr++;
        if (c=='x' || c=='X')
        {   if (base==0 || base==16)
            {   ok = 0;
                base = 16;
                c = *nptr++;
            }
        }
        else if (base==0) base = 8;
    }
    if (base==0) base = 10;
    {   unsigned long dhigh = 0, dlow = 0;
        int digit;
        while ((digit = ch_val(c,base)) >= 0)
        {   ok = 1;
            dlow = base * dlow + digit;
            dhigh = base * dhigh + (dlow >> 16);
            dlow &= 0xffff;
            if (dhigh >= 0x10000) overflowed = 1;
            c = *nptr++;
        }
        if (endptr) *endptr = ok ? (char *)nptr-1 : (char *)nsptr;
                                                /* extra result */
        return overflowed ? (errno = ERANGE, ULONG_MAX)
                          : (dhigh << 16) | dlow;
    }
}

double atof(const char *nptr)
{
    int save_errno = errno;
    double res = strtod(nptr, (char **)NULL);
    errno = save_errno;
    return res;
}

int atoi(const char *nptr)
{
    int save_errno = errno;
    long int res = strtol(nptr, (char **)NULL, 10);
    errno = save_errno;
    return (int)res;
}

long int atol(const char *nptr)
{
    int save_errno = errno;
    long int res = strtol(nptr, (char **)NULL, 10);
    errno = save_errno;
    return res;
}

/* end of scanf.c */
