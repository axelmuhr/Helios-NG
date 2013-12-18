/* string.c: Moved from the C library to the Utility library */
/* for use with processors that have not downcoded these fns to asm. */
/* RCSId: $Id: string.c,v 1.2 1992/04/10 16:45:10 paul Exp $ */

/* string.c: ANSI draft (X3J11 Oct 86) library code, section 4.11 */
/* Copyright (C) A.C. Norman and A. Mycroft */
/* version 0.03 : %W% %G% */

/* Also the routine 'strerror' is in 'perror.c' rather than here */

/* 19-nov-86:  If _copywords is #define'd then all cmp/cpy/set (but not cat)
   routines use word operations if operands lie on a word boundary.
   Enabling this assumes sizeof(int)=sizeof(void*)=4 and byte addressing.
   memset and strlen do even better.  Beware however, memcpy and memmove
   also require length to be a multiple of 4 (change?).  Further beware that
   memmove makes assumptions about memory layout.
*/

#include <stddef.h>                /* for size_t                           */

#define _chararg int               /* arg spec for char when ANSI says int */
#define _copywords 1               /* do fast cpy/cmp if word aligned      */

#ifdef __BIGENDIAN
#  define BYTESEX_ODD
#else
#  define BYTESEX_EVEN
#endif
/* BYTESEX_EVEN or BYTESEX_ODD should be defined for ARM/370 respectively  */

/* The following magic check was designed by A. Mycroft. It yields a     */
/* nonzero value if the argument w has a zero byte in it somewhere. The  */
/* messy constants have been unfolded a bit in this code so that they    */
/* get preloaded into registers before relevant loops.                   */

#  define ONES_WORD   0x01010101
#  define EIGHTS_WORD 0x80808080
#  define nullbyte_prologue_() \
      int ones_word = ONES_WORD; int eights_word = ones_word << 7
#  define word_has_nullbyte(w) (((w) - ones_word) & ~(w) & eights_word)

#ifndef __C40	/* 'C40 has assembler function in kernel for this */ 
void *memcpy(void *a, const void *b, size_t n)
/* copy memory (upwards) - it is an errof for args to overlap. */
/* Relies on sizeof(int)=sizeof(void *) and byte addressing.   */
{
#ifdef _copywords
    /* do it fast if word aligned ... */
    if ((((int)a | (int)b | (int)n) & 3) == 0)
    { int *wa,*wb;
      n >>= 2;
      for (wa = (int *)a, wb = (int *)b; n-- > 0;) *wa++ = *wb++;
    }
    else
#endif
    { char *ca,*cb;
      for (ca = (char *)a, cb = (char *)b; n-- > 0;) *ca++ = *cb++;
    }
    return a;
}
#endif


/*#ifndef __C40*/	/* 'C40 assembler function */ 
char *strcpy(char *a, const char *b)                 /* copy from b to a */
{   char *p = a;
#ifdef _copywords
    if ((((int)p | (int)b) & 3) == 0)
    {   int w;
        nullbyte_prologue_();
        while (w = *(int *)b, !word_has_nullbyte(w))
            *(int *)p = w, p += 4, b += 4;
/* This next piece of code relies on knowledge of the order of bytes     */
/* within a word.                                                        */
#ifdef BYTESEX_EVEN
        for (;;)
        {   if ((*p++ = w) == 0) return a;
            w >>= 8;
        }
#else
        for (;;)
/* I rather assume that shifts are fast operations here.                 */
        {   if ((*p++ = (w >> 24)) == 0) return a;
            w <<= 8;
        }
#endif
    }
#endif
    while ((*p++ = *b++) != 0);
    return a;
}
/*#endif*/


