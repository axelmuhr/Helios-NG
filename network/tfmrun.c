/*------------------------------------------------------------------------
--                                                                      --
--           H E L I O S   N E T W O R K I N G   S O F T W A R E	--
--           ---------------------------------------------------	--
--                                                                      --
--             Copyright (C) 1990, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- tfmrun.c								--
--                                                                      --
--	Routines for running taskforces.				--
--                                                                      --
--	Author:  BLV 1/5/90						--
--                                                                      --
------------------------------------------------------------------------*/
/* RcsId: $Header: /hsrc/network/RCS/tfmrun.c,v 1.14 1993/12/20 13:33:59 nickc Exp $*/

/*{{{  headers */
#include <stdio.h>
#include <stddef.h>
#include <syslib.h>
#include <servlib.h>
#include <sem.h>
#include <codes.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <posix.h>
#include <ctype.h>
#include <nonansi.h>
#include <attrib.h>
#include <pwd.h>
#include <signal.h>
#include <module.h>
#include <root.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include "exports.h"
#include "private.h"
#include "netutils.h"
#include "rmlib.h"
#include "tfmaux.h"
/*}}}*/
/*{{{  statics and module initialisation */
	int	MonitorDelay;

void	InitRun(void)
{
  MonitorDelay = get_int_config("monitor_interval", environ);
  if ((MonitorDelay eq Invalid_config) || (MonitorDelay < 30))
   MonitorDelay = 30;
  elif (MonitorDelay > (30 * 60))
   MonitorDelay = 30 * 60;
}
/*}}}*/

/*{{{  task_AddTaskEntry() */

/**-----------------------------------------------------------------------------
*** task_AddTaskEntry(). The TaskEntry structure associated with every
*** top-level task or taskforce and with every component task is critical
*** to the operation of the TFM. It contains all the information that the
*** TFM needs to administer this task. This routine is responsible for
*** allocating the necessary space and initialising it. The exact
*** initialisation will depend somewhat on the nature of the task or
*** taskforce.
**/
int	task_AddTaskEntry(RmTask task, ...)
{ TaskEntry	*task_entry;
  int		number_streams = 0;
  RmTaskforce	taskforce = (RmTaskforce) NULL;

	/* Possibilities:					*/
	/* 1) top-level taskforce				*/
	/* 2) component task, parent is taskforce		*/
	/* 3) top-level task					*/
  if (RmIsTaskforce(task))
   taskforce = (RmTaskforce) task;
  elif ((task->ObjNode.Parent ne NULL) && (RmIsTaskforce((RmTask) task->ObjNode.Parent)))
   { number_streams = RmCountChannels(task);
     if (number_streams < DefaultStreams) number_streams = DefaultStreams;
   }

  task_entry = (TaskEntry *) Malloc(sizeof(TaskEntry) + ((long)number_streams * sizeof(Stream *)));
  if (task_entry eq Null(TaskEntry)) return(RmE_ServerMemory);

	/* Clear the whole structure to NULL				*/
  memset(task_entry, 0, sizeof(TaskEntry) + (number_streams * sizeof(Stream *)));

	/* Initialise the stream vector to MinInt's with NULL terminator */
	/* This guarantees that the program will receive the right	 */
	/* number of streams.						 */
  task_entry->Streams[number_streams--] = Null(Stream);
  for ( ; number_streams >= 0; number_streams--)
   task_entry->Streams[number_streams] = (Stream *) MinInt;

	/* Some fields must be initialised to non-zero values.		*/
  task_entry->Task	= task;
  InitSemaphore(&(task_entry->Finished), 0);
  InitList(&(task_entry->Waiting));
  InitList(&(task_entry->ComponentCode.List));
  InitSemaphore(&(task_entry->ComponentCode.Ready), 0);
  InitSemaphore(&(task_entry->ComponentCode.StructLock), 1);

  if (taskforce ne (RmTaskforce) NULL)
   RmSetTaskforcePrivate(taskforce, (int) task_entry);
  else
   RmSetTaskPrivate(task, (int) task_entry);

	/* Install capabilities within the task/taskforce	*/
  task->ObjNode.Key	= NewKey() ^ _cputime();
  if (taskforce ne (RmTaskforce) NULL)
   NewCap(&(taskforce->TfmCap), (ObjNode *) taskforce, AccMask_Full);
  else
   NewCap(&(task->TfmCap), &(task->ObjNode), AccMask_Full);
  return(RmE_Success);
}

/*}}}*/
/*{{{  task_FilterArgs() */
/**
*** Argument filter - given a task with some number of arguments embedded,
*** possibly including filters such as $1, an additional vector of
*** arguments, and an outgoing Environment structure, construct the
*** outgoing environment argv vector. The caller provides a default vector
*** for up to eight arguments, if necessary a larger vector will be allocated
*** and will have to be freed by the caller.
**/
bool task_FilterArgs(RmTask task, Environ *sending, char **received, char **default_args)
{ int	number_args = RmCountTaskArguments(task);
  int	i;
  char  **argv;    

  if ((number_args + 1) < 8)
   argv = default_args;
  else
   argv = (char **) Malloc(((long)number_args + 1) * sizeof(char *));
  if (argv eq Null(char *)) return(FALSE);
  sending->Argv = argv;       

  for (i = 0; i < number_args; i++)
   { argv[i] = (char *) RmGetTaskArgument(task, i);
     if (*(argv[i]) eq '$')
      { int	offset = 0;
        char	*ptr   = &((argv[i])[1]);
        int	j;
        bool  got_null = FALSE;

        for ( ; ('0' <= *ptr) && (*ptr <= '9'); ptr++)
         offset = (10 * offset) + (*ptr - '0');          
        for (j = 0; j <= offset; j++)
         if (received[j] eq Null(char))
          { got_null = TRUE; break; }
        if (got_null)
         argv[i] = "";
        else
         argv[i] = received[offset];
      }
   }
  argv[i] = Null(char); 
  return(TRUE);
}
/*}}}*/

/*{{{  task_Run() */
/**-----------------------------------------------------------------------------
*** task_Run(). This routine is responsible for executing a single task,
*** stand-alone, on its mapped processor. A different routine is used to
*** execute taskforce components to allow for pre-loading resident libraries
*** etc.
**/

word task_Run(RmProcessor processor, RmTask task, Object *program)
{ TaskEntry	*task_entry;
  WORD		e = Err_Null;
  Object	*real_processor = Null(Object);
  Object	*loader		= Null(Object);
  Object	*procman	= Null(Object);
  
  Debug(dbg_Create, ("running task %T (%O) on processor %P", \
		  	task, program, processor));
  	
  task_entry = GetTaskEntry(task);
  if (task_entry eq Null(TaskEntry)) return(EC_Error + EG_Invalid + EO_Task);

  task_entry->Program		= program;
  
  Wait(&(processor->ObjNode.Lock));
  
	/* Get all the details for the processor. */
  real_processor = TfmMapProcessorToObject(processor);
  if (real_processor eq Null(Object))
   { e = EC_Error + EG_NoMemory + EO_Processor; goto done; }

  loader = Locate(real_processor, "loader");
  if (loader eq Null(Object))
   { Debug(dbg_Create, ("failed to locate %O/loader, fault %x",\
   		 real_processor, Result2(real_processor)));
     e = EC_Error + EG_Unknown + EO_Loader; 
     goto done; 
   }

  procman = Locate(real_processor, "tasks");
  if (procman eq Null(Object))
   { Debug(dbg_Create, ("failed to locate %O/tasks, fault %x",\
   		real_processor, Result2(real_processor)));
     e = EC_Error + EG_Unknown + EO_ProcMan; 
     goto done; 
   }

	/* Try to load the program onto the processor */  
  task_entry->LoadedCode = Load(loader, program);
  if (task_entry->LoadedCode eq Null(Object))
   { Debug(dbg_Create, ("failed to load program %O into %O, fault %x",\
   		 program,	loader, Result2(loader)));
     e = EC_Error + EG_Create + EO_Program; 
     goto done; 
   }

  	/* Then execute it. */
  task_entry->ProgramObject = Execute(procman, task_entry->LoadedCode);
  if (task_entry->ProgramObject eq Null(Object))
   { Debug(dbg_Create, ("failed to execute %O in %O, fault %x", \
   		task_entry->LoadedCode, procman, Result2(procman)));
     e = EC_Error + EG_Create + EO_Program;
     goto done; 
   }

  	/* Now open a stream to it. */
  task_entry->ProgramStream = 
  	Open(task_entry->ProgramObject, Null(char), O_ReadWrite);
  if (task_entry->ProgramStream eq Null(Stream))
   { Debug(dbg_Create, ("failed to open %O, fault %x",\
	task_entry->ProgramObject, Result2(task_entry->ProgramObject)));
     e = EC_Error + EG_Open + EO_Program; 
     goto done; 
   }

  unless(Fork(Monitor_Stack, task_Monitor, sizeof(RmTask), task))
   { Debug(dbg_Create, ("failed to Fork() monitor process"));
     e =  EC_Error + EG_NoMemory + EO_Task; 
     goto done; 
   }

	/* On success the task state and structure type should be adjusted */  
  e = Err_Null;
  RmSetTaskState(task, RmS_Running);
  task->StructType = RmL_Executing;
  task_entry->UseCount++;

done:
  if (e ne Err_Null)
   { if (task_entry ne Null(TaskEntry))
      { if (task_entry->ProgramStream ne Null(Stream))
         { SendSignal(task_entry->ProgramStream, SIGKILL);
           Close(task_entry->ProgramStream);
           task_entry->ProgramStream = Null(Stream);
         }
        if (task_entry->ProgramObject ne Null(Object))
         { task_Exterminate(task);
           Close(task_entry->ProgramObject);
           task_entry->ProgramObject = Null(Object);
         }
        if (task_entry->LoadedCode ne Null(Object))
         { (void) Delete(task_entry->LoadedCode, Null(char));
           Close(task_entry->LoadedCode);
           task_entry->LoadedCode = Null(Object);
         }
      }    
   }
  Signal(&(processor->ObjNode.Lock));
  if (real_processor ne Null(Object)) Close(real_processor);
  if (loader ne Null(Object)) Close(loader);
  if (procman ne Null(Object)) Close(procman);
  return(e);  
}
/*}}}*/
/*{{{  task_Monitor() */
/**
*** 2) Monitor a task, i.e. wait for it to finish. The main purpose of this
***    code is to handle termination which can be caused by a variety of
***    reasons, including processor crashes. In addition this routine is
***    responsible for sending environments and signals since it already
***    has a stream to the running program, thus avoiding the need to
***    open another stream. 
***
***    The main loop performs InitProgramInfo() and GetProgramInfo() in
***    a loop. The timeout argument to GetProgramInfo() is the main 
***    network timeout specified in nsrc, possibly adjusted in this
***    module's initialisation code. The result can be one of the following:
***
***    a) EK_Timeout : the program is believed to be still running. The
***       TFM checks up on the program just to be sure.
***
***    Any other value means that the program has terminated or has crashed.
***    Termination codes are as follows:
***
***    b) EC_Fatal + SS_TFM + EG_Create + EO_Taskforce : aborting components
***       because the TFM failed to start the whole taskforce
***    c) EC_Fatal + SS_TFM + EG_Broken + EO_Program   : there was a timeout
***       on the GetProgramInfo(), and the program has disappeared
***    d) EC_Fatal + SS_TFM + EG_Broken + EO_Processor :  the Network Server
***       has reclaimed this processor or reported is as crashed
***    e) other errors, generated by the nucleus somewhere along the way
***    f) 0x0000yy00, program terminated with return code yy
***    g) 0x0000008z, program terminated because of a signal z (signal number
***       occupies bottom 6 bits)
**/

