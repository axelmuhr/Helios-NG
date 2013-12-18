

#ifdef TEST

#include <asm.h>
#include <stdio.h>
#include <syslib.h>
#include <sem.h>

#define malloc _malloc_
#define free _free_
#define realloc _realloc_

extern void *malloc(int size);
extern void *realloc(void *addr, int size);
extern void free(void *addr);


static void *_malloc(int size);
static void *_realloc(void *addr, int size);
static void _free(void *addr);

#else

#include <sysinfo.h>
#include <asm.h>
#include <sem.h>

#define ProbeValue1	0x87654321
#define ProbeValue2	0xaaaaaaaa
#define ProbeValue3	0x55555555
#define ProbeStep	0x10000		/* probe increment in words	*/

static void *_malloc(int size);
static void *_realloc(void *addr, int size);
static void _free(void *addr);

extern void *memtop(void)
{
	SysInfo *sysinfo = _SYSINFO;
	byte *base = sysinfo->freestore;
	word probestep = ProbeStep;
	word *probe = (word *)(((word)base + 3) & ~3);
	word *p, *q;

	/* make probe step > memory used so far */
	while( probestep < ((word)probe-MinInt)/4 ) probestep = probestep<<1;
	
	*probe = ProbeValue1;

	/* first get any memory shadow frequency */	
	for( p = probe+probestep ; (*p != *probe) && ((word)p < 0) ; p += probestep );
	
	/* now see how much of this is occupied */
	for( q = probe+probestep ; q < p ; q += probestep )
	{
		*q = ProbeValue2;
		if( *q != ProbeValue2 ) break;
		*q = ProbeValue3;
		if( *q != ProbeValue3 ) break;
	}
	
	/* return address of top of memory */

	q -= 1024;	/* leave space for 4k trace vector */
	sysinfo->TraceVec = (word *)(0x80000000+(word)q-(word)probe);
		
	return (void *)(0x80000000+(word)q-(word)probe);
}

extern int memtest(word *base, int size)
{
	word *p;
	word *end = base + size/4;

	for(p = base; p != end; p++ )
	{
		*p = ProbeValue2;
		if( *p != ProbeValue2 ) return 1;
		*p = ProbeValue3;
		if( *p != ProbeValue3 ) return 2;
	}
	
	for( p = end-1; p >= base; p-- ) *p = (word)p;
	
	for( p = end-1; p >= base; p-- )
	{
		if( *p != (word)p ) return 3;
	}
		
	return 0;
}
#endif

#define BFfree		0x01		/* set if block is free		*/

#define BFree_(i)	(mem.blocks[i]&BFfree)
#define SetFree_(i)	(mem.blocks[i]|=BFfree)
#define ClrFree_(i)	(mem.blocks[i]&=~BFfree)
#define Size_(i)	(mem.blocks[i]&~BFfree)

#define MemInc		8		/* increment on block list	*/

static struct memdesc {
	byte	*base;
	word	*blocks;
	word	inuse;
	word	max;
} mem, fast, save;

static Semaphore lock;

static void alloc_init()
{
	static void *alloc(int size);
	word size;
#ifdef TEST
	byte *base = Malloc(10000);
	byte *top = (void *)((char *)base+10000);
#else
	SysInfo *sysinfo = _SYSINFO;
	byte *base = sysinfo->freestore;
	byte *top = (byte *)sysinfo->TraceVec;
	if( top == NULL ) top = memtop();
#endif		
	size = top-base;
	
	mem.base = base;
	mem.blocks = (word *)base;
	size -= MemInc * sizeof(word);

	mem.blocks[0] = MemInc * sizeof(word);
	
	mem.blocks[1] = size;
	SetFree_(1);
	
	mem.inuse = 2;
	mem.max = MemInc;
	
	fast.base = (byte *)0x80000070;
	fast.blocks = (word *)alloc(MemInc*sizeof(word));
	fast.blocks[0] = (0x1000-0x70)+1;
	fast.inuse = 1;
	fast.max = MemInc;
	
	InitSemaphore(&lock,1);
}

#ifdef TEST
void show(struct memdesc mem)
{
	int i;
	byte *baddr = mem.base;
	printf("in use %d/%d\n",mem.inuse,mem.max);
	for( i = 0; i < mem.inuse; baddr+=Size_(i),i++ )
	{
		printf("%2d %8x %4d %s\n",i,baddr,Size_(i),
			BFree_(i)?"free":"alloc");
	}
}
#endif

static void split(int i, word size, word diff)
{
	int j;
	int freebit = BFree_(i);

	for( j = mem.inuse; j > i; j-- ) mem.blocks[j] = mem.blocks[j-1];

	mem.blocks[i+1] = diff;

	mem.blocks[i] = size;

	if( freebit ) SetFree_(i),SetFree_(i+1);
	

	mem.inuse++;
}

static void *alloc(int size)
{
	byte *baddr = mem.base;
	int i;
	
	for( i = 0; i < mem.inuse; baddr+=Size_(i),i++ )
	{
		if( BFree_(i) && Size_(i) >= size )
		{
			int diff = Size_(i)-size;
			if( diff > 32 ) split(i,size,diff);
			ClrFree_(i);
			
			return baddr;
		}
	}
	return NULL;
}

static bool extend(void)
{
	word newsize = mem.max + MemInc;
	word *new = alloc(newsize * sizeof(word));
	word *old = mem.blocks;

	if( new == NULL ) return FALSE;
		
	move_(mem.max*sizeof(word),new,old);
		
	mem.blocks = new;
	mem.max = newsize;
		
	_free(old);
	
	return TRUE;
}
	
