/*------------------------------------------------------------------------
--									--
--			H E L I O S					--
--			-----------					--
--									--
--		Copyright (C) 1990, Perihelion Software Ltd.		--
--			All Rights Reserved.				--
--									--
--	pa_c.c								--
--		Configuration driver for Parsytec machines.		--
--									--
--	Author : BLV, 14.3.91						--
--									--
------------------------------------------------------------------------*/

static char *rcsid = "$Header: /users/bart/hsrc/network/parsytec/TRAN/RCS/pa_c.c,v 1.6 1992/01/15 17:09:51 bart Exp $";

#define	VersionNumber	"1.01"
/**
*** Version numbers :
***	1.00, developed by Bart Veer, for use with version 3.0 of the Parsytec
***	Network Configuration Manager.
***     1.01, updated to use post 1.2.x RmLib
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
#include <link.h>
#include "../private.h"
#include "../rmlib.h"
#include "../netaux.h"

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
#define RmE_BadLink		0x0d6
#define RmE_ServerMemory	0x0dA
#define RmE_NotPossible		0x0dB

#define RmGetProcessorAttribute(a, b) RmGetObjectAttribute((RmObject) a, b, FALSE)

#endif

#define TIMEOUT		( 5 * OneSec ) 	/* timeout for messages */

#ifdef Malloc
#undef Malloc
#endif

/**
*** Processor identification. As far as the configuration driver is
*** concerned every processor has no less than three identifiers. 
*** 1) a Helios name, e.g. /00
*** 2) a logical integer id, obtained in various nasty ways. Every user
***    starts up with logical id 1 onwards, and special processors are given
***    id's 1100, 1200, etc. These id's are referred to as Pin's, as per
***    the Telmat technology.
*** 3) a hardware id. This can be obtained only by a separate call, and is
***    not currently used.
***
*** The NetworkDCB structure contains twenty spare words to hold information
*** required permanently by the device driver, i.e. global variables.
*** The slots are allocated here.
**/
#define		Mutex		Spare[0] /* pointer to semaphore	   */
#define		ConfLink	Spare[1] /* link to configuration driver   */
#define		Pin_Table	Spare[2] /* mapping of Pin's to processors */
#define		LinkAlloc	Spare[3] /* has link been allocated ?	   */
#define		ParsytecRoot	Spare[4] /* is root processor based on	   */
					 /* Parsytec or Inmos hardware ?   */
#define		SensibleNetwork Spare[5] /* Is there a separate link to	   */
					 /* the NCM software ?		   */
#define		Limited		Spare[6] /* limited reconfiguration only ? */
#define		Debug		Spare[7] /* debugging enabled		   */

	/* Used for binary search, mapping Pin->Processor */
typedef struct	Pin_Entry {
	int		Pin;
	RmProcessor	Processor;
} Pin_Entry;

/**
*** Machine independent routines.
**/
static void	DeviceOperate(DCB *, DriverRequest *);
static word	DeviceClose(DCB *);
static void	driver_fatal(NetworkDCB *, char *, ...);

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
  if (network_dcb->Mutex eq 0)
   driver_fatal(network_dcb, "pa_c.d, out of memory during device open");
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
  if (network_dcb->LinkAlloc)
   FreeLink(network_dcb->ConfLink);
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
*** 1) Given a device driver and a specification, extract the correct
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
*** 2) and given a processor, work out the pin number.
**/
static int ExtractPin(NetworkDCB *device, RmProcessor Processor)
{ DriverEntry	*driver = ExtractDriver(device, Processor);
  if (driver eq Null(DriverEntry))
   return(-1);
  else
   return(driver->Aux1);
}

/**
*** 3) given a Pin number, find the processor. This uses a binary search of
***    a table created in the initialisation phase.
**/
static	RmProcessor ExtractProcessor(NetworkDCB *device, int pin)
{ Pin_Entry		*table = (Pin_Entry *) device->Pin_Table;
  RmHardwareFacility	*hardware = device->HardwareFacility;
  int			max = hardware->NumberProcessors;
  int			min = 0;
  int			i = max / 2;

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
*** 4) This routine builds the table of Pin numbers/processor pairs. It assumes
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
     if (pin >= 1100)	/* Put all special processors at the end */
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
*** 5) driver_atoi and itoa routines. The current NCM expects names in places,
***    but returns numbers in other places.
**/
static int driver_atoi(char *str)
{ int	count = 0;

  while ((*str >= '0') && (*str <= '9'))
   count = (10 * count) + *str++ - '0';
  return(count);
}

