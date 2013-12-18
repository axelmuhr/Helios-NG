/*------------------------------------------------------------------------
--									--
--			H E L I O S					--
--			-----------					--
--									--
--		Copyright (C) 1990, Perihelion Software Ltd.		--
--			All Rights Reserved.				--
--									--
--	tram_ra.c							--
--		Reset/analyse driver for Tram modules.			--
--	This does very little because of the restrictions of the	--
--	hardware.							--
--									--
--	Author : BLV, 15.8.90						--
--									--
------------------------------------------------------------------------*/

static char *rcsid = "$Header: /users/bart/hsrc/network/TRAN/RCS/tram_ra.c,v 1.4 1992/10/28 13:03:39 bart Exp $";

#define	VersionNumber	"1.01"

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

static RmProcessor driver_LookupProcessor(NetworkDCB *device, char *name)
{ RmNetwork	Network = device->Net;
  NetworkFuncs	*Functions  = device->Functions;
  word		temp;
  
  temp = (*(Functions->LookupProcessor))(Network, name);
  return((RmProcessor) temp);
}

static word driver_rexec(NetworkDCB *device, RmProcessor processor, 
	Object *code, Environ *env, word delay)
{ NetworkFuncs	*Functions	= device->Functions;
  word		rc;
  
  rc = (*(Functions->rexec))(processor, code, env, delay);
  return(rc);
}

/**
*** The hardware specific routines. These are documented separately.
**/

#define ControlProc	Spare[0]
static	word	do_reset(NetworkDCB *);

/**
*** driver_Initialise().
*** For the tram reset scheme the following is implemented.
*** 1) if the root processor is one of the controlled processors this cannot
***    be reset.
*** 2) if an option is given in the resource map specifying a particular
***    processor, this processor has the subsystem control and cannot be
***    reset
*** 3) if only one processor is left then the driver has full reset control
***    over this one.
*** 4) otherwise the driver has limited reset control over all the
***    remaining processors.
***
*** The essential field of the hardware facility should be filled in.
*** This indicates that a particular processor, usually the root, is
*** essential to the device driver's operation.
**/

static void	driver_Initialise(NetworkDCB *device, DriverRequest *request)
{ RmHardwareFacility 	*hardware;
  RmProcessor		Processor;
  ProcessorEntry	*proc_entry;
  int			i, j;
  int			affected;
  bool			reclaim	= FALSE;

	/* Sign-on message */
  driver_report(device, "tram_ra.d driver, version %s", VersionNumber);
  request->FnRc	= Err_Null;
  
	/* work out how many processors are affected. The root is never	*/
	/* affected, nor is the control processor if specified.		*/    
  hardware = device->HardwareFacility;
  affected = hardware->NumberProcessors;
  for (i = 0; i < hardware->NumberProcessors; i++)
   if (hardware->Processors[i] eq device->RootProcessor)
    { affected--; break; }

  device->ControlProc		= 0;
  hardware->Essential		= (RmProcessor) NULL;

	/* If a device driver option has been given, examine it */
  if (strlen(hardware->Option) > 0)
   { 
     if (!strcmp(hardware->Option, "reclaim"))
      { reclaim = TRUE; goto done_option; }

     if (hardware->Option[0] ne '/')
      { driver_report(device, "tram_ra.d driver, unexpected option %s",
       		hardware->Option);
        driver_report(device, "tram_ra.d driver, ignoring option");
        goto done_option;
      }
      	/* for tram_ra.d, the option should be a processor name */
     Processor = driver_LookupProcessor(device, &(hardware->Option[1]));
     if (Processor eq (RmProcessor) NULL)
      { driver_report(device,
        	      "tram_ra.d driver, failed to find control processor %s",
      		      hardware->Option);
      	goto done_option;
      }
      	/* the control processor is special, and is essential for resets */
	/* However, if it is the current processor ignore it.		 */
     if (Processor ne device->RootProcessor)	
      { device->ControlProc	= (int) Processor;
        hardware->Essential	= Processor;
      }
   }
   
done_option:

	/* If the device driver cannot do anything, tough */
  if (affected eq 0) return;

	/* For every processor affected, set the flag indicating what	*/
	/* can be done.							*/  
  for (i = 0; i < hardware->NumberProcessors; i++)
   { Processor	= hardware->Processors[i];
     if ((Processor eq (RmProcessor) device->RootProcessor) ||
         (Processor eq (RmProcessor) device->ControlProc))
      continue;
     proc_entry = (ProcessorEntry *) RmGetProcessorPrivate(Processor);
     for (j = 0; j < proc_entry->NumberDrivers; j++)
      { DriverEntry *driver = &(proc_entry->DriverEntry[j]);
        if (driver->Hardware ne hardware) continue;
        if (affected eq 1)
         driver->Flags	|= DriverFlags_DefiniteReset;
        else
         driver->Flags |= DriverFlags_PossibleReset;
	if (reclaim)
	 driver->Flags |= DriverFlags_Reclaim;
      }
   }
}

