/*---------------------------------------------------------------------*/
/*                                                     rectRequests.c  */
/*---------------------------------------------------------------------*/

/* $Header: rectRequests.c,v 1.3 90/07/11 15:36:59 charles Locked $ */
/* $Source: /server/usr/users/a/charles/world/drawp/RCS/source/rectRequests.c,v $ */

/* This file contains the definitions of the requests 'dpFillRectangle' */
/*    and 'dpFillRectangles'                                            */

/*---------------------------------------------------------------------*/
/*                                                     #include files  */
/*---------------------------------------------------------------------*/

#include "private.h"

/*---------------------------------------------------------------------*/
/*                                            Look-ahead declarations  */
/*---------------------------------------------------------------------*/

static void clipRectangle ( BlitterControlBlock_t   *bcb,
                            DpPixmap_t              *des,
                            int lftX, int topY,
                            int rgtX, int botY
                          );

/*---------------------------------------------------------------------*/
/*                                               dpFillRectangle(...)  */
/*---------------------------------------------------------------------*/

int  dpFillRectangle  (  DpPixmap_t            *des,
                         DpGraphicsContext_t    *gc,
                         int                      x,
                         int                      y,
                         unsigned int         width,
                         unsigned int         height
                      )
/* Returns 0 if there was a problem allocating memory (there never is) */
{  BcbContainer_t bcbc;

   dpDecodeGcToBcb(&bcbc.bcbProper,gc,1);
   bcbc.bcbProper.desBase = (char*)des->rawBase + 4;
   bcbc.bcbProper.desBPP  = des->depth;
   bcbc.bcbProper.desWPV  = des->wpv;
   
   clipRectangle(&bcbc.bcbProper,des,x,y,x+width,y+height);
   return 1;
}

/*---------------------------------------------------------------------*/
/*                                              dpFillRectangles(...)  */
/*---------------------------------------------------------------------*/

int  dpFillRectangles (  DpPixmap_t            *des,
                         DpGraphicsContext_t    *gc,
                         DpRectangle_t       *rects,
                         int                 nrects
                      )
/* Returns 0 if there was a problem allocating memory (there never is) */
{  BcbContainer_t bcbc;

   dpDecodeGcToBcb(&bcbc.bcbProper,gc,1);
   bcbc.bcbProper.desBase = (char*)des->rawBase + 4;
   bcbc.bcbProper.desBPP  = des->depth;
   bcbc.bcbProper.desWPV  = des->wpv;
   
   for(; nrects>0; nrects--,rects++)
   {  clipRectangle ( &bcbc.bcbProper,des,
                      rects->x              , rects->y ,
                      rects->x+rects->width , rects->y+rects->height
                    );
   }
   return 1;
}

/*---------------------------------------------------------------------*/
/*                                                 clipRectangle(...)  */
/*---------------------------------------------------------------------*/

static void clipRectangle ( BlitterControlBlock_t   *bcb,
                            DpPixmap_t              *des,
                            int lftX, int topY,
                            int rgtX, int botY
                          )
{  DpClipNode_t          *cn;
   DpClipNodeEntry_t *vp,*xp;
   int       blkLftX,blkRgtX,blkTopY,blkBotY;

   if((cn=des->visRgn)==DpDefaultClipping)
   {  if(lftX<0)          lftX = 0;
      if(rgtX>des->sizeX) rgtX = des->sizeX;
      if(topY<0)          topY = 0;
      if(botY>des->sizeY) botY = des->sizeY;
      bcb->desLftX = lftX;
      bcb->desRgtX = rgtX;
      bcb->desTopY = topY;
      bcb->desBotY = botY;
      bcb->ctlRtn(bcb);
      return;
   }
   
   for(;cn;cn=cn->nextNode)
   {  for(vp=cn->node+3;!DpPosInf(*vp);vp+=DpVisOff(vp[1])+1)
      {  blkTopY = *vp;                   if(blkTopY>=botY) break;
         blkBotY = vp[DpVisOff(vp[1])+1]; if(blkBotY<=topY) continue;
         if(blkTopY<topY) blkTopY=topY;
         if(blkBotY>botY) blkBotY=botY;
         for(xp=vp+2;!DpPosInf(*xp);xp+=2)
         {  blkLftX=xp[0]; if(blkLftX>=rgtX) break;
            blkRgtX=xp[1]; if(blkRgtX<=lftX) continue;
            if(blkLftX<lftX) blkLftX=lftX;
            if(blkRgtX>rgtX) blkRgtX=rgtX;
            bcb->desLftX = blkLftX;
            bcb->desRgtX = blkRgtX;
            bcb->desTopY = blkTopY;
            bcb->desBotY = blkBotY;
            bcb->ctlRtn(bcb);
         }
      }
   }
}
