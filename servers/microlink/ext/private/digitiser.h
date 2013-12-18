/* $Source: /server/usr/users/b/charles/world/microlink/RCS/ext/private/digitiser.h,v $ */
/* $Header: digitiser.h,v 1.2 91/01/31 13:50:38 charles Locked $ */

/*------------------------------------------------------------------------*/
/*                                         microlink/private/digitiser.h  */
/*------------------------------------------------------------------------*/

/* This file contains declarations which are specific to the way in which */
/*   the "digitiser" part of the microlink servers is implemented.        */
/* The public part "microlink/digitiser.h" contains definitions which are */
/*   useful for clients of the "digitiser" part of the microlink server:  */
/*   To be more specific, client of the "digitiser" server need to have   */
/*   access to the structures with which to communicate via. GetInfo()    */
/*   and SetInfo() calls on the digitiser stream.                         */

/*------------------------------------------------------------------------*/
/*                                                             Interlock  */
/*------------------------------------------------------------------------*/

#ifndef MicrolinkPrivateDigitiser_h
#define MicrolinkPrivateDigitiser_h

/*------------------------------------------------------------------------*/
/*                                                          Header Files  */
/*------------------------------------------------------------------------*/

#include "microlink/digitiser.h"

/*------------------------------------------------------------------------*/
/*                                        Raw stylus information packets  */
/*------------------------------------------------------------------------*/

/* The microcontroller send over the microlink a series of alternating    */
/*   (between x- and y-) packets of information conatining raw A/D values */
/*   which can be used to determine the position of the stylus. The       */
/*   following desribes the structure of these packets and various        */
/*   associated conatants.                                                */

/* The message-type if the stylus-information messages. The byte has the  */
/*   bottom two length bits masked out, so the byte that would be         */
/*   received as message header actually has value '9F' hex.              */
#define MLEstylu            0x9C

typedef struct MLEstyluPacket /* Structure of stylus packet body ...      */
{  ubyte         hdr;       /* Header byte : uL frame information         */
   ubyte         len;       /* Length byte : uL frame information         */
   ubyte         ord;       /* 0=>x-ordinate 255=>y-ordinate              */
   signed char  w[5];       /* Signed wire values.                        */
   signed char    cp;       /* Coarse position (-1=>out-of-prox)          */
   ubyte          bt;       /* Button states (see below)                  */
   ubyte          d1;       /* Dubugging info : #1                        */
   ubyte          d2;       /* Dubugging info : #2                        */
   ubyte         cyc;       /* Cycle number from microcontroller          */
} MLEstyluPacket;

#define MLEstyluLeft   (1<<0)  /* Left  hand button alongside LCD         */
#define MLEstyluMiddle (1<<1)  /* Middle     button alongside LCD         */
#define MLEstyluRight  (1<<2)  /* Right hand button alongside LCD         */
#define MLEstyluBarrel (1<<3)  /* Bit mask for ->bt above                 */
#define MLEstyluTip    (1<<4)  /* Bit mask for ->bt above                 */
#define MLEstyluTopLf  (1<<7)  /* Top right-hand button alongside LCD     */
#define MLEstyluXord     0     /* Encoding for ->ord above                */
#define MLEstyluYord    255    /* Encoding for ->ord above                */

/*------------------------------------------------------------------------*/
/*                                         Digitiser information packets  */
/*------------------------------------------------------------------------*/

/* A circular buffer is maintained: On the input side, a raw-packet-      */
/*   handler is called every time the microcontroller sends a stylus-     */
/*   packet over the microlink, and the packet-handler copies the packet  */
/*   into the appropriate part of the circular buffer. On the output side,*/
/*   every time the digitiser server needs some stylus-co-ordinate        */
/*   information, it waits for a complete raw packet to turn-up in the    */
/*   buffer and reads it out and processes it into a Helios event or      */
/*   what-have-you.                                                       */
/* The following is the structure of each element of the circular buffer  */
/*   in which the raw-packets are stored.                                 */

typedef struct DigRawPacket
{  word           cyc;  /* Cycle number of this complete packet           */
   MLEstyluPacket x;    /* x-wire information                             */
   MLEstyluPacket y;    /* y-wire information                             */
} DigRawPacket;

/*------------------------------------------------------------------------*/
/*                             Digitiser state encoding of buttons field  */
/*------------------------------------------------------------------------*/

