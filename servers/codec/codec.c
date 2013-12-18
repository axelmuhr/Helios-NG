/* $Header: /hsrc/servers/codec/RCS/codec.c,v 1.2 1991/10/09 11:19:51 paul Exp $ */
/* $Source: /hsrc/servers/codec/RCS/codec.c,v $ */
/************************************************************************/ 
/* codec.c - ARM Helios device driver for Codec device on AB		*/
/* 	       functional prototype board.				*/
/*									*/
/* Copyright (C) 1989 Active Book Company Ltd., Cambridge, England	*/
/*									*/
/* Author: Brian Knight, 30th October 1989				*/
/************************************************************************/


/************************************************************************/
/* This is an asynchronous driver in the sense that it holds		*/
/* lists of client buffers and performs input and output without	*/
/* blocking its caller. However, all of the calls to the driver are	*/
/* synchronous - i.e. Action will be called back before DevOperate	*/
/* returns, so can be a dummy routine.					*/
/* The driver has no buffering of its own (so avoids copying the data).	*/ 
/* However, this means that its clients must keep it supplied with	*/
/* buffers to avoid overrun or underrun. (The driver should never be	*/
/* without buffers for more than the time taken to half fill/empty the	*/
/* fifo - i.e. about 1/16th second (512 bytes @ 8000 bytes/sec).)	*/
/************************************************************************/

/*
 * $Log: codec.c,v $
 * Revision 1.2  1991/10/09  11:19:51  paul
 * changed fproto.h include path
 *
 * Revision 1.1  1991/02/14  23:00:44  paul
 * Initial revision
 *
 * Revision 1.1  90/08/06  12:57:33  brian
 * Initial revision
 * 
 * Revision 1.4  90/03/22  16:07:00  brian
 * VerifyCodecPodule() now looks at the `not present' bit for each slot.
 * 
 * Revision 1.3  90/02/06  10:59:55  brian
 * First released version.
 * 
 * Revision 1.2  90/01/28  12:06:18  brian
 * Saved before experimenting with enabling reception late.
 * 
 */


/* Fool helios.h into defining Code */
#define in_kernel
#include <helios.h>
#undef in_kernel
#include <syslib.h>
#include <device.h>
#include <codes.h>
#include <root.h>
#include <event.h>
#include <stdio.h>
#include <abcARM/fproto.h>

#include "codec.h"

#ifdef TRACING
#include <dev/trace.h>
#endif

/*----------------------------------------------------------------------*/

/* Driver configuration */

/* #define CODECDEBUG   0 */    /* Turn off debugging messages */
#define CODECIRQ     42   /* Magic number for Jamie */

#define BUFQUEUEMAX  16	  /* Limit on no. of read or write buffers queued */
#define FIFOSIZE     1023
#define HALFFIFOSIZE 512  /* Point at which interrupt occurs */
#define MAXPHONENUMBERDIGITS 24 /* No real phone number longer than this */
#define WRITETHRESHOLD HALFFIFOSIZE /* Enable rx when this much in fifo */

/* Memory-mapped register addresses */
#define DATAREGADDR	0x00400060	/* Address of codec data register */
#define CSREGADDR	0x00400071	/* Address of control/status reg  */

/* Pulse dial timings (microseconds) */
#define DIALTONEDELAY	1000000	/* Time to wait for dial tone to appear */
#define DIALPULSELENGTH	  66000	/* Length of each pulse			*/
#define DIALPULSEGAP	  33000	/* Gap between pulses			*/
#define INTERDIGITGAP	 840000	/* Gap between digits			*/

#ifdef TRACING
# define TRACE_ENTRIES	1000
# define TRACE(event, value) Trace(&dcb->traceBuf, (event), (value))
#else
# define TRACE(event, value)
#endif

/*----------------------------------------------------------------------*/

typedef volatile unsigned char vu_char;

/* Structure which maps onto the memory-mapped registers of the codec	*/
/* chip on the functional prototype. Char fields are used to encourage	*/
/* the compiler to generate byte memory accesses.                       */

typedef struct CodecRegs
{
  vu_char data;		  /* receive/transmit data register */
  vu_char _pad[CSREGADDR - DATAREGADDR - 1];
  vu_char controlStatus;  /* control/status register        */
} CodecRegs, *RefCodecRegs;

/* Bit positions in the memory-mapped device registers */

/* control register */
#define CODEC_CONTROLLP   0x01	/* 0 = Pulse dialling enabled		*/
#define CODEC_CONTROLOFFH 0x02	/* 1 = Off Hook				*/
#define CODEC_CONTROLTNRS 0x04	/* 0 = Transmit FIFO reset		*/
#define CODEC_CONTROLRNRS 0x08	/* 0 = Receiver FIFO reset		*/
#define CODEC_CONTROLTEN  0x10	/* 1 = Enable transmitter to FIFO transfer */
#define CODEC_CONTROLREN  0x20	/* 1 = Enable FIFO to receiver transfer */
#define CODEC_CONTROLMIC  0x40  /* 0 = phone i/p; 1 = microphone i/p	*/
#define CODEC_CONTROLSPKR 0x80  /* 0 = phone o/p; 1 = speaker o/p	*/

/* status register bits defined in codec.h */

/* Buffer queues							*/
/* These structures are used to hold the read and write buffers         */
/* supplied by clients. The current implementation uses a fixed-size    */
/* array as a circular buffer, leading to a static upper limit on the	*/
/* number of buffers which may be held.					*/

typedef struct Buffer
{
  unsigned char *addr;	  /* buffer address				*/
  unsigned int	size:24,  /* size of buffer				*/
                key:8;    /* used to avoid repeating handles too often  */
  unsigned int  xfrd:24,  /* bytes transferred				*/
                signal:1, /* set if int routine should signal top half	*/
                padding:1,/* set if is dummy buffer for write padding	*/
                :6;	
} Buffer;

/* A BufferQueue holds a list of buffers in a (circular) array.		*/
/* Buffers from `firstReady' up to (but not including) `current' have   */
/* already been filled or emptied. `firstReady' == `current' if none.	*/
/* Buffers from `current' up to (but not including) `firstFree' are	*/
/* waiting for I/O. `current' == `firstFree' if none are waiting.	*/
/* If `firstFree' points at the slot before `firstReady', then the 	*/
/* circular array is full.						*/

