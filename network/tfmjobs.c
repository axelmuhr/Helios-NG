/*------------------------------------------------------------------------
--                                                                      --
--           H E L I O S   N E T W O R K I N G   S O F T W A R E	--
--           ---------------------------------------------------	--
--                                                                      --
--             Copyright (C) 1990, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- tfmjobs.c								--
--                                                                      --
--	This module of the Taskforce Manager deals with RmLib requests.	--
--                                                                      --
--	Author:  BLV 4/9/90						--
--                                                                      --
------------------------------------------------------------------------*/
/* RcsId: $Header: /hsrc/network/RCS/tfmjobs.c,v 1.23 1994/03/01 12:35:03 nickc Exp $*/

/*{{{  header files */
#include <stdio.h>
#include <syslib.h>
#include <stddef.h>
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
#include <sys/socket.h>
#include <sys/un.h>
#include "exports.h"
#include "private.h"
#include "netutils.h"
#include "rmlib.h"
#include "tfmaux.h"
/*}}}*/
/*{{{  statics and initialisation */
bool	DomainLocked;
int	Socket_stoc;
int	Socket_ctos;

void	InitJobs(void)
{
#ifdef SingleProcessor
  DomainLocked = TRUE;
#else
  DomainLocked = FALSE;
#endif

  Socket_ctos = Socket_stoc = -1;
}
/*}}}*/
/*{{{  accept new connections */

/*------------------------------------------------------------------------------
*** The main purpose of the TFM is to support requests generated
*** by the Resource Management library. This is done via Unix-domain sockets.
*** The TFM has a thread running continuously accepting new
*** connections from clients, and this will start up a ConnectionGuardian thread
*** per connection. The TFM provides a more general interface than the
*** Execute() facilities of the System Library and Processor Manager.
**/
static	void	connection_guardian(TfmConn);

void	AcceptConnections(void)
{ struct sockaddr_un	address;
  int			next_connection = 0;
  int			len;
  Capability		cap;

  Socket_ctos	= socket(AF_UNIX, SOCK_STREAM, 0);
  Socket_stoc	= socket(AF_UNIX, SOCK_STREAM, 0);
  if ((Socket_ctos < 0) || (Socket_stoc < 0))
   fatal("unable to set up sockets");

  address.sun_family	= AF_UNIX;
  address.sun_path[0]	= '.';
  strcpy(&(address.sun_path[1]), Root.Name);
  strcat(address.sun_path, "_ctos");
  len = sizeof(address.sun_family) + strlen(address.sun_path) + 1;

  if (bind(Socket_ctos, (struct sockaddr * ) &address, len) ne 0)
   fatal("failed to bind socket name");

  address.sun_family	= AF_UNIX;
  address.sun_path[0]	= '.';
  strcpy(&(address.sun_path[1]), Root.Name);
  strcat(address.sun_path, "_stoc");
  len = sizeof(address.sun_family) + strlen(address.sun_path) + 1;

  if (bind(Socket_stoc, (struct sockaddr *) &address, sizeof(address)) ne 0)
   fatal("failed to bind socket name");

  listen(Socket_ctos, SOMAXCONN);
  listen(Socket_stoc, SOMAXCONN);

  forever
   { TfmConn	new_conn = (TfmConn)Malloc(sizeof(TfmConnStruct));
     if (new_conn eq (TfmConn) NULL)
      { Delay(2 * OneSec); continue; }

     InitSemaphore(&(new_conn->WriteLock), 1);
     InitList(&(new_conn->Processors));
     InitList(&(new_conn->Tasks));
     InitList(&(new_conn->Taskforces));
     new_conn->FullAccess	= FALSE;
     new_conn->Id		= next_connection++;

     while ((new_conn->Socket_ctos = accept(Socket_ctos, NULL, NULL)) < 0)
      Delay(5 * OneSec);

	/* Pray that the client has not crashed in between...	*/
     new_conn->Socket_stoc = accept(Socket_stoc, NULL, NULL);

	/* Perform some initial communication to ensure that things	*/
	/* are working.							*/
     if (read(new_conn->Socket_ctos, (BYTE *) &cap, sizeof(Capability))
		ne sizeof(Capability))
      goto fail;

     unless(GetAccess(&cap, TFM.Key) && (cap.Access & AccMask_D))
      goto fail;
     new_conn->FullAccess	= TRUE;

     if (read(new_conn->Socket_ctos, (BYTE *) &(new_conn->Program), sizeof(int)) ne sizeof(int))
     goto fail;

	/* See abort_socket() in tfm.c					*/
     if (new_conn->Program eq -1)
      { 	/* Shut down the Helios domain sockets			*/
        close(Socket_ctos);
        close(Socket_stoc);
	(void) write(new_conn->Socket_stoc, (BYTE *) &len, sizeof(int));
	return;
      }

     len = 0;
     if (write(new_conn->Socket_stoc, (BYTE *) &len, sizeof(int)) ne sizeof(int))
      goto fail;

	/* System library calls are currently preferred, because	*/
	/* timeouts are necessary for fault tolerance.			*/
     new_conn->Pipe_ctos = fdstream(new_conn->Socket_ctos);
     new_conn->Pipe_stoc = fdstream(new_conn->Socket_stoc);

	/* Spawn a connection guardian and accept the next connection	*/
     if (Fork(ConnectionGuardian_Stack, &connection_guardian, sizeof(TfmConn), new_conn))
      continue;

fail:
     close(new_conn->Socket_ctos);
     close(new_conn->Socket_stoc);
     Free(new_conn);
   }  
}

/*}}}*/
/*{{{  the ConnectionGuardian(), one per client */

/**
*** The ConnectionGuardian process. Essentially this is just another
*** dispatcher. It reads an integer JobId from the pipe, followed by
*** an RmRequest structure which may incorporate a network, taskforce,
*** processor, task, and variable data vector. The TfmConnection structure,
*** JobId, RmRequest structure, and a suitable RmReply structure are
*** passed on to the worker routines. For convenience the RmReply
*** is held on the stack of the Connection Guardian.
**/
static void HandleGetNetwork		( TfmConn, int, RmRequest *, RmReply *);
static void HandleProcessorAllocation	( TfmConn, int, RmRequest *, RmReply *);
static void HandleLastChange		( TfmConn, int, RmRequest *, RmReply *);
static void HandleIsProcessorFree	( TfmConn, int, RmRequest *, RmReply *);
static void HandleGetId			( TfmConn, int, RmRequest *, RmReply *);
#if !(defined(SingleProcessor)) && Native_Supported
static void HandleNativeStuff		( TfmConn, int, RmRequest *, RmReply *);
static void HandleConnectionStuff	( TfmConn, int, RmRequest *, RmReply *);
#endif
#if !(defined(SingleProcessor))
static void HandleLocking		( TfmConn, int, RmRequest *, RmReply *);
#endif
static void HandleGetTaskforce		( TfmConn, int, RmRequest *, RmReply *);
static void HandleGetTask		( TfmConn, int, RmRequest *, RmReply *);
static void HandleExecute		( TfmConn, int, RmRequest *, RmReply *);
static void HandleUpdate		( TfmConn, int, RmRequest *, RmReply *);
static void HandleWait			( TfmConn, int, RmRequest *, RmReply *);
static void HandleSendSignal		( TfmConn, int, RmRequest *, RmReply *);
static void HandleLeave			( TfmConn, int, RmRequest *, RmReply *);

typedef struct TfmRequestTableEntry {
	int			FnCode;
	bool			Synchronous;
	TfmRequestHandler	*handler;
} TfmRequestTableEntry;

