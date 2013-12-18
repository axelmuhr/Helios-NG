/*-----------------------------------------------------------*/
/*                                      source/test/test3.c  */
/*-----------------------------------------------------------*/

/* First test file for the new drawing package. This file will */
/*   simply plot a rectangle using fill solid on the screen    */

/*-----------------------------------------------------------*/
/*                                            Include files  */
/*-----------------------------------------------------------*/

#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <math.h>
#include "drawp/public.h"
#include "masks.h"
#include "drawp/debug.h"

/*-----------------------------------------------------------*/
/*                                                main(...)  */
/*-----------------------------------------------------------*/

int submain()
{  DpGraphicsContext_t gc;
   DpPixmap_t          pm,sc;
   int                 cx,cy,x,y,r,c;
   
   printf("This simple test draws filled circles concentrically\n");
   printf("  in alternating colours starting with colour 0 in an\n");
   printf("  off-screen pixmap, and then copies that pixmap onto\n");
   printf("  the screen using a simple GXcopy tiled operation\n");
   printf("\nPress RETURN to start test ...\n");
   while(getchar()!='\n');

   dpDebug = 0;
   
   if(dpMapScreen(&sc,2)==NULL)
   {  printf("Cannot map screen\n");
      return 1;
   }

   setMask(&sc,5,5);
   
   if(dpConstructPixmap(&pm,sc.sizeX,sc.sizeY,1)==NULL)
   {  printf("Cannot create off-screen pixmap\n"); }
   
   r = sc.sizeX/2;
   if(sc.sizeY/2<r) r = sc.sizeY/2;
   cx=sc.sizeX/2; cy=sc.sizeY/2;
   
   gc.depth       = 1;
   gc.function    = GXcopy;
   gc.planeMask   = 0xFFFFFFFF;
   gc.foreground  = 1;
   gc.background  = 1;
   gc.fillStyle   = FillSolid;
   gc.tile        = NULL;
   gc.stipple     = NULL;
   gc.tsXOrigin   = 0;
   gc.tsYOrigin   = 0;
   gc.clipXOrigin = 0;
   gc.clipYOrigin = 0;
   gc.clipMask    = NULL;
   
   dpUnClippedRect(&gc,&pm,0,0,sc.sizeX,sc.sizeY);

   for(c=0;r>10;r-=20,c^=1)
   {  for(y=-r; y<=r; y++)
      {  x = r*r - y*y;
         x = (int)sqrt((double)x);
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
   
         dpUnClippedRect(&gc,&pm,cx-x,cy+y,cx+x,cy+y+1);
      }
   }

   printf("Circle drawn in pixmap, about to copy to screen ...\n");
   fflush(stdout);
   
   dpDebug = 1;
   
   gc.depth       = 1;
   gc.function    = GXcopy;
   gc.planeMask   = 0xFFFFFFFF;
   gc.foreground  = 0;
   gc.background  = 0;
   gc.fillStyle   = FillTiled;
   gc.tile        = &pm;
   gc.stipple     = NULL;
   gc.tsXOrigin   = 0;
   gc.tsYOrigin   = 0;
   gc.clipXOrigin = -5;
   gc.clipYOrigin = -5;
   gc.clipMask    = msk;
   
   dpUnClippedRect(&gc,&sc,0,0,sc.sizeX,sc.sizeY);

   printf("Operation completed sucessfully\n");
   while(getchar()!='\n');
   return 0;
}