void task_Monitor(RmTask task)
{ TaskEntry	*task_entry = GetTaskEntry(task);
  word		flags;
  RmTaskforce	taskforce;
  RmProcessor	processor;
  TaskWaiter	*waiter;

  unless (task->ObjNode.Flags & TfmFlags_Special) 
   for ( task->ReturnCode = (int)EK_Timeout; task->ReturnCode eq EK_Timeout; )
    { int	retries	= 0;

  		/* Check the current stream				*/
      if (InitProgramInfo(task_entry->ProgramStream, PS_Terminate) < Err_Null)
       { Debug(dbg_Monitor, ("error when monitoring %S", task_entry->ProgramStream));
         task->ReturnCode = (int)(EC_Fatal + SS_TFM + EG_Broken + EO_Program);
         break;
       }
back:
		/* wait for the termination message			*/
      task->ReturnCode = (int)GetProgramInfo(task_entry->ProgramStream, NULL, MonitorDelay * OneSec);

      Wait(&(task->ObjNode.Lock));

		/* After a timeout, reopen the connection to the task.	*/
		/* This is more expensive than I would like, but it	*/
		/* avoids problems with broken stream connections.	*/
		/* There is a problem if the second Open fails. This	*/
		/* could be due to transient problems such as a		*/
		/* shortage of memory in the TFM processor, or because	*/
		/* some other processor has crashed and the world is	*/
		/* still trying to sort itself out. I am reluctant to	*/
		/* assume that a task has crashed until the Network	*/
		/* Server has indicated the failure, which		*/
		/* GetProgramInfo() will pick up. Only if three		*/
		/* successive ReOpens fail does the TFM assume weird	*/
		/* program failure.					*/
      if (task->ReturnCode eq EK_Timeout)
       { Stream	*temp = Open(task_entry->ProgramObject, Null(char), O_ReadWrite);
 	 Stream	*temp2;

	 if (temp ne Null(Stream))
	  { Debug(dbg_Monitor, ("component %S is still running", task_entry->ProgramStream));
	    temp2 = task_entry->ProgramStream;
            task_entry->ProgramStream = temp;
            Close(temp2);
	    retries = 0;	   
	  }
	 elif (((Result2(task_entry->ProgramObject) & EG_Mask) == EG_NoMemory) 
		|| (++retries < 3))
	  { Debug(dbg_Monitor, ("potential problem with %T (%S), retrying", task, task_entry->ProgramStream));
	    Signal(&(task->ObjNode.Lock));
	    goto back;
	  }
	 else 
	  { Debug(dbg_Monitor, ("lost contact with %T (%S)", task, task_entry->ProgramStream));
	    task->ReturnCode = (int)(EC_Fatal + SS_TFM + EG_Broken + EO_Program);
	  }
       }
	/* After any other type of error, break out of the loop */
      if (task->ReturnCode eq EK_Timeout) Signal(&(task->ObjNode.Lock));
    }
  else	/* Special task	*/
   { Object	*running;

     for (running = Locate(NULL, task_entry->ProgramObject->Name);
	  running ne NULL;
	  running = Locate(NULL, task_entry->ProgramObject->Name))
      { Close(running);
	Delay(5 * OneSec);
      }
     task->ReturnCode = 0;
   }

  Debug(dbg_Monitor, ("component %O has terminated with %x",\
  	task_entry->ProgramObject, task->ReturnCode));

	/* make sure that tasks, taskforces, and processors are not freed */
  MRSW_GetRead();

  task->ObjNode.Account += 1000;  
  task_entry->UseCount--;
  task->StructType = RmL_Terminated;
  RmSetTaskState(task, RmS_Finished);

  if ((task->ObjNode.Parent eq &TFM) || (task->ObjNode.Parent eq NULL))
   taskforce = (RmTaskforce) NULL;
  else
   taskforce = RmRootTaskforce(task);

  processor = RmFollowTaskMapping(Domain, task);

  if (task->ReturnCode eq EC_Fatal + SS_TFM + EG_Broken + EO_Program)
   {	/* The program has disappeared, report the processor and send	*/
	/* a suitable signal to siblings.				*/
     if (processor ne RmM_NoProcessor)
      MarkProcessor(processor);

     if (taskforce ne (RmTaskforce) NULL)
      taskforce_DoSignal(taskforce, SIGSIB);
   }
  elif (task->ReturnCode eq EC_Fatal + SS_TFM + EG_Create + EO_Taskforce)
   {	/* The TFM is aborting a taskforce create		*/
   }
  elif (task->ReturnCode eq EC_Fatal + SS_TFM + EG_Broken + EO_Processor)
   { 	/* The processor running this task has gone.		*/
	/* See LostProcessor(), tfmwoe.c			*/
   }
  elif (task->ReturnCode < Err_Null)
   {	/* No idea what to do with nucleus errors.		*/
     if (processor ne RmM_NoProcessor)
      MarkProcessor(processor);
   }
  elif (task->ReturnCode & 0x0080)
   { 	/* Some signals should be reported to the other taskforce	*/
	/* components, others should be quietly ignored. Which signal	*/
	/* goes into which category is open to debate.			*/
     int	signal = task->ReturnCode & 0x003F;
     switch(signal)
      { case	SIGABRT :	/* program abort			*/
	case	SIGFPE	:	/* hardware funnies			*/
	case	SIGILL	:
	case	SIGSEGV	:
	case	SIGSTAK :
	case	SIGHUP	:	/* hangup signal			*/
			  if (taskforce ne (RmTaskforce) NULL)
			   taskforce_DoSignal(taskforce, SIGSIB);
			  break;

	case	SIGZERO :
	case	SIGINT	:	/* ctrl-C				   */
	case	SIGTERM	:
	case	SIGALRM	:
	case	SIGPIPE	:	/* communication funnies should be handled */
				/* by the components themselves.	   */
	case	SIGQUIT	:
	case	SIGTRAP	:
	case	SIGUSR1	:
	case	SIGUSR2	:
	case	SIGCHLD	:
	case	SIGURG	:
	case	SIGCONT	:
	case	SIGSTOP	:
	case	SIGTSTP	:
	case	SIGTTIN	:
	case	SIGTTOU	:
	case	SIGWINCH:
	case	SIGSIB	:	/* generated by TFM already		*/
	case	SIGKILL	:	/* almost certainly global		*/
	default		:
			  break;
      }
   }
  else
   {	/* Normal termination */
   }

  MRSW_FreeRead();
  MRSW_GetWrite();

  for (waiter = Head_(TaskWaiter, task_entry->Waiting); !EndOfList_(waiter); waiter = Next_(TaskWaiter, waiter))
   if (waiter->Sem ne NULL)
    Signal(waiter->Sem);

	/* If a taskforce component finishes, inform the taskforce_Monitor() */
  if (taskforce ne (RmTaskforce) NULL)
   { TaskEntry   *taskforce_entry = (TaskEntry *) RmGetTaskforcePrivate(taskforce);
     Signal(&(taskforce_entry->Finished));
     MRSW_FreeWrite();
     Signal(&(task->ObjNode.Lock));
     return;
   }

	/* If a single task finishes, generate the proginfo and, possibly, */
	/* destroy this task. This task may be the user's login shell.	   */
  flags = task->ObjNode.Flags;
  task_GenProgInfo(task);
  task->ObjNode.Account	+= 1000;
  if (task_entry->UseCount eq 0)
   task_Destroy(task);
  else
   Signal(&(task->ObjNode.Lock));

  MRSW_FreeWrite();
  if (flags & TfmFlags_FirstTask)
   while (!Fork(Terminate_Stack, TerminateTFM, 0));
}
/*}}}*/
/*{{{  task_Exterminate() (stop it running) */
/**
*** 3) Exterminate a task. The processor manager should get rid of any task
***    when it has been deleted three times.
**/
void task_Exterminate(RmTask task)
{ TaskEntry	*task_entry = GetTaskEntry(task);
  Object	*program;
  Object	*temp;
  int		state = RmGetTaskState(task);

  if ((state eq RmS_Dead) || (state eq RmS_Crashed)) return;
  
  if (task_entry eq Null(TaskEntry)) return;
  program = task_entry->ProgramObject;
  if (program eq Null(Object)) return;

  Debug(dbg_Delete, ("exterminating task %O", program));

  temp = Locate(program, Null(char));
  
  if (temp eq Null(Object))
    {
      return;
    }
  
  temp->FnMod |= 2;	/* to guarantee a kill */
  temp->Access = program->Access;
  
  (void) Delete(temp, Null(char));
  
  Close(temp);  
}
/*}}}*/
/*{{{  task_GenProgInfo() */
/**
*** 4) task_GenProgInfo()
***    Generate a reply to an outstanding ProgInfo request.
**/
void task_GenProgInfo(RmTask task)
{ TaskEntry	*task_entry = GetTaskEntry(task);
  MCB		m;
  
 if ((task_entry->ProgInfoMask ne 0) && (task_entry->ProgInfoPort ne NullPort))
  { InitMCB(&m, MsgHdr_Flags_preserve, task_entry->ProgInfoPort, NullPort,
  		task->ReturnCode);
    Debug(dbg_Delete, ("generating termination message for task %T",\
    		task));
    task->ObjNode.Account += 100;
    PutMsg(&m);
    task->ObjNode.Account += 100;
  }
}
/*}}}*/
/*{{{  task_Destroy()     (release data structures) */
/**
*** 5) task_Destroy().
***    Kill/abort a task. The /tfm entry is made inaccessible.
***    If the task is running it is exterminated. All the data structures
***    are freed.
**/

