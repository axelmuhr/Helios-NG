/*{{{  Header */

/*------------------------------------------------------------------------
--                                                                      --
--                      H E L I O S  C O M M A N D                      --
--                      --------------------------                      --
--                                                                      --
--             Copyright (C) 1993 - 1994, Perihelion Software Ltd.      --
--                        All Rights Reserved.                          --
--                                                                      --
-- tps.c								--
--                                                                      --
-- Print information about the current set of threads in the system.	--
-- Does not use any floating point code					--
--                                                                      --
--                                                                      --
-- This command will only work for processors with the generic		--
-- executive.								--
--                                                                      --
-- Author: PAB 23/4/92							--
--                                                                      --
------------------------------------------------------------------------*/
/* RcsId: $Id: threadps.c,v 1.12 1994/03/16 10:33:11 nickc Exp $ */

/*}}}*/
/*{{{  Includes */

#include "kernel.h"	/* for Id */

#ifdef NEW_SYSTEM
#include "gexec.h"	/* for GetExecRoot() */
#endif

#include <message.h>
#include <root.h>
#include <task.h>
#include <cpustate.h>
#include <process.h>
#include <stdlib.h>
#include <stdio.h>
#include <syslib.h>
#include <nonansi.h>
#include <string.h>
#include <ctype.h>
#include <syslib.h>
#include <module.h>
#include <root.h>
#include <event.h>
#include <time.h>
#include <ctype.h>

/*}}}*/
/*{{{  Types */

/* *Warning*: STOLEN from servlib.h due to incompatible headers: */
/* (kernel.h vs servlib.h)*/

typedef struct ObjNode
  {
	Node		Node;		/* link in directory list	*/
	char		Name[NameMax];	/* entry name			*/
	word		Type;		/* entry type			*/
	word		Flags;		/* flag word			*/
	Matrix		Matrix;		/* access matrix		*/
	Semaphore	Lock;		/* locking semaphore		*/
	Key		Key;		/* protection key		*/
	struct DirNode  *Parent;	/* parent directory		*/
	DateSet		Dates;		/* dates of object		*/
	word		Account;	/* owning account		*/
	word		Size;		/* object size			*/
	List		Contents;	/* whatever this object contains*/
					/* may be cast to something else*/
      }
ObjNode ;


typedef struct ThreadInfo
  {
    word	ss;
    word	pc;
    word	mt;
    word	sp;
    word	wakeup;
    word	status;
    word	pri;
    word	cputotal;
    word	timestart;
  }
ThreadInfo;

/*}}}*/
/*{{{  Constants */

#define TQSTORESIZE	100
#define	RQSTORESIZE	1000
#define	KTQSTORESIZE	1000
#define FnNameMax	16	/* max length of fn name allowed */

/*}}}*/
/*{{{  Macros */

#define HeadNode_( type, pList, field )\
  (type *)((char *)((pList)->Head) - offsetof( type, field ))
    
#define NextNode_( type, pStruct, field )\
  (type *)((char *)((pStruct)->field.Next) - offsetof( type, field ))
    

/*}}}*/
/*{{{  Variables */

#ifdef NEW_SYSTEM
ExecInfo	sExecInfo;
#endif

Semaphore capture;
Semaphore display;

word	rate 		= OneSec; /* capture rate */
word	repeat		= FALSE;
word	verbose		= FALSE;
word	all		= TRUE;
word	TQCaptureTime	= 0;
word	TQCaptureTime2	= 0;
word	TQSize		= 0;
word	RQSize		= 0;
#ifdef NEW_SYSTEM
word	ResumeQSize	= 0;
word	HighestAvailPri = 8;
#endif
word	KTQSize		= 0;


ThreadInfo TQStore[  TQSTORESIZE ];
ThreadInfo RQStore[  RQSTORESIZE ];
#ifdef NEW_SYSTEM
ThreadInfo ResumeQStore[  RQSTORESIZE ];
#endif
ThreadInfo KTQStore[ KTQSTORESIZE ];

/*}}}*/
/*{{{  Code */

/*{{{  StatusText() */

#ifdef NEW_SYSTEM
/*{{{  StatusText() */

