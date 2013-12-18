/**
*
* Title:  Helios Debugger - Memory allocation routines.
*
* Author: Andy England
*
* Date:   February 1989
*
*         (c) Copyright 1989 - 1993, Perihelion Software Limited.
*
*         All Rights Reserved.
*
**/

#ifdef __TRAN
static char *rcsid = "$Header: /hsrc/cmds/debugger/RCS/mem.c,v 1.6 1993/07/12 15:48:35 nickc Exp $";
#endif

#include "tla.h"

PRIVATE word totalmem = 0;
PRIVATE word numblocks = 0;

#ifdef MEMCHECK

#define MAGIC 0xdeadffcc
#define WORDS(n) ((n + 3) >> 2)

typedef struct
{
  NODE node;
  word size;
  word buffer;
  word store[1];
} MEM;

PRIVATE LIST memlist;
PRIVATE Semaphore memlock;

PRIVATE void memcheck(MEM *mem)
{
  unless (mem->buffer == MAGIC AND mem->store[WORDS(mem->size)] == MAGIC)
    {
      exit(1);
    }
}

PRIVATE void memchecker(void)
{
  forever
  {
    Delay(2000000);
    Wait(&memlock);
    
    (void)WalkList(&memlist, (WordFnPtr)memcheck, 0);

    Signal(&memlock);
  }
}

/**
*
* initmem(memchecking)
*
* Initialise the memory routines.
*
**/
PUBLIC void initmem(BOOL memchecking)
{
  totalmem = numblocks = 0;
  InitSemaphore(&memlock, 1);
  InitList(&memlist);
  if (memchecking)
    {
      IOdebug( "TLA: Memory Checking Enabled" );
      
      Fork(5000, memchecker, 0);
    }
  
}

PUBLIC void *newmem(int size)
{
  MEM *mem;

  Wait(&memlock);
  if ((mem = (MEM *)malloc(sizeof(MEM) + (WORDS(size) << 2))) == NULL)
  {
    IOdebug( "TLA: OUT OF MEMORY" );
    exit(1);
  }
  totalmem += mem->size = size;
  numblocks++;
  mem->buffer = MAGIC;
  mem->store[WORDS(size)] = MAGIC;
  AddTail(&memlist, &mem->node);
  /* IOdebug( "alloc  %x, size %x %s", mem, mem->size, mem->size < 10 ? procname( NULL ) : "" );*/
  
  Signal(&memlock);
  return &mem->store[0];
}

PUBLIC void freemem(void *store)
{
  MEM *  mem;

  
  mem = (MEM *)(((byte *)store) - sizeof(MEM) + sizeof(word));

  Wait(&memlock);

  if (mem->buffer != MAGIC)
    {
      IOdebug( "TLA: Memory Error @ %x (in freemem) [no starting magic] (size %x)",
	      mem, mem->size );
#ifndef __TRAN
      back_trace();
#endif
      
      exit(1);
    }
  else if (mem->store[WORDS(mem->size)] != MAGIC)
    {
      IOdebug( "TLA: Memory Error @ %x (in freemem) [no ending magic]", mem );
      exit(1);
    }

  totalmem -= mem->size;
  numblocks--;
  Remove(&mem->node);
  /* IOdebug( "* free %x, size %x %s", mem, mem->size, mem->size < 10 ? procname( NULL ) : "" );*/
  free(mem);
  Signal(&memlock);
}
#else /* not MEMCHECK */
/**
*
* mem = newmem(size);
*
* Allocate memory.
*
**/
PUBLIC void *newmem(int size)
{
  int *mem;

  if ((mem = malloc(size + sizeof(int))) == NULL)
  {
#ifdef OLDCODE
    error("Out of memory");
    return NULL;
#else
    IOdebug( "TLA: OUT OF MEMORY" );
    exit(1);
#endif
  }
  *mem++ = size;
  totalmem += size;
  numblocks++;
  return mem;
}

/**
*
* freemem(mem);
*
* Deallocate memory.
*
**/
PUBLIC void freemem(void *mem)
{
  int * ptr = (int *)mem;
  
  totalmem -= *--ptr;
  numblocks--;
  
  IOdebug( "free %x, size %x", ptr, *ptr );
  
  free(ptr);
}


/**
*
* initmem(memchecking)
*
* Initialise the memory routines.
*
**/
PUBLIC void initmem(BOOL memchecking)
{
  totalmem = numblocks = 0;

  return;
}

#endif /* not MEMCHECK */

/**
*
* putmem();
*
* Display memory usage.
*
**/
PUBLIC void putmem(void)
{
  debugf("%d bytes in %d blocks", totalmem, numblocks);
}

