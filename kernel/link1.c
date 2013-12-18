/*------------------------------------------------------------------------
--                                                                      --
--                      H E L I O S   K E R N E L                       --
--                      -------------------------                       --
--                                                                      --
--             Copyright (C) 1988, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- link.c								--
--                                                                      --
--	Link Guardian and related stuff.				--
--	This file is almost entirely transputer-specific.		--
--                                                                      --
--	Author:  NHG 8/8/88						--
--                                                                      --
------------------------------------------------------------------------*/
/* $Id: link1.c,v 1.46 1993/08/11 09:54:05 nickc Exp $ */


#define __in_link 1	/* flag that we are in this module */

#include "kernel.h"
#include <config.h>
#include <task.h>
#include <process.h>
#include <sem.h>

#ifdef LINKIO

#define SOFT_RESET		1

#ifdef __C40
#  define DONT_IDLE_WITH_IOPROC	1
#else
#  define DONT_IDLE_WITH_IOPROC	0
#endif

bool LinkMsg(LinkInfo *link);
void InitDoubleBufferProcess(LinkInfo *link);
		
void LinkGuardian(LinkInfo *link);
void IntelligentServer(LinkInfo *link);
bool GetInfo(LinkInfo *link);
void SendInfo(bool reply, LinkInfo *link);
void ReadReply(LinkInfo *link,word type);
void Probe(LinkInfo *link);
void ChangeState(LinkInfo *link, word newstate);
word _Configure(LinkConf newconf, bool sendrecon);
void _JumpLink(LinkInfo *link, Id *id);
#define JumpLink _JumpLink
#if PERFMON
# ifdef __TRAN
void HiPriMon(void);
void LowPriMon(void);
# else
void IdleMon(void);
# endif
#endif
#if defined(__ARM) && defined IDLEMON
void HiIdleMon(void);
void LowIdleMon(void);
#endif
static void _Terminate(void);
static void Reset(void);
void HandleXoff(LinkInfo *link);
void HandleXon(LinkInfo *link);
#if defined(__ARM) && defined(__SERIALLINK)
void ResetSerialLink(LinkInfo *link) ;
#endif

/*--------------------------------------------------------
-- LinkInit						--
--							--
-- initialise the link guardians.			--
--							--
--------------------------------------------------------*/

void LinkInit(Config *config, Task *procman )
{
	word i;
	LinkInfo *link;
	LinkInfo **links;
	LinkConf *lc;
	RootStruct *root = GetRoot();

	root->Incarnation = config->Incarnation;

	/* alloc Nlinks + 1 pointers + Nlinks * LinkInfo structs */
	links = (LinkInfo **)Allocate(config->NLinks*(sizeof(LinkInfo)
				+ sizeof(word)) + sizeof(word),
				root->FreePool,&root->SysPool);
	/* since we are just starting, we assume this allocate will not fail! */

	root->Links = links;

	/* link points to LinkConfig structs */	
	link = (LinkInfo *)(&links[config->NLinks+1]);
	
	lc = &config->LinkConf[0];

	for( i = 0; i < config->NLinks; i++,link++ )
	{
		links[i] = link;    /* init table of ptrs to LinkConf structs */

		*(word *)link = *(word *)&lc[i];

		link->TxFunction = link->RxFunction = NULL;

		/* @@@ BOGGLE surely some misktak */
		/* This was never set previously! */
		link->TxQueue = NULL;

#ifndef __TRAN
		/* no threads using link */
		link->TxThread = link->RxThread = NULL;
#endif
		link->Incarnation = 0;

		link->Sync = NULL;
				
		{
			Port port = _NewPort();
			PTE *pte = GetPTE(port,root);
			
			pte->Owner = (word)procman;

			link->LocalIOCPort = port;
		}
		
		{
			Port port = _NewPort();
			PTE *pte = GetPTE(port,root);
			
			port |= (Port_Flags_Tx|Port_Flags_Remote);
			
			link->RemoteIOCPort = port;

			pte->Type = T_Permanent;
			pte->Link = (byte)i;
			pte->Owner = 0;
		}

#if defined(__C40) || defined(__ARM)
		/* Block any message send until we have first Proto_Go. */
		InitSemaphore(&link->HalfDuplex, 0);
#endif

#ifndef __TRAN
		if (link->Mode == Link_Mode_Special)
		{
			LinkInitSpecial(link);
		}
		else
#endif
		{
#ifdef __TRAN
			link->TxChan = &(LinkVector[i]);
			link->RxChan = &(LinkVector[i+4]);
#else
# ifdef __C40

			link->RxChan = link->TxChan = (Channel)(0x100040 + (0x10L * i));
#  ifndef ALLOCDMA
			/* pre-allocate same numbered DMA engine to this link */
			/* calc WPTR for DMA control reg address */
			link->DMAEng = 0x001000a0 + (i * 16);
#  endif
# else
			/* Set Tx/Rx channel numbers in link structure that */
			/* will be passed to _/__LinkTx/Rx functions to */
			/* identify the comms hardware for this link. */
			InitLink2(link);
# endif
#endif
		}

		NewWorker(LinkGuardian,link);
		InitDoubleBufferProcess(link);
	}

	links[i] = NULL;	/* end of vector == NULL */

	root->LocalMsgs = 0;
	root->BufferedMsgs = 0;
	root->Latency = 0;
	root->MaxLatency = 0;
	root->LoadAverage = 0;
	
#if PERFMON
# ifdef __TRAN
	NewWorker(LowPriMon);
	NewWorker(HiPriMon);
# else
	NewWorker(IdleMon);
# endif
#endif
}

#if PERFMON
/* The LoadAverage is determined in two ways:
 *	The transputer uses a load average defined as the average number of
 *	microseconds each thread receives in the time period.
 *
 *	Other processors define their load average as a number between 0 and
 *	2000 (for yukky compatibility with the transputer). This number defines
 *	The amount of time NOT spent in idle.
 */

