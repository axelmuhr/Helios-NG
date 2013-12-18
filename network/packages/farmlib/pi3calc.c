/**
*** pi3calc.c
***		Calculation module of the Monte Carlo program to
***		calculate pi.
**/

#include <farmlib.h>
#include <time.h>
#include "pi3.h"

void Pi3_Worker(void)
{ double	 x, y;
  Pi3_Job	*job;
  Pi3_Reply	*reply;
  int		 i;
  time_t	 start;
  int		 hits;
  int		 experiments;

  job		= FmGetJob();
  reply		= FmGetReplyBuffer(job, sizeof(Pi3_Reply));

  hits		= 0;
  experiments	= 0;
  start		= time(NULL);

  while ((time(NULL) - start) < job->Seconds)
	/* Avoid polling the clock too often	*/
   for (i = 0; i < 10240; i++)
    { experiments++;
      x = (double) FmRand() / (double) Fm_MaxRand;
      y = 4.0 * (double) FmRand() / (double) Fm_MaxRand;
      if (4.0 / (1.0 + (x * x)) >= y)
       hits++;
    }

  reply->Experiments	= experiments;
  reply->Hits		= hits;
  FmSendReply(reply);
}