TfmRequestTableEntry request_handlers[] =  
{
 { RmC_GetNetwork,	 	TRUE,	&HandleGetNetwork		},
 { RmC_ProcessorPermanent,	TRUE,	&HandleProcessorAllocation	},
 { RmC_ProcessorTemporary,	TRUE,	&HandleProcessorAllocation	},
 { RmC_ProcessorExclusive,	TRUE,	&HandleProcessorAllocation	},
 { RmC_ProcessorShareable,	TRUE,	&HandleProcessorAllocation	}, 
 { RmC_ProcessorBooked,		TRUE,	&HandleProcessorAllocation	},
 { RmC_ProcessorCancelled,	TRUE,	&HandleProcessorAllocation	},
 { RmC_LastChange,		TRUE,	&HandleLastChange		},
 { RmC_ObtainProcessor,		TRUE,	&HandleObtainProcessor		},
 { RmC_ReleaseProcessor,	TRUE,	&HandleReleaseProcessor		},
 { RmC_ObtainExactNetwork,	TRUE,	&HandleObtainNetwork		},
 { RmC_ObtainNetwork,		TRUE,	&HandleObtainNetwork		},
 { RmC_ReleaseNetwork,		TRUE,	&HandleReleaseNetwork		},
#ifndef SingleProcessor
 { RmC_Lock,			TRUE,	&HandleLocking			},
 { RmC_Unlock,			TRUE,	&HandleLocking			},
#endif
 { RmC_IsProcessorFree,		TRUE,	&HandleIsProcessorFree		},
 { RmC_ObtainProcessors,	TRUE,	&HandleObtainProcessors		},
 { RmC_ObtainExactProcessors,	TRUE,	&HandleObtainProcessors		},
#if !(defined(SingleProcessor)) && Native_Supported
 { RmC_Reboot,			TRUE,	&HandleNativeStuff		},
 { RmC_ResetProcessors,		TRUE,	&HandleNativeStuff		},
 { RmC_SetNative,		TRUE,	&HandleNativeStuff		},
 { RmC_Revert,			TRUE,	&HandleConnectionStuff		},
 { RmC_MakeConnections,		TRUE,	&HandleConnectionStuff		},
#endif
 { RmC_GetId,			TRUE,	&HandleGetId			},
 { RmC_ReportProcessor,		TRUE,	&HandleReportProcessor		},
 { RmC_GetTaskforce,		TRUE,	&HandleGetTaskforce		},
 { RmC_GetTask,			TRUE,	&HandleGetTask			},
 { RmC_Execute,			TRUE,	&HandleExecute			},
 { RmC_Update,			TRUE,	&HandleUpdate			},
 { RmC_SendSignal,		TRUE,	&HandleSendSignal		}, 
 { RmC_Leave,			TRUE,	&HandleLeave			},
 { RmC_Wait,			FALSE,	&HandleWait			},
 { -1, TRUE, NULL }
};

static word kill_task(Node *node);
static word kill_taskforce(Node *node);
static void guardian_aux(TfmConn, int job_id, int i, RmRequest *, RmReply *);

static void connection_guardian(TfmConn connection)
{ int		JobId;
  RmRequest	*request = NULL;
  RmReply	*reply	 = NULL;
  int		timeouts = 0;
  Stream	*pipe	 = connection->Pipe_ctos;  
  bool		broken	 = FALSE;

  while (!broken)
   { word rc;
     int i;

     while (request eq NULL)
      if ((request = New(RmRequest)) eq NULL)
	Delay(OneSec);
     while (reply eq NULL)
      if ((reply = New(RmReply)) eq NULL)
	Delay(OneSec);

     memset(request, 0, sizeof(RmRequest));
     memset(reply, 0, sizeof(RmReply));

     rc = Read(pipe, (BYTE *) &JobId, sizeof(int), -1);
     if (rc <= 0)
      { if (rc eq 0)
         { timeouts++;
           if (timeouts < 5) continue;
         }
        if ((Result2(pipe) & EG_Mask) eq EG_Timeout) 
         continue;
	else
 	 break;
      }

     timeouts = 0;
     if ((JobId & RmR_Private) ne 0)
      { if ((JobId & ~RmR_Private) eq RmR_Synch) continue;
        IOdebug("tfm : private protocol received down pipe");
        continue;
      }

     rc = FullRead(pipe, (BYTE *) request, sizeof(RmRequest), -1);
     if (rc < (int) sizeof(RmRequest)) break;

     if ((request->Network ne (RmNetwork) NULL) ||
         (request->Taskforce ne (RmTaskforce) NULL))
      if ((rc = RmReadStream(pipe, &(request->Network), &(request->Taskforce)))
          ne RmE_Success)
       break;

     if (request->Processor ne (RmProcessor) NULL)
      if ((rc = RmReadProcessor(pipe, &(request->Processor), FALSE))
          ne RmE_Success)
       break;

     if (request->Task ne (RmTask) NULL)
      if ((rc = RmReadTask(pipe, &(request->Task), FALSE))
          ne RmE_Success)
       break;

     if (request->VariableSize > 0)
      { request->VariableData = (char *)Malloc(request->VariableSize);
        if (request->VariableData eq NULL)
         break;
        if (FullRead(pipe, request->VariableData, request->VariableSize, -1) < 0)
	 break;
      }

     if (DebugOptions & dbg_Comms)
      report("request %x, jobid %x, Network %N,\n          Taskforce %T, Processor %P, Task %T, size %d",
		request->FnRc, JobId, request->Network, request->Taskforce, 
		request->Processor, request->Task,
		request->VariableSize);
      	
     MRSW_GetRead();

     for (i = 0; request_handlers[i].handler ne NULL; i++)
      if (request->FnRc eq request_handlers[i].FnCode)
       break;
     if (request_handlers[i].handler ne NULL)
      { if (request_handlers[i].Synchronous)
         (*request_handlers[i].handler)(connection, JobId, request, reply);
	else
	 { until(Fork(GuardianAux_Stack, &guardian_aux, 20, connection, JobId, i, request, reply))
		Delay(OneSec);
	   request = NULL; reply = NULL;
	 }
      }
     else
      { report("unexpected request %x", request->FnRc);
	broken = TRUE;
      }

     MRSW_FreeRead();

     if (request ne NULL)
      { if (request->Network ne (RmNetwork) NULL) RmFreeNetwork(request->Network);
        if (request->Taskforce ne (RmTaskforce) NULL) RmFreeTaskforce(request->Taskforce);
        if (request->Processor ne (RmProcessor) NULL) RmFreeProcessor(request->Processor);
        if (request->Task ne (RmTask) NULL) RmFreeTask(request->Task);
        if (request->VariableData ne NULL) Free(request->VariableData);
      }
   }

	/* Recovery code for communication failures, rather primitive	*/
	/* at present.							*/
  close(connection->Socket_ctos);
  close(connection->Socket_stoc);
  MRSW_GetWrite();
  (void) WalkList(&(connection->Processors), &AutomaticRelease);
  (void) WalkList(&(connection->Tasks), &kill_task);
  (void) WalkList(&(connection->Taskforces), &kill_taskforce);
  MRSW_FreeWrite();
  if (request ne NULL) Free(request);
  if (reply ne NULL) Free(reply);
  Free(connection);
}

/**
*** For some requests, particularly ones which can involve blocking such as
*** waiting for task or taskforce termination, the request has to be
*** handled in a separate thread from the ConnectionGuardian to avoid
*** blocking the application. This auxiliary routine performs the necessary.
**/
static void guardian_aux(TfmConn connection, int job_id, int i, RmRequest *request, RmReply *reply)
{
  MRSW_GetRead();
  (*request_handlers[i].handler)(connection, job_id, request, reply);
  MRSW_FreeRead();

  if (request->Network ne (RmNetwork) NULL) RmFreeNetwork(request->Network);
  if (request->Taskforce ne (RmTaskforce) NULL) RmFreeTaskforce(request->Taskforce);
  if (request->Processor ne (RmProcessor) NULL) RmFreeProcessor(request->Processor);
  if (request->Task ne (RmTask) NULL) RmFreeTask(request->Task);
  if (request->VariableData ne NULL) Free(request->VariableData);
  Free(request); Free(reply);
}

/*}}}*/
/*{{{  ReplyRmLib() */
int ReplyRmLib(TfmConn connection, int JobId, RmReply *reply)
{ int		rc = RmE_CommsBreakdown;
  Stream	*pipe;

  if (DebugOptions & dbg_Comms)
   report("reply %x, jobid %x, Network %N,\n          Taskforce %T, Processor %P, Task %T, size %d",
	reply->FnRc, JobId, reply->Network, reply->Taskforce,
	reply->Processor, reply->Task, reply->VariableSize);

  Wait(&(connection->WriteLock));
  pipe = connection->Pipe_stoc;

  if (Write(pipe, (BYTE *) &JobId, sizeof(int), -1) ne sizeof(int))
   goto fail;

  if (Write(pipe, (BYTE *) reply, sizeof(RmReply), -1) ne sizeof(RmReply))
   goto fail;

  if ((reply->Network ne (RmNetwork) NULL) ||
      (reply->Taskforce ne (RmTaskforce) NULL))
   if ((rc = RmWriteStream(pipe, reply->Network, reply->Taskforce, reply->Filter))
       ne RmE_Success)
    goto fail;

  if (reply->Processor ne (RmProcessor) NULL)
   if ((rc = RmWriteProcessor(pipe, reply->Processor, reply->Filter))
       ne RmE_Success)
    goto fail;

  if (reply->Task ne (RmTask) NULL)
   if ((rc = RmWriteTask(pipe, reply->Task, reply->Filter))
       ne RmE_Success)
    goto fail;

  if (reply->VariableSize > 0)
   if (Write(pipe, reply->VariableData, reply->VariableSize, -1)
       ne reply->VariableSize)
    { rc = RmE_CommsBreakdown; goto fail; }

  Signal(&(connection->WriteLock));
  return(RmE_Success);

fail:
#ifdef SYSDEB
  IOdebug("TfmJobs, ReplyRmLib: failed to send reply down pipe %s",
		pipe->Name);
#endif
  Signal(&(connection->WriteLock));
  return(rc);
}
/*}}}*/
/*{{{  CreateDup2() */
/**
*** This horrible piece of code is used to cause the client to open a
*** particular posix file descriptor. It is assumed that the client
*** side has used the KeepLocked flag to ensure that no other requests
*** can come in, and that the request is being handled synchronously.
**/
bool CreateDup2(TfmConn connection, int fd, Stream *stream)
{ Dup2Details	details;
  word		temp;

  details.FileDescriptor	= fd;
  DecodeCapability(details.Name, &(stream->Access));
  strcat(details.Name, stream->Name);

  Wait(&connection->WriteLock);
  temp	= RmR_Private | RmR_Dup2;
  Write(connection->Pipe_stoc, (BYTE *) &temp, sizeof(word), -1);
  Write(connection->Pipe_stoc, (BYTE *) &details, sizeof(Dup2Details), -1);
  Signal(&connection->WriteLock);
  Read(connection->Pipe_ctos, (BYTE *) &temp, sizeof(word), -1);
  return(temp);
}
/*}}}*/
/*{{{  clean up connections */
/**
*** When a top-level task or taskforce has terminated and the owning client has
*** been informed about this termination, the task or taskforce is automatically
*** detached from the connection. This is vaguely analogous to Unix where a
*** process can go away only when its parent has done a wait(). This code
*** is also used to detach a task or taskforce from a connection.
***
*** If the UseCount drops to zero as a result of detaching this task or
*** taskforce then the thingy is guaranteed to have terminated and to
*** have no outstanding clients. Hence it can be destroyed.
**/
static void remove_from_connection(RmTask task, TfmConn connection)
{ TaskEntry	*task_entry;

  if (RmIsTaskforce(task))
   task_entry = (TaskEntry *) RmGetTaskforcePrivate((RmTaskforce) task);
  else
   task_entry = GetTaskEntry(task);

  if (task_entry->Connection eq connection)
   { Remove(&(task_entry->ConnectionNode));
     task_entry->Connection = NULL;
     task_entry->UseCount--;
     if (task_entry->UseCount eq 0)
      { if (RmIsTaskforce(task))
         taskforce_Destroy((RmTaskforce) task);
        else
	 task_Destroy(task);
      }
   }
}

