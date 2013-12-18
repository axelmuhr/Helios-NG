/*------------------------------------------------------------------------
--                                                                      --
--                     H E L I O S   N U C L E U S                      --
--                     ---------------------------                      --
--                                                                      --
--             Copyright (C) 1987, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- ram.c								--
--                                                                      --
--	RAM disc handler						--
--                                                                      --
--	Author:  NHG 22/11/87						--
--                                                                      --
------------------------------------------------------------------------*/
/* SccsId: %W% %G% Copyright (C) 1987, Perihelion Software Ltd.*/
/* $Id: ram.c,v 1.8 1992/10/05 09:04:42 nickc Exp $ */

#include <helios.h>	/* standard header */

#define __in_ram 1	/* flag that we are in this module */

/*--------------------------------------------------------
-- 		     Include Files			--
--------------------------------------------------------*/

#include <string.h>
#include <codes.h>
#include <syslib.h>
#include <servlib.h>
#include <task.h>

/*--------------------------------------------------------
--		Private Data Definitions 		--
--------------------------------------------------------*/

typedef struct File {
	ObjNode		ObjNode;	/* node for directory struct	*/
} File;
#define Upb		ObjNode.Size	/* use ObjNode size field	*/
#define Buffers		ObjNode.Contents /* use ObjNode contents field	*/
#define Users		ObjNode.Account  /* number of opens		*/

#define FileMax		1024		/* bytes in each block		*/

DirNode		Root;			/* root of directory system	*/

#ifndef DEMANDLOADED
NameInfo Info =
{	NullPort,
	Flags_StripName,
	DefNameMatrix,
	(word *)NULL
};
#endif

static File *CreateNode(MCB *m, DirNode *d, string pathname);
void FileRead(MCB *m, File *f);
void FileWrite(MCB *m, File *f);
void FileSeek(MCB *m, File *f);

static void do_open(ServInfo *);
static void do_create(ServInfo *);
static void do_delete(ServInfo *);
static void do_serverinfo(ServInfo *);

static DispatchInfo RamInfo = {
	&Root,
	NullPort,
	SS_RamDisk,
	NULL,
	{ NULL, 0 },
	{
		{ do_open,	2000 },		/* FG_Open		*/
		{ do_create,	2000 },		/* FG_Create		*/
		{ DoLocate,	2000 },		/* FG_Locate		*/
		{ DoObjInfo,	2000 },		/* FG_ObjectInfo	*/
		{ do_serverinfo,2000 },		/* FG_ServerInfo	*/
		{ do_delete,	2000 },		/* FG_Delete		*/
		{ DoRename,	2000 },		/* FG_Rename		*/
		{ DoLink,	2000 },		/* FG_Link		*/
		{ DoProtect,	2000 },		/* FG_Protect		*/
		{ DoSetDate,	2000 },		/* FG_SetDate		*/
		{ DoRefine,	2000 },		/* FG_Refine		*/
		{ NullFn,	2000 },		/* FG_CloseObj		*/
		{ DoRevoke,	2000 },		/* FG_Revoke		*/
		{ InvalidFn,	2000 },		/* Reserved 		*/
		{ InvalidFn,	2000 }		/* Reserved		*/
	}
};


/*--------------------------------------------------------
-- main							--
--							--
-- Entry point of Ram handler.				--
--							--
--------------------------------------------------------*/

