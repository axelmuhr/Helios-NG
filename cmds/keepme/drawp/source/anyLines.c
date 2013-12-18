/*------------------------------------------------------------*/
/*                                                anyLines.c  */
/*------------------------------------------------------------*/

/* This file contains public routines to process the line     */
/*   drawing request functions:                               */
/* void dpDrawLine(...)                                       */
/* void dpDrawSegments(...)                                   */
/* void dpDrawLines(...)                                      */
/* These have functionality similar to that of 'X'.           */
/* Almost the first thing these routines do is to check to    */
/*   see if the 'lineWidth' is set to zero in the graphics    */
/*   context. If so, they branch over to routines in the file */
/*   'thinLines.c' which deals with drawing zero-width lines  */
/*   separately.                                              */

/*------------------------------------------------------------*/
/*                                             Include files  */
/*------------------------------------------------------------*/

#include <stdlib.h>
#include "private.h"

/*------------------------------------------------------------*/
/*                 Local functions : Look-ahead declarations  */
/*------------------------------------------------------------*/

static int dpDrawThickLine     (  DpPixmap_t             *pm,
                                  DpGraphicsContext_t    *gc,
                                  int                     x1,
                                  int                     y1,
                                  int                     x2,
                                  int                     y2,
                                  int                 doEven
                               );


static int dpDrawThickLines    (  DpPixmap_t             *pm,
                                  DpGraphicsContext_t    *gc,
                                  DpPoint_t            *pnts,
                                  int                  npnts,
                                  int                    mde,
                                  int                 doEven
                               );


static int dpDrawThickSegments (  DpPixmap_t             *pm,
                                  DpGraphicsContext_t    *gc,
                                  DpSegment_t          *segs,
                                  int                  nsegs,
                                  int                 doEven
                               );

static int dpFlattenLine      (  int x1, int y1,
                                 int x2, int y2,
                                 Flattening_t *li,
                                 int capOrJoinStyle,
                                 int toDrawLine
                              );

/*------------------------------------------------------------*/
/*                                           dpDrawLine(...)  */
/*------------------------------------------------------------*/

void dpDrawLine ( DpPixmap_t          *des,
                  DpGraphicsContext_t *gc,
                  int x1, int y1,
                  int x2, int y2
                )
{  if(gc->lineWidth==0)
   {  dpDrawThinLine(des,gc,x1,y1,x2,y2);
      return;
   }
   dpDrawThickLine(des,gc,x1,y1,x2,y2,1);
   if(gc->lineStyle==LineDoubleDash)
      dpDrawThickLine(des,gc,x1,y1,x2,y2,0);
}

/*------------------------------------------------------------*/
/*                                       dpDrawSegments(...)  */
/*------------------------------------------------------------*/

void dpDrawSegments ( DpPixmap_t          *des,
                      DpGraphicsContext_t *gc,
                      DpSegment_t         *seg,
                      int                  qty
                    )
{  if(gc->lineWidth==0)
   {  dpDrawThinSegments(des,gc,seg,qty);
      return;
   }
   dpDrawThickSegments(des,gc,seg,qty,1);
   if(gc->lineStyle==LineDoubleDash)
      dpDrawThickSegments(des,gc,seg,qty,0);
}

/*------------------------------------------------------------*/
/*                                          dpDrawLines(...)  */
/*------------------------------------------------------------*/

void dpDrawLines ( DpPixmap_t          *des,
                   DpGraphicsContext_t *gc,
                   DpPoint_t           *pnts,
                   int                  npnts,
                   int                  mde
                 )
{  if(gc->lineWidth==0)
   {  dpDrawThinLines(des,gc,pnts,npnts,mde);
      return;
   }
   dpDrawThickLines(des,gc,pnts,npnts,mde,1);
   if(gc->lineStyle==LineDoubleDash)
      dpDrawThickLines(des,gc,pnts,npnts,mde,0);
}

/*------------------------------------------------------------*/
/*                                      dpDrawThickLine(...)  */
/*------------------------------------------------------------*/

