/*
 * $Header: /Chris/00/helios/msc/RCS/scsi.c,v 2.0 91/08/21 18:05:48 chris Exp
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
** scsi.c								**
**									**
**	- Xilinx and Controller access for MSC Device Driver		**
**	- SCSI bus device handler					**
**									**
**************************************************************************
** HISTORY   :								**
** -----------								**
** Author    :	xx/xx/90 : C. Fleischer					**
*************************************************************************/

#ifndef	Driver
#include "msc.h"
#endif

#define	EventAck()	{ word dummy; in_ (4, event, &dummy ); }

#define	bmove_		move_

static int      NextInt (SDB * sdb, word Timeout, uword status, char *fname);
static int      NextIntD (SDB * sdb, word Timeout, uword status, char *fname);
static int      NextWriteBuf (SDB * sdb);
static int      NextReadBuf (SDB * sdb);
static int      NextData (SDB * sdb);
static int      WriteData (SDB * sdb, RCB * rcb, DP * dp);
static int      ReadData (SDB * sdb, RCB * rcb, DP * dp);
static int      WriteCommand (SDB * sdb, uword size, ubyte * data);
static int      ReadStatus (SDB * sdb, RCB * rcb);
static int      WriteMessage (SDB * sdb, uword size, byte * data);
static int      ReadMessage (SDB * sdb, uword size, byte * data, char *cname);
static bool     WDInit (SDB * sdb);
static void     Tidyup (SDB * sdb);

#if	DEBUG & DEBUG_HR
static void     HandleRequest (SDB * sdb, word Index, int level);

#else
static void     HandleRequest (SDB * sdb, word Index);

#endif

#if	DEBUG
static void
ClearDebug (Debug * debug)
{
    unless (debug->out) return;
/*FIXME*/    
    debug->out = TRUE;
    debug->line = 0;
    memset (debug->buf, 0, DebugSize);
}

static void
PutDebug (Debug * debug)
{
    char            buf[32];
    word            i;

    if (debug->out)
	return;
    i = debug->line;
    do {
	if (debug->buf[i << DebugLShift])
	    IOputs (&debug->buf[i << DebugLShift]);
	i = ((i + 1) & (DebugLines - 1));
    }
    while (i != debug->line);
    debug->out = TRUE;
    IOdebug ("(%s) :", stime (buf));
}

static void
MyDebug (Debug * debug, char *fmt,...)
{
    char           *digits = "0123456789abcdef";
    char           *null = "<NULL>";
    char            buf[DebugLength];
    char           *bp = buf;
    va_list         args;


    if (debug->out)			/* mark line as current or past	 */
	*bp++ = '+';
    else
	*bp++ = '-';
    va_start (args, fmt);
    while (*fmt && ((bp - buf) < (DebugLength - 16))) {
	if (*fmt == '%') {
	    fmt++;
	    switch (*fmt) {
	    case 'd':
		{
		    int             v = va_arg (args, int);
		    int             n = v < 0;
		    int             p = 0;
		    char            b[12];

		    if (n)
			v = -v;
		    do {
			b[p++] = digits[v % 10];
			v /= 10;
		    }
		    while (v);
		    if (n)
			*bp++ = '-';
		    while (p--)
			*bp++ = b[p];
		}
		break;
	    case 'x':
		{
		    uword           x = va_arg (args, uword);
		    int             p = 0;
		    char            b[8];

		    do {
			b[p++] = digits[x & 0xF];
			x >>= 4;
		    }
		    while (x);
		    while (p--)
			*bp++ = b[p];
		}
		break;
	    case 'c':
		{
		    int             i = va_arg (args, int);

		    *bp++ = (char) i;
		}
		break;
	    case 's':
		{
		    char           *s = va_arg (args, char *);

		    if (s == NULL)
			s = null;
		    while (*s)
			*bp++ = *s++;
		}
		break;
	    case '\0':
		goto no_newline;
	    default:
		*bp++ = *fmt;
	    }
	    fmt++;
	} else
	    *bp++ = *fmt++;
    }
    *bp++ = '\n';
no_newline:
    *bp++ = '\0';
    *bp++ = '\0';
    *bp++ = '\0';
    *bp++ = '\0';
    *bp++ = '\0';
    *bp++ = '\0';
    *bp++ = '\0';
    *bp++ = '\0';
    buf[DebugLength - 1] = '\0';

    if (debug->out) {
	IOputs (buf);
    } else {
	word            i;
	word            idx = debug->line;
	word           *dst = (word *) & debug->buf[idx << DebugLShift];
	word           *src = (word *) buf;

	for (i = 0; i < DebugLength >> 2; i++)
	    dst[i] = 0;
	while (*src)
	    *dst++ = *src++;
	debug->line = (idx + 1) & (DebugLines - 1);
    }
}

#else

#define PutDebug(x)
#define ClearDebug(x)
#endif


/************************************************************************
 * SOME USEFUL MACROS FOR SETTING XILINX FLIPFLOPS
 *
 * Parameter :	sdb	= SCSI data block, pointer to static values
 *		flag	= Flag Name without XFn_ prefix
 ***********************************************************************/


#define X_Set(sdb,ff)	*R##ff = (sdb)->r##ff |= (uword)(S##ff)
#define X_Clr(sdb,ff)	*R##ff = (sdb)->r##ff &= (uword)(C##ff)

/************************************************************************
 * SWITCH THE XILINX TO SCSI OPERATION
 ***********************************************************************/

#define X_SelectScsi(sdb)	X_Clr(sdb,XF_DBA);X_Clr(sdb,XF_Read);X_Set(sdb,XF_Scsi)

/************************************************************************
 * SOME USEFUL MACROS FOR CONTROLLER ACCESS
 *
 * - All these macros may only be used if the DBA mode is not active.
 *
 ***********************************************************************/

#define S_ReadReg(reg)		\
	(*WD_Address = (uword)(reg), (*WD_Register & 0xFF))

#define S_ReadAuxStatus()	\
	(*WD_AuxStatus & 0xFF)

#define S_ReadScsiStatus()	\
	(/*!sdb->ScsiTime = sum_(20, ldtimer_()),!*/ *WD_Address = WR_ScsiStatus, (*WD_Register & 0xFF))

#define S_ReadData()		\
	(*WD_Address = WR_Data, (*WD_Register & 0xFF))

#define S_WriteReg(reg,val)	\
	*WD_Address = (uword)(reg), *WD_Register = (uword)(val)

#define S_WriteData(val)	\
	*WD_Address = WR_Data, *WD_Register = (uword)(val)

#define S_WriteCmd(val)		\
	/*!tin_(sdb->ScsiTime),!*/ *WD_Address = WR_Command, *WD_Register = (uword)(val)

#define	S_WriteTCount(size)	\
    *WD_Address = WR_TCountH,	*WD_Register = (size) >> 16,\
    *WD_Register = (size) >> 8,	*WD_Register = (size)

#define S_ReadTCount()		\
    (*WD_Address = WR_TCountH,	\
    ((((( *WD_Register & 0xFF ) << 8 ) | ( *WD_Register & 0xFF )) << 8 ) | ( *WD_Register & 0xFF )))

#define S_ClearTCount()		\
    *WD_Address = WR_TCountH,	*WD_Register = 0,\
    *WD_Register = 0,		*WD_Register = 0


/************************************************************************
 * WAIT FOR THE NEXT INTERRUPT FROM SCSI CONTROLLER
 *
 * - Wait for an Event until the Interrupt bit of the Xilinx is set.
 *
 * Parameter :	sdb	= SCSI data block, pointer to static values
 *		Timeout	= timelimit
 *		status	= expected ScsiStatus value
 *		fname	= Name of calling function
 * Result :	0	timeout occured
 *		1	expected SCSI Interrupt
 *		2	unexpected SCSI Interrupt
 *
 ***********************************************************************/

static int
NextInt (SDB * sdb, word Timeout, uword status, char *fname)
{
    Channel        *event = sdb->Event;
    word            result;

    forever
    {					/* wait for next int		 */
	if (!IntWait (sdb->Event, Timeout)) {

#if	DEBUG
	    PutDebug (&sdb->debug);
	    MyDebug (&sdb->debug, "NextInt timeout in %s (0x%x,%d)", 
	    	fname, status, Timeout);
#endif

	    return 0;			/* return 0 on timeout		 */
	}
	result = *XI_Status;		/* read reason for Event	 */
	if ((result & XB_Interrupt))	/* Interrupt or enabled buf?	 */
	    break;

#if	DEBUG & DEBUG_INT
	PutDebug (&sdb->debug);
	MyDebug (&sdb->debug, "NextInt unused Event %x in %s", result, fname);
#endif

	X_Clr (sdb, XF_DBA);		/* reset WORD bit		 */
	EventAck ();			/* acknowledge unused Event	 */
    }

    sdb->ScsiStatus = S_ReadScsiStatus ();	/* get SCSI status	 */

    EventAck ();			/* acknowledge Event		 */


    if (status == 0xFF ||		/* check SCSI status		 */
	status == sdb->ScsiStatus) {

#if	DEBUG & DEBUG_INT
	MyDebug (&sdb->debug, "Int %x in %s", sdb->ScsiStatus, fname);
#endif

	return 1;
    } else {

#if	DEBUG & DEBUG_INT
	MyDebug (&sdb->debug, "Int %x, expected %x in %s", sdb->ScsiStatus, status, fname);
#endif

	return 2;
    }
    fname = fname;
}


