/* More subroutines needed by GCC output code on some machines.  */
/* Compile this one with gcc.  */

#include "config.h"
#include <stddef.h>

/* long long ints are pairs of long ints in the order determined by
   WORDS_BIG_ENDIAN.  */

#ifdef WORDS_BIG_ENDIAN
  struct longlong {long high, low;};
#else
  struct longlong {long low, high;};
#endif

/* Internally, long long ints are strings of unsigned shorts in the
   order determined by BYTES_BIG_ENDIAN.  */

#define B 0x10000
#define low16 (B - 1)

#ifdef BYTES_BIG_ENDIAN

#define HIGH 0
#define LOW 1

#define big_end(n)	0 
#define little_end(n)	((n) - 1)
#define next_msd(i)	((i) - 1)
#define next_lsd(i)	((i) + 1)
#define is_not_msd(i,n)	((i) >= 0)
#define is_not_lsd(i,n)	((i) < (n))

#else

#define LOW 0
#define HIGH 1

#define big_end(n)	((n) - 1)
#define little_end(n)	0 
#define next_msd(i)	((i) + 1)
#define next_lsd(i)	((i) - 1)
#define is_not_msd(i,n)	((i) < (n))
#define is_not_lsd(i,n)	((i) >= 0)

#endif

/* These algorithms are all straight out of Knuth, vol. 2, sec. 4.3.1. */

#define bdiv __div_internal

static int badd ();
static int bsub ();
static void bmul ();
static void bdiv ();
static int bneg ();
static int bshift ();

#ifdef L_adddi3
struct longlong 
__adddi3 (u, v)
     struct longlong u, v;
{
  long a[2], b[2], c[2];
  struct longlong w;

  a[HIGH] = u.high;
  a[LOW] = u.low;
  b[HIGH] = v.high;
  b[LOW] = v.low;

  badd (a, b, c, sizeof c);

  w.high = c[HIGH];
  w.low = c[LOW];
  return w;
}

static int 
badd (a, b, c, n)
     unsigned short *a, *b, *c;
     size_t n;
{
  unsigned long acc;
  int i;

  n /= sizeof *c;

  acc = 0;
  for (i = little_end (n); is_not_msd (i, n); i = next_msd (i))
    {
      acc += a[i] + b[i];
      c[i] = acc & low16;
      acc = acc >> 16;
    }
  return acc;
}
#endif

#ifdef L_subdi3
struct longlong 
__subdi3 (u, v)
     struct longlong u, v;
{
  long a[2], b[2], c[2];
  struct longlong w;

  a[HIGH] = u.high;
  a[LOW] = u.low;
  b[HIGH] = v.high;
  b[LOW] = v.low;

  bsub (a, b, c, sizeof c);

  w.high = c[HIGH];
  w.low = c[LOW];
  return w;
}

static int 
bsub (a, b, c, n)
     unsigned short *a, *b, *c;
     size_t n;
{
  signed long acc;
  int i;

  n /= sizeof *c;

  acc = 0;
  for (i = little_end (n); is_not_msd (i, n); i = next_msd (i))
    {
      acc += a[i] - b[i];
      c[i] = acc & low16;
      acc = acc >> 16;
    }
  return acc;
}
#endif

#ifdef L_muldi3
struct longlong 
__muldi3 (u, v)
     struct longlong u, v;
{
  long a[2], b[2], c[2][2];
  struct longlong w;

  a[HIGH] = u.high;
  a[LOW] = u.low;
  b[HIGH] = v.high;
  b[LOW] = v.low;

  bmul (a, b, c, sizeof a, sizeof b);

  w.high = c[LOW][HIGH];
  w.low = c[LOW][LOW];
  return w;
}

