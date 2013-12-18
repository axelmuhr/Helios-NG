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
-- testaux.c								--
--		Part of the farm library test harness, together with	--
--	farmtest.c							--
--                                                                      --
--	Author:  BLV 28/10/92						--
--                                                                      --
------------------------------------------------------------------------*/
/* RcsId: $Id: testaux.c,v 1.1 1992/10/30 19:00:21 bart Exp $ */

/*{{{  header files etc. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <signal.h>
#include <helios.h>
#include <syslib.h>
#include <gsp.h>
#include <rmlib.h>
#include <farmlib.h>
/*}}}*/
/*{{{  statics etc */
	/* Protect access to the C library			*/
static	Semaphore	Clib;

	/* Processor name for reporting purposes.		*/
static	char		ProcessorName[IOCDataMax];

/*{{{  initialise statics */
static void initialise_statics(void)
{ InitSemaphore(&Clib, 1);
  MachineName(ProcessorName);
}
/*}}}*/
/*}}}*/
/*{{{  utility and dummy routines */
/*{{{  output routines */
static	void Report(char *text, ...)
{ va_list	args;

  va_start(args, text);
  Wait(&Clib);
  vfprintf(stderr, text, args);
  Signal(&Clib);
  va_end(args);
}

static void Fatal(char *text, ...)
{ va_list	args;

  va_start(args, text);
  Wait(&Clib);
  vfprintf(stderr, text, args);
  Signal(&Clib);
  va_end(args);
  exit(EXIT_FAILURE);
}
/*}}}*/
/*{{{  dummy routines */
static void dummy_producer(void)
{ Delay(2 * OneSec);
}

static void dummy_consumer(void)
{ Delay(2 * OneSec);
}

static void dummy_worker(void)
{
}
/*}}}*/
/*{{{  reporting routines */

static void report_producer(void)
{ 
  Report("Producer running on %s.\n", ProcessorName);
  Delay(2 * OneSec);
}

static void report_consumer(void)
{
  Report("Consumer running on %s.\n", ProcessorName);
  Delay(2 * OneSec);
}

static void report_worker(void)
{ 
  Report("Worker %d running on %s.\n", FmWorkerNumber, ProcessorName);
}
/*}}}*/
/*{{{  random numbers */
#define MULT	1103515245
#define ADD	12345
#define MASK	0x7fffffff
static	int	my_seed;

int my_rand(void)
{ my_seed = ((my_seed * MULT) + ADD) & MASK;
  return(my_seed);
}
/*}}}*/
/*}}}*/
/*{{{  test routines */
/*{{{  basic operation 1 */
static void basic1(void)
{
}
/*}}}*/
/*{{{  basic operation 2 */
static void basic2(void)
{
}
/*}}}*/
/*{{{  flood network		x */
static void flood_network(void)
{ char	*text = "\
Fm_Network: this routine floods the network with workers. Every worker\n\
routine reports the name of the processor it is running on.\n\n\
";

  unless(FmInWorker()) Report(text);

  FmWorker	= &report_worker;
  FmProducer	= &report_producer;
  FmConsumer	= &dummy_consumer;
  FmFloodOption	= Fm_Network;
  FmInitialise();
}
/*}}}*/
/*{{{  flood domain		x */
static void domain_exit(void)
{
  Report("Exit routine : waiting a while before releasing the obtained processors.\n");
  Delay(3 * OneSec);
  system("domain free all >& /null");
}

