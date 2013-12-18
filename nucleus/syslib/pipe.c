/*------------------------------------------------------------------------
--                                                                      --
--                     H E L I O S   N U C L E U S                      --
--                     ---------------------------                      --
--                                                                      --
--            Copyright (C) 1987,1990, Perihelion Software Ltd.         --
--                        All Rights Reserved.                          --
--                                                                      --
-- syslib/pipe.c							--
--                                                                      --
--	Point-to-point pipe protocol handling				--
--									--
--                                                                      --
--	Author:  NHG 16/8/87						--
--		 NHG 03/8/90						--
--                                                                      --
------------------------------------------------------------------------*/
/* SccsId:	 %W%	%G% Copyright (C) 1987, Perihelion Software Ltd.*/
/* $Id: pipe.c,v 1.32 1993/08/06 12:42:03 bart Exp $ */

#define _in_pipe

#include "sys.h"
#include <nonansi.h>
#include <process.h>

/*----------------------------------------------------------------
--	Stack Handling						--
----------------------------------------------------------------*/

#if defined(__TRAN) && !defined(SYSDEB)
	/* Transputer Helios does not have automatic stack extension.	*/
# define pipestack 2000
#else
#if defined(STACKEXTENSION)
	/* Some processors have automatic stack extension support.	*/
	/* N.B. stack checking must be used !!!				*/
#define pipestack	800

#ifndef STACKCHECK
#error Stack checking must be enabled when compiling this module.
#endif

#else
	/* This size should be used on the transputer when debugging	*/
	/* is enabled, on any processor without automatic stack		*/
	/* extension, or when porting to a new processor.		*/
#  define pipestack 4000
# endif
#endif

#if defined(STACKCHECK) && defined(__TRAN)
extern void _Trace(...);
#pragma -s1

static void _stack_error(Proc *p)
{
	_Trace(0xaaaaaaaa,p);
	IOdebug("Loader stack overflow in %s at %x",p->Name,&p);	
}
#pragma -s0
#endif

#define RemoteBit	0x00000008

#define FG_NewPort	(FG_Abort+0x10)

#define Code_Connect	0xa1
#define Code_Timeout	0xa2
#define Code_Close	0xa3
#define Code_Connect1	0xa4

#define TfrInc		IOCDataMax
#define MaxPipeTfr	(60*1024)
#define InitPipeTfr	MaxPipeTfr

struct P
{
	MCB *	req;
	Port	ack;
	uword	pos;
};

typedef struct PipeInfo
{
	Node		Node;
	int		State;
	Stream *	Stream1;
	Stream *	Stream2;
	Port		ServerPort;
	Port		DataPort;
	Port		RemotePort;
	struct
	{
		byte *	buf;
		word	max;
		word	size;
		word	read;
		word	got;
		word	newmax;
	} Pending;
	struct P	Read, Write;
	Port		ReadSelect;
	Port		WriteSelect;
	MCB *		PendingWrite;
	word		MaxTfr;
	uword		ReadPos;
	uword		WritePos;
} PipeInfo;


STATIC Semaphore PipeLock;

STATIC List PipeList;

static MCB *DoPipeWrite( PipeInfo *p, MCB *mcb);
static MCB *ExchangeData(PipeInfo *p);
static Port InitProtocol(PipeInfo *p);

#ifdef SYSDEB
#pragma -s0
#endif

