/*------------------------------------------------------------------------
--                                                                      --
--           H E L I O S   N E T W O R K I N G   S O F T W A R E	--
--           ---------------------------------------------------	--
--                                                                      --
--             Copyright (C) 1990, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- netboot.c								--
--                                                                      --
--	The bootstrap module of the Network Server			--
--                                                                      --
--	Author:  BLV 18/8/90						--
--                                                                      --
------------------------------------------------------------------------*/
/*$Header: /dsl/HeliosRoot/Helios/network/RCS/netboot.c,v 1.35 1993/09/28 14:59:39 richardp Exp $*/

/*{{{  headers and compile-time options */
#define	__Netboot_Module
#define __NetworkServer

#include <stdio.h>
#include <syslib.h>
#include <servlib.h>
#include <link.h>
#include <root.h>
#include <config.h>
#include <sem.h>
#include <codes.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <posix.h>
#include <ctype.h>
#include <nonansi.h>
#include <stddef.h>
#include <c40.h>
#include "exports.h"
#include "private.h"
#include "netutils.h"
#include "rmlib.h"
#include "netaux.h"
/*}}}*/
/*{{{  forward declarations and statics */
/**
*** Bootstrap happens in the form of ``jobs''. The initial bootstrap is
*** one job. When processors are switched from native to Helios mode this
*** is another job. There are routines to create a new job, to add a processor
*** to a job, and to start a job. 
**/

#define 	SafetyMagic		0x73de190a
#define		MaxRetries		5

static	int	ProtectionMatrix;

static	void	BootstrapProcess(BootstrapJob *);
static	void	do_protection(RmProcessor);
static	void	ValidateProcessorDetails(RmProcessor);
static	int	execute_mapped_task(RmTask, ...);
static	void	set_pending(RmProcessor, BootstrapJob *);
static	void	set_enabled(RmProcessor, BootstrapJob *);

/**
*** Perform any one-off initialisation. In particular, work out the access
*** matrices to install on every processor.
**/
void InitBootstrap(void)
{ 
  if (get_config("processor_protection", nsrc_environ))
   ProtectionMatrix = 0x21212147;	/* rwdv, not a, for owner only */
   				/* currently read-only for the rest */ 
  else
   ProtectionMatrix = DefNameMatrix | AccMask_A;
}
/*}}}*/

/*{{{  start the initial network bootstrap */
/**
*** This routine is called from the main module netserv.c to initiate
*** the whole network bootstrap process.
***
*** First, the root processor, the I/O processor, etc. should be
*** updated. This involves modifying the name. There is a special case
*** situation for I/O Processors, where I cannot run the network agent
*** on the target processor.
**/
static int	StartBootstrapAux1(RmProcessor , ...);

void StartBootstrap(void)
{ BootstrapJob	*job	= NewBootstrapJob();

  Debug(dbg_Boot, ("initialising bootstrap"));
  
  if (job eq Null(BootstrapJob))
   fatal("not enough memory to initialise bootstrap");

  Debug(dbg_Boot, ("root processor is %P", RootProcessor));
  
  RmSetProcessorState(RootProcessor, RmS_Running);
  
  if (BootIOProcessor ne RmM_NoProcessor)
   { RmSetProcessorState(BootIOProcessor, RmS_Running);
     Debug(dbg_Boot, ("boot I/O processor is %P", BootIOProcessor));
   }
   
  if (BootIOProcessor ne RmM_NoProcessor)
   UpdateIOProcessor(BootIOProcessor, FALSE);
  UpdateProcessor(RootProcessor, FALSE);

	/* These hacks are necessary to cope with the fact that Object	*/
	/* names may have changed, and the next time the object is	*/
	/* used the name may get updated and overflow the allocated	*/
	/* buffer.							*/
  if (ThisProcessor ne Null(Object))
   { Object	*temp = ThisProcessor;
     ThisProcessor = Locate(Null(Object), ThisProcessor->Name);
     Close(temp);
   }
  if (NetAgent ne Null(Object))
   { Object	*temp = NetAgent;
     NetAgent = Locate(Null(Object), NetAgent->Name);
     NetAgent->Access = temp->Access;
     Close(temp);
   } 
    
  (void) RmApplyProcessors(Net, &StartBootstrapAux1, job);
  
  FullReset = FALSE;
  unless(StartBootstrapJob(job))
   fatal("not enough memory to start bootstrap job");

  if (get_config("waitfor_network", nsrc_environ) ne Null(char))
   WaitBootstrapJob(job);
}

static int StartBootstrapAux1(RmProcessor processor, ...)
{ va_list	args;
  BootstrapJob	*job;
  int		purpose;
    
  va_start(args, processor);
  job = va_arg(args, BootstrapJob *);
  va_end(args);

  if (processor eq RootProcessor)	return(0);
  if (processor eq BootIOProcessor)	return(0);

  purpose = RmGetProcessorPurpose(processor);

	/* BLV - do something, probably */
  if ((purpose & RmP_Mask) eq RmP_Router)
   return(0);

  if (!FullReset)
   { int	state = RmGetProcessorState(processor);
     RmSetProcessorState(processor, RmS_Special);
     if (CheckProcessor(processor))
      { if ((purpose & RmP_Mask) eq RmP_IO)
         UpdateIOProcessor(processor, FALSE);
        elif ((purpose & RmP_Mask) eq RmP_Helios)
         UpdateProcessor(processor, FALSE);
        RmSetProcessorState(processor, state);
        if ( CheckProcessor(processor))
         { RmSetProcessorState(processor, RmS_Running);
           return(0);
         }
      }
     else
      RmSetProcessorState(processor, state);
   }

  if ((purpose & RmP_Mask) eq RmP_IO)
   { RmSetProcessorState(processor, RmS_Crashed);
     return(0);
   }

  RmSetProcessorState(processor, RmS_Crashed);
  unless(AddProcessorToBootstrapJob(job, processor))
   fatal("not enough memory to initiate bootstrap");

  return(0);
}
/*}}}*/
/*{{{  bootstrap jobs - the main external interface */

/*{{{  NewBootstrapJob() */
BootstrapJob *NewBootstrapJob(void)
{ BootstrapJob	*new_job = New(BootstrapJob);
  static	int bootstrap_count = 0;
  
  if (new_job eq Null(BootstrapJob)) return(Null(BootstrapJob));
  new_job->Sequence		= bootstrap_count++;
  new_job->NumberProcessors	= 0;
  new_job->MaxProcessors	= 0;
  new_job->Next			= 0;
  new_job->Table		= Null(RmProcessor);
  new_job->Progress		= Null(RmProcessor);
  new_job->SafetyCheck		= SafetyMagic;
  InitSemaphore(&(new_job->Lock), 1);
  InitSemaphore(&(new_job->Finished), 0);
  InitSemaphore(&(new_job->ProcessesFinished), 0);
  InitSemaphore(&(new_job->LinksPending), 0);
  InitSemaphore(&(new_job->LinksEnabled), 0);
  InitSemaphore(&(new_job->ClearNames), 0);
  new_job->JobStarted		= FALSE;
  return(new_job);
}
/*}}}*/
/*{{{  AddProcessorToBootstrapJob() */

bool	AddProcessorToBootstrapJob(BootstrapJob *job, RmProcessor processor)
{ bool	result = FALSE;

  Wait(&(job->Lock));

  if (job->JobStarted) goto done;
  if (job->SafetyCheck ne SafetyMagic) goto done;
  
  if (job->NumberProcessors >= job->MaxProcessors)
   { int		amount = job->MaxProcessors;
     RmProcessor	*table = Null(RmProcessor);

     if (amount eq 0) amount = 4;
     amount *= 2;
     table = (RmProcessor *) Malloc(sizeof(RmProcessor) * (word)amount);
     if (table eq Null(RmProcessor)) goto  done;

     if (job->NumberProcessors > 0)
      { memcpy((void *) table, (void *) job->Table, 
      		job->NumberProcessors * sizeof(RmProcessor));
      	Free(job->Table);
      }

     job->Table = table;
     job->MaxProcessors	= amount;
   } 

  job->Table[job->NumberProcessors++] = processor;
  result = TRUE;

done:
  Signal(&(job->Lock));
  return(result);
}

/*}}}*/
/*{{{  StartBootstrapJob() */
bool	StartBootstrapJob(BootstrapJob *job)
{ RmProcessor	*progress_table;
  bool		result = FALSE;

  Wait(&(job->Lock));
  if (job->SafetyCheck ne SafetyMagic) goto done;
  if (job->JobStarted) goto done;
  job->JobStarted = TRUE;
  Signal(&(job->Lock));
  
  if (job->NumberProcessors eq 0)
   { result = TRUE; 
     Signal(&(job->Finished));
     Delay(2 * OneSec);
     Free(job); 
     goto done; 
   }

  progress_table = (RmProcessor *) 
  		Malloc((word) job->MaxProcessors * sizeof(RmProcessor));
  if (progress_table eq Null(RmProcessor)) goto done;
  job->Progress		= progress_table;

  if(Fork(Bootstrap_Stack, &BootstrapProcess, sizeof(BootstrapJob *), job))
   result = TRUE;
   
done:
  return(result);
}
/*}}}*/
/*{{{  WaitBootstrapJob() */
/**
*** Wait for the whole network bootstrap to finish.
*** Optionally used by startns to delay starting up the Session Manager
*** until the whole network is ready. Also used for native networks and
*** rebooting after faults. This routine is always called with a write lock.
**/
void	WaitBootstrapJob(BootstrapJob *job)
{ MRSW_SwitchRead();
  Wait(&(job->Finished));
  MRSW_FreeRead();
  MRSW_GetWrite();
}
/*}}}*/
/*{{{  AbortBootstrapJob() */
/**
*** Abandon a bootstrap job. This is used typically when filling in a
*** bootstrap job  causes memory failure.
**/
void	AbortBootstrapJob(BootstrapJob *job)
{ if (job->JobStarted) return;
  if (job->SafetyCheck ne SafetyMagic) return;
  job->SafetyCheck = 0;
  if (job->Table ne Null(RmProcessor)) Free(job->Table);
  if (job->Progress ne Null(RmProcessor)) Free(job->Progress);
  Free(job);
}
/*}}}*/

