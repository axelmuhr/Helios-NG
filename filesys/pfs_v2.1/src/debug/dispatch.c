/*************************************************************************
**									**
**	                  I O S   N U C L E U S              		**
**	                  ---------------------           		**
**									**
**		    Copyright (C) 1989, Parsytec GmbH			**
**			  All Rights Reserved.				**
**									**
**									**
** dispatch.c								**
**									**
**	- Abortable dispatcher						**
**									**
**************************************************************************
** HISTORY   :								**
** -----------								**
** Author    :	01/12/89 : C. Fleischer					**
*************************************************************************/

#define __in_dispatch 1			/* flag that we are in this module */

#include <helios.h>	/* standard header */
#include <stdarg.h>
#include <string.h>
#include <queue.h>
#include <protect.h>
#include <message.h>
#include <codes.h>
#include <syslib.h>
#include <process.h>
#include <gsp.h>
#include <servlib.h>

#undef	Malloc

static void	*SafetyNet	= NULL;
static int	SafetySize	= 5 * 1024;

/*************************************************************************
 * DISPATCHER MALLOC WITH SAFETYNET
 *
 * - If a Malloc fails the SafetyNet block is freed, 
 *   but a memory failure is reported anyway. 
 *   This should allow the server to respond to 
 *   any subsequent cleanup requests sent it.
 *
 * Parameter  :	size	= Malloc size
 * Return     :	pointer to allocated block or NULL if failure.
 *
 ************************************************************************/

void 
*DispMalloc (word size) 
{
    void 	*v 	= Malloc (size);

    if (v == NULL) 
    {
	Free (SafetyNet);
	SafetyNet = NULL;
    }
    return v;
}


/*************************************************************************
 * DISPATCH WORKER TO CALL THE DEFINED HANDLING FUNCTION
 *
 * Parameter  :	m	= MsgBuf of Request
 *		info	= DispatchInfo
 *		e	= selected DispatchEntry
 * Return     :	- nothing -
 *
 ************************************************************************/

static void 
Worker (MsgBuf *m, DispatchInfo *info, DispatchEntry *e) 
{
    DirNode	*d;
    ServInfo	servinfo;
    WordFnPtr	fn;

    if (setjmp (servinfo.Escape) != 0)  /* Install user emergency exit.	*/
        goto done;

    servinfo.m = &m->mcb;		/* Create the ServInfo for this	*/
    servinfo.Context = info->Root;	/* request.			*/
    servinfo.Target = (ObjNode *) info->Root;
    servinfo.TargetLocked = FALSE;
    servinfo.FnCode = m->mcb.MsgHdr.FnRc;
    servinfo.DispatchInfo = info;
    m->mcb.MsgHdr.FnRc = info->SubSys;

    d = GetContext (&servinfo);		/* Check and lock the Context.	*/

    if (d == Null (DirNode)) 		/* failed, abort.		*/
        ErrorMsg (&m->mcb, 0);
    else
    {
	if ((e == NULL) || (e == &info->PrivateProtocol)) 
	{				/* unknown or private function:	*/
	    fn = (WordFnPtr) (e->Fn);	/* try to get handler, 		*/

	    if ((e == NULL) || (fn == NULL) || (!fn (&servinfo))) 
	    {				/* send error if not handled.	*/
		m->mcb.MsgHdr.FnRc = Err_Null;
		ErrorMsg (&m->mcb, EC_Error + info->SubSys + EG_FnCode);
	    }
	}
	else 				/* GSP function :		*/
	     (*e->Fn) (&servinfo);	/* call function handler.	*/
    }

done:					/* Request finished :		*/
    UnLockTarget (&servinfo);		/* release target		*/
    Free (m);				/* and free request MsgBuf.	*/
}


/*************************************************************************
 * ABORTABLE SERVER DISPATCHER
 *
 * - The dispatcher terminates upon reception of a CloseObj message.
 *
 * Parameter  :	info	= DispatchInfo
 * Return     :	- nothing -
 *
 ************************************************************************/

void 
Dispatcher (DispatchInfo *info) 
{
    MsgBuf	*m = NULL;
    word        fn;
    DispatchEntry *e;

    forever
    {
	m = ServMalloc (sizeof (MsgBuf));	/* Allocate a buffer.	*/

	if (m == Null (MsgBuf)) 
	{
	    Delay (OneSec * 5); 
	    continue; 
	}

	m->mcb.MsgHdr.Dest  = info->ReqPort;	/* Initialise the MCB.	*/
	m->mcb.Timeout	    = OneSec * 30;
	m->mcb.Control	    = m->control;
	m->mcb.Data	    = m->data;

lab1:

/* if we have no safety net, and the largest free block is more than	*/
/* twice the safety net size, get the safety net.			*/
	if (SafetyNet == NULL && SafetySize > 0 &&
	  (int) Malloc (-2) > 2 * SafetySize)
	    SafetyNet = Malloc (SafetySize);

	fn = GetMsg (&m->mcb);		/* Get the next Message.	*/

	if (fn == EK_Timeout) 		/* Timeout : try once more,	*/
	    goto lab1;

	if (fn < 0) 			/* any other Error aborts	*/
	    break;			/* the Dispatcher.		*/

	fn = m->mcb.MsgHdr.FnRc & FG_Mask;

	if (fn == FG_Terminate)			/* FG_Terminate ?	*/
	    break;
	elif (fn < FG_Open || fn > FG_CloseObj)	/* Private message ?	*/
	    e = &info->PrivateProtocol;
	else 					/* GSP Server Request.	*/
	    e = &info->Fntab[(fn - FG_Open) >> FG_Shift];

	unless (Fork (e->StackSize, Worker, 12, m, info, e)) 
	{				/* failed to start Worker.	*/
	    Free (SafetyNet); SafetyNet = NULL;
	    ErrorMsg (&m->mcb, EC_Error + info->SubSys + EG_NoMemory);
	    goto lab1;
	}
    }
    if (m != NULL) Free (m);
}

/*--- end of dispatch.c ---*/

