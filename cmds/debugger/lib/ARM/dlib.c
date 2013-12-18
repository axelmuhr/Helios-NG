/**
*
* Title:  Debug Library.
*
* Author: Andy England
*
* Date:   March 1989
*
*         (c) Copyright 1989, Perihelion Software Ltd.
*
*         All Rights Reserved.
*
* Modified for C40 by N Clifton, October 1992
*
*	Copyright (c) 1992 Perihelios Software Ltd.
*	All Rights Reserved.
**/

/*
static char *rcsid = "$Header: /m/giga/HeliosRoot/Helios/cmds/debugger/lib/ARM/RCS/dlib.c,v 1.3 1992/09/29 13:59:37 nickc Exp $";
*/

#include <stdlib.h>
#include <helios.h>
#include <syslib.h>
#include <servlib.h>
#undef Malloc
#include <codes.h>
#include <module.h>
#include <posix.h>
#include <nonansi.h>
#include <string.h>
#include "dlib.h"
#include "dmsg.h"

/**
*
* Static data.
*
**/

PRIVATE THREAD thread;
PRIVATE DEBUG *debug;

/**
*
* Private function prototypes.
*
**/

/* -- crf : 07/08/91 - not used -
PRIVATE void dummy (WATCHPOINT *, int );
*/

PRIVATE BOOL initdebug(void);
PRIVATE void remdebug(void);
PRIVATE void debugworker(void);
PRIVATE word getdebug(Port, DBG *);
PRIVATE word putdebug(Port, DBG *);
PRIVATE void initthread(THREAD *);
PRIVATE void remthread(THREAD *);
PRIVATE void suspend(THREAD *);
PRIVATE void resume(THREAD *);
PRIVATE void patchentry(THREAD *, void (*)(Proc *, byte *));
PRIVATE void patchcommand(THREAD *, void (*)(word, Module *));
PRIVATE void patchreturn(THREAD *, word (*)(Proc *, word));
PRIVATE void ignore_entry(Proc *, byte *);
PRIVATE void ignore_command(word, Module *);
PRIVATE word ignore_return(Proc *, word);
PRIVATE void framestop_command(word, Module *);
PRIVATE void *DBGNewProcess(word, VoidFnPtr, word);
PRIVATE word DBGFork(word, VoidFnPtr, word, ...);
#ifdef V1_1
PRIVATE void addbreak(word, word, word, BOOL);
PRIVATE void remtempbreak(BREAKPOINT *);
#else
PRIVATE void addbreak(word, word, word);
#endif
PRIVATE BREAKPOINT *findbreak(word, word);
PRIVATE void rembreak(BREAKPOINT *);
PRIVATE void addwatch(void *, word);
PRIVATE WATCHPOINT *findwatch(void *, word);
PRIVATE void remwatch(WATCHPOINT *);
PRIVATE void checkwatch(WATCHPOINT *, word );
PRIVATE int checkwatchret(WATCHPOINT *, word );

/* 
-- crf : 05/08/91 - explicitly remove watchpoints on exiting from routines 
*/
PRIVATE void delwatchpoints (void) ;

PRIVATE void profilethread(THREAD *, BOOL);
PRIVATE void stopthread(THREAD *);
PRIVATE void unstopthread(THREAD *);
PRIVATE void tracethread(THREAD *, word);
PRIVATE void gotoframethread(THREAD *, int);
PRIVATE void timeoutthread(THREAD *);
PRIVATE byte *dataloc(word, word);
PRIVATE byte *stackloc(THREAD *, word, word);
PRIVATE void peekmem(Port, byte *, word);
PRIVATE void pokemem(byte *, word, byte *);
PRIVATE void peekdata(Port, word, word, word);
PRIVATE void pokedata(word, word, word, byte *);
PRIVATE void peekstack(Port, THREAD *, word, word, word);
PRIVATE void pokestack(THREAD *, word, word, word, byte *);
PRIVATE void locatedata(Port, word, word);
PRIVATE void locatestack(Port, THREAD *, word, word);
PRIVATE void call(Port, VoidFnPtr, word, word *, word);
PRIVATE void where(Port, THREAD *, word);

PRIVATE Proc *findproc(word, word);
PRIVATE FUNC *addfunc(Proc *);
PRIVATE void remfunc(FUNC *);
PRIVATE FUNC *findfunc(Proc *);
PRIVATE void entered(Proc *);
PRIVATE void returned(Proc *);

PRIVATE word DBGGetMsg(MCB *);
PRIVATE word DBGPutMsg(MCB *);

extern word (*patchfork(word (*)(word, VoidFnPtr, word, ...)))(word, VoidFnPtr, word, ...);
extern word (*patchputmsg(word (*)(MCB *)))(MCB *);
extern word (*patchgetmsg(word (*)(MCB *)))(MCB *);
extern void _DBGProcExit(void);
extern word DoCall(VoidFnPtr, word, word *);

#ifdef MEMCHECK
PRIVATE void memcheck(void)
{
  int i;
  unsigned char *lowmem = 0;
  unsigned char savemem[256];

  for (i = 0; i < 256; i++) savemem[i] = lowmem[i];

  forever
  {
    Delay(2000000);
    for (i = 0; i < 256; i++)
    {
      unless (savemem[i] == lowmem[i])
      {
      	IOdebug("DLIB: Memory corruption @ %x", i);
      	savemem[i] = lowmem[i];
      }
    }
  }
}
#endif

/*
-- crf : 11/08/91 - No longer needed
*/
#ifdef OLDCODE
/**
*
* code = locatecode();
*
* Return loaded object.
*
**/
PRIVATE Object *locatecode(void)
{
  Object *code = NULL;
  char name[256];

  MachineName(name);
  strcat(name, "/loader/");
  strcat(name, MyTask->Program->Module.Name);
  if ((code = Locate(NULL, name)) == NULL)
  {
    /*
    * We aren't where we thought we were.
    */
    Object *loader;

    MachineName(name);
    strcat(name, "/loader");
    unless ((loader = Locate(NULL, name)) == NULL)
    {
      Stream *dir;

      unless ((dir = Open(loader, "", O_ReadOnly)) == NULL)
      {
        DirEntry direntry;

        until (Read(dir, (byte *)&direntry, sizeof(DirEntry), -1) <= 0)
        {
          Stream *stream;

          unless ((stream = Open(loader, direntry.Name, O_Execute)) == NULL)
          {
            if ((Program *)stream->Server == MyTask->Program)
            {
              Close(stream);
              code = Locate(loader, direntry.Name);
              break;
            }
            Close(stream);
          }
        }
        Close(dir);
      }
      Close(loader);
    }
  }
  return code;
}
#endif

/*
-- crf : 11/08/91 - No longer needed
*/
#ifdef OLDCODE
/**
*
* code = locatetask();
*
* Return task object.
*
**/
PRIVATE Object *locatetask(void)
{
  char name[256];

  MachineName(name);
  strcat(name, "/tasks/");
  strcat(name, ((ObjNode *)MyTask->TaskEntry)->Name);
  return Locate(NULL, name);
}
#endif

