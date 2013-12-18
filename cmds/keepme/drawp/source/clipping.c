/*----------------------------------------------------------------*/
/*                                                    clipping.c  */
/*----------------------------------------------------------------*/

/* This file contains routines which process lists of rectangles  */
/*    and generate a clipping list for those rectangles suitable  */
/*    for placing in a graphics context and so passing to the     */
/*    graphics primitive routines.                                */

/*----------------------------------------------------------------*/
/*                                                 Include files  */
/*----------------------------------------------------------------*/

#include <stdlib.h>
#include <stddef.h>
#include <limits.h>
#include "private.h"

/*----------------------------------------------------------------*/
/*                              The structure of a clipping list  */
/*----------------------------------------------------------------*/

/* Please refer to the file 'drawp/clipping.h' */

/*----------------------------------------------------------------*/
/*                   Look-ahead declarations for local funcitons  */
/*----------------------------------------------------------------*/

static void intersectLineRecord 
             (  DpClipNode_t     **nnl,
                int               *il,
                int               llx,
                int               lrx,
                int                ty,
                int                by,
                DpClipNodeEntry_t *xv,
                DpRectangleList_t *rl,
                DpClipNodeEntry_t *av
             );

static void mergeXrecords 
             (  DpClipNode_t       **nnl,
                int                  *il,
                int                   lx,
                int                   rx,
                int                  llx,
                int                  lrx,
                int                   ty,
                int                   by,
                DpClipNodeEntry_t    *xv,
                DpRectangleList_t    *rl,
                DpClipNodeEntry_t    *av
             );

static void reduceLineRecord
             (  DpClipNode_t     **nnl,
                int               *il,
                int                ty,
                int                by,
                DpClipNodeEntry_t *xv,
                DpRectangleList_t *rl
             );

static void divideXrecords
             (  DpClipNode_t      **nnl,
                int                 *il,
                int                  lx,
                int                  rx,
                DpClipNodeEntry_t   *xv,
                int                  ty,
                int                  by,
                DpRectangleList_t   *rl
             );

static DpClipNode_t *getNewNode
             (  DpClipNode_t **cnl,int *il  );


/*----------------------------------------------------------------*/
/*                                     dpDestructClipRegion(...)  */
/*----------------------------------------------------------------*/

void dpDestructClipList(DpClipNode_t *cn)
/* This function frees the clip-node list pointed to */
{  DpClipNode_t *nn;
   if(cn==DpDefaultClipping) return;
   while(cn) { nn=cn->nextNode; free(cn); cn=nn; }
}

/*----------------------------------------------------------------*/
/*                                            dpSetClipNode(...)  */
/*----------------------------------------------------------------*/

DpClipNode_t *dpSetClipRegion(int lx,int ty,int rx,int by)
/* This routine sets up a clip-node corresponding to the single   */
/*   rectangle given by the co-ordinates supplied as parameters.  */
/* A pointer to the clip-node list generated is supplied as a     */
/*  return parameter. The parameter returned is NULL if the       */
/*  routine failed to allocate memory for the clip-region.        */
/* lx = left x (inclusive) rx = right x  (exclusive)              */
/* ty = top  y (inclusive) by = bottom y (exclusive)              */
/* All co-ordinates must be >=0.                                  */
{  DpClipNode_t *nd;

   nd = (DpClipNode_t*)calloc(1,sizeof(DpClipNode_t));
   if(nd==NULL) return NULL;
   
   nd->node[0]  = DpMakeNegInf(12);
   nd->node[1]  = DpMakeNegInf(2);
   nd->node[2]  = DpMakePosInf(2);
   nd->node[3]  = ty;
   nd->node[4]  = DpMakeNegInf(4);
   nd->node[5]  = lx;
   nd->node[6]  = rx;
   nd->node[7]  = DpMakePosInf(4);
   nd->node[8]  = by;
   nd->node[9]  = DpMakeNegInf(2);
   nd->node[10] = DpMakePosInf(2);
   nd->node[11] = DpMakePosInf(12);
   
   nd->nextNode = NULL;
   
   return nd;
}

