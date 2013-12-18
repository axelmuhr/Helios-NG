/*------------------------------------------------------------------------
--                                                                      --
--           H E L I O S   N E T W O R K I N G   S O F T W A R E	--
--           ---------------------------------------------------	--
--                                                                      --
--             Copyright (C) 1990, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- netmisc.c								--
--                                                                      --
--	Various miscellaneous routines needed by the Network Server	--
--	which do not seem to fit anywhere else.				--
--                                                                      --
--	Author:  BLV 1/10/91						--
--                                                                      --
------------------------------------------------------------------------*/
/*$Header: /hsrc/network/RCS/netmisc.c,v 1.10 1993/08/12 12:07:00 nickc Exp $*/

/*{{{  contents */
/**
*** This module gathers together various utility routines that used to
*** be spread over the other network server sources. It includes the
*** following:
***    BuildName(), NetMapProcessorToObject()
***    FullRead()
***    rexec(), rexec_task()
***    StartNetworkAgent(), StopNetworkAgent(), XchNetworkAgent()
**/
/*}}}*/
/*{{{  headers and compile-time options */
#define	__NetworkServer
#define __Netmisc_Module

#include <stdio.h>
#include <syslib.h>
#include <servlib.h>
#include <link.h>
#include <sem.h>
#include <codes.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <posix.h>
#include <signal.h>
#include <ctype.h>
#include <nonansi.h>
#include "exports.h"
#include "private.h"
#include "netutils.h"
#include "rmlib.h"
#include "netaux.h"
/*}}}*/
/*{{{  forward declarations and statics */
static	void	NetagentMonitor(void);

void	InitMisc(void)
{
  if (!Fork(AgentMonitor_Stack, &NetagentMonitor, 0))
   fatal("not enough memory to monitor netagents");
}
/*}}}*/
/*{{{  MapProcessorToObject() and BuildName() */
/**
*** Mapping an RmProcessor structure onto a Helios object, e.g. for
*** executing programs. This only makes sense if the processor is
*** currently up and running, so NewObject()/Locate() are used.
***
*** The first step is to build up the full network name of the processor.
*** The object can then be located/newobject()'ed, as required. There is
*** a problem in that the processor may not yet have a full network name.
*** This is only likely in a few special cases, which set the processor
*** state to RmS_Special to indicate this.
**/

char *BuildName(char *buffer, RmProcessor processor)
{ const char *name;

  if (processor eq (RmProcessor) RmRootNetwork(processor))
   { 
     name = NetworkName;
     *buffer++ = '/';
     for ( ; *name ne '\0'; ) *buffer++ = *name++;
     return(buffer);
   }
  else
   { 
     buffer = BuildName(buffer, (RmProcessor) RmParentNetwork(processor));
     *buffer++ = '/';
     name = (RmIsNetwork(processor)) ? RmGetNetworkId((RmNetwork) processor) :
     			(const char *) Procname(processor);
     for ( ; *name ne '\0'; ) *buffer++ = *name++;
     *buffer = '\0';
     return(buffer);
   }
}

Object	*NetMapProcessorToObject(RmProcessor processor)
{ char		*buf = RmGetObjectAttribute((RmObject) processor, "PUID", TRUE);
  Object	*result;
  Capability	*Cap;

  Cap = RmGetProcessorCapability(processor, TRUE);
  if ((*((word *) Cap) eq 0) || (RmGetProcessorState(processor) eq RmS_Special))
   result = Locate(Null(Object), buf);
  else
   result = NewObject(buf, Cap);

  if (result eq Null(Object))
   if (RmGetProcessorState(processor) eq RmS_Special)
    { char	*temp = &(buf[1]);

      for ( ; *temp ne '\0'; temp++)
       if (*temp eq '/') 
        { result = Locate(Null(Object), temp);
          if (result ne Null(Object)) goto done;
        }
    }
done:       
  return(result);
}
/*}}}*/
/*{{{  rexec() */

