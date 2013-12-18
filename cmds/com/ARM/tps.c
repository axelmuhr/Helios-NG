/*------------------------------------------------------------------------
--                                                                      --
--                      H E L I O S  C O M M A N D                      --
--                      --------------------------                      --
--                                                                      --
--             Copyright (C) 1993, Perihelion Software Ltd.             --
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
/* RcsId: $Id: tps.c,v 1.2 1994/06/07 12:31:25 nickc Exp $ */

#include "../kernel/kernel.h" 

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

/*---------------------------------------------------------------------------*/

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
word	KTQSize		= 0;

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


ThreadInfo TQStore[100];
ThreadInfo RQStore[1000];
ThreadInfo KTQStore[1000];

#define FnNameMax	16	/* max length of fn name allowed */



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
#if NEW_SYSTEM
	{0, 			"unused                       "},
#else
	{THREAD_TIMEDWAIT, 	"blocked on timer & semaphore "},
#endif
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
#if NEW_SYSTEM
	,{THREAD_MUTEX,		"blocked waiting on mutex     "},
	{THREAD_MUTEX_MANIP,	"blocked manipulaitng mutex   "}
#endif
};


char *StatusText(word i)
{
	/* should be sequential, but check */
	if(threadstatus[i].code != i)
		return "illegal status value";

	return threadstatus[i].description;
}

/*---------------------------------------------------------------------------*/



/*---------------------------------------------------------------------------*/

void DumpTQ(void)
{
	int i;
	SaveState *s = TimerQHead();

	TQCaptureTime = _cputime() * 10000;

	for (i=0; s != NULL; i++, s = P_TimerNext(s)) {
#ifdef __C40
		TQStore[i].ss = C40WordAddress(s);
		TQStore[i].pc = s->CPUcontext.PC;
		TQStore[i].sp = s->CPUcontext.R_USP;
#else
		TQStore[i].ss = (word)s;
		TQStore[i].pc = s->CPUcontext.R_PC
#if defined __ARM && !defined __ARM6
		  & 0x03FFFFFC
#endif
		  ;
#ifdef __ARM6
		TQStore[i].sp = s->CPUcontext.R_SP;
#else
		TQStore[i].sp = s->CPUcontext.R_USER_SP;
#endif
#endif
		TQStore[i].mt = s->CPUcontext.R_MT;
		TQStore[i].wakeup = s->endtime;
		TQStore[i].pri = s->priority;
		TQStore[i].status = s->status;
		TQStore[i].cputotal = s->CPUTimeTotal;
		TQStore[i].timestart = s->InitialTime;
	}

	TQSize = i;
}


void DumpRQ(void)
{
	int i=0;
	int j;
	SaveState *s;

	IOdebug( "dumping run Qs" );
	
	for (j = 0; j <= GetPhysPriRange(); j++)
	  {
		s = ReadyQBase(j)->head;
	
		while (s != NULL) {
#ifdef __C40
			RQStore[i].ss = C40WordAddress(s);
			RQStore[i].pc = s->CPUcontext.PC;
			RQStore[i].sp = s->CPUcontext.R_USP;
#else
			RQStore[i].ss = (word)s;
			RQStore[i].pc = s->CPUcontext.R_PC
#if defined __ARM && !defined __ARM6
			  & 0x03FFFFFC
#endif
			  ;
#ifdef __ARM6
			RQStore[i].sp = s->CPUcontext.R_SP;
#else
			RQStore[i].sp = s->CPUcontext.R_USER_SP;
#endif
#endif
			RQStore[i].mt = s->CPUcontext.R_MT;
			RQStore[i].pri = j;
			RQStore[i].status = s->status;
			RQStore[i].cputotal = s->CPUTimeTotal;
			RQStore[i].timestart = s->InitialTime;

			i++;
			s = P_RunqNext(s);
		}
	}

	IOdebug( "got run Qs, i = %d", i );
	RQSize = i;
}


/* Dump all known threads and their status from known thread Q */
void DumpKTQ(void)
{
	SaveState *s = GetExecRoot()->KnownThreads;
	int i;

	for ( i=0; s != NULL; i++, s = s->nextknown) {
#ifdef __C40
		KTQStore[i].ss = C40WordAddress(s);
		KTQStore[i].pc = s->CPUcontext.R_LR;
		KTQStore[i].sp = s->CPUcontext.R_USP;
#else
		KTQStore[i].ss = (word)s;
# if defined __ARM && !defined __ARM6
		KTQStore[i].pc = s->CPUcontext.R_USER_LR & 0x03FFFFFC;
		KTQStore[i].sp = s->CPUcontext.R_USER_SP;
# else
		KTQStore[i].sp = s->CPUcontext.R_SP;
		KTQStore[i].pc = s->CPUcontext.R_LR;
# endif
#endif
		KTQStore[i].mt = s->CPUcontext.R_MT;
		KTQStore[i].pri = s->priority;
		KTQStore[i].status = s->status;
		KTQStore[i].cputotal = s->CPUTimeTotal;
		KTQStore[i].timestart = s->InitialTime;
	}

	KTQSize = i;
}


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


