/*-----------------------------------------------------------*/
/*                                      source/test/test7.c  */
/*-----------------------------------------------------------*/

/* This is similar to test 6, except that instead of the     */
/*    being plotted onto the screen and then scrolled, it is */
/*    plotted into the same place of an off-screen pixmap    */
/*    which is the same size as the screen, and then copied  */
/*    out into the respective location on the screen. The    */
/*    other difference is that the combination modes and     */
/*    colours are cycled to test them as well.               */

/*-----------------------------------------------------------*/
/*                                            Include files  */
/*-----------------------------------------------------------*/

#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include "math.h"
#include "drawp/public.h"
#include "masks.h"
#include "sys/time.h" /* Local versions of non-ANSI file */
int gettimeofday(struct timeval*,struct timezone*);
void waitSecond(void);
DpPixmap_t sc;

/*-----------------------------------------------------------*/
/*                                                   main()  */
/*-----------------------------------------------------------*/

void waitSecond(void);
void doFrame(DpPixmap_t*,DpPixmap_t*,int,int,int,int,int,int,int,int,int,int,int);
void incrementCombine(int *sb,int *sf,int *db,int *fn);
int getCorrectCombine(int *sb,int *sf,int *db,int *fn);
int submain()
{  int intact,ch;
   DpPixmap_t pm;
   int cx,cy,sx,sy,sd,dir;
   int sb,sf,db,fn;
   int i;

   printf("This file tests general block copy operations with  \n");
   printf("  the various combination modes taken into account  \n");
   printf("  it does this by drawing a circle in an off-screen \n");
   printf("  pix-map, and then copying it into various places  \n");
   printf("  on the screen, just like the scrlling test6,      \n");
   printf("  so that the combination in all the various        \n");
   printf("  directions is tested.                             \n");
   printf("As usual, if you press 'i' before pressing RETURN a \n");
   printf("  prompted pause will be introduced between frames  \n");
   printf("Each frame should appear as a colour 1 circle in a  \n");
   printf("  colour 0 box somewhere on the screen, around this \n");
   printf("  box, the rest of the screen will form a border of \n");
   printf("  one or other colour                               \n");
   printf("This test should take about 3 minutes to complete   \n");
   printf("The circle will not necassarily have the correct    \n");
   printf("  aspect ratio.                                     \n");

   printf("\nPress RETURN...\n");
   while((ch=getchar())!='\n') if(ch=='i') intact=1;

   if(dpMapScreen(&sc,2)==NULL)
   {  printf("Cannot map screen\n");
      return 1;
   }
   
   setMask(&sc,3,4);
   
   if(dpConstructPixmap(&pm,sc.sizeX,sc.sizeY,1)==NULL)
   {  printf("Cannot allocate an off-screen pixmap\n");
      return 1;
   }

   cx = sc.sizeX/2;
   cy = sc.sizeY/2;
   sx = cx/2; sy = cy/2;

   sb=sf=db=fn=0;
   if(sc.sizeX<sc.sizeY) sd=sc.sizeX; else sd=sc.sizeY;
   sd = (sd/4)-5;
   if((sd%32)==0) sd-=5;
   printf("First scroll distance is %d, then %d\n",sd,(sd/32)*32+32);
   for(i=0; i<2; i++)
   {  for(dir=0;dir<8;dir++)
      while(getCorrectCombine(&sb,&sf,&db,&fn))
      {  if(intact)
         {  printf("\n----------------------------------------------\n");
            printf("About to plot from a %d over %d pixmap into\n",sf,sb);
            printf("  into a %d screen with function 0x%X\n",db,fn);
            printf("Press RETURN for next frame ...\n");
            while(getchar()!='\n');
            waitSecond();
         }
         doFrame(&sc,&pm,cx,cy,sx,sy,sd,dir,sb,sf,db,fn,intact);
         if(!intact) waitSecond();
         incrementCombine(&sb,&sf,&db,&fn);
      }
      sd = (sd/32)*32+32;
   }

   printf("Operation complete, press RETURN\n");
   while(getchar()!='\n');
   dpDestructPixmap(&sc,0);
   return 0;
}

/*---------------------------------------------------------------*/
/*                                              waitSecond(...)  */
/*---------------------------------------------------------------*/

