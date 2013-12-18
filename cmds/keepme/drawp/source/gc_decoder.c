/*------------------------------------------------------------*/
/*                                       source/gc_decoder.c  */
/*------------------------------------------------------------*/

/* $Header: gc_decoder.c,v 1.15 90/07/01 17:28:26 charles Locked $ */
/* $Source: /server/usr/users/a/charles/world/drawp/RCS/source/gc_decoder.c,v $ */

/* This file contains code which decodes the relevalnt information */
/*   in an 'X' type graphics context into a BlitterControlBlock_t, */
/*   in one of various ways so that the appropriate graphics       */
/*   primitive or copy operation can be executed                   */

/* There are also routines which decode graphics contexts in the   */
/*   file 'lineControl.c'                                          */

/*------------------------------------------------------------*/
/*                                             Include files  */
/*------------------------------------------------------------*/

#include "drawp/graphicsContexts.h"
#include "code_interface.h"
#include "private.h"

/*------------------------------------------------------------*/
/*                              setCtlAndFnWithCol() (macro)  */
/*------------------------------------------------------------*/

/* This large macro to be used in routines below to fill up the    */
/*     combination functions vector and the control routine vector */
/*     appropriately for a colour orientated plot.                 */
/* To be used like a function                                      */

#define setCtlAndFnWithCol(bcb,gc,useFgd,zerFn,oneFn,ctlEntry,cmbGroup) \
{  int           plane, cmbEntry, depth;                                \
   unsigned long planeMask, col;                                        \
                                                                        \
   bcb->ctlRtn     = getControlAddress(ctlEntry);                       \
   planeMask       = gc->planeMask;                                     \
   if(useFgd)  col = gc->foreground;                                    \
   else        col = gc->background;                                    \
   depth           = gc->depth;                                         \
                                                                        \
   for(plane=0;plane<depth;plane++)                                     \
   {  if(planeMask&1)                                                   \
      {  if(col&1) cmbEntry=(oneFn); else cmbEntry=(zerFn);  }          \
      else                                                              \
      {  cmbEntry = GXnoop | (cmbGroup);                     }          \
      bcb->combMode[plane] = getCombineAddress(cmbEntry);               \
      planeMask>>=1;  col>>=1;                                          \
   }                                                                    \
}

/*------------------------------------------------------------*/
/*                              setCtlAndFnWithCol() (macro)  */
/*------------------------------------------------------------*/

/* This large macro to be used in routines below to fill up the    */
/*     combination functions vector and the control routine vector */
/*     appropriately for a copy orientated plot.                   */
/* To be used like a function                                      */

#define setCtlAndFnNoCol(bcb,gc,function,ctlEntry,cmbGroup)             \
{  int           plane, cmbEntry, depth;                                \
   unsigned long planeMask;                                             \
                                                                        \
   bcb->ctlRtn     = getControlAddress(ctlEntry);                       \
   planeMask       = gc->planeMask;                                     \
   depth           = gc->depth;                                         \
                                                                        \
   for(plane=0;plane<depth;plane++)                                     \
   {  if(planeMask&1) cmbEntry = (function) | (cmbGroup);               \
      else            cmbEntry = GXnoop     | (cmbGroup);               \
      bcb->combMode[plane] = getCombineAddress(cmbEntry);               \
      planeMask>>=1;                                                    \
   }                                                                    \
}

/*------------------------------------------------------------*/
/*                                  setDesData(...)  (macro)  */
/*------------------------------------------------------------*/

#define setDesData(bcb,des)           \
bcb->desBPP  = des->depth;            \
bcb->desBase = (char*)des->rawBase+4; \
bcb->desWPV  = des->wpv;

/*------------------------------------------------------------*/
/*                 Combination and Control Rouitne Addresses  */
/*------------------------------------------------------------*/

/* These macros are used to look-up the addresses of the       */
/*   control and combination routine macros, to make the code  */
/*   relocatable under the Helios linking concepts, the        */
/*   addresses stored in the look-up table are relative to the */
/*   start of the table, hence the start of the table is added */
/*   to the values looked up to get the proper address.        */

#define getCombineAddress(entry) dpLookUpCombineAddress(entry)
/* (void*)(dpCombineLookUp[entry] + (char*)dpCombineLookUp) */

/* Because of casting restrictions we actually have to call   */
/*    some assembly code simply to do the same as the above   */
/*    and get a pointer to a function.                        */
#define getControlAddress(entry) dpLookUpControlAddress(entry)

/*------------------------------------------------------------*/
/*                                           Local functions  */
/*------------------------------------------------------------*/

