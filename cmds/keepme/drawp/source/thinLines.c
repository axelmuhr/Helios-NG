/*-------------------------------------------------------------------*/
/*                                                      thinLines.c  */
/*-------------------------------------------------------------------*/

/* $Header: thinLines.c,v 1.8 90/07/11 15:37:21 charles Locked $ */
/* $Source: /server/usr/users/a/charles/world/drawp/RCS/source/thinLines.c,v $ */

/* This file contains routines which accept line-drawing requests for */
/*    zero width lines, decode the graphics context, compute the      */
/*    quadrant in which the line runs and the major axis of the line, */
/*    and call the appropriate one-of-eight routines 'dpClipLine...'  */
/* The 'dpClipLine...' routines are compiled by compiling the file    */
/*    'lineClipping' eight times, each with 'LineMode' set            */
/*    appropriately so as to compile the correct routine.             */

/*-------------------------------------------------------------------*/
/*                                              Other include files  */
/*-------------------------------------------------------------------*/

#include <stdlib.h>
#include "private.h"
#include "code_interface.h"

/*-------------------------------------------------------------------*/
/*                            Look-ahead declarations for this file  */
/*-------------------------------------------------------------------*/

static void decodeGcForLine 
     (  LineControlBlock_t  *lcb,
        DpPixmap_t          *des,
        DpGraphicsContext_t *gc
     );

static void decodeSolid
     (  LineControlBlock_t  *lcb,
        DpGraphicsContext_t *gc
     );

static void copyDashList
     (  LineControlBlock_t *lcb,
        DpGraphicsContext_t *gc
     );

/*-------------------------------------------------------------------*/
/*                                              dpDrawThinLine(...)  */
/*-------------------------------------------------------------------*/

void dpDrawThinLine(DpPixmap_t *pm,
                    DpGraphicsContext_t *gc,
                    int x1,int y1,
                    int x2,int y2)
{  LcbContainer_t  lcbc;
   DpClipNode_t       *cn;

   decodeGcForLine(&lcbc.lcbProper,pm,gc);

   /* Repeat the primitive for each clip-node: */
   for(cn=pm->visRgn;cn;cn=cn->nextNode)
   {  DpClipNodeEntry_t *vp;
      DpDefaultNode_t   dcn;
      if(cn==DpDefaultClipping)
      {  dpSetDefaultClipping(&dcn,pm);
         vp = dcn;
      } else vp=cn->node;
      lcbc.lcbProper.strOff = gc->dashOffset;
      dpClipLine(&lcbc.lcbProper,vp,x1<<Bf,y1<<Bf,x2<<Bf,y2<<Bf,gc->capStyle);
      if(cn==DpDefaultClipping) break;
   }
   if(lcbc.lcbProper.strAllocd) free(lcbc.lcbProper.strBase);
}

/*-------------------------------------------------------------------*/
/*                                          dpDrawThinSegments(...)  */
/*-------------------------------------------------------------------*/

void dpDrawThinSegments ( DpPixmap_t          *pm,
                          DpGraphicsContext_t *gc,
                          DpSegment_t         *seg,
                          int                  qty
                        )
{  LcbContainer_t    lcbc;
   DpClipNode_t       *cn;
   int                 sn;

   decodeGcForLine(&lcbc.lcbProper,pm,gc);

   /* Repeat the primitive for each clip-node: */
   for(cn=pm->visRgn;cn;cn=cn->nextNode)
   {  DpClipNodeEntry_t *vp;
      DpDefaultNode_t   dcn;
      if(cn==DpDefaultClipping)
      {  dpSetDefaultClipping(&dcn,pm);
         vp = dcn;
      } else vp=cn->node;
      for(sn=0;sn<qty;sn++)
      {  lcbc.lcbProper.strOff = gc->dashOffset;
         dpClipLine(&lcbc.lcbProper,vp,
                    seg[sn].p.x<<Bf,seg[sn].p.y<<Bf,
                    seg[sn].q.x<<Bf,seg[sn].q.y<<Bf,
                    gc->capStyle);
      }
      if(cn==DpDefaultClipping) break;
   }
   if(lcbc.lcbProper.strAllocd) free(lcbc.lcbProper.strBase);
}

/*-------------------------------------------------------------------*/
/*                                             dpDrawThinLines(...)  */
/*-------------------------------------------------------------------*/

