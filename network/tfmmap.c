/*------------------------------------------------------------------------
--                                                                      --
--           H E L I O S   N E T W O R K I N G   S O F T W A R E	--
--           ---------------------------------------------------	--
--                                                                      --
--             Copyright (C) 1990, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- tfmmap.c								--
--                                                                      --
--	This module of the Taskforce Manager deals with mapping		--
--	taskforces onto the available processors.			--
--                                                                      --
--	Author:  BLV 4/9/90						--
--                                                                      --
------------------------------------------------------------------------*/
/*$Header: /hsrc/network/RCS/tfmmap.c,v 1.7 1993/12/20 13:35:00 nickc Exp $*/

/*{{{  headers */
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
#include "exports.h"
#include "private.h"
#include "netutils.h"
#include "rmlib.h"
#include "tfmaux.h"
/*}}}*/
/*{{{  statics and module initialisation */
static int	AllocationId;
static void	SortDomain(RmNetwork);

void InitMap(void)
{ AllocationId = 1;
}
/*}}}*/
/*{{{  HandleObtainProcessor() */

/**----------------------------------------------------------------------------
*** ObtainProcessor(). This is where the fun starts.
***
*** 1) housekeeping, read the processor template from the client, allocate
***    any buffers required, check that the client is authorised to obtain
***    processors inside this TFM...
*** 2) if the request is for an existing processor, then this may or may
***    not be inside the current domain. If it is fine. Otherwise an
***    attempt is made to get the specified processor from the system pool.
*** 3) if the request contains a puid attribute life is much the same as
***    in step 2.
*** 4) an initial search is made in the domain to find a processor that
***    may be suitable. This search is quite restrictive. However, the
***    best match to date is remembered (this may be null for a weird request)
*** 5) if the search is unsuccessful an attempt is made to get another
***    processor from the system pool. If that succeeds fine.
*** 6) otherwise if the search revealed any suitable processor at all,
***    that processor is used. If the domain does not contain a suitable
***    processor and the system pool could not provide one, tough.
**/
static int		ObtainProcessor_filter(RmProcessor, RmProcessor);
static RmProcessor	GetFromPool(RmProcessor Template, int *rc);
static int		ObtainProcessorSearch(RmProcessor, ...);

void HandleObtainProcessor(TfmConn Connection, int JobId,
		RmRequest *request, RmReply *reply)
{ RmProcessor		Template	= (RmProcessor) NULL;
  RmProcessor		result		= (RmProcessor) NULL;
  RmFilterStruct	filter;
  DomainEntry		*domain_entry;
  int			rc;
  int			i;
  RmProcessor		LeastBusy;
  int			LowestCost;

  Debug(dbg_Allocate, ("request for a single processor"));
           
  Template = request->Processor;

  if (Template->StructType eq RmL_Existing)
   { 
     Debug(dbg_Allocate, ("request is for an existing processor"));
     result = RmFindProcessor(Domain, Template->Uid);
     if ((result eq RmM_NoProcessor) || (result eq RmM_ExternalProcessor))
      if (!DomainLocked)
       if ((result = GetFromPool(Template, &rc)) eq (RmProcessor)NULL)
        goto done;

     goto found;
   }

  { char *puid = RmGetObjectAttribute((RmObject) Template, "puid", FALSE);
    
    if (puid ne Null(char))
     { Debug(dbg_Allocate, ("puid has been specified"));
       result = RmLookupProcessor(Domain, puid);
       if (result eq (RmProcessor)NULL)
        if (!DomainLocked)
         if ((result = GetFromPool(Template, &rc)) eq (RmProcessor) NULL)
          { rc = RmE_NotFound; goto done; }
       goto found;
     }
  }

	/* No particular processor is required. Perform a search of the	*/
	/* current domain.						*/
   LeastBusy	= (RmProcessor) NULL;
   LowestCost	= (int) MaxInt;
   Debug(dbg_Allocate, ("searching current domain for a free processor"));

   result = (RmProcessor) RmSearchProcessors(Domain, &ObtainProcessorSearch,
   		Template, &LeastBusy, &LowestCost);
   if (result ne (RmProcessor) NULL) goto found;

   if (!DomainLocked)
    { result = GetFromPool(Template, &rc);
      if (result ne (RmProcessor) NULL) goto found;
    }
    
   if (LeastBusy eq (RmProcessor) NULL) goto done;
   Debug(dbg_Allocate, ("making do with an existing processor"));
   result = LeastBusy;
        
	/* At this point, result should point to a valid processor	*/
	/* that meets the user's requirements. This does not mean	*/
	/* it is safe to allocate...					*/
	/* BLV - check processor purpose as well */
found:

  Debug(dbg_Allocate, ("selected processor %P", result));

  domain_entry = GetDomainEntry(result);
  if (domain_entry->NumberUsers >= MaxUsersPerProcessor)
   { rc = RmE_InUse; goto done; }
  if ((result->AllocationFlags & RmF_Exclusive) &&
      (domain_entry->NumberUsers > 0))
   { rc = RmE_InUse; goto done; }
  for (i = 0; i < MaxUsersPerProcessor; i++)
   if (domain_entry->AllocationTable[i].Connection eq Connection)
    { rc = RmE_InUse; goto done; }
  for (i = 0; i < MaxUsersPerProcessor; i++)    
   if (domain_entry->AllocationTable[i].Id eq 0)
    { domain_entry->AllocationTable[i].Id		= AllocationId++;
      domain_entry->AllocationTable[i].Connection	= Connection;
      AddTail(	&(Connection->Processors),
      		&(domain_entry->AllocationTable[i].Node));
      break;
    }

  Debug(dbg_Allocate, ("successfully obtained a processor"));
  rc = RmE_Success;
  domain_entry->NumberUsers++;
  if (Template->AllocationFlags & RmF_Exclusive)
   { result->AllocationFlags	|= RmF_Exclusive;
     result->ApplicationId 	 = Connection->Id;
   }
  else
   result->ApplicationId	= -1;

  LastChange = result->ObjNode.Dates.Access = GetDate();
  
done:
  reply->FnRc = rc;
  if (rc eq RmE_Success)
   { filter.Processor	= &ObtainProcessor_filter;
     reply->Filter	= &filter;     
     reply->Processor	= result;
   }
  (void) ReplyRmLib(Connection, JobId, reply);
}

static	int	ObtainProcessor_filter(RmProcessor real, RmProcessor copy)
{ DomainEntry	*domain_entry;

  domain_entry = GetDomainEntry(real);
  copy->ObjNode.Key	= 0;
  copy->ObjNode.Parent	= Null(DirNode);
  copy->Root		= (RmNetwork) NULL;
  copy->StructType	= RmL_Obtained;
  NewCap(&(copy->NsCap), &(real->ObjNode), AccMask_R + AccMask_W + AccMask_D);
  copy->Private		= 0;
  return(RmE_Success);
}

static	int	ObtainProcessorSearch(RmProcessor Processor, ...)
{ va_list	args;
  RmProcessor	Template;
  DomainEntry	*domain_entry;
  RmProcessor	*LeastBusy;
  int		*LowestCost;
  int		cost_factor;
    
  va_start(args, Processor);
  Template	= va_arg(args, RmProcessor);
  LeastBusy	= va_arg(args, RmProcessor *);
  LowestCost	= va_arg(args, int *);
  va_end(args);
  
	/* Booked access means that the processor cannot be allocated	 */
	/* to meet an arbitrary request.				 */
  if (Processor->AllocationFlags & RmF_Booked) return(0);

	/* If the processor does not match the Template's requirements	*/
	/* forget it							*/  
  unless(MatchProcessor(Processor, Template)) return(0);

      	/* This processor appears to stand a chance. If the number of	*/
      	/* users is currently zero than this processor is ideal.	*/
  domain_entry = GetDomainEntry(Processor);
  if (domain_entry->NumberUsers eq 0) return((int) Processor);

 	/* If the number of users is greater than zero than it is	*/
 	/* necessary to calculate a cost factor for using this processor*/
 	/* This equation may need fine-tuning.				*/
  cost_factor = domain_entry->NumberUsers;
  if (RmGetProcessorMemory(Processor) > 0x100000) cost_factor--;
  if (cost_factor < *LowestCost)
   { *LeastBusy		= Processor;
     *LowestCost	= cost_factor;
   }

	/* However, this processor is not ideal and there may be a	*/
	/* better one.							*/
  return(0);
}

