
/* string.c: ANSI draft (X3J11 Oct 86) library code, section 4.11 */
/* Copyright (C) A.C. Norman and A. Mycroft */
/* version 0.03 */

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

/* comparison functions */

int memcmp(const void *a, const void *b, size_t n)
{   const unsigned char *ac = a, *bc = b;
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

/* search functions - ordered more logically then in ANSI spec */

void *memchr(const void *s, _chararg ch, size_t n)
                                            /* first instance of ch in s */
{   unsigned char *t = s;
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

char *strtok(char *s1, const char *s2)
{   static char *saves1 = "";
    char *s0;
    if (s1 == 0) s1 = saves1;                          /* use saved pointer */
    if (*(s1 += strspn(s1,s2)) == 0) s0 = 0;             /* no tokens */
    else { s0 = s1;                                 
           if (*(s1 += strcspn(s1,s2)) != 0) *s1++ = 0;  /* insert 0 if nec */
         }
    return (saves1 = s1, s0);
}

/* End string processing library */
