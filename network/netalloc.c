/*------------------------------------------------------------------------
--                                                                      --
--           H E L I O S   N E T W O R K I N G   S O F T W A R E	--
--           ---------------------------------------------------	--
--                                                                      --
--             Copyright (C) 1990, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- netalloc.c								--
--                                                                      --
--	The processor allocation module of the Network Server		--
--                                                                      --
--	Author:  BLV 4/9/90						--
--                                                                      --
------------------------------------------------------------------------*/
/*$Header: /hsrc/network/RCS/netalloc.c,v 1.20 1993/08/11 10:32:20 bart Exp $*/

/*{{{  headers and compile-time options */
#define	__Netalloc_Module
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
#include <stddef.h>
#include "exports.h"
#include "private.h"
#include "netutils.h"
#include "rmlib.h"
#include "netaux.h"
/*}}}*/
/*{{{  forward declarations and statics */
static	void	SendToCleaners(RmProcessor);
static	Semaphore	SingleAlloc;

void	InitAlloc(void)
{ InitSemaphore(&SingleAlloc, 1);
}
/*}}}*/
/*{{{  match_processor() utility */
/**
*** Match a processor with a template. This can get quite complicated.
*** 1) there are restrictions on the processors that can be allocated for
***    native use
*** 2) if the template has any attributes then the processor must
***    have the same attributes, but not vice versa
*** 3) various combinations of processor types may or may not match
*** 4) the real processor must have at least the amount of memory requested
**/

static	bool	match_processor(RmProcessor template, RmProcessor real)
{ int	attribute_count = RmCountProcessorAttributes(template);
  int	template_ptype;
  int	real_ptype;

  if (RmGetProcessorPurpose(template) eq RmP_Native)
   { int		number_links, i, destlink, control;
     RmProcessor	neighbour;
     control = RmGetProcessorControl(real);
     unless(control & RmC_Native) return(FALSE);
     number_links = RmCountLinks(real);
     for (i = 0; i < number_links; i++)
      { neighbour = RmFollowLink(real, i, &destlink);
        if (neighbour eq RmM_NoProcessor) continue;
        if (neighbour eq RmM_ExternalProcessor) return(FALSE);
        if ((RmGetProcessorPurpose(neighbour) & RmP_Mask) eq RmP_IO)
         return(FALSE);
      }
   }
   
  if (attribute_count > 0)
   { char	*attribs[10];
     char	**real_attribs;
     int	i;

	/* very simple test, to start with */
     if (attribute_count > RmCountProcessorAttributes(real)) return(FALSE);

     if (attribute_count > 10)
      { real_attribs = (char **) Malloc(attribute_count * sizeof(char *));
        if (real_attribs eq Null(char *)) return(FALSE);
      }
     else
      real_attribs = attribs;
     if (RmListProcessorAttributes(template, real_attribs) ne RmE_Success)
      { if (attribute_count > 10) Free(real_attribs);
        return(FALSE);
      }
     for (i = 0; i < attribute_count; i++)
      unless(RmTestProcessorAttribute(real, real_attribs[i]) eq RmE_Success)
       { if (attribute_count > 10) Free(real_attribs);
         return(FALSE);
       }
    if (attribute_count > 10) Free(real_attribs); 
   }

  if (RmGetProcessorMemory(real) < RmGetProcessorMemory(template))
   return(FALSE);

  template_ptype	= RmGetProcessorType(template);
  real_ptype		= RmGetProcessorType(real);
  if ((template_ptype ne RmT_Default) && (template_ptype ne real_ptype))
   return(FALSE);

  	/* I have no way of working out the true requirement */
   return(TRUE);
}
/*}}}*/

/*{{{  HandleGetNetwork() */
/**----------------------------------------------------------------------------
*** HandleGetNetwork(). This is used to obtain full details of the current
*** network. It is also used to get network hardware details and network
*** hierarchies, i.e. the name of the root network and all subnets but
*** no details of any processors.
**/
static int  GetNetwork_NetworkFilter(RmNetwork, RmNetwork);
static int  GetNetwork_ProcessorFilter(RmProcessor, RmProcessor);
static int  GetHierarchy_NetworkFilter(RmNetwork, RmNetwork);
static int  GetHierarchy_ProcessorFilter(RmProcessor, RmProcessor);

