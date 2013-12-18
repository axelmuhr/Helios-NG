/* $Header: /hsrc/servers/serial/RCS/serial.c,v 1.14 1991/05/31 15:04:12 paul Exp $ */
/* $Source: /hsrc/servers/serial/RCS/serial.c,v $ */
/************************************************************************/ 
/* serial.c - Helios server for RS232 serial line devices.		*/
/*									*/
/* Copyright 1990 Active Book Company Ltd., Cambridge, England	*/
/*									*/
/* Author: Brian Knight, 10th March 1990				*/
/************************************************************************/

/*
 * $Log: serial.c,v $
 * Revision 1.14  1991/05/31  15:04:12  paul
 * brians update
 *
 * Revision 1.6  91/02/20  13:18:55  brian
 * Checkpoint before adding serial line link guardian.
 * 
 * Revision 1.12  90/10/18  16:44:52  brian
 * Added "/00/rom" to path name of serial device driver.
 * 
 * Revision 1.11  90/10/18  16:39:31  brian
 * Removed "/files" from path name of serial device driver.
 * 
 * Revision 1.10  90/10/18  13:54:10  brian
 * Bug fix to number decoding.
 * 
 * Revision 1.9  90/10/18  13:44:44  brian
 * More useful help information.
 * 
 * Revision 1.7  90/10/18  11:26:23  brian
 * Scaling and smoothing tweaked for latest demo digitiser.
 * 
 * Revision 1.5  90/10/07  09:40:32  brian
 * Checkpoint before moving to SMT
 * 
 * Revision 1.4  90/07/25  16:24:02  brian
 * Checkpoint before adding eventing mouse/stylus support.
 * 
 * Revision 1.3  90/07/05  16:17:20  brian
 * Checkpoint before fiddling with semaphore handling
 * 
 * Revision 1.2  90/06/21  15:40:36  brian
 * Checkpoint before abolishing call of DoRead().
 * 
 * Revision 1.1  90/06/12  10:01:20  brian
 * Initial revision
 * 
 */

#include <helios.h>
#include <string.h>
#include <codes.h>
#include <syslib.h>
#include <servlib.h>
#include <task.h>
#include <message.h>
#include <attrib.h>
#include <stdio.h>
#include <process.h>
#include <stdlib.h>
#include <dev/serialdev.h>

#ifdef __SERIALLINK
#include <cpustate.h>
#include <link.h>
#include <root.h>

word *_GetModTab(void); /* Not in any C header (really for kernel use only) */

#endif /* __SERIALLINK */


/************************************************************************/
/* Macros								*/ 
/************************************************************************/

/* #define LINKTRACE */	 /* Enable tracing IOdebugs from link guardian	*/

#define REQSTACK    2000 /* Stack size for request-handling processes	*/
#define MACHNAMEMAX  100 /* Space for longest machine name		*/
#define DEVNAME     "/rom/sys/helios/lib/serialdev.dev"
                         /* File containing device driver		*/
#define READBUFSIZE  256 /* Size of input buffer used in server		*/
#define WRITEBUFSIZE 256 /* Size of output buffer used in server	*/
#define EVENTBUFSIZE 256 /* Size of serial buffer used for event streams*/

/* Default serial line: use channel B of on-board serial chip		*/
#define DEFAULTUNIT    0 /* Unit number for "/mcname/rs232/default"	*/
#define DEFAULTCHANNEL 1 /* Channel number for "/mcname/rs232/default"	*/

/* Channels used for stylus and mouse devices */
#define STYLUSUNIT     DEFAULTUNIT
#define STYLUSCHANNEL  DEFAULTCHANNEL
#define MOUSEUNIT      DEFAULTUNIT
#define MOUSECHANNEL   DEFAULTCHANNEL

/* Device stream modes. The same underlying serial channel may be 	*/
/* opened in different modes via different names.			*/
#define RAWDEVICE      1	/* Unprocessed serial stream		*/
#define STYLUSDEVICE   2	/* Event stream for stylus/digitiser	*/
#define MOUSEDEVICE    3	/* Event stream for serial mouse	*/

/* Entry in name server and root directory name */
#define SERVERNAME  "rs232" /* Standard Helios name for serial devices  */

#define WAITFOREVER   MaxInt        /* Infinite timeout value		    */
#define EVENTTIMEOUT  (OneSec * 30) /* Discard events not sent in this time */

/* Number of IO events which can be sent in one message */
#define MAXEVENTSINMESSAGE (IOCDataMax / sizeof(IOEvent))


/************************************************************************/
/* Types								*/ 
/************************************************************************/

typedef struct SerialDev
{
  ObjNode objNode;	/* Must be first field in this structure	*/
} SerialDev;

/* Macros for extracting unit and channel numbers, etc. from a SerialDev */
#define GETUNIT(dev)    (int)(dev)->objNode.Contents.Head
#define GETCHANNEL(dev) (int)(dev)->objNode.Contents.Earth
#define GETDEVTYPE(dev) (int)(dev)->objNode.Contents.Tail
#define GETDCB(dev)	dcbs[GETUNIT(dev)]

/* Structure used to hold partial events between calls to the serial	*/
/* driver. It also holds the previous coordinates (for mouse events)	*/
/* and the previous button state.					*/

typedef struct EventRec
{
  unsigned char buf[EVENTBUFSIZE];    /* Buffer for serial data   	 */
  int           residue;	      /* Offset of next free buffer slot */
  int 		counter;	      /* Serial number of event		 */
  SHORT		lastX;		      /* Last X position reported	 */
  SHORT		lastY;		      /* Last Y position reported	 */
  int		lastFlags;	      /* Previous state of flags         */
} EventRec;

/************************************************************************/
/* Forward references							*/
/************************************************************************/

static void SerialDoEscape(ServInfo *);
static void SerialDoOpen(ServInfo *);
static void SerialDoCreate(ServInfo *);
static void SerialDoServerInfo(ServInfo *);
static void SerialDoDelete(ServInfo *);
static void SerialDoClose(ServInfo *);
static void SerialDoObjInfo(ServInfo *);
static void SerialDoLocate(ServInfo *);

static void SerialGetInfo(MCB *m, SerialDev *dev);
static void SerialSetInfo(MCB *m, SerialDev *dev);
static void SerialRead(MCB *m, SerialDev *dev);
static void SerialWrite(MCB *m, SerialDev *dev);
static int  OpenDevices(void);
static void Action(DCB *dcb, SerialReq *req);
static void PrintFault(char *prefix, int code);
static SerialDev *NewDirEntry(DirNode *dir, char *name, int flags, 
			      Matrix matrix, int unit, int channel, 
			      int streamType);
static int OpenChannel(SerialDev *dev);
static int CloseChannel(SerialDev *dev);
static int DeviceRead(DCB *dcb, int chanNum, unsigned char *buf, word len, 
		      word timeout);
static int DeviceWrite(DCB *dcb, int chanNum, byte *buf, word len, 
		       word timeout);
static SaveState *DeviceAbort(DCB *dcb, int chanNum, int write);
static void GenerateMouseEvents(SerialDev *dev, EventRec *eventRec, Port port);
static void GenerateStylusEvents(SerialDev *dev, EventRec *eventRec, 
				 Port port);
static void SetMouseEvent(IOEvent *event, EventRec *eventRec, 
			  WORD buttonEvent);
static void SetStylusEvent(IOEvent *event, EventRec *eventRec, 
			   WORD buttonEvent);
void DecodeArgs(int argc, char **argv);

#ifdef __SERIALLINK
void SerialLinkGuardian(void);
#endif /* __SERIALLINK */

/************************************************************************/
/* Static variables							*/
/************************************************************************/

DirNode serialDir;	/* Directory containing all the serial devices */

/* Dispatch table							*/
/* The "Do..." functions are the standard ones from the server library,	*/
/* while the "SerialDo..." ones are local.				*/