static void flood_domain(void)
{ char	*text	= "\
Fm_Domain: this routine uses the domain command to obtain a number of\n\
processors and then floods the domain. All workers report the processor\n\
they are running on. The controller exit routine is responsible for\n\
releasing the obtained processors again afterwards.\n\n\
";

  unless (FmInWorker()) 
   { Report(text);
     system("domain get 4");
   }

  FmWorker		= &report_worker;
  FmProducer		= &report_producer;
  FmConsumer		= &dummy_consumer;
  FmControllerExit	= &domain_exit;
  FmFloodOption		= Fm_Domain;
  FmInitialise();
}
/*}}}*/
/*{{{  flood fixed		x */
static void flood_fixed(void)
{ char	*text = "\
Fm_Fixed: this test uses FmCountProcessors() to determine the number\n\
of processors available, halves this, and distributes the application\n\
over the resulting number of processors.\n\n\
";

  unless(FmInWorker())
   { int n = FmCountProcessors(Fm_Network);
     Report(text);
     FmNumberWorkers	= (n + 1) / 2;
     if (FmNumberWorkers < 2)
      Fatal("Fm_Fixed: not enough processors in the network.\n");
     Report("There should be %d workers.\n", FmNumberWorkers);
   }

  FmFloodOption	= Fm_Fixed;
  FmWorker	= &report_worker;
  FmProducer	= &report_producer;
  FmConsumer	= &dummy_consumer;
  FmInitialise();
}
/*}}}*/
/*{{{  flood fixed single		x */
static void flood_single(void)
{ char	*text = "\
Fm_Fixed (1) : this option checks the extreme case of a single worker.\n\n\
";

  unless (FmInWorker()) Report(text);
  FmFloodOption		= Fm_Fixed;
  FmNumberWorkers	= 1;
  FmWorker		= &report_worker;
  FmProducer		= &report_producer;
  FmConsumer		= &dummy_consumer;
  FmInitialise();
}
/*}}}*/
/*{{{  flood fixed many		x */
static void flood_many(void)
{ char	*text = "\
Fm_Fixed (many) : this option determines the number of suitable processors\n\
using FmCountProcessors() and then runs two workers per processors.\n\n\
";

  unless (FmInWorker())
   { int n		= FmCountProcessors(Fm_Network);
     Report(text);
     FmNumberWorkers	= 2 * n;
     FmFloodOption	= Fm_Fixed;
     Report("There should be %d workers.\n", FmNumberWorkers);
   }
  FmWorker	= &report_worker;
  FmProducer	= &report_producer;
  FmConsumer	= &dummy_consumer;
  FmInitialise();
}
/*}}}*/
/*{{{  flood processors		x */
/*{{{  flood filter function */
static int flood_filter(RmProcessor processor, ...)
{ int	purpose	= RmGetProcessorPurpose(processor);
  int	state	= RmGetProcessorState(processor);
  int	owner	= RmGetProcessorOwner(processor);
  bool	ok	= FALSE;

  if (purpose & RmP_System) goto done;
  if ((purpose & RmP_Mask) != RmP_Helios) goto done;
  unless (state & RmS_Running) goto done;
  if (owner != RmO_FreePool)
   { if (owner != RmWhoAmI()) goto done;
     if (RmGetProcessorSession(processor) != RmGetSession()) goto done;
   }
  ok = TRUE;

done:
  if (my_rand() & 0x0100) ok = FALSE;
  if (ok)
   Report("Processor %s has been selected.\n", RmGetProcessorId(processor));
  else
   RmFreeProcessor(RmRemoveProcessor(processor));
  return(0);
}
/*}}}*/

