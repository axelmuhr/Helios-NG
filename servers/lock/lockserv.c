/*------------------------------------------------------------------------
--                                                                      --
--                     H E L I O S			                --
--                     -----------                                      --
--                                                                      --
--             Copyright (C) 1990, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- lockserv.c								--
--                                                                      --
--	very simple locking server					--
--                                                                      --
--	Author:  BLV 21.2.90						--
--                                                                      --
------------------------------------------------------------------------*/

/*--------------------------------------------------------
-- 		     Include Files			--
--------------------------------------------------------*/
#include <helios.h>	/* standard header */
#include <string.h>
#include <codes.h>
#include <syslib.h>
#include <servlib.h>
#include <gsp.h>
#include <root.h>
#include <link.h>
#include <message.h>
#include <protect.h>
#ifdef __TRAN
#include <event.h>
#else
#include <intr.h>
#endif
#include <nonansi.h>
#include <stdlib.h>

	/* There is a shortage of subsystem codes...	*/
#ifndef SS_LockDevice
#define SS_LockDevice SS_NullDevice
#endif

/*--------------------------------------------------------
--		Private Data Definitions 		--
--------------------------------------------------------*/


static void do_open(ServInfo *);
static void do_create(ServInfo *);
static void do_delete(ServInfo *);

static DirNode LockRoot;
static DispatchInfo LockInfo = {
	(DirNode *) &LockRoot,
	NullPort,
	SS_LockDevice,
	NULL,
	{ NULL,	0 },
	{
		{ do_open,	5000 },
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
		{ InvalidFn,	2000 }
	}
};


int main(void)
{ Port		server_port;
  NameInfo	nameinfo;
  Object	*this_processor;
  Object	*nametable_entry;
  char		mcname[IOCDataMax];

#ifdef DEBUG
	/* If the program is to be started from the shell	*/
	/* but still linked with s0.o then it must accept 	*/
	/* an environment.					*/
  { Environ	env;
    GetEnv(MyTask->Port,&env);
  }
#endif

	/* If embedded in the nucleus, acknowledge start-up	*/
#ifdef IN_NUCLEUS
  { MCB		m;
    InitMCB(&m, 0, MyTask->Parent, NullPort, 0x456);
    (void) PutMsg(&m);
  }
#endif

  server_port = NewPort();
  if (server_port == NullPort)
   { IOdebug("Lock server : failed to allocate port");
     Exit(EXIT_FAILURE << 8);
   }
  LockInfo.ReqPort = server_port;

  MachineName(mcname);
  LockInfo.ParentName	= mcname;

  if ((this_processor = Locate(Null(Object), mcname)) == Null(Object))
   { IOdebug("Lock server : failed to locate processor %s", mcname);
     FreePort(server_port);
     Exit(EXIT_FAILURE << 8);
   }

	/* Initialise the directory tree.			*/
  InitNode( (ObjNode *) &LockRoot, "lock", Type_Directory, 0, DefDirMatrix );
  InitList( &LockRoot.Entries);
  LockRoot.Nentries = 0;

	/* .. of the server is a link to the processor level	*/
  { LinkNode *Parent;

    Parent = (LinkNode *)Malloc(sizeof(LinkNode) + (word)strlen(mcname));	
    if (Parent == NULL)
     { IOdebug("Lock Server: out of memory during initialisation");
       Exit(EXIT_FAILURE << 8);
     }
    InitNode( &Parent->ObjNode, "..", Type_Link, 0, DefLinkMatrix );
    Parent->Cap = this_processor->Access;
    strcpy(Parent->Link, this_processor->Name);
    LockRoot.Parent = (DirNode *)Parent;
  }

	/* Register the server with the system.		*/
  nameinfo.Port     = server_port;
  nameinfo.Flags    = Flags_StripName;
  nameinfo.Matrix   = DefDirMatrix;
  nameinfo.LoadData = NULL;

  nametable_entry = Create(this_processor, "lock", Type_Name,
	 sizeof(NameInfo), (byte *)&nameinfo);
  Close(this_processor);

  if (nametable_entry == Null(Object))
   { IOdebug("Lock server : failed to enter name in name table");
     FreePort(server_port);
     Exit(EXIT_FAILURE << 8);
   }

	/* Call the dispatcher.						*/
  Dispatch(&LockInfo);

	/* Tidy up.							*/
	/* Dispatch will return iff the server_port has been freed	*/  
  Delete(nametable_entry, Null(char));

  Exit(EXIT_SUCCESS);
}


