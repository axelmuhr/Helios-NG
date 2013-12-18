/*
 * $Header: /Chris/00/helios/msc/RCS/utility.c,v 2.0 91/08/21 18:06:30 chris
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
** utility.c								**
**									**
**	- Process handling functions					**
**									**
**************************************************************************
** HISTORY   :								**
** -----------								**
** Author    :	xx/xx/90 : C. Fleischer					**
*************************************************************************/


#ifndef	Driver
#include "msc.h"
#endif

#include <root.h>

/*************************************************************************
 * ADD AN INTEGER TO A STRING
 *
 * - Check sign, calc the digits and add them in the right order.
 *
 * Parameter  :	bp	= string pointer
 *		v	= integer value
 *		w	= minimum number width ( fill with zeroes )
 * Result :	updated string pointer
 *
 ************************************************************************/

char           *
putint (char *bp, int v, int w)
{
    char           *digits = "0123456789";
    int             n = v < 0;
    int             p = 0;
    char            b[12];

    if (n)
	v = -v;
    do {
	b[p++] = digits[v % 10];
	v /= 10;
    }
    while (--w > 0 || v);
    if (n)
	*bp++ = '-';
    while (p--)
	*bp++ = b[p];
    return bp;
}

/*************************************************************************
 * CREATE AN ASCII TIMESTAMP FROM THE CURRENT SYSTEM TIME
 *
 * - Get the system time and convert it into a user-supplied buffer.
 *
 * Parameter  :	sbuf	= user buffer pointer
 *
 ************************************************************************/

char           *
stime (char *sbuf)
{
    char           *nam = "JanFebMarAprMaiJunJulAugSepOctNovDecBad";
    char           *sp = sbuf;
    word            t = GetRoot ()->Time;
    word            sec;
    word            min;
    word            hour;
    word            day;
    word            month;
    word            year = 1900;

    sec = t % 60;
    t /= 60;
    min = t % 60;
    t /= 60;
    hour = t % 24;
    t /= 24;
    t += 70 * 365 + 18;			/* convert t into days since 1.1.1900	 */
    year += 4 * (t / (365 * 4 + 1));
    t %= 365 * 4 + 1;
    if (t >= 366)
	year += (t - 1) / 365, t = (t - 1) % 365;
    if ((year & 3) != 0 && t >= 31 + 28)
	t++;
    if (t < 31)
	month = 0;
    elif ((t -= 31) < 29) month = 1;
    elif ((t -= 29) < 31) month = 2;
    elif ((t -= 31) < 30) month = 3;
    elif ((t -= 30) < 31) month = 4;
    elif ((t -= 31) < 30) month = 5;
    elif ((t -= 30) < 31) month = 6;
    elif ((t -= 31) < 31) month = 7;
    elif ((t -= 31) < 30) month = 8;
    elif ((t -= 30) < 31) month = 9;
    elif ((t -= 31) < 30) month = 10;
    elif ((t -= 30) < 31) month = 11;
    else
    month = 12;
    nam += month * 3;
    day = t + 1;
    *sp++ = *nam++;
    *sp++ = *nam++;
    *sp++ = *nam++;
    *sp++ = ' ';
    sp = putint (sp, day, 0);
    *sp++ = ',';
    sp = putint (sp, year, 4);
    *sp++ = ' ';
    sp = putint (sp, hour, 2);
    *sp++ = ':';
    sp = putint (sp, min, 2);
    *sp++ = ':';
    sp = putint (sp, sec, 2);
    *sp = '\0';
    return sbuf;
}

#ifndef	EVENTS
/*************************************************************************
 * WATCH THE XILINX STATUS REGISTER
 *
 * - Poll the Xilinx Status register every 100 us, send a word on the
 *   Event channel if the Interrupt bit or the Word bit is set.
 *
 * Parameter  :	Event	= Channel to send on
 *		stop	= pointer to termination flag
 *
 ************************************************************************/

#define Sleep(t) 	tin_(sum_((t),ldtimer_()))