static int
NextIntD (SDB * sdb, word Timeout, uword status, char *fname)
{
    Channel        *event = sdb->Event;
    word            result;

    forever
    {					/* wait for next int		 */
	if (!IntWait (sdb->Event, Timeout)) {
	    PutDebug (&sdb->debug);
	    IOdebug ("NextIntD timeout in %s (%d)", fname, Timeout);
	    return 0;			/* return 0 on timeout		 */
	}
	result = *XI_Status;		/* read reason for Event	 */
	if ((result & XB_Interrupt))	/* Interrupt or enabled buf?	 */
	    break;

	PutDebug (&sdb->debug);
	IOdebug ("Unused Event %x in %s", result, fname);

	X_Clr (sdb, XF_DBA);		/* reset WORD bit		 */
	EventAck ();			/* acknowledge unused Event	 */
    }

    sdb->ScsiStatus = S_ReadScsiStatus ();	/* get SCSI status	 */
    EventAck ();			/* acknowledge Event		 */


    if (status == 0xFF ||		/* check SCSI status		 */
	status == sdb->ScsiStatus) {

#if	DEBUG
	MyDebug (&sdb->debug, "Int %x, expected %x in %s", sdb->ScsiStatus, status, fname);
#else
	IOdebug ("Int %x in %s", sdb->ScsiStatus, fname);
#endif

	return 1;
    } else {

#if	DEBUG
	MyDebug (&sdb->debug, "Int %x, expected %x in %s", sdb->ScsiStatus, status, fname);
#else
	IOdebug ("Int %x, expected %x in %s", sdb->ScsiStatus, status, fname);
#endif

	return 2;
    }
    fname = fname;
}

static int
NextIntND (SDB * sdb, word Timeout, uword status, char *fname)
{
    Channel        *event = sdb->Event;
    word            result;

    forever
    {					/* wait for next int		 */
	if (!IntWait (sdb->Event, Timeout)) {
	    PutDebug (&sdb->debug);
	    IOdebug ("NextInt timeout in %s (%d)", fname, Timeout);
	    return 0;			/* return 0 on timeout		 */
	}
	result = *XI_Status;		/* read reason for Event	 */
	if ((result & XB_Interrupt))	/* Interrupt or enabled buf?	 */
	    break;

	X_Clr (sdb, XF_DBA);		/* reset WORD bit		 */
	EventAck ();			/* acknowledge unused Event	 */
    }

    sdb->ScsiStatus = S_ReadScsiStatus ();	/* get SCSI status	 */
    EventAck ();			/* acknowledge Event		 */

    if (status == 0xFF ||		/* check SCSI status		 */
	status == sdb->ScsiStatus) {

#if	DEBUG
	MyDebug (&sdb->debug, "Int %x, expected %x in %s", sdb->ScsiStatus, status, fname);
#endif

	return 1;
    } else {

#if	DEBUG
	MyDebug (&sdb->debug, "Int %x, expected %x in %s", sdb->ScsiStatus, status, fname);
#endif

	return 2;
    }
    fname = fname;
}


/************************************************************************
 * WAIT FOR THE NEXT INTERRUPT FROM SCSI CONTROLLER OR FROM BIGLATCH
 *
 * - Wait for an Event until the Interrupt bit or the Word bit
 *   of the Xilinx is set.
 * - During Data transfers, it is not possible to read
 *   the ScsiStatus register while the Xilinx is set to DBA mode.
 *
 * Parameter :	sdb	= SCSI data block, pointer to static values
 * Result :	0	timeout occured
 *		1	SCSI Interrupt, not acknowledged !
 *		2	Biglatch Interrupt, not acknowledged !
 *
 ***********************************************************************/

static int
NextWriteBuf (SDB * sdb)
{
    Channel        *event = sdb->Event;
    word            result;

    if ((result = (*XI_Status & (XB_Interrupt | XB_Word))) != 0)
	goto found;

    forever
    {					/* wait for next int		 */
	if (!IntWait (sdb->Event, DataTimeout)) {

#if	DEBUG & DEBUG_INT
	    MyDebug (&sdb->debug, "NextWriteBuf timed out %d", DataTimeout);
#endif

	    return 0;			/* return 0 on timeout		 */
	}
	if ((result = (*XI_Status & (XB_Interrupt | XB_Word))) != 0)
	     break;			/* read reason for Event	 */
# if	DEBUG & DEBUG_INT
	MyDebug (&sdb->debug, "NextWriteBuf unused Event %x", result);
#endif

	EventAck ();			/* acknowledge unused Event	 */
    }

found:
    if (result & XB_Interrupt)		/* get return value		 */
	return 1;
    return 2;
}


/************************************************************************
 * WAIT FOR THE NEXT INTERRUPT FROM SCSI CONTROLLER OR FROM BIGLATCH
 *
 * - Wait for an Event until the Interrupt bit or the Word bit
 *   of the Xilinx is set.
 * - During Data transfers, it is not possible to read
 *   the ScsiStatus register while the Xilinx is set to DBA mode.
 *
 * Parameter :	sdb	= SCSI data block, pointer to static values
 * Result :	0	timeout occured
 *		1	SCSI Interrupt, not acknowledged !
 *		2	Biglatch Interrupt, not acknowledged !
 *
 ***********************************************************************/

static int
NextReadBuf (SDB * sdb)
{
    Channel        *event = sdb->Event;
    word            result;

    if ((result = (*XI_Status & (XB_Interrupt | XB_Word))) != 0)
	goto found;

    forever
    {					/* wait for next int		 */
	if (!IntWait (sdb->Event, DataTimeout)) {

#if	DEBUG & DEBUG_INT
	    MyDebug (&sdb->debug, "NextReadBuf timed out %d", DataTimeout);
#endif

	    return 0;			/* return 0 on timeout		 */
	}
	if ((result = (*XI_Status & (XB_Interrupt | XB_Word))) != 0)
	    break;			/* read reason for Event	 */

#if	DEBUG & DEBUG_INT
	MyDebug (&sdb->debug, "NextReadBuf unused Event %x", result);
#endif

	EventAck ();			/* acknowledge unused Event	 */
    }

found:
    if (result & XB_Word)		/* get return value		 */
	return 2;
    return 1;
}


/************************************************************************
 * WAIT FOR THE DATA BUFFER READY BIT TO BE SET
 *
 * - Poll the DBR bit and the INT bit of the Aux Status register
 *
 * Parameter :	sdb	= SCSI data block, pointer to static values
 * Result :	0	timeout occured
 *		1	INT bit set
 *		2	DBR bit set
 *
 ***********************************************************************/

static int
NextData (SDB * sdb)
{
    word            stat, now, end;

    now = ldtimer_ ();
    end = sum_ (now, DataTimeout);	/* set end time		 	 */

    while (((stat = S_ReadAuxStatus ())	/* poll bits		 	 */
	    &(WB_Interrupt | WB_DataReady)) == 0) {
	now = ldtimer_ ();
	if (diff_ (now, end) <= 0)
	    break;
    }
    sdb->AuxStatus = stat;
    if (diff_ (now, end) <= 0) {	/* test timeout		 	 */

#if	DEBUG & DEBUG_INT
	PutDebug (&sdb->debug);
	MyDebug (&sdb->debug, "NextData timeout %d", DataTimeout);
#endif

	return 0;
    }
    if (stat & WB_Interrupt)		/* test interrupt	 	 */
	return 1;

    return 2;
}

/* Transfer functions for the different SCSI bus phases			 */

/************************************************************************
 * TRANSFER DATA TO THE SCSI CONTROLLER
 *
 * - The SCSI controller has been initialised for the transfer,
 *   all Registers have been set up according to the transfer state and
 *   a transfer command has been issued.
 * - Wait for the Controller's data requests and copy data to the BigLatch.
 * - The next incoming interrupt is read and saved in the sdb.
 *
 * Parameter :	sdb	= SCSI data block, pointer to static values
 *		rcb	= Request control block with Register values
 *		dp	= Data parameter pointer
 * Return    :	-1	Blockmove timeout
 *		0	Interrupt timeout
 *		1	everything ok
 *
 ***********************************************************************/

