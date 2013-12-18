/*------------------------------------------------------------------------
--                                                                      --
--                     H E L I O S   N U C L E U S                      --
--                     ---------------------------                      --
--                                                                      --
--            Copyright (C) 1987-1990, Perihelion Software Ltd.         --
--                        All Rights Reserved.                          --
--                                                                      --
-- syslib/misc.c							--
--                                                                      --
--	System Library internal support routines, plus a few external	--
--	routines which do not fit any other category.			--
--                                                                      --
--	Author:  NHG 16/8/87						--
--		 NHG 03/8/90						--
--                                                                      --
------------------------------------------------------------------------*/
/* $Id: misc.c,v 1.31 1993/07/09 13:36:31 nickc Exp $ */

#define _in_misc

#include "sys.h"
#include <root.h>
#include <config.h>
#include <stdarg.h>

static void AddObject(Object *o);
static void AddStream(Stream *s);
static WORD CheckObject(Object *obj, word options);
static WORD CheckStream(Stream *str, word options);
static WORD IOCMsg(MCB *m, void *data);
static word CloseObject(Object *object);

/*--------------------------------------------------------
--		Private Data Definitions 		--
--------------------------------------------------------*/

STATIC Semaphore	BufLock;	/* lock on buffer list		*/

STATIC MsgBuf		*IOCBuf;	/* IOC message buffer		*/

static word		IOCBufCount;	/* Number of buffers in use	*/

STATIC	Semaphore	ObjectLock;	/* lock on Object list		*/
			
STATIC List		Objects; 	/* list of Objects		*/

STATIC	Semaphore	StreamLock;	/* lock on Stream list		*/
			
STATIC	List		Streams;	/* list of Streams		*/

STATIC bool 		Terminating;	/* TRUE if program is quitting	*/

#ifdef SYSDEB
static Semaphore 	SysDebugLock;	/* serializes all SysDebugs	*/ 
#endif

/*--------------------------------------------------------
-- _SysLib_Init						--
--							--
-- Initialisation routine for syslib. This is called	--
-- from the module initialisation routine before the	--
-- program is entered.					--
--							--
--------------------------------------------------------*/

PRIVATE void _SysLib_Init(void)
{
	InitList(&Objects);
	InitList(&Streams);
	InitSemaphore(&ObjectLock,1);
	InitSemaphore(&StreamLock,1);
	
	InitSemaphore(&BufLock,1);
	IOCBuf = NULL;
	IOCBufCount = 0;

#ifdef SYSDEB
	InitSemaphore(&SysDebugLock,1);
#endif
	
	{
		extern Semaphore PipeLock;
		extern List PipeList;
		InitList(&PipeList);
		InitSemaphore(&PipeLock,1);
	}
}

/*--------------------------------------------------------
-- NewMsgBuf						--
-- FreeMsgBuf						--
--							--
-- Message buffer management. A single buffer is kept	--
-- permanently, but extras are allocated dynamically	--
-- when needed.						--
--							--
--------------------------------------------------------*/

static MCB *NewMsgBuf(int dsize)
{
	MsgBuf *m;

	if( dsize < IOCDataMax ) dsize = IOCDataMax;
retry:
	Wait(&BufLock);
	
	if( dsize == IOCDataMax && IOCBuf != NULL ) 
	{
		m = IOCBuf;
		IOCBuf = *(MsgBuf **)m;
	}
	else 
	{
		m = (MsgBuf *)Malloc(sizeof(MsgBuf) + (word)dsize - IOCDataMax);
		if( m == NULL )
		{
			Signal(&BufLock);
			Delay(OneSec);
			goto retry;
		}
		IOCBufCount++;
	}
	m->size = dsize;
	m->mcb.Control = m->control;
	m->mcb.Data = m->data;

#ifdef SYSDEB
	SysDebug(memory)("NewMsgBuf %x from %s",m,procname(returnlink_(dsize)));
#endif

	Signal(&BufLock);

	return &m->mcb;
}

static void FreeMsgBuf( MCB *mcb )
{
	MsgBuf *m = (MsgBuf *)mcb;
	
	if( m == NULL ) return;
	
#ifdef SYSDEB
	SysDebug(memory)("FreeMsgBuf %x from %s",mcb,procname(returnlink_(mcb)));
#endif

	if( m->size > IOCDataMax ) 
	{
		Free(m);
		IOCBufCount++;
	}
	else
	{
		Wait(&BufLock);
		*(MsgBuf **)m = IOCBuf;
		IOCBuf = m;
		Signal(&BufLock);
	}
}