const char *
StatusText( word iStatus )
{
  static char 	aStatus[ 256 ];		/* XXX arbitary size for array */


  aStatus[ 0 ] = '\0';
  
  while (iStatus)			/* loop on status bits */
    {
      word	iBit;
      

      iBit = iStatus & (-iStatus); 	/* get least significant bit in status */

      iStatus &= ~iBit;			/* remove this bit from status */

      switch (iBit)			/* add text based on this bit */
	{
	case THREAD_SLICED:	strcat( aStatus, "Sliced" ); break;
	case THREAD_RUNNABLE:	strcat( aStatus, "Runnable" ); break;
	case THREAD_RUNNING:	strcat( aStatus, "Running" ); break;
	case THREAD_SLEEP:	strcat( aStatus, "Sleeping" ); break;
	case THREAD_SEMAPHORE:	strcat( aStatus, "Waiting on Semaphore" ); break;
	case THREAD_MSGREAD:	strcat( aStatus, "Waiting in GetMsg" ); break;
	case THREAD_MSGWRITE:	strcat( aStatus, "Waiting in PutMsg" ); break;
	case THREAD_MULTIWAIT:	strcat( aStatus, "MultiWaiting" ); break;
	case THREAD_MUTEX:	strcat( aStatus, "Waiting on Mutex" ); break;
	case THREAD_GRABMUTEX:	strcat( aStatus, "Waiting to Grab Mutexes" ); break;
	case THREAD_SAVED:	strcat( aStatus, "Saved" ); break;
	case THREAD_RESUMED:	strcat( aStatus, "Resumed" ); break;
	case THREAD_KILLED:	strcat( aStatus, "Killed" ); break;
	case THREAD_LINKWRITEQ:	strcat( aStatus, "Link Write Q" ); break;
	case THREAD_LINKREADQ:	strcat( aStatus, "Link Read Q" ); break;
	case THREAD_LINKRX:	strcat( aStatus, "LinkRx" ); break;
	case THREAD_LINKTX:	strcat( aStatus, "LinkTx" ); break;
	case THREAD_LINKWAIT:	strcat( aStatus, "LinkWait" ); break;
	case THREAD_LINKXOFF:	strcat( aStatus, "LinkXOff" ); break;
	case THREAD_LINKBUF:	strcat( aStatus, "LinkBuf" ); break;
	case THREAD_REAPED:	strcat( aStatus, "Reaped" ); break;
	default:		sprintf( aStatus, "<unknown %lx %lx>", iBit, iStatus );
	  return aStatus;
	}
      
      if (iStatus)
 	strcat( aStatus, ", " ); 	/* if we are going to loop, then add a comma */
    }

  return aStatus;			/* return composite status string */
}

/*}}}*/
#else
/*{{{  threadstatus[] */

struct threadstatus
  {
    int	code;
    char	*description;
  }
threadstatus[] =
  {
      {  THREAD_STARTUP,	"new thread about to startup  "},
	{THREAD_SLICED,		"runnable, was sliced         "},
	{THREAD_RUNNABLE,	"runnable, rescheduled        "},
	{THREAD_RUNNING,	"current CPU thread           "},
	{THREAD_KILLED,		"thread has been Stop()'ed    "},
	{THREAD_BOGUS,		"illegal state of thread      "},
      /* THREAD_SLICED status in normal dispatch */
	{THREAD_SAVED,	 	"user SaveCPUState()          "},
	{THREAD_SLEEP,		"on timer Q                   "},
	{THREAD_TIMEDWAIT, 	"blocked on timer & semaphore "},
	{THREAD_SEMAPHORE,	"blocked on semaphore         "},
	{THREAD_MSGREAD	,	"blocked reading msg          "},
	{THREAD_MSGWRITE,	"blocked writing internal msg "},
	{THREAD_MULTIWAIT,	"blocked during MultiWait()   "},
	{THREAD_LINKRX,		"blocked reading link msg     "},
	{THREAD_LINKTX,		"blocked writing link msg     "},
	{THREAD_LINKWRITEQ,	"blocked on link msg write Q  "},
	{THREAD_LINKWAIT,	"guardian waiting on dumb link"},
	{THREAD_LINKEND,	"blocked waiting to use link  "},
				/* TaskKill/JumpLink */
	{THREAD_LINKXOFF,	"waiting for XON on link      "},
	{THREAD_LINKTHRU1,	"single buffer thru-routed msg"},
	{THREAD_LINKTHRU2,	"double buffer thru-routed msg"},
#ifdef __C40
	{THREAD_DMAREQ,		"waiting for a DMA engine     "}, /* unused */
#else
	{0,			"Unused"},
#endif
	{THREAD_MSGWRITE2,	"blocked writing internal msg "}
};

