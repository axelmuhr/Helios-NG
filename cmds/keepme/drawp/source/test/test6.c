/*-----------------------------------------------------------*/
/*                                      source/test/test6.c  */
/*-----------------------------------------------------------*/

/* This test is for copying an area of a pixmap in the same   */
/*    pix-map, ie. scrolling. This file only tests the GXcopy */
/*   combination mode, as scrolling without GXcopy is         */
/*   awkward to test. The next test routine should test other */
/*   orientation copying using a different pixmap, by copying */
/*   a pattern from an off-screen pixmap into eight different */
/*   locations per frame.                                     */

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
#include "drawp/debug.h"
#include "sys/time.h" /* Local versions of non-ANSI file */
int gettimeofday(struct timeval*,struct timezone*);
DpPixmap_t sc;

/*-----------------------------------------------------------*/
/*                                                   main()  */
/*-----------------------------------------------------------*/

void waitSecond(void);
void doFrame(DpPixmap_t*,int,int,int,int,int,int);
int submain()
{  int intact,ch;
   int cx,cy,sx,sy,sd,dir;

   printf("This file tests the scrolling operation by drawing  \n");
   printf("  a circle in the centre of the screen and then     \n");
   printf("  scrolling it in some direction. The direction     \n");
   printf("  cyles through eight different directions, and     \n");
   printf("  the direction has cycled, the distance by which   \n");
   printf("  the pattern is scrolled is increased by seven     \n");
   printf("  pixels. The initial scrolling distance is three   \n");
   printf("  pixels.                                           \n");
   printf("As usual, if you press 'i' before pressing RETURN a \n");
   printf("  prompted pause will be introduced between frames  \n");
   
   printf("\nPress RETURN...\n");
   intact = 0;
   while((ch=getchar())!='\n') if(ch=='i') intact=1;

   if(dpMapScreen(&sc,2)==NULL)
   {  printf("Cannot map screen\n");
      return 1;
   }
   
   setMask(&sc,0,0);
   
   cx = sc.sizeX/2;
   cy = sc.sizeY/2;
   sx = cx/2; sy = cy/2;
   
   for(sd=3; sd<sx&&sd<sy; sd+=7)
   {  for(dir=0;dir<8;dir++)
      {  if(intact)
         {  printf("Press RETURN for next frame ...\n");
            while(getchar()!='\n');
            waitSecond();
         }
         doFrame(&sc,cx,cy,sx,sy,sd,dir);
         if(!intact) waitSecond();
      }
   }
   
   printf("Operation complete, press RETURN\n");
   while(getchar()!='\n');
   dpDestructPixmap(&sc,0);
   return 0;
}

/*---------------------------------------------------------------*/
/*                                                 doFrame(...)  */
/*---------------------------------------------------------------*/

void clearArea(DpPixmap_t*,int,int,int,int,int);
void doFrame(DpPixmap_t *sc,
             int cx,int cy,
             int sx,int sy,
             int sd,int dir)
{  DpGraphicsContext_t gc;
   int x,y,r,err;
   int slx,sty,dlx,dty,bx,by;

   clearArea(sc,0,0,sc->sizeX,sc->sizeY,0);
   
   r = sy+1 ;
   
   for(y=-sy; y<=sy; y++)
   {  x = r*r - y*y;
      x = ((int)sqrt((double)x)*sx)/sy;
      if(x>=sx) x=sx-1;
      clearArea(sc,cx-x,cy+y,cx+x,cy+y+1,1);
   }
   
   waitSecond();
   
   gc.depth       = 1;
   gc.function    = GXcopy;
   gc.planeMask   = 0xFFFFFFFF;
   gc.foreground  = 0;
   gc.background  = 0;
   gc.fillStyle   = FillSolid;
   gc.tile        = NULL;
   gc.stipple     = NULL;
   gc.tsXOrigin   = 0;
   gc.tsYOrigin   = 0;
   gc.clipXOrigin = 0;
   gc.clipYOrigin = 0;
   gc.clipMask    = msk;
   
   slx=cx-sx-sd; sty=cy-sy-sd;
   switch(dir)
   {  case 0: dlx=slx   ; dty=sty-sd; break;
      case 1: dlx=slx+sd; dty=sty-sd; break;
      case 2: dlx=slx+sd; dty=sty   ; break;
      case 3: dlx=slx+sd; dty=sty+sd; break;
      case 4: dlx=slx   ; dty=sty+sd; break;
      case 5: dlx=slx-sd; dty=sty+sd; break;
      case 6: dlx=slx-sd; dty=sty   ; break;
      case 7: dlx=slx-sd; dty=sty-sd; break;
      default: printf("Internal Error in test prog\n"); exit(1);
   }

   bx=(sx+sd)*2; by=(sy+sd)*2;

   if(slx<0) { err=-slx; bx-=err; slx+=err; dlx+=err; }
   if(sty<0) { err=-sty; by-=err; sty+=err; dty+=err; }
   if(dlx<0) { err=-dlx; bx-=err; slx+=err; dlx+=err; }
   if(dty<0) { err=-dty; by-=err; sty+=err; dty+=err; }
   
   if(slx+bx>sc->sizeX) { err=slx+bx-sc->sizeX; bx-=err; }
   if(sty+by>sc->sizeY) { err=sty+by-sc->sizeY; by-=err; }
   if(dlx+bx>sc->sizeX) { err=dlx+bx-sc->sizeX; bx-=err; }
   if(dty+by>sc->sizeY) { err=dty+by-sc->sizeY; by-=err; }

   dpUnClippedCopy(sc,sc,slx,sty,dlx,dty,bx,by,&gc);
   
   clearArea(sc,0,0,sc->sizeX,dty,0);
   clearArea(sc,0,dty,dlx,dty+by,0);
   clearArea(sc,dlx+bx,dty,sc->sizeX,dty+by,0);
   clearArea(sc,0,dty+by,sc->sizeX,sc->sizeY,0);
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
   gc.clipXOrigin = 0;
   gc.clipYOrigin = 0;
   gc.clipMask    = (pm==&sc)?msk:NULL;
   dpUnClippedRect(&gc,pm,lx,ty,rx,by);

}