char *strncpy(char *a, const char *b, size_t n)
            /* as strcpy, but at most n chars */
            /* NB may not be nul-terminated   */
{   char *p = a;
#ifdef _copywords
    if ((((int)p | (int)b) & 3) == 0)
    {   int w;
        nullbyte_prologue_();
        while (n >= 4 && (w = *(int *)b, !word_has_nullbyte(w)))
            *(int *)p = w, p += 4, b += 4, n -= 4;
    }
/* Although the above code has fetched the last part-filled word I will  */
/* copy the last few bytes by steam in this case. The test on n and the  */
/* need for padding seem to make anything else seem too messy.           */
#endif
    while (n-- > 0)
        if ((*p++ = *b++) == 0)
        {   while (n-- > 0) *p++ = 0;   /* ANSI says pad out with nul's */
            return a;
        }
    return a;
}


/* concatenation functions */

char *strcat(char *a, const char *b)    /* concatenate b on the end of a */
{   char *p = a;
    while (*p != 0) p++;
    while ((*p++ = *b++) != 0);
    return a;
}


char *strncat(char *a, const char *b, size_t n)
                                       /* as strcat, but at most n chars */
{   char *p = a;
    while (*p != 0) p++;
    while (n-- > 0)
        if ((*p++ = *b++) == 0) return a;
    *p = 0;
    return a;
}


#ifndef __C40 /* 'C40 has assembler function */
int strcmp(const char *a, const char *b) /* lexical comparison on strings */
{
#ifdef _copywords
    if ((((int)a | (int)b) & 3) == 0)
    {   int w;
        nullbyte_prologue_();
        while ((w = *(int *)a) == *(int *)b && !word_has_nullbyte(w))
            a += 4, b += 4;
    }
#endif
    for (;;)
    {   char c1,c2;
        if ((c1 = *a++) != (c2 = *b++)) return c1 - c2;
        if (c1 == 0) return 0;     /* no need to check c2 */
    }
}
#endif

int strncmp(const char *a, const char * b, size_t n)
                                        /* as strcmp, but at most n chars */
{
#ifdef _copywords
    if ((((int)a | (int)b) & 3) == 0)
    {   int w;
        nullbyte_prologue_();
        while (n >= 4 && (w = *(int *)a) == *(int *)b && !word_has_nullbyte(w))
            a += 4, b += 4, n -= 4;
    }
#endif
    while (n-- > 0)
    {   char c1,c2;
        if ((c1 = *a++) != (c2 = *b++)) return c1 - c2;
        if (c1 == 0) return 0;     /* no need to check c2 */
    }
    return 0;
}


#ifndef __C40	/* 'C40 has assembler function in kernel for this */ 
void *memset(void *s, _chararg c, size_t n)
{
    unsigned char *p = s;
    while (n > 0)
    {
#ifdef _copywords
        if (n >= 4 && ((int)p & 3) == 0)
        {   int w = 0x01010101 * (unsigned char)c;     /* duplicate 4 times */
            do *(int *)p = w, p += 4, n -= 4; while (n >= 4);
        }
        else
#endif
            *p++ = (unsigned char)c, n--;
    }
    return s;
}
#endif


#ifndef __C40 /* 'C40 has assembler function */
size_t strlen(const char *a)            /* find number of chars in a string */
{   const char *x = a + 1;
#ifdef _copywords
    int w;
    while ((int)a & 3)
    {   if (*a++ == 0) return a - x;
    }
    {
        nullbyte_prologue_();
        while (w = *(int *)a, !word_has_nullbyte(w)) a += 4;
    }
#ifdef BYTESEX_EVEN
    if (w & 0xff)
    {   if (w & 0xff00)
        {   if (w & 0xff0000) a += 4;
            else a += 3;
        }
        else a += 2;
    }
    else a += 1;
#else
    if (w & 0xff000000)
    {   if (w & 0xff0000)
        {   if (w & 0xff00) a += 4;
            else a += 3;
        }
        else a += 2;
    }
    else a += 1;
#endif
#else
    while (*a++ != 0);
#endif
    return a - x;
}
#endif


/* end of string.c */