/*----------------------------------------------------------------*/
/*                                     dpSetDefaultClipping(...)  */
/*----------------------------------------------------------------*/

void dpSetDefaultClipping (  DpDefaultNode_t  *dcn,
                             DpPixmap_t       *pm
                          )
/* This function will set up a special small clip-node of the     */
/*   type 'DpDefaultNode_t' to correspond to the bounding         */
/*   rectangle of the given pixmap.                               */
{  (*dcn)[ 0] = DpMakeNegInf(12);
   (*dcn)[ 1] = DpMakeNegInf( 2);
   (*dcn)[ 2] = DpMakePosInf( 2);
   (*dcn)[ 3] = 0;
   (*dcn)[ 4] = DpMakeNegInf( 4);
   (*dcn)[ 5] = 0;
   (*dcn)[ 6] = pm->sizeX;
   (*dcn)[ 7] = DpMakePosInf( 4);
   (*dcn)[ 8] = pm->sizeY;
   (*dcn)[ 9] = DpMakeNegInf( 2);
   (*dcn)[10] = DpMakePosInf( 2);
   (*dcn)[11] = DpMakePosInf(12);   
}   

/*----------------------------------------------------------------*/
/*                                         dpReduceClipList(...)  */
/*----------------------------------------------------------------*/

DpClipNode_t *dpReduceClipList ( DpClipNode_t      *cl,
                                 DpRectangleList_t *rl,
                                 int                fcn
                               )
/* This function is used to 'remove' the area given by the        */
/*   rectangle list from the clip-node list given by cn yielding  */
/*   a new clip-node list which is returned as a parameter. NULL  */
/*   is returned if there was any problem allocating memory. The  */
/*   orignial clip-node list <cn> is free'd if <fcn> is non-zero  */
/*   on entry. The operation essentially intersects the region    */
/*   defined by <cn> with the inverse of the union of the         */
/*   rectangles in the list <rl>. None of the co-ordinates in the */
/*   rectangle list may be less than zero.                        */
{  DpClipNode_t *nl,*nn,*cn;
   int i,j,ty,by;

   nl = (DpClipNode_t*)calloc(1,sizeof(DpClipNode_t));  
                                      /* Make first node of new list */
   if(nl==NULL) return NULL;

   nl->node[1] = DpMakeNegInf(2);      /* Start off the new list */
   nl->node[2] = DpMakePosInf(2);
   nl->nextNode = NULL;

   nn = nl; /* nn points to the last node on the newly created list */
   cn = cl; /* cn points to the next node to examine on input list  */
   
   i=j=3;   /* i points into nn, j points into cn */

   do /* Once per line record in old list */
   {  ty = cn->node[j];                  /* Get top y                */
      by = cn->node                      /* Get bottom y ...         */
          [j+1+DpVisOff(cn->node[j+1])]; /*  ... continued           */
      if(DpPosInf(by))                   /* Trap end of this node    */
      {  cn=cn->nextNode;                /*   - go to next node      */
         if(cn==NULL) break;             /* Trap list end            */
         nn->node[i++] = ty;             /* Finish output node       */
         if(getNewNode(&nn,&i)==NULL)    /* New output node ...      */
         {  dpDestructClipList(nl); return NULL; }
         j=3; continue;                  /* Get vectors from new node*/
      }       
      nn->node[i+0] = ty;                /* Write x-vector top y     */
      i+=1;                              /* Where to place x-vector  */
      reduceLineRecord(&nn,&i,ty,by,cn->node+j+1,rl);
                                         /* Process this line record */
      if(nn==NULL) {dpDestructClipList(nl); return NULL;}
                                         /* Trap memory full         */
      j=j+1+DpVisOff(cn->node[j+1]);     /* To next line record      */
   } while(cn);
   
   nn->node[i++] = ty;                   /* Finish off end of node   */
   nn->node[i++] = DpMakeNegInf(2);
   nn->node[i++] = DpMakePosInf(2);
   nn->node[0]   = DpMakeNegInf(i+1);    /* y infinity values.       */
   nn->node[i]   = DpMakePosInf(i+1);

   if(fcn) dpDestructClipList(cl);
   return nl;
}

