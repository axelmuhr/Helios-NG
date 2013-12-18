#include <stdlib.h>
#include <time.h>
#include <stdio.h>
static clock_t calltime, cumcal;

double gettimer()

{
   clock_t time;
   long clock();

   time = clock();
   time = time-cumcal;
   cumcal += calltime;
   return (double)time/(double)CLK_TCK;
}
void initimer()
{
  double time1, time2;

/*	 Calibrate the clock	 */

  calltime = 0;
  cumcal   = 0;
  time1 = gettimer();
  time2 = gettimer();
  calltime = (clock_t)((time2-time1)*CLK_TCK);
  cumcal = time2;

}
   = 0;
  t