void HandleGetNetwork(NsConn connection, int job_id,
		RmRequest *request, RmReply *reply)
{ RmFilterStruct	filter;

  if (request->FnRc eq RmC_GetNetworkHardware)
   filter.SendHardware = TRUE;
  else
   filter.SendHardware = FALSE;
  if (request->FnRc eq RmC_GetHierarchy)
   { filter.Network	= &GetHierarchy_NetworkFilter;
     filter.Processor	= &GetHierarchy_ProcessorFilter;
   }
  else
   { filter.Network	= &GetNetwork_NetworkFilter;
     filter.Processor	= &GetNetwork_ProcessorFilter;
   }
  filter.Taskforce	= NULL;
  filter.Task		= NULL;

  reply->FnRc		= RmE_Success;
  reply->Network	= Net;
  reply->Filter		= &filter;
  ReplyRmLib(connection, job_id, reply);
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
  if (copy->ObjNode.Flags & NsFlags_DenyReadOnly)
   memset(&(copy->RealCap), 0, sizeof(Capability));
  else
   { ProcessorEntry	*proc_entry;
     proc_entry = GetProcEntry(real);
     memcpy(&(copy->RealCap), &(proc_entry->General), sizeof(Capability));
   }
  copy->Private = 0;
  
  return(RmE_Success);
}

static int GetHierarchy_NetworkFilter(RmNetwork real, RmNetwork copy)
{ 
  copy->DirNode.Nentries	= copy->NoSubnets;
  copy->DirNode.Key		= 0;
  copy->StructType		= RmL_Obtained;
  if (real eq RmRootNetwork((RmProcessor) real))
   strcpy(copy->DirNode.Name, NetworkName);
  return(RmE_Success);
}

static int GetHierarchy_ProcessorFilter(RmProcessor real, RmProcessor copy)
{
  real = real; copy = copy;
  return(RmE_Skip);
}
/*}}}*/
/*{{{  HandleLastChange() */
/**----------------------------------------------------------------------------
*** Handle last change. A very simple routine to send back a single integer
**/
void HandleLastChange(NsConn connection, int job_id,
		RmRequest *request, RmReply *reply)
{ 
  reply->FnRc	= RmE_Success;
  reply->Reply1	= LastChange;
  ReplyRmLib(connection, job_id, reply);
  request = request;
}
/*}}}*/
/*{{{  HandleIsProcessorFree() */
/**----------------------------------------------------------------------------
*** IsProcessorFree(), a simple routine for the Network Server
**/
void HandleIsProcessorFree(NsConn connection, int job_id, 
		RmRequest *request, RmReply *reply)
{ RmProcessor		processor;
  int			rc;
  int			state;
    
  processor = RmFindProcessor(Net, request->Uid);
  if (processor eq (RmProcessor) NULL)
   { rc = RmE_BadProcessor; goto done; }
  if (processor->ObjNode.Account ne RmO_FreePool)
   { rc = RmE_InUse; goto done; }
  state = RmGetProcessorState(processor);
  if ((state & RmS_Running) eq 0)
   { rc = RmE_InUse; goto done; }
  rc = RmE_Success;
  
done:
  reply->FnRc	= rc;
  ReplyRmLib(connection, job_id, reply);
}
/*}}}*/
/*{{{  HandleObtainProcessor() */
/**-----------------------------------------------------------------------------
*** Obtaining a processor, this involves the following stages.
***
*** 1) check the access allowed. At present only the Session Manager
***    and Taskforce Managers are allowed to obtain processors.
***    The Session Manager receives a capability from startns, and
***    passes it on to Taskforce Managers.
*** 2) allocate buffer space.
*** 3) there is a special case. In a single user environment where
***    the root processor may be shared between the user and the system,
***    the root processor should always be allocated to the Session Manager.
*** 4) if a particular PUID is specified for the processor, only that
***    processor should be allocated.
*** 5) otherwise it is necessary to search the network for a suitable
***    processor. A starting place is required. If the NEAR attribute
***    has been specified then that defines the starting place, with
***    a bit of environment (NEAR might be /Cluster/IO/window/console)
***    Otherwise the root processor is used for the starting place.
**/
/*{{{  ObtainProcessor_Filter() */
static	int	ObtainProcessor_Filter(RmProcessor real, RmProcessor copy)
{ ProcessorEntry	*proc_entry;

  proc_entry = GetProcEntry(real);
  copy->ObjNode.Key	= 0;
  copy->ObjNode.Parent	= Null(DirNode);
  copy->Root		= (RmNetwork) NULL;
  copy->StructType	= RmL_Obtained;
  NewCap(&(copy->NsCap), &(real->ObjNode), AccMask_R + AccMask_W + AccMask_D);
  memcpy(&(copy->RealCap), &(proc_entry->Owner), sizeof(Capability));
  copy->Private		= 0;
  return(RmE_Success);
}
/*}}}*/

