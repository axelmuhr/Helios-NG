/*------------------------------------------------------------------------
--									--
--			H E L I O S					--
--			-----------					--
--									--
--		Copyright (C) 1990, Perihelion Software Ltd.		--
--			All Rights Reserved.				--
--									--
--	rte_ra.c							--
--		Reset/analyse driver for the Meiko computing surface.	--
--	This driver accesses a device /NetworkController inside the	--
--	Meiko I/O Server to perform all the operations.			--
--									--
--	Author : BLV, 28.8.91						--
--	Based on code by JMP						--
--									--
------------------------------------------------------------------------*/

static char *rcsid = "$Header: /users/bart/hsrc/network/meiko/TRAN/RCS/rte_ra.c,v 1.2 1992/01/14 14:27:02 bart Exp $";

#define	VersionNumber	"1.00"
/**
*** Version Numbers :
***	1.00, first experimental version
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
#include "../private.h"
#include "../rmlib.h"
#include "../netaux.h"

#ifdef Malloc
#undef Malloc
#endif

#ifdef HELIOS1_2
#define RmGetProcessorId RmGetProcessorID
#endif

#define		ControlStream	Spare[0]

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
{ NetworkDCB	*network_dcb = (NetworkDCB *) device;
  Stream	*Controller  = (Stream *) network_dcb->ControlStream;
  if (Controller ne Null(Stream)) Close(Controller);
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

static void driver_fatal(NetworkDCB *device, char *format, ...)
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
   (*(Functions->fatal))(format, arg1, arg2, arg3, arg4, arg5);
}

/**
*** With the Meiko driver the only legal way of using the driver is to
*** have a single reset driver and a single configuration driver, rte_ra.d
*** and rte_c.d, for every processor except the I/O processor. This must
*** be checked. Then every processor is given a sequence number, for
*** subsequent requests to the NetworkController device. This should be similar
*** to the old sid field, i.e. 0 for the root processor (always assumed but
*** not actually true), 1 for the next, etc. The I/O processor if any does
*** not get a number. Finally a stream is opened to the /NetworkController
*** server.
**/
static int	initialise_aux1(RmProcessor, ...);

static void	driver_Initialise(NetworkDCB *device, DriverRequest *request)
{ RmNetwork		Network = device->Net;
  int			sequence_number = 1;
  RmHardwareFacility	*hardware;  
  Stream		*controller;
  Object		*control_obj;
    
  driver_report(device, "rte_ra.d driver version %s", VersionNumber);

  hardware = device->HardwareFacility;
  if (strlen(hardware->Option) ne 0)
   driver_report(device, "rte_ra.d driver, ignoring option %s", 
   		hardware->Option);

  if (RmSearchNetwork(Network, &initialise_aux1, &sequence_number, device)
 	ne RmE_Success)
   driver_fatal(device, "rte_ra.d driver, unable to work in this network");
   
  control_obj = Locate(Null(Object), "/NetworkController");
  if (control_obj eq Null(Object))
   driver_fatal(device, "rte_ra.d driver, failed to locate /NetworkController");

  controller = Open(control_obj, Null(char), O_ReadOnly);
  if (controller eq Null(Stream))
   driver_fatal(device,
   		"rte_ra.d driver, failed to open /NetworkController, fault %x", 
		Result2(control_obj));

  device->ControlStream	= (int) controller;
  Close(control_obj);
  request->FnRc = Err_Null;
}

static int	initialise_aux1(RmProcessor Processor, ...)
{ va_list		args;
  int			*sequence_number;
  NetworkDCB		*device;
  ProcessorEntry	*proc_entry;
  int			temp;
      
  va_start(args, Processor);
  sequence_number = va_arg(args, int *);
  device	  = va_arg(args, NetworkDCB *);
  va_end(args);

  if (RmIsNetwork(Processor))
   return(RmSearchNetwork((RmNetwork) Processor, &initialise_aux1,
   		sequence_number, device));

	/* I/O processors cannot be reset or configured.	*/
#ifdef HELIOS1_2  
  if (RmGetProcessorPurpose(Processor) eq RmP_IO)
#else
  if ((RmGetProcessorPurpose(Processor) & RmP_Mask) eq RmP_IO)
#endif
   return(0);
  
	/* There must be exactly two drivers for every processor.	*/
  proc_entry = (ProcessorEntry *) RmGetProcessorPrivate(Processor);
  if (proc_entry->NumberDrivers ne 2)
   { driver_report(device,
   	 "rte_ra.d driver, processor %s has wrong number of network drivers",
   	 RmGetProcessorId(Processor));
     return(1);
   }

	/* driver[0] and driver[1] should be the reset and configure driver */
	/* (or the other way around).					    */
  if (proc_entry->DriverEntry[0].Hardware eq device->HardwareFacility)
   temp = 0;
  elif (proc_entry->DriverEntry[1].Hardware eq device->HardwareFacility)
   temp = 1;
  else
   { driver_report(device, "rte_ra.d driver has no control over processor %s",
		RmGetProcessorId(Processor));
     return(1);
   }

	/* rte_ra.d has full reset over its processor	*/
  proc_entry->DriverEntry[temp].Flags |= DriverFlags_DefiniteReset;

	/* The root processor has id 0, the others have ids 1 onwards.	*/	
	/* This id is filled in for both the reset and the configuration*/
	/* driver.							*/
  if (Processor eq device->RootProcessor)
   proc_entry->DriverEntry[0].Aux1 = 0;
  else
   proc_entry->DriverEntry[0].Aux1 = *sequence_number++;
  proc_entry->DriverEntry[1].Aux1 = proc_entry->DriverEntry[0].Aux1;

  return(0);
}