/*}}}*/
/*{{{  StatusText() */

char *StatusText(word i)
{
	/* should be sequential, but check */
	if(threadstatus[i].code != i)
		return "illegal status value";

	return threadstatus[i].description;
}

/*}}}*/
#endif

/*}}}*/
/*{{{  VerboseDumpTQ() */

void
VerboseDumpTQ( void )
{
  int 		i;

#ifdef NEW_SYSTEM
  Id * 		pId;


  WaitMutex( sExecInfo.TimerQLock );
  
  pId = Head_( Id, GetExecRoot()->TimerQ );
#else
  SaveState *	s;
  
  s = TimerQHead();
#endif
  
  TQCaptureTime = _cputime() * 10000;
  
  for (i = 0;
#ifdef NEW_SYSTEM
       !EndOfList_( pId ) && i < TQSTORESIZE;
       pId = Next_( Id, pId ), i++
#else
       s != NULL && i < TQSTORESIZE;
       i++, s = P_TimerNext( s )
#endif
       )
    {
#ifdef NEW_SYSTEM
      SaveState *	s = pId->state;
#endif

      
      TQStore[i].ss        = (word) CtoM_( s );
#ifdef __C40
      TQStore[i].pc        = s->CPUcontext.PC;
      TQStore[i].sp        = s->CPUcontext.R_USP;
#else
      TQStore[i].pc        = s->CPUcontext.R_PC;
#ifdef __ARM6
      TQStore[i].sp        = s->CPUcontext.R_USR_SP;
#else
      TQStore[i].sp        = s->CPUcontext.R_USER_SP;
#endif
#endif
      TQStore[i].mt        = s->CPUcontext.R_MT;
#ifdef NEW_SYSTEM	  
      TQStore[i].wakeup    = pId->endtime;
#else
      TQStore[i].wakeup    = s->endtime;
#endif
      TQStore[i].pri       = s->priority;
      TQStore[i].cputotal  = s->CPUTimeTotal;
      TQStore[i].timestart = s->InitialTime;
    }
  
#ifdef NEW_SYSTEM
  SignalMutex( sExecInfo.TimerQLock );
#endif
  
  TQSize = i;
}

/*}}}*/
/*{{{  NormalDumpTQ() */

void
NormalDumpTQ( void )
{
  int 		i;
  
#ifdef NEW_SYSTEM
  Id * 		pId;


  WaitMutex( sExecInfo.TimerQLock );
  
  pId = Head_( Id, GetExecRoot()->TimerQ );
#else
  SaveState *	s;
  
  s = TimerQHead();
#endif
  
  TQCaptureTime = _cputime() * 10000;
  
  for (i = 0;
#ifdef NEW_SYSTEM
       !EndOfList_( pId )   && i < TQSTORESIZE;
       pId = Next_( Id, pId ), i++
#else
       s != NULL && i < TQSTORESIZE;
       i++, s = P_TimerNext( s )
#endif
	 )
    {
#ifdef NEW_SYSTEM	  
      TQStore[i].ss     = (word) CtoM_( pId->state );
      TQStore[i].mt     = pId->state->CPUcontext.R_MT;
      TQStore[i].wakeup = pId->endtime;
#else
      TQStore[i].ss     = (word) CtoM_( s );
      TQStore[i].mt     = s->CPUcontext.R_MT;
      TQStore[i].wakeup = s->endtime;
#endif
    }

#ifdef NEW_SYSTEM
  SignalMutex( sExecInfo.TimerQLock );
#endif
  
  TQSize = i;
}

/*}}}*/
/*{{{  VerboseDumpRQ() */