/*----------------------------------------------------------------*/
/*                                         reduceLineRecord(...)  */
/*----------------------------------------------------------------*/

static void reduceLineRecord ( DpClipNode_t     **nnl,
                               int               *il,
                               int                ty,
                               int                by,
                               DpClipNodeEntry_t *xv,
                               DpRectangleList_t *rl
                             )

/* --------------------- Defintion of routine --------------------- */
/* *nnl points to a clip-node and *il to some location in that     */
/*    clip-node where an x-vector must be written. The x-vector    */
/*    to be written is that obtained by intersecting the x-vector  */
/*    <xv>, which is taken to have y-limits (ty;by) with the       */
/*    inverse of the union of the rectangles in the rectangle list */
/*    <rl>. In fact, this x-vector may in fact be several line-    */
/*    records in which case the intervening (but not the last)     */
/*    line-records should have their bottom-y (top-y of next       */
/*    records) correctly written. The bottom-y of the last line-   */
/*    record written is then implicitly <by>. If new clip-nodes    */
/*    need to be allocated then they can be. *nnl and *il must     */
/*    be altered to point to the end of the last x-vector written  */
/*    If there is a problem allocating space, then the routine     */
/*    should return with *nnl=NULL.                                */

/*------------ Implementation of : intersectLineRecord -------------*/

/* We discard any rectangles at the top of the list which do not   */
/*  intersect with our line record. We then divide the             */
/*  infinate plane up into three vertically stacked areas: The     */
/*  area above the rectangle on top of the list, the area in it's  */
/*  y-range, and the area below it. If the top-area intersects     */
/*  with our line-record y-limits, we pass the x-vector            */
/*  recursively down to be intersected with the negative regions   */
/*  of the remainder of the line-records, and be written into the  */
/*  output. The middle area will always intersect, and we create   */
/*  a local temporary x-vector which has the x-limit section of    */
/*  the rectangle we supplied removed, and pass that recursively   */
/*  down to be sub-divided. If the x-vector ever gets too large    */
/*  then we divide it up, and send each section down recursively   */
/*  to be sub-divided.                                             */

/*------------------ reduceLineRecord funciton --------------------*/

{  DpClipNode_t      *nn;
   int               i,j,vlx,vrx;

   /* Tail-recursion can sometimes be avoided by branching back   */
   /*   to the beggining of this function:                        */
   tailRecursion: ;
   
   /* Load vector left and right co-ordinate limits to check      */
   /*   against the rectangles:                                   */
   vlx = xv[1]; if(DpPosInf(vlx)) rl=NULL; /* Ignore all rectangles */
   vrx = xv[DpVisOff(*xv)-2];

   /* Any rectangles which do not intersect with the current line */
   /*   record do not give rise to any additions to the output    */
   /*   vector, and hence are ignored:                            */
   while( rl && (rl->ty>=by||rl->by<=ty||rl->rx<=vlx||rl->lx>=vrx) ) 
      rl=rl->next;
   
   /* Dereference nnl and il:                                     */
   nn=*nnl; i=*il;
   
   /* Detect leaves of the recursion: If there are no more        */
   /*  rectangles in the list, simply copy the vector and return  */
   if(rl==NULL)
   {  /* Make sure there is enough space to fit the vector: */
      if(i+DpVisOff(xv[0])+3>=DpClipNodeLength)
      {  getNewNode(nnl,il);
         nn=*nnl; i=*il; if(nn==NULL) return;
         nn->node[i++] = ty;
      }
      j=0; do ; while(!DpPosInf(nn->node[i++]=xv[j++]));
      *il=i;
      return;
   }
   
   /* This checks if the upper-y region above the current        */
   /*   rectangle intersects with the y-division of our line     */
   /*   record. If it does, we make a vertical division in our   */
   /*   current line-record, and we send the upper part of to be */
   /*   intersected with the rest of the rectangles in the list. */
   
   if(rl->ty>ty) 
   {  /* Vertical division and processing of the line record:    */
      reduceLineRecord(nnl,il,ty,rl->ty,xv,rl->next);
      nn = *nnl; if(nn==NULL) return;
      nn->node[(*il)++] = rl->ty; /* Add new start y for next line record */
      ty = rl->ty;
   }

   /* Now we call a routine which 'removes' the current x-region */
   /*  of the rectangle in question from the x-vector xv, and    */
   /*  passes the result down the chain in a recursion. If the   */
   /*  x-vector gets to large during division, then it is passed */
   /*  down in sections.                                         */
   if(rl->by>=by)
      divideXrecords(nnl,il,rl->lx,rl->rx,xv,ty,by,rl->next);
   else
      divideXrecords(nnl,il,rl->lx,rl->rx,xv,ty,rl->by,rl->next);
   nn=*nnl; if(nn==NULL) return;
   
   /* Now we check whether we have a remaining bottom-bit to do,  */
   /*  and if so, we simply go back to the top of this function   */
   /*  and process it there, thus avoiding tail-recursion.        */
   
   if(rl->by<by)
   {  nn->node[(*il)++] = rl->by; /* Add new start-y for next line record */
      ty = rl->by;
      rl=rl->next;
      goto tailRecursion;
   }
}
   
