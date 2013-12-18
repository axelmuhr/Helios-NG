/*------------------------------------------------------------------------
--                                                                      --
--           H E L I O S   N E T W O R K I N G   S O F T W A R E	--
--           ---------------------------------------------------	--
--                                                                      --
--             Copyright (C) 1990 - 1993, Perihelion Software Ltd.      --
--                        All Rights Reserved.                          --
--                                                                      --
-- rmlib3.c								--
--                                                                      --
--	The ``real work'' module. This module provides the routines	--
--	that actually interact with servers, using the facilities	--
--	provided by rmlib2.c, and the data structures provided by	--
--	rmlib1.c.							--
--                                                                      --
--	Author:  BLV 1/5/90						--
--                                                                      --
------------------------------------------------------------------------*/
/* RcsId: $Header: /hsrc/network/RCS/rmlib3.c,v 1.12 1993/12/08 17:57:24 nickc Exp $*/

#define in_rmlib	1

/*{{{  Headers */
#if defined(__SUN4) || defined(RS6000)
#include </hsrc/include/memory.h>
#include </hsrc/include/link.h>
#define _link_h
#endif
#include <syslib.h>
#include <stdarg.h>
#include <string.h>
#include <root.h>
#include <posix.h>
#include <gsp.h>
#include <nonansi.h>
#include <stddef.h>

#include "exports.h"
#include "private.h"
#include "rmlib.h"

#ifdef Malloc		/* courtesy of servlib.h */
#undef Malloc
#endif
/*}}}*/
/*{{{  Compile-time options */

#ifdef __TRAN
#pragma -f0		/* 0 == disable vector stack			*/
#pragma -g0		/* remove names from code			*/
#endif

#ifdef STACKCHECK
#pragma	-s0
#else
#pragma -s1
#endif
/*}}}*/
/*{{{  The current network */
RmNetwork	RmGetNetwork(void)
{ int		rc;
  RmRequest	request;
  RmReply	reply;

  Clear(request); Clear(reply);
  request.FnRc = RmC_GetNetwork;
  rc = RmXch(&RmNetworkServer, &request, &reply);
  if (rc ne RmE_Success) RmErrno = rc;
  return(reply.Network);
}

RmNetwork	RmGetNetworkAndHardware(void)
{ int		rc;
  RmRequest	request;
  RmReply	reply;

  Clear(request); Clear(reply);
  request.FnRc = RmC_GetNetworkHardware;
  rc = RmXch(&RmNetworkServer, &request, &reply);
  if (rc ne RmE_Success) RmErrno = rc;
  return(reply.Network);
}

RmNetwork	RmGetNetworkHierarchy(void)
{ int		rc;
  RmRequest	request;
  RmReply	reply;

  Clear(request); Clear(reply);
  request.FnRc = RmC_GetHierarchy;
  rc = RmXch(&RmNetworkServer, &request, &reply);
  if (rc ne RmE_Success) RmErrno = rc;
  return(reply.Network);
}

RmNetwork	RmGetDomain(void)
{ int		rc;
  RmRequest	request;
  RmReply	reply;

  Clear(request); Clear(reply);
  request.FnRc = RmC_GetNetwork;
  rc = RmXch(&RmParent, &request, &reply);
  if (rc ne RmE_Success) RmErrno = rc;
  return(reply.Network);
}
/*}}}*/
/*{{{  Time stamps */
int		RmLastChangeDomain(void)
{ int		rc;
  RmRequest	request;
  RmReply	reply;

  Clear(request); Clear(reply);
  request.FnRc = RmC_LastChange;
  rc = RmXch(&RmParent, &request, &reply);
  if (rc ne RmE_Success)
   { RmErrno = rc; return(-1); }
  else
   return(reply.Reply1);
}

int		RmLastChangeNetwork(void)
{ int		rc;
  RmRequest	request;
  RmReply	reply;

  Clear(request); Clear(reply);
  request.FnRc = RmC_LastChange;
  rc = RmXch(&RmNetworkServer, &request, &reply);
  if (rc ne RmE_Success)
   { RmErrno = rc; return(-1); }
  else
   return(reply.Reply1);
}
/*}}}*/
/*{{{  Obtaining and releasing single processor */

/**
*** Obtaining a processor. This involves sending a template to the
*** parent server, i.e. to the Taskforce Manager or the Network
*** server.
**/
RmProcessor	RmObtainProcessor(RmProcessor Template)
{ int		rc;
  RmRequest	request;
  RmReply	reply;

  CheckProcessorFail(Template, (RmProcessor) NULL);

  if ((Template->StructType ne RmL_Existing) &&
      (Template->StructType ne RmL_New))
   { RmErrno = Template->Errno = RmE_BadArgument; return((RmProcessor)NULL); }

  Clear(request); Clear(reply);
  request.FnRc		= RmC_ObtainProcessor;
  request.Processor	= Template;

  rc = RmXch(&RmParent, &request, &reply);
  if (rc ne RmE_Success) RmErrno = Template->Errno = rc;
  return(reply.Processor);
}

/**
*** releasing a processor. This involves sending a packet to the parent,
*** giving the processor uid and capability. Also, once the processor
*** has been released the structure type must change.
**/
int		RmReleaseProcessor(RmProcessor processor)
{ int			rc;
  RmRequest		request;
  RmReply		reply;

  CheckProcessor(processor);
  if (processor->StructType ne RmL_Obtained) 
   return(RmErrno = processor->Errno = RmE_NoAccess);

  Clear(request); Clear(reply);
  request.FnRc	= RmC_ReleaseProcessor;
  request.Uid	= processor->Uid;
  request.Cap	= processor->NsCap;
  rc = RmXch(&RmParent, &request, &reply);
  if (rc ne RmE_Success)
   RmErrno = processor->Errno = rc;
  else
   processor->StructType = RmL_Existing;
  return(rc);
}

bool	RmIsProcessorFree(RmProcessor processor)
{ int		rc;
  RmRequest	request;
  RmReply	reply;

  CheckProcessorFail(processor, FALSE);
  if ((processor->StructType ne RmL_Existing) &&
      (processor->StructType ne RmL_Obtained))
   return(processor->Errno = RmErrno = RmE_BadProcessor);

  Clear(request); Clear(reply);
  request.FnRc	= RmC_IsProcessorFree;
  request.Uid	= processor->Uid;

  rc = RmXch(&RmParent, &request, &reply);

  if (rc eq RmE_Success)
   return(TRUE);
  elif (rc eq RmE_InUse)
   return(FALSE);
  else
   return(processor->Errno = RmErrno = rc, FALSE);
}

/*}}}*/
/*{{{  Processor allocation options */
/**
*** Manipulating processor and network allocation strategies. These
*** only happen between clients and the Taskforce Manager
**/
static		int RmSetProcessorAllocation(int fnrc, RmProcessor processor)
{ int		rc;
  RmRequest	request;  
  RmReply	reply;

  Clear(request); Clear(reply);

  request.FnRc	= fnrc;
  request.Uid	= processor->Uid;
  request.Cap	= processor->NsCap;
  rc = RmXch(&RmParent, &request, &reply);
  if (rc ne RmE_Success) RmErrno = processor->Errno = rc;
  return(rc);     
}

int		RmSetProcessorShareable(RmProcessor processor)
{ 
  CheckProcessor(processor);

  if ((processor->StructType eq RmL_New) || 
      (processor->StructType eq RmL_Existing))
   { processor->AllocationFlags &= ~RmF_Exclusive;
     return(RmE_Success);
   }
  if (processor->StructType ne RmL_Obtained)
   return(RmErrno = processor->Errno = RmE_NoAccess);

  return(RmSetProcessorAllocation(RmC_ProcessorShareable, processor));
}

int		RmSetProcessorExclusive(RmProcessor processor)
{
  CheckProcessor(processor);

  if ((processor->StructType eq RmL_New) ||
      (processor->StructType eq RmL_Existing))
   { processor->AllocationFlags |= RmF_Exclusive;
     return(RmE_Success);
   }
  if (processor->StructType ne RmL_Obtained)
   return(RmErrno = processor->Errno = RmE_NoAccess);

  return(RmSetProcessorAllocation(RmC_ProcessorExclusive, processor));   
}

bool	RmIsProcessorShareable(RmProcessor processor)
{
  CheckProcessorFail(processor, FALSE); 

  if (processor->AllocationFlags & RmF_Exclusive)
   return(FALSE);
  else
   return(TRUE);
}

bool	RmIsProcessorExclusive(RmProcessor processor)
{
  CheckProcessorFail(processor, FALSE);

  if (processor->AllocationFlags & RmF_Exclusive)
   return(TRUE);
  else
   return(FALSE);
}

int		RmSetProcessorTemporary(RmProcessor processor)
{
  CheckProcessor(processor);

  if ((processor->StructType eq RmL_New) ||
      (processor->StructType eq RmL_Existing))
   { processor->AllocationFlags &= ~RmF_Permanent;
     return(RmE_Success);
   }
  if (processor->StructType ne RmL_Obtained)
   return(RmErrno = processor->Errno = RmE_NoAccess);
  return(RmSetProcessorAllocation(RmC_ProcessorTemporary, processor));   
}

int		RmSetProcessorPermanent(RmProcessor processor)
{
  CheckProcessor(processor);

  if ((processor->StructType eq RmL_New) ||
      (processor->StructType eq RmL_Existing))
   { processor->AllocationFlags |= RmF_Permanent;
     return(RmE_Success);
   }
  if (processor->StructType ne RmL_Obtained)
   return(RmErrno = processor->Errno = RmE_NoAccess);
  return(RmSetProcessorAllocation(RmC_ProcessorPermanent, processor));   
}

