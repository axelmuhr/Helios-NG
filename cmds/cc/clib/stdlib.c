
/* stdlib.c: ANSI draft (X3J11 Oct 86) library, section 4.10 */
/* Copyright (C) A.C. Norman and A. Mycroft */
/* version 0.02 */
/* $Id: stdlib.c,v 1.3 1992/08/17 09:53:20 nickc Exp $ */

#include <stddef.h>
#include <stdlib.h>
#include <signal.h>
#include "norcrosys.h"      /* for _terminateio(), and _exit()   */

/* atof, atoi, atol, strtod, strtol, strtoul are implemented in scanf.c  */
/* mblen, mbtowc, wctomb, mbstowcs, wcstombs are implemented in locale.c */

static unsigned long int next = 1;

int
rand( void )
{
  /* I do not like this random number generator very much, but it is given */
  /* in the ANSI document and for portability and conformance this is what */
  /* must be provided.                                                     */

  next = next * 1103515245 + 12345;

  return (unsigned int) ((next >> 16) & RAND_MAX);
}

void
srand( unsigned int seed )
{
  next = seed;
}


/* Now the random-number generator that the world is expected to use */

static unsigned _random_number_seed[ 55 ] =
/* The values here are just those that would be put in this horrid
   array by a call to __srand(1). DO NOT CHANGE __srand() without
   making a corresponding change to these initial values.
*/
  {
    0x00000001, 0x66d78e85, 0xd5d38c09, 0x0a09d8f5, 0xbf1f87fb,
    0xcb8df767, 0xbdf70769, 0x503d1234, 0x7f4f84c8, 0x61de02a3,
    0xa7408dae, 0x7a24bde8, 0x5115a2ea, 0xbbe62e57, 0xf6d57fff,
    0x632a837a, 0x13861d77, 0xe19f2e7c, 0x695f5705, 0x87936b2e,
    0x50a19a6e, 0x728b0e94, 0xc5cc55ae, 0xb10a8ab1, 0x856f72d7,
    0xd0225c17, 0x51c4fda3, 0x89ed9861, 0xf1db829f, 0xbcfbc59d,
    0x83eec189, 0x6359b159, 0xcc505c30, 0x9cbc5ac9, 0x2fe230f9,
    0x39f65e42, 0x75157bd2, 0x40c158fb, 0x27eb9a3e, 0xc582a2d9,
    0x0569d6c2, 0xed8e30b3, 0x1083ddd2, 0x1f1da441, 0x5660e215,
    0x04f32fc5, 0xe18eef99, 0x4a593208, 0x5b7bed4c, 0x8102fc40,
    0x515341d9, 0xacff3dfa, 0x6d096cb5, 0x2bb3cc1d, 0x253d15ff
};

static int _random_j = 23, _random_k = 54;

int
__rand( void )
{
  unsigned int temp;
  
  /*
   * See Knuth vol 2 section 3.2.2 for a discussion of this random
   *  number generator.
   */
  
  temp = (_random_number_seed[ _random_k ] =+ _random_number_seed[ _random_j ]);

  if (--_random_j == 0)
    _random_j = 54,
    --_random_k;
  else
    if (--_random_k == 0)
      _random_k = 54;

  return (temp & __RAND_MAX);         /* result is a 31-bit value */

  /* It seems that it would not be possible, under ANSI rules, to */
  /* implement this as a 32-bit value. What a shame!              */

} /* __rand */


void
__srand( unsigned int seed )
{
  int i;

  /*
   * This only allows you to put 32 bits of seed into the random sequence,
   * but it is very improbable that you have any good source of randomness
   * that good to start with anyway! A linear congruential generator
   * started from the seed is used to expand from 32 to 32*55 bits.
   */

  _random_j = 23;
  _random_k = 54;
  
  for (i = 0; i<55; i++)
    {
      _random_number_seed[ i ] = seed + (seed >> 16);
      
      /* This is not even a good way of setting the initial values.  For instance */
      /* a better scheme would have r<n+1> = 7^4*r<n> mod (3^31-1).  Still I will */
      /* leave this for now.                                                      */
      
      seed = 69069 * seed + 1725307361;  /* computed modulo 2^32 */
    }

  return;
  
} /* __srand */

/* free, malloc, realloc etc are in the file alloc.c                     */

#ifndef POSIX
#define EXIT_LIMIT 33

static void (*_exitvector[EXIT_LIMIT])(void);
static int number_of_exit_functions;

void _exit_init()
{
  number_of_exit_functions = 0;
}

int atexit(void (*func)(void))
{ 
    if (number_of_exit_functions >= EXIT_LIMIT) return 1;    /* failure */
    _exitvector[number_of_exit_functions] = func;
    number_of_exit_functions++;    /* BLV - compiler bug array[element++] */
    return 0;                                                /* success */
}

void exit(int n)
{
    while (number_of_exit_functions!=0)
         (*_exitvector[--number_of_exit_functions])();
     _terminateio();
    _exit(n);
}

void abort()
{   raise(SIGABRT);
    exit(1);
}
#else
int _atexit_stub(void (*func)(void))
{
	return atexit(func);
}

void _exit_stub(int n)
{
	exit(n);
}

void _abort_stub()
{
	abort();
}

#endif

int abs(int x)
{
    if (x<0) return (-x);
    else return x;
}

long int labs(long int x)
{
    if (x<0) return (-x);
    else return x;
}

div_t div(int numer, int denom)
{
/* This is a candidate for re-implementation in machine code so that the */
/* quotient and remainder can be computed all at once. However I am not  */
/* really convinced about the importance of the function so will not do  */
/* that yet!                                                             */
    div_t res;
    res.quot = numer / denom;
    res.rem  = numer % denom;
    return res;
}

ldiv_t ldiv(long int numer, long int denom)
{
    ldiv_t res;
    res.quot = numer / denom;
    res.rem  = numer % denom;
    return res;
}


/* end of stdlib.c */