/*----------------------------------------------------------------*/
/*                                           divideXrecords(...)  */
/*----------------------------------------------------------------*/

static void divideXrecords( DpClipNode_t      **nnl,
                            int                 *il,
                            int                  lx,
                            int                  rx,
                            DpClipNodeEntry_t   *xv,
                            int                  ty,
                            int                  by,
                            DpRectangleList_t   *rl
                          )
/* *nnl points to a clip-node, *il points to an area in that      */
/*   node where an x-vector (or vectors) should be placed. The    */
/*   vector implicitly has y-limits (yt;yb). The x-vector to be   */
/*   placed in the output is obtained by 'removing' the x-area    */
/*   given by (xl;xr) from it, plus all the regions in the        */
/*   rectangle list <rl>. Many x-vectors may be produced, and the */
/*   protocol is the same as for 'reduceLineRecord'.              */
{  DpClipNodeEntry_t loc[DpClipNodeLength]; /* Local copy         */
   int               j;                     /* Pointer into <loc> */
   int               vlx,vrx;

   /* First: Copy the x-vector <xv> into <loc> and simultaneuosly */
   /*   'remove' the x-region (xl;xr). If <loc> ever gets too big */
   /*   send the small-enough part of it down to be sub-divided   */
   /*   by the other rectangles, then get a new line-record and   */
   /*   start filling that.                                       */
   
   /* Copy the parts of the x-vector totally to the left          */
   /*  of (xl;xr):                                                */
   xv++; /* Point to first area */
   j=1;  /* Point to first destination location */
   loc[0] = DpMakeNegInf(0); /* Set left terminator */
   do
   {  vlx = xv[0]; if(DpPosInf(vlx)) goto AllCopied;
      vrx = xv[1]; if(vrx>lx) break;
      loc[j+0] = vlx; loc[j+1] = vrx;
      j+=2; xv+=2;
   } while(1);
   
   /* Add left-portion of next record if applicable: */
   if(vlx<lx) {  loc[j+0]=vlx; loc[j+1]=lx;  j+=2; }

   /* Skip all record within region: */
   while(vrx <= rx)
   {  xv+=2;
      vlx = xv[0]; if(DpPosInf(vlx)) goto AllCopied;
      vrx = xv[1];
   }

   /* Add right-portion of next record if applicable: */
   if(vlx<rx)
   {  if((j+3)+8>=DpClipNodeLength) /* Trap vector too large */
      {  loc[j] = DpMakePosInf(j+1);
         loc[0] = DpMakeNegInf(j+1);
         reduceLineRecord(nnl,il,ty,by,loc,rl);
         if(*nnl==NULL) return;
         (*nnl)->node[(*il)++] = by; /* Write final bottom-y value */
         getNewNode(nnl,il);
         if(*nnl==NULL) return;
         (*nnl)->node[(*il)++] = ty; /* New line record. Same top y */
         j=1; loc[0] = DpMakeNegInf(0);
      }
      loc[j+0]=rx; loc[j+1]=vrx; 
      xv+=2; j+=2;
   }
   
   /* Copy remaining x-vectors: */
   do
   {  vlx = xv[0]; if(DpPosInf(vlx)) break;
      vrx = xv[1];
      if((j+2)+8>=DpClipNodeLength) /* Trap vector too large */
      {  loc[j] = DpMakePosInf(j+1);
         loc[0] = DpMakeNegInf(j+1);
         reduceLineRecord(nnl,il,ty,by,loc,rl);
         if(*nnl==NULL) return;
         (*nnl)->node[(*il)++] = by; /* Write final bottom-y value */
         getNewNode(nnl,il);
         if(*nnl==NULL) return;
         (*nnl)->node[(*il)++] = ty; /* New line record. Same top y */
         j=1; loc[0] = DpMakeNegInf(0);
      }
      loc[j+0]=vlx; loc[j+1]=vrx;
      j+=2; xv+=2;
   } while(1);
   
   AllCopied:
   loc[j] = DpMakePosInf(j+1);
   loc[0] = DpMakeNegInf(j+1);
   reduceLineRecord(nnl,il,ty,by,loc,rl);
}