# ifdef __TRAN
void LowPriMon(void)
{
	RootStruct *root = GetRoot();

	/* switch to low priority */
#  if defined(__TRAN)
	runp_(ldlp_(0)|1); stopp_();
#  else
	/* Note That this is a bit useless for multiprocess level execs */ 
	SetPhysPri(LogToPhysPri(StandardPri));	/* change hipri process to stdpri */
#  endif	
	forever
	{
		word now, wakeup, descheduled;
		word perproc;
		word nprocs = 0;
		SaveState *p;
		struct { SaveState *head, *tail; } runq;

		/* wait for 1/10 second 			*/
#  ifdef __TRAN
		Sleep(OneSec/640);	/* 64 microsecond lowpri timer */
#  else
#   ifdef __C40	/* if sleep can be called at any priority */
		Sleep(OneSec/10);
#   else
		System((WordFnPtr)Sleep,OneSec/10);
#   endif
#  endif
		/* count the number of processes in the run queue 	*/
		/* this must take less than a timeslice period		*/
		/* (if it takes more something is seriously wrong!)	*/
		/* as long as we are not timesliced, the only thing that*/
		/* can alter the runq is for hipri processes to add a	*/
		/* process to the end.					*/
#  ifdef __TRAN
		RunqPtrs(&runq);
		p = runq.head;

		if( !NullStateP(p) )
		{
			SaveState *pp;
			again:
				nprocs++;
				pp = p;
				p = P_RunqNext(p);
				if( pp != runq.tail ) goto again;

/*			nprocs++;		/ * and count tail */
		}
#  else
		/* @@@ just level StdPri for the moment - FIX! */
		RunqPtrs((SaveState **)&runq, LogToPhysPri(StandardPri));
		p = runq.head;

		/* No need to disable all interrupts as threads can only be */
		/* appended to Q and only after their Next has been NULL'ed */

		ClockIntsOff(); /* do not slice us */
			while ( !NullStateP(p) ) {
				nprocs++;
				p = P_RunqNext(p);
			}
		ClockIntsOn();
#  endif
		now = Timer();

#  ifdef __TRAN
		runp_(ldlp_(0)|1); stopp_();	/* go to end of runq	*/
#  elif defined(__ARM) || defined(__C40)
		/* use Yield if it will work for low pri processes */
		/* - Sleep will not be as accurate */
		Yield(); /* place our proc on end of runq */
#  else
		System((WordFnPtr)Sleep,0); /* Same effect */
#  endif
		wakeup = Timer();

#  ifdef __TRAN
		descheduled = DiffTimes(wakeup,now)*64;	/* time spent on queue	 */
#  else
		descheduled = DiffTimes(wakeup,now);	/* time spent on queue	 */
#  endif

		if( nprocs == 0 ) perproc = 0;
		else perproc = descheduled/nprocs; /* average time per proc */	
	
		/* Migrate average towards perproc by 1/30 difference	*/
		/* This smooths out variations over a 3 second period	*/
		
		root->LoadAverage += (perproc-root->LoadAverage)/30;

#  if defined(__ARM) || defined(__C40)
		/* remember number of processes on Q */
		root->StdPriProcs = nprocs;
#  endif
	}
}

void HiPriMon(void)
{
	RootStruct *root = GetRoot();

#  if defined(__TRAN) && defined(SYSDEB)
	word tv = (word)(root->TraceVec);
#  endif
	forever
	{
		word wakeup;
		word actual;
		word latency;

		wakeup = AddTimes((OneSec/10),Timer());	/* every 1/10 second */
		
#  ifdef __TRAN
		tin_(wakeup);
#  else
		Sleep(OneSec/10);
#  endif
		
		actual = Timer();
		
		latency = DiffTimes(actual,wakeup);	/* time on runq	*/
		
		/* migrate average towards real latency by 1/10 difference */
		root->Latency += (latency-root->Latency)/10;
		if( latency > root->MaxLatency ) root->MaxLatency = latency;
#  ifdef __TRAN
#   ifdef SYSDEB 
		{
			word *savew = (word *)0x8000002c;
			word *savei = (word *)0x80000030;
			
			if( *savew != (word)0x80000001 )
			{
				if( (*savew > tv) || (*savei > tv) )
				{
					_Trace(0xbb0000bb,*savew,*savei);
					for(;;);
				}
			}
		}
#   endif
#  endif
	}
}

# else /* generic executive supported version */

			/* (@@@ really want to reduce this time) sample  period */
#  define TESTPERIOD	(OneSec/10)

			/* range of varience (0-2000 for transputer compat.) */
#  define TESTRANGE	(TESTPERIOD / 2000)

void IdleMon(void)
{
	RootStruct *root = GetRoot();
	ExecRoot *xroot = GetExecRoot();

	forever	{
		Sleep(TESTPERIOD);

		/* Migrate average towards true inverse idle rating by 1/30 */
		/* difference. This smooths out variations over 3 seconds. */
		
		root->LoadAverage += (((TESTPERIOD - xroot->IdleTime) / TESTRANGE) - root->LoadAverage) / 30;
		xroot->IdleTime = 0;
	}
}
# endif
#endif /* PERFMON */


/*--------------------------------------------------------
-- LinkGuardian						--
--							--
-- Link Guardian main procedure.			--
--							--
--------------------------------------------------------*/

void LinkGuardian(LinkInfo *link)
{
	forever
	{
#if defined(__ABC) && defined(__SERIALLINK)
	  	if (link->RxChan == 0) 
			ResetLinks();
		else
		  	ResetSerialLink(link);
#else
		AbortLinkRx(link);
		AbortLinkTx(link);	
#endif
		link->TxUser = NULL;
		link->RxUser = NULL;
		/* intelligent links are served by the Intelligent server */
		if( link->Mode == Link_Mode_Intelligent ) 	
		{
			IntelligentServer(link);
		}
		else
		{
			/* a dumb link simply waits for a wakeup on the	*/
			/* sync list.					*/
			Id id;
			id.next = NULL;
			id.tail = &id;
			link->Sync = &id;
			Suspend(&id.state, THREAD_LINKWAIT);
		}
	}
}


/*--------------------------------------------------------
-- IntelligentServer					--
--							--
-- Server for intelligent links, waits for messages to 	--
-- arrive and dispatches them to the relevant dest.	--
--							--
--------------------------------------------------------*/