/*
 * BLV - code to preallocate a set number of message buffers. In the
 * networking software during bootstrap and taskforce start-up there
 * are a large number of parallel system library calls going on, each
 * one resulting in a separate message buffer. These buffers are spread
 * over various heaps, resulting in horrible memory fragmentation. This
 * routine can be used to avoid such problems.
 */
void PreallocMsgBufs(int n)
{ MCB	*m	= Null(MCB);
  MCB	*New	= Null(MCB);

  while (n-- > 0)
   { New = NewMsgBuf(0);
     if (New == NULL) break;
     *(MCB **) New = m;
     m = New;
   }

  while (New != NULL)
   { m = New;
     New = *(MCB **)m;
     FreeMsgBuf(m);
   }
}

/*--------------------------------------------------------
-- CopyObject						--
--							--
-- make a copy of the given object, but allocate a new	--
-- reply port for it. An error is produced if the	--
-- object is closeable.					--
--							--
--------------------------------------------------------*/

PUBLIC Object *CopyObject(Object *source)
{
	word oblen;
	Object *obj = NULL;
	word rc = 0;
	
#ifdef SYSDEB
	SysDebug(ioc)("CopyObject(%O)",source);
#endif

	if( CheckObject(source,0) != Err_Null ) return NULL;

	if( source->Flags & Flags_Closeable ) 
	{
		rc = EC_Error|SS_SysLib|EG_WrongFn|EO_Object;
		goto Done;
	}

	oblen = sizeof(Object) + (word)strlen(source->Name) + SafetyMargin;

	obj = (Object *)Malloc(oblen);

	if( obj == Null(Object) ) 
	{
		rc = EC_Error+SS_SysLib+EG_NoMemory+EO_Object;
		goto Done;
	}
	else memcpy((byte *)obj,(byte *)source,(int)oblen);

	obj->Reply = NewPort();

	AddObject( obj );

Done:

#ifdef SYSDEB
	SysDebug(ioc)("CopyObject: %E %O",rc,obj);
#endif
	source->Result2 = rc;

	return obj;
}

/*--------------------------------------------------------
-- NewObject						--
--							--
-- Create a new object structure from the name and	--
-- capability provided. These should have been derived	--
-- from a capability initially. The new object is give	--
-- Type Pseudo. It will only be Located at the server	--
-- if/when it is first touched.				--
--							--
--------------------------------------------------------*/

Object *NewObject(string name, Capability *cap)
{
	Object *obj;
	word oblen = sizeof(Object) + (word)strlen(name) + SafetyMargin;

	obj = (Object *)Malloc(oblen);

	if( obj == NULL ) return NULL;

	memclr( (void *)obj, (int)oblen );

	obj->Type = Type_Pseudo;
	obj->Access = *cap;
	strcpy(obj->Name,name);

	AddObject( obj );
	
	return obj;
}

/*--------------------------------------------------------
-- ReLocate						--
--							--
-- Do a locate operation on the given object. This is	--
-- normally called from CheckObject to check a pseudo	--
-- object created by NewObject.				--
--							--
--------------------------------------------------------*/

extern word ReLocate( Object *obj )
{
	MCB *mcb;
	IOCReply1 *rep;
	Port reply = NewPort();
	word rc;
	
	mcb = NewMsgBuf(0);	
	rep = (IOCReply1 *)mcb->Control;
	
	InitMCB(mcb,MsgHdr_Flags_preserve,
		MyTask->IOCPort,reply,FC_GSP+FG_Locate);

	MarshalString(mcb,obj->Name);
	MarshalWord(mcb,-1);
	MarshalWord(mcb,1);
	MarshalCap(mcb,&obj->Access);
	
	if( (rc = IOCMsg(mcb,NULL)) >= Err_Null )
	{
		obj->Type = rep->Type;
		obj->Flags = rep->Flags;
		obj->Result2 = 0;
		obj->FnMod = rc;
		obj->Timeout = IOCTimeout;
		obj->Reply = reply;
		obj->Access = rep->Access;
		strcpy(obj->Name,mcb->Data+rep->Pathname);
	}
	else 
	{
		FreePort(reply);
		obj->Result2 = rc;
	}
	
#ifdef SYSDEB
	SysDebug(ioc)("ReLocate: %E %O",rc,obj);
#endif

	FreeMsgBuf(mcb);
	
	return rc;
}

