/*{{{  Header */

/*------------------------------------------------------------------------
--                                                                      --
--                     P O S I X    L I B R A R Y			--
--                     --------------------------                       --
--                                                                      --
--             Copyright (C) 1988 - 1994, Perihelion Software Ltd.      --
--                        All Rights Reserved.                          --
--                                                                      --
-- signal.c								--
--                                                                      --
--	Signal handling routines					--
--                                                                      --
--	Author:  NHG 8/5/88						--
--                                                                      --
------------------------------------------------------------------------*/
/* $Id: signal.c,v 1.24 1994/03/15 11:53:19 nickc Exp $ */

/*}}}*/
/*{{{  Includes */

#include <helios.h>	/* standard header */

#define __in_signal 1	/* flag that we are in this module */

#include <posix.h>

#include "pposix.h"

#include <signal.h>
#include <codes.h>
#include <ioevents.h>
#include <nonansi.h>	/* for _cputime() */

#ifndef __TRAN
#include <process.h>
#endif

/*}}}*/
/*{{{  Macros and Constants */

/*
** BLV - in a multi-priority system handling signals correctly is quite
** difficult, and various calls to SetPriority() are needed. I do not
** want these calls in a transputer system.
*/
#ifdef __TRAN
#define SetPriority(a)
#endif

#define _Trace(a,b,c)

#define S(x) (1U<<x)

/* The following signals may be delivered asyncronously by default */
#define DFL_ASYNC	(S(SIGABRT)|S(SIGHUP)|\
			S(SIGINT)|S(SIGKILL)|S(SIGQUIT)|S(SIGTERM)|S(SIGSIB))

/* The following signals are ignored by default			  */
#define DFL_IGNORE	(S(SIGCHLD))

/* The following signals will be passed on to the children	  */
#define DFL_PROPAGATE	(S(SIGINT)|S(SIGKILL)|S(SIGTERM)|S(SIGHUP)|S(SIGABRT))

/* The following signals may not be blocked, cought or ignored		  */
#define DFL_ILLEGAL (S(SIGKILL)|S(SIGSTOP))

#ifdef __TRAN
#define DFL_UNSUPPORTED (S(SIGSEGV)|S(SIGFPE)|S(SIGILL))
#else
#if defined(__ARM) || defined(__C40)
	/* @@@ may change for C40 */
#define DFL_UNSUPPORTED 0
#else
#error Unknown processor type
#endif
#endif

/* This macro checks that the given signal is within the range and not	*/
/* one of the unsupported set.						*/
#define VALIDSIGNAL(sig) \
		((sig >= 0) && (sig < sizeof(sigset_t)*8) && \
		(((1<<sig) & DFL_UNSUPPORTED)==0))

/* This macro checks that the signal is not one  of the set that we may */
/* not mask, ignore or catch.						*/
#define CATCHABLE(sig) (((1<<sig) & DFL_ILLEGAL)==0)

#define CHECKSIGS()	if(mask.pending) raise_sync()

#define MyName	((char *)(MyTask->TaskEntry)+8)

/*}}}*/
/*{{{  Local Variables */

STATIC struct sigmask	mask;	  /* current set of signal masks	*/
STATIC struct sigmask	savemask; /* set saved after vfork		*/

STATIC struct sigaction sigactions[sizeof(sigset_t)*8];

static bool		Async;		/* are we in sync or async mode? */

static Semaphore	siglock;
STATIC Port		PausePort;
static Port		SignalPort;
static Port		EventPort;

static int		sigstacksize;

static int		SysTimeout;
static int		AlarmSet;
static int		AlarmTime;

static int		sigchild;

/*}}}*/
/*{{{  Forward References */

static int dokill(int pcb, int sig);
static int inner_raise(int sig, bool async, bool propagate);
static void _raise_pending_sigs(bool async, bool locked);
static void _raise(int sig);
static void raise_sync(void);
static void propagate_signals(void);

/*}}}*/
/*{{{  Code */

/*{{{  init_signal() */