static MCB *DoPipeRead(PipeInfo *p, MCB *mcb)
{
	ReadWrite *rw = (ReadWrite *)mcb->Control;
	word size = rw->Size;
	uword pos = rw->Pos;
	
#ifdef SYSDEB
	SysDebug(pipe)("%x PipeRead %d pos %d pending %d",p,size,rw->Pos,p->Pending.size);
#endif

	if( p->Read.req != NULL )
	{
		FreeMsgBuf(p->Read.req);
		p->Read.req = NULL;
	}

	/* Adaptive message sizes...					  */
	/* If the pos of this read is the same as the last one we saw, it */
	/* must be a retry. If so, reduce the maximum transfer size. If	  */
	/* not, and the current tfr size is < max, increase it. Note that */
	/* while the size is decreased multiplicatively, is is only	  */
	/* increased additively.					  */
	if( p->ReadPos == pos )
	{
		p->MaxTfr /= 2;
		if( p->MaxTfr < TfrInc ) p->MaxTfr = TfrInc;
	}
	else
	{
		/* pos can be > than ReadPos because it may be advanced	*/
		/* by writes. It should be impossible for it to be <	*/
		if( p->MaxTfr < MaxPipeTfr ) p->MaxTfr += TfrInc;
	}
	
	if( p->WriteSelect != NullPort )
	{
		SendException(p->WriteSelect,O_WriteOnly);
		p->WriteSelect = NullPort;
	}

	if( p->Pending.size > 0 )
	{
		MCB m;
		word tfr;

		p->Read.req = NULL;
		InitMCB(&m,0,mcb->MsgHdr.Reply,NullPort,ReadRc_EOD);
		tfr = size;
		if( tfr > p->Pending.size ) tfr = p->Pending.size;
		if( tfr > p->MaxTfr ) tfr = p->MaxTfr;
		m.Data = p->Pending.buf+p->Pending.read;
		m.MsgHdr.DataSize = (unsigned short)tfr;
		PutMsg(&m);

		p->Read.ack = mcb->MsgHdr.Reply;
		p->Read.pos = pos;

		return mcb;
	}

	p->Read.req = mcb;	

	if( p->Write.req ) return ExchangeData(p);
	elif( rw->Timeout == 0 || (p->State & O_WriteOnly) == 0 )
	{
		word rc = (p->State & O_WriteOnly)?(EC_Recover|SS_Pipe|EG_Timeout|EO_Pipe):ReadRc_EOF;

		InitMCB(mcb,0,mcb->MsgHdr.Reply,NullPort,rc);
		PutMsg(mcb);
		p->Read.ack = mcb->MsgHdr.Reply;
		p->Read.req = NULL;
		p->Read.pos = pos;

		return mcb;
	}
	
	return NULL;
}

static MCB *DoReadAck(PipeInfo *p, MCB *mcb)
{
	word got = mcb->Control[0];
	
#ifdef SYSDEB
	SysDebug(pipe)("%x ReadAck %d %s",p,got,p->Write.ack?"WriteAck":"");
#endif
	InitMCB(mcb,0,mcb->MsgHdr.Reply,NullPort,Err_Null);
	PutMsg(mcb);

	p->Read.ack = NullPort;
	if( got > 0 ) p->ReadPos = p->Read.pos;
	
	if( p->Write.ack )
	{
		InitMCB(mcb,0,p->Write.ack,NullPort,WriteRc_Done);
		MarshalWord(mcb,got);
		PutMsg(mcb);
		p->Write.ack = NullPort;
		if( got > 0 ) p->WritePos = p->Write.pos;
	}
	
	if( got > 0 && p->Pending.size > 0 )
	{
		p->Pending.size -= got;
		if( p->Pending.size == 0 )
		{
			p->Pending.read = 0;
			p->Pending.got = 0;
			if( p->Pending.newmax != p->Pending.max )
			{
				byte *newbuf = (byte *)Malloc(p->Pending.newmax);
				if( newbuf )
				{
					Free(p->Pending.buf);
					p->Pending.buf = newbuf;
					p->Pending.max = p->Pending.newmax;
				}
				else p->Pending.newmax = p->Pending.max;
			}
		}
		else p->Pending.read += got;
		if( p->Write.req ) FreeMsgBuf(DoPipeWrite(p,p->Write.req));
		if( p->WriteSelect != NullPort )
		{
			SendException(p->WriteSelect,O_WriteOnly);
			p->WriteSelect = NullPort;
		}
	}

	return mcb;	
}

