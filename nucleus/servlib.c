/*------------------------------------------------------------------------
--                                                                      --
--                     H E L I O S   N U C L E U S                      --
--                     ---------------------------                      --
--                                                                      --
--             Copyright (C) 1987, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- servlib.c								--
--                                                                      --
--	Server support library.						--
--	Contains code to create and manipulate data structures to 	--
--	implement directories. Also code to perform all the common	--
--	request decoding functions.					--
--                                                                      --
--	Author:  NHG 16/8/87						--
--                                                                      --
------------------------------------------------------------------------*/
/* SccsId:	 %W%	%G% Copyright (C) 1987, Perihelion Software Ltd.*/
/* $Id: servlib.c,v 1.16 1993/09/01 17:56:04 bart Exp $ */

#include <helios.h>	/* standard header */

#define __in_servlib 1	/* flag that we are in this module */



/*--------------------------------------------------------
-- 		     Include Files			--
--------------------------------------------------------*/

#include <root.h>
#include <stdarg.h>
#include <string.h>
#include <queue.h>
#include <protect.h>
#include <message.h>
#include <codes.h>
#include <syslib.h>
#include <process.h>
#include <gsp.h>
#include <servlib.h>

#ifdef STACKCHECK
	/* Stack checking is controlled by -DSTACKCHECK on the command	*/
	/* line, not by -ps1 or similar	pragmas.			*/
#ifdef __TRAN
extern void _Trace(...);
#pragma -s1
static void _stack_error(Proc *p)
{
	_Trace(0xaaaaaaaa,p);
	IOdebug("Servlib stack overflow in %s at %x",p->Name,&p);	
}
#endif
#pragma -s0
#else
# pragma -s1
#endif

#undef Malloc

#ifdef SYSDEB
#define ServDebug if(MyTask->Flags&(Task_Flags_servlib))_ServDebug
static void _ServDebug(char *str, ... );
#endif

extern int _cputime(void);

static void Worker(MsgBuf *m,DispatchInfo *info, DispatchEntry *e);
static void MarshalEntry( MCB *m, ObjNode *o, char *name);
static void HandleLink(LinkNode *l, ServInfo *servinfo);

static void CryptCap( bool encrypt, Key key, Capability *cap);

/* local statics */

static void *SafetyNet = NULL;		/* safety net memory		*/
extern int SafetySize /*= 5*1024*/;	/* size of same, default to 5k  */

/*--------------------------------------------------------
-- InitNode						--
--							--
-- Initialise a node					--
--							--
--------------------------------------------------------*/

extern void InitNode(ObjNode *o, char *name, int type, int flags, Matrix matrix)
{
	Date date = GetDate();

	strncpy(o->Name,name,NameMax-1);
	o->Name[31] = 0;
	o->Type = type;
	o->Flags = flags;
	o->Matrix = matrix;
	InitSemaphore(&o->Lock,1);
	o->Key = NewKey();
	o->Dates.Creation = date;
	o->Dates.Access   = date;
	o->Dates.Modified = date;
	o->Account = 0;
	o->Size = 0;
}

/*--------------------------------------------------------
-- Dispatch						--
--							--
-- General server dispatcher. 				--
--							--
--------------------------------------------------------*/

extern void Dispatch(DispatchInfo *info)
{
	MsgBuf	*m = NULL;
	word fn;
	DispatchEntry *e;

	forever
	{
		m = (MsgBuf *)ServMalloc(sizeof(MsgBuf));

		if( m == Null(MsgBuf) ) { Delay(OneSec*5); continue; }

		m->mcb.MsgHdr.Dest	= info->ReqPort;
		m->mcb.Timeout		= OneSec*30;
		m->mcb.Control		= m->control;
		m->mcb.Data		= m->data;

	lab1:
		/* if we have no safety net, and the largest free block	*/
		/* is more than twice the safety net size, get the	*/
		/* safety net.						*/
		if( SafetyNet == NULL	&& 
		    SafetySize > 0 	&& 
		    (int)Malloc(-2) > 2*SafetySize )
			SafetyNet = Malloc(SafetySize);

		fn = GetMsg(&m->mcb);
		
		if( fn == EK_Timeout ) goto lab1;

		/* IOdebug( "Dispatch: got message for sub system %x", info->SubSys ); */
		
		if( fn < 0 ) break;

		fn = m->mcb.MsgHdr.FnRc & FG_Mask;
#ifdef SYSDEB
ServDebug("Dispatcher %F context >%s< path >%s< next %d",fn,
	m->control[0]==-1?NULL:&m->data[m->control[0]],
	m->control[1]==-1?NULL:&m->data[m->control[1]],
	m->control[2]);	
#endif
		if( fn < FG_Open || fn > FG_LastIOCFn ) e = &info->PrivateProtocol;
		else e = &info->Fntab[(fn-FG_Open) >> FG_Shift];

		/* BLV 27.7.93: stacksize == 0 means that the worker should	*/
		/* run on the current stack rather than in a separate thread.	*/
		/* This is only feasible if the worker performs only internal	*/
		/* processing and then sends back a reply.			*/
		if (e->StackSize == 0)
			Worker(m, info, e);
		elif (! Fork(e->StackSize, Worker, 12, m, info, e) )
		{
			Free(SafetyNet); SafetyNet = NULL;
			ErrorMsg(&m->mcb,EC_Error+info->SubSys+EG_NoMemory);
			goto lab1;
		}
	}

	if( m != NULL ) Free(m);
}

/*--------------------------------------------------------
-- Worker						--
--							--
-- Process to respond to requests.			--
--							--
--------------------------------------------------------*/

static void Worker(MsgBuf *m,DispatchInfo *info, DispatchEntry *e)
{
	DirNode *d;
	ServInfo servinfo;
	
	if( setjmp(servinfo.Escape) != 0 ) goto done;

	servinfo.m = &m->mcb;
	servinfo.Context = info->Root;
	servinfo.Target = (ObjNode *)info->Root;
	servinfo.TargetLocked = false;
	servinfo.FnCode = m->mcb.MsgHdr.FnRc;
	servinfo.DispatchInfo = info;
	m->mcb.MsgHdr.FnRc = info->SubSys;	

	d = GetContext( &servinfo );

#ifdef SYSDEB
ServDebug("Worker %F Context %s rest %s",servinfo.FnCode,servinfo.Pathname,
	m->control[2]==-1?NULL:&m->data[m->control[2]]);
#endif

	if( d == Null(DirNode) )
		ErrorMsg(&m->mcb,0);
	else {
		if( (e == NULL) || (e == &info->PrivateProtocol) ) {
			if( e==NULL ) {
				m->mcb.MsgHdr.FnRc = Err_Null;
				ErrorMsg(&m->mcb,EC_Error+info->SubSys+EG_FnCode );
			} else {
				WordFnPtr f = (WordFnPtr)(e->Fn);

				if( (f==NULL) || (!f(&servinfo)) ) {
					m->mcb.MsgHdr.FnRc = Err_Null;
					ErrorMsg(&m->mcb,EC_Error+info->SubSys+EG_FnCode );
				}
			}
		} else {
			(*e->Fn)(&servinfo);
		}
		
	}

done:
	UnLockTarget(&servinfo);

	Free( m );
	
	if( MyTask->Flags & Task_Flags_fork )
	{
#ifdef __TRAN
		Proc *proc = (Proc *)(e->Fn);

		while( proc->Type != T_Proc ) proc = (Proc *)((word)proc-4);
		IOdebug("%s %",proc->Name);
#elif defined(__ARM)
		char *proc;
		fncast	ansibodge;

		ansibodge.vfn = e->Fn;
		proc = ansibodge.cp;

		proc -= 4; /* point to RPTR */
		proc += *(word *)proc; /* convert to point at fn name */
		IOdebug("%s %%",proc);
#else
		/* @@@ if required, add 'C40 code here */
#endif
	}
}

