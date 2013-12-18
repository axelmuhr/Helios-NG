#ifndef	NSIG
#define NSIG	32
#endif

typedef int sig_atomic_t;

#define	SIGHUP     1	/* hangup */
#define	SIGINT     2	/* interrupt */
#define	SIGQUIT    3	/* quit */
#define	SIGILL     4	/* illegal instruction (not reset when caught) */
#define	SIGTRAP    5	/* trace trap (not reset when caught) */
#define	SIGIOT     6	/* IOT instruction */
#define	SIGEMT     7	/* EMT instruction */
#define	SIGFPE     8	/* floating point exception */
#define	SIGKILL    9	/* kill (cannot be caught or ignored) */
#define	SIGBUS    10	/* bus error */
#define	SIGSEGV   11	/* segmentation violation */
#define	SIGSYS    12	/* bad argument to system call */
#define	SIGPIPE   13	/* write on a pipe with no one to read it */
#define	SIGALRM   14	/* alarm clock */
#define	SIGTERM   15	/* software termination signal from kill */
#define	SIGURG    16	/* urgent condition on IO channel */
#define	SIGSTOP   17	/* sendable stop signal not from tty */
#define	SIGTSTP   18	/* stop signal from tty */
#define	SIGCONT   19	/* continue a stopped process */
#define	SIGCHLD   20	/* to parent on child stop or exit */
#define	SIGTTIN   21	/* to readers pgrp upon background tty read */
#define	SIGTTOU   22	/* like TTIN for output if (tp->t_local&LTOSTOP) */
#define	SIGIO     23	/* input/output possible signal */
#define	SIGXCPU   24	/* exceeded CPU time limit */
#define	SIGXFSZ   25	/* exceeded file size limit */
#define	SIGVTALRM 26	/* virtual time alarm */
#define	SIGPROF   27	/* profiling time alarm */
#define SIGWINCH  28	/* window size changes */
#define SIGLOST	  29	/* Sys-V rec lock: notify user upon server crash */
#define SIGUSR1   30	/* User signal 1 (from SysV) */
#define SIGUSR2   31	/* User signal 2 (from SysV) */

#define SIGABRT SIGIOT  /* Added from BRL package for /usr/group. Signal
                         * returned by abort(3C).  Map onto SIGIOT.
                         */

#define	SIG_BLOCK	1		/* Add these signals to block mask	*/
#define	SIG_UNBLOCK	2		/* Remove these signals from block mask */
#define	SIG_SETMASK	3		/* Set block mask to this mask		*/
#define	BADSIG		((void (*)())(-1))
#define	SIG_ERR		((void (*)())(-1))
#define	SIG_DFL		((void (*)())( 0))
#define	SIG_IGN		((void (*)())( 1))

extern void (*signal(int sig, void (*func)(int)))(int);
extern int raise(int sig);

/* Not part of the standard but useful 
 * - taken from /usr/include/signal.h
 */
/*
 * Macro for converting signal number to a mask suitable for
 * sigblock().
 */
#define sigmask(m)	(1 << ((m)-1))
