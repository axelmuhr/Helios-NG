/* $Header: /giga/Helios/servers/codec/RCS/codecHigh.c,v 1.1 91/02/14 23:01:34 paul Exp $ */
/* $Source: /giga/Helios/servers/codec/RCS/codecHigh.c,v $ */
/************************************************************************/ 
/* codecHigh.c - library wrapper for ARM Helios codec device driver     */
/*									*/
/* Copyright (C) 1989 Active Book Company Ltd., Cambridge, England	*/
/*									*/
/* Author: Brian Knight, 15th December 1989				*/
/************************************************************************/

/*
 * $Log:	codecHigh.c,v $
 * Revision 1.1  91/02/14  23:01:34  paul
 * Initial revision
 * 
 * Revision 1.1  90/08/06  12:58:02  brian
 * Initial revision
 * 
 */

#include <dev/result.h>
#include <string.h>
#include <device.h>
#include <time.h>
/*#include <codecHigh.h>*/
#include "codecHigh.h"
/*#include <codec.h>*/
#include "codec.h"

#ifdef TRACING
#include <dev/trace.h>
#endif

/*----------------------------------------------------------------------*/

#define DEVNAME "/files/helios/lib/codec.dev" /* Path name of codec driver */

#define MAXRINGGAP 2 /* Longest gap in phone ringing (seconds) */

/*----------------------------------------------------------------------*/

typedef struct StreamInfo
{
  CodecHigh_DevHandle handle;  /* handle issued for this stream direction */
  CodecHigh_Mode      mode;    /* input or output stream                  */
  int		      timeout; /* for CodecHigh_Wait (seconds: -ve => never) */
  CodecHigh_Switch    start;   /* specifies start condition               */
  CodecHigh_Switch    end;     /* specifies end condition                 */
} StreamInfo, *RefStreamInfo;


typedef struct DevInfo
{
  DCB        *dcb;      /* underlying device (0 if device not open) */
  StreamInfo inStream;  /* input stream (handle == 0 if not open)   */
  StreamInfo outStream; /* output stream (handle == 0 if not open)  */
} DevInfo, *RefDevInfo;

/*----------------------------------------------------------------------*/
/* Static variables							*/
/*----------------------------------------------------------------------*/

/* This array holds the information on each codec device currently held */
/* open by this invocation of this library. This works best if there is	*/
/* only one such invocation.						*/

static DevInfo devInfoTable[MAXCODECDEVICES];

static int handleSeq = 0; /* Used to make unique CodecHigh_DevHandles */

/*----------------------------------------------------------------------*/
/* Function prototypes							*/
/*----------------------------------------------------------------------*/

static Result CheckHandle(CodecHigh_DevHandle handle, RefDevInfo *devInfo,
			  RefStreamInfo *streamInfo);
static void DummyAction(DCB *dcb, CodecReq *req);
static CodecHigh_DevHandle NewHandle(int devNo, CodecHigh_Mode mode);
static Result DeviceOp(DCB *dcb, word func);

/*----------------------------------------------------------------------*/
/* Stream and buffer management						*/
/*----------------------------------------------------------------------*/

