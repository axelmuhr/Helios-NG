-- File:	signal.m
-- Subsystem:	Helios AMPP macros
-- Author:	Paul Beskeen
-- Date:	Oct. '92
--
-- Description: Signal numbers derived from <signal.h> C header file.
--
-- WARNING:	This file should be kept up to date with <signal.h>.
--		Be aware that <signal.h> is itself generated automatically
--		from the master codes database.
--
--
-- RcsId: $Id: signal.m,v 1.1 1993/08/05 13:12:29 paul Exp $
--
-- (C) Copyright 1992 Perihelion Software Ltd.
--     All RIghts Reserved.


_report ['include signal.m]
_def 'signal.m_flag 1


struct sigaction [
        word	sa_handler		-- void (*sa_handler)();
        word	sa_mask			-- sigset_t (unsigned long)
	word	sa_flags		-- int
]


-- Flags
_if _not _defp '_POSIX_SOURCE [
	_def 'SA_SETSIG       1       -- handler set by signal()               
	_def 'SA_ASYNC        2       -- signal can be delivered asynchronously
]
_def 'SA_NOCLDSTOP	4       -- do not generate SIGCHLD               


-- Default signal handler names

_def 'SIG_IGN 		_ignore_signal_handler
_def 'SIG_DFL 		_default_signal_handler
_def 'SIG_ERR 		_error_signal_marker


-- Signal numbers

_def 'SIGZERO		0		-- no signal
_def 'SIGABRT		1		-- abort
_def 'SIGFPE		2		-- arithmetic exception
_def 'SIGILL		3		-- illegal instruction
_def 'SIGINT		4		-- attention from user
_def 'SIGSEGV		5		-- bad memory access
_def 'SIGTERM		6		-- termination request
_def 'SIGSTAK		7		-- stack overflow
_def 'SIGALRM		8		-- alarm/timeout signal
_def 'SIGHUP		9		-- hangup
_def 'SIGPIPE		10		-- pipe signal
_def 'SIGQUIT		11		-- interactive termination
_def 'SIGTRAP		12		-- trace trap
_def 'SIGUSR1		13		-- user 1
_def 'SIGUSR2		14		-- user 2
_def 'SIGCHLD		15		-- child termination
_def 'SIGURG		16		-- urgent data available
_def 'SIGCONT		17		-- continue
_def 'SIGSTOP		18		-- stop	
_def 'SIGTSTP		19		-- interactive stop
_def 'SIGTTIN		20		-- background read
_def 'SIGTTOU		21		-- background write
_def 'SIGWINCH		22		-- window changed
_def 'SIGSIB		23		-- sibling crashed
_def 'SIGKILL		31		-- termination

_if _not _defp '_POSIX_SOURCE [
	_def 'NSIG      32
]

-- codes for sigprocmask
_def 'SIG_BLOCK		1
_def 'SIG_UNBLOCK	2
_def 'SIG_SETMASK	3


_if _defp '_BSD [

	-- BSD compatability

	struct sigvec [
		word 	sv_handler	-- int (*sv_handler)();
		word	sv_mask		-- int
		word	sv_onstack	-- int
	]
	
	struct sigstack [
	        word	ss_sp		-- char *
	        word	ss_onstack	-- int
	]
	
	_def 'sigmask[x]	[(1<<(x)]
]



-- end of signal.m
