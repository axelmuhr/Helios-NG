/*------------------------------------------------------------------------
--                                                                      --
--                     H E L I O S   N U C L E U S                      --
--                     ---------------------------                      --
--                                                                      --
--             Copyright (C) 1987, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- fifo.c								--
--                                                                      --
--	FIFO file handler, used to support pipes and stream sockets	--
--                                                                      --
--	Author:  NHG 18/11/87						--
--                                                                      --
------------------------------------------------------------------------*/
/* SccsId:	 %W%	%G% Copyright (C) 1987, Perihelion Software Ltd.*/
/* $Id: fifo.c,v 1.4 1992/10/05 09:04:36 nickc Exp $ */


#include <helios.h>	/* standard header */

#define __in_fifo 1	/* flag that we are in this module */

/*--------------------------------------------------------
-- 		     Include Files			--
--------------------------------------------------------*/

#include <string.h>
#include <codes.h>
#include <config.h>
#include <module.h>
#include <syslib.h>
#include <stdarg.h>
#include <nonansi.h>
#include <servlib.h>

/*--------------------------------------------------------
--		Private Data Definitions 		--
--------------------------------------------------------*/

typedef struct Fifo {
	ObjNode		ObjNode;	/* node for directory struct	*/
	word		ReadPos;	/* position of last read	*/
	word		WritePos;	/* position of last write	*/
	word		CreadPos;	/* current read position	*/
	word		Readers;	/* state of reader		*/
	word		Writers;	/* state of writer		*/
	word		Read;		/* true if Fifo has been read.	*/
	word		Written;	/* true if fifo has been written*/
	Port		RDelayPort;	/* port for Read to wait on	*/
	Port		WDelayPort;	/* port for Write to wait on	*/
	Port		SelectPort;	/* port for select		*/
} Fifo;
#define Users		ObjNode.Account	/* number of open streams	*/
#define Upb 		ObjNode.Size	/* use ObjNode size field	*/
#define Buffers		ObjNode.Contents /* use ObjNode Contents field	*/

#define data_available(f) ((f)->Upb - (f)->CreadPos)

#define FifoMax		1024		/* byte in each block		*/

DirNode		Root;			/* root of fifo directory	*/

static Fifo *CreateFifo(DirNode *d, string pathname, word mode);
void FifoRead(MCB *m, Fifo *f);
void FifoWrite(MCB *m, Fifo *f);
void FifoClose(Fifo *f, word isreader);

static void do_open(ServInfo *);
static void do_create(ServInfo *);
static void do_serverinfo(ServInfo *);
static void do_delete(ServInfo *);

static DispatchInfo FifoInfo = {
	&Root,
	NullPort,
	SS_Fifo,
	NULL,
	{ NULL, 0 },
	{
		{ do_open,	  2000 },
		{ do_create,	  2000 },
		{ DoLocate,	  2000 },
		{ DoObjInfo,	  2000 },
		{ do_serverinfo,  2000 },
		{ do_delete,	  2000 },
		{ DoRename,	  2000 },
		{ InvalidFn,	  2000 },
		{ DoProtect,	  2000 },
		{ NullFn,	  2000 },
		{ DoRefine,	  2000 },
		{ NullFn,	  2000 },
		{ DoRevoke,	  2000 },
		{ InvalidFn,	  2000 },
		{ InvalidFn,	  2000 }
	}
};

/*--------------------------------------------------------
-- main							--
--							--
-- Entry point of fifo handler.				--
--							--
--------------------------------------------------------*/

int main()
{

	char mcname[100];

	MachineName(mcname);
	FifoInfo.ParentName = mcname;

	InitNode( (ObjNode *)&Root, "fifo", Type_Directory, 0, DefRootMatrix );
	InitList( &(Root.Entries) );
	Root.Nentries = 0;

	FifoInfo.ReqPort = MyTask->Port;

	/* .. of root is a link to our machine root	*/
	{
		Object *o;
		LinkNode *Parent;

		o = Locate(NULL,mcname);

		Parent = (LinkNode *)Malloc( sizeof (LinkNode) + (word)strlen( mcname ) );	
		InitNode( &Parent->ObjNode, "..", Type_Link, 0, DefLinkMatrix );
		Parent->Cap = o->Access;
		strcpy(Parent->Link,mcname);
		Root.Parent = (DirNode *)Parent;

		Close(o);
	}

	Dispatch(&FifoInfo);

}

