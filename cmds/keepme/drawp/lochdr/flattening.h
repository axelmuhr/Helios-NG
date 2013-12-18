/*------------------------------------------------------------*/
/*                                              flattening.h  */
/*------------------------------------------------------------*/

/* $Header: flattening.h,v 1.1 90/03/03 18:01:18 charles Locked $ */
/* $Source: /server/usr/users/charles/world/drawp/RCS/lochdr/flattening.h,v $ */

/* This file contains definitions of structures used to       */
/*     control the process of flattening geometric images,    */
/*     ie converting them to lists of points defining the     */
/*     boundary of a polygon.                                 */

/*------------------------------------------------------------*/
/*                                                 Interlock  */
/*------------------------------------------------------------*/

#ifndef _DpFlattening_h
#define _DpFlattening_h

/*------------------------------------------------------------*/
/*                                             Include files  */
/*------------------------------------------------------------*/

#include "drawp/coordinates.h"

/*------------------------------------------------------------*/
/*                                              Flattening_t  */
/*------------------------------------------------------------*/

/* The following is a locally used structure used to control  */
/*    the processing of lines:                                */

/* The <strokeLength> is the length of the next stroke in the */
/*    lines to be plotted, <isDrawStroke> is non-zero if that */
/*    stroke is to be drawn this go round. <strokeList> is    */
/*    a pointer to the list of dot-dash lengths, and          */
/*    <strokePosn> is the offset within that list where the   */
/*    next stroke length is to be read. <strokeListSize> is   */
/*    the number of entries in the vector pointed to by       */
/*    <strokeList> so <strokePosn> has to be reduced to be in */
/*    in the range 0..<strokeListSize-1>. <strokePeriod> is   */
/*    the sum of the values in the strokeList.                */
/* If <strokeList> is null, then the line-style is assumed to */
/*    be LineSolid, and the other stroke information need not */
/*    be valid, except the <isDrawStroke> must be 1.          */
/*  <polyVector> is the pointer into the vector of polygons   */
/*    where the next stroke (dot or dash or join style or cap */
/*   style) polygon is to be written.                         */
/* <prvLft> and <prvRgt> are the end-points of the previous   */
/*    line                                                    */
/* <lineWidth> is in sub-pixel units                          */

typedef struct Flattening_s
{  int              strokeLength;   /* Sub-pixel units        */
   unsigned int     isDrawStroke;   /* Boolean                */
   unsigned char   *strokeList;     /* List of unsigned chars */
   unsigned int     strokeIndex;    /* Offset in above list   */
   unsigned int     strokeListSize; /* Length of above list   */
   unsigned int     strokePeriod;   /* Rpt len of dot/dashes  */
   int              lineWidth;      /* Width of lines         */
   DpPolygon_t     *polyVector;     /* Where to put next poly */
   DpSubPixel_t     prvLft,prvRgt;  /* End pts prev. line     */
} Flattening_t;

/*------------------------------------------------------------*/
/*                                          End of Interlock  */
/*------------------------------------------------------------*/

#endif
