/*
 * stack_c.c : 	C Stack manipulation code, C part
 *
 *   Copyright (c) 1992, 1993 Perihelion Software Ltd.
 *     All rights reserved.
 *
 * Author :	N Clifton
 * Version :	$Revision: 1.19 $
 * Date :	$Date: 1993/10/04 15:06:57 $
 * Id :		$Id: stack_c.c,v 1.19 1993/10/04 15:06:57 paul Exp $
 */

#ifndef __C40
#error	stack_c.c is designed for Helios-C40 only
#endif

/*{{{  pragmas      */

#pragma no_stack_checks

/*}}}*/
/*{{{  header files */

#include "../kernel.h"
#include <helios.h>
#include <stdio.h>
#include <task.h>
#include <signal.h>
#include <stdlib.h>
#include <c40.h>

/*}}}*/
/*{{{  constants    */

#define STACK_DEBUG	0

/*}}}*/
/*{{{  functions    */

/*{{{  StackMalloc  */

/*
 * FUNCTION: StackMalloc
 *
 * DESCRIPTION:
 *	Allocates a new stack chunk
 *
 * ARGUMENTS:
 * 	numWords -- required minimum size of the stack chunk
 *
 * RETURNS:
 * 	Pointer to the head of a new stack chunk, word aligned,
 *	contiguous, and guaranteed to be at least 'numWords'
 *	long.  The pointer points to a stack chunk structure.
 *	The stack itself starts just below this structure,
 *	descending through memory.
 *	Returns NULL upon failure.
 */

PRIVATE stack_chunk *
StackMalloc ( unsigned long int numWords )		/* number of words required in new chunk */
{
  stack_chunk *		pChunk;
  stack_chunk *		pCurrentChunk;
  
  
  if (STACK_DEBUG)
    KDebug( "StackMalloc: asked to provided a chunk of %d words\n", numWords );
  
  /* get hold of the current stack chunk structure */
  
  pCurrentChunk = (stack_chunk *)(GetExecRoot()->CurrentSaveArea->stack_chunk);
  
  if (STACK_DEBUG && pCurrentChunk == NULL)
    {
      KDebug( ( "StackMalloc: unable to find current stack chunk\n" ) );
      
      return NULL;
    }  
  
  /* remember that we are going to knock some space off the stack */
      
  numWords += (PCS_STACKGUARD / sizeof (word));
  
  /* grab task's stack lock */
  
  Wait( & _Task_ -> StackLock );
  
  /* scan free chunks for a chunk of the right size */
  
  for (pChunk  = Head_( stack_chunk, _Task_ -> StackChunks );
       pChunk != NULL &&
       !EndOfList_( pChunk ) &&
       pChunk->size < numWords;
       pChunk  = Next_( stack_chunk, pChunk ) )
    ;
  
  /* check the result of our scan */

  if (pChunk != NULL && !EndOfList_( pChunk ))
    {
      /* we found a suitable node - remove it from free list */
      
      Remove( (Node *)pChunk );
    }
  
  /* release stack lock */
  
  Signal( & _Task_ -> StackLock );
      
  /* if we failed to find a suitable free stack chunk then allocate one */
  
  if (pChunk == NULL || EndOfList_( pChunk ))
    {
      unsigned long int *	ptr;
      unsigned long int		min_num_words = ProgramWord_( _Task_->Program, Stacksize ) / sizeof (word);
      
      
      /*
       * Ensure that the stack chunk to be allocated meets
       * our minimum requirements, and also check that if we
       * are going to allocate a large chunk, then we allocate
       * a bit extra to cope with further stack growth.
       */
      
      if (numWords < min_num_words)
	numWords = min_num_words;
      else
	numWords += min_num_words;
      
      /* allocate space for the new chunk and stack chunk structure */
      
      ptr = (unsigned long int *)LowAllocMem( numWords * sizeof(word) + sizeof (stack_chunk),
					  &(_Task_->MemPool) );
      
      /* check the value returned */
      
      if (ptr == NULL)
	{
	  /* no memory available - abort */
	  
	  if (STACK_DEBUG)
	    KDebug( ( "StackMalloc: unable to allocate memory for a new stack chunk\n" ) );
	  
	  CallException( _Task_, SIGSTAK );
	  
	  return NULL;
	}
      
      /* Malloc'd pointer is at bottom of allocated RAM, but we have a descending stack, so move to top */
      
      pChunk = (stack_chunk *)(ptr + numWords);
      
      /* fill in stack chunk size */
      
      pChunk->size = numWords;	  /* inclusive of STACK_GUARD */
      
      /* initialise stack chunk pointers */
      
      pChunk->prev = (stack_chunk *)pChunk;
      pChunk->next = (stack_chunk *)pChunk;      
    }
  
# if STACK_DEBUG
  KDebug( ( "StackMalloc: using chunk %x, ending at %x\n",
	   pChunk, ((word *)(pChunk - 1)) - pChunk->size  ) );
#endif
      
  /* insert it after current stack chunk */
  
  pChunk->prev = pCurrentChunk;
  pCurrentChunk->next = pChunk;
  
  /* install this new chunk in this thread's save state structure */
  
  GetExecRoot()->CurrentSaveArea->stack_chunk = pChunk;
  
#if STACK_DEBUG
    {
      int a;
      
      
      a = _word( 0x08000012 );
      
      KDebug( ("StackMalloc: current chunk %x new chunk %x, se = %x\n", 
	       pCurrentChunk, pChunk, a * 4 ) );
    }
#endif
  
  return pChunk;
  
} /* StackMalloc */

