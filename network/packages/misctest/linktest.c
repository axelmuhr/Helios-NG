/*------------------------------------------------------------------------
--                                                                      --
--           H E L I O S   N E T W O R K I N G   S O F T W A R E	--
--           ---------------------------------------------------	--
--									--
--			Link I/O Reliability Test Suite			--
--			-------------------------------			--
--                                                                      --
--             Copyright (C) 1993, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- linktest.c								--
--                                                                      --
--	Author:  BLV 18/1/93						--
--                                                                      --
------------------------------------------------------------------------*/
static char *rcsid = "$Header: /users/bart/hsrc/network/packages/hwtests1/RCS/hwtest.c,v 1.4 1993/01/15 18:20:00 bart Exp $";

/*{{{  version number and revision history */
#define VersionNumber	"1.00"

/*
** Revision history :
**	1.00	first version of the link test suite
*/
/*}}}*/
/*{{{  header files etc. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <time.h>
#include <helios.h>
#include <link.h>
#include <sem.h>
#include <syslib.h>
#include <gsp.h>
#include <servlib.h>
#include <codes.h>
#include <nonansi.h>
#include <rmlib.h>
#include <farmlib.h>
/*}}}*/
/*{{{  data structures, statics and manifests */
	/* To avoid overflow problems calculating some of the summary	*/
	/* information unsigned arithmetic is used in places.		*/
typedef unsigned int uint;

	/* Command line arguments.					*/
static	bool	 Verbose	= FALSE;
static	bool	 ReallyVerbose	= FALSE;
static	uint	 Days		= -1;
static	uint	 Hours		= -1;
static	uint	 Minutes	= -1;
static	bool	 RandomData	= FALSE;
static  bool	 ConstantData	= FALSE;
static	bool	 UniDirectional	= FALSE;

	/* This structure is used to report link statistics.	*/
typedef struct LinkStats {
	uint		LinkNumber;
	uint		ReadIterations;
	uint		ReadFailures;
	uint		ReadTimeouts;
	uint		ReadFatals;
	uint		WriteIterations;
	uint		WriteTimeouts;
	uint		WriteFatals;
} LinkStats;

#define MaxLinkTransfer		1024
#define DataPerIteration	2040	/* 8+16+32+64...+ 1024 */

	/* This structure is used to report the number of links, and	*/
	/* LinkStats information for every link. There is an upper	*/
	/* limit on the number of links for simplicity.			*/
#define MaxLinks		8
typedef struct FullStats {
	char		ProcessorName[IOCDataMax];
	uint		NumberLinks;
	LinkStats	LinkStats[MaxLinks];
} FullStats;

	/* This is used to hold the information associated with this	*/
	/* worker.							*/
static FullStats	Results;


	/* It is necessary to keep track of the various threads		*/
	/* started up to allow for sensible termination.		*/
static int		NumberThreads;
static Semaphore	ThreadsDone;

	/* When the controller decides that time is up it sends a	*/
	/* message to all the other components of the taskforce and	*/
	/* waits for replies. The Finished flag is checked regularly	*/
	/* by the various threads performing tests which then have to	*/
	/* synchronise for access to the Results structure and signal	*/
	/* to indicate when their results have been filled in.		*/
static	bool		Finished	= FALSE;
static	Semaphore	ResultsLock;

	/* This is used for diagnostics.				*/
char *ProcessorName = Results.ProcessorName;

	/* Control access to the C library statics			*/
static Semaphore Clib;

	/* Stacksize for the various threads running.			*/
#define StackSize	(3000 + MaxLinkTransfer)

	/* This is the number used when running on constant data mode	*/
#define LinkConstant	0x87654321

#if 0
#define Debug(a)
#else
#define Debug(a) Report a
#endif

	/* On start-up the program reads in a skiplink file which	*/
	/* specifies which links should be ignored. This file contains	*/
	/* processor names and link numbers. The file is processed by	*/
	/* the producer thread and broadcast to all the workers.	*/
static int	NumberSkips;
typedef struct	SkipInfo {
	char	ProcName[NameMax];	/* ignore pathname		*/
	int	Link;
} SkipInfo;

#define MaxSkips	256
static SkipInfo SkipTable[MaxSkips];

/*{{{  initialise statics */
	/* This routine is called from inside main() to initialise	*/
	/* the various semaphores etc. which cannot be done		*/
	/* automatically.						*/