/*}}}*/
/*{{{  HandleReleaseProcessor() */
/**----------------------------------------------------------------------------
*** ReleaseProcessor(). A single set of Processor details will be sent.
*** If valid, the number of users for that processor is decremented and
*** the slot in the DomainEntry structure will be released. If the number
*** of users has decremented to zero then the processor may be returned to
*** the system pool.
**/
void HandleReleaseProcessor(TfmConn Connection, int JobId,
		RmRequest *request, RmReply *reply)
{ RmProcessor		Processor;
  DomainEntry		*domain_entry;
  int			rc;
  int			i;

  Debug(dbg_Release, ("release-processor request"));
    
  Processor = RmFindProcessor(Domain, request->Uid);
  if ((Processor eq RmM_NoProcessor) || (Processor eq RmM_ExternalProcessor))
   { rc = RmE_NotFound; goto done; }

  Debug(dbg_Release, ("processor affected is %P", Processor));
  unless(GetAccess(&(request->Cap), Processor->ObjNode.Key) &&
         (request->Cap.Access & AccMask_D))
   { rc = RmE_NoAccess; goto done; }

  LastChange = Processor->ObjNode.Dates.Access = GetDate();
  /* Processor->ApplicationId	= -1;*/
  
  domain_entry = GetDomainEntry(Processor);
  for (i = 0; i < MaxUsersPerProcessor; i++)
   if (domain_entry->AllocationTable[i].Connection eq Connection)
    { domain_entry->AllocationTable[i].Connection = (TfmConn) NULL;
      domain_entry->AllocationTable[i].Id	  = 0;
      Remove(&(domain_entry->AllocationTable[i].Node));
      break;
    }
  domain_entry->NumberUsers--;
  if (domain_entry->NumberUsers eq 0)
   { if ((Processor->AllocationFlags & (RmF_Permanent | RmF_TfmProcessor)) eq 0)
      ReturnProcessorToPool(Processor);
     else
      Processor->AllocationFlags &= ~RmF_Exclusive;
   }
  
  rc = RmE_Success;
         
done:
  reply->FnRc	= rc;
  ReplyRmLib(Connection, JobId, reply);
}
/*}}}*/
/*{{{  HandleObtainNetwork() */

/**----------------------------------------------------------------------------
*** ObtainNetwork(). This is the big one.
***
*** 1) do some housekeeping. Basically this involves reading in the
***    starting position from the search (which is ignored) and the template
***    network, and building a network hierarchy structure for the results
***    as they are produced.
*** 2) as a first step, walk down the template and look for existing processors
***    or processors with puid attributes. If these can be matched inside
***    the domain move them from the template to the allocation unit, zapping
***    the data structure. If the processor contains the NEW attribute then
***    step 3 should be skipped
*** 3) second step, try to match processors in the template with processors
***    in the domain (except the existing ones or ones with puid's). Again,
***    following a match move the processor from the template to the result
*** 4) the template now contains only processors which cannot be matched
***    satisfactorily with processors in the domain. A request is made to
***    the network server to get these from the system pool. Any processors
***    that matched can now be moved from the template to the result
*** 5) a final search is made in the template, to see if a less satisfactory
***    match can be found. I.e. is it possible to share certain processors
***    between users.
*** 6) the results are processed.
BLV
BLV the current algorithm only works for simple cases. What should happen
BLV is an initial search of the template, putting the processors into a
BLV linked list in order of how restrictive the template is. The processors
BLV should then be processed in that list order.
**/
static RmNetwork BuildHierarchy(void);
static int	 ObtainNetwork_Search1(RmProcessor, ...);
static int	 ObtainNetwork_Search2(RmProcessor, ...);
static int	 GetNetworkFromPool(RmNetwork, TfmConn, RmNetwork);
static int	 ObtainNetwork_Search3(RmProcessor, ...);
static void	 AbortObtainNetwork(RmNetwork, TfmConn);
static bool	 MoveProcessor(RmProcessor, RmProcessor, RmNetwork, TfmConn);
static void	 CleanOutResult(RmNetwork);
static bool	 CheckAvailable(RmProcessor real, TfmConn, RmProcessor);

static void ReallyObtainNetwork(TfmConn Connection, int JobId,
		RmRequest *request, RmReply *reply, RmNetwork Template);
	
void HandleObtainNetwork(TfmConn Connection, int JobId, 
		RmRequest *request, RmReply *reply)
{ 
  Debug(dbg_Allocate, ("request for a network of processors"));
  ReallyObtainNetwork(Connection, JobId, request, reply, request->Network);
}

void HandleObtainProcessors(TfmConn connection, int JobId, 
		RmRequest *request, RmReply *reply)
{ RmNetwork	Template = RmNewNetwork();
  int		count, i;
    
  Debug(dbg_Allocate, ("request for a group of processors"));

  count = request->Arg2;
  for (i = 0; i < count; i++)
   { RmProcessor	junk;
     if (RmReadProcessor(connection->Pipe_ctos, &junk, FALSE) ne RmE_Success)
      goto error;
     RmAddtailProcessor(Template, junk);
   }
  
  ReallyObtainNetwork(connection, JobId, request, reply, Template);
  return;

error:
	/* BLV - improve */
  Debug(dbg_Allocate, ("communication error"));
}

static	void ReallyObtainNetwork(TfmConn Connection, int JobId,
		RmRequest  *request, RmReply *reply, RmNetwork Template)
{ int		number_to_match;
  int		number_to_get;
  int		rc = RmE_Success;
  RmNetwork	result = (RmNetwork) NULL;

  result = BuildHierarchy();
  if (result eq (RmNetwork) NULL)
   { rc = RmE_ServerMemory; goto done; }
   
  number_to_get = number_to_match = RmCountProcessors(Template);
  if (number_to_match eq 0)
   { rc = RmE_Success; goto done; }

  Debug(dbg_Allocate, ("the request is for %d processors", number_to_match));
     
  number_to_match -= RmApplyProcessors(Template, &ObtainNetwork_Search1, Connection, result);
  if (number_to_match eq 0) goto finished;

  Debug(dbg_Allocate, ("examining domain for suitable processors"));
  number_to_match -= RmApplyProcessors(Template, &ObtainNetwork_Search2, Connection, result);
  if (number_to_match eq 0) goto finished;

  if (!DomainLocked)
   { Debug(dbg_Allocate, ("attempting to get some processors from the free pool"));
        number_to_match -= GetNetworkFromPool(Template, Connection, result);
     if (number_to_match eq 0) goto finished;
   }
   
  Debug(dbg_Allocate, ("trying to make do with existing processors"));
  number_to_match -= RmApplyProcessors(Template, &ObtainNetwork_Search3, Connection, result);

finished:

  number_to_match = RmCountProcessors(result);
  if (number_to_match eq 0)
   { rc = RmE_NotFound; goto done; }
   
  number_to_get -= number_to_match;
  if (number_to_get ne 0)
   { if ((request->FnRc eq RmC_ObtainExactNetwork) ||
         (request->FnRc eq RmC_ObtainExactProcessors))
      rc = RmE_NotFound;
     else
      rc = RmE_PartialSuccess;
   }

  Debug(dbg_Allocate, ("failed to match %d processors", number_to_get));
  
done:

  reply->FnRc	= rc;
  if ((rc eq RmE_Success) || (rc eq RmE_PartialSuccess))
   reply->Network = result;
  elif (result ne (RmNetwork) NULL)
   AbortObtainNetwork(result, Connection);
  ReplyRmLib(Connection, JobId, reply);
  if (result ne (RmNetwork) NULL) CleanOutResult(result);
}

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
    tab = (RmUidTableEntry **) Malloc((word) Domain->NoTables * sizeof(RmUidTableEntry *));
    if (tab eq Null(RmUidTableEntry *))
     { RmFreeNetwork(result); return((RmNetwork) NULL); }
    for (i = 0; i < Domain->NoTables; i++)
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
    result->NoTables	= Domain->NoTables;
    result->Tables	= tab;
  }
  
  if (Domain->NoSubnets > 0)
   rc = RmSearchNetwork(Domain, &BuildHierarchyAux, result);
  if (rc ne RmE_Success)
   { RmFreeNetwork(result); return((RmNetwork)NULL); }
  else
   return(result);
}

