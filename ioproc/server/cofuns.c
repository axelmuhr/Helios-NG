/*------------------------------------------------------------------------
--                                                                      --
--         H E L I O S   I N P U T / O U T P U T   S E R V E R          --
--         ---------------------------------------------------          --
--                                                                      --
--              Copyright (C) 1987, Perihelion Software Ltd.            --
--                         All Rights Reserved.                         --
--                                                                      --
--  cofuns.c                                                            --
--                                                                      --
--          Extra coroutine library functions, and the linked           --
--                                                                      --
--          lists library.                                              --
--                                                                      --
--  Author:  BLV 8/10/87                                                --
--                                                                      --
------------------------------------------------------------------------*/
/* RcsId: $Id: cofuns.c,v 1.13 1994/07/06 10:44:59 mgun Exp $ */
/* Copyright (C) 1987, Perihelion Software Ltd.       			*/

/**
*** I do not intend to document this code, since it should work if you have
*** got the low level coroutine library right. Anyway, it is all simple.
**/

#define CofunsModule
#include "helios.h"

#if !PC
#if SOLARIS
#include <stdarg.h>
#else
#include <varargs.h>
#endif
#include <stdlib.h>
#endif

/**
*** The low level coroutine library, written in assembler language
**/
#if SOLARIS

PUBLIC int fn (InitCo,	   (void));
PUBLIC ptr fn (CreateCo,   (void * (*)(void *), int));
PUBLIC int fn (CallCo,	   (ptr, int));
PUBLIC int fn (WaitCo,	   (int));
PUBLIC int fn (DeleteCo,   (ptr));

#else
/* On the MAC I require also the stacksize for InitCo */
#if MAC
PUBLIC  word fn( InitCo,   (word));
#else
PUBLIC  word fn( InitCo,   (void));
#endif
PUBLIC  word fn( CallCo,   (ptr, word));
PUBLIC  ptr  fn( WaitCo,   (word));
PUBLIC  word fn( DeleteCo, (ptr));
PUBLIC  ptr  fn( CreateCo, (VoidFnPtr, word));
PUBLIC  word CurrentCo;
#endif

/**
*** Coroutine stack sizes : these can be configured
**/
PRIVATE word Stacksize;
#define CoDeleted (123456L)
Conode *current_conode;

               /* virtual memory systems need much larger stacks */
#if (UNIX && !MEIKORTE && !ARMBSD && !SCOUNIX && !SOLARIS)
#define default_stacksize 50000L
#endif
#if (ARMBSD)
         /* R140 has limited swap space, and 10K suffices */
#define default_stacksize 10000L
#endif
#if (MSWINDOWS)
#define default_stacksize 4096
#endif
#if (MAC || (PC && !MSWINDOWS))
#define default_stacksize 2000L
#endif
#if (SOLARIS)
#define default_stack_size	0	/* Works it out for itself */
#endif

#ifndef default_stacksize
#define default_stacksize 5000L
#endif


word InitColib()
{ 
  Stacksize = get_int_config("Coroutine_Stack");
  if (Stacksize eq Invalid_config) Stacksize = default_stacksize;
#if (PC)
	/* The stack must fit into the buffer in main()'s stackframe	*/
  if (Stacksize > 4096) Stacksize = 4096;
#endif
#if WINDOWS
	/* On Windows the stack must be fairly large. */
  if (Stacksize < 3000) Stacksize = 3000;
#endif

  current_conode = (Conode *) NULL;
#if MAC
  return(InitCo(Stacksize));
#else
  return(InitCo()); 
#endif  
}

/*
 * BLV - this function does not appear to have been called from anywhere
 * for a very long time. If the coroutine library involves a memory
 * allocation for CurrentCo/RootCo during InitCo then the result would
 * be a memory leak every time the I/O Server reboots. I have added a call
 * to TidyColib() to the I/O Server's tidy-up code in server.c, but to
 * avoid possible complications for now this is only effective on the PC.
 * It should really be checked for all implementations.
 */
void TidyColib()
{ 
#if (PC)
  iofree((byte *)CurrentCo);
#endif
}

void Suspend()
{ (void) WaitCo(false);
}

void Seppuku()
{
 (void) WaitCo(CoDeleted);
}