/*----------------------------------------------------------------------*/
/* Open a codec device for input or output.				*/
/* `deviceName' should be "codec0", "codec1", etc. to specify the       */
/* logical device number.						*/
/* `sampleRate' is ignored at present as there is no hardware support	*/
/* for it on the functional prototype.					*/
/*----------------------------------------------------------------------*/
CodecHigh_DevHandle 
CodecHigh_Open			/* -ve Result returned for error */
(
  char            *deviceName,	/* name of device to open */
  CodecHigh_Mode  mode,		/* input or output */
  int             sampleRate,	/* input and output rates must match */
  CodecHigh_Route route		/* signal source/destination */
)
{
  Result              rc;
  int                 logDevNum;
  RefDevInfo          info;
  CodecHigh_DevHandle handle;
  int		      devOpened = 0; /* Set if device opened this time */
  int		      routeFunc;     /* Function code for audio route  */
  
  sampleRate = sampleRate; /* Silence compiler warnings */

  /* Decode the supplied name */
  rc = CODECHIGH_GERRORPARAMETER; /* Assume faulty until proved otherwise */

  if (strlen(deviceName) == 6)
  {
    char nameCopy[7];

    strcpy(nameCopy, deviceName);
    nameCopy[5] = 0; /* Remove the digit */
    if (strcmp(nameCopy, "codec") == 0)
    {
      logDevNum = deviceName[5] - '0'; /* Get logical device number */
      if ((logDevNum >= 0) && (logDevNum < MAXCODECDEVICES))
        rc = 0; /* Name ok */
    }
  }

  if (rc) goto OpenExit;

  /* Check that the route parameter matches the stream direction and	*/
  /* determine which function code to send to the driver.		*/

  routeFunc = -1; /* Invalid value */
  if (mode == CodecHigh_ModeInput)
  {
    if      (route == CodecHigh_RouteLine) routeFunc = FC_PhoneInput;
    else if (route == CodecHigh_RouteMike) routeFunc = FC_MicInput;
  }
  else
  {
    if      (route == CodecHigh_RouteLine)    routeFunc = FC_PhoneOutput;
    else if (route == CodecHigh_RouteSpeaker) routeFunc = FC_SpeakerOutput;
  }

  if (routeFunc == -1) 
    { rc = CODECHIGH_GERRORPARAMETER; goto OpenExit; }

  /* The device driver can support only one (bidirectional) open for	*/
  /* each physical device, so we have to detect here if we already have	*/
  /* the other direction open.						*/

  info = &devInfoTable[logDevNum];
  if (info->dcb) /* Is this device driver already open? */
  {
    if (((mode == CodecHigh_ModeInput)  && (info->inStream.handle)) ||
        ((mode == CodecHigh_ModeOutput) && (info->outStream.handle)))
      { rc = CODECHIGH_GERRORINUSE; goto OpenExit; }
  }
  else /* Device driver not already open */
  {
    CodecDevInfo codecInfo;

    codecInfo.logDevNum = logDevNum;
    info->dcb = OpenDevice(DEVNAME, &codecInfo);
    if (info->dcb == 0)
      { rc = CODECHIGH_SERRORNODEVICE; goto OpenExit; }

    info->inStream.handle  = 0;
    info->outStream.handle = 0;
    devOpened = 1; /* Device opened in this call */
  }

  /* Set the requested route for the audio signal. */

  rc = DeviceOp(info->dcb, routeFunc);
  if (rc)
  {
    if (devOpened)	/* Close device if we just opened it */
    {
      CloseDevice(info->dcb);
      info->dcb = 0;
    }

    goto OpenExit;
  }

  /* Make a unique handle for this device stream and return it. */

  handle = NewHandle(logDevNum, mode);
  if (mode == CodecHigh_ModeOutput)
  {
    info->outStream.handle = handle;
    info->outStream.mode   = CodecHigh_ModeOutput;
  }
  else
  {
    info->inStream.handle  = handle;
    info->inStream.mode    = CodecHigh_ModeInput;
  }

  rc = handle;    

OpenExit:
  return rc;
}


/*----------------------------------------------------------------------*/
/* Close a device opened by CodecHigh_Open. The underlying device       */
/* driver is shut down only if both input and output are closed.	*/
/*----------------------------------------------------------------------*/
Result 
CodecHigh_Close(CodecHigh_DevHandle devHandle)
{
  RefDevInfo    devInfo;
  RefStreamInfo streamInfo;
  Result        rc;

  rc = CheckHandle(devHandle, &devInfo, &streamInfo);
  if (rc == 0)
  {
    if (streamInfo->mode == CodecHigh_ModeOutput)
      devInfo->outStream.handle = 0;
    else
      devInfo->inStream.handle = 0;

    if ((devInfo->inStream.handle == 0) && (devInfo->outStream.handle == 0))
    {
      CloseDevice(devInfo->dcb);
      devInfo->dcb = 0;
    }
  }

  return rc;
}

