/*------------------------------------------------------------------------
--                                                                      --
--           H E L I O S   N E T W O R K I N G   S O F T W A R E	--
--           ---------------------------------------------------	--
--                                                                      --
--             Copyright (C) 1990, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- netmon.c								--
--                                                                      --
--	The monitor module of the Network Server			--
--                                                                      --
--	Author:  BLV 18/8/90						--
--                                                                      --
------------------------------------------------------------------------*/
/*$Header: /hsrc/network/RCS/netmon.c,v 1.24 1993/08/17 11:36:22 bart Exp $*/

/*{{{  headers and compile-time options */
#define	__Netboot_Module
#define __NetworkServer

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
#include <link.h>
#include <root.h>
#include "exports.h"
#include "private.h"
#include "netutils.h"
#include "rmlib.h"
#include "netaux.h"
#include "session.h"
/*}}}*/
/*{{{  forward declarations and statics */
static	void		Monitor_Thread(void);
static	void		StartProblemHandler(void);
static	void		ProblemHandler(void);
	int		Monitor_Delay;
static	Semaphore	ProblemHandlerLock;
static	bool		ProblemHandlerRunning;
static	void		scan_network(RmNetwork network, bool locked);
static	void		scan_processor(RmProcessor);
static	void 		HandleIOChange(RmProcessor IO, int mode, int state);
#if Joinnet_Supported
static	void 		LostExternalNetwork(RmProcessor from, int link);
static	void		HandleIOExternal(RmProcessor IO);
static	void		CopeWithExternalSubnet(RmNetwork);
#endif

void InitMonitor(void)
{ 
  InitSemaphore(&ProblemHandlerLock, 1);
  ProblemHandlerRunning	= FALSE;

	/* Work out the delay between monitor loops. This should	*/
	/* be no more than 30 minutes, to avoid time overflows. If it	*/
	/* is a negative number then the monitor thread is suppressed	*/
	/* completely.							*/
  Monitor_Delay = get_int_config("monitor_interval", nsrc_environ);
  if (Monitor_Delay eq Invalid_config)
   Monitor_Delay = 30;
  elif (Monitor_Delay > (30 * 60))
   Monitor_Delay = 30 * 60;

  if (Monitor_Delay >= 0)
   if (!Fork(Monitor_Stack, &Monitor_Thread, 0))
    fatal("not enough memory to initialise network monitoring");
}
/*}}}*/

/*{{{  continuous monitoring of the network */
/**
*** Network monitoring. This module is responsible for implementing the
*** fault tolerance aspects of the Network Server. It includes a thread
*** spawned off to monitor the whole network. When this thread detects
*** a problem of any sort a separate thread is responsible for cleaning
*** things up. The monitor thread runs without locking the network,
*** but the cleaning thread does not.
***/

/*{{{  the monitoring thread */
/**
*** The monitor thread. This runs continuously checking every processor
*** in the network. There is a system parameter controlling the delay
*** between checks, settable in the nsrc file.
***
*** To cope with sub-networks the implementation involves calling a
*** scan_network() routine which can recurese. The locking is as follows.
*** This thread runs asynchronously to anything else in the network,
*** so the global MRSW locks have to be used. However the routines must
*** not run permanently with a read lock since that would permanently block
*** anything trying to get a read lock. In fact the routines regularly
*** unlock and relock, to allow other threads to get a write lock. 
*** Note that MRSW_GetRead() suspends if anything else is trying to get a
*** write lock.
**/

