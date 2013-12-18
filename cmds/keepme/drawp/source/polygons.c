/*-----------------------------------------------------------------------*/
/*                                                           polygons.c  */
/*-----------------------------------------------------------------------*/

/* $Header: polygons.c,v 1.8 90/07/11 15:37:11 charles Locked $ */
/* $Source: /server/usr/users/a/charles/world/drawp/RCS/source/polygons.c,v $ */

/* This file contains code to plot filled polygons. A routine in this   */
/*   file will accept any finite list of polygons, each expressed as a  */
/*   finite list of vertices, and will plot there interior according to */
/*   any given blitter-control-block graphics context, and through a    */
/*   given clip-list.                                                   */

/*-----------------------------------------------------------------------*/
/*                                                        Include files  */
/*-----------------------------------------------------------------------*/

#include <stddef.h>
#include <stdlib.h>
#include <limits.h>
#include "private.h"

/*-----------------------------------------------------------------------*/
/*                                     Abbreviation for 'BinaryFigures'  */
/*-----------------------------------------------------------------------*/

#define F(x) ((x)&(Accuracy-1))
#define I(x) ((x)>>BinaryFigures)
#define M(x) ((x)<<BinaryFigures)

/*-----------------------------------------------------------------------*/
/*                                        Internal structure : The edge  */
/*-----------------------------------------------------------------------*/

typedef struct Edge_s
{  short           x,y;
   short           ey;
   unsigned short  signs;
   unsigned int    dx,dy;
   int             Bn;
   struct Edge_s  *next;
} Edge_t;

#define DYneg  ((unsigned short)1)
#define DXneg  ((unsigned short)2)


/*-----------------------------------------------------------------------*/
/*                          Internal routines : Look-ahead declarations  */
/*-----------------------------------------------------------------------*/

static Edge_t *copyAndSort  (  DpPolygon_t    **pl,
                               int             *il,
                               int              vq,
                               Edge_t          *el,
                               int   ty , int   by,
                               int *rlx , int *rly,
                               int mode , int accr
                            );

static Edge_t *copyLeaf     (  DpPolygon_t    **pl,
                               int             *il,
                               Edge_t          *el,
                               int   ty , int   by,
                               int *rlx , int *rly,
                               int mode , int accr
                            );

static void processEdgeList (  Edge_t               **ael,
                               Edge_t                **el,
                               int                      y
                            );

static void sortEdgeList    (  Edge_t               **ael  );

static void displayScanLine (  BlitterControlBlock_t *bcb,
                               DpClipNodeEntry_t      *xp,
                               Edge_t                *aep,
                               int                      y,
                               int               fillRule
                            );

/*-----------------------------------------------------------------------*/
/*                                                   dpFillPolygon(...)  */
/*-----------------------------------------------------------------------*/

int dpFillPolygon ( DpPixmap_t           *des,
                    DpGraphicsContext_t   *gc,
                    DpPoint_t           *pnts,
                    int                 npnts,
                    int                 shape,
                    int                  mode
                  )
/* Public routine to implement fill of single polygon. This routine will */
/*    return a zero value if there was a problem allocating memory       */
{  BcbContainer_t       bcbc;
   DpPolygon_t            pl;
   DpClipNode_t          *cn;
   DpDefaultNode_t       dcn;
   
   shape = Complex;  /* Always assume complex shape */
   
   /* Set up a one-polygon fill area: */
   pl.npnts        = npnts;
   pl.pnts.coarse  =  pnts;
   
   dpDecodeGcToBcb(&bcbc.bcbProper,gc,1);
   bcbc.bcbProper.desBase = (char*)des->rawBase + 4;
   bcbc.bcbProper.desBPP  = des->depth;
   bcbc.bcbProper.desWPV  = des->wpv;
   
   if((cn=des->visRgn)==DpDefaultClipping)
   {  dpSetDefaultClipping(&dcn,des);
      if(!dpExecPolygons(&bcbc.bcbProper,dcn,&pl,1,mode,gc->fillRule,0)) return 0;
   } else
   {  for(;cn;cn=cn->nextNode)
         if(!dpExecPolygons(&bcbc.bcbProper,cn->node,&pl,1,mode,gc->fillRule,0)) return 0;
   }
   
   return 1;
}

/*-----------------------------------------------------------------------*/
/*                                       dpPolygonsComplexClipping(...)  */
/*-----------------------------------------------------------------------*/

