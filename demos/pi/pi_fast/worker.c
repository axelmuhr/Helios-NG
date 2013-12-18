#include <helios.h>
#include <stdio.h>
#include <posix.h>
#include <nonansi.h>
#include <memory.h>
#include <task.h>
#include <stdlib.h>

extern void eval( int position, int number_workers,
                   int intervals, double * result );


#define	 TO_RING	1	/* file handle for sending messages into pi ring */
#define	 FROM_RING	0	/* file handle for receiving messages from pi ring */


void
safe_read(
	  char *	buffer,		/* buffer into which to place read data */
	  unsigned int	size )		/* the size of 'buffer' in bytes */
{
  int	got	= 0;


  do
    {
      int	res;
      
      
      res = read( FROM_RING, buffer + got, size - got );

      if (res < 0)
	{
	  fprintf( stderr, "Pi worker: read(), failed, errno = %d\n", errno );

	  exit( EXIT_FAILURE );
	}

      if (res == 0)
	{
	  fprintf( stderr, "Pi worker: warning read() returned 0\n" );
	}

      got += res;
    }
  while (got < size);

  return;
  
} /* safe_read */


void
safe_write(
	   char *	buffer,		/* buffer from which data should be taken */
	   unsigned int	size )		/* the size of 'buffer' in bytes */
{
  int	sent = 0;


  do
    {
      int	res;
      
      
      res = write( TO_RING, buffer + sent, size - sent );

      if (res < 0)
	{
	  fprintf( stderr, "Pi worker: write(), failed, errno = %d\n", errno );

	  exit( EXIT_FAILURE );
	}

      if (res == 0)
	{
	  fprintf( stderr, "Pi worker: warning write() returned 0\n" );
	}

      sent += res;
    }
  while (sent < size);

  return;
  
} /* safe_write */


int
main( void )
{
  int 		position;
  int		number_workers;
  int		temp;
  int		intervals;
  double	sum;
  double	total;
  Carrier *	fast_stack;

  
  /* get the worker's position in the pipeline */
  
  safe_read(  (char *) &position, sizeof (position) );
  
  temp = position + 1;
  
  safe_write( (char *) &temp, sizeof (temp) );


  /* get the length of the pipeline */
  
  safe_read(  (char *) &number_workers, sizeof (number_workers) );
  safe_write( (char *) &number_workers, sizeof (number_workers) );
  

  /* get the number of intervals per worker */
  
  safe_read(  (char *) &intervals, sizeof (intervals) );
  safe_write( (char *) &intervals, sizeof (intervals) );


  /* put the code for eval() into Fast RAM */
  
  AccelerateCode( eval );


  /* get some more Fast RAM to use as an execution stack */
  
  fast_stack = AllocFast( 500, &MyTask->MemPool );

  
  /* run the evaluate function */
  
  if (fast_stack == Null(Carrier))
    {
      eval( position, number_workers, intervals, &sum );
    }
  else
    {
      Accelerate( fast_stack, eval,
		 sizeof (position)  + sizeof (number_workers) +
		 sizeof (intervals) + sizeof (&sum),
		 position, number_workers, intervals, &sum );

      FreeMem( fast_stack );
    }

  
  /* get the running total */
  
  safe_read( (char *) &total, sizeof (total) );

  
  /* add in our contribution and send on */
  
  total = total + sum;
  
  safe_write( (char *) &total, sizeof (total) );


  /* finished */
  
  return EXIT_SUCCESS;

} /* main */

