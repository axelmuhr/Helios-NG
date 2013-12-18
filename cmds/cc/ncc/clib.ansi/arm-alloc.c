/*
  Title:        alloc - Storage management (dynamic allocation/deallocation)
  $Revision: 1.4 $  LDS 31-Jul-87 $ BJK 23-Oct-1987
  $Revision: 1.4 $  LH 22-Dec-1987
  $Revision: 1.4 $  LH 03-Feb-1988
  Modified by John Fitch Oct 1990

  Copyright (C) Acorn Computers Ltd., 1988
*/

/* ***** IMPORTANT ** IMPORTANT ** IMPORTANT ** IMPORTANT ** IMPORTANT *****
 * The #defines which control a large part of this source file are decribed
 * in the header file.
 */

/*
 * NOTES:
 *  Non-implemented (possible) functionality is described under ASSUMPTIONS
 *   and marked with a '!'.
 *  Heap extensions inside the current heap (in a previous heap hole) has not
 *   been tested, but the code is there.
 *  A certain percentage (FRACTION_OF_HEAP_NEEDED_FREE) of the heap is always
 *   kept free, this is a bit wasteful but the number of coalesces and garbage
 *   collections goes down as this percentage rises. It has been found by
 *   experimentation that this fraction should be approximately between 1/8 and
 *   1/4 (currently at 1/6). Large blocks are allocated from the start of the
 *   overflow list ie the low memory addresses and small and medium sized
 *   blocks are allocated from the end of the overflow list. For this reason
 *   the overflow list is a doubly linked list with a head at both ends. A
 *   pointer to the last free block on the heap is also kept so that when the
 *   heap is extended and the old bitmap is returned to the free list (and
 *   merged with any adjacent free block), the last heap block, if it is free,
 *   can be merged with it also.
 * ASSUMPTIONS:
 *  Address units are in bytes.
 *  There are an exact number of address units (bytes) per word.
 *  All target machines are either word aligned or run slower with non word
 *   alignment (so word aligning is a good and right thing to do).
 *  The heap can not grow downwards (all heap extensions must be above the
 *   heap base determined by the first block claimed from OSStorage), and if
 *   two consecutive (in time) blocks are doled out by OSStorage they are only
 *   assumed to be contiguous if the lower limit (arithmetically) of the second
 *   block is equal to the higher limit (arithmetically) of the first block
 *   plus one.
 *  Blocks may be doled out in unspecified address order (but note that
 *   every time a heap extension, which is inside my heap bounds, is given out
 *   the heap has to be scanned in order to find and modify the heap hole in
 *   which the extension has been given.
!*  The range of address units to be found in a single bin can only be the
 *   number of address units in a word, extra code will have to be written to
 *   manage bin ranges other than this size (more trouble than its worth, if
 *   its worth anything at all).
 *  MAXCARD is the largest number representable in a word (ie all bits set).
 * ALLOCATE:
 *  An array of lists of free blocks of similar sizes (bins) is kept so that
 *   when an ALLOCATE of size n is requested the list starting at array entry
 *   n DIV BINRANGE will automatically have as the first element of the list
 *   a block of the correct size (plus the OVERHEADWORDS) or no block at all
 *   (or the block requested may be too big to be in the allocate bins). If
 *   there is no block available in the bin, then bins containing lists of
 *   larger blocks are checked and the block allocated from one of these (if
 *   the bin block is big enough, it is split). If there is still no block
 *   available then the overflow list is checked and if available, the block
 *   is cut from here (the block required is cut from the end of the larger
 *   block if the size required is not large (size < LARGEBLOCK) otherwise it
 *   is taken from the start of the large block). If the remainder of the block
 *   is greater than the largest bin block then it remains in the overflow
 *   list, otherwise it is removed to the correct bin. If the overflow list
 *   does not have a block large enough then the heap is either extended (more
 *   memory claimed from OSStorage), coalesced or garbage collected, depending
 *   on the state of the heap etc and whether garbage collection is enabled.
 *   After coalescing or garbage collection the allocate algorithm is executed
 *   again in order to allocate the block.
 * COALESCE:
 *  if the overflow list does not contain a block large enough and a
 *   reasonable amount of storage has been deallocated since the last coalesce,
 *   (reasonable is difficult to define and is only deducable by
 *   experimentation) then all allocatable blocks (by storage) and all blocks
 *   on the overflow deallocate list are marked free, the heap is scanned and
 *   the blocks scattered into bins and overflow list in increasing address
 *   order.
 * DEALLOCATE:
 *  When a block is DEALLOCATED, if it will fit in a bin then it is put at
 *   the start of the relevant bin list otherwise it is conceptually released
 *   to the overflow deallocate list (there is no need for a list, set the
 *   block's header bits to indicate it is free and it will automatically be
 *   sucked in at the next coalesce).
 * HEAP EXTENSIONS:
 *  Whenever the heap is extended, a certain amount (if available) is allocated
 *   for the garbage collection bit maps (even if garbage collection has not
 *   been enabled.
 */