bool		RmIsProcessorTemporary(RmProcessor processor)
{
  CheckProcessorFail(processor, FALSE);	

  if (processor->AllocationFlags & RmF_Permanent)
   return(FALSE);
  else
   return(TRUE);
}

bool		RmIsProcessorPermanent(RmProcessor processor)
{
  CheckProcessorFail(processor, FALSE);

  if (processor->AllocationFlags & RmF_Permanent)
   return(TRUE);
  else
   return(FALSE);
}

int		RmSetProcessorCancelled(RmProcessor processor)
{ 
  CheckProcessor(processor);

  if ((processor->StructType eq RmL_New) || 
      (processor->StructType eq RmL_Existing))
   { processor->AllocationFlags &= ~RmF_Booked;
     return(RmE_Success);
   }
  if (processor->StructType ne RmL_Obtained)
   return(RmErrno = processor->Errno = RmE_NoAccess);

  return(RmSetProcessorAllocation(RmC_ProcessorCancelled, processor));
}

int		RmSetProcessorBooked(RmProcessor processor)
{
  CheckProcessor(processor);

  if ((processor->StructType eq RmL_New) ||
      (processor->StructType eq RmL_Existing))
   { processor->AllocationFlags |= RmF_Booked;
     return(RmE_Success);
   }
  if (processor->StructType ne RmL_Obtained)
   return(RmErrno = processor->Errno = RmE_NoAccess);

  return(RmSetProcessorAllocation(RmC_ProcessorBooked, processor));   
}
/*}}}*/
/*{{{  Obtaining a network or group of processors */

/**----------------------------------------------------------------------------
*** Obtaining a network. This involves the following stages.
*** 1) check the network passed as argument. Every processor should be
***    either new or existing for now. Also, if the network is empty
***    forget it.
*** 2) start a new job. Send it the function code RmC_ObtainNetwork or
***    RmC_ObtainExactNetwork, together with the magic integer
***    RmStartSearchHere, and then the template network.
*** 3) wait around a long time for a reply. This reply may be 0 to indicate
***    complete success, RmE_PartialSuccess indicate partial success, 
***    or an error code. In the first two cases read in a network
*** 4) in the template, fill in the appropriate mapping info.
**/

static		int RmObtainAux1(RmProcessor Processor, ...);
static		int RmObtainAux2(RmProcessor Processor, ...);

RmNetwork	RmObtainNetwork(RmNetwork network, bool exact, int *rc_ptr)
{ int		rc;
  RmRequest	request;
  RmReply	reply;

  CheckNetworkFail(network, (RmNetwork) NULL);

  unless(network->Root eq (RmSet) network)
   return(network->Errno = RmErrno = RmE_NotRootNetwork, (RmNetwork) NULL);
   
  if ((rc = RmSearchProcessors(network, &RmObtainAux1)) ne RmE_Success)
   goto done;

  Clear(request); Clear(reply);
  request.FnRc		= (exact) ? RmC_ObtainExactNetwork : RmC_ObtainNetwork;
  request.Arg1		= RmStartSearchHere;
  request.Network	= network;

  rc = RmXch(&RmParent, &request, &reply);

  (void) RmApplyProcessors(reply.Network, &RmObtainAux2, network);

done:
  if (rc_ptr ne Null(int)) *rc_ptr = rc;
  if (rc ne RmE_Success) RmErrno = network->Errno = rc;
  return(reply.Network);
}

static		int RmObtainAux1(RmProcessor processor, ...)
{ 
  if ((processor->StructType ne RmL_New) &&
      (processor->StructType ne RmL_Existing))
   return(RmE_BadArgument);

  processor->MappedTo = 0;  
  return(0);
}

static		int RmObtainAux2(RmProcessor processor, ...)
{ va_list	args;
  RmNetwork	Template;
  RmProcessor	match;
  
  va_start(args, processor);
  Template = va_arg(args, RmNetwork);
  va_end(args);

	/* it is possible to get a processor without asking */
  if (processor->MappedTo eq 0) return(RmE_Success);

  match = RmFindProcessor(Template, processor->MappedTo);
  if (match ne (RmProcessor) NULL)
   match->MappedTo = processor->Uid;
  return(RmE_Success);
}


static	int	RmObtainProcessorsAux2(RmProcessor, ...);

RmNetwork RmObtainProcessors(int count, RmProcessor *processors, bool exact, int *rc_ptr)
{ RmJob		job;
  int		rc;
  RmNetwork	result = (RmNetwork) NULL;
  Stream	*pipe;
  bool		got_job = FALSE;
  int		real_count, i;
  RmRequest	request;
  RmReply	reply;

  if (processors eq Null(RmProcessor))
   return(RmErrno = RmE_BadArgument, (RmNetwork) NULL);
   
  for (real_count = 0, i = 0; i < count; i++)
   if (processors[i] ne (RmProcessor) NULL)
    { if (processors[i]->ObjNode.Type ne Type_Processor)
       return(RmErrno = RmE_BadArgument, (RmNetwork) NULL);
      elif ((processors[i]->StructType ne RmL_New) &&
            (processors[i]->StructType ne RmL_Existing))
        return(RmErrno = RmE_BadArgument, (RmNetwork) NULL);
      else
       { real_count++;
         processors[i]->MappedTo = 0;
       }
    }

  if (real_count eq 0)
   { if ((result = RmNewNetwork()) eq (RmNetwork) NULL)
      RmErrno = RmE_NoMemory;
     return(result);
   }
    
  rc		= RmNewJob(&RmParent, &job);
  if (rc ne RmE_Success) goto done;
  got_job	= TRUE;
  pipe		= job->Server->Pipe_ctos;

  Clear(request); Clear(reply);
  request.FnRc		= (exact) ? RmC_ObtainExactProcessors : RmC_ObtainProcessors;
  request.Arg1		= RmStartSearchHere;
  request.Arg2		= real_count;

  if ((rc = RmTx(job, &request)) ne RmE_Success)
   goto done;
  for (i = 0; i < count; i++)
   if (processors[i] ne (RmProcessor) NULL)
    if ((rc = RmWriteProcessor(pipe, processors[i], (RmFilter) NULL)) 
        ne RmE_Success)
     { RmUnlockWrite(job); goto done; }

  rc = RmRx(job, &reply);
  if ((rc ne RmE_Success) && (rc ne RmE_PartialSuccess)) goto done;

  (void) RmApplyProcessors(result, &RmObtainProcessorsAux2, processors, count);

done:
  if (got_job) RmFinishedJob(job);
  if (rc_ptr ne Null(int)) *rc_ptr = rc;
  if (rc ne RmE_Success) RmErrno = rc;
  return(reply.Network);
}

static	int	RmObtainProcessorsAux2(RmProcessor processor, ...)
{ va_list	args;
  RmProcessor	*Template;
  int		i, count;
    
  va_start(args, processor);
  Template = va_arg(args, RmProcessor *);
  count	   = va_arg(args, int);
  va_end(args);
  
  if (processor->MappedTo eq 0) return(RmE_Success);
  for (i = 0; i < count; i++)
   if (Template[i] ne (RmProcessor) NULL)
    if (Template[i]->Uid eq processor->MappedTo)
     Template[i]->MappedTo = processor->Uid;
    
  return(RmE_Success);
}

/*}}}*/
/*{{{  Releasing a network of processors */

/**-----------------------------------------------------------------------------
*** Releasing a network. This involves sending the uid and capability for
*** every processor in the network.
**/

static int	RmReleaseAux1(RmProcessor processor, ...);

int		RmReleaseNetwork(RmNetwork network) 
{ int			rc;
  RmRequest		request;
  RmReply		reply;
  int			count;

  CheckNetwork(network);
  unless(network->Root eq (RmSet) network)
   return(network->Errno = RmErrno = RmE_NotRootNetwork);
   
  count = RmCountProcessors(network);
  if (count eq 0)
   return(RmE_Success);
  elif (count < 0)
   return(RmE_NotNetwork);

  Clear(request); Clear(reply);
  request.VariableSize = count * sizeof(ProcessorDetails);
  request.VariableData = (char *)Malloc(request.VariableSize);
  if (request.VariableData eq NULL)
   return(RmE_NoMemory);

  count		= 0;
  rc	= RmApplyProcessors(network, &RmReleaseAux1, request.VariableData, &count);
  if (rc ne RmE_Success)
   { Free(request.VariableData); return(rc); }
  request.FnRc	= RmC_ReleaseNetwork;
  request.Arg1	= count;

  rc = RmXch(&RmParent, &request, &reply);
  Free(request.VariableData);
  if (rc ne RmE_Success) RmErrno = network->Errno = rc;
  return(rc);
}

static int RmReleaseAux1(RmProcessor processor, ...)
{ va_list		args;
  ProcessorDetails	*details;
  int			*count;
  
  va_start(args, processor);
  details	= va_arg(args, ProcessorDetails *);
  count		= va_arg(args, int *);
  va_end(args);
  
  if (processor->StructType ne RmL_Obtained) return(RmE_NoAccess);
  details[*count].Uid	= processor->Uid;
  details[*count].Cap	= processor->NsCap;
  *count += 1;

  processor->StructType = RmL_Existing;
  return(RmE_Success);
}

/*}}}*/
/*{{{  Link stuff */
/**
*** Manipulating link modes
**/
int		RmGetLinkMode(RmProcessor processor, int link, int *mode)
{ int		rc;
  RmRequest	request;
  RmReply	reply;

  CheckProcessor(processor);  

  if ((processor->StructType ne RmL_Existing) &&
      (processor->StructType ne RmL_Obtained))
   return(RmErrno = RmE_BadArgument);
  if ((link < 0) || (link > processor->Connections) || (mode eq Null(int)))
   return(RmErrno = RmE_BadArgument);

  Clear(request); Clear(reply);
  request.FnRc	= RmC_GetLinkMode;
  request.Uid	= processor->Uid;
  request.Arg1	= link;
  rc = RmXch(&RmNetworkServer, &request, &reply);

  if (rc ne RmE_Success) 
   RmErrno = processor->Errno = rc;
  else
   *mode = reply.Reply1;

  return(rc);
}