/*
-- crf : 11/08/91 - No longer needed
*/
#ifdef OLDCODE
/**
*
* obj = locatewm (Environ *env)
*
* Return Window Manager object.
*
**/
PRIVATE Object *locatewm (Environ *env)
{
#ifdef HE12
  return env->Objv[OV_CServer];
/* 
-- crf : OV_CServer = 7 = control console/window server (syslib.h)
*/

#else
  int i;
  Stream *window;
  char wmname[256];

  for ( i = 0; i < 3; i++ )	/* CR: search for an interactive I/O stream */
    if ( env->Strv[i]->Flags & Flags_Interactive )
    {
      window = env->Strv[i];
      break;
    }
  unless ( window ) return Null ( Object );
  strcpy ( wmname, window->Name );
  * ( strrchr ( wmname, '/' )) = '\0';
  return Locate ( NULL, wmname );
#endif
}
#endif

/**
*
* err = MySendEnv(port, env);
*
*
*
**/
PRIVATE word MySendEnv(Port port, Environ *env)
{
  Stream **strv = env->Strv;
  Stream *str;
  word *flagv = NULL;
  word err;
  word i;

  for (i = 0; strv[i] != NULL; i++);
  unless (i == 0)
  {
    if ((flagv = Malloc(i * sizeof(word))) == NULL) return EC_Error+EG_NoMemory;
  }
  for (i = 0; (str = strv[i]) != NULL; i++)
  {
    unless (str == (Stream *)MinInt)
    {
      flagv[i] = str->Flags;
      str->Flags &= ~(Flags_CloseOnSend | Flags_OpenOnGet);
    }
  }
  err = SendEnv(port, env);
  for (i = 0; (str = strv[i]) != NULL; i++)
  {
    unless (str == (Stream *)MinInt) str->Flags = flagv[i];
  }
  unless (flagv == NULL) Free(flagv);
  return err;
}

/**
*
* debugging = initdebug();
*
* Try to locate the debug server. If found register ourselves with it
* otherwise execute as normal.
*
*/
PRIVATE BOOL initdebug(void)
{
  Object *debugger;

#ifdef MEMCHECK
  Fork(10000, memcheck, 0);
#endif
  /*
  * Is the debug server loaded ?
  */
#ifdef	MYDEBUG
  unless ((debugger = Locate(NULL, "/mydebug")) == NULL)
#else
  unless ((debugger = Locate(NULL, "/debug")) == NULL)
#endif
  {
    Stream *dbgstream ;

    /*
    * Open a stream to it and register ourselves.
    */
    unless  ((dbgstream = Open(debugger, MyTask->Program->Module.Name, O_ReadWrite | O_Create)) == NULL)
    {
/*
-- crf : 12/08/91 - do not need these routines
*/
#ifdef OLDCODE
      Environ *env = getenviron();
      Object *newobjv[5];

      newobjv[0] = env->Objv[0];
      /* ACE: should test for NULL object */
      newobjv[1] = locatecode();
      newobjv[2] = locatetask();
      newobjv[3] = locatewm ( env );
      newobjv[4] = Null ( Object );
      
      env->Objv = newobjv;
      if (MySendEnv(dbgstream->Server, env) == Err_Null)
#endif
/*
-- crf : 12/08/91 - need the following entries
OV_Cdir		0	current directory			
OV_Task		1	ProcMan task entry			
OV_Code		2	Loader code entry			
OV_CServer	7	control console/window server	

-- don't need these ...
OV_Source	3	original program source file		
OV_Parent	4	this task's parent			
OV_Home		5	home directory			
OV_Console	6	control console			
OV_Session	8	user's session manager entry		
OV_TFM		9	user's task force manager	
OV_TForce	10	TFM entry for parent task force	
OV_End		11	NULL at end of Objv			
*/
      
      Environ *env = getenviron();

      if (MySendEnv(dbgstream->Server, env) == Err_Null)
      {
        unless ((debug = New(DEBUG)) == NULL)
        {
          int i;

          debug->stream = dbgstream;
          debug->port = dbgstream->Server;
          debug->reply = dbgstream->Reply;
          InitList(&debug->threadlist);
          InitList(&debug->breakpointlist);
          InitList(&debug->watchpointlist);
          InitSemaphore(&debug->lock, 1);
          for (i = 0; i < HashMax; i++) InitList(&debug->functable[i]);
          initthread(&thread);
          Close(debugger);
#ifdef CRs
          IOdebug ("remdebug called from initdebug ");
#endif
          atexit(remdebug);
          debug->GetMsg = patchgetmsg(DBGGetMsg);
          debug->PutMsg = patchputmsg(DBGPutMsg);
          {
            DBG dbg;

            dbg.data = NULL;
            (void)getdebug(debug->reply, &dbg);
          }
          if (Fork(10000, debugworker, 0))
          {
            debug->Fork = patchfork(DBGFork);
            return TRUE;
          }
        }
      }
      Close(dbgstream);
    }
    Close(debugger);
  }
  patchentry(&thread, ignore_entry);
  patchcommand(&thread, ignore_command);
  patchreturn(&thread, ignore_return);
  return FALSE;
}

/**
*
* remdebug();
*
* Terminate debugging.
*
**/
PRIVATE void remdebug(void)
{
  unless (debug == NULL)
  {
    int i;
    
#ifdef CRs
    IOdebug ("remdebug running");
#endif
    
    Wait(&debug->lock);
    Signal(&debug->lock);
        	    
    (void)patchgetmsg(debug->GetMsg);
#ifdef CRs
    IOdebug("patchgetmsg done");
#endif
    (void)patchputmsg(debug->PutMsg);
#ifdef CRs
    IOdebug("patchputmsg done");
#endif
    (void)patchfork(debug->Fork);
#ifdef CRs
    IOdebug("patchfork done");
#endif

#ifdef OLDCODE
    /* ACE: This should kill off all threads */
    /* ACE: Threads will be killed off anyway.
            More importantly this frees the thread structure which
            has disasterous effects - the patched module table is
            thrown away ! */
    (void)WalkList(&debug->threadlist, (WordFnPtr)remthread, 0);
#endif
    (void)WalkList(&debug->breakpointlist, (WordFnPtr)rembreak, 0);
#ifdef CRs
    IOdebug("rembreak done");
#endif

/*    (void)WalkList(&debug->watchpointlist, (WordFnPtr)remwatch, 0);*/
    for (i = 0; i < HashMax; i++)
      (void)WalkList(&debug->functable[i], (WordFnPtr)remfunc, 0);
#ifdef CRs
    IOdebug("remfunc done");
#endif
    Close(debug->stream);
    Free(debug);
    debug = NULL;
  }
}

/**
*
* err = getdebug(port, dbg);
*
* Get debug command from the debuggger.
*
**/
PRIVATE word getdebug(Port port, DBG *dbg)
{
  MCB mcb;
  word err;

  InitMCB(&mcb, 0, port, NullPort, 0);
#ifdef	MYDEBUG
  mcb.Timeout = -1;
#endif  
  mcb.MsgHdr.ContSize = sizeof(DBGCMD) / sizeof(word);
  mcb.MsgHdr.DataSize = (dbg->data == NULL) ? 0 : (int)dbg->cmd.size;
  mcb.Control = (word *)&dbg->cmd;
  mcb.Data = dbg->data;
  while ((err = RealGetMsg(&mcb)) == EK_Timeout);
  dbg->port = mcb.MsgHdr.Reply;
#ifdef CRs
  IOdebug("getdebug %x %x %x %d %d",dbg->cmd.action, dbg->cmd.thread, 
      dbg->cmd.modnum, dbg->cmd.offset, dbg->cmd.size);
#endif      
  return err;
}