/*----------------------------------------------------------------*/
/*                                      dpIntersectClipList(...)  */
/*----------------------------------------------------------------*/

DpClipNode_t *dpIntersectClipList ( DpClipNode_t      *cl,
                                    DpRectangleList_t *rl,
                                    int                fcn
                                  )
/* This function intersects the set of rectangles given by the    */
/*   rectangle list with the region already specified by the clip */
/*   node list given by cl. The new clip-node list is passed as   */
/*   a return parameter, or NULL is returned if there was a       */
/*   problem allocating memory. If the parameter <fcn> is         */
/*   non-zero, then the clip-node list <cn> originally supplied   */
/*   is cleared: this will never occur if NULL is returned.       */
/* NOTE: this particular function is virtually identical to the   */
/*    function 'reduceClipList' except that it contains a call to */
/*    'intersectLineRecord' instead of to 'reduceLineRecord'      */
{  DpClipNode_t *nl,*nn,*cn;
   DpClipNodeEntry_t nv[2];
   int i,j,k,ty,by,lx,rx;
   
   nv[0] = DpMakeNegInf(2); /* Set up null vector */
   nv[1] = DpMakePosInf(2);

   nl = (DpClipNode_t*)calloc(1,sizeof(DpClipNode_t));  
                                      /* Make first node of new list */
   if(nl==NULL) return NULL;

   nl->node[1] = DpMakeNegInf(2);      /* Start off the new list */
   nl->node[2] = DpMakePosInf(2);
   nl->nextNode = NULL;

   nn = nl; /* nn points to the last node on the newly created list */
   cn = cl; /* cn points to the next node to examine on input list  */
   
   i=j=3;   /* i points into nn, j points into cn */
   
   by = 0; /* Default value if no line-records to output */
   do /* Once per line record in old list */
   {  k=j+1+DpVisOff(cn->node[j+1]);     /* Get end-y value pointer  */
      ty = cn->node[j];                  /* Get top y                */
      by = cn->node[k];                  /* Get bottom y             */
      if(DpPosInf(by))                   /* Trap end of this node    */
      {  cn=cn->nextNode;                /*   - go to next node      */
         if(cn==NULL) break;             /* Trap list end            */
         nn->node[i++] = ty;             /* Finish output node       */
         if(getNewNode(&nn,&i)==NULL)    /* New output node ...      */
         {  dpDestructClipList(nl); return NULL; }
         j=3; continue;                  /* Get vectors from new node*/
      }       
      nn->node[i+0] = ty;                /* Write x-vector top y     */
      i+=1;                              /* Where to place x-vector  */
      if(k>j+3)                          /* Get x-limits ...         */
      {  lx=cn->node[j+2]; rx=cn->node[k-2];  }
      else lx=rx=0;
      intersectLineRecord(&nn,&i,lx,rx,ty,by,cn->node+j+1,rl,nv);
                                         /* Process this line record */
      if(nn==NULL) {dpDestructClipList(nl); return NULL;}
                                         /* Trap memory full         */
      j=k;                               /* To next line record      */
   } while(cn);
   
   nn->node[i++] = ty;               /* Finish off end of node */
   nn->node[i++] = DpMakeNegInf(2);
   nn->node[i++] = DpMakePosInf(2);

   nn->node[0]   = DpMakeNegInf(i+1); /* Write y-terminators */
   nn->node[i]   = DpMakePosInf(i+1);

   if(fcn) dpDestructClipList(cl);
   return nl;
}
      