static char *driver_itoa(char *str, int i)
{ bool	started = FALSE;

  if (i >= 1000)
   { *str++ = (i / 1000) + '0';
     i = i % 1000;
     started = TRUE;
   }
   
  if (i >= 100)
   { *str++ = (i / 100) + '0';
     i = i % 100;
     started = TRUE;
   }
  elif (started)
   *str++ = '0';
   
  if (i >= 10)
   { *str++ = (i / 10) + '0';
     i = i % 10;
   }
  elif (started)
   *str++ = '0';
   
  *str++ = i + '0';
  return(str); 
}

/**
*** 6) RingNcm(), this performs some strange operations to dial up the NCM
***    attached to the specified link. Some of the code is taken from the
***    hconfig software. The routine returns the user's channel number. 
***    The driver may be running on a tram-based system or a Parsytec one,
***    affecting the way the ringing happens. Also, the driver may or may
***    not be running in a sensible network where the link to the NCM does
***    not have to be returned to the system afterwards.
**/
static void Ncm_Write(NetworkDCB *device, int amount, BYTE *buffer);
static void Ncm_Read( NetworkDCB *device, int amount, BYTE *buffer);

static int RingNcm(NetworkDCB *device)
{ int result	= -1;
  int link	= device->ConfLink;

  if (device->Debug) driver_report(device, "pa_c.d, ringing the NCM");
     
	/* Allocate the link to the NCM */
  if (!device->LinkAlloc)
   { LinkInfo	info;
     LinkConf	conf;
    
     if (LinkData(link, &info) < Err_Null)
      driver_fatal(device, "pa_c.d, failed to examine link %d to the NCM", link);
     conf.Flags	= info.Flags;
     conf.Id	= info.Id;
     conf.Mode	= Link_Mode_Dumb;
     conf.State	= Link_State_Dumb;
    
     if (Configure(conf) < Err_Null)
      driver_fatal(device, "pa_c.d, failed to set link %d to the NCM into dumb mode", link);
    
     if (AllocLink(link) < Err_Null)
      driver_fatal(device, "pa_c.d, failed to allocate link %d to the NCM", link);

     device->LinkAlloc = TRUE;
   }

  if (device->Debug) driver_report(device, "pa_c.d, allocating link %d to the NCM", link);
    
  if (device->ParsytecRoot)
   { int *reset = (int *) 0x000000C0;

     if (device->Debug) driver_report(device, "pa_c.d, asserting Parsytec-style reset");
     
	/* 1) Reset the processor currently connected, if any */
     *reset = 0;
     *reset = 1;
     *reset = 2;
     *reset = 3;
     *reset = 1 << link;
     Delay(5000);  /* 5 Msec is what is used by the network agent when booting */
     *reset = 0;

	/* 2) dial up the NCM itself */
     *reset = 0;
     *reset = 1;
     *reset = 2;
     *reset = 3;
     *reset = 1 << link;
   }
  else
   { int	*reset		= (int *) 0;
     int	*analyse	= (int *) 4;

     if (device->Debug) driver_report(device, "pa_c.d, asserting subsystem reset");
     
	/* 1) reset the currently connected processor, if any	*/
	/*    timings have been taken from tr_reset.c		*/
     *analyse	= 0;	/* stop it floating */
     Delay(10000);
     *analyse	= 1;
     Delay(10000);
     *reset	= 1;
     Delay(10000);
     *reset	= 0;
     Delay(10000);
     *analyse	= 0;
     Delay(10000);
     
    	/* Now ring up the NCM */
     *reset = 1;
   }

  { BYTE	ncm_junk[15];

    if (device->Debug) driver_report(device, "pa_c.d, attempting initial interaction with the NCM");
    Ncm_Read(device, 15, ncm_junk);
    Ncm_Write(device, 15, ncm_junk);
    Ncm_Read(device, 4, (BYTE *) &result);
    if (device->Debug) driver_report(device, "pa_c.d, channel to NCM is %d", result);
  }
  
  if (device->ParsytecRoot)
   { int *reset = (int *) 0x000000C0;
     *reset = 0;
   }
  else
   { int *reset = (int *) 0;
     *reset = 0;
   }
   
  if (!device->SensibleNetwork)
   { if (FreeLink(link) < Err_Null)
      driver_report(device, "pa_c.d warning, failed to release link to the NCM");
     else
      device->LinkAlloc = FALSE;
   }

  if (result eq -1)
   driver_fatal(device, "pa_c.d unable to initialise NCM interaction");
   
  return(result);
}