#include "arm-alloc.h"
#include <stddef.h>
#include "hostsys.h"
#include <string.h>             /* for memset(...), memcpy(...) */

#include <stdio.h>

#define IGNORE(param) param = param
static int n, d, xxx;
#define FD(i, b) {n = i; d = 1; \
        while (n >= b) {d *= b; n /= b;} n = i; \
	while(d) {if ((n/d) > 9) { xxx = n/d+'a'-19; \
			           _syscall3(SYS_write,0,(int)&xxx,1);} \
                  else { xxx = n/d+'0'; _syscall3(SYS_write,0,(int)&xxx,1);} \
                  n = n-(n/d)*d; d /= b;}}

#define FALSE 0
#define TRUE  1

#define BITSIZE(bytes) ((bytes)<<3)
#define BITSPERWORD  BITSIZE(sizeof(int))
#define BITSPERBYTE  (BITSPERWORD/BYTESPERWORD)
/*
 * The following constants are all in address units
 */
/* MAXBYTES should be something outrageously big */
#define MAXBYTES     0x01000000
#define OVERHEAD     (FIRSTUSERWORD * BYTESPERWORD)
#define HOLEOVERHEAD OVERHEAD
#define MINBLOCKSIZE (OVERHEAD + BYTESPERWORD)
#define HOLEBITS     (DATA | HEAPHOLEBIT)

/* the following constants are tunable */
/* multiple of required block size needing to be free before coalesce done */
#define BINRANGE     (BYTESPERWORD * 1) /* see assumptions */
#define NBINS        16
#define MAXBINSIZE   (BINRANGE*(NBINS)-1)
#define LARGEBLOCK   512
/*
 * FRACTION_OF_HEAP_NEEDED_FREE is used when deciding whether to coalesce, GC
 * or extend the heap. An attempt is made to keep this amount free, if it is
 * not free then the heap is extended. The amount of free space is the total of
 * all free blocks (without overheads). If there is a bitmap at the end of the
 * heap, it is not included in the heap size.
 */
#define FRACTION_OF_HEAP_NEEDED_FREE 6
/* initialisation for blocks on allocation */

typedef void *VoidStar;

static BlockP heapLow;  /* address of the base of the heap */
static BlockP heapHigh; /* address of heap hole guard at the top of heap */
static BlockP sys_heap_top; /* address of top of system heap, should = heapLow
                               after _init_user_alloc is called          */
/*
 * amount of heap that user can actually write to, does not include bitmaps
 * and block overheads
 */
static size_t totalFree;
static size_t userHeap;  /* size of heap (bytes) excluding gc bitmaps */
static size_t totalHeap; /* size of heap (bytes) including gc bitmaps */
/*
 * The overflow list is a chain of large blocks ready for use, the chain is a
 * doubly linked list of blocks in increasing address order.
 * bin[0] is the start of the overflow list.
 * bin[NBINS+1] is end of the overflow list.
 *
 * bin is an array of pointers to lists of free small blocks ( <= MAXBINSIZE)
 * of the same size. Last deallocated block is at the start of the list.
 */
static BlockP bin[NBINS+2];

static BlockP endOfLastExtension;

static int checkDeallocates;
static int checkAllocates;

static int lookInBins;

static BlockP lastFreeBlockOnHeap;

static char sys_message[60];

/*
 * Code macros.
 */