#ifdef __TRAN
static
#endif
void init_signal(sigset_t sigmask, sigset_t ignore)
{
	word i;
_Trace(0xcccc1111,MyTask,0);
	InitSemaphore(&siglock,1);
	mask.pending = 0;
	mask.mask = sigmask;
	mask.async = DFL_ASYNC;
	mask.ignore = ignore|DFL_IGNORE;
	mask.propagate = 0;
	
	Async = FALSE;
	
	sigstacksize = 0;
	
	for( i = 0 ; i < sizeof(sigset_t)*8 ; i++ ) 
	{
	       if( (1L << i) & mask.ignore )
		     sigactions[i].sa_handler = SIG_IGN;
		else sigactions[i].sa_handler = SIG_DFL;
		sigactions[i].sa_flags = 0;
		sigactions[i].sa_mask = 0;
	}
}

/*}}}*/
/*{{{  _posix_exception_handler() */

/* _posix_exception_handler is called from a language-runtime-	*/
/* system created process with at least 2k of stack. This will	*/
/* not return.							*/
/* BLV - 15.6.93, major changes, see the comment at the start	*/
/* of tcsetpgrp() for details.					*/
extern void _posix_exception_handler(void)
{
	word	e;
	MCB	mcb;
	IOEvent	event;
	Port	ports[2];
	
	SignalPort	= NewPort();
	EventPort	= NewPort();
	
	/* Assure that signal handler can process all new EnableEvents ports */
	/* sent via tcsetpgrp() (called by the shell) quickly enough. */
	/* Whenever we force a signal to be raised, we drop back down to */
	/* Standard priority to stop the Exit() code looping at a higher */
	/* priority than the code that is waiting to abort */
	SetPriority(HighServerPri);

	mcb.Data = (byte *)&event;
	
	SetSignalPort(SignalPort);

	AlarmTime  = -1;
	
	for(;;)
	{
		unsigned long	time = _cputime();
		word		index;
		
		mcb.Timeout	= AlarmTime;
		ports[0]	= SignalPort;
		ports[1]	= EventPort;

		index		= MultiWait(&mcb, 2, ports);
		if( index == EK_Timeout ) 
		{
		raisealarm:

			SysTimeout	= 0;
			AlarmTime	= -1;
			e		= EK_Timeout;

			SetPriority(StandardPri);
			inner_raise(SIGALRM,TRUE,FALSE);
			SetPriority(HighServerPri);

#if 1
			/* This continue appears to have been missing since	*/
			/* 15.11.1990, between the 1.1A and 1.2 releases. The	*/
			/* result was that the signal port was reallocated	*/
			/* after every timeout. Unfortunately there may have	*/
			/* been a reason for this reallocation...		*/
			continue;
#else
			index = 0;	/* this gives the old behaviour	*/
#endif
		}
		elif (index < 0)
			e = index;
		else
			e = mcb.MsgHdr.FnRc;
		
		/* adjust the wait period by the length of time we were away */
		if( e < 0 && AlarmTime != -1 ) 
		{
			AlarmTime -= 10000*(int)(((unsigned)_cputime() - time));
			if( AlarmTime <= 0 ) goto raisealarm;
		}

		/* ctrl-C events go to the Event port in slot 1		*/
		if( index == 1 && mcb.MsgHdr.DataSize == Keyboard_EventSize)
		{
#if 0
			IOdebug("%s: ^C", MyName);
#endif
			_Trace(0xCCCC0001,MyTask,0);
			SetPriority(StandardPri);
			inner_raise(SIGINT,TRUE,TRUE);
			SetPriority(HighServerPri);
			continue;
		}
			/* tcsetpgrp() can send a message to indicate that a	*/
			/* new event port should be enabled.			*/
		elif (index == 1 && e == 789)
		{
			int	 fd;
			fdentry	*f;
			Stream	*s;
			Port	 port;

			/* The file descriptor has just been checked. I hope it is	*/
			/* still ok...							*/
			fd	= *((int *)mcb.Data);
			f	= checkfd(fd);
			s	= f->pstream->stream;

			port	= EnableEvents(s, Event_Break);

			/* If successful, use the new port. Otherwise stick	*/
			/* with the current one.				*/
			if (port != NullPort)
			{
				FreePort(EventPort);
				EventPort	= port;
			}

			/* Now send back a reply to tcsetpgrp()			*/
			InitMCB(&mcb, 0, mcb.MsgHdr.Reply, NullPort,
				(port == NullPort) ? Result2(s) : Err_Null);
			PutMsg(&mcb);
			continue;
		}
			/* No other messages should ever arrive at the event	*/
			/* port. However if the route to the window server is	*/
			/* broken for any reason, for example a crashed		*/
			/* processor, then the port trail may be invalidated.	*/
			/* There is no easy way of recovering, but the event	*/
			/* port must be re-allocated anyway to prevent spinning	*/
			/* on a duff message port. Strictly speaking a		*/
			/* semaphore is needed to avoid clashing with any calls	*/
			/* to tcsetpgrp(). This semaphore could also be used	*/
			/* to prevent multiple calls to tcsetpgrp() upsetting	*/
			/* things. Unfortunately there could still be race	*/
			/* conditions.						*/
		elif (index == 1)
		{
#ifdef SYSDEB
			if (e != 0 && e != 0xc10e0005 /* Err_Abort */)
			  IOdebug("%s: Unexpected error %x received on event port", MyName, e);
#endif
			FreePort(EventPort);
			EventPort	= NewPort();
			continue;
		}
			/* All other errors or messages must be for the current	*/
			/* signal port. Signals must be raised. Anything else	*/
			/* causes the reallocation of the signal port, for	*/
			/* similar reasons to the reallocation of the event	*/
			/* port.						*/
		elif( (e & (ErrBit|EG_Mask|EE_Signal)) == 
		 (ErrBit|EG_Exception|EE_Signal) )
		 {
			SetPriority(StandardPri);
			inner_raise((int)(e & 0x1f),TRUE,TRUE);
			SetPriority(HighServerPri);
		 }
		else
		{
			if (e < 0) {
				/* An error from MultiWait() contains the */
				/* index of the port to which the error */
				/* corresponds or'd into its low order bits. */
				if (e & 1) {
					FreePort(EventPort);
					EventPort	= NewPort();
				} else {
					FreePort(SignalPort);
					SignalPort	= NewPort();
					SetSignalPort(SignalPort);
				}
			}
#ifdef SYSDEB
			elif (e != 456 && e != 0)	/* 456 == kosha alarm() setup */
			  IOdebug("%s: unexpected error %x received on signal port", MyName, e);
#endif
			continue;
		}

	}
}

