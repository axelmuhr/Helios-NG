/*-----------------------------------------------------------*/
/*                                      source/test/test4.c  */
/*-----------------------------------------------------------*/

/* Test file for FillTiled: simple test, just testing combination */

/*-----------------------------------------------------------*/
/*                                            Include files  */
/*-----------------------------------------------------------*/

#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include "drawp/public.h"
#include "masks.h"
#include "drawp/debug.h"
#include "sys/time.h" /* Local versions of non-ANSI file */
int gettimeofday(struct timeval*,struct timezone*);
DpPixmap_t sc;

/*-----------------------------------------------------------*/
/*                                                main(...)  */
/*-----------------------------------------------------------*/

void doChequer(DpPixmap_t*,int,int,int,int,DpPixmap_t*,DpPixmap_t*);
void serviceQuery(DpPixmap_t*,int,int,DpPixmap_t*,DpPixmap_t*);
void clearArea(DpPixmap_t*,int,int,int,int,int);
int submain()
{  DpPixmap_t  c0,c1;
   int         sz,dir,intact,ch;
   int         query,gblQuery;
   
   dpDebug=0;

   printf("This test is the same as the test in out/test2,  \n");
   printf("  except that, instead of using FillSolid, with a\n");
   printf("  foreground colour, we use FillTiled with a     \n");
   printf("  source pixmap containing all that colour.      \n");

   printf("\nPress RETURN for a full description ...\n");
   while(getchar()!='\n');
   
   printf("This test program should display a succession of \n");
   printf("  black-and-white chequer-boards on the screen,  \n");
   printf("  occupying the whole screen, with the top left  \n");
   printf("  corner having colour 0. The first chequer board\n");
   printf("  should be 8x8 and they get successively smaller\n");
   printf("  and smaller until only a dither is displayed.  \n");
   printf("In addition, the chequer boards will be draw in  \n");
   printf("  different directions each time so that messy   \n");
   printf("  operations in the various deirections can be   \n");
   printf("  detected.                                      \n");
   printf("The code cycles through the 'X' type graphics    \n");
   printf("  functions in order to test them all            \n");
   printf("There will be a one second interval between each \n");
   printf("  frame, unless you type 'i' before pressing     \n");
   printf("  RETURN in which case there will be a pause for \n");
   printf("  you to press RETURN between each frame         \n");
   printf("If you also enter 'q' before pressing RETURN then\n");
   printf("  the plot mode for every rectangle will be      \n");
   printf("  printed                                        \n");
   printf("If you are in interactive mode, and you press    \n");
   printf("  'q' before RETURN after a screen, then you will\n");
   printf("  be prompted for the (x,y) co-ordinate from the \n");
   printf("  top-left of a square, and the function number  \n");
   printf("  for it will be displayed. (0,0) is top-left    \n");

   intact=0;
   gblQuery=0;
   printf("\nPress RETURN:\n");
   while((ch=getchar())!='\n') 
   {  if(ch=='i') intact=1;
      if(ch=='q') gblQuery=1;
   }
   
   if(dpMapScreen(&sc,2)==NULL)
   {  printf("Cannot map screen\n");
      return 1;
   }
   
   setMask(&sc,5,5);
   
   if(dpConstructPixmap(&c0,sc.sizeX,sc.sizeY,1)==NULL)
   {  printf("Cannot construct colour zero pixmap\n");
      return 1;
   }
   
   if(dpConstructPixmap(&c1,sc.sizeX,sc.sizeY,1)==NULL)
   {  printf("Cannot construct colour one pixmap\n");
      return 1;
   }
   
   clearArea(&c0,0,0,sc.sizeX,sc.sizeY,0);
   clearArea(&c1,0,0,sc.sizeX,sc.sizeY,1);
   
   sz=8;
   dir=0;
   while(sc.sizeX/sz>0 && sc.sizeY/sz>0)
   {  doChequer(&sc,sz,dir,-1,gblQuery?0:-1,&c0,&c1);
      if(intact)
      {  printf("... Board displayed, press RETURN:\n");
         query=0;
         while((ch=getchar())!='\n') if(ch=='q') query=1;
         if(query)
         {  serviceQuery(&sc,sz,dir,&c0,&c1);  }
      }
      else waitSecond();
      sz = (sz*6)/5;
      dir = (dir+1)&0x03;
   }
   
   printf("Test program has completed\n");
   printf("Hit RETURN:\n");
   while(getchar()!='\n');
   return 0;
}

