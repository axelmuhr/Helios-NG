/*
 * mip/store.c: Storage allocation for the Codemist C compiler
 * Copyright (C) Codemist Ltd., 1987.
 * Copyright (C) Acorn Computers Ltd., 1988.
 */

/*
 * RCS $Revision: 1.1 $
 * Checkin $Date: 1993/11/23 16:41:52 $
 * Revising $Author: nickc $
 */

#ifndef NO_VERSION_STRINGS
extern char store_version[];
char store_version[] = "\nstore.c $Revision: 1.1 $ 55\n";
#endif

#ifdef __STDC__
#  include <stdlib.h>
#  include <string.h>
#else
#  include "stddef.h"                                   /* for size_t */
#  include "strings.h"
extern char *malloc();
extern free();
#endif
#include "globals.h"
#include "store.h"
#include "defs.h"
#include "mcdep.h"  /* usrdbg(xxx) */
#include "errors.h"

static char *alloc_chain;    /* see alloc_init(), alloc_dispose() */

static int32 stuse_total, stuse_waste;
static int32 stuse[SU_Other-SU_Data+1];
static int32 maxAEstore;

static VoidStar cc_alloc(int32 n)
{   char *p;
    stuse_total += n;
/* The next line's test probably only generates code on a PC.           */
    p = (sizeof(size_t) < sizeof(int32) && (unsigned32)(n+4) > 0xffff) ? 0 :
        malloc((size_t)(n+4));
    if (p != 0)
    { *(char **)p = alloc_chain;
      alloc_chain = p;
      return p+4;
    }
#ifdef TARGET_IS_ARM
    if (usrdbg(DBG_ANY))
        cc_fatalerr(misc_fatalerr_space2);
    else
#endif
        cc_fatalerr(misc_fatalerr_space3);
    return 0;   /* stop compiler wingeing re implicit junk return */
}

void alloc_dispose(void)
{
    unsigned32 count = 0;
    if (debugging(DEBUG_STORE)) cc_msg("Freeing block(s) at:");
    while (alloc_chain != NULL)
    {   char *next = *(char **)alloc_chain;
        if (debugging(DEBUG_STORE))
          cc_msg("%s %p", count++ % 8 == 0 ? "\n":"", alloc_chain);
        free(alloc_chain);
        alloc_chain = next;
    }
    if (debugging(DEBUG_STORE)) cc_msg("\n");
}

typedef struct Mark {
    struct Mark *prev;
    int syn_segno;
    char *syn_allp; int32 syn_hwm;
    int bind_segno;
    char *bind_allp; int32 bind_hwm;
} Mark;

static Mark *marklist;
static Mark *freemarks;

typedef struct FreeList {
    struct FreeList *next;
    int32 rest[1];
} FreeList;

static int     globallcnt;       /* count of segments allocated (int ok) */
static int32   globallxtra;      /* oversize global store allocated.    */
static char    *globallp;        /* pointers into symbol table          */
static char    *globalltop;      /* end of symbol table                 */
char    *currentfunction;

/* 'segmax' is the size of the notional arrays synsegbase[] etc.        */
static int segmax;
#define SEGMAX_INIT  16
#define SEGMAX_FACTOR 4

/* AM: one day turn segbase/segptr into a struct.                       */
static char    **synsegbase;       /* array of blocks of 'per routine' store */
static char    **synsegptr;        /* array of corresponding free addresses  */
static int     synsegcnt;          /* number thereof 0..segmax */
static char *synallp, *synalltop;  /* allocators therein */
static int32 synallhwm, synallmax; /* high water         */
static FreeList *synall2;          /* and a dispose list */
static FreeList *synall3;

static char    **bindsegbase;       /* list of blocks of 'per routine' store */
static char    **bindsegptr;        /* list of corresponding free addresses  */
static int     bindsegcnt;          /* number thereof    0..segmax */
static int     bindsegcur;          /* next block to use 0..segmax */
static char *bindallp, *bindalltop; /* allocators therein */
static int32 bindallhwm, bindallmax;/* high water         */
static FreeList *bindall2;          /* and a dispose list */
static FreeList *bindall3;

char *phasename;