/**
*** Remotely execute the specified program on the specified processor,
*** using the environment given. If the delay is 0 do not wait for
*** termination. Otherwise return the result.
**/  
word rexec(RmProcessor processor, Object *code, Environ *env, word delay)
{ Object *proc		= Null(Object);
  Object *procman	= Null(Object);
  Object *program	= Null(Object);
  Stream *prog_stream	= Null(Stream);
  word	 rc		= Err_Null;
  int	 step		= 1;

  Debug(dbg_Execute, ("rexec, running program %O on processor %P", code, processor));
  	
  proc = NetMapProcessorToObject(processor);  
  if (proc eq Null(Object))
   { rc = EC_Error + SS_NetServ + EG_Invalid + EO_Processor; goto done; }
  step++;

  Debug(dbg_Execute, ("rexec, full processor name is %O", proc));
  
  procman = Locate(proc, "tasks");
  if (procman eq Null(Object))
   { rc = Result2(proc); goto done; }
  step++;

  program = Execute(procman, code);
  if (program eq Null(Object))
   { rc = Result2(procman); goto done; }
  step++;

  Debug(dbg_Execute, ("rexec, program %O is now running on %P", code, processor));
     
  if ((env ne Null(Environ)) || (delay ne 0))
   { prog_stream = Open(program, Null(char), O_ReadWrite);
     if (prog_stream eq Null(Stream))
      { rc = Result2(program); goto done; }
   }
  step++;

  if (env ne Null(Environ))
   { if (env->Objv[0] ne Null(Object))
      {  env->Objv[OV_Code]	= (Object *) MinInt;
         env->Objv[OV_Task]	= program;
         env->Objv[OV_Source]	= code;
      }
     if (SendEnv(prog_stream->Server, env) < Err_Null)
      { rc = Result2(prog_stream); goto done; }
   }
  step++;

  if (delay ne 0)
   { 
     Debug(dbg_Execute, ("rexec, waiting for program termination"));
     if ((rc = InitProgramInfo(prog_stream, PS_Terminate)) < Err_Null)
      goto done;
     rc = GetProgramInfo(prog_stream, Null(WORD), delay);
   }
  step++;
   
done:
  if (rc ne Err_Null)
   Debug(dbg_Execute, ("rexec for %O on %P failed with %x(%d)",\
		code, processor, rc, step));
  
  if (prog_stream ne Null(Stream)) Close(prog_stream);
  if (rc < Err_Null)
   { if (program ne Null(Object))
      { (void) Delete(program, Null(char));
        (void) Delete(program, Null(char));
        (void) Delete(program, Null(char));
      }
   }
  if (program ne Null(Object))	Close(program);
  if (proc ne Null(Object))	Close(proc);
  if (procman ne Null(Object))	Close(procman);
  return(rc);
}

/*}}}*/
/*{{{  rexec_task() */

/**
*** Similar to rexec, but instead of taking an Object this routine
*** takes an RmTask pointer. Also, the arguments are filled in from
*** the RmTask structure.
**/
word	rexec_task(RmProcessor processor, RmTask task, Environ *env, word delay)
{ char		*procname	= (char *) RmGetTaskCode(task);
  Object	*program	= Null(Object);
  int		number_args	= RmCountTaskArguments(task);
  int		i;
  char		*argv[8];
  word		result		= Err_Null;
    
  if (procname[0] eq '/')
   program = Locate(Null(Object), procname);
  else
   { Object *heliosbin = Locate(Null(Object), "/helios/bin");
     if (heliosbin ne Null(Object))
      { program = Locate(heliosbin, procname);
        Close(heliosbin);
      }
   }
  if (program eq Null(Object))
   { report("warning, failed to locate program %s.", procname);
     return(0);
   }
 
  if (env eq Null(Environ))
   { result = rexec(processor, program, Null(Environ), delay);
     goto done;
   }

  if (number_args < 7)	/* allow for terminator */
   env->Argv = argv;
  else
   { env->Argv = (char **) Malloc(((word) number_args + 1) * sizeof(char *));
     if (env->Argv eq Null(char *))
      { report("warning, out of memory when trying to run program.");
        goto done;
      }
   }

  env->Argv[0] = objname(program->Name);
  for (i = 1; i < number_args; i++)
   env->Argv[i] = (char *) RmGetTaskArgument(task, i);
  env->Argv[i] = Null(char);
  
  result = rexec(processor, program, env, delay);
  
done:
  if (env ne Null(Environ))
   if (number_args >= 7)
    Free(env->Argv);
  if (program ne Null(Object))
   Close(program);
  return(result);
}

/*}}}*/
/*{{{  FullRead() */
/**
*** A little utility routine to cope with the fact that pipe reads do
*** not necessarily return the amount of data requested.
**/
word FullRead(Stream *pipe, BYTE *buffer, word amount, word timeout)
{ word	read = 0;
  word	temp;

  forever  
  { temp = Read(pipe, &(buffer[read]), amount - read, timeout);
    if (temp < 0)
     return((read eq 0) ? temp : read);
    read += temp;
    if (read >= amount) return(read);
    if (timeout ne -1) return(read);
  }
}
/*}}}*/

/*{{{  netagent code */

