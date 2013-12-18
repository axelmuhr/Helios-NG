/*------------------------------------------------------------------------
--                                                                      --
--                      H E L I O S   K E R N E L                       --
--                      -------------------------                       --
--                                                                      --
--             Copyright (C) 1988, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- memory.c								--
--                                                                      --
--	Kernel memory allocation.					--
--	This also contains Buffer management and Kernel worker process	--
--	routines.							--
--                                                                      --
--	Author:  NHG 10/8/88						--
--	Extras:	 PAB Nov 90
--                                                                      --
------------------------------------------------------------------------*/
/* $Id: memory1.c,v 1.41 1993/10/04 15:05:59 paul Exp $ */

/*{{{  Defines and Includes */

#define __in_memory 1	/* flag that we are in this module */

#include "kernel.h"

#include <memory.h>
#include <root.h>
#include <config.h>
#include <stdarg.h>
#ifdef __RRD
# include "ram.h" /* -I $(NUCLEUS)/ARM in makefile - should be $(SERVERS) */
# include <limits.h>
# include <stddef.h>
#endif

#ifdef __TRAN
# ifdef SYSDEB
#  define MEMCHECK 1	/* set to 1 for memory checking */
# else
#  define MEMCHECK 0
# endif
#else
# define MEMCHECK 0
#endif

#if MEMCHECK
# define v_(x) ((void *)(x))
# define CHECKBLOCK(id,b,p)						\
if( (v_(b) < v_(p)) || (v_(b) > v_((p)->Memory.Head)) ||		\
    (v_((b)->Node.Next->Prev) != v_(b)) ||				\
    (v_((b)->Node.Prev->Next) != v_(b)) ||				\
    ((b)->Size < 0) || ((b)->Size > (p)->Max+4) )			\
{									\
	_Trace(0xbbaadd00+id,(word)(b),(word)(p)); for(;;);		\
}
#else
# define CHECKBLOCK(id,b,p)
#endif

#ifdef __MI
# ifdef SYSDEB
#  define DBOutput(x) Output(x)
#  define DBWriteHex8(x) WriteHex8(x)
# else
#  define DBOutput(x) 
#  define DBWriteHex8(x)
# endif
#endif

#define ProbeValue1	0x87654321
#define ProbeValue2	0xaaaaaaaa
#define ProbeValue3	0x55555555
#define ProbeStep	0x10000		/* probe increment in words	*/
#define Epsilon		(sizeof(Memory)*2)

/*}}}*/
/*{{{  Forward Definitions */

#ifdef __C40
#define GLOBALRAM	1
#endif

#if defined(ONCHIPRAM) || defined(GLOBALRAM)

#  define USESPECIAL	1

void InitFastPool(void);

typedef struct MC { Memory m; Carrier c; } MC;
static Carrier *AllocIndirect(word size, word type, Pool *pool);
void FreeIndirect(MC *carrier);

#endif

#ifdef __TRAN
static byte *StoreSize(byte *base);
#else
# ifdef ONCHIPRAM
word FastStoreSize(MPtr *base);
# endif
byte *StoreSize(byte *base);
word *_GetModTab(void);
#endif
#ifndef __CARD
static Pool *BuildPool(byte *base, word size, bool scavenge);
#endif
#ifdef __RRD
# ifndef __CARD
static word RRDPoolInit(MIInfo *, Memory *magicbase, Memory *magictop);
# endif
static word Scavenge(word *bottom, word size);
#endif
extern void debinit(void);
#ifdef __C40
static Pool * BuildC40FreePool(byte *base, word size, bool scavenge);
#endif
#ifdef GLOBALRAM
static void InitGlobalPool(void);
#endif
#ifdef __MI
/* These sizes must be a multiple of 16 */
# define MIINC	128	/* Size of MI table is incremented by when full */
# define MIINIT	128	/* Initial number of slots in table */
#endif

/*}}}*/
/*{{{  MemInit */
/*--------------------------------------------------------
-- MemInit						--
--							--
-- Initialise memory system				--
--							--
--------------------------------------------------------*/

void MemInit(Config *config, word confsize)
{
	byte *membase;
	byte *storetop;

	RootStruct *root = GetRoot();

	InitPool(&root->SysPool);
	
	/* config follows root struct */
	membase = (byte *)((byte *)GetConfig() + confsize);

	if( config->MemSize == 0 ) storetop = StoreSize(membase);
#ifdef __TRAN
	else storetop = (byte *)(MinInt+config->MemSize);
#else
# ifdef __C40
	else {
		storetop = (byte *)C40CAddress(GetExecRoot()->ID_ROM.LBASE0) +
			config->MemSize;
	}
# else
	else
		storetop = (byte *)(config->MemSize);
# endif
#endif

	/* set aside (usually 4k) memory for trace buffer */
	storetop -= TVSize;
	root->TraceVec = (word *)storetop;
	debinit();

#ifndef __TRAN
# if defined KERNELDEBUG3 && defined SYSDEB
	/* set aside (usually 20k) memory for kernel dbg msg dump buffer */
	storetop -= KDEBUG3SIZE;
# endif
#endif

#ifndef __RRD
# ifdef __C40
	if( config->MemSize == 0 )
		/* Support for non-contiguous strobe 0 and 1 banks on local bus */
		root->FreePool = BuildC40FreePool(membase, storetop - membase, FALSE);
	else
		/* user specified memsize can only be used for contigous blocks */
		root->FreePool = BuildPool(membase, storetop - membase, FALSE);
# else
	/* simply build the free pool using all available memory */
	root->FreePool = BuildPool(membase, storetop - membase, FALSE);
# endif
#else

/* If Robust Ram Disk supporting system, Scavenge memory for surviving	*/
/* RAMFS blocks.							*/

/* These blocks are compacted to the high end of memory and entered	*/
/* into the RRD pool. The Free pool is then built in the remaining space*/
/* (@@@ We need a startup sequence that can force the meminit to	*/
/* disreguard any blocks - exec could simply zero RAM?			*/
/* When the RRD starts up, it examines the contents of its Pool,	*/
/* further validating the blocks and building a directory hierachy	*/
/* from its contents							*/
{
	Memory *magicbase, *magictop;
	
	/* move magic blocks to high memory and build free pool below them */
	root->FreePool = BuildPool(membase, storetop-membase, TRUE);

	/* get pointer to start of magic block save area */
	magicbase = (Memory *)((word)(root->FreePool->Memory.Tail) + (((Memory *)(root->FreePool->Memory.Tail))->Size & ~Memory_Size_BitMask));
	magictop = (Memory *)root->FreePool->Memory.Head;
	
	/* RRDPool and MISysMem automatically allocated after FreePool pool */
	root->RRDPool = root->FreePool + 1;
	InitPool(root->RRDPool);
	root->MISysMem = (MIInfo *)(root->RRDPool + 1);

	MIInit(root->MISysMem, root->FreePool, root->RRDPool);

	root->RRDScavenged = RRDPoolInit(root->MISysMem, magicbase, magictop);
}
#endif /* __RRD */

	InitList(&root->BufferPool);

	/* setup buffer pool counters					*/
	/* We allow about 100 buffers per Mbyte before reporting	*/
	/* congestion, and about 4 buffers per Mbyte of permanently	*/
	/* allocated buffers and never less than 8 in total.		*/
	
	root->BuffPoolSize = root->FreePool->Max/250000;
	if( root->BuffPoolSize < 8 ) root->BuffPoolSize = 8;

	root->BufferCount  = 0;
	root->MaxBuffers   = 0;

#ifdef USESPECIAL
	root->SpecialPools = (Pool **)Allocate(sizeof(Pool *)*8,root->FreePool,&root->SysPool);
#endif
	
#ifdef ONCHIPRAM

	InitFastPool();

#endif

#ifdef GLOBALRAM
	/* Global RAM is memory which is outside the normal address	*/
	/* range. It must be managed indirectly in the same way as	*/
	/* on chip RAM.							*/

	InitGlobalPool();
#endif

}

void MemInit2(void)
{
	RootStruct *root = GetRoot();
	word i = root->BuffPoolSize;
	Buffer *b;
	List bp;

	InitList(&bp);
	
	while( i-- )
	{
		b = GetBuf(Worker_stacksize);
		b->Type = Buffer_Type_Cache;
		AddTail(&bp,&b->Node);
	}
	
	until( EmptyList_(bp) ) {
		FreeBuf( (Buffer *)RemHead(&bp) );
	}

	root->BufferCount  = 0;
	root->MaxBuffers   = 0;
}
/*}}}*/
/*{{{  StoreSize */

/*--------------------------------------------------------
-- StoreSize						--
--							--
-- Find out how much memory we really have.		--
--							--
--------------------------------------------------------*/

#ifdef __TRAN
/* On other processors this function is replaced by a executive fn
 * that returns amount of available memory. As this function destructively
 * sizes memory, it cannot be used with the Robust Ram Disk
 */
static byte *StoreSize(byte *base)
{
	word *probe = (word *)(((word)base + 3) & ~3);
	word *p, *q;

	*probe = ProbeValue1;

	/* first get any memory shadow frequency */	
	for( p = probe+ProbeStep ; (*p != *probe) && ((word)p < 0) ; p += ProbeStep );
	
	/* now see how much of this is occupied */
	for( q = probe+ProbeStep ; q < p ; q += ProbeStep )
	{
		*q = ProbeValue2;
		if( *q != ProbeValue2 ) break;
		*q = ProbeValue3;
		if( *q != ProbeValue3 ) break;
	}
	
	/* return address of top of memory */
	return (byte *)(MinInt+((word)q-(word)probe));
}
#endif
/*}}}*/
/*{{{  BuildPool */

/*--------------------------------------------------------
-- BuildPool						--
--							--
-- Initialise a given chunk of memory to be a memory	--
-- pool.						--
--							--
-- If scavenge is TRUE, move all magic blocks to	--
-- high memory and build the pool below them.		--
--							--
--------------------------------------------------------*/