PRIVATE VoidConFnPtr co_function;
PRIVATE Conode    *new_conode;

#if ANSI_prototypes

#if SOLARIS
PRIVATE void * new_coroutine(void * not_used)
#else
PRIVATE void new_coroutine(void)
#endif

#else
PRIVATE void new_coroutine()
#endif
{ VoidConFnPtr my_function = co_function;
  Conode    *my_conode  = new_conode;

  (void) WaitCo(false);

  (*my_function)(my_conode);

#if SOLARIS
  not_used = NULL;

  return (void *)NULL;
#endif
}

Conode *NewCo(fun)       /* create a new coroutine and put it on the */
VoidConFnPtr fun;           /* waiting list */
{
  co_function = fun;
  new_conode  = (Conode *) malloc(sizeof(Conode));

  if (new_conode ne (Conode *) NULL)
     { memset((char *) new_conode, 0, sizeof(Conode));
#if SOLARIS
       new_conode->cobase = CreateCo(&new_coroutine, Stacksize);
#else
       new_conode->cobase = CreateCo(func(new_coroutine), Stacksize);
#endif

       if (!(new_conode->cobase))
         { iofree(new_conode); return(false); }
       CallCo(new_conode->cobase, 0L);  /* first CallCo loses argument, fun  */

       return(new_conode);              /* must always WaitCo near beginning */
     }
  return(false);
} 

                      /* call a coroutine, check to see if deleted */
void StartCo(conode)           
Conode *conode;
{ Conode *save_conode = current_conode;
  current_conode = conode;

  if (CallCo( conode->cobase, (word) conode) eq CoDeleted)
   { 
     Remove(&(conode->node));
     DeleteCo( conode->cobase);
     iofree(conode);
   }
  current_conode = save_conode;
}

/**
*** Basic semaphore-style operations for use within coroutines. Aimed
*** at providing locking on the various devices. The Wait() routine
*** must never be called from inside the main coroutine. Also, it must
*** be called if and only if the coroutine is on the WaitingCo list.
**/
typedef struct semaphore_wait {
        Node   node;
        Conode *cortn;
} semaphore_wait;

void Wait(sem)
Semaphore *sem;
{
  sem->count--;
  if (sem->count < 0)   /* access denied */
   { semaphore_wait *wait = (semaphore_wait *) malloc(sizeof(semaphore_wait));

     if (wait eq (semaphore_wait *) NULL)
      return;	/* Up the creek anyway */

     wait->cortn = current_conode;
     AddTail(&(wait->node), &(sem->list));
     current_conode->flags |= CoFlag_Waiting;
     current_conode->type = 0L;
     AddTail(Remove(&(current_conode->node)), PollingCo);

     Suspend();

     if (current_conode->type ne CoSuicide)
      AddTail(Remove(&(current_conode->node)), WaitingCo);
     current_conode->flags &= ~CoFlag_Waiting;
     Remove(&(wait->node));
     iofree(wait);
     if ((current_conode->type eq CoSuicide) || 
         (current_conode->type eq CoTimeout))
      sem->count++;
   }
}

void Signal(sem)
Semaphore *sem;
{ 
  sem->count++;
  if (sem->count < 1)  /* another coroutine is waiting */
   { semaphore_wait *wait;
     wait = (semaphore_wait *) sem->list.head;
     if (wait->node.next ne (Node *) NULL)
      { 
        if (wait->cortn->type ne CoSuicide)
         StartCo(wait->cortn);
      }
   }				
}

void InitSemaphore(sem, count)
Semaphore *sem;
int count;
{ InitList(&sem->list);
  sem->count = count;
}

/*------------------------------------------------------------------------
--                                                                      --
--  Lists                                                               --
--                                                                      --
--      Usual linked list library, but containing only the              --
--      functions which I can be bothered to use.                       --
------------------------------------------------------------------------*/

/* create and initialise a list header, returning 0 to indicate failure */
List *MakeHeader()
{ List *newptr;

  newptr = (List *) malloc( sizeof(List) );
  if (newptr ne NULL)
    InitList(newptr);

  return(newptr);
}

/* initialise a list header to the empty list */
void InitList(listptr)
List *listptr;
{
  listptr->head  = (Node *) &(listptr->earth);
  listptr->earth = NULL;
  listptr->tail  = (Node *) listptr;
} 

