/*
 * $Header: /Chris/00/helios/msc/RCS/msc.c,v 2.0 91/08/21 18:04:06 chris Exp
 * Locker: chris $
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
** msc.c								**
**									**
**	- MSC Device Driver						**
**									**
**************************************************************************
** HISTORY   :								**
** -----------								**
** Author    :	xx/xx/90 : C. Fleischer					**
*************************************************************************/

#include "msc.h"

#ifdef Driver
#include "rdscsi.c"			/* Reading of ScsiInfo file	 */
#include "scsi.c"			/* SCSI access and ScsiHandler	 */
#include "device.c"			/* single Device controller	 */
#include "utility.c"			/* Process handling functions	 */

#else

Semaphore       IOlock;

#endif

void            DevOperate (DiscDCB * dcb, DiscReq * req);
word            DevClose (DiscDCB * dcb);

/************************************************************************
 * CHECK WHETHER AN SCSI ID HAS BEEN USED BEFORE AND MARK IT AS USED
 *
 * - Check and mark a single ID/LUN combination.
 *
 * Parameter :	used	= array of usage marks
 *		ID	= SCSI ID for a Device
 * Return    :	TRUE	ID was free and has been marked
 *		FALSE	ID could not be marked
 *
 ***********************************************************************/

static          bool
MarkScsiID (byte * used, word ID)
{
    word            addr = GetID (ID);
    word            mask = 1 << GetLUN (ID);

    if ((used[addr] & mask) != 0) {
	IOdebug ("LUN %d of Scsi Address %d already used !",
		 GetLUN (ID), GetID (ID));
	return FALSE;
    }
    used[addr] |= mask;
    return TRUE;
}

/************************************************************************
 * CHECK WHETHER AN SCSI ID IS MARKED AS USED AND RELEASE THE MARK
 *
 * - Check and unmark a single ID/LUN combination.
 *
 * Parameter :	used	= array of usage marks
 *		ID	= SCSI ID for a Device
 *
 ***********************************************************************/

static void
UnmarkScsiID (byte * used, word ID)
{
    word            addr = GetID (ID);
    word            mask = 1 << GetLUN (ID);

    if ((used[addr] & mask) == 0) {
	IOdebug ("LUN %d of Scsi Address %d not used !",
		 GetLUN (ID), GetID (ID));
	return;
    }
    used[addr] &= ~mask;
}


/************************************************************************
 * COUNT THE NUMBER OF INFO NODES IN A CHAIN
 *
 * Parameter :	info	= ptr to a Device Info structure
 * Return    :	number of Info entries
 *
 ***********************************************************************/

static          word
CountInfos (word * info)
{
    word            count = 1;

    forever
    {
	if (*info == -1)
	    return count;
	info = (word *) RTOA (*info);
	count++;
    }
}

/************************************************************************
 * INITIALISE THE DEVICE DRIVER
 *
 * - Load the ScsiInfo, allocate and initialise the DiscDCB.
 * - All static data structures are allocated in one piece. Therefore,
 *   this function splits the allocated memory into the different
 *   structures and initialises the DCB, SDB and DriveDesc structures.
 *
 * Parameter :	dev	= Device structure
 *		ddi	= DevInfo's DiscDeviceInfo
 *
 ***********************************************************************/