void	HandleObtainProcessor(NsConn connection, int job_id, 
		RmRequest *request, RmReply *reply)
{ RmProcessor		template	= (RmProcessor) NULL;
  RmProcessor		result		= (RmProcessor) NULL;
  RmFilterStruct	filter;
  int			rc;
  RmProcessor		*search_table	= Null(RmProcessor);
  int			current_index	= 0;
  int			max_index	= 0;
  int			purpose;
  int			flags;

  Wait(&SingleAlloc);

  Debug(dbg_Allocate, ("obtain processor request"));
  template = request->Processor;  

  search_table		= (RmProcessor *) 
  		Malloc(NumberProcessors * sizeof(RmProcessor));
  if (search_table eq Null(RmProcessor))
   { rc = RmE_ServerMemory; goto done; }

  if (connection->Program eq Rm_Session)
   if ( (get_config("single_user", nsrc_environ) ne Null(char)) &&
          (get_config("share_root_processor", nsrc_environ) ne Null(char)) &&
          (RootProcessor->ObjNode.Account eq RmO_FreePool) &&
          (RmGetProcessorPurpose(RootProcessor) eq RmP_Helios) )
    { result = RootProcessor;
      goto found;
    }

  if (template->StructType eq RmL_Existing)
   { Debug(dbg_Allocate, ("request is for existing processor"));
     result = RmFindProcessor(Net, template->Uid);
     if (result eq (RmProcessor) NULL)
      { rc = RmE_NotFound; goto done; }
     if (result->ObjNode.Account ne RmO_FreePool)
      { result = (RmProcessor) NULL; rc = RmE_InUse; goto done; }
     purpose = RmGetProcessorPurpose(result);
     if (purpose & RmP_System)
      { result = (RmProcessor) NULL; rc = RmE_NoAccess; goto done; }
     rc = RmE_Success;
     goto found;
   }

  { char *puid = RmGetObjectAttribute((RmObject) template, "puid", FALSE);
    
    if (puid ne Null(char))
     { if (*puid eq '#')
	{ int uid = atoi(++puid);
	  Debug(dbg_Allocate, ("request is for existing processor"));
	  result = RmFindProcessor(Net, uid);
	}
       else
        { Debug(dbg_Allocate, ("request is for processor %s", puid));
          result = RmLookupProcessor(Net, puid);
	}
       if (result eq (RmProcessor)NULL)
        { rc = RmE_NotFound; goto done; }
       if (result->ObjNode.Account ne RmO_FreePool)
        { result = (RmProcessor) NULL; rc = RmE_InUse; goto done; }
       purpose = RmGetProcessorPurpose(result);
       if (purpose & RmP_System)
        { result = (RmProcessor) NULL; rc = RmE_NoAccess; goto done; }
       rc = RmE_Success;
       goto found;
     }
  }

  { char *near = (char *) RmGetProcessorAttribute(template, "NEAR");
    if (near eq Null(char))
     { if (RmGetProcessorPurpose(template) eq RmP_Native)
        search_table[max_index++] = LastBooted;
       else
        search_table[max_index++] = RootProcessor;
     }
    else
     { char	*temp = near + strlen(near);
       Debug(dbg_Allocate, ("request is for processor near %s", near));
       near++;
       forever
        { RmProcessor proc = RmLookupProcessor(Net, near);
          if (proc ne (RmProcessor) NULL)
           { search_table[max_index++] = proc;
	     (void) RmRemoveProcessorAttribute(template, &(near[-6]));
             break;
           }
           	/* strip off a bit of the name, and try to match again */
          for ( ; (temp > near) && (*temp ne '/'); temp--);
          if (*temp eq '/')
           { *temp = '\0'; continue; }
          else
           { rc = RmE_NotFound; goto done; }
        }
     }
  }

  Debug(dbg_Allocate, ("starting search for processor at %P",search_table[0]));
  		 
  for ( ; current_index < max_index; current_index++)
   { RmProcessor current = search_table[current_index];
     int		number_links;
     int		state;
     int		i, j;
     RmProcessor	neighbour;
     int		destlink;     
     int		purpose;
     
     purpose = RmGetProcessorPurpose(current);
     if ((purpose & RmP_Mask) eq RmP_Native) 
      goto check_links;
     if ((purpose & RmP_System) ||	/* cannot be allocated */
 	 (purpose eq RmP_IO) ||
 	 (purpose eq RmP_Router))
      goto check_links;
      
	/* if the processor is already owned it cannot be allocated */     
     if (current->ObjNode.Account ne RmO_FreePool) goto check_links;

	/* A suspicous processor is not allocated, but not ignored. */
	/* A crashed, dead or booting processor is ignored. */
     state = RmGetProcessorState(current);
     if (state & RmS_Suspicious) goto check_links;
     unless ((state & RmS_Running) && !(state & RmS_Booting)) continue;

     if (match_processor(template, current))     
      { result = current; goto found; }
      
check_links:
     number_links = RmCountLinks(current);
     for (i = 0; i < number_links; i++)
      { flags = RmGetLinkFlags(current, i);
        if ((flags & RmF_Suspicious) ne 0) continue;

        neighbour = RmFollowLink(current, i, &destlink);
        if ((neighbour eq RmM_NoProcessor) ||
            (neighbour eq RmM_ExternalProcessor))
         continue;

        for (j = 0; j < max_index; j++)
         if (neighbour eq search_table[j])
          break;

        if (j >= max_index)
         search_table[max_index++] = neighbour;
      }
   }
  
  result = (RmProcessor) NULL;
  rc	 = RmE_NotFound;
  Debug(dbg_Allocate, ("failed to find a suitable processor"));
  goto done;
      
found:
  Debug(dbg_Allocate, ("found processor %P", result));
  
  result->ObjNode.Account	= template->ObjNode.Account;
  result->SessionId		= connection->Id;

  { ProcessorEntry	*proc_entry;
    proc_entry = GetProcEntry(result);
    AddTail(&(connection->Processors), &(proc_entry->Connection));
    proc_entry->CommandDate = GetDate() - 1;
  }
  LastChange = result->ObjNode.Dates.Access	= GetDate();
  rc = RmE_Success;
  
done:  
  Signal(&SingleAlloc);

  reply->FnRc	= rc;
  if (rc eq RmE_Success)
   { filter.Processor	= &ObtainProcessor_Filter;
     reply->Filter	= &filter;
     reply->Processor	= result;
   }

  ReplyRmLib(connection, job_id, reply);

  if (search_table ne Null(RmProcessor)) Free(search_table);
}
/*}}}*/
/*{{{  HandleReleaseProcessor() */
/**
*** Releasing a processor. The other side will send a Uid and a capability,
*** which is enough to verify the data given. The processor is sent to
*** the cleaners.
**/

