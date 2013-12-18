/* $Header: general.h,v 1.2 91/01/09 12:30:26 charles Locked $ */
/* $Source: /server/usr/users/b/charles/world/microlink/RCS/ext/private/general.h,v $ */

/*------------------------------------------------------------------------*/
/*                                           microlink/private/general.h  */
/*------------------------------------------------------------------------*/

/* This header file contains definitions and declarations that only need  */
/*   to be accessed by the implementation of the "microlink/general" part */
/*   of the microlink server. The public part, containing definitions of  */
/*   structures for GetInfo() and SetInfo() and event-message structures  */
/*   is availiable in "microlink/general.h".                              */

/*------------------------------------------------------------------------*/
/*                                                             Interlock  */
/*------------------------------------------------------------------------*/

#ifndef MicrolinkPrivateGeneral_h
#define MicrolinkPrivateGeneral_h

/*------------------------------------------------------------------------*/
/*                                                          Header Files  */
/*------------------------------------------------------------------------*/

#include "microlink/general.h"

/*------------------------------------------------------------------------*/
/*                                       General Event Context Structure  */
/*------------------------------------------------------------------------*/

/* The "general" microlink stream can have any amount of event-channels   */
/*   set up: up to one per type of microlink message that may be received */
/*   from the microlink controller. For each open event context structure */
/*   a linked list of active event-stream records is kept with one entry  */
/*   for each type of event enabled on that open stream. The structures   */
/*   in the linked list are defined below.                                */

typedef struct GeneralEventChannel
{  struct GeneralEventChannel *nxt;  /* Linkage in list                   */
   ubyte                       typ;  /* Type of message expected          */
   ML_MsgHandler                mh;  /* Low level handler record          */
   Port                        evp;  /* Port over which to send events    */
   GeneralEvent               *bse;  /* Circular buffer base pointer      */
   GeneralEvent                *rp;  /* Circular buffer read pointer      */
   GeneralEvent                *wp;  /* Circular buffer write pointer     */
   GeneralEvent               *lim;  /* Circular buffer last+1 entry      */
   int                      maxsem;  /* Limit count for below semaphore   */
   Semaphore                   sem;  /* For above: sig on wr, wait to rd  */
   Semaphore                   acc;  /* Wait to signal or init '->sem'    */
   int                         cyc;  /* Cycle counter for events          */
   int                         ovf;  /* Buffer overflow flag              */
   int                         req;  /* Kill procedure communication      */
   Semaphore                   ack;  /* Kill procedure communication      */
} GeneralEventChannel;

/*------------------------------------------------------------------------*/
/*                                         The General Context Structure  */
/*------------------------------------------------------------------------*/

/* For each open stream to the "general" microlink server, one of the     */
/*   following structures exists:                                         */

typedef struct GeneralContext
{  int                isInRq;     /* "Request initiated" flag             */
   MlkLinkRequestControl ctl;     /* Control block for requests           */
   GeneralMlkReply        rx;     /* Buffer to collect reply info         */
   GeneralMessageBlock    tx;     /* Message to transmit                  */
   GeneralEventChannel *evts;     /* Events enabled on the channel        */
} GeneralContext;

/*------------------------------------------------------------------------*/
/*                                           Sizeof General Event Buffer  */
/*------------------------------------------------------------------------*/

/* Below is defined the buffer size for buffers which collect packets     */
/*   over the microlink.                                                  */

#define GeneralBufferSize 10

/*------------------------------------------------------------------------*/
/*                                                      End Of Interlock  */
/*------------------------------------------------------------------------*/

#endif

