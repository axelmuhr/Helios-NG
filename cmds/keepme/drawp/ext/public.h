/*---------------------------------------------------------------*/
/*                                                 ext/public.h  */
/*---------------------------------------------------------------*/

/* This file contains declarations of all the public functions   */
/*   provided by this file for use by others                     */

/*---------------------------------------------------------------*/
/*                                                    Interlock  */
/*---------------------------------------------------------------*/

#ifndef _DpPublic_h
#define _DpPublic_h

/*---------------------------------------------------------------*/
/*                                          Other include files  */
/*---------------------------------------------------------------*/

#include "drawp/graphicsContexts.h"
#include "drawp/pixmaps.h"
#include "drawp/coordinates.h"

/*---------------------------------------------------------------*/
/*                          Pixmap constructors and destructors  */
/*---------------------------------------------------------------*/

DpPixmap_t *dpConstructPixmap (  DpPixmap_t *pm,
                                 int         sizeX,
                                 int         sizeY,
                                 int         depth
                              );

DpPixmap_t *dpMapScreen       (  DpPixmap_t  *pm,
                                 int          subtype
                              );

void        dpDestructPixmap  (  DpPixmap_t  *pm,
                                 int          pma  /* 1 => free(pm); */
                              );

/*---------------------------------------------------------------*/
/*                                  Graphcs context constructor  */
/*---------------------------------------------------------------*/

DpGraphicsContext_t
           *dpConstructGC     (  DpGraphicsContext_t *dpGc,/*0 => new */
                                 int depth
                              );

/*---------------------------------------------------------------*/
/*                        Clip-region list processing funcitons  */
/*---------------------------------------------------------------*/

/* These functions written in the file clipping.c are used to    */
/*  create and process clip-node lists which describe areas of   */
/*  the screen that can be written into. Clip-lists are passed   */
/*  to all graphics primitives.                                  */

void          dpDestructClipList    (  DpClipNode_t *cn  );

DpClipNode_t *dpSetClipRegion       (  int lx,
                                       int ty,
                                       int rx,
                                       int by
                                    );

DpClipNode_t *dpIntersectClipList  (  DpClipNode_t      *cl,
                                      DpRectangleList_t *rl,
                                      int                fcn
                                    );

DpClipNode_t *dpReduceClipList      (  DpClipNode_t      *cl,
                                       DpRectangleList_t *rl,
                                       int                fcn
                                    );

/*---------------------------------------------------------------*/
/*                                       Line drawing primitive  */
/*---------------------------------------------------------------*/

void dpDrawLine    (   DpPixmap_t          *des,
                       DpGraphicsContext_t *gc,
                       int x1, int y1,
                       int x2, int y2
                    );

void dpDrawSegments (  DpPixmap_t          *des,
                       DpGraphicsContext_t *gc,
                       DpSegment_t         *seg,
                       int                  qty
                    );

void dpDrawLines    (  DpPixmap_t          *des,
                       DpGraphicsContext_t *gc,
                       DpPoint_t           *pnts,
                       int                  npnts,
                       int                  mde
                    );
                        
/*---------------------------------------------------------------*/
/*                                       Arc drawing primitives  */
/*---------------------------------------------------------------*/

/* First we define the accuracy with which angles are specified: */
/*    all angles should be passed in units of 2^-dpAngleAccr     */
/*    degrees:                                                   */

#define DpAngleAccr 6

/*---------------------------------------------------------------*/
/*                                  The 'FillPolygon' primitive  */
/*---------------------------------------------------------------*/

int dpFillPolygon   (  DpPixmap_t           *des,
                       DpGraphicsContext_t   *gc,
                       DpPoint_t           *pnts,
                       int                 npnts,
                       int                 shape,
                       int                  mode
                    );

/*---------------------------------------------------------------*/
/*                                     The 'Rectangle' requests  */
/*---------------------------------------------------------------*/

int  dpFillRectangle  (  DpPixmap_t          *des,
                         DpGraphicsContext_t  *gc,
                         int x,  int y,
                         unsigned int        width,
                         unsigned int        height
                      );

int  dpFillRectangles (  DpPixmap_t            *des,
                         DpGraphicsContext_t    *gc,
                         DpRectangle_t       *rects,
                         int                 nrects
                      );

/*---------------------------------------------------------------*/
/*                                             Block-operations  */
/*---------------------------------------------------------------*/

/* These can be found in the file 'blockOps.c'. They control     */
/*   block-orientated operations and the copyArea operation in   */
/*   particular is capable of dealing with scrolling.            */

void dpCopyArea     (  DpPixmap_t          *src,
                       DpPixmap_t          *des,
                       DpGraphicsContext_t  *gc,
                       int srcLftX,  int srcTopY,
                       unsigned int width,  unsigned int height,
                       int desLftX,  int desTopY
                    );

void dpCopyPlane    (  DpPixmap_t          *src,
                       DpPixmap_t          *des,
                       DpGraphicsContext_t  *gc,
                       int srcLftX,  int srcTopY,
                       unsigned int width,  unsigned int height,
                       int desLftX,  int desTopY,
                       unsigned long  plane
                    );

/*---------------------------------------------------------------*/
/*                                             End Of Interlock  */
/*---------------------------------------------------------------*/

#endif

