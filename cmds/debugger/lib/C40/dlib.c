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
*	Copyright (c) 1992, 1993 Perihelion Software Ltd.
*	All Rights Reserved.
**/


/* NB/ KEEP THIS FILE UP TO DATE WITH THE TRANSPUTER VERSION */

/*
static char *rcsid = "$Header: /users/nickc/RTNucleus/cmds/debugger/lib/C40/RCS/dlib.c,v 1.14 1994/03/29 15:26:05 nickc Exp $";
*/

#include <stdlib.h>
#include <helios.h>
#include <syslib.h>
#include <servlib.h>
#include <c40.h>
#include <stddef.h>
#include <stdarg.h>
#include <process.h>
#undef Malloc
#include <codes.h>
#include <module.h>
#include <posix.h>
#include <nonansi.h>
#include <string.h>

#include "dlib.h"
#include "dmsg.h"

/* Static data */

#define NUM_SAVED_REGS	33    /* MAXREGNUM in ncc/mbe/target.h is 28, + four extra words for four FP registers, + counting from 0, not 1 */

extern THREAD 		thread;
extern DEBUG *		debug;
extern word		saved_regs[ NUM_SAVED_REGS ];


extern word (*		patchfork(		word (*)(word, VoidFnPtr, word, ...)))(word, VoidFnPtr, word, ...);
extern word (*		patchputmsg(		word (*)(MCB * )) )( MCB * );
extern word (*		patchgetmsg(		word (*)(MCB * )) )( MCB * );
extern word		DoCall(			VoidFnPtr, word, word * );
extern void		FreeStop(		void * );
extern MPtr		get_module_table(	void );


#ifdef MEMCHECK

#define MEMVECSIZE 256

PRIVATE void
memcheck( void )
{
  int 			i;
  unsigned char *	lowmem = 0;
  unsigned char		savemem[ MEMVECSIZE ];

  
  for (i = 0; i < MEMVECSIZE; i++)
    savemem[ i ] = lowmem[ i ];

  forever
    {
      Delay( 2000000 );
    
      for (i = 0; i < MEMVECSIZE; i++)
	{
	  unless (savemem[ i ] == lowmem[ i ])
	    {
	      IOdebug( "DLIB: Memory corruption @ %x", i );
	      
	      savemem[ i ] = lowmem[ i ];
	    }
	}
    }
}
#endif /* MEMCHECK */


PRIVATE word
MySendEnv(
	  Port 		port,
	  Environ *	env )
{
  Stream **	strv = env->Strv;
  Stream *	str;
  word *	flagv = NULL;
  word		err;
  word		i;

  
  for (i = 0; strv[ i ] != NULL; i++)
    ;

  unless (i == 0)
    {
      if ((flagv = (word *)Malloc( i * sizeof (word))) == NULL)
	return EC_Error + EG_NoMemory;
    }
  
  for (i = 0; (str = strv[ i ]) != NULL; i++)
    {
      unless (str == (Stream *)MinInt)
	{
	  flagv[ i ] = str->Flags;
	  
	  str->Flags &= ~(Flags_CloseOnSend | Flags_OpenOnGet);
	}
    }
  
  err = SendEnv( port, env );
  
  for (i = 0; (str = strv[ i ]) != NULL; i++)
    {
      unless (str == (Stream *)MinInt) str->Flags = flagv[ i ];
    }
  
  unless (flagv == NULL)
    Free(flagv);

  return err;

} /* MySendEnv */


/* Get debug command from the debuggger */

PRIVATE word
getdebug(
	 Port 	port,
	 DBG *	dbg )
{
  MCB mcb;
  word err;


  InitMCB( &mcb, 0, port, NullPort, 0 );

#ifdef	MYDEBUG
  mcb.Timeout         = -1;
#endif  
  mcb.MsgHdr.ContSize = sizeof(DBGCMD) / sizeof(word);
  mcb.MsgHdr.DataSize = (dbg->data == NULL) ? 0 : (int)dbg->cmd.size;
  mcb.Control         = (word *)&dbg->cmd;
  mcb.Data            = dbg->data;

  while ((err = RealGetMsg( &mcb )) == EK_Timeout)
    ;
  
  dbg->port = mcb.MsgHdr.Reply;
  
#ifdef CRs
  IOdebug("getdebug %x %x %x %d %d",dbg->cmd.action, dbg->cmd.thread, 
      dbg->cmd.modnum, dbg->cmd.offset, dbg->cmd.size);
#endif
  
  return err;
}


/* Send debug message to the debugger */

PRIVATE word
putdebug(
	 Port 	port,
	 DBG *	dbg )
{
  MCB 	mcb;
  word	err;

  
  InitMCB( &mcb, MsgHdr_Flags_preserve, port, NullPort, FC_Private|FG_DebugCmd );
  
#ifdef MYDEBUG
  mcb.Timeout         = -1;
#endif 
  mcb.MsgHdr.ContSize = sizeof (DBGCMD) / sizeof (word);
  mcb.MsgHdr.DataSize = (dbg->data == NULL) ? 0 : (int)dbg->cmd.size;
  mcb.Control         = (word *)&dbg->cmd;
  mcb.Data            = dbg->data;
  
  while ((err = RealPutMsg( &mcb )) == EK_Timeout)
    ;
  
#ifdef CRs
  IOdebug("putdebug %x %x %x %d %d",dbg->cmd.action, dbg->cmd.thread, 
      dbg->cmd.modnum, dbg->cmd.offset, dbg->cmd.size);
#endif
  
  return err;
}


/* Send debug message to the debugger */

PRIVATE word
senddebug(Port port, Port reply, DBG *dbg)
{
  MCB mcb;
  word err;

  
  InitMCB(&mcb, MsgHdr_Flags_preserve, port, reply, FC_Private|FG_DebugCmd);
  
#ifdef MYDEBUG
  mcb.Timeout         = -1;
#endif  
  mcb.MsgHdr.ContSize = sizeof(DBGCMD) / sizeof(word);
  mcb.MsgHdr.DataSize = (dbg->data == NULL) ? 0 : (int)dbg->cmd.size;
  mcb.Control         = (word *)&dbg->cmd;
  mcb.Data            = dbg->data;
  
  while ((err = RealPutMsg(&mcb)) == EK_Timeout)
    ;
  
#ifdef CRs
  IOdebug("senddebug %x %x %x %d %d",dbg->cmd.action, dbg->cmd.thread, 
      dbg->cmd.modnum, dbg->cmd.offset, dbg->cmd.size);
#endif

  return err;
}


/* Record new activation record for current thread */