/*---------------------------------------------------------------*/
/*                                            serviceQuery(...)  */
/*---------------------------------------------------------------*/

void serviceQuery(DpPixmap_t *pm,int sz,int dir,DpPixmap_t *c0,DpPixmap_t *c1)
{  int qx,qy,ch,cnt;
   char str[100];
   
   printf("Query rectangle column X (0=> leftmost) : ");
   cnt=0;
   while((ch=getchar())!='\n')
      if(cnt<99) str[cnt++]=ch;
   str[cnt]=0;
   qx = atoi(str);
   printf("Query rectangle row    Y (0=>uppermost) : ");
   cnt=0;
   while((ch=getchar())!='\n')
      if(cnt<99) str[cnt++]=ch;
   str[cnt]=0;
   qy = atoi(str);
   printf("\n");
   printf("Query on rectangle (%d,%d) ... \n",qx,qy);
   doChequer(pm,sz,dir,qx,qy,c0,c1);
   printf("Query complete, board re-displayed. Press RETURN\n");
   while(getchar()!='\n');
}

/*---------------------------------------------------------------*/
/*                                               doChequer(...)  */
/*---------------------------------------------------------------*/

void chequerComponent(DpPixmap_t*,int*,int*,int*,int*,int*,int*,
                      int,int,int,int,int,DpPixmap_t *c0,DpPixmap_t *c1);
void doRect(DpPixmap_t*,int,int,int,int,int,int,int,DpPixmap_t*,DpPixmap_t*);

void doChequer(DpPixmap_t *pm,
               int sz,int dir,int qx,int qy,
               DpPixmap_t*c0,DpPixmap_t*c1)
{  int fn0,fn1;
   int fc0,fc1;
   int bc0,bc1;
   int x,y;

   /* Clear the screen: */
   clearArea(pm,0,0,pm->sizeX,pm->sizeY,0);

   fn0 = fn1 = fc0 = fc1 = bc0 = bc1 = 0;
   
   switch(dir)
   {   case 0:
          for(x=0;x<sz;x++) for(y=0;y<sz;y++)
             chequerComponent(pm,&fn0,&fn1,&fc0,&fc1,&bc0,&bc1,x,y,
                              sz,qx,qy,c0,c1);
          break;
       case 1:
          for(x=sz-1;x>=0;x--) for(y=0;y<sz;y++)
             chequerComponent(pm,&fn0,&fn1,&fc0,&fc1,&bc0,&bc1,x,y,
                              sz,qx,qy,c0,c1);
          break;
       case 2:
          for(y=0;y<sz;y++) for(x=0;x<sz;x++)
             chequerComponent(pm,&fn0,&fn1,&fc0,&fc1,&bc0,&bc1,x,y,
                              sz,qx,qy,c0,c1);
          break;
       case 3:
          for(y=sz-1;y>=0;y--) for(x=0;x<sz;x++)
             chequerComponent(pm,&fn0,&fn1,&fc0,&fc1,&bc0,&bc1,x,y,
                              sz,qx,qy,c0,c1);
          break;
   }
}
            
/*---------------------------------------------------------------*/
/*                                        chequerComponent(...)  */
/*---------------------------------------------------------------*/