/**
*** Routines for interacting with the NCM software. This involves transmitting
*** data across a dumb link.
**/
static void Ncm_Read(NetworkDCB *device, int amount, BYTE *buffer)
{ if (LinkIn(amount, device->ConfLink, buffer, 90 * OneSec) ne Err_Null)
   driver_fatal(device, "pa_c.d, failed to read %d bytes of data from link %d to the NCM",
   		amount, device->ConfLink);
}

static void Ncm_Write(NetworkDCB *device, int amount, BYTE *buffer)
{ if (LinkOut(amount, device->ConfLink, buffer, 90 * OneSec) ne Err_Null)
   driver_fatal(device, "pa_c.d, failed to write %d bytes of data to link %d to the NCM",
   		amount, device->ConfLink);
}

/**-----------------------------------------------------------------------------
*** driver_Initialise(). This is responsible for assigning Pins to all the
*** processors controlled by this configuration driver. In addition it
*** processes the option string.
**/
static	void	initialise_aux1(RmProcessor, int *, NetworkDCB *);
static	void	parse_options(NetworkDCB *device, char *);

static void	driver_Initialise(NetworkDCB *device, DriverRequest *request)
{ int			sequence_number = 1;
  RmHardwareFacility	*hardware;  
  int			i;

  driver_report(device, "pa_c.d driver version %s", VersionNumber);

  
  hardware		= device->HardwareFacility;

	/* The link to the NCM has not been allocated when things start up */
  device->LinkAlloc	= FALSE;

	/* If no hardware option is given then the configuration	*/
	/* driver defaults to the Perihelion MultiCluster.		*/
  device->ParsytecRoot		= TRUE;
  device->Limited		= TRUE;
  device->Debug			= FALSE;
  if (strlen(hardware->Option) ne 0)
   parse_options(device, hardware->Option);

	/* The configuration link can be determined. If the root	*/
	/* processor has a link to a native processor called NCM then	*/
	/* this is the config link and the network is considered	*/
	/* sensible. Otherwise the config link is the first link to	*/
	/* a processor controlled by this device driver, and the	*/
	/* network is not sensible.					*/
  { RmProcessor	rootproc = device->RootProcessor;
    int		number_links = RmCountLinks(rootproc);
    RmProcessor	neighbour;
    int		i, j, destlink;
    
    for (i = 0; i < number_links; i++)
     { neighbour = RmFollowLink(rootproc, i, &destlink);
       if ((neighbour eq RmM_NoProcessor) || (neighbour eq RmM_ExternalProcessor))
        continue;
       if (!strcmp(neighbour->ObjNode.Name, "NCM"))
        { device->ConfLink = i;
          device->SensibleNetwork = TRUE;
          goto done_rootproc;
        }
     }

    device->SensibleNetwork = FALSE;
    for (i = 0; i < number_links; i++)
     { neighbour = RmFollowLink(rootproc, i, &destlink);
       if ((neighbour eq RmM_NoProcessor) || (neighbour eq RmM_ExternalProcessor))
        continue;
       for (j = 0; j < hardware->NumberProcessors; j++)
        if (neighbour eq hardware->Processors[j])
         { device->ConfLink = i;
           goto done_rootproc;
         }
     }
    driver_fatal(device, "pa_c.d, the root processor does not have a connection to the NCM");
  }
done_rootproc:

  for (i = 0; i < hardware->NumberProcessors; i++)
   { RmProcessor Processor = hardware->Processors[i];
     initialise_aux1(Processor, &sequence_number, device);
     if (device->Debug)
      driver_report(device, "pa_c.d, processor %s has pin %d",
	        Processor->ObjNode.Name, ExtractPin(device, Processor));
   }   

  device->Pin_Table = (int) Malloc(hardware->NumberProcessors * sizeof(Pin_Entry));
  if (device->Pin_Table eq (int) NULL)
   { request->FnRc = EC_Error + SS_NetServ + EG_NoMemory + EO_Link; return; }

  BuildPinTable(device);

  RingNcm(device);

  request->FnRc = Err_Null;
}