int		RmSetLinkMode(RmProcessor processor, int link, int mode)
{ int		rc;
  RmRequest	request;
  RmReply	reply;

  CheckProcessor(processor);  

  if ((processor->StructType ne RmL_Existing) &&
      (processor->StructType ne RmL_Obtained))
   return(RmErrno = processor->Errno = RmE_BadArgument);
  if ((link < 0) || (link > processor->Connections) ||
      (mode < RmL_NotConnected) || (mode > RmL_Dead))
   return(RmErrno = RmE_BadArgument);

  Clear(request); Clear(reply);
  request.FnRc	= RmC_SetLinkMode;
  request.Uid	= processor->Uid;
  request.Arg1	= link;
  request.Arg2	= mode;
  rc = RmXch(&RmNetworkServer, &request, &reply);
  if (rc ne RmE_Success) RmErrno = processor->Errno = rc;
  return(rc);
}
/*}}}*/
/*{{{  Reporting problems */
/**-----------------------------------------------------------------------------
*** Reporting problems.
**/
int RmReportProcessor(RmProcessor processor)
{ int		rc;
  RmRequest	request;
  RmReply	reply;

  CheckProcessor(processor);
  if ((processor->StructType ne RmL_Existing) &&
      (processor->StructType ne RmL_Obtained))
   return(RmErrno = processor->Errno = RmE_BadArgument);

  Clear(request); Clear(reply);
  request.FnRc	= RmC_ReportProcessor;
  request.Uid	= processor->Uid;
  rc = RmXch(&RmParent, &request, &reply);
  if (rc ne RmE_Success) RmErrno = processor->Errno = rc;
  return(rc);
}
/*}}}*/
/*{{{  Examining the current task/taskforce */
/**----------------------------------------------------------------------------
*** Examining the current taskforce/task.
**/

RmTaskforce RmGetTaskforce(void)
{ RmRequest	request;
  RmReply	reply;
  int		result;
  Environ	*env	= getenviron();
  Object	*tf  	= env->Objv[OV_TForce];

  if ((tf eq Null(Object)) || (tf eq (Object *) MinInt))
   return(RmErrno = RmE_NotTaskforce, (RmTaskforce) NULL);
  { char *tmp = objname(tf->Name);
    if (strncmp(&(tmp[-5]), "/tfm/", 5))
     return(RmErrno = RmE_NotTaskforce, (RmTaskforce) NULL);
  }

  Clear(request); Clear(reply);
  request.FnRc		= RmC_GetTaskforce;
  request.VariableData	= objname(tf->Name);
  request.VariableSize	= strlen(request.VariableData) + 1;
  result		= RmXch(&RmParent, &request, &reply);

  if (result eq RmE_Success)
   return(reply.Taskforce);
  else
   { RmErrno = result; return((RmTaskforce) NULL); }
}

RmTask RmGetTask(void)
{ RmRequest	request;
  RmReply	reply;
  int		result;
  Environ	*env	= getenviron();
  Object	*tf  	= env->Objv[OV_TForce];

  if ((tf eq Null(Object)) || (tf eq (Object *) MinInt))
   return(RmErrno = RmE_NotTask, (RmTask) NULL);
  { char *tmp = objname(tf->Name);
    if (strcmp(&(tmp[-5]), "/tfm/"))
     return(RmErrno = RmE_NotTask, (RmTask) NULL);
  }

  Clear(request); Clear(reply);
  request.FnRc		= RmC_GetTask;
  request.VariableData	= objname(tf->Name);
  request.VariableSize	= strlen(request.VariableData + 1);
  result		= RmXch(&RmParent, &request, &reply);

  if (result eq RmE_Success)
   return(reply.Task);
  else
   { RmErrno = result; return((RmTask) NULL); }
}  
/*}}}*/
/*{{{  Task and taskforce execution */

/*{{{  Execution */

/**-----------------------------------------------------------------------------
*** Executing tasks and taskforces
**/
/*{{{  Environment */

/**
*** Constructing environment information. This is similar but not identical
*** to the environment construction in execve() (posix/exec.c) and the
*** system library routine SendEnv() (nucleus/syslib/task.c), munged
*** together. The aim is to construct a single vector with all the
*** required environment information, which can be sent off in the
*** execution request.
***
*** a) determine the size of the required vector. This involves scanning
***     1) the supplied argument vector
***     2) the current program's environment vector
***     3) the current program's context, i.e. the object vector. Only some
***        of the objects should be passed, but space in the control vector
***        is still required for the remainder to make life easier for the TFM
***     4) standard streams
***    Note that all of the data structures will have to be aligned to
***    word boundaries.
*** b) allocate a suitable vector
*** c) put all the data in the vector. This uses the system library's
***    marshalling routines  so an MCB is faked up.
*** d) put the data vector and the size in the request.
**/
static int  count_object_size(Object **, int);
static void add_object(MCB *, Object **, int);

static bool buildenv(RmRequest *request, char **argv)
{ int		  argc, envc, objc, strc;
  int		  args, envs, objs, strs;
  Environ	 *my_environ = getenviron();
  char		**my_envv;
  Object	**my_objv;
  BYTE		 *datavec;
  int		  datasize;
  int		  i, j;
  MCB		  m;

  if (argv eq NULL)
   argc = args = 0;
  else
   for (i = 0, argc = 0, args = 0; argv[i] ne Null(char); i++, argc++)
    args = wordlen(args + strlen(argv[i]) + 1);

  my_envv = my_environ->Envv;
  for (i = 0, envc = 0, envs = 0; my_envv[i] ne Null(char); i++, envc++)
   envs = wordlen(envs + strlen(my_envv[i]) + 1);

	/* The following objects need to be sent: Cdir, Parent, Home,	  */
	/* Console, CServer, Session, TFM. The TFM takes care of the rest */
  my_objv = my_environ->Objv;
  objc = OV_End;
  objs = count_object_size(my_objv, OV_Cdir) +
  	 count_object_size(my_objv, OV_Task) +
  	 count_object_size(my_objv, OV_Home) +
  	 count_object_size(my_objv, OV_Console) +
  	 count_object_size(my_objv, OV_CServer) +
	 count_object_size(my_objv, OV_Session) +
	 count_object_size(my_objv, OV_TFM);

	 /* Only stdin, stdout, stderr, and stdlog are inherited. Care	*/
	 /* has to be taken with replicated streams. The library has no	*/
	 /* access to the CloseOnExec flag, so there is no simple way	*/
	 /* of implementing the full inheritance.			*/
  strc = 4; strs = 0;
  { Stream	*stream_tab[4];
    for (i = 0; i < 4; i++)
     { if ((stream_tab[i] = fdstream(i)) eq Null(Stream))
        continue;
       else
        { for (j = 0; j < i; j++)
           if (stream_tab[j] eq stream_tab[i])
            { stream_tab[j] = (Stream *) MinInt;
              goto skipstream;
            }  
 	  strs += sizeof(StrDesc) + strlen(stream_tab[i]->Name);
	  strs = wordlen(strs);
 	}
skipstream:
       j = i;
     }
  }

	/* Allocate the necessary amount of space	*/
  datasize  = sizeof(WORD) * (argc + envc + objc + strc +4);
  datasize += args + envs + strs + objs;
  datavec   = (BYTE *)Malloc(datasize);
  if (datavec eq NULL)
   return(FALSE);

	/* Initialise an MCB to allow marshalling	*/
  m.MsgHdr.ContSize = m.MsgHdr.DataSize = 0;
  m.Control = (WORD *) datavec;
  m.Data    = &(datavec[sizeof(WORD) * (argc + envc + strc + objc + 4)]);

  if (argv ne NULL)
   { for (i = 0; i < argc; i++)
      if (*(argv[i]) ne '\0')	/* test for empty string */
       MarshalString(&m, argv[i]);
      else
       { word  zero = 0;
         MarshalOffset(&m);
         MarshalData(&m, 4, (BYTE *) &zero);
       }
   }
  MarshalWord(&m, -1);

  my_envv = my_environ->Envv;
  for (i  = 0; i < envc; i++)
   MarshalString(&m, my_envv[i]);
  MarshalWord(&m, -1);

  my_objv = my_environ->Objv;
  add_object(&m, my_objv, OV_Cdir);
  MarshalWord(&m, MinInt);		/* OV_Task   */
  MarshalWord(&m, MinInt);		/* OV_Code   */
  MarshalWord(&m, MinInt);		/* OV_Source */
  add_object(&m, my_objv, OV_Task);	/* OV_Parent */
  add_object(&m, my_objv, OV_Home);
  add_object(&m, my_objv, OV_Console);
  add_object(&m, my_objv, OV_CServer);
  add_object(&m, my_objv, OV_Session);
  add_object(&m, my_objv, OV_TFM);
  MarshalWord(&m, MinInt);		/* OV_TForce */
  MarshalWord(&m, -1);

  { Stream	*stream_tab[4];
    for (i = 0; i < 4; i++)
     { if ((stream_tab[i] = fdstream(i)) eq Null(Stream))
        { MarshalWord(&m, MinInt); continue; }
       for (j = 0; j < i; j++)
        if (stream_tab[j] eq stream_tab[i])
         { MarshalWord(&m, (0xFFFF0000L | (word)j)); goto skip_stream2; }
       MarshalStream(&m, stream_tab[i]);
skip_stream2:
       j = i;
     }
  }
  MarshalWord(&m, -1);

	/* And put the vector into the request		*/
  request->VariableData	= datavec;
  request->VariableSize	= datasize;
  return(TRUE);
}