/*--------------------------------------------------------
-- Action Procedures					--
--							--
--------------------------------------------------------*/

static void do_open(ServInfo *servinfo)
{ MCB            *m          = servinfo->m;
  MsgBuf         *r;
  char           *pathname   = servinfo->Pathname;
  Port           reqport;
  DirNode        *d;
  ObjNode        *f;
    
  d = (DirNode *) GetTargetDir(servinfo);
  if (d == Null(DirNode))
   { ErrorMsg(m, EO_Directory); return; }
  
  f = GetTargetObj(servinfo);
  if (f == Null(ObjNode))
   { ErrorMsg(m, EO_File); return; }

  if (f->Type != Type_Directory)
   { ErrorMsg(m, EO_Directory + EG_WrongFn + EO_Object); return; }
   
  r = New(MsgBuf);
  if( r == Null(MsgBuf) )
   { ErrorMsg(m,EC_Error+EG_NoMemory); return; }

  FormOpenReply(r, m, f, Flags_Server | Flags_Closeable, pathname);
  reqport		= NewPort();  
  r->mcb.MsgHdr.Reply	= reqport;
  PutMsg(&r->mcb);
  Free(r);
   
  DirServer(servinfo, m, reqport);
  FreePort(reqport);
  return;
}

static void do_create(ServInfo *servinfo)
{ MCB 		*m		= servinfo->m;
  IOCCreate	*req		= (IOCCreate *)(m->Control);
  char		*pathname	= servinfo->Pathname;
  MsgBuf 	*r;
  DirNode 	*d;
  ObjNode	*f;

  d = GetTargetDir(servinfo);
  if (d == Null(DirNode))
   { ErrorMsg(m,EO_Directory); return; }

  f = GetTargetObj(servinfo);
  if (f != Null(ObjNode))
   { ErrorMsg(m,EC_Error + EG_InUse + EO_File); return; }

  m->MsgHdr.FnRc	= SS_LockDevice;

  if (req->Type == Type_Stream)
   { f = New(ObjNode);
     if (f == Null(ObjNode))
      { ErrorMsg(m, EC_Error + EG_NoMemory + EO_Object); return; }
     InitNode(f, objname(pathname), Type_Stream, 0, DefFileMatrix);
     Insert(d, f, TRUE);
   }
  else
   { DirNode *subdir = New(DirNode);
     if (subdir == Null(DirNode))
      { ErrorMsg(m, EC_Error + EG_NoMemory + EO_Directory); return; }
     InitNode((ObjNode *) subdir, objname(pathname), Type_Directory,
		0, DefDirMatrix);
     InitList(&subdir->Entries);
     Insert(d, (ObjNode *) subdir, TRUE);
     f = (ObjNode *) subdir;
   }

  r = New(MsgBuf);
  if (r == Null(MsgBuf))
   { Unlink(f, TRUE);
     Free(f);
     ErrorMsg(m, EC_Error + EG_NoMemory + EO_Message);
     return;
   }

  req->Common.Access.Access = AccMask_Full;
  FormOpenReply(r, m, f, 0, pathname);
  PutMsg(&r->mcb);
  Free(r);
}

static void do_delete(ServInfo *servinfo)
{ MCB		*m	= servinfo->m;
  ObjNode	*f;

  f = GetTarget(servinfo);
  if (f == Null(ObjNode))
   { ErrorMsg(m, EO_File); return; }

  if (f->Type == Type_Directory)
   { DirNode	*d	= (DirNode *) f;
     if (d->Nentries != 0)
      { ErrorMsg(m, EC_Error + EG_InUse + EO_Directory); return; }
   }

	/* Allow the server to be terminated.			*/
  if (f == (ObjNode *) &LockRoot)
   { ErrorMsg(m, Err_Null);	/* send back a success code	*/
     AbortPort(LockInfo.ReqPort, EC_Fatal + EG_Exception + EE_Abort);
   }
  else
   { Unlink(f, FALSE);
     servinfo->TargetLocked	= FALSE;
     Free(f);
     ErrorMsg(m, Err_Null);
   }
}