/**
*** For every processor, figure out an pin. For the workers these
*** are simple integers starting at 0. For a non-worker life is a little
*** bit more complicated. Non-workers can be given names of the form
*** 1100, 1200, etc. for compatibility. Alternatively they can be 
*** given an attribute Id=1100, to make things a little bit cleaner.
*** Unlike the Telmat system there is no check for attributes DSK, MEM,
*** etc. to determine the type of processor since there is no SNconfig
*** file to map these sensibly onto slot numbers.
**/

static void	initialise_aux1(RmProcessor Processor, int *sequence_number,
		NetworkDCB *device)
{ DriverEntry	*driver;

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
#if 0
	/* Similarly, the current Parsytec configuration driver does not    */
	/* yet support dynamic reconfiguration because the NCM is not up to */
	/* it.								    */
  { int i;
    for (i = 0; i < 4; i++)	/* Parsytec transputers always have four links */
     { RmLink *link = RmFindLink(Processor, i);
       link->Flags |= RmF_Configurable;
     }    
  }
#endif

  if (strlen(Processor->ObjNode.Name) eq 4)
   { char *temp = Processor->ObjNode.Name;
     if ((temp[2] eq '0') && (temp[3] eq '0'))	/* 1100, 1200, etc. */
      { driver->Aux1 = driver_atoi(Processor->ObjNode.Name);
        return;
      }      
   }
  { char *temp = (char *) RmGetProcessorAttribute(Processor, "pin");
    if (temp ne Null(char))
     { driver->Aux1 = driver_atoi(temp);
       if (driver->Aux1 eq 0)
        driver_fatal(device, "pa_c.d, processor %s has an invalid pin attribute %s",
        		Processor->ObjNode.Name, temp);
       return;
     }
  }
	/* If I fall through to here then the processor is just a worker. */
	/* Workers do not have to be given names 01, 02 etc. corresponding*/
	/* to the Pin numbers.						  */
  driver->Aux1 = (*sequence_number)++;
}

/**
*** Parsing the device driver options. At present the following options
*** are accepted:
*** 
**/
static void parse_options(NetworkDCB *device, char *str)
{ char	option[64], *tmp, *start, junk;

  strcpy(option, str);
  
  for (tmp = option; (*tmp eq ' ') || (*tmp eq '\t'); tmp++);

  until (*tmp eq '\0')
   { start = tmp;
     for ( ; (*tmp ne ':') && (*tmp ne ',') && (*tmp ne ' ') && (*tmp ne '\t') && (*tmp ne '\0'); tmp++);
     junk = *tmp;
     *tmp = '\0';

     if (!strcmp(start, "debug"))
      { device->Debug = TRUE; goto done_opt; }
     if (!strcmp(start, "inmos"))
      { device->ParsytecRoot = FALSE; goto done_opt; }
     if (!strcmp(start, "MC2/32-2") || !strcmp(start, "MC2/64-4"))
      { device->Limited = TRUE; goto done_opt; }
     if (!strcmp(start, "MC2/16-1") ||
         !strcmp(start, "SC16") ||
         !strcmp(start, "SC32") ||
         !strcmp(start, "SC48") ||
         !strcmp(start, "SC64") ||
         !strcmp(start, "SC128") ||
         !strcmp(start, "SC256") ||
         !strcmp(start, "SC320"))
      goto done_opt;
      
     driver_report(device, "pa_c.d, unknown option %s", start);
     driver_report(device, "pa_c.d, options are: debug, inmos, MC2/32-2, MC2/64-4, MC2/16-1");
     driver_report(device, "pa_c.d, SC16, SC32, SC48, SC64, SC128, SC256 or SC320");

done_opt:
     *tmp = junk;
     for ( ; (*tmp eq ':') || (*tmp eq ' ') || (*tmp eq ',') || (*tmp eq '\t'); tmp++);
   }
}