void IntelligentServer(LinkInfo *link)
{
	Id l;
	word type;
	bool idle = FALSE;
	RootStruct *root = GetRoot();	

		/* A ten second timeout should be plenty between	*/
		/* Helios processors. I/O processors may be blocked	*/
		/* for fairly long periods of time.			*/
	if (link->Flags & Link_Flags_ioproc)
		link->Timeout = 30 * OneSec;
	else
		link->Timeout = 10 * OneSec;

		
	/* If this was our parent, probe it */
	if(	(link->Flags & Link_Flags_parent) != 0 && 
		(link->State == Link_State_Running) ) {
#if defined(__C40) /* || defined(__ARM) */
			/* Due to problems with half duplex links */
			/* dont send an initial info, to the ioproc */
			/* we don't require one anyway. */
			/* These flags are set by the config vector */
			if (link->Flags & Link_Flags_ioproc) {
				PTE *pte;

				pte = GetPTE(link->RemoteIOCPort,root);
				pte->TxId = (Id *)IOProcIOCPort;
			} else
				SendInfo(1,link);
#else
			SendInfo(1,link); 
#endif
	}

	while( link->Mode == Link_Mode_Intelligent )
	{
		l.rc = 0;
		l.endtime = AddTimes(link->Timeout,Timer());

		link->RxUser = &l;
		
		type = Proto_Null;

		LinkRx(sizeof(ProtoPrefix),link,&type);

		if( l.rc != 0 )
		{
			/* reception from link has timed out, if it is	*/
			/* already dead, ignore it. If we have already	*/
			/* been through here then treat it seriously.	*/

			if( link->State == Link_State_Dead ) continue;

#ifdef FLOWCONTROL
			/* If we have blocked all incoming traffic or,	*/
			/* if the other processor has Xoffed, dont idle	*/
			/* handshake. This is a little unsatisfactory	*/
			/* since we would like to maintain the idle	*/
			/* handshake regardless of flow control. The	*/
			/* way things are organised at present makes	*/
			/* this a little difficult.			*/

			if( link->Flags & Link_Flags_stopped ||
			    root->Flags & Root_Flags_xoffed ) continue;
#endif

#if DONT_IDLE_WITH_IOPROC
			if( link->Flags & Link_Flags_ioproc ) continue;
#endif
			if( idle )
			{
				goto RxTimeout;			
			}

			idle = TRUE;
			NewWorker(Probe,link);

			continue;
		}

		l.endtime = AddTimes(link->Timeout,Timer());

		if( type == Proto_Msg ) {
			/* A message, call LinkMsg to deal with it 	*/
			idle = FALSE;
			/* BLV - a major change here. A message failure	*/
			/* may indicate a problem on a remote processor	*/
			/* rather than on this particular link, courtesy*/
			/* of single and double buffering. Hence a	*/
			/* failure gives no information about this link.*/
			/* A success does.				*/
			if (LinkMsg(link))
			 if( link->State != Link_State_Running )
				ChangeState(link,Link_State_Running);
			continue;
		}
		else switch( type )
		{
		/* A debug write, accept it but ignore it.		*/
		case Proto_Write:
			idle = FALSE;
			
			LinkRx(4,link,&type);
			if( l.rc != 0 ) goto RxTimeout;
	
			LinkRx(4,link,&type);
			if( l.rc != 0 ) goto RxTimeout;

			continue;
		
		/* Debug read, if this is the magic probe address,	*/
		/* return the inverse of the probe value.		*/
		case Proto_Read:
			idle = FALSE;

			LinkRx(4,link,&type);
			if( l.rc != 0 ) goto RxTimeout;

			if( type == Probe_Address ) {
				type = ~Probe_Value;
			} else {
				type = *(word *)type;
			}

			if( !NewWorker(ReadReply,link,type) )
				ReadReply(link,type);

			continue;

#ifdef __TRAN
		/* This is the result of a probe of a transputer in	*/
		/* reset or analysed state, it is the lsbyte of 	*/
		/* Probe_Value.	Simply get the rest of the word but	*/
		/* otherwise do nothing.				*/
		case Proto_Dead:
			LinkRx(3,link,&type);
			if( l.rc != 0 ) goto RxTimeout;
			
			continue;
#endif

#ifdef __C40		
		/* This is the result of a probe of a processor which	*/
		/* is running and is the inverse of the ls byte of	*/
		/* Probe_Value. The response to this is to exchange	*/
		/* info.						*/
		/* As the C40 can only transfer words and the prototype */
		/* byte is now a word, the reply comes as part of the	*/
		/* proto word.						*/
		case Proto_AliveFull:
			idle = FALSE;
			SendInfo(1,link);
			
			continue;
#else
		/* This is the result of a probe of a processor which	*/
		/* is running and is the inverse of the ls byte of	*/
		/* Probe_Value. The response to this is to exchange	*/
		/* info.						*/
		case Proto_Alive:
			idle = FALSE;
			
			LinkRx(3,link,&type);
			if( l.rc != 0 ) goto RxTimeout;
			
			SendInfo(1,link);
			
			continue;
#endif
		/* The other side of the link is sending info. If it 	*/
		/* wants a reply GetInfo will return true, and we call	*/
		/* SendInfo with no reply set.				*/
#ifdef __C40
		/* C40 receives entire Proto_Sync in its proto word */
		case Proto_Sync:
#else
		case Proto_Info:
#endif
			idle = FALSE;
			if( GetInfo(link) )
			{
				SendInfo(0,link);
			}

			break;

		/* The processor on the other side of the link is	*/
		/* terminating, inform the rest of the world.		*/
		case Proto_Term:
			ChangeState(link,Link_State_Dead);

			/* ...and drop through to reconfigure the link	*/
			
		/* The processor on the other side of the link has been	*/
		/* reconfigured, perform the same action here.		*/
		case Proto_Reconfigure:
		{
			LinkConf c = *(LinkConf *)link;
			c.Mode = Link_Mode_Dumb;
			c.State = Link_State_Dead;
			_Configure(c,FALSE);
			continue;
		}

		/* Proto_Reset is an 8 byte message which if sent to an */
		/* already reset processor looks like a bootstrap which */
		/* executes the start instruction, putting it back to	*/
		/* reset state.	Here we simply accept the whole message */
		/* and execute start ourself.				*/
#ifdef __C40
		case Proto_Reset0:
			/* C40 systems will already have read entire word */
			if( l.rc == 0 ) {
				LinkRx(4,link,&type);
			}

			_Terminate();
#else
		case Proto_Reset:
			LinkRx(3,link,&type);

			if( l.rc == 0 ) {
				LinkRx(4,link,&type);
			}
	
			_Terminate();
#endif

#ifdef FLOWCONTROL
		/* Xoff is sent when the remote processor is beginning	*/
		/* to run out of buffer memory, we should not send any	*/
		/* more messages until we get an Xon.			*/
		case Proto_Xoff:
			NewWorker(HandleXoff,link);
			continue;
			
		case Proto_Xon:
			HandleXon(link);
			continue;
#endif
	
		case Proto_ReSync:
			/* Sending 4 ReSyncs could help resync the system. */
			/* But not if we were in the middle of a message! */
			/* Helps if some garbage comes in when a link is */
			/* physically connected and a person immediately */
			/* starts up an IO Server */
			/* Only used in the ARM IO Server at present */
			continue;

#if defined(__C40) || defined(__ARM)
		case Proto_Go:
			/* This fix allows us to send messages down a half */
			/* duplex link without having to worry about */
			/* blocking the link while the IO Server is trying to */
			/* send us some info. The full story is very */
			/* involved... (you dont want to know) */
			_Signal(&link->HalfDuplex);
			continue;
#endif

		/* Any unknown type byte comes here where we emit	*/
		/* a trace record of it.				*/
		default:
#if defined(__C40) && defined(SYSDEB)
			_Trace(0x0badbad0,link->Id,type);
#endif
			continue;
		}

		if ( !idle && link->State != Link_State_Running )
		{
			ChangeState(link, Link_State_Running);
		}

		continue;
	
	/* Reception timeout: we jump here if we have failed to		*/
	/* receive data from a link.					*/
	/* We set idle false so we will force an idle exchange the	*/
	/* next time we time out. This should let us detect the link	*/
	/* reconnecting.						*/

	RxTimeout:
		ChangeState(link,Link_State_Timedout);
/*		idle = FALSE; */
		
	} /* end of main loop */
}