void	HandleReleaseProcessor(NsConn connection, int job_id, 
		RmRequest *request, RmReply *reply)
{ RmProcessor		processor;
  int			rc;

  Debug(dbg_Release, ("got request to release a processor"));

  processor = RmFindProcessor(Net, request->Uid);
  if (processor eq (RmProcessor) NULL)
   { rc = RmE_NotFound; goto done; }

  if (processor->SessionId ne connection->Id)
   { rc = RmE_NoAccess; goto done; }

  unless(GetAccess(&(request->Cap), processor->ObjNode.Key) &&
  	 (request->Cap.Access & AccMask_D))
   { rc = RmE_NoAccess; goto done; }

  Debug(dbg_Release, ("releasing processor %P", processor));

  { ProcessorEntry	*proc_entry;
    proc_entry = GetProcEntry(processor);
    (void) Remove(&(proc_entry->Connection));
    processor->SessionId	= -1;
  }
  SendToCleaners(processor);     
  processor->ObjNode.Key	= NewKey() + _cputime() + 
  	(int) processor + ((int) (&request) & 0x0F0F0FF0);
  LastChange = processor->ObjNode.Dates.Access = GetDate();
  
  rc = RmE_Success;

done:
  reply->FnRc	= rc;
  ReplyRmLib(connection, job_id, reply);
}
/*}}}*/
/*{{{  HandleObtainNetwork() */
/**----------------------------------------------------------------------------
*** Obtaining a whole network.
***
*** 1) do some housekeeping. Basically this involves reading in the
***    starting position from the search (which is ignored) and the template
***    network, and building a network hierarchy structure for the results
***    as they are produced.
*** 2) as a first step, walk down the template and look for existing processors
***    or processors with puid attributes. If these can be matched
***    move them from the template to the allocation unit.
*** 3) second step, try to match processors in the template. Again
***    following a match move the processor from the template to the result.
***    This involves a search through the network from the starting position,
***    as per ObtainProcessor.
*** 4) the results are processed.
BLV
BLV - this needs work, c.f. stable marriage etc.
**/

/*{{{  BuildHierarchy() */
/**
*** Make a copy of the network hierarchy. To avoid running out of memory at
*** an awkward moment, enough Uid tables are also allocated.
**/
static	int	BuildHierarchyAux(RmProcessor, ...);

static	RmNetwork	BuildHierarchy(void)
{ RmNetwork	result = RmNewNetwork();
  int		rc = RmE_Success;
  int		i, j;
  
  if (result eq (RmNetwork) NULL) return(result);
  strcpy(result->DirNode.Name, NetworkName);
  result->StructType = RmL_Obtained;
  
  { RmUidTableEntry	**tab;
    tab = (RmUidTableEntry **) Malloc(Net->NoTables * sizeof(RmUidTableEntry *));
    if (tab eq Null(RmUidTableEntry *))
     { RmFreeNetwork(result); return((RmNetwork) NULL); }
    for (i = 0; i < Net->NoTables; i++)
     { tab[i] = (RmUidTableEntry *) Malloc(RmL_SlotsPerTable * sizeof(RmUidTableEntry));
       if (tab[i] eq Null(RmUidTableEntry))
        { for (j = 0; j < i; j++) Free(tab[i]);
          Free(tab);
          RmFreeNetwork(result);
          return((RmNetwork) NULL);
        }
       for (j = 0; j < RmL_SlotsPerTable; j++)
        { (tab[i])[j].Cycle	= 0;
          (tab[i])[j].Free	= TRUE;
          (tab[i])[j].Target	= (void *) RmL_NoObject;
        }
     }
    result->NoTables	= Net->NoTables;
    result->Tables	= tab;
  }
  
  if (Net->NoSubnets > 0)
   rc = RmSearchNetwork(Net, &BuildHierarchyAux, result);
  if (rc ne RmE_Success)
   { RmFreeNetwork(result); return((RmNetwork)NULL); }
  else
   return(result);
}

