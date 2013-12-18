/*------------------------------------------------------------------------
--                                                                      --
--                     H E L I O S   N U C L E U S                      --
--                     ---------------------------                      --
--                                                                      --
--             Copyright (C) 1987, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- queue.h								--
--                                                                      --
--	
--                                                                      --
--	Author:  NHG 16/8/87						--
--                                                                      --
--                                                                      --
--   Non ANSI version for use with the linker on the orion              --
--                                                                      --
------------------------------------------------------------------------*/
/* SccsId: @(#)queue.h	1.4	19/12/88 Copyright (C) 1987, Perihelion Software Ltd.	*/
/* RcsId: $Id: queue.nonansi.h,v 1.1 1990/10/22 15:47:32 paul Exp $ */

#ifndef __queue_h
#define __queue_h

#ifndef __helios_h
#include <helios.h>
#endif


typedef struct List {
	struct Node	*Head;		/* list head pointer		*/
	struct Node	*Earth;		/* NULL pointer			*/
	struct Node	*Tail;		/* list tail pointer		*/
} List;

typedef struct Node {
	struct Node	*Next;		/* next item in list		*/
	struct Node	*Prev;		/* previous item in list	*/
} Node;

#ifdef __STDC__
PUBLIC void InitList(List *);
PUBLIC void PreInsert(Node *, Node *);
PUBLIC void PostInsert(Node *, Node *);
PUBLIC Node *Remove(Node *);
PUBLIC void AddHead(List *, Node *);
PUBLIC void AddTail(List *, Node *);
PUBLIC Node *RemHead(List *);
PUBLIC Node *RemTail(List *);
PUBLIC word WalkList(List *,WordFnPtr,...);
PUBLIC Node *SearchList(List *,WordFnPtr,...);
#else
PUBLIC void InitList();
PUBLIC void PreInsert();
PUBLIC void PostInsert();
PUBLIC Node *Remove();
PUBLIC void AddHead();
PUBLIC void AddTail();
PUBLIC Node *RemHead();
PUBLIC Node *RemTail();
PUBLIC word WalkList();
PUBLIC Node *SearchList();
#endif
/* some useful macros */

#define EndOfList_(n) (((Node *)n)->Next == NULL)
#define Next_(type,n) ((type *)(((Node *)n)->Next))
#define Prev_(type,n) ((type *)(((Node *)n)->Prev))
#define Head_(type,l) ((type *)(((List)l).Head))
#define Tail_(type,l) ((type *)(((List)l).Tail))

#endif

/* -- End of queue.h */
