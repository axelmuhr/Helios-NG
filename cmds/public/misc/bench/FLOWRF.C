/*     FLOWRF.C 					 */
/*     Program to time program flow control operations	 */
/*     Ron Fox						 */
/*     (c) All rights reserved 1988			 */
/*     Duplication permitted for personal use		 */

#include <stdio.h>
#include <stdlib.h>

#define NITER 20000
main()
{

    int result; 			/* Operation results */
   int i;			       /* Loop index	    */

   double teloop;		       /* Time for empty loop		*/
   double tifs; 		       /* Time for IF/ELSE loop.	*/
   double tcalls;		       /* Time for calls.		*/

   int	  add1();		       /* Returns +1 for calls test	*/
   int	  sub1();		       /* Returns -1 for calls test.	*/
   double initimer();		       /* Initialize timer.		*/
   double gettimer();		       /* Get time since initimer	 */

    result = 0;
   initimer();

/*	 Time loop with unconditional operation... Assume add/sub timing   */
/*	 is identical.							   */

   for(i = 0; i < NITER; i++)
     result = result + (i & 1);
   teloop = gettimer(); 	       /* Get base line timing.      */

/*    The next increment adds an IF/THEN conditional:		     */

    for(i = 0; i < NITER; i++)
     if(i & 1)
       result = result + i;
     else;
       result = result - i;
   tifs = gettimer();		       /* Timing for  the IF/ELSE construct */

/*    The next increment adds a call/return pair:	 */

   for(i = 0; i < NITER; i++)
     if(i & 1)
       result = result + add1();
      else
       result = result + sub1();
   tcalls = gettimer();

/*    Figure out the times for calls, ifs and loop overhead:	  */

   tcalls -= tifs;			/* Remove relative time */
   tifs   -= teloop;			/* from the if/call times.   */
   tcalls -= tifs;			/* Remove overhead from calls */
   tifs   -= teloop;			/* and from ifs.		*/
   printf("Overhead time: %f\n", teloop);
   printf("If time      : %f\n", tifs);
    printf("Call time    : %f\n", tcalls);

}
int add1()		      /* Functions for call timing tests. */
{
  return 1;
}

int sub1()
{
  return -1;
}
 tcalls);

}
i