/*}}}*/
/*{{{  kill() */

static int inner_kill(int pid, int sig, bool internal);

extern int kill(int pid, int sig)
{
	int ret;
	CHECKSIGS();
	ret = inner_kill(pid,sig,FALSE);
	CHECKSIGS();
	return ret;
}

/*}}}*/
/*{{{  inner_kill() */

static int inner_kill(int pid, int sig, bool internal)
{
	int e = 0;

	_Trace(0xCCCC0002,MyTask,sig|(pid<<16)|(internal<<7));

	if( (sig < 0) || (sig >= sizeof(sigset_t)*8) )	{ errno = EINVAL; e = -1; goto done; }

	/* if pid == 0 send signal to my process group		*/
	/* if pid < 0 signal process group |pid|		*/
	/* if pid > 0 signal process pid			*/

	if( pid == 0 ) pid = -childvec[sigchild].pgrp;  
	
	if ( !internal && pid == childvec[0].pid ) inner_raise(sig,FALSE,FALSE);
	elif( pid < 0 )
	{
		int i;
		pid_t pgrp = -pid;
		for( i = 1; i < childvecsize ; i++ ) 
		{
			PCB *pcb = childvec+i;
			if( 	pcb->pid != 0		&& 
				pcb->status == MinInt	&&
				pcb->pgrp == pgrp
			  )
			  dokill(i,sig);
		}
		if( !internal && pgrp == childvec[0].pgrp ) inner_raise(sig,FALSE,FALSE);
	}
	else
	{
		int pcb = 0;
		int i;
		for( i = 1 ; i < childvecsize ; i++ )
			if( childvec[i].pid == pid ) {pcb = i; break; }
			
		if( pcb == 0 )
		{ errno = ESRCH; e = -1; goto done; }

		e = dokill(pcb,sig);
	}
done:
	_Trace(0xCCCC0012,MyTask,sig|(pid<<16)|(internal<<7));
	
	return e;
}

