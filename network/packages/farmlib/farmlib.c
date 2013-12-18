/*------------------------------------------------------------------------
--                                                                      --
--      H E L I O S   P A R A L L E L   P R O G R A M M I N G		--
--	-----------------------------------------------------		--
--									--
--	  F A R M   C O M P U T A T I O N   L I B R A R Y		--
--	  -----------------------------------------------		--
--									--
--		Copyright (C) 1992, Perihelion Software Ltd.		--
--                        All Rights Reserved.                          --
--                                                                      --
-- farmlib.c								--
--                                                                      --
--	Author:  BLV 3/1/92						--
--                                                                      --
------------------------------------------------------------------------*/
/* RcsId: $Id: farmlib.c,v 1.7 1993/03/27 20:38:54 bart Exp $ */

/*{{{  version number and revision history */
#define	VersionNumber	"1.01"

/**
*** Revision history:
***  1.00	first release for beta 2 of Helios 1.3
***  1.01	various bug fixes for C40 release
**/
/*}}}*/
/*{{{  header files and compile-time options */
#include <helios.h>
#include <memory.h>
#include <queue.h>
#include <sem.h>
#include <syslib.h>
#include <servlib.h>
#include <unistd.h>
#include <string.h>
#include <rmlib.h>
#include <gsp.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <nonansi.h>
#include <stdarg.h>
#include <codes.h>
#include <errno.h>
#include <farmlib.h>

#define eq ==
#define ne !=
/*}}}*/
/*{{{  library configuration variables */
/** 
*** These variables can/should be set by the application to control the
*** behaviour of the farming library.
**/
int		FmFloodOption		= Fm_Network;
RmNetwork	FmSelectedNetwork	= NULL;
int		FmNumberWorkers		= -1;
bool		FmFastStack		= FALSE;
bool		FmFastCode		= FALSE;
bool		FmOverloadController	= FALSE;
int		FmJobSize		= -1;
int		FmReplySize		= -1;
int		FmSeed			= 0;
bool		FmFaultHandling		= FALSE;
void		(*FmProducer)(void)	= NULL;
int		FmProducerStack		= 5000;
void		(*FmConsumer)(void)	= NULL;
int		FmConsumerStack		= 5000;
void		(*FmWorker)(void)	= NULL;
int		FmWorkerStack		= 5000;
void		(*FmProvider)(void)	= NULL;
int		FmProviderStack		= 5000;
void		(*FmControllerInitialise)(void)	= NULL;
void		(*FmWorkerInitialise)(void)	= NULL;
void		(*FmControllerExit)(void)	= NULL;
void		(*FmWorkerExit)(void)		= NULL;
bool		FmVerbose		= FALSE;
char		*FmVersionNumber	= VersionNumber;
int		FmDebugFlags		= 0;

/**
*** These variables are available to the application for information
**/
int		FmRunningWorkers	= -1;
int		FmWorkerNumber		= -1;
/*}}}*/
/*{{{  data structures and statics */
/**
*** "Header" is a structure which prefixes every packet. Some of the
*** information is transmitted, other information is kept locally in order
*** to minimise comms overhead.
**/

typedef struct Header	{
	Node		Node;
	int		Magic;
	int		Destination;
	int		Size;	/* From here on the structure is sent	*/
	int		Flags;
	BYTE		Data[1]; /* variable */
} Header;

#define sizeof_Header		24	/* Not including Data field	*/

#define	Header_Transferred	 8	/* Size + flags fields		*/

	/* Header flags							*/
#define Header_NoAck		0x0001
#define Header_Malloced		0x0002
#define Header_InfoRequest	0x0004
#define Header_InfoReply	0x0008

	/* Magic numbers to check up on the user			*/
#define	Magic_Job			0x0BADBAD0
#define Magic_Reply			0x1BADBAD1
#define Magic_InfoRequest		0x2BADBAD2
#define Magic_InfoReply			0x3BADBAD3

/**
*** Some of the library's private variables used across folds
**/
	/* Exclusive access to the C library, mainly for diagnostics	*/
static	Semaphore		Clib;

	/* Quick way of testing whether or not running in controller	*/
static	bool			InControl	= FALSE;

	/* Similar test for worker. N.B. both conditions may be true.	*/
static	bool			InWorker	= FALSE;

	/* The file descriptors used by the worker to communicate	*/
	/* depend on whether this is a separate worker task or just	*/
	/* an overload thread within the controlller.			*/
static	int			WorkerInput	= -1;
static	int			WorkerOutput	= -1;

	/* In an overloaded controller, these two are used to store	*/
	/* temporarily the pipes to use between the controller and its	*/
	/* internal worker.						*/
static	int			OverloadToWorker 	= -1;
static	int			OverloadFromWorker	= -1;

	/* To implement FmFreeInfoReplyBuffer() it is necessary to	*/
	/* keep track of the last info reply buffer returned to the	*/
	/* worker routine.						*/
static	Header		*LastInfoReplyHeader	= NULL;

	/* To leave plenty of file descriptors open the farm library	*/
	/* uses file descriptors 20 onwards to interact with the	*/
	/* workers. The exception is the overloaded worker if any :	*/
	/* the pipes are generated internally using the pipe call.	*/
#define	FirstWorkerPipe		20

	/* The farm library spawns various internal threads to handle	*/
	/* communication etc. These all use the following stacksize.	*/
#define	StackSize	3000

#define Debug(a, b) if (FmDebugFlags & a) debug b

	/* This is the "name" of a worker task, i.e. argv[0]		*/
#define WorkerName	"<worker>"

	/* This is the name of the controller task. It is sent to all	*/
	/* the workers during initialisation and provides a quick way	*/
	/* of detecting a controller crash when SIGSIB is received.	*/
static char	ControllerName[IOCDataMax];
/*}}}*/
/*{{{  utility routines */
/*{{{  diagnostics */
/**
*** Error reporting routines with locking to prevent simultaneous access
*** to the C library
**/

static void report(char *str, ...)
{ va_list	args;
  va_start(args, str);
  Wait(&Clib);
  fputs("Farmlib: ", stderr);
  if (InControl)
   fputs("control, ", stderr);
  else
   { if (FmWorkerNumber eq -1)
      fputs("worker (anon), ", stderr);
     else
      fprintf(stderr, "worker %d, ", FmWorkerNumber);
   }
  vfprintf(stderr, str, args);
  fputs("\r\n", stderr);
  Signal(&Clib);
  va_end(args);
}

static void debug(char *str, ...)
{ va_list	args;
  va_start(args, str);
  Wait(&Clib);
  fputs("Farmlib: debug, ", stderr);
  if (InControl)
   fputs("control, ", stderr);
  else
   { if (FmWorkerNumber eq -1)
      fputs("worker (anon), ", stderr);
     else
      fprintf(stderr, "worker %d, ", FmWorkerNumber);
   }
  vfprintf(stderr, str, args);
  fputs("\r\n", stderr);
  Signal(&Clib);
  va_end(args);
}

static void warn(char *str, ...)
{ va_list	args;

  if (!FmVerbose) return;

  va_start(args, str);
  Wait(&Clib);
  fputs("Farmlib: ", stderr);
  if (InControl)
   fputs("control, ", stderr);
  else
   { if (FmWorkerNumber eq -1)
      fputs("worker (anon), ", stderr);
     else
      fprintf(stderr, "worker %d, ", FmWorkerNumber);
   }
  fputs("warning, ", stderr);
  vfprintf(stderr, str, args);
  fputs("\r\n", stderr);
  Signal(&Clib);
  va_end(args);
}

static void fatal(char *str, ...)
{ va_list	args;

  va_start(args, str);
  Wait(&Clib);
  fputs("FarmLib: ", stderr);
  if (InControl)
   fputs("control, ", stderr);
  else
   { if (FmWorkerNumber eq -1)
      fputs("worker (anon), ", stderr);
     else
      fprintf(stderr, "worker %d, ", FmWorkerNumber);
   }
  fputs("fatal error, ", stderr);
  vfprintf(stderr, str, args);
  fputs("\r\n", stderr);
  Signal(&Clib);
  va_end(args);
  exit(EXIT_FAILURE);
}

static void internal(char *str, ...)
{ va_list	args;

  va_start(args, str);
  Wait(&Clib);
  fputs("FarmLib: ", stderr);
  if (InControl)
   fputs("control, ", stderr);
  else
   { if (FmWorkerNumber eq -1)
      fputs("worker (anon), ", stderr);
     else
      fprintf(stderr, "worker %d, ", FmWorkerNumber);
   }
  fputs("fatal internal error, ", stderr);
  vfprintf(stderr, str, args);
  fputs("\r\n", stderr);
  Signal(&Clib);
  va_end(args);
  exit(EXIT_FAILURE);
}
/*}}}*/
/*{{{  comms */
/**
*** These communication routines have been designed to cope with 
*** various failures including signals.
**/
static bool full_read(int *fd, BYTE *buffer, int amount)
{ int	to_read	= amount;
  int	temp;

  Debug(FmD_IO, ("full_read(%d, %p, %d) (%s)", *fd, buffer, to_read, fdstream(*fd)->Name));

  while ((to_read > 0) && (*fd >= 0))
   { temp= read(*fd, buffer, to_read);

     if (temp <= 0)	/* I/O error, including timeout			*/
      { if (InControl)	/* Ignore all errors, *fd will be set to -1	*/
	 continue;	/* by the exception handling code.		*/
	elif (errno eq EINTR)	/* A signal of some sort...		*/
	 { errno = 0; continue; }
	else
	 { Debug(FmD_IO, ("full_read(%d): failure", *fd));
	   return(FALSE);		/* EOF */
	 }
      }

     to_read -= temp;
     buffer   = &(buffer[temp]);
   }

  if (*fd < 0)
   { Debug(FmD_IO, ("full_read: file descriptor aborted"));
     return(FALSE);
   }

  Debug(FmD_IO, ("full_read(%d, %p, %d)(%s) done", *fd, buffer, amount, fdstream(*fd)->Name));
  return(TRUE);
}

static bool full_write(int *fd, BYTE *buffer, int amount)
{ int	to_write	= amount;
  int	temp;

  Debug(FmD_IO, ("full_write(%d, %p, %d)(%s)", *fd, buffer, amount, fdstream(*fd)->Name));

  while ((to_write > 0) && (*fd >= 0))
   { temp = write(*fd, buffer, to_write);

     if (temp <= 0)
      { Debug(FmD_IO, ("full_write(%d): failure, rc %d, errno %d", *fd, temp, errno));
	if (InControl)
	 continue;
	elif (errno eq EINTR)
	 { errno = 0; continue; }
	else
	 return(FALSE);
      }

     to_write	-= temp;
     buffer	 = &(buffer[temp]);
   }

  if (*fd < 0)
   { Debug(FmD_IO, ("full_write: file descriptor aborted"));
     return(FALSE);
   }

  Debug(FmD_IO, ("full_write(%d, %p, %d)(%s) done", *fd, buffer, amount, fdstream(*fd)->Name));  
  return(TRUE);
}
/*}}}*/
/*{{{  is_compatible() */
/**
*** This routine checks whether two processors are "compatible". Any
*** transputer is compatible with any other transputer (hopefully
*** there are no problems with floating point...). Otherwise a
*** processor is only compatible with its own type.
**/
static bool is_compatible(RmProcessor proc1, RmProcessor proc2)
{ int	type1 = RmGetProcessorType(proc1);
  int	type2 = RmGetProcessorType(proc2);

  if ((type1 eq -1) || (type2 eq -1))
   return(FALSE);

  if ((type1 eq RmT_T800) || (type1 eq RmT_T414) || (type1 eq RmT_T425) ||
      (type1 eq RmT_T400))
   { if ((type2 eq RmT_T800) || (type2 eq RmT_T414) || (type2 eq RmT_T425) ||
	 (type2 eq RmT_T400))
      return(TRUE);
     else
      return(FALSE);
   }

  if (type1 eq type2)
   return(TRUE);
  else
   return(FALSE);
}
/*}}}*/
/*}}}*/
/*{{{  buffer management */
/*{{{  variables */
	/* This lock is used to provide exclusive access to the		*/
	/* various linked lists of buffers.				*/
static	Semaphore	BufferLock;

	/* Lists of buffers of the default sizes.			*/
static	List		DefaultJobList;
static	List		DefaultReplyList;

	/* Counting semaphore to keep track of the free buffers		*/
static	Semaphore	JobCount;
static	Semaphore	ReplyCount;

	/* If default sizes are specified and there is enough free	*/
	/* memory then these variables will be set to the default	*/
	/* sizes. Otherwise they will be set to -1. This facilitates	*/
	/* testing buffer requests for default sizes.			*/
static	int		fm_JobSize;
static	int		fm_ReplySize;
/*}}}*/
/*{{{  initialisation */
/**
*** Buffer initialisation occurs within both the controller and the workers.
*** Within the controller the number of buffers depends on the number of
*** workers. Within each worker the number of buffers is fixed. If
*** default sizes have been specified and there is enough free memory then
*** buffers will be pre-allocated. 
***
*** No pre-allocation of information request or reply buffers takes place.
**/

static	void	init_buffers(int inputs, int outputs)
{ int	i;

  Debug(FmD_Buffers, ("initialising %d input buffers and %d output buffers", inputs, outputs));

  InitSemaphore(&BufferLock, 1);
  InitList(&DefaultJobList);
  InitList(&DefaultReplyList);
  InitSemaphore(&JobCount, 0);
  InitSemaphore(&ReplyCount, 0);
  fm_JobSize	= FmJobSize;
  fm_ReplySize	= FmReplySize;

  if (FmJobSize ne -1)
   { int	 size;
     BYTE 	*buffers; 
     Header	*header;

     if (FmJobSize < 0)
      fatal("illegal value %d for FmJobSize", FmJobSize);

     size	= (sizeof_Header + FmJobSize + 3) & ~3;
     buffers	= malloc(inputs * size);
     if (buffers eq NULL)
      { if (InControl)
	 warn("not enough memory to preallocate job buffers");
        fm_JobSize = -1;
      }
     else
      { Debug(FmD_Buffers, ("obtained the input buffers"));
	for (i = 0; i < inputs; i++)
         { header = (Header *) &(buffers[i * size]);
           AddTail(&DefaultJobList, &(header->Node));
           header->Size		= FmJobSize;
           header->Magic	= Magic_Job;
           header->Flags        = 0;
         }
        InitSemaphore(&JobCount, inputs);
      }
   }
  else
   { Debug(FmD_Buffers, ("no input buffers pre-allocated"));
   }

  if (FmReplySize ne -1)
   { int	 size;
     BYTE	*buffers;
     Header	*header;

     if (FmReplySize < 0)
      fatal("illegal value %d for FmReplySize", FmReplySize);

     size	= (sizeof_Header + FmReplySize + 3) & ~3;
     buffers	= malloc(outputs * size);
     if (buffers eq NULL)
      { if (InControl)
         warn("not enough memory to preallocate reply buffers");
        fm_ReplySize = -1;
      }
     else
      { Debug(FmD_Buffers, ("obtained the output buffers"));
        for (i = 0; i < outputs; i++)
         { header = (Header *) &(buffers[i * size]);
           AddTail(&DefaultReplyList, &(header->Node));
           header->Size		= FmReplySize;
           header->Magic	= Magic_Reply;
           header->Flags	= 0;
         }
        InitSemaphore(&ReplyCount, outputs);
      }
   }
  else
   { Debug(FmD_Buffers, ("no output buffers pre-allocated"));
   }
}
/*}}}*/
/*{{{  job buffers */
/**
*** Job buffers can be obtained explicitly from within the Producer
*** routine, or implicitly inside the worker. If there is a default size
*** and the buffer is of that size then the next free buffer will be
*** taken off the list. Otherwise a suitable buffer has to be allocated
*** dynamically.
**/
void *FmGetJobBuffer(int size)
{ Header	*header;

  if (size < 0)
   fatal("illegal size %d passed to FmGetJobBuffer", size);

  Debug(FmD_Buffers, ("request for job buffer of %d bytes", size));

  if (size eq fm_JobSize)
   { Wait(&JobCount);
     Wait(&BufferLock);
     header = (Header *) RemHead(&DefaultJobList);
     Signal(&BufferLock);
     header->Magic = Magic_Job;
     Debug(FmD_Buffers, ("returning pre-allocated job buffer %p", &(header->Data[0])));
     return(&(header->Data[0]));
   }

	/* ??? - should the size be validated ?		*/
  while ((header = malloc(sizeof_Header + size)) eq NULL)
   Delay(OneSec);
  header->Size	= size;
  header->Magic = Magic_Job;
  header->Flags = Header_Malloced;
  Debug(FmD_Buffers, ("returning malloced job buffer %p", &(header->Data[0])));
  return(&(header->Data[0]));
}