static void initialise_statics(void)
{ int	i;

  InitSemaphore(&Clib, 1);
  MachineName(Results.ProcessorName);
  Results.NumberLinks		= 0;
  for (i = 0; i < MaxLinks; i++)
   { Results.LinkStats[i].LinkNumber		= 0;
     Results.LinkStats[i].ReadIterations	= 0;
     Results.LinkStats[i].ReadFailures		= 0;
     Results.LinkStats[i].ReadTimeouts		= 0;
     Results.LinkStats[i].ReadFatals		= 0;
     Results.LinkStats[i].WriteIterations	= 0;
     Results.LinkStats[i].WriteTimeouts		= 0;
     Results.LinkStats[i].WriteFatals		= 0;
   }

  NumberThreads	= 0;
  InitSemaphore(&ResultsLock, 1);
  InitSemaphore(&ThreadsDone, 0);
  NumberSkips	= 0;
}
/*}}}*/
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

  Wait(&Clib);
  va_start(args, str);
  fprintf(stderr, "linktest(%s): ", ProcessorName);
  vfprintf(stderr, str, args);
  fputs("\n", stderr);
  va_end(args);
  Signal(&Clib);
}

static void Fatal(char *str, ...)
{ va_list	args;

  Wait(&Clib);
  va_start(args, str);
  fprintf(stderr, "linktest(%s): ", ProcessorName);
  vfprintf(stderr, str, args);
  fputs("\n", stderr);
  va_end(args);
  exit(EXIT_FAILURE);
}
/*}}}*/
/*{{{  link routines */
/*
** There are four link routines. initialise_links() is invoked from
** inside the worker and is responsible for allocating suitable links
** and starting up rx and tx threads. link_receiver() and link_transmitter()
** perform the actual communication. tidy_links() frees and reconfigures the
** links. In addition there are rx and tx
** routines which detect when an operation has taken rather a long time.
*/

	/* The transmitter attempts to send this data structure to a	*/
	/* receiver at the other end of the link, to find out whether	*/
	/* or not that link is connected.				*/
typedef struct InitialInfo {
	char	ProcessorName[IOCDataMax];
	int	Link;
} InitialInfo;

/*{{{  rx and tx */
/*
** These two routines perform LinkIn() and LinkOut() but also keep track
** of the time taken, and will report if the time is excessive.
*/
static bool rx(int size, int link, void *buffer)
{ int	rc;
  int	timeouts	= 0;

  memset(buffer, 0, size);

  while (((rc = (int) LinkIn(size, link, buffer, 5 * OneSec)) == EK_Timeout) &&
	  (++timeouts < 5))
   Results.LinkStats[link].ReadTimeouts++;

  if (rc < Err_Null)
   { if (Verbose)
      Report("LinkIn() on link %d failed completely with %x", link, rc);
     Results.LinkStats[link].ReadFatals++;
     return(FALSE);
   }

  if (timeouts && Verbose)
   Report("LinkIn() on link %d succeeded after %d timeout%s", link, timeouts,
		(timeouts != 1) ? "s" : "");
  return(TRUE);
}