void dpDrawThinLines ( DpPixmap_t          *pm,
                       DpGraphicsContext_t *gc,
                       DpPoint_t           *pnts,
                       int                  npnts,
                       int                  mde
                     )
{  LcbContainer_t    lcbc;
   DpClipNode_t       *cn;
   int                 pn;
   int                 x1,y1,x2,y2;

   decodeGcForLine(&lcbc.lcbProper,pm,gc);

   /* Repeat the primitive for each clip-node: */
   for(cn=pm->visRgn;cn;cn=cn->nextNode)
   {  DpClipNodeEntry_t *vp;
      DpDefaultNode_t   dcn;
      if(cn==DpDefaultClipping)
      {  dpSetDefaultClipping(&dcn,pm);
         vp = dcn;
      } else vp=cn->node;
      /* Get first point: */
      x1 = pnts[0].x; y1 = pnts[0].y;
      /* Reset dash-offset: */
      lcbc.lcbProper.strOff = gc->dashOffset;
      /* Draw the lines: */
      for(pn=1;pn<npnts-1;pn++)
      {  x2 = pnts[pn].x; y2 = pnts[pn].y;
         if(mde==CoordModePrevious) x2+=x1,y2+=y1;
         dpClipLine(&lcbc.lcbProper,vp,x1<<Bf,y1<<Bf,x2<<Bf,y2<<Bf,CapNotLast);
         x1 = x2; y1 = y2;
      }
      /* Last point: */
      x2 = pnts[pn].x; y2 = pnts[pn].y;
      if(mde==CoordModePrevious) x2+=x1,y2+=y1;
      if(x2==pnts[0].x&&y2==pnts[0].y)
         dpClipLine(&lcbc.lcbProper,vp,x1<<Bf,y1<<Bf,x2<<Bf,y2<<Bf,CapNotLast);
      else
         dpClipLine(&lcbc.lcbProper,vp,x1<<Bf,y1<<Bf,x2<<Bf,y2<<Bf,gc->capStyle);
      if(cn==DpDefaultClipping) break;
   }
   if(lcbc.lcbProper.strAllocd) free(lcbc.lcbProper.strBase);
}

/*-------------------------------------------------------------------*/
/*                                             decodeGcForLine(...)  */
/*-------------------------------------------------------------------*/

static void decodeGcForLine ( LineControlBlock_t  *lcb,
                              DpPixmap_t          *des,
                              DpGraphicsContext_t *gc
                            )
/* Assumes a fill-style of 'FillSolid'                               */
{
   lcb->desBase  = (char*)des->rawBase+4;  /* Write destination info... */
   lcb->desWpv   = des->wpv;
   lcb->desWpl   = des->depth*des->wpv;
   lcb->desDepth = des->depth;
      
   if(gc->clipMask)
   {  DpPixmap_t *msk;                    /* Write mask info ...        */
      msk = gc->clipMask;
      lcb->mskBase  = (char*)msk->rawBase+4;
      lcb->mskWpl   = msk->depth*msk->wpv;
      lcb->mskOffX  = gc->clipXOrigin;
      lcb->mskOffY  = gc->clipYOrigin;
   }
   else lcb->mskBase = NULL;
   
   decodeSolid(lcb,gc);
}

/*-------------------------------------------------------------------*/
/*                                                 decodeSolid(...)  */
/*-------------------------------------------------------------------*/

