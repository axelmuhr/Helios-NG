/*------------------------------------------------------------------------
--                                                                      --
--           H E L I O S   N E T W O R K I N G   S O F T W A R E	--
--           ---------------------------------------------------	--
--                                                                      --
--             Copyright (C) 1991, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- netnativ.c								--
--                                                                      --
--	The module of the Network Server to cope with native networks	--
--	etc.								--
--                                                                      --
--	Author:  BLV 21.1.91						--
--                                                                      --
------------------------------------------------------------------------*/
/*$Header: /hsrc/network/RCS/netnativ.c,v 1.6 1993/08/11 10:35:36 bart Exp $*/

/*{{{  headers and compile-time options */
#define	__Netnative_Module
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
#include "exports.h"
#include "private.h"
#include "netutils.h"
#include "rmlib.h"
#include "netaux.h"
#include "session.h"
/*}}}*/
/*{{{  statics and forward declarations */
/**
*** Currently no initialisation is needed
**/
void	InitNative(void)
{
}
/*}}}*/

#if Native_Supported
/*{{{  HandleNative() - SetNative, Reset and Reboot */
/**
*** HandleNative(). This routine is called for SetNative, Reset, and
*** Reboot requests. The basic data passed to and from is the same,
*** but the work done in the middle tends to vary somewhat.
**/
/*{{{  HandleSetNative() */
/**
*** SetNative(). This is where life starts to be fun. Fortunately this
*** routine can get exclusive access to the network.
*** 1) Ignore any processors that are not going to change, for example
***    processors that are already native.
*** 2) It is necessary to check that setting these processors to native
***    mode would not break the connectivity of the network.
***    a) count the processors that can be reached at present
***    b) mark up the ones that are about to become native
***    c) repeat step b
***    d) undo step b
***    e) if the difference is more than the number of processors marked
***       then the network connectivity would fail. Hence illegal
*** 3) work out a strategy for terminating the processors.
***    a) for every processor with a Helios neighbour not affected by the
***       SetNative, move it to the back of the list
***    b) until list is empty or no more changes can be made, for every
***       processor marked in step a, check the neighbours and see if
***       these are to be turned native in this code.
*** 4) from the start of the table, terminate the processors
*** 5) disconnect any links to Helios processors that might be left
*** 6) try to reset the processors, but failure is not important since it is
***    not part of the specification
**/
static	int 	CountConnectedProcessors(void);
static	void	SortProcessors(int count, RmProcessor *);

static	int HandleSetNative(int count, ProcessorDetails *details)
{ int	i, j, rc;
  RmProcessor	*table = Malloc(count * sizeof(RmProcessor));

  Debug(dbg_Native, ("request to switch %d processors to native mode", count));
    
  if (table eq Null(RmProcessor)) return(RmE_ServerMemory);

  for (i = 0, j = 0; i < count; i++)
   if (details[i].Uid ne RmL_NoUid)
    { int	purpose;
      table[j] = RmFindProcessor(Net, details[i].Uid);
      purpose = RmGetProcessorPurpose(table[j]) & RmP_Mask;
      if (purpose eq RmP_Native) continue;
      if (purpose ne RmP_Helios)
       { rc = RmE_BadProcessor; goto done; }
      else
       j++;
    }
    
  if (j eq 0)	/* There is nothing to do, typical */
   { rc = RmE_Success; goto done; }
  
  count = j;	/* The job has been simplified to ``count'' real processors */

  Debug(dbg_Native, ("%d processors are affected", count));
  
  { int	old_count = CountConnectedProcessors();
    int new_count;

    if (old_count eq -1)
     { rc = RmE_ServerMemory; goto done; }
    for (i = 0; i < count; i++)
     RmSetProcessorPurpose(table[i], RmP_Native);
    new_count = CountConnectedProcessors();
    if (new_count eq -1)
     { rc = RmE_ServerMemory; goto done; }
    for (i = 0; i < count; i++)
     RmSetProcessorPurpose(table[i], RmP_Helios);

    if (new_count < (old_count - count))
     { Debug(dbg_Native, ("illegal, network connectivity would be broken"));
       rc = RmE_WrongNetwork; 
       goto done; 
     }
  }    

  SortProcessors(count, table);

  for (i = 0; i < count; i++)
   { 
     Debug(dbg_Native, ("terminating processor %P", table[i]));
     TerminateProcessor(table[i]);
     RmSetProcessorPurpose(table[i], RmP_Native);
   }

	/* Terminate does not disable the links, it leaves them pending */
  for (i = 0; i < count; i++)
   { int 		number_links = RmCountLinks(table[i]);
     int 		destlink, purpose;
     RmProcessor	neighbour;
     
     for (j = 0; j < number_links; j++)
      { neighbour = RmFollowLink(table[i], j, &destlink);
        if ((neighbour eq RmM_NoProcessor) ||
            (neighbour eq RmM_ExternalProcessor))
         continue;
        unless(RmGetProcessorState(neighbour) & RmS_Running)
         continue;
        purpose = RmGetProcessorPurpose(neighbour) & RmP_Mask;
        if (purpose eq RmP_Helios)
         SetLinkMode(neighbour, destlink, RmL_NotConnected);
      }
   }

  ResetProcessors(count, table);
  rc = RmE_Success;
  
done:
  if (table ne Null(RmProcessor)) Free(table);
  return(rc);
}

