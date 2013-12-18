/**
*
* Title:  Helios Debugger - Debug Server.
*
* Author: Andy England
*
* Date:   February 1989
*
*         (c) Copyright 1988 - 1992, Perihelion Software Ltd.
*
*         All Rights Reserved.
*
**/

#ifdef __TRAN
static char *rcsid = "$Header: /hsrc/cmds/debugger/RCS/server.c,v 1.7 1992/11/04 14:45:42 nickc Exp $";
#endif

#include "tla.h"

#define SS_DebugServer 0x13000000

typedef struct
{
  ObjNode objnode;
  DEBUG *debug;
} File;

PRIVATE NameInfo debugnameinfo =
{
  NullPort,
  Flags_StripName,
  DefDirMatrix,
  NULL
};

PRIVATE DirNode root;
PRIVATE Object *debugger;

PRIVATE void do_open(ServInfo *);
PRIVATE void do_create(ServInfo *);
PRIVATE void do_delete(ServInfo *);
/** #ifdef NEWCODE   not yet defined
PRIVATE void do_serverinfo(ServInfo *);
#endif **/

PRIVATE DispatchInfo debuginfo =
{
  &root,
  NullPort,
  SS_DebugServer,
  NULL,
  { NULL, 0 },
  {
    { do_open,      20000},
    { do_create,     2000},
    { DoLocate,      2000},
    { DoObjInfo,     2000},
    { NullFn,        2000}, /* do_serverinfo() */
    { do_delete,     2000},
    { DoRename,      2000},
    { DoLink,        2000},
    { DoProtect,     2000},
    { DoSetDate,     2000},
    { DoRefine,      2000},
    { NullFn,        2000}
  }
};

PUBLIC void tidyup(void)
{
  Delete(debugger, NULL);
  Close(debugger);
}

#ifdef MEM_REPORT
/*
-- crf : 16/08/91 - Bart's patch routines
-- (very useful for tracking down memory leaks)
*/
/**
*** This unbelievably gruesome code zaps the module table entries for
*** Malloc and Free, installing my own routines which call the system ones
*** and produce some debugging.
**/

static	WordFnPtr	real_Malloc;
static	WordFnPtr	real_Free;

static word	my_Malloc(int x)
{ word result = ((real_Malloc)(x));

  IOdebug( "TLA: Malloc(%d) : %x", x, result);
  return(result);
}

static word	my_Free(int x)
{ word	result = ((real_Free)(x));

  IOdebug( "TLA: Free(%x)", x);
  return(result);
}

void PatchMalloc(void)
{ int	*table = (int *) &MyTask;

  IOdebug( "TLA: Installing own versions of Malloc and Free");
/*
  printf("table[26] is %x\n", table[26]);
  printf("table[27] is %x\n", table[27]);
  printf("&Malloc is %x\n", &Malloc);
  printf("&Free is %x\n", &Free);
*/
  real_Malloc = (WordFnPtr) table[26];
  real_Free   = (WordFnPtr) table[27];
  table[26]   = (int) &my_Malloc;
  table[27]   = (int) &my_Free;
}
#endif


PUBLIC int main(int argc, char *argv[])
{
  char mcname[100];

#ifdef MEM_REPORT
/*
-- crf : 16/08/91 - tracking down memory leaks ...
*/
  PatchMalloc() ;
  IOdebug ( "TLA: starting ...") ;
  IOdebug ( "TLA: Bytes free : %d   Heap size : %d", Malloc(-1), Malloc(-3));
#endif

/*
-- crf : 01/10/91 - Bug 706
-- Ensure that the debugger does not already exist.
*/	
    if (Locate (NULL, "/debug") != NULL)
      {	
	exit(EEXIST);
      }

  /*
   * Find out the name of our processor.
   */
  
  MachineName(mcname);
  
  debuginfo.ParentName = mcname;
  
  /*
   * Initialise the root of the directory structure.
   */
  
  initdebug(argc, argv);

#ifdef	MYDEBUG
  InitNode((ObjNode *)&root, "mydebug", Type_Directory, 0, DefDirMatrix);
#else
#if 0
  InitNode((ObjNode *)&root, "debug", Type_Directory, 0, DefDirMatrix);
#else
/*
-- crf: 26/08/92 - Bug 928 (cannot run clients on remote processors if 
-- processor protection is set (nsrc))
*/
  InitNode((ObjNode *)&root, "debug", Type_Directory, 0, 0x23134BC7 /* DefDirMatrix */);
#endif
#endif

  InitList(&root.Entries);
  
  root.Nentries = 0;
  
  /*
   * 
   */
  
/* CR: MyTask->Port might be used for signal handling...
  debugnameinfo.Port = debuginfo.ReqPort = MyTask->Port;
*/
  
  debugnameinfo.Port = debuginfo.ReqPort = NewPort();
  
  /*
   * Make the parent of root the machine.
   */
  
  {
    Object *machine;
    LinkNode *parent;

    machine = Locate(NULL, mcname);
    parent = (LinkNode *)Malloc(sizeof(LinkNode) + (word)strlen(mcname));
    InitNode(&parent->ObjNode, "..", Type_Link, 0, DefDirMatrix);
    parent->Cap = machine->Access;
    strcpy(parent->Link, mcname);
    root.Parent = (DirNode *)parent;
#ifdef MYDEBUG
    debugger = Create(machine, "mydebug", Type_Name, sizeof(NameInfo), (BYTE *)&debugnameinfo);
#else
    debugger = Create(machine, "debug", Type_Name, sizeof(NameInfo), (BYTE *)&debugnameinfo);
#endif

    Close(machine);

    if(debugger == NULL)
      exit(1);
  }
  
  Dispatch(&debuginfo);

  tidyup();
  return 0;
}

