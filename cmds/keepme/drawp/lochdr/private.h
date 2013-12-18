/*---------------------------------------------------------------*/
/*                                             lochdr/private.h  */
/*---------------------------------------------------------------*/

/* This file contains function declarations for all the private  */
/*   routines shared by files in the drawing package             */

/*---------------------------------------------------------------*/
/*                                                    Interlock  */
/*---------------------------------------------------------------*/

#ifndef _DpPrivate_h
#define _DpPrivate_h

/*---------------------------------------------------------------*/
/*                                          Other include files  */
/*---------------------------------------------------------------*/

#include "code_interface.h"
#include "flattening.h"
#include "drawp/public.h"
#include "drawp/debug.h"

/*---------------------------------------------------------------*/
/*                          Graphics context decoding functions  */
/*---------------------------------------------------------------*/

void dpDecodeGcToBcb   (  BlitterControlBlock_t *bcb,
                          DpGraphicsContext_t   *gc,
                          int                    useFgd
                       );

void dpDecodeCopyArea  (  BlitterControlBlock_t *bcb,
                          DpPixmap_t *src, DpPixmap_t *des,
                          int srcOffX, int srcOffY,
                          DpGraphicsContext_t *gc
                       );

void dpDecodeCopyPlane (  BlitterControlBlock_t *bcb,
                          DpPixmap_t *src, DpPixmap_t *des,
                          int srcOffX, int srcOffY,
                          DpGraphicsContext_t *gc,
                          int planeNo
                       );

/*---------------------------------------------------------------*/
/*                                 Internal clip-list functions  */
/*---------------------------------------------------------------*/

/* The following function sets-up an internal default clip-node  */
/*   according to the bounding rectangle of the pixmap.          */

void dpSetDefaultClipping (  DpDefaultNode_t  *dcn,
                             DpPixmap_t       *pm
                          );

/*---------------------------------------------------------------*/
/*                     The low-level polygon blitting functions  */
/*---------------------------------------------------------------*/

int dpExecPolygons            (  BlitterControlBlock_t    *bcb,
                                 DpClipNodeEntry_t         *yp,
                                 DpPolygon_t            *plist,
                                 int                    npolys,
                                 int                      mode,
                                 int                  fillRule,
                                 int                      accr
                              );

int dpPolygonsComplexClipping (  BlitterControlBlock_t *bcb,
                                 DpPixmap_t            *des,
                                 DpPolygon_t           *plist,
                                 int                    npolys,
                                 int                    mode,
                                 int                    fillRule,
                                 int                    accr
                              );

/*---------------------------------------------------------------*/
/*                          Line-flattening control information  */
/*---------------------------------------------------------------*/

void dpDecodeLineInformation (  Flattening_t        *fl,
                                DpGraphicsContext_t *gc,
                                int                  doEven
                             );


int dpEstimatePolygonsFor   (  int x1, int y1,
                               int x2, int y2,
                               Flattening_t *fl
                            );

/*---------------------------------------------------------------*/
/*                          Internal curve-flattening functions  */
/*---------------------------------------------------------------*/

int dpGiveArcSize ( int xs, int ys,
                    int sa, int ra
                  );

void dpFlattenArc  ( int cx, int cy,
                     int xs, int ys,
                     int sa, int ra,
                     DpSubPixel_t *pnts, int *sz
                   );

/*---------------------------------------------------------------*/
/*               Cap- and Join- style curve flattening routines  */
/*---------------------------------------------------------------*/

int dpDoCapOrJoin      (  Flattening_t    *pl,
                          DpSubPixel_t    *l2,
                          DpSubPixel_t    *r2,
                          int capOrJoinStyle
                       );

int dpDoCapButt        (  Flattening_t *fl,
                          DpSubPixel_t *l2,
                          DpSubPixel_t *r2
                       );

int dpDoCapRound       (  Flattening_t *fl,
                          DpSubPixel_t *l2,
                          DpSubPixel_t *r2
                       );

int dpDoCapProjecting  (  Flattening_t *fl,
                          DpSubPixel_t   *l2,
                          DpSubPixel_t   *r2
                       );

int dpDoJoinRound      (  Flattening_t *fl,
                          DpSubPixel_t   *l2,
                          DpSubPixel_t   *r2
                       );

int dpDoJoinBevelled   (  Flattening_t *fl,
                          DpSubPixel_t   *l2,
                          DpSubPixel_t   *r2
                       );

int dpDoJoinMitered    (  Flattening_t *fl,
                          DpSubPixel_t   *l2,
                          DpSubPixel_t   *r2
                       );

int dpGetOrientation (  int x1, int y1,    /* See 'arithmetic.s' */
                        int x2, int ys
                     );