static MCB *DoPipeWrite( PipeInfo *p, MCB *mcb)
{
	ReadWrite *rw = (ReadWrite *)mcb->Control;
	word pos      = rw->Pos;
	word size     = rw->Size;
	word timeout  = rw->Timeout;
	word got      = p->Pending.got;
	word avail    = p->Pending.max - got;
	word idata    = mcb->MsgHdr.DataSize;
	word e;

	
#ifdef SYSDEB
	SysDebug(pipe)("%x PipeWrite %d pos %d avail %d",p,size,rw->Pos,avail);
#endif
	if( p->Write.req != NULL && p->Write.req != mcb )
	{
#if 1
		FreeMsgBuf(p->Write.req);
		p->Write.req = NULL;
#else
		InitMCB(mcb,0,mcb->MsgHdr.Reply,NullPort,
				EC_Error|SS_Pipe|EG_InUse|EO_Pipe);
		PutMsg(mcb);
		return mcb;
#endif	
	}

	/* Adaptive message sizes... */
	if( p->WritePos == pos )
	{
		p->MaxTfr /= 2;
		if( p->MaxTfr < TfrInc ) p->MaxTfr = TfrInc;

		/* This is a write retry, however, the reader	*/
		/* has got this data.				*/
		/* Hence we must junk this write.		*/
		/* Do this by sending the writer a WriteRc_Already */
		/* message with the appropriate size set.	*/
		/* Essentially this just repeats the ACK which	*/
		/* was lost.					*/
		InitMCB(mcb,0,mcb->MsgHdr.Reply,NullPort,WriteRc_Already);
		MarshalWord(mcb,size);
		PutMsg(mcb);
		return mcb;
	}
	else if( p->MaxTfr < MaxPipeTfr ) p->MaxTfr += TfrInc;
	
	/* if there is no reader, report a broken pipe	*/
	unless( p->State & O_ReadOnly )
	{
		InitMCB(mcb,0,mcb->MsgHdr.Reply,NullPort,EC_Error|SS_Pipe|EG_Broken|EO_Pipe);
		p->Write.req = NULL;
		return mcb;
	}

	if( p->ReadSelect != NullPort )
	{
		SendException(p->ReadSelect,O_ReadOnly);
		p->ReadSelect = NullPort;
	}
	
	p->Write.req = mcb;
	
	if( p->Read.req ) return ExchangeData(p);
	elif( idata )
	{
		/* Immediate data, accept as much as possible, and bounce */
		/* the request back.					*/
		if( avail > 0 )
		{
			if( idata > avail ) idata = avail;
			memcpy(p->Pending.buf+got,mcb->Data,(int)idata);
			p->Pending.got += idata;
			p->Pending.size += idata;
			p->WritePos = pos;
			p->Write.req = NULL;
			InitMCB(mcb,0,mcb->MsgHdr.Reply,NullPort,WriteRc_Done);
			MarshalWord(mcb,idata);
			PutMsg(mcb);
			return mcb;
		}
		/* else let it wait */
	}
	elif( avail > 0 )
	{
		MCB m;
		WriteReply rep;
		Port dataport = p->DataPort;
		word tfr = size;
		
		if( tfr > avail ) tfr = avail;
		
		if( tfr > p->MaxTfr ) tfr = p->MaxTfr;

		m.Control = (word *)&rep;

		p->Write.req = NULL;
		
		rep.first = tfr;
		rep.rest = tfr;
		rep.max = tfr;

		InitMCB(&m,MsgHdr_Flags_preserve,mcb->MsgHdr.Reply,dataport,WriteRc_Sizes);
		m.MsgHdr.ContSize = 3;
		e = PutMsg(&m);

		m.MsgHdr.Dest = dataport;
		m.Data = p->Pending.buf+got;
		if( e >= 0 ) e = GetMsg(&m);

		if( m.MsgHdr.DataSize != tfr ) e = EC_Error;
		else got += tfr;
		
		InitMCB(&m,0,mcb->MsgHdr.Reply,NullPort,WriteRc_Done);
		rep.first = e<0?e:tfr;

		m.MsgHdr.ContSize = 1;
		PutMsg(&m);	

		p->Pending.got = got;
		p->Pending.size += tfr;
		p->WritePos = pos;
				
		return mcb;
	}
	
	if( timeout == 0 )
	{
#if 1
		rw->Timeout = OneSec;
#else
		InitMCB(mcb,0,mcb->MsgHdr.Reply,NullPort,EC_Recover|SS_Pipe|EG_Timeout|EO_Pipe);
		PutMsg(mcb);
#endif
	}
	
	return NULL;
}

