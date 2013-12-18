/*
-- Copyright INMOS Limited 1991
--
-- **************************************************************************
--
-- Group:	Iq software
--
-- Product:	IMS-F006 Ethernet TRAM support software
--
-- File:	b431drvr.h
--
-- Version:	1.0
--
-- **************************************************************************
--
-- Contains constants that define the private interface between the B431
-- device driver and the corresponding client libraries. A similar header
-- file exists that defines the public interface, ie: constants that users
-- of the client library will need in order to make the library calls.
--
-- The message passing protocol that the driver and client libraries adopt
-- is specified by this file.
--
-- **************************************************************************
--
-- History:	
--
-- (15-SEP-91) Bob Wipfel, Created file.
--
-- **************************************************************************
*/

/* Messages are sent to and received from the driver on a transputer
   channel. Messages are transferred using a fixed sized header followed
   by variable length data. The length of the data part is contained within
   the header. The header also contains a function code field, and an optional
   simple result field. The function code is used to tag function request
   or response messages and the simple result field can be used to return a
   result code.
*/

/* Message header structure */

typedef struct message_hdr {
	short	length;		/* Length of the following data part */
	char	fn_code;	/* Function code */
	char	result;		/* Simple result code */
} MESSAGE_HDR;

/* Macro to initialise MESSAGE_HDR */

#define INIT_MESSAGE_HDR( h, l, f, r ) \
	{ h.length = l; h.fn_code = f; header.result = r; }

/* Function request messages implemented by the B431 driver */

/* These messages are synchronous, the function code is carried both in
   the request and the response message. The simple result field is used
   to carry the result.
*/

#define FN_INIT_NORMAL		1	/* Init interface in normal mode */
#define FN_INIT_LOOPBACK	2	/* Init interface in loopback mode */
#define FN_START_ETHER		3	/* Start ethernet interface */
#define FN_CHECK_ALIVE		4	/* Check device driver is alive */

/* These messages are only ever sent to the driver. It does not send an
   acknowledge message.
*/

#define FN_TX_PACKET1		5	/* Send packet, type 1 parameters */
#define FN_TX_PACKET2		6	/* Send packet, type 2 parameters */
#define FN_RESET_STATS		7	/* Reset ethernet statistics */

/* These messages are asynchronous, the function code is carried only in
   the request message. The driver will send the response message when it
   has completed the operation. In the meantime it may send other messages
   to indicate the occurence of an event condition. The response function
   and result codes are defined in a public header file.
*/

#define FN_STOP_ETHER		8	/* Stop ethernet interface */
#define FN_TERMINATE		9	/* Terminate device driver */
#define FN_ETHER_STATS		10	/* Request ethernet statistics */