/**
*** Perform a subsystem reset, locally if possible, otherwise by remotely
*** executing tr_reset.
**/
static void	driver_Reset(NetworkDCB *device, DriverRequest *request)
{ RmHardwareFacility *hardware;
  int		i;
  int		state;
  RmProcessor	Processor;

  hardware = device->HardwareFacility;
  request->FnRc = do_reset(device);
  if (request->FnRc eq Err_Null)
   for (i = 0; i < hardware->NumberProcessors; i++)
    { Processor	= hardware->Processors[i];
      if ((Processor eq device->RootProcessor) ||
          (Processor eq (RmProcessor) device->ControlProc))
       continue;
      state = RmGetProcessorState(Processor);
      state |= RmS_Reset;
      (void) RmSetProcessorState(Processor, state);
    }
}

/**
*** Perform the actual reset. If there is a controlling processor different
*** from the root processor, the driver runs tr_reset remotely. Otherwise
*** a subsystem reset is performed locally.
**/
#define		Subsystem_Reset		0x00000000L
#define		Subsystem_Analyse 	0x00000004L
#define		Subsystem_Error		0x00000000L

static word	do_reset(NetworkDCB *device)
{ 
  if ((device->ControlProc eq NULL) ||
      (device->ControlProc eq (int) device->RootProcessor))
   { uword	*reg;

	/* Step 1 : force analyse low, to get into a known state */
     reg = (uword *) Subsystem_Analyse;
     *reg = 0;
     Delay(10000);	/* 10 Msec */
  
 	/* Step 2 : force analyse high, to start the reset */
     *reg = 1;
     Delay(10000);
  
 	/* Step 3 : assert the reset */
     reg = (uword *) Subsystem_Reset;
     *reg = 1;
     Delay(10000);
  
  	/* Step 4 : release the reset */
     *reg = 0;
     Delay(10000);
  
 	/* Step 5 : release the analyse */
     reg = (uword *) Subsystem_Analyse;
     *reg = 0;
     
     return(Err_Null);
   }
  else
   { Object	*prog = Locate(Null(Object), "/helios/bin/tr_reset");
     Environ	env;
     Stream	*Strv[1];
     char	*Argv[1];
     char	*Envv[1];
     Object	*Objv[1];
     word	rc;
          
     if (prog eq Null(Object))
      { driver_report(device, 
      		"tram_ra.d, failed to locate program /helios/bin/tr_reset");
        return(EC_Error + SS_NetServ + EG_Unknown + EO_Program);
      }
     Strv[0]	= Null(Stream);
     Objv[0]	= Null(Object);
     Argv[0]	= Null(char);
     Envv[0]	= Null(char);
     env.Strv	= Strv;
     env.Objv	= Objv;
     env.Argv	= Argv;
     env.Envv	= Envv;
     rc		= driver_rexec(device, (RmProcessor) device->ControlProc,
     	 		prog, &env, 5 * OneSec);
     Close(prog);
     if (rc ne Err_Null) driver_report(device,
     	"tram_ra.d warning, failed to execute program tr_reset, fault %x",
     	rc);
     return(rc);
   }     
     
}

