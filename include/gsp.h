/*------------------------------------------------------------------------
--                                                                      --
--                     H E L I O S   N U C L E U S                      --
--                     ---------------------------                      --
--                                                                      --
--             Copyright (C) 1987, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- gsp.h								--
--                                                                      --
--	General Server Protocol definitions				--
--                                                                      --
--	Author:  NHG 16/8/87						--
--                                                                      --
------------------------------------------------------------------------*/
/* SccsId: %W% %G% Copyright (C) 1987, Perihelion Software Ltd.	*/
/* $Id: gsp.h,v 1.6 1992/06/23 17:27:11 paul Exp $ */

#ifndef __gsp_h
#define __gsp_h

#ifndef __helios_h
#include <helios.h>
#endif

#include <syslib.h>

typedef	word		Offset;		/* offset into message data vector    */
typedef word		String;		/* offset to 0 terminated string      */
typedef word		Struct;		/* offset to structure in data vector */

/*----------------------------------------------------------------------*/
/* Common substructure for all IOC messages				*/
/*----------------------------------------------------------------------*/

typedef struct IOCCommon {
	String		Context;	/* offset of context string	*/
	String		Name;		/* offset of object name string	*/
	String		Next;		/* offset of next element in path */
	Capability	Access;		/* capability of context object	*/
} IOCCommon;


/*
 * A minimal message, used by the following message types:
 * ObjectInfo, ServerInfo, Delete, Locate
 */

typedef struct IOCMsg1 {
	IOCCommon	Common;		/* common part of message	*/
} IOCMsg1;


/* A message with a single extra word in the control vector. Used by:
 * Open(mode), Rename(toname), Protect(newmatrixmask), SetDate(date)
 */

typedef struct IOCMsg2 {
	IOCCommon	Common;		/* common part of message	*/
	union {
		word		Mode;	/* open mode			*/
		String		ToName;	/* new name			*/
		Matrix		Matrix;	/* new access matrix		*/
		AccMask		AccMask;/* mask for access bits in cap	*/
	} Arg;
} IOCMsg2;


/* A Link message */

typedef struct IOCMsg3 {
	IOCCommon	Common;		/* common part of message	*/
	String		Name;		/* object to be linked to	*/
	Capability	Cap;		/* its capability		*/
} IOCMsg3;

/* A SetDate message */

typedef struct IOCMsg4 {
	IOCCommon	Common;		/* common part of message	*/
	DateSet		Dates;		/* dates to be set		*/
} IOCMsg4;

/* A Create message							*/

typedef struct IOCCreate {
	IOCCommon	Common;		/* common part of message	*/
	word		Type;		/* type of object to create	*/
	word		Size;		/* size of info data		*/
	Offset		Info;		/* offset of info in Data vec	*/
} IOCCreate;

/* Messages for Sockets */

typedef struct IOCBind
{
	IOCCommon	Common;
	word		Protocol;
	Struct		Addr;
} IOCBind;

/* Reply message formats						*/

/* Replies consisting solely of a message header are returned to:
 * Delete, Rename, Link, SetDate
 */ 

/* The following reply is returned to:
 * Open, Create, Locate
 * In the case of Open the Reply field of the message header 
 * may contain a port on which the client may send further requests.
 */

typedef struct IOCReply1 {
	WORD		Type;		/* object type code 		*/
	WORD 		Flags;		/* flag word			*/
	Capability	Access;		/* a capability for it		*/
	String		Pathname;	/* full pathname of object	*/
	word		Object;		/* object value if no reply port*/
} IOCReply1;

/* The following reply is returned to a Refine request			*/

typedef struct IOCReply2 {
	Capability	Cap;		/* refined capability		*/
} IOCReply2;

/* The reply to a ServerInfo request to a filesystem has the following	*/
/* structure in its data vector.					*/

typedef struct FSInfo
{
	word		Flags;		/* flags word			*/
	word		Size;		/* file system size in bytes	*/
	word		Avail;		/* space available for use	*/
	word		Used;		/* space used			*/
} FSInfo;

/* Any new message structures should be added to the appropriate union here */

union IOCRequestSet {
	IOCCommon	IOCCommon;
	IOCMsg1		IOCMsg1;
	IOCMsg2		IOCMsg2;
	IOCMsg3		IOCMsg3;
	IOCMsg4		IOCMsg4;
	IOCCreate	IOCCreate;
	IOCBind		IOCBind;
};

union IOCReplySet {
	IOCReply1	IOCReply1;
	IOCReply2	IOCReply2;
};

/*----------------------------------------------------------------------*/
/* Stream Requests							*/
/*----------------------------------------------------------------------*/

/*
 * Replies to Close, SetSize consist of just the message header.
 *
 * A reply to GetSize has just the file size in bytes in the control vec.
 */

typedef struct GetSizeReply {
	word		Size;		/* file size in bytes		*/
} GetSizeReply;

/*
 * A seek request consists of the following structure:
 */

typedef struct SeekRequest {
	word		CurPos;		/* current file position	*/
	word		Mode;		/* seek mode			*/
	word		NewPos;		/* new position (rel. to mode)	*/
} SeekRequest;

typedef struct SeekReply {
	word		NewPos;		/* new file position		*/
} SeekReply;

/*
 * The following is used in all read/write requests.
 */

typedef struct ReadWrite {
	WORD		Pos;		/* file position		*/
	WORD		Size;		/* size of transfer		*/
	WORD		Timeout;	/* transfer completion time	*/
} ReadWrite;