typedef int BufQueueState;

typedef struct BufferQueue
{
  int firstReady;	/* index of first ready (filled/emptied) buffer */
  int current;		/* index of current (i.e. first waiting) buffer	*/
  			/* `current' is updated by interrupt routines.	*/
  int firstFree;	/* index of first free element (i.e. one after  */
                        /* last waiting buffer)				*/
  BufQueueState state;	/* state of I/O using this buffer queue		*/
  int timeout;		/* in seconds for buffer waits (-ve for never)	*/
  Semaphore waitSema;	/* used to wait for a buffer to fill or empty	*/
  int transferred;	/* total bytes read or written so far		*/
  Buffer queue[BUFQUEUEMAX]; /* array to hold the queue			*/
} BufferQueue;


/* Device Control Block 					   */
/* Volatile is used for those fields which refer to memory-mapped  */
/* device registers, and those which are altered in interrupt      */
/* routines.							   */

typedef struct CodecDCB
{
  DCB                    dcb;	       /* Standard DCB                    */
  volatile RefCodecRegs  codecRegs;    /* Memory mapped Codec registers   */
  Semaphore              codecLock;    /* For serialising access to Codec */
  Event                  intHandler;   /* Event structure for int handler */
  int			 logDev;       /* logical device number           */
  int			 controlCopy;  /* soft copy of control register   */
  BufferQueue		 readQueue;    /* q of input (transmit) buffers   */
  BufferQueue		 writeQueue;   /* q of output (receive) buffers	  */
  volatile unsigned char rflags;       /* flags read irq handler can change */
  volatile unsigned char wflags;       /* flags write irq handler can change*/
#ifdef TRACING
  TraceBuf		 traceBuf;     /* tracing buffer		  */
#endif
} CodecDCB;

/* Bits in dcb rflags and wflags, which may be modified by IRQ handlers */

#define CRF_READSLOW    0x01	/* Read too slow, buffer has overflowed    */
#define CRF_FIRSTREAD   0x02    /* For ignoring overflow before first read */

#define CWF_WRITESLOW   0x01    /* Write too slow, buffer has underflowed  */
#define CWF_FIRSTWRITE  0x02    /* For ignoring underflow before 1st write */

/*----------------------------------------------------------------------*/

/* Forward references */

int  CodecInt(CodecDCB *dcb);
void DevOperate(CodecDCB *dcb, CodecReq *req);
word DevClose(CodecDCB *dcb);
static int EnableRead(CodecDCB *dcb);
static int StartRead(CodecDCB *dcb);
static int AbortRead(CodecDCB *dcb);
static int EnableWrite(CodecDCB *dcb);
static int StartWrite(CodecDCB *dcb);
static int AbortWrite(CodecDCB *dcb);
static int ValidBuffer(BufferQueue *qp, int slot);
static void CodecReadInt(CodecDCB *dcb);
static void CodecWriteInt(CodecDCB *dcb);
static void SetControlBits(CodecDCB *dcb, unsigned char bits);
static void ClearControlBits(CodecDCB *dcb, unsigned char bits);
static void InitBufQueue(BufferQueue *qp);
static void PulseDialDigit(CodecDCB *dcb, int digit);
static Result QueueBuffer(CodecDCB *dcb, int write, CodecReq *req);
static Result MarkFinalBuffer(CodecDCB *dcb, int write,
                              Codec_BufHandle *handle /*out*/);
static Result WaitForBuffer(CodecDCB *dcb, CodecReq *req);
static Result BufEnquire(CodecDCB *dcb, CodecReq *req);
static Result PulseDial(CodecDCB *dcb, CodecReq *req);
static Result CheckBufferHandle(CodecDCB *dcb, Codec_BufHandle handle,
				int *write, int	*slot, Buffer **buf);
static Codec_Status ReadStatus(CodecDCB *dcb, int write);
static Codec_BufHandle NewBufferHandle(CodecDCB *dcb, int write, int slot);
static Codec_BufHandle BufferHandleForSlot(CodecDCB *dcb, int write, int slot);

/*----------------------------------------------------------------------*/

CodecDCB *DevOpen(Device *dev, CodecDevInfo *info)
{
  CodecDCB  *dcb;
  CodecRegs *regs;

  /* The `info' argument tells us which logical Codec device to open.   */
  /* There is only one on the functional prototype board (and this	*/
  /* driver does not support podules).					*/

  if (info->logDevNum != 0) return NULL; /* No such codec device */
  regs = (RefCodecRegs)DATAREGADDR;

  dcb = Malloc(sizeof(CodecDCB));
  if (dcb == NULL) return NULL;
  
  dcb->dcb.Device  = dev;
  dcb->dcb.Operate = DevOperate;
  dcb->dcb.Close   = DevClose;
  dcb->codecRegs   = regs;
  InitSemaphore(&dcb->codecLock, 1);
  
  /* Set up the interrupt routine */
 
  dcb->intHandler.Pri  = CODECIRQ;
  dcb->intHandler.Code = (WordFnPtr)CodecInt;
  dcb->intHandler.Data = dcb;    /* Pass DCB address to int routine */
  SetEvent(&dcb->intHandler); 

  dcb->rflags = CRF_FIRSTREAD;
  dcb->wflags = CWF_FIRSTWRITE;
  dcb->logDev = info->logDevNum;

  /* Set the control register to a known value and clear both fifos */
  dcb->controlCopy = 0;
  SetControlBits(dcb, CODEC_CONTROLLP | CODEC_CONTROLSPKR | CODEC_CONTROLMIC);
                      /* Not pulse dialling; use loudspeaker & microphone */
  /* Finished resetting the fifos */
  SetControlBits(dcb, CODEC_CONTROLRNRS | CODEC_CONTROLTNRS);
  DisableIRQ(INT_CODECRX | INT_CODECTX); /* Mask out both interrupts */

  /* Initialise the buffer queues */
  InitBufQueue(&dcb->readQueue);
  InitBufQueue(&dcb->writeQueue);

#ifdef TRACING
  InitTraceBuf(&dcb->traceBuf, TRACE_ENTRIES);
#endif

  return dcb;
}

/*----------------------------------------------------------------------*/