static VoidStar new_global_segment(void)
{
    char *w;
/* I will recycle a segment that had been used for local space if there  */
/* are any such available.                                               */
    if (bindsegcur < bindsegcnt)
    {   w = bindsegbase[--bindsegcnt];
        if (debugging(DEBUG_STORE))
            cc_msg("Global store %d from binder size %ld at %p\n",
                    (int)globallcnt, (long)SEGSIZE, w);
    }
    else
    {   w = cc_alloc(SEGSIZE);
        if (debugging(DEBUG_STORE))
            cc_msg("Global store alloc %d size %ld at %p (in %s)\n",
                    (int)globallcnt, (long)SEGSIZE, w, currentfunction);
    }
    globallcnt++;
    globallp = w, globalltop = w + SEGSIZE;
    return w;
}

VoidStar GlobAlloc(StoreUse t, int32 n)
{   char *p = globallp;
    n = n + (sizeof (int) - 1) & ~(int32)(sizeof (int) - 1);     /* make n a multiple of sizeof(int) */
    
    if (n > SEGSIZE)
    {   /* Big global store requests get a single oversize page.        */
        p = cc_alloc(n);
        if (debugging(DEBUG_STORE))
            cc_msg("Global overlarge store alloc size %ld at %p (in %s)\n",
                    (long)n, p, currentfunction);
        globallxtra += n;               /* could update globallcnt?     */
    }
    else
    {   if (p+n > globalltop)
            stuse_waste += globalltop-p,
            p = new_global_segment();
        globallp = p + n;
    }
    stuse[(int)t] += n;
    return p;
}

VoidStar xglobal_cons2(StoreUse t, int32 a, int32 b)
{
    int32 *p = (int32 *) GlobAlloc(t, sizeof(int32)*2);
    p[0] = a; p[1] = b;
    return (VoidStar) p;
}

VoidStar xglobal_list3(StoreUse t, int32 a, int32 b, int32 c)
{
    int32 *p = (int32 *) GlobAlloc(t, sizeof(int32)*3);
    p[0] = a; p[1] = b; p[2] = c;
    return (VoidStar) p;
}

VoidStar xglobal_list4(StoreUse t, int32 a, int32 b, int32 c, int32 d)
{
    int32 *p = (int32 *) GlobAlloc(t, sizeof(int32)*4);
    p[0] = a; p[1] = b; p[2] = c; p[3] = d;
    return (VoidStar) p;
}

VoidStar xglobal_list5(StoreUse t, int32 a, int32 b, int32 c, int32 d, int32 e)
{
    int32 *p = (int32 *) GlobAlloc(t, sizeof(int32)*5);
    p[0] = a; p[1] = b; p[2] = c; p[3] = d; p[4] = e;
    return (VoidStar)p;
}

VoidStar xglobal_list6(StoreUse t, int32 a, int32 b, int32 c, int32 d, int32 e, int32 f)
{
    int32 *p = (int32 *) GlobAlloc(t, sizeof(int32)*6);
    p[0] = a; p[1] = b; p[2] = c; p[3] = d; p[4] = e; p[5] = f;
    return (VoidStar)p;
}

/* Volatile storage allocation for use within treatment of 1 function.  */

/* The following functions avoid a fixed limit on the number of pages   */
/* of local allocation without excessive store use.                     */
/* @@@ make this function more global soon (e.g. for ICODE &c)          */
/* The argument sizes are in bytes and old is unexamined if oldsize=0.  */
static VoidStar expand_array(VoidStar old, int32 oldsize, int32 newsize)
{   /* beware the next line if we ever record GlobAlloc's:              */
    VoidStar new = GlobAlloc(SU_Other, newsize);
    if (oldsize != 0) memcpy(new, old, (size_t)oldsize);
    return new;
}

static void expand_segmax(int newsegmax)
{   int32 osize = (int32)segmax * sizeof(char *),
          nsize = (int32)newsegmax * sizeof(char *);
    synsegbase =  (char **)expand_array((VoidStar)synsegbase, osize, nsize);
    synsegptr  =  (char **)expand_array((VoidStar)synsegptr, osize, nsize);
    bindsegbase = (char **)expand_array((VoidStar)bindsegbase, osize, nsize);
    bindsegptr  = (char **)expand_array((VoidStar)bindsegptr, osize, nsize);
    segmax = newsegmax;
}

