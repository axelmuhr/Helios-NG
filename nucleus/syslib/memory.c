/*------------------------------------------------------------------------
--                                                                      --
--                     H E L I O S   N U C L E U S                      --
--                     ---------------------------                      --
--                                                                      --
--            Copyright (C) 1987,1990, Perihelion Software Ltd.         --
--                        All Rights Reserved.                          --
--                                                                      --
-- syslib/memory.c							--
--                                                                      --
--	System Library, the common program interface to the operating   --
--	system.								--
--                                                                      --
--	Author:  NHG 16/8/87						--
--		 NHG 03/8/90						--
--                                                                      --
------------------------------------------------------------------------*/
/* SccsId:	 %W%	%G%	Copyright (C) 1987, Perihelion Software Ltd.*/
/* $Id: memory.c,v 1.26 1993/07/09 13:36:38 nickc Exp $ */

#define _in_memory

#include "sys.h"
#include <root.h>

/* Heap Management...						*/

#ifdef	SYSDEB
#define CHECKMEM	1		/* 0 = off, !=0 = on, 2 = checkheap on free */
#define CHECKTAG	0xaabbccdd
#else
#define CHECKMEM	0		/* no SYSDEB no CHECKMEM */
#endif

typedef struct Memb {
	word		Size;		
	struct Memb	*Next;		/* link in freeq or CHECKTAG */
					/* followed by memory	*/
} Memb;


typedef struct HeapBlock {
	Node		Node;		/* in heap list		*/
	word		Size;		/* actual size of Mem	*/
	word		Free;		/* number of bytes free */
	Memb		*FreeQ;		/* list of free blocks	*/
					/* followed by memory	*/
} HeapBlock;

typedef struct TryMallocArg {
	word		size;
	Memb		*result;
} TryMallocArg;

PRIVATE Semaphore	HeapLock;

PRIVATE List		Heap;		/* list of HeapBlocks	*/

PRIVATE bool		HeapInitialised;

PRIVATE char		*ThisTask;
PRIVATE char 		*Caller;

#if CHECKMEM
static void CheckHeap(HeapBlock *heap);
static void settags(Memb *m);
static void checktags(char *proc,Memb *m);
#endif

static HeapBlock *NewHeap(HeapBlock *heap, word size)
{
	HeapBlock *heap1 = heap;
	
	/* trim heap and size to 16 byte boundaries */
	
	heap = (HeapBlock *)(((word)heap + 15) &~15);
	size -= 16;	
	size = (((word)heap1+size) & ~15) - (word)heap;
	size -= sizeof(HeapBlock);	

	heap->Free = heap->Size = size;

	heap->FreeQ = (Memb *)(heap+1);	
	
	heap->FreeQ->Size = size+1;
	heap->FreeQ->Next = NULL;

#if CHECKMEM
	settags(heap->FreeQ);
#endif
	AddTail(&Heap,&heap->Node);
	
	return heap;
}

static bool TryMalloc(HeapBlock *heap, TryMallocArg *tma)
{
	Memb *prev;
	Memb *m;
	word size = tma->size;
	
	for( prev = NULL, m = heap->FreeQ ; m != NULL ; prev = m, m = m->Next )
	{
		word bsize = m->Size & ~1;
		if( bsize >= size )
		{
			word diff = bsize-size;
			Memb *next;

			/* if the sizes are close, allocate whole block */
			if( diff <= 4*sizeof(Memb))
			{
				if( prev == NULL ) heap->FreeQ = m->Next;
				else prev->Next = m->Next;
				heap->Free -= bsize;
				m->Size = bsize;
				break;
			}
			
			/* otherwise carve this block up	*/
			next = (Memb *)((byte *)m + size);

			next->Size = diff+1;
			next->Next = m->Next;
			
#if CHECKMEM
			settags(next);
#endif
			if( prev == NULL ) heap->FreeQ = next;
			else prev->Next = next;
				
			m->Size = size;
			heap->Free -= size;
	
			break;
		}
	}

#if CHECKMEM
	if( m != NULL ) settags(m);
#endif
	tma->result = m;
	
	return (m!=NULL);
}

