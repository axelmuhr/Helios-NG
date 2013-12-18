/*
  Title:        alloc - Storage management (dynamic allocation/deallocation)
  $Revision: 1.1 $  John Fitch Oct 1990
  Copyright (C) Codemist Ltd., 1990
*/

/*
 * This version is for UNIX only.  It make assumptions about the use of SBRK
 * but not much else.  Mechanism is first-fit, with lazy amalgamation of 
 * continguous free blocks.
 * AM: various changes Jun 91 to allow for alignof(double) = 8.
 */

#include "hostsys.h"
#include <string.h>
#include <stddef.h>

#undef DEBUG

typedef struct BlockStruct {
  int			flag;
  int			size;
  struct BlockStruct	*cdr;
  int			pad;	/* avoid cross compile trouble! */
  double		space[1];
} Block, *BlockP;

#define FREEBIT		(1<<28)
#define ACTIVEBIT	(2<<28)
#define BYTES2WORDS(n)	((n+7 & ~7)>>2)
#define SIZE		BYTES2WORDS(offsetof(Block,space))

#ifdef DEBUG
#define _monitor(s) _ttywrite(s,sizeof(s),NULL)
static int fd_n, fd_d, xxx;
#define FD(i, b) {fd_n = i; fd_d = 1; \
        while (fd_n >= b) {fd_d *= b; fd_n /= b;} fd_n = i; \
	while(fd_d) {if ((fd_n/fd_d) > 9) { xxx = fd_n/fd_d+'a'-10; \
			           _syscall3(SYS_write,0,(int)&xxx,1);} \
                  else { xxx = fd_n/fd_d+'0'; \
			 _syscall3(SYS_write,0,(int)&xxx,1);} \
                  fd_n = fd_n-(fd_n/fd_d)*fd_d; fd_d /= b;}}
#endif

extern void* _sbrk(int);
BlockP heaps;

void _init_alloc(void)
{
  heaps = NULL;
}

static char sys_message[60];

static void _alloc_die(message)
char *message;
{
  strcpy(sys_message, message);
  _sysdie(sys_message);
}

void* malloc(int n)
{
  BlockP next = heaps;
  BlockP last = next;
  int words = BYTES2WORDS(n);
  if (n==0) return NULL;
#ifdef DEBUG
  _monitor("Malloc called "); FD(n,16) _monitor(" ");
                              FD(words,16) _monitor("\n");
#endif
  if (words<0 || words>4*1024*1024) _sysdie("Too large an allocation\n");
  while (next != NULL) {
#ifdef DEBUG
    _monitor("Next is "); FD((int)next,16) _monitor("\n");
#endif
    if (next->flag == FREEBIT &&
        next->cdr != NULL && next->cdr->flag == FREEBIT &&
	next->cdr == (Block *)&((int*)next->space)[next->size]) {
#ifdef DEBUG
      _monitor("Amalgamate two chunks\n");
#endif
      next->size += next->cdr->size + SIZE;
    }
    else if (next->flag == FREEBIT && next->size == words) {
#ifdef DEBUG
      _monitor("Exact size found\n");
#endif
      next->size = ACTIVEBIT;
      return next->space;
    }
    else if (next->flag == FREEBIT &&
	     next->size > words + SIZE) {
      /* Split the block -- use the lower address part: */
      BlockP s = (BlockP) ((int*)next->space)[words];
      s->flag = FREEBIT;
      s->size = next->size - (words+SIZE);
      s->cdr = next->cdr;
      next->flag = ACTIVEBIT;
      next->size = words;
      next->cdr = s;
      return next->space;
    }
    else {
      last = next;
      next = next->cdr;
    }
  }
#ifdef DEBUG
  _monitor("Getting more store\n");
#endif
  next = _sbrk(n+offsetof(Block,space));
  if (next == (void*)-1) return (void*)0; /* Failed to get memory */
  next->flag = ACTIVEBIT;
  next->size = words;
  next->cdr = NULL;
  if (last == NULL) heaps = next; else last->cdr = next;
  return next->space;
}

void free(void* vblk)
{
  BlockP blk = vblk;
  blk->flag = FREEBIT;
/*   _monitor("Free called on "); FD((int)blk, 16) _monitor("\n"); */
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
  void *ans = malloc(n);
  if (ans != NULL) return ans;
  _alloc_die("No store left for I/O buffer or the like");
  return 0;
}

void* realloc(void *p, size_t size)
{
  void *new;
/*  _monitor("Realloc\n"); */
  new = malloc(size);
  if (p == NULL) return new;
  memcpy(new, p, size);
  return new;
}