static VoidStar new_bindalloc_segment(void)
{
    if (bindsegcur >= bindsegcnt)
    {   char *w = cc_alloc(SEGSIZE);
        if (bindsegcnt >= segmax) expand_segmax(segmax * SEGMAX_FACTOR);
        if (debugging(DEBUG_STORE))
            cc_msg("Binder store alloc %d size %ld at %p (%s in %s)\n",
                    (int)bindsegcnt, (long)SEGSIZE, w, phasename, currentfunction);
        bindsegbase[bindsegcnt++] = w;
    }
    return bindsegbase[bindsegcur++];
}

static VoidStar new_synalloc_segment(void)
{
    char *w;
    if (synsegcnt >= segmax) expand_segmax(segmax * SEGMAX_FACTOR);
    if (bindsegcur < bindsegcnt)
    {   w = bindsegbase[--bindsegcnt];
        if (debugging(DEBUG_2STORE) && synsegcnt>0)
            cc_msg("Syntax store %d from binder size %ld at %p\n",
                    (int)synsegcnt, (long)SEGSIZE, w);
    }
    else
    {   w = cc_alloc(SEGSIZE);
        if (debugging(DEBUG_STORE))
            cc_msg("Syntax store alloc %d size %ld at %p (%s in %s)\n",
                    (int)synsegcnt, (long)SEGSIZE, w, phasename, currentfunction);
    }
    return synsegbase[synsegcnt++] = w;
}

VoidStar BindAlloc(int32 n)
{
    char *p = bindallp;
    n = n + 3 & ~(int32)3;     /* make n a multiple of sizeof(int) */
    if (n > SEGSIZE) syserr(syserr_overlarge_store1, (long)n);
    if (p + n > bindalltop)
    {   int i;                                 /* 0..segmax */
        if (bindsegcur > 0)
            bindsegptr[bindsegcur-1] = p;      /* stash highest used */
        for (i = marklist->bind_segno;; i++)   /* search for scraps  */
        {   if (i == bindsegcur)               /* nowhere big enough */
            {   p = new_bindalloc_segment();
                bindalltop = p + SEGSIZE;
                break;
            }
            p = bindsegptr[i];                 /* hope springs eternal */
            bindalltop = bindsegbase[i] + SEGSIZE;
            if (p+n <= bindalltop)             /* fingers crossed      */
            {   /* we have scavenged something useful - swap to current */
                char *t = bindsegbase[i];
                bindsegbase[i] = bindsegbase[bindsegcur-1];
                bindsegbase[bindsegcur-1] = t;
                bindsegptr[i] = bindsegptr[bindsegcur-1];
                if (debugging(DEBUG_2STORE))
                {   cc_msg("Scavenge binder %d (%p), %ld left\n",
                            (int)i, t, (long)(bindalltop-(p+n)));
                }
                break;
            }
        }
        bindsegptr[bindsegcur-1] = DUFF_ADDR;
    }
    bindallp = p + n;
    if ((bindallhwm += n) > bindallmax) bindallmax = bindallhwm;
    return p;
}

VoidStar SynAlloc(int32 n)
{   char *p = synallp;
    n = n + 3 & ~(int32)3;     /* make n a multiple of sizeof(int) */
    if (n > SEGSIZE) syserr(syserr_overlarge_store2, (long)n);
    if (p + n > synalltop)
    {   int i;                                 /* 0..segmax */
        if (synsegcnt > 0)
            synsegptr[synsegcnt-1] = p;        /* stash highest used */
        for (i = marklist->syn_segno;; i++)    /* search for scraps  */
        {   if (i == synsegcnt)                /* nowhere big enough */
            {   p = new_synalloc_segment();
                synalltop = p + SEGSIZE;
                break;
            }
            p = synsegptr[i];                  /* hope springs eternal */
            synalltop = synsegbase[i] + SEGSIZE;
            if (p+n <= synalltop)              /* fingers crossed      */
            {   /* we have scavenged something useful - swap to current */
                char *t = synsegbase[i];
                synsegbase[i] = synsegbase[synsegcnt-1];
                synsegbase[synsegcnt-1] = t;
                synsegptr[i] = synsegptr[synsegcnt-1];
                if (debugging(DEBUG_2STORE))
                {   cc_msg("Scavenge syntax %d (%p), %ld left\n",
                            (int)i, t, (long)(synalltop-(p+n)));
                }
                break;
            }
        }
        synsegptr[synsegcnt-1] = DUFF_ADDR;
    }
    synallp = p + n;
    if ((synallhwm += n) > synallmax) synallmax = synallhwm;
    return p;
}