/**
*** Free'ing of job buffers only ever occurs from inside the library.
*** Inside the controller it occurs when the buffer is no longer needed:
***   1) no reply is expected
***   2) no reply is possible because the worker has failed
***   3) a reply has been received
***
*** Inside the worker the previous buffer is automatically freed when
*** FmGetJob() is called.
**/
static void FmFreeJobBuffer(void *buf)
{ Header *header = (Header *) ((BYTE *) buf - sizeof_Header);

  Debug(FmD_Buffers, ("freeing job buffer %p", buf));

  if (header->Magic ne Magic_Job)
   internal("FmFreeJobBuffer has been given an invalid buffer");

  header->Magic = 0;
  if (header->Flags & Header_Malloced)
   { Debug(FmD_Buffers, ("freeing malloced job buffer %p", buf));
     free(header);
   }
  else
   { Debug(FmD_Buffers, ("reclaiming pre-allocated job buffer %p", buf));
     Wait(&BufferLock);
     AddTail(&DefaultJobList, &(header->Node));
     Signal(&BufferLock);
     Signal(&JobCount);
   }
}
/*}}}*/
/*{{{  reply buffers */
/**
*** Reply buffer management is very similar to job buffers. FmGetReplyBuffer()
*** can be called explicitly from inside the Worker(), or implicitly
*** within the controller when a reply has been received. The reply header
*** contains information relating it to the job.
**/
void *FmGetReplyBuffer(void *job, int size)
{ Header	*header;
  Header	*job_header;

  if (job eq NULL)
   fatal("NULL job pointer passed to FmGetReplybuffer");

  job_header = (Header *) ((BYTE *) job - sizeof_Header);

  if (!InControl && (job_header->Flags & Header_NoAck))
   fatal("attempt to get reply buffer for a job that should not be acknowledged");

  if (size < 0)
   fatal("illegal size %d passed to FmGetReplyBuffer", size);

  Debug(FmD_Buffers, ("request for reply buffer of %d bytes to match job %p", size, job));

  if (size eq fm_ReplySize)
   { Wait(&ReplyCount);
     Wait(&BufferLock);
     header = (Header *) RemHead(&DefaultReplyList);
     Signal(&BufferLock);
     header->Magic	= Magic_Reply;
     Debug(FmD_Buffers, ("returning pre-allocated reply buffer %p", &(header->Data[0])));
     return(&(header->Data[0]));
   }

	/* ??? should the size be validated ?	*/
  while ((header = malloc(sizeof_Header + size)) eq NULL)
   Delay(OneSec);

  header->Size	= size;
  header->Flags	= Header_Malloced;
  header->Magic	= Magic_Reply;
  Debug(FmD_Buffers, ("returning malloc'ed reply buffer %p", &(header->Data[0])));
  return(&(header->Data[0]));
}

/**
*** Freeing a reply buffer only occurs from inside the farm library.
*** Within the worker it occurs as soon as the reply has been transmitted
*** to the controller. Within the worker the previous reply buffer is freed
*** automatically from inside FmGetReply().
**/
static void 	FmFreeReplyBuffer(void *buf)
{ Header	*header = (Header *) ((BYTE *) buf - sizeof_Header);

  unless(header->Magic eq Magic_Reply)
   internal("FmFreeReplyBuffer has been passed an invalid buffer");

  Debug(FmD_Buffers, ("request to free reply buffer %p", buf));

  header->Magic = 0;
  if (header->Flags & Header_Malloced)
   { Debug(FmD_Buffers, ("freeing malloced reply buffer %p", buf));
     free(header);
   }
  else
   { Debug(FmD_Buffers, ("reclaiming pre-allocated reply buffer %p", buf));
     Wait(&BufferLock);
     AddTail(&DefaultReplyList, &(header->Node));
     Signal(&BufferLock);
     Signal(&ReplyCount);
   }
}
/*}}}*/
/*{{{  info buffers */
/**
*** Information buffers are rather easier to handle than job or reply
*** buffers, as they are always allocated dynamically. Information
*** requests are meant to be fairly sporadic, so the cost of the occasional
*** malloc is worth paying for not having to pre-allocate yet another set
*** of buffers.
***
*** FmGetInfoRequestBuffer() is called explicitly within the worker and
*** implicitly within the controller when an information request is
*** received.
**/
void *FmGetInfoRequestBuffer(int size)
{ Header	*header;

  if (size < 0)
   fatal("illegal size %d passed to FmGetInfoRequestBuffer", size);

  Debug(FmD_Buffers, ("request for InfoRequest buffer of %d bytes", size));

  while ((header = malloc(sizeof_Header + size)) eq NULL)
   Delay(OneSec);
  header->Size	= size;
  header->Magic	= Magic_InfoRequest;
  header->Flags = (Header_Malloced | Header_InfoRequest);
  Debug(FmD_Buffers, ("returning malloced InfoRequest buffer %p", &(header->Data[0])));
  return(&(header->Data[0]));     
}

/**
*** FmGetInfoReplyBuffer() is called explicitly from within the provider
*** routine, and implicitly within the workers when an information reply
*** is received.
**/
void *FmGetInfoReplyBuffer(void *request, int size)
{ Header	*header;
  Header	*request_header;

  if (request eq NULL)
   fatal("NULL request pointer passed to FmGetInfoReplyBuffer");

  request_header = (Header *) ((BYTE *) request - sizeof_Header);
  if (request_header->Magic ne Magic_InfoRequest)
   fatal("illegal info request buffer passed to FmGetInfoReplyBuffer");
  
  if (size < 0)
   fatal("illegal size %d passed to FmGetInfoReplyBuffer", size);

  Debug(FmD_Buffers, ("request for InfoReply buffer of %d bytes, to match request %p", size, request));

  while ((header = malloc(sizeof_Header + size)) eq NULL)
   Delay(OneSec);

  header->Magic 	= Magic_InfoReply;
  header->Flags 	= (Header_Malloced + Header_InfoReply);
  header->Size		= size;
  header->Destination	= request_header->Destination;
  Debug(FmD_Buffers, ("returning malloced InfoReply buffer %p", &(header->Data[0])));
  return(&(header->Data[0]));
}

/**
*** Information request buffers are never freed explicitly. They are
*** freed implicitly within the provider in the next request to
*** FmGetInfoRequest(), and within the worker when the request
*** has been sent out.
**/
static void FmFreeInfoRequestBuffer(void *buffer)
{ Header	*header;
  char		*name = "FmFreeInfoRequestBuffer";

  if (buffer eq NULL)
   internal("NULL buffer passed to %s", name);

  Debug(FmD_Buffers, ("freeing InfoRequest buffer %p", buffer));

  header = (Header *) ((BYTE *) buffer - sizeof_Header);
  if (header->Magic ne Magic_InfoRequest)
   internal("invalid buffer %p passed to %s", buffer, name);

  header->Magic = 0;
  free(header);
}

/**
*** Information reply buffers are normally freed implicitly within both
*** the controller and the worker. However they can be freed by the
*** application from within the worker, if the buffer is no longer needed
*** and there are unlikely to be any future calls to FmGetInfo().
**/
void FmFreeInfoReplyBuffer(void *buffer)
{ Header	*header;
  char		*name = "FmFreeInfoReplyBuffer";

  if (buffer eq NULL)
   fatal("NULL buffer passed to %s", name);

  header = (Header *) ((BYTE *) buffer - sizeof_Header);
  if (header->Magic ne Magic_InfoReply)
   fatal("invalid buffer passed to %s", name);

  Debug(FmD_Buffers, ("freeing InfoReply buffer %p", buffer));

	/* The global variable LastInfoReplyBuffer is used to keep	*/
	/* track of the last reply buffer returned by FmGetInfo().	*/
	/* Normally this buffer would be freed automatically by the	*/
	/* next call to FmGetInfo(), but the application is allowed to	*/
	/* free the buffer early. This variable allows FmGetInfo() to	*/
	/* do the right thing.						*/
  if (header eq LastInfoReplyHeader)
   LastInfoReplyHeader = NULL;

  header->Magic = 0;
  free(header);
}
/*}}}*/
/*}}}*/
/*{{{  worker code */
/*{{{  description */
/**
*** The code running within each worker is as follows:
***  1) there is an input thread responsible for reading data from the
***     controller. This data may be jobs or information replies.
***  2) there is an output thread responsible for sending replies or
***     information requests to the controller.
***  3) the FmGetJob() routine waits for a job to be received by the
***     input thread. It will also free previous job buffers.
***  4) the FmSendReply() routine passes a reply to the output thread
***  5) the FmGetInfo() routine passes the information request on to
***     the output thread and then waits for the reply to be received by
***     the input thread.
***
*** Termination conditions are as follows:
***  1) end-of-file is detected in the input thread. This can mean that
***     the controller has exited normally and is shutting down its
***     pipes, or that the controller has crashed and the exception handling
***     code has detected this.
***
***  2) the worker routine has finished. Note that the output thread may
***     still be handling the last reply packet so synchronisation is
***     required. What happens is that the worker thread signals the
***     RepliesReady semaphore without adding another node to the list,
***     and the output thread detects an empty list.
***
***  3) a communication failure occurs on the output pipe. Conceivably this
***     could be detected before the failure on the input pipe.
**/
/*}}}*/
/*{{{  worker variables */
	/* Control access to the various workers lists			*/
static	Semaphore	WorkerLock;

	/* Jobs are added by the input thread and removed by FmGetJob()	*/
static	List		JobList;

	/* Count of the number of available jobs.			*/
static	Semaphore	JobsReady;

	/* Replies are added by FmSendReply() and consumed by output	*/
	/* thread. Also, information requests are added to this list.	*/
static	List		ReplyList;

	/* Count of number of entries on ReplyList			*/
static	Semaphore	RepliesReady;

	/* Input thread signals when an information reply has been	*/
	/* received.							*/
static	Semaphore	InfoAvailable;

	/* This is the current information request, if any. It is	*/
	/* needed to obtain the reply buffer.				*/
static	Header		*InfoRequest;

	/* This is the information reply that has been received.	*/
static	Header		*InfoReply;

	/* This semaphore is signalled when the worker routine returns,	*/
	/* to allow for clean exiting.					*/
static	Semaphore	WorkerDone;

	/* This is the last job packet returned by FmGetJob()		*/
static	Header		*LastJob;
/*}}}*/
/*{{{  worker input thread */
/**
*** The worker input thread runs continuously reading packets from the
*** controller. It is primarily responsible for handling worker termination
*** when EOF is detected. The worker thread should be blocked forever
*** inside FmGetJob() or FmGetInfo(), and the output thread should be
*** blocked waiting for a reply packet. It is not safe to hang around
*** until the worker thread finishes, as it may be stuck in a loop when
*** the controller already has sufficient data to finish (without
*** waiting for all the replies).
**/