/**
*
* putdebug(port, dbg);
*
* Send debug message to the debugger.
*
**/
PRIVATE word putdebug(Port port, DBG *dbg)
{
  MCB mcb;
  word err;

  InitMCB(&mcb, MsgHdr_Flags_preserve, port, NullPort, FC_Private|FG_DebugCmd);
#ifdef MYDEBUG
  mcb.Timeout = -1;
#endif 
  mcb.MsgHdr.ContSize = sizeof(DBGCMD) / sizeof(word);
  mcb.MsgHdr.DataSize = (dbg->data == NULL) ? 0 : (int)dbg->cmd.size;
  mcb.Control = (word *)&dbg->cmd;
  mcb.Data = dbg->data;
  while ((err = RealPutMsg(&mcb)) == EK_Timeout);
#ifdef CRs
  IOdebug("putdebug %x %x %x %d %d",dbg->cmd.action, dbg->cmd.thread, 
      dbg->cmd.modnum, dbg->cmd.offset, dbg->cmd.size);
#endif      
  return err;
}

/**
*
* senddebug(port, reply, dbg);
*
* Send debug message to the debugger.
*
**/
PRIVATE word senddebug(Port port, Port reply, DBG *dbg)
{
  MCB mcb;
  word err;

  InitMCB(&mcb, MsgHdr_Flags_preserve, port, reply, FC_Private|FG_DebugCmd);
#ifdef MYDEBUG
  mcb.Timeout = -1;
#endif  
  mcb.MsgHdr.ContSize = sizeof(DBGCMD) / sizeof(word);
  mcb.MsgHdr.DataSize = (dbg->data == NULL) ? 0 : (int)dbg->cmd.size;
  mcb.Control = (word *)&dbg->cmd;
  mcb.Data = dbg->data;
  while ((err = RealPutMsg(&mcb)) == EK_Timeout);
#ifdef CRs
  IOdebug("senddebug %x %x %x %d %d",dbg->cmd.action, dbg->cmd.thread, 
      dbg->cmd.modnum, dbg->cmd.offset, dbg->cmd.size);
#endif
  return err;
}

