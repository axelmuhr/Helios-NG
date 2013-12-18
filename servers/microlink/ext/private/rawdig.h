/* $Source: /server/usr/users/b/charles/world/microlink/RCS/ext/private/rawdig.h,v $ */
/* $Header: rawdig.h,v 1.1 91/01/31 13:52:03 charles Locked $ */

/*------------------------------------------------------------------------*/
/*                                            microlink/private/rawdig.h  */
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

#ifndef MicrolinkPrivateRawDig_h
#define MicrolinkPrivateRawDig_h

/*------------------------------------------------------------------------*/
/*                                                          Header Files  */
/*------------------------------------------------------------------------*/

#include "microlink/rawdig.h"

/*------------------------------------------------------------------------*/
/*                                                            DigContext  */
/*------------------------------------------------------------------------*/

#define RawDigBuffSize    4     /* No of raw packets in raw packet buffer */

typedef struct RawDigContext
{  
   ML_MsgHandler     *rph;   /* Raw packet handler structure              */
   RawDigEvent      *base;   /* Base of raw packet buffer                 */
   RawDigEvent       *lim;   /* Last+1 entry in raw packet buffer         */
   RawDigEvent        *rp;   /* Raw packet buffer read pointer            */
   RawDigEvent        *wp;   /* Raw packet buffer write pointer           */
   Semaphore          sem;   /* For above : signal on write, wait to read */
   int             maxsem;   /* Count limit for above to avoid overflow   */
   WORD               cyc;   /* Cycle number of next raw packet from uL   */

   word                tm;   /* Last processed packet time-stamp          */

   Port         eventPort;   /* Port to send events over, else NullPort   */
   int                req;   /* For main thread to request events to stop */
   Semaphore          ack;   /* For event thread to signal it has stopped */

} RawDigContext;

/*------------------------------------------------------------------------*/
/*                                                      End Of Interlock  */
/*------------------------------------------------------------------------*/

#endif



