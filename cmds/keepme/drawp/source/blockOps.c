/*-----------------------------------------------------------*/
/*                                               blockOps.c  */
/*-----------------------------------------------------------*/

/* This file performs the block-operation functions:         */
/*    basically the copyArea and clearArea functions, and    */
/*    the copyPlane funciton. copyArea takes account of the  */
/*    direction of copying so that copying within a single   */
/*    pixmap (scrolling), occurs correctly.                  */
/* All operations take account of the clip-list in the       */
/*   graphics context.                                       */

/*-----------------------------------------------------------*/
/*                                           #include files  */
/*-----------------------------------------------------------*/

#include "private.h"

/*-----------------------------------------------------------*/
/*              Look-ahead declarations for local funcitons  */
/*-----------------------------------------------------------*/

static void processClipList
        (   BlitterControlBlock_t *bcb,
            DpPixmap_t            *des,
            int lftX,  int topY,
            int rgtX,  int botY
        );

/*-----------------------------------------------------------*/
/*                                          dpCopyArea(...)  */
/*-----------------------------------------------------------*/

void dpCopyArea ( DpPixmap_t          *src,
                  DpPixmap_t          *des,
                  DpGraphicsContext_t  *gc,
                  int srcLftX,  int srcTopY,
                  unsigned int width,  unsigned int height,
                  int desLftX,  int desTopY
                )
{  BcbContainer_t     bcbc;
   int desRgtX,desBotY;
   
   if(width>0)  desRgtX=desLftX+width ; else desRgtX=des->sizeX;
   if(height>0) desBotY=desTopY+height; else desBotY=des->sizeY;
   
   dpDecodeCopyArea(&bcbc.bcbProper,src,des,desLftX-srcLftX,desTopY-srcTopY,gc);
   
   processClipList(&bcbc.bcbProper,des,desLftX,desTopY,desRgtX,desBotY);
}

/*-----------------------------------------------------------*/
/*                                         dpCopyPlane(...)  */
/*-----------------------------------------------------------*/

void dpCopyPlane ( DpPixmap_t          *src,
                   DpPixmap_t          *des,
                   DpGraphicsContext_t  *gc,
                   int srcLftX,  int srcTopY,
                   unsigned int width,  unsigned int height,
                   int desLftX,  int desTopY,
                   unsigned long  plane
                 )
{  int planeNo;
   int desRgtX,desBotY;
   BcbContainer_t bcbc;
   
   if(plane==0) return;
   for(planeNo=0;(plane&1)==0;planeNo++) plane>>=1;

   if(width>0)  desRgtX=desLftX+width ; else desRgtX=des->sizeX;
   if(height>0) desBotY=desTopY+height; else desBotY=des->sizeY;

   dpDecodeCopyPlane
   (&bcbc.bcbProper,src,des,desLftX-srcLftX,desTopY-srcTopY,gc,planeNo);
   
   processClipList(&bcbc.bcbProper,des,desLftX,desTopY,desRgtX,desBotY);
}

/*-----------------------------------------------------------*/
/*                                   dpProcessClipList(...)  */
/*-----------------------------------------------------------*/

static void processClipList ( BlitterControlBlock_t *bcb,
                              DpPixmap_t            *des,
                              int lftX, int topY, int rgtX, int botY
                            )
/* Decompose the rectangular area passed by intersecting it with */
/*   the clip-list, and for every region generated, pass it down */
/*   by indirection through the blitter control block to the     */
/*   appropriate operation.                                      */
{  DpClipNode_t      *cn;
   DpClipNodeEntry_t *vp,*xp;
   int                blkLftX,blkRgtX,blkTopY,blkBotY;
   
   if((cn=des->visRgn)==DpDefaultClipping)
   {  if(lftX<0)          lftX=0;
      if(rgtX>des->sizeX) rgtX=des->sizeX;
      if(topY<0)          topY=0;
      if(botY>des->sizeY) botY=des->sizeY;
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
