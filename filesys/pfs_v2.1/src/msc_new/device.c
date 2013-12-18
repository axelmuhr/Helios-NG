/*
 * $Header: /Chris/00/helios/msc/RCS/device.c,v 2.0 91/08/21 18:05:33 chris
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
** device.c								**
**									**
**	- Device controller process					**
**									**
**************************************************************************
** HISTORY   :								**
** -----------								**
** Author    :	xx/xx/90 : C. Fleischer					**
*************************************************************************/

#ifndef	Driver
#include "msc.h"
#endif

/* #define DEBUG_ERR	1 */

/************************************************************************
 * SET UP THE REQUEST CONTROL BLOCK FOR A SCSI COMMAND
 *
 * - Collect Parameters from the Drive Descriptor and the Command Info.
 *
 * Parameter :	dd	= Drive Descriptor
 *		rcb	= Request Control Block
 *		cmd	= Command Info
 *
 ***********************************************************************/

void
SetupRCB (DriveDesc * dd, RCB * rcb, CmdInfo * cmd)
{
    word            to;

    rcb->DriveID = dd->DriveID;		/* copy params from DriveDesc	 */
    rcb->BlkSize = dd->BlkSize < 0 ? 512 : dd->BlkSize;

    rcb->Read = cmd->Read;		/* copy params from CmdInfo	 */
    rcb->Block = cmd->Blockmove;
    rcb->Regs[WR_CDBSize] = cmd->CDBSize;
    move_ (12, &rcb->Regs[WR_CDB01], cmd->CDB);

    rcb->Regs[WR_CDB02] |= GetLUN (dd->DriveID) << 5;
    to = rcb->Req->DevReq.Timeout;
    if (to + 1)
	rcb->EndTime = sum_ (_ldtimer (0), to);
    else
	rcb->EndTime = to;
}

/************************************************************************
 * ADD PARAMETERS TO A RANDOM ACCESS REQUEST CONTROL BLOCK
 *
 * - Complete the RCB with Block Address and Transfer length
 *   according to the CDB Size.
 *
 * Parameter :	rcb	= Request Control Block
 *		addr	= Logical Block Address
 *		length	= Transfer length
 * Result    :	FALSE	addr or length not within CDB Range
 *
 ***********************************************************************/

bool
SetupRandomSizes (RCB * rcb, word addr, word length)
{
    char           *fname = "SetRandomSizes";

    switch (rcb->Regs[WR_CDBSize]) {	/* switch with CDB Size		 */
    case 6:				/* 6 byte CDB			 */
	if ((uword) addr > 0x1FFFFF) {	/* check for 21 bit range	 */
	    IOdebug ("%s: address %d too large for 6 byte CDB !",
		     fname, addr);
	    return FALSE;
	}
	if ((uword) length > 256) {	/* check for 8 bit range	 */
	    IOdebug ("%s: length %d too large for 6 byte CDB !", fname, length);
	    return FALSE;
	}
	if (addr) {			/* 21 bit Block Address		 */
	    rcb->Regs[WR_CDB02] |= (addr >> 16) & 0x1F;
	    rcb->Regs[WR_CDB03] = addr >> 8;
	    rcb->Regs[WR_CDB04] = addr;
	}
	if (length)			/* 8 bit Transfer Size, 0 = 256	 */
	    rcb->Regs[WR_CDB05] = length;
	break;

    case 10:				/* 10 byte CDB			 */
	if ((uword) length > 0xFFFF) {	/* check for 16 bit range	 */
	    IOdebug ("%s: length %d too large for 10 byte CDB !", fname, length);
	    return FALSE;
	}
	if (addr) {
	    rcb->Regs[WR_CDB03] = addr >> 24;	/* 32 bit Block Address	 */
	    rcb->Regs[WR_CDB04] = addr >> 16;
	    rcb->Regs[WR_CDB05] = addr >> 8;
	    rcb->Regs[WR_CDB06] = addr;
	}
	if (length) {
	    rcb->Regs[WR_CDB08] = length >> 8;	/* 16 bit Transfer Size	 */
	    rcb->Regs[WR_CDB09] = length;
	}
	break;
    default:
	IOdebug ("%s: unknown CDB Size %d, treated as 12 !", fname, rcb->Regs[WR_CDBSize]);
    case 12:
	if ((uword) length > 0xFFFF) {	/* check for 16 bit range	 */
	    IOdebug ("%s: length %d too large for 12 byte CDB !", fname, length);
	    return FALSE;
	}
	if (addr) {
	    rcb->Regs[WR_CDB03] = addr >> 24;	/* 32 bit Block Address	 */
	    rcb->Regs[WR_CDB04] = addr >> 16;
	    rcb->Regs[WR_CDB05] = addr >> 8;
	    rcb->Regs[WR_CDB06] = addr;
	}
	if (length) {
	    rcb->Regs[WR_CDB10] = length >> 8;	/* 16 bit Transfer Size	 */
	    rcb->Regs[WR_CDB11] = length;
	}
	break;
    }
    return TRUE;
}