static bool TryFree(HeapBlock *heap, Memb *mem)
{
	Memb *next, *prev;

	/* see whether the block belongs in this heap */
	
	if(	(char *)mem < (char *)(heap+1) ||
		(char *)mem > (char *)(heap+1)+heap->Size ) return FALSE;
		

#ifdef SYSDEB
	SysDebug(memory)("TryFree: %x size %d",mem,mem->Size);
#if (CHECKMEM==2)
	CheckHeap(heap);
#endif
#endif

	if( (mem->Size & 1) != 0 )
	{
#if CHECKMEM
		IOdebug("%s/%s: Double free attempt on %x %x",ThisTask,Caller,mem,mem->Size);
#endif
		return TRUE;	/* block already free */
	}
	
	heap->Free += mem->Size;
	prev = NULL;

#if CHECKMEM
	checktags("TryFree",mem);
#endif

	for(next = heap->FreeQ; ; prev = next,next = next->Next)
	{
		if( next == NULL || next >= mem ) /* we have found where it goes	*/
		{
			word inlist = false;			

			if( next == mem ) 
			{
#if CHECKMEM
				IOdebug("%s/%s: Double free attempt on %x %x",ThisTask,Caller,mem,mem->Size);
#endif		
				return TRUE; /* freeing a free block */
			}

			if( prev == NULL )
			{
				/* new heap head			*/
				mem->Next = heap->FreeQ;
				heap->FreeQ = mem;
				mem->Size += 1;
				inlist = true;
			}
			/* check this in not an already free block	*/
			elif( (word)prev + (prev->Size&~1) > (word)mem ) 
			{
#if CHECKMEM
				IOdebug("%s/%s: Double free attempt on %x %x",ThisTask,Caller,mem,mem->Size);
#endif
				return TRUE;
			}
			elif( (word)prev + (prev->Size&~1) == (word)mem )
			{
				/* coalesce with prev			*/
				prev->Size += mem->Size;
				inlist = true;
				mem = prev;
#if CHECKMEM
				settags(mem);
#endif
			}
			if( next == NULL )
			{
				/* it goes at the end			*/
				if( inlist ) break;
				prev->Next = mem;
				mem->Next = NULL;
				mem->Size += 1;		/* set free bit */
				break;
			}
			elif( (word)mem + mem->Size == (word)next )
			{
				/* we can coalesce with next		*/
				mem->Size = (mem->Size&~1) + next->Size;
#if CHECKMEM
				settags(mem);
#endif
				if( inlist ) mem->Next = next->Next;
				else 
				{	/* replace next in list		*/
					prev->Next = mem;
					mem->Next = next->Next;
					break;
				}
			}
			if( !inlist )
			{
				mem->Next = next;
				prev->Next = mem;
				mem->Size += 1;
#if CHECKMEM
				settags(mem);
#endif
			}
			break;
		}
	}

	return TRUE;
}

static word TryFreeMem(HeapBlock *heap)
{
	/* see if we have emptied this heap,if so return it	*/
	/* to system.						*/

	if( heap->Free == heap->Size )
	{
		Remove(&heap->Node);
		FreeMem(heap);
	}
	return 0;
}

static word HeapSize(HeapBlock *heap) { return heap->Size; }

static word HeapFree(HeapBlock *heap) { return heap->Free; }

static word HeapMax(HeapBlock *heap, word *max) 
{
	Memb *m;
	
	for( m = heap->FreeQ; m != NULL ; m = m->Next )
	{
		if( m->Size > *max ) *max = m->Size-1;
	}
	return 0;
}

static void InitHeap(void)
{
	word hsize = ProgramWord_(MyTask->Program,Heapsize);
	TryMallocArg tma;
	HeapBlock *heap = (HeapBlock *)MyTask->HeapBase;

	InitSemaphore(&HeapLock,1);
	InitList(&Heap);
	heap = NewHeap(heap,hsize);
	
	ThisTask = (char *)(MyTask->TaskEntry)+8;

	Caller = "<unknown>";
#if CHECKMEM
	/* allocate a small block so we never try to return this heap	*/
	/* to system							*/
	tma.size = sizeof(Memb)+2*sizeof(word);
#else
	tma.size = sizeof(Memb);
#endif
	TryMalloc(heap,&tma);

	HeapInitialised = true;
}