static DispatchInfo serialInfo =
{
  &serialDir,		/* Directory holding serial devices		*/
  NullPort,		/* Request port (filled in later)		*/
  SS_Device,		/* Subsystem code (is this the appropriate one?)*/
  NULL,			/* Parent name (set to processor name later)	*/
  {SerialDoEscape, REQSTACK}, /* Escape function for non-standard requests */
  {
    {SerialDoOpen,	 REQSTACK},	/* FG_Open	 */
    {SerialDoCreate,	 REQSTACK},	/* FG_Create	 */
    {SerialDoLocate,   	 REQSTACK},	/* FG_Locate	 */
    {SerialDoObjInfo, 	 REQSTACK},	/* FG_ObjectInfo */
    {SerialDoServerInfo, REQSTACK},	/* FG_ServerInfo */
    {SerialDoDelete,	 REQSTACK},	/* FG_Delete	 */
    {DoRename,		 REQSTACK},	/* FG_Rename	 */
    {DoLink,		 REQSTACK},	/* FG_Link	 */
    {DoProtect,		 REQSTACK},	/* FG_Protect	 */
    {DoSetDate,		 REQSTACK},	/* FG_SetDate	 */
    {DoRefine,		 REQSTACK},	/* FG_Refine	 */
    {SerialDoClose,	 REQSTACK}	/* FG_CloseObj	 */
  }
};

/* Array to hold the DCB pointer for each device present */

static DCB *dcbs[MAXSERIALDEVICES];

/* Scaling for raw digitiser coordinates */

static int stylusXMin =  1500; /* Defaults for digitiser V0.34 18/10/90 */
static int stylusXMax = 10700;
static int stylusYMin =  2000;
static int stylusYMax =  7850;

#ifdef __SERIALLINK
static int debugSerialLinkGuardian = 0;
#endif /* __SERIALLINK */

/*----------------------------------------------------------------------*/

int main(int argc, char **argv)
{
  char   mcName[MACHNAMEMAX];
  Object *machine;

  SetPriority(DevicePri); /* This server runs at high priority */

  DecodeArgs(argc, argv);

#ifdef __SERIALLINK
  if (debugSerialLinkGuardian)
  {
    SerialLinkGuardian(); /* Run link guardian instead of serial server */
    return 0;
  }
#endif /* __SERIALLINK */

  /* Set parent name to processor name */
  MachineName(mcName); /* Have to hope array is big enough */
  serialInfo.ParentName = mcName;
  machine = Locate(NULL, mcName); /* This machine's `directory' */
  
  /* Initialise the directory structure */

  InitNode((ObjNode *)&serialDir, SERVERNAME, Type_Directory, 0, DefDirMatrix);
  InitList(&serialDir.Entries);
  serialDir.Nentries = 0;
  
  serialInfo.ReqPort = NewPort(); /* Fill in request port */

  /* Put this server's name in the name server. It must have the same	*/
  /* name as its root directory node.					*/
  {
    Object   *nameTableEntry;
    NameInfo nameInfo;
    
    nameInfo.Port     = serialInfo.ReqPort;
    nameInfo.Flags    = Flags_StripName;
    nameInfo.Matrix   = DefDirMatrix;
    nameInfo.LoadData = NULL;
    
    nameTableEntry = Create(machine, serialDir.Name, Type_Name,
    			    sizeof(NameInfo), (byte *)&nameInfo);
    if (nameTableEntry == NULL)
    {
      printf("serial: failed to create directory '%s'\n", serialDir.Name);
      Exit(1);
      printf("Exit returned!\n");
    }
  }
  
  /* Put in a parent entry for the root directory (as a symbolic link) */
  {
    LinkNode *parent;
    
    parent = (LinkNode *)Malloc(sizeof(LinkNode) + (word)strlen(mcName));
    InitNode(&parent->ObjNode, "..", Type_Link, 0, DefDirMatrix);
    parent->Cap = machine->Access;
    strcpy(parent->Link, mcName);
    serialDir.Parent = (DirNode *)parent;
  }

  Close(machine);
  
  /* Open the underlying devices and set up a directory entry for each	*/
  /* channel found.							*/

  if (OpenDevices() > 0)
  {
    /* Enter the dispatcher, which will not normally return */
    Dispatch(&serialInfo);
  }
  else
    printf("serialserv: no serial devices found\n");
  
  Exit(0); /* Clean up name table entry and exit */
}

/*----------------------------------------------------------------------*/

/*----------------------------------------------------------------------*/
/* Convert a string to a number or return 0 if it is invalid.		*/
/*----------------------------------------------------------------------*/

int 
StrToNum(char *s, int *val)
{
  int len = strlen(s);
  int i;
  int v = 0;

  for (i = 0; i < len; ++i)
  {
    char ch = s[i];

    if ((ch < '0') || (ch > '9')) return 0; /* Bad digit */

    v = v*10 + ch - '0';
  }

  *val = v;
  return 1;
}

/*----------------------------------------------------------------------*/
/* Read a numerical argument if it is present				*/
/*----------------------------------------------------------------------*/
 
void ReadNumArg(int argc, char **argv, int argNum, int *result)
{
  if (argNum >= argc)
  {
    printf("numerical argument for '%s' missing\n", argv[argNum - 1]);
    return;
  }

  if (!StrToNum(argv[argNum], result))
    printf("bad numerical argument '%s' for '%s'\n", 
	   argv[argNum], argv[argNum - 1]);
}

/*----------------------------------------------------------------------*/
/* Decode any command line arguments					*/
/*----------------------------------------------------------------------*/

void
DecodeArgs(int argc, char **argv)
{
  int a = 1;

  while (a < argc)
  {
    char *arg = argv[a++];

    if (strcmp(arg, "-xmin") == 0)
      ReadNumArg(argc, argv, a++, &stylusXMin);
    else if (strcmp(arg, "-xmax") == 0)
      ReadNumArg(argc, argv, a++, &stylusXMax);
    else if (strcmp(arg, "-ymin") == 0)
      ReadNumArg(argc, argv, a++, &stylusYMin);
    else if (strcmp(arg, "-ymax") == 0)
      ReadNumArg(argc, argv, a++, &stylusYMax);
#ifdef __SERIALLINK
    else if (strcmp(arg, "-link") == 0)
      debugSerialLinkGuardian = 1;
    else if (strcmp(arg, "-help") == 0)
    {
      printf("usage: %s [-xmin n] [-xmax n] [-ymin n] [-ymax n] [-link]\n",
	     argv[0]);
      printf("current values: xmin %d, xmax %d, ymin %d, ymax %d\n",
	     stylusXMin, stylusXMax, stylusYMin, stylusYMax);
    }
#else /* __SERIALLINK */
    else if (strcmp(arg, "-help") == 0)
    {
      printf("usage: %s [-xmin n] [-xmax n] [-ymin n] [-ymax n]\n", argv[0]);
      printf("current values: xmin %d, xmax %d, ymin %d, ymax %d\n",
	     stylusXMin, stylusXMax, stylusYMin, stylusYMax);
    }
#endif /* __SERIALLINK */
    else 
      printf("argument '%s' ignored\n", arg);
  }
}

/*----------------------------------------------------------------------*/
/* Open all the serial devices present on this machine.			*/
/* A directory entry is created for each channel of each device, with	*/
/* names of the form "intUC" (for internal serial ports) and "extUC"	*/
/* (for external ports), where `U' is the unit number of the		*/
/* serial device, and `C' is a letter (`a', `b', etc.) giving the 	*/
/* channel name within the device. Thus the first channel of the first	*/
/* external unit is called "ext0a".					*/
/* The entry "default" is created as an alias for one port (but only if */
/* the relevant device is present).					*/
/* Entries "stylus" and "mouse" are also created as aliases for		*/
/* specific ports to provide event streams for those devices.		*/
/* Returns 0 if no devices were opened, 1 otherwise.			*/
/*----------------------------------------------------------------------*/
static int
OpenDevices(void)
{
  int unit;
  int nDevices = 0;

  /* Look for internal ports */
  for (unit = 0; unit < MAXSERIALDEVICES; ++unit)
  {
    SerialDevInfo info;

    info.logDevNum = unit;
    dcbs[unit] = OpenDevice(DEVNAME, &info);
    if (dcbs[unit])
    {
      SerialReq req;

      req.DevReq.Request   = FS_GetNumLines;
      req.DevReq.Action    = Action;
      req.DevReq.SubDevice = 0;
      req.DevReq.Timeout   = WAITFOREVER;

      Operate(dcbs[unit], &req); /* know that driver is synchronous */

      if (req.DevReq.Result < 0) 
	PrintFault("get num lines", (int)req.DevReq.Result);
      else
      {
	/* Install an entry for each line in the "/mcname/rs232" directory */
	int channel;

        /* IOdebug("opened podule %d (%d lines)\n", unit, req.Actual); */
	for (channel = 0; channel < req.Actual; ++channel)
	{
	  SerialDev *dev;
	  char      entryName[] = "intUC";

	  entryName[3] = '0' + unit;
	  entryName[4] = 'a' + channel;
	  dev = NewDirEntry(&serialDir, entryName, 0, DefFileMatrix, 
			    unit, channel, RAWDEVICE);
	  nDevices++;

	  /* Set up special aliases only if the underlying devices have	*/
	  /* been opened successfully.					*/

	  if ((unit == DEFAULTUNIT) && (channel == DEFAULTCHANNEL))
	    dev = NewDirEntry(&serialDir, "default", 0, DefFileMatrix, 
			      DEFAULTUNIT, DEFAULTCHANNEL, RAWDEVICE);

	  if ((unit == STYLUSUNIT) && (channel == STYLUSCHANNEL))
	    dev = NewDirEntry(&serialDir, "stylus", 0, DefFileMatrix, 
			      STYLUSUNIT, STYLUSCHANNEL, STYLUSDEVICE);

	  if ((unit == MOUSEUNIT) && (channel == MOUSECHANNEL))
	    dev = NewDirEntry(&serialDir, "mouse", 0, DefFileMatrix, 
			      MOUSEUNIT, MOUSECHANNEL, MOUSEDEVICE);
	}
      }
    }
    else
      printf("OpenDevice('%s') failed\n", DEVNAME);
  }

  return nDevices;
}

