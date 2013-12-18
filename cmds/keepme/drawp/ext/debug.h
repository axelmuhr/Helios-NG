/*---------------------------------------------------------------*/
/*                                                drawp/debug.h  */
/*---------------------------------------------------------------*/

/* #include this file to make use of the debugging routines      */
/*   supplied in the draw package                                */

#include <stdio.h>

/*---------------------------------------------------------------*/
/*                                               Debugging flag  */
/*---------------------------------------------------------------*/

/* Set to 0 to disable debugging messages  */
extern int dpDebug;

/*---------------------------------------------------------------*/
/*                                          Debugging utilities  */
/*---------------------------------------------------------------*/

/* The following functions are usually found in 'debug.c' and    */
/*   will printout diagnostics and self-check information.       */

int dpExamineClipList(DpClipNode_t *cn,FILE *fd);
void dpPrintRectList(DpRectangleList_t *rl,FILE *fd);

/*---------------------------------------------------------------*/
/*                               Access to arithmetic functions  */
/*---------------------------------------------------------------*/

/* These daclarations allow the test programs access to the      */
/*   internal arithmetic functions in order to test them.        */

int dpVectorLength      ( int dx, int dy );
int dpUnitComponent     ( int c1, int  l );
int dpDecodeLength      ( int  l         );
int dpNormalizeLength   ( int  l         );
int dpAddLengths        ( int l1, int l2 );
int dpMeasureComponent  ( int  l, int  u );

/*---------------------------------------------------------------*/
/*                                         Debugging primitives  */
/*---------------------------------------------------------------*/

/* These are low-level development routines written to enable    */
/*   testing of the blitter. No obscuration is performed.        */

void         dpUnClippedRect  (  DpGraphicsContext_t *gc,
                                 DpPixmap_t          *des,
                                 int                  lftX,
                                 int                  topY,
                                 int                  rgtX,
                                 int                  botY
                              );
                              
void         dpUnClippedCopy  (  DpPixmap_t          *src,
                                 DpPixmap_t          *des,
                                 int                  srcLftX,
                                 int                  srcTopY,
                                 int                  desLftX,
                                 int                  desTopY,
                                 int                  blkSizX,
                                 int                  blkSizY,
                                 DpGraphicsContext_t *gc
                              );