static void flood_processors(void)
{ char	*text = "\
Fm_Processors : in this test the network is examined for suitable\n\
processors. Unsuitable processors are rejected completely. Suitable\n\
ones are rejected or accepted randomly. Accepted processors are reported.\n\n\
";
  RmNetwork	network;

  unless (FmInWorker())
   { Report(text);
     network	= RmGetNetwork();
     if (network == NULL)
      Fatal("Failed to get network details.\n");
     my_seed = time(NULL) ^ (int) network;
     (void) RmApplyProcessors(network, flood_filter);
     if (RmCountProcessors(network) < 1)
      Fatal("Failed to select any processors.\n");
   }
  FmFloodOption		= Fm_Processors;
  FmSelectedNetwork	= network;
  FmWorker		= &report_worker;
  FmProducer		= &report_producer;
  FmConsumer		= &dummy_consumer;
  FmInitialise();
}
/*}}}*/
/*{{{  FmCountProcessors		x */
static void count_processors(void)
{ int		result;
  RmNetwork	domain;

  Report("FmCountProcessors: checking behaviour.\n\n", stdout);
  result = FmCountProcessors(Fm_Network);
  Report("FmCountProcessors(Fm_Network) returned %d.\n", result);

  result = FmCountProcessors(Fm_Domain);
  Report("FmCountProcessors(Fm_Domain) returned %d.\n", result);

  domain = RmGetDomain();
  if (domain == NULL)
   Fatal("farmtest: internal error, failed to get domain details.\n", stderr);

  Report("Currently there are %d processor(s) in the domain, attempting to get some more.\n",
		RmCountProcessors(domain));
  RmFreeNetwork(domain);

  system("domain get 4");

  domain = RmGetDomain();
  if (domain == NULL)
   Fatal("farmtest: internal error, failed to get new domain details.\n", stderr);

  Report("The domain now has %d processors.\n", RmCountProcessors(domain));
  RmFreeNetwork(domain);

  result = FmCountProcessors(Fm_Domain);
  Report("FmCountProcessors(Fm_Domain) now returned %d.\n", result);

  result = FmCountProcessors(Fm_Network);
  Report("FmCountProcessors(Fm_Network) now returned %d.\n", result);

  Report("Releasing processors again.\n", stdout);

  system("domain free all");
}
/*}}}*/
/*{{{  FmNumberWorkers		x */
static void number_workers_producer(void)
{ 
  Report("Producer on %s, there are %d workers\n", ProcessorName, FmNumberWorkers);
  Delay(2 * OneSec);
}

static void number_workers_worker(void)
{ 
  Report("Worker %d on %s, there are %d workers\n",
		FmWorkerNumber, ProcessorName, FmNumberWorkers);
}

static void number_workers(void)
{ char *text = "\
FmNumberWorkers: the producer and all the worker routines display the\n\
value of FmNumberWorkers.\n\n\
";

  unless(FmInWorker()) Report(text);
  FmProducer	= &number_workers_producer;
  FmConsumer	= &dummy_consumer;
  FmWorker	= &number_workers_worker;
  FmInitialise();
}
/*}}}*/
/*{{{  FmOverloadController	x */
static void overload_producer(void)
{ 
  Report("Producer on %s, there are %d workers\n", ProcessorName, FmNumberWorkers);
  Delay(2 * OneSec);
}

static void overload_worker(void)
{ 
  Report("Worker %d on %s, there are %d workers\n",
		FmWorkerNumber, ProcessorName, FmNumberWorkers);
}

static void overload_controller(void)
{ char *text = "\
FmOverloadController: the producer and all the worker routines display the\n\
value of FmNumberWorkers. The last worker should be on the same processor\n\
as the producer and consumer.\n\n\
";

  unless (FmInWorker()) Report(text);

  FmProducer		= &overload_producer;
  FmConsumer		= &report_consumer;
  FmWorker		= &overload_worker;
  FmOverloadController	= TRUE;
  FmInitialise();
}
/*}}}*/
/*{{{  overload single		x */
static void single_worker(void)
{ void	*job;
  void	*reply;

  Report("Worker %d running on %s\n", FmWorkerNumber, ProcessorName);

  forever
   { job	= FmGetJob();
     reply	= FmGetReplyBuffer(job, 4321);
     FmSendReply(reply);
   }
}

static void single_producer(void)
{ void	*job;
  int	 i;

  Report("Producer running on %s\n", ProcessorName);

  for (i = 0; i < 5; i++)
   { job = FmGetJobBuffer(128);
     FmSendJob(Fm_Any, TRUE, job);
   }
}

static void single_consumer(void)
{ void	*reply;
  int	 i;

  for (i = 0; i < 5; i++)
   reply = FmGetReply(Fm_Any);
}

