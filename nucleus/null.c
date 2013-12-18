/*------------------------------------------------------------------------
--                                                                      --
--                     H E L I O S   N U C L E U S                      --
--                     ---------------------------                      --
--                                                                      --
--             Copyright (C) 1987, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- null.c								--
--                                                                      --
--	NULL device handler						--
--                                                                      --
--	Author:  NHG 30/07/88						--
--                                                                      --
------------------------------------------------------------------------*/
/* SccsId: %W% %G% Copyright (C) 1987, Perihelion Software Ltd.*/
/* $Id: null.c,v 1.4 1992/10/05 09:04:28 nickc Exp $ */

#include <helios.h>	/* standard header */

#define __in_null 1	/* flag that we are in this module */

/*--------------------------------------------------------
-- 		     Include Files			--
--------------------------------------------------------*/

#include <string.h>
#include <codes.h>
#include <syslib.h>
#include <servlib.h>

/*--------------------------------------------------------
--		Private Data Definitions 		--
--------------------------------------------------------*/

ObjNode		Root;

#ifndef DEMANDLOADED
NameInfo Info =
{	NullPort,
	Flags_StripName,
	DefNameMatrix,
	(word *)NULL
};
#endif

static void do_open(ServInfo *);

static DispatchInfo NullInfo = {
	(DirNode *)&Root,
	NullPort,
	SS_NullDevice,
	NULL,
	{ NULL,	0 },
	{
		{ do_open,	2000 },
		{ InvalidFn,	2000 },
		{ DoLocate,	2000 },
		{ DoObjInfo,	2000 },
		{ InvalidFn,	2000 },
		{ InvalidFn,	2000 },
		{ InvalidFn,	2000 },
		{ InvalidFn,	2000 },
		{ InvalidFn,	2000 },
		{ InvalidFn,	2000 },
		{ InvalidFn,	2000 },
		{ InvalidFn,	2000 },
		{ DoRevoke,	2000 },
		{ InvalidFn,	2000 },
		{ InvalidFn,	2000 }
	}
};

/*--------------------------------------------------------
-- main							--
--							--
-- Entry point of NULL handler.				--
--							--
--------------------------------------------------------*/

int main()
{
#ifndef DEMANDLOADED
	Object *nte;
#endif

	InitNode( &Root, "null", Type_File, 0, 0x03030303 );

#ifdef DEMANDLOADED
	/* if we are demand loaded our server port is passed in task struct */
	NullInfo.ReqPort = MyTask->Port;
#else
	Info.Port = NullInfo.ReqPort = NewPort();

	{
		Object *o;
		char mcname[100];

		MachineName(mcname);
		o = Locate(NULL,mcname);

		/* demand loaded servers already have name entry */
		nte = Create(o,"null",Type_Name,sizeof(NameInfo),
				(byte *)&Info);
		Close(o);
	}
#endif

#ifdef INSYSTEMIMAGE
	/* reply to procman that we have started */
	/* as we are part of system image 0x456 is expect to be returned */
	{
		MCB m;
		word e;
		InitMCB(&m,0,MyTask->Parent,NullPort,0x456);
		e = PutMsg(&m);
	}
#endif

	Dispatch(&NullInfo);

#ifndef DEMANDLOADED
	Delete(nte,NULL);
#endif
}

static void do_open(ServInfo *servinfo)
{
	MCB *m = servinfo->m;
	MsgBuf *r;
	IOCMsg2 *req = (IOCMsg2 *)(m->Control);
	char *pathname = servinfo->Pathname;

/*IOdebug("Null Open %s %x",m->Data,req->Arg.Mode);*/

	r = New(MsgBuf);

	if( r == Null(MsgBuf) )
	{
		ErrorMsg(m,EC_Error+EG_NoMemory);
		return;
	}

	unless( CheckMask( req->Common.Access.Access, (int)(req->Arg.Mode) & Flags_Mode ) ) 
	{
		ErrorMsg(m,EC_Error+EG_Protected+EO_File);
		Free(r);
		return;
	}

	FormOpenReply(r,m,&Root,0,pathname);

	MarshalWord(&r->mcb,NullPort);
	
	PutMsg(&r->mcb);
	Free(r);
}

/* -- End of null.c */