static int count_object_size(Object **my_objv, int index)
{ int	i, result;
  for (i = 0; i <= index; i++)
   if (my_objv[i] eq NULL)
    return(0);
  if (my_objv[index] eq (Object *) MinInt)
   return(0);
  result = sizeof(ObjDesc) + strlen(my_objv[index]->Name);
  return(wordlen(result));
}

static void add_object(MCB *m, Object **my_objv, int index)
{ int i;

  for (i = 0; i <= index; i++)
   if (my_objv[i] eq NULL)
    { MarshalWord(m, MinInt); return; }

  if (my_objv[index] ne (Object *) MinInt)
   MarshalObject(m, my_objv[index]);
  else
   MarshalWord(m, MinInt);
}

/*}}}*/
/*{{{  Single task */
/**
*** Executing a single task.
*** a) the task must not be running or have terminated already
*** b) if a processor is specified then this processor must have been
***    obtained.
*** c) the request is constructed. The TFM is going to need full details
***    of the task, plus the uid and capability of the processor if any
*** d) the environment information is gathered together
*** e) the request is sent off to the TFM. The reply should be another
***    task structure with type RmL_Executing
**/
RmTask	RmExecuteTask(RmProcessor processor, RmTask task, char **argv)
{ RmRequest	request;
  RmReply	reply;
  int		rc;
  
  Clear(request); Clear(reply);
  
  CheckTaskFail(task, (RmTask) NULL);
  unless(task->StructType eq RmL_New)
   return(RmErrno = task->Errno = RmE_InUse, (RmTask) NULL);

  if (processor ne (RmProcessor) NULL)
   { if (processor->ObjNode.Type ne Type_Processor)
      return(RmErrno = task->Errno = RmE_NotProcessor, (RmTask) NULL);
     unless(processor->StructType eq RmL_Obtained)
      return(RmErrno = task->Errno = RmE_NoAccess, (RmTask) NULL);
   }

  request.FnRc		= RmC_Execute;
  request.Task		= task;
  request.Uid		= processor->Uid;
  request.Cap		= processor->NsCap;
  unless(buildenv(&request, argv))
   return(RmErrno = task->Errno = RmE_BadArgument, (RmTask) NULL);

  rc = RmXch(&RmParent, &request, &reply);
  Free(request.VariableData);
  if (rc ne RmE_Success) RmErrno = task->Errno = rc;
  return(reply.Task);
}

/*}}}*/
/*{{{  Taskforce */
/**
*** Executing a taskforce is very similar. In this case the various components'
*** mapped processors have to be checked using an RmSearch. The whole network
*** is sent off to the TFM.
**/
static int RmExecuteTaskforce_aux1(RmTask task, ...);

RmTaskforce RmExecuteTaskforce(RmNetwork network, RmTaskforce taskforce, char **argv)
{ RmRequest	request;
  RmReply	reply;
  int		rc;

  Clear(request); Clear(reply);

  CheckTaskforceFail(taskforce, (RmTaskforce) NULL);
  unless(taskforce->StructType eq RmL_New)
   return(RmErrno = taskforce->Errno = RmE_InUse, (RmTaskforce) NULL);

  if (network ne (RmNetwork) NULL)
   if (network->DirNode.Type ne Type_Network)
    return(RmErrno = RmE_NotNetwork, (RmTaskforce) NULL);

  rc = RmSearchTasks(taskforce, &RmExecuteTaskforce_aux1, network);
  if (rc ne RmE_Success)
   return(RmErrno = taskforce->Errno = rc, (RmTaskforce) NULL);

  request.FnRc		= RmC_Execute;
  request.Taskforce	= taskforce;
  request.Network	= network;
  unless(buildenv(&request, argv))
   return(RmErrno = taskforce->Errno = RmE_BadArgument, (RmTaskforce) NULL);

  rc = RmXch(&RmParent, &request, &reply);
  Free(request.VariableData);
  if (rc ne RmE_Success) RmErrno = taskforce->Errno = rc;
  return(reply.Taskforce);
}

static int RmExecuteTaskforce_aux1(RmTask task, ...)
{ va_list	args;
  RmNetwork	network;
  RmProcessor	processor;

  va_start(args, task);
  network = va_arg(args, RmNetwork);
  va_end(args);

  unless(task->StructType eq RmL_New)
   return(RmErrno = task->Errno = RmE_InUse);

  if ((network ne (RmNetwork) NULL) && (task->MappedTo ne RmL_NoUid))
   { processor = RmFindProcessor(network, task->MappedTo);
     if (processor eq (RmProcessor) NULL)
      return(RmErrno = task->Errno = RmE_NotProcessor);
     unless (processor->StructType eq RmL_Obtained)
      return(RmErrno = task->Errno = RmE_NoAccess);
   }

  return(RmE_Success);
}

/*}}}*/

/*}}}*/
/*{{{  Status */

/**
*** Return codes, checking tasks whether they are still running, etc.
*** 1) if the relevant task or taskforce is of type RmL_New then it has
***    not been submitted to the TFM, so the request is illegal.
*** 2) if the relevant task or taskforce is of type RmL_Done then
***    the TFM has already informed this client about the termination.
***    Hence the routine can return immediately.
*** 3) if the relevant task or taskforce is of type RmL_Executing then
***    things are a bit more complicated:
***    a) it is necessary to update the task or taskforce, i.e. give the
***       TFM a chance to fill in termination status and return code. For
***       some requests this operation should block if the task is still
***       running.
***    b) if the TFM does not know about the relevant task or taskforce then
***       chances are that the object has already disappeared.
***    c) otherwise the TFM does the necessary work.
**/
static int update_tasks(int, RmTask *, bool wait);

int	RmGetTaskReturncode(RmTask task)
{ int	rc;

  CheckTaskFail(task, -1);
  unless((task->StructType eq RmL_Executing) || (task->StructType eq RmL_Done))
   return(RmErrno = task->Errno = RmE_BadArgument, -1);
   
  if (task->StructType eq RmL_Executing)
   if ((rc= update_tasks(1, &task, FALSE)) ne RmE_Success)
    return(RmErrno = task->Errno = rc, -1);

  if (task->StructType eq RmL_Executing)
   return(RmErrno = task->Errno = RmE_InUse, -1);
  else
   return(task->ReturnCode);
}

int	RmGetTaskforceReturncode(RmTaskforce taskforce)
{ int	rc;

  CheckTaskforceFail(taskforce, -1);
  unless((taskforce->StructType eq RmL_Executing) ||
         (taskforce->StructType eq RmL_Done))
   return(RmErrno = taskforce->Errno = RmE_BadArgument, -1);

  if (taskforce->StructType eq RmL_Executing)
   if ((rc = update_tasks(1, (RmTask *) &taskforce, FALSE)) ne RmE_Success)
    return(RmErrno = taskforce->Errno = rc, -1);

  if (taskforce->StructType eq RmL_Executing)
   return(RmErrno = taskforce->Errno = RmE_InUse, -1);
  else  
   return(taskforce->ReturnCode);
}

bool	RmIsTaskRunning(RmTask task)
{ int	rc;

  CheckTaskFail(task, FALSE);
  unless((task->StructType eq RmL_Executing) || (task->StructType eq RmL_Done))
   return(RmErrno = task->Errno = RmE_BadArgument, FALSE);
   
  if (task->StructType eq RmL_Executing)
   if ((rc = update_tasks(1, &task, FALSE)) ne RmE_Success)
    return(RmErrno = task->Errno = rc, FALSE);

  if (task->StructType eq RmL_Executing)
   return(TRUE);
  else
   return(FALSE);
}

bool	RmIsTaskforceRunning(RmTaskforce taskforce)
{ int rc;

  CheckTaskforceFail(taskforce, FALSE);
  unless((taskforce->StructType eq RmL_Executing) ||
         (taskforce->StructType eq RmL_Done))
   return(RmErrno = taskforce->Errno = RmE_BadArgument, FALSE);

  if (taskforce->StructType eq RmL_Executing)
   if ((rc = update_tasks(1, (RmTask *) &taskforce, FALSE)) ne RmE_Success)
    return(RmErrno = taskforce->Errno = rc, FALSE);

  if (taskforce->StructType eq RmL_Executing)
   return(TRUE);
  else  
   return(FALSE);
}

int	RmWaitforTask(RmTask task)
{ int	rc;

  CheckTask(task);
  unless((task->StructType eq RmL_Executing) || (task->StructType eq RmL_Done))
   return(RmErrno = task->Errno = RmE_BadArgument);
   
  if (task->StructType eq RmL_Executing)
   if ((rc = update_tasks(1, &task, TRUE)) ne RmE_Success)
    return(RmErrno = task->Errno = rc);

  if (task->StructType eq RmL_Done)
   return(RmE_Success);
  else
   return(RmE_BadArgument);
}

int	RmWaitforTaskforce(RmTaskforce taskforce)
{ int	rc;

  CheckTaskforce(taskforce);
  unless((taskforce->StructType eq RmL_Executing) ||
         (taskforce->StructType eq RmL_Done))
   return(RmErrno = taskforce->Errno = RmE_BadArgument);

  if (taskforce->StructType eq RmL_Executing)
   if ((rc = update_tasks(1, (RmTask *) &taskforce, TRUE)) ne RmE_Success)
    return(RmErrno = taskforce->Errno = rc);

  if (taskforce->StructType eq RmL_Done)
   return(RmE_Success);
  else  
   return(RmE_BadArgument);
}


