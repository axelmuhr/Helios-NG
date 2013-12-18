
/* printf.c: ANSI draft (X3J11 Oct 86) part of section 4.9 code */
/* Copyright (C) A.C. Norman and A. Mycroft */
/* version 0.05 */
/* $Id: printf.c,v 1.5 1992/08/12 13:55:15 nickc Exp $ */

/* printf and its friends return the number of characters planted. In    */
/* the case of sprintf this does not include the terminating '\0'.       */
/* Consider using ftell instead of charcount in printf (see scanf).      */
/* see c.ansilib re possible use of #define NO_FLOATING_POINT.           */


#include "norcrosys.h"
#include "sysdep.h"
#include <stdio.h>
#include <stdarg.h>
#include <ctype.h>
#include <string.h>
#include <math.h>

#ifndef fp_print_defined
#define fp_print_defined
/* The following typedef and type of __vfprintf must match  fpprintf.c  */
typedef int (*fp_print)(int ch, double *d, char buff[], int flags,
                        char **lvprefix, int *lvprecision, int *lvbefore_dot,
                        int *lvafter_dot);

/* The following macros must match those in fpprintf.c:                  */
/* The code here tends to be a bit generous about meaningless flag       */
/* combinations of options in the format string - it chooses the set     */
/* that it likes and prints according to that style without commenting   */
/* on the redundant options.                                             */

#define _LJUSTIFY         01
#define _SIGNED           02
#define _BLANKER          04
#define _VARIANT         010
#define _PRECGIVEN       020
#define _LONGSPECIFIER   040
#define _SHORTSPEC      0100
#define _PADZERO        0200    /* *** DEPRECATED FEATURE *** */
#define _FPCONV         0400
#endif

#define intofdigit(x) ((x)-'0')

#define pr_padding(ch, n, p)  while(--n>=0) charcount++, putc(ch, p);

#define pre_padding(p)                                                    \
        if (!(flags&_LJUSTIFY))                                           \
        {   char padchar = flags & _PADZERO ? '0' : ' ';                  \
            pr_padding(padchar, width, p); }

#define post_padding(p)                                                   \
        if (flags&_LJUSTIFY)                                              \
        {   char padchar = flags & _PADZERO ? '0' : ' ';                  \
            pr_padding(padchar, width, p); }

#ifdef never /* do it this way? */
static int pr_num(unsigned int v, int flags, char *prefix,
                   int width, int precision, FILE *p)
{}
#endif