#ifdef __CARD
PUBLIC Pool *BuildPool(byte *base, word size, bool scavenge)
#else
static Pool *BuildPool(byte *base, word size, bool scavenge)
#endif
{
	Pool *pool;
	Memory *top;
	Memory *main;
	
	/* align pool to 16 byte boundary */
	pool = (Pool *)(((word)base + 15) & ~15);

	/* trim size to multiple of 16 bytes */
	size = (((word)base+size) & ~15) - (word)pool - sizeof(Memory) - sizeof(Pool);

#ifdef __MI
	/* automatically allocate RRDPool and MI Info block */
	size -= ((sizeof(Pool) + sizeof(MIInfo) + 15) & ~15);

	/* Note that the positions of these blocks and the first Memory */
	/* block in the pool are assumed in MICompact(), card insertion and */
	/* system memory initialisation */
#endif

	InitList(&pool->Memory);
	pool->Max = size;
	
#ifdef __MI
	main = (Memory *)( (word)(pool + 1) + ((sizeof(Pool) + sizeof(MIInfo) + 15) & ~15) );
#else
	main = (Memory *)(pool + 1);			/* main block */
#endif
	top = (Memory *)((word)main + size);		/* dummy top block */

	/* now set up dummy top block */
	top->Size = sizeof(Memory) | Memory_Size_FwdBit;
	top->Pool = pool;
	AddHead(&pool->Memory, &top->Node);

#ifdef __RRD
	if (scavenge)
		size = Scavenge((word *)main, size);
#endif

	/* set up pool info */
	pool->Blocks = 1;
	pool->Size = size;

	/* set up main block */
	main->Size = size | Memory_Size_BwdBit;
	main->Pool = pool;
	AddTail(&pool->Memory, &main->Node);

	/* set main blocks back pointer */
	((word *)((word)main + size))[-1] = (word)main;

	return pool;
}
/*}}}*/
/*{{{  BuildC40FreePool */

#ifdef __C40
/*------------------------------------------------------------------------
-- BuildC40FreePool							--
--									--
-- Support for non-contiguous strobe 0 and 1 on local bus. StoreSize()	--
-- simply returns a pointer to the last usable byte of local memory.	--
-- This may mean that on boards such as the Hema DSP1, there is a null	--
-- area of memory between the two banks that cannot be used.		--
-- BuildC40FreePool() is similar to the normal BuildPool(), but in	--
-- addition it may inserts a dummy pre-allocated block that will never	--
-- be used or free'd that bridges this inter-memory-block gap.		--
--									--
-- When this routine is called the situation may be as follows:		--
--  1) There is some memory on local strobe 0 or local strobe 1 or both.--
--  2) Strobe 1 may occur before strobe 0.				--
--  3) The nucleus may be loaded into either strobe, not necessarily	--
--     at the start of local memory. If so the root structure etc. will	--
--     follow after the nucleus.					--
--  4) Or the nucleus may be loaded somewhere in the global bus. If so	--
--     the root structure etc. will still be somewhere on the local bus,--
--     but there are no guarantees about where.				--
--  5) The base argument points to just after the root structure etc.	--
--     This may not be anywhere near the real start of local memory.	--
--  6) The msize argument gives a maximum size for usable memory, and	--
--     has been updated to allow for the trace vector and other special	--
--     areas of memory.							--
--  7) On a C40 the various strobes are guaranteed word-aligned but not	--
--     16-byte aligned, a requirement for the memory allocation code.	--
--									--
------------------------------------------------------------------------*/

static Pool *BuildC40FreePool(byte *base_ptr, word msize, bool scavenge)
{ IDROM		*idrom	= &(GetExecRoot()->ID_ROM);
  RootStruct	*root	= GetRoot();
  Pool		*pool;
  Memory	*main;
  Memory	*dummy;
  Memory	*above;
  Memory	*top;

	/* Step 1: stop worrying about strobe 0 and strobe 1, and	*/
	/* switch to 1st and 2nd which are sorted. words rather than	*/
	/* pointers are used to avoid excessive numbers of warnings	*/
	/* from the C compiler.						*/
  word		first_mem	= (word) C40CAddress(idrom->LBASE0);
  word		second_mem	= (word) C40CAddress(idrom->LBASE1);
  word		first_size	= idrom->LSIZE0 * sizeof(word);
  word		second_size	= idrom->LSIZE1 * sizeof(word);
  word		base		= (word) base_ptr;
  word		storetop;

  if ((idrom->LBASE0 == 0xffffffff) || (idrom->LSIZE0 == 0))
   { first_mem	 = second_mem;
     first_size	 = second_size;
     second_mem	 = NULL;
     second_size = 0;
   }
  elif ((idrom->LBASE1 == 0xffffffff) || (idrom->LSIZE1 == 0))
   { second_mem	 = NULL;
     second_size = 0;
   }
  elif (idrom->LBASE0 < idrom->LBASE1)
   {	/* This is the usual sensible layout for two banks of memory.	*/
   }
  else
   { first_mem  	= second_mem;
     first_size		= second_size;
     second_mem		= (word) C40CAddress(idrom->LBASE0);
     second_size	= idrom->LSIZE0 * sizeof(word);
   }

	/* Step two: adjust one of these memory blocks to allow for the	*/
	/* base argument. Usable memory starts after the ExecRoot, root	*/
	/* and saved configuration vector. Also after the nucleus if	*/
	/* that is held in local rather than global memory.		*/
  if (base < (first_mem + first_size))
   { first_size -= base - first_mem;
     first_mem   = base;
   }
  else
   { second_size -= base - second_mem;
     second_mem   = base;
   }

	/* Step three: adjust one of these memory blocks to allow for	*/
	/* the msize argument. The trace vector is held at the end	*/
	/* of memory. Other kernel debug areas may be there as well.	*/
	/* N.B. the msize argument passed is storetop - base, which may	*/
	/* bear very little relation to the real memory size.		*/  
  storetop = base + msize;
  if ((first_mem + first_size) > storetop)
   first_size = storetop - first_mem;
  else
   second_size = storetop - second_mem;

	/* Step 4: align the two bits of memory to 16 byte boundaries	*/
	/* and adjust the sizes accordingly. Note that Pool and Memory	*/
	/* structures are both multiples of 16 bytes so no further	*/
	/* alignment will be needed after this phase.			*/
  first_mem	= (first_mem + 15) & ~15;
  first_size	= (first_size - 16) & ~15;
  second_mem	= (second_mem + 15) & ~15;
  second_size	= (second_size - 16) & ~15;

	/* Step 5: it is now possible to determine the locations of	*/
	/* pool and the various bits of memory and initialise		*/
	/* everything.							*/
  pool		= (Pool *) first_mem;
  InitList(&(pool->Memory));
  pool->Max	= first_size + second_size;
		/* allow for pool, main and top				*/
  pool->Max	= pool->Max - sizeof(Pool) - sizeof(Memory) - sizeof(Memory);
  pool->Size	= pool->Max;
  pool->Blocks	= 1; /* only main for now */
  main		= (Memory *) (pool + 1);
  main->Pool	= pool;
  AddTail(&(pool->Memory), &(main->Node));

  if (second_mem == NULL)
   { top	= (Memory *) (first_mem + first_size - sizeof(Memory));
     main->Size = ((word)top - (word)main)   | Memory_Size_BwdBit;
     top->Size	= sizeof(Memory) | Memory_Size_FwdBit;
     top->Pool	= pool;
     AddHead(&(pool->Memory), &(top->Node));
	/* Back pointer	*/
     ((word *)((word)main + main->Size))[-1] = (word) main;
   }
  else
   { dummy		= (Memory *) (first_mem + first_size - sizeof(Memory));
     main->Size		= ((word)dummy - (word)main);
     above		= (Memory *) second_mem;
     dummy->Size	= ((word)above - (word)dummy) | Memory_Size_FwdBit;
     top		= (Memory *) (second_mem + second_size - sizeof(Memory));
     above->Size	= ((word)top - (word)above);
     top->Size		= sizeof(Memory) | Memory_Size_FwdBit;

     dummy->Pool	= &(root->SysPool);
     AddTail(&(root->SysPool.Memory), &(dummy->Node));
     above->Pool	= pool;
     AddHead(&(pool->Memory), &(above->Node)); /* between top and main */
     pool->Blocks++;
     top->Pool		= pool;
     AddHead(&(pool->Memory), &(top->Node));

	/* Back pointers	*/
     ((word *)((word)main + main->Size))[-1]   = (word) main;
     ((word *)((word)above + above->Size))[-1] = (word) above;
     main->Size  |= Memory_Size_BwdBit;
     above->Size |= Memory_Size_BwdBit;

	/* allow for dummy and above	*/
     pool->Size -= 2 * sizeof(Memory);
   }

  return(pool);
}

#endif

/*}}}*/
/*{{{  Allocate */

/*--------------------------------------------------------
-- Allocate						--
--							--
-- Allocate from the given free source pool into the	--
-- given destination pool.				--
--							--
--------------------------------------------------------*/

void *Allocate(word size, Pool *source, Pool *dest)
{
	Memory *block;
	word bsize;

#if 0
	/* as soon as the processor xoffs, start failing all Allocates	*/
	/* this may cause a program or two to exit, but this should then*/
	/* allow the system to continue.				*/
	
	if( GetRoot()->Flags & Root_Flags_xoffed ) return NULL;
#endif

	/* add size of Memory header and up to next 16 bytes */
	size = (size + 31) & ~15;

	/* list is in high to low block address order */
	block = Next_(Memory,Head_(Node,source->Memory));

	while( !EndOfList_(block) )
	{
		bsize = block->Size & ~Memory_Size_BitMask;

CHECKBLOCK(1,block,source);
#if MEMCHECK
if( ((word *)block)[(bsize/4)-1] != (word)block )
{ _Trace(0xbbaadd11,(word)block,0); for(;;); };
#endif
		if( bsize >= size )
		{
			if( bsize-size <= Epsilon )
			{
				/* sizes are close, alloc whole block */
				Remove(&block->Node);
				source->Blocks--;
			}
			else
			{
				/* subdivide this block */
				bsize -= size;
				
				/* set new size of old block */
				block->Size = (block->Size & Memory_Size_BitMask)
						+ bsize;
						
				/* set new back pointer */
				((word *)block)[(bsize/sizeof(word))-1] = (word)block;

				/* point to new block */
				block = (Memory *)((word)block+bsize);
				block->Size = bsize = size;
			}
			
			/* here block points to the block we want to allocate */
			/* set its forward bit and successors backward bit    */
			/* FwdBit = this block is allocated */ 
			/* BwdBit = previous block is allocated */
			block->Size |= Memory_Size_FwdBit;
			((Memory *)((word)block+bsize))->Size |= Memory_Size_BwdBit;
			
			/* move block into dest pool */
			block->Pool = dest;	
			AddTail(&dest->Memory, &block->Node);

			dest->Blocks++;
			dest->Size += bsize;
			
			/* adjust source pool size */
			source->Size -= bsize;

#if 1/*def __TRAN*/ /* @@@ debug until we can fix problem in rrd! */
			/* This is the biggest overhead in large allocs */
			/* Transputer version grew up with it so keep it for */
			/* compatibility */
			ZeroBlock(block+1,bsize-sizeof(Memory));
#endif
			/* return address of first free byte */
			return (void *)(block+1);
		}
		
		block = Next_(Memory,block);
	}

#if (defined(__C40) || defined(__ARM)) && defined(SYSDEB)
	KDebug("Allocate Fail! size %d srcp %a dstp %a\n", size, source, dest);
#endif
	return NULL;
}

