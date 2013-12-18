/*------------------------------------------------------------------------
-- 									--
-- 			H E L I O S					--
--			-----------					--
--									--
--		Copyright (C) 1990, Perihelion Software Ltd.		--
--		Copyright (C) 1990, Telmat Informatique			--
--			All Rights Reserved.				--
--									--
--	telmat_r.c							--
--		Reset/analyse driver for the Telmat T.Node. This 	--
--	version does not yet support larger machines such as the	--
--	Meganode.							--
--									--
--	Authors : BLV, 15.8.90						--
--		  Caroline Burrer et al, Telmat Informatique		--
--									--
------------------------------------------------------------------------*/

static char *rcsid = "$Header: /users/bart/hsrc/network/telmat/TRAN/RCS/telmat_r.c,v 1.2 1992/01/14 14:26:27 bart Exp $";

#define	VersionNumber	"1.01"

/**
*** Version numbers :
***	1.00, developed by Bart Veer, Caroline Burrer, Philippe Moliere,
***	      for Helios 1.2
***     1.01, Bart Veer, enhancements to support native networks
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

#include "TRAN/client.h"	/* for CBMan (control bus manager ) */
#include "TRAN/cbcom.h"
#include "TRAN/special.h"

#ifdef Malloc
#undef Malloc
#endif

#define SOLEIL_DBG	FALSE	/* debug telmat */

#define		ControlStream	Spare[0]
#define		Mutex		Spare[1]
#define		Silent		Spare[2]

#define TIMEOUT		( 5 * OneSec ) 	/* timeout for messages */

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
  network_dcb->Mutex 		= (int) New(Semaphore);
  InitSemaphore((Semaphore*)network_dcb->Mutex,1);
  return(&(network_dcb->DCB));  
}

static void DeviceOperate(DCB *device, DriverRequest *request)
{ NetworkDCB	*network_dcb = (NetworkDCB *) device;

  Wait ((Semaphore*)network_dcb->Mutex);
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
  Signal ((Semaphore *)network_dcb->Mutex);
}

static word DeviceClose(DCB *device)
{ NetworkDCB	*network_dcb = (NetworkDCB *) device;
  Stream	*controller  = (Stream *) network_dcb->ControlStream;

  Wait((Semaphore *) network_dcb->Mutex);
  if (controller ne Null(Stream)) Close(controller);
  Free((Semaphore *) network_dcb->Mutex);
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
   Signal((Semaphore*)&device->Mutex);
   (*(Functions->fatal))(format, arg1, arg2, arg3, arg4, arg5);
}

/**
*** Various utility routines.
***
*** 1) Locate the specified server with a timeout  20sec
**/
static void	LocateServer ( NetworkDCB *device , BYTE* name )
{
#define MAX_RETRY	10		
  int			locate_timedout = TRUE;
  Stream		*controller;
  Object		*control_obj;

    while (locate_timedout){
  	control_obj = Locate(Null(Object), name );
	if ( locate_timedout++ == MAX_RETRY )
		driver_fatal(device, "telmat_r.d driver, failed to locate %s ",name);
	if (control_obj eq Null(Object))
		driver_report(device, "telmat_r.d driver, failed to locate %s, retrying ... ",name);
	else (locate_timedout = 0);
	Delay(OneSec * 2); 
    }
    driver_report(device, "telmat_r.d driver, found %s ",name);

    controller = Open(control_obj, Null(char), O_ReadOnly);
    if (controller eq Null(Stream)) 
    	driver_fatal(device,"telmat_r.d driver, failed to open %s, fault %x", name, Result2(control_obj));
    device->ControlStream	= (int) controller;
}

/**
*** 2) Given a device driver and a specification, extract the correct
*** DriverEntry structure.
**/
static DriverEntry *ExtractDriver(NetworkDCB *device, RmProcessor Processor)
{ ProcessorEntry	*proc_entry;
  int			i;
  
  proc_entry = (ProcessorEntry *) RmGetProcessorPrivate(Processor);
  for (i = 0; i < proc_entry->NumberDrivers; i++)
   if (proc_entry->DriverEntry[i].Hardware eq device->HardwareFacility)
    return(&(proc_entry->DriverEntry[i]));

  driver_fatal(device, "telmat_r.d, corruption detected, processor %s's driver details have been lost.\n",
  		RmGetProcessorId(Processor));
  return(Null(DriverEntry));
}

