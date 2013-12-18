/*
 * semi-optimised prime number sieve
 *
 * NC 27/1/89
 */

#ifdef __C40
#pragma few_modules
#pragma no_stack_checks
#pragma little_data
#endif

#define forever		for (;;)

/*
 * Basically this algorithm generates numbers in the sequence
 *
 *  5, 7, 11, 13, 17, 19, 23, 25, 29, 31, 35, 37 ...
 *
 * The only numbers in this sequence which are not primes are
 * multiples of a previously generated primes.  These prime
 * divisiors will always be less than or equal to the square
 * root of then generated number.  (cf 25, 35)  The algorithm
 * maintains a pointer into the list of generated primes
 * indicating the largest prime discovered so far which is less
 * than the square root of the current number.  Each generated
 * number is then checked against the primes in this region to
 * see if it has prime factors.
 */

void
calc_primes(
            int *	max_primes,
            int *	primes )
{
  int *		third_prime	= primes + 2;
  int *		next_free 	= third_prime;
  int *		lim 		= third_prime;
  int		limit 		= 25;
  int		prime 		= 1;
	

  forever
    {
      int *	j;


    restart:
      prime += 4;

      while (prime >= limit)
	{
	  limit  = *++lim;
	  limit *= limit;
	}

      for (j = third_prime; j < lim; j++)
	{
	  if (prime % *j == 0)
	    {
	      goto next;
	    }
	}

      *next_free++  = prime;
      
      if (next_free == max_primes)
	{
	  break;
	}
      
    next:
      prime += 2;

      while (prime >= limit)
	{
	  limit  = *++lim;
	  limit *= limit;
	}

      for (j = third_prime; j < lim; j++)
	{
	  if (prime % *j == 0)
	    {
	      goto restart;
	    }
	}

      *next_free++ = prime;
      
      if (next_free == max_primes)
	{
	  break;
	}
    }

  return;
  
} /* calc_primes */