/*}}}*/
/*{{{  StackFree    */

/*
 * FUNCTION: StackFree
 *
 * DESCRIPTION:
 *	Frees an allocated stack chunk
 *
 * ARGUMENTS:
 * 	pChunk	-- chunk to be freed
 *
 * RETURNS:
 * 	Nothing
 */

/*
 * NB/
 * This code is running on the old (full) stack and so
 * it must not call any routines that perform stack checking
 */

PRIVATE void
StackFree ( stack_chunk * pChunk )		/* chunk to be freed */
{
  if (STACK_DEBUG)
    {
      if (pChunk == NULL)
	{
	  KDebug( ( "StackFree: trying to free a NULL chunk\n" ) );

	  return;
	}    
      
      if (pChunk->prev == NULL)
	{
	  KDebug( ( "StackFree: no previous stack chunk!\n" ) );

	  return;
	}
    }
  
  /* record the new current stack chunk */

  GetExecRoot()->CurrentSaveArea->stack_chunk = pChunk->prev;

  /* get hold of the stack lock */
  
  Wait( &_Task_ -> StackLock );

  /* Add the free stack chunk to the end of the free stack chunk list */
  
  AddTail( &_Task_ -> StackChunks, (Node *)pChunk );

  /* release the stack lock */

  Signal( &_Task_ -> StackLock );  

#if STACK_DEBUG
    KDebug( ("StackFree: freed %x, new %x ends at %x, SE = %x\n", 
	     pChunk,
	     GetExecRoot()->CurrentSaveArea->stack_chunk,
	     ((word *)(GetExecRoot()->CurrentSaveArea->stack_chunk - 1)) -
	     GetExecRoot()->CurrentSaveArea->stack_chunk->size,
	     pChunk->stack_end_pointer ) );
#endif
  
  return;
  
} /* StackFree */

/*}}}*/
/*{{{  ReleaseStack */

/*
 * FUNCTION: ReleaseStack
 *
 * DESCRIPTION:
 *	Releases all free stack chunks back to the free pool
 *
 * ARGUMENTS:
 * 	None
 *
 * RETURNS:
 * 	Nothing
 */

/*
 * Note/ This function is no longer needed and could be removed.
 * The 1.3 kernel, however, does export it - hence it remains here.
 */

PUBLIC void
ReleaseStack( void )
{
  stack_chunk *	pChunk;

  
  if (STACK_DEBUG)
    KDebug( "ReleaseStack: Starting" );

  /* grab stack lock */

  Wait( & _Task_ -> StackLock );
  
  /*
   * now walk along list, freeing chunks, until we
   * reach the end.
   */

  for (pChunk = Head_( stack_chunk, _Task_ -> StackChunks );
       pChunk != NULL &&
       !EndOfList_( pChunk );
       )
    {
      stack_chunk *	pNext;


      /* remember the pointer to the next chunk */
      
      pNext = Next_( stack_chunk, pChunk );
      
      /* free the memory of the current chunk */

      if (STACK_DEBUG)
	KDebug( "ReleaseStack: Freeing block at %x",
	       (unsigned long int *)pChunk - (pChunk->size) );
      
      FreeMem( (unsigned long int *)pChunk - (pChunk->size) );
      
      /* point to the next chunk */
      
      pChunk = pNext;
    }

  /* release stack lock */

  Signal( & _Task_ -> StackLock );
  
  if (STACK_DEBUG)
    KDebug( "ReleaseStack: Finished" );
      
  return;
  
} /* ReleaseStack */

/*}}}*/

/*}}}*/

/* end of stack_c.c */