PRIVATE void
pushframe(
	  MPtr		procinfo,
	  WordPtr	frame_pointer,
	  WordPtr	stack_pointer )
{
  /* ACE: Should be able to take the test out at some stage */

  unless (thread.framestk == NULL)
    {
      thread.frameptr--;
      
      /* ACE: ! */

      if (thread.frameptr < thread.framestk OR
	  thread.frameptr > thread.framestk + FrameStackSize)
	IOdebug("DLIB: push: Frame pointer (%x) out of range (%x %x)",
		thread.frameptr, thread.framestk, thread.framestk + FrameStackSize);
      
      thread.frameptr->procinfo = procinfo;
      thread.frameptr->frameptr = frame_pointer;
      thread.frameptr->stackptr = stack_pointer;
    }
}


/* Record termination of activation record for current thread */

PRIVATE void
popframe( void )
{
  unless (thread.framestk == NULL)
    {
      thread.frameptr++;
      
      /* ACE: ! */
      
      if (thread.frameptr < thread.framestk OR
	  thread.frameptr > thread.framestk + FrameStackSize)
	IOdebug("DLIB: pop: Frame pointer (%x) out of range (%x %x)",
		thread.frameptr, thread.framestk, thread.framestk + FrameStackSize);
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
PRIVATE void
profile(
	word 	modnum,
	word	line )
{
  modnum = modnum;
  line   = line;

  IOdebug( "profile ???" );

  return;  
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
  dbg.data       = NULL;

  Wait(&debug->lock); /* ACE: This should not be necessary */

#ifdef V1_1
/* remove all tempory breakpoints set by go until */
  (void)WalkList(&debug->breakpointlist, (WordFnPtr)remtempbreak, 0);
#endif
  
  putdebug(debug->port, &dbg);
  
  Signal(&debug->lock);

  suspend( &thread );

  return;
}

/**
*
* found = cmpbreak(breakpoint, loc);
*
* Support routine for findbreak();
*
**/
PRIVATE BOOL
cmpbreak(BREAKPOINT *breakpoint, LOCATION *loc)
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
PRIVATE BREAKPOINT *
findbreak(word modnum, word line)
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
* breakpoint(modnum, line);
*
* Breakpoint command.
*
**/
PRIVATE BOOL
breakpoint(word modnum, word line)
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

PRIVATE void
trace(
      word 	modnum,
      word	line )
{
  DBG 		dbg;
#ifndef BUG
  Port		port = NewPort();
  word		err;
#endif

  
  dbg.cmd.action = DBG_Traced;
  dbg.cmd.thread = (word)&thread;
  dbg.cmd.modnum = modnum;
  dbg.cmd.offset = line;
  dbg.data       = NULL;
  
  Wait( &debug->lock ); /* ACE: This should not be necessary */
  
#ifdef BUG
  putdebug( debug->port, &dbg );
#else
  
  if ((err = senddebug( debug->port, port, &dbg )) == Err_Null)
    {
      getdebug( port, &dbg );
    }
  else
    {
      IOdebug( "DLIB: senddebug returned an error (%x)", err );
    }
  
  FreePort( port );
#endif
  
  Signal( &debug->lock );

  return;
  
} /* trace */

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
PRIVATE void
delwatchpoints ()
{
  DBG dbg;
  word err;
  Port port = NewPort();
  
#ifdef CRs
  IOdebug ("entered delwatchpoints");  
#endif

  dbg.port       = port;
  dbg.cmd.action = DBG_DelWatchIds;
  dbg.data       = NULL;

  if ((err = senddebug(debug->port, port, &dbg)) == Err_Null)
    {
      getdebug ( port, &dbg);
    }

  FreePort (port);

  return;
  
} /* delwatchpoints */

/**
*
* checkwatch(watchpoint);
*
* Check a watchpoint to see if it has been activated.
*
**/
PRIVATE void
checkwatch(
	   WATCHPOINT * watchpoint,
	   word	        scope )
{
  unless (memcmp( watchpoint->loc.addr, &watchpoint->copy, (int)watchpoint->loc.size ) == 0)
    {
      DBG dbg;
      Port port = NewPort();  
      word err;

      
#ifdef CRs
      IOdebug ("entered checkwatch");  
#endif
      dbg.port       = port;
      dbg.cmd.action = DBG_Changed;
      dbg.cmd.modnum = (word)watchpoint->loc.addr;
      dbg.cmd.size   = watchpoint->loc.size;
      dbg.cmd.offset = scope ;/* CR: should transfer the callers id */
      dbg.data       = NULL;
      
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
	  getdebug( port, &dbg );
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
      else
	IOdebug( "DLIB: senddebug returned an error (%x)", err);
      
      FreePort(port);
      
      memcpy(&watchpoint->copy, watchpoint->loc.addr, (int)watchpoint->loc.size);
      
#ifdef OLDCODE
      thread.watchstop = TRUE;
#endif
    }

  return;
  
} /* checkwatch */

/**
*
* checkwatchret(watchpoint);
*
* notify debugger of changed frame
*
**/

PRIVATE int
checkwatchret(
	      WATCHPOINT * watchpoint,
	      word         scope )
{
  DBG  dbg;
  word err;
  Port port = NewPort();

  
#ifdef CRs
  IOdebug( "entered checkwatchreturn" );  
#endif

  dbg.port       = port;
  dbg.cmd.action = DBG_Changed;
  dbg.cmd.modnum = (word)watchpoint->loc.addr;
  dbg.cmd.size   = watchpoint->loc.size;
  dbg.cmd.offset = scope;		/* CR: should transfer the callers id */
  dbg.data       = NULL;

  if ((err = senddebug( debug->port, port, &dbg )) == Err_Null)
    {
      getdebug ( port, &dbg );
    }

  FreePort( port );

  return TRUE;
  
} /* checkwatchret */


PRIVATE word
hashval( MPtr procinfo )
{
  return (uword)procinfo % HashMax;
}

PRIVATE int
cmpfunc(
	FUNC *	func,
	MPtr	procinfo )
{
  return func->procinfo == procinfo;
}

PRIVATE FUNC *
findfunc( MPtr procinfo )
{
  return (FUNC *)SearchList( &debug->functable[ hashval( procinfo ) ], (WordFnPtr)cmpfunc, procinfo );
}

PRIVATE void
remfunc( FUNC * func )
{
  Remove(&func->node);
  
  Free(func);
}


PRIVATE void
entered( MPtr procinfo )
{
  DBG dbg;

  
  dbg.cmd.action = DBG_Entered;
  dbg.cmd.thread = (word)&thread;
  dbg.cmd.modnum = GetProcInfo( procinfo, Modnum );
  dbg.cmd.offset = GetProcInfo( procinfo, Offset );
  dbg.data       = NULL;

  Wait( &debug->lock ); /* ACE: This should not be necessary */
  
  putdebug( debug->port, &dbg );

  Signal( &debug->lock );

  return;
  
} /* entered */


PRIVATE void
returned( MPtr procinfo )
{
  DBG dbg;

  
  dbg.cmd.action = DBG_Returned;
  dbg.cmd.thread = (word)&thread;
  dbg.cmd.modnum = GetProcInfo( procinfo, Modnum );
  dbg.cmd.offset = GetProcInfo( procinfo, Offset );
  dbg.data       = NULL;
  
  Wait( &debug->lock ); /* ACE: This should not be necessary */
  
  putdebug( debug->port, &dbg );
  
  Signal( &debug->lock );

  return;
  
} /* returned */


/* Standard notify entry function */

PRIVATE void
notify_entry(
	     MPtr	procinfo,
	     MPtr	frame_ptr,
	     MPtr	stack_ptr )
{
  FUNC *	func;
  word		flags = thread.flags;
  WordPtr	fptr;
  WordPtr	sptr;
  

  Wait( &debug->lock );
  
  (void)WalkList( &debug->watchpointlist, (WordFnPtr)checkwatchret, 1 );
  
  Signal( &debug->lock );

#if defined CODE_IN_HIGH_MEMORY && defined NEVER
  fptr  = frame_ptr;
  sptr  = stack_ptr;  
#else
  fptr  = (WordPtr)C40CAddress( frame_ptr );
  sptr  = (WordPtr)C40CAddress( stack_ptr );
#endif

  unless ((func = findfunc( procinfo )) == NULL)
    flags = func->flags;
  
  pushframe( procinfo, fptr, sptr );

  if (flags & TraceEntry)
    {
      entered( procinfo );
    }

  thread.tracing   = (BOOL)(flags & TraceCommand);
  thread.profiling = (BOOL)(flags & Profile);

  return;
  
} /* notify_entry */


/* Standard notify return function */

PRIVATE word
notify_return(
	      MPtr	procinfo,
	      word	result )
{
  FUNC *	func;
  word		flags;
  

  /* ACE: Need a more efficient way of doing this */
  
  Wait( &debug->lock );
  
  (void)WalkList( &debug->watchpointlist, (WordFnPtr)checkwatchret, -1 );
  
  Signal( &debug->lock );

  delwatchpoints () ; /* -- crf */

  unless ((func = findfunc( procinfo )) == NULL)
    flags = func->flags;
  else
    flags = thread.flags;
  
  if (flags & TraceReturn)
    {
      returned( procinfo );
    }
  
  popframe();
  
  flags = thread.flags;
  
  unless ((func = findfunc( thread.frameptr->procinfo )) == NULL)
    flags = func->flags;

  thread.tracing   = (BOOL)(flags & TraceCommand);
  thread.profiling = (BOOL)(flags & Profile);

  return result;

} /* notify_return */


/* Standard notify command function */

PRIVATE void
notify_command(
	       word 	line,
	       MPtr	sourceinfo )
{
  word 		modnum;
  WordPtr	sinfo;


#ifdef CODE_IN_HIGH_MEMORY
  sinfo = sourceinfo;
#else
  sinfo = (WordPtr)C40CAddress( sourceinfo );
#endif

  /* sanity check */
  
  if (MP_StructGet( SourceInfo, Type, sinfo ) != T_SourceInfo)
    {
      IOdebug( "DLIB: Corrupt object file: could not find SourceInfo structure (line %d)", line );

      return;
    }
  
  modnum = MP_StructGet( SourceInfo, Modnum, sinfo );

  thread.frameptr->line = line;

  if (thread.profiling)
    {
      profile( modnum, line );
    }
  
#ifndef OLDCODE
  Wait( &debug->lock );
  
  (void)WalkList( &debug->watchpointlist, (WordFnPtr)checkwatch, 0 );
  
  Signal( &debug->lock );
#endif

  if (thread.stopping)
    {
      stop( modnum, line );
    }  
  else
    {
      unless (breakpoint( modnum, line ))
	{
#ifdef OLDCODE
	  Wait( &debug->lock );

	  (void)WalkList( &debug->watchpointlist, (WordFnPtr)checkwatch, 0 );

	  Signal( &debug->lock );
#endif
	  if (thread.tracing)
	    {
	      trace( modnum, line );
	    }	  
	}
    }
  
  return;
  
} /* notify_command */


PRIVATE void
ignore_entry(
	     MPtr	procinfo,
	     MPtr	frame_ptr,
	     MPtr	stack_ptr )
{
  return;  
}

PRIVATE word
ignore_return(
	      MPtr	procinfo,
	      word      result)
{
  return result;
}

PRIVATE void
ignore_command(
	       word 	line,
	       MPtr	sourceinfo )
{
  return;  
}

PRIVATE void
framestop_command(
		  word 	line,
		  MPtr	sourceinfo )
{
  word 	modnum;
#ifdef CODE_IN_HIGH_MEMORY

#define source	sourceinfo

#else
  WordPtr	source;


  source = (WordPtr)C40CAddress( sourceinfo );
#endif

  modnum = MP_StructGet( SourceInfo, Modnum, source );

  thread.frameptr->line = line;
  
  if (thread.profiling)
    profile( modnum, line );
  
  Wait( &debug->lock );
  
  (void)WalkList( &debug->watchpointlist, (WordFnPtr)checkwatch, 0 );
  
  Signal( &debug->lock );
  
  if (thread.frameptr >= thread.stopframe)
    stop( modnum, line );
  else if (thread.tracing)
    {
      trace( modnum, line );
    }

  return;
  
} /* framestop_command */


/* Intialise a thread structure */

PRIVATE void
initthread( THREAD * thread )
{
  Wait( &debug->lock );

  AddHead( &debug->threadlist, &thread->node );

  Signal( &debug->lock );

  InitSemaphore( &thread->sync, 0 );
  
  if ((thread->framestk = (FRAME *)Malloc( FrameStackSize * sizeof (FRAME) )) == NULL)
    {
      IOdebug("DLIB: Failed to allocate frame stack");
    }  

  thread->watchstop	 = FALSE;
  thread->frameptr       = thread->framestk + FrameStackSize;
  thread->stopframe	 = NULL;  
  thread->profiling      = FALSE;
  thread->stopping       = TRUE;
  thread->tracing        = FALSE;
#ifdef NEWCODE
  thread->watching       = FALSE;
#endif
  thread->port           = NullPort;
  thread->flags		 = 0;  
  
  thread->notify_entry   = notify_entry;
  thread->notify_command = notify_command;
  thread->notify_return  = notify_return;

  return;  

} /* initthread */


/* Remove a thread */

PRIVATE void
remthread( THREAD * thread )
{
  Remove( &thread->node );
  
  unless (thread->framestk == NULL)
    Free(thread->framestk);

  return;
  
} /* remthread */

/**
*
* resume(thread);
*
* Resume execution of a suspended thread.
*
**/
PRIVATE void
resume( THREAD * thread )
{
  if (TestSemaphore( &thread->sync ) < 0)
    Signal( &thread->sync );

  return;
  
} /* resume */

/**
*
* profilethread(thread, on);
*
* Set profile flag.
*
**/
PRIVATE void
profilethread(
	      THREAD *	thread,
	      BOOL	on )
{
  if (on)
    thread->flags |= Profile;
  else
    thread->flags &= ~ Profile;

  return;
  
} /* profile */

/**
*
* stopthread(thread);
*
* Set stop flag.
*
**/
PRIVATE void
stopthread( THREAD * thread )
{
  thread->stopping = TRUE;

  return;
  
} /* stopthread */

/**
*
* unstopthread(thread);
*
* Unset stop flag.
*
**/
PRIVATE void
unstopthread( THREAD * thread )
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
PRIVATE void
tracethread(
	    THREAD *	thread,
	    word	flags )
{
  if (flags & TraceOff)
    thread->flags &= ~flags;
  else
    thread->flags |= flags;

  if (thread->flags & TraceCommand)
    thread->tracing = TRUE;
  else
    thread->tracing = FALSE;

  return;
}

/**
*
* gotoframethread(thread, frame);
*
* Stop a thread when it is in a particular frame.
*
**/
PRIVATE void
gotoframethread(
		THREAD * thread,
		int      frame )
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
PRIVATE void
timeoutthread( THREAD * thread )
{
  unless (thread->port == NullPort)
    AbortPort( thread->port, EK_Timeout );
}


/* Patch the notify entry routine */

PRIVATE void
patchentry(
	   THREAD *	thread,
	   void (*	func)( MPtr, MPtr, MPtr ) )
{
  thread->notify_entry = func;
}


/* Patch the notify command routine */

PRIVATE void
patchcommand(
	     THREAD *	thread,
	     void (*	func)( word, MPtr ) )
{
  thread->notify_command = func;
}


/* Patch the notify return routine */

PRIVATE void
patchreturn(
	    THREAD *	thread,
	    word (*	func)( MPtr, word ) )
{
  thread->notify_return = func;
}


/* Create a new module table */

PRIVATE word *
newmodtab( void )
{
  MPtr		oldtab;
  WordPtr	module;  
  word		maxid      = 0;
  word		maxdata    = 0;
  word *	newtab;
  void *	ptr;
  

  oldtab = get_module_table();
  
  module = (MPtr) MyTask->Program;	/* XXX relies on Module field being first in Program structure */
  
  while (MP_StructGet( Module, Type, module ) != 0)
    {
      word	id;

      
      id = MP_StructGet( Module, Id, module );
      
      if (id > maxid) maxid = id;
      
      if (id == DLib_Slot)
	{
	  MPtr	mod;


	  if (maxdata != 0)
	    IOdebug( "DLIB: more than one debugger library present !" );
	  
	  mod     = (MPtr)MP_StructGet( ResRef, Module, module );	  
	  maxdata = MP_StructGet( Module, MaxData,  mod );
	}
      
      module += MP_StructGet( Module, Size, module ) / sizeof (word);
    }
  
  if (maxid < 1)
    return NULL;
  else
    maxid = (maxid + 1) * 2;

  /* allocate a new module table */
  
  if ((newtab = (word *)Malloc( maxid * sizeof (word) + 100000 )) == NULL)
    return NULL;

  /* copy the old module table into the new one */
  
  MP_memcpy( newtab, oldtab, 0, maxid );

  /* set up the self-referential vectors */

  newtab[ 0 ] = (word)newtab;
  newtab[ 1 ] = C40WordAddress( newtab );

  /* copy the debug library's data structures */
  
  if ((ptr = Malloc( maxdata * sizeof (word) )) == NULL)
    {
      Free( newtab );
      
      return NULL;
    }

  memcpy( ptr, (char *)MP_GetWord( oldtab, DLib_Slot * 2 ), (int)maxdata * sizeof (word) );

  newtab[ DLib_Slot * 2 ] = (word)ptr;
  
  /* initialise the thread with this new module table */
  
  /* NB/ this relies upon the thread structure being the first structure declared in dlibstar.a */
  
  initthread( (THREAD *)ptr );

  return newtab;

} /* newmodtab */ 


PRIVATE void
copy_module_table( void )
{
  MPtr		module;
  MPtr		wptr;  
  word		kernelfunc = 0;
  word		utilfunc   = 0;
  void *	ptr;
  

  module = (MPtr) MyTask->Program;	/* XXX relies on Module field being first in Program structure */
  
  while (MP_StructGet( Module, Type, module ) != 0)
    {
      word	id;

      
      id = MP_StructGet( Module, Id, module );
      
#define KERNEL_SLOT	1
      
      if (id == KERNEL_SLOT)
	{
	  MPtr	mod;


	  mod        = (WordPtr)MP_StructGet( ResRef, Module,   module );	  
	  kernelfunc = MP_StructGet( Module, MaxCodeP, mod );
	}
      
#define UTIL_SLOT	4
      
      if (id == UTIL_SLOT)
	{
	  MPtr	mod;

	  
	  mod      = (WordPtr)MP_StructGet( ResRef, Module,   module );	  
	  utilfunc = MP_StructGet( Module, MaxCodeP, mod );
	}
      
      module += MP_StructGet( Module, Size, module ) / sizeof (word);
    }

  /* we must copy the kernel's codetable area */
  
  if ((ptr = Malloc( kernelfunc * sizeof (word) )) == NULL)
    {
      IOdebug( "DLIB: Out of Memory" );
      
      return;
    }

  wptr = (MPtr) MP_GetWord( get_module_table(), KERNEL_SLOT * 2 + 1 );

  MP_memcpy( ptr, wptr, 0, kernelfunc );

  MP_PutWord( get_module_table(), KERNEL_SLOT * 2 + 1, C40WordAddress( ptr ) );
  
  /* we must copy Util's codetable area */
  
  if ((ptr = Malloc( utilfunc * sizeof (word) )) == NULL)
    {
      IOdebug( "DLIB: Out of Memory" );
      
      return;
    }

  wptr = (MPtr) MP_GetWord( get_module_table(), UTIL_SLOT * 2 + 1 );

  MP_memcpy( ptr, wptr, 0, utilfunc );

  MP_PutWord( get_module_table(), UTIL_SLOT * 2 + 1, C40WordAddress( ptr ) );
  
  return;

} /* copy_module_table */ 


/*
 * Despite compiler messages to the contrary, this
 * function IS used.  It is called by _DBGProcExit()
 * in the dlibasm.a file.
 */

PRIVATE void
ProcStop( MPtr modtab ) 
{
  word *	mtab;

  
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
      dbg.data       = NULL;

      putdebug( debug->port, &dbg );
    }
  
  remthread( &thread );

  mtab = (word *)C40CAddress( modtab );
  
  Free( (void *)mtab[ DLib_Slot ] );
  Free( mtab );

  return;
  
} /* ProcStop */


/* A private version of _ProcExit() */
/* Original is in util/misc.c       */

PRIVATE void
_DBGProcExit( word * stackbase )
{
  ProcStop( get_module_table() );
  FreeStop( stackbase );
  return;  
}
  

/* A private version of NewProcess() */
/* Original is in util/misc.c        */

PRIVATE word *
DBGNewProcess(
	      word 	stacksize,
	      VoidFnPtr func,
	      word	argsize )
{
  word *	stack;  
  word *	display;


  stack   = (word *)Malloc( stacksize + sizeof (SaveState) );
  display = stack + ((stacksize + sizeof (SaveState)) / sizeof (word)) -
#ifdef NEW_SYSTEM
    3
#else
    2
#endif
      ;
  
  if (stack == NULL)
    return NULL;

  display[ 0 ] = (word)newmodtab();
  
  if (display[ 0 ] == NULL)
    {
      Free( stack );
      
      return NULL;
    }
  
  display[ 1 ] = (word)stack;
  
  if (argsize < 8) argsize = 8;


#ifdef NEW_SYSTEM
  display[ 2 ] = argsize;
  
  return InitProcess( stack, func, _DBGProcExit, display );
#else
  return InitProcess( display, func, _DBGProcExit, display, argsize );
#endif
} /* DBGNewProcess */


/* private version of Fork()  */
/* original is is util/misc.c */

PRIVATE word
DBGFork(
	word 		stacksize,
	VoidFnPtr	func,
	word		argsize,
	...		)
{
  word *		proc;
  word *		ptr;
  va_list		argp;
  word			i;


  va_start( argp, argsize );

  proc = DBGNewProcess( stacksize, func, argsize );
  
  if (proc == NULL)
    return FALSE;
  
  ptr = proc;
  
  for (i = (argsize / sizeof (word)); i--;)
    {
      *ptr++ = va_arg( argp, word );
    }
  
  va_end( argp );
  
  StartProcess( proc, StandardPri );
  
  return TRUE;
  
} /* DBGFork */


/* Add a breakpoint */

#ifdef V1_1
PRIVATE void
addbreak(
	 word 	modnum,
	 word	line,
	 word	threshold,
	 BOOL	temp )
#else
PRIVATE void addbreak(word modnum, word line, word threshold)
#endif
{
  BREAKPOINT *	breakpoint;
  
  
  if ((breakpoint = New (BREAKPOINT)) == NULL)
    return;
  
  breakpoint->loc.modnum = modnum;
  breakpoint->loc.line   = line;
  breakpoint->threshold  = threshold;
  breakpoint->count      = 0;
#ifdef V1_1
  /* set a variable if this has be called by a go until function */
  breakpoint->temp       = temp;
#endif
  
  Wait( &debug->lock );
  
  AddHead( &debug->breakpointlist, &breakpoint->node );
  
  Signal( &debug->lock );
  
  return;
  
} /* addbreak */
  

PRIVATE void
rembreak( BREAKPOINT * breakpoint )
{
  Remove( &breakpoint->node );
  
  Free( breakpoint );

  return;
  
} /* rembreak */
  
  
/**
 *
 * found = cmpwatch(watch, loc);
 *
 * Support routine for findwatch().
 *
 **/
PRIVATE BOOL
cmpwatch(
	 WATCHPOINT *	watchpoint,
	 MEMLOCATION *	loc )
{	 
  return (watchpoint->loc.addr == loc->addr AND
	  watchpoint->loc.size == loc->size);
  
} /* cmpwatch */
  
/**
 *
 * watchpoint = findwatch(addr, size);
 *
 * Find a watchpoint.
 *
 **/
PRIVATE WATCHPOINT *
findwatch(
	  void *	addr,
	  word		size )
{
  WATCHPOINT *		watchpoint;
  MEMLOCATION		loc;
    
    
  loc.addr = addr;
  loc.size = size;
    
  watchpoint = (WATCHPOINT *)SearchList( &debug->watchpointlist, (WordFnPtr)cmpwatch, (word)&loc );
    
  return watchpoint;
  
} /* findwatch */
  
/**
 *
 * addwatch(addr, size);
 *
 * Add a watchpoint.
 *
 **/
PRIVATE void
addwatch(
	 void * addr,
	 word   size )
{
  WATCHPOINT *  watchpoint;
  
  
  Wait( &debug->lock );

  if ((watchpoint = findwatch( addr, size )) == NULL)
    {
      unless ((watchpoint = (WATCHPOINT *)Malloc( sizeof (WATCHPOINT) + size)) == NULL)
	{
	  watchpoint->loc.addr = addr;
	  watchpoint->loc.size = size;
	  watchpoint->usage    = 1;

	  memcpy( &watchpoint->copy, addr, (int)size );
	  
	  AddHead( &debug->watchpointlist, &watchpoint->node );
	}
    }
  else
    {
      watchpoint->usage++;
    }
  
  Signal( &debug->lock );
  
  return;
  
} /* addwatch */
  
/**
 *
 * remwatch(watchpoint);
 *
 * Remove a watchpoint.
 *
 **/
PRIVATE void
remwatch( WATCHPOINT * watchpoint )
{
#ifdef CRs
  IOdebug ("remwatch %x", watchpoint);
#endif
  
  if (watchpoint->usage == 1)
    {
      Remove( &watchpoint->node );
      
      Free( watchpoint );
    }
  else
    watchpoint->usage --;

  return;
  
} /* remwatch */
  
/**
 *
 * same = memcmp(m1, m2, size);
 *
 * Compare two blocks of memory.
 *
 **/
int
memcmp(
       const void * m1,
       const void * m2,
       unsigned int size )
{
  byte *b1 = (byte *)m1;
  byte *b2 = (byte *)m2;
  
  while (size--)
    unless (*b1++ == *b2++) return 1;
  
  return 0;

} /* memcmp */
  

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
  
  
/* return the address of global data of the indicated module */
PRIVATE byte *
dataloc(
	word 	modnum,
	word	offset )
{
  return (byte *)(MP_GetWord( get_module_table(), modnum * 2 ) + offset );
}
  
/* return the address of a function of the indicated module */
  
PRIVATE MPtr
codeloc(
	word 	modnum,
	word	offset )
{
  return (MPtr)(MP_GetWord( get_module_table(), modnum * 2 + 1 ) + offset );
}
  
/* returns the address of an item on the thread's stack */
  
PRIVATE WordPtr
stackloc(
	 THREAD *	thread,
	 word		frame,
	 word		offset )
{
  return thread->frameptr[ frame ].stackptr - (offset);
    
} /* stackloc */
  
  
PRIVATE WordPtr
frameloc(
	 THREAD *	thread,
	 word		frame,
	 word		offset )
{
  return thread->frameptr[ frame ].frameptr + (offset / sizeof (word));
    
} /* frameloc */
  
  
PRIVATE void
peekmem(
	Port	port,
	byte *	addr,
	word	size )
{
  DBG dbg;
    

  dbg.cmd.action = DBG_Dump;
  dbg.cmd.modnum = (word)addr;
  dbg.cmd.size   = size;
  dbg.data       = addr;
    
  /* IOdebug( "DLIB: peekmem: addr = %x, size = %d", addr, size ); */
    
  putdebug(port, &dbg);

  return;
  
} /* peekmem */
  
  
PRIVATE void
pokemem(
	byte *	addr,
	word	size,
	byte *	data )
{
  /* IOdebug( "DLIB: pokemem: addr = %x, data = %x", addr, *((word *)data) ); */
    
  memcpy( addr, data, (int)size );
    
  return;
  
} /* pokemem */
  
  
PRIVATE void
peekdata(
	 Port 	port,
	 word	modnum,
	 word	offset,
	 word	size )
{
  DBG 		dbg;
  byte *	addr = dataloc( modnum, offset );
    
    
  /* IOdebug( "DLIB: peekdata: addr = %x, offset = %x, modnum = %x, size = %x", addr, offset, modnum, size ); */
    
  dbg.cmd.action = DBG_Dump;
  dbg.cmd.modnum = (int)addr;
  dbg.cmd.size   = size;
  dbg.data       = addr;
    
  putdebug( port, &dbg );
    
  return;
  
} /* peekdata */
  

PRIVATE void
pokedata(
	 word 	modnum,
	 word	offset,
	 word	size,
	 byte *	data )
{
  /* IOdebug( "DLIB: pokedata: addr = %x, data = %x", dataloc( modnum, offset ), *((word *)data) ); */
    
  memcpy( dataloc( modnum, offset ), data, (int)size );
  
  return;  
    
} /* pokedata */
  
  
PRIVATE void
peekstack(
	  Port 		port,
	  THREAD *	thread,
	  word		frame,
	  word		offset,
	  word		size )
{
  DBG 		dbg;
  WordPtr	addr = stackloc( thread, frame, offset );
    
  
  dbg.cmd.action = DBG_Dump;
  dbg.cmd.modnum = (word)addr;
  dbg.cmd.size   = size;
  dbg.data       = (byte *)addr;
  
  /* IOdebug( "DLIB: peekstack: addr = %x", addr ); */
  
  putdebug( port, &dbg );
  
  return;
    
} /* peekstack */
  
  
PRIVATE void
pokestack(
	  THREAD *	thread,
	  word		frame,
	  word		offset,
	  word		size,
	  byte *	data )
{
  /* IOdebug( "DLIB: pokestack: addr = %x, data = %x", stackloc( thread, frame, offset ), *((word *)data) ); */
    
  memcpy( (char *)stackloc( thread, frame, offset ), data, (int)size );
    
  return;
    
} /* pokestack */
  
  
PRIVATE void
locatedata(
	   Port 	port,
	   word		modnum,
	   word		offset )
{
  DBG 			dbg;
    
    
  /* IOdebug( "DLIB: locatedata: offset = %x", offset ); */
  
  dbg.cmd.action = DBG_Address;
  dbg.cmd.modnum = (int)dataloc( modnum, offset );
  dbg.data       = NULL;
  
  putdebug( port, &dbg );
  
  return;
  
} /* locatedata */
  
  
PRIVATE void
locatestack(
	    Port 	port,
	    THREAD *	thread,
	    word	frame,
	    word	offset )
{
  DBG 			dbg;
    
    
  /* IOdebug( "DLIB: locatestack: offset = %x", offset ); */
    
  dbg.cmd.action = DBG_Address;
  dbg.cmd.modnum = (word)stackloc( thread, frame, offset );
  dbg.data       = NULL;
  
  putdebug( port, &dbg );
  
  return;
  
} /* locatestack */
  

PRIVATE void
locateframe(
	      Port 	port,
	      THREAD *	thread,
	      word	frame,
	      word	offset )
{
  DBG dbg;
    
    
  /* IOdebug( "DLIB: locateframe: offset = %x", offset ); */
  
  dbg.cmd.action = DBG_Address;
  dbg.cmd.modnum = (word)frameloc( thread, frame, offset );
  dbg.data       = NULL;
  
  putdebug( port, &dbg );
  
  return;
  
} /* locateframe */
  
  
PRIVATE void
locateregister(
	       Port 	port,
	       THREAD *	thread,
	       word	reg )
{
  DBG dbg;
    
  /* IOdebug( "DLIB: locateregister: reg = %d", reg ); */
    
  dbg.cmd.action = DBG_Address;
  dbg.cmd.modnum = (int)(&saved_regs[ reg ]);
  dbg.data       = NULL;
    
  putdebug( port, &dbg );
    
  return;
    
} /* locateregister */
  

PRIVATE void
call(
     Port 	port,
     VoidFnPtr	func,
     word	argsize,
     word *	args,
     word	retsize )
{    
  DBG 		dbg;
  word		rets;
    
  
  /*
   * ACE: Only support functions return single word
   */
    
  rets = DoCall( func, argsize, args );
  
  dbg.cmd.action = DBG_Return;
  dbg.cmd.size   = retsize;
  dbg.data       = (byte *)&rets;
  
  putdebug( port, &dbg );
  
  return;
  
} /* call */
  
  
PRIVATE void
where(
      Port 	port,
      THREAD *	thread,
      word	frame )
{
  DBG dbg;
    
    
  dbg.cmd.action = DBG_Position;
  
  if (thread->frameptr + frame >= thread->framestk + FrameStackSize)
    {
      dbg.cmd.modnum = -1;
    }
  else
    {
      MPtr	procinfo = thread->frameptr[ frame ].procinfo;
      
      
      dbg.cmd.modnum = GetProcInfo( procinfo, Modnum );
      dbg.cmd.offset = thread->frameptr[ frame ].line;
      dbg.cmd.size   = GetProcInfo( procinfo, Offset );
    }
  
  dbg.data = NULL;
  
  putdebug( port, &dbg );
  
  return;
  
} /* where */
  
  
/* returns the address of the ProcInfo structure associated with the given procedure */
  
PRIVATE MPtr
findprocinfo(
	     word 	modnum,
	     word	offset )
{
  MPtr		code;
  word		i;
    
    
  code = MP_GetWord( codeloc( modnum, offset ), 0 );  
    
  i = 0;
    
  while (MP_GetWord( code, i ) != T_ProcInfo)	/* XXX - could flow off end of Code space ! */
    i++;
    
  code += i;
  
  if (GetProcInfo( code, Offset ) == offset &&
      GetProcInfo( code, Modnum ) == modnum)
    {
      return code;
    }
    
  return NULL;
    
} /* findprocinfo */
  
  
PRIVATE FUNC *
addfunc( MPtr procinfo )
{
  FUNC *	func;
  word		hash = hashval( procinfo );
  
  
  if (procinfo == 0)
    return NULL;  
  
  if ((func = (FUNC *)SearchList( &debug->functable[ hash ], (WordFnPtr)cmpfunc, procinfo )) == NULL)
    {
      /* ACE: Test for no memory */
      
      func = New(FUNC);
      
      func->procinfo = procinfo;
      func->flags    = 0;
      func->time     = 0;
      func->calls    = 0;
      
      AddHead( &debug->functable[ hash ], &func->node );
    }
  
  return func;
  
} /* addfunc */


PRIVATE word
DBGGetMsg( MCB * mcb )
{
  word err;

  
  thread.port  = mcb->MsgHdr.Dest;

  /*
   * XXX - NC - 27/4/93
   *
   * Do NOT set the timeout to -1.
   * This used to be done, (because in theory
   * using the debugger will alter the timing
   * characteristics of the thread).  Doing
   * this breaks signle stepping over sleep(),
   * amoungst other things, and so it has been
   * abandoned.
   */
  
  err          =  RealGetMsg(mcb);
  thread.port  = NullPort;
  
  return err;

} /* DBGGetMsg */

  
PRIVATE word
DBGPutMsg( MCB * mcb )
{
  word err;
    

  thread.port  = mcb->MsgHdr.Dest;
  
  /*
   * XXX - NC - 27/4/93
   *
   * Do NOT set the timeout to -1.
   * This used to be done, (because in theory
   * using the debugger will alter the timing
   * characteristics of the thread).  Doing
   * this breaks signle stepping over sleep(),
   * amoungst other things, and so it has been
   * abandoned.
   */
  
  err          =  RealPutMsg(mcb);
  thread.port  = NullPort;

  return err;
  
} /* DBGPutMsg */


/**
 *
 * remdebug();
 *
 * Terminate debugging.
 *
 **/

PRIVATE void
remdebug( void )
{
  unless (debug == NULL)
      {
	int i;
	
	
	Wait(   &debug->lock );
	Signal( &debug->lock );
	
	(void) patchgetmsg( debug->GetMsg );
	(void) patchputmsg( debug->PutMsg );
#if 1
	(void) patchfork( debug->Fork );
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
	
	/*    (void)WalkList(&debug->watchpointlist, (WordFnPtr)remwatch, 0);*/
	
	for (i = 0; i < HashMax; i++)
	  (void)WalkList( &debug->functable[ i ], (WordFnPtr)remfunc, 0 );
	
	Close( debug->stream );
	
	Free( debug );
	
	debug = NULL;
      }
  
  return;
  
} /* remdebug */

  
/**
 *
 * debugworker();
 *
 * Accepts and obeys debug commands from debugger.
 *
 **/
PRIVATE void
debugworker( void )
{
  DBG 	dbg;
  byte	data[ 1024 ]; /* ACE: not perfect */
  
  
  dbg.cmd.size = 1024;
  dbg.data     = data;
  
  while (getdebug(debug->reply, &dbg) >= Err_Null)
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
	  
	  patchentry(   &thread, ignore_entry   );
	  patchcommand( &thread, ignore_command );
	  patchreturn(  &thread, ignore_return  );
	  
	  call( dbg.port, (VoidFnPtr)dbg.cmd.modnum, dbg.cmd.size, (word *)dbg.data, dbg.cmd.offset );
	  
	  patchentry(   &thread, notify_entry   );
	  patchcommand( &thread, notify_command );
	  patchreturn(  &thread, notify_return  );
	  
	  continue;
	  
	case DBG_Fork:
	  /*
	    dbg.cmd.modnum = address of function
	    dbg.cmd.size   = size of parameters
	    dbg.cmd.offset = size of return parameters expected
	    dbg.data       = parameters
	    */
	    {
	      word * proc = DBGNewProcess(10000, (VoidFnPtr)dbg.cmd.modnum, dbg.cmd.size);
	      
	      unless (proc == NULL)
		{
		  memcpy(proc, dbg.data, (int)dbg.cmd.size);
		  StartProcess(proc, 1);
		}
	    }
	  continue;
	  
	case DBG_Free:
	  patchentry(   (THREAD *)dbg.cmd.thread, ignore_entry   );
	  patchcommand( (THREAD *)dbg.cmd.thread, ignore_command );
	  patchreturn(  (THREAD *)dbg.cmd.thread, ignore_return  );
	  
	  resume((THREAD *)dbg.cmd.thread);
	  
	  continue;
	  
	case DBG_FreeAll:
	  (void)patchgetmsg(debug->GetMsg);
	  (void)patchputmsg(debug->PutMsg);
#if 1
	  (void)patchfork(debug->Fork);
#endif
	  Wait(&debug->lock);
	  (void)WalkList(&debug->threadlist, (WordFnPtr)patchentry,   (word)ignore_entry);
	  (void)WalkList(&debug->threadlist, (WordFnPtr)patchcommand, (word)ignore_command);
	  (void)WalkList(&debug->threadlist, (WordFnPtr)patchreturn,  (word)ignore_return);
	  (void)WalkList(&debug->threadlist, (WordFnPtr)resume, 0);
	  Signal(&debug->lock);
#ifdef CRs
	  IOdebug (" remdebug called from debugworker" );
#endif    
	  remdebug();
	  return;
	  
	case DBG_Go:
	  unstopthread((THREAD *)dbg.cmd.thread);
	  patchcommand((THREAD *)dbg.cmd.thread, notify_command);
	  resume((THREAD *)dbg.cmd.thread);
	  continue;
	  
	case DBG_GoAll:
	  Wait(&debug->lock);
	  (void)WalkList(&debug->threadlist, (WordFnPtr)unstopthread, 0);
	  (void)WalkList(&debug->threadlist, (WordFnPtr)patchcommand, (word)notify_command);
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
	  patchcommand((THREAD *)dbg.cmd.thread, notify_command);
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
	  (void)WalkList(&debug->threadlist, (WordFnPtr)patchcommand, (word)notify_command);
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
	  (void)WalkList(&debug->threadlist, (WordFnPtr)patchentry,   (word)notify_entry);
	  (void)WalkList(&debug->threadlist, (WordFnPtr)patchreturn,  (word)notify_return);                  
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
	      patchcommand((THREAD *)dbg.cmd.thread, notify_command);
	    }
	  else
	    {
	      FUNC *func = addfunc(findprocinfo(dbg.cmd.modnum, dbg.cmd.offset));
	      
	      if (dbg.cmd.size) func->flags |= Profile;
	      else func->flags &= ~Profile;
	    }
	  continue;
	  
	case DBG_ProfileAll:
	  Wait(&debug->lock);
	  (void)WalkList(&debug->threadlist, (WordFnPtr)profilethread, dbg.cmd.size);
	  (void)WalkList(&debug->threadlist, (WordFnPtr)patchcommand, (word)notify_command);
	  Signal(&debug->lock);
	  continue;
	  
	case DBG_Step:
	  stopthread((THREAD *)dbg.cmd.thread);
	  patchcommand((THREAD *)dbg.cmd.thread, notify_command);
	  resume((THREAD *)dbg.cmd.thread);
	  continue;
	  
	case DBG_StepAll:
	  Wait(&debug->lock);
	  (void)WalkList(&debug->threadlist, (WordFnPtr)stopthread, 0);
	  (void)WalkList(&debug->threadlist, (WordFnPtr)patchcommand, (word)notify_command);
	  (void)WalkList(&debug->threadlist, (WordFnPtr)resume, 0);
	  Signal(&debug->lock);
	  continue;
	  
	case DBG_Stop:
	  stopthread((THREAD *)dbg.cmd.thread);
	  patchcommand((THREAD *)dbg.cmd.thread, notify_command);
	  continue;
	  
	case DBG_StopAll:
	  Wait(&debug->lock);
	  (void)WalkList(&debug->threadlist, (WordFnPtr)stopthread, 0);
	  (void)WalkList(&debug->threadlist, (WordFnPtr)patchcommand, (word)notify_command);
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
	      patchcommand((THREAD *)dbg.cmd.thread, notify_command);
	    }
	  else
	    {
	      FUNC *	func;
	      
	      
	      func = addfunc( findprocinfo( dbg.cmd.modnum, dbg.cmd.offset ) );
	      
	      if (dbg.cmd.size & TraceOff)
		func->flags &= ~dbg.cmd.size;
	      else
		func->flags |= dbg.cmd.size;
	    }
	  continue;
	  
	case DBG_TraceAll:
	  Wait(&debug->lock);
	  (void)WalkList(&debug->threadlist, (WordFnPtr)tracethread, 0);
	  (void)WalkList(&debug->threadlist, (WordFnPtr)patchcommand, (word)notify_command);
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
	  peekmem(dbg.port, (char *)dbg.cmd.modnum, dbg.cmd.size);
	  continue;
	  
	case DBG_PokeMem:
	  pokemem( (char *)dbg.cmd.modnum, dbg.cmd.size, dbg.data);
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
	  
	case DBG_LocateFrame:
	  locateframe( dbg.port, (THREAD *)dbg.cmd.thread, dbg.cmd.modnum, dbg.cmd.offset );
	  continue;
	  
	case DBG_LocateRegister:
	  locateregister( dbg.port, (THREAD *)dbg.cmd.thread, dbg.cmd.offset );
	  continue;
	  
	case DBG_Where:
	  where(dbg.port, (THREAD *)dbg.cmd.thread, dbg.cmd.modnum);
	  continue;
	  
	default:
	  continue;
	}
    }
  
  return;  
  
} /* debugworker */
  
  
/*
 * Try to locate the debug server. If found register ourselves with it
 * otherwise execute as normal.
 */

