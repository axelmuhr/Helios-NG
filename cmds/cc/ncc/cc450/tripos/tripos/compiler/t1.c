#include <stdio.h>

main()
{
   static double a[1000], b[1000];
   int i,j;

   a[0] = 0.9;
   b[0] = 0.9;

   for( i = 0; i < 100; i++)
   {
      for( j = 0; j < 999; j++)
      {
         a[j+1] = a[j] + b[j];
         a[j+1] = a[j+1] - b[j];
         b[j+1] = a[j] / b[j];
         b[j+1] = a[j+1] * b[j+1];
      }
   }
   printf("%f\n",a[999]);
}
