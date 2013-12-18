/*
 * $Header: /Chris/00/helios/msc/RCS/msc.h,v 2.0 91/08/21 18:04:24 chris Exp
 * Locker: chris $
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
** msc.h								**
**									**
**	- Headers and Definitions for the MSC Device Driver		**
**									**
**************************************************************************
** HISTORY   :								**
** -----------								**
** Author    :	14/09/90 : C. Fleischer					**
*************************************************************************/

#ifndef __msc_h
#define __msc_h

#include <helios.h>
#include <syslib.h>
#include <asm.h>
#include <sem.h>
#include <queue.h>
#include <device.h>
#include <codes.h>
#include <stdarg.h>
#include <process.h>
#include <string.h>
#include <ctype.h>
#include <queue.h>
#include <memory.h>
#include <nonansi.h>
#include "mscaddr.h"
#include "mscstruc.h"

#ifndef DEBUG
#define DEBUG		0
#endif

#define DEBUG_HR	0x1		/* show HandleRequest levels	 */
#define	DEBUG_INT	0x2		/* show Interrupts		 */
#define DEBUG_CMD	0x4		/* show Command Phase		 */
#define DEBUG_MSGOUT	0x8		/* show Message out Phase	 */
#define DEBUG_MSGIN	0x10		/* show Message in Phase	 */
#define DEBUG_RESET	0x20		/* show SCSI controller reset	 */
#define DEBUG_TIDYUP	0x40		/* show Tidyup works		 */
#define DEBUG_INIT	0x80		/* show initialisation checks	 */
#define DEBUG_REGS	0x100		/* show command registers	 */
#define DEBUG_WDCMD	0x200		/* show HR controller commands	 */
#define DEBUG_STAT	0x400		/* show Status Phase		 */
#define DEBUG_HACK	0x800		/* show hacks			 */
#define DEBUG_ABORT	0x1000		/* show SCSI command aborting	 */
#define DEBUG_SCSI	0x2000		/* SCSI handler debugging	 */
#define DEBUG_SENSE	0x4000		/* show Sense result & ErrCode	 */
#define DEBUG_FMT	0x8000		/* show Format messages		 */

#define	ResetDelay	10		/* 10 us reset pulse width	 */

#define NoTimeout	(-1)

#ifdef	DEBUGGING
#define	ScsiTimeout	(10*OneSec/64)	/* 1 s timeout too short...	 */
#define	ResetTimeout	ScsiTimeout+1	/* 1  s reset int timeout	 */
#define DataTimeout	NoTimeout	/* 1  s data arrival timeout	 */
#define MessageTimeout	NoTimeout	/* 1  s message timeout	 */
#define SelectTimeout	ScsiTimeout	/* 5  s arbitration timeout	 */
#define StatusTimeout	NoTimeout	/* 1  s status phase timeout	 */
#define CommandTimeout	NoTimeout	/* 1  s cmd phase timeout	 */
#define InterruptTimeout NoTimeout	/* 1  s timeout between ints	 */

#else

#define	ScsiTimeout	(10*OneSec)	/* 1 s timeout too short...	 */
#define	ResetTimeout	ScsiTimeout+1	/* 1  s reset int timeout	 */
#define DataTimeout	NoTimeout	/* 1  s data arrival timeout	 */
#define MessageTimeout	NoTimeout	/* 1  s message timeout	 */
#define SelectTimeout	ScsiTimeout	/* 5  s arbitration timeout	 */
#define StatusTimeout	NoTimeout	/* 1  s status phase timeout	 */
#define CommandTimeout	NoTimeout	/* 1  s cmd phase timeout	 */
#define InterruptTimeout NoTimeout	/* 1  s timeout between ints	 */
#endif

/*-------------------- Channel communication macros --------------------*/

#define WriteWord(c,w)	(ajw_(-1),_operate(0x0F,(w),(c)),ajw_(1))
#define ReadWord(c,w)	(ajw_(-1),in_(4,(c),&(w)),ajw_(1))
#define ChanReset(c)	resetch_(c)

extern void     (WriteWord) (Channel * c, word w);
extern void     (ReadWord) (Channel * c, word w);
extern          word (ChanReset) (Channel * c);

/*---------------------------- other macros ----------------------------*/

#define ShL(val,sh)	(((sh)>0)?(val)<<(sh):(val)>>(-(sh)))
#define ShR(val,sh)	(((sh)>0)?(val)>>(sh):(val)<<(-(sh)))

/*-----------------------  External prototypes  ------------------------*/

int             _ldtimer (int pri);

/*------------------------  Local prototypes  --------------------------*/

/*--- mscasm.a ---*/

word 		AltWait (Channel * Event, int nInOut,
			 Channel ** InOut, RCB ** Request, word Timeout);
word            IntWait (Channel * Event, word Timeout);

/*--- scsi.c ---*/

bool            ScsiInit (SDB * sdb);
void            ScsiHandler (SDB * sdb);
bool            StartScsiHandler (DiscDCB * dcb);
void            StopScsiHandler (DiscDCB * dcb);


/*--- utility.c ---*/

#define	HighPri		0		/* Priority code values		 */
#define LowPri		1

char           *stime (char *buf);
word 		PriFork (word pri, word stsize,
		 VoidFnPtr fn, word argsize,...);
word 		WspFork (word pri, word * stack, word stsize,
		 VoidFnPtr fn, word argsize,...);
void            StartEvents (DiscDCB * dcb);
void            StopEvents (DiscDCB * dcb);

/*--- device.c ---*/

bool 		StartDriveHandler (DriveDesc * dd, word dnum,
		 DiscReq * req, LoadResult * lr);

#endif

/*--- end of msc.h ---*/
