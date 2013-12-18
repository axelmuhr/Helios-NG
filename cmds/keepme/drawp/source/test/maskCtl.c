/*-----------------------------------------------------------------*/
/*                                          source/test/maskCtl.c  */
/*-----------------------------------------------------------------*/

/* This file is linked to by each test program and contains the    */
/*    'main' routine which traps the 'm' option on the command     */
/*    line to assist drawing through masks                         */

/*-----------------------------------------------------------------*/
/*                                                  Include files  */
/*-----------------------------------------------------------------*/

#include <stdlib.h>
#include <stddef.h>
#ifdef HeliosMode
#include <syslib.h>
#include <attrib.h>
#include <nonansi.h>
#endif
#include <stdio.h>
#include <string.h>
#include "drawp/public.h"
#include "drawp/debug.h"
#include "masks.h"
#ifndef HeliosMode
#include "sys/time.h" /* Local fixed-up versions of non-ANSI header */
int gettimeofday(struct timeval*,struct timezone*);
#endif

/*-----------------------------------------------------------------*/
/*                                               Static variables  */
/*-----------------------------------------------------------------*/

DpPixmap_t mpm,*msk;
int enm=0;

/*-----------------------------------------------------------------*/
/*                                                         main()  */
/*-----------------------------------------------------------------*/

extern int submain(int,char**);
int main(int argc,char **argv)
{
#ifdef HeliosMode
   {  Attributes attr;
      if(GetAttributes(Heliosno(stdin),&attr)<0)
         printf("Cannot disable echo\n"), exit(1);
      RemoveAttribute(&attr,ConsoleEcho);
      if(SetAttributes(Heliosno(stdin),&attr)<0)
         printf("Cannot disable echo\n"), exit(1);
   }
#endif
   if(argc>1&&argv[1][0]=='m') enm=1; else enm=0;
   if(enm==0)
      printf("Add the 'm' option to test in addition the masking\n");
   msk = enm?&mpm:NULL;
   return submain(argc,argv);
}

/*---------------------------------------------------------------*/
/*                                                 setMask(...)  */
/*---------------------------------------------------------------*/

static void mcClearArea(DpPixmap_t *pm,int lx,int ty,int rx,int by,int c);
void setMask(DpPixmap_t *sc,int offX,int offY)
{  DpGraphicsContext_t gc;
   DpPixmap_t im,ht;
   int x,y,i,s;
   unsigned long *hp;

   if(!enm) return;

   if(dpConstructPixmap(&mpm,sc->sizeX+2*offX,sc->sizeY+2*offY,1)==NULL)
   {  printf("Cannot allocate mask pixmap\n");
      exit(1);
   }

   if(dpConstructPixmap(&im,sc->sizeX+2*offX,sc->sizeY+2*offY,1)==NULL)
   {  printf("Cannot allocate inverted mask pixmap\n");
      exit(1);
   }

   if(dpConstructPixmap(&ht,32,32,1)==NULL)
   {  printf("Cannot allocate hatch pattern pixmap\n");
      exit(1);
   }

   mcClearArea(&mpm,0,0,mpm.sizeX,mpm.sizeY,1);
   for(s=(sc->sizeY*4)/10,i=8;i>0;s-=sc->sizeY/20,i--)
   {  for(y=-s;y<s;y++)
      {  x = ((s+y)*sc->sizeX)/(sc->sizeY*2);
         mcClearArea(&mpm,
                   offX+sc->sizeX/2-x,offY+sc->sizeY/2+y,
                   offX+sc->sizeX/2+x,offY+sc->sizeY/2+y+1,
                   (i&1));
      }
   }

   gc.depth       = 1;
   gc.function    = GXcopyInverted;
   gc.planeMask   = 0xFFFFFFFF;
   gc.foreground  = 0;
   gc.background  = 0;
   gc.fillStyle   = FillTiled;
   gc.tile        = &mpm;
   gc.stipple     = NULL;
   gc.tsXOrigin   = 0;
   gc.tsYOrigin   = 0;
   gc.clipXOrigin = 0;
   gc.clipYOrigin = 0;
   gc.clipMask    = NULL;
   dpUnClippedRect(&gc,&im,0,0,im.sizeX,im.sizeY);
   
   for(i=0,hp=((unsigned long*)ht.rawBase)+1;i<32;i++)
   {  if(i&1) *hp++ = 0xAAAAAAAAUL;
      else    *hp++ = 0x55555555UL;
   }

   mcClearArea(sc,0,0,sc->sizeX,sc->sizeY,1);

   gc.depth       = 1;
   gc.function    = GXcopy;
   gc.planeMask   = 0xFFFFFFFF;
   gc.foreground  = 0xFFFFFFFF;
   gc.background  = 0x00000000;
   gc.fillStyle   = FillOpaqueStippled;
   gc.tile        = NULL;
   gc.stipple     = &ht;
   gc.tsXOrigin   = 0;
   gc.tsYOrigin   = 0;
   gc.clipXOrigin = -offX;
   gc.clipYOrigin = -offY;
   gc.clipMask    = &im;
   dpUnClippedRect(&gc,sc,0,0,sc->sizeX,sc->sizeY);
      
   printf("Mask bit-map displayed on screen. Press RETURN\n");
   while(getchar()!='\n');
   
   dpDestructPixmap(&ht,0);
   dpDestructPixmap(&im,0);
   
}   
      
/*---------------------------------------------------------------*/
/*                                               clearArea(...)  */
/*---------------------------------------------------------------*/

static void mcClearArea(DpPixmap_t *pm,int lx,int ty,int rx,int by,int c)
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
   gc.clipMask    = NULL;
   dpUnClippedRect(&gc,pm,lx,ty,rx,by);

}

/*---------------------------------------------------------------*/
/*                                              waitSecond(...)  */
/*---------------------------------------------------------------*/

void waitSecond()
{
#ifdef HeliosMode
   Delay(500000);
#else
   struct timeval tp;
   long          usec,udly;

   gettimeofday(&tp,NULL);
   usec = tp.tv_usec;
   do 
   {  gettimeofday(&tp,NULL); 
      udly = tp.tv_usec - usec;
      if(udly<0) udly+=1000000;
   } while (udly<500000);
#endif
}
