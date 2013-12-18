/*------------------------------------------------------------------------
--                                                                      --
--                     H E L I O S   N U C L E U S                      --
--                     ---------------------------                      --
--                                                                      --
--             Copyright (C) 1990, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- helios.c								--
--                                                                      --
--	Native /helios server						--
--                                                                      --
--	Author:  PAB 1/5/90						--
--                                                                      --
-- The native /helios directory only allows subdirectories and links	--
-- to be made in its directory structure. It is simply to allow A ROM	--
-- based Helios system to support several filing systems that may wish	--
-- to supply different parts of the system, for instance a plug-in ROM	--
-- card may supply a new resident library, this has to be loaded from   --
-- /helios/lib, /helios/lib now being a subdirectory in /helios that	--
-- just holds links to the real file systems that provide the relevant	--
-- files.								--
--                                                                      --
------------------------------------------------------------------------*/
/* RCSId: $Id: helios.c,v 1.4 1991/05/06 17:05:50 paul Exp $ */
 
#include <helios.h>	/* standard header */

#define __in_helios 1	/* flag that we are in this module */
#define in_kernel 1	/* Cheat to get access to the "Config" information */

/*--------------------------------------------------------
-- 		     Include Files			--
--------------------------------------------------------*/

#include <string.h>
#include <codes.h>
#include <syslib.h>
#include <servlib.h>
#include <task.h>
#include <config.h>
#include <root.h>
#ifdef __ARM
# include <abcARM/ABClib.h>
# include <limits.h>
#endif

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

static int basename(char *base, char *str);
static void StdLink(char *src, char *dst);

static void do_open(ServInfo *);
static void do_create(ServInfo *);
static void do_delete(ServInfo *);
static void do_serverinfo(ServInfo *);

static DispatchInfo HServerInfo = {
	&Root,
	NullPort,
	SS_HServer,
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
		{ DoRevoke,	2000 },		/* FG_Revoke	 	*/
		{ InvalidFn,	2000 },		/* Reserved 		*/
		{ InvalidFn,	2000 }		/* Reserved 	 	*/
	}
};


/*--------------------------------------------------------
-- main							--
--							--
-- Entry point of /helios server.			--
--							--
--------------------------------------------------------*/

int main()
{
#ifndef DEMANDLOADED
	Object *nte;
#endif
	char mcname[100];

#ifdef __ARM
	char basefsdir[PATH_MAX];
#endif

#ifdef STANDALONE
	Environ env;
	GetEnv(MyTask->Port,&env); /* posix exec send's env ! */
	/* if executed by procman alone, we shouldn't read this */
#endif

	MachineName(mcname);
	
	HServerInfo.ParentName = mcname;
	
	InitNode( (ObjNode *)&Root, "helios", Type_Directory, 0, DefRootMatrix );
	InitList( &Root.Entries );
	Root.Nentries = 0;

#ifdef DEMANDLOADED
	HServerInfo.ReqPort = MyTask->Port;
#else
	Info.Port = HServerInfo.ReqPort = NewPort();
#endif
	/* .. of root is a link to our machine root	*/
	{
		Object *o;
		LinkNode *Parent;

		o = Locate(NULL,mcname);

		Parent = (LinkNode *)Malloc((word)sizeof(LinkNode) + (word)strlen(mcname));	
		InitNode( &Parent->ObjNode, "..", Type_Link, 0, DefLinkMatrix );
		Parent->Cap = o->Access;
		strcpy(Parent->Link,mcname);
		Root.Parent = (DirNode *)Parent;

#ifndef DEMANDLOADED
		/* demand loaded servers already have name entry */
		nte = Create(o,"helios",Type_Name,sizeof(NameInfo),
				(byte *)&Info);
#endif				
		Close(o);
	}

#ifdef __ARM
	if ((GetRoot()->Flags) & Root_Flags_ROM)
	{
		if (ResetKeyState() & (1 << ShellBootKey))
		{
			/* If user holds down "shell boot key" during system */
			/* reset, use special default helios shell startup */
			strcpy(basefsdir, "/rom/sys");
		}
		else
		{
			/* get EEPROM configuration user defined boot device */
			const char *basefs = ServerIndexToName(ReadEEPROM(EEPROM_ServerID));

			strcpy(basefsdir, basefs);
		}
	}
	else
		strcpy(basefsdir, "/files");
#else
		strcpy(basefsdir, "/fs");
#endif

	strcat(basefsdir, "/helios");

	/* Create set of standard default links in /helios */
	/* StdLink will create any directories that the dst. name is prefixed by */
	
#if 0
	IOdebug("basefsdir = %s",basefsdir);
#endif

	StdLink(basefsdir, "lib") ;
	StdLink(basefsdir, "etc") ;
	StdLink(basefsdir, "bin") ;
	StdLink(basefsdir, "local");
	StdLink(basefsdir, "system");
	StdLink(basefsdir, "users");
	StdLink(basefsdir, "include");
#ifdef __ARM
	StdLink("/ram/sys", "tmp"); /* special - force to ramfs */
#else
	StdLink("basefsdir", "tmp"); /* special - force to ramfs */
#endif

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

	Dispatch(&HServerInfo);

#ifndef DEMANDLOADED
	Delete(nte,NULL);
#endif
}

/*--------------------------------------------------------
-- NewDir						--
--							--
-- Add a new subdirectory 				--
--							--
--------------------------------------------------------*/