static void coalesce(int i)
{
	mem.blocks[i] += Size_(i+1);		/* preserves free bit of block */

	mem.inuse--;	
	
	move_((mem.inuse-i)*sizeof(word),&mem.blocks[i+1],&mem.blocks[i+2]);
}

static void _free(void *addr)
{
	int i;
	byte *baddr = mem.base;
	
	/* look for Block which starts at this address			*/
	for( i = 0; i < mem.inuse; baddr+=Size_(i),i++ )
	{
		if( baddr == addr )
		{
			/* if already free, return			*/
			if( BFree_(i) ) return;
			SetFree_(i);
			/* coalesce with next block if necessary	*/
			if( i+1 != mem.inuse && BFree_(i+1) ) coalesce(i);
			/* coalesce with prev block too			*/
			if( i != 0 && BFree_(i-1) ) coalesce(i-1);
			return;
		}
	}
	return;					/* invalid block */
}

static void *_malloc(int size)
{
	/* adjust size of memory to whole number of words	*/
	size = (size + sizeof(word) - 1);
	size -= size % sizeof(word);
	
	/* if there is only 1 more block slot, extend table	*/
	if( mem.inuse+1 == mem.max && !extend() ) return NULL;

	return alloc(size);
}

static void *_realloc(void *addr, int size)
{
	int i;
	byte *baddr = mem.base;
	
	/* adjust size of memory to whole number of words	*/
	size = (size + sizeof(word) - 1);
	size -= size % sizeof(word);
	
	/* look for Block which starts at this address			*/
	for( i = 0; i < mem.inuse; baddr+=Size_(i),i++ )
	{
		if( baddr == addr )
		{
			void *new;
			int oldsize = Size_(i);
			int diff = size-oldsize;
			
			if( BFree_(i) ) return NULL;

			/* if the block is shrinking, just lop off the	*/
			/* remainder					*/
			if( diff <= 0 )
			{
				/* do nothing if shrink is small	*/
				if( diff >= -32 ) return addr;
			
				/* ensure space for split */
				if( mem.inuse+1 == mem.max && !extend() ) return NULL;

				split(i,size,-diff);
				SetFree_(i+1);
				return addr;
			}
			
			/* else the block is growing */
			
			/* if the next block is free and large enough	*/
			/* just grow current block into it		*/
			if( BFree_(i+1) && Size_(i+1) >= diff )
			{
				if( (Size_(i+1)-diff) > 32 )
				{
					mem.blocks[i] += diff;
					mem.blocks[i+1] -= diff;
				}
				else coalesce(i);
				return addr;
			}

			/* finally we have no other choice but to move	*/
			/* the whole block				*/

			new = _malloc(size);

			move_(size,new,addr);

			_free(addr);

			return new;
		}
	}
}

/* external procedures */

extern void *malloc(int size)
{
	void *x;

	/* init memory system if necessary 			*/
	if( mem.blocks == NULL ) alloc_init();
	
	Wait(&lock);
	x = _malloc(size);
	Signal(&lock);
	return x;
}

extern void *realloc(void *addr, int size)
{
	void *x;
	Wait(&lock);
	x = _realloc(addr,size);
	Signal(&lock);
	return x;
}

extern void free(void *addr)
{
	Wait(&lock);
	_free(addr);
	Signal(&lock);
}

extern void freestop(void *addr)
{
	Wait(&lock);
	_free(addr);
	SignalStop(&lock);
}

extern void *malloc_fast(word size)
{
	void *x;
	
	/* init memory system if necessary 			*/
	if( mem.blocks == NULL ) alloc_init();
	
	Wait(&lock);
	
	/* adjust size of memory to whole number of words	*/
	size = (size + sizeof(word) - 1);
	size -= size % sizeof(word);

	/* if there is not enough room in fast memory blocks extend it */
	if( fast.inuse == fast.max )
	{
		word newsize = fast.max + MemInc;
		word *new = _malloc(newsize * sizeof(word));
		word *old = fast.blocks;

		if( new == NULL ) return NULL;
		
		move_(fast.max*sizeof(word),new,old);
		
		fast.blocks = new;
		fast.max = newsize;
		
		_free(old);
	}

	/* switch memory blocks data to fast RAM		*/
	save = mem;
	mem = fast;
	
	/* allocate in fast RAM					*/
	x = alloc(size);
	
	/* switch data back to normal				*/
	fast = mem;
	mem = save;
	
	Signal(&lock);
	
	return x;
}


extern void free_fast(void *addr)
{
	Wait(&lock);

	/* switch memory blocks data to fast RAM		*/
	save = mem;
	mem = fast;
	
	/* free in fast RAM					*/
	_free(addr);
	
	/* switch data back to normal				*/
	fast = mem;
	mem = save;
		
	Signal(&lock);
}

#ifdef TEST

int main()
{
	void *x,*y,*z;
	int i;
	
	x = malloc(100);

	y = malloc_fast(200);
	y = malloc_fast(200);
	y = malloc_fast(200);
	y = malloc_fast(200);
	y = malloc_fast(200);
	y = malloc_fast(200);
	y = malloc_fast(200);
	y = malloc_fast(200);
	y = malloc_fast(200);
	y = malloc_fast(200);
	
	show(fast);
	show(mem);
}

#endif