word DevClose(CodecDCB *dcb)
{
  BufferQueue *wq = &dcb->writeQueue;

  Wait(&dcb->codecLock);

#ifdef CODECDEBUG
  /* IOdebug("codec close: log %d", dcb->logDev); */
#endif /* CODECDEBUG */

  /* Wait for write to finish if the interrupt routine is still putting	*/
  /* out the dummy buffer of padding zeros. This works only if the	*/
  /* client has waited for his last buffer (and written at least half a */
  /* fifo's worth of data). In other cases, the device is simply closed */
  /* immediately.							*/

  while ((wq->state == Codec_StateEnding) && 
	 (wq->current != wq->firstFree) &&
         ((dcb->wflags & CWF_WRITESLOW) == 0))
  {
    HardenedWait(&wq->waitSema);
  }


  /* Set the control register to a sensible value (phone on hook, read	*/
  /* and write ints disabled) 						*/
  dcb->controlCopy = 0;
  SetControlBits(dcb, CODEC_CONTROLLP);
  DisableIRQ(INT_CODECRX | INT_CODECTX); /* Disable both interrupts */

  dcb->rflags        = 0;
  dcb->wflags	     = 0;

  /* IOdebug("calling RemEvent in DevClose"); */
  RemEvent(&dcb->intHandler);

#ifdef TRACING
  FreeTraceBuf(&dcb->traceBuf);
#endif

  Free(dcb);  
  /* IOdebug("returning from DevClose"); */
  return Err_Null;
}

/*----------------------------------------------------------------------*/

void DevOperate(CodecDCB *dcb, CodecReq *req)
{
  word            error = 0;
  Codec_BufHandle bufHandle;

  /* IOdebug("DevOperate: request %x", req->DevReq.Request); */

  TRACE(TR_REQUEST, (int)req->DevReq.Request);  
  switch (req->DevReq.Request)
  {
    case FC_QueueReadBuf:	
      /* Add another input buffer, and start data transfer if necessary */
      Wait(&dcb->codecLock);
      bufHandle = QueueBuffer(dcb, 0 /*not write*/, req);

      if ((int)bufHandle >= 0) /* result is handle or error */
      {
        /* If reading (transmission) is not enabled, then start up	*/
        /* the transmit side of the codec.				*/
        if ((dcb->controlCopy & CODEC_CONTROLTEN) == 0)
	  error = StartRead(dcb);
      }

      if (error == 0) error = bufHandle; /* Return handle if no error */
      Signal(&dcb->codecLock);
      break;
      

    case FC_EnableRead:
      /* Start another stream of buffers */
      Wait(&dcb->codecLock);
      error = EnableRead(dcb);
      Signal(&dcb->codecLock);
      break;
      
    case FC_FinishRead:	
      /* Mark the last queued buffer as being the final one */
      {
        Codec_BufHandle handle;

        Wait(&dcb->codecLock);
        error = MarkFinalBuffer(dcb, 0 /*read*/, &handle);
        Signal(&dcb->codecLock);
        break;
      }
      
    case FC_AbortRead:	
      /* Stop reading immediately, and release all the buffers */
      Wait(&dcb->codecLock);
      error = AbortRead(dcb);
      Signal(&dcb->codecLock);
      break;
      
    case FC_QueueWriteBuf:	
      /* Add another input buffer, and start data transfer if necessary */
      Wait(&dcb->codecLock);
      bufHandle = QueueBuffer(dcb, 1 /*write*/, req);

      if ((int)bufHandle >= 0) /* result is handle or error */
      {
        /* If writing (reception) is not enabled, then start up		*/
        /* the receive side of the codec.				*/
        if (dcb->writeQueue.state == Codec_StateBegun)
	  error = StartWrite(dcb);
      }

      if (error == 0) error = bufHandle; /* Return handle if no error */
      Signal(&dcb->codecLock);
      break;
      

    case FC_EnableWrite:
      /* Start another stream of buffers */
      Wait(&dcb->codecLock);
      error = EnableWrite(dcb);
      Signal(&dcb->codecLock);
      break;
      
    case FC_FinishWrite:	
      /* Mark the last queued buffer as being the final one */
      {
        Codec_BufHandle handle;

        Wait(&dcb->codecLock);
        error = MarkFinalBuffer(dcb, 1 /*write*/, &handle);
        Signal(&dcb->codecLock);
        break;
      }
      
    case FC_AbortWrite:	
      /* Stop writing immediately, and release all the buffers */
      Wait(&dcb->codecLock);
      error = AbortWrite(dcb);
      Signal(&dcb->codecLock);
      break;
      
    case FC_BufWait:
      Wait(&dcb->codecLock);
      error = WaitForBuffer(dcb, req);
      Signal(&dcb->codecLock);
      break;
      
    case FC_LPLow:
      Wait(&dcb->codecLock);
      ClearControlBits(dcb, CODEC_CONTROLLP);
      Signal(&dcb->codecLock);
      break;

    case FC_LPHigh:
      Wait(&dcb->codecLock);
      SetControlBits(dcb, CODEC_CONTROLLP);
      Signal(&dcb->codecLock);
      break;

    case FC_ReadRawStatus:	/* Return a set of h/w status flags */
      Wait(&dcb->codecLock);
      error = dcb->codecRegs->controlStatus; /* OK as top bit clear */
      Signal(&dcb->codecLock);
      break;
      
    case FC_ReadInputStatus:	
    case FC_ReadOutputStatus:	
      {
        Codec_Status status;

        Wait(&dcb->codecLock);
        status = ReadStatus(dcb, req->DevReq.Request == FC_ReadOutputStatus);
        req->Actual = *(int *)&status;
        Signal(&dcb->codecLock);
        break;
      }
      
    case FC_Enquire:	
      Wait(&dcb->codecLock);
      error = BufEnquire(dcb, req);
      Signal(&dcb->codecLock);
      break;
      
    case FC_OffHook:
      Wait(&dcb->codecLock);
      SetControlBits(dcb, CODEC_CONTROLOFFH);
      Signal(&dcb->codecLock);
      break;

    case FC_OnHook:
      Wait(&dcb->codecLock);
      ClearControlBits(dcb, CODEC_CONTROLOFFH);
      Signal(&dcb->codecLock);
      break;

    case FC_PulseDial:
      Wait(&dcb->codecLock);
      error = PulseDial(dcb, req);
      Signal(&dcb->codecLock);
      break;

    case FC_PhoneInput:	/* Take audio input from telephone */
      Wait(&dcb->codecLock);
      ClearControlBits(dcb, CODEC_CONTROLMIC);
      Signal(&dcb->codecLock);
      break;

    case FC_MicInput:	/* Take audio input from microphone */
      Wait(&dcb->codecLock);
      SetControlBits(dcb, CODEC_CONTROLMIC);
      Signal(&dcb->codecLock);
      break;

    case FC_PhoneOutput: /* Send audio output to telephone */
      Wait(&dcb->codecLock);
      ClearControlBits(dcb, CODEC_CONTROLSPKR);
      Signal(&dcb->codecLock);
      break;

    case FC_SpeakerOutput: /* Send audio output to loudspeaker */
      Wait(&dcb->codecLock);
      SetControlBits(dcb, CODEC_CONTROLSPKR);
      Signal(&dcb->codecLock);
      break;

#ifdef TRACING      
    case FC_GetTraceBuf:
      Wait(&dcb->codecLock);
      error = (int)&dcb->traceBuf;
      Signal(&dcb->codecLock);
      break;
#endif
      
    default:
      error = EG_FnCode;
      break;
  }
  
  req->DevReq.Result = error;      /* Set error/result */
  (*req->DevReq.Action)(dcb, req); /* Call back client's Action() routine */
}