static int dpDrawThickLine ( DpPixmap_t            *des,
                             DpGraphicsContext_t    *gc,
                             int                     x1,
                             int                     y1,
                             int                     x2,
                             int                     y2,
                             int                 doEven
                           )
/* Draw the odd (resp even) strokes of the line if 'doEven' is   */
/*    is zero (resp. non-zero). Use the graphics context and     */
/*    pixmap provided. If 'doEven' is zero, the line-style must  */
/*    be 'LineDoubleDash'                                        */
/* '0' is returned of there was a problem allocating memory      */
{  BcbContainer_t        bcbc;
   Flattening_t           fl;
   int                   nplys;
   DpPolygon_t           *plys;

   /* Get the 'brush' type information (colours, fill styles     */
   /*    etc, into a form suitable for the assembley routines:   */
   dpDecodeGcToBcb(&bcbc.bcbProper,gc,doEven);
   bcbc.bcbProper.desBase = (char*)des->rawBase + 4;
   bcbc.bcbProper.desBPP  = des->depth;
   bcbc.bcbProper.desWPV  = des->wpv;
   
   /* Prepare the geometric information of the line:             */
   dpDecodeLineInformation(&fl,gc,doEven);
   
   /* Get an upper limit for the number of polygons required to  */
   /*    make up a line, and allocate space for them:            */
   nplys = dpEstimatePolygonsFor(x1,y1,x2,y2,&fl) +2;
   plys  = calloc(nplys,sizeof(DpPolygon_t));
   if(plys==NULL) return 0;
   fl.polyVector = plys;
   
   /* Flatten the line applying the cap- and join- styles:       */
   dpFlattenLine(x1<<Bf,y1<<Bf,x2<<Bf,y2<<Bf,&fl,gc->capStyle,1);
   dpDoCapOrJoin(&fl,&fl.prvLft,&fl.prvRgt,gc->capStyle);
   
   /* Plot the resulting polygons: */
   if( !dpPolygonsComplexClipping
       (  &bcbc.bcbProper, des,
          plys, fl.polyVector-plys,
          CoordModeOrigin, WindingRule, 1
       )
     )
   {  free(plys);
      return 0;
   }
   
   free(plys);
   return 1;
}

/*------------------------------------------------------------*/
/*                                     dpDrawThickLines(...)  */
/*------------------------------------------------------------*/

static int dpDrawThickLines ( DpPixmap_t            *des,
                              DpGraphicsContext_t    *gc,
                              DpPoint_t            *pnts,
                              int                  npnts,
                              int                    mde,
                              int                 doEven
                            )