int dpPolygonsComplexClipping ( BlitterControlBlock_t *bcb,
                                DpPixmap_t            *des,
                                DpPolygon_t           *plist,
                                int                    npolys,
                                int                    mode,
                                int                    fillRule,
                                int                    accr
                              )
/* Like dpExecPolygons below, but accepts an entire clip-list for        */
/*    clipping, and it could have the default value. Works by repeated   */
/*    calls to dpExecPolygons, like the above routine.                   */
/* The visRgn is taken from the pixmap <des>, and if it is the default   */
/*     clip-region, the default clip-node is set up from the boundaries  */
/*     of the given pixmap.                                              */
{  DpClipNode_t          *cn;
   DpDefaultNode_t       dcn;
   
   if((cn=des->visRgn)==DpDefaultClipping)
   {  dpSetDefaultClipping(&dcn,des);
      if(!dpExecPolygons(bcb,dcn,plist,npolys,mode,fillRule,0)) return 0;
   } else
   {  for(;cn;cn=cn->nextNode)
         if(!dpExecPolygons(bcb,cn->node,plist,npolys,mode,fillRule,accr)) 
            return 0;
   }
   
   return 1;
}

/*-----------------------------------------------------------------------*/
/*                                                  dpExecPolygons(...)  */
/*-----------------------------------------------------------------------*/

int dpExecPolygons ( BlitterControlBlock_t *bcb,
                     DpClipNodeEntry_t      *yp,
                     DpPolygon_t           *plist,
                     int                    npolys,
                     int                    mode,
                     int                    fillRule,
                     int                    accr
                   )
/* Decompose the polygons in the list supplied into a non-intersecting   */
/*   set of horizontal lines, intersect these with the clipping region   */
/*   defined by the clip-node <yp>, and plot the results using the       */
/*   blitter control block <bcb>. Note that only one clip-node is used:  */
/*   if there is a clip-list of more than one clip-node, the routine     */
/*   must be re-executed for every clip-node on the list.                */
/* <fillRule> must be set to 'EvenOddRule' or 'WindingRule'              */
/* This routine returns 0 if there was a problem allocating memory       */
/* If the parameter 'accr' is 1 then the parameter <plist> is a list of  */
/*   pointers to polygons whose vertices are in units of 1/Accuracy      */
/*   pixel widths, whereas if 'accr' is zero, then the points are in     */
/*   units of one pixel width. The member 'pnts' of 'Polygon_t' defined  */
/*   in 'drawp/coordinates.h' should shed some light on the matter       */
{  int                totalVertices,n,y,ty,by,rlx,rly;
   DpPolygon_t       *pl;
   Edge_t            *el,*ep,*aep;
   
   /* Compute the total number of edges (maximum) to be processed: */
   totalVertices = 0;
   for(pl=plist,n=npolys;n;n--,pl++) totalVertices+=pl->npnts;
   
   /* Allocate space to store edges: */
   el = (Edge_t*)calloc(totalVertices,sizeof(Edge_t));
   if(el==NULL) return 0;

   /* Compute the top- and bottom- y-limits of the current clip region */
   ty=yp[3]; by=yp[DpVisOff(yp[0])-4];
   
   /* Copy and sort the edge data. Edges are sorted according to their */
   /*   upper co-ordinate: first by it's y then by it's x. Horizontal  */
   /*   edges are discarded. The edges are sorted along the link field */
   /*   in the ActiveEdge structure. The routine returns a link to the */
   /*   first edge.                                                    */
   pl=plist; n=0; rlx=rly=0;
   ep=copyAndSort(&pl,&n,totalVertices,el,ty,by,&rlx,&rly,mode,accr);
   
   for(yp+=3,by=yp[DpVisOff(yp[1])+1];ep;)
   {  y = ep->y;
      aep = NULL;
      do
      {  while(by<=y)
         {  yp += DpVisOff(*++yp);
            by = yp[DpVisOff(yp[1])+1];
            if(DpPosInf(by)) goto FinishedPlot;
         }
         processEdgeList(&aep,&ep,y); 
         displayScanLine(bcb,yp+1,aep,y,fillRule);
         y++;
      } while(aep);
   }
   FinishedPlot: 
   free(el);
   return 1;
}

/*-----------------------------------------------------------------------*/
/*                                              Spec of 'copy and sort'  */
/*-----------------------------------------------------------------------*/