/*--------------------------------------------------------
-- DoRead						--
--							--
-- Reply to a debug read probe, run as a worker process	--
-- from the link guardian.				--
--							--
--------------------------------------------------------*/

void ReadReply(LinkInfo *link,word type)
{
	Id l;
	
	_WaitLink(link, &l);
	if( l.rc != Err_Null ) return;

	l.endtime = AddTimes(link->Timeout,Timer());

	LinkTx(4,link,&type);

	_SignalLink(link);
}

/*--------------------------------------------------------
-- Probe						--
--							--
-- send a write/read command down the link.		--
--							--
--------------------------------------------------------*/

void ProbeLink(LinkInfo *link)
{
	Id *l = link->TxUser;
	
	word buf;

	l->endtime = AddTimes(link->Timeout,Timer());
	
	buf = Proto_Write;
	
	LinkTx(sizeof(ProtoPrefix), link,&buf);	/* write command	*/
	if( l->rc != 0 ) goto linkfail;
	
	buf = Probe_Address;
	LinkTx(4,link,&buf);			/* address		*/
	if( l->rc != 0 ) goto linkfail;

	buf = Probe_Value;		
	LinkTx(4,link,&buf);			/* data			*/
	if( l->rc != 0 ) goto linkfail;

	/* now start to read it back	*/
	
	buf = Proto_Read;
	LinkTx(sizeof(ProtoPrefix), link,&buf);	/* read command		*/
	if( l->rc != 0 ) goto linkfail;

	buf = Probe_Address;
	LinkTx(4,link,&buf);			/* address to read	*/
	if( l->rc != 0 ) goto linkfail;

	/* We do not read the data here but let the main loop of the	*/
	/* link guardian get it as either a Proto_Alive or a Proto_Dead */
	/* message type.						*/
		
linkfail:;
}

void Probe(LinkInfo *link)
{
	Id l;

	JumpLink(link, &l);
	if( l.rc != Err_Null ) return;

	ProbeLink(link);
	
	_SignalLink(link);
}

/*--------------------------------------------------------
-- SendInfo						--
--							--
-- Send a sync/info message through the link.		--
-- This is done asynchronously with the link guardian	--
-- to avoid deadlock when both processors try this at	--
-- the same time.					--
--							--
--------------------------------------------------------*/

void _SendInfo(bool reply, LinkInfo *link);

void SendInfo(bool reply, LinkInfo *link)
{
#if defined(__C40) /*|| defined(__ARM)*/
	/* This causes problems on half duplex links and is not required */
	if (!(link->Flags & Link_Flags_HalfDuplex))
		NewWorker(_SendInfo,reply,link);
#else
	NewWorker(_SendInfo,reply,link);
#endif
}

void _SendInfo(bool reply, LinkInfo *link)
{
	Id l;
	InfoMsg	info;

	JumpLink(link, &l);
	if( l.rc != Err_Null ) return;

	l.endtime = AddTimes(link->Timeout,Timer());

	info.Sync = Proto_Sync;
	info.RxInc = (byte)link->Incarnation;
	info.TxInc = (byte)(GetRoot()->Incarnation);
	info.Reply = (byte)reply;
	info.Spare = 0;
	info.IOCPort = link->LocalIOCPort;

#if defined(SML_SPIRIT40) || defined(SML_VC40)
	LinkTx(sizeof(info.Sync), link, &info.Sync);
	LinkTx(sizeof(info)-sizeof(info.Sync),link,&info.TxInc);	
#else
	LinkTx(sizeof(info),link,&info);
#endif
	_SignalLink(link);
}


/*--------------------------------------------------------
-- GetInfo						--
--							--
-- Get an info message from the link			--
--							--
--------------------------------------------------------*/