/************************************************************************
 * ADD PARAMETERS TO A RANDOM ACCESS REQUEST CONTROL BLOCK
 *
 * - Complete the RCB with Transfer length.
 *
 * Parameter :	rcb	= Request Control Block
 *		size	= Transfer length
 * Result    :	FALSE	size not within CDB Range
 *
 ***********************************************************************/

bool
SetupSequentialSize (RCB * rcb, word size)
{
    char           *fname = "SetSequentialSize";

    if (rcb->Regs[WR_CDBSize] != 6) {
	IOdebug ("%s: unknown CDB Size %d, treated as 6 !",
		 fname, rcb->Regs[WR_CDBSize]);
    }
    if (size > 0x007FFFFF || size < 0xFF800000) {
	IOdebug ("%s: size %d too large for 6 byte CDB !", fname, size);
	return FALSE;
    }

    rcb->Regs[WR_CDB03] = size >> 16;	/* 24 bit Transfer size		 */
    rcb->Regs[WR_CDB04] = size >> 8;
    rcb->Regs[WR_CDB05] = size;

    return TRUE;
}

/************************************************************************
 * SEND A RCB ADDRESS TO THE SCSI HANDLER AND WAIT FOR THE RESULT
 *
 * - This Function will be replaced if linked Scsi commands have to be
 *   supported.
 *
 * Parameter :	dd	= Drive Descriptor
 *		rcb	= Request Control Block
 *
 ***********************************************************************/

void
ExecRequest (DriveDesc * dd, RCB * rcb)
{
    RCB            *res;

    WriteWord (dd->Chan, rcb);
    ReadWord (dd->Chan, res);
    if (res != rcb) {
	IOdebug ("ExecRequest #%d: got bad RCB from ScsiHandler !\n"
		 "Sent %x ( ID 0x%x, Fn 0x%x ), Got %x ( ID 0x%x, Fn 0x%x)",
		 dd->DriveNum,
		 rcb, rcb->DriveID, rcb->Req->DevReq.Request,
		 res, res->DriveID, res->Req->DevReq.Request);
    }
}

#if	DEBUG & DEBUG_SENSE
static char    *
AddStr (char *buf, char *str)
{
    while (*str)
	*buf++ = *str++;
    return buf;
}

static char    *
AddHex (char *buf, uword x, int w)
{
    int             p = 0;
    char            b[8];
    char           *digits = "0123456789abcdef";

    do {
	b[p++] = digits[x & 0xF];
	x >>= 4;
    }
    while (--w > 0 || x);
    while (p--)
	*buf++ = b[p];
    return buf;
}

static char    *
AddInt (char *buf, int v)
{
    int             n = v < 0;
    int             p = 0;
    char            b[12];
    char           *digits = "0123456789";

    if (n)
	v = -v;
    do {
	b[p++] = digits[v % 10];
	v /= 10;
    }
    while (v);
    if (n)
	*buf++ = '-';
    while (p--)
	*buf++ = b[p];
    return buf;
}

#endif

/************************************************************************
 * CHECK A SENSE RESULT FOR KNOWN CONDITIONS
 *
 * - Step through the list of known errors and check their conditions.
 * - If all conditions of an error are met return this error code,
 *   otherwise proceed with the next error.
 * - If no error will match, return an unspecified error code.
 *
 * Parameter :	dd	= Drive Descriptor
 *		data	= Request Sense reply
 * Return    :	error code
 *
 ***********************************************************************/

word
CheckError (DriveDesc * dd, CmdInfo * cmd, byte * Data, bool info)
{

#if	DEBUG & DEBUG_SENSE
    char           *fname = "CheckError";
    char            buf[256];
    char           *bp = buf;

#endif
    ErrorInfo      *err = dd->Info->Errors;
    CondInfo       *cnd;

#if	DEBUG & DEBUG_SENSE
    bool            ext = (Data[0] & 0x70) == 0x70;
    int             i = 0;

#endif

#if	DEBUG & DEBUG_SENSE
    if (info) {
	bp = AddStr (bp, fname);
	bp = AddStr (bp, " #");
	bp = AddInt (bp, dd->DriveNum);
	bp = AddStr (bp, " Command '");
	bp = AddStr (bp, cmd->Name);
	bp = AddStr (bp, "'\nSense");
	for (i = 0; i < 4; i++) {
	    if (0 == (i % 4))
	        *bp++ = '\n';
	    else
	        *bp++ = ' ';
	    bp = AddHex (bp, Data[i], 2);
	}
	if (ext)			/* extended Sense Data		 */
	    for (; i < Data[7] + 8; i++) {
                if (0 == (i % 4))
	            *bp++ = '\n';
	        else
	            *bp++ = ' ';
		bp = AddHex (bp, Data[i], 2);
	    }

	bp = AddStr (bp, "\nSense Key ");
	bp = AddHex (bp, Data[2] & 0x0f, 2);

	if (ext && Data[7] > 5) {
	    bp = AddStr (bp, ", Error Code ");
	    bp = AddHex (bp, Data[12], 2);
	}
	if (ext) {
	    bp = AddStr (bp, ", Address ");
	    bp = AddInt (bp, (Data[3] << 24) + (Data[4] << 16)
			 + (Data[5] << 8) + Data[6]);
	}
    }
#endif

    while (err != NULL) {		/* check defined errors		 */
	cnd = err->Conditions;		/* get first condition		 */
	while (cnd != NULL) {
	/* check condition		 */
	    if ((Data[cnd->Offset] & cnd->Mask) != cnd->Value)
		goto nexterr;		/* not met: check next error	 */

	    cnd = cnd->Next;		/* step to next condition	 */
	}

#if	DEBUG & DEBUG_SENSE
	if (info && err->Code) {
	    bp = AddStr (bp, ", Helios Code ");
	    bp = AddHex (bp, err->Code, 8);
	    *bp = '\0';
	    IOdebug (buf);
	}
#endif

    /* here all conditions have are	 */
	return err->Code;		/* met, return the ErrCode.	 */

nexterr:				/* condition has failed:	 */
	err = err->Next;		/* check next defined error	 */
    }

#if	DEBUG & DEBUG_SENSE
    if (info) {
	bp = AddStr (bp, ": Unknown Helios Code ");
	bp = AddHex (bp, EC_Error + SS_Device, 8);
	*bp = '\0';
	IOdebug (buf);
    }
#endif

    return EC_Error + SS_Device;	/* return unspecified error	 */
    cmd = cmd;
    info = info;
}