/*}}}*/
/*{{{  update processor */
/**
*** Updating processors. It is possible for a processor to be running
*** but not yet fully initialised. In particular, the root processor is
*** likely to come up as /00 instead of /Net/00. This causes a problem,
*** because the usual routines NetMapProcessorToObject() etc cannot be
*** used.
***
*** 1) Update processor. This involves running a network agent on the
***    target processor, building up the full network name, and sending
***    an update request to the network agent. After this the appropriate
***    protection mode is checked, and the processor's type and memory
***    sizes are checked. Also, links are handled.
***
*** 2) UpdateIOProcessor(). This involves running a network agent on
***    a neighbouring processor and asking it to rename a link.
**/

void UpdateProcessor(RmProcessor processor, bool name_only)
{ int			state		= RmGetProcessorState(processor);
  char			*procname	= Null(char);
  NA_Message		message;
  int			length;
  int			rc;
  int			number_links, i;
  RmProcessor		neighbour;
  bool			started_agent = FALSE;

  Debug(dbg_Boot, ("updating processor %P", processor));

  processor->ObjNode.Dates.Modified	=
  processor->ObjNode.Dates.Access	= GetDate();
          
  RmSetProcessorState(processor, RmS_Special);/* see NetMapProcessorToObject()*/

  unless(StartNetworkAgent(processor)) goto done;
  started_agent = TRUE;

  procname = (char *) Malloc(IOCDataMax);
  if (procname eq Null(char))
   { report("not enough memory to update processor %P", processor);
     goto done;
   }
  (void) BuildName(procname, processor);
  length		= strlen(procname) + 1;
  message.FnRc		= NA_Cupdate;
  message.Size		= length;
  message.Data		= procname;

  Debug(dbg_Boot, ("processor update, sending request to netagent"));

  rc = XchNetworkAgent(processor, &message, TRUE, 0, NULL);  
  if (rc ne Err_Null)
   report("warning, failed to update processor name for %P, fault %x",
   		processor, rc);

  if (name_only) goto done;
  
  do_protection(processor);

  ValidateProcessorDetails(processor);

	/* set any external links to pending. This includes links to I/O */
	/* processors other than the booting one.			 */
  number_links = RmCountLinks(processor);
  for (i = 0; i < number_links; i++)
   { int destlink;
     int current_mode;
      
     message.FnRc = NA_GetLinkMode;
     message.Arg1 = i;
     message.Size = 0;
     rc = XchNetworkAgent(processor, &message, TRUE, sizeof(int), (BYTE *) &current_mode);
     if (rc ne RmE_Success) goto done;
       
     neighbour = RmFollowLink(processor, i, &destlink);
     if (neighbour eq RmM_NoProcessor)
      { if (current_mode ne RmL_NotConnected)
         report("warning, processor %P, link %d, has a connection not shown in the resource map",
          		processor, i);
        continue;
      }

     if (neighbour ne RmM_ExternalProcessor)
      if ((RmGetProcessorPurpose(neighbour) & RmP_Mask) ne RmP_IO)
       continue;
       
     message.FnRc = NA_SetLinkMode;
     message.Arg1 = i;
     if (current_mode eq RmL_Running)
      message.Arg2 = RmL_Running;
     else
      message.Arg2 = RmL_Pending;
     message.Size  = 0;
     (void) XchNetworkAgent(processor, &message, TRUE, 0, NULL);     
   }

  (void) RmApplyMappedTasks(processor, &execute_mapped_task, processor);
done:

  Debug(dbg_Boot, ("update processor done"));
  
  if (started_agent) StopNetworkAgent(processor);
  RmSetProcessorState(processor, state);	/* restore */
  if (procname ne Null(char)) Free(procname);
}
/*}}}*/
/*{{{  update I/O processor */

void UpdateIOProcessor(RmProcessor processor, bool name_only)
{ RmProcessor		neighbour = 0;
  int			destlink;
  NA_Message		message;
  bool			netagent_started = FALSE;
  char			*procname = Null(char);
  int			state;
  int			purpose;
  int			length;
  word			rc;

  Debug(dbg_Boot, ("updating I/O processor %P", processor));

  processor->ObjNode.Dates.Modified =
  processor->ObjNode.Dates.Access   = GetDate();
  
  if (RmCountLinks(processor) ne 1)
   { report("error, I/O processor %P has more than one link",
   		processor);
     goto done;
   }
   
  neighbour = RmFollowLink(processor, 0, &destlink);
  if ((neighbour eq RmM_NoProcessor) || (neighbour eq RmM_ExternalProcessor))
   { report("error, I/O processor %P is not connected to the main network",
   		processor);
     goto done;
   }

  Debug(dbg_Boot, ("I/O processor's neighbour is %P", neighbour));

  state = RmGetProcessorState(neighbour);
  unless(state & RmS_Running) goto done;
  purpose = RmGetProcessorPurpose(neighbour);
  unless ((purpose & RmP_Mask) eq RmP_Helios) goto done;
       
  procname   = (char *) Malloc(IOCDataMax);  
  if (procname eq Null(char)) goto done;
  (void) BuildName(procname, processor);
  length	= strlen(procname) + 1;

  RmSetProcessorState(neighbour, RmS_Special);
  rc = StartNetworkAgent(neighbour);
  RmSetProcessorState(neighbour, state);
  unless (rc) goto done;
  
  netagent_started = TRUE;
    
  message.FnRc	= NA_UpdateIO;
  message.Arg1	= destlink;
  message.Size	= length;
  message.Data	= procname;
  Debug(dbg_Boot, ("sending request to netagent on neighbour"));
  rc = XchNetworkAgent(neighbour, &message, TRUE, 0, NULL);    
   
  if (rc ne Err_Null)
   report("warning, failed to update I/O processor %P, fault %x",
   	processor, rc);

  if (name_only) goto done;
  name_only = name_only;
    
done:
  Debug(dbg_Boot, ("update I/O Processor done"));
  
  if (netagent_started) StopNetworkAgent(neighbour);
  if (procname ne Null(char)) Free(procname); 
}

/*}}}*/
  
/*{{{  BootstrapProcess() - one per job */
/**
*** The real bootstrap code
***
*** A job consists of the following sequence of operations.
*** 1) attempt to reset every processor in the job. Do some admin work as
***    well, in particular invalidate capabilities and set the state.
***
*** Retry three times or as long as partial success is achieved
*** 2) find a processor to start the bootstrap from. This must be an active
***    Helios processor. Start a network agent on that processor and
***    initiate a parallel bootstrap. As each processor is booted up
***    a thread is spawned off to deal with things like protection.
*** 3) sort out all the connections for all the processors. This involves
***    setting some links to pending, then some links to running. The
***    work is done by waking up the various threads.
*** 4) clear the name tables of all the processors in this job, again by
***    waking up the various threads. At this point the various threads
***    will terminate.
**/

static	RmProcessor	find_starting_place(BootstrapJob *job, int *destlink);
static	int		actual_boot(BootstrapJob *job, RmProcessor, int link);
static	void		continue_boot(BootstrapJob *job, RmProcessor);

static void BootstrapProcess(BootstrapJob *job)
{ RmProcessor		*table = job->Table; 
  int			number = job->NumberProcessors;
  int			left   = number;
  int			i;
  int			count; 
  int			retries;
  RmProcessor		start;
  int			destlink;
  ProcessorEntry	*proc_entry;
  bool			all_done = TRUE;

  MRSW_GetRead();

  Debug(dbg_Boot, ("starting bootstrap job %d for %d processors", job->Sequence, number));
    
  for (i = 0; i < number; i++)
   { RmProcessor processor = table[i];
     int	 state     = RmGetProcessorState(processor);
     state &= ~(RmS_Running | RmS_Suspicious | RmS_Crashed | RmS_Dead);
     state |= RmS_Booting;
     (void) RmSetProcessorState(processor, state);
     proc_entry = GetProcEntry(processor);
     memset((void *) &(proc_entry->Owner), 0, 2 * sizeof(Capability));
   }

  Debug(dbg_Boot, ("attempting initial resets for job %d", job->Sequence));
       
  ResetProcessors(number, table);

	/* Native processors should not be considered further   */
  for (i = 0; i < number; i++)
   { RmProcessor processor = table[i];
     if ((RmGetProcessorPurpose(processor) & RmP_Mask) eq RmP_Native)
      { RmSetProcessorState(processor, RmS_Running);
        left--;	/* There is one less processor to boot up	*/
      }
   }

  for (retries = 0; (retries < MaxRetries) && (left > 0); )
   { 
     start = find_starting_place(job, &destlink);
     if (start eq (RmProcessor) NULL)
      {	report("error, cannot find a suitable processor from which to continue bootstrap");
	break;
      }

     Debug(dbg_Boot, ("bootstrap job %d, attempt %d, starting from %P", \
     	job->Sequence, retries+1, start));
     i = actual_boot(job, start, destlink);
     if (i eq 0)	/* None booted this time around */
      { retries++; continue; }
     left -= i;
   }

  Debug(dbg_Boot, ("bootstrap, enabling all the connections for job %d", job->Sequence));

	/* Step 1, let all the threads set suitable links to pending */
  for (i = 0, count = 0; i < job->NumberProcessors; i++)
   { RmProcessor processor	= job->Table[i];
     if ((RmGetProcessorPurpose(processor) & RmP_Mask) ne RmP_Helios) continue;
     unless(RmGetProcessorState(processor) & RmS_Running) continue;
     Signal(&(job->LinksPending));
     count++;
   }

  for (i = 0; i < count; i++)	/* wait for all the threads */
   Wait(&(job->ProcessesFinished));
  
	/* Step 2, let the same threads enable cross links */
  for (i = 0; i < count; i++)
   Signal(&(job->LinksEnabled));
  for (i = 0; i < count; i++)
   Wait(&(job->ProcessesFinished));

	/* Step 3, clear the name tables */	
  for (i = 0; i < count; i++)
   Signal(&(job->ClearNames));
  for (i = 0; i < count; i++)
   Wait(&(job->ProcessesFinished));

	/* Step 4, there might still be extra threads lying around	*/
	/* which have not been woken up because their processors have	*/
	/* crashed. I think this will only happen if count is currently	*/
	/* zero. Usually this will be a no-op.				*/
  for (i = 0; i < job->NumberProcessors; i++)
   Signal(&(job->LinksPending));

  Debug(dbg_Boot, ("bootstrap job %d done", job->Sequence));

	/* Final report on this job */
  for (i = 0; i < job->NumberProcessors; i++)
   { RmProcessor processor	= job->Table[i];
     int	 state 		= RmGetProcessorState(processor);
     if (state & RmS_Running)
      { state &= ~RmS_Booting;
        RmSetProcessorState(processor, state);
      }
     else
      { if (!(state & RmS_AutoBoot))
         report("failed to boot processor %P", processor);
        if (!(state & (RmS_Suspicious | RmS_Crashed)))
         { state |= RmS_Dead;
           state &= ~(RmS_AutoBoot | RmS_Booting | RmS_Running);
	   RmSetProcessorState(processor, state);
         }
        all_done = FALSE;
      }
   }

  if (job->Sequence eq 0)
   { if (all_done)
      report("network booted.");
     else
      report("network partly booted.");
   }

  MRSW_FreeRead();

  Signal(&(job->Finished));   	/* See WaitBootstrap() above */
  Free(job->Progress);
  Free(job->Table);
  Delay(OneSec);		/* Yukk, WaitBootstrap() may be waiting on */
  				/* a semaphore in this structure.	   */
  Free(job);
}
/*}}}*/
/*{{{  resetting processors */

