/*------------------------------------------------------------------------
--                                                                      --
--                      H E L I O S   K E R N E L                       --
--                      -------------------------                       --
--                                                                      --
--             Copyright (C) 1988, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- event.c								--
--                                                                      --
--	Access to external event sources.				--
--	On the transputer this only accesses the event channel,	on	--
--	other processor types this will give access to interrupt	--
--	vectors.							--
--                                                                      --
--	Author:  NHG 8/8/88						--
--                                                                      --
------------------------------------------------------------------------*/
/* $Id: event.c,v 1.11 1993/11/09 12:16:55 nickc Exp $ */


#define __in_event 1	/* flag that we are in this module */

#include "kernel.h"
#include <stdarg.h>
#include <event.h>

#ifndef TESTER /* transputer test exec - ignores entire source */

#ifdef __TRAN
 void EventProcess(RootStruct *root);
 void CallEvent(Event *event);
#else /* __ARM, __C40, etc */
 void RootEventHandler(word priority);
 word CallEvent(Event *event, word priority);
#endif


/*--------------------------------------------------------
-- EventInit						--
--							--
-- Initialise event system.				--
--							--
--------------------------------------------------------*/

#ifndef __TRAN
void EventInit(Config *config)
{
	RootStruct *root = GetRoot();
	int i;

	for(i=0; i < InterruptVectors; i++) {
		InitList(&root->EventList[i]);
	}

	/* Executive assembler function to initialise */
	/* interrupt vectors to call the root event handler */
	InitEventHandler(RootEventHandler);

#ifdef __ABC
	/* initialise user event handling system */
	for(i=0; i < UserVectors; i++)
		InitList(&root->UserEventList[i]);
#endif

	config = config;
}
#else /*XPUTER*/
void EventInit(Config *config)
{
	RootStruct *root = GetRoot();
	
	InitList(&root->EventList);
	
	root->EventCount = 0;	
	
	NewWorker(EventProcess,root);	
	
	config = config;
}
#endif


/*--------------------------------------------------------
-- EventProcess						--
--							--
-- Process to await events and call down event chain	--
--							--
--------------------------------------------------------*/

#ifdef __TRAN
void EventProcess(RootStruct *root)
{
	word junk;
	Channel *eventchan = (Channel *)(MinInt + 0x20);

	*eventchan = MinInt;

	forever
	{
# ifdef TRANSPUTER
		in_(1,eventchan,&junk);
# else
		Stop();
# endif
		root->EventCount++;
		
		WalkList(&root->EventList, CallEvent);
	}
}

void CallEvent(Event *event)
{
	CallWithModTab((word)event->Data,(word)event,event->Code,event->ModTab);
}

#else /* __ARM, __C40, etc */

/*
** The root event handler could be called directly from an interrupt vector,
** assuming that the vector number is passed automatically as the first arg.
** In some systems this will have to be set up via small piece of jacket
** code that will just set the first arg to the vectors number and then jmp
** to the root handler. In other systems the executive may process the
** interrupt first, detecting the interrupt's source. The source then being
** passed as the vector number to the root handler.
**
** Event handlers are executed within a limited environment. Handler writers
** should be aware that they are being executed in the processors interrupt
** mode using the system stack. Handlers should avoid being suspended by
** for example, calling any message primitives. 
**
** When an event handler decides that an event is its responsibility,
** it should clear the interrupt, optionally HardenedSignal() a server
** process to perform any further work and then return TRUE to stop other
** event handlers being called.
**
** If data is read off a device, then it should be stored in a linked
** list structure within Event->Data. This allows multiple interrupts to occur
** before the server process is rescheduled. The server should use
** AvoidEvents() when Remove()ing from the linked list.
**
** Note that in non transputer versions, the handlers are called assuming
** the following prototype:
**
**	word handler_function(void *EventData, word vector) ;
**
** i.e. the second argument is their vector number rather than a pointer to
** their event structure that is passed in the transputer version.
*/

void RootEventHandler(word vector)
{
	RootStruct *root = GetRoot();
	Event *e = Head_(Event, root->EventList[vector]);

	root->EventCount++;

	/* Call all event handlers, for this vector, scan will stop as soon */
	/* as an event handler returns TRUE */

	while (	!EndOfList_(e) ) {
#ifdef __C40
		_SetAddrBase(e->AddrBase);
#endif
		if (CallWithModTab((word)e->Data, vector, e->Code, e->ModTab) == TRUE )
			return;
		e = Next_(Event, e);
	}
}
#endif