/*--------------------------------------------------------
-- NewStream						--
--							--
-- Create a stream structure from the supplied 		--
-- parameters and then re-open it. This is used to 	--
-- open up streams which have been passed as name/cap	--
-- pairs.						--
--							--
--------------------------------------------------------*/

Stream *NewStream(string name,Capability *cap, word mode)
{
	Stream *stream;
	word stlen = sizeof(Stream) + (word)strlen(name) + SafetyMargin;

#ifdef SYSDEB
	SysDebug(stream)("NewStream(%N,%C,%x)",name,cap,mode);
#endif

	stream = (Stream *)Malloc(stlen);

	if( stream == NULL ) return NULL;

	stream->Server = NullPort;
	stream->Type = Type_Pseudo;
	stream->Flags = Flags_Stream|mode;
	stream->Access = *cap;
	stream->Reply = NewPort();
	InitSemaphore( &stream->Mutex, 1 );
	stream->Pos = 0;
	stream->FnMod = 0;
	stream->Timeout = IOCTimeout;
	strcpy(stream->Name,name);

	AddStream ( stream );	

	/* if the stream has the OpenOnGet bit set, open it here */
	if( mode & Flags_OpenOnGet )
	{
		ReOpen(stream);

		stream->FnMod &= ~FR_Mask;
	
		if( stream->Result2 < Err_Null )
		{
			Wait(&StreamLock);
			Remove( &stream->Node );	
			Signal(&StreamLock);	
			Free(stream);
			return NULL;
		}
	}

	return stream;	
}

/*--------------------------------------------------------
-- PseudoStream						--
--							--
-- Create a pseudo stream structure from the supplied	--
-- object and mode. This can be used in place of a	--
-- real stream structure for environments etc.		--
-- If we try to use it then it will be ReOpened.	--
--							--
--------------------------------------------------------*/

Stream *PseudoStream(Object *object, word mode)
{
	Stream *s;
#ifdef SYSDEB
	SysDebug(stream)("PseudoStream(%O,%x)",object,mode);
#endif

	if( CheckObject(object,0) != Err_Null ) return NULL;

#if 1
	s = NewStream(object->Name,&object->Access,object->Flags|mode);
#else	
	s = NewStream(object->Name,&object->Access,0);

	if( s == NULL ) return NULL;
	
	s->Flags |= object->Flags|mode;	
	
	/* For pipes, do a CloseObject call when they are closed */
	
	if( object->Type == Type_Pipe ) s->Flags |= Flags_Closeable;
#endif
	return s;
}

/*--------------------------------------------------------
-- CopyStream						--
--							--
-- Generate a complete copy of the given stream. If the	--
-- stream has a reply port, this is re-allocated.	--
-- This function is a little dangerous since we will end--
-- up with more Stream structures than the server is	--
-- aware of. To try and alleviate this I clear the	--
-- Closeable flag in the copy so it will close silently.--
--							--
--------------------------------------------------------*/

PUBLIC Stream *CopyStream(Stream *s)
{
	Stream *s1;
	int len;

#ifdef SYSDEB
	SysDebug(stream)("CopyStream(%x=%S) from %s",s,s,procname(returnlink_(s)));
#endif
	if( CheckStream(s,C_ReOpen) != Err_Null ) return NULL;

	len = sizeof(Stream) + strlen(s->Name) + SafetyMargin;

	s1 = (Stream *)Malloc(len);

	if( s1 == NULL )
	{ s->Result2 = EC_Error|SS_SysLib|EG_NoMemory|EO_Stream; return NULL; }

	memcpy(s1,s,len);

	if( s1->Reply != NullPort ) s1->Reply = NewPort();

	if( s->Type == Type_Pipe ) 
	{
		if( !CopyPipe(s,s1) )
		{
			FreePort(s1->Reply);
			Free(s1);
			s->Result2 = EC_Error|SS_SysLib|EG_InUse|EO_Pipe;
			return NULL;
		}
	}

	s1->Flags &= ~Flags_Closeable;
	
	InitSemaphore(&s1->Mutex,1);

	AddStream(s1);

#ifdef SYSDEB
	SysDebug(stream)("CopyStream: %x %S)",s1,s1);
#endif
	return s1;
}