/**
*** Try to reset every processor in a job. If a particular processor is already
*** reset skip it (to avoid executing tr_reset a hundred times etc.). Otherwise
*** assert a definite reset if there is one, otherwise a conditional reset
*** if there is one.
**/

static void TryResetCommand(RmProcessor processor, RmHardwareFacility *);

int ResetProcessors(int count, RmProcessor *table)
{ int			i, j, k, l, state;
  int			successes = 0;
  DriverRequest		*request;
  ProcessorEntry	*proc_entry;
  DriverEntry		*driver_entry;
  RmHardwareFacility	*hardware;
  NetworkDCB		*device;

  request = Malloc(sizeof(DriverRequest) + ((word)count * sizeof(RmProcessor)));
  if (request eq Null(DriverRequest))
   return(RmE_ServerMemory);

	/* Set all the processors to ShouldBeReset */
  for (i = 0; i < count; i++)
   table[i]->ObjNode.Size |= RmS_ShouldBeReset;
   
	/* Go through the list of processors three times.	*/
	/* First look for device drivers with a definite reset.	*/
	/* Then look for device drivers with a possible reset.	*/
	/* Then look for reset commands.			*/          
  for (i = 0; i < count; i++)	/* for every processor */
   { RmProcessor	processor	= table[i];

     state = RmGetProcessorState(processor);
     if ((state & RmS_Reset) || ((state & RmS_ShouldBeReset) eq 0))
      continue;

     proc_entry = GetProcEntry(processor);

     for (j = 0; j < proc_entry->NumberDrivers; j++)
      { driver_entry	= &(proc_entry->DriverEntry[j]);
        hardware	= driver_entry->Hardware;
	device		= hardware->Device;
	
        if ((hardware->Type ne RmH_ResetDriver) &&
            (hardware->Type ne RmH_ConfigureDriver))
         continue;	/* next driver */

        unless (driver_entry->Flags & DriverFlags_DefiniteReset)
         continue;	/* next driver */
         
        request->FnRc			= ND_Reset;
        request->NumberProcessors	= 1;
        request->Processor[0]		= processor;

		/* look for other processors controlled by the same driver */
        for (k = i + 1; k < count; k++)
	 { RmProcessor		temp = table[k];
	   ProcessorEntry	*temp_entry;

	   if (RmGetProcessorState(temp) & RmS_Reset) continue;

	   temp_entry = GetProcEntry(temp);
	   for (l = 0; l < temp_entry->NumberDrivers; l++)
	    { DriverEntry *driver = &(temp_entry->DriverEntry[l]);
	      if (driver->Hardware ne hardware) continue;
	      unless(driver->Flags & DriverFlags_DefiniteReset) continue;
	      request->Processor[request->NumberProcessors] = temp;
	      request->NumberProcessors++;
	      break;
	    }
	 }	/* for subsequent processors in the same job.	*/
	 
	(*(device->DCB.Operate))(device, request);
        if (RmGetProcessorState(processor) & RmS_Reset) break;
      }	/* for every driver in the current processor */
   } /* for every processor in the job */

	/* Look for possible resets only.	*/
  for (i = 0; i < count; i++)	/* for every processor */
   { RmProcessor	processor	= table[i];

     state = RmGetProcessorState(processor);
     if ((state & RmS_Reset) || ((state & RmS_ShouldBeReset) eq 0))
      continue;

     proc_entry = GetProcEntry(processor);

     for (j = 0; j < proc_entry->NumberDrivers; j++)
      { driver_entry	= &(proc_entry->DriverEntry[j]);
        hardware	= driver_entry->Hardware;
	device		= hardware->Device;
	
        if ((hardware->Type ne RmH_ResetDriver) &&
            (hardware->Type ne RmH_ConfigureDriver))
         continue;	/* next driver */
         
        unless (driver_entry->Flags & DriverFlags_PossibleReset)
         continue;	/* next driver */
         
        request->FnRc			= ND_Reset;
        request->NumberProcessors	= 1;
        request->Processor[0]		= processor;

		/* look for other processors controlled by the same driver */
        for (k = i + 1; k < count; k++)
	 { RmProcessor		temp = table[k];
	   ProcessorEntry	*temp_entry;

	   if (RmGetProcessorState(temp) & RmS_Reset) continue;

	   temp_entry = GetProcEntry(temp);
	   for (l = 0; l < temp_entry->NumberDrivers; l++)
	    { DriverEntry *driver = &(temp_entry->DriverEntry[l]);
	      if (driver->Hardware ne hardware) continue;
	      unless(driver->Flags & DriverFlags_PossibleReset) continue;
	      request->Processor[request->NumberProcessors] = temp;
	      request->NumberProcessors++;
	      break;
	    }
	 }	/* for subsequent processors in the same job.	*/
	 
	(*(device->DCB.Operate))(device, request);
        if (RmGetProcessorState(processor) & RmS_Reset) break;
      }	/* for every driver in the current processor */
   } /* for every processor in the job */

	/* Finally look for reset commands */
  for (i = 0; i < count; i++)	/* for every processor */
   { RmProcessor	processor	= table[i];

     state = RmGetProcessorState(processor);
     if ((state & RmS_Reset) || ((state & RmS_ShouldBeReset) eq 0))
      continue;

     proc_entry = GetProcEntry(processor);

     for (j = 0; j < proc_entry->NumberDrivers; j++)
      { driver_entry	= &(proc_entry->DriverEntry[j]);
        hardware	= driver_entry->Hardware;
	device		= hardware->Device;
	
        if (hardware->Type ne RmH_ResetCommand)
         continue;	/* next driver */

        TryResetCommand(processor, hardware);         

      }	/* for every driver in the current processor */
   } /* for every processor in the job */
   
  for (i = 0; i < count; i++)
   { RmProcessor processor = table[i];
     state = RmGetProcessorState(processor);
     if (state & RmS_Reset) successes++;
     state &= ~RmS_ShouldBeReset;
     RmSetProcessorState(processor, state);
   }

  Free(request);  
  if (successes eq count)
   return(RmE_Success);
  else
   return(RmE_PartialSuccess);
}

/**
*** Reset commands. Various conditions have to be met.
*** 1) if the processor is believed to be reset already, do not use this
***    command. The cost is excessive.
*** 2) if the controller processor is not running, the command cannot be
***    used.
*** 3) if any of the controlled processors should not be reset, do not
***    use this command.
*** Otherwise use the command and update all the processors.
**/
static	void TryResetCommand(RmProcessor processor, RmHardwareFacility *hardware)
{ RmProcessor	controller;
  int		i;
  int		state = RmGetProcessorState(processor);
  RmTask	task;
  Environ	env;
  Environ	*net_env;
    
  if ((state & RmS_Reset) ne 0) return;	/* do not reset again */

  controller = (RmProcessor) hardware->Essential;
  state = RmGetProcessorState(controller);
  unless ((state & RmS_Running) ne 0) return;
  
  for (i = 0; i < hardware->NumberProcessors; i++)
   { if (hardware->Processors[i] eq processor) continue;
     state = RmGetProcessorState(hardware->Processors[i]);
     unless ((state & RmS_ShouldBeReset) ne 0)
      return;
   }    
	/* OK, it looks like the command can be used.	*/
  task = (RmTask) hardware->Device;

  if (task->ObjNode.Name[0] eq '0')
   (void) rexec_task(controller, task, Null(Environ), 5 * OneSec);
  else
   { net_env	= getenviron();
     env.Strv	= net_env->Strv;
     env.Objv	= net_env->Objv;
     env.Envv	= net_env->Envv;
     (void) rexec_task(controller, task, &env, 5 * OneSec);
   }
 
  for (i = 0; i < hardware->NumberProcessors; i++)
   { state = RmGetProcessorState(hardware->Processors[i]);
     state |= RmS_Reset;
     (void) RmSetProcessorState(hardware->Processors[i], state);
   }
}   

/*}}}*/
/*{{{  find a starting place for the bootstrap */