static void overload_single(void)
{ char *text = "\
FmOverloadController (1) : in this test a single worker is specified plus\n\
the FmOverloadController flag. Hence there is no need to run any additional\n\
worker programs. This situation matches the Tiny Helios world.\n\n\
";

  unless (FmInWorker()) Report(text);
  FmFloodOption		= Fm_Fixed;
  FmNumberWorkers	= 1;
  FmOverloadController	= TRUE;
  FmWorker		= &single_worker;
  FmProducer		= &single_producer;
  FmConsumer		= &single_consumer;
  FmInitialise();
}
/*}}}*/
/*{{{  FmWorkerNumber		x */
static void worker_number(void)
{ char *text = "\
FmWorkerNumber: all the workers should report their number.\n\n\
";

  unless (FmInWorker()) Report(text);
  FmProducer		= &dummy_producer;
  FmConsumer		= &dummy_consumer;
  FmWorker		= &report_worker;
  FmInitialise();
}
/*}}}*/
/*{{{  FmInWorker			x */
static void in_worker(void)
{ char *text = "\
FmInWorker(): this routine is invoked inside the controller and all the\n\
workers and the results are displayed. The producer routine gives the\n\
name of the processor it is running on to allow the results to be\n\
verified.\n\n\
";

  unless (FmInWorker()) Report(text);

  Report("Processor %s: FmInWorker() returned %d\n", ProcessorName, FmInWorker());

  FmProducer		= &report_producer;
  FmConsumer		= &dummy_consumer;
  FmWorker		= &dummy_worker;
  FmInitialise();
}
/*}}}*/
/*{{{  FmFastCode			x */
extern void fastcode_worker(void);

static void fast_code(void)
{ char *text = "\
FmFastCode: the worker routines will output their addresses which should be\n\
somewhere in on-chip memory.\n\n\
";

  unless(FmInWorker()) Report(text);

  FmProducer	= &dummy_producer;
  FmConsumer	= &dummy_consumer;
  FmWorker	= &fastcode_worker;
  FmFastCode	= TRUE;
  FmInitialise();
}
/*}}}*/
/*{{{  FmFastStack		x */
static void fast_stack_worker(void)
{ int	x;

  Report("Worker %d running on %s, stack variable is at %p\n",
		FmWorkerNumber, ProcessorName, &x);
}

static void fast_stack(void)
{ char	*text = "\
FmFastStack: all the workers should display the address of one variable\n\
on their stack. This address should be in on-chip RAM.\n\n\
";

  unless (FmInWorker()) Report(text);

  FmProducer	= &dummy_producer;
  FmConsumer	= &dummy_consumer;
  FmWorker	= &fast_stack_worker;
  FmFastStack	= TRUE;
  FmWorkerStack	= 3000;
  FmInitialise();
}
/*}}}*/
/*{{{  info 1			x */
/*
** This test involves a single request with every worker requesting some
** info and then sending back a reply. The aim is to check the provider
** code while in state ProcessingPrimary.
*/
typedef struct info1_inforeq {
	int	Value;
} info1_inforeq;

typedef struct info1_inforeply {
	int	Value;
} info1_inforeply;

static void info1_worker(void)
{ void			*job;
  void			*reply;
  info1_inforeq		*req;
  info1_inforeply	*rep;
 
  job		= FmGetJob();
  reply		= FmGetReplyBuffer(job, 1);

  req		= FmGetInfoRequestBuffer(sizeof(info1_inforeq));
  req->Value	= FmWorkerNumber;
  rep		= FmGetInfo(req);

  if (rep->Value != (99 - FmWorkerNumber))
   Fatal("Worker %d, got wrong reply from the provider.\n", FmWorkerNumber);
  else
    Report("Worker %d, got info reply\n", FmWorkerNumber);

  FmSendReply(reply);
}

static void info1_producer(void)
{ int	 i;
  void	*job;

  for (i = 0; i < FmNumberWorkers; i++)
   { job = FmGetJobBuffer(3);
     FmSendJob(i, TRUE, job);
   }
}

static void info1_consumer(void)
{ int	 i;
  void	*reply;

  for (i = 0; i < FmNumberWorkers; i++)
   reply = FmGetReply(i);
  Report("Consumer: got all the replies.\n");
}

static void info1_provider(void)
{ info1_inforeq		*req;
  info1_inforeply	*rep;

  forever
   { req	= FmGetInfoRequest();
     Report("Provider: got a request from worker %d\n", req->Value);
     rep	= FmGetInfoReplyBuffer(req, sizeof(info1_inforeply));
     rep->Value	= 99 - req->Value;
     FmSendInfoReply(rep);
   }     
}