static int BuildHierarchyAux(RmProcessor Processor, ...)
{ va_list	args;
  RmNetwork	parent;
  RmNetwork	New;
  RmNetwork	actual;
    
  unless(RmIsNetwork(Processor)) return(RmE_Success);
  va_start(args, Processor);
  parent = va_arg(args, RmNetwork);
  va_end(args);

  actual	= (RmNetwork) Processor;
  New		= RmNewNetwork();
  if (New eq (RmNetwork)NULL) return(RmE_ServerMemory);
  strcpy(New->DirNode.Name, Processor->ObjNode.Name);
  New->StructType = RmL_Obtained;
  if (RmAddtailProcessor(parent, (RmProcessor) New) eq (RmProcessor) NULL)
   { RmFreeNetwork(New); return(RmE_Corruption); }

  if (actual->NoSubnets > 0)
   return(RmSearchNetwork(actual, &BuildHierarchyAux, New));
  else
   return(RmE_Success);
}

/**
*** The first phase in the mapping algorithm. For every processor in the
*** template that already exists or that has a puid:
*** 1) if the processor is already in the domain and is available, allocate
***    it.
*** 2) If it is not available remove it from the template and get rid of it.
*** 3) If it is not in the domain, leave it alone for a subsequent
***    RmObtainNetwork().
**/

static int	 ObtainNetwork_Search1(RmProcessor Template, ...)
{ va_list	args;
  TfmConn	Connection;
  RmNetwork	result;
  
  va_start(args, Template);
  Connection = va_arg(args, TfmConn);
  result     = va_arg(args, RmNetwork);
  va_end(args);
  
  Template->ObjNode.Account	= TfmProcessor->ObjNode.Account;
  
  RmSetProcessorPrivate(Template, 0);
  
  if (Template->StructType eq RmL_Existing)
   { RmProcessor match = RmFindProcessor(Domain, Template->Uid);

     Debug(dbg_Allocate, ("request for existing processor %P", Template));
     RmSetProcessorPrivate(Template, 1);
     
     if ((match eq RmM_NoProcessor) || (match eq RmM_ExternalProcessor))
      return(0);

     if (CheckAvailable(match, Connection, Template))
      if (MoveProcessor(Template, match, result, Connection))
       return(1); 
     RmFreeProcessor(RmRemoveProcessor(Template));
     return(1);
   }

  { char *puid = RmGetObjectAttribute((RmObject) Template, "puid", FALSE);
    if (puid ne Null(char))
     { RmProcessor match = RmLookupProcessor(Domain, puid);

       Debug(dbg_Allocate, ("puid has been specified"));
       RmSetProcessorPrivate(Template, 1);
       if (match eq (RmProcessor) NULL) return(0);
       
       if (CheckAvailable(match, Connection, Template))
        if (MoveProcessor(Template, match, result, Connection))
         return(1);
       RmFreeProcessor(RmRemoveProcessor(Template));
       return(1);
     }
  }

  if (RmGetObjectAttribute((RmObject) Template, "NEW", TRUE) ne Null(char))
   RmSetProcessorPrivate(Template, 1);

  return(0);     
}

/**
*** Phase 2 of the search. For every processor left in the template
*** that has not had its private field set to 1, search the domain
*** for a suitable processor. Only completely unused processors matching
*** the description are acceptable in this phase.
**/
static int	 ObtainNetwork_Search2Aux(RmProcessor, ...);

static int	 ObtainNetwork_Search2(RmProcessor Template, ...)
{ va_list	args;
  TfmConn	Connection;
  RmNetwork	result;
  RmProcessor	match;
  
  va_start(args, Template);
  Connection = va_arg(args, TfmConn);
  result     = va_arg(args, RmNetwork);
  va_end(args);
  
  if (RmGetProcessorPrivate(Template) eq 1) return(0);
  match = (RmProcessor) RmSearchProcessors(Domain, &ObtainNetwork_Search2Aux,
  		 Template, Connection);
  if (match eq (RmProcessor) NULL)
   return(0);
   
  if (MoveProcessor(Template, match, result, Connection))
   return(1);
  else
   return(0);
}

static	int ObtainNetwork_Search2Aux(RmProcessor real, ...)
{ va_list	args;
  RmProcessor	Template;
  TfmConn	Connection;
  DomainEntry	*domain_entry;

	/* booked processors cannot be allocated for this */
  if (real->AllocationFlags & RmF_Booked) return(0);
  
  va_start(args, real);
  Template   = va_arg(args, RmProcessor);
  Connection = va_arg(args, TfmConn);
  va_end(args);
  
  domain_entry = GetDomainEntry(real);
  if (domain_entry->NumberUsers > 0) return(0);
  
  unless(MatchProcessor(real, Template)) return(0);
  unless(CheckAvailable(real, Connection, Template)) return(0);
  return((int) real);
}

/**
*** The current domain is not big enough, so more processors have to be
*** obtained from the system pool. 
*** 1) Use what is left in the current template to get some more processors
*** 2) If more processors could be obtained, try to allocate DomainEntry
***    structures for them all.
*** 3) Then try to merge them in to the existing domain.
*** 4) for every processor in the template, if it was mapped onto a processor
***    in the result, allocate that processor.
**/   		

static int	GetNetworkFromPool_Aux1(RmProcessor, ...);
static int	GetNetworkFromPool_Aux2(RmProcessor, ...);
static int	GetNetworkFromPool_Aux3(RmProcessor, ...);

static int	 GetNetworkFromPool(RmNetwork Template, TfmConn Connection,
			RmNetwork result)
{ RmNetwork	obtained;
  int		rc;

  obtained = RmObtainNetwork(Template, FALSE, &rc);
  if (obtained eq (RmNetwork) NULL) return(0);

  Debug(dbg_Allocate, ("got some processors from the pool"));  
  if (RmSearchProcessors(obtained, &GetNetworkFromPool_Aux1) ne RmE_Success)
   { RmApplyProcessors(obtained, &GetNetworkFromPool_Aux2);
     RmReleaseNetwork(obtained);
     RmFreeNetwork(obtained);
     return(0);
   }
  Wait(&(Domain->DirNode.Lock));
  RmMergeNetworks(Domain, obtained);
  SortDomain(Domain);
  Signal(&(Domain->DirNode.Lock));
  RmFreeNetwork(obtained);
  return(RmApplyProcessors(Template, &GetNetworkFromPool_Aux3, Connection, result));
}

static int GetNetworkFromPool_Aux1(RmProcessor Processor, ...)
{ 
  if (!AddDomainEntry(Processor))
   return(1);

  Processor->ObjNode.Key = NewKey() + _cputime();
  LastChange = Processor->ObjNode.Dates.Access = GetDate();
  return(0);
}

static int GetNetworkFromPool_Aux2(RmProcessor Processor, ...)
{ DomainEntry	*domain_entry;

  domain_entry = GetDomainEntry(Processor);
  if (domain_entry ne Null(DomainEntry))
   Free(domain_entry);
  return(0);
}

static int GetNetworkFromPool_Aux3(RmProcessor Template, ...)
{ va_list	args;
  TfmConn	Connection;
  RmNetwork	result;
  RmProcessor	match;
  
  va_start(args, Template);
  Connection = va_arg(args, TfmConn);
  result     = va_arg(args, RmNetwork);
  va_end(args);
  
  match = RmFindMatchingProcessor(Template, Domain);
  if (match eq (RmProcessor) NULL) return(0);
  if (MoveProcessor(Template, match, result, Connection))
   return(1);
  else
   return(0);
}

/**
*** Final phase, similar to phase two but less restrictive.
**/
static int	 ObtainNetwork_Search3Aux(RmProcessor, ...);

static int	 ObtainNetwork_Search3(RmProcessor Template, ...)
{ va_list	args;
  TfmConn	Connection;
  RmNetwork	result;
  RmProcessor	match;

  va_start(args, Template);
  Connection = va_arg(args, TfmConn);
  result     = va_arg(args, RmNetwork);
  va_end(args);
  
  if (RmGetProcessorPrivate(Template) eq 1) return(0);
  match = (RmProcessor) RmSearchProcessors(Domain, &ObtainNetwork_Search3Aux,
  		 Template, Connection);
  if (match eq (RmProcessor) NULL)
   return(0);
   
  if (MoveProcessor(Template, match, result, Connection))
   return(1);
  else
   return(0);
}

static	int ObtainNetwork_Search3Aux(RmProcessor real, ...)
{ va_list	args;
  RmProcessor	Template;
  TfmConn	Connection;

	/* Booked means that the processor cannot be allocated	*/
	/* to meet an arbitrary request.			*/
  if (real->AllocationFlags & RmF_Booked) return(0);
	    
  va_start(args, real);
  Template   = va_arg(args, RmProcessor);
  Connection = va_arg(args, TfmConn);
  va_end(args);
  
  unless(MatchProcessor(real, Template)) return(0);
  unless(CheckAvailable(real, Connection, Template)) return(0);
  return((int) real);
}