/*}}}*/
/*{{{  Free */

/*--------------------------------------------------------
-- Free							--
--							--
-- Release a block of memory back to given pool.	--
--							--
--------------------------------------------------------*/

void _Free(Memory *block, Pool *pool)
{
	Memory *next;
	bool prevdone = false;
	bool inlist = false;

	block--;			/* step back to Memory header */
CHECKBLOCK(2,block,pool);

	if ((block->Size & Memory_Size_FwdBit) == 0)
	  {
	    /*
	     * XXX - NC - 23/3/93 - (bug ref 1155)
	     *
	     * The FwdBit is set if the block is alloced, and
	     * unset if the block is free.  If we are passed
	     * a random pointer, then it is still OK to test
	     * this bit, as we are trying to prevent the free
	     * from happening, rather than allowing it to continue.
	     */

#if defined SYSDEB
	    KDebug( "Attempt to free an already free block %x\n", block + 1 );
#endif	    
	    return;
	  }
	else
	  {
	    block->Size &= ~Memory_Size_FwdBit;
	  }
	
	/* remove from current pool */
	Remove(&block->Node);
	{
		Pool *source = block->Pool;
		source->Blocks--;
		source->Size -= block->Size & ~Memory_Size_BitMask;
	}

	/* increment dest pool size */
	pool->Size += block->Size & ~Memory_Size_BitMask;

	/* try to coalesce with physical predecessor */
	if( (block->Size & Memory_Size_BwdBit) == 0 )
	{
		Memory *prev = (Memory *)*(word *)((word)block - sizeof(word));

CHECKBLOCK(3,prev,pool);
		/* add sizes together */
		prev->Size += block->Size & ~Memory_Size_BitMask;
		
		/* switch attention to prev */
		block = prev;
		prevdone = inlist = true;		/* now in list */
	}	

	/* get address of physical successor */
	next = (Memory *)((word)block+(block->Size&~Memory_Size_BitMask));

	/* now try to coalesce with successor */
	if( (next->Size & Memory_Size_FwdBit) == 0 )
	{
CHECKBLOCK(4,next,pool);
		/* combine sizes */
		block->Size += next->Size & ~Memory_Size_BitMask;
		
		if( !inlist )
		{
			/* if not in the list, replace next */
			block->Node.Next = next->Node.Next;
			block->Node.Prev = next->Node.Prev;
			next->Node.Next->Prev = &block->Node;
			next->Node.Prev->Next = &block->Node;
			inlist = true;		/* now in list */
		}
		else {
			/* else remove next from list */
			Remove(&next->Node);
		}
	}

	block->Pool = pool;

	{
		word wsize = (block->Size&~Memory_Size_BitMask)/sizeof(word);
		((word *)block)[wsize-1] = (word)block;
	}

#if MEMCHECK
if( ((word *)block)[((block->Size&0xfffffff0)/4)-1] != (word)block )
{ _Trace(0xbbaadd12,(word)block,0); for(;;); };
#endif

	/* if both neighbours were allocated, we have no option but to	*/
	/* search the list for our correct place.			*/
	if( !inlist )
	{
		next = Head_(Memory,pool->Memory);
		
		while( !EndOfList_(next) )
		{
CHECKBLOCK(5,next,pool);
			if( (word)next < (word)block ) break;
			next = Next_(Memory,next);
		}

		PreInsert(&next->Node,&block->Node);
	}

	/* now set the allocation bits correctly		*/
	block->Size &= ~(Memory_Size_BitMask^Memory_Size_BwdBit);
	
	next = (Memory *)((word)block+(block->Size&~Memory_Size_BitMask));

	next->Size &= ~Memory_Size_BwdBit;
}

/*}}}*/
/*{{{  AllocFast */
#ifdef ONCHIPRAM
/*--------------------------------------------------------
-- AllocFast						--
--							--
-- Allocate memory from fast Ram. This is now just a	--
-- compatibility stub to AllocIndirect.			--
--							--
--------------------------------------------------------*/

Carrier *AllocFast(word size, Pool *pool)
{
	word type = RAMType_Local|RAMType_Direct|RAMType_Static;

#ifdef __C40
	/* On C40, Fast memory is allocated in WORD units.	*/
	/* So, divide size by 4.				*/
	
	size = (size + 3)/4;
#endif
	return (Carrier *)System((WordFnPtr)AllocIndirect,size,type,pool);
}

#endif
/*}}}*/
/*{{{  InitFastPool */

#ifdef ONCHIPRAM
/*------------------------------------------------------------------------
-- InitFastPool()							--
--									--
-- Initialise the fast RAM pool. On the transputer this is the onchip	--
-- RAM. On the C40 this is the onchip RAM, plus any static RAM which	--
-- may be adjecent to it (as on HEMA DSP1). On the C40 the RAM may	--
-- be either directly addressable, or indirectly addressable, depending	--
-- on the placement of RAM on the local BUS.				--
--									--
------------------------------------------------------------------------*/

void InitFastPool()
{
	RootStruct *root = GetRoot();
	Pool *pool = &root->FastPool;
	Carrier *carrier;
	
	/* support for transputers on chip memory */
	/* or any system that has a small amount of high speed RAM */

	InitPool(pool);

	carrier = (Carrier *)Allocate(sizeof(Carrier),root->FreePool,pool);

	/* At present, all fast RAM is on the local Bus			*/
	carrier->Type = RAMType_Local|RAMType_Static;
	
# ifdef __TRAN
	carrier->Addr = (byte *)0x80000070;
	carrier->Size = 0x1000-0x70;
# else
	carrier->Size = FastStoreSize(&carrier->Addr);

	if( C40CAddress(carrier->Addr) == 0 )
			carrier->Type |= RAMType_Indirect;
# endif
	((Memory *)carrier - 1)->Size |= Memory_Size_Carrier;

	root->SpecialPools[RAMTypeIndex(carrier->Type)] = pool;

	/* also set indirect local static RAM pool, for compatibility	*/
	root->SpecialPools[RAMTypeIndex(carrier->Type)|RAMType_Indirect] = pool;	
}
#endif

/*}}}*/
/*{{{  InitGlobalPool */

#ifdef GLOBALRAM

/*------------------------------------------------------------------------
-- InitGlobalPool()							--
--									--
-- The C40 may have memory on the global bus which is outside the	--
-- normal addressing range of Helios programs. This memory may be of	--
-- use to applications, and the loader can put code there. 		--
--									--
-- Depending on the configuration, there may be memory on either, both	--
-- or neither strobes. Also, the nucleus may have been loaded at the	--
-- bottom of on of these areas.						--
------------------------------------------------------------------------*/

void InitGlobalPool(void)
{
	Pool *pool;
	Carrier *carrier;
	RootStruct *root = GetRoot();
	ExecRoot *xroot = GetExecRoot();
	IDROM *idrom = &xroot->ID_ROM;
	MPtr nucleus = xroot->Nucleus;
	word waitg = idrom->WAIT_G;
	word pwaitg = idrom->PWAIT_G;
	
	struct { MPtr base; word size; word type; } first,second,tmp;
#define StrCpy(to,from) to.base = from.base;to.size = from.size;to.type = from.type;

	first.base	= idrom->GBASE0;
	first.size	= idrom->GSIZE0;
	first.type	= RAMType_Global|RAMType_Indirect;
	first.type     |= (((waitg&0x0F)!=(pwaitg&0x0F))?
				RAMType_Dynamic:RAMType_Static);

	second.base	= idrom->GBASE1;
	second.size	= idrom->GSIZE1;
	second.type	= RAMType_Global|RAMType_Indirect;
	second.type    |= (((waitg&0xF0)!=(pwaitg&0xF0))?
				RAMType_Dynamic:RAMType_Static);

	/* If first.base does not exist, shuffle second base down	*/
	if( first.base == 0xFFFFFFFF || first.size == 0 )
	{
		StrCpy(first,second);
		second.base = 0xFFFFFFFF;
		second.size = 0;
		second.type = 0;
	}

	/* if the strobes are reversed, swap over the two areas		*/
	if( first.base > second.base )
	{
		StrCpy(tmp,first);
		StrCpy(first,second);
		StrCpy(second,tmp);
	}

	pool = (Pool *)Allocate(sizeof(Pool),root->FreePool,&root->SysPool);
	
	InitPool(pool);

	/* Put first global RAM block into pool				*/
	if( first.base != 0xFFFFFFFF && first.size != 0 )
	{
		MPtr b;

		/* Allow for the nucleus being loaded at bottom of this	*/
		/* area of memory					*/
		if( first.base == nucleus )
		{
			first.base += MWord_(nucleus,0)/sizeof(word);
			first.size -= MWord_(nucleus,0)/sizeof(word);
		}

		/* Adjust base and size to multiples of 16 words. Note	*/
		/* that we have to be careful with the size.		*/
		b = first.base;
		first.base = (first.base + 15) &~15;
		first.size = ((b+first.size)-first.base) & ~15;

		/* Create a local RAM carrier for this block		*/
		carrier = (Carrier *)Allocate(sizeof(Carrier),root->FreePool,pool);

		/* Install address, size and type			*/
		carrier->Addr = first.base;
		carrier->Size = first.size;	
		carrier->Type = first.type;
		
		/* Set Carrier bit in carrier				*/
		(((Memory *)carrier) - 1)->Size |= Memory_Size_Carrier;

		root->SpecialPools[RAMTypeIndex(first.type)] = pool;
	}


	/* Put second Global RAM block into pool.			*/
	/* The order of these insertions is important to ensure that	*/
	/* The pool is ordered by acending addresses.			*/
	if( second.base != 0xFFFFFFFF && second.size != 0 )
	{
		MPtr b;

		/* If blocks are different types, save first pool and 	*/
		/* make a new one for the second block.			*/
		if( first.type != second.type )
		{
			pool = (Pool *)Allocate(sizeof(Pool),root->FreePool,&root->SysPool);
			InitPool(pool);
		}
		
		/* Allow for the nucleus being loaded at bottom of this	*/
		/* area of memory					*/
		if( second.base == nucleus )
		{
			second.base += MWord_(nucleus,0)/sizeof(word);
			second.size -= MWord_(nucleus,0)/sizeof(word);
		}

		/* Adjust base and size to multiples of 16 words. Note	*/
		/* that we have to be careful with the size.		*/
		b = second.base;
		second.base = (second.base + 15) &~15;
		second.size = ((b+second.size)-second.base) & ~15;

		/* Create a local RAM carrier for this block		*/
		carrier = (Carrier *)Allocate(sizeof(Carrier),root->FreePool,pool);

		/* Install address and size				*/
		carrier->Addr = second.base;
		carrier->Size = second.size;	
		carrier->Type = second.type;
		
		/* Set Carrier bit in carrier				*/
		((Memory *)carrier - 1)->Size |= Memory_Size_Carrier;	

		root->SpecialPools[RAMTypeIndex(second.type)] = pool;
	}



}
#endif