/**
*** 3) and given a processor, work out the pin number.
**/
static int ExtractPin(NetworkDCB *device, RmProcessor Processor)
{ DriverEntry	*driver = ExtractDriver(device, Processor);
  return(driver->Aux1);
}

/**
*** Initialisation. First of all it is necessary to open a stream to the
*** control bus manager, otherwise nothing can be done. Then every processor is
*** given a pin number, to permit communication with CbMan. Note that it
*** is essential that the same code is used in telmat_r.d and telmat_c.d for
*** determining the pin numbers. The code for identifying special processors
*** is kept in a separate module and header, TRAN/special.[ch], to facilitate
*** upgrades with additional boards.
**/

static	void	initialise_aux1(RmProcessor, int *, NetworkDCB *, SPC_Processor *special_processor);

static void	driver_Initialise(NetworkDCB *device, DriverRequest *request)
{ int			sequence_number = 0;
  RmHardwareFacility	*hardware;  
  int			i;
  SPC_Processor		*special_processor; 
  
  driver_report(device, "telmat_r.d driver version %s", VersionNumber);
  LocateServer ( device, "/CbMan" );

	/* This builds a table of the special processor names and pins. */
  special_processor = (SPC_Processor*) Malloc(SPC_MAX_PROCESSOR * sizeof(SPC_Processor));
  if (special_processor eq Null(SPC_Processor))
   { request->FnRc = EC_Error + SS_NetServ + EG_NoMemory + EO_Link; return; }
  Initialise_all_special ( special_processor );

	/* The current reset driver can take the option silent, to	*/
	/* suppress generating too many messages.			*/
  device->Silent	= FALSE;
  hardware		= device->HardwareFacility;
  if (strlen(hardware->Option) ne 0)
   { if (!strcmp(hardware->Option, "silent"))
      device->Silent = TRUE;
     else
      driver_report(device, "telmat_r.d driver, ignoring option %s", 
   		hardware->Option);
   }
	/* This fills in the pin numbers for all controlled processors.	*/
  for (i = 0; i < hardware->NumberProcessors; i++)
   { RmProcessor Processor = hardware->Processors[i];
     initialise_aux1(Processor, &sequence_number, device, special_processor );
#if 0
     driver_report(device, "telmat_r.d, processor %s has Pin %d",
     		Processor->ObjNode.Name, ExtractPin(device, Processor));
#endif
   }   
/**
*** A check here that the sequence number is sensible, i.e. that the user
*** has not specified too many worker processors, might make sense.
**/   
  Free(special_processor);
  request->FnRc = Err_Null;
}

/**
*** For every processor, figure out an pin. For the workers in a T.Node these
*** are simple integers starting at 0. For a non-worker life is a little
*** bit more complicated.
**/
static void	initialise_aux1(RmProcessor Processor, int *sequence_number,
		NetworkDCB *device ,SPC_Processor *special_processor)
{ DriverEntry	*driver;
  int		i, attrib_count;
  int		test;
  char		*attrib_list[10];

  if ((RmGetProcessorPurpose(Processor) & RmP_Mask) eq RmP_IO) return;
	/* For the reset driver there is never any control over the root*/ 
	/* processor.							*/
  if (Processor eq device->RootProcessor) return;
  
 	/* find the correct driver */
  driver = ExtractDriver(device, Processor);
  driver->Flags |= (DriverFlags_DefiniteReset + DriverFlags_NativePossible);

	/* Check all the processor attributes for one of the magic	*/
	/* strings MEM, DSK, etc. This means that the processor must	*/
	/* be given a special pin.					*/  
  attrib_count = RmCountProcessorAttributes(Processor);
  if (attrib_count > 10)
   driver_fatal(device, "telmat_r.d driver, processor %s has too many attributes\n",
   		Processor->ObjNode.Name);
  RmListProcessorAttributes(Processor, attrib_list);

  for (i = 0; i < attrib_count; i++)
   { test = Is_special_processor(special_processor, attrib_list[i]);
     if (test == SPC_TYPE_USED)		/* 3 disk boards, for example ? */
      { driver_report(device, "telmat_r.d driver, not enough %s processors in this T.NODE",
      		 attrib_list[i]); 
	break;
      }
     elif ( test == SPC_NO_EXIST)	/* user attribute of some sort */
      continue;
     else 
      { driver->Aux1 = test;		/* recognised attribute	*/
#if 0		/* If enabled this prevents special processors from being */
		/* used as native processors.				  */
	driver->Flas &= ~DriverFlags_NativePossible;
#endif	
        return;
      }
   }
	/* If I fall through to here then the processor is just a worker. */
  driver->Aux1 = (*sequence_number)++;
}