static	int	CountConnectedProcessors(void)
{ RmProcessor	*search_table = Malloc(NumberProcessors * sizeof(RmProcessor));
  int		current_index = 0;
  int		max_index     = 0;

  if (search_table eq Null(RmProcessor)) return(-1);
  search_table[max_index++] = RootProcessor;
  for ( ; current_index < max_index; current_index++)
   { RmProcessor	current = search_table[current_index];
     int		number_links, i, j, state, destlink, purpose;
     RmProcessor	neighbour;
     
     number_links = RmCountLinks(current);     
     for (i = 0; i < number_links; i++)
      { neighbour = RmFollowLink(current, i, &destlink);
        if ((neighbour eq RmM_NoProcessor) ||
            (neighbour eq RmM_ExternalProcessor))
         continue;
        
        state = RmGetProcessorState(neighbour);
        unless(state & RmS_Running) continue;
        
        purpose = RmGetProcessorPurpose(neighbour) & RmP_Mask;
	if (purpose eq RmP_Native) continue;

        for (j = 0; j < max_index; j++)
         if (neighbour eq search_table[j])
          goto skip;
          
        search_table[max_index++] = neighbour;
skip:
	neighbour = neighbour;
      }        
   }
   
  Free(search_table);
  return(max_index);
}

static void	SortProcessors(int count, RmProcessor	*table)
{ int		end = count;
  int		i, j, number_links, destlink, changes;
  RmProcessor	temp, neighbour;
  
 	/* Step one, set all the processors in the table to native */
  for (i = 0; i < count; i++)
   RmSetProcessorPurpose(table[i], RmP_Native);
   
  	/* Step two, search the table for processors with Helios neighbours. */
  	/* This is done in a bubble-sort style loop.			     */
  for (changes = 1; changes ne 0; )
   { changes = 0;
     for (i = 0; i < end; i++)
      { number_links = RmCountLinks(table[i]);
        for (j = 0; j < number_links; j++)
         { neighbour = RmFollowLink(table[i], j, &destlink);
           if ((neighbour eq RmM_ExternalProcessor) ||
               (neighbour eq RmM_NoProcessor))
            continue;
           if ((RmGetProcessorPurpose(neighbour) & RmP_Mask) ne RmP_Helios)
            continue;
           unless(RmGetProcessorState(neighbour) & RmS_Running)
            continue;
           
          	/* this processor can be moved to the end. */
           end--;
           temp = table[end];
           table[end] = table[i];
           table[i] = temp;
           RmSetProcessorPurpose(table[end], RmP_Helios); /* This works !! */
           changes++;
           break;	/* out of links loop */
         }
      }
   }
}
/*}}}*/
/*{{{  HandleReset() */
/**
*** Resetting processors. This goes via a routine in netboot.c, which is also
*** used for re-booting things.
**/
static	int HandleReset(int count, ProcessorDetails *details)
{ RmProcessor	*table = Malloc(count * sizeof(RmProcessor));
  int		real_count, i, rc;

  Debug(dbg_Native, ("request to reset %d processors", count));
  
  if (table eq Null(RmProcessor)) return(RmE_ServerMemory);
  for (i = real_count = 0; i < count; i++)
   if (details[i].Uid ne RmL_NoUid) 
    table[real_count++] = RmFindProcessor(Net, details[i].Uid);
 
  for (i = 0; i < real_count; i++)
   { int state = RmGetProcessorState(table[i]);
     state &= ~(RmS_Running | RmS_Suspicious | RmS_Crashed | RmS_Dead);
     state |= RmS_ShouldBeReset;
     RmSetProcessorState(table[i], state);
   }

  rc = ResetProcessors(real_count, table); 
  Free(table);
  return(rc);
}
/*}}}*/
/*{{{  HandleReboot() */
/**
*** Rebooting processors. This involves a new bootstrap job. All the code is
*** already in netboot.c. Rebooting processors that are native by default
*** is a no-op.
**/
static	int	HandleReboot(int count, ProcessorDetails *details)
{ BootstrapJob	*job	= NewBootstrapJob();
  int		i, booted, really_booted;

  Debug(dbg_Native, ("request to reboot %d processors", count));
  
  if (job eq Null(BootstrapJob)) goto fail;

  for (i = 0; i < count; i++)
   if (details[i].Uid ne RmL_NoUid)
    { RmProcessor	processor = RmFindProcessor(Net, details[i].Uid);
      ProcessorEntry	*proc_entry;

	/* Reset the default purpose */
      proc_entry = GetProcEntry(processor);
      RmSetProcessorPurpose(processor, proc_entry->Purpose);

      unless(AddProcessorToBootstrapJob(job, processor))
       { AbortBootstrapJob(job); goto fail; }
    }

  unless(StartBootstrapJob(job))
   { AbortBootstrapJob(job); goto fail; }

  WaitBootstrapJob(job);

  for (i = 0, booted = 0, really_booted = 0; i < count; i++)
   if (details[i].Uid eq RmL_NoUid)
    booted++;
   else
    { RmProcessor processor = RmFindProcessor(Net, details[i].Uid);

      if ((RmGetProcessorPurpose(processor) & RmP_Mask) eq RmP_Native)
       booted++;
      elif (RmGetProcessorState(processor) & RmS_Running)
       { booted++; really_booted++; }
    }

  if (booted eq count)
   return(RmE_Success);
  elif (really_booted > 0)
   return(RmE_PartialSuccess);
  else
   return(RmE_BadProcessor);

fail:
  return(RmE_ServerMemory);
}
/*}}}*/

