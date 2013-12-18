
/* alloc.c: ANSI draft (X3J11 May 86) library, section 4.10 (part) */
/* The storage allocation package.                                 */
/* Copyright (C) A.C. Norman and A. Mycroft */
/* version 0.07b */
/* $Id: alloc.c,v 1.3 1992/02/05 15:09:45 nickc Exp $ */

#include "norcrosys.h"
#include <stddef.h>
#include <string.h>
#include <syslib.h> /* HELIOSARM fix */

/* AM: ensure that malloc/calloc/realloc act consistently     */
/* #define _alloc_bad_size() return NULL - better?            */

#ifdef HELIOS                   
#define MAXSTORE 0x7fffffff 
#endif

#ifdef POSIX
#define MAXSTORE 0x7fffffff 
#endif

static void _alloc_bad_size()
{
    _sysdie("Overlarge or negative argument to malloc/calloc/realloc");
}


void *malloc(size_t size)
{ 
  return(Malloc(size));
}

void free(void *p)
{ Free(p);
}

void *realloc(void *ptr, size_t size)
{ size_t old_size;
  void *newptr;

  if (ptr == (void *) NULL)		/* NULL pointer legal here */
    return(Malloc(size));

  old_size = (size_t)MemSize(ptr);
  newptr   = Malloc(size);
  if (newptr == (void *) NULL)
    return(NULL);

  if (size >= old_size)			/* copy data without zapping memory */
    memcpy(newptr, ptr, old_size);
  else
    memcpy(newptr, ptr, size);

  Free(ptr);

  return(newptr);
}

  
void *calloc(size_t count, size_t size)
{
    void *r;
/* This miserable code computes a full 64-bit product for count & size   */
/* just so that it can verify that the said product really is in range   */
/* for handing to malloc.                                                */
    unsigned h = (count>>16)*(size>>16);
    unsigned m1 = (count>>16)*(size&0xffff);
    unsigned m2 = (count&0xffff)*(size>>16);
    unsigned l = (count&0xffff)*(size&0xffff);
    h += (m1>>16) + (m2>>16);
    m1 = (m1&0xffff) + (m2&0xffff) + (l>>16);
    l = (l&0xffff) | (m1<<16);
    h += m1>>16;
    if (h ||
        l & ~MAXSTORE) _alloc_bad_size();
    r = malloc(l);
    if (r != NULL) memset(r, 0, l);
    return r;
}

void *_sys_alloc(size_t n)
{   void *a = malloc(n);
    /* do a 'raise' on the next line??? */

    if (a == 0) _sysdie("No store left for I/O buffer or the like");
    return a;
}

/* End of alloc.c */
