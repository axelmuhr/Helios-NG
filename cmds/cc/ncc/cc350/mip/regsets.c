/*
 * regsets.c - space-efficient set representations, version 1a.
 * Copyright (C) Acorn Computers Ltd., 1988.
 */

/*
 * RCS $Revision: 1.1 $
 * Checkin $Date: 1993/11/23 16:41:52 $
 * Revising $Author: nickc $
 */

#ifndef NO_VERSION_STRINGS
extern char regsets_version[];
char regsets_version[] = "\nregsets.c $Revision: 1.1 $ 1a\n";
#endif

/* Memo: @@@ someone misunderstands either the use of unsigned32 or     */
/* the binding power of cases in    (unsigned)regno / RANGESIZE.        */
/* [ACN has made RANGESIZE an unsigned value and has put extra parens   */
/* near where it is used: Happy now, Alan?]                             */
/* AM thinks that regno should really be unsigned32 (or VRegnum?)       */

#ifdef __STDC__
#  include <string.h>
#else
#  include <strings.h>
#endif
#include "globals.h"
#include "regsets.h"
#include "store.h"


/* RegList operations */

RegList *rldiscard(RegList *x)
{   return (RegList *)discard2((List *)x);
}

bool member(VRegnum a, RegList *l)
{
    for (; l != NULL; l = l->rlcdr)
        if (a == l->rlcar) return YES;
    return NO;
}

RegList *ndelete(VRegnum a, RegList *l)
{
/* delete a from the list l (destructive operation) */
    RegList *r = l, *s;
    if (l == NULL)
        return l;
    else if (l->rlcar == a)
        return rldiscard(l);
    do
    {   s = l;
        l = l->rlcdr;
        if (l == NULL) return r;
    } while (l->rlcar != a);
    s->rlcdr = rldiscard(l);
    return r;
}


/*
 * Macros for bit-vector access. Not sure quite how portable these are,
 * but having BitmapChunk as unsigned char probably makes it quite safe.
 */

#define bitmapchunk(bitmap, bitno) ((bitmap)[(bitno) >> 3])
#define bitmapbit(bitno) (1 << ((bitno) & (BITSPERCHUNK-1)))

/*
 *       **************** Sets of registers *****************
 *  This implementation works best if the sets are locally dense, but
 *  globally sparse, without being too awful in the two extremes.
 *  (Implementation is by a segmented bitmap)
 *  The segments are now stored ordered by element size (rather than most
 *  recently referenced first), since that makes the set operations (union
 *  intersection, difference, compare) much faster.
 */

/* Beware: the number RANGESIZE on the next line is not freely          */
/* changeable.  See the union for VRegset and calls to discard3().      */
/* Moreover magic number 0x80 below must correspond to BITSPERCHUNK.    */
#define RANGESIZE    ((unsigned32)32)
#define BITSPERCHUNK 8

typedef unsigned char BitmapChunk;

typedef struct VRegSet {
    struct VRegSet *next;
    unsigned rangeid;
    union {
       unsigned32 bitword;
       BitmapChunk bits[RANGESIZE/BITSPERCHUNK];
    } bits;
} VRegSet;

static VRegSet *newchunk(VRegSetAllocRec *allocrec, VRegSet *next, int32 id, int32 n)
{
    AllocType type = allocrec->alloctype;
    (*allocrec->statsloc)++;
    return (VRegSet*) (
           (type == AT_Syn) ?  syn_list3((int32) next, id, n) :
           (type == AT_Bind) ? binder_list3(next, id, n) :
                               global_list3(SU_Other, next, id, n));
}

extern VRegSet *vregset_insert(int32 regno, VRegSet *set, bool *oldp,
                               VRegSetAllocRec *allocrec)

{   VRegSet *p, *prev;
    unsigned32 range = ((unsigned32)regno) / RANGESIZE;

    regno &= (RANGESIZE-1);
    
    for (prev = NULL, p = set; p != NULL; prev = p, p = p->next)
      {
        if (p->rangeid <= range) break;
      }
    
    if (p == NULL || p->rangeid < range)
      {
        VRegSet *q = newchunk(allocrec, p, range, 0);
        if (prev == NULL)
            set = q;
        else
            prev->next = q;
        p = q;
      }
    
    if (oldp) *oldp = bitmapchunk(p->bits.bits, regno) & bitmapbit(regno);
    
    bitmapchunk(p->bits.bits, regno) |= bitmapbit(regno);
    
    return set;
}