void
VerboseDumpRQ( word range )
{
  int 		i = 0;
  word		j;
  SaveState *	s;

  
  for (j = range; j--;)
    {
#ifdef NEW_SYSTEM
      s = Head_( SaveState, GetExecRoot()->Queues[ j ] );
#else
      s = ReadyQBase( j )->head;
#endif
      
      while (
#ifdef NEW_SYSTEM
	     !EndOfList_( s )
#else
	     s != NULL
#endif
	     )
	{
	  if (i >= RQSTORESIZE)
	    break;
	  RQStore[i].ss        = (word) CtoM_( s );
#ifdef __C40
	  RQStore[i].pc        = s->CPUcontext.PC;
	  RQStore[i].sp        = s->CPUcontext.R_USP;
#else
	  RQStore[i].pc        = s->CPUcontext.R_PC;
#ifdef __ARM6
	  RQStore[i].sp        = s->CPUcontext.R_USR_SP;
#else
	  RQStore[i].sp        = s->CPUcontext.R_USER_SP;
#endif
#endif
	  RQStore[i].mt        = s->CPUcontext.R_MT;
	  RQStore[i].pri       = j;
	  RQStore[i].status    = s->status;
	  RQStore[i].cputotal  = s->CPUTimeTotal;
	  RQStore[i].timestart = s->InitialTime;
	  
	  i++;
#ifdef NEW_SYSTEM
	  s = Next_( SaveState, s );
#else
	  s = P_RunqNext( s );
#endif
	}
    }
  
  RQSize = i;
}

/*}}}*/
/*{{{  NormalDumpRQ() */

void
NormalDumpRQ( word range )
{
  int 		i = 0;
  word		j;
  SaveState *	s;

  
  for (j = range; j--;)
    {
#ifdef NEW_SYSTEM
      s = Head_( SaveState, GetExecRoot()->Queues[ j ] );
#else
      s = ReadyQBase( j )->head;
#endif
      
      while (
#ifdef NEW_SYSTEM
	     !EndOfList_( s )
#else
	     s != NULL
#endif
	     )
	{
	  if (i >= RQSTORESIZE)
	    break;
	  
	  RQStore[i].mt        = s->CPUcontext.R_MT;
	  RQStore[i].status    = s->status;
#ifdef __C40
	  RQStore[i].ss        = C40WordAddress( s );
#else
	  RQStore[i].ss        = (word)s;
#endif
	  i++;

#ifdef NEW_SYSTEM
	  s = Next_( SaveState, s );
#else
	  s = P_RunqNext( s );
#endif
	}
    }
  
  RQSize = i;
}

/*}}}*/
#ifdef NEW_SYSTEM
/*{{{  VerboseDumpResumeQ() */

void
VerboseDumpResumeQ( word range )
{
  int 		i = 0;
  word		j;
  SaveState *	s;

  HighestAvailPri = GetExecRoot()->HighestAvailPri;
  
  for (j = range; j--;)
    {
      s = Head_( SaveState, GetExecRoot()->ResumeQueues[ j ] );
      
      while (!EndOfList_( s ))
	{
	  if (i >= RQSTORESIZE)
	    break;
	  
	  ResumeQStore[i].ss        = (word) CtoM_( s );
#ifdef __C40
	  ResumeQStore[i].pc        = s->CPUcontext.PC;
	  ResumeQStore[i].sp        = s->CPUcontext.R_USP;
#else
	  ResumeQStore[i].pc        = s->CPUcontext.R_PC;
#ifdef __ARM6
	  ResumeQStore[i].sp        = s->CPUcontext.R_USR_SP;
#else
	  ResumeQStore[i].sp        = s->CPUcontext.R_USER_SP;
#endif
#endif
	  ResumeQStore[i].mt        = s->CPUcontext.R_MT;
	  ResumeQStore[i].pri       = j;
	  ResumeQStore[i].status    = s->status;
	  ResumeQStore[i].cputotal  = s->CPUTimeTotal;
	  ResumeQStore[i].timestart = s->InitialTime;
	  
	  i++;

	  s = Next_( SaveState, s );
	}
    }
  
  ResumeQSize = i;
}

/*}}}*/
/*{{{  NormalDumpResumeQ() */

void
NormalDumpResumeQ( word range )
{
  int 		i = 0;
  word		j;
  SaveState *	s;


  for (j = range; j--;)
    {
      s = Head_( SaveState, GetExecRoot()->ResumeQueues[ j ] );
      
      while (!EndOfList_( s ))
	{
	  if (i >= RQSTORESIZE)
	    break;
	  
	  ResumeQStore[i].mt        = s->CPUcontext.R_MT;
	  ResumeQStore[i].status    = s->status;
#ifdef __C40
	  ResumeQStore[i].ss        = C40WordAddress( s );
#else
	  ResumeQStore[i].ss        = (word)s;
#endif
	  i++;

	  s = Next_( SaveState, s );
	}
    }
  
  ResumeQSize = i;
}

/*}}}*/
#endif
/*{{{  VerboseDumpKTQ() */