/**
*** Abort the network in question. Every processor in the specified network
*** has been allocated to this connection. This must be undone.
**/
static int	AbortObtainAux(RmProcessor Processor, ...);

static void	AbortObtainNetwork(RmNetwork Network, TfmConn Connection)
{ Debug(dbg_Allocate, ("aborting network obtain request"));
  (void) RmApplyProcessors(Network, &AbortObtainAux, Connection);
}

static int	AbortObtainAux(RmProcessor Processor, ...)
{ va_list	args;
  TfmConn	Connection;
  DomainEntry	*domain_entry;
  int		i;
    
  va_start(args, Processor);
  Connection = va_arg(args, TfmConn);
  va_end(args);
  
  domain_entry = GetDomainEntry(Processor);
  for (i = 0; i < MaxUsersPerProcessor; i++)
   if (domain_entry->AllocationTable[i].Connection eq Connection)
    { domain_entry->AllocationTable[i].Connection	= (TfmConn) NULL;
      domain_entry->AllocationTable[i].Id		= 0;
      (void) Remove(&(domain_entry->AllocationTable[i].Node));
    }
  domain_entry->NumberUsers--;
  return(0);
}
  

/**
*** Check that a given processor can be allocated.
**/
static bool	CheckAvailable(RmProcessor match, TfmConn Connection,
		RmProcessor Template)     
{ DomainEntry *domain_entry;
  int	 i;

  domain_entry = GetDomainEntry(match);
  if (domain_entry->NumberUsers > 0)
   { if ((match->AllocationFlags & RmF_Exclusive) ||
         (Template->AllocationFlags & RmF_Exclusive))
      return(FALSE);
      
     if (domain_entry->NumberUsers >= MaxUsersPerProcessor)
      return(FALSE);
     for (i = 0; i < MaxUsersPerProcessor; i++)
      if (domain_entry->AllocationTable[i].Connection eq Connection)
       return(FALSE);
   }
   
  return(TRUE);
}

/**
*** A suitable processor has been found. It must be moved to the
*** appropriate position in the result network, complete with attribute
*** information, link information, etc.
**/
static bool MoveProcessor(RmProcessor Template, RmProcessor real, 
				RmNetwork result, TfmConn Connection)
{ RmProcessor	new_proc;
  DomainEntry	*domain_entry;
  int		i;

  new_proc = RmNewProcessor();  
  if (new_proc eq (RmProcessor) NULL) return(FALSE);

  Debug(dbg_Allocate, ("processor %P has been allocated", real));

  memcpy(new_proc, real, sizeof(RmProcessorStruct));  
  new_proc->ObjNode.Key		= 0;
  new_proc->StructType		= RmL_Obtained;
  new_proc->ObjNode.Parent	= Null(DirNode);
  new_proc->Root		= (RmNetwork) NULL;
  new_proc->MappedTo		= Template->Uid;
  NewCap(&(new_proc->NsCap), &(real->ObjNode), AccMask_R + AccMask_W + AccMask_D);
  InitList(&(new_proc->MappedTasks));

  RmInsertProcessor(result, new_proc);

  if (Template->AllocationFlags & RmF_Exclusive)
   { real->AllocationFlags	|= RmF_Exclusive;
     real->ApplicationId	 = Connection->Id;
     new_proc->ApplicationId	 = Connection->Id;
   }
  else
   real->ApplicationId = new_proc->ApplicationId = -1;

  RmRemoveProcessor(Template);
  RmFreeProcessor(Template);

  domain_entry	= GetDomainEntry(real);
  for (i = 0; i < MaxUsersPerProcessor; i++)
   if (domain_entry->AllocationTable[i].Id eq 0)
    { domain_entry->AllocationTable[i].Id		= AllocationId++;
      domain_entry->AllocationTable[i].Connection	= Connection;
      AddTail(	&(Connection->Processors),
      		&(domain_entry->AllocationTable[i].Node));
      break;
    }
  domain_entry->NumberUsers++;

  return(TRUE);   
}

/**
*** Cleaning out the result. The above code has cheated by duplicating
*** various fields rather than making a new copy. To avoid horrible
*** problems this must now be undone before the network is freed.
**/
static int	CleanOutAux1(RmProcessor, ...);

static void CleanOutResult(RmNetwork Network)
{
  (void) RmApplyProcessors(Network, &CleanOutAux1);
  RmFreeNetwork(Network);
}

static int CleanOutAux1(RmProcessor Processor, ...)
{ 
  Processor->Connections	= 0;
  Processor->OtherLinks		= Null(RmLink);
  Processor->AttribSize		= 0;
  Processor->AttribFree		= 0;
  Processor->AttribData		= Null(char);
  Processor->PAttribSize	= 0;
  Processor->PAttribFree	= 0;
  Processor->PAttribData	= Null(char);
  return(0);
}

/*}}}*/
/*{{{  HandleReleaseNetwork() */
/**----------------------------------------------------------------------------
*** Releasing a network is mostly a case of ReleaseProcessor() in a loop.
**/
void HandleReleaseNetwork(TfmConn Connection, int JobId,
		RmRequest *request, RmReply *reply)
{ int			rc = RmE_Success;
  ProcessorDetails	*details;
  RmProcessor		Processor;
  DomainEntry		*domain_entry;
  int			i, j;

  Debug(dbg_Release, ("release-network request"));
 
  details = (ProcessorDetails *) request->VariableData;

  for (i = 0; i < request->Arg1; i++)
   { Processor = RmFindProcessor(Domain, details[i].Uid);
     if ((Processor eq RmM_NoProcessor) ||(Processor eq RmM_ExternalProcessor))
      { rc = RmE_NotFound; continue; }

     Debug(dbg_Release, ("releasing processor %P", Processor));

     unless(GetAccess(&(details[i].Cap), Processor->ObjNode.Key) &&
            (details[i].Cap.Access & AccMask_D))
      { rc = RmE_NoAccess; continue; }

     LastChange = Processor->ObjNode.Dates.Access = GetDate();
     /* Processor->ApplicationId	= -1;*/
     
     domain_entry = GetDomainEntry(Processor);
     for (j = 0; j < MaxUsersPerProcessor; j++)
      if (domain_entry->AllocationTable[j].Connection eq Connection)
       { domain_entry->AllocationTable[j].Connection	= (TfmConn) NULL;
         domain_entry->AllocationTable[j].Id		= 0;
	 Remove(&(domain_entry->AllocationTable[j].Node));
         break;
       }
     domain_entry->NumberUsers--;
     if (domain_entry->NumberUsers eq 0)
      { if ((Processor->AllocationFlags & (RmF_Permanent | RmF_TfmProcessor))
      		 eq 0)
         ReturnProcessorToPool(Processor);
        else
         Processor->AllocationFlags &= ~RmF_Exclusive;
      }
   }

  reply->FnRc	= rc;
  ReplyRmLib(Connection, JobId, reply);
}
/*}}}*/
/*{{{  misc: SortDomain(), AddDomainEntry(), ... */

/**
*** Sort the current domain, thus ensuring that the processors within
*** the user's domain have the same or at least a very similar order to
*** the processors within the network. Given the fairly small size of a
*** domain a bubble-search should suffice.
**/
static void SortDomain(RmNetwork network)
{ RmProcessor	current;
  RmProcessor	next;
  bool		changes = TRUE;

  while (changes)
   { changes = FALSE;
     current = RmFirstProcessor(network);
     next    = RmNextProcessor(current);
     for ( ; next ne (RmProcessor) NULL; current = next, next = RmNextProcessor(current))
      { if (RmIsNetwork(current))
         { SortDomain((RmNetwork) current);
 	   continue;
         }
        if (RmIsNetwork(next))
         continue;
        if (current->Uid > next->Uid)
         { RmPreinsertProcessor(current, next);
           changes = TRUE;
           next = current;
	 }
      }
   }
}

/**
*** Add the DomainEntry structure to a newly-obtained processor
**/
bool	AddDomainEntry(RmProcessor Processor)
{ DomainEntry	*entry = New(DomainEntry);
  int		i;
  
  if (entry eq Null(DomainEntry)) return(FALSE);
  entry->NumberUsers		= 0;
  entry->Processor		= Processor;
  for (i = 0; i < MaxUsersPerProcessor; i++)
   { entry->AllocationTable[i].Processor	= Processor;
     entry->AllocationTable[i].Connection	= (TfmConn) NULL;
     entry->AllocationTable[i].Id		= 0;
   }
  RmSetProcessorPrivate(Processor, (int) entry);
  
  return(TRUE);
}