/**
*** The netagent comms code comes in two varieties. The first variety uses
*** pipes for the communication. When a netagent is started it is passed
*** an environment containing a pipe created on the root processor. 
*** The second variety uses message passing. The netagent is started
*** without an environment, and installs itself as the server /.netagent.
*** A Stream connection is opened, and message passing is used for the
*** communication.
**/
#define	PIPEIO	0

/*{{{  the monitor thread */
static	Semaphore	NetagentStructLock;
static	List		NetagentList;

static void NetagentMonitor(void)
{ ProcessorEntry	*proc_entry;
  ProcessorEntry	*next_entry;
  int			date;

  InitSemaphore(&NetagentStructLock, 1);
  InitList(&(NetagentList));

  forever
   { 
     Delay(10 * OneSec);		/* avoid busy-waiting		  */

     MRSW_GetRead();			/* stop changes to the network	  */
					/* also block RemNetagent below	  */

     Wait(&NetagentStructLock);		/* block with StartNetagent below */

					/* check all current netagents	  */
     date = GetDate(); 
     for (proc_entry = Head_(ProcessorEntry, NetagentList);
          !EndOfList_(proc_entry);
          proc_entry = next_entry)
      { next_entry = Next_(ProcessorEntry, proc_entry);

        if ((proc_entry->NetagentCount eq 0) &&
	    ((date - proc_entry->NetagentDate) > 10))
         { NA_Message		message;
	   RmProcessor		processor = proc_entry->Processor;

		/* If the processor is involved in a bootstrap, keep it */
	   if (RmGetProcessorState(processor) & RmS_Booting)
	    { message.FnRc		= NA_Noop;
	      message.Size		= 0;
	      XchNetworkAgent(processor, &message, FALSE, 0, NULL);
              continue;
            }

	   Debug(dbg_Execute, ("killing netagent on %P", processor));
#if PIPEIO	   
           message.FnRc		= NA_Quit;
	   message.Size		= 0;
	   XchNetworkAgent(processor, &message, FALSE, 0, NULL);
#endif
	   Wait(&(proc_entry->NetagentLock));
	   Close(proc_entry->Netagent);
	   proc_entry->Netagent = Null(Stream);
           Remove(&(proc_entry->NetagentNode));
	   proc_entry->NetagentDate = 0;
	   Signal(&(proc_entry->NetagentLock));
         }		/* if	*/
      }			/* for	*/

     Signal(&NetagentStructLock);

     MRSW_FreeRead();
   }
}
/*}}}*/
/*{{{  StopNetworkAgent() */
/**
*** StopNetworkAgent(). This is used to get rid of a netagent once
*** it is no longer needed, e.g. at the end of a boot job.
**/
bool StopNetworkAgent(RmProcessor processor)
{ ProcessorEntry *proc_entry;

  Debug(dbg_Execute, ("stopping netagent on processor %P", processor));

  proc_entry = GetProcEntry(processor);
  Wait(&(proc_entry->NetagentLock));
  if (proc_entry->Netagent ne Null(Stream))
   proc_entry->NetagentCount--;
  Signal(&(proc_entry->NetagentLock));
  return(TRUE);
}
/*}}}*/
/*{{{  RemNetworkAgent() - in case a processor crashes */
/**
*** If a processor disappears from the network, e.g. because an
*** external subnet has disconnected, and this processor has a network
*** agent running then it is necessary to perform some abort operations.
*** This routine will be called with a write lock.
**/
void	RemNetworkAgent(RmProcessor processor)
{ ProcessorEntry	*proc_entry;

  proc_entry = GetProcEntry(processor);

  Wait(&NetagentStructLock);
  Wait(&(proc_entry->NetagentLock));

  if (proc_entry->Netagent eq Null(Stream)) goto done;

#if !(PIPEIO)
  proc_entry->Netagent->Flags &= ~Flags_Closeable;
#endif
  Close(proc_entry->Netagent);
  proc_entry->Netagent = Null(Stream);
  Remove(&(proc_entry->NetagentNode));
  proc_entry->NetagentDate	= 0;
  proc_entry->NetagentCount	= 0;

done:
  Signal(&(proc_entry->NetagentLock));
  Signal(&NetagentStructLock);
}
/*}}}*/
/*{{{  TerminateProcessor() */
/**
*** Terminating a processor. This requires some special attention.
**/
void	TerminateProcessor(RmProcessor processor)
{ NA_Message		message;

  unless(StartNetworkAgent(processor)) return;
  message.FnRc = NA_Terminate;
  message.Size = 0;
  (void) XchNetworkAgent(processor, &message, FALSE, 0, NULL);
  RemNetworkAgent(processor);
  RmSetProcessorState(processor, 0);
}
/*}}}*/
/*{{{  StartNetworkAgent() */
/**
*** StartNetworkAgent(). Start the network agent on the specified processor. 
*** If it is already running, this is a no-op.
**/
bool StartNetworkAgent(RmProcessor processor)
{ ProcessorEntry *proc_entry;
  bool		 result = FALSE;
  word		 rc;

  Debug(dbg_Execute, ("starting netagent on %P", processor));
  proc_entry = GetProcEntry(processor);

  Wait(&NetagentStructLock);
  Wait(&(proc_entry->NetagentLock));

  if (proc_entry->Netagent ne Null(Stream))
   { result = TRUE; goto done; }

#if PIPEIO
   /*{{{  pipe-based start-up code */
   { Object	*pipe;
     char	 namebuf[NameMax + 5];
     static int	 pipenum = 1;
     Environ	 env;
     char	*argv[1];
     char	*envv[1];
     Object	*objv[1];
     Stream	*strv[2];
   
     strcpy(namebuf, "pipe/netagent.");
     addint(namebuf, pipenum++);
     pipe = Create(ThisProcessor, namebuf, Type_Pipe, 0, Null(BYTE));
     if (pipe eq Null(Object))
      { report("warning, failed to create pipe on root processor, fault %x",
      		Result2(ThisProcessor));
        goto done;
      }
   
     strv[0]	= PseudoStream(pipe, O_ReadWrite);
     if (strv[0] eq Null(Stream))
      { report("warning, failed to access pipe on root processor, fault %x",
    		Result2(pipe));
        (void) Delete(pipe, Null(char)); Close(pipe);
        goto done;
      }
     proc_entry->Netagent = PseudoStream(pipe, O_ReadWrite);
     if (proc_entry->Netagent eq Null(Stream))
      { report("warning, failed to open pipe to netagent, fault %x", Result2(pipe));
        Close(strv[0]);
        (void) Delete(pipe, Null(char)); Close(pipe);
        goto done;
      }
     Close(pipe);
        
     strv[1]	= Null(Stream);
     objv[0]	= Null(Object);
     argv[0]	= Null(char);
     envv[0]	= Null(char);
     env.Strv	= strv;
     env.Objv	= objv;
     env.Argv	= argv;
     env.Envv	= envv;
     rc		= rexec(processor, NetAgent, &env, 0);
   
     Close(strv[0]);
   
     if (rc eq Err_Null)
      if (Write(proc_entry->Netagent, (BYTE *) &rc, sizeof(WORD), 2 * OneSec) ne sizeof(WORD))
       rc = EC_Error + SS_NetServ + EG_Congested + EO_Pipe;      
     if (rc eq Err_Null)
      if (Read(proc_entry->Netagent, (BYTE *) &rc, sizeof(WORD), 2 * OneSec) ne sizeof(WORD))
       rc = EC_Error + SS_NetServ + EG_Congested + EO_Pipe;
   
     if (rc ne Err_Null)
      { 
        Close(proc_entry->Netagent);
        proc_entry->Netagent = Null(Stream);
   	Debug(dbg_Execute | dbg_Problem,\
   		("failed to execute netagent on processor %p, fault %x",\
   		processor, rc));
        MarkProcessor(processor, TRUE);
        (void) Delete(pipe, Null(char));
        goto done;
      }
   }
   /*}}}*/
#else
   /*{{{  message-based code */
   { Object	*proc;
     int	 i;
     
     rc = rexec(processor, NetAgent, NULL, 0);
     if (rc ne Err_Null)
      { Debug(dbg_Execute | dbg_Problem, \
   		("failed to execute netagent on processor %P, fault %x",\
   		processor, rc));
   	MarkProcessor(processor, TRUE);
        goto done;
      }
   
   	/* The program should now be running, attempt to access the netagent */
   	/* The processor may be fairly busy at this time, so the netagent    */
        /* may be blocked for a second or so trying to install its name	     */
        /* table entry.							     */
     proc = NetMapProcessorToObject(processor);
     if (proc eq Null(Object)) goto done;
   
     for (i = 0; i < 10; i++)
      if ((proc_entry->Netagent = Open(proc, ".netagent", O_ReadOnly)) != Null(Stream))
       break;  
      else
       Delay(OneSec / 5);
     Close(proc);
     if (proc_entry->Netagent eq Null(Stream))
      { report("warning, failed to access netagent on processor %P, %x", processor, Result2(proc));
        goto done;
      }
   }
   /*}}}*/
#endif   

  result = TRUE;

	/* After a processor terminate the netagent node could still    */
	/* be in the netagent queue, to avoid deadlock problems.	*/
	/* Hence an additional test is needed.				*/
  if (proc_entry->NetagentDate == 0)
   AddTail(&NetagentList, &(proc_entry->NetagentNode));
  
done:

  if (result)
   { Debug(dbg_Execute, ("netagent now running on %P", processor));
     proc_entry->NetagentCount++;
     proc_entry->NetagentDate = GetDate();
   }  
  Signal(&(proc_entry->NetagentLock));
  Signal(&NetagentStructLock);
  return(result);  
}
/*}}}*/
/*{{{  XchNetworkAgent() */