static int printf_display(FILE *p, int flags, int ch, int precision, int width,
                   unsigned int v, fp_print fp_display_fn, char *prefix,
                   char *hextab, double *d)
{
    int charcount = 0;
    int len = 0, before_dot = -1, after_dot = -1;
    char buff[32];       /* used to accumulate value to print    */
/* here at the end of the switch statement I gather together code that   */
/* is concerned with displaying integers.                                */
/* AM: maybe this would be better as a proc if we could get arg count down */
            if ((flags & _FPCONV+_PRECGIVEN)==0) precision = 1;
            switch (ch)
            {
    case 'p':
    case 'X':
    case 'x':   while(v!=0)
                {   buff[len++] = hextab[v & 0xf];
                    v = v >> 4;
                }
                break;
    case 'o':   while(v!=0)
                {   buff[len++] = '0' + (v & 07);
                    v = v >> 3;
                }
                break;
    case 'u':
    case 'i':
    case 'd':   while (v != 0)
                  {
		    unsigned int 	vDiv10 = _kernel_udiv10( v );

		    
		    buff[ len++ ] = '0' + v - vDiv10 * 10U;
		    
                    v = vDiv10;
		  }
                break;

    case 'f':
    case 'g':   case 'G':
    case 'e':   case 'E':
                len = fp_display_fn(ch, d, buff, flags,
/* The following arguments are set by fp_display_fn                      */
                                    &prefix, &precision,
                                    &before_dot, &after_dot);
                break;
#ifdef never
/* If floating point is not supported I display ALL %e, %f and %g        */
/* items as 0.0                                                          */
    default:    buff[0] = '0';
                buff[1] = '.';
                buff[2] = '0';
                len = 3;
                break;
#endif
            }
/* now work out how many leading '0's are needed for precision specifier */
/* _FPCONV is the case of FP printing in which case extra digits to make */
/* up the precision come within the number as marked by characters '<'   */
/* and '>' in the buffer.                                                */
            if (flags & _FPCONV)
            {   precision = 0;
                if (before_dot>0) precision = before_dot-1;
                if (after_dot>0) precision += after_dot-1;
            }
            else if ((precision -= len)<0) precision = 0;

/* and how much padding is needed */
            width -= (precision + len + strlen(prefix));

/* AM: ANSI appear (Oct 86) to suggest that the padding (even if with '0') */
/*     occurs before the possible sign!  Treat this as fatuous for now.    */
            if (!(flags & _PADZERO)) pre_padding(p);

            {   int c;                                      /* prefix    */
                while((c=*prefix++)!=0) { putc(c, p); charcount++; }
            }

            pre_padding(p);

/* floating point numbers are in buff[] the normal way around, while     */
/* integers have been pushed in with the digits in reverse order.        */
            if (flags & _FPCONV)
            {   int i, c;
                for (i = 0; i<len; i++)
                {   switch (c = buff[i])
                    {
            case '<':   pr_padding('0', before_dot, p);
                        break;
            case '>':   pr_padding('0', after_dot, p);
                        break;
            default:    putc(c, p);
                        charcount++;
                        break;
                    }
                }
            }
            else
            {   pr_padding('0', precision, p);
                charcount += len;
                while((len--)>0) putc(buff[len], p);
            }

/* By here if the padding has already been printed width will be zero    */
            post_padding(p);
            return charcount;
}