static bool tx(int size, int link, void *buffer)
{ int	rc;
  int	timeouts	= 0;

  while (((rc = (int) LinkOut(size, link, buffer, 5 * OneSec)) == EK_Timeout) &&
          (++timeouts < 5))
   Results.LinkStats[link].WriteTimeouts++;

  if (rc < Err_Null)
   { if (Verbose)
      Report("LinkOut() on link %d failed completely with %x", link, rc);
     Results.LinkStats[link].WriteFatals++;
     return(FALSE);
   }

  if (timeouts && Verbose)
   Report("LinkOut() on link %d succeeded after %d timeout%s", link, timeouts,
		(timeouts != 1) ? "s" : "");
  return(TRUE);
}
/*}}}*/
/*{{{  receiver */
/*
** The link receiver attempts to get some InitialInfo from a transmitter
** at the other end of the link. If this times out then the link is not
** connected. Otherwise it will go into a loop receiving data from the
** other end, until the other end has detected the terminate.
*/
static void link_receiver(int link)
{ InitialInfo		info;
  int			wbuf[MaxLinkTransfer / sizeof(int)];
  int			iteration;

  if (LinkIn(sizeof(InitialInfo), link, &info, 20 * OneSec) < Err_Null)
   { if (ReallyVerbose)
      Report("rx on link %d, failed to get any data", link);
     Signal(&ThreadsDone);
     return;
   }

  if (UniDirectional)
   { int x = strcmp(ProcessorName, info.ProcessorName);
     if ((x < 0) || ((x == 0) && (link < info.Link)))
      { if (ReallyVerbose)
         Report("rx on link %d, unidirectional so tx only", link);
        Signal(&ThreadsDone);
        return;
      }

	/* To allow the transmitting side to reproduce the	*/
	/* above test it will resend the message.		*/
     (void) LinkIn(sizeof(InitialInfo), link, &info, 20 * OneSec);
   }

  if (Verbose)
   Report("rx on link %d running with %s link %d", link, info.ProcessorName, info.Link);

  for (iteration = 0; ; iteration++)
   { int size;
     int i;

	/* receive the initial packet, the remote end's Finished flag	*/
     unless (rx(sizeof(int), link, wbuf)) break;
     if (wbuf[0])
	break;

     for (size = 8; size <= MaxLinkTransfer; size *= 2)
      { 
        unless (rx(size, link, wbuf)) break;

        if (RandomData)
         { for (i = 1; i < (size / sizeof(int)); i++)
	    wbuf[0] -= wbuf[i];
	   if (wbuf[0] != 0)
            { if (Verbose)
	       Report("rx on link %d, iteration %d, size %d, corruption detected", link, iteration, size);
	      Results.LinkStats[link].ReadFailures++;
	    }
	 }
	elif (ConstantData)
	 { for (i = 1; i < (size / sizeof(int)); i++)
	    if (wbuf[i] != LinkConstant)
	     { if (Verbose)
		Report("rx on link %d, iteration %d, size %d, corruption detected: buf[%d] is %x, not %x", link, iteration, size, i, wbuf[i], LinkConstant);
	       Results.LinkStats[link].ReadFailures++;
	       break;
	     }
	 }
        else
	 { for (i = 0; i < (size / sizeof(int)); i++)
	    if (wbuf[i] != iteration)
	     { if (Verbose)
		Report("rx on link %d, iteration %d, size %d, corruption detected: buf[%d] is %x, not %x", link, iteration, size, i, wbuf[i], iteration);
	       Results.LinkStats[link].ReadFailures++;
	       break;
	     }
	 }
      }
   }

  if (Verbose)
   Report("rx on link %d done", link);

  Wait(&ResultsLock);
  Results.LinkStats[link].ReadIterations = iteration;
  Signal(&ResultsLock);
  Signal(&ThreadsDone);
}
/*}}}*/
/*{{{  transmitter */
static void link_transmitter(int link)
{ InitialInfo		info;
  int			wbuf[MaxLinkTransfer / sizeof(int)];
  int			iteration;

  strcpy(info.ProcessorName, ProcessorName);
  info.Link = link;

  if (LinkOut(sizeof(InitialInfo), link, &info, 10 * OneSec) < Err_Null)
   { if (ReallyVerbose)
      Report("tx on link %d, failed to send initial data", link);
     Signal(&ThreadsDone);
     return;
   }

  if (UniDirectional)
   if (LinkOut(sizeof(InitialInfo), link, &info, 10 * OneSec) < Err_Null)
    { if (ReallyVerbose)
       Report("tx on link %d, unidirectional so rx only", link);
      Signal(&ThreadsDone);
      return;
    }

  if (ReallyVerbose)
   Report("tx on link %d running", link);

  for (iteration = 0; ; iteration++)
   { int	size;
     int	i;
     int	my_finished;

     my_finished	= (int) Finished;
     unless (tx(sizeof(int), link, &(my_finished))) break;
     if (my_finished) break;

     for (size = 8; size <= MaxLinkTransfer; size *= 2)
      { if (RandomData)
         { wbuf[0] = 0;
	   for (i = 1; i < (size / sizeof(int)); i++)
	    { wbuf[i]  = hw_rand();
	      wbuf[0] += wbuf[i];
	    }
	 }
	elif (ConstantData)
	 for (i = 0; i < (size / sizeof(int)); i++)
	  wbuf[i] = LinkConstant;
	else
	 for (i = 0; i < (size / sizeof(int)); i++)
	  wbuf[i] = iteration;

        unless (tx(size, link, wbuf)) break;
      }
   }

  if (Verbose)
   Report("tx on link %d done", link);

  Wait(&ResultsLock);
  Results.LinkStats[link].WriteIterations = iteration;
  Signal(&ResultsLock);
  Signal(&ThreadsDone);
}
/*}}}*/
/*{{{  initialise */
/*
** The initialise_links() routine is invoked from the worker. It scans
** the various links on this processor, ignores any links except
** unconnected ones, configures and allocates those links, and starts up
** transmitter and receiver threads.
*/
static void initialise_links(void)
{ LinkInfo	data;
  LinkConf	conf;
  int		i, j;

  for (i = 0; LinkData(i, &data) == Err_Null; i++)
   { if (data.Mode != Link_Mode_Null) continue;

     for (j = 0; j < NumberSkips; j++)
      if ((SkipTable[j].Link == i) && !strcmp(SkipTable[j].ProcName, objname(ProcessorName)))
       break;
     if (j != NumberSkips) 
      { if (Verbose)
	 Report("skipping link %d", j);
        continue;
      }

     conf.Mode		= Link_Mode_Dumb;
     conf.State		= Link_State_Null;
     conf.Id		= data.Id;
     conf.Flags		= data.Flags;

     if (Configure(conf) != Err_Null)
      continue;

     if (AllocLink(data.Id) != Err_Null)
      { conf.Mode	= Link_Mode_Null;
	conf.State	= Link_State_Null;
	(void) Configure(conf);
	continue;
      }

     unless (Fork(StackSize, &link_transmitter, sizeof(int), conf.Id) &&
	     Fork(StackSize, &link_receiver, sizeof(int), conf.Id))
      Fatal("out of memory starting threads for link %d", conf.Id);

     NumberThreads += 2;
   }   

  Results.NumberLinks = i;
}
/*}}}*/
/*{{{  tidy */
/*
** This routine is called by the worker once all the results have been
** gathered, and is responsible for releasing allocated links and
** reconfiguring them back to not connected. This allows the test program
** to be run again without rebooting the machine.
*/
static void tidy_links(void)
{ LinkInfo	info;
  LinkConf	conf;
  int		i;

  for (i = 0; LinkData(i, &info) == Err_Null; i++)
   { if (info.Mode != Link_Mode_Dumb) continue;

     FreeLink(info.Id);
     conf.Mode		= Link_Mode_Null;
     conf.State		= Link_State_Null;
     conf.Id		= info.Id;
     conf.Flags		= info.Flags;
     Configure(conf);
   }
}
/*}}}*/
/*}}}*/
/*{{{  farm library routines */
/*{{{  producer */
/*
** First the producer reads in the skip file and broadcasts the information
** to all the workers. Then the producer goes to sleep for the amount of 
** time specified by the user. It then broadcasts an empty message which
** causes the workers to set the Finished flag, eventually resulting in 
** program termination.
*/
/*{{{  read skip file */
static void read_skipfile(void)
{ char		*filename	= getenv("LINKTEST_SKIPFILE");
  FILE		*skip_file;
  char		 buf[128];

  if (filename == NULL) filename = "linkskip";
  skip_file = fopen(filename, "r");
  if (skip_file == NULL) return;

  for (NumberSkips = 0; NumberSkips < MaxSkips; )
   { if (fgets(buf, 127, skip_file) == NULL) break;
     if (sscanf(buf, "%s %d", SkipTable[NumberSkips].ProcName, &(SkipTable[NumberSkips].Link)) == 2)
      NumberSkips++;
   }
  if (NumberSkips == MaxSkips)
   Report("Warning: too many entries in the linkskip file.");
  fclose(skip_file);
}
/*}}}*/