static MCB *ExchangeData(PipeInfo *p)
{
	MCB *	   rmcb = p->Read.req;
	MCB *	   wmcb = p->Write.req;
	MCB	   m;
	WriteReply r;
	int	   rsize  = (int)((ReadWrite *)(rmcb->Control))->Size;
	int	   tfr    = (int)((ReadWrite *)(wmcb->Control))->Size;
	Port	   reader = rmcb->MsgHdr.Reply;
	Port	   writer = wmcb->MsgHdr.Reply;
	word	   idata  = wmcb->MsgHdr.DataSize;
	
	p->Read.pos   = ((ReadWrite *)(rmcb->Control))->Pos;
	p->Write.pos  = ((ReadWrite *)(wmcb->Control))->Pos;

#ifdef SYSDEB
	SysDebug(pipe)("%x PipeXch %x[%d] -> %x[%d]",p,writer,tfr,reader,rsize);
#endif
	m.Control = (word *)&r;

	p->Read.req = p->Write.req = NULL;

	if( tfr > rsize ) tfr = rsize;
	
	if( idata )
	{
		/* send writer's immediate data to reader */
		InitMCB(&m,0,reader,NullPort,ReadRc_EOD);
		m.Data = wmcb->Data;
		m.MsgHdr.DataSize = tfr;
		PutMsg(&m);		
	}
	else
	{
		word bsize = tfr;
		if( bsize > p->MaxTfr ) bsize = p->MaxTfr;
	
		/* tell writer to send data directly to reader */
		InitMCB(&m,MsgHdr_Flags_preserve,writer,reader,WriteRc_Sizes);
		r.first = bsize;
		r.rest = r.first;
		r.max = tfr;
		m.MsgHdr.ContSize = 3;
		PutMsg(&m);

		/* the writer will now send its data to the reader	*/
	}
	
	p->Read.ack = rmcb->MsgHdr.Reply;
	p->Write.ack = wmcb->MsgHdr.Reply;
		
	FreeMsgBuf(rmcb);
	return wmcb;
}


static MCB *DoPipeSelect(PipeInfo *p, MCB *mcb)
{
	word mode = mcb->MsgHdr.FnRc & Flags_Mode;
	Port port = mcb->MsgHdr.Reply;
	word rep = 0;

#ifdef SYSDEB
	SysDebug(pipe)("%x PipeSelect",p);
#endif
	if( mode & O_ReadOnly )
	{
		FreePort(p->ReadSelect); p->ReadSelect = NullPort;
		if( p->WriteSelect || p->Pending.size > 0 || 
		    p->Write.req || (p->State & O_WriteOnly) == 0 ) 
			rep |= O_ReadOnly;
		else p->ReadSelect = port;
	}
	elif( mode & O_WriteOnly )
	{
		FreePort(p->WriteSelect); p->WriteSelect = NullPort;
		if( p->ReadSelect || p->Read.req ||
		    (p->State & O_ReadOnly) == 0 ) rep |= O_WriteOnly;
		else p->WriteSelect = port;	
	}
	
	if( rep ) 
	{
		InitMCB(mcb,0,port,NullPort,rep);
		PutMsg(mcb);
	}
	
	return mcb;
}

static void AbortRequest(struct P *p, word code)
{
	MCB *mcb = p->req;
	if( mcb == NULL ) return;
	InitMCB(mcb,0,mcb->MsgHdr.Reply,NullPort,code);
	PutMsg(mcb);
	FreeMsgBuf(p->req);
	p->req = NULL;
}

static MCB *DoPipeClose(PipeInfo *p, MCB *mcb)
{
	Port reply = mcb->MsgHdr.Reply;

#ifdef SYSDEB
	SysDebug(pipe)("%x PipeClose %x &= ~%x",p,p->State,(mcb->MsgHdr.FnRc&FF_Mask));
#endif

	p->State &= (int)(~(mcb->MsgHdr.FnRc & FF_Mask));

	if( !Terminating ) AbortRequest(&p->Read,ReadRc_EOF);

	AbortRequest(&p->Write,EC_Error|SS_Pipe|EG_Broken|EO_Pipe);
	if( !Terminating && p->ReadSelect )
	{
		InitMCB(mcb,0,p->ReadSelect,NullPort,O_ReadOnly);
		PutMsg(mcb);
		p->ReadSelect = NULL;
	}
	if( p->WriteSelect )
	{
		InitMCB(mcb,0,p->WriteSelect,NullPort,O_WriteOnly);
		PutMsg(mcb);
		p->WriteSelect = NULL;
	}
	if( reply != NullPort )
	{
		InitMCB(mcb,0,reply,NullPort,0);
		PutMsg(mcb);
	}

	return mcb;
}