/*----------------------------------------------------------------------*/
/* Grab the input/output signal lines (e.g. from automatic speech	*/
/* recognition and indicate the stimuli to start and end sampling.	*/
/*----------------------------------------------------------------------*/
Result 
CodecHigh_Begin
(
  CodecHigh_DevHandle devHandle,/* as returned by CodecHigh_Open() */
  CodecHigh_Switch start,	/* specifies start condition */
  CodecHigh_Switch end,		/* specifies end condition   */
  int timeout			/* timeout in seconds (-ve => never) for */
                                /* calls to CodecHigh_Wait().		 */
)
{
  RefDevInfo     devInfo;
  RefStreamInfo  streamInfo;
  Result         rc = CheckHandle(devHandle, &devInfo, &streamInfo);

  /* No PTT button support on codec podule */
  if ((rc == 0) &&
      ((start != CodecHigh_SwitchQueue) || (end != CodecHigh_SwitchQueue)))
    rc = CODECHIGH_GERRORPARAMETER;

  if (rc == 0)
  {
    CodecReq req;

    streamInfo->start   = start;
    streamInfo->end     = end;
    streamInfo->timeout = timeout;
    req.DevReq.Request  = (streamInfo->mode == CodecHigh_ModeInput ? 
                          FC_EnableRead : FC_EnableWrite);
    req.DevReq.Action   = DummyAction; /* we know the driver is synchronous */

    Operate(devInfo->dcb, &req);
    rc = (Result)req.DevReq.Result;
  }
 
  return rc;
}

/*----------------------------------------------------------------------*/
/* Queue a buffer for IO transfer. The result is negative if an error   */
/* occurs, otherwise is a buffer handle.				*/
/*----------------------------------------------------------------------*/
CodecHigh_BufHandle 
CodecHigh_Queue	/* queues a buffer for IO transfer */
(
  CodecHigh_DevHandle devHandle,/* as returned by CodecHigh_Open() */
  void *bufferStart,		/* pointer to buffer */
  int bufferLength		/* length of buffer in bytes */
)
{
  RefDevInfo     devInfo;
  RefStreamInfo  streamInfo;
  Result         rc = CheckHandle(devHandle, &devInfo, &streamInfo);
  
  if (rc == 0)
  {
    CodecReq req;

    req.DevReq.Request = (streamInfo->mode == CodecHigh_ModeInput ? 
                          FC_QueueReadBuf : FC_QueueWriteBuf);
    req.DevReq.Action  = DummyAction; /* we know the driver is synchronous */
    req.Buf	       = bufferStart;
    req.Size	       = bufferLength;

    Operate(devInfo->dcb, &req);

    rc = (Result)req.DevReq.Result;
    /* req.Actual irrelevant here */
  }
 
  return rc;
}

/*----------------------------------------------------------------------*/
/* Return the current status of a specified buffer.			*/
/*----------------------------------------------------------------------*/
Result 
CodecHigh_Enquire
(
  CodecHigh_DevHandle devHandle, /* as returned by CodecHigh_Open() */
  CodecHigh_BufHandle bufHandle, /* as returned by CodecHigh_Queue() */
  int *flag			 /* where to put status: !=0 => buffer ready */
)				 /* 0 => buffer still busy or not started    */
{
  RefDevInfo     devInfo;
  RefStreamInfo  streamInfo;
  Result         rc = CheckHandle(devHandle, &devInfo, &streamInfo);
  
  if (rc == 0)
  {
    CodecReq req;

    req.DevReq.Request = FC_Enquire;
    req.DevReq.Action  = DummyAction; /* we know the driver is synchronous */
    req.Buf	       = (void *)bufHandle;

    Operate(devInfo->dcb, &req);

    rc = (Result)req.DevReq.Result;
    if (rc >= 0)
      *flag = (int)req.Actual;
  }

  return rc;
}

