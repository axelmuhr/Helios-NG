/*-----------------------------------------------------------*/
/*                                     source/test/test2M.c  */
/*-----------------------------------------------------------*/

/* $Header: test2M.c,v 1.1 90/03/03 18:01:24 charles Locked $ */
/* $Source: /server/usr/users/charles/world/drawp/RCS/source/test/test2M.c,v $ */

/* This file is used to test the internal arithmetic functions  */
/*    provided by the draw package                              */

/* Tests the following functions quite thoroughly: */

/* dpNormalizeLength (...)  */
/* dpAddLengths      (...)  */

/*-----------------------------------------------------------*/
/*                                            Include files  */
/*-----------------------------------------------------------*/

#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include "math.h"
#include "drawp/public.h"
#include "drawp/debug.h"

/*-----------------------------------------------------------*/
/*                                            Abbreviations  */
/*-----------------------------------------------------------*/

#define Bf  BinaryFigures
#define DBf ((double)Bf)

/*-----------------------------------------------------------*/
/*                                                   main()  */
/*-----------------------------------------------------------*/

#define testSize 7
int isNormal(int l);
int main()
{  static int testData[testSize] = 
   {  0,1,2,3,1000,30000,32767  } ;
   int l1mh,l1ml,l1e,l1x,l1,l1n; double l1fer,l1er,l1f,l1fn;
   int l2mh,l2ml,l2e,l2x,l2,l2n; double l2fer,l2er,l2f,l2fn;
   int l3; double l3r,l3f;
   double le,lef;
   int err,errs,tests,totalTests;
   
   totalTests = testSize*testSize*testSize*testSize*testSize;
   tests = 0; errs = 0;

   for(l1mh=0;l1mh<testSize;l1mh++)
   for(l1ml=0;l1ml<testSize;l1ml++)
   for(l1e =0;l1e <testSize;l1e ++)
   for(l2mh=0;l2mh<testSize;l2mh++)
   for(l2ml=0;l2ml<testSize;l2ml++)
   for(l2e =0;l2e <testSize;l2e ++)
   {  err = 0;
 
      if(!(tests%100))
         fprintf(stderr,"%6d/%6d tests performed. %6d errors found\n",
                        tests,totalTests,errs);

      l1 = (testData[l1mh]<<Bf) + testData[l1ml];
      if((l1>>(16+Bf))!=0) goto skipTest;
      if(testData[l1e]>31) goto skipTest;
      l1 = (l1<<5) | testData[l1e];
      l2 = (testData[l2mh]<<Bf) + testData[l2ml];
      if((l2>>(16+Bf))!=0) goto skipTest;
      if(testData[l2e]>31) goto skipTest;
      l2 = (l2<<5) | testData[l2e];
      
      printf("l1 = 0x%10X ; l2 = 0x%10X\n",l1,l2);

      l1n = dpNormalizeLength(l1);
      l2n = dpNormalizeLength(l2);
      l3  = dpAddLengths(l1n,l2n);
      
      l1f  = (double)(l1 >>5)*pow(2.0,(double)(l1 &0x1F)-DBf-DBf-16);
      l2f  = (double)(l2 >>5)*pow(2.0,(double)(l2 &0x1F)-DBf-DBf-16);
      l1fn = (double)(l1n>>5)*pow(2.0,(double)(l1n&0x1F)-DBf-DBf-16);
      l2fn = (double)(l2n>>5)*pow(2.0,(double)(l2n&0x1F)-DBf-DBf-16);
      l3f  = (double)(l3 >>5)*pow(2.0,(double)(l3 &0x1F)-DBf-DBf-16);
      l3r  = l1f+l2f;
      
      l1er = l1fn - l1f ; if(l1er<0.0) l1er = -l1er;
      if(l1f==0.0) l1fer=l1er; else l1fer=l1er/l1f;
      l2er = l2fn - l2f ; if(l2er<0.0) l2er = -l2er;
      if(l2f==0.0) l2fer=l2er; else l2fer=l2er/l2f;
      le  = l3r-l3f    ; if( le<0.0) le  = -le;
      if(l3r!=0.0) lef=le/l3r; else lef=le;
      
      if(l1fer>pow(2.0,-DBf-16)) err=1,l1x=1; else l1x=0;
      if(!isNormal(l1n)) err=1;
      if(l2fer>pow(2.0,-DBf-16)) err=1,l2x=1; else l2x=0;
      if(!isNormal(l2n)) err=1;
      if(lef>pow(2.0,-DBf-16)) err=1;
      
      if(err==1)
      {  printf("-------------------------------------- ERROR\n");
         errs++;
      }
      
      printf("dpNormalize(0x%X) = 0x%X ",l1,l1n);
      if(l1x) printf("(INCORRECT) Fractional Error = %g",l1fer);
      else if(!isNormal(l1n)) printf("(NOT NORMALIZED)");
      printf("\n");
      printf("dpNormalize(0x%X) = 0x%X ",l2,l2n);
      if(l2x) printf("(INCORRECT) Fractional Error = %g",l2fer);
      else if(!isNormal(l2n)) printf("(NOT NORMALIZED)");
      printf("\n");
      printf("dpAddLengths     (%10g,%10g) = ",l1f,l2f);
      printf("%10g ; %10g ; Error = %10f fractional\n",l3f,l3r,lef);

      if(err==1)
         printf("^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ ERROR\n");
         
      printf("\n");
      
      skipTest:

      tests++;
   }
   
   fprintf(stderr,"Total of %d errors found\n",errs);
   return 0;
}

/*-----------------------------------------------------------*/
/*                                            isNormal(...)  */
/*-----------------------------------------------------------*/

int isNormal(int l)
/* Determine whether l is normalized or not. Return a truth  */
/*    value accordingly                                      */
{  int m,e;
   if(l==0) return 1;
   
   m = (int)((unsigned int)l>>5);
   e = (l&0x1F);
   if(e==0) return 1;
   if(m>=(1<<(Bf+16-1))) return 1;
   return 0;
}