VoidStar discard2(VoidStar p)
{
/* As cdr_(p) but returns the cell p to freestorage pool.      */
/* The freechain has a funny number xored in to help debugging */
    FreeList *pp = (FreeList *)(int) p;
    VoidStar q = (VoidStar) pp->next;
    int i;                   /* 0..segmax */
    pp->rest[0] ^= (unsigned long)0x99990000;   /* to help with debugging */
    for (i = 0; i < synsegcnt; i++)
       if (synsegbase[i] <= (char *)pp && (char *)pp < synsegbase[i]+SEGSIZE)
       {   pp->next = synall2;
           synall2 = (FreeList *)(((int32) pp) ^ 0x6a6a6a6a);
           return q;
       }
    for (i = 0; i < bindsegcur; i++)
       if (bindsegbase[i] <= (char *)pp && (char *)pp < bindsegbase[i]+SEGSIZE)
       {   pp->next = bindall2;
           bindall2 = (FreeList *)(((int32) pp) ^ 0x5a5a5a5a);
           return q;
       }
    syserr(syserr_discard2, (VoidStar) pp);
    return q;
}

VoidStar xsyn_list2(int32 a, int32 b)
{   int32 *p;
    if (synall2==NULL)
        p = (int32 *) SynAlloc(sizeof(int32)*2);
    else
    {   p = (int32 *)((int32) synall2 ^ 0x6a6a6a6a);
        synall2 = (FreeList *) p[0];
    }
    p[0] = a; p[1] = b;
    return (VoidStar) p;
}

VoidStar xbinder_list2(int32 a, int32 b)
{   if (bindall2==NULL)
    {   int32 *p = (int32 *) BindAlloc(sizeof(int32)*2);
        p[0] = a; p[1] = b;
        return (VoidStar) p;
    }
    else
    {   int32 *p = (int32 *)((int32) bindall2 ^ 0x5a5a5a5a);
        bindall2 = (FreeList *) p[0];
        p[0] = a; p[1] = b;
        return (VoidStar) p;
    }
}

VoidStar discard3(VoidStar p)
{
/* Returns the cell p to freestorage pool, with a funny number xored in to
 * help debugging.
 * Return value is (the old value of) p->next */
    FreeList *pp = (FreeList *)(int) p;
    VoidStar q = (VoidStar) pp->next;
    int i;                   /* 0..segmax */
    for (i = 0; i < synsegcnt; i++)
       if (synsegbase[i] <= (char *)pp && (char *)pp < synsegbase[i]+SEGSIZE)
       {   pp->next = synall3;
           synall3 = (FreeList *)(((int32) pp) ^ 0x6a6a6a6a);
           return q;
       }
    for (i = 0; i < bindsegcur; i++)
       if (bindsegbase[i] <= (char *)pp && (char *)pp < bindsegbase[i]+SEGSIZE)
       {   pp->next = bindall3;
           bindall3 = (FreeList *)(((int32) pp) ^ 0x5a5a5a5a);
           return q;
       }
    syserr(syserr_discard3, (VoidStar) pp);
    return q;
}

VoidStar xbinder_list3(int32 a, int32 b, int32 c)
{
    int32 *p;
    if (bindall3 == NULL)
        p = (int32 *) BindAlloc(sizeof(int32)*3);
    else {
        p = (int32 *)((int32) bindall3 ^ 0x5a5a5a5a);
        bindall3 = (FreeList *) p[0];
    }
    p[0] = a; p[1] = b; p[2] = c;
    return (VoidStar) p;
}

