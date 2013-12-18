/*-------------------------------------------------------------*/
/*                                     ext/graphicsContexts.h  */
/*-------------------------------------------------------------*/

/* This file contains a definitions of the graphics context    */
/*    structure used to pass 'X' type graphics context         */
/*    information to this, the drawing package.                */

/*-------------------------------------------------------------*/
/*                                                  Interlock  */
/*-------------------------------------------------------------*/

#ifndef _DpGraphicsContexts_h
#define _DpGraphicsContexts_h

/*-------------------------------------------------------------*/
/*                                              Include files  */
/*-------------------------------------------------------------*/

#include "drawp/pixmaps.h"

/*-------------------------------------------------------------*/
/*                                          GraphicsContext_t  */
/*-------------------------------------------------------------*/

typedef struct DpGraphicsContext_s
{  int           depth;        /* Max depth of des pixmap        */
   int           function;     /* One of 16 functions            */
   unsigned long planeMask;    /* Which dest planes to alter     */
   unsigned long foreground;   /* Fill/Stipple colour            */
   unsigned long background;   /* Fill/Stipple colour            */
   int           fillStyle;    /* Fill/Tile/Stipp Opaque/Trans   */
   DpPixmap_t   *tile;         /* Pixmap for tiling              */
   DpPixmap_t   *stipple;      /* Pixmap for stippling           */
   int           tsXOrigin;    /* Tile/Stipple Origin : X        */
   int           tsYOrigin;    /* TIle/Stipple Origin : Y        */
   int           clipXOrigin;  /* Clip mask    Origin : X        */
   int           clipYOrigin;  /* Clip mask    Origin : Y        */
   DpPixmap_t   *clipMask;     /* Clip mask : pixmap to use      */
   int           lineWidth;    /* Width of drawn lines           */
   int           lineStyle;    /* Solid/On-Off/Double-Dash       */
   int           capStyle;     /* NotLast/Butt/Round/Projecting  */
   int           joinStyle;    /* Miter/Round/Bevel              */
   int           dashOffset;   /* Start index into dot/dash ptn  */
   int           dashLength;   /* Number of entries in dot/dash  */
   unsigned char *dashList;    /* The dot/dash list              */
   int           fillRule;     /* The fill-rule to use           */
} DpGraphicsContext_t;

/*-------------------------------------------------------------*/
/*                                      fillStyle enumeration  */
/*-------------------------------------------------------------*/

#define FillSolid           0
#define FillOpaqueStippled  1
#define FillStippled        2
#define FillTiled           3

/*-------------------------------------------------------------*/
/*                                       function enumeration  */
/*-------------------------------------------------------------*/

#define GXclear        0x0
#define GXand          0x1
#define GXandReverse   0x2
#define GXcopy         0x3
#define GXandInverted  0x4
#define GXnoop         0x5
#define GXxor          0x6
#define GXor           0x7
#define GXnor          0x8
#define GXequiv        0x9
#define GXinvert       0xA
#define GXorReverse    0xB
#define GXcopyInverted 0xC
#define GXorInverted   0xD
#define GXnand         0xE
#define GXset          0xF

/*-------------------------------------------------------------*/
/*                                      lineStyle enumeration  */
/*-------------------------------------------------------------*/

#define LineSolid      0
#define LineOnOffDash  1
#define LineDoubleDash 2

/*-------------------------------------------------------------*/
/*                        capStyle and joinStyle enumerations  */
/*-------------------------------------------------------------*/

/* These two enumerations must be mutually overloadable: */

#define NoCapOrJoin   0

#define CapNotLast    1
#define CapButt       2
#define CapRound      3
#define CapProjecting 4

#define JoinMiter     5
#define JoinRound     6
#define JoinBevel     7

/*-------------------------------------------------------------*/
/*                               Co-ordinate mode enmueration  */
/*-------------------------------------------------------------*/

/* Used for some graphics contexts: */

#define CoordModeOrigin   0
#define CoordModePrevious 1

/*-------------------------------------------------------------*/
/*                                       fillRule enumeration  */
/*-------------------------------------------------------------*/

/* Used to set the fill-rule for polygon filling: */

#define EvenOddRule 0
#define WindingRule 1

/*-------------------------------------------------------------*/
/*                                          Shape enumeration  */
/*-------------------------------------------------------------*/

/* Used to describe the shape of a polygon, and hence allow the */
/*   server to optimize to a particular polygon-fill strategy,  */
/*   the information is accepted but ignored by this package:   */

#define Convex     0
#define NonConvex  1
#define Complex    2

/*-------------------------------------------------------------*/
/*                                           End Of Interlock  */
/*-------------------------------------------------------------*/

#endif