/*----------------------------------------------------------------------*/
/* Create a new directory entry for a device.				*/
/*----------------------------------------------------------------------*/

static SerialDev *
NewDirEntry(DirNode *dir, char *name, int flags, Matrix matrix, 
	    int unit, int channel, int streamType)
{
  SerialDev *dev = New(SerialDev);
  
  if (dev == NULL) return NULL;

  InitNode(&dev->objNode, name, Type_Device, flags, matrix);

  dev->objNode.Size    = 0;
  dev->objNode.Account = 0;
  /* Use Contents field to hold device number, channel number and type */
  dev->objNode.Contents.Head  = (Node *)unit; 
  dev->objNode.Contents.Earth = (Node *)channel;
  dev->objNode.Contents.Tail  = (Node *)streamType;

  Insert(dir, &dev->objNode, TRUE);
  return dev;
}

/* Create new node in response to server request (not useful?) */
static SerialDev *
CreateNode(MCB *m, DirNode *d, char *pathname)
{
  SerialDev *dev;
  char      *name;

  /* IOCCreate *req = (IOCCreate *)(m->Control); */
  name = objname(pathname);

  dev = NewDirEntry(d, name, 0, DefFileMatrix, 0, 0 /* channel TBD */, 0);
  if (dev == NULL) m->MsgHdr.FnRc |= EC_Error | EG_Create;
  return dev;
}

/*----------------------------------------------------------------------*/
/* Handle server request to create a new directory entry		*/
/*----------------------------------------------------------------------*/
static void
SerialDoCreate(ServInfo *servInfo)
{
  MCB       *m = servInfo->m;
  MsgBuf    *r;
  DirNode   *d;
  SerialDev *dev;
  IOCCreate *req = (IOCCreate *)(m->Control);
  char      *pathname = servInfo->Pathname;

  d = GetTargetDir(servInfo); /* Find target's parent directory */

  if (d == NULL)
  {
    ErrorMsg(m, EO_Directory);
    return;
  }

  dev = (SerialDev *)GetTargetObj(servInfo);	/* Look for target */
  m->MsgHdr.FnRc = SS_Device;		/* Reinitialise return code */

  if (dev != NULL)
  {
    ErrorMsg(m, EC_Error+EG_Create+EO_File);
    return;
  }

  /* Check that we can write to directory (i.e. create a new entry) */
  if (!CheckMask(req->Common.Access.Access, AccMask_W))
  {
    ErrorMsg(m, EC_Error+EG_Protected+EO_Directory);
    return;
  }

  r = New(MsgBuf);
  if (r == NULL)
  {
    ErrorMsg(m, EC_Error+EG_NoMemory+EO_Message);
    return;
  }

  dev = CreateNode(m, d, pathname);	/* Create new entry */
  if (dev == NULL)
  {
    ErrorMsg(m, EC_Error+EG_NoMemory+EO_File);
    Free(r);
    return;
  }

  FormOpenReply(r, m, &dev->objNode, 0, pathname);
  PutMsg(&r->mcb);
  Free(r);
}

/*----------------------------------------------------------------------*/
/* Handle server request to delete a directory entry			*/
/*----------------------------------------------------------------------*/
static void
SerialDoDelete(ServInfo *servInfo)
{
  MCB       *m = servInfo->m;
  SerialDev *dev;
  IOCCommon *req = (IOCCommon *)(m->Control);

  dev = (SerialDev *)GetTarget(servInfo); /* Find target entry */

  if (dev == NULL)
  {
    ErrorMsg(m, EO_Object); /* There is no EO_Device */
    return;
  }

  /* Check that we can delete this entry */
  if (!CheckMask(req->Access.Access, AccMask_D))
  {
    ErrorMsg(m, EC_Error+EG_Protected+EO_Object);
    return;
  }

  if (dev->objNode.Type == Type_Device)
  {
    if (dev->objNode.Account > 0)
    {
      /* Device in use: complain */
      ErrorMsg(m, EC_Error+EG_InUse+EO_Object);
      return;
    }
  }

  Unlink(&dev->objNode, FALSE /* parent dir not locked */);
  Free(dev);
  ErrorMsg(m, Err_Null);
}

