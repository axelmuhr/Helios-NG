/*------------------------------------------------------------------------
--                                                                      --
--           H E L I O S   N E T W O R K I N G   S O F T W A R E	--
--           ---------------------------------------------------	--
--									--
--			Hardware Reliability Test Suite			--
--			-------------------------------			--
--                                                                      --
--             Copyright (C) 1992, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- hwtest.c								--
--                                                                      --
--	Author:  BLV 18/10/92						--
--                                                                      --
------------------------------------------------------------------------*/
static char *rcsid = "$Header: /hsrc/network/packages/hwtests1/RCS/hwtest.c,v 1.4 1993/01/15 18:20:00 bart Exp $";

/*{{{  version number and revision history */
#define VersionNumber	"1.00"

/*
** Revision history :
**	1.00	first version of the hardware test suite that consists of
**		a single program and uses the Resource Management library
**		for distribution.
*/
/*}}}*/
/*{{{  header files etc. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <helios.h>
#include <sem.h>
#include <syslib.h>
#include <gsp.h>
#include <codes.h>
#include <root.h>
#include <task.h>
#include <memory.h>
#include <rmlib.h>
#include <unistd.h>
#include <nonansi.h>
/*}}}*/
/*{{{  data structures, statics and manifests */
	/* To avoid overflow problems calculating some of the summary	*/
	/* information unsigned arithmetic is used in places.		*/
typedef unsigned int uint;

	/* Command line arguments.					*/
static	bool	 Verbose	= FALSE;
static	bool	 Detach		= FALSE;
static	uint	 MemoryInterval	= 10;
static	bool	 NoMem		= FALSE;
static	bool	 NoFastmem	= FALSE;
static	bool	 NoLinks	= FALSE;
static	uint	 Days		= -1;
static	uint	 Hours		= -1;
static	uint	 Minutes	= -1;
static	bool	 ExtraComms	= FALSE;

	/* This structure is used to report link statistics.	*/
typedef struct LinkStats {
	uint		LinkNumber;
	uint		ReadIterations;
	uint		ReadFailures;
	uint		WriteIterations;
} LinkStats;

#define MaxLinkTransfer		1024
#define DataPerIteration	2040	/* 8+16+32+64...+ 1024 */

	/* This structure is used to report memory sizes, the number	*/
	/* of links, and LinkStats information for every link. There	*/
	/* is an upper limit on the number of links for simplicity.	*/
#define MaxLinks		8
typedef struct FullStats {
	char		ProcessorName[IOCDataMax];
	uint		OffchipMemory;	/* size in Kbytes		*/
	uint		OffchipFailures;
	uint		OffchipIterations;
	uint		OnchipMemory;	/* size in bytes		*/
	uint		OnchipFailures;
	uint		OnchipIterations;
	uint		NumberLinks;
	LinkStats	LinkStats[MaxLinks];
} FullStats;

	/* This is used to hold the information associated with this	*/
	/* worker.							*/
static FullStats	Results;


	/* It is necessary to synchronise when starting up the various	*/
	/* threads, so that the memory thread can work out when the	*/
	/* various buffers have been allocated and then allocate its	*/
	/* own buffer.							*/
static int		NumberThreads;	/* not including Helios consistency	*/
static Semaphore	ThreadsReady;

	/* When the controller decides that time is up it sends a	*/
	/* message to all the other components of the taskforce and	*/
	/* waits for replies. The Finished flag is checked regularly	*/
	/* by the various threads performing tests which then have to	*/
	/* synchronise for access to the Results structure and signal	*/
	/* to indicate when their results have been filled in.		*/
static	bool		Finished	= FALSE;
static	Semaphore	ResultsLock;
static	Semaphore	ResultsReady;

	/* This is used for diagnostics.				*/
char *ProcessorName = Results.ProcessorName;

	/* Control access to the C library statics			*/
static Semaphore LibraryLock;	/* Control access to the C library statics			*/

/*{{{  initialise statics */
	/* This routine is called from inside main() to initialise	*/
	/* the various semaphores etc. which cannot be done		*/
	/* automatically.						*/
static void initialise_statics(void)
{ int	i;

  InitSemaphore(&LibraryLock, 1);
  MachineName(Results.ProcessorName);
  Results.OffchipMemory		= 0;
  Results.OffchipFailures	= 0;
  Results.OnchipMemory		= 0;
  Results.OnchipFailures	= 0;
  Results.NumberLinks		= 0;
  for (i = 0; i < MaxLinks; i++)
   { Results.LinkStats[i].LinkNumber		= 0;
     Results.LinkStats[i].ReadIterations	= 0;
     Results.LinkStats[i].ReadFailures		= 0;
     Results.LinkStats[i].WriteIterations	= 0;
   }

  NumberThreads	= 0;
  InitSemaphore(&ThreadsReady, 0);
  InitSemaphore(&ResultsLock, 1);
  InitSemaphore(&ResultsReady, 0);
}
/*}}}*/

	/* Stacksize for the various threads running.			*/
#define StackSize	3000

	/* When the -c option is given to force extra communication	*/
	/* the controller component of the taskforce will send data	*/
	/* to all the workers and receive it back. The sizes of these	*/
	/* bits of data are 32, 64, 128, 256, 512 and 1024 bytes to	*/
	/* test the various forms of kernel buffering. Note that links	*/
	/* are likely to be busy except when checksumming the larger	*/
	/* link test transfers so the buffering code will be heavily	*/
	/* exercised.							*/
#define MaxCommsTransfer	1024

#if 0
#define Debug(a)
#else
#define Debug(a) Report a
#endif
/*}}}*/
/*{{{  utilities */
/*
** Random number generators. 32 bit random numbers are used.
*/
static unsigned int seed = 1;

static void hw_srand(int n)
{ seed = n;
}

static unsigned int hw_rand(void)
{ seed = (seed * 1103515245) + 12345;
  return(seed);
}

/*
** Error reporting etc.
*/
static void Report(char *str, ...)
{ va_list	args;

  Wait(&LibraryLock);
  va_start(args, str);
  fprintf(stderr, "hwtest: processor %s, ", ProcessorName);
  vfprintf(stderr, str, args);
  fputs("\n", stderr);
  va_end(args);
  Signal(&LibraryLock);
}

static void Fatal(char *str, ...)
{ va_list	args;

  Wait(&LibraryLock);
  va_start(args, str);
  fprintf(stderr, "hwtest: processor %s, ", ProcessorName);
  vfprintf(stderr, str, args);
  fputs("\n", stderr);
  va_end(args);
  exit(EXIT_FAILURE);
}

/*
** Usual full_read() routine
*/
static int full_read(int fd, char *buffer, int amount)
{ int	to_read	= amount;
  int	temp;

  while (to_read > 0)
   { temp= read(fd, buffer, to_read);
     if (temp < 0)	/* I/O error, including timeout			*/
      return(-1);
     to_read -= temp;
     buffer   = &(buffer[temp]);
   }
  return(amount);
}