/* Draw the odd (resp even) strokes of the lines if 'doEven' is  */
/*    is zero (resp. non-zero). Use the graphics context and     */
/*    pixmap provided. If 'doEven' is zero, the line-style must  */
/*    be 'LineDoubleDash'                                        */
/* '0' is returned of there was a problem allocating memory      */
/* Refer to 'dpDrawThickLine' for the basic principles           */
{  BcbContainer_t         bcbc;
   Flattening_t             fl;
   int          nplys,i,closed;
   int             x1,y1,x2,y2;
   int                 xi2,yi2;
   DpPolygon_t           *plys;
   DpPoint_t              *pnt;

   dpDecodeGcToBcb(&bcbc.bcbProper,gc,doEven);
   bcbc.bcbProper.desBase = (char*)des->rawBase + 4;
   bcbc.bcbProper.desBPP  = des->depth;
   bcbc.bcbProper.desWPV  = des->wpv;
   dpDecodeLineInformation(&fl,gc,doEven);
   x2=pnts->x; y2=pnts->y;
   for(nplys=i=1,pnt=pnts;i<npnts;i++,pnt++)
   {  x1=x2; x2=pnt->x; y1=y2; y2=pnt->y;
      if(mde==CoordModePrevious) x2+=x1,y2+=y1;
      nplys += dpEstimatePolygonsFor(x1<<Bf,y1<<Bf,x2<<Bf,y2<<Bf,&fl)+1;
   }
   if(x2==pnts->x && y2==pnts->y) closed=1; else closed=0;
   plys  = calloc(nplys,sizeof(DpPolygon_t));
   if(plys==NULL) return 0;
   fl.polyVector = plys;
   
   /* Flatten the first line in a special way: if the lines form a  */
   /*    closed loop, then ensure that no initial cap- or join-     */
   /*    style is applied, otherwise, apply the initial cap- style  */
   /*    in addition:                                               */
   x1=pnts[0].x; y1=pnts[0].y; x2=pnts[1].x; y2=pnts[1].y;
   if(mde==CoordModePrevious) xi2+=x1,yi2+=y1;
   xi2=x2; yi2=y2;
   dpFlattenLine
   (  x1<<Bf,y1<<Bf,x2<<Bf,y2<<Bf,
      &fl,closed?NoCapOrJoin:gc->capStyle,1
   );
   
   /* Go through a loop for the rest of the lines, apply intermediate */
   /*    join styles between the lines:                               */
   for(pnt=pnts+1,i=2;i<npnts;i++)
   {  pnt++; x1=x2; x2=pnt->x; y1=y2; y2=pnt->y;
      if(mde==CoordModePrevious) x2+=x1,y2+=y1;
      dpFlattenLine(x1<<Bf,y1<<Bf,x2<<Bf,y2<<Bf,&fl,gc->joinStyle,1);
   }

   /* At this point, the second end point of the last line is in      */
   /*    (x2,y2), and the second end point of the first line is in    */
   /*    (xi2,yi2). Thus, if the lines form a closed loop, then the   */
   /*    first line co-ordinates are (x2,y2) to (xi2,yi2).            */
   if(closed)
   {  /* Apply final join style between end of last line and start of */
      /*   first line:                                                */
      dpFlattenLine(x1<<Bf,y1<<Bf,x2<<Bf,y2<<Bf,&fl,gc->joinStyle,0);
   } else
   {  /* Apply the last cap-style: */
      dpDoCapOrJoin(&fl,&fl.prvLft,&fl.prvRgt,gc->capStyle);
   }
   
   /* Plot the resulting polygons: */
   if( !dpPolygonsComplexClipping
       (  &bcbc.bcbProper, des,
          plys, fl.polyVector-plys,
          CoordModeOrigin, WindingRule, 1
       )
     )
   {  free(plys);
      return 0;
   }
   
   free(plys);
   return 1;
}

/*------------------------------------------------------------*/
/*                                  dpDrawThickSegments(...)  */
/*------------------------------------------------------------*/

static int dpDrawThickSegments ( DpPixmap_t            *des,
                                 DpGraphicsContext_t    *gc,
                                 DpSegment_t          *segs,
                                 int                  nsegs,
                                 int                 doEven
                               )
/* Draw the odd (resp even) strokes of the lines if 'doEven' is  */
/*    is zero (resp. non-zero). Use the graphics context and     */
/*    pixmap provided. If 'doEven' is zero, the line-style must  */
/*    be 'LineDoubleDash'                                        */
/* '0' is returned of there was a problem allocating memory      */
/* Refer to 'dpDrawThickLine' for the basic principles           */
{  BcbContainer_t         bcbc;
   Flattening_t         fl,ifl;
   int                 nplys,i;
   DpPolygon_t           *plys;
   DpSegment_t            *seg;
   int             x1,y1,x2,y2;

   dpDecodeGcToBcb(&bcbc.bcbProper,gc,doEven);
   bcbc.bcbProper.desBase = (char*)des->rawBase + 4;
   bcbc.bcbProper.desBPP  = des->depth;
   bcbc.bcbProper.desWPV  = des->wpv;
   dpDecodeLineInformation(&ifl,gc,doEven);

   for(i=0,seg=segs;i<nsegs;i++,seg++)
   {  x1=seg->p.x; y1=seg->p.y; x2=seg->q.x; y2=seg->q.y;
      fl=ifl;
      nplys = dpEstimatePolygonsFor(x1<<Bf,y1<<Bf,x2<<Bf,y2<<Bf,&fl) +2;
      plys  = calloc(nplys,sizeof(DpPolygon_t));
      if(plys==NULL) return 0;
      fl.polyVector = plys;
      dpFlattenLine(x1,y1,x2,y2,&fl,gc->capStyle,1);
      dpDoCapOrJoin(&fl,&fl.prvLft,&fl.prvRgt,gc->capStyle);
      /* Plot the line here: */
      if( !dpPolygonsComplexClipping
          (  &bcbc.bcbProper, des,
             plys, fl.polyVector-plys,
             CoordModeOrigin, WindingRule, 1
          )
        )
      {  free(plys);
         return 0;
      }
      free(plys);
   }
   
   return 1;
}