/*--------------------------------------------------------
-- Close						--
--							--
-- Close the stream or object down. This is a poly-	--
-- morphic function which should be applied to all	--
-- streams and objects. This is only a hint so we	--
-- do not have to try too hard to get it through.	--
-- Note also that Close avoids getting hung up on any	--
-- semaphores.						--
-- 							--
--------------------------------------------------------*/

PUBLIC word
Close( Stream * stream )
{
	word e = Err_Null;
	Object *object = (Object *)stream;
#ifdef SYSDEB
	SysDebug(stream)("Close(%O) called from %s",object,procname(returnlink_(stream)));
#endif
	if( object == NULL ) return Err_Null;
	
	/* start by aborting any current operations on the stream/object */
	
	unless( Terminating ) Abort(object);

	if( closebits_(stream->Flags)&closebits_(Flags_Stream) )
	{	

		if((e = CheckStream(stream,C_Close)) != Err_Null) return e; 
		
		if( stream->Type == Type_Pipe ) PipeClose(stream);

		/* ensure all waiters are away from stream, then claim it ourself */
		while( TestWait(&stream->Mutex) == FALSE)
			Abort((Object *)stream);
	
		e = CloseStream(stream);

		stream->Type = 0;

		Free(stream);

	}
	else 
	{  
		if((e = CheckObject(object,C_Close)) != Err_Null) return e;
		
		e = CloseObject(object);

		object->Type = 0;

		Free(object);
	}

	return e;	
}

PRIVATE word CloseStream(Stream *stream)
{
	word e = Err_Null;

#ifdef SYSDEB
		SysDebug(stream)("Close %S flags: %x",stream,stream->Flags);
#endif

	if( stream->Type == Type_Pseudo ) goto close1;

	switch( closebits_(stream->Flags) )
	{	

	/* a served closeable stream */
	case closebits_(Flags_Stream|Flags_Closeable|Flags_Server):
	{
#ifdef SYSDEB
		SysDebug(stream)("Close Served Closeable Stream(%S)",stream);
#endif
		SendException(stream->Server,FC_GSP+FG_Close|stream->FnMod);
		break;
	}
	
	/* an un-served closeable stream */
	case closebits_(Flags_Stream|Flags_Closeable):
	{
		MCB mcb;
		word control[5];
		word e;
#ifdef SYSDEB
		SysDebug(stream)("Close UnServed Closeable Stream(%S)",stream);
#endif
		InitMCB(&mcb,MsgHdr_Flags_preserve,
			MyTask->IOCPort,NullPort,FC_GSP+FG_CloseObj|stream->FnMod);
		mcb.Control = control;
		mcb.Data = stream->Name;
		mcb.MsgHdr.DataSize = strlen(stream->Name)+1;
		MarshalWord(&mcb,0);
		MarshalWord(&mcb,-1);
		MarshalWord(&mcb,1);
		MarshalCap(&mcb,&stream->Access);

		e = PutMsg(&mcb);

		break;
	}
	
	/* a served stream which need not be closed */
	case closebits_(Flags_Stream|Flags_Server):
#ifdef SYSDEB
		SysDebug(stream)("Close Served Stream(%S)",stream);
#endif
		if( !Terminating ) FreePort(stream->Server);
		break;
			
	/* an un-served stream which need not be closed */
	case closebits_(Flags_Stream):
#ifdef SYSDEB
		SysDebug(stream)("Close UnServed Stream(%S)",stream);
#endif
		break;

	default:
#ifdef SYSDEB
		SysDebug(stream)("Close Impossible Stream(%S)",stream);
#endif
		return EC_Error+SS_SysLib+EG_Invalid+EO_Stream;

	} /* end of switch */

close1:
	if( !Terminating ) FreePort(stream->Reply);	
	return e;
}

