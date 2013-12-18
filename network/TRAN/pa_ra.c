/*------------------------------------------------------------------------
--									--
--			H E L I O S					--
--			-----------					--
--									--
--		Copyright (C) 1990, Perihelion Software Ltd.		--
--			All Rights Reserved.				--
--									--
--	pa_ra.c								--
--		Reset/analyse driver for the Parsytec reset scheme,	--
--	supporting both SuperClusters and Megaframes.			--
--									--
--	Author : BLV, 15.8.90						--
--									--
------------------------------------------------------------------------*/

static char *rcsid = "$Header: /users/bart/hsrc/network/TRAN/RCS/pa_ra.c,v 1.6 1992/05/08 16:44:41 bart Exp $";

#define	VersionNumber "1.02"

/**
*** History :
***	     1.00, initial version
***	     1.01, Helios 1.3 alpha, fixed bug in bootstrap re. freeing
***		   configuration vector.
***	     1.02, changed definition of XchNetworkAgent()
**/

#pragma -s1		/* switch off stack checking			*/
#pragma -f0		/* switch off vector stack			*/
#pragma -g0		/* and do not put the names into the code	*/

#include <syslib.h>
#include <device.h>
#include <codes.h>
#include <root.h>
#include <gsp.h>
#include <module.h>
#include <stdarg.h>
#include <string.h>
#include "private.h"
#include "rmlib.h"
#include "netaux.h"

#ifdef Malloc
#undef Malloc
#endif

/**
*** Machine independent routines.
**/
static void	DeviceOperate(DCB *, DriverRequest *);
static word	DeviceClose(DCB *);

/**
*** Machine specific routines
**/
static void	driver_Initialise(	NetworkDCB *, DriverRequest *);
static void	driver_Reset(		NetworkDCB *, DriverRequest *);
static void	driver_Analyse(		NetworkDCB *, DriverRequest *);
static void	driver_TestReset(	NetworkDCB *, DriverRequest *);
static void	driver_Boot(		NetworkDCB *, DriverRequest *);
static void	driver_ConditionalReset(NetworkDCB *, DriverRequest *);

DCB	*DevOpen(Device *dev, NetworkDCB *network_dcb)
{
  network_dcb->DCB.Device	= dev;
  network_dcb->DCB.Operate	= &DeviceOperate;
  network_dcb->DCB.Close	= &DeviceClose;
  return(&(network_dcb->DCB));  
}

static void DeviceOperate(DCB *device, DriverRequest *request)
{ NetworkDCB	*network_dcb = (NetworkDCB *) device;

  switch(request->FnRc)
   {
   	case	ND_Initialise	: 
   		driver_Initialise(network_dcb, request); break;
	case	ND_Reset	:
		driver_Reset(network_dcb, request); break;
	case	ND_Analyse	:
		driver_Analyse(network_dcb, request); break;
	case	ND_TestReset	:
		driver_TestReset(network_dcb, request); break;
	case	ND_Boot		:
		driver_Boot(network_dcb, request); break;	
	case	ND_ConditionalReset :
		driver_ConditionalReset(network_dcb, request); break;
	default	:
		request->FnRc = EC_Error + SS_NetServ + EG_WrongFn;
   }
}

static word DeviceClose(DCB *device)
{ device = device;
  return(Err_Null);
}

/**
*** Access to routines in the network server, if needed
**/
static void driver_report(NetworkDCB *device, char *format, ...)
{ va_list	args;
  NetworkFuncs	*Functions = device->Functions;
  int		arg1, arg2, arg3, arg4, arg5;
  
   va_start(args, format);
   arg1 = va_arg(args, int);
   arg2 = va_arg(args, int);
   arg3 = va_arg(args, int);
   arg4 = va_arg(args, int);
   arg5 = va_arg(args, int);
   va_end(args);
   (*(Functions->report))(format, arg1, arg2, arg3, arg4, arg5);
}