void task_Destroy(RmTask task)
{ TaskEntry	*task_entry = GetTaskEntry(task);
  int		 i;
  
  if (task_entry->UseCount > 0)
   { report("internal error: task %T, attempt to destroy while use count is %d", 
		task, task_entry->UseCount);
     return;
   }

  Debug(dbg_Delete, ("destroying task %T", task));

  Unlink(&(task->ObjNode), FALSE);
  task->ObjNode.Parent = Null(DirNode);

  if (task_entry->ProgInfoPort ne NullPort) FreePort(task_entry->ProgInfoPort);
  if (task_entry->ProgramStream ne Null(Stream))
   Close(task_entry->ProgramStream);
  if (task_entry->ProgramObject ne Null(Object))
   { task_Exterminate(task);
     Close(task_entry->ProgramObject);
   }
  if (task_entry->LoadedCode ne Null(Object)) Close(task_entry->LoadedCode);
  if (task_entry->Program ne Null(Object))    Close(task_entry->Program);
  if (task_entry->Mapped) domain_UnmapTask(task);
  for (i = 0;   (i < MaxLibrariesPerComponent) && 
		(task_entry->ComponentCode.Libraries[i] ne Null(Object));
	i++)
   Close(task_entry->ComponentCode.Libraries[i]);

  Free(task_entry);
  RmFreeTask(task);
}
/*}}}*/
/*{{{  task_DoSignal() */
/**
*** 6) task_DoSignal()
***    Send a signal to the specified task. This involves opening another
***    stream to the task because the current stream is used for monitoring.
**/
void task_DoSignal(RmTask task, WORD signal)
{ TaskEntry	*task_entry = GetTaskEntry(task);
  Stream	*s;

  if ((task->ObjNode.Flags & TfmFlags_Special) && (signal ne SIGSIB))
   return;

  if (task_entry eq Null(TaskEntry)) return;
  if (task_entry->ProgramObject eq Null(Object)) return;
  unless (RmGetTaskState(task) & RmS_Running) return;
   
  Debug(dbg_Signal, ("sending signal %d to task %O", signal, \
		task_entry->ProgramObject));  

  s = Open(task_entry->ProgramObject, Null(char), O_ReadWrite);
  if (s eq Null(Stream)) return;
  SendSignal(s, signal);
  Close(s);
}
/*}}}*/
/*{{{  task_HandleEnv() */
/**
*** 7) task_HandleEnv()
***    This routine is called when the first message of a SendEnv request
***    has been received. The rest of the environment is obtained by
***    a modified form of the system's GetEnv() routine, tfm_GetEnv().
***    This environment is then adjusted as appropriate for the task,
***    and sent off.
**/

word task_HandleEnv(RmTask task, Environ *received)
{ Environ	sending;
  Object	*Objv[OV_End + 1];
  TaskEntry	*task_entry = GetTaskEntry(task);
  int		i;
  word		result = Err_Null;
  int		length;
  char		*buffer;
  Capability	cap;
  
  sending.Argv		= received->Argv;
  sending.Envv		= received->Envv;
  sending.Strv		= received->Strv;
  Objv[OV_Cdir]		= received->Objv[OV_Cdir];
  Objv[OV_Task]		= task_entry->ProgramObject;
  Objv[OV_Code]		= task_entry->LoadedCode;
  Objv[OV_Source]	= task_entry->Program;
  Objv[OV_Parent]	= received->Objv[OV_Parent];
  Objv[OV_Home]		= received->Objv[OV_Home];
  Objv[OV_Console]	= received->Objv[OV_Console];
  Objv[OV_CServer]	= received->Objv[OV_CServer];
  Objv[OV_Session]	= received->Objv[OV_Session];
  Objv[OV_TFM]		= received->Objv[OV_TFM];

	/* The Taskforce Manager must zap one entry in the object	*/
	/* vector, that for the taskforce itself. This must be done	*/
	/* without actually accessing the taskforce because that would	*/
	/* deadlock. The name should be : /Cluster/00/bart/tfm/job.6	*/
  length = strlen(ProcessorName) + strlen(Root.Name) +
  	strlen(task->ObjNode.Name) + 7;
  buffer = (char *) Malloc(length);
  if (buffer eq Null(char)) { result = RmE_ServerMemory; goto done; }
  strcpy( buffer, ProcessorName);
  pathcat(buffer, Root.Name);
  strcat( buffer, "/tfm/");
  strcat( buffer, task->ObjNode.Name);
  NewCap(&cap, (ObjNode *) &Root, AccMask_Full);
  if ((received->Objv[OV_TForce] ne Null(Object)) &&
      (received->Objv[OV_TForce] ne (Object *) MinInt))
   Close(received->Objv[OV_TForce]);
   
  received->Objv[OV_TForce] = NewObject(buffer, &cap);
  Free(buffer);
  if (received->Objv[OV_TForce] eq Null(Object))
   { result = RmE_ServerMemory; goto done; }  
  Objv[OV_TForce]	= received->Objv[OV_TForce];
  Objv[OV_End]		= Null(Object);
  for (i = 0; i < OV_End; i++)
   if (Objv[i] eq Null(Object))
    Objv[i] = (Object *) MinInt;
  sending.Objv		= Objv;

  if (DebugOptions & dbg_Environ)
   { int i;
     report("sending environment to task %T", task);
     for (i = 0; sending.Argv[i] ne Null(char); i++)
      report("argument %d is %s", i, sending.Argv[i]);
     for (i = 0; sending.Envv[i] ne Null(char); i++)
      report("environment string %d is %s", i, sending.Envv[i]);
     for (i = 0; sending.Strv[i] ne Null(Stream); i++)
      if (sending.Strv[i] ne (Stream *) MinInt)
       report("stream %d is %S", i, sending.Strv[i]);
     for (i = 0; sending.Objv[i] ne Null(Object); i++)
      if (sending.Objv[i] ne (Object *) MinInt)
       report("object %d is %O", i, sending.Objv[i]);
   }
   

  result = SendEnv(task_entry->ProgramStream->Server, &sending);

done:
  return(result);
}
/*}}}*/

/*{{{  taskforce_Start() */
/*{{{  forward declarations */
static word determine_code(RmTask, ...);
static word taskforce_Start_aux1(RmTask, ...);
static int  taskforce_Start_aux2(RmTask, ...);
static int  taskforce_Start_aux3(RmTask, ...);
static int  taskforce_Start_aux4(RmTask, ...);
static void component_Handler(RmTask, Semaphore *);
/*}}}*/
/*{{{  taskforce_Start()      - the main routine */
/**
*** taskforce_Start(). This routine is given a single taskforce structure,
*** probably containing only tasks that are not yet running.
***
*** 1) every component task will have been mapped already and will have a
***    TaskEntry structure. It is necessary to work out where the various
***    bits of code are going to come from, i.e. decide when to load from
***    disk and when to load from neighbour, taking into account different
***    processor types and binaries. 
***
*** 2) a separate thread will be spawned off for every component. This is
***    responsible for starting that component, creating pipes, and
***    monitoring that component. The threads synchronise with the main
***    taskforce_Start() routine using a semaphore.
***
*** 3) once all the threads have signalled that they have finished their
***    current job, i.e. that they failed or that they are monitoring
***    their component, taskforce_Start() checks whether the start-up
***    was completely successful. During this phase it is also possible
***    to release various objects held by each task, thus saving memory.
***
*** 4) if the whole taskforce is running then a separate thread can be
***    spawned to monitor the taskforce as a whole, and everything is OK.
***
*** 5) otherwise it is necessary to abort the taskforce. This is a 
***    two-stage process. First the monitor threads are aborted, which
***    will cause the component handler to exterminate the task.
***    Second it is necessary to wait for all the components to perform
***    this extermination.
***
*** N.B. it is essential that taskforce_Start() is an atomic operation.
*** Either the taskforce must be completely running, or none of the
*** components must be running. In particular, there must be no
*** monitor threads active after a failure because the data structures
*** will be freed immediately.
**/