DirNode *NewDir(DirNode *dir,string name, word flags, Matrix matrix)
{
	DirNode *d = New(DirNode);

	if( d == NULL ) return NULL;

	InitNode((ObjNode *)d,name,Type_Directory,flags,matrix);

	InitList(&d->Entries);
	d->Nentries = 0;
	d->Parent = dir;

	Insert(dir, (ObjNode *)d, TRUE );

	return d;
}

/*--------------------------------------------------------
--  StdLink						--
--							--
--  StdLink() creates a link from /helios/'postfix'	--
--  to 'basedir'/'postfix'. It will also create any	--
--  directories that the 'postfix' name is prefixed by	--
--							--
--------------------------------------------------------*/
static void StdLink(char *basedir, char *postfix)
{
	LinkNode *l;
	char dst[PATH_MAX];
	Object *dobj;

	strcpy(dst,basedir);
	strcat(dst,"/");
	strcat(dst,postfix);

	if ((l = (LinkNode *)Malloc((word)sizeof(LinkNode) + (word)strlen(dst))) != NULL)
	{
		int idx = 0;
		DirNode *Dir = &Root;
		DirNode *NewDir = &Root;
		char base[NameMax];

		while ((idx = basename(base, postfix)) != 0)
		{
			postfix += idx;
			if ((NewDir = (DirNode *) Lookup(Dir, base, TRUE)) == NULL)
			{
				DirNode *d = New(DirNode);

				if( d == NULL ) return;

				InitNode((ObjNode *)d,base,Type_Directory,0,DefDirMatrix);
				InitList(&d->Entries);
				d->Nentries = 0;
				d->Parent = Dir;

				Insert(Dir, (ObjNode *)d, TRUE );
				NewDir = d;
			}
			Dir = NewDir;
		}

		InitNode(&l->ObjNode,base,Type_Link,0,DefLinkMatrix);
		if ((dobj = Locate(NULL, dst)) != NULL)
			l->Cap = dobj->Access; /* copy protection info */
		else
		{	/* server not currently loaded - fudge it */
			l->Cap.Access = AccMask_Full; /* default total access */
			/* force eight validation bytes to 0xff... */
			*(word *)(&l->Cap.Valid[0]) = -1;
			*(word *)(&l->Cap.Valid[4]) = -1;
		}
		strcpy(l->Link,dst);
		Insert(NewDir,&l->ObjNode,TRUE);
	}
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
	char *pathname = servinfo->Pathname;
	

	d = (DirNode *)GetTargetDir(servinfo);

	if( d == NULL )
	{
		ErrorMsg(m,EO_Directory);
		return;
	}

	r = New(MsgBuf);

	if( r == NULL )
	{
		ErrorMsg(m,EC_Error+EG_NoMemory);
		return;
	}

	f = (File *)GetTargetObj(servinfo);

	if( f == NULL && (req->Arg.Mode & O_Create) ) 
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

		{
			char *name;
			IOCCreate *req = (IOCCreate *)(m->Control);
	
			name = objname(pathname);	

			if( req->Type == Type_Directory ) 
				f = (File *)NewDir(d, name, 0, DefDirMatrix );
			else
			{
				/* don't allow file creation */
				ErrorMsg(m,EC_Error+EG_Create+EO_File);
				Free(r);
				return;
			}
		}
	}
			

	if( f == NULL )
	{
		ErrorMsg(m,EO_File);
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

	/* Should be no files, so dont allow any stream operations */
	if( f->ObjNode.Type != Type_Directory )
		ErrorMsg(m,EC_Error+EG_Open+EO_File);

	DirServer(servinfo,m,reqport);
	FreePort(reqport);
	return;
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

	r = New(MsgBuf);

	if( r == NULL )
	{
		ErrorMsg(m,EC_Error+EG_NoMemory);
		return;
	}

	{
		char *name;
		IOCCreate *req = (IOCCreate *)(m->Control);

		name = objname(pathname);	

		if( req->Type == Type_Directory ) 
			f = (File *)NewDir(d, name, 0, DefDirMatrix );
		else
		{
			/* don't allow file creation */
			ErrorMsg(m,EC_Error+EG_Create+EO_File);
			Free(r);
			return;
		}
	}

	if( f == NULL )
	{
		ErrorMsg(m,EC_Error+EG_NoMemory+EO_Directory);
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
	
	Unlink(&f->ObjNode,FALSE);
	Free(f);

	/* Gsp reply for delete is just a simple message header */
	ErrorMsg(m,Err_Null);
}

static void do_serverinfo(ServInfo *servinfo)
{
/*	char *pathname = servinfo->Pathname;*/
	NullFn(servinfo);
}


/*--------------------------------------------------------
-- basename()						--
--							--
-- This is used for splitting up file names.		--
-- Copy chars out of str into pfix until either a null 	--
-- is found or '/' is encountered. Returns pos in str 	--
-- of char after ch, or 0 if null is encountered	--
-- 							--
--------------------------------------------------------*/

static int basename(char *base, char *str)
{
	int ptr = 0;
	while( str[ptr] != '/' )
	{
		if( str[ptr] == '\0' )
		{ base[ptr] = '\0'; return 0; }
		else if( ptr < NameMax-1 ) base[ptr] = str[ptr];
		ptr++;
	}
	base[ptr] = '\0';
	return ptr+1;
}


#ifdef __TRAN
void _stack_error(void)
{
	IOdebug("/helios server stack overflow!!");
}
#elif defined (__ARM)
static void __stack_overflow(void)
{
	IOdebug("/helios server stack overflow!");
}
static void __stack_overflow_1(void)
{
	IOdebug("/helios server stack overflow1!");
}
#endif


/* End of helios.c */