/*--------------------------------------------------------
-- NewFifo						--
--							--
-- Add an image entry to the directory			--
--							--
--------------------------------------------------------*/

Fifo *NewFifo(DirNode *dir,string name, word flags, Matrix matrix)
{
	Fifo *f = New(Fifo);

	if( f == Null(Fifo) ) return Null(Fifo);

	memset(f,0,sizeof(Fifo));

	InitNode( &f->ObjNode, name, Type_Fifo, (int)flags, matrix );
	
	InitList(&f->Buffers);
	f->RDelayPort = NewPort();
	f->WDelayPort = NewPort();	
	/* all other field are initially zero */
	
	Insert( dir, &f->ObjNode, TRUE );

	return f;
}

/*--------------------------------------------------------
-- Action Procedures					--
--							--
--------------------------------------------------------*/

static void do_open(ServInfo *servinfo)
{
	MCB *m = servinfo->m;
	MsgBuf *r;
	DirNode *d;
	Fifo *f;
	IOCMsg2 *req = (IOCMsg2 *)(m->Control);
	Port reqport;
	word mode = req->Arg.Mode;
	byte *data = m->Data;
	char *pathname = servinfo->Pathname;

	d = (DirNode *)GetTargetDir( servinfo);

	if( d == Null(DirNode) )
	{
		ErrorMsg(m,Err_Null);
		return;
	}

	r = New(MsgBuf);

	if( r == Null(MsgBuf) )
	{
		ErrorMsg(m,EC_Error+EG_NoMemory);
		return;
	}

	f = (Fifo *)GetTargetObj(servinfo);

	if( f == Null(Fifo)  && (mode & O_Create)  )
		f = CreateFifo(d, pathname, mode);


	if( f == Null(Fifo) )
	{
		ErrorMsg(m,Err_Null);
		Free(r);
		return;
	}

	unless( CheckMask( req->Common.Access.Access, (int)mode & Flags_Mode ) ) 
	{
		ErrorMsg(m,EC_Error+EG_Protected+EO_Fifo);
		Free(r);
		return;
	}

	FormOpenReply(r,m,&f->ObjNode,Flags_Closeable, pathname);

	reqport = NewPort();
	r->mcb.MsgHdr.Reply = reqport;
	PutMsg(&r->mcb);
	Free(r);

	if( f->ObjNode.Type == Type_Directory )
	{
		DirServer(servinfo,m,reqport);
		FreePort(reqport);
		return;
	}

	f->Users++;

	if( (mode&O_ReadOnly) ) f->Readers++;
	if( (mode&O_WriteOnly) ) f->Writers++; 

	UnLockTarget(servinfo);
	
	forever
	{
		word e;
		m->MsgHdr.Dest = reqport;
		m->Timeout = StreamTimeout;
		m->Data = data;

		e = GetMsg(m);

		if( e == EK_Timeout ) break;

		if( e < Err_Null ) continue;

		Wait(&f->ObjNode.Lock);

		m->MsgHdr.FnRc = SS_Fifo;

		switch( e & FG_Mask )
		{
		case FG_Read:
			f->Read = true;
			FifoRead(m,f);
			if( f->SelectPort != NullPort )
			{
				FreePort(f->SelectPort);
				f->SelectPort = NullPort;
			}
			break;

		case FG_Write:
			f->Written = true;
			FifoWrite(m,f);
			if( f->SelectPort != NullPort && data_available(f) > 0 )
			{
				SendException(f->SelectPort,O_ReadOnly);
				f->SelectPort = NullPort;
			}
			break;
		
		case FG_Close:
			if( m->MsgHdr.Reply != NullPort ) ErrorMsg(m,Err_Null);
			goto done;
			return;

		case FG_GetSize:
			InitMCB(m,0,m->MsgHdr.Reply,NullPort,Err_Null);
			MarshalWord(m,data_available(f));
			PutMsg(m);
			break;

		case FG_Seek:
		case FG_SetSize:
			ErrorMsg(m,EC_Error+EG_WrongFn+EO_Fifo);
			break;
			
		case FG_Select:
		{
			int reply = 0;
			if( e & O_ReadOnly ) 
			{
				if( data_available(f) > 0 ) reply |= O_ReadOnly;
			}
			/* write select succeeds immediately */
			if( e & O_WriteOnly ) reply |= O_WriteOnly;
			if( reply ) ErrorMsg(m,reply);
			else 
			{
				/* we only set up here for read selects	*/
				FreePort(f->SelectPort);
				f->SelectPort = m->MsgHdr.Reply;
			}
			break;
		}
		
		default:
			ErrorMsg(m,EC_Error+EG_FnCode+EO_Fifo);
			break;
		}
		Signal(&f->ObjNode.Lock);
	}

done:	
	FifoClose(f,mode);
	FreePort(reqport);
}

