/*    SAVAGERF.C    */
#include <stdio.h>
/*
** savage.c - Floating point speed and accuracy test. C veraion is derived
** from a BASIC version which appeard in Dr. Dobb's Journal, Sep. 1983,
** pp. 120-122
*/
 
#define  ILOOP 2500
extern double tan(), atan(), exp(),log(),sqrt();
extern double gettimer();

int main()
{
  int i;
  double a;
  double tover, tmain;

  initimer();
 
/*			Section 1: Time without the transcendentals	*/

  a = 1.0;
  for(i = 1; i<= (ILOOP-1); i++)
    a = a + 1.0;
  tover = gettimer();

/*			Section 2: Time with the transcendentals added */

  a= 1.0;
  for(i = 1; i<= (ILOOP-1); i++)
     a = tan(atan(exp(log(sqrt(a*a))))) + 1.0;
  tmain = gettimer();

  printf("a= %20.14e\n Time required: %f\n", a, (tmain - 2.0*tover));

}
log(sqrt(a*a))))) + 1.0;
  tmain = gettimer();

  printf("a= %20.14e\n Time required: %f\n", a