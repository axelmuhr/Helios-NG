/*
-- Copyright INMOS Limited 1991
--
-- **************************************************************************
--
-- Group:	Iq software
--
-- Product:	IMS-F006 Ethernet TRAM support software
--
-- File:	b431io.h
--
-- Version:	1.0
--
-- **************************************************************************
--
-- Contains constants that define the interface between the B431 device
-- driver and the corresponding client libraries. Users of the client library
-- will need to include this file in order to make the library calls. This
-- contains ethernet related constants, procedure return codes and describes
-- the ethernet statistics vector structure.
--
-- **************************************************************************
--
-- History:	
--
-- (20-SEP-91) Bob Wipfel, Created file.
--
-- **************************************************************************
*/

/* Physical ethernet address size (in bytes) */

#define PHYSICAL_ADDRESS_SIZE 6

/* Logical ethernet address filter size (in bytes) */

#define LOGICAL_ADDRESS_FILTER_SIZE 8

/* Ethernet packet header size. All packets have a header prepended to
   the front of the packet data. In IEEE 802.3 ethernet systems it contains
   the destination address, source address and a 16 bit length field. In
   Ethernet V2 systems the length field is used as a packet type code. The
   header is therefore 14 bytes long.
*/

#define PACKET_HEADER_SIZE (2 * PHYSICAL_ADDRESS_SIZE) + 2

/* Minimum / Maximum ethernet packet lengths (in bytes). Short packets will
   be padded out to the minimum length. The values include the 14 byte packet
   header and the trailing CRC field (4 bytes). User supplied packets should
   only include a CRC field if the ethernet interface is initialised with CRC
   generation disabled. Packets supplied by the ethernet interface will always
   contain the trailing CRC field bytes.
*/

#define MIN_PACKET_LENGTH 64
#define MAX_PACKET_LENGTH 1518

/* Minimum / Maximum ethernet packet lengths (in bytes) when operating the
   B431 driver in loopback mode. The values include the 14 byte packet
   header and the trailing CRC field (4 bytes). User supplied packets should
   only include a CRC field if the ethernet interface is initialised with CRC
   generation disabled. Packets supplied by the ethernet interface will always
   contain the trailing CRC field bytes.
*/

#define MIN_LOOPBACK_LENGTH 18
#define MAX_LOOPBACK_LENGTH 36

/* Failed transmit packet length, the B431 driver returns part of a packet it
   failed to transmit.
*/

#define FAILED_PACKET_LENGTH 64

/* Initialisation mode flags. Bit masks are defined to enable optional
   Ethernet interface functions or to modify the default operating mode.
*/

#define MEMORY_CHECK		0x0001
#define PROMISCUOUS_RX		0x0002
#define MONITOR_HEARTBEAT	0x0004
#define DISABLE_TX_CRC		0x0008
#define DISABLE_TX_RETRY	0x0010
#define INTERNAL_LOOPBACK	0x0020
#define FORCE_COLLISION		0x0040

/* Initialisation return codes. */

#define INIT_SUCCESS		0
#define INIT_NOT_STOPPED	1
#define INIT_HARDWARE_FAILED	2
#define INIT_MEMORY_FAULT	3

/* Start ethernet return codes. */

#define START_SUCCESS		0
#define START_NO_INIT_DONE	1

/* The B431 driver sends asynchronous messages to the client library to
   indicate the occurence of an internal event, such as the reception of an
   ethernet packet, or to respond to a previous asynchronous request message.
   The following codes are defined to indicate which message was received.
*/

/* These messages handshake a previous request. A request message is sent to
   the B431 driver by calling the appropriate client library procedure.
*/

#define B431_STOP_ETHER		16	/* Interface shut down */
#define B431_TERMINATE		17	/* Device driver terminated */
#define B431_ETHER_STATS	18	/* Ethernet statistics available */

/* The B431 driver sends these messages to indicate the occurence of an
   internal event condition.
*/

#define B431_RX_PACKET		19	/* Ethernet packet received */
#define B431_ERROR_REPORT	20	/* Device driver error report */

/* This code is returned by B431_Waitfor_Event() when cancelled. It allows
   the B431 interface to be polled.
*/

#define B431_WAIT_CANCELLED	21

/* The B431_ETHER_STATS message contains ethernet statistics returned by the
   driver. The statistics are contained in the following structure.
*/

typedef struct ether_stats {
	unsigned long	tx_packets;
	unsigned long	rx_packets;
	unsigned long	framing_errors;
	unsigned long	crc_errors;
	unsigned long	packets_dropped;
	unsigned long	packets_missed;
	unsigned long	deferred_transmits;
	unsigned long	late_collisions;
	unsigned long	carrier_lost;
	unsigned long	no_more_retries;
	unsigned long	single_retries;
	unsigned long	multiple_retries;
	unsigned long	average_tdr_value;
} ETHER_STATS;

/* The B431_ERROR_REPORT message contains an error code, it will also contain
   ethernet packet data in the case of ERROR_TX_PACKET_FAILED.
*/

#define ERROR_NO_ERROR			0
#define ERROR_TX_PACKET_FAILED		1
#define ERROR_HEARTBEAT_STOPPED		2
#define ERROR_TX_BABBLE_FAULT		3
#define ERROR_DMA_REQUEST_LATE		4
#define ERROR_TX_BUFFER_INVALID		5
#define ERROR_TX_BUFFER_UNDERFLOW	6

/* Function prototypes */

extern int B431_Init_Normal( long, const char *,
			const char *, const long );

extern void B431_Terminate( long );
extern int B431_Start_Ether( long );
extern void B431_Ether_Stats( long );

extern void B431_Tx_Packet1( long, const char *, const int );

extern int B431_Waitfor_Event( long, ETHER_STATS *, char *,
				int *, int *, char * );