int main()
{
#ifndef DEMANDLOADED
	Object *nte;
#endif
	char mcname[100];

#ifdef STANDALONE
	Environ env;
	GetEnv(MyTask->Port,&env); /* posix exec send's env ! */
	/* if executed by procman alone, we shouldn't read this */
#endif

	MachineName(mcname);
	
	RamInfo.ParentName = mcname;
	
	InitNode( (ObjNode *)&Root, "ram", Type_Directory, 0, DefRootMatrix );
	InitList( &Root.Entries );
	Root.Nentries = 0;

#ifdef DEMANDLOADED
	RamInfo.ReqPort = MyTask->Port;
#else
	Info.Port = RamInfo.ReqPort = NewPort();
#endif
	/* .. of root is a link to our machine root	*/
	{
		Object *o;
		LinkNode *Parent;

		o = Locate(NULL,mcname);

		Parent = (LinkNode *)Malloc(sizeof(LinkNode) + (word)strlen(mcname));	
		InitNode( &Parent->ObjNode, "..", Type_Link, 0, DefLinkMatrix );
		Parent->Cap = o->Access;
		strcpy(Parent->Link,mcname);
		Root.Parent = (DirNode *)Parent;

#ifndef DEMANDLOADED
		/* demand loaded servers already have name entry */
		nte = Create(o,"ram",Type_Name,sizeof(NameInfo),
				(byte *)&Info);
#endif				
		Close(o);
	}

#ifdef INSYSTEMIMAGE
	/* reply to procman that we have started */
	/* if we are part of system image 0x456 is expect to be returned */
	{
		MCB m;
		word e;
		InitMCB(&m,0,MyTask->Parent,NullPort,0x456);
		e = PutMsg(&m);
	}
#endif

	Dispatch(&RamInfo);

#ifndef DEMANDLOADED
	Delete(nte,NULL);
#endif
}

/*--------------------------------------------------------
-- NewFile						--
--							--
-- Add a new file to the directory			--
--							--
--------------------------------------------------------*/

File *NewFile(DirNode *dir,string name, word flags, Matrix matrix)
{
	File *f = New(File);

	if( f == NULL ) return NULL;

	InitNode( &f->ObjNode, name, Type_File, (int)flags, matrix );
	
	InitList(&f->Buffers);
	f->Upb = 0;
	f->Users = 0;
	
	Insert( dir, &f->ObjNode, TRUE );

	return f;
}

DirNode *NewDir(DirNode *dir,string name, word flags, Matrix matrix)
{
	DirNode *d = New(DirNode);

	if( d == NULL ) return NULL;

	InitNode( (ObjNode *)d, name, Type_Directory, (int)flags, matrix);

	InitList(&d->Entries);
	d->Nentries = 0;
	d->Parent = dir;

	Insert(dir, (ObjNode *)d, TRUE );

	return d;
}

static File *CreateNode(MCB *m, DirNode *d, string pathname)
{
	File *f;
	char *name;
	IOCCreate *req = (IOCCreate *)(m->Control);

	name = objname(pathname);	

	if( req->Type == Type_Directory ) 
		f = (File *)NewDir(d, name, 0, DefDirMatrix );
	else	f = NewFile(d, name, Flags_Selectable, DefFileMatrix );

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
	File *f;
	IOCMsg2 *req = (IOCMsg2 *)(m->Control);
	Port reqport;
	byte *data = m->Data;
	char *pathname = servinfo->Pathname;
	word mode = req->Arg.Mode;
	
	d = (DirNode *)GetTargetDir(servinfo);

	if( d == NULL )
	{
		ErrorMsg(m,Err_Null);
		return;
	}

	r = New(MsgBuf);

	if( r == NULL )
	{
		ErrorMsg(m,EC_Error+EG_NoMemory);
		return;
	}

	f = (File *)GetTargetObj(servinfo);

	if( f == NULL && (mode & O_Create) ) 
	{
		m->MsgHdr.FnRc &= SS_Mask; /* PAB: clear any error set by GetContextObj */

		/* if file does not exist, see whether we are allowed to */
		/* create a file here.					 */
		unless( CheckMask(req->Common.Access.Access,AccMask_W) ) 
		{
			ErrorMsg(m,EC_Error+EG_Protected+EO_Directory);
			Free(r);
			return;
		}
		f = CreateNode(m, d, pathname);
	}

	if( f == NULL )
	{
		ErrorMsg(m,Err_Null);
		Free(r);
		return;
	}

	unless( CheckMask( req->Common.Access.Access, (int)mode & Flags_Mode ) ) 
	{
		ErrorMsg(m,EC_Error+EG_Protected+EO_File);
		Free(r);
		return;
	}

	FormOpenReply(r,m,&f->ObjNode,Flags_Server|Flags_Closeable, pathname);

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

	if( mode & O_Truncate ) 
	{
		f->Upb = 0;
		AdjustBuffers(&f->Buffers,0,0,FileMax);
	}

	f->Users++;
	
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

		m->MsgHdr.FnRc = SS_RamDisk;
		
		switch( e & FG_Mask )
		{
		case FG_Read:
			unless( mode & O_ReadOnly ) goto badmode;
			FileRead(m,f);
			break;

		case FG_Write:
			unless( mode & O_WriteOnly ) goto badmode;
			FileWrite(m,f);
			break;
		
		case FG_Close:
			if( m->MsgHdr.Reply != NullPort ) ErrorMsg(m,Err_Null);
			FreePort(reqport);
			f->Users--;
			Signal(&f->ObjNode.Lock);
			return;

		case FG_GetSize:
			InitMCB(m,0,m->MsgHdr.Reply,NullPort,Err_Null);
			MarshalWord(m,f->Upb);
			PutMsg(m);
			break;

		case FG_Seek:
			FileSeek(m,f);
			break;

		case FG_SetSize:
			unless( mode & O_WriteOnly ) goto badmode;
			else
			{
				word newsize = m->Control[0];
				if( newsize > f->Upb )
					ErrorMsg(m,EC_Error+EG_Parameter+1);
				else {
					if( newsize < f->Upb ) f->Upb = newsize;
					if(!AdjustBuffers(&f->Buffers,0,f->Upb,FileMax) )
					{
						ErrorMsg(m,EC_Error|EG_NoMemory);
						break;
					}
					InitMCB(m,0,m->MsgHdr.Reply,NullPort,Err_Null);
					MarshalWord(m,f->Upb);
					PutMsg(m);
			}
			break;
		}

		case FG_Select:
			/* a file is always ready for I/O so reply now */
			ErrorMsg(m,e&Flags_Mode);
			break;
			
		default:
			ErrorMsg(m,EC_Error+EG_FnCode+EO_File);
			break;
		badmode:
			ErrorMsg(m,EC_Error+EG_WrongFn+EO_File);
			break;
		}
		Signal(&f->ObjNode.Lock);
	}

	f->Users--;
	FreePort(reqport);
}

