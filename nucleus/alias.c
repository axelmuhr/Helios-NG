/*------------------------------------------------------------------------
--                                                                      --
--                     	H E L I O S   S E R V E R                       --
--                      -------------------------                       --
--                                                                      --
--             Copyright (C) 1987, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- alias.c								--
--                                                                      --
--	Alias server. This server sets itself up as a free-standing	--
--	link to some other directory. Requests directed to this server	--
--	are passed on in the usual way to the linked object.		--
--                                                                      --
--	Author:  NHG 16/8/87						--
--                                                                      --
------------------------------------------------------------------------*/
/* SccsId: %W% %G% Copyright (C) 1987, Perihelion Software Ltd.*/
/* $Id: alias.c,v 1.4 1992/10/05 09:04:50 nickc Exp $ */

#include <helios.h>	/* standard header */

#include <syslib.h>
#include <servlib.h>
#include <string.h>
#include <root.h>
#include <message.h>

LinkNode	*MyNode;

NameInfo Info =
{
	NullPort,
	Flags_StripName,
	DefNameMatrix,
	(word *)NULL
};

static void do_delete(ServInfo *servinfo);
static void PassOn(ServInfo *servinfo);

static DispatchInfo DInfo = {
	NULL,
	NullPort,
	0,
	NULL,
	{ NULL,	0 },
	{
		{ PassOn,	1000 },
		{ PassOn,	1000 },
		{ PassOn,	1000 },
		{ DoObjInfo,	1000 },
		{ PassOn,	1000 },
		{ do_delete,	1000 },
		{ PassOn,	1000 },
		{ PassOn,	1000 },
		{ PassOn,	1000 },
		{ PassOn,	1000 },
		{ DoRefine,	1000 },
		{ PassOn,	1000 },
		{ DoRevoke,	1000 },
		{ PassOn,	1000 },
		{ PassOn,	1000 }
	}
};


int main(void)
{
	Object *nte;
	char **argv;
	int argc;
	Port myport = NewPort();
	Environ env;
	char *myname;
	char *dirname;
	Object *dir;	

	GetEnv(MyTask->Port,&env);

	argv = env.Argv;
	for( argc = 0; argv[argc] ; argc++ );
	
	if( argc != 3 )
	{
		char *usage = "usage: alias name dir\n";
		Write(env.Strv[2],usage,strlen(usage),-1);
		Exit(0x100);
	}
	
	myname = argv[1];
	dirname = argv[2];

	Info.Port = DInfo.ReqPort = myport;

	dir = Locate(NULL,dirname);

	if( dir == NULL )
	{
		char *msg = "cannot locate directory\n";
		Write(env.Strv[2],msg,strlen(msg),-1);
		Exit(0x100);
	}

	{
		Object *o;
		char mcname[100];

		MachineName(mcname);
		o = Locate(NULL,mcname);

		nte = Create(o,myname,Type_Name,sizeof(NameInfo),
			(byte *)&Info);
		Close(o);
	}

	if( nte == NULL )
	{
		char *msg = "cannot create name table entry\n";
		Write(env.Strv[2],msg,strlen(msg),-1);
		Exit(0x100);
	}

	MyNode = (LinkNode *)Malloc( sizeof (LinkNode) + (word)strlen( dir->Name ) );

	if( MyNode == NULL )
	{
		char *msg = "cannot allocate node structure\n";
		Write(env.Strv[2],msg,strlen(msg),-1);
		Exit(0x100);
	}

	DInfo.Root = (DirNode *)MyNode;
	InitNode(&MyNode->ObjNode,myname, Type_Link, 0, DefLinkMatrix );
	MyNode->Cap = dir->Access;
	strcpy(MyNode->Link,dir->Name);
	MyNode->ObjNode.Parent = NULL;

	Dispatch(&DInfo);

	Delete(nte,NULL);

	Exit(0);

	return 0;
}

static void do_delete(ServInfo *servinfo)
{
	MCB *m = servinfo->m;
	IOCCommon *req = (IOCCommon *)(m->Control);
	LinkNode *l;
	word e;

	l = (LinkNode *)GetTarget(servinfo);

	if( l == NULL ) { ErrorMsg(m,EO_Link); return; }

	if( l != MyNode ) { ErrorMsg(m,EG_Invalid|EO_Link); return; }
	
	unless( CheckMask(req->Access.Access,AccMask_D) )
	{ ErrorMsg(m,EC_Error|EG_Protected|EO_Link); return; }
	
	ErrorMsg(m,0);
			
	e = FreePort(DInfo.ReqPort);
}

static void PassOn(ServInfo *servinfo)
{
	MCB *m = servinfo->m;
#if 0		
	char *data = m->Data;
	word *control = m->Control;

IOdebug("pass on %s %s",control[0]==-1?NULL:&data[control[0]],
			control[1]==-1?NULL:&data[control[1]]);
#endif
	GetTarget(servinfo);
	ErrorMsg(m,EC_Fatal|EG_WrongFn|EO_Link);
}	

/* -- End of alias.c */