static void info1(void)
{ char	*text	= "\
Info1: in this test all the workers are sent a single request. While\n\
processing this request they perform an FmGetInfo(). The Provider\n\
and all the workers should report on this info handling.\n\n\
";

  unless(FmInWorker()) Report(text);
  FmWorker	= &info1_worker;
  FmProvider	= &info1_provider;
  FmProducer	= &info1_producer;
  FmConsumer	= &info1_consumer;
  FmInitialise();
}
/*}}}*/
/*{{{  info 2			x */
/*
** This test is very similar to info1 but all the workers are sent two
** packets and do not perform their info requests until after a delay.
** Hence the info requests arrive when a secondary packet has been sent.
*/
typedef struct info2_inforeq {
	char	Buf[512];
	int	Value;
} info2_inforeq;

typedef struct info2_inforeply {
	int	Value;
	char	Buf[1024];
} info2_inforeply;

static void info2_worker(void)
{ void			*job;
  void			*reply;
  info2_inforeq		*req;
  info2_inforeply	*rep;
 
  job		= FmGetJob();
  reply		= FmGetReplyBuffer(job, 4096);

  Delay(2 * OneSec);

  req		= FmGetInfoRequestBuffer(sizeof(info2_inforeq));
  req->Value	= FmWorkerNumber;
  rep		= FmGetInfo(req);

  if (rep->Value != (99 - FmWorkerNumber))
   Fatal("Worker %d, got wrong reply from the provider.\n", FmWorkerNumber);
  else
    Report("Worker %d, got info reply\n", FmWorkerNumber);

  FmSendReply(reply);

  job	= FmGetJob();
  reply	= FmGetReplyBuffer(job, 1234);
  FmSendReply(reply);
}

static void info2_producer(void)
{ int	 i, j;
  void	*job;

  for (i = 0; i < 2; i++)
   for (j = 0; j < FmNumberWorkers; j++)
    { job = FmGetJobBuffer(32768);
      FmSendJob(j, TRUE, job);
    }
}

static void info2_consumer(void)
{ int	 i, j;
  void	*reply;

  for (i = 0; i < 2; i++)
   for (j = 0; j < FmNumberWorkers; j++)
    reply = FmGetReply(j);

  Report("Consumer: got all the replies.\n");
}

static void info2_provider(void)
{ info2_inforeq		*req;
  info2_inforeply	*rep;

  forever
   { req	= FmGetInfoRequest();
     Report("Provider: got a request from worker %d\n", req->Value);
     rep	= FmGetInfoReplyBuffer(req, sizeof(info1_inforeply));
     rep->Value	= 99 - req->Value;
     FmSendInfoReply(rep);
   }     
}

static void info2(void)
{ char	*text	= "\
Info2: in this test all the workers are sent two requests. While\n\
processing the first request they sleep for a while and then\n\
request some information. The Provider and all the workers should\n\
report on this info handling.\n\n\
";

  unless(FmInWorker()) Report(text);
  FmWorker	= &info2_worker;
  FmProvider	= &info2_provider;
  FmProducer	= &info2_producer;
  FmConsumer	= &info2_consumer;
  FmInitialise();
}
/*}}}*/
/*{{{  FmRand			x */
/*
** This code tests the behaviour of the random number generators. This
** test program contains the same algorithm as the farm library. Every worker
** is asked for a series of random numbers and these are compared with what
** the internal random number generator produces. In addition a number of
** random numbers are displayed to allow the user to verify their random
** nature.
*/
typedef struct rand_reply {
	int	Value;
} rand_reply;

static void rand_worker(void)
{ void		*job;
  rand_reply	*reply;

  forever
   { job		= FmGetJob();
     reply		= FmGetReplyBuffer(job, sizeof(rand_reply));
     reply->Value	= FmRand();
     FmSendReply(reply);
   }
}

static void rand_producer(void)
{ int	 i, j;
  void	*job;

	/* Make the internal random number generator use the default seed */
  my_seed = FmSeed;

  for (i = 0; i < 5; i++)
   for (j = 0; j < FmNumberWorkers; j++)
    { job	= FmGetJobBuffer(0);
      FmSendJob(j, TRUE, job);
    }
}