#define SIZE(block) ((size_t)((block)->size & SIZEMASK))
#define BITSTOWORDS(bits) ((bits+(BITSPERWORD-1))/BITSPERWORD)
#define BYTESTOWORDS(bytes) ((bytes+(BYTESPERWORD-1))/BYTESPERWORD)
#define ADDBYTES(bp, bytes) (BlockP)((char *)bp + (bytes))
#define ADDBYTESTO(bp, bytes) bp = (BlockP)((char *)bp + (bytes))
#define PTRDIFF(hi, lo) ((char *)hi - (char *)lo)
#define FREE(block) (FREEBIT & ((BlockP)block)->size)
#define HEAPHOLE(block) (HEAPHOLEBIT & block->size)
#ifdef BLOCKS_GUARDED
#define INVALID(block) (((BlockP)block)->guard != GUARDCONSTANT)
#else
#define INVALID(block) (0)
#endif
#define BADUSERBLOCK(block) (INVALID(ADDBYTES(block,-OVERHEAD)) \
                            || FREE(ADDBYTES(block,-OVERHEAD)))


/*
 * This code will use a maximum of 32 words of stack excluding any used by
 * the system storage wholesaler.
 *
 * Turn off stack overflow checking.
 */
#pragma -s1

extern void* _sbrk(int);

static void _alloc_die(message, rc)
char *message;
int rc;
{
  /* nb rc is here so that it can be examined - otherwise the C compiler
   * tends to lose a useful value.
   */
  IGNORE(rc);
  strcpy(sys_message, message);
  if (rc == CORRUPT) strcat(sys_message, ", (heap corrupt)");
  _sysdie(sys_message);
}

static void bad_size(size)
size_t size;
{
  IGNORE(size);
  _alloc_die("Over-large or -ve size request", FAILED);
}


#ifdef BLOCKS_GUARDED
extern void __heap_checking_on_all_deallocates(on)
int on;
{
  checkDeallocates = on;
}

extern void __heap_checking_on_all_allocates(on)
int on;
{
  checkAllocates = on;
}
#endif

static int internal_coalesce(void)
{ BlockP block;
  BlockP previous;
  BlockP tail;
#ifndef BLOCKS_GUARDED
  BlockP bin_copy[NBINS+2];
#endif
  size_t size;
  /* where size is used to specify an element of an array it should really be
   * called index, but to generate better code I got rid of the index variable
   */

  lookInBins = FALSE;
  totalFree = 0;
  /* set bins and overflow lists to empty */
  for (size = 0; size <= NBINS+1; size++)
  { bin[size] = NULL;
#ifndef BLOCKS_GUARDED
    bin_copy[size] = NULL;
#endif
  }

  block = heapLow;

  /* NULL indicates previous doesn't point to start of free block */
  previous = NULL; tail = NULL;

  while (block <= heapHigh) {
    if (INVALID(block)) return CORRUPT;
    if (FREE(block)) { /* free block */
      if (previous == NULL) previous = block;
    } else if (previous != NULL) {
      size = PTRDIFF(block, previous) - OVERHEAD;
      /* set flags to GCAble, Free, and not PureData */
      totalFree += size;
      previous->size = (size | FREEBIT);
      if (size <= MAXBINSIZE) { /* return to bin */
        size /= BINRANGE;
        if (bin[size] == NULL) bin[size] = previous;
        else {
          /* if not BLOCKS_GUARDED use guard word of first block in bin to hold
           * a pointer to the last block in the list for this bin otherwise
           * use the bin_copy array. This allows me to keep the list in
           * ascending address order. Remember to put back the guard words at
           * the end of coalescing if BLOCKS_GUARDED.
           */
#ifdef BLOCKS_GUARDED
          ((BlockP) bin[size]->guard)->next = previous;
#else
          (bin_copy[size])->next = previous;
#endif
        }
#ifdef BLOCKS_GUARDED
        bin[size]->guard = (int) previous;
#else
        bin_copy[size] = previous;
#endif
      } else { /* put block on overflow list */
        if (bin[0] == NULL)
          {bin[0] = previous; previous->previous = NULL;}
        else
          {tail->next = previous; previous->previous = tail;}
        tail = previous;
      }
      previous = NULL;
    }
    ADDBYTESTO(block, SIZE(block) + OVERHEAD);
  }

  /* replace the guard words at the start of the bins lists */
  for (size = 1; size <= NBINS; size++) {
    if (bin[size] != NULL) {
      lookInBins = TRUE;
#ifdef BLOCKS_GUARDED
      ((BlockP) bin[size]->guard)->next = NULL;
      bin[size]->guard = GUARDCONSTANT;
#else
      (bin_copy[size])->next = NULL;
#endif
    }
  }

  /* do both ends of overflow list */
  if (bin[0] != NULL) {
    tail->next = NULL;
    bin[NBINS+1] = tail;
  } else { bin[NBINS+1] = NULL; }
  lastFreeBlockOnHeap = bin[NBINS+1];

  return OK;
}

