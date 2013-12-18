/* $Id: queue.h,v 1.2 1994/06/07 12:34:02 nickc Exp $ */

#ifndef __queue_h
#define __queue_h

typedef struct List
  {
    struct Node	*	Head;		/* list head pointer		*/
    struct Node	*	Earth;		/* NULL pointer			*/
    struct Node	*	Tail;		/* list tail pointer		*/
  }
List;

typedef struct Node
  {
    struct Node	*	Next;		/* next item in list		*/
    struct Node	*	Prev;		/* previous item in list	*/
  }
Node;

#ifdef __STDC__
extern void InitList(List *);
extern void PreInsert(Node *, Node *);
extern void PostInsert(Node *, Node *);
extern Node *Remove(Node *);
extern void AddHead(List *, Node *);
extern void AddTail(List *, Node *);
extern Node *RemHead(List *);
extern Node *RemTail(List *);
extern long int WalkList(List *, long int (*)(),...);
extern Node *SearchList(List *,long int (*)(),...);
#else
extern void InitList();
extern void PreInsert();
extern void PostInsert();
extern Node *Remove();
extern void AddHead();
extern void AddTail();
extern Node *RemHead();
extern Node *RemTail();
extern long int WalkList();
extern Node *SearchList();
#endif

/* some useful macros */

#define EmptyList_(l) ((l).Head == (Node *)&((l).Earth))
#define EndOfList_(n) (((Node *)n)->Next == NULL)
#define Next_(type,n) ((type *)(((Node *)n)->Next))
#define Prev_(type,n) ((type *)(((Node *)n)->Prev))
#define Head_(type,l) ((type *)(((List)l).Head))
#define Tail_(type,l) ((type *)(((List)l).Tail))

#endif

/* -- End of queue.h */