/*----------------------------------------------------------------------*/

/************************************************************************/
/*                    Read and write operations				*/
/************************************************************************/

/************************************************************************/
/* Allow read buffers to be queued.					*/
/************************************************************************/

static int
EnableRead(CodecDCB *dcb)
{
  BufferQueue *qp = &dcb->readQueue;

  if ((qp->state != Codec_StateQuiescent) &&
      (qp->state != Codec_StateBadEnd))
    return CODEC_SERRORFUNCORDER; /* Must be idle before starting to read */

  qp->state = Codec_StateBegun;
  return 0;
}

/************************************************************************/
/* Allow write buffers to be queued.					*/
/************************************************************************/

static int
EnableWrite(CodecDCB *dcb)
{
  BufferQueue *qp = &dcb->writeQueue;

  if ((qp->state != Codec_StateQuiescent) &&
      (qp->state != Codec_StateBadEnd))
    return CODEC_SERRORFUNCORDER; /* Must be idle before starting to write */

  qp->state = Codec_StateBegun;
  return 0;
}

/************************************************************************/
/* Start reading samples from the codec into the first buffer		*/
/************************************************************************/

static int
StartRead(CodecDCB *dcb)
{
  int         error = 0;
  BufferQueue *qp;

  /* Set up things for the interrupt routine */

  qp = &dcb->readQueue;
  if ((qp->state != Codec_StateBegun) &&
      (qp->state != Codec_StateBusy)) /* Must be ready for buffers */
    { error = CODEC_SERRORFUNCORDER; goto StartReadExit; }

  dcb->rflags = (dcb->rflags & ~CRF_READSLOW) | CRF_FIRSTREAD;
                /* To avoid immediate overrun error */

  /* Reset the fifo, start reading samples, and enable transmit interrupts. */
  
  ClearControlBits(dcb, CODEC_CONTROLTNRS); /* Take reset line low briefly */
  SetControlBits(dcb, CODEC_CONTROLTEN | CODEC_CONTROLTNRS);
  EnableIRQ(INT_CODECTX);

StartReadExit:    
  return error;
}


/************************************************************************/
/* Start writing out samples from the first buffer.			*/
/************************************************************************/

static int
StartWrite(CodecDCB *dcb)
{
  int         error = 0;
  BufferQueue *qp;

  /* Set things up for the interrupt routine */
  qp = &dcb->writeQueue;
  if (qp->state != Codec_StateBegun) /* Must be ready for writing */
    { error = CODEC_SERRORFUNCORDER; goto StartWriteExit; }

  qp->state   = Codec_StateBusy; /* Started output */
  dcb->wflags = (dcb->wflags & ~CWF_WRITESLOW) | CWF_FIRSTWRITE;
                /* To avoid immediate underrun error */

  /* Reset the fifo and enable receive interrupts. 			*/
  /* Don't start writing samples yet: wait until the fifo is half full  */
  /* to prevent an immediate underrun.					*/

  TRACE(TR_STARTWRITE, qp->current);  
  ClearControlBits(dcb, CODEC_CONTROLRNRS); /* Take reset line low briefly */
  SetControlBits(dcb, CODEC_CONTROLRNRS);
  EnableIRQ(INT_CODECRX);

StartWriteExit:    
  return error;
}

/************************************************************************/
/* Abort reading from the codec, and release all the buffers.		*/
/************************************************************************/

static int
AbortRead(CodecDCB *dcb)
{
  BufferQueue *qp = &dcb->readQueue;

  /* Disable codec transmission */
  ClearControlBits(dcb, CODEC_CONTROLTEN | CODEC_CONTROLTNRS);
  DisableIRQ(INT_CODECTX);

  InitBufQueue(qp);  /* Release (i.e. forget) all the buffers */
  qp->state = Codec_StateBadEnd;
  /* Note that we have not signalled any waiting processes */
  return 0;
}


/************************************************************************/
/* Abort writing to the codec, and release all the buffers.		*/
/************************************************************************/

static int
AbortWrite(CodecDCB *dcb)
{
  BufferQueue *qp = &dcb->writeQueue;

  /* Disable codec reception */
  ClearControlBits(dcb, CODEC_CONTROLREN | CODEC_CONTROLRNRS);
  DisableIRQ(INT_CODECRX);

  InitBufQueue(qp);  /* Release (i.e. forget) all the buffers */
  qp->state = Codec_StateBadEnd;
  /* Note that we have not signalled any waiting processes */
  return 0;
}

/*----------------------------------------------------------------------*/

/************************************************************************/
/*			Interrupt routines				*/
/************************************************************************/

/************************************************************************/
/* Low-level device interrupt routine.  It is called from the executive */
/* and passed the DCB address.						*/
/************************************************************************/