static void linktest_producer(void)
{ void		*terminate_packet;
  int		*skipsize_packet;
  SkipInfo	*skipdata_packet;
  uint	 	 minutes;

  read_skipfile();

  skipsize_packet	= FmGetJobBuffer(sizeof(int));
  skipsize_packet[0]	= NumberSkips;
  FmSendJob(Fm_All, FALSE, skipsize_packet);

  if (NumberSkips > 0)
   { skipdata_packet	= FmGetJobBuffer(NumberSkips * sizeof(SkipInfo));
     memcpy(skipdata_packet, SkipTable, NumberSkips * sizeof(SkipInfo));
     FmSendJob(Fm_All, FALSE, skipdata_packet);
   }

  minutes = (((Days * 24) + Hours) * 60) + Minutes;
  while (minutes-- > 0)
   Delay(60 * OneSec);

  terminate_packet = FmGetJobBuffer(0);
  FmSendJob(Fm_All, TRUE, terminate_packet);
}
/*}}}*/
/*{{{  consumer */
/*
** The consumer has to accept results packets from all the workers. In
** verbose mode it will give full details from all the workers. Otherwise
** it will only output a little summary. In addition it maintains a logfile.
*/
/*{{{  data structures etc */
	/* This structure is used to hold summary information, as	*/
	/* stored in the logfile.					*/
