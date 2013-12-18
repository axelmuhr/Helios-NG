#include <helios.h>
#include <stdio.h>
#include <stdlib.h>
#include <posix.h>
#include <lb.h>
#include <sem.h>
#include <nonansi.h>

typedef struct job_data {
	LB_HEADER	header;
	int		start;
} job_data;

typedef struct reply_data {
	LB_HEADER	header;
	int		best;
	int		count;
} reply_data;

static void process_job(job_data *, reply_data *);
static int  square_root(int);

int main(void)
{ job_data	job;
  reply_data	reply;
	
  forever
   { read(0, (BYTE *) &job, sizeof(job_data));
     if ((job.header.control & LB_FN) == Fn_Terminate)
      exit(0);

     process_job(&job, &reply);
     reply.header.control = 0;
     reply.header.size    = sizeof(reply_data) - sizeof(LB_HEADER);
     write(1, (BYTE *) &reply, sizeof(reply_data));
   }

  return(0);
} 

static void process_job(job_data *job, reply_data *reply)
{ int x, i, root, number_factors;

  reply->count = -1;
  
  for (x = job->start; x < job->start + 100; x++)
   { number_factors = 2;
     root = square_root(x);
     
     for (i = 2; i <= root; i++)
      if (x % i == 0)
       number_factors += 2;
       
     if (root * root == x)
      number_factors--;
      
     if (number_factors > reply->count)
      { reply->count = number_factors;
        reply->best  = x;
      }
   }
}

	/* evaluate a square root without using floating point */
	/* five iterations of the Newton-Raphson method with a */
	/* starting point of sqrt(1,500,000,000) will suffice  */
static int square_root(int x)
{ int estimate = 38730, i;

  for (i = 0; i < 5; i++)
   estimate = (estimate + (x / estimate)) / 2;

  return(estimate);
}

      