#ifdef SYSDEB

static void HeapInfo(HeapBlock *heap)
{
	Memb *m = (Memb *)(heap+1);
	Memb *f = heap->FreeQ;
	
	IOdebug("%s/%s: Heap %x Size %d Free %d FreeQ %x",
			ThisTask,Caller,heap,heap->Size,heap->Free,heap->FreeQ);
	
	while( m < (Memb *)((char *)(heap+1) + heap->Size) )
	{
		if( m == f )
		{
			IOdebug("Free  : %x %d",m,m->Size);
			f = f->Next;
		}
		else {
			IOdebug("Alloc : %x %d",m,m->Size);
		}
		m = (Memb *)((char *)m + m->Size);
	}
#if CHECKMEM
	CheckHeap(heap);
#endif
}

static void MemInfo(void)
{
	word free, total, max = 0;

	MyTask->Flags ^= Task_Flags_meminfo;
	
	Wait(&HeapLock);
	
	free = WalkList(&Heap,HeapFree);
	total = WalkList(&Heap,HeapSize);
	WalkList(&Heap,HeapMax,&max);
	
	IOdebug("Memory Size %d Free %d Max %d",total,free,max);
	
	WalkList(&Heap,(WordFnPtr)HeapInfo);
	
	Signal(&HeapLock);
}

#if CHECKMEM

static void CheckHeap(HeapBlock *heap)
{
	Memb *m = (Memb *)(heap+1);
	Memb *f = heap->FreeQ;
	
	while( m < (Memb *)((word)(heap+1) + heap->Size) )
	{
		word size = m->Size&~1;
		word freebit = m->Size & 1;
		
		if( size < 0 || size > heap->Size )
		{
			IOdebug("%s/%s: CheckHeap %x: Size error on %x %x",ThisTask,Caller,heap,m,m->Size);
			return;
		}
		if( freebit )
		{
			if( m != f ) 
			{
				IOdebug("%s/%s: CheckHeap %x: free block not in FreeQ",ThisTask,Caller,heap,m,m->Size);
			}
			else f = f->Next;
		}
		else
		{
			if( m == f )
			{
				IOdebug("%s/%s: CheckHeap %x: free bit clear on block in FreeQ %x %x",ThisTask,Caller,heap,m,m->Size);
				f = f->Next;
			}
		}
		while( f < m )
		{
			IOdebug("%s/%s: CheckHeap %x: missed block in FreeQ",ThisTask,Caller,heap,f,f->Size);
			f = f->Next;
		}
		checktags("CheckHeap",m);
		m = (Memb *)((char *)m + size);
	}
}

PRIVATE void CheckMem(void)
{
	WalkList(&Heap,(WordFnPtr)CheckHeap);
}

static void settags(Memb *m)
{
	word size = (m->Size&~1);
	word wsize = size/4;
	word *wm = (word *)m;
	wm[wsize-1] = (word)m;
	wm[wsize-2] = (word)m;
	if( (m->Size & 1) == 0 ) m->Next = (Memb *)CHECKTAG;
}

static void checktags(char *proc,Memb *m)
{
	word msize = m->Size < 0 ? -m->Size : m->Size;
	word size = (msize&~1);
	word wsize = size/4;
	word *wm = (word *)m;

	if( (msize & 1) == 0 && (word)m->Next != CHECKTAG)
	{
		char mcn[NameMax+1];

		MachineName(mcn);

		IOdebug("%s/%s/%s/%s: CHECKTAG not set on %x sz %x nxt %x (%a)",mcn, ThisTask,Caller,proc,m,m->Size,m->Next, m+1);
	}
	
	if( wm[wsize-1] != (word)m || wm[wsize-2] != (word)m )
	{
		char mcn[NameMax+1];

		MachineName(mcn);

		IOdebug("%s/%s/%s/%s: end tags not set on %x sz %x (%a)", mcn,ThisTask,Caller,proc,m,m->Size, m+1);
	}
}
#endif /* CHECKMEM */