void	HandleNative(NsConn connection, int job_id, 
		RmRequest *request, RmReply *reply)
{ ProcessorDetails	*details;
  ProcessorUpdate	*updates = Null(ProcessorUpdate);
  RmProcessor		processor;
  int			rc, i;
  int			count;

  count = request->Arg1;
  details	= (ProcessorDetails *) request->VariableData;
  
  updates = Malloc(count * sizeof(ProcessorUpdate));
  if (updates eq Null(ProcessorUpdate))
   { rc = RmE_ServerMemory; goto done; }
   
  for (i = 0; i < count; i++)
   { if (details[i].Uid eq RmL_NoUid) continue;
     processor = RmFindProcessor(Net, details[i].Uid);
     if (processor eq (RmProcessor) NULL)
      { rc = RmE_BadProcessor; goto done; }

     if (processor->SessionId ne connection->Id)
      { rc = RmE_BadProcessor; goto done; }

     unless(GetAccess(&(details[i].Cap), processor->ObjNode.Key) &&
     		(details[i].Cap.Access & AccMask_D))
      { rc = RmE_NoAccess; goto done; }

	/* Reset and reboot operations can be applied only to	*/
	/* currently native processors.				*/
     if (request->FnRc ne RmC_SetNative)
      unless((RmGetProcessorPurpose(processor) & RmP_Mask) eq RmP_Native)
       { rc = RmE_BadProcessor; goto done; }
   } 

  switch(request->FnRc)
   { case RmC_SetNative		: rc = HandleSetNative(count, details); break;
     case RmC_ResetProcessors 	: rc = HandleReset(count, details); break;
     case RmC_Reboot		: rc = HandleReboot(count, details); break;
     default			: rc = RmE_Corruption;
   }

done:
  if ((rc eq RmE_Success) || (rc eq RmE_PartialSuccess))
   for (i = 0; i < count; i++)
    { processor = RmFindProcessor(Net, details[i].Uid);
      if (processor eq (RmProcessor) NULL) continue;
      updates[i].Uid	 = details[i].Uid;
      updates[i].Purpose = RmGetProcessorPurpose(processor);
      updates[i].State	 = RmGetProcessorState(processor);

      	/* After the reply is sent the Network Server cannot be sure that */
      	/* the processor is still reset, because an application is likely */
      	/* to have been booted into it.					  */
      processor->ObjNode.Size &= ~(RmS_Reset | RmS_ShouldBeReset);
    }

   reply->FnRc = rc;
   if ((rc eq RmE_Success) || (rc eq RmE_PartialSuccess))
    { reply->VariableData	= (BYTE *) updates;
      reply->VariableSize	= count * sizeof(ProcessorUpdate);
    }
   ReplyRmLib(connection, job_id, reply);
   if (updates ne Null(ProcessorUpdate)) Free(updates);
}

/*}}}*/
/*{{{  Link connections */
/**
*** Connection stuff. 
**/

static	int CheckCapabilities(int request, int count, LinkConnection *conns,
			NsConn connection);
static	int RevertLinks(int count, LinkConnection *conns);
static	int CheckHeliosWorld(int count, LinkConnection *conns);
static  RmHardwareFacility *CheckHardware(int count, LinkConnection *conns);
static	int	BuildConnectionTable(int count, LinkConnection *conns, 
		DriverConnection *driver_conns);
		