/*---------------------------------------------------------------*/
/*                              Line vector-measuring functions  */
/*---------------------------------------------------------------*/

/* These functions, used for manipulating vectors to get them to    */
/*     the required lengths, are defined as assembled routines in   */
/*     the file 'arithmetic.s'                                      */

int dpVectorLength      ( int dx, int dy );
int dpUnitComponent     ( int c1, int  l );
int dpDecodeLength      ( int  l         );
int dpNormalizeLength   ( int  l         );
int dpAddLengths        ( int l1, int l2 );
int dpMeasureComponent  ( int  l, int  u );

/*---------------------------------------------------------------*/
/*                                  Thin-line drawing functions  */
/*---------------------------------------------------------------*/

/* The following functions are provided in the file 'thinLines.c'   */
/*   for processing X-type line drawing requests on zero width      */
/*   lines. They will be branched to from the public line drawing   */
/*   primitive functions provided in 'anyLines.c' whenever they     */
/*   detect zero-width lines.                                       */

void dpDrawThinLine      (  DpPixmap_t          *des,
                            DpGraphicsContext_t *gc,
                            int                  x1,
                            int                  y1,
                            int                  x2,
                            int                  y2
                         );

void dpDrawThinSegments  (  DpPixmap_t          *des,
                            DpGraphicsContext_t *gc,
                            DpSegment_t         *seg,  /* Array of */
                            int                  qty
                         );

void dpDrawThinLines     (  DpPixmap_t          *des,
                            DpGraphicsContext_t *gc,
                            DpPoint_t           *pnts,
                            int                  npnts,
                            int                  mde   /* CoordMode... */
                         );

/*-------------------------------------------------------------------*/
/*                                     Declaration for 'dpClipLine'  */
/*-------------------------------------------------------------------*/

/* These functions are defined in the eight separate compilations of */
/*   the file 'lineQuadrant.c', except the last, which is defined in */
/*   'thinLines.c' and bracnhes to the appropriate routine according */
/*   to the quadrant in which the line lies.                         */

#define Params (LineControlBlock_t*,DpClipNodeEntry_t*,int,int,int,int,int)
void dpClipLineXpYpMx Params;
void dpClipLineXpYpMy Params;
void dpClipLineXpYnMx Params;
void dpClipLineXpYnMy Params;
void dpClipLineXnYpMx Params;
void dpClipLineXnYpMy Params;
void dpClipLineXnYnMx Params;
void dpClipLineXnYnMy Params;
void dpClipLine       Params;
#undef Params

/*-------------------------------------------------------------------*/
/*                                            The sine-lookup table  */
/*-------------------------------------------------------------------*/

/* The following is a declaration for a table generated automatically */
/*   containing the sines of various angles in the range 0..Pi/2      */
/*   radians. The manifest constant 'LogAngles' and 'Angles' define   */
/*   the size of the table: Angles = 2^LogAngles, and there will be   */
/*   (90*Angles+1) entries, corresponding to evenly distributed       */
/*   angles, the first being zero and the last being Pi/2. Each entry */
/*   is in the form of a structure containing two unsigned sixteen    */
/*   bit short integers 'hi' and 'lo' such that:                      */
/*   sin(a) = (hi * 2^-16) + (lo * 2^-32)                             */

#define LogAngles 4
#define Angles    (1<<LogAngles)

typedef struct SineValue_s
{  unsigned short hi;
   unsigned short lo;
} SineValue_t;

extern SineValue_t sineTable[90*Angles+1];

/*---------------------------------------------------------------*/
/*                                Curve-flattening computations  */
/*---------------------------------------------------------------*/

/* 'maxArcError' is set so that the maximum acceptable error as  */
/*    a contribution from the curve-flattening process may be    */
/*    2^-maxArcError. Note that further errors are incurred when */
/*    rounding the resulting quantity to the co-ordinate values  */
/*    to be passed to polygon-fill or thin-line -draw routines,  */
/*    as dictated by the values for 'Accuracy' and               */
/*    'BinaryFigures' in 'coordinates.h', but that after that no */
/*    nominal errors are introdued in chosing which pixels to    */
/*    plot.                                                      */

/* 'arcThresholds' is a look-up table of <LogAngles+5> entries   */
/*   set so that entry <i> is the maximum radius permitted to    */
/*   keep the error within range, given that the step through    */
/*   the angles table is kept to 2^i angle divisions per line.   */
/* The last entry is a termination value of -1.                  */
/* 'arcThresholds' is defined in 'objects/tables.c' which is     */
/*   generated by 'source/tableGenerator.c'.                     */

#define maxArcError 7
extern int arcThresholds[LogAngles+4+1];

/*---------------------------------------------------------------*/
/*                                             End Of Interlock  */
/*---------------------------------------------------------------*/

#endif
