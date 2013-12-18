/**
*** pi.c
***	This program uses the farm library to estimate a value
***	for pi by an integration.
**/
#include <stdio.h>
#include <farmlib.h>
#include "pi.h"

static void pi_producer(void);
static void pi_consumer(void);
static void pi_worker(void);

int main(int argc, char **argv)
{
  FmProducer	= &pi_producer;
  FmConsumer	= &pi_consumer;
  FmWorker	= &pi_worker;
  FmInitialise();
}

static void pi_producer(void)
{ int		number_intervals;
  int		i;
  Pi_Job	*job;

  printf("Farm Library version of Pi with %d workers.\n",
		FmNumberWorkers);
  printf("Please specify the number of intervals per worker : ");
  fflush(stdout);
  scanf("%d", &number_intervals);

  for (i = 0; i < FmNumberWorkers; i++)
   { job		= FmGetJobBuffer(sizeof(Pi_Job));
     job->WorkerNumber	= i;
     job->NumberWorkers	= FmNumberWorkers;
     job->Intervals	= number_intervals;
     FmSendJob(i, TRUE, job);
   }
}

static void pi_consumer(void)
{ Pi_Reply	*reply;
  int		 i;
  double	 total = 0.0;

  for (i = 0; i < FmNumberWorkers; i++)
   { reply	 = FmGetReply(i);
     total	+= reply->PartialArea;
   }

  printf("Pi is approximately %g.\n", total);
}

static void pi_worker(void)
{ Pi_Job	*job;
  Pi_Reply	*reply;
  double	 sum;		/* Partial area for this range		*/
  double	 width;		/* Width of one rectangle		*/
  double	 tmp;		/* X coordinate of current rectangle	*/
  int		 first;		/* Position of first rectangle		*/
  int		 current;	/* Loop counter, current rectangle	*/
  int		 last;		/* End of final rectangle		*/

  job	= FmGetJob();
  reply	= FmGetReplyBuffer(job, sizeof(Pi_Reply));

  sum	= 0.0;
  width	= 1.0 / (double) (job->NumberWorkers * job->Intervals);
  first	= job->WorkerNumber * job->Intervals;
  last	= first + job->Intervals;

  for (current = first; current < last; current++)
   { tmp = (0.5 + (double) current) * width;
     sum = sum + width * (4.0 / (1.0 + tmp * tmp));
   }

  reply->PartialArea = sum;
  FmSendReply(reply);
}