static	void rand_consumer(void)
{ int		 i, j;
  rand_reply	*reply;
  int		 expected;

  for (i = 0; i < 4; i++)
   for (j = 0; j < FmNumberWorkers; j++)
    { reply	= FmGetReply(j);
      expected	= my_rand();
      if (reply->Value != expected)
       { Report("FmRand(): divergence detected in iteration %d, worker %d\n", i, j);
	 Report("        : worker returned %d, expected %d\n", reply->Value, expected);
	 return;
       }
    }

  for (j = 0; j < FmNumberWorkers; j++)
   { reply	= FmGetReply(j);
     Report("Worker %d produced random number %d\n", j, reply->Value);
   }
}

static void fm_rand(void)
{ char	*text = "\
FmRand(): in this test all the workers have to supply a series of random\n\
numbers in sequence, which are compared with the results produced by the\n\
same random number algorithm running sequentially. Also, the results produced\n\
by the workers in a final iteration are displayed so that the random nature\n\
can be checked.\n\n\
";

  unless (FmInWorker()) Report(text);
  FmWorker	= &rand_worker;
  FmProducer	= &rand_producer;
  FmConsumer	= &rand_consumer;
  FmJobSize	= 0;
  FmReplySize	= sizeof(rand_reply);
  FmInitialise();
}
/*}}}*/
/*{{{  FmSeed			x */
/*
** This code is very similar to the random number test above but
** uses an initial seed.
*/
typedef struct seed_reply {
	int	Value;
} seed_reply;

static void seed_worker(void)
{ void		*job;
  rand_reply	*reply;

  forever
   { job		= FmGetJob();
     reply		= FmGetReplyBuffer(job, sizeof(rand_reply));
     reply->Value	= FmRand();
     FmSendReply(reply);
   }
}

static void seed_producer(void)
{ int	 i, j;
  void	*job;

  for (i = 0; i < 5; i++)
   for (j = 0; j < FmNumberWorkers; j++)
    { job	= FmGetJobBuffer(0);
      FmSendJob(j, TRUE, job);
    }
}

static	void seed_consumer(void)
{ int		 i, j;
  rand_reply	*reply;
  int		 expected;

  for (i = 0; i < 4; i++)
   for (j = 0; j < FmNumberWorkers; j++)
    { reply	= FmGetReply(j);
      expected	= my_rand();
      if (reply->Value != expected)
       { Report("FmRand(): divergence detected in iteration %d, worker %d\n", i, j);
	 Report("        : worker returned %d, expected %d\n", reply->Value, expected);
	 return;
       }
    }

  for (j = 0; j < FmNumberWorkers; j++)
   { reply	= FmGetReply(j);
     Report("Worker %d produced random number %d\n", j, reply->Value);
   }
}

static void fm_seed(void)
{ char	*text = "\
FmSeed: in this test all the workers have to supply a series of random\n\
numbers in sequence, which are compared with the results produced by the\n\
same random number algorithm running sequentially. Also, the results produced\n\
by the workers in a final iteration are displayed so that the random nature\n\
can be checked. A random initial seed is generated.\n\n\
";

  unless (FmInWorker()) Report(text);
  FmWorker	= &seed_worker;
  FmProducer	= &seed_producer;
  FmConsumer	= &seed_consumer;
  FmSeed	= my_seed	= time(NULL) ^ (int) text;
  FmInitialise();
}
/*}}}*/
/*{{{  FmWorkerExit		x */
static void worker_exit_dummyexit(void)
{ 
  Report("Worker %d on processor %s, in exit routine.\n",
		FmWorkerNumber, ProcessorName);
}

static void worker_exit(void)
{ char *text = "\
FmWorkerExit: in this test a worker exit routine is installed. Every \n\
worker should produce a message, followed by another message from the \n\
exit routine.\n\n\
";

  unless(FmInWorker()) Report(text);

  FmProducer		= &dummy_producer;
  FmConsumer		= &dummy_consumer;
  FmWorker		= &report_worker;
  FmWorkerExit		= &worker_exit_dummyexit;
  FmInitialise();
}
/*}}}*/
/*{{{  FmWorkerInitialise		x */
static void worker_initialise_dummyinit(void)
{ 
  Report("Worker %d on processor %s, in initialise routine.\n",
		FmWorkerNumber, ProcessorName);
}

