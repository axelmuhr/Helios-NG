/*-----------------------------------------------------------*/
/*                                     source/test/test1L.c  */
/*-----------------------------------------------------------*/

/* This is the first test for the line-drawing primitives,   */
/*    simply draws a nice horizontal line.                   */

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

#define StringArc 100
void waitSecond(void);
void clearArea(DpPixmap_t*,int,int,int,int,int);
int submain(int argc,char **argv)
{  DpGraphicsContext_t gc;
   int x1,y1,x2,y2,cx,cy,dx,dy,dx1,dy1,dx2,dy2,i,j,k,x,y,r,rdx,rdy,dblDash;
   int sax1[StringArc],say1[StringArc],sax2[StringArc],say2[StringArc];
   static unsigned char dashes[]={4,2,1,2};
   double th,t1,t2;
   DpClipNode_t scn;
   
   if( argc>1 && argv[argc-1][0]=='d')
   {  dblDash=1;
   } else
   {  printf("Add a 'd' to have the lines in double-dash\n");
      dblDash=0;
   }
   printf("This is the first line-drawing primitive test code\n\n");
   printf("First map the screen\n");

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
   gc.lineWidth   = 0;
   gc.lineStyle   = dblDash?LineDoubleDash:LineSolid;
   gc.capStyle    = CapButt;
   gc.joinStyle   = JoinMiter;
   gc.dashOffset  = 0;
   gc.dashLength  = 4;
   gc.dashList    = dashes;
   
   scn.nextNode = NULL;
   scn.node[0]  = DpMakeNegInf(12);
   scn.node[1]  = DpMakeNegInf(2);
   scn.node[2]  = DpMakePosInf(2);
   scn.node[3]  = 0;
   scn.node[4]  = DpMakeNegInf(4);
   scn.node[5]  = 0;
   scn.node[6]  = sc.sizeX;
   scn.node[7]  = DpMakePosInf(4);
   scn.node[8]  = sc.sizeY;
   scn.node[9]  = DpMakeNegInf(2);
   scn.node[10] = DpMakePosInf(2);
   scn.node[11] = DpMakePosInf(12);
   
   printf("Clear the screen to col 0 and draw a line centred  \n");
   printf("  horizontally about the centre of the screen      \n");
   printf("  and half a screen-width in length                \n");
   
   printf("Press RETURN\n");
   while(getchar()!='\n');
   
   clearArea(&sc,0,0,sc.sizeX,sc.sizeY,0);
   x1 = sc.sizeX/4; x2 = 3*sc.sizeX/4;
   y1 = sc.sizeY/2; y2 = sc.sizeY/2;
   dpDrawLine(&sc,&gc,x1,y1,x2,y2);
   
   printf("First test complete. Press RETURN\n");
   while(getchar()!='\n');
   
   printf("Now similarly but vertical. Press RETURN\n");
   
   clearArea(&sc,0,0,sc.sizeX,sc.sizeY,0);
   x1 = sc.sizeX/2 ; x2 = sc.sizeX/2;
   y1 = sc.sizeY/4 ; y2 = 3*sc.sizeY/4;
   dpDrawLine(&sc,&gc,x1,y1,x2,y2);
   
   printf("Now draw a series of nearly horizontal lines, all\n");
   printf("  going through the centre point. All lines are  \n");
   printf("  drawn from left to right.                      \n");
   printf("Press RETURN\n");
   while(getchar()!='\n');
   
   clearArea(&sc,0,0,sc.sizeX,sc.sizeY,0);
   
   cx = sc.sizeX/2; cy = sc.sizeY/2;
   dx = 3*cx/4; dy = dx-1;
   for(;dy>-dx+1;dy-=3)
      dpDrawLine(&sc,&gc,cx-dx,cy-dy,cx+dx,cy+dy);

   printf("\nNext test : A set of lines forming the spokes of a wheel\n");
   printf("(36 lines in total, drawn from the centre)\n");
   printf("Press RETURN\n");
   while(getchar()!='\n');
   
   clearArea(&sc,0,0,sc.sizeX,sc.sizeY,0);
   
   cx = sc.sizeX/2; cy = sc.sizeY/2;
   r=3*sc.sizeX/8; if(r>3*sc.sizeY/8) r=3*sc.sizeY/8;
   for(th=0.0,i=0;i<36;th+=(2*Pi)*10/360,i++)
   {  x = (int)(cos(th)*r); y = (int)(sin(th)*r);
      dpDrawLine(&sc,&gc,cx,cy,cx+x,cy+y);
   }
   
   printf("\nNext test : A pretty pattern\n");
   printf("Press RETURN ...\n");
   while(getchar()!='\n');
   
   clearArea(&sc,0,0,sc.sizeX,sc.sizeY,0);
   
   cx=sc.sizeX/2; cy=sc.sizeY/2;
   rdx=7*sc.sizeX/16; rdy=7*sc.sizeY/16;
   for(t1=0.0,t2=3*Pi/180,i=0; i<90; t1+=28*Pi/180,t2+=42*Pi/180,i++)
   {   dpDrawLine(&sc,&gc,cx+(int)(cos(t1)*rdx),cy+(int)(sin(t1)*rdy),
                          cx+(int)(cos(t2)*rdx),cy+(int)(sin(t2)*rdy));
   }
   
   printf("\nNext test : A string-arc\n");
   printf("Press RETURN ...\n");
   while(getchar()!='\n');
   
   clearArea(&sc,0,0,sc.sizeX,sc.sizeY,0);

   sax1[0]=100;say1[0]=100;
   sax2[0]=300;say2[0]=320;
   gc.function=GXxor;
   dx1=3; dy1=5; dx2=7; dy2=11;
   for(i=0;1;i++)
   {  j=i%StringArc;
      k=(i+1)%StringArc;
      if(i>=StringArc) dpDrawLine(&sc,&gc,sax1[k],say1[k],sax2[k],say2[k]);
      x = sax1[j]+dx1; 
      y = say1[j]+dy1; 
      if(x>=sc.sizeX) { x=2*sc.sizeX-x-1; dx1=-dx1; }
      if(x<0) { x=-x; dx1=-dx1; }
      if(y>=sc.sizeY) { y=2*sc.sizeY-y-1; dy1=-dy1; }
      if(y<0) { y=-y; dy1=-dy1; }
      sax1[k]=x; say1[k]=y;
      x = sax2[j]+dx2; 
      y = say2[j]+dy2; 
      if(x>=sc.sizeX) { x=2*sc.sizeX-x-1; dx2=-dx2; }
      if(x<0) { x=-x; dx2=-dx2; }
      if(y>=sc.sizeY) { y=2*sc.sizeY-y-1; dy2=-dy2; }
      if(y<0) { y=-y; dy2=-dy2; }
      sax2[k]=x; say2[k]=y;
      dpDrawLine(&sc,&gc,sax1[k],say1[k],sax2[k],say2[k]);
   }

   printf("End of test1L. Press RETURN\n");
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

