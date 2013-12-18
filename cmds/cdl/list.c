/**
*
* Title:  List Support.
*
* Author: Andy England
*
* Date:   May 1988
*
*         (c) Copyright 1988, Perihelion Software Ltd.
*
*         All Rights Reserved.
*
**/
static char *rcsid = "$Header: /hsrc/cmds/cdl/RCS/list.c,v 1.1 1990/08/28 10:41:46 james Exp $";

#include "list.h"

#ifdef helios
int CountList(LIST *ListPtr, int (*Function)())
#else
int CountList(ListPtr, Function)
LIST *ListPtr;
int (*Function)();
#endif
{
  NODE *Node, *Next;
  int Count = 0;

  for (Node = ListPtr->Head; Node->Next; Node = Next)
  {
    Next = Node->Next;
    if (Function == NULL) Count++;
    else Count += (*Function)(Node);
  }
  return Count;
}

#ifndef helios
void WalkList(ListPtr, Function)
LIST *ListPtr;
int (*Function)();
{
  NODE *Node, *Next;

  for (Node = ListPtr->Head; Node->Next; Node = Next)
  {
    Next = Node->Next;
    (*Function)(Node);
  }
}

void InitList(ListPtr)
LIST *ListPtr;
{
  ListPtr->Head  = (NODE *)&(ListPtr->Earth);
  ListPtr->Earth = NULL;
  ListPtr->Tail  = (NODE *)&(ListPtr->Head);
}

void AddHead(ListPtr, Node)
LIST *ListPtr;
NODE *Node;
{
  Node->Next = ListPtr->Head;
  Node->Prev = (NODE *)&(ListPtr->Head);
  ListPtr->Head = ListPtr->Head->Prev = Node;
}

void AddTail(ListPtr, Node)
LIST *ListPtr;
NODE *Node;
{
  Node->Next = (NODE *)&(ListPtr->Earth);
  Node->Prev = ListPtr->Tail;
  ListPtr->Tail = ListPtr->Tail->Next = Node;
}
#endif
  
