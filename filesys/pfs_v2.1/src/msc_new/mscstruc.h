/*
 * $Header: /Chris/00/helios/msc/RCS/mscstruc.h,v 2.0 91/08/21 18:04:38 chris
 * Exp Locker: chris $
 */

/*************************************************************************
**									**
**	            M S C   D I S C   D E V I C E			**
**	            -----------------------------			**
**									**
**		  Copyright (C) 1990, Parsytec GmbH			**
**			 All Rights Reserved.				**
**									**
**									**
** mscstruc.h								**
**									**
**	- Data structures for the Device Driver				**
**									**
**************************************************************************
** HISTORY   :								**
** -----------								**
** Author    :	14/09/90 : C. Fleischer					**
*************************************************************************/

#ifndef	__mscstruc_h
#define	__mscstruc_h

#define	Structured
#include "scsiinfo.h"

/*-------------------------- Type Definitions --------------------------*/

#define	DebugLines	64
#define DebugLength	128
#define DebugLShift	7
#define DebugSize	(DebugLines * DebugLength)

/*
 * Debug Buffer
 * Diagnostic output is not directly performed with IOdebug. Instead,
 * the messages are kept in a circular buffer until an error occurs.
 * In this case, the last messages ( up to 128 ) are flushed using
 * IOdebug(), and all forthcoming messages are directly sent.
 */
 
typedef struct Debug {
    bool            out;		/* TRUE if direct output	 */
    word            line;		/* line position in the buffer	 */
    char            buf[DebugSize];	/* buffer for lines		 */
}               Debug;

/*
 * Data Pointers
 * This Structure is passed to TransferData and returned with the
 * values updated. A Save Data Pointer Message causes these values
 * to be copied into the Request Control Block.
 */

typedef struct DP {
    word            Rest;		/* bytes remaining to transfer	 */
    byte           *Data;		/* Data pointer			 */
}                DP;

/*
 * Request Control Block
 * holds the request chaining node, request selection parameters, the
 * SCSI register set which shell be used for register setup and the
 * Save Data Pointers state and other request-related Data such as a
 * pointer to the original DiscReq structure.The RCB will be used as
 * Workspace for the Action() function and is freed afterwards. Fields
 * are set by: DO : DevOperate DH : DriveHandler SH : ScsiHandler
 */
 
typedef struct RCB {
    Node            Node;		/* DO Chaining node		 */
    DiscReq        *Req;		/* DO Pointer to Client request	 */

    word            Start;		/* DO Drive's Block number	 */
    word            Size;		/* DO Transfer Size in bytes	 */
    byte           *Data;		/* DO Data buffer pointer	 */

    word            DriveID;		/* DH Drive's SCSI ID and LUN	 */
    word            BlkSize;		/* DH Drive's Block size	 */

    bool            Read;		/* DH Data direction (TRUE=Read) */
    bool            Block;		/* DH use Block move		 */

    bool            Done;		/* SH Request has been returned	 */
#if 0
    bool            Discon;		/* SH Target has disconnected	 */
#endif

    ubyte           Regs[23];		/* DH,SH initialised Registers	 */

    DP              sdp;		/* SH Saved Data Pointers	 */

    word            EndTime;		/* SH endtime for timeout	 */
    word            Status;		/* SH returned Status value	 */
}                RCB;

/*
 * SCSI Data Block
 * holds the hardware-related values, the Xilinx register copies,
 * the SCSI bus status and the common Register set which holds the
 * default values for all special registers.
*/

typedef struct SDB {
    uword           OwnId;		/* our SCSI Address (priority)	 */
    uword           XI_reg0;		/* Copy of Xilinx Reg 0		 */
    uword           XI_reg1;		/* Copy of Xilinx Reg 1		 */
    uword           ScsiStatus;		/* Last ScsiStatus value	 */
    uword           AuxStatus;		/* Last AuxStatus value		 */
    uword           ScsiTime;		/* last Status time + delay	 */
    word            nInOut;		/* number of Request channels	 */
    Channel        *Event;		/* Event channel		 */
    Channel       **InOut;		/* array of Request channels	 */
    RCB           **Request;		/* array for pending Requests	 */
    uword          *DevState;		/* word holds the Device ID and
    					 * LUN, a sync flag bit and the
    					 * sync transfer value.		 */
    Debug           debug;		/* Debug output buffer		 */
}                SDB;

/* Mask values and macros for the DevState word				 */

#define DS_FullMask	0x007700FF	/* Full Ident & sync mask	 */
#define DS_IdentMask	0x00770000	/* Full Ident mask		 */
#define DS_IDMask	0x00700000	/* SCSI ID mask			 */
#define DS_IDShift	20		/* SCSI ID shift		 */
#define DS_LUNMask	0x00070000	/* Target LUN mask		 */
#define DS_LUNShift	16		/* Target LUN shift		 */
#define DS_SyncMask	0xFF		/* Sync Transfer Register value	 */
#define DS_SyncBit	0x100		/* Sync mode checked bit	 */

#define SetID(st,id)		\
	((st) = (st) & ~DS_IDMask | ((id) & 7) << 20)

#define SetLUN(st,lun)		\
	((st) = (st) & ~DS_LUNMask | ((lun) & 7) << 16)

#define SetIdent(st,id,lun)	\
	(st) = (st) & ~DS_IdentMask | (((((id) & 7) << 4) | ((lun) & 7)) << 16)

#define SetSync(st,val)	((st) = (st) & ~0xFF | ((val) & 0xFF))

#define ClrCheck(st)	((st) &= ~DS_SyncBit)
#define SetCheck(st)	((st) |= DS_SyncBit)
#define GetCheck(st)	(!((st) & DS_SyncBit))	/* Sync was checked	 */