word taskforce_Start(RmTaskforce taskforce)
{ Semaphore	all_done;
  int		count;
  word		rc;
  char		*name = Taskforcename(taskforce);
  TaskEntry	*task_entry = (TaskEntry *) RmGetTaskforcePrivate(taskforce);

  Debug(dbg_Create, ("determining loading strategy for %s", name));
  rc = RmSearchTasks(taskforce, (int(*)(RmTask,...))&determine_code);
  if (rc ne Err_Null)
   { Debug(dbg_Create, ("failed to work out loading strategy for %s", name));
     return(rc);
   }

  Debug(dbg_Create, ("starting threads for the various components of %s", name));
  InitSemaphore(&all_done, 0);
  count = 0;
  rc = RmSearchTasks(taskforce, (int(*)(RmTask,...))&taskforce_Start_aux1, &all_done, &count);

  Debug(dbg_Create, ("waiting for components of %s to settle down", name));
  for ( ; count > 0; count--)
   Wait(&all_done);

  Debug(dbg_Create, ("testing for successful start-up of %s", name));
  if (rc eq Err_Null)
   rc = RmSearchTasks(taskforce, &taskforce_Start_aux2);

  if (rc eq Err_Null)
   { Debug(dbg_Create, ("starting thread to monitor taskforce %s", name));
     unless(Fork(Monitor_Stack, &taskforce_Monitor, sizeof(RmTaskforce), taskforce))
      rc = EC_Error + EG_NoMemory + EO_Taskforce;
   }

  if (rc eq Err_Null)
   { task_entry->UseCount++;
     RmSetTaskforceState(taskforce, RmS_Running);
     taskforce->StructType = RmL_Executing;
   }
  else
   { Debug(dbg_Create, ("aborting start-up of taskforce %s, error was %x", name, rc));
     MRSW_FreeRead();	/* BLV - temporary patch to avoid deadlocks */
     (void) RmApplyTasks(taskforce, &taskforce_Start_aux3);
     (void) RmApplyTasks(taskforce, &taskforce_Start_aux4);
     MRSW_GetRead();
   }
  return(rc);
}
/*}}}*/
/*{{{  taskforce_Start_aux1() - start a thread for every component */
/**
*** taskforce_Start_aux1(). This routine is responsible for spawning a
*** separate thread for every component that is not already running. The
*** main routine needs to know how many threads have been spawned, and
*** the various threads need to know about the semaphore in the main routine.
**/
static word taskforce_Start_aux1(RmTask task, ...)
{ va_list	args;
  Semaphore	*all_done;
  int		*count;

  va_start(args, task);
  all_done = va_arg(args, Semaphore *);
  count    = va_arg(args, int *);
  va_end(args);

  if (task->StructType eq RmL_Executing)
   return(0);

  unless(Fork(Monitor_Stack, &component_Handler, 8, task, all_done))
   return (EC_Error + EG_NoMemory + EO_Task);

  *count += 1;
  return(Err_Null);
}
/*}}}*/
/*{{{  taskforce_Start_aux2() - check every component */
/**
*** Once all the threads for the various components have signalled a
*** steady state, this routine checks that the steady state was a success
*** rather than a failure. In addition it is possible to do some cleaning-up
*** because all the components are now running, hence things like resident
*** library references are not needed anymore.
**/
static int taskforce_Start_aux2(RmTask task, ...)
{ TaskEntry	*task_entry = GetTaskEntry(task);
  int		i;

  for (i = 0; (i < MaxLibrariesPerComponent) && 
	      (task_entry->ComponentCode.Libraries[i] ne Null(Object));
       i++)
   { Close(task_entry->ComponentCode.Libraries[i]);
     task_entry->ComponentCode.Libraries[i] = NULL;
   }

  if (task->StructType ne RmL_Executing)
   return(task->ReturnCode);

  return((int)Err_Null);
}
/*}}}*/
/*{{{  taskforce_Start_aux3() - abort every component */
/**
*** This routine is called when a taskforce start-up failed. All components
*** are in one of three possible states:
***  a) never started up, the taskforce start-up failed before or during
***     this component
***  b) started normally, but already terminated
***  c) running normally, waiting for environment 
*** The only other thread that can access the task during this operation is
*** the monitor routine for that task. By locking the Task component it
*** is possible to synchronise with that thread. Note that the component
*** is not yet in the /tfm directory structure nor attached to an RmLib
*** connection.
**/
static int taskforce_Start_aux3(RmTask task, ...)
{ TaskEntry	*task_entry;

  if (task->StructType ne RmL_Executing)
   return(0);

  Wait(&(task->ObjNode.Lock));
  task_entry = GetTaskEntry(task);
  if (task_entry->ProgramStream ne Null(Stream))
   { task->ReturnCode = (int)(EC_Fatal + SS_TFM + EG_Create + EO_Taskforce);
     AbortPort(task_entry->ProgramStream->Reply, EC_Fatal + SS_TFM + EG_Create + EO_Taskforce);
   }
  else
   task->StructType = RmL_Terminated;
  Signal(&(task->ObjNode.Lock));
  return(0);
}

/*}}}*/
/*{{{  taskforce_Start_aux4() - wait for component abort */
/**
*** It is essential that a taskforce start-up is an atomic operation. Either
*** it succeeds completely with all components running, or it fails
*** completely with all components aborted. aux3() above will have
*** caused any running components to abort. This routine waits for the
*** ProgramObject to go to NULL, a sure sign that the monitor thread
*** has aborted.
**/
static int taskforce_Start_aux4(RmTask task, ...)
{ TaskEntry	*task_entry;
  Object	*x;

  forever
   { Wait(&(task->ObjNode.Lock));
     task_entry = GetTaskEntry(task);
     x = task_entry->ProgramObject;
     Signal(&(task->ObjNode.Lock));
     if (x eq NULL)
      break;
     else
      { Debug(dbg_Create, ("waiting for component %T to abort", task));
        Delay(OneSec / 10);
      }
   }
  return(0);
}
/*}}}*/
/*{{{  determine_code()       - load-from-neighbour etc. */
/**
*** determine_code(). This routine is responsible for determining where
*** to get the code from for a particular component. The name of the code
*** is determined, as is the processor type. A search is then made of
*** the taskforce up to this component task for another component with the
*** same requirements. If successful that task will be used as the source
*** for the code and resident libraries. Otherwise this task will be a
*** starting point.
**/
static int determine_code_aux(RmTask, ...);

static word determine_code(RmTask task, ...)
{ char		*code_name = (char *) RmGetTaskCode(task);
  RmProcessor	 processor;
  RmTask	 matching_task;
  TaskEntry	*task_entry = GetTaskEntry(task);

  if (task->ObjNode.Flags & TfmFlags_Special)
   return(Err_Null);

  if (code_name eq NULL)
   { Debug(dbg_Create, ("missing code specification for component %T", task));
     return(EC_Error + EG_Unknown + EO_Program);
   }
  processor = RmFollowTaskMapping(Domain, task);
  if (processor eq (RmProcessor) NULL)
   { Debug(dbg_Create, ("missing processor mapping for component %T", task));
     return(EC_Error + EG_Unknown + EO_Processor);
   }

  matching_task = (RmTask) RmSearchTasks(task->Root, &determine_code_aux, task, 
		code_name, RmGetProcessorType(processor));
  if (matching_task ne task)
   task_entry->ComponentCode.StartingPoint = matching_task;
  else
   { task_entry->Program = Locate(Null(Object), code_name);
     if (task_entry->Program eq Null(Object))
      { Debug(dbg_Create, ("cannot locate program %s", code_name));
        return(EC_Error + EG_Unknown + EO_Program);
      }
   }
  return(Err_Null);
}

static int determine_code_aux(RmTask task, ...)
{ va_list	args;
  RmTask	source_task;
  char		*code_name;
  int		processor_type;
  RmProcessor	processor;

  va_start(args, task);
  source_task    = va_arg(args, RmTask);
  code_name  = va_arg(args, char *);
  processor_type = va_arg(args, int);
  va_end(args);

	/* stop the search as soon as possible */
  if (task eq source_task) return((int) task);
  if (task->ObjNode.Flags & TfmFlags_Special) return(0);
  if (strcmp(code_name, (char *) RmGetTaskCode(task)))
   return(0);
  processor = RmFollowTaskMapping(Domain, task);
  if (processor_type ne RmGetProcessorType(processor))
   return(0);
  return((int) task);
}
/*}}}*/
/*{{{  component_Handler()    - responsible for a single component */
/**
*** This routine is responsible for managing a single component of a
*** taskforce. It performs the following operations.
*** a) start up the component. There are two possibilities here: the code
***    may have to be loaded from disk, or it may have to fetched from
***    another processor. In the former case it is necessary to keep track
***    of the various resident libraries.
*** b) create the pipes for this component, if any.
*** c) inform the parent taskforce that this component is running.
*** d) monitor the component. This may be interrupted for signals and
***    environment information.
*** e) clean up the component.
***
*** Care has to be taken with the locking. If the routine fails to start
*** up the component then it must abort the component before waking up
*** the parent, to avoid confusion.
**/
/*{{{  component_LoadFromDisk() */
/**
*** This routine is responsible for loading a piece of code from disk.
*** In addition it keeps track of the resident libraries that were
*** loaded before the code started running and the resident libraries
*** afterwards. Any resident libraries that are not normally part of the
*** nucleus and which have had their usage count incremented are recorded.
**/
typedef struct	LibraryInfo {
	int	Count;
	char	Name[NameMax];
} LibraryInfo;
static LibraryInfo *build_library_info(Object *loader);

