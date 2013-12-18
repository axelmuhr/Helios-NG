/*-------------------------------------------------------------*/
/*                                        drawp/coordinates.h  */
/*-------------------------------------------------------------*/

/* $Header: coordinates.h,v 1.6 90/03/03 17:59:28 charles Locked $ */
/* $Source: /server/usr/users/charles/world/drawp/RCS/ext/coordinates.h,v $ */

/* This file defines the various graphics structures used to   */
/*   communicate graphics requests between the draw package    */
/*   and other modules.                                        */

/*-------------------------------------------------------------*/
/*                                                  Interlock  */
/*-------------------------------------------------------------*/

#ifndef _DpCoordinates_h
#define _DpCoordinates_h

/*-------------------------------------------------------------*/
/*                                                    DpPoint  */
/*-------------------------------------------------------------*/

/* This is for specifying points with pixel-width accuracy:    */

typedef struct DpPoint_s
{  short x;
   short y;
} DpPoint_t,DpPoint;

/* This is for specifying points to sub-pixel accuracy using a */
/*   fixed-point binary number, such specifications are only   */
/*   used internally for the moment:                           */

typedef struct DpSubPixel_s
{  int x;
   int y;
} DpSubPixel_t, DpSubPixel;

/* The following defines the number of bits of sub-pixel accuracy   */
/*   is used internally to specify co-ordinates when flattening     */
/*   circles and so-on. The accuracy is specified in bits, and must */
/*   be any value from 1 to 10 inclusive, the upper limit is to     */
/*   avoid overflow when performimg computations.                   */

#define BinaryFigures 10
#define Accuracy (1<<BinaryFigures)

/* The following are abbreviations: */

#define Bf   BinaryFigures
#define Accr Accuracy

/*-------------------------------------------------------------*/
/*                                                  DpSegment  */
/*-------------------------------------------------------------*/

/* The end-points of a line-segment: */

typedef struct DpSegment_s
{  DpPoint_t p;
   DpPoint_t q;
} DpSegment_t,DpSegment;

/*-------------------------------------------------------------*/
/*                                              DpRectangle_t  */
/*-------------------------------------------------------------*/

typedef struct DpRectangle_s
{  short                   x,y;
   unsigned short width,height;
} DpRectangle,DpRectangle_t;

/*-------------------------------------------------------------*/
/*                                          DpRectangleList_t  */
/*-------------------------------------------------------------*/

/* A single entry on a linked list of rectangles, used for     */
/*   passing clip-regions:                                     */

typedef struct DpRectangleList_s
{  int   lx,ty,rx,by;
   struct DpRectangleList_s *next;
} DpRectangleList_t;

/*-------------------------------------------------------------*/
/*                                                DpPolygon_t  */
/*-------------------------------------------------------------*/

/* A polygon is simply a list of vertices, the vertices may be */
/*   specified to sub-pixel accuracy or only pixel-width       */
/*   accuracy, the exact choice being determined by context,   */
/*   this structure at present only used internally:           */

typedef struct DpPolygon_s
{  int         npnts;
   union
   {  DpPoint_t  *coarse;          /* An array of length <npnts>   */
      DpSubPixel *fine;            /* Points to sub-pixel accuracy */
   } pnts;
} DpPolygon_t,DpPolygon;

/*-------------------------------------------------------------*/
/*                                           End of Interlock  */
/*-------------------------------------------------------------*/

#endif