VoidStar xsyn_list3(int32 a, int32 b, int32 c)
{   int32 *p;
    if (synall3 == NULL)
        p = (int32 *) SynAlloc(sizeof(int32)*3);
    else {
        p = (int32 *)((int32) synall3 ^ 0x6a6a6a6a);
        synall3 = (FreeList *) p[0];
    }
    p[0] = a; p[1] = b; p[2] = c;
    return (VoidStar) p;
}

VoidStar xsyn_list4(int32 a, int32 b, int32 c, int32 d)
{   int32 *p = (int32 *) SynAlloc(sizeof(int32)*4);
    p[0] = a, p[1] = b, p[2] = c, p[3] = d;
    return (VoidStar) p;
}

VoidStar xsyn_list5(int32 a, int32 b, int32 c, int32 d, int32 e)
{
    int32 *p = (int32 *) SynAlloc(sizeof(int32)*5);
    p[0] = a; p[1] = b; p[2] = c; p[3] = d; p[4] = e;
    return (VoidStar) p;
}

VoidStar xsyn_list6(int32 a, int32 b, int32 c, int32 d, int32 e, int32 f)
{   int32 *p = (int32 *) SynAlloc(sizeof(int32)*6);
    p[0] = a, p[1] = b, p[2] = c, p[3] = d, p[4] = e, p[5] = f;
    return (VoidStar) p;
}

VoidStar xsyn_list7(int32 a, int32 b, int32 c, int32 d, int32 e, int32 f,
                   int32 g)
{   int32 *p = (int32 *) SynAlloc(sizeof(int32)*7);
    p[0] = a, p[1] = b, p[2] = c, p[3] = d, p[4] = e, p[5] = f, p[6] = g;
    return (VoidStar) p;
}

void alloc_mark(void)           /* @@@ not used essentially for C       */
{
    Mark *p;
    if ((p = freemarks) != NULL)
        freemarks = p->prev;
    else
        p = (Mark *) GlobAlloc(SU_Other, sizeof(Mark));

    p->prev = marklist; marklist = p;
    p->syn_segno = synsegcnt;
    p->syn_allp = synallp; p->syn_hwm = synallhwm;
    p->bind_segno = bindsegcur;
    p->bind_allp = bindallp; p->bind_hwm = bindallhwm;

    if (debugging(DEBUG_STORE))
        cc_msg("Mark %d, %p, %lx :: %d, %p, %lx\n",
                synsegcnt, synallp, (long)synallhwm,
                bindsegcur, bindallp, (long)bindallhwm);
}

#ifdef PASCAL_OR_FORTRAN
void alloc_unmark(void)
{
    Mark *p = marklist;
    if (p->prev == NULL) syserr(syserr_alloc_unmark);
    if (synsegcnt > p->syn_segno)
        syserr(syserr_alloc_unmark1);
    marklist = p->prev;
    p->prev = freemarks; freemarks = p;
    synsegcnt = p->syn_segno; synallp = p->syn_allp;
    synalltop = (synallp == DUFF_ADDR) ? DUFF_ADDR
                                       : synsegbase[synsegcnt-1] + SEGSIZE;
    synallhwm = p->syn_hwm;
    bindsegcur = p->bind_segno; bindallp = p->bind_allp;
    bindalltop = (bindallp == DUFF_ADDR) ? DUFF_ADDR
                                         : bindsegbase[bindsegcur-1] + SEGSIZE;
    bindallhwm = p->bind_hwm;

    if (debugging(DEBUG_STORE))
        cc_msg("Unmark %d, %p, %lx :: %d, %p, %lx\n",
                synsegcnt, synallp, (long)synallhwm,
                bindsegcur, bindallp, (long)bindallhwm);
}
#endif

void drop_local_store(void)
{
/* Here the threat issued using SynAlloc or syn_xxx materialises, and a
   lot of local store is trampled upon. */
/* N.B. drop_local_store *MUST* be called before reinit_alloc()          */
   while (synsegcnt > marklist->syn_segno)
    {    char *p = synsegbase[--synsegcnt];
         if (bindsegcnt >= segmax) expand_segmax(segmax * SEGMAX_FACTOR);
#ifdef never
         if (debugging(DEBUG_2STORE))
             cc_msg("Re-using syntax store %p as binder %d\n",
                     p, (int)bindsegcnt);
#endif
/* we do not need to mess with limits here as set to SEGSIZE when used */
         bindsegbase[bindsegcnt++] = p;
    }
    synallp = marklist->syn_allp;
    synalltop = (synallp == DUFF_ADDR) ? DUFF_ADDR
                                       : synsegbase[synsegcnt-1] + SEGSIZE;
    if (debugging(DEBUG_2STORE) && synallhwm==synallmax)
        cc_msg("Max SynAlloc %ld in %s\n",
                (long)synallmax, currentfunction);
    synallhwm = marklist->syn_hwm;
    synall2 = NULL; synall3 = NULL;
}