/*----------------------------------------------------------------*/
/*                                      intersectLineRecord(...)  */
/*----------------------------------------------------------------*/

static void intersectLineRecord ( DpClipNode_t     **nnl,
                                  int               *il,
                                  int               llx,
                                  int               lrx,
                                  int                ty,
                                  int                by,
                                  DpClipNodeEntry_t *xv,
                                  DpRectangleList_t *rl,
                                  DpClipNodeEntry_t *av
                                )

/* --------------------- Defintion of routine --------------------- */

/* This routine is the analogue of reduceLineRecord except for      */
/*   intersections rather than reductions. As before, *nnl points   */
/*   to a clip-node where output is to be placed, and *il points    */
/*   to the entry in that clip-node where the first x-vector to be  */
/*   written is to be placed. This routine will compute the         */
/*   intersection of the rectangle list <rl>, the x-vector <xv>     */
/*   given y-limits (ty;by) and clipped x-wise to (llx;lrx). The    */
/*   region so computed in added (union'd) with the region given    */
/*   by the 'accumualted' x-vector <av> with the same y-limits,     */
/*   and written as a series of complete x-vectors with the correct */
/*   intervening y-values to the clip-node *nnl. If *nnl overflows  */
/*   new clip-nodes are allocated as necassary.                     */

{  DpClipNode_t *nn;
   int           i,j;
   
   tailRecursion: ; /* Branch back here to avoid tail-recursion */
   
   nn=*nnl; i=*il; /* Dereferencing */
   
   /* Any rectangles which do not intersect with the current line */
   /*   record do not give rise to any additions to the output    */
   /*   vector, and hence are ignored:                            */
   while( rl && (rl->ty>=by||rl->by<=ty||rl->rx<=llx||rl->lx>=lrx) ) 
      rl=rl->next;
   
   if(rl==NULL)
   {  /* Leaf detection occurs here: <av> is copied to the output */
      /*   and the routine returns.                               */
      if(i+DpVisOff(*av)+4>=DpClipNodeLength) /* Check for overflow */
      {  if(getNewNode(nnl,il)==NULL) return;
         nn=*nnl; nn->node[(*il)++]=ty; i=*il;
      }
      j=0; do ; while(!DpPosInf(nn->node[i++]=av[j++]));
      *il=i; return;
   }
   
   /* Check for a vertical division of al at the top of the current */
   /*   rectangle:                                                  */
   
   if(rl->ty>ty)
   {  intersectLineRecord(nnl,il,llx,lrx,ty,rl->ty,xv,rl,av);
      nn=*nnl; if(nn==NULL) return; /* Trap memory full */
      nn->node[(*il)++]=rl->ty; i=*il;
      ty=rl->ty;
   }
   
   /* Now process the horizontal division by the current rectangle: */
   
   if(rl->by<by)
      mergeXrecords(nnl,il,rl->lx,rl->rx,llx,lrx,ty,rl->by,xv,rl->next,av);
   else
      mergeXrecords(nnl,il,rl->lx,rl->rx,llx,lrx,ty,by,xv,rl->next,av);
   nn=*nnl; if(nn==NULL) return; /* Trap memory full */
   
   /* Now process the lower division by branching back to the top   */
   /*   (avoids tail recursion)                                     */
   if(rl->by<by)
   {  nn->node[(*il)++] = rl->by;
      ty=rl->by; rl=rl->next; 
      goto tailRecursion; 
   }
}