void chequerComponent(DpPixmap_t *pm,
                      int *fn0,int *fn1,
                      int *fc0,int *fc1,
                      int *bc0,int *bc1,
                      int x,int y,
                      int sz,
                      int qx,int qy,
                      DpPixmap_t *c0,DpPixmap_t *c1)
{  int bit;

   if(x>0&&y>5) dpDebug=1;
   if((x^y)&1)
   {  /* Try to get a colour one */
      do
      {  bit = 3 - ((*fc1<<1)|*bc1);
         if((*fn1>>bit)&1) break;
         if(*bc1==0) { *bc1=1; continue; } else *bc1=0;
         if(*fc1==0) { *fc1=1; continue; } else *fc1=0;
         *fn1 = (*fn1+1)&0x0F;
      } while(1);
      doRect(pm,(x*pm->sizeX)/sz,
                (y*pm->sizeY)/sz,
                ((x+1)*pm->sizeX)/sz,
                ((y+1)*pm->sizeY)/sz,
                *fn1,*fc1,*bc1,c0,c1);
      if((qx==x&&qy==y)||(qx==-1&&qy==0))
      {  printf("Rectangle at (%d,%d) should be colour 1\n",
                x,y);
         printf("Drawing solid %d over %d in function 0x%X\n\n",
                 *fc1,*bc1,*fn1);
         fflush(stdout);
      }
      if(*bc1==0) { *bc1=1; return; } else *bc1=0;
      if(*fc1==0) { *fc1=1; return; } else *fc1=0;
      *fn1 = (*fn1+1)&0x0F; return;
    }
    else
    {  /* Try to get a colour zero */
       do
       {  bit = 3 - ((*fc0<<1)|*bc0);
          if(!((*fn0>>bit)&1)) break;
          if(*bc0==0) { *bc0=1; continue; } else *bc0=0;
          if(*fc0==0) { *fc0=1; continue; } else *fc0=0;
          *fn0 = (*fn0+1)&0x0F;
       } while(1);
       if((qx==x&&qy==y)||(qx==-1&&qy==0))
       {  printf("Rectangle at (%d,%d) should be colour 0\n",
                 x,y);
          printf("Drawing solid %d over %d in function 0x%X\n\n",
                  *fc0,*bc0,*fn0);
          fflush(stdout);
       }
       doRect(pm,(x*pm->sizeX)/sz,
                 (y*pm->sizeY)/sz,
                 ((x+1)*pm->sizeX)/sz,
                 ((y+1)*pm->sizeY)/sz,
                 *fn0,*fc0,*bc0,c0,c1);
      if(*bc0==0) { *bc0=1; return; } else *bc0=0;
      if(*fc0==0) { *fc0=1; return; } else *fc0=0;
      *fn0 = (*fn0+1)&0x0F; return;
    }
    
}
                      
/*---------------------------------------------------------------*/
/*                                                  doRect(...)  */
/*---------------------------------------------------------------*/

void doRect(DpPixmap_t *pm,
            int lx,int ty,int rx,int by,
            int funct,
            int fore,int back,
            DpPixmap_t *c0,DpPixmap_t *c1)
{  DpGraphicsContext_t gc;

   /* Set backgound over which to draw using a fairly reliable */
   /*   fill mode:                                             */
   gc.depth       = 1;
   gc.function    = GXcopy;
   gc.planeMask   = 0xFFFFFFFF;
   gc.foreground  = back;
   gc.background  = 0;
   gc.fillStyle   = FillSolid;
   gc.tile        = NULL;
   gc.stipple     = NULL;
   gc.tsXOrigin   = 0;
   gc.tsYOrigin   = 0;
   gc.clipXOrigin = -5;
   gc.clipYOrigin = -5;
   gc.clipMask    = msk;
   dpUnClippedRect(&gc,pm,lx,ty,rx,by);

   /* Draw the rectangle, by tiled op from appropriate pixmap */
   gc.depth       = 1;
   gc.function    = funct;
   gc.planeMask   = 0xFFFFFFFF;
   gc.foreground  = 0;
   gc.background  = 0;
   gc.fillStyle   = FillTiled;
   gc.tile        = fore?c1:c0;
   gc.stipple     = NULL;
   gc.tsXOrigin   = 0;
   gc.tsYOrigin   = 0;
   gc.clipXOrigin = -5;
   gc.clipYOrigin = -5;
   gc.clipMask    = msk;
   dpDebug=1;
   dpUnClippedRect(&gc,pm,lx,ty,rx,by);

   fflush(stdout);
      
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
