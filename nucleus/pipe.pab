/*------------------------------------------------------------------------
--                                                                      --
--                     	H E L I O S   S E R V E R                       --
--                      -------------------------                       --
--                                                                      --
--             Copyright (C) 1987, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- pipe.c								--
--                                                                      --
--	Point-to-point pipe handler.					--
--	This merely acts as a rendezvous point for the communicators.	--
--                                                                      --
--	Author:  NHG 16/8/87						--
--                                                                      --
------------------------------------------------------------------------*/
/* SccsId:	 %W%	%G% Copyright (C) 1987, Perihelion Software Ltd.*/
/* $Id: pipe.c,v 1.12 1993/07/27 08:34:12 nickc Exp $ */

#include <helios.h>	/* standard header */

#define __in_pipe 1	/* flag that we are in this module */

#define SA 0		/* set to 1 for Stand Alone	*/


#include <syslib.h>
#include <servlib.h>
#include <string.h>
#include <root.h>

#define Code_Connect	0xa1
#define Code_Timeout	0xa2
#define Code_Close	0xa3
#define Code_Connect1	0xa4


#define State_Mask	0xFFFFFFF0L

typedef struct Pipe {
	ObjNode		ObjNode;
	Port		Connect;
	bool		Finish;
} Pipe;
#define Users	ObjNode.Size
#define State	ObjNode.Account

DirNode		Root;

word		pipeno = 1;

NameInfo PipeInfo =
{
	NullPort,
	Flags_StripName,
	DefNameMatrix,
	(word *)NULL
};

static void do_open(ServInfo *servinfo);
static void do_create(ServInfo *servinfo);
static void do_delete(ServInfo *servinfo);
static void do_closeobj(ServInfo *servinfo);

static DispatchInfo PipeDInfo = {
	(DirNode *)&Root,
	NullPort,
	SS_Pipe,
	NULL,
	{ NULL,	0 },
	{
		{ do_open,	2000 },
		{ do_create,	2000 },
		{ DoLocate,	2000 },
		{ DoObjInfo,	2000 },
		{ InvalidFn,	2000 },
		{ do_delete,	2000 },
		{ InvalidFn,	2000 },
		{ InvalidFn,	2000 },
		{ InvalidFn,	2000 },
		{ InvalidFn,	2000 },
		{ InvalidFn,	2000 },
		{ do_closeobj,	2000 },
		{ DoRevoke,	2000 },
		{ InvalidFn,	2000 },
		{ InvalidFn,	2000 }
	}
};

extern word Fork(int stacksize, VoidFnPtr fn, int args_size, ...);

Pipe *NewPipe(DirNode *d,char *name);
void PipeConnect(Pipe *p);

int main(void)
{

	Object *nte = Null(Object);
	bool	in_nucleus;

#if SA	
	Environ env;
	GetEnv(MyTask->Port,&env);
#endif

#ifdef __TRAN
	in_nucleus = (((word) &main) < (word) GetRoot());
#else
# ifdef __C40
	  {
	    MPtr base = GetSysBase();	/* returns word address on C40 */

	    in_nucleus = (
			  (base < (MPtr)main) &&
			  ((MPtr)main < MRTOA_(base) )
			  );
	  }
# else
	  {
	    word *	base = (word *)GetSysBase();
	    
	    in_nucleus = (
			  (base < (word *)main) &&
			  ((word *)main < base + *base)
			  );
	  }
# endif
#endif
	
	InitNode( (ObjNode *)&Root, "pipe", Type_Directory, 0, DefRootMatrix );
	InitList( &(Root.Entries) );
	Root.Nentries = 0;

	PipeInfo.Port = 
	PipeDInfo.ReqPort = MyTask->Port;

	/* .. of root is a link to our machine root	*/
	{
		Object *o;
		LinkNode *Parent;
		char mcname[100];

		
		MachineName(mcname);
		
		o = Locate(NULL,mcname);

		Parent = (LinkNode *)Malloc(sizeof(LinkNode) + (word)strlen(mcname));
		
		InitNode( &Parent->ObjNode, "..", Type_Link, 0, DefLinkMatrix );
		
		Parent->Cap = o->Access;
		
		strcpy(Parent->Link,mcname);
		
		Root.Parent = (DirNode *)Parent;

#if !(SA)
		if (in_nucleus)
#endif
  		  nte = Create(o,"pipe",Type_Name,sizeof(NameInfo),
			(byte *)&PipeInfo);
		
		Close(o);
	}

	if (in_nucleus)
	 { MCB	m;
	   InitMCB(&m, 0, MyTask->Parent, NullPort, 0x456);

	   (void) PutMsg(&m);
	 }

	Dispatch(&PipeDInfo);

	if (nte != NULL)
 	 Delete(nte, NULL);

	return 0;
}