static word  component_LoadFromDisk(Object *processor, RmTask task)
{ TaskEntry	*task_entry	= GetTaskEntry(task);
  Object	*loader;
  word		 rc		= Err_Null;
  LibraryInfo	*original;
  LibraryInfo	*final		= NULL;

  loader = Locate(processor, "loader");
  if (loader eq Null(Object))
   { Debug(dbg_Create, ("failed to locate %O/loader", processor));
     return(EC_Error + EG_Unknown + EO_Loader);
   }

  original = build_library_info(loader);

  task_entry->LoadedCode = Load(loader, task_entry->Program);
  if (task_entry->LoadedCode eq Null(Object))
   { rc = Result2(loader);
     if (rc eq Err_Null) rc = EC_Error + EG_Broken + EO_Loader;
     Debug(dbg_Create, ("failed to load %O into %O", task_entry->Program, loader));
   }
  elif ((original ne Null(LibraryInfo)) &&
        ((final = build_library_info(loader)) ne Null(LibraryInfo)))
   { int 	i, j, k;
     Object	**libraries = task_entry->ComponentCode.Libraries;

		/* for every library that was loaded at the end		*/
     for (i = 0, k = 0; final[i].Count ne -1; i++)
      { 	/* check whether it was already loaded and compare usage*/
	for (j = 0; original[j].Count ne -1; j++)
         if (!strcmp(final[i].Name, original[j].Name))
	  { if (final[i].Count eq original[j].Count)
             goto skip_this_one;
	    else
	     break;
	  }

		/* at this point final[i] refers to a library that was	*/
		/* newly-loaded or whose usage count has changed	*/
	libraries[k] = Locate(loader, final[i].Name);
	if (libraries[k] ne Null(Object))
         k++;
skip_this_one:
	j = j;
      }
   }

  if (original ne Null(LibraryInfo)) Free(original);
  if (final ne Null(LibraryInfo))    Free(final);   
  Close(loader);

  AddTail(&(task_entry->ComponentCode.List), &(task_entry->ComponentCode.Node));
  Signal(&(task_entry->ComponentCode.Ready));
  return(rc);
}

/**
*** Construct a table of the current resident library usage in a particular
*** processor. This requires the following:
*** 1) open a stream to the appropriate /loader directory
*** 2) determine the directory size, malloc a buffer, and read it in
*** 3) for every resident library except kernel, syslib, servlib, and util,
***    record the usage count. This requires an ObjectInfo
*** 4) malloc another buffer containing details of just the required
***    libraries, and fill it in.
***
*** If this code fails for any reason that is not a disaster, as it will
*** merely slow down the taskforce loading a bit.
**/
static LibraryInfo *build_library_info(Object *loader)
{ Stream	*s = Open(loader, Null(char), O_ReadOnly);
  word		 dirsize;
  int		 i, j;
  DirEntry	*direntries = NULL;
  ObjInfo	 info;
  LibraryInfo	*result = NULL;
  int		 libraries = 0;
  bool		 changes;

  if (s eq Null(Stream)) return(NULL);
  dirsize = GetFileSize(s);
  if (dirsize < 0) goto done;

  direntries = (DirEntry *) Malloc(dirsize);
  if (direntries eq NULL) goto done;

  if (Read(s, (BYTE *) direntries, dirsize, -1) ne dirsize) goto done;
  Close(s); s = Null(Stream);

  dirsize /= sizeof(DirEntry);
  for (i = 0; i < dirsize; i++)
   if (direntries[i].Type ne Type_Module)
    direntries[i].Type = -1;
   elif ((!strcmp(direntries[i].Name, "Kernel")) ||
	 (!strcmp(direntries[i].Name, "SysLib")) ||
	 (!strcmp(direntries[i].Name, "ServLib")) ||
	 (!strcmp(direntries[i].Name, "Util")) )
    direntries[i].Type = -1;
   elif (ObjectInfo(loader, direntries[i].Name, (BYTE *) &info) < Err_Null)
    direntries[i].Type = -1;
   else
    { direntries[i].Type = info.Account;
      libraries++;
    }

  result = (LibraryInfo *) Malloc(sizeof(LibraryInfo) * ((word)libraries + 1));
  if (result eq NULL) goto done;

  for (i = 0, j = 0; i < dirsize; i++)
   if (direntries[i].Type ne -1)
    { strcpy(result[j].Name, direntries[i].Name);
      result[j].Count = (int) direntries[i].Type;
      j++;
    }
  result[j].Count = -1;	/* terminator */

	/* unfortunately the libraries have to be sorted in order of usage */
	/* given the small number of libraries, bubble sort is fine	   */
  for (changes = TRUE; changes; )
   { changes = FALSE;
     for (i = 0; i < (j - 1); i++)
      if (result[i].Count < result[i+1].Count)
       { char buf[NameMax];
	 int  temp;
         strcpy(buf, result[i].Name);
	 temp = result[i].Count;
	 strcpy(result[i].Name, result[i+1].Name);
	 result[i].Count = result[i+1].Count;
	 strcpy(result[i+1].Name, buf);
	 result[i+1].Count = temp;
	 changes = TRUE;
       }
   }

done:
  if (s ne Null(Stream)) Close(s);
  if (direntries ne NULL) Free(direntries);
  return(result);
}

/*}}}*/
/*{{{  component_LoadFromNeighbour() */
/**
*** This code is used to load a program from one processor to another,
*** plus any resident libraries that are required. The TaskEntry structure 
*** contains a pointer to the task that is the starting point for this
*** particular program. It is necessary to wait for another task to have
*** loaded itself and added itself to the appropriate list. Then the
*** resident libraries can be loaded, followed by the actual code.
***
*** The routine attempts to perform a true load-from-neighbour, by
*** scanning the list for a neighbouring processor. If that fails
*** then some arbitrary processor will be selected.
***
*** This routine involves no less than three RmTasks. my_task is the one
*** currently being loaded. starting_task is the first task involving this
*** program and this processor type, and would have been loaded off disk.
*** other_task is some other task involving the same program and processor
*** type which has been loaded, directly or indirectly, from the
*** starting_task.
**/
static word  component_LoadFromNeighbour(Object *proc, RmProcessor processor,
				 RmTask task)
{ Object	*loader = Locate(proc, "loader");
  TaskEntry	*my_task_entry;
  TaskEntry	*start_task_entry;
  TaskEntry	*other_task_entry;
  ComponentCode	*other_component_code;
  ComponentCode *my_component_code;
  RmTask	 starting_point;
  RmTask	 other_task;
  RmProcessor	 other_processor;
  int		 number_links, link, destlink;
  int		 i;
  word		 rc = Err_Null;

  if (loader eq Null(Object))
   { Debug(dbg_Create, ("failed to access %O/loader", proc));
     return(EC_Error + EG_Unknown + EO_Loader);
   }

  my_task_entry     = GetTaskEntry(task);
  my_component_code = &(my_task_entry->ComponentCode);
  starting_point    = my_component_code->StartingPoint;
  start_task_entry  = GetTaskEntry(starting_point);

	/* unlock the processor while waiting for another task to load	*/
  Signal(&(processor->ObjNode.Lock));
  Wait(&(start_task_entry->ComponentCode.Ready));
  Wait(&(processor->ObjNode.Lock));

	/* Get details of another processor sharing the code		*/
	/* The list is locked, and the code walks down it finding the	*/
	/* corresponding processor. If that processor is a neighbour of	*/
	/* the current one then it is used.				*/
  Wait(&(start_task_entry->ComponentCode.StructLock));

  for ( other_component_code = Head_(ComponentCode, start_task_entry->ComponentCode.List);
	!EndOfList_(other_component_code);
	other_component_code = Next_(ComponentCode, other_component_code))
   { other_task_entry = (TaskEntry *) (((BYTE *) other_component_code) - offsetof(TaskEntry, ComponentCode));
     other_task	      = other_task_entry->Task;
     other_processor  = RmFindProcessor(Domain, other_task->MappedTo);
     number_links     = RmCountLinks(processor);
     for (link = 0; link < number_links; link++)
      if (other_processor eq RmFollowLink(processor, link, &destlink))
       goto found;
   }
found:

	/* If no neighbouring processor is found, take an arbitrary one	*/
  if (EndOfList_(other_component_code))
   other_component_code = (ComponentCode *) RemHead(&(start_task_entry->ComponentCode.List));
  else
   Remove(&(other_component_code->Node));

  Signal(&(start_task_entry->ComponentCode.StructLock));
  other_task_entry = (TaskEntry *) (((BYTE *) other_component_code) - offsetof(TaskEntry, ComponentCode));
  other_task	   = other_task_entry->Task;

	/* other_component_code now contains full details of the code	*/
	/* and resident libraries used by this program. These can be	*/
	/* loaded, together with the actual program.			*/
  for (i = 0; (i < MaxLibrariesPerComponent) && 
              (other_component_code->Libraries[i] ne Null(Object));
       i++)
   { 
     my_component_code->Libraries[i] = Locate(loader, objname(other_component_code->Libraries[i]->Name));
     if (my_component_code->Libraries[i] eq Null(Object))
      my_component_code->Libraries[i] = Load(loader, other_component_code->Libraries[i]);
     if (my_component_code->Libraries[i] eq Null(Object))
      { Debug(dbg_Create, ("error, failed to load %O into %O", \
		other_component_code->Libraries[i], loader));
        rc = Result2(loader);
	goto fail;
      }
   }

	/* now load the actual code.					*/
  my_task_entry->LoadedCode = Locate(loader, objname(other_task_entry->LoadedCode->Name));
  if (my_task_entry->LoadedCode eq Null(Object))
   my_task_entry->LoadedCode = Load(loader, other_task_entry->LoadedCode);   
  if (my_task_entry->LoadedCode eq Null(Object))
   { Debug(dbg_Create, ("failed to load %O into %O", other_task_entry->LoadedCode, loader));
     rc = Result2(loader);
     goto fail;
   }

	/* Now add the other task_entry back to the list, and add mine	*/
	/* as well.							*/
  Wait(&(start_task_entry->ComponentCode.StructLock));
  AddTail(&(start_task_entry->ComponentCode.List), &(other_component_code->Node));
  AddTail(&(start_task_entry->ComponentCode.List), &(my_component_code->Node));
  Signal(&(start_task_entry->ComponentCode.StructLock));
  Signal(&(start_task_entry->ComponentCode.Ready));
  Signal(&(start_task_entry->ComponentCode.Ready));
  Close(loader);
  return(Err_Null);

fail:
  if (loader != Null(Object)) Close(loader);
  return(rc);
}
/*}}}*/
/*{{{  component_CreatePipes() */
/**
*** Create the communication streams for this taskforce.
*** For every task, check every channel.
*** 1) if it is not connected, leave it.
*** 2) if it is external, i.e. a file, create a stream to that file creating
***    it if necessary.
*** 3) if it is a pipe, check the other end. If the Uid is less then the
***    pipe already exists. If the uid is greater then create it and set up
***    both ends. If they are the same, check the channel numbers.
**/
static word component_handle_file(RmTask, int channel);
static void build_pipe_name(char *buffer);

