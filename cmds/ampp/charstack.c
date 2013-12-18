/****************************************************************/
/* File: charstack.c                                            */
/*                                                              */
/* Maintains and manipulates character stacks both for macro    */
/* definitions and the putback facility                         */
/*                                                              */
/* Author: NHG 17-Feb-87                                        */
/****************************************************************/
#ifdef __TRAN
static char RcsId[] = "$Id: charstack.c,v 1.2 1993/08/12 16:49:20 nickc Exp $ Copyright (C) Perihelion Software Ltd.";
#endif
  
#include "ampp.h"

struct List freeq;

PUBLIC void initcs()
{
        InitList(&freeq);
}

/********************************************************/
/* addch                                                */
/*                                                      */
/* Add a character to a stack.                          */
/*                                                      */
/********************************************************/

PUBLIC void addch(
		  struct List *queue,
		  BYTE ch )
{
        struct Charbuf *buf = (struct Charbuf *)(queue->Head);
        INT size;

        if ( buf->node.Next == NULL || buf->size == charbuf_max )
        {
                buf = (struct Charbuf *)RemHead(&freeq);
                if( buf == NULL ) buf = New(struct Charbuf);
                AddHead(queue,(Node *)buf);
                size = 0;
        }
        else size = buf->size;

        buf->text[size] = ch;
        buf->size = size+1;
}

/********************************************************/
/* popchar                                              */
/*                                                      */
/* Remove the last char added to the stack              */
/*                                                      */
/********************************************************/

PUBLIC INT popch(struct List *queue)
{
        struct Charbuf *buf = (struct Charbuf *)(queue->Head);
        INT size;
        INT ch;

        if( buf->node.Next == NULL || buf->size <= 0 ) return -1;

        size = buf->size - 1;
        ch = buf->text[size];

        if( size == 0 )
        {
                AddHead(&freeq,RemHead(queue));
        }
        else buf->size = size;

        return( ch );
}

/********************************************************/
/* freebuf                                              */
/*                                                      */
/* Free all the buffers on the queue                    */
/*                                                      */
/********************************************************/

PUBLIC void freebuf(struct List *queue)
{
        struct Node *node;
        while( (node = RemHead(queue) )!= NULL ) AddHead(&freeq,node);
}