static void do_create(ServInfo *servinfo)
{
	DirNode *d;
	Pipe *p;
	MsgBuf *r;
	MCB *m = servinfo->m;
	IOCCreate *req = (IOCCreate *)(m->Control);	
	char *pathname = servinfo->Pathname;

	
	d = GetTargetDir(servinfo);
	
	if( d == NULL )
	{
		ErrorMsg(m,EO_Directory);
		return;
	}
	
	p = (Pipe *)GetTargetObj(servinfo);
	
	
	if( p != NULL )
	{
		ErrorMsg(m,EC_Error+EG_Create+EO_Pipe);
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
		ErrorMsg(m,EC_Error|EG_NoMemory|EO_Pipe);
		return;
	}

	p = NewPipe(d,objname(pathname));
	
	/* give creator full access rights */
	
	req->Common.Access.Access = AccMask_Full;
	
	FormOpenReply(r,m,&p->ObjNode,0,pathname);
	
	PutMsg(&r->mcb);
	
	Free(r);

	return;	
}

static void do_open(ServInfo *servinfo)
{
	MCB *m = servinfo->m;
	MsgBuf *r;
	IOCMsg2 *req = (IOCMsg2 *)(m->Control);
	char *pathname = servinfo->Pathname;
	DirNode *d;
	Pipe *p;

	r = New(MsgBuf);

	if( r == Null(MsgBuf) )
	{
		ErrorMsg(m,EC_Error+EG_NoMemory);
		return;
	}

	d = GetTargetDir(servinfo);
	
	if( d == NULL )
	{
		ErrorMsg(m,EO_Directory);
		Free(r);
		return;
	}
	
	p = (Pipe *)GetTargetObj(servinfo);
	
	if( p == NULL )
	{
		p = NewPipe(d,objname(pathname));
		/* transfer lock to new pipe */
		Wait( &p->ObjNode.Lock );
		Signal( &servinfo->Target->Lock );
		servinfo->Target = &p->ObjNode;
	}
	
	if( p == NULL )
	{
		ErrorMsg(m,EC_Error|EG_NoMemory|EO_Pipe);
		Free(r);
		return;
	}

	unless( CheckMask(req->Common.Access.Access,(int)(req->Arg.Mode & Flags_Mode)) ) 
	{
		ErrorMsg(m,EC_Error+EG_Protected+EO_Pipe);
		Free(r);
		return;
	}

	if( p->ObjNode.Type == Type_Directory )
	{
		Port reqport = NewPort();
		FormOpenReply(r,m,&p->ObjNode,Flags_Closeable, pathname);
		r->mcb.MsgHdr.Reply = reqport;
		PutMsg(&r->mcb);
		Free(r);
		DirServer(servinfo,m,reqport);
		FreePort(reqport);
		return;
	}
	else {
		unless( servinfo->FnCode & FF_ReOpen ) p->Users++;
		FormOpenReply(r,m,&p->ObjNode,Flags_Closeable, pathname);

		r->mcb.MsgHdr.Reply = p->Connect;
		PutMsg(&r->mcb);
		Free(r);
		return;
	}
}