void	HandleConnections(NsConn connection, int job_id, 
		RmRequest *request, RmReply *reply)
{ LinkConnection	*conns;
  int			count, driver_count;
  int			rc, i;
  DriverConnection	*driver_conns = Null(DriverConnection);
  RmHardwareFacility	*confdriver = Null(RmHardwareFacility);
  DriverConfRequest	driver_request;
  DCB			*device;

	/* Get all the required data from the TFM */  
  count		= request->Arg1;
  conns		= (LinkConnection *) request->VariableData;
  
  Debug(dbg_Native, ("request to manipulate %d processor connections", count));

  driver_conns = Malloc(count * sizeof(DriverConnection));
  if (driver_conns eq Null(DriverConnection))
   { rc = RmE_ServerMemory; goto done; }

	/* Step one, check the capabilities.				*/
  if ((rc = CheckCapabilities(request->FnRc, count, conns, connection))
	 ne RmE_Success)
   goto done;

	/* Step two, if the processors are reverting then it is 	*/
	/* necessary to fill in the destination processors and links	*/
	/* from the saved information in the ProcessorEntry structure.	*/
	/* There is no guarantee that these links can actually be made,	*/
	/* but that is handled later on.				*/
  if (request->FnRc eq RmC_Revert)
   { Debug(dbg_Native, ("revert request, filling in links"));
     if ((rc = RevertLinks(count, conns)) ne RmE_Success)
      goto done;
   }

	/* Step three, check that the connections make sense in terms of*/
	/* the Helios world.						*/
  Debug(dbg_Native, ("checking validity of requested connections"));
  if ((rc = CheckHeliosWorld(count, conns)) ne RmE_Success)
   goto done;

  Debug(dbg_Native, ("checking hardware limitations"));
  confdriver = CheckHardware(count, conns);
  if (confdriver eq Null(RmHardwareFacility))
   { rc = RmE_NotPossible; goto done; }

	/* Make the required connections */
  driver_count = BuildConnectionTable(count, conns, driver_conns);

  if (driver_count eq 0)	/* after all that ? */
   { rc = RmE_Success; goto done; }
  if (driver_count < 0)		/* what ? */
   { rc = RmE_Corruption; goto done; }

  if (DebugOptions & dbg_Native)
   { int i;
     debug("calling the device driver to %s %d connections",
	(request->FnRc eq RmC_TestConnections) ? "test" : "make", driver_count);
     for (i = 0; i < driver_count; i++)
      debug("%d: %P link %d -> %s link %d", i,
	driver_conns[i].Source, driver_conns[i].SourceLink,
	(driver_conns[i].Dest eq RmM_NoProcessor) ? "(NULL)" :
        (driver_conns[i].Dest eq RmM_ExternalProcessor) ? "(EXT)" :
	Procname(driver_conns[i].Dest), driver_conns[i].DestLink);
   }
  	   
	/* Invoke the device driver */   
  switch(request->FnRc)
   { case RmC_Revert :
     case RmC_MakeConnections : driver_request.FnRc = ND_MakeLinks; break;
     case RmC_TestConnections : driver_request.FnRc = ND_TestLinks; break;
   }
  driver_request.NumberConnections = driver_count;
  driver_request.Exact		   = request->Arg2;
  driver_request.Preserve	   = request->Arg3;
  driver_request.Connections	   = driver_conns;

  device = (DCB *) confdriver->Device;
  (*(device->Operate))(device, &driver_request);

  rc = driver_request.FnRc;
     
done:
	/* Build the new reply data, so that the TFM and client can update */
	/* their connections.						   */
  if (((request->FnRc eq RmC_Revert) || (request->FnRc eq RmC_MakeConnections)) &&
      ((rc eq RmE_Success) || (rc eq RmE_PartialSuccess)))
   for (i = 0; i < count; i++)
    { RmProcessor	source = RmFindProcessor(Net, conns[i].SourceUid);
      RmLink		*link;
      if (source eq (RmProcessor) NULL) continue;
      link = RmFindLink(source, conns[i].SourceLink);
      conns[i].DestUid	= link->Target;
      conns[i].DestLink = link->Destination;  
    }

  reply->FnRc	= rc;
  if (((request->FnRc eq RmC_Revert) || (request->FnRc eq RmC_MakeConnections)) &&
      ((rc eq RmE_Success) || (rc eq RmE_PartialSuccess)))
   { reply->VariableData	= (BYTE *) conns;
     reply->VariableSize	= count * sizeof(LinkConnection);
   }
  ReplyRmLib(connection, job_id, reply);
  if (driver_conns ne Null(DriverConnection)) Free(driver_conns);
}


/**
*** Checking the capabilities. For every processor in the table, check the
*** source processor Uid/capability pair. If reverting then the destination
*** entries in the table can be ignored. Otherwise these must be checked too,
*** to some extent. There may be problems: the user may have left a connection
*** from a native processor to somebody else's processor, and this connection
*** would be sent. As far as the user is concerned this link is external, and
*** hence he/she does not have a capability for the destination processor.
***
*** In addition to the capabilities it is desirable to check the link
*** numbers. These are unlikely to be wrong, and even if they were then the
*** TFM should have caught the error, but the check is cheap. 
**/
static int	CheckCapabilities(int request, int count, LinkConnection *conns,
			NsConn connection)
{ int 		i;
  RmProcessor	processor;

  for (i = 0; i < count; i++)
   { processor = RmFindProcessor(Net, conns[i].SourceUid);
     if (processor eq RmM_NoProcessor) return(RmE_Corruption);
     if (processor->SessionId ne connection->Id)
      return(RmE_NoAccess);
     unless(GetAccess(&(conns[i].SourceCap), processor->ObjNode.Key) &&
            (conns[i].SourceCap.Access & AccMask_D))
      return(RmE_NoAccess);
     if ((conns[i].SourceLink < 0) ||
         (conns[i].SourceLink >= RmCountLinks(processor)))
      return(RmE_BadLink);
   }

  unless(request eq RmC_Revert)
   for (i = 0; i < count; i++)
    { if ((conns[i].DestUid eq RmL_NoUid) ||
          (conns[i].DestUid eq RmL_ExtUid) ||
          (conns[i].DestCap.Access eq 0))
       continue;
      processor = RmFindProcessor(Net, conns[i].DestUid);
      if (processor eq RmM_NoProcessor) return(RmE_Corruption);

      unless(GetAccess(&(conns[i].DestCap), processor->ObjNode.Key) &&
             (conns[i].DestCap.Access & AccMask_D))
       return(RmE_NoAccess);
      if ((conns[i].DestLink < 0) ||
          (conns[i].DestLink >= RmCountLinks(processor)))
       return(RmE_BadLink);
    }            

  return(RmE_Success);    
}