static int BuildHierarchyAux(RmProcessor processor, ...)
{ va_list	args;
  RmNetwork	parent;
  RmNetwork	new;
  RmNetwork	actual;
    
  unless(RmIsNetwork(processor)) return(RmE_Success);
  va_start(args, processor);
  parent = va_arg(args, RmNetwork);
  va_end(args);

  actual	= (RmNetwork) processor;
  new		= RmNewNetwork();
  if (new eq (RmNetwork)NULL) return(RmE_ServerMemory);
  strcpy(new->DirNode.Name, Procname(processor));
  new->StructType = RmL_Obtained;
  if (RmAddtailProcessor(parent, (RmProcessor) new) eq (RmProcessor) NULL)
   { RmFreeNetwork(new); return(RmE_Corruption); }

  if (actual->NoSubnets > 0)
   return(RmSearchNetwork(actual, &BuildHierarchyAux, new));
  else
   return(RmE_Success);
}
/*}}}*/
/*{{{  MoveProcessor() */
/**
*** A suitable processor has been found. It must be moved to the
*** appropriate position in the result network, complete with attribute
*** information, link information, etc.
**/
static bool MoveProcessor(RmProcessor template, RmProcessor real, 
				RmNetwork result, NsConn connection)
{ RmProcessor		new_proc;
  ProcessorEntry	*proc_entry;

  new_proc = RmNewProcessor();  
  if (new_proc eq (RmProcessor) NULL) return(FALSE);

  proc_entry			= GetProcEntry(real);
  real->ObjNode.Account		= template->ObjNode.Account;
  real->SessionId		= connection->Id;
  memcpy(new_proc, real, sizeof(RmProcessorStruct));
  new_proc->ObjNode.Key		= 0;
  new_proc->StructType		= RmL_Obtained;
  new_proc->ObjNode.Parent	= Null(DirNode);
  new_proc->Root		= (RmNetwork) NULL;
  new_proc->MappedTo		= template->Uid;
  memcpy(&(new_proc->RealCap), &(proc_entry->Owner), sizeof(Capability));
  NewCap(&(new_proc->NsCap), &(real->ObjNode), AccMask_R + AccMask_W + AccMask_D);

  RmInsertProcessor(result, new_proc);

  RmRemoveProcessor(template);
  RmFreeProcessor(template);

  AddTail(&(connection->Processors), &(proc_entry->Connection));
  proc_entry->CommandDate = GetDate() - 1;
  Debug(dbg_Allocate, ("allocated processor %P", real));
  return(TRUE);   
}
/*}}}*/
/*{{{  Search1() */
/**
*** The first phase in the mapping algorithm. For every processor in the
*** template that already exists or that has a puid:
*** 1) if the processor is already is available, allocate it.
*** 2) If it is not available remove it from the template and get rid of it.
**/