/*}}}*/
/*{{{  AllocSpecial */
#ifdef USESPECIAL
/*--------------------------------------------------------
-- AllocSpecial						--
--							--
-- This is a generic allocator for all memory which is	--
-- managed indirectly via Carriers. The type indicates	--
-- the properties that the memory should have.		--
--							--
--------------------------------------------------------*/

Carrier *AllocSpecial(word size, word type, Pool *pool)
{
	return (Carrier *)System((WordFnPtr)AllocIndirect,size,type,pool);
}

#endif
/*}}}*/
/*{{{  AllocIndirect */

#if defined(ONCHIPRAM) || defined(GLOBALRAM)
/*--------------------------------------------------------
-- AllocIndirect					--
--							--
-- Allocate memory which is accessed via a carrier.	--
--							--
--------------------------------------------------------*/

static Carrier *AllocIndirect(word size, word type, Pool *pool)
{
	RootStruct *root = GetRoot();
	MC *carrier = NULL;
	Pool *src = root->SpecialPools[RAMTypeIndex(type)];

	if( src == NULL ) return NULL;

/*	size = (size + 15) & ~15;	/ * adjust to multiple of 16 units */

	carrier = Head_(MC,src->Memory);
	
	while( !EndOfList_(carrier) )
	{
		word bsize = carrier->c.Size;

		if( bsize >= size )
		{
			if( bsize-size > 16 )
			{
				/* split the block */
				MC *next = (MC *)Allocate(sizeof(Carrier),
							   root->FreePool,
							   src);
				if( next == NULL ) return NULL;
				next = (MC *)((word)next - sizeof(Memory));
				Remove(&next->m.Node);
				PostInsert(&carrier->m.Node,&next->m.Node);
				next->c.Size = bsize-size;
				next->c.Addr = carrier->c.Addr + size;
				next->c.Type = carrier->c.Type;
				next->m.Size |= Memory_Size_Carrier;
				bsize = carrier->c.Size = size;
			}
			else
			{
				/* allocate whole block, just alter pool count */
				src->Blocks--;
			}
			/* after that, carrier is the block to be allocated */

			/* remove from Pool */
			Remove(&carrier->m.Node);
			src->Size -= sizeof(MC);

			/* add to dest pool */
			pool->Blocks++;
			pool->Size += sizeof(MC);
			carrier->m.Pool = pool;
			AddTail(&pool->Memory,&carrier->m.Node);

			return &carrier->c;
		}
		carrier = Next_(MC,carrier);
	}

	return NULL;
}

#endif

/*}}}*/
/*{{{  FreeIndirect */
#ifdef USESPECIAL
/*--------------------------------------------------------
-- FreeIndirect						--
--							--
-- Release memory which is accessed via a carrier.	--
--							--
--------------------------------------------------------*/

void FreeIndirect(MC *carrier)
{
	bool inpool = false;
	MC *prev, *next;
	RootStruct *root = GetRoot();
	Pool *freepool = root->SpecialPools[RAMTypeIndex(carrier->c.Type)];

	if( freepool == 0 ) return;
	
	prev = (MC *)&(freepool->Memory);
	next = Next_(MC,prev);

	while( next != NULL )
	{
		if( (next->m.Node.Next == NULL) || 
		    (next->c.Addr > carrier->c.Addr ) )
		{
			/* it goes here between next and prev */
			
			/* try to coalesce with prev */
			if( (prev->m.Node.Prev != NULL) &&
				prev->c.Addr+prev->c.Size == carrier->c.Addr )
			{
				prev->c.Size += carrier->c.Size;
				carrier->m.Size &= ~Memory_Size_Carrier;
				_Free(&carrier->m+1,root->FreePool);
				carrier = prev;
				inpool = true;
			}
			
			/* now try to coalesce with next	*/
			if( next->m.Node.Next != NULL &&
			    carrier->c.Addr+carrier->c.Size == next->c.Addr )
			{
				next->c.Addr = carrier->c.Addr;
				next->c.Size += carrier->c.Size;
				carrier->m.Size &= ~Memory_Size_Carrier;
				_Free(&carrier->m+1,root->FreePool);
				inpool = true;
			}
			
			if( !inpool )
			{
				Pool *pool = carrier->m.Pool;

				Remove(&carrier->m.Node);
				pool->Blocks--;
				pool->Size -= sizeof(MC);
				
				PostInsert(&prev->m.Node,&carrier->m.Node);
				
				freepool->Blocks++;
				freepool->Size += sizeof(MC);
				carrier->m.Pool = freepool;
			}
			return;
		}
		prev = next;
		next = Next_(MC,next);
	}
}
#endif
/*}}}*/
/*{{{  AllocMem */


/*--------------------------------------------------------
-- AllocMem						--
--							--
-- External memory allocator				--
--							--
--------------------------------------------------------*/

void *AllocMem(word size, Pool *pool)
{
	RootStruct *root = GetRoot();
#ifdef __MI
	void *v = (void *)System((WordFnPtr)Allocate,size,root->FreePool,pool);
	if (v) return v;
	MICompact(root->MISysMem);
#endif
	return (void *)System((WordFnPtr)Allocate,size,root->FreePool,pool);
}
/*}}}*/
/*{{{  FreeMem */
/*--------------------------------------------------------
-- FreeMem						--
--							--
-- External memory freeer				--
--							--
--------------------------------------------------------*/

word FreeMem(void *mem)
{

#ifdef USESPECIAL
	Memory *m = (Memory *)((word)mem - sizeof(Memory));

	if( (m->Size & Memory_Size_Carrier) != 0 )
		return System((WordFnPtr)FreeIndirect,m);
#endif		
	return System((WordFnPtr)_Free,mem,GetRoot()->FreePool);
}
/*}}}*/
/*{{{  FreeMemStop */

/*--------------------------------------------------------
-- FreeMemStop						--
--							--
-- Free Memory and halt current process. Used for 	--
-- freeing process stacks.				--
--							--
--------------------------------------------------------*/

void _FreeMemStop(void *mem)
{
	RootStruct *root = GetRoot();

#ifdef USESPECIAL
	Memory *m = (Memory *)((word)mem - sizeof(Memory));

	if( (m->Size & Memory_Size_Carrier) != 0 )
		FreeIndirect((MC *)m);
	else
#endif		
		_Free((Memory *)mem,root->FreePool);

	/* on return from Free we will be running in free memory */
	/* This is no problem because we are running at hi pri	 */
	/* so no-one else can allocate it until we stop.	 */
	Stop();
}

void FreeMemStop(void *mem)
{
	System((WordFnPtr)_FreeMemStop,mem);
}

/*}}}*/
/*{{{  InitPool */

/*--------------------------------------------------------
-- InitPool						--
--							--
-- Initialise a memory pool to empty			--
--							--
--------------------------------------------------------*/

void InitPool(Pool *pool)
{
	InitList(&pool->Memory);
	pool->Blocks = 0;
	pool->Size = 0;
	pool->Max = 0;
}
/*}}}*/
/*{{{  FreePool */
/*--------------------------------------------------------
-- FreePool						--
--							--
-- Release entire pool					--
--							--
--------------------------------------------------------*/

void _FreePool(Pool *pool);

void FreePool(Pool *pool)
{
	System((WordFnPtr)_FreePool,pool);
}

void _FreePool(Pool *pool)
{
	RootStruct *root = GetRoot();
	while( !EndOfList_(pool->Memory.Head) )
	{
		Memory *block = Head_(Memory,pool->Memory);

#ifdef USESPECIAL

		if( (block->Size & Memory_Size_Carrier) != 0 )
			FreeIndirect((MC *)block);
		else
#endif		
		
		_Free(block+1,root->FreePool);
		
	}
}
/*}}}*/
/*{{{  InPool */
/*--------------------------------------------------------
-- inpool						--
-- InPool						--
--							--
-- test whether a given memory block is contained in	--
-- a given pool.					--
--							--
--------------------------------------------------------*/