static	void	Monitor_Thread(void)
{
  forever
   { scan_network(Net, FALSE);
     Delay(Monitor_Delay * OneSec);
   }
}
/*}}}*/
/*{{{  scan_network() */
static void	scan_network(RmNetwork network, bool locked)
{ int		uid;
  RmProcessor	processor;

  unless(locked) MRSW_GetRead();

  processor = RmFirstProcessor(network);

  forever
   { if (ProblemHandlerRunning) break;

     if (RmIsNetwork(processor))
      scan_network((RmNetwork) processor, TRUE);
     else
      scan_processor(processor);

     processor	= RmNextProcessor(processor);
     if (processor eq NULL) break;

	/* Do not delay for subnets, only for processors	*/
     if (RmIsNetwork(processor)) continue;

	/* Remember the Uid for the next processor		*/
     uid = processor->Uid;

	/* Allow other threads to get a write lock		*/
     MRSW_FreeRead();
     MRSW_GetRead();

	/* Check that the processor is still there. If not then	*/
	/* the system is in a state of flux and it is a good	*/
	/* idea to abort this particular run of the monitor.	*/
     processor = RmFindProcessor(Net, uid);
     if (processor eq (RmProcessor) NULL)
      break;
   }
  unless(locked) MRSW_FreeRead();
}
/*}}}*/
/*{{{  scan_processor() */
/**
*** scan_processor(). This performs a very basic test that the processor
*** is still accessible and that all its links are in a sensible state.
*** This involves locating the processor, locating the processor manager,
*** performing a ServerInfo, and scanning the links. This routine can only
*** be called from the single monitor thread.
**/
static void scan_processor(RmProcessor processor)
{ Object	*proc_obj	= Null(Object);
  Object	*procman	= Null(Object);
  static BYTE	servinfo_buf[IOCDataMax];
  ProcStats	*procstats	= (ProcStats *) servinfo_buf;
  int		purpose;
  bool		proc_ok		= TRUE;
  int		state;
int rc;

	/* Native processors and routers cannot be checked.		*/
  purpose	= RmGetProcessorPurpose(processor) & RmP_Mask;
  if ((purpose eq RmP_Native) || (purpose eq RmP_Router))
   return;

	/* Unless the processor is believed to be running, ignore it	*/
  state = RmGetProcessorState(processor);
  unless(state & RmS_Running) return;
	/* Processors in the middle of being booted are guaranteed dodgy*/
  if (state & RmS_Booting) return;

  Debug(dbg_Monitor, ("checking processor %P", processor));
  
  proc_obj	= NetMapProcessorToObject(processor);
  if (proc_obj eq Null(Object)) 
    { proc_ok = FALSE; goto done; }

	/* For I/O processors, check for the error logger.		*/
  if (purpose eq RmP_IO)
   { Object	*logger = Locate(proc_obj, "logger");
     if (logger eq Null(Object))
      proc_ok = FALSE;
     else
      Close(logger);
     goto done;
   }

  procman	= Locate(proc_obj, "tasks");
  if (procman eq Null(Object))
   { proc_ok = FALSE; goto done; }

	/* Clear the buffer to avoid possible confusion			*/
  memset(servinfo_buf, 0, sizeof(servinfo_buf));

	/* This request is guaranteed not to be handled by the cache	*/
  if ((rc = ServerInfo(procman, servinfo_buf)) < Err_Null)
   { proc_ok = FALSE; goto done; }

  if (procstats->NLinks eq 0)
   { report("confused system, ServerInfo(%P) has succeeded but NLinks is 0", processor);
     proc_ok = FALSE;
     goto done;
   }

  { int	number_links	= RmCountLinks(processor);
    int	i;

    if (number_links ne procstats->NLinks)
     { report("processor %P claims to have %d links instead of %d", processor, procstats->NLinks, number_links);
       proc_ok	= FALSE;
       goto	done;
     }

    for (i = 0; i < number_links; i++)
     { RmProcessor	neighbour;
       int		destlink;
       LinkConf		linkconf = procstats->Link[i].Conf;

       neighbour = RmFollowLink(processor, i, &destlink);
       if (neighbour eq RmM_NoProcessor)
        { if ((linkconf.Mode ne Link_Mode_Null) 
              && (linkconf.Mode ne Link_Mode_Dumb))
            MarkLink(processor, i);
	  continue;
        }
       if (neighbour eq RmM_ExternalProcessor)
        { if (linkconf.Mode ne Link_Mode_Intelligent)
	    MarkLink(processor, i);
	  elif ((linkconf.State ne Link_State_Running) &&
		(linkconf.State ne Link_State_Dead))
	    MarkLink(processor, i);
	  continue;
	}
	/* must be a real processor	*/
	{ int its_purpose = RmGetProcessorPurpose(neighbour) & RmP_Mask;
	  int its_running = RmGetProcessorState(neighbour) & (RmS_Running + RmS_Booting);

		/* If the neighbour is an I/O processor then the link	*/
		/* should always be intelligent, pending if the I/O	*/
		/* processor is not running, else running.		*/
	  if (its_purpose eq RmP_IO)
           { if (linkconf.Mode ne Link_Mode_Intelligent)
	      { MarkLink(processor, i); continue; }
	     if (its_running)
              { if (linkconf.State ne Link_State_Running)
		 MarkLink(processor, i);
	      }
	     else
	      { if (linkconf.State ne Link_State_Dead)
	  	 MarkLink(processor, i);
	      }
	     continue;
	   }

		/* If the neighbour is native then the link mode should	*/
		/* be dumb or not connected.				*/
	  if (its_purpose eq RmP_Native)
	   { if ((linkconf.Mode ne Link_Mode_Dumb) &&
		 (linkconf.Mode ne Link_Mode_Null))
	      MarkLink(processor, i);
	     continue;
	   }

		/* If the neighbour is a router I do not know what to	*/
		/* do yet.						*/
	  if (its_purpose eq RmP_Router)
	   continue;

		/* If the neighbour is not a Helios node, HELP!!!	*/
	  if (its_purpose ne RmP_Helios)
	   continue;

		/* If the neighbour is a Helios node that is being	*/
		/* booted then there is little I can guarantee about	*/
		/* this node.						*/
          if ((its_running & RmS_Booting) ne 0)
	   continue;

		/* Otherwise the link must be intelligent, running if	*/
		/* the processor is, otherwise dead.			*/
          if (linkconf.Mode ne Link_Mode_Intelligent)
	   { MarkLink(processor, i); continue; }

	  if (its_running)
	   { if (linkconf.State ne Link_State_Running)
	      MarkLink(processor, i);
	   }
	  else
	   { if (linkconf.State ne Link_State_Dead)
	      MarkLink(processor, i);
	   }
	  {	/* It is OK, reset the suspicious flag	*/
	    RmLink *rm_link = RmFindLink(processor, i);
	    rm_link->Flags &= ~RmF_Suspicious;
	  }
        }
     }
   }

	/* BLV - other checks. The processor should not be owned by	*/
	/* the cleaners for too long a time, its state should not be	*/
	/* funny for too long a time, if dead there should still	*/
	/* be a reboot attempt every 20 minutes or so, etc.		*/

done:
  unless(proc_ok) 
   { Debug(dbg_Monitor, ("processor %P failed its regular checkup", processor));
     MarkProcessor(processor, TRUE);
   }
  if (procman ne Null(Object))	Close(procman);
  if (proc_obj ne Null(Object))	Close(proc_obj);
  return;
}
/*}}}*/
/*}}}*/
/*{{{  MarkProcessor() */
/**
*** MarkProcessor(). This routine is called when something appears to have
*** gone wrong with a particular processor. It performs some checks on
*** that processor and, if it has reason to believe that there is a problem,
*** a ProblemHandler() gets invoked to clean up the mess.
***
*** The second argument indicates that the processor has almost certainly
*** died because either the processor or the processor manager could not
*** be located, an rexec failed, or some other serious failure. If this
*** second argument is FALSE then something may have gone wrong with the
*** processor, but it is not obvious what.
**/
void		MarkProcessor(RmProcessor processor, bool locate_failed)
{ int	state = RmGetProcessorState(processor);

  if (Monitor_Delay < 0)
   { Debug(dbg_Problem, ("monitoring disabled, ignoring potential problem on %P", processor));
     return;
   }

  if (!locate_failed)
   { if (ProblemHandlerRunning)
      { Debug(dbg_Problem, ("leaving problem on %P to existing problem handler", processor));
	return;
      }
     elif (CheckProcessor(processor))
      { Debug(dbg_Problem, ("cannot find reported problem on %P", processor));
        return;
      }
   }
 
  unless (state & RmS_Running) return;

  Debug(dbg_Problem, ("problem reported on processor %P", processor));
  
  state &= ~(RmS_Running | RmS_Crashed | RmS_Dead);
  state |= RmS_Suspicious;
  RmSetProcessorState(processor, state);

  StartProblemHandler();   
}
/*}}}*/
/*{{{  MarkLink() */
void		MarkLink(RmProcessor processor, int link)
{ RmLink	*rm_link = RmFindLink(processor, link);
  RmProcessor	neighbour;
  int		destlink;
  int		purpose;
  int		mode;

  if (Monitor_Delay < 0)
   { Debug(dbg_Problem, ("monitoring disabled, ignoring potential problem on link %d of  %P", link, processor));
     return;
   }

	/* Check if this report is already being handled	*/
  if (rm_link->Flags & RmF_Suspicious) return;

  Debug(dbg_Problem, ("investigating link problem on processor %P, link %d", \
		processor, link));

	/* If there is a problem handler running already, wait	*/
  if (ProblemHandlerRunning)
   { Debug(dbg_Problem, ("leaving link problem to existing problem handler"));
     return;
   }

  rm_link->Flags |= RmF_Suspicious;

  unless(CheckProcessor(processor))
   { StartProblemHandler(); goto done; }
  mode = GetLinkMode(processor, link);

  purpose = RmGetProcessorPurpose(processor) & RmP_Mask;
  if (purpose ne RmP_Helios) goto done;
   
  neighbour = RmFollowLink(processor, link, &destlink);
  if (neighbour eq RmM_NoProcessor)
   { SetLinkMode(processor, link, RmL_NotConnected);
     goto done;
   }
  if (neighbour eq RmM_ExternalProcessor)
   { switch(mode)
       { case RmL_Dead		: SetLinkMode(processor, link, RmL_NotConnected);
	 case RmL_NotConnected	:
	 case RmL_Dumb		: SetLinkMode(processor, link, RmL_Pending);
				  break;
       } 
     goto done;
   }

  Debug(dbg_Problem, ("processor %P, link %d, goes to %P, link %d, current setting %L",\
		processor, link, neighbour, destlink, mode));

  purpose = RmGetProcessorPurpose(neighbour) & RmP_Mask;
  if (purpose eq RmP_IO)
   { if (mode ne RmL_Intelligent)
      { SetLinkMode(processor, link, RmL_NotConnected);
        SetLinkMode(processor, link, RmL_Pending);
      }
     goto done;
   }

  if (purpose eq RmP_Native)
   { if ((mode ne RmL_NotConnected) && (mode ne RmL_Dumb))
      SetLinkMode(processor, link, RmL_NotConnected);
     goto done;
   }

  if (purpose eq RmP_Router)
   {
     goto done;
   }

  if ((RmGetProcessorState(neighbour) & (RmS_Suspicious | RmS_Crashed | RmS_Dead))
	|| !CheckProcessor(neighbour))
   { MarkProcessor(neighbour, TRUE);
     SetLinkMode(processor, link, RmL_Dead);
     goto done;
   }

  SetLinkMode(processor, link,		RmL_NotConnected);
  SetLinkMode(neighbour, destlink,	RmL_NotConnected);
  SetLinkMode(processor, link,		RmL_Pending);
  SetLinkMode(neighbour, destlink,	RmL_Intelligent);

done:
  rm_link->Flags &= ~RmF_Suspicious;
}
/*}}}*/
/*{{{  CheckProcessor() */
/**
*** This routine performs a detailed check of a processor, but ignoring
*** the link connections.
**/
bool		CheckProcessor(RmProcessor processor)
{ Object	*proc;
  int		purpose = RmGetProcessorPurpose(processor) & RmP_Mask;

	/* It is not possible to check native processors, so assumed to	*/
	/* be OK.							*/
  if (purpose eq RmP_Native)
   return(TRUE);

	/* BLV - router support						*/
  if (purpose eq RmP_Router)
   return(TRUE);

	/* For I/O and Helios processors, attempt to locate the		*/
	/* processor.							*/
  proc = NetMapProcessorToObject(processor);
  if (proc eq Null(Object)) return(FALSE);

	/* For I/O processors, the only sensible test is to check for	*/
	/* the error logger.						*/
  if (purpose eq RmP_IO)
   { Object	*logger = Locate(proc, "logger");
     Close(proc);
     if (logger eq Null(Object))
      return(FALSE);
     else
      return(TRUE);
   }

	/* For Helios nodes, check the following:			*/
	/* 1) existence of procman /tasks				*/
	/* 2) existence of /loader					*/
	/* 3) existence of /tasks/ProcMan.0				*/
	/* 4) existence of /loader/Kernel				*/
  { Object	*procman	= Null(Object);
    Object	*loader		= Null(Object);
    Object	*procman_entry	= Null(Object);
    Object	*kernel_entry	= Null(Object);
    bool	result		= FALSE;
    if ((procman	= Locate(proc, "tasks"))	 eq Null(Object))
     goto done;
    if ((loader		= Locate(proc, "loader"))	 eq Null(Object))
     goto done;
    if ((procman_entry	= Locate(procman, "ProcMan.0")) eq Null(Object)) 
     goto done;
    if ((kernel_entry	= Locate(loader, "Kernel"))	 eq Null(Object))
     goto done;

    result = TRUE;
done:
    if (procman ne Null(Object)) Close(procman);
    if (loader ne Null(Object))  Close(loader);
    if (procman_entry ne Null(Object))	Close(procman_entry);
    if (kernel_entry ne Null(Object))	Close(kernel_entry);
    Close(proc);
    return(result);
  }
}
/*}}}*/
/*{{{  HandleReportProcessor() */
/**
*** This routine is called when a client such as a TFM reports a problem.
**/
void	HandleReportProcessor(NsConn connection, int job_id,
			RmRequest *request, RmReply *reply)
{ RmProcessor	processor = RmFindProcessor(Net, request->Uid);

  if (processor ne (RmProcessor) NULL)
   { Debug(dbg_Problem, ("a client has reported a problem on %P", processor));
     MarkProcessor(processor, FALSE);
   }
  reply->FnRc = RmE_Success;
  ReplyRmLib(connection, job_id, reply);
}
/*}}}*/