/*----------------------------------------------------------------*/
/*                                             mergeXrecord(...)  */
/*----------------------------------------------------------------*/

static void mergeXrecords ( DpClipNode_t       **nnl,
                            int                  *il,
                            int                   lx,
                            int                   rx,
                            int                  llx,
                            int                  lrx,
                            int                   ty,
                            int                   by,
                            DpClipNodeEntry_t    *xv,
                            DpRectangleList_t    *rl,
                            DpClipNodeEntry_t    *av
                          )
/* This function is the analogue of the function 'divideXrecord'   */
/*   for reducing clip-nodes. This function will add  the x-record */
/*   <av> to the intersection of the x-limits (lx;rx) and the      */
/*   general limits (llx;lrx) (which must evaluate to a non-null   */
/*   region) with the x-vector <xv>, and pass the result           */
/*   recursively to the above function intersectLineRecords with   */
/*   the rectangle list <rl> so that eventually <av> is added to   */
/*   the intersection between <rl>, and <xv> plus (lx;rx) and <xv> */
/*   all clipped to (llx;lrx), and written to the output clip-     */
/*   nodes. If the line record accumulated from <av> ever looks    */
/*   like getting too large, it is divided up horizontally, and    */
/*   each section is passed down the recursion separately, which   */
/*   is where the limits (llx;lrx) come in.                        */
{  DpClipNodeEntry_t aav[DpClipNodeLength]; /* Result of accumulation */
   DpClipNodeEntry_t *xvp;                  /* Points into xv.        */
   int alx,arx,vlx,vrx,i,dlx,drx;           /* i points into aav      */

   i=1;   /* Initialize for writing into aal */
   
   /* First clip (lx;rx) against (llx;lrx):                        */
   if(lx<llx) lx=llx;
   if(rx>lrx) rx=lrx;
   
   /* The following code merges the result of intersecting <xv>    */
   /*   with (lx;rx) with the vector <av>.                         */
   
   av++; alx=av[0]; if(DpPosInf(alx)) alx=arx=INT_MAX; else arx=av[1],av+=2;
   xvp=xv+1; do 
   {  vlx=xvp[0]; if(DpPosInf(vlx)) { vlx=vrx=INT_MAX; break; }
      vrx=xvp[1]; xvp+=2;
   } while(vrx<lx);
   if(lx>vlx)vlx=lx;  if(rx<vrx)vrx=rx;
   
   /* Now initial records from xv and al have been read. The code  */
   /*   below works by holding the next two records from each      */
   /*   vector in the variables (vlx;vrx) and (alx;arx). At any    */
   /*   particular time, the 'dominant' vector is the one with     */
   /*   it's record lying further to the left. The routine works   */
   /*   by trying to accumualte the longest possible unbroken      */
   /*   record by merging records from the two lists. dlx will     */
   /*   hold the left-value of the current record to be output.    */
   /* Note we are guaranteed that none of the new records we read  */
   /*   from vx are to the left of (lx;rx)                         */
   
   /*-------------------- Merging process -------------------------*/
   
   while( vlx<INT_MAX || alx<INT_MAX )  /* Once per record output */
   {  if(alx>=vlx)
      {  dlx=vlx;
         XVdominant:
         if(vrx>=alx)
         {  /* xv is dominant and (vlx;vrx) intersects (alx;arx) */
            if(vrx>=arx)
            {  /* (vlx;vrx) fully contains (alx;arx) */
               alx=av[0];
               if(!DpPosInf(alx)) arx=av[1],av+=2; else alx=arx=INT_MAX; 
               goto XVdominant;
            } else
            {  /* (vlx;vrx) and (alx;arx) overlap. */
               vlx=av[0];
               if(!DpPosInf(vlx)&&vlx<rx) 
               {  vrx=xvp[1]; xvp+=2;
                  if(lx>vlx)vlx=lx; if(rx<vrx)vrx=rx; 
               } else vlx=vrx=INT_MAX;
               goto AVdominant;
            }
         } else
         {  /* (vlx;vrx) is disjoint from (alx;arx). */
            vlx=xvp[0]; drx=vrx;
            if(!DpPosInf(vlx)&&vlx<rx)
            {  vrx=xvp[1]; xvp+=2;
               if(lx>vlx)vlx=lx; if(rx<vrx)vrx=rx;
            } else vlx=vrx=INT_MAX;
         }
      } else
      {  dlx=alx;
         AVdominant:
         if(arx>=vlx)
         {  /* av is dominant and (alx;arx) intersects (vlx;vrx) */
            if(arx>=vrx)
            {  /* (alx;arx) fully contains (vlx;vrx) */
               vlx=xvp[0];
               if(!DpPosInf(vlx))
               {  vrx=xvp[1]; xvp+=2;
                  if(lx>vlx)vlx=lx; if(rx<vrx)vrx=rx; 
               } else vlx=vrx=INT_MAX;
               goto AVdominant;
            } else
            {  /* (alx;arx) and (vlx;vrx) overlap. */
               alx=av[0];
               if(!DpPosInf(alx)) arx=av[1],av+=2; else alx=arx=INT_MAX;
               goto XVdominant;
            }
         } else
         {  /* (alx;arx) is disjoint from (vlx;vrx). */
            alx=av[0]; drx=arx;
            if(!DpPosInf(alx)) arx=av[1],av+=2; else alx=arx=INT_MAX; 
         }
      }
      /* Here to output the vector (dlx;drx) */
      if((i+1)+8>=DpClipNodeLength)
      {  /* Vector already full, process lhs and get new node for rhs: */
         aav[0]=DpMakeNegInf(i+1); aav[i]=DpMakePosInf(i+1);
         intersectLineRecord(nnl,il,llx,dlx,ty,by,xv,rl,aav);
         if(*nnl==NULL) return;
         (*nnl)->node[(*il)++]=by;
         if(getNewNode(nnl,il)==NULL) return;
         (*nnl)->node[(*il)++]=ty;
         llx=dlx; i=1;
      }
      aav[i++]=dlx; aav[i++]=drx;
   }
   
   /* We now have a merged line record to pass down through the   */
   /*    recursion.                                               */
   aav[0]=DpMakeNegInf(i+1); aav[i]=DpMakePosInf(i+1);
   intersectLineRecord(nnl,il,llx,lrx,ty,by,xv,rl,aav);
}
      

