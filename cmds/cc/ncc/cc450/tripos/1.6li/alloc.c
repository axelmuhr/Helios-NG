
/* alloc.c: ANSI draft (X3J11 May 86) library, section 4.10 (part) */
/* The storage allocation package.                                 */
/* Copyright (C) A.C. Norman and A. Mycroft */
/* version 8 */

#include "hostsys.h"
#include <stdlib.h>
#include <stddef.h>
#include <string.h>

/* This version keep a map of blocks separate from the                   */
/* data that is allocated. This helps keep things safe when silly users  */
/* write (just) outside the limits of an allocated block.                */

/* I have a big worry here - if malloc and friends are called from a     */
/* signal handler (e.g. some asynchronous signal like an interrupt sent  */
/* by the user) the freestore pool might get updated in a peculiar way.  */
/* I need to set semaphores here and there to avoid this - the nastiest  */
/* case seems to be realloc. Hence funny settings of flags here.         */

/* In the current system the only genuinely assynchronous event is the   */
/* one that raises SIGINT (the user hitting the escape button). This is  */
/* treated specially in clib.s.startup, in that if _interrupts_off is    */
/* true and an escape happens then a local flag is set. A future call to */
/* _raise_stacked_interrupts() clears _interrupts_off and raises the     */
/* SIGINT event. The worst case misbehaviour I can see with the current  */
/* code is that if multiple escapes are sent some may get lost. For the  */
/* event concerned that is not too serious given that I guarantee that   */
/* at least one gets through.                                            */

/* AM: ensure that malloc/calloc/realloc act consistently     */
/* #define _alloc_bad_size() return NULL - better?            */
static void _alloc_bad_size()
{
    _sysdie("Overlarge or negative argument to malloc/calloc/realloc");
}

/* It is intended that #defining svc_allocation will allow the use of the */
/* following map maintenance code one day, for systems which allocate via */
/* SVC, but do not save the lengths.  Currently, however, the 370 version */
/* saves the length (slightly riskily) in [-1].                           */


#ifdef HOST_LACKS_ALLOC       /* no suitable alloc -- do it by steam */

#define round_to_words(size) ((size + (sizeof(int)-1)) & -sizeof(int))

typedef struct map_entry {void *address; size_t size; int busy;} map_entry;

static map_entry *map;
static int maptop, mapsize, first_free, inside_alloc;

#define INITIAL_MAP_SIZE 40

void _initalloc(char *endofprog, char *stackfirst)
{
    map = (map_entry *) endofprog;
/* map[0] is used to hold some check-words that can not possibly be      */
/* addresses.                                                            */
    map[0].address = 0;
    map[0].size = 0x456789ab;
    map[0].busy = 0xcba98765;
    map[1].address = (void *)endofprog;
    map[1].size = INITIAL_MAP_SIZE * sizeof(map_entry);
    map[1].busy = 1;
    map[2].address = (void *)((char *)map[1].address + map[1].size);
    map[2].size = stackfirst - (char *)map[2].address;
    map[2].busy = 0;
    maptop = 3;
    mapsize = INITIAL_MAP_SIZE;
    first_free = 1;
    inside_alloc = 0;
}

/* AM: until stack extension is implemented I want to know about all
   stack overflows.  Too many routines below did not stack check.
   The only possible problem is that 'syserr' may
   point to a file which has been 'freopen'd and needs an I/O buffer
   for its first write.  The answer is probably to make _sysdie be
   able to write without using a buffered file (c.f. 370 WTO).
*/
/*               #pragma -s1   */
/* This code is used in various critical places withing the library so   */
/* to avoid trouble it is compiled without stack checks. It only ever    */
/* uses a tiny amount of stack anyway (except maybe when things fail and */
/* it calls _sysdie, but that sort of disaster will lead to a mess anyway*/

static void *_realloc(void *p, size_t size);    /* for forward reference */

static void *_malloc(size_t size)
{
    int i, found_free;
    size_t s;
    if (size == 0) return NULL;
    if (map[0].address != 0 ||          /* Crude check on map validity   */
        map[0].size != 0x456789ab ||
        map[0].busy != 0xcba98765) _sysdie("Freestore directory corrupted");
    if (!inside_alloc && maptop+2>=mapsize)
    {   map_entry *newmap;
        inside_alloc = 1;
        newmap = (map_entry *)_realloc((void *)map,
                                    (mapsize *= 2) * sizeof(map_entry));
/* Unable to keep track of store status - give up!                       */
        if (newmap==NULL) return NULL;
        map = newmap;
        inside_alloc = 0;
    }
    size = round_to_words(size);
    found_free = 0;
    for (i=first_free; i < maptop; i++)
        if (!map[i].busy)
        {   if (!found_free)
            {   first_free = i;
                found_free = 1;
            }
            if ((s=map[i].size) >= size)
            {   char *b = (char *)map[i].address; /* The block I can use */
                map[i].busy = 1;
                if (s != size)                    /* Exact fit?          */
                {   int j;
                    map[i].size = size;
                    i++;
                    for (j = maptop; j > i; j--) map[j] = map[j-1];
                    maptop++;
                    map[i].address = (void *)((char *)b + size);
                    map[i].size = s - size;
                    map[i].busy = 0;
                }
                return (void *)b;
            }
        }
    return NULL;
}

