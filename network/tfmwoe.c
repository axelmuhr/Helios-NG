/*------------------------------------------------------------------------
--                                                                      --
--           H E L I O S   N E T W O R K I N G   S O F T W A R E	--
--           ---------------------------------------------------	--
--                                                                      --
--             Copyright (C) 1991, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- tfmwoe.c								--
--                                                                      --
--	This module of the Taskforce Manager deals with various		--
--	exceptions such as processors being lost or reclaimed.		--
--                                                                      --
--	Author:  BLV 12/12/91						--
--                                                                      --
------------------------------------------------------------------------*/
/* RcsId: $Header: /hsrc/network/RCS/tfmwoe.c,v 1.6 1993/08/12 14:25:10 nickc Exp $*/

#include <stdio.h>
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
#include <sys/socket.h>
#include <sys/un.h>
#include "exports.h"
#include "private.h"
#include "netutils.h"
#include "rmlib.h"
#include "tfmaux.h"

void	InitWoe(void)
{
}

/**----------------------------------------------------------------------------
*** Exception handling. The Network Server can send various types of exception
*** messages when processors crash or are reclaimed. 
**/
void Tfm_ExceptionHandler(int fnrc, int size, byte *data)
{ word		uid = *((word *) data);
  RmProcessor	processor;

  if (size ne sizeof(uid))
   { report("ignoring unexpected exception message from Network server");
     return;
   }

  MRSW_GetRead();

  processor = RmFindProcessor(Domain, (int) uid);
  if (processor eq (RmProcessor) NULL) goto done;

  switch(fnrc)
   { case RmR_Private + RmR_Crashed	:
     case RmR_Private + RmR_Reclaimed	:
		report("processor %P has %s", processor,
			(fnrc eq RmR_Private + RmR_Crashed) ? "crashed" :
			"been reclaimed");
		until(Fork(LostProcessor_Stack, &LostProcessor, sizeof(uid), uid))
		 Delay(OneSec);
		break;

     case RmR_Private + RmR_Suspicious	:
		until(Fork(LostProcessor_Stack, &SuspiciousProcessor, sizeof(uid), uid))
		 Delay(OneSec);
		break;
   }
done:
  MRSW_FreeRead();
}

/**-----------------------------------------------------------------------------
*** A processor is marked whenever some other part of the TFM or an
*** application running under the TFM believes it has detected a fault.
*** The TFM performs a basic check of the processor, similar to the
*** checks done within the Network Server. If this check fails then the
*** problem is reported to the Network Server. The Network Server may
*** choose to reboot the processor, inform the TFM that the processor
*** really has crashed, etc. If the TFM's check is passed then the problem
*** is not reported to the Network Server, but the processor is still reported
*** as suspicious which means that affected applications will receive
*** signals. This routine should be called with at least a read lock.
**/
void MarkProcessor(RmProcessor processor)
{ Object	*proc;
  int		purpose;
  bool		OK = FALSE;
  int		uid;

  purpose = RmGetProcessorPurpose(processor) & RmP_Mask;
  uid	  = processor->Uid;

  if (purpose eq RmP_Native) goto done;
  if (purpose eq RmP_Router) goto done;

	/* For I/O and Helios processors, attempt to locate the		*/
	/* processor.							*/
  proc = TfmMapProcessorToObject(processor);
  if (proc eq Null(Object)) goto done ;

	/* For I/O processors, the only sensible test is to check for	*/
	/* the error logger.						*/
  if (purpose eq RmP_IO)
   { Object	*logger = Locate(proc, "logger");
     if (logger ne Null(Object))
      { OK = TRUE; Close(logger); }
   }
  else
	/* For Helios nodes, check the following:			*/
	/* 1) existence of procman /tasks				*/
	/* 2) existence of /loader					*/
	/* 3) existence of /tasks/ProcMan.0				*/
	/* 4) existence of /loader/Kernel				*/
   { Object *procman		= Null(Object);
     Object *loader		= Null(Object);
     Object *procman_entry	= Null(Object);
     Object *kernel_entry	= Null(Object);

     if ((procman	= Locate(proc, "tasks"))  eq Null(Object)) goto skip;
     if ((loader	= Locate(proc, "loader")) eq Null(Object)) goto skip;
     if ((procman_entry	= Locate(procman, "ProcMan.0")) eq Null(Object)) goto skip;
     if ((kernel_entry	= Locate(loader, "Kernel")) eq Null(Object)) goto skip;

     OK = TRUE;
skip:
     if (procman	ne Null(Object)) Close(procman);
     if (loader		ne Null(Object)) Close(loader);
     if (procman_entry	ne Null(Object)) Close(procman_entry);
     if (kernel_entry 	ne Null(Object)) Close(kernel_entry);
   }

  Close(proc);

done:
  if (!OK)
   RmReportProcessor(processor);
  else
   (void) Fork(LostProcessor_Stack, &SuspiciousProcessor, sizeof(uid), uid);
}

	/* This routine can be invoked from applications to report	*/
	/* a suspected problem.						*/
