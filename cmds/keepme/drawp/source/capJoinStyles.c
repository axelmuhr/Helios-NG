/*--------------------------------------------------------------------*/
/*                                                   capJoinStyles.c  */
/*--------------------------------------------------------------------*/

/* $Header: capJoinStyles.c,v 1.1 90/03/03 18:01:09 charles Locked $ */
/* $Source: /server/usr/users/charles/world/drawp/RCS/source/capJoinStyles.c,v $ */

/* The routines in this file perform the function of producing        */
/*     polygons to represent the effect of implementing a cap- or     */
/*     join- style to a line.                                         */

/* All routines accept as parameters a 'Flattening_t' structure       */
/*    'fl', and the left- and right- hand end-points of the line      */
/*    end being joined to or capped. In the case of a join-style, the */
/*    left- and right- handed end-points of the previous line end to  */
/*    which this line-end is being connected can be found in the      */
/*    structure <fl>. <fl> also contains various other parameters     */
/*    required, such as the width of the lines and the pointer to     */
/*    the DpPolygon_t structure where the polygon information is to   */
/*    be placed. This pointer (called 'polyVector') must then be      */
/*    incremented to point to the next DpPolygon_t structure. If      */
/*    was a problem allocating memory, the routines return 0 and the  */
/*    'polyVector' structure pointer is not incremented. If the       */
/*    'isDrawStroke' parameter is zero on entry then no polygon for   */
/*    cap- or join- style is created, 'polyVector' is not incremented */
/*    and the routine returnes with value 1. The type of cap- or      */
/*    style is determined by the name of the various functions        */
/*    except for the function 'dpDoCapOrJoin' which accepts as a      */
/*    final parameter, an enumeration indicating which cap- or join-  */
/*    stlye to implement, as defined in the file 'graphicsContexts.h' */
/*    If the enumeration is out-of-bounds, the routine does not       */
/*    create any polygon, does not increment 'polyVector', and        */
/*    and returns with value 1.                                       */

/* The 'left-' (resp. 'right-'), end-point of a line, is that corner  */
/*    point of the line that would be to your left (resp. right), as  */
/*    you stood on the end of the line looking out from the end-point */
/*    of the line to where the line would be extended to if it were   */
/*    to become longer.                                               */

/*------------------------------------------------------------------*/
/*                                                  #include files  */
/*------------------------------------------------------------------*/

#include <stddef.h>
#include <stdlib.h>
#include "private.h"

/*------------------------------------------------------------------*/
/*                                              dpDoCapOrJoin(...)  */
/*------------------------------------------------------------------*/

int dpDoCapOrJoin ( Flattening_t    *fl,
                    DpSubPixel_t    *l2,
                    DpSubPixel_t    *r2,
                    int capOrJoinStyle
                  )
{  switch(capOrJoinStyle)
   {  case NoCapOrJoin:   return 1;

      case CapNotLast:    return dpDoCapButt       (fl,l2,r2);
      case CapButt:       return dpDoCapButt       (fl,l2,r2);
      case CapRound:      return dpDoCapRound      (fl,l2,r2);
      case CapProjecting: return dpDoCapProjecting (fl,l2,r2);

      case JoinMiter:     return dpDoJoinMitered   (fl,l2,r2);
      case JoinRound:     return dpDoJoinRound     (fl,l2,r2);
      case JoinBevel:     return dpDoJoinBevelled  (fl,l2,r2);

   }
   return 1;
}

/*------------------------------------------------------------------*/
/*                                                dpDoCapButt(...)  */
/*------------------------------------------------------------------*/

int dpDoCapButt ( Flattening_t *fl,
                  DpSubPixel_t *l2,
                  DpSubPixel_t *r2
                )
{  fl=NULL; l2=NULL; r2=NULL;
   return 1;
}

/*------------------------------------------------------------------*/
/*                                               dpDoCapRound(...)  */
/*------------------------------------------------------------------*/

int dpDoCapRound ( Flattening_t *fl,
                   DpSubPixel_t *l2,
                   DpSubPixel_t *r2
                 )
{  DpPolygon_t *pl;
   DpSubPixel_t *pnts;
   int width,size,x,y;
   
   if(!fl->isDrawStroke) return 1;
   pl    = (fl->polyVector)++;
   width = fl->lineWidth;
   size  = dpGiveArcSize(width/2,width/2,0,360*Angles);
   pnts  = (DpSubPixel_t*)calloc(size,sizeof(DpSubPixel_t));
   if(pnts==NULL) return 0;
   
   x = (l2->x + r2->x) / 2  ;  y = (l2->y + r2->y) / 2 ;
   size = 0;
   dpFlattenArc(x,y,width/2,width/2,0,360*Angles,pnts,&size);

   pl->npnts     = size;
   pl->pnts.fine = pnts;
   return 1;
}

/*------------------------------------------------------------------*/
/*                                          dpDoCapProjecting(...)  */
/*------------------------------------------------------------------*/

