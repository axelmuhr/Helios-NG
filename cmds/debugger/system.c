/**
*
* Title:  Helios Debugger - System dependent parts.
*
* Author: Andy England
*
* Date:   September 1988
*
*         (c) Copyright 1988 - 1992, Perihelion Software Ltd.
*
*         All Rights Reserved.
*
**/

#ifdef __TRAN
static char *rcsid = "$Header: /hsrc/cmds/debugger/RCS/system.c,v 1.6 1993/07/06 14:09:14 nickc Exp $";
#endif

#include "tla.h"

PRIVATE bool carsten1 = FALSE; /* CR: to enable some debugs */

/**
*
* syserr(debug, err);
*
* Display a system error.
*
**/
PRIVATE void syserr(DEBUG *debug, word err)
{
  char msg[128];

  Fault(err, msg, 128);
  dprintf(debug->display, "Communication failure: (%08x) \"%s\"", err, msg);
}

/**
*
* err = getdebug(port, dbg);
*
* Get a debug message.
*
**/
PRIVATE BOOL getdebug(Port port, DBG *dbg)
{
  MCB mcb;
  WORD err;

  InitMCB(&mcb, 0, port, NullPort, 0);
  mcb.MsgHdr.ContSize = sizeof(DBGCMD) / sizeof(word);
  mcb.MsgHdr.DataSize = (dbg->data == NULL) ? 0 : (int)dbg->cmd.size;
  mcb.Control = (word *)&dbg->cmd;
  mcb.Data = dbg->data;
  while ((err = GetMsg(&mcb)) == EK_Timeout);
  dbg->port = mcb.MsgHdr.Reply;
if (carsten1)
  debugf("getdebug %x %x %x %d %d",dbg->cmd.action, dbg->cmd.thread, 
      dbg->cmd.modnum, dbg->cmd.offset, dbg->cmd.size);
  if ((mcb.MsgHdr.FnRc & FG_Mask) == FG_Close) return FALSE;
  return err >= Err_Null;
}

/**
*
* err = putdebug(port, reply, dbg);
*
* Put a debug message.
*
**/
PRIVATE word putdebug(Port port, DBG *dbg)
{
  MCB mcb;

  InitMCB(&mcb, MsgHdr_Flags_preserve, port, NullPort, FC_Private|FG_DebugCmd);
  mcb.MsgHdr.ContSize = sizeof(DBGCMD) / sizeof(word);
  mcb.MsgHdr.DataSize = (dbg->data == NULL) ? 0 : (int)dbg->cmd.size;
  mcb.Timeout = DebugTimeout;
  mcb.Control = (word *)&dbg->cmd;
  mcb.Data = dbg->data;
if (carsten1)
  debugf("putdebug %x %x %x %d %d",dbg->cmd.action, dbg->cmd.thread, 
      dbg->cmd.modnum, dbg->cmd.offset, dbg->cmd.size);
  return PutMsg(&mcb);
}

/**
*
* err = ackdebug(port, reply, dbg);
*
-- crf : 13/08/91
-- Same as putdebug(), but does not specify MsgHdr_Flags_preserve
*
* Put a debug message.
*
**/
PRIVATE word ackdebug(Port port, DBG *dbg)
{
  MCB mcb;

  InitMCB(&mcb, 0, port, NullPort, FC_Private|FG_DebugCmd);
  mcb.MsgHdr.ContSize = sizeof(DBGCMD) / sizeof(word);
  mcb.MsgHdr.DataSize = (dbg->data == NULL) ? 0 : (int)dbg->cmd.size;
  mcb.Timeout = DebugTimeout;
  mcb.Control = (word *)&dbg->cmd;
  mcb.Data = dbg->data;
if (carsten1)
  debugf("ackdebug %x %x %x %d %d",dbg->cmd.action, dbg->cmd.thread, 
      dbg->cmd.modnum, dbg->cmd.offset, dbg->cmd.size);
  return PutMsg(&mcb);
}