/**
*** When a connection with a client is broken it is necessary to reclaim
*** any resources associated with that client. This includes owned processors,
*** see AutomaticRelease() in tfmmap.c, and running tasks and taskforces.
*** The task or taskforce is detached from the connection, then destroyed
*** if no longer running, else exterminated.
**/
static word kill_task(Node *node)
{ TaskEntry	*task_entry;
  RmTask	task;

  task_entry	= (TaskEntry *) (((BYTE *)node) - offsetof(TaskEntry,ConnectionNode));
  task		= task_entry->Task;
  Remove(&(task_entry->ConnectionNode));
  task_entry->Connection = NULL;
  task_entry->UseCount--;
  if (task_entry->UseCount eq 0)
   task_Destroy(task);
  else
   task_Exterminate(task);
  return(0);  
}

static word kill_taskforce(Node *node)
{ TaskEntry	*task_entry;
  RmTaskforce	 taskforce;

  task_entry	= (TaskEntry *) (((BYTE *)node) - offsetof(TaskEntry, ConnectionNode));
  taskforce	= (RmTaskforce) task_entry->Task;

  Remove(&(task_entry->ConnectionNode));
  task_entry->Connection = NULL;
  task_entry->UseCount--;
  if (task_entry->UseCount eq 0)
   taskforce_Destroy(taskforce);
  else
   taskforce_DoSignal(taskforce, SIGKILL);
  return(0);
}
/*}}}*/
/*{{{  getting network details */
/**----------------------------------------------------------------------------
*** HandleGetNetwork(). This is used to obtain full details of the current
*** domain.
**/
static int  GetNetwork_NetworkFilter(RmNetwork, RmNetwork);
static int  GetNetwork_ProcessorFilter(RmProcessor, RmProcessor);

static void HandleGetNetwork(TfmConn Connection, int JobId,
				RmRequest *request, RmReply *reply)
{ RmFilterStruct	filter;

  filter.SendHardware	= FALSE;
  filter.Network	= &GetNetwork_NetworkFilter;
  filter.Processor	= &GetNetwork_ProcessorFilter;
  filter.Taskforce	= NULL;
  filter.Task		= NULL;
  reply->FnRc		= RmE_Success;
  reply->Network	= Domain;
  reply->Filter		= &filter;

  (void) ReplyRmLib(Connection, JobId, reply);
  request = request;
}

static int GetNetwork_NetworkFilter(RmNetwork real, RmNetwork copy)
{
  copy->DirNode.Key	= 0;
  copy->StructType	= RmL_Existing;
  if (real eq RmRootNetwork((RmProcessor) real))
   strcpy(copy->DirNode.Name, NetworkName);
  return(RmE_Success);
}

static int GetNetwork_ProcessorFilter(RmProcessor real, RmProcessor copy)
{
  copy->ObjNode.Key	= 0;
  copy->StructType	= RmL_Existing;
  memset(&(copy->NsCap), 0, sizeof(Capability));
  memcpy(&(copy->RealCap), &(copy->ReadOnlyCap), sizeof(Capability));
  copy->Private		= 0;
  real = real;
  return(RmE_Success);
}
/*}}}*/
/*{{{  processor allocation options */
/**----------------------------------------------------------------------------
*** Cope with requests to change the processor allocation strategy for a
*** particular processor.
**/
static void HandleProcessorAllocation(TfmConn Connection, int JobId, 
		RmRequest *request, RmReply *reply)
{ RmProcessor		Processor; 
  int			rc;

  Processor = RmFindProcessor(Domain, request->Uid);
  if ((Processor eq RmM_NoProcessor) || (Processor eq RmM_ExternalProcessor))
   { rc = RmE_NotFound; goto done; }
   
  unless(GetAccess(&(request->Cap), Processor->ObjNode.Key) &&
  	 (request->Cap.Access & AccMask_D))
   { rc = RmE_NoAccess; goto done; }
   
  switch(request->FnRc)
   { case RmC_ProcessorPermanent :
   		Processor->AllocationFlags |= RmF_Permanent;  break;
     case RmC_ProcessorTemporary :
		Processor->AllocationFlags &= ~RmF_Permanent; break;
     case RmC_ProcessorExclusive :
		{ DomainEntry *domain_entry = GetDomainEntry(Processor);
		  if (domain_entry->NumberUsers > 1)
		   { rc = RmE_InUse; goto done; }
		}
     		Processor->AllocationFlags	|= RmF_Exclusive;
		Processor->ApplicationId	 = Connection->Id;
		break;
     case RmC_ProcessorShareable :
     		if (Processor->Purpose eq RmP_Native)
     		 { rc = RmE_BadProcessor; goto done; }
     		Processor->AllocationFlags	&= ~RmF_Exclusive;
		Processor->ApplicationId	 = -1;
		break;
     case RmC_ProcessorBooked	 :
     		Processor->AllocationFlags |= RmF_Booked; break;
     case RmC_ProcessorCancelled :
     		Processor->AllocationFlags &= ~RmF_Booked; break;
   }
  rc = RmE_Success;

done:
  reply->FnRc = rc;
  (void) ReplyRmLib(Connection, JobId, reply);
}
/*}}}*/
/*{{{  time stamp for the last change */

/**----------------------------------------------------------------------------
*** Handle last change. A very simple routine to send back a single integer
**/
static void HandleLastChange(TfmConn Connection, int JobId, 
		RmRequest *request, RmReply *reply)
{ 
  reply->FnRc	= (int) RmE_Success;
  reply->Reply1	= (int) LastChange;
  (void) ReplyRmLib(Connection, JobId, reply);
  request = request;
}

