/*------------------------------------------------------------------------
--                                                                      --
--                      H E L I O S  C O M M A N D                      --
--                      --------------------------                      --
--                                                                      --
--             Copyright (C) 1988, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- showprocs.c								--
--                                                                      --
--	Although in theory processor-specific, if the implementation	--
--	of the scheduler on other processors is close enough to the	--
--	transputer, this should be usable.				--
--                                                                      --
-- Author: PAB 20/6/90							--
--                                                                      --
------------------------------------------------------------------------*/
/* SccsId: %W%\t%G% Copyright (C) 1987, Perihelion Software Ltd.*/
/* RcsId: $Id: showprocs.c,v 1.4 1991/11/13 14:38:33 paul Exp $	*/

typedef struct ProcessQ {
	SaveState *head;
	SaveState *tail;
} ProcessQ;

#if 0
#include "/hsrc/kernel/kernel.h" 
#else
#define P_TimerNext(x) ((x)->next)
#define P_RunqNext(x) ((x)->next)
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
#include <servlib.h>
#include <module.h>
#include <root.h>

/*---------------------------------------------------------------------------*/

#if 1
extern ProcessQ  *ReadyQBase(word pri) ;
extern SaveState **TimerQAddr(void);
extern void IntsOff(void);
extern void IntsOn(void);
#endif

void ShowQs(void);
void JacketDumpHPQs(void);
void DumpHPQs(void);
void DumpTQ(void);
void DumpRQ(void);
bool TaskName(word *dp, char *name);

/*---------------------------------------------------------------------------*/

word rate = OneSec; /* capture rate */
Semaphore capture;
Semaphore display;
int 	TQCaptureTime = 0;
int 	TQCaptureTime2 = 0;
int 	TQSize = 0;
int	RQSize = 0;


struct TQ {
	word	pc;
	word	dp;
	word	sp;
	word	wakeup;
	word	pri;
} TQStore[50];

struct RQ {
	word	pc;
	word	dp;
	word	sp;
	word	wakeup;
	word	pri;
} RQStore[2000];

/*---------------------------------------------------------------------------*/


int main ()
{
	InitSemaphore(&display,0);
	InitSemaphore(&capture,0);

#if 0
	System(JacketDumpHPQs);
#else
	JacketDumpHPQs();
#endif
	ShowQs();
}


/*---------------------------------------------------------------------------*/


void JacketDumpHPQs()
{
	Fork(10000, DumpHPQs, 0);
}

void DumpHPQs()
{
	forever {
		IntsOff();
			DumpTQ();
			DumpRQ();
		IntsOn();

		Signal(&display);
		Wait(&capture);
		Delay(rate);
	}
}

/*---------------------------------------------------------------------------*/

void DumpTQ(void)
{
	int i;
	SaveState *s = *TimerQAddr();

	/* round down to compensate for ~10/ms error */
/*	TQCaptureTime = (((unsigned) _cputime()) - 100) * 10000;*/

	TQCaptureTime = Timer();

	TQCaptureTime2 = _cputime() * 10000;
	
	for (i=0; s != NULL; i++, s = P_TimerNext(s)) {
		TQStore[i].pc = s->pc;
		TQStore[i].dp = s->dp;
		TQStore[i].sp = s->sp;
		TQStore[i].wakeup = s->endtime;
		TQStore[i].pri = s->pri;
	}

	TQSize = i;
}


void DumpRQ(void)
{
	int i=0;
	int j;
	SaveState *s;

	for (j = 0; j <= GetPhysPriRange(); j++)
	{
		ProcessQ *runq = ReadyQBase(j);
		s = runq->head;
	
		while (s != NULL)
		{
			RQStore[i].pc = s->pc;
			RQStore[i].dp = s->dp;
			RQStore[i].sp = s->sp;
			RQStore[i].pri = s->pri;

			i++;
			s = P_RunqNext(s);
		}
	}

	RQSize = i;
}

/*---------------------------------------------------------------------------*/


void ShowQs()
{
	int i;
	uword wakeup, now;
	char taskname[NameMax];
	
	printf("TQSize = %d\n",TQSize);

	forever {
		Wait(&display);
		
		now = TQCaptureTime;
                wakeup = 0; /* I do not know the real top of Q wakeup time ! */

		printf("\n\nTimerQ: Capture uS time %8lx, _cputime %8lx\n",now, TQCaptureTime2);

		for(i=0; i < TQSize; i++)
		{
			wakeup += TQStore[i].wakeup;
			printf("pri %1lx, pc %8lx, dp %8lx, sp %8lx, wakeup %8lx (%d csecs) ",
				TQStore[i].pri, TQStore[i].pc, TQStore[i].dp,
				TQStore[i].sp, TQStore[i].wakeup, wakeup/(OneSec/100));

			fflush(stdout);
			TaskName((word *)(TQStore[i].dp),taskname);
			printf("%s\n",taskname);
		}


		printf("\n\nRunQs: Runable = %d\n",RQSize);

		for(i=0; i<RQSize; i++)
		{
			printf("pri %1lx, pc %8lx, dp %8lx, sp %8lx ",
				RQStore[i].pri, RQStore[i].pc, RQStore[i].dp,
				RQStore[i].sp);

			fflush(stdout);
			TaskName((word *)(RQStore[i].dp),taskname);
			printf("%s\n",taskname);
		}

		Signal(&capture);
	}
}


/*---------------------------------------------------------------------------*/

/* get validated taskname from dp reg. */
bool TaskName(word *dp, char *name)
{
	word *km;
	int i;
	ObjNode *objnode;
	Task *tp;
	char *tn;
	Pool *syspool, *freepool;

	syspool = &(GetRoot()->SysPool);
	freepool = GetRoot()->FreePool;

	if (InPool((void *)dp,syspool)) {
		strcpy(name,"System");
		return false;
	}
	if(InPool((void *)dp,freepool)) {
		strcpy(name,"Error: dp in FreePool");
		return false;
	}

	if (*dp != (word) dp) { /* check its a module table pointer */
		strcpy(name,"No Module Table");
		return false;
	}

#if __SMT /* Split module table  version */
	km = (word *) *(dp + 2); /* kernel module */

	tp = (Task *) *(km); /* data entry 0 points at task struct */
#else
	km = (word *) *(dp + 1); /* kernel module */

	tp = (Task *) *(km + 48); /* entry 48 points at task struct */
#endif

	/* get first element of taskentry */
	objnode = (ObjNode *) tp->TaskEntry;

	tn = objnode->Name;

	for (i =0 ; i<NameMax; i++)
	{
		if (tn[i] == '\0') {
			strcpy(name, tn);
			return true;
		}
		if (!isprint(tn[i])) {
			strcpy(name, "Error: Corrupt task name");
			return false;
		}
	}

	strcpy(name, "Error: Invalid task Name");
	return false;
}