int CodecInt(CodecDCB *dcb)
{
  int           res = 0;
  unsigned char status;

  *(int *)0x740800 = 0xFFFF0000;
  status = dcb->codecRegs->controlStatus;
  
  /* Determine which interrupts are outstanding.                      */
  
  if ((status & CODEC_STATUSTNHF) == 0)
    { res = 1; CodecReadInt(dcb); }
  if ((status & CODEC_STATUSRNHF) != 0)
    { res = 1; CodecWriteInt(dcb); }

  *(int *)0x740800 = 0x0000FFFF;
  return res; /* Return 0 if not my interrupt */
}

/************************************************************************/
/* Read interrupt routine						*/
/*									*/
/* This is called when read (transmit) FIFO is half full, so that data  */
/* may be copied into client buffers. 					*/
/************************************************************************/

static void 
CodecReadInt(CodecDCB *dcb)
{
  BufferQueue *qp		    = &dcb->readQueue;
  Buffer *bp 			    = &qp->queue[qp->current];
  unsigned char *readPtr            = bp->addr + bp->xfrd;
  unsigned char *readEnd            = bp->addr + bp->size;
  volatile unsigned char *statusReg = &dcb->codecRegs->controlStatus;
  volatile unsigned char *dataReg   = &dcb->codecRegs->data;
  int moreToRead;

  *(int *)0x740700 = 0xFFFF0000;
  TRACE(TR_READINT, qp->current);
  /* Abort transfer and flag overrun if FIFO has filled up */
  if ((*statusReg & CODEC_STATUSTNFF) == 0)
  { 
    dcb->rflags |= CRF_READSLOW;
    TRACE(TR_OVERRUN, qp->current);
    /* Send a signal if there is a process waiting for the current buffer */
    if (bp->signal)
      HardenedSignal(&qp->waitSema);
    goto ReadIntExit;
  }

  /* Each time round this loop we read data into the current buffer */
  /* iterating if we need to step on to the next buffer.	    */
  do
  {
    moreToRead = 0;

    /* Read data until the buffer is full or the fifo is empty.     */
    /* (This loop could be made faster by reading only HALFFIFOSIZE */
    /* bytes, if it turns out to matter. On the other hand, reading */
    /* more will sometimes save an interrupt)			    */

    while ((*statusReg & CODEC_STATUSTNEF) && (readPtr < readEnd)) 
      *readPtr++ = *dataReg;

    /* Is the current buffer full? */
    if (readPtr >= readEnd)
    {
      /* Update the amount transferred into this buffer */
      bp->xfrd = readPtr - bp->addr;

      /* Send a signal if there is a process waiting for this buffer */
      if (bp->signal)
	{ HardenedSignal(&qp->waitSema); TRACE(TR_READSIG, qp->current); }

      /* Step on to the next buffer; exit if none */
      if (++qp->current >= BUFQUEUEMAX) qp->current = 0;
      if (qp->current == qp->firstFree) /* No more buffers? */
      {
        if (qp->state == Codec_StateEnding) /* Was that the last buffer? */
	{
	  DisableIRQ(INT_CODECTX); /* Stop tx interrupts */
	  qp->state = Codec_StateEndedOK;
	}
      }
      else
      {
        TRACE(TR_READNEXT, qp->current);
	moreToRead = 1;	/* There is another buffer to be filled */
	bp	   = &qp->queue[qp->current];
	readPtr    = bp->addr;
	readEnd    = readPtr + bp->size;
      }
    }
    else
      bp->xfrd = readPtr - bp->addr; /* Current buffer not full */
  } while (moreToRead);

ReadIntExit:
  /* If the transfer is not complete, return leaving the interrupt */
  /* enabled and without waking up the top half.		   */

  if ((readPtr >= readEnd) || (dcb->rflags & CRF_READSLOW))
  {
    /* Disable transmitter interrupts, but leave sampling enabled.	  */

    DisableIRQ(INT_CODECTX); /* Stop tx interrupts */
  }
  TRACE(TR_READEXIT, /*qp->current*/ readEnd-readPtr);
  *(int *)0x740700 = 0x0000FFFF;
}

/************************************************************************/
/* Write interrupt routine						*/
/*									*/
/* This is called when write (receive) FIFO is half empty, so that data */
/* may be copied from the client buffer. 				*/
/************************************************************************/