/* This file contains the following local static functions:   */

static void solidFill
            (BlitterControlBlock_t*,DpGraphicsContext_t*,int);
static void solidThroughMask
            (BlitterControlBlock_t*,DpGraphicsContext_t*,int);
static void solidNoMask
            (BlitterControlBlock_t*,DpGraphicsContext_t*,int);
static void stippledTrans
            (BlitterControlBlock_t*,DpGraphicsContext_t*,int);
static void stippledOpaque
            (BlitterControlBlock_t*,DpGraphicsContext_t*,int);
static void tiledSource
            (BlitterControlBlock_t*,DpGraphicsContext_t*,int);
static void setSrcAndMskData 
            (  BlitterControlBlock_t*,
               DpPixmap_t*,int,int,
               DpPixmap_t*,int,int,
               int
            );

/*------------------------------------------------------------*/
/*                                 cbDecodeForPrimitive(...)  */
/*------------------------------------------------------------*/

void dpDecodeGcToBcb(BlitterControlBlock_t *bcb,
                     DpGraphicsContext_t   *gc,
                     int                    useFgd)
/* This routine decodes a graphics context for subsequent use */
/*   drawing primitives                                       */
{  

   switch(gc->fillStyle)
   {  case FillSolid:          solidFill      (bcb,gc,useFgd); break;
      case FillOpaqueStippled: stippledOpaque (bcb,gc,useFgd); break;
      case FillStippled:       stippledTrans  (bcb,gc,useFgd); break;
      case FillTiled:          tiledSource    (bcb,gc,useFgd); break;
      default: bcb->ctlRtn = NULL;
   }
   return;
}

/*------------------------------------------------------------*/
/*                                            solidFill(...)  */
/*------------------------------------------------------------*/

static void solidFill(BlitterControlBlock_t *bcb,
                      DpGraphicsContext_t   *gc,
                      int                    useFgd)
{  if(gc->clipMask)
      solidThroughMask(bcb,gc,useFgd);
   else
      solidNoMask(bcb,gc,useFgd);
}

/*------------------------------------------------------------*/
/*                                     solidThroughMask(...)  */
/*------------------------------------------------------------*/

static void solidThroughMask(BlitterControlBlock_t *bcb,
                             DpGraphicsContext_t   *gc,
                             int                    useFgd)
{  int             function,zerFn,oneFn;
   
   function  = gc->function & 0x0F;
   zerFn = 0x04 | (function>>2)  | CmbLeftRight | CmbUnMasked;
   oneFn = 0x04 | (function&0x3) | CmbLeftRight | CmbUnMasked;

   setCtlAndFnWithCol
   (  bcb, gc, useFgd, zerFn, oneFn,
      CtlBinary | CtlStippled | CtlLeftRight | CtlTopBottom,
      CmbUnMasked | CmbLeftRight
   );

   /* In this mode of plotting only, the source takes the place  */
   /*   of the mask, and there is no mask pixmap.                */
   setSrcAndMskData
   ( bcb,gc->clipMask,gc->clipXOrigin,gc->clipYOrigin,NULL,0,0,1 );
   
   bcb->desBPP = gc->depth;
}

/*------------------------------------------------------------*/
/*                                          solidNoMask(...)  */
/*------------------------------------------------------------*/

static void solidNoMask(BlitterControlBlock_t *bcb,
                        DpGraphicsContext_t   *gc,
                        int                    useFgd)
{  int             function,zerFn,oneFn;

   function  = gc->function & 0x0F;
   zerFn     = (function>>2) | (function&0xC) | CmbLeftRight | CmbUnMasked;
   oneFn     = (function&0x3);
   oneFn     = (oneFn | oneFn<<2)             | CmbLeftRight | CmbUnMasked;

   setCtlAndFnWithCol
   (  bcb, gc, useFgd, zerFn, oneFn,
      CtlUnary, CmbUnMasked | CmbLeftRight
   );

   bcb->desBPP = gc->depth;
}

/*------------------------------------------------------------*/
/*                                        stippledTrans(...)  */
/*------------------------------------------------------------*/