/**
*** Perform some communication with a Netagent. Take suitable action
*** if the communication fails, typically because the Netagent has gone.
**/
int XchNetworkAgent(RmProcessor processor, NA_Message *message,
	bool read_rc, int rsize1, BYTE *rdata1)
{ ProcessorEntry *proc_entry = GetProcEntry(processor);
  Stream	 *connection;
  int		 result = RmE_Success;
  int		 step;

  step = 0;

  Wait(&(proc_entry->NetagentLock));

	/* There are potential problems with communication problems	*/
	/* while several threads are queueing for this agent.		*/
  connection = proc_entry->Netagent;
  if (connection eq Null(Stream))
   { Signal(&(proc_entry->NetagentLock));
     unless(StartNetworkAgent(processor))
      { Debug(dbg_Problem, ("error restarting netagent on %P", processor));
        return(RmE_CommsBreakdown);
      }
     Wait(&(proc_entry->NetagentLock));
     connection = proc_entry->Netagent;
   }

#if PIPEIO
  /*{{{  pipe-based communication */
  	/* pipes should be reliable, hence no attempt is made to retry */
  if (Write(connection, (BYTE *) message, sizeof(NA_Message), 2 * OneSec) ne
        sizeof(NA_Message))
   goto broken;
  step++;
  
  if (message->Size > 0)
   if (Write(connection, message->Data, message->Size, 2 * OneSec) ne message->Size)
    goto broken;
  step++;
  
  unless(read_rc) goto done;
  if (Read(connection, (BYTE *) &result, sizeof(WORD), 10 * OneSec) ne sizeof(WORD))
     goto broken;
  step++;
  
  if ((result eq RmE_Success) && (rsize1 > 0))
   if (Read(connection, rdata1, rsize1, 2 * OneSec) ne rsize1)
    goto broken;
  /*}}}*/
#else
  /*{{{  message-based communication */

  { MCB		m;
    int		retries;
    word		rc;
  
    for (retries = 0; retries < 3; retries++)
     {
       InitMCB(&m, MsgHdr_Flags_preserve, connection->Server, connection->Reply,
  		FC_Private);
       m.Control		= (WORD *) message;
       m.MsgHdr.ContSize	= sizeof(NA_Message);
       m.Data			= message->Data;
       m.MsgHdr.DataSize	= (int)message->Size;
       if ((rc = PutMsg(&m)) < Err_Null)
        { Debug(dbg_Problem, ("failed to send message to netagent on %P, fault %x",\
  		processor, rc));
   	  continue;
        }
  
       unless(read_rc) goto done;
    
       m.MsgHdr.Dest		= (int) connection->Reply;
       m.Data			= rdata1;
       m.Timeout		= 10 * OneSec;
       result			= (int) GetMsg(&m);
  	
  	/* errors may come from the netagent or from the kernel,	*/
  	/* depending on whether or not it is a communication failure	*/
       if ((result < Err_Null) && ((result & SS_Mask) eq SS_Kernel))
        { if (ReOpen(connection) < Err_Null)
  	   { Debug(dbg_Problem, ("failed to reopen connection to netagent on %P, fault %x", processor, Result2(connection)));
  	     break;
  	   }
        }
       else
        { if (m.MsgHdr.DataSize > rsize1)
           IOdebug("netserv: memory corruption during netagent interaction");
          break;
        }
     }
    if ((result < Err_Null) && ((result & SS_Mask) eq SS_Kernel))
     goto broken;
  }

  /*}}}*/
#endif  
  
done:  
  Signal(&(proc_entry->NetagentLock));
  return(result);

broken:
  Debug(dbg_Problem, ("failed to communicate with netagent on processor %P, %d",\
		processor, step));
  Signal(&(proc_entry->NetagentLock));
  RemNetworkAgent(processor);
  MarkProcessor(processor, FALSE);
  return(RmE_CommsBreakdown);
}

/*}}}*/

/*}}}*/