/* Dump all known threads and their status from known thread Q */
void
VerboseDumpKTQ( void )
{
  SaveState * s; 
  int i;

#ifdef NEW_SYSTEM
  WaitMutex( sExecInfo.KnownThreadsLock );

  s = HeadNode_( SaveState, sExecInfo.KnownThreads, threadlist );
#else
  s = GetExecRoot()->KnownThreads;
#endif

  for (i = 0;
#ifdef NEW_SYSTEM
       ! EndOfList_( s ) && i < KTQSTORESIZE;
       s = NextNode_( SaveState, s, threadlist ), i++
#else
       s != NULL && i < KTQSTORESIZE;
       i++, s = s->nextknown
#endif
       )
    {
      KTQStore[i].ss        = (word) CtoM_( s );
#ifdef __C40
      KTQStore[i].pc        = s->CPUcontext.R_LR;
      KTQStore[i].sp        = s->CPUcontext.R_USP;
#else
#ifdef __ARM6
      KTQStore[i].pc        = s->CPUcontext.R_USR_LR;
      KTQStore[i].sp        = s->CPUcontext.R_USR_SP;
#else
      KTQStore[i].pc        = s->CPUcontext.R_USER_LR;
      KTQStore[i].sp        = s->CPUcontext.R_USER_SP;
#endif
#endif
      KTQStore[i].mt        = s->CPUcontext.R_MT;
      KTQStore[i].pri       = s->priority;
      KTQStore[i].status    = s->status;
      KTQStore[i].cputotal  = s->CPUTimeTotal;
      KTQStore[i].timestart = s->InitialTime;
    }
  
  KTQSize = i;

#ifdef NEW_SYSTEM
  SignalMutex( sExecInfo.KnownThreadsLock );
#endif
  
}

/*}}}*/
/*{{{  NormalDumpKTQ() */

void
NormalDumpKTQ( void )
{
  SaveState *	pThread; 
  int		i = 0;


#ifdef NEW_SYSTEM
  WaitMutex( sExecInfo.KnownThreadsLock );
#endif

  for (
#ifdef NEW_SYSTEM
                           pThread = HeadNode_( SaveState, sExecInfo.KnownThreads, threadlist );
       i < KTQSTORESIZE && !EndOfList_( &pThread->threadlist );
       i++,                pThread = NextNode_( SaveState, pThread, threadlist )
#else
                           pThread = GetExecRoot()->KnownThreads;
       i < KTQSTORESIZE && pThread != NULL;
       i++,                pThread = pThread->nextknown
#endif
       )
    {
      KTQStore[ i ].ss     = (word) CtoM_( pThread );
      KTQStore[ i ].mt     = pThread->CPUcontext.R_MT;
      KTQStore[ i ].status = pThread->status;
    }
  
#ifdef NEW_SYSTEM
  SignalMutex( sExecInfo.KnownThreadsLock );
#endif
  
  KTQSize = i;

  return;
}

/*}}}*/
/*{{{  TaskName() */

/* get validated taskname from mt reg. */
/* and pad it to FnNameMax chars */
bool
TaskName(
	 word *	mt,
	 char *	name )
{
  word *	km;
  int		i;
  ObjNode *	objnode;
  Task *	tp;
  char *	tn;
  Pool *	syspool;
  Pool *	freepool;

  
  if (mt == NULL)
    {
      strcpy( name, "<mt> is NULL!  " );
      return false;
    }
  
#ifdef __C40
  mt = (word *)C40CAddress((word)mt);
#endif
  
  syspool  = &(GetRoot()->SysPool);
  freepool = GetRoot()->FreePool;
  
  if (InPool( (void *)mt, syspool ))
    {
      strcpy(name, "System");
      strncat(name, "                ", FnNameMax - 6);
      return false;
    }
  
  if (InPool( (void *)mt, freepool) )
    {
      strcpy(name, "ModTab is Free ");
      return false;
    }
  
  if (*mt != (word) mt)
    {
      /* check its a module table pointer */
      strcpy(name, "Invalid ModTab  ");
      return false;
    }
  
#ifdef __SMT /* Split module table  version */
  km = (word *) *(mt + 2); /* kernel module */
  
  tp = (Task *) *(km); /* data entry 0 points at task struct */
#else
  km = (word *) *(mt + 1); /* kernel module */
  
  tp = (Task *) *(km + 48); /* entry 48 points at task struct */
#endif
  
  /* get first element of taskentry */
  objnode = (ObjNode *) tp->TaskEntry;
  
  tn = objnode->Name;
  
  for (i =0 ; i < NameMax; i++)
    {
      if (tn[i] == '\0')
	{
	  strcpy(name, tn);
	  strncat(name, "                ", FnNameMax-i);
	  return true;
	}
      
      if (!isprint(tn[i]))
	{
#if 1
	  /* tn[i]=' '; */
	  sprintf( name, "0x%*.*p", -(FnNameMax - 3), FnNameMax - 3, tn );
#else
	  strcpy(name, "Corrupt name    ");
	  return false;
#endif
	}
    }
  
  strcpy(name, "Invalid name");
  return false;
  
} /* TaskName */