/*----------------------------------------------------------------------*/
/* Inform the driver that the immediately previous buffer queued is the */
/* final buffer and that it should terminate I/O at the end of that     */
/* buffer, if that end condition was set by CodecHigh_Begin.		*/
/*----------------------------------------------------------------------*/
Result 
CodecHigh_Finish
(
  CodecHigh_DevHandle devHandle, /* as returned by CodecHigh_Open() */
  CodecHigh_BufHandle *bufHandle /* to receive last buffer handle   */
)
{
  RefDevInfo     devInfo;
  RefStreamInfo  streamInfo;
  Result         rc = CheckHandle(devHandle, &devInfo, &streamInfo);
  
  if (rc == 0)
  {
    CodecReq req;

    req.DevReq.Request = (streamInfo->mode == CodecHigh_ModeInput ? 
                          FC_FinishRead : FC_FinishWrite);
    req.DevReq.Action  = DummyAction; /* we know the driver is synchronous */

    Operate(devInfo->dcb, &req);

    if (req.DevReq.Result >= 0)
      *bufHandle = (CodecHigh_BufHandle)req.DevReq.Result;
    else
      rc = (Result)req.DevReq.Result;
  }
 
  return rc;
}

/*----------------------------------------------------------------------*/
/* Read status of device stream.					*/
/*----------------------------------------------------------------------*/
Result 
CodecHigh_ReadStatus
(
  CodecHigh_DevHandle devHandle, /* as returned by CodecHigh_Open() */
  CodecHigh_Status    *status    /* receives current status	    */
)
{
  RefDevInfo       devInfo;
  RefStreamInfo    streamInfo;
  Result           rc = CheckHandle(devHandle, &devInfo, &streamInfo);
  
  if (rc == 0)
  {
    CodecReq req;

    req.DevReq.Request = 
      (streamInfo->mode == CodecHigh_ModeInput ? FC_ReadInputStatus : 
                                                 FC_ReadOutputStatus);
    req.DevReq.Action  = DummyAction; /* we know the driver is synchronous */
    Operate(devInfo->dcb, &req);
    rc = (Result)req.DevReq.Result;

    if (rc >= 0)
    { /* Copy driver status info to library status structure */
      Codec_Status driverStatus;

      *(word *)&driverStatus = req.Actual; 
      status->state       = driverStatus.state; /* Should translate really */
      status->bufsUnready = driverStatus.bufsUnready;
      status->bufsReady   = driverStatus.bufsReady;
      status->errorFlag   = driverStatus.errorFlag;
    }
  }
 
  return rc;
}

/*----------------------------------------------------------------------*/

static int wCount = 0; /* debugging */

Result 
CodecHigh_Wait		/* waits for a queued buffer */
(
  CodecHigh_DevHandle devHandle,  /* as returned by CodecHigh_Open() */
  CodecHigh_BufHandle bufHandle,  /* as returned by CodecHigh_queue() */
  int *transferred		  /* actual number of bytes transferred */
)
{
  RefDevInfo     devInfo;
  RefStreamInfo  streamInfo;
  Result         rc = CheckHandle(devHandle, &devInfo, &streamInfo);
  
  if (rc == 0)
  {
    CodecReq req;

    req.DevReq.Request = FC_BufWait;
    req.DevReq.Action  = DummyAction; /* we know the driver is synchronous */
    req.Buf	       = (void *)bufHandle;

    *(int *)0x740500 = wCount;   /* Clear value from previous run */
    *(int *)0x740400 = ++wCount;
    Operate(devInfo->dcb, &req);
    *(int *)0x740500 = wCount;

    rc = (Result)req.DevReq.Result;
    *transferred = (int)req.Actual;
  }
 
  return rc;
}