/*}}}*/
/*{{{  dokill() */

static int dokill(int pcb, int sig)
{
	Stream *s;
	word e;
_Trace(0xCCCC0003,MyTask,sig|(childvec[pcb].pid<<16));

	if( sig == 0 ) 
	{
		errno = ESRCH;
		return childvec[pcb].status == MinInt ? 0 : -1 ;
	}

	s = CopyStream(childvec[pcb].stream);

	if( s == NULL )
	{ errno = ESRCH; return -1; }

	if( (e = SendSignal( s, sig )) < 0 )
	{
		errno = posix_error(Result2(s)); 
		Close(s);
		return -1;
	}
	
	Close(s);

	_Trace(0xCCCC0013,MyTask,sig|(childvec[pcb].pid<<16));

	return 0;
}

/*}}}*/
/*{{{  sigemptyset() */

extern int sigemptyset(sigset_t *set)
{
	*set = 0;
	return 0;	
}

/*}}}*/
/*{{{  sigfillset() */

extern int sigfillset(sigset_t *set)
{
	*set = 0xFFFFFFFF;
	return 0;
}

/*}}}*/
/*{{{  sigaddset() */

extern int sigaddset(sigset_t *set, int sig)
{
	if( !VALIDSIGNAL(sig) )	{ errno = EINVAL; return -1; }
	*set |= 1L << sig;
	return 0;
}

/*}}}*/
/*{{{  sigdelset() */

extern int sigdelset(sigset_t *set, int sig)
{
	if( !VALIDSIGNAL(sig) )	{ errno = EINVAL; return -1; }
	*set &= ~(1L << sig);
	return 0;
}

/*}}}*/
/*{{{  sigismember() */

extern int sigismember(sigset_t *set, int sig)
{
	if( !VALIDSIGNAL(sig) )	{ errno = EINVAL; return -1; }
	return (*set & (1L << sig)) != 0 ;
}

/*}}}*/
/*{{{  sigaction() */

extern int sigaction(int sig, struct sigaction *act, struct sigaction *oact)
{
	if( !VALIDSIGNAL(sig) ) { errno = EINVAL; return -1; }

	Wait(&siglock);

	if( oact != NULL ) *oact = sigactions[sig];
	
	if( act != NULL ) 
	{
		sigset_t sigbit = (1L << sig);

		if( !CATCHABLE(sig) )
		{ errno = EINVAL; Signal(&siglock); return -1; }

		sigactions[sig] = *act;
		
		/* If we are setting the default action, and the	*/
		/* default is to ignore signals, or if we are setting	*/
		/* the action to SIG_IGN, set the ignore mask bit and 	*/
		/* clear any pending signal. Otherwise clear the ignore	*/
		/* mask bit.						*/
		if( ((sigbit & DFL_IGNORE) && act->sa_handler == SIG_DFL) ||
			act->sa_handler == SIG_IGN ) 
		{
			mask.ignore |= sigbit;
			mask.pending &= ~sigbit;
		}
		else mask.ignore &= ~sigbit;
		
		/* If async flag is set, allow async delivery	*/
		/* else if setting to default && asynchronous	*/
		/* is default behaviour set async bit,		*/
		/* else clear the bit.				*/
		if( sigactions[sig].sa_flags & SA_ASYNC ) mask.async |= sigbit;
		else if( act->sa_handler == SIG_DFL && 
			 (sigbit & DFL_ASYNC) ) mask.async |= sigbit;
		else mask.async &= ~sigbit;
	}

	_raise_pending_sigs(FALSE,TRUE);
	
	Signal(&siglock);
	
	return 0;
}

/*}}}*/
/*{{{  sigprocmask() */