/**
*
* err = senddebug(port, reply, dbg);
*
* Send a debug message.
*
**/
PRIVATE word senddebug(Port port, Port reply, DBG *dbg)
{
  MCB mcb;

  InitMCB(&mcb, MsgHdr_Flags_preserve, port, reply, FC_Private|FG_DebugCmd);
  mcb.MsgHdr.ContSize = sizeof(DBGCMD) / sizeof(word);
  mcb.MsgHdr.DataSize = (dbg->data == NULL) ? 0 : (int)dbg->cmd.size;
  mcb.Timeout = DebugTimeout;
  mcb.Control = (word *)&dbg->cmd;
  mcb.Data = dbg->data;
if (carsten1)
  debugf("senddebug %x %x %x %d %d",dbg->cmd.action, dbg->cmd.thread, 
      dbg->cmd.modnum, dbg->cmd.offset, dbg->cmd.size);
  return PutMsg(&mcb);
}

/**
*
* receiver(debug);
*
* Listen for messages from the debug library.
*
**/
PUBLIC void
receiver( DEBUG * debug )
{
  DBG 	dbg;
  word	err;
  bool	delwatch = FALSE;
  
  /* 
   * -- crf : keep record of "to-be-deleted" watchpoints
   * -- refer to case DBG_Changed, DBG_DelWatchIds 
   */
  UWORD del_watch_id  = 0 ; 

  
  dbg.data       = NULL;
  dbg.cmd.action = DBG_Ready;  /* ACE: Tell the client we are ready */

  unless ((err = putdebug(debug->reply, &dbg)) == Err_Null)
    {
      syserr(debug, err);
      return;
    }
  
  while (getdebug(debug->port, &dbg))
    {
      switch (dbg.cmd.action)
	{
	case DBG_Stopped:
	  stopped(debug, (int)dbg.cmd.thread, (int)dbg.cmd.modnum, (int)dbg.cmd.offset);
	  continue;
	  
	case DBG_Entered:
	  entered(debug, (int)dbg.cmd.thread, (int)dbg.cmd.modnum, (int)dbg.cmd.offset);
	  continue;
	  
	case DBG_Returned:
	  returned(debug, (int)dbg.cmd.thread, (int)dbg.cmd.modnum, (int)dbg.cmd.offset);
	  continue;
	  
	case DBG_Traced:
	  traced(debug, (int)dbg.cmd.thread, (int)dbg.cmd.modnum, (int)dbg.cmd.offset);
#ifndef BUG
	  dbg.data = NULL;
	  dbg.cmd.action = DBG_Ready;
#ifdef OLDCODE
	  putdebug(dbg.port, &dbg);
#endif
	  /*
	    -- crf : 13/08/91 - use instead of putdebug()
	    */
	  ackdebug(dbg.port, &dbg);
#endif
	  continue;
	  
	case DBG_Continue:
#ifdef NEWCODE
	  if (checkwatchpoints(debug))
	    {
	      stopped(debug, dbg.cmd.modnum, dbg.cmd.offset);
	      continue;
	    }
#endif
	  dbg.cmd.action = DBG_Go;
	  dbg.data = NULL;
	  putdebug(debug->reply, &dbg);
	  continue;
	  
	case DBG_Changed:
#ifdef PARSYTEC
	  delwatch = actualisewatchpoints(debug, dbg.cmd.modnum, dbg.cmd.size, dbg.cmd.offset);
#endif
	  
	  /* 
	    -- crf : 05/08/91 - additional parameter "del_watch_id" 
	    */
	  delwatch = actualisewatchpoints(debug, dbg.cmd.modnum, (int)dbg.cmd.size, 
					  dbg.cmd.offset, &del_watch_id);
	  
	  /* 
	    -- crf : 27/07/91 
	    -- Note : Parsytec Fix : notifywatchpoints() replaced by delwatch()
	    */
#ifdef OLDCODE
	  notifywatchpoints(debug, dbg.cmd.modnum, dbg.cmd.size);
#endif
	  /* 
	    -- crf : 25/07/91 - Bug 700
	    -- Problem : if a structure is watchpointed within a loop, the debug window is
	    -- active for some time after the program terminates, continually giving the
	    -- message "<var_name> changed value". If the program is re-run immediately
	    -- after termination, the system usually hangs. This is a timing problem - 
	    -- i.e. a loss of synchronization between the debugger and dlib.
	    -- Solution : force explicit synchronization via acknowledgement messages.
	    -- I've simply copied in the fix from "case DBG_Traced:" above, and the 
	    -- corresponding changes have been made to "dlib.c" ("checkwatch()").
	    
	    -- crf : 25/07/91 - The above fix also fixes the following :
	    -- Bug 652 - watchpointing multiple variables (1)
	    -- Bug 701 - watchpointing multiple variables (2)
	    -- Bug 703 - watchpointing multiple variables (3)
	    */
	  
	  /* 
	    -- crf : 27/07/91 
	    -- Note : Pardebug - conditional synchronization ... there are major timing
	    -- problems here.
	    */
#ifdef PARSYTEC
	  if ( dbg.cmd.offset != 0) /* CR: syncronisation for framecount */
	    {
	      dbg.data = NULL;
	      dbg.cmd.action = DBG_Ready;
	      putdebug(dbg.port, &dbg);
	    }
#endif
	  
	  /*
	    -- crf : 05/08/91 - Solution to the timing problems (not nice !)
	    -- Overview of problem : 
	    -- 1. DBG_Changed actions are invoked from "checkwatch()" & "checkwatchret()"
	    -- in "dlib.c". Prior to these routines being invoked, a "Wait(&debug->lock)"
	    -- is issued.
	    -- 2. If "vdelete()" is called from "actualisewatchpoints()", the message
	    -- "DBG_RemWatch" is passed to the receiver routine in dlib. On receipt
	    -- of this message, another "Wait(&debug->lock)" is attempted. This will
	    -- obviously not succeed until the corresponding "Signal(...)" is issued.
	    -- 3. "Signal(&debug->lock)" is only performed on exit from "checkwatch()"
	    -- & "checkwatchret()" ... but these routines are waiting for a synchronization
	    -- message to arrive (which is sent at the end of this case routine).
	    -- 4. So, surprise, surprise, we have deadlock.
	    --
	    -- Solution to problem : 
	    -- 1. Instead of calling "vdelete()" from "actualisewatchpoints()", the
	    -- relevant watchpoint identities are simply recorded in "del_watch_id".
	    -- 2. On termination of "checkwatchret()" (inside "_notify_return()" - this
	    -- is the only case that gives problems), the message "DBG_DelWatchIds" is
	    -- sent back here.
	    -- 3. When this message is received (see "case DBG_DelWatchIds" below),
	    -- "del_watch_id" is decoded, and the respective watchpoint identities
	    -- passed to "vdelete()".
	    -- This solution is obviously not great, but will have to do for now.
	    */
	  
	  if (delwatch)
	    {
	      dbg.cmd.offset = -10;
#ifdef PARSYTEC
	      delwatch = actualisewatchpoints(debug, dbg.cmd.modnum, dbg.cmd.size, dbg.cmd.offset);
#endif
	      /* 
		-- crf : 05/08/91 - additional parameter "del_watch_id" 
		*/
	      delwatch = actualisewatchpoints(debug, dbg.cmd.modnum, (int)dbg.cmd.size, 
					      dbg.cmd.offset, &del_watch_id);
	    }
	  
	  /* -- crf : synchronization fix */
	  
	  dbg.data       = NULL;
	  dbg.cmd.action = DBG_Ready;
#ifdef OLDCODE
	  putdebug(dbg.port, &dbg);
#endif
	  /*
	    -- crf : 13/08/91 - use instead of putdebug()
	    */
	  ackdebug(dbg.port, &dbg);
	  
	  continue;
	  
	case DBG_DelWatchIds:
	  /*
	    -- crf : 05/08/91 - remove outstanding watchpoints
	    -- On exiting from sub-routines (and from the main program), watchpoints
	    -- must be removed. Previously, this was done via "actualisewatchpoints()" -
	    -- this, however, caused major synchronization problems (see above). A 
	    -- solution to the problem is not to delete the watchpoints from 
	    -- "actualisewatchpoints()" - instead, a record of the to-be-deleted 
	    -- watchpoint idenitities is maintained using "del_watch_id". Here, 
	    -- "del_watch_id" is decoded, and the respective watchpoint identities
	    -- passed to "vdelete()".
	    */
	  if (del_watch_id != 0)
	    {
	      WORD k ;
	      UWORD mask ; 
	      
	      for (k = 31; k >= 0; k--)
		{
		  mask = 1UL << k ;
		  if (mask & del_watch_id)
		    {
		      vdelete(debug->display, (int)k);
		      cmdmsg(debug,"watchpoint %s removed", debug->display->varvec[k]->expr);
		      del_watch_id -= mask ;
		    }
		}
	    }
	  
	  /* -- crf : synchronize */
	  
	  dbg.data       = NULL;
	  dbg.cmd.action = DBG_Ready;
#ifdef OLDCODE
	  putdebug(dbg.port, &dbg);
#endif
	  /*
	    -- crf : 13/08/91 - use instead of putdebug()
	    */
	  ackdebug( dbg.port, &dbg );
	  
	  continue;
	  
	case DBG_EndThread:
	  endthread(debug, (int)dbg.cmd.thread);
	  continue;
	  
	default:
	  IOdebug( "TLA: Unexpected reply %x", dbg.cmd.action);
	  continue;
	}
    }

  return;
  
} /* receiver */