/*--------------------------------------------------------
-- GetContext						--
--							--
-- Returns the context object of the supplied request   --
-- with the access permissions checked and validated.	--
-- If NULL is returned the global variable ServErr is	--
-- set to the appropriate error code.			--
-- Leaves Context object locked on exit if it was	--
-- successful.						--
--							--
--------------------------------------------------------*/

extern DirNode *GetContext(ServInfo *servinfo)
{
	MCB *		m        = servinfo->m;
	DirNode *	root     = servinfo->Context;
	IOCCommon *	req      = (IOCCommon *)(m->Control);
	int		context  = (int)req->Context;
	int		name     = (int)req->Name;
	int		next     = (int)req->Next;
	byte *		data     = m->Data;
	char *		pathname = servinfo->Pathname;

#ifdef SYSDEB
ServDebug("GetContext %x %x",m->MsgHdr.FnRc,servinfo->FnCode);
#endif

	/* check that this is a GSP message */
	if ( (servinfo->FnCode & FC_Mask) != FC_GSP )
	{
		m->MsgHdr.FnRc |= EC_Error+EG_FnCode;
		return servinfo->Context = NULL;
	}

	/* initialise pathname */

	if( servinfo->DispatchInfo->ParentName == NULL ) MachineName(pathname);
	else strcpy(pathname,servinfo->DispatchInfo->ParentName);

	pathcat(pathname,root->Name);
	
	/* If there is no context string or the pathname has already    */
	/* been entered the capability is not valid but the access 	*/
	/* mask is.							*/
	/* This means that the name string is an absolute path name  	*/
	/* which need not be followed further here. 			*/
	/* Simply modify the access mask by the matrix of the root 	*/
	/* directory.							*/
	if( context == -1 || (name > 0 && next >= name) )
	{
		req->Access.Access = UpdMask(req->Access.Access,root->Matrix);
		servinfo->Target = (ObjNode *)(servinfo->Context = root);
		LockTarget(servinfo);
		return root;
	}

	/* Otherwise simply follow the remains of the context name	*/
	/* through the directories to the end.				*/
	{
		int len;
		DirNode *d = root;
		char pfx[NameMax];
		LockTarget(servinfo);
		while( (len = splitname( pfx, c_dirchar, &data[next])) != 0 )
		{
			d = (DirNode *)Lookup(d,pfx,TRUE);
			if( d == Null(DirNode) ) 
			{
				m->MsgHdr.FnRc |= EC_Error+EG_Name;
				UnLockTarget(servinfo);
				servinfo->Target = NULL; servinfo->Context = NULL;
				return NULL;
			}

			/* unlock old target and lock new */
			Wait(&d->Lock);
			Signal(&servinfo->Target->Lock);

			servinfo->Target = (ObjNode *)(servinfo->Context = d);
			pathcat(pathname,d->Name);
			next += len;
		}

		/* check that the capability is valid */
		if( !GetAccess(&req->Access,d->Key) )
		{
			m->MsgHdr.FnRc |= EC_Error+EG_Invalid+EO_Capability;
			UnLockTarget(servinfo);
			servinfo->Target = NULL; servinfo->Context = NULL;

			return NULL;
		}

		/* check we have SOME access			*/
		unless( req->Access.Access != 0 )
		{
			m->MsgHdr.FnRc |= EC_Error+EG_Protected;
			return servinfo->Context = Null(DirNode);
		}

		req->Next = name;
		return d;
	}

	m->MsgHdr.FnRc |= EC_Fatal;

	UnLockTarget(servinfo);
	servinfo->Target = NULL; servinfo->Context = NULL;
	return NULL;
}

/*--------------------------------------------------------
-- GetTarget						--
--							--
-- Returns the target object of the supplied request    --
-- with the access permissions checked and validated.	--
-- The request must have been passed through GetContext --
-- first.						--
-- We are entered with context == target and the node	--
-- locked.						--
--							--
--------------------------------------------------------*/

extern ObjNode *GetTarget(ServInfo *servinfo)
{
	ObjNode *o;
	DirNode *d;

	d = GetTargetDir(servinfo);

	if( d == NULL ) return NULL;

	o = GetTargetObj(servinfo);

	return o;
}

/*--------------------------------------------------------
-- GetTargetObj						--
--							--
-- Returns the target object of the supplied request    --
-- with the access permissions checked and validated.	--
-- The request must have been passed through GetContext --
-- and GetTargetDir first.				--
-- servinfo->Target is the parent directory, and is	--
-- locked.						--
--							--
--------------------------------------------------------*/

extern ObjNode *GetTargetObj(ServInfo *servinfo)
{
	DirNode *d = (DirNode *)servinfo->Target;
	ObjNode *o = Null(ObjNode);
	MCB *m = servinfo->m;
	IOCCommon *req = (IOCCommon *)(m->Control);
	byte *data = m->Data;
	word next = req->Next;
	word len = 0;
	char name[NameMax];
	char *pathname = servinfo->Pathname;
#ifdef SYSDEB
ServDebug("GetTargetObj c %s t %s p %s",servinfo->Context->Name,servinfo->Target->Name,pathname);
#endif
	/* If the context is the target we have artificially stepped	*/
	/* back to its parent. Step back down here, except if the	*/
	/* context is the root, when we have not stepped back.		*/
	if( next == -1 || data[next] == '\0' )
	{
		/* if target is root then context == target here */
		if( servinfo->Context == (DirNode *)servinfo->Target ) 
		{ o = servinfo->Target; goto found; }

		/* else step on down to context */
		Wait(&servinfo->Context->Lock);
		Signal(&servinfo->Target->Lock);

		o = servinfo->Target = (ObjNode *)servinfo->Context;
		goto found;
	}

	if( (len = splitname(name, c_dirchar, &data[next] )) == 0 )
	{
		m->MsgHdr.FnRc |= EC_Error+EG_Name;
		return NULL;
	}

	/* special case . and .. */
	if( name[0] == '.' && name[1] == '\0' )
	{
		req->Next = next+len;		
		return (ObjNode *)d;
	}
	elif( name[0] == '.' && name[1] == '.' && name[2] == '\0') 
	{
		int l = strlen(pathname);
		while( pathname[l--] != c_dirchar );
		pathname[l+1] = '\0';
		o = (ObjNode *)(d->Parent);
		UnLockTarget(servinfo);
		servinfo->Target = (ObjNode *)o;
		LockTarget(servinfo);
		req->Access.Access = UpdMask(req->Access.Access,o->Matrix);
		goto found;
	}
	else 
	{
		if( (d->Type & Type_Flags) != Type_Directory )
		{
			/* if the parent is not a directory, moan */
			m->MsgHdr.FnRc |= EC_Error|EG_Invalid|EO_Directory;
			return NULL;
		}
		o = (ObjNode *)Lookup( d, name, servinfo->TargetLocked );
		pathcat(pathname, name );
	}

	if( o == NULL )
	{
		/* if the required object is not found, we keep its	*/
		/* parent dir locked and set as the target.		*/
		m->MsgHdr.FnRc |= EC_Error+EG_Name;
		return NULL;
	}

	req->Access.Access = UpdMask(req->Access.Access,o->Matrix);

	/* lock o and unlock target */
	Wait( &o->Lock );
	Signal(&servinfo->Target->Lock );
	servinfo->Target = o;

	/* This is general code for all above variations. We come here	*/
	/* with o locked.						*/
found:
	if( o->Type == Type_Link )
	{
		switch( servinfo->FnCode & FG_Mask )
		{
		default:
			req->Next = next+len;
			HandleLink((LinkNode *)o,servinfo);
			return NULL;
	
		/* These should be applied to the link, not to the	*/
		/* linked object.					*/
		case FG_Delete:
		case FG_Protect:
		case FG_Rename:
		case FG_ObjectInfo:
			break;
		}
		
	}

	req->Next = next+len;
	
	return servinfo->Target = o;
}

