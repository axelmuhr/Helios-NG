/*-----------------------------------------------------------*/
/*                                     source/test/test1P.c  */
/*-----------------------------------------------------------*/

/* This is the first test for the polygon filling primitives */

/*-----------------------------------------------------------*/
/*                                            Include files  */
/*-----------------------------------------------------------*/

#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include "math.h"
#include "drawp/public.h"
#include "masks.h"
#include "drawp/debug.h"
#include "sys/time.h" /* Local versions of non-ANSI file */
int gettimeofday(struct timeval*,struct timezone*);
DpPixmap_t sc;
#define Pi 3.1415926535897932384626

/*-----------------------------------------------------------*/
/*                                                   main()  */
/*-----------------------------------------------------------*/

#define PolyN 8
void waitSecond(void);
void clearArea(DpPixmap_t*,int,int,int,int,int);
int submain(int argc,char **argv)
{  DpGraphicsContext_t gc,pgc,cgc,lgc;
   int fillRule;
   static DpPoint_t pnts[4]
    = { {100,250}, {350,50}, {300,250}, {50,50} };
   DpPoint_t pl[PolyN+1];
   int       pdx[PolyN],pdy[PolyN],i;
   double th;
   int rdx,rdy,cx,cy;
   DpPixmap_t pm,tpm,stpm;
   
   if( argc>1 && argv[argc-1][0]=='w')
   {  fillRule=WindingRule;
   } else
   {  printf("Add a 'w' to have the non-zero winding rule\n");
      fillRule=EvenOddRule;
   }
   printf("This is the first polygon primitive test code\n\n");
   printf("First clear the screen\n");

   printf("Press RETURN\n");
   while(getchar()!='\n');
   
   if(dpMapScreen(&sc,2)==NULL)
   {  printf("Cannot map screen\n");
      return 1;
   }

   setMask(&sc,5,5);
   
   gc.depth       = 1;
   gc.function    = GXcopy;
   gc.planeMask   = 0xFFFFFFFF;
   gc.foreground  = 1;
   gc.background  = 0;
   gc.fillStyle   = FillSolid;
   gc.tile        = NULL;
   gc.stipple     = NULL;
   gc.tsXOrigin   = 0;
   gc.tsYOrigin   = 0;
   gc.clipXOrigin = -5;
   gc.clipYOrigin = -5;
   gc.clipMask    = msk;
   gc.fillRule    = fillRule;
   
   printf("\nFirst test, a cross-wired trapesium\n");
   printf("Press RETURN\n");
   while(getchar()!='\n');
   
   clearArea(&sc,0,0,sc.sizeX,sc.sizeY,0);

   dpFillPolygon(&sc,&gc,pnts,4,Complex,CoordModeOrigin);
   
   printf("\nNext test : A string-polygon test\n");
   printf("Drawn into an off-screen pixmap first\n");
   printf("The polygon is filled with a tiled pattern and bounded by\n");
   printf("   a thin line\n");
   printf("Press RETURN ...\n");
   while(getchar()!='\n');
   waitSecond();
   
   clearArea(&sc,0,0,sc.sizeX,sc.sizeY,0);

   if(dpConstructPixmap(&pm,sc.sizeX,sc.sizeY,1)==NULL)
   {  printf("Could not construct off-screen pixmap\n");
      return 1;
   }

   if(dpConstructPixmap(&stpm,5,5,1)==NULL)
   {  printf("Could not construct off-screen pixmap\n");
      return 1;
   }
   *((long*)stpm.rawBase+1) = 0x00000003;
   *((long*)stpm.rawBase+2) = 0x00000003;
   *((long*)stpm.rawBase+3) = 0x00000000;
   *((long*)stpm.rawBase+4) = 0x00000008;
   *((long*)stpm.rawBase+5) = 0x00000000;
   
   if(dpConstructPixmap(&tpm,5*32,5*32,1)==NULL)
   {  printf("Could not construct off-screen pixmap\n");
      return 1;
   }
   
   cgc.depth       = 1;
   cgc.function    = GXcopy;
   cgc.planeMask   = 0xFFFFFFFF;
   cgc.foreground  = 1;
   cgc.background  = 0;
   cgc.fillStyle   = FillTiled;
   cgc.tile        = &stpm;
   cgc.stipple     = NULL;
   cgc.tsXOrigin   = 0;
   cgc.tsYOrigin   = 0;
   cgc.clipXOrigin = 5;
   cgc.clipYOrigin = 5;
   cgc.clipMask    = NULL;
   
   dpUnClippedRect(&cgc,&tpm,0,0,tpm.sizeX,tpm.sizeY);

   pgc.depth       = 1;
   pgc.function    = GXcopy;
   pgc.planeMask   = 0xFFFFFFFF;
   pgc.foreground  = 0xFFFFFFFF;
   pgc.background  = 0x00000000;
   pgc.fillStyle   = FillTiled;
   pgc.tile        = &tpm;
   pgc.stipple     = NULL;
   pgc.tsXOrigin   = 0;
   pgc.tsYOrigin   = 0;
   pgc.clipXOrigin = 5;
   pgc.clipYOrigin = 5;
   pgc.clipMask    = NULL;
   pgc.fillRule    = fillRule;   

   cgc.depth       = 1;
   cgc.function    = GXcopy;
   cgc.planeMask   = 0xFFFFFFFF;
   cgc.foreground  = 1;
   cgc.background  = 0;
   cgc.fillStyle   = FillSolid;
   cgc.tile        = NULL;
   cgc.stipple     = NULL;
   cgc.tsXOrigin   = 0;
   cgc.tsYOrigin   = 0;
   cgc.clipXOrigin = -5;
   cgc.clipYOrigin = -5;
   cgc.clipMask    = msk;

   lgc.depth       = 1;
   lgc.function    = GXcopy;
   lgc.planeMask   = 0xFFFFFFFF;
   lgc.foreground  = 1;
   lgc.background  = 0;
   lgc.fillStyle   = FillSolid;
   lgc.tile        = NULL;
   lgc.stipple     = NULL;
   lgc.tsXOrigin   = 0;
   lgc.tsYOrigin   = 0;
   lgc.clipXOrigin = 5;
   lgc.clipYOrigin = 5;
   lgc.clipMask    = NULL;
   lgc.lineWidth   = 0;
   lgc.lineStyle   = LineSolid;
   lgc.capStyle    = CapButt;
   lgc.joinStyle   = JoinMiter;
   lgc.dashOffset  = 0;
   lgc.dashLength  = 4;
   lgc.dashList    = NULL;
   
   cx=sc.sizeX/2; cy=sc.sizeY/2; rdx=3*cx/4; rdy=3*cy/4;
   for(i=0,th=0.0;i<PolyN;i++,th+=2.0*Pi/PolyN)
   {  pl[i].x=(int)(cx+rdx*cos(th)); 
      pl[i].y=(int)(cy+rdy*sin(th));
      pdx[i]=(int) ( (rdx/100.0)*(i+1)*sin(th));
      pdy[i]=(int) (-(rdy/100.0)*(i+1)*cos(th));
   }
   while(1)
   {  clearArea(&pm,0,0,sc.sizeX,sc.sizeY,0);
      dpFillPolygon(&pm,&pgc,pl,PolyN,Complex,CoordModeOrigin);
      pl[PolyN]=pl[0];
      dpDrawLines(&pm,&lgc,pl,PolyN+1,CoordModeOrigin);
      dpCopyArea(&pm,&sc,&cgc,0,0,sc.sizeX,sc.sizeY,0,0);
      for(i=0;i<PolyN;i++)
      {  pl[i].x+=pdx[i]; pl[i].y+=pdy[i];
         if(pl[i].x>=sc.sizeX) pl[i].x=sc.sizeX*2-pl[i].x-1,pdx[i]=-pdx[i];
         if(pl[i].y>=sc.sizeY) pl[i].y=sc.sizeY*2-pl[i].y-1,pdy[i]=-pdy[i];
         if(pl[i].x<0) pl[i].x=-pl[i].x,pdx[i]=-pdx[i];
         if(pl[i].y<0) pl[i].y=-pl[i].y,pdy[i]=-pdy[i];
      }
   }
            
   printf("\nTest program completed, press RETURN ...\n");
   while(getchar()!='\n');
   
   dpDestructPixmap(&sc,0);
   
   return 0;
}

/*---------------------------------------------------------------*/
/*                                               clearArea(...)  */
/*---------------------------------------------------------------*/

void clearArea(DpPixmap_t *pm,int lx,int ty,int rx,int by,int c)
{  DpGraphicsContext_t gc;

   gc.depth       = 1;
   gc.function    = GXcopy;
   gc.planeMask   = 0xFFFFFFFF;
   gc.foreground  = c;
   gc.background  = 0;
   gc.fillStyle   = FillSolid;
   gc.tile        = NULL;
   gc.stipple     = NULL;
   gc.tsXOrigin   = 0;
   gc.tsYOrigin   = 0;
   gc.clipXOrigin = -5;
   gc.clipYOrigin = -5;
   gc.clipMask    = (pm==&sc)?msk:NULL;
   dpUnClippedRect(&gc,pm,lx,ty,rx,by);

}