bool inpool(void *addr, Pool *pool)
{
	Memory *block = Head_(Memory,pool->Memory);
	
	while( !EndOfList_(block) )
	{
		void *start, *end;

#ifdef USESPECIAL		
		if( (block->Size & Memory_Size_Carrier) != 0 )
		{
			/* If this is a carried block, indirect to the	*/
			/* RAM carried.					*/
			Carrier *c = (Carrier *)(block+1);

#ifdef __C40
			if( (c->Type & RAMType_Direct) == 0 )
			{
				/* We can only check directly addressable */
				/* blocks here. 			  */
				block = Next_(Memory,block);
				continue;
			}
			else
			{
				start = C40CAddress(c->Addr);
				end = (void *)((word)start + c->Size);
			}
#else

			start = (void *)c->Addr;
			end = (void *)((word)start + c->Size);
#endif

		}
		else
#endif
		{
			start = (void *)block;
			end = (void *)((byte *)block + (block->Size & ~Memory_Size_BitMask));
		}
		
		if( start <= addr && addr <= end ) return TRUE;
		
		block = Next_(Memory,block);
	}
	
	return FALSE;
}

bool InPool(void *addr, Pool *pool)
{
	return (bool)System(inpool,addr,pool);
}
/*}}}*/

/*{{{  NewWorker */
/*--------------------------------------------------------
-- NewWorker						--
--							--
-- Create new kernel worker process.			--
--							--
--------------------------------------------------------*/

void EndWorker(void);

bool NewWorker(VoidFnPtr fn, ... )
{
	word *stack;
	va_list a;
	word a1, a2;
	
	va_start(a,fn);
	a1 = va_arg(a,word);
	a2 = va_arg(a,word);
	va_end(a);

	stack = (word *)GetBuf(Worker_stacksize);
	
	if( stack == NULL ) return false;

	{
		word *s = &stack[Worker_stacksize/sizeof(word)];
		word *w;

#ifndef __TRAN
		/* cheat a little and set modtab to stack base */
		/* we can then pull this out easily with _GetModTab */
		/* when we want to free the stack in EndWorker */
		/* (we pass the stack base as the descriptor) */
		stack[0] = (word)stack;	/* set bogus module table pointer */
		stack[1] = (word)stack; /* set stack base */
#endif
		/* NewWorker doesn't set up modtab pointer properly so */
		/* workers should only use fns within kernel,  */
		/* and NO static data */

#ifdef __ARM
		/* On the ARM automatically create the workers in SVC mode */
		{
			fncast f, x;

			f.vfn = fn;
			f.w |= SVCMode;

			/* EndWorker must also have SVCMode set so that */
			/* The dispatcher entered as a result of its Stop() */
			/* will RestoreSlicedState correctly, even if this is */
			/* Swapping to a SVC mode thread. */
			x.vfn = EndWorker;
			x.w |= SVCMode;

			w = CreateProcess(s, f.vfn, x.vfn, stack, 8);
		}
#else
		w = CreateProcess(s,fn,EndWorker,stack,8);
#endif
			
		w[0] = a1;
		w[1] = a2;
		
		EnterProcess(w,0);
	}

	return true;
}
/*}}}*/
/*{{{  EndWorker */
void EndWorker(void)
{
#ifdef __TRAN
	/* EndWorker is entered with the stack pointing to the end of	*/
	/* the stack. The following strucuture maps onto the entry	*/
	/* stack frame built by NewWorker.				*/
	/* Note that this code is a bit dodgy, we return from FreeBuf	*/
	/* using a stack which is now in free memory. But since we are	*/
	/* running at high priority, noone can get in to allocate it.	*/
	struct { word ret, stack, arg1, arg2; } frame;
	FreeBuf((Buffer *)frame.stack);

#else
	/* To get hold of the stack base (currently masquerading as the */
	/* modtab), we simply use the kernel asm fn _GetModTab() */
	FreeBuf((Buffer *)_GetModTab());
#endif
	Stop();
}
/*}}}*/
/*{{{  GetBuf */

/*--------------------------------------------------------
-- GetBuf						--
-- FreeBuf						--
--							--
-- Kernel buffer management. These buffers are used	--
-- both as message buffers and as worker process stacks.--
--							--
--------------------------------------------------------*/

Buffer *GetBuf(word size)
{
	RootStruct *root = GetRoot();
	Buffer *buf = NULL;

	/* Stop Kernel allocations when less than 10% of available memory left */
	if( root->BufferCount > root->FreePool->Size*9 ) return NULL;

	size += sizeof(Buffer)-sizeof(((Buffer *)0)->Buf);

	if( size <= sizeof(Buffer) )
		buf = (Buffer *)RemHead(&root->BufferPool);
	
	if( buf == NULL )
	{
		if (size > 2048)
			buf = (Buffer *)Allocate(size,root->FreePool,&root->SysPool);
		else
			buf = (Buffer *)LowAllocate(size, root->FreePool, &root->SysPool);
		
		if( buf == NULL ) return NULL;

		buf->Type = Buffer_Type_Special;
	}

	root->BufferCount += ((Memory *)buf-1)->Size & ~Memory_Size_BitMask;
	if( root->BufferCount > root->MaxBuffers ) root->MaxBuffers = root->BufferCount;

	return buf;
}

/*}}}*/
/*{{{  FreeBuf */

void FreeBuf(Buffer *buf)
{
	RootStruct *root = GetRoot();

	root->BufferCount -= ((Memory *)buf-1)->Size & ~Memory_Size_BitMask;
	
	if( buf->Type != Buffer_Type_Cache )
		_Free((Memory *)buf,root->FreePool);
	else 
	{
		AddTail(&root->BufferPool,&buf->Node);
	}
}

/*}}}*/

/*{{{  Extensions */

/*--------------------------------------------------------
 * Memory system extensions added:
 *
 * Additions for st80. The st80 additions are of general use
 * and include statmem(), realloc() and LowAllocate() functions.
 * Relocatable blocks via Memory Indirection (__MI).
 * Reset proof (Robust) Ram Disks (__RRD).
 * Insertable memory Cards (__CARD).
 *
 *--------------------------------------------------------*/

/*{{{  LowAllocMem */
/*--------------------------------------------------------
-- LowAllocMem						--
--							--
-- External memory allocator for LowAllocate		--
--							--
--------------------------------------------------------*/

void *LowAllocate(word size, Pool *source, Pool *dest);

void *LowAllocMem(word size, Pool *pool)
{
	RootStruct *root = GetRoot();
# ifdef __MI
	void *v = (void *)System((WordFnPtr)LowAllocate,size,root->FreePool,pool);
	if (v)
		return v;
	/* if alloc fails, try to de-fragament and compact memory */
	MICompact(root->MISysMem);
# endif
	return (void *)System((WordFnPtr)LowAllocate,size,root->FreePool,pool);
}

/*--------------------------------------------------------
-- LowAllocate						--
--							--
-- Allocate from the given free source pool into the	--
-- given destination pool. Allocating memory from the	--
-- The base of the free pool rather than the top.	--
--							--
--------------------------------------------------------*/

void *LowAllocate(word size, Pool *source, Pool *dest)
{
	Memory *block;
	word bsize;
	Memory *start = Head_(Memory, source->Memory);

	/* add size of Memory header and up to next 16 bytes */
	size = (size + 31) & ~15;
	
	/* start from end of list i.e. low memory first */
	block = Tail_(Memory,source->Memory);
	
	/* Test is valid as head block is never allocated */
	while( block != start )
	{
		bsize = block->Size & ~Memory_Size_BitMask;

		if( bsize >= size )
		{
			if( bsize-size <= Epsilon )
			{
				/* sizes are close, alloc whole block */
				Remove(&block->Node);
				source->Blocks--;
			}
			else
			{
				/* in LowAllocate, the high portion of the */
				/* block is retained as the free space, and */
				/* the low portion is allocated */
				Memory *HiBlock = (Memory *)((word)block+size);

				HiBlock->Pool = source; /* note owner */

				/* subdivide this block */
				bsize -= size;
				
				/* set new size of hi half */
				HiBlock->Size = bsize;
						
				/* set new back pointer */
				((word *)HiBlock)[(bsize/sizeof(word))-1] = (word)HiBlock;
				
				/* re-organise free list */
				/* PreInsert(block->Node, HiBlock->Node) */
				/* Remove(block->Node) */
				HiBlock->Node.Next = block->Node.Next;
				HiBlock->Node.Prev = block->Node.Prev;
				HiBlock->Node.Next->Prev = &HiBlock->Node;
				HiBlock->Node.Prev->Next = &HiBlock->Node;
				
				/* set size of low portion */
				bsize = size;
				block->Size = (block->Size & Memory_Size_BitMask) + bsize;
			}
			
			/* here block points to the block to allocate */
			/* set its forward bit and successors backward bit */
			/* FwdBit = this block is allocated */ 
			/* BwdBit = previous (lower mem) block is allocated */
			block->Size |= Memory_Size_FwdBit;
			start = (Memory *)((word)block+bsize);
			start->Size |= Memory_Size_BwdBit;

			/* move block into dest pool */
			block->Pool = dest;	
			AddTail(&dest->Memory,&block->Node);

			dest->Blocks++;
			dest->Size += bsize;
			
			/* adjust source pool size */
			source->Size -= bsize;

#ifdef __TRAN
			ZeroBlock(block+1,bsize-sizeof(Memory));
#endif
			
			/* return address of first free byte */
			return (void *)(block+1);
		}

		block = Prev_(Memory,block);
	}

	return NULL;
}

/*}}}*/

#ifndef __TRAN
/*{{{  ReAllocate */
#ifdef WASTE_OF_TIME

/*--------------------------------------------------------
-- ReAllocate						--
--							--
-- Extend or contract existing block's size if possible	--
--							--
--------------------------------------------------------*/