bool GetInfo(LinkInfo *link)
{
	Id l;
	InfoMsg	 info;
	PTE *pte;
	RootStruct *root = GetRoot();

	l.rc = 0;
	l.endtime = AddTimes(link->Timeout,Timer());

	link->RxUser = &l;

	info.Sync = Proto_Sync;

#ifdef __C40
	/* the first sync word has already been got */
	LinkRx(sizeof(info)-4,link,(void *)(((word)&info)+4));
#else
	/* the first sync byte has already been got */
# ifdef __TRAN
	LinkRx(sizeof(info)-1,link,((word)&info)+1);
# else
	/* get around compiler error */
	LinkRx(sizeof(info)-1,link,(void *)(((word)&info)+1));
# endif
#endif

	link->RxUser = NULL;

#ifdef __ARM
	/* if an error is detected, startup a new info exchange */
	if( l.rc != 0 || info.Sync != Proto_Sync ) {
		SendInfo(1,link);
		return(FALSE);
	}
#else
	if( l.rc != 0 || info.Sync != Proto_Sync )
	{
	    return FALSE;
	}
#endif

	link->Incarnation = info.TxInc;
	
	if( info.RxInc > root->Incarnation ) root->Incarnation = info.RxInc;
	
	pte = GetPTE(link->RemoteIOCPort,root);

	pte->TxId = (Id *)info.IOCPort;

	if( info.IOCPort == IOProcIOCPort )
#if 1
		/* if we get an attaching IOServer, send IODebugs to it */
		link->Flags |= Link_Flags_ioproc | Link_Flags_debug;
#else
		link->Flags |= Link_Flags_ioproc;
#endif

	return info.Reply;
}

/*--------------------------------------------------------
-- ChangeState						--
--							--
-- The link is changing state, report this to LinkIOC	--
-- if necessary.					--
--							--
--------------------------------------------------------*/

void ChangeState(LinkInfo *link, word newstate)
{
	/* do nothing if we already know its state	*/
	if( link->State != (byte)newstate )
	{
		link->State = (byte)newstate;

		/* tell the LinkIOC if report flag set	*/
		if( link->Flags & Link_Flags_report ) {
			NewWorker(Exception,
				  EC_Warn|SS_Kernel|EG_Broken|newstate,
				  link->LocalIOCPort);
		}
	}
}

/*--------------------------------------------------------
-- Reconfigure						--
--							--
-- Reconfigure links.					--
--							--
--------------------------------------------------------*/

Code _Reconfigure(LinkConf *lc);

word Configure(LinkConf newconf)
{
	return(Code)System(_Configure,newconf,TRUE);
}

Code Reconfigure(LinkConf *lc)
{
	return (Code)System(_Reconfigure,lc);
}

 Code _Reconfigure(LinkConf *lc)
{
	int i;
	RootStruct *root = GetRoot();
	
	for( i = 0; root->Links[i] != NULL ; i++ )
	{
		_Configure(lc[i],TRUE);
	}
	return Err_Null;
}

#if 0
void SendRecon(LinkInfo *link)
{
	Id l;
	word fn = Proto_Reconfigure;
	
	_WaitLink(link,&l);
	if( l.rc != Err_Null ) return;

	l.endtime = AddTimes(link->Timeout,Timer());
	
	LinkTx(sizeof(ProtoPrefix),link,&fn);

	_SignalLink(link);	
}
#endif

#define Link_Flags_user (Link_Flags_debug|Link_Flags_report)

word _Configure(LinkConf newconf,bool sendrecon)
{
	RootStruct *root = GetRoot();
	LinkInfo *link = (root->Links)[newconf.Id];

	newconf.Flags &= Link_Flags_user;

	newconf.Flags |= (link->Flags & ~Link_Flags_user);

	/* Only do anything if the link is changing mode.	*/
	if( newconf.Mode != link->Mode )
	{
		/* For an intelligent node, fake a timeout	*/
		/* which will cause the guardian to loop.	*/
		if( link->Mode == Link_Mode_Intelligent )
		{
			Id l;
			Id *lu;
			Id *id;

			do
			{
				JumpLink(link,&l);
			} while( l.rc != Err_Null );

			/* We are now current TxUser of the link. 	*/

			/* change state here to fend off any further	*/
			/* Tx attempts to this link.			*/

			if( link->State != Link_State_Running )
				sendrecon = FALSE;
			link->State = Link_State_Dead;
			link->Mode = newconf.Mode;

			/* Start by sending a reconfigure byte to the	*/
			/* other side.					*/
			if( sendrecon )
			{
				word fn = Proto_Reconfigure;

				l.endtime = AddTimes(OneSec,Timer());
	
				LinkTx(sizeof(ProtoPrefix),link,&fn);
			}
			
			/* evict TxQueue */
			for( id=link->TxQueue; id!=NULL; id=id->next)
			{
				id->rc = Err_BadRoute;
				Resume(id->state);
			}
			link->TxQueue = NULL;
			
			/* If link guardian is in middle of message tfr */
			/* wait for it to finish			*/
			lu = link->RxId;
			if( lu != NULL )
	    		{
	    			Id me;
    				me.next = NULL;
    				me.tail = &me;
    				me.endtime = -1;
	    			link->Sync = &me;
    				Suspend(&me.state, THREAD_LINKEND);
			}

			/* look for a RxUser, which must be the link guardian */
			/* and restart it with a timeout, this will cause it  */
			/* to re-inspect the mode and quit.		      */

	    		lu = link->RxUser;
	    		if( lu != NULL )
	    		{
				SaveState *s = AbortLinkRx(link);

    				if( !NullStateP(s) )
    				{
	    				lu->rc = Err_Timeout;
	    				Resume(s);
    				}
    			}	

			/* return link to normal state */
			link->TxUser = NULL;

			/* clear out any port which reference this link */

			ClearPorts(link);
		}
		else
		{
			/* a dumb link guardian is just waiting	*/
			/* on the sync list, re-start it.	*/
			Resume(link->Sync->state);
			link->Sync = NULL;
			link->State = newconf.State;
		}
	}

	newconf.State = link->State;
	*(word *)link = *(word *)&newconf;
	
	return Err_Null;
}

/*--------------------------------------------------------
-- _BootLink						--
--							--
-- Kernel link boot entry point.			--
--							--
--------------------------------------------------------*/