/*}}}*/

# ifdef __ARM
/*{{{  EmbeddedName() */

char *
EmbeddedName( VoidFnPtr pFn )
{
  word w, i;
  word *  fn;

  
  fn = (word *)((word)pFn & ~0xfc000003);	/* remove any flags and word align */

  if (fn == NULL)
    return NULL;

  /* Search up to 10 words before the STM looking for */
  /* the marker that shows where the function name is. */
  
  for (i = 0; (i < 10); i++)
    {
      w = *--fn;

      if ((w & 0xffff0000) == 0xff000000)
	{
	  return (char *)fn - (w & 0xffff);
	}
    }
  
  return NULL;
}

/*}}}*/
#endif /* __ARM */

/*{{{  FnName() */

void
FnName(
       VoidFnPtr 	fn,
       const char *	name )
{
  char *	pName =
#if defined __ARM
	      EmbeddedName( fn )
#else
	      procname( fn )
#endif
    ;

  
#ifdef __ARM
  if (pName == NULL)
    pName = procname( fn );
#endif

  if (pName == NULL)
    {
      sprintf( (char *) name, "0x%*.*p", -(FnNameMax - 3), FnNameMax - 3, fn );
      return;
    }
  
  if (sprintf( (char *) name, "%*.*s", -(FnNameMax - 1), FnNameMax - 1, pName ))
    {
      int	i;
      for (i = strlen( name ); i--;)
	if (!isprint( name[ i ] ))
	  {
	    sprintf( (char *) name, "0x%*.*p", -(FnNameMax - 3), FnNameMax - 3, fn );
	    break;
	  }
    }
  else
    ((char *)name)[ 0 ] ='\0';

  return;
}

/*}}}*/
/*{{{  ShowQs() */

