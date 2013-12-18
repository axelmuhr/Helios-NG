/*------------------------------------------------------------------------
--									--
--			H E L I O S					--
--			-----------					--
--									--
--		Copyright (C) 1990, Perihelion Software Ltd.		--
--			All Rights Reserved.				--
--									--
--	rte_c.c								--
--		Configuration driver for the Meiko computing surface.	--
--	This driver accesses a device /NetworkController inside the	--
--	Meiko I/O Server to perform all the operations.			--
--									--
--	Author : BLV, 15.8.90						--
--	Based on code by JMP						--
--									--
------------------------------------------------------------------------*/

static char *rcsid = "$Header: /users/bart/hsrc/network/meiko/TRAN/RCS/rte_c.c,v 1.2 1992/01/14 14:27:02 bart Exp $";

#define VersionNumber "1.00"
/**
*** Version numbers :
***	1.00, experimental first version
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

/**
*** The NetworkDCB structure contains some spare words to hold information
*** required permanently by the device driver, i.e. global variables.
*** The slots are allocated here.
**/
#define		ControlStream	Spare[0]
#define		Mutex		Spare[1] /* pointer to semaphore */

#ifdef HELIOS1_2
typedef struct DriverConnection {
	RmProcessor	Source;
	int		SourceLink;
	RmProcessor	Dest;
	int		DestLink;
} DriverConnection;

typedef struct DriverConfRequest {
	int			FnRc;
	int			NumberConnections;
	bool			Exact;
	bool			Preserve;
	DriverConnection	*Connections;
} DriverConfRequest;

extern int	RmGetProcessorPrivate(RmProcessor);
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
static void	driver_MakeLinks(	NetworkDCB *, DriverConfRequest *);
static void	driver_TestLinks(	NetworkDCB *, DriverConfRequest *);
static void	driver_ObtainProcessors(NetworkDCB *, DriverRequest *);
static void	driver_MakeInitialLinks(NetworkDCB *, DriverRequest *);
static void	driver_FreeProcessors(	NetworkDCB *, DriverRequest *);
static int	make_connection(NetworkDCB *device, RmProcessor, int,
			RmProcessor, int);

DCB	*DevOpen(Device *dev, NetworkDCB *network_dcb)
{
  network_dcb->DCB.Device	= dev;
  network_dcb->DCB.Operate	= &DeviceOperate;
  network_dcb->DCB.Close	= &DeviceClose;
  network_dcb->Mutex		= (int) New(Semaphore);
  InitSemaphore((Semaphore *) network_dcb->Mutex, 1);
  return(&(network_dcb->DCB));  
}

static void DeviceOperate(DCB *device, DriverRequest *request)
{ NetworkDCB	*network_dcb = (NetworkDCB *) device;

  Wait((Semaphore *) network_dcb->Mutex);
  switch(request->FnRc)
   {
   	case	ND_Initialise	: 
   		driver_Initialise(network_dcb, request); break;
	case	ND_MakeLinks	:
		driver_MakeLinks(network_dcb, (DriverConfRequest *) request); break;
	case	ND_TestLinks	:
		driver_TestLinks(network_dcb, (DriverConfRequest *) request); break;
	case	ND_ObtainProcessors :
		driver_ObtainProcessors(network_dcb, request); break;
	case	ND_MakeInitialLinks :
		driver_MakeInitialLinks(network_dcb, request); break;
	case	ND_FreeProcessors :
		driver_FreeProcessors(network_dcb, request); break;
	default	:
		request->FnRc = EC_Error + SS_NetServ + EG_WrongFn;
   }
  Signal((Semaphore *) network_dcb->Mutex);
}

static word DeviceClose(DCB *device)
{ NetworkDCB	*network_dcb = (NetworkDCB *) device;
  Stream	*controller	= (Stream *) network_dcb->ControlStream;
  if (controller ne Null(Stream))
   { Close(controller);
     network_dcb->ControlStream = NULL;
   }
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
   Signal ((Semaphore*)&device->Mutex);
   (*(Functions->fatal))(format, arg1, arg2, arg3, arg4, arg5);
}

/**-----------------------------------------------------------------------------
*** The driver_initialise() routine. It is not necessary to assign id numbers
*** or anything like that, since the reset driver will take care of that.
*** Instead the driver simply displays its sign-on message and opens a
*** stream to the /NetworkController device.
**/

static void	driver_Initialise(NetworkDCB *device, DriverRequest *request)
{ RmHardwareFacility	*hardware;
  Stream		*controller;
  Object		*control_obj;
  
  driver_report(device, "rte_c.d driver version %s", VersionNumber);

  hardware = device->HardwareFacility;
  if (strlen(hardware->Option) ne 0)
   driver_report(device, "rte_c.d driver, ignoring option %s",
		hardware->Option);

  control_obj = Locate(Null(Object), "/NetworkController");
  if (control_obj eq Null(Object))
   driver_fatal(device, "rte_c.d driver, failed to locate /NetworkControl");

  controller = Open(control_obj, Null(char), O_ReadOnly);
  if (controller eq Null(Stream))
   driver_fatal(device,
		"rte_c.d driver, failed to open /NetworkController, fault %x",
  		Result2(control_obj));

  device->ControlStream	= (int) controller;
  Close(control_obj);
  request->FnRc	= Err_Null;
}

/**-----------------------------------------------------------------------------
*** The current Meiko configuration driver does not support dynamic
*** link reconfiguration or native networks.
**/
static void	driver_MakeLinks(NetworkDCB *device, DriverConfRequest *request)
{ driver_report(device, "rte_c.d, MakeLinks reconfiguration routine called illegally");
  request->FnRc = EC_Fatal + SS_NetServ + EG_WrongFn + EO_Network;
}