static int
WriteData (SDB * sdb, RCB * rcb, DP * dp)
{
    char           *fname = "WriteData";
    Channel        *event = sdb->Event;
    uword          *BLatch = BigLatch;
    byte           *data = dp->Data;	/* Data pointer			 */
    word            rest = dp->Rest;	/* bytes to transfer		 */
    byte           *end;		/* end value of Data pointer	 */
    byte           *bend;		/* burst end ptr		 */
    byte           *tend;		/* transfer end ptr		 */
    word            bsize;		/* burst transfer size		 */
    word            msize;		/* blockmove transfer size	 */
    word            tsize;		/* intermediate transfer size	 */
    word            result = 2;

    X_Set (sdb, XF_DBA);		/* allow Direct Buffer Access.	 */

/* If the rest is not a multiple of 4, round it up and write		 */
/* some garbage into the BigLatch.					 */

    end = data + ((rest + 3) & ~3);

    if (rcb->Block) {

    /*
     * Blockmove Write : The WD 3393 can transfer bursts of up to 4096 byte
     * without delay, but needs a 80 us break between bursts. During this
     * break, the Controller does not read anything from the BigLatch. At the
     * beginning of a burst, the transputer has to fill the BigLatch and the
     * Controller FIFO (4 * 4 bytes) and then wait for the transfer to begin.
     * Then the transputer has to send some sectors and a short sector for
     * the end of the burst. If the total amount of bytes to transfer is not
     * a multiple of the burst size, a shortened burst has to be sent FIRST
     * !!!
     */

	while (data < end) {
	    if ((bsize = (end - data) & 4095) == 0)
		bend = data + 4096;
	    else
		bend = data + bsize;	/* set up burst end	 	 */

	/* Prefill the BigLatch and the Controller FIFO			 */

	    if ((tend = data + 16) > bend)
		tend = bend;

	    while (data < tend) {	/* poll for data ready		 */
		if (((*XI_Status & (XB_Interrupt | XB_Word)) == XB_Word) ||
		    ((*XI_Status & (XB_Interrupt | XB_Word)) == XB_Word) ||
		    (result = NextWriteBuf (sdb)) == 2) {
		    *BLatch = *(word *) data;	/* transfer 4 bytes	 */
		    EventAck ();
		    data += 4;		/* adjust data pointer and rest	 */
		} else
		    goto done;

#if	0
					
	     if ((msize = tend - data) > 0) {	/* set blockmove size	*/
	     			 	/* wait for data ready		*/
	         if (((*XI_Status & (XB_Interrupt | XB_Word)) == XB_Word) 
	           || ((*XI_Status & (XB_Interrupt | XB_Word)) == XB_Word) 
	           || (result = NextWriteBuf (sdb)) == 2) {
	           	bmove_ (msize, BLatch, data); 
	           	EventAck (); 
	           	if (*XI_Status & XB_Timeout) { 
#if	DEBUG
	     		    PutDebug (&sdb->debug); 
	     		    MyDebug (&sdb->debug, "bm write to 1"); 
#endif 
			    result = -1; 
			    goto done; 
			} 
			data += msize;	/* adjust data pointer		*/ 
		    } else 
		    	goto done; 
		} 
#endif		
	    }

	/* send sectors, last shorter, until bsize is exhausted			 */

	    while (data < bend) {
		if ((tend = data + rcb->BlkSize) > bend)
		    tend = bend;	/* set transfer size		 */

	    /* set blockmove size and wait for data ready		 */
		if ((msize = tend - data - 12) > 0) {
		    if (((*XI_Status & (XB_Interrupt | XB_Word)) == XB_Word) ||
		     ((*XI_Status & (XB_Interrupt | XB_Word)) == XB_Word) ||
			(result = NextWriteBuf (sdb)) == 2) {
			bmove_ (msize, BLatch, data);	/* latch free, move */
			EventAck ();
			if (*XI_Status & XB_Timeout) {

#if	DEBUG
			    PutDebug (&sdb->debug);
			    MyDebug (&sdb->debug, "bm write to 2");
#endif

			    result = -1;
			    goto done;
			}
			data += msize;	/* adjust data pointer and rest	 */
		    } else
			goto done;
		}

	    /*
	     * poll the last three words separately, because some devices
	     * send them very late (causing Xilinx timeouts)
	     */

		while (data < tend) {	/* wait for data ready		 */
		    if (((*XI_Status & (XB_Interrupt | XB_Word)) == XB_Word) ||
		     ((*XI_Status & (XB_Interrupt | XB_Word)) == XB_Word) ||
			(result = NextWriteBuf (sdb)) == 2) {
			*BLatch = *(word *) data;	/* move a word	 */
			EventAck ();
			data += 4;	/* adjust data pointer		 */
			continue;
		    } else
			goto done;
		}			/* sector transfer completed	 */
	    }				/* burst transfer completed	 */
	}				/* write transfer completed	 */
    } else {

    /* Polling Write : write single words into the Biglatch.		 */

	while (data < end) {		/* wait for data ready		 */
	    if (((*XI_Status & (XB_Interrupt | XB_Word)) == XB_Word) ||
		((*XI_Status & (XB_Interrupt | XB_Word)) == XB_Word) ||
		(result = NextWriteBuf (sdb)) == 2) {
		*BLatch = *(word *) data;	/* move the word	 */
		EventAck ();
		data += 4;		/* adjust data pointer		 */
		continue;
	    } else
		goto done;
	}
    }
    while ((result = NextWriteBuf (sdb)) == 2)
	*BLatch = 0;

done:					/* an unexpected condition	 */
    if (result < 0) {
	while ((result = NextWriteBuf (sdb)) == 2)
	    *BLatch = 0;
	result = -1;
    }
    X_Clr (sdb, XF_DBA);		/* allow controller access	 */
    if (result == 1) {			/* non-acknowledged interrupt	 */
	sdb->ScsiStatus = S_ReadScsiStatus ();
	EventAck ();

#if	DEBUG & DEBUG_INT
	MyDebug (&sdb->debug, "Int %x in %s", sdb->ScsiStatus, fname);
#endif
    }
    elif (result == 0) {		/* NextWriteBuf timed out	 */
	PutDebug (&sdb->debug);
	IOdebug ("%s NextWriteBuf timed out", fname);
    }
    else				/* blockmove timed out		 */
    {
	PutDebug (&sdb->debug);
	IOdebug ("%s blockmove timed out", fname);
    }
    rest = end - data;

    tsize = S_ReadTCount ();		/* read controller rest		 */

    dp->Data += dp->Rest - tsize;	/* calculate new Data pointer	 */
    dp->Rest = tsize;			/* from Controller's rest	 */

#if	DEBUG
    if (result < 0 && tsize != rest)
	MyDebug (&sdb->debug, "WriteData : contr. rest %d, my rest %d", tsize, rest);
#endif

    return result;
}


/************************************************************************
 * TRANSFER DATA FROM THE SCSI CONTROLLER
 *
 * - The SCSI controller has been initialised for the transfer,
 *   all Registers have been set up according to the transfer state and
 *   a transfer command has been issued.
 * - Wait for the Controller's data requests and copy data to the BigLatch.
 * - The next incoming interrupt is read and saved in the sdb.
 *
 * Parameter :	sdb	= SCSI data block, pointer to static values
 *		rcb	= Request control block with Register values
 *		dp	= Data parameter pointer
 * Return    :	-1	Blockmove timeout
 *		0	Interrupt timeout
 *		1	everything ok
 *
 ***********************************************************************/

static int
ReadData (SDB * sdb, RCB * rcb, DP * dp)
{
    char           *fname = "ReadData";
    Channel        *event = sdb->Event;
    uword          *BLatch = BigLatch;
    byte           *data = dp->Data;	/* Data pointer			 */
    word            rest = dp->Rest;	/* bytes to transfer		 */
    byte           *end;
    byte           *tend;
    word            msize;		/* block move transfer size	 */
    word            tsize;		/* intermediate transfer size	 */
    word            result;

    X_Set (sdb, XF_DBA);		/* allow direct buffer access	 */

    end = data + (rest & ~3);		/* ignore last partword for now	 */

    if (rcb->Block) {

    /*
     * Blockmove Read : the last word of a sector may be somewhat delayed, so
     * that a Blockmove might time out. Therefore, get the last word
     * separately from the BigLatch if available.
     */

	while (data < end) {

	    if ((tend = data + rcb->BlkSize) > end)
		tend = end;		/* set transfer size		 */
	/* set blockmove size and wait for data ready		 	 */
	    if ((msize = tend - data - 4) > 0) {
		if (((*XI_Status & (XB_Interrupt | XB_Word)) == XB_Word) ||
		    ((*XI_Status & (XB_Interrupt | XB_Word)) == XB_Word) ||
		    (result = NextReadBuf (sdb)) == 2) {
		    bmove_ (msize, data, BLatch);	/* move block	 */
		    EventAck ();
		/* check timeout bit		 */
		    if (*XI_Status & XB_Timeout) {

#if	DEBUG
			PutDebug (&sdb->debug);
			MyDebug (&sdb->debug, "bm read to 1");
#endif

			result = -1;
			goto done;
		    }
		    data += msize;	/* adjust data pointer		 */
		} else
		    goto done;
	    }
	    while (data < tend) {	/* poll the last words		 */
	    /* wait for data ready		 */
		if (((*XI_Status & (XB_Interrupt | XB_Word)) == XB_Word) ||
		    ((*XI_Status & (XB_Interrupt | XB_Word)) == XB_Word) ||
		    (result = NextReadBuf (sdb)) == 2) {
		    *(word *) data = *BLatch;	/* move the word		 */
		    EventAck ();
		    data += 4;		/* adjust data pointer 		 */
		    continue;
		} else
		    goto done;
	    }
	}
    } else {

    /*
     * Polling Read : read single words from the BigLatch. If the rest is not
     * a multiple of 4, stop before the last partword and get it after the
     * next interrupt has occurred.
     */

	while (data < end) {		/* wait for data ready		 */
	    if (((*XI_Status & (XB_Interrupt | XB_Word)) == XB_Word) ||
		((*XI_Status & (XB_Interrupt | XB_Word)) == XB_Word) ||
		(result = NextReadBuf (sdb)) == 2) {
		*(word *) data = *BLatch;	/* move the word		 */
		EventAck ();
		data += 4;		/* adjust data pointer and rest	 */
	    } else
		goto done;
	}
    }

    result = NextReadBuf (sdb);

#if	DEBUG
    if (result == 2)
	MyDebug (&sdb->debug, "%s: unexpected data arrived after transfer completion", fname);
#endif

done:
    if (result < 0) {
	while ((result = NextReadBuf (sdb)) == 2)
	    result = *BLatch;
	result = -1;
    }
    if (result == 1			/* check pending partword data	 */
	&& (msize = end - data) > 0 && msize < 5) {
	bmove_ (msize, data, BLatch);	/* get partword rest	 	 */

#if	DEBUG
	if (*XI_Status & XB_Timeout)
	    MyDebug (&sdb->debug, "Read to 3");
#endif

	data += msize;
    }
    X_Clr (sdb, XF_DBA);		/* allow controller access	 */

    rest = end - data;

    if (result == 1) {			/* non-acknowledged interrupt	 */
	sdb->ScsiStatus = S_ReadScsiStatus ();
	EventAck ();

#if	DEBUG & DEBUG_INT
	MyDebug (&sdb->debug, "Int %x in %s", sdb->ScsiStatus, fname);
#endif
    }
    elif (result == 2) {		/* transfer ok, disable Word	 */
	result = NextReadBuf (sdb);
	goto done;
    }
    elif (result == 0) {		/* NextReadBuf timed out	 */
	PutDebug (&sdb->debug);
	IOdebug ("%s NextReadBuf timed out", fname);
    }
    else				/* blockmove timed out		 */
    {
	PutDebug (&sdb->debug);
	IOdebug ("%s blockmove timed out", fname);
    }

    tsize = S_ReadTCount ();		/* read controller rest		 */

    dp->Data += dp->Rest - tsize;	/* calculate new Data pointer	 */
    dp->Rest = tsize;			/* from Controller's rest	 */

#if	DEBUG
    if (tsize != rest)
	MyDebug (&sdb->debug, "ReadData : contr. rest %d, my rest %d", tsize, rest);
#endif

    return result;
}


/************************************************************************
 * SEND A COMMAND OUT
 *
 * - Set up the TransferCount registers, issue a Transfer Command
 *   and write the Command bytes to the Data register
 *   each time the DBR bit gets set.
 *
 * Parameter :	sdb	= SCSI data block, pointer to static values
 *		size	= Command size
 *		msg	= Command data
 * Result :	0	timeout occured
 *		> 0	number of bytes sent
 *
 ***********************************************************************/