static void stippledTrans(BlitterControlBlock_t *bcb,
                          DpGraphicsContext_t   *gc,
                          int                    useFgd)
{  int             cmbGroup,ctlEntry,function,zerFn,oneFn;

   if(gc->clipMask) ctlEntry = CtlTertiary, cmbGroup = CmbMasked;
   else             ctlEntry = CtlBinary,   cmbGroup = CmbUnMasked;
   ctlEntry |= CtlStippled | CtlLeftRight | CtlTopBottom;
   cmbGroup |= CmbLeftRight;

   function  = gc->function & 0x0F;
   zerFn     = 0x04 | (function>>2)  | CmbLeftRight | cmbGroup;
   oneFn     = 0x04 | (function&0x3) | CmbLeftRight | cmbGroup;

   setCtlAndFnWithCol
   (  bcb, gc, useFgd, zerFn, oneFn, ctlEntry, cmbGroup );

   setSrcAndMskData
   (  bcb,gc->stipple,gc->tsXOrigin,gc->tsYOrigin,
      gc->clipMask,gc->clipXOrigin,gc->clipYOrigin,
      1
   );

   bcb->desBPP = gc->depth;
}

/*------------------------------------------------------------*/
/*                                       stippledOpaque(...)  */
/*------------------------------------------------------------*/

static void stippledOpaque(BlitterControlBlock_t *bcb,
                           DpGraphicsContext_t   *gc,
                           int                    useFgd)
{  int             cmbGroup,cmbEntry,ctlEntry,function,plane;
   unsigned long   colF,colB,planeMask;
   
   useFgd = 0; /* Ignore this parameter */

   if(gc->clipMask) ctlEntry = CtlTertiary, cmbGroup = CmbMasked;
   else             ctlEntry = CtlBinary,   cmbGroup = CmbUnMasked;
   ctlEntry |= CtlStippled | CtlLeftRight | CtlTopBottom;
   cmbGroup |= CmbLeftRight;
   bcb->ctlRtn = getControlAddress(ctlEntry);
   
   colF      = gc->foreground;
   colB      = gc->background;
   planeMask = gc->planeMask;
   function  = gc->function & 0x0F;

   /* Special code required to set up the function combination mode    */
   /*      vector ...                                                  */

   for(plane=0;plane<gc->depth;plane++)
   {  if(planeMask&1)
      {  if(colF&1) cmbEntry  = (function&0x3); 
         else       cmbEntry  = (function&0xC)>>2;
         if(colB&1) cmbEntry |= (function&0x3)<<2;
         else       cmbEntry |= (function&0xC);
         cmbEntry |= cmbGroup;
      } else cmbEntry = GXnoop | cmbGroup | CmbLeftRight;

      bcb->combMode[plane] = getCombineAddress(cmbEntry);
      planeMask>>=1; colF>>=1; colB>>=1;
   }
   
   setSrcAndMskData
   (  bcb,gc->stipple,gc->tsXOrigin,gc->tsYOrigin,
      gc->clipMask,gc->clipXOrigin,gc->clipYOrigin,
      1
   );

   bcb->desBPP = gc->depth;
}

/*------------------------------------------------------------*/
/*                                          tiledSource(...)  */
/*------------------------------------------------------------*/

static void tiledSource(BlitterControlBlock_t *bcb,
                        DpGraphicsContext_t   *gc,
                        int                    useFgd)
{  int             cmbGroup,ctlEntry,function;
   
   useFgd = 0;  /* Ignore this parameter */

   if(gc->clipMask) ctlEntry = CtlTertiary, cmbGroup = CmbMasked;
   else             ctlEntry = CtlBinary,   cmbGroup = CmbUnMasked;
   ctlEntry |= CtlUnStippled | CtlLeftRight | CtlTopBottom;
   cmbGroup |= CmbLeftRight;
   function  = gc->function & 0x0F;

   setCtlAndFnNoCol
   (  bcb, gc, function, ctlEntry, cmbGroup );

   setSrcAndMskData
   (  bcb,gc->tile,gc->tsXOrigin,gc->tsYOrigin,
      gc->clipMask,gc->clipXOrigin,gc->clipYOrigin,
      0
   );

   bcb->desBPP = gc->depth;
}

/*------------------------------------------------------------*/
/*                                     dpDecodeCopyArea(...)  */
/*------------------------------------------------------------*/