/**
*** Reverting the links. For every source processor extract the destination
*** processor and link from the processor entry structure. If the destination
*** processor/link is not currently available, set this link to not-connected
*** instead. It is probable that later in the job the destination processor
*** will be handled.
**/
static int	RevertLinks(int count, LinkConnection *conns)
{ int 			i, j, destlink;
  RmProcessor		processor, neighbour;
  ProcessorEntry	*proc_entry;
  RmLink		*link;

  for (i = 0; i < count; i++)
   { 
     processor = RmFindProcessor(Net, conns[i].SourceUid);
	
     proc_entry = GetProcEntry(processor);
     conns[i].DestUid  = proc_entry->StandardLinks[conns[i].SourceLink].Target;
     conns[i].DestLink = proc_entry->StandardLinks[conns[i].SourceLink].Destination;

	/* If the default processor/link matches the current one, then	*/
	/* this link revert is a no-op.					*/
     link = RmFindLink(processor, conns[i].SourceLink);
     if ((link->Target eq conns[i].DestUid) &&
	 (link->Destination eq conns[i].DestLink))
      goto next_conn;

	/* If the default processor/link is also reverting, fine */
     for (j = 0; j < count; j++)
      if ((conns[i].DestUid eq conns[j].SourceUid) &&
          (conns[i].DestLink eq conns[j].SourceLink))
       goto next_conn;

	/* Check what is at the other end by default */     
     processor = RmFindProcessor(Net, conns[i].DestUid);
     if ((processor eq RmM_NoProcessor) || (processor eq RmM_ExternalProcessor))
      continue;

     neighbour = RmFollowLink(processor, conns[i].DestLink, &destlink);
     if (neighbour ne RmM_NoProcessor)
      { 
        conns[i].DestUid  = RmL_NoUid;	/* leave this link broken for now */
        conns[i].DestLink = -1;
      }

next_conn:
     i = i;      
   }

  return(RmE_Success); 
}

/**
*** Checking the Helios world. There are lots of little problems to consider
*** here.
*** 0) if the link is not changing it must be legal, since it is not possible
***    to produce an illegal network.
*** 1) the processor is currently a Helios node, note the check in RevertLinks()
***    above
***    a) the new destination is not connected
***      1) the current destination is not connected, so this is a no-op
***      2) the current destination is external, not allowed
***      3) the current destination is native, ok
***      4) the current destination is not native, not allowed
***    b) the new destination is external or another Helios node
***      1) the current destination is not connected or a native node
***        allow iff this remakes a default connection and this connection is
***        free.
***      2) the current destination is external or a non-Helios node
***        what's going on ?
***    c) the new destination is a native node
***      1) the current destination is not connected or a native node,
***	    or an external connector, ok
***      2) any other combination is illegal
*** 2) the processor is currently a native node
***    a) the new destination is not connected, always ok
***    b) the new destination is external, always ok
***    c) the new destination is another native node, always ok
***    d) the new destination is a non-native node, allow iff that link of
***       that non-native node is currently not connected or goes to another
***       native node. Not connected could cause problems when reverting.
**/
static	int	CheckHeliosWorld(int count, LinkConnection *conns)
{ RmProcessor	processor, new_neighbour, current_neighbour;
  RmLink	*link;
  int		i, destlink;
  bool		this_native, current_native, new_native;
  
  for (i = 0; i < count; i++)
   { processor	= RmFindProcessor(Net, conns[i].SourceUid);
     link	= RmFindLink(processor, conns[i].SourceLink);

	/* Test 0, is the link unchanged ? */
     if ((link->Target eq conns[i].DestUid) &&
         (link->Destination eq conns[i].DestLink))
      continue;

     this_native = current_native = new_native = FALSE;
     if ((RmGetProcessorPurpose(processor) & RmP_Mask) eq RmP_Native)
      this_native = TRUE;
      
     current_neighbour = RmFollowLink(processor, conns[i].SourceLink, &destlink);
     if ((current_neighbour ne RmM_NoProcessor) && 
         (current_neighbour ne RmM_ExternalProcessor))
      if ((RmGetProcessorPurpose(current_neighbour) & RmP_Mask) eq RmP_Native)
       current_native = TRUE;
       
     new_neighbour = RmFindProcessor(Net, conns[i].DestUid);
     if ((new_neighbour ne RmM_NoProcessor) &&
         (new_neighbour ne RmM_ExternalProcessor))
      if ((RmGetProcessorPurpose(new_neighbour) & RmP_Mask) eq RmP_Native)
       new_native = TRUE;

       		/* Test 1, is the node currently not native ? */
     if (!this_native)
      { 
        if (new_neighbour eq RmM_NoProcessor)	/* test 1.a */
         { 
	   if (current_neighbour eq RmM_NoProcessor) continue;
           if (current_neighbour eq RmM_ExternalProcessor)
            return(RmE_BadLink);
           if (current_native) continue;
           return(RmE_BadProcessor);
         }					/* test 1.b */
        if ((new_neighbour eq RmM_ExternalProcessor) || !new_native)
         { 
	   if ((current_neighbour eq RmM_NoProcessor) || current_native)
            { ProcessorEntry	*proc_entry;
              RmLink		*this_link;
              RmProcessor	temp;
              proc_entry = GetProcEntry(processor);
              this_link = &(proc_entry->StandardLinks[conns[i].SourceLink]);
              if ((this_link->Target ne conns[i].DestUid) ||
                  (this_link->Destination ne conns[i].DestLink))
               return(RmE_BadLink);

              temp = RmFollowLink(new_neighbour, conns[i].DestLink, NULL);
              if (temp ne RmM_NoProcessor)
               return(RmE_InUse);
              continue;
            }
         }
        if (new_native)		/* test 1.c */
         { 
	   if ((current_neighbour eq RmM_NoProcessor) ||
               (current_neighbour eq RmM_ExternalProcessor) ||
               current_native)
            continue;
           else
	    return(RmE_InUse);
	 }            
      }
     else		/* test 2 */
      { RmProcessor	temp;
        int		destlink;
        if (new_neighbour eq RmM_NoProcessor) continue;
        if (new_neighbour eq RmM_ExternalProcessor) continue;
        if (new_native) continue;
        
        temp = RmFollowLink(new_neighbour, conns[i].DestLink, &destlink);
        if (temp eq RmM_NoProcessor) continue;
        if (temp eq RmM_ExternalProcessor) return(RmE_InUse);
        if ((RmGetProcessorPurpose(temp) & RmP_Mask) ne RmP_Native)
         return(RmE_InUse);
      }
   }	/* end of for loop */
}

