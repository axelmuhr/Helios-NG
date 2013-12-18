/* Placed into the public domain by Daniel J. Bernstein. */

/* This is one of those functions that everyone needs occasionally */
/* but nobody wants to write: computing what percentage x/y is, where */
/* x and y are unsigned longs. Sure, you can do it in floating point, */
/* but would you bet your sister on the accuracy of the result? Sure, */
/* you can just compute ((100 * x) + (y/2)) / y, but what are you */
/* going to do about overflow? percent() is the answer. Unless I've */
/* flubbed something, percent() will never overflow, will always */
/* return a properly rounded value, and will never take too long. */

#ifndef PERCENT_H
#define PERCENT_H

long percent();

/* long percent(a,b,limit) unsigned long a; unsigned long b; long limit; */
/* returns the correctly rounded value of 100a/b */
/* returns 100 if b is 0 */
/* returns limit if the result is limit or larger */
/* limit must be at least 100 */
/* note that halves are rounded up */

#endif