/*}}}*/
/*{{{  link checking */
/*{{{  description */
/*
** These two routines deal with link checking. For every link attached to
** this processor which goes to another processor running a component of
** the taskforce there will be two channels to the other component, one
** for receiving and one for sending. There will also be two threads running
** for that link, one for sending and one for receiving.
** 
** The data transmitted via the channels consists of blocks of random
** data between eight and MaxLinkTransfer bytes. The first word of this
** data contains a checksum for the remaining data. Each iteration
** involves sending 8, 16, 32, ... MaxLinkTransfer bytes.
**
** There is a synchronisation problem when terminating. The component
** doing the sending may detect the termination condition after the
** component doing the receiving, and hence it may try to send data
** when the other end has already gone away. To avoid this the receiving
** end does not test the Finished flag itself. Instead there is an
** additional transfer at the start of every iteration which is the contents
** of the sending end's finished flag. If the sending end has finished then
** both ends will break out of the loop.
*/
/*}}}*/
/*{{{  receiver routine */
	/* The receiver routine. The index indicates an entry within	*/
	/* the Results structure.					*/
static void	link_receiver(int index, int linkno, int channel)
{ char		*buf	= NULL;
  int		*wbuf;
  int		 checksum;
  int		 i;
  int		 size;
  int		 failures	= 0;
  int		 iterations;

  buf	= malloc(MaxLinkTransfer);
  if (buf == NULL)
   Fatal("receiver for link %d : out of memory.", linkno);
  wbuf	= (int *) buf;

	/* Initial read to set up the pipe.				*/
  if (full_read(channel, buf, sizeof(int)) != sizeof(int))
   Fatal("receiver for link %d : failed to get initial data.",linkno);

  Signal(&ThreadsReady);	/* checked by memory tester		*/

  for (iterations = 0; ; iterations++)
   { 
     if (full_read(channel, buf, sizeof(bool)) != sizeof(bool))
      Fatal("receiver for link %d : read error %d getting Finished flag.", linkno, errno);
     if (*((bool *) buf))	/* Sending end's Finished flag		*/
      break;

     for (size = 8; size <= MaxLinkTransfer; size *= 2)
      { if (full_read(channel, buf, size) != size)
         Fatal("receiver for link %d : read error %d getting block of %d bytes.", linkno, errno, size);
	for (i = 1, checksum = 0; i < (size / sizeof(int)); i++)
	 checksum += wbuf[i];

	if (checksum != wbuf[0])
	 failures++;
      }
   }

	/* The remote end has finished, it is time to update the	*/
	/* results structure.						*/
  Wait(&ResultsLock);
  Results.LinkStats[index].LinkNumber		= linkno;
  Results.LinkStats[index].ReadIterations	= iterations;
  Results.LinkStats[index].ReadFailures		= failures;
  Signal(&ResultsLock);
  Signal(&ResultsReady);

	/* Receiver all done.						*/
}
/*}}}*/
/*{{{  sender routine */
	/* The sender routine is very similar to the receiver.		*/
static void	link_sender(int index, int linkno, int channel)
{ char		*buf	= NULL;
  int		*wbuf;
  int		 checksum;
  int		 i;
  int		 size;
  int		 iterations;

  buf = malloc(MaxLinkTransfer);
  if (buf == NULL)
   Fatal("sender for link %d : out of memory.", linkno);
  wbuf = (int *) buf;

	/* Perform an initial write to set up the pipe.			*/
  if (write(channel, buf, sizeof(int)) != sizeof(int))
   Fatal("sender for link %d : failed to send initial data.", linkno);

  Signal(&ThreadsReady);	/* checked by memory tester		*/

  for (iterations = 0; ; iterations++)
   { bool temp = Finished;
     if (write(channel, (char *) &temp, sizeof(bool)) != sizeof(bool))
      Fatal("sender for link %d : write error %d sending Finished flag.", linkno, errno);
     if (temp)
      break;

     for (size = 8; size <= MaxLinkTransfer; size *= 2)
      { for (i = 1, checksum = 0; i < (size / sizeof(int)); i++)
         { wbuf[i]   = hw_rand();
	   checksum += wbuf[i];
         }
        wbuf[0] = checksum;
        if (write(channel, buf, size) != size)
         Fatal("sender for link %d : write error %d sending block of %d bytes.", linkno, errno, size);
      }
   }

	/* The Finished flag has been set and the remote receiving end	*/
	/* has been informed. It is time to update the results		*/
	/* structure.							*/
  Wait(&ResultsLock);
  Results.LinkStats[index].LinkNumber		= linkno;
  Results.LinkStats[index].WriteIterations	= iterations;
  Signal(&ResultsLock);
  Signal(&ResultsReady);

	/* Sender all done.						*/
}
/*}}}*/
/*{{{  start link threads */
static void start_link_threads(int index, char *argv)
{ int	linkno, channel;

  unless (sscanf(argv, "%d.%d", &linkno, &channel) == 2)
   Fatal("invalid link specification %s", argv);

  NumberThreads++;
  unless(Fork(StackSize, &link_receiver, 3 * sizeof(int), index, linkno, channel))
   Fatal("out of memory starting receiver for link %d", linkno);

  NumberThreads++;
  unless(Fork(StackSize, &link_sender, 3 * sizeof(int), index, linkno, channel+1))
   Fatal("out of memory starting sender for link %d", linkno);
}
/*}}}*/
/*}}}*/
/*{{{  main memory checking */
static void check_mem(void)
{ int	 size;
  int	 i;
  char	*buf;
  int	*wbuf;
  int	 checksum;
  int	 tmp;
  int	 failures	= 0;
  int	 iterations	= 0;

	/* Before allocating memory it is necessary to wait for the	*/
	/* various other threads to allocate their buffers.		*/
  for (i = 1; i < NumberThreads; i++)
   Wait(&ThreadsReady);

	/* Now examine the free pool to determine the amount of memory	*/
	/* to get. The system is left with 50K of memory for buffering	*/
	/* etc.								*/
  size	 = GetRoot()->FreePool->Size;
  size	-= (50 * 1024);
  size	 = (size + 1023) & ~1023;

	/* If the TFM is running on this processor reduce the memory	*/
	/* size. The TFM may need more memory as well as the kernel and	*/
	/* Processor Manager.						*/
  { Object	*tmp = Locate(NULL, "/loader/tfm");
    if (tmp != NULL)
     { size -= (50 * 1024); Close(tmp); }
    tmp = Locate(NULL, "/loader/netserv");
    if (tmp != NULL)
     { size -= (50 * 1024); Close(tmp); }
  }
    
	/* Now try to get hold of a suitable buffer. The memory may	*/
	/* be fragmented so it is necessary to try smaller and smaller	*/
	/* mallocs.							*/
  for (buf = NULL; (buf == NULL) && (size > 0); size -= (10 * 1024))
   buf = malloc(size);

  if (buf == NULL)
   { Report("main memory checker, failed to allocate a buffer");
     Wait(&ResultsLock);
     Results.OffchipMemory	= 0;
     Results.OffchipFailures	= 0;
     Signal(&ResultsLock);
     Signal(&ResultsReady);
     return;
   }

	/* Fill the buffer with random data and calculate the checksum	*/
  wbuf = (int *) buf;
  for (i = 0, checksum = 0; i < (size / sizeof(int)); i++)
   { wbuf[i]	 = hw_rand();
     checksum	+= wbuf[i];
   }

  for (iterations = 0; ; iterations++)
   { for (i = 0; (i < MemoryInterval) && !Finished; i++)
      Delay(OneSec);
     if (Finished) break;

     for (i = 0, tmp = 0; i < (size / sizeof(int)); i++)
      tmp += wbuf[i];

     if (tmp != checksum)
      { checksum = tmp; failures++; }
   }

  Wait(&ResultsLock);
  Results.OffchipMemory		= size / 1024;
  Results.OffchipFailures	= failures;
  Results.OffchipIterations	= iterations;
  Signal(&ResultsLock);
  Signal(&ResultsReady);
}
/*}}}*/
/*{{{  on-chip memory checking */
#if !(defined(__TRAN)) && !(defined(__C40))
	/* This processor does not have on-chip memory.			*/