/**
*** When a client shuts down a connection every processor obtained by
*** that client must be freed. This routine is called in a WalkList
**/
word AutomaticRelease(Node *node)
{ DomainEntry		*domain_entry;
  RmProcessor		Processor;
  AllocationInfo	*info = (AllocationInfo *) node;

  Processor		= info->Processor;
  domain_entry		= GetDomainEntry(Processor);
  Debug(dbg_Release, ("automatic processor release activated on %P", Processor));

  domain_entry->NumberUsers--;
  info->Connection	= (TfmConn) NULL;
  info->Id		= 0;

  /*Processor->ApplicationId	= -1;*/
  
  if (domain_entry->NumberUsers eq 0)
   { if ((Processor->AllocationFlags & (RmF_Permanent | RmF_TfmProcessor)) eq 0)
      ReturnProcessorToPool(Processor);
     else
      Processor->AllocationFlags &= ~RmF_Exclusive;
   }
  return(0);
}

/**
*** Returning a processor to the pool involves removing it from the directory
*** structure, free'ing its domain info, and calling ReleaseProcessor()
**/
void	ReturnProcessorToPool(RmProcessor Processor)
{ DomainEntry	*domain_entry;

  if (DomainLocked) return;
  
  Debug(dbg_Release, ("returning processor %P to the system pool", Processor));  
  if (RmRemoveProcessor(Processor) eq (RmProcessor) NULL)
   { /* report("internal error returning processor %s to system pool",
   		Processor->ObjNode.Name); */
     return;
   }

  domain_entry = GetDomainEntry(Processor);
  Free(domain_entry);
  (void) RmReleaseProcessor(Processor);
  RmFreeProcessor(Processor);
}

/**
*** Getting a processor from the pool is another question entirely.
**/
static RmProcessor GetFromPool(RmProcessor Template, int *rc_ptr)
{ int		rc;
  RmProcessor	result;

  if (DomainLocked)
   { *rc_ptr = RmE_InUse; return((RmProcessor) NULL); }
   
  Debug(dbg_Allocate, ("attempting to get a processor from the free pool"));
      
  Template->ObjNode.Account	= TfmProcessor->ObjNode.Account;
  result = RmObtainProcessor(Template);
  if (result eq (RmProcessor) NULL) { rc = RmErrno; goto done; }
  Debug(dbg_Allocate, ("got a processor from the pool"));
  
  unless(AddDomainEntry(result))
   { RmReleaseProcessor(result); 
     RmFreeProcessor(result);
     rc = RmE_ServerMemory; 
     goto done; 
   }

  if (RmInsertProcessor(Domain, result) eq (RmProcessor) NULL)
   { DomainEntry *domain_entry;
     domain_entry = GetDomainEntry(result);
     Free(domain_entry);
     RmReleaseProcessor(result);
     RmFreeProcessor(result);
     rc = RmE_Corruption;
     goto done;
   }
  SortDomain(Domain);

  result->AllocationFlags = 0;
  if ((Template->AllocationFlags & RmF_Exclusive) ne 0)
   result->AllocationFlags |= RmF_Exclusive;

  LastChange = result->ObjNode.Dates.Access = GetDate();
  result->ObjNode.Key = NewKey() + _cputime();
  
  rc = RmE_Success;
     
done:
  *rc_ptr = rc;
  if (rc eq RmE_Success)
   return(result);
  else
   return((RmProcessor) NULL);
} 

/*}}}*/

/*{{{  domain_FillInTaskMapping() */
/**
*** Given a task and a processor that has been selected to run this task,
*** associate the task with that processor. This involves RmMapTask()
*** plus some additional work to update the TaskEntry and DomainEntry
*** fields, allowing the TFM to keep track of how many users there are
*** for every processor. 
**/
bool	domain_FillInTaskMapping(RmTask task, RmProcessor processor)
{ TaskEntry	*task_entry	= GetTaskEntry(task);
  DomainEntry	*domain_entry	= GetDomainEntry(processor);
  int		 i;

  if (domain_entry->NumberUsers >= MaxUsersPerProcessor)
   return(FALSE);

  for (i = 0; i < MaxUsersPerProcessor; i++)
   if (domain_entry->AllocationTable[i].Id eq 0)
    { domain_entry->AllocationTable[i].Id = AllocationId;
      break;
    }
  domain_entry->NumberUsers++;
  RmMapTask(processor, task);
  task_entry->Mapped = AllocationId++;
  return(TRUE);
}
/*}}}*/
/*{{{  domain_MapTask() */

/*------------------------------------------------------------------------------
***
*** This code deals with mapping tasks and taskforces automatically onto
*** the available domain, expanding the domain as required.
***
*** domain_MapTask() is based very closely on HandleObtainProcessor()
***
*** 1) if the request is for an existing processor, then this may or may
***    not be inside the current domain. If it is fine. Otherwise an
***    attempt is made to get the specified processor from the system pool.
*** 2) an initial search is made in the domain to find a processor that
***    may be suitable. This search is quite restrictive. However, the
***    best match to date is remembered (this may be null for a weird request)
*** 3) if the search is unsuccessful an attempt is made to get another
***    processor from the system pool. If that succeeds fine.
*** 4) otherwise if the search revealed any suitable processor at all,
***    that processor is used. If the domain does not contain a suitable
***    processor and the system pool could not provide one, tough.
**/
static int		MapTask_Search(RmProcessor, ...);

bool	domain_MapTask(RmTask task)
{ RmProcessor		Template	= (RmProcessor) NULL;
  RmProcessor		result		= (RmProcessor) NULL;
  int			rc;
  RmProcessor		LeastBusy;
  int			LowestCost;

  Debug(dbg_Mapping, ("mapping a single task"));
        
  { char *puid = RmGetObjectAttribute((RmObject) task, "puid", FALSE);
    
    if (puid ne Null(char))
     { Debug(dbg_Mapping, ("puid has been specified"));
       result = RmLookupProcessor(Domain, puid);
       if (result eq (RmProcessor)NULL)
        if (!DomainLocked)
         { Template = RmNewProcessor();
           if (Template eq (RmProcessor) NULL) goto done;
           RmAddProcessorAttribute(Template, &(puid[-5]));
           if ((result = GetFromPool(Template, &rc)) eq (RmProcessor) NULL)
            { rc = RmE_NotFound; goto done; }
         }
       goto found;
     }
  }

	/* No particular processor is required. Perform a search of the	*/
	/* current domain.						*/
   LeastBusy	= (RmProcessor) NULL;
   LowestCost	= (int) MaxInt;
   Debug(dbg_Mapping, ("searching current domain for a free processor"));

   result = (RmProcessor) RmSearchProcessors(Domain, &MapTask_Search,
   		task, &LeastBusy, &LowestCost);
   if (result ne (RmProcessor) NULL) goto found;

   if (LowestCost < 3)	/* tfm, shell, one other on same processor */
    { result = LeastBusy; goto found; }

   if (!DomainLocked)    
    { Debug(dbg_Mapping, ("attempting to get another processor from the pool"));
      Template = RmNewProcessor();
      if (Template ne (RmProcessor) NULL)
       { int	attribs;
         RmSetProcessorType(Template, RmGetTaskType(task));
         RmSetProcessorMemory(Template, RmGetTaskMemory(task));
         attribs = RmCountTaskAttributes(task);
         if (attribs > 0)
          { char	*default_attribs[10];
            char	**real_attribs;
            if (attribs > 10)
             real_attribs = (char **) Malloc((word) attribs * sizeof(char *));
            else
             real_attribs = default_attribs;
            if (real_attribs ne Null(char *))
             { int i;
               RmListTaskAttributes(task, real_attribs);
               for (i = 0; i < attribs; i++)
                RmAddProcessorAttribute(Template, real_attribs[i]);
               if (attribs > 10)
                Free(real_attribs);
             }    
          }
         result = GetFromPool(Template, &rc);
         if (result ne (RmProcessor) NULL) goto found;
       }
    }
    
   if (LeastBusy eq (RmProcessor) NULL) goto done;
   Debug(dbg_Mapping, ("making do with an existing processor"));
   result = LeastBusy;
        
	/* At this point, result should point to a valid processor	*/
	/* that meets the user's requirements. This does not mean	*/
	/* it is safe to allocate...					*/
found:

  Debug(dbg_Mapping, ("selected processor %P", result));
  if (domain_FillInTaskMapping(task, result))
   { Debug(dbg_Mapping, ("successfully obtained a processor"));
     rc = RmE_Success;
     LastChange = result->ObjNode.Dates.Access = GetDate();
   }
  else
   { Debug(dbg_Mapping, ("processor %P is overused", result));
     rc = RmE_InUse;
   }

done:
  if (Template ne (RmProcessor) NULL) RmFreeProcessor(Template);  
  return((rc eq RmE_Success) ? TRUE : FALSE);
}

