#include "hostsys.h"
#include <string.h>

#define DEBUG
#define INITIAL_ALLOCATION	1024 /* in Words */

#define FREEBIT		(1<<24)
#define ACTIVEBIT	(2<<24)
#define END_OF_HUNK	(4<<24)

#define GETBITS(n)	(*(n)&0xff000000)
#define GETSIZE(n)	(*(n)&0x00ffffff)
#define SETFLAGBITS(n,m) { int *_x_x_ = (n); *_x_x_ = GETSIZE(_x_x_) | m;}
#define SETSIZEBITS(n,m) { int *_x_x_ = (n); *_x_x_ = GETBITS(_x_x_) | m;}

#define _monitor(s) _ttywrite(s,sizeof(s),NULL)

extern void* _sbrk(int);
int* heaps;
int* end_of_heap;

void _init_alloc(void)
{
  heaps = _sbrk(sizeof(int)*INITIAL_ALLOCATION + 2*sizeof(size_t));
  *heaps = INITIAL_ALLOCATION | FREEBIT;
  end_of_heap = &(heaps[INITIAL_ALLOCATION+1]);
  *end_of_heap = END_OF_HUNK;
}

void* malloc(int n)
{
  int* next = heaps;
  _monitor("Malloc called\n");
  n = (n+3)>>2;			/* Convert to a number of words */
  if (n<0 || n>4*1024*1024) _sysdie("Too large an allocation\n");
  while (1) {
    if (GETBITS(next) == FREEBIT) {
#ifdef DEBUG
      fprintf(stderr,"First free %p(%x; %p; %x)\n",
	      next, *next, &next[GETSIZE(next)+1], next[GETSIZE(next)+1]);
#endif
      if (GETBITS(&next[GETSIZE(next)+1]) == FREEBIT) {
				/* Amalgamate blocks */
#ifdef DEBUG
	_monitor("Amalgamate blocks\n");
#endif
	SETSIZEBITS(next,GETSIZE(next)+GETSIZE(&next[GETSIZE(next)+1])+1);
      }
      else if (GETSIZE(next) == n) {
				/* Perfect fit */
#ifdef DEBUG
	_monitor("Perfect fit\n");
#endif
	SETFLAGBITS(next, ACTIVEBIT);
	return &next[1]; 
      }
      else if (GETSIZE(next) > n) {
				/* Split a block */
	int* ans = next + (GETSIZE(next) - n);
#ifdef DEBUG
	_monitor("Split block\n");
#endif
	*next -= n+1;
	*ans = n | ACTIVEBIT;
	return &ans[1];
      }
      else {
#ifdef DEBUG
	fprintf(stderr,"Small free block at %p(%x)\n", next, next[0]);
#endif
	next = next + GETSIZE(next) + 1;
	if (GETBITS(next) == END_OF_HUNK) {
#ifdef DEBUG
	fprintf(stderr,"End of a hunk\n");
#endif
	  if (GETSIZE(next) == 0) {
				/* Add new chunk */
	    int size = 2*sizeof(int)+(n>INITIAL_ALLOCATION? n : INITIAL_ALLOCATION);
	    int *nn = _sbrk(size);
	    if (nn == NULL) _sysdie("Out of heap");
#ifdef DEBUG
	    fprintf(stderr,"New hunk at %p\n", nn);
#endif
	    if (nn == &end_of_heap[1]) {
				/* Contiguous block */
#ifdef DEBUG
	      fprintf(stderr,"Contiguous hunk\n");
#endif
	      end_of_heap = &nn[size];
	      nn[(size>>2)-1] = END_OF_HUNK;
	      next[0] += (size>>2)-1;
#ifdef DEBUG
	    fprintf(stderr,"Now next[] is %x, end at %p\n",next[0], &nn[(size>>2)-1]);
#endif
	    }
	    else { nn[(size>>2)-1] = (int)heaps | END_OF_HUNK;
		 nn[0] = (size>>2)-1 | FREEBIT;
 		 heaps = next = nn;
		 end_of_heap = &nn[size];
		 }
	  }
	  else next = (int*)GETSIZE(next);
	}
      }
    }
    else {
#ifdef DEBUG
      fprintf(stderr,"Active block at %p(%x)\n", next, next[0]);
#endif
      next = next + GETSIZE(next) + 1;
      if (GETBITS(next) == END_OF_HUNK) {
#ifdef DEBUG
	fprintf(stderr,"End of a hunk\n");
#endif
	if (GETSIZE(next) == 0) {
				/* Add new chunk */
	  int size = 2*sizeof(int) + (n>(sizeof(int)*INITIAL_ALLOCATION) ?
				    n :
				    sizeof(int)*INITIAL_ALLOCATION);
	  int *nn = _sbrk(size);
	  if (nn == NULL) _sysdie("Out of heap");
#ifdef DEBUG
	  fprintf(stderr,"New hunk at %p, currently at %p\n", nn, next);
#endif
	  if (nn == &end_of_heap[1]) {
				/* Contiguous block */
#ifdef DEBUG
	    fprintf(stderr,"Contiguous hunk\n");
#endif
	    end_of_heap = &nn[size];
	    nn[(size>>2)-1] = END_OF_HUNK;
	    next[0] = (size>>2)-1 | FREEBIT;
#ifdef DEBUG
	    fprintf(stderr,"Now next[] is %x, end at %p\n",next[0], &nn[(size>>2)-1]);
#endif
	  }
	  else { nn[(size>>2)-1] = (int)heaps | END_OF_HUNK;
		 nn[0] = (size>>2)-1 | FREEBIT;
		 end_of_heap = &nn[size];
 		 heaps = next = nn;
	       }
	}
	else next = (int*)GETSIZE(next);
      }
#ifdef DEBUG
      fprintf(stderr,"Search for %x..(%p) ",n,next);
#endif
    }
  }
  return _sbrk(n);
}

void free(void* vblk)
{
  int *blk = vblk;
  SETFLAGBITS(&blk[-1], FREEBIT);
#ifdef DEBUG
  fprintf(stderr,"Free called on %p(%x,%x)\n", blk, blk[-1],blk[0]);
#endif
}

extern void* calloc(count, size)
size_t count;
size_t size;
{ void* r;
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
  if (l >= 4*1024*1024) _sysdie("calloc called for too much");
  r = malloc(l);
  if (r != NULL) memset(r, 0, l);
  return r;
}

void* _sys_alloc(size_t n)
{
  return malloc(n);
}

void* realloc(void *p, size_t size)
{
  void *new;
  _monitor("Realloc\n");
  new = malloc(size);
  if (p == NULL) return new;
  memcpy(new, p, size);
  return new;
}
