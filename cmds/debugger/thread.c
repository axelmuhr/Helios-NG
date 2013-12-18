/**
*
* Title:  Helios Debugger - Thread support.
*
* Author: Andy England
*
* Date:   March 1989
*
*         (c) Copyright 1988, Perihelion Software Ltd.
*
*         All Rights Reserved.
*
**/

#ifdef __TRAN
static char *rcsid = "$Header: /hsrc/cmds/debugger/RCS/thread.c,v 1.3 1992/10/27 13:49:27 nickc Exp $";
#endif

#include "tla.h"

/**
*
* thread = newthread(debug, id);
*
* Create a new thread.
*
**/
PUBLIC THREAD *newthread(DEBUG *debug, int id)
{
  THREAD *thread;
  WINDOW *window;

  if ((window = wopen(debug->display)) == NULL) return NULL;
  thread = NEW(THREAD);
  thread->id = id;
  thread->loc.module = NULL;
  thread->loc.line = 0;
  thread->block = NULL;
  thread->function = NULL;
  thread->window = window;
  InitSemaphore(&thread->sync, 0);
  /* ACE: Not sure about this */
  if (debug->threadlist.Head->Next == NULL) debug->thread = thread;
  AddHead(&debug->threadlist, &thread->node);
  return thread;
}

/**
*
* remthread(thread);
*
* Remove a thread.
*
**/
PUBLIC void remthread(THREAD *thread)
{
  Remove(&thread->node);
  wclose(thread->window);
  freemem(thread);
}

PRIVATE BOOL cmpthread(THREAD *thread, int id)
{
  return thread->id == id;
}

PUBLIC THREAD *findthread(DEBUG *debug, int id)
{
  THREAD *thread;

  if ((thread = (THREAD *)SearchList(&debug->threadlist, (WordFnPtr)cmpthread, id)) == NULL)
    return newthread(debug, id);
  return thread;
}

/**
*
* nextthread(debug)
*
* Cycle forward to next thread.
*
**/
PUBLIC void nextthread(DEBUG *debug)
{
  THREAD *thread;

  if ((thread = debug->thread) == NULL) return;
  thread = (THREAD *)thread->node.Next;
  if (thread->node.Next == NULL) thread = (THREAD *)debug->threadlist.Head;
  debug->thread = thread;
  wselect(thread->window);
}

/**
*
* prevthread(debug)
*
* Cycle backward to previous thread.
*
**/
PUBLIC void prevthread(DEBUG *debug)
{
  THREAD *thread;

  if ((thread = debug->thread) == NULL) return;
  thread = (THREAD *)thread->node.Prev;
  if (thread->node.Prev == NULL) thread = (THREAD *)debug->threadlist.Tail;
  debug->thread = thread;
  wselect(thread->window);
}

/**
*
* resume(thread);
*
*
*
**/
PUBLIC void resume(THREAD *thread)
{
  lowlight(thread->window);
}
