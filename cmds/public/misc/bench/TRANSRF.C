/*		TRANSRF.C						*/
/*		Time transcendental functions and check for accuracy	*/
/*		via inverses.						*/
/*		Ron Fox 						*/
/*		(c) All rights reserved 1988				*/
/*		Duplication permitted for personal use			*/

#include <stdio.h>
 #include <math.h>

#define NITER 5000
main()
{
  double initimer();			     /* Initialize timer  */
  double gettimer();			     /* Read out timer	  */

  double exact; 			     /* Exact total.	     */
  double stot;				     /* Total from sin loop  */
  double ltot;				     /* Total from log loop */
  double sqrtot;			     /* Total from square root loop */
 
  double toverhead;			     /* Over head time	  */
  double tsine; 			     /* arcsin/sine time  */
  double tlogs; 			     /* log/exp time	  */
  double tsqrt; 			     /* Square/square root time */

  int	 i;				     /* Loop index.	  */


/*   In order to use incremental timing without fear of optimization  */
/*   We use the savage like summing methods			      */
/*   Get the overhead time:					      */
   initimer();
  exact = 1.0;
  for(i = 0; i < NITER; i++)
    exact = exact + 1.0;		  /* This one should be precise */
  toverhead = gettimer();		  /* Measure overhead time	*/

/*  Time sin/arcsin:	*/

  stot = 1.0;
  for(i = 0; i < NITER; i++)
    stot = asin(sin(stot)) + 1.0;
  tsine = gettimer();			  /* Additional time for SIN loop */
 
/*  Time log/exp:     */

  ltot = 1.0;
  for(i = 0; i < NITER; i++)
    ltot = exp(log(ltot)) + 1.0;
  tlogs = gettimer();			  /* Additional time for log loop */

/*  Time sqrt	    */

  stot = 1.0;
  for(i = 0; i < NITER; i++)
     stot = sqrt(stot*stot) + 1.0;
  tsqrt = gettimer();			/* Additional time for sqrt loop */

/*  Get overhead corrected times for each loop: */

  tsqrt -= (tlogs  + toverhead);
  tlogs  -= (tsine + toverhead);
  tsine -= (2.0 * toverhead);

/*  Print results:    */

  printf("Sine time: %f  Error : %f\n", tsine, fabs(stot - exact));
   printf("Log time : %f  Error : %f\n", tlogs,  fabs(ltot - exact));
  printf("Sqrt time: %f  Error : %f\n", tsqrt, fabs(stot - exact));
}
ime : %f  Error : %f\n", tlogs,  fabs(ltot - exact)