static void
WatchXilinx (Channel * chan, bool * stop, Semaphore * stopped)
{
    uword           status;

    forever
    {
	if ((status = *XI_Status & 0xD0) != 0) {

#if	DEBUG & DEBUG_INT
	    IOdebug ("Event %x", status);
#endif

	    WriteWord (chan, status);
	} else
	    Sleep (100);
	if (*stop) {
	    Signal (stopped);
	    return;
	}
    }
}

/*************************************************************************
 * START THE WATCH XILINX PROCESS
 *
 * - Initialise the related dcb variables
 *   and fork the Watch process at HighPri.
 *
 * Parameter  :	dcb	= Disc DCB
 *
 ************************************************************************/

void
StartEvents (DiscDCB * dcb)
{
    ChanReset (&dcb->EventChan);
    dcb->stop = FALSE;
    InitSemaphore (&dcb->stopped, 0);

    IOdebug ("starting WatchXilinx");

    PriFork (HighPri, 500, WatchXilinx,
	     12, &dcb->EventChan, &dcb->stop, &dcb->stopped);
}

/*************************************************************************
 * START THE WATCH XILINX PROCESS
 *
 * - Set the termination flag and wait for the Watch process to terminate.
 *
 * Parameter  :	dcb	= Disc DCB
 *
 ************************************************************************/

void
StopEvents (DiscDCB * dcb)
{
    dcb->stop = TRUE;
    Wait (&dcb->stopped);
    IOdebug ("WatchXilinx stopped.");

    Delay (OneSec / 10);
}

#endif

/*************************************************************************
 * FORK A PROCESS WITH SELECTED PRIORITY
 *
 * Parameter  :	pri	= process priority (0 = high, 1 = low)
 *		stsize	= required stack size for the process
 *		fn	= function which forms the process
 *		argsize	= number of parameter bytes
 *		...	= parameters to be passed to the new process
 * Return	TRUE	if the process was started
 *		FALSE	if the stack allocation failed
 *
 ************************************************************************/

word
PriFork (word pri, word stsize, VoidFnPtr fn, word argsize,...)
{
    byte           *args = ((byte *) & argsize) + sizeof (argsize);
    word           *proc = (word *) NewProcess (stsize, fn, argsize);

    if (proc == NULL)
	return FALSE;

    memcpy (proc, args, argsize);

    StartProcess (proc, pri & 1);

    return TRUE;
}


void            ProcExit (void);	/* only part in assembler	 */

/*************************************************************************
 * INITIALISE A PROCESS WITH A GIVEN STACK
 *
 * Parameter  :	stack	= pointer to stack
 *		stsize	= stack size
 *		fn	= function which forms the process
 *		nargs	= number of parameter words
 * Return	pointer to initialised process
 *
 ************************************************************************/

static void    *
WspProcess (word * stack, word stacksize, VoidFnPtr fn, word nargs)
{
/* display size is 6 words 	 */
    word           *display = stack + (stacksize / sizeof (word)) - 6;
    word           *p;

    if (stack == NULL)
	return NULL;

/* transputer magic 		 */
    display[0] = (((word **) & stacksize)[-1])[0];
    display[1] = (word) stack;

    if (nargs < 8)
	nargs = 8;			/* leave at least 2 words 	 */

    p = InitProcess (display, fn, ProcExit, display, nargs);

    return p;
}


/*************************************************************************
 * FORK A PROCESS WITH A PREALLOCATED WORKSPACE
 *
 * Parameter  :	pri	= process priority (0 = high, 1 = low)
 *		stack	= pointer to stack
 *		stsize	= stack size
 *		fn	= function which forms the process
 *		argsize	= number of parameter bytes
 *		...	= parameters to be passed to the new process
 * Return	TRUE	if the process was started
 *		FALSE	if the stack allocation failed
 *
 ************************************************************************/

word
WspFork (word pri, word * stack, word stsize, VoidFnPtr fn, word argsize,...)
{
    byte           *args = ((byte *) & argsize) + sizeof (argsize);
    word           *proc = (word *) WspProcess (stack, stsize, fn, argsize);

    if (proc == NULL)
	return NULL;

    memcpy (proc, args, argsize);

    StartProcess (proc, pri & 1);
}

/*--- end of utility.c ---*/