extern int sigprocmask( int how, sigset_t *set, sigset_t *oset)
{
	Wait(&siglock);	

	if( oset != NULL ) *oset = mask.mask;
	
	if( set == NULL ) goto done;

	switch( how )
	{
	case SIG_BLOCK:
		mask.mask |= *set;
		break;
	case SIG_UNBLOCK:
		mask.mask &= ~(*set);
		break;
	case SIG_SETMASK:
		mask.mask = *set;
		break;
	default:
		errno = EINVAL;
		Signal(&siglock);	/* PAB fix */
		return -1;
	}
	
	/* ensure that the unblockable set is still unblocked	*/
	mask.mask &= ~DFL_ILLEGAL;

done:
	_raise_pending_sigs(FALSE,TRUE);

	Signal(&siglock);	
			
	return 0;
}

/*}}}*/
/*{{{  sigpending() */

extern int sigpending(sigset_t *set)
{
	Wait(&siglock);	
	*set = mask.pending & mask.mask;
	Signal(&siglock);	
	CHECKSIGS();
	return 0;
}

/*}}}*/
/*{{{  sigsuspend() */

extern int sigsuspend(sigset_t *set)
{
	sigset_t old;
	
	Wait(&siglock);	
	
	old = mask.mask;
	
	mask.mask = *set;
	
	Signal(&siglock);	
	
	pause();
	
	Wait(&siglock);	
	
	mask.mask = old;
	
	Signal(&siglock);	
	
	return -1;
}

/*}}}*/
/*{{{  signal() */

extern void (*signal (int sig, void (*func)(int)))(int)
{
	struct sigaction New,old;
	
	New.sa_handler = func;
	New.sa_mask = 0;
	New.sa_flags = SA_SETSIG;
	
	if( sigaction(sig,&New,&old) == -1 ) return SIG_ERR;
	
	return old.sa_handler;
}

/*}}}*/
/*{{{  setsigstacksize() */

extern int setsigstacksize(int stacksize)
{
	int old;
	Wait(&siglock);
	old = sigstacksize;
	sigstacksize = stacksize & ~3;
	Signal(&siglock);
	CHECKSIGS();
	return old;
}

/*}}}*/
/*{{{  raise() */

extern int raise(int sig)
{
	return inner_raise(sig,FALSE,FALSE);
}

/*}}}*/
/*{{{  raise_sync() */

static void raise_sync(void)
{
	int olderrno = errno;
	int oldoserr = oserr;

	if (~mask.mask & mask.pending) {
		Wait(&siglock);
		if (~mask.mask & mask.pending)
			_raise_pending_sigs(FALSE,TRUE);
		Signal(&siglock);
	}

	errno = olderrno;
	oserr = oldoserr;
}

/*}}}*/
/*{{{  inner_raise() */

static int inner_raise(int sig, bool async,bool propagate)
{
	sigset_t sigbit;
	struct sigmask *m = &mask;
	
	if( sig < 0 || sig >= sizeof(sigset_t)*8) return -1;

	sigbit = 1L << sig;

	Wait(&siglock);
_Trace(0xCCCC0004,MyTask,sig|(async<<8)|(propagate<<16)|(sigchild<<24));

	/* If we are in a vfork(), do not deliver the signal, but simply*/
	/* set it pending for the parent. This means that signals will	*/
	/* not be delivered, or can be generated, between vfork and exec*/
	/* This does not seem a great problem given the other		*/
	/* restrictions imposed during this period.			*/
	
	if( sigchild != 0 ) m = &savemask;

	/* Unless the signal is to be ignored, set it pending. If this	*/
	/* signal was generated in a way which we want to propagate it	*/
	/* to any children, and it is propagatable(?), set the bit in	*/
	/* the propagate mask.						*/

	unless( m->ignore & sigbit ) m->pending |= sigbit;	
	
	if( propagate && (sigbit & DFL_PROPAGATE) ) m->propagate |= sigbit;

	/* If this is an alarm signal, abort any pause/sleep calls.	*/
	
	if( sig == SIGALRM )
	{
		SysTimeout = 0;
		if( PausePort != NullPort ) AbortPort(PausePort,123);
	}

	_raise_pending_sigs(async,TRUE);
	
 	_Trace(0xCCCC0014,MyTask,sig|(async<<8)|(propagate<<16)|(sigchild<<24));
	
	Signal(&siglock);

	return 0;
}

/*}}}*/
/*{{{  propogate_signals() */

