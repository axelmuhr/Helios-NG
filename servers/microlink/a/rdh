/* $Header: rawdig.h,v 1.1 91/01/31 13:51:54 charles Locked $ */
/* $Source: /server/usr/users/b/charles/world/microlink/RCS/ext/rawdig.h,v $ */

/*------------------------------------------------------------------------*/
/*                                                hdr/microlink/rawdig.h  */
/*------------------------------------------------------------------------*/

/* The following file contains declarations of the structures and         */
/*   so-forth that would be required by a client to the digitiser server: */
/* To be specific, it contains the structure of the DigitiserInfo         */
/*   packets returned by the digitiser part of the microlink server by    */
/*   client calls to GetInfo() and also accepted by it in response to     */
/*   client calls to SetInfo().                                           */

/*------------------------------------------------------------------------*/
/*                                                             Interlock  */
/*------------------------------------------------------------------------*/

#ifndef MicrolinkRawDig_h
#define MicrolinkRawDig_h

/*------------------------------------------------------------------------*/
/*                                            RawDig information packets  */
/*------------------------------------------------------------------------*/

/* The message-type if the stylus-information messages. The byte has the  */
/*   bottom two length bits masked out, so the byte that would be         */
/*   received as message header actually has value 'BF' hex.              */
#define MLErawdg            0xBC

typedef struct MLErawdgPacket /* Structure of rawdig packet body ...      */
{  ubyte         hdr;       /* Header byte : uL frame information         */
   ubyte         len;       /* Length byte : uL frame information         */
   ubyte         buttons;   /* Button state                               */
   signed char   ppmean;    /* Peak-to-peak mean                          */
   signed char   dcavg;     /* DC average                                 */
   ubyte         d4;        /* Fourth debugging byte - as yet undefined   */
   ubyte         dcsumlo;   /* DC sum LOW                                 */
   ubyte         dcsumhi;   /* DC sum HIGH                                */
   signed char w[44];       /* Raw wire values                            */
} MLErawdgPacket;

/*------------------------------------------------------------------------*/
/*                                                           RawDigEvent  */
/*------------------------------------------------------------------------*/

typedef struct RawDigEvent
{  word           cyc;      /* Cycle number for detect overflows          */
   MLErawdgPacket pkt;      /* The packet itself.                         */
} RawDigEvent;

/*------------------------------------------------------------------------*/
/*                                                      End-Of-Interlock  */
/*------------------------------------------------------------------------*/

#endif