/*--------------------------------------------------------
-- GetTargetDir						--
--							--
-- Returns the directory containing the 		--
-- target object of the supplied request		--
-- with the access permissions checked and validated.	--
-- The request must have been passed through GetContext --
-- first.						--
-- This is used in request which affect an object's	--
-- directory, e.g. create, delete, rename.		--
--							--
-- Entered with Context == Target, and node locked.	--
--							--
--------------------------------------------------------*/

extern DirNode *GetTargetDir(ServInfo *servinfo)
{
	MCB *		m        = servinfo->m;
	DirNode *	context  = servinfo->Context;
	byte *		data     = m->Data;
	IOCCommon *	req      = (IOCCommon *)(m->Control);
	int		next     = (int)req->Next;
	char *		pathname = servinfo->Pathname;
	
#ifdef SYSDEB
ServDebug("GetTargetDir c %s t %s p %s",servinfo->Context->Name,servinfo->Target->Name,pathname);
#endif
	/* if context is target, step back to parent, unless we are at	*/
	/* the root in which case stay where we are.			*/
	if( next == -1 || data[next] == '\0' )
	{
		req->Next = -1;
		if( servinfo->Target == (ObjNode *)servinfo->DispatchInfo->Root ) 
			return (DirNode *)servinfo->Target;
		/* if we try to lock the parent before unlocking the node */
		/* we could get into a deadlock here. 			  */
		UnLockTarget(servinfo);
		servinfo->Target = (ObjNode *)context->Parent;
		LockTarget(servinfo);
		return context->Parent;
	}

	/* Otherwise we must follow the path through the directory	*/
	/* structure, checking access permissions on the way.		*/
	{
		int len = 0;
		DirNode *d = context;
		char name[NameMax];
		AccMask mask = req->Access.Access;

		forever 
		{
#ifdef SYSDEB
ServDebug("Follow dir %s mask %x rest %s",d->Name,mask,&data[next]);
#endif
		    switch( d->Type & Type_Flags )
		    {
		    case Type_Directory:
			
			while( data[next] == c_dirchar ) next++;

			len = splitname( name, c_dirchar, &data[next]);

			if( data[next+len] == '\0' ) goto done;

			/* special case . and .. */
			if( name[0] == '.' && name[1] == '\0' )
			{
				next += len;
				continue;
			}
			else if( name[0] == '.' && name[1] == '.' 
				&& name[2] == '\0') 
			{
				int l = strlen(pathname);
				while( pathname[l--] != c_dirchar );
				pathname[l+1] = '\0';
				UnLockTarget(servinfo);
				if ((servinfo->Target = (ObjNode *)(d = d->Parent)) == NULL)
					return NULL;
				LockTarget(servinfo);
				mask = UpdMask(mask,d->Matrix);
				next += len;
				continue;
			}

			unless( CheckMask(mask,AccMask_R) )
			{
				m->MsgHdr.FnRc |= EC_Error+EG_Protected;
				UnLockTarget(servinfo);
				servinfo->Target = Null(ObjNode);
				return Null(DirNode);
			}

			pathcat(pathname, name );
			d = (DirNode *)Lookup(d, name, servinfo->TargetLocked);

			if( d == NULL )
			{
				m->MsgHdr.FnRc |= EC_Error+EG_Name;
				UnLockTarget(servinfo);
				servinfo->Target = NULL;
				return NULL;
			}
			
			mask = UpdMask(mask,d->Matrix);

			/* lock d and unlock parent */
			Wait(&d->Lock);
			Signal(&servinfo->Target->Lock);

			servinfo->Target = (ObjNode *)d;
			next += len;
				
			break;
			
		    case 0:
		    	if( d->Type == Type_Link )
		    	{
				req->Access.Access = mask;			
				req->Next = next;
				HandleLink((LinkNode *)d,servinfo);
				/* we never actually return */
			}
			break;

		    default:
			m->MsgHdr.FnRc |= EC_Error|EG_Invalid|EO_Directory;
			UnLockTarget(servinfo);
			servinfo->Target = Null(ObjNode);
			return Null(DirNode);
		    	
		    } /* switch */
		    
		} /* for */
	done:
		req->Access.Access = mask;
		req->Next = next;
		servinfo->Target = (ObjNode *)d;
		return d;
	}
}

/*--------------------------------------------------------
-- HandleLink						--
--							--
-- Deal with a symbolic link in the directory path.	--
-- This routine does not return, but longjumps out to 	--
-- the root of the worker process.			--
-- Entered with Target locked.				--
--							--
--------------------------------------------------------*/

static void HandleLink(LinkNode *l, ServInfo *servinfo)
{
	MCB *mcb = servinfo->m;
	MsgBuf *m = New(MsgBuf);
	IOCCommon *req = (IOCCommon *)mcb->Control;
	word next = req->Next;
	byte *data = mcb->Data;

	if( m == NULL )
	{
		ErrorMsg(mcb,EC_Error|EG_NoMemory|EO_Message);
		return;
	}

	m->mcb.Control	= m->control;
	m->mcb.Data	= m->data;

	InitMCB(&m->mcb,mcb->MsgHdr.Flags,NullPort,
			mcb->MsgHdr.Reply,servinfo->FnCode);

	MarshalString(&m->mcb,l->Link);
	if( next != -1 && data[next] != '0' ) MarshalString(&m->mcb,&data[next]);
	else MarshalWord(&m->mcb,-1);
#ifdef SYSDEB
ServDebug("HandleLink %s %s",l->Link,&data[next]);
#endif
	MarshalWord(&m->mcb,1);

	MarshalCap(&m->mcb,&l->Cap);

	/* copy across any more parameters in the control vector */

	while( mcb->MsgHdr.ContSize > m->mcb.MsgHdr.ContSize )
	{
		word i = m->mcb.MsgHdr.ContSize;
		MarshalWord(&m->mcb,mcb->Control[i]);
	}

	/* and in the data vector */
	while( data[next++] != '\0' );

	if( next < mcb->MsgHdr.DataSize )
		MarshalData(&m->mcb,mcb->MsgHdr.DataSize-next,&data[next]);

	SendIOC(&m->mcb);

	Free(m);

	UnLockTarget(servinfo);
	
	longjmp(servinfo->Escape,1);
}

