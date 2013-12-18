/*   MOVESRF.C						    */
/*   Timing test for data movement operations		    */
/*	 Ron Fox					    */
/*	 (c) All rights reserved 1988			    */
/*	 Duplication permitted for personal use only	    */

#include <stdio.h>
#include <stdlib.h>

#define ASIZE 5000
 main()
{

   int dest[ASIZE];		       /* Destination array */
   int src[ASIZE];		       /* Source array	    */
   int i;			       /* Loop index	    */
   register int reg;		       /* Register source variable */
   int optkiller;		       /* Used to kill loop optimization */
   double t1loop;		       /* Time for loop with 1 randomizer */
   double t2loop;		       /* Time for loop with 2 randomizers */
   double teloop;		       /* Time for empty loop		*/
   double tcloop;		       /* Time for const to memory loop  */
    double trloop;			/* Time for register to memory loop */
   double tmloop;		       /* Time for memory to memory loop   */

   int rand();			       /* Gives random from 0-RANGE	 */
   void   initimer();		       /* Initialize timer		 */
   double gettimer();		       /* Get time since initimer	 */

   reg = rand();
   initimer();

/*    Empty loop timing... We actually do a random number sum to defeat    */
/*    most optimizers							   */
 /*    We use incremental timing to get the empty loop timing from this  */

   optkiller = 0;
   for(i = 0; i < ASIZE; i++)
     optkiller += rand();			/* First time loop with 1 */
   t1loop = gettimer(); 			/* randomizer sum	*/

   for(i = 0; i < ASIZE; i++)
   {
     optkiller += rand();
     optkiller += rand();		/* Then with 2. 		*/
   }
    t2loop = gettimer();		 /* difference is empty loop time */


/*	    Clear the dest array for constant to memory loop.	  */

   optkiller = 0;
   for( i= 0; i < ASIZE; i++)
      dest[i] = 0;
   tcloop = gettimer(); 		/* Cumulative CPU time. */

/*    Move random number in reg from register variable to memory.     */

    optkiller = 0;
   for(i=0; i < ASIZE; i++)
      src[i] = reg;
   trloop = gettimer(); 		/* Cumulative CPU time. */

/*	 Memory to memory move operations from src to dest  */

   optkiller = 0;
   for(i = 0; i < ASIZE; i++)
      dest[i] = src[i];
   tmloop = gettimer(); 		/* Cumulative CPU time. */

 /* Correct the times for the loop overheads and print out timings:   */

   teloop =t1loop - (t2loop - 2.0*t1loop); /* This is empty loop time	   */
   tmloop -= (trloop + teloop); 	/* Time for only the mem to mem */
   trloop -= (tcloop + teloop); 	/* Time for only reg to mem	*/
   tcloop -= (t2loop + teloop); 	/* Time for only the const to mem */
   printf("Overhead time     : %f\n", teloop);
   printf("Constant to memory: %f\n", tcloop);
   printf("Register to memory: %f\n", trloop);
   printf("Memory to memory  : %f\n", tmloop);
}
to memory: %f\n", tcloop);
   printf("Register to memory: %f\n", trloop);
   printf("Memory to memory  : %f\n", tmloo