PRIVATE File *newfile(DirNode *dir, char *name, word flags, Matrix matrix)
{
  File *file;
  static int debugid = 1;

  if ((file = New(File)) == NULL) return NULL;
  /* ACE: Temporary hack */
  strcat(name, ".");
  addint(name, debugid++);

  InitNode(&file->objnode, name, Type_File, (int)flags, matrix);
  file->objnode.Size = 0;
  file->objnode.Account = 0;
  file->debug = newdebug(name);
  Insert(dir, &file->objnode, TRUE);
  return file;
}

PRIVATE DirNode *newdir(DirNode *dir, char *name, word flags, Matrix matrix)
{
  DirNode *dirnode;

  if ((dirnode = New(DirNode)) == NULL) return NULL;
  InitNode((ObjNode *)dirnode, name, Type_Directory, (int)flags, matrix);
  InitList(&dirnode->Entries);
  dirnode->Nentries = 0;
  dirnode->Parent = dir;
  Insert(dir, (ObjNode *)dirnode, TRUE);
  return dirnode;
}

PRIVATE File *createnode(MCB *mcb, DirNode *dir, char *name)
{
  IOCCreate *req = (IOCCreate *)mcb->Control;

  if (req->Type == Type_Directory)
    return (File *)newdir(dir, name, 0, DefDirMatrix);
  return newfile(dir, name, 0, DefFileMatrix);
}

/*
* Action procedues.
*/
PRIVATE void do_create(ServInfo *servinfo)
{
  MCB *mcb = servinfo->m;
  MsgBuf *msgbuf;
  DirNode *dir;
  File *file;
  IOCCreate *req = (IOCCreate *)(mcb->Control);
  char *pathname = servinfo->Pathname;

  if ((dir = (DirNode *)GetTargetDir(servinfo)) == NULL)
  {
    ErrorMsg(mcb, EO_Directory);
    return;
  }
  mcb->MsgHdr.FnRc = SS_DebugServer;
  /*
  * Check file does not already exist.
  */
  unless ((file = (File *)GetTargetObj(servinfo)) == NULL)
  {
    ErrorMsg(mcb, EC_Error+EG_Create+EO_File);
    return;
  }
  /*
  * Check that we can write to the directory.
  */
  unless (CheckMask(req->Common.Access.Access, AccMask_W))
  {
    ErrorMsg(mcb, EC_Error+EG_Protected+EO_Directory);
    return;
  }
  if ((msgbuf = New(MsgBuf)) == NULL)
  {
    ErrorMsg(mcb, EC_Error+EG_NoMemory+EO_Message);
    return;
  }
  if ((file = createnode(mcb, dir, objname(pathname))) == NULL)
  {
    ErrorMsg(mcb, EC_Error+EG_NoMemory+EO_File);
    Free(msgbuf);
    return;
  }
  FormOpenReply(msgbuf, mcb, &file->objnode, 0, pathname);
  PutMsg(&msgbuf->mcb);
  Free(msgbuf);
}

