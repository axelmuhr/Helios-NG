/*-----------------------------------------------------------------*/
/*                                                         data.c  */
/*-----------------------------------------------------------------*/

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#define Pi 3.141592653589793238462643383271904

int max[32768];
double eps[32768];

int main()
{  double e,r,n,l;
   int i,mn,j;

   printf("Maximum table sizes for various acceptable errors \n");
   printf("  in the circle drawing routines \n");
   
   for(e=1.0,i=0;e>.001;e/=2.0,i++)
   {  printf("\n\nEpsilon = %g\n",e);
      mn = 0;
      for(r=2.0;r<=32768.0;r*=2)
      {  printf("Radius %8d  : ",(int)r);
         n = (Pi/(acos(1.0-e/r)))+1;
         printf(" Table size = %8d ",(int)n);
         l = 2*r*sin(Pi/n);
         printf(" Segment length = %8.6g\n",l);
         if((int)n>mn) mn=(int)n;
      }
      printf("\nMaximum table size for epsilon = %g is %d\n",e,mn);
      max[i]=mn; eps[i]=e;
   }

   printf("\n\n--------------------------- Summary ... \n\n");
    
   for(j=0;j<i;j++)
      printf("Epsilon = %8.6g ; Maximum table size = %8d\n",eps[j],max[j]);
    
   printf("\n\nEnd of program\n");
   return 0;
}
    