int	RmWaitforTasks(int count, RmTask *tasks)
{ int		i;
  int		rc;
  
  for (i = 0; i < count; i++)
   { RmTask	task = tasks[i];
     if (task eq (RmTask) NULL) continue;
     if (task->ObjNode.Type ne Type_Task) return(RmE_NotTask);
     if (task->StructType eq RmL_Done)
      return(RmE_Success);
     if (task->StructType ne RmL_Executing)
      return(RmErrno = task->Errno = RmE_BadArgument);
   }

  if ((rc = update_tasks(count, tasks, TRUE)) ne RmE_Success)
   return(RmErrno = rc);

  for (i = 0; i < count; i++)
   { RmTask	task = tasks[i];
     if (task eq (RmTask) NULL) continue;
     if (task->StructType eq RmL_Done) return(RmE_Success);
   }
  return(RmErrno = RmE_BadArgument);
}

/**
*** Waiting for any component task to finish involves building up a table
*** of all the component tasks.
**/
static int RmWaitforAnyTask_aux1(RmTask, ...);

int	RmWaitforAnyTask(RmTaskforce taskforce)
{ int		number_tasks	= RmCountTasks(taskforce);
  RmTask	*task_table	= Null(RmTask);
  int		rc;
  int		i;
  
  CheckTaskforce(taskforce);
  if (number_tasks eq 0) return(RmE_Success);
  task_table = (RmTask *)Malloc((word) number_tasks * sizeof(RmTask));
  if (task_table eq Null(RmTask))
   return(RmErrno = taskforce->Errno = RmE_NoMemory);

  i = 0;
  (void) RmApplyTasks(taskforce, &RmWaitforAnyTask_aux1, task_table, &i);

  rc = RmWaitforTasks(number_tasks, task_table);
  Free(task_table);
  return(rc);
}

static int RmWaitforAnyTask_aux1(RmTask task, ...)
{ va_list	args;
  RmTask	*task_table;
  int		*i_ptr;
  
  va_start(args, task);
  task_table = va_arg(args, RmTask *);
  i_ptr	     = va_arg(args, int *);
  va_end(args);

  task_table[*i_ptr] = task;
  *i_ptr += 1;
  return(0);
}

/**
*** Update the information associated with one or more tasks or taskforces.
*** If an entire taskforce has terminated then all its components should
*** be set to terminated using an RmApply.
**/
static int update_tasks_aux1(RmTask task, ...)
{ task->StructType = RmL_Done;
  return(0);
}

static int update_tasks(int count, RmTask *tasks, bool wait)
{ RmRequest	request;
  RmReply	reply;
  int		rc;
  int		i;
  TaskDetails	*details = (TaskDetails *)Malloc((word) count * sizeof(TaskDetails));
  TaskUpdate	*info;

  if (details eq Null(TaskDetails)) return(RmE_NoMemory);
  for (i = 0; i < count; i++)
   { RmTask	task = tasks[i];

     if (task eq (RmTask) NULL)
      details[i].Name[0] = '\0';
     elif (task->ObjNode.Type eq Type_Taskforce)
      { RmTaskforce taskforce = (RmTaskforce) task;
        strcpy(details[i].Name, taskforce->DirNode.Name);
        details[i].Uid = RmL_NoUid;
        details[i].Cap = taskforce->TfmCap;
      }
     elif (task->Root eq (RmTaskforce) NULL)
      { strcpy(details[i].Name, task->ObjNode.Name);
	details[i].Uid = RmL_NoUid;
	details[i].Cap = task->TfmCap;
      }
     else
      { RmTaskforce taskforce = task->Root;
        strcpy(details[i].Name, taskforce->DirNode.Name);
        details[i].Uid = task->Uid;
        details[i].Cap = task->TfmCap;
      }
   }
     
  Clear(request); Clear(reply);
  if (wait)
   request.FnRc		= RmC_Wait;
  else
   request.FnRc		= RmC_Update;
  request.VariableData	= (BYTE *) details;
  request.VariableSize	= count * sizeof(TaskDetails);

  rc = RmXch(&RmParent, &request, &reply);
  Free(details);
  if (rc ne RmE_Success) return(rc);

  info = (TaskUpdate *) reply.VariableData;
  for (i = 0; i < count; i++)
   { RmTask	task    = tasks[i];
     TaskUpdate	*update = &(info[i]);

     if (task eq (RmTask) NULL)    continue;
     if (update->Errno eq RmE_Skip) continue;

     if (task->ObjNode.Type eq Type_Taskforce)
      { RmTaskforce taskforce = (RmTaskforce) task;
        if (update->Errno ne RmE_Success)
         taskforce->Errno = update->Errno;
        taskforce->StructType = update->StructType;
        taskforce->ReturnCode = update->ReturnCode;
        if (taskforce->StructType eq RmL_Done)
         (void) RmApplyTasks(taskforce, &update_tasks_aux1);
      }
     else
      { if (update->Errno ne RmE_Success)
         task->Errno = update->Errno;
        task->StructType = update->StructType;
        task->ReturnCode = update->ReturnCode;
      }
   }
  Free(info);
  return(RmE_Success);
}

/*}}}*/
/*{{{  Detaching */
int	RmLeaveTask(RmTask task)
{ RmRequest	request;
  RmReply	reply;
  TaskDetails	details;
  
  CheckTask(task);
  unless(task->StructType eq RmL_Executing)
   return(RmErrno = task->Errno = RmE_BadArgument);
  if (task->Root ne (RmTaskforce) NULL)
   return(RmErrno = task->Errno = RmE_InUse);
   
  Clear(request); Clear(reply);
  request.FnRc		= RmC_Leave;
  strcpy(details.Name, task->ObjNode.Name);
  details.Uid		= RmL_NoUid;
  details.Cap		= task->TfmCap;
  request.VariableData	= (BYTE *) &details;
  request.VariableSize	= sizeof(TaskDetails);
  return(RmXch(&RmParent, &request, &reply));
}

int	RmLeaveTaskforce(RmTaskforce taskforce)
{ RmRequest	request;
  RmReply	reply;
  TaskDetails	details;
  
  CheckTaskforce(taskforce);
  unless(taskforce->StructType eq RmL_Executing)
   return(RmErrno = taskforce->Errno = RmE_BadArgument);
  if (taskforce->Root ne (RmSet) taskforce)
   return(RmErrno = taskforce->Errno = RmE_NotRootTaskforce);
   
  Clear(request); Clear(reply);
  request.FnRc	= RmC_Leave;
  strcpy(details.Name, taskforce->DirNode.Name);
  details.Uid	= RmL_NoUid;
  details.Cap	= taskforce->TfmCap;
  request.VariableData	= (BYTE *) &details;
  request.VariableSize	= sizeof(TaskDetails);
  return(RmXch(&RmParent, &request, &reply));
}

/*}}}*/
/*{{{  Signals */
int	RmSendTaskSignal(RmTask task, int signo)
{ RmRequest	request;
  RmReply	reply;
  TaskDetails	details;

  CheckTask(task);
  unless(task->StructType eq RmL_Executing)
   return(RmErrno = task->Errno = RmE_BadArgument);

  Clear(request); Clear(reply);
  if (task->Root eq (RmTaskforce) NULL)
   { strcpy(details.Name, task->ObjNode.Name);
     details.Uid	= RmL_NoUid;
     details.Cap	= task->TfmCap;
   }
  else
   { RmTaskforce taskforce = task->Root;
     strcpy(details.Name, taskforce->DirNode.Name);
     details.Uid	= task->Uid;
     details.Cap	= task->TfmCap;
   }
  request.FnRc		= RmC_SendSignal;
  request.Arg1		= signo;
  request.VariableData	= (BYTE *) &details;
  request.VariableSize	= sizeof(TaskDetails);
  return(RmXch(&RmParent, &request, &reply));
}

int	RmSendTaskforceSignal(RmTaskforce taskforce, int signo)
{ RmRequest	request;
  RmReply	reply;
  TaskDetails	details;
  
  CheckTaskforce(taskforce);
  unless(taskforce->StructType eq RmL_Executing)
   return(RmErrno = taskforce->Errno = RmE_BadArgument);

  Clear(request); Clear(reply);
  strcpy(details.Name, taskforce->DirNode.Name);
  details.Uid	= RmL_NoUid;
  details.Cap	= taskforce->TfmCap;

  request.FnRc		= RmC_SendSignal;
  request.Arg1		= signo;
  request.VariableData	= (BYTE *) &details;
  request.VariableSize	= sizeof(TaskDetails);
  return(RmXch(&RmParent, &request, &reply));
}
/*}}}*/
/*{{{  Task->Taskforce */

/**
*** Converting a task to a taskforce requires the following:
*** 1) check that the arguments are valid
*** 2) check that the controller is part of the taskforce
*** 3) set the TfmFlags_Special flag in the controller
*** 4) add a puid attribute to the controller identifying the task. This
***    is necessary to let the TFM monitor the task correctly.
*** 5) do an RmExecuteTaskforce() to make the TFM do the hard work
*** 6) modify the OV_TForce entry in the environment to reflect that this
***    program is now part of a taskforce.
**/
RmTaskforce	RmConvertTaskToTaskforce(RmTask controller, RmTaskforce taskforce)
{ char		*tid		= NULL;
  Environ	*env		= getenviron();
  Object	*this_task	= env->Objv[OV_Task];
  int		 rc;
  RmTaskforce	 result;
  Object	*new_taskforce;

  CheckTaskFail(controller, NULL);
  CheckTaskforceFail(taskforce, NULL);

  if (RmRootTaskforce(controller) ne taskforce)
   { RmErrno = RmE_WrongTaskforce; return(NULL); }

  controller->ObjNode.Flags |= 0x08000000;	/* see tfmaux.h */
  tid = (char *)Malloc(4 + 17 + 1 + (word) strlen(this_task->Name));
  if (tid eq NULL)
   { RmErrno = RmE_NoMemory; return(NULL); }
  strcpy(tid, "tid=");
  DecodeCapability(&(tid[4]), &(this_task->Access));
  strcpy(&(tid[21]), this_task->Name);
  rc = RmAddObjectAttribute((RmObject) controller, tid, TRUE);
  Free(tid);
  if (rc ne RmE_Success)
   { RmErrno = rc; return(NULL); }
  
  result = RmExecuteTaskforce(NULL, taskforce, NULL);
  if (result eq NULL) return(NULL);

  new_taskforce = Locate(env->Objv[OV_TFM], result->DirNode.Name);
  if (new_taskforce eq NULL) return(result);	/* BLV recovery ??? */

  if (env->Objv[OV_TForce] ne (Object *) MinInt)
   Close(env->Objv[OV_TForce]);
  env->Objv[OV_TForce] = new_taskforce;
  return(result);
}