typedef struct SummaryStats {
	uint	NumberLinks;
	uint	LinkG;
	uint	LinkM;
	uint	LinkK;
	uint	Failures;
	uint	ReadTimeouts;
	uint	WriteTimeouts;
	uint	ReadFatals;
	uint	WriteFatals;
} SummaryStats;

	/* This is the information held in the logfile.			*/
static SummaryStats	LogStats;

	/* This is the information for this run.			*/
static SummaryStats	TotalStats;

	/* The logfile keeps track of the number of runs.		*/
static int		NumberRuns	= 0;

	/* And the number of hours and minutes				*/
static int		NumberHours	= 0;
static int		NumberMinutes	= 0;

	/* These strings are used for input from and output to the	*/
	/* logfile, for consistency.					*/
static char *str1 = "\t\tHelios Raw Link Reliability Test Suite";
static char *str2 = "\t\t======================================";
static char *str3 = "Summary Information after %d runs (%d hours %d mins) : %s";
static char *str4 = "Transfer   : %6d GBytes, %4d MBytes, %4d KBytes";
static char *str5 = "Failures   : %d";
static char *str6 = "Timeouts   : %d on LinkIn (%d fatals), %d on LinkOut (%d fatals)";

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
  fprintf(tmp, str3, 0, 0, 0, "today"); fputs("\n\n", tmp);
  fprintf(tmp, str4, 0, 0, 0); fputs("\n", tmp);
  fprintf(tmp, str5, 0); fputs("\n", tmp);
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
  char		 buf[91];
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
      (sscanf(buf, str3, &NumberRuns, &NumberHours, &NumberMinutes, buf2) != 3) ||
      (fgets(buf, 90, tmp) == NULL) ||		/* blank line	*/
      (fgets(buf, 90, tmp) == NULL) ||
      (sscanf(buf, str4, &(LogStats.LinkG), &(LogStats.LinkM),
		 &(LogStats.LinkK))
	!= 3) ||
      (fgets(buf, 90, tmp) == NULL) ||
      (sscanf(buf, str5, &(LogStats.Failures)) != 1) ||
      (fgets(buf, 90, tmp) == NULL) ||
      (sscanf(buf, str6, &(LogStats.ReadTimeouts), &(LogStats.ReadFatals),
			 &(LogStats.WriteTimeouts), &(LogStats.WriteFatals))
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
  fprintf(tmp, str3, ++NumberRuns, NumberHours, NumberMinutes, realnow); fputs("\n\n", tmp);
  fprintf(tmp, str4,	LogStats.LinkG, LogStats.LinkM, LogStats.LinkK);
  fputs("\n", tmp);
  fprintf(tmp, str5,	LogStats.Failures);
  fputs("\n", tmp);
  fprintf(tmp, str6,	LogStats.ReadTimeouts, LogStats.ReadFatals,
			LogStats.WriteTimeouts,LogStats.WriteFatals);
  fputs("\n", tmp);
	/* Allow for the numbers to shrink as well as grow		*/
  fputs("                                                    ", tmp);

	/* Now append the results for this run to the logfile		*/
  if (fseek(tmp, 0, SEEK_END) != 0)
   Fatal("failed to seek to end of logfile.");

  fprintf(tmp, "Run %d on %d links, %s, %d Days, %d Hours, %d Minutes\n",
		NumberRuns, TotalStats.NumberLinks, realnow, Days, Hours, Minutes);
  fprintf(tmp, str4,	TotalStats.LinkG, TotalStats.LinkM, TotalStats.LinkK);
  fprintf(tmp, str5,	TotalStats.Failures);
  fprintf(tmp, str6,	TotalStats.ReadTimeouts,  TotalStats.ReadFatals,
			TotalStats.WriteTimeouts, TotalStats.WriteFatals);
  fputs("\n\n", tmp);
  fclose(tmp);
}