static int InsertBlockInOverflowList(block)
BlockP block;
{
  /* OK to add remainder of block to tail of overflow list */
  if (bin[0] == NULL) {bin[0] = block; block->previous = NULL;}
  else {bin[NBINS+1]->next = block; block->previous = bin[NBINS+1];}
  bin[NBINS+1] = block; block->next = NULL;
  return OK;
}

/* Get heap from Operating system. We need at least minSize bytes.  Address
 * of the space found is placed in base_ptr and the atual size in size_ptr
 */
static int GetMoreOSHeap(size_t minSize, BlockP *base_ptr, size_t *size_ptr)
{
  size_t size = *size_ptr;
  BlockP base = *base_ptr;
  BlockP bitmap;
  int gotWhatWasWanted;

  minSize += OVERHEAD + HOLEOVERHEAD;
  if (userHeap/FRACTION_OF_HEAP_NEEDED_FREE > totalFree)
    minSize += userHeap / FRACTION_OF_HEAP_NEEDED_FREE - totalFree;

  base = _sbrk(minSize);
  _syscall3(SYS_write,0,(int)"Did a break for  ",16); FD(minSize,10)
  _syscall3(SYS_write,0,(int)"\nGot ",5); FD((unsigned)base,16)
  _syscall3(SYS_write,0,(int)"\n",1);
  if (base) {
    size = minSize;
    gotWhatWasWanted = TRUE;
    _syscall3(SYS_write,0,(int)"Worked\n",7);
  }
  else size = 0, gotWhatWasWanted = FALSE;
  if (size <= HOLEOVERHEAD) {size = 0; base = NULL;}
  else size -= HOLEOVERHEAD;

  bitmap = base;
  if (base == endOfLastExtension) {
    _syscall3(SYS_write,0,(int)"Contiguous\n",11);
    /* extension contiguous with last block on heap. */
    if (lastFreeBlockOnHeap != NULL &&
            ADDBYTES(lastFreeBlockOnHeap,
                          SIZE(lastFreeBlockOnHeap)+OVERHEAD) == bitmap) {
      /* so do the merge of the extension and last block on the heap */
      lastFreeBlockOnHeap->size = SIZE(lastFreeBlockOnHeap) + OVERHEAD;
      totalFree -= lastFreeBlockOnHeap->size;
      size += lastFreeBlockOnHeap->size;

      bitmap = lastFreeBlockOnHeap;
      if (lastFreeBlockOnHeap == bin[NBINS+1]) {
        /* remove block from end of overflow list */
        if (lastFreeBlockOnHeap->previous == NULL) bin[0] = NULL;
        else lastFreeBlockOnHeap->previous->next = NULL;
        bin[NBINS+1] = lastFreeBlockOnHeap->previous;
      } /* else it is not in any list ie waiting for coalesce */
    }
  }

  /* SEE WHAT TO DO WITH NEW BLOCK (IF THERE IS ONE) */
  if (size > MAXBINSIZE+OVERHEAD) {
    /* block is big enough to do something with */
    /* HANDLE BEING DROPPED INTO A HEAP HOLE, AND CREATING THE HEAP HOLE
       MARKER AT THE END OF THE NEW EXTENSION BLOCK. */
    if (base >= heapHigh) {
      if (endOfLastExtension != NULL && base != endOfLastExtension) {
        /* heap hole, mark it as allocated */
        endOfLastExtension->size =
                 (PTRDIFF(base, endOfLastExtension) - HOLEOVERHEAD) | HOLEBITS;
      }
      endOfLastExtension = ADDBYTES(bitmap, size);
#ifdef BLOCKS_GUARDED
      endOfLastExtension->guard = GUARDCONSTANT;
#endif
      endOfLastExtension->size = 0; /* as an end marker for Coalesce */
    }

    /* INITIALISE HEADER OF NEW BLOCK */
    base = bitmap;
    if (base > lastFreeBlockOnHeap) lastFreeBlockOnHeap = base;
    size -= OVERHEAD;
#ifdef BLOCKS_GUARDED
    base->guard = GUARDCONSTANT;
#endif
    /* set flags to GCAble, Free, and not PureData */
    base->size = size | FREEBIT;
    totalFree += size;
    if (!gotWhatWasWanted) {
      if (InsertBlockInOverflowList(base) != OK) return FAILED;
    }
  } else /* block is not big enough to worry about, throw it away */

  /* endOfLastExtension is the address of the storage after the end of the
     block (used to handle heap holes) */
  if (endOfLastExtension > heapHigh) heapHigh = endOfLastExtension;
  if (base < heapLow && base != NULL) heapLow = base;
  totalHeap = PTRDIFF(heapHigh, heapLow);
  userHeap = totalHeap;

  *size_ptr = size;
  *base_ptr = base;
  if (gotWhatWasWanted) return OK; else return FAILED;
}