static int	 ObtainNetwork_Search1(RmProcessor template, ...)
{ va_list	args;
  NsConn	connection;
  RmNetwork	result;
  
  va_start(args, template);
  connection = va_arg(args, NsConn);
  result     = va_arg(args, RmNetwork);
  va_end(args);
  
  if (template->StructType eq RmL_Existing)
   { RmProcessor match = RmFindProcessor(Net, template->Uid);

     Debug(dbg_Allocate, ("request is for existing processor %P", template));
     RmSetProcessorPrivate(template, 1);
     
     if (match eq (RmProcessor) NULL) return(0);
     if (match->ObjNode.Account eq RmO_FreePool)
      if (MoveProcessor(template, match, result, connection))
       return(1); 
     RmFreeProcessor(RmRemoveProcessor(template));
     return(1);
   }

  { char *puid = RmGetObjectAttribute((RmObject) template, "puid", FALSE);
    if (puid ne Null(char))
     { RmProcessor	match;

       if (*puid eq '#')
        { int uid = atoi(++puid);
	  Debug(dbg_Allocate, ("specific existing processor has been specified"));
          match = RmFindProcessor(Net, uid);
	}
       else
	{ Debug(dbg_Allocate, ("puid has been specified"));
          match = RmLookupProcessor(Net, puid);
	}

       RmSetProcessorPrivate(template, 1);
       if (match eq (RmProcessor) NULL) return(0);

       if (match->ObjNode.Account eq RmO_FreePool)       
        if (MoveProcessor(template, match, result, connection))
         return(1);
       RmFreeProcessor(RmRemoveProcessor(template));
       return(1);
     }
  }

  return(0);     
}
/*}}}*/
/*{{{  Search2() */
/**
*** Second phase of the search. For every available processor in the network 
*** this routine will be applied to every processor in the template, aborting
*** as soon as the request has been satisfied. This routine should match
*** the real processor with the template processor, and if successful
*** allocate the processor.
**/
static int	ObtainNetwork_Search2(RmProcessor template, ...)
{ va_list	args;
  RmProcessor	real;
  NsConn	connection;
  RmNetwork	result;
  
  va_start(args, template);
  real		= va_arg(args, RmProcessor);
  connection	= va_arg(args, NsConn);
  result	= va_arg(args, RmNetwork);
  va_end(args);
  
  if (match_processor(template, real))
   if (MoveProcessor(template, real, result, connection))
    return(1);
  return(0);
}
/*}}}*/
/*{{{  SearchNative() */
static int ObtainNetwork_SearchNative(RmProcessor processor, ...)
{ if (RmGetProcessorPurpose(processor) eq RmP_Native)
   return(1);
  else
   return(0);
}
/*}}}*/
/*{{{  AbortObtain() */
/**
*** Abort the network in question. Every processor in the specified network
*** has been allocated to this connection. This must be undone.
**/
static int	AbortObtainAux(RmProcessor, ...);

static void	AbortObtainNetwork(RmNetwork network, NsConn connection)
{ 
  (void) RmApplyProcessors(network, &AbortObtainAux, connection);
}

static int	AbortObtainAux(RmProcessor processor, ...)
{ va_list		args;
  NsConn		connection;
  RmProcessor		real;
  ProcessorEntry	*proc_entry;
      
  va_start(args, processor);
  connection = va_arg(args, NsConn);
  va_end(args);
  
  real = RmFindProcessor(Net, processor->Uid);
  real->ObjNode.Account	= RmO_FreePool;
  real->SessionId	= -1;
  proc_entry = GetProcEntry(real);
  Remove(&(proc_entry->Connection));
  return(0);
}
/*}}}*/
/*{{{  CleanoutResult() */
/**
*** Cleaning out the result. The above code has cheated by duplicating
*** various fields rather than making a new copy. To avoid horrible
*** problems this must now be undone before the network is freed.
**/
static int	CleanOutAux1(RmProcessor, ...);

static void CleanOutResult(RmNetwork network)
{ (void) RmApplyProcessors(network, &CleanOutAux1);
   RmFreeNetwork(network);
}

static int CleanOutAux1(RmProcessor processor, ...)
{ 
  processor->Connections	= 0;
  processor->OtherLinks		= Null(RmLink);
  processor->AttribSize		= 0;
  processor->AttribFree		= 0;
  processor->AttribData		= Null(char);
  processor->PAttribSize	= 0;
  processor->PAttribFree	= 0;
  processor->PAttribData	= Null(char);
  InitList(&(processor->MappedTasks));
  return(0);
}
/*}}}*/