/**
*** Find a processor to start the bootstrap from. This must satisfy the
*** following requirements.
*** 1) it must be adjacent to one of the processors in the job not already
***    booted.
*** 2) it must be running, or booting but not ShouldReset (implies booted in
***    this job)
*** 3) if successful, the boot job is rearranged somewhat to avoid finding
***    the same starting processor everytime.
*** There are special cases. If a bootstrap fails then that link is set to
*** Suspicious. The link will not be used again for a bootstrap unless
*** there is no choice. If a link is set to delayed mode then it has not
*** yet been made by the configuration/routing hardware and this routine
*** should wait a little while before continuing.
**/
static RmProcessor find_starting_place(BootstrapJob *job, int *destlink)
{ RmProcessor	processor;
  int		i, j;
  int		state;
  int		number_links;
  int		purpose;
  RmProcessor	neighbour;
  int		flags;
  RmProcessor	suspicious = (RmProcessor) NULL;
  int		suspect_link = 0;
  RmProcessor	delayed	= (RmProcessor) NULL;
  int		retries = 0;

retry:  
  for (i = 0; i < job->NumberProcessors; i++)
   { processor	= job->Table[i];
     state	= RmGetProcessorState(processor);

     unless ((state & RmS_Booting) && !(state & RmS_Running))
      continue;
      
     number_links = RmCountLinks(processor);
     for (j = 0; j < number_links; j++)
      { neighbour = RmFollowLink(processor, j, destlink);
        if ((neighbour eq RmM_NoProcessor) ||
           (neighbour eq RmM_ExternalProcessor))
         continue;

        purpose = RmGetProcessorPurpose(neighbour);
        unless((purpose & RmP_Mask) eq RmP_Helios) continue;

        state = RmGetProcessorState(neighbour);
        unless(state  & RmS_Running)
         continue;
         
	flags = RmGetLinkFlags(neighbour, *destlink);
	if (flags & RmF_Delayed)
         delayed = neighbour;
	elif (flags & RmF_Suspicious) 
	 { suspicious   = neighbour;
	   suspect_link = *destlink;
	 }
	else
         goto found;        
      }
   }
   
  if (delayed ne (RmProcessor) NULL)
   { if (++retries >= 5 * 60)
      { report("the link to processor %P is not appearing", delayed);
        goto done;
      }
     if ((retries % 10) eq 0)
      report("waiting for link to processor %P", delayed);
     Delay(OneSec);
     delayed = suspicious = (RmProcessor) NULL;
     goto retry;
   }
   
done:      
  if (suspicious ne (RmProcessor) NULL)
   { *destlink = suspect_link;
     return(suspicious);
   }
  else
   return((RmProcessor) NULL);

	/* Processor is a processor in this job which should be booted.  */
	/* neighbour is a suitable processor from which to start the	 */
	/* bootstrap. Swap Processor and the last processor in the table */
found:
  { RmProcessor	temp = job->Table[job->NumberProcessors - 1];       
    job->Table[job->NumberProcessors - 1] = processor;
    job->Table[i] = temp;
  }

  return(neighbour);
}

/*}}}*/
/*{{{  use the starting place to initiate the parallel bootstrap */

/**
*** actual_boot()
***
*** Attempt to initiate a parallel bootstrap from a given starting processor.
*** This means attempting to boot the first processor. If that is successful
*** a process is forked off to continue the bootstrap. Then the routine
*** waits for all the processes to terminate.
**/
static bool boot_down_link(RmProcessor start, int link);

static int actual_boot(BootstrapJob *job, RmProcessor start, int link)
{ RmProcessor	*progress = job->Progress;
  RmProcessor	next;
  int		destlink;
  int		booted;
  int		i, j;
    
  next = RmFollowLink(start, link, &destlink);

  unless(StartNetworkAgent(start)) return(0);

  booted = (int) boot_down_link(start, link);
  StopNetworkAgent(start);

  if (!booted) return(0);
  progress[job->Next++] = next;

  InitSemaphore(&(job->ProcessesFinished), 0);
  job->NumberProcesses = 1;
  
  if (!Fork(Bootstrap_Stack, &continue_boot, 8, job, next))
   return(1);
   
  for(i = 0; ; i++)
   { Wait(&(job->Lock));
     j = job->NumberProcesses;
     Signal(&(job->Lock));
     if (i eq j) break;
     Wait(&(job->ProcessesFinished));
   }

  return(job->NumberProcesses);
}

/*}}}*/
/*{{{  continue the bootstrap from a newly-booted processor, in parallel */

/**
*** Once a processor has been booted, this process is Fork'ed off
*** to see whether or not it is possible to do some more booting from
*** this processor, and then initialise the processor.
***
*** It is necessary to serialise the checks for each link, because a given
*** processor might be examined as a possible bootstrap target from more
*** than one other process. The locking is fairly brutal, but safe.
***
*** The newly-booted processor will already be running the Network Agent,
*** because that happens as the final test in boot_down_link(). The
*** agent is currently shut down at the end of this thread.
**/

static void continue_boot(BootstrapJob *job, RmProcessor current)
{ int	 		linkcount	= RmCountLinks(current);
  int	 		state;
  ProcessorEntry	*its_proc_entry;  
  int			j, k;
  int			destlink;
  RmProcessor		next;
  int			temp;

  unless(SilentMode)
   report("processor %P booted.", current);

	/* If there are any system programs that are supposed to run on	*/
	/* this processor, run them now.				*/
  (void) RmApplyMappedTasks(current, &execute_mapped_task, current);
        
  for (j = 0; j < linkcount; j++)
   { 
     Wait(&(job->Lock));
     
     next = RmFollowLink(current, j, &destlink);
     if ((next eq RmM_NoProcessor) || (next eq RmM_ExternalProcessor))
         goto next_link;
     state = RmGetProcessorState(next);
     unless((state & RmS_Booting) && !(state & RmS_Running))
      goto next_link;

     its_proc_entry = GetProcEntry(next);
     if (its_proc_entry->BeingBooted)
      goto next_link;

		/* Cope with the special case of subnet names		*/
		/* If there is a processor /Net/Cluster/07 and the	*/
		/* current processor is not inside /Net/Cluster and	*/
		/* some processor inside /Net/Cluster has been booted	*/
		/* already , do not boot down this link because the	*/
		/* system's distributed search will fail to find the	*/
		/* processor.						*/
     if (RmParentNetwork(current) ne RmParentNetwork(next))
      { RmNetwork	parent = RmParentNetwork(next);
	bool		first_in_subnet = TRUE;
        for (k = 0; k < job->Next; k++)
         if ((current ne job->Table[k]) && 
             (parent eq RmParentNetwork(job->Table[k])))
          { first_in_subnet = FALSE; break; }
        unless(first_in_subnet) goto next_link;
      }

		/* Check that the processor really is in this bootstrap	*/
		/* job. It is conceivable that there are two bootstrap	*/
		/* jobs at any one time, with connected processors.	*/
     { bool in_job	= FALSE;
       for (k = 0; k < job->NumberProcessors; k++)
        if (next eq job->Table[k])
         { in_job = TRUE; break; }
       if (!in_job) goto next_link;
     }

		/* prevent other processes from booting this processor	*/
		/* at the same time.					*/
     its_proc_entry->BeingBooted = TRUE;
     
		/* let other processes check whether or not they can	*/
		/* do some booting.					*/
     Signal(&(job->Lock));

     temp = (int) boot_down_link(current, j);

		/* Re-lock, before storing the results.			*/
     Wait(&(job->Lock));		

     its_proc_entry->BeingBooted = FALSE;
     if (temp)
      { job->Progress[job->Next++] = next;
        if (job->Sequence eq 0)
         LastBooted = next;
        if (Fork(Bootstrap_Stack, &continue_boot, 8, job, next))
         job->NumberProcesses++;
      }

next_link:
     Signal(&(job->Lock));	/* unlock for this link			*/
   }

	/* Setting the protection and validating details like processor	*/
	/* type can be done immediately after bootstrap. In fact they	*/
	/* should be done at this point.				*/
  do_protection(current);
  ValidateProcessorDetails(current);

	/* If one of the system programs was console, clean up.		*/
  { ProcessorEntry *proc_entry = GetProcEntry(current);
    if (proc_entry->WindowServer != Null(Object))
     Close(proc_entry->WindowServer);
    if (proc_entry->ConsoleWindow != Null(Object))
     Close(proc_entry->ConsoleWindow);
    proc_entry->WindowServer = proc_entry->ConsoleWindow = Null(Object);
  }

	/* Inform the main bootstrap process that this thread is ready	*/
  Signal(&(job->ProcessesFinished));

	/* At this point the thread has to block while the remaining	*/
	/* processors in the job get booted. Once that stage is		*/
	/* finished it is necessary to fiddle with cross-links etc.	*/
  Wait(&(job->LinksPending));

	/* There is a problem here if the processor being handled by	*/
	/* this thread has failed after all the commands have run but	*/
	/* before the main bootstrap thread is ready to enable cross	*/
	/* links. This thread may have received a wake-up it should not	*/
	/* have. 							*/
  if ((!(RmGetProcessorState(current) & RmS_Running)) ||
      ((RmGetProcessorPurpose(current) & RmP_Mask) ne RmP_Helios))
   { Signal(&(job->LinksPending)); goto done; }

	/* Set certain links to pending.				*/
  set_pending(current, job);

	/* Synchronise with the other threads.				*/
  Signal(&(job->ProcessesFinished));
  Wait(&(job->LinksEnabled));

	/* Set other links to running.					*/
  if (RmGetProcessorState(current) & RmS_Running)
   set_enabled(current, job);

	/* Synchronise again.						*/
  Signal(&(job->ProcessesFinished));
  Wait(&(job->ClearNames));

	/* There are problems in the Processor Manager when		*/
	/* deleting names while system tasks are starting up. This	*/
	/* should bypass the problems.					*/
  if ((RmCountMappedTasks(current) eq 0) &&
      (RmGetProcessorState(current) & RmS_Running))
   { NA_Message	    message;
     message.FnRc = NA_ClearNames;
     message.Size = 0;
     XchNetworkAgent(current, &message, FALSE, 0, NULL);
   }

	/* And finish off */
  Signal(&(job->ProcessesFinished));  
done:
  StopNetworkAgent(current);
}

/*}}}*/
/*{{{  boot down link */

/**
*** Boot the neighbour of the given processor down the link specified.
*** If any of the device drivers indicate a special bootstrap, the
*** bootstrap routine is executed. If that fails the network server will
*** default to the standard bootstrap mechanism.
***
*** First a configuration vector is built up. This may include a field
*** for the nucleus if the target and current processors are of incompatible
*** type, or if a special nucleus has been specified for either of them.
*** Next, the processor is reset again. Even if the processor was reset
*** earlier on in the bootstrap sequence there is no guarantee that
*** it is still reset, and better safe than sorry. If there are several
*** different ways to reset a processor only the first successful
*** one is used.
***
**/
/*{{{  build a transputer configuration vector */

/**
*** Build a configuration vector suitable for booting processor dest
*** through its link destlink, from processor source through its
*** sourcelink. This is not easy.
***
*** The first step is to figure out which nucleus to use. This depends
*** on whether or not special nuclei have been specified in the resource
*** map, on the processor types, etc. Default nuclei for different
*** processors are handled by the following routine.
**/

static char	*default_nucleus(RmProcessor processor)
{ int	type = RmGetProcessorType(processor);
  switch(type)
   { case	RmT_Default	:
     case	RmT_T800	:
     case	RmT_T414	:
     case	RmT_T425	:
     case	RmT_T400	:
     case	RmT_i860	:
     case	RmT_C40		: /* until heterogenous networks... */
				  return("/helios/lib/nucleus");
     default			: return(Null(char));
   }
}