/*--------------------------------------------------------
-- SetEvent						--
--							--
-- Add an event to the event chain			--
--							--
--------------------------------------------------------*/

Code _SetEvent(Event *event);

Code SetEvent(Event *event)
{
	return (Code)System(_SetEvent,event);
}

Code _SetEvent(Event *event)
{
	RootStruct *root = GetRoot();
#ifdef __TRAN
	Event *next = Head_(Event,root->EventList);
#else
	Event *next = Head_(Event,root->EventList[event->Vector]);
#endif
	while( !EndOfList_(next) ) {
		if ( next->Pri >= event->Pri ) break;
		next = Next_(Event,next);
	}

#ifdef __TRAN
	event->ModTab = *((word ***)(&event))[-1]; /* xputer magic to get modtab */
#else /* __ARM, __C40, etc */
# ifdef __C40
	event->AddrBase = _GetAddrBase();
# endif
	event->ModTab = _GetModTab();	
	IntsOff();
#endif

	PreInsert(&next->Node,&event->Node);

#ifndef __TRAN
	IntsOn();
#endif

	return Err_Null;
}


/*--------------------------------------------------------
-- RemEvent						--
--							--
-- Remove an event from chain.				--
--							--
--------------------------------------------------------*/

Code RemEvent(Event *event)
{
#ifdef __TRAN
	System((WordFnPtr)Remove,event);
#else
	IntsOff();
		Remove((Node *)event);
	IntsOn();
#endif

	return Err_Null;
}


#ifndef __TRAN /* @@@ perhaps call System() as the TRAN equivalent? */
/* Call a function with interrupts disabled for the duration of the call */
word AvoidEvents(WordFnPtr fn, ...)
{
	va_list	ap;
	word	a, b, c;
	word	r;

	/* grab arguments */
	va_start(ap, fn);
		a = va_arg(ap, word);
		b = va_arg(ap, word);
		c = va_arg(ap, word);
	va_end(ap);

	IntsOff();
		r = fn(a,b,c);
	IntsOn();

	return r;
}
#endif


# ifdef __ABC
/*
** CauseUserEvent:
**
** Simulate an interrupt on one of the user event lists.
**
** This is a light-weight mechanism to support call back handlers on a per
** processor basis. It can be used for such purposes as reporting critical
** memory space conditions, etc to interested parties.
**
** Handler writers should be aware that they will be executing with the
** stack of the process that called CauseUserEvent(), or put another way,
** Callers of CallUserEvent() should be aware that handlers will use its
** stack! For this reason, handlers should be as simple as possible, possibly
** just signal()ing (HardenedSignal() not required) a thread within their task.
**
** The handlers are called assuming the following prototype:
**
**	word handler_function(void *EventData, word argument);
**
** The second argument passed to the handler can be encoded in any way
** required, i.e. it could be a pointer to a structure, or simply a
** protocol word.
**
** User event handlers should always return FALSE unless they wish to
** stop the remaining event handlers on list(s) being called.
*/

void CauseUserEvent(word vector, word argument)
{
	RootStruct *root = GetRoot();
	int i;

	if (vector >= 0 && vector < UserVectors)
		SearchList(&root->UserEventList[vector], CallEvent, argument);
	else
	{
		if (vector != -1)
			return;

		for(i=0; i < UserVectors; i++)
			if (SearchList(&root->UserEventList[i], CallEvent, i))
				break;
	}
}


/*--------------------------------------------------------
-- SetUserEvent						--
--							--
-- Add an event to a user event list			--
--							--
--------------------------------------------------------*/

static Code _SetUserEvent(Event *event);

Code SetUserEvent(Event *event)
{
	return (Code)System(_SetUserEvent,event);
}

static Code _SetUserEvent(Event *event)
{
	RootStruct *root = GetRoot();
	Event *next = Head_(Event,root->UserEventList[event->Vector]);

	while( !EndOfList_(next) )
	{
		if( next->Pri > event->Pri ) break;
		next = Next_(Event,next);
	}

	event->ModTab = _GetModTab();

	IntsOff();
		PreInsert(&next->Node,&event->Node);
	IntsOn();

	return Err_Null;
}


/*--------------------------------------------------------
-- RemUserEvent						--
--							--
-- Remove an event from a user event list.		--
--							--
--------------------------------------------------------*/

Code RemUserEvent(Event *event)
{
	IntsOff();
		Remove(event);
	IntsOn();
	
	return Err_Null;
}

# endif /* __ABC*/

#endif /* TESTER */


/* -- End of event.c */