void HandleObtainNetwork(NsConn connection, int job_id, 
		RmRequest *request, RmReply *reply)
{ int		number_to_match, number_to_get;
  int		start_from;
  int		rc;
  RmNetwork	template	= (RmNetwork) NULL;
  RmNetwork	result		= (RmNetwork) NULL;
  RmProcessor	*search_table	= Null(RmProcessor);
  RmProcessor	current;
  int		current_index	= 0;
  int		max_index	= 0;
  int		flags;
  bool		found_one	= FALSE;  

  Wait(&SingleAlloc);
  
  Debug(dbg_Allocate, ("got an allocation request for a network"));

  start_from	= request->Arg1;
  template	= request->Network;

  result = BuildHierarchy();
  if (result eq (RmNetwork) NULL)
   { rc = RmE_ServerMemory; goto done; }

  search_table = (RmProcessor *) Malloc(NumberProcessors * sizeof(RmProcessor));
  if (search_table eq Null(RmProcessor))
   { rc = RmE_ServerMemory; goto done; }
   
  number_to_get = number_to_match = RmCountProcessors(template);
  if (number_to_match eq 0)
   { rc = RmE_Success; goto done; }

  Debug(dbg_Allocate, ("request is for %d processors", number_to_match));
  Debug(dbg_Allocate, ("attempting to match with existing processors"));
     
  number_to_match -= RmApplyProcessors(template, &ObtainNetwork_Search1, connection, result);
  if (number_to_match eq 0) goto finished;

  if (start_from ne 0)
   { current = RmFindProcessor(Net, start_from);
     if (current eq (RmProcessor)NULL)
      current = RootProcessor;
   }
  else
   current = RootProcessor;

  if (RmSearchProcessors(template, &ObtainNetwork_SearchNative) ne 0)
   current = LastBooted;
   
  search_table[max_index++] = current;


  Debug(dbg_Allocate, ("searching the network, starting at %P", current));
    
  for ( ; current_index < max_index; current_index++)
   { int      		number_links;
     int		state;
     int		i, j;
     RmProcessor	neighbour;
     int		destlink;
     int		purpose;
     int		this_time;
          
     current = search_table[current_index];

     purpose = RmGetProcessorPurpose(current);
     if ((purpose & RmP_Mask) eq RmP_Native)
      goto check_links;
     if ((purpose & RmP_System) ||
         (purpose eq RmP_IO) ||
         (purpose eq RmP_Router))
      goto check_links;

     	/* if the processor is already owned it cannot be allocated */     
     if (current->ObjNode.Account ne RmO_FreePool) goto check_links;
      
	/* A suspicous processor is not allocated, but not ignored. */
	/* A crashed, dead or booting processor is ignored. */
     state = RmGetProcessorState(current);
     if (state & RmS_Suspicious) goto check_links;
     unless ((state & RmS_Running) && !(state & RmS_Booting)) continue;

     this_time = RmSearchProcessors(template, &ObtainNetwork_Search2,
     		current, connection, result);
     number_to_match -= this_time;
     if (number_to_match eq 0) goto finished;

	/* This piece of code restarts the breadth-first search when	*/
	/* the first suitable processor has been found. This should	*/
	/* reduce the amount of network fragmentation going on, at the	*/
	/* cost of at most doubling the time taken.			*/
     if ((!found_one) && (this_time > 0))
      { search_table[0] = current;
        current_index = 0;
        max_index = 1;
        found_one = TRUE;
      }
      
check_links:
     number_links = RmCountLinks(current);
     for (i = 0; i < number_links; i++)
      { flags = RmGetLinkFlags(current, i);
        if ((flags & RmF_Suspicious) ne 0) continue;

        neighbour = RmFollowLink(current, i, &destlink);
        if ((neighbour eq RmM_NoProcessor) ||
            (neighbour eq RmM_ExternalProcessor))
         continue;
        for (j = 0; j < max_index; j++)
         if (neighbour eq search_table[j])
          break;
        if (j >= max_index)
         search_table[max_index++] = neighbour;
      }
   }

finished:

  number_to_match = RmCountProcessors(result);
  if (number_to_match eq 0)
   { rc = RmE_NotFound; goto done; }
   
  number_to_get -= number_to_match;
  if (number_to_get eq 0)
   rc = RmE_Success;
  elif (request->FnRc ne RmC_ObtainExactNetwork)
   rc = RmE_PartialSuccess;
  else
   rc = RmE_NotFound;
   
done:
  reply->FnRc	= rc;
  if ((rc eq RmE_Success) || (rc eq RmE_PartialSuccess))
   reply->Network	= result;
  elif (result ne (RmNetwork) NULL)
    AbortObtainNetwork(result, connection);

  Signal(&SingleAlloc);

  ReplyRmLib(connection, job_id, reply);

  if (result ne (RmNetwork) NULL) CleanOutResult(result);
  if (search_table ne Null(RmProcessor)) Free(search_table);
}
/*}}}*/
/*{{{  HandleReleaseNetwork() */
/**-----------------------------------------------------------------------------
*** Releasing a network is just a case of looping and doing the same
*** sort of thing as release processor
**/
void HandleReleaseNetwork(NsConn connection, int job_id, 
		RmRequest *request, RmReply *reply)
{ int			rc = RmE_Success;
  ProcessorDetails	*details;
  RmProcessor		processor;
  ProcessorEntry	*proc_entry;
  int			i;

  details = (ProcessorDetails *) request->VariableData;

  for(i = 0; i < request->Arg1; i++)
   { 
     processor = RmFindProcessor(Net, details[i].Uid);
     if (processor eq (RmProcessor) NULL)
      { rc = RmE_NotFound; continue; }

     if (processor->SessionId ne connection->Id)
      { rc = RmE_NoAccess; continue; }

     unless(GetAccess(&(details[i].Cap), processor->ObjNode.Key) &&
            (details[i].Cap.Access & AccMask_D))
      { rc = RmE_NoAccess; continue; }
      
     proc_entry = GetProcEntry(processor);
     (void) Remove(&(proc_entry->Connection));
     processor->SessionId = -1;
     SendToCleaners(processor);
     processor->ObjNode.Key	= NewKey() + _cputime() + (int) processor +
     		((int) (&request) & 0x0F0F0FF0);
     LastChange = processor->ObjNode.Dates.Access = GetDate();
   }

  reply->FnRc	= rc;
  ReplyRmLib(connection, job_id, reply);
}
/*}}}*/