PUBLIC void syscall(DEBUG *debug, void *func, int argsize, void *args, int retsize, void *retbuf)
{
  DBG dbg;
  Port port = NewPort();
  word err;

  dbg.cmd.action = DBG_Call;
  dbg.cmd.modnum = (word)func;
  dbg.cmd.offset = retsize;
  dbg.cmd.size   = argsize;
  dbg.data       = (byte *)args;
  if ((err = senddebug(debug->reply, port, &dbg)) == Err_Null)
  {
    dbg.cmd.size = retsize;
    dbg.data     = (byte *)retbuf;
    getdebug(port, &dbg);
  }
  else syserr(debug, err);
  
  FreePort(port);
  
  
}

PUBLIC void sysfree(DEBUG *debug, int thread)
{
  DBG dbg;
  word err;

  dbg.cmd.action = DBG_Free;
  dbg.cmd.thread = thread;
  dbg.data       = NULL;
  unless ((err = putdebug(debug->reply, &dbg)) == Err_Null) syserr(debug, err);
}

PUBLIC void sysfreeall(DEBUG *debug)
{
  DBG dbg;
  word err;

  dbg.cmd.action = DBG_FreeAll;
  dbg.data       = NULL;
  unless ((err = putdebug(debug->reply, &dbg)) == Err_Null) syserr(debug, err);
}