static void 
bmul (a, b, c, m, n)
    unsigned short *a, *b, *c;
    size_t m, n;
{
  int i, j;
  unsigned long acc;

  bzero (c, m + n);

  m /= sizeof *a;
  n /= sizeof *b;

  for (j = little_end (n); is_not_msd (j, n); j = next_msd (j))
    {
      acc = 0;
      for (i = little_end (m); is_not_msd (i, m); i = next_msd (i))
	{
	  acc += a[i] * b[j] + (c + next_lsd (j))[i];
	  (c + next_lsd (j))[i] = acc & low16;
	  acc = acc >> 16;
	}
      c[j] = acc;
    }
}
#endif

#ifdef L_divdi3
long long
__divdi3 (u, v)
     long long u, v;
{
  if (u < 0)
    if (v < 0)
      return (unsigned long long) -u / (unsigned long long) -v;
    else
      return - ((unsigned long long) -u / (unsigned long long) v);
  else
    if (v < 0)
      return - ((unsigned long long) u / (unsigned long long) -v);
    else
      return (unsigned long long) u / (unsigned long long) v;
}
#endif

#ifdef L_moddi3
long long
__moddi3 (u, v)
     long long u, v;
{
  if (u < 0)
    if (v < 0)
      return - ((unsigned long long) -u % (unsigned long long) -v);
    else
      return - ((unsigned long long) -u % (unsigned long long) v);
  else
    if (v < 0)
      return (unsigned long long) u % (unsigned long long) -v;
    else
      return (unsigned long long) u % (unsigned long long) v;
}
#endif

#ifdef L_udivdi3
struct longlong 
__udivdi3 (u, v)
     struct longlong u, v;
{
  unsigned long a[2][2], b[2], q[2], r[2];
  struct longlong w;

  a[HIGH][HIGH] = 0;
  a[HIGH][LOW] = 0;
  a[LOW][HIGH] = u.high;
  a[LOW][LOW] = u.low;
  b[HIGH] = v.high;
  b[LOW] = v.low;

  bdiv (a, b, q, r, sizeof a, sizeof b);

  w.high = q[HIGH];
  w.low = q[LOW];
  return w;
}
#endif

#ifdef L_umoddi3
struct longlong 
__umoddi3 (u, v)
     struct longlong u, v;
{
  unsigned long a[2][2], b[2], q[2], r[2];
  struct longlong w;

  a[HIGH][HIGH] = 0;
  a[HIGH][LOW] = 0;
  a[LOW][HIGH] = u.high;
  a[LOW][LOW] = u.low;
  b[HIGH] = v.high;
  b[LOW] = v.low;

  bdiv (a, b, q, r, sizeof a, sizeof b);

  w.high = r[HIGH];
  w.low = r[LOW];
  return w;
}
#endif

#ifdef L_negdi2
struct longlong 
__negdi2 (u)
     struct longlong u;
{
  unsigned long a[2], b[2];
  struct longlong w;

  a[HIGH] = u.high;
  a[LOW] = u.low;

  bneg (a, b, sizeof b);

  w.high = b[HIGH];
  w.low = b[LOW];
  return w;
}

static int
bneg (a, b, n)
     unsigned short *a, *b;
     size_t n;
{
  signed long acc;
  int i;

  n /= sizeof (short);

  acc = 0;
  for (i = little_end (n); is_not_msd (i, n); i = next_msd (i))
    {
      acc -= a[i];
      b[i] = acc & low16;
      acc = acc >> 16;
    }
  return acc;
}
#endif

/* Divide a by b, producing quotient q and remainder r.

       sizeof a is m
       sizeof b is n
       sizeof q is m - n
       sizeof r is n

   The quotient must fit in m - n bytes, i.e., the most significant
   n digits of a must be less than b, and m must be greater than n.  */