extern int vregset_member(int32 regno, VRegSet *set)
{   VRegSet  *p;
    unsigned32 range = ((unsigned32)regno) / RANGESIZE;
    regno &= (RANGESIZE-1);
    for (p = set; p != NULL; p = p->next) {
        if (p->rangeid <= range) { /* may have it */
            if (p->rangeid < range) return NO;
            return(bitmapchunk(p->bits.bits, regno) & bitmapbit(regno));
        }
    }
    return NO;
}

extern VRegSet *vregset_delete(int32 regno, VRegSet *set, bool *oldp)
{   VRegSet  *p, *prev = NULL;
    unsigned32 range = ((unsigned32)regno) / RANGESIZE;
    regno &= (RANGESIZE-1);
    if (oldp) *oldp = NO;
    for (p = set; p != NULL; prev = p, p = p->next)
        if (p->rangeid <= range) { /* may have it */
            if (p->rangeid < range) return set;
            if (oldp) *oldp = bitmapchunk(p->bits.bits, regno) & bitmapbit(regno);

            bitmapchunk(p->bits.bits, regno) &= ~bitmapbit(regno);

            if (p->bits.bitword == 0)
	      {
                VRegSet *next = (VRegSet *) discard3((VoidStar) p);

		
                if (prev == NULL)
                    set = next;
                else
                    prev->next = next;
                return set;
            }
        }
    return set;
}


extern void vregset_map(VRegSet *set, RProc *dothis, VoidStar arg)
{
    VRegSet *p;
    for (p = set; p != NULL; p = p->next) {
        unsigned32 base = p->rangeid * RANGESIZE;
        int32    j;
        for (j = RANGESIZE/BITSPERCHUNK-1 ; j >= 0 ; j--) {
            BitmapChunk chunk = p->bits.bits[j];
            if (chunk != 0) {
                int32 reg = base + j*BITSPERCHUNK + BITSPERCHUNK-1;
                do {
                    /* AM: funny order to map regs in...                */
                    if (chunk & 0x80) dothis(reg, arg);
                    reg--;
                    chunk = chunk<<1;
                } while (chunk!=0);
            }
        }
    }
}

extern void vregset_map2(VRegSet *set, RProc2 *dothis)
{
    vregset_map(set, (RProc *)dothis, (VoidStar)NULL);
}

extern void vregset_discard(VRegSet *set)
{
    while (set != NULL) set = (VRegSet *) discard3((VoidStar) set);
}

extern int vregset_compare(VRegSetP s1, VRegSetP s2)
{
    bool res = VR_EQUAL;
    while (s1 != NULL && s2 != NULL) {
        int32 r1 = s1->rangeid,
              r2 = s2->rangeid;
        if (r1 == r2) {
            int32 w1 = s1->bits.bitword,
                  w2 = s2->bits.bitword;
            if (w1 == w2);
            else if ((w1 & w2) == w1) {
                if (res == VR_SUPERSET) return VR_UNORDERED;
                if (res == VR_EQUAL) res = VR_SUBSET;
            } else if ((w1 & w2) == w2) {
                if (res == VR_SUBSET) return VR_UNORDERED;
                if (res == VR_EQUAL) res = VR_SUPERSET;
            } else
                return VR_UNORDERED;
            s1 = s1->next; s2 = s2->next;

        } else if (r1 > r2) {
            if (res == VR_SUBSET) return VR_UNORDERED;
            if (res == VR_EQUAL) res = VR_SUPERSET;
            s1 = s1->next;

        } else {
            if (res == VR_SUPERSET) return VR_UNORDERED;
            if (res == VR_EQUAL) res = VR_SUBSET;
            s2 = s2->next;
        }
    }
    if (s1 != NULL)
        return (res == VR_SUBSET) ? VR_UNORDERED : VR_SUPERSET;
    else if (s2 != NULL)
        return (res == VR_SUPERSET) ? VR_UNORDERED : VR_SUBSET;
    return res;
}