#ifdef __TRAN
/* the following structure fixes up System only allowing 3 args */
struct BootInfo {
	word linkid;
	RPTR *image;
	Config *config;
	word confsize;
};

 Code __BootLink(struct BootInfo *info)
{
	RootStruct *root = GetRoot();

	LinkInfo *link = root->Links[info->linkid];
	void *bootstrap;
	word bootsize;
	word state = 0;
	Id l;

	if( info->linkid < 0 || info->linkid > GetConfig()->NLinks || 
		link->Mode != Link_Mode_Intelligent ) return Err_BadRoute;

	if( info->image == NULL ) info->image = (RPTR *)GetSysBase();

	_WaitLink(link, &l);
	if( l.rc != Err_Null ) return Err_BadRoute;

#ifdef SOFT_RESET
	/* first we send a soft reset message through the link to make */
	/* sure the far processor is not running.		       */
	/* additionally if this fails to get through then there is     */
	/* little point in sending the whole bootstrap.		       */

	l.endtime = AddTimes(OneSec,Timer());

	bootsize = Proto_Reset0;
	state = 1;
	LinkTx(4,link,&bootsize);
	if( l.rc != 0 ) goto failed;
	
	bootsize = Proto_Reset1;
	state = 2;
	LinkTx(4,link,&bootsize);
	if( l.rc != 0 ) goto failed;

	/* if the remote processor was alive, give it a little time here */
	/* to reset itself.						 */
	Sleep( 100 );
#endif
	l.endtime = AddTimes(OneSec,Timer());

	bootstrap = (void *)RTOA(info->image[IVecBootStrap]);
	bootsize = (word)RTOA(info->image[IVecProcMan])-(word)bootstrap;

	state = 3;
	LinkTx(1,link,&bootsize);			/* bootstrap size */
	if( l.rc != 0 ) goto failed;

	state = 4;
	LinkTx(bootsize,link,bootstrap);		/* bootstrap	*/
	if( l.rc != 0 ) goto failed;

	/* This delay gives the bootstrap time to initialise itself.	*/
	/* In particular there is a small delay between startup and 	*/
	/* resetting the links while it saves the old processor state.  */
	Sleep( 20 );

#ifdef CLEAR_ON_BOOT
	/* if the memory size is non-zero, get bootstrap to clear it	*/
	if( info->config->MemSize != 0 )
	{
		l.endtime = AddTimes(OneSec,Timer());

		bootsize = 5;
		state = 5;
		LinkTx(1,link,&bootsize);
		if( l.rc != 0 ) goto failed;
		
		bootsize = MinInt + info->config->MemSize;
		state = 6;
		LinkTx(4,link,&bootsize);
		if( l.rc != 0 ) goto failed;

		state = 7;
		LinkTx(1,link,&bootsize);
		if( l.rc != 0 ) goto failed;
	}
#endif	
	l.endtime = AddTimes(OneSec,Timer());

	bootsize = 4;					/* boot command */
	state = 8;
	LinkTx(1,link,&bootsize);
	if( l.rc != 0 ) goto failed;

	state = 9;
	LinkTx(info->image[0],link,info->image);	/* image */
	if( l.rc != 0 ) goto failed;

	state = 10;
	LinkTx(4,link,&info->confsize);		/* config size	*/
	if( l.rc != 0 ) goto failed;

	state = 11;
	LinkTx(info->confsize,link,info->config);	/* configuration */
	if( l.rc != 0 ) goto failed;

failed:
	_SignalLink(link);
	
	if( l.rc != 0 ) l.rc = (l.rc & ~EO_Mask) | state;
	
	return l.rc;
}


Code _BootLink(word linkid, void *image, Config *config, word confsize)
{
	return (Code)System(__BootLink,(struct BootInfo *)&linkid);
	image = image;
	config = config;
	confsize = confsize;
}
#endif /* __TRAN */


/*--------------------------------------------------------
-- EnableLink						--
--							--
-- Enable link for message passing.			--
--							--
--------------------------------------------------------*/

Code _EnableLink(word linkid);

Code EnableLink(word linkid)
{
	return (Code)System(_EnableLink,linkid);
}

Code _EnableLink(word linkid)
{
	RootStruct *root = GetRoot();
	LinkInfo *link = root->Links[linkid];
	byte oflags = link->Flags;

	if( linkid < 0 || linkid >= GetConfig()->NLinks || 
		link->Mode != Link_Mode_Intelligent ) return Err_BadRoute;


	/* put link into known state					*/
	link->State = Link_State_Timedout;
	
	/* ensure any state changes here do not produce reports		*/
	link->Flags &= ~Link_Flags_report;
	
	/* now do a probe 						*/

	NewWorker(Probe,link);

	/* loop here until the link resolves itself into either Running	*/
	/* or Dead state.						*/
	while( link->State == Link_State_Timedout )
	{
		Sleep( 200 );
	}

	link->Flags = oflags;
	
	return Err_Null;
}

/*--------------------------------------------------------
-- LinkData						--
--							--
-- Get a copy of a given link's info struct.		--
--							--
--------------------------------------------------------*/

Code LinkData(word linkid, LinkInfo *info)
{
	RootStruct *root = GetRoot();
	LinkInfo *link;
	
	if( linkid < 0 || linkid >= GetConfig()->NLinks) return Err_BadLink;

	link = root->Links[linkid];
	
	*info = *link;		/* structure copy */
	
	return Err_Null;
}

#ifdef __TRAN

/*--------------------------------------------------------
-- SoftReset						--
--							--
-- Send a soft reset message through the given link	--
--							--
--------------------------------------------------------*/

Code _SoftReset(word linkid)
{
	Id l;
	word buf;
	RootStruct *root = GetRoot();
	LinkInfo *link = root->Links[linkid];

	if( linkid < 0 || linkid >= GetConfig()->NLinks) return Err_BadRoute;

	_WaitLink(link, &l);
	if( l.rc != Err_Null ) return Err_BadRoute;

	l.endtime = AddTimes(OneSec,Timer());

	buf = Proto_Reset0;
	LinkTx(4,link,&buf);
	if( l.rc != 0 ) goto failed;

	buf = Proto_Reset1;
	LinkTx(4,link,&buf);
	if( l.rc != 0 ) goto failed;

	ChangeState(link,Link_State_Dead);	

failed:
	_SignalLink(link);
	return l.rc;	
}

Code SoftReset(word linkid)
{
	return (Code)System(_SoftReset,linkid);
}
#endif /* __TRAN */


/*----------------------------------------------------------------
-- AllocLink							--
-- FreeLink							--
-- LinkIn							--
-- LinkOut							--
--								--
--	Raw link control.					--
--								--
----------------------------------------------------------------*/