static word CloseObject(Object *object)
{
	word e = Err_Null;

#ifdef SYSDEB
		SysDebug(ioc)("Close %O flags: %x",object,object->Flags);
#endif
	
	switch( closebits_(object->Flags) )
	{
	/* a closeable object */
	case closebits_(Flags_Closeable):
	{
		MCB mcb;
		word control[5];
#ifdef SYSDEB
		SysDebug(ioc)("Close Closeable Object(%O)",object);
#endif
		InitMCB(&mcb,MsgHdr_Flags_preserve,
			MyTask->IOCPort,NullPort,FC_GSP+FG_CloseObj|object->FnMod);
		mcb.Control = control;
		mcb.Data = object->Name;
		mcb.MsgHdr.DataSize = strlen(object->Name)+1;
		MarshalWord(&mcb,0);
		MarshalWord(&mcb,-1);
		MarshalWord(&mcb,1);
		MarshalCap(&mcb,&object->Access);

		PutMsg(&mcb);

		break;
	}
	

	/* an object which need not be closed */
	case closebits_(0):
#ifdef SYSDEB
		SysDebug(ioc)("Close Normal Object(%O)",object);
#endif
		break;

	default:
#ifdef SYSDEB
		SysDebug(ioc)("Close Impossible Object(%O)",object);
#endif
		return EC_Error+SS_SysLib+EG_Invalid+EO_Object;

	} /* end of switch */

	if( !Terminating ) FreePort(object->Reply);

	return e;
}


/*--------------------------------------------------------
-- Result2						--
--							--
-- Extract error code/second result from object struct	--
-- 							--
--------------------------------------------------------*/

PUBLIC WORD
Result2( Object * object )
{
	return object->Result2;
}

/*--------------------------------------------------------
-- Abort						--
--							--
-- Abort operations on the given object/stream.		--
-- 							--
--------------------------------------------------------*/

PUBLIC WORD
Abort( Object * object )
{
	Stream *stream = (Stream *)object;
	word e = EC_Error|SS_SysLib|EG_Exception|EE_Abort;
	
#ifdef SYSDEB
	SysDebug(ioc)("Abort(%O)",object);
#endif
	switch( closebits_(object->Flags) )
	{
	case closebits_(Flags_Stream|Flags_Closeable|Flags_Server):
	case closebits_(Flags_Stream|Flags_Server):
		if( stream->Type == Type_Pipe ) PipeAbort(stream);
		else AbortPort(stream->Server,e);	
		/* drop through */
	case closebits_(Flags_Stream|Flags_Closeable):
	case closebits_(Flags_Stream):
	case closebits_(Flags_Closeable):
	case closebits_(0):
		AbortPort(stream->Reply, e);
		/* Free the reply port here. This will be re-allocated in */
		/* CheckStream/CheckObject if needed.			  */
		FreePort(stream->Reply);
		return Err_Null;

	default:
		return EC_Error+SS_SysLib+EG_Invalid+EO_Object;
	}
}

/*--------------------------------------------------------
-- AddObject						--
-- AddStream						--
--							--
-- Add a new object/stream to appropriate list. They	--
-- will be removed by CheckObject/Stream.		--
--							--
--------------------------------------------------------*/

static void AddObject(Object *o)
{
	Wait(&ObjectLock);
	AddTail( &Objects, &o->Node );
	Signal(&ObjectLock);	
}

static void AddStream(Stream *s)
{
	Wait(&StreamLock);
	AddTail( &Streams, &s->Node );
	Signal(&StreamLock);	
}

/*--------------------------------------------------------
-- CheckObject						--
-- CheckStream						--
--							--
-- Check the integrity of an object or stream struct	--
--							--
--------------------------------------------------------*/

static WORD CheckObject(Object *obj, word options)
{
	Object *o;
	PortInfo p;
	
	if( obj == NULL ) goto fail;

	Wait(&ObjectLock);

	for( o = Head_(Object,Objects) ; !EndOfList_(o) ; o = Next_(Object,o) )
	{
		if( o == obj )
		{
			if( options & C_Close ) Remove(&obj->Node);
			break;
		}
	}
	
	Signal(&ObjectLock);

	if( o != obj || o->Type == 0 ) goto fail;

	if( (options & C_Locate) && o->Type == Type_Pseudo ) return ReLocate(o);
	elif( GetPortInfo(o->Reply,&p) != Err_Null ) o->Reply = NewPort();
	
	return Err_Null;

fail:
#ifdef SYSDEB
	SysDebug(error)("Invalid object passed to %s: %x %O",
		procname(returnlink_(obj)),obj,obj);
#endif
	return EC_Error|SS_SysLib|EG_Invalid|EO_Object; 
}