/*
 * The reply to a read will consist of an arbitrary number of messages
 * containing the requested data, or a failure message. In addition to
 * possible error codes, the FnRc field of these messages will contain
 * a sequence number starting from 16 and incrementing in steps of 16
 * (Thus leaving the lower 4 bits clear). These lower 4 bits contain one
 * of the following values.
 */

#define ReadRc_Mask	0xf	/* mask for lower 4 bits		*/
#define ReadRc_More	0	/* more data to come			*/
#define ReadRc_EOD	1	/* end of data				*/
#define ReadRc_EOF	2	/* end of data and of file		*/

#define ReadRc_SeqInc	16	/* increment for sequence numbers	*/

/*
 * The first reply to a write will consist of the following structure
 * telling the sender how to format the data transfer. This is so copies
 * may be eliminated at the server end.
 * Once the data has been sent a second reply is made confirming that the
 * data were received.
 */

typedef struct WriteReply {
	word		first;		/* size of first data message	*/
	word		rest;		/* size of rest of messages	*/
	word		max;		/* max qty of data to send	*/
					/* this is present only in the	*/
					/* extended protocol format	*/
} WriteReply;

#define WriteRc_Done	0		/* return code if data has been	*/
#define WriteRc_Sizes	1		/* return code indicating sizes	*/
#define WriteRc_Already	2		/* server already has data	*/

/* Messages for Sockets */

typedef struct AcceptReply
{
	WORD		Type;		/* object type code 		*/
	WORD 		Flags;		/* flag word			*/
	Capability	Access;		/* a capability for it		*/
	String		Pathname;	/* full pathname of object	*/
	Struct		Addr;		/* network address of connector	*/	
} AcceptReply;

typedef struct ConnectRequest {
	Struct		DestAddr;	/* network address for connection */
	Struct		SourceAddr;	/* address of source		  */
} ConnectRequest;

/* This structure is built progressively as it is passed from program to*/
/* program, SendMessage builds it as far as DestAddr, the server adds	*/
/* SourceAddr and passes it back to SendMessage which adds the data and	*/
/* forwards it to RecvMessage.						*/

typedef struct DataGram {
	word		Flags;		/* flag word			*/
	word		DataSize;	/* actual data size		*/
	word		Timeout;	/* time to wait for tfr		*/
	Struct		AccRights;	/* access rights		*/
	Struct		DestAddr;	/* destination address		*/
	Struct		SourceAddr;	/* source address		*/
	Offset		Data;		/* message data			*/
} DataGram;

typedef struct SocketInfoReq
{
	word		Level;		/* option level			*/
	word		Option;		/* option name			*/
	Struct		Optval;		/* option value (optional)	*/
} SocketInfoReq;

/* Any new message structures should be added to the appropriate union here */

union StreamRequestSet {
	ReadWrite	ReadWrite;
	SeekRequest	SeekRequest;
	ConnectRequest	ConnectRequest;
	SocketInfoReq	SocketInfoReq;
};

union StreamReplySet {
	GetSizeReply	GetSizeReply;
	SeekReply	SeekReply;
	WriteReply	WriteReply;
	AcceptReply	AcceptReply;
};

/* This union is used mainly to determine the maximum size of a message	*/

typedef union GSPMessageSet {
	union IOCRequestSet	IOCRequestSet;
	union IOCReplySet	IOCReplySet;
	union StreamRequestSet	StreamRequestSet;
	union StreamReplySet	StreamReplySet;
	word Default[16];
} GSPMessageSet;
	
/* Miscellaneous */

#define IOCMsgMax 	(sizeof(GSPMessageSet)/sizeof(word)) /* largest control vector	*/
#define IOCDataMax	512		/* max data in IOC message	*/
#define IOCTimeout	(20*OneSec)	/* Response time from IOC	*/
#define IdleTimeout	(IOCTimeout/2)	/* Idle handshake time		*/
#define StreamTimeout	(30*60*OneSec)	/* stream server timeout	*/
#define WriteTimeout	(5*OneSec)	/* wait for data		*/
#define LoadTimeout	(2*60*OneSec)	/* loading can take a long time */

/*----------------------------------------------------------------------*/
/* Object Types								*/
/*----------------------------------------------------------------------*/

/* bottom 4 bits are flags for major sub type				*/

#define Type_Flags	0xf	/* mask for flag bits			*/
#define Type_Directory	1	/* supports GSP directory interface	*/
#define Type_Stream	2	/* supports GSP stream interface	*/
#define Type_Private	4	/* own private protocol			*/
				/* remaining bit reserved		*/

#define Type_File	(0x010|Type_Stream)
#define Type_Fifo	(0x020|Type_Stream)
#define Type_Module	(0x030|Type_Stream)
#define Type_Program	(0x040|Type_Stream)
#define Type_Task	(0x050|Type_Stream)
#define Type_Link	(0x060)
#define Type_Name	(0x070)
#define Type_TaskForce  (0x080|Type_Directory) 
#define Type_Processor	(0x090|Type_Stream)
#define Type_CacheName  (0x0a0)
#define Type_Pipe	(0x0b0|Type_Stream)
#define Type_Pseudo	(0x0c0|Type_Stream)
#define Type_Device	(0x0d0|Type_Private)
#define Type_Session	(0x0e0|Type_Stream)
#define Type_Socket	(0x0f0)
#define Type_Auto	(0x100)
#define Type_Network	(0x110|Type_Directory)
#endif

/* -- End of gsp.h */
