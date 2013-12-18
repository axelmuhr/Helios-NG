/*------------------------------------------------------------------------
--                                                                      --
--                     H E L I O S   N U C L E U S                      --
--                     ---------------------------                      --
--                                                                      --
--             Copyright (C) 1989, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- rom.c								--
--                                                                      --
--	ROM disc handler						--
--                                                                      --
--	Author:  NHG 12/09/89						--
--                                                                      --
------------------------------------------------------------------------*/
/* SccsId:	 %W%	%G% Copyright (C) 1989, Perihelion Software Ltd.*/


#include <helios.h>	/* standard header */

#define __in_rom 1	/* flag that we are in this module */

/*--------------------------------------------------------
-- 		     Include Files			--
--------------------------------------------------------*/

#include <string.h>
#include <codes.h>
#include <syslib.h>
#include <servlib.h>
#include <task.h>
#include <root.h>

/*--------------------------------------------------------
--		Private Data Definitions 		--
--------------------------------------------------------*/

typedef struct FileData {
	word		Size;
	char		Name[NameMax];
} FileData;

typedef struct File {
	ObjNode		ObjNode;	/* node for directory struct	*/
	void		*Data;		/* pointer to file data		*/
	Buffer		Buf;		/* buffer for DoRead		*/
} File;
#define Upb		ObjNode.Size	/* use ObjNode size field	*/
#define Users		ObjNode.Account  /* number of opens		*/

DirNode		Root;			/* root of directory system	*/

#define DefRomMatrix	0x01010101	/* r:r:r:r		*/

void FileRead(MCB *m, File *f);
void FileSeek(MCB *m, File *f);
static void loadFiles(void);

NameInfo Info =
{
	NullPort,
	Flags_StripName,
	DefNameMatrix,
	(word *)NULL
};


static void do_open(ServInfo *);
static void do_serverinfo(ServInfo *);

static DispatchInfo RomInfo = {
	&Root,
	NullPort,
	SS_RamDisk,
	NULL,
	{ NULL, 0 },
	{
		{ do_open,	2000 },		/* FG_Open		*/
		{ InvalidFn,	2000 },		/* FG_Create		*/
		{ DoLocate,	2000 },		/* FG_Locate		*/
		{ DoObjInfo,	2000 },		/* FG_ObjectInfo	*/
		{ do_serverinfo,2000 },		/* FG_ServerInfo	*/
		{ InvalidFn,	2000 },		/* FG_Delete		*/
		{ InvalidFn,	2000 },		/* FG_Rename		*/
		{ InvalidFn,	2000 },		/* FG_Link		*/
		{ InvalidFn,	2000 },		/* FG_Protect		*/
		{ InvalidFn,	2000 },		/* FG_SetDate		*/
		{ InvalidFn,	2000 },		/* FG_Refine		*/
		{ NullFn,	2000 }		/* FG_CloseObj		*/
	}
};


/*--------------------------------------------------------
-- main							--
--							--
-- Entry point of Rom handler.				--
--							--
--------------------------------------------------------*/

int main()
{
	Object *nte;
	char mcname[100];
	bool in_nucleus = (word)main < (word)GetRoot(); /* better test needed */

	if( !in_nucleus )
	{
		Environ env;
		GetEnv(MyTask->Port,&env);
	}
		
	MachineName(mcname);
	
	RomInfo.ParentName = mcname;
	
	InitNode( (ObjNode *)&Root, "rom", Type_Directory, 0, DefRootMatrix );
	InitList( &Root.Entries );
	Root.Nentries = 0;

	Info.Port = RomInfo.ReqPort = NewPort();

	/* .. of root is a link to our machine root	*/
	{
		Object *o;
		LinkNode *Parent;

		o = Locate(NULL,mcname);

		Parent = (LinkNode *)Malloc(sizeof(LinkNode) + strlen(mcname));	
		InitNode( &Parent->ObjNode, "..", Type_Link, 0, DefLinkMatrix );
		Parent->Cap = o->Access;
		strcpy(Parent->Link,mcname);
		Root.Parent = (DirNode *)Parent;

		nte = Create(o,"rom",Type_Name,sizeof(NameInfo),
				(byte *)&Info);
				
		Close(o);

	}

	/* load the file data from the end of this module */
	
	loadFiles();

	/* reply to procman that we have started */
	if( in_nucleus )
	{
		MCB m;
		word e;
		InitMCB(&m,0,MyTask->Parent,NullPort,0x456);
		e = PutMsg(&m);
	}

	Dispatch(&RomInfo);
	
	Delete(nte,NULL);
}

static void loadFiles(void)
{
	Object *ro = Locate(NULL,"/loader/Rom");

	/* Kludge for testing */
	if( ro == NULL ) ro = Locate(NULL,"/loader/rom.i");

	if( ro != NULL ) 
	{
		Stream *rs = Open(ro,NULL,O_Execute);

		if( rs != NULL )
		{
			FileData *f;
			Module *m = (Module *)rs->Server;

			until( strcmp(m->Name,"Files") == 0 )
				m = (Module *)((word)m + m->Size);
		
			f = (FileData *)(m+1);
			
			until( f->Size == 0 )
			{
				File *file = New(File);
				if( file )
				{
					InitNode(&file->ObjNode,f->Name,Type_File,0,DefRomMatrix);
					file->Upb = f->Size;
					file->Data = (void *)(f+1);
					Insert(&Root,&file->ObjNode,FALSE);
				}
				f = (FileData *)((word)f + f->Size + sizeof(FileData));
			}
			Close(rs);
		}
		Close(ro);	
	}
	/* else find files elsewhere ?? */
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

	d = (DirNode *)GetTargetDir(servinfo);

	if( d == NULL )
	{
		ErrorMsg(m,Err_Null);
		return;
	}

	f = (File *)GetTargetObj(servinfo);

	r = New(MsgBuf);

	if( r == NULL )
	{
		ErrorMsg(m,EC_Error+EG_NoMemory);
		return;
	}

	if( f == NULL )
	{
		ErrorMsg(m,Err_Null);
		Free(r);
		return;
	}

	unless( CheckMask(req->Common.Access.Access,req->Arg.Mode&Flags_Mode) ) 
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

		switch( m->MsgHdr.FnRc & FG_Mask )
		{
		case FG_Read:
			FileRead(m,f);
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

		case FG_Select:
			/* a file is always ready for I/O so reply now */
			ErrorMsg(m,e&Flags_Mode);
			break;
			
		default:
			ErrorMsg(m,EC_Error+EG_FnCode+EO_File);
			break;
		}
		Signal(&f->ObjNode.Lock);
	}

	f->Users--;
	FreePort(reqport);
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

static Buffer *GetRomBuffer(word pos, File *f)
{
	Buffer *b = &f->Buf;
	
	b->Pos = 0;
	b->Size = b->Max = f->Upb;
	b->Data = f->Data;
	
	return b;
	pos=pos;
}

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

	DoRead(m,GetRomBuffer,f);

	f->ObjNode.Dates.Access = GetDate();
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

void _stack_error(void)
{
	IOdebug("Rom Disk stack overflow!!");
}

/* -- End of rom.c */
