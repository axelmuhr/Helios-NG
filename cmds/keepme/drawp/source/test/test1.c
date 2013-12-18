/*-----------------------------------------------------------*/
/*                                      source/test/test1.c  */
/*-----------------------------------------------------------*/

/* First test file for the new drawing package. This file will */
/*   simply plot a rectangle using fill solid on the screen    */

/*-----------------------------------------------------------*/
/*                                            Include files  */
/*-----------------------------------------------------------*/

#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include "drawp/public.h"
#include "masks.h"
#include "drawp/debug.h"

/*-----------------------------------------------------------*/
/*                                                main(...)  */
/*-----------------------------------------------------------*/

int submain()
{  DpGraphicsContext_t gc;
   DpPixmap_t          pm;
   int                 lx,rx,by,ty;
   
   if(dpMapScreen(&pm,2)==NULL)
   {  printf("Cannot map screen\n");
      return 1;
   }

   printf("Mapped Screen\n");
   printf("Pixmap base address minus 4 is %X\n",(int)pm.rawBase);

   setMask(&pm,5,5);
   
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
   gc.clipXOrigin = -5;
   gc.clipYOrigin = -5;
   gc.clipMask    = msk;

/*    dpDebug = 0; */
   dpUnClippedRect(&gc,&pm,0,0,pm.sizeX,pm.sizeY);

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

   lx =   pm.sizeX/4;
   rx = 3*pm.sizeX/4;
   ty =   pm.sizeY/4;
   by = 3*pm.sizeY/4;

   printf("Pixmap base address minus 4 is %X\n",(int)pm.rawBase);

   dpDebug = 1;
   dpUnClippedRect(&gc,&pm,lx,ty,rx,by);
   
   printf("Operation completed sucessfully\n");
   while(getchar()!='\n');
   return 0;
}