/*------------------------------------------------------------*/
/*                              dpDecodeLineInformation(...)  */
/*------------------------------------------------------------*/

void dpDecodeLineInformation ( Flattening_t        *fl,
                               DpGraphicsContext_t *gc,
                               int                  doEven
                             )
{  unsigned char  *strList;
   unsigned int    strSize;
   unsigned int    strPosn;
   unsigned int  strPeriod;
   int            strIndex;
   unsigned int     strElt;
   int           startEven;
   int                   i;

   /* Write the fileds that are always written to fl:          */
   fl->lineWidth = gc->lineWidth;
   
   /* Trap a LineSolid type line:                              */
   if(gc->lineStyle==LineSolid)
   {  fl->isDrawStroke = 1;
      fl->strokeList   = NULL;
      return;
   }
   
   /* Load basic stroke-list information                       */
   fl->strokeList     = strList = gc->dashList;
   fl->strokeListSize = strSize = gc->dashLength;

   /* Locate starting position in stroke list:                 */
   strPosn = gc->dashOffset;
   for(i=0,strPeriod=0;i<strSize;i++) strPeriod+=strList[i];
   startEven = ( (strSize&1) && ((strPosn/strPeriod)&1) );
   strPosn %= strPeriod;

   /* Continued:                                              */
   for(strElt=strList[strIndex=0];strElt<=strPosn;)
      strPosn-=strElt, strElt=strList[++strIndex], startEven^=1;
   if(++strIndex>=strSize) strIndex=0;

   /* Write remaining information to fl:                      */
   fl->strokeLength = (strElt-strPosn)<<Bf;
   fl->strokeIndex  = strIndex;
   fl->strokePeriod = strPeriod;
   fl->isDrawStroke = doEven^startEven^1;
}

/*------------------------------------------------------------*/
/*                                dpEstimatePolygonsFor(...)  */
/*------------------------------------------------------------*/

int dpEstimatePolygonsFor ( int x1, int y1,
                            int x2, int y2,
                            Flattening_t *fl
                          )
/* This function will estimate the number of polygon entries  */
/*    required to flatten the given line. The result will be  */
/*    an upper limit.                                         */
{  int l,dx,dy;

   /* Estimate an upper limit for the length of the line: */
   if(fl->strokeList==NULL) return 1;
   dx = x1-x2; if(dx<0) dx=-dx;
   dy = y1-y2; if(dy<0) dy=-dy;
   l=dx+dy;

   /* Estimate the maximum number of traversals of the dot   */
   /*    dash list will be made (upper limit)                */
   l = (l/fl->strokePeriod)+1;
   
   return l*(fl->strokeListSize+1)/2;
}

/*------------------------------------------------------------*/
/*                                        dpFlattenLine(...)  */
/*------------------------------------------------------------*/

static int dpFlattenLine ( int x1, int y1,
                           int x2, int y2,
                           Flattening_t *li,
                           int capOrJoinStyle,
                           int toDrawLine
                         )