bool ReAllocate(word newsize, Memory *block)
{
	Memory *next;
	word	oldsize = (--block)->Size & ~Memory_Size_BitMask;
	word	delta;
	RootStruct *root = GetRoot();

	if (newsize > oldsize) /* expand block */
	{
		word nsize;

		next = (Memory *)((word)block + oldsize);
		nsize = next->Size & ~Memory_Size_BitMask;

		/* align size up to 16 byte boundary */
		newsize = (newsize + 15) & ~15;

		delta = newsize - oldsize;

		/* check if next block is free and large enough */
		if ( (next->Size & Memory_Size_FwdBit)
		   || (nsize < delta) )
			return FALSE;

		if (nsize-delta > Epsilon) /* contract free block to make room */
		{
			Memory *next2 = (Memory *)((word)next + delta);

			/* contract free block (keeping flags) */
			next2->Size = next->Size - delta;

			/* set free blocks new back pointer */
			((word *)next2)[((next2->Size & ~Memory_Size_BitMask)
			/ sizeof(word)) - 1] = (word)next2;
			
			next2->Pool = root->FreePool; /* set owner */

			/* re-organise free list */
			/* PreInsert(next2->Node, next->Node) */
			/* Remove(next->Node) */
			next2->Node.Next = next->Node.Next;
			next2->Node.Prev = next->Node.Prev;
			next2->Node.Next->Prev = &next2->Node;
			next2->Node.Prev->Next = &next2->Node;
		}
		else /* if remaining space is tiny, subsume entire block */
		{
			delta = next->Size & ~Memory_Size_BitMask;
			Remove(&next->Node);
			root->FreePool->Blocks--;
		}

		/* extend original block */
		block->Size += delta;
		block->Pool->Size += delta;

		root->FreePool->Size -= delta;

		return TRUE;
	}
	elif (newsize < oldsize) /* contract */
	{
		/* adjust size down to 16 byte boundary */
		delta = (oldsize - newsize) & ~15;

		/* not worth freeing this amount of memory */
		/* @@@ N.B. if next block was free, then it might? */
		if (delta <= Epsilon)
			return TRUE; /* we did (Honest!) */

		/* shrink original block (preserving flags) */
		block->Size -= delta;
		block->Pool->Size -= delta;

		/* create new block at end */
		next = (Memory *)((word)block + (block->Size & ~Memory_Size_BitMask));

		/* move new block to free pool */
		{
			Memory *next2 = (Memory *)((word)next + delta);

			/* if successor is free, coalesce with it */
			if( (next2->Size & Memory_Size_FwdBit) == 0 )
			{
				/* combine sizes */
				next->Size = delta + next2->Size; /* assumes next2 will have BwdBit set */
		
				/* set new back pointer */
				((word *)next)[(
				(next->Size&~Memory_Size_BitMask)/sizeof(word))
							-1] = (word)next;

				/* usurp its place in the free pool list */
				next->Node.Next = next2->Node.Next;
				next->Node.Prev = next2->Node.Prev;
				next->Node.Next->Prev = &next->Node;
				next->Node.Prev->Next = &next->Node;
			}
			else
			{
				/* cannot coalesce with successor */
				/* so reset successor's status flag */
				next2->Size &= ~Memory_Size_BwdBit;

				/* both neighbours are allocated, so search */
				/* free pool list for our correct position. */
				next2 = Head_(Memory,root->FreePool->Memory);
		
				while( !EndOfList_(next2) )
				{
					if( (word)next2 < (word)next ) break;
					next2 = Next_(Memory,next2);
				}

				PreInsert(&next2->Node,&next->Node);
				root->FreePool->Blocks++;
				
				/* set new block's size and back pointer */
				next->Size = delta | Memory_Size_BwdBit;
				((word *)next)[(delta/sizeof(word))-1] = (word)next;
			}
		}

		next->Pool = root->FreePool;
		root->FreePool->Size += delta;
	}

	return TRUE;
}
#endif
/*}}}*/
/*{{{  _StatMem */

/*--------------------------------------------------------
-- StatMem						--
--							--
-- System memory statistics				--
--							--
--------------------------------------------------------*/

static word _StatMem(word *totalsize, word *totalfree, word *largestfree)
{
	Memory *m = Head_(Memory,GetRoot()->FreePool->Memory);
	word i, max = 0;

	*totalsize = GetRoot()->FreePool->Max;
	*totalfree = GetRoot()->FreePool->Size;

	while( !EndOfList_(m) ) {
		i = m->Size & ~Memory_Size_BitMask;
		max = i > max ? i : max;
		m = Next_(Memory, m);
	}
	*largestfree = max;

	return max;
}
/*}}}*/
/*{{{  TrimAlloc */
# ifdef __MI
/*
 * TrimAlloc() is only used by MITrim() at present.
 * it is  not exported
 */

/*--------------------------------------------------------
-- TrimAlloc 						--
--							--
-- Trim from the start of the memory block. This can 	--
-- only	release memory, not increase it. (A sort of	--
-- realloc from the other end). The amount to trim the	--
-- blocks size by must be a multiple of 16 and be of	--
-- at least 'Epsilon' size.				--
--							--
-- Currently is only used by the RRD to trim the size	--
-- of ram disk blocks that are being read destructively.--
--							--
--------------------------------------------------------*/

void *_TrimAlloc(word amount, Memory *block)
{
	RootStruct *root = GetRoot();
	Memory *newstart = (Memory *) ((word *)(--block) + amount/sizeof(word));

	if ( (amount & ~15) != amount || amount < Epsilon
	   || amount >= ((block->Size & ~Memory_Size_BitMask) - sizeof(Memory)))
		return NULL;

	/* adjust remaining data to new size and relink into Pool */
	/* preserve original flags */
	newstart->Size = (block->Size - amount) & ~Memory_Size_BwdBit;
	newstart->Pool = block->Pool;
	block->Pool->Size -= amount;

	/* correct list pointers to new pos */
	newstart->Node.Next = block->Node.Next;
	newstart->Node.Prev = block->Node.Prev;
	newstart->Node.Next->Prev = &newstart->Node;
	newstart->Node.Prev->Next = &newstart->Node;

	if (block->Size & Memory_Size_BwdBit)
	{
		/* Block below is allocated, so create new free block */
		Memory *next;

		/* set new block's size and back pointer */
		block->Size = amount | Memory_Size_BwdBit;
		((word *)block)[(amount/sizeof(word))-1] = (word)block;

		/* search free pool list for our correct position. */
		next = Head_(Memory,root->FreePool->Memory);
		while( !EndOfList_(next) )
		{
			if( (word)next < (word)block) break;
			next = Next_(Memory,next);
		}

		/* insert into free pool */
		PreInsert(&next->Node,&block->Node);

		/* set owner and inc free pool counts */
		block->Pool = root->FreePool;
		root->FreePool->Blocks++;
	}
	else
	{
		/* Merge new free area with free block below */

		/* follow backpointer */
		Memory *next = (Memory *) *((word *)block - 1);

		/* combine sizes (preserving flags) */
		next->Size += amount;
		
		/* set new back pointer */
		((word *)next)[((next->Size&~Memory_Size_BitMask)/sizeof(word))-1] = (word)next;
	}

	root->FreePool->Size += amount;

	return (++newstart);
}
# endif /* __MI */
/*}}}*/
/*{{{  ReAllocMem */
#if 0
/*--------------------------------------------------------
-- ReAllocMem						--
--							--
-- External memory re-allocator				--
--							--
--------------------------------------------------------*/

bool ReAllocMem(word newsize, void *block)
{
	return (bool)System((WordFnPtr)ReAllocate,newsize,block);
}
#else
bool ReAllocMem(word newsize, void *block) { return 0; }
#endif
/*}}}*/
/*{{{  TrimAllocMem */
# if 0
/* also add prototype to <memory.h> if this is used: */
PUBLIC void *TrimAllocMem(WORD newsize, void *block);
/*--------------------------------------------------------
-- TrimAllocMem						--
--							--
-- External memory re-allocator (works from other end	--
-- compared with ReAllocMem (i.e. the front).		--
-- The amount must be a multiple of 16 and be greater	--
-- than 2*sizeof(struct Memory).			--
--							--
--------------------------------------------------------*/
void *TrimAllocMem(word amount, void *block)
{
	return ((void *)System((WordFnPtr)_TrimAlloc,amount,block));
}
# endif
/*}}}*/
/*{{{  StatMem */
/*--------------------------------------------------------
-- External StatMem interface				--
--							--
-- system memory statistics				--
--							--
--------------------------------------------------------*/

word StatMem(word *totsize, word *totfree, word *largestfree, word *percfree)
{
	word	a = System(_StatMem, totsize, totfree, largestfree);

	*percfree = *totfree * 100UL / *totsize;

	return	a;
}
/*}}}*/
#endif

/*}}}*/
/*{{{  Relocatable Memory */
#ifdef __MI
/*--------------------------------------------------------
-- Relocateable Memory Manager				--
--							--
-- Creates and manipulates dynamically allocated blocks	--
-- These blocks can then be moved without the users	--
-- consent (unless they are locked). The main user is	--
-- the Robust RAM filing system.			--
--							--
-- Relocateable memory blocks are denoted by:		--
-- 	Memory->Size & Memory_Size_Reloc		--
--							--
--------------------------------------------------------*/

/*{{{  _MIAlloc */
/*--------------------------------------------------------
-- Allocate a relocatable block of memory.		--
--							--
--------------------------------------------------------*/
Handle _MIAlloc(MIInfo *mi, word size)
{
	word **MItable = mi->MITable;

	if (mi->MIWriteProtect) {
#ifdef SYSDEB
		KDebug("Attempt to MIAlloc write protected area");
#endif
		return NULL;
	}

	if (mi->MIFreeSlots == 0) {
		if (MItable == NULL) {
			/* Initialise MI indirection table */
			if ((mi->MITable = Allocate(MIINIT*sizeof(word), mi->FreePool, mi->DstPool)) == NULL) {
				KDebug("Initial Allocate failed\n");
				return 0;
			}
			ZeroBlock(mi->MITable,MIINIT * sizeof(word));
			mi->MITableSize = MIINIT;
			mi->MIFreeSlots = MIINIT -1; /* -1 to avoid allocating slot zero == NULL */
		}
		else
		{
			/* Increase indirection table's size */

			if (!ReAllocate((mi->MITableSize + MIINC) * sizeof(word), (Memory *)MItable)) {
				word **MItableold = mi->MITable;
				word **MItable2;
				word i;

				if ((MItable2 = Allocate((mi->MITableSize + MIINC) * sizeof(word), mi->FreePool, mi->DstPool)) == NULL)
					return 0;

				mi->MITable = MItable2;

				/* copy existing handles to new table */
				for (i = mi->MITableSize-1; i > 0; --i)
					*(++MItable2) = *(++MItable);

				_Free((Memory *)MItableold, mi->FreePool);
			}

			/* ReAlloc Worked */
			/* Mark new slots as unused */
			ZeroBlock((char *)((word *)mi->MITable + mi->MITableSize), MIINC * sizeof(word));
			mi->MIFreeSlots = MIINC;
			mi->MITableSize += MIINC;
		}

			/* Easy allocate of last slot just allocated */
		{
			word *addr;

			if ((addr = mi->MITable[mi->MITableSize-1] = Allocate(size, mi->FreePool, mi->DstPool)) == NULL)
				return 0;
			mi->MIFreeSlots--;
			((Memory *)addr - 1)->Size |= Memory_Size_Reloc;
			return (mi->MITableSize-1);
		}
	}

	/* allocate a relocatable block */

	/* search for free table entry (always decending from top of table)*/
	MItable += mi->MITableSize;
	while (*(--MItable) != NULL) ; /* null stat. */

	if((*MItable = Allocate(size, mi->FreePool, mi->DstPool)) == NULL)
		return 0;

	mi->MIFreeSlots--;

	/* note that block is relocatable */
	((Memory *)*MItable - 1)->Size |= Memory_Size_Reloc;

	return (((word)MItable - (word)(mi->MITable)) / sizeof(word));
}