/* static Edge_t *copyAndSort ( DpPolygon_t    **pl,  */
/*                              int             *il,  */
/*                              int              vq,  */
/*                              Edge_t          *el,  */
/*                              int    ty , int  by,  */
/*                              int  *rlx , int *rly, */
/*                              int mode,             */
/*                              int accr              */
/*                            );                      */

/* <*pl> points to an entry in a vector of 'Polygon_t' structures, and   */
/*   <*il> is the offset of some point in that polygon. If <accr> is 1,  */
/*   the points are specified as 'DpSubPixel_t' co-ordinates to          */
/*   sub-pixel accuracy; if <accr> is 0, the points are specified to     */
/*   'DpPoint_t' pixel accuracy. The points define edges of polygons     */
/*   given by connecting consective points in the current polygon list   */
/*   and connecting the last point to the first point of the current     */
/*   polygon, then overflowing to the next polygon in the vector and     */
/*   continuing.                                                         */
/* The routine will use a recursive merge-sort to read in the next <vq>  */
/*   edges in the manner defined above, leaving <*pl> and <*il>          */
/*   referring to the next edge after that. The routine places these     */
/*   edges in arbitrary order in the <vq> entries of the vector <el>,    */
/*   and ignoring edges which do not cross any vertical pixel            */
/*   boundaries (this includes horizontal edges), and ignoring edges     */
/*   which are completely outside the vertical boundaries (ty;by), will  */
/*   link the edges into an ordered list using the link field in the     */
/*   'Edge_t' structure. The order is primarily by the y-value of the    */
/*   upper point of each edge, and secondarily by the x-value of the     */
/*   upper point of each edge. Edges whose upper point is above ty are   */
/*   clipped correctly. The fields (dx,dy) are both positive integers    */
/*   giving the gradient of the line. (x,y) taken in conjuction with Bn  */
/*   gives the precise co-ordinate of the upper-most point on the edge   */
/*   which lies on a vertical tiling boundary. This co-ordinate has      */
/*   value (x+Bn/dy,y), and Bn is in the range (-dy,0], so that Bn/dy    */
/*   has the range (-1,0]. The field 'signs' is a bit field where bit0   */
/*   is the sign bit for dy, and bit1 is the sign bit for dx.            */
/*   'ey' is the y-co-ordinate of the lowest point on the line which     */ 
/*   lies on a vertical pixel boundary, note that the line is for these  */
/*   purposes considered as exclusive in the last point, so that if the  */
/*   last point lies on a vertical pixel boundary (as it always is when  */
/*   working to non-sub-pixel accuracy), then ey is the y-co-ordinate of */
/*   that point less 1.                                                  */
/* Caveat: Those edges which are ignored because they are too horizontal */
/*   (described above), are not even copied into the <el> vector,        */
/*   although a 'hole' will be left where they should have been copied.  */
/* Note that a ploygon may have zero edges.                              */

/*-----------------------------------------------------------------------*/
/*                                                     copyAndSort(...)  */
/*-----------------------------------------------------------------------*/

static Edge_t *copyAndSort ( DpPolygon_t    **pl,
                             int             *il,
                             int              vq,
                             Edge_t          *el,
                             int   ty , int   by,
                             int *rlx , int *rly,
                             int mode , int accr
                           )
/* See above for spec */
{  int          hvq;
   Edge_t      *l0,*l1,**lnk;
   
   if(vq==0) return NULL;
   if(vq==1) return copyLeaf(pl,il,el,ty,by,rlx,rly,mode,accr);

   hvq = vq/2;
   l0 = copyAndSort(pl,il,   hvq,el    ,ty,by,rlx,rly,mode,accr);
   l1 = copyAndSort(pl,il,vq-hvq,el+hvq,ty,by,rlx,rly,mode,accr);
   
   /* Merge together two ordered lists, l0 and l1. lnk points to */
   /*    where to link the next entry onto                       */
   lnk = &el;
   for(;l0&&l1;)
   {  if (   (   l0->y > l1->y   )
          || (   l0->y==l1->y 
              && (   l0->x>l1->x
                  || (   l0->x==l1->x
                      && (  l0->Bn>l1->Bn )
                     )
                 )
             )
          )
      {  *lnk=l1; lnk=&l1->next; l1=l1->next;  }
      else
      {  *lnk=l0; lnk=&l0->next; l0=l0->next;  }
   }

   /* At the end, one or both of the input lists is exhasted, so link  */
   /*   the output list to the non-exhausted one, or either if both    */
   /*   are exhausted.                                                 */
   if(l0) *lnk=l0; else *lnk=l1;
   return el;
}