static void propagate_signals(void)
{
	if ( sigchild != 0 ) return;
	
	/* If we are not in a vfork()/exec() sequence, and the	*/
	/* propagate bit is set, kill any children in the same	*/
	/* process group.					*/
	
	while( mask.propagate != 0 ) 
	{
		sigset_t sigbit = mask.propagate;
		int sig = -1;
		sigbit &= -sigbit;
		mask.propagate &= ~sigbit;
		while( sigbit ) sig++,sigbit>>=1;
		
		inner_kill(-childvec[0].pgrp,sig,TRUE);
	}
}

/*}}}*/
/*{{{  _raise_pending_sigs() */

/* _raise_pending_sigs gets control of the signal system and simply	*/
/* loops until all pending, unmasked signals are raised.		*/
static void _raise_pending_sigs(bool async, bool locked)
{
	sigset_t todo;
	bool oldAsync;

	unless( locked ) Wait(&siglock);

	/* If we are in a vfork/exec sequence, do not raise any */
	/* signals.						*/
	
	if( sigchild != 0 ) goto done;
	
	propagate_signals();
	
	/* Set Async TRUE here if we have been called from 	*/
	/* _posix_exception_handler, OR from an asynchronous	*/
	/* signal handler. This ensures that we do not think we	*/
	/* are in synchronous mode when in async mode.		*/

	oldAsync = Async;
	Async = async = async || oldAsync;

	/* pending is a mask of all pending signals which we	*/
	/* raised or want to raise. If it is non zero at the	*/
	/* end of the loop, we must abort all IO operations	*/

	if( Async && (~mask.mask & mask.pending) ) 
	{
_Trace(0xCCCC000b,MyTask,0);
		/* abort any I/O operations */
		abortfdv();
		
		/* abort any sleep or pause calls */
		if( PausePort != NullPort ) AbortPort(PausePort,123);
		AbortPort(WaitPort,123);
	}

	while( (todo = (~mask.mask & mask.pending & (async?mask.async:-1))) != 0 )
	{
		int sig;
		for( sig = 0; (todo & (1L << sig)) == 0; sig++);
		_raise(sig);
	}
	
	Async = oldAsync;

done:	
_Trace(0xCCCC001b,MyTask,0);
	unless( locked ) Signal(&siglock);
}

/*}}}*/
/*{{{  _raise() */

/* _raise causes the given signal routine to be executed.	*/
static void _raise(int sig)
{
	int setsig;
	void (*func)(int);	
	sigset_t sigbit;
	sigset_t oldmask;
#if defined(__TRAN) || defined(__C40)
	int stacksize = sigstacksize;
	Carrier c;
#endif

_Trace(0xCCCC0006,MyTask,sig);

	if( sig == SIGKILL ) _sigexit(sig);
	
	sigbit = 1L << sig;
	
	if( (mask.mask & sigbit) != 0 ) return;

	setsig = (sigactions[sig].sa_flags & SA_SETSIG) != 0;
	
	func = sigactions[sig].sa_handler;

#ifdef NOT_BSD_SIGNAL_BEHAVIOUR
	if( setsig ) 
	{
		sigactions[sig].sa_flags = 0;
		sigactions[sig].sa_handler = SIG_DFL;
	}
#endif	
	oldmask = mask.mask;
	
	mask.mask = oldmask | sigactions[sig].sa_mask | sigbit;
	
	mask.pending &= ~sigbit;		/* this signal no longer pending */

	/* call the action procedure with the signal lock off so that	*/
	/* new signals may arrive.					*/

	Signal( &siglock );

#if defined(__TRAN) || defined(__C40)
	if( (stacksize == 0) ||
	    ((c.Addr = CtoM_(Malloc(stacksize))) == NULL)) func(sig);
	else
	{
		c.Size = stacksize;
		Accelerate(&c,func,sizeof(sig),sig);
		Free(MtoC_(c.Addr));
	}
#else
	func(sig);	/* @@@ must re-introduce Accelerate */
			/* or compat. fn, so that we can swap stacks with ease */
#endif
	Wait( &siglock );
_Trace(0xCCCC000a,MyTask,sig);
	
	/* we finish by restoring the mask.mask to the original value */
	mask.mask = oldmask;
_Trace(0xCCCC0016,MyTask,sig);
}