static void check_fast(void)
{ Signal(&ThreadsReady);
  Wait(&ResultsLock);
  Results.OnchipMemory		= 0;
  Results.OnchipFaillures	= 0;
  Signal(&ResultsLock);
  Signal(&ResultsReady);
}
#else
	/* This processor does have on-chip memory. The size varies and	*/
	/* cannot easily be determined automatically.			*/
#ifdef __TRAN
#define FastmemSize	(0x80001000 - 0x80000070)
#endif
#ifdef __C40
		/* Ignore any SRAM on strobe 0 of the local bus		*/
#define FastmemSize	8192
#endif
#ifndef FastmemSize
#error Please specify the size of the on-chip memory for this processor.
#endif

static void check_fast(void)
{ Carrier	*carrier	= NULL;
  int		 checksum;
  int		 i;
  int		 tmp;
  int		*wptr;
  int		 failures	= 0;
  int		 iterations	= 0;

  carrier = AllocFast(FastmemSize, &(MyTask->MemPool));
  Signal(&ThreadsReady);

  if (carrier == NULL)
   { Report("warning, failed to allocate on-chip memory.");
     Wait(&ResultsLock);
     Results.OnchipMemory	= 0;
     Results.OnchipFailures	= 0;
     Signal(&ResultsLock);
     Signal(&ResultsReady);
     return;
   }

	/* Fill the on-chip memory with random data.			*/
  wptr = (int *) carrier->Addr;
  for (i = 0, checksum = 0; i < (FastmemSize / sizeof(int)); i++)
   { wptr[i]	 = hw_rand();
     checksum	+= wptr[i];
   }

  for (iterations = 0; ; iterations++)
   { for (i = 0; (i < MemoryInterval) && !Finished; i++)
      Delay(OneSec);
     if (Finished) break;

     for (i = 0, tmp = 0; i < (FastmemSize / sizeof(int)); i++)
      tmp += wptr[i];
     if (tmp != checksum)
      { failures++; checksum = tmp; }
   }

  Wait(&ResultsLock);
  Results.OnchipMemory		= FastmemSize;
  Results.OnchipFailures	= failures;
  Results.OnchipIterations	= iterations;
  Signal(&ResultsLock);
  Signal(&ResultsReady);
}
#endif	/* processor has on-chip memory	*/
/*}}}*/
/*{{{  Helios consistency checks */
	/* This code is responsible for checking the state of the	*/
	/* Helios world, providing early detection of errors such as	*/
	/* consumption of message ports, system buffers, etc. It also	*/
	/* monitors the trace vector.					*/
static void check_helios(void)
{ int	ptsize;		/* size of port table after startup		*/
  int	freepool_size;
  int	trace_vector_index;
  int	kernel_buffer_count;
  bool	freepool_lowater	= FALSE;

	/* Go to sleep for a while to let the rest of the system	*/
	/* settle down.							*/
  Delay(30 * OneSec);

  ptsize		= GetRoot()->PTSize;
  freepool_size		= GetRoot()->FreePool->Size;
  trace_vector_index	= GetRoot()->TraceVec[0];
  kernel_buffer_count	= GetRoot()->MaxBuffers;

  forever
   { Delay(10 * OneSec);
     if ((2 * ptsize) < GetRoot()->PTSize)
      { Report("port table size has gone up from %d to %d", ptsize, GetRoot()->PTSize);
        ptsize = GetRoot()->PTSize;
      }

     if (!freepool_lowater && (GetRoot()->FreePool->Size < (25 * 1024)))
      { Report("FreePool size has dropped to %d", GetRoot()->FreePool->Size);
        freepool_lowater = TRUE;
      }

     if (freepool_size > (2 * GetRoot()->FreePool->Size))
      { Report("FreePool size has dropped from %d to %d", freepool_size, GetRoot()->FreePool->Size);
	freepool_size = GetRoot()->FreePool->Size;
      }

     if (trace_vector_index != GetRoot()->TraceVec[0])
      { Report("there are new entries in the trace vector.");
	trace_vector_index = GetRoot()->TraceVec[0];
      }

     if (kernel_buffer_count < ((4 * GetRoot()->MaxBuffers) / 5))
      { Report("kernel buffer count has gone up from %d to %d",
		kernel_buffer_count, GetRoot()->MaxBuffers);
        kernel_buffer_count = GetRoot()->MaxBuffers;
      }
   }
}
/*}}}*/
/*{{{  controller interactions and log file handling */
/*{{{  data structures etc. */
	/* This structure is used to hold summary information, as	*/
	/* stored in the logfile.					*/
typedef struct SummaryStats {
	uint	OffchipMemoryG;		/* Gigabyte hours		*/
	uint	OffchipMemoryM;		/* Megabyte hours		*/
	uint	OffchipMemoryK;		/* Kilobyte hours		*/
	uint	OffchipFailures;
	uint	OnchipMemoryG;		/* ditto			*/
	uint	OnchipMemoryM;
	uint	OnchipMemoryK;
	uint	OnchipFailures;
	uint	LinkG;			/* Gigabytes transferred	*/
	uint	LinkM;			/* Megabytes transferred	*/
	uint	LinkK;			/* Kilobytes transferred	*/
	uint	LinkFailures;
} SummaryStats;

static SummaryStats	LogStats;

	/* This is used to combine the information from all the		*/
	/* components.							*/