/**
*** Analyse the processor(s) specified. This is not actually used at
*** present, but I may have to put it in again at a later stage.
*** Currently it is just a copy of driver_Reset().
**/
static void	driver_Analyse(NetworkDCB *device, DriverRequest *request)
{ RmHardwareFacility *hardware;
  int		i;
  int		state;
  RmProcessor	Processor;
 
  hardware = device->HardwareFacility;
  request->FnRc = do_reset(device);
  if (request->FnRc eq Err_Null)
   for (i = 0; i < hardware->NumberProcessors; i++)
    { Processor	= hardware->Processors[i];
      if ((Processor eq (RmProcessor) device->RootProcessor) ||
          (Processor eq (RmProcessor) device->ControlProc))
       continue;
      state = RmGetProcessorState(Processor);
      state |= RmS_Reset;
      (void) RmSetProcessorState(Processor, state);
    }
}

/**
*** To see whether or not a reset is possible, every processor is examined.
*** If all processors (except root and control) should be reset then
*** a reset is legal. The list of processors in the request can be ignored
*** because of the global nature of the tram reset. Even if all the processors
*** can be reset, unless the controlling processor is running there is no
*** chance.
**/
static void	driver_TestReset(NetworkDCB *device, DriverRequest *request)
{ RmHardwareFacility	*hardware;
  int		i;
  int		state;

  hardware = device->HardwareFacility;
  for (i = 0; i < hardware->NumberProcessors; i++)
   { RmProcessor Processor = hardware->Processors[i];
     if ((Processor eq device->RootProcessor) ||
         (Processor eq (RmProcessor) device->ControlProc))
      continue;
     state = RmGetProcessorState(Processor);
     unless(state & RmS_ShouldBeReset)
      { request->FnRc = EC_Error + SS_NetServ + EG_Protected + EO_Processor;
        return;
      }
   }
  if (device->ControlProc ne 0)
   { RmProcessor controller = (RmProcessor) device->ControlProc;
     state = RmGetProcessorState(controller);
     unless(state & RmS_Running)
      { request->FnRc = EC_Error + SS_NetServ + EG_State + EO_Processor;
        return; 
      }
   }
   
  request->FnRc = Err_Null;
}

/**
*** This function should not be called with the Tram reset scheme because
*** no special bootstrap mechanism is required.
**/
static void	driver_Boot(NetworkDCB *device, DriverRequest *request)
{ driver_fatal(device, "tram_ra.d, driver bootstrap routine called illegally");
  request = request;
}

/**
*** This does a similar loop to TestReset() above, but if successful the
*** reset is performed and the processors are updated.
**/
static void driver_ConditionalReset(NetworkDCB *device, DriverRequest *request)
{ RmHardwareFacility	*hardware;
  int		i;
  int		state;

  hardware = device->HardwareFacility;
  for (i = 0; i < hardware->NumberProcessors; i++)
   { RmProcessor Processor = hardware->Processors[i];
     if ((Processor eq device->RootProcessor) ||
         (Processor eq (RmProcessor) device->ControlProc))
      continue;
     state = RmGetProcessorState(Processor);
     unless(state & RmS_ShouldBeReset)
      { request->FnRc = EC_Error + SS_NetServ + EG_Protected + EO_Processor;
        return; 
      }
   }
  if (device->ControlProc ne 0)
   { RmProcessor controller = (RmProcessor) device->ControlProc;
     state = RmGetProcessorState(controller);
     unless(state & RmS_Running)
      { request->FnRc = EC_Error + SS_NetServ + EG_State + EO_Processor;
        return; 
      }
   }
   
  request->FnRc = do_reset(device);
  if (request->FnRc eq Err_Null)
   for (i = 0; i < hardware->NumberProcessors; i++)
    { RmProcessor Processor = hardware->Processors[i];
      if ((Processor eq device->RootProcessor) ||
          (Processor eq (RmProcessor) device->ControlProc))
       continue;
      state = RmGetProcessorState(Processor);
      state |= RmS_Reset;
      (void) RmSetProcessorState(Processor, state);
    }
}