static void	worker_input(void)
{ Header	 header;
  BYTE		*buffer;
  Header	*obtained;
  
  forever
   { unless(full_read(&WorkerInput, (BYTE *)&(header.Size), Header_Transferred))
      break;

     Debug(FmD_Packets, ("receiving packet, size %d", header.Size));

     if (header.Flags & Header_InfoReply)
      buffer = FmGetInfoReplyBuffer(&(InfoRequest->Data[0]), header.Size);
     else
      buffer = FmGetJobBuffer(header.Size);
     obtained = (Header *) (buffer - sizeof_Header);
     memcpy(&(obtained->Size), &(header.Size), Header_Transferred);

     if (header.Size > 0)
      unless(full_read(&WorkerInput, buffer, header.Size))
       { if (header.Flags & Header_InfoReply)
          FmFreeInfoReplyBuffer(buffer);
         else
          FmFreeJobBuffer(buffer);
         break;
       }

     if (header.Flags & Header_InfoReply)
      { Debug(FmD_Packets, ("received InfoReply packet %p", buffer));
	InfoReply = obtained;
        Signal(&InfoAvailable);
      }
     else
      { Debug(FmD_Packets, ("received job packet %p", buffer));
	Wait(&WorkerLock);
        AddTail(&JobList, &(obtained->Node));
        Signal(&WorkerLock);
        Signal(&JobsReady);
      }
   }
  Debug(FmD_Packets, ("end of file detected"));
  Debug(FmD_Exit, ("input thread done"));
	/* At this point end-of-file has been detected and the worker	*/
	/* should exit, even if the worker thread and/or the output	*/
	/* thread is still active.					*/
  Signal(&WorkerDone);
}
/*}}}*/
/*{{{  worker output thread */
/**
*** The worker output thread is responsible for taking reply packets and
*** information request packets and sending them on the controller. If the
*** semaphore indicates that there is a packet to be sent but the list is
*** empty then the worker routine has finished.
**/
static void worker_output(void)
{ Header	*header;

  forever
   { Debug(FmD_Packets, ("waiting for a reply packet to send"));
     Wait(&RepliesReady);

     Wait(&WorkerLock);
     if (EmptyList_(ReplyList))
      header = NULL;
     else
      header = (Header *) RemHead(&ReplyList);
     Signal(&WorkerLock);

     if (header eq NULL) break;	/* worker routine has finished */

     Debug(FmD_Packets, ("sending packet %p, size %d", &(header->Data[0]), header->Size));

     unless(full_write(&WorkerOutput, (BYTE *) &(header->Size), Header_Transferred))
      break;

     if (header->Size > 0)
      unless(full_write(&WorkerOutput, &(header->Data[0]), header->Size))
       break;

     if ((header->Flags & Header_InfoRequest) eq 0)
      FmFreeReplyBuffer(&(header->Data[0]));
   }   

  Debug(FmD_Packets, ("broken output pipe detected"));
  Debug(FmD_Exit, ("output thread done"));
  Signal(&WorkerDone);
}    
/*}}}*/
/*{{{  worker routine */
/**
*** The worker routine always runs in a separate thread. The worker may
*** finish before the output thread has sent the last reply back to the
*** controller. A semaphore is used to detect when the worker routine has 
*** terminated, hence the thread starts executing in the library before 
*** calling the application routine.
**/
static void worker_routine(void)
{ Carrier	*fast_mem	= NULL;

  Debug(FmD_WorkerInit, ("starting worker, fast stack %d, fast code %d", FmFastStack, FmFastCode));

  if (FmFastStack)
   { fast_mem = AllocFast(FmWorkerStack, &(MyTask->MemPool));
     if (fast_mem eq NULL)
      warn("failed to allocate on-chip memory for stack");
   }
  if (FmFastCode)
   if (AccelerateCode(FmWorker) < Err_Null)
    warn("failed to copy worker code to on-chip memory");

  if (fast_mem)
   Accelerate(fast_mem, FmWorker, 0);
  else
   (FmWorker)();

  Debug(FmD_Exit, ("worker routine done"));
  Signal(&RepliesReady);
}
/*}}}*/
/*{{{  FmGetJob() */
/**
*** FmGetJob() simply has to wait on the JobsReady semaphore and then
*** extract the next job from the list. There is no need to worry about
*** EOF conditions.
**/
void *FmGetJob(void)
{ Header	*header;

  unless(InWorker)
   fatal("FmGetJob() routine called from inside the controller");

  if (LastJob ne NULL)
   { FmFreeJobBuffer(&(LastJob->Data[0])); LastJob = NULL; }

  Debug(FmD_Jobs, ("waiting for a job"));
  Wait(&JobsReady);
  Wait(&WorkerLock);
  header = (Header *) RemHead(&JobList);
  Signal(&WorkerLock);
  Debug(FmD_Jobs, ("returning job %p", &(header->Data[0])));
  LastJob = header;
  return(&(header->Data[0]));
}
/*}}}*/
/*{{{  FmSendReply() */
/**
*** The SendReply() routine is also very simple, it just adds the
*** message to the list and signals a semaphore. This will wake up the
*** worker output thread.
**/
void FmSendReply(void *buf)
{ Header	*header;

  unless(InWorker)
   fatal("FmSendReply() called from inside the controller");

  if (buf eq NULL)
   fatal("NULL buffer passed to FmSendReply()");

  header = (Header *) ((BYTE *) buf - sizeof_Header);
  if (header->Magic ne Magic_Reply)
   fatal("invalid buffer %p passed to FmSendReply()", buf);

  Debug(FmD_Replies, ("sending reply %p", buf));
  Wait(&WorkerLock);
  AddTail(&ReplyList, &(header->Node));
  Signal(&WorkerLock);
  Signal(&RepliesReady);
}
/*}}}*/
/*{{{  FmGetInfo() */
/**
*** The FmGetInfo() routine is effectively a combination of FmSendReply()
*** and FmGetJob(), with the complication that it may be necessary to
*** free the previous reply buffer and that it is always necessary to
*** free the request buffer.
**/
void *FmGetInfo(void *buf)
{ Header	*request_header;
  Header	*reply_header;

  unless(InWorker)
   fatal("FmGetInfo() called from inside the controller");

  if (LastInfoReplyHeader ne NULL)
   FmFreeInfoReplyBuffer(&(LastInfoReplyHeader->Data[0]));

  if (buf eq NULL)
   fatal("NULL packet passed to FmGetInfo()");

  request_header = (Header *) ((BYTE *) buf - sizeof_Header);
  if (request_header->Magic ne Magic_InfoRequest)
   fatal("invalid packet %p passed to FmGetInfo()");

	/* This allows the input thread to obtain a reply buffer	*/
  InfoRequest	= request_header;

  Debug(FmD_Info, ("sending info request %p", buf));

  Wait(&WorkerLock);
  AddTail(&ReplyList, &(request_header->Node));
  Signal(&WorkerLock);
  Signal(&RepliesReady);

  Wait(&InfoAvailable);
  Wait(&WorkerLock);
  reply_header	= InfoReply;
  Signal(&WorkerLock);

  InfoRequest		= NULL;
  InfoReply		= NULL;
  FmFreeInfoRequestBuffer(buf);

  LastInfoReplyHeader	= reply_header;
  Debug(FmD_Info, ("returning info reply %p", &(reply_header->Data[0])));
  return(&(reply_header->Data[0]));
}
/*}}}*/
/*{{{  worker start-up code */
/**
*** This routine is called near the end of the FmInitialise() routine
*** to get things up and running. It is responsible for starting things
*** up inside a worker including the input and output threads and the
*** thread that actually runs the application worker routine.
**/
static void worker_startup(void)
{ 
  InitSemaphore(&WorkerLock, 1);
  InitList(&JobList);
  InitSemaphore(&JobsReady, 0);
  InitList(&ReplyList);
  InitSemaphore(&RepliesReady, 0);
  InitSemaphore(&InfoAvailable, 0);
  InfoReply	= NULL;
  InitSemaphore(&WorkerDone, 0);
  LastJob	= NULL;

  if (FmWorkerStack <= 0)
   fatal("illegal worker stack size %d specified", FmWorkerStack);

	/* Perform a sanity check on the stack size		*/
  if (FmWorkerStack < 1000)
   FmWorkerStack = 1000;

  unless(Fork(StackSize, &worker_input, 0) &&
         Fork(StackSize, &worker_output, 0))
   fatal("not enough memory to spawn I/O threads");

  unless(Fork(FmWorkerStack, &worker_routine, 0))
   fatal("not enough memory to spawn worker thread");
}

static void waitfor_worker(void)
{ Wait(&WorkerDone);
  Debug(FmD_Exit, ("worker has finished"));
}
/*}}}*/
/*}}}*/
/*{{{  controller code */
/*{{{  description */
/**
*** Inside the controller program there is a separate thread to manage
*** each worker. Each worker can be in a number of states:
***
***  1) WaitingPrimary, the worker has not received a primary packet and
***	is completely idle. Nothing much will happen until the producer
***	generates a packet that can be sent to this worker. This packet will
***	be placed in the Worker data structure to be sent by the thread.
***
***  2) GotPrimary, the producer routine has generated a primary packet for 
***	this worker and reactivated the worker thread. The packet now has to
***	be transmitted to the worker program.
***
***  3) ProcessingPrimary, the primary packet has been sent to the worker
***	program and the worker thread is now waiting for a reply
***
***  4) SendingSecondary, while the worker program is processing its primary
***	packet and the worker thread is waiting for the reply the producer
***	routine can generate a secondary packet and send this asynchronously.
***	This is a very temporary state.
***
***  5) GotSecondary, the worker program is still processing its primary
***	packet and has received a secondary packet as well.
***
***  6) Info1, the worker program is processing its primary packet and has
***	requested some additional information. It does not have a secondary
***	packet. The worker thread is blocked until the information reply
***	is provided.
***
***  7) Reply1, a reply is available to an information request and is
***	currently being sent. The worker program does not have a secondary
***	packet.
***
***  8) SendingSecondary2, this is a temporary state between Info1 and
***	Info2, when a secondary packet is being sent to a worker task that
***	is in the middle of an Info request.
***
***  9) Info2, similar to Info1 but the worker program has received a
***	secondary packet.
***
*** 10) Reply2, similar to Reply1 but the worker program has received a
***	secondary packet.
***
*** 11) Dead, this worker program is believed to be dead. The worker thread
***	will perform some tidying up and then exit.
***
*** Transitions between the various threads occur for the following reasons:
*** 
***  a) a packet is sent to the worker program
***  b) a packet is received from the worker program
***  c) an exception occurs. This could be an I/O error or the result of the
***     SIGSIB handling.
**/
/*}}}*/
/*{{{  worker data structure */
/**
*** This is the main data structure associated with each worker thread.
**/
typedef struct	Worker {
		/* Worker threads may be on Primary or Secondary lists,	*/
		/* facilitating the allocation of packets to workers.	*/
	Node		 Node;

		/* Identification number for this worker, in the range	*/
		/* 0 <= Number < FmNumberWorkers. In the case of an	*/
		/* overloaded controller the number will be 		*/
		/* FmNumberWorkers - 1, to ensure that the overload	*/
		/* occurs after the other workers have received their	*/
		/* jobs.						*/
	int		 Number;

		/* When the worker thread is waiting for something to	*/
		/* happen inside either the producer or the provider	*/
		/* it will be waiting on this semaphore.		*/
	Semaphore	 Wakeup;

		/* This is the current primary packet being processed	*/
		/* by the worker task. It is held in the controller so	*/
		/* that it may be passed on to another worker should	*/
		/* this one fail.					*/
	Header		*PrimaryPacket;

		/* Similarly for the secondary packet.			*/
	Header		*SecondaryPacket;

		/* This is the current state of this worker task, as	*/
		/* described above.					*/
	int	 	 State;

		/* These two file descriptors are used for		*/
		/* communication with the workers.			*/
	int		 ToWorker;
	int		 FromWorker;

		/* This semaphore is used when the producer needs to	*/
		/* send a packet to this particular worker, e.g. a	*/
		/* broadcast, and needs to know when the channel is	*/
		/* available. The semaphore will typically reside	*/
		/* somewhere on the producer's stack.			*/
	Semaphore	*Sending;

		/* This semaphore is used when the consumer is waiting	*/
		/* for a reply from this particular worker.		*/
	Semaphore	*GotReply;

		/* This holds the information reply packet generated by	*/
		/* the provider in response to an information request.	*/
		/* The worker thread will be blocked on the Wakeup	*/
		/* semaphore.						*/
	Header		*InfoReply;
} Worker;

#define	Worker_WaitingPrimary		 1
#define	Worker_GotPrimary		 2
#define Worker_ProcessingPrimary	 3
#define Worker_SendingSecondary		 4
#define Worker_GotSecondary		 5
#define Worker_Info1			 6
#define Worker_Reply1			 7
#define Worker_SendingSecondary2	 8
#define Worker_Info2			 9
#define Worker_Reply2			10
#define Worker_Dead			11
/*}}}*/
/*{{{  controller variables and prototypes */
/**
*** These variables are private to the controller part of the farm library.
**/

	/* This lock is used to manipulate worker states and the	*/
	/* various linked lists.					*/
static	Semaphore	 ControllerLock;

	/* This semaphore is used for broadcasting, to indicate that a	*/
	/* worker has received a broadcast packet.			*/
static	Semaphore	 Broadcast;

	/* These two are used to handle termination.			*/
static	Semaphore	 ControllerDone;
static	bool		 InFmGetReply;

	/* This list is used to keep track of workers waiting for their	*/
	/* primary packets.						*/
static	List		 PrimaryList;

	/* This list is used to keep track of workers which can	receive	*/
	/* a secondary packet.						*/
static	List		 SecondaryList;

	/* This table holds the various Worker structures.		*/
static	Worker		*WorkerVec;

	/* This function is used to send a specific packet to a		*/
	/* specific worker.						*/
static	bool		fm_SendPacket(Worker *, Header *);

	/* Similarly, a function to receive the next packet from a	*/
	/* worker.							*/
static	Header		*fm_ReceivePacket(Worker *);

	/* This is a dummy packet used for acknowledged broadcasts.	*/
	/* The broadcast packet should be freed inside FmSendJob() when	*/
	/* the packet has been sent on to all workers, but I need to	*/
	/* put a dummy into the Primary or Secondary packet field of	*/
	/* the Worker data structure.					*/
static	Header		DummyHeader;

	/* This list is used to hold outstanding information requests.	*/
static	List		InfoRequests;

	/* This semaphore is used to keep track of the number of	*/
	/* entries in the InfoRequests list.				*/
static	Semaphore	InfoRequestCount;

	/* Similarly a list and a semaphore for reply packets. Note	*/
	/* the choice of names to avoid a clash with the worker		*/
	/* variables.							*/
static	List		C_ReplyList;
static	Semaphore	C_RepliesReady;

	/* This is used to keep track of the last reply header so that	*/
	/* it can be freed in FmGetReply().				*/
static	Header		*LastReply;
/*}}}*/
/*{{{  worker thread management */
/*{{{  WaitingPrimary */
/**
*** In waiting_primary state the worker program currently does not have
*** any packets to process. The worker thread will block on a semaphore until
*** the producer generates a packet for this worker. The packet will be
*** placed in the PrimaryPacket field of the worker structure.
***
*** The state may change to GotPrimary or to Dead.
**/
static void waiting_primary(Worker *worker)
{
  Signal(&ControllerLock);
  Wait(&(worker->Wakeup));
  Wait(&ControllerLock);

  if (worker->State eq Worker_Dead) return;

  if (worker->State ne Worker_GotPrimary)
   internal("worker %d, invalid transition from WaitingPrimary to %d",
		worker->Number, worker->State);
}
/*}}}*/
/*{{{  GotPrimary */
/**
*** In got_primary state the worker has been given a primary packet which
*** should be sent off. The state may change to dead while this packet is
*** sent. If the packet is successfully sent then the new state depends
*** on the nature of the packet:
****
***  a) if the packet should be acknowledged by the worker task then the
***	worker thread should be ready to read it in. This requires a
***	transition to state ProcessingPrimary. The worker thread should
***	move onto the Secondary list to allow a secondary packet to be
***	sent.
***
***  b) if the packet should not be acknowledged then the worker thread
***	should switch back to WaitingPrimary and put itself onto the
***	Primary list.
***
*** There are also various problems relating to free'ing buffers, signalling
*** the Broadcast semaphore, etc.
**/

static void got_primary(Worker *worker)
{ Header	*packet		= worker->PrimaryPacket;
  bool		 result;
  bool		 must_free	= FALSE;

  Signal(&ControllerLock);
  result	= fm_SendPacket(worker, packet);
  Wait(&ControllerLock);

  if ((!result) || (worker->State eq Worker_Dead))
   { worker->State = Worker_Dead; return; }

  if (worker->State ne Worker_GotPrimary)
   internal("worker %d, illegal transition from GotPrimary to %d",
		worker->Number, worker->State);

	/* Acknowledged broadcast packets are freed early and not when	*/
	/* all the replies have been received. It is necessary to	*/
	/* remember that the primary packet was one of these.		*/
  if ((packet->Destination eq Fm_All) && 
      ((packet->Flags & Header_NoAck) eq 0))
   worker->PrimaryPacket	= &DummyHeader;

	/* Unacknowledged jobs mean that the worker thread should	*/
	/* go back to WaitingPrimary. Usually the packet can be freed.	*/
  if (packet->Flags & Header_NoAck)
   { worker->PrimaryPacket	= NULL;
     worker->State		= Worker_WaitingPrimary;
     AddTail(&PrimaryList, &(worker->Node));
     if (packet->Destination ne Fm_All)
      must_free = TRUE; 
   }
  else
   {	/* Acknowledged jobs should be remembered. The worker thread	*/
	/* should switch to reading the reply packet, and be ready to	*/
	/* handle a secondary packet.					*/
     worker->State = Worker_ProcessingPrimary;
     AddTail(&SecondaryList, &(worker->Node));
   }

	/* If some other thread is waiting to send to this worker, e.g.	*/
	/* a broadcast packet, wake it up.				*/
  if (worker->Sending ne NULL)
   { Signal(worker->Sending); worker->Sending = NULL; }

	/* It remains to do some tidying up relating to this particular	*/
	/* packet. If it is a broadcast then the broadcasting thread	*/
	/* should be informed. If the packet is no longer needed then	*/
	/* it should be freed.						*/
  if (packet->Destination eq Fm_All)
   Signal(&Broadcast);

  if (must_free)
   FmFreeJobBuffer(&(packet->Data[0]));
}
/*}}}*/
/*{{{  ProcessingPrimary */
/**
*** When this routine starts running the worker program has received its
*** primary packet and may send a reply at any time. If reading the reply
*** fails then the worker has died. Otherwise things get complete.
*** 
*** While waiting for a reply the worker state may change asynchronously to:
***  1) SendingSecondary, a temporary condition
***  2) GotSecondary
***  3) Dead
***
*** The packet received from the worker task may be an information request
*** or a real reply.
**/