/*{{{  the Problem handler */
/**----------------------------------------------------------------------------
*** Problem Handler(). This is the big one. Whenever there is a problem
*** this routine gets invoked and has to sort it out, somehow...
***
*** The first step is to determine the extent of the problem. This requires
*** a breadth-first search of the network starting at the root processor.
*** Note that, courtesy of the single and double buffering code in the
*** kernel, the failure of a single node may affect links not directly
*** connected to that node. The Private2 field of each processor is used
*** to hold the current state. As each processor is reached its name table
*** is cleared, to bypass crashed links. 
***
*** Once the breadth-first search has finished the network can be scanned
*** to work out which processors, if any, should be rebooted. This involves
*** starting a bootstrap job and waiting for it to finish.
**/

/*{{{  StartProblemHandler() */
static void StartProblemHandler(void)
{ Wait(&ProblemHandlerLock);
  if (ProblemHandlerRunning)
   { Debug(dbg_Problem, ("leaving it to existing problem handler thread"));
   }
  else
   { if (Fork(Problem_Stack, &ProblemHandler, 0))
      { ProblemHandlerRunning	= TRUE;
        Debug(dbg_Problem, ("started up a problem handler thread"));
        Delay(OneSec / 10);	/* to let the problem handler start */
      }
     else
      { Debug(dbg_Problem, ("failed to start problem handler thread"));
      }
   }
  Signal(&ProblemHandlerLock);
}
/*}}}*/


#define	PH_NotFound	0
#define	PH_Found	1
#define PH_Suspicious	2
#define PH_Ignore	3

typedef struct	BreadthFirstNode {
	RmProcessor	Processor;
	Object		*Object;
} BreadthFirstNode;

static int	Problem_aux1(RmProcessor processor, ...);
static int	Problem_aux2(RmProcessor processor, ...);
static int	Problem_aux3(RmProcessor processor, ...);
static int	Problem_aux4(RmProcessor processor, ...);
static int	Problem_aux5(RmProcessor processor, ...);
static int	Problem_auxReclaim(RmProcessor processor, ...);
static void	Problem_search(void);

static void	ProblemHandler(void)
{ int		to_find;
  BootstrapJob	*reboot = Null(BootstrapJob);

  Debug(dbg_Problem, ("ProblemHandler thread running, waiting for lock"));
  MRSW_GetWrite();

  Debug(dbg_Problem, ("scanning network"));

  to_find  = RmApplyProcessors(Net, &Problem_aux1);
  Debug(dbg_Problem, ("%d processors should be running", to_find));
  Problem_search();

  (void) RmApplyProcessors(Net, &Problem_auxReclaim);
  to_find -= RmApplyProcessors(Net, &Problem_aux2);

  Debug(dbg_Problem, ("%d processors inaccessible", to_find));

	/* Did the search fail to find any of the processors...	*/ 
  if (to_find <= 0)
   goto done;

  reboot = NewBootstrapJob();
  if (reboot eq Null(BootstrapJob))
   goto done;

  (void) RmApplyProcessors(Net, &Problem_aux3, reboot);
  (void) RmApplyProcessors(Net, &Problem_aux4);

  Debug(dbg_Problem, ("starting reboot"));

  if (StartBootstrapJob(reboot))
   WaitBootstrapJob(reboot);
  else
   AbortBootstrapJob(reboot);

  (void ) RmApplyProcessors(Net, &Problem_aux5);

done:
  MRSW_FreeWrite();

	/* this delay lets the world settle down a bit		*/
  Delay(2 * OneSec);

  Wait(&ProblemHandlerLock);
  ProblemHandlerRunning = FALSE;
  Signal(&ProblemHandlerLock);

  KickSessionManager();

  Debug(dbg_Problem, ("ProblemHandler done"));
}