Code AllocLink(word linkid)
{
	RootStruct *root = GetRoot();
	LinkInfo *link = root->Links[linkid];
	Id id;
	
	if( 	linkid < 0 || linkid > GetConfig()->NLinks || 
		link->Mode != Link_Mode_Dumb ||
		link->State == Link_State_Running ) return Err_BadRoute;
		
	_WaitLink(link, &id);
	if( id.rc != Err_Null ) return Err_BadRoute;

        link->TxUser = NULL;
	link->State = Link_State_Running;
		
	return Err_Null;
}

Code FreeLink(word linkid)
{
	RootStruct *root = GetRoot();
	LinkInfo *link = root->Links[linkid];
	
	if( 	linkid < 0 || linkid > GetConfig()->NLinks || 
		link->Mode != Link_Mode_Dumb ||
		link->State != Link_State_Running ) return Err_BadRoute;
	
	_SignalLink(link);
	link->State = Link_State_Dead;
	
	return Err_Null;
}


#ifdef __C40
Code LinkIn(word size, word linkid, void *buf, word timeout)
{
#ifdef __C40
	/* If not a word multiple aligned to a word boundary */
	/* then return an invalid message error. */
	if (size & 3 || (word)buf & 3)
		return EC_Error+SS_Kernel+EG_Invalid+EO_Message;
#endif
	return MP_LinkIn(size >> 2, linkid, C40WordAddress(buf), timeout);
}

Code LinkOut(word size, word linkid, void *buf, word timeout)
{
#ifdef __C40
	/* If not a word multiple aligned to a word boundary */
	/* then return an invalid message error. */
	if (size & 3 || (word)buf & 3)
		return EC_Error+SS_Kernel+EG_Invalid+EO_Message;
#endif
	return MP_LinkOut(size >> 2, linkid, C40WordAddress(buf), timeout);
}


/* Read from Link into buffer, buffer address is a word pointer and size */
/* is defined in terms of words. */

word MP_LinkIn(word size, word linkid, MPtr buf, word timeout)
{
	RootStruct *root = GetRoot();
	LinkInfo *link = root->Links[linkid];
	Id l;

	if( linkid < 0 || linkid >= GetConfig()->NLinks || link->Mode != Link_Mode_Dumb ) return Err_BadRoute;

	if( size == 0 ) return Err_Null;

	if( timeout > 0 && timeout < OneSec*2 ) timeout = OneSec*2;
			
	l.rc = 0;
	l.endtime = (timeout==-1)?-1:AddTimes(root->Timer,timeout);

	link->RxUser = &l;
	link->RxId = &l;
#ifndef __TRAN
	/* allows KillTask to safely kill threads that are doing link */
	/* operations. */
	l.state = GetExecRoot()->CurrentSaveArea;
#endif
	System((WordFnPtr)MP_LinkRx, size, link, buf);

	link->RxUser = NULL;
	link->RxId = NULL;

	return l.rc;
}

/* Write from buffer to Link, buffer address is a word pointer and size */
/* is defined in terms of words. */

word MP_LinkOut(word size, word linkid, MPtr buf, word timeout)
{
	RootStruct *root = GetRoot();
	LinkInfo *link = root->Links[linkid];
	Id l;

	if ( linkid < 0 || linkid >= GetConfig()->NLinks || (
	     link->Mode == Link_Mode_Intelligent &&
	     link->State == Link_State_Running)
        )
		return Err_BadRoute;

	if( size == 0 ) return Err_Null;

	if( timeout > 0 && timeout < OneSec*2 ) timeout = OneSec*2;

	l.rc = 0;
	l.endtime = (timeout==-1)?-1:AddTimes(root->Timer,timeout);

	link->TxUser = &l;
#ifndef __TRAN
	/* allows KillTask to safely kill threads that are doing link */
	/* operations. */
	l.state = GetExecRoot()->CurrentSaveArea;
#endif
	System((WordFnPtr)MP_LinkTx, size, link, buf);

	link->TxUser = NULL;

	return l.rc;
}
#else
Code LinkIn(word size, word linkid, void *buf, word timeout)
{
	RootStruct *root = GetRoot();
	LinkInfo *link = root->Links[linkid];
	Id l;

	if( linkid < 0 || linkid >= GetConfig()->NLinks || link->Mode != Link_Mode_Dumb ) return Err_BadRoute;

	if( size == 0 ) return Err_Null;

#ifdef __C40
	/* If not a word multiple aligned to a word boundary */
	/* then return an invalid message error. */
	if (size & 3 || (word)buf & 3)
		return EC_Error+SS_Kernel+EG_Invalid+EO_Message;
#endif

	if( timeout > 0 && timeout < OneSec*2 ) timeout = OneSec*2;
			
	l.rc = 0;
	l.endtime = (timeout==-1)?-1:AddTimes(root->Timer,timeout);

	link->RxUser = &l;
	link->RxId = &l;
#ifndef __TRAN
	/* allows KillTask to safely kill threads that are doing link */
	/* operations. */
	l.state = GetExecRoot()->CurrentSaveArea;
#endif
	LinkRx(size,link,buf);

	link->RxUser = NULL;
	link->RxId = NULL;

	return l.rc;
}

Code LinkOut(word size, word linkid, void *buf, word timeout)
{
	RootStruct *root = GetRoot();
	LinkInfo *link = root->Links[linkid];
	Id l;

	if ( linkid < 0 || linkid >= GetConfig()->NLinks || (
	     link->Mode == Link_Mode_Intelligent &&
	     link->State == Link_State_Running)
        )
		return Err_BadRoute;

	if( size == 0 ) return Err_Null;

#ifdef __C40
	/* If not a word multiple aligned to a word boundary */
	/* then return an invalid message error. */
	if (size & 3 || (word)buf & 3)
		return EC_Error+SS_Kernel+EG_Invalid+EO_Message;
#endif

	if( timeout > 0 && timeout < OneSec*2 ) timeout = OneSec*2;

	l.rc = 0;
	l.endtime = (timeout==-1)?-1:AddTimes(root->Timer,timeout);

	link->TxUser = &l;
#ifndef __TRAN
	/* allows KillTask to safely kill threads that are doing link */
	/* operations. */
	l.state = GetExecRoot()->CurrentSaveArea;
#endif
	LinkTx(size,link,buf);

	link->TxUser = NULL;

	return l.rc;
}
#endif


