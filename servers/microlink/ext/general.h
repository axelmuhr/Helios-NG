/* $Header: general.h,v 1.2 91/01/09 12:30:11 charles Locked $ */
/* $Source: /server/usr/users/b/charles/world/microlink/RCS/ext/general.h,v $ */

/*------------------------------------------------------------------------*/
/*                                                   microlink/general.h  */
/*------------------------------------------------------------------------*/

/* This file contains information useful for clients to the microlink     */
/*   "general" server: The part of the server which allows external       */
/*   clients to communicate messages of their own choosing over the       */
/*   microlink via. a file-type-interface.                                */

/*------------------------------------------------------------------------*/
/*                                                             Interlock  */
/*------------------------------------------------------------------------*/

#ifndef MicrolinkGeneral_h
#define MicrolinkGeneral_h

/*------------------------------------------------------------------------*/
/*                                                     Message structure  */
/*------------------------------------------------------------------------*/

/* A microlink message is a sequence of up to 36 bytes. The actual length */
/*   of the message is inferred from framing information. Internally,     */
/*   microlink messages are always stored with the first byte (the header */
/*   byte) on a word-boundary.                                            */

typedef struct GeneralMessageBlock
{  ubyte byt[36];
} GeneralMessageBlock;

/*------------------------------------------------------------------------*/
/*                                                   SetInfo() structure  */
/*------------------------------------------------------------------------*/

/* The following is the structure is used in transmitting a SetInfo()     */
/*   request: A stream may be opened to "/microlink/general" and          */
/*   SetInfo() commands issued to it: These commands result in the        */
/*   optional transmission of a messages plus optionally the reception of */
/*   a message combined with a timeout on the whole operation. The whole  */
/*   operation is interlocked with other operations to communicate with   */
/*   the microcontroller. The SetInfo() structure contains a flag, to     */
/*   indicate whether the tranmission phase of the operation should be    */
/*   excecuted, in which case the contents of the array <msg> indicates   */
/*   the message to be tranmitted. There is another member of the         */
/*   structure which indicates what type of message is expected to be     */
/*   received in the reply, or can be set to zero to indicate that no     */
/*   reply message is expected.                                           */
/* To read the message received in response use a GetInfo() call (see     */
/*   later). GetInfo() should be used shortly after the SetInfo()         */
/*   otherwise the process will time-out and the GetInfo() will return    */
/*   information to that effect.                                          */

typedef struct GeneralMlkRequest
{  int     rxType;          /* Expected type of reply message (or 0)      */
   int     txFlag;          /* Flag: To transmit message first or not     */
   GeneralMessageBlock msg; /* Transmission message                       */
} GeneralMlkRequest;

/*------------------------------------------------------------------------*/
/*                                                   GetInfo() structure  */
/*------------------------------------------------------------------------*/

/* Shortly after any SetInfo() request which expects a reply message to   */
/*   be received from the microcontroller, a GetInfo() request should be  */
/*   issued to wait for the reply message and see what it is. The reply   */
/*   message will be returned in the following format.                    */

typedef struct GeneralMlkReply
{  word             status;            /* Status of transfer              */
   word               code;            /* Helios error code if appl.      */
   GeneralMessageBlock msg;            /* Received message                */
} GeneralMlkReply;

/* The status in the above structure is encoded as follows:                */

# define General_Ok        0           /* Message transferred sucessfully  */
# define General_TxFailed  1           /* Helios error code in '->code'    */
# define General_RxFailed  2           /* Helios error code in '->code'    */

/*------------------------------------------------------------------------*/
/*                                               General event structure  */
/*------------------------------------------------------------------------*/

/* This structure is the structure recevied by client event-handlers      */
/*   set up to handle events on the "/microlink/general" server.          */
/* This structure is similar to the 'IOEvent' structure provided by       */
/*   Helios except that the '->Device' field is replaced by the received  */
/*   message block. The cycle number is guaranteed to increment by        */
/*   exactly on between events so that loss of events can be detected,    */
/*   but lost events will never get re-transmitted. The time stamp is not */
/*   guaranteed to contain any useful information. The type field is      */
/*   guaranteed to contain the first byte of the received message.        */

typedef struct GeneralEvent
{  word Type;
   word Counter;
   word Stamp;
   union Device
   {  GeneralMessageBlock microlink;
   } Device;
} GeneralEvent;

/*------------------------------------------------------------------------*/
/*                                                      End Of Interlock  */
/*------------------------------------------------------------------------*/

#endif
