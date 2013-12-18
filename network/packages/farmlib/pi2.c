/**
*** pi2.c
***	 Alternative version of the pi example, using a broadcast
***	 to send the job packets and enabling speed-ups.
**/

#include <stdio.h>
#include <farmlib.h>
#include "pi2.h"

static void pi2_producer(void);
static void pi2_consumer(void);

int main(int argc, char **argv)
{ 
  FmProducer		= &pi2_producer;
  FmConsumer		= &pi2_consumer;
  FmWorker		= &Pi2_Worker;
  FmFastStack		= TRUE;
  FmWorkerStack		= 1500;
  FmFastCode		= TRUE;
  FmOverloadController	= TRUE;
  FmJobSize		= sizeof(Pi2_Job);
  FmReplySize		= sizeof(Pi2_Reply);
  FmInitialise();
}

static void pi2_producer(void)
{ int		number_intervals;
  Pi2_Job	*job;

  printf("Second Pi program using the Farm Library, running on %d workers.\n",
		FmNumberWorkers);
  printf("Please specify the number of intervals per workers : ");
  fflush(stdout);
  scanf("%d", &number_intervals);

  job			= FmGetJobBuffer(sizeof(Pi2_Job));
  job->Intervals	= number_intervals;
  FmSendJob(Fm_All, TRUE, job);
}

static void pi2_consumer(void)
{ Pi2_Reply	*reply;
  int		 i;
  double	 total = 0.0;

  for (i = 0; i < FmNumberWorkers; i++)
   { reply	 = FmGetReply(Fm_Any);
     total	+= reply->PartialArea;
   }

  printf("Pi is approximately %g.\n", total);
}