/*{{{  AutomaticRelease() */
/**
*** When a client shuts down a connection every processor obtained by
*** that client must be freed. This routine is called in a WalkList
**/
word AutomaticRelease(Node *node)
{ ProcessorEntry	*proc_entry;
  RmProcessor		processor;

  { BYTE *temp	= (BYTE *) node;
    temp	= temp - offsetof(ProcessorEntry, Connection);
    proc_entry	= (ProcessorEntry *) temp;
    processor	= proc_entry->Processor;
  }

  Debug((dbg_Release | dbg_Problem), ("automatic release for processor %P",processor));

  if (processor->ObjNode.Type ne Type_Processor)
   { 
     Debug((dbg_Release | dbg_Problem), ("processor is no longer in the network"));
     return(0);
   }

  Remove(&(proc_entry->Connection));
  processor->SessionId	= -1;
  
  if (RmGetProcessorState(processor) eq RmS_Crashed)
   { Debug((dbg_Release | dbg_Problem), ("processor has crashed"));
     return(0);
   }

  SendToCleaners(processor);     
  processor->ObjNode.Key	= GetDate() + _cputime() + 
  	(int) processor + ((int) (&proc_entry) & 0x0F0F0FF0);
  return(0);
}
/*}}}*/
/*{{{  the cleaners */
static void	CleaningProcess(int uid);

static void SendToCleaners(RmProcessor processor)
{ 
  processor->ObjNode.Account	= RmO_Cleaners;
  processor->SessionId		= -1;

  while (!Fork(Cleaning_Stack, &CleaningProcess, 4, processor->Uid))
   Delay(OneSec / 2);
}

static void CleaningProcess(int uid)
{ bool			started_agent	= FALSE;
  int  			rc		= Err_Null;
  NA_Message		message;
  int			purpose;
  RmProcessor		processor;
  ProcessorEntry	*proc_entry;
  int			 native_retries = 0;

back:

  MRSW_GetRead();

	/* Check that the processor has not disappeared while claiming	*/
	/* the lock.							*/
  processor = RmFindProcessor(Net, uid);
  if ((processor eq RmM_NoProcessor) || (processor eq RmM_ExternalProcessor))
   { MRSW_FreeRead(); return; }
  proc_entry = GetProcEntry(processor);

  purpose = RmGetProcessorPurpose(processor) & RmP_Mask;

	/* Under certain conditions it is necessary/desirable to reboot	*/
	/* this processor or do some tidying up.			*/
  if (purpose eq RmP_IO)
   goto done;
#if Native_Supported
  elif (purpose eq RmP_Native)
   CleanNative(processor);
  elif ((RmGetProcessorState(processor) & (RmS_Crashed | RmS_Dead)) ne 0)
   CleanNative(processor);
  else
   { int	number_links, i;
     RmLink	*current, *proper;
     number_links = RmCountLinks(processor);
     for (i = 0; i < number_links; i++)
      { current = RmFindLink(processor, i);
        proper = &(proc_entry->StandardLinks[i]);
        if (((current->Destination ne proper->Destination) ||
             (current->Target ne proper->Target)) &&
	     (proper->Target ne RmL_ExtUid))
         { CleanNative(processor); break; }
      }
   } 
#endif

	/* Processors which are native by default cannot be cleaned */
  if ((RmGetProcessorPurpose(processor) & RmP_Mask) eq RmP_Native)
   goto done;

  unless(RmGetProcessorState(processor) & RmS_Running)
   { if (++native_retries < 6)
      { Debug(dbg_Release, ("processor %P temporarily unavailable", processor));
        MRSW_FreeRead();
        Delay(10 * OneSec);
	goto back;
      }
     Debug(dbg_Release, ("processor %P has been lost", processor));
     RmSetProcessorState(processor, RmS_Crashed);
     processor->ObjNode.Account = RmO_Graveyard;
     MRSW_FreeRead();
     return;
   }
   
  unless (StartNetworkAgent(processor)) goto done;
  started_agent	= TRUE;

  message.FnRc	= NA_Clean;
  message.Arg1	= proc_entry->CommandDate;
  message.Size	= 0;
  rc = XchNetworkAgent(processor, &message, TRUE, 0, NULL);

done:
  if (rc ne Err_Null)
   report("warning, failed to clean out processor %P, fault %x",
   	processor, rc);

  if (started_agent)
   { StopNetworkAgent(processor); started_agent = FALSE; }
  if (rc eq Err_Null)
   processor->ObjNode.Account = RmO_FreePool;
  else
   processor->ObjNode.Account = RmO_Graveyard;

  MRSW_FreeRead();
}
/*}}}*/