extern VRegSetP vregset_difference(VRegSetP s1, VRegSetP s2)
{
    VRegSet *p1 = s1, *p2 = s2, *prev1 = NULL;
    while (p1 != NULL && p2 != NULL) {
        if (p1->rangeid < p2->rangeid)
            p2 = p2->next;
        else if (p1->rangeid == p2->rangeid) {
            VRegSet *next1 = p1->next;
            p1->bits.bitword &= ~p2->bits.bitword;
            if (p1->bits.bitword == 0) {
                if (prev1 == NULL)
                    s1 = next1;
                else
                    prev1->next = next1;
                discard3((VoidStar) p1);
            } else
                prev1 = p1;
            p1 = next1;
        } else {
           prev1 = p1;
           p1 = p1->next;
        }
    }
    return s1;
}

extern VRegSetP vregset_union(VRegSetP s1, VRegSetP s2, VRegSetAllocRec *allocrec)
{
    VRegSet *p1 = s1, *p2 = s2, *prev1 = NULL;
    while (p1 != NULL && p2 != NULL) {
        if (p1->rangeid == p2->rangeid) {
            p1->bits.bitword |= p2->bits.bitword;
            p2 = p2->next;
        } else if (p1->rangeid < p2->rangeid) {
            VRegSet *q = newchunk(allocrec, p1, p2->rangeid, p2->bits.bitword);
            if (prev1 == NULL)
                s1 = q;
            else
                prev1->next = q;
            p1 = q; p2 = p2->next;
        }
        prev1 = p1; p1 = p1->next;
    }
    while (p2 != NULL) {
        VRegSet *q = newchunk(allocrec, NULL, p2->rangeid, p2->bits.bitword);
        if (prev1 == NULL)
            s1 = q;
        else
            prev1->next = q;
        prev1 = q; p2 = p2->next;
    }
    return s1;
}

extern VRegSetP vregset_intersection(VRegSetP s1, VRegSetP s2)
{
    VRegSet *p1 = s1, *p2 = s2, *prev1 = NULL;
    while (p1 != NULL && p2 != NULL) {
        if (p1->rangeid < p2->rangeid)
            p2 = p2->next;
        else {
            VRegSet *next1 = p1->next;
            if (p1->rangeid == p2->rangeid) {
                p1->bits.bitword &= p2->bits.bitword;
                if (p1->bits.bitword != 0) {
                    prev1 = p1; p1 = next1;
                    continue;
                }
            }
            if (prev1 == NULL)
                s1 = next1;
            else
                prev1->next = next1;
            discard3((VoidStar) p1);
            p1 = next1;
        }
    }
    if (p1 != NULL) {
        if (prev1 == NULL)
            s1 = NULL;
        else
            prev1->next = NULL;
        vregset_discard(p1);
    }
    return s1;
}

extern VRegSetP vregset_copy(VRegSetP set, VRegSetAllocRec *allocrec)
{
    VRegSet *copy = NULL;
    for ( ; set != NULL ; set = set->next) {
        copy = newchunk(allocrec, copy, set->rangeid, set->bits.bitword);
    }
    return (VRegSetP) dreverse((List *) copy);
}

extern void vregset_init(void)
{
}

/*
 * Start of (fairly) abstract type for reg clash matrix. RCC 19/02/88.
 * The matrix representation exploits the sparseness, symmetry, and
 * locality of the clash matrix.  Essentially we split it up into
 * 16x16 bit squares, and store only those squares which are not entirely
 * zero.
 *
 * To exploit symmetry we do not store the transpose of each square,
 * but just link it off the other idx.  We choose to access things
 * from the larger idx whenever possible (choose larger as local vars
 * tend to have low VReg numbers, and clash with lots of things: want
 * the resulting squares to be spread over many lists).
 *
 * The sparse matrix and the clash2 lists are allocated with SynAlloc,
 * as I believe they are now no bigger than the syntax tree (for a
 * big proc, 6000 lines and 12400 vregs, had about 1.1MB in front-end
 * and 560K here).
 *
 * Things rest of the file can use from here are:
 *   add_clash(), vregset_member(), remove_clashes(),
 *   clashkillbits(), printvregclash(), printvregclash2().
 */