PUBLIC void sysgo(DEBUG *debug, int thread)
{
  DBG dbg;
  word err;

  dbg.cmd.action = DBG_Go;
  dbg.cmd.thread = thread;
  dbg.data       = NULL;
  unless ((err = putdebug(debug->reply, &dbg)) == Err_Null) syserr(debug, err);
}

PUBLIC void sysgoall(DEBUG *debug)
{
  DBG dbg;
  word err;

  dbg.cmd.action = DBG_GoAll;
  dbg.data       = NULL;
  unless ((err = putdebug(debug->reply, &dbg)) == Err_Null) syserr(debug, err);
}

PUBLIC void sysgoto(DEBUG *debug, int thread, int modnum, int line)
{
  DBG dbg;
  word err;

  dbg.cmd.action = DBG_Goto;
  dbg.cmd.thread = thread;
  dbg.cmd.modnum = modnum;
  dbg.cmd.offset = line;
  dbg.data       = NULL;
  unless ((err = putdebug(debug->reply, &dbg)) == Err_Null) syserr(debug, err);
}

PUBLIC void sysgotoall(DEBUG *debug, int modnum, int line)
{
  DBG dbg;
  word err;

  dbg.cmd.action = DBG_GotoAll;
  dbg.cmd.modnum = modnum;
  dbg.cmd.offset = line;
  dbg.data       = NULL;
  unless ((err = putdebug(debug->reply, &dbg)) == Err_Null) syserr(debug, err);
}

PUBLIC void sysgotoframe(DEBUG *debug, int thread, int frame)
{
  DBG dbg;
  word err;

  dbg.cmd.action = DBG_GotoFrame;
  dbg.cmd.thread = thread;
  dbg.cmd.modnum = frame;
  dbg.data       = NULL;
  unless ((err = putdebug(debug->reply, &dbg)) == Err_Null) syserr(debug, err);
}

