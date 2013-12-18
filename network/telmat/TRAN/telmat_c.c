/*------------------------------------------------------------------------
--									--
--			H E L I O S					--
--			-----------					--
--									--
--		Copyright (C) 1990, Perihelion Software Ltd.		--
--		Copyright (C) 1990, Telmat Informatique			--
--			All Rights Reserved.				--
--									--
--	telmat_c.c							--
--		Configuration driver for the Telmat T.Node. This	--
--	version does not yet support larger machines such as the	--
--	Meganode.							--
--									--
--	Authors : BLV, 15.8.90						--
--		  Caroline Burrer et al, Telmat Informatique		--
--									--
------------------------------------------------------------------------*/

static char *rcsid = "$Header: /users/bart/hsrc/network/telmat/TRAN/RCS/telmat_c.c,v 1.2 1992/01/14 14:26:27 bart Exp $";

#define	VersionNumber	"1.02"
/**
*** Version numbers :
***	1.00, developed by Bart Veer, Caroline Burrer, Philippe Moliere,
***	      for Helios 1.2
***     1.01, Bart Veer, enhancements to support native networks
***	1.02, fixed problems with reconfiguring links involving the root
***	      processor
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
				/* and SwMan ( switch manager ) */
#include "TRAN/tkswdefs.h"	/* for SwMan ( switch manager ) */
#include "TRAN/special.h"	/* for SwMan ( switch manager ) */

#define RootPin		80000

#define SOLEIL_PMO FALSE

extern int configure ( int *tab, int *tab1, int *tab2, int *err, char *tab3);

#define TIMEOUT		( 5 * OneSec ) 	/* timeout for messages */

#ifdef Malloc
#undef Malloc
#endif

/**
*** The NetworkDCB structure contains twenty spare words to hold information
*** required permanently by the device driver, i.e. global variables.
*** The slots are allocated here.
**/
#define		ControlStream	Spare[0]
#define		Mutex		Spare[1] /* pointer to semaphore */
#define		Configure	Spare[2] /* configuration possible ? */
#define		C004_Table	Spare[3]
#define		ItemTable	Spare[4]
#define		Pin_Table	Spare[5]
#define		Machine		Spare[6] /* hold details of the machine */
					 /* for switching restrictions. */
#define		Silent		Spare[7]

	/* Used for binary search, mapping Pin->Processor */
typedef struct	Pin_Entry {
	int		Pin;
	RmProcessor	Processor;
} Pin_Entry;

typedef	struct	Connection {
	int		Pin;
	int		DestPin;
	int		Link;
	int		DestLink;
} Connection;

static	bool		MakeConnection(NetworkDCB *device, DriverRequest *,
			Connection *);

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

DCB	*DevOpen(Device *dev, NetworkDCB *network_dcb)
{
  network_dcb->DCB.Device	= dev;
  network_dcb->DCB.Operate	= &DeviceOperate;
  network_dcb->DCB.Close	= &DeviceClose;
  network_dcb->Mutex		= (int) New(Semaphore);
  InitSemaphore ((Semaphore*)network_dcb->Mutex,1);
  return(&(network_dcb->DCB));  
}

static void DeviceOperate(DCB *device, DriverRequest *request)
{ NetworkDCB	*network_dcb = (NetworkDCB *) device;

  Wait ((Semaphore*)network_dcb->Mutex);
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
  Signal ((Semaphore*)network_dcb->Mutex);
}