static int
WriteCommand (SDB * sdb, uword size, ubyte * data)
{
    int             res;
    int             i;

#if	DEBUG & DEBUG_CMD
    MyDebug (&sdb->debug, "WriteCommand");
#endif

    S_WriteTCount (size);		/* set Transfer count		 */

    S_WriteCmd (WC_Transfer);

    for (i = 0; i < size; i++) {	/* wait for Data Buffer Ready	 */
	if ((res = NextData (sdb)) == 2) {

#if	DEBUG & DEBUG_CMD
	    MyDebug (&sdb->debug, "CDB %d : 0x%x", i, data[i]);
#endif

	    S_WriteData (data[i]);	/* send Command byte out	 */
	}
	elif (res == 1)
	  break;			/* interrupt pending.....	 */
	else
	return 0;			/* timed out.....		 */
    }
    if ((res = NextInt (sdb, CommandTimeout, 0xff, "WriteCommand")) != 0)
	return i;
    return 0;
}


/************************************************************************
 * RECEIVE A STATUS BYTE
 *
 * - Set up the TransferCount to 1, issue a Transfer Command
 *   and read the Status byte from the Data register if DBR gets set.
 *
 * Parameter :	sdb	= SCSI data block, pointer to static values
 * Result :	0	timeout occured
 *		1	status byte received
 *
 ***********************************************************************/

static int
ReadStatus (SDB * sdb, RCB * rcb)
{
    int             res;

/* Delay ( 10 );							 */
    S_WriteCmd (WC_TransferSingle);


    if ((res = NextData (sdb)) == 2)	/* wait for data buffer ready	 */
	rcb->Status = S_ReadData ();
    elif (res == 1)			/* pending interrupt		 */
      return 1;
    else
    return 0;				/* timeout occurred		 */

    if ((res = NextInt (sdb, CommandTimeout, 0xff, "WriteCommand")) != 0)
	return 1;
    return 0;
}


/************************************************************************
 * SEND A MESSAGE OUT
 *
 * - This command requires the ATN line to be asserted.
 * - Set up the TransferCount registers, issue a Transfer Command
 *   and write the Message bytes to the Data register
 *   each time the DBR bit gets set.
 *
 * Parameter :	sdb	= SCSI data block, pointer to static values
 *		size	= Message size
 *		msg	= Message data
 * Result :	0	timeout occured
 *		>0	number of bytes sent
 *
 ***********************************************************************/

static int
WriteMessage (SDB * sdb, uword size, byte * data)
{
    int             res;
    int             i;

    S_WriteTCount (size);		/* set Transfer count		 */

    S_WriteCmd (WC_Transfer);

    for (i = 0; i < size; i++) {	/* wait for Data Buffer Ready	 */
	if ((res = NextData (sdb)) == 2) {

#if	DEBUG & DEBUG_MSGOUT
	    MyDebug (&sdb->debug, "Write Msg %x", data[i]);
#endif

	    S_WriteData (data[i]);	/* send Message byte out	 */
	}
	elif (res == 1)
	  break;			/* interrupt pending.....	 */
	else
	return 0;			/* timed out.....		 */
    }
    if ((res = NextInt (sdb, CommandTimeout, 0xff, "WriteMessage")) != 0)
	return i;
    return 0;
}


/************************************************************************
 * RECEIVE A MESSAGE
 *
 * - Set up the TransferCount registers, issue a Transfer Command
 *   and read the Message bytes from the Data register if DBR gets set.
 *
 * Parameter :	sdb	= SCSI data block, pointer to static values
 *		size	= expected Message size
 *		msg	= Message data
 * Result :	< 0	- (number of bytes received), last acknowledged
 *		0	timeout occured
 *		> 0	number of bytes received, last not acknowledged
 *
 ***********************************************************************/

static int
ReadMessage (SDB * sdb, uword size, byte * data, char *cname)
{
    char           *fname = "ReadMessage";

    int             t = 5;
    int             c = 0;		/* transferred data count	 */
    int             d;			/* data byte			 */
    int             e = 1;		/* Msg end count		 */
    int             r;			/* result from NextInt		 */

#if	DEBUG & DEBUG_MSGIN
    MyDebug (&sdb->debug, "ReadMsg from %s Aux %x Scsi %x Cmd %x",
	cname, S_ReadAuxStatus (), sdb->ScsiStatus, S_ReadReg (WR_Command));
#endif

    while (S_ReadAuxStatus () && t--) {
	S_WriteCmd (WC_Abort);
	Delay (99);

#if	DEBUG & DEBUG_MSGIN
	MyDebug (&sdb->debug, "ReadMsg now Aux %x Scsi %x Cmd %x",
	       S_ReadAuxStatus (), sdb->ScsiStatus, S_ReadReg (WR_Command));
#endif
    }
/* Delay ( 99 );							 */
    forever
    {

#if	DEBUG & DEBUG_MSGIN
	if (t < 5)
	    MyDebug (&sdb->debug, "ReadMsg ok %d Aux %x Scsi %x Cmd %x",
	    t, S_ReadAuxStatus (), sdb->ScsiStatus, S_ReadReg (WR_Command));
#endif

	t = 5;
	S_WriteCmd (WC_TransferSingle);

	if (NextData (sdb) != 2) {	/* wait for data buffer ready	 */
	    PutDebug (&sdb->debug);
	    IOdebug ("%s: NextData timed out (%x)", fname, S_ReadAuxStatus ());
	    return 0;
	}
	d = S_ReadData ();		/* read data byte		 */

	if (c < size)
	    data[c++] = d;		/* within limits, store byte	 */

suck:
	if ((r = NextInt (sdb, MessageTimeout, 0x20, fname)) != 1) {
	    if (r)			/* unexpected Interrupt		 */
		return (t == 5 ? c : -c);
	    PutDebug (&sdb->debug);
	    IOdebug ("%s: NextInt 0x20 timed out (%x,%x)",
		     fname, d, S_ReadAuxStatus ());
	    r = S_ReadData ();
	    S_WriteCmd (WC_NegateACK);
	    if (t == 3) {
		while (S_ReadAuxStatus () & 0x10);
		S_WriteCmd (WC_Abort);
		while (S_ReadAuxStatus () & 0x10);
		if (S_ReadAuxStatus () & 0x01)
		    r = S_ReadData ();
	    }
	    if (--t)
		goto suck;
	    return 0;			/* timeout			 */
	}
	if (c == 1) {			/* first Msg byte		 */
	    if (d != 1)			/* not an extended Msg		 */
		return 1;
	    else
		e = 2;			/* extend for Msg size		 */
	}
	elif (c == 2)			/* extended Msg size		 */
	  e += d;			/* add data to end value	 */

	if (c >= e)			/* complete Msg transferred	 */
	    return c;			/* return without Negate ACK	 */

	S_WriteCmd (WC_NegateACK);	/* Negate ACK			 */
	unless (NextInt (sdb, MessageTimeout, 0xff, fname)) {
	    PutDebug (&sdb->debug);
	    IOdebug ("%s: NextInt 0xff timed out (%x)", fname, S_ReadAuxStatus ());
	    return 0;
	}
	if (sdb->ScsiStatus != 0x8f)	/* not Msg in phase		 */
	    return -c;
    }

#if	! DEBUG
    cname = cname;
#endif
}

/***********************************************************************
 * INTITIALISE THE WD 3393 SCSI CONTROLLER TO ADVANCED MODE
 *
 * Parameter :	sdb	= SCSI data block, pointer to static values
 * Result :	TRUE	if initialisation succeeded
 *
 ***********************************************************************/

static          bool
WDInit (SDB * sdb)
{
    char           *fname = "WDInit";

#if	DEBUG & DEBUG_RESET
    MyDebug (&sdb->debug, "Scsi Controller reset");
#endif

    S_WriteReg (WR_OwnID, GetID (sdb->OwnId));
    S_WriteCmd (WC_Reset);		/* Write Reset command.		 */

    return NextInt (sdb, ResetTimeout, 0x00, fname) == 1;
}


/***********************************************************************
 * FORCE THE SCSI CONTROLLER AND A DEVICE INTO A KNOWN STATE
 * - Try to solve the connection between the Drive and the Controller
 *   after an error has occured.
 *
 * Parameter :	sdb	= SCSI data block, pointer to static values
 *
 ***********************************************************************/

