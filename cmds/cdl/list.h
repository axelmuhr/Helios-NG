/**
*
* Title:  List Support.
*
* Author: Andy England
*
* Date:   May 1988
*
* $Header: /hsrc/cmds/cdl/RCS/list.h,v 1.1 1990/08/28 10:42:06 james Exp $
*
*/

#ifdef helios
#include <queue.h>
#include <stdlib.h>
#include <string.h>
#endif

#ifndef helios
#define NULL 0
#define unless(c) if (!(c))
#define until(c)  while (!(c))
#endif
#define strequ(s,t) (strcmp(s,t) == 0)

#ifdef helios
#define NODE Node
#define LIST List
#else
typedef struct Node
{
  struct Node *Next;
  struct Node *Prev;
} NODE;

typedef struct List
{
  NODE *Head;
  NODE *Earth;
  NODE *Tail;
} LIST;

void InitList();
void WalkList();
#endif