static void merge_blocks(int i)
{
/* If the blocks indexed by i and i+1 are consecutive and both free then */
/* consolidate them into a single free block.                            */
    int j = i + 1;
    if (i < 1 || j >= maptop || map[i].busy || map[j].busy) return;
    if ((char *)map[i].address + map[i].size != (char *)map[j].address) return;
    map[i].size += map[j].size;
    maptop--;
    while (j < maptop)
    {   map[j] = map[j+1];
        j++;
    }
    if (first_free > i) first_free--;
}

static size_t _free(char *p)
{
/* Extended from ANSI spec to return size of the freed block.            */
    int low, high;
    if (p==NULL) return 0;  /* defined behaviour on NULL argument        */
    if (map[0].address != 0 ||          /* Crude check on map validity   */
        map[0].size != 0x456789ab ||
        map[0].busy != 0xcba98765) _sysdie("Freestore directory corrupted");
    low = 1;
    high = maptop - 1;
    while (low <= high)
    {   int i = (low + high) >> 1;
        void *midval = map[i].address;
        if (midval < (void *)p) low = i + 1;
        else if (midval > (void *)p) high = i - 1;
        else
        {   size_t size = map[i].size;
#ifdef svc_allocation            /* see top of file comment */
            _svc_free(p, size);
            squeeze_map(i);
#else
            if (!map[i].busy) _sysdie("free() on a block that was already free");
            map[i].busy = 0;
            if (i < first_free) first_free = i;
/* It is important to call merge_blocks in the order shown below since   */
/* each such call can rearrange the map.                                 */
            merge_blocks(i);
            merge_blocks(i-1);
#endif
            return size;
        }
    }
    _sysdie("free() on a block that had not been allocated");
    return 0;
}

static void *_realloc(void *p, size_t size)
{
    size_t old_size;
    if (p == NULL) return _malloc(size);
    if (size == 0)
    {   _free(p);
        return NULL;
    }
    if (!inside_alloc && maptop+2>=mapsize)
    {   map_entry *newmap;
        inside_alloc = 1;
        newmap = (map_entry *)_realloc((void *)map,
                                    (mapsize *= 2) * sizeof(map_entry));
        if (newmap==NULL) return NULL;
        map = newmap;
        inside_alloc = 0;
    }
    old_size = _free(p);       /* releases space but does not clobber it */
    {   char *newp = (char *)_malloc(size);
        int i;
        if (newp == NULL)
/* Here it is necessary to reconstruct the map entries that indicated */
/* that the block p was alive. At least I know that there will be     */
/* enough free space in the map, since _free() can never shrink it    */
/* even when the map decreases in size by large factors.              */
        {   int i;
            char *b;
            int s;
            for (i=first_free ;; i++)
            {   b = (char *)map[i].address;
                s = map[i].size;
                if (i >= maptop) _sysdie("Corrupt map in realloc()");
                if (b <= (char *)p &&
                    b + s >= (char *)p) break;
            }
/* That has found the slot in the map that spans the block p.         */
            if (b != (char *)p)  /* need to leave gap beneath block p */
            {   int j;
                for (j = maptop; j > i; j--) map[j] = map[j-1];
                maptop++;
                map[i].size = (char *)p - b;
                i++;
                s -= (char *)p - b;
                map[i].address = (void *)p;
                map[i].size = s;
            }
            map[i].busy = 1;       /* It is alive again.              */
            if (s != old_size)     /* need to leave gap above block p */
            {   int j;
                map[i].size = old_size;
                i++;
                for (j = maptop; j > i; j--) map[j] = map[j-1];
                maptop++;
                map[i].address = (void *)((char *)p + old_size);
                map[i].size = s - old_size;
                map[i].busy = 0;
            }
           return NULL;
        }
        else if (newp == (char *)p) return (void *)p;
/* Here I have to copy from the old to the new region. The only way they */
/* can overlap is with the new one at a lower address than the old, so   */
/* it is always safe to copy the lowest word first.                      */
        if (old_size < size) size = old_size;
        for (i=0; i<size; i+=4)
            *(int *)(newp + i) = *(int *)((char *)p + i);
        return (void *)newp;
    }
}

/* The procedures here interact with the signal handling code so that    */
/* assynchronous events never get raised while I am inside the freestore */
/* manager - instead the events get saved up and raised at the end.      */

void *malloc(size_t size)
{
    void *p;
    if (size & ~MAXSTORE) _alloc_bad_size();
    _interrupts_off = 1;
    p = _malloc(size);
    _raise_stacked_interrupts();
    return p;
}

void free(void *p)
{
    _interrupts_off = 1;
    (void)_free((char *)p);
    _raise_stacked_interrupts();
}

void *realloc(void *p, size_t size)
{
    if (size & ~MAXSTORE) _alloc_bad_size();
    _interrupts_off = 1;
    p = _realloc((char *)p, size);
    _raise_stacked_interrupts();
    return p;
}


/* More stack checking stuff:          #pragma -s0                       */

/* for internal use only */

#endif  /* HOST_LACKS_ALLOC */

void *_sys_alloc(size_t n)
{   void *a = malloc(n);
    /* do a 'raise' on the next line??? */
    if (a == 0) _sysdie("No store left for I/O buffer or the like");
    return a;
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

/* End of alloc.c */