static word DeviceClose(DCB *device)
{ NetworkDCB	*network_dcb = (NetworkDCB *) device;
  Stream	*controller  = (Stream *) network_dcb->ControlStream;
  if (controller ne Null(Stream)) Close(controller);
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

/**
*** Locate the specified server with a timeout  20sec
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
		driver_fatal(device, "telmat_c.d driver, failed to locate %s ",name);
	if (control_obj eq Null(Object))
		driver_report(device, "telmat_c.d driver, failed to locate %s, retry ... ",name);
	else (locate_timedout = 0);
	Delay(OneSec * 2); 
    }
    driver_report(device, "telmat_c.d driver, found %s ",name);

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

  return(Null(DriverEntry));
}

/**
*** 3) and given a processor, work out the pin number.
**/
static int ExtractPin(NetworkDCB *device, RmProcessor Processor)
{ DriverEntry	*driver = ExtractDriver(device, Processor);
  if (driver eq Null(DriverEntry))
   { if (Processor eq device->RootProcessor)
      return(RootPin);
     else
     return(-1);
   }
  else
   return(driver->Aux1);
}

/**
*** 4) given a Pin number, find the processor. This uses a binary search of
***    a table created in the initialisation phase.
**/
static	RmProcessor ExtractProcessor(NetworkDCB *device, int pin)
{ Pin_Entry		*table = (Pin_Entry *) device->Pin_Table;
  RmHardwareFacility	*hardware = device->HardwareFacility;
  int			max = hardware->NumberProcessors;
  int			min = 0;
  int			i = max / 2;

  if (pin eq RootPin)
   return(device->RootProcessor);

  until(table[i].Pin eq pin)
   { if (table[i].Pin < pin)
      { min = i;
        i = (i + max) / 2;
      }
     else
      { max = i;
        i = (i + min) / 2;
      }
   } 
  return(table[i].Processor);
}

/**
*** 5) This routine builds the table of Pin numbers/processor pairs. It assumes
*** that the table has been allocated already. The processors are put into
*** the table in no particular order, and then sorted. Given that the
*** number of processors will never be more than a few hundred, a bubble sort
*** will suffice for now. Also, since most of the processors are workers
*** these will be almost sorted already anyway. To facilitate the sorting
*** special processors with large pins are put at the back of the table.
**/
static void BuildPinTable(NetworkDCB *device)
{ Pin_Entry		*table = (Pin_Entry *) device->Pin_Table;
  RmHardwareFacility	*hardware = device->HardwareFacility;
  RmProcessor		processor;
  int			number_procs = hardware->NumberProcessors;
  int			tail = number_procs - 1;
  int			head = 0;
  int			i;
  int			changes = 1;
  int			pin;
    
  for (i = 0; i < number_procs; i++)
   { processor = hardware->Processors[i];
     pin = ExtractPin(device, processor);
     if (pin >= SPC_PIN_1)
      { table[tail].Pin 	= pin;
        table[tail].Processor	= processor;
        tail--;
      }
     else
      { table[head].Pin		= pin;
        table[head].Processor	= processor;
        head++;
      }
   }
   
  until(changes eq 0)
   for (i = 0, changes = 0; i < (number_procs - 1); i++)
    { if (table[i].Pin <= table[i+1].Pin)
       continue;
      else
       { processor = table[i].Processor;
         pin = table[i].Pin;
         table[i].Processor = table[i + 1].Processor;
         table[i].Pin = table[i + 1].Pin;
         table[i + 1].Processor = processor;
         table[i + 1].Pin	= pin;
         changes++;
       }
    }
}
  
/**
*** 6) Initialising the Switch Manager. This is used to reset all connections
*** in the main switch and the C004 when the device driver initialises itself.
**/
static void Initialise_SwMan ( NetworkDCB *device )
{
  int	retries;
  PARAM	command;
  int 	size;  
  int	status;
  int	reply;

  command.elt[0]		= TK_swm_init;
  command.len = 0 ;

  for ( retries = 0 ; retries < 2 ; retries++ ){    
    reply = PutServer ( (Stream*) device->ControlStream, (BYTE *) &command, 
			sizeof(PARAM), FC_Private + FG_SW_Manager, TIMEOUT );
    if ( reply < Err_Null )
     { ReOpen((Stream *) device->ControlStream); continue; }
    reply = GetServer ( (Stream*) device->ControlStream, (BYTE*) &status,
			&size, TIMEOUT );
    if ( reply < Err_Null ) 
     { ReOpen((Stream *) device->ControlStream); continue; }
    else break;
  }
  if (reply < Err_Null)
    driver_fatal( device,"telmat_c.d driver, Failed to communicate with /SwMan");  
  if (status)
    driver_fatal( device,"telmat_c.d driver, SwMan error %d",status);  
	
}

/**
*** 7) Make the specified connection, which may be internal or
***    external, by invoking the Switch Manager.
**/
static bool MakeConnection(NetworkDCB *device, DriverRequest *request,
		 Connection * conn)
{	PARAM	command;
	int	reply; 
	int 	status = 1;
	int 	size;
	int 	retries;
	
	command.elt[0]		= TK_swm_connect_link;
	if (( conn->Pin >= 80000 ) && (conn->Pin <= 80100 ))
  	  command.elt[0]		= TK_swm_connect_helios;
	command.elt[1]		= conn->Pin; 
	command.elt[2]		= conn->Link; 
	command.elt[3]		= conn->DestPin; 
	command.elt[4]		= conn->DestLink; 
	command.len		= 4;

	for ( retries = 0 ; retries < 2 ; retries++ ){    
	  unless(device->Silent)
           driver_report( device,"telmat_c.d driver, Connection Pin %d Link %d to Pin %d Link %d",conn->Pin, conn->Link, conn->DestPin, conn->DestLink);

	  reply = PutServer ( (Stream*) device->ControlStream, (BYTE *) &command, 
			sizeof(PARAM), FC_Private + FG_SW_Manager, TIMEOUT );
	  if ( reply < Err_Null )
	   { ReOpen((Stream *) device->ControlStream); continue; }
	  reply = GetServer ( (Stream*) device->ControlStream, (BYTE*) &status, 
			&size, TIMEOUT );
	  if ( reply < Err_Null )
	   { ReOpen((Stream *) device->ControlStream); continue; }
	  else break;
	}
	if (status)
	 { driver_report( device,"telmat_c.d driver, SwMan error %d",status);  
	   request->FnRc = EC_Error + SS_NetServ + EG_Broken + EO_Network;
	   return(FALSE);
	 }
	elif (reply < Err_Null)
	 { driver_report( device,"telmat_c.d driver, Failed to communicate with /SwMan");  
           request->FnRc = reply;
	   return(FALSE);
	 }

  return(TRUE);
}

/**
*** 8) Given a configuration request, attempt to determine its feasibility.
***    At present this ignores the EXACT and PRESERVE options for reconfiguring,
***    so unless the requested configuration is available exactly the routine
***    will fail. This should be improved in the future.
BLV Also, it is important to keep track of the external ITEM usage and
BLV the switchability of the C004. At present I do not understand the configurer
BLV well enough to keep track of these things.
BLV
BLV The device driver can be given an option in the resource map specifying
BLV exactly what hardware is in use, to facilitate these tests. For now
BLV a standard T.Node is assumed.
**/
static int TestExternalConnection(NetworkDCB *, Connection *);

static int TestConnections(NetworkDCB *device, DriverConfRequest *request,
				int count, Connection *conns)
{ int	i, rc;

  for (i = 0; i < count; i++)
   {
     if ((conns[i].Pin >= 80000) || (conns[i].DestPin >= 80000))
      { rc = TestExternalConnection(device, &(conns[i]));
        if (rc ne RmE_Success) 
         return(rc);
        else
         continue;
      }

	/* Legal connections are: 0 <-> 3, 1 <-> 2	*/
     if ((conns[i].Link + conns[i].DestLink) ne 3)
      return(RmE_BadLink);
   }
  return(RmE_Success);      
  request = request;	/* BLV Exact and Preserve should be used */
}

static int	TestExternalConnection(NetworkDCB *device, Connection *conn)
{ device = device;
  conn = conn;
  return(RmE_Success);
}

/**
*** 9) translate a configuration request into something understood by
***    the Telmat software.
**/
static int TranslateConfRequest(NetworkDCB *device,
		DriverConfRequest *request, Connection *conns)
{ int		i;
  int		index;

  for (i = 0, index = 0; i < request->NumberConnections; i++)
   { DriverConnection *this = &(request->Connections[i]);
   
  	/* The Telmat hardware cannot electronically isolate links, so	*/
  	/* setting links to not-connected is a no-op.			*/
     if ((this->Source eq RmM_NoProcessor) ||
         (this->Dest eq RmM_NoProcessor))
      continue;
      
     if (this->Source eq RmM_ExternalProcessor)
      { conns[index].Pin  = 80000 + (this->SourceLink / 100);
        conns[index].Link = this->SourceLink & 0x03;
      }
     else
      { conns[index].Pin  = ExtractPin(device, this->Source);
        if (conns[index].Pin eq -1)
         { driver_report(device, "telmat_c.d, received invalid configuration request");
           return(-1);
         }
        conns[index].Link = this->SourceLink;
      }
      
     if (this->Dest eq RmM_ExternalProcessor)
      { 
        conns[index].DestPin  = conns[index].Pin;
        conns[index].DestLink = conns[index].Link;
        conns[index].Pin      = 80000 + (this->DestLink / 100);
        conns[index].Link     = this->DestLink & 0X03;
      }
     else
      { conns[index].DestPin = ExtractPin(device, this->Dest);
        if (conns[index].DestPin eq -1)
         { driver_report(device, "telmat_c.d, received invalid configuration request");
           return(-1);
         }
        conns[index].DestLink = this->DestLink;
      }
      
     index++;
   }
  return(index);
}

/**-----------------------------------------------------------------------------
*** driver_Initialise(). This is a copy of the telmat_ra.d initialisation
*** code.
**/
static	void	initialise_aux1(RmProcessor, int *, NetworkDCB *, SPC_Processor * );

static void	driver_Initialise(NetworkDCB *device, DriverRequest *request)
{ int			sequence_number = 0;
  RmHardwareFacility	*hardware;  
  int			i;
  SPC_Processor		*special_processor;
      
  driver_report(device, "telmat_c.d driver version %s", VersionNumber);
  LocateServer ( device, "/SwMan" );

  hardware		= device->HardwareFacility;
  device->Silent	= FALSE;
  if (strlen(hardware->Option) ne 0)
   { if (!strcmp(hardware->Option, "silent"))
      device->Silent = TRUE;
     else
      driver_report(device, "telmat_c.d driver, ignoring option %s", 
   		hardware->Option);
   } 
 
  if ( !(strcmp(hardware->Name,"telmatnc.d")))
   device->Configure = FALSE;
  else
   device->Configure = TRUE;

	/* This builds a table of the special processor names and pins. */
  special_processor = (SPC_Processor*) Malloc(SPC_MAX_PROCESSOR * sizeof(SPC_Processor));
  if (special_processor eq Null(SPC_Processor))
   { request->FnRc = EC_Error + SS_NetServ + EG_NoMemory + EO_Link; return; }
  Initialise_all_special ( special_processor );

  for (i = 0; i < hardware->NumberProcessors; i++)
   { RmProcessor Processor = hardware->Processors[i];
     initialise_aux1(Processor, &sequence_number, device,special_processor); 
   }   
  Free(special_processor);

  device->Pin_Table = (int) Malloc(hardware->NumberProcessors * sizeof(Pin_Entry));
  if (device->Pin_Table eq (int) NULL)
   { request->FnRc = EC_Error + SS_NetServ + EG_NoMemory + EO_Link; return; }
  BuildPinTable(device);

  request->FnRc = Err_Null;
  Initialise_SwMan ( device );
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
  if (Processor eq device->RootProcessor)
   { /* The root processor is never really controlled by this driver,	*/
     /* it is not connected to the main switch. Hence it should be	*/
     /* treated in the same way as an external link.			*/
     /* To keep things simple, I set its PIN to invalid.		*/
     driver = ExtractDriver(device, Processor);
     driver->Aux1 = -1;
     return;
   }
  
 	/* find the correct driver */
  driver = ExtractDriver(device, Processor);
#if 0
	/* this is not yet true, Pins are permanently associated with names */  
  driver->Flags |= DriverFlags_MappingFlexible;
#endif
  for (i = 0; i < 4; i++)	/* T.Node transputers always have four links */
   { RmLink *link = RmFindLink(Processor, i);
     link->Flags |= RmF_Configurable;
   }    
	/* Check all the processor attributes for one of the magic	*/
	/* strings MEM, DSK, etc. This means that the processor must	*/
	/* be given a special pin.					*/  
  attrib_count = RmCountProcessorAttributes(Processor);
  if (attrib_count > 10)
   driver_fatal(device, "telmat_c.d driver, processor %s has too many attributes\n",
   		Processor->ObjNode.Name);
  RmListProcessorAttributes(Processor, attrib_list);

  for (i = 0; i < attrib_count; i++)
   { test = Is_special_processor(special_processor, attrib_list[i]);
     if (test == SPC_TYPE_USED)		/* 3 disk boards, for example ? */
      { driver_report(device, "telmat_c.d driver, not enough %s processors in this T.NODE",
      		 attrib_list[i]); 
	break;
      }
     elif ( test == SPC_NO_EXIST)	/* user attribute of some sort */
      continue;
     else 
      { driver->Aux1 = test;		/* recognised attribute	*/
        return;
      }
   }
	/* If I fall through to here then the processor is just a worker. */
  driver->Aux1 = (*sequence_number)++;
}

/**
**/
static void	driver_MakeLinks(NetworkDCB *device, DriverConfRequest *request)
{ Connection *conns = Malloc(request->NumberConnections * sizeof(Connection));
  int	     count, i;
  int	     rc;

  if (conns eq Null(Connection))
   { request->FnRc = RmE_ServerMemory; return; }

  count = TranslateConfRequest(device, request, conns);
  if (count < 0)
   { request->FnRc = RmE_NotPossible; Free(conns); return; }

  rc = TestConnections(device, request, count, conns);
  if (rc ne RmE_Success)
   { request->FnRc = rc; Free(conns); return; }

/**
*** Pray that the testing has sufficed and that all the connections can be
*** made. This code breaks the links in the Network Server's copy of
*** the network.
**/
  for (i = 0; i < request->NumberConnections; i++)
   { DriverConnection *this = &(request->Connections[i]);
     if ((this->Source ne RmM_NoProcessor) &&
         (this->Source ne RmM_ExternalProcessor))
      RmBreakLink(this->Source, this->SourceLink);

     if ((this->Dest ne RmM_NoProcessor) &&
         (this->Dest ne RmM_ExternalProcessor))
      RmBreakLink(this->Dest, this->DestLink);
   }

	/* Now try to make all the connections... */
	/* There is little that can be done for now about errors.	*/
	/* Errors should not happen at this point because the requested */
	/* connections have been verified....				*/
  for (i = 0; i < count; i++)
   (void) MakeConnection(device, (DriverRequest *) request, &(conns[i]));
   
	/* Now update the connections in the resource map, by using	*/
	/* the table of connections actually made.			*/
  for (i = 0; i < count; i++)
   { RmProcessor source, dest;
     int	 sourcelink, destlink;

     if (conns[i].Pin > 80000)	/* external connection */
      { dest       = RmM_ExternalProcessor;
        destlink   = ((conns[i].Pin - 80000) * 100) + conns[i].Link;
        source	   = ExtractProcessor(device, conns[i].DestPin);
        sourcelink = conns[i].Link;
      }
     else
      { source     = ExtractProcessor(device, conns[i].Pin);
        sourcelink = conns[i].Link;
        if(conns[i].DestPin > 80000)
         { dest     = RmM_ExternalProcessor;
           destlink = ((conns[i].DestPin - 80000) * 100) + conns[i].DestLink;
         }
        else
         { dest     = ExtractProcessor(device, conns[i].DestPin);
           destlink = conns[i].DestLink;
         }
      }

     RmMakeLink(source, sourcelink, dest, destlink);
   }

 Free(conns);
 request->FnRc = RmE_Success;	/* guesswork */
}

/**
*** Testing some links is a copy of the initial part of MakeLinks, but
*** the Network Server's representation of the network is not affected, nor
*** is the Switch Manager invoked to do any work.
**/
static void	driver_TestLinks(NetworkDCB *device, DriverConfRequest *request)
{ Connection *conns = Malloc(request->NumberConnections * sizeof(Connection));
  int	     count;
  
  if (conns eq Null(Connection))
   { request->FnRc = RmE_ServerMemory; return; }

  count = TranslateConfRequest(device, request, conns);
  if (count < 0)
   { request->FnRc = RmE_NotPossible; Free(conns); return; }
   
  request->FnRc = TestConnections(device, request, count, conns);

  Free(conns);
}


/**
*** No-op for now
**/
static void driver_FreeProcessors(NetworkDCB *device, DriverRequest *request)
{
}

/**
*** No-op for now
**/
static void driver_ObtainProcessors(NetworkDCB *device, DriverRequest *request)
{ request->FnRc = Err_Null; return;
}

/**
*** This routine does the following jobs.
*** 1) Allocate space for two tables. LinkTable is used for holding internal
***    connections not involving the C004. ExtTable is used for holding
***    the C004 connections.
*** 2) for every real processor (the root processor is assumed to be outside
***    the T.Node), check every link. 
***     a) unconnected links can be ignored
***     b) connections to other controlled processors are put into the
***	   LinkTable. This connection is broken to ensure that it is not
***	   duplicated in the table.
***     c) connections to external links or external processors are put into
***	   the ExtTable
*** 3) Some magic takes place before calling the configurer. When I understand
***    the configurer, or alternatively when I have rewritten it, I shall
***    tidy up that code.
*** 4) Using the results of the configuration phase, first the external
***    connections are sorted out. If these have changed then the binary
***    form of the resource map has to be tidied up. All connections are
***    made by invoking the Switch Manager.
BLV I would like to keep track of the current usage of the C004 connections
BLV and the items used, using slots in the device Spare table, so that I
BLV can perform more validation in the calls to TestLinks and MakeLinks. 
**/

#define EXT	10	/* offset for external processor in EXT table */

static void driver_MakeInitialLinks(NetworkDCB *device, DriverRequest *request)
{ Connection	*LinkTable;	/* internal connections */
  Connection	*ExtTable;	/* external connections */
  int			i;
  RmHardwareFacility	*hardware = device->HardwareFacility;
  int			number_procs = hardware->NumberProcessors;
  RmProcessor		current;
  RmProcessor		neighbour;
  int			NbConn 		= 0;
  int			NbConnExt	= EXT;	/* connection externes */
  int			NbConnExt1;	/* externes a configurer */
  int			NbConnExt2;	/* externes a rajouter */
  int			NbConnRoot	= 0;	/* connections root */
  int			link;
  int			destlink;
  int			externes[5][2];	/* for configurer */

  driver_report(device, "telmat_c.d, sorting out link configuration");
  
  request->FnRc = EC_Error + SS_NetServ + EG_Broken + EO_Network;
  
  LinkTable = (Connection *) Malloc(number_procs * 4 * sizeof(Connection));
  if (LinkTable eq Null(Connection))
   { request->FnRc = EC_Error + SS_NetServ + EG_NoMemory + EO_Link; return; }

  ExtTable = (Connection *) Malloc(number_procs * 4 * sizeof(Connection));
  if (ExtTable eq Null(Connection))
   { request->FnRc = EC_Error + SS_NetServ + EG_NoMemory + EO_Link;
     Free(LinkTable);
     return;
   }

  for (i = 0; i < number_procs; i++)
   { int		number_links = RmCountLinks(current);
     int		sourcepin;
     int		destpin;
     current = hardware->Processors[i];
     
	/* For now I assume that the root processor is outside the T.Node */
	/* However, with the old-style resource maps the root processor   */
	/* is considered to be under the control of this driver, so I	  */
	/* need a test to avoid making silly links such as the link	  */
	/* between the root processor and the I/O processor.		  */
     if (current eq device->RootProcessor)  continue;

     sourcepin = ExtractPin(device, current);

     for (link = 0; link < number_links; link++)
      { neighbour = RmFollowLink(current, link, &destlink);

		/* unconnected links must be ignored */
	if (neighbour eq RmM_NoProcessor)
	 continue;
	
		/* Connected links may go to another controlled processor */
		/* or to an uncontrolled processor.			  */
        if (neighbour ne RmM_ExternalProcessor)
         {  destpin = ExtractPin(device, neighbour);
            if ((destpin ne -1) && (destpin ne RootPin))
             {            
		  LinkTable[NbConn].Pin			= sourcepin ;
		  LinkTable[NbConn].DestPin		= destpin ;
		  LinkTable[NbConn].Link		= link;
		  LinkTable[NbConn].DestLink		= destlink;
		  RmBreakLink(current, link);
		  NbConn++;
		  continue;
	     }
	 }

	/* If I get here, then this link is either external or it goes	*/
	/* to a connection outside the T.Node/Tandem Node/Meganode.	*/
	/* The behaviour is much the same				*/
        if (neighbour eq device->RootProcessor)
	 { ExtTable[NbConnRoot].Pin		= RootPin ;
	   ExtTable[NbConnRoot].DestPin		= sourcepin ;
	   ExtTable[NbConnRoot].Link		= destlink;
	   ExtTable[NbConnRoot].DestLink	= link;
	   NbConnRoot++;
	 }
	elif (neighbour eq RmM_ExternalProcessor)
		/* This means ext[x] in the resource map, where x is the */
		/* external number and the link number.			 */
		/* Ext = ( x / 100  ) + 80000			 	 */
		/* Link = ( x & 0x03 ) 					 */
		/* For the configurer, it is possible to have 4 external */
		/* Link , so we add some external	 		 */
		/* links to external entry for the configurer.		 */
		/* Other external entry are added as a new virtual 	 */
		/* processor for use the configuration.			 */
	 { ExtTable[NbConnExt].Pin		= 80000 + ( destlink / 100 ) ;
	   ExtTable[NbConnExt].DestPin		= sourcepin ;
	   ExtTable[NbConnExt].Link		= destlink & 0x03;
	   ExtTable[NbConnExt].DestLink		= link;
	   RmBreakLink(current, link);
	   NbConnExt++;
         }
	else
		/* this can be ignored for now, links like this are external */
		/* connections, i.e. they go through an item to an outside   */
		/* processor, but I do not know which item. Possibly it	     */
		/* should assign any free item, starting at 0, or something  */
		/* similar. Possibly the resource map may have to be extended */
		/* to specify the item.					      */
	 { driver_report(device, "telmat_c.d, cannot cope with links to outside processors yet");
	 }	 
      }		/* for every link on the current processor		*/
   }		/* for every processor controlled in this network	*/



	/* When I get here ExtTable should have been filled in with	*/
	/* suitable PIN's and link numbers for all connections to	*/
	/* outside world. It is now necessary to call the configurer	*/
	/* with some magic incantations.				*/

{
	int 	news[4];	/* direction des connections externes */
	int 	err;		/* max 4 */
	char 	msg[100]; 
	int 	test;
	int	extern_processor = 99000;	/* numero de processor virtuel */

		/* nombre de connections externes a rajouter dans les externes */
	NbConnExt1 = 4 - NbConnRoot;		
	if ( NbConnExt1 > ( NbConnExt - EXT ) ) NbConnExt1 = NbConnExt - EXT;
		/* nbre de connections externes a connecter sur transputer virtuel */
	NbConnExt2 = NbConnExt - EXT - NbConnExt1;  
	if (NbConnExt2 < 0 ) NbConnExt2 = 0;
	
				/* chargement des externes pour configurer */
				/* voir doc configurer */
	for ( i = 0 ; i < 4 ; news[i++] = 0); /* initialise la table */
	for ( i = 0 ; i < NbConnRoot ; i++ ){
		news[i] 	= 1;	
		externes[i][0] 	= ExtTable[i].DestPin;
		externes[i][1] 	= ExtTable[i].DestLink;}
	for ( i = 0 ; i < NbConnExt1 ; i++ ){
		news	[NbConnRoot+i] 	= 1;
		externes[NbConnRoot+i][0] 	= ExtTable[EXT+i].DestPin;
		externes[NbConnRoot+i][1] 	= ExtTable[EXT+i].DestLink;}
	externes[NbConnRoot+NbConnExt1][0] = -1;

	/* si 2 externes, le configurer doit avoir 2 entrees nord-sud ou 
	est-west ( cas specifique ) */
	if ( (NbConnRoot + NbConnExt1) == 2 ) {
		news[0] = 0; news[2] = 1;
	}
				/* processor virtuel */	
	for (i = 0; i < NbConnExt2; i++){
		LinkTable[NbConn+i].Pin	= extern_processor;
		LinkTable[NbConn+i].DestPin	= ExtTable[EXT+NbConnExt1+i].DestPin;
		LinkTable[NbConn+i].Link	= i % 4;
		LinkTable[NbConn+i].DestLink	= ExtTable[EXT+NbConnExt1+i].DestLink;    
		if ( ((i+1)  % 4) == 0  ) {
			extern_processor++; 
		}
	}

	LinkTable[NbConn+NbConnExt2].Pin 	= -1;
/* for ( i = 0 ; i < NbConn+NbConnExt2 ; i ++ ) {
IOdebug("---> Connection  = P%d L%d to P%d L%d ",
	LinkTable[i].Pin,	LinkTable[i].Link,
	LinkTable[i].DestPin,	LinkTable[i].DestLink);
} */
	if ( device->Configure ) {	
		test = configure ( (int*) &LinkTable[0], &externes[0][0], &news[0], &err, &msg[0]);
		if ( test != 1 ){
		  driver_report( device,"telmat_c.d driver, Failed to configure network ");  
		  driver_fatal( device,"telmat_c.d driver, errno = %d %s", err, msg  );  
		}

	/* for ( i = 0 ; i < NbConn+NbConnExt2 ; i ++ ) {
		IOdebug("---> Connection  = P%d L%d to P%d L%d ",
		LinkTable[i].Pin,	LinkTable[i].Link,
		LinkTable[i].DestPin,	LinkTable[i].DestLink); } */

	}
}

	/* First, update the external connections by comparing every	*/
	/* entry in the initial ExtTable with the entry in the final	*/
	/* ExtTable. If these are different, figure out the T.Node	*/
        /* worker that is affected, follow the current link to get the  */
 	/* required information, break the current link, and build a	*/
	/* new link.							*/
	/* For example: root processor -> link 2 of worker 0 instead of	*/
	/* link 1. current becomes the processor worker 0, neighbour	*/
	/* becomes the root processor, link 1 is broken, and link 2 is	*/
	/* made.							*/
  for (i = 0; i < NbConnRoot; i++)
   { 
      { current 	= ExtractProcessor(device, ExtTable[i].DestPin);
        neighbour 	= RmFollowLink(current,ExtTable[i].DestLink,&destlink);
        RmBreakLink(current,ExtTable[i].DestLink);
/* IOdebug("neighbour %x",neighbour);
IOdebug("BREAK %d",ExtTable[i].DestLink); */
	RmMakeLink(current, externes[i][1], neighbour, destlink) ;
/* IOdebug("MAKE %d %d to %d %d",ExtTable[i].DestPin,destlink,
	ExtTable[i].Pin,externes[i][1]); */
	ExtTable[i].DestLink 	= externes[i][1];
      }
     unless(MakeConnection(device, request, &(ExtTable[i]))) goto error;
   }
  for (i = 0; i < NbConnExt1; i++)
   { 
      { current 	= ExtractProcessor(device, ExtTable[EXT+i].DestPin);
	RmMakeLink(current, externes[NbConnRoot+i][1], RmM_ExternalProcessor,
		 ((ExtTable[EXT+i].Pin -  80000) * 100) | ExtTable[EXT+i].Link );
	ExtTable[EXT+i].DestLink = externes[NbConnRoot+i][1];
      }
     unless(MakeConnection(device, request, &(ExtTable[EXT+i]))) goto error;
   }
  for (i = 0; i < NbConnExt2; i++)
   { 
      { current = ExtractProcessor(device, ExtTable[EXT+NbConnExt1+i].DestPin);
	RmMakeLink(current, LinkTable[NbConn+i].DestLink, RmM_ExternalProcessor,
		 ((ExtTable[EXT+NbConnExt1+i].Pin -  80000) * 100) | ExtTable[EXT+NbConnExt1+i].Link );
	ExtTable[EXT+NbConnExt1+i].DestLink = LinkTable[NbConn+i].DestLink;
      }
     unless(MakeConnection(device, request, &(ExtTable[EXT+NbConnExt1+i]))) goto error;
   }

	/* For all the links in the Linktable, find the two affected	*/
	/* processors and make the connection. Note that the link was	*/
	/* broken at the time that LinkTable[] was built up.		*/
  for (i = 0; i < NbConn ; i++)
   {  	current = ExtractProcessor(device, LinkTable[i].Pin);
	neighbour = ExtractProcessor(device, LinkTable[i].DestPin);
/* IOdebug("MAKE %d %d to %d %d",LinkTable[i].Pin,LinkTable[i].Link,
	LinkTable[i].DestPin,LinkTable[i].DestLink); */
	RmMakeLink(current, LinkTable[i].Link, neighbour, LinkTable[i].DestLink) ;
     unless(MakeConnection(device, request, &(LinkTable[i]))) goto error;
   }

  request->FnRc		= Err_Null;

error:  
  Free(LinkTable);
  Free(ExtTable);      
  
  driver_report(device, "telmat_c.d, links configured");
  return;
}

