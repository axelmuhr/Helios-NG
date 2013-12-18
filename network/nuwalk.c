/*------------------------------------------------------------------------
--                                                                      --
--           H E L I O S   N E T W O R K I N G   S O F T W A R E	--
--           ---------------------------------------------------	--
--                                                                      --
--             Copyright (C) 1990, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- netutils : nuwalk.c							--
--	This code walks down the current heap, displaying the address	--
--	and size of all allocated bits of memory.			--
--									--
--	Author:  BLV 10/4/92						--
--                                                                      --
------------------------------------------------------------------------*/
/* RcsId: $Header: /hsrc/network/RCS/nuwalk.c,v 1.2 1993/08/11 10:40:46 bart Exp $*/

#include <helios.h>
#include <task.h>
#include <syslib.h>
#include <queue.h>
#include <memory.h>

typedef struct Memb {
	word		Size;
	struct Memb	*Next;
} Memb;

typedef struct HeapBlock {
	Node	Node;
	word	Size;
	word	Free;
	Memb	*FreeQ;
} HeapBlock;

static void ShowOne(HeapBlock *heap)
{ Memb		*m;
  BYTE		*heap_end;

  IOdebug("heap at %x : size %d, free %d", heap, heap->Size, heap->Free);
  m		= (Memb *) (heap + 1);
  heap_end	= ((BYTE *) (heap + 1)) + heap->Size;
  
  while ((BYTE *) m < heap_end)
   { if ((m->Size & 1) == 0)
      IOdebug("Alloc : %x, size %d", ((BYTE *) m) + 8, m->Size);
     m = (Memb *) ((BYTE *)m + (m->Size & ~1));
   }
}

void ShowHeap(void)
{ HeapBlock *heap = (HeapBlock *) MyTask->HeapBase;
  List	    *HeapList;

  heap = (HeapBlock *) (((word) heap + 15) & ~15);
  HeapList = (List *) heap->Node.Prev;
  IOdebug("ShowHeap");
  (void) WalkList(HeapList, (WordFnPtr) &ShowOne);
}