/*----------------------------------------------------------------------*/
/* Open a serial device and handle all requests for it while it is open	*/
/*----------------------------------------------------------------------*/
static void
SerialDoOpen(ServInfo *servInfo)
{
  MCB       *m = servInfo->m;
  MsgBuf    *r;
  DirNode   *d;
  SerialDev *dev;
  IOCMsg2   *req = (IOCMsg2 *)(m->Control);
  Port      reqPort;
  Port	    eventPort = NullPort; /* Port for sending events */
  word      enabledEvent = 0; /* May be set to Event_Stylus or Event_Mouse */
  byte      *data = m->Data;
  char      *pathname = servInfo->Pathname;
  EventRec  eventRec; /* used to hold incomplete packets of serial data */

  /* Find target's parent directory */
  d = (DirNode *)GetTargetDir(servInfo);
  if (d == NULL)
  {
    ErrorMsg(m, EO_Directory);
    return;
  }

  r = New(MsgBuf);	/* Get reply buffer */
  if (r == NULL)
  {
    ErrorMsg(m, EC_Error+EG_NoMemory);
    return;
  }

  /* Find the target itself */
  dev = (SerialDev *)GetTargetObj(servInfo);

  /* Complain if entry does not exist */
  if (dev == NULL)
  {
    ErrorMsg(m, EO_File);
    Free(r);
    return;
  }

  /* Check that we are allowed to perform the operation requested */
  if (!CheckMask(req->Common.Access.Access, 
		 (AccMask)(req->Arg.Mode & Flags_Mode)))
  {
    ErrorMsg(m, EC_Error+EG_Protected+EO_File);
    Free(r);
    return;
  }

  /* If the target object is a device (rather than the directory	*/
  /* the devices) open the underlying device channel.			*/

  if (dev->objNode.Type != Type_Directory)
  {
    if (!OpenChannel(dev))
    {
      ErrorMsg(m, EC_Error+EG_InUse+EO_File);
      Free(r);
      return;
    }
  }

  /* Generate reply message */
  FormOpenReply(r, m, &dev->objNode, Flags_Server|Flags_Closeable, pathname);
  reqPort = NewPort();	/* Install message reply port */
  r->mcb.MsgHdr.Reply = reqPort;

  PutMsg(&r->mcb);
  Free(r);

  /* Deal with target object which is a directory */
  if (dev->objNode.Type == Type_Directory)
  {
    DirServer(servInfo, m, reqPort); /* Let server library handle it */
    FreePort(reqPort);
    return;
  }

  /* Enter a loop servicing requests from the client to access the open	*/
  /* device. We leave the target locked as it is not useful to have	*/
  /* multiple access to a serial line.					*/
  /* If events are enabled, then this loop is also used to collect	*/
  /* serial data and convert it to events.				*/

  dev->objNode.Account++; /* Count the user */
  /* Do not call UnLockTarget() here */

  for (;;)
  {
    word e;

    /* Collect any serial input available and generate events from it */
    if (enabledEvent == Event_Mouse)
      GenerateMouseEvents(dev, &eventRec, eventPort);
    else if (enabledEvent == Event_Stylus)
      GenerateStylusEvents(dev, &eventRec, eventPort);

    m->MsgHdr.Dest = reqPort;
    m->Timeout     = StreamTimeout;
    m->Data	   = data;

    /* Do not wait in GetMsg if events are enabled */
    if ((enabledEvent != 0) && (GetReady(reqPort) != 0))
      continue;

    e = GetMsg(m);
    
    if (e == EK_Timeout) 
    {
      if (enabledEvent)
	continue;	/* Timeout is not an error if generating events */
      else
      { 
	IOdebug("GetMsg timeout"); 
	break;	/* Real timeout - quit */
      }
    }

    if (e < Err_Null) 
      { IOdebug("GetMsg error %x", e); continue; }	/* Other errors - just loop */
    
    /* No need to lock object here as we allow only one open for each	*/
    /* device, so all operations are processed serially by this thread.	*/
    
    switch (m->MsgHdr.FnRc & FG_Mask)
    {
      case FG_Read:
        SerialRead(m, dev);
	break;

      case FG_Write:
	/* IOdebug("FG_Write"); */
	SerialWrite(m, dev);
	break;

      case FG_Close:
	/* IOdebug("FG_Close"); */
	/* Free the request port, close the device channel, unlock the	*/
	/* object and return.						*/
	if (m->MsgHdr.Reply != NullPort) ErrorMsg(m, Err_Null);
	FreePort(reqPort);
	dev->objNode.Account--;
	CloseChannel(dev); /* Close device channel */
	UnLockTarget(servInfo);
	return;

      case FG_GetSize:
	/* IOdebug("FG_GetSize"); */
	InitMCB(m, 0, m->MsgHdr.Reply, NullPort, Err_Null);
	MarshalWord(m, 0); /* Always return size as zero */
	PutMsg(m);
	break;

      case FG_GetInfo:
	/* IOdebug("FG_GetInfo"); */
	SerialGetInfo(m, dev);
	break;

      case FG_SetInfo:
	/* IOdebug("FG_SetInfo"); */
	SerialSetInfo(m, dev);
	break;

      case FG_EnableEvents:
      {
	word event = m->Control[0];

	if (event == 0) /* Disabling events? */
	{
	  if (enabledEvent != 0)
	  {
	    if (FreePort(eventPort) != Err_Null)
	      IOdebug("failed to free event port");
	    eventPort    = NullPort;
	    enabledEvent = 0;
	  }

	  ErrorMsg(m, Err_Null); /* Not an error to disable twice */
	  /* May be better to construct reply by hand... */
	}
	else if ((event == Event_Mouse) || (event == Event_Stylus))
	{
	  /* It is an error if events are already enabled on this stream */
	  if (eventPort != NullPort)
	    ErrorMsg(m, EC_Error+EG_InUse+EO_File);
	  else
	  {
	    enabledEvent = event;
	    eventPort    = m->MsgHdr.Reply; /* Port provided by EnableEvents */
	    eventRec.lastX 	 = 0; /* Initialise event buffer */
	    eventRec.lastY       = 0;
	    eventRec.lastFlags 	 = 0;
	    eventRec.residue	 = 0;
	    eventRec.counter	 = 0;
	    
	    m->Control[0] 	 = event;
	    m->MsgHdr.Flags	 = (event == 0) ? 0 : MsgHdr_Flags_preserve;
	    m->MsgHdr.ContSize	 = 1;
	    m->MsgHdr.DataSize 	 = 0;
	    m->MsgHdr.Dest	 = m->MsgHdr.Reply;
	    m->MsgHdr.Reply	 = NullPort;
	    m->MsgHdr.FnRc	 = 0;
	    m->Timeout		 = 5 * OneSec;
	    (void) PutMsg(m);
	  }
	}
	else
	  ErrorMsg(m, EC_Error+EG_Parameter+EO_File); /* Invalid event type */

        break;
      }

      default: /* Including FG_Seek and FG_SetSize */
	/* IOdebug("bad func code"); */
	ErrorMsg(m, EC_Error+EG_FnCode+EO_File);
	break;
    }
    
    /* Would unlock object here if we had locked it above */
  } /* End of main loop */
  
  /* Come here on exit from main loop when stream closed */
  
  dev->objNode.Account--;	/* Uncount this user */
  FreePort(reqPort);
  
  /* Return, terminating process which handled an open serial device */
}

/*----------------------------------------------------------------------*/
/* Read any serial data available and generate mouse events from it.	*/
/* The event message will contain more than one event if we have 	*/
/* several complete serial packets (thus helping a slow client to catch	*/
/* up). Any incomplete serial packet is left in `eventRec' for next	*/
/* time.								*/
/* 									*/
/* A 3-button serial mouse produces 5-byte packets of data:		*/
/*									*/
/*   Byte 0: 10000LMR   Phase and buttons (1 for not pressed)		*/
/*   Byte 1: X inc      Signed 8-bit X increment			*/
/*   Byte 2: Y inc      Signed 8-bit Y increment			*/
/*   Byte 3: X inc      Another signed 8-bit X increment		*/
/*   Byte 4: Y inc      Another signed 8-bit X increment		*/
/*----------------------------------------------------------------------*/

#define MAXEVENTSPERPACKET    3	/* One mouse packet can result in 3 events */
#define LEFTMOUSEBUTTON	   0x04
#define MIDDLEMOUSEBUTTON  0x02
#define RIGHTMOUSEBUTTON   0x01
#define MOUSEBUTTONS (LEFTMOUSEBUTTON | MIDDLEMOUSEBUTTON | RIGHTMOUSEBUTTON)

static void
GenerateMouseEvents(SerialDev *dev, EventRec *eventRec, Port port)
{
  int 		residue  = eventRec->residue;
  unsigned char *buf     = &eventRec->buf[0];
  int 		maxBytes = EVENTBUFSIZE - residue;
  DCB 		*dcb     = GETDCB(dev);
  int 		chanNum  = GETCHANNEL(dev);
  int 		got      = DeviceRead(dcb, chanNum, &buf[residue],
				      maxBytes, 0);
  int  		len      = residue + got;
  IOEvent	events[MAXEVENTSINMESSAGE]; /* Buffer for events message */
  int		eventNum = 0; /* Position within `events' */
  int		pos;
  int		i;

  if (got == 0)
  {
    Delay(OneSec/10); /* Prevent excessive polling rate when no data */
    return;
  }

  /* Look for complete mouse packets within the buffer and generate an	*/
  /* event for each one. The mouse can generate other bytes between	*/
  /* packets (when power is cycled or its mode is changed) so it is	*/
  /* safest not to assume that packets are contiguous.			*/

  /* The loop continues while there are complete mouse packets in the	*/
  /* serial buffer and while the buffer for the event message still has	*/
  /* room for the maximum number of events which one packet might	*/
  /* generate.								*/

  pos = 0;

  while ((pos < (len-4)) && 
	 (eventNum <= (MAXEVENTSINMESSAGE - MAXEVENTSPERPACKET)))
  {
    if ((buf[pos] & 0xF8) != 0x80)
      ++pos; /* This byte is not the start of a mouse packet */
    else
    {
      /* Have found a complete packet */
      int flags     = ~(buf[pos] & MOUSEBUTTONS);
      int flagsOn   = flags & ~eventRec->lastFlags; /* Flags which came on */
      int flagsOff  = ~flags & eventRec->lastFlags; /* Flags which went off */
      int xInc	    = (signed char)buf[pos+1] + (signed char)buf[pos+3];
      int yInc      = (signed char)buf[pos+2] + (signed char)buf[pos+4];
      int oldEventNum = eventNum; /* Used to see if any events sent */

      pos += 5; /* Move on to next packet */

      eventRec->lastX     += xInc;  /* Record the new position */
      eventRec->lastY     += yInc;
      eventRec->lastFlags =  flags; /* Record the flag state   */

      /* Look for buttons which have gone down */
      if (flagsOn)
      {
	if (flagsOn & LEFTMOUSEBUTTON)  
	  SetMouseEvent(&events[eventNum++], eventRec, Buttons_Left_Down);
	if (flagsOn & MIDDLEMOUSEBUTTON)
	  SetMouseEvent(&events[eventNum++], eventRec, Buttons_Middle_Down);
	if (flagsOn & RIGHTMOUSEBUTTON) 
	  SetMouseEvent(&events[eventNum++], eventRec, Buttons_Right_Down);
      }

      /* Look for buttons which have gone up */
      if (flagsOff)
      {
	if (flagsOff & LEFTMOUSEBUTTON)   
	  SetMouseEvent(&events[eventNum++], eventRec, Buttons_Left_Up);
	if (flagsOff & MIDDLEMOUSEBUTTON)
	  SetMouseEvent(&events[eventNum++], eventRec, Buttons_Middle_Up);
	if (flagsOff & RIGHTMOUSEBUTTON)
	  SetMouseEvent(&events[eventNum++], eventRec, Buttons_Right_Up);
      }

      /* If no button-change events have been sent, but the mouse has	*/
      /* moved, then send an event to announce the new position.	*/

      if ((eventNum == oldEventNum) & ((xInc) || (yInc)))
	SetMouseEvent(&events[eventNum++], eventRec, Buttons_Unchanged);
    }
  }

  /* All the currently available serial data has been processed. Move	*/
  /* any remaining partial packet down to the start of the buffer.	*/

  for (i = 0; i < (len - pos); ++i)
    buf[i] = buf[pos + i];
  eventRec->residue = len - pos;

  /* If any events were generated, send them off to the client */

  if (eventNum > 0)
  {
    MCB mcb;

    mcb.MsgHdr.Flags    = MsgHdr_Flags_preserve;
    mcb.MsgHdr.ContSize = 0;
    mcb.MsgHdr.DataSize = eventNum * sizeof(IOEvent);
    mcb.MsgHdr.FnRc	= EventRc_IgnoreLost;
    mcb.MsgHdr.Dest	= port;
    mcb.MsgHdr.Reply    = NullPort;
    mcb.Control		= Null(WORD);
    mcb.Data		= (BYTE *)&events[0];
    mcb.Timeout		= EVENTTIMEOUT;
      
    if (PutMsg(&mcb) != 0)
      IOdebug("PutMsg of mouse events failed");
  }
}