/*}}}*/

/*}}}*/
#ifdef __TRAN
/*{{{  Native network support */
/**----------------------------------------------------------------------------
*** Native network support
**/

static	bool	CheckConnections(int count, LinkConnection *, bool, bool);
static	int	MakeConnections(int count, LinkConnection *, bool, bool);

bool	RmIsLinkPossible(RmProcessor source, int sourcelink, RmProcessor dest,
		int destlink)
{ LinkConnection	conn;

  CheckProcessorFail(source, FALSE);
  if ((source->StructType ne RmL_Existing) &&
      (source->StructType ne RmL_Obtained))
   return(RmErrno = source->Errno = RmE_BadProcessor, FALSE);

  if ((sourcelink < 0) || (sourcelink >= source->Connections))
   return(RmErrno = source->Errno  = RmE_BadLink, FALSE);

  conn.SourceUid	= source->Uid;
  conn.SourceCap	= source->NsCap;
  conn.SourceLink	= sourcelink;
 
  if (dest eq RmM_ExternalProcessor)
   { conn.DestUid	= RmL_ExtUid;
     conn.DestLink	= destlink;
   }
  else
   { CheckProcessorFail(dest, FALSE);
     if ((dest->StructType ne RmL_Existing) &&
         (dest->StructType ne RmL_Obtained))
      return(RmErrno = dest->Errno = RmE_BadProcessor, FALSE);
     if ((destlink < 0) || (destlink >= dest->Connections))
      return(RmErrno = dest->Errno = RmE_BadLink, FALSE);

     conn.DestUid	= dest->Uid;
     conn.DestCap	= dest->NsCap;
     conn.DestLink	= destlink;
   }

  return(CheckConnections(1, &conn, TRUE, TRUE));    
}

static	int Plausibility_aux1(RmProcessor processor, ...);
static	int Plausibility_aux2(RmProcessor processor, ...);

bool	RmIsNetworkPossible(RmNetwork network, bool exact, bool preserve)
{ LinkConnection	*conns;
  int			rc;
  int			conns_count = 0;
  bool			result;
      
  CheckNetworkFail(network, FALSE);
  unless(network->Root eq (RmSet) network)
   return(network->Errno = RmErrno = RmE_NotRootNetwork);
   
  rc = RmSearchProcessors(network, &Plausibility_aux1, &conns_count);
  if (rc ne RmE_Success)
   return(RmErrno = rc, FALSE);
   
  conns = Malloc(conns_count * sizeof(LinkConnection));
  if (conns eq Null(LinkConnection))
   return(RmErrno = RmE_NoMemory, FALSE);
   
  conns_count = 0;
  rc = RmSearchProcessors(network, &Plausibility_aux2, conns, &conns_count);
  if (rc ne RmE_Success)
   { Free(conns);
     return(RmErrno = rc, FALSE);
   }

  result = CheckConnections(conns_count, conns, exact, preserve);
  Free(conns);
  return(result);
}

bool	RmAreProcessorsPossible(int count, RmProcessor *processors, bool exact,
		bool preserve)
{ int			real_count, i;
  int			rc;
  int			conns_count;
  bool			result;
  LinkConnection	*conns;
      
  if (processors eq Null(RmProcessor))
   return(RmErrno = RmE_BadArgument, FALSE);
   
  for (i = 0, real_count = 0, conns_count = 0; i < count; i++)
   if (processors[i] ne (RmProcessor) NULL)
    { real_count++;
      unless((rc = Plausibility_aux1(processors[i], &conns_count)) eq RmE_Success)
       return(RmErrno = rc, FALSE);
    }

  if (real_count eq 0)
   return(RmErrno = RmE_BadArgument, FALSE);
        
  conns = Malloc(conns_count * sizeof(LinkConnection));
  if (conns eq Null(LinkConnection))
   return(RmErrno = RmE_NoMemory, FALSE);
   
  conns_count = 0;
  for (i = 0; i < count; i++)
   if (processors[i] ne (RmProcessor) NULL)
    if ((rc = Plausibility_aux2(processors[i], conns, &conns_count))
        ne RmE_Success)
     { Free(conns);
       return(RmErrno = rc, FALSE);
     }
     
  result = CheckConnections(conns_count, conns, exact, preserve);
  Free(conns);
  return(result);    
}

static	int Plausibility_aux1(RmProcessor processor, ...)
{ va_list	args;
  int		*conns_count;
  
  va_start(args, processor);
  conns_count = va_arg(args, int *);
  va_end(args);

  CheckProcessor(processor);
  unless((processor->StructType eq RmL_Existing) ||
         (processor->StructType eq RmL_Obtained))
   return(RmErrno = processor->Errno = RmE_BadProcessor);
  *conns_count += processor->Connections;
  return(RmE_Success);
}

static	int Plausibility_aux2(RmProcessor processor, ...)
{ va_list		args;
  int			*conns_count_ptr;
  int			conns_count;
  LinkConnection	*conns;
  int			i;
  RmProcessor		neighbour;
  int			destlink;
      
  va_start(args, processor);
  conns		  = va_arg(args, LinkConnection *);
  conns_count_ptr = va_arg(args, int *);
  va_end(args);
  conns_count	  = *conns_count_ptr;
  
  for (i = 0; i < processor->Connections; i++)
   {
     neighbour = RmFollowLink(processor, i, &destlink);
     if (neighbour eq (RmProcessor) NULL)
      { conns[conns_count].SourceUid	= processor->Uid;
        conns[conns_count].SourceCap	= processor->NsCap;
        conns[conns_count].SourceLink	= i;
        conns[conns_count].DestUid	= RmL_NoUid;
        conns[conns_count].DestLink	= destlink;
        conns_count++;
        continue;
      }
      	/* probably a processor outside the user's network */
     if (neighbour eq RmM_ExternalProcessor)
      { RmLink *link = RmFindLink(processor, i);
        conns[conns_count].SourceUid	= processor->Uid;
        conns[conns_count].SourceCap	= processor->NsCap;
        conns[conns_count].SourceLink	= i;
        conns[conns_count].DestUid	= link->Target;
        conns[conns_count].DestLink	= link->Destination;
        conns[conns_count].DestCap.Access = 0;
        conns_count++;
        continue;
      }

     conns[conns_count].SourceUid	= processor->Uid;
     conns[conns_count].SourceCap	= processor->NsCap;
     conns[conns_count].SourceLink	= i;
     conns[conns_count].DestUid		= neighbour->Uid;
     conns[conns_count].DestCap		= neighbour->NsCap;
     conns[conns_count].DestLink	= destlink;
     conns_count++;
   }
           
  *conns_count_ptr = conns_count;
  return(RmE_Success);
}

static	bool	CheckConnections(int count, LinkConnection *conns,
		bool exact, bool preserve)
{ int		rc;
  RmRequest	request;
  RmReply	reply;

  Clear(request); Clear(reply);
  request.FnRc		= RmC_TestConnections;
  request.Arg1		= count;
  request.Arg2		= (int)exact;
  request.Arg3		= (int)preserve;
  request.VariableSize	= count * sizeof(LinkConnection);
  request.VariableData	= (BYTE *) conns;
  rc = RmXch(&RmNetworkServer, &request, &reply);
  return(rc eq RmE_Success ? TRUE : FALSE);
}

static	int	MakeConnections(int count, LinkConnection *conns,
		bool exact, bool preserve)
{ int		rc;
  RmRequest	request;
  RmReply	reply;

  Clear(request); Clear(reply);
  if (exact eq 666)
   request.FnRc = RmC_Revert;
  else
   request.FnRc = RmC_MakeConnections;
  request.Arg1		= count;
  request.Arg2		= (int)exact;
  request.Arg3		= (int)preserve;
  request.VariableSize	= count * sizeof(LinkConnection);
  request.VariableData	= (BYTE *) conns;

  reply.VariableData	= (BYTE *) conns;

  rc = RmXch(&RmParent, &request, &reply);
  return(rc);
}
		