static MCB *DoPipeAbort(PipeInfo *p, MCB *mcb)
{
	AbortRequest(&p->Read,EC_Error|SS_Pipe|EG_Exception|EE_Abort);
	AbortRequest(&p->Write,EC_Error|SS_Pipe|EG_Exception|EE_Abort);
	return mcb;
}

static void DoTimeout( struct P *p)
{
	if( p->req != NULL )
	{
		ReadWrite *rw = (ReadWrite *)p->req->Control;
		rw->Timeout-=OneSec*2;
		if( rw->Timeout <= 0 )
		{
			AbortRequest(p,EC_Recover|SS_Pipe|EG_Timeout|EO_Pipe);
		}
	}
}

static MCB  *DoPipeGSize(PipeInfo *p, MCB *mcb)
{
#ifdef SYSDEB
	SysDebug(pipe)("%x PipeGSize",p);
#endif
	InitMCB(mcb,0,mcb->MsgHdr.Reply,NullPort,Err_Null);
	MarshalWord(mcb,p->Pending.size);
	PutMsg(mcb);
	return mcb;
}

static MCB  *DoPipeSSize(PipeInfo *p, MCB *mcb)
{
	int newsize = (int)(mcb->Control[0]);
	word rc = Err_Null;
	
#ifdef SYSDEB
	SysDebug(pipe)("%x PipeSSize %d",p,newsize);
#endif
	if( p->Pending.size == 0 )
	{
		byte *newbuf = (byte *)Malloc(newsize);
		if( newbuf )
		{
			Free(p->Pending.buf);
			p->Pending.buf = newbuf;
			p->Pending.max = newsize;
		}
	}
	
	p->Pending.newmax = newsize;
	

	InitMCB(mcb,0,mcb->MsgHdr.Reply,NullPort,rc);
	PutMsg(mcb);	
	
	return mcb;
}

static void PipeServer(PipeInfo *p)
{
	MCB *mcb = NULL;
	Port serverport = p->ServerPort;
	PortInfo info;

	while( p->State )
	{
		word e;

		if( p->PendingWrite )
		{
			mcb = p->PendingWrite;
			p->PendingWrite = NULL;
			e = mcb->MsgHdr.FnRc;
			goto dispatch;
		}

		if( mcb == NULL ) mcb = NewMsgBuf(0);
		
		InitMCB(mcb,0,serverport,NullPort,0);
		mcb->Timeout = OneSec*2;

		e = GetMsg(mcb);

		if( e == EK_Timeout ) 
		{
			e = GetPortInfo(p->RemotePort,&info);
			if ( e != Err_Null )
			{
#ifdef SYSDEB
				SysDebug(error)("%x PipeServer GetPortInfo Error %E",p,e);
#endif
				AbortRequest(&p->Read,EC_Warn|SS_Pipe);
				AbortRequest(&p->Write,EC_Warn|SS_Pipe);
				continue;
			}

			DoTimeout(&p->Read);
			DoTimeout(&p->Write);
			continue;
		}
				
		if( e < Err_Null )
		{
#ifdef SYSDEB
			SysDebug(error)("%x PipeServer GetMsg Error %E",p,e);
#endif
#if 0
			if ( Terminating ) StopProcess();
#endif
			
			AbortRequest(&p->Read,EC_Warn|SS_Pipe);
			AbortRequest(&p->Write,EC_Warn|SS_Pipe);

			continue;
		}
		

	dispatch:
#if 0
#ifdef SYSDEB
		SysDebug(pipe)("%x PipeServer %F",p,e);
#endif
#endif
		
		switch( e & FG_Mask )
		{
		case FG_Read:	 mcb = DoPipeRead(p,mcb);	break;
		case FG_ReadAck: mcb = DoReadAck(p,mcb);	break;
		case FG_Write:	 mcb = DoPipeWrite(p,mcb);	break;
		case FG_Select:	 mcb = DoPipeSelect(p,mcb);	break;
		case FG_GetSize: mcb = DoPipeGSize(p,mcb);	break;
		case FG_SetSize: mcb = DoPipeSSize(p,mcb);	break;
		case FG_Abort:	 mcb = DoPipeAbort(p,mcb);	break;
		case FG_Close:	 mcb = DoPipeClose(p,mcb);	break;
		case FG_NewPort:
				FreePort(serverport);
				serverport = p->ServerPort;
				break;
		default:
			InitMCB(mcb,0,mcb->MsgHdr.Reply,NullPort,
				EC_Error|SS_Pipe|EG_WrongFn|EO_Pipe);
			PutMsg(mcb);
		}
	}

#ifdef SYSDEB
	SysDebug(pipe)("%x PipeServer quitting",p);
#endif

	if( Terminating ) return;
	Wait(&PipeLock);
	Remove(&p->Node);
	FreeMsgBuf(mcb);
	FreePort(p->ServerPort);
	FreePort(p->DataPort);
	Free(p->Pending.buf);
	Free(p);
	Signal(&PipeLock);

	return;	
}