# ifdef __ARM
char *
EmbeddedName( VoidFnPtr pFn )
{
  word w, i;
  word *  fn;

  
  fn = (word *)((word)pFn & ~0xfc000003);	/* remove any flags and word align */

  if (fn == NULL)
    return NULL;

  if (fn > (word *)0x003d7000)
    {
      IOdebug( "EmbeddedName: fn = %x, pFn = %x, URG", fn, pFn );
      return NULL;
    }
    
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
#endif /* __ARM */

void
FnName(VoidFnPtr fn, char *name)
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
      sprintf( name, "0x%*.*p", -(FnNameMax - 3), FnNameMax - 3, fn );
      return;
    }
  
  if (sprintf( name, "%*.*s", -(FnNameMax - 1), FnNameMax - 1, pName ))
    {
      int	i;
      for (i = strlen( name ); i--;)
	if (!isprint( name[ i ] ))
	  {
	    sprintf( name, "0x%*.*p", -(FnNameMax - 3), FnNameMax - 3, fn );
	    break;
	  }
    }
  else
    name[ 0 ] ='\0';

  return;
}

void
ShowQs( void )
{
  int	i;
  uword	wakeup;
  char	*status;
  char	taskname[NameMax];
  char	threadname[FnNameMax];
  
  do
    {
      Wait(&display);
      
      wakeup = 0; /* I do not know the real top of Q wakeup time ! */
      
      printf("\fTimer Queue:\n");
      
      if (verbose)
	{
	  printf("Number of sleeping threads: %ld\n",TQSize);
	  printf("Timer value at capture time: %#8lx\n", TQCaptureTime);
	}
      
      for (i=0; i < TQSize; i++)
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
	      printf("%s: %s\n",
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
	      printf("%s: %s: wakeup in %ld secs\n",
		     taskname, threadname,
		     (word)(TQStore[i].wakeup - TQCaptureTime) / OneSec);
	    }
	}
      
      printf("Run Queues:\n");
      
      if (verbose)
	printf("Number of runnable threads = %ld\n", RQSize);

      IOdebug( "RQSize = %d", RQSize );

      for (i=0; i < RQSize; i++)
	{
	  TaskName((word *)(RQStore[i].mt),taskname);

	  IOdebug( "task = %s", taskname );
	  
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
#if ! NEW_SYSTEM
	  if (RQStore[i].status == THREAD_TIMEDWAIT)
	    status = StatusText(THREAD_RUNNABLE);
	  else
#endif
	    status = StatusText(RQStore[i].status);

	  IOdebug( "thread = %s", threadname );
	  
	  if (verbose)
	    {
	      printf("%s: %s: %s\n", taskname, threadname, status);
	      
	      printf("\t\trun time %8dS, CPU time consumed %ldS\n",
		     time(NULL) - (time_t)RQStore[i].timestart,
		     RQStore[i].cputotal / OneSec);
	      
	      printf("\t\tss %8lx, physpri %1lx, pc %8lx, mt %8lx, usp %8lx\n",
		     RQStore[i].ss, RQStore[i].pri,
		     RQStore[i].pc, RQStore[i].mt,
		     RQStore[i].sp);
	    } else
	      printf("%s: %s: %s\n", taskname, threadname, status);
	}
      
      printf("All threads:\n");

      IOdebug( "All Threads" );
      
      if (verbose)
	{
	  printf("Total number of threads: %ld\n", KTQSize);
	}
      
      /* print known thread Q contents */
      
      for (i=0; i < KTQSize; i++)
	{
	  TaskName((word *)(KTQStore[i].mt),taskname);

	  IOdebug( "task = %s", taskname );
	  
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

	  IOdebug( "thread = %s", threadname );
	  
	  printf("%s: %s: %s\n", taskname, threadname, status);
	  
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
      
      Signal(&capture);
    }
  while (repeat);
}


/*---------------------------------------------------------------------------*/


void
DumpQs(void)
{
 	ClockIntsOff();
		DumpTQ();
	ClockIntsOn();
 	ClockIntsOff();
		DumpKTQ();
	ClockIntsOn();
	IntsOff();
		DumpRQ();
	IntsOn();
}

void
DumpThread(void)
{
	forever
	  {
	    DumpQs();
	    Signal(&display);
	    Wait(&capture);
	    Delay(rate);
	  }
}

int
main (int argc, char **argv)
{
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

/* end of tps.c */