int __vfprintf(FILE *p, const char *fmt, va_list args, fp_print fp_display_fn)
/* ACN: I apologize for this function - it seems long and ugly. Some of  */
/*      that is dealing with all the jolly flag options available with   */
/*      printf, and rather a lot more is a cautious floating point print */
/*      package that takes great care to avoid the corruption of its     */
/*      input by rounding, and to generate consistent decimal versions   */
/*      of all possible values in all possible formats.                  */
{
    int ch, charcount = 0;

    while ((ch = *fmt++) != 0)
    {   if (ch != '%') { putc(ch,p); charcount++; }
        else
        {   int flags = 0, width = 0, precision = 0;
/* The initialisation of hextab is spurious in that it will be set       */
/* to a real string before use, but necessary in that passing unset      */
/* parameters to functions is illegal in C.                              */
            char *prefix, *hextab = 0;
            unsigned int v;
            double d;

/* This decodes all the nasty flags and options associated with an       */
/* entry in the format list. For some entries many of these options      */
/* will be useless, but I parse them all the same.                       */
            for (;;)
            {   switch (ch = *fmt++)
                {
/* '-'  Left justify converted flag. Only relevant if width specified    */
/* explicitly and converted value is too short to fill it.               */
        case '-':   flags |= _LJUSTIFY;
                    continue;

/* '+'  Always print either '+' or '-' at start of numbers.              */
        case '+':   flags |= _SIGNED;
                    continue;

/* ' '  Print either ' ' or '-' at start of numbers.                     */
        case ' ':   flags |= _BLANKER;
                    continue;

/* '#'  Variant on main print routine (effect varies across different    */
/*      styles, but for instance %#x puts 0x on the front of displayed   */
/*      numbers.                                                         */
        case '#':   flags |= _VARIANT;
                    continue;

        default:    break;
                }
                break;
            }
            /* now look for 'width' spec */
            {   int t = 0;
                if (ch=='*')
                {   t = va_arg(args, int);
/* If a negative width is passed as an argument I take its absolute      */
/* value and use the negativeness to indicate the presence of the '-'    */
/* flag (left justification). If '-' was already specified I lose it.    */
                    if (t<0)
                    {   t = - t;
                        flags ^= _LJUSTIFY;
                    }
                    ch = *fmt++;
                }
                else
                {   /* the next line is a *** DEPRECATED FEATURE *** */
                    if (ch == '0') flags |= _PADZERO;
                    while (isdigit(ch))
                    {   t = t*10 + intofdigit(ch);
                        ch = *fmt++;
                    }
                }
                width = t>=0 ? t : 0;                 /* disallow -ve arg */
            }
            if (ch == '.')                            /* precision spec */
            {   int t = 0;
                ch = *fmt++;
                if (ch=='*')
                {   t = va_arg(args, int);
                    ch = *fmt++;
                }
                else while (isdigit(ch))
                {   t = t*10 + intofdigit(ch);
                    ch = *fmt++;
                }
                if (t >= 0) flags |= _PRECGIVEN, precision = t;
            }
            if (ch=='l' || ch=='L')
/* 'l'  Indicate that a numeric argument is 'long'. Here int and long    */
/*      are the same (32 bits) and so I can ignore this flag!            */
/* 'L'  Marks floating arguments as being of type long double. Here this */
/*      is the same as just double, and so I can ignore the flag.        */
            {   flags |= _LONGSPECIFIER;
                ch = *fmt++;
            }
            else if (ch=='h')
/* 'h' Indicates that an integer value is to be treated as short.        */
            {   flags |= _SHORTSPEC;
                ch = *fmt++;
            }

/* Now the options have been decoded - I can process the main dispatch   */
            switch (ch)
            {

/* %c causes a single character to be fetched from the argument list     */
/* and printed. This is subject to padding.                              */
    case 'c':   ch = va_arg(args, int);
                /* drop through */

/* %? where ? is some character not properly defined as a command char   */
/* for printf causes ? to be displayed with padding and field widths     */
/* as specified by the various modifers. %% is handled by this general   */
/* mechanism.                                                            */
    default:    width--;                        /* char width is 1       */
                pre_padding(p);
                putc(ch, p);
                charcount++;
                post_padding(p);
                continue;

/* If a '%' occurs at the end of a format string (possibly with a few    */
/* width specifiers and qualifiers after it) I end up here with a '\0'   */
/* in my hand. Unless I do something special the fact that the format    */
/* string terminated gets lost...                                        */
    case 0:     fmt--;
                continue;

/* %n assigns the number of chars printed so far to the next arg (which  */
/* is expected to be of type (int *).                                    */
    case 'n':   {   int *xp = va_arg(args, int *);
                    *xp = charcount;
                }
                continue;

/* %s prints a string. If a precision is given it can limit the number   */
/* of characters taken from the string, and padding and justification    */
/* behave as usual.                                                      */
    case 's':   {   char *str = va_arg(args, char *);
                    int i, n = strlen(str);
		    
                    if ((flags&_PRECGIVEN) && (precision<n))
                        n = precision;
                    width -= n;
                    pre_padding(p);
                    for (i=0; i<n; i++)
		      {
			putc(str[i], p);
		      }
                    charcount += n;
                    post_padding(p);
                }
                continue;

/* %x prints in hexadecimal. %X does the same, but uses upper case       */
/* when printing things that are not (decimal) digits.                   */
/* I can share some messy decoding here with the code that deals with    */
/* octal and decimal output via %o and %d.                               */
    case 'X':   v = va_arg(args, int);
                if (flags & _SHORTSPEC) v = (unsigned short)v;
                hextab = "0123456789ABCDEF";
                prefix = (flags&_VARIANT) ? "0X" : "";
                if (flags & _PRECGIVEN) flags &= ~_PADZERO;
                break;

    case 'x':   v = va_arg(args, int);
                if (flags & _SHORTSPEC) v = (unsigned short)v;
                hextab = "0123456789abcdef";
                prefix = (flags&_VARIANT) ? "0x" : "";
                if (flags & _PRECGIVEN) flags &= ~_PADZERO;
                break;

/* %p is for printing a pointer - I print it as a hex number with the    */
/* precision always forced to 8.                                         */
    case 'p':   v = (unsigned int)va_arg(args, void *);
                hextab = "0123456789abcdef";
                prefix = (flags&_VARIANT) ? "@" : "";
                flags |= _PRECGIVEN;
                precision = 8;
                break;

    case 'o':   v = va_arg(args, int);
                if (flags & _SHORTSPEC) v = (unsigned short)v;
                prefix = (flags&_VARIANT) ? "0" : "";
                if (flags & _PRECGIVEN) flags &= ~_PADZERO;
                break;

    case 'u':   v = va_arg(args, unsigned int);
                if (flags & _SHORTSPEC) v = (unsigned short)v;
                prefix = "";
                if (flags & _PRECGIVEN) flags &= ~_PADZERO;
                break;

    case 'i':
    case 'd':   {   int w = va_arg(args, int);
                    if (flags & _SHORTSPEC) w = (signed short)w;
                    if (w<0) v = 0U-w, prefix = "-";
                    else
                        v = w, prefix = (flags&_SIGNED) ? "+" :
                                        (flags&_BLANKER) ? " " : "";
                }
                if (flags & _PRECGIVEN) flags &= ~_PADZERO;
                break;
                                
    case 'f':
    case 'e':
    case 'E':
    case 'g':
    case 'G':   flags |= _FPCONV;
                if (!(flags & _PRECGIVEN)) precision = 6;

                d = va_arg(args, double);

                /* technically, for the call to printf_display() below to  */
                /* be legal and not reference an undefined variable we     */
                /* need to do the following (overwritten in fp_display_fn) */
                /* (It also stops dataflow analysis (-fa) complaining!)    */

                prefix = 0, hextab = 0, v = 0;
                break;

            }
            charcount += printf_display(p, flags, ch, precision, width, v,
                                        fp_display_fn, prefix, hextab, &d);
            continue;
        }
    }

    return charcount;
}