static	int	MapTask_Search(RmProcessor Processor, ...)
{ va_list	args;
  RmTask	Template;
  DomainEntry	*domain_entry;
  RmProcessor	*LeastBusy;
  int		*LowestCost;
  int		cost_factor;
    
  va_start(args, Processor);
  Template	= va_arg(args, RmTask);
  LeastBusy	= va_arg(args, RmProcessor *);
  LowestCost	= va_arg(args, int *);
  va_end(args);
  
	/* Booked means that the processor cannot be allocated	*/
	/* to meet an arbitrary request.			*/
  if ((Processor->AllocationFlags & RmF_Booked) ne 0) return(0);

	/* If the processor does not match the Template's requirements	*/
	/* forget it							*/  
  unless(MatchTask(Processor, Template)) return(0);

      	/* This processor appears to stand a chance. If the number of	*/
      	/* users is currently zero than this processor is ideal.	*/
  domain_entry = GetDomainEntry(Processor);
  if (domain_entry->NumberUsers eq 0) return((int) Processor);

 	/* If the number of users is greater than zero than it is	*/
 	/* necessary to calculate a cost factor for using this processor*/
 	/* This equation may need fine-tuning.				*/
  cost_factor = domain_entry->NumberUsers;
  if (RmGetProcessorMemory(Processor) > 0x100000) cost_factor--;
  if (cost_factor < *LowestCost)
   { *LeastBusy		= Processor;
     *LowestCost	= cost_factor;
   }

	/* However, this processor is not ideal and there may be a	*/
	/* better one.							*/
  return(0);
}

/*}}}*/
/*{{{  domain_UnmapTask() and domain_UnmapTaskforce() */
/**----------------------------------------------------------------------------
*** Unmapping a task. This is a bit like HandleReleaseProcessor()
**/
void	domain_UnmapTask(RmTask task)
{ RmProcessor		processor;
  DomainEntry		*domain_entry;
  TaskEntry		*task_entry;
  int			i;

  Debug(dbg_Mapping, ("unmapping a task"));
  processor	= RmFollowTaskMapping(Domain, task);
  if (processor eq (RmProcessor) NULL) return;

  RmUnmapTask(processor, task);

  domain_entry	= GetDomainEntry(processor);
  task_entry	= GetTaskEntry(task);
  LastChange	= processor->ObjNode.Dates.Access = GetDate();  
  for (i = 0; i < MaxUsersPerProcessor; i++)
   if (domain_entry->AllocationTable[i].Id eq task_entry->Mapped)
    domain_entry->AllocationTable[i].Id	= 0;
  domain_entry->NumberUsers--;
  if (domain_entry->NumberUsers eq 0)
   { if ((processor->AllocationFlags & (RmF_Permanent | RmF_TfmProcessor)) eq 0)
      ReturnProcessorToPool(processor);
     else
      processor->AllocationFlags &= ~RmF_Exclusive;
   }

  task_entry->Mapped = 0;
}

/**
*** Releasing a taskforce. This is very similar to releasing a single task
**/
static int domain_UnmapTaskforceAux(RmTask task, ...);

void		domain_UnmapTaskforce(RmTaskforce taskforce)
{ (void) RmApplyTasks(taskforce, &domain_UnmapTaskforceAux);
}

static	int domain_UnmapTaskforceAux(RmTask task, ...)
{ 
  if (RmFollowTaskMapping(Domain, task) ne (RmProcessor) NULL)
   domain_UnmapTask(task);
  return(0);
}
/*}}}*/
/*{{{  domain_MapTaskforce() */

/*{{{  General description */

/**-----------------------------------------------------------------------------
*** Mapping a taskforce, this is where the fun starts.
***
*** 1) do some housekeeping.
*** 2) check the network passed as argument, if any. If the taskforce has
***    already been mapped partially or completely and all the required
***    processors are in the domain, the mapping is easy. If one or more
***    of the required processors is not yet in the domain life gets
***    a bit more difficult.
*** 3) next, walk down the template and look for 
***    tasks processors with puid attributes. If these can be matched inside
***    the domain move them from the template to the allocation unit, zapping
***    the data structure. 
*** 4) try to match tasks in the template with processors
***    in the domain (except the existing ones or ones with puid's). Again,
***    following a match assign the processor. This search is responsible
***    for making the mappings efficient if possible.
*** 5) the template now contains only tasks which cannot be matched
***    satisfactorily with processors in the domain. A request is made to
***    the network server to get these from the system pool.
*** 6) the taskforce is checked again for tasks with puid attributes or
***    tasks that were already mapped to processors. If the required
***    processor is still not in the domain then the mapping can be
***    aborted.
*** 7) step 4 is repeated. This allows for the case where there were no
***    specific requirements but the required processors had not been
***    pre-allocated.
*** 8) a check is made to ensure that the taskforce can be mapped at all.
***    In particular, for every task with particular requirements the
***    domain is searched for a processor with these requirements.
*** 9) a final search is made in the template, to see if a less satisfactory
***    match can be found. A manifest in tfmaux.h, set to 32 at the time of
***    writing, controls the maximum number of components permitted per
***    processor.
*** 9) the results are processed.
**/

/*}}}*/
/*{{{  forward declarations */
static int	MapTaskforce_SearchExisting(RmTask, ...);
static int	MapTaskforce_SearchExisting2(RmTask, ...);
static int	MapTaskforce_SearchPuid(RmTask, ...);
static int	MapTaskforce_SearchPuid2(RmTask, ...);
static int	MapTaskforce_MainSearch(RmTask, ...);
static int	MapTaskforce_Check(RmTask, ...);
static void	Map_GetNetworkFromPool(RmTaskforce);
static void	AbortMapTaskforce(RmTaskforce);
static bool	AllocProcessor(RmTask, RmProcessor);
static bool	Map_CheckAvailable(RmProcessor);

#define MappingFailure	1000000
/*}}}*/

/*{{{  MapTaskforce()     : the main routine */
bool		domain_MapTaskforce(RmNetwork network, RmTaskforce Taskforce)
{ int		number_to_match;
  int		limit;
    
  Debug(dbg_Mapping, ("mapping a taskforce"));
 
  number_to_match = RmCountTasks(Taskforce);
  if (number_to_match <= 0) goto finished;

  Debug(dbg_Mapping, ("the request is for %d tasks", number_to_match));

  if (network ne (RmNetwork) NULL)
   { number_to_match -= RmApplyTasks(Taskforce, &MapTaskforce_SearchExisting, network);
     Debug(dbg_Mapping, ("after comparing supplied network and the domain, %d tasks left", number_to_match));
     if (number_to_match <= 0) goto finished;
   }

  number_to_match -= RmApplyTasks(Taskforce, &MapTaskforce_SearchPuid);
  Debug(dbg_Mapping, ("after checking for puid's, %d tasks left", number_to_match));
  if (number_to_match <= 0) goto finished;

  Debug(dbg_Mapping, ("examining current domain for suitable processors"));
  number_to_match -= RmApplyTasks(Taskforce, &MapTaskforce_MainSearch, 0);
  Debug(dbg_Mapping, ("after checking for empty processors, %d tasks left", number_to_match));
  if (number_to_match <= 0) goto finished;

  if (!DomainLocked)
   { Debug(dbg_Mapping, ("attempting to get some processors from the free pool"));
     Map_GetNetworkFromPool(Taskforce);
   }
  else
   Debug(dbg_Mapping, ("domain is locked, cannot get additional processors"));

  Debug(dbg_Mapping, ("checking puids again"));
  number_to_match -= RmApplyTasks(Taskforce, &MapTaskforce_SearchExisting2);
  if (number_to_match <= 0) goto finished;
  number_to_match -= RmApplyTasks(Taskforce, &MapTaskforce_SearchPuid2);
  if (number_to_match <= 0) goto finished;

  Debug(dbg_Mapping, ("trying to make do with existing processors, limit 0"));
  number_to_match -= RmApplyTasks(Taskforce, &MapTaskforce_MainSearch, 0);
  if (number_to_match <= 0) goto finished;

  Debug(dbg_Mapping, ("checking taskforce feasibility"));
  if (RmSearchTasks(Taskforce, &MapTaskforce_Check) ne 0)
   goto finished;
   
  for (limit = 1; limit < MaxComponentsPerProcessor; limit++)
   { Debug(dbg_Allocate, ("trying to make do with existing processors, limit %d", limit));
     number_to_match -= RmApplyTasks(Taskforce, &MapTaskforce_MainSearch, limit);
     if (number_to_match <= 0) goto finished;
   }

finished:

  if (number_to_match ne 0)
   { Debug(dbg_Allocate, ("failed to map taskforce"));
     AbortMapTaskforce(Taskforce);
   }
  return(number_to_match eq 0);
}
/*}}}*/