static void
Tidyup (SDB * sdb)
{
    char           *fname = "Tidyup";
    word            a_stat;
    word            s_stat = sdb->ScsiStatus;
    word            dummy, res;

    PutDebug (&sdb->debug);
    IOdebug ("%s: AuxStat %x, ScsiStat %x, Phase %x, Command %x",
	     fname, S_ReadAuxStatus (), sdb->ScsiStatus,
	     S_ReadReg (WR_CmdPhase), S_ReadReg (WR_Command));

next_a_stat:
    a_stat = S_ReadAuxStatus ();

#if	DEBUG & DEBUG_TIDYUP
    MyDebug (&sdb->debug, "%s: Aux status %x", fname, a_stat);
#endif

    if (a_stat & 0x80)			/* interrupt pending		 */
	goto nextint;
    elif (a_stat & 0x40) {		/* last command ignored		 */
	while (!WDInit (sdb));		/* reset the SCSI controller	 */
	S_WriteCmd (WC_AssertATN);	/* set ATN and wait for reaction */
    }
    elif (a_stat & 0x10) {		/* command in progress		 */
	Delay (100);
	goto next_a_stat;
    }
    elif (a_stat & 0x20)		/* busy with level 2 command	 */
      S_WriteCmd (WC_Abort);
    else
    goto nextstate;

nextint:
    if (NextIntD (sdb, SelectTimeout, 0xff, fname) == 0) {

#if 0
	MyDebug (&sdb->debug, "ScsiHandler halted.");
	forever
	  Delay (OneSec);
#endif

	goto next_a_stat;
    }
    s_stat = sdb->ScsiStatus;
nextstate:
    IOdebug ("%s: Scsi status %x", fname, s_stat);

#if 0
    MyDebug (&sdb->debug, "ScsiHandler halted.");
    forever
      Delay (OneSec);
#endif

    switch (s_stat) {
    case 0x18:				/* Data out phase		 */
    case 0x28:
    case 0x48:
    case 0x88:

#if	DEBUG & DEBUG_TIDYUP
	MyDebug (&sdb->debug, "%s: Data out", fname);
#endif

	S_WriteCmd (WC_AssertATN);
	S_WriteReg (WR_Control, 0x04);
	S_WriteTCount (-1);
	S_WriteCmd (WC_Transfer);
	while ((res = NextData (sdb)) == 2)
	    S_WriteData (0);
	if (res)
	    goto nextint;
	goto timeout;

    case 0x19:				/* Data in phase		 */
    case 0x29:
    case 0x49:
    case 0x89:

#if	DEBUG & DEBUG_TIDYUP
	MyDebug (&sdb->debug, "%s: Data in", fname);
#endif

	S_WriteCmd (WC_AssertATN);
	S_WriteReg (WR_Control, 0x04);
	S_WriteTCount (-1);
	S_WriteCmd (WC_Transfer);
	while ((res = NextData (sdb)) == 2)
	    dummy = S_ReadData ();
	if (res)
	    goto nextint;
	goto timeout;

    case 0x1A:				/* Command phase		 */
    case 0x2A:
    case 0x4A:
    case 0x8A:

#if	DEBUG & DEBUG_TIDYUP
	MyDebug (&sdb->debug, "%s: Command", fname);
#endif

	S_WriteCmd (WC_AssertATN);
	Delay (10);
	S_WriteCmd (WC_TransferSingle);
	if ((res = NextData (sdb)) == 2)
	    S_WriteData (0xff);
	if (res)
	    goto nextint;
	goto timeout;

    case 0x1B:				/* Status phase			 */
    case 0x2B:
    case 0x4B:
    case 0x8B:

#if	DEBUG & DEBUG_TIDYUP
	MyDebug (&sdb->debug, "%s: Status", fname);
#endif

	S_WriteCmd (WC_AssertATN);
	Delay (100);
	S_WriteCmd (WC_TransferSingle);
	if ((res = NextData (sdb)) == 2)
	    dummy = S_ReadData ();
	if (res)
	    goto nextint;
	goto timeout;

    case 0x1E:				/* Message out phase		 */
    case 0x2E:
    case 0x4E:
    case 0x8E:

#if	DEBUG & DEBUG_TIDYUP
	MyDebug (&sdb->debug, "%s: Message out", fname);
#endif

	S_WriteCmd (WC_TransferSingle);
	if ((res = NextData (sdb)) == 2)
	    S_WriteData (0x06);		/* send ABORT message		 */
	if (res)
	    goto nextint;
	goto timeout;

    case 0x1F:				/* Message in phase		 */
    case 0x2F:
    case 0x4F:
    case 0x8F:

#if	DEBUG & DEBUG_TIDYUP
	MyDebug (&sdb->debug, "%s: Message in", fname);
#endif

	S_WriteCmd (WC_AssertATN);
	Delay (100);
	S_WriteCmd (WC_TransferSingle);
	if ((res = NextData (sdb)) == 2)
	    dummy = S_ReadData ();
	if (res)
	    goto nextint;
	goto timeout;

    case 0x20:				/* Msg in waits for ACK		 */

#if	DEBUG & DEBUG_TIDYUP
	MyDebug (&sdb->debug, "%s: Message in ACK", fname);
#endif

	S_WriteCmd (WC_NegateACK);
	goto nextint;

    case 0x22:				/* Command aborted		 */

#if	DEBUG & DEBUG_TIDYUP
	MyDebug (&sdb->debug, "%s: Command aborted", fname);
#endif

	S_WriteCmd (WC_NegateACK);
	goto nextint;

    case 0x41:				/* unexpected disconnect	 */
    case 0x85:				/* Target has disconnected	 */
	IOdebug ("%s: bus free", fname);
	return;

    default:
timeout:

#if	DEBUG & DEBUG_TIDYUP
	MyDebug (&sdb->debug, "%s: unknown", fname);
#endif

	S_WriteCmd (WC_AssertATN);
	Delay (100);
	S_WriteCmd (WC_NegateACK);
	Delay (100);
	goto nextint;
    }
}


/***********************************************************************
 * TEST AND INTITIALISE THE WD 3393 SCSI CONTROLLER
 * - Test the functionality of the SCSI controller and initialise it
 *   to the advanced mode.
 *
 * Parameter :	sdb	= SCSI data block, pointer to static values
 * Result :	TRUE if test and initialisation succeeded
 *
 ***********************************************************************/

bool
ScsiInit (SDB * sdb)
{
    char           *fname = "ScsiInit";
    word            i, j;
    word            result;

#if	DEBUG & DEBUG_INIT
    MyDebug (&sdb->debug, "Hardware Reset");
#endif

    X_SelectScsi (sdb);			/* switch to SCSI controller	 */

    X_Set (sdb, XF_Pres);		/* assert the reset line	 */
    Delay (ResetDelay);			/* let the reset come through	 */
    X_Clr (sdb, XF_Pres);		/* deassert the reset line	 */

/* wait for Reset interrupt	 */
    if (NextIntND (sdb, ResetTimeout, 0x00, fname) != 1)
	return FALSE;

    result = 0;				/* no errors yet		 */

/*
 * Register read/write test : All registers should be read/write.
 */

#if	DEBUG & DEBUG_INIT
    MyDebug (&sdb->debug, "Register Read/Write Test");
#endif

    for (i = 0; i < 300; i++) {
	*WD_Address = 0;		/* reset Register Address	 */

	for (j = 0; j < 20; j++)	/* write something in		 */
	    *WD_Register = (i + j * 7);

	*WD_Address = 0;		/* reset Register Address	 */

	for (j = 0; j < 20; j++)	/* check for written values	 */
	    if ((*WD_Register & 0xFF) != ((i + j * 7) & 0xFF))
		result++;		/* count errors			 */
    }

    if (result)				/* error in read/write test	 */
	return FALSE;

/*
 * Reset clear test : All registers should be cleared after WC_Reset
 */

#if	DEBUG & DEBUG_INIT
    MyDebug (&sdb->debug, "Reset Clear Test");
#endif

    *WD_Address = 1;
    for (j = 1; j < 21; j++)
	*WD_Register = j;

    S_WriteReg (0, 0);			/* Register 0 is special...	 */

    S_WriteCmd (WC_Reset);		/* Write Reset command.		 */

/* wait for Interrupt...	 */
    if (NextInt (sdb, ResetTimeout, 0x00, fname) != 1)
	return FALSE;

    *WD_Address = 1;			/* now all Registers		 */
    for (j = 1; j < 21; j++)		/* should be cleared.		 */
	if ((*WD_Register & 0xFF) != 0)
	    result++;

    if (result)				/* error in reset clear test	 */
	return FALSE;

/*
 * Translate Address command test : the command should return the right
 * values
 */

#if	DEBUG & DEBUG_INIT
    MyDebug (&sdb->debug, "Translate Address Command Test");
#endif

    *WD_Address = 3;			/* set up the parameters	 */
    *WD_Register = 0x15;		/* Total sectors		 */
    *WD_Register = 0x06;		/* Total heads			 */
    *WD_Register = 0x13;		/* Total cylinders (msb)	 */
    *WD_Register = 0x45;		/* Total cylinders (lsb)	 */
    *WD_Register = 0x00;		/* Logical address (msb)	 */
    *WD_Register = 0x00;		/* :	      :			 */
    *WD_Register = 0x12;		/* :	      :			 */
    *WD_Register = 0x34;		/* Logical address (lsb)	 */
    *WD_Register = 0x00;		/* Sector	(result)	 */
    *WD_Register = 0x00;		/* Head		(result)	 */
    *WD_Register = 0x00;		/* Cylinder msb	(result)	 */
    *WD_Register = 0x00;		/* Cylinder lsb	(result)	 */

    S_WriteCmd (WC_TranslateAddress);	/* start command		 */

/*
 * wait for completion Interrupt...
 */
    if (NextInt (sdb, ResetTimeout, 0x15, fname) != 1)
	return FALSE;

    *WD_Address = 0x0B;
    i = 0;
    for (j = 0; j < 4; j++)
	i = (i << 8) + (*WD_Register & 0xFF);

    if (i != 0x13050024)		/* error in translate test	 */
	return FALSE;

/*
 * Invalid command test: the Receive data command is invalid if not conneted
 * as a Target.
 */

#if	DEBUG & DEBUG_INIT
    MyDebug (&sdb->debug, "Invalid Command Test");
#endif

    S_WriteCmd (WC_ReceiveData);	/* start command		 */

/* wait for Interrupt...						 */
    if (NextInt (sdb, ResetTimeout, 0x40, fname) != 1)
	return FALSE;

/* Initialise into advanced mode					 */

    unless (WDInit (sdb))
      return FALSE;

#if	DEBUG & DEBUG_INIT
    MyDebug (&sdb->debug, "Tests completed.");
#endif

    return TRUE;			/* all tests passed.... puh !	 */
}

/************************************************************************
 * HANDLE A SINGLE SCSI REQUEST 
 *
 * - Initialise the Controller with the RCB values, start the command
 * - If the Device has not been sent a SDT message, use the Select-w-ATN
 *   Command to establish the connection, then send an ID Message
 *   followed by the SDT Message and continue with Reselect-and-Transfer
 *   or single steps.
 *   Call TransferData() to perform data I/O.
 *
 * Parameter :	sdb	= SCSI data block, pointer to static values
 *		Index	= Device Channel number
 *
 ***********************************************************************/

static void