/*--------------------------------------------------------
-- GetName						--
--							--
-- Extracts the final part of the object named in the	--
-- MCB. Updates pathname accordingly.			--
--							--
--------------------------------------------------------*/

extern bool GetName(MCB *m, string name, string pathname)
{
	IOCCommon *req = (IOCCommon *)(m->Control);
	byte *data = m->Data;
	int next = (int)req->Next;

/*debug("GetName %s %s",&data[next],pathname);	*/
	if( splitname(name, c_dirchar, &data[next] ) == 0 )
	{
		m->MsgHdr.FnRc |= EC_Error+EG_Name;
		return false;
	}

	pathcat(pathname, name );
	return true;
}

extern void pathcat(string s1, string s2)
{
	while( *s1 ) s1++;
	if( *(s1-1) != c_dirchar && *s2 != c_dirchar ) *s1++ = c_dirchar;
	while( (*s1++ = *s2++) != 0 );
}

extern string objname(string path)
{
	string p = path + strlen(path);
	
	until( *p == c_dirchar || p < path ) p--;

	return p+1;
}

extern word addint(char *s, word i)
{	
	word len;

	if( i == 0 ) return strlen(s);

	len = addint(s,i / 10);
  
	s[len] = (char)(i % 10) + '0';
  
	s[len+1] = '\0';

	return len + 1;
}

/*--------------------------------------------------------
-- Lookup						--
--							--
-- Search a directory for a named entry			--
--							--
--------------------------------------------------------*/

static bool FindNode(ObjNode *node, char *name)
{
	return strcmp(node->Name,name) == 0;
}

extern ObjNode *Lookup(DirNode *d, char *name, bool dirlocked)
{
	ObjNode *x;

	if( !dirlocked ) Wait( &d->Lock );

	x = (ObjNode *)SearchList(&d->Entries,FindNode,name);

	if( !dirlocked ) Signal( &d->Lock );

	return x;
}

/*--------------------------------------------------------
-- Insert						--
--							--
-- Insert an ObjNode into the directory	which is the	--
-- Target in servinfo.					--
--							--
--------------------------------------------------------*/

extern void Insert(DirNode *d, ObjNode *obj, bool dirlocked)
{

	if( !dirlocked ) Wait( &d->Lock );
	obj->Parent = d;
	AddTail(&d->Entries,&obj->Node);
	d->Nentries++;
	d->Dates.Modified = GetDate();
	if( !dirlocked ) Signal( &d->Lock );
}

/*--------------------------------------------------------
-- Unlink						--
--							--
-- Remove given ObjNode from its parent directory.	--
-- dirlocked indicates whether we have the parent dir	--
-- locked.						--
--							--
--------------------------------------------------------*/

extern void Unlink(ObjNode *obj, bool dirlocked)
{
	DirNode *dir = obj->Parent;
	
	if( dir == NULL ) return;
	
	if( !dirlocked ) Wait( &dir->Lock );

	if( obj->Parent == dir )
	{
		Remove(&obj->Node);
		dir->Nentries--;
		dir->Dates.Modified = GetDate();
		obj->Parent = NULL;
	}

	if( !dirlocked ) Signal( &dir->Lock );
}

/*--------------------------------------------------------
-- FormOpenReply					--
--							--
-- Build a reply message for Open,Create and Locate.	--
--							--
--------------------------------------------------------*/

extern void FormOpenReply(MsgBuf *r, MCB *m, ObjNode *o, word flags, char *pathname)
{
	IOCCommon *req = (IOCCommon *)(m->Control);
	Capability cap;


	if( m->MsgHdr.Reply & Port_Flags_Remote ) flags |= Flags_Remote;

	*((int *)&(r->mcb)) = 0;
	r->mcb.MsgHdr.Dest = m->MsgHdr.Reply;
	r->mcb.MsgHdr.Reply = NullPort;
	r->mcb.MsgHdr.FnRc = Err_Null;
	
	r->mcb.Timeout = IOCTimeout;
	r->mcb.Control = r->control;
	r->mcb.Data    = r->data;

	MarshalWord(&r->mcb,o->Type);
	MarshalWord(&r->mcb,o->Flags|flags);
	NewCap(&cap, o, req->Access.Access);
	MarshalCap(&r->mcb,&cap);
	MarshalString(&r->mcb,pathname);
}


/*--------------------------------------------------------
-- DirServer						--
--							--
-- Service Read requests on a directory			--
--							--
--------------------------------------------------------*/

extern void DirServer(ServInfo *servinfo, MCB *m, Port reqport)
{
	DirNode *dir = (DirNode *)servinfo->Target;
	
	UnLockTarget(servinfo);
	
	forever
	{
		word e;
		m->MsgHdr.Dest = reqport;
		m->Timeout = StreamTimeout;

		e = GetMsg(m);

		if( e == EK_Timeout ) break;

		if( e < Err_Null ) continue;

		switch( m->MsgHdr.FnRc & FG_Mask )
		{
		case FG_Read:
		{
			ReadWrite *r = (ReadWrite *)(m->Control);
			word pos = r->Pos;
			word size = r->Size;
			word dirsize;
			word dpos = sizeof(DirEntry);
			word tfr = 0;
			word seq = 0;
			Port reply = m->MsgHdr.Reply;
			ObjNode *o;

			Wait( &dir->Lock );

				/* See if .. is defined for this directory */
			dirsize = (dir->Nentries+1) * sizeof(DirEntry);
			if (dir->Parent != NULL) 
			 { dirsize += sizeof(DirEntry);
			   dpos    += sizeof(DirEntry);
			 }

			if( pos == dirsize )
			{
				InitMCB(m,0,reply,NullPort,ReadRc_EOF|seq);
				PutMsg(m);
				Signal( &dir->Lock );
				break;
			}
			
			if( pos % sizeof(DirEntry) != 0 ||
			    pos < 0 || pos > dirsize )
			{
				Signal( &dir->Lock );
				ErrorMsg(m,EC_Error+EG_Parameter+1);
				break;
			}
			if( pos + size > dirsize ) size = dirsize - pos;

			o = (ObjNode *)dir->Entries.Head;

			InitMCB(m,MsgHdr_Flags_preserve,
				reply,NullPort,ReadRc_More|seq);

			while( size >= sizeof(DirEntry) )
			{
				/* special cases for . & .. */
				if( pos == 0 ) 
					MarshalEntry(m,(ObjNode *)dir,".");
				else if( pos == sizeof(DirEntry) && dir->Parent != NULL)
					MarshalEntry(m,(ObjNode *)dir->Parent,"..");
				else
				{
					if( dpos == pos )
					{
						MarshalEntry(m,o,o->Name);
						o = (ObjNode *)o->Node.Next;
						dpos += sizeof(DirEntry);
					}
					else 
					{
						dpos += sizeof(DirEntry);
						o = (ObjNode *)o->Node.Next;
						continue;
					}
				}

				tfr += sizeof(DirEntry);
				size -= sizeof(DirEntry);
				pos += sizeof(DirEntry);

				if( size < sizeof(DirEntry) ||
				    IOCDataMax - tfr < sizeof(DirEntry) )
				{
					word e;
					if( size < sizeof(DirEntry) )
					{
						m->MsgHdr.Flags = 0;
						if( dpos == dirsize )
						    m->MsgHdr.FnRc = ReadRc_EOF|seq;
						else
						    m->MsgHdr.FnRc = ReadRc_EOD|seq;
					}

					e = PutMsg(m);
					InitMCB(m,MsgHdr_Flags_preserve,
						reply,NullPort,ReadRc_More|seq);
					tfr = 0;
					seq += ReadRc_SeqInc;
					m->MsgHdr.FnRc = ReadRc_More|seq;
				}

			}

			Signal( &dir->Lock );

			if( tfr > 0 )
			{
				m->MsgHdr.Flags = 0;
				if( dpos == dirsize )
					m->MsgHdr.FnRc = ReadRc_EOF|seq;
				else
					m->MsgHdr.FnRc = ReadRc_EOD|seq;
				e = PutMsg(m);
			}

			break;
		}
		
		case FG_Close:
			ErrorMsg(m,Err_Null);
			return;

		case FG_GetSize:
		{
			InitMCB(m,0,m->MsgHdr.Reply,NullPort,Err_Null);
			MarshalWord(m, (dir->Nentries + 1) * sizeof(DirEntry) +
				((dir->Parent == NULL) ? 0 : sizeof(DirEntry)));
			PutMsg(m);
			break;
		}

		case FG_Seek:
		case FG_Write:
		case FG_SetSize:
			ErrorMsg(m,EC_Error+EG_WrongFn+EO_Directory);
		default:
			ErrorMsg(m,EC_Error+EG_FnCode+EO_Directory);
			break;
		}
	}

}