/**
*** Checking that the reconfiguration requested actually makes sense in
*** general hardware terms, without bothering about the specifics of certain
*** machines. This requires the following:
***  1) find the configuration driver, if any. There can be at most one.
***  2) skip any links that are not changing
***  3) check the source link to make sure it is configurable
***  4) If a link is changing then there must be a configuration driver
***  5) repeat for the destination if appropriate
***  6) and for the current connection
**/
static	RmHardwareFacility *CheckHardware(int count, LinkConnection *conns)
{ RmProcessor		processor, neighbour;
  int			destlink, i, flags, j;
  ProcessorEntry	*proc_entry;
  RmHardwareFacility	*hardware = Null(RmHardwareFacility);
  RmLink		*link;
  bool			found_driver;
      
  for (i = 0; i < count; i++)
   { processor = RmFindProcessor(Net, conns[i].SourceUid);

     proc_entry = GetProcEntry(processor);
     for (j = 0; j < proc_entry->NumberDrivers; j++)
      { DriverEntry *driver = &(proc_entry->DriverEntry[j]);
      	if (driver->Flags & DriverFlags_ConfigureDriver)
         { if (hardware eq Null(RmHardwareFacility))
            hardware = driver->Hardware;
           if (hardware ne driver->Hardware)
            return(Null(RmHardwareFacility));
           break;
         }
      }

     link = RmFindLink(processor, conns[i].SourceLink);
     if ((link->Destination eq conns[i].DestLink) &&
         (link->Target eq conns[i].DestUid))
      continue;
      
     flags = RmGetLinkFlags(processor, conns[i].SourceLink);
     unless(flags & RmF_Configurable)
      return(Null(RmHardwareFacility));

      	/* If that loop has failed to reveal a configuration driver.... */
     if (hardware eq Null(RmHardwareFacility))
      return(Null(RmHardwareFacility));

	/* Now check the desired destination processor */
     if ((conns[i].DestUid ne RmL_NoUid) && (conns[i].DestUid ne RmL_ExtUid))
      { found_driver = FALSE;
        neighbour = RmFindProcessor(Net, conns[i].DestUid);
        flags = RmGetLinkFlags(neighbour, conns[i].DestLink);
        unless(flags & RmF_Configurable)
         return(Null(RmHardwareFacility));
        
        proc_entry = GetProcEntry(neighbour);
        for (j = 0; j < proc_entry->NumberDrivers; j++)
         { DriverEntry *driver = &(proc_entry->DriverEntry[j]);
           
           if (driver->Flags & DriverFlags_ConfigureDriver)
            { if (hardware ne driver->Hardware)
               return(Null(RmHardwareFacility));
              found_driver = TRUE;
              break;
            }
         }
        unless(found_driver) return(Null(RmHardwareFacility));
      }

	/* and check the current connection. Some of this seems redundant */
     neighbour = RmFollowLink(processor, conns[i].SourceLink, &destlink);
     if ((neighbour eq RmM_NoProcessor) || (neighbour eq RmM_ExternalProcessor))
      continue;

     flags = RmGetLinkFlags(neighbour, destlink);
     unless(flags & RmF_Configurable)
      return(Null(RmHardwareFacility));
      
     proc_entry = GetProcEntry(neighbour);
     found_driver = FALSE;
     for (j = 0; j < proc_entry->NumberDrivers; j++)
      { DriverEntry	*driver = &(proc_entry->DriverEntry[j]);
        if (driver->Flags & DriverFlags_ConfigureDriver)
         { if (hardware ne driver->Hardware)
            return(Null(RmHardwareFacility));
           found_driver = TRUE;
           break;
         }
      }
     unless(found_driver) return(Null(RmHardwareFacility));
   }

	/* Using the available information, it would appear that	*/
	/* this attempt at configuration might actually be possible.	*/
  return(hardware);      
}