#if	DEBUG & DEBUG_HR
HandleRequest (SDB * sdb, word Index, int level)
#else
HandleRequest (SDB * sdb, word Index)
#endif
{
    char           *fname = "HandleRequest";
    RCB            *rcb = sdb->Request[Index];
    uword          *DevState = &(sdb->DevState[Index]);
    byte            Message[8];		/* Message buffer		 */
    DP              cdp;		/* current data pointers	 */
    bool            SelTrans;
    bool            IntError = FALSE;
    word            IntCode = 0xff;
    word            CmdCode;
    word            Phase;
    int             i;
    word            res;

#if	DEBUG & DEBUG_HR
    MyDebug (&sdb->debug, "%d->%d/%x", level, Index, GetIdent (*DevState));
#endif

start:
    Message[0] = 0x08;			/* Default NOP Msg		 */

    rcb->Done = FALSE;

    cdp = rcb->sdp;			/* restore Data pointers	 */

    X_SelectScsi (sdb);			/* Initialise Xilinx for SCSI	 */

/*
 * Check Device ID & LUN: if changed, store new ID and reset sync xfer	
 */

    if ((*DevState & DS_IdentMask) != (rcb->DriveID & DS_IdentMask))
    	*DevState = (rcb->DriveID & DS_IdentMask) | 0x20;

#if	DEBUG

/*
 * check Controller status, report if unexpected bits are set
 */
 
    if ((sdb->AuxStatus = S_ReadAuxStatus ()) != 0) {
	MyDebug (&sdb->debug, "%s %x: 1. unexpected Aux %x, Scsi %x, Cmd %x",
	       fname, GetIdent (*DevState), sdb->AuxStatus, sdb->ScsiStatus,
		 S_ReadReg (WR_Command));
    }
#endif

/*
 * Initialise Registers for this Request
 */
 
    rcb->Regs[0x01] = 0x44;
    rcb->Regs[0x02] = 0x80;
    rcb->Regs[0x0f] = GetLUN (*DevState);
    rcb->Regs[0x11] = GetSync (*DevState);
    rcb->Regs[0x12] = cdp.Rest >> 16;
    rcb->Regs[0x13] = cdp.Rest >> 8;
    rcb->Regs[0x14] = cdp.Rest;
    rcb->Regs[0x15] = GetID (*DevState) | (rcb->Read ? 0x40 : 0x00);
    rcb->Regs[0x16] = 0x00;

#if	DEBUG & DEBUG_REGS
    MyDebug (&sdb->debug, "%x Regs: %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x",
	     GetIdent (*DevState),
	 rcb->Regs[0x00], rcb->Regs[0x01], rcb->Regs[0x02], rcb->Regs[0x03],
	 rcb->Regs[0x04], rcb->Regs[0x05], rcb->Regs[0x06], rcb->Regs[0x07],
	 rcb->Regs[0x08], rcb->Regs[0x09], rcb->Regs[0x0a], rcb->Regs[0x0b],
	 rcb->Regs[0x0c], rcb->Regs[0x0d], rcb->Regs[0x0e], rcb->Regs[0x0f],
	 rcb->Regs[0x10], rcb->Regs[0x11], rcb->Regs[0x12], rcb->Regs[0x13],
	     rcb->Regs[0x14], rcb->Regs[0x15], rcb->Regs[0x16]);
#endif

/*
 * Initialise WD controller for the request
 */
    *WD_Address = 0;
    for (i = 0; i <= 0x16; i++)
	*WD_Register = rcb->Regs[i];

/*
 * Sync transfer not checked ?
 */
    if (GetSync (rcb->DriveID)		/* Device supports sync mode	 */
      && GetCheck (*DevState)) {	/* Sync mode not checked	 */
	SetCheck (*DevState);		/* mark as checked...		 */

#if	DEBUG & DEBUG_WDCMD
	MyDebug (&sdb->debug, "%x SelectATN", GetIdent (*DevState));
#endif

	if ((*WD_AuxStatus & 0x80) != 0) {	/* intermediate Int !	 */
	    IntCode = 0xff;
	    goto nextint;
	}
	S_WriteCmd (WC_SelectATN);	/* Select with ATN		 */

    /*
     * set up ID Message without Dis/Reconnect during Selection
     */

	Message[0] = 0x80 | GetLUN (*DevState);
	Message[1] = 0x01;		/* Extended Message		 */
	Message[2] = 0x03;		/* Extended Message length	 */
	Message[3] = 0x01;		/* SDT request code		 */
	Message[4] = 200 / 4;		/* Transfer period (200 ns / 4)	 */
	Message[5] = 12;		/* Offset (fifo depth)		 */

	if (NextInt (sdb, SelectTimeout, 0xff, fname) != 1) {
	    PutDebug (&sdb->debug);
	    IOdebug ("%s %x: SelectATN timed out !", fname, GetIdent (*DevState));
	    goto timeout;
	}
	i = sdb->ScsiStatus;
	if (i != 0x11) {		/* Select completed, ok		 */
	    if (i == 0x42)		/* Timeout during Arbitration	 */
	      goto noselect;
	    else			/* others should not happen	 */
	    goto interrupt;
	}
    /*
     * wait for Message out phase
     */
	if ((res = NextInt (sdb, MessageTimeout, 0x8e, fname)) != 1) {
	    if (res)
		goto interrupt;
	    else {
		PutDebug (&sdb->debug);
		IOdebug ("%s %x: Msg Out phase timed out !",
			 fname, GetIdent (*DevState));
		goto timeout;
	    }
	}
    /* 
     * send Message with ATN asserted	 
     */
	unless (WriteMessage (sdb, 6, Message)) {
	    PutDebug (&sdb->debug);
	    IOdebug ("%s %x: WriteMessage timed out !", fname, GetIdent (*DevState));
	    goto timeout;		/* time out error		 */
	}
    /*
     * check for Message in phase	 
     */
	if (sdb->ScsiStatus != 0x1f && sdb->ScsiStatus != 0x4f && sdb->ScsiStatus != 0x8f)
	    goto interrupt;
    /* 
     * receive Message		 
     */
	if ((i = ReadMessage (sdb, 5, Message, "HR sync")) != 5) {

#if	DEBUG & DEBUG_MSGIN
	    MyDebug (&sdb->debug, "%s %x: got only %d from 5 SDT bytes",
		     fname, GetIdent (*DevState), i);
#endif

	    unless (i) {		/* timeout error		 */
		PutDebug (&sdb->debug);
		IOdebug ("%s %x: ReadMessage timed out !", fname, GetIdent (*DevState));
		goto timeout;
	    }
	    if (Message[0] == 9)	/* Message parity error, retry	 */
		ClrCheck (*DevState);	/* with the next command	 */

	    if (i < 0)			/* ACK already negated		 */
		goto nextphase;
	}
	elif (Message[2] != 1) {
	    PutDebug (&sdb->debug);
	    IOdebug ("%s %x: received wrong Message (%x %x %x %x %x)",
		     fname, GetIdent (*DevState), Message[0], Message[1],
		     Message[2], Message[3], Message[4]);
	}
	else				/* received a sync message	 */
	{
	    int             tp = Message[3];
	    int             sync = Message[4];

	    if (sync != 0) {		/* sync facility found :	 */
		if (sync > 12)		/* adjust offset		 */
		    sync = 12;

		if (tp <= (200 / 4))	/* 5 MByte / sec	 */
		    sync |= 0x20;
		elif (tp <= (300 / 4))	/* 3,3 MByte / sec	 */
		  sync |= 0x30;
		else			/* too slow, asynchronous...	 */
		{
		    sync = 0x20;
		    S_WriteCmd (WC_AssertATN);	/* Assert ATN	 */
		    Message[0] = 0x07;
		}
	    }
	    SetSync (*DevState, sync);
	    S_WriteReg (WR_Synch, sync);
	    IOdebug ("%s %x : found Sync Facility: offset %d, Transfer Period %d",
		     fname, GetIdent (*DevState), sync & 0xf, ((sync & 0xf0) >> 4) * 100);
	}

    /*
     * S_WriteCmd ( WC_NegateACK );	/@* Negate ACK			*@/
     * IntCode = 0xff; goto nextint;
     */
	S_WriteReg (WR_CmdPhase, 0x20);
	rcb->Regs[WR_CmdPhase] = 0x20;
    }
transfer:
    if (rcb->Read)			/* set Data transfer direction,	 */
	X_Set (sdb, XF_Read);
    else
	X_Clr (sdb, XF_Read);

#if 1
    if (rcb->Regs[WR_CmdPhase] == 0) {

#if	DEBUG & DEBUG_WDCMD
	MyDebug (&sdb->debug, "%x SelectTransferATN", GetIdent (*DevState));
#endif

	CmdCode = WC_SelectTransferATN;
    } else {

#if	DEBUG & DEBUG_WDCMD
	MyDebug (&sdb->debug, "%x SelectTransfer", GetIdent (*DevState));
#endif

	CmdCode = WC_SelectTransfer;
    }

#if	DEBUG
/* check Controller status	 */
    if ((sdb->AuxStatus = S_ReadAuxStatus ()) != 0) {
    /* report extraordinary status	 */
	MyDebug (&sdb->debug, "%s %x: 2. unexpected Aux %x, Scsi %x, Cmd %x",
	       fname, GetIdent (*DevState), sdb->AuxStatus, sdb->ScsiStatus,
		 S_ReadReg (WR_Command));
    }
#endif

    *WD_Address = WR_Command;
    if ((*WD_AuxStatus & 0x80) != 0) {	/* intermediate Interrupt !	 */
	IntCode = 0xff;
	goto nextint;
    }
    *WD_Register = (uword) CmdCode;

#else

#if	DEBUG & DEBUG_WDCMD
    MyDebug (&sdb->debug, "%x SelectTransferATN", GetIdent (*DevState));
#endif

    if ((*WD_AuxStatus & 0x80) != 0) {	/* intermediate Interrupt !	 */
	IntCode = 0xff;
	goto nextint;
    }
    S_WriteCmd (WC_SelectTransferATN);
#endif

    SelTrans = TRUE;
    if (cdp.Rest <= 0) {
	IntCode = 0xff;
	goto nextint;
    }
    if (rcb->Read)
	res = ReadData (sdb, rcb, &cdp);
    else
	res = WriteData (sdb, rcb, &cdp);

    if (res < 1) {
	IOdebug ("%s %x: Data phase timed out !", fname, GetIdent (*DevState));
	goto timeout;
    }
    goto nextphase;

resume:

#if	DEBUG & DEBUG_WDCMD
    MyDebug (&sdb->debug, "%x Resume SelectTransfer", GetIdent (*DevState));
#endif

    if ((*WD_AuxStatus & 0x80) != 0)	/* intermediate Interrupt !	 */
	goto nextint;

    S_WriteCmd (WC_SelectTransferATN);
    SelTrans = TRUE;
    IntCode = 0xff;

nextint:
    if ((res = NextInt (sdb, NoTimeout, IntCode, fname)) != 1) {
	if (res)
	    goto interrupt;		/* unexpected interrupt		 */
	else {				/* timeout error		 */
	    PutDebug (&sdb->debug);
	    IOdebug ("%s %x: NextInt timed out !", fname, GetIdent (*DevState));
	    goto timeout;
	}
    }
nextphase:
    Phase = S_ReadReg (WR_CmdPhase);
    switch (sdb->ScsiStatus) {
    case 0x11:				/* Select complete		 */
	IntCode = 0x8a;
	goto nextint;

    case 0x16:				/* SelectTransfer complete	 */
	rcb->sdp = cdp;			/* save the pointers		 */
	rcb->Status = S_ReadReg (WR_TargetLUN);
	unless (rcb->Done) {
	    rcb->Done = TRUE;		/* send result to DevHandler	 */
	    WriteWord (sdb->InOut[Index], (word) rcb);
	    sdb->Request[Index] = NULL;	/* clear pending mark	 */

#if	DEBUG & DEBUG_HR
	    MyDebug (&sdb->debug, "%d \\ End1 %d/%x:%x",
		     level, Index, GetIdent (*DevState), rcb);
#endif
	}
	SelTrans = FALSE;
	IntCode = 0xff;
	goto nextint;

    case 0x18:				/* Data out phase		 */
    case 0x88:
	cdp = rcb->sdp;
	i = cdp.Rest;
	S_WriteTCount (i);		/* set Transfer count		 */
	if (SelTrans) {			/* resume before data completed	 */
	    S_WriteReg (WR_CmdPhase, 0x41);
	    goto transfer;
	}
	X_Clr (sdb, XF_Read);		/* set Data transfer direction,	 */
	S_WriteReg (WR_Control, 0x44);
	S_WriteReg (WR_Synch, GetSync (*DevState));
	S_WriteCmd (WC_Transfer);
	unless (WriteData (sdb, rcb, &cdp)) {
	    IOdebug ("%s %x: WriteData timed out !", fname, GetIdent (*DevState));
	    goto timeout;
	}
	goto nextphase;

    case 0x19:				/* Data in phase		 */
    case 0x89:
	cdp = rcb->sdp;
	i = cdp.Rest;
	S_WriteTCount (i);		/* set Transfer count		 */
	if (SelTrans) {			/* resume before data completed	 */
	    S_WriteReg (WR_CmdPhase, 0x41);
	    goto transfer;
	}
	X_Set (sdb, XF_Read);		/* set Data transfer direction,	 */
	S_WriteReg (WR_Control, 0x44);
	S_WriteReg (WR_Synch, GetSync (*DevState));
	S_WriteCmd (WC_Transfer);
	unless (ReadData (sdb, rcb, &cdp)) {
	    IOdebug ("%s %x: ReadData timed out !", fname, GetIdent (*DevState));
	    goto timeout;
	}
	goto nextphase;

    case 0x1a:				/* Command phase		 */
    case 0x4a:
    case 0x8a:
	res = WriteCommand (sdb, rcb->Regs[WR_CDBSize], &rcb->Regs[WR_CDB01]);

#if	DEBUG & DEBUG_CMD
	MyDebug (&sdb->debug, "WriteCommand (%d) = %d",
		 rcb->Regs[WR_CDBSize], res);
#endif

	if (res == 0) {
	    PutDebug (&sdb->debug);
	    IOdebug ("%s %x: WriteCommand timed out !",
		     fname, GetIdent (*DevState));
	    goto timeout;
	}
	goto nextphase;

    case 0x1b:				/* Status phase			 */
    case 0x8b:				/* inmplies command end, save	 */
	rcb->sdp = cdp;			/* data pointers		 */
	res = ReadStatus (sdb, rcb);

#if	DEBUG & DEBUG_STAT
	MyDebug (&sdb->debug, "%s %x: Status %d",
		 fname, GetIdent (*DevState), rcb->Status);
#endif

	if (res == 0) {
	    PutDebug (&sdb->debug);
	    IOdebug ("%s %x: ReadStatus timed out !", fname, GetIdent (*DevState));
	    goto timeout;
	}
	goto nextphase;

    case 0x1e:
    case 0x4e:
    case 0x8e:				/* Message out phase, error	 */
	unless (WriteMessage (sdb, 1, Message)) {
	    PutDebug (&sdb->debug);
	    IOdebug ("%s %x: WriteMessage timed out !",
		     fname, GetIdent (*DevState));
	    goto timeout;
	}
	Message[0] = 0x08;		/* next send a nop message	 */
	goto nextphase;

    case 0x1f:				/* Message in phase		 */
    case 0x4f:
    case 0x8f:

#if 0
	i = S_ReadReg (WR_SourceID);
	if ((i & 8) && ((i & 7) != GetID (*DevState))) {

#if	DEBUG
	    MyDebug (&sdb->debug, "%s %x: Connect changed from %x to %x ! (Aux %x, Cmd %x)",
		   fname, GetIdent (*DevState), GetID (*DevState), i & 0x07,
		     S_ReadAuxStatus (), S_ReadReg (WR_Command));
#endif

#if 0
	    MyDebug (&sdb->debug, "%x Msg Regs: %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x data %x",
		     GetIdent (*DevState),
		     S_ReadReg (0x00), S_ReadReg (0x01), S_ReadReg (0x02),
		     S_ReadReg (0x03), S_ReadReg (0x04), S_ReadReg (0x05),
		     S_ReadReg (0x06), S_ReadReg (0x07), S_ReadReg (0x08),
		     S_ReadReg (0x09), S_ReadReg (0x0a), S_ReadReg (0x0b),
		     S_ReadReg (0x0c), S_ReadReg (0x0d), S_ReadReg (0x0e),
		     S_ReadReg (0x0f), S_ReadReg (0x10), S_ReadReg (0x11),
		     S_ReadReg (0x12), S_ReadReg (0x13), S_ReadReg (0x14),
		     S_ReadReg (0x15), S_ReadReg (0x16), S_ReadReg (0x19));
#endif

	/* Find reconnect index		 */
	    i = FindIndex (sdb, S_ReadReg (WR_SourceID), S_ReadData ());
	    if (i >= 0) {		/* complete Request		 */
		sdb->Request[i]->Regs[WR_CmdPhase] = 0x44;

#if	DEBUG & DEBUG_HR
		HandleRequest (sdb, i, level + 1);
#else
		HandleRequest (sdb, i);
#endif
	    }
	    goto start;			/* retry the Arbitration	 */
	}
#endif

	if ((i = ReadMessage (sdb, 1, (byte *) & res, "HR msg")) == 0) {
	    PutDebug (&sdb->debug);
	    IOdebug ("%s %x: ReadMessage timed out !",
		     fname, GetIdent (*DevState));
	    goto timeout;
	}

#if	DEBUG & DEBUG_MSGIN
	MyDebug (&sdb->debug, "%s %x: Msg in %x",
		 fname, GetIdent (*DevState), res);
#endif

	switch (res) {			/* evaluate Message		 */
	case 0:				/* command complete		 */
	case 8:				/* no operation			 */
	case 10:			/* linked command complete	 */
	case 11:			/* linked command complete w/flg */
	    break;
	case 2:
	    rcb->sdp = cdp;		/* save data pointers		 */
	    break;
	case 3:				/* restore pointers		 */
	    cdp = rcb->sdp;
	    break;
	default:
	    PutDebug (&sdb->debug);
	    IOdebug ("%s %x: Unknown message code received: 0x%x",
		     fname, GetIdent (*DevState), res);
	}
	if (i > 0)
	    S_WriteCmd (WC_NegateACK);
	goto nextint;

    case 0x42:				/* timeout during selection	 */
noselect:
	rcb->Status = EC_Error + SS_Device + EG_Unknown + EO_Drive;

	unless (rcb->Done) {
	    rcb->Done = TRUE;		/* send result to DevHandler	 */
	    WriteWord (sdb->InOut[Index], (word) rcb);
	    sdb->Request[Index] = NULL;	/* clear pending mark	 */

#if	DEBUG & DEBUG_HR
	    MyDebug (&sdb->debug, "%d \\ End2 %d/%x:%x",
		     level, Index, GetIdent (*DevState), rcb);
#endif
	}
	return;

    case 0x47:				/* bad status byte		 */
	res = S_ReadData ();
	PutDebug (&sdb->debug);
	IOdebug ("%s %x: Bad Status byte: phase %x, msg %x",
		 fname, GetIdent (*DevState), Phase, res);
	goto interrupt;

    /* case 0x4a : */
    /* unexpected Command phase	 */
	if (SelTrans && Phase >= 0x50) {/* status byte received		 */
	    rcb->sdp = cdp;		/* save the pointers		 */
	/* save device status		 */
	    rcb->Status = S_ReadReg (WR_Data);
	    unless (rcb->Done) {
		rcb->Done = TRUE;	/* send result to DevHandler	 */
		WriteWord (sdb->InOut[Index], (word) rcb);
		sdb->Request[Index] = NULL;	/* clear pending mark	 */

#if	DEBUG & DEBUG_HR
		MyDebug (&sdb->debug, "%d \\ End3 %d/%x:%x",
			 level, Index, GetIdent (*DevState), rcb);
#endif
	    }
	/* read next request from this DevHandler		 */
	    ReadWord (sdb->InOut[Index], rcb);
	    sdb->Request[Index] = rcb;	/* clear pending mark		 */

#if	DEBUG & DEBUG_HR
	    MyDebug (&sdb->debug, "%d / Link %d", level, Index);
#endif

	/* skip over Arbitration and	 */
	    rcb->Regs[WR_CmdPhase] = 0x30;	/* ID Message transfer	 */
	    goto start;			/* and go on.			 */
	}
	goto interrupt;			/* otherwise this is an error !	 */

    case 0x4b:				/* unexpected status phase	 */
	S_ClearTCount ();		/* reset data count		 */
	S_WriteReg (WR_CmdPhase, 0x41);
	goto resume;			/* resume command		 */

    case 0x85 :				/* target has disconnected	*/
	unless ( rcb->Done )		/* request not completed	*/
	{
	    *WD_Address = 0;		/* save register contents	*/
	    for ( i = 0; i < 23; i++ )
		rcb->Regs [i] = *WD_Register;
	    rcb->Done = TRUE;
	    WriteWord ( sdb->InOut [Index], ( word ) rcb );
	    sdb->Request [Index] = NULL;	/* clear pending mark	*/
#if	DEBUG & DEBUG_HR
	    MyDebug ( &sdb->debug, "%d \\ End4 %d/%x:%x",
		level, Index, GetIdent ( *DevState ), rcb);
#endif
	}
	return;

    case 0xff:				/* Status register not readable	 */
	while (*WD_AuxStatus & 0x10)	/* wait until CIP is cleared	 */
	    Delay (10);
	sdb->ScsiStatus = S_ReadScsiStatus ();
	goto nextphase;

    }

interrupt:				/* first interrupt, try again	 */
    unless (IntError) {
	PutDebug (&sdb->debug);
	IOdebug ("%s %x: unexpected interrupt %x, phase %x, trying to continue.",
		 fname, GetIdent (*DevState), sdb->ScsiStatus, Phase);
	IntError = TRUE;
	if (sdb->ScsiStatus == 0xff) {
	    while (*WD_AuxStatus & 0x10)
		Delay (10);
	    sdb->ScsiStatus = S_ReadScsiStatus ();
	}
	goto nextphase;
    }
/* next interrupt is an error	 */
    PutDebug (&sdb->debug);
    IOdebug ("%s %x: second unexpected interrupt %x, phase %x, aborting.",
	     fname, GetIdent (*DevState), sdb->ScsiStatus, Phase);
    res = EC_Error + SS_Device + EG_UnknownError + EO_Drive;
    goto tidyup;

timeout:				/* any timeout condition	 */
    res = EC_Error + SS_Device + EG_Timeout + EO_Request;
tidyup:
    unless (rcb->Done) {
	rcb->Status = res;
	rcb->Done = TRUE;		/* send result to DevHandler	 */
	WriteWord (sdb->InOut[Index], (word) rcb);
	sdb->Request[Index] = NULL;	/* clear pending mark		 */

#if	DEBUG & DEBUG_HR
	MyDebug (&sdb->debug, "%d \\ End5 %d/%x:%x",
		 level, Index, GetIdent (*DevState), rcb);
#endif
    }

#if	DEBUG & DEBUG_TIDYUP
    MyDebug (&sdb->debug, "%s %x calling Tidyup...", fname, GetIdent (*DevState));
#endif

    Tidyup (sdb);
    return;
}

