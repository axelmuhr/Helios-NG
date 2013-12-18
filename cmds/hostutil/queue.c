/****************************************************************/
/* File: queue.c                                                */
/*                                                              */
/* Manipulates queues made up of List and Node structures.      */
/*                                                              */
/* Author: NHG 17-Feb-87                                        */
/****************************************************************/

/* $Header: /hsrc/cmds/hostutil/RCS/queue.c,v 1.2 1993/06/09 15:01:07 nickc Exp $ */

#include "queue.h"

#define NULL 0

/********************************************************/
/* InitList                                             */
/*                                                      */
/* Initialize a list structure.                         */
/*                                                      */
/********************************************************/

void InitList(list)
struct List *list;
{
        list->Head = (struct Node *)(&list->Earth);
        list->Earth = NULL;
        list->Tail = (struct Node *)list;
}

/********************************************************/
/* AddTail                                              */
/*                                                      */
/* Append a node to the end of a list.                  */
/*                                                      */
/********************************************************/

void AddTail(list,node)
struct List *list;
struct Node *node;
{
        register struct Node *oldtail = list->Tail;

        node->Next = oldtail->Next;
        node->Prev = oldtail;

        oldtail->Next = node;
        list->Tail = node;
}

/********************************************************/
/* AddHead                                              */
/*                                                      */
/* add a node to the head of a queue.                   */
/*                                                      */
/********************************************************/

void AddHead(list,node)
struct List *list;
struct Node *node;
{
        register struct Node *oldhead = list->Head;

        node->Next = oldhead;
        node->Prev = oldhead->Prev;

        oldhead->Prev = node;
        list->Head = node;
}

/********************************************************/
/* RemHead                                              */
/*                                                      */
/* Extract the head node from a list                    */
/*                                                      */
/********************************************************/

struct Node *RemHead(list)
struct List *list;
{
        register struct Node *node = list->Head;
        register struct Node *newhead = node->Next;

        if( newhead == NULL ) return NULL;

        list->Head = newhead;
        newhead->Prev = node->Prev;

        return node;
}

/********************************************************/
/* RemTail                                              */
/*                                                      */
/* Extract the tail node from a list                    */
/*                                                      */
/********************************************************/

struct Node *RemTail(list)
struct List *list;
{
        register struct Node *node = list->Tail;
        register struct Node *newtail = node->Prev;

        if( newtail == NULL ) return NULL;

        list->Tail = newtail;
        newtail->Next = node->Next;

        return node;
}

/********************************************************/
/* PreInsert                                            */
/*                                                      */
/* Add a node to a list just before the given one       */
/* Note: next may not be a list, but it may be          */
/* list->Head of an empty list                          */
/********************************************************/

void
PreInsert(next,node)
struct Node *next, *node;
{
        node->Next = next;
        node->Prev = next->Prev;
        next->Prev = node;
        node->Prev->Next = node;
}

/********************************************************/
/* PostInsert                                           */
/*                                                      */
/* Add a node to a list just after the given one        */
/* pred may be a list                                   */
/********************************************************/

void
PostInsert(pred,node)
struct Node *pred, *node;
{
        node->Next = pred->Next;
        node->Prev = pred;
        pred->Next = node;
        node->Next->Prev = node;
}

/********************************************************/
/* Remove						*/
/*							*/
/* Remove node from its queue				*/
/*							*/
/********************************************************/

struct Node *Remove(node)
struct Node *node;
{
	node->Next->Prev = node->Prev;
	node->Prev->Next = node->Next;
	return node;
}