/************************************************************************
 * EXECUTE A SINGLE SCSI COMMAND
 *
 * - Set up the RCB and execute the request. If Scsi Status is not ok,
 *   execute a Request_Sense Command to get the error code.
 * - Extract result parameters for some known commands for the Drive
 *   Descriptor if necessary.
 *
 * Parameter :	dd	= Drive Descriptor
 *		rcb	= Request Control Block
 *		cmd	= Command Info
 * Return    :	error code
 *
 ***********************************************************************/

word
Do_Command (DriveDesc * dd, RCB * rcb, CmdInfo * cmd)
{
    RCB            *crcb;
    word            code;
    word            retries = 5;
    word            sense;
    word            status;

    code = cmd->Code;

/*FIXME*/
/*IOdebug ("Do_Command %s", cmd->Name);*/
	
    forever
    {
	crcb = rcb;			/* use passed Request		 */
	sense = FALSE;
	SetupRCB (dd, crcb, cmd);	/* init common part of RCB	 */
	switch (code) {			/* check for standard commands	 */
	default:			/* unknown cmds get no params	 */
	case CC_Test_Unit_Ready:
	case CC_Rewind:
	case CC_Prevent_Media_Removal:
	case CC_Allow_Media_Removal:
	    rcb->Size = cmd->DataSize < 0 ? 0 : cmd->DataSize;
	    rcb->Data = dd->DefData;
	    if (!rcb->Read && cmd->DataSize > 0)
		memcpy (rcb->Data, cmd->Data, cmd->DataSize);
	    break;

	case CC_Request_Sense:
	    rcb->Size = cmd->DataSize < 0 ? 4 : cmd->DataSize;
	    rcb->Data = dd->DefData;
	    SetupRandomSizes (rcb, 0, cmd->DataSize < 0 ? 0 : cmd->DataSize);
	    break;

	case CC_Inquiry:
	case CC_Mode_Sense:
	    rcb->Size = cmd->DataSize < 0 ? 0 : cmd->DataSize;
	    rcb->Data = dd->DefData;
	    SetupRandomSizes (rcb, 0, rcb->Size);
	    break;

	case CC_Read:
	case CC_Write:
	    rcb->Size = ShL (rcb->Req->Size, dd->UnitShift);
	    rcb->Data = rcb->Req->Buf;
	    if (dd->Type == 0)		/* Random Access Device		 */
		SetupRandomSizes (rcb, rcb->Start, ShR (rcb->Size, dd->BlkShift));
	    else			/* Sequential Access Device	 */
		SetupSequentialSize (rcb, ShR (rcb->Size, dd->BlkShift));
	    break;

	case CC_Verify:
	    if (dd->Type == 0)		/* Random Access Device		 */
		SetupRandomSizes (rcb, rcb->Start, rcb->Size);
	    else			/* Sequential Access Device	 */
		SetupSequentialSize (rcb, rcb->Size);
	    rcb->Data = rcb->Req->Buf;
	    rcb->Size = 0;
	    break;

	case CC_Format:
	    rcb->Size = rcb->Req->Size;
	    rcb->Data = rcb->Req->Buf;
	    break;

	case CC_Reassign_Blocks:
	case CC_Read_Capacity:
	    rcb->Size = 8;
	    rcb->Data = dd->DefData;
	    break;

	case CC_Write_Filemarks:
	    rcb->Size = 0;
	    rcb->Data = dd->DefData;
	    SetupSequentialSize (rcb, rcb->Req->Size);
	    break;

	case CC_Space:
	    rcb->Size = 0;
	    rcb->Data = dd->DefData;
	    if (rcb->Req->Pos & 3)
		SetupSequentialSize (rcb, rcb->Req->Size);
	    else
		SetupSequentialSize (rcb, ShL (rcb->Req->Size, dd->PosShift));
	    rcb->Regs[WR_CDB02] |= rcb->Req->Pos & 3;
	    break;
	}
retry:

/*FIXME*/
/*IOdebug("<Xr");*/
	
	ExecRequest (dd, crcb);		/* execute Request		 */

/*FIXME*/
/*IOdebug(">Xr (%x)", crcb->Status);*/

	if ((status = crcb->Status) < 0)/* status is ErrCode	 	*/
	    return status;

	switch (status &= 0x1e) {	/* status is Scsi Status	 */
	case 0x00:			/* Good				 */
	case 0x04:			/* Condition Met / Good		 */
	case 0x10:			/* Intermediate / Good		 */
	case 0x14:			/* Interm. / Cond. Met / Good	 */
	    if (sense) {		/* Check Sense, status ok
					 * scan for error codes		 */
		sense = CheckError (dd, cmd, dd->ChkData, TRUE);
		if ((sense & EC_Mask) != EC_Recover)
		{
		    return sense;
		}
		break;			/* ErrCode == 0, retry		 */
	    }
	    goto done;			/* status is OK			 */

	case 0x02:			/* Check Condition		 */
	    if (sense)			/* after Check Condition: fatal	 */
		return EC_Error + SS_Device + EG_Broken + EO_Drive;

					/* setup Request Sense command	 */
	    crcb = dd->ChkRCB;		/* using Chk RCB		 */
	    SetupRCB (dd, crcb, dd->DefCmds[CC_Request_Sense]);
	    sense = dd->DefCmds[CC_Request_Sense]->DataSize;
	    crcb->Size = sense < 0 ? 4 : sense;
	    crcb->Data = dd->ChkData;
	    SetupRandomSizes (crcb, 0, crcb->Size);
	    sense = TRUE;
	    goto retry;  		/* exec. Request Sense command	*/

 	}
        if (retries-- <= 0)
            break;
        Delay (OneSec);
    }
/* 
 * Here is sense != 0 or status bad after 5 retries	 
 */
    switch (status) {
    case 0x08:				/* Busy				 */
	return EC_Error + SS_Device + EG_Timeout + EO_Drive;
    case 0x18:				/* Reservation Conflict		 */
	return EC_Error + SS_Device + EG_InUse + EO_Drive;
    default:				/* Busy, Reserv. Conflict et al	 */
	return EC_Error + SS_Device + EG_Execute + EO_Request;
    }
done:

/*FIXME*/
/*IOdebug ("Do_Command ok");*/

    switch (code) {			/* process Scsi results		 */
    case CC_Request_Sense:
	return CheckError (dd, cmd, dd->DefData, FALSE);

    case CC_Inquiry:
	if (strncmp (&rcb->Data[8], dd->Info->Ident, strlen (dd->Info->Ident))) {
	    IOdebug ("Do_Command #%d: Inquiry: got '%s', expected '%s' !",
		     dd->DriveNum, &rcb->Data[8], dd->Info->Ident);
	    return EC_Error + SS_Device + EG_Unknown + EO_Drive;
	}
	if (dd->Type < 0)		/* Type already checked ?	 */
	    dd->Type = rcb->Data[0];

	if (dd->Removable < 0)		/* Loadable already checked ?	 */
	    dd->Removable = rcb->Data[1] >> 7;
	break;

    case CC_Mode_Sense:
	if (dd->Protected < 0)		/* Protected checked ?		 */
	    dd->Protected = rcb->Data[2] >> 7;

	if (rcb->Data[3] == 8) {	/* Block Descriptor available	 */
	    if (dd->BlkCount <= 0)	/* BlockCount checked ?		 */
		dd->BlkCount = (rcb->Data[5] << 16) +
		  (rcb->Data[6] << 8) + rcb->Data[7];

	    if (dd->BlkSize <= 0) {	/* BlockSize checked ?		 */
		dd->BlkSize = (rcb->Data[9] << 16) +
		  (rcb->Data[10] << 8) + rcb->Data[11];
	    /* check for variable size	 */
		if (dd->Type && dd->BlkSize == 0)
		    dd->BlkSize = 1;
	    }
	}

	IOdebug ("Do_Command Mode_Sense: prot. %d bcnt %d bsize %d", 
		dd->Protected, dd->BlkCount, dd->BlkSize);
	break;

    case CC_Read_Capacity:
	if (dd->BlkCount <= 0)		/* BlockCount already checked ?	 */
	    dd->BlkCount =
	      (rcb->Data[0] << 24) + (rcb->Data[1] << 16) +
	      (rcb->Data[2] << 8) + rcb->Data[3] + 1;

	if (dd->BlkSize <= 0)		/* Blocksize already checked ?	 */
	    dd->BlkSize =
	      (rcb->Data[4] << 24) + (rcb->Data[5] << 16) +
	      (rcb->Data[6] << 8) + rcb->Data[7];
	break;

    case CC_Prevent_Media_Removal:
	dd->Locked = TRUE;
	break;

    case CC_Allow_Media_Removal:
	dd->Locked = FALSE;
	break;
    }
    return 0;				/* ready, everything fine.	 */
}