static	int Problem_aux1(RmProcessor processor, ...)
{ int	purpose = RmGetProcessorPurpose(processor) & RmP_Mask;
  int	state	= RmGetProcessorState(processor);

  if ((purpose eq RmP_Native) || (purpose eq RmP_Router) || (state & RmS_Dead))
   { RmSetProcessorPrivate2(processor, PH_Ignore);
     return(0);
   }

  RmSetProcessorPrivate2(processor, PH_NotFound);
  return(1);
}

	/* If a processor has not been found and the controlling device	*/
	/* driver has set the reclaim flag then all processors		*/
	/* controlled by this driver are set to NotFound. The Network	*/
	/* Server will then attempt to reboot all of them.		*/
static	int Problem_auxReclaim(RmProcessor processor, ...)
{ int			 result	= RmGetProcessorPrivate2(processor);
  ProcessorEntry	*proc_entry;
  int			 i, j;
  DriverEntry		*driver_entry;
  RmHardwareFacility	*hardware;
  RmProcessor		 other;
  int			 purpose;

  if (result ne PH_NotFound) return(0);
  unless(processor->Control & RmC_Reclaim) return(0);

  proc_entry = GetProcEntry(processor);
  for (i = 0; i < proc_entry->NumberDrivers; i++)
   { driver_entry	= &(proc_entry->DriverEntry[i]);
     hardware		= driver_entry->Hardware;
     if (driver_entry->Flags & DriverFlags_Reclaim)
      { for (j = 0; j < hardware->NumberProcessors; j++)
         { other = hardware->Processors[j];
 	   if (other eq RootProcessor)
	    continue;
	   purpose = RmGetProcessorPurpose(other) & RmP_Mask;
	   if ((purpose eq RmP_IO) || (purpose eq RmP_Router))
	    continue;
	   RmSetProcessorPrivate2(other, PH_NotFound);
	 }
	break;
      }
   }
  return(0);
}

static	int Problem_aux2(RmProcessor processor, ...)
{ int	result = RmGetProcessorPrivate2(processor);

  if (result eq PH_Found)
   { RmSetProcessorState(processor, RmS_Running);
     return(1);
   }
  elif (result eq PH_NotFound)
   RmSetProcessorState(processor, RmS_Crashed);
  return(0);
}

#if Joinnet_Supported
/*{{{  Problem_aux3_aux */
static void Problem_aux3_aux(RmNetwork subnet)
{ MRSW_GetRead();
  CopeWithExternalSubnet(subnet);
  MRSW_FreeRead();
}
/*}}}*/
#endif

static	int Problem_aux3(RmProcessor processor, ...)
{ va_list	args;
  BootstrapJob	*reboot;
  RmProcessor	neighbour;
  int		destlink;
  int		number_links, i;

  va_start(args, processor);
  reboot = va_arg(args, BootstrapJob *);
  va_end(args);

  if (RmGetProcessorPrivate2(processor) ne PH_NotFound) return(0);

#if Joinnet_Supported
	/* Check whether or not this processor is part of an	*/
	/* external network. If so do not attempt to reboot	*/
	/* it, instead lose the external network.		*/
  { RmNetwork	parent, current;
    for (parent = RmParentNetwork(processor), current = NULL;
	 parent ne Net;
	 parent = RmParentNetwork((RmProcessor) parent))
     current = parent;
    if (current ne NULL)
     { ExternalNetwork *ext_net = (ExternalNetwork *)RmGetNetworkPrivate(current);
       if (ext_net ne NULL)
        { if (ext_net->Reported eq FALSE)
	   { ext_net->Reported = TRUE;
             Fork(ProblemAux3_Stack, Problem_aux3_aux, 4, current);
	   }
	  RmSetProcessorPrivate2(processor, PH_Ignore);
          goto skip_bootstrap;
	}
     }
  }
#endif
	/* Rebooting I/O processors is difficult.			*/
	/* BLV - more work needed to set the link to pending.		*/
  if ((RmGetProcessorPurpose(processor) & RmP_Mask) eq RmP_IO)
   { report("I/O processor %P appears to have failed", processor);
     goto skip_bootstrap;
   }

  report("processor %P appears to have crashed, attempting a reboot", processor);

  number_links = RmCountLinks(processor);
  for (i = 0; i < number_links; i++)
   { neighbour = RmFollowLink(processor, i, &destlink);
     if ((neighbour eq RmM_NoProcessor) || (neighbour eq RmM_ExternalProcessor))
      continue;
     if ((RmGetProcessorPurpose(neighbour) & RmP_Mask) ne RmP_Helios)
      continue;
     unless(RmGetProcessorState(neighbour) & RmS_Running)
      continue;
     (void) SetLinkMode(neighbour, destlink, RmL_NotConnected);
   }

  AddProcessorToBootstrapJob(reboot, processor);

skip_bootstrap:
  RemNetworkAgent(processor);
  return(0);
}

	/* To cope with the case of a TFM processor crashing, ownership	*/
	/* problems have to be resolved after all affected processors	*/
	/* have been marked as crashed.					*/
static int Problem_aux4(RmProcessor processor, ...)
{
	/* Only deal with processors that the search could not find	*/
  if (RmGetProcessorPrivate2(processor) ne PH_NotFound) return(0);

  if (RmGetProcessorState(processor) eq RmS_Crashed)
   { RemConnection(processor, RmR_Crashed);
     processor->ObjNode.Account = RmO_Graveyard;
   }
  return(0);
}

static int Problem_aux5(RmProcessor processor, ...)
{ 
  if (RmGetProcessorPrivate2(processor) ne PH_NotFound) return(0);

  if (RmGetProcessorState(processor) eq RmS_Crashed)
   { report("failed to reboot processor %P", processor);
     RmSetProcessorState(processor, RmS_Dead);
   }
  else
   { if ((RmGetProcessorPurpose(processor) & RmP_System) eq 0)
      processor->ObjNode.Account = RmO_FreePool;
     else
      processor->ObjNode.Account = RmO_System;
   }
  return(0);
}