static word driver_BuildConfig(NetworkDCB *device, RmProcessor source,
	RmProcessor dest, int destlink, Config **config_vec, word *confsize)
{ NetworkFuncs	*Functions	= device->Functions;
  word		temp;
  
  temp = (*(Functions->BuildConfig))(source, dest, destlink, config_vec, confsize);
  return(temp);
}

static word driver_StartNetworkAgent(NetworkDCB *device, RmProcessor processor)
{ NetworkFuncs	*Functions	= device->Functions;
  word		temp;
  
  temp = (*(Functions->StartNetworkAgent))(processor);
  return(temp);
}

static word driver_StopNetworkAgent(NetworkDCB *device, RmProcessor processor)
{ NetworkFuncs	*Functions	= device->Functions;
  word		temp;
  
  temp = (*(Functions->StopNetworkAgent))(processor);
  return(temp);
}

static word driver_XchNetworkAgent(NetworkDCB *device,
	RmProcessor processor, NA_Message *message,
	bool get_rc, int rsize, BYTE *rdata)
{ NetworkFuncs	*Functions	= device->Functions;
  word		temp;

  temp = (*(Functions->XchNetworkAgent))(processor, message,
		get_rc, rsize, rdata);
  return(temp);
}

/**
*** The Parsytec hardware allows limited reset control only. A processor
*** can be reset if and only if it currently has a Helios neighbour,
*** which is not easy to test here. Parsytec transputers need a slightly
*** different bootstrap routine.
**/
static void	driver_Initialise(NetworkDCB *device, DriverRequest *request)
{ RmHardwareFacility	*hardware;
  int			i, j;
  RmProcessor		Processor;
  ProcessorEntry	*proc_entry;
  
  driver_report(device, "pa_ra.d driver, version %s", VersionNumber);

  request->FnRc = Err_Null;
  hardware = device->HardwareFacility;

	/* for every processor controlled by this driver */
  for (i = 0; i < hardware->NumberProcessors; i++)
   { Processor	= hardware->Processors[i];
   	/* nobody controls the root processor */
     if (Processor eq (RmProcessor) device->RootProcessor) continue;

	/* find the info slot corresponding to this driver */
     proc_entry = (ProcessorEntry *) RmGetProcessorPrivate(Processor);
     for (j = 0; j < proc_entry->NumberDrivers; j++)
      { DriverEntry *driver = &(proc_entry->DriverEntry[j]);
        if (driver->Hardware ne hardware) continue;
        /* and fill it in */
        driver->Flags |= (DriverFlags_PossibleReset + DriverFlags_SpecialBootstrap);
	break;
      }
   }
}

/**
*** Reset the processor or processors specified in the request.
*** For now only support this if the request specifies a single
*** processor. If the processor is in the middle of being booted
*** forget the reset, because it will happen during the bootstrap
*** stage.
***
*** Eventually this should be improved to allow resetting a set of
*** processors at once, figuring out a suitable order in which to do so.
**/
static void	driver_Reset(NetworkDCB *device, DriverRequest *request)
{ RmProcessor		processor = request->Processor[0];
  RmProcessor		neighbour;
  int			destlink;
  int			number_links;
  int			i;
  int			state;
  NA_Message		message;
    
  if (request->NumberProcessors > 1)
   { request->FnRc = EC_Error + SS_NetServ + EG_WrongSize + EO_Message;
     return;
   }

  state = RmGetProcessorState(processor);
  if (state & RmS_Booting)
   { request->FnRc = Err_Null; return; }

	/* Try to find a neighbouring processor that is up and running */     
  number_links = RmCountLinks(processor); 
  for (i = 0; i < number_links; i++)
   { neighbour = RmFollowLink(processor, i, &destlink);
     if ((neighbour eq RmM_NoProcessor) || (neighbour eq RmM_ExternalProcessor))
      continue;
     state = RmGetProcessorState(neighbour);
     unless(state & RmS_Running) continue;

	/* Use the network agent to perform the reset. Some		*/
	/* optimisation is possible here. First check if the network	*/
	/* agent is already running on any of the neighbours, and if so	*/
	/* use that. Otherwise driver_rexec(/helios/bin/pa_reset) on	*/
	/* a running neighbour. This is not worthwhile just now.	*/
     unless(driver_StartNetworkAgent(device, neighbour)) continue;

     message.FnRc	= NA_ParsytecReset;
     message.Arg1	= destlink;
     message.Size	= 0;
     (void) driver_XchNetworkAgent(device, neighbour, &message, FALSE, 0, NULL);

     driver_StopNetworkAgent(device, neighbour);
     state = RmGetProcessorState(processor);
     state |= RmS_Reset;
     RmSetProcessorState(processor, state);
     request->FnRc	= Err_Null;
     return;
   }

  request->FnRc	= EC_Error + SS_NetServ + EG_Unknown + EO_Link;
}

