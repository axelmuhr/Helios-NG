/*------------------------------------------------------------------------
--									--
--			H E L I O S					--
--			-----------					--
--									--
--		Copyright (C) 1990, Perihelion Software Ltd.		--
--			All Rights Reserved.				--
--									--
--	null_ra.c							--
--		  A null driver. This claims to reset every processor,	--
--	but actually does nothing.					--
--									--
--	Author : BLV, 24.8.90						--
--									--
------------------------------------------------------------------------*/
/* RcsId: $Header: /hsrc/network/RCS/null_ra.c,v 1.9 1993/08/12 13:52:31 nickc Exp $*/

#define	VersionNumber "1.03"

#ifdef __TRAN
#pragma -f0		/* switch off vector stack			*/
#pragma -g0		/* and do not put the names into the code	*/
#endif

#ifdef STACKCHECK
#ifdef __TRAN
#pragma -s1
static void _stack_error(Proc *p)
{ IOdebug("null_ra.d: stack overflow in %s at %x",p->Name,&p);
  Exit(0x0080 | SIGSTAK);
}
#endif
#pragma	-s0
#else
#pragma -s1
#endif

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

DCB	*DevOpen(MPtr dev, NetworkDCB *network_dcb)
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
		request->FnRc = (int)(EC_Error + SS_NetServ + EG_WrongFn);
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

#ifdef __C40
  char		buf[80];
  int		format_mp, code_mp;

  	/* format_mp is a C pointer which may be invalid. It refers to	*/
  	/* a word-aligned pointer within the code.			*/
  format_mp   = (int)  format;
  format_mp >>= 2;
  	/* subtract ir0, to cope with funny memory layouts.		*/
  format_mp  += (int) _DataToFuncConvert(NULL);
  	/* work out the top two bits of the mptr, using the address of	*/
  	/* a function which is known to be within the same piece of	*/
  	/* memory as the code.						*/
  code_mp     = (int) &driver_report;
  code_mp    &= 0xC0000000;
  format_mp  |= code_mp;
  	/* put them together, and you get an mptr for the string.	*/
  MP_GetData(buf, format_mp, 0, 80 / sizeof(int));
  format      = buf;
#endif
 
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

#ifdef __C40
  char		buf[80];
  int		format_mp, code_mp;
  format_mp   = (int)  format;
  format_mp >>= 2;
  format_mp  += (int) _DataToFuncConvert(NULL);
  code_mp     = (int) &driver_report;
  code_mp    &= 0xC0000000;
  format_mp  |= code_mp;
  MP_GetData(buf, format_mp, 0, 80 / sizeof(int));
  format      = buf;
#endif
  
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
*** Initialise(). For every processor except the root processor and
*** I/O processors, indicate that a definite reset is available.
*** This is of course a big whopper.
**/
static void	driver_Initialise(NetworkDCB *device, DriverRequest *request)
{ RmHardwareFacility	*hardware = device->HardwareFacility;
  int			i, j;

  driver_report(device, "null_ra.d driver, version %W", VersionNumber);

  request->FnRc = (int) Err_Null;
  for (i = 0; i < hardware->NumberProcessors;i++)
   { RmProcessor	Processor = hardware->Processors[i];
     ProcessorEntry	*proc_entry;
     
     if (Processor eq (RmProcessor) device->RootProcessor) continue;
     if ((RmGetProcessorPurpose(Processor) & RmP_Mask) eq RmP_IO)
      continue;
     
     proc_entry = (ProcessorEntry *) RmGetProcessorPrivate(Processor);
     for (j = 0; j < proc_entry->NumberDrivers; j++)
      { DriverEntry *driver = &(proc_entry->DriverEntry[j]);
        if (driver->Hardware ne hardware) continue;
        driver->Flags |= DriverFlags_DefiniteReset;
      }
   }
}

/**
*** Reset the processor or processors specified in the request. For the
*** null device this simply means specifying that the requested processors
*** have been reset.
***
*** After an initial reset, the DefiniteReset flag is cleared. This stops
*** network show etc. displaying garbage information.
**/
static void	driver_Reset(NetworkDCB *device, DriverRequest *request)
{ RmHardwareFacility	*hardware = device->HardwareFacility;
  int	i, j;

  for (i = 0; i < request->NumberProcessors; i++)
   { RmProcessor Processor = request->Processor[i];
     int	 state	   = RmGetProcessorState(Processor);
     state |= RmS_Reset;
     (void) RmSetProcessorState(Processor, state);
   }

  for (i = 0; i < hardware->NumberProcessors; i++)
   { RmProcessor	processor	= hardware->Processors[i];
     ProcessorEntry	*proc_entry;

     if (processor eq (RmProcessor) device->RootProcessor) continue;
     if ((RmGetProcessorPurpose(processor) & RmP_Mask) eq RmP_IO)
      continue;

     proc_entry = (ProcessorEntry *) RmGetProcessorPrivate(processor);
     for (j = 0; j < proc_entry->NumberDrivers; j++)
      { DriverEntry *driver = &(proc_entry->DriverEntry[j]);
        if (driver->Hardware ne hardware) continue;
        driver->Flags &= ~DriverFlags_DefiniteReset;
      }
     processor->Control &= ~(RmC_Native | RmC_Reset);
   }

  request->FnRc = (int) Err_Null;
  device = device;
}

/**
*** Analyse the specified processors. For the null device this means
*** claiming to have reset them.
**/
static void	driver_Analyse(NetworkDCB *device, DriverRequest *request)
{ int	i;

  for (i = 0; i < request->NumberProcessors; i++)
   { RmProcessor Processor = request->Processor[i];
     int	 state	   = RmGetProcessorState(Processor);
     state |= RmS_Reset;
     (void) RmSetProcessorState(Processor, state);
   }
  request->FnRc = (int) Err_Null;
  device = device;
}

static void	driver_TestReset(NetworkDCB *device, DriverRequest *request)
{ driver_fatal(device, "null_ra.d, driver TestReset routine called illegally");
  request = request;
}

static void	driver_Boot(NetworkDCB *device, DriverRequest *request)
{ driver_fatal(device, "null_ra.d, driver bootstrap routine called illegally");
  request = request;
}

static void driver_ConditionalReset(NetworkDCB *device, DriverRequest *request)
{ driver_fatal(device,
               "null_ra.d, driver conditional reset routine called illegally");
  request = request;
}
