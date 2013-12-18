/*--------------------------------------------------------------------*/
/*                                                source/construct.c  */
/*--------------------------------------------------------------------*/

/* This file contains routines to set-up and destroy pix-map          */
/*    structures. It can set up a pixmap structure corresponding to   */
/*    an in-memory pix-map, or a pix-map structure corresponding to   */
/*    the screen itself.                                              */

/* Synopsis:                                                          */
/* DpPixmap_t *dpConstructPixmap(DpPixmap_t*,int,int,int);            */
/* DpPixmap_t *dpMapScreen      (DpPixmap_t*,int);                    */
/* void       *dpDestructPixmap (DpPixmap_t*,int);                    */

/*--------------------------------------------------------------------*/
/*                                                     Include files  */
/*--------------------------------------------------------------------*/

#include <stdlib.h>
#include <stddef.h>
#include "drawp/pixmaps.h"
#include "private.h"

#ifndef HeliosMode
#include    "sys/types.h"
#include    "sys/ioctl.h"
#include    "sys/fbioctl.h"
#include    "sys/vcioctl.h"
#include    "sys/photron.h"
#endif

/*--------------------------------------------------------------------*/
/*                                       Further system declarations  */
/*--------------------------------------------------------------------*/

/* These are potentially missing in the header files:                */

extern int ioctl(int,unsigned long,void*);

/* NOTE: fileno is not defined in ANSI headers */
# ifndef HeliosMode
#    ifndef fileno
#       define fileno(p) ((p)->_file)
#    endif
# endif

/*--------------------------------------------------------------------*/
/*                                            dpConstructPixmap(...)  */
/*--------------------------------------------------------------------*/

DpPixmap_t *dpConstructPixmap(DpPixmap_t *pm,
                              int         sizeX,
                              int         sizeY,
                              int         depth)
/* This routine will allocate raw data for an in-memory pixmap of the  */
/*   parameters supplid to this routine, and set the pix-map structure */
/*   pointed to by <pm> to point into this pix-map. It will return <pm>*/
/*   if sucessful, and NULL otherwise. If <pm> is passed as NULL, then */
/*   a Pixmap structure will first be allocated, and the pointer to    */
/*   that structure will be returned if the routine is sucessful       */
{  int wpv;
   int pma;

   if(sizeX<=0||sizeY<=0)        return NULL;
   if(depth<=0||depth>=DpMaxBpp) return NULL;
   if(pm==NULL)
   {  pm = (DpPixmap_t*) calloc(1,sizeof(DpPixmap_t));
      if(pm==NULL) return NULL;
      pma = 1; /* Set flag   */
   }
   else
      pma = 0; /* Clear flag */
   
   wpv = (sizeX+31)/32;
   
   pm->scrFd   = NULL;
   pm->depth   = depth;
   pm->sizeX   = sizeX;
   pm->sizeY   = sizeY;
   pm->wpv     = wpv;
   pm->visRgn  = DpDefaultClipping;
   
   pm->rawBase = calloc( sizeY*depth*wpv+2 , 4 );

   if(pm->rawBase==NULL)
   {  if(pma) free(pm);
      return NULL;
   }
   return pm;
}

/*--------------------------------------------------------------------*/
/*                                                  dpMapScreen(...)  */
/*--------------------------------------------------------------------*/

DpPixmap_t *dpMapScreen(DpPixmap_t *pm,
                        int         subtype)