/*}}}*/
/*{{{  setuptimeout() */

/* We are about to call a function which allows a timeout,see whether	*/
/* an alarm call has been made. If so, cancel the alarm and use the	*/
/* timeout on the call to advance time.					*/
#ifdef __TRAN
static
#endif
int setuptimeout()
{
	if( SysTimeout == 0 ) return -1;

	SysTimeout = alarm(0);

	return (int)(SysTimeout*OneSec);
}

/*}}}*/
/*{{{  resettimeout() */

/* After a timed call has returned, this will either reset the alarm	*/
/* to the residue of the time remaining, or will raise SIGALRM if the	*/
/* alotted period has expired.						*/
#ifdef __TRAN
static
#endif
void resettimeout(void)
{
	int rest;
	int now;
	
	if( SysTimeout == 0 ) return;
	
	now = GetDate();
	rest = AlarmSet+SysTimeout-now;

	if( rest > 0 ) alarm(rest);
	else 
	{
		raise(SIGALRM);
		errno = EINTR;
	}
}

/*}}}*/
/*{{{  alarm() */

extern unsigned int alarm(unsigned int sec)
{
	int left;
#ifdef SYSDEB
	word res;
#endif
	
	CHECKSIGS();
	
	if( SysTimeout == 0 ) left = 0;
	elif((left = GetDate()-AlarmSet) == 0) left = SysTimeout;
	else left = SysTimeout - left;

	AlarmTime = sec == 0 ? -1 : (int)(sec * OneSec);
	AlarmSet = GetDate();
	SysTimeout = sec;

#ifdef SYSDEB
	res = AbortPort(SignalPort,456);
	
	if (res != Err_Null)
	  IOdebug( "alarm: AbortPort failed, return code = %x", res );
#else
	AbortPort(SignalPort,456);
#endif
	
	return left;
}

/*}}}*/
/*{{{  pause() */

extern int pause(void)
{
	MCB m;

	CHECKSIGS();
	if( PausePort == NullPort ) PausePort = NewPort();
	
	InitMCB(&m,0,PausePort,NullPort,0);

	m.Timeout = -1;
	
	forever
	{
		word e = GetMsg(&m);
		if( e == 123 ) break;
		if( e == EK_Timeout ) continue;
		FreePort(PausePort);
		m.MsgHdr.Dest = PausePort = NewPort();
	}

	errno=EINTR; 
	CHECKSIGS();
	return -1;
}

/*}}}*/
/*{{{  sleep() */

extern unsigned int sleep(unsigned int seconds)
{
	time_t starttime, timeslept;
	struct sigaction *sleepact = NULL;
	struct sigaction *alarmact = NULL;

	int oldalarm, sleeptime;

	
	CHECKSIGS();

	oldalarm = alarm(0);

	/* if there is no current alarm, or the one that is present is	*/
	/* longer than we are sleeping for, ensure that the alarm signal*/
	/* we will cause is ignored.					*/
	
	if( oldalarm == 0 || oldalarm > seconds )
	{
		sleepact = (struct sigaction *)Malloc(sizeof(struct sigaction));
		alarmact = (struct sigaction *)Malloc(sizeof(struct sigaction));

		sleepact->sa_handler = SIG_IGN;
		sleepact->sa_mask = 0;
		sleepact->sa_flags = SA_SETSIG;
	
		sigaction(SIGALRM,sleepact,alarmact);
		
		sleeptime = seconds;
	}
	else sleeptime = oldalarm;
				
	alarm(sleeptime);

	starttime = time(NULL);

	pause();

	/* pause will return either as a result of the alarm signal or	*/
	/* because some other signal occurred. In both cases, we must	*/
	/* return.							*/
	
	timeslept = time(NULL)-starttime;

	/* if we saved an alarm call, restore its signal action here	*/
	
	if( alarmact != NULL )
	{
		sigaction(SIGALRM,alarmact,NULL);

		Free(sleepact);
		Free(alarmact);
	}
		
	if( timeslept < oldalarm ) { alarm(oldalarm-timeslept); return 0; }
	else alarm(0);
	
	if( timeslept < seconds ) return seconds-timeslept;
	else return 0;
}

