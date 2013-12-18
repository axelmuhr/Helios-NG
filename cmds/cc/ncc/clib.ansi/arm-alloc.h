/*
  Title:        alloc - Storage management (dynamic allocation/deallocation)
  Copyright:    (C) 1987, Acorn Computers Ltd., Cambridge, England.
  $Revision: 1.3 $  LDS, LH 20-Dec-87
*/

#ifndef __alloc__h
#define __alloc__h

#ifndef __stddef__h
#include <stddef.h>
#endif

#define BLOCKS_GUARDED
/*
 * if the OSStorage can not be depended upon to allocate areas of the
 *   heap in consistently increasing address order then the following
 *   constant must be set to FALSE.
 */
#define HEAP_ALLOCATED_IN_ASCENDING_ADDRESS_ORDER 1

typedef struct BlockStruct {
#ifdef BLOCKS_GUARDED
  unsigned int guard; /* guard word should contain GuardConstant if all ok */
#endif
  size_t size; /* block size (not including header) in address units. */
               /* The top 4 bits of size hold the flags declared above */
  struct BlockStruct *next; /* next and previous are used internaly in */
  struct BlockStruct *previous; /* coalescing and managing free blocks. */
                                /* next is the first word of the user block */
} Block, *BlockP;

#define OK       0
#define FAILED  -1
#define CORRUPT -2

#define SIZEMASK      0x0fffffff /* all except top four bits, when applied */
                                 /* to block.size yields no of address units */
                                 /* of user space in this block. */
#define PUREDATABIT   (1<<31) /* indicates the block contains no pointers */
#define NOTGCABLEBIT  (1<<30) /* the block is not to be garbage collected */
#define FREEBIT       (1<<29) /* if set, indicates that the block is free */
#define HEAPHOLEBIT   (1<<28) /* block used for marking start of heap hole */
#define GUARDCONSTANT 0x3E694C3C /* not a legal word pointer */
#define GCABLE        0
#define GCDATA        PUREDATABIT
#define DATA          (NOTGCABLEBIT|PUREDATABIT)
#define BYTESPERWORD  sizeof(int)
/* Block.next offset from Block (in words) */
#define FIRSTUSERWORD ((sizeof(Block)-sizeof(BlockP)*2) / BYTESPERWORD)

extern void _init_alloc(void);
/*
 * Initialise allocate (not to be called by anyone outside the M2Core!)
 */

typedef void FreeProc (BlockP block);
/* block = pointer to start of header of block to be freed */

/*
 * alloc maintains a bitmap to be used by the registered GCProc, this is
 * referred to a bitmapA. If the heap maintained by alloce has a hole in it
 * due to OSStorage not extending the heap contiguously then another bitmap
 * (bitmapB) is used to describe the second part of the heap. There may or may
 * not be a bitmapB, but if heapAEnd < topOfHeap there will be. This does not
 * mean that either of the bitmaps will never represent an area of the heap
 * which contains heap holes.
 */
typedef int (*GCProc) (
  BlockP base,
  BlockP limit, /* of the heap */
  char *mapA, /* one bit per word in [base..heapAEnd-1] */
  char *mapB, /* one bit per word in [heapBStart..limit-1] */
  BlockP heapAEnd, /* base == heapA < heapAEnd */
  BlockP heapBStart, /* if heapAEnd < limit --> heapBStart == heapB < limit */
  FreeProc *freeProc);
  /* returns values OK or Corrupt. */

#ifdef GC
extern void _gcallocate(void **a, size_t bitlen, int gcbits);
/*
 * Support for AEM-2 ALLOCATE.
 */

extern void _set_gcbits(void **a, int gcbits);
/*
 * Function to set the GCAble bit and PureData bit to the given values.
 */

extern int __register_gc_proc(GCProc proc);
/*
 * alloc will (at its discretion) call the registered GarbageCollect proc
 * to search for (and instruct alloc to free) any appropriate user block
 * (depends on the gcBits it was allocated with) which has no pointer it's
 * start. Failed or OK can be returned. Failed is returned if no procedure
 * was previously registered and the heap is almost full (alloc will not be
 * able to allocate an area of the heap to hold the bitmaps required for
 * garbage collection).
 */