/**
*** If the resource map specifies a nucleus for either source or destination
*** processor, and the two are different, then it is easy to pick a
*** nucleus. Otherwise default nuclei are obtained for both processors
*** and compared. If the same, easy.
**/

bool		BuildTRANConfig(RmProcessor source,
	RmProcessor dest, int destlink, Config **config_vec, word *confsize)
{ char			*nucleus_string = "";
  char			*source_nuc	= (char *)RmGetProcessorNucleus(source);
  char			*dest_nuc	= (char *)RmGetProcessorNucleus(dest);  
  char			*buffer		= Null(char);
  int			length = 0, temp_length;
  int			number_links;
  int			i;
  bool			result		= FALSE;
  Config		*config;
  ProcessorEntry	*proc_entry;      

  proc_entry = GetProcEntry(dest);

  if (source_nuc ne Null(char))
   if (source_nuc[0] eq '\0')
    source_nuc = Null(char);

  if (dest_nuc ne Null(char))
   if (dest_nuc[0] eq '\0')
    dest_nuc = Null(char);
          
  if ((source_nuc eq Null(char)) && (dest_nuc ne Null(char)))
   { nucleus_string = dest_nuc; goto done_nuc; }

  if ((source_nuc ne Null(char)) && (dest_nuc eq Null(char)))
   { nucleus_string = default_nucleus(dest); goto done_nuc; }

  if ((source_nuc ne Null(char)) && (dest_nuc ne Null(char)))
   { if (strcmp(source_nuc, dest_nuc)) nucleus_string = dest_nuc;
     goto done_nuc;
   }

  source_nuc = default_nucleus(source);
  dest_nuc   = default_nucleus(source);
  if (source_nuc ne dest_nuc)		/* Assumes constant strings */
   nucleus_string = dest_nuc;
  
done_nuc:
  temp_length = strlen(nucleus_string) + 1;
  temp_length = (temp_length + 3) & ~3;

  buffer = (char *) Malloc(IOCDataMax);
  if (buffer eq Null(char)) goto done;
  
  (void) BuildName(buffer, source);
  temp_length += strlen(buffer) + 1;
  (void) BuildName(buffer, dest);
  temp_length += strlen(buffer) + 1;

  temp_length += sizeof(Config);
  number_links = RmCountLinks(dest);
  if (number_links > 4)
   temp_length = temp_length + ((number_links - 4) * sizeof(LinkConf));
  length = (temp_length + 3) & ~3;
  
  if (length > IOCDataMax)
   { report("internal error, the bootstrap information for %P is too large", dest);
     goto done;
   }

  strcpy(buffer, nucleus_string);
  temp_length = strlen(nucleus_string) + 1;
  temp_length = (temp_length + 3) & ~3;
  
  config = (Config *) &(buffer[temp_length]);
  config->PortTabSize	= 32;
  config->Incarnation	= proc_entry->Incarnation++;
  config->ImageSize	= 0;	/* Filled in by Netagent */
  config->Date		= 0;	/* Ditto.		 */
  config->MemSize	= RmGetProcessorMemory(dest);
  config->NLinks	= number_links;
  config->FirstProg	= 6;
  config->Flags		= 0;
  config->LoadBase	= (MPtr) 0x80001000;

  for (i = 0; i < number_links; i++)
   { config->LinkConf[i].Flags	= 0;
     config->LinkConf[i].Mode	= Link_Mode_Null;
     config->LinkConf[i].State	= Link_State_Null;
     config->LinkConf[i].Id	= i;
   }
  config->LinkConf[destlink].Flags	=
  		 (Link_Flags_parent + Link_Flags_debug);
  config->LinkConf[destlink].Mode	= Link_Mode_Intelligent;
  config->LinkConf[destlink].State	= Link_State_Running;

  { char *temp  = (char *) config;
    temp = &(temp[sizeof(Config)]);
    if (number_links > 4)
     temp = &(temp[(number_links - 4) * sizeof(LinkConf)]);
    config->MyName	= temp - (char *) &(config->MyName);
    temp		= BuildName(temp, dest);
    config->ParentName	= temp - (char *) &(config->ParentName);
    (void)BuildName(++temp, source);
  }

  result = TRUE;

done:
  if (result)
   { *config_vec	= (Config *) buffer;
     *confsize		= length;
   }
  else
   { *config_vec	= Null(Config);
     *confsize		= 0;
     if (buffer ne Null(char)) Free(buffer);
   }
  return(result);
}

/*}}}*/
/*{{{  build a C40 configuration vector */

/*{{{  decode() */
/** This little routine decodes ascii strings back to binary. Ascii
*** strings are used to hold the hardware configuration word and
*** the idrom info.
**/
static void decode(char *text, byte *dest)
{ while (*text ne '\0')
   { int x;
     if ((*text >= '0') && (*text <= '9'))
      x = *text - '0';
     else
      x = *text - 'a' + 10;
     x = (x << 4) & 0x0F0;
     text++;
     if ((*text >= '0') && (*text <= '9'))
      x += *text - '0';
     else
      x += *text - 'a' + 10;
     text++;
     *dest++ = x;
   }
}
/*}}}*/

bool		BuildC40Config(RmProcessor source,
	RmProcessor dest, int destlink, Config **config_vec, word *confsize)
{ char			*nucleus_string = "";
  char			*source_nuc	= (char *)RmGetProcessorNucleus(source);
  char			*dest_nuc	= (char *)RmGetProcessorNucleus(dest);  
  char			*bootfile	= RmGetObjectAttribute((RmObject) dest, "bootfile", TRUE);
  char			*hwconfig	= RmGetObjectAttribute((RmObject) dest, "hwconfig", TRUE);
  char			*idrom		= RmGetObjectAttribute((RmObject) dest, "idrom", TRUE);
  char			*buffer		= Null(char);
  int			length;
  int			number_links;
  int			i;
  bool			result		= FALSE;
  Config		*config;
  int			 config_length;
  ProcessorEntry	*proc_entry;      
  C40_Bootstrap		*boot_info = NULL;

  proc_entry = GetProcEntry(dest);

  if (source_nuc ne Null(char))
   if (source_nuc[0] eq '\0')
    source_nuc = Null(char);

  if (dest_nuc ne Null(char))
   if (dest_nuc[0] eq '\0')
    dest_nuc = Null(char);
          
  if ((source_nuc eq Null(char)) && (dest_nuc ne Null(char)))
   { nucleus_string = dest_nuc; goto done_nuc; }

  if ((source_nuc ne Null(char)) && (dest_nuc eq Null(char)))
   { nucleus_string = default_nucleus(dest); goto done_nuc; }

  if ((source_nuc ne Null(char)) && (dest_nuc ne Null(char)))
   { if (strcmp(source_nuc, dest_nuc)) nucleus_string = dest_nuc;
     goto done_nuc;
   }

  source_nuc = default_nucleus(source);
  dest_nuc   = default_nucleus(source);
  if (source_nuc ne dest_nuc)		/* Assumes constant strings */
   nucleus_string = dest_nuc;
  
done_nuc:

	/* Work out the size of the C40 bootstrap structure.		*/
  length	= sizeof(C40_Bootstrap);

  if (nucleus_string[0] ne '\0')
   length += strlen(nucleus_string) + 1;

  if (bootfile ne NULL)
   length += strlen(bootfile) + 1;

	/* Now add on the size of the configuration vector. This	*/
	/* requires knowing the size of the two processor names, for	*/
	/* which I need a buffer. This same buffer can then be used for	*/
	/* the actual bootstrap info.					*/
  buffer = (char *) Malloc(IOCDataMax);
  if (buffer eq Null(char)) goto done;

  config_length	= sizeof(Config);
  (void) BuildName(buffer, source);
  config_length	+= strlen(buffer) + 1;
  (void) BuildName(buffer, dest);
  config_length	+= strlen(buffer) + 1;

  number_links = RmCountLinks(dest);
  if (number_links > 4)
     config_length	+= ((number_links - 4) * sizeof(LinkConf));
  config_length = (config_length + 3) & ~3;

  length -= sizeof(Config);	/* Do not count this twice	*/
  length += config_length;
  length  = (length + 3) & ~3;

  if (length > IOCDataMax)
   { report("internal error, the bootstrap information for %P is too large", dest);
     goto done;
   }

  boot_info		= (C40_Bootstrap *) buffer;
  config		= &(boot_info->Config);
  boot_info->Nucleus	= 0;
  boot_info->Bootfile	= 0;

  if (hwconfig eq NULL)
   {	/* If there is no hardware config information in an idrom file	*/
	/* then I want to default to values used in the bootstrap of	*/
	/* the root processor, i.e. the host.con values.		*/
#ifndef __C40
	/* Heterogenous network, there is no hwconfig word...		*/
     boot_info->Hwconfig	= 0;
#else
     word this_hwconfig	= GetHWConfig();
     boot_info->Hwconfig	= 0;
     if (this_hwconfig & HW_NucleusLocalS1)
      boot_info->Hwconfig	|= HW_NucleusLocalS1;
     elif (this_hwconfig & HW_NucleusGlobalS0)
      boot_info->Hwconfig	|= HW_NucleusGlobalS0;
     elif (this_hwconfig & HW_NucleusGlobalS1)
      boot_info->Hwconfig	|= HW_NucleusGlobalS1;
     if (this_hwconfig & HW_CacheOff)
      boot_info->Hwconfig	|= HW_CacheOff;
#endif
   }
  else
   decode(hwconfig, (BYTE *) &(boot_info->Hwconfig));

  if (idrom ne NULL)
   decode(idrom, (BYTE *) &(boot_info->Idrom));
  boot_info->ConfigSize	= config_length;

  buffer	 = (BYTE *) boot_info;
  buffer	+= offsetof(C40_Bootstrap, Config);
  buffer	+= config_length;

  if (nucleus_string[0] ne '\0')
   { strcpy(buffer, nucleus_string);
     boot_info->Nucleus = (RPTR) buffer;
     boot_info->Nucleus = ATOR(boot_info->Nucleus);
     buffer += strlen(nucleus_string) + 1;
   }
  if (bootfile ne NULL)
   { strcpy(buffer, bootfile);
     boot_info->Bootfile = (RPTR) buffer;
     boot_info->Bootfile = ATOR(boot_info->Bootfile);
   }

  config->PortTabSize	= 32;
  config->Incarnation	= proc_entry->Incarnation++;
  config->ImageSize	= 0;	/* Filled in by Netagent */
  config->Date		= 0;	/* Ditto.		 */
  config->MemSize	= RmGetProcessorMemory(dest);
  config->NLinks	= number_links;
  config->FirstProg	= 6;
  config->Flags		= 0;
  config->Spare[0]      = 0;
  if (boot_info->Hwconfig & HW_CacheOff)
   config->Flags |= Root_Flags_CacheOff;
  config->LoadBase	= 0;
  for (i = 0; i < number_links; i++)
   { config->LinkConf[i].Flags	= 0;
     config->LinkConf[i].Mode	= Link_Mode_Null;
     config->LinkConf[i].State	= Link_State_Null;
     config->LinkConf[i].Id	= i;
   }
  config->LinkConf[destlink].Flags	=
  		 (Link_Flags_parent + Link_Flags_debug);
  config->LinkConf[destlink].Mode	= Link_Mode_Intelligent;
  config->LinkConf[destlink].State	= Link_State_Running;

  { char *temp  = (char *) config;
    temp = &(temp[sizeof(Config)]);
    if (number_links > 4)
     temp = &(temp[(number_links - 4) * sizeof(LinkConf)]);
    config->MyName	= temp - (char *) &(config->MyName);
    temp		= BuildName(temp, dest);
    config->ParentName	= temp - (char *) &(config->ParentName);
    (void)BuildName(++temp, source);
  }

  result = TRUE;

done:
  if (result)
   { *config_vec	= (Config *) boot_info;
     *confsize		= length;
   }
  else
   { *config_vec	= Null(Config);
     *confsize		= 0;
     if (buffer ne Null(char)) Free(buffer);
   }
  return(result);
}