#define BLOCKBITS 3  /* 3 or 4 are probably the only sensible values */
#define BLOCKSIZE (1<<BLOCKBITS)
#define BITSPERSQUARE (BLOCKSIZE*BLOCKSIZE)
#define BITMAPSIZE ((BLOCKSIZE*BLOCKSIZE)/BITSPERCHUNK)

#define blockno(reg) ((BlockNo)((reg) >> BLOCKBITS))
#define lowbits(reg) ((reg)&(BLOCKSIZE-1))

#define bitidx(a, b) (((a)<<BLOCKBITS) | (b))

typedef unsigned BlockNo;

typedef struct Square {
    struct Square *next;            /* list of squares owned by masteridx */
    unsigned32    blockid;          /* masteridx + otheridx<<16 */
    struct Square *weaknext;        /* list of squares involving otheridx */
    BitmapChunk bitmap[BITMAPSIZE]; /* contents of this square */
} Square;

#define masteridx(blockid) ((BlockNo)((blockid) & 0xffff))
#define otheridx(blockid)  ((BlockNo)((blockid) >> 16))

typedef struct SquareLists {
    Square *list;     /* list of squares owned by this idx */
    Square *weaklist; /* list of other squares involving this idx */
} SquareLists;


static VoidStar allocate(AllocType type, int32 size)
{
    return (type == AT_Syn)  ? SynAlloc(size) :
           (type == AT_Bind) ? BindAlloc(size) :
                               GlobAlloc(SU_Other, size);
}

extern bool relation_member(int32 a, int32 b, Relation matrix)
{   SquareLists *master;
    Square      *p, *prev;
    BlockNo     other;

    if (a < b) { int32 t = a; a = b; b = t; }
    master = &(matrix[blockno(a)]);
    other = blockno(b);
    for (prev = NULL, p = master->list; p != NULL; prev = p, p = p->next) {
        if ((p->blockid >> 16) == other) { /* eureka! */
            if (prev != NULL) { /* move to front */
                prev->next = p->next; p->next = master->list; master->list = p;
            }
            {   unsigned32  bitno = (unsigned32)bitidx(lowbits(a), lowbits(b));
                return ((bitmapchunk(p->bitmap, bitno) & bitmapbit(bitno)) != 0);
            }
        }
    }
    return NO;
}

extern bool relation_add(int32 a, int32 b, Relation matrix,
                         RelationAllocRec *allocrec)
{   SquareLists *master;
    Square      *p, *prev;
    BlockNo     other;

    if (a < b) { int32 t = a; a = b; b = t; }
    master = &(matrix[blockno(a)]);
    other = blockno(b);
    for (prev = NULL, p = master->list; p != NULL; prev = p, p = p->next) {
        if ((p->blockid >> 16) == other) { /* eureka! */
            if (prev != NULL) { /* move to front */
                prev->next = p->next; p->next = master->list; master->list = p;
            }
            break;
        }
    }
    if (p == NULL) {
        /* need a new square */
        (*allocrec->statloc)++;
        *allocrec->statbytes += sizeof(Square);
        p = (Square *) allocate(allocrec->alloctype, sizeof(Square));
        p->next     = master->list;
        p->blockid  = ((int32)blockno(a)) | (((int32)other) << 16);
        p->weaknext = matrix[other].weaklist;
        memclr(p->bitmap, sizeof(p->bitmap));
        master->list = p;
        matrix[other].weaklist = p;
    }
    {   unsigned32  bitno = (unsigned32)bitidx(lowbits(a), lowbits(b));
        BitmapChunk chunk = bitmapchunk(p->bitmap, bitno);
        BitmapChunk bit   = bitmapbit(bitno);
        if ((chunk & bit) == 0) { /* a new clash! */
            bitmapchunk(p->bitmap, bitno) = (chunk | bit);
            return 1;
        }
    }
    return 0;
}