#ifdef BLOCKS_GUARDED
static int check_heap(void)
{ BlockP block;
  if (userHeap > 0) {
    for (block = heapLow; ; ) {
      if (block >= heapHigh) {
        if (block > ADDBYTES(heapHigh,OVERHEAD)) return CORRUPT;
        else return OK;
      }
      if (INVALID(block)) return CORRUPT;
      ADDBYTESTO(block, SIZE(block)+OVERHEAD);
    }
  }
  return OK;
}
#endif

#define COALESCED     (1<<31)
#define DONEGC        (1<<30)
#define FORCECOALESCE (1<<29)

static int primitive_alloc(gcBits, size/*words*/)
int gcBits;
size_t size;
{ BlockP block;
  size_t actualSize;
  register int index;
  int fromHighMemory;
  int status = 0;

#ifdef BLOCKS_GUARDED
  if (checkAllocates && check_heap() != OK) return(CORRUPT);
#endif
  /* convert size from words to addresss units */
  size *= BYTESPERWORD;
  if (size >= MAXBYTES) return(FAILED);
  else if (size == 0) return(NULL);

  index = 0;
  fromHighMemory = ((size <= LARGEBLOCK) && sys_heap_top);
  for (;;) {
    if (size <= MAXBINSIZE && lookInBins) { /* get from bin (if not empty) */
      index = size / BINRANGE;
      do {
        block = bin[index];
        if (block != NULL) { /* got a block */
          if (INVALID(block)) return(CORRUPT);
          bin[index] = block->next;
          actualSize = SIZE(block);
          goto got_block;
        } /* else try other bins */
      } while (++index <= NBINS);
    }

    /* block bigger than largest bin / bin is empty, check overflow list */
    /* if large block required, take it from high memory otherwise from low */
get_from_overflow:
    if (fromHighMemory) {block = bin[NBINS+1];}
    else {block = bin[0]; }

    while (block != NULL) {
      if (INVALID(block)) return(CORRUPT);
      actualSize = SIZE(block);
      if (actualSize >= size) {
        /* got a block big enough, now see if it needs splitting */
        if (actualSize-size <= MAXBINSIZE+OVERHEAD) {
          /* remove all of block from overflow list */
          if (block == lastFreeBlockOnHeap) lastFreeBlockOnHeap = NULL;
          if (block->previous == NULL) bin[0] = block->next;
          else block->previous->next = block->next;
          if (block->next == NULL) bin[NBINS+1] = block->previous;
          else block->next->previous = block->previous;
          goto got_block;
        } else { /* split and leave unwanted part of the block in list */
          goto split_block;
        }
      } else {
          if (fromHighMemory) {block = block->previous;}
          else {block = block->next; }
      }
    }

    /* no block in bin or overflow list, try coalesce if desirable */
    if (!(COALESCED & status) &&
         ((totalFree > size<<2 &&
           totalFree > userHeap/FRACTION_OF_HEAP_NEEDED_FREE)
         || FORCECOALESCE & status)) {
      if (internal_coalesce() != OK) return(CORRUPT);
      status |= COALESCED;
      continue; /* try the allocation again */
    } else 
      /* no block available in Storage, must go to OSStorage to get one */
    { BlockP blockCopy;
      size_t actual;
      /* now we have to get more heap */
      switch (GetMoreOSHeap(size, &blockCopy, &actual)) {
        case OK:
          block = blockCopy; actualSize = actual;
          if (InsertBlockInOverflowList(block) != OK) return(CORRUPT);
          goto get_from_overflow;
        case FAILED:
          block = blockCopy; actualSize = actual;
          break;
        case CORRUPT:
          return CORRUPT;
        default: _alloc_die("internal error: bad switch selector", FAILED);
      }
    }
  }

got_block:
  if (fromHighMemory && (actualSize > size+MINBLOCKSIZE)) {
    /* split and put unwanted part of block into a bin or on overflow list*/
split_block:
    { BlockP tempBlock = block;
      totalFree -= OVERHEAD;
      /* large block taken from bottom of this block */
      /* medium and small blocks (and bitmaps) taken off top of this block */
      if ((size > LARGEBLOCK) || (!sys_heap_top)) ADDBYTESTO(tempBlock, size+OVERHEAD);
      else ADDBYTESTO(block, actualSize-size);
      block->size = size;
      /* set flags on block to GCAble, Free, and not PureData */
      size = actualSize - (size + OVERHEAD);
      tempBlock->size = size | FREEBIT;

      if (!fromHighMemory) {
      /* The block has been cut from the start of the overflow block.
         This means that the large block that was in the overflow list
         has to be replaced with new one (tempBlock).
       */
        tempBlock->previous = block->previous;
        tempBlock->next = block->next;
        if (tempBlock->previous == NULL) bin[0] = tempBlock;
        else tempBlock->previous->next = tempBlock;
        if (tempBlock->next == NULL) bin[NBINS+1] = tempBlock;
        else tempBlock->next->previous = tempBlock;
      }
#ifdef BLOCKS_GUARDED
      tempBlock->guard = GUARDCONSTANT;
#endif

      if (size <= MAXBINSIZE) {
        /* work out the bin number */
        lookInBins = TRUE;
        index = size / BINRANGE;
        tempBlock->next = bin[index]; bin[index] = tempBlock;
      }
    }
  }  /* no split, take the whole block */

  size = SIZE(block);
  /* set flags to not Free, and gcbits */
  block->size = size | (gcBits & DATA);
#ifdef BLOCKS_GUARDED
  block->guard = GUARDCONSTANT;
#endif
  totalFree -= size;
  if (bin[NBINS+1] > lastFreeBlockOnHeap) lastFreeBlockOnHeap = bin[NBINS+1];
  ADDBYTESTO(block, OVERHEAD);
  return((int)block);
}