/*----------------------------------------------------------------------*/
/* abort - Immediately aborts current I/O on that stream and releases   */
/* all buffers. A call to codecHigh_begin is needed before more I/O can */
/* be done.								*/
/*----------------------------------------------------------------------*/
Result 
CodecHigh_Abort
(
  CodecHigh_DevHandle devHandle /* as returned by CodecHigh_Open() */
)
{
  RefDevInfo     devInfo;
  RefStreamInfo  streamInfo;
  Result         rc = CheckHandle(devHandle, &devInfo, &streamInfo);
  
  if (rc == 0)
  {
    CodecReq req;

    req.DevReq.Request = (streamInfo->mode == CodecHigh_ModeInput ? 
                          FC_AbortRead : FC_AbortWrite);
    req.DevReq.Action  = DummyAction; /* we know the driver is synchronous */

    Operate(devInfo->dcb, &req);

    rc = (Result)req.DevReq.Result;
  }
 
  return rc;
}

/*----------------------------------------------------------------------*/
/* Telephone Line Handling						*/
/*----------------------------------------------------------------------*/

/* dial - Dials the given telephone number and waits till answered or timeout
 * (i.e. goes offhook, waits for dial tone, dials digits, waits for ringing
 * tone, waits for end of ringing tone).
 */
Result 
CodecHigh_Dial
(
  CodecHigh_DevHandle devHandle, /* as returned by CodecHigh_Open() */
  CodecHigh_DialMode dialMode,	 /* DTMF or pulse code */
  char *phoneString,		 /* ASCII string of number to dial */
  int *flag			 /* where to return 0 <=> answered ok */
)				 /*  otherwise <>0 <=> timeout */
{
  RefDevInfo     devInfo;
  RefStreamInfo  streamInfo;
  Result         rc = CheckHandle(devHandle, &devInfo, &streamInfo);
  
  if (rc == 0)
  {
    if (dialMode == CodecHigh_DialPulseCode)
    {
      CodecReq req;

      req.DevReq.Request = FC_PulseDial;
      req.DevReq.Action  = DummyAction;
      req.Buf            = phoneString;

      Operate(devInfo->dcb, &req);

      rc = (Result)req.DevReq.Result;
    }
    else
      rc = CODECHIGH_SERRORNOTIMPLEMENTED; /* Tone dialling TBD */
  }
 
  *flag = rc; /* Detection of answering TBD */
  return rc;
}

/*----------------------------------------------------------------------*/
/* Return number of seconds which have passed since given value		*/
/* from previous call of time().					*/
/* This is not very accurate for small time differences as the clock    */
/* used ticks only once per second.					*/
/*----------------------------------------------------------------------*/
static unsigned int
TimeSince(unsigned int secTime)
{
  return time(0) - secTime;
}

/*----------------------------------------------------------------------*/
/* answer - Auto-answers the telephone when it rings and waits
 * until called or timeout (i.e. waits for ringing tone, waits for enough
 * ringing, goes offhook).
 */
Result 
CodecHigh_Answer
(
  CodecHigh_DevHandle devHandle, /* as returned by CodecHigh_Open() */
  int ringTime,			 /* ring time (in seconds) before answering */
  int timeout,			 /* timeout in seconds */
  int *flag			 /* where to return 0 <=> answered ok */
)				 /*  otherwise <>0 <=> timeout */
{
  RefDevInfo     devInfo;
  RefStreamInfo  streamInfo;
  Result         rc = CheckHandle(devHandle, &devInfo, &streamInfo);
  
  if (rc == 0)
  {
    CodecReq req;
    unsigned int entryTime = time(0);
    int          ringingLongEnough = 0;

    req.DevReq.Request = FC_OnHook;
    req.DevReq.Action  = DummyAction;
    Operate(devInfo->dcb, &req);
    rc = (Result)req.DevReq.Result; if (rc) goto AnswerExit;

    req.DevReq.Request = FC_ReadRawStatus;
    while (!ringingLongEnough)
    {
      unsigned int ringStartTime;
      unsigned int lastRingTime;

      for (;;)
      {
        Delay(OneSec/5); /* Poll the ringing indicator a few times a second */
        Operate(devInfo->dcb, &req);
        if ((req.DevReq.Result & CODEC_STATUSNRI) == 0) break; /* Ringing */
        if ((timeout >= 0) && (TimeSince(entryTime) >= timeout))
	  { rc = CODECHIGH_SERRORTIMEOUT; goto AnswerExit; }
      }

      /* Now wait for the phone to ring for the required period */
      /* and loop back if it stops ringing early.		*/
      /* This is a bit fiddly because the ring indicator goes 	*/
      /* on and off during ringing, so we mustn't give up until	*/
      /* it has not been seen for a while.			*/
      ringStartTime = time(0);
      lastRingTime  = ringStartTime;
      for (;;)
      {
        Delay(OneSec/5); /* Poll the ringing indicator a few times a second */
        Operate(devInfo->dcb, &req);
        if ((req.DevReq.Result & CODEC_STATUSNRI) == 0)
	  lastRingTime = time(0);
	else
	  if (TimeSince(lastRingTime) > MAXRINGGAP)
	    break; /* Go back and wait for ringing to restart */

	if (TimeSince(ringStartTime) >= ringTime)
	  { ringingLongEnough = 1; break; } /* Can answer it now */
        if ((timeout >= 0) && (TimeSince(entryTime) >= timeout))
	  { rc = CODECHIGH_SERRORTIMEOUT; goto AnswerExit; }
      }
    }

    req.DevReq.Request = FC_OffHook;
    Operate(devInfo->dcb, &req);
    rc = (Result)req.DevReq.Result;
  }
 
AnswerExit:
  *flag = rc;
  return rc;
}