int	RmReconfigureNetwork(RmNetwork network, bool exact, bool preserve)
{ LinkConnection	*conns;
  int			rc;
  int			conns_count = 0;
  int			i;

  CheckNetwork(network);
  unless(network->Root eq (RmSet) network)
   return(network->Errno = RmErrno = RmE_NotRootNetwork);
   
  rc = RmSearchProcessors(network, &Plausibility_aux1, &conns_count);
  if (rc ne RmE_Success)  
   return(RmErrno = rc, FALSE);

  conns = Malloc(conns_count * sizeof(LinkConnection));
  if (conns eq Null(LinkConnection))
   return(RmErrno = RmE_NoMemory, FALSE);

  conns_count = 0;
  rc = RmSearchProcessors(network, &Plausibility_aux2, conns, &conns_count);
  if (rc ne RmE_Success)
   { Free(conns);
     return(RmErrno = rc);
   }

  if ((rc = MakeConnections(conns_count, conns, exact, preserve))  
      ne RmE_Success)
   { Free(conns);
     return(RmErrno = network->Errno = rc);
   }

  for (i = 0; i < conns_count; i++)
   { RmProcessor source = RmFindProcessor(network, conns[i].SourceUid);
     RmLink		*link;

     if (source eq (RmProcessor) NULL) continue;
		
     link = RmFindLink(source, conns[i].SourceLink);
     { RmProcessor neighbour;	/* update the old neighbour, if appropriate */
       int	   destlink;
       neighbour = RmFollowLink(source, conns[i].SourceLink, &destlink);
       if ((neighbour ne RmM_NoProcessor) && (neighbour ne RmM_ExternalProcessor))
        { RmLink *link = RmFindLink(neighbour, destlink);
          if ((link->Target eq source->Uid) &&
              (link->Destination eq conns[i].SourceLink))
           { link->Destination = -1;
             link->Target = RmL_NoUid;
           }
        }
     }
     	/* update the current processor */
     link->Destination	= conns[i].DestLink;
     link->Target	= conns[i].DestUid;
     
    	/* update the new neighbour */
     if ((conns[i].DestUid ne RmL_NoUid) &&
         (conns[i].DestUid ne RmL_ExtUid))
      { source = RmFindProcessor(network, conns[i].DestUid);
        if (source eq (RmProcessor) NULL) continue;
        link = RmFindLink(source, conns[i].DestLink);
        link->Destination = conns[i].SourceLink;
        link->Target	  = conns[i].SourceUid;
      }        
   }
   
  Free(conns);
  return(rc);     
}

int	RmReconfigureProcessors(int count, RmProcessor *processors,
		bool exact, bool preserve)
{ int			real_count, i, j;
  int			rc;
  int			conns_count;
  LinkConnection	*conns;

  if (processors eq Null(RmProcessor))
   return(RmErrno = RmE_BadArgument, FALSE);

  for (i = 0, real_count = 0, conns_count = 0; i < count; i++)
   if (processors[i] ne (RmProcessor) NULL)
    { real_count++;
      unless((rc = Plausibility_aux1(processors[i], &conns_count)) eq RmE_Success)
       return(RmErrno = rc);
    }
  if (real_count eq 0)
   return(RmErrno = RmE_BadArgument);
   
  conns = Malloc(conns_count * sizeof(LinkConnection));
  if (conns eq Null(LinkConnection))
   return(RmErrno = RmE_NoMemory);
   
  conns_count = 0;
  for (i = 0; i < count; i++)
   if (processors[i] ne (RmProcessor) NULL)
    if ((rc = Plausibility_aux2(processors[i], conns, &conns_count))
        ne RmE_Success)
     { Free(conns);
       return(RmErrno = rc);
     }

   if ((rc = MakeConnections(conns_count, conns, exact, preserve))
        ne RmE_Success)
    { Free(conns);
      return(RmErrno = rc);
    }
    
  for (i = 0; i < conns_count; i++)
   for (j = 0; j < count; j++)
    { RmLink		*link;
      
      if (processors[j] eq (RmProcessor) NULL) continue;
      if (processors[j]->Uid ne conns[i].SourceUid) continue;
      link = RmFindLink(processors[j], conns[i].SourceLink);
      { RmProcessor neighbour;	/* update the old neighbour, if appropriate */
        int	    destlink;
        neighbour = RmFollowLink(processors[j], conns[i].SourceLink, &destlink);
        if ((neighbour ne RmM_NoProcessor) && (neighbour ne RmM_ExternalProcessor))
         { RmLink *link = RmFindLink(neighbour, destlink);
           if ((link->Target eq processors[j]->Uid) &&
               (link->Destination eq conns[i].SourceLink))
            { link->Destination = -1;
              link->Target = RmL_NoUid;
            }
         }
      }
		/* update this processor */
      link->Destination = conns[i].DestLink;
      link->Target	= conns[i].DestUid;
      
     		/* update the new neighbour, if appropriate */
      if ((conns[i].DestUid ne RmL_NoUid) &&
          (conns[i].DestUid ne RmL_ExtUid))
       { RmProcessor neighbour;
         neighbour = RmFindProcessor(RmRootNetwork(processors[j]), conns[i].DestUid);
         if ((neighbour ne RmM_NoProcessor) && (neighbour ne RmM_ExternalProcessor))
          { link = RmFindLink(neighbour, conns[i].DestLink);
            link->Destination = conns[i].SourceLink;
            link->Target	= conns[i].SourceUid;
            break;
          }
       }
      break;	/* out of j loop, for next i */
    }
    
  Free(conns);
  return(rc);
}

int	RmRevertNetwork(RmNetwork network)
{ LinkConnection	*conns;
  int			rc;
  int			conns_count = 0;
  int			i;

  CheckNetwork(network);
  unless(network->Root eq (RmSet) network)
   return(network->Errno = RmErrno = RmE_NotRootNetwork);
   
  rc = RmSearchProcessors(network, &Plausibility_aux1, &conns_count);
  if (rc ne RmE_Success)  
   return(RmErrno = network->Errno = rc, FALSE);

  conns = Malloc(conns_count * sizeof(LinkConnection));
  if (conns eq Null(LinkConnection))
   return(RmErrno = RmE_NoMemory, FALSE);

  conns_count = 0;
  rc = RmSearchProcessors(network, &Plausibility_aux2, conns, &conns_count);
  if (rc ne RmE_Success)
   { Free(conns);
     return(RmErrno = network->Errno = rc);
   }

  if ((rc = MakeConnections(conns_count, conns, 666, 666))  
      ne RmE_Success)
   { Free(conns);
     return(RmErrno = network->Errno = rc);
   }

  for (i = 0; i < conns_count; i++)
   { RmProcessor source = RmFindProcessor(network, conns[i].SourceUid);
     RmLink		*link;
     
     if (source eq (RmProcessor) NULL) continue;
     link = RmFindLink(source, conns[i].SourceLink);
     { RmProcessor neighbour;
       int	   destlink;
       neighbour = RmFollowLink(source, conns[i].SourceLink, &destlink);
       if ((neighbour ne RmM_NoProcessor) && (neighbour ne RmM_ExternalProcessor))
        { RmLink *link = RmFindLink(neighbour, destlink);
          if ((link->Target eq source->Uid) &&
              (link->Destination eq conns[i].SourceLink))
           { link->Destination = -1;
             link->Target = RmL_NoUid;
           }
        }
     }
     link->Destination	= conns[i].DestLink;
     link->Target	= conns[i].DestUid;
     if ((conns[i].DestUid ne RmL_NoUid) &&
         (conns[i].DestUid ne RmL_ExtUid))
      { source = RmFindProcessor(network, conns[i].DestUid);
        if (source eq (RmProcessor) NULL) continue;
        link = RmFindLink(source, conns[i].DestLink);
        link->Destination = conns[i].SourceLink;
        link->Target	  = conns[i].SourceUid;
      }        
   }
   
  Free(conns);
  return(rc);     
}

int	RmRevertProcessors(int count, RmProcessor *processors)
{ int			real_count, i, j;
  int			rc;
  int			conns_count;
  LinkConnection	*conns;

  if (processors eq Null(RmProcessor))
   return(RmErrno = RmE_BadArgument, FALSE);

  for (i = 0, real_count = 0, conns_count = 0; i < count; i++)
   if (processors[i] ne (RmProcessor) NULL)
    { real_count++;
      unless((rc = Plausibility_aux1(processors[i], &conns_count)) eq RmE_Success)
       return(RmErrno = rc);
    }
  if (real_count eq 0)
   return(RmErrno = RmE_BadArgument);
   
  conns = Malloc(conns_count * sizeof(LinkConnection));
  if (conns eq Null(LinkConnection))
   return(RmErrno = RmE_NoMemory);
   
  conns_count = 0;
  for (i = 0; i < count; i++)
   if (processors[i] ne (RmProcessor) NULL)
    if ((rc = Plausibility_aux2(processors[i], conns, &conns_count))
        ne RmE_Success)
     { Free(conns);
       return(RmErrno = rc);
     }

   if ((rc = MakeConnections(conns_count, conns, 666, 666))
        ne RmE_Success)
    { Free(conns);
      return(RmErrno = rc);
    }
    
  for (i = 0; i < conns_count; i++)
   for (j = 0; j < count; j++)
    { RmLink		*link;
      
      if (processors[j] eq (RmProcessor) NULL) continue;
      if (processors[j]->Uid ne conns[i].SourceUid) continue;
      link = RmFindLink(processors[j], conns[i].SourceLink);
      { RmProcessor neighbour;
        int	    destlink;
        neighbour = RmFollowLink(processors[j], conns[i].SourceLink, &destlink);
        if ((neighbour ne RmM_NoProcessor) && (neighbour ne RmM_ExternalProcessor))
         { RmLink *link = RmFindLink(neighbour, destlink);
           if ((link->Destination eq conns[i].SourceLink) &&
               (link->Target eq processors[j]->Uid))
            { link->Destination = -1;
              link->Target = RmL_NoUid;
            }
         }
      }
      link->Destination = conns[i].DestLink;
      link->Target	= conns[i].DestUid;
      if ((conns[i].DestUid ne RmL_NoUid) &&
          (conns[i].DestUid ne RmL_ExtUid))
       { int	k;
         for (k = 0; k < count; k++)
          if (processors[k] ne (RmProcessor) NULL)
           if (processors[k]->Uid eq conns[i].DestUid)
            { link = RmFindLink(processors[k], conns[i].DestLink);
              link->Destination = conns[i].SourceLink;
              link->Target	= conns[i].SourceUid;
              break;
            }
       }
      break;	/* out of j loop, for next i */
    }
    
  Free(conns);
  return(rc);
}