/************************************************************************
 * HANDLE A REQUEST AND ITS COMMAND SEQUENCE
 *
 * - Step through the items of the request and execute the single
 *   commands.
 *
 * Parameter :	dd	= Drive Descriptor
 *		ri	= Request Info
 *		rcb	= Request Control Block
 * Result    :	error code
 *
 ***********************************************************************/

word
Do_Sequence (DriveDesc * dd, RCB * rcb, ReqInfo * ri)
{
    ItemInfo       *item;
    word            result;

    item = ri->Items;			/* get the first item		 */
    while (item != NULL) {
    /* execute a command		 */
	if ((result = Do_Command (dd, rcb, item->Item)) != 0)
	    return result;		/* return error code		 */

	item = item->Next;		/* step to next item		 */
    }

    return 0;				/* ready, everything ok.	 */
}

/************************************************************************
 * SELECT THE NEXT REQUEST FROM THE REQUEST QUEUE
 *
 * - This function implements an Elevator Algorithm to reduce
 *   head movement.
 * - Only Read and Write requests are processed, other requests are
 *   returned if they reach the head of the Request queue (FIFO).
 *   The search extends to the end of the request queue or to the first
 *   non-IO request.
 * - The Elevator Algorithm acts as follows :
 *   - search the request with the smallest difference between
 *     start position and current head position in the direction of
 *     the last head movement and return this request.
 *   - If no request in the last head movement direction can be found,
 *     switch the direction and take the next backward request.
 *
 * Parameter :	dd	= Drive Descriptor
 * Result    :	rcb	next Request's Control block
 *
 ***********************************************************************/