/*----------------------------------------------------------------------*/
/* Hang up the phone line */
Result CodecHigh_HangUp
(
  CodecHigh_DevHandle devHandle	/* as returned by CodecHigh_Open() */
)
{
  RefDevInfo     devInfo;
  RefStreamInfo  streamInfo;
  Result         rc = CheckHandle(devHandle, &devInfo, &streamInfo);
  
  if (rc == 0)
  {
    CodecReq req;

    req.DevReq.Request = FC_OnHook;
    req.DevReq.Action  = DummyAction;

    Operate(devInfo->dcb, &req);

    rc = (Result)req.DevReq.Result;
  }
 
  return rc;
}

/*----------------------------------------------------------------------*/
/* Set variable gain block */
Result 
CodecHigh_SetGain
(
  CodecHigh_DevHandle devHandle,	/* as returned by CodecHigh_Open() */
  CodecHigh_Gain newGain,	/* new VGB setting */
  CodecHigh_Gain *oldGain	/* where to put old VGB setting */
)
{
  devHandle = devHandle; newGain = newGain; oldGain = oldGain;
  return CODECHIGH_SERRORNOTIMPLEMENTED;
}


#ifdef TRACING
void CodecHigh_PrintTrace(CodecHigh_DevHandle devHandle)
{
  RefDevInfo     devInfo;
  RefStreamInfo  streamInfo;
  Result         rc = CheckHandle(devHandle, &devInfo, &streamInfo);
  
  if (rc == 0)
  {
    CodecReq req;

    req.DevReq.Request = FC_GetTraceBuf;
    req.DevReq.Action  = DummyAction; /* we know the driver is synchronous */
    Operate(devInfo->dcb, &req);

    if (req.DevReq.Result < 0)
      printf("failed to get trace buffer\n");
    else
    {
      TraceBuf     *tb = (TraceBuf *)req.DevReq.Result;
      int          slot = (tb->wrapped ? tb->next : 0);
      unsigned int firstTime = tb->buf[slot].time;

      if (tb->next == 0 && !tb->wrapped)
	{ printf("no entries\n"); return; }

      do
      {
        TraceEntry *entry = &tb->buf[slot];
	printf("%5d: ", ((entry->time - firstTime) >> 10));
	switch (entry->event)
	{
          case TR_REQUEST:
	    printf("request: "); 
	    switch (entry->value)
	    {
  	      case FC_QueueReadBuf:
	        printf("queue read buf"); break;
	      case FC_QueueWriteBuf:
	        printf("queue write buf"); break;
	      case FC_BufWait:
	        printf("bufwait"); break;
	      case FC_GetTraceBuf:
	        printf("get trace buffer"); break;

	      default: 
	        printf("%x", entry->value); break;
	    }
	    break;

          case TR_READINT:
	    printf("read int %d", entry->value); break;
          case TR_WRITEINT:
	    printf("write int %d", entry->value); break;
          case TR_STARTWAIT:
	    printf("start wait %d", entry->value); break;
          case TR_ENDWAIT:
	    printf("end wait %d", entry->value); break;
          case TR_QREADBUF:  
	    printf("q read buf %d", entry->value); break;
          case TR_QWRITEBUF: 
	    printf("q write buf %d", entry->value); break;
          case TR_OVERRUN:   
	    printf("overrun %d", entry->value); break;
          case TR_UNDERRUN:  
	    printf("underrun %d", entry->value); break;
          case TR_READSIG:   
	    printf("read signal %d", entry->value); break;
          case TR_READNEXT:  
	    printf("read next buf %d", entry->value); break;
          case TR_READEXIT:  
	    printf("read int exit %d", entry->value); break;
          case TR_WRITESIG:  
	    printf("write signal %d", entry->value); break;
          case TR_WRITENEXT: 
	    printf("write next buf %d", entry->value); break;
          case TR_WRITEEXIT: 
	    printf("write int exit %d", entry->value); break;
          case TR_REN: 
	    printf("enable rx %d", entry->value); break;
          case TR_STARTWRITE: 
	    printf("start write %d", entry->value); break;

          default: 
	    printf("%3d, %x", entry->event, entry->value);
	    break;
	}

	printf("\n");
	if (++slot >= tb->size) slot = 0;
      } while (slot != tb->next);
    }
  }
}
#endif /* TRACING */

