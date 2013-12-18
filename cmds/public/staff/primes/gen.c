/*
 * semi-optimised prime number sieve
 *
 * NC 27/1/89
 */

#include <stdio.h>
#include <stdlib.h>

#ifdef TIME
#include <time.h>
#include <sys/types.h>
#include <sys/times.h>
#endif

#if FAST > 0
#include <nonansi.h>
#include <memory.h>
#include <task.h>
#endif

#ifdef __C40
#pragma few_modules
#pragma no_stack_checks
#pragma little_data
#endif

#define MAX_PRIMES	10

extern void	calc_primes( int * max_primes, int * primes );

int
main(
     int	argc,
     char **	argv )
{
  int *		primes;
  int		max_primes;
  int 		stub;
  char *        root;
#ifdef PRINT
  int		i;
#endif
#ifdef FAST
#if (FAST & 1)
  word		res;
#endif
#if (FAST & 2)
  Carrier *	fast_stack;
#endif
#endif
#ifdef TIME
  struct tms	buffer;
  time_t	start;
#endif

  
  if (argc < 2)
    {
      max_primes = MAX_PRIMES;
    }
  else
    {
      max_primes = atoi( argv[ 1 ] );
    }
	
  if (max_primes < 3)
    {
      max_primes = 3;
    }
	  
  if ((primes = (int *)malloc( max_primes * sizeof( int ) )) == NULL)
    {
      fprintf( stderr, "not enough memory for %d primes\n", max_primes );
      
      return 1;	
    }
  
  primes[ 0 ] = 2;
  primes[ 1 ] = 3;

#ifdef TIME
  times( &buffer );
  
  start = buffer.tms_utime;
#endif

#if (FAST & 1)
  if ((res = AccelerateCode( calc_primes )) != 0)
    {
      fprintf( stderr, "Failed to run AccelerateCode, return value = %lx\n", res );
    }
#endif  

#if (FAST & 2)
  fast_stack = AllocFast( 1000, &MyTask->MemPool );
  
  if (fast_stack == NULL)
    {
      fprintf( stderr, "could not allocate FAST stack\n" );
      
      calc_primes( primes + max_primes, primes );
    }
  else
   {
      Accelerate( fast_stack, calc_primes, sizeof (primes) * 2,
	primes + max_primes, primes );

      FreeMem( fast_stack );
   }  
#else
  calc_primes( primes + max_primes, primes );
#endif

#ifdef TIME	
  times( &buffer );

  start = buffer.tms_utime - start;
	
  printf( "elapsed time = %f seconds\n", 
    (double)start / 100.0 );
#endif

#ifdef PRINT		
  printf( "The first %d primes are :-\n", max_primes );
	
  for (i = 0; i < max_primes; i++)
    {
      stub = i % 10;
      root = (stub == 1 ? "st" : stub == 2 ? "nd" : "th");
	
      printf( "%4d%s prime is %5d\n", i, root, primes[ i ] );
    }
#else
  stub = max_primes % 10;
  root = (stub == 1 ? "st" : stub == 2 ? "nd" : "th");
  
  printf( "%d%s prim is %d\n", max_primes, root, primes[ max_primes - 1 ] );
#endif /* PRINT */

  free( primes );

  return 0;
	
} /* main */