static int primitive_dealloc(block)
BlockP block;
{ int size;
  
  if ((block <= heapLow) || (block >= heapHigh)) {
    if (block == NULL) return(OK);
    else return(FAILED);
  }
  ADDBYTESTO(block, -OVERHEAD);

#ifdef BLOCKS_GUARDED
  if (checkDeallocates) {
    BlockP searchBlock = heapLow;
    for (; searchBlock != block; ) {
      if (searchBlock >= heapHigh) return(FAILED);
      if (INVALID(searchBlock)) return(CORRUPT);
      ADDBYTESTO(searchBlock, OVERHEAD + SIZE(searchBlock));
    }
  }

  if (INVALID(block)) return(CORRUPT);
#endif
  size = block->size;
  if (FREEBIT & size) return(FAILED);
  /* set flags to GCAble, Free, and not PureData */
  size &= SIZEMASK;
  block->size = size | FREEBIT;
  totalFree += size;

  if (size <= MAXBINSIZE) { /* return to bin */
    lookInBins = TRUE; size /= BINRANGE;
    block->next = bin[size]; bin[size] = block;
  } else {
    /* put block on deallocate overflow list, for reuse after coalesce */
    if (block > lastFreeBlockOnHeap) lastFreeBlockOnHeap = block;
  }

  return(OK);
}

/*
 * Put the veneer functions here for now: don't really need all these.
 */
extern size_t _byte_size(p)
VoidStar p;
{ BlockP block = (BlockP)p;
  if (block != NULL) {
    /* decrement the pointer (block) by the number of overhead bytes */
    ADDBYTESTO(block, -OVERHEAD);
    if (!INVALID(block)) return (SIZE(block));
  }
  return 0;
}

