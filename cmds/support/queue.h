/****************************************************************/
/* queue.h                                                      */
/*                                                              */
/* Queuing structures                                           */
/*                                                              */
/****************************************************************/
/* RcsId: $Id: queue.h,v 1.2 1991/10/21 09:42:23 paul Exp $ Copyright (C) Perihelion Software Ltd.	*/

#ifndef __queueh
#define __queueh

typedef struct Node {
        struct Node *Next;      /* next node in list            */
        struct Node *Prev;      /* previous node in list        */
} Node;

typedef struct List {
        struct Node *Head;      /* head node on list            */
        struct Node *Earth;     /* always NULL                  */
        struct Node *Tail;      /* tail node on list            */
} List;

#endif