DiscDCB        *
DevOpen (Device * dev, DiscDevInfo * ddi)
{
    char           *fname = "MSC Device Driver";
    char            buf[32];
    int             DCBSize;		/* cumulated statics size	 */
    void           *si;			/* SCSI Commands info		 */
    InfoNode       *sn;			/* Info for a single Drive	 */
    DriveInfo      *di;			/* DevInfo for Drive		 */
    PartitionInfo  *pi;			/* DevInfo for Partition	 */
    DriveDesc      *dd;			/* Drive Descriptor		 */
    PartDesc       *pd;			/* Partition Descriptor		 */
    DiscDCB        *dcb;		/* our DiscDCB, to be allocated	 */
    int             nDrive;		/* number of Drives		 */
    int             nPart;		/* number of Partitions		 */
    int             id, lun;
    char           *ptr;		/* pointer to split statics	 */
    int             i;

/* load Command info		 */
    if ((si = LoadScsiInfo ()) == NULL) {
	IOdebug ("%s: Failed to load ScsiInfo !", fname);
	return NULL;
    }
    nDrive = CountInfos ((word *) RTOA (ddi->Drives));
    nPart = CountInfos ((word *) RTOA (ddi->Partitions));

    DCBSize = sizeof (DiscDCB);		/* take DiscDCB size		 */
/* add Drive-related sizes	 */
    DCBSize += nDrive * (DriveDescSize + sizeof (Channel) +
		      sizeof (Channel *) + sizeof (RCB *) + sizeof (uword));

/* add Partition-related sizes	 */
    DCBSize += nPart * sizeof (PartDesc);

/* allocate static memory	 */
    if ((dcb = Malloc (DCBSize)) == NULL) {
	IOdebug ("%s: Failed to allocate DiscDCB (%d bytes) !", fname, DCBSize);
	return NULL;
    }
    memset (dcb, 0, DCBSize);		/* clear static memory		 */

    ptr = (char *) dcb + sizeof (DiscDCB);	/* set split ptr	 */

    for (id = 1, i = 0; id < ddi->Addressing; id <<= 1, i++);
    if (id != ddi->Addressing) {
	IOdebug ("%s: Addressing (0x%x) is not a power of 2 !",
		 fname, ddi->Addressing);
	Free (dcb);
	return NULL;
    }
    dcb->UnitSize = ddi->Addressing;
    dcb->UnitShift = i;

    lun = ddi->Controller & 0x0f;
    id = (ddi->Controller >> 4) & 0x0f;
    SetIdent (dcb->ScsiAddress, id, lun);
    MarkScsiID (dcb->used, dcb->ScsiAddress);	/* mark my SCSI ID	 */

    dcb->nDrive = nDrive;
    dcb->nPart = nPart;
/* setup Drive table	 */
    dcb->DriveTab = (DriveDesc *) ptr;
    ptr += nDrive * sizeof (DriveDesc);
/* setup Partn table	 */
    dcb->PartTab = (PartDesc *) ptr;
    ptr += nPart * sizeof (PartDesc);

    dcb->Channels = (Channel **) ptr;
    ptr += nDrive * sizeof (Channel *);

    for (i = 0; i < nDrive; i++) {
	dcb->Channels[i] = (Channel *) ptr;
	ChanReset ((Channel *) ptr);
	ptr += sizeof (Channel);
    }

    dcb->ScsiInfo = si;

    dcb->DCB.Device = dev;		/* init common DCB 	 */
    dcb->DCB.Operate = DevOperate;
    dcb->DCB.Close = DevClose;

    dcb->sdb.OwnId = dcb->ScsiAddress;	/* init sdb		 */
    dcb->sdb.nInOut = nDrive;

#ifndef	EVENTS
    dcb->sdb.Event = &dcb->EventChan;
#else
    dcb->sdb.Event = (Channel *) (MinInt + 0x20);
#endif

    dcb->sdb.InOut = dcb->Channels;

    dcb->sdb.Request = (RCB **) ptr;	/* setup RCB array	 */
    ptr += nDrive * sizeof (RCB *);

    dcb->sdb.DevState = (uword *) ptr;	/* setup DevState array	 */
    ptr += nDrive * sizeof (uword);

    di = (DriveInfo *) RTOA (ddi->Drives);
    dd = dcb->DriveTab;
    i = 0;
    forever				/* for each Drive		 */
    {
	dcb->sdb.DevState[i] = 0x20;	/* default transfer speed	 */
	lun = di->DriveId & 0x0f;
	id = (di->DriveId >> 4) & 0x0f;
	if (id > 7 || lun > 7) {	/* check range			 */
	    IOdebug ("%s: illegal SCSI address %d with lun %d in DevInfo entry for Drive #%d",
		     fname, id, lun, i);
	    Free (dcb);
	    return NULL;
	}
    /* get ScsiDevInfo		 */
	sn = FindScsiInfo (si, di->DriveType);
	if (sn == NULL) {
	    IOdebug ("%s: failed to find ScsiDevInfo entry #%d for Drive #%d",
		     fname, di->DriveType, i);
	    Free (dcb);
	    return NULL;
	}
	dd->Info = (ScsiDevInfo *) sn->Info;

    /* set up DriveDesc struct	 */
	dd->DriveNum = i;
	dd->Chan = dcb->Channels[i];
	dd->Lock = (Semaphore *) ptr;
	ptr += sizeof (Semaphore);
	InitSemaphore (dd->Lock, 1);

	SetIdent (dd->DriveID, id, lun);
	SetSync (dd->DriveID, dd->Info->Synchronous);
	dd->BlkCount = -1;
	dd->BlkSize = -1;
	dd->UnitShift = dcb->UnitShift;
	dd->BlkShift = 0;
	dd->PosShift = dcb->UnitShift;
	dd->Loaded = FALSE;
	dd->Closed = FALSE;
	dd->Type = dd->Info->Type;
	dd->Formatted = -1;
	dd->Removable = -1;
	dd->Locked = -1;
	dd->Protected = -1;

	dd->ReqCount = (Semaphore *) ptr;
	ptr += sizeof (Semaphore);
	InitSemaphore (dd->ReqCount, 0);

	dd->ReqList = (List *) ptr;
	ptr += sizeof (List);
	InitList (dd->ReqList);

	dd->DefCmds = (CmdInfo **) ptr;
	ptr += DefCmdsSize * sizeof (CmdInfo *);

	dd->DefReqs = (ReqInfo **) ptr;
	ptr += DefReqsSize * sizeof (ReqInfo *);

	dd->DefData = (byte *) ptr;
	ptr += DefDataSize;

	dd->ChkRCB = (RCB *) ptr;
	ptr += sizeof (RCB);

	dd->ChkData = (byte *) ptr;
	ptr += ChkDataSize;

	dd->TestData = NULL;

	if (di->Next == -1)		/* step to next Drive		 */
	    break;
	di = (DriveInfo *) RTOA (di->Next);
	i++;
	dd++;
    }
/* initialise Partition table	 */
    pi = (PartitionInfo *) RTOA (ddi->Partitions);
    pd = dcb->PartTab;
    i = 0;
    forever				/* for each Partition		 */
    {
	if (pi->Drive >= nDrive) {	/* check Drive number	 */
	    IOdebug ("%s: failed to find Drive %d in DevInfo drivelist",
		     fname, pi->Drive);
	    Free (dcb);
	    return NULL;
	}
    /* complete PartDesc struct	 */
	pd->DriveNum = pi->Drive;
	pd->Drive = &dcb->DriveTab[pi->Drive];

	if (pd->Drive->Type)		/* adapt EndCyl for Tapes	 */
	    pi->EndCyl = -1;
	pd->FirstUnit = pi->StartCyl;
	pd->LastUnit = 0;
	pd->DILastUnit = pi->EndCyl;

	if (pi->Next == -1)		/* step to next Partition	 */
	    break;
	pi = (PartitionInfo *) RTOA (pi->Next);
	i++;
	pd++;
    }

    if (!StartScsiHandler (dcb)) {	/* start SCSI Handler process	 */
	IOdebug ("%s: failed to start ScsiHandler", fname);
	Free (dcb);
	return NULL;
    }
    IOdebug ("(%s) : Helios MSC Device Driver opened.", stime (buf));
    return dcb;
}