static void worker_initialise(void)
{ char *text = "\
FmWorkerInitialise: this routine installs a worker initialise routine \n\
which should produce a message in every worker, followed by another \n\
message from the actual worker routines.\n\n\
";

  unless (FmInWorker()) Report(text);

  FmProducer		= &dummy_producer;
  FmConsumer		= &dummy_consumer;
  FmWorker		= &report_worker;
  FmWorkerInitialise	= &worker_initialise_dummyinit;
  FmInitialise();
}
/*}}}*/
/*{{{  FmControllerExit		x */
static void controller_exit_dummyexit(void)
{
  Report("Controller on processor %s, in exit routine.\n", ProcessorName);
}

static void controller_exit(void)
{ char *text = "\
FmControllerExit: in this test a controller exit routine is installed.\n\
The producer and consumer should produce messages, followed by another\n\
message from the exit routine.\n\n\
";

  unless (FmInWorker()) Report(text);

  FmProducer		= &report_producer;
  FmConsumer		= &report_consumer;
  FmWorker		= &dummy_worker;
  FmControllerExit	= &controller_exit_dummyexit;
  FmInitialise();
}
/*}}}*/
/*{{{  FmControllerInitialise	x */
static void controller_initialise_dummyinit(void)
{
  Report("Controller on processor %s, in initialise routine.\n", ProcessorName);
}

static void controller_initialise(void)
{ char *text = "\
FmControllerInitialise: this routine installs a controller initialise routine\n\
which should produce a single message, followed by another message from the\n\
producer and consumer.\n\n\
";

  unless (FmInWorker()) Report(text);

  FmProducer			= &report_producer;
  FmConsumer			= &report_consumer;
  FmWorker			= &dummy_worker;
  FmControllerInitialise	= &controller_initialise_dummyinit;
  FmInitialise();
}
/*}}}*/
/*{{{  fault 1 */
static	void	fault1_worker(void)
{ void	*job;
  void	*reply;

  job	= FmGetJob();
  Delay(OneSec);
  if (FmWorkerNumber == (FmNumberWorkers / 2))
   { Report("Worker %d : raising SIGSEGV\n", FmWorkerNumber);
#if 0
     raise(SIGSEGV);
#else
     Exit(0x085);
#endif
   }
  Delay(3 * OneSec);
  reply = FmGetReplyBuffer(job, 32);
  FmSendReply(reply);
  Report("Worker %d : sent reply\n", FmWorkerNumber);
}

static void fault1_producer(void)
{ void	*job;
  int	 i;

  for (i = 0; i < FmNumberWorkers; i++)
   { job = FmGetJobBuffer(64);
     FmSendJob(i, TRUE, job);
   }
  Report("Producer: all jobs sent\n");
}

static void fault1_consumer(void)
{ void	*reply;
  int	 i;

  for (i = 0; i < FmNumberWorkers; i++)
   reply = FmGetReply(i);

  Report("Consumer: all replies received.\n");
}

static void fault1(void)
{ char *text = "\
Fault 1: in this test the FmFaultHandling option is not enabled and one\n\
of the workers will exit by raising SIGSEGV. The whole application should\n\
abort with a suitable error code.\n\n\
";

  unless (FmInWorker()) Report(text);
  FmWorker	= &fault1_worker;
  FmProducer	= &fault1_producer;
  FmConsumer	= &fault1_consumer;
  FmInitialise();
}
/*}}}*/
/*{{{  fault 2 */
static	void	fault2_worker(void)
{ void	*job;
  void	*reply;

  job	= FmGetJob();
  if (FmWorkerNumber == (FmNumberWorkers / 2))
   { Report("Worker %d : raising SIGSEGV\n", FmWorkerNumber);
#if 0
     raise(SIGSEGV);
#else
     Exit(0x085);
#endif
   }
  Delay(5 * OneSec);
  reply = FmGetReplyBuffer(job, 32);
  FmSendReply(reply);
  Report("Worker %d : sent reply\n", FmWorkerNumber);
}