/*--------------------------------------------------------
-- Terminate						--
--							--
-- Inform neighbours that the processor is about to	--
-- quit.						--
--							--
--------------------------------------------------------*/

static void _Terminate(void)
{
	RootStruct *root = GetRoot();
	int i;

	for( i = 0; root->Links[i] != NULL ; i++ )
	{
		LinkInfo *link = root->Links[i];
		
		/* send a Term byte to all running links.	*/
		if( link->Mode == Link_Mode_Intelligent && 
		    link->State == Link_State_Running )
		{
			Id l;
			word proto = Proto_Term;
			
			_WaitLink(link, &l);
			
			l.endtime = AddTimes(LinkTxTimeout,Timer());
			
			LinkTx(sizeof(ProtoPrefix), link, &proto);
		}
	}

	Reset();
}

static void Reset(void)
{
#ifdef __TRAN
	word *links = (word *)MinInt;
	int i;
	
	/* first zap all link channels plus event channel */
	/* it would be nice if we could allow all tfrs to finish first...*/

	for( i = 0; i < 9 ; i++) resetch_(links+i);
#else
	ResetLinks();
#endif

	/* finally we execute start which resets the processor */

#ifdef __TRAN
	forever;
	/* start_();*/
#else
	ResetCPU();
#endif
}

void Terminate(void)
{
	System((WordFnPtr)_Terminate);
}

#ifdef FLOWCONTROL

#define FC_Xon		100	/* rc value HandleXon -> HandleXoff 	   */
#define FC_LinkTx	101	/* rc value for HandleXoff while in LinkTx */

void FlowControl(word code)
{
	RootStruct *root = GetRoot();
	int i;

	/* set root xoffed flag if this is xoff	*/
	if( code == Proto_Xoff ) 
	{
		if( root->Flags & Root_Flags_xoffed ) return;
		root->Flags |= Root_Flags_xoffed;
	}
	else
	{
		unless( root->Flags & Root_Flags_xoffed ) return;
		root->Flags &= ~Root_Flags_xoffed;
	}
	
	for( i = 0; root->Links[i] != NULL ; i++ )
	{
		LinkInfo *link = root->Links[i];
		
		/* do not send Xon/Xoff to an io processor	*/
				
		if( link->Flags & Link_Flags_ioproc ) continue;
		
		/* send the code to each running link */

		if( link->Mode == Link_Mode_Intelligent && 
		    link->State == Link_State_Running )
		{
			/* If the link is stopped, we cannot wait here	*/
			/* tell the Xoff process to send the code	*/
			if( link->Flags & Link_Flags_stopped )
			{
				link->TxUser->rc = code;
				Resume(link->TxUser->state);
			}
			else 
			{
				Id l;

				do
				{
					JumpLink(link, &l);
				} while( l.rc != Err_Null );

			
				l.endtime = AddTimes(LinkTxTimeout,Timer());

				LinkTx(sizeof(ProtoPrefix),link,&code);
						
				_SignalLink(link);
			}
		}
	}
	
}

void HandleXoff(LinkInfo *link)
{
	Id id;
	
	/* allow for multiple xoffs */
	if( link->Flags & Link_Flags_stopped ) return;

	/* Jump to the HEAD of the TxQueue			*/
	
	do
	{
		_JumpLink(link, &id);
	} while( id.rc != Err_Null );
		
	link->Flags |= Link_Flags_stopped;
	
	/* and sleep, forever, thereby blocking all transmissions */

	id.rc = 0;
	while( id.rc != FC_Xon )
	{
		id.rc = 0;
		id.endtime = -1;
		Suspend(&id.state, THREAD_LINKXOFF);

		/* if rc != FC_Xon we have been woken by FlowControl	*/
		/* to send an Xon or Xoff byte through the link 	*/
		if( id.rc != FC_Xon )
		{
			word code = id.rc;
			id.rc = FC_LinkTx;
			id.endtime = AddTimes(LinkTxTimeout,Timer());
			LinkTx(sizeof(ProtoPrefix), link, &code);
		}
	}

	link->Flags &= ~Link_Flags_stopped;

	_SignalLink(link);
}

void HandleXon(LinkInfo *link)
{
	Id *id;

	unless( link->Flags & Link_Flags_stopped ) return;

	id = link->TxUser;
	
	/* if the Xoff process is in the middle of a link Tx abort it */
	/* here. This will only happen if the link is jammed and the  */
	/* Tx will timeout anyway. 				      */

	if( id->rc == FC_LinkTx )
	{
		SaveState *s = AbortLinkTx(link);
		if( !NullStateP(s) )
		{
			CheckState(s);
			id->rc = FC_Xon;
			Resume(s);
		}		
	}
	else
	{
		id->rc = FC_Xon;
		Resume(id->state);
	}
}
#endif

void _WaitLink(LinkInfo *link, Id *id)
{	
	id->rc = Err_Null;
	id->endtime = -1;

	/* if the link is in use, queue for it		*/
	if( link->TxUser != NULL )
	{
		id->next = NULL;
		id->tail = id;
		id->state = NULL;
		id->mcb = NULL;
		
		if( link->TxQueue == NULL ) link->TxQueue = id;
		else {
			link->TxQueue->tail->next = id;
			link->TxQueue->tail = id;
		}

		Suspend(&id->state, THREAD_LINKEND);
		/* wakeup having reached head of queue	*/
	}
	else link->TxUser = id;
}


void _JumpLink(LinkInfo *link, Id *id)
{
	id->rc = Err_Null;
	id->endtime = -1;

	if( link->TxUser != NULL )
	{
		if( link->TxQueue == NULL ) id->next = NULL, id->tail = id;
		else id->next = link->TxQueue, id->tail = link->TxQueue->tail;
		
		id->state = NULL;
		id->mcb = NULL;
		link->TxQueue = id;

		Suspend(&id->state, THREAD_LINKEND);
	}
	else link->TxUser = id;
}

void _SignalLink(LinkInfo *link)
{
	/* start next process waiting for link */
	if( link->TxQueue != NULL )
	{
		Id *succ = link->TxQueue;
		link->TxQueue = succ->next;
		if( succ->next != NULL ) succ->next->tail = succ->tail;
		Resume(succ->state);
		link->TxUser = succ;
	}
	else link->TxUser = NULL;
}


#endif /* LINKIO */


/* -- End of link.c */