RCB            *
NextRequest (DriveDesc * dd)
{
    RCB            *last = NULL;	/* next request with delta < 0	 */
    RCB            *next = NULL;	/* next request with delta > 0	 */
    word            dlast = MinInt;	/* largest negative delta	 */
    word            dnext = MaxInt;	/* smallest positive delta	 */
    RCB            *test = Head_ (RCB, *dd->ReqList);
    word            code;
    word            delta;
    word            dir = dd->LastDir;	/* last direction		 */
    word            pos = dd->LastPos;	/* current head position	 */

    while (!EndOfList_ (test)) {	/* search through the ReqList	 */
	code = test->Req->DevReq.Request & FG_Mask;
	if (code != FG_Read && code != FG_Write)
	    break;			/* break if no IO-request	 */

	delta = test->Start - pos;	/* calc delta			 */
	if (delta >= 0 && delta < dnext) {	/* smaller than dnext ?	 */
	    next = test;		/* yes, save delta & rcb ptr	 */
	    dnext = delta;
	}
	elif (delta <= 0 && delta > dlast) {	/* larger than dlast ?	 */
	    last = test;		/* yes, save delta & rcb ptr	 */
	    dlast = delta;
	}
	test = Next_ (RCB, test);	/* step to next request		 */
    }

    if (next || last) {			/* an IO request found...	 */
	if (dir > 0) {			/* upward movement		 */
	    if (next == NULL) {		/* no upward request ?		 */
		dd->LastDir = -1;	/* change direction to down	 */
		test = last;
	    } else
		test = next;
	} else {			/* downward movement		 */
	    if (last == NULL) {		/* no downward request ?	 */
		dd->LastDir = 1;	/* change direction to up	 */
		test = next;
	    } else
		test = last;
	}
    /* save new request end pos	 */
	dd->LastPos = test->Start + test->Req->Size;
    }
    Remove ((Node *) test);
    return test;			/* return selected request	 */
}

/************************************************************************
 * REASSIGN THE BLOCK SPECIFIED IN CHKDATA
 *
 * - Check whether a reassignment is possible. Errors of EC_Warn are
 *   assumed to be recoverable by reassigning the defect block.
 * - Get the defect block number from ChkData and issue a Reassign Blocks
 *   command.
 *   commands.
 *
 * Parameter :	dd	= Drive Descriptor
 *		rcb	= Request Control Block
 *		result	= last error code
 * Result    :	< 0	error code
 *		>= 0	number of reassigned block
 *
 ***********************************************************************/