/*}}}*/
/*{{{  locking the current domain */
/**----------------------------------------------------------------------------
*** Locking, this is fairly easy. It is not supported in single-processor
*** systems to ensure that the domain is always locked and the TFM never
*** tries to access the Network Server.
**/
#ifndef SingleProcessor
static	void HandleLocking(TfmConn Connection, int JobId,
		RmRequest *request, RmReply *reply)
{ 
  if (request->FnRc eq RmC_Lock)
   DomainLocked = TRUE;
  else
   DomainLocked = FALSE;

  reply->FnRc = RmE_Success;   
  ReplyRmLib(Connection, JobId, reply);
}
#endif
/*}}}*/
/*{{{  session and application ids */
/**----------------------------------------------------------------------------
*** Getting the Session or Application Id. 
**/
static void HandleGetId(TfmConn Connection, int JobId,
		RmRequest *request, RmReply *reply)
{
#ifndef SingleProcessor
  if (request->Arg1 > 0)
   reply->Reply1 = RmGetApplication();
  else
#endif
   reply->Reply1 = Connection->Id;
  reply->FnRc = RmE_Success;
  ReplyRmLib(Connection, JobId, reply);
}
/*}}}*/
/*{{{  is processor free ? */
/**-----------------------------------------------------------------------------
*** HandleIsProcessorFree(). This can be called with any existing processor,
*** without special authorisation, to get a snap shot of the processor's
*** availability. If the processor is currently in the user's domain then
*** exclusive access etc must be checked. Otherwise the Network Server
*** must be interrogated.
**/
static	void HandleIsProcessorFree(TfmConn Connection, int JobId,
		RmRequest *request, RmReply *reply)
{ RmProcessor		processor;
  int			rc, i;
  DomainEntry		*domain_entry;
    
  processor = RmFindProcessor(Domain, request->Uid);
  if (processor eq RmM_NoProcessor)	/* not in current domain */
   { processor = RmNewProcessor();
     if (processor eq (RmProcessor) NULL)
      { rc = RmE_ServerMemory; goto done; }
     processor->StructType = RmL_Existing;
     processor->Uid	   = request->Uid;
     if (RmIsProcessorFree(processor))
      rc = RmE_Success;
     else
      rc = RmE_InUse;
     RmFreeProcessor(processor);
     goto done;
   }

  domain_entry = GetDomainEntry(processor);
  if (domain_entry->NumberUsers >= MaxUsersPerProcessor)
   { rc = RmE_InUse; goto done; }

  for (i = 0; i < MaxUsersPerProcessor; i++)
   if (domain_entry->AllocationTable[i].Connection eq Connection)
    { rc = RmE_InUse; goto done; }

  if (processor->AllocationFlags & RmF_Exclusive)
   if (domain_entry->NumberUsers > 0)
    { rc = RmE_InUse; goto done; }
   
  rc = RmE_Success;

done:
  reply->FnRc	= rc;
  ReplyRmLib(Connection, JobId, reply);
}
/*}}}*/
#if !(defined(SingleProcessor)) && Native_Supported
/*{{{  set-native, reboot, reset */
/**-----------------------------------------------------------------------------
*** Native network support: set-native, reboot, and reset. 
***
*** The request contains a count of the number of processors affected,
*** and a table of processor details. The access matrices must
*** be verified. Then the operation must be verified: to set processors
*** to native the application must have exclusive access; to reset or
*** reboot processors the processors must currently be native. After
*** the validation the job is passed on to the Network Server, changing
*** the capabilities of course. The Network Server attempts to do the
*** necessary, and if successful or partially successful it will send
*** back a table of ProcessorUpdate's which must be used to update the
*** domain and then sent back to the application.
**/

static	void	HandleNativeStuff(TfmConn Connection, int JobId,
		RmRequest *request, RmReply *reply)
{ ProcessorDetails	*details = Null(ProcessorDetails);
  ProcessorUpdate	*updates = Null(ProcessorUpdate); 
  RmProcessor		processor;
  int			rc, i;
  int			count;

  MRSW_FreeRead();
  MRSW_GetWrite();

  count		= request->Arg1;
  details	= (ProcessorDetails *) request->VariableData;

  updates = Malloc(count * sizeof(ProcessorUpdate));
  if (updates eq Null(ProcessorUpdate))
   { rc = RmE_ServerMemory; goto done; }

  for (i = 0; i < count; i++)
   { if (details[i].Uid eq RmL_NoUid) continue;
     processor = RmFindProcessor(Domain, details[i].Uid);
     if (processor eq (RmProcessor) NULL)
      { rc = RmE_BadProcessor; goto done; }
     unless(GetAccess(&(details[i].Cap), processor->ObjNode.Key) &&
     		(details[i].Cap.Access & AccMask_D))
      { rc = RmE_NoAccess; goto done; }

	/* Update the capability for the Network Server */
     details[i].Cap	= processor->NsCap;
     
     if (request->FnRc eq RmC_SetNative)
      { unless(processor->AllocationFlags & RmF_Exclusive)
         { rc = RmE_NoAccess; goto done; }
      }
     else
      { unless(RmGetProcessorPurpose(processor) eq RmP_Native)
         { rc = RmE_BadProcessor; goto done; }
      }
   }     		           

		/* Now send the request off to the Network server */
		/* This is mostly copied from rmlib3.c		  */
  { RmRequest	out_request;
    RmReply	out_reply;

    Clear(out_request); Clear(out_reply);
    out_request.FnRc		= request->FnRc;
    out_request.Arg1		= count;
    out_request.VariableData	= (BYTE *) details;
    out_request.VariableSize	= count * sizeof(ProcessorDetails);
    out_reply.VariableData	= (BYTE *) updates;

    rc = RmXch(&RmParent, &out_request, &out_reply);
    if (rc eq RmE_CommsBreakdown) rc = RmE_MissingServer;
   }
         
done:
  if ((rc eq RmE_Success) || (rc eq RmE_PartialSuccess))
   for (i = 0; i < count; i++)
    { if (updates[i].Uid eq RmL_NoUid) continue;
      processor = RmFindProcessor(Domain, updates[i].Uid);
      if (processor eq (RmProcessor) NULL) continue;
      processor->ObjNode.Size = updates[i].State & ~RmS_Reset;
      processor->Purpose = updates[i].Purpose;
    }

  reply->FnRc		= rc;
  reply->VariableData	= (BYTE *) updates;
  if ((rc eq RmE_Success) || (rc eq RmE_PartialSuccess))
   reply->VariableSize	= count * sizeof(ProcessorUpdate);

  MRSW_FreeWrite();
  MRSW_GetRead();

  ReplyRmLib(Connection, JobId, reply);
  if (updates ne Null(ProcessorUpdate)) Free(updates);
}
/*}}}*/
/*{{{  reconfiguring links */
/**-----------------------------------------------------------------------------
*** Reconfiguration support
***
*** This refers to Revert and MakeConnections. Either way the data
*** consists of a table of three integers - count, exact, and preserve -
*** followed by a table of LinkConnections. The processors have to be
*** verified, and then the request is passed on to the Network Server.
*** If the Network Server succeeds then it will return a new table of
*** LinkConnections which must be used to update the domain and then
*** passed back to the application.
**/
static	void	HandleConnectionStuff(TfmConn Connection, int JobId,
		RmRequest *request, RmReply *reply)
{ LinkConnection	*conns;
  int			count;
  RmProcessor		processor;
  int			rc, i;

  MRSW_FreeRead();
  MRSW_GetWrite();

  count = request->Arg1;
  conns	= (LinkConnection *) request->VariableData;

  for (i = 0; i < count; i++)
   { processor = RmFindProcessor(Domain, conns[i].SourceUid);
     if (processor eq (RmProcessor) NULL)
      { rc = RmE_BadProcessor; goto done; }

     unless(GetAccess(&(conns[i].SourceCap), processor->ObjNode.Key) &&
            (conns[i].SourceCap.Access & AccMask_D))
      { rc = RmE_NoAccess; goto done; }

     conns[i].SourceCap = processor->NsCap;
     unless((conns[i].SourceLink >= 0) &&
            (conns[i].SourceLink < RmCountLinks(processor)))
      { rc = RmE_BadLink; goto done; }
      
     if ((conns[i].DestUid ne RmL_NoUid) &&
         (conns[i].DestUid ne RmL_ExtUid) &&
         (conns[i].DestCap.Access ne 0) &&
         (request->FnRc ne RmC_Revert))
      { processor = RmFindProcessor(Domain, conns[i].DestUid);
        if (processor eq (RmProcessor) NULL)
         { rc = RmE_BadProcessor; goto done; }
         
        unless(GetAccess(&(conns[i].DestCap), processor->ObjNode.Key) &&
               (conns[i].DestCap.Access & AccMask_D))
         { rc = RmE_NoAccess; goto done; }
        conns[i].DestCap = processor->NsCap;
        unless((conns[i].DestLink >= 0) &&
               (conns[i].DestLink < RmCountLinks(processor)))
         { rc = RmE_BadLink; goto done; }
      }
   }
		/* Now send the request off to the Network server */
		/* This is mostly copied from rmlib3.c		  */
  { RmRequest	out_request;
    RmReply	out_reply;    

    Clear(out_request); Clear(out_reply);
    out_request.FnRc		= request->FnRc;
    out_request.Arg1		= request->Arg1;	/* count	*/
    out_request.Arg2		= request->Arg2;	/* exact	*/
    out_request.Arg3		= request->Arg3;	/* preserve	*/
    out_request.VariableData	= (BYTE *) conns;
    out_request.VariableSize	= count * sizeof(LinkConnection);
    out_reply.VariableData	= (BYTE *) conns;

    rc = RmXch(&RmParent, &out_request, &out_reply);
   }

done:
  if ((rc eq RmE_Success) || (rc eq RmE_PartialSuccess))
   for (i = 0; i < count; i++)
    { RmProcessor source = RmFindProcessor(Domain, conns[i].SourceUid);
      RmLink	 *link;
     
      if (source eq (RmProcessor) NULL) continue;
      link = RmFindLink(source, conns[i].SourceLink);

	/* update the old neighbour, if any */
      { RmProcessor neighbour;
        int	    destlink;
        neighbour = RmFollowLink(source, conns[i].SourceLink, &destlink);
        if ((neighbour ne RmM_NoProcessor) && (neighbour ne RmM_ExternalProcessor))
         { RmLink *link = RmFindLink(neighbour, destlink);
           if ((link->Target eq source->Uid) &&
               (link->Destination eq conns[i].SourceLink))
            { link->Destination = -1;
              link->Target      = RmL_NoUid;
            }
         }
      }

	/* update this processor */      
      link->Destination	= conns[i].DestLink;
      link->Target	= conns[i].DestUid;
      
     	/* update the new neighbour, if any */
      if ((conns[i].DestUid ne RmL_NoUid) &&
          (conns[i].DestUid ne RmL_ExtUid))
       { source = RmFindProcessor(Domain, conns[i].DestUid);
         if (source eq (RmProcessor) NULL) continue;
         link = RmFindLink(source, conns[i].DestLink);
         link->Destination = conns[i].SourceLink;
         link->Target      = conns[i].SourceUid;
       }
    }

/*PrintNetwork(Domain);   */

  MRSW_FreeWrite();
  MRSW_GetRead();

  reply->FnRc		= rc;
  if ((rc eq RmE_Success) || (rc eq RmE_PartialSuccess))
   { reply->VariableData = (BYTE *) conns;
     reply->VariableSize = count * sizeof(LinkConnection);
   }
  ReplyRmLib(Connection, JobId, reply);
}
/*}}}*/
#endif	/* SingleProcessor */
/*{{{  get details of current task/taskforce */
/**----------------------------------------------------------------------------
*** Support for examining tasks and taskforces
**/
static void HandleGetTaskforce(TfmConn connection, int job_id,
			RmRequest *request, RmReply *reply)
{ RmTaskforce		taskforce;


	/* Search the /tfm subdirectory for the specified taskforce	*/
  taskforce = (RmTaskforce) Lookup(&TFM, request->VariableData, TRUE);
  if (taskforce eq (RmTaskforce) NULL)
   reply->FnRc  = RmE_NotTaskforce;
  elif (taskforce->DirNode.Type ne Type_Taskforce)
   reply->FnRc = RmE_NotTaskforce;
  else
   { reply->FnRc	= RmE_Success;
     reply->Taskforce	= taskforce;
   }
  ReplyRmLib(connection, job_id, reply);
}