#endif /* SYSDEB */

/*--------------------------------------------------------
-- Malloc						--
--							--	
-- Allocate memory from the task's pool.		--
-- 							--
--------------------------------------------------------*/

PUBLIC void *
Malloc( WORD size )
{
	Memb *m;
	word hsize;

#ifdef SYSDEB
	SysDebug(memory)("Malloc(%d) from %s",size, procname(returnlink_(size)));
	if( MyTask->Flags & Task_Flags_meminfo ) MemInfo();
#endif
#if 0
	if( size == 0 )
	{
#ifdef SYSDEB
		SysDebug(memory)("Malloc(0) returning NULL");
#endif	
		return NULL;
	}
#endif

	if( !HeapInitialised ) InitHeap();
	Wait(&HeapLock);

#if CHECKMEM
	Caller = procname(returnlink_(size));
#endif	
	if( size < 0 )
	{
		switch( size )
		{
		case -1:	/* return total no. of free bytes in heap */
			hsize = WalkList(&Heap,HeapFree);
			break;

		case -2:	/* return size of largest free block in heap */
			hsize = 0;
			WalkList(&Heap,HeapMax,&hsize);
			break;

		case -3:	/* return total size of heap		*/
			hsize = WalkList(&Heap,HeapSize);
			break;
		
		case -4:	/* return % of heap free		*/
			hsize = WalkList(&Heap,HeapSize);
			hsize = WalkList(&Heap,HeapFree)*100/hsize;
			break;

		case -5:	/* return amount of free mem in system	*/
			hsize = GetRoot()->FreePool->Size;
			break;

#ifndef __TRAN
		case -6:	/* return largest free block in system	*/
		{
			word d;

			hsize = StatMem(&d,&d,&d,&d) - sizeof(Memb);
			break;
		}
#endif

		case -7:	/* return total size of mem in system	*/
			hsize = GetRoot()->FreePool->Max;
			break;

		case -8:	/* return % of system memory free	*/
		{
			RootStruct *root = GetRoot();

			hsize = root->FreePool->Size * 100 / root->FreePool->Max;
			break;
		}

		default:	/* illegal argument			*/
			hsize = 0;
			break;
		}
#if CHECKMEM
		Caller = "<unknown>";
#endif
		Signal(&HeapLock);
		return (void *)hsize;
	}

	/* allow for Memb and round size up to whole no. of Membs	*/
#if CHECKMEM
	size += sizeof(word)*2;	/* allocate 2 words extra	*/
#endif
	size = (size + 2*sizeof(Memb) - 1) & ~(sizeof(Memb)-1);

	/* BLV - catch malloc(0x7fffffff) which turns into a -ve size	*/
	if (size < 0)
		{ m = NULL; goto failed; }
	
	hsize = ProgramWord_(MyTask->Program,Heapsize);
	
	/* Check whether block is more than half a heap block, if so	*/
	/* allocate it explicitly from system if we are allowed to	*/
	/* Otherwise fall through to allocate it from heap.		*/
	if( size > hsize/2 && (MyTask->Flags&Task_Flags_fixmem) == 0)
	{
		m = (Memb *)AllocMem(size,&MyTask->MemPool);
		if( m == NULL ) goto failed;
#if CHECKMEM
		m->Size = size;
		settags(m);
#endif
		m->Size = -size;		/* flag as special	*/
	}
	else forever
	{
		HeapBlock *heap;
		TryMallocArg tma;
		
		tma.size = size;

		SearchList(&Heap,TryMalloc,&tma);

		m = tma.result;
				
		if( m != NULL )
		{
#if CHECKMEM
			if( m->Size < size ) 
			{
				IOdebug("%s/%s: size error %d %x %d",ThisTask,Caller,size,m,m->Size);
			}
#endif
			break;
		}

		/* If fixmem is set, we may not expand heap	*/
		if( MyTask->Flags&Task_Flags_fixmem ) goto failed;
		
		/* allocation failed, get a new heap block from */
		/* system and add it to the heap.		*/
		
		heap = (HeapBlock *)AllocMem(hsize,&MyTask->MemPool);
		if( heap == NULL ) goto failed;
		heap = NewHeap(heap,hsize);
	}
#ifdef SYSDEB
	SysDebug(memory)("Malloc allocated %P[%x]",&m->Next,m->Size);
#endif
#if CHECKMEM
	checktags("Malloc",m);
#endif
failed:
	WalkList(&Heap,TryFreeMem);
#if CHECKMEM
	Caller = "<unknown>";
#endif
#ifdef SYSDEB
	if( m == NULL )
	{
		word f = MyTask->Flags;
		MyTask->Flags = Task_Flags_error;
		_SysDebug("Malloc(%d) from %s failed",size,procname(returnlink_(size)));
		MyTask->Flags = f;
	}
#endif

	Signal(&HeapLock);
	return m==NULL?NULL:(void *)(m+1);	
}