/*{{{  SearchExisting     : check the supplied network */
/**
*** With the Resource Management library the application may have partially
*** or wholly mapped the taskforce onto the existing domain. Alternative
*** an RmLib binary object may be being executed, containing both a
*** network and a mapped taskforce. This routine matches the network supplied
*** with the current domain. If a task can be re-mapped immediately this
*** happens. Otherwise the task is unmapped but given a UID private
*** attribute, which will then be passed to the Network Server. Effectively
*** the same mechanisms are used as for PUID attributes.
**/
static int MapTaskforce_SearchExisting(RmTask task, ...)
{ RmNetwork	network;
  va_list	args;
  RmProcessor	supplied;
  RmProcessor	domain_proc;
  char		buf[32];

  va_start(args, task);
  network = va_arg(args, RmNetwork);
  va_end(args);

  if (task->MappedTo eq RmL_NoUid) return(0);
  supplied = RmFindProcessor(network, task->MappedTo);
  RmUnmapTask(supplied, task);
  if (supplied eq (RmProcessor) NULL)	/* What ??? */
   { report("internal error, received mapped task without a matching processor");
     task->MappedTo = RmL_NoUid;
     return(0);
   }

  domain_proc = RmFindProcessor(Domain, supplied->Uid);
  if (domain_proc ne (RmProcessor) NULL)
   { Debug(dbg_Create, ("component %T has been mapped already to processor %P",\
		task, domain_proc));
     if (AllocProcessor(task, domain_proc))
      return(1);
     else
      return(MappingFailure);
   }

	/* If the processor is not currently in the user's domain then	*/
	/* the taskforce probably came from a file, i.e. the user had	*/
	/* pre-mapped an application and written the mapping to a file.	*/
	/* The task is given a PUID attribute which will be passed on	*/
	/* to the Network Server.					*/
  strcpy(buf, "puid=#");
  addint(buf, supplied->Uid);
  RmAddTaskAttribute(task, buf);
  task->MappedTo = -1;
  return(0);  
}  

/**
*** SearchExisting2 : this is called after additional processors have been
*** obtained from the network. If the given task has a puid attribute of the
*** form #1234 and has not been mapped yet then there should now be
*** a processor with Uid 1234 in the domain. The puid attribute should be
*** removed at this point to avoid confusion.
**/
static int MapTaskforce_SearchExisting2(RmTask task, ...)
{ char 		*puid;
  int		 uid;
  RmProcessor	 match;

  if (task->MappedTo ne -1) return(0);
  puid = RmGetObjectAttribute((RmObject) task, "puid", FALSE);
  if ((puid eq Null(char)) || (*puid ne '#'))
   return(0);

  uid = atoi(&(puid[1]));
  RmRemoveTaskAttribute(task, &(puid[-5]));

  match = RmFindProcessor(Domain, uid);
  if (match eq (RmProcessor) NULL)
   { Debug(dbg_Create, ("failed to obtain mapped processor for component %T", task));
     return(MappingFailure);
   }
  if (AllocProcessor(task, match))
   return(1);
  else
   return(MappingFailure);
}

/*}}}*/
/*{{{  SearchPuid         : check for puid attributes */

/**
*** The second phase in the mapping algorithm.  For every task in the
*** template that has a puid attribute:
*** 1) if the processor is already in the domain and is available, allocate
***    it.
*** 2) If it is in the domain but not available, error
*** 3) If it is not in the domain, leave it alone for a subsequent
***    RmObtainNetwork().
**/

static int	 MapTaskforce_SearchPuid(RmTask task, ...)
{ char		*puid;
  RmProcessor	 match;

  if (task->MappedTo ne RmL_NoUid) return(0);

  puid = RmGetObjectAttribute((RmObject) task, "puid", FALSE);
  if ((puid ne Null(char)) && (*puid ne '#'))
   { Debug(dbg_Mapping, ("component %T, puid has been specified", task));
     match = RmLookupProcessor(Domain, puid);
     if (match eq (RmProcessor) NULL)
      { task->MappedTo = -1; return(0); }
       
     if (AllocProcessor(task, match))
      return(1);
     else
      return(MappingFailure);
   }
  return(0);     
}

static int MapTaskforce_SearchPuid2(RmTask task, ...)
{ char		*puid;
  RmProcessor	 match;

  if (task->MappedTo ne -1) return(0);
  puid = RmGetObjectAttribute((RmObject) task, "puid", FALSE);
  if (puid eq Null(char))
   return(0);

  match = RmLookupProcessor(Domain, puid);
  if ((match eq (RmProcessor) NULL) || !AllocProcessor(task, match))
   { Debug(dbg_Mapping, ("component %T, cannot satisfy PUID %s", task, puid));
     return(MappingFailure);
   }
  return(1);
}

/*}}}*/
/*{{{  MainSearch         : the general case */
/**
*** This routine is the heart of the taskforce mapping algorithm.
*** It is applied to every task in the taskforce. If the task has already
*** been mapped nothing more can be done. Otherwise the current domain
*** is searched for a processor that can run this task given the current
*** usage limit. 
***
*** The clever stuff happens when a match is found. At this point all
*** connected component tasks and all connected processors are considered.
*** If a match is found then the appropriate task and processor are mapped,
*** and the search continues from that match. Note that the search does
*** not continue with the same task, as the mapping performance for pipelines
*** and rings is better that way. The mapping never
*** backtracks. Furthermore the search is always linear rather than
*** breadth-first. Hence the mapping should still take a linear amount of
*** time.
**/
static int MapTaskforce_MainSearch_aux1(RmProcessor, ...);
static int MapTaskforce_MainSearch_continue(RmTask, RmProcessor, int limit);

static int MapTaskforce_MainSearch(RmTask task, ...)
{ va_list	args;
  int		usage_limit;
  RmProcessor	match;

  va_start(args, task);
  usage_limit = va_arg(args, int);
  va_end(args);

  if (task->MappedTo ne RmL_NoUid) return(0);

  match = (RmProcessor) RmSearchProcessors(Domain, &MapTaskforce_MainSearch_aux1, task, usage_limit);
  if (match eq (RmProcessor) NULL)
   return(0);

  return(1 + MapTaskforce_MainSearch_continue(task, match, usage_limit)); 
}

static int MapTaskforce_MainSearch_aux1(RmProcessor processor, ...)
{ va_list	args;
  RmTask	task;
  int		usage_limit;
  DomainEntry	*domain_entry;

  va_start(args, processor);
  task		= va_arg(args, RmTask);
  usage_limit	= va_arg(args, int);
  va_end(args);

  domain_entry = GetDomainEntry(processor);

  if (   (domain_entry->NumberUsers > usage_limit) ||
	! Map_CheckAvailable(processor) ||
	! MatchTask(processor,task) ||
        ! AllocProcessor(task, processor))
   return(0);

  return((int) processor);
}

static MapTaskforce_MainSearch_continue(RmTask task, RmProcessor processor, int usage_limit)
{ RmProcessor	 neighbour_processor;
  RmTask	 neighbour_task;
  int		 number_links, link, destlink;
  int		 number_channels, channel, destchannel;
  RmTask	 current_task;
  RmProcessor 	 current_processor;
  DomainEntry	*domain_entry;
  int		 result = 0;

  while (task ne (RmTask) NULL)
   { current_task	= task;
     current_processor	= processor;
     task 		= (RmTask) NULL;
     processor		= (RmProcessor) NULL;

     number_channels	= RmCountChannels(current_task);
     number_links	= RmCountLinks(current_processor);
     
     for (channel = 0; channel < number_channels; channel++)
      { neighbour_task = RmFollowChannel(current_task, channel, &destchannel);
        if ((neighbour_task eq RmM_NoTask) ||
	    (neighbour_task eq RmM_ExternalTask) ||
	    (neighbour_task->MappedTo ne RmL_NoUid))
         continue;

	for (link = 0; link < number_links; link++)
	 { neighbour_processor = RmFollowLink(current_processor, link, &destlink);
	   if ((neighbour_processor eq RmM_NoProcessor) ||
	       (neighbour_processor eq RmM_ExternalProcessor))
	    continue;

	   domain_entry = GetDomainEntry(neighbour_processor);
	   if ( (domain_entry->NumberUsers > usage_limit) ||
		!Map_CheckAvailable(neighbour_processor)  ||
		!MatchTask(neighbour_processor, neighbour_task) ||
		!AllocProcessor(neighbour_task, neighbour_processor))
	    continue;
	   else
	    { result++;
	      if (task eq (RmTask) NULL)
	       { task = neighbour_task; processor = neighbour_processor; }
	      goto done_task;
	    }

	 }	/* for each link		*/
      }		/* for each channel		*/
done_task:
    task = task;
   }		/* while partial success	*/
  return(result);
}
  
