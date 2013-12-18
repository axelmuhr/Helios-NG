/*-------------------------------------------------------------*/
/*                                           drawp/clipping.h  */
/*-------------------------------------------------------------*/

/* $Header: clipping.h,v 1.2 90/02/09 02:53:49 charles Locked $ */
/* $Source: /server/usr/users/charles/world/drawp/RCS/ext/clipping.h,v $ */

/* The clipping list describes a set of non-intersecting       */
/*    rectangles into which any graphics primitive may be      */
/*    clipped. A clipping list consists of a linked list of    */
/*    <n> clipping nodes.                                      */
/* A clipping node is a structure which can represent a        */
/*    variety of clipping regions finitely-limited in          */
/*    complexity by the size of the clipping node.             */
/* The graphics routines work by re-excecuting the graphics    */
/*    primitive for every clipping node, so the more clipping  */
/*    nodes on a visible region the less efficiently the       */
/*    region is plotted. Hence there is a tradeoff between     */
/*    making the clipping node size large and hence increasing */
/*    memory usage, and making clipping regions small, and     */
/*    hence decreasing speed in complex windowing situations:  */

/*-------------------------------------------------------------*/
/*                                                  Interlock  */
/*-------------------------------------------------------------*/

#ifndef _DpClipping_h
#define _DpClipping_h

/*-------------------------------------------------------------*/
/*                                    The visible region node  */
/*-------------------------------------------------------------*/

/* The size of a clipping node (which is basically just an     */
/*    array of words, is set here, and should be an absolute   */
/*    minimum of 12 words.                                     */

#define DpClipNodeLength 14

typedef int DpClipNodeEntry_t;

typedef struct DpClipNode_s
{  struct DpClipNode_s *nextNode;
   DpClipNodeEntry_t    node[DpClipNodeLength];
} DpClipNode_t;

/*-------------------------------------------------------------*/
/*                            The minimal-visible region node  */
/*-------------------------------------------------------------*/

/* The minimal visible region node is one capable of holding   */
/*    the simplest clip-region information: That corresponding */
/*    to a single node. It is used in graphics primitives to   */
/*    set-up the default clip-node when that has been          */
/*    specified.                                               */

typedef DpClipNodeEntry_t DpDefaultNode_t[12];

/* The following is the value used for a DpClipNode_t* when    */
/*    it is used to indicate 'default' such a value might for  */
/*    example be assigned to the member 'visRgn' of a          */
/*    'DpPixmap_t' structure to indicate that the clipping     */
/*    region to be used for the pixmap is to be the bounding   */
/*    box of the screen. This is the case where the primitive  */
/*    function will then set up the above-type clip-node.      */

#define DpDefaultClipping ((DpClipNode_t*)(-1))

/*-------------------------------------------------------------*/
/*                    The components of a visible-reigon node  */
/*-------------------------------------------------------------*/

/* The clipping node is an array of 32-bit integers. The node  */
/*   contains soley non-negative finite co-ordinate values     */
/*   augmented with escape values for +inifity and -infinity   */
/*   +infinity is identified by a '1' in bit 30 and -inifnity  */
/*   by a '1' in bit 31. A '1' in both bits is illegal. A '0'  */
/*   in both bits implies that the rest of the word contains   */
/*   a finite non-negative co-ordinate. The +infinity and      */
/*   -infinity values also contain a subsidiary value in the   */
/*   remainder of the word which is called the offset.         */

/* DpPosInf(i) is true if i is the +infinity escape value.     */
/* DpNegInf(i) is true if i is the -infinity escape value.     */
/* DpVisOff(i) obtains the offset in i (i is +/- inifinity)    */

#define DpPosInf(i) (((i)&(1<<30))!=0)
#define DpNegInf(i) (((i)&(1<<31))!=0)
#define DpVisOff(i) ((i)&0x3FFFFFFF)

/* DpMakePosInf(o) produces a +infinity value with embedded    */
/*    offset <o>:                                              */

#define DpMakePosInf(o) ((o)|(1<<30))
#define DpMakeNegInf(o) ((o)|(1<<31))

/*-------------------------------------------------------------*/
/*                           The structure of a clipping node  */
/*-------------------------------------------------------------*/

/* The clipping node consists of a finite sequence of          */
/*   x-vectors with y-values inserted at the beginning,        */
/*   between, and at the end of the sequence of x-vectors.     */
/* The y-values are single co-ordinates which must form a      */
/*   strictly increasing sequence starting with -infinity and  */
/*   ending with +infinity. The offsets in the infinity values */
/*   must both be the total length of the visible reigon list  */
/* An x-vector is a list of co-ordinates which also form a     */
/*   strictly increasing sequence. The first must be -infinity */
/*   and the last +infinity, and there must be an even number  */
/*   of them.                                                  */
/* The first and last x-vectors must both only contain the     */
/*   -infinity,+infinity pair.                                 */
/* The subsidiary offset values contained in the infinity      */
/*   values at either end of any x-vector must each be the     */
/*   length of that x-vector in words.                         */
/* A point is considered as visible if and only if it is       */
/*   contained in some rectangle whose x-limit co-ordinates    */
/*   are given by a pair of co-ordinates in some x-vector      */
/*   which lie an odd offset into the x-vector, and whose      */
/*   y-limit co-ordinates are given by the y-values at the     */
/*   start and end of the x-vector. The limits for a rectangle */
/*   are always considered to be exclusive to the right and    */
/*   to the bottom.                                            */

/*-------------------------------------------------------------*/
/*                                           End-of-Interlock  */
/*-------------------------------------------------------------*/

#endif


