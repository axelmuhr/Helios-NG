#include <stdio.h>
#include <time.h>

main()
{
   static double a[1000], b[1000];
   int i,j;
   clock_t t1,t2;

   a[0] = 0.9;
   b[0] = 0.9;

   t1 = clock();

   for( i = 0; i < 10; i++)
   {
      for( j = 0; j < 999; j++)
      {
         b[j+1] = b[j] + a[j];
         b[j+1] = b[j+1] - a[j];
         a[j+1] = b[j] / a[j];
         a[j+1] = b[j+1] * a[j+1];
      }
   }
   t2 = clock();

   printf("Answer is %f, %d loops in %d secs\n",a[999], 10, (int)(t2-t1)/100);
}
