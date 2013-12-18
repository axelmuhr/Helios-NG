
/* signal.h: ANSI draft (May 88) library header, section 4.7 */
/* Copyright (C) Codemist Ltd. */
/* version 0.02 */

/*
 * signal.h declares a type and two functions and defines several macros, for
 * handling various signals (conditions that may be reported during program
 * execution).
 */

#ifndef __signal_h
#define __signal_h

typedef int sig_atomic_t;
   /* type which is the integral type of an object that can be modified as */
   /* an atomic entity, even in the presence of asynchronous interrupts. */

extern void __default_signal_handler(int);
extern void __error_signal_marker(int);
extern void __ignore_signal_handler(int);
   /*
    * Each of the following macros expand to distinct constant expressions that
    * have the same type as the second argument to and the return value of the
    * signal function, and whose value matches no declarable function.
    */
#define SIG_DFL __default_signal_handler
#define SIG_ERR __error_signal_marker
#define SIG_IGN __ignore_signal_handler

   /*
    * Each of the following macros expand to a positive integral constant
    * expression that is the signal number corresponding the the specified
    * condition.
    */
/* #define SIGSTAK 7   /* stack overflow                */
#define	SIGHUP	1	/* hangup */
#define	SIGINT	2	/* interrupt */
#define	SIGQUIT	3	/* quit */
#define SIGABRT 3       /* abort                         */
#define	SIGILL	4	/* illegal instruction (not reset when caught) */
#define	    ILL_RESAD_FAULT	0x0	/* reserved addressing fault */
#define	    ILL_PRIVIN_FAULT	0x1	/* privileged instruction fault */
#define	    ILL_RESOP_FAULT	0x2	/* reserved operand fault */
/* CHME, CHMS, CHMU are not yet given back to users reasonably */
#define	SIGTRAP	5	/* trace trap (not reset when caught) */
#define	SIGIOT	6	/* IOT instruction */
#define	SIGEMT	7	/* EMT instruction */
#define	SIGFPE	8	/* floating point exception */
#define     FPE_INTDIV_TRAP	(0x208>>3)	/* Integer division by zero */
#define     FPE_FLTOVF_TRAP	(0x1a0>>3)	/* Floating overflow trap */
#define     FPE_FLTUND_TRAP	(0x188>>3)	/* Floating underflow trap */
#define     FPE_FLTDIV_TRAP	(0x190>>3)	/* Floating division by zero */
#define     FPE_FLTIOP_TRAP	(0x1c0>>3)	/* Floating invalid operand */
#define     FPE_FLTINX_TRAP	(0x180>>3)	/* Floating inexact result */
#define	SIGKILL	9	/* kill (cannot be caught or ignored) */
#define	SIGBUS	10	/* bus error */
#define	SIGSEGV	11	/* segmentation violation */
#define	SIGSYS	12	/* bad argument to system call */
#define	SIGPIPE	13	/* write on a pipe with no one to read it */
#define	SIGALRM	14	/* alarm clock */
#define	SIGTERM	15	/* software termination signal from kill */
#define	SIGURG	16	/* urgent condition on IO channel */
#define	SIGSTOP	17	/* sendable stop signal not from tty */
#define	SIGTSTP	18	/* stop signal from tty */
#define	SIGCONT	19	/* continue a stopped process */
#define	SIGCHLD	20	/* to parent on child stop or exit */
#define	SIGTTIN	21	/* to readers pgrp upon background tty read */
#define	SIGTTOU	22	/* like TTIN for output if (tp->t_local&LTOSTOP) */
#define	SIGIO	23	/* input/output possible signal */
#define	SIGXCPU	24	/* exceeded CPU time limit */
#define	SIGXFSZ	25	/* exceeded file size limit */
#define	SIGVTALRM 26	/* virtual time alarm */
#define	SIGPROF	27	/* profiling time alarm */
#define	SIGWINCH 28	/* window changed */


extern void (*signal (int /*sig*/, void (* /*func*/ )(int)))(int);
   /*
    * chooses one of three ways in which receipt of the signal number sig is to
    * be subsequently handled. If the value of func is SIG_DFL, default
    * handling for that signal will occur. If the value of func is SIG_IGN, the
    * signal will be ignored. Otherwise func shall point to a functionto be
    * called when that signal occurs.
    * When a signal occurs, if func points to a function, first the equivalent
    * of signal(sig, SIG_DFL); is executed. (If the value of sig is SIGILL,
    * whether the reset to SIG_DFL occurs is implementation defined (under
    * Arthur/Brazil the reset does occur)). Next the equivalent of
    * (*func)(sig); is executed. The function may terminate by calling the
    * abort, exit or longjmp function. If func executes a return statement and
    * the value of sig was SIGFPE or any other implementation defined value
    * corresponding to a computational exception, the behaviour is undefined.
    * Otherwise, the program will resume execution at the point it was
    * interrupted.
    * If the signal occurs other than as a result of calling the abort or raise
    * function, the behaviour is undefined if the signal handler calls any
    * function in the standard library other than the signal function itself
    * or refers to any object with static storage duration other than by
    * assigning a value to a volatile static variable of type sig_atomic_t.
    * At program startup, the equivalent of signal(sig, SIG_IGN); may be
    * executed for some signals selected in an implementation defined manner
    * (under Arthur/Brazil this does not occur); the equivalent of
    * signal(sig, SIG_DFL); is executed for all other signals defined by the
    * implementation.
    * Returns: If the request can be honoured, the signal function returns the
    *          value of func for most recent call to signal for the specified
    *          signal sig. Otherwise, a value of SIG_ERR is returned and the
    *          integer expression errno is set to indicate the error.
    */
extern int raise(int /*sig*/);
   /* sends the signal sig to the executing program. */
   /* Returns: zero if successful, non-zero if unsuccessful. */

#endif

/* end of signal.h */