/************************************************************************
 * HANDLE THE SCSI PROTOCOL
 * - Test and initialise the SCSI controller, return a result on InOut[0].
 * - Alternate on the Event channel and the InOut channels and
 *   handle all coming requests.
 *
 * Parameter :	sdb	= SCSI data block, pointer to static values
 *
 ***********************************************************************/

void
ScsiHandler (SDB * sdb)
{
    char           *fname = "ScsiHandler";

#if	DEBUG & DEBUG_SCSI
    word            start = 0;
    word            end;

#endif
    word            dummy;
    Channel        *Event = sdb->Event;
    Channel       **InOut = sdb->InOut;
    word            nInOut = sdb->nInOut;

    word            Index;
    RCB            *rcb;
    word            result;

/*
 * Test and initialise the SCSI controller,
 * send the result back and terminate on error.
 */
    result = ScsiInit (sdb);
    WriteWord (InOut[0], result);
    if (!result)
	return;

    forever
    {

#if	DEBUG & DEBUG_SCSI
	end = ldtimer_ ();
	MyDebug (&sdb->debug, "ScsiHandler time %d", end - start);
#endif

#if	DEBUG
	MyDebug (&sdb->debug, "done.");
	ClearDebug (&sdb->debug);
#endif

test:
	Index = AltWait (Event, nInOut, InOut, sdb->Request, -1);
	if (!(Index + 2))		/* timeout occured		 */
	    goto test;

#if	DEBUG & DEBUG_SCSI
	start = ldtimer_ ();
#endif

/*
 * An Event has triggered. This may be from any event source, 
 * but as we do not support disconnect/reconnect, there should 
 * never be an SCSi event...
 */
	if (!(Index + 1)) {		/* triggered by Event		 */
	    dummy = *XI_Status;		/* check for SCSI interrupt.	 */
	    if ((dummy & XB_Interrupt) == 0) {

#if	DEBUG
		MyDebug (&sdb->debug, "%s : Unexpected event (%x) ignored !",
			 fname, dummy & 0xF0);
#endif

		X_Clr (sdb, XF_DBA);	/* reset WORD bit	 */
		ReadWord (Event, dummy);/* and ack Event.	 */
		continue;
	    }
	    sdb->ScsiStatus = S_ReadScsiStatus ();

#if	DEBUG & DEBUG_INT
	    MyDebug (&sdb->debug, "Int %x in %s", sdb->ScsiStatus, fname);
#endif

	    ReadWord (Event, dummy);	/* and ack Event.	 */

	    PutDebug (&sdb->debug);
	    IOdebug ("%s : Unexpected SCSI interrupt (%x) !",
			 fname, sdb->ScsiStatus);
	    Tidyup (sdb);
	    continue;
	} else {			/* triggered by InOut channel	 */
	
	    ReadWord (InOut[Index], rcb);	/* read rcb address	 */
	    if (rcb == NULL) {		/* received a NULL pointer	 */

#if	DEBUG & DEBUG_SCSI
		MyDebug (&sdb->debug, "%s : got NULL rcb from %d", fname, Index);
#endif

#if	DEBUG & DEBUG_HR
		MyDebug (&sdb->debug, "S NULL %d", Index);
#endif

		WriteWord (InOut[Index], rcb);

#if	DEBUG & DEBUG_HR
		MyDebug (&sdb->debug, "S \\ End6 %d:%x", Index, rcb);
#endif

		if (Index == 0) {	/* InOut [0] will terminate us.	 */

#if	DEBUG & DEBUG_SCSI
		    MyDebug (&sdb->debug, "%s terminating.", fname);
#endif

		    return;
		} else			/* other InOuts are ignored.	 */
		    continue;
	    } else {			/* valid request pointer	 */
		rcb->Regs[WR_CmdPhase] = 0;	/* clear Command Phase	 */
		rcb->sdp.Rest = rcb->Size;
		rcb->sdp.Data = rcb->Data;
		sdb->Request[Index] = rcb;

#if	DEBUG & DEBUG_HR
		MyDebug (&sdb->debug, "S / New %d:%x", Index, rcb);
#endif
	    }
	}

#if	DEBUG & DEBUG_HR
	MyDebug (&sdb->debug, "Call HR");
	HandleRequest (sdb, Index, 1);
#else
	HandleRequest (sdb, Index);
#endif
    }
}