extern void *_gc_malloc(int gcbits, size_t bytesize);
/*
 * A variant of malloc which allocates garbage-collectable store.
 * Note that the returned pointer must not be lost or chaos may ensue.
 * 'gcbits' allows blocks to be marked as pure data (which speeds up
 * garbage collection).
 */
#endif

extern size_t _byte_size(void *p);
/*
 * Return an approximation to the allocated size of the object pointed
 * to by p. The only guarantee is that:-
 *   requested_size(p) <= _byte_size(p) <= allocated_size(p).
 */

extern void *malloc(size_t size);
/*
 * Allocate an area of memory of size 'size' and return a pointer to it.
 */

extern void *realloc(void *p, size_t size);
/*
 * Reallocate the block pointed to by p with size 'size' (assumed >
 * _byte_size(p) or this is a no-op). In general, this leads to p
 * being reallocated at a different address.
 */

extern void *_sys_alloc(size_t n);
/*
 * A paranoid variant of malloc used by the C compiler.
 */

extern void *calloc(size_t count, size_t size);
/*
 * A variant of malloc which allocates 'count' zeroed objects of size 'size'.
 */

extern void free(void *p);
/*
 * Deallocate the block pointed to by 'p'.
 */

extern void _deallocate(void **p, size_t bitlen);
/*
 * Deallocate the block pointed to by 'p' raising the AllocateFailed
 * exception on error.  The M2 DEALLOCATE function.  The bitlen value
 * is ignored.
 */

extern int __coalesce(void);
/*
 * Perform a colesce step on the heap. Returns OK or Corrupt.
 */

extern int _alloc_reinit(void);
/*
 * re-initialise the heap returns 1 for success otherwise 0.
 */

/* ---------------------------- Debug aids --------------------------------- */

#ifdef BLOCKS_GUARDED
extern void __heap_checking_on_all_deallocates(int on);
/*
 * If on = TRUE, the structure of the heap is checked on every deallocate
 */

extern void __heap_checking_on_all_allocates(int on);
/*
 * If on = TRUE, the structure of the heap is checked on every allocate
 */
#endif

/* ------------------------- Statistics reporting --------------------------*/
typedef struct StorageInfoStruct {
  BlockP heapLow; /* heap base */
  BlockP heapHigh; /* heap top */
  unsigned int userHeap; /* user part of heap = heapHigh-heapLow-gcbitmaps */
  unsigned int maxHeapRequirement; /* max storage actually requested by user */
  unsigned int currentHeapRequirement;  /* current user requested storage */
  unsigned int coalesces; /* number of coalesces performed, includes number
                             of garbage collections cos a coalesce is done
                             after every garbage collection. */
  unsigned int heapExtensions;    /* number of times that heap is extended */
  unsigned int garbageCollects;   /* number of garbage collections */
  unsigned int blocksAllocated;   /* total number of allocates requested */
  unsigned int blocksDeallocated; /* total number of deallocates requested */
  unsigned int bytesAllocated;    /* total number of bytes allocated */
  unsigned int bytesDeallocated;  /* total number of bytes deallocated */
  unsigned int totalGCBlocks;     /* total number of blocks gc'd */
  unsigned int totalGCBytes;      /* total number of bytes garbage collected */
} StorageInfo, *StorageInfoP;

typedef struct EventInfoStruct {
  int event;
  size_t blockThatCausedEvent; /* size of allocate that caused event */
  unsigned int totalFree;  /* amount of heap that user can actually write to,
                              does not include bitmaps and block overheads */
  unsigned int userHeap;   /* user part of heap = heapHigh-heapLow-gcbitmaps */
  /* the following are changes from previous event */
  unsigned int allocates;        /* number of allocates requested */
  unsigned int bytesAllocated;   /* number of bytes allocated */
  unsigned int deallocates;      /* number of deallocates requested */
  unsigned int bytesDeallocated; /* number of bytes deallocated */
  unsigned int bytesGCd;         /* number of bytes garbage collected */
  unsigned int blocksGCd;        /* number of blocks garbage collected */
} EventInfo, *EventInfoP;

#endif