#ifdef SYSDEB

#pragma -s1

void _stack_error(Proc *p)
{
	IOdebug("Pipe stack overflow in %s at %x",p->Name,&p);	
}
#endif 

static Port InitProtocol(PipeInfo *p)
{
	Stream *s = p->Stream1;
	word e;
	MCB *mcb = NewMsgBuf(0);
	Port port;

again:
	/* send off a connect message to pipe server	*/
	InitMCB(mcb,MsgHdr_Flags_preserve,s->Server,s->Reply,0);

	e = PutMsg(mcb);

again1:
	InitMCB(mcb,0,s->Reply,NullPort,0);
	if( e >= 0 ) e = GetMsg(mcb);

	switch( e )
	{
	case Code_Connect:
	case Code_Connect1:
		break;		/* simple case drop straight out    */

	case Code_Timeout:	/* other side not yet ready, retry  */
		goto again;
		
	case FG_Write:		/* The other side's write got here	*/
				/* before the server's reply, squirrel	*/
				/* it away for later consumption.	*/
		p->PendingWrite = mcb;
		mcb = NewMsgBuf(0);
		e = 0;
		goto again1;
		
	case Code_Close:	/* other end has simply closed	    */
	default:		/* an error of some sort	    */
#ifdef SYSDEB
		SysDebug(error)("Pipe Init error: %x (called from %s)",e,procname(returnlink_(s)));
#if 0
		IOdebug("Pipe Init error: %x (called from %s)",e,procname(returnlink_(s)));
		back_trace();
#endif
#endif
		if( (e&EG_Mask) == EG_Timeout ) goto again;
		if (e == Code_Close)
		 s->Result2 = ReadRc_EOF;
		else
		 s->Result2 = e;
		FreeMsgBuf(mcb);
		return NullPort;
	}
	port = mcb->MsgHdr.Reply;
	FreeMsgBuf(mcb);
	return port;
}

static PipeInfo *InitPipe(Stream *s)
{
	PipeInfo *p;
	Port server;
	PipeInfo **process = NULL;
	
	p = New(PipeInfo);
	
	if( p == NULL ) 
	{
		CloseStream(s); s->Type = Type_Pseudo;
		s->Result2 = EC_Error|SS_SysLib|EG_NoMemory|EO_Pipe;
		return NULL;
	}

	if( s->Flags & O_ReadOnly ) 
	{
		process = (PipeInfo **)NewProcess(pipestack,PipeServer,sizeof(p));
	
		if( process == NULL )
		{
			Free(p);
			CloseStream(s);	s->Type = Type_Pseudo;
			s->Result2 = EC_Error|SS_SysLib|EG_NoMemory|EO_Pipe;
			return NULL;
		}
	}

	memset(p,0,sizeof(PipeInfo));

	p->Stream1 = s;
	
	server = InitProtocol(p);

	if( server == NullPort )
	{
		Free(p);
		if (process != NULL)
		  ZapProcess((void *)process);
		CloseStream(s); s->Type = Type_Pseudo;
		return NULL;
	}

	/* The pipe is connected */

	Wait(&PipeLock);
	AddTail(&PipeList,&p->Node);
	Signal(&PipeLock);

#ifdef SYSDEB
	SysDebug(pipe)("%x InitPipe s %x",p,s);
#endif
	
	p->Stream2        = NULL;
	s->Server	  = server;
	s->Flags	 |= Flags_Extended|Flags_Fast;
	s->Timeout	  = IOCTimeout;
	p->Pending.max	  = 2048;
	p->Pending.newmax = 2048;
	p->DataPort       = NewPort();
	p->RemotePort     = server;
	p->MaxTfr         = InitPipeTfr;
	p->ReadPos        = 1;
	p->WritePos       = 1;
				
	p->State = (int)(O_WriteOnly | (s->Flags & O_ReadWrite));

	if( p->State & O_ReadOnly )
	{
		p->ServerPort = s->Reply;
		s->Reply = NewPort();

		p->Pending.buf = (byte *)Malloc(p->Pending.max);
		if( p->Pending.buf == NULL )
		{
			p->Pending.max = 0;
			p->Pending.newmax = 0;
		}

		p->Pending.read = 0;
		p->Pending.got = 0;
		p->Pending.size = 0;
	
		*process = p;
			
		ExecProcess( (void *)process, HighServerPri );
	}
	else p->ServerPort = NULL;
	
	return p;
}