/*}}}*/
/*{{{  build an i860 configuration vector */

bool Build_i860Config(RmProcessor source,
	RmProcessor dest, int destlink, Config **config_vec, word *confsize)
{ char			*nucleus_string = "";
  char			*source_nuc	= (char *)RmGetProcessorNucleus(source);
  char			*dest_nuc	= (char *)RmGetProcessorNucleus(dest);  
  char			*buffer		= Null(char);
  int			length = 0, temp_length;
  int			number_links;
  int			i;
  bool			result		= FALSE;
  Config		*config;
  ProcessorEntry	*proc_entry;      

  proc_entry = GetProcEntry(dest);

  if (source_nuc ne Null(char))
   if (source_nuc[0] eq '\0')
    source_nuc = Null(char);

  if (dest_nuc ne Null(char))
   if (dest_nuc[0] eq '\0')
    dest_nuc = Null(char);
          
  if ((source_nuc eq Null(char)) && (dest_nuc ne Null(char)))
   { nucleus_string = dest_nuc; goto done_nuc; }

  if ((source_nuc ne Null(char)) && (dest_nuc eq Null(char)))
   { nucleus_string = default_nucleus(dest); goto done_nuc; }

  if ((source_nuc ne Null(char)) && (dest_nuc ne Null(char)))
   { if (strcmp(source_nuc, dest_nuc)) nucleus_string = dest_nuc;
     goto done_nuc;
   }

  source_nuc = default_nucleus(source);
  dest_nuc   = default_nucleus(source);
  if (source_nuc ne dest_nuc)		/* Assumes constant strings */
   nucleus_string = dest_nuc;
  
done_nuc:

  temp_length = strlen(nucleus_string) + 1;
  temp_length = (temp_length + 3) & ~3;

  buffer = (char *) Malloc(IOCDataMax);
  if (buffer eq Null(char)) goto done;
  
  (void) BuildName(buffer, source);
  temp_length += strlen(buffer) + 1;
  (void) BuildName(buffer, dest);
  temp_length += strlen(buffer) + 1;

  temp_length += sizeof(Config);
  number_links = RmCountLinks(dest);
  if (number_links > 4)
   temp_length = temp_length + ((number_links - 4) * sizeof(LinkConf));
  length = (temp_length + 3) & ~3;
  
  if (length > IOCDataMax)
   { report("internal error, the bootstrap information for %P is too large", dest);
     goto done;
   }

  strcpy(buffer, nucleus_string);
  temp_length = strlen(nucleus_string) + 1;
  temp_length = (temp_length + 3) & ~3;
  
  config = (Config *) &(buffer[temp_length]);
  config->PortTabSize	= 32;
  config->Incarnation	= proc_entry->Incarnation++;
  config->ImageSize	= 0;	/* Filled in by Netagent */
  config->Date		= 0;	/* Ditto.		 */
  config->MemSize	= RmGetProcessorMemory(dest);
  config->NLinks	= number_links;
  /* Work to be done */

  for (i = 0; i < number_links; i++)
   { config->LinkConf[i].Flags	= 0;
     config->LinkConf[i].Mode	= Link_Mode_Null;
     config->LinkConf[i].State	= Link_State_Null;
     config->LinkConf[i].Id	= i;
   }
  config->LinkConf[destlink].Flags	=
  		 (Link_Flags_parent + Link_Flags_debug);
  config->LinkConf[destlink].Mode	= Link_Mode_Intelligent;
  config->LinkConf[destlink].State	= Link_State_Running;

  { char *temp  = (char *) config;
    temp = &(temp[sizeof(Config)]);
    if (number_links > 4)
     temp = &(temp[(number_links - 4) * sizeof(LinkConf)]);
    config->MyName	= temp - (char *) &(config->MyName);
    temp		= BuildName(temp, dest);
    config->ParentName	= temp - (char *) &(config->ParentName);
    (void)BuildName(++temp, source);
  }

  result = TRUE;

done:
  if (result)
   { *config_vec	= (Config *) buffer;
     *confsize		= length;
   }
  else
   { *config_vec	= Null(Config);
     *confsize		= 0;
     if (buffer ne Null(char)) Free(buffer);
   }
  return(result);
}

/*}}}*/


static bool boot_down_link(RmProcessor start, int link)
{ RmProcessor		target;
  int			destlink;
  ProcessorEntry	*proc_entry;
  Config		*config = Null(Config);
  word			confsize;
  bool			result = FALSE;
  int			i;
  int			NetagentRequest;

  target = RmFollowLink(start, link, &destlink);
  if ((target eq RmM_NoProcessor) || (target eq RmM_ExternalProcessor))
   return(FALSE);

  Debug(dbg_Boot, ("attempting to boot processor %P via link %d of %P",\
  		target, link, start));

	/* Check for any special bootstrap facilities.			    */
	/* If there is one, invoke it. If successful, update the processor. */
  proc_entry = GetProcEntry(target);
  for (i = 0; i < proc_entry->NumberDrivers; i++)
   { DriverEntry	*driver_entry	= &(proc_entry->DriverEntry[i]);
     RmHardwareFacility	*hardware	= driver_entry->Hardware;
     if ((hardware->Type ne RmH_ResetDriver) &&
         (hardware->Type ne RmH_ConfigureDriver))
      continue;
     if (driver_entry->Flags & DriverFlags_SpecialBootstrap)
      { NetworkDCB	*device = (NetworkDCB *) hardware->Device;
        DriverRequest	request;
        request.FnRc			= ND_Boot;
        request.NumberProcessors	= link;
	request.Processor[0]		= start;
        (*(device->DCB.Operate))(device, &request);
        if (request.FnRc eq Err_Null) goto success;
        if ((request.FnRc & EC_Mask) >= EC_Error) goto fail;
      }
   }

	/* Build a suitable configuration vector. This is done in a	*/
	/* separate routine which may be called from device drivers.	*/
	/* Also, select the correct processor type. This may need	*/
	/* updating in future, to allow for combinations of bootstrap.	*/
	/* e.g. a different bootstrap is needed for trannie->trannie	*/
	/* than for i860->trannie.					*/
  switch (RmGetProcessorType(target))
   {
#ifdef __TRAN
  case		RmT_Default	:
#endif
  case		RmT_T800	:
  case		RmT_T414	:
  case		RmT_T425	:
  case		RmT_T400	:
		unless(BuildTRANConfig(start, target, destlink, &config, &confsize))
			goto fail;
		NetagentRequest = NA_TransputerBoot;
		break;
#ifdef __C40
     case RmT_Default :
#endif
     case RmT_C40     :
		unless(BuildC40Config(start, target, destlink, &config, &confsize))
			goto fail;
		NetagentRequest	= NA_C40Boot;
		break;

#ifdef __I860
     case	RmT_Default	:
#endif
     case	RmT_i860	:
		unless(Build_i860Config(start, target, destlink, &config, &confsize))
			goto fail;
		NetagentRequest = NA_i860Boot;
		break;

     default :
     	report("this version cannot boot up processor %P, a %s",
	       target, RmT_Names[RmGetProcessorType(target)]);
	goto	fail;			
   }

	/* Reset the processor again if possible */
  (void) ResetProcessors(1, &target);     
  { int state = RmGetProcessorState(target);
    unless(state & RmS_Reset)
     report("warning, trying to boot processor %P which has not been reset",
     		target);
  }
	/* Try to access the Network agent running on the start processor */
  { NA_Message		message;
    word		reply;
    int			state;

    message.FnRc = NetagentRequest;
    message.Arg1 = link;
    message.Size = confsize;
    message.Data = (BYTE *) config;
    reply = XchNetworkAgent(start, &message, TRUE, 0, NULL);

    state = RmGetProcessorState(target);
    state &= ~RmS_Reset;
    RmSetProcessorState(target, state);

    if (reply ne Err_Null)
     { report("failed to boot processor %P via link %d of processor %P",
     		target, link, start);
       report("reported fault was %x", reply);

       goto fail;
     }
  }

	/* Check that the processor really has come up, by	*/
	/* starting a network agent on it.			*/
  unless(StartNetworkAgent(target))
   { SetLinkMode(start, link, RmL_NotConnected);
     report("failed to start Network Agent on newly-booted processor %P",
		target);
     goto fail;
   }
    
success:
  RmSetProcessorState(target, RmS_Running | RmS_Booting);
  LastChange = target->ObjNode.Dates.Modified = GetDate();
  result = TRUE;
  goto done;
    
fail:
  { RmLink *linkstruct = RmFindLink(start, link);
    int	    state = RmGetProcessorState(target);
    linkstruct->Flags |= RmF_Suspicious;
    state &= ~RmS_Reset;
    RmSetProcessorState(target, state);
  }

done:  
  if (config ne Null(Config)) Free(config);
  return(result);
}

/*}}}*/