/**
*** Reset involves four message interactions with the ControlBus Manager.
*** First a request to assert reset, a reply from CbMan, then another
*** request to clear reset, and another reply. This is painfully expensive,
*** and would make a joke out of the parallel bootstrap code in the
*** Network Server. Hence if the request is for a single processor that
*** is already reset then it is treated as a no-op. The Network Server
*** ensures that if a bootstrap fails then the processor is marked as
*** no longer reset.
**/
static void	driver_Reset(NetworkDCB *device, DriverRequest *request)
{ int			i;
  word			retries;
  word			size;
  Stream		*stream = (Stream *) device->ControlStream;
  TK_CB_control_slave_list_COM		*command;	/* message for CBMan */
  TK_CB_control_slave_list_REPLY 	CB_reply;	/* message from CBMan */

  if (request->NumberProcessors eq 1)
   { int state = RmGetProcessorState(request->Processor[0]);
     if (state & RmS_Reset)
      { request->FnRc = Err_Null; return; }
   }
   
  size = sizeof(TK_CB_control_slave_list_COM) +
         ((request->NumberProcessors -1) * sizeof(int));
  command = Malloc(size);
  if (command eq Null(TK_CB_control_slave_list_COM))
   { request->FnRc = EC_Error + SS_NetServ + EG_NoMemory + EO_Message;
     return;
   }  

#if SOLEIL_DBG
	IOdebug("NBR Processor %d",request->NumberProcessors);
#endif
  command->Command	= TK_CB_control_slave_list;
  command->Data		= 0;
  for (i = 0; i < request->NumberProcessors; i++)
   command->PIN[i]	= ExtractPin(device, request->Processor[i]);

  for (retries = 0; retries < 2; retries++)
   { CB_reply.Status	= TK_illegal_arg;
     command->Opcode	= CBunadRESETset;
     request->FnRc	= PutServer(stream, (BYTE *) command, size, FC_Private, TIMEOUT);
     if (request->FnRc < Err_Null)
      { ReOpen(stream); continue; }	/* retry... */
     request->FnRc	= GetServer(stream, (BYTE *) &CB_reply, &i, TIMEOUT);
     if (request->FnRc < Err_Null)
      { ReOpen(stream); continue; }
     if (CB_reply.Status ne 1) break;	/* error from CBbus manager: */
     
     command->Opcode	= CBunadRESETclear;
     request->FnRc	= PutServer(stream, (BYTE *) command, size, FC_Private, TIMEOUT);
     if (request->FnRc < Err_Null)
      { ReOpen(stream); continue; }	/* retry... */
     request->FnRc	= GetServer(stream, (BYTE *) &CB_reply, &i, TIMEOUT);
     if (request->FnRc < Err_Null)
      { ReOpen(stream); continue; }
     if (CB_reply.Status eq 1) break;
   }

  if (request->FnRc ne Err_Null)
   driver_report(device, "telmat_r.d driver, failed to communicate with /CbMan, fault %x",
   			request->FnRc);       
  elif ( CB_reply.Status ne 1 ) 
   driver_report(device, "telmat_r.d driver, CbMan error %d ",CB_reply.Status);
  else
   for (i = 0; i < request->NumberProcessors; i++)
    { int state = RmGetProcessorState(request->Processor[i]);
      state |= RmS_Reset;
      state &= ~(RmS_Running);
      RmSetProcessorState(request->Processor[i], state);
      unless(device->Silent)
       driver_report(device, "telmat_r.d driver, successfully reset processor /%s (Pin %d)",
         	RmGetProcessorId(request->Processor[i]), command->PIN[i]);
    }
  Free(command);
}