/**-----------------------------------------------------------------------------
*** The current Parsytec configuration driver does not support dynamic
*** link reconfiguration.
**/
static void	driver_MakeLinks(NetworkDCB *device, DriverConfRequest *request)
{ driver_report(device, "pa_c.d, MakeLinks reconfiguration routine called illegally");
  request->FnRc = EC_Fatal + SS_NetServ + EG_WrongFn + EO_Network;
}

/**
*** Similarly, it is not possible to test for link configurations if no
*** dynamic link reconfiguration is supported.
**/
static void	driver_TestLinks(NetworkDCB *device, DriverConfRequest *request)
{ driver_report(device, "pa_c.d, TestLinks reconfiguration routine called illegally");
  request->FnRc = EC_Fatal + SS_NetServ + EG_WrongFn + EO_Network;
}


/**
*** FreeProcessors(). This is not currently used by the Network Server,
*** although it might become part of the system shutdown procedure at some
*** time in the future. It actually makes a small amount of sense with version
*** 3.0 of the NCM software.
**/
static void driver_FreeProcessors(NetworkDCB *device, DriverRequest *request)
{ int	fncode = 0;

  (void) RingNcm(device);
  Ncm_Write(device, sizeof(int), (BYTE *) &fncode);
  request->FnRc = Err_Null;
}

/**
*** No-op for now. With version 3.0 of the NCM it is not possible to obtain
*** a network separate from configuring it, so the obtaining cannot be done
*** until the initial links are made.
**/
static void driver_ObtainProcessors(NetworkDCB *device, DriverRequest *request)
{ request->FnRc = Err_Null;
  device = device;
  return;
}

/**
*** This routine does the following jobs.
*** 1) there is already a link to the NCM, courtesy of the driver
***    initialise routine
*** 2) send the NCM function code 1 (request network) or function code 5
***    (request Eulerian network), depending on the network.
*** 3) send the NCM a line for every processor controlled by this driver,
***    i.e. an allocate request
*** 4) then send it details of all the links
*** 5) send a terminator, int 0
*** 6) read back a reply code
*** 7) if the network involves Eulerian cycles, read back the actual
***    configuration and update the internal network representation
*** 8) if the network is not sensible, contact with the NCM has been lost
*** 9) return control to the Network Server
**/

typedef struct	LinkConn {
	int	Pin;
	int	Link;
} LinkConn;

typedef struct	ProcessorConn {
	RmProcessor	Processor;
	int		Pin;
	LinkConn	Connection[4];
} ProcessorConn;

typedef struct NcmReply {
	int		Garbage;
	int		Pin;
	int		Link[4];
} NcmReply;
	