void waitSecond()
{  struct timeval tp;
   long          usec,udly;

   gettimeofday(&tp,NULL);
   usec = tp.tv_usec;
   do 
   {  gettimeofday(&tp,NULL); 
      udly = tp.tv_usec - usec;
      if(udly<0) udly+=1000000;
   } while (udly<500000);
}

/*---------------------------------------------------------------*/
/*                                                 doFrame(...)  */
/*---------------------------------------------------------------*/

void clearArea(DpPixmap_t*,int,int,int,int,int);
void doFrame(DpPixmap_t *sc,
             DpPixmap_t *pm,
             int cx,int cy,
             int sx,int sy,
             int sd,int dir,
             int sb,int sf,int db,int fn,
             int intact)
{  DpGraphicsContext_t gc;
   int x,y,r,err;
   int slx,sty,dlx,dty,bx,by;
   
   clearArea(sc,0,0,sc->sizeX,sc->sizeY,db);
   clearArea(pm,0,0,sc->sizeX,sc->sizeY,sb);
   
   r = sy-3 ;
   
   for(y=-sy+3; y<=sy-3; y++)
   {  x = r*r - y*y;
      x = ((int)sqrt((double)x)*(sx-3))/(sy-3);
      if(x>=sx) x=sx-1;
      clearArea(pm,cx-x,cy+y,cx+x,cy+y+1,sf);
   }
   
   waitSecond();
   
   gc.depth       = 1;
   gc.function    = fn;
   gc.planeMask   = 0xFFFFFFFF;
   gc.foreground  = 0;
   gc.background  = 0;
   gc.fillStyle   = FillSolid;
   gc.tile        = NULL;
   gc.stipple     = NULL;
   gc.tsXOrigin   = 0;
   gc.tsYOrigin   = 0;
   gc.clipXOrigin = 3;
   gc.clipYOrigin = 4;
   gc.clipMask    = msk;
   
   slx=cx-sx; sty=cy-sy;
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

   bx=sx*2; by=sy*2;

   if(slx<0) { err=-slx; bx-=err; slx+=err; dlx+=err; }
   if(sty<0) { err=-sty; by-=err; sty+=err; dty+=err; }
   if(dlx<0) { err=-dlx; bx-=err; slx+=err; dlx+=err; }
   if(dty<0) { err=-dty; by-=err; sty+=err; dty+=err; }
   
   if(slx+bx>sc->sizeX) { err=slx+bx-sc->sizeX; bx-=err; }
   if(sty+by>sc->sizeY) { err=sty+by-sc->sizeY; by-=err; }
   if(dlx+bx>sc->sizeX) { err=dlx+bx-sc->sizeX; bx-=err; }
   if(dty+by>sc->sizeY) { err=dty+by-sc->sizeY; by-=err; }
   
   if(intact)
   {  printf("Source      rectangle is (%d,%d)-(%d,%d)\n",
              slx,sty,slx+bx,sty+by);
      printf("Destination rectangle is (%d,%d)-(%d,%d)\n",
              dlx,dty,dlx+bx,dty+by);
   }

   dpCopyArea(pm,sc,slx,sty,dlx,dty,bx,by,&gc);
   
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
   gc.clipXOrigin = 3;
   gc.clipYOrigin = 4;
   gc.clipMask    = (pm==&sc)?msk:NULL;
   dpDrawRect(&gc,pm,lx,ty,rx,by);

}

/*---------------------------------------------------------------*/
/*                                       getCorrectCombine(...)  */
/*---------------------------------------------------------------*/

int getCorrectCombine(int *sb,int *sf,int *db,int *fn)
{  int bitf,bitb;

   do
   {  bitf = 3-((*sf<<1)|*db);
      bitb = 3-((*sb<<1)|*db);
      if(((*fn>>bitb)&1)==0 && ((*fn>>bitf)&1)==1) return 1;
      fflush(stdout);
      incrementCombine(sb,sf,db,fn);
      if(*sb==0&&*sf==0&&*db==0&&*fn==0) return 0;
   } while(1);
   return 0;
}

/*---------------------------------------------------------------*/
/*                                        incrementCombine(...)  */
/*---------------------------------------------------------------*/

void incrementCombine(int *sb,int *sf,int *db,int *fn)
{  if(*sb==0) { *sb=1; return; } else *sb=0;
   if(*sf==0) { *sf=1; return; } else *sf=0;
   *fn = (*fn+1)&0xF; if(*fn>0) return;
   if(*db==0) { *db=1; return; } else *db=0;
}

