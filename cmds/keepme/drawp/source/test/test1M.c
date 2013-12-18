/*-----------------------------------------------------------*/
/*                                     source/test/test1M.c  */
/*-----------------------------------------------------------*/

/* $Header: test1M.c,v 1.4 90/03/03 18:00:51 charles Locked $ */
/* $Source: /server/usr/users/charles/world/drawp/RCS/source/test/test1M.c,v $ */

/* This file is used to test the internal arithmetic functions  */
/*    provided by the draw package                              */

/* tests the following functions quite thoroughly: */

/* dpVectorLength     (...)  */
/* dpUnitComponent    (...)  */
/* dpDecodeLength     (...)  */
/* dpMeasureComponent (...)  */

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
   int dxs,dxi,dxfi;
   int dys,dyi,dyfi;
   int wi,wfi;
   int     dx , dy , w , l , ux , ld , mx ;
   double  dxf, dyf, wf, lf, uxf, ldf, mxf;
   double                lr, uxr, ldr, mxr;
   double                le, uxe, lde, mxe, lfe;
   int err,errs,tests,totalTests;
   
   totalTests = testSize*testSize*2*testSize*testSize*2*testSize*testSize;
   tests = 0; errs = 0;
   for(dxs=0;dxs<2;dxs++)
   for(dxi=0;dxi<testSize;dxi++)
   for(dxfi=0;dxfi<testSize;dxfi++)
   for(dys=0;dys<2;dys++)
   for(dyi=0;dyi<testSize;dyi++)
   for(dyfi=0;dyfi<testSize;dyfi++)
   for(wi=0;wi<testSize;wi++)
   for(wfi=0;wfi<testSize;wfi++)
   {  err = 0;
 
      if(!(tests%100))
         fprintf(stderr,"%6d/%6d tests performed. %6d errors found\n",
                        tests,totalTests,errs);

      dx  = (testData[dxi]<<Bf)+testData[dxfi]; if(dxs) dx=-dx;
      dy  = (testData[dyi]<<Bf)+testData[dyfi]; if(dys) dy=-dy;
      w   = (testData[ wi]<<Bf)+testData[ wfi];
      
      if(dx==0&&dxs) goto skipTest;
      if(dy==0&&dys) goto skipTest;
      if(dx>32767||dx<-32768) goto skipTest;
      if(dy>32767||dy<-32768) goto skipTest;
      if( w>32767|| w<     0) goto skipTest;
      
      printf("dx = 0x%10X ; dy = 0x%10X ; w = 0x%10X\n",dx,dy,w);

      l   = dpVectorLength(dx,dy);
      ux  = dpUnitComponent(dx,l);
      ld  = dpDecodeLength(l);
      mx  = dpMeasureComponent(w,ux);
      
      dxf = ((double)dx)/pow(2.0,DBf);
      dyf = ((double)dy)/pow(2.0,DBf);
      wf  = ((double) w)/pow(2.0,DBf);
      
      lf  = (((double)(l>>5))/pow(2.0,16.0+DBf+DBf))*pow(2.0,(double)(l&0x1F));
      uxf = ((double)ux)/pow(2.0,DBf+16.0);
      ldf = ((double)ld)/pow(2.0,DBf);
      mxf = ((double)mx)/pow(2.0,DBf);

      lr  = sqrt((dxf*dxf)+(dyf*dyf));
      uxr = (lf==0.0)?(dxf):(dxf/lf);
      ldr = (double) ( /*(int)*/ (lf*pow(2.0,DBf)) );
      mxr = uxf * wf;
      
      le  =  lr- lf; if( le<0.0)  le=- le;
      uxe = uxr-uxf; if(uxe<0.0) uxe=-uxe;
      lde = ldr-ldf; if(lde<0.0) lde=-lde;
      mxe = mxr-mxf; if(mxe<0.0) mxe=-mxe;
      lfe = le/(lr<0.0?-lr:lr>0.0?lr:1.0);
      
      if(!isNormal(l))      err=1;
      if(lfe>pow(2.0,-DBf)) err=1;
      if(uxe>pow(2.0,-DBf)) err=1;
      if(lde>pow(2.0,-DBf)) err=1;
      if(mxe>pow(2.0,-DBf)) err=1;
      
      if(err==1)
      {  printf("-------------------------------------- ERROR\n");
         errs++;
      }
      
      printf("dpVectorLength     (%15.5f,%15.5f) = ",dxf,dyf);
      printf("%15.5f ; %15.5f ; Error = %15.5f fractional\n",lf,lr,lfe);
      if(!isNormal(l)) printf("(Result 0x%X is not normalized)\n",l);
      printf("dpUnitComponent    (%15.5f,%15.5f) = ",dxf,lf);
      printf("%15.5f ; %15.5f ; Error = %15.5f\n",uxf,uxr,uxe);
      printf("dpDecodeLength     (%15.5f)                 = ",lf);
      printf("%15.5f ; %15.5f ; Error = %15.5f\n",ldf,ldr,lde);
      printf("dpMeasureComponent (%15.5f,%15.5f) = ",wf,uxf);
      printf("%15.5f ; %15.5f ; Error = %15.5f\n",mxf,mxr,mxe);

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

