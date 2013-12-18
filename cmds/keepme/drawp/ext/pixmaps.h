/*--------------------------------------------------------------------*/
/*                                                     ext/pixmaps.h  */
/*--------------------------------------------------------------------*/

/* $Header: pixmaps.h,v 1.2 90/02/09 02:56:37 charles Locked $ */
/* $Source: /server/usr/users/charles/world/drawp/RCS/ext/pixmaps.h,v $ */

/* This file describes the structure of a pixel-map information block */
/*   which describes where in memory the pixel map is stored, how it  */
/*   is stored, how many bit-planes it posseses, it's width and height*/
/*   and whether it is the screen pixel map or not                    */

/*--------------------------------------------------------------------*/
/*                                                         Interlock  */
/*--------------------------------------------------------------------*/

#ifndef _DpPixmaps_h
#define _DpPixmaps_h

/*--------------------------------------------------------------------*/
/*                                               Other include files  */
/*--------------------------------------------------------------------*/

#include <stdio.h>
#include "drawp/clipping.h"

/*--------------------------------------------------------------------*/
/*                                             Structure of a Pixmap  */
/*--------------------------------------------------------------------*/

/* As far as this drawing package is concerned, to each pixmap there  */
/*    exists an object of structure 'DpPixmap_t' (defined below), and */
/*    this structre contains a pointer to an area of memory where the */
/*    pixel map raw data itself is stored. In fact, this points to    */
/*    a point 4 bytes earlier than the start of the raw data. This is */
/*    beacuse, when each pix-map is allocated, 4 bytes are allocated  */
/*    on either side of the raw data to allow for slightly messy      */
/*    behaviour of the low-level blitting routines.                   */

/* The pixel map raw data consists of <number of rows> contigously    */
/*    stored equally sized blocks, each corresponding to a row of the */
/*    pixmap starting from the top, and going down. Each block        */
/*    consists of <number of bit planes> equally sized blocks called  */
/*    vectors. The first vector corresponds to the bit-plane          */
/*    corresponding to the least significant bit of a colour, and     */
/*    so-on. Each vector is essentially a huge bit-field corresponding*/
/*    to the row of that bit-plane, and may have any amount of        */
/*    redundant bits at the right hand end. So, to access bit <c> of  */
/*    the colour of the pixel stored at location (x,y) of a pixel map */
/*    you should access the <y>th block, the <c>th vector in that     */
/*    block, the <x div 32>th word in that vector, and look at bit    */
/*    number <x mod 32> where the least significant bit is bit 0      */

/*--------------------------------------------------------------------*/
/*                                                          DpMaxBpp  */
/*--------------------------------------------------------------------*/

/* The maximum depth of a pixmap is defined here */

#define DpMaxBpp 8

/* NOTE : If a depth of 31 is exceeded, the routines in rectangles.s  */
/*   will not work. Also, the value above of DpMaxBpp MUST BE KEPT IN */
/*   STEP with the value defined in <lochdr/structures.ss>            */

/*--------------------------------------------------------------------*/
/*                                          The DpPixmap_t structure  */
/*--------------------------------------------------------------------*/

/* The member <scrFd> is set to NULL for memory pix-maps, and to the  */
/*   file pointer for the screen if this is the pixmap which maps the */
/*   screen.                                                          */

typedef struct DpPixmap_s
{  FILE*   scrFd;   /* NULL if memory pix-map        */
   int     depth;   /* Number of bit-planes          */
   int     sizeX;   /* Width                         */
   int     sizeY;   /* Height                        */
   int     wpv;     /* Words per vector              */
   void   *rawBase; /* pointer to raw data (less 4)  */
   DpClipNode_t *visRgn;
                    /* Clipping for this pixmap      */
} DpPixmap_t;

/*--------------------------------------------------------------------*/
/*                                                  End Of Interlock  */
/*--------------------------------------------------------------------*/

#endif