/*}}}*/
/*{{{  _MIFree */
/*--------------------------------------------------------
--							--
-- Free relocateable blocks of memory.			--
--							--
--------------------------------------------------------*/
void _MIFree(MIInfo *mi, Handle handle)
{

	if (handle < 1 || handle > mi->MITableSize) {
		KDebug("MIFree handle error\n");
		return;
	}

	if (mi->MIWriteProtect) {
#ifdef SYSDEB
		KDebug("Attempt to free write protected area");
#endif
		return;
	}

	_Free((Memory *)mi->MITable[handle], mi->FreePool);
	mi->MITable[handle] = NULL;
	mi->MIFreeSlots++;
}

/* Make MIHandle for a relocatable block point to a new position */
void MIMoveHandle(MIInfo *mi, void *old, void *new)
{
	word **MItable = mi->MITable;

#if 0
	/* @@@ if extra speed is required, use this comparison */
	MItable += mi->MITableSize;

	while (*(--MItable) != old) ; /*null stat*/

	*MItable = new;
#else
	word i = mi->MITableSize;
	
	while (--i) {
		if (MItable[i] == old) {
			MItable[i] = new;
			return;
		}
	}

	DBOutput("MIMoveHandle: invalid old handle address: %x\n"(word)old);
#endif
}
/*}}}*/
/*{{{  nextbase */
/* Get next relocatable block in memory */
/* Return NULL if no more blocks */
Memory *nextbase(Memory *block, Memory *poolend)
{
	do {
		block = (Memory *)((word)block + (block->Size & ~Memory_Size_BitMask));
		if (block >= poolend)
			return NULL;
	} while(!(block->Size & Memory_Size_Reloc));

	return block;
}
/*}}}*/
/*{{{  findfirstfit */
/* Find first reloc block after base block that fits size */
/* If no blocks of equal to or less than the desired size, return NULL */
Memory *findfirstfit(Memory *start, Memory *end, word size)
{
	forever {
		start = (Memory *)((word)start + (start->Size & ~Memory_Size_BitMask));
		if (start >= end)
			return NULL;
		if (start->Size & Memory_Size_Reloc)
 			/* first fit test */
			if ((start->Size & ~Memory_Size_BitMask) <= size)
				return start;
	}
}
/*}}}*/
/*{{{  _MICompact */
/* to speed up compaction ignore free areas smaller than this */
#define	SIZELIMIT 32	/* < sizeof(RRDFileBlock) */

/*--------------------------------------------------------
--							--
-- Cause all relocateable blocks to be compacted towards--
-- High memory.						--
--							--
-- Finding free blocks from high to low memory,		--
-- move relocatable blocks starting from low memory	--
-- into these higher memory Gaps.			--
--							--
--------------------------------------------------------*/
#define FirstPoolBlock(pool) ((Memory *)((word)((pool) + 1) + ((sizeof(Pool) + sizeof(MIInfo) + 15) & ~15)))

void _MICompact(MIInfo *mi)
{
	Memory	*poolend = (Memory *)(mi->FreePool->Size + (word)(FirstPoolBlock(mi->FreePool)));
#if 0
	/* removed this as scavenged blocks may occur before dummy block */
	Memory	*poolend = Head_(Memory,mi->FreePool->Memory); /* dummy block */
	Memory	*freeblock = Next_(Memory,poolend);  /* first free block */
#else
	/* first free block, stepping over dummy block */
	Memory	*freeblock = Next_(Memory, (Head_(Node, mi->FreePool->Memory)));
#endif

	/* start search for reloc block to move from here */
	Memory	*baseblock = nextbase(FirstPoolBlock(mi->FreePool), poolend);
	Memory	*rblock, *nextfreeblock;

	if (mi->MIWriteProtect) {
#ifdef SYSDEB
		KDebug("Attempt to compact write protected area");
#endif
		return;
	}

	while (freeblock > baseblock && baseblock != NULL && !EndOfList_(freeblock)) {
		word fitsize = freeblock->Size & ~Memory_Size_BitMask;
		word rsize;
		
		if (fitsize < SIZELIMIT || (freeblock->Size & Memory_Size_FwdBit)) {
			freeblock = Next_(Memory,freeblock);
			continue;
		}

		if ((baseblock->Size & ~Memory_Size_BitMask) <= fitsize) {
			/* advance base block if possible */
			rblock = baseblock;
			baseblock = nextbase(baseblock,poolend);
		}
		else if ((rblock = findfirstfit(baseblock,freeblock,fitsize)) == NULL) {
			/* if no reloc block small enough to fit, try next */
			freeblock = Next_(Memory,freeblock);
			continue;
		}

		rsize = rblock->Size & ~Memory_Size_BitMask;

		/* similar code to  Allocate() - Allocate this freeblock */
		/* major differences are: no searching for block to alloc */
		/* and recording of next free block pos. */
		if( fitsize-rsize <= Epsilon ) {
			/* sizes are close, alloc whole block */
			mi->FreePool->Blocks--;
			nextfreeblock = Next_(Memory,freeblock);
			Remove(&freeblock->Node);
		}
		else
		{
			/* subdivide this block */
			fitsize -= rsize;
				
			/* set new size of old block - keep old alloc bits*/
			freeblock->Size = (freeblock->Size & Memory_Size_BitMask)
								+ fitsize;
			/* set new back pointer */				
			((word *)freeblock)[(fitsize/sizeof(word))-1] = (word)freeblock;

			/* remeber old but now smaller freeblock */
			nextfreeblock = freeblock;

			/* point to new block */
			freeblock = (Memory *)((word)freeblock+fitsize);
			freeblock->Size = fitsize = rsize;
		}

		/* here freeblock points to the block we want to allocate */
		/* set its forward bit and successors backward bit    */
		/* FwdBit = this block is allocated */ 
		/* BwdBit = previous block is allocated */
		freeblock->Size |= (Memory_Size_FwdBit | Memory_Size_Reloc);
		((Memory *)((word)freeblock+fitsize))->Size |= Memory_Size_BwdBit;

		/* move block into dest pool */
		freeblock->Pool = mi->DstPool;
		AddTail(&mi->DstPool->Memory, &freeblock->Node);

		mi->DstPool->Size += fitsize;
		mi->DstPool->Blocks++;

		/* adjust source pool size */
		mi->FreePool->Size -= fitsize;

		/* step past memory headers */
		++freeblock; ++rblock;

		/* relocate block to new position */
		MoveBlock(freeblock, rblock, rsize - sizeof(Memory));

		/* make MIHandle for reloc block point to new position */
		MIMoveHandle(mi, rblock, freeblock);

		/* free memory used for original rblock */
		_Free(rblock, mi->FreePool);
		
		freeblock = nextfreeblock;
	}
}

/*}}}*/
/*{{{  _MILock */
/*--------------------------------------------------------
--							--
-- Lock down a relocateable block and return its true	--
-- address.						--
-- Blocks must be locked down for the minimum possible	--
-- amount of time. Otherwise compaction could fail.	--
-- Care should be used if sharing blocks with other	--
-- threads as the lock/unlocking is not stacked		--
--							--
--------------------------------------------------------*/
void *_MILock(MIInfo *mi, Handle handle)
{
	void *addr;

	if (handle < 1 || handle > mi->MITableSize) {
		KDebug("MILock: invalid handle error\n");
		return NULL;
	}

	addr = mi->MITable[handle];

	/* if write protected, dont lock handle as this would alter mem */
	/* can't relocate it anyway! */
	if (mi->MIWriteProtect) {
		/*KDebug("Attempt to lock write protected area");*/
		return addr;
	}

	(((Memory *)addr)-1)->Size &= ~Memory_Size_Reloc;

	return addr;
}

/*}}}*/
/*{{{  _MIUnlock */
/*--------------------------------------------------------
--							--
-- Unlock a block and make it relocateable again.	--
--							--
--------------------------------------------------------*/
void _MIUnLock(MIInfo *mi, Handle handle)
{
#ifdef SYSDEB
	/* if write protected, don't alter memory in any way */
	if (mi->MIWriteProtect) {
		/*KDebug("Attempt to unlock write protected area handle");*/
		return;
	}

	if (handle == 0 || handle > mi->MITableSize) {
		KDebug("MIUnLock invalid handle error\n");
		return;
	}
#else
	if (handle == 0 || handle > mi->MITableSize || mi->MIWriteProtect)
		return;
#endif

	((Memory *)(mi->MITable[handle])-1)->Size |= Memory_Size_Reloc;

	return;
}
/*}}}*/
/*{{{  _MITrim */
/*--------------------------------------------------------
-- MITrim						--
--							--
-- The relocatable block will be reduced in size by the --
-- amount specified. In this case the block is reduced	--
-- from the front (i.e. a realloc from the other end).	--
-- The amount to trim the blocks by must be a multiple  --
-- of 16 and be of at least 'Epsilon' size, otherwise 	--
-- the system will probably crash.			--
--							--
-- Locked blocks will become unlocked.			--
--							--
--------------------------------------------------------*/
bool _MITrim(MIInfo *mi, Handle handle, word amount)
{
	void *addr = mi->MITable[handle];

#ifdef SYSDEB
	if (mi->MIWriteProtect) {
		KDebug("Attempt to compact write protected area");
		return FALSE;
	}
#endif

	if(handle < 1 || handle > mi->MITableSize) {
		KDebug("MITrim invalid handle error\n");
		return FALSE;
	}

	/* reset handle in table to new address */
	if((mi->MITable[handle] = _TrimAlloc(amount, (Memory *)addr)) == NULL)
	{
		KDebug("******** _TrimAlloc returned NULL!");
		mi->MITable[handle] = addr;
		return FALSE;
	}

	return TRUE;
}
/*}}}*/
/*{{{  External Interface */
/*--------------------------------------------------------
-- External interface to the Relocatable Memory Manager --
-- functions.						--
--------------------------------------------------------*/
Handle MIAlloc(MIInfo *mi, word size)
{
	return (System(_MIAlloc, mi, size));
}