/*----------------------------------------------------------------------*/
/* Handles: these contain the logical device number, an input/output    */
/* flag, and a unique number to guard against use of stale handles.     */
/* The top bit must be zero to avoid confusion with errors, and another	*/
/* bit is always set so that a zero handle is invalid.			*/
/*									*/
/*                 01SSSSSSSSSSSSSSSSSSSSSSNNNNNNND	       		*/
/*									*/
/*  S: serial number   N: device number   D: direction (1 for output)	*/
/*  									*/
/*----------------------------------------------------------------------*/

static CodecHigh_DevHandle
NewHandle(int devNo, CodecHigh_Mode mode)
{
  return 0x40000000 |
         ((++handleSeq << 8) & 0x3FFFFF00) |
         (devNo << 1) |
         (mode == CodecHigh_ModeOutput ? 1 : 0);
}

/* Result is 0 for valid handle, error code otherwise */
static Result
CheckHandle
(
  CodecHigh_DevHandle handle, 
  RefDevInfo          *devInfo    /*out*/,
  RefStreamInfo	      *streamInfo /*out*/
)
{
  int    devNum = (handle >> 1) & 0x7F;
  int	 output = handle & 1;
  Result rc = CODECHIGH_SERRORDEVHANDLE;

  if (devNum < MAXCODECDEVICES)
  {
    RefDevInfo    di = &devInfoTable[devNum];
    RefStreamInfo stream = (output ? &di->outStream : &di->inStream);

    if (stream->handle == handle)
    {
      *devInfo    = di;
      *streamInfo = stream;
      rc          = 0;
    }
  }

  return rc;
}


/*----------------------------------------------------------------------*/
/* Perform the specified device operation and return its result.	*/
/*----------------------------------------------------------------------*/

static Result 
DeviceOp(DCB *dcb, word func)
{
  CodecReq req;

  req.DevReq.Request = func;
  req.DevReq.Action  = DummyAction; /* we know the driver is synchronous */
  Operate(dcb, &req);
  return (Result)req.DevReq.Result;
}

/*----------------------------------------------------------------------*/
/* All calls to the codec device driver are synchronous, so no use is   */
/* made of the callback to the Action routine.                          */
/*----------------------------------------------------------------------*/

void DummyAction(DCB *dcb, CodecReq *req)
{
  dcb = dcb; req = req; /* To keep the compiler quiet */
}

/*----------------------------------------------------------------------*/

/* End of codecHigh.c */