static WORD CheckStream(Stream *str, word options)
{
	Stream *s;
	PortInfo p;
	
	if( str == NULL ) goto fail;

	Wait(&StreamLock);

	for( s = Head_(Stream,Streams) ; !EndOfList_(s) ; s = Next_(Stream,s) )
	{
		if( s == str ) 
		{
			if( options & C_Close ) Remove(&str->Node);
			break;
		}
	}

	Signal(&StreamLock);

	if( s != str || s->Type == 0 ) goto fail;

	if( (options & C_ReOpen) && s->Type == Type_Pseudo )
	 { word result;
	   Wait(&(s->Mutex));
	   if (s->Type == Type_Pseudo)
	    result = ReOpen(s);
	   else
	    result = Err_Null;
	   Signal(&(s->Mutex));
	   return(result);
	  }
	elif( !(options & C_Close) && GetPortInfo(s->Reply,&p) != Err_Null ) 
	 s->Reply = NewPort();

	return Err_Null;

fail:
#ifdef SYSDEB
	SysDebug(error)("Invalid stream passed to %s: %x %S",
			procname(returnlink_(str)),str,str);
#endif

	return EC_Error|SS_SysLib|EG_Invalid|EO_Stream; 
}

/*--------------------------------------------------------
-- SendIOC						--
--							--
-- Send a pre-constructed message to the IOC.		--
--							--
--------------------------------------------------------*/

PUBLIC void SendIOC(MCB *m)
{

#ifdef SYSDEB
	SysDebug(ioc)("SendIOC(%M)",m);
#endif

	m->MsgHdr.Dest = MyTask->IOCPort;

	PutMsg(m);
}

/*--------------------------------------------------------
-- SendMsg						--
--							--
-- Transmit a message using the procedure arguments as  --
-- an mcb.						--
--							--
--------------------------------------------------------*/

PUBLIC WORD SendMsg(word flagsize, ...)
{
	MCB *mcb = (MCB *)(&flagsize);
#ifdef SYSDEB
	SysDebug(process)("SendMsg(%M)",mcb);
#endif
	return PutMsg(mcb);
}

/*--------------------------------------------------------
-- XchMsg						--
--							--
-- Transmit a message and then receive a reply using	--
-- the same MCB.					--
--							--
--------------------------------------------------------*/

PUBLIC WORD
XchMsg1( MCB * mcb )
{
	return XchMsg(mcb,0);
}

/*--------------------------------------------------------
-- IOCMsg						--
--							--
-- Send a message to the IOC and cope with errors and	--
-- retries.						--
--							--
--------------------------------------------------------*/

static WORD IOCMsg(MCB *m, void *data)
{
	word r;
	MCB t;
	word time;

	t = *m;			/* struct copy of MCB */

	t.Timeout = OneSec;	/* tx should finish v. fast */

	/* set up client MCB for rx */
	m->MsgHdr.Dest = m->MsgHdr.Reply;	
	if( data != NULL ) m->Data = (byte *)data;
	
	forever
	{
	  extern word _ldtimer( int );
	  
		time = _ldtimer(0);
		
		r = XchMsg(&t,m);
		
		time = _ldtimer(0) - time;

		
		/* if the result is successful, or an error too serious	*/
		/* for us to handle here, return.			*/
		if( (r >= Err_Null) || ((r&EC_Mask) >= EC_Error) ) goto done;

		/* otherwise increment the retry count and do it again	*/
		t.MsgHdr.FnRc = (t.MsgHdr.FnRc & ~FR_Mask) |
				((t.MsgHdr.FnRc+FR_Inc) & FR_Mask);

		/* we just loop until we succeed or we get a real error	*/
#ifdef SYSDEB
		SysDebug(error)("IOCMsg Retry: error %E new fn %F",r,t.MsgHdr.FnRc);
#endif

	}
done:
	m->Timeout = time;
	
	return r;
}

PRIVATE WORD StreamMsg(MCB *mcb, Stream *stream)
{
	word e;
	MCB tx;
	
	tx = *mcb;
	mcb->MsgHdr.Dest = mcb->MsgHdr.Reply;	
	
	while( ( e = XchMsg(&tx,mcb) ) < Err_Null )
	{
#ifdef SYSDEB
		SysDebug(error)("StreamMsg XchMsg Error %x",e);
#endif	
		/* Recoverable errors are handled by XchMsg. 	*/
		/* Anything more serious than a warning should	*/
		/* be reported. Warnings can be handled here by	*/
		/* ReOpening the stream.			*/
		if( (e & EC_Mask) > EC_Warn ) break;

		if( (e = ReOpen(stream)) < Err_Null ) break;
		
		/* ReOpen can re-assign the ports */
		tx.MsgHdr.Dest = stream->Server;
		tx.MsgHdr.Reply = mcb->MsgHdr.Dest = stream->Reply;
	}
	
	return e;
}