static SummaryStats	TotalStats;

	/* The logfile keeps track of the number of runs.		*/
static int		NumberRuns	= 0;

	/* And the number of hours.					*/
static int		NumberHours	= 0;

	/* These strings are used for input from and output to the	*/
	/* logfile, for consistency.					*/
static char *str1 = "\t\tHelios Hardware Reliability Test Suite";
static char *str2 = "\t\t======================================";
static char *str3 = "Summary Information after %d runs (%d hours) : %s";
static char *str4 = "Main memory   : %6d GByteHours, %4d MByteHours, %4d KByteHours, %2d failures";
static char *str5 = "On-chip memory: %6d GByteHours, %4d MByteHours, %4d KByteHours, %2d failures";
static char *str6 = "Link data     : %6d GBytes    , %4d MBytes    , %4d KBytes    , %2d failures";

	/* This string holds the name of the logfile			*/
static char *logname = NULL;
/*}}}*/
/*{{{  create new logfile */
static void create_new_logfile(void)
{ FILE		*tmp;
  int		 i;

  tmp = fopen(logname, "w");
  if (tmp == NULL)
   Fatal("failed to create logfile %s", logname);
  fputs(str1, tmp); fputs("\n", tmp);
  fputs(str2, tmp); fputs("\n\n", tmp);
  fprintf(tmp, str3, 0, 0, "today"); fputs("\n\n", tmp);
  fprintf(tmp, str4, 0, 0, 0, 0); fputs("\n", tmp);
  fprintf(tmp, str5, 0, 0, 0, 0); fputs("\n", tmp);
  fprintf(tmp, str6, 0, 0, 0, 0); fputs("\n", tmp);

	/* Leave room for the numbers to grow.				*/
  for (i = 0; i < 8; i++)
   fputs("                                                            \n", tmp);

  fclose(tmp);
}
/*}}}*/
/*{{{  read in logfile */
static void readin_logfile(void)
{ FILE		*tmp;
  char		 buf[90];
  char		 buf2[80];
  
  tmp = fopen(logname, "r");
  if (tmp == NULL)
   { create_new_logfile(); tmp = fopen(logname, "r"); }
  if (tmp == NULL)
   Fatal("failed to open logfile %s", logname);

  if ((fgets(buf, 90, tmp) == NULL) ||		/* title line	*/
      (fgets(buf, 90, tmp) == NULL) ||		/* underlines	*/
      (fgets(buf, 90, tmp) == NULL) ||		/* blank line	*/
      (fgets(buf, 90, tmp) == NULL) ||		/* summary information	*/
      (sscanf(buf, str3, &NumberRuns, &NumberHours, buf2) != 3) ||
      (fgets(buf, 90, tmp) == NULL) ||		/* blank line	*/
      (fgets(buf, 90, tmp) == NULL) ||
      (sscanf(buf, str4, &(LogStats.OffchipMemoryG), &(LogStats.OffchipMemoryM),
			 &(LogStats.OffchipMemoryK), &(LogStats.OffchipFailures))
	!= 4) ||
      (fgets(buf, 90, tmp) == NULL) ||
      (sscanf(buf, str5, &(LogStats.OnchipMemoryG), &(LogStats.OnchipMemoryM),
			 &(LogStats.OnchipMemoryK), &(LogStats.OnchipFailures))
	!= 4) ||
      (fgets(buf, 90, tmp) == NULL) ||
      (sscanf(buf, str6, &(LogStats.LinkG), &(LogStats.LinkM),
			 &(LogStats.LinkK), &(LogStats.LinkFailures))
	!= 4))
   Fatal("corrupt logfile %s", logname);

  fclose(tmp);
}  
/*}}}*/
/*{{{  update logfile */
static void writeout_logfile(void)
{ FILE		*tmp;
  time_t	 clck	= time(NULL);
  char	*now	= asctime(localtime(&clck));
  char	 realnow[32];

  strcpy(realnow, now);
  realnow[strlen(realnow) - 1] = '\0';	/* remove \n	*/

  tmp = fopen(logname, "r+");
  if (tmp == NULL)
   Fatal("failed to reopen logfile %s", logname);
  fputs(str1, tmp); fputs("\n", tmp);
  fputs(str2, tmp); fputs("\n\n", tmp);
  fprintf(tmp, str3, ++NumberRuns, NumberHours, realnow); fputs("\n\n", tmp);
  fprintf(tmp, str4,	LogStats.OffchipMemoryG, LogStats.OffchipMemoryM,
			LogStats.OffchipMemoryK, LogStats.OffchipFailures);
  fputs("\n", tmp);
  fprintf(tmp, str5,	LogStats.OnchipMemoryG, LogStats.OnchipMemoryM,
			LogStats.OnchipMemoryK, LogStats.OnchipFailures);
  fputs("\n", tmp);
  fprintf(tmp, str6,	LogStats.LinkG, LogStats.LinkM,
			LogStats.LinkK, LogStats.LinkFailures);
  fputs("\n", tmp);
	/* Allow for the numbers to shrink as well as grow		*/
  fputs("                                                    ", tmp);
  if (fseek(tmp, 0, SEEK_END) != 0)
   Fatal("failed to seek to end of logfile.");

  fprintf(tmp, "Run %d, %s, %d Days, %d Hours, %d Minutes\n",
		NumberRuns, realnow, Days, Hours, Minutes);
  fprintf(tmp, str4,	TotalStats.OffchipMemoryG, TotalStats.OffchipMemoryM,
			TotalStats.OffchipMemoryK, TotalStats.OffchipFailures);
  fprintf(tmp, str5,	TotalStats.OnchipMemoryG, TotalStats.OnchipMemoryM,
			TotalStats.OnchipMemoryK, TotalStats.OnchipFailures);
  fprintf(tmp, str6,	TotalStats.LinkG, TotalStats.LinkM,
			TotalStats.LinkK, TotalStats.LinkFailures);
  fputs("\n\n", tmp);
  fclose(tmp);
}

/*}}}*/
/*{{{  display results */
	/* This routine is invoked when running in verbose mode to give	*/
	/* detailed results from all the components.			*/