/*----------------------------------------------------------------------*/
/* Set the contents of one IOEvent structure for a mouse event.		*/
/*----------------------------------------------------------------------*/
static void
SetMouseEvent(IOEvent *event, EventRec *eventRec, WORD buttonEvent)
{
  event->Type	       	      = Event_Mouse;
  event->Counter       	      = eventRec->counter++;
  event->Stamp	       	      = GetDate();
  event->Device.Mouse.X       = eventRec->lastX;
  event->Device.Mouse.Y       = eventRec->lastY;
  event->Device.Mouse.Buttons = buttonEvent;
}


/*----------------------------------------------------------------------*/
/* Read any serial data available and generate stylus events from it.	*/
/* The event message will contain more than one event if we have 	*/
/* several complete serial packets (thus helping a slow client to catch	*/
/* up). Any incomplete serial packet is left in `eventRec' for next	*/
/* time.								*/
/* The three buttons on the digitiser board are also handled here.	*/
/* This routine filters stylus coordinates to remove jitter and 	*/
/* glitches.								*/
/*									*/
/* The AB serial digitiser board produces 5-byte packets of data:	*/
/*									*/
/*   Byte 0: 1P0TSBGR   Phase bit, proximity and buttons		*/
/*   Byte 1: 0xxxxxxx   ls 7 bits of x coordinate			*/
/*   Byte 2: 0XXXXXXX   ms 7 bits of x coordinate			*/
/*   Byte 3: 0yyyyyyy	ls 7 bits of y coordinate			*/
/*   Byte 4: 0YYYYYYY	ms 7 bits of y coordinate			*/
/*									*/
/* Buttons (1 when pressed):						*/
/*									*/
/*   T: Tip switch 							*/
/*   S: Side (barrel) button						*/
/*   B: Blue button							*/
/*   G: Grey (yellow) button						*/
/*   R: Red button							*/
/*									*/
/*   P: 0 for in proximity						*/
/*									*/
/* Coordinates are 14 bits unsigned. The value 0x3FFF indicates an	*/
/* out-of-range value which should be ignored.				*/
/*----------------------------------------------------------------------*/
#define GLITCH               50	/* Be suspicious about movements > GLITCH */
#define MOVAV_WINDOW          5	/* Points included in moving average */
#define MOVAV(old, new) ((old*(MOVAV_WINDOW-1) + new) / MOVAV_WINDOW)
#define REDSTYLUSBUTTON	   0x01
#define GREYSTYLUSBUTTON   0x02
#define BLUESTYLUSBUTTON   0x04
#define STYLUSBARRELSWITCH 0x08
#define STYLUSTIPSWITCH    0x10
#define STYLUSBUTTONS	   (REDSTYLUSBUTTON  | GREYSTYLUSBUTTON   | \
			    BLUESTYLUSBUTTON | STYLUSBARRELSWITCH | \
			    STYLUSTIPSWITCH  | STYLUSOUTOFPROX)
#define STYLUSOUTOFPROX	   0x40
#define STYLUSPHASEFLAG	   0x80

static void
GenerateStylusEvents(SerialDev *dev, EventRec *eventRec, Port port)
{
  int 		residue  = eventRec->residue;
  unsigned char *buf     = &eventRec->buf[0];
  int 		maxBytes = EVENTBUFSIZE - residue;
  DCB 		*dcb     = GETDCB(dev);
  int 		chanNum  = GETCHANNEL(dev);
  int 		got      = DeviceRead(dcb, chanNum, &buf[residue],
				      maxBytes, 0);
  int  		len      = residue + got;
  IOEvent	events[MAXEVENTSINMESSAGE]; /* Buffer for events message */
  int		eventNum = 0; /* Position within `events' */
  int		pos;
  int		i;
  int lastJumpBig = 0; /* Used in glitch removal */

  if (got == 0)
  {
    Delay(OneSec/20); /* Prevent excessive polling rate when no data */
    return;
  }

  /* Look for complete stylus packets within the buffer and generate an	*/
  /* event for each one. 						*/

  /* The loop continues while there are complete stylus packets in the	*/
  /* serial buffer and while the buffer for the event message still has	*/
  /* room for the maximum number of events which one packet might	*/
  /* generate.								*/

  pos = 0;

  while ((pos < (len-4)) && 
	 (eventNum <= (MAXEVENTSINMESSAGE - MAXEVENTSPERPACKET)))
  {
    if ((buf[pos] & STYLUSPHASEFLAG) == 0)
      ++pos; /* This byte is not the start of a stylus packet */
    else
    {
      /* Have found a complete packet */
      int flags     = buf[pos] & STYLUSBUTTONS;
      int flagsOn   = flags & ~eventRec->lastFlags; /* Flags which came on */
      int flagsOff  = ~flags & eventRec->lastFlags; /* Flags which went off */
      int x = (buf[pos+2] << 7) + buf[pos+1];
      int y = (buf[pos+4] << 7) + buf[pos+3];
      int validPoint = (x != 0x3FFF) && (y != 0x3FFF);
#ifdef INVERTED
      int pixelX = XMAX - ((x - stylusXMin)*640)/(stylusXMax - stylusXMin);
      int pixelY = YMAX - ((y - stylusYMin)*400)/(stylusYMax - stylusYMin);
#else /* INVERTED */
      int pixelX = ((x - stylusXMin)*640)/(stylusXMax - stylusXMin);
      int pixelY = ((y - stylusYMin)*400)/(stylusYMax - stylusYMin);
#endif /* INVERTED */
      int xDiff  = abs(pixelX - eventRec->lastX);
      int yDiff  = abs(pixelY - eventRec->lastY);
      int oldEventNum = eventNum; /* Used to see if any events sent */
      int stylusMoved = 0; /* Set for valid, different, in-prox point */

      pos += 5; /* Move on to next packet */
      eventRec->lastFlags = flags; /* Record the flag state */

      /* Stylus movement */
      if (validPoint && ((flags & STYLUSOUTOFPROX) == 0))
      {
	/* Discard isolated distant samples (probably glitches) */
	if ((xDiff > GLITCH) || (yDiff > GLITCH))
	{
	  if (lastJumpBig)
	    lastJumpBig = 0; /* This could be a real movement */
	  else
	  {
	    lastJumpBig = 1;
	    validPoint  = 0; /* Ignore this sample */
	  }
	}
	  
	if (validPoint)
	{
	  int newX = MOVAV(eventRec->lastX, pixelX); 
	  int newY = MOVAV(eventRec->lastY, pixelY);

	  /* Has the stylus moved? */
	  if ((newX != eventRec->lastX) || (newY != eventRec->lastY))
	  {
	    eventRec->lastX = newX;  /* Record the new position */
	    eventRec->lastY = newY;
	    stylusMoved = 1; /* It is worth telling the client */
	  }
	}
      }

      /* Look for buttons which have gone down */
      if (flagsOn)
      {
	if (flagsOn & REDSTYLUSBUTTON)
	  SetStylusEvent(&events[eventNum++], eventRec, Buttons_Left_Down);
	if (flagsOn & GREYSTYLUSBUTTON)
	  SetStylusEvent(&events[eventNum++], eventRec, Buttons_Middle_Down);
	if (flagsOn & BLUESTYLUSBUTTON)
	  SetStylusEvent(&events[eventNum++], eventRec, Buttons_Right_Down);
	if (flagsOn & STYLUSTIPSWITCH)
	  SetStylusEvent(&events[eventNum++], eventRec, Buttons_Tip_Down);
	if (flagsOn & STYLUSBARRELSWITCH)
	  SetStylusEvent(&events[eventNum++], eventRec, Buttons_Barrel_Down);
	if (flagsOn & STYLUSOUTOFPROX)
	  SetStylusEvent(&events[eventNum++], eventRec, Buttons_OutOf_Prox);
      }

      /* Look for buttons which have gone up */
      if (flagsOff)
      {
	if (flagsOff & REDSTYLUSBUTTON)
	  SetStylusEvent(&events[eventNum++], eventRec, Buttons_Left_Up);
	if (flagsOff & GREYSTYLUSBUTTON)
	  SetStylusEvent(&events[eventNum++], eventRec, Buttons_Middle_Up);
	if (flagsOff & BLUESTYLUSBUTTON)
	  SetStylusEvent(&events[eventNum++], eventRec, Buttons_Right_Up);
	if (flagsOff & STYLUSTIPSWITCH)
	  SetStylusEvent(&events[eventNum++], eventRec, Buttons_Tip_Up);
	if (flagsOff & STYLUSBARRELSWITCH)
	  SetStylusEvent(&events[eventNum++], eventRec, Buttons_Barrel_Up);
	if (flagsOff & STYLUSOUTOFPROX)
	  SetStylusEvent(&events[eventNum++], eventRec, Buttons_Into_Prox);
      }

      /* If no button-change events have been sent, but the stylus has	*/
      /* moved, then send an event to announce the new position.	*/

      if ((eventNum == oldEventNum) & stylusMoved)
	SetStylusEvent(&events[eventNum++], eventRec, Buttons_Unchanged);
    }
  }

  /* All the currently available serial data has been processed. Move	*/
  /* any remaining partial packet down to the start of the buffer.	*/

  for (i = 0; i < (len - pos); ++i)
    buf[i] = buf[pos + i];
  eventRec->residue = len - pos;

  /* If any events were generated, send them off to the client */

  if (eventNum > 0)
  {
    MCB mcb;

    mcb.MsgHdr.Flags    = MsgHdr_Flags_preserve;
    mcb.MsgHdr.ContSize = 0;
    mcb.MsgHdr.DataSize = eventNum * sizeof(IOEvent);
    mcb.MsgHdr.FnRc	= EventRc_IgnoreLost;
    mcb.MsgHdr.Dest	= port;
    mcb.MsgHdr.Reply    = NullPort;
    mcb.Control		= Null(WORD);
    mcb.Data		= (BYTE *)&events[0];
    mcb.Timeout		= EVENTTIMEOUT;
      
    if (PutMsg(&mcb) != 0)
      IOdebug("PutMsg of stylus events failed");
  }
}