static void MarshalEntry( MCB *m, ObjNode *o, char *name)
{
	MarshalData(m,4,(byte *)&o->Type);
	MarshalData(m,4,(byte *)&o->Flags);
	MarshalData(m,4,(byte *)&o->Matrix);
	MarshalData(m,32,name);
}

extern void MarshalInfo(MCB *m, ObjNode *o)
{
	InitMCB(m,0,m->MsgHdr.Reply,NullPort,Err_Null);
	MarshalEntry(m,o,o->Name);
	if( o->Type == Type_Link )
	{
		LinkNode *l = (LinkNode *)o;
		MarshalData(m,sizeof(Capability),(byte *)&l->Cap);
		MarshalData(m,(word)strlen(l->Link)+1,l->Link);
	}
	else
	{
		word size = o->Size;
		MarshalData(m,4,(byte *)&o->Account);
		if( o->Type & Type_Directory )
			size = (((DirNode *)o)->Nentries+2) * sizeof(DirEntry);
		MarshalData(m,4,(byte *)&size);
		MarshalData(m,4,(byte *)&o->Dates.Creation);
		MarshalData(m,4,(byte *)&o->Dates.Access);
		MarshalData(m,4,(byte *)&o->Dates.Modified);
	}
}

/*--------------------------------------------------------
-- DoLocate						--
--							--
-- Generic support for object location.			--
--							--
--------------------------------------------------------*/

extern void DoLocate(ServInfo *servinfo)
{
	MCB *m = servinfo->m;
	MsgBuf *r;
	ObjNode *o;
	IOCCommon *req = (IOCCommon *)(m->Control);
	char *pathname = servinfo->Pathname;

	o = GetTarget(servinfo);

	if( o == Null(ObjNode) )
	{
		ErrorMsg(m,0);
		return;
	}

	/* The client is only allowed to know about the object if he	*/
	/* has any access at all.					*/
	if( req->Access.Access == 0 ) 
	{
		ErrorMsg(m,EC_Error+EG_Protected);
		return;
	}

	r = New(MsgBuf);

	if( r == Null(MsgBuf) )
	{
		ErrorMsg(m,EC_Error+EG_NoMemory+EO_Message);
		return;
	}

	FormOpenReply(r, m, o, 0, pathname);

	PutMsg(&r->mcb);

	Free(r);
	
	o->Dates.Access = GetDate();
}

/*--------------------------------------------------------
-- DoRename						--
--							--
-- General support for an internal rename operation	--
--							--
--------------------------------------------------------*/

extern void DoRename(ServInfo *servinfo)
{
	MCB *m = servinfo->m;
	IOCMsg2 *req = (IOCMsg2 *)(m->Control);
	word hdr = *(word *)m;
	AccMask mask = req->Common.Access.Access;
	DirNode *src, *dest;
	ObjNode *o, *o1;
	char *name2;
	char *pathname = servinfo->Pathname;
	
	src = GetTargetDir(servinfo);

	if( src == Null(DirNode) )
	{
		ErrorMsg(m,EO_Directory);
		return;
	}

	unless( CheckMask(req->Common.Access.Access,AccMask_W) ) 
	{
		ErrorMsg(m,EC_Error+EG_Protected+EO_Directory);
		return;
	}

	o = GetTargetObj(servinfo);

	if( o == NULL )
	{
		ErrorMsg(m,EC_Error+EG_Unknown);
		return;
	}

	if( o == (ObjNode *)servinfo->DispatchInfo->Root )
	{
		ErrorMsg(m,EC_Error+EG_WrongFn+EO_Directory);
		return;
	}
	
	/* we now have the source object, find the dest dir	*/
	/* restore state to context dir				*/
	/* at this point we have the target locked		*/
	
	*(word *)m = hdr;
	req->Common.Access.Access = mask;
	req->Common.Next = req->Arg.ToName;
	servinfo->Target = (ObjNode *)(servinfo->Context);
	Wait(&servinfo->Target->Lock);		/* relock context	*/
	
	dest = GetTargetDir( servinfo );

	if( dest == NULL )
	{
		ErrorMsg(m,EO_Directory);
		Signal(&o->Lock);		/* unlock object	*/
		return;
	}

	unless( CheckMask(req->Common.Access.Access,AccMask_W) ) 
	{
		ErrorMsg(m,EC_Error+EG_Protected+EO_Directory);
		Signal(&o->Lock);		/* unlock object	*/
		return;
	}

	o1 = GetTargetObj(servinfo);

	name2 = objname(pathname);

	if( o1 != NULL )
	{
		ErrorMsg(m,EC_Error|EG_Create);
		Signal(&o->Lock);		/* unlock object	*/
		return;
	}
	else m->MsgHdr.FnRc = Err_Null;
		
	/* if we get here we have the source and dest directories	*/
	/* the object itself, and the new name in name2.		*/
	/* now do the rename.						*/

	strcpy(o->Name,name2);
	
	if( src != dest )
	{
		Unlink(o, false);
		Insert((DirNode *)servinfo->Target, o, servinfo->TargetLocked);
	}

	o->Dates.Access = GetDate();
	o->Dates.Modified = GetDate();
		
	Signal(&o->Lock);		/* unlock object */
	UnLockTarget( servinfo );	/* unlock dest dir */
	
	ErrorMsg(m, Err_Null);	
}

/*--------------------------------------------------------
-- DoLink						--
--							--
-- General support for creating a link 			--
--							--
--------------------------------------------------------*/