static void display_results(FullStats *stats)
{ int	i;

  printf("Results from processor %s\n", stats->ProcessorName);
  printf("Main memory    : %d KBytes, checked %d times, %d failures.\n",
		stats->OffchipMemory, stats->OffchipIterations,
		stats->OffchipFailures);
  printf("On chip memory : %d bytes, checked %d times, %d failures.\n",
		stats->OnchipMemory, stats->OnchipIterations,
		stats->OnchipFailures);

  for (i = 0; i < stats->NumberLinks; i++)
   printf("Link %d : %d read iterations with %d failures, %d write iterations.\n",
		stats->LinkStats[i].LinkNumber,
		stats->LinkStats[i].ReadIterations,
		stats->LinkStats[i].ReadFailures,
		stats->LinkStats[i].WriteIterations);
}
/*}}}*/
/*{{{  controller routine */
/*{{{  description */
/*
** The controller routine is given a string argument of the
** form x.y. This means that it is controlling x workers and
** should use the channels from y onwards to communicate with
** them.
**
** First the controller should initialise the pipes to and
** from all the workers, so that when the memory tester starts
** it does not grab required memory. Next it goes into a loop
** for the specified amount of time. If the extra comms option
** has been given then this time is spent sending data to
** and from the workers. Otherwise it is spent idle.
**
** When the run time has expired a message is sent to all the
** workers, which should then reply with their statistics.
** In verbose mode these are output. The statistics from the
** test routines running within the controller are collected
** as well to give a total for this run. Finally the log
** information is read in and updated.
*/
/*}}}*/
static void controller_routine(char *arg)
{ int		 number_workers;
  int		 first_channel;
  int		 i;
  char		*buf;
  int		*wbuf;
  int		 endtime;
  int		 size;
  int		 checksum;
  int		 worker;
  FullStats	 stats;

#define to_worker(i)   (first_channel + i + i + 1)
#define from_worker(i) (first_channel + i + i)

/*{{{  initialise */
  if (sscanf(arg, "%d.%d", &number_workers, &first_channel) != 2)
   Fatal("controller routine, invalid argument %s", arg);

  logname = getenv("HWTEST_LOGFILE");
  if (logname == NULL)
   logname = "/helios/local/tests/hwtest/hwtest.log";

  buf = malloc(MaxCommsTransfer);
  if (buf == NULL)
   Fatal("controller routine, out of memory.");
  wbuf = (int *) buf;

  for (worker = 0; worker < number_workers; worker++)
   { if (write(to_worker(worker), buf, sizeof(int)) != sizeof(int))
      Fatal("controller routine, failed to send initial data to worker %d", worker);
     if (full_read(from_worker(worker), buf, sizeof(int)) != sizeof(int))
      Fatal("controller routine, failed to receive initial data from worker %d", worker);
   }

	/* The main memory tester can now allocate its buffers.		*/
  Signal(&ThreadsReady);

  endtime  = ((((24 * Days) + Hours) * 60) + Minutes) * 60;
  endtime += time(NULL);
/*}}}*/
/*{{{  communication with workers */
  while (endtime > time(NULL))
   { 
     if (!ExtraComms)
      { Delay(5 * OneSec); continue; }

     for (size = 32; size <= MaxCommsTransfer; size *= 2)
      {		/* initialise the buffer with random data.		*/
	wbuf[0] = 0;	/* checksum */
	for (i = 1; i < (size / sizeof(int)); i++)
	 { wbuf[i] = hw_rand();
	   wbuf[0] += wbuf[i];
         }

	for (worker = 0; worker < number_workers; worker++)
	 { if (write(to_worker(worker), (char *) &size, sizeof(int)) != sizeof(int))
	   	Fatal("controller routine failed to send size %d to worker %d", size, worker);
	   if (write(to_worker(worker), buf, size) != size)
	   	Fatal("controller routine failed to write block of %d bytes to worker %d", size, worker);
	   if (full_read(from_worker(worker), buf, size) != size)
		Fatal("controller routine, failed to read back buffer of %d bytes from worker %d", size, worker);
	   for (i = 1, checksum = 0; i < (size / sizeof(int)); i++)
	    checksum += wbuf[i];
	   if (checksum != wbuf[0])
	    { Report("controller routine, corruption in buffer of %d bytes read back from worker %d",
	 		size, worker);
	      wbuf[0] = checksum;
	    }
         }	/* for all the workers		*/

        if (endtime < time(NULL))
	  break;
      }		/* sizes loop			*/
   }		/* while (endtime > time())	*/
/*}}}*/
/*{{{  terminating the workers */
	/* At this point the runtime has expired and it is necessary	*/
	/* to send a terminate message to all the workers. Also the	*/
	/* Finished flag should be set so that the testing going on	*/
	/* within the controller will stop.				*/
  Finished = TRUE;
  size     = 0;
  for (worker = 0; worker < number_workers; worker++)
   if (write(to_worker(worker), (char *) &size, sizeof(int)) != sizeof(int))
    Fatal("controller routine, failed to send terminate request to worker %d", worker);
/*}}}*/
/*{{{  collecting the stats */
	/* Now collect together the results from all the workers.	*/
  TotalStats.OffchipMemoryG	=
  TotalStats.OffchipMemoryM	=
  TotalStats.OffchipMemoryK	=
  TotalStats.OffchipFailures	=
  TotalStats.OnchipMemoryG	=
  TotalStats.OnchipMemoryM	=
  TotalStats.OnchipMemoryK	=
  TotalStats.OnchipFailures	=
  TotalStats.LinkG		=
  TotalStats.LinkM		=
  TotalStats.LinkK		=
  TotalStats.LinkFailures	= 0;

  for (worker = 0; worker < number_workers; worker++)
   { 
     if (full_read(from_worker(worker), (char *) &stats, sizeof(FullStats)) != sizeof(FullStats))
      Fatal("controller routine, failed to collect results from worker %d", worker);

     if (Verbose)
      display_results(&stats);
      
     TotalStats.OffchipMemoryK	+= stats.OffchipMemory;
     TotalStats.OffchipFailures	+= stats.OffchipFailures;
     TotalStats.OnchipMemoryK	+= stats.OnchipMemory;
     TotalStats.OnchipFailures	+= stats.OnchipFailures;
     for (i = 0; i < stats.NumberLinks; i++)
      { TotalStats.LinkK	+= stats.LinkStats[i].ReadIterations;
	TotalStats.LinkFailures	+= stats.LinkStats[i].ReadFailures;
      }
   }

	/* Include the results from the test routines running	*/
	/* within this worker.					*/
  for (i = 1; i < NumberThreads; i++)
   Wait(&ResultsReady);

  TotalStats.OffchipMemoryK	+= Results.OffchipMemory;
  TotalStats.OffchipFailures	+= Results.OffchipFailures;
  TotalStats.OnchipMemoryK	+= Results.OnchipMemory;
  TotalStats.OnchipFailures	+= Results.OnchipFailures;
  for (i = 0; i < Results.NumberLinks; i++)
   { TotalStats.LinkK		+= stats.LinkStats[i].ReadIterations;
     TotalStats.LinkFailures	+= stats.LinkStats[i].ReadFailures;
   }
/*}}}*/
/*{{{  calculate the real stats */
	/* The stats structure now contains the total amount of memory	*/
	/* that has been checked, plus the total number of link		*/
	/* iterations. This has to be converted to KByte hours etc,	*/
	/* taking some care to avoid overflow. Running on very large	*/
	/* networks with 256 4Mbyte processors for a whole week should	*/
	/* still be OK - just.						*/
	/*  1) divide by 60 to give a minutes rating.			*/
  TotalStats.OffchipMemoryK	/= 60;
  TotalStats.OnchipMemoryK	/= 60;
	/*  2) now multiply by the number of minutes in the run		*/
  TotalStats.OffchipMemoryK	*= ((((Days * 24) + Hours) * 60) + Minutes);
  TotalStats.OnchipMemoryK	*= ((((Days * 24) + Hours) * 60) + Minutes);
	/*  3) on-chip sizes were in bytes, not K			*/
  TotalStats.OnchipMemoryK	/= 1024;
	/*  4) update G and M values					*/
  TotalStats.OffchipMemoryM	 = TotalStats.OffchipMemoryK / 1024;
  TotalStats.OffchipMemoryK	 = TotalStats.OffchipMemoryK % 1024;
  TotalStats.OffchipMemoryG	 = TotalStats.OffchipMemoryM / 1024;
  TotalStats.OffchipMemoryM	 = TotalStats.OffchipMemoryM % 1024;
  TotalStats.OnchipMemoryM	 = TotalStats.OnchipMemoryK / 1024;
  TotalStats.OnchipMemoryK	 = TotalStats.OnchipMemoryK % 1024;
  TotalStats.OnchipMemoryG	 = TotalStats.OnchipMemoryM / 1024;
  TotalStats.OnchipMemoryM	 = TotalStats.OnchipMemoryM % 1024;
	/*  5) update link stats in much the same way			*/
  TotalStats.LinkK		/= 32;
  TotalStats.LinkK		*= DataPerIteration;
  TotalStats.LinkK		/= 32;	/* 32 * 32 == 1K */
  TotalStats.LinkM		 = TotalStats.LinkK / 1024;
  TotalStats.LinkK		 = TotalStats.LinkK % 1024;
  TotalStats.LinkG		 = TotalStats.LinkM / 1024;
  TotalStats.LinkM		 = TotalStats.LinkM % 1024;

  if (Verbose)
   { printf("Summary for this run.\n\n");
     printf(str4, TotalStats.OffchipMemoryG, TotalStats.OffchipMemoryM,
		  TotalStats.OffchipMemoryK, TotalStats.OffchipFailures);
     printf(str5, TotalStats.OnchipMemoryG, TotalStats.OnchipMemoryM,
		  TotalStats.OnchipMemoryK, TotalStats.OnchipFailures);
     printf(str6, TotalStats.LinkG, TotalStats.LinkM,
		  TotalStats.LinkK, TotalStats.LinkFailures);
     puts("\n");
   }
/*}}}*/
/*{{{  update the log file info */
  readin_logfile();	/* automatically created if necessary	*/

  LogStats.OffchipMemoryG	+= TotalStats.OffchipMemoryG;
  LogStats.OffchipMemoryM	+= TotalStats.OffchipMemoryM;
  LogStats.OffchipMemoryK	+= TotalStats.OffchipMemoryK;
  LogStats.OnchipMemoryG	+= TotalStats.OnchipMemoryG;
  LogStats.OnchipMemoryM	+= TotalStats.OnchipMemoryM;
  LogStats.OnchipMemoryK	+= TotalStats.OnchipMemoryK;
  LogStats.OffchipFailures	+= TotalStats.OffchipFailures;
  LogStats.OnchipFailures	+= TotalStats.OnchipFailures;
  LogStats.LinkG		+= TotalStats.LinkG;
  LogStats.LinkM		+= TotalStats.LinkM;
  LogStats.LinkK		+= TotalStats.LinkK;
  LogStats.LinkFailures		+= TotalStats.LinkFailures;

	/* And adjust these results as appropriate		*/
  LogStats.OffchipMemoryM	+= LogStats.OffchipMemoryK / 1024;
  LogStats.OffchipMemoryK	%= 1024;
  LogStats.OffchipMemoryG	+= LogStats.OffchipMemoryM / 1024;
  LogStats.OffchipMemoryM	%= 1024;
  LogStats.OnchipMemoryM	+= LogStats.OnchipMemoryK / 1024;
  LogStats.OnchipMemoryK	%= 1024;
  LogStats.OnchipMemoryG	+= LogStats.OnchipMemoryM / 1024;
  LogStats.OnchipMemoryM	%= 1024;
  LogStats.LinkM		+= LogStats.LinkK / 1024;
  LogStats.LinkK		%= 1024;
  LogStats.LinkG		+= LogStats.LinkM / 1024;
  LogStats.LinkM		%= 1024;
  NumberHours			+= ((24 * Days) + Hours);

  writeout_logfile();
/*}}}*/
}
/*}}}*/
/*{{{  worker routine */
	/* The worker routine should start by reading some data from	*/
	/* the controller and writing it back, so as to initiate the	*/
	/* pipes. It should then loop waiting for messages from the	*/
	/* controller. 							*/
	/* 	1) a single word 0 indicates that the application	*/
	/* 	   should finish. The Finished flag is set and the	*/
	/*	   routine waits for all the active threads to update	*/
	/*	   the results structure. This is then sent back to	*/
	/*	   the controller.					*/
	/*	2) any other value indicates the size of a checksummed	*/
	/*	   buffer which the controller is sending to test	*/
	/*	   Helios communications.				*/