static void HandleGetTask(TfmConn connection, int job_id,
			RmRequest *request, RmReply *reply)
{ RmTask	task;

	/* Search the /tfm subdirectory for the specified task	*/
  task = (RmTask) Lookup(&TFM, request->VariableData, TRUE);
  if (task eq (RmTask) NULL)
   reply->FnRc  = RmE_NotTask;
  elif (task->ObjNode.Type ne Type_Task)
   reply->FnRc = RmE_NotTask;
  else
   { reply->FnRc = RmE_Success;
     reply->Task = task;
   }
  ReplyRmLib(connection, job_id, reply);
}
/*}}}*/

/*{{{  execute a task or taskforce */

/*{{{  ExtractEnv() */

/**
*** Extracting an environment given a data vector containing the required
*** information. The first part of the data vector is four sets of tables,
*** representing offsets for argv, envv, objv, and strv. For argv and
*** envv these can be converted into pointers and included directly in
*** the main environment structure. The strv and objv vectors are
*** slightly trickier, and require converting the descriptor info to
*** real objects and streams.
**/
static bool ExtractEnv(Environ *env, BYTE *datavec)
{ int	*cvec = (int *) datavec;

	/* determine the starting points for the various bits,	*/
	/* including the actual data.				*/
  env->Argv = (char **) cvec;
  while (*cvec ne -1) cvec++;
  cvec++;
  env->Envv = (char **) cvec;
  while (*cvec ne -1) cvec++;
  cvec++;
  env->Objv = (Object **) cvec;
  while (*cvec ne -1) cvec++;
  cvec++;
  env->Strv = (Stream **) cvec;
  while (*cvec ne -1) cvec++;
  cvec++;
  datavec = (BYTE *) cvec;

  for (cvec = (int *) env->Argv; *cvec ne -1; cvec++)
   *cvec = (int) &(datavec[*cvec]);
  *cvec = NULL;

  for (cvec = (int *) env->Envv; *cvec ne -1; cvec++)
   *cvec = (int) &(datavec[*cvec]);
  *cvec = NULL;

  for (cvec = (int *) env->Objv; *cvec ne -1; cvec++)
   if (*cvec ne MinInt)
    { ObjDesc *o = (ObjDesc *) &(datavec[*cvec]);
      *cvec = (int) NewObject(o->Name, &(o->Cap));
      if (*cvec eq NULL) goto fail;
    }
  *cvec = NULL;

  for (cvec = (int *) env->Strv; *cvec ne -1; cvec++)
   if (*cvec ne MinInt)
    { if ((*cvec & 0xFFFF0000) eq 0xFFFF0000)
       { int ix = *cvec & 0x0000FFFF;
         *cvec = (int) (env->Strv[ix]);
       }
      else
      { StrDesc *s = (StrDesc *) &(datavec[*cvec]);
        int      openonget = (int) (s->Mode & Flags_OpenOnGet);
        s->Mode &= ~Flags_OpenOnGet;
        *cvec = (int) NewStream(s->Name, &(s->Cap), s->Mode);
        if (*cvec eq NULL) goto fail;
        ((Stream *)(*cvec))->Pos = s->Pos;
        if (openonget ne 0) ((Stream *)(*cvec))->Flags |= Flags_OpenOnGet;
      }
    }
  *cvec = NULL;

  if (DebugOptions & dbg_Environ)
   { int	i;
     report("received environment");
     for (i = 0; env->Argv[i] ne Null(char); i++)
      report("argument %d is %s", i, env->Argv[i]);
     for (i = 0; env->Envv[i] ne Null(char); i++)
      report("environment string %d is %s", i, env->Envv[i]);
     for (i = 0; env->Strv[i] ne Null(Stream); i++)
      if (env->Strv[i] ne (Stream *) MinInt)
       report("stream %d is %S", i, env->Strv[i]);
     for (i = 0; env->Objv[i] ne Null(Object); i++)
      if (env->Objv[i] ne (Object *) MinInt)
       report("object %d is %O", i, env->Objv[i]);
   }
  return(TRUE);

fail:
  Debug(dbg_Environ, ("error receiving environment"));
	/* BLV - the test for valid Objects/Streams leaves something to	*/
	/* be desired. Hopefully Close()'ing an invalid object does not	*/
	/* cause too many problems...					*/
  { int i, j;
    for (i = 0; env->Objv[i] != Null(Object); i++)
     { j = (int) env->Objv[i];
       if ((j ne MinInt) && ((j < 0) || (j > 2048)))
       Close(env->Objv[i]);
     }
    for (i = 0; env->Strv[i] != Null(Stream); i++)
     { j = (int) env->Strv[i];
       if ((j ne MinInt) && ((j < 0) || (j > 2048)))
       Close(env->Strv[i]);
     }
  }
  env->Argv = NULL;
  return(FALSE);
}

/*}}}*/
/*{{{  HandleExecuteTask() */

/**
*** This code is responsible for executing a single task, as requested
*** through the Resource Management library. Parameters are as follows:
*** 1) the Task field of the request is an RmTask structure which should
***    have been suitably initialised by the application.
*** 2) the Uid and Cap may refer to a processor or may be NULL, depending
***    on whether or not the application has mapped the task already
*** 3) the VariableData vector contains environment information. Note that
***    there is potential conflict between the arguments in the environment
***    vector and the arguments in the RmTask structure.
***
*** The work required is as follows.
*** a) the environment information has to be unpacked.
*** b) a TaskEntry structure has to be added to the RmTask
*** c) the argument vector of the environment and the task's arguments
***    have to be resolved
*** d) the task code has to be Locate()'ed, if possible
*** e) the task's name has to be resolved to avoid conflicts
*** f) the task has to be mapped. If a Uid has been supplied then this
***    must refer to a processor in the current domain.
*** g) the task can be executed in the usual way.
*** h) the environment information can now be sent.
*** i) the task has to be inserted into the /tfm subdirectory and
***    associated with this connection, so that it will be aborted if
***    the client disappears.
*** i) at this point the task is running, so the client can be sent a reply.
**/