/*}}}*/

static void linktest_consumer(void)
{ FullStats	*reply;
  int		 i, j;
  uint		 number_links		= 0;
  uint		 read_iterations	= 0;
  uint		 write_iterations	= 0;

  logname = getenv("LINKTEST_LOGFILE");
  if (logname == NULL)
   logname = "linktest.log";

/*{{{  collect data from all the workers */

  TotalStats.WriteTimeouts	=
  TotalStats.ReadTimeouts	=
  TotalStats.Failures		= 0;

  for (i = 0; i < FmNumberWorkers; i++)
   { reply	= FmGetReply(i);

     if (ReallyVerbose)
      { Report("\rResults from processor %s", reply->ProcessorName);
	for (j = 0; j < reply->NumberLinks; j++)
	 if (reply->LinkStats[j].WriteIterations || reply->LinkStats[j].ReadIterations)
	  { Report("\rLink %d: %d read iterations with %d failures and %d timeouts (%d fatal)",
		j, reply->LinkStats[j].ReadIterations, reply->LinkStats[j].ReadFailures,
		 reply->LinkStats[j].ReadTimeouts, reply->LinkStats[j].ReadFatals);
	    Report("\rLink %d: %d write iterations with %d timeouts (%d fatal)",
		j, reply->LinkStats[j].WriteIterations, reply->LinkStats[j].WriteTimeouts,
		reply->LinkStats[j].ReadFatals);
	  }
	Report("\r\033[K\n");
        
      }

     for (j = 0; j < reply->NumberLinks; j++)
      { unless(reply->LinkStats[j].ReadIterations || reply->LinkStats[j].WriteIterations)
	 continue;
	number_links++;
	read_iterations			 += reply->LinkStats[j].ReadIterations;
	write_iterations		 += reply->LinkStats[j].WriteIterations;
	TotalStats.ReadTimeouts 	 += reply->LinkStats[j].ReadTimeouts;
	TotalStats.WriteTimeouts	 += reply->LinkStats[j].WriteTimeouts;
	TotalStats.Failures	 	 += reply->LinkStats[j].ReadFailures;
	TotalStats.ReadFatals		 += reply->LinkStats[j].ReadFatals;
	TotalStats.WriteFatals		 += reply->LinkStats[j].WriteFatals;
      }
   }
/*}}}*/
/*{{{  calculate totals */

  number_links /= 2;
  if (read_iterations != write_iterations)
   Report("\rFunny, there is a sizes mismatch.");

  TotalStats.NumberLinks	 = number_links;
  TotalStats.LinkK  		 = (read_iterations * DataPerIteration) / 1024;
  TotalStats.LinkM  		 = TotalStats.LinkK / 1024;
  TotalStats.LinkK 		%= 1024;
  TotalStats.LinkG 		 = TotalStats.LinkM / 1024;
  TotalStats.LinkM 		%= 1024;
/*}}}*/
/*{{{  output totals for this run */
     if (Verbose)
      { Report("\rTotals for this run, which involved %d links:", number_links);
        Report("\rData transferred: %6d GBytes, %4d MBytes, %4d KBytes",
		TotalStats.LinkG, TotalStats.LinkM, TotalStats.LinkK);
	Report("\rThere were %d cases of data corruption", TotalStats.Failures);
	Report("\rThere were %d timeouts on LinkIn() and %d timeouts on LinkOut()",
		TotalStats.ReadTimeouts, TotalStats.WriteTimeouts);
	Report("\r\033[K");
      }
/*}}}*/

     readin_logfile();
/*{{{  update logfile info */

  LogStats.LinkK		+= TotalStats.LinkK;
  LogStats.LinkM		+= TotalStats.LinkM;
  LogStats.LinkG		+= TotalStats.LinkG;
  LogStats.LinkM		+= (LogStats.LinkK / 1024);
  LogStats.LinkK		%= 1024;
  LogStats.LinkG		+= (LogStats.LinkM / 1024);
  LogStats.LinkM		%= 1024;
  LogStats.WriteTimeouts	+= TotalStats.WriteTimeouts;
  LogStats.ReadTimeouts		+= TotalStats.ReadTimeouts;
  LogStats.Failures		+= TotalStats.Failures;
  LogStats.ReadFatals		+= TotalStats.ReadFatals;
  LogStats.WriteFatals		+= TotalStats.WriteFatals;
/*}}}*/
     writeout_logfile(); 
}
/*}}}*/
/*{{{  worker */
/*
** The worker routine gets three packets from the producer. The first
** indicates the number of links that should be skipped. If non-zero then
** the second packet contains the actual skip data. At this point the
** links can be initialised and the receivers and transmitters can
** start running. Finally the worker waits for a terminate packet from
** the producer to indicate the end of the run. At this point the worker
** sets the Finished flag to halt the various link I/O threads, waits for
** all the threads to finish, and sends the final results to the
** consumer.
*/
static void linktest_worker(void)
{ FullStats	*reply;
  int		*skipsize_packet;
  SkipInfo	*skipinfo_packet;
  void		*terminate_packet;

  skipsize_packet	= FmGetJob();
  NumberSkips		= skipsize_packet[0];
  if (NumberSkips > 0)
   { skipinfo_packet	= FmGetJob();
     memcpy(SkipTable, skipinfo_packet, NumberSkips * sizeof(SkipInfo));
   }

  initialise_links();

  terminate_packet	= FmGetJob();
  Finished		= TRUE;

  while (NumberThreads--)
   Wait(&ThreadsDone);

  reply = FmGetReplyBuffer(terminate_packet, sizeof(FullStats));
  memcpy(reply, &Results, sizeof(FullStats));
  FmSendReply(reply);
}
/*}}}*/
/*}}}*/
/*{{{  main */
/*
** main() is responsible only for parsing the arguments and setting up
** the farm library correctly.
*/