/*----------------------------------------------------------------------*/
/* Set the contents of one IOEvent structure for a stylus event.	*/
/*----------------------------------------------------------------------*/
static void
SetStylusEvent(IOEvent *event, EventRec *eventRec, WORD buttonEvent)
{
  event->Type	       	       = Event_Stylus;
  event->Counter               = eventRec->counter++;
  event->Stamp	               = GetDate();
  event->Device.Stylus.X       = eventRec->lastX;
  event->Device.Stylus.Y       = eventRec->lastY;
  event->Device.Stylus.Buttons = buttonEvent;
}


/*----------------------------------------------------------------------*/
/* Handle server request to close an object.				*/
/*----------------------------------------------------------------------*/
static void
SerialDoClose(ServInfo *servInfo)
{
  MCB       *m = servInfo->m;
  SerialDev *dev;
  /* IOCCommon *req = (IOCCommon *)(m->Control); */

  IOdebug("SerialDoClose");
  dev = (SerialDev *)GetTarget(servInfo); /* Find target entry */

  if (dev == NULL)
  {
    ErrorMsg(m, EO_Object); /* There is no EO_Device */
    return;
  }

  ErrorMsg(m, Err_Null);
}

/*----------------------------------------------------------------------*/
/* Handle server request to give server info.				*/
/*----------------------------------------------------------------------*/
static void
SerialDoServerInfo(ServInfo *servInfo)
{
  IOdebug("SerialDoServerInfo called\n");
  NullFn(servInfo);
}

/*----------------------------------------------------------------------*/
/* Handle non-standard server requests					*/
/*----------------------------------------------------------------------*/
static void
SerialDoEscape(ServInfo *servInfo)
{
  MCB       *m = servInfo->m;
  SerialDev *dev;
  /* IOCCommon *req = (IOCCommon *)(m->Control); */

  IOdebug("SerialDoEscape");
  dev = (SerialDev *)GetTarget(servInfo); /* Find target entry */

  if (dev == NULL)
  {
    ErrorMsg(m, EO_Object); /* There is no EO_Device */
    return;
  }

  ErrorMsg(m, Err_Null);
}

/************************************************************************/
/* Read from the serial device driver.					*/
/************************************************************************/
static void
SerialRead(MCB *m, SerialDev *dev)
{
  ReadWrite *rw       = (ReadWrite *)m->Control;
  word      pos       = rw->Pos;
  int       size      = (int)rw->Size;
  int	    bytesLeft = size;
  word	    timeout   = rw->Timeout; /* Microseconds */
  unsigned char buf[READBUFSIZE]; /* Buffer used for reading from device */
  DCB       *dcb      = GETDCB(dev);
  int	    chanNum   = GETCHANNEL(dev);
  word	    seq       = 0;
  Port	    reply     = m->MsgHdr.Reply;

  if (pos < 0) /* Actually, pos is irrelevant for a device stream */
  {
    IOdebug("pos < 0");
    ErrorMsg(m, EC_Error | EG_Parameter | 1);
    return;
  }

  if (size < 0)
  {
    IOdebug("size < 0");
    ErrorMsg(m, EC_Error | EG_Parameter | 2);
    return;
  }

  InitMCB(m, MsgHdr_Flags_preserve, reply, NullPort, ReadRc_More);

  while (bytesLeft > 0)
  {
    int thisTime = (bytesLeft > READBUFSIZE ? READBUFSIZE : bytesLeft);
    int got      = DeviceRead(dcb, chanNum, buf, thisTime, timeout);

    if (got < 0)
    {
      PrintFault("read", got);
      ErrorMsg(m, got);
      break;
    }

    bytesLeft -= got;
    /* Adjusting timeout TBD */

    m->Data            = (byte *)buf;
    m->MsgHdr.DataSize = got;

    /* Check for last buffer of transfer (all read or timed out) */
    if ((bytesLeft == 0) || (timeout <= 0))
    {
      m->MsgHdr.FnRc  = ReadRc_EOD | seq;
      m->MsgHdr.Flags = 0;
    }

    if (PutMsg(m) < Err_Null)
    {
      IOdebug("PutMsg failed in SerialRead");
      break;
    }

    if (timeout <= 0) break;

    seq += ReadRc_SeqInc;
    m->MsgHdr.FnRc = ReadRc_More | seq;
  }

  rw->Size = (word)size - (word)bytesLeft;
  dev->objNode.Dates.Access = GetDate();
}

/************************************************************************/
/* Routines to write to the device					*/
/************************************************************************/

/* Structure used to pass info to GetOutputBuffer */

typedef struct WriteInfo
{
  Buffer buf;	   /* Fixed buffer used in each call of GetOutputBuffer	*/
  DCB    *dcb;	   /* DCB of underlying device				*/
  int    chanNum;  /* Channel number on device				*/
  word   timeout;  /* In microseconds					*/
} WriteInfo;