static void processing_primary(Worker *worker)
{ Header	*packet;
  Header	*packet_to_free	= NULL;

  Signal(&ControllerLock);
  packet = fm_ReceivePacket(worker);
  Wait(&ControllerLock);

  while (worker->State eq Worker_SendingSecondary)
   { Signal(&ControllerLock);
     Delay(OneSec / 5);
     Wait(&ControllerLock);
   }

  if ((packet eq NULL) || (worker->State eq Worker_Dead))
   { worker->State = Worker_Dead; return; }

  if ((worker->State ne Worker_ProcessingPrimary) &&
      (worker->State ne Worker_GotSecondary))
   internal("worker thread %d, illegal transition from ProcessingPrimary to %d",
		worker->Number, worker->State);

	/* Information requests mean switching to an Info state and	*/
	/* waking up the provider routine.				*/
  if (packet->Flags & Header_InfoRequest)
   { 
     if (FmProvider eq NULL)
      fatal("worker %d has performed an FmGetInfo() but no provider routine is running.",
		worker->Number);

     if (worker->State eq Worker_ProcessingPrimary)
      worker->State = Worker_Info1;
     else
      worker->State = Worker_Info2;

     AddTail(&InfoRequests, &(packet->Node));
     Signal(&InfoRequestCount);
   }
  else
   {	/* A reply packet has been received, store it on the list	*/
     Debug(FmD_Replies, ("worker %d has sent reply %p", worker->Number, &(packet->Data[0])));
     AddTail(&C_ReplyList, &(packet->Node));
     Signal(&C_RepliesReady);

	/* If a thread is waiting for a reply from this worker, wake	*/
	/* it up.							*/
     if (worker->GotReply ne NULL)
      { Signal(worker->GotReply); worker->GotReply = NULL; }

	/* The job that caused this reply should be freed, unless it	*/
	/* was an acknowledged broadcast.				*/
     if (worker->PrimaryPacket ne &DummyHeader)
      packet_to_free = worker->PrimaryPacket;
     worker->PrimaryPacket = NULL;

	/* The new state depends on whether or not a secondary packet	*/
	/* has been sent already.					*/
     if (worker->State eq Worker_GotSecondary)
      { worker->State		= Worker_ProcessingPrimary;
        worker->PrimaryPacket	= worker->SecondaryPacket;
        worker->SecondaryPacket	= NULL;
        AddTail(&SecondaryList, &(worker->Node));
      }
     else
      { worker->State		= Worker_WaitingPrimary;
        Remove(&(worker->Node));	/* from secondary list	*/
        AddTail(&PrimaryList, &(worker->Node));
      }

	/* If another thread was waiting to send, it can now be woken	*/
	/* up. 								*/
     if (worker->Sending ne NULL)
      { Signal(worker->Sending); worker->Sending = NULL; }
   }

	/* Free the original job buffer, if any.			*/
  if (packet_to_free ne NULL)
   FmFreeJobBuffer(&(packet_to_free->Data[0]));
}
/*}}}*/
/*{{{  SendingSecondary */
/**
*** SendingSecondary is a temporary state that can arise between
*** GotPrimary and ProcessingPrimary: if another packet is sent during
*** this switch over the worker will remain in state SendingSecondary
*** until the packet has been sent, and will switch to state
*** GotSecondary or ProcessingPrimary, depending on the type of the
*** second packet (acknowledged or unacknowledged).
**/
static void sending_secondary(Worker *worker)
{ 
  Signal(&ControllerLock);

  while(worker->State eq Worker_SendingSecondary)
   Delay(OneSec / 5);
  if ((worker->State ne Worker_GotSecondary) &&
      (worker->State ne Worker_ProcessingPrimary) &&
      (worker->State ne Worker_Dead))
   internal("worker thread %d, illegal transition from SendingSecondary to %d",
		worker->Number, worker->State);

  Wait(&ControllerLock);
}
/*}}}*/
/*{{{  GotSecondary */
/**
*** GotSecondary means that the worker task is currently processing its
*** primary packet and has already received a secondary one. The code for
*** handling this state is almost identical to that for ProcessingPrimary,
*** except that fewer transitions are possible.
**/
static void got_secondary(Worker *worker)
{ Header	*packet;
  Header	*packet_to_free	= NULL;

  Signal(&ControllerLock);
  packet = fm_ReceivePacket(worker);
  Wait(&ControllerLock);

  if ((packet eq NULL) || (worker->State eq Worker_Dead))
   { worker->State = Worker_Dead; return; }

  if (worker->State ne Worker_GotSecondary)
   internal("worker thread %d, illegal transition from GotSecondary to %d",
		worker->Number, worker->State);

	/* Information requests mean switching to an Info state and	*/
	/* waking up the provider routine.				*/
  if (packet->Flags & Header_InfoRequest)
   { 
     if (FmProvider eq NULL)
      fatal("worker %d has performed an FmGetInfo() but no provider routine is running",
		worker->Number);

     worker->State = Worker_Info2;
     AddTail(&InfoRequests, &(packet->Node));
     Signal(&InfoRequestCount);
   }
  else
   {	/* A reply packet has been received, store it on the list	*/
     Debug(FmD_Replies, ("worker %d has sent reply %p", worker->Number, &(packet->Data[0])));
     AddTail(&C_ReplyList, &(packet->Node));
     Signal(&C_RepliesReady);

	/* If a thread is waiting for a reply from this worker, wake	*/
	/* it up.							*/
     if (worker->GotReply ne NULL)
      { Signal(worker->GotReply); worker->GotReply = NULL; }

	/* The job that caused this reply should be freed, unless it	*/
	/* was an acknowledged broadcast.				*/
     if (worker->PrimaryPacket ne &DummyHeader)
      packet_to_free = worker->PrimaryPacket;
     worker->PrimaryPacket = NULL;

     worker->State		= Worker_ProcessingPrimary;
     worker->PrimaryPacket	= worker->SecondaryPacket;
     worker->SecondaryPacket	= NULL;
     AddTail(&SecondaryList, &(worker->Node));

	/* If another thread was waiting to send, it can now be woken	*/
	/* up. 								*/
     if (worker->Sending ne NULL)
      { Signal(worker->Sending); worker->Sending = NULL; }
   }
	/* Free the original job buffer, if any.			*/
  if (packet_to_free ne NULL)
   FmFreeJobBuffer(&(packet_to_free->Data[0]));
}
/*}}}*/
/*{{{  Info1 */
/**
*** In info1 state the worker task is waiting for a reply in an FmGetInfo()
*** call. The info request has been put onto the provider's list and
*** the worker thread should block until the provider has processed this
*** packet. A secondary packet may be being sent or may have been sent
*** while this is going on.
***
*** There is a potential conflict between sending a secondary packet and
*** sending the information reply. To avoid this conflict the worker is
*** removed from the Secondary list and added back in in Reply1.
**/
static void info1(Worker *worker)
{
  Signal(&ControllerLock);
  Wait(&(worker->Wakeup));
  Wait(&ControllerLock);

  while (worker->State eq Worker_SendingSecondary2)
   { Signal(&ControllerLock);
     Delay(OneSec / 5);
     Wait(&ControllerLock);
   }

  if (worker->State eq Worker_Dead) return;

  if (worker->State eq Worker_Info1)
   { Remove(&(worker->Node));
     worker->State = Worker_Reply1;
   }
  elif (worker->State eq Worker_Info2)
   worker->State = Worker_Reply2;
  else
   internal("worker thread %d, illegal state transition from Info1 to %d",
		worker->Number, worker->State);
}
/*}}}*/
/*{{{  Reply1 */
/**
*** In state reply1 the situation is as follows:
***  1) the worker task is processing its primary packet
***  2) it has performed a GetInfo call and is waiting for the reply
***  3) the reply is in the InfoReply field of the Worker structure
***  4) to avoid a collision between sending a secondary packet and
***     the information reply, the worker thread has been removed from
***     the secondary list.
**/
static void reply1(Worker *worker)
{ Header	*header;
  bool		 result;

  header		= worker->InfoReply;
  worker->InfoReply	= NULL;

  if ((header eq NULL) || (header->Magic ne Magic_InfoReply))
   internal("worker thread %d, Reply1, illegal info reply packet %p",
		worker->Number, header);

  Signal(&ControllerLock);
  result = fm_SendPacket(worker, header);
  FmFreeInfoReplyBuffer(&(header->Data[0]));
  Wait(&ControllerLock);

  if ((result eq FALSE) || (worker->State eq Worker_Dead))
   { worker->State = Worker_Dead; return; }

  if (worker->State ne Worker_Reply1)
   internal("worker thread %d, illegal transition from Reply1 to %d",
		worker->Number, worker->State);

  worker->State = Worker_ProcessingPrimary;
  AddTail(&SecondaryList, &(worker->Node));
}
/*}}}*/
/*{{{  SendingSecondary2 */
/**
*** There is a window of opportunity when an information request is
*** received and switching from ProcessingPrimary to Info1 in which
*** the secondary packet can be sent. This is a temporary state which
*** should resolve itself to either Info2 or Info1, depending on
*** whether or not the secondary packet is acknowledged.
**/
static void sending_secondary2(Worker *worker)
{ 
  Signal(&ControllerLock); 
  while (worker->State eq Worker_SendingSecondary2)
   Delay(OneSec / 5);
  Wait(&ControllerLock);

  if ((worker->State ne Worker_Dead) &&
      (worker->State ne Worker_Info1) &&
      (worker->State ne Worker_Info2))
   internal("worker thread %d, illegal transition from SendingSecondary2 to %d",
		worker->Number, worker->State);
}
/*}}}*/
/*{{{  Info2 */
/**
*** Info2 state is almost identical to Info1. The worker task is processing
*** its primary packet and has received a secondary one. While processing
*** the primary one the worker has performed an FmGetInfo() call and the
*** information request has been received by the worker thread and has been
*** put onto the provider's list of packets. The worker thread should block
*** until the info reply is available.
**/
static void info2(Worker *worker)
{
  Signal(&ControllerLock);
  Wait(&(worker->Wakeup));
  Wait(&ControllerLock);

  if (worker->State eq Worker_Dead) return;

  if (worker->State ne Worker_Info2)
   internal("worker thread %d, illegal state transition from Info2 to %d",
		worker->Number, worker->State);

  worker->State = Worker_Reply2;
}
/*}}}*/
/*{{{  Reply2 */
/**
*** In state Reply2 the situation is slightly simpler than in Reply1.
*** The worker task has received its primary and secondary packets and
*** is currently in a call to FmGetInfo().
**/
static void reply2(Worker *worker)
{ Header	*header	= worker->InfoReply;
  bool		 result;

  if ((header eq NULL) || (header->Magic ne Magic_InfoReply))
   internal("worker thread %d, Reply2, illegal info reply packet %p",
		worker->Number, header);

  Signal(&ControllerLock);
  result = fm_SendPacket(worker, header);
  FmFreeInfoReplyBuffer(&(header->Data[0]));
  Wait(&ControllerLock);

  if ((result eq FALSE) || (worker->State eq Worker_Dead))
   { worker->State = Worker_Dead; return; }

  if (worker->State ne Worker_Reply2)
   internal("worker thread %d, illegal transition from Reply2 to %d",
		worker->Number, worker->State);

  worker->State = Worker_GotSecondary;
}
/*}}}*/
/*{{{  Dead */
/**
*** When a worker task had died it is necessary to take some recovery
*** action:
***  1) the count of the number of running workers is decremented
***  2) the user is given a warning, which may of course be suppressed
***  3) any outstanding I/O to that worker is aborted, in case
***	some other thread is still trying to send a packet
***  4) the primary and secondary packets, if any, are analysed. In some
***	cases these packets can be sent to other workers. In other cases
***	the packets should be freed.
***  5) any other threads waiting on this worker should be woken up.
***  6) the worker thread should halt.
***
*** Not too much care has to be taken about locking. The rest of the library
*** checks whether or not a worker is dead wherever appropriate and takes
*** suitable action.
**/
static void dead(Worker *worker)
{ Header	*packet;
  Stream	*stream;

  FmRunningWorkers--;

  Signal(&ControllerLock);

  warn("worker %d has gone", worker->Number);

  if (worker->ToWorker >= 0)
   { stream = fdstream(worker->ToWorker);
     worker->ToWorker = -1;
     if (stream ne NULL)
      Abort(stream);
   }

  if (worker->FromWorker >= 0)
   { stream = fdstream(worker->FromWorker);
     worker->FromWorker = -1;
     if (stream ne NULL)
      Abort(stream);
   }

  if (worker->PrimaryPacket ne NULL)
   { packet			= worker->PrimaryPacket;
     worker->PrimaryPacket	= NULL;

	/* case 1: a broadcast packet that has been sent.	*/
     if (packet eq &DummyHeader)
      {	/* no work required */
      }
	/* case 2: a broadcast packet that not yet been sent.	*/
	/* Wake up the thread doing the broadcasting.		*/
     elif (packet->Destination eq Fm_All)
      { Signal(&Broadcast);
      }
	/* case 3: a packet addressed to this particular worker	*/
	/* No recovery possible, just free it.			*/
     elif (packet->Destination ne Fm_Any)
      { FmFreeJobBuffer(&(packet->Data[0]));
      }
	/* case 4: destination Fm_Any, find another free worker	*/
     else
      { FmSendJob(Fm_Any, TRUE, &(packet->Data[0]));
      }
   }

  if (worker->SecondaryPacket ne NULL)
   { packet			= worker->SecondaryPacket;
     worker->SecondaryPacket	= NULL;

	/* case 1: a broadcast packet that has been sent.	*/
     if (packet eq &DummyHeader)
      {	/* no work required */
      }
	/* case 2: a broadcast packet that not yet been sent.	*/
	/* Wake up the thread doing the broadcasting.		*/
     elif (packet->Destination eq Fm_All)
      { Signal(&Broadcast);
      }
	/* case 3: a packet addressed to this particular worker	*/
	/* No recovery possible, just free it.			*/
     elif (packet->Destination ne Fm_Any)
      { FmFreeJobBuffer(&(packet->Data[0]));
      }
	/* case 4: destination Fm_Any, find another free worker	*/
     else
      { FmSendJob(Fm_Any, TRUE, &(packet->Data[0]));
      }
   }

  if (worker->InfoReply ne NULL)
   FmFreeInfoReplyBuffer(&(worker->InfoReply->Data[0]));

  Wait(&ControllerLock);
  if (worker->Sending ne NULL)
   { Signal(worker->Sending); worker->Sending = NULL; }
  if (worker->GotReply ne NULL)
   { Signal(worker->GotReply); worker->GotReply = NULL; }
  Signal(&ControllerLock);

  forever
   Delay(20 * 60 * OneSec);
}
/*}}}*/

/*{{{  worker_thread() */
static void worker_thread(int x)
{ Worker	*worker	= &(WorkerVec[x]);

  Wait(&ControllerLock);

  forever
   { Debug(FmD_State, ("worker %d is now in state %d", x, worker->State));
     switch(worker->State)
      { case Worker_WaitingPrimary	: waiting_primary(worker);	break;
        case Worker_GotPrimary		: got_primary(worker);		break;
        case Worker_ProcessingPrimary	: processing_primary(worker);	break;
        case Worker_SendingSecondary	: sending_secondary(worker);	break;
        case Worker_GotSecondary	: got_secondary(worker);	break;
        case Worker_Info1		: info1(worker);		break;
        case Worker_Reply1		: reply1(worker);		break;
        case Worker_SendingSecondary2	: sending_secondary2(worker);	break;
        case Worker_Info2		: info2(worker);		break;
        case Worker_Reply2		: reply2(worker);		break;
        case Worker_Dead		: dead(worker);			break;

        default :
	  internal("worker %d is in an invalid state %d", x, worker->State);
      }
  }
  internal("worker %d has broken out of its loop", x);
}
/*}}}*/
/*{{{  FmIsRunning() */