PRIVATE BOOL
initdebug( void )
{
  Object *	debugger;
  
  
#ifdef MEMCHECK
  Fork( 10000, memcheck, 0 );  
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
      char	name[ 10 * sizeof (word) ];
      Stream *	dbgstream;
	
	
      /*
       * Open a stream to it and register ourselves.
       */
      
      ModuleName_( name, MyTask->Program );
      
      unless ((dbgstream = Open( debugger, name, O_ReadWrite | O_Create )) == NULL)
	{
#ifdef OLDCODE
	  /*
	   * -- crf : 12/08/91 - do not need these routines
	   */
	  
	  Environ *	env = getenviron();
	  Object *	newobjv[ 5 ];
	    
	    
	  newobjv[ 0 ] = env->Objv[ 0 ];	  /* ACE: should test for NULL object */
	  newobjv[ 1 ] = locatecode();
	  newobjv[ 2 ] = locatetask();
	  newobjv[ 3 ] = locatewm ( env );
	  newobjv[ 4 ] = Null ( Object );
	  
	  env->Objv = newobjv;
	  
	  if (MySendEnv( dbgstream->Server, env ) == Err_Null)
#endif	  
	    
	    /*
	      -- crf : 12/08/91 - need the following entries
	      OV_Cdir		0	current directory			
	      OV_Task		1	ProcMan task entry			
	      OV_Code		2	Loader code entry			
	      OV_CServer	7	control console/window server	
	      
	      -- don't need these ...
	      
	      OV_Source		3	original program source file		
	      OV_Parent		4	this task's parent			
	      OV_Home		5	home directory			
	      OV_Console	6	control console			
	      OV_Session	8	user's session manager entry		
	      OV_TFM		9	user's task force manager	
	      OV_TForce		10	TFM entry for parent task force	
	      OV_End		11	NULL at end of Objv			
	      */
	    
	    Environ *	env = getenviron();
	  
	  if (MySendEnv( dbgstream->Server, env ) == Err_Null)
	    {
	      unless ((debug = New( DEBUG )) == NULL)
		{
		  int i;
		  
		  
		  debug->stream = dbgstream;
		  debug->port   = dbgstream->Server;
		  debug->reply  = dbgstream->Reply;
		  
		  InitList( &debug->threadlist );
		  InitList( &debug->breakpointlist );
		  InitList( &debug->watchpointlist );
		  
		  InitSemaphore( &debug->lock, 1 );
		  
		  for (i = 0; i < HashMax; i++)
		    InitList( &debug->functable[ i ] );
		  
		  initthread( &thread );
		  
		  Close( debugger );
#ifdef CRs	  
		  IOdebug( "remdebug called from initdebug" );
#endif		  
		  atexit( remdebug );
		  
		  /*
		   * Since we have a shared module table, we
		   * cannot just directly modify the function
		   * addresses for Fork(), etc.  Instead we
		   * must copy the module table first.
		   */
		  
		  copy_module_table();

#if 1
		  debug->GetMsg = patchgetmsg( DBGGetMsg );		  
		  debug->PutMsg = patchputmsg( DBGPutMsg );
#else
		  debug->GetMsg = GetMsg;		  
		  debug->PutMsg = PutMsg;
#endif		  
		    {
		      DBG dbg;
		      
		      
		      dbg.data = NULL;
		      
		      (void)getdebug( debug->reply, &dbg );
		    }
		  
		  if (Fork( 10000, debugworker, 0 ))
		    {
#if 1		    
		      debug->Fork = patchfork( DBGFork );
#else		      
		      debug->Fork = Fork;
#endif		      
		      return TRUE;
		    }
		} 
	    }	  
	    
	  Close( dbgstream );
	}
	
      Close( debugger );
    }
  else
    {
      IOdebug( "DLIB: Failed to locate debug server" );
    }  
  
  patchentry(   &thread, ignore_entry   );
  patchcommand( &thread, ignore_command );
  patchreturn(  &thread, ignore_return  );
  
  return FALSE;

} /* initdebug */
 


PUBLIC void
_notify_entry(
	      MPtr	procinfo,
	      MPtr	frame_ptr,
	      MPtr	stack_ptr )
{
  if (debug == NULL AND !initdebug())
    {
      return;
    }  

  thread.notify_entry( procinfo, frame_ptr, stack_ptr );

  return;
  
} /* _notify_entry */


PUBLIC word
_notify_return(
	       MPtr	procinfo,
	       word	result )
{
  if (debug == NULL)
    {
      return result;
    }
  
  return thread.notify_return( procinfo, result );
  
} /* _notify_return */


PUBLIC void
_notify_command(
		word 	line,
		MPtr	sourceinfo )
{
  if (debug == NULL)
    {
      return;
    }

  thread.notify_command( line, sourceinfo );
  
  return;
  
} /* _notify_command */