/*--------------------------------------------------------
-- NewPort						--
-- FreePort						--
--							--
-- Routines to allocate and free ports from the kernel	--
-- and to keep track of them.				--
-- 							--
--------------------------------------------------------*/

#ifdef __TRAN
PUBLIC Port _SysNewPort(void) { return NewPort(); }
PUBLIC void _SysFreePort(Port p) { FreePort(p); }
#endif

#ifdef __TRAN
/*--------------------------------------------------------
-- BootLink						--
--							--
-- Boot the processor through a given link.		--
--							--
--------------------------------------------------------*/

PUBLIC WORD BootLink(word linkid, void *image, Config *config, word confsize)
{
	word rc = Err_Null;
	MCB *mcb;
	Port reply = NewPort();
	LinkInfo *link = GetRoot()->Links[linkid];

#ifdef SYSDEB
	SysDebug(process)("BootLink(%d,%P,%P,%d)",linkid,image,config,confsize);
#endif
	
	if( linkid < 0 || linkid > 3 ) return EC_Error|SS_SysLib|EG_Invalid|EO_Link;

	mcb = NewMsgBuf(0);
	
	InitMCB(mcb,0,link->LocalIOCPort,reply,FC_Private|FG_BootLink);

	MarshalData(mcb,confsize,(byte *)config);

	rc = XchMsg(mcb,0);

#ifdef SYSDEB
	SysDebug(process)("BootLink: %E",rc);
	if( mcb->MsgHdr.Reply != NullPort ) SysDebug(error)("BootLink: Non-Null Reply port %x",mcb->MsgHdr.Reply);
#endif
	if( mcb->MsgHdr.Reply != NullPort ) FreePort(mcb->MsgHdr.Reply);
	FreePort(reply);

	FreeMsgBuf(mcb);
	
	return rc;
}
#endif /* __TRAN */

#ifdef SYSDEB

static char SysDebugMsg[128];
static char SysDebugMcName[50];
word SysDebugArgs[10];

static void _SysDebug(char *str, ... )
{
	Wait(&SysDebugLock);
	
	/* If any flags except info are set, output message */
/*	if( MyTask->Flags != Task_Flags_info) */
	{
		int i;
		va_list a;
		
		va_start(a,str);
		
		for( i = 0 ; i < 10 ; i++ ) SysDebugArgs[i] = va_arg(a,int);
		
		if( SysDebugMcName[0] == 0 ) 
		{
			word of = MyTask->Flags;
			MyTask->Flags = 0;
			MachineName(SysDebugMcName);
			MyTask->Flags = of;
		}
		
		strcpy(SysDebugMsg,SysDebugMcName);
		strcat(SysDebugMsg,"-");
		strcat(SysDebugMsg,(char *)(MyTask->TaskEntry)+8);
		strcat(SysDebugMsg,": ");
		strcat(SysDebugMsg,str);
		
		IOdebug(SysDebugMsg,SysDebugArgs[0],SysDebugArgs[1],
			SysDebugArgs[2],SysDebugArgs[3],SysDebugArgs[4],
			SysDebugArgs[5],SysDebugArgs[6],SysDebugArgs[7],
			SysDebugArgs[8],SysDebugArgs[9]);
	}
	
	/* If info is set print Objects & Streams and clear bit	*/
	if(MyTask->Flags&Task_Flags_info)
	{
		Object *o;
		Stream *s;

		MyTask->Flags ^= Task_Flags_info;
		
		Wait(&ObjectLock);
		for( o = Head_(Object,Objects) ; !EndOfList_(o) ; o = Next_(Object,o) )
			IOdebug("%O",o);
		Signal(&ObjectLock);

		Wait(&StreamLock);
		for( s = Head_(Stream,Streams) ; !EndOfList_(s) ; s = Next_(Stream,s) )
			IOdebug("%S",s);
		Signal(&StreamLock);

		IOdebug("Message Buffers: %d",IOCBufCount);

		IOdebug("Memory: free %d largest %d",Malloc(-1),Malloc(-2));
		
	}
	
	Signal(&SysDebugLock);
}

#endif

/* end of misc.c */
