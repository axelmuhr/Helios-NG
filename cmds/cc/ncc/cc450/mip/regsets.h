#pragma force_top_level
#pragma include_only_once
/*
 * regsets.h, version 1a
 * Copyright (C) Acorn Computers Ltd., 1988.
 */

/*
 * RCS $Revision: 1.1 $
 * Checkin $Date: 1993/07/14 14:07:18 $
 * Revising $Author: nickc $
 */

#ifndef _regsets_h
#define _regsets_h 1

#ifndef _defs_LOADED
#  include "defs.h"
#endif
#ifndef _cgdefs_LOADED
#  include "cgdefs.h"
#endif

/* Some operations on RegLists
 * (RegLists are defined in cgdefs.h, and are simply Lists for which the data
 *  type is a VRegnum.  These are really generic list operations, but it happens
 *  that we want to do them only on RegLists)
 */

extern RegList *rldiscard(RegList *x);
/* Discard the head entry of the argument reglist, result is the tail
 */

extern bool member(VRegnum a, RegList *l);

extern RegList *ndelete(VRegnum a, RegList *ll);
/* Destructively modify the argument reglist by removing from it the first entry
 * whose data field is  a
 */

typedef void RProc2(int32, VoidStar);
typedef void RProc1(int32);

/*
 * vregsets - a space efficient representation for sets of integers which are
 * expected to be too sparse for a straghtforward bitmap, and too dense for
 * a Lisp-style list.
 * The name is historical, from the origins of this representation in regalloc,
 * but it is useful (and used) for sets of other things besides registers.
 *
 * The precise representation of a vregset is not revealed.  However, it is
 * guaranteed that NULL is A valid value for an empty vregset.
 */

typedef enum {
    AT_Glob,
    AT_Bind,
    AT_Syn
} AllocType;

typedef struct {
    AllocType     alloctype;
    unsigned32    *statsloc,
                  *statsloc1,
                  *statsbytes;
} VRegSetAllocRec;
/* A (pointer to a) record of this type gets passed to any vregset procedure
 * which may allocated new structure.
 *    allocproc  should be one of (Syn, Bind, Glob)Alloc
 *    statsloc   is incremented by one for each piece of structure added to the
 *               set
 *    statsloc1  is incremented by one for each freshly allocated piece of
 *               structure (rather than reclaimed from a free chain)
 *    statsbytes is incremented by the size of each freshly allocated piece of
 *               structure.
 */

extern VRegSetP vregset_insert(int32 regno, VRegSetP set, bool *oldp, VRegSetAllocRec *allocrec);
/*
 * Add an element to the set.  *oldp is set TRUE if it was already in the set,
 * FALSE otherwise.
 */

extern bool vregset_member(int32 regno, VRegSetP set);

extern VRegSetP vregset_delete(int32 regno, VRegSetP set, bool *oldp);
/*
 * Remove an element from the set.  *oldp is set FALSE if it was in the set,
 * TRUE otherwise
 */

/*
 * difference, union and intersection destructively modify the set which
 * is their first argument (and return the modified set as result)
 */

extern VRegSetP vregset_difference(VRegSetP s1, VRegSetP s2);

extern VRegSetP vregset_union(VRegSetP s1, VRegSetP s2, VRegSetAllocRec *allocrec);

extern VRegSetP vregset_intersection(VRegSetP s1, VRegSetP s2);

extern VRegSetP vregset_copy(VRegSetP set, VRegSetAllocRec *allocrec);

#define VR_SUBSET   -1
#define VR_EQUAL     0
#define VR_SUPERSET  1
#define VR_UNORDERED 2

extern int vregset_compare(VRegSetP s1, VRegSetP s2);

#define vregset_equal(s1,s2) (vregset_compare(s1,s2)==VR_EQUAL)

extern void vregset_map(VRegSetP set, RProc2 *dothis, VoidStar arg);

/*
 * For each x in  set , call dothis(x, arg).
 * Dothis may not access  set  in any way (not merely not modify it).
 */

extern void vregset_map1(VRegSetP set, RProc1 *dothis);
/* As vregset_map, except that the call is  dothis(x)
 */

extern void vregset_discard(VRegSetP set);

extern void vregset_init(void);

/*
 * Relations - symmetric relations between integers.  The representation is
 * designed for the same conditions as that of vregsets.
 * Only a minimal set of operations are provided (those currently wanted by
 * clients).
 */

typedef struct SquareLists *Relation;
/*
 * The precise representation of a Relation is not revealed.  NULL is not a
 * valid Relation value - a new Relation must be given a value returned by
 * relation_init.
 */

typedef struct {
    AllocType  alloctype;
    unsigned32 *statsloc,
               *statsbytes;
} RelationAllocRec;


extern bool relation_member(int32 a, int32 b, Relation matrix);

extern bool relation_add(int32 a, int32 b, Relation matrix, RelationAllocRec *allocrec);

extern bool relation_delete(int32 a, int32 b, Relation matrix);

extern void relation_map(int32 a, Relation matrix, RProc2 *dothis, VoidStar arg);
/* For all x such that x is related to a, call dothis(x, arg).
 * dothis may not access the Relation in any way.
 */

extern void relation_map1(int32 a, Relation matrix, RProc1 *dothis);
/* As relation_map, except that the call is  dothis(x)
 */


extern void relation_mapanddelete(int32 a, Relation matrix, RProc2 *dothis, VoidStar arg);

extern Relation relation_init(RelationAllocRec *allocrec, int32 size, unsigned32 *statsloc);

#endif

/* end of regsets.h */