/**
*** This is a routine for applications that want to find out the
*** current state of the taskforce.
**/
bool FmIsRunning(int worker)
{
  if ((worker < 0) || (worker >= FmNumberWorkers))
   fatal("invalid argument %d passed to FmIsRunning()", worker);

  if (WorkerVec[worker].State eq Worker_Dead)
   return(FALSE);
  else
   return(TRUE);
}
/*}}}*/
/*{{{  kill_worker() */
/**
*** This routine is used to clean up when a worker has died. It can be
*** killed from inside the signal handling code or from inside the
*** termination detection code.
**/
static void kill_worker(int number)
{ int		 fd;
  Worker	*worker;

  Debug(FmD_Signals, ("in kill_worker for worker %d", number));

  worker = &(WorkerVec[number]);

  Wait(&ControllerLock);

  if (worker->State ne Worker_Dead)
   { worker->State = Worker_Dead;

     fd = worker->ToWorker;
     worker->ToWorker = -1;
     if (fd >= 0) Abort(fdstream(fd));

     fd = worker->FromWorker;
     worker->FromWorker	= -1;
     if (fd >= 0) Abort(fdstream(fd));
   }

  Signal(&ControllerLock);
}
/*}}}*/
/*}}}*/
/*{{{  send and receive functions */
/*{{{  fm_SendPacket() */
/**
*** This routine sends a single packet to a particular worker. It is safe
*** to write directly to the pipe. The routine simply returns success or
*** failure.
**/
static bool fm_SendPacket(Worker *worker, Header *header)
{
  Debug(FmD_Packets, ("worker %d, sending packet %p", worker->Number, &(header->Data[0])));

  unless(full_write(&(worker->ToWorker), (BYTE *) &(header->Size), Header_Transferred))
   return(FALSE);

  if (header->Size > 0)
   unless(full_write(&(worker->ToWorker), (BYTE *) &(header->Data[0]), header->Size))
    return(FALSE);

  return(TRUE);
}
/*}}}*/
/*{{{  fm_ReceivePacket() */
/**
*** This routine reads in a single packet from a given worker. It returns
*** the packet on success, NULL on failure.
**/
static Header	*fm_ReceivePacket(Worker *worker)
{ Header	 header;
  BYTE		*buffer;
  Header	*obtained;

  unless(full_read(&(worker->FromWorker), (BYTE*) &(header.Size), Header_Transferred))
   return(NULL);

  Debug(FmD_Packets, ("worker %d, receiving packet of %d bytes", worker->Number, header.Size));

  if (header.Flags & Header_InfoRequest)
   buffer = FmGetInfoRequestBuffer(header.Size);
  else
   buffer = FmGetReplyBuffer(worker->PrimaryPacket, header.Size);

  obtained = (Header *) (buffer - sizeof_Header);
  memcpy(&(obtained->Size), &(header.Size), Header_Transferred);
  obtained->Destination = worker->Number;

  if (header.Size > 0)
   unless(full_read(&(worker->FromWorker), buffer, header.Size))
    { if (header.Flags & Header_InfoRequest)
       FmFreeInfoRequestBuffer(buffer);
      else
       FmFreeReplyBuffer(buffer);
      return(NULL);
    }
  return(obtained);
}
/*}}}*/
/*{{{  fm_SendSecondary() */
/**
*** This routine is used to send a secondary packet to a particular
*** worker. It will be called with the ControllerLock and should return
*** with the ControllerLock, although this lock should be released while
*** the packet is being sent.
*** 
*** Much of this code is very similar to that in GotPrimary()
*** above.
**/
static void fm_SendSecondary(Worker *worker, Header *packet)
{ int	old_state	= worker->State;
  bool	result;
  bool	must_free	= FALSE;

  Debug(FmD_Packets, ("worker %d, sending secondary packet %p", worker->Number, &(packet->Data[0])));

	/* The worker should only be on the secondary list if in one of	*/
	/* these two states.						*/
  if ((old_state ne Worker_ProcessingPrimary) &&
      (old_state ne Worker_Info1))
   internal("worker thread %d, illegal call to fm_SendSecondary while state is %d",
		worker->Number, old_state);

	/* Update the state to a temporary one. Note that this routine	*/
	/* is called asynchronously from the worker thread and the	*/
	/* latter may wake up at any time.				*/
  if (old_state eq Worker_ProcessingPrimary)
   worker->State = Worker_SendingSecondary;
  else
   worker->State = Worker_SendingSecondary2;

	/* Store the packet in case the worker crashes.			*/
  worker->SecondaryPacket = packet;

	/* Take the worker off the secondary list, to prevent further	*/
	/* packets being sent.						*/
  Remove(&(worker->Node));

	/* At this point it should be safe to release the lock and	*/
	/* try to send the packet.					*/
  Signal(&ControllerLock);

  result = fm_SendPacket(worker, packet);

	/* Reclaim the lock before any further manipulation of the	*/
	/* worker states.						*/
  Wait(&ControllerLock);

  if ((worker->State eq Worker_Dead) || (!result))
   { worker->State = Worker_Dead;
     return;
   }

	/* Check for illegal transitions.				*/
  unless (((old_state eq Worker_ProcessingPrimary) &&
	   (worker->State eq Worker_SendingSecondary)) ||
          ((old_state eq Worker_Info1) &&
           (worker->State eq Worker_SendingSecondary2)))
   internal("worker thread %d, illegal transition to %d inside fm_SendSecondary",
		worker->Number, worker->State);

	/* Work out what to do with the secondary packet, etc.		*/
  if ((packet->Destination eq Fm_All) &&
      ((packet->Flags & Header_NoAck) eq 0))
   worker->SecondaryPacket = &DummyHeader;

  if (packet->Flags & Header_NoAck)
   { worker->SecondaryPacket	= NULL;
     worker->State		= old_state;
     AddTail(&(SecondaryList), &(worker->Node));
     if (packet->Destination ne Fm_All)
      must_free = TRUE;

     if (worker->Sending ne NULL)
      { Signal(worker->Sending); worker->Sending = NULL; }
   }
  else
   { if (old_state eq Worker_ProcessingPrimary)
      worker->State = Worker_GotSecondary;
     else
      worker->State = Worker_Info2;
   }

  if (packet->Destination eq Fm_All)
   Signal(&Broadcast);

  if (must_free)
   FmFreeJobBuffer(&(packet->Data[0]));
}
/*}}}*/
/*{{{  fm_SendAll() */
/**
*** This code is responsible for broadcasting packets to all the workers.
*** At any one time there can be only one broadcast in progress, as the
*** call to FmSendJob() will block until the packet has been passed on to
*** all the workers.
***
*** The packet is sent strictly in order of the workers. This is not
*** optimal but it makes the implementation a lot easier. The action
*** taken depends on the current worker state.
***   Dead		no action can be taken
***   WaitingPrimary	the state is switched to GotPrimary and the worker
***			thread is woken up.
***   GotPrimary
***   GotSecondary
***   Info2
***   Reply1
***   Reply2		the routine blocks on the Sending semaphore, until
***			the worker task is ready for another packet
***   ProcessingPrimary
***   Info1		the packet is sent as a secondary packet
***   SendingSecondary
***   SendingSecondary2	the routine waits while the worker thread resolves
***			itself
***
*** Note that there can other packets being sent in parallel with this
*** broadcast. Although the producer routine will be blocked until the
*** broadcast has finished, more packets can be sent by the worker threads
*** should a worker crash. Also, the provider routine is running in parallel.
**/
static	void	fm_SendAll(Header *packet)
{ int		 i;
  Worker	*worker;
  bool		 sent;
  Semaphore	 wait;

  Debug(FmD_Packets, ("broadcast packet %p", &(packet->Data[0])));

  InitSemaphore(&Broadcast, 0);

  Wait(&ControllerLock);

  for (i = 0; i < FmNumberWorkers; i++)
   { worker	= &(WorkerVec[i]);
     for (sent = FALSE; !sent; )
      switch(worker->State)
       { case Worker_WaitingPrimary :
			worker->PrimaryPacket	= packet;
			worker->State		= Worker_GotPrimary;
			Remove(&(worker->Node));
			Signal(&(worker->Wakeup));
			sent			= TRUE;
			break;

	 case Worker_GotPrimary		:
	 case Worker_GotSecondary	:
	 case Worker_Info2		:
	 case Worker_Reply1		:
	 case Worker_Reply2		:
			InitSemaphore(&wait, 0);
			worker->Sending	= &wait;
			Signal(&ControllerLock);
			Wait(&wait);
			Wait(&ControllerLock);
			break;

	 case Worker_ProcessingPrimary	:
	 case Worker_Info1		:
			fm_SendSecondary(worker, packet);
			sent	= TRUE;
			break;

	 case Worker_SendingSecondary	:
	 case Worker_SendingSecondary2	:
			Signal(&ControllerLock);
			Delay(OneSec / 5);
			Wait(&ControllerLock);
			break;

	 case Worker_Dead		:
			Signal(&Broadcast);
			sent = TRUE;
			break;
       }
   }

	/* At this point the broadcast packet has been handed on to	*/
	/* all the workers, although it may not actually have been sent	*/
	/* yet. The broadcast semaphore is used to determine when the	*/
	/* packets have really been sent out.				*/
  Signal(&ControllerLock);
  for (i = 0; i < FmNumberWorkers; i++)
   { Debug(FmD_Packets, ("broadcast, waiting for worker %d", i));
     Wait(&Broadcast);
   }

  Debug(FmD_Packets, ("broadcast done"));

	/* The buffer can now be freed.					*/
  FmFreeJobBuffer(&(packet->Data[0]));
}
/*}}}*/
/*{{{  fm_SendOne() */
/**
*** This code is responsible for sending a packet to a specific worker.
*** The various tests required are very similar to those for fm_SendAll().
**/
static void fm_SendOne(Header *packet)
{ Worker	*worker	= &(WorkerVec[packet->Destination]);
  bool		 sent	= FALSE;
  Semaphore	 wait;

  Debug(FmD_Packets, ("sending packet %p to specific worker %d", &(packet->Data[0]), worker->Number));

  Wait(&ControllerLock);

  while (!sent)
   switch(worker->State)
    { case Worker_WaitingPrimary	:
			worker->PrimaryPacket	= packet;
			worker->State		= Worker_GotPrimary;
			Remove(&(worker->Node));
			Signal(&(worker->Wakeup));
			sent			= TRUE;
			break;

      case Worker_GotPrimary		:
      case Worker_GotSecondary		:
      case Worker_Info2			:
      case Worker_Reply1		:
      case Worker_Reply2		:
			InitSemaphore(&wait, 0);
			worker->Sending	= &wait;
			Signal(&ControllerLock);
			Wait(&wait);
			Wait(&ControllerLock);
			break;

      case Worker_ProcessingPrimary	:
      case Worker_Info1			:
			fm_SendSecondary(worker, packet);
			sent	= TRUE;
			break;

      case Worker_SendingSecondary	:
      case Worker_SendingSecondary2	:
			Signal(&ControllerLock);
			Delay(OneSec / 5);
			Wait(&ControllerLock);
			break;

      case Worker_Dead		:
			sent = TRUE;
			break;
    }

  Signal(&ControllerLock);
}
/*}}}*/
/*{{{  fm_SendAny() */
/**
*** Effectively this is the load-balancing code, responsible for sending
*** a packet to the next free worker. The code loops looking for a worker
*** which can accept either a primary or secondary packet, aborting if
*** there are no more running workers.
**/
static void fm_SendAny(Header *packet)
{ bool		 sent	= FALSE;
  Worker	*worker;

  Debug(FmD_Packets, ("sending packet %p to next free worker", &(packet->Data[0])));

  while (!sent)
   { Wait(&ControllerLock);

     if (FmRunningWorkers eq 0)
      fatal("all workers have failed");

     if (!EmptyList_(PrimaryList))
      { worker			= (Worker *) RemHead(&PrimaryList);
	if (worker->State ne Worker_WaitingPrimary)
 	 internal("worker thread %d is on the primary list while in state %d",
		worker->Number, worker->State);
        worker->PrimaryPacket	= packet;
	worker->State		= Worker_GotPrimary;
	Signal(&(worker->Wakeup));
	sent			= TRUE;
      }
     elif (!EmptyList_(SecondaryList))
      { worker			= (Worker *) RemHead(&SecondaryList);
	if ((worker->State ne Worker_ProcessingPrimary) &&
	    (worker->State ne Worker_Info1))
	 internal("worker thread %d is on the secondary list while in state %d",
		worker->Number, worker->State);
	fm_SendSecondary(worker, packet);
	sent			= TRUE;
      }

     Signal(&ControllerLock);
     unless(sent) Delay(OneSec / 10);
   }

  Debug(FmD_Packets, ("selected worker %d for packet %p", worker->Number, &(packet->Data[0])));
}
/*}}}*/
/*{{{  FmSendJob() */
/**
*** The FmSendJob() routine is relatively simple, since the bulk of the
*** work is done in the routines above.
**/
void FmSendJob(int where, bool ack, void *buffer)
{ Header	*header;

  unless(InControl)
   fatal("FmSendJob() called from inside a worker");

  if (buffer eq NULL)
   fatal("NULL packet passed to FmSendJob()");

  header = (Header *) ((BYTE *)buffer - sizeof_Header);
  unless(header->Magic eq Magic_Job)
   fatal("invalid job buffer %p passed to FmSendJob()", header);

  Debug(FmD_Jobs, ("in FmSendJob() with packet %p", buffer));

  if (ack)
   header->Flags &= ~Header_NoAck;
  else
   header->Flags |= Header_NoAck;

  header->Destination = where;

  if (where eq Fm_All)
   fm_SendAll(header);
  elif (where eq Fm_Any)
   fm_SendAny(header);
  else
   { if ((where < 0) || (where >= FmNumberWorkers))
      fatal("illegal destination %d passed to FmSendJob()", where);
     fm_SendOne(header);
   }     
}
/*}}}*/
/*{{{  FmGetReply() */
/**
*** The FmGetReply() routine is slightly trickier. There are two main cases
*** to consider.
***  1) if the requested source is Fm_Any then the routine should simply
***     wait until there is a packet in the C_ReplyList and return that one.
***  2) if a particular worker has been specified then life becomes trickier.
***     a) the worker id has to be validated
***     b) the list of existing replies is checked for a packet from that
***        worker
***     c) if the worker is dead then the routine fails
***     d) the routine now blocks itself on the GotReply semaphore for that
***        worker and waits for a packet to arrive.
***     e) the list of existing replies is checked again
***     f) if the worker is now dead then the routine fails
***     g) otherwise an internal error, the GotReply semaphore should not
***        have been signalled.
**/
void	*FmGetReply(int where)
{ Header	*packet;
  Worker	*worker;
  Semaphore	 wait;

  if (LastReply ne NULL)
   { FmFreeReplyBuffer(&(LastReply->Data[0])); LastReply = NULL; }

  if (where eq Fm_Any)
   { InFmGetReply = TRUE;
     Debug(FmD_Replies, ("FmGetReply(), waiting for a reply from any worker"));
     Wait(&C_RepliesReady);
     InFmGetReply = FALSE;
     Wait(&ControllerLock);
     packet = (Header *) RemHead(&C_ReplyList);
     Signal(&ControllerLock);
     Debug(FmD_Replies, ("FmGetReply(), returning packet %p", &(packet->Data[0])));
     LastReply = packet;
     return(&(packet->Data[0]));
   }

  if ((where < 0) || (where >= FmNumberWorkers))
   fatal("FmGetReply() has been given an invalid source %d", where);

  worker = &(WorkerVec[where]);

  Debug(FmD_Replies, ("FmGetReply(), waiting for a reply from worker %d", where));

  Wait(&ControllerLock);

  for (packet = Head_(Header, C_ReplyList);
       !EndOfList_(packet);
       packet = Next_(Header, packet))
   if (packet->Destination eq where)
    { Remove(&(packet->Node));
      Signal(&ControllerLock);
      Wait(&C_RepliesReady);
      Debug(FmD_Replies, ("FmGetReply(), packet %p was already available", &(packet->Data[0])));
      LastReply = packet;
      return(&(packet->Data[0]));
    }

  if (worker->State eq Worker_Dead)
   { Signal(&ControllerLock);
     Debug(FmD_Replies, ("FmGetReply(), specified worker is dead"));
     return(NULL);
   }

  InitSemaphore(&wait, 0);
  worker->GotReply = &wait;
  Signal(&ControllerLock);
  InFmGetReply = TRUE;
  Wait(&wait);
  InFmGetReply = FALSE;
  Debug(FmD_Replies, ("FmGetReply(), worker %d has replied", where));
  Wait(&ControllerLock);

  for (packet = Head_(Header, C_ReplyList);
       !EndOfList_(packet);
       packet = Next_(Header, packet))
   if (packet->Destination eq where)
    { Remove(&(packet->Node));
      Signal(&ControllerLock);
      Wait(&C_RepliesReady);
      Debug(FmD_Replies, ("FmGetReply(), returning packet %p", &(packet->Data[0])));
      LastReply = packet;
      return(&(packet->Data[0]));
    }

  if (worker->State eq Worker_Dead)
   { Signal(&ControllerLock);
     Debug(FmD_Replies, ("FmGetReply(), worker %d has died", where));
     return(NULL);
   }

  internal("FmGetReply(), failed to find the right packet");
  return(NULL);
}
/*}}}*/
/*{{{  FmGetInfoRequest() */
/**
*** The FmGetInfoRequest() is very simple, all that is needed is to wait
*** for an information request to be received and extract it from the list.
*** However, it is necessary to keep track of the previous information
*** request so that it can be freed.
**/
void	*FmGetInfoRequest(void)
{ static	Header	*header	= NULL;

  unless(InControl)
   fatal("FmGetInfoRequest() called outside the controller");

  if (FmProvider eq NULL)
   fatal("FmGetInfoRequest() called when no provider routine has been specified");

  Debug(FmD_Info, ("waiting for an info request"));

  if (header ne NULL)
   { FmFreeInfoRequestBuffer(&(header->Data[0]));
     header = NULL;
   }

  Wait(&InfoRequestCount);
  Wait(&ControllerLock);
  if (EmptyList_(InfoRequests))
   internal("unexpected empty InfoRequests list");
  header	= (Header *) RemHead(&InfoRequests);
  Signal(&ControllerLock);
  Debug(FmD_Info, ("returning info request %p", &(header->Data[0])));
  return(&(header->Data[0]));
}
/*}}}*/
/*{{{  FmSendInfoReply() */
/**
*** The FmSendInfoReply() function is called from inside the FmProvider()
*** routine. The code should identify the worker that requested the
*** information (this should be held in the message), validate the state
*** of that worker, and wake up the worker thread.
**/
void FmSendInfoReply(void *buffer)
{ Header	*header;
  Worker	*worker;

  if (buffer eq NULL)
   fatal("NULL packet passed to FmSendInfoReply()");

  header = (Header *) ((BYTE *) buffer - sizeof_Header);
  if (header->Magic ne Magic_InfoReply)
   fatal("invalid packet %p passed to FmSendInfoReply()", buffer);

  if ((header->Destination < 0) || (header->Destination >= FmNumberWorkers))
   internal("FmSendInfoReply, invalid destination in packet");

  Debug(FmD_Info, ("sending info reply %p to worker %d", buffer, header->Destination));

  Wait(&ControllerLock);
  worker = &(WorkerVec[header->Destination]);

  while (worker->State eq Worker_SendingSecondary2)
   { Signal(&ControllerLock);
     Delay(OneSec / 5);
     Wait(&ControllerLock);
   }

  if (worker->State eq Worker_Dead)
   { FmFreeInfoReplyBuffer(buffer);
     Signal(&ControllerLock);
     Debug(FmD_Info, ("cannot send info reply, worker %d has died", header->Destination));
     return;
   }

  if ((worker->State ne Worker_Info1) && (worker->State ne Worker_Info2))
   internal("FmSendInfoReply(), worker %d is not waiting for a reply",
		header->Destination);

  worker->InfoReply	= header;
  Signal(&(worker->Wakeup));
  Signal(&ControllerLock);
}   
/*}}}*/
/*}}}*/
/*{{{  controller thread management */
/*{{{  consumer thread */
/**
*** If the consumer routine finishes then the whole application finishes,
*** irrespective of what any of the other threads are doing.
**/
static void consumer_thread(void)
{ (*FmConsumer)();
  Debug(FmD_Exit, ("consumer done"));
  Signal(&ControllerDone);
}
/*}}}*/
/*{{{  producer thread */
/**
*** If the producer routine finishes then life becomes rather more
*** difficult. The condition for termination is unfortunately rather
*** complicated.
***
***    1) all the workers must be idle. This means that they must be
***       dead or WaitingPrimary. In any other state they still have at least
***       one packet to process.
***
***    2) the consumer routine must be inside a call to FmGetReply(). A flag
***       is used to keep track of this.
***
*** Note that even after the producer finishes a worker may receive additional
*** jobs, courtesy of crashed processors and jobs being resent. Hence using
*** a semaphore to keep track of idle workers is probably a bad idea.
*** A simple polling loop is used instead.
**/
static void producer_thread(void)
{ int	i;
  bool	all_done;

  (*FmProducer)();

  Debug(FmD_Exit, ("producer done"));
  for (all_done = FALSE; !all_done; )
   { 
     Delay(2 * OneSec);

     if (!InFmGetReply) continue;

     for (i = 0; i < FmNumberWorkers; i++)
      if ((WorkerVec[i].State ne Worker_WaitingPrimary) &&
          (WorkerVec[i].State ne Worker_Dead))
       break;

     if (i eq FmNumberWorkers)
      all_done = TRUE;
   }
  Debug(FmD_Exit, ("all workers done"));
  Signal(&ControllerDone);
}
/*}}}*/
/*{{{  provider thread */
/**
*** The provider thread is only started up if a provider routine has been
*** specified. The only problem occurs if there are info requests after
*** the provider routine has returned. Note that the worker thread checks
*** for a NULL FmProvider routine when it receives an FmInfoRequest.
**/
static void provider_thread(void)
{ 
  (*FmProvider)();

  Debug(FmD_Exit, ("provider done"));
  Wait(&ControllerLock);
  FmProvider = NULL;

  if (!EmptyList_(InfoRequests))
   fatal("the provider routine has returned when there outstanding info requests");

  Signal(&ControllerLock);
}
/*}}}*/
/*{{{  start_threads() */
/**
*** This routine is responsible for starting up the various threads inside
*** the controller. At this point all the worker tasks should be up and
*** running and the pipes should have been set up. Hence a lot of memory
*** has been used up already, and a memory allocation failure at this
*** point is a distinct failure. The worker threads are started up first,
*** because these all start up in state WaitingPrimary and hence will be
*** idle. These are followed by the provider, consumer, and producer threads.
**/
static bool controller_start_threads(void)
{ int	i;

  Debug(FmD_ControlInit, ("starting various controller threads"));

  for (i = 0; i < FmNumberWorkers; i++)
   unless(Fork(StackSize, &worker_thread, sizeof(int), i))
    return(FALSE);

  Debug(FmD_ControlInit, ("worker threads are running"));

  if (FmProvider ne NULL)
   { if (FmProviderStack <= 0)
      fatal("invalid value %d for FmProviderStack", FmProviderStack);
     if (FmProviderStack < 1000)
      FmProviderStack = 1000;
     unless(Fork(FmProviderStack, &provider_thread, 0))
      return(FALSE);
     Debug(FmD_ControlInit, ("provider routine is running"));
   }

  if (FmConsumerStack <= 0)
   fatal("invalid value %d for FmConsumerStack", FmConsumerStack);
  if (FmConsumerStack < 1000)
   FmConsumerStack = 1000;
  unless(Fork(FmConsumerStack, &consumer_thread, 0))
   return(FALSE);

  Debug(FmD_ControlInit, ("consumer routine is running"));

  if (FmProducerStack <= 0)
   fatal("invalid value %d for FmProducerStack", FmProducerStack);
  if (FmProducerStack < 1000)
   FmProducerStack = 1000;
  unless(Fork(FmProducerStack, &producer_thread, 0))
   return(FALSE);

  Debug(FmD_ControlInit, ("producer routine is running"));
  return(TRUE);
}
/*}}}*/
/*{{{  waitfor_controller() */
static void waitfor_controller(void)
{ Wait(&ControllerDone);
  Debug(FmD_Exit, ("controller done"));
}
/*}}}*/
/*}}}*/
/*{{{  controller data initialisation */
/**
*** This routine is responsible for initialising the various statics
*** associated with the controller code and for allocating and initialising
*** the WorkerVec structure.
**/
static void initialise_controller_data(void)
{ int	i;

  Debug(FmD_ControlInit, ("initialising controller data with %d workers", FmNumberWorkers));

  InitSemaphore(&ControllerLock, 1);
  InitSemaphore(&Broadcast, 0);
  InitSemaphore(&ControllerDone, 0);
  InFmGetReply = FALSE;
  InitList(&PrimaryList);
  InitList(&SecondaryList);
  InitList(&InfoRequests);
  InitSemaphore(&InfoRequestCount, 0);
  InitList(&C_ReplyList);
  InitSemaphore(&C_RepliesReady, 0);
  LastReply	= NULL;

  WorkerVec = (Worker *) malloc(FmNumberWorkers * sizeof(Worker));
  if (WorkerVec eq NULL)
   fatal("not enough memory to initialise library");

  for (i = 0; i < FmNumberWorkers; i++)
   { WorkerVec[i].Number		= i;
     InitSemaphore(&(WorkerVec[i].Wakeup), 0);
     WorkerVec[i].PrimaryPacket		= NULL;
     WorkerVec[i].SecondaryPacket	= NULL;
     WorkerVec[i].State			= Worker_WaitingPrimary;
     WorkerVec[i].ToWorker		= (FirstWorkerPipe + 1 + i + i);
     WorkerVec[i].FromWorker		= (FirstWorkerPipe + i + i);
     WorkerVec[i].Sending		= NULL;
     WorkerVec[i].GotReply		= NULL;
     WorkerVec[i].InfoReply		= NULL;
     AddTail(&PrimaryList, &(WorkerVec[i].Node));
   }

  if (FmOverloadController)
   { WorkerVec[FmNumberWorkers - 1].ToWorker	= OverloadToWorker;
     WorkerVec[FmNumberWorkers - 1].FromWorker	= OverloadFromWorker;
   }
}
/*}}}*/
/*}}}*/
/*{{{  random numbers */
/**
*** These random number routines are essentially the same as Fox:
*** Solving problems on concurrent processors VolII, appendix E.
**/
#define MULT	1103515245
#define ADD	12345
#define MASK	0x7fffffff
static	int	Multiplier	= 1;
static	int	Adder		= 0;