extern bool relation_delete(int32 a, int32 b, Relation matrix)
{
    SquareLists *master;
    Square      *p, *prev;
    BlockNo     other;

    if (a < b) { int32 t = a; a = b; b = t; }
    master = &(matrix[blockno(a)]);
    other = blockno(b);
    for (prev = NULL, p = master->list; p != NULL; prev = p, p = p->next) {
        if ((p->blockid >> 16) == other) { /* eureka! */
            if (prev != NULL) { /* move to front */
                prev->next = p->next; p->next = master->list; master->list = p;
            }
            break;
        }
    }
    if (p == NULL) {
        unsigned32  bitno = (unsigned32)bitidx(lowbits(a), lowbits(b));
        BitmapChunk chunk = bitmapchunk(p->bitmap, bitno);
        BitmapChunk bit   = bitmapbit(bitno);
        if ((chunk & bit) != 0) {
            bitmapchunk(p->bitmap, bitno) = chunk & ~bit;
            return 1;
        }
    }
    return 0;
}

extern void relation_map(int32 a, Relation matrix, RProc *dothis, VoidStar arg)
{
    BlockNo      block;
    Square       *p;
    int32        j;

    for (p = matrix[blockno(a)].list; p != NULL; p = p->next) {
        block = otheridx(p->blockid) << BLOCKBITS;
        for (j = 0; j < BLOCKSIZE; ++j) {
            int32       bitno = bitidx(lowbits(a), j);
            BitmapChunk chunk = bitmapchunk(p->bitmap, bitno);
            if (chunk != 0) {
                if (chunk & bitmapbit(bitno)) {
                    int32 n = block | j;
                    dothis(n, arg);
                }
            }
        }
    }
    for (p = matrix[blockno(a)].weaklist; p != NULL; p = p->weaknext) {
        block = masteridx(p->blockid) << BLOCKBITS;
        for (j = 0; j < BLOCKSIZE; ++j) {
            int32       bitno = bitidx(j, lowbits(a));
            BitmapChunk chunk = bitmapchunk(p->bitmap, bitno);
            if (chunk != 0) {
                if (chunk & bitmapbit(bitno)) {
                    int32 n = block | j;
                    dothis(n, arg);
                }
            }
        }
    }
}

extern void relation_map1(int32 a, Relation matrix, RProc2 *dothis)
{
    relation_map(a, matrix, (RProc *)dothis, (VoidStar)NULL);
}

extern void relation_mapanddelete(int32 a, Relation matrix, RProc *dothis, VoidStar arg)
{
    BlockNo      block;
    Square       *p;
    int32        j;

    for (p = matrix[blockno(a)].list; p != NULL; p = p->next) {
        block = otheridx(p->blockid) << BLOCKBITS;
        for (j = 0; j < BLOCKSIZE; ++j) {
            int32       bitno = bitidx(lowbits(a), j);
            BitmapChunk chunk = bitmapchunk(p->bitmap, bitno);
            if (chunk != 0) {
                if (chunk & bitmapbit(bitno)) {
                    int32 n = block | j;
                    bitmapchunk(p->bitmap, bitno) &= ~bitmapbit(bitno);
                    dothis(n, arg);
                }
            }
        }
    }
    for (p = matrix[blockno(a)].weaklist; p != NULL; p = p->weaknext) {
        block = masteridx(p->blockid) << BLOCKBITS;
        for (j = 0; j < BLOCKSIZE; ++j) {
            int32       bitno = bitidx(j, lowbits(a));
            BitmapChunk chunk = bitmapchunk(p->bitmap, bitno);
            if (chunk != 0) {
                if (chunk & bitmapbit(bitno)) {
                    int32 n = block | j;
                    bitmapchunk(p->bitmap, bitno) &= ~bitmapbit(bitno);
                    dothis(n, arg);
                }
            }
        }
    }
}

extern Relation relation_init(RelationAllocRec *allocrec, int32 size, unsigned32 *statloc)
{
    SquareLists *p;
    int32  vecsize = ((size + BLOCKSIZE-1) / BLOCKSIZE) * sizeof(p[0]);
    p = (SquareLists *)allocate(allocrec->alloctype, vecsize);
    memclr((VoidStar)p, (size_t) vecsize);
    *statloc += vecsize;
    return p;
}

/* end of mip/regsets.c */