/*-----------------------------------------------------------------------*/
/*                                                        copyLeaf(...)  */
/*-----------------------------------------------------------------------*/

static Edge_t *copyLeaf (  DpPolygon_t    **pl,
                           int             *il,
                           Edge_t          *el,
                           int   ty , int   by,
                           int *rlx , int *rly,
                           int mode , int accr
                        )
{  DpPolygon_t *pe;
   int           i,sx,sy,ex,ey,dx,dy,Bn,npnts;
   
   /* Ensure we have a polygon with finite points: */
   pe=*pl; i=*il;
   while((npnts=pe->npnts)==0) *pl=++pe;
   
   if(accr) sx=pe->pnts.fine[i].x            , sy=pe->pnts.fine[i].y;
   else     sx=(int)pe->pnts.coarse[i].x<<Bf , sy=(int)pe->pnts.coarse[i].y<<Bf;
   if(++i>=pe->npnts) 
   {  *il=i=0,*pl=pe+1;
      if(mode==CoordModePrevious) {  sx+=*rlx; sy+=*rly;  *rlx=*rly=0;  }
   }
   else 
   {  *il=i;
      if(mode==CoordModePrevious) {  *rlx=(sx+=*rlx); *rly=(sy+=*rly);  }
   }
   if(accr) ex=pe->pnts.fine[i].x            , ey=pe->pnts.fine[i].y;
   else     ex=(int)pe->pnts.coarse[i].x<<Bf , ey=(int)pe->pnts.coarse[i].y<<Bf;
   if(mode==CoordModePrevious) ex+=*rlx,ey+=*rly;

   if(ey<sy) 
   {  ey^=sy,sy^=ey,ey^=sy;
      ex^=sx,sx^=ex,ex^=sx;
      el->signs=DYneg;
   }
   else
      el->signs=0;
  
   dx=ex-sx; dy=ey-sy;

   if( sy >= M(by) || ey <= M(ty) ) return NULL;
   
   if(dx<0)
   {  el->dx=dx=-dx; el->signs |= DXneg;
      Bn = F(sx) - I(dx)*(Accr-1-F(sy-1)) - I(F(dx)*(Accr-1-F(sy-1)));
      sy = I(sy-1)+1; sx=I(sx); ey = I(ey-1);
      if(sy>ey) return NULL;
      if(sy<ty) Bn-=dx*(ty-sy),sy=ty;
      sx++; Bn-=dy;
      while(Bn<=-dy) sx--,Bn+=dy;
   }
   else
   {  el->dx=dx;
      Bn = F(sx) + I(dx)*(Accr-1-F(sy-1)) + I(F(dx)*(Accr-1-F(sy-1)));
      sy = I(sy-1)+1; sx=I(sx); ey = I(ey-1);
      if(sy>ey) return NULL;
      if(sy<ty) Bn+=dx*(ty-sy),sy=ty;
      while(Bn>0) sx++,Bn-=dy;
   }

   el->x  = sx; el->y  = sy;
   el->dy = dy; el->ey = ey;
   el->Bn = Bn; el->next = NULL;
   
   return el;
}

/*-----------------------------------------------------------------------*/
/*                                                 processEdgeList(...)  */
/*-----------------------------------------------------------------------*/

static void processEdgeList ( Edge_t               **ael,
                              Edge_t                **el,
                              int                      y
                            )