static	int	NetworkOp(RmNetwork, int);
static	int	ProcessorsOp(int count, RmProcessor *, int);

int	RmSetNetworkNative(RmNetwork network)
{ return(NetworkOp(network, RmC_SetNative));
}

int	RmSetProcessorsNative(int count, RmProcessor *processors)
{ return(ProcessorsOp(count, processors, RmC_SetNative));
}

int	RmResetNetwork(RmNetwork network)
{ return(NetworkOp(network, RmC_ResetProcessors));
}

int	RmResetProcessors(int count, RmProcessor *processors)
{ return(ProcessorsOp(count, processors, RmC_ResetProcessors));
}

int	RmRebootNetwork(RmNetwork network)
{ return(NetworkOp(network, RmC_Reboot));
}

int	RmRebootProcessors(int count, RmProcessor *processors)
{ return(ProcessorsOp(count, processors, RmC_Reboot));
}


static	int	GeneralOp(int count, ProcessorDetails *, ProcessorUpdate *,
			int op);
static	int	fill_in_details(RmProcessor processor, ...);

static	int	NetworkOp(RmNetwork network, int op)
{ int			count;
  ProcessorDetails	*details;
  ProcessorUpdate	*updates;
  int			rc;
  int			i;
  RmProcessor		current;

  CheckNetwork(network);
  unless(network->Root eq (RmSet) network)
   return(network->Errno = RmErrno = RmE_NotRootNetwork);
   
  count = RmCountProcessors(network);
  if (count <= 0)
   return(RmErrno = network->Errno = RmE_BadArgument);

  details = Malloc(count * sizeof(ProcessorDetails));
  if (details eq Null(ProcessorDetails))
   return(RmErrno = network->Errno = RmE_NoMemory);
  updates = Malloc(count * sizeof(ProcessorUpdate));
  if (updates eq Null(ProcessorUpdate))
   { Free(details); return(RmErrno = network->Errno = RmE_NoMemory); }
   
  count = 0; 
  if ((rc = RmSearchProcessors(network, &fill_in_details, details, &count))
      ne RmE_Success)
   { Free(details); Free(updates); return(RmErrno = network->Errno = rc); }

  rc = GeneralOp(count, details, updates, op);
  if ((rc eq RmE_Success) || (rc eq RmE_PartialSuccess))
   for (i = 0; i < count; i++)
    { current = RmFindProcessor(network, updates[i].Uid);
      if (current eq (RmProcessor) NULL) continue;
      current->ObjNode.Size = updates[i].State;
      current->Purpose = updates[i].Purpose;
    }
  else
   RmErrno = network->Errno = rc;
       
  Free(details);
  Free(updates);
  return(rc);     
}

static	int	ProcessorsOp(int count, RmProcessor *processors, int op)
{ int			real_count, i, j;
  ProcessorDetails	*details;
  ProcessorUpdate	*updates;
  int			rc;

  if (processors eq Null(RmProcessor))
   return(RmErrno = RmE_BadArgument);
     
  for (i = 0, real_count = 0; i < count; i++)
   if (processors[i] ne (RmProcessor) NULL)
    { if (processors[i]->ObjNode.Type ne Type_Processor)
       return(RmErrno = RmE_NotProcessor);
      real_count++;
    }
    
  if (real_count eq 0)
   return(RmErrno = RmE_BadArgument);

  details = Malloc(real_count * sizeof(ProcessorDetails));
  if (details eq Null(ProcessorDetails))
   return(RmErrno = RmE_NoMemory);

  updates = Malloc(real_count * sizeof(ProcessorUpdate));
  if (updates eq Null(ProcessorUpdate))
   { Free(details); return(RmErrno = RmE_NoMemory); }
      
  for (i = 0, real_count = 0; i < count; i++)
   if (processors[i] ne (RmProcessor) NULL)
    if ((rc = fill_in_details(processors[i], details, &real_count))
        ne RmE_Success)
     { Free(details); Free(updates); return(RmErrno = rc); }
    
  rc = GeneralOp(real_count, details, updates, op);
  if ((rc eq RmE_Success) || (rc eq RmE_PartialSuccess))
   for (i = 0; i < real_count; i++)
    for (j = 0; j < count; j++)
     { if (processors[j] eq (RmProcessor) NULL) continue;
       if (processors[j]->Uid eq updates[i].Uid)
        { processors[j]->ObjNode.Size = updates[i].State;
          processors[j]->Purpose = updates[i].Purpose;
          break;
        }
     }
  else
   RmErrno = rc;
   
  Free(details);
  Free(updates);
  return(rc);
}

static	int	fill_in_details(RmProcessor processor, ...)
{ va_list		args;
  ProcessorDetails	*details;
  int			*real_count;
  
  if (processor->StructType ne RmL_Obtained)
   return(RmE_NoAccess);
  va_start(args, processor);
  details = va_arg(args, ProcessorDetails *);
  real_count = va_arg(args, int *);
  va_end(args);

	/* processors which are not set to native are ignored by these	*/
	/* routines.							*/
  if (RmGetProcessorPurpose(processor) ne RmP_Native)
   details[*real_count].Uid	= RmL_NoUid;
  else  
   { details[*real_count].Uid	= processor->Uid;
    details[*real_count].Cap	= processor->NsCap;
   }
  *real_count += 1;
  return(RmE_Success);
}

static	int	GeneralOp(int count, ProcessorDetails *details, 
		ProcessorUpdate *updates, int op)
{ int		rc;
  RmRequest	request;
  RmReply	reply;

  Clear(request); Clear(reply);
  request.FnRc		= op;
  request.Arg1		= count;
  request.VariableSize	= count * sizeof(ProcessorDetails);
  request.VariableData	= (BYTE *) details;
  reply.VariableData	= (BYTE *) updates;
  
  rc = RmXch(&RmParent, &request, &reply);
  return(rc);
}
/*}}}*/
#else
# ifdef __C40
/*{{{  Stubs needed for backward compatibility */
int RmRebootNetwork(RmNetwork network)
{
	network = network; return(RmE_YouMustBeJoking);
}

int RmRebootProcessors(int count, RmProcessor *processors)
{
	count = count; processors = processors; return(RmE_YouMustBeJoking);
}

int RmReconfigureNetwork(RmNetwork network, bool exact, bool preserve)
{
	network = network; exact = preserve; return(RmE_YouMustBeJoking);
}

int RmReconfigureProcessors(int count, RmProcessor *processors, bool exact, bool preserve)
{
	count = count; processors = processors; exact = preserve; return(RmE_YouMustBeJoking);
}

int RmResetNetwork(RmNetwork network)
{
	network = network; return(RmE_YouMustBeJoking);
}

int RmResetProcessors(int count, RmProcessor *processors)
{
	count = count; processors = processors; return(RmE_YouMustBeJoking);
}

int RmRevertNetwork(RmNetwork network)
{
	network = network; return(RmE_YouMustBeJoking);
}

int RmRevertProcessors(int count, RmProcessor *processors)
{
	count = count; processors = processors; return(RmE_YouMustBeJoking);
}

bool RmIsLinkPossible(RmProcessor source, int sourcelink, RmProcessor dest, int destlink)
{
	source = dest; sourcelink = destlink; return(FALSE);
}

bool RmIsNetworkPossible(RmNetwork network, bool exact, bool preserve)
{
	network = network; exact = preserve; return(FALSE);
}

bool RmAreProcessorsPossible(int count, RmProcessor *processors, bool exact, bool preserve)
{
	count = count; processors = processors; exact = preserve; return(FALSE);
}

int RmSetNetworkNative(RmNetwork network)
{
	network = network; return(RmE_YouMustBeJoking);
}

int RmSetProcessorsNative(int count, RmProcessor *processors)
{
	count = count; processors = processors; return(RmE_YouMustBeJoking);
}

/*}}}*/
# endif
#endif
/*{{{  Session and application ids */
/**-----------------------------------------------------------------------------
*** The processor identification routines requested by Telmat. The routines
*** RmGetSession() and RmGetApplication() are almost identical and
*** always go to the parent, usually the TFM, but if arg1 is set then
*** the TFM will forward the request to the Network Server.
**/
int	RmGetProcessorSession(RmProcessor Processor)
{
  CheckProcessorFail(Processor, -1);
  if ((Processor->StructType ne RmL_Existing) &&
      (Processor->StructType ne RmL_Obtained))
   { RmErrno = Processor->Errno = RmE_BadArgument; return(-1); }
  return(Processor->SessionId);
}

int RmGetProcessorApplication(RmProcessor Processor)
{
  CheckProcessorFail(Processor, -1);
  if ((Processor->StructType ne RmL_Existing) &&
      (Processor->StructType ne RmL_Obtained))
   { RmErrno = Processor->Errno = RmE_BadArgument; return(-1); }
  return(Processor->ApplicationId);
}

int RmGetSession()
{ RmRequest	request;
  RmReply	reply;
  int		result;

  Clear(request); Clear(reply);
  request.FnRc	= RmC_GetId;
  request.Arg1	= 1;
  result	= RmXch(&RmParent, &request, &reply);
  if (result eq RmE_Success)
   return(reply.Reply1);
  else
   { RmErrno = result; return(-1); }
}

int RmGetApplication()
{ RmRequest	request;
  RmReply	reply;
  int		result;

  Clear(request); Clear(reply);
  request.FnRc	= RmC_GetId;
  request.Arg1	= 0;
  result	= RmXch(&RmParent, &request, &reply);
  if (result eq RmE_Success)
   return(reply.Reply1);
  else
   { RmErrno = result; return(-1); }
}
/*}}}*/