extern void DoLink(ServInfo *servinfo)
{
	MCB *m = servinfo->m;
	IOCMsg3 *req = (IOCMsg3 *)(m->Control);
	byte *data = m->Data;
	DirNode *d;
	ObjNode *o;
	LinkNode *l;
	

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

	o = GetTargetObj(servinfo);
		
	if( o != Null(ObjNode) )
	{
		ErrorMsg(m,EC_Error+EG_Create);
		return;
	}
	else m->MsgHdr.FnRc = 0;

	/* We now know that there is not an existing entry with the	*/
	/* desired name. Install the link.				*/

	l = (LinkNode *)ServMalloc(sizeof(LinkNode) + (word)strlen(&data[req->Name]));

	if( l == Null(LinkNode) )
	{
		ErrorMsg(m,EC_Error+EG_NoMemory+EO_Link);
		return;
	}

	InitNode(&l->ObjNode, objname(servinfo->Pathname), Type_Link, 0, DefLinkMatrix );
	l->Cap = req->Cap;
	strcpy(l->Link,&data[req->Name]);

	Insert( (DirNode *)servinfo->Target, &l->ObjNode, servinfo->TargetLocked );

	ErrorMsg(m, Err_Null);	
}

/*--------------------------------------------------------
-- DoProtect						--
--							--
-- Alter the matrix on an object			--
--							--
--------------------------------------------------------*/

extern void DoProtect(ServInfo *servinfo)
{
	MCB *m = servinfo->m;
	IOCMsg2 *req = (IOCMsg2 *)(m->Control);
	ObjNode *o;
	Matrix newmatrix = req->Arg.Matrix;
	
	o = GetTarget(servinfo);

	if( o == Null(ObjNode) )
	{
		ErrorMsg(m,0);
		return;
	}

	unless( CheckMask(req->Common.Access.Access,AccMask_A) ) 
	{
		ErrorMsg(m,EC_Error+EG_Protected);
		return;
	}

	/* We are allowed to alter the matrix, ensure that it is 	*/
	/* resonable, and that someone still has delete or alter access */

	if( (UpdMask(AccMask_Full, newmatrix) & (AccMask_A|AccMask_D)) == 0 )
	{
		ErrorMsg(m,EC_Error+EG_Invalid+EO_Matrix);
		return;
	}

	o->Matrix = newmatrix;

	o->Dates.Access = GetDate();
	o->Dates.Modified = GetDate();	
	
	ErrorMsg(m,0);
}

/*--------------------------------------------------------
-- DoRevoke						--
--							--
-- Alter the encryption key on an object, invalidating	--
-- all existing capabilities.				--
--							--
--------------------------------------------------------*/

extern void DoRevoke(ServInfo *servinfo)
{
	MCB *m = servinfo->m;
	IOCCommon *req = (IOCCommon *)m->Control;
	ObjNode *o;
	Capability cap;
		
	o = GetTarget(servinfo);

	if( o == Null(ObjNode) )
	{
		ErrorMsg(m,0);
		return;
	}

	/* only allow revocation if the target is the context object 	*/
	/* and the client has alter rights 				*/
	unless( o == (ObjNode *)servinfo->Context && 
		CheckMask(req->Access.Access,AccMask_A) ) 
	{
		ErrorMsg(m,EC_Error+EG_Protected);
		return;
	}

	/* get out all the rights encoded in the capability	*/
	req->Access.Access = AccMask_Full;
	GetAccess(&req->Access, o->Key);
	
	/* change the key */
	o->Key = NewKey();

	/* pass back a new capability with all the original rights */
	NewCap(&cap, o, req->Access.Access);
	InitMCB(m,0,m->MsgHdr.Reply,NullPort,Err_Null);
	MarshalCap(m,&cap);
	PutMsg(m);
	
	o->Dates.Access = GetDate();
	o->Dates.Modified = GetDate();
}

/*--------------------------------------------------------
-- DoObjInfo						--
--							--
-- Generic ObjInfo support. Returns the correct things	--
-- for directories and links, but all other things are	--
-- given minimal treatment.				--
--							--
--------------------------------------------------------*/

extern void DoObjInfo(ServInfo *servinfo)
{
	MCB *m = servinfo->m;
	IOCMsg1 *req = (IOCMsg1 *)(m->Control);
	ObjNode *o;

/*debug("DoObjInfo %x %s %s",m,&servinfo->Context->Name,servinfo->pathname);*/

	o = GetTarget(servinfo);

	if( o == Null(ObjNode) )
	{
		ErrorMsg(m,0);
		return;
	}

	/* only allow if user has any access at all */
	if( req->Common.Access.Access == 0 ) 
	{
		ErrorMsg(m,EC_Error+EG_Protected);
		return;
	}

	MarshalInfo(m,o);

	PutMsg(m);
	
	o->Dates.Access = GetDate();
}

/*--------------------------------------------------------
-- DoSetDate						--
--							--
-- Alter the date on an object				--
--							--
--------------------------------------------------------*/

extern void DoSetDate(ServInfo *servinfo)
{
	MCB *m = servinfo->m;
	IOCMsg4 *req = (IOCMsg4 *)(m->Control);
	ObjNode *o;
	DateSet *dates = &req->Dates;
	word e = Err_Null;
	
/*debug("DoSetDate %x %x %s",m,context,servinfo->pathname);*/

	o = GetTarget(servinfo);

	if( o == Null(ObjNode) )
	{
		ErrorMsg(m,0);
		return;
	}

	/* only allow the user to set the date on an object if he	*/
	/* could write to it.						*/
	unless( CheckMask(req->Common.Access.Access,AccMask_W) ) 
	{
		ErrorMsg(m,EC_Error+EG_Protected);
		return;
	}

	if ( dates->Creation != 0 ) o->Dates.Creation = dates->Creation;
	if ( dates->Access != 0 ) o->Dates.Access = dates->Access;
	if ( dates->Modified != 0 ) o->Dates.Modified = dates->Modified;
		
	ErrorMsg(m,e);
}

/*--------------------------------------------------------
-- DoRefine						--
--							--
-- Generate a new capability with restricted access	--
--							--
--------------------------------------------------------*/

extern void DoRefine(ServInfo *servinfo)
{
	MCB *m = servinfo->m;
	IOCMsg2 *req = (IOCMsg2 *)(m->Control);
	ObjNode *o;
	Capability cap;
	AccMask newmask = req->Arg.AccMask;	
	
	o = GetTarget(servinfo);

	if( o == Null(ObjNode) )
	{
		ErrorMsg(m,0);
		return;
	}

	InitMCB(m,0,m->MsgHdr.Reply,NullPort,Err_Null);

	/* If the client has Alter permission to the object just use 	*/
	/* the new mask as given, otherwise restrict it by his actual	*/
	/* access. He could get the same effect by changing the objects	*/
	/* matrix which is a lot less safe.			     	*/
		
	unless( CheckMask(req->Common.Access.Access,AccMask_A) )	
		newmask &= req->Common.Access.Access;
		
	NewCap(&cap, o, newmask );
	
	MarshalCap(m,&cap);

	PutMsg(m);
	
	o->Dates.Access = GetDate();
}