/*----------------------------------------------------------------------*/
/* Routine called by DoWrite() to get a buffer.				*/
/* DoWrite() does not make any explicit call to dispose of a full 	*/
/* buffer, so this routine writes out the last set of data to the 	*/
/* serial device before returning the same buffer as an empty one.	*/
/*----------------------------------------------------------------------*/
static Buffer *
GetOutputBuffer(word pos, WriteInfo *info)
{
  word size = info->buf.Size;

  /* IOdebug("GetOutputBuffer"); */
  if (size > 0)
  {
    word written;

    /* IOdebug("calling DeviceWrite"); */
    written = DeviceWrite(info->dcb, info->chanNum, info->buf.Data, size,
			  info->timeout);
    if (written != size) return NULL; /* We lose the real error */
  }

  /* Return the same buffer */
  info->buf.Pos  = pos; /* Don't care what this is but must return it */
  info->buf.Size = 0;
  info->buf.Max  = WRITEBUFSIZE;
  /* Adjusting remaining timeout TBD */
  return &info->buf;
}

/*----------------------------------------------------------------------*/
/* Write to the serial device driver.					*/
/*----------------------------------------------------------------------*/
static void
SerialWrite(MCB *m, SerialDev *dev)
{
  ReadWrite *rw  = (ReadWrite *)m->Control;
  word	    timeout = rw->Timeout; /* What units is this in? */
  WriteInfo writeInfo;
  byte      data[WRITEBUFSIZE];	/* Buffer used by GetOutputBuffer() */

  /* Set up the info structure which DoWrite will pass to		*/
  /* GetOutputBuffer on each call. The same buffer is emptied and 	*/
  /* returned each time.					 	*/

  writeInfo.buf.Data = data;
  writeInfo.buf.Size = 0;
  writeInfo.buf.Max  = WRITEBUFSIZE;
  writeInfo.dcb	     = GETDCB(dev);
  writeInfo.chanNum  = GETCHANNEL(dev);
  writeInfo.timeout  = timeout;

  /* It would probably be better to include a private DoWrite() here	*/
  /* (Better handling of errors, less fiddling with Buffers.)	    	*/

  DoWrite(m, GetOutputBuffer, &writeInfo);

  /* DoWrite can leave the last buffer with some data in it, so flush 	*/
  /* it here.								*/

  if (writeInfo.buf.Size > 0)
  {
    word written;

    /* IOdebug("flushing %d bytes", writeInfo.buf.Size); */
    written = DeviceWrite(writeInfo.dcb, writeInfo.chanNum, 
			  writeInfo.buf.Data, writeInfo.buf.Size, timeout);
    if (written != writeInfo.buf.Size)
      ErrorMsg(m, EC_Error+EG_Broken+EO_Stream); /* Better error code TBD */
  }


  dev->objNode.Dates.Modified = GetDate();
  dev->objNode.Dates.Access   = GetDate();
}

/*----------------------------------------------------------------------*/
/* Get the attributes of this stream					*/
/*----------------------------------------------------------------------*/
static void
SerialGetInfo(MCB *m, SerialDev *dev)
{
  SerialReq  req;
  int	     unit    = GETUNIT(dev);
  int        channel = GETCHANNEL(dev);
  DCB        *dcb    = dcbs[unit];
  Attributes attrs;

  req.DevReq.Request   = FG_GetInfo;
  req.DevReq.Action    = Action;
  req.DevReq.SubDevice = channel;
  req.DevReq.Timeout   = WAITFOREVER;
  req.Buf              = &attrs;

  Operate(dcb, &req);  /* This operation is synchronous */

  if (req.DevReq.Result < 0) 
  {
    ErrorMsg(m, req.DevReq.Result);
    return;
  }

  /* Generate reply message */
  InitMCB(m, 0, m->MsgHdr.Reply, NullPort, Err_Null);
  MarshalData(m, sizeof(Attributes), req.Buf);
  PutMsg(m);
}


/*----------------------------------------------------------------------*/
/* Set the attributes of this stream					*/
/*----------------------------------------------------------------------*/
static void
SerialSetInfo(MCB *m, SerialDev *dev)
{
  SerialReq  req;
  int	     unit    = GETUNIT(dev);
  int        channel = GETCHANNEL(dev);
  DCB        *dcb    = dcbs[unit];
  Attributes *attrs  = (Attributes *)m->Data;

  req.DevReq.Request   = FG_SetInfo;
  req.DevReq.Action    = Action;
  req.DevReq.SubDevice = channel;
  req.DevReq.Timeout   = WAITFOREVER;
  req.Buf              = attrs;

  Operate(dcb, &req);  /* This operation is synchronous */

  if (req.DevReq.Result < 0) 
  {
    ErrorMsg(m, req.DevReq.Result);
    return;
  }

  /* Generate reply message */
  InitMCB(m, 0, m->MsgHdr.Reply, NullPort, Err_Null);
  PutMsg(m);
}

/*----------------------------------------------------------------------*/
/* Dummy routine to give us control before DoObjInfo is entered.	*/
/*----------------------------------------------------------------------*/
static void
SerialDoObjInfo(ServInfo *servInfo)
{
  printf("SerialDoObjInfo");
  DoObjInfo(servInfo);
}

/*----------------------------------------------------------------------*/
/* Dummy routine to give us control before DoLocate is entered.		*/
/*----------------------------------------------------------------------*/
static void
SerialDoLocate(ServInfo *servInfo)
{
  /* IOdebug("SerialDoLocate"); */
  DoLocate(servInfo);
}

/*----------------------------------------------------------------------*/

/************************************************************************/
/* Operations on the underlying device driver.				*/
/************************************************************************/

/*----------------------------------------------------------------------*/
/* Open one channel of a device. 0 => failure, non-0 => success.	*/
/*----------------------------------------------------------------------*/

static int
OpenChannel(SerialDev *dev)
{
  SerialReq req;
  int	    unit    = GETUNIT(dev);
  int       channel = GETCHANNEL(dev);
  DCB       *dcb    = dcbs[unit];

  req.DevReq.Request   = FS_OpenChannel;
  req.DevReq.Action    = Action;
  req.DevReq.SubDevice = channel;
  req.DevReq.Timeout   = WAITFOREVER;

  Operate(dcb, &req);  /* This operation is synchronous */

  if (req.DevReq.Result < 0) 
    PrintFault("open channel", (int)req.DevReq.Result);
  return (req.DevReq.Result == 0);
}

/*----------------------------------------------------------------------*/
/* Close one channel of a device. 0 => failure, non-0 => success.	*/
/*----------------------------------------------------------------------*/

static int
CloseChannel(SerialDev *dev)
{
  SerialReq req;
  int	    unit    = GETUNIT(dev);
  int       channel = GETCHANNEL(dev);
  DCB       *dcb    = dcbs[unit];

  req.DevReq.Request   = FS_CloseChannel;
  req.DevReq.Action    = Action;
  req.DevReq.SubDevice = channel;
  req.DevReq.Timeout   = WAITFOREVER;

  Operate(dcb, &req);  /* This operation is synchronous */

  if (req.DevReq.Result < 0) 
    PrintFault("close channel", (int)req.DevReq.Result);
  return (req.DevReq.Result == 0);
}

/*----------------------------------------------------------------------*/

static int 
DeviceRead(DCB *dcb, int chanNum, unsigned char *buf, word len, word timeout)
{
  SerialReq req;

  req.DevReq.Request   = FG_Read;
  req.DevReq.Action    = Action;
  req.DevReq.SubDevice = chanNum;
  req.DevReq.Timeout   = timeout;
  req.Buf              = buf;
  req.Size             = len;

  Operate(dcb, &req); /* read operation is synchronous */

  if (req.DevReq.Result < 0) 
    return (int)req.DevReq.Result;
  else
    return (int)req.Actual;
}

/*----------------------------------------------------------------------*/

static int 
DeviceWrite(DCB *dcb, int chanNum, byte *buf, word len, word timeout)
{
  SerialReq req;

  req.DevReq.Request   = FG_Write;
  req.DevReq.Action    = Action;
  req.DevReq.SubDevice = chanNum;
  req.DevReq.Timeout   = timeout;
  req.Buf              = buf;
  req.Size             = len;

  /* IOdebug("writing %d bytes", len); */
  Operate(dcb, &req); /* write operation is synchronous */

  if (req.DevReq.Result < 0) PrintFault("write", (int)req.DevReq.Result);
  return (int)req.Actual;
}