/**
*
* debugworker();
*
* Accepts and obeys debug commands from debugger.
*
**/
PRIVATE void debugworker(void)
{
  DBG dbg;
  byte data[1024]; /* ACE: not perfect */

  dbg.cmd.size = 1024;
  dbg.data = data;
  
  until (getdebug(debug->reply, &dbg) < Err_Null)
  {
    switch (dbg.cmd.action)
    {
      case DBG_Call:
      /* ACE: Implement this by forking a new process to execute the function.
              When the process has terminated we must return the result on the
              reply port.
      dbg.cmd.modnum = address of function
      dbg.cmd.size   = size of parameters
      dbg.cmd.offset = size of return parameters expected
      dbg.data       = parameters
      */
      patchentry(&thread, ignore_entry);
      patchcommand(&thread, ignore_command);
      patchreturn(&thread, ignore_return);
      call(dbg.port, (VoidFnPtr)dbg.cmd.modnum, dbg.cmd.size, (word *)dbg.data, dbg.cmd.offset);
      patchentry(&thread, _notify_entry);
      patchcommand(&thread, _notify_command);
      patchreturn(&thread, _notify_return);
      continue;

      case DBG_Fork:
      /*
      dbg.cmd.modnum = address of function
      dbg.cmd.size   = size of parameters
      dbg.cmd.offset = size of return parameters expected
      dbg.data       = parameters
      */
      {
        word *proc = (word *)DBGNewProcess(10000, (VoidFnPtr)dbg.cmd.modnum, dbg.cmd.size);

        unless (proc == NULL)
        {
          memcpy(proc, dbg.data, (int)dbg.cmd.size);
          StartProcess(proc, 1);
        }
      }
      continue;

      case DBG_Free:
      patchentry((THREAD *)dbg.cmd.thread, ignore_entry);
      patchcommand((THREAD *)dbg.cmd.thread, ignore_command);
      patchreturn((THREAD *)dbg.cmd.thread, ignore_return);
      resume((THREAD *)dbg.cmd.thread);
      continue;

      case DBG_FreeAll:
      (void)patchgetmsg(debug->GetMsg);
      (void)patchputmsg(debug->PutMsg);
      (void)patchfork(debug->Fork);
      Wait(&debug->lock);
      (void)WalkList(&debug->threadlist, (WordFnPtr)patchentry, (word)ignore_entry);
      (void)WalkList(&debug->threadlist, (WordFnPtr)patchcommand, (word)ignore_command);
      (void)WalkList(&debug->threadlist, (WordFnPtr)patchreturn, (word)ignore_return);
      (void)WalkList(&debug->threadlist, (WordFnPtr)resume, 0);
      Signal(&debug->lock);
#ifdef CRs
      IOdebug (" remdebug called from debugworker" );
#endif    
      remdebug();
      return;

      case DBG_Go:
      unstopthread((THREAD *)dbg.cmd.thread);
      patchcommand((THREAD *)dbg.cmd.thread, _notify_command);
      resume((THREAD *)dbg.cmd.thread);
      continue;

      case DBG_GoAll:
      Wait(&debug->lock);
      (void)WalkList(&debug->threadlist, (WordFnPtr)unstopthread, 0);
      (void)WalkList(&debug->threadlist, (WordFnPtr)patchcommand, (word)_notify_command);
      (void)WalkList(&debug->threadlist, (WordFnPtr)resume, 0);
      Signal(&debug->lock);
      continue;

      case DBG_Goto:
#ifdef V1_1
      addbreak(dbg.cmd.modnum, dbg.cmd.offset, 1, FALSE);
#else
      /* This implementation of goto is crap */
      addbreak(dbg.cmd.modnum, dbg.cmd.offset, 1);
#endif
      unstopthread((THREAD *)dbg.cmd.thread);
      patchcommand((THREAD *)dbg.cmd.thread, _notify_command);
      resume((THREAD *)dbg.cmd.thread);
      continue;

      case DBG_GotoAll:
#ifdef V1_1
      addbreak(dbg.cmd.modnum, dbg.cmd.offset, 1, TRUE);
#else
      addbreak(dbg.cmd.modnum, dbg.cmd.offset, 1);
#endif
      Wait(&debug->lock);
      (void)WalkList(&debug->threadlist, (WordFnPtr)unstopthread, 0);
      (void)WalkList(&debug->threadlist, (WordFnPtr)patchcommand, (word)_notify_command);
      (void)WalkList(&debug->threadlist, (WordFnPtr)resume, 0);
      Signal(&debug->lock);
      continue;

      case DBG_GotoFrame:
      gotoframethread((THREAD *)dbg.cmd.thread, (int)dbg.cmd.modnum);
      patchcommand((THREAD *)dbg.cmd.thread, framestop_command);
      resume((THREAD *)dbg.cmd.thread);
      continue;

      case DBG_GotoFrameAll:
      Wait(&debug->lock);
      (void)WalkList(&debug->threadlist, (WordFnPtr)gotoframethread, dbg.cmd.modnum);
      (void)WalkList(&debug->threadlist, (WordFnPtr)patchcommand, (word)framestop_command);
      (void)WalkList(&debug->threadlist, (WordFnPtr)patchentry, (word)_notify_entry);
      (void)WalkList(&debug->threadlist, (WordFnPtr)patchreturn, (word)_notify_return);                  
      (void)WalkList(&debug->threadlist, (WordFnPtr)resume, 0);
      Signal(&debug->lock);
      continue;

      case DBG_Kill:
      Wait(&debug->lock);
      remthread((THREAD *)dbg.cmd.thread);
      Signal(&debug->lock);
      /* ACE: Have to actually stop the thread */
      continue;

      case DBG_KillAll:
      _exit(1);
      continue;

      case DBG_Profile:
      if (dbg.cmd.modnum == -1)
      {
        profilethread((THREAD *)dbg.cmd.thread, (int)dbg.cmd.size);
        patchcommand((THREAD *)dbg.cmd.thread, _notify_command);
      }
      else
      {
      	FUNC *func = addfunc(findproc(dbg.cmd.modnum, dbg.cmd.offset));

        if (dbg.cmd.size) func->flags |= Profile;
	else func->flags &= ~Profile;
      }
      continue;

      case DBG_ProfileAll:
      Wait(&debug->lock);
      (void)WalkList(&debug->threadlist, (WordFnPtr)profilethread, dbg.cmd.size);
      (void)WalkList(&debug->threadlist, (WordFnPtr)patchcommand, (word)_notify_command);
      Signal(&debug->lock);
      continue;

      case DBG_Step:
      stopthread((THREAD *)dbg.cmd.thread);
      patchcommand((THREAD *)dbg.cmd.thread, _notify_command);
      resume((THREAD *)dbg.cmd.thread);
      continue;

      case DBG_StepAll:
      Wait(&debug->lock);
      (void)WalkList(&debug->threadlist, (WordFnPtr)stopthread, 0);
      (void)WalkList(&debug->threadlist, (WordFnPtr)patchcommand, (word)_notify_command);
      (void)WalkList(&debug->threadlist, (WordFnPtr)resume, 0);
      Signal(&debug->lock);
      continue;

      case DBG_Stop:
      stopthread((THREAD *)dbg.cmd.thread);
      patchcommand((THREAD *)dbg.cmd.thread, _notify_command);
      continue;

      case DBG_StopAll:
      Wait(&debug->lock);
      (void)WalkList(&debug->threadlist, (WordFnPtr)stopthread, 0);
      (void)WalkList(&debug->threadlist, (WordFnPtr)patchcommand, (word)_notify_command);
      Signal(&debug->lock);
      continue;

      case DBG_Timeout:
      timeoutthread((THREAD *)dbg.cmd.thread);
      continue;

      case DBG_TimeoutAll:
      Wait(&debug->lock);
      (void)WalkList(&debug->threadlist, (WordFnPtr)timeoutthread, 0);
      Signal(&debug->lock);
      continue;

      case DBG_Trace:
      if (dbg.cmd.modnum == -1)
      {
        tracethread((THREAD *)dbg.cmd.thread, (int)dbg.cmd.size);
        patchcommand((THREAD *)dbg.cmd.thread, _notify_command);
      }
      else
      {
      	FUNC *func = addfunc(findproc(dbg.cmd.modnum, dbg.cmd.offset));

	if (dbg.cmd.size & TraceOff) func->flags &= ~dbg.cmd.size;
	else func->flags |= dbg.cmd.size;
      }
      continue;

      case DBG_TraceAll:
      Wait(&debug->lock);
      (void)WalkList(&debug->threadlist, (WordFnPtr)tracethread, 0);
      (void)WalkList(&debug->threadlist, (WordFnPtr)patchcommand, (word)_notify_command);
      Signal(&debug->lock);
      continue;

      case DBG_AddBreak:
#ifdef V1_1
      addbreak(dbg.cmd.modnum, dbg.cmd.offset, (int)dbg.cmd.size, FALSE);
#else
      addbreak(dbg.cmd.modnum, dbg.cmd.offset, dbg.cmd.size);
#endif
      continue;

      case DBG_RemBreak:
      {
      	BREAKPOINT *breakpoint;
      	
        unless ((breakpoint = findbreak(dbg.cmd.modnum, dbg.cmd.offset)) == NULL)
          rembreak(breakpoint);
      }
      continue;

      case DBG_AddWatch:
      addwatch((void *)dbg.cmd.modnum, dbg.cmd.size);
      continue;

      case DBG_RemWatch:
      {
      	WATCHPOINT *watchpoint;
	Wait (&debug->lock);      	
#ifdef CRs
	IOdebug ("case RemWatch"); 
#endif
        unless ((watchpoint = findwatch((void *)dbg.cmd.modnum, dbg.cmd.size)) == NULL)
        {
#ifdef CRs
	  IOdebug ("RemWatch calling for %x", watchpoint);
#endif
         remwatch(watchpoint);
        }
/*        dbg.data = NULL;
        dbg.cmd.action = DBG_Ready;
        putdebug(dbg.port, &dbg);  CR: does not work */
	Signal (&debug->lock);
      }
      continue;

      case DBG_PeekMem:
      peekmem(dbg.port, (void *)dbg.cmd.modnum, dbg.cmd.size);
      continue;

      case DBG_PokeMem:
      pokemem((void *)dbg.cmd.modnum, dbg.cmd.size, dbg.data);
      continue;

      case DBG_PeekData:
      peekdata(dbg.port, dbg.cmd.modnum, dbg.cmd.offset, dbg.cmd.size);
      continue;

      case DBG_PokeData:
      pokedata(dbg.cmd.modnum, dbg.cmd.offset, dbg.cmd.size, dbg.data);
      continue;

      case DBG_PeekStack:
      peekstack(dbg.port, (THREAD *)dbg.cmd.thread, dbg.cmd.modnum, dbg.cmd.offset, dbg.cmd.size);
      continue;

      case DBG_PokeStack:
      pokestack((THREAD *)dbg.cmd.thread, dbg.cmd.modnum, dbg.cmd.offset, dbg.cmd.size, dbg.data);
      continue;

      case DBG_LocateData:
      locatedata(dbg.port, dbg.cmd.modnum, dbg.cmd.offset);
      continue;

      case DBG_LocateStack:
      locatestack(dbg.port, (THREAD *)dbg.cmd.thread, dbg.cmd.modnum, dbg.cmd.offset);
      continue;

      case DBG_Where:
      where(dbg.port, (THREAD *)dbg.cmd.thread, dbg.cmd.modnum);
      continue;

      default:
      IOdebug("Unknown debug command %d", dbg.cmd.action);
      continue;
    }
  }
}

/**
*
* pushframe(proc, wptr);
*
* Record new activation record for current thread.
*
**/
PRIVATE void pushframe(Proc *proc, byte *wptr)
{
  /* ACE: Should be able to take the test out at some stage */
  unless (thread.framestk == NULL)
  {
    thread.frameptr--;
    /* ACE: ! */
    if (thread.frameptr < thread.framestk OR
        thread.frameptr > thread.framestk + FrameStackSize)
      IOdebug("DLIB: Frame pointer out of range");
    thread.frameptr->proc = proc;
    thread.frameptr->wptr = wptr;
  }
}

/**
*
* popframe();
*
* Record termination of activation record for current thread.
*
**/
PRIVATE void popframe(void)
{
  unless (thread.framestk == NULL)
  {
    thread.frameptr++;
    /* ACE: ! */
    if (thread.frameptr < thread.framestk OR
        thread.frameptr > thread.framestk + FrameStackSize)
      IOdebug("DLIB: Frame pointer out of range");
  }
}