void
ShowQs( void )
{
  int		i;
  uword		wakeup;
#ifdef NEW_SYSTEM
  const char *	status;
#else
  char *	status;
#endif
  char		taskname[ NameMax ];
  char		threadname[ FnNameMax ];

  
  do
    {
      Wait( &display );
      
      wakeup = 0; /* I do not know the real top of Q wakeup time ! */
      
/*{{{  Timer Q */

      printf("\fTimer Queue:\n");
      
      if (verbose)
	{
	  printf("Number of sleeping threads: %ld\n",TQSize);
	  printf("Timer value at capture time: %#8lx\n", TQCaptureTime);
	}
      
      for (i = 0; i < TQSize; i++)
	{
	  TaskName((word *)(TQStore[i].mt), taskname);
	  
	  if (!all)
	    {
	      /*
	       * @@@ Should also knock out signal handler processes
	       * but this is difficult without any names.
	       */
	      
	      if (strncmp( taskname, "ProcMan.0", 9 ) == 0 ||
		  strncmp( taskname, "System", 6 )    == 0  )
		continue;
	    }			
#ifdef __C40
	  FnName(((SaveState *)C40CAddress(TQStore[i].ss))->InitialFn, threadname);
#else
	  FnName(((SaveState *)TQStore[i].ss)->InitialFn, threadname);
#endif
	  if (verbose)
	    {
	      printf( " %s: %s\n",
		     taskname, threadname);
#if 0 /* fixed C fp vararg bug? */
	      printf("\t\twakeup @ %#08lxuS, wakeup in %8lx uS (%d secs)\n",
		     TQStore[i].wakeup,
		     TQStore[i].wakeup - TQCaptureTime,
		     ((float)(TQStore[i].wakeup - TQCaptureTime))/OneSec);
#else
	      printf("\t\twakeup @ %#08lxuS, ",
		     TQStore[i].wakeup);
	      printf("wakeup in %8lx uS (%ld secs)\n",
		     TQStore[i].wakeup - TQCaptureTime,
		     (word)(TQStore[i].wakeup - TQCaptureTime) / OneSec);
#endif
	      printf("\t\trun time %8dS, CPU time consumed %ldS\n",
		     time(NULL) - (time_t)TQStore[i].timestart,
		     (TQStore[i].cputotal) / OneSec);
	      
	      printf("\t\tss %8lx, physpri %1lx, pc %8lx, mt %8lx, usp %8lx\n",
		     TQStore[i].ss, TQStore[i].pri,
		     TQStore[i].pc, TQStore[i].mt,
		     TQStore[i].sp);
	    }
	  else
	    {
	      printf( " %s: %s: wakeup in %ld secs\n",
		     taskname, threadname,
		     (word)(TQStore[i].wakeup - TQCaptureTime) / OneSec);
	    }
	}
      

/*}}}*/
/*{{{  Run Qs */

      printf("Run Queues:\n");
      
      if (verbose)
	printf("Number of runnable threads = %ld\n", RQSize);
      
      for (i=0; i < RQSize; i++)
	{
	  TaskName((word *)(RQStore[i].mt),taskname);
	  
	  if (!all)
	    {
	      if (strncmp( taskname, "ProcMan.0", 9 ) == 0 ||
		  strncmp( taskname, "System", 6 )    == 0  )
		continue;
	    }			
#ifdef __C40
	  FnName(((SaveState *)C40CAddress(RQStore[i].ss))->InitialFn, threadname);
#else
	  FnName(((SaveState *)RQStore[i].ss)->InitialFn, threadname);
#endif
	  status = StatusText(RQStore[i].status);
	  
	  if (verbose)
	    {
	      printf( " %s: %s: %s\n", taskname, threadname, status);
	      
	      printf("\t\trun time %8dS, CPU time consumed %ldS\n",
		     time(NULL) - (time_t)RQStore[i].timestart,
		     RQStore[i].cputotal / OneSec);
	      
	      printf("\t\tss %8lx, physpri %1lx, pc %8lx, mt %8lx, usp %8lx\n",
		     RQStore[i].ss, RQStore[i].pri,
		     RQStore[i].pc, RQStore[i].mt,
		     RQStore[i].sp);
	    } else
	      printf( " %s: %s: %s\n", taskname, threadname, status);
	}
      

/*}}}*/
#ifdef NEW_SYSTEM
/*{{{  Resume Qs */

      printf("Resume Queues:\n");
      
      if (verbose)
	printf("Number of resumable threads = %ld, HighestAvailPri = %ld\n",
	       ResumeQSize, HighestAvailPri );
      
      for (i=0; i < ResumeQSize; i++)
	{
	  TaskName((word *)(ResumeQStore[i].mt),taskname);
	  
	  if (!all)
	    {
	      if (strncmp( taskname, "ProcMan.0", 9 ) == 0 ||
		  strncmp( taskname, "System", 6 )    == 0  )
		continue;
	    }			
#ifdef __C40
	  FnName(((SaveState *)C40CAddress(ResumeQStore[i].ss))->InitialFn, threadname);
#else
	  FnName(((SaveState *)ResumeQStore[i].ss)->InitialFn, threadname);
#endif
	  status = StatusText(ResumeQStore[i].status);
	  
	  if (verbose)
	    {
	      printf( " %s: %s: %s\n", taskname, threadname, status);
	      
	      printf("\t\trun time %8dS, CPU time consumed %ldS\n",
		     time(NULL) - (time_t)ResumeQStore[i].timestart,
		     ResumeQStore[i].cputotal / OneSec);
	      
	      printf("\t\tss %8lx, physpri %1lx, pc %8lx, mt %8lx, usp %8lx\n",
		     ResumeQStore[i].ss, ResumeQStore[i].pri,
		     ResumeQStore[i].pc, ResumeQStore[i].mt,
		     ResumeQStore[i].sp);
	    } else
	      printf( " %s: %s: %s\n", taskname, threadname, status);
	}
      

/*}}}*/
#endif
/*{{{  Known Threads */

      printf("All threads:\n");
      
      if (verbose)
	{
	  printf("Total number of threads: %ld\n", KTQSize);
	}
      
      /* print known thread Q contents */
      
      for (i=0; i < KTQSize; i++)
	{
	  TaskName((word *)(KTQStore[i].mt),taskname);
	  
	  if (!all)
	    {
	      if (strncmp( taskname, "ProcMan.0", 9 ) == 0 ||
		  strncmp( taskname, "System", 6 )    == 0  )
		continue;
	    }			
#ifdef __C40
	  FnName(((SaveState *)C40CAddress(KTQStore[i].ss))->InitialFn, threadname);
#else
	  FnName(((SaveState *)KTQStore[i].ss)->InitialFn, threadname);
#endif
	  status = StatusText(KTQStore[i].status);
	  
	  printf( " %s: %s: %s\n", taskname, threadname, status);
	  
	  if (verbose)
	    {
	      printf("\t\trun time %8dS, CPU time consumed %ldS\n",
		     time(NULL) - (time_t)KTQStore[i].timestart,
		     KTQStore[i].cputotal / OneSec);
	      
	      printf("\t\tss %8lx, physpri %1lx, pc %8lx, mt %8lx, usp %8lx\n",
		     KTQStore[i].ss, KTQStore[i].pri,
		     KTQStore[i].pc, KTQStore[i].mt,
		     KTQStore[i].sp);
	    }
	}
      

/*}}}*/

      Signal( &capture );
    }
  while (repeat);
}