/*----------------------------------------------------------------------*/
/* Abort the current read or write operation, returning a pointer to	*/
/* the SaveState of the waiting process (or 0 if none).			*/
/* The waiting process is not Resumed.					*/
/*----------------------------------------------------------------------*/

static SaveState *
DeviceAbort(DCB *dcb, int chanNum, int write)
{
  SerialReq req;

  req.DevReq.Request   = (write ? FS_AbortWrite : FS_AbortRead);
  req.DevReq.Action    = Action;
  req.DevReq.SubDevice = chanNum;

  Operate(dcb, &req); /* abort operation is synchronous */

  if (req.DevReq.Result < 0) PrintFault("abort", (int)req.DevReq.Result);

  return (SaveState *)req.Actual;
}

/*----------------------------------------------------------------------*/

/* Dummy Action routine: all driver calls are synchronous */
static void 
Action(DCB *dcb, SerialReq *req)
{
  dcb = dcb; req = req;
}

/*----------------------------------------------------------------------*/

/* THIS SHOULD NOT BE DONE LIKE THIS! */
void PrintFault(char *prefix, int code)
{
  /* char mess[256]; */

  /* Fault(code, mess, 256); */
  /* printf("%s: %s\n", prefix, mess); */

  printf("%s: ", prefix);
  switch(code)
  {
    case SERIAL_SERROROVERRUN:
      printf("overrun"); break;

    case SERIAL_SERRORBADSUBDEVICE:
      printf("bad subdevice"); break;

    case SERIAL_SERRORINUSE:
      printf("in use"); break;

    default:
      printf(" fault %x", code); break;
  }
  printf("\n");
}

/*----------------------------------------------------------------------*/

#ifdef __SERIALLINK

/*----------------------------------------------------------------------*/
/* Procedures used to perform (and abort) link transfers on the		*/
/* serial link. The address of this procedure is recorded in the link	*/
/* structure so that it can be called from LinkTx etc. in the kernel.	*/
/* info->size == 0 specifies the abort function.			*/
/* The result is really a SaveState pointer when transmission or 	*/
/* reception is aborted.					       	*/
/*----------------------------------------------------------------------*/
word
SerialLinkTx(LinkTransferInfo *info)
{
  DCB  *dcb = (DCB *)info->link->LinkInfo_DCB;
  char *cb  = info->buf;
  int  res;

#ifdef LINKTRACE
  if (info->size == 0)
    IOdebug("SerialLinkTx abort");
  else
    IOdebug("SerialLinkTx size %d", info->size);
#endif

  if (info->size == 0)
    return (word)DeviceAbort(dcb, DEFAULTCHANNEL, 1 /* abort tx */);

  res = DeviceWrite(dcb, DEFAULTCHANNEL, info->buf, info->size, WAITFOREVER);

#ifdef LINKTRACE
  if (res == info->size)
    IOdebug("tx data starts: %x %x %x %x", cb[0], cb[1], cb[2], cb[3]);
  else
    IOdebug("tx failed (sent %d): starts: %x %x %x %x", res,
	    cb[0], cb[1], cb[2], cb[3]);

  {
    int i;

    for (i = 4; i < res; i += 8)
    {
      IOdebug("tx continues: %x %x %x %x %x %x %x %x",
	      cb[i],   cb[i+1], cb[i+2], cb[i+3], 
	      cb[i+4], cb[i+5], cb[i+6], cb[i+7]);
    }
  }
#endif

  return 0; /* To keep compiler happy */
}

word
SerialLinkRx(LinkTransferInfo *info)
{
  DCB  *dcb = (DCB *)info->link->LinkInfo_DCB;
  char *cb  = info->buf;
  int  res;

#ifdef LINKTRACE
  if (info->size == 0)
    IOdebug("SerialLinkRx abort");
  else
    IOdebug("SerialLinkRx size %d", info->size);
#endif

  if (info->size == 0)
    return (word)DeviceAbort(dcb, DEFAULTCHANNEL, 0 /* abort rx */);

  res = DeviceRead(dcb, DEFAULTCHANNEL, info->buf, info->size, WAITFOREVER);

#ifdef LINKTRACE
  if (res == info->size)
    IOdebug("rx data starts: %x %x %x %x", cb[0], cb[1], cb[2], cb[3]);
  else
    IOdebug("rx failed (got %d): starts: %x %x %x %x", res,
	    cb[0], cb[1], cb[2], cb[3]);

  {
    int i;

    for (i = 4; i < res; i += 8)
    {
      IOdebug("rx continues: %x %x %x %x %x %x %x %x",
	      cb[i],   cb[i+1], cb[i+2], cb[i+3], 
	      cb[i+4], cb[i+5], cb[i+6], cb[i+7]);
    }
  }
#endif

  return 0; /* To keep compiler happy */
}

/*----------------------------------------------------------------------*/
/* Run a link guardian using a serial line instead of a transputer link	*/
/*----------------------------------------------------------------------*/
void 
SerialLinkGuardian(void)
{
  LinkInfo  	**links;	/* Table of link structures */
  LinkInfo  	*link;
  DCB	 	*dcb;
  SerialDevInfo info;
  SerialReq	req;
  RootStruct	*root = GetRoot();
  Config	*config = (Config *)root->Configuration;
  Attributes	attrs;

  IOdebug("starting serial link guardian");

  if (SERIALLINKID >= config->NLinks)
  {
    IOdebug("Not enough links configured: need %d, have %d", SERIALLINKID+1,
	    config->NLinks);
    return;
  }

  links = root->Links;
  link  = links[SERIALLINKID];

  if (link->State != Link_State_Dead)
  {
    IOdebug("link %d already running (state %d)", SERIALLINKID, link->State);
    return;
  }

  info.logDevNum = DEFAULTUNIT;

  dcb = OpenDevice(DEVNAME, &info);
  if (dcb == 0)
  {
    IOdebug("failed to open serial device");
    return;
  }

  req.DevReq.Request   = FS_OpenChannel;
  req.DevReq.Action    = Action;
  req.DevReq.SubDevice = DEFAULTCHANNEL;
  req.DevReq.Timeout   = WAITFOREVER;

  Operate(dcb, &req);  /* This operation is synchronous */

  if (req.DevReq.Result < 0) 
  {
    IOdebug("failed to open serial channel %d", DEFAULTCHANNEL);
    CloseDevice(dcb);
    return;
  }

  /* Disable XON and XOFF handling on this channel.			*/
  /* First get the current attributes.					*/

  req.DevReq.Request   = FG_GetInfo;
  req.Buf              = &attrs;

  Operate(dcb, &req);  /* This operation is synchronous */

  if (req.DevReq.Result < 0) 
  {
    IOdebug("failed to get serial channel attributes");
    goto LinkGuardianExit;
  }

  RemoveAttribute(&attrs, RS232_IXON);  /* Do not steal incoming XON/XOFF */
  RemoveAttribute(&attrs, RS232_IXOFF); /* Do not issue XON/XOFF	  */
#if 1
  IOdebug("forcing 38400 baud");
  SetInputSpeed(&attrs, RS232_B38400);
  SetOutputSpeed(&attrs, RS232_B38400);
#endif

  req.DevReq.Request   = FG_SetInfo;
  Operate(dcb, &req);  /* This operation is synchronous */

  if (req.DevReq.Result < 0) 
  {
    IOdebug("failed to set serial channel attributes");
    goto LinkGuardianExit;
  }

  /* Fill in link fields necessary for calling this device.		  */
  /* The DCB and module table ptrs live in the channel fields of the link */
  /* Note that this assumes that a single system-wide address space. 	  */

  link->LinkInfo_DCB    = (Channel *)dcb;
  link->LinkInfo_ModTab = (Channel *)_GetModTab(); /* This task's mod table */
  link->TxFunction      = SerialLinkTx;
  link->RxFunction      = SerialLinkRx;
  link->Mode		= Link_Mode_Intelligent;

  /* Call the link guardian routine in the kernel */
  IntelligentServer(link);
  IOdebug("IntelligentServer returned");

LinkGuardianExit:
  req.DevReq.Request   = FS_CloseChannel;
  req.DevReq.Action    = Action;
  req.DevReq.SubDevice = DEFAULTCHANNEL;
  req.DevReq.Timeout   = WAITFOREVER;

  Operate(dcb, &req);  /* This operation is synchronous */

  CloseDevice(dcb);
}

#endif /* __SERIALLINK */

/*----------------------------------------------------------------------*/

/* End of serial.c */
