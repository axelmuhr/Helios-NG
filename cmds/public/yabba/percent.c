/* Placed into the public domain by Daniel J. Bernstein. */

#include "percent.h"

#define MAXULONG ((unsigned long) (-1))

#define HENCE(x) ;

#define ENOUGH(a) { long p = (a); if (q) { \
if (q - 1 > limit / 100) return limit; \
if ((q - 1) * 100 > limit - 100) return limit; \
if (q * 100 > limit - p) return limit; \
return (long) (p + q*100); } else if (p > limit) return limit; else return p; }

long percent(a,b,limit)
unsigned long a;
unsigned long b;
long limit;
{
 unsigned long q;
 unsigned long s;
 unsigned long t;

 q = 0;

 if (b == 0)
   ENOUGH(100)
 HENCE(b > 0);

 if (b < MAXULONG - 200)
   if (a < (MAXULONG / 200) - (b / 200) - 1) /* cannot go below 0 */
     ENOUGH((100 * a + (b / 2)) / b) /* cannot overflow */
 HENCE(a + (b / 200) + 1 >= MAXULONG / 200);
 /* How often will a and b be more than 21 million on a 32-bit machine? */

 if (a < b / 200)
   ENOUGH(0)
 HENCE(a >= b / 200);
 HENCE(a + a + 1 >= MAXULONG / 200);

 if (a >= b)
  {
   q = a / b;
   HENCE(a - q * b < b);
   a -= q * b;
  }
 HENCE(a < b);

 t = 0; s = a;
 /* now s is a mod b, t is floor(a/b) */
 if (s >= b - a) { t += 1; s -= b - a; } else { s += a; }
 /* now s is 2a mod b, t is floor(2a/b) */
 if (s >= b - a) { t += 1; s -= b - a; } else { s += a; }
 /* now s is 3a mod b, t is floor(3a/b) */
 if (s >= b - s) { t += t + 1; s -= b - s; } else { t += t; s += s; }
 /* now s is 6a mod b, t is floor(6a/b) */
 if (s >= b - s) { t += t + 1; s -= b - s; } else { t += t; s += s; }
 /* now s is 12a mod b, t is floor(12a/b) */
 if (s >= b - s) { t += t + 1; s -= b - s; } else { t += t; s += s; }
 /* now s is 24a mod b, t is floor(24a/b) */
 if (s >= b - a) { t += 1; s -= b - a; } else { s += a; }
 /* now s is 25a mod b, t is floor(25a/b) */
 if (s >= b - s) { t += t + 1; s -= b - s; } else { t += t; s += s; }
 /* now s is 50a mod b, t is floor(50a/b) */
 if (s >= b - s) { t += t + 1; s -= b - s; } else { t += t; s += s; }
 /* now s is 100a mod b, t is floor(100a/b) */
 if (s >= b - (b/2)) { t += 1; s -= b - (b/2); } else { s += (b/2); }

 ENOUGH(t)
 /* whew. */
}