static	void Problem_search(void)
{ BreadthFirstNode	*search_table		= Null(BreadthFirstNode);
  int			search_tail, search_head;

  search_table = Malloc(NumberProcessors * sizeof(BreadthFirstNode));
  if (search_table eq Null(BreadthFirstNode))
   goto done;

  search_head = 0;
  search_tail = 0;

  search_table[search_tail].Processor	= RootProcessor;
  search_table[search_tail].Object	= NetMapProcessorToObject(RootProcessor);
  if (search_table[search_tail].Object eq Null(Object))
   goto done;
  search_tail++;
  RmSetProcessorPrivate2(RootProcessor, PH_Found);

  for ( ;search_head < search_tail; search_head++)
   { RmProcessor	processor = search_table[search_head].Processor;
     RmProcessor	neighbour;
     int		destlink;
     int		number_links;
     int		i;
     char		name_buf[8];
     Object		*neighbour_obj;

     strcpy(name_buf, "link.x");
     ClearNames(processor);

     number_links = RmCountLinks(search_table[search_head].Processor);
     for (i = 0; i < number_links; i++)
      { neighbour = RmFollowLink(processor, i, &destlink);
        if ((neighbour eq RmM_NoProcessor) || (neighbour eq RmM_ExternalProcessor))
         continue;

		/* Ignore processors we have already found or that	*/
		/* should be ignored.					*/
        if (RmGetProcessorPrivate2(neighbour) ne PH_NotFound) continue;

	name_buf[5]	= i + '0';
	neighbour_obj	= Locate(search_table[search_head].Object, name_buf);
	if (neighbour_obj eq Null(Object))
	 { Debug(dbg_Problem, ("failed to access processor %P via link %d of %P",\
			neighbour, i, processor));
           continue;
	 }
	unless(CheckProcessor(neighbour))
	 { Close(neighbour_obj); 
           Debug(dbg_Problem, ("processor %P failed its checkup", neighbour));
           continue;
         }
	RmSetProcessorPrivate2(neighbour, PH_Found);
	search_table[search_tail].Processor	= neighbour;
	search_table[search_tail].Object	= neighbour_obj;
	search_tail++;
      }
     Close(search_table[search_head].Object);
   } 

done:
  if (search_table ne Null(BreadthFirstNode)) Free(search_table);
}
/*}}}*/

/*{{{  link changes */
/**-----------------------------------------------------------------------------
*** Link Change Handling.
***
*** It is possible to request a processor manager to report state
*** changes on any or all of its links. This is one of the main ways
*** of detecting crashed processors: processor A attempts to send a
*** message to or through processor B, which has crashed; the link I/O
*** fails, so processor A marks the connecting link as crashed and
*** reports this to the network server.
***
*** The data structure is as follows:
*** IOCCommon	:	string /ns, no capability, 5 words of control vector
*** index into data vector for reporting processor name
*** word for link number
*** word for link mode
*** word for link state
**/

#if Joinnet_Supported
/*{{{  ForwardLinkChange() */
/**
*** Forward a message to the right network server. This means finding the
*** network server. First, the reported processor is located and put into a
*** table. Then the table is searched: for every processor, check for a
*** network server, and then check all four links. This is much the same
*** as the work done by findns, findsm, etc.
**/
#if 1
static void ForwardLinkChange(char *name, ServInfo *info)
{ name = name; info = info;
}
#else

	/* BLV - in Helios 1.3 this ForwardLinkChange() fails.	*/
	/* The search of the network returns invalid names.	*/
	/* With the fault-tolerance features of 1.3 this should	*/
	/* not be too serious a problem.			*/
static	void ForwardLinkChangeAux(Object *, ServInfo *);

static	void ForwardLinkChange(char *name, ServInfo *info)
{ Object	*first_processor = Locate(Null(Object), name);
  Object	**procvec;
  int		max_processors;
  int		next_proc = 0;
  int		cur_proc;
  char		linkbuf[10];
  char		*temp;
  int		len = 2;
  int		number_links;

  Debug(dbg_Links, ("link change has to be forwarded"));

  if (first_processor eq Null(Object)) return;

	/* Only follow processors that have the right base name */
	/* E.g name = /ClusterA/xx, only compare 10 characters  */
  for (temp = &(name[1]); *temp ne '/'; temp++) len++;
  
  procvec = (Object **) Malloc(16 * sizeof(Object *));
  if (procvec eq Null(Object *))
   { Close(first_processor); return; }
   
  max_processors = 16;
  procvec[next_proc++] = first_processor;
  strcpy(linkbuf, "link.");

  for (cur_proc = 0; cur_proc < next_proc; cur_proc++)
   { Object	*current = procvec[cur_proc];
     Object	*ns = Locate(current, "ns");
     Object	*next;
     int	i;   

     if (ns ne Null(Object))
      { ForwardLinkChangeAux(ns, info); 
        Close(ns);
        goto done; 
      }

     number_links = Util_CountLinks(current);
     for (i = 0; i < number_links; i++)
      { 
        linkbuf[5] = '\0';
        if (i eq 0)
         strcat(linkbuf, "0");
        else
         addint(linkbuf, i);

        next = Locate(current, linkbuf);

        if (next ne Null(Object))
         { int j;
		/* Check we are still in the right network */
	   if (strncmp(next->Name, name, len)) 
	    goto skip;
	    
	   	/* Check that this processor has not been found already */
           for (j = 0; j < next_proc; j++)
            if (!strcmp(procvec[j]->Name, next->Name))
             goto skip;
             
           if (next_proc eq max_processors)
            { Object **temp = Malloc(2 * max_processors * sizeof(Object *));
              if (temp eq Null(Object *)) goto done;
              memcpy(temp, procvec, max_processors * sizeof(Object *));
              Free(procvec);
              procvec = temp;
              max_processors *= 2;
            }
           procvec[next_proc++] = next;
         }
skip:
	continue;
      }
   }

done:
  for (cur_proc = 0; cur_proc < next_proc; cur_proc++)
   Close(procvec[cur_proc]);
  Free(procvec);
}

/**
*** Given a network server and a link change message that has been wrongly
*** addressed, send the message again. See nucleus/procman.c for more details.
**/
static	void ForwardLinkChangeAux(Object *ns, ServInfo *info)
{ MsgBuf	*m = New(MsgBuf);
  MCB		*old = info->m;
  char		*name;
  int		link;
  int		state;
  int		mode;
  word		controlvec[10];

  if (m eq Null(MsgBuf)) return;

  m->mcb.Control	= controlvec;
  m->mcb.Data		= m->data;
  
  name  = &(old->Data[(old->Control[5])]);
  link  = old->Control[6];
  mode  = old->Control[7];
  state = old->Control[8];
  
  InitMCB(&m->mcb, MsgHdr_Flags_preserve, NullPort, NullPort,
  		FC_GSP | SS_NetServ | FG_NetStatus);
  MarshalWord(&m->mcb, -1);
  MarshalString(&m->mcb, ns->Name);
  MarshalWord(&m->mcb, 1);
  MarshalWord(&m->mcb, -1);
  MarshalWord(&m->mcb, -1);

  MarshalString(&m->mcb, name);
  MarshalWord(&m->mcb, link);
  MarshalWord(&m->mcb, mode);
  MarshalWord(&m->mcb, state);

  Debug(dbg_Links, ("sending link change on to %s", ns->Name));
    
  SendIOC(&m->mcb);  
  Free(m);
}
#endif
/*}}}*/
#endif