int FmRand(void)
{ int result	= FmSeed;
  FmSeed = ((FmSeed * Multiplier) + Adder) & MASK;
  return(result);
}

static void initialise_rand(void)
{ int	i;

  for (i = 0; i <= FmWorkerNumber; i++)
   FmSeed = ((FmSeed * MULT) + ADD) & MASK;

  for (i = 0; i < FmNumberWorkers; i++)
   { Multiplier = (Multiplier * MULT) & MASK;
     Adder	= ((MULT * Adder) + ADD) & MASK;
   }
}
/*}}}*/
/*{{{  exception handling */
/**
*** This code takes care of SIGSIB signals generated when a processor
*** running part of the application has died. Communication failures
*** are dealt with separately.
**/
/*{{{  worker exceptions */
/**
*** In the worker the SIGSIB handler checks whether or not the controller
*** exists, using system library calls rather than RmLib to avoid
*** excessive numbers of connections to the TFM.
**/
static void worker_signal_handler(int x)
{ Object	*controller;
  int		 retries;

  Debug(FmD_Signals, ("SIGSIB received"));

  for (retries = 0; retries < 3; retries++)
   { controller = Locate(NULL, ControllerName);
     if (controller ne Null(Object))
      { Close(controller); return; }
     Delay(3 * OneSec);
   }

  warn("lost contact with controller");
  x = x;
  Exit(0x81);
}
/*}}}*/
/*{{{  controller exceptions */
/**
*** In the controller most of the signal handling is taken care off by
*** a separate problem handler thread. The signal handler wakes up this
*** thread using a semaphore.
**/
static	Semaphore	SigsibCount;

static void controller_signal_handler(int x)
{ x = x;
  Debug(FmD_Signals, ("SIGSIB received"));
  Signal(&SigsibCount);
}

/**
*** Given the current taskforce this routine is responsible for identifying
*** the controller, i.e. the one program that is not the worker.
**/
static int find_control(RmTask task, ...)
{
  if (strcmp(RmGetTaskId(task), WorkerName))
   return((int) task);
  else
   return(0);
}

/**
*** For every signal received the problem handler performs the following:
***  1) get hold of the current taskforce details
***  2) identify the controller
***  3) check all the workers, except the overloaded one if any
**/
static void controller_problem_handler(void)
{ RmTaskforce	taskforce;
  RmTask	control;
  RmTask	worker;
  int		i;

  forever
   { Wait(&SigsibCount);

     Debug(FmD_Signals, ("controller problem handler running"));
     
     taskforce = RmGetTaskforce();
     if (taskforce eq NULL)
      { report("problem handler, failed to get taskforce details from TFM");
        continue;
      }

     control = (RmTask) RmSearchTasks(taskforce, &find_control);
     if (control eq NULL)
      { report("problem handler, failed to identify controller");
        continue;
      }

     Debug(FmD_Signals, ("problem handler, found controller, scanning workers"));

     for (i = 0; i < (FmOverloadController ? (FmNumberWorkers - 1) : FmNumberWorkers); i++)
      { worker = RmFollowChannel(control, FirstWorkerPipe + i + i, NULL);
        if (worker eq NULL)
         { report("problem handler, failed to identify worker %d", i);
           kill_worker(i);
         }
        elif(!RmIsTaskRunning(worker))
         { if (WorkerVec[i].State ne Worker_Dead)
            { Debug(FmD_Signals, ("problem handler, worker %d has failed", i));
	      kill_worker(i);
	    }
	 }
      }

     RmFreeTaskforce(taskforce);

     Debug(FmD_Signals, ("problem handler done"));
   }
}
/*}}}*/
/*{{{  initialise exceptions */
/**
*** This code deals with initialising signals.
***  1) SIGPIPE is always ignored. If there is a problem with the pipes
***     then the communication code takes care of it.
***  2) SIGSIB handling depends on whether or not fault handling is
***     enabled.
***     a) in the worker a simple signal handler is installed
***     b) in the controller life becomes more difficult, to cope with
***        multiple SIGSIB signals. A separate problem handler is
***        spawned off and is woken up by the signal handler whenever
***        a signal is received, using a semaphore. This then examines
***        the taskforce.
**/
static void initialise_exceptions(void)
{ 
  signal(SIGPIPE, SIG_IGN);

  unless(FmFaultHandling)
   return;

  if (InControl)
   { struct sigaction temp;

     Debug(FmD_ControlInit, ("initialising SIGSIB handling"));

     InitSemaphore(&SigsibCount, 0);
     unless(Fork(StackSize, &controller_problem_handler, 0))
      fatal("not enough memory to initialise signal handling");

     sigaction(SIGSIB, NULL, &temp);
     temp.sa_handler = &controller_signal_handler;
     temp.sa_flags  |= SA_ASYNC;
     sigaction(SIGSIB, &temp, NULL);     
   }
  else
   { struct sigaction temp;
     Debug(FmD_WorkerInit, ("initialising SIGSIB handling"));
     sigaction(SIGSIB, NULL, &temp);
     temp.sa_handler = &worker_signal_handler;
     temp.sa_flags  |= SA_ASYNC;
     temp.sa_mask   |= (1 << SIGSIB);
     sigaction(SIGSIB, &temp, NULL);
   }
}
/*}}}*/
/*}}}*/
/*{{{  initialising the pipes */
/*{{{  structures */
typedef struct	ControlToWorker {
	int	FmNumberWorkers;
	int	FmWorkerNumber;
	int	FmSeed;
	char	ControllerName[IOCDataMax];
} ControlToWorker;