/* Process the active edge list <ael> to be in a state representing the */
/*   scan-line <y> assuming it was in the state representing scan line  */
/*   <y-1> by removing expired edges, adding new edges and incrementing */
/*   existing edges to the next scan line. The routine tries to         */
/*   maintain x-order in the edge list without resorting to a non-      */
/*   linear scan through the list. If the routine detects that order    */
/*   has changed in the list, it will then call 'sort edge list' which  */
/*   will restore x-order in the list.                                  */
{  int     px,x,dx,dy,Bn,ord;
   Edge_t *aep,*aet,*ep,**aels;
   
   aels=ael; aep=*ael; ep=*el; px=INT_MIN; ord=1;
   while(1)
   {  /* Remove expired edges: */
      while(aep&&aep->ey<y) aep=*ael=aep->next;
      /* See if a new edge is to be inserted: */
      if( (ep && ep->y==y) && (!aep || aep->x>=ep->x ) )
      {  aet=*ael=ep; ep=*el=ep->next; aet->next=aep; aep=aet;  }
      /* Otherwise increment existing edge: */
      else if (aep)
      {  dx=aep->dx; dy=aep->dy;
         if(!(aep->signs&DXneg))
         {  Bn=aep->Bn+dx;
            for(x=aep->x;Bn>0;x++) Bn-=dy;
         }
         else
         {  Bn=aep->Bn-dx;
            for(x=aep->x;Bn<=-dy;x--) Bn+=dy;
         }
         /* Save back new x and Bn values: */
         aep->x=x; aep->Bn=Bn;
      }
      /* Otherwise we are at the end of the active edge list: */
      else break;
      aep=*(ael=&aep->next);
      if(x<px) ord=0;
      px=x;
   }
   if(!ord) sortEdgeList(aels);
}

/*-----------------------------------------------------------------------*/
/*                                                    sortEdgeList(...)  */
/*-----------------------------------------------------------------------*/

static void sortEdgeList ( Edge_t **ael )
/* Sort the active-edge list <*ael> so that <*ael> points to the new list */
/*   a bubble-sort is used since this routine is only used where lines    */
/*   cross, and a bubble-sort is fairly good for transpositions.          */
{  Edge_t *spp,**spl,*snp,**snl,*thisSwap,*lastSwap;

   thisSwap=NULL;
   do
   {  spp=*(spl=ael);
      lastSwap=thisSwap; thisSwap=NULL;
      do
      {  if(spp==lastSwap)                     break;
         if((snp=*(snl=&spp->next))==lastSwap) break;
         if(spp->x>snp->x) 
         {  *spl=snp; *snl=snp->next; snp->next=spp; 
            spl=&snp->next; thisSwap=spp;
         }
         else
         {  spp=snp; spl=snl;  }
      } while(1);
   } while(thisSwap);
}

/*-----------------------------------------------------------------------*/
/*                                                 displayScanLine(...)  */
/*-----------------------------------------------------------------------*/

static void displayScanLine ( BlitterControlBlock_t *bcb,
                              DpClipNodeEntry_t      *xp,
                              Edge_t                *aep,
                              int                      y,
                              int               fillRule
                            )
/* Draw the processed active edge list by sending the co-ordinates */
/*   through the blitter-control-block <bcb>. Clipping is through  */
/*   the x-vector pointed to be <xp>. The scan line number is <y>  */
/* <fillRule> must be one of 'EvenOddRule' or 'WindingRule'        */
{  int Llx,Lrx,sx,ex,fillState;

   Llx=*++xp; Lrx=*++xp; sx=0; fillState=0;
   for(;aep;aep=aep->next)
   {  ex = aep->x;
      if(fillState)
      {  if(DpPosInf(Llx)) goto PlotFinished;
         while(Lrx<=sx) 
         {  Llx=*++xp,Lrx=*++xp; if(DpPosInf(Llx)) goto PlotFinished;  }
         if(Llx<=sx&&Lrx>=ex)
         {  bcb->desLftX=sx; bcb->desRgtX=ex;
            bcb->desTopY=y; bcb->desBotY=y+1;
            bcb->ctlRtn(bcb);
         }
         else
         {  if(Llx<sx)
            {  bcb->desLftX=sx; bcb->desRgtX=Lrx;
               bcb->desTopY=y; bcb->desBotY=y+1;
               bcb->ctlRtn(bcb);
               Llx=*++xp,Lrx=*++xp; if(DpPosInf(Llx)) goto PlotFinished;
            }
            while(Lrx<=ex)
            {  bcb->desLftX=Llx; bcb->desRgtX=Lrx;
               bcb->desTopY=y; bcb->desBotY=y+1;
               bcb->ctlRtn(bcb);
               Llx=*++xp,Lrx=*++xp; if(DpPosInf(Llx)) goto PlotFinished;
            }
            if(Llx<ex)
            {  bcb->desLftX=Llx; bcb->desRgtX=ex;
               bcb->desTopY=y; bcb->desBotY=y+1;
               bcb->ctlRtn(bcb);
            }
         }
         PlotFinished:;
      }
      /* Shift just-processed edge: */
      sx=ex; 
      if(fillRule==EvenOddRule) fillState ^= 1;
      else                      fillState += (aep->signs&DYneg)?-1:1;
   }
}