static void do_create(ServInfo *servinfo)
{
	MCB *m = servinfo->m;
	MsgBuf *r;
	DirNode *d;
	Fifo *f;
	IOCCreate *req = (IOCCreate *)(m->Control);
	char *pathname = servinfo->Pathname;

	d = GetTargetDir(servinfo);

	if( d == Null(DirNode) )
	{
		ErrorMsg(m,EO_Directory);
		return;
	}

	unless( CheckMask(req->Common.Access.Access,AccMask_W) ) 
	{
		ErrorMsg(m,EC_Error+EG_Protected+EO_Directory);
		return;
	}

	r = New(MsgBuf);

	if( r == Null(MsgBuf) )
	{
		ErrorMsg(m,EC_Error+EG_NoMemory);
		return;
	}
	
	f = (Fifo *)GetTargetObj(servinfo);

	if( f != Null(Fifo) )
	{
		ErrorMsg(m,EC_Error+EG_Create+EO_Fifo);
		Free(r);
		return;
	}

	f = CreateFifo(d, pathname, 0);
	
	if( f == Null(Fifo) )
	{
		ErrorMsg(m,0);
		Free(r);
		return;
	}

	/* give creator full access */
	req->Common.Access.Access = AccMask_Full;
		
	FormOpenReply(r,m,&f->ObjNode, 0, pathname);

	PutMsg(&r->mcb);

	Free(r);
}

static Fifo *CreateFifo(DirNode *d, string pathname, word mode)
{
	Fifo *f;
	word flags = 0;
	char *name = objname(pathname);
	
	f = NewFifo(d, name, flags|Flags_Selectable, DefFileMatrix );

	return f;
	
	mode=mode; /* ignore */
}

static void do_delete(ServInfo *servinfo)
{
	MCB *m = servinfo->m;
	Fifo *f;
	IOCCommon *req = (IOCCommon *)(m->Control);

	f = (Fifo *)GetTarget(servinfo);

	if( f == Null(Fifo) )
	{
		ErrorMsg(m,EO_Fifo);
		return;
	}
	
	if( f->ObjNode.Type != Type_Fifo )
	{
		ErrorMsg(m,EC_Error|EG_WrongFn|EO_Directory);
		return;
	}
	
	unless( CheckMask(req->Access.Access,AccMask_D) ) 
	{
		ErrorMsg(m,EC_Error+EG_Protected+EO_Fifo);
		return;
	}


	/* only allow a delete if there are no opens on it */
	if( f->Users == 0 )
	{
		f->ReadPos = f->WritePos = f->Upb + FifoMax;
		AdjustBuffers(&f->Buffers,0,0,FifoMax);
		Unlink(&f->ObjNode, FALSE);
		FreePort(f->RDelayPort);
		FreePort(f->WDelayPort);		
		Free(f);
		ErrorMsg(m,Err_Null);
	}
	else ErrorMsg(m,EC_Error|EG_InUse|EO_Fifo);
}

static void do_serverinfo(ServInfo *servinfo)
{
/*	char *pathname = servinfo->Pathname;*/
	NullFn(servinfo);
}

/*--------------------------------------------------------
-- FifoRead						--
-- 							--
-- Read data from the fifo and return it to the client.	--
-- To preserve the retryability of the protocols we	--
-- must not dispose of any data that has been returned  --
-- until the next read comes back since the client may	--
-- need to retry the read.				--
--							--
--------------------------------------------------------*/