PUBLIC void sysgotoframeall(DEBUG *debug, int frame)
{
  DBG dbg;
  word err;

  dbg.cmd.action = DBG_GotoFrameAll;
  dbg.cmd.modnum = frame;
  dbg.data       = NULL;
  unless ((err = putdebug(debug->reply, &dbg)) == Err_Null) syserr(debug, err);
}

PUBLIC void syskill(DEBUG *debug, int thread)
{
  DBG dbg;
  word err;

  dbg.cmd.action = DBG_Kill;
  dbg.cmd.thread = thread;
  dbg.data       = NULL;
  unless ((err = putdebug(debug->reply, &dbg)) == Err_Null) syserr(debug, err);
}

PUBLIC void syskillall(DEBUG *debug)
{
  DBG dbg;
  word err;

  dbg.cmd.action = DBG_KillAll;
  dbg.data       = NULL;
  unless ((err = putdebug(debug->reply, &dbg)) == Err_Null) syserr(debug, err);
}

PUBLIC void sysprofile(DEBUG *debug, int thread, int modnum, int offset, int mode)
{
  DBG dbg;
  word err;

  dbg.cmd.action = DBG_Profile;
  dbg.cmd.thread = thread;
  dbg.cmd.modnum = modnum;
  dbg.cmd.offset = offset;
  dbg.cmd.size   = mode;
  dbg.data       = NULL;
  unless ((err = putdebug(debug->reply, &dbg)) == Err_Null) syserr(debug, err);
}

PUBLIC void sysstep(DEBUG *debug, int thread)
{
  DBG dbg;
  word err;

  dbg.cmd.action = DBG_Step;
  dbg.cmd.thread = thread;
  dbg.data       = NULL;
  unless ((err = putdebug(debug->reply, &dbg)) == Err_Null) syserr(debug, err);
}

PUBLIC void sysstepall(DEBUG *debug)
{
  DBG dbg;
  word err;

  dbg.cmd.action = DBG_StepAll;
  dbg.data       = NULL;
  unless ((err = putdebug(debug->reply, &dbg)) == Err_Null) syserr(debug, err);
}

PUBLIC void sysstop(DEBUG *debug, int thread)
{
  DBG dbg;
  word err;

  dbg.cmd.action = DBG_Stop;
  dbg.cmd.thread = thread;
  dbg.data       = NULL;
  unless ((err = putdebug(debug->reply, &dbg)) == Err_Null) syserr(debug, err);
}

PUBLIC void sysstopall(DEBUG *debug)
{
  DBG dbg;
  word err;

  dbg.cmd.action = DBG_StopAll;
  dbg.data       = NULL;
  unless ((err = putdebug(debug->reply, &dbg)) == Err_Null) syserr(debug, err);
}

PUBLIC void systrace(DEBUG *debug, int thread, int modnum, int offset, int mode)
{
  DBG dbg;
  word err;

  dbg.cmd.action = DBG_Trace;
  dbg.cmd.thread = thread;
  dbg.cmd.modnum = modnum;
  dbg.cmd.offset = offset;
  dbg.cmd.size   = mode;
  dbg.data       = NULL;
  unless ((err = putdebug(debug->reply, &dbg)) == Err_Null) syserr(debug, err);
}

PUBLIC void systraceall(DEBUG *debug)
{
  DBG dbg;
  word err;

  dbg.cmd.action = DBG_TraceAll;
  dbg.data       = NULL;
  unless ((err = putdebug(debug->reply, &dbg)) == Err_Null) syserr(debug, err);
}

PUBLIC void sysaddbreak(DEBUG *debug, int modnum, int line, int count)
{
  DBG dbg;
  word err;

  dbg.cmd.action = DBG_AddBreak;
  dbg.cmd.modnum = modnum;
  dbg.cmd.offset = line;
  dbg.cmd.size   = count;
  dbg.data       = NULL;
  unless ((err = putdebug(debug->reply, &dbg)) == Err_Null) syserr(debug, err);
}

PUBLIC void sysrembreak(DEBUG *debug, int modnum, int line)
{
  DBG dbg;
  word err;

  dbg.cmd.action = DBG_RemBreak;
  dbg.cmd.modnum = modnum;
  dbg.cmd.offset = line;
  dbg.data       = NULL;
  unless ((err = putdebug(debug->reply, &dbg)) == Err_Null) syserr(debug, err);
}