/* add a node to the beginning of the list */
Node *AddHead(node, header)
Node *node;
List *header;
{
  node->next = header->head;
  node->prev = (Node *)header;
  header->head = node;
  (node->next)->prev = node;
  return(node);
}

Node *AddTail(node, header)
Node *node;
List *header;
{
  Node * head;

  node->prev = header->tail;
  header->tail = node;
  node->next = (Node *) &(header->earth);
  (node->prev)->next = node;

  return(node);
}

Node *listRemove(node)
Node *node;
{

 if (node == NULL)	return NULL;
 
 if (node->prev != NULL)
   {
     (node->prev)->next = node->next;
   }
 if (node -> next != NULL)
   {
     (node->next)->prev = node->prev;
   }
  return(node);
}

Node *NextNode(node)
Node *node;
{ return(node->next);
}

word TstList(header)
List *header;
{ 
  if ((header->head)->next eq NULL) return(false);
  return(true);
}

#if (ANSI_prototypes && !AMIGA)
	/* To complicate things, some compilers completely disallow K&R C	*/

void WalkList(List *list, VoidNodeFnPtr fun, ...)
{
 Node *node, *node2;
  word arg1, arg2;
  va_list args;

  va_start(args, fun);
  arg1 = va_arg(args, word);
  arg2 = va_arg(args, word);
  va_end(args);

  for (node=list->head; node->next ne NULL; node=node2)
    { node2 = node->next;
      (*fun)(node, arg1, arg2);
    }
}

word Wander(List *list, WordNodeFnPtr fun, ...)
{ Node *node, *node2;
  word result, arg1, arg2;
  va_list args;

  va_start(args, fun);
  arg1 = va_arg(args, word);
  arg2 = va_arg(args, word);
  va_end(args);

  result = false;

  for (node=list->head; (node->next ne NULL) && !result;
       node = node2)
    { node2 = node->next;
      result = (*fun)(node, arg1, arg2);
    }

  return(result);
}

#else /* ANSI_prototypes && !AMIGA */

void WalkList(list, fun, arg1, arg2)
List *list;
VoidFnPtr fun;
word arg1, arg2;
{ Node *node, *node2;

  for (node=list->head; node->next ne NULL; node=node2)
    { node2 = node->next;
      (*fun)(node, arg1, arg2);
    }
}

word Wander(list, fun, arg1, arg2)
List *list;
WordFnPtr fun;
word arg1, arg2;
{ Node *node, *node2;
  word result;

  result = false;

  for (node=list->head; (node->next ne NULL) && !result;
       node = node2)
    { node2 = node->next;
      result = (*fun)(node, arg1, arg2);
    }

  return(result);
}

#endif /* ANSI_prototypes */

                /* This routine just frees every node in the list */
void FreeList(list)
List *list;
{ Node *node, *node2;
  Node * head = list -> head;

#if SOLARIS

  if (head -> prev == NULL)
    { /* empty list */
      return;
    }

  if (list -> earth != NULL)
    {
      /*
       * Somewhere along the line, the earth element of a list gets corrupted in the
       * directory Conode pointer.  This is a quick fix to avoid core dumping
       * messily.
       */

      Debug (Directory_Flag, ("Warning: corrupted earth in list 0x%lx", (long)list));

      InitList (list);

      return;
    }

#endif

  for (node = list->head; node->next ne NULL; node = node2)
    { node2 = node->next;
      iofree(Remove(node));
    }
}

                          /* this routine is needed by the debugger */
void PreInsert(next, node)
Node *next, *node;
{
  node->next = next;
  node->prev = next->prev;
  next->prev = node;
  node->prev->next = node;
}

          /* This routine is used to maintain the order of nodes in WaitingCo */
          /* It puts the node just after the list node given                  */
void PostInsert(node, before)
Node *node, *before;
{
  node->next       = before->next;
  node->prev       = before;
  before->next     = node;
  node->next->prev = node;
}

          /* This routine returns the number of nodes in a list. */
word ListLength(header)
List *header;
{ Node *node;
  word result = 0L;

  for (node = header->head; node->next ne (Node *) NULL; node = node->next)
    result++;

  return(result);
}