static word	component_CreatePipes(RmTask task, RmProcessor processor, Object *real_processor)
{ int		number_channels;
  int		i;
  RmTask	dest;
  int		destlink;
  TaskEntry	*task_entry;
  word		rc;
  bool		created_pipe = FALSE;
          
  Debug(dbg_Create, ("creating standard streams for component %T", task));
  		
  task_entry = GetTaskEntry(task);
  
  number_channels = RmCountChannels(task);
  if (number_channels eq -1)
   { rc = EC_Error + EG_Broken + EO_Taskforce; goto done; }

	/* For every channel in this component */
  for (i = 0; i < number_channels; i++)
   { TaskEntry	*its_task_entry;
     dest = RmFollowChannel(task, i, &destlink);

	/* If not connected, ignore it */
     if (dest eq RmM_NoTask) continue;
     
    	/* external channels are filenames of some sort. */
     if (dest eq RmM_ExternalTask)
      { rc = component_handle_file(task, i);
        if (rc ne Err_Null) goto done;
        continue;
      }

	/* The taskforce must have been fully resolved before it reaches here */
     if (destlink eq RmM_AnyChannel)
      { rc = EC_Error + EG_Invalid + EO_Pipe; goto done; }

     if ((dest->Uid < task->Uid) ||
         ((dest->Uid eq task->Uid) && (destlink < i)))
      { Debug(dbg_Create, ("channel %d, done elsewhere", i));
      	 continue;
      }
     its_task_entry = GetTaskEntry(dest);

	/* It is necessary to create a new pipe. This pipe is created on*/
	/* the same processor as the component, for efficiency.	If this	*/
	/* is the first time that a pipe has been created for this	*/
	/* component it is a good idea to check whether the pipe server	*/
	/* is already loaded and, if not, load it from this processor	*/
     if ((PipeCode ne Null(Object)) && !created_pipe)
      { Object *new_code = Locate(real_processor, "loader/pipe");
        Object *loader;
        if (new_code eq Null(Object))
         { loader = Locate(real_processor, "loader");
           new_code = Load(loader, PipeCode);
           Close(loader);
	 }
		/* exploit code persistence (10 seconds)	*/
        if (new_code ne Null(Object)) Close(new_code);
      }
     { Object		*pipe;
       char		buffer[NameMax * 2];
       Stream		*my_stream = Null(Stream);
       Stream		*its_stream = Null(Stream);

       build_pipe_name(buffer);
       Debug(dbg_Create, ("channel %d, creating %s in processor %O", i, \
		buffer, real_processor));

       pipe	= Create(real_processor, buffer, Type_Pipe, 0, Null(BYTE));
       if (pipe eq Null(Object))
	{ Debug(dbg_Create, ("failed to create pipe, fault %x", \
	 		Result2(real_processor)));
	  MarkProcessor(processor);
	  rc = EC_Error + EG_Create + EO_Pipe;
	  goto done;
	}

		/* Stop PseudoStream() from performing a real Open	*/
	pipe->Flags &= ~Flags_OpenOnGet;

	my_stream	= PseudoStream(pipe, O_ReadWrite);
        if (my_stream ne Null(Stream))
 	 its_stream	= PseudoStream(pipe, O_ReadWrite);
        if ((my_stream eq Null(Stream)) ||(its_stream eq Null(Stream)))
         { if (my_stream  ne Null(Stream)) Close(my_stream);
           if (its_stream ne Null(Stream)) Close(its_stream);
	   Delete(pipe, Null(char));
           Debug(dbg_Create, ("failed to open both sides of %O, fault %x", \
           	pipe, Result2(pipe)));
           Close(pipe);
           rc = EC_Error + EG_NoMemory + EO_Pipe;
           goto done;
         }
        Close(pipe);
        its_task_entry->Streams[destlink] = its_stream;
        task_entry->Streams[i] = my_stream;
        its_stream->Flags	|= (TfmFlags_InternalStream + Flags_Stream + Flags_OpenOnGet);
        my_stream->Flags	|= (TfmFlags_InternalStream + Flags_Stream + Flags_OpenOnGet);
        its_stream->Flags       &= ~Flags_CloseOnSend;
        my_stream->Flags	&= ~Flags_CloseOnSend;
      }
   }
   
  rc = Err_Null;
    
done:
  return(rc);
}

/**
*** building a unique name for a pipe. This is quite tricky, because there
*** may be multiple components of one taskforce on the same processor
*** as well as components from different taskforces. Hence I cheat by
*** using a simple static counter.
**/
static void	build_pipe_name(char *buffer)
{ static	int counter = 1;
  strcpy(buffer, "pipe/x");
  Wait(&LibraryLock);
  addint(buffer, counter++);
  Signal(&LibraryLock);
}

/**
*** Handle redirection to/from a file. This involves extracting
*** the file name etc. from the RmTask structure. The file name can
*** take two forms:
*** 1) @xxxxxxxxxxxxxxxxxx/helios/tmp:xx
***    This has a capability for the directory /helios/tmp
*** 2) /helios/tmp/xx
***    This is an absolute pathname without capability.
**/
static word	component_handle_file(RmTask task, int channel)
{ int		mode;
  const char	*filename = RmFollowChannelToFile(task, channel, &mode);
  TaskEntry	*task_entry = GetTaskEntry(task);

  if (filename eq Null(const char)) return(EC_Error + EG_Broken + EO_Taskforce);

  Debug(dbg_Create, ("channel %d is a file %s, open mode %x", channel,\
  	filename, mode));
  	
  if (filename[0] eq '@')
   { Capability		Access;
     Object		*directory; 
     char		*objname;
     char		*dirname;
     Stream		*result;
     
     dirname		= DecodeCapability((char *) filename, &Access);
     for (objname = dirname; (*objname ne ':') && (*objname ne '\0');
          objname++);
     if (*objname eq '\0') return(EC_Error + EG_Broken + EO_Taskforce);
     *objname = '\0';
      
     directory		= NewObject(dirname, &Access);
     *objname++		= ':';
     if (directory eq Null(Object)) return(EC_Error + EG_Invalid + EO_Capability);
     result		= Open(directory, objname, mode);
     Close(directory);
     if (result eq Null(Stream))
      { Debug(dbg_Create, ("failed to open file %s in directory %O, fault %x",\
                    objname, directory, Result2(directory)));
        return(EC_Error + EG_Open + EO_File);
      }
      
     result->Flags |= TfmFlags_InternalStream;
     if (mode & O_Append) (void) Seek(result, S_End, 0);
    
     task_entry->Streams[channel] = result;
     return(Err_Null);
   }
  else
   { char	*obj_name = objname((char *) filename);
     Object	*directory;
     Stream	*result;
     
     obj_name[-1] = '\0';
     directory	  = Locate(Null(Object), (char *) filename);
     obj_name[-1] = '/';

     if (directory eq Null(Object)) return(EC_Error + EG_Unknown + EO_Directory);
     result	  = Open(directory, obj_name, mode);
     Close(directory);
     if (result eq Null(Stream))
      { Debug(dbg_Create, ("failed to open file %s in directory %O, fault %x",\
      		obj_name, directory, Result2(directory)));
        return(EC_Error + EG_Open + EO_File);
      }
     result->Flags	|= TfmFlags_InternalStream;
     if (mode & O_Append) (void) Seek(result, S_End, 0);
      
     task_entry->Streams[channel] = result;
   }
  return(Err_Null); 
}

/*}}}*/