extern VoidStar malloc(size)
size_t size;
{ VoidStar ptr;

  ptr = (VoidStar) primitive_alloc(NOTGCABLEBIT, BYTESTOWORDS(size));
  if ((int)ptr < OK) {
    if ((int)ptr == CORRUPT) _alloc_die("malloc failed", CORRUPT);
    else return NULL;
  }
  return ptr;
}

extern VoidStar realloc(p, size)
VoidStar p;
size_t size;
{ int rc;
  size_t old;
  VoidStar new = NULL;

  size = BYTESTOWORDS(size)*BYTESPERWORD;
  if (p == NULL) return malloc(size);
  if (BADUSERBLOCK(p)) _alloc_die("realloc failed, (bad user block)", FAILED);

  old = _byte_size(p);
  if (old < size) {
    new = malloc(size);
    if (new == NULL) return NULL;
    memcpy(new, p, old);        /* copies 0 words for bad p! */
  }
  if ((old < size) || (size == 0) || (old > size+MINBLOCKSIZE+BYTESPERWORD)) {
    if ((old > size+MINBLOCKSIZE+BYTESPERWORD) && (size != 0)) {
      BlockP b = ADDBYTES(p, -OVERHEAD);
      b->size = size+BYTESPERWORD | (b->size&(!SIZEMASK));
      new = p;
      ADDBYTESTO(b, size+BYTESPERWORD+OVERHEAD);
#ifdef BLOCKS_GUARDED
      b->guard = GUARDCONSTANT;
#endif
      b->size = (old-OVERHEAD-BYTESPERWORD-size) | DATA;
      p = ADDBYTES(b, OVERHEAD);
    }
    rc = primitive_dealloc((BlockP) p);
    if (rc != OK) {
      _alloc_die("deallocate of old block in realloc failed", rc);
    }
    return new;
  } else
    return p;
}

extern VoidStar calloc(count, size)
size_t count;
size_t size;
{ VoidStar r;
/*
 * This miserable code computes a full 64-bit product for count & size
 * just so that it can verify that the said product really is in range
 * for handing to malloc.
 */
  unsigned h = (count>>16)*(size>>16);
  unsigned m1 = (count>>16)*(size&0xffff);
  unsigned m2 = (count&0xffff)*(size>>16);
  unsigned l = (count&0xffff)*(size&0xffff);
  h += (m1>>16) + (m2>>16);
  m1 = (m1&0xffff) + (m2&0xffff) + (l>>16);
  l = (l&0xffff) | (m1<<16);
  h += m1>>16;
  if (h) l = (unsigned)(-1);
  if (l >= MAXBYTES) bad_size(l);
  r = malloc(l);
  if (r != NULL) memset(r, 0, l);
  return r;
}

extern void free(p)
VoidStar p;
{ int rc;
  rc = primitive_dealloc((BlockP)p);
  /* following line may not be correct ANSI - but for the moment we
   * have problems if we don't detect invalid free's.
   */
  if (rc != OK) {
    _alloc_die("free failed", rc);
  }
}

extern VoidStar _sys_alloc(n)
size_t n;
{ VoidStar a = malloc(n);
  if (a == NULL)
    _alloc_die("No store left for I/O buffer or the like", FAILED);
  return a;
}


/*
 * End of veneer functions
 *
 * Garbage collection interface.
 */

extern int __coalesce(void)
{ int rc;
  rc = internal_coalesce();
  if (rc != OK) {
    _alloc_die("_coalesce failed", rc);
  }
  return rc;
}

extern void _terminate_user_alloc(void)
{
  heapLow = sys_heap_top;
}

extern void _init_user_alloc(void)
{
  sys_heap_top = heapLow;
  heapLow = bin[0];
  totalHeap = PTRDIFF(heapHigh, heapLow);
  userHeap = totalHeap;
}

extern void _init_alloc(void)
{ int j;
  lastFreeBlockOnHeap = NULL;
  checkDeallocates = FALSE;
  checkAllocates = FALSE;
  /* to get rid of warnings */
  lookInBins = FALSE;
  totalFree = 0;
  endOfLastExtension = NULL;
  /* set allocate bins to empty */
  for (j=0; j <= NBINS; ++j) { bin[j] = NULL; }
  /* set overflow lists to empty */
  bin[0] = NULL; bin[NBINS+1] = NULL;
  totalHeap = 0; userHeap = 0;
  sys_heap_top = heapHigh = 0; heapLow = (BlockP) 0x7fffffff;
}