/**
*** Building the connections table.
**/
static bool AddConnection(int *count_ptr, DriverConnection *conns,
		RmProcessor source, int sourcelink, RmProcessor dest, int destlink)
{ int i;
  int count = *count_ptr;

  for (i = 0; i < count; i++)
   if ((conns[i].Dest eq source) && (conns[i].DestLink eq sourcelink))
    return(TRUE);
	
  conns[count].Source		= source;
  conns[count].SourceLink	= sourcelink;
  conns[count].Dest		= dest;
  conns[count].DestLink		= destlink;
  count++;
  *count_ptr = count;
  return(TRUE);
}

static int BuildConnectionTable(int count, LinkConnection *conns, 
		DriverConnection *driver_conns)
{ int driver_count = 0;
  int i;
  
  for (i = 0; i < count; i++)
   { RmProcessor source = RmFindProcessor(Net, conns[i].SourceUid);
     RmProcessor dest	= RmFindProcessor(Net, conns[i].DestUid);
     if (!AddConnection(&driver_count, driver_conns, source,
     		conns[i].SourceLink, dest, conns[i].DestLink))
      return(-1);
   }
  return(driver_count);
}
/*}}}*/
/*{{{  Cleaning out native processors */
/**
*** Cleaning out native processors. This may be called for two reasons:
*** 1) the processor is running Helios, but one or more of its links have
***    been reconnected to native subnetworks. This links must be
***    restored if possible, otherwise left disconnected.
*** 2) the processor is in native mode, may or may not have some of its
***    links reconfigured, but definitely has to be rebooted. Rebooting
***    this processor may allow previously dead processors to be rebooted
***    as well.
***
*** The work done is as follows:
*** a) for all the current links, check if they have been changed. If so
***    attempt to revert them. Note that during this phase the network
***    server has to be single stepped in case links are being fiddled
***    with in another process.
*** b) if the processor is currently running Helios, there is nothing left
***    to do.
*** c) mark the processor as a Helios processor again
*** d) if the processor does not currently have a Helios neighbour then
***    it must be marked as crashed, as there is no way of rebooting it.
*** e) otherwise a bootstrap job has to be prepared. A breadth-first search
***    takes place for adjacent crashed processors, and these are all
***    added to the bootstrap job.
*** f) the routine waits for the bootstrap job to finish
**/

static	void CleanLinks(RmProcessor);
static	void RebootCrashed(RmProcessor);

void	CleanNative(RmProcessor processor)
{ int			number_links = RmCountLinks(processor);
  int			i;
  RmLink		*current, *dflt;
  ProcessorEntry	*proc_entry;
  int			uid = processor->Uid;

  MRSW_FreeRead();
  MRSW_GetWrite();
  processor = RmFindProcessor(Net, uid);
  if (processor eq RmM_NoProcessor)
   goto done;

  Debug(dbg_Native, ("having to clean out native processor %P", processor));
  
  proc_entry = GetProcEntry(processor);

  for (i = 0; i < number_links; i++)
   { current = RmFindLink(processor, i);
     dflt = &(proc_entry->StandardLinks[i]);
     if ((current->Target eq dflt->Target) &&
         (current->Destination eq dflt->Destination))
      continue;
     CleanLinks(processor);
     break;
   }     

  if ((RmGetProcessorPurpose(processor) eq RmP_Helios) &&
      (RmGetProcessorState(processor) & RmS_Running))
   goto done;

	/* If the processor's purpose is native by default, do not	*/
	/* attempt to reboot it. However, resetting it is a good idea.	*/
  if ((proc_entry->Purpose & RmP_Mask) eq RmP_Native)
   { ResetProcessors(1, &processor);
     RmSetProcessorState(processor, RmS_Running);
     goto done;
   }

	/* Otherwise reset the processor to be a Helios one, and see	*/
	/* if a reboot is possible.					*/
  RmSetProcessorPurpose(processor, RmP_Helios);
  RmSetProcessorState(processor, RmS_Dead);

  for (i = 0; i < number_links; i++)
   { RmProcessor neighbour = RmFollowLink(processor, i, NULL);
     if ((neighbour eq RmM_NoProcessor) ||
         (neighbour eq RmM_ExternalProcessor))
      continue;

     if (RmGetProcessorPurpose(neighbour) ne RmP_Helios)
      continue;
     unless(RmGetProcessorState(neighbour) & RmS_Running)
      continue;
     RebootCrashed(processor);
     break;
   }      

done:
  MRSW_SwitchRead();
}