PUBLIC void peekmem(DEBUG *debug, void *buf, void *addr, int size)
{
  DBG dbg;
  Port port = NewPort();
  word err;

  dbg.cmd.action = DBG_PeekMem;
  dbg.cmd.modnum = (word)addr;
  dbg.cmd.size   = size;
  dbg.data       = NULL;
  
  if ((err = senddebug(debug->reply, port, &dbg)) == Err_Null)
  {
    dbg.cmd.size = size;
    dbg.data     = (byte *)buf;
    getdebug(port, &dbg);
  }
  else syserr(debug, err);
  FreePort(port);
}

PUBLIC UBYTE peekbyte(DEBUG *debug, void *addr)
{
  UBYTE val;

  peekmem(debug, &val, addr, sizeof(UBYTE));
  return val;
}

PUBLIC USHORT peekshort(DEBUG *debug, void *addr)
{
  USHORT val;

  peekmem(debug, &val, addr, sizeof(USHORT));
  return val;
}

PUBLIC UWORD peekword(DEBUG *debug, void *addr)
{
  UWORD val;

  peekmem(debug, &val, addr, sizeof(UWORD));
  return val;
}

PUBLIC void peekdata(DEBUG *debug, void *buf, int modnum, int offset, int size)
{
  DBG dbg;
  Port port = NewPort();
  word err;

  dbg.cmd.action = DBG_PeekData;
  dbg.cmd.modnum = modnum;
  dbg.cmd.offset = offset;
  dbg.cmd.size   = size;
  
  if ((err = senddebug(debug->reply, port, &dbg)) == Err_Null)
  {
    dbg.cmd.size = size;
    dbg.data     = (byte *)buf;
    getdebug(port, &dbg);
  }
  else syserr(debug, err);
  FreePort(port);
}

#ifdef NOT_USED
PUBLIC void peekstack(DEBUG *debug, void *buf, int thread, int frame, int offset, int size)
{
  DBG dbg;
  Port port = NewPort();
  word err;

  dbg.cmd.action = DBG_PeekStack;
  dbg.cmd.thread = thread;
  dbg.cmd.modnum = frame;
  dbg.cmd.offset = offset;
  dbg.cmd.size   = size;
  dbg.data       = NULL;
  if ((err = senddebug(debug->reply, port, &dbg)) == Err_Null)
  {
    dbg.cmd.size = size;
    dbg.data = buf;
    getdebug(port, &dbg);
  }
  else syserr(debug, err);
  FreePort(port);
}
#endif /* NOT_USED */

PUBLIC void pokemem(DEBUG *debug, void *data, void *addr, int size)
{
  DBG dbg;
  word err;

  dbg.cmd.action = DBG_PokeMem;
  dbg.cmd.modnum = (word)addr;
  dbg.cmd.size   = size;
  dbg.data       = (byte *)data;
  
  unless ((err = putdebug(debug->reply, &dbg)) == Err_Null) syserr(debug, err);
}

PUBLIC void *locatedata(DEBUG *debug, int modnum, int offset)
{
  DBG dbg;
  Port port = NewPort();
  word err;

  dbg.cmd.action = DBG_LocateData;
  dbg.cmd.modnum = modnum;
  dbg.cmd.offset = offset;
  dbg.data       = NULL;
  if ((err = senddebug(debug->reply, port, &dbg)) == Err_Null)
    getdebug(port, &dbg);
  else syserr(debug, err);
  FreePort(port);
  return (void *)dbg.cmd.modnum;
}

PUBLIC void *locatestack(DEBUG *debug, int thread, int frame, int offset)
{
  DBG dbg;
  Port port = NewPort();
  word err;
  
  dbg.cmd.action = DBG_LocateStack;
  dbg.cmd.thread = thread;
  dbg.cmd.modnum = frame;
  dbg.cmd.offset = offset;
  dbg.data = NULL;
  if ((err = senddebug(debug->reply, port, &dbg)) == Err_Null)
    getdebug(port, &dbg);
  else syserr(debug, err);
  FreePort(port);
  if ( debug->ifwatchpoint == OLD_Watchpoint)/* CR: I know it is no good solution */
  	return debug->tempwatchpoint;/* but I need the old adress */
  return (void *)dbg.cmd.modnum;
}

