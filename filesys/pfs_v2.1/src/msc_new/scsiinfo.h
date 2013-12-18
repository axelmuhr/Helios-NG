/*
 * $Header: /Chris/00/helios/msc/RCS/scsiinfo.h,v 2.0 91/08/21 18:04:48 chris
 * Exp Locker: chris $
 */

/*************************************************************************
**									**
**		    M S C   D I S C   D E V I C E			**
**		    -----------------------------			**
**									**
**		  Copyright (C) 1990, Parsytec GmbH			**
**			 All Rights Reserved.				**
**									**
**									**
** scsiinfo.h								**
**									**
**	- Extensions to the device.h file for the scsiinfo file		**
**									**
**	These extensions should allow the usage of devinfo-related	**
**	functions for reading the scsiinfo file.			**
**									**
**************************************************************************
** HISTORY   :								**
** -----------								**
** Author    :	xx/xx/90 : C. Fleischer					**
*************************************************************************/

#ifndef __scsiinfo_h
#define __scsiinfo_h

#include <helios.h>

#ifndef	Structured
typedef struct CmdInfo {
    RPTR            Next;		/* link to next command		 */
    RPTR            Name;		/* Command Name			 */
    word            Code;		/* Default Command code		 */
    word            Read;		/* direction of data phase	 */
    word            Blockmove;		/* use Blockmove yes/no		 */
    word            CDBSize;		/* size of CDB			 */
    RPTR            CDB;		/* CDB contents			 */
    word            DataSize;		/* amount of data		 */
    RPTR            Data;		/* Default data bytes		 */
}

                CmdInfo;

typedef struct CondInfo {
    RPTR            Next;		/* link to next condition	 */
    word            Offset;		/* offset in Request Sense Data	 */
    word            Mask;		/* mask for bits to be checked	 */
    word            Value;		/* comparison value		 */
}

                CondInfo;

typedef struct ErrorInfo {
    RPTR            Next;		/* link to next error info	 */
    word            Code;		/* Error code			 */
    RPTR            Conditions;		/* list of conditions		 */
}

                ErrorInfo;

typedef struct ItemInfo {
    RPTR            Next;		/* link to next item		 */
    RPTR            Item;		/* pointer to CmdInfo		 */
}

                ItemInfo;

typedef struct ReqInfo {
    RPTR            Next;		/* link to next request		 */
    word            FnCode;		/* Helios function code		 */
    RPTR            Items;		/* list of commands		 */
}

                ReqInfo;

typedef struct ScsiDevInfo {
    word            Type;		/* Device Type: random/seq	 */
    word            Synchronous;	/* Drive supports Sync transfer	 */
    RPTR            Ident;		/* Inquiry name			 */
    RPTR            Commands;		/* list of commands		 */
    RPTR            Errors;		/* list of error conditions	 */
    RPTR            Requests;		/* list of request sequences	 */
}

                ScsiDevInfo;

#else

typedef struct CmdInfo {
    struct CmdInfo *Next;		/* link to next command		 */
    char           *Name;		/* Command Name			 */
    word            Code;		/* Default Command code		 */
    word            Read;		/* direction of data phase	 */
    word            Blockmove;		/* use Blockmove yes/no		 */
    word            CDBSize;		/* size of CDB			 */
    byte           *CDB;		/* CDB contents			 */
    word            DataSize;		/* amount of data		 */
    byte           *Data;		/* Default data bytes		 */
}

                CmdInfo;

typedef struct CondInfo {
    struct CondInfo *Next;		/* link to next condition	 */
    word            Offset;		/* offset in Request Sense Data	 */
    word            Mask;		/* mask for bits to be checked	 */
    word            Value;		/* comparison value		 */
}

                CondInfo;

typedef struct ErrorInfo {
    struct ErrorInfo *Next;		/* link to next error info	 */
    word            Code;		/* Error code			 */
    CondInfo       *Conditions;		/* list of conditions		 */
}

                ErrorInfo;

typedef struct ItemInfo {
    struct ItemInfo *Next;		/* link to next item		 */
    CmdInfo        *Item;		/* pointer to CmdInfo		 */
}

                ItemInfo;

typedef struct ReqInfo {
    struct ReqInfo *Next;		/* link to next request		 */
    word            FnCode;		/* Helios function code		 */
    ItemInfo       *Items;		/* list of commands		 */
}

                ReqInfo;

typedef struct ScsiDevInfo {
    word            Type;		/* Device Type: random/seq	 */
    word            Synchronous;	/* Drive supports Sync transfer	 */
    char           *Ident;		/* Inquiry name			 */
    CmdInfo        *Commands;		/* list of commands		 */
    ErrorInfo      *Errors;		/* list of error conditions	 */
    ReqInfo        *Requests;		/* list of request sequences	 */
}

                ScsiDevInfo;

#endif

#define Info_ScsiDev	101

#define CC_Unknown		-1
#define CC_Test_Unit_Ready	0
#define CC_Request_Sense	1
#define CC_Inquiry		2
#define CC_Mode_Sense		3
#define CC_Read			4
#define CC_Write		5
#define CC_Format		6
#define CC_Reassign_Blocks	7
#define CC_Read_Capacity	8
#define CC_Verify		9
#define CC_Rewind		10
#define CC_Write_Filemarks      11
#define CC_Space		12
#define CC_Prevent_Media_Removal	13
#define CC_Allow_Media_Removal	14
#endif

void           *LoadScsiInfo (void);
InfoNode       *FindScsiInfo (void *sinfo, int index);

/*--- end of scsiinfo.h ---*/