void
DevOperate (DiscDCB * dcb, DiscReq * req)
{
    char           *fname = "DevOperate";
    RCB            *rcb = NULL;
    LoadResult     *lr;
    DriveDesc      *dd;
    PartDesc       *pd;
    word            FnCode = req->DevReq.Request & FG_Mask;
    word            PIndex = req->DevReq.SubDevice;
    word            Result;

    if (dcb->Closing) {			/* check Driver closing down	 */
	IOdebug ("%s: Device driver terminating !", fname);
	req->DevReq.Result = EC_Error + SS_Device + EG_Invalid;
	(*req->DevReq.Action) (req);
	return;
    }
    if (PIndex >= dcb->nPart) {		/* check valid Partition #	 */
	IOdebug ("%s Drive #%d: invalid partition number #%d !",
		 fname, pd->DriveNum, PIndex);
	req->DevReq.Result = EC_Error + SS_Device + EG_Parameter;
	(*req->DevReq.Action) (req);
	return;
    }
    pd = &dcb->PartTab[PIndex];		/* get PartDesc ptr		 */
    dd = pd->Drive;			/* get DriveDesc ptr		 */

    Wait (dd->Lock);			/* get access to DriveDesc	 */

    unless (dd->Loaded) {		/* special cases		 */
	if (dd->Closed) {		/* drive closing down		 */
	    IOdebug ("%s: Drive #%d terminated !", fname, pd->DriveNum);
	    Result = EC_Error + SS_Device + EG_Invalid + EO_Drive;
	} else {			/* drive not loaded		 */
	    if (FnCode == FG_Open) {	/* Open request: start handler	 */
		unless (MarkScsiID (dcb->used, dd->DriveID)) {
		    ((LoadResult *) req->Buf)->Error =
		      EC_Error + SS_Device + EG_InUse + EO_Drive;
		}
		else
		Result = StartDriveHandler (dd, pd->DriveNum, req,
					    (LoadResult *) req->Buf);

		if (!Result && !((LoadResult *) req->Buf)->Error) {
		    if (dd->Removable != 0 || pd->DILastUnit < 0)
			pd->LastUnit = ShR (dd->BlkCount, dd->PosShift);
		    else
			pd->LastUnit = pd->DILastUnit;
		} else
		    UnmarkScsiID (dcb->used, dd->DriveID);
	    }
	    elif (FnCode == FG_Close)	/* Close request: ignore it	 */
	      Result = Err_Null;

	    else			/* other request: return error	 */
	    {
		IOdebug ("%s: Drive #%d not loaded !", fname, pd->DriveNum);
		Result = EC_Error + SS_Device + EG_WrongFn + EO_Drive;
	    }
	}
	goto done;
    }
    switch (FnCode) {
    case FG_Write:			/* writing Drive Requests	 */
    case FG_WriteMark:
    case FG_Format:
	if (dd->Protected) {		/* check for write protection	 */
	    Result = EC_Error + SS_Device + EG_Protected + EO_Drive;
	    goto done;
	}
    case FG_Read:			/* non-writing Drive requests	 */
    case FG_Seek:
	req->Actual = 0;
	break;

    case FG_Close:			/* Close request, mark State	 */
	if (--dd->Loaded > 0) {
	    Result = Err_Null;
	    goto done;
	}
	dd->Closed = TRUE;
	dd->Loaded = FALSE;
	UnmarkScsiID (dcb->used, dd->DriveID);
	break;

    case FG_GetSize:			/* Getsize can be done here	 */
	Result = pd->LastUnit - pd->FirstUnit + 1;
	req->Actual = dcb->UnitSize;
	goto done;

    case FG_GetInfo:
	{
	    word            i;

	    for (i = 0; i < req->Size && i < 4; i++)
		((byte *) req->Buf)[i] = dd->ChkData[i];
	    if ((dd->ChkData[0] & 0x70) == 0x70)
		for (; i < req->Size && i < dd->ChkData[7] + 8; i++)
		    ((byte *) req->Buf)[i] = dd->ChkData[i];
	    req->Actual = i;
	    Result = Err_Null;
	    goto done;
	}
    case FG_Open:			/* Open an opened Drive:	 */
	Result = Err_Null;
	lr = (LoadResult *) req->Buf;
    /* ok if not (raw or removable)	 */
	lr->Error = (dd->Type > 0 || dd->Removable);
	lr->Raw = dd->Type != 0;
	lr->Removable = dd->Removable;
	lr->Loaded = dd->Loaded;
	lr->Protected = dd->Protected;
	lr->Formatted = dd->Formatted;
	lr->NotLocked = !dd->Locked;
	unless (lr->Error) {
	    dd->Loaded++;

	    if (dd->Removable != 0 || pd->DILastUnit < 0)
		pd->LastUnit = ShR (dd->BlkCount, dd->PosShift);
	    else
		pd->LastUnit = pd->DILastUnit;
	}
	goto done;

    default:				/* unknown request...		 */
	Result = EC_Error + SS_Device + EG_FnCode + EO_Request;
	goto done;
    }
/* allocate RCB for request	 */
    if ((rcb = (RCB *) Malloc (sizeof (RCB))) == NULL) {
	IOdebug ("%s: failed to allocate RCB !", fname);
	Result = EC_Error + SS_Device + EG_NoMemory + EO_Request;
	goto done;
    }
    rcb->Req = req;			/* initialise RCB		 */
    rcb->Start = ShL (req->Pos + pd->FirstUnit, dd->PosShift);

/* add RCB to Drive Queue	 */
    AddTail (dd->ReqList, (Node *) rcb);
    Signal (dd->ReqCount);

    Signal (dd->Lock);			/* release DriveDesc		 */

    return;				/* that's all for now		 */

done:					/* any code in Result:		 */
    Signal (dd->Lock);			/* release DriveDesc		 */
    req->DevReq.Result = Result;	/* store the Result		 */
    (*req->DevReq.Action) (req);	/* and call the Action func.	 */
    return;
}