static void component_Handler(RmTask task, Semaphore *all_done)
{ TaskEntry	*task_entry = GetTaskEntry(task);
  RmProcessor	 processor  = RmFollowTaskMapping(Domain, task);
  Object	*proc;
  Object	*procman    = NULL;
  word		 rc;
  char		*name	    = Taskname(task);

	/* prevent too many simultaneous accesses to a given processor	*/
  Wait(&(processor->ObjNode.Lock));

	/* Switch from RmLib to Helios world				*/
  proc = TfmMapProcessorToObject(processor);
  if (proc eq Null(Object))
   { Debug(dbg_Create, ("component %s, failed to access processor %P", name, processor));
     rc = EC_Error + EG_Invalid + EO_Processor;
     goto done_startup;
   }
  procman = Locate(proc, "tasks");
  if (procman eq Null(Object))
   { Debug(dbg_Create, ("component %s, failed to locate %O/tasks", name, proc));
     rc = EC_Error + EG_Unknown + EO_ProcMan;
     goto done_startup;
   }

  unless (task->ObjNode.Flags & TfmFlags_Special)
   { Debug(dbg_Create, ("running task %s on processor %O", name, proc));

	/* Load all the code and libraries into memory			*/
     if (task_entry->Program ne Null(Object))
      rc = component_LoadFromDisk(proc, task);
     else
      rc = component_LoadFromNeighbour(proc, processor, task);
     if (rc ne Err_Null) goto done_startup;

	/* The component can now be executed and opened.		*/
     task_entry->ProgramObject = Execute(procman, task_entry->LoadedCode);
     if (task_entry->ProgramObject eq Null(Object))
      { rc = Result2(procman);
        if (rc eq Err_Null) rc = EC_Error + EG_Create + EO_Task;
        Debug(dbg_Create, ("failed to execute %O in %O, fault %x", \
		task_entry->LoadedCode, procman, rc));
        goto done_startup;
      }

     task_entry->ProgramStream = Open(task_entry->ProgramObject, NULL, O_ReadWrite);
     if (task_entry->ProgramStream eq Null(Stream))
      { rc = Result2(task_entry->ProgramObject);
        if (rc  eq Err_Null) rc = EC_Error + EG_Open + EO_Task;
        Debug(dbg_Create, ("failed to open %O, fault %x", \
		task_entry->ProgramObject, rc));
        goto done_startup;
      }
   }
  else
   { char *tid = (char *) RmGetObjectAttribute((RmObject) task, "tid", TRUE);
     if (tid eq NULL)
      { Debug(dbg_Create, ("special task %T, no tid attribute", task));
	rc = EC_Error + EG_Unknown + EO_Task;
	goto done_startup;
      }
     task_entry->ProgramObject = Locate(NULL, tid);
     if (task_entry->ProgramObject eq NULL)
      { Debug(dbg_Create, ("special task %T, failed to locate %s", task, tid));
        rc = EC_Error + EG_Invalid + EO_Task;
	goto done_startup;
      }
   }
	/* Mark the task as running. This affects signal handling etc.	*/
  RmSetTaskState(task, RmS_Running);

	/* With an open stream the task is unlikely to go away at this	*/
	/* point. Failures could be a problem here.			*/
  rc = component_CreatePipes(task, processor, proc);

	/* If successful mark the task as executing. This determines	*/
	/* whether or not the taskforce got started.			*/
  if (rc eq Err_Null)	
   task->StructType = RmL_Executing;

done_startup:
	/* let other threads manipulate this processor	*/
  Signal(&(processor->ObjNode.Lock));

	/* inform the parent about the state of this task */
  task->ReturnCode = task->ReturnCode = (int)rc;

	/* perform some clean-ups			*/
  if (proc ne Null(Object))    Close(proc);
  if (procman ne Null(Object)) Close(procman);

	/* If the component is running, monitor it and wake up the parent */
  if (rc eq Err_Null)
   { Signal(all_done); /* component is in a stable state */
     task_entry->UseCount++;
     task_Monitor(task);
   }

	/* When the monitor returns the task has terminated, so it is	*/
	/* possible to release some more data. In particular the TFM	*/
	/* can shut down the program that was running and hence let	*/
	/* the processor free some memory early on. This code may also	*/
	/* run if the task was created but, for example, it proved	*/
	/* impossible to create all the pipes.				*/
  if (task_entry->ProgramStream ne Null(Stream))
   { SendSignal(task_entry->ProgramStream, SIGKILL);
     Close(task_entry->ProgramStream);
     task_entry->ProgramStream = Null(Stream);
   }

  if (task_entry->ProgramObject ne Null(Object))
   { task_entry->ProgramObject->FnMod |= 2;
     Delete(task_entry->ProgramObject, Null(char));
     Close(task_entry->ProgramObject);
     task_entry->ProgramObject = Null(Object);
   }

	/* on a failure the taskforce_Start() routine cannot be woken	*/
	/* up until this component is in a stable state, i.e. until now */
  if (rc ne Err_Null)
   Signal(all_done);
}
/*}}}*/
/*}}}*/
/*{{{  taskforce_Monitor() */
/**
*** 3) taskforce_Monitor()
***
*** This routine waits for the termination of an entire taskforce.
*** Whenever a component is terminated or aborted a Semaphore will
*** be signalled. This routine waits for every component to signal,
*** and then takes care of the proginfo etc.
**/
static int	taskforce_Monitor_aux2(RmTask task, ...);

void	taskforce_Monitor(RmTaskforce Taskforce)
{ int		count;
  TaskEntry	*task_entry = (TaskEntry *) RmGetTaskforcePrivate(Taskforce);
  word		flags;
  TaskWaiter	*waiter;

  count = RmCountTasks(Taskforce);
  Debug(dbg_Monitor, ("monitoring taskforce %T", Taskforce));
  while (count-- > 0)
   { Wait(&(task_entry->Finished));
     Debug(dbg_Monitor, ("taskforce %T, %d components still running",\
     		Taskforce, count));
   }

  Wait(&(Taskforce->DirNode.Lock));

  MRSW_GetWrite();
  RmSetTaskforceState(Taskforce, RmS_Finished);
  Taskforce->StructType		 = RmL_Terminated;
  Taskforce->DirNode.Account	+= 1000;
  Taskforce->ReturnCode		 = RmSearchTasks(Taskforce, 
  					&taskforce_Monitor_aux2);
  taskforce_GenProgInfo(Taskforce);
  Taskforce->DirNode.Account	+= 1000;
  flags = Taskforce->DirNode.Flags;
  task_entry->UseCount--;

  for (waiter = Head_(TaskWaiter, task_entry->Waiting);	!EndOfList_(waiter); waiter = Next_(TaskWaiter, waiter))
   if (waiter->Sem ne NULL)
    Signal(waiter->Sem);

  if (task_entry->UseCount eq 0)
   taskforce_Destroy(Taskforce);
  else
   Signal(&(Taskforce->DirNode.Lock));

  MRSW_FreeWrite();

  if (flags & TfmFlags_FirstTask)
   while (!Fork(Terminate_Stack, TerminateTFM, 0));
}

static int	taskforce_Monitor_aux2(RmTask task, ...)
{ 
  return(task->ReturnCode);
}
/*}}}*/
/*{{{  taskforce_GenProgInfo() */
/**
*** 4) taskforce_GenProgInfo()
***    This is called when all the components of a taskforce have
***    terminated.
**/
void	taskforce_GenProgInfo(RmTaskforce Taskforce)
{ TaskEntry	*task_entry = (TaskEntry *) RmGetTaskforcePrivate(Taskforce);
  MCB		m;

  if ((task_entry->ProgInfoMask ne 0) && (task_entry->ProgInfoPort ne NullPort))
   { Debug(dbg_Delete,("generating termination message for taskforce %T",\
   		Taskforce));
     InitMCB(&m, MsgHdr_Flags_preserve, task_entry->ProgInfoPort, NullPort,
   		Taskforce->ReturnCode);
     Taskforce->DirNode.Account	+= 100;
     PutMsg(&m);
     Taskforce->DirNode.Account += 100;
   }
}
/*}}}*/
/*{{{  taskforce_Destroy() */
/**
*** 5) taskforce_Destroy()
***    Destroy a Taskforce. This means destroying every single task in the
***    taskforce, including any pipes that may have been set up. This is not
***    allowed if the taskforce is still running. First the taskforce is
***    removed from the /bart/tfm directory so that nothing else can access
***    it. Then the various resources owned by this taskforce are freed,
***    most of them by walking down the taskforce.
**/

static int	taskforce_DestroyAux(RmTask task, ...);
static int	taskforce_DestroyAux2(RmTask task, ...);
		
void	taskforce_Destroy(RmTaskforce Taskforce)
{ TaskEntry	*task_entry = (TaskEntry *) RmGetTaskforcePrivate(Taskforce);

  if (task_entry eq Null(TaskEntry)) return;
  
  if (task_entry->UseCount > 0)
   { report("internal error, attempt to destroy taskforce %T while use count is %d",
		Taskforce, task_entry->UseCount);
     return;
   }

  Debug(dbg_Delete, ("destroying taskforce %T", Taskforce));
  if (Taskforce->DirNode.Parent ne NULL)
   { Unlink((ObjNode *) &(Taskforce->DirNode), FALSE);
     Taskforce->DirNode.Parent = Null(DirNode);
   }

  (void) RmApplyTasks(Taskforce, &taskforce_DestroyAux);
  if (task_entry->Mapped) domain_UnmapTaskforce(Taskforce);
  (void) RmApplyTasks(Taskforce, &taskforce_DestroyAux2);

  if (task_entry->ProgInfoPort ne NullPort) FreePort(task_entry->ProgInfoPort);
  Free(task_entry);

  Debug(dbg_Delete, ("freeing taskforce %T", Taskforce));
  RmFreeTaskforce(Taskforce);
  Debug(dbg_Delete, ("taskforce freed"));
}

static int	taskforce_DestroyAux(RmTask task, ...)
{ TaskEntry	*task_entry;
  int		i;
  
  task_entry = GetTaskEntry(task);
  if (task_entry eq Null(TaskEntry)) return(RmE_Success);
  for (i = 0; task_entry->Streams[i] ne Null(Stream); i++)
   { Stream	*stream = task_entry->Streams[i];
     if (stream eq (Stream *) MinInt) continue;
     
     if (stream->Type ne Type_Pipe)	/* file redirection */
      Close(stream);
     else
      { Object	*o = NewObject(stream->Name, &(stream->Access));
        Close(stream);
        if (o ne Null(Object))
         { (void) Delete(o, Null(char)); Close(o); }
      }
   }
  if (task_entry->ProgramStream ne Null(Stream))
   Close(task_entry->ProgramStream);
  if (task_entry->ProgramObject ne Null(Object))
  { task_Exterminate(task);
    Close(task_entry->ProgramObject);
  }
  if (task_entry->LoadedCode ne Null(Object))
   Close(task_entry->LoadedCode);
  if (task_entry->Program ne Null(Object))
   Close(task_entry->Program);

  for (i = 0; (i < MaxLibrariesPerComponent) &&
              (task_entry->ComponentCode.Libraries[i] ne Null(Object));
       i++)
   Close(task_entry->ComponentCode.Libraries[i]);

  return(RmE_Success);
}

static int	taskforce_DestroyAux2(RmTask task, ...)
{ TaskEntry	*task_entry;
  
  task_entry = GetTaskEntry(task);
  if (task_entry eq Null(TaskEntry)) return(RmE_Success);
  RmSetTaskPrivate(task, 0);
  Free(task_entry);
  return(RmE_Success);
}
/*}}}*/
/*{{{  taskforce_DoSignal() */
/**
*** 6) taskforce_DoSignal()
***    A signal sent to a taskforce is simply propagated to all its components.
**/

static int	taskforce_DoSignalAux(RmTask task, ...);

void	taskforce_DoSignal(RmTaskforce Taskforce, word Signal)
{ Debug(dbg_Signal, ("sending signal %d to taskforce %T", Signal, \
		Taskforce));
  (void) RmApplyTasks(Taskforce, &taskforce_DoSignalAux, Signal);
  Debug(dbg_Signal, ("sent signal to taskforce %T", Taskforce));
}

