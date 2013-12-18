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

int base, end;
Semaphore finished;

static void reader_process(void);
static void writer_process(void);

int main(int argc, char **argv)
{ LB_HEADER terminate;

  if ( argc < 3 )
    {
      printf("Usage: %s <start> <end>\n", argv[0]);
      printf("argc = %d, %s %s\n",argc,argv[1],argv[2]);
      exit(0);
    }
     	
  base = atoi(argv[1]);
  end  = atoi(argv[2]);
  
  InitSemaphore(&finished, 0);
  
  unless(Fork(2000, &reader_process, 0))
   { fprintf(stderr, "Unable to fork off reader process.\n");
     exit(1);
   }
   
  writer_process();  /* send all the job packets */
  
  Wait(&finished);   /* signalled by the reader process */

  terminate.control = LB_MASTER + Fn_Terminate;
  terminate.size    = 0;
  write(5, (BYTE *) &terminate, sizeof(LB_HEADER));
  return(0);
}

static void writer_process(void)
{ job_data data;
  int      i;
  
  data.header.control = 0;
  data.header.size    = sizeof(int);
  for (i = base; i < end; i+= 100)
   { data.start = i;
     write(5, (BYTE *) &data, sizeof(job_data));
   }
}

static void reader_process(void)
{ reply_data data;
  int i, best, count = -1;
  
  for (i = base; i < end; i+= 100)
   { read(4, (BYTE *) &data, sizeof(reply_data));
     if (data.count > count)
      { best = data.best; count = data.count; }
   }
   
  printf("The winner, with a score of %d, is %d.\n", count, best);
  
  Signal(&finished);
}