void	HandleLinkChange(ServInfo *servinfo)
{ char		*name;
  RmProcessor	reporter;
  int		link;
  int		state;
  int		mode;
  MCB		*m = servinfo->m;
  RmProcessor	neighbour;
  int		destlink;

  MRSW_GetRead();
  
  name = &(m->Data[(m->Control[5])]);  

  link  = m->Control[6];
  mode  = m->Control[7];
  state = m->Control[8];

  Debug(dbg_Links, ("link change reported on processor %s, link %d, new mode %d, state %d",\
	name, link, mode, state));

  reporter = RmLookupProcessor(Net, name);
#if Joinnet_Supported
  if (reporter eq (RmProcessor) NULL)  
   { ForwardLinkChange(name, servinfo); goto done; }
#else
  goto done;
#endif
	/* Unless there is a real processor at the other end of this	*/
	/* link, ignore the change message.				*/
  neighbour = RmFollowLink(reporter, link, &destlink);
  if ((neighbour eq RmM_NoProcessor) ||
      (neighbour eq RmM_ExternalProcessor))
   goto done;

	/* For I/O Processors all changes are important.		*/   
  if ((RmGetProcessorPurpose(neighbour) & RmP_Mask) eq RmP_IO)
   { HandleIOChange(neighbour, mode, state);  goto done; }

	/* If the link report says that the link is now running, ignore	*/
	/* the message. It is the incoming network's responsibility to	*/
	/* request a service.						*/
  if ((state eq Link_State_Running) && (mode eq Link_Mode_Intelligent))
   goto done;

	/* if the link is set back to pending mode, ignore it	*/
  if ((state eq Link_State_Dead) && (mode eq Link_Mode_Intelligent))
   goto done;
      
	/* Check whether the link is internal or external to the network */
  { ProcessorEntry 	*proc_entry;
    RmLink		*default_link;
    
    proc_entry = GetProcEntry(reporter);
    default_link = &(proc_entry->StandardLinks[link]);
    if (default_link->Target ne RmL_ExtUid) 
     { MarkLink(reporter, link); goto done; }
#if Joinnet_Supported
    else
	/* OK, an external network of some sort has been disconnected.	*/
     LostExternalNetwork(reporter, link);
#endif
  }

done:
  Debug(dbg_Links, ("link change on processor %s, link %d, handled",\
		name, link));
  MRSW_FreeRead();
}


/*}}}*/
/*{{{  I/O Processor link changes */
/**
*** Changes to an I/O processor. There are three cases to consider
*** 1) an outside I/O processor has connected in. In that case
***    a suitable window should be created and registered.
*** 2) an outside I/O processor has been disconnected. In that case
***    any sessions using that I/O Server must be aborted.
*** 3) the I/O processor is part of an external network. In that case
***    the whole external network must be disconnected.
**/

/*{{{  I/O processor connecting in */
/**
*** If an I/O Processor connects it, first it must be updated. This takes
*** care of getting the name right, sorting out capabilities, etc. Then
*** a window server is located inside the I/O Processor, and a console
*** window is created. This is registered with the Session Manager
**/
static	void	HandleIOConnecting(RmProcessor IO)
{ Object	*real_processor = Null(Object);
  Object	*window_server = Null(Object);
  Object	*console_window = Null(Object);
  Stream	*window_stream = Null(Stream);
  word		error;

  Debug(dbg_Problem, ("I/O Processor %P has connected in", IO));
  
  UpdateIOProcessor(IO, FALSE);
  
  real_processor = NetMapProcessorToObject(IO);
  if (real_processor eq Null(Object))
   { report("warning, failed to start session in I/O Processor %P", IO);
     return;
   }
  window_server = Locate(real_processor, "window");
  if (window_server eq Null(Object))
   { report("warning, I/O Processor %P is not running a window server", IO);
     goto done;
   }
  console_window = Create(window_server, "Login", Type_Stream, 0, Null(BYTE));
  if (console_window eq Null(Object))
   { report("warning, failed to create login window in I/O Processor %P", IO);
     goto done;
   }
  window_stream = Open(console_window, Null(char), O_ReadWrite);
  if (window_stream eq Null(Stream))
   { report("warning, failed to open login window in I/O Processor %P", IO);
     goto done;
   }

  unless(RmRegisterWindow(window_server, window_stream, Null(char), &error))
   { report("warning, failed to register login window in I/O Processor %P, fault %x",
   		IO, error);
     goto done;
   }
  RmSetProcessorState(IO, RmS_Running);
  IO->ObjNode.Dates.Modified	=
  IO->ObjNode.Dates.Access	= GetDate();
    
done:
  if (window_stream  ne Null(Stream)) Close(window_stream);
  if (console_window ne Null(Object)) Close(console_window);
  if (window_server  ne Null(Object)) Close(window_server);
  if (real_processor ne Null(Object)) Close(real_processor);
}
/*}}}*/
/*{{{  I/O processor disconnecting */
/**
*** If an I/O Processor disconnects from the network, because the I/O Server
*** exits, the machine is switched off, or something similar, then
*** it is necessary to do some tidying up. In particular, the
*** Session Manager must be informed that this processor is no longer
*** active so that any sessions running inside its /window server are
*** aborted. Next the link on the neighbouring processor must be put
*** back into a sensible state.
**/
static	void	HandleIODisconnecting(RmProcessor IO)
{ RmProcessor	neighbour;
  int		destlink;

  Debug(dbg_Problem, ("I/O Processor %P has disconnected", IO));
  
  neighbour = RmFollowLink(IO, 0, &destlink);

  RmSetProcessorState(IO, RmS_Dead);
  IO->ObjNode.Dates.Modified	=
  IO->ObjNode.Dates.Access	= GetDate();

  Delay(5 * OneSec);	/* to let the world settle down */
  if ((neighbour ne RmM_NoProcessor) && (neighbour ne RmM_ExternalProcessor))
   { SetLinkMode(neighbour, destlink, RmL_NotConnected);
     SetLinkMode(neighbour, destlink, RmL_Pending);  
   }   
  ClearNames(RootProcessor);

  KickSessionManager();
}
/*}}}*/

static	void HandleIOChange(RmProcessor IO, int mode, int state)
{
#if Joinnet_Supported
	/* For case 3, a flag will be set in the ObjNode sub-structure */
  if (IO->ObjNode.Flags & NsFlags_NotInResourceMap)
   { if ((mode eq Link_Mode_Intelligent) && (state eq Link_State_Running))
      return;
     HandleIOExternal(IO);
     return;
   }	
#endif

  if ((mode eq Link_Mode_Intelligent) && (state eq Link_State_Running))
   HandleIOConnecting(IO);
  elif ((mode eq Link_Mode_Intelligent) && (state eq Link_State_Dead))
   return;
  else
   HandleIODisconnecting(IO);
}
/*}}}*/
/*{{{  external networks (joinnet etc) */
#if Joinnet_Supported
/**
*** Now for external networks. The most likely time for a lost external
*** network to be noticed is when the I/O Server is shut down. The
*** remaining processors are still accessible through the links, but
*** must not be used any more.
***
*** There are four routines to cope with external networks.
*** 1) HandleAcceptNetwork(). This is an RmLib-style request sent by
***    another Network Server which has had enough of life and wants to
***    hand over responsibility for all its processors. Details of the
***    network are read in, as usual. 
***
*** 2) HandleJoinNetwork(). This is the other side of AcceptNetwork. It
***    is generated by the joinnet command.
***
*** 3) HandleIOExternal(). An I/O Processor in an external network has
***    disappeared, i.e. the I/O Server has stopped running. It is
***    necessary to get rid of the whole external network.
***
*** 4) LostExternalNetwork(). This is when a whole external network has
***    disappeared, typically because its power has been switched off.
**/