static void 
CodecWriteInt(CodecDCB *dcb)
{
  BufferQueue *qp		    = &dcb->writeQueue;
  Buffer *bp 			    = &qp->queue[qp->current];
  unsigned char *writePtr           = bp->addr + bp->xfrd;
  unsigned char *writeEnd           = bp->addr + bp->size;
  volatile unsigned char *statusReg = &dcb->codecRegs->controlStatus;
  volatile unsigned char *dataReg   = &dcb->codecRegs->data;
  int moreToWrite		    = 0;

  TRACE(TR_WRITEINT, qp->current);
  /* Abort transfer and flag underrun if FIFO has emptied unless this 	*/
  /* is the first write. Signal any process waiting on the current 	*/
  /* or future buffers if underrun has occurred.			*/

  if ((dcb->wflags & CWF_FIRSTWRITE) == 0)
  {
    if ((*statusReg & CODEC_STATUSRNEF) == 0)
    {
      dcb->wflags |= CWF_WRITESLOW;
      qp->state = Codec_StateBadEnd;
      TRACE(TR_UNDERRUN, qp->current);

      /* Skip over all buffers still to be processed */
      while (qp->current != qp->firstFree)
      {
        Buffer *bp = &qp->queue[qp->current];

        /* Send a signal if there is a process waiting for this buffer */
        if (bp->signal)
	  { HardenedSignal(&qp->waitSema); }

	if (++qp->current >= BUFQUEUEMAX) qp->current = 0;
      }

    goto WriteIntExit;
    }
  }

  /* Each time round this loop we write data from the current buffer */
  /* iterating if we need to step on to the next buffer.	     */
  do
  {
    unsigned char *oldWritePtr = writePtr;
    moreToWrite = 0;

    /* Write data until the buffer is empty or the fifo is full.    */
    /* (This loop could be made faster by writing only HALFFIFOSIZE */
    /* bytes, if it turns out to matter. On the other hand, writing */
    /* more will sometimes save an interrupt)			    */

    if (bp->padding)
    { /* padding, so write zero bytes */
      while ((*statusReg & CODEC_STATUSRNFF) && (writePtr < writeEnd)) 
        { *dataReg = 0; writePtr++; }
    }
    else
    { /* real data */
      while ((*statusReg & CODEC_STATUSRNFF) && (writePtr < writeEnd)) 
        { *dataReg = *writePtr++; }
    }
    qp->transferred += (writePtr - oldWritePtr); /* Total put in fifo */
 
    /* Update the amount transferred from this buffer */
    bp->xfrd = writePtr - bp->addr;

    /* Is the current buffer empty? */
    if (writePtr >= writeEnd)
    {
      int oldCurrent = qp->current;

      /* Send a signal if there is a process waiting for this buffer */
      if (bp->signal)
	{ HardenedSignal(&qp->waitSema); TRACE(TR_WRITESIG, qp->current); }

      /* Step on to the next buffer; exit if none */
      if (++qp->current >= BUFQUEUEMAX) qp->current = 0;
      if (qp->current == qp->firstFree) /* No more buffers? */
      {
        if (qp->state == Codec_StateEnding) /* Was that the last buffer? */
	{
	  /* If we have just finished the last real buffer, step back	*/
	  /* `current' pointer and fabricate a dummy buffer used to 	*/
	  /* generate half a fifo of padding. This ensures that the	*/
	  /* tail end of the data will go through the codec before the  */
	  /* device is closed. The dummy buffer is identified by having */
	  /* its `padding' flag set.					*/
	  if (bp->padding == 0)
	  { /* End of last real buffer */
	    moreToWrite = 1;
	    qp->current = oldCurrent; /* reuse the last buffer slot */
	    /* Padding with half a fifo guarantees that all the real 	*/
	    /* data will be gone when the next interrupt happens.	*/
	    bp->size    = HALFFIFOSIZE; 
	    bp->addr    = 0;
	    bp->key     = 0;
	    bp->xfrd    = 0;
	    bp->padding = 1;
	    bp->signal  = 1; /* So Close routine can wait for it */
	    writePtr    = bp->addr;
	    writeEnd    = writePtr + bp->size;
	  }
	  else /* End of dummy padding buffer */
	  {
	    DisableIRQ(INT_CODECRX); /* Stop rx interrupts */
	    qp->state = Codec_StateEndedOK;
	  }
	}
      }
      else
      {
        TRACE(TR_WRITENEXT, qp->current);
	moreToWrite = 1;  /* There is another buffer to be emptied */
	bp	    = &qp->queue[qp->current];
	writePtr    = bp->addr;
	writeEnd    = writePtr + bp->size;
      }
    }
  } while (moreToWrite);

  /* Common exit */

WriteIntExit:
  /* If the transfer is not complete, return leaving the interrupt */
  /* enabled and without waking up the top half.		   */

  if ((writePtr >= writeEnd) || (dcb->wflags & CWF_WRITESLOW))
  {
    /* Disable receiver interrupts, but leave sampling enabled.	  	  */

    DisableIRQ(INT_CODECRX);
  }

  /* If we have only just started writing but the fifo now holds at 	*/
  /* least WRITETHRESHOLD bytes then enable the codec receiver. This	*/
  /* allows the fifo to be `primed' so we do not get underrun		*/
  /* immediately after starting to write.				*/

  if ((dcb->wflags & CWF_FIRSTWRITE) && (qp->transferred > WRITETHRESHOLD))
  {
    dcb->wflags &= ~CWF_FIRSTWRITE;        /* Not first write any more */
    TRACE(TR_REN, qp->current);
    SetControlBits(dcb, CODEC_CONTROLREN); /* Enable reception */
  }

  TRACE(TR_WRITEEXIT, /*qp->current*/ writeEnd-writePtr);
}

/*----------------------------------------------------------------------*/

/************************************************************************/
/* Update the write only control register and the soft copy.		*/
/* The caller should have claimed dcb->codecLock beforehand to prevent  */
/* clashes with other foreground processes.                             */
/* These functions are also used in interrupt routines to disable       */
/* rx and tx interrupts. It is important that the soft copy is updated  */
/* before the real register, to limit the damage if nested calls occur. */
/* (A clash will result in the foreground process re-enabling           */
/* interrupts, causing an interrupt to be taken immediately. The        */
/* interrupt routine will do nothing other than disable that interrupt  */
/* again (as the buffer pointers will prevent any data transfer).)	*/
/*									*/
/* It would be better to lock these more thoroughly...                  */
/************************************************************************/

static void
SetControlBits(CodecDCB *dcb, unsigned char bits)
{
  dcb->controlCopy |= bits;
  dcb->codecRegs->controlStatus = dcb->controlCopy;
}

static void
ClearControlBits(CodecDCB *dcb, unsigned char bits)
{
  dcb->controlCopy &= ~bits;
  dcb->codecRegs->controlStatus = dcb->controlCopy;
}

/*----------------------------------------------------------------------*/

/************************************************************************/
/* 		     	Buffer queue management				*/
/************************************************************************/

/************************************************************************/
/* Initialise a buffer queue structure.					*/
/************************************************************************/
static void
InitBufQueue(BufferQueue *qp)
{
  int    i;
  Buffer *bp;

  qp->firstReady  = 0;
  qp->current     = 0;
  qp->firstFree   = 0;
  qp->timeout     = -1; /* wait for ever */
  qp->state	  = Codec_StateQuiescent;
  qp->transferred = 0;
  InitSemaphore(&qp->waitSema, 0); /* Wait blocks until signalled */

  bp = &qp->queue[0];
  for (i=0; i<BUFQUEUEMAX; ++i)
  {
    bp->addr   = 0;
    bp->size   = 0;
    bp->xfrd   = 0;
    bp->signal = 0;
    bp->padding= 0;
    bp->key    = 0;
    ++bp;
  }
}


/************************************************************************/
/* Queue an input or output buffer, returning a buffer handle or error.	*/
/************************************************************************/