/**
*** Analyse is almost a duplicate of reset, except that it always has
*** to do some work. The special case of a single processor is not
*** permitted.
BLV Question, does asserting the control bus analyse do a proper analyse,
BLV or does it merely fiddle with the analyse pin. If the latter then it
BLV is also necessary to manipulate the reset pin.
**/
static void	driver_Analyse(NetworkDCB *device, DriverRequest *request) 
{ int			i;
  word			retries;
  word			size;
  Stream		*stream = (Stream *) device->ControlStream;
  TK_CB_control_slave_list_COM		*command;	/* message for CBMan */
  TK_CB_control_slave_list_REPLY 	CB_reply;	/* message from CBMan */

  size = sizeof(TK_CB_control_slave_list_COM) +
         (request->NumberProcessors * sizeof(int));
  command = Malloc(size);
  if (command eq Null(TK_CB_control_slave_list_COM))
   { request->FnRc = EC_Error + SS_NetServ + EG_NoMemory + EO_Message;
     return;
   }  

#if SOLEIL_DBG
	IOdebug("NBR Processor %d",request->NumberProcessors);
#endif
  command->Command	= TK_CB_control_slave_list;
  command->Data		= 0;
  for (i = 0; i < request->NumberProcessors; i++)
   command->PIN[i]	= ExtractPin(device, request->Processor[i]);

  for (retries = 0; retries < 2; retries++)
   { CB_reply.Status	= TK_illegal_arg;
     command->Opcode	= CBunadANALset;
     request->FnRc	= PutServer(stream, (BYTE *) command, size, FC_Private, TIMEOUT);
     if (request->FnRc < Err_Null) continue;	/* retry... */
     request->FnRc	= GetServer(stream, (BYTE *) &CB_reply, &i, TIMEOUT);
     if (request->FnRc < Err_Null) continue;
     if (CB_reply.Status ne 1) break;	/* error from CBbus manager: */
     
     command->Opcode	= CBunadANALclear;
     request->FnRc	= PutServer(stream, (BYTE *) command, size, FC_Private, TIMEOUT);
     if (request->FnRc < Err_Null) continue;	/* retry... */
     request->FnRc	= GetServer(stream, (BYTE *) &CB_reply, &i, TIMEOUT);
     if (request->FnRc < Err_Null) continue;
     if (CB_reply.Status eq 1) break;
   }

  if (request->FnRc ne Err_Null)
   driver_report(device, "telmat_r.d driver, failed to communicate with /CbMan, fault %x",
   			request->FnRc);       
  elif ( CB_reply.Status ne 1 ) 
   driver_report(device, "telmat_r.d driver, CbMan error %d ",CB_reply.Status);
  else
   for (i = 0; i < request->NumberProcessors; i++)
    { int state = RmGetProcessorState(request->Processor[i]);
      state |= RmS_Reset;
      state &= ~(RmS_Running);
      RmSetProcessorState(request->Processor[i], state);
      unless(device->Silent)
       driver_report(device, "telmat_r.d driver, successfully analysed processor /%s (%d)",
         	RmGetProcessorId(request->Processor[i]), command->PIN[i]);
    }
  Free(command);
}


/**
*** The telmat driver always has full reset over all the processors, so this
*** routine should never be called.
**/
static void	driver_TestReset(NetworkDCB *device, DriverRequest *request)
{ driver_fatal(device, "telmat_r.d, driver TestReset routine called illegally");
  request = request;
}

/**
*** The telmat driver does not set the special bootstrap flag, so this routine
*** should never get called.
**/
static void	driver_Boot(NetworkDCB *device, DriverRequest *request)
{ driver_fatal(device, "telmat_r.d, driver bootstrap routine called illegally");
  request = request;
}

/**
*** The telmat driver sets the full reset flag so this routine should never
*** get called.
**/
static void driver_ConditionalReset(NetworkDCB *device, DriverRequest *request)
{ driver_fatal(device,
               "telmat_r.d, driver conditional reset routine called illegally");
  request = request;
}