/* This defines the bit-field encoding of the ctx->is and ctx->was fields */
/*   of the "digitiser context" structure described below.                */

#define DigProxBitPos       0
#define DigTipBitPos        1
#define DigBarrelBitPos     2
#define DigLeftBitPos       3
#define DigMddlBitPos       4
#define DigRghtBitPos       5

#define DigTip              (1<<DigTipBitPos)
#define DigBarrel           (1<<DigBarrelBitPos)
#define DigProx             (1<<DigProxBitPos)
#define DigLeft             (1<<DigLeftBitPos)
#define DigMddl             (1<<DigMddlBitPos)
#define DigRght             (1<<DigRghtBitPos)

/*------------------------------------------------------------------------*/
/*                                   Digitiser data-processing constants  */
/*------------------------------------------------------------------------*/

/* Defined below are constants used for processing digitiser information: */
/* DigProxLimit gives the minimum delta between the two middle wire-      */
/*   values before the stylus is considered to be in proximity.           */
/* DigCountsPerWire gives the amount of units to measure from one coarse  */
/*   position to the next in the computed un-processed co-ordaintes.      */
/* The Min/Max X/Y Coarse values are used along with counts-per-wire by   */
/*   the server to compute the maximum and minimum expected co-ordinate   */
/*   values.                                                              */
/* DigFilterBits is used to control the filtering of data - see 'ext/     */
/*   digitiser.h'                                                         */

#define DigProxLimit     10
#define DigCountsPerWire 55
#define DigMinXCoarse    0
#define DigMinYCoarse    0
#define DigMaxXCoarse    44
#define DigMaxYCoarse    32
#define DigFilterBits    12

/*------------------------------------------------------------------------*/
/*                                                            DigContext  */
/*------------------------------------------------------------------------*/

/* The following context structure is used by the digitiser server whilst */
/*   a stream is open in order to control the buffering and processing of */
/*   raw digitiser events to produce processed stylus events or ascii-    */
/*   type text.                                                           */

#define DigLineLength  80
#define DigBuffSize     5    /* No of raw packets in raw packet buffer    */

/* Ord is encoded in exactly the same way as the same member of the       */
/*   structure MLEstyluPacket defined in 'source/protocol.h'              */

typedef struct DigContext
{  
   ML_MsgHandler     *rph;   /* Raw packet handler structure              */
   DigRawPacket     *base;   /* Base of raw packet buffer                 */
   DigRawPacket      *lim;   /* Last+1 entry in raw packet buffer         */
   DigRawPacket       *rp;   /* Raw packet buffer read pointer            */
   DigRawPacket       *wp;   /* Raw packet buffer write pointer           */
   Semaphore          sem;   /* For above : signal on write, wait to read */
   int             maxsem;   /* Count limit for above to avoid overflow   */
   WORD               cyc;   /* Cycle number of next raw packet from uL   */
   int                ord;   /* Which ord (x- or y-) expected from uL     */

   SHORT             lstX;   /* Last computed x-position : For next event */
   SHORT             lstY;   /* Last computed y-position : For next event */
   word            lstCyc;   /* Cycle number of last position             */
   ubyte              was;   /* Status of buttons according to events     */
   ubyte               is;   /* Last processed status of buttons          */
   ubyte              ctt;   /* Set when ready to send coord as event     */
   word                tm;   /* Last processed packet time-stamp          */

   Port         eventPort;   /* Port to send events over, else NullPort   */
   int                req;   /* For main thread to request events to stop */
   Semaphore          ack;   /* For event thread to signal it has stopped */

   char asciiLine            /* Buffer to store textual report of the     */
        [DigLineLength+1];   /*   stylus position.                        */
   word         readPhase;   /* Status of ascii report line read          */

} DigContext;

/*------------------------------------------------------------------------*/
/*                                          Digitiser division algorithm  */
/*------------------------------------------------------------------------*/

/* Because of overflow errors, and to avoid floating-point arithmetic,    */
/*   we implement a special purpose division algorithm in assembler in    */
/*   'divide.s'. The function definition occurs here.                     */

extern int DigDivide(int,int);

/*------------------------------------------------------------------------*/
/*                                                      End Of Interlock  */
/*------------------------------------------------------------------------*/

#endif