/**
*
* Notify functions:
*
* Next follows a list of notify functions to suit all occasions.
*
**/

/**
*
* profile(modnum, line);
*
* Profile command.
*
**/
PRIVATE void profile(word modnum, word line)
{
  modnum = modnum;
  line = line;
}

/**
*
* stop(modnum, line);
*
* Stop command.
*
**/
PRIVATE void stop(word modnum, word line)
{
  DBG dbg;

  dbg.cmd.action = DBG_Stopped;
  dbg.cmd.thread = (word)&thread;
  dbg.cmd.modnum = modnum;
  dbg.cmd.offset = line;
  dbg.data = NULL;
  Wait(&debug->lock); /* ACE: This should not be necessary */
#ifdef V1_1
/* remove all tempory breakpoints set by go until */
  (void)WalkList(&debug->breakpointlist, (WordFnPtr)remtempbreak, 0);
#endif
  putdebug(debug->port, &dbg);
  Signal(&debug->lock);
  suspend(&thread);
}

/**
*
* breakpoint(modnum, line);
*
* Breakpoint command.
*
**/
PRIVATE BOOL breakpoint(word modnum, word line)
{
  BREAKPOINT *breakpoint;

  if ((breakpoint = findbreak(modnum, line)) == NULL) return FALSE;
  if (++breakpoint->count < breakpoint->threshold) return FALSE;
  breakpoint->count = 0;
  stop(modnum, line);
  return TRUE;
}

/**
*
* trace(modnum, line);
*
* Trace command function.
*
**/
PRIVATE void trace(word modnum, word line)
{
  DBG dbg;
#ifndef BUG
  Port port = NewPort();
  word err;
#endif

  dbg.cmd.action = DBG_Traced;
  dbg.cmd.thread = (word)&thread;
  dbg.cmd.modnum = modnum;
  dbg.cmd.offset = line;
  dbg.data = NULL;
  Wait(&debug->lock); /* ACE: This should not be necessary */
#ifdef BUG
  putdebug(debug->port, &dbg);
#else
  if ((err = senddebug(debug->port, port, &dbg)) == Err_Null)
  {
    getdebug(port, &dbg);
  }
  else IOdebug("senddebug returned an error (%x)", err);
  FreePort(port);
#endif
  Signal(&debug->lock);
}

/**
*
* _notify_entry(proc, wptr);
*
* Standard notify entry function.
*
**/
PUBLIC void _notify_entry(Proc *proc, byte *wptr)
{
  FUNC *func;
  word flags = thread.flags;

  if (debug == NULL AND !initdebug()) return;
  Wait(&debug->lock);
  (void)WalkList(&debug->watchpointlist, (WordFnPtr)checkwatchret, 1);
  Signal(&debug->lock);
  unless ((func = findfunc(proc)) == NULL) flags = func->flags;
  pushframe(proc, wptr);
  if (flags & TraceEntry) entered(proc);
  if (flags & TraceCommand) thread.tracing = TRUE;
  else thread.tracing = FALSE;
  if (flags & Profile) thread.profiling = TRUE;
  else thread.profiling = FALSE;
}

/**
*
* _notify_return(proc, result);
*
* Standard noifty return function.
*
**/
PUBLIC word _notify_return(Proc *proc, word result)
{
  FUNC *func;
  word flags = thread.flags;

  /* ACE: Need a more efficient way of doing this */
  Wait(&debug->lock);
  (void)WalkList(&debug->watchpointlist, (WordFnPtr)checkwatchret, -1);
  Signal(&debug->lock);

  delwatchpoints () ; /* -- crf */

  unless ((func = findfunc(proc)) == NULL) flags = func->flags;
  if (flags & TraceReturn) returned(proc);
  popframe();
  flags = thread.flags;
  unless ((func = findfunc(thread.frameptr->proc)) == NULL) flags = func->flags;
  if (flags & TraceCommand) thread.tracing = TRUE;
  else thread.tracing = FALSE;
  if (flags & Profile) thread.profiling = TRUE;
  else thread.profiling = FALSE;
  return result;
}

/**
*
* _notify_command(line, module);
*
* Standard notify command function.
*
**/
PUBLIC void _notify_command(word line, Module *module)
{
  word modnum = (module->Type == T_Module) ? module->Id : ((Source *)module)->Id;

  thread.frameptr->line = line;
  if (thread.profiling) profile(modnum, line);
#ifndef OLDCODE
  Wait(&debug->lock);
  (void)WalkList(&debug->watchpointlist, (WordFnPtr)checkwatch, 0);
  Signal(&debug->lock);
#endif
  if (thread.stopping) stop(modnum, line);
  else
  {
    unless (breakpoint(modnum, line))
    {
#ifdef OLDCODE
      Wait(&debug->lock);
      (void)WalkList(&debug->watchpointlist, (WordFnPtr)checkwatch, 0);
      Signal(&debug->lock);
#endif
      if (thread.tracing) trace(modnum, line);
    }
  }
}

/**
*
* ignore_entry(proc, wptr)
*
*
*
**/
PRIVATE void ignore_entry(Proc *proc, byte *wptr)
{
  proc = proc;
  wptr = wptr;
}

PRIVATE word ignore_return(Proc *proc, word result)
{
  proc = proc;
  return result;
}

PRIVATE void ignore_command(word line, Module *module)
{
  line = line;
  module = module;
}

PRIVATE void framestop_command(word line, Module *module)
{
  word modnum = (module->Type == T_Module) ? module->Id : ((Source *)module)->Id;

  thread.frameptr->line = line;
  if (thread.profiling) profile(modnum, line);
  Wait(&debug->lock);
  (void)WalkList(&debug->watchpointlist, (WordFnPtr)checkwatch, 0);
  Signal(&debug->lock);
  if (thread.frameptr >= thread.stopframe) stop(modnum, line);
  else if (thread.tracing) trace(modnum, line);
}

/**
*
* initthread(thread);
*
* Intialise a thread structure.
*
**/
PRIVATE void initthread(THREAD *thread)
{
  Wait(&debug->lock);
  AddHead(&debug->threadlist, &thread->node);
  Signal(&debug->lock);
  InitSemaphore(&thread->sync, 0);
  if ((thread->framestk = (FRAME *)Malloc(FrameStackSize * sizeof(FRAME))) == NULL)
    IOdebug("DLIB: Failed to allocate frame stack");
  thread->frameptr = thread->framestk + FrameStackSize;
  thread->profiling = FALSE;
  thread->stopping = TRUE;
  thread->tracing = FALSE;
#ifdef NEWCODE
  thread->watching = FALSE;
#endif
  thread->port = NullPort;
}

/**
*
* remthread(thread);
*
* Remove a thread.
*
**/
PRIVATE void remthread(THREAD *thread)
{
  Remove(&thread->node);
  unless (thread->framestk == NULL) Free(thread->framestk);
}

/**
*
* suspend(thread);
*
* Suspend execution of a thread.
*
**/
PRIVATE void suspend(THREAD *thread)
{
  Wait(&thread->sync);
}

/**
*
* resume(thread);
*
* Resume execution of a suspended thread.
*
**/
PRIVATE void resume(THREAD *thread)
{
  if (TestSemaphore(&thread->sync) < 0) Signal(&thread->sync);
}