word
Do_Reassign (DriveDesc * dd, RCB * rcb, word result)
{
    char           *fname = "Do_Reassign";
    word            ra_blk;


    if (((result & EC_Mask) > EC_Warn)	/* check possible reassign	 */
	|| (dd->DefCmds[CC_Reassign_Blocks] == NULL)
	|| ((dd->ChkData[0] & 0x80) == 0))
	return (result & ~EC_Mask) | EC_Error;

    if ((dd->ChkData[0] & 0x70) == 0x70)/* extended Sense	 	*/
	ra_blk = (dd->ChkData[3] << 24) + (dd->ChkData[4] << 16)
	  + (dd->ChkData[5] << 8) + dd->ChkData[6];
    else				/* non-extended Sense	 	*/
	ra_blk = ((dd->ChkData[1] << 16) & 0x1f)
	  + (dd->ChkData[2] << 8) + dd->ChkData[3];

    memset (dd->DefData, 0, 8);		/* clear used data block	 */
    dd->DefData[3] = 4;			/* prepare Data buffer		 */
    dd->DefData[4] = ra_blk >> 24;
    dd->DefData[5] = ra_blk >> 16;
    dd->DefData[6] = ra_blk >> 8;
    dd->DefData[7] = ra_blk;
/* reassign the defect block	 */
    result = Do_Command (dd, rcb, dd->DefCmds[CC_Reassign_Blocks]);
    if (result) {			/* reassign failed		 */
	IOdebug ("%s #%d: Reassigning block %d failed !",
		 fname, dd->DriveNum, ra_blk);
	return EC_Error + SS_Device + EG_Broken + EO_Medium;
    }
    return ra_blk;
}

word
Do_Read (DriveDesc * dd, RCB * rcb)
{
    char           *fname = "Do_Read";
    bool            ra_done = FALSE;
    word            result;

retry:
    result = Do_Command (dd, rcb, dd->DefCmds[CC_Read]);

    if (!result  			/* no error or recovered error	 */
      || (result & EC_Mask) == EC_Recover) {
	rcb->Req->Actual = rcb->Req->Size;
	return ra_done ? EC_Recover + SS_Device + EG_Broken + EO_Medium 
		       : Err_Null;
    }

    if ((result & EC_Mask) > EC_Warn	/* real error or no disk	 */
      || dd->Type != 0)
	return result;
    IOdebug ("%s #%d: Failed to read block %d (%x), trying to reassign !",
    	fname, dd->DriveNum, rcb->Start, result);
					/* try to reassign bad block	 */
    if ((result = Do_Reassign (dd, rcb, result)) < Err_Null)
	return result;			/* reassign failed...		 */

    ra_done = TRUE;
    IOdebug ("%s #%d: Block %d reassigned !", fname, dd->DriveNum, result);
    IOdebug ("Drive may become unusable, make sure to have a backup !");
    goto retry;
}

word
Do_Write (DriveDesc * dd, RCB * rcb)
{
    char           *fname = "Do_Write";
    bool            ra_done = FALSE;
    word            result;

retry:
    result = Do_Command (dd, rcb, dd->DefCmds[CC_Write]);

    if (!result 			/* no error or recovered error	 */
      || (result & EC_Mask) == EC_Recover) {
	rcb->Req->Actual = rcb->Req->Size;
	return ra_done ? EC_Recover + SS_Device + EG_Broken + EO_Medium : Err_Null;
    }

    if ((result & EC_Mask) > EC_Warn 	/* real error or no disk	 */
      || dd->Type != 0)
	return result;
/* try to reassign bad block	 */
    if ((result = Do_Reassign (dd, rcb, result)) < Err_Null)
	return result;			/* reassign failed...		 */

    ra_done = TRUE;
    IOdebug ("%s #%d: Block %d reassigned !", fname, dd->DriveNum, result);
    IOdebug ("Drive may become unusable, make sure to have a backup !");
    goto retry;
}

word
Do_Seek (DriveDesc * dd, RCB * rcb)
{
    if ((dd->Type) < 1)
	return EC_Error + SS_Device + EG_FnCode;
    if (rcb->Req->Pos == 0 && rcb->Req->Size == 0)
	return Do_Command (dd, rcb, dd->DefCmds[CC_Rewind]);
    else
	return Do_Command (dd, rcb, dd->DefCmds[CC_Space]);
}

word
Do_WriteMark (DriveDesc * dd, RCB * rcb)
{
    if ((dd->Type) < 1)
	return EC_Error + SS_Device + EG_FnCode;
    return Do_Command (dd, rcb, dd->DefCmds[CC_Write_Filemarks]);
}

