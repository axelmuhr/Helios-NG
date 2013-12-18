/*------------------------------------------------------------------------
--                                                                      --
--                      H E L I O S   K E R N E L                       --
--                      -------------------------                       --
--                                                                      --
--             Copyright (C) 1988, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- queue.c								--
--                                                                      --
--	Queue handling routines						--
--                                                                      --
--	Author:  NHG 8/8/88						--
--                                                                      --
-- Taken from Helios kernel code 2/6/89					--
-- Only functions used have been included				--
-- And only these have been converted to K&R				--
------------------------------------------------------------------------*/
/* SccsId: %W% %G% Copyright (C) 1987, Perihelion Software Ltd.*/
/* RcsId: $Id: queue.c,v 1.1 1993/04/14 17:19:30 nickc Exp $ */

#ifdef __STDC__
# include <queue.h>
#else
# include "queue.nonansi.h"
#endif


#if 1
/*only if searchlist/walklist are used */
# include <stdarg.h>
#endif
/*--------------------------------------------------------
-- InitList                                             --
--                                                      --
-- Initialise a list structure                          --
--                                                      --
--------------------------------------------------------*/

void InitList(list)
List *list;
{
	list->Head  = (Node *)&list->Earth;
	list->Earth = NULL;
	list->Tail  = (Node *)list;
}

#if 0
/*--------------------------------------------------------
-- PreInsert                                            --
--                                                      --
-- Insert a node before another                         --
-- Note that next must not be a List, but may point to  --
-- the Earth/Tail pair of a List structure.             --
--                                                      --
--------------------------------------------------------*/

void PreInsert(Node *next, Node *node)
{
	node->Prev	 = next->Prev;
	node->Next	 = next;
	next->Prev	 = node;
	node->Prev->Next = node;
}

/*--------------------------------------------------------
-- PostInsert                                           --
--                                                      --
-- Insert a node after another                          --
-- Note that pred may be a List structure.              --
--                                                      --
--------------------------------------------------------*/

void PostInsert(Node *prev, Node *node)
{
	node->Next	 = prev->Next;
	node->Prev	 = prev;
	prev->Next	 = node;
	node->Next->Prev = node;
}

#endif
/*--------------------------------------------------------
-- Remove                                               --
--                                                      --
-- Remove a node from its current list                  --
-- NOTE: It MUST be on list!!                           --
--                                                      --
--------------------------------------------------------*/

Node *Remove(node)
Node *node;
{
	node->Next->Prev = node->Prev;
	node->Prev->Next = node->Next;
	node->Next = NULL; node->Prev = NULL;
	return node;
}

#if 0
/*--------------------------------------------------------
-- AddHead                                              --
--                                                      --
-- Add a node to the head of the list                   --
--                                                      --
--------------------------------------------------------*/

void AddHead(List *list, Node *node)
{
	node->Next	 = list->Head;
	node->Prev	 = (Node *)list;
	list->Head->Prev = node;
	list->Head	 = node;
}

#endif

/*--------------------------------------------------------
-- AddTail                                              --
--                                                      --
-- Add the node to the tail of the list                 --
--                                                      --
--------------------------------------------------------*/

void AddTail(list, node)
List *list;
Node *node;
{
	node->Prev	 = list->Tail;
	node->Next	 = (Node *)&list->Earth;
	list->Tail->Next = node;
	list->Tail	 = node;
}

#if 1
/*--------------------------------------------------------
-- RemHead                                              --
--                                                      --
-- Remove the head node from the list, if there is one. --
--                                                      --
--------------------------------------------------------*/

Node *RemHead(List *list)
{
	Node *node = list->Head;
	if( node->Next == NULL ) return NULL;
	node->Next->Prev = (Node *)list;
	list->Head = node->Next;
	node->Next = NULL; node->Prev = NULL;
	return node;
}

/*--------------------------------------------------------
-- RemTail                                              --
--                                                      --
-- Remove tail node from the list, if there is one.     --
--                                                      --
--------------------------------------------------------*/

Node *RemTail(List *list)
{
	Node *node = list->Tail;
	if( node->Next == NULL ) return NULL;
	node->Prev->Next = (Node *)&list->Earth;
	list->Tail = node->Prev;
	node->Next = NULL; node->Prev = NULL;
	return node;
}

/*--------------------------------------------------------
-- WalkList						--
--							--
-- This function scans down the given list applying	--
-- the given function to each Node in turn. Takes care	--
-- to avoid problems is the node if removed from the	--
-- list by the function.				--
--							--
--------------------------------------------------------*/

word WalkList(List *list, WordFnPtr fn, ...)
{
	Node *node = list->Head;
	Node *next = node->Next;
	word sum = 0;
	word arg;
	va_list a;

	va_start(a,fn);
	arg = va_arg(a,word);
	
	while( next != NULL )
	{
		sum += fn(node,arg);
		node = next;
		next = node->Next;
	}
	va_end(a);
	return sum;
}

/*--------------------------------------------------------
-- SearchList						--
--							--
-- Similar to WalkList except that the result of the fn	--
-- is used. If it returns 0 the walk continues,		--
-- otherwise it is terminated. The terminating node, or	--
-- Null is returned.					--
--							--
--------------------------------------------------------*/


Node *SearchList(List *list, WordFnPtr fn, ... )
{
	Node *node = list->Head;
	Node *next = node->Next;
	word arg;
	va_list a;

	va_start(a,fn);
	arg = va_arg(a,word);
		
	while( next != NULL )
	{
		if( fn(node,arg) ) 
		{
			va_end(a);
			return node;
		}
		node = next;
		next = node->Next;
	}
	va_end(a);
	return NULL;
}
#endif

/* -- End of queue.c */