static void worker_routine(void)
{ int		 n;
  char		*buf;
  int		*wbuf;
  int		 i;
  int		 checksum;
  int		 res;

  if (full_read(0, (char *) &n, sizeof(int)) != sizeof(int))
   Fatal("worker routine, failed to receive initial data");

  if (write(1, (char *) &n, sizeof(int)) != sizeof(int))
   Fatal("worker routine, failed to send initial data.");

  buf = malloc(MaxCommsTransfer);
  if (buf == NULL)
   Fatal("worker routine, out of memory.");

  wbuf = (int *) buf;

  Signal(&ThreadsReady);

  forever
   { 
     if (full_read(0, (char *) &n, sizeof(int)) != sizeof(int))
      Fatal("worker routine, error %d getting data from the controller.", errno);

     if (n == 0)	/* This is the termination condition	*/
      break;

     if ((res = full_read(0, buf, n)) != n)
      Fatal("worker routine, read returned only %d bytes, not %d", res, n);

     for (i = 1, checksum = 0; i < (n / sizeof(int)); i++)
      checksum += wbuf[i];

     if (checksum != wbuf[0])
      Fatal("worker routine, message of %d bytes from the controller has been corrupted.", n);

     if (write(1, buf, n) != n)
      Fatal("worker routine, error writing back buffer.");
   }

	/* When the code gets here the controller has sent a terminate	*/
	/* request. The Finished flag should be set and the various	*/
	/* testing threads should detect this within a few seconds.	*/
	/* They will update the results structure which must then be	*/
	/* transferred back to the controller.				*/
  Finished	= TRUE;
  for (i = 1; i < NumberThreads; i++)	/* this thread is included	*/
   Wait(&ResultsReady);

  if (write(1, (char *) &Results, sizeof(Results)) != sizeof(Results))
   Fatal("worker routine, failed to send back results.");
}
/*}}}*/
/*}}}*/
/*{{{  distribution around the network */
/*{{{  net_filter() */
	/* This routine is used to filter out unsuitable processors	*/
	/* from the network.						*/