/**
*
* profilethread(thread, on);
*
* Set profile flag.
*
**/
PRIVATE void profilethread(THREAD *thread, BOOL on)
{
  if (on) thread->flags |= Profile;
  else thread->flags &= ~ Profile;
}

/**
*
* stopthread(thread);
*
* Set stop flag.
*
**/
PRIVATE void stopthread(THREAD *thread)
{
  thread->stopping = TRUE;
}

/**
*
* unstopthread(thread);
*
* Unset stop flag.
*
**/
PRIVATE void unstopthread(THREAD *thread)
{
  thread->stopping = FALSE;
}

/**
*
* tracethread(thread, flags);
*
* Set trace flags.
*
**/
PRIVATE void tracethread(THREAD *thread, word flags)
{
  if (flags & TraceOff) thread->flags &= ~flags;
  else thread->flags |= flags;
  if (thread->flags & TraceCommand) thread->tracing = TRUE;
  else thread->tracing = FALSE;
}

/**
*
* gotoframethread(thread, frame);
*
* Stop a thread when it is in a particular frame.
*
**/
PRIVATE void gotoframethread(THREAD *thread, int frame)
{
  thread->stopframe = thread->frameptr + frame;
}

/**
*
* timeoutthread(thread);
*
* Force a timeout to occur if the thread is communicating.
*
**/
PRIVATE void timeoutthread(THREAD *thread)
{
  unless (thread->port == NullPort) AbortPort(thread->port, EK_Timeout);
}

/**
*
* patchentry(thread, func);
*
* Patch the notify entry routine.
*
**/
PRIVATE void patchentry(THREAD *thread, void (*func)(Proc *, byte *))
{
  thread->notify_entry = func;
}

/**
*
* patchcommand(thread, func);
*
* Patch the notify command routine.
*
**/
PRIVATE void patchcommand(THREAD *thread, void (*func)(word, Module *))
{
  thread->notify_command = func;
}

/**
*
* patchreturn(thread, func);
*
* Patch the notify return routine.
*
**/
PRIVATE void patchreturn(THREAD *thread, word (*func)(Proc *, word))
{
  thread->notify_return = func;
}

/**
*
* modtab = newmodtab(oldmodtab);
*
* Create a new modtab.
*
**/
word *newmodtab(word *oldmodtab)
{
  word *modtab;
  Module *module = &MyTask->Program->Module;
  word maxid = 0;
  word maxdata = 0;

#ifdef OLDCODE
  /* remove from release version */
  /* ACE: Check we're right */
  unless (MyTask->ModTab == oldmodtab); /* IOdebug("DLIB: Not the modtab!");*/
#endif

  until (module->Type == 0)
  {
    if (module->Id > maxid) maxid = module->Id;
    if (module->Id == DLib_Slot) maxdata = ((ResRef *)module)->Module->MaxData;
    module = (Module *)((byte *)module + module->Size);
  }
  if ((modtab = (word *)Malloc((maxid + 1) * sizeof(word))) == NULL)
    return NULL;
  memcpy(modtab, oldmodtab, (int)(maxid + 1) * sizeof(word));
  modtab[0] = (word)modtab;
  if ((modtab[DLib_Slot] = (word)Malloc(maxdata * sizeof(word))) == NULL)
  {
    Free(modtab);
    return NULL;
  }
  /* ACE: Only need to copy debug pointer really */
  memcpy((void *)modtab[DLib_Slot], (void *)oldmodtab[DLib_Slot], (int)maxdata * sizeof(word));
  initthread((THREAD *)modtab[DLib_Slot]);
  return modtab;
}

/**
*
* proc = DBGNewProcess(stacksize, func, argsize);
*
* A private version of NewProcess().
*
**/
PRIVATE void *DBGNewProcess(word stacksize, VoidFnPtr func, word argsize)
{
  word *stack = Malloc(stacksize);
  word *display = stack + (stacksize / sizeof(word)) - 6;

  if (stack == NULL) return NULL;
  if ((display[0] = (word)newmodtab((word *)(((word **)&stacksize)[-1][0]))) == NULL)
  {
    Free(stack);
    return NULL;
  }
  display[1] = (word)stack;
  return InitProcess(display, func, _DBGProcExit, display, argsize);
}

/**
*
* success = DBGFork(stacksize, func, argsize, ...);
*
*
*
**/
PRIVATE word DBGFork(word stacksize, VoidFnPtr func, word argsize, ...)
{
  byte *args = (byte *)(&argsize + 1);
  word *proc = (word *)DBGNewProcess(stacksize, func, argsize);

  if (proc == NULL) return FALSE;
  memcpy(proc, args, (int)argsize);
  StartProcess(proc, 1);
  return TRUE;
}

#ifdef NOT_USED
/**
*
* ProcStop(modtab);
*
*
*
**/
PRIVATE void ProcStop(word *modtab)
{
  /* ACE: Not sure if this always gets called */
  /* ACE: I am a bit worried that this can get called after I have freed the
          debug structure. Locking things with the semaphore in the debug
          structure is not a complete solution because of the fact that it
          is part of the debug structure. For now I test debug against NULL */
  unless (debug == NULL)
  {
    DBG dbg;

    dbg.cmd.action = DBG_EndThread;
    dbg.cmd.thread = (word)&thread;
    dbg.data = NULL;
    putdebug(debug->port, &dbg);
  }
  remthread(&thread);
  Free((void *)modtab[DLib_Slot]);
  Free(modtab);
}

#endif /* NOT_USED */

/**
*
* addbreak(modnum, line, count);
* addbreak(modnum, line, count,temp); version 1.1 onwards
*
* Add a breakpoint.
*
**/
#ifdef V1_1
PRIVATE void addbreak(word modnum, word line, word threshold,BOOL temp)
#else
PRIVATE void addbreak(word modnum, word line, word threshold)
#endif
{
  BREAKPOINT *breakpoint;

  if ((breakpoint = New(BREAKPOINT)) == NULL) return;
  
  breakpoint->loc.modnum = modnum;
  breakpoint->loc.line   = line;
  breakpoint->threshold  = threshold;
  breakpoint->count      = 0;
#ifdef V1_1
/* set a variable if this has be called by a go until function */
  breakpoint->temp       = temp;
#endif
  
  Wait(&debug->lock);
  AddHead(&debug->breakpointlist, &breakpoint->node);
  Signal(&debug->lock);
}

/**
*
* found = cmpbreak(breakpoint, loc);
*
* Support routine for findbreak();
*
**/
PRIVATE BOOL cmpbreak(BREAKPOINT *breakpoint, LOCATION *loc)
{
  return breakpoint->loc.modnum == loc->modnum AND
         breakpoint->loc.line == loc->line;
}

/**
*
* breakpoint = findbreak(modnum, line);
*
* Find a breakpoint.
*
**/
PRIVATE BREAKPOINT *findbreak(word modnum, word line)
{
  BREAKPOINT *breakpoint;
  LOCATION loc;

  loc.modnum = modnum;
  loc.line = line;
  Wait(&debug->lock);
  breakpoint = (BREAKPOINT *)SearchList(&debug->breakpointlist, (WordFnPtr)cmpbreak, (word)&loc);
  Signal(&debug->lock);
  return breakpoint;
}