static void driver_MakeInitialLinks(NetworkDCB *device, DriverRequest *request)
{ RmHardwareFacility		*hardware;
  int				i, j, len, fncode;
  char				buf[80], *tmp;
  ProcessorConn			*conns = Null(ProcessorConn);

  if (device->Debug) driver_report(device, "pa_c.d, attempting to make initial connections");
  hardware = device->HardwareFacility;

	/* If it is necessary to re-build the link connections after	*/
	/* the NCM has finished, some extra data is needed.		*/
  if (device->Limited)
   { conns = Malloc(hardware->NumberProcessors * sizeof(ProcessorConn));
     if (conns eq Null(ProcessorConn))
      driver_fatal(device, "pa_c.d, not enough memory to initialise");
     for (i = 0; i < hardware->NumberProcessors; i++)
      { conns[i].Processor = RmM_NoProcessor;
        conns[i].Pin	   = -1;
        for (j = 0; j < 4; j++)
         { conns[i].Connection[j].Pin  = -1;
           conns[i].Connection[j].Link = -1;
         }
      }
   }
    
  if (device->Limited)
   fncode = 5;
  else
   fncode = 1;

  if (device->Debug) driver_report(device, "pa_c.d, sending request code %d", fncode);
  Ncm_Write(device, sizeof(int), (BYTE *) &fncode);

  strcpy(buf, "PROC 0 0");
  len = strlen(buf);
  if (device->Debug) driver_report(device, "pa_c.d, sending (%s)", buf);
  Ncm_Write(device, sizeof(int), (BYTE *) &len);
  Ncm_Write(device, len, buf);
  
  for (i = 0; i < hardware->NumberProcessors; i++)
   { RmProcessor processor = hardware->Processors[i];
     if (processor eq device->RootProcessor) continue;
     
     strcpy(buf, "PROC ");
     tmp = buf + strlen(buf);
     tmp = driver_itoa(tmp, i+1);	/* dummy processor number */
     *tmp++ = ' ';
     tmp = driver_itoa(tmp, ExtractPin(device, processor));
     *tmp = '\0';
     len = strlen(buf);

     if (device->Debug) driver_report(device, "pa_c.d, sending (%s)", buf);
     Ncm_Write(device, sizeof(int), (BYTE *) &len);
     Ncm_Write(device, len, buf);
   }

  if (device->Debug) driver_report(device, "pa_c.d, determining link connections");
  
  for (i = 0; i < hardware->NumberProcessors; i++)
   { RmProcessor	processor = hardware->Processors[i];
     RmProcessor	neighbour;
     int		link, destlink, number_links;
     int		pin, neighbour_pin;

     if (processor eq device->RootProcessor) continue;
     pin  = ExtractPin(device, processor);

     if (device->Limited)
      { conns[i].Processor = processor;
        conns[i].Pin	   = pin;
      }
           
     number_links = RmCountLinks(processor);
     for (link = 0; link < number_links; link++)
      { neighbour = RmFollowLink(processor, link, &destlink);

		/* Unconnected processors can always be ignored */
        if (neighbour eq RmM_NoProcessor) continue;

        	/* External processors cannot be supported with NCM 3.0 */
        if (neighbour eq RmM_ExternalProcessor)
         { driver_report(device, "pa_c.d, cannot yet cope with external links");
           RmBreakLink(processor, link);
           continue;
         }
		/* The root processor is special. As far as I can tell it has Pin 0 */
        if (neighbour eq device->RootProcessor)
	 neighbour_pin = 0;
	else
	 { neighbour_pin = ExtractPin(device, neighbour);
	   if (neighbour_pin < 0)
	    { driver_report(device, "pa_c.d, cannot connect internal and external links");
	      RmBreakLink(processor, link);
	      continue;
	    }
	   if (pin > neighbour_pin) continue;	/* avoid duplicates */
	   if ((pin eq neighbour_pin) && (link > destlink)) continue;
         }
		/* If only limited configurations are available,	*/
		/* break all links at this point.			*/
	if (device->Limited)
	 { RmBreakLink(processor, link);
	   conns[i].Connection[link].Pin  = neighbour_pin;
	   conns[i].Connection[link].Link = destlink;
	 }

	strcpy(buf, "LINK ");
	tmp = buf + strlen(buf);
	tmp = driver_itoa(tmp, pin);
	*tmp++ = ' ';
	tmp = driver_itoa(tmp, link);
	*tmp++ = ',';
	*tmp++ = ' ';
	tmp = driver_itoa(tmp, neighbour_pin);
	*tmp++ = ' ';
	tmp = driver_itoa(tmp, destlink);
	*tmp = '\0';
	len = strlen(buf);
	
        if (device->Debug) driver_report(device, "pa_c.d, requesting (%s)", buf);
 	Ncm_Write(device, sizeof(int), (BYTE *) &len);
 	Ncm_Write(device, len, buf);
      }		/* for every link of this processor	*/
   }		/* for every controlled processor	*/
   
  	/* Terminate the Ncm request */
  len = 0;
  Ncm_Write(device, sizeof(int), (BYTE *) &len);

  if (device->Debug) driver_report(device, "pa_c.d, awaiting availability code");
  
	/* Receive back the availability code */
  Ncm_Read(device, sizeof(int), (BYTE *) &fncode);
  if (fncode ne 0)
   { Ncm_Read(device, sizeof(int), (BYTE *) &len);
     driver_fatal(device, "pa_c.d, failed to allocate processors, only %d are available",
      		fncode - 1);
   }

  if (device->Debug) driver_report(device, "pa_c.d, awaiting error code");
  
	/* Receive back the error code */
  Ncm_Read(device, sizeof(int), (BYTE *) &fncode);
  if (fncode ne 0)
   driver_fatal(device, "pa_c.d, failed to configure network, fault %d", fncode);
    
	/* The configuration request has succeeded */
  request->FnRc = Err_Null;

  if (!device->Limited)
   {	/* The NCM sends back some strange information about number of	*/
   	/* NCU's and their connections					*/
     int	number_ncus, i, j, ncu_number, channel;

     if (device->Debug) driver_report(device, "pa_c.d, reading NCU details");
     
     Ncm_Read(device, sizeof(int), (BYTE *) &number_ncus);
     for (i = 0; i < number_ncus; i++)
      { Ncm_Read(device, sizeof(int), (BYTE *) &ncu_number);
      	for (j = 0; j < 96; j++)
      	 Ncm_Read(device, sizeof(int), (BYTE *) &channel);
      }
   }
  else
   { int		connection_count, i, j, index, index2;
     NcmReply		*reply = Null(NcmReply);
          
     if (device->Debug) driver_report(device, "pa_c.d, reading back actual connections");
     Ncm_Read(device, sizeof(int), (BYTE *) &connection_count);

     reply = Malloc(connection_count * sizeof(NcmReply));
     if (reply eq Null(NcmReply))
      driver_fatal(device, "pa_c.d, not enough memory to initialise");

	/* Some nasties are required to cope with possible structure	*/
	/* re-arrangement by the compiler				*/      
     for (i = 0; i < connection_count; i++)
      { int	temp[6];
        Ncm_Read(device, sizeof(NcmReply), (BYTE *) temp);
        reply[i].Garbage = temp[0];
        reply[i].Pin	 = temp[1];
        reply[i].Link[0] = temp[2];
        reply[i].Link[1] = temp[3];
        reply[i].Link[2] = temp[4];
        reply[i].Link[3] = temp[5];
      }

     if (device->Debug)
      { for (i = 0; i < connection_count; i++)
         driver_report(device, "NCM reply: pin %d, links %d %d %d %d",
         		reply[i].Pin, reply[i].Link[0], reply[i].Link[1],
         		reply[i].Link[2], reply[i].Link[3]);
      }
 		      
	/* For every connection specified in the conns table...	*/
     for (i = 0; i < hardware->NumberProcessors; i++)
      for (j = 0; j < 4; j++)
       { if (conns[i].Connection[j].Pin eq -1) continue;

		/* find the relevant entry in the reply table for the source */
	 for (index = 0; index < connection_count; index++)
	  if (reply[index].Pin eq conns[i].Pin) break;
	 if (index >= connection_count)
	  driver_fatal(device, "pa_c.d, NCM failed to return details of Pin %d",
	  		conns[i].Pin);
	  
		/* special and nasty case for the root processor	*/
	 if (conns[i].Connection[j].Pin eq 0)
	  { RmProcessor	this       = conns[i].Processor;
	    int		sourcelink = reply[index].Link[j];
	    int		destlink   = conns[i].Connection[j].Link;

	    if (device->Debug)
	     driver_report(device, "pa_c.d, connecting /%s link %d to /%s link %d",
	     	this->ObjNode.Name, sourcelink,
	     	device->RootProcessor->ObjNode.Name, destlink);
	    
	    RmMakeLink(this, sourcelink, device->RootProcessor, destlink);
	    continue;
	  }

		/* find the relevant entry in the reply table for the neighbour */	  
	 for (index2 = 0; index2 < connection_count; index2++)
	  if (reply[index2].Pin eq conns[i].Connection[j].Pin)
	   break;
	 if (index2 >= connection_count)
	  driver_fatal(device, "pa_c.d, NCM has not sent back details for Pin %d",
		conns[i].Connection[j].Pin);

	 { RmProcessor source		= conns[i].Processor;
	   int	       sourcelink	= reply[index].Link[j];
	   RmProcessor dest		= ExtractProcessor(device,
	   				     reply[index2].Pin);
	   int	       destlink		=
	   	reply[index2].Link[conns[i].Connection[j].Link];
	   	
	   if (device->Debug)
	    driver_report(device, "pa_c.d, connecting /%s link %d to /%s link %d",
	    		source->ObjNode.Name, sourcelink,
	    		dest->ObjNode.Name, destlink);

	   RmMakeLink(source, sourcelink, dest, destlink);
	 }
       }	/* for every link, for every processor, in conns table */
     Free(conns);
     Free(reply);             
   }

	/* Once the network has been configured, I may have lost the	*/
	/* connection to the NCM					*/
  if (!device->SensibleNetwork)
   { FreeLink(device->ConfLink);
     device->LinkAlloc = FALSE;
   }
}