static int net_filter(RmProcessor processor, ...)
{ int	purpose;
  int	type;
  int	state;
  bool	ok	= FALSE;

  purpose = RmGetProcessorPurpose(processor);
  if (purpose != RmP_Helios) goto done;

  state	= RmGetProcessorState(processor);
  unless(state & RmS_Running) goto done;

  type = RmGetProcessorType(processor);

  switch (MachineType())
   { case 400 :
     case 414 :		/* allow for future floating point expansion	*/
     case 425 : if ((type != RmT_T414) && (type != RmT_T425) &&
		    (type != RmT_T400))
		 goto done;
		else
		 break;

     case 800 :
     case 801 :
     case 805 : if (type != RmT_T800)
		 goto done;
		else
		 break;

     case 0xA3 : if (type != RmT_Arm)
		  goto done;
		 else
		  break;

     case 0x320C40 : if (type != RmT_C40)
			goto done;
		      else
			break;

     case 0x86 : if (type != RmT_i860)
		  goto done;
		 else
		  break;
   }

  ok = TRUE;

done:
  if (ok)
   return(1);
  else
   { RmFreeProcessor(RmRemoveProcessor(processor));
     return(0);
   }     
}
/*}}}*/
/*{{{  add_task() */
	/* This routine creates a task mapped to a particular processor	*/
static int add_task(RmProcessor processor, ...)
{ va_list	args;
  RmTaskforce	template;
  RmTask	task;
  char		buf[16];
  bool		ok;
  int		nextarg	= 1;

  va_start(args, processor);
  template = va_arg(args, RmTaskforce);
  va_end(args);

  task = RmNewTask();
  if (task == NULL)
   Fatal("out of memory mapping taskforce");

  if (RmSetTaskId(task, "<hwtest>") != RmE_Success) goto done;
  if (RmSetTaskCode(task, getenviron()->Objv[OV_Code]->Name) != RmE_Success) goto done;

  if (NoMem)
   if (RmAddTaskArgument(task, nextarg++, "-nm") != RmE_Success) goto done;
  if (NoFastmem)
   if (RmAddTaskArgument(task, nextarg++, "-nf") != RmE_Success) goto done;
  if (MemoryInterval != 10)
   { sprintf(buf, "-m%d", MemoryInterval);
     if (RmAddTaskArgument(task, nextarg++, buf) != RmE_Success) goto done;
   }
  if (ExtraComms)
   if (RmAddTaskArgument(task, nextarg++, "-C") != RmE_Success) goto done;
  if (Verbose)
   if (RmAddTaskArgument(task, nextarg++, "-v") != RmE_Success) goto done;
  sprintf(buf, "%d", Days);
  if (RmAddTaskArgument(task, nextarg++, buf) != RmE_Success) goto done;
  sprintf(buf, "%d", Hours);
  if (RmAddTaskArgument(task, nextarg++, buf) != RmE_Success) goto done;
  sprintf(buf, "%d", Minutes);
  if (RmAddTaskArgument(task, nextarg++, buf) != RmE_Success) goto done;

	/* The private field is used to keep track of the next suitable	*/
	/* channel.							*/
  if (RmSetTaskPrivate(task, 4) != RmE_Success) goto done;

	/* Finally the task can be added to the taskforce and		*/
	/* mapped onto the processor.					*/
  if (RmAddtailTask(template, task) != task) goto done;
  if (RmMapTask(processor, task) != RmE_Success) goto done;

  ok = TRUE;
done:
  if (!ok)
   Fatal("failed to create suitable task for processor %s", RmGetProcessorId(processor));
  
  return(1);
}
   

/*}}}*/
/*{{{  link channels */
/*{{{  find_task() */
	/* Given a processor it is necessary to find the task mapped	*/
	/* to that processor. There is guaranteed to be only one. A	*/
	/* simple RmApplyMappedTasks() returning the task does the	*/
	/* trick.							*/
static int find_task(RmTask task, ...)
{ return((int) task);
}
/*}}}*/

	/* This routine is responsible for adding channels to the	*/
	/* taskforce corresponding to every link in the network		*/
static int add_channels(RmTask task, ...)
{ va_list	args;
  RmNetwork	obtained;
  RmProcessor	processor;
  RmProcessor	neighbour;
  RmTask	neighbour_task;
  int		number_links;
  int		destlink;
  int		link;
  char		buf[16];
  int		next_channel;
  int		neighbour_next_channel;

  va_start(args, task);
  obtained = va_arg(args, RmNetwork);
  va_end(args);

  processor = RmFollowTaskMapping(obtained, task);
  if (processor == (RmProcessor) NULL)
   Fatal("internal error, there is an unmapped task");

  number_links = RmCountLinks(processor);
  for (link = 0; link < number_links; link++)
   { neighbour = RmFollowLink(processor, link, &destlink);
     if ((neighbour == RmM_NoProcessor) ||
	 (neighbour == RmM_ExternalProcessor) ||
	 (neighbour == processor))
      continue;

     neighbour_task = (RmTask) RmApplyMappedTasks(neighbour, &find_task);
     if (neighbour_task == NULL)
      Fatal("there is no task mapped to neighbouring processor %s", RmGetProcessorId(neighbour));
      

	/* work out the channels to use					*/
     next_channel = RmGetTaskPrivate(task);
     RmSetTaskPrivate(task, next_channel + 2);
     neighbour_next_channel = RmGetTaskPrivate(neighbour_task);
     RmSetTaskPrivate(neighbour_task, neighbour_next_channel + 2);
     RmMakeChannel(task, next_channel + 1, neighbour_task, neighbour_next_channel);
     RmMakeChannel(neighbour_task, neighbour_next_channel + 1, task, next_channel);
     sprintf(buf, "-l%d.%d", link, next_channel);
     RmAddTaskArgument(task, RmCountTaskArguments(task), buf);
     sprintf(buf, "-l%d.%d", destlink, neighbour_next_channel);
     RmAddTaskArgument(neighbour_task, RmCountTaskArguments(neighbour_task), buf);

	/* stop this link being used twice				*/
     RmBreakLink(processor, link);
  }	/* for all the links on this processor				*/     

  return(0);
}
/*}}}*/
/*{{{  do_controller() */
	/* This routine is responsible for making one of the components	*/
	/* the controller and setting up the necessary channels.	*/