void
FastScsiHandler (SDB * sdb)
{
    Carrier        *FastMem;

    if ((FastMem = AllocFast (2000, &MyTask->MemPool)) == NULL)
	ScsiHandler (sdb);
    else {
	Accelerate (FastMem, ScsiHandler, sizeof (SDB *), sdb);
	FreeMem (FastMem);
    }
}

/************************************************************************
 * START THE SCSI HANDLER PROCESS AT HIGH PRIORITY
 * - Fork the ScsiHandler() as a high priority process and
 *   wait for the result of the WD 33C93A Scsi Controller test.
 * - After completion, the ScsiHandler is ready to use.
 *
 * Parameter :	dcb	= Disc DCB
 * Result    :	TRUE	ScsiHandler started successfully
 *		FALSE	failed to fork, or Controller test failed
 *
 ***********************************************************************/

bool
StartScsiHandler (DiscDCB * dcb)
{
    word            ok;

    ok = resetch_ (dcb->sdb.Event);
    ok = resetch_ (dcb->sdb.Event);
    ok = resetch_ (dcb->sdb.Event);
    X_Set (&dcb->sdb, XF_Event);

#ifdef	DEBUGGING
    if (!Fork (2000, ScsiHandler, 4, &dcb->sdb))
#else

/*
 * if ( ! Fork ( 2000, ScsiHandler, 4, &dcb->sdb ))
 * 
 * if ( ! PriFork ( LowPri, 2000, FastScsiHandler, 4, &dcb->sdb )) 
 *
 * if ( ! PriFork ( LowPri, 2000, ScsiHandler, 4, &dcb->sdb )) 
 *
 * if ( ! PriFork ( HighPri, 2000, FastScsiHandler, 4, &dcb->sdb ))
 */
    if (!PriFork (HighPri, 2000, FastScsiHandler, 4, &dcb->sdb))
#endif

	return FALSE;

    ReadWord (dcb->Channels[0], ok);
    if (!ok)
	IOdebug ("SCSI controller test failed");
    return ok;
}

/************************************************************************
 * STOP THE SCSI HANDLER PROCESS
 * - Send a NULL word over Channel 0. This causes the ScsiHandler
 *   to terminate. Wait for the resut word, which indicates recognition
 *   of the termination request.
 *
 * Parameter :	dcb	= Disc DCB
 *
 ***********************************************************************/

void
StopScsiHandler (DiscDCB * dcb)
{
    word            ok;

    WriteWord (dcb->Channels[0], 0);
    ReadWord (dcb->Channels[0], ok);

    X_Clr (&dcb->sdb, XF_Event);
    ChanReset (&dcb->sdb.Event);
}

/*--- end of scsi.c ---*/
