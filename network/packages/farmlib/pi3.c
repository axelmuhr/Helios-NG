/**
*** Pi3.c
***		Evaluating the value of pi using a Monte Carlo
***		simulation.
**/

#include <stdio.h>
#include <farmlib.h>
#include "pi3.h"

static void	pi3_producer(void);
static void	pi3_consumer(void);

int main(int argc, char **argv)
{
  FmFaultHandling 	= TRUE;
  FmProducer		= &pi3_producer;
  FmConsumer		= &pi3_consumer;
  FmWorker		= &Pi3_Worker;
  FmFastCode		= TRUE;
  FmFastStack		= TRUE;
  FmWorkerStack		= 1500;
  FmInitialise();
}

static void pi3_producer(void)
{ int		 number_seconds;
  Pi3_Job	*job;

  printf("Monte Carlo simulation to calculate pi.\n");
  printf("Please specify the duration of the run in seconds : ");
  fflush(stdout);
  scanf("%d", &number_seconds);

  job		= FmGetJobBuffer(sizeof(Pi3_Job));
  job->Seconds	= number_seconds;
  FmSendJob(Fm_All, TRUE, job);
}

static void pi3_consumer(void)
{ int		 number_experiments	= 0;
  int		 number_hits		= 0;
  int		 crashes		= 0;
  double	 pi;
  int		 i;
  Pi3_Reply	*reply;

  for (i = 0; i < FmNumberWorkers; i++)
   { reply = FmGetReply(i);
     if (reply == NULL)
      crashes++;
     else
      { number_experiments	+= reply->Experiments;
	number_hits		+= reply->Hits;
      }
   }

  pi = 4.0 * ((double) number_hits / (double) number_experiments);
  printf("Pi is approximately %g.\n", pi);
  if (crashes > 0)
   printf("%d workers crashed during the run.\n", crashes);
}