void MIFree(MIInfo *mi, Handle handle)
{
	System((WordFnPtr)_MIFree, mi, handle);
}

void MICompact(MIInfo *mi)
{
	System((WordFnPtr)_MICompact, mi);
}

void *MILock(MIInfo *mi, Handle handle)
{
	return((void *)System((WordFnPtr)_MILock, mi, handle));
}

void MIUnLock(MIInfo *mi, Handle handle)
{
	System((WordFnPtr)_MIUnLock, mi, handle);
}

bool MITrim(MIInfo *mi, Handle handle, word amount)
{
	return (System((WordFnPtr)_MITrim, mi, handle, amount));
}

void MIInit(MIInfo *mi, Pool *free, Pool *dst)
{
	mi->MIWriteProtect = FALSE;
	mi->MITable = NULL;
	mi->MIFreeSlots = 0;
	mi->MITableSize = 0;
	mi->FreePool = free;
	mi->DstPool = dst;
}

/*}}}*/

#endif /* __MI */

#ifdef __RRD
/*{{{  RRDPoolInit */
/*--------------------------------------------------------
-- RRDPoolInit						--
--							--
-- Examine magic blocks saved by buildpool and 		--
-- handcraft them into the given pool. Then Place all	--
-- blocks in pool into the Memory Indirect (MI)		--
-- relocatable memory system.				--
--							--
--------------------------------------------------------*/
# ifdef __CARD
PUBLIC word RRDPoolInit(MIInfo *mi, Memory *magicbase, Memory *magictop)
# else
static word RRDPoolInit(MIInfo *mi, Memory *magicbase, Memory *magictop)
# endif
{
	Memory *mem = magicbase;
	word blocks = 0;
	word size = 0;
	word **MItable;
	word *rp;
	word blks;

	if (magicbase == magictop)
		return 0;

	while ( mem < magictop)
	{
		blocks++;
		size += mem->Size;

		/* set up attributes of blocks */
		mem->Size |= Memory_Size_BwdBit | Memory_Size_FwdBit | Memory_Size_Reloc;

		mem->Pool = mi->DstPool;

		AddHead(&mi->DstPool->Memory, &mem->Node);

		mem = (Memory *) ((word)mem + (mem->Size & ~Memory_Size_BitMask));
	}

	/* area before magic will be free */
	magicbase->Size &= ~Memory_Size_BwdBit;

	if (blocks)
		/* we have found some memory */
		/* so set bwds bit in dummy top block */
		magictop->Size |= Memory_Size_BwdBit;

	mi->DstPool->Blocks = blocks;
	mi->DstPool->Size = size;
	mi->DstPool->Max = 0;

	/*--------------------------------------------------------
	-- Place all blocks in pool into the Memory Indirect 	--
	-- (MI) relocatable memory system.			--
	--------------------------------------------------------*/

	/* allocate MI indirection table of at least MIINIT size */
	mi->MITableSize = (blocks + MIINIT + 1) & ~(word)(MIINC-1);
	mi->MIFreeSlots = mi->MITableSize - blocks - 1;
	MItable = mi->MITable = (word **) Allocate(mi->MITableSize * sizeof(word), mi->FreePool, mi->DstPool);

	mem = magicbase;

	/* copy addresses of blocks into indirection table */
	for (blks = blocks; blks != 0; mem = (Memory *)((word)mem + (mem->Size & ~Memory_Size_BitMask)), blks--)
	{
		rp = (word *)(mem + 1);
		MItable[blks] = rp;
	}

	return blocks;
}
/*}}}*/
/*{{{  Checksum */
/*--------------------------------------------------------
-- Checksum						--
-- 							--
-- Calculates checksum for given block.			--
--							--
-- Area checksumed starts from the headers TotalSize	--
-- element, to the end of the buffer. The end of the	--
-- buffer being specified by TotalSize.			--
--							--
--------------------------------------------------------*/
static word Checksum(word *start, word size)
{
	word sum = 0;

	size /= sizeof(word);

	while (size--)
		sum += *start++;

	return sum;
}
/*}}}*/
/*{{{  Scavenge */
/*--------------------------------------------------------
-- Scavenge						--
-- 							--
-- Find and Save magic marked blocks.			--
--							--
-- Memory blocks that are to be saved over a reset are	--
-- marked with a 'magic' header. These blocks are sought--
-- out and verified. If they are found to be self	--
-- consistent they are compacted to the top of memory.	--
-- The remaining memory's size is returned.		--
--							--
--------------------------------------------------------*/
static word Scavenge(word *bott, word size)
{
	register word *bottom = bott; /* force to reg */
	word *top = (word *)((word)bottom + size);
	word *curp = top;
	word *savepoint = top;

	while (--curp > bottom)
	{
		if ( (*curp & RRDCommonMagicMask) != RRDCommonMagic )
			continue; /* usual case */

		if (    *curp == RRDBuffMagic
		          || *curp == RRDFileMagic
		          || *curp == RRDDirMagic
		          || *curp == RRDSymbMagic )
		{
			word memsize, blksize, adjblksize;

			/* found a possible magic */
			memsize = ((Memory *)curp - 1)->Size & ~Memory_Size_BitMask;
			blksize = ((RRDCommon *)curp)->TotalSize;
			/* add mem header and 16 byte alignment */
			adjblksize = (blksize + 31) & ~15;

			/* Check that the size of the block is
			consistent and that the block is not overly
			large. Each blocks intra and inter block
			consistency will be further tested by the
			RRD */

			  /* compare with possible upper and lower   */
			  /* bounds of recorded memory size,         */
			if ( (memsize >= adjblksize)
			  && (memsize <= adjblksize + Epsilon)

			  /* see if it would be larger than life (or */
			  /* at least the available memory!)        */
			  && ((word)curp - sizeof(Memory) + adjblksize <= (word)top)

			  /* check that the size is consistent with  */
			  /* possible sizes for the blocks type.     */
			  && (  (*curp == RRDBuffMagic && blksize <= sizeof(RRDBuffBlock) + RRDBuffMax)
			     || (*curp == RRDFileMagic && blksize <= sizeof(RRDFileBlock) + PATH_MAX)
			     || (*curp == RRDDirMagic  && blksize <= sizeof(RRDDirBlock) + PATH_MAX)
			     || (*curp == RRDSymbMagic && blksize <= sizeof(RRDSymbBlock) + PATH_MAX * 2) ) )
			{
				/* block size is consistent with memory size */

				if ( ((RRDCommon *)curp)->Checksum == Checksum(&((RRDCommon *)curp)->TotalSize, blksize - offsetof(RRDCommon, TotalSize)) )
				{
					word cp_size, *b_end, *s_end;

					size -= adjblksize;

					s_end = savepoint;
					savepoint -= adjblksize / sizeof(word);

					/* dont copy Memory header */
					cp_size = (adjblksize - sizeof(Memory)) / sizeof(word);
					b_end = curp + cp_size;

					/* copy backwards so blocks can overlap */
					if (s_end != b_end)
					{
						while (cp_size--)
							*(--s_end) = *(--b_end);

						/* remove magic marker at old block position */
						*curp = 0;
					}

					/* set new Memory headers size */
					((Memory *)savepoint)->Size = adjblksize;
				}
				else
				{
#if 1 /*DBG*/
{					word i; char *cp; char pr[2];
				
					KDebug("CheckFail:\n\t\t{ %s } start: %x  memsize: %x blksize: %x\n",
						(int)curp, (char *)curp, memsize,blksize);

					cp = (char *)(&((RRDCommon *)curp)->TotalSize);
					pr[1] = '\0';
					
					KDebug("START>>>\n");
					for(i=blksize - offsetof(RRDCommon, TotalSize); i > 0; i--)
					{
						pr[0] = *cp++;
						KDebug(pr);
					}
					KDebug("\n<<END\n");
}
#endif
				}
			}
#if 1 /*dbg */
			else
			{
				KDebug("Inconsistent size:\n\t\t{ %s } start: %x  memsize: %x blksize: %x\n",
						(int)curp, (char *)curp, memsize,blksize);

			        if(*curp == RRDBuffMagic && blksize > sizeof(RRDBuffBlock) + RRDBuffMax)
			        	KDebug("	BUFFsize wrong\n");
			        elif(*curp == RRDFileMagic && blksize > sizeof(RRDFileBlock) + PATH_MAX)
			        	KDebug("	FILEsize wrong\n");
			        elif(*curp == RRDDirMagic  && blksize > sizeof(RRDDirBlock) + PATH_MAX)
			        	KDebug("	DIRsize wrong\n");
			        elif(*curp == RRDSymbMagic && blksize > sizeof(RRDSymbBlock) + PATH_MAX * 2)
			        	KDebug("SYMBsize wrong\n");

				if (memsize < adjblksize)
		        		KDebug("	adjblock to large\n");
				if (memsize > adjblksize + Epsilon)
		        		KDebug("	adjblock to small\n");

			  	if ((word)curp - sizeof(Memory) + adjblksize > (word)top) {
					KDebug("	adjblksize over top %x, curp+memsize %x, Blksize %x\n",
						top, curp + memsize, blksize);
				  	KDebug("	adjblksize %x, curp %x, memsize %x\n",
						(word)adjblksize, (word)curp, (word)memsize);
			  	}
			}
#endif
		}
	}

	/* return size of memory left */
	return size;
}
/*}}}*/
#endif

/*}}}*/

/* -- End of memory.c */