/*----------------------------------------------------------------*/
/*                                               getNewNode(...)  */
/*----------------------------------------------------------------*/

static DpClipNode_t *getNewNode(DpClipNode_t **cnl,int *il)
/* Clean-up the existing clip-node by writing the correct initial */
/*   -infinity with offset value, and the final -inf,+inf x-vector*/
/*   and the final +infinity y-value, link on a new clip node and */
/*   point *cnl to it, and return it as a parameter. Fill in the  */
/*   initial -inf,+inf x-vector in the correct position and set   */
/*   *ip to 3, so as to point to the location to store the first  */
/*   finite y-value. On entry *ip mut point to the first location */
/*   after the last y-value on the old clip-node.                 */
/* If the new clip-node cannot be calloc'd, then set *cnp to NULL,*/
/*   and return NULL.                                             */
{  DpClipNode_t *cn;
   int i;
   
   cn=*cnl; i=*il;
   
   cn->node[i++] = DpMakeNegInf(2);    /* Final x-vector          */
   cn->node[i++] = DpMakePosInf(2);

   cn->node[0]   = DpMakeNegInf(i+1);  /* First and last y-values */
   cn->node[i]   = DpMakePosInf(i+1);

   *cnl=(DpClipNode_t*)calloc(1,sizeof(DpClipNode_t));
   if(*cnl==NULL) return NULL;
   
   cn->nextNode = *cnl;
   cn = *cnl;
   
   cn->node[1] = DpMakeNegInf(2);  /* First x-vector */
   cn->node[2] = DpMakePosInf(2);
   *il = 3;
   return cn;
}