/**
*** Similarly, it is not possible to test for link configurations if no
*** dynamic link reconfiguration is supported.
**/
static void	driver_TestLinks(NetworkDCB *device, DriverConfRequest *request)
{ driver_report(device, "rte_c.d, TestLinks reconfiguration routine called illegally");
  request->FnRc = EC_Fatal + SS_NetServ + EG_WrongFn + EO_Network;
}


/**
*** FreeProcessors(). This is not currently used by the Network Server,
*** although it might become part of the system shutdown procedure at some
*** time in the future. In theory this could interact with the
*** /NetworkController device to release processors via supervisor bus
*** calls.
**/
static void driver_FreeProcessors(NetworkDCB *device, DriverRequest *request)
{ request->FnRc = Err_Null;
  device	= device;
  return;
}

/**
*** In theory driver_ObtainProcessors() could send a request to the
*** /NetworkController device to obtain the required number of processors,
*** instead of forcing the user to pre-allocate the required number with
*** the runhelios command. For now this is not supported.
**/
static void driver_ObtainProcessors(NetworkDCB *device, DriverRequest *request)
{ request->FnRc = Err_Null;
  device = device;
  return;
}

/**-----------------------------------------------------------------------------
*** Making a single connection. This involves sending a request to the
*** /NetworkController device of the I/O Server. A separate request is needed
*** for each connection, given the current code in the Meiko I/O Server,
*** but this could be optimised later. The hardware id's for the
*** processors involved are extracted from the driver entry structures,
*** which were filled in by the reset driver.
**/
#define	NC_Connect	0x2030

static int make_connection(NetworkDCB *device, RmProcessor source,
	int source_link, RmProcessor dest, int dest_link)
{ MCB			mcb;
  Port			reply = NewPort();
  WORD			vec[4];
  int			retries;
  Stream		*controller = (Stream *) device->ControlStream;
  ProcessorEntry	*proc_entry;
  int			rc;

  proc_entry	= (ProcessorEntry *) RmGetProcessorPrivate(source);
  vec[0]	= proc_entry->DriverEntry[0].Aux1;
  vec[1]	= source_link;
  proc_entry	= (ProcessorEntry *) RmGetProcessorPrivate(dest);
  vec[2]	= proc_entry->DriverEntry[0].Aux1;
  vec[3]	= dest_link;

  for (retries = 0; retries < 2; retries++)
   { mcb.Data			= (BYTE *) vec;
     mcb.Control		= Null(WORD);
     mcb.Timeout		= 10 * OneSec;
     mcb.MsgHdr.DataSize	= 4 * sizeof(WORD);
     mcb.MsgHdr.ContSize	= 0;
     mcb.MsgHdr.Flags		= MsgHdr_Flags_preserve;
     mcb.MsgHdr.Dest		= controller->Server;
     mcb.MsgHdr.Reply		= reply;
     mcb.MsgHdr.FnRc		= FC_Private + NC_Connect;

     if ((rc = PutMsg(&mcb)) < Err_Null)
      { ReOpen(controller); continue; }

     mcb.MsgHdr.Dest		= reply;
     mcb.Timeout		= 10 * OneSec;
     rc				= GetMsg(&mcb);
     if (rc >= Err_Null) break;

     if ((rc & SS_Mask) eq SS_IOProc) break;
     ReOpen(controller);
   }

  FreePort(reply);
  return((rc < Err_Null) ? rc : Err_Null);
}

/**-----------------------------------------------------------------------------
*** Making the initial links. This involves sending a request to the
*** /NetworkController device for every link, except the one to the
*** I/O Processor. Care has to be taken not to make every link twice.
**/

static int  MakeInitialLinks_aux1(RmProcessor Processsor, ...);

static void driver_MakeInitialLinks(NetworkDCB *device, DriverRequest *request)
{
  request->FnRc = RmSearchNetwork(device->Net, &MakeInitialLinks_aux1, device);
}

static int MakeInitialLinks_aux1(RmProcessor Processor, ...)
{ int		number_links, i;
  RmProcessor	neighbour;
  int		destlink;
  int		my_uid, its_uid;
  int		rc;
  va_list	args;
  NetworkDCB	*device;

  va_start(args, Processor);
  device = va_arg(args, NetworkDCB *);
  va_end(args);

  if (RmIsNetwork(Processor))
   return(RmSearchNetwork((RmNetwork) Processor, &MakeInitialLinks_aux1, device));

#ifdef HELIOS1_2
  if (RmGetProcessorPurpose(Processor) eq RmP_IO)
#else
  if ((RmGetProcessorPurpose(Processor) & RmP_Mask) eq RmP_IO)
#endif
   return(0);

  number_links = RmCountLinks(Processor);
  for (i = 0; i < number_links; i++)
   { neighbour = RmFollowLink(Processor, i, &destlink);
     if ((neighbour eq RmM_NoProcessor) || (neighbour eq RmM_ExternalProcessor))
      continue;
#ifdef HELIOS1_2
     if (RmGetProcessorPurpose(neighbour) eq RmP_IO)
#else
     if ((RmGetProcessorPurpose(neighbour) & RmP_Mask) eq RmP_IO)   
#endif
      continue;

     my_uid	= RmGetProcessorUid(Processor);
     its_uid	= RmGetProcessorUid(neighbour);
     if (its_uid < my_uid) continue;
     if ((its_uid eq my_uid) && (destlink < i)) continue;
    
     if ((rc = make_connection(device, Processor, i, neighbour, destlink))
          ne Err_Null)
      return(rc);
   }
  return(0);
}