void HandleReportProcessor(TfmConn connection, int jobid,
				RmRequest *request, RmReply *reply)
{ RmProcessor processor = RmFindProcessor(Domain, request->Uid);
  if (processor ne RmM_NoProcessor)
   MarkProcessor(processor);
  reply->FnRc	= RmE_Success;
  ReplyRmLib(connection, jobid, reply);
}

/**-----------------------------------------------------------------------------
*** LostProcessor(). This routine is invoked by the Network Server using the
*** Resource Management library exception mechanism to indicate that a certain
*** processor is no longer owned by the TFM. All programs that used to run
*** on that processor can be assumed to have crashed. All affected taskforces
*** should be informed via a SIGSIB signal.
**/
static int LostProcessor_aux1(RmTask, ...);

void LostProcessor(int uid)
{ RmProcessor	processor;
  RmTaskforce	taskforces[MaxComponentsPerProcessor];
  int		tabhead = 0;

  MRSW_GetRead();

  processor = RmFindProcessor(Domain, uid);
  if (processor eq RmM_NoProcessor) goto done;

	/* stop the mapping code from using this processor		*/
  processor->ObjNode.Size = (processor->ObjNode.Size & ~RmS_Running) | RmS_Crashed;


  if (RmApplyMappedTasks(processor, &LostProcessor_aux1, taskforces, &tabhead) > 0)
   { int	i;
     Delay(5 * OneSec);
     for (i = 0; i < tabhead; i++)
      taskforce_DoSignal(taskforces[i], /* SIGKILL */ SIGSIB);
   }

	/* Remove the processor from the domain. There is no need to	*/
	/* release it, since the Network Server has already reclaimed	*/
	/* it. Removing it requires a write lock.			*/
  MRSW_FreeRead();
  MRSW_GetWrite();

  processor = RmFindProcessor(Domain, uid);
  if (processor ne RmM_NoProcessor)
   RmFreeProcessor(RmRemoveProcessor(processor));

  MRSW_SwitchRead();

done:
  MRSW_FreeRead();
}

static int LostProcessor_aux1(RmTask task, ...)
{ va_list	args;
  RmTaskforce	*tab;
  int		*tab_head;
  TaskEntry	*task_entry;

  va_start(args, task);
  tab      = va_arg(args, RmTaskforce *);
  tab_head = va_arg(args, int *);
  va_end(args);

  RmSetTaskState(task, RmS_Dead);
  task->StructType = RmL_Terminated;

  task_entry = GetTaskEntry(task);
  if (task_entry->ProgramStream ne Null(Stream))
   { task_entry->ProgramStream->Flags &= ~Flags_Closeable;
     AbortPort(task_entry->ProgramStream->Reply,
		EC_Fatal + SS_TFM + EG_Broken + EO_Processor);
   }

	/* Send SIGSIB only to taskforces and only if a signal has not	*/
	/* been generated already from inside task_Monitor().		*/
  if ((task->ObjNode.Parent ne &TFM) && 
      (task->ReturnCode ne (EC_Fatal + SS_TFM + EG_Broken + EO_Program)))
   { RmTaskforce root_taskforce = RmRootTaskforce(task);
     int	 i;
     for (i = 0; i < *tab_head; i++)
      if (root_taskforce eq tab[i])
       return(0);
     tab[*tab_head] = root_taskforce;
     *tab_head += 1;
     return(1);
   }

  return(0);
}

  
/**-----------------------------------------------------------------------------
*** SuspiciousProcessor(). This routine is called when the TFM has reason to
*** believe that something may have gone wrong with a particular processor.
*** All affected taskforces should be sent a SIGSUSP signal.
**/
void SuspiciousProcessor(int uid)
{
}

