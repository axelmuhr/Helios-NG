
/* string.c: ANSI draft (X3J11 Oct 86) library code, section 4.11 */
/* Copyright (C) A.C. Norman and A. Mycroft */
/* version 0.03 */
/* $Id: string.c,v 1.4 1993/07/12 10:41:09 nickc Exp $ */

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

/* BYTESEX_EVEN or BYTESEX_ODD should be defined for ARM/370 respectively  */

/* The following magic check was designed by A. Mycroft. It yields a     */
/* nonzero value if the argument w has a zero byte in it somewhere. The  */
/* messy constants have been unfolded a bit in this code so that they    */
/* get preloaded into registers before relevant loops.                   */

#ifdef _copywords  /* for safety */
#  define ONES_WORD   0x01010101
#  define EIGHTS_WORD 0x80808080
#  define nullbyte_prologue_() \
      int ones_word = ONES_WORD; int eights_word = ones_word << 7
#  define word_has_nullbyte(w) (((w) - ones_word) & ~(w) & eights_word)
#endif

/* copying functions */
#ifdef never
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

void *memmove(void *a, const void *b, size_t n)
/* copy memory taking care of overlap */
/* Relies on sizeof(int)=sizeof(void *) and byte addressing.
   Also that memory does not wrap round for direction test. */
{
#ifdef _copywords
    /* do it fast if word aligned ... */
    if ((((int)a | (int)b | (int)n) & 3) == 0)
    { int *wa,*wb;
      n >>= 2;
      if (a < (void *)b)
         for (wa = (int *)a, wb = (int *)b; n-- > 0;) *wa++ = *wb++;
      else for (wa = n+(int *)a, wb = n+(int *)b; n-- > 0;) *--wa = *--wb;
    }
    else
#endif
    { char *ca,*cb;
      if (a < (void *)b)
         for (ca = (char *)a, cb = (char *)b; n-- > 0;) *ca++ = *cb++;
      else for (ca = n+(char *)a, cb = n+(char *)b; n-- > 0;) *--ca = *--cb;
    }
    return a;
}

#if !defined(__ARM) && !defined(__C40)
/* ARM/C40 fn now in kernel (memory.c) as _memcpy is used in std resident libs */

void *_memcpy(void *a, const void *b, size_t n)
/* copy memory assuming no overlap, word aligned etc */
/* Relies on sizeof(int)=sizeof(void *) and byte addressing.
   Used by compiler for structure assignments */
{   
    int *wa,*wb;
    n >>= 2;
    for (wa = (int *)a, wb = (int *)b; n-- > 0;) *wa++ = *wb++;
    return a;
}
#endif

#ifdef POSIX
extern char * strcpy( char * dest, const char * source );
#else
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
#endif

/* comparison functions */

int memcmp(const void *a, const void *b, size_t n)
{   const unsigned char *ac = (const unsigned char *)a, *bc = (const unsigned char *)b;
#ifdef _copywords
    if ((((int)ac | (int)bc) & 3) == 0)
    {   while (n >= 4 && *(int *)ac == *(int *)bc)
            ac += 4, bc += 4, n -= 4;
    }
#endif
    while (n-- > 0)
    {   unsigned char c1,c2;   /* unsigned cmp seems more intuitive */
        if ((c1 = *ac++) != (c2 = *bc++)) return c1 - c2;
    }
    return 0;
}

#ifdef POSIX
extern int strcmp( const char * a, const char * b );
#else
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
#endif

/* search functions - ordered more logically then in ANSI spec */

void *memchr(const void *s, _chararg ch, size_t n)
                                            /* first instance of ch in s */
{   unsigned char *t = (unsigned char *)s; /* HELIOSARM fix */
    while (n-- > 0) if (*t == (unsigned char)ch) return t; else t++;
    return (void *)NULL;
}

/*  for the next two functions ANSI say you CAN search for '\0'.          */

char *strchr(const char *s, _chararg ch)
                                        /* find first instance of ch in s */
{   
    for (;; s++)
    {   if (*s == (char)ch) return (char *)s;
        if (*s == 0) return (char *)NULL;
    }
}

char *strrchr(const char *s, _chararg ch)  /* find last instance of ch in s */
{   const char *p = s;
    while (*p++ != 0);
    do { if (*--p == (char)ch) return (char *)p; } while (p!=s);
    return (char *)NULL;
}

/* N.B. strspn(s,"")==0 & strcspn(s,"")==strlen(s) means that the next two
   fns are not quite symmetric.  */

size_t strspn(const char *s, const char *p)
                                        /* find first char in s not in p */
{   const char *ss, *pp;
    char ch;
    for (ss=s;;ss++)
    {   if ((ch = *ss) == 0) return ss-s;
        for (pp=p;; pp++) 
        {   if (*pp == 0) return ss-s;
            if (*pp == ch) break;
        }
    }
}

size_t strcspn(const char *s, const char *p)
                                     /* find first char in s that is in p */
{   const char *ss, *pp;
    char ch;
    for (ss=s;;ss++)
    {   if ((ch = *ss) == 0) return ss-s;
        for (pp=p; *pp!=0; pp++) if (*pp == ch) return ss-s;
    }
}

char *strpbrk(const char *s, const char *p)
                                        /*  ditto, except ptr/NULL result */
{   const char *ss, *pp;
    char ch;
    for (ss=s;;ss++)
    {   if ((ch = *ss) == 0) return (char *) NULL;
        for (pp=p; *pp!=0; pp++) if (*pp == ch) return (char *)ss;
    }
}

char *strstr(const char *a, const char *b)
                              /* find first occurrence of b in a, or NULL */
{   int i;
    for (;;)
    {   for (i=0;; i++)
        {   if (b[i] == 0) return (char *)a;
            if (a[i] != b[i]) break;
        }
        if (*a++ == 0) return (char *) NULL;
    }
}

/* get around Helios/ARM compiler bug - no statics in fns */
static char *saves1 = "";

char *strtok(char *s1, const char *s2)
{
    char *s0;
    if (s1 == 0) s1 = saves1;                          /* use saved pointer */
    if (*(s1 += strspn(s1,s2)) == 0) s0 = 0;             /* no tokens */
    else { s0 = s1;                                 
           if (*(s1 += strcspn(s1,s2)) != 0) *s1++ = 0;  /* insert 0 if nec */
         }
    return (saves1 = s1, s0);
}

/* Miscellaneous functions */

#ifdef POSIX
extern size_t strlen( const char * str );
#else
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

/* BLV - I have added the two functions below. They are fairly useless as
   they are supposed to interact with the program's locale, and the only
   locale supported is "C". However, given this restriction the definitions
   below are perfectly legal and better than nothing.
*/

int strcoll(const char *s1, const char *s2)
{ return(strcmp(s1, s2));
}

size_t strxfrm(char *s1, const char *s2, size_t n)
{ size_t temp = strlen(s2);

  if (temp > n)
    return(0);
  else
    { strcpy(s1, s2);
      return(temp);
    }
}
 
/* End string processing library */
