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
------------------------------------------------------------------------*/
/* SccsId: %W% %G% Copyright (C) 1987, Perihelion Software Ltd.	*/
/* $Id: queue.h,v 1.3 1993/08/18 16:15:18 nickc Exp $ */

#ifndef __queue_h
#define __queue_h

#ifndef __helios_h
#include <helios.h>
#endif


struct List 
{
	struct Node	*Head;		/* list head pointer		*/
	struct Node	*Earth;		/* NULL pointer			*/
	struct Node	*Tail;		/* list tail pointer		*/
};

#ifndef __cplusplus
typedef struct List List;
#endif

struct Node 
{
	struct Node	*Next;		/* next item in list		*/
	struct Node	*Prev;		/* previous item in list	*/
};

#ifndef __cplusplus
typedef struct Node Node;
#endif

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

/* some useful macros */

#define EmptyList_(l) ((l).Head == (Node *)&((l).Earth))
#define EndOfList_(n) (((Node *)(n))->Next == NULL)
#define Next_(type,n) ((type *)(((Node *)(n))->Next))
#define Prev_(type,n) ((type *)(((Node *)(n))->Prev))
#ifdef RS6000 /* XXX - xlc compiler cannot cope with cast to List type */
#define Head_(type,l) ((type *)((l).Head))
#define Tail_(type,l) ((type *)((l).Tail))
#else
#define Head_(type,l) ((type *)(((List)(l)).Head))
#define Tail_(type,l) ((type *)(((List)(l)).Tail))
#endif

#endif

/* -- End of queue.h */