static void do_create(ServInfo *servinfo)
{
	MCB *m = servinfo->m;
	MsgBuf *r;
	DirNode *d;
	File *f;
	IOCCreate *req = (IOCCreate *)(m->Control);
	char *pathname = servinfo->Pathname;

	d = GetTargetDir(servinfo);

	if( d == NULL )
	{
		ErrorMsg(m,EO_Directory);
		return;
	}

	f = (File *)GetTargetObj(servinfo);
	
	if( f != NULL )
	{
		ErrorMsg(m,EC_Error+EG_Create+EO_File);
		return;
	}

	unless( CheckMask(req->Common.Access.Access,AccMask_W) ) 
	{
		ErrorMsg(m,EC_Error+EG_Protected+EO_Directory);
		return;
	}

	/* Now give creator (only) full access to new object	*/
	req->Common.Access.Access = AccMask_Full;

	r = New(MsgBuf);

	if( r == NULL )
	{
		ErrorMsg(m,EC_Error+EG_NoMemory);
		return;
	}

	f = CreateNode( m, d, pathname );

	if( f == NULL )
	{
		ErrorMsg(m,0);
		Free(r);
		return;
	}

	FormOpenReply(r,m,&f->ObjNode, 0, pathname);

	PutMsg(&r->mcb);

	Free(r);
}

static void do_delete(ServInfo *servinfo)
{
	MCB *m = servinfo->m;
	File *f;
	IOCCommon *req = (IOCCommon *)(m->Control);

	f = (File *)GetTarget(servinfo);

	if( f == NULL )
	{
		ErrorMsg(m,EO_File);
		return;
	}
	
	unless( CheckMask(req->Access.Access,AccMask_D) ) 
	{
		ErrorMsg(m,EC_Error+EG_Protected+EO_File);
		return;
	}

	if( f->ObjNode.Type == Type_Directory &&
		((DirNode *)f)->Nentries != 0 )
	{
		ErrorMsg(m,EC_Error+EG_Delete+EO_Directory);
		return;
	}
	else if ( f->ObjNode.Type == Type_File )
	{
		if( f->Users != 0 )
		{
			ErrorMsg(m,EC_Error+EG_InUse+EO_File);
			return;
		}
		f->Upb = 0;
		AdjustBuffers(&f->Buffers,0,0,FileMax);
	}
	
	
	Unlink(&f->ObjNode,FALSE);
	servinfo->TargetLocked = FALSE;	/* to stop ServLib worker indirection through Free'd target on unlock */
	Free(f);

	ErrorMsg(m,Err_Null);
}