#ifdef __C40

PUBLIC void *
locateframe(
	    DEBUG *	debug,
	    int		thread,
	    int		frame,
	    int		offset )
{
  DBG 			dbg;
  Port			port = NewPort();
  word			err;

  
  dbg.cmd.action = DBG_LocateFrame;
  dbg.cmd.thread = thread;
  dbg.cmd.modnum = frame;
  dbg.cmd.offset = offset;
  dbg.data       = NULL;
  
  if ((err = senddebug( debug->reply, port, &dbg )) == Err_Null)
    getdebug( port, &dbg );
  else
    syserr( debug, err );

  FreePort( port );

  if (debug->ifwatchpoint == OLD_Watchpoint)	/* CR: I know it is no good solution */
  	return debug->tempwatchpoint;		/* but I need the old adress */

  return (void *)dbg.cmd.modnum;

} /* locateframe */

PUBLIC void *
locateregister(
	       DEBUG *	debug,
	       int	thread,
	       int	frame,
	       int	offset )
{
  DBG 	dbg;
  Port	port = NewPort();
  word	err;

  
  dbg.cmd.action = DBG_LocateRegister;
  dbg.cmd.thread = thread;
  dbg.cmd.modnum = frame;
  dbg.cmd.offset = offset;
  dbg.data       = NULL;
  
  if ((err = senddebug( debug->reply, port, &dbg )) == Err_Null)
    getdebug( port, &dbg );
  else
    syserr( debug, err );
  
  FreePort( port );
  
  return (void *)dbg.cmd.modnum;
  
} /* locateregister */

#endif /* __C40 */


PUBLIC void sysaddwatch(DEBUG *debug, void *addr, int size)
{
  DBG dbg;
  word err;

  dbg.cmd.action = DBG_AddWatch;
  dbg.cmd.modnum = (word)addr;
  dbg.cmd.size = size;
  dbg.data = NULL;
  unless ((err = putdebug(debug->reply, &dbg)) == Err_Null) syserr(debug, err);
}

PUBLIC void sysremwatch(DEBUG *debug, void *addr, int size)
{
  DBG dbg;
  word err;
  Port port = NewPort();
  
  
  dbg.port = port;
  dbg.cmd.action = DBG_RemWatch;
  dbg.cmd.modnum = (word)addr;
  dbg.cmd.size = size;
  dbg.data = NULL;
  err = senddebug(debug->reply, port, &dbg);
/*  if (err == Err_Null)
 {
  	getdebug (port, &dbg);
  }  CR: syncronisation did not work */
  unless (err == Err_Null) syserr(debug, err);
  FreePort (port);
}

PUBLIC int
syswhere(
	 DEBUG *	debug,
	 int		thread,
	 int		frame,
	 LOCATION *	loc )
{
  DBG 			dbg;
  Port			port = NewPort();
  word			err;

  
  dbg.cmd.action = DBG_Where;
  dbg.cmd.thread = thread;
  dbg.cmd.modnum = frame;
  dbg.data       = NULL;
  
  if ((err = senddebug(debug->reply, port, &dbg)) == Err_Null)
    {
      getdebug(port, &dbg);

      FreePort(port);

      if (dbg.cmd.modnum == -1)
	return -1;

      loc->module = getmodule( debug, (int)dbg.cmd.modnum );
      loc->line   = (int)dbg.cmd.offset;

      return (int) dbg.cmd.size;
    }

  syserr(debug, err);

  FreePort(port);

  return -1;
}

PUBLIC void systimeout(DEBUG *debug, int thread)
{
  DBG dbg;
  word err;

  dbg.cmd.action = DBG_Timeout;
  dbg.cmd.thread = thread;
  dbg.data = NULL;
  unless ((err = putdebug(debug->reply, &dbg)) == Err_Null) syserr(debug, err);
}

PUBLIC void systimeoutall(DEBUG *debug)
{
  DBG dbg;
  word err;

  dbg.cmd.action = DBG_TimeoutAll;
  dbg.data = NULL;
  unless ((err = putdebug(debug->reply, &dbg)) == Err_Null) syserr(debug, err);
}