/*}}}*/
/*{{{  _ignore_signal_handler() */

extern void _ignore_signal_handler(int sig)
{
    sig = sig;          /* reference it */
    return;
}

/*}}}*/
/*{{{  _default_signal_handler() */

extern void _default_signal_handler(int sig)
{
    _sigexit(0x80|sig);
}

/*}}}*/
/*{{{  _error_signal_marker() */

extern void _error_signal_marker(int sig)
/* This function should NEVER be called - its value is used as a marker     */
/* return from signal (SIG_ERR).   If someone manages to pass this          */
/* value back to signal and thence get it invoked we make it behave as      */
/* if signal got SIG_DFL:                                                   */
{
    _default_signal_handler(sig);
}

/*}}}*/
/*{{{  savesigmasks() */

void savesigmasks(void)
{
	Wait( &siglock );
	sigchild = inchild;
	savemask = mask;
	mask.pending = 0;
_Trace(0xCCCC0007,MyTask,sigchild);
	Signal( &siglock );
}

/*}}}*/
/*{{{  restoresigmasks() */

void restoresigmasks(void)
{
	Wait( &siglock );
_Trace(0xCCCC0008,MyTask,sigchild);
	sigchild = 0;
	mask = savemask;
	Signal( &siglock );	
}

/*}}}*/
/*{{{  tcgetpgrp() */

extern pid_t tcgetpgrp(int fd)
{
	fd = fd;
	CHECKSIGS();
	return childvec[sigchild].pgrp;
}

/*}}}*/
/*{{{  tcsetpgrp() */

/*
** BLV 15.6.93. The previous behaviour of tcsetpgrp() was unreliable. It
** installed a new event port, then sent a message to the posix exception
** handler which made the new event port the main signal port. Unfortunately
** the previous event port should be free by the server, causing the
** exception handler to receive an error and allocate a new signal port.
** Result: a race condition. There was a bug in the I/O Server which meant
** that the port was not actually freed and the race condition did not arise,
** but this has been fixed. The tty server was freeing the port, possibly
** explaining some of the funny ctrl-C behaviour when using it.
**
** The new scheme is as follows: tcsetpgrp() sends a message to the current
** event port containing the file descriptor, and it is the posix exception
** handler that is responsible for enabling the event. The event port is
** no longer the same as the signal handler port. Instead the posix exception
** handler makes use of MultiWait() to receive either events from the
** window server, signals from various sources, or messages from tcsetpgrp().
** It is important that tcsetpgrp() does not return until the event has
** been enabled, therefore it waits for a reply message back from the posix
** exception handler.
*/
extern int tcsetpgrp(int fd, int pid)
{
	fdentry	*f;
	MCB	 mcb;
	word	 e;
	Port	 tempport;
	
_Trace(0xCCCC0009,MyTask,fd|(pid<<16));	
	CHECKSIGS();

	if( (f = checkfd(fd)) == NULL ) return -1;

	unless( f->pstream->stream->Flags & Flags_Interactive )
	{
		errno = ENOTTY;
		return -1;
	}

	/* Note that AbortPort() may not be good enough. The exception handler	*/
	/* may still be processing some previous message, in which case calling	*/
	/* AbortPort() would have no effect. PutMsg() blocks until successful.	*/

	tempport		= NewPort();
	InitMCB(&mcb, 0, EventPort, tempport, 789);
	mcb.Data		= (BYTE *) &fd;
	mcb.MsgHdr.DataSize	= sizeof(int);

	e = PutMsg(&mcb);
	if (e < 0)
	{
#ifdef SYSDEB
		IOdebug("tcsetpgrp: failed to abort event port");
#endif
		FreePort(tempport);
		errno = EPERM;
		return(-1);
	}
	mcb.MsgHdr.Dest	= tempport;
	mcb.Timeout	= 20 * OneSec;
	e = GetMsg(&mcb);
	FreePort(tempport);
	if (e < 0)
	{
#ifdef SYSDEB
		IOdebug("setpgrp: received error %x back from exception handler", e);
#endif
		errno = posix_error(e);
		return(-1);
	}
	return(0);
}

/*}}}*/

/*}}}*/
		
/* end of signal.c */