/**
*
* rembreak(breakpoint);
*
* Remove breakpoint.
*
**/
PRIVATE void rembreak(BREAKPOINT *breakpoint)
{
  Remove(&breakpoint->node);
  Free(breakpoint);
}

#ifdef V1_1
/**
*
* remtempbreak(breakpoint);
*
* Remove breakpoint.
*
**/
PRIVATE void remtempbreak(BREAKPOINT *breakpoint)
{
  if (breakpoint->temp == TRUE)
  {
    Remove(&breakpoint->node);
    Free(breakpoint);
  }
}
#endif

/**
*
* addwatch(addr, size);
*
* Add a watchpoint.
*
**/
PRIVATE void addwatch(void *addr, word size)
{
  WATCHPOINT *watchpoint;
  
  Wait (&debug->lock);
  if ((watchpoint = findwatch(addr, size)) == NULL)
  {
    unless ((watchpoint = (WATCHPOINT *)Malloc(sizeof(WATCHPOINT) + size)) == NULL)
    {
      watchpoint->loc.addr = addr;
      watchpoint->loc.size = size;
      watchpoint->usage = 1;
      memcpy(&watchpoint->copy, addr, (int)size);
      AddHead(&debug->watchpointlist, &watchpoint->node);
    }
  }
  else watchpoint->usage++;
  Signal(&debug->lock);  
}

/**
*
* found = cmpwatch(watch, loc);
*
* Support routine for findwatch().
*
**/
PRIVATE BOOL cmpwatch(WATCHPOINT *watchpoint, MEMLOCATION *loc)
{
  return watchpoint->loc.addr == loc->addr AND
         watchpoint->loc.size == loc->size;
}

/**
*
* watchpoint = findwatch(addr, size);
*
* Find a watchpoint.
*
**/
PRIVATE WATCHPOINT *findwatch(void *addr, word size)
{
  WATCHPOINT *watchpoint;
  MEMLOCATION loc;
  
  loc.addr = addr;
  loc.size = size;

  watchpoint = (WATCHPOINT *)SearchList(&debug->watchpointlist, (WordFnPtr)cmpwatch, (word)&loc);

  return watchpoint;
}

/**
*
* remwatch(watchpoint);
*
* Remove a watchpoint.
*
**/
PRIVATE void remwatch(WATCHPOINT *watchpoint)
{
#ifdef CRs
  IOdebug ("remwatch %x", watchpoint);
#endif
  
  if (watchpoint->usage == 1)
  {
    Remove(&watchpoint->node);
    Free(watchpoint);
  }
  else watchpoint->usage --;

}

/**
*
* same = memcmp(m1, m2, size);
*
* Compare two blocks of memory.
*
**/
int memcmp(const void *m1, const void *m2, unsigned int size)
{
  byte *b1 = (byte *)m1;
  byte *b2 = (byte *)m2;

  while (size--)
    unless (*b1++ == *b2++) return 1;
  return 0;
}

/**
*
* checkwatch(watchpoint);
*
* Check a watchpoint to see if it has been activated.
*
**/
PRIVATE void checkwatch(WATCHPOINT *watchpoint, word scope)
{
  unless (memcmp(watchpoint->loc.addr, &watchpoint->copy, (int)watchpoint->loc.size) == 0)
  {
    DBG dbg;
    Port port = NewPort();  
    word err;
    
#ifdef CRs
    IOdebug ("entered checkwatch");  
#endif
    dbg.port = port;
    dbg.cmd.action = DBG_Changed;
    dbg.cmd.modnum = (word)watchpoint->loc.addr;
    dbg.cmd.size = watchpoint->loc.size;
    dbg.cmd.offset = scope ;/* CR: should transfer the callers id */
    dbg.data = NULL;

/* 
-- crf : 25/07/91 - Bug 700
-- Problem : loss of synchronization between debugger and dlib with regard
-- to watchpointing (refer "system.c").
-- Solution : explicitly force synchronization. I am using the same fixes
-- that were made to "trace()".

-- crf : 25/07/91 - The above fix also fixes the following :
-- Bug 652 - watchpointing multiple variables (1)
-- Bug 701 - watchpointing multiple variables (2)
-- Bug 703 - watchpointing multiple variables (3)
*/

/* -- crf : synchronization fix */
#ifdef BUG
    putdebug(debug->port, &dbg);
#else
    if ((err = senddebug(debug->port, port, &dbg)) == Err_Null)
    {
      getdebug(port, &dbg);
    }
#endif

/*
-- crf : 28/07/91
-- Take note of Carsten's comment below ("CR: causes timing problems")
-- I think I have sorted out the synchronization problems with the use of
-- the sub-routine "delwatchpoints()" ... this will require testing.
*/

#ifdef PARSYTEC
    if ((err = senddebug(debug->port, port, &dbg)) == Err_Null)
    {
      /* getdebug(port, &dbg); CR: causes timing problems */
#ifdef CRs
    IOdebug ("err = %d in checkwatch",err);  
#endif
    }
#endif

    else IOdebug("senddebug returned an error (%x)", err);
    FreePort(port);
    memcpy(&watchpoint->copy, watchpoint->loc.addr, (int)watchpoint->loc.size);
#ifdef OLDCODE
    thread.watchstop = TRUE;
#endif
  }
}

#ifdef OLDCODE
PRIVATE void activategoto(int modnum, int firstline, int lastline)
{
  thread.gotobreak.active = TRUE;
  thread.gotobreak.modnum = modnum;
  thread.gotobreak.firstline = firstline;
  thread.gotobreak.lastline = lastline;
}

PRIVATE BOOL isgoto(int modnum, int line)
{
  unless (thread.gotobreak.active) return FALSE;
  if (modnum == thread.gotobreak.modnum AND
      line >= thread.gotobreak.firstline AND line <= thread.gotobreak.lastline)
  {
    thread.gotobreak.active = FALSE;
    return TRUE;
  }
  return FALSE;
}
#endif

PRIVATE byte *dataloc(word modnum, word offset)
{
  return (byte *)(MyTask->ModTab[modnum] + offset);
}

PRIVATE byte *stackloc(THREAD *thread, word frame, word offset)
{
  return thread->frameptr[frame].wptr + offset;
}

PRIVATE void peekmem(Port port, byte *addr, word size)
{
  DBG dbg;

  dbg.cmd.action = DBG_Dump;
  dbg.cmd.modnum = (word)addr;
  dbg.cmd.size = size;
  dbg.data = addr;
  putdebug(port, &dbg);
}

PRIVATE void pokemem(byte *addr, word size, byte *data)
{
  memcpy(addr, data, (int)size);
}

PRIVATE void peekdata(Port port, word modnum, word offset, word size)
{
  DBG dbg;
  byte *addr = dataloc(modnum, offset);

  dbg.cmd.action = DBG_Dump;
  dbg.cmd.modnum = (int)addr;
  dbg.cmd.size = size;
  dbg.data = addr;
  putdebug(port, &dbg);
}

PRIVATE void pokedata(word modnum, word offset, word size, byte *data)
{
  memcpy(dataloc(modnum, offset), data, (int)size);
}

PRIVATE void peekstack(Port port, THREAD *thread, word frame, word offset, word size)
{
  DBG dbg;
  byte *addr = stackloc(thread, frame, offset);

  dbg.cmd.action = DBG_Dump;
  dbg.cmd.modnum = (word)addr;
  dbg.cmd.size = size;
  dbg.data = addr;
  putdebug(port, &dbg);
}