/*{{{  HandleAcceptNetwork() */
static	int	HandleAcceptAux(RmProcessor processor, ...);
static	int	HandleAcceptAux2(RmProcessor processor, ...);
static	int	HandleAcceptAux3(RmProcessor processor, ...);

void	HandleAcceptNetwork(NsConn connection, int job_id, 
		RmRequest *request, RmReply *reply)
{ Stream		*pipe = connection->Pipe_ctos;
  RmNetwork		subnet;
  RmProcessor		connector;
  RmProcessor		neighbour;
  int			link;
  char			*procname; 
  int			its_uid;
  int			destlink;
  ExternalNetwork	*ext_net;

  MRSW_FreeRead();
  MRSW_GetWrite();

  Debug(dbg_Problem, ("accepting external network"));

  procname	= (char *) request->VariableData;
  link		= request->Arg1;
  its_uid	= request->Arg2;
  destlink	= request->Arg3;
  
  connector = RmLookupProcessor(Net, procname);
  if (connector eq RmM_NoProcessor)
   { report("internal error 2 receiving network");
     goto fail;
   }

  subnet		= request->Network;
  request->Network	= NULL;		

	/* prepare to make the link connection.				*/
  neighbour = RmFindProcessor(subnet, its_uid);
  if (neighbour eq RmM_NoProcessor)
   { report("internal error 4 accepting network");
     RmFreeNetwork(subnet);
     goto fail;
   }

	/* incorporate the external subnet into the main network	*/
  if (RmAddtailProcessor(Net, (RmProcessor) subnet) eq (RmProcessor) NULL)
   { report("internal error 5 accepting network");
     RmFreeNetwork(subnet);
     goto fail;
   }
  NumberProcessors = RmCountProcessors(Net);
  
	/* Read in ProcessorEntry structures for every processor	*/
	/* N.B. The hardware facilities will have been thrown away	*/
	/* N.B. This must be done after the subnet has been added.	*/
	/* BLV - remember the hardware					*/
  if (RmApplyProcessors(subnet, &HandleAcceptAux, pipe) ne 0)
   { report("internal error 6 accepting network");
     RmRemoveProcessor((RmProcessor) subnet);
     RmFreeNetwork(subnet);
     goto fail;
   }

	/* make the connection between the main and external network	*/   
  RmBreakLink(connector, link);
  RmBreakLink(neighbour, destlink);
  RmMakeLink(connector, link, neighbour, destlink);
  { RmLink *link_entry = RmFindLink(connector, link);
    link_entry->Flags |= RmF_Suspicious;
  }
  
	/* fill in an ExternalNetwork structure to allow future recovery */
  ext_net = New(ExternalNetwork);
  if (ext_net eq Null(ExternalNetwork)) goto done;
  ext_net->Connector	= connector;
  ext_net->Link		= link;
  ext_net->Reported	= FALSE;
  RmSetNetworkPrivate(subnet, (int) ext_net);
 
	/* Update the processors.	*/  
  (void) RmApplyProcessors(subnet, &HandleAcceptAux2);
  (void) RmApplyProcessors(subnet, &HandleAcceptAux3);

	/* and send back a reply. The remote Network Server will now exit */
done:
  reply->FnRc	= RmE_Success;
  ReplyRmLib(connection, job_id, reply);
  MRSW_SwitchRead();
  return;

fail:
  AbortConnection(connection);
  MRSW_SwitchRead();
}

static	int HandleAcceptAux(RmProcessor processor, ...)
{ va_list		args;
  Stream		*pipe;
  ProcessorEntry	*proc_entry;
  int			size;
  int			hardware_count;
      
  va_start(args, processor);
  pipe = va_arg(args, Stream *);
  va_end(args);

  if (FullRead(pipe, (BYTE *) &size, sizeof(int), -1) ne sizeof(int))
   return(1);

  proc_entry = (ProcessorEntry *) Malloc(size);
  if (proc_entry eq Null(ProcessorEntry))
   return(1);
  if (FullRead(pipe, (BYTE *) proc_entry, size, -1) < size)
   return(1);

  hardware_count		= proc_entry->NumberDrivers;
  proc_entry->NumberDrivers	= 0;
  proc_entry->StandardLinks	= (RmLink *) 
   ( (BYTE *) proc_entry + sizeof(ProcessorEntry) + (hardware_count * sizeof(DriverEntry)));
  proc_entry->Netagent		= Null(Stream);
  proc_entry->NetagentCount	= 0;
  proc_entry->NetagentDate	= 0;
  InitSemaphore(&(proc_entry->NetagentLock), 1);
  proc_entry->Processor		= processor;
  RmSetProcessorPrivate(processor, (int) proc_entry);
  return(0);  		
}   

/**
*** Update the names for all the processors in the external subnet.
*** Also update the PUID entry in the object's attributes.
**/
static	int	HandleAcceptAux2(RmProcessor processor, ...)
{ int	purpose;
  char  full_name[IOCDataMax];
  
  purpose = RmGetProcessorPurpose(processor) & RmP_Mask;
  if (purpose eq RmP_IO)
   UpdateIOProcessor(processor, TRUE);
  elif (purpose eq RmP_Helios)
   UpdateProcessor(processor, TRUE);

  { char *tmp = RmGetObjectAttribute((RmObject) processor, "PUID", TRUE);
    if (tmp ne Null(char))
     RmRemoveObjectAttribute((RmObject) processor, &(tmp[-5]), TRUE);
  }

  strcpy(full_name, "PUID=");
  BuildName(&(full_name[5]), processor);
  RmAddObjectAttribute((RmObject) processor, full_name, TRUE);
  
  return(0);
}

/**
*** Clear the names for all the processors in the external subnet
**/
static	int	HandleAcceptAux3(RmProcessor processor, ...)
{ int	purpose;

  purpose = RmGetProcessorPurpose(processor) & RmP_Mask;
  if (purpose eq RmP_Helios)
   ClearNames(processor);
  return(0);
}
/*}}}*/
/*{{{  HandleJoinnet() */
/**
*** This routine is called when the joinnet program is running. It
*** performs the following:
*** 1) get various bits of information about the network it is supposed
***    to join.
*** 2) open a connection to the remote network server
*** 3) send the information needed by HandleAcceptNetwork()
*** 4) terminate the Network Server
**/

static	int	HandleJoinnetAux(RmProcessor processor, ...);
static	int	joinnet_filter(RmProcessor, RmProcessor);
static	int	joinnet_netfilter(RmNetwork, RmNetwork);