static word GetPipe(PipeInfo *p, Stream *s) 
{ return (p->Stream1 == s) || (p->Stream2 == s); }

PRIVATE word FindPorts(Stream *s, word fn, Port *server, Port *reply)
{
	PipeInfo *p;

	if( s->Type != Type_Pipe ) *server = s->Server;
	else
	{
		Wait( &PipeLock );
		p = (PipeInfo *)SearchList(&PipeList,GetPipe,s);
		Signal( &PipeLock );

		if( p == NULL ) 
		{
			p = InitPipe(s);
			if( p == NULL )
			{
				if( fn == FG_Write && s->Result2 == ReadRc_EOF )
				{
					s->Result2 = EC_Error|SS_Pipe|EG_Broken|EO_Pipe;
				}
				return s->Result2;
			}
		}
		
		switch( fn )
		{
		case FG_Read:
		case FG_GetSize:
		case FG_SetSize:
		case FG_Select|O_ReadOnly:
			unless( p->State & O_ReadOnly ) return EC_Error|SS_Pipe|EG_Broken|EO_Pipe;
			*server = p->ServerPort;
			break;
		case FG_Select|O_WriteOnly:
		default:
			*server = s->Server;
			break;
		}
	}

	if( (GetReady(s->Reply)&EG_Mask) == EG_Invalid ) s->Reply = NewPort();
	
	*reply = s->Reply;

	return 0;
}

PRIVATE word PipeReOpen(Stream *s, word code, word rc)
{
	Stream *s1 = s, *s2;
	PipeInfo *p;
	Port port;
	word e;

	Wait( &PipeLock );
	p = (PipeInfo *)SearchList(&PipeList,GetPipe,s);
	Signal( &PipeLock );

	if( p == NULL ) return EC_Error|SS_Pipe|EG_Broken|EO_Pipe;
#ifdef SYSDEB
	SysDebug(error)("PipeReOpen: %S",s);
#endif
	s2 = p->Stream2;

	if( s1 == p->Stream1 ) { if( s2 ) Wait(&s2->Mutex); }
	elif( s1 == s2 )
	{
		s1 = p->Stream1;
		Wait(&s1->Mutex);
	}

	/* ReOpen will re-allocate these		*/
	FreePort(s1->Server);
	FreePort(s1->Reply);
	s1->Server = s1->Reply = NullPort;

	e = ReOpen(s1);
#ifdef SYSDEB
	if( e < 0 ) SysDebug(error)("PipeReOpen: ReOpen error %x %S",e,s);
#endif
	if( e < Err_Null ) goto done;

	port = InitProtocol(p);
#ifdef SYSDEB
	if( port == 0 ) SysDebug(error)("PipeReOpen: Init error %x %S",s1->Result2,s);
#endif
	if( port == NullPort ) { e = s1->Result2; goto done; }

	p->RemotePort = port;
	s1->Flags |= Flags_Extended|Flags_Fast;	
	s1->Server = port;
	
	if( s2 )
	{
		s2->Server = port;
		FreePort(s2->Reply);
		s2->Reply = NewPort();
		s2->Flags |= Flags_Extended|Flags_Fast;
	}
	
	if( p->State & O_ReadOnly )
	{
		Port oldsp = p->ServerPort;
		p->ServerPort = s1->Reply;
		SendException(oldsp,FG_NewPort);
		s1->Reply = NewPort();
	}
	
done:
#ifdef SYSDEB
	SysDebug(error)("PipeReOpen: done %x %S",e,s);
#endif
	if( s == s2 ) Signal(&s1->Mutex);
	else if( s2 ) Signal(&s2->Mutex);
	
	return e;
}