/**
*** Analyse the processor(s) specified. This is not actually used at
*** present, but I may have to put it in again at a later stage.
*** The parsytec reset is actually an analyse, so the same routine is
*** used.
**/
static void	driver_Analyse(NetworkDCB *device, DriverRequest *request)
{ driver_Reset(device, request);
}

/**
*** The code for a real reset will do for now.
**/
static void driver_ConditionalReset(NetworkDCB *device, DriverRequest *request)
{ driver_Reset(device, request);
}

/**
*** At present a reset is only possible if the request is for a single
*** processor, and if that processor has an active Helios neighbour.
**/
static void	driver_TestReset(NetworkDCB *device, DriverRequest *request)
{ RmProcessor	processor	= request->Processor[0];
  RmProcessor	neighbour;
  int		destlink;
  int		number_links;
  int		i;
  int		state;

  if (request->NumberProcessors > 1)
   { request->FnRc = EC_Error + SS_NetServ + EG_WrongSize + EO_Message;
     return;
   }

  number_links = RmCountLinks(processor);
  for (i = 0; i < number_links; i++)
  { neighbour = RmFollowLink(processor, i, &destlink);
    if ((neighbour eq RmM_NoProcessor) || (neighbour eq RmM_ExternalProcessor))
     continue;
    state = RmGetProcessorState(neighbour);
    if (state & RmS_Running)
     { request->FnRc	= Err_Null;
       return;
     }
   }
  request->FnRc = EC_Error + SS_NetServ + EG_Unknown + EO_Link;
  device = device;
}

/**
*** Parsytec hardware does require a special bootstrap. In fact it
*** needs to send the standard bootstrap request to a network agent,
*** with request Parsytec boot instead of transputer boot. The request
*** contains the booting processor and the link number.
**/
static void	driver_Boot(NetworkDCB *device, DriverRequest *request)
{ RmProcessor		start		= request->Processor[0];
  int			link		= request->NumberProcessors;
  RmProcessor		target;
  int			destlink;
  Config		*config		= Null(Config);
  word			confsize;
  NA_Message		message;
  word			reply;
  
  target = RmFollowLink(start, link, &destlink);
  unless(driver_BuildConfig(device, start, target, destlink, &config, &confsize)) 
   { reply = EC_Warn + SS_NetServ + EG_NoMemory + EO_Message; goto fail; }

  message.FnRc		= NA_ParsytecBoot;
  message.Arg1		= link;
  message.Size		= confsize;
  message.Data		= (BYTE *) config;

  reply = driver_XchNetworkAgent(device, start, &message, TRUE, 0, NULL);  

  if (reply ne Err_Null)
   { driver_report(device, "pa_ra.d, failed to boot /%s via link %d of /%s, fault %x",
   		RmGetProcessorId(target), link, RmGetProcessorId(start), reply);
     goto fail;
   }

  request->FnRc	  = Err_Null;
  Free(config);
  return;
  
fail:
  if (config ne Null(Config)) Free(config);
  request->FnRc	 = EC_Error + EG_Broken + EO_Processor;;
}