void FifoRead(MCB *m, Fifo *f)
{
	ReadWrite *rw = (ReadWrite *)m->Control;
	word pos = rw->Pos;
	word rwsize = rw->Size;
	word timeout = rw->Timeout;
	word e = Err_Null;

	if( pos < f->ReadPos )
	{
		ErrorMsg(m,EC_Error|EG_Parameter|1);
		return;
	}

	f->ReadPos = pos;	/* position about to read from 	*/
	f->CreadPos = pos;	/* current read position	*/
	
	/* re-arrange the buffers in the fifo */
	if( !AdjustBuffers(&f->Buffers,f->ReadPos<f->WritePos ? f->ReadPos : f->WritePos,f->Upb,FifoMax) )
	{
		ErrorMsg(m,EC_Error|EG_NoMemory);
		return;
	}
	
Again:

	/* if there is no more data, and the writer is closed, report	*/
	/* end of Fifo.							*/

	if( (f->ReadPos == f->Upb) && f->Written && (f->Writers == 0) )
	{
		m->MsgHdr.FnRc = ReadRc_EOF;
		ErrorMsg(m,0);
		return;
	}

	if( pos + rwsize > f->Upb ) rw->Size = f->Upb - pos;
	else rw->Size = rwsize;

	/* If there is no data to send, wait for the timeout period.	*/
	/* If some data does arrive before the timeout, the port will be*/
	/* aborted and we will get an exception result here.		*/
	/* Note that while we delay the lock on the fifo must be 	*/
	/* released.							*/
	if( rw->Size == 0 )
	{
		MCB m1;
      
     		if( timeout != 0 )
     		{
			InitMCB(&m1,0,f->RDelayPort,NullPort,0);
			m1.Timeout = timeout;
			Signal(&f->ObjNode.Lock);
			e = GetMsg(&m1);
			Wait(&f->ObjNode.Lock);
		} else e = 0;
		if( (e&EG_Mask) != EG_Exception )
		{
			m->MsgHdr.FnRc = EC_Recover+SS_Fifo+EG_Timeout+EO_Fifo;
			ErrorMsg(m,0);
			return;
		}
		else goto Again;
	}

	DoRead(m,GetReadBuffer,&f->Buffers);

	f->ObjNode.Dates.Access = GetDate();
	
	f->CreadPos += rw->Size;	/* increment current read */
					/* pos by amount read	  */
}


/*--------------------------------------------------------
-- FifoWrite						--
-- 							--
-- Write data to the Fifo.				--
-- To preserve retryability we should allow the client	--
-- to over-write existing data.				--
--							--
--------------------------------------------------------*/

void FifoWrite(MCB *m, Fifo *f)
{
	ReadWrite *rw = (ReadWrite *)m->Control;
	word pos = rw->Pos;

/*dbg("FifoWrite f %x pos %d size %d",f,pos,size);*/

	if( pos < f->WritePos )
	{
		ErrorMsg(m,EC_Error|EG_Parameter|1);
		return;
	}
	
	f->WritePos = pos;
	f->Upb = pos+rw->Size;

	/* re-arrange the buffers in the fifo */
	if( !AdjustBuffers(&f->Buffers,f->ReadPos<f->WritePos ? f->ReadPos : f->WritePos,f->Upb,FifoMax) )
	{
		ErrorMsg(m,EC_Error|EG_NoMemory|EO_Fifo);
		return;
	}

	/* we now know the fifo has enough buffers to contain our write */

	DoWrite(m,GetWriteBuffer,&f->Buffers);
		
	f->ObjNode.Dates.Modified = GetDate();

	AbortPort(f->RDelayPort,EC_Error|EG_Exception|EE_Abort);
}

/*--------------------------------------------------------
-- FifoClose						--
-- 							--
-- Close one side of a Fifo.				--
--							--
--------------------------------------------------------*/

void FifoClose(Fifo *f, word mode)
{
	if( mode & O_ReadOnly )	f->Readers -= f->Readers==0?0:1;
	if( mode & O_WriteOnly ) f->Writers -= f->Writers==0?0:1;

	f->Users--;

	if( (f->Read    && f->Readers == 0) &&
	    (f->Written && f->Writers == 0) &&
	     f->Users == 0 )
	{
		f->ReadPos = f->WritePos = f->Upb + FifoMax;
		AdjustBuffers(&f->Buffers,0,0,FifoMax);
		Unlink(&f->ObjNode, FALSE);
		FreePort(f->RDelayPort);
		FreePort(f->WDelayPort);		
		Free(f);
		return;
	}
	else Signal(&f->ObjNode.Lock);

	/* abort the delay port so both Readers & Writers will have a   */
	/* chance to respond to this event.				*/

	AbortPort(f->RDelayPort,EC_Error|EG_Exception|0xaaaa);
	AbortPort(f->WDelayPort,EC_Error|EG_Exception|0xaaaa);
}



/* -- End of fifo.c */

