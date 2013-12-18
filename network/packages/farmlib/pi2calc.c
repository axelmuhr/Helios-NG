/**
*** pi2calc.c
***		Calculation component of the second pi example. This is
***		in a separate module to facilitate the use of the
***		FmFastCode option.
**/

#include <farmlib.h>
#include "pi2.h"

void Pi2_Worker(void)
{ Pi2_Job	*job;
  Pi2_Reply	*reply;
  double	 sum;		/* Partial area for this range		*/
  double	 width;		/* Width of one rectangle		*/
  double	 tmp;		/* X coordinate of current rectangle	*/
  int		 first;		/* Position of first rectangle		*/
  int		 current;	/* Loop counter, current rectangle	*/
  int		 last;		/* End of final rectangle		*/

  job	= FmGetJob();
  reply	= FmGetReplyBuffer(job, sizeof(Pi2_Reply));

  sum	= 0.0;
  width	= 1.0 / (double) (FmNumberWorkers * job->Intervals);
  first	= FmWorkerNumber * job->Intervals;
  last	= first + job->Intervals;

  for (current = first; current < last; current++)
   { tmp = (0.5 + (double) current) * width;
     sum = sum + width * (4.0 / (1.0 + tmp * tmp));
   }

  reply->PartialArea = sum;
  FmSendReply(reply);
}