/*--------------------------------------------------------
-- InvalidFn						--
-- NullFn						--
--							--
-- Server function table default entries which either	--
-- complain or quietly reply respectively.		--
--							--
--------------------------------------------------------*/

extern void InvalidFn(ServInfo *servinfo)
{
	ErrorMsg(servinfo->m,EC_Error+EG_WrongFn+EO_Object);
}

extern void NullFn(ServInfo *servinfo)
{
	ErrorMsg(servinfo->m,Err_Null);
}

/*--------------------------------------------------------
-- ErrorMsg						--
--							--
-- Return the given error to the sender of the message.	--
--							--
--------------------------------------------------------*/

extern void ErrorMsg(MCB *mcb, word err)
{
	err |= mcb->MsgHdr.FnRc;
	
	if( mcb->MsgHdr.Reply == NullPort ) return;

/*debug("ErrorMsg %x %x %x",mcb,mcb->MsgHdr.Reply,err);*/

	*((int *)mcb) = 0;	/* no shorts at present */
	mcb->MsgHdr.Dest = mcb->MsgHdr.Reply;
	mcb->MsgHdr.Reply = NullPort;
	mcb->MsgHdr.FnRc = err;
	PutMsg(mcb);
}

/*--------------------------------------------------------
-- ServMalloc						--
--							--
-- Server Library Malloc routine, if a Malloc fails the --
-- SafetyNet block is freed, but a memory failure is	--
-- reported anyway. This should allow the server to 	--
-- respond to any subsequent cleanup requests sent it.	--
--							--
--------------------------------------------------------*/

extern void *ServMalloc(word size)
{
	void *v = Malloc(size);
	
	if( v == NULL ) 
	{
		Free(SafetyNet); 
		SafetyNet = NULL;
	}
	return v;
}

/*--------------------------------------------------------
-- UpdMask						--
--							--
-- Generate a new mask from the old, modified by the	--
-- matrix.						--
--							--
--------------------------------------------------------*/

extern AccMask UpdMask(AccMask mask, Matrix matrix)
{
	AccMask res = 0;
/*debug("UpdMask(%x %x)",mask,matrix);*/
	if( mask & AccMask_V ) res |=  (int)matrix        & 0xff;
	if( mask & AccMask_X ) res |= ((int)matrix >> 8)  & 0xff;
	if( mask & AccMask_Y ) res |= ((int)matrix >> 16) & 0xff;
	if( mask & AccMask_Z ) res |= ((int)matrix >> 24) & 0xff;
	return res;
}

/*--------------------------------------------------------
-- CheckMask						--
--							--
-- Check that the given mask allows the given access	--
-- 							--
--							--
--------------------------------------------------------*/

extern int CheckMask(AccMask mask,AccMask access)
{
	if( (access == 0) || ((mask & access) != access) ) return 0;
	return 1;
}

/*--------------------------------------------------------
-- NewCap						--
--							--
-- Create a new capability for the object with the 	--
-- given access mask.					--
--							--
--------------------------------------------------------*/

extern void NewCap(Capability *cap, ObjNode *obj, AccMask mask)
{
	int i;
	Key key = obj->Key;
	byte check = (byte)(key>>16)&0xff;

	cap->Access = mask;
	
	for( i=0; i<7 ; cap->Valid[i++] = check );
	cap->Valid[0] = mask;
	cap->Valid[3] = mask;

	CryptCap(1, key, cap);
}

/*--------------------------------------------------------
-- GetAccess						--
--							--
-- Update the capability Access field with the access	--
-- rights actually allowed by the capability.		--
-- Thought: if this ran at high priority, noone would	--
-- be able to interrupt and take a peek at the decypted	--
-- capability.						--
--							--
--------------------------------------------------------*/

extern word GetAccess(Capability *cap, Key key)
{
	Capability c;
	AccMask mask;
	int i;
	byte check = (byte)(key>>16)&0xff;

	c = *cap;
	
	/* decrypt the capability */
	CryptCap(0, key, &c);

	/* check that the capability is valid */
	for( i = 0; i < 7 ; i++ )
	{
		if( i == 0 || i == 3 ) continue;
		if(c.Valid[i] != check) break;
	}

	if( i != 7 ) return false;

	mask = c.Valid[0];

	if( mask != c.Valid[3] ) return false;
	
	cap->Access &= mask;
	
	return true;
}

/*--------------------------------------------------------
-- CryptCap						--
--							--
-- Capability encryptor. 				--
--							--
--------------------------------------------------------*/

static void CryptCap( bool encrypt, Key key, Capability *cap)
{
#if 0
	Crypt( encrypt, key, (byte *)&cap->Valid, 7);
#else
	AccMask mask = cap->Access;	
	uword s[2];
	uword kk[2];
	uword ks[32];
	
	kk[0] = kk[1] = key;
	
	DES_KeySchedule(kk, ks);
	
	s[0] = s[1] = 0;
	
	DES_Inner(TRUE, s, kk, ks);
	
	if( !encrypt ) s[0] = s[1] = ((uword *)cap)[1];

	((uword *)cap)[1] ^= kk[0];

	if( encrypt ) s[0] = s[1] = ((uword *)cap)[1];
		
	DES_Inner(TRUE, s, kk, ks);
	
	((uword *)cap)[0] ^= kk[0];

	cap->Access = mask;
#endif
}

/*--------------------------------------------------------
-- Crypt						--
--							--
-- Encryption/decryption routine. Intended for 		--
-- capabilities, but may be used for anything.		--
--							--
--------------------------------------------------------*/

extern void Crypt(bool encrypt, Key key, byte *data, word size)
{
#if 1
	uword kk[2];
	kk[0] = kk[1] = key;
	DES_CFB(encrypt, (char *)&kk, data, (int)size);
#else
	word c;
	word salt = size;

	while( size-- )
	{
		c = *data;

		/* we are using a 29 bit key, if it overflows, feed bit	*/
		/* back in at the bottom.				*/
		key &= 0x1fffffff;
		if( key & 0x10000000 ) key ^= 0x0040a001;

		/* encrypt the character */
		c = (key & 0xff) - c;

		if( ++salt >= 20857 ) salt = 0;

		/* the new key is made dependant on the last cleartext char */
		/* this is *data on encryption and c on decryption.	    */
		if( encrypt ) key = key + key + *data + salt;
		else key = key + key + (c&0xff) + salt;
		
		*data++ = c;
	}
#endif
}

extern Key NewKey()
{
	int x;
	/* make a key out of the date plus the current processor uptime */
	/* plus the current stack pointer, that lot should be quite	*/
	/* difficult to guess.						*/

	return ((word)&x)^GetDate()^_cputime();
}

/*--------------------------------------------------------
-- AdjustBuffers					--
-- 							--
-- Data buffer support. 				--
-- Trims the given buffer list so that the given bounds	--
-- lie within the buffers.				--
-- This routine assumes that the list has been allocated--
-- here.						--
--							--
--------------------------------------------------------*/