int dpDoCapProjecting ( Flattening_t   *fl,
                        DpSubPixel_t   *l2,
                        DpSubPixel_t   *r2
                      )
{  DpPolygon_t  *pl;
   DpSubPixel_t *pnts;
   int dx,dy;
   
   if(!fl->isDrawStroke) return 1;
   pl   = (fl->polyVector)++;
   pnts = (DpSubPixel_t*)calloc(4,sizeof(DpSubPixel_t));
   if(pnts==NULL) return 0;
   
   dx = ( r2->y - l2->y ) /2;
   dy = ( l2->x - r2->x ) /2;

   pnts[0] = *l2;
   pnts[1] = *r2;
   pnts[2].x = r2->x + dx;
   pnts[2].y = r2->y + dy;
   pnts[3].x = l2->x + dx;
   pnts[3].y = l2->y + dy;

   pl->npnts     = 4;
   pl->pnts.fine = pnts;
   return 1;
}

/*--------------------------------------------------------------------*/
/*                                                dpDoJoinRound(...)  */
/*--------------------------------------------------------------------*/

int dpDoJoinRound ( Flattening_t   *fl,
                    DpSubPixel_t   *l2,
                    DpSubPixel_t   *r2
                  )
{  DpPolygon_t *pl;
   int width,siz,x,y;
   DpSubPixel_t *pnts,*l1,*r1;

   if(!fl->isDrawStroke) return 1;
   pl    = (fl->polyVector)++;
   l1    = &fl->prvLft;
   r1    = &fl->prvRgt;
   width = fl->lineWidth;
   
   /* Allocate space for polygon:  */
   siz  = dpGiveArcSize(width/2,width/2,0,360*Angles);
   pnts = (DpSubPixel_t*)calloc(siz,sizeof(DpSubPixel_t));
   if(pnts==NULL) return NULL;
   
   /* Flatten join: */
   siz=0;
   x = (l1->x+r1->x)/2; y = (l2->y+r2->y)/2;
   dpFlattenArc(x,y,width/2,width/2,0,360*Angles,pnts,&siz);
   
   /* Write information into polygon structure: */
   pl->npnts     = siz;
   pl->pnts.fine = pnts;
   
   return 1;
}

/*--------------------------------------------------------------------*/
/*                                             dpDoJoinBevelled(...)  */
/*--------------------------------------------------------------------*/

int dpDoJoinBevelled ( Flattening_t   *fl,
                       DpSubPixel_t   *l2,
                       DpSubPixel_t   *r2
                     )
{  DpPolygon_t  *pl;
   DpSubPixel_t *pnts,*l1,*r1;

   if(!fl->isDrawStroke) return 1;
   pl = (fl->polyVector)++;
   l1 = &fl->prvLft;
   r1 = &fl->prvRgt;
   
   /* Allocate space for polygon:  */
   pnts = (DpSubPixel_t*)calloc(3,sizeof(DpSubPixel_t));
   if(pnts==NULL) return NULL;
   
   /* Flatten join: */
   if ( dpGetOrientation ( r1->x - l1->x , r1->y - l1->y ,
                           r2->x - l2->x , r2->y - l2->y
                         ) < 0
      )
   {  pnts[0]   = *r2;
      pnts[1]   = *l1;
      pnts[2].x = (l1->x + r1->x)/2;
      pnts[2].y = (l1->y + r1->y)/2;
   }
   else
   {  pnts[0]   = *r1;
      pnts[1]   = *l2;
      pnts[2].x = (l1->x + r1->x)/2;
      pnts[2].y = (l1->y + r1->y)/2;
   }
  
   /* Write information into polygon structure: */
   pl->npnts     = 3;
   pl->pnts.fine = pnts;
   
   return 1;
}

/*--------------------------------------------------------------------*/
/*                                              dpDoJoinMitered(...)  */
/*--------------------------------------------------------------------*/

int dpDoJoinMitered ( Flattening_t   *fl,
                      DpSubPixel_t   *l2,
                      DpSubPixel_t   *r2
                    )
/* Not implemented as yet - just do a join bevelled: */
{  return dpDoJoinBevelled(fl,l2,r2); }

/*--------------------------------------------------------------------*/
/*                                                  dpGetOrientation  */
/*--------------------------------------------------------------------*/

/* The implementation can be found in the file 'arithmetic.s'         */

/* 'getOrientation' is a function used to identify which two points   */
/*    of the four corner points where two lines join are the 'outer'  */
/*    points that are joined. The routine is declared in private.h as */
/* int dpGetOrientation(int x1,int y1,int x2,int y2);                 */
/* Where (x1,y1) is a vector pointing from the left-hand corner of    */
/*   line 1 to the right-hand corner, and (x2,y2) is a vector         */
/*   pointing from the left hand corner of line 2 to the right hand   */
/*   corner. The routine returns a result which is >=0 if the right   */
/*   hand corner of line 1 is to be joined to the left hand corner of */
/*   line 2 as outer corners, or it returns a result which is <0 if   */
/*   the right hand corner of line 2 is to be joined to the left hand */
/*   corner of line 1.                                                */
/* The 'left-hand' corner of one end of a line is the corner which    */
/*   you would see to your left if you were standing on the end of    */
/*   the line looking outwards in the direction in which the line     */
/*   would go if it were extended from that endpoint.                 */
/* The routine needs to perform high-precision arithmetic, namely the */
/*   multiplication of two signed 32 biot numbers to get a signed 64  */
/*   bit number, and it needs to do this twice, hence it is written   */
/*   in assembler.                                                    */