static void do_controller(RmTaskforce template)
{ RmTask	controller	= RmFirstTask(template);
  RmTask	next;
  int		next_channel;
  char		buf[16];

  next_channel = RmGetTaskPrivate(controller);
  sprintf(buf, "-c%d.%d", RmCountTasks(template) - 1, next_channel);
  RmAddTaskArgument(controller, RmCountTaskArguments(controller), buf);

  for (next = RmNextTask(controller); next != NULL; next = RmNextTask(next))
   { RmMakeChannel(next, 1, controller, next_channel++);
     RmMakeChannel(controller, next_channel++, next, 0);
   }
}
/*}}}*/

static int distribute_application(void)
{ RmNetwork	net;
  RmNetwork	obtained;
  RmTaskforce	template;
  RmTaskforce	executing;
  int		n;

  net = RmGetNetwork();
  if (net == NULL)
   Fatal("failed to get network details.");

  n = RmApplyProcessors(net, &net_filter);
  if (n == 0)
   Fatal("there are no suitable processors in the network.");

  obtained = RmObtainNetwork(net, FALSE, NULL);
  if ((obtained == (RmNetwork) NULL) || (RmCountProcessors(obtained) == 0))
   Fatal("failed to obtain any processors.");

  RmFreeNetwork(net);

  template = RmNewTaskforce();
  if (template == NULL)
   Fatal("out of memory building taskforce");
  RmSetTaskforceId(template, "hwtest");

  (void) RmApplyProcessors(obtained, &add_task, template);

  unless(NoLinks)
   (void) RmApplyTasks(template, &add_channels, obtained);

  do_controller(template);

  executing = RmExecuteTaskforce(obtained, template, NULL);
  if (executing == NULL)
   Fatal("failed to execute taskforce");

  if (Detach)
   { RmLeaveTaskforce(executing);
     return(EXIT_SUCCESS);
   }
  else
   return(RmWaitforTaskforce(executing));
}
/*}}}*/
/*{{{  component main() */
static int component_main(int argc, char **argv)
{ int	 i;
  int	 link_index		= 0;
  char	*controller_string	= NULL;

  unless(Fork(StackSize, &check_helios, 0))
   Fatal("out of memory starting check_helios()");

  for (i = 1; i < argc; i++)
   if (argv[i][0] == '-')
    switch(argv[i][1])
     { case 'l'	: start_link_threads(link_index++, &(argv[i][2]));
		  break;

	case 'm' :	MemoryInterval = atoi(&(argv[i][2]));
			break;

	case 'n' :	if (argv[i][2] == 'm')
			 NoMem = TRUE;
			else
			 NoFastmem = TRUE;
			break;

	case 'C' :	ExtraComms = TRUE;
			break;

	case 'c' :	controller_string = &(argv[i][2]);
			break;

	case 'v' :	Verbose = TRUE;
			break;
     }
  else
   { if (Days == -1)    Days = atoi(argv[i]);
     elif (Hours == -1) Hours = atoi(argv[i]);
     else Minutes = atoi(argv[i]);
   }

	/* Keep track of the number of links being tested.		*/
  Results.NumberLinks	= link_index;

	/* At this point the various link threads are up and running,	*/
	/* as is the Helios consistency checker. The on-chip memory	*/
	/* tester can be started up. The number of threads is		*/
	/* incremented to allow for the controller interaction. The	*/
	/* off-chip memory tester can now be started. Finally either	*/
	/* the controller or the worker routine can be invoked.		*/
  unless(NoFastmem)
   { NumberThreads++;
     unless(Fork(StackSize, &check_fast, 0))
      Fatal("out of memory starting on-chip memory tester.");
   }

  NumberThreads++;	/* to stop the main memory checker allocating	*/
			/* its buffer while the controller pipes are	*/
			/* initialised.					*/
  unless(NoMem)
   { NumberThreads++;
     unless(Fork(StackSize, &check_mem, 0))
      Fatal("out of memory starting main memory tester.");
   }

  if (controller_string == NULL)
   worker_routine();
  else
   controller_routine(controller_string);

  return(EXIT_SUCCESS);
}
/*}}}*/
/*{{{  main() */
/*
** main(). This program can be run in two different ways. It can be
** invoked from the command line by the user, or it can be run as
** a taskforce component.
*/
/*{{{  usage() */
static void usage(void)
{ fputs("hwtest: usage, hwtest [-v] [-d] [-m x] [-nl] [-nm] [-nf] [-c] [days] [hours] minutes\n", stderr);
  fputs("      : -v    produce verbose output.\n", stderr);
  fputs("      : -d    detach, run in the background.\n", stderr);
  fputs("      : -m20  check memory at 20 second intervals (default 10).\n", stderr);
  fputs("      : -nl   do not perform any links checking.\n", stderr);
  fputs("      : -nm   do not check off-chip memory.\n", stderr);
  fputs("      : -nf   do not check on-chip fast memory.\n", stderr);
  fputs("      : -c    perform additional communication across the network to test Helios\n", stderr);
  fputs("      : time  in days, hours and minutes.\n\n", stderr);
  fputs("For example: hwtest 14 0\n", stderr);
  fputs("         or: hwtest -v -m30 2 15 0\n", stderr);
  exit(EXIT_FAILURE);
}
/*}}}*/

int main(int argc, char **argv)
{ int	i;

  initialise_statics();
  hw_srand(time(NULL));

  if (!strcmp(argv[0], "<hwtest>"))
   return(component_main(argc, argv));

  printf("Hardware test suite version %s\n\n", VersionNumber);

  for (i = 1; i < argc; i++)
   if (argv[i][0] == '-')
    switch(argv[i][1])
     {  case 'v'	: Verbose = TRUE; break;
	case 'd'	: Detach  = TRUE; break;
	case 'm'	: if (argv[i][2] == '\0')
			   { if (++i >= argc) usage();
			     MemoryInterval = atoi(argv[i]);
			   }
			  else
			   MemoryInterval = atoi(&(argv[i][2]));
			  break;
	case 'n'	:
			  switch(argv[i][2])
			   { case 'l' : NoLinks	  = TRUE; break;
			     case 'm' : NoMem	  = TRUE; break;
			     case 'f' : NoFastmem = TRUE; break;
		  	     default  : usage();
			   }
			  break;

	case 'c'	: ExtraComms = TRUE; break;

	default	: usage();
     }
   else
     {  if (Days      == -1) Days    = atoi(argv[i]);
	elif (Hours   == -1) Hours   = atoi(argv[i]);
	elif (Minutes == -1) Minutes = atoi(argv[i]);
	else usage();
     }

  if (Days == -1)
   { fputs("hwtest: please specify a run time.\n", stderr);
     usage();
   }
  if (Minutes == -1) { Minutes = Hours; Hours = Days; Days = 0; }
  if (Minutes == -1) { Minutes = Hours; Hours = Days; Days = 0; }

  return(distribute_application());
}
/*}}}*/