/*{{{  install the protection on a processor */
/**
*** do_protection(). For every processor in the bootstrap job set up the
*** appropriate protection modes and initialise some capabilities. This
*** involves interacting with the network agent on that processor.
***
*** 1) The Network agent is guaranteed to be running already.
*** 3) construct a protect message. This involves a single argument, the
***    protection matrix to use, which is filled in during initialisation
***    in routine InitBootstrap() above. The message is followed by
***    the current capability, which may be the only capability in the
***    system with alter permission.
*** 4) send the message and get a reply. This reply consists of
***    an error code followed by three capabilities, one for the owner, one
***    read-only, and one for the network server itself.
**/
static void do_protection(RmProcessor processor)
{ ProcessorEntry	*proc_entry;
  NA_Message		message;
  WORD			rc;

  proc_entry = GetProcEntry(processor);

  Debug(dbg_Boot, ("setting protection on processor %P", processor));
     	
  message.FnRc = NA_Protect;
  message.Arg1 = ProtectionMatrix;
  message.Size = sizeof(Capability);
  message.Data = (BYTE *) &(proc_entry->Full);
  rc = XchNetworkAgent(processor, &message, TRUE, 3 * sizeof(Capability), 
		(BYTE *) &(proc_entry->Owner));

  if (rc ne Err_Null)
   { report("warning, failed to set protection on processor %P, fault %x",
      		processor, rc);
     memset((void *) &(proc_entry->Owner), 0, 3 * sizeof(Capability));
   }
  else
   { memcpy(&(processor->RealCap), &(proc_entry->Full), sizeof(Capability));
     memcpy(&(processor->ReadOnlyCap), &(proc_entry->General), 
        		sizeof(Capability));
   }
}
/*}}}*/
/*{{{  validate details of a processor, e.g. processor type */

/**
*** ValidateProcessor. Check out some of the details in the resource map. In
*** particular, validate the processor type and the amount of memory.
**/

static void	ValidateProcessorDetails(RmProcessor processor)
{ int		state		= RmGetProcessorState(processor);
  Object	*real_processor = Null(Object);
  ProcStats	*stats		= Null(ProcStats);
  int		number_links	= RmCountLinks(processor);
  int		proc_type	= RmGetProcessorType(processor);
  int		real_type;

  unless (state & (RmS_Running | RmS_Special)) return;
  real_processor = NetMapProcessorToObject(processor);
  if (real_processor eq Null(Object))
   { MarkProcessor(processor, TRUE); goto done; }
  stats = (ProcStats *) Malloc(sizeof(ProcStats) + 
  		((word) number_links * (sizeof(LinkConf) + 3 * sizeof(WORD))) +
  		IOCDataMax);
  if (stats eq Null(ProcStats)) goto done;

  if (ServerInfo(real_processor, (BYTE *) stats) < Err_Null)
   { report("error %x when attempting to validate processor %P",
   		Result2(real_processor), processor);
     MarkProcessor(processor, TRUE);
     goto done;
   }

	/* the memory information returned by ServerInfo does not include */
	/* the nucleus, trace vector, root structure, and other kernel	  */
	/* data. Hence I align the memory to the next 512K boundary.	  */
  if (RmGetProcessorMemory(processor) eq 0)
   { word	memory = stats->MemMax;
     memory	= memory + (512 * 1024) - 1;
     memory	&= ~((512 * 1024) - 1);
     (void) RmSetProcessorMemory(processor, memory);
   }
	/* if an amount was specified in the resource map then this	*/
	/* was used to boot up the processor. Hence there is no point	*/
	/* in adjusting the memory size, except possibly for the root	*/
	/* processor.							*/

	/* Next job, check the processor type.				*/
  switch(stats->Type)
   { case	800	: 
     case	805	:
     case	801	:
   			  real_type = RmT_T800; break;
     case	414	: real_type = RmT_T414; break;
     case	425	: real_type = RmT_T425; break;
     case	400	: real_type = RmT_T400; break;
     case	0xA3	: real_type = RmT_Arm;  break;
     case	0x320C40: real_type = RmT_C40;  break;
     case	0x86	: real_type = RmT_i860; break;

     default :	report("warning, processor %P is of unknown type %d",
			processor, stats->Type);
     		goto done;
   }

  if (proc_type eq RmT_Default)
   RmSetProcessorType(processor, real_type);
  elif (proc_type ne real_type)
   { report(
     "warning, processor %P is a %s, not a %s as specified in the resource map",
	       processor, RmT_Names[real_type], RmT_Names[proc_type]);
     RmSetProcessorType(processor, real_type);
   }

done:
  if (real_processor ne Null(Object)) Close(real_processor);
  if (stats ne Null(ProcStats)) Free(stats);
}

/*}}}*/
/*{{{  cross-links, pending phase */
/**
*** set_pending(processor, job). This routine is called during the
*** first stage of enabling the cross links. There is guaranteed to be
*** a netagent running already. The routine performs the following
*** operations for each link:
***  1) if the link is not connected or goes to a native processor
***     it is ignored, as the link will have been set to not-connected
***     during the bootstrap.
***  2) if the link goes to a processor that is not currently running
***     it is set to not-connected. The recovery code should do the
***     rest.
***  3) if the link is external to the network it is set to pending.
***     Similarly for I/O processors.
***  4) if the link is to a processor outside the bootstrap job then
***     a netagent is started on the other processor, the other link is
***     set to pending, and this link is enabled.
***  5) if the link goes to another link on the same processor then set
***     it pending or ignore it, depending on the order of the links
***  6) if the link goes to a processor that was booted later on in the
***     job then the link is set to pending.
BLV This code does not currently know which links were used for the
BLV bootstrap. Hence it may attempt to fiddle with some links that are
BLV already running. The netagent sorts that out.
**/
static void set_pending(RmProcessor processor, BootstrapJob *job)
{ int		this_position;
  int		its_position;
  int		number_links;
  int		link;
  int		destlink;
  int		mode;
  int		its_purpose;
  RmProcessor	neighbour;
  NA_Message	message;

	/* Determine this processor's position within the job */
  for (this_position = 0; this_position < job->Next; this_position++)
   if (job->Progress[this_position] eq processor)
    break;

  number_links = RmCountLinks(processor);

  for (link = 0; link < number_links; link++)
   { neighbour = RmFollowLink(processor, link, &destlink);

     if (neighbour eq RmM_NoProcessor) continue;

     if (neighbour eq RmM_ExternalProcessor)
      { mode = RmL_Pending; goto set_link; }

     its_purpose = RmGetProcessorPurpose(neighbour) & RmP_Mask;

     if (its_purpose eq RmP_IO)
      { mode = RmL_Pending; goto set_link; }

     if (its_purpose ne RmP_Helios)
      continue;

     if (!(RmGetProcessorState(neighbour) & RmS_Running))
      { mode = RmL_NotConnected; goto set_link; }

     if (processor eq neighbour)
      { if (link < destlink)
         { mode = RmL_Pending; goto set_link; }
        else
         continue;
      }

     for (its_position = 0; its_position < job->Next; its_position++)
      if (job->Progress[its_position] eq neighbour)
       break;

     if (its_position >= job->Next)
      { SetLinkMode(neighbour, destlink, RmL_Pending);
        mode = RmL_Running;
        goto set_link;
      }

    if (its_position < this_position)
     continue;

    mode = RmL_Pending;

set_link:
    Debug(dbg_Links, ("processor %P, setting link %d to %L",\
		processor, link, mode));

    message.FnRc	= NA_SetLinkMode;
    message.Arg1	= link;
    message.Arg2	= mode;
    message.Size	= 0;
    XchNetworkAgent(processor, &message, TRUE, 0, NULL);
  }
}
/*}}}*/
/*{{{  cross-links, enabling phase */
/**
*** set_enabled(processor, job). This routine complements the previous one.
*** For links within the bootstrap job one end will have been set to pending.
*** The other end should now be set to running and enabled.
**/
static void set_enabled(RmProcessor processor, BootstrapJob *job)
{ int		this_position;
  int		its_position;
  int		number_links;
  int		link;
  int		destlink;
  int		mode;
  int		its_purpose;
  RmProcessor	neighbour;
  NA_Message	message;

	/* Determine this processor's position within the job */
  for (this_position = 0; this_position < job->Next; this_position++)
   if (job->Progress[this_position] eq processor)
    break;

  number_links = RmCountLinks(processor);

  for (link = 0; link < number_links; link++)
   { neighbour = RmFollowLink(processor, link, &destlink);

     if ((neighbour eq RmM_NoProcessor) || (neighbour eq RmM_ExternalProcessor))
	continue;

     its_purpose = RmGetProcessorPurpose(neighbour) & RmP_Mask;
     if (its_purpose ne RmP_Helios)
      continue;

     if (!(RmGetProcessorState(neighbour) & RmS_Running))
      continue;

     if (processor eq neighbour)
      { if (link > destlink)
         { mode = RmL_Running; goto set_link; }
        else
         continue;
      }

     for (its_position = 0; its_position < job->Next; its_position++)
      if (job->Progress[its_position] eq neighbour)
       break;

     if (its_position >= job->Next)
      continue;

    if (its_position >= this_position)
     continue;

    mode = RmL_Running;

set_link:
    Debug(dbg_Links, ("processor %P, setting link %d to %L",\
		processor, link, mode));

    message.FnRc	= NA_SetLinkMode;
    message.Arg1	= link;
    message.Arg2	= mode;
    message.Size	= 0;
    XchNetworkAgent(processor, &message, TRUE, 0, NULL);
  }
}
/*}}}*/
/*{{{  executing system tasks defined in the resource map */