static Result
QueueBuffer(CodecDCB *dcb, int write, CodecReq *req)
{
  BufferQueue *qp  = (write ? &dcb->writeQueue : &dcb->readQueue);
  int         slot = qp->firstFree;
  int         next = ((slot+1) < BUFQUEUEMAX ? slot+1 : 0);
  Buffer      *buf = &qp->queue[slot];
  Codec_BufHandle handle;

  if (qp->state == Codec_StateEnding)
    return CODEC_SERRORFINISHING; /* The final buffer is already queued */

  /* Check for overrun/underrun */
  if (write)
  {
    if (dcb->wflags & CWF_WRITESLOW) return CODEC_SERRORUNDERRUN;
  }
  else
    if (dcb->rflags & CRF_READSLOW) return CODEC_SERROROVERRUN;

  if ((qp->state != Codec_StateBegun) &&
      (qp->state != Codec_StateBusy))
    return CODEC_SERRORFUNCORDER; /* Must be ready for buffers */

  if (next == qp->firstReady)
    return CODEC_SERRORTOOMANYBUFS; /* Too many buffers queued at once */
  
  buf->addr   = req->Buf;
  buf->size   = (int)req->Size;
  buf->signal = 0;
  buf->padding= 0;
  buf->xfrd   = 0;
  handle    = NewBufferHandle(dcb, write, slot);
  qp->firstFree = next; /* Step this last to avoid race with int routine */

  TRACE(write ? TR_QWRITEBUF : TR_QREADBUF, slot);
  return (Result)handle;
}

/************************************************************************/
/* Mark the last buffer queued as the final buffer in this stream, and	*/
/* return its handle.							*/
/************************************************************************/
static Result 
MarkFinalBuffer(CodecDCB *dcb, int write, Codec_BufHandle *handlePtr /*out*/)
{
  BufferQueue *qp  = (write ? &dcb->writeQueue : &dcb->readQueue);
  int slot = qp->firstFree - 1;
  if (slot < 0) slot += BUFQUEUEMAX;

  if (qp->state == Codec_StateEnding)
    return CODEC_SERRORFINISHING; /* The final buffer is already queued */

  if (ValidBuffer(qp, slot))
    *handlePtr = BufferHandleForSlot(dcb, write, slot);
  else
    return CODEC_SERRORNOBUFFERS; /* No buffers queued at present */  

  qp->state = Codec_StateEnding; /* Record that this stream is ending */
  return 0;
}

/************************************************************************/
/* Check whether a slot number refers to a queued buffer.		*/
/************************************************************************/
static int
ValidBuffer(BufferQueue *qp, int slot)
{
  int first = qp->firstReady;
  int last  = qp->firstFree;

  if (last < first) last += BUFQUEUEMAX; /* Ensure first <= last */
  if (slot < first) slot += BUFQUEUEMAX; /* Ensure first <= slot */
  return (slot < last);
}

/************************************************************************/
/* Wait for a buffer (which must be the oldest queued buffer) to be	*/
/* filled or emptied. On successful return, the buffer is dequeued, and */
/* req->Actual holds the number of bytes actually transferred.		*/
/* This is called with the dcb already locked, so must unlock it if it	*/
/* has to sleep.							*/
/* *** TIMEOUTS ARE NOT YET IMPLEMENTED ***				*/
/************************************************************************/
static Result
WaitForBuffer(CodecDCB *dcb, CodecReq *req)
{
  Codec_BufHandle handle = (Codec_BufHandle)req->Buf;
  int	          write;
  int		  slot;
  Buffer	  *buf;
  BufferQueue	  *qp;
  Result	  error;
  unsigned int    intBit;
  int 		  needToWait = 0;

  error = CheckBufferHandle(dcb, handle, &write, &slot, &buf);
  if (error) goto WaitExit;

  qp = (write ? &dcb->writeQueue : &dcb->readQueue);
  if (slot != qp->firstReady)
    { error = CODEC_SERRORBUFORDER; goto WaitExit; } /* Not oldest buffer */

  /* If this buffer is not yet ready, then tell the interrupt routine	*/
  /* to signal us when it is ready, and wait.				*/

  /***** CRITICAL REGION *****/
  /* This code must be atomic, so disable the appropriate device 	*/
  /* interrupt. It is always safe to disable the interrupt without 	*/
  /* knowing its current state; we can safely decide whether it should	*/
  /* be re-enabled by looking at the buffer queue.			*/

  intBit = (write ? INT_CODECRX : INT_CODECTX);
  DisableIRQ(intBit); /* Disable interrupt */
  *(int *)0x740900 = 0xFFFF0000;

  if (slot == qp->current)
  {
    buf->signal = 1; /* Tell int routine to signal when this buffer ready */
    needToWait  = 1;
  }

  /* Re-enable interrupt if there are still any buffers to fill or empty */
  if (qp->current != qp->firstFree)
  {
    EnableIRQ(intBit); /* Enable interrupt */
    *(int *)0x740900 = 0x0000FFFF;
  }
  /***** END OF CRITICAL REGION *****/

  if (needToWait)
  {
    Signal(&dcb->codecLock);	 /* Release lock on device registers */
    TRACE(TR_STARTWAIT, write);
    *(int *)0x740600 = 0xFFFF0000;
    HardenedWait(&qp->waitSema); /* Wait for buffer to be ready */
    *(int *)0x740600 = 0x0000FFFF;
    TRACE(TR_ENDWAIT, write);
    Wait(&dcb->codecLock);	 /* Reclaim lock on device registers */
  }

  /* Dequeue the buffer and return the actual number of bytes transferred */
  req->Actual = buf->xfrd;
  if (++qp->firstReady >= BUFQUEUEMAX) qp->firstReady = 0;

WaitExit:
  return error;
}

/************************************************************************/
/* Enquire about the status of a particular buffer.			*/
/* req->Actual is set non-zero if buffer is ready, and to 0 otherwise.	*/
/************************************************************************/
static Result
BufEnquire(CodecDCB *dcb, CodecReq *req)
{
  Codec_BufHandle handle = (Codec_BufHandle)req->Buf;
  int	          write;
  int		  slot, firstReady, current;
  Buffer	  *buf;
  BufferQueue	  *qp;
  Result	  error;

  error = CheckBufferHandle(dcb, handle, &write, &slot, &buf);
  if (error) return error;

  qp = (write ? &dcb->writeQueue : &dcb->readQueue);
  firstReady = qp->firstReady;
  current    = qp->current;

  /* Unravel the circular buffer pointers so they can be compared */
  if (slot < firstReady) slot += BUFQUEUEMAX;
  if (current < firstReady) current += BUFQUEUEMAX;

  req->Actual = (slot < current); /* i.e. slot is between first and current */
  return error;
}

