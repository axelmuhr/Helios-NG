/*------------------------------------------------------------------------
--                                                                      --
--           H E L I O S   N E T W O R K I N G   S O F T W A R E	--
--           ---------------------------------------------------	--
--                                                                      --
--             Copyright (C) 1990, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- netutils : qrtns							--
--									--
--	Author:  BLV 1/10/91						--
--                                                                      --
------------------------------------------------------------------------*/
/* RcsId: $Header: /hsrc/network/RCS/nuqrtns.c,v 1.3 1993/08/12 11:32:34 nickc Exp $*/

#include <message.h>
#include <codes.h>
#include <queue.h>
#include <sem.h>
#include <syslib.h>
#include <root.h>
#include <nonansi.h>

/**
*** The qRoutines. These routines are equivalent to system library routines
*** such as Locate(), ServerInfo(), and so on. However the routines are
*** timed out and guaranteed to return within 2 seconds, rather than the
*** more typical 20 seconds plus of the system library. This is achieved
*** using separate threads.
**/

typedef	struct	qRoutine {
	Node		Node;
	Port		Port;
	WORD		FnRc;
	WORD		Arg1;
	WORD		Arg2;
	WORD		Arg3;
	WORD		Result;
	bool		Aborted;
	bool		Finished;
	Semaphore	WaitForClient;
	Semaphore	Exclusive;
} qRoutine;

static	List		qRoutineList;
static	int		qRoutineCounter;
static	int		MaxqThreads;
static	int		qDelay;
static	Semaphore	qRoutineLock;

	/* This routine should be called when the program starts up	*/
	/* and simply sets up the qRoutine world.			*/
void Init_qRoutines(int threads, int delay)
{ InitList(&qRoutineList);
  InitSemaphore(&qRoutineLock, 1);
  qRoutineCounter	= 0;
  MaxqThreads		= threads;
  qDelay		= delay;
}

	/* Several different threads are spawned off with this routine	*/
	/* which actually performs the interaction with the IOC.	*/
	/* Usually the thread runs in a loop, handling one request	*/
	/* after another. When the system starts running low on these	*/
	/* threads a few more may be spawned off to handle single	*/
	/* requests.							*/
static	void	qMain(bool one_request_only)
{ qRoutine	this_thread;

if (one_request_only) IOdebug("qMain : threads overflow");
  InitSemaphore(&this_thread.WaitForClient, 0);
  InitSemaphore(&this_thread.Exclusive, 1);
  if ((this_thread.Port = NewPort()) == NullPort) return;

  Wait(&qRoutineLock);
if (one_request_only) IOdebug("qMain : incrementing counter");
  qRoutineCounter++;
  Signal(&qRoutineLock);

  for ( ; !one_request_only; )
   { this_thread.Aborted = this_thread.Finished = FALSE;

	/* Put this thread on the list of waiting routines.		*/
     Wait(&qRoutineLock);
if (one_request_only) IOdebug("qMain, adding myself to the list");
     AddTail(&qRoutineList, &(this_thread.Node));
     Signal(&qRoutineLock);

	/* Wait for this thread to be allocated to a client.		*/
     Wait(&this_thread.WaitForClient);

	/* Perform the requested operation.				*/
     switch(this_thread.FnRc)
      { case	FG_Locate :
	  this_thread.Result = (word) Locate((Object *) this_thread.Arg1, (string) this_thread.Arg2);
	  break;

	case	FG_ServerInfo :
	  this_thread.Result = ServerInfo((Object *) this_thread.Arg1, (BYTE *) this_thread.Arg2);
	  break;

	default	:
	  IOdebug("monitor: qMain has been given an invalid FnRc %x", this_thread.FnRc);
	  return;
      }

	/* There are potential race conditions between the client	*/
	/* routine and this worker. These can be avoided using a	*/
	/* semaphore.							*/
    Wait(&this_thread.Exclusive);

    this_thread.Finished	= TRUE;

    if (this_thread.Aborted)
     {	/* Something has gone wrong, the client has given up.	*/
	/* Time to clean up.					*/

       switch(this_thread.FnRc)
        { case FG_Locate :
		Close((Object *) this_thread.Result);
		break;

	  case FG_ServerInfo :
		break;
	}
       Signal(&this_thread.Exclusive);
       continue;
     }

	/* The routine has finished within the time constraints set	*/
	/* by the client. The client is re-activated, this thread waits	*/
	/* for the client to extract the result, and life goes on.	*/
    AbortPort(this_thread.Port, Err_Null);
    Signal(&this_thread.Exclusive);
    Wait(&this_thread.WaitForClient);
  }

  Wait(&qRoutineLock);
  qRoutineCounter--;
  Signal(&qRoutineLock);
}

	/* This thread does most of the work on the client side.	*/
	/* A suitable worker thread is identified and allocated. This	*/
	/* thread is given the required parameters to do the job.	*/
	/* Then a GetMsg() call is used to wait for the worker thread	*/
	/* or for a timeout. No message will actually be sent, and the	*/
	/* code could be replaced eventually by a timed semaphore.	*/
	/* Careful use of semaphores and flags is needed to avoid race	*/
	/* conditions when the timeout occurs at the same time as the	*/
	/* worker thread gets its result.				*/