PRIVATE void pokestack(THREAD *thread, word frame, word offset, word size, byte *data)
{
  memcpy(stackloc(thread, frame, offset), data, (int)size);
}

PRIVATE void locatedata(Port port, word modnum, word offset)
{
  DBG dbg;

  dbg.cmd.action = DBG_Address;
  dbg.cmd.modnum = (int)dataloc(modnum, offset);
  dbg.data = NULL;
  putdebug(port, &dbg);
}

PRIVATE void locatestack(Port port, THREAD *thread, word frame, word offset)
{
  DBG dbg;

  dbg.cmd.action = DBG_Address;
  dbg.cmd.modnum = (word)stackloc(thread, frame, offset);
  dbg.data = NULL;
  putdebug(port, &dbg);
}

PRIVATE void call(Port port, VoidFnPtr func, word argsize, word *args, word retsize)
{
  DBG dbg;
  word rets;

/*
  ACE: Only support functions return single word
*/
  rets = DoCall(func, argsize, args);
  dbg.cmd.action = DBG_Return;
  dbg.cmd.size = retsize;
  dbg.data = (byte *)&rets;
  putdebug(port, &dbg);	
}

PRIVATE void where(Port port, THREAD *thread, word frame)
{
  DBG dbg;

  dbg.cmd.action = DBG_Position;
  if (thread->frameptr + frame >= thread->framestk + FrameStackSize)
    dbg.cmd.modnum = -1;
  else
  {
    ProcInfo *procinfo = GetProcInfo(thread->frameptr[frame].proc);

    dbg.cmd.modnum = procinfo->Modnum;
    dbg.cmd.offset = thread->frameptr[frame].line;
    dbg.cmd.size   = (procinfo->Offset << 2);
  }
  dbg.data = NULL;
  putdebug(port, &dbg);
}

PRIVATE Module *findmodule(word modnum)
{
  Module *module = &MyTask->Program->Module;

  until (module->Type == 0)
  {
    if (module->Id == modnum) return module;
    module = (Module *)((byte *)module + module->Size);
  }
  return NULL;
}

PRIVATE Proc *findproc(word modnum, word offset)
{
  byte *code = *(byte **)dataloc(modnum, offset);
  Proc *proc;
  Module *module;
  int i;

  if ((module = findmodule(modnum)) == NULL) return NULL;
  proc = (Proc *)module;
  for (i = 0; i < module->Size; i += sizeof(word))
  {
    if (proc->Type == T_Proc AND RTOA(proc->Proc) == code) return proc;
    proc = (Proc *)((word *)proc + 1);
  }
  return NULL;
}

PRIVATE word hashval(Proc *proc)
{
  return (uword)proc % HashMax;
}

PRIVATE int cmpfunc(FUNC *func, Proc *proc)
{
  return func->proc == proc;
}

PRIVATE FUNC *addfunc(Proc *proc)
{
  FUNC *func;
  int hash = hashval(proc);

  if ((func = (FUNC *)SearchList(&debug->functable[hash], (WordFnPtr)cmpfunc, (word)proc)) == NULL)
  {
    /* ACE: Test for no memory */
    func = New(FUNC);
    func->proc = proc;
    func->flags = 0;
    func->time = 0;
    func->calls = 0;
    AddHead(&debug->functable[hash], &func->node);
  }
  return func;
}

PRIVATE FUNC *findfunc(Proc *proc)
{
  return (FUNC *)SearchList(&debug->functable[hashval(proc)], (WordFnPtr)cmpfunc, (word)proc);
}

PRIVATE void remfunc(FUNC *func)
{
  Remove(&func->node);
  Free(func);
}

PRIVATE void entered(Proc *proc)
{
  DBG dbg;
  ProcInfo *procinfo = GetProcInfo(proc);

  dbg.cmd.action = DBG_Entered;
  dbg.cmd.thread = (word)&thread;
  dbg.cmd.modnum = procinfo->Modnum;
  dbg.cmd.offset = (procinfo->Offset << 2);
  dbg.data = NULL;
  Wait(&debug->lock); /* ACE: This should not be necessary */
  putdebug(debug->port, &dbg);
  Signal(&debug->lock);
}

PRIVATE void returned(Proc *proc)
{
  DBG dbg;
  ProcInfo *procinfo = GetProcInfo(proc);

  dbg.cmd.action = DBG_Returned;
  dbg.cmd.thread = (word)&thread;
  dbg.cmd.modnum = procinfo->Modnum;
  dbg.cmd.offset = (procinfo->Offset << 2);
  dbg.data = NULL;
  Wait(&debug->lock); /* ACE: This should not be necessary */
  putdebug(debug->port, &dbg);
  Signal(&debug->lock);
}

PRIVATE word DBGGetMsg(MCB *mcb)
{
  word err;

  thread.port = mcb->MsgHdr.Dest;
  mcb->Timeout = -1;
  err =  RealGetMsg(mcb);
  thread.port = NullPort;
  return err;
}

PRIVATE word DBGPutMsg(MCB *mcb)
{
  word err;

  thread.port = mcb->MsgHdr.Dest;
  mcb->Timeout = -1;
  err =  RealPutMsg(mcb);
  thread.port = NullPort;
  return err;
}
/**
*
* checkwatchret(watchpoint);
*
* notify debugger of changed frame
*
**/
PRIVATE int checkwatchret(WATCHPOINT *watchpoint, word scope)
{
  DBG dbg;
  word err;
  Port port = NewPort();
  
#ifdef CRs
    IOdebug ("entered checkwatchreturn");  
#endif
    dbg.port = port;
    dbg.cmd.action = DBG_Changed;
    dbg.cmd.modnum = (word)watchpoint->loc.addr;
    dbg.cmd.size = watchpoint->loc.size;
    dbg.cmd.offset = scope ;/* CR: should transfer the callers id */
    dbg.data = NULL;
    if((err = senddebug(debug->port, port, &dbg)) == Err_Null)
    {
    	getdebug ( port, &dbg);
    }
    FreePort (port);
    return TRUE;
}

/**
*
-- crf : 05/08/91 - explicitly remove watchpoints on exiting from routines
-- refer system.c : receiver () for the story ...
*
* delwatchpoints();
*
* delete outstanding watchpoints
*
**/
PRIVATE void delwatchpoints ()
{
  DBG dbg;
  word err;
  Port port = NewPort();
  
#ifdef CRs
    IOdebug ("entered delwatchpoints");  
#endif

    dbg.port = port;
    dbg.cmd.action = DBG_DelWatchIds;
    dbg.data = NULL;

    if((err = senddebug(debug->port, port, &dbg)) == Err_Null)
    {
    	getdebug ( port, &dbg);
    }
    FreePort (port);
}

char* strrchr (const char *str, int c)
{
   char *s = ( char * ) str;
   
   while (*s) s++;
   while (*s != c && s > ( char * ) str)
      s--;
   if (*s == c) return s;
   return NULL;
}

/***********************************************
*
*
*	CR: just a dummy to be used in a Walk
*
*
***********************************************/

/* -- crf : 07/08/91 - not used -

PRIVATE void dummy (WATCHPOINT *watchpoint, int i)
{
	if (watchpoint && i) return;
	return;
}
*/