/* If <pm> is NULL, a DpPixmap_t structure is allocated, mapped to    */
/*    the screen, and a pointer to the Pixmap is returned if sucessful*/
/*    or NULL is returned otherwise. If <pm> is not NULL then it is   */
/*    used as the pixmap to map to the screen.                        */
/* <subtype> is the sub-type number of a RISCiX frame buffer          */
/*    configuration to use for the screeen, the screen dimentions are */
/*    read from the operating system. One bit per pixel is always     */
/*    assumed. If <subtype> is -1, then an appropriate sub-type is    */
/*    chosen to drive the photron tablet.                             */
{  
#ifdef HeliosMode
   subtype = 0; /* Ignore this parameter */
   if(pm==NULL)
   {  pm = (DpPixmap_t*) calloc(1,sizeof(DpPixmap_t));
      if(pm==NULL) return NULL;
   }
   pm->scrFd   = (void*)1;
   pm->depth   = 1;
   pm->sizeX   = 640;
   pm->sizeY   = 398;
   pm->wpv     = 64;
   pm->rawBase = (void*)(0x740000+64*4-4);
   pm->visRgn  = DpDefaultClipping;
   return pm;
#else
   int    sizeX,sizeY,wpv,pma,uph,res;
   FBInfo infoBuf;
   FILE  *phfd;
   
   if(pm==NULL)
   {  pm = (DpPixmap_t*) calloc(1,sizeof(DpPixmap_t));
      if(pm==NULL) return NULL;
      pma = 1; /* Set flag   */
   }
   else
      pma = 0; /* Clear flag */

   /* Trap photron selection */
   if(subtype==-1) uph = 1; else uph = 0;

   /* Try to open the frame buffer: */
   pm->scrFd = fopen("/dev/fb","w"); 
   if(pm->scrFd==NULL) goto Fail;
   
   res = ioctl(fileno(pm->scrFd),FBIOGCURINFO,&infoBuf);
   if(res<0) goto Fail;

   /* If using the photron tablet, find a suitable sub-type which */
   /*    can be re-configured for the Photron ...                 */
   if(uph)
      for(subtype=0;subtype<infoBuf.subtypes;subtype++)
      {  infoBuf.subtype = subtype;
         res = ioctl(fileno(pm->scrFd),FBIOGINFO,&infoBuf);
         if(res<0) goto Fail;
         if( infoBuf.bitmap->info.x_pixels>=640
          && infoBuf.bitmap->info.y_pixels>=400
          && infoBuf.bitmap->info.bits_per_pixel==1
         ) break;
      }
   
   /* Validate <subtype> */
   if( subtype<0 || subtype>=infoBuf.subtypes ) goto Fail;

   res = ioctl(fileno(pm->scrFd),FBIOSSUBTYPE,&subtype);
   if(res<0) goto Fail;

   /* Get info on chosen sub-type ... */
   res = ioctl(fileno(pm->scrFd),FBIOGCURINFO,&infoBuf);
   if(res<0) goto Fail;
   
   sizeX = infoBuf.bitmap->info.x_pixels;
   sizeY = infoBuf.bitmap->info.y_pixels;
   
   /* If using photron, open Photron device and override information */
   if(uph)
   {  phfd = fopen("/dev/photron","w");
      if(phfd==NULL) goto Fail;
      res = ioctl(fileno(phfd),PHOTRONVIDC,NULL);
      if(res<0) goto Fail;
      sizeX = 640;
      sizeY = 400;
   }

   wpv = (sizeX+31)/32;

   pm->depth   = 1;
   pm->sizeX   = sizeX;
   pm->sizeY   = sizeY;
   pm->wpv     = wpv;
   pm->rawBase = (char*)infoBuf.bitmap->base_address - 4;
   pm->visRgn  = DpDefaultClipping;

   return pm;
   
   Fail:
   if(pm->scrFd!=NULL) fclose(pm->scrFd);
   if(pma)             free(pm);
   return NULL;
#endif
}

/*--------------------------------------------------------------------*/
/*                                             dpDestructPixmap(...)  */
/*--------------------------------------------------------------------*/

void dpDestructPixmap(DpPixmap_t *pm,int pma)
/* This function will destruct the pix-map pointed to by pm, whether  */
/*    it is a memory pixmap or the screen pixmap, and if <pma> is non */
/*    zero, it will also free the pixmap structure <pm> as well       */
{  
#ifdef HeliosMode
   if(pm->scrFd==NULL) free(pm->rawBase);
#else
   if(pm->scrFd)
      fclose(pm->scrFd);
   else
      free(pm->rawBase);
#endif
   if(pma) free(pm);
}


/*--------------------------------------------------------------------*/
/*                                                dpConstructGC(...)  */
/*--------------------------------------------------------------------*/

DpGraphicsContext_t *dpConstructGC(DpGraphicsContext_t *dpGc, int depth)
/* This function will initialize a graphics context to default values  */
/*    the graphics context initialized will be either dpGc or, if that */
/*    is NULL, it will be one allocated using 'calloc' the pointer to  */
/*    whichever Gc it is will be returned, or else NULL will be        */
/*    returned if memory could not be allocated.                       */
{
   if (dpGc==NULL)
   {   dpGc = (DpGraphicsContext_t*)calloc(1,sizeof(DpGraphicsContext_t));
       if(dpGc==NULL) return NULL;
   }
      
   dpGc->depth         = depth;
   dpGc->function      = GXcopy;
   dpGc->planeMask     = ~0;
   dpGc->fillStyle     = FillSolid;
   dpGc->foreground    = 0;
   dpGc->background    = 1;
   dpGc->tile          = NULL;
   dpGc->stipple       = NULL;
   dpGc->tsXOrigin     = 0;
   dpGc->tsYOrigin     = 0;
   dpGc->clipMask      = NULL;
   dpGc->clipXOrigin   = 0;
   dpGc->clipYOrigin   = 0;

   dpGc->lineWidth     = 0;
   dpGc->lineStyle     = LineSolid;
   dpGc->capStyle      = CapButt;
   dpGc->joinStyle     = JoinMiter;
   dpGc->fillRule      = EvenOddRule;
   dpGc->dashOffset    = 0;
   dpGc->dashLength    = 0;
   dpGc->dashList      = NULL;

   return dpGc;
}