static	int	qOp(int fnrc, int arg1, int arg2, int arg3)
{ qRoutine	*target_thread;
  MCB		m;
  int		result = 0;

again:
  Wait(&qRoutineLock);

	/* Check whether there are any threads ready to handle		*/
	/* this request.						*/
  if (EmptyList_(qRoutineList))
   { bool	only_once;

	/* There are no spare threads, hence another one is started.	*/
     if (qRoutineCounter <= MaxqThreads)
      only_once = FALSE;
     else
      only_once = TRUE;

     (void) Fork(1000, &qMain, sizeof(int), only_once);
     Signal(&qRoutineLock);
	/* sleep for a while, to let the thread settle down.		*/
     Delay(OneSec / 5);
     goto again;
   }

	/* When the code reaches here the qRoutineLock semaphore is	*/
	/* still locked and there is a suitable thread on the list.	*/
	/* This thread is removed and activated.			*/
  target_thread = (qRoutine *) RemHead(&qRoutineList);
  Signal(&qRoutineLock);
  target_thread->FnRc	= fnrc;
  target_thread->Arg1	= arg1;
  target_thread->Arg2	= arg2;
  target_thread->Arg3	= arg3;

  InitMCB(&m, 0, target_thread->Port, NullPort, 0);
  m.Timeout	= qDelay;

	/* Wake up the thread, and use GetMsg() to await a response.	*/
  Signal(&target_thread->WaitForClient);
  (void) GetMsg(&m);

	/* If the worker has succeeded then it will set the finished	*/
	/* flag. This flag is set iff the Exclusive semaphore is locked.*/
	/* If the worker has not yet finished then the abort flag is	*/
	/* set so that the worker can discard its results. Examination	*/
	/* and setting of these two flags is done only when the		*/
	/* Exclusive semaphore is locked, thus preventing race		*/
	/* conditions.							*/
  Wait(&target_thread->Exclusive);
  if (target_thread->Finished)
   { result = (int) target_thread->Result;
     Signal(&target_thread->WaitForClient);
   }
  else
   {
     target_thread->Aborted = TRUE;
     switch(fnrc)
      { case FG_Locate		: result = (int) Null(Object); break;
        case FG_ServerInfo	: result = (int)(EC_Error + EG_Timeout); break;
      }
   }
  Signal(&target_thread->Exclusive);

  return(result);
}

Object	*qLocate(Object *obj, string name)
{ return((Object *) qOp(FG_Locate, (int) obj, (int) name, 0));
}

int	qServerInfo(Object *obj, BYTE *buffer)
{ return(qOp(FG_ServerInfo, (int) obj, (int) buffer, 0));
}