static void do_serverinfo(ServInfo *servinfo)
{
	NullFn(servinfo);
}

/*--------------------------------------------------------
-- FileRead						--
-- 							--
-- Read data from the File and return it to the client.	--
--							--
--------------------------------------------------------*/

void FileRead(MCB *m, File *f)
{
	ReadWrite *rw = (ReadWrite *)m->Control;

	word pos = rw->Pos;
	word size = rw->Size;

	if( pos < 0 )
	{
		ErrorMsg(m,EC_Error|EG_Parameter|1);
		return;
	}

	if( size < 0 )
	{
		ErrorMsg(m,EC_Error|EG_Parameter|2);
		return;
	}

	if( pos == f->Upb )
	{
		m->MsgHdr.FnRc = ReadRc_EOF;
		ErrorMsg(m,0);
		return;
	}


	if( pos + size > f->Upb ) size = rw->Size = f->Upb - pos;
#if 0
	/* if we have no data to send, wait for the timeout to expire	*/
	/* and return a timeout message. This should cause syslib to 	*/
	/* retry.							*/
	if( size == 0 )
	{
		Delay(timeout);
		m->MsgHdr.FnRc = EC_Recover+EG_Timeout+EO_File;
		ErrorMsg(m,0);
		return;
	}
#endif
	DoRead(m,GetReadBuffer,&f->Buffers);

	f->ObjNode.Dates.Access = GetDate();
}

/*--------------------------------------------------------
-- FileWrite						--
-- 							--
-- Write data to the File.				--
-- To preserve retryability we must allow the client	--
-- to over-write existing data.				--
--							--
--------------------------------------------------------*/

void FileWrite(MCB *m, File *f)
{
	ReadWrite *rw = (ReadWrite *)m->Control;
	word pos = rw->Pos;
	word size = rw->Size;

	if( pos > f->Upb )
	{
		ErrorMsg(m,EC_Error|EG_Parameter|1);
		return;
	}

	f->Upb = (f->Upb>(pos+size))?f->Upb:pos+size;

	/* re-arrange the buffers in the File */
	if( !AdjustBuffers(&f->Buffers,0,f->Upb,FileMax) )
	{
		f->Upb = ((Buffer *)f->Buffers.Tail)->Pos+FileMax;
		m->MsgHdr.FnRc = 0;
		ErrorMsg(m,EC_Error|SS_RamDisk|EG_NoMemory|EO_File);
		return;
	}	

	DoWrite(m,GetWriteBuffer,&f->Buffers);

	f->ObjNode.Dates.Modified = GetDate();
	f->ObjNode.Dates.Access   = GetDate();

}

/*--------------------------------------------------------
-- FileSeek						--
-- 							--
-- Seek to a new file position. This is merely a hint	--
-- which is useful to 'real' file servers but is of no	--
-- use to us. Perform the necessary calculation in any	--
-- case.						--
--							--
--------------------------------------------------------*/

void FileSeek(MCB *m, File *f)
{
	SeekRequest *req = (SeekRequest *)m->Control;
	word curpos = req->CurPos;
	word mode = req->Mode;
	word newpos = req->NewPos;

	switch( mode )
	{
	case S_Beginning:	break;
	case S_Relative:	newpos += curpos; break;
	case S_End:		newpos += f->Upb; break;
	}

	InitMCB(m,0,m->MsgHdr.Reply,NullPort,Err_Null);

	MarshalWord(m,newpos);

	PutMsg(m);
}

#ifdef __XPUTER
void _stack_error(void)
{
	IOdebug("Ram Disk stack overflow!!");
}
#elif defined(__ARM)
static void __stack_overflow(void)
{
	IOdebug("Ram Disk stack overflow!");
}
static void __stack_overflow_1(void)
{
	IOdebug("Ram Disk stack overflow1!");
}
#endif

/* -- End of ram.c */