static void fault2_producer(void)
{ void	*job;
  int	 i;

  for (i = 0; i < FmNumberWorkers; i++)
   { job = FmGetJobBuffer(64);
     FmSendJob(i, TRUE, job);
   }
  Report("Producer: all jobs sent\n");
}

static void fault2_consumer(void)
{ void	*reply;
  int	 i;

  for (i = 0; i < FmNumberWorkers; i++)
   { reply = FmGetReply(i);
     if (reply == NULL)
      Report("Consumer: worker %d has failed.\n", i);
   }

  Report("Consumer: all replies received.\n");
}

static void fault2(void)
{ char *text = "\
Fault 2: in this test every worker is sent a single job destined\n\
specifically for that worker. FmFaultHandling is enabled. One of the\n\
workers will generate a SIGSEGV but the application should finish.\n\
The consumer should report that one of the workers failed.\n\n\
";

  unless (FmInWorker()) Report(text);
  FmWorker		= &fault2_worker;
  FmProducer		= &fault2_producer;
  FmConsumer		= &fault2_consumer;
  FmFaultHandling	= TRUE;
  FmInitialise();
}
/*}}}*/
/*{{{  FmIsRunning */
static void is_running(void)
{
}
/*}}}*/
/*{{{  FmRunningWorkers */
static void running_workers(void)
{
}
/*}}}*/
/*}}}*/
/*{{{  table of tests */
/*
** The farmtest program used to run the library test suite invokes
** testaux with a single argument, the name of the test to be performed.
** There is a routine associated with each of these tests. This table
** holds the names of all the tests and their associated routines.
*/
typedef struct TestEntry {
	char		*Name;		/* of test		*/
	VoidFnPtr	 Routine;	/* to perform the test	*/
} TestEntry;

TestEntry Tests[] = {
	{ "basic operation 1",		&basic1			},
	{ "basic operation 2",		&basic2			},
	{ "flood network",		&flood_network		},
	{ "flood domain",		&flood_domain		},
	{ "flood fixed",		&flood_fixed		},
	{ "flood fixed single",		&flood_single		},
	{ "flood fixed many",		&flood_many		},
	{ "flood processors",		&flood_processors	},
	{ "FmCountProcessors",		&count_processors	},
	{ "FmNumberWorkers",		&number_workers		},
	{ "FmOverloadController",	&overload_controller	},
	{ "overload single",		&overload_single	},
	{ "FmRunningWorkers",		&running_workers	},
	{ "FmWorkerNumber",		&worker_number		},
	{ "FmInWorker",			&in_worker		},
	{ "FmIsRunning",		&is_running		},
	{ "FmFastCode",			&fast_code		},
	{ "FmFastStack",		&fast_stack		},
	{ "info 1",			&info1			},
	{ "info 2",			&info2			},
	{ "FmRand",			&fm_rand		},
	{ "FmSeed",			&fm_seed		},
	{ "FmWorkerExit",		&worker_exit		},
	{ "FmWorkerInitialise",		&worker_initialise	},
	{ "FmControllerExit",		&controller_exit	},
	{ "FmControllerInitialise",	&controller_initialise	},
	{ "fault 1",			&fault1			},
	{ "fault 2",			&fault2			},
	{ NULL,				NULL			}
};

/*}}}*/
/*{{{  main() */
int main(int argc, char **argv)
{ int	i;

  initialise_statics();

  if (argc != 2)
   { Report("testaux: this program should not be run directly.\n");
     Fatal("       : use farmtest to run the test harness.\n");
   }

  for (i = 0; Tests[i].Name != NULL; i++)
   if (!strcmp(Tests[i].Name, argv[1]))
    { (*(Tests[i].Routine))();
      exit(EXIT_SUCCESS);
    }

  Fatal("testaux: unknown test %s\n", argv[1]);
  return(EXIT_FAILURE);
}
/*}}}*/