extern bool AdjustBuffers(List *list, word start, word end, word bufsize)
{
	Buffer *buf = (Buffer *)(list->Head);

	word last;			/* pos of 1st byte beyond end of list */

	/* adjust to block boundaries */
	start -= start % bufsize;	/* pos of 1st byte in list	*/
	end += bufsize - 1;
	end -= end % bufsize;		/* desired value for last	*/
	
	/* first trim off any unwanted buffers from the front of the list */
	while(buf->Node.Next != NULL && buf->Pos < start )
	{
		Buffer *next = (Buffer *)(buf->Node.Next);
		Remove(&buf->Node);
		Free(buf);
		buf = next;
	}

	if( list->Tail->Prev == NULL ) last = start;
	else last = ((Buffer *)(list->Tail))->Pos + bufsize;

	/* now add any new buffers required at the end of the list */
	while( last < end )
	{
		Buffer *b = (Buffer *)ServMalloc(sizeof(Buffer)+bufsize);
		if( b == NULL ) return false;

		b->Pos = last;
		b->Size = 0;
		b->Max = bufsize;
		b->Data = (byte *)(b+1);
		AddTail(list,&b->Node);

		last += bufsize;
	}

	/* alternatively remove any unwanted buffers	*/
	while( end < last )
	{
		Buffer *b = (Buffer *)RemTail(list);
		Free(b);
		
		if( list->Tail->Prev == NULL ) last = start;
		else last = ((Buffer *)list->Tail)->Pos + bufsize;	
	}
	
	return true;
}

/*--------------------------------------------------------
-- DoRead						--
--							--
-- Read protocol support.				--
-- The client is assumed to have detemined that the read--
-- can be satisfied and updated the fields of the	--
-- control vector accordingly.				--
--							--
--------------------------------------------------------*/

extern word DoRead(MCB *m, Buffer *(*GetBuffer)(), void *info)
{
	ReadWrite *rw	= (ReadWrite *)m->Control;
	Port reply 	= m->MsgHdr.Reply;	
	word pos	= rw->Pos;
	word size	= rw->Size;
	word offset	= 0;
	word sent	= 0;
	word seq	= 0;
	word e		= Err_Null;
	Buffer *buf	= GetBuffer(pos,info);
		
	InitMCB(m,MsgHdr_Flags_preserve,reply,NullPort,ReadRc_More);
	
	/* if there are no buffers, EOF */
	if( buf == NULL )
	{
		m->MsgHdr.Flags = 0;
		m->MsgHdr.FnRc = ReadRc_EOF;
		PutMsg(m);
		return true;
	}
	
	/* start read at correct place in buffer */
	offset = pos - buf->Pos;
	
	while( sent < size )
	{
		word dsize = buf->Size - offset;

		if( dsize + sent >= size )
		{
			dsize = size - sent;
			m->MsgHdr.FnRc = ReadRc_EOD|seq;
			m->MsgHdr.Flags = 0;
		}

		m->Data = &buf->Data[offset];
		m->MsgHdr.DataSize = (unsigned short)dsize;

		if( (e = PutMsg(m)) < Err_Null ) return false;
		
		offset += dsize;
		sent += dsize;
		seq += ReadRc_SeqInc;
		m->MsgHdr.FnRc = ReadRc_More|seq;
		
		if( offset == buf->Size ) 
		{
			buf = GetBuffer(pos+sent,info);
			offset = 0;
		}

	}

	rw->Size = sent;
	
	return true;
}

/*--------------------------------------------------------
-- DoWrite						--
--							--
-- Write protocol support.				--
-- The client is assumed to have detemined that the write--
-- can be satisfied and updated the fields of the	--
-- control vector accordingly.				--
--							--
--------------------------------------------------------*/

extern word DoWrite(MCB *m, Buffer *(*GetBuffer)(), void *info)
{
	ReadWrite *rw	= (ReadWrite *)m->Control;
	Port request	= m->MsgHdr.Dest;
	Port reply	= m->MsgHdr.Reply;
	word msgdata	= m->MsgHdr.DataSize;

	word pos	= rw->Pos;
	word size	= rw->Size;
	word offset	= 0;
	word got	= 0;
	word e		= Err_Null;
	Buffer *buf	= GetBuffer(pos,info);

	if( buf == NULL ){ e = EC_Error|EG_NoMemory; goto done; }

	offset = pos - buf->Pos;

	/* If the user has supplied any data in the request, deal with	*/
	/* it here and adjust things for later.				*/
	if( msgdata > 0 )
	{
		while( got < msgdata )
		{
			word dsize = buf->Max - offset;
			if( dsize > size - got ) dsize = size - got;
			memcpy(&buf->Data[offset],&m->Data[got],(int)dsize);
			if( offset+dsize > buf->Size ) buf->Size = offset+dsize;
			got += dsize;
			buf = GetBuffer(pos+got,info);
			offset = 0;
		}

		pos += got;
		offset = pos - buf->Pos;
		if( got == size ) goto done;
	}

	InitMCB(m,MsgHdr_Flags_preserve,reply,NullPort,WriteRc_Sizes);

	MarshalWord(m,buf->Max - offset);
	MarshalWord(m,buf->Max);

	if( (e = PutMsg(m)) < Err_Null) return false;

	m->MsgHdr.Dest = request;
	m->Timeout = WriteTimeout;

	while( got < size )
	{
		word dsize;
		
		if( buf == NULL ){ e = EC_Error|EG_NoMemory; goto done; }

		m->Data = &buf->Data[offset];

		if( (e = GetMsg(m)) < Err_Null )
		{
			e &= ~SS_Mask;
			e |= SS_SysLib;
			break;
		}
		
		dsize = m->MsgHdr.DataSize;

		if( offset+dsize > buf->Size ) buf->Size = offset+dsize;
		offset += dsize;
		got += dsize;

		if( offset == buf->Max ) 
		{
			buf = GetBuffer(pos+got,info);
			offset = 0;
		}

	}

done:

	/* all done, tell user how much data we got */

	InitMCB(m,0,reply,NullPort,e<0?e:WriteRc_Done);

	MarshalWord(m,got);

	PutMsg(m);

	rw->Size = got;
	
	return true;
}

/*--------------------------------------------------------
-- GetReadBuffer					--
-- GetWriteBuffer					--
--							--
-- When a buffer list is being maintained using 	--
-- AdjustBuffers, These can be used as the GetBuffer	--
-- arguments to DoRead and DoWrite. The info argument	--
-- should be a pointer to the buffer List head.		--
--							--
--------------------------------------------------------*/

static bool PickReadBuf(Buffer *b, word pos)
{
	return (b->Pos <= pos) && (pos < b->Pos+b->Size);
}

extern Buffer *GetReadBuffer(word pos, List *list)
{
	return (Buffer *)SearchList(list,PickReadBuf,pos);
}

static bool PickWriteBuf(Buffer *b, word pos)
{
	return (b->Pos <= pos) && (pos < b->Pos+b->Max);
}

extern Buffer *GetWriteBuffer(word pos, List *list)
{
	return (Buffer *)SearchList(list,PickWriteBuf,pos);
}

#ifdef SYSDEB
static void _ServDebug(char *str, ... )
{
	int x[10];
	int i;
	va_list a;
		
	va_start(a,str);
		
	for( i = 0 ; i < 10 ; i++ ) x[i] = va_arg(a,int);
		
	IOdebug(str,x[0],x[1],x[2],x[3],x[4],x[5],x[6],x[7],x[8],x[9]);
}

#endif

/* -- End of servlib.c */