word
Do_Format (DriveDesc * dd, RCB * rcb)
{
    char           *fname = "Do_Format";
    word            size, pos, length;
    word            ra_cnt, ra_sum, ra_old, ra_new;
    CmdInfo        *cmd;
    byte           *DList = rcb->Data;
    word            result = 0;

/* check Format cmd defined	 */
    if ((cmd = dd->DefCmds[CC_Format]) == NULL) {

#if	DEBUG & DEBUG_FMT
	IOdebug ("%s #%d: Format Command not defined !", fname, dd->DriveNum);
#endif

	return EC_Error + SS_Device + EG_WrongFn;
    }
    if (dd->Protected != 0) {

#if	DEBUG & DEBUG_FMT
	IOdebug ("%s #%d: Drive is write-protected !", fname, dd->DriveNum);
#endif

	return EC_Error + SS_Device + EG_Protected;
    }
    rcb->Data = dd->DefData;

    if (dd->DefReqs[RI_Format] != NULL)	/* execute Format init sequence	 */
	if ((result = Do_Sequence (dd, rcb, dd->DefReqs[RI_Format])) != 0) {

#if	DEBUG & DEBUG_FMT
	    IOdebug ("%s #%d: Format Sequence returned error %x",
		     fname, dd->DriveNum, result);
#endif

	    return result;
	}
    rcb->Data = DList;
    if ((result = Do_Command (dd, rcb, dd->DefCmds[CC_Format])) != 0) {

#if	DEBUG & DEBUG_FMT
	IOdebug ("%s #%d: Format command returned error %x",
		 fname, dd->DriveNum, result);
#endif

	return result;
    }
    if (dd->DefCmds[CC_Verify] == NULL) {

#if	DEBUG & DEBUG_FMT
	IOdebug ("%s #%d: Verify command not defined, Drive not verified.",
		 fname, dd->DriveNum);
#endif

	return 0;
    }
    size = dd->BlkCount;
    pos = 0;
    length = 4096;
    ra_cnt = 0;
    ra_sum = 0;
    ra_old = -1;

    rcb->Data = dd->DefData;
    if ((pos + length) > size)
	length = size - pos;

    while (length > 0) {
	rcb->Start = pos;
	rcb->Size = length;
	result = Do_Command (dd, rcb, dd->DefCmds[CC_Verify]);

	unless (result) {		/* everything fine: verify next	 */
	    pos += length;
	    if ((pos + length) > size)
		length = size - pos;
	    continue;
	}

	if ((result = Do_Reassign (dd, rcb, result)) < Err_Null)
	    return result;

	if (result == ra_old) {
	    if (++ra_cnt > 8) {		/* more than 8 times		 */

#if	DEBUG & DEBUG_FMT
		IOdebug ("%s #%d: Block %d still defect after reassigning %d times !",
			 fname, dd->DriveNum, result, ra_cnt);
#endif

		return EC_Error + SS_Device + EG_Broken + EO_Medium;
	    }
	} else {
	    ra_cnt = 0;			/* reset reassignment counter	 */
	    ra_sum++;
	}
	ra_old = result;

	if ((pos = ra_new - 10) < 0)
	    pos = 0;
    }

#if	DEBUG & DEBUG_FMT
    IOdebug ("%s #%d: Drive formatted with %d blocks reassigned.",
	     fname, dd->DriveNum, ra_sum);
#endif

    return 0;
    fname = fname;
}

/************************************************************************
 * HANDLE ALL REQUESTS FOR A DRIVE
 *
 * - Wait for the request queue fo be non-empty and take a request
 *   from the queue.
 * - Use the appropriate sequence to perform the request and
 *   call back to the Client if the request is completed.
 *
 * Parameter :	dd	= Drive Descriptor
 *
 ***********************************************************************/

void
DriveHandler (DriveDesc * dd)
{
    char           *fname = "DriveHandler";
    bool            random = dd->Type == 0;
    RCB            *rcb;
    DiscReq        *req;
    word            FnCode;

    forever
    {
	Wait (dd->ReqCount);		/* wait for the next Request	 */
	if (random)
	    rcb = NextRequest (dd);
	else
	    rcb = (RCB *) RemHead (dd->ReqList);
	req = rcb->Req;
	req->Actual = 0;
	switch (FnCode = req->DevReq.Request) {
	case FG_Read:
	    req->DevReq.Result = Do_Read (dd, rcb);
	    break;
	case FG_Write:
	    req->DevReq.Result = Do_Write (dd, rcb);
	    break;
	case FG_Seek:
	    req->DevReq.Result = Do_Seek (dd, rcb);
	    break;
	case FG_Format:
	    req->DevReq.Result = Do_Format (dd, rcb);

	/*
	 * unless ( req->DevReq.Result ) req->Actual = req->Size;
	 */
	 
	    break;
	case FG_WriteMark:
	    req->DevReq.Result = Do_WriteMark (dd, rcb);
	    break;
	case FG_Close:
	    req->DevReq.Result = Do_Sequence (dd, rcb, dd->DefReqs[RI_Close]);
	    break;
	default:
	    IOdebug ("%s #%d: unknown Request code 0x%x !",
		     fname, dd->DriveNum, req->DevReq.Request);
	    req->DevReq.Result = EC_Error + SS_Device + EG_FnCode;
	    break;
	}

	(*req->DevReq.Action) (req);
	Free (rcb);

	if (FnCode == FG_Close) {
	    dd->Closed = FALSE;
	    return;
	}
    }
}

/************************************************************************
 * OPEN THE DRIVE, INITIALISE IT AND START THE DRIVE HANDLER
 *
 * - Step through the items of the request and execute the single
 *   commands. If a command returns a non-zero result, execute the
 *   Request_Sense command. If Request_Sense returns an error code,
 *   abort processing and save the error code as result, otherwise,
 *   ignore the erronous command.
 *
 * Parameter :	dd	= Drive Descriptor
 *		cmd	= Command Info
 *		rcb	= Request Control Block
 * Result    :	error code as stored in the DevReq struct.
 *
 ***********************************************************************/