#ifdef L_div_internal
void 
bdiv (a, b, q, r, m, n)
     unsigned short *a, *b, *q, *r;
     size_t m, n;
{
  unsigned long qhat, rhat;
  unsigned long acc;
  unsigned short *u = (unsigned short *) alloca (m);
  unsigned short *v = (unsigned short *) alloca (n);
  unsigned short *u1 = next_lsd (u);
  unsigned short *u2 = next_lsd (u1);
  unsigned short *vn;
  int d, qn;
  int i, j;

  m /= sizeof *a;
  n /= sizeof *b;
  qn = m - n;

  /* Shift divisor and dividend left until the high bit of the divisor
     is 1.  */

  while (b[big_end (n)] == 0)
    {
      r[big_end (n)] = 0;

      a += little_end (2);
      b += little_end (2);
      r += little_end (2);
      m--;
      n--;

      /* Check for zero divisor.  */
      if (n == 0)
	abort ();
    }
      
  for (d = 0; d < 16; d++)
    if (b[big_end (n)] & (1 << (16 - 1 - d)))
      break;

  bshift (a, d, u, 0, m);
  bshift (b, d, v, 0, n);

  /* Get a pointer to the high divisor digit (so some of the upcoming code
     fits on a line).  Provide the divisor with a trailing zero digit if
     it is only one digit long.  */

  vn = v + big_end (n);
  if (n == 1)
    *next_lsd (vn) = 0;

  /* Main loop: find a quotient digit, multiply it by the divisor,
     and subtract that from the dividend, shifted over the right amount. */

  for (j = big_end (qn); is_not_lsd (j, qn); j = next_lsd (j))
    {
      /* Quotient digit initial guess: high 2 dividend digits over high
	 divisor digit.  */

      if (u[j] == *vn)
	{
	  qhat = B - 1;
	  rhat = *vn + u1[j];
	}
      else
	{
	  unsigned long numerator = u[j] << 16 |  u1[j];
	  qhat = numerator / *vn;
	  rhat = numerator % *vn;
	}

      /* Now get the quotient right for high 3 dividend digits over
	 high 2 divisor digits.  */

      while (rhat < B && qhat * *next_lsd (vn) > (rhat << 16 | u2[j]))
	{
	  qhat -= 1;
	  rhat += *vn;
	}
	    
      /* Multiply quotient by divisor, subtract from dividend.  */

      acc = 0;
      for (i = little_end (n); is_not_msd (i, n); i = next_msd (i))
	{
	  acc += (u1 + j)[i] - v[i] * qhat;
	  (u1 + j)[i] = acc & low16;
	  if (acc < B)
	    acc = 0;
	  else acc = acc >> 16 | -B;
	}

      q[j] = qhat;

      /* Quotient may have been too high by 1.  If dividend went negative,
	 decrement the quotient by 1 and add the divisor back.  */

      if ((signed long) (acc + u[j]) < 0)
	{
	  q[j] -= 1;
	  acc = 0;
	  for (i = little_end (n); is_not_msd (i, n); i = next_msd (i))
	    {
	      acc += (u1 + j)[i] + v[i];
	      (u1 + j)[i] = acc & low16;
	      acc = acc >> 16;
	    }
	}
    }

  /* Now the remainder is what's left of the dividend, shifted right
     by the amount of the normalizing left shift at the top.  */

  if (d == 0)
    bshift (&u[j], d, r, 0, n);
  else
    r[big_end (n)] = bshift (&u[j], 16 - d, &r[next_lsd (big_end (n))],
			     u[little_end (m)] >> d, n - 1);
}

/* Left shift U by K giving W; fill the introduced low-order bits with
   CARRY_IN.  Length of U and W is N.  Return carry out.  K must be
   in 0 .. 16.  */

static int
bshift (u, k, w, carry_in, n)
     unsigned short *u, *w, carry_in;
     int k, n;
{
  unsigned long acc;
  int i;

  if (k == 0)
    {
      bcopy (u, w, n * sizeof *u);
      return 0;
    }

  acc = carry_in;
  for (i = little_end (n); is_not_msd (i, n); i = next_msd (i))
    {
      acc |= u[i] << k;
      w[i] = acc & low16;
      acc = acc >> 16;
    }
  return acc;
}
#endif