/*--------------------------------------------------------
-- Free							--
--							--
-- Release a memory block back to the task pool		--
-- 							--
--------------------------------------------------------*/

PUBLIC word Free(void *memarg)
{
	Memb *mem = ((Memb *)memarg)-1;
	word r = Err_Null;

	/* dont complain if a null pointer is freed */
	if( memarg == NULL ) return r;

#ifdef SYSDEB
	/* check this memory really belongs to us... */
	if( !InPool(memarg,&MyTask->MemPool)) 
	{
		SysDebug(error)("Attempt to free un-owned memory: %x from %s",memarg,procname(returnlink_(memarg)));
		return EC_Error|SS_SysLib|EG_Invalid|EO_Memory;
	}
#endif		
	/* if the size field is negative, it was allocated specially	*/
	/* so return it to the system.					*/
	if( mem->Size < 0 ) 
	{
#ifdef SYSDEB
		SysDebug(memory)("FreeMem %P size %x",mem,mem->Size);
#endif
#if CHECKMEM
	checktags("Free",mem);
#endif
		FreeMem((void *)mem);
		return Err_Null;
	}

	Wait(&HeapLock);

#if CHECKMEM
	Caller = procname(returnlink_(memarg));
#endif	
	if( !SearchList(&Heap,TryFree,mem) ) 
		r = EC_Error|SS_SysLib|EG_Invalid|EO_Memory;

	/* now see if we can return any free blocks to the Kernel */
	WalkList(&Heap,TryFreeMem);

#if CHECKMEM
	Caller = "<unknown>";
#endif

	Signal(&HeapLock);

	return r;
}

void FreeStop(void *memarg)
{
	Memb *mem = ((Memb *)memarg)-1;

	/* dont complain if a null pointer is freed */
	if( memarg == NULL ) StopProcess();

#ifdef SYSDEB
	/* check this memory really belongs to us... */
	if( !InPool(memarg,&MyTask->MemPool)) 
	{
		SysDebug(error)("Attempt to free un-owned memory: %x",memarg);
		StopProcess();
	}
#endif		
#if CHECKMEM
	Caller = procname(returnlink_(memarg));

	checktags("FreeStop (stack overflow?)",mem);
#endif

	/* if the size field is negative, it was allocated specially	*/
	/* so return it to the system.					*/
	if( mem->Size < 0 ) 
	{
#ifdef SYSDEB
		SysDebug(memory)("FreeMem %P size %x",mem,mem->Size);
#endif
		FreeMemStop((void *)mem);
	}

	Wait(&HeapLock);

	SearchList(&Heap,TryFree,mem);

#if CHECKMEM
	Caller = "<unknown>";
#endif
	SignalStop(&HeapLock);
}

extern word MemSize(void *memarg)
{
	Memb *mem = ((Memb *)memarg)-1;
	if( mem->Size < 0 ) return -mem->Size;
	return mem->Size;
}

