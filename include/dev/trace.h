/* $Header: /dsl/HeliosARM/nick/RCS/trace.h,v 1.1 1991/03/03 22:15:36 paul Exp $ */
/* $Source: /dsl/HeliosARM/nick/RCS/trace.h,v $ */
/************************************************************************/ 
/* trace.h - Routines to maintain an in-core trace buffer		*/
/*									*/
/* Copyright (C) 1990 Active Book Company Ltd., Cambridge, England	*/
/*									*/
/* Author: Brian Knight,  26th January 1990				*/
/************************************************************************/

#ifndef __TRACE_H
#define __TRACE_H

/* Each entry in the buffer consists of 3 words in the following format */
typedef struct TraceEntry
{
  unsigned int time;	/* Timestamp in some units */
  int          event;	/* What happened */
  int	       value;	/* Further info  */
} TraceEntry;

/* Structure describing a trace buffer */
typedef struct TraceBuf
{
  TraceEntry *buf;	/* Base of circular trace buffer */
  int        size;	/* Size of buf as number of TraceEntry slots */
  int	     next;	/* Next slot to be used */
  int 	     wrapped;	/* Non-zero if circular buffer has wrapped round */
} TraceBuf;

extern int  InitTraceBuf(TraceBuf *buf, int size); /* Returns 0 if fails */
extern void FreeTraceBuf(TraceBuf *buf);
extern void Trace(TraceBuf *buf, int event, int value);

#endif /* __TRACE_H */

/* End of trace.h */