void	HandleJoinNetwork(NsConn connection, int job_id, 
		RmRequest *request, RmReply *reply)
{ char			*sockserv_name;
  char			*connector_name;
  char			*neighbour_name;
  int			connector_link;
  int			neighbour_link;
  int			neighbour_uid;
  int			rc = Err_Null;
  RmServer		server;
  RmJob			job;
  RmFilterStruct	filter;
  RmRequest		out_request;
  RmReply		out_reply;

  MRSW_FreeRead();
  MRSW_GetWrite();

  sockserv_name		= (char *) request->VariableData;
  connector_name	= sockserv_name + strlen(sockserv_name) + 1;
  neighbour_name	= connector_name + strlen(connector_name) + 1;  
  connector_link	= request->Arg1;
  neighbour_link	= request->Arg2;

  { RmProcessor	neighbour = RmLookupProcessor(Net, neighbour_name);
    if (neighbour eq RmM_NoProcessor)
     { rc = RmE_BadProcessor; goto done; }
    neighbour_uid = neighbour->Uid;
  }

  rc = RmOpenServer(sockserv_name, ".NS_ctos", Null(Capability), &server);
  if (rc ne RmE_Success) goto done;

  rc = RmNewJob(&server, &job);
  if (rc ne RmE_Success) goto done;

  Clear(out_request); Clear(out_reply);
  out_request.FnRc		= RmC_AcceptNetwork;
  out_request.Arg1		= connector_link;
  out_request.Arg2		= neighbour_uid;
  out_request.Arg3		= neighbour_link;
  out_request.VariableSize	= strlen(connector_name) + 1;
  out_request.VariableData	= connector_name;
  out_request.Filter		= &filter;
  filter.Network		= &joinnet_netfilter;
  filter.Processor		= &joinnet_filter;
  filter.Task			= NULL;
  filter.Taskforce		= NULL;
  filter.SendHardware		= TRUE;
  out_request.Network		= Net;

  rc = RmTx(job, &out_request);
  if (rc ne RmE_Success) goto done;
  
  rc = RmSearchProcessors(Net, &HandleJoinnetAux, job->Server->Pipe_ctos);
  if (rc ne RmE_Success) 
   { RmUnlockWrite(job); goto done; }

  rc = RmRx(job, &out_reply);
  if (rc ne RmE_Success) goto done;

  reply->FnRc	= rc;
  ReplyRmLib(connection, job_id, reply);

  if (rc eq RmE_Success)  
   TerminateNetworkServer();
  else
   { MRSW_SwitchRead(); return; }
  
done:  
  AbortConnection(connection);
  MRSW_SwitchRead();
  report("internal error in HandleJoinnet");
}

static	int	HandleJoinnetAux(RmProcessor processor, ...)
{ va_list		args;
  Stream		*pipe;
  ProcessorEntry	*proc_entry;
  int			size;
    
  va_start(args, processor);
  pipe = va_arg(args, Stream *);
  va_end(args);
  
  proc_entry = GetProcEntry(processor);
  
  size = sizeof(ProcessorEntry) + 
         (proc_entry->NumberDrivers * sizeof(DriverEntry)) +
         (RmCountLinks(processor) * sizeof(RmLink));
  if (Write(pipe, (BYTE *) &size, sizeof(int), -1) ne sizeof(int))
   return(RmE_CommsBreakdown);
  if (Write(pipe, (BYTE *) proc_entry, size, -1) ne size)
   return(RmE_CommsBreakdown);

  return(0);
}
         
static	int joinnet_filter(RmProcessor real, RmProcessor copy)
{ real = real;
  copy->ObjNode.Flags |= NsFlags_NotInResourceMap;
  return(RmE_Success);
}

static	int joinnet_netfilter(RmNetwork real, RmNetwork copy)
{ real = real;
  if (real eq Net)
   strcpy(copy->DirNode.Name, NetworkName);
  copy->DirNode.Flags |= NsFlags_NotInResourceMap;
  return(RmE_Success);
}
/*}}}*/
/*{{{  External I/O processor has been lost */
/**
*** An I/O Processor inside an external network has been lost.
*** It is necessary to work out the whole external subnet, and disconnect
*** it. This is fairly easy, since external subnets are always added at
*** the top level.
**/

static	void HandleIOExternal(RmProcessor IO)
{ RmNetwork	current	= RmParentNetwork(IO);
  RmNetwork	parent;

  for (parent = RmParentNetwork((RmProcessor) current);
       parent ne Net;
       parent = RmParentNetwork((RmProcessor) current))
   current = parent;
   
  CopeWithExternalSubnet(current);
}
/*}}}*/
/*{{{  Processor in an external network has been lost */
static	void	LostExternalNetwork(RmProcessor reporter, int link)
{ RmProcessor	neighbour;
  int		destlink;
  RmNetwork	current;
  RmNetwork	parent;

  neighbour = RmFollowLink(reporter, link, &destlink);
  if (neighbour eq RmM_NoProcessor) return;
  current = RmParentNetwork(neighbour);
  
  for (parent = RmParentNetwork((RmProcessor) current);
       parent ne Net;
       parent = RmParentNetwork((RmProcessor) current))
   current = parent;

  CopeWithExternalSubnet(current);  
}
/*}}}*/
/*{{{  CopeWithExternalSubnet() - this does the necessary cleaning up */
/**
*** Cope with an external network that has gone away.
*** 1) for every processor in the external network, report it to the
***    Session Manager
*** 2) find the connecting link and break it.
*** 3) remove the external network, and free it.
*** 4) reset the connecting link to pending
**/
static	int ExternalCopeAux(RmProcessor, ...);

static	void CopeWithExternalSubnet(RmNetwork subnet)
{ ExternalNetwork	*ext_net;
  RmProcessor		connector	= RmM_NoProcessor;
  ProcessorEntry	*proc_entry;
  int			conn_link;

  MRSW_FreeRead();
  MRSW_GetWrite();

  if (subnet->DirNode.Parent ne &(Net->DirNode))
   { MRSW_SwitchRead(); return; }

  Debug(dbg_Problem, ("external network %N has disconnected", subnet));
  
  RmApplyProcessors(subnet, &ExternalCopeAux);

  ext_net = (ExternalNetwork *) RmGetNetworkPrivate(subnet);
  if (ext_net ne Null(ExternalNetwork))
  { connector  = ext_net->Connector;
    conn_link  = ext_net->Link;
    proc_entry = GetProcEntry(connector);
    RmBreakLink(connector, conn_link);
    RmMakeLink(connector, conn_link, RmM_ExternalProcessor,
      proc_entry->StandardLinks[conn_link].Destination);
  }

  if (RmRemoveProcessor((RmProcessor) subnet) ne (RmProcessor) subnet)
   { report("error, failed to remove external subnet");
     subnet = NULL;
   }

  NumberProcessors = RmCountProcessors(Net);   

  if (connector ne RmM_NoProcessor)
   { SetLinkMode(connector, conn_link, RmL_NotConnected);
     SetLinkMode(connector, conn_link, RmL_Pending);
   }
  ClearNames(RootProcessor);

  MRSW_SwitchRead();

  KickSessionManager();

	/* BLV - do not free the subnet for a while just in case other	*/
	/* threads have got pointers to it.				*/
  MRSW_FreeRead();
  Delay(120 * OneSec);
  MRSW_GetRead();
  if (subnet ne NULL)
   RmFreeNetwork(subnet);
}

static int ExternalCopeAux(RmProcessor processor, ...)
{ ProcessorEntry	*proc_entry;

	/* Remove the processor from any other lists it might be on	*/
  RemNetworkAgent(processor);
  RemConnection(processor, RmR_Crashed);

  proc_entry = GetProcEntry(processor);
  Free(proc_entry);
  RmSetProcessorPrivate(processor, 0);
  return(0);
}
/*}}}*/
#endif
/*}}}*/