/*{{{  usage() */
static void usage(void)
{ fputs("linktest: usage, linktest [-v] [-1] [-r] [days] [hours] minutes\n", stderr);
  fputs("        : -v     produce verbose output.\n", stderr);
  fputs("        : -V     produce very verbose output.\n", stderr);
  fputs("        : -1     run uni-directionally.\n", stderr);
  fputs("        : -r     use random data for the communication.\n", stderr);
  fputs("        : -k     use the same constant data for the communication.\n", stderr);
  fputs("        : time   in days, hours and minutes.\n\n", stderr);
  fputs("For example: linktest 14 0\n", stderr);
  fputs("         or: linktest -v -r 2 15 0\n", stderr);
  exit(EXIT_FAILURE);
}
/*}}}*/

int main(int argc, char **argv)
{ int	i;

  unless(FmInWorker())
   printf("Link I/O test suite version %s\n\n", VersionNumber);

  initialise_statics();
  hw_srand(GetDate());

  for (i = 1; i < argc; i++)
   if (argv[i][0] == '-')
    switch(argv[i][1])
     { case 'v'		: Verbose	 = TRUE; break;
       case 'V'		: Verbose = ReallyVerbose = TRUE; break;
       case '1'		: UniDirectional = TRUE; break;
       case 'r'		: RandomData	 = TRUE; break;
       case 'k'		: ConstantData	 = TRUE; break;
       default		: usage();
     }
    else
     { if   (Days == -1)	Days	= atoi(argv[i]);
       elif (Hours == -1)	Hours	= atoi(argv[i]);
       elif (Minutes == -1)	Minutes = atoi(argv[i]);
       else usage();
      }

  if (Days == -1)
   { fputs("linktest: please specify a run time.\n", stderr);
     usage();
   }

  if (Minutes == -1) { Minutes = Hours; Hours = Days; Days = 0; }
  if (Minutes == -1) { Minutes = Hours; Hours = Days; Days = 0; }

	/* the links on this processor should be checked as well.	*/
  FmOverloadController	= TRUE;
  FmJobSize		= 0;
  FmReplySize		= sizeof(FullStats);
  FmProducer		= &linktest_producer;
  FmConsumer		= &linktest_consumer;
  FmWorker		= &linktest_worker;
  FmWorkerExit		= &tidy_links;
/*FmDebugFlags = -1;*/
  FmInitialise();
}
/*}}}*/