PRIVATE word PipeClose(Stream *s)
{
	PipeInfo *p;
	bool freepipe = false;

	
	Wait( &PipeLock );
	p = (PipeInfo *)SearchList(&PipeList,GetPipe,s);
	Signal( &PipeLock );

	if( p != NULL )
	{
		if( p->State & s->Flags & O_ReadOnly ) 
		{
			MCB mcb;
			Port port = NewPort();

			InitMCB(&mcb,0,p->ServerPort,port,FG_Close|O_ReadOnly);
			XchMsg(&mcb,NULL);
			FreePort(port);
		}
		else freepipe = !Terminating;
/*		if( p->State & s->Flags & O_WriteOnly ) */

#if 1
		/* Do close syncronously so that rest of close code doesn't */
		/* free the port, before the receiver gets the pipe close.  */
		{
			MCB mcb;

			InitMCB(&mcb,0,s->Server,NullPort,FG_Close|O_WriteOnly);
			mcb.Timeout = 10 * OneSec;

			PutMsg(&mcb);
		}	
#else
		SendException(s->Server,FG_Close|O_WriteOnly);
#endif
		if( p->Stream1 == s ) p->Stream1 = NULL;
		elif( p->Stream2 == s ) p->Stream2 = NULL;
		if( freepipe && p->Stream1 == NULL && p->Stream2 == NULL )
		{
			Wait(&PipeLock);
			Remove(&p->Node);
			FreePort(p->DataPort);
			Free(p->Pending.buf);
			Free(p);		
			Signal(&PipeLock);
		}

		FreePort(s->Server);

		return true;
	}
	else
	{
		/* A pipe which has been opened but not connected,	*/
		/* do a FreePort on the server port which will get rid	*/
		/* of the dangling trail.				*/
		FreePort(s->Server);
	}
	
	return false;
}

PRIVATE void PipeAbort(Stream *s)
{
	PipeInfo *p;
	
	Wait( &PipeLock );
	p = (PipeInfo *)SearchList(&PipeList,GetPipe,s);
	Signal( &PipeLock );

	if( p != NULL )
	{
		MCB mcb;
		InitMCB(&mcb,MsgHdr_Flags_preserve,s->Server,NullPort,FG_Abort|O_WriteOnly);
		PutMsg(&mcb);		
		if( p->ServerPort )
		{
			mcb.MsgHdr.Dest = p->ServerPort;
			mcb.MsgHdr.FnRc = FG_Abort|O_ReadOnly;
			PutMsg(&mcb);
		}
		
	}
}

PRIVATE bool CopyPipe(Stream *old, Stream *New)
{
	PipeInfo *p;
	
	Wait( &PipeLock );
	p = (PipeInfo *)SearchList(&PipeList,GetPipe,old);
	Signal( &PipeLock );

	if( p == NULL )
	{
		p = InitPipe(old);
		if( p == NULL ) return false;
	}
	
#ifdef SYSDEB
	SysDebug(pipe)("%x CopyPipe %x -> %x",p,old,New);
#endif

	if( p->Stream1 == old && p->Stream2 == NULL )
	{
		p->Stream2 = New;
		return true;
	}
	return false;
}

PRIVATE Port PipeSelect(Stream *s, word mode)
{
	Port server;
	Port reply = NullPort;
	MCB mcb;

	if( mode & O_WriteOnly )
	{
		if(FindPorts(s,FG_Select|O_WriteOnly,&server,&reply) == Err_Null)
		{
			InitMCB(&mcb,MsgHdr_Flags_preserve,server,reply,FG_Select|O_WriteOnly);
			PutMsg(&mcb);	
		}
	}
	if( mode & O_ReadOnly )
	{
		if(FindPorts(s,FG_Select|O_ReadOnly,&server,&reply) == Err_Null)
		{
			InitMCB(&mcb,MsgHdr_Flags_preserve,server,reply,FG_Select|O_ReadOnly);
			PutMsg(&mcb);	
		}
	}
	return reply;
}

#ifdef __TRAN
PUBLIC word GrabPipe(Stream *s, Port *ports)
{
	return EC_Error|SS_Pipe|EG_Invalid|EO_Pipe;
}

PUBLIC word UnGrabPipe(Stream *s)
{
	return EC_Error|SS_Pipe|EG_Invalid|EO_Pipe;
}
#endif /* __TRAN */

/* end of pipe.c */