void
DevSignal (DiscReq * req)
{
    Signal (&req->WaitLock);
}

word
DevClose (DiscDCB * dcb)
{
    char            buf[32];
    int             i;
    DriveDesc      *dd;
    DiscReq         req;
    RCB            *rcb;

    dcb->Closing = TRUE;
    req.DevReq.Request = FG_Close;
    req.DevReq.Action = DevSignal;
    req.DevReq.Timeout = -1;
    req.Pos = 0;
    req.Size = 0;
    req.Buf = NULL;
    InitSemaphore (&req.WaitLock, 0);

    for (i = 0; i < dcb->nDrive; i++) {
	dd = &dcb->DriveTab[i];
	Wait (dd->Lock);
	if (dd->Loaded) {
	    while ((rcb = (RCB *) Malloc (sizeof (RCB))) == NULL)
		Delay (OneSec);
	    rcb->Req = &req;
	    AddTail (dd->ReqList, (Node *) rcb);
	    Signal (dd->ReqCount);
	    Wait (&req.WaitLock);
	}
	Signal (dd->Lock);
    }
    StopScsiHandler (dcb);
    Free (dcb->ScsiInfo);
    Free (dcb);
    IOdebug ("(%s) : Helios MSC Device Driver closed.", stime (buf));
}

/*--- end of msc.c ---*/