static int no_fp_display(int ch, double *d, char buff[], int flags,
                         char **lvprefix, int *lvprecision, int *lvbefore_dot,
                         int *lvafter_dot)
{
    ch = ch;
    d = d;
    buff = buff;
    flags = flags;
    lvprefix = lvprefix;
    lvprecision = lvprecision;
    lvbefore_dot = lvbefore_dot;
    lvafter_dot = lvafter_dot;
    return 0;
}

#ifdef NO_FLOATING_POINT
int fprintf(FILE *fp, const char *fmt, ...)
{
    va_list a;
    int n;
    va_start(a, fmt);
    n = __vfprintf(fp, fmt, a, no_fp_display);
    va_end(a);
    return n;
}

int printf(const char *fmt, ...)
{
    va_list a;
    int n;
    va_start(a, fmt);
    n = __vfprintf(stdout, fmt, a, no_fp_display);
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
    hack._flag = _IOSTRG+_IOWRITE;
    hack._ptr = (unsigned char *)buff;
    hack._ocnt = 0x7fffffff;
    length = __vfprintf(&hack, fmt, a, no_fp_display);
    putc(0,&hack);
    va_end(a);
    return(length);
}

int vfprintf(FILE *p, const char *fmt, va_list args)
{
    return __vfprintf(p, fmt, args, no_fp_display);
}

/* NHG - the following 2 were left out of clib!!! */
int vprintf(const char *fmt, va_list a)
{
    return __vfprintf(stdout, fmt, a, no_fp_display);
}

int vsprintf(char *buff, const char *fmt, va_list a)
{
    FILE hack;
/*************************************************************************/
/* Note that this code interacts in a dubious way with the putc macro.   */
/*************************************************************************/
    int length;
    memclr(&hack, sizeof(FILE));
    hack._flag = _IOSTRG+_IOWRITE;
    hack._ptr = (unsigned char *)buff;
    hack._ocnt = 0x7fffffff;
    length = __vfprintf(&hack, fmt, a, no_fp_display);
    putc(0,&hack);
    return(length);
}
#endif /* NO_FLOATING_POINT */

/* End of printf.c */