#define GetID(st)	(((st) >> 20) & 7)	/* Target SCSI bus ID	 */
#define GetLUN(st)	(((st) >> 16) & 7)	/* Target LUN		 */
#define GetIdent(st)	(((st) >> 16) & 0x77)	/* Target ID and LUN	 */
#define GetSync(st)	((st) & 0xFF)	/* Sync Trans reg value	 */

/*
 * Drive Description
 * This structure holds those parameters which are related to the
 * physical Device, such as State, SCSI Address & LUN and the Command
 * table pointer.
 */

typedef struct DriveDesc {
    word            DriveNum;		/* Drive Number in DevInfo	 */
    Channel        *Chan;		/* Channel to SCSI Handler	 */
    Semaphore      *Lock;		/* DriveDesc Lock for Init	 */
    word            DriveID;		/* SCSI Address & LUN		 */
    word            BlkCount;		/* number of Blocks in Drive	 */
    word            BlkSize;		/* Drive block size		 */
    word            UnitShift;		/* LShift Unit -> Byte values	 */
    word            BlkShift;		/* LShift Block -> Byte values	 */
    word            PosShift;		/* LShift Unit -> Block values	 */
    word            Loaded;		/* Drive opened successfully	 */
    word            Closed;		/* Drive closing / closed	 */
    word            Type;		/* SCSI Device type		 */
    word            Removable;		/* Medium is removable		 */
    word            Formatted;		/* Medium is formatted		 */
    word            Locked;		/* Removable Medium is locked	 */
    word            Protected;		/* Medium is Write-protected	 */
    Semaphore      *ReqCount;		/* Lock and List length counter	 */
    List           *ReqList;		/* Request list			 */

/* Elevator algorithm statics	 					 */
    word            LastPos;		/* last Head position		 */
    word            LastDir;		/* last Head movement direction	 */

    ScsiDevInfo    *Info;		/* Scsi Command definitions	 */
    CmdInfo       **DefCmds;		/* default Command Infos	 */
    ReqInfo       **DefReqs;		/* default Request Infos	 */

    byte           *DefData;		/* default Data buffer		 */
    RCB            *ChkRCB;		/* RCB for Check Condition	 */
    byte           *ChkData;		/* buffer for Check Condition	 */
    byte           *TestData;		/* buffer for R/W tests		 */
}                DriveDesc;

#define DefCmdsSize	13
#define DefReqsSize	3
#define DefDataSize	1024
#define ChkDataSize	256

#define DriveDescSize	( sizeof ( DriveDesc ) + \
			2 * sizeof ( Semaphore ) + sizeof ( List ) + \
			DefCmdsSize * sizeof ( CmdInfo * ) + \
			DefReqsSize * sizeof ( ReqInfo * ) + \
			DefDataSize + sizeof ( RCB ) + ChkDataSize )

#define DD_Loaded	0x01		/* Drive loaded, Handler active	 */
#define DD_Terminating	0x04		/* Drive got a Close request	 */
#define DD_Closing	0x08		/* Driver shutting down		 */
#define DD_Error	0xf0		/* any Drive error, Handler
					 * terminated or not started	 */

#define RI_Format		0	/* Indices for Requests		 */
#define RI_Open			1
#define RI_Close		2

/*
 * Partition Description
 * This structure holds the description of a single partition
 * as it is defined in the DevInfo file verified from the Device.
 */

typedef struct PartDesc {
    word            DriveNum;		/* Number of related Drive	 */
    DriveDesc      *Drive;		/* relared DriveDesc		 */
    word            FirstUnit;		/* First Unit of partition	 */
    word            LastUnit;		/* Last Unit of partition	 */
    word            DILastUnit;		/* DevInfo specified Last Unit	 */
}                PartDesc;

/*
 * MSC Device Control Block
 * This structure holds all the data which is necessary for Driver
 * operation, it acts as a static data storage. It contains all data
 * for interconnecting the different processes.
 */

typedef struct DiscDCB {
    DCB             DCB;		/* standard DCB			 */
    SDB             sdb;		/* SCSI Data block		 */
    word            UnitSize;		/* Block size (Addressing)	 */
    word            UnitShift;		/* LShift Unit -> Byte values	 */
    word            ScsiAddress;	/* MSC SCSI Address (priority)	 */
    word            nDrive;		/* Number of DriveTab entries	 */
    word            nPart;		/* Number of PartTab entries	 */
    word            Handlers;		/* # of running DriveHandlers	 */
    DriveDesc      *DriveTab;		/* Drive descriptors		 */
    PartDesc       *PartTab;		/* Partition descriptors	 */
    Channel       **Channels;		/* Chans between DriveHandlers
					 * and ScsiHandler		 */
    bool            Closing;		/* DevClose has started		 */
    byte            used[8];		/* table of used SCSI ID's	 */

#ifndef	EVENTS
    Channel         EventChan;		/* my Event Channel		 */
    bool            stop;		/* quit flag for WatchXilinx	 */
    Semaphore       stopped;		/* quit sem for WatchXilinx	 */
#endif
    void           *ScsiInfo;		/* Scsi Info buffer		 */
}                DiscDCB;

typedef struct LoadResult {		/* Result for FG_Open request	 */
    word            Error;		/* TRUE if any Problem arises	 */
    word            Raw;		/* Drive is for tapes		 */
    word            Removable;		/* Medium removable from Drive	 */
    word            Loaded;		/* Drive opened successfully	 */
    word            Protected;		/* Medium is write-protected	 */
    word            Formatted;		/* Medium is formatted		 */
    word            NotLocked;		/* Medium removal any time !	 */
}                LoadResult;

#endif

/*--- end of mscstruc.h ---*/