/**
*** Cleaning up the links. For all the links on this processor,
***  1) if the link is OK, do nothing
***  2) if the default is not-connected, break this link
***  3) if the default link is free, make this connection
***  4) otherwise break this link
**/
static	void CleanLinks(RmProcessor processor)
{ ProcessorEntry	*proc_entry;
  int			number_links = RmCountLinks(processor);
  int			i, index;
  RmLink		*current, *dflt;
  DriverConfRequest	request;
  RmHardwareFacility	*hardware;
  DriverConnection	*conns;
  DriverEntry		*driver_entry;

  Debug(dbg_Native, ("attempting to revert default links of processor %P", processor));
      
  conns = Malloc(number_links * sizeof(DriverConnection));
  if (conns eq Null(DriverConnection))
   return;
  request.FnRc		= ND_MakeLinks;
  request.Preserve	= TRUE;
  request.Exact		= TRUE;
  request.Connections	= conns;
  
  proc_entry = GetProcEntry(processor);
  for (i = 0, index = 0; i < number_links; i++)
   { 
     current = RmFindLink(processor, i);
     dflt = &(proc_entry->StandardLinks[i]);
     
     if ((current->Target eq dflt->Target) &&
         (current->Destination eq dflt->Destination))
      continue;
      
     conns[index].Source	= processor;
     conns[index].SourceLink	= i;
     if ((dflt->Target eq RmL_NoUid) ||
         (dflt->Target eq RmL_ExtUid))
      { conns[index].Dest     = (RmProcessor) dflt->Target;
        conns[index].DestLink = dflt->Destination;
      }
     else
      { RmProcessor neighbour = RmFindProcessor(Net, dflt->Target);
        RmLink	    *temp = RmFindLink(neighbour, dflt->Destination);
        if (temp->Target eq RmL_NoUid)
         { conns[index].Dest	 = neighbour;
           conns[index].DestLink = dflt->Destination;
         }
        else
         { conns[index].Dest	 = RmM_NoProcessor;
           conns[index].DestLink = dflt->Destination;
         }
      }
     index++;
   }

  if (index eq 0) goto done;	/* nothing can be done */
  
  request.NumberConnections = index;
  hardware = Null(RmHardwareFacility);
  for (i = 0; i < proc_entry->NumberDrivers; i++)
   { driver_entry = &(proc_entry->DriverEntry[i]);
     if (driver_entry->Flags & DriverFlags_ConfigureDriver)
      { hardware = driver_entry->Hardware;
        break;
      }
   }
   
  if (hardware ne Null(RmHardwareFacility))
   { DCB *device = (DCB *) hardware->Device;
     (*(device->Operate))(device, &request);
   }

done:
  Free(conns);
}

/**
*** Rebooting a processor that has crashed or that used to be a native node.
*** In theory this routine is very easy. In practice there is a complication:
*** when native processors are released one by one it may not be possible
*** to reboot them as they become free, because at the time they do not
*** have a running Helios processor. Hence this routine needs to do a
*** breadth-first search for neighbouring crashed processors and add them
*** to the bootstrap job.
**/

static	void RebootCrashed(RmProcessor processor)
{ RmProcessor	*search_table = Malloc(NumberProcessors * sizeof(RmProcessor));
  int		current_index = 0;
  int		max_index = 0;
  BootstrapJob	*job;

  Debug(dbg_Native, ("attempting to reboot processor %P and any crashed neighbours", processor));
  
  if (search_table eq Null(RmProcessor)) return;
  job = NewBootstrapJob();
  if (job eq Null(BootstrapJob)) goto done;

  Debug(dbg_Boot, ("attempting to reboot crashed processor %P in job %d",\
	  	processor, job->Sequence));
  	
  search_table[max_index++] = processor;
  for ( ; current_index ne max_index; current_index++)
   { RmProcessor processor = search_table[current_index];
     int	 number_links = RmCountLinks(processor);
     int	 i, j;

     Debug(dbg_Boot, ("including processor %P in job %d", processor, \
     		job->Sequence));
     		     
     unless(AddProcessorToBootstrapJob(job, processor))
      { AbortBootstrapJob(job);
        goto done;
      }
     for (i = 0; i < number_links; i++)
      { RmProcessor neighbour = RmFollowLink(processor, i, NULL);
        if ((neighbour eq RmM_NoProcessor) ||
            (neighbour eq RmM_ExternalProcessor))
         continue;
        if (RmGetProcessorPurpose(neighbour) ne RmP_Helios)
         continue;
        unless(RmGetProcessorState(neighbour) & RmS_Crashed)
         continue;
        for (j = 0; j < max_index; j++)
         if (neighbour eq search_table[j])
          goto next_link;
        search_table[max_index++] = neighbour;
next_link:
        neighbour = neighbour;
      }		/* for every link */
   } /* for every crashed processor */
   
  Free(search_table); search_table = Null(RmProcessor);
  unless(StartBootstrapJob(job))
   { AbortBootstrapJob(job); goto done; }

  WaitBootstrapJob(job);
  
done:
  if (search_table ne Null(RmProcessor)) Free(search_table);
}
/*}}}*/
#endif
