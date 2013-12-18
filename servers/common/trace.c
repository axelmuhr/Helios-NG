/* $Header: /hsrc/servers/common/RCS/trace.c,v 1.1 1991/03/04 13:15:43 paul Exp $ */
/* $Source: /hsrc/servers/common/RCS/trace.c,v $ */
/************************************************************************/ 
/* trace.c - Routines to maintain an in-core trace buffer		*/
/*									*/
/* Copyright (C) 1990 Active Book Company Ltd., Cambridge, England	*/
/*									*/
/* Author: Brian Knight,  26th January 1990				*/
/************************************************************************/

#include <syslib.h>
#include <dev/trace.h>

#define TIMEWORD ((int *)0x1C8) /* I know Jamie keeps the time here! */

/*----------------------------------------------------------------------*/

int InitTraceBuf(TraceBuf *buf, int size)
{
  buf->buf = Malloc((int)(sizeof(TraceEntry) * size));
  if (buf->buf == 0) return 0;

  buf->size    = size;
  buf->next    = 0;
  buf->wrapped = 0;
  return 1;
}

/*----------------------------------------------------------------------*/

extern void FreeTraceBuf(TraceBuf *buf)
{
  Free(buf->buf);
  buf->buf     = 0;
  buf->size    = 0;
  buf->next    = 0;
  buf->wrapped = 0;
}

/*----------------------------------------------------------------------*/

/* Record one trace event. Should disable interrupts really */
void Trace(TraceBuf *buf, int event, int value)
{
  TraceEntry *entry = &buf->buf[buf->next];
  int newNext       = buf->next + 1;

  if (newNext < buf->size)
    buf->next = newNext;
  else
    { buf->next = 0; buf->wrapped = 1; }

  entry->time  = *TIMEWORD; /* Peek time out of executive */
  entry->event = event;
  entry->value = value;
}

/*----------------------------------------------------------------------*/

/* End of trace.c */
