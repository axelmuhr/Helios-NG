/*------------------------------------------------------------------------
--                                                                      --
--                     H E L I O S   N U C L E U S                      --
--                     ---------------------------                      --
--                                                                      --
--             Copyright (C) 1987, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- message.h								--
--                                                                      --
--	Structures and procedures supported by Kernel to perform	--
--	message passing.						--
--                                                                      --
--	Author:  NHG 16/8/87						--
--                                                                      --
------------------------------------------------------------------------*/
/* SccsId: %W%	%G%	Copyright (C) 1987, Perihelion Software Ltd.	*/
/* $Id: message.h,v 1.4 1992/11/11 17:51:58 nickc Exp $ */

#ifndef __message_h
#define __message_h

#ifndef __helios_h
#include <helios.h>
#endif

/* Message port */

typedef uword		Port;		/* true structure hidden	*/

#define Port_Flags_Tx	   0x40000000	/* set if port if for tx only	*/
#define Port_Flags_Remote  0x20000000	/* set if port is a surrogate	*/

#define NullPort	   ((Port)0)	/* zero is never a valid port	*/


/* Message Header */
typedef struct MsgHdr {
	unsigned short	DataSize;	/* 16 bit data size		*/
	unsigned char	ContSize;	/* control vector size		*/
	unsigned char	Flags;		/* flag byte			*/
	Port		Dest;		/* destination port descriptor	*/
	Port		Reply;		/* reply port descriptor	*/
	word		FnRc;		/* function/return code		*/
} MsgHdr;

#define MsgHdr_Flags_nothdr	0x80	/* used by kernel		*/
#define MsgHdr_Flags_preserve	0x40	/* preserve destination route	*/
#define MsgHdr_Flags_exception	0x20	/* exception message		*/
#define MsgHdr_Flags_sacrifice	0x10	/* receive data from link	*/
#define MsgHdr_Flags_bytesex	0x08	/* 0 = even, 1 = odd		*/
#ifdef __C40
# define MsgHdr_Flags_bytealign	0x03	/* 0 = word aligned, 1,2,3 =	*/
#endif					/* invalid bytes in first word	*/
					/* used by kernel		*/

/* Message control block */

typedef struct MCB {
	MsgHdr		MsgHdr;		/* message header buffer	*/
	word		Timeout;	/* message timeout in usecs	*/
	word		*Control;	/* pointer to control buffer	*/
	byte		*Data;		/* pointer to data buffer	*/
} MCB;

/* Port Info Structure */

typedef struct PortInfo {
	byte		Type;		/* entry type			*/
	byte		Cycle;		/* cycle value			*/
	byte		Link;		/* link for surrogates		*/
	byte		Age;		/* GCTicks since last use	*/
	word		Owner;		/* owning task			*/
	Port		Surrogate;	/* port for type >= T_Surrogate	*/
} PortInfo;

#define	T_Free			0	/* unused slot			*/
#define T_Local			1	/* local message port		*/
#define T_Surrogate		2	/* surrogate message port	*/
#define T_Trail			3	/* intermediate route entry	*/
#define T_Permanent		4	/* permanent surrogate		*/

/* Kernel Procedures */

PUBLIC Port NewPort(void);
PUBLIC word FreePort(Port port);
PUBLIC word PutMsg(MCB *mcb);
PUBLIC word GetMsg(MCB *mcb);
PUBLIC word XchMsg(MCB *txmcb, MCB *rxmcb);
PUBLIC word GetPortInfo(Port port, PortInfo *info);
PUBLIC word AbortPort(Port port,word code);
PUBLIC void SendException(Port port,word code);
PUBLIC word GetReady(Port port);
PUBLIC word PutReady(Port port);
PUBLIC word MultiWait(MCB *mcb, word nports, Port *ports);

#endif

/* -- End of message.h */