bool
StartDriveHandler (DriveDesc * dd, word dnum, DiscReq * req, LoadResult * lr)
{
    char           *fname = "StartDriveHandler";
    CmdInfo        *cmd;
    ReqInfo        *ri;
    RCB             rcb;
    word            result;
    word            i;

    for (cmd = dd->Info->Commands; cmd != NULL; cmd = cmd->Next) {
	if (cmd->Code > CC_Unknown)
	    dd->DefCmds[cmd->Code] = cmd;
    }

    for (ri = dd->Info->Requests; ri != NULL; ri = ri->Next) {
	switch (ri->FnCode) {
	case FG_Format:
	    dd->DefReqs[RI_Format] = ri;
	    break;
	case FG_Open:
	    dd->DefReqs[RI_Open] = ri;
	    break;
	case FG_Close:
	    dd->DefReqs[RI_Close] = ri;
	    break;
	}
    }

    if (dd->DefReqs[RI_Open] == NULL || dd->DefReqs[RI_Close] == NULL) {
	IOdebug ("%s #%d: no Commands defined for FG_Open or FG_Close !",
		 fname, dd->DriveNum);
	return EC_Error + SS_Device + EG_WrongFn + EO_Drive;
    }
    lr->Error = FALSE;
    lr->Raw = dd->Type != 0;
    lr->Loaded = FALSE;
    lr->Protected = FALSE;
    lr->Formatted = FALSE;
    lr->NotLocked = FALSE;

    dd->BlkCount = -1;			/* reset Drive variables	 */
    dd->BlkSize = -1;
    dd->Removable = -1;
    dd->Formatted = -1;
    dd->Locked = -1;
    dd->Protected = -1;

    rcb.Req = req;

    lr->Error = Do_Sequence (dd, &rcb, dd->DefReqs[RI_Open]);
    if (lr->Error) {
	IOdebug ("%s #%d: failed to open Drive ( 0x%x ) !",
		 fname, dd->DriveNum, lr->Error);
	return Err_Null;
    }
    if (dd->Type == 0 && dd->BlkCount < 0) {
	IOdebug ("%s #%d: failed to detect Disc size !", fname, dd->DriveNum);
	lr->Error = EC_Error + SS_Device + EG_Open + EO_Medium;
	return Err_Null;
    }
    if (dd->BlkSize <= 0) {
	IOdebug ("%s #%d: failed to detect Block size !", fname, dd->DriveNum);
	lr->Error = EC_Error + SS_Device + EG_Open + EO_Medium;
	return Err_Null;
    }
    for (i = 1, result = 0; i < dd->BlkSize && i > 0; i <<= 1, result++);
    if (i != dd->BlkSize) {
	IOdebug ("%s #%d: Blocksize %d is not a power of 2 !",
		 fname, dd->DriveNum, dd->BlkSize);
	lr->Error = EC_Error + SS_Device + EG_Parameter;
	return Err_Null;
    }
    dd->BlkShift = result;
    dd->PosShift = dd->UnitShift - result;
    if (dd->Protected < 0) {
	IOdebug ("%s #%d: Write Protection not checked !", fname, dd->DriveNum);
	lr->Error = EC_Error + SS_Device + EG_Open + EO_Medium;
	return Err_Null;
    }
    if (dd->Type == 0) {		/* Random Access Device		 */
	byte           *TestData = Malloc (ShL (1, dd->UnitShift));
	DiscReq         myreq;

	unless (TestData) {
	    lr->Error = EC_Error + SS_Device + EG_NoMemory;
	    return 0;
	}
	myreq.Pos = 0;
	myreq.Size = 1;
	myreq.Buf = TestData;
	rcb.Req = &myreq;
	result = Do_Command (dd, &rcb, dd->DefCmds[CC_Read]);
	unless (result) {
	    myreq.Pos = ShR (dd->BlkCount, dd->PosShift) - 1;
	    myreq.Size = 1;
	    myreq.Buf = TestData;
	    rcb.Req = &myreq;
	    result = Do_Command (dd, &rcb, dd->DefCmds[CC_Read]);
	    unless (result)
	      dd->Formatted = TRUE;
	}
	Free (TestData);
    }
    if (!Fork (2560, DriveHandler, 8, dd, dnum)) {
	IOdebug ("%s #%d: failed to fork DriveHandler !", fname, dd->DriveNum);
	return EC_Error + SS_Device + EG_NoMemory + EO_Drive;
    }
    dd->Loaded++;			/* count completed Load		 */

    lr->Loaded = TRUE;			/* complete LoadResult		 */
    lr->Removable = dd->Removable;
    lr->Protected = dd->Protected;
    lr->Formatted = dd->Formatted;
/* unlocked if removable	 */
/* and not locked		 */
    lr->NotLocked = dd->Removable > 0 && dd->Locked < 1;
    return Err_Null;
}

/*--- end of device.c ---*/
