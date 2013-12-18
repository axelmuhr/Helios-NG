/****************************************************************/
/* File: queue.c                                                */
/*                                                              */
/* Manipulates queues made up of List and Node structures.      */
/*                                                              */
/* Author: NHG 17-Feb-87                                        */
/****************************************************************/

#include <queue.h>

#define PUBLIC
#define PRIVATE static

#define NULL 0

/********************************************************/
/* InitList                                             */
/*                                                      */
/* Initialize a list structure.                         */
/*                                                      */
/********************************************************/

PUBLIC void InitList(list)
struct List *list;
{
        list->head = (struct Node *)(&list->earth);
        list->earth = NULL;
        list->tail = (struct Node *)list;
}

/********************************************************/
/* AddTail                                              */
/*                                                      */
/* Append a node to the end of a list.                  */
/*                                                      */
/********************************************************/

PUBLIC void AddTail(list,node)
struct List *list;
struct Node *node;
{
        register struct Node *oldtail = list->tail;

        node->next = oldtail->next;
        node->prev = oldtail;

        oldtail->next = node;
        list->tail = node;
}

/********************************************************/
/* AddHead                                              */
/*                                                      */
/* add a node to the head of a queue.                   */
/*                                                      */
/********************************************************/

PUBLIC void AddHead(list,node)
struct List *list;
struct Node *node;
{
        register struct Node *oldhead = list->head;

        node->next = oldhead;
        node->prev = oldhead->prev;

        oldhead->prev = node;
        list->head = node;
}

/********************************************************/
/* RemHead                                              */
/*                                                      */
/* Extract the head node from a list                    */
/*                                                      */
/********************************************************/

PUBLIC struct Node *RemHead(list)
struct List *list;
{
        register struct Node *node = list->head;
        register struct Node *newhead = node->next;

        if( newhead == NULL ) return NULL;

        list->head = newhead;
        newhead->prev = node->prev;

        return node;
}

/********************************************************/
/* RemTail                                              */
/*                                                      */
/* Extract the tail node from a list                    */
/*                                                      */
/********************************************************/

PUBLIC struct Node *RemTail(list)
struct List *list;
{
        register struct Node *node = list->tail;
        register struct Node *newtail = node->prev;

        if( newtail == NULL ) return NULL;

        list->tail = newtail;
        newtail->next = node->next;

        return node;
}

/********************************************************/
/* PreInsert                                            */
/*                                                      */
/* Add a node to a list just before the given one       */
/* Note: next may not be a list, but it may be          */
/* list->head of an empty list                          */
/********************************************************/

PUBLIC PreInsert(next,node)
struct Node *next, *node;
{
        node->next = next;
        node->prev = next->prev;
        next->prev = node;
        node->prev->next = node;
}

/********************************************************/
/* PostInsert                                           */
/*                                                      */
/* Add a node to a list just after the given one        */
/* pred may be a list                                   */
/********************************************************/

PUBLIC PostInsert(pred,node)
struct Node *pred, *node;
{
        node->next = pred->next;
        node->prev = pred;
        pred->next = node;
        node->next->prev = node;
}

/********************************************************/
/* Remove                                               */
/*                                                      */
/* Remove node from its queue                           */
/*                                                      */
/********************************************************/

PUBLIC struct Node *Remove(node)
struct Node *node;
{
        node->next->prev = node->prev;
        node->prev->next = node->next;
        return node;
}