static void HandleExecuteTask(TfmConn connection, int job_id,
		 RmRequest *request, RmReply *reply)
{ Environ	env;
  word		rc;
  RmTask	task		= request->Task;
  RmProcessor	processor;
  Object	*program	= Null(Object);
  TaskEntry	*task_entry	= NULL;
  char		*default_args[8];

  MRSW_FreeRead();
  MRSW_GetWrite();

  Debug(dbg_Create, ("request to execute a task"));

  env.Argv	= NULL;
  unless(ExtractEnv(&env, request->VariableData))
   { rc = RmE_ServerMemory; goto done; }

  if (task_AddTaskEntry(task) ne RmE_Success)
   { rc = RmE_ServerMemory; goto done; }
  task_entry = GetTaskEntry(task);

	/* Make certain that the task will not be freed automatically etc. */
  request->Task		= (RmTask) NULL;
  task->MappedTo	= RmL_NoUid;
  task->ObjNode.Parent	= NULL;

  { char	**junk = env.Argv;
    env.Argv = NULL;
    unless(task_FilterArgs(task, &env, junk, default_args))
     { rc = RmE_ServerMemory; goto done; }
  }

	/* the user may not have named the task yet...	*/
  if (strlen(task->ObjNode.Name) < 1)
   strcpy(task->ObjNode.Name, "<anon>");

	/* Sort out the binary for this program.			*/
	/* All binaries can be relative to the current directory, and	*/
	/* may be specified explicitly using RmSetTaskCode() or		*/
	/* implicitly using the task name.				*/
  { char *code_name = (char *) RmGetTaskCode(task);
    if (code_name eq Null(char)) code_name = task->ObjNode.Name;
    program = Locate(env.Objv[OV_Cdir], code_name);
    if (program eq Null(Object))
     { Debug(dbg_Create, ("failed to locate program %s", code_name));
       rc = RmE_NotFound;
       goto done;
     }
  }

	/*  see if there is already a task or taskforce with this name */
  { ObjNode *x = Lookup(&TFM, task->ObjNode.Name, TRUE);
    if (x ne Null(ObjNode))
     { Wait(&LibraryLock);
       addint(task->ObjNode.Name, TaskSequenceNumber++);
       Signal(&LibraryLock);
     }
  }

  Debug(dbg_Create, ("sorting out mapping for %T", task));
  if (request->Uid ne RmL_NoUid)
   { RmProcessor mappedto = RmFindProcessor(Domain, request->Uid);
     if (mappedto eq (RmProcessor) NULL)
      { rc = RmE_BadProcessor; goto done; }
     unless(GetAccess(&(request->Cap), mappedto->ObjNode.Key) &&
	    (request->Cap.Access & AccMask_D))
      { rc = RmE_NoAccess; goto done; }
     unless(domain_FillInTaskMapping(task, mappedto))
      { rc = RmE_InUse; goto done; }
   }
  else
   unless(domain_MapTask(task))
    { rc = RmE_NoResource; goto done; }

  processor = RmFollowTaskMapping(Domain, task);
  if (processor eq (RmProcessor) NULL)
   { rc = RmE_NoResource; goto done; }
  Debug(dbg_Create, ("task %T is mapped to %P", task, processor));

  rc = task_Run(processor, task, program);
  if (rc ne Err_Null)
    { Debug(dbg_Create, ("attempt to run task produced error code %x", rc ) );
     rc = RmE_BadProcessor; goto done; }

  Debug(dbg_Create, ("task %T is running", task));

  rc = task_HandleEnv(task, &env);
  if (rc ne Err_Null)
   { task_Exterminate(task);
     rc = RmE_BadProcessor;
   }
  else
   { Debug(dbg_Create, ("task %T has received its environment", task));
     AddTail(&(connection->Tasks), &(task_entry->ConnectionNode));
     task_entry->Connection = connection;
     task_entry->UseCount++;
     Insert(&TFM, &(task->ObjNode), FALSE);
     rc		 = RmE_Success;
     reply->Task = task;  
   }
  
done:
  if (rc eq RmE_Success)
   { Debug(dbg_Create, ("task %T is running", task)); }
  else
   { Debug(dbg_Create, ("failed to execute task %T, error %x", task, rc)); }

  reply->FnRc = (int) rc;
  ReplyRmLib(connection, job_id, reply);

  if ((rc ne RmE_Success) && (task->StructType eq RmL_New))
   { if (task->MappedTo ne RmL_NoUid)
      domain_UnmapTask(task);
     task_Destroy(task);     
   }

  MRSW_SwitchRead();

	/* Cleaning up the environment stuff. It is necessary to	*/
	/* consider the following possibilities:			*/
	/* 1) the supplied environment could not be extracted. env.Argv	*/
	/*    will be set to NULL.					*/
	/* 2) the supplied environment could be extracted. The argv	*/
	/*    vector will have been filtered, which may have resulted	*/
	/*    in a new argv vector.					*/
	/* Note that it is necessary to Close all the streams and	*/
	/* objects to release all the memory, and that some of the	*/
	/* environment data can overlay the supplied data vector.	*/
  if ((env.Argv ne NULL) && (env.Argv ne default_args))
   Free(env.Argv);
  if (env.Argv eq default_args)
   env.Argv = (BYTE **) request->VariableData;
  if (env.Argv ne NULL)
   { tfm_FreeEnv(&env); request->VariableData = NULL; }
}

/*}}}*/
/*{{{  HandleExecuteTaskforce() */

/**
*** This code is responsible for executing a taskforce, as requested
*** through the Resource Management library. Parameters are as follows:
*** 1) the Taskforce field of the request is an RmTaskforce structure which
***    should have been suitably initialised by the application
*** 2) the Network field may be NULL or it may contain a set of obtained
***    processors, with tasks mapped to particular processors
*** 3) the VariableData vector contains environment information.
***
*** The work required is as follows:
*** a) the environment information has to be unpacked
*** b) the code associated with every component has to be resolved. Often
***    this will be relative to the current directory.
*** c) a TaskEntry structure has to be added to all components and to the
***    top-level taskforce
*** d) the taskforce name has to be resolved
*** e) the taskforce has to be mapped. The exact details of this operation
***    depend on whether or not a network has been supplied.
*** f) the taskforce can be executed in the usual way
*** g) the environment information can be sent
*** h) the taskforce can be inserted into the /tfm subdirectory and
***    associated with this connection, so that it will be aborted if
***    the client disappears.
*** i) at this point the taskforce is running, so the client can be sent
***    a reply
**/

/*{{{  SortOutCode() */
/**
*** For flexibility, users need not specify an absolute pathname for
*** every component. In particular, I want to allow the following options:
*** 1) an absolute pathname as code. This pathname is not checked at
***    this point as it will be checked during the taskforce start-up,
***    and two Locate()'s for the same file is undesirable.
*** 2) a relative pathname as code. This pathname is assumed relative
***    to the current directory. The code name is altered to absolute, but
***    again no attempt is made to Locate() the program.
*** 3) no code is specified. In this case the component name is taken
***    as the relate pathname.
**/
static int taskforce_SortOutCode(RmTask task, ...)
{ va_list	args;
  Object	*cdir;
  char		*code_name;
  char		buf[IOCDataMax];

  if (task->ObjNode.Flags & TfmFlags_Special) return(RmE_Success);

  va_start(args, task);
  cdir = va_arg(args, Object *);
  va_end(args);

  code_name = (char *) RmGetTaskCode(task);
  if ((code_name ne NULL) && ((*code_name eq '/') || (*code_name eq '@')))
   return(RmE_Success);
  strcpy(buf, cdir->Name);
  if (code_name ne NULL)
   { pathcat(buf, code_name);
     RmRemoveObjectAttribute((RmObject) task, &(code_name[-5]), TRUE);
   } 
  else
   pathcat(buf, task->ObjNode.Name);
  return(RmSetTaskCode(task, buf));
}
/*}}}*/