/************************************************************************/
/* Return the status of a device stream (i.e. buffer queue).		*/
/************************************************************************/
static Codec_Status
ReadStatus(CodecDCB *dcb, int write) 
{
  BufferQueue *qp = (write ? &dcb->writeQueue : &dcb->readQueue);
  Codec_Status res;
  int          firstReady = qp->firstReady;
  int 	       current    = qp->current;
  int	       firstFree  = qp->firstFree;

  /* Unwind the circular buffer pointers so we can take differences */
  if (current < firstReady) current += BUFQUEUEMAX;
  if (firstFree < firstReady) firstFree += BUFQUEUEMAX;

  res.state       = qp->state;
  res.bufsUnready = firstFree - current;
  res.bufsReady   = current - firstReady;
  res.errorFlag   = (qp->state == Codec_StateBadEnd) ||
                    (write && (dcb->wflags & CWF_WRITESLOW)) ||
		    (!write && (dcb->rflags & CRF_READSLOW));

  return res;
}

/*----------------------------------------------------------------------*/

/************************************************************************/
/* Buffer handle management						*/
/*									*/
/* Buffer handles are 32-bit values with the following structure:	*/
/*									*/
/*		0W00000000000000KKKKKKKKSSSSSSSS			*/
/*									*/
/*  Top bit is zero to avoid confusion with error codes			*/
/*  W is 0 for a read (transmit) buffer, 1 for a write (receive) buffer */
/*  K is a key to aid detection of old/invalid handles (0 is invalid)	*/
/*  S is the slot number in the buffer queue array			*/
/************************************************************************/

static Codec_BufHandle
NewBufferHandle(CodecDCB *dcb, int write, int slot)
{
  BufferQueue *qp  = (write ? &dcb->writeQueue : &dcb->readQueue);
  Buffer      *buf = &qp->queue[slot];

  if (++(buf->key) == 0) buf->key = 1; /* Keep 0 as an invalid key */
  return (write ? 0x40000000 : 0x00000000) |
         ((buf->key & 0xFF) << 8) |
	 (slot & 0xFF);
}	


/* Return the existing handle for a given (valid) buffer slot. */
static Codec_BufHandle
BufferHandleForSlot(CodecDCB *dcb, int write, int slot)
{
  BufferQueue *qp  = (write ? &dcb->writeQueue : &dcb->readQueue);
  Buffer      *buf = &qp->queue[slot];

  return (write ? 0x40000000 : 0x00000000) |
         ((buf->key & 0xFF) << 8) |
	 (slot & 0xFF);
}


static Result
CheckBufferHandle
(
  CodecDCB        *dcb, 
  Codec_BufHandle handle,
  int	          *write /*out*/, /* set to 1 for write buf, 0 for read */
  int		  *slot  /*out*/, /* set to slot number in queue	*/
  Buffer	  **buf  /*out*/  /* set to point to buffer structure	*/
)
{
  unsigned int writeField = (handle & 0x40000000) != 0;
  unsigned int slotField  = handle & 0xFF;
  unsigned int keyField   = (handle >> 8) & 0xFF;
  BufferQueue  *qp;
  Buffer       *bp;
  Result       error = CODEC_SERRORBUFHANDLE;

  if ((slotField >= BUFQUEUEMAX) || (handle & 0xBFFF0000) || (keyField == 0))
    goto CheckExit;

  qp = (writeField ? &dcb->writeQueue : &dcb->readQueue);
  bp = &qp->queue[slotField];

  if (!ValidBuffer(qp, slotField) || (bp->key != keyField))
    goto CheckExit;

  /* This is a valid handle */
  error  = 0;
  *write = writeField;
  *buf   = bp;
  *slot  = slotField;

CheckExit:
  return error;
} 

/*----------------------------------------------------------------------*/

/************************************************************************/
/* 		       Phone Line Operations				*/
/************************************************************************/

/************************************************************************/
/* Pulse dial the number given as an ASCII string in req->Buf.		*/
/* The phone is taken off hook, and left off hook on exit.		*/
/************************************************************************/
static Result
PulseDial(CodecDCB *dcb, CodecReq *req)
{
  char *numStr = req->Buf;
  int  len = 0;
  int  i;
  char ch;

  /* See if we like the number string */
  while ((ch = numStr[len++]) != 0)
  {
    if ((ch < '0') || (ch > '9')) 
      return CODEC_SERRORPHONENUMBER; /* Non digit */
  }

  if (len > MAXPHONENUMBERDIGITS) 
    return CODEC_SERRORPHONENUMBER; /* Too long to be a real phone number */

  /* Number OK, so dial it */
  SetControlBits(dcb, CODEC_CONTROLOFFH); /* Take the phone off hook */
  Delay(DIALTONEDELAY);	/* Wait for dial tone. Should listen really */
  ClearControlBits(dcb, CODEC_CONTROLLP); /* Take LP low for dialling */

  for (i=0; i<len; ++i)
    PulseDialDigit(dcb, numStr[i] - '0');

  SetControlBits(dcb, CODEC_CONTROLLP); /* Finished dialling: take LP high */

  /* Phone left off hook on exit */
  return 0;
}

/************************************************************************/
/* Pulse dial one digit. The inter-digit gap is included afterwards.	*/
/************************************************************************/
static void
PulseDialDigit(CodecDCB *dcb, int digit)
{
  int pulses = (digit == 0 ? 10 : digit);
  int i;

  for (i=0; i<pulses; ++i)
  {
    ClearControlBits(dcb, CODEC_CONTROLOFFH);	/* Start a pulse */
    Delay(DIALPULSELENGTH);			/* Hold the pulse */
    SetControlBits(dcb, CODEC_CONTROLOFFH);	/* End the pulse */
    Delay(DIALPULSEGAP);			/* Inter-pulse gap */
  }

  Delay(INTERDIGITGAP - DIALPULSEGAP); /* Gap between dialled digits */
}

/*----------------------------------------------------------------------*/

/* End of codec.c */