/**
*** Execute_mapped_task(). The network resource map may contain various tasks
*** that may have been mapped onto a processor. This routine is used to
*** execute the appropriate task. In addition to ordinary commands the
*** tasks may include the specials waitfor and console
**/
/*{{{  waitfor command */
static	void	execute_waitfor(RmTask task, RmProcessor processor)
{ char		*server_name = (char *) RmGetTaskArgument(task, 1);
  int		i = 1;
  Object	*server;

  if (server_name[0] ne '/')
   { report("processor %P, invalid server name %s in waitfor",
		processor, server_name);
     return;
   }

  forever
   { server = Locate(Null(Object), server_name);
     if (server != Null(Object))
      { Close(server); return; }
     if ((i++ % 30) == 0)
      report("processor %P, still waiting for %s", processor, server_name);
     Delay(OneSec);
   }
}
/*}}}*/
/*{{{  console command */
static void	execute_console(RmTask task, RmProcessor processor)
{ ProcessorEntry	*proc_entry;
  Object		*window_server = Null(Object);
  char			*server_name;
  char			*window_name;

  proc_entry = GetProcEntry(processor);

  server_name = (char *) RmGetTaskArgument(task, 1);
  if (server_name[0] ne '/')
   { report("processor %P, invalid console server %s", processor,
		server_name);
     return;
   }

  window_server = Locate(Null(Object), server_name);
  if (window_server eq Null(Object))
   { report("processor %P, cannot find window server %s", processor,
		server_name);
     return;
   }

  if (proc_entry->WindowServer != Null(Object))
   Close(proc_entry->WindowServer);
  if (proc_entry->ConsoleWindow != Null(Object))
   Close(proc_entry->ConsoleWindow);

  if (RmCountTaskArguments(task) eq 2) /* console /logger */
   { proc_entry->WindowServer = Null(Object);
     proc_entry->ConsoleWindow = window_server;
     return;
   }

  window_name = (char *) RmGetTaskArgument(task, 2);
  proc_entry->WindowServer = window_server;
  proc_entry->ConsoleWindow = Create(window_server, window_name, 
			Type_File, 0, NULL);
  if (proc_entry->ConsoleWindow eq Null(Object))
   { report("processor %P, failed to create window %s/%s", processor,
		window_server->Name, window_name);
     proc_entry->WindowServer = Null(Object);
     return;
   }
}
/*}}}*/

static	int	execute_mapped_task(RmTask task, ...)
{ va_list		args;
  RmProcessor		processor;
  ProcessorEntry	*proc_entry;

  va_start(args, task);
  processor = va_arg(args, RmProcessor);
  va_end(args);

  if (!strcmp(task->ObjNode.Name, "waitfor"))
   { execute_waitfor(task, processor); return(0); }
  if (!strcmp(task->ObjNode.Name, "console"))
   { execute_console(task, processor); return(0); }

	/* Must be an ordinary command */

  proc_entry = GetProcEntry(processor);
  proc_entry->CommandDate = (word) GetDate() + 2;
  
  if (task->ObjNode.Name[0] eq '0')	/* hack for no -e, see rmgen */
   return((int) rexec_task(processor, task, Null(Environ), 0));

  { Environ	env;
    Environ	*net_env = getenviron();
    Stream	*strv[5];
    Object	*objv[OV_End + 1];
    int		i;

	/* By default, inherit context and streams from Network Server */
    for (i = 0; (net_env->Objv[i] ne Null(Object)) && (i <= OV_End); i++)
     objv[i] = net_env->Objv[i];
    objv[OV_End] = Null(Object);
    for (i = 0; i < 4; i++)
     strv[i] = net_env->Strv[i];
    strv[4] = Null(Stream);

    if (proc_entry->WindowServer ne Null(Object))
     objv[OV_CServer] = proc_entry->WindowServer;

    if (proc_entry->ConsoleWindow ne Null(Object))
     { objv[OV_Console] = proc_entry->ConsoleWindow;
       strv[0] = Open(proc_entry->ConsoleWindow, Null(char), O_ReadOnly);
       strv[1] = Open(proc_entry->ConsoleWindow, Null(char), O_WriteOnly);
       strv[2] = strv[1];
     }
    env.Strv	= strv;
    env.Objv	= objv;
    env.Envv	= net_env->Envv;
    env.Argv	= Null(char *);
    
    i = (int) rexec_task(processor, task, &env, 0);
    if (proc_entry->ConsoleWindow ne Null(Object))
     { Close(strv[0]); Close(strv[1]); }
    return(i);
  }
}

/*}}}*/

/*{{{  manipulating link modes outside a bootstrap */

/*{{{  HandleGetLinkMode() */
/**
*** Code to deal with requests for manipulating link modes. This is temporary.
*** NB the network server is not locked up while this is going on.
**/
void	HandleGetLinkMode(NsConn connection, int job_id,
		RmRequest *request, RmReply *reply)
{ RmProcessor		processor;
  int			rc = RmE_Success;
  NA_Message		message;
  bool			agent_started	= FALSE;
  int			mode;    

  processor = RmFindProcessor(Net, request->Uid);

  if (processor eq (RmProcessor) NULL)   
   { rc = RmE_BadProcessor; goto done; }
  if ((RmGetProcessorPurpose(processor) & RmP_Mask) ne RmP_Helios)
   { rc = RmE_BadProcessor; goto done; }

  unless(StartNetworkAgent(processor))
   { rc = RmE_BadProcessor; goto done; }
  agent_started = TRUE;
  
  message.FnRc	= NA_GetLinkMode;
  message.Arg1	= request->Arg1;  
  message.Size	= 0;
  rc = XchNetworkAgent(processor, &message, TRUE, sizeof(int), (BYTE *) &mode);

done:
  if (agent_started) StopNetworkAgent(processor);
  reply->FnRc	= rc;
  reply->Reply1	= mode;
  ReplyRmLib(connection, job_id, reply);
}
/*}}}*/
/*{{{  HandleSetLinkMode() */
void	HandleSetLinkMode(NsConn connection, int job_id,
		RmRequest *request, RmReply *reply)
{ RmProcessor		processor;
  int			rc = RmE_Success;

  processor = RmFindProcessor(Net, request->Uid);
  if (processor eq (RmProcessor) NULL)   
   { rc = RmE_BadProcessor; goto done; }
  if ((RmGetProcessorPurpose(processor) & RmP_Mask) ne RmP_Helios)
   { rc = RmE_BadProcessor; goto done; }

  rc = SetLinkMode(processor, request->Arg1, request->Arg2);
  
done:
  reply->FnRc	= rc;
  ReplyRmLib(connection, job_id, reply);
}
/*}}}*/
/*{{{  SetLinkMode() */
/**
*** Set a link to a particular mode. This is used for one-off changes,
*** when all the links need changing there are more efficient ways.
**/
int	SetLinkMode(RmProcessor processor, int link, int mode)
{ int			rc = RmE_Success;
  NA_Message		message;
  bool			agent_started	= FALSE;

  Debug(dbg_Links, ("setting link %d of %P to %L", link, processor, mode));

  unless(StartNetworkAgent(processor))
   { rc = RmE_BadProcessor; goto done; }
  agent_started = TRUE;  
  
  message.FnRc	= NA_SetLinkMode;
  message.Arg1	= link;
  message.Arg2	= mode;
  message.Size	= 0;
  rc = XchNetworkAgent(processor, &message, TRUE, 0, NULL);

done:
  if (agent_started) StopNetworkAgent(processor);
  return(rc);
}
/*}}}*/
/*{{{  GetLinkMode() */

int	GetLinkMode(RmProcessor processor, int link)
{ Object	*proc_obj;
  Object	*procman;
  BYTE		buf[IOCDataMax];
  ProcStats	*stats;
  word		rc;
  LinkConf	conf;

  proc_obj = NetMapProcessorToObject(processor);
  if (proc_obj eq Null(Object)) return(RmL_NotConnected);

  procman = Locate(proc_obj, "tasks");
  Close(proc_obj);
  if (procman eq Null(Object)) return(RmL_NotConnected);

  rc = ServerInfo(procman, buf);
  Close(procman);
  if (rc < Err_Null) return(RmL_NotConnected);

  stats = (ProcStats *) buf;
  if (link > stats->NLinks) return(RmL_NotConnected);
  conf  = stats->Link[link].Conf;
  if (conf.Mode eq Link_Mode_Null)
   return(RmL_NotConnected);
  if (conf.Mode eq Link_Mode_Dumb)
   return(RmL_Dumb);
  if (conf.State eq Link_State_Running)
   return(RmL_Running);
  if (conf.State eq Link_State_Dead)
   return(RmL_Pending);
  return(RmL_Dead);
}

/*}}}*/

/*}}}*/
/*{{{  clearing name tables */
#if 0
/*{{{  old code */
/**
*** Clearing names. This is yet another message to the network agent, for
*** most cases. However, when fiddling with native networks it is necessary
*** to clear the names of the root processor very frequently, so there
*** is a special case.
**/
static  void ClearLocalNames(void);

void	ClearNames(RmProcessor processor)
{ int			rc;
  NA_Message		message;
  bool			agent_started	= FALSE;

  if (processor eq RootProcessor)
   { ClearLocalNames(); return; }
     
  unless(StartNetworkAgent(processor)) goto done;
  agent_started = TRUE;
  
  message.FnRc	= NA_ClearNames;
  message.Size	= 0;
  rc = XchNetworkAgent(processor, &message, FALSE, 0, NULL);

done:  
  if (agent_started) StopNetworkAgent(processor);
}

static Object *local_procman = Null(Object);

static void ClearLocalNames(void)
{ MsgBuf	*r = New(MsgBuf);

  if (r eq Null(MsgBuf)) return;
  
  if (local_procman eq Null(Object))
   local_procman = Locate(Null(Object), "/tasks");
   
  InitMCB(&(r->mcb), MsgHdr_Flags_preserve, NullPort, NullPort,
  		FC_GSP + FG_Reconfigure);
  r->mcb.Control = r->control;
  r->mcb.Data	 = r->data;
  MarshalCommon(&(r->mcb), local_procman, Null(char));
  SendIOC(&(r->mcb));
  
  Free(r);
}
/*}}}*/
#else
/*{{{  current code */
	/* BLV - alternative approach, send the ClearNames() message	*/
	/* directly to the remote ProcMan.				*/
void	ClearNames(RmProcessor	processor)
{ Object	*proc_obj	= Null(Object);
  Object	*procman	= Null(Object);
  MsgBuf	*r;

  proc_obj	= NetMapProcessorToObject(processor);
  if (proc_obj eq Null(Object)) return;

  procman	= Locate(proc_obj, "tasks");
  Close(proc_obj);
  if (procman eq Null(Object)) return;

  r = New(MsgBuf);
  if (r eq Null(MsgBuf)) 
   { Close(procman); return; }
  r->mcb.Control	= r->control;
  r->mcb.Data		= r->data;
  InitMCB(&(r->mcb), MsgHdr_Flags_preserve, NullPort, NullPort,
		FC_GSP + FG_Reconfigure);
  MarshalCommon(&(r->mcb), procman, Null(char));

  SendIOC(&(r->mcb));

  Free(r);
  Close(procman);
}
/*}}}*/
#endif
/*}}}*/