typedef struct	WorkerToControl {
	int	Nothing;
} WorkerToControl;
/*}}}*/
/*{{{  worker_initialise_pipes() */
/**
*** Each worker task starts by receiving some data from the controller
*** giving the correct values for certain library variables, and
*** sends back an acknowledgement. The aim is to make sure that the
*** pipes are correctly set up.
**/
static void worker_initialise_pipes(void)
{ ControlToWorker	packet1;
  WorkerToControl	packet2;

  Debug(FmD_WorkerInit, ("initialising pipes"));

  unless(full_read(&(WorkerInput), (BYTE *) &packet1, sizeof(ControlToWorker)))
   exit(EXIT_FAILURE);

  FmNumberWorkers	= packet1.FmNumberWorkers;
  FmWorkerNumber	= packet1.FmWorkerNumber;
  FmSeed		= packet1.FmSeed;
  strcpy(ControllerName, packet1.ControllerName);

  Debug(FmD_WorkerInit, ("received packet from controller"));

  unless(full_write(&(WorkerOutput), (BYTE *) &packet2, sizeof(WorkerToControl)))
   exit(EXIT_FAILURE);

  Debug(FmD_WorkerInit, ("sent acknowledgement back to controller"));
}
/*}}}*/
/*{{{  controller_initialise_pipes() */
/**
*** The controller is responsible for sending a packet to every worker
*** containing some necessary library variables and then reading back
*** a reply. The aim is to ensure that all the pipes are set up.
**/
static void controller_initialise_pipes(void)
{ ControlToWorker	packet1;
  WorkerToControl	packet2;
  int			i;
  int			fd;

  for (i = 0; i < (FmOverloadController ? (FmNumberWorkers - 1) : FmNumberWorkers); i++)
   { Debug(FmD_ControlInit, ("sending initial packet to worker %d", i));

     packet1.FmNumberWorkers	= FmNumberWorkers;
     packet1.FmWorkerNumber	= i;
     packet1.FmSeed		= FmSeed;
     strcpy(packet1.ControllerName, ControllerName);
     fd = FirstWorkerPipe + i + i + 1;

     unless(full_write(&fd, (BYTE *) &packet1, sizeof(ControlToWorker)))
      fatal("failed to contact worker %d", i);
     fd--;

     Debug(FmD_ControlInit, ("sent initial packet, awaiting reply"));

     unless(full_read(&fd, (BYTE *) &packet2, sizeof(WorkerToControl)))
      fatal("failed to get reply from worker %d", i);

     Debug(FmD_ControlInit, ("got reply"));
   }
}
/*}}}*/
/*{{{  initialise_pipes() */
/**
*** Note: in an overloaded controller the pipes between the controller
*** and its own worker is not initialised. There is no need to exchange
*** any information and there are potential deadlock problems while the
*** pipe is connecting.
**/
static void initialise_pipes(void)
{ if (InControl)
   controller_initialise_pipes();
  else
   worker_initialise_pipes();
}
/*}}}*/
/*}}}*/
/*{{{  FmCountProcessors() */
/*{{{  FilterNetwork() */
/**
*** This routine is used to examine the current network and find out
*** whether a processor is suitable for running a component of the farm
*** library. It is necessary to consider status, ownership, and
*** processor types.
**/

static int FmCountProcessors_FilterNetwork(RmProcessor processor, ...)
{ va_list	args;
  RmProcessor	this_processor;

  va_start(args, processor);
  this_processor = va_arg(args, RmProcessor);
  va_end(args);

	/* The current processor does not count towards the total.	*/
  if (processor eq this_processor)
   return(0);

	/* A processor should be either free or owned by this session	*/
  if (RmGetProcessorOwner(processor) ne RmO_FreePool)
   { if (RmGetProcessorOwner(processor) ne RmWhoAmI())
      return(0);
     if (RmGetProcessorSession(processor) ne RmGetSession())
      return(0);
   }

	/* The processor purpose should be Helios			*/
  unless((RmGetProcessorPurpose(processor) & RmP_Mask) eq RmP_Helios)
   return(0);

	/* The processor should be running.				*/
  unless(RmGetProcessorState(processor) & RmS_Running)
   return(0);

	/* The processor types should be compatible.			*/
  unless(is_compatible(this_processor, processor))
   return(0);

	/* Check memory usage ???					*/

  return(1);
}
/*}}}*/
/*{{{  FilterDomain() */
/**
*** The suitability test for a processor within the domain is less severe
*** because ownership conditions have been resolved already.
**/
static int FmCountProcessors_FilterDomain(RmProcessor processor, ...)
{ va_list	args;
  RmProcessor	this_processor;

  va_start(args, processor);
  this_processor = va_arg(args, RmProcessor);
  va_end(args);

	/* The current processor does not count towards the total.	*/
  if (processor eq this_processor)
   return(0);

	/* The processor purpose should be Helios			*/
  unless((RmGetProcessorPurpose(processor) & RmP_Mask) eq RmP_Helios)
   return(0);

	/* The processor should be running.				*/
  unless(RmGetProcessorState(processor) & RmS_Running)
   return(0);

	/* The processor types should be compatible.			*/
  unless(is_compatible(this_processor, processor))
   return(0);

	/* Check memory usage ???					*/

  return(1);
}
/*}}}*/

int FmCountProcessors(int option)
{ int		result;
  char		procname[IOCDataMax];
  RmProcessor	this_processor;

  if ((option ne Fm_Network) && (option ne Fm_Domain))
   { fprintf(stderr, "Farmlib: illegal argument %d passed to FmCountProcessors()\n", option);
     exit(EXIT_FAILURE);
   }

  MachineName(procname);

  if (option eq Fm_Network)
   { RmNetwork network = RmGetNetwork();
     if (network eq NULL)	
      { if (RmErrno eq RmE_MissingServer) /* Tiny Helios	*/
         return(0);
	else
	 { fputs("Farmlib: out of memory in FmCountProcessors().\n", stderr);
	   exit(EXIT_FAILURE);
	 }
      }

     this_processor = RmLookupProcessor(network, procname);
     if (this_processor eq NULL)
      { fputs("Farmlib: failed to identify current processor.\n", stderr);
        exit(EXIT_FAILURE);
      }

     result = RmApplyProcessors(network, &FmCountProcessors_FilterNetwork, this_processor);
     RmFreeNetwork(network);
     return(result);
   }
  else
   { RmNetwork domain = RmGetDomain();
     if (domain eq NULL)
      { fputs("Farmlib: failed to contact Taskforce Manager.\n", stderr);
        exit(EXIT_FAILURE);
      }
     
     this_processor = RmLookupProcessor(domain, procname);
     if (this_processor eq NULL)
      { fputs("Farmlib: failed to identify current processor.\n", stderr);
	exit(EXIT_FAILURE);
      }

     result = RmApplyProcessors(domain, &FmCountProcessors_FilterDomain, this_processor);
     RmFreeNetwork(domain);
     return(result);
   }
}
/*}}}*/
/*{{{  distribution around the network */
/*{{{  new_worker() */
/**
*** This utility adds worker n to a given taskforce. The first component
*** of the taskforce is guaranteed to be the controller. A static is used
*** to optimise determining the code etc. All the workers should receive
*** the same arguments as the controller, so that they can perform their
*** initialisation in the same way.
**/
static	char	CodeName[IOCDataMax];
static	bool	CodeDetermined = FALSE;

static RmTask	new_worker(RmTaskforce taskforce, int number)
{ RmTask	 task;
  Environ	*env	= getenviron();
  int		 i;
  char		*memerr	= "insufficient memory to initialise taskforce";

  Debug(FmD_Distribute, ("adding worker %d", number));

  task	= RmNewTask();
  if (task eq NULL) fatal(memerr);

  RmSetTaskId(task, WorkerName);

	/* The code should consist of a capability followed by a	*/
	/* full pathname. Both of these can be obtained from the	*/
	/* environment. If the loaded object is known this is used,	*/
	/* otherwise the source file is taken. The performance		*/
	/* difference is marginal since the TFM will perform load from	*/
	/* neighbour anyway.						*/
  unless(CodeDetermined)
   { Object	*code;

     for(i = 0; i < OV_Parent; i++)
      if (env->Objv[i] eq NULL)
       fatal("failed to identify the code associated with this program");

     if (env->Objv[OV_Code] ne (Object *) MinInt)
      code = env->Objv[OV_Code];
     elif (env->Objv[OV_Source] ne (Object *) MinInt)
      code = env->Objv[OV_Source];
     else
      fatal("unable to identify the code associated with this program");

     DecodeCapability(CodeName, &(code->Access));
     strcat(CodeName, code->Name);
     CodeDetermined = TRUE;
   }

  if (RmSetTaskCode(task, CodeName) ne RmE_Success)
   fatal(memerr);

	/* Copy all of the controller task's arguments.			*/
  for (i = 1; env->Argv[i] ne NULL; i++)
   if (RmAddTaskArgument(task, i, env->Argv[i]) ne RmE_Success)
    fatal(memerr);

	/* Make the new task part of the taskforce.			*/
  unless(RmAddtailTask(taskforce, task) eq task)
   fatal(memerr);

	/* And add the streams to/from the controller. This should be	*/
	/* the first task in the taskforce.				*/
  unless ((RmMakeChannel(RmFirstTask(taskforce), FirstWorkerPipe + 1 + number + number,task, 0) eq RmE_Success) &&
	  (RmMakeChannel(RmFirstTask(taskforce), FirstWorkerPipe + number + number, task, 1) eq RmE_Success))
   fatal(memerr);

	/* Finally return the task so that it can be given a puid	*/
	/* attribute, if necessary.					*/
  return(task); 
}
/*}}}*/
/*{{{  distribute_build() */
/**
*** Given a processor and a taskforce, this routine adds a worker to the
*** taskforce which is mapped onto that processor using a puid attribute.
**/
static int distribute_build(RmProcessor processor, ...)
{ va_list	 args;
  RmTaskforce	 taskforce;
  int		*worker_number;
  char		 puidname[IOCDataMax];
  RmTask	 task;

  va_start(args, processor);
  taskforce	= va_arg(args, RmTaskforce);
  worker_number	= va_arg(args, int *);
  va_end(args);

  task		    = new_worker(taskforce, *worker_number);
  (*worker_number) += 1;

	/* This code fragment performs the mapping.			*/
  strcpy(puidname, "puid=");
  RmBuildProcessorName(&(puidname[5]), processor);
  if (RmAddTaskAttribute(task, puidname) ne RmE_Success)
   fatal("insufficient memory to map taskforce");

  Debug(FmD_Distribute, ("worker is mapped onto processor %s", &(puidname[5])));
  return(1);
}
/*}}}*/
/*{{{  distribute_network() */
/**
*** To distribute a farm around the network requires the following:
***  1) The network details are obtained
***  2) If there is no Network Server then this is a Tiny Helios system.
***     The FmOverloadController flag is set and a NULL taskforce is returned.
***  3) Otherwise the network is filtered and an attempt is made to obtain
***     the remaining processors. It is not essential to obtain all the
***     processors.
***  4) A taskforce is then constructed with all the tasks mapped to 
***     processors, using PUID attributes.
**/
/*{{{  distribute_network_filter() */
/**
*** This routine is very similar to FmCountProcessors_FilterNetwork(), but
*** removes and frees unsuitable processors.
**/
static int distribute_network_filter(RmProcessor processor, ...)
{ va_list	args;
  RmProcessor	this_processor;
  bool		ok	= FALSE;

  va_start(args, processor);
  this_processor = va_arg(args, RmProcessor);
  va_end(args);

  if (processor eq this_processor)  return(0);

	/* A processor should be either free or owned by this session.	*/
  if (RmGetProcessorOwner(processor) ne RmO_FreePool)
   { if (RmGetProcessorOwner(processor) ne RmWhoAmI())		goto done;
     if (RmGetProcessorSession(processor) ne RmGetSession())	goto done;
   }

	/* The processor purpose should be Helios			*/
  unless ((RmGetProcessorPurpose(processor) & RmP_Mask) eq RmP_Helios)
   goto done;

	/* The processor should be running.				*/
  unless(RmGetProcessorState(processor) & RmS_Running)
   goto done;

	/* The processor types should be compatible.			*/
  unless(is_compatible(this_processor, processor))
   goto done;

  ok = TRUE;

done:
  if (ok)
   { Debug(FmD_Distribute, ("processor %s is suitable", RmGetProcessorId(processor)));
     return(1);
   }
  else
   { Debug(FmD_Distribute, ("processor %s is unsuitable", RmGetProcessorId(processor)));
     (void) RmFreeProcessor(RmRemoveProcessor(processor));
     return(0);
   }
}
/*}}}*/

static RmTaskforce distribute_network(RmTaskforce taskforce)
{ RmNetwork		whole_network;
  char			procname[IOCDataMax];
  RmProcessor		this_processor;
  RmNetwork		obtained_network;
  int			worker_number;

  Debug(FmD_Distribute, ("distributing around the current network"));

	/* Get network details from the Network Server			*/
  whole_network = RmGetNetwork();
  if (whole_network eq NULL)
   { if (RmErrno eq RmE_MissingServer)	/* Tiny Helios	*/
      return(NULL);
     else
      fatal("distribution option Fm_Network, failed to get network details");
   }

 Debug(FmD_Distribute, ("got network details"));

  MachineName(procname);
  this_processor = RmLookupProcessor(whole_network, procname);
  if (this_processor eq NULL)
   fatal("failed to identify current processor");

  Debug(FmD_Distribute, ("filtering out unsuitable processors"));

	/* Remove any processors that are unsuitable			*/
  if (RmApplyProcessors(whole_network, &distribute_network_filter, this_processor) eq 0)
   {	/* No free processors	*/
     warn("there are no free processors within the network");
     RmFreeNetwork(whole_network);
     return(NULL);
   }
  RmFreeProcessor(RmRemoveProcessor(this_processor));

  Debug(FmD_Distribute, ("attempting to obtain remaining processors"));

	/* Obtain some or all of the processors that remain.		*/
  obtained_network = RmObtainNetwork(whole_network, FALSE, NULL);
  RmFreeNetwork(whole_network);
  if ((obtained_network eq NULL) || (RmCountProcessors(obtained_network) eq 0))
   { warn("failed to obtain any processors to run the application");
     if (obtained_network ne NULL) RmFreeNetwork(obtained_network);
     return(NULL);
   }

	/* This adds a task for every processor.			*/
  Debug(FmD_Distribute, ("processors obtained, mapping taskforce"));
  worker_number = 0;
  (void) RmApplyProcessors(obtained_network, &distribute_build, taskforce, &worker_number);

	/* The taskforce is now ready.					*/
  return(taskforce);
}
/*}}}*/
/*{{{  distribute_domain() */
/**
*** The code for distributing around the domain is almost identical to that
*** for distributing around the network.
**/
/*{{{  distribute_domain_filter() */
/**
*** This routine is very similar to FmCountProcessors_FilterDomain(), but
*** removes and frees unsuitable processors.
**/
static int distribute_domain_filter(RmProcessor processor, ...)
{ va_list	args;
  RmProcessor	this_processor;
  bool		ok	= FALSE;

  va_start(args, processor);
  this_processor = va_arg(args, RmProcessor);
  va_end(args);

  if (processor eq this_processor)
   return(0);

	/* The processor purpose should be Helios			*/
  unless ((RmGetProcessorPurpose(processor) & RmP_Mask) eq RmP_Helios)
   goto done;

	/* The processor should be running.				*/
  unless(RmGetProcessorState(processor) & RmS_Running)
   goto done;

	/* The processor types should be compatible.			*/
  unless(is_compatible(this_processor, processor))
   goto done;

  ok = TRUE;

done:
  if (ok)
   return(1);
  else
   { (void) RmFreeProcessor(RmRemoveProcessor(processor));
     return(0);
   }
}
/*}}}*/