/*}}}*/
/*{{{  MapTaskforce_Check : check the feasibility of mapping this taskforce */
/**
*** Check that the taskforce can be mapped at all.
**/

static int MapTaskforce_CheckAux(RmProcessor, ...);

static int MapTaskforce_Check(RmTask task, ...)
{
  if (task->MappedTo > 0) return(0);
  
  if (RmSearchProcessors(Domain, &MapTaskforce_CheckAux, task) eq 0)
   { Debug(dbg_Mapping, ("cannot map %T in the current domain", task));
     return(1);
   }
  else 
   return(0);
}  

static int MapTaskforce_CheckAux(RmProcessor processor, ...)
{ va_list	args;
  RmTask	task;
  
  va_start(args, processor);
  task = va_arg(args, RmTask);
  va_end(args);
  
  if ((Map_CheckAvailable(processor)) &&
      (MatchTask(processor, task)))
   return(1);
  else
   return(0);
}
/*}}}*/
/*{{{  GetNetworkFromPool : request additional resources */

/**
*** The current domain is not big enough, so more processors have to be
*** obtained from the system pool. 
*** 1) build a template to get some more processors
*** 2) If more processors could be obtained, try to allocate DomainEntry
***    structures for them all.
*** 3) Then try to merge them in to the existing domain.
**/   		
static int	Map_BuildTemplate(RmTask Task, ...);

static void	 Map_GetNetworkFromPool(RmTaskforce Taskforce)
{ RmNetwork	Template = RmNewNetwork();
  RmNetwork	obtained;
  int		rc;
  
  if (Template eq (RmNetwork) NULL) goto fail;

  Debug(dbg_Mapping, ("building network template"));
  
  if (RmSearchTasks(Taskforce, &Map_BuildTemplate, Template) ne RmE_Success)
   goto fail;
   
  obtained = RmObtainNetwork(Template, FALSE, &rc);
  RmFreeNetwork(Template);
  if (obtained eq (RmNetwork) NULL) goto fail;

  Debug(dbg_Mapping, ("got some processors from the pool"));  
  if (RmSearchProcessors(obtained, &GetNetworkFromPool_Aux1) ne RmE_Success)
   { RmApplyProcessors(obtained, &GetNetworkFromPool_Aux2);
     RmReleaseNetwork(obtained);
     RmFreeNetwork(obtained);
     goto fail;
   }
  Wait(&(Domain->DirNode.Lock));
  RmMergeNetworks(Domain, obtained);
  SortDomain(Domain);
  Signal(&(Domain->DirNode.Lock));

  RmFreeNetwork(obtained);
  return;

fail:
  Debug(dbg_Mapping, ("failed to get additional processors from free pool"));
}

static int Map_BuildTemplate(RmTask Task, ...)
{ va_list	args;
  RmNetwork	Template;
  RmProcessor	New;
  int		attribs;
    
  va_start(args, Task);
  Template = va_arg(args, RmNetwork);
  va_end(args);
  
  if (Task->MappedTo > 0) return(0);

  New = RmNewProcessor();
  if (New eq (RmProcessor) NULL) return(1);
  New->ObjNode.Account	= TfmProcessor->ObjNode.Account;
  
  if (Task->MappedTo eq -1)
   { char *puid = RmGetObjectAttribute((RmObject) Task, "puid", FALSE);
     if ((puid eq Null(char)) || 
         (RmAddProcessorAttribute(New, &(puid[-5])) ne RmE_Success) ||
         (RmAddtailProcessor(Template, New) eq (RmProcessor) NULL) )
      { RmFreeProcessor(New); return(1); }
     return(0);      
   }
     
  RmSetProcessorType(New, RmGetTaskType(Task));
  RmSetProcessorMemory(New, RmGetTaskMemory(Task));
  attribs = RmCountTaskAttributes(Task);
  if (attribs > 0)
   { char	*default_attribs[10];
     char	**real_attribs;
     int	i;
     
     if (attribs > 10)
      real_attribs = (char **) Malloc((word) attribs * sizeof(char *));
     else
      real_attribs = default_attribs;
     if (real_attribs eq Null(char *))
      { RmFreeProcessor(New); return(1); }
     RmListTaskAttributes(Task, real_attribs);
     for (i = 0; i < attribs; i++)
      RmAddProcessorAttribute(New, real_attribs[i]);
     if (attribs > 10)
      Free(real_attribs);
   } 
  if (RmAddtailProcessor(Template, New) eq (RmProcessor) NULL)
   { RmFreeProcessor(New); return(1); }
  else
   return(0);
}

/*}}}*/
/*{{{  AbortMapTaskforce  : this taskforce cannot be mapped */
/**
*** Abort the mapping. Some of the tasks in the taskforce may have been
*** mapped already. This must now be undone.
**/
static int	AbortMapTaskforceAux(RmTask Task, ...);

static void	AbortMapTaskforce(RmTaskforce Taskforce)
{ Debug(dbg_Mapping, ("aborting mapping"));
  (void) RmApplyTasks(Taskforce, &AbortMapTaskforceAux);
}

static int	AbortMapTaskforceAux(RmTask Task, ...)
{ RmProcessor	mapped_to;
  TaskEntry	*task_entry;
  DomainEntry	*domain_entry;
  int		i;

  mapped_to = RmFollowTaskMapping(Domain, Task);
  if (mapped_to eq RmM_NoProcessor) return(0);

  RmUnmapTask(mapped_to, Task);
  domain_entry = GetDomainEntry(mapped_to);
  task_entry   = GetTaskEntry(Task);  
  for (i = 0; i < MaxUsersPerProcessor; i++)
   if (domain_entry->AllocationTable[i].Id eq task_entry->Mapped)
    { domain_entry->AllocationTable[i].Id = 0; break; }
  domain_entry->NumberUsers--;
  task_entry->Mapped = 0;
  if (domain_entry->NumberUsers eq 0)
   if ((mapped_to->AllocationFlags & (RmF_Permanent || RmF_TfmProcessor)) eq 0)
    ReturnProcessorToPool(mapped_to);
  return(0);
}
/*}}}*/
/*{{{  CheckAvailable     : is a particular processor available */
/**
*** Check that a given processor can be allocated
**/
static bool	Map_CheckAvailable(RmProcessor match)
{ DomainEntry *domain_entry;

  if (match->AllocationFlags & RmF_Booked) return(FALSE);
  domain_entry = GetDomainEntry(match);
  if (domain_entry->NumberUsers >= MaxUsersPerProcessor) return(FALSE);
  if (domain_entry->NumberUsers > 0)
   if (match->AllocationFlags & RmF_Exclusive)
    return(FALSE);
  return(TRUE);
}
/*}}}*/
/*{{{  AllocProcessor     : map a task to a processor */

/**
*** A suitable processor has been found.
**/
static bool AllocProcessor(RmTask Template, RmProcessor match)
{ DomainEntry	*domain_entry;
  TaskEntry	*task_entry;
  int		i;

  Debug(dbg_Mapping, ("using processor %P for %T", match, Template));

  domain_entry	= GetDomainEntry(match);
  for (i = 0; i < MaxUsersPerProcessor; i++)
   if (domain_entry->AllocationTable[i].Id eq 0)
    { domain_entry->AllocationTable[i].Id = AllocationId;
      domain_entry->NumberUsers++;
      task_entry		= GetTaskEntry(Template);
      task_entry->Mapped	= AllocationId++;
      Template->MappedTo	= 0;	/* could have been -1 if PUID specified */
      RmMapTask(match, Template);
      return(TRUE);   
    }
  Debug(dbg_Mapping, ("failure, processor %P is overused", match));
  return(FALSE);
}

/*}}}*/

/*}}}*/





