/*------------------------------------------------------------*/
/*                                          tableGenerator.c  */
/*------------------------------------------------------------*/

/* This file contains code to generate all the automatic      */
/*   look-up tables of the draw package.                      */
/* This currently includes a sine/cosine look-up table, and a */
/*   table used to look-up various radius threshold values    */
/*   used to determine the amount of straight lines are       */
/*   needed to flatten a curve.                               */

/*------------------------------------------------------------*/
/*                                             Include files  */
/*------------------------------------------------------------*/

#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <math.h>
#include "private.h"

#define Pi 3.14159265358979323846264383

/*------------------------------------------------------------*/
/*                                   Look-ahead declarations  */
/*------------------------------------------------------------*/

void doSineTable(void);
void doArcThresholds(void);

/*------------------------------------------------------------*/
/*                                         The main function  */
/*------------------------------------------------------------*/

int main()
{  printf("#include \"private.h\"\n");
   doSineTable();
   doArcThresholds();
   return 0;
}

/*------------------------------------------------------------*/
/*                                            The sine-table  */
/*------------------------------------------------------------*/

void doSineTable()
{  int i,hi,lo;
   double a,sv;
   
   printf("SineValue_t sineTable[90*Angles+1] = \n{  \n");
   for(i=0;i<=90*Angles;i++)
   {  a = ((double)i/(90*Angles))*Pi/2.0;
      sv = sin(a);
      if(sv>=1.0) sv=1.0;
      if(sv< 0.0) sv=0.0;
      sv *= 65536.0;
      hi = (int)sv;
      if(hi>=65536) hi=65535;
      sv -= hi;
      sv *= 65536.0;
      lo = (int)sv;
      if(lo>=65536) lo=65535;
      printf("   { %8d,%8d } /* %8g */",hi,lo,
               ((double)hi+(double)lo/65536.0)/65536.0);
      if(i<90*Angles) printf(",\n"); else printf("\n");
   }
   printf("};\n");
}

/*------------------------------------------------------------*/
/*                                        The arc-thresholds  */
/*------------------------------------------------------------*/

void doArcThresholds()
/* See the description in 'private.h' to explain this table   */
{  int i;
   double e,th,r;
   
   printf("int arcThresholds[LogAngles+4+1] = \n{  \n");
   for(i=0; i<LogAngles+4; i++)
   {  e  = pow(2.0,(double)(-maxArcError));
      th = ((Pi/2)/(90.0*(double)Angles))*pow(2.0,(double)i);
      r  = e / (1.0-cos(th/2.0));
      printf("   %8d",(int)r);
      printf(",\n");
   }
   printf("   %8d\n};\n",-1);
}