static void do_delete(ServInfo *servinfo)
{
	MCB *m = servinfo->m;
	DirNode *d;
	Pipe *p;

	d = GetTargetDir(servinfo);
	
	if( d == NULL )
	{
		ErrorMsg(m,EO_Directory);
		return;
	}

	p = (Pipe *)GetTargetObj(servinfo);

	if( p != NULL  && p->ObjNode.Parent != NULL )
	{
		if( p->Users <= 1 )
		{
			p->Finish = TRUE;
			AbortPort(p->Connect,Code_Close);
		}
		if( p->Users == 0 )
		{
			Unlink(&p->ObjNode,FALSE);
			Free(p);
			servinfo->TargetLocked = FALSE;
		}
	}
	
	m->MsgHdr.FnRc = Err_Null;
	ErrorMsg(m,0);
}

static void do_closeobj(ServInfo *servinfo)
{
	MCB *m = servinfo->m;
	DirNode *d;
	Pipe *p;

	d = GetTargetDir(servinfo);
	
	if( d == NULL )
	{
		ErrorMsg(m,EO_Directory);
		return;
	}

	p = (Pipe *)GetTargetObj(servinfo);

	if( p != NULL  && p->ObjNode.Parent != NULL )
	{
		p->Users--;

		if( p->Users <= 1 )
		{
			p->Finish = TRUE;
			SendException(p->Connect,Code_Close);
		}
		if( p->Users == 0 )
		{
			Unlink(&p->ObjNode,FALSE);
			Free(p);
		}
	}
	m->MsgHdr.FnRc = Err_Null;
	ErrorMsg(m,0);
}


Pipe *
NewPipe(
	DirNode *	d,
	char *		name )
{
  Pipe *	p = New( Pipe );
		

  if (p == NULL)
    return NULL;

  InitNode( &p->ObjNode, name, Type_Pipe,
	   Flags_Selectable | Flags_OpenOnGet | Flags_CloseOnSend, DefFileMatrix );

  p->Connect = NewPort();
  p->Finish  = FALSE;

  Insert( d, &p->ObjNode, TRUE);
  
  if (! Fork( 2000, PipeConnect, 4, p ))
    {
      Unlink( &p->ObjNode, TRUE );	/* XXX - added by Bart, 14/08/92 */
	  
      Free( p );
      
      p = Null( Pipe );
    }
		
  return p;

} /* NewPipe */


void PipeConnect(Pipe *p)
{
	MCB m;
	word e;
	Port port1 = NullPort;
	Port port2 = NullPort;

	until( p->Finish )
	{
		InitMCB(&m,0,p->Connect,NullPort,0);
		m.Timeout = -1;

		p->State &= State_Mask;
		p->State |= 1;
		
		e = GetMsg(&m);

		if( e != 0 ) continue;

		port1 = m.MsgHdr.Reply;

		m.Timeout = IOCTimeout/2;

		p->State &= State_Mask;		
		p->State |= 2;	
		e = GetMsg(&m); 

		if( e < 0 )
		{
			InitMCB(&m,0,port1,NullPort,Code_Timeout);
			PutMsg(&m);
			port1 = NullPort;
			continue;
		}
		elif( e == Code_Close )
		{
			InitMCB(&m,0,port1,NullPort,Code_Close);
			PutMsg(&m);
			break;
		}
		
		port2 = m.MsgHdr.Reply;
		p->State &= State_Mask;
		p->State |= 3;	

		InitMCB(&m,MsgHdr_Flags_preserve,port1,port2,Code_Connect);
		e = PutMsg(&m);

		InitMCB(&m,MsgHdr_Flags_preserve,port2,port1,Code_Connect1);
		e = PutMsg(&m);

		p->State += 16;     /* increment no. of connects */
		port1 = port2 = NullPort;			
	}
	FreePort(p->Connect);
}

/* -- End of pipe.c */