/*}}}*/
/*{{{  VerboseDumpQs() */

void
VerboseDumpQs( word range )
{
#ifdef NEW_SYSTEM
  VerboseDumpTQ();

  VerboseDumpKTQ();
  
  IntsOff();
  	VerboseDumpRQ( range );
  	VerboseDumpResumeQ( range );
  IntsOn();
#else
  ClockIntsOff();
  	VerboseDumpTQ();
  ClockIntsOn();
  
  ClockIntsOff();
  	VerboseDumpKTQ();
  ClockIntsOn();
  
  IntsOff();
  	VerboseDumpRQ( range );
  IntsOn();
#endif
}

/*}}}*/
/*{{{  NormalDumpQs() */

void
NormalDumpQs( word range )
{
#ifdef NEW_SYSTEM
  NormalDumpTQ();
  
  NormalDumpKTQ();
  
  IntsOff();
    NormalDumpRQ( range );
    NormalDumpResumeQ( range );
  IntsOn();
#else
  ClockIntsOff();
  	NormalDumpTQ();
  ClockIntsOn();
  
  ClockIntsOff();
  	NormalDumpKTQ();
  ClockIntsOn();
  
  IntsOff();
  	NormalDumpRQ( range );
  IntsOn();
#endif
}

/*}}}*/
/*{{{  DumpThread() */

void
DumpThread(void)
{
  word	range = GetPhysPriRange();

  
  if (verbose)
    {
      forever
	{
	  VerboseDumpQs( range );
	  
	  Signal( &display );
	  
	  Wait( &capture );
	  
	  Delay( rate );
	}
    }
  else
    {
      forever
	{
	  NormalDumpQs( range );
	  
	  Signal( &display );
	  
	  Wait( &capture );
	  
	  Delay( rate );
	}
    }
}

/*}}}*/
/*{{{  main () */

int
main (int argc, char **argv)
{
#ifdef NEW_SYSTEM
  GetExecInfo( &sExecInfo );
#endif
  
  InitSemaphore(&display,0);
  InitSemaphore(&capture,0);
  
  while (*(++argv) != NULL)
    {
      if (strcmp(*argv, "-r") == 0)
	repeat = TRUE;
      
      if (strcmp(*argv, "-l") == 0)
	verbose = TRUE;
      
      if (strcmp(*argv, "-s") == 0)	/* XXX - ought to be a '-a' option to enable full listing */
	all = FALSE;
      
      if (strncmp(*argv, "-h", 2) == 0)
	{
	  fprintf(stderr,"usage: threadps [-r] [-l] [-s] [-h]\n" );
	  fprintf(stderr,"       -r: repeat forever\n" );
	  fprintf(stderr,"       -l: long listing\n" );
	  fprintf(stderr,"       -s: ignore boring processes\n" );
	  fprintf(stderr,"       -h: display help message\n" );
	  exit(0);		    
	}		
    }

  Fork( 2000, DumpThread, 0 );
  
  ShowQs();

  return 0;
  
} /* main */

/*}}}*/

/*}}}*/

/* end of threadps.c */