static RmTaskforce distribute_domain(RmTaskforce taskforce)
{ RmNetwork		domain;
  char			procname[IOCDataMax];
  RmProcessor		this_processor;
  RmNetwork		obtained;
  int			worker_number;

  Debug(FmD_Distribute, ("distributing application around the current domain"));

	/* Get domain details from the TFM				*/
  domain = RmGetDomain();
  if (domain eq NULL)
   fatal("unable to contact a Taskforce Manager for this session");

  Debug(FmD_Distribute, ("obtained details of the current domain"));

  MachineName(procname);
  this_processor = RmLookupProcessor(domain, procname);
  if (this_processor eq NULL)
   fatal("failed to identify current processor");

  Debug(FmD_Distribute, ("filtering unsuitable processors from the domain"));

	/* Remove any processors that are unsuitable			*/
  if (RmApplyProcessors(domain, &distribute_domain_filter, this_processor) eq 0)
   {	/* No free processors	*/
     warn("there are no free processors within the domain");
     RmFreeNetwork(domain);
     return(NULL);
   }
  RmFreeProcessor(RmRemoveProcessor(this_processor));

  Debug(FmD_Distribute, ("attempting to obtain remaining processors"));

	/* Obtain some or all of the processors that remain.		*/
  obtained = RmObtainNetwork(domain, FALSE, NULL);
  RmFreeNetwork(domain);
  if ((obtained eq NULL) || (RmCountProcessors(obtained) eq 0))
   { warn("failed to obtain any extra processors to run the application");
     if (obtained ne NULL) RmFreeNetwork(obtained);
     return(NULL);
   }

  Debug(FmD_Distribute, ("obtained processors, constructing the rest of the taskforce"));
  worker_number = 0;
  (void) RmApplyProcessors(obtained, &distribute_build, taskforce, &worker_number);

	/* The taskforce is now ready.					*/
  return(taskforce);
}
/*}}}*/
/*{{{  distribute_fixed() */
/**
*** If the taskforce size is fixed then there is no need to examine the
*** network, except possibly as a safety check. The taskforce can be
*** constructed immediately.
**/
static RmTaskforce distribute_fixed(RmTaskforce taskforce)
{ int	i;

  if (FmOverloadController)
   FmNumberWorkers--;	/* distribute() will add it on again.	*/

  Debug(FmD_Distribute, ("constructing a taskforce with %d workers", FmNumberWorkers));

  if (FmNumberWorkers <= 0)
   return(NULL);

  for (i = 0; i < FmNumberWorkers; i++)
   new_worker(taskforce, i);

  return(taskforce);
}
/*}}}*/
/*{{{  distribute_processors() */
/**
*** Distributing around the specified processors is similar to distributing
*** around a network, but the user has already worked out which processors
*** are required.
**/
static RmTaskforce distribute_processors(RmTaskforce taskforce)
{ int	worker_number;

  Debug(FmD_Distribute, ("constructing a taskforce using the specified processors"));

  if (RmCountProcessors(FmSelectedNetwork) <= 0)
   { warn("Fm_Processors, the selected network does not have any processors");
     return(NULL);
   }

  worker_number = 0;
  (void) RmApplyProcessors(FmSelectedNetwork, &distribute_build, taskforce, &worker_number);
  return(taskforce);
}
/*}}}*/
/*{{{  distribute() */
/*{{{  description */
/**
*** This is the most original part of the farming library, where a single
*** program manages to turn itself into a whole taskforce. The basic
*** algorithm is as follows:
***  1) Is there a TFM ? If not, complain vociferously.
***  2) The taskforce template is allocated and the controller task is
***     initialised.
***  3) Next the taskforce is constructed. The flood option is rather
***	significant. Basically the main distribute function creates the
***	basis of the taskforce, containing just the controller, and then
***	calls one of the four distribute functions. These distribute
***	functions return NULL to indicate that no extra components were
***	added to the taskforce, or a suitable enlarged taskforce.
***	For the Fm_Network and Fm_Domain options the various tasks will have
***	been mapped onto obtained processors. For Fm_Fixed the processors
***	will be unmapped, allowing more than one worker to run on a given
***	processor. For Fm_Processors the supplied processors are used,
***	and hopefully these will have been obtained already.
***  4) Given this taskforce, RmConvertTaskToTaskforce() will do the
***     rest of the work. The remaining tricky bits are the responsibility
***	of the Resource Management library and the TFM.
**/
/*}}}*/
/*{{{  add_controller() */
static RmTask add_controller(RmTaskforce taskforce)
{ char		 controller_name[NameMax];
  char		 controller_puid[IOCDataMax];
  RmTask	 controller;
  char		*tmp;
  Environ	*env	= getenviron();

  controller = RmNewTask();
  if (controller eq NULL)
   fatal("insufficient memory to construct controller");

	/* Given task /Net/00/tasks/pi.12, produce id pi */
  strcpy(controller_name, objname(env->Objv[OV_Task]->Name));
  for (tmp = controller_name + strlen(controller_name);
       (*tmp ne '.') && (tmp > controller_name);
       tmp--);
  if (*tmp eq '.') *tmp = '\0';
  RmSetTaskId(controller, controller_name);

	/* Also add a PUID attribute			*/
  strcpy(controller_puid, "puid=");
  MachineName(&(controller_puid[5]));
  if (RmAddTaskAttribute(controller, controller_puid) ne RmE_Success)
   fatal("insufficient memory to map controller");

  if (RmAddtailTask(taskforce, controller) eq NULL)
   fatal("insufficient memory to incorporate controller into taskforce");

  return(controller);
}
/*}}}*/

static	RmTaskforce	Executing	= NULL;

static void distribute(void)
{ RmTaskforce		taskforce;
  RmTaskforce		result;
  RmTask		controller;

  Debug(FmD_Distribute, ("checking for the presence of a Taskforce Manager"));

	/* Check that there is a TFM associated with this session	*/
  { Environ	*env	= getenviron();
    int		 i;
    for (i = 0; i < OV_TFM; i++)
     if (env->Objv[i] eq NULL)
      break;
    if ((env->Objv[i] eq NULL) || (env->Objv[i] eq (Object *) MinInt))
     fatal("cannot find a Taskforce Manager associated with this program");
  }

  Debug(FmD_Distribute, ("allocating taskforce structure"));

	/* Now start constructing the taskforce.			*/
  taskforce = RmNewTaskforce();
  if (taskforce eq NULL)
   fatal("insufficient memory to construct taskforce");

  Debug(FmD_Distribute, ("adding controller to taskforce"));
  controller = add_controller(taskforce);

	/* The taskforce name is derived from the controller's.		*/
  { char	buf[NameMax];
    strncpy(buf, RmGetTaskId(controller), NameMax - 1);
    strncat(buf, ".farmlib", (NameMax - 1) - strlen(buf));
    RmSetTaskforceId(taskforce, buf);
  }

  Debug(FmD_Distribute, ("building taskforce using the specified flood option"));

	/* Add additional tasks depending on the flood option		*/
  switch(FmFloodOption)
   { case Fm_Network	: result = distribute_network(taskforce);
		      	  break;
     case Fm_Domain	: result = distribute_domain(taskforce);
			  break;
     case Fm_Fixed	: result = distribute_fixed(taskforce);
			  break;
     case Fm_Processors	: result = distribute_processors(taskforce);
			  break;
     default : fatal("memory corruption detected");
   }

	/* NULL means no additional tasks, not failure.			*/
  if (result eq NULL)
   {
     Debug(FmD_Distribute, ("the taskforce does not contain any worker tasks"));
     RmFreeTaskforce(taskforce);
     Executing = NULL; 
     FmNumberWorkers = 0;
     unless(FmOverloadController)
      { report("aborting start-up, there are no workers");
	exit(EXIT_FAILURE);
      }
   }
  else
   { Debug(FmD_Distribute, ("executing the resulting taskforce"));
     Executing = RmConvertTaskToTaskforce(controller, result);
     if (Executing eq NULL)
      { report("unable to distribute taskforce as requested");
	exit(EXIT_FAILURE);
      }
     Debug(FmD_Distribute, ("the taskforce is running"));     
     FmNumberWorkers = RmCountTasks(Executing) - 1;	/* allow for the controller */
   }

	/* If the controller should be overloaded, creating two pipes	*/
	/* internally for the communication between the controller and	*/
	/* the internal worker. Also, update FmNumberWorkers.		*/
  if (FmOverloadController)
   { int	pipes[2];
     Debug(FmD_Distribute, ("starting up overloaded worker"));
     if (pipe(pipes) < 0)
      fatal("failed to create internal pipes");
     WorkerInput	= pipes[0];
     OverloadToWorker	= pipes[1];
     if (pipe(pipes) < 0)
      fatal("cannot create internal pipes");
     WorkerOutput	= pipes[1];
     OverloadFromWorker	= pipes[0];
     FmNumberWorkers++;
     FmWorkerNumber	= FmNumberWorkers - 1;
   }

	/* Keep track of the number of running workers, to allow for	*/
	/* an early abort.						*/
  FmRunningWorkers = FmNumberWorkers;
  Debug(FmD_Distribute, ("there are %d running workers", FmRunningWorkers));
}
/*}}}*/
/*}}}*/
/*{{{  FmInitialise() */
/*{{{  FmInWorker() */
/**
*** This routine can be called before FmInitialise() to determine whether
*** the program is running as a worker or not.
**/
bool FmInWorker(void)
{ Environ *env	= getenviron();
  if (!strcmp(env->Argv[0], WorkerName))
   return(TRUE);
  else
   return(FALSE);
}
/*}}}*/

/**
*** FmInitialise() is the routine that does most of the real work. It
*** performs the following:
***  1) initialise library statics, after this the built-in diagnostics
***     routines can be used. There is a slight problem in that the
***     workers do not know their id yet, but I can live with that.
***  2) validate some of the user parameters
***  3) if InControl, distribute around the network. The actual number
***     of workers will be known at the end of this phase.
***  4) if InControl, allocate the WorkerVec
***  5) install signal handlers
***  6) initialise the pipes, the exact behaviour depending on whether
***     this is the controller or a worker
***  7) call the user's initialisation routines, if any
***  8) set up the random numbers
***  9) initialise buffer space
*** 10) start up the various threads
*** 11) wait for termination condition
**/
void FmInitialise(void)
{
  /*{{{  initialise statics */
    InitSemaphore(&Clib, 1);
    { Environ	*env	= getenviron();
      if (!strcmp(env->Argv[0], WorkerName))
       { InControl	= FALSE;
         InWorker	= TRUE;
       }
      else
       { InControl	= TRUE;
         if (FmOverloadController)
          InWorker	= TRUE;
         else
          InWorker	= FALSE;
       }
    }
  
    unless(InControl)
     { WorkerInput	= 0;
       WorkerOutput	= 1;
     }
  
    if (InControl)
     { Environ *env	= getenviron();
       strcpy(ControllerName, env->Objv[OV_Task]->Name);
     }
  /*}}}*/
  /*{{{  validate parameters */
  if (FmProducer eq NULL)
   fatal("no FmProducer routine has been specified");
  if (FmConsumer eq NULL)
   fatal("no FmConsumer routine has been specified");
  if (FmWorker eq NULL)
   fatal("no FmWorker routine has been specified");
  
  if (InControl)
   { if ((FmFloodOption ne Fm_Network) && (FmFloodOption ne Fm_Domain) &&
         (FmFloodOption ne Fm_Fixed) && (FmFloodOption ne Fm_Processors))
      fatal("unrecognised value %d for FmFloodOption", FmFloodOption);
  
     if (FmFloodOption eq Fm_Fixed)
      { if (FmNumberWorkers < 0)
         fatal("no worker count has been specified for FmFloodOption Fm_Fixed");
        if ((FmNumberWorkers eq 0) && !FmOverloadController)
         fatal("attempt to use FmFloodOption Fm_Fixed with zero workers");
      }
  
     if (FmFloodOption eq Fm_Processors)
      { unless(RmIsNetwork(FmSelectedNetwork))
         fatal("FmFloodOption Fm_Processors, no network has been supplied");
        if (RmCountProcessors(FmSelectedNetwork) <= 0)
         fatal("FmFloodOption Fm_Processors, there are no processors in the supplied network");
      }
   }
  /*}}}*/

  if (InControl)
   distribute();

  if (InControl)  
   initialise_controller_data();

  initialise_exceptions();

  initialise_pipes();

  if (InControl && (FmControllerInitialise ne NULL))
   (*FmControllerInitialise)();

  if (InWorker && (FmWorkerInitialise ne NULL))
   (*FmWorkerInitialise)();

  if (InControl && (FmControllerExit ne NULL))
   atexit(FmControllerExit);

  if (InWorker && (FmWorkerExit ne NULL))
   atexit(FmWorkerExit);

  if (InWorker)
   initialise_rand();

  if (InControl)
   { if (InWorker)
      init_buffers(2 * FmNumberWorkers + 3, FmNumberWorkers + 3);
     else
      init_buffers(2 * FmNumberWorkers + 1, FmNumberWorkers + 1);
   }
  else
   init_buffers(2, 2);

  if (InWorker)
   worker_startup();

  if (InControl)
   controller_start_threads();

  if (InControl)
   waitfor_controller();
  else
   waitfor_worker();  

	/* Closing the internal pipes. This is incredibly difficult	*/
	/* because of various timing problems and deadlock problems	*/
	/* within the system library.					*/
  if (InControl && FmOverloadController)
   { int old_worker_input	= WorkerInput;
     int old_worker_output	= WorkerOutput;
     Debug(FmD_Exit, ("closing internal pipes"));
     WorkerVec[FmNumberWorkers - 1].ToWorker	= -1;
     WorkerVec[FmNumberWorkers - 1].FromWorker	= -1;
     WorkerInput				= -1;
     WorkerOutput				= -1;
     Abort(fdstream(OverloadToWorker));
     Abort(fdstream(old_worker_input));
     (void) Fork(2000, (VoidFnPtr) &close,  sizeof(int), old_worker_input);
     close(OverloadToWorker);
     Abort(fdstream(old_worker_output));
     (void) Fork(2000, (VoidFnPtr) &close, sizeof(int), old_worker_output);
     close(OverloadFromWorker); 
   }

  Debug(FmD_Exit, ("exiting"));
  exit(EXIT_SUCCESS);   
}
/*}}}*/