PRIVATE void do_open(ServInfo *servinfo)
{
  MCB *mcb = servinfo->m;
  MsgBuf *msgbuf;
  DirNode *dir;
  File *file;
  IOCMsg2 *req = (IOCMsg2 *)(mcb->Control);
  Port reply = mcb->MsgHdr.Reply;
  Port reqport;
#ifdef NEWCODE
  byte *data = mcb->Data;
#endif
  char *pathname = servinfo->Pathname;

  debugf("do_open(%s)", pathname);

  putmem();

  if ((dir = (DirNode *)GetTargetDir(servinfo)) == NULL)
  {
    ErrorMsg(mcb, EO_Directory);
    return;
  }
  
  if ((msgbuf = New(MsgBuf)) == NULL)
  {
    ErrorMsg(mcb, EC_Error+EG_NoMemory);
    return;
  }
  
  if ((file = (File *)GetTargetObj(servinfo)) == NULL AND
      (req->Arg.Mode & O_Create))
  {
    mcb->MsgHdr.FnRc = SS_DebugServer;
    unless (CheckMask(req->Common.Access.Access, AccMask_W))
    {
      ErrorMsg(mcb, EC_Error+EG_Protected+EO_Directory);
      Free(msgbuf);
      return;
    }
    file = createnode(mcb, dir, objname(pathname));
  }
  
  if (file == NULL)
  {
    ErrorMsg(mcb, EO_File);
    Free(msgbuf);
    return;
  }
  
  unless (CheckMask(req->Common.Access.Access, (AccMask)(req->Arg.Mode & Flags_Mode)))
  {
    ErrorMsg(mcb, EC_Error+EG_Protected+EO_Directory);
    Free(msgbuf);
    return;
  }
  
  FormOpenReply(msgbuf, mcb, &file->objnode, Flags_Server|Flags_Closeable, pathname);
  /* ACE ? */
  msgbuf->mcb.MsgHdr.Flags |= MsgHdr_Flags_preserve;
  msgbuf->mcb.MsgHdr.Reply = reqport = NewPort();
  PutMsg(&msgbuf->mcb);
  Free(msgbuf);

  if (file->objnode.Type == Type_Directory)
  {
    DirServer(servinfo, mcb, reqport);
    FreePort(reqport);
    return;
  }
  
  file->objnode.Account++;
  
  UnLockTarget(servinfo);
  
  /* ACE */

#ifdef MEM_REPORT
/*
-- crf : 16/08/91 - tracking down memory leaks ...
*/
  IOdebug ( "TLA: Debug session starting ...") ;
  IOdebug ( "TLA: Bytes free : %d   Heap size : %d", Malloc(-1), Malloc(-3));
#endif

  startdebug(file->debug, reqport, reply);

#ifdef NEWCODE
  forever
  {
    word err;

    
    mcb->MsgHdr.Dest = reqport;
    mcb->Timeout     = StreamTimeout;
    mcb->Data        = data;

    err = GetMsg(mcb);

    if (err == EK_Timeout)
      break;
    
    if (err < Err_Null) continue;

    Wait(&file->objnode.Lock);

    switch (mcb->MsgHdr.FnRc & FG_Mask)
    {
      case FG_Read:
      break;

      case FG_Write:
      break;

      case FG_Close:
      if (mcb->MsgHdr.Reply != NullPort) ErrorMsg(mcb, Err_Null);
      FreePort(reqport);
      file->objnode.Account--;
      Signal(&file->objnode.Lock);
      break;

      case FG_GetSize:
      case FG_Seek:
      case FG_SetSize:
      break;

      default:
      ErrorMsg(mcb, EC_Error+EG_FnCode+EO_File);
      break;
    }
    
    Signal(&file->objnode.Lock);
  }
#endif
  if (--file->objnode.Account == 0)
  {
    remdebug(file->debug);
    Unlink(&file->objnode, FALSE);
    Free(file);
  }
/*#ifdef NEWCODE    CR: this is necessary */
  FreePort(reqport);
  FreePort(reply);   	/* CR: hope I am right here */
/*#endif*/

  putmem();

#ifdef MEM_REPORT
/*
-- crf : 16/08/91 - tracking down memory leaks ...
*/
  IOdebug ( "TLA: Debug session finished") ;
  IOdebug ( "TLA: Bytes free : %d   Heap size : %d", Malloc(-1), Malloc(-3));
#endif

}

PRIVATE void do_delete(ServInfo *servinfo)
{
  MCB *mcb = servinfo->m;
  File *file;
  IOCCommon *req = (IOCCommon *)(mcb->Control);

  if ((file = (File *)GetTargetObj(servinfo)) == NULL)
  {
    ErrorMsg(mcb, EO_File);
    return;
  }
  unless (CheckMask(req->Access.Access, AccMask_D))
  {
    ErrorMsg(mcb, EC_Error+EG_Protected+EO_File);
    return;
  }
  if (file->objnode.Type == Type_Directory AND ((DirNode *)file)->Nentries != 0)
  {
    ErrorMsg(mcb, EC_Error+EG_Delete+EO_Directory);
    return;
  }
  else if (file->objnode.Type == Type_File)
  {
    unless (file->objnode.Account == 0)
    {
      ErrorMsg(mcb, EC_Error+EG_InUse+EO_File);
      return;
    }
    file->objnode.Size = 0;
    remdebug(file->debug);
  }
  Unlink(&file->objnode, FALSE);
  Free(file);
  ErrorMsg(mcb, Err_Null);
}