static void HandleExecuteTaskforce(TfmConn connection, int job_id, 
			RmRequest *request, RmReply *reply)
{ Environ	env;
  word		rc;
  RmTaskforce	taskforce	= request->Taskforce;
  TaskEntry	*task_entry	= NULL;

  Debug(dbg_Create, ("request to execute a taskforce"));

  env.Argv	= NULL;
  unless(ExtractEnv(&env, request->VariableData))
   { rc = RmE_ServerMemory; goto done; }
  Debug(dbg_Create, ("got environment information from client"));

  if ((rc = RmSearchTasks(taskforce, &taskforce_SortOutCode, env.Objv[OV_Cdir])) ne RmE_Success)
   goto done;
  Debug(dbg_Create, ("determined binaries for this taskforce"));

	/* Make certain that the taskforce will not be freed automatically */
	/* It must go via taskforce_Destroy() to release the TaskEntry     */
	/* structures.							   */
  request->Taskforce	= (RmTaskforce) NULL;

  if ((task_AddTaskEntry((RmTask) taskforce) ne RmE_Success) ||
      (RmSearchTasks(taskforce, &task_AddTaskEntry) ne RmE_Success))
   { rc = RmE_ServerMemory; goto done; }
  task_entry = (TaskEntry *) RmGetTaskforcePrivate(taskforce);
  task_entry->Connection	= connection;
  AddTail(&(connection->Taskforces), &(task_entry->ConnectionNode));

	/* the user may not have named the taskforce yet...	*/
  if (strlen(taskforce->DirNode.Name) < 1)
   strcpy(taskforce->DirNode.Name, "<anon>");

  MRSW_FreeRead();
  MRSW_GetWrite();

	/*  see if there is already a task or taskforce with this name */
  { ObjNode *x = Lookup(&TFM, taskforce->DirNode.Name, TRUE);
    if (x ne Null(ObjNode))
     { Wait(&LibraryLock);
       addint(taskforce->DirNode.Name, TaskSequenceNumber++);
       Signal(&LibraryLock);
     }
  }

  Debug(dbg_Create, ("sorting out mapping for %T", taskforce));
  unless(domain_MapTaskforce(request->Network, taskforce))
   { rc = RmE_NoResource; goto done; }
  task_entry->Mapped = TRUE;

  MRSW_SwitchRead();
  rc = taskforce_Start(taskforce);
  MRSW_FreeRead();
  MRSW_GetWrite();
  if (rc ne Err_Null)
   { rc = RmE_BadProcessor; goto done; }

  Debug(dbg_Create, ("taskforce %T is running", taskforce));

  MRSW_SwitchRead();
  rc = taskforce_HandleEnv(taskforce, &env);
  MRSW_FreeRead();
  MRSW_GetWrite();

  if (rc ne Err_Null)
   { rc = RmE_BadProcessor; 
     taskforce_DoSignal(taskforce, SIGKILL);
     goto done;
   }

  Debug(dbg_Create, ("taskforce %T has received its environment", taskforce));
  task_entry->UseCount++;
  Insert(&TFM, (ObjNode *) &(taskforce->DirNode), FALSE);
  rc = RmE_Success;
  reply->Taskforce = taskforce;
  
done:
  if (rc eq RmE_Success)
   { Debug(dbg_Create, ("taskforce %T is running", taskforce)); }
  else
   { Debug(dbg_Create, ("failed to execute taskforce %T, error %x", taskforce, rc)); }

  reply->FnRc = (int) rc;
  ReplyRmLib(connection, job_id, reply);

	/* only destroy here if not running...			*/ 
  if (task_entry->UseCount eq  0)
   taskforce_Destroy(taskforce);

  MRSW_SwitchRead();
  if (env.Argv ne NULL)
   { tfm_FreeEnv(&env);
     request->VariableData = NULL;
   }
}

/*}}}*/

static void HandleExecute(TfmConn connection, int job_id, RmRequest *request, RmReply *reply)
{ if (request->Task ne (RmTask) NULL)
   HandleExecuteTask(connection, job_id, request, reply);
  else
   HandleExecuteTaskforce(connection, job_id, request, reply);
}

/*}}}*/
/*{{{  get the current state of a task or taskforce */

/**
*** This routine is used to take a snap-shot of the current state of one
*** or more tasks, taskforces, or taskforce components. The request vector
*** defines some number of these. The reply vector consists of a set of
*** TaskUpdate structures.
***
*** It is possible that the specified task(s) etc. no longer exist in
*** this TFM. This is because their use count had dropped to 0, i.e. they
*** were no longer attached to a connection and no threads were accessing
*** them. This is analogous to examining a Unix process that has terminated
*** and whose parent has performed a wait(). For simplicity the task is
*** assumed to have terminated with a zero return code.
***
*** If a top-level task or taskforce has terminated and is attached to
*** this connection it will be removed. The client now knows that the
*** task has terminated, and hence should not access it again.
**/


static void HandleUpdate(TfmConn connection, int job_id, RmRequest *request, RmReply *reply)
{ int		 count	 = request->VariableSize / sizeof(TaskDetails);
  TaskUpdate	*updates = NULL;
  TaskDetails	*details = (TaskDetails *) request->VariableData;
  int		 rc	 = RmE_Success;
  int		 i;
  RmTask	 task;
  RmTaskforce	 taskforce;

  MRSW_FreeRead();
  MRSW_GetWrite();

  if (count eq 0) goto done;

  updates = (TaskUpdate *) Malloc((word)count * sizeof(TaskUpdate));
  if (updates eq NULL)
   { rc = RmE_ServerMemory; goto done; }
  memset(updates, 0, count * sizeof(TaskUpdate));

  for (i = 0; i < count; i++)
   { task = (RmTask) Lookup(&TFM, details[i].Name, TRUE);
     if (task eq (RmTask) NULL)
      { updates[i].Errno	= RmE_NotFound;
        updates[i].StructType	= RmL_Done;
      }
     elif (RmIsTask(task))
      { unless(GetAccess(&(details[i].Cap), task->ObjNode.Key) &&
		(details[i].Cap.Access & AccMask_D))
	 { rc = RmE_NoAccess; goto done; }
	updates[i].StructType	= task->StructType;
	if (task->StructType eq RmL_Done)
	 { updates[i].ReturnCode = task->ReturnCode;
	   remove_from_connection(task, connection);
	 }
      }
     else	/* entry must be a taskforce */
      { taskforce = (RmTaskforce) task;
        if (details[i].Uid eq RmL_NoUid)
         { unless(GetAccess(&(details[i].Cap), taskforce->DirNode.Key) &&
			(details[i].Cap.Access & AccMask_D))
	    { rc = RmE_NoAccess; goto done; }
	   updates[i].StructType	= taskforce->StructType;
	   if (taskforce->StructType eq RmL_Done)
	    { updates[i].ReturnCode	= taskforce->ReturnCode;
	      remove_from_connection(task, connection);
	    }
	 }
	else
	 { task = RmFindTask(taskforce, details[i].Uid);
	   if (task eq (RmTask) NULL)
	    { updates[i].Errno		= RmE_NotFound;
	      updates[i].StructType	= RmL_Done;
	    }
	   else
	    { unless(GetAccess(&(details[i].Cap), task->ObjNode.Key) &&
			(details[i].Cap.Access & AccMask_D))
	       { rc = RmE_NoAccess; goto done; }
	      updates[i].StructType	= task->StructType;
	      if (task->StructType eq RmL_Done)
	       updates[i].ReturnCode	= task->ReturnCode;
	    }
	 }
      }
   }
  reply->VariableData	= (BYTE *) updates;
  reply->VariableSize	= count * sizeof(TaskUpdate);

done:
  MRSW_SwitchRead();
  reply->FnRc	= rc;
  ReplyRmLib(connection, job_id, reply);
  if (updates ne NULL) Free(updates);
}

/*}}}*/
/*{{{  wait for termination of a task or taskforce */

/**
*** This request is probably the trickiest to handle, as it may involve
*** blocking for some indeterminate period of time. Note that it runs
*** in a separate thread from the ConnectionGuardian.
***
*** The TaskDetails vector describes one or more tasks, taskforces, or
*** component tasks. It is necessary to wait for any one of these to
*** terminate, if they have not already done so, and return update information.
*** There are four passes.
***  a) in pass one the various thingies are checked. If any of them no
***     longer exist or have already terminated there is no point in blocking.
***     This pass is also used to check capabilities.
***  b) in pass two (optional), the routine attaches itself to the various
***     thingies and waits for one of them to terminate. Note that a thingy
***     "cannot" terminate between a and b, because termination involves a
***	write lock. The UseCount is incremented to avoid the thingy going
***     away while this thread is waiting for termination.
***  c) in pass three the update information is filled in. 
***  d) in pass four (optional) the work of pass two is undone.
**/