void dpDecodeCopyArea(BlitterControlBlock_t *bcb,
                      DpPixmap_t *src, DpPixmap_t *des,
                      int srcOffX, int srcOffY,
                      DpGraphicsContext_t *gc)
{  int depth,plane,function;
   int ctlEntry,cmbGroup;
   int vertBt,offst;
   unsigned long planeMask;
   
   /* In this operation, take account of the 'shift' between source */
   /*    and destination in case in same pixmap                     */
   
   ctlEntry = CtlUnStippled;
   if(gc->clipMask) ctlEntry |= CtlTertiary;  else ctlEntry |= CtlBinary;
   if(srcOffX>=0)   ctlEntry |= CtlRightLeft; else ctlEntry |= CtlLeftRight;
   if(srcOffY>=0)   ctlEntry |= CtlBottomTop; else ctlEntry |= CtlTopBottom;
   if(gc->clipMask) cmbGroup  = CmbMasked;    else cmbGroup  = CmbUnMasked;
   if(srcOffX>=0)   cmbGroup |= CmbRightLeft; else cmbGroup |= CmbLeftRight;
   if(srcOffY>=0)   vertBt    = 1;            else vertBt    = 0;

   bcb->ctlRtn = getControlAddress(ctlEntry);
   depth       = gc->depth;
   function    = gc->function & 0x0F;
   planeMask   = gc->planeMask;

   for(plane=0;plane<depth;plane++)
   {  if(vertBt) offst = depth-plane-1; else offst = plane;
      if(planeMask&1) 
         bcb->combMode[offst] = getCombineAddress(cmbGroup|function);
      else
         bcb->combMode[offst] = getCombineAddress(cmbGroup|GXnoop);
      planeMask >>= 1;
   }
   
   setSrcAndMskData
   (  bcb,src,srcOffX,srcOffY,
      gc->clipMask,gc->clipXOrigin,gc->clipYOrigin,
      0
   );
   
   setDesData(bcb,des);
}

/*------------------------------------------------------------*/
/*                                    dpDecodeCopyPlane(...)  */
/*------------------------------------------------------------*/

void dpDecodeCopyPlane ( BlitterControlBlock_t *bcb,
                         DpPixmap_t *src, DpPixmap_t *des,
                         int srcOffX, int srcOffY,
                         DpGraphicsContext_t *gc,
                         int planeNo
                       )
{  int             cmbGroup,cmbEntry,ctlEntry,function,plane;
   unsigned long   colF,colB,planeMask;

   if(gc->clipMask) ctlEntry = CtlTertiary, cmbGroup = CmbMasked;
   else             ctlEntry = CtlBinary,   cmbGroup = CmbUnMasked;
   ctlEntry |= CtlStippled | CtlLeftRight | CtlTopBottom;
   cmbGroup |= CmbLeftRight;
   bcb->ctlRtn = getControlAddress(ctlEntry);
   
   colF      = gc->foreground;  colB     = gc->background;
   planeMask = gc->planeMask;   function = gc->function & 0x0F;

   for(plane=0;plane<gc->depth;plane++)
   {  if(planeMask&1)
      {  if(colF&1) cmbEntry  = (function&0x3);
         else       cmbEntry  = (function&0xC)>>2;
         if(colB&1) cmbEntry |= (function&0x3)<<2; 
         else       cmbEntry |= (function&0xC);
         cmbEntry |= cmbGroup;
      } else cmbEntry = GXnoop | cmbGroup | CmbLeftRight;

      bcb->combMode[plane] = getCombineAddress(cmbEntry);
      planeMask>>=1; colF>>=1; colB>>=1;
   }
   
   setSrcAndMskData
   (  bcb,src,srcOffX,srcOffY,
      gc->clipMask,gc->clipXOrigin,gc->clipYOrigin,
      1
   );
   bcb->srcBase = (char*)bcb->srcBase + planeNo*src->wpv*4;
   bcb->srcLast = (char*)bcb->srcLast + planeNo*src->wpv*4;
   setDesData(bcb,des);
}

/*------------------------------------------------------------*/
/*                                     setSrcAndMskData(...)  */
/*------------------------------------------------------------*/

static void setSrcAndMskData ( BlitterControlBlock_t  *bcb,
                               DpPixmap_t             *src,
                               int                     srcOffX,
                               int                     srcOffY,
                               DpPixmap_t             *msk,
                               int                     mskOffX,
                               int                     mskOffY,
                               int                     stipple
                             )
{  char *srcBase;

   bcb->srcBase = srcBase = (char*)src->rawBase + 4;
   bcb->srcLast = srcBase + src->wpv*4*(src->depth*src->sizeY-1);
   bcb->srcWPL  = src->wpv*src->depth;
   if(stipple) bcb->srcWPV = bcb->srcWPL;
   else        bcb->srcWPV = src->wpv;
   bcb->srcSizX = src->sizeX;
   bcb->srcSizY = src->sizeY;
   bcb->srcOffX = srcOffX;
   bcb->srcOffY = srcOffY;
   
   if(msk!=NULL)
   {  bcb->mskBase = (char*)msk->rawBase+4;
      bcb->mskWPV  = msk->wpv * msk->depth;
      bcb->mskOffX = mskOffX;
      bcb->mskOffY = mskOffY;
   }

}