static void decodeSolid ( LineControlBlock_t  *lcb,
                          DpGraphicsContext_t *gc
                        )
{  unsigned long d0,d1,col;
   int fn;
   
   lcb->mainGroup = (gc->clipMask?LmnMasked:LmnUnMasked);

   fn = gc->function;
   /* d0 is the colour a destination pixel would be mapped to if it */
   /*     were 2_000000..., d1 is the colour if it were 2_11111...  */
   col=gc->foreground; d0=d1=0;
   if(fn&(1<<3)) d0 |= ~col;   if(fn&(1<<1)) d0 |= col;
   if(fn&(1<<2)) d1 |= ~col;   if(fn&(1<<0)) d1 |= col;
   d0 &= gc->planeMask; d1 |= ~gc->planeMask;
   d0 &= 0xFFFF; d1 &= 0xFFFF;
   /* Now encode this information into the line control block:  */
   lcb->foreground = (d1<<16)|(d0);
   
   switch(gc->lineStyle)
   {  case LineSolid:
      {  lcb->strBase    = NULL;
         lcb->strPtr     = lcb->strList;
         lcb->strAllocd  = 0;
         lcb->background = lcb->foreground;
         break;
      }
      case LineOnOffDash:
      {  copyDashList(lcb,gc);
         lcb->background = 0xFFFF0000; /* NOP code */
         break;
      }
      case LineDoubleDash:
      {  copyDashList(lcb,gc);
         col=gc->background; d0=d1=0;
         if(fn&(1<<3)) d0 |= ~col; if(fn&(1<<2)) d0 |= col;
         if(fn&(1<<1)) d1 |= ~col; if(fn&(1<<0)) d1 |= col;
         d0 &= gc->planeMask; d1 |= ~gc->planeMask;
         d0&=0xFFFF; d1&=0xFFFF;
         lcb->background = (d1<<16)|(d0);
         break;
      }
   }
}

/*-------------------------------------------------------------------*/
/*                                                copyDashList(...)  */
/*-------------------------------------------------------------------*/

static void copyDashList(LineControlBlock_t *lcb,DpGraphicsContext_t *gc)
{  int            reqd,cnt;
   unsigned char *des,*src;
   unsigned int   period;
   
   if(gc->dashLength&1)
   {  /* Here if dot/dash length was odd */
      reqd = gc->dashLength*2+1;
      if(reqd>MaxDotDash)
      {  lcb->strBase   = des = calloc(reqd,sizeof(unsigned char));
         lcb->strAllocd = 1;
      } else
      {  lcb->strBase   = des = lcb->strList;
         lcb->strAllocd = NULL;
      }
      period = 0;
      src = gc->dashList;
      for(cnt=gc->dashLength;cnt;cnt--) period+=(*des++=*src++);
      src = gc->dashList;
      for(cnt=gc->dashLength;cnt;cnt--) period+=(*des++=*src++);
      *des=0;
      lcb->strPeriod = period;
   }
   else
   {  /* Here if dot/dash length was even */
      reqd = gc->dashLength+1;
      if(reqd>MaxDotDash)
      {  lcb->strBase   = des = calloc(reqd,sizeof(unsigned char));
         lcb->strAllocd = 1;
      } else
      {  lcb->strBase   = des = lcb->strList;
         lcb->strAllocd = NULL;
      }
      src = gc->dashList;
      period = 0;
      for(cnt=gc->dashLength;cnt;cnt--) period+=(*des++=*src++);
      *des=0;
      lcb->strPeriod = period;
   }
}   

/*-------------------------------------------------------------------*/
/*                                              drawSingleLine(...)  */
/*-------------------------------------------------------------------*/
 
void dpClipLine ( LineControlBlock_t *lcb,
                  DpClipNodeEntry_t   *vp,
                  int x,int y,
                  int ex,int ey,
                  int capStyle
                )
/* Generic version of the 'dpClipLine...' functions compiled by      */
/*   separate compilations of the file 'lineClipping.c'. This        */
/*   function computes the octant, and branches through to the       */
/*   appropriate routine                                             */
{  
   if(ex>=x)
   {  if(ey>=y)
      {  if(ex-x>=ey-y)
            dpClipLineXpYpMx(lcb,vp,x,y,ex,ey,capStyle);
         else
            dpClipLineXpYpMy(lcb,vp,x,y,ex,ey,capStyle);
      }
      else
      {  if(ex-x>=y-ey)
            dpClipLineXpYnMx(lcb,vp,x,y,ex,ey,capStyle);
         else
            dpClipLineXpYnMy(lcb,vp,x,y,ex,ey,capStyle);
      }
   }
   else
   {  if(ey>=y)
      {  if(x-ex>=ey-y)
            dpClipLineXnYpMx(lcb,vp,x,y,ex,ey,capStyle);
         else
            dpClipLineXnYpMy(lcb,vp,x,y,ex,ey,capStyle);
      }
      else
      {  if(x-ex>=y-ey)
            dpClipLineXnYnMx(lcb,vp,x,y,ex,ey,capStyle);
         else
            dpClipLineXnYnMy(lcb,vp,x,y,ex,ey,capStyle);
      }
   }
}