static void HandleWait(TfmConn connection, int job_id, RmRequest *request, RmReply *reply)
{ int		 count = request->VariableSize / sizeof(TaskDetails);
  TaskDetails	*details = (TaskDetails *) request->VariableData;
  int		 i;
  TaskWaiter	*waiters = NULL;
  int		 rc = RmE_Success;
  TaskUpdate	*updates = NULL;
  Semaphore	 done;
  RmTask	 task;
  RmTaskforce	 taskforce;
  TaskEntry	*task_entry;
  bool		 all_running = TRUE;

  InitSemaphore(&done, 0);

  MRSW_FreeRead();
  MRSW_GetWrite();

  if (count eq 0) goto done;

  waiters = (TaskWaiter *) Malloc((word)count * sizeof(TaskWaiter));
  if (waiters eq NULL) { rc = RmE_ServerMemory; goto done; }
  memset(waiters, 0, count * sizeof(TaskWaiter));

  updates = (TaskUpdate *) Malloc((word)count * sizeof(TaskUpdate));
  if (updates eq NULL) { rc = RmE_ServerMemory; goto done; }
  memset(updates, 0, count * sizeof(TaskUpdate));

  /*{{{  loop 1 */
  for (i = 0; i < count; i++)
   { task = (RmTask) Lookup(&TFM, details[i].Name, TRUE);
  	/* if a thingy is missing, it defaults to terminated. */
     if (task eq (RmTask) NULL)
      { all_running = FALSE; break; }
  
     if (RmIsTask(task))
      { unless(GetAccess(&(details[i].Cap), task->ObjNode.Key) &&
  		(details[i].Cap.Access & AccMask_D))
  	  { rc = RmE_NoAccess; goto done; }
        if (task->StructType eq RmL_Done)
         { all_running = FALSE; break; }
      }
  
     taskforce = (RmTaskforce) task;
     if (details[i].Uid eq RmL_NoUid)
      { unless(GetAccess(&(details[i].Cap), taskforce->DirNode.Key) &&
  		(details[i].Cap.Access & AccMask_D))
  	 { rc = RmE_NoAccess; goto done; }
        if (taskforce->StructType eq RmL_Done)
  	 { all_running = FALSE; break; }
      }
     else
      { task = RmFindTask(taskforce, details[i].Uid);
        if (task eq (RmTask) NULL)
  	 { all_running = FALSE; break; }
  	unless(GetAccess(&(details[i].Cap), task->ObjNode.Key) &&
  		(details[i].Cap.Access & AccMask_D))
  	 { rc = RmE_NoAccess; goto done; }
        if (task->StructType eq RmL_Done)
         { all_running = FALSE; break; }
      }
   }
  /*}}}*/

  if (all_running)
   /*{{{  loop 2 */
   for (i = 0; i < count; i++)
    { task = (RmTask) Lookup(&TFM, details[i].Name, TRUE);
      if (RmIsTask(task))
       task_entry = GetTaskEntry(task);
      else
       { taskforce = (RmTaskforce) task;
   	if (details[i].Uid eq RmL_NoUid)
   	 task_entry = (TaskEntry *) RmGetTaskforcePrivate(taskforce);
         else
   	 { task = RmFindTask(taskforce, details[i].Uid);
   	   task_entry = GetTaskEntry(task);
   	 }
       }
      task_entry->UseCount++;
      AddTail(&(task_entry->Waiting), &(waiters[i].Node));
      waiters[i].Sem = &done;
    }
   /*}}}*/

  if (all_running)
   { MRSW_FreeWrite();
     Wait(&done);
     MRSW_GetWrite();
   }

  /*{{{  loop 3 */
  for (i = 0; i < count; i++)
   { task = (RmTask) Lookup(&TFM, details[i].Name, TRUE);
     if (task eq (RmTask) NULL)
      { updates[i].Errno	= RmE_NotFound;
  	updates[i].StructType	= RmL_Done;
      }
     elif (RmIsTask(task))
      { updates[i].StructType	= task->StructType;
  	if (task->StructType eq RmL_Done)
  	 { updates[i].ReturnCode = task->ReturnCode;
  	   remove_from_connection(task, connection);
  	 }
      }
     else	/* entry must be a taskforce */
      { taskforce = (RmTaskforce) task;
  	if (details[i].Uid eq RmL_NoUid)
  	 { updates[i].StructType	= taskforce->StructType;
  	   if (taskforce->StructType eq RmL_Done)
  	    { updates[i].ReturnCode	= taskforce->ReturnCode;
  	      remove_from_connection(task, connection);
  	    }
  	 }
  	else
  	 { task = RmFindTask(taskforce, details[i].Uid);
  	   if (task eq (RmTask) NULL)
  	    { updates[i].Errno		= RmE_NotFound;
  	      updates[i].StructType	= RmL_Done;
  	    }
  	   else
  	    { updates[i].StructType	= task->StructType;
  	      if (task->StructType eq RmL_Done)
  	       updates[i].ReturnCode = task->ReturnCode;
  	    }
  	 }
      }
   }
  /*}}}*/
  reply->VariableData	= (BYTE *) updates;
  reply->VariableSize	= count * sizeof(TaskUpdate);

  if (all_running)
   /*{{{  loop 4 */
   for (i = 0; i < count; i++)
    { List		*list;
      TaskWaiter	*waiter;
   	/* go to the end of the list */
      for (waiter = &(waiters[i]); !EndOfList_(waiter); waiter = Next_(TaskWaiter, waiter));
        /* now at end of list... */
      list	 = (List *) (((BYTE *) waiter) - offsetof(List, Earth));
      task_entry = (TaskEntry *) (((BYTE *) list) - offsetof(TaskEntry, Waiting));
      task	 = task_entry->Task;
      Remove(&(waiters[i].Node));
      task_entry->UseCount--;
      if ((task_entry->UseCount eq 0) && (task->ObjNode.Parent eq &TFM))
       { if (RmIsTask(task))
          task_Destroy(task);
   	 else
   	  taskforce_Destroy((RmTaskforce) task);
       }
    }
   /*}}}*/

done:
  MRSW_SwitchRead();
  reply->FnRc = rc;
  ReplyRmLib(connection, job_id, reply);
  if (waiters ne NULL) Free(waiters);
  if (updates ne NULL) Free(updates);
}

/*}}}*/
/*{{{  send a signal */
/**
*** The client will have sent details of one or more tasks, taskforces, or
*** component tasks which should receive a signal. These are in the form of
*** a TaskDetails vector. There are two passes. First the TFM checks that
*** the various thingies are valid and that the client has the required
*** access. Next the TFM sends the signal.
**/
static void HandleSendSignal(TfmConn connection, int job_id, RmRequest *request, RmReply *reply)
{ TaskDetails	*details	= (TaskDetails *) request->VariableData;
  int		 count		= request->VariableSize / sizeof(TaskDetails);
  int		 i;
  RmTask	 task;
  RmTaskforce	 taskforce;
  int		 signo		= request->Arg1;
  int		 rc		= RmE_Success;

  MRSW_FreeRead();
  MRSW_GetWrite();

  for (i = 0; i < count; i++)
   { task = (RmTask) Lookup(&TFM, details[i].Name, TRUE);
     if (task eq (RmTask) NULL) continue;
     if (RmIsTask(task))
      { unless(GetAccess(&(details[i].Cap), task->ObjNode.Key) &&
		(details[i].Cap.Access & AccMask_D))
	 { rc = RmE_NoAccess; goto done; }
      }
     else	/* must be a taskforce */
      { taskforce = (RmTaskforce) task;
	if (details[i].Uid eq RmL_NoUid)
	 { unless(GetAccess(&(details[i].Cap), taskforce->DirNode.Key) &&
			(details[i].Cap.Access & AccMask_D))
	    { rc = RmE_NoAccess; goto done; }
         }
	else
	 { task = RmFindTask(taskforce, details[i].Uid);
	   if (task eq (RmTask) NULL) continue;
	   unless(GetAccess(&(details[i].Cap), task->ObjNode.Key) &&
			(details[i].Cap.Access & AccMask_D))
	     { rc = RmE_NoAccess; goto done; }
	 }
      }		/* taskforce */
   }		/* first loop */

  for (i = 0; i < count; i++)
   { task = (RmTask) Lookup(&TFM, details[i].Name, TRUE);
     if (task eq (RmTask) NULL) continue;
     if (RmIsTask(task))
      { if (task->StructType eq RmL_Executing)
	 task_DoSignal(task, signo);
      }
     else	/* must be a taskforce */
      { taskforce = (RmTaskforce) task;
	if (details[i].Uid eq RmL_NoUid)
	 { if (taskforce->StructType eq RmL_Executing)
	    taskforce_DoSignal(taskforce, signo);
         }
	else
	 { task = RmFindTask(taskforce, details[i].Uid);
	   if (task eq (RmTask) NULL) continue;
	   if (task->StructType eq RmL_Executing)
	    task_DoSignal(task, signo);
	 }
      }		/* taskforce */
   }		/* second loop */


done:
  MRSW_SwitchRead();
  reply->FnRc	= rc;
  ReplyRmLib(connection, job_id, reply);
}
/*}}}*/
/*{{{  detach a task or taskforce from this connection */
/**
*** The VariableVector of the request specifies a number of top-level
*** tasks or taskforces which should be detached from this connection.
*** It is theoretically possible that the thingy no longer exists or is
*** not owned by this connection, which is a silent error. There are
*** two passes, one to check for access rights and one to remove the
*** thingy from the connection. RmLib does not expect any extra information
*** back.
**/
static void HandleLeave(TfmConn connection, int job_id, RmRequest *request, RmReply *reply)
{ int		rc	= RmE_Success;
  int		count	= request->VariableSize / sizeof(TaskDetails);
  int		i;
  RmTask	task;
  RmTaskforce	taskforce;
  TaskDetails	*details = (TaskDetails *) request->VariableData;

  MRSW_FreeRead();
  MRSW_GetWrite();

  for (i = 0; i < count; i++)
   { task = (RmTask) Lookup(&TFM, details[i].Name, TRUE);
     if (task eq (RmTask) NULL) continue;
     if (RmIsTask(task))
      { unless(GetAccess(&(details[i].Cap), task->ObjNode.Key) &&
		(details[i].Cap.Access & AccMask_D))
	 { rc = RmE_NoAccess; goto done; }
      }
     else
      { taskforce = (RmTaskforce) task;
	unless(GetAccess(&(details[i].Cap), taskforce->DirNode.Key) &&
		(details[i].Cap.Access & AccMask_D))
	 { rc = RmE_NoAccess; goto done; }
      }
   }

  for (i = 0; i < count; i++)
   { task = (RmTask) Lookup(&TFM, details[i].Name, TRUE);
     if (task ne (RmTask) NULL)
      remove_from_connection(task, connection);
   }

done:
  MRSW_SwitchRead();
  reply->FnRc	= rc;
  ReplyRmLib(connection, job_id, reply);
}
/*}}}*/
