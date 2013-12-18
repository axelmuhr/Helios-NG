/*	     PARALLEL.C 				       */
/*	     Program to time basic floating point operations   */
/*	     Ron Fox					       */
/*	     (c) 1988 All rights reserved		       */
/*	     Duplication permitted for personal use.	       */
 #include <stdio.h>
#include <stdlib.h>

#define ASIZE 5000
#define RANGE 32767
main()
{

   float dest[ASIZE];			 /* Destination array */
   float src[ASIZE];			 /* Source array      */
   float result;			 /* Operation results */
   int i;				 /* Loop index	      */
 
   double teloop;		       /* Time for empty loop		*/
   double tadds;		       /* Addition times		 */
   double tsubs;		       /* Subtraction times.		 */
   double tmuls;		       /* Multiplication times		 */
   double tdivs;		       /* Division times		 */
   double tands;		       /* AND times			 */
   double tors; 		       /* OR times			 */
   void   initimer();		       /* Initialize timer		 */
   double gettimer();		       /* Get time since initimer	 */

/*	    Before turning on the timer we initialize the src to non	*/
 /*	     zeros in order to prevent divide by zeros: 		 */

   for(i = 0; i < ASIZE; i++)
      src[i] =(float)(i+1);

   initimer();

/*	 The loop below gets memory, source, destination and loop overhead */
/*	 timing:							   */

   for( i = 0; i < ASIZE; i++)
      dest[i] = src[i]; 	       /* Moves are the timing basis	*/
    teloop = gettimer();

/*	    Additions				   */

   for(i = 0; i < ASIZE; i++)
   {
      dest[i] = src[i]; 	       /* The simplest of peephole	*/
      result  += src[i];	       /* optimizers should prevent a	*/
   }				       /* double fetch of src[i]	*/
   tadds = gettimer();

/*	 Subtractions				   */
 
   for(i = 0; i < ASIZE; i++)
   {
      dest[i] = src[i];
      result -= src[i];
   }
   tsubs = gettimer();

/*	 Multiplications		     */

   for(i = 0; i < ASIZE; i++)
   {
      dest[i] = src[i];
     result *= src[i];
   }
   tmuls = gettimer();

/*	 Divisions are a bit trickier... once the dividend has gone to zero */
/*	 a division algorithm could pull up short.  So our result will be   */
/*	 dest/src, which will introduce some timing perturbation	    */

   for(i = 0; i < ASIZE; i++)
   {
     dest[i] = src[i];
      result  = dest[i]/src[i];
   }
   tdivs = gettimer();

/*	 Now produce overhead corrected timings       */

   tdivs -= (tmuls + teloop);
   tmuls -= (tsubs + teloop);
   tsubs -= (tadds + teloop);
   tadds -= (teloop+ teloop);

   printf("Overhead time   : %f\n",teloop);
    printf("Add timing      : %f\n",tadds);
   printf("Sub timing      : %f\n",tsubs);
   printf("Mul timing      : %f\n",tmuls);
   printf("Div timing      : %f\n",tdivs);

}
"Sub timing      : %f\n",tsubs);
   printf("Mul timing      : %f\n",tm