void alloc_reinit(void)
{   if (synsegcnt > marklist->syn_segno)
        syserr(syserr_alloc_reinit);
    synallp = marklist->syn_allp;
    synalltop = (synallp == DUFF_ADDR) ? DUFF_ADDR
                                       : synsegbase[synsegcnt-1] + SEGSIZE;
    synall2 = NULL; synall3 = NULL;
    bindallhwm = marklist->bind_hwm;
    bindsegcur = marklist->bind_segno; bindallp = marklist->bind_allp;
    bindalltop = (bindallp == DUFF_ADDR) ? DUFF_ADDR
                                         : bindsegbase[bindsegcur-1] + SEGSIZE;
    bindall2 = NULL; bindall3 = NULL;
}

void alloc_init(void)
{
    /* reset the following vars for each one of a many file compilation */
    stuse_total = 0, stuse_waste = 0;
    memclr(stuse, sizeof(stuse));
    alloc_chain = NULL;
    synsegcnt = 0;
    synallp = synalltop = DUFF_ADDR;
    synall2 = NULL; synall3 = NULL;
    synallhwm = 0, synallmax = 0;
    bindsegcur = 0, bindsegcnt = 0;
    bindallp = bindalltop = DUFF_ADDR;
    bindallhwm = 0, bindallmax = 0;
    bindall2 = NULL; bindall3 = NULL;
    globallcnt = 0; globallxtra = 0;
    globallp = globalltop = DUFF_ADDR;
    marklist = NULL; freemarks = NULL;
    maxAEstore = 0;
    synsegbase = synsegptr = bindsegbase = bindsegptr = DUFF_ADDR;
    segmax = 0; expand_segmax(SEGMAX_INIT);
    alloc_mark();
}

void alloc_noteAEstoreuse(void)
/* Calculate as blocks allocated minus space unused in (only) LAST BLOCK */
{   int32 n = ((int32)synsegcnt*SEGSIZE - (synalltop - synallp)) +
            ((int32)bindsegcur*SEGSIZE - (bindalltop - bindallp));
    if (n > maxAEstore) maxAEstore = n;
}

void show_store_use(void)
{
#ifdef ENABLE_STORE
    cc_msg(
        "Total store use (excluding stdio buffers/stack) %ld bytes\n",
        (long)stuse_total);
    cc_msg("Global store use %ld/%ld + %ld bytes\n",
        (long)((int32)globallcnt*SEGSIZE - (globalltop - globallp)),
        (long)((int32)globallcnt*SEGSIZE),
        (long)globallxtra);
    cc_msg(
        "  thereof %ld+%ld bytes pended relocation, %ld bytes pended data\n",
        (long)stuse[(int)SU_Xref],
        (long)stuse[(int)SU_Xsym],
        (long)stuse[(int)SU_Data]);
    cc_msg(
        "  %ld bytes symbols, %ld bytes top-level vars, %ld bytes types\n",
        (long)stuse[(int)SU_Sym],
        (long)stuse[(int)SU_Bind],
        (long)stuse[(int)SU_Type]);
    cc_msg(
        "  %ld bytes constants, %ld bytes pre-processor, %ld bytes wasted\n",
        (long)stuse[(int)SU_Const], (long)stuse[(int)SU_PP], (long)stuse_waste);
    cc_msg( "Local store use %ld+%ld/%ld bytes - front end max %ld\n",
        (long)synallmax, (long)bindallmax,
        (long)((int32)(int)(synsegcnt+bindsegcnt)*SEGSIZE),
        (long)maxAEstore);
#endif /* ENABLE_STORE */
}

/* end of mip/store.c */
