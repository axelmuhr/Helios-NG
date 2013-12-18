/*-----------------------------------------------------------*/
/*                                      source/test/test5.c  */
/*-----------------------------------------------------------*/

/* Test for tiling a pixel map: This test will cycle through */
/*   any applicable draw modes                               */

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

/*-----------------------------------------------------------*/
/*                                                   main()  */
/*-----------------------------------------------------------*/

void doTiledPattern(DpPixmap_t*,int,int,int*,int*,int*,int*,int);
int submain()
{  int intact,ch;
   int sb,sf,db,fn;
   int ssx,ssy;

   printf("This file is to test the tiling facility of the   \n");
   printf("  blitter rectangles. We start by tiling a 127x127\n");
   printf("  pixmap onto the screen, and we then decrease by \n");
   printf("  5(x), and 3(y), the size of the pixmap to be    \n");
   printf("  tiled from frame to frame. There is a one-second\n");
   printf("  delay between each frame unless you press 'i'   \n");
   printf("  before pressing RETURN next, in which case you  \n");
   printf("  will get a pause where you press return between \n");
   printf("  each frame.                                     \n");
   printf("The pattern being tiled should always appear as a \n");
   printf("  colour 1 circle on a colour 0 background. All   \n");
   printf("  combinations of source pixmap versus screen     \n");
   printf("  colours and plotting functions are used.        \n");
   
   printf("\nPress RETURN...\n");
   intact = 0;
   while((ch=getchar())!='\n') if(ch=='i') intact=1;

   if(dpMapScreen(&sc,2)==NULL)
   {  printf("Cannot map screen\n");
      return 1;
   }
   
   setMask(&sc,32,32);
   
   sf=sb=db=fn=0;
   for(ssx=127,ssy=127; ssx>0&&ssy>0; ssx-=5,ssy-=3)
   {  /*ssx=ssy=32;*/
      if(intact)
      {  printf("Source pixmap size (%d,%d) ...\n",ssx,ssy);
         printf("Press RETURN\n");
         while(getchar()!='\n');
      }
      doTiledPattern(&sc,ssx,ssy,&sf,&sb,&db,&fn,intact);
      if(!intact) waitSecond();
   }
   printf("Operation complete, press RETURN\n");
   while(getchar()!='\n');
   dpDestructPixmap(&sc,0);
   return 0;
}

/*---------------------------------------------------------------*/
/*                                          doTiledPattern(...)  */
/*---------------------------------------------------------------*/

void clearArea(DpPixmap_t*,int,int,int,int,int);
void doTiledPattern(DpPixmap_t *sc,
                    int ssx,int ssy,
                    int *sf,int *sb,int *db,int *fn,
                    int intact)
{  int bitb,bitf;
   DpPixmap_t pm;
   int cx,cy,x,y,r;
   DpGraphicsContext_t gc;
   struct timeval tp;
   long int       usec;

   do
   {  bitf = 3 - ((*sf<<1)|*db);
      bitb = 3 - ((*sb<<1)|*db);
      if((((*fn>>bitf)&1)==1)&&(((*fn>>bitb)&1)==0)) break;
      if(*sf==0) { *sf=1; continue; } else *sf=0;
      if(*sb==0) { *sb=1; continue; } else *sb=0;
      if(*db==0) { *db=1; continue; } else *db=0;
      *fn = (*fn+1)&0xF;
   } while(1);
   
   if(dpConstructPixmap(&pm,ssx,ssy,1)==NULL)
   {  printf("Could not create an off-screen pixmap\n");
      exit(1);
   }

   clearArea(&pm,0,0,ssx,ssy,*sb);

   r = (ssy/2) +1 ;
   cx=ssx/2; cy=ssy/2;
   
   for(y=-ssy/2; y<=ssy/2; y++)
   {  x = r*r - y*y;
      x = ((int)sqrt((double)x)*(ssx/2))/(ssy/2);
      clearArea(&pm,cx-x,cy+y,cx+x,cy+y+1,*sf);
   }
   
   clearArea(sc,0,0,sc->sizeX,sc->sizeY,*db);
   
   gc.depth       = 1;
   gc.function    = *fn;
   gc.planeMask   = 0xFFFFFFFF;
   gc.foreground  = 0;
   gc.background  = 0;
   gc.fillStyle   = FillTiled;
   gc.tile        = &pm;
   gc.stipple     = NULL;
   gc.tsXOrigin   = 0;
   gc.tsYOrigin   = 0;
   gc.clipXOrigin = -32;
   gc.clipYOrigin = -32;
   gc.clipMask    = msk;
   
#ifndef HeliosMode
   gettimeofday(&tp,NULL);
   usec = tp.tv_usec;
#else
   usec = 0;
   tp.tv_usec = 0;
#endif
   dpUnClippedRect(&gc,sc,0,0,sc->sizeX,sc->sizeY);
#ifndef HeliosMode
   gettimeofday(&tp,NULL);
   usec = tp.tv_usec - usec;
   if (usec<0) usec+=1000000;
   /*printf("Time taken = %d microseconds\n",usec);*/
#endif

   if(intact)
   {  printf("That screen was displayed from a source pixmap\n");
      printf("   in colour %d over %d onto the screen painted\n",*sf,*sb);
      printf("   in %d with plot function %d\n\n",*db,*fn);
   }

   if(*sf==0) { *sf=1; return; } else *sf=0;
   if(*sb==0) { *sb=1; return; } else *sb=0;
   if(*db==0) { *db=1; return; } else *db=0;
   *fn = (*fn+1)&0xF;

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
   gc.clipXOrigin = -32;
   gc.clipYOrigin = -32;
   gc.clipMask    = (pm==&sc)?msk:NULL;
   dpUnClippedRect(&gc,pm,lx,ty,rx,by);

}