/* This routine is used to flatten the odd or even strokes of   */
/*   a line segment. The relevant information is passed to and  */
/*   from this routine in the structure <*li>. The line to be   */
/*   drawn is passed as parameters which are in sub-pixel unit  */
/*   co-ordinates.                                              */
/* A join- or cap- style of style <es> is applied to the        */
/*   beginning of the current line. <es> can be 'NoJoinOrCap'   */
/*   to indicate neither.                                       */
/* The routine returns zero if there was a problem allocating   */
/*   memory, in which case li->polyVector will point 1 past the */
/*   last polygon for which memory was sucessfully allocated    */
/* The <capOrJoinStyle> is applied between the previous         */
/*    line-end as stored in <li> and the current line-start,    */
/*    may involve the addition of an extra polygon. Also if the */
/*    parameter <toDrawLine> is zero, then that is all that     */
/*   happens, and the remainder of the line is not drawn.       */
/* If the routine returns with <isDrawStroke> set to 1 in <li>  */
/*    the <prvLft> and <prvRgt> will also be set correctly      */
{  DpPolygon_t *ply;  DpSubPixel_t *pnts,l2,r2;
   int lf,l,al,le,wid,drw,dx,dy,wx,wy,x3,y3,x4,y4;
   unsigned char *strList;  int strIndex,strLimit;

   dx  = x2-x1;                       dy  =  y2-y1;
   lf  = dpVectorLength(dx,dy);
   
   dx  = dpUnitComponent(dx,lf);      dy  = dpUnitComponent(dy,lf);
   l   = dpDecodeLength(lf);          wid = li->lineWidth;
   wx  = -dpMeasureComponent(wid,dy); wy  = dpMeasureComponent(wid,dx);
   drw = li->isDrawStroke;            al  = 0;
   le  = li->strokeLength;
   
   if ( capOrJoinStyle!=NoCapOrJoin ) 
   {  l2.x = x1-wx;  l2.y = y1-wy;
      r2.x = x1+wx;  r2.y = y1+wy;
      if(!dpDoCapOrJoin(li,&l2,&r2,capOrJoinStyle)) return 0;
   }

   if(!toDrawLine) return 1;
   
   strList  = li->strokeList; if(strList==NULL) goto trapFillSolid;
   strIndex = li->strokeIndex;
   strLimit = li->strokeListSize;

   while(le<l)
   {  /* Now to create the stroke: */
      if(drw)
      {  x3 = x1 + dpMeasureComponent(al,dx);
         y3 = y1 + dpMeasureComponent(al,dy);
         x4 = x1 + dpMeasureComponent(le,dx);
         y4 = y1 + dpMeasureComponent(le,dy);
         pnts = calloc(4,sizeof(DpSubPixel_t));
         if(pnts==NULL) return 0;
         ply = (li->polyVector)++;
         ply->npnts = 4;  ply->pnts.fine = pnts;
         pnts[0].x = x4+wx;  pnts[0].y = y4+wy;
         pnts[1].x = x4-wx;  pnts[1].y = y4-wy;
         pnts[2].x = x3-wx;  pnts[2].y = y3-wy;
         pnts[3].x = x3+wx;  pnts[3].y = y3+wy;
      }
      al=le;
      le += strList[strIndex++]; if(strIndex>=strLimit) strIndex=0;
      drw = drw^1;
   }

   /* Deal with the last stroke: */
   trapFillSolid:
   if(drw)
   {  x3 = x1 + dpMeasureComponent(al,dx);
      y3 = y1 + dpMeasureComponent(al,dy);
      li->prvLft.x = x2-wx  ;  li->prvLft.y = y2-wy;
      li->prvRgt.x = x2+wx  ;  li->prvRgt.y = y2+wy;
      pnts = calloc(4,sizeof(DpSubPixel_t));
      if(pnts==NULL) return 0;
      ply = (li->polyVector)++;
      ply->npnts = 4; ply->pnts.fine = pnts;
      pnts[0].x = x2+wx;  pnts[0].y = y2+wy;
      pnts[1].x = x2-wx;  pnts[1].y = y2-wy;
      pnts[2].x = x3-wx;  pnts[2].y = y3-wy;
      pnts[3].x = x3+wx;  pnts[3].y = y3+wy;
   }

   if(strList==NULL) return 1;

   if(l-al==0)
   {  l += strList[strIndex++]; if(strIndex>=strLimit) strIndex=0;  }
   li->strokeLength = l-al;
   li->isDrawStroke = drw;
   li->strokeIndex  = strIndex;
   return 1;
}