static int	taskforce_DoSignalAux(RmTask task, ...)
{ va_list	args;
  WORD		signal;
   
  va_start(args, task);
  signal = va_arg(args, WORD);
  va_end(args);

  if (RmGetTaskState(task) & RmS_Running)
   task_DoSignal(task, signal);

  return(0);
}
/*}}}*/
/*{{{  taskforce_HandleEnv() */
/**
*** 7) taskforce_HandleEnv()
***    This takes care of an environment sent to a taskforce. The environment
***    has to be accepted, propagated with suitable modifications to all
***    the components, and then freed.
**/
/*{{{  HandleSpecial() : send Dup2 commands to a controller */
static word	taskforce_HandleSpecialEnv(RmTask task)
{ TfmConn	 connection;
  TaskEntry	*task_entry;
  int		 i;

  task_entry	= (TaskEntry *) RmGetTaskforcePrivate(RmRootTaskforce(task));
  connection	= task_entry->Connection;
  task_entry	= GetTaskEntry(task);
  for (i = 0; task_entry->Streams[i] ne NULL; i++)
   { if (task_entry->Streams[i] ne (Stream *) MinInt)
      { Debug(dbg_Environ, ("task %T, channel %d, requesting a dup2", task, i));
	unless(CreateDup2(connection, i, task_entry->Streams[i]))
         return(EC_Error + EG_Create + EO_Stream);
      }
   }

  for (i = 0; task_entry->Streams[i] ne NULL; i++)
   if (task_entry->Streams[i] ne (Stream *) MinInt)
    { Close(task_entry->Streams[i]);
      task_entry->Streams[i] = (Stream *) MinInt;
    }

  return(Err_Null);
}
/*}}}*/
/*{{{  HandleEnvAux() : send the environment to one component */
static word	taskforce_HandleEnvAux(RmTask task, ...)
{ va_list	args;
  Environ	*received;
  TaskEntry	*task_entry;
  Object	*Objv[OV_End + 1];
  Environ	sending;
  Stream	*program_stream;
  word		rc;
  char		*default_args[8];
    
  va_start(args, task);
  received = va_arg(args, Environ *);
  va_end(args);
  
  task_entry		= GetTaskEntry(task);
  sending.Strv		= task_entry->Streams;
  { int		i;

  	/* standard streams that have not been overloaded are	*/
  	/* inherited anyway, e.g. stderr			*/
    for (i = 0; (task_entry->Streams[i] ne Null(Stream)) &&
    		(received->Strv[i] ne Null(Stream));
         i++)
     if (task_entry->Streams[i] eq (Stream *) MinInt)
      task_entry->Streams[i] = received->Strv[i];
  }
  sending.Envv		= received->Envv;
  sending.Objv		= Objv;
  sending.Argv		= NULL;
  unless(task_FilterArgs(task, &sending, received->Argv, default_args))
   { rc = EC_Error + SS_TFM + EG_NoMemory; goto done; }

  Objv[OV_Cdir]		= received->Objv[OV_Cdir];
  Objv[OV_Task]		= task_entry->ProgramObject;
  Objv[OV_Code]		= task_entry->LoadedCode;
  if (task_entry->Program ne Null(Object))
   Objv[OV_Source]	= task_entry->Program;
  else
   { RmTask code_start  = task_entry->ComponentCode.StartingPoint;
     Objv[OV_Source]    = GetTaskEntry(code_start)->Program;
   }
  Objv[OV_Parent]	= received->Objv[OV_Parent];
  Objv[OV_Home]		= received->Objv[OV_Home];
  Objv[OV_Console]	= received->Objv[OV_Console];
  Objv[OV_CServer]	= received->Objv[OV_CServer];
  Objv[OV_Session]	= received->Objv[OV_Session];
  Objv[OV_TFM]		= received->Objv[OV_TFM];
  Objv[OV_TForce]	= received->Objv[OV_TForce];
  Objv[OV_End]		= Null(Object);

  if (DebugOptions & dbg_Environ)
   { int	i;
     report("sending environment to component %T", task);
     for (i = 0; sending.Argv[i] ne Null(char); i++)
      report("argument %d is %s", i, sending.Argv[i]);
     for (i = 0; sending.Envv[i] ne Null(char); i++)
      report("environment string %d is %s", i, sending.Envv[i]);
     for (i = 0; sending.Strv[i] ne Null(Stream); i++)
      if (sending.Strv[i] ne (Stream *) MinInt)
       report("stream %d is %S", i, sending.Strv[i]);
     for (i = 0; sending.Objv[i] ne Null(Object); i++)
      if (sending.Objv[i] ne (Object *) MinInt)
       report("object %d is %O", i, sending.Objv[i]);
   }
      
  program_stream = task_entry->ProgramStream;
  rc = SendEnv(program_stream->Server, &sending);

  Debug(dbg_Environ, ("sent environment to component %T", task));

done:
  { Stream	**streams;
    for (streams = sending.Strv; *streams ne Null(Stream); streams++)
     if (*streams ne (Stream *) MinInt)
      { if ((*streams)->Flags & TfmFlags_InternalStream)
         (void) Close(*streams); 
        *streams = (Stream *) MinInt; 
      }
  }
  if ((sending.Argv ne NULL) && (sending.Argv ne default_args))
   Free(sending.Argv);
  return(rc);
}
/*}}}*/
/*{{{  ParallelEnv()  : send an environment in parallel */
/**
*** Sending an environment in parallel. Environments can take up a lot
*** of memory and can involve heavy communication traffic. Hence it is
*** desirable to limit the number of environments being sent in parallel.
***
*** The code works as follows:
***  ParallelEnv() applies aux1 to all component tasks. The result of
***    the apply is the number of threads spawned to send environments.
***    It has to wait for all these threads to complete. The number of
***    arguments passed to the apply function exceeds three, so a 
***    temporary structure is used.
***  ParallelEnvAux1() waits for the number of active threads to drop
***     below the maximum. Then it Fork()'s off a thread to handle this
***     particular task's environment.
***  ParallelEnvAux2() sends the environment and updates the semaphores.
***
BLV The number of parallel environments should be based on the amount
BLV of memory available, not an arbitrary 16
**/

#define MaxEnvironments		16
typedef struct	env_details {
	Semaphore	Limit;
	Semaphore	Done;
	int		Rc;
	Environ		*Env;
} env_details;

	/* This routine is Fork()'ed off during the search of the taskforce */
	/* It calls the routine to actually send the environment and	    */
	/* updates the semaphores.					    */
static void taskforce_ParallelEnv_aux2(RmTask task, env_details *details)
{ word rc;

  if (task->ObjNode.Flags & TfmFlags_Special)
   rc = taskforce_HandleSpecialEnv(task);
  else
   rc = taskforce_HandleEnvAux(task, details->Env);

  if ((rc ne Err_Null) && (details->Rc eq Err_Null))
   details->Rc = (int)rc;
  Signal(&(details->Limit));
  Signal(&(details->Done));
}

	/* This routine is called during the search of the taskforce.	*/
	/* It blocks while the maximum number of threads are running,	*/
	/* then spawns another thread to send the environment to this	*/
	/* particular task.						*/
static int taskforce_ParallelEnv_aux1(RmTask task, ...)
{ va_list	args;
  env_details	*details;

  va_start(args, task);
  details = va_arg(args, env_details *);
  va_end(args);

	/* block if there are already 16 environments being sent	*/
  Wait(&(details->Limit));
  unless(Fork(ParallelEnv_Stack, &taskforce_ParallelEnv_aux2, 8, task, details))
   { if (details->Rc eq Err_Null)
      details->Rc = (int)(EC_Error + EG_NoMemory + EO_Taskforce);
     return(0);
    }
  return(1);
}

static int taskforce_ParallelEnv(RmTaskforce taskforce, Environ *env)
{ env_details	details;
  int		number_threads;

  InitSemaphore(&(details.Limit), MaxEnvironments);
  InitSemaphore(&(details.Done), 0);
  details.Rc  = (int)Err_Null;
  details.Env = env;

  number_threads = RmApplyTasks(taskforce, &taskforce_ParallelEnv_aux1, &details);
  for ( ; number_threads > 0; number_threads--) Wait(&(details.Done));
  return(details.Rc);
}		
/*}}}*/

word	taskforce_HandleEnv(RmTaskforce Taskforce, Environ *received)
{ int		rc;
  int		length;
  char		*buffer;
  Capability	cap;
          
	/* The Taskforce Manager must zap one entry in the object	*/
	/* vector, that for the taskforce itself. This must be done	*/
	/* without actually accessing the taskforce because that would	*/
	/* deadlock. The name should be : /Cluster/00/bart/tfm/job.6	*/
  length = strlen(ProcessorName) + strlen(Root.Name) +
  	strlen(Taskforce->DirNode.Name) + 7;
  buffer = (char *) Malloc(length);
  if (buffer eq Null(char)) { rc = RmE_ServerMemory; goto done; }
  strcpy( buffer, ProcessorName);
  pathcat(buffer, Root.Name);
  strcat( buffer, "/tfm/");
  strcat( buffer, Taskforce->DirNode.Name);
  NewCap(&cap, (ObjNode *) &Root, AccMask_Full);
  if ((received->Objv[OV_TForce] ne Null(Object)) &&
      (received->Objv[OV_TForce] ne (Object *) MinInt))
   Close(received->Objv[OV_TForce]);
   
  received->Objv[OV_TForce] = NewObject(buffer, &cap);
  Free(buffer);
  if (received->Objv[OV_TForce] eq Null(Object))
   { rc = RmE_ServerMemory; goto done; }  

  Debug(dbg_Environ, ("sending environment to taskforce %T", Taskforce));
  rc = taskforce_ParallelEnv(Taskforce, received);

done:  
  if (rc ne RmE_Success)
   return(EC_Error + EG_NoMemory + EO_Message);
  else
   return(Err_Null);
}
/*}}}*/