/**
*** Resetting one or more processors involves sending a message to the
*** open stream to /NetworkController. This is pretty horrible in case
*** things go wrong, but hopefully the retries and ReOpen() code will
*** take care of any problems.
**/
#define	NC_Reset		0x2010L
#define NC_Analyse		0x2020L

static void	driver_Reset(NetworkDCB *device, DriverRequest *request)
{ MCB		mcb;
  Port		reply = NewPort();
  WORD		vec[10];
  WORD		*real_vec = vec;
  int		i;
  int		retries;
  Stream	*controller = (Stream *) device->ControlStream;
    
  if (request->NumberProcessors > 10)
   { real_vec = (WORD *) Malloc(sizeof(WORD) * request->NumberProcessors);
     if (real_vec eq Null(WORD))
      { request->FnRc = EC_Error + SS_NetServ + EG_NoMemory + EO_Message;
        goto done;
      }
   }
  for (i = 0; i < request->NumberProcessors; i++)
   { RmProcessor	Processor  = request->Processor[i];
     ProcessorEntry	*proc_entry;
     proc_entry  = (ProcessorEntry *) RmGetProcessorPrivate(Processor);
     real_vec[i] = proc_entry->DriverEntry[0].Aux1;
   }

  for (retries = 0; retries < 2; retries++)
   { mcb.Data			= (BYTE *) real_vec;
     mcb.Control		= Null(WORD);
     mcb.Timeout		= 10 * OneSec;
     mcb.MsgHdr.DataSize	= sizeof(WORD) * request->NumberProcessors;
     mcb.MsgHdr.ContSize	= 0;
     mcb.MsgHdr.Flags		= MsgHdr_Flags_preserve;
     mcb.MsgHdr.Dest		= controller->Server;
     mcb.MsgHdr.Reply		= reply;
     mcb.MsgHdr.FnRc		= FC_Private + NC_Reset;
     if ((request->FnRc	= PutMsg(&mcb)) < Err_Null)
       { ReOpen(controller); continue; }
     mcb.MsgHdr.Dest		= mcb.MsgHdr.Reply;
     mcb.Timeout		= 10 * OneSec;
     request->FnRc = GetMsg(&mcb);
     if (request->FnRc eq Err_Null) 
      { for (i = 0; i < request->NumberProcessors; i++)
         { RmProcessor	Processor = request->Processor[i];
           int		state = RmGetProcessorState(Processor);
           state |= RmS_Reset;
           (void) RmSetProcessorState(Processor, state);
         }
        break;
      }
     if (retries eq 0) ReOpen(controller);
   }

done:
  if ((real_vec ne vec) && (real_vec ne Null(WORD))) Free(real_vec);    
  if (reply ne NullPort) FreePort(reply);
}

/**
*** Analyse need not be supported for now, since the Network Server does
*** not invoke it. Should it be needed then it would be almost a
*** complete copy of the reset code.
**/
static void	driver_Analyse(NetworkDCB *device, DriverRequest *request)
{ driver_fatal(device, "rte_ra.d, driver Analyse routine called illegally");
  request = request;
}

/**
*** The RTE driver always has full reset over all the processors, so this
*** routine should never be called.
**/
static void	driver_TestReset(NetworkDCB *device, DriverRequest *request)
{ driver_fatal(device, "rte_ra.d, driver TestReset routine called illegally");
  request = request;
}

/**
*** The RTE driver does not set the special bootstrap flag, so this routine
*** should never get called.
**/
static void	driver_Boot(NetworkDCB *device, DriverRequest *request)
{ driver_fatal(device, "rte_ra.d, driver bootstrap routine called illegally");
  request = request;
}

/**
*** The RTE driver sets the full reset flag so this routine should never
*** get called.
**/
static void driver_ConditionalReset(NetworkDCB *device, DriverRequest *request)
{ driver_fatal(device,
               "rte_ra.d, driver conditional reset routine called illegally");
  request = request;
}

