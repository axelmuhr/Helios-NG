/*
 * $Header: stdio.h,v 1.4 88/10/29 01:55:25 keith Exp $
 * $Source: /zigguratusr/LCU/src/include/RCS/stdio.h,v $
 *
 * Copyright (c) 1988 Acorn Computers Ltd., Cambridge, England
 *
 * $Desc$
 * $Log:	stdio.h,v $
 * Revision 1.4  88/10/29  01:55:25  keith
 * Made this file usable on the vax by redefining putc()/getc() when not on th arm.
 * This makes those few cross compilation programs much easier to build.
 * 
 * Revision 1.3  88/06/19  15:22:48  beta
 * Acorn Unix initial beta version
 * 
 */
/*
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)stdio.h	5.3 (Berkeley) 3/15/86
 */
/*
 * ARM specific version,
 * Copyright (c) 1988 Acorn Computers Ltd
 *
 * Significant Changes:-
 *
 * 26-Feb-1988 JB	To take advantage of the ARM RISC architecture
 *			putc and getc are implemented as functions.
 */

# ifndef FILE
#define	BUFSIZ	1024
extern	struct	_iobuf {
	int	_cnt;
	char	*_ptr;		/* should be unsigned char */
	char	*_base;		/* ditto */
	int	_bufsiz;
	short	_flag;
	char	_file;		/* should be short */
} _iob[];

#define	_IOREAD	01
#define	_IOWRT	02
#define	_IONBF	04
#define	_IOMYBUF	010
#define	_IOEOF	020
#define	_IOERR	040
#define	_IOSTRG	0100
#define	_IOLBF	0200
#define	_IORW	0400
#define	NULL	0
#define	FILE	struct _iobuf
#define	EOF	(-1)

#define	stdin	(&_iob[0])
#define	stdout	(&_iob[1])
#define	stderr	(&_iob[2])

#ifndef arm
/* The old versions of getc and putc, as macros:- */
# ifndef lint
# define	getc(p)		(--(p)->_cnt>=0? (int)(*(unsigned char *)(p)->_ptr++):_filbuf(p))
# endif not lint
# ifndef lint
# define putc(x, p)	(--(p)->_cnt >= 0 ?\
	(int)(*(unsigned char *)(p)->_ptr++ = (x)) :\
	(((p)->_flag & _IOLBF) && -(p)->_cnt < (p)->_bufsiz ?\
		((*(p)->_ptr = (x)) != '\n' ?\
			(int)(*(unsigned char *)(p)->_ptr++) :\
			_flsbuf(*(unsigned char *)(p)->_ptr, p)) :\
		_flsbuf((unsigned char)(x), p)))
# endif not lint
#else
/* ARM specific functions:- */
int getc(/*FILE *p*/);
int putc(/*char x, FILE *p*/);
#endif

#define	getchar()	getc(stdin)
#define	putchar(x)	putc(x,stdout)
#define	feof(p)		(((p)->_flag&_IOEOF)!=0)
#define	ferror(p)	(((p)->_flag&_IOERR)!=0)
#define	fileno(p)	((p)->_file)
#define	clearerr(p)	((p)->_flag &= ~(_IOERR|_IOEOF))

FILE	*fopen();
FILE	*fdopen();
FILE	*freopen();
FILE	*popen();
long	ftell();
char	*fgets();
char	*gets();
#ifdef vax
char	*sprintf();		/* too painful to do right */
#endif
# endif
/*
 * $Header: ctype.h,v 1.3 88/06/19 15:20:31 beta Exp $
 * $Source: /zigguratusr/LCU/src/include/RCS/ctype.h,v $
 *
 * Copyright (c) 1988 Acorn Computers Ltd., Cambridge, England
 *
 * $Desc$
 * $Log:	ctype.h,v $
 * Revision 1.3  88/06/19  15:20:31  beta
 * Acorn Unix initial beta version
 * 
 */
/*	ctype.h	4.2	85/09/04	*/

#define	_U	01
#define	_L	02
#define	_N	04
#define	_S	010
#define _P	020
#define _C	040
#define _X	0100
#define	_B	0200

extern	char	_ctype_[];

#define	isalpha(c)	((_ctype_+1)[c]&(_U|_L))
#define	isupper(c)	((_ctype_+1)[c]&_U)
#define	islower(c)	((_ctype_+1)[c]&_L)
#define	isdigit(c)	((_ctype_+1)[c]&_N)
#define	isxdigit(c)	((_ctype_+1)[c]&(_N|_X))
#define	isspace(c)	((_ctype_+1)[c]&_S)
#define ispunct(c)	((_ctype_+1)[c]&_P)
#define isalnum(c)	((_ctype_+1)[c]&(_U|_L|_N))
#define isprint(c)	((_ctype_+1)[c]&(_P|_U|_L|_N|_B))
#define isgraph(c)	((_ctype_+1)[c]&(_P|_U|_L|_N))
#define iscntrl(c)	((_ctype_+1)[c]&_C)
#define isascii(c)	((unsigned)(c)<=0177)
#define toupper(c)	((c)-'a'+'A')
#define tolower(c)	((c)-'A'+'a')
#define toascii(c)	((c)&0177)
/*
 * $Header: setjmp.h,v 1.12 88/12/03 16:56:42 john Exp $
 * $Source: /zigguratusr/LCU/src/include/RCS/setjmp.h,v $
 *
 * Copyright (c) 1988 Acorn Computers Ltd., Cambridge, England
 *
 * $Desc$
 * $Log:	setjmp.h,v $
 * Revision 1.12  88/12/03  16:56:42  john
 * Corrected typo in the definition of JMPBUF_FPS
 * 
 * Revision 1.11  88/12/03  15:51:11  john
 * Changed a jmp_buf back to hold a complete sigcontext.  This allows
 * both methods of working if desired (stack unwinding and simple
 * sigreturn restoration.)
 * 
 * Revision 1.10  88/12/01  19:33:31  john
 * Added a #define of _setjmp to setjmp for the Norcroft
 * compiler - the internal implementation of _setjmp is
 * identical to that of setjmp anyway.  This change ensures
 * that Norcroft correctly handles the wierd control flow
 * which setjmp produces.
 * 
 * Revision 1.9  88/11/30  19:25:54  john
 * Reverted to LDS compatibility mode - basically store the sl
 * value after the rest.
 * 
 * Revision 1.8  88/11/30  18:49:17  john
 * Added slot for floating point status register.
 * 
 * Revision 1.7  88/11/30  17:57:34  john
 * Corrected a typo in the type name (jmpbuf instead of jmp_buf.)
 * 
 * Revision 1.6  88/11/30  17:52:08  john
 * Changed for new (stack unwinding) version of setjmp
 * 
 * Revision 1.5  88/08/22  12:18:17  keith
 * Whoops, structs are different from arrays when
 * passed as parameters to procedures. jmp_buf must be
 * an array to get the implicit call by reference.
 * 
 * Revision 1.4  88/08/08  16:00:52  keith
 * jmp_buf is now typedef'd to a sigcontext (from <sys/signal.h> )
 * 
 * Revision 1.3  88/06/19  15:22:35  beta
 * Acorn Unix initial beta version
 * 
 */
/*	setjmp.h	4.1	83/05/03	*/

#include <sys/signal.h>
/*
 * The numbers of the cpu registers corresponding to various functions.
 * <arm/reg.h> can also be used, but this defines symbols which may
 * clash with those used in user programmes.
 */
#define _REGISTER_SL	10
#define _REGISTER_FP	11
#define _REGISTER_IP	12
#define _REGISTER_SP	13
#define _REGISTER_LR	14
#define _REGISTER_PC	15

/*
 * Macros for accessing the jmp_buf array - which is acutally a
 * struct sigcontext
 */
#define _JMPBUF_INDEX(f)	(((int)&((struct sigcontext *)0)->f)/sizeof (int))
#define _JMPBUF_CPUREGINDEX(r)	_JMPBUF_INDEX(sc_cpu_regs[r])
#define _JMPBUF_FPREGINDEX(r)	_JMPBUF_INDEX(sc_fp_regs.fp_reg[r])

/*
 * And indices of useful fields:-
 */
#define JMPBUF_SS_ONSTACK	_JMPBUF_INDEX(sc_onstack)
#define JMPBUF_SS_MASK		_JMPBUF_INDEX(sc_mask)

#define JMPBUF_SL		_JMPBUF_CPUREGINDEX(_REGISTER_SL)
#define JMPBUF_FP		_JMPBUF_CPUREGINDEX(_REGISTER_FP)
#define JMPBUF_IP		_JMPBUF_CPUREGINDEX(_REGISTER_IP)
#define JMPBUF_SP		_JMPBUF_CPUREGINDEX(_REGISTER_SP)
#define JMPBUF_LR		_JMPBUF_CPUREGINDEX(_REGISTER_LR)
#define JMPBUF_PC		_JMPBUF_CPUREGINDEX(_REGISTER_PC)

#define JMPBUF_FPS		_JMPBUF_INDEX(sc_fp_regs.fp_status)

#define JMPBUF_SIZE		((sizeof (struct sigcontext) + sizeof (int) - 1)/sizeof (int))

typedef int jmp_buf[JMPBUF_SIZE];
/*
 * $Header: sgtty.h,v 1.3 88/06/19 15:22:40 beta Exp $
 * $Source: /zigguratusr/LCU/src/include/RCS/sgtty.h,v $
 *
 * Copyright (c) 1988 Acorn Computers Ltd., Cambridge, England
 *
 * $Desc$
 * $Log:	sgtty.h,v $
 * Revision 1.3  88/06/19  15:22:40  beta
 * Acorn Unix initial beta version
 * 
 */
/*	sgtty.h	4.2	85/01/03	*/

#ifndef	_IOCTL_
#include <sys/ioctl.h>
#endif
/*
 * $Header: signal.h,v 1.4 88/08/11 09:27:39 keith Exp $
 * $Source: /zigguratusr/LCU/src/sys/h/RCS/signal.h,v $
 *
 * Copyright (c) 1988 Acorn Computers Ltd., Cambridge, England
 *
 * $Desc$
 * $Log:	signal.h,v $
 * Revision 1.4  88/08/11  09:27:39  keith
 * Addition of two kernel private psuedo-signals for FP context switching.
 * Also #ifdef _SIGNAL_ to allow multiple inclusions.
 * 
 * Revision 1.3  88/06/17  20:21:05  beta
 * Acorn Unix initial beta version
 * 
 */
/* @(#)signal.h	1.3 87/05/29 3.2/4.3NFSSRC */
/*
 * Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)signal.h	7.1 (Berkeley) 6/4/86
 */

#ifndef	_SIGNAL_
#define	_SIGNAL_

#ifndef	NSIG
#define NSIG	32

#define	SIGHUP	1	/* hangup */
#define	SIGINT	2	/* interrupt */
#define	SIGQUIT	3	/* quit */
#define	SIGILL	4	/* illegal instruction (not reset when caught) */
#define	    ILL_RESAD_FAULT	0x0	/* reserved addressing fault */
#define	    ILL_PRIVIN_FAULT	0x1	/* privileged instruction fault */
#define	    ILL_RESOP_FAULT	0x2	/* reserved operand fault */
/* CHME, CHMS, CHMU are not yet given back to users reasonably */
#define	SIGTRAP	5	/* trace trap (not reset when caught) */
#define	SIGIOT	6	/* IOT instruction */
#define	SIGABRT	SIGIOT	/* compatibility */
#define	SIGEMT	7	/* EMT instruction */
#define	SIGFPE	8	/* floating point exception */
#define	    FPE_INTOVF_TRAP	0x1	/* integer overflow */
#define	    FPE_INTDIV_TRAP	0x2	/* integer divide by zero */
#define	    FPE_FLTOVF_TRAP	0x3	/* floating overflow */
#define	    FPE_FLTDIV_TRAP	0x4	/* floating/decimal divide by zero */
#define	    FPE_FLTUND_TRAP	0x5	/* floating underflow */
#define	    FPE_DECOVF_TRAP	0x6	/* decimal overflow */
#define	    FPE_SUBRNG_TRAP	0x7	/* subscript out of range */
#define	    FPE_FLTOVF_FAULT	0x8	/* floating overflow fault */
#define	    FPE_FLTDIV_FAULT	0x9	/* divide by zero floating fault */
#define	    FPE_FLTUND_FAULT	0xa	/* floating underflow fault */
#define	    FPE_INVALID_OP      0xb     /* floating invalid operand */
#define     FPE_INEXACT         0xc	/* floating inexactitude! */
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
#define	SIGCLD	SIGCHLD	/* compatibility */
#define	SIGTTIN	21	/* to readers pgrp upon background tty read */
#define	SIGTTOU	22	/* like TTIN for output if (tp->t_local&LTOSTOP) */
#define	SIGIO	23	/* input/output possible signal */
#define	SIGXCPU	24	/* exceeded CPU time limit */
#define	SIGXFSZ	25	/* exceeded file size limit */
#define	SIGVTALRM 26	/* virtual time alarm */
#define	SIGPROF	27	/* profiling time alarm */
#define SIGWINCH 28	/* window size changes */
#define SIGUSR1 30	/* user defined signal 1 */
#define SIGUSR2 31	/* user defined signal 2 */
#define	SIGLOST 29	/* resource lost (eg, record-lock lost) */

/* The fp context changer uses two psuedo signals to invoke trampoline
 * code to save/load floating point registers which cannot be accessed
 * in supervisor mode.
 */
#ifdef	KERNEL
#define	SIG_SAVE_FPREGS	NSIG+1
#define	SIG_LOAD_FPREGS NSIG+2
#endif	KERNEL


#ifndef KERNEL
int	(*signal())();
#endif

/*
 * Signal vector "template" used in sigvec call.
 */
struct	sigvec {
	int	(*sv_handler)();	/* signal handler */
	int	sv_mask;		/* signal mask to apply */
	int	sv_flags;		/* see signal options below */
};
#define SV_ONSTACK	0x0001	/* take signal on signal stack */
#define SV_INTERRUPT	0x0002	/* do not restart system on signal return */
#define sv_onstack sv_flags	/* isn't compatibility wonderful! */

/*
 * Structure used in sigstack call.
 */
struct	sigstack {
	char	*ss_sp;			/* signal stack pointer */
	int	ss_onstack;		/* current status */
};


#ifdef	KERNEL
#include "../arm/fp.h"
#else
#include <arm/fp.h>
#endif	KERNEL

/*
 * Information pushed on stack when a signal is delivered.
 * This is used by the kernel to restore state following
 * execution of the signal handler.  It is also made available
 * to the handler to allow it to properly restore state if
 * a non-standard exit is performed.
 */
struct	sigcontext {
	int	sc_onstack;		/* sigstack state to restore */
	int	sc_mask;		/* signal mask to restore */
	int	sc_cpu_regs[16];	/* ARM cpu registers to restore */
	struct	fp_regs sc_fp_regs;	/* floating point register to restore */
};

struct	syslongjmpcontext {
	int	sc_onstack;		/* sigstack state to restore */
	int	sc_mask;		/* signal mask to restore */
	int	sc_r4, sc_r5, sc_r6;	/* r4-r6 to restore */
	int	sc_r7, sc_r8, sc_r9;	/* r7-r9 to restore */
	int	sc_sb;			/* static base to restore */
	int	sc_fp;			/* fp to restore */
	int	sc_spt;			/* sp temporary to restore */
	int	sc_sp;			/* sp to restore */
	int	sc_lr;			/* lr to restore */
	int	sc_pc;			/* pc to restore */
};

#define	BADSIG		(int (*)())-1
#define	SIG_DFL		(int (*)())0
#define	SIG_IGN		(int (*)())1

#ifdef KERNEL
#define	SIG_CATCH	(int (*)())2
#define	SIG_HOLD	(int (*)())3
#endif
#endif

/*
 * Macro for converting signal number to a mask suitable for
 * sigblock().
 */
#define sigmask(m)	(1 << ((m)-1))
#endif	_SIGNAL_
/*
 * $Header: fcntl.h,v 1.3 88/06/17 20:19:13 beta Beta1 $
 * $Source: /zigguratusr/LCU/src/sys/h/RCS/fcntl.h,v $
 *
 * Copyright (c) 1988 Acorn Computers Ltd., Cambridge, England
 *
 * $Desc$
 * $Log:	fcntl.h,v $
 * Revision 1.3  88/06/17  20:19:13  beta
 * Acorn Unix initial beta version
 * 
 */
/* @(#)fcntl.h	1.2 87/08/26 3.2/4.3NFSSRC */
/* @(#)fcntl.h	1.2 86/12/15 NFSSRC */
/*	@(#)fcntl.h 1.1 86/09/25 SMI; from UCB 5.1 85/05/30	*/
/*
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#ifndef __FCNTL_HEADER__
#define __FCNTL_HEADER__

/*
 * Flag values accessible to open(2) and fcntl(2)
 *  (The first three can only be set by open)
 */
#define	O_RDONLY	0
#define	O_WRONLY	1
#define	O_RDWR		2
#define	O_NDELAY	FNDELAY	/* Non-blocking I/O */
#define	O_APPEND	FAPPEND	/* append (writes guaranteed at the end) */
#define	O_CREAT		FCREAT	/* open with file create */
#define	O_TRUNC		FTRUNC	/* open with truncation */
#define	O_EXCL		FEXCL	/* error on create if file exists */

/* flags for F_GETFL, F_SETFL-- needed by <sys/file.h> */
#define	FNDELAY		00004		/* non-blocking reads */
#define	FAPPEND		00010		/* append on each write */
#define	FASYNC		00100		/* signal pgrp when data ready */
#define	FCREAT		01000		/* create if nonexistant */
#define	FTRUNC		02000		/* truncate to zero length */
#define	FEXCL		04000		/* error if already created */

/* fcntl(2) requests */
#define	F_DUPFD	0	/* Duplicate fildes */
#define	F_GETFD	1	/* Get fildes flags */
#define	F_SETFD	2	/* Set fildes flags */
#define	F_GETFL	3	/* Get file flags */
#define	F_SETFL	4	/* Set file flags */
#define	F_GETOWN 5	/* Get owner */
#define F_SETOWN 6	/* Set owner */
#define F_GETLK  7      /* Get record-locking information */
#define F_SETLK  8      /* Set or Clear a record-lock (Non-Blocking) */
#define F_SETLKW 9      /* Set or Clear a record-lock (Blocking) */

/* access(2) requests */
#define	F_OK		0	/* does file exist */
#define	X_OK		1	/* is it executable by caller */
#define	W_OK		2	/* writable by caller */
#define	R_OK		4	/* readable by caller */

/* System-V record-locking options */
/* lockf(2) requests */
#define F_ULOCK 0       /* Unlock a previously locked region */
#define F_LOCK  1       /* Lock a region for exclusive use */ 
#define F_TLOCK 2       /* Test and lock a region for exclusive use */
#define F_TEST  3       /* Test a region for other processes locks */

/* fcntl(2) flags (l_type field of flock structure) */
#define F_RDLCK 1       /* read lock */
#define F_WRLCK 2       /* write lock */
#define F_UNLCK 3       /* remove lock(s) */


/* file segment locking set data type - information passed to system by user */
struct flock {
        short   l_type;		/* F_RDLCK, F_WRLCK, or F_UNLCK */
        short   l_whence;	/* flag to choose starting offset */
        long    l_start;	/* relative offset, in bytes */
        long    l_len;          /* length, in bytes; 0 means lock to EOF */
        short   l_pid;		/* returned with F_GETLK */
        short   l_xxx;		/* reserved for future use */
};

#endif !__FCNTL_HEADER__
/*
 * $Header: time.h,v 1.4 88/10/11 10:26:26 keith Exp $
 * $Source: /zigguratusr/LCU/src/sys/h/RCS/time.h,v $
 *
 * Copyright (c) 1988 Acorn Computers Ltd., Cambridge, England
 *
 * $Desc$
 * $Log:	time.h,v $
 * Revision 1.4  88/10/11  10:26:26  keith
 * Protect <sys/time.h> against multiple inclusion.
 * 
 * Revision 1.3  88/06/17  20:21:51  beta
 * Acorn Unix initial beta version
 * 
 */
/* @(#)time.h	1.2 87/05/15 3.2/4.3NFSSRC */
/*
 * Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)time.h	7.1 (Berkeley) 6/4/86
 */

#ifndef __SYS_TIME__
#define __SYS_TIME__
/*
 * Structure returned by gettimeofday(2) system call,
 * and used in other calls.
 */
struct timeval {
	long	tv_sec;		/* seconds */
	long	tv_usec;	/* and microseconds */
};

struct timezone {
	int	tz_minuteswest;	/* minutes west of Greenwich */
	int	tz_dsttime;	/* type of dst correction */
};
#define	DST_NONE	0	/* not on dst */
#define	DST_USA		1	/* USA style dst */
#define	DST_AUST	2	/* Australian style dst */
#define	DST_WET		3	/* Western European dst */
#define	DST_MET		4	/* Middle European dst */
#define	DST_EET		5	/* Eastern European dst */
#define	DST_CAN		6	/* Canada */

/*
 * Operations on timevals.
 *
 * NB: timercmp does not work for >= or <=.
 */
#define	timerisset(tvp)		((tvp)->tv_sec || (tvp)->tv_usec)
#define	timercmp(tvp, uvp, cmp)	\
	((tvp)->tv_sec cmp (uvp)->tv_sec || \
	 (tvp)->tv_sec == (uvp)->tv_sec && (tvp)->tv_usec cmp (uvp)->tv_usec)
#define	timerclear(tvp)		(tvp)->tv_sec = (tvp)->tv_usec = 0

/*
 * Names of the interval timers, and structure
 * defining a timer setting.
 */
#define	ITIMER_REAL	0
#define	ITIMER_VIRTUAL	1
#define	ITIMER_PROF	2

struct	itimerval {
	struct	timeval it_interval;	/* timer interval */
	struct	timeval it_value;	/* current value */
};

#ifndef KERNEL
#include <time.h>
#endif
#endif !__SYS_TIME__
/*
 * $Header: types.h,v 1.3 88/06/17 20:22:16 beta Beta1 $
 * $Source: /zigguratusr/LCU/src/sys/h/RCS/types.h,v $
 *
 * Copyright (c) 1988 Acorn Computers Ltd., Cambridge, England
 *
 * $Desc$
 * $Log:	types.h,v $
 * Revision 1.3  88/06/17  20:22:16  beta
 * Acorn Unix initial beta version
 * 
 */
/* @(#)types.h	1.4 87/09/03 3.2/4.3NFSSRC */
/*
 * Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)types.h	7.1 (Berkeley) 6/4/86
 */

#ifndef _TYPES_
#define	_TYPES_
/*
 * Basic system types and major/minor device constructing/busting macros.
 */

/* major part of a device */
#define	major(x)	((int)(((unsigned)(x)>>8)&0377))

/* minor part of a device */
#define	minor(x)	((int)((x)&0377))

/* make a device number */
#define	makedev(x,y)	((dev_t)(((x)<<8) | (y)))

typedef	unsigned char	u_char, u_byte;
typedef	unsigned short	u_short;
typedef	unsigned int	u_int;
typedef	unsigned long	u_long;
typedef	unsigned short	ushort;		/* sys III compat */

#ifdef vax
typedef	struct	_physadr { int r[1]; } *physadr;
typedef	struct	label_t	{
	int	val[14];
} label_t;
#endif vax

#ifdef arm
typedef	struct	_physadr { int r[1]; } *physadr;
typedef struct label_t {
	int	val[16];		/* bigger than need be, so what */
} label_t;
#endif arm

typedef	struct	_quad { long val[2]; } quad;
typedef	long	daddr_t;
typedef	char *	caddr_t;
typedef	u_long	ino_t;
typedef	long	swblk_t;
typedef	long	size_t;
typedef	long	time_t;
typedef	short	dev_t;
typedef	long	off_t;
typedef	u_short	uid_t;
typedef	u_short	gid_t;
typedef int	key_t;		/* For System V IPC calls */

#define	NBBY	8		/* number of bits in a byte */
/*
 * Select uses bit masks of file descriptors in longs.
 * These macros manipulate such bit fields (the filesystem macros use chars).
 * FD_SETSIZE may be defined by the user, but the default here
 * should be >= NOFILE (param.h).
 */
#ifndef	FD_SETSIZE
#define	FD_SETSIZE	256
#endif

typedef long	fd_mask;
#define NFDBITS	(sizeof(fd_mask) * NBBY)	/* bits per mask */
#ifndef howmany
#define	howmany(x, y)	(((x)+((y)-1))/(y))
#endif

typedef	struct fd_set {
	fd_mask	fds_bits[howmany(FD_SETSIZE, NFDBITS)];
} fd_set;

#define	FD_SET(n, p)	((p)->fds_bits[(n)/NFDBITS] |= (1 << ((n) % NFDBITS)))
#define	FD_CLR(n, p)	((p)->fds_bits[(n)/NFDBITS] &= ~(1 << ((n) % NFDBITS)))
#define	FD_ISSET(n, p)	((p)->fds_bits[(n)/NFDBITS] & (1 << ((n) % NFDBITS)))
#define FD_ZERO(p)	bzero((char *)(p), sizeof(*(p)))

#endif
/*
 * $Header: dir.h,v 1.3 88/06/17 20:18:36 beta Beta1 $
 * $Source: /zigguratusr/LCU/src/sys/h/RCS/dir.h,v $
 *
 * Copyright (c) 1988 Acorn Computers Ltd., Cambridge, England
 *
 * $Desc$
 * $Log:	dir.h,v $
 * Revision 1.3  88/06/17  20:18:36  beta
 * Acorn Unix initial beta version
 * 
 */
/* @(#)dir.h	1.4 87/06/02 3.2/4.3NFSSRC */
/*
 * Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)dir.h	7.1 (Berkeley) 6/4/86
 */

/*
 * A directory consists of some number of blocks of DIRBLKSIZ
 * bytes, where DIRBLKSIZ is chosen such that it can be transferred
 * to disk in a single atomic operation (e.g. 512 bytes on most machines).
 *
 * Each DIRBLKSIZ byte block contains some number of directory entry
 * structures, which are of variable length.  Each directory entry has
 * a struct direct at the front of it, containing its inode number,
 * the length of the entry, and the length of the name contained in
 * the entry.  These are followed by the name padded to a 4 byte boundary
 * with null bytes.  All names are guaranteed null terminated.
 * The maximum length of a name in a directory is MAXNAMLEN.
 *
 * The macro DIRSIZ(dp) gives the amount of space required to represent
 * a directory entry.  Free space in a directory is represented by
 * entries which have dp->d_reclen > DIRSIZ(dp).  All DIRBLKSIZ bytes
 * in a directory block are claimed by the directory entries.  This
 * usually results in the last entry in a directory having a large
 * dp->d_reclen.  When entries are deleted from a directory, the
 * space is returned to the previous entry in the same directory
 * block by increasing its dp->d_reclen.  If the first entry of
 * a directory block is free, then its dp->d_ino is set to 0.
 * Entries other than the first in a directory do not normally have
 * dp->d_ino set to 0.
 */
/* so user programs can just include dir.h */
#if !defined(KERNEL) && !defined(DEV_BSIZE)
#define	DEV_BSIZE	512
#endif
#define DIRBLKSIZ	DEV_BSIZE
#define	MAXNAMLEN	255

	/*  nfs_xdr.c uses d_fileno  */
struct	direct {
        u_long  d_fileno;               /* file number of entry */
	u_short	d_reclen;		/* length of this record */
	u_short	d_namlen;		/* length of string in d_name */
	char	d_name[MAXNAMLEN + 1];	/* name must be no longer than this */
};

#ifndef KERNEL
#define d_ino   d_fileno                /* compatablity */

/*
 * The DIRSIZ macro gives the minimum record length which will hold
 * the directory entry.  This requires the amount of space in struct direct
 * without the d_name field, plus enough space for the name with a terminating
 * null byte (dp->d_namlen+1), rounded up to a 4 byte boundary.
 */
#undef DIRSIZ
#define DIRSIZ(dp) \
    ((sizeof (struct direct) - (MAXNAMLEN+1)) + (((dp)->d_namlen+1 + 3) &~ 3))

/*
 * Definitions for library routines operating on directories.
 */
typedef struct _dirdesc {
	int	dd_fd;
	long	dd_loc;
	long	dd_size;
	long	dd_bbase;
	long	dd_entno;
	long	dd_bsize;
	char	*dd_buf;
} DIR;
#ifndef NULL
#define NULL 0
#endif
extern	DIR *opendir();
extern	struct direct *readdir();
extern	long telldir();
extern	void seekdir();
#define rewinddir(dirp)	seekdir((dirp), (long)0)
extern	void closedir();
#endif

#ifdef KERNEL
/*
 * Template for manipulating directories.
 * Should use struct direct's, but the name field
 * is MAXNAMLEN - 1, and this just won't do.
 */
struct dirtemplate {
	u_long	dot_ino;
	short	dot_reclen;
	short	dot_namlen;
	char	dot_name[4];		/* must be multiple of 4 */
	u_long	dotdot_ino;
	short	dotdot_reclen;
	short	dotdot_namlen;
	char	dotdot_name[4];		/* ditto */
};
#endif
/*
 * $Header: stat.h,v 1.3 88/06/17 20:21:18 beta Beta1 $
 * $Source: /zigguratusr/LCU/src/sys/h/RCS/stat.h,v $
 *
 * Copyright (c) 1988 Acorn Computers Ltd., Cambridge, England
 *
 * $Desc$
 * $Log:	stat.h,v $
 * Revision 1.3  88/06/17  20:21:18  beta
 * Acorn Unix initial beta version
 * 
 */
/* @(#)stat.h	1.4 87/09/09 3.2/4.3NFSSRC */
/*
 * Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)stat.h	7.1 (Berkeley) 6/4/86
 */

struct	stat
{
	dev_t	st_dev;
	ino_t	st_ino;
	unsigned short st_mode;
	short	st_nlink;
	uid_t	st_uid;
	gid_t	st_gid;
	dev_t	st_rdev;
	off_t	st_size;
	time_t	st_atime;
	int	st_spare1;
	time_t	st_mtime;
	int	st_spare2;
	time_t	st_ctime;
	int	st_spare3;
	long	st_blksize;
	long	st_blocks;
	long	st_spare4[2];
};

#define	S_IFMT	0170000		/* type of file */
#define		S_IFDIR	0040000	/* directory */
#define		S_IFCHR	0020000	/* character special */
#define		S_IFBLK	0060000	/* block special */
#define		S_IFREG	0100000	/* regular */
#define		S_IFLNK	0120000	/* symbolic link */
#define		S_IFSOCK 0140000/* socket */
#define         S_IFIFO 0010000 /* fifo */
#define	S_ISUID	0004000		/* set user id on execution */
#define	S_ISGID	0002000		/* set group id on execution */
#define	S_ISVTX	0001000		/* save swapped text even after use */
#define	S_IREAD	0000400		/* read permission, owner */
#define	S_IWRITE 0000200	/* write permission, owner */
#define	S_IEXEC	0000100		/* execute/search permission, owner */
/*
 * $Header: vfs.h,v 1.3 88/06/17 20:22:43 beta Beta1 $
 * $Source: /zigguratusr/LCU/src/sys/h/RCS/vfs.h,v $
 *
 * Copyright (c) 1988 Acorn Computers Ltd., Cambridge, England
 *
 * $Desc$
 * $Log:	vfs.h,v $
 * Revision 1.3  88/06/17  20:22:43  beta
 * Acorn Unix initial beta version
 * 
 */
/* @(#)vfs.h	1.2 87/09/09 3.2/4.3NFSSRC */
/*	@(#)vfs.h 1.1 86/09/25 SMI	*/

/*
 * File system identifier. Should be unique (at least per machine).
 */
typedef struct {
	long val[2];			/* file system id type */
} fsid_t;

/*
 * File identifier. Should be unique per filesystem on a single machine.
 */
#define	MAXFIDSZ	16
#define	freefid(fidp) \
    kmem_free((caddr_t)(fidp), sizeof (struct fid) - MAXFIDSZ + (fidp)->fid_len)

struct fid {
	u_short		fid_len;		/* length of data in bytes */
	char		fid_data[MAXFIDSZ];	/* data (variable length) */
};

/*
 * Structure per mounted file system.
 * Each mounted file system has an array of
 * operations and an instance record.
 * The file systems are put on a singly linked list.
 */
struct vfs {
	struct vfs	*vfs_next;		/* next vfs in vfs list */
	struct vfsops	*vfs_op;		/* operations on vfs */
	struct vnode	*vfs_vnodecovered;	/* vnode we mounted on */
	int		vfs_flag;		/* flags */
	int		vfs_bsize;		/* native block size */
	fsid_t		vfs_fsid;		/* file system id */
	uid_t		vfs_exroot;		/* exported fs uid 0 mapping */
	short		vfs_exflags;		/* exported fs flags */
	caddr_t		vfs_data;		/* private data */
};

/*
 * vfs flags.
 * VFS_MLOCK lock the vfs so that name lookup cannot proceed past the vfs.
 * This keeps the subtree stable during mounts and unmounts.
 */
#define VFS_RDONLY	0x01		/* read only vfs */
#define VFS_MLOCK	0x02		/* lock vfs so that subtree is stable */
#define VFS_MWAIT	0x04		/* someone is waiting for lock */
#define VFS_NOSUID	0x08		/* someone is waiting for lock */
#define	VFS_EXPORTED	0x10		/* file system is exported (NFS) */

/*
 * exported vfs flags.
 */
#define	EX_RDONLY	0x01		/* exported read only */

/*
 * Operations supported on virtual file system.
 */
struct vfsops {
	int	(*vfs_mount)();		/* mount file system */
	int	(*vfs_unmount)();	/* unmount file system */
	int	(*vfs_root)();		/* get root vnode */
	int	(*vfs_statfs)();	/* get fs statistics */
	int	(*vfs_sync)();		/* flush fs buffers */
	int	(*vfs_vget)();		/* get vnode from fid */
};

#define VFS_MOUNT(VFSP, PATH, DATA) \
				 (*(VFSP)->vfs_op->vfs_mount)(VFSP, PATH, DATA)
#define VFS_UNMOUNT(VFSP)	 (*(VFSP)->vfs_op->vfs_unmount)(VFSP)
#define VFS_ROOT(VFSP, VPP)	 (*(VFSP)->vfs_op->vfs_root)(VFSP,VPP)
#define VFS_STATFS(VFSP, SBP)	 (*(VFSP)->vfs_op->vfs_statfs)(VFSP,SBP)
#define VFS_SYNC(VFSP)		 (*(VFSP)->vfs_op->vfs_sync)(VFSP)
#define VFS_VGET(VFSP, VPP, FIDP) (*(VFSP)->vfs_op->vfs_vget)(VFSP, VPP, FIDP)

/*
 * file system statistics
 */
struct statfs {
	long f_type;			/* type of info, zero for now */
	long f_bsize;			/* fundamental file system block size */
	long f_blocks;			/* total blocks in file system */
	long f_bfree;			/* free block in fs */
	long f_bavail;			/* free blocks avail to non-superuser */
	long f_files;			/* total file nodes in file system */
	long f_ffree;			/* free file nodes in fs */
	fsid_t f_fsid;			/* file system id */
	long f_spare[7];		/* spare for later */
};

#ifdef KERNEL
/*
 * public operations
 */
extern void	vfs_mountroot();	/* mount the root */
extern int	vfs_add();		/* add a new vfs to mounted vfs list */
extern void	vfs_remove();		/* remove a vfs from mounted vfs list */
extern int	vfs_lock();		/* lock a vfs */
extern void	vfs_unlock();		/* unlock a vfs */
extern struct vfs *getvfs();		/* return vfs given fsid */

#define VFS_INIT(VFSP, OP, DATA)	{ \
	(VFSP)->vfs_next = (struct vfs *)0; \
	(VFSP)->vfs_op = (OP); \
	(VFSP)->vfs_flag = 0; \
	(VFSP)->vfs_exflags = 0; \
	(VFSP)->vfs_data = (DATA); \
}

/*
 * globals
 */
extern struct vfs *rootvfs;		/* ptr to root vfs structure */

#endif
/*
 * $Header: file.h,v 1.4 88/09/11 18:00:16 keith Exp $
 * $Source: /zigguratusr/LCU/src/sys/h/RCS/file.h,v $
 *
 * Copyright (c) 1988 Acorn Computers Ltd., Cambridge, England
 *
 * $Desc$
 * $Log:	file.h,v $
 * Revision 1.4  88/09/11  18:00:16  keith
 * Make inclusion of fcntl.h conditional on it not already benn done.
 * This allows user programs (eg pstat, netstat, etc) to do an explictit
 * #define KERNEL (to get the definitions of kernel data structures)
 * without the compiler attempting to look for "../h/fcntl.h"
 * 
 * Revision 1.3  88/06/17  20:19:16  beta
 * Acorn Unix initial beta version
 * 
 */
/* @(#)file.h	1.5 87/07/21 3.2/4.3NFSSRC */
/*
 * Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)file.h	7.1 (Berkeley) 6/4/86
 */

#ifndef __FCNTL_HEADER__
#  ifdef KERNEL
#    include "../h/fcntl.h"
#  else KERNEL
#    include <sys/fcntl.h>
#  endif KERNEL
#endif !__FCNTL_HEADER__

#ifdef KERNEL
/*
 * Descriptor table entry.
 * One for each kernel object.
 */
struct	file {
	int	f_flag;		/* see below */
	short	f_type;		/* descriptor type */
	short	f_count;	/* reference count */
	short	f_msgcount;	/* references from message queue */
	struct	fileops {
		int	(*fo_rw)();
		int	(*fo_ioctl)();
		int	(*fo_select)();
		int	(*fo_close)();
	} *f_ops;
	caddr_t	f_data;		/* ptr to file specific struct (vnode/socket) */
	off_t	f_offset;
	struct	ucred *f_cred;	/* credentials of user who opened file */
};

#ifdef	DYNALLOC
struct	file *file, *fileNFILE;
#else	DYNALLOC
struct	file stat_file[NFILE], *file, *fileNFILE;
#endif	DYNALLOC
int	nfile;
struct	file *getf();
struct	file *falloc();
#endif

/*
 * flags- also for fcntl call.
 */
#define	FOPEN		(-1)
#define	FREAD		00001		/* descriptor read/receive'able */
#define	FWRITE		00002		/* descriptor write/send'able */
#define	FMARK		00020		/* mark during gc() */
#define	FDEFER		00040		/* defer for next gc pass */
#define	FSHLOCK		00200		/* shared lock present */
#define	FEXLOCK		00400		/* exclusive lock present */

/* bits to save after open */
#define	FMASK		00113
#define	FCNTLCANT	(FREAD|FWRITE|FMARK|FDEFER|FSHLOCK|FEXLOCK)

/*
 * User definitions.
 */

/*
 * Flock call.
 */
#define	LOCK_SH		1	/* shared lock */
#define	LOCK_EX		2	/* exclusive lock */
#define	LOCK_NB		4	/* don't block when locking */
#define	LOCK_UN		8	/* unlock */

/*
 * Lseek call.
 */
#define	L_SET		0	/* absolute offset */
#define	L_INCR		1	/* relative to current offset */
#define	L_XTND		2	/* relative to end of file */

#ifdef KERNEL
#define	GETF(fp, fd) { \
	if ((unsigned)(fd) >= NOFILE || ((fp) = u.u_ofile[fd]) == NULL) { \
		u.u_error = EBADF; \
		return; \
	} \
}

#define	DTYPE_VNODE	1	/* file */
#define	DTYPE_SOCKET	2	/* communications endpoint */
#endif
/*
 * $Header: errno.h,v 1.4 88/10/25 13:21:59 brian Exp $
 * $Source: /zigguratusr/LCU/src/sys/h/RCS/errno.h,v $
 *
 * Copyright (c) 1988 Acorn Computers Ltd., Cambridge, England
 *
 * $Desc$
 * $Log:	errno.h,v $
 * Revision 1.4  88/10/25  13:21:59  brian
 * Shared library errors added.
 * 
 * Revision 1.3  88/06/17  20:19:06  beta
 * Acorn Unix initial beta version
 * 
 */
/* @(#)errno.h	1.4 87/06/02 3.2/4.3NFSSRC */
/*
 * Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)errno.h	7.1 (Berkeley) 6/4/86
 */

/*
 * Error codes
 */

#define	EPERM		1		/* Not owner */
#define	ENOENT		2		/* No such file or directory */
#define	ESRCH		3		/* No such process */
#define	EINTR		4		/* Interrupted system call */
#define	EIO		5		/* I/O error */
#define	ENXIO		6		/* No such device or address */
#define	E2BIG		7		/* Arg list too long */
#define	ENOEXEC		8		/* Exec format error */
#define	EBADF		9		/* Bad file number */
#define	ECHILD		10		/* No children */
#define	EAGAIN		11		/* No more processes */
#define	ENOMEM		12		/* Not enough core */
#define	EACCES		13		/* Permission denied */
#define	EFAULT		14		/* Bad address */
#define	ENOTBLK		15		/* Block device required */
#define	EBUSY		16		/* Mount device busy */
#define	EEXIST		17		/* File exists */
#define	EXDEV		18		/* Cross-device link */
#define	ENODEV		19		/* No such device */
#define	ENOTDIR		20		/* Not a directory*/
#define	EISDIR		21		/* Is a directory */
#define	EINVAL		22		/* Invalid argument */
#define	ENFILE		23		/* File table overflow */
#define	EMFILE		24		/* Too many open files */
#define	ENOTTY		25		/* Not a typewriter */
#define	ETXTBSY		26		/* Text file busy */
#define	EFBIG		27		/* File too large */
#define	ENOSPC		28		/* No space left on device */
#define	ESPIPE		29		/* Illegal seek */
#define	EROFS		30		/* Read-only file system */
#define	EMLINK		31		/* Too many links */
#define	EPIPE		32		/* Broken pipe */

/* math software */
#define	EDOM		33		/* Argument too large */
#define	ERANGE		34		/* Result too large */

/* non-blocking and interrupt i/o */
#define	EWOULDBLOCK	35		/* Operation would block */
#define	EINPROGRESS	36		/* Operation now in progress */
#define	EALREADY	37		/* Operation already in progress */

/* ipc/network software */

	/* argument errors */
#define	ENOTSOCK	38		/* Socket operation on non-socket */
#define	EDESTADDRREQ	39		/* Destination address required */
#define	EMSGSIZE	40		/* Message too long */
#define	EPROTOTYPE	41		/* Protocol wrong type for socket */
#define	ENOPROTOOPT	42		/* Protocol not available */
#define	EPROTONOSUPPORT	43		/* Protocol not supported */
#define	ESOCKTNOSUPPORT	44		/* Socket type not supported */
#define	EOPNOTSUPP	45		/* Operation not supported on socket */
#define	EPFNOSUPPORT	46		/* Protocol family not supported */
#define	EAFNOSUPPORT	47		/* Address family not supported by protocol family */
#define	EADDRINUSE	48		/* Address already in use */
#define	EADDRNOTAVAIL	49		/* Can't assign requested address */

	/* operational errors */
#define	ENETDOWN	50		/* Network is down */
#define	ENETUNREACH	51		/* Network is unreachable */
#define	ENETRESET	52		/* Network dropped connection on reset */
#define	ECONNABORTED	53		/* Software caused connection abort */
#define	ECONNRESET	54		/* Connection reset by peer */
#define	ENOBUFS		55		/* No buffer space available */
#define	EISCONN		56		/* Socket is already connected */
#define	ENOTCONN	57		/* Socket is not connected */
#define	ESHUTDOWN	58		/* Can't send after socket shutdown */
#define	ETOOMANYREFS	59		/* Too many references: can't splice */
#define	ETIMEDOUT	60		/* Connection timed out */
#define	ECONNREFUSED	61		/* Connection refused */

	/* */
#define	ELOOP		62		/* Too many levels of symbolic links */
#define	ENAMETOOLONG	63		/* File name too long */

/* should be rearranged */
#define	EHOSTDOWN	64		/* Host is down */
#define	EHOSTUNREACH	65		/* No route to host */
#define	ENOTEMPTY	66		/* Directory not empty */

/* quotas & mush */
#define	EPROCLIM	67		/* Too many processes */
#define	EUSERS		68		/* Too many users */
#define	EDQUOT		69		/* Disc quota exceeded */

/* Network File System */
#define ESTALE          70              /* Stale NFS file handle */
#define EREMOTE         71              /* Too many levels of remote in path */

/* SystemV Record Locking */
#define EDEADLK         78              /* Deadlock condition. */
#define ENOLCK          79              /* No record locks available. */

/* SystemV IPC */
#define	ENOMSG		80		/* No suitable message on queue */
#define EIDRM		81		/* Identifier removed from system */

/* Shared libraries */
#define ELIBVER		82		/* Wrong version of shared library */
#define ELIBACC		83		/* Permission denied (shared library) */ 
#define ELIBLIM		84		/* Shared libraries nested too deeply */
#define ELIBNOENT	85		/* Shared library file not found */
#define ELIBNOEXEC	86		/* Shared library exec format error */
/*
 * $Header: socket.h,v 1.3 88/06/17 20:21:11 beta Beta1 $
 * $Source: /zigguratusr/LCU/src/sys/h/RCS/socket.h,v $
 *
 * Copyright (c) 1988 Acorn Computers Ltd., Cambridge, England
 *
 * $Desc$
 * $Log:	socket.h,v $
 * Revision 1.3  88/06/17  20:21:11  beta
 * Acorn Unix initial beta version
 * 
 */
/* @(#)socket.h	1.2 87/05/15 3.2/4.3NFSSRC */
/*
 * Copyright (c) 1982,1985, 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)socket.h	7.1 (Berkeley) 6/4/86
 */

/*
 * Definitions related to sockets: types, address families, options.
 */

/*
 * Types
 */
#define	SOCK_STREAM	1		/* stream socket */
#define	SOCK_DGRAM	2		/* datagram socket */
#define	SOCK_RAW	3		/* raw-protocol interface */
#define	SOCK_RDM	4		/* reliably-delivered message */
#define	SOCK_SEQPACKET	5		/* sequenced packet stream */

/*
 * Option flags per-socket.
 */
#define	SO_DEBUG	0x0001		/* turn on debugging info recording */
#define	SO_ACCEPTCONN	0x0002		/* socket has had listen() */
#define	SO_REUSEADDR	0x0004		/* allow local address reuse */
#define	SO_KEEPALIVE	0x0008		/* keep connections alive */
#define	SO_DONTROUTE	0x0010		/* just use interface addresses */
#define	SO_BROADCAST	0x0020		/* permit sending of broadcast msgs */
#define	SO_USELOOPBACK	0x0040		/* bypass hardware when possible */
#define	SO_LINGER	0x0080		/* linger on close if data present */
#define	SO_OOBINLINE	0x0100		/* leave received OOB data in line */

/*
 * Additional options, not kept in so_options.
 */
#define SO_SNDBUF	0x1001		/* send buffer size */
#define SO_RCVBUF	0x1002		/* receive buffer size */
#define SO_SNDLOWAT	0x1003		/* send low-water mark */
#define SO_RCVLOWAT	0x1004		/* receive low-water mark */
#define SO_SNDTIMEO	0x1005		/* send timeout */
#define SO_RCVTIMEO	0x1006		/* receive timeout */
#define	SO_ERROR	0x1007		/* get error status and clear */
#define	SO_TYPE		0x1008		/* get socket type */

/*
 * Structure used for manipulating linger option.
 */
struct	linger {
	int	l_onoff;		/* option on/off */
	int	l_linger;		/* linger time */
};

/*
 * Level number for (get/set)sockopt() to apply to socket itself.
 */
#define	SOL_SOCKET	0xffff		/* options for socket level */

/*
 * Address families.
 */
#define	AF_UNSPEC	0		/* unspecified */
#define	AF_UNIX		1		/* local to host (pipes, portals) */
#define	AF_INET		2		/* internetwork: UDP, TCP, etc. */
#define	AF_IMPLINK	3		/* arpanet imp addresses */
#define	AF_PUP		4		/* pup protocols: e.g. BSP */
#define	AF_CHAOS	5		/* mit CHAOS protocols */
#define	AF_NS		6		/* XEROX NS protocols */
#define	AF_NBS		7		/* nbs protocols */
#define	AF_ECMA		8		/* european computer manufacturers */
#define	AF_DATAKIT	9		/* datakit protocols */
#define	AF_CCITT	10		/* CCITT protocols, X.25 etc */
#define	AF_SNA		11		/* IBM SNA */
#define AF_DECnet	12		/* DECnet */
#define AF_DLI		13		/* Direct data link interface */
#define AF_LAT		14		/* LAT */
#define	AF_HYLINK	15		/* NSC Hyperchannel */
#define	AF_APPLETALK	16		/* Apple Talk */

#define	AF_MAX		17

/*
 * Structure used by kernel to store most
 * addresses.
 */
struct sockaddr {
	u_short	sa_family;		/* address family */
	char	sa_data[14];		/* up to 14 bytes of direct address */
};

/*
 * Structure used by kernel to pass protocol
 * information in raw sockets.
 */
struct sockproto {
	u_short	sp_family;		/* address family */
	u_short	sp_protocol;		/* protocol */
};

/*
 * Protocol families, same as address families for now.
 */
#define	PF_UNSPEC	AF_UNSPEC
#define	PF_UNIX		AF_UNIX
#define	PF_INET		AF_INET
#define	PF_IMPLINK	AF_IMPLINK
#define	PF_PUP		AF_PUP
#define	PF_CHAOS	AF_CHAOS
#define	PF_NS		AF_NS
#define	PF_NBS		AF_NBS
#define	PF_ECMA		AF_ECMA
#define	PF_DATAKIT	AF_DATAKIT
#define	PF_CCITT	AF_CCITT
#define	PF_SNA		AF_SNA
#define PF_DECnet	AF_DECnet
#define PF_DLI		AF_DLI
#define PF_LAT		AF_LAT
#define	PF_HYLINK	AF_HYLINK
#define	PF_APPLETALK	AF_APPLETALK

#define	PF_MAX		AF_MAX

/*
 * Maximum queue length specifiable by listen.
 */
#define	SOMAXCONN	5

/*
 * Message header for recvmsg and sendmsg calls.
 */
struct msghdr {
	caddr_t	msg_name;		/* optional address */
	int	msg_namelen;		/* size of address */
	struct	iovec *msg_iov;		/* scatter/gather array */
	int	msg_iovlen;		/* # elements in msg_iov */
	caddr_t	msg_accrights;		/* access rights sent/received */
	int	msg_accrightslen;
};

#define	MSG_OOB		0x1		/* process out-of-band data */
#define	MSG_PEEK	0x2		/* peek at incoming message */
#define	MSG_DONTROUTE	0x4		/* send without using routing tables */

#define	MSG_MAXIOVLEN	16
/*
 * $Header: un.h,v 1.3 88/06/17 20:22:21 beta Beta1 $
 * $Source: /zigguratusr/LCU/src/sys/h/RCS/un.h,v $
 *
 * Copyright (c) 1988 Acorn Computers Ltd., Cambridge, England
 *
 * $Desc$
 * $Log:	un.h,v $
 * Revision 1.3  88/06/17  20:22:21  beta
 * Acorn Unix initial beta version
 * 
 */
/* @(#)un.h	1.2 87/05/15 3.2/4.3NFSSRC */
/*
 * Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)un.h	7.1 (Berkeley) 6/4/86
 */

/*
 * Definitions for UNIX IPC domain.
 */
struct	sockaddr_un {
	short	sun_family;		/* AF_UNIX */
	char	sun_path[108];		/* path name (gag) */
};

#ifdef KERNEL
int	unp_discard();
#endif
/*
 * $Header: in.h,v 1.3 88/06/17 20:26:14 beta Beta1 $
 * $Source: /zigguratusr/LCU/src/sys/netinet/RCS/in.h,v $
 *
 * Copyright (c) 1988 Acorn Computers Ltd., Cambridge, England
 *
 * $Desc$
 * $Log:	in.h,v $
 * Revision 1.3  88/06/17  20:26:14  beta
 * Acorn Unix initial beta version
 * 
 */
/* @(#)in.h	1.2 87/06/18 3.2/4.3NFSSRC */
/*
 * Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)in.h	7.1 (Berkeley) 6/5/86
 */

/*
 * Constants and structures defined by the internet system,
 * Per RFC 790, September 1981.
 */

/*
 * Protocols
 */
#define	IPPROTO_IP		0		/* dummy for IP */
#define	IPPROTO_ICMP		1		/* control message protocol */
#define	IPPROTO_GGP		2		/* gateway^2 (deprecated) */
#define	IPPROTO_TCP		6		/* tcp */
#define	IPPROTO_EGP		8		/* exterior gateway protocol */
#define	IPPROTO_PUP		12		/* pup */
#define	IPPROTO_UDP		17		/* user datagram protocol */
#define	IPPROTO_IDP		22		/* xns idp */

#define	IPPROTO_RAW		255		/* raw IP packet */
#define	IPPROTO_MAX		256


/*
 * Ports < IPPORT_RESERVED are reserved for
 * privileged processes (e.g. root).
 * Ports > IPPORT_USERRESERVED are reserved
 * for servers, not necessarily privileged.
 */
#define	IPPORT_RESERVED		1024
#define	IPPORT_USERRESERVED	5000

/*
 * Link numbers
 */
#define	IMPLINK_IP		155
#define	IMPLINK_LOWEXPER	156
#define	IMPLINK_HIGHEXPER	158

/*
 * Internet address (a structure for historical reasons)
 */
struct in_addr {
	u_long s_addr;
};

/*
 * Definitions of bits in internet address integers.
 * On subnets, the decomposition of addresses to host and net parts
 * is done according to subnet mask, not the masks here.
 */
#define	IN_CLASSA(i)		(((long)(i) & 0x80000000) == 0)
#define	IN_CLASSA_NET		0xff000000
#define	IN_CLASSA_NSHIFT	24
#define	IN_CLASSA_HOST		0x00ffffff
#define	IN_CLASSA_MAX		128

#define	IN_CLASSB(i)		(((long)(i) & 0xc0000000) == 0x80000000)
#define	IN_CLASSB_NET		0xffff0000
#define	IN_CLASSB_NSHIFT	16
#define	IN_CLASSB_HOST		0x0000ffff
#define	IN_CLASSB_MAX		65536

#define	IN_CLASSC(i)		(((long)(i) & 0xc0000000) == 0xc0000000)
#define	IN_CLASSC_NET		0xffffff00
#define	IN_CLASSC_NSHIFT	8
#define	IN_CLASSC_HOST		0x000000ff

#define	INADDR_ANY		(u_long)0x00000000
#define	INADDR_BROADCAST	(u_long)0xffffffff	/* must be masked */

/*
 * Socket address, internet style.
 */
struct sockaddr_in {
	short	sin_family;
	u_short	sin_port;
	struct	in_addr sin_addr;
	char	sin_zero[8];
};

/*
 * Loopback Address (in host-order)
 */
#define INADDR_LOOPBACK 0x7f000001

/*
 * Options for use with [gs]etsockopt at the IP level.
 */
#define	IP_OPTIONS	1		/* set/get IP per-packet options */

#if !defined(vax) && !defined(arm) && !defined(ntohl) && !defined(lint)
/*
 * Macros for number representation conversion.
 */
#define	ntohl(x)	(x)
#define	ntohs(x)	(x)
#define	htonl(x)	(x)
#define	htons(x)	(x)
#endif

#if !defined(ntohl) && (defined(vax) || defined(arm) || defined(lint))
u_short	ntohs(), htons();
u_long	ntohl(), htonl();
#endif

#ifdef KERNEL
extern	struct domain inetdomain;
extern	struct protosw inetsw[];
struct	in_addr in_makeaddr();
u_long	in_netof(), in_lnaof();
#endif
/*
 * $Header: netdb.h,v 1.3 88/06/19 15:21:28 beta Exp $
 * $Source: /zigguratusr/LCU/src/include/RCS/netdb.h,v $
 *
 * Copyright (c) 1988 Acorn Computers Ltd., Cambridge, England
 *
 * $Desc$
 * $Log:	netdb.h,v $
 * Revision 1.3  88/06/19  15:21:28  beta
 * Acorn Unix initial beta version
 * 
 */
/* @(#)netdb.h	1.2 87/07/17 3.2/4.3NFSSRC */
/*
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)netdb.h	5.7 (Berkeley) 5/12/86
 */

/*
 * Structures returned by network
 * data base library.  All addresses
 * are supplied in host order, and
 * returned in network order (suitable
 * for use in system calls).
 */
struct	hostent {
	char	*h_name;	/* official name of host */
	char	**h_aliases;	/* alias list */
	int	h_addrtype;	/* host address type */
	int	h_length;	/* length of address */
	char	**h_addr_list;	/* list of addresses from name server */
#define	h_addr	h_addr_list[0]	/* address, for backward compatiblity */
};

/*
 * Assumption here is that a network number
 * fits in 32 bits -- probably a poor one.
 */
struct	netent {
	char		*n_name;	/* official name of net */
	char		**n_aliases;	/* alias list */
	int		n_addrtype;	/* net address type */
	unsigned long	n_net;		/* network # */
};

struct	servent {
	char	*s_name;	/* official service name */
	char	**s_aliases;	/* alias list */
	int	s_port;		/* port # */
	char	*s_proto;	/* protocol to use */
};

struct	protoent {
	char	*p_name;	/* official protocol name */
	char	**p_aliases;	/* alias list */
	int	p_proto;	/* protocol # */
};

struct rpcent {
        char    *r_name;        /* name of server for this rpc program */
        char    **r_aliases;    /* alias list */
        int     r_number;       /* rpc program number */
};

struct hostent	*gethostbyname(), *gethostbyaddr(), *gethostent();
struct netent	*getnetbyname(), *getnetbyaddr(), *getnetent();
struct servent	*getservbyname(), *getservbyport(), *getservent();
struct protoent	*getprotobyname(), *getprotobynumber(), *getprotoent();
struct rpcent   *getrpcbyname(), *getrpcbynumber(), *getrpcent();

/*
 * Error return codes from gethostbyname() and gethostbyaddr()
 */

extern  int h_errno;	

#define	HOST_NOT_FOUND	1 /* Authoritive Answer Host not found */
#define	TRY_AGAIN	2 /* Non-Authoritive Host not found, or SERVERFAIL */
#define	NO_RECOVERY	3 /* Non recoverable errors, FORMERR, REFUSED, NOTIMP */
#define NO_ADDRESS	4 /* Valid host name, no address, look for MX record */
/*------------------------------------------------------------------------
--                                                                      --
--          H E L I O S   I N P U T / O U T P U T   S E R V E R         --
--          ---------------------------------------------------         --
--                                                                      --
--               Copyright (C) 1987, Perihelion Software Ltd.           --
--                            All Rights Reserved.                      --
--                                                                      --
--  sunlocal.h                                                          --
--                                                                      --
--  Author:  DJCH (Bath University), BLV                                --
--                                                                      --
------------------------------------------------------------------------*/
/* SccsId: 3.8 28/3/90\ Copyright (C) 1987, Perihelion Software Ltd.        */

#ifndef Files_Module
extern struct stat searchbuffer;
#endif

extern void socket_write();
extern void socket_read();
#ifdef Local_Module
static  void pipe_broken();
#endif

typedef long clock_t;
#define CLK_TCK       1	
#define clock() time(NULL)
#define SEEK_SET	0

extern char *sys_errlist[];
extern int  sys_nerr;

/**
*** These define the interface between the I/O Server and the
*** server window program
**/ 
#define FUNCTION_CODE	0xFF
#define WINDOW_SIZE     0x01
#define WINDOW_KILL     0x02
#define WINDOW_MESS	0x03
#define WINDOW_PANEL    0x04
#define WINDOW_DIED     0x05

/* third byte for debug function codes */

#define WIN_MEMORY      0x01
#define WIN_RECONF      0x02
#define WIN_MESSAGES	0x03
#define WIN_SEARCH	0x04
#define WIN_OPEN	0x05
#define WIN_CLOSE	0x06
#define IOWIN_NAME	0x07
#define WIN_READ	0x08
#define WIN_BOOT	0x09
#define WIN_KEYBOARD	0x0A
#define WIN_INIT	0x0B
#define WIN_WRITE	0x0C
#define WIN_QUIT	0x0D
#define WIN_GRAPHICS	0x0E
#define WIN_TIMEOUT     0x0F
#define WIN_OPENREPLY   0x10
#define WIN_FILEIO      0x11
#define WIN_DELETE      0x12
#define WIN_DIRECTORY   0x13
#ifdef NEVER
#define WIN_COM         0x15
#define WIN_HARDDISK    0x16
#endif
#define WIN_ALL		0x14
/* Not needed : nopop, listall */
/* ALL and logger are separate */

#define WIN_REBOOT	0x21
#define WIN_DEBUGGER	0x22
#define WIN_STATUS	0x23
#define WIN_EXIT	0x24
#define WIN_LOG_FILE	0x25
#define WIN_LOG_SCREEN	0x26
#define WIN_LOG_BOTH	0x27
#define WIN_DEBUG	0x28

#define WIN_OFF         0x00
#define WIN_ON		0x01
/*------------------------------------------------------------------------
--                                                                      --
--          H E L I O S   I N P U T / O U T P U T   S E R V E R         --
--          ---------------------------------------------------         --
--                                                                      --
--               Copyright (C) 1990, Perihelion Software Ltd.           --
--                            All Rights Reserved.                      --
--                                                                      --
--  armlocal.h                                                          --
--                                                                      --
--  Author:  BLV			                                --
--                                                                      --
------------------------------------------------------------------------*/
/* SccsId: 3.8 28/3/90\ Copyright (C) 1990, Perihelion Software Ltd.        */

extern int errno;
/*------------------------------------------------------------------------
--                                                                      --
--         H E L I O S   I N P U T / O U T P U T   S E R V E R          --
--         ---------------------------------------------------          --
--                                                                      --
--              Copyright (C) 1987, Perihelion Software Ltd.            --
--                         All Rights Reserved.                         --
--                                                                      --
--      Defines.h                                                       --
--                                                                      --
--      Author:  BLV 15/5/88                                            --
--                                                                      --
------------------------------------------------------------------------*/

/* RcsId: $Id: defines.h,v 1.8 1991/07/30 10:47:25 martyn Exp $ */
/* Copyright (C) 1987, Perihelion Software Ltd.       			*/

/**
*** This header file defines the facilities available with the various
*** configurations on which the input/output server runs. The server should
*** be made with one and only one of the following strings #define'd :
*** ST, PC, AMIGA, SUN, TRIPOS, VAX, OS2, OS9, XENIX (your own additions)
**/

#ifdef ST
#undef ST
#define ST 1
#else
#define ST 0
#endif

#ifdef PC
#undef PC
#define PC 1
#else
#define PC 0
#endif

#ifdef NECPC
#undef NECPC
#define NECPC 1
#else
#define NECPC 0
#endif

#ifdef AMIGA
#undef AMIGA       /* Lattice C defines AMIGA as well, a couple of times */
#undef AMIGA
#define AMIGA 1
#else
#define AMIGA 0
#endif

#ifdef TRIPOS
#undef TRIPOS
#define TRIPOS 1
#else
#define TRIPOS 0
#endif

#ifdef SUN3
#undef SUN3
#define SUN 1
#define SUN3 1
#define UNIX 1
#else
#define SUN3 0
#endif

#ifdef SUN4
#undef SUN4
#define SUN 1
#define SUN4 1
#define UNIX 1
#else
#define SUN4 0
#endif

#ifdef SUN386
#undef SUN386
#define SUN 1
#define SUN386 1
#define UNIX 1
#else
#define SUN386 0
#endif

#ifdef SM90
#undef SM90
#define SM90 1
#define UNIX 1
#else
#define SM90 0
#endif

#ifdef ARMBSD
#undef ARMBSD
#define ARMBSD 1
#define UNIX 1
#else
#define ARMBSD 0
#endif

#ifdef MEIKORTE
#undef MEIKORTE
#define MEIKORTE 1
#define UNIX 1       /* for now */
#else
#define MEIKORTE 0
#endif

                /* Bleistein-Rohde Systemtechnik GmbH Port */
                /* to a 386 box */
#ifdef UNIX386
#undef UNIX386
#define UNIX386 1
#define UNIX    1
#else
#define UNIX386 0
#endif

/* define for a 386 box running interactive 386/ix */
#ifdef IUNIX386
#undef IUNIX386
#define IUNIX386 1
#define UNIX    1
#else
#define IUNIX386 0
#endif

#ifndef SUN
#define SUN 0
#else
#endif

#ifndef UNIX
#define UNIX 0
#endif

#ifdef VAX
#undef VAX
#define VAX 1
#else
#define VAX 0
#endif


#ifdef OS2
#undef OS2
#define OS2 1
#else
#define OS2 0
#endif


#ifdef OS9
#undef OS9
#define OS9 1
#else
#define OS9 0
#endif


#ifdef XENIX
#undef XENIX
#define XENIX 1
#else
#define XENIX 0
#endif

#ifdef APOLLO
#undef APOLLO
#define APOLLO 1
#else
#define APOLLO 0
#endif

#ifdef FLEXOS
#undef FLEXOS
#define FLEXOS 1
#else
#define FLEXOS 0
#endif

#ifdef MAC
#undef MAC
#define MAC 1
#else
#define MAC 0
#endif


#if ST
/**
*** The following lines define the hardware used. Only one of the #define's
*** should have a 1. In the main code I use tests like #if ST or
*** #if (ST || PC) to provide all conditional compilation.
***
*** This determines whether the host machine has the same byte ordering as
*** a transputer or a different one.
**/
#define swapping_needed              1
/**
*** These lines specify which of the optional devices are supported
**/
#define gem_supported                0
#define mouse_supported              1
#define keyboard_supported           1
/**
*** The main use of the RS232 is for use with kermit and terminal emulator
*** programs, so that you can access other machines without leaving Helios.
*** It is not intended as a device which can be used for arbitrary networking.
**/
#define RS232_supported              1
#define Centronics_supported         1
#define Midi_supported               0
/**
*** For some machines there is a separate printer device, which might map
*** onto either the parallel or the serial port or which might send data
*** to a printer spooler of some sort. On something like a Sun or Vax you
*** should probably just support this, and not a separate Centronics device.
**/
#define Printer_supported            1
/**
*** Some machines may have ethernet boards accessible from Helios
**/
#define Ether_supported              0
/**
*** A rawdisk device allows you to run the Helios filing system on the
*** transputer. It requires reads and writes in terms of disk sectors
*** rather than files and directories.
**/
#define Rawdisk_supported            1
/**
*** The /x device is the standard Helios X server
**/
#define X_supported                  0
/**
*** The /NetworkController device provides reset and link configuration
*** support, if these have to be provided by the I/O Server rather than
*** by the root transputer (the latter is greatly preferred).
**/
#define Network_supported            0
/**
*** If the machine has multiple drives which must be treated specially and which
*** are readily distinguished from normal subdirectories, the following should
*** be defined.
**/
#define drives_are_special           1
/**
*** If the machine has floppy disks then the server provides limited support
*** for the special errors generated. Unfortunately much of this is hardware
*** dependant.
**/
#define floppies_available           1
/**
*** If it is desirable for the server to do its own memory allocation then the
*** following should be defined.
**/
#define use_own_memory_management    1
/**
*** Redirecting stdout under the Helios shell involves a file being opened twice
*** for write-only mode. Many systems including TOS do not allow this.
*** To get around this, Helios allows servers to close streams at any time,
*** so that I can keep track of which files are open and close the stream if
*** a get an open request for a file which is already open.
**/
#define files_cannot_be_opened_twice 1
/**
*** If it is important that the Server does not hog the entire the machine,
*** denying its use to other users or tasks, then the following should be
*** defined. It does mean more work in porting the server !
**/
#define multi_tasking                0
/**
*** There is a lot of demand for some interaction facility between Helios
*** programs and programs or routines on the Host machine. Although I do not
*** like the idea, I have to support it. Hence there is a special device,
*** /ST or /PC or whatever to which messages can be sent.
**/
#define interaction_supported        1
/**
*** It is rather desirable to have the host name available as a string.
*** At the moment though, this is only used in conjunction with
*** interaction_supported above.
**/
#define machine_name                 "st"
/**
*** If the IO Server supports multiple windows, you can define the following.
*** Note that multiple_windows should be set on nearly all machines,
*** implemented either as real windows or as pseudo-windows with a
*** hot-key switching mechanism.
**/
#define multiple_windows             1
/**
*** I provide a general-purpose ANSI screen emulator, which can be
*** incorporated into the main server fairly easily. Define the following
*** if you want it(recommended)
**/
#define use_ANSI_emulator            1
/**
*** The built-in debugger is optional, as a way of saving some memory.
*** Anyway, it is of little use to the man in the street.
**/
#define debugger_incorporated        1
/**
*** If the compiler supports ANSI-style function prototypes then this
*** should be defined.
**/
#define ANSI_prototypes              0
/**
*** If the Server should use the routines in the linkio.c module, this
*** should be defined.
**/
#define Use_linkio                   0
/**
*** If the Server should use the hosts socket/internet support (UNIX) this
*** should be defined.
**/
#define internet_supported	     0
#endif

#if PC
#define swapping_needed              0
#define mouse_supported              1
#define keyboard_supported           1
#define RS232_supported              1
#define Centronics_supported         1
#define Printer_supported            1
#define Midi_supported               0
#define Ether_supported              1
#define Rawdisk_supported            1
#ifdef HUNTROM
#define Romdisk_supported            1
#endif
#define X_supported                  0
#define Network_supported            0
#define drives_are_special           1
#define floppies_available           1
#define use_own_memory_management    1
#define files_cannot_be_opened_twice 0
#define multi_tasking                0
#define interaction_supported        1
#define machine_name                 "pc"
#define multiple_windows             1
#define use_ANSI_emulator            1
#define ANSI_prototypes              1
#define Use_linkio                   1
#ifdef SMALL
#define gem_supported                0
#define debugger_incorporated        0
#else
#define gem_supported                1
#define debugger_incorporated        1
#endif
#define internet_supported	     0
#endif

#if NECPC
#define swapping_needed              0
#define gem_supported                0
#define mouse_supported              1
#define keyboard_supported           1
#define RS232_supported              0
#define Centronics_supported         0
#define Printer_supported            0
#define Midi_supported               0
#define Ether_supported              0
#define Rawdisk_supported            0
#define X_supported                  0
#define Network_supported            0
#define drives_are_special           1
#define floppies_available           1
#define use_own_memory_management    1
#define files_cannot_be_opened_twice 0
#define multi_tasking                0
#define interaction_supported        1
#define machine_name                 "pc"
#define multiple_windows             1
#define use_ANSI_emulator            1
#define debugger_incorporated        1
#define internet_supported	     0
#define ANSI_prototypes              1
#define Use_linkio                   1
#endif

#if AMIGA
#define swapping_needed              1
#define gem_supported                0
#define mouse_supported              0
#define keyboard_supported           0
#define RS232_supported              0
#define Centronics_supported         0
#define Printer_supported            0
#define Midi_supported               0
#define Ether_supported              0
#define Rawdisk_supported            0
#define X_supported                  0
#define Network_supported            0
#define drives_are_special           1
#define floppies_available           0
#define use_own_memory_management    0
#define files_cannot_be_opened_twice 0
#define multi_tasking                0
#define interaction_supported        0
#define machine_name                 "amiga"
#define multiple_windows             1
#define use_ANSI_emulator            0
#define debugger_incorporated        1
#define internet_supported	     0
#define ANSI_prototypes              1
#define Use_linkio                   0
#endif

#if TRIPOS
#define swapping_needed              1
#define gem_supported                0
#define mouse_supported              0
#define keyboard_supported           0
#define RS232_supported              0
#define Centronics_supported         0
#define Printer_supported            0
#define Midi_supported               0
#define Ether_supported              0
#define Rawdisk_supported            0
#define X_supported                  0
#define Network_supported            0
#define drives_are_special           0
#define floppies_available           0
#define use_own_memory_management    0
#define files_cannot_be_opened_twice 0
#define multi_tasking                0
#define interaction_supported        0
#define machine_name                 "tripos"
#define multiple_windows             0
#define use_ANSI_emulator            0
#define debugger_incorporated        1
#define internet_supported	     0
#define ANSI_prototypes              1
#define Use_linkio                   0
#endif

#if UNIX 
#if (SUN3 || SUN4 || SM90)
#define swapping_needed              1
#else
#define swapping_needed              0
#endif

#define gem_supported                0
#define RS232_supported              0
#define Centronics_supported         0
#define Printer_supported            0
#define Midi_supported               0
#define Ether_supported              0
#define Rawdisk_supported            0
#define drives_are_special           0
#define floppies_available           0
#define use_own_memory_management    0
#define files_cannot_be_opened_twice 0
#define Use_linkio                   1
#define interaction_supported        0
#define multiple_windows             1
#define use_ANSI_emulator            1
#define debugger_incorporated        1
#if ARMBSD
#define internet_supported	     0
#else
#define internet_supported	     1
#endif
#if (0)
#define X_supported                  1
#else
#define X_supported                  0
#endif

#if (MEIKORTE)
#define multi_tasking                0
#define Network_supported            1
#else
#define Network_supported            0
#define multi_tasking                1
#endif

#if (0)
#define ANSI_prototypes              1
#else
#define ANSI_prototypes              0
#endif

#if (SM90)
#define mouse_supported              1
#define keyboard_supported           1
#else
#define mouse_supported              0
#define keyboard_supported           0
#endif

#endif  /* UNIX */

#if VAX
#define swapping_needed              0
#define gem_supported                0
#define mouse_supported              0
#define keyboard_supported           0
#define RS232_supported              0
#define Centronics_supported         0
#define Printer_supported            0
#define Midi_supported               0
#define Ether_supported              0
#define Rawdisk_supported            0
#define X_supported                  0
#define Network_supported            0
#define drives_are_special           0
#define floppies_available           0
#define use_own_memory_management    0
#define files_cannot_be_opened_twice 0
#define multi_tasking                0
#define interaction_supported        0
#define machine_name                 "vax"
#define multiple_windows             0
#define use_ANSI_emulator            0
#define debugger_incorporated        1
#define internet_supported	     0
#define ANSI_prototypes              0
#define Use_linkio                   0
#endif


#if OS2
#define swapping_needed              0
#define gem_supported                0
#define mouse_supported              0
#define keyboard_supported           0
#define RS232_supported              0
#define Centronics_supported         0
#define Printer_supported            0
#define Midi_supported               0
#define Ether_supported              0
#define Rawdisk_supported            0
#define X_supported                  0
#define Network_supported            0
#define drives_are_special           0
#define floppies_available           0
#define use_own_memory_management    0
#define files_cannot_be_opened_twice 0
#define multi_tasking                0
#define interaction_supported        0
#define machine_name                 "OS2"
#define multiple_windows             0
#define use_ANSI_emulator            0
#define debugger_incorporated        1
#define internet_supported	     0
#define ANSI_prototypes              0
#define Use_linkio                   0
#endif

#if OS9
#define swapping_needed              0
#define gem_supported                0
#define mouse_supported              0
#define keyboard_supported           0
#define RS232_supported              0
#define Centronics_supported         0
#define Printer_supported            0
#define Midi_supported               0
#define Ether_supported              0
#define Rawdisk_supported            0
#define X_supported                  0
#define Network_supported            0
#define drives_are_special           0
#define floppies_available           0
#define use_own_memory_management    0
#define files_cannot_be_opened_twice 0
#define multi_tasking                0
#define interaction_supported        0
#define machine_name                 "OS9"
#define multiple_windows             0
#define use_ANSI_emulator            0
#define debugger_incorporated        1
#define internet_supported	     0
#define ANSI_prototypes              0
#define Use_linkio                   0
#endif

#if XENIX
#define swapping_needed              0
#define gem_supported                0
#define mouse_supported              0
#define keyboard_supported           0
#define RS232_supported              0
#define Centronics_supported         0
#define Printer_supported            0
#define Midi_supported               0
#define Ether_supported              0
#define Rawdisk_supported            0
#define X_supported                  0
#define Network_supported            0
#define drives_are_special           0
#define floppies_available           0
#define use_own_memory_management    0
#define files_cannot_be_opened_twice 0
#define multi_tasking                0
#define interaction_supported        0
#define machine_name                 "XENIX"
#define multiple_windows             0
#define use_ANSI_emulator            0
#define debugger_incorporated        1
#define internet_supported	     0
#define ANSI_prototypes              0
#define Use_linkio                   0
#endif

#if APOLLO
#define swapping_needed              0
#define gem_supported                0
#define mouse_supported              0
#define keyboard_supported           0
#define RS232_supported              0
#define Centronics_supported         0
#define Printer_supported            0
#define Midi_supported               0
#define Ether_supported              0
#define Rawdisk_supported            0
#define X_supported                  0
#define Network_supported            0
#define drives_are_special           0
#define floppies_available           0
#define use_own_memory_management    0
#define files_cannot_be_opened_twice 0
#define multi_tasking                0
#define interaction_supported        0
#define machine_name                 "apollo"
#define multiple_windows             0
#define use_ANSI_emulator            0
#define debugger_incorporated        1
#define internet_supported	     0
#define ANSI_prototypes              0
#define Use_linkio                   0
#endif

#if FLEXOS
#define swapping_needed              0
#define gem_supported                0
#define mouse_supported              0
#define keyboard_supported           0
#define RS232_supported              0
#define Centronics_supported         0
#define Printer_supported            0
#define Midi_supported               0
#define Ether_supported              0
#define Rawdisk_supported            0
#define X_supported                  0
#define Network_supported            0
#define drives_are_special           0
#define floppies_available           0
#define use_own_memory_management    0
#define files_cannot_be_opened_twice 0
#define multi_tasking                0
#define interaction_supported        0
#define machine_name                 "flexos"
#define multiple_windows             0
#define use_ANSI_emulator            0
#define debugger_incorporated        1
#define internet_supported	     0
#define ANSI_prototypes              0
#define Use_linkio                   0
#endif

#if MAC
#define swapping_needed              1
#define gem_supported                0
#define mouse_supported              0
#define keyboard_supported           0
#define RS232_supported              0
#define Centronics_supported         0
#define Printer_supported            0
#define Midi_supported               0
#define Ether_supported              0
#define Rawdisk_supported            0
#define X_supported                  0
#define Network_supported            0
#define interrupts_use_clock         0
#define drives_are_special           1
#define floppies_available           0
#define use_own_memory_management    0
#define files_cannot_be_opened_twice 1
#define limit_on_files_open          20
#define multi_tasking                0
#define interaction_supported        0
#define machine_name                 "mac"
#define multiple_windows             0
#define use_ANSI_emulator            0
#define debugger_incorporated        1
#define internet_supported	     0
#define ANSI_prototypes              1
#endif


/**
*** Here I worry about some missing entries in the defines table
**/

#ifndef swapping_needed
To be or not to be, that is the question...

You must either define one of the machines, or extend this file to include
yours.
#endif

#ifndef gem_supported
#define gem_supported                0
#endif
#ifndef mouse_supported
#define mouse_supported              0
#endif
#ifndef keyboard_supported
#define keyboard_supported           0
#endif
#ifndef RS232_supported
#define RS232_supported              0
#endif
#ifndef Centronics_supported
#define Centronics_supported         0
#endif
#ifndef Printer_supported
#define Printer_supported            0
#endif
#ifndef Midi_supported
#define Midi_supported               0
#endif
#ifndef Ether_supported
#define Ether_supported              0
#endif
#ifndef Rawdisk_supported
#define Rawdisk_supported            0
#endif
#ifndef X_supported
#define X_supported                  0
#endif
#ifndef Network_supported
#define Network_supported            0
#endif
#ifndef drives_are_special
#define drives_are_special           0
#endif
#ifndef floppies_available
#define floppies_available           0
#endif
#ifndef use_own_memory_management
#define use_own_memory_management    0
#endif
#ifndef files_cannot_be_opened_twice
#define files_cannot_be_opened_twice 0
#endif
#ifndef multi_tasking
#define multi_tasking                0
#endif
#ifndef interaction_supported
#define interaction_supported        0
#endif
#ifndef machine_name
#define machine_name                 "Host"
#endif
#ifndef multiple_windows
#define multiple_windows             0
#endif
#ifndef use_ANSI_emulator
#define use_ANSI_emulator            0
#endif
#ifndef debugger_incorporated
#define debugger_incorporated        1
#endif
#ifndef ANSI_prototypes
#define ANSI_prototypes              0
#endif
#ifndef Use_linkio
#define Use_linkio                   0
#endif
#ifndef internet_supported
#define internet_supported	     0
#endif

/**
*** It is useful for me to know whether any of the communication ports are
*** supported, because they share code.
**/
#if (RS232_supported || Centronics_supported || Printer_supported || Midi_supported)
#define Ports_used 1
#else
#define Ports_used 0
#endif

/**
*** Now I need to know additional details about certain devices
**/

#if Centronics_supported
/**
*** Is it possible to read from the Centronics port as well as write to it ?
**/
#if (ST || PC)
#define Centronics_readable 0
#else
#define Centronics_readable 1
#endif

/**
*** Is there always a Centronics port, or is it optional ?
**/

#if (ST)
#define Centronics_Always_Available 1
#else
#define Centronics_Always_Available 0
#endif

#endif

#if RS232_supported

#if (ST)
#define RS232_Always_Available 1
#else
#define RS232_Always_Available 0
#endif

#endif

#if Printer_supported

#if (ST)
#define Printer_Always_Available 1
#else
#define Printer_Always_Available 0
#endif

#endif

#if Midi_supported

#if (0)
#define Midi_Always_Available 1
#else
#define Midi_Always_Available 0
#endif

#endif


#if gem_supported

#if 0
#define Gem_Always_Available 1
#else
#define Gem_Always_Available 0
#endif

#endif

#if mouse_supported

#if (ST)
#define Mouse_Always_Available 1
#else
#define Mouse_Always_Available 0
#endif

#endif

#if keyboard_supported

#if (ST)
#define Keyboard_Always_Available 1
#else
#define Keyboard_Always_Available 0
#endif

#endif

/*------------------------------------------------------------------------
--                                                                      --
--          H E L I O S   I N P U T / O U T P U T   S E R V E R         --
--          ---------------------------------------------------         --
--                                                                      --
--             Copyright (C) 1987, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
--      barthdr                                                         --
--                                                                      --
--         A header file for all my C programs, containing odds and     --
--                                                                      --
--         ends to turn C into a slightly less useless language.        --
--                                                                      --
--     Author:  BLV 8/10/87                                             --
--                                                                      --
------------------------------------------------------------------------*/
/* RcsId: $Id: barthdr,v 1.2 90/10/18 15:27:00 alan Exp $ */
/* Copyright (C) 1987, Perihelion Software Ltd.         		*/

typedef long *ptr;
/* RTE : may not be required */
#if (ST || PC || AMIGA || ARMBSD || MEIKORTE)
typedef unsigned int uint;
#endif
#define eq ==
#define ne !=
#define MAXINT 0x7FFFFFFFL

/**
*** This macro takes care of ANSI prototyping for ANSI and for non-ANSI
*** compilers - I hope. Functions can be defined by e.g.
*** extern void fn(xyz, (char *));
*** which expands to
*** extern void xyz (char *);   or
*** extern void xyz();
*** I have checked these macros using 5 different C compilers, and they
*** seem to work fine.
**/
#if ANSI_prototypes
#define fn(a, b) a b
#else
#define fn(a, b) a()
#endif

/**
*** The following macro is designed to get around problems with compilers
*** that complain about variables not being used. If your compiler suffers from
*** the same problem then you can || your defines.h entry with the #if ST
**/

#if (ST || ARMBSD || MEIKORTE)
#define use(a) a=a;
#else
#if (MAC)
#define use(a) #pragma unused (a)
#else
#define use(a)
#endif /*MAC*/
#endif

/**
*** And another macro for compiler problems : func(x) expands to &x if your
*** compiler expects the address of a function to be passed as argument,
*** or to just x otherwise.
**/
#if ST
#define func(x) (&x)
#else
#define func(x) (x)
#endif

/**
*** On some machines, tolower and toupper work only  if the character is
*** already uppercase or lowercase respectively. On others the library bothers
*** to check. The following takes care of this bit of nastiness.
**/

#if (PC)
        /* Microsoft C 5.0 library does it for me */
#define ToLower(x) tolower(x)
#define ToUpper(x) toupper(x)

#else

#ifdef Server_Module

int ToLower(x)
int x;
{ return(isupper(x) ? tolower(x) : x);
}

int ToUpper(x)
int x;
{ return(islower(x) ? toupper(x) : x);
}

#else

extern int fn(ToLower, (int));
extern int fn(ToUpper, (int));

#endif /* Server_Module */

#endif

/**
*** Mark Williams C does not support memcpy...
**/
#if (ST)

#ifdef Server_Module
void memcpy(dest, source, count)
char *dest, *source;
int count;
{ for ( ; count > 0; count--) *dest++ = *source++;
}

void memmove(dest, source, count)
char *dest, *source;
int count;
{ if (dest < source)
   for ( ; count > 0; count--) *dest++ = *source++;
  else
    for (dest += count, source += count;  count > 0; count--)
      *(--dest) = *(--source);
}

void memset(dest, val, count)
char *dest;
int val, count;
{ for ( ; count > 0; count--) *dest++ = val;
}

#else
extern void fn(memcpy, (char *dest, char *source, int count));
extern void fn(memmove, (char *dest, char *source, int count));
extern void fn(memset, (char *dest, int val, int count));
#endif /* Server_Module */
#endif /* ST */

#if (SUN || ARMBSD)
  /* These have memcpy() and memset(), but not memmove() */
#define memmove(a, b, c) bcopy(b,a,c)
#endif

#if (PC)
extern long fn( divlong, (long, long));
#else
#define divlong(a, b) (a) / (b)
#endif

/*------------------------------------------------------------------------
--                                                                      --
--          H E L I O S   I N P U T / O U T P U T   S E R V E R         --
--          ---------------------------------------------------         --
--                                                                      --
--             Copyright (C) 1989, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--   protocol.h                                                         --
--                                                                      --
--    A header file defining the Helios protocols used by the Server.   --
--    Based on the Helios header files in /helios/include/syslib.h,     --
--    but modified to cope with 16-bit compilers.                       --
--                                                                      --
--    Author:  BLV 21/1/89                                              --
--                                                                      --
------------------------------------------------------------------------*/
/* RcsId: $Id: protocol.h,v 1.4 1991/09/13 14:43:44 martyn Exp $ */
/* Copyright (C) 1989, Perihelion Software Ltd.        			*/

/*------------------------------------------------------------------------
--                                                                      --
-- helios.h                                                             --
--                                                                      --
------------------------------------------------------------------------*/

/* standard type definitions */

typedef  long           WORD    ;       /* a machine word, 32 bits      */
typedef  unsigned long  UWORD   ;       /* a machine word, 32 bits      */
typedef  WORD           INT     ;       /* a synonym                    */
typedef  WORD           word    ;       /* another synonym              */
typedef  WORD           Offset  ;       /* and another                  */
typedef  short int      SHORT   ;       /* a 16 bit word                */
typedef  unsigned short USHORT  ;       /* an unsigned 16 bit value     */
typedef  char           BYTE    ;       /* a byte, used in place of char*/
typedef  BYTE           byte    ;       /* a synonym                    */
typedef  unsigned char  UBYTE   ;       /* an unsigned byte             */
typedef  char           *STRING ;       /* character string             */
typedef  char           *string ;       /* synonym                      */
typedef  word           bool    ;       /* boolean value                */

typedef  void           (*VoidFnPtr)(); /* pointer to void function     */
typedef  word           (*WordFnPtr)(); /* pointer to word function     */

#define PUBLIC          extern          /* an exported symbol           */
#define PRIVATE         static          /* an unexported symbol         */
#define FORWARD         extern          /* forward proc reference       */

/* Syntactic enrichment... */

#define forever         for(;;)
#define unless(x)       if(!(x))
#define until(x)        while(!(x))
#define elif(x)         else if(x)

#define TRUE            1l
#define true            1l
#define FALSE           0l
#define false           0l
#define Variable        1
#define MinInt          0x80000000L

#define MemStart  MinInt+0x70L
#define LoadBase  (MinInt+0x1000L)

#define Null(_type) ((_type *)NULL)

#define NameMax   32
#define c_dirchar '/'                   /* Helios directory separator */

#define OneSec           1000000L       /* one second in micro-seconds */

/* end of iohelios.h */

/*------------------------------------------------------------------------
--                                                                      --
-- ioattrib.h                                                           --
--                                                                      --
------------------------------------------------------------------------*/

typedef struct Attributes {
                            WORD  Input;
                            WORD  Output;
                            WORD  Control;
                            WORD  Local;
#if swapping_needed
                            short Time;
                            short Min;
#else
                            short Min;
                            short Time;
#endif
} Attributes;

typedef WORD Attribute;

#define ConsoleEcho           0x00000007L
#define ConsoleIgnoreBreak    0x00000100L
#define ConsoleBreakInterrupt 0x00000200L
#define ConsolePause          0x00000400L
#define ConsoleRawInput       0x0000000bL
#define ConsoleRawOutput      0x00000101L

#define RS232_IgnPar          0x00000800L
#define RS232_ParMrk          0x00001000L
#define RS232_InPck           0x00002000L
#define RS232_IXON            0x00004000L
#define RS232_IXOFF           0x00008000L
#define RS232_Istrip          0x00010000L
#define RS232_IgnoreBreak     0x00000100L
#define RS232_BreakInterrupt  0x00000200L
#define RS232_Cstopb          0x00000102L
#define RS232_Cread           0x00000202L
#define RS232_ParEnb          0x00000402L
#define RS232_ParOdd          0x00000802L
#define RS232_HupCl           0x00001002L
#define RS232_CLocal          0x00002002L
#define RS232_Csize           0x0003C000L   /* Mask for the sizes */
#define RS232_Csize_5         0x00004002L
#define RS232_Csize_6         0x00008002L
#define RS232_Csize_7         0x00010002L
#define RS232_Csize_8         0x00020002L

#define RS232_B0              0
#define RS232_B50             1
#define RS232_B75             2
#define RS232_B110            3
#define RS232_B134            4
#define RS232_B150            5
#define RS232_B200            6
#define RS232_B300            7
#define RS232_B600            8
#define RS232_B1200           9
#define RS232_B1800          10
#define RS232_B2400          11
#define RS232_B4800          12
#define RS232_B9600          13
#define RS232_B19200         14
#define RS232_B38400         15

/* end of ioattrib.h */

/*------------------------------------------------------------------------
--                                                                      --
-- iocodes.h                                                            --
--                                                                      --
------------------------------------------------------------------------*/

/*----------------------------------------------------------------
-- Subsystems
----------------------------------------------------------------*/

#define SS_Mask         0x1f000000L

#define SS_Unknown      0x00000000L
#define SS_Kernel       0x01000000L
#define SS_SysLib       0x02000000L
#define SS_ProcMan      0x03000000L
#define SS_Loader       0x04000000L
#define SS_TFM          0x05000000L
#define SS_RamDisk      0x06000000L
#define SS_HardDisk     0x07000000L
#define SS_Fifo         0x08000000L
#define SS_NameTable    0x09000000L
#define SS_IOProc       0x0A000000L
#define SS_Window       0x0B000000L
#define SS_IOC          0x0C000000L
#define SS_NullDevice   0x0d000000
#define SS_Pipe         0x0e000000
#define SS_Batch        0x0f000000
#define SS_Login        0x10000000
#define SS_NetServ      0x11000000
#define SS_SM           0x12000000
#define SS_Device       0x13000000
#define SS_InterNet     0x14000000

/*----------------------------------------------------------------
-- Function Codes
----------------------------------------------------------------*/
/*----------------------------------------------------------------
-- Function Classes
----------------------------------------------------------------*/

#define FC_Mask         0x60000000L
#define FC_GSP          0x00000000L
#define FC_Private      0x60000000L

/*----------------------------------------------------------------
-- Retry Counter
----------------------------------------------------------------*/

#define FR_Mask         0x00F00000
#define FR_Inc          0x00100000

/*----------------------------------------------------------------
-- General Functions
----------------------------------------------------------------*/

#define FG_Mask         0x00FFFFF0L

/* IOC requests */
#define FG_Unknown      0x00000000L
#define FG_Open         0x00000010L
#define FG_Create       0x00000020L
#define FG_Locate       0x00000030L
#define FG_ObjectInfo   0x00000040L
#define FG_ServerInfo   0x00000050L
#define FG_Delete       0x00000060L
#define FG_Rename       0x00000070L
#define FG_Link         0x00000080L
#define FG_Protect      0x00000090L
#define FG_SetDate      0x000000a0L
#define FG_Refine       0x000000b0L
#define FG_CloseObj     0x000000c0L

/* direct server requests */
#define FG_Read                 0x00001010L
#define FG_Write                0x00001020L
#define FG_GetSize              0x00001030L
#define FG_SetSize              0x00001040L
#define FG_Close                0x00001050L
#define FG_Seek                 0x00001060L
#define FG_GetAttr              0x00001070L
#define FG_SetAttr              0x00001080L
#define FG_EnableEvents         0x00001090L
#define FG_Acknowledge          0x000010A0L
#define FG_NegAcknowledge       0x000010B0L
#define FG_Select               0x000010C0L

/* Distributed search codes */
#define FG_Search               0x00002010L
#define FG_FollowTrail          0x00002020L

/*Socket Related Requests*/
#define FG_Socket               0x00008010  /* create socket */
#define FG_Bind                 0x00008020  /* bind socket to address */
#define FG_Listen               0x00008030  /* set socket connection queue size */
#define FG_Accept               0x00008040  /* accept a connection */
#define FG_Connect              0x00008050  /* connect to a remote service */
#define FG_SendMessage          0x00008060  /* send datagram or other message */
#define FG_RecvMessage          0x00008070  /* receieve datagram or other message */

/*Device Requests*/
#define FG_Format               0x0000a010  /* format disc */
#define FG_WriteBoot            0x0000a020  /* write boot area */

/* General Server Terminate */
#define FG_Terminate            0x00001FF0
#define FG_Reboot               0x00002FF0

/*----------------------------------------------------------------
-- Error Codes
----------------------------------------------------------------*/

#define ErrBit          0x80000000L     /* set for all error codes */
#define Err_Null        0L              /* no error at all         */

/*----------------------------------------------------------------
-- Error Classes
----------------------------------------------------------------*/

#define EC_Mask         0xe0000000L

#define EC_Recover      0x80000000L      /* a retry might succeed */
#define EC_Warn         0xA0000000L      /* recover & try again   */
#define EC_Error        0xC0000000L      /* client fatal          */
#define EC_Fatal        0xE0000000L      /* system fatal          */

/*----------------------------------------------------------------
-- General Error codes
----------------------------------------------------------------*/

#define EG_Mask         0x00FF0000L      /* mask to isolate             */

#define EG_UnknownError 0x00000000L
#define EG_NoMemory     0x00010000L     /* memory allocation failure    */
#define EG_Create       0x00020000L     /* failed to create             */
#define EG_Delete       0x00030000L     /* failed to delete             */
#define EG_Protected    0x00040000L     /* object is protected          */
#define EG_Timeout      0x00050000L     /* timeout                      */
#define EG_Unknown      0x00060000L     /* object not found             */
#define EG_FnCode       0x00070000L     /* unknown function code        */
#define EG_Name         0x00080000L     /* mal-formed name              */
#define EG_Invalid      0x00090000L     /* invalid/corrupt object       */
#define EG_InUse        0x000a0000L     /* object in use/locked         */
#define EG_Congested    0x000b0000L     /* server/route overloaded      */
#define EG_WrongFn      0x000c0000L     /* fn inappropriate to object   */
#define EG_Broken       0x000d0000L     /* object broken in some way    */
#define EG_Exception    0x000e0000L     /* exception message            */
#define EG_WrongSize    0x000f0000L     /* object wrong size            */
#define EG_ReBooted     0x00100000L     /* server/processor rebooted    */
#define EG_Open         0x00110000L
#define EG_Execute      0x00120000L
#define EG_Boot         0x00130000L
#define EG_State        0x00140000L
#define EG_NoResource   0x00150000L
#define EG_Errno        0x00160000L
#define EG_Parameter    0x00ff0000L     /* bad parameter value          */

/*----------------------------------------------------------------
-- Object codes for general errors
----------------------------------------------------------------*/

#define EO_Unknown       0x00000000L
#define EO_Message       0x00008001L     /* error refers to a message    */
#define EO_Task          0x00008002L     /* error refers to a task       */
#define EO_Port          0x00008003L     /* error refers to a port       */
#define EO_Route         0x00008004L     /* error refers to a route      */
#define EO_Directory     0x00008005L     /* error refers to a directory  */
#define EO_Object        0x00008006L     /* error refers to Object struct*/
#define EO_Stream        0x00008007L     /* error refers to Stream       */
#define EO_Program       0x00008008L
#define EO_Module        0x00008009L
#define EO_Matrix        0x0000800aL     /* access matrix                */
#define EO_Fifo          0x0000800bL
#define EO_File          0x0000800cL
#define EO_Capability    0x0000800dL
#define EO_Name          0x0000800eL     /* name in name table           */
#define EO_Window        0x0000800fL
#define EO_Server        0x00008010L
#define EO_TaskForce     0x00008011L
#define EO_Link          0x00008012L
#define EO_Memory        0x00008013L
#define EO_Pipe          0x00008014L
#define EO_NetServ       0x00008015L     /* error refers to NS           */
#define EO_Subnetwork    0x00008016L     /* error refers to Subnetwork   */
#define EO_User          0x00008017L
#define EO_Session       0x00008018L
#define EO_Loader        0x00008019L
#define EO_ProcMan       0x0000801AL
#define EO_TFM           0x0000801BL
#define EO_Attribute     0x0000801CL
#define EO_NoProcessors  0x0000801DL
#define EO_ProcessorType 0x0000801EL
#define EO_Processor     0x0000801FL
#define EO_Socket        0x00008020L
#define EO_Medium        0x00008021L

/*----------------------------------------------------------------------*/
/*-- Exception codes                                                  --*/
/*----------------------------------------------------------------------*/

#define	EE_Mask		    0x0000ffff
#define	EE_Shift	    0

#define	EE_Null		    0x00000000
#define	EE_Kill		    0x00000004
#define	EE_Abort	    0x00000005
#define	EE_Suspend	    0x00000006
#define	EE_Restart	    0x00000007
#define	EE_Interrupt	    0x00000008
#define	EE_ErrorFlag	    0x00000009
#define	EE_StackError	    0x0000000a
#define	EE_Signal	    0x00007f00

/* End of iocodes.h */

/*------------------------------------------------------------------------
--                                                                      --
-- ioprot.h                                                             --
--                                                                      --
------------------------------------------------------------------------*/

typedef unsigned long   Matrix;    /* access matrix */
typedef UBYTE           AccMask;   /* access mask   */
typedef word            Key;       /* encryption key */

/*----------------------------------------------------------------------*/
/* Access capability                                                    */
/*----------------------------------------------------------------------*/

typedef struct Capability {
        AccMask         Access;   /* access mask      */
        UBYTE           Valid[7]; /* validation value */
} Capability;

/*----------------------------------------------------------------------*/
/* Access mask bits                                                     */
/*----------------------------------------------------------------------*/

#define AccMask_Full    0xff            /* all bits set       */

/* All */
#define AccMask_R       0x01            /* Read permission   */
#define AccMask_W       0x02            /* Write permission  */
#define AccMask_D       0x40            /* Delete permission */
#define AccMask_A       0x80            /* Alter permission  */

/* Files only */
#define AccMask_E       0x04            /* Execute permission */
#define AccMask_F       0x08            /* unused - arbitrary letters */
#define AccMask_G       0x10
#define AccMask_H       0x20

/* Directories only */
#define AccMask_V       0x04            /* V access category  */
#define AccMask_X       0x08            /* X access category  */
#define AccMask_Y       0x10            /* Y access category  */
#define AccMask_Z       0x20            /* Z access category  */

/* Tasks only */
#define AccMask_K       AccMask_D       /* Kill task (== Delete) */

/*----------------------------------------------------------------------*/
/* Access Matrix category masks                                         */
/*----------------------------------------------------------------------*/

#define AccMatrix_V     0x000000ffL
#define AccMatrix_X     0x0000ff00L
#define AccMatrix_Y     0x00ff0000L
#define AccMatrix_Z     0xff000000L

/*----------------------------------------------------------------------*/
/* Printed matrix letters                                               */
/*----------------------------------------------------------------------*/

#define FileChars   "rwefghda"
#define DirChars    "rwvxyzda"
#define TaskChars   "rw????ka"
#define ModChars    "r?e???da"
#define ProgChars   "rwe???da"

/*----------------------------------------------------------------------*/
/* Default Matrices                                                     */
/*----------------------------------------------------------------------*/

#define DefDirMatrix    0x21134BC7L     /* DARWV/DRWX/RWY/RZ */
#define DefFileMatrix   0x010343C3L     /* DARW/DRW/RW/R     */
#define DefLinkMatrix   0x201088C4L     /* dav:dx:y:z        */
#define DefTFMatrix     0x21134BC7L     /* darwv:drwx:rwy:rz */
#define DefTaskMatrix   0x010343C3L     /* darw:drw:rw:r     */
#define DefModuleMatrix 0x010545C5L     /* dare:dre:re:r     */
#define DefProgMatrix   0x010545C5L     /* dare:dre:re:r     */
#define DefNameMatrix   0x21110907L     /* rwv:rx:ry:rz      */
#define DefRootMatrix   0x21130B87L     /* arwv:rwx:rwy:rz   */


/* End of ioprot.h */

/*------------------------------------------------------------------------
--                                                                      --
-- iomess.h                                                             --
--                                                                      --
------------------------------------------------------------------------*/

/* Message port */

typedef UWORD           Port;           /* true structure hidden      */
#define NullPort        ((Port)0L)      /* zero is never a valid port */

typedef struct MsgHdr {
#if swapping_needed
        byte            Flags;          /* flag byte           */
        byte            ContSize;       /* control vector size */
        USHORT          DataSize;       /* 16 bit data size    */
#else
        USHORT          DataSize;
        byte            ContSize;
        byte            Flags;
#endif
        Port            Dest;           /* destination port descriptor */
        Port            Reply;          /* reply port descriptor       */
        word            FnRc;           /* function/return code        */
} MsgHdr;

#define MsgHdr_Flags_nothdr     0x80    /* used by kernel              */
#define MsgHdr_Flags_preserve   0x40    /* preserve destination route  */
#define MsgHdr_Flags_exception  0x20    /* exception message           */
#define MsgHdr_Flags_sacrifice  0x10    /* kernel may throw message away */
#define MsgHdr_Flags_bytesex    0x08    /* 0 = even, 1 = odd           */

/* Message control block */

typedef struct MCB {
        MsgHdr          MsgHdr;         /* message header buffer       */
        word            Timeout;        /* message timeout             */
        word            *Control;       /* pointer to control buffer   */
        byte            *Data;          /* pointer to data buffer      */
} MCB;

/* -- End of iomess.h */

/*------------------------------------------------------------------------
--                                                                      --
--      iogsp.h                                                         --
--                                                                      --
------------------------------------------------------------------------*/

/* offsets within the control vector for GSP requests   */      

#define Context_off          0   /* offsets in the control vector for all */
#define Pathname_off         1   /* directory requests */
#define Nextname_off         2
#define Cap1_off             3
#define Cap2_off             4
#define cont_minsize         5

#define arg1_off             5  /* additional offsets for directory requests */
#define arg2_off             6
#define arg3_off             7
#define arg4_off             8

#define OpenMode_off         5  /* additional offsets for individual requests */
#define CreateType_off       5
#define CreateSize_off       6
#define CreateInfo_off       7
#define RenameToname_off     5
#define LinkPathname_off     5
#define LinkCap1_off         6
#define LinkCap2_off         7
#define ProtectNewmatrix_off 5
#define RefineAccessMask_off 5
#define SetDateDate_off      7  /* Only interested in Modified */

#define ReadPos_off          0   /* offsets for stream requests */
#define ReadSize_off         1
#define ReadTimeout_off      2
#define WritePos_off         0
#define WriteSize_off        1
#define WriteTimeout_off     2
#define SeekPos_off          0
#define SeekMode_off         1
#define SeekNewPos_off       2
#define SetFileSizeSize_off  0
#define EnableEventsMask_off 0
#define AcknowledgeCount_off 0
#define NegAcknowledgeCount_off 0

#define Reply1_off           0     /* plus offsets for replies */
#define Reply2_off           1
#define Reply3_off           2
#define Reply4_off           3
#define Reply5_off           4
#define Reply6_off           5

#define open_reply           6L    /* size of a reply to open, locate, create */

/*
 * The reply to a read will consist of an arbitrary number of messages
 * containing the requested data, or a failure message. In addition to
 * possible error codes, the FnRc field of these messages will contain
 * a sequence number starting from 16 and incrementing in steps of 16
 * (Thus leaving the lower 4 bits clear). These lower 4 bits contain one
 * of the following values.
 */

#define ReadRc_Mask     0xfL    /* mask for lower 4 bits                */
#define ReadRc_More     0L      /* more data to come                    */
#define ReadRc_EOD      1L      /* end of data                          */
#define ReadRc_EOF      2L      /* end of data and of file              */

#define ReadRc_SeqInc   16L     /* increment for sequence numbers       */

/*
 * The first reply to a write will consist of the following structure
 * telling the sender how to format the data transfer. This is so copies
 * may be eliminated at the server end.
 * Once the data has been sent a second reply is made confirming that the
 * data were received.
 */

#define WriteRc_Done    0L
#define WriteRc_Sizes   1L

/*----------------------------------------------------------------------*/
/* some timeout constants                                               */
/*----------------------------------------------------------------------*/

                                      /* a 30-minute timeout in ticks */
#define DefaultStreamTimeout ((word) 30L * 60L * OneSec)
#define IOCDataMax    512             /* maximum size of full filenames   */
#define ControlMax    16              /* maximum size of a control vector */
#define MAXTIME       0x7FFFFFFFL     /* for timeouts of -1 (infinity)  */


/*----------------------------------------------------------------------*/
/* Object Types                                                         */
/*----------------------------------------------------------------------*/

/* bottom 4 bits are flags for major sub type                           */

#define Type_Directory  1L      /* supports GSP directory interface     */
#define Type_Stream     2L      /* supports GSP stream interface        */
#define Type_Private    4L      /* own private protocol                 */
                                /* remaining bit reserved               */

#define Type_File       0x12L   /*(0x10L+Type_Stream)*/
#define Type_Fifo       0x22L   /*(0x20L+Type_Stream)*/
#define Type_Module     0x32L   /*(0x30L+Type_Stream)*/
#define Type_Program    0x42L   /*(0x40L+Type_Stream)*/
#define Type_Task       0x52L   /*(0x50L+Type_Stream)*/
#define Type_Link       (0x60L)
#define Type_Name       (0x70L)
#define Type_TaskForce  0x81L   /*(0x80|Type_Directory)*/
#define Type_LTaskForce 0x91L   /*(0x90|Type_Directory)*/
#define Type_CacheName  (0xa0L)
#define Type_Pipe       0xb2L   /*(0xb0|Type_Stream)*/
#define Type_Pseudo     0xc2L   /*(0xc0|Type_Stream)*/
#define Type_Device     0xd4L   /*(0xd0|Type_Private)*/
#define Type_Session    0xe0L
#define Type_Socket     0xf0L
/**
*** The reply to Open, Locate and Create requests
**/
typedef struct IOCReply1 {
    WORD        Type;       /* object type code */
    WORD        Flags;      /* flag word        */
    BYTE        Access[8];  /* a capability for it */
    Offset      Pathname;   /* full pathname of object */
    word        Object;     /* object value if no reply port*/
} IOCReply1;


/* end of iogsp.h */

/*------------------------------------------------------------------------
--                                                                      --
--      iosyslib.h                                                      --
--                                                                      --
------------------------------------------------------------------------*/

/*----------------------------------------------------------------------*/
/* Directory Entry structure                                            */
/*----------------------------------------------------------------------*/
typedef struct { word           Type;
                 word           Flags;
                 Matrix         Matrix;
                 char           Name[32];
} DirEntry;

/*----------------------------------------------------------------------*/
/* Generic Object Info structure                                        */
/*----------------------------------------------------------------------*/
/* data structure returned in response to an object info request. The structure
   is returned in the data vector of the message, with all the words swapped as
   necessary to get them to work on the transputer.
*/
typedef struct ObjInfo {
        DirEntry        DirEntry;       /* re-iteration of common info  */
        WORD            Account;        /* accounting identifier        */
        WORD            Size;           /* object size in bytes         */
        time_t          Creation;       /* three date stamps            */
        time_t          Access;
        time_t          Modified;
} ObjInfo;

/*----------------------------------------------------------------------*/
/* Modes for Open                                                       */
/*----------------------------------------------------------------------*/

#define O_ReadOnly      1L              /* For the Helios side */
#define O_WriteOnly     2L
#define O_ReadWrite     3L
#define O_Execute       4L
#define O_Create        0x0100L          /* create if does not exist   */
#define O_Exclusive     0x0200L          /* get exclusive access       */
#define O_Truncate      0x0400L          /* truncate if already exists */
#define O_NonBlock      0x0800L          /* do not block on read/write */
#define O_Append        0x1000L          /* append data (not to be used */
#define O_Sync          0x2000L          /* synchronous transfers */

/* The flags array of the Select call contains the standard bits in the */
/* bottom 2 bits, plus the following bits...                            */

#define O_Exception	0x04		 /* select for exception */
#define O_Selected      0x10             /* set if stream ready  */

/*----------------------------------------------------------------------*/
/* Modes for Seek                                                       */
/*----------------------------------------------------------------------*/

#define S_Beginning     0L              /* relative to start of file    */
#define S_Relative      1L              /* relative to current pos      */
#define S_End           2L              /* relative to end of file      */
#define S_Last          3L              /* relative to last operation   */

/*----------------------------------------------------------------------*/
/* Flag Bits                                                            */
/*----------------------------------------------------------------------*/

/* First column of comments indicates whose responsibility the state    */
/* of the bit is: S = Server, L = Syslib.                               */

#define Flags_Mode         0x0000000fL  /* L copy of open mode      */
#define Flags_More         0x00000010L  /* S More info available    */
#define Flags_Seekable     0x00000020L  /* S stream is seekable     */
#define Flags_Remote       0x00000040L  /* L server is non-local    */
#define Flags_StripName    0x00000080L  /* S names are stripped before pass on */
#define Flags_CacheName    0x00000100L  /* S name is cached         */
#define Flags_LinkName     0x00000200L  /* S name is for h/w link   */
#define Flags_PStream      0x00000400L  /* L set for PseudoStreams  */
#define Flags_ResetContext 0x00000800L  /* S set for remote servers */
#define Flags_Pipe         0x00001000L  /* S set for pipes          */
#define Flags_CloseOnSend  0x00002000L  /* S close in SendEnv       */
#define Flags_OpenOnGet    0x00004000L  /* S open in GetEnv         */
#define Flags_Selectable   0x00008000L  /* S can be used in Select  */

#define Flags_Interactive 0x00010000L   /* S if stream is interactive   */
#define Flags_MSdos       0x00020000L   /* S MSdos format files         */

#define Flags_CloseMask 0xe0000000L     /*   mask for following flags   */
#define Flags_Closeable 0x20000000L     /* S set if needs closing       */
#define Flags_Server    0x40000000L     /* L set if served stream       */
#define Flags_Stream    0x80000000L     /* L set if stream structure    */

#define closebits_(x)    ((((UWORD)(x))>>Flags_CloseShift)&0x7)

/* end of iosyslib.h */

/*------------------------------------------------------------------------
--                                                                      --
--  ioaddon.h                                                           --
--                                                                      --
------------------------------------------------------------------------*/

#define getfnrc(request)   (((request)->MsgHdr).FnRc & FG_Mask)

#define ReqDie             0x00FFFFF0L         /* still necessary */

#define ReplyOK            (Err_Null)

/* The reply to a ServerInfo request */

typedef struct { WORD type;        /* always Type_Directory */
                 WORD size;        /* size in bytes */
                 WORD used;        /* how many used */
                 WORD alloc;       /* unit of allocation in bytes */
} servinfo;

/**
*** These are additional error codes for floppies
**/

#define floppy_invalid   1        /* catch-all error code       */
#define floppy_protected 2        /* write-protected            */
#define floppy_removed   3        /* disk removed during access */
#define floppy_full      4        /* insufficient space on disk */

/**
*** And for printer devices.
**/
#define printer_invalid      0x11      /* catch-all */
#define printer_offline      0x12
#define printer_outofpaper   0x13
#define printer_error        0x14

/* End of ioaddon.h */

/*------------------------------------------------------------------------
--                                                                      --
-- ioevents.h                                                           --
--                                                                      --
------------------------------------------------------------------------*/

/* These are valid types for the Type field of an Event */
#define Event_Mouse            0x1L
#define Event_Keyboard         0x2L
#define Event_Break            0x4L
#define Event_SerialBreak      0x8L
#define Event_ModemRing        0x10L

#define Flag_Buffer            0x80000000L

/* additional reply codes */
#define EventRc_Acknowledge     0x1L   /* the other side should acknowledge   */
#define EventRc_IgnoreLost      0x2L   /* unimportant messages have been lost */

#if (mouse_supported || gem_supported)
/**
*** Potential problem here, with the way shorts are packed into words in
*** data structures.
**/
typedef struct {
#if (ST || AMIGA || SUN3)    /* || TRIPOS  I imagine */
                 SHORT  Y;
                 SHORT  X;
#else
                 SHORT  X;
                 SHORT  Y;
#endif
                 WORD   Buttons;
} Mouse_Event;

#define Buttons_Unchanged       0x00000000L
#define Buttons_Button0_Down    0x00000001L
#define Buttons_Button0_Up      0x00008001L
#define Buttons_Button1_Down    0x00000002L
#define Buttons_Button1_Up      0x00008002L
#define Buttons_Button2_Down    0x00000004L
#define Buttons_Button2_Up      0x00008004L
#define Buttons_Button3_Down    0x00000008L
#define Buttons_Button3_Up      0x00008008L
#endif  /* mouse_supported */

#if keyboard_supported
typedef struct { WORD   Key;
                 WORD   What;
} Keyboard_Event;

#define Keys_KeyUp      1L
#define Keys_KeyDown    2L
#endif /* keyboard_supported */

typedef struct { WORD   junk1;
                 WORD   junk2;
} Break_Event;   /* this is for ctrl-C */

#if RS232_supported
typedef struct { WORD    junk1;
                 WORD    junk2;
} SerialBreak_Event;

typedef struct { WORD    junk1;
                 WORD    junk2;
} ModemRing_Event;
#endif

typedef struct IOevent { WORD Type;
                         WORD Counter;
                         WORD Stamp;
                         union {
#if (mouse_supported || gem_supported)
                         Mouse_Event        Mouse;
#endif
#if keyboard_supported
                         Keyboard_Event     Keyboard;
#endif
                         Break_Event        Break;
#if RS232_supported
                         SerialBreak_Event  RS232_Break;
                         ModemRing_Event    ModemRing;
#endif
                    } Device;
} IOEvent;


                /* this structure is used to keep track of event handlers */
typedef struct { WORD port;
                 WORD *ownedby; /* to keep track of streams */
} event_handler;

/* end of ioevents.h */

/*------------------------------------------------------------------------
--                                                                      --
-- ioconfig.h                                                           --
--                                                                      --
------------------------------------------------------------------------*/

typedef struct Config {
        word    PortTabSize;    /* # slots in port table        */
        word    Incarnation;    /* what booter believes our incarnation is */
        word    loadbase;       /* address at which system was loaded */
        word    ImageSize;      /* size of system image         */
        word    Date;           /* current system date          */
        word    FirstProg;      /* offset of initial program    */
        word    Memory;         /* Size of transputer memory, or 0 */
        word    Flags;          /* Various flags */
        word    Spare;          /* a spare slot                 */
        word    MyName;         /* full path name               */
        word    ParentName;     /* ditto                        */
        word    NLinks;         /* number of links              */
        word    LinkConf[4];    /* NLinks LinkConf structs      */
        char    names[50];
} Config;

#define Config_flags_rootnode   0x01L

/* -- End of ioconfig.h */

/**
*** Link protocol bytes
**/
#define Proto_Write               0
#define Proto_Read                1
#define Proto_Msg                 2
#define Proto_Null                3
#define Proto_Term                4
#define Proto_Reconfigure         5
#define Proto_SecurityCheck       6
#define Proto_Reset               7
#define Proto_Info                0x0F0

#define Proto_Debug               0x064
#define Proto_RemoteReset         0x0F2
#define Proto_RemoteAnalyse       0x0F3
#define Proto_Close               0x0F4
#define Proto_Boot                0x0F5

/**
*** Network control function codes
**/
#define NC_Reset                 0x2010L
#define NC_Analyse               0x2020L
#define NC_Connect               0x2030L
#define NC_Disconnect            0x2040L
#define NC_Enquire               0x2050L

#define ND_INVALID               0x0000L
#define ND_HARDWIRED             0x0001L
#define ND_SOFTWIRED             0x0002L
#define ND_NOCONNECT             0x0003L

/**
*** Bootstrap stuff, used by tload.c and hydra
**/
#define Processor_Trannie   1
#define Processor_Arm       2
#define Processor_i860      3
#define Processor_68000     4
/* and lots of others... */

#define B_Reset_Processor    0x0001
#define B_Send_Bootstrap     0x0002
#define B_Send_Image         0x0004
#define B_Send_Config        0x0008
#define B_Wait_Sync          0x0010
#define B_Check_Processor    0x0020
#define B_Send_Sync          0x0040

#ifdef internet_supported

#ifndef SOL_SOCKET
/*
 * Level number for (get/set)sockopt() to apply to socket itself.
 */
#define	SOL_SOCKET	0xffff		/* options for socket level */
#endif

/*
 * Level number for (get/set)sockopt() to apply to system.
 */
#define SOL_SYSTEM	0xfff0		/* options for system level */

/*
 * Level number for (get/set)sockopt() to apply an ioctl (yuk).
 */
#define SOL_IOCTL	0xfff1		/* options for ioctl level */


#ifndef SO_DEBUG
/*
 * Option flags per-socket.
 */
#define	SO_DEBUG	0x0001		/* turn on debugging info recording */
#define	SO_ACCEPTCONN	0x0002		/* socket has had listen() */
#define	SO_REUSEADDR	0x0004		/* allow local address reuse */
#define	SO_KEEPALIVE	0x0008		/* keep connections alive */
#define	SO_DONTROUTE	0x0010		/* just use interface addresses */
#define	SO_BROADCAST	0x0020		/* permit sending of broadcast msgs */
#define	SO_USELOOPBACK	0x0040		/* bypass hardware when possible */
#define	SO_LINGER	0x0080		/* linger on close if data present */
#define	SO_OOBINLINE	0x0100		/* leave received OOB data in line */

#endif
/*
 * Additional options, not kept in so_options.
 */
#define SO_SNDBUF	0x1001		/* send buffer size */
#define SO_RCVBUF	0x1002		/* receive buffer size */
#define SO_SNDLOWAT	0x1003		/* send low-water mark */
#define SO_RCVLOWAT	0x1004		/* receive low-water mark */
#define SO_SNDTIMEO	0x1005		/* send timeout */
#define SO_RCVTIMEO	0x1006		/* receive timeout */
#define	SO_ERROR	0x1007		/* get error status and clear */
#define	SO_TYPE		0x1008		/* get socket type */

/* Extra options for Helios */
#define	SO_HOSTID	0x8001		/* Host Id (system level)	*/
#define	SO_HOSTNAME	0x8002		/* Host Name (system level)	*/
#define SO_PEERNAME	0x8003		/* Peer name (Get only)		*/
#define SO_SOCKNAME	0x8004		/* socket name (Get only)	*/

#endif
/*------------------------------------------------------------------------
--                                                                      --
--          H E L I O S   I N P U T / O U T P U T   S E R V E R         --
--          ---------------------------------------------------         --
--                                                                      --
--             Copyright (C) 1988, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
--      structs.h                                                       --
--                                                                      --
--         Declarations of the server's manifests and data structures   --
--                                                                      --
--      Author:  BLV 9/6/87                                             --
--                                                                      --
------------------------------------------------------------------------*/
/* RcsId: $Id: structs.h,v 1.3 1991/09/13 14:46:46 martyn Exp $ */
/* Copyright (C) 1987, Perihelion Software Ltd.   			*/


/**
***   The Server makes extensive use of arrays of functions to handle all
***  the Helios requests in a general way, and the following manifests
***  declare offsets within the arrays for these requests.
**/
#define InitServer_off      0
#define TidyServer_off      1
#define Private_off         2
#define Testfun_off         3
#define Open_off            4
#define Create_off          5 
#define Locate_off          6
#define ObjectInfo_off      7
#define ServerInfo_off      8
#define Delete_off          9
#define Rename_off         10
#define Link_off           11
#define Protect_off        12
#define SetDate_off        13
#define Refine_off         14
#define CloseObj_off       15
#define handler_max        16

#define InitStream_off      0
#define TidyStream_off      1
#define StreamPrivate_off   2
#define Read_off            3
#define Write_off           4
#define GetSize_off         5
#define SetSize_off         6
#define Close_off           7
#define Seek_off            8
#define GetAttr_off         9
#define SetAttr_off        10
#define EnableEvents_off   11
#define Acknowledge_off    12
#define NegAcknowledge_off 13
#define Select_off         14
#define Stream_max         15

/**
*** This is returned by get_int_config() in module server.c to indicate failure
**/
#define Invalid_config     0x64928AF2L

/**
*** The main data structure in the Server is the linked list.
**/
typedef struct Node { struct Node *next;       /* next node in list */
                      struct Node *prev;       /* previous node in list */
} Node;

typedef struct List { Node *head;       /* first node in list  */
                      Node *earth;      /* always NULL pointer */
                      Node *tail;       /* last node in list   */
} List;

typedef struct GenData {
                 Node    node;
                 uint    size;
                 int     junk;     /* To guarantee word alignment of data */
                 BYTE    data[1];
} GenData;

typedef struct Semaphore {
                 List    list;
                 int     count;
} Semaphore;

/**                                                                                                             
*** The Server depends heavily on coroutines. All the coroutines are held in
*** linked lists, using the following structure.
**/
typedef struct { Node node;           /* conodes held in linked list  */
                 word id;             /* unique integer identifier    */
                 word type;           /* usually zero, may be suicide */
                                      /* or timedout or ready         */
                 word flags;
                 word timelimit;      /* when stream should die       */
                 ptr  cobase;         /* from CreateCo                */
                 void (**handlers)(); /* request handlers             */
                 ptr  extra;          /* coroutines global data       */
                 char name[128];      /* a name field if required     */
} Conode;

/**
*** Various odds and ends for use with coroutines
**/
#define CoSuicide     (654321L)
#define CoTimeout     (666666L)
#define CoAbortSelect (655321L)
#define CoReady       (777777L)

#define CoFlag_Floppy       0x0001L
#define CoFlag_CtrlCed      0x0002L
#define CoFlag_FileStream   0x0004L
#define CoFlag_Waiting      0x0008L
#define CoFlag_EOFed        0x0010L

/**
*** This structure is used to declare my devices. The type is File, Directory
*** or Private, the name gives the device name e.g. Console, and the handlers
*** field gives the Helios request handlers. I.E. there is an entry in this
*** array to deal with Open requests, another for Locate requests, etc.
**/

typedef struct device_declaration {
       WORD type;
       char *name;
       VoidFnPtr handlers[handler_max];
} device_declaration;

/**
*** The Server contains various directories. In addition to disk directories,
*** the IO processor itself is a directory of servers, and some of these servers
*** such as the communication port servers are directories of ports. Hence I
*** share code to deal with these various directories, possible by keeping the
*** directory in a linked list of ObjNode's. There are a number of
*** supersets of ObjNodes.  DirEntryNode is an alias for ObjNode, for
*** compatibility.
**/
   
typedef struct { Node       node;
                 DirEntry   direntry;
                 WORD       size;
                 WORD       account;
} ObjNode;

#define DirEntryNode ObjNode

typedef struct { List list;
                 WORD entries;
} DirHeader;

/**
*** Communication Port_node's are slightly more complicated because I need
*** additional information.
**/

typedef struct { Attributes     attr;
                 Semaphore      lock;
                 WORD           id;
                 VoidFnPtr      error_fn;
                 VoidFnPtr      done_fn;
                 VoidFnPtr      configure_fn;
                 WordFnPtr      send_fn;
                 WordFnPtr      pollwrite_fn;
                 WordFnPtr      abortwrite_fn;
                 WordFnPtr      receive_fn;
                 WordFnPtr      pollread_fn;
                 WordFnPtr      abortread_fn;
} ComsPort;

typedef struct { ObjNode      objnode;
                 ComsPort     *port;
                 VoidFnPtr    *handlers;
} Port_node;

/**
*** Initially the server boots up a simple network consisting of the root
*** processor and the IO processor. These must be named during the booting
*** processes, so the following manifests define the default names.
**/
#define DefaultServerName "IO"
#define DefaultRootName   "00"
#define slashDefaultServerName "/IO"
#define slashDefaultRootName   "/00"

/**
*** The following structures are used for handling windows and the console
*** device. Microwave is used to handle cooked input processing (pun
*** definitely intended), Screen is used by the ANSI emulator, and Window is
*** used to point at a window structure.
**/
#define       Console_limit 255

typedef struct Microwave {
        UBYTE  buffer[Console_limit+1];/* where data is processed    */
        int    count;                  /* amount in  buffer */
} Microwave;

#define Cooked_EOF  1               /* ctrl-D detected         */
#define Cooked_EOD  2               /* no more data in buffer  */   
#define Cooked_Done 3               /* Read has been satisfied */

#if use_ANSI_emulator
typedef struct Screen { BYTE          **map;
                        BYTE          *whole_screen;
                        int           Rows;
                        int           Cols;
                        int           Cur_x;
                        int           Cur_y;
                        int           mode;
                        int           flags;
                        int           args[5];
                        int           *args_ptr;
                        int           gotdigits;
} Screen;

#define ANSI_in_escape     0x01
#define ANSI_escape_found  0x02
#define ANSI_firstdigit    0x04
#define ANSI_dirty         0x08

#endif

typedef struct Window { ObjNode       node;
                        Attributes    attr;
                        event_handler break_handler;
                        Microwave     cooker;
                        Semaphore     read_lock;
                        Conode        *readerco;
                        Semaphore     write_lock;
                        Conode        *writerco;
#if use_ANSI_emulator
                        Screen        screen;
#endif
                        UBYTE         Table[Console_limit];
                        WORD          handle;
                        int           head, tail;
                        int           XOFF;
#if multi_tasking
                        WORD          any_data;
#endif
} Window;

#define WindowFlags_Deleted  0x00000001L

/**
*** Multi-tasking support. To prevent the server from hogging all the
*** CPU time, the main sources inform the local module whenever it is
*** waiting for particular input, e.g. for a key to be pressed or for
*** the mouse to be moved. Then at a strategic point in the server's main
*** loop I call a multiwait function. This can suspend the CPU for upto
*** half a second or until one of the devices specified is ready.
*** The following constants define the various forms of IO which the
*** server can be waiting on.
**/
#if multi_tasking
#define Multi_LinkMessage          1L
#define Multi_WindowInput          2L
#define Multi_GemInput             3L
#define Multi_MouseInput           4L
#define Multi_KeyboardInput        5L
#define Multi_RS232Event           6L
#define Multi_PortIO               7L
#define Multi_SocketInput          8L
#define Multi_SocketOutput         9L
#define Multi_SocketExcp          10L
#define Multi_StreamInput         11L
#define Multi_StreamOutput        12L
#endif

/**
*** The following structure defines a transputer link.
**/
typedef struct Trans_link {
        WORD    state;
        WORD    fildes;
        WORD    ready;
        WORD    flags;
        WORD    connection;
        BYTE    link_name[32];
} Trans_link;

/**
*** These flags have the following meanings :
*** waiting : there is a Multiwait active on this link
*** free    : the link has been successfully opened etc, and may be used
***           by any software that needs it
*** unused  : the link was not opened successfully, typically because another
***           user has locked it, but it may be useable in the future
*** not_selectable : the link does not support the select call
*** uninitialised  : the link is in an unknown state, it has not been
***           reset since a successful open, so another user may have put
***           it into a funny mode
*** firsttime : the link has never been initialised, so if something goes
***           wrong it is reasonable to display an error message
**/
#define Link_flags_waiting        0x01
#define Link_flags_free           0x02
#define Link_flags_unused         0x04
#if (UNIX)
#define Link_flags_not_selectable 0x010000  
#define Link_flags_uninitialised  0x020000
#define Link_flags_firsttime      0x040000
#endif

#define Link_Reset          1
#define Link_Booting        2
#define Link_Running        3

#if (UNIX)
/**
*** This structure is used between hydra and the server
**/
typedef struct socket_msg {
        long fnrc;
        long extra;
        char userid[16];   /* cannot use l_cuserid, server and hydra may be */
        char hostname[64]; /* compiled with different headers. */
        char linkname[32]; /* name of the link */
} socket_msg;
#define Any_Link           0x12345600
#define Link_Unavailable   0x12345601
#define Invalid_Link       0x12345602
#define Debug_Connection   0x12345603
#define Hydra_Busy         0x12345604

typedef struct debug_msg {
	long fnrc;
        long link;
} debug_msg;
#define Debug_Info       0x12345605
#define Debug_Disconnect 0x12345606
#define Debug_Close      0x12345607
#define Debug_Use        0x12345608
#define Debug_Free       0x12345609

#endif /* UNIX */

#define Mode_Normal      1
#define Mode_Auxiliary   2
#define Mode_Subordinate 3
#define Mode_Remote      4
#define Mode_Daemon      5

/*----------------------------------------------------------------------*/
/* Common substructure for all IOC messages				*/
/*----------------------------------------------------------------------*/

typedef word Struct;
typedef word String;

typedef struct IOCCommon {
	String		Context;	/* offset of context string	*/
	String		Name;		/* offset of object name string	*/
	String		Next;		/* offset of next element in path */
	Capability	Access;		/* capability of context object	*/
} IOCCommon;


/* Messages for Sockets */

typedef struct IOCBind
{
	IOCCommon	Common;
	word		Protocol;
	Struct		Addr;
} IOCBind;

/* Messages for Sockets */

typedef struct AcceptReply
{
	WORD		Type;		/* object type code 		*/
	WORD 		Flags;		/* flag word			*/
	Capability	Access;		/* a capability for it		*/
	String		Pathname;	/* full pathname of object	*/
	Struct		Addr;		/* network address of connector	*/	
} AcceptReply;

typedef struct ConnectRequest {
	Struct		DestAddr;	/* network address for connection */
	Struct		SourceAddr;	/* address of source		  */
} ConnectRequest;

/* This structure is built progressively as it is passed from program to*/
/* program, SendMessage builds it as far as DestAddr, the server adds	*/
/* SourceAddr and passes it back to SendMessage which adds the data and	*/
/* forwards it to RecvMessage.						*/

typedef struct DataGram {
	word		Flags;		/* flag word			*/
	word		DataSize;	/* actual data size		*/
	word		Timeout;	/* time to wait for tfr		*/
	Struct		AccRights;	/* access rights		*/
	Struct		DestAddr;	/* destination address		*/
	Struct		SourceAddr;	/* source address		*/
	Offset		Data;		/* message data			*/
} DataGram;

typedef struct SocketInfoReq
{
	word		Level;		/* option level			*/
	word		Option;		/* option name			*/
	Struct		Optval;		/* option value (optional)	*/
} SocketInfoReq;

/* Any new message structures should be added to the appropriate union here */

typedef struct ReadWrite {
	WORD		Pos;		/* file position		*/
	WORD		Size;		/* size of transfer		*/
	WORD		Timeout;	/* transfer completion time	*/
} ReadWrite;

typedef struct GetSizeReply {
	word		Size;		/* file size in bytes		*/
} GetSizeReply;

typedef struct SeekRequest {
	word		CurPos;		/* current file position	*/
	word		Mode;		/* seek mode			*/
	word		NewPos;		/* new position (rel. to mode)	*/
} SeekRequest;


union StreamRequestSet {
	ReadWrite	ReadWrite;
	SeekRequest	SeekRequest;
	ConnectRequest	ConnectRequest;
	SocketInfoReq	SocketInfoReq;
};

typedef struct SeekReply {
	word		NewPos;		/* new file position		*/
} SeekReply;

typedef struct WriteReply {
	word		first;		/* size of first data message	*/
	word		rest;		/* size of rest of messages	*/
	word		max;		/* max qty of data to send	*/
					/* this is present only in the	*/
					/* extended protocol format	*/
} WriteReply;

union StreamReplySet {
	GetSizeReply	GetSizeReply;
	SeekReply	SeekReply;
	WriteReply	WriteReply;
	AcceptReply	AcceptReply;
};
/*------------------------------------------------------------------------
--                                                                      --
--          H E L I O S   I N P U T / O U T P U T   S E R V E R         --
--          ---------------------------------------------------         --
--                                                                      --
--             Copyright (C) 1987, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
--      fundefs.h                                                       --
--                                                                      --
--         Declarations of the functions shared between modules         --
--                                                                      --
--      Author:  BLV 8/10/87                                            --
--                                                                      --
------------------------------------------------------------------------*/
/* RcsId: $Id: fundefs.h,v 1.4 1991/07/30 10:42:29 martyn Exp $ */
/* Copyright (C) 1987, Perihelion Software Ltd.       			*/

/**
*** Linked list library, the same linked lists as under AmigaDos and Helios
**/
PUBLIC List *fn( MakeHeader,  (void));
PUBLIC void fn(  InitList,    (List *));
PUBLIC void fn(  WalkList,    (List *, VoidFnPtr, ...));
PUBLIC void fn(  FreeList,    (List *));
PUBLIC Node *fn( AddHead,     (Node *, List *));
PUBLIC Node *fn( AddTail,     (Node *, List *));
PUBLIC Node *fn( listRemove,  (Node *));
PUBLIC Node *fn( NextNode,    (Node *));
PUBLIC WORD fn(  Wander,      (List *, WordFnPtr, ...));
PUBLIC WORD fn(  TstList,     (List *));
PUBLIC WORD fn(  ListLength,  (List *));
PUBLIC void fn(  PreInsert,   (Node *, Node *));
PUBLIC void fn(  PostInsert,  (Node *, Node *));

/**
*** There are problems with Remove because all ANSI and pseudo-ANSI libraries
*** including the Microsoft one for the PC have a routine remove(), which
*** clashes with the list Remove with some linkers.
**/
#define Remove listRemove

/**
*** an 8086 has the same byte ordering as the transputer, a 68000 does not
**/
#if swapping_needed
PUBLIC WORD fn( swap, (WORD));
#else
#define swap(a) (a)
#endif

PUBLIC char *fn( get_config,      (char *));
PUBLIC WORD  fn(  get_int_config,  (char *));
PUBLIC WORD  fn(  mystrcmp,        (char *, char *));

#ifndef Daemon_Module

/**
*** A higher level coroutine library using the above linked lists.
**/
PUBLIC Conode *fn( NewCo,     (VoidFnPtr));
PUBLIC void   fn(  TidyColib, (void));
PUBLIC void   fn(  StartCo,   (Conode *));
PUBLIC void   fn(  Suspend,   (void));
PUBLIC void   fn(  Seppuku,   (void));
PUBLIC WORD   fn(  InitColib, (void));
PUBLIC void   fn(  Wait,        (Semaphore *));
PUBLIC void   fn(  Signal,      (Semaphore *));
PUBLIC void   fn( InitSemaphore,(Semaphore *, int));

/**
*** Some general purpose request handlers. For example, doing things like
*** Rename on the Clock is not sensible, so Clock_Rename is #define'd to be
*** Invalidfn_handler
**/
PUBLIC  void fn( Invalidfn_handler,         (Conode *)); 
PUBLIC  void fn( Create_handler,            (Conode *));
PUBLIC  void fn( Dummy_handler,             (Conode *));
PUBLIC  void fn( IgnoreVoid,                (void));
PUBLIC  void fn( Device_ObjectInfo_handler, (Conode *));
PUBLIC  void fn( GetDefaultAttr,            (Conode *));
PUBLIC  void fn( Device_GetSize,            (Conode *));
PUBLIC  void fn( NullFn,                    (Conode *));
PUBLIC  WORD fn( Ignore,                    (void));
PUBLIC  void fn( Forward_handler,           (Conode *));
PUBLIC  void fn( Dir_Locate,                (Conode *));
PUBLIC  void fn( Dir_ObjInfo,               (Conode *));
PUBLIC  void fn( Dir_TidyServer,            (Conode *));
PUBLIC  ObjNode      *fn( Dir_find_node,    (Conode *));
#define Nullfn ((VoidFnPtr) NULL)
PUBLIC  ObjNode      *fn( NewObjNode,       (void));
#define NewDirEntryNode NewObjNode
PUBLIC  void fn( FreeObjNode,               (ObjNode *));
PUBLIC  void fn( Protect_handler,           (Conode *));
PUBLIC  void fn( Refine_handler,            (Conode *));
PUBLIC  void fn( Select_handler,            (Conode *));

/**
*** Some utilities
**/
PUBLIC WORD fn(  convert_name,    (void));
PUBLIC WORD fn(  flatten,         (char *));
PUBLIC WORD fn(  mystrcmp,        (char *, char *));
PUBLIC WORD fn(  get_unix_time,   (void));
PUBLIC void fn(  NewStream,       (WORD, WORD, WORD, VoidFnPtr *));
PUBLIC void fn(  AddAttribute,    (Attributes *, Attribute));
PUBLIC void fn(  RemoveAttribute, (Attributes *, Attribute));
PUBLIC void fn(  SetInputSpeed,   (Attributes *, WORD));
PUBLIC void fn(  SetOutputSpeed,  (Attributes *, WORD));
PUBLIC void fn(  InitAttributes,  (Attributes *));
PUBLIC void fn(  CopyAttributes,  (Attributes *, Attributes *));
PUBLIC WORD fn(  IsAnAttribute,   (Attributes *, Attribute));
PUBLIC WORD fn(  GetInputSpeed,   (Attributes *));
PUBLIC WORD fn(  GetOutputSpeed,  (Attributes *));
PUBLIC WORD fn(  Request_Stat,    (void));
PUBLIC void fn(  Request_Return,  (WORD, WORD, WORD));
PUBLIC void fn(  goto_sleep,      (WORD));
PUBLIC WORD fn(  FormOpenReply,   (WORD, WORD, WORD, WORD));
PUBLIC void fn(  copy_event,      (IOEvent *, IOEvent *));
PUBLIC void fn(  pathcat,         (char *, char *));

/**
*** The various options for console/window devices.
**/
PUBLIC void fn( initialise_windowing, (void));
PUBLIC void fn( restore_windowing,    (void));
PUBLIC void fn( restart_windowing,    (void));

#if !(multiple_windows)

#if use_ANSI_emulator

#define output(a)     ANSI_out(a, &Server_window)
#define window_output ANSI_out 
PUBLIC  void fn( ANSI_out, (char *, Window *));

#if PC
PUBLIC  void fn( send_ch, (int));
#endif
#if ST
#define send_ch(x) Bconout(2, x)
#endif

#else /* use_ANSI_emulator */

#define window_output(a, b)  output(a)
PUBLIC void fn( output, (char *));

#endif /* use_ANSI_emulator */

#else /* multiple_windows */

PUBLIC WORD fn( create_a_window, (char *));
PUBLIC void fn( window_size,     (WORD, WORD *, WORD *));
PUBLIC void fn( close_window,    (WORD));
PUBLIC void fn( send_to_window,  (char *, WORD));
#if use_ANSI_emulator

PUBLIC  void fn( ANSI_out, (char *, Window *));
#define window_output ANSI_out

#else /* use_ANSI_emulator */

#if AMIGA
#define window_output(a, b) send_to_window(a, (b)->handle)
#else
#define window_output send_to_window
#endif

#endif /* use_ANSI_emulator     */

#define output(a) window_output(a, &Server_window)

#endif /* multiple_windows */

PUBLIC void fn( ServerDebug, (char *, ...));
PUBLIC void fn( outputch,    (int, Window *));
#if use_ANSI_emulator
PUBLIC WORD fn( Init_Ansi,   (Window *, WORD, WORD));
PUBLIC void fn( Tidy_Ansi,   (Screen *));
PUBLIC void fn( Resize_Ansi, (Window *, WORD, WORD, WORD));
#endif

/**
*** The following declarations are used in header file server.h and module
*** server.c to indicate the functions available on each device.
*** Many functions are no-ops or cannot be implemented satisfactorily, so
*** they are hash-defined to suitable defaults. Also, some devices can
*** share code to handle particular requests.
**/

PUBLIC  void fn( Helios_InitServer, (Conode *));
#define Drive_InitServer            IgnoreVoid
#define Drive_TidyServer            IgnoreVoid
#define Drive_Private               Invalidfn_handler
#define Drive_Testfun               Nullfn
PUBLIC  void fn( Drive_Locate,      (Conode *));
PUBLIC  void fn( Drive_Open,        (Conode *));
PUBLIC  void fn( Drive_Create,      (Conode *));
PUBLIC  void fn( Drive_Delete,      (Conode *));
PUBLIC  void fn( Drive_ObjectInfo,  (Conode *));
PUBLIC  void fn( Drive_ServerInfo,  (Conode *));
PUBLIC  void fn( Drive_Rename,      (Conode *));
PUBLIC  void fn( Drive_Link,        (Conode *));
#define Drive_Protect               Protect_handler
PUBLIC  void fn( Drive_SetDate,     (Conode *));
#define Drive_Refine                Refine_handler
#define Drive_CloseObj              Invalidfn_handler

PUBLIC  WORD fn( File_InitStream, (Conode *));
PUBLIC  WORD fn( File_TidyStream, (Conode *));
#define File_PrivateStream        Invalidfn_handler
PUBLIC  void fn( File_Read,       (Conode *));
PUBLIC  void fn( File_Write,      (Conode *));
PUBLIC  void fn( File_Close,      (Conode *));
PUBLIC  void fn( File_GetSize,    (Conode *));
#define File_SetSize              Invalidfn_handler
PUBLIC  void fn( File_Seek,       (Conode *));
#define File_GetAttr              GetDefaultAttr
#define File_SetAttr              NullFn
#define File_EnableEvents         Invalidfn_handler
#define File_Acknowledge          Invalidfn_handler
#define File_NegAcknowledge       Invalidfn_handler
#define File_Select               Select_handler

PUBLIC  WORD fn( Dir_InitStream,  (Conode *));
PUBLIC  WORD fn( Dir_TidyStream,  (Conode *));
#define Dir_PrivateStream         Invalidfn_handler
PUBLIC  void fn( Dir_Read,        (Conode *));
#define Dir_Write                 Invalidfn_handler
PUBLIC  void fn( Dir_Close,       (Conode *));
PUBLIC  void fn( Dir_GetSize,     (Conode *));
#define Dir_SetSize               Invalidfn_handler
#define Dir_Seek                  Invalidfn_handler
#define Dir_GetAttr               GetDefaultAttr
#define Dir_SetAttr               NullFn
#define Dir_EnableEvents          Invalidfn_handler
#define Dir_Acknowledge           Invalidfn_handler
#define Dir_NegAcknowledge        Invalidfn_handler
#define Dir_Select                Select_handler

PUBLIC  void fn( IOPROC_InitServer, (Conode *));
PUBLIC  void fn( IOPROC_TidyServer, (Conode *));
PUBLIC  void fn( IOPROC_Private,    (Conode *));
#define IOPROC_Testfun              Nullfn
PUBLIC  void fn( IOPROC_Open,       (Conode *));
PUBLIC  void fn( IOPROC_Locate,     (Conode *));
#define IOPROC_Create               IOPROC_Locate
#define IOPROC_Delete               Forward_handler
PUBLIC  void fn( IOPROC_ObjectInfo, (Conode *));
#define IOPROC_ServerInfo           Forward_handler
PUBLIC  void fn( IOPROC_Rename,     (Conode *));
#define IOPROC_Link                 Forward_handler
#define IOPROC_Protect              Forward_handler
#define IOPROC_SetDate              Forward_handler
#define IOPROC_Refine               Forward_handler
#define IOPROC_CloseObj             Forward_handler

#define IOPROC_InitStream           Ignore
#define IOPROC_TidyStream           Ignore
#define IOPROC_PrivateStream        Invalidfn_handler
#define IOPROC_Read                 Dir_Read
#define IOPROC_Write                Invalidfn_handler
PUBLIC  void fn( IOPROC_Close,      (Conode *));
#define IOPROC_GetSize              Dir_GetSize
#define IOPROC_SetSize              Invalidfn_handler
#define IOPROC_Seek                 Invalidfn_handler
#define IOPROC_GetAttr              GetDefaultAttr
#define IOPROC_SetAttr              NullFn
#define IOPROC_EnableEvents         Invalidfn_handler
#define IOPROC_Acknowledge          IgnoreVoid
#define IOPROC_NegAcknowledge       IgnoreVoid
#define IOPROC_Select               Select_handler

#if multiple_windows
PUBLIC  void fn( Window_InitServer, (Conode *));
PUBLIC  void fn( Window_TidyServer, (Conode *));
#define Window_Private              Invalidfn_handler
PUBLIC  void fn( Window_Testfun,    (bool *));
PUBLIC  void fn( Window_Open,       (Conode *));
#define Window_Locate               Dir_Locate
PUBLIC  void fn( Window_Create,     (Conode *));
PUBLIC  void fn( Window_Delete,     (Conode *));
#define Window_ObjectInfo           Dir_ObjInfo
#define Window_ServerInfo           Invalidfn_handler
#define Window_Rename               Invalidfn_handler
#define Window_Link                 Invalidfn_handler
#define Window_Protect              Protect_handler
#define Window_SetDate              Invalidfn_handler
#define Window_Refine               Refine_handler
#define Window_CloseObj             Invalidfn_handler

#define WindowDir_InitStream        Ignore
#define WindowDir_TidyStream        Ignore
#define WindowDir_PrivateStream     Invalidfn_handler
#define WindowDir_Read              Dir_Read
#define WindowDir_Write             Invalidfn_handler
#define WindowDir_Close             IOPROC_Close
#define WindowDir_GetSize           Dir_GetSize
#define WindowDir_SetSize           Invalidfn_handler
#define WindowDir_Seek              Invalidfn_handler
#define WindowDir_GetAttr           GetDefaultAttr
#define WindowDir_SetAttr           NullFn
#define WindowDir_EnableEvents      Invalidfn_handler
#define WindowDir_Acknowledge       IgnoreVoid
#define WindowDir_NegAcknowledge    IgnoreVoid
#define WindowDir_Select            Select_handler
#endif

/**
*** The console device is always optionally available.
**/

PUBLIC  void fn( Console_InitServer, (Conode *));
#define Console_TidyServer           IgnoreVoid
#define Console_Private              Invalidfn_handler
#if multiple_windows
PUBLIC  void fn( Console_Testfun,    (bool *));
#else
#define Console_Testfun              Nullfn
#endif
PUBLIC  void fn( Console_Open,       (Conode *));
#define Console_Locate               Create_handler
#define Console_Create               Create_handler
#define Console_Delete               Invalidfn_handler
#define Console_ObjectInfo           Device_ObjectInfo_handler
#define Console_ServerInfo           Invalidfn_handler
#define Console_Rename               Invalidfn_handler
#define Console_Link                 Invalidfn_handler
#define Console_Protect              Protect_handler
#define Console_SetDate              Invalidfn_handler
#define Console_Refine               Refine_handler
#define Console_CloseObj             Invalidfn_handler

PUBLIC  WORD fn( Console_InitStream,   (Conode *));
PUBLIC  WORD fn( Console_TidyStream,   (Conode *));
#define Console_PrivateStream          Invalidfn_handler
PUBLIC  void fn( Console_Read,         (Conode *));
PUBLIC  void fn( Console_Write,        (Conode *));
PUBLIC  void fn( Console_Close,        (Conode *));
#define Console_GetSize                Device_GetSize
#define Console_SetSize                Invalidfn_handler
#define Console_Seek                   Invalidfn_handler
PUBLIC  void fn( Console_GetAttr,      (Conode *));
PUBLIC  void fn( Console_SetAttr,      (Conode *));
PUBLIC  void fn( Console_EnableEvents, (Conode *));
#define Console_Acknowledge            IgnoreVoid
#define Console_NegAcknowledge         IgnoreVoid
PUBLIC  void fn(Console_Select,	       (Conode*));

PUBLIC  void fn( write_to_log,      (char *));
PUBLIC  void fn( init_logger,       (void));
PUBLIC  void fn( tidy_logger,       (void));

#define Logger_InitServer           IgnoreVoid
#define Logger_TidyServer           IgnoreVoid
#define Logger_Private              Invalidfn_handler
#define Logger_Testfun              Nullfn
PUBLIC  void fn( Logger_Open,       (Conode *));
#define Logger_Locate               Create_handler
#define Logger_Create               Create_handler
PUBLIC  void fn( Logger_Delete,     (Conode *));
PUBLIC  void fn( Logger_ObjectInfo, (Conode *));
#define Logger_ServerInfo           Invalidfn_handler
#define Logger_Rename               Invalidfn_handler
#define Logger_Link                 Invalidfn_handler
#define Logger_Protect              Protect_handler
#define Logger_SetDate              Invalidfn_handler
#define Logger_Refine               Refine_handler
#define Logger_CloseObj             Invalidfn_handler

#define Logger_InitStream             Ignore
#define Logger_TidyStream             Ignore
PUBLIC  void fn( Logger_PrivateStream,(Conode *));
PUBLIC  void fn( Logger_Read,         (Conode *));
PUBLIC  void fn( Logger_Write,        (Conode *));
PUBLIC  void fn( Logger_Close,        (Conode *));
PUBLIC  void fn( Logger_GetSize,      (Conode *));
#define Logger_SetSize                Invalidfn_handler
PUBLIC  void fn( Logger_Seek,         (Conode *));
#define Logger_GetAttr                Invalidfn_handler
#define Logger_SetAttr                Invalidfn_handler
#define Logger_EnableEvents           Invalidfn_handler
#define Logger_Acknowledge            IgnoreVoid
#define Logger_NegAcknowledge         IgnoreVoid
#define Logger_Select                 Select_handler

#define Clock_InitServer           IgnoreVoid
#define Clock_TidyServer           IgnoreVoid
#define Clock_Private              Invalidfn_handler
#define Clock_Testfun              Nullfn
#define Clock_Locate               Create_handler
#define Clock_Open                 Invalidfn_handler
#define Clock_Create               Create_handler
#define Clock_Delete               Invalidfn_handler
PUBLIC  void fn( Clock_ObjectInfo, (Conode *));
#define Clock_ServerInfo           Invalidfn_handler
#define Clock_Rename               Invalidfn_handler
#define Clock_Link                 Invalidfn_handler
#define Clock_Protect              Protect_handler
#if (UNIX || MAC)
#define Clock_SetDate              Invalidfn_handler
#else
PUBLIC  void fn( Clock_SetDate,    (Conode *));
#endif
#define Clock_Refine               Refine_handler
#define Clock_CloseObj             Invalidfn_handler

#if interaction_supported
#define Host_InitServer              IgnoreVoid
#define Host_TidyServer              IgnoreVoid
#define Host_Private                 Invalidfn_handler
#define Host_Testfun                 Nullfn
#define Host_Locate                  Create_handler
PUBLIC  void fn( Host_Open,          (Conode *));
#define Host_Create                  Create_handler
#define Host_Delete                  Invalidfn_handler
#define Host_ObjectInfo              Device_ObjectInfo_handler
#define Host_ServerInfo              Invalidfn_handler
#define Host_Rename                  Invalidfn_handler
#define Host_Link                    Invalidfn_handler
#define Host_Protect                 Protect_handler
#define Host_SetDate                 Invalidfn_handler
#define Host_Refine                  Refine_handler
#define Host_CloseObj                Invalidfn_handler

#define Host_InitStream              Ignore
#define Host_TidyStream              Ignore
PUBLIC  void fn( Host_PrivateStream, (Conode *));
#define Host_Read                    Invalidfn_handler 
#define Host_Write                   Invalidfn_handler
PUBLIC  void fn( Host_Close,         (Conode *));
#define Host_GetSize                 Invalidfn_handler
#define Host_SetSize                 Invalidfn_handler
#define Host_Seek                    Invalidfn_handler
#define Host_GetAttr                 Invalidfn_handler
#define Host_SetAttr                 Invalidfn_handler
#define Host_EnableEvents            Invalidfn_handler
#define Host_Acknowledge             IgnoreVoid
#define Host_NegAcknowledge          IgnoreVoid
#define Host_Select                  Select_handler

#endif /* interaction_supported */

#if gem_supported
PUBLIC  void fn( Gem_InitServer,     (Conode *));
PUBLIC  void fn( Gem_TidyServer,     (Conode *));
#define Gem_Private                  Invalidfn_handler
#if Gem_Always_Available
#define Gem_Testfun                  Nullfn
#else
PUBLIC  void fn( Gem_Testfun,        (bool *));
#endif
#define Gem_Locate                   Create_handler
PUBLIC  void fn( Gem_Open,           (Conode *));
#define Gem_Create                   Create_handler
#define Gem_Delete                   Invalidfn_handler
#define Gem_ObjectInfo               Device_ObjectInfo_handler
#define Gem_ServerInfo               Invalidfn_handler
#define Gem_Rename                   Invalidfn_handler
#define Gem_Link                     Invalidfn_handler
#define Gem_Protect                  Protect_handler
#define Gem_SetDate                  Invalidfn_handler
#define Gem_Refine                   Refine_handler
#define Gem_CloseObj                 Invalidfn_handler

PUBLIC  WORD fn( Gem_InitStream,     (Conode *));
#define Gem_TidyStream               Ignore
PUBLIC  void fn( Gem_PrivateStream,  (Conode *));
#define Gem_Read                     Invalidfn_handler
#define Gem_Write                    Invalidfn_handler
PUBLIC  void fn( Gem_Close,          (Conode *));
#define Gem_GetSize                  Invalidfn_handler
#define Gem_SetSize                  Invalidfn_handler
#define Gem_Seek                     Invalidfn_handler
#define Gem_GetAttr                  Invalidfn_handler
#define Gem_SetAttr                  Invalidfn_handler
PUBLIC  void fn( Gem_EnableEvents,   (Conode *));
#define Gem_Acknowledge              IgnoreVoid
#define Gem_NegAcknowledge           IgnoreVoid
#define Gem_Select                   Select_handler

PUBLIC void fn( poll_gem,    (void));
PUBLIC void fn( restart_gem, (void));
PUBLIC void fn( vdi,         (int **));
#endif /* gem_supported */

/**
*** The RS232, Centronics Midi and Printer devices share most of their code
**/
#if Ports_used
PUBLIC void fn( Port_Open,       (Conode *));
PUBLIC void fn( Port_TidyServer, (Conode *));
PUBLIC void fn( Port_Close,      (Conode *));
PUBLIC void fn( Port_Rename,     (Conode *));
PUBLIC void fn( Port_Read,       (Conode *));
PUBLIC void fn( Port_Write,      (Conode *));
PUBLIC void fn( Port_GetAttr,    (Conode *));
PUBLIC void fn( Port_SetAttr,    (Conode *));

#define PortDir_InitStream       Ignore
#define PortDir_TidyStream       Ignore
#define PortDir_PrivateStream    Invalidfn_handler
#define PortDir_Read             Dir_Read
#define PortDir_Write            Invalidfn_handler
#define PortDir_Close            IOPROC_Close
#define PortDir_GetSize          Dir_GetSize
#define PortDir_SetSize          Invalidfn_handler
#define PortDir_Seek             Invalidfn_handler
#define PortDir_GetAttr          GetDefaultAttr
#define PortDir_SetAttr          NullFn
#define PortDir_EnableEvents     Invalidfn_handler
#define PortDir_Acknowledge      IgnoreVoid
#define PortDir_NegAcknowledge   IgnoreVoid
#define PortDir_Select           Select_handler

#endif

#if RS232_supported
PUBLIC  void fn( RS232_InitServer,   (Conode *));
PUBLIC  void fn( RS232_TidyServer,   (Conode *));
#define RS232_Private                Invalidfn_handler
#if RS232_Always_Available
#define RS232_Testfun                Nullfn
#else
PUBLIC  void fn( RS232_Testfun,      (bool *));
#endif
#define RS232_Locate                 Dir_Locate
#define RS232_Open                   Port_Open
#define RS232_Create                 Dir_Locate
#define RS232_Delete                 Invalidfn_handler
#define RS232_ObjectInfo             Dir_ObjInfo
#define RS232_ServerInfo             Invalidfn_handler
#define RS232_Rename                 Port_Rename
#define RS232_Link                   Invalidfn_handler
#define RS232_Protect                Protect_handler
#define RS232_SetDate                Invalidfn_handler
#define RS232_Refine                 Refine_handler
#define RS232_CloseObj               Invalidfn_handler
#define RS232_InitStream             Ignore
#define RS232_TidyStream             Ignore
#define RS232_PrivateStream          Invalidfn_handler
#define RS232_Read                   Port_Read
#define RS232_Write                  Port_Write
#define RS232_Close                  Port_Close
#define RS232_GetSize                Device_GetSize
#define RS232_SetSize                Invalidfn_handler
#define RS232_Seek                   Invalidfn_handler
#define RS232_GetAttr                Port_GetAttr
#define RS232_SetAttr                Port_SetAttr
PUBLIC  void fn( RS232_EnableEvents, (Conode *));
#define RS232_Acknowledge            IgnoreVoid
#define RS232_NegAcknowledge         IgnoreVoid
#define RS232_Select                 Select_handler

PUBLIC  WORD fn( RS232_initlist,       (List *, ComsPort **));
PUBLIC  void fn( RS232_check_events,   (void));
PUBLIC  void fn( RS232_disable_events, (ComsPort *));
PUBLIC  void fn( RS232_enable_events,  (ComsPort *, WORD, WORD));
#endif

#if Centronics_supported
PUBLIC  void fn( Centronics_InitServer, (Conode *));
#define Centronics_TidyServer           Port_TidyServer
#define Centronics_Private              Invalidfn_handler
#if Centronics_Always_Available
#define Centronics_Testfun              Nullfn
#else
PUBLIC  void fn( Centronics_Testfun,    (bool *));
#endif
#define Centronics_Locate               Dir_Locate
#define Centronics_Open                 Port_Open
#define Centronics_Create               Dir_Locate
#define Centronics_Delete               Invalidfn_handler
#define Centronics_ObjectInfo           Dir_ObjInfo
#define Centronics_ServerInfo           Invalidfn_handler
#define Centronics_Rename               Port_Rename
#define Centronics_Link                 Invalidfn_handler
#define Centronics_Protect              Protect_handler
#define Centronics_SetDate              Invalidfn_handler
#define Centronics_Refine               Refine_handler
#define Centronics_CloseObj             Invalidfn_handler

#define Centronics_InitStream           Ignore
#define Centronics_TidyStream           Ignore
#define Centronics_PrivateStream        Invalidfn_handler
#if Centronics_readable
#define Centronics_Read                 Port_Read
#else
#define Centronics_Read                 Invalidfn_handler
#endif
#define Centronics_Write                Port_Write
#define Centronics_Close                Port_Close
#define Centronics_GetSize              Device_GetSize
#define Centronics_SetSize              Invalidfn_handler
#define Centronics_Seek                 Invalidfn_handler
#define Centronics_GetAttr              GetDefaultAttr
#define Centronics_SetAttr              NullFn
#define Centronics_EnableEvents         Invalidfn_handler
#define Centronics_Acknowledge          IgnoreVoid
#define Centronics_NegAcknowledge       IgnoreVoid
#define Centronics_Select               Select_handler

PUBLIC  WORD fn( Centronics_initlist,   (List *, ComsPort **));
#endif

#if Printer_supported
PUBLIC  void fn( Printer_InitServer,    (Conode *));
#define Printer_TidyServer              Port_TidyServer
#define Printer_Private                 Invalidfn_handler
#if Printer_Always_Available
#define Printer_Testfun                 Nullfn
#else
PUBLIC  void fn( Printer_Testfun,       (bool *));
#endif
#define Printer_Locate                  Dir_Locate
#define Printer_Open                    Port_Open
#define Printer_Create                  Dir_Locate
#define Printer_Delete                  Invalidfn_handler
#define Printer_ObjectInfo              Dir_ObjInfo
#define Printer_ServerInfo              Invalidfn_handler
#define Printer_Rename                  Port_Rename
#define Printer_Link                    Invalidfn_handler
#define Printer_Protect                 Protect_handler
#define Printer_SetDate                 Invalidfn_handler
#define Printer_Refine                  Refine_handler
#define Printer_CloseObj                Invalidfn_handler

#define Printer_InitStream              Ignore
#define Printer_TidyStream              Ignore
#define Printer_PrivateStream           Invalidfn_handler
#define Printer_Read                    Invalidfn_handler
#define Printer_Write                   Port_Write
#define Printer_Close                   Port_Close
#define Printer_GetSize                 Device_GetSize
#define Printer_SetSize                 Invalidfn_handler
#define Printer_Seek                    Invalidfn_handler
#define Printer_GetAttr                 Port_GetAttr
#define Printer_SetAttr                 Port_SetAttr
#define Printer_EnableEvents            Invalidfn_handler
#define Printer_Acknowledge             IgnoreVoid
#define Printer_NegAcknowledge          IgnoreVoid
#define Printer_Select                  Select_handler

PUBLIC  WORD fn( Printer_initlist,      (List *, ComsPort **));
#endif

#if Midi_supported
PUBLIC  void fn( Midi_InitServer,       (Conode *));
#define Midi_TidyServer                 Port_TidyServer
#define Midi_Private                    Invalidfn_handler
#if Midi_Always_Available
#define Midi_Testfun                    Nullfn
#else
PUBLIC  void fn( Midi_Testfun,          (bool *));
#endif
#define Midi_Locate                     Dir_Locate
#define Midi_Open                       Port_Open
#define Midi_Create                     Dir_Locate
#define Midi_Delete                     Invalidfn_handler
#define Midi_ObjectInfo                 Dir_ObjInfo
#define Midi_ServerInfo                 Invalidfn_handler
#define Midi_Rename                     Port_Rename
#define Midi_Link                       Invalidfn_handler
#define Midi_Protect                    Protect_handler
#define Midi_SetDate                    Invalidfn_handler
#define Midi_Refine                     Refine_handler
#define Midi_CloseObj                   Invalidfn_handler

#define Midi_InitStream                 Ignore
#define Midi_TidyStream                 Ignore
#define Midi_PrivateStream              Invalidfn_handler
#define Midi_Read                       Invalidfn_handler
#define Midi_Write                      Port_Write
#define Midi_Close                      Port_Close
#define Midi_GetSize                    Device_GetSize
#define Midi_SetSize                    Invalidfn_handler
#define Midi_Seek                       Invalidfn_handler
#define Midi_GetAttr                    Port_GetAttr
#define Midi_SetAttr                    Port_SetAttr
#define Midi_EnableEvents               Invalidfn_handler
#define Midi_Acknowledge                IgnoreVoid
#define Midi_NegAcknowledge             IgnoreVoid
#define Midi_Select                     Select_handler

PUBLIC  WORD fn( Midi_initlist,         (List *, ComsPort **));
#endif

#if Ether_supported
extern  void fn( Ether_InitServer,      (Conode *));
extern  void fn( Ether_TidyServer,      (Conode *));
#define Ether_Private                   Invalidfn_handler
extern  void fn( Ether_Testfun,         (bool *));
extern  void fn( Ether_Open,            (Conode *));
#define Ether_Locate                    Create_handler
#define Ether_Create                    Create_handler
#define Ether_Delete                    Invalidfn_handler
#define Ether_ObjectInfo                Device_ObjectInfo_handler
#define Ether_ServerInfo                Invalidfn_handler
#define Ether_Rename                    Invalidfn_handler
#define Ether_Link                      Invalidfn_handler
#define Ether_Protect                   Invalidfn_handler
#define Ether_SetDate                   Invalidfn_handler
#define Ether_Refine                    Invalidfn_handler
#define Ether_CloseObj                  Invalidfn_handler

#define Ether_InitStream                Ignore
#define Ether_TidyStream                Ignore
#define Ether_PrivateStream             Invalidfn_handler
extern  void fn( Ether_Read,            (Conode *));
extern  void fn( Ether_Write,           (Conode *));
extern  void fn( Ether_Close,           (Conode *));
#define Ether_GetSize                   Invalidfn_handler
#define Ether_SetSize                   Invalidfn_handler
#define Ether_Seek                      Invalidfn_handler
extern  void fn( Ether_GetAttr,         (Conode *));
extern  void fn( Ether_SetAttr,         (Conode *));
#define Ether_EnableEvents              Invalidfn_handler
#define Ether_Acknowledge               IgnoreVoid
#define Ether_NegAcknowledge            IgnoreVoid
#define Ether_Select                    Select_handler
#endif

#if Rawdisk_supported
PUBLIC  void fn( RawDisk_InitServer,    (Conode *));
#define RawDisk_TidyServer              Dir_TidyServer
#define RawDisk_Private                 Invalidfn_handler
PUBLIC  void fn( RawDisk_Testfun,       (bool *));
PUBLIC  void fn( RawDisk_Open,          (Conode *));
#define RawDisk_Locate                  Dir_Locate
#define RawDisk_Create                  Dir_Locate
#define RawDisk_Delete                  Invalidfn_handler
#define RawDisk_ObjectInfo              Dir_ObjInfo
#define RawDisk_ServerInfo              Invalidfn_handler
#define RawDisk_Rename                  Invalidfn_handler
#define RawDisk_Link                    Invalidfn_handler
#define RawDisk_Protect                 Protect_handler
#define RawDisk_SetDate                 Invalidfn_handler
#define RawDisk_Refine                  Refine_handler
#define RawDisk_CloseObj                Invalidfn_handler

#define RawDisk_InitStream         Ignore
#define RawDisk_TidyStream         Ignore
#define RawDisk_PrivateStream      Invalidfn_handler
PUBLIC  void fn( RawDisk_Read,     (Conode *));
PUBLIC  void fn( RawDisk_Write,    (Conode *));
PUBLIC  void fn( RawDisk_Close,    (Conode *));
PUBLIC  void fn( RawDisk_GetSize,  (Conode *));
#define RawDisk_SetSize            Invalidfn_handler
PUBLIC  void fn( RawDisk_Seek,     (Conode *));
#define RawDisk_GetAttr            Invalidfn_handler
#define RawDisk_SetAttr            Invalidfn_handler
#define RawDisk_EnableEvents       Invalidfn_handler
#define RawDisk_Acknowledge        IgnoreVoid
#define RawDisk_NegAcknowledge     IgnoreVoid
#define RawDisk_Select             Select_handler

#endif
#if Romdisk_supported
PUBLIC  void fn( RomDisk_InitServer,    (Conode *));
#define RomDisk_TidyServer              Dir_TidyServer
#define RomDisk_Private                 Invalidfn_handler
PUBLIC  void fn( RomDisk_Testfun,       (bool *));
PUBLIC  void fn( RomDisk_Open,          (Conode *));
#define RomDisk_Locate                  Dir_Locate
#define RomDisk_Create                  Dir_Locate
#define RomDisk_Delete                  Invalidfn_handler
#define RomDisk_ObjectInfo              Dir_ObjInfo
#define RomDisk_ServerInfo              Invalidfn_handler
#define RomDisk_Rename                  Invalidfn_handler
#define RomDisk_Link                    Invalidfn_handler
#define RomDisk_Protect                 Protect_handler
#define RomDisk_SetDate                 Invalidfn_handler
#define RomDisk_Refine                  Refine_handler
#define RomDisk_CloseObj                Invalidfn_handler

#define RomDisk_InitStream         Ignore
#define RomDisk_TidyStream         Ignore
#define RomDisk_PrivateStream      Invalidfn_handler
PUBLIC  void fn( RomDisk_Read,     (Conode *));
#define RomDisk_Write      	   Invalidfn_handler
PUBLIC  void fn( RomDisk_Close,    (Conode *));
PUBLIC  void fn( RomDisk_GetSize,  (Conode *));
#define RomDisk_SetSize            Invalidfn_handler
#define RomDisk_Seek           	   Invalidfn_handler
#define RomDisk_GetAttr            Invalidfn_handler
#define RomDisk_SetAttr            Invalidfn_handler
#define RomDisk_EnableEvents       Invalidfn_handler
#define RomDisk_Acknowledge        IgnoreVoid
#define RomDisk_NegAcknowledge     IgnoreVoid
#define RomDisk_Select             Select_handler

#endif

#if mouse_supported
PUBLIC  void fn( Mouse_InitServer,      (Conode *));
PUBLIC  void fn( Mouse_TidyServer,      (Conode *));
#define Mouse_Private                   Invalidfn_handler
#if Mouse_Always_Available
#define Mouse_Testfun                   Nullfn
#else                                                                                                                                          
PUBLIC  void fn( Mouse_Testfun,         (bool *));
#endif
PUBLIC  void fn( Mouse_Open,            (Conode *));
#define Mouse_Locate                    Create_handler
#define Mouse_Create                    Create_handler
#define Mouse_Delete                    Invalidfn_handler
#define Mouse_ObjectInfo                Device_ObjectInfo_handler
#define Mouse_ServerInfo                Invalidfn_handler
#define Mouse_Rename                    Invalidfn_handler
#define Mouse_Link                      Invalidfn_handler
#define Mouse_Protect                   Protect_handler
#define Mouse_SetDate                   Invalidfn_handler
#define Mouse_Refine                    Refine_handler
#define Mouse_CloseObj                  Invalidfn_handler

PUBLIC  WORD fn( Mouse_InitStream,      (Conode *));
PUBLIC  WORD fn( Mouse_TidyStream,      (Conode *));
#define Mouse_PrivateStream             Invalidfn_handler
#define Mouse_Read                      Invalidfn_handler
#define Mouse_Write                     Invalidfn_handler
PUBLIC  void fn( Mouse_Close,           (Conode *));
#define Mouse_GetSize                   Invalidfn_handler
#define Mouse_SetSize                   Invalidfn_handler
#define Mouse_Seek                      Invalidfn_handler
#define Mouse_GetAttr                   Invalidfn_handler
#define Mouse_SetAttr                   Invalidfn_handler
PUBLIC  void fn( Mouse_EnableEvents,    (Conode *));
PUBLIC  void fn( Mouse_Acknowledge,     (Conode *));
PUBLIC  void fn( Mouse_NegAcknowledge,  (Conode *));
#define Mouse_Select                    Select_handler

PUBLIC  void fn( initialise_mouse,      (void));
PUBLIC  void fn( tidy_mouse,            (void));
PUBLIC  void fn( start_mouse,           (void));
PUBLIC  void fn( stop_mouse,            (void));
#endif

#if keyboard_supported
PUBLIC  void fn( Keyboard_InitServer,     (Conode *));
PUBLIC  void fn( Keyboard_TidyServer,     (Conode *));
#define Keyboard_Private                  Invalidfn_handler
#if Keyboard_Always_Available
#define Keyboard_Testfun                  Nullfn
#else
PUBLIC  void fn( Keyboard_Testfun,        (bool *));
#endif
PUBLIC  void fn( Keyboard_Open,           (Conode *));
#define Keyboard_Locate                   Create_handler
#define Keyboard_Create                   Create_handler
#define Keyboard_Delete                   Invalidfn_handler
#define Keyboard_ObjectInfo               Device_ObjectInfo_handler
#define Keyboard_ServerInfo               Invalidfn_handler
#define Keyboard_Rename                   Invalidfn_handler
#define Keyboard_Link                     Invalidfn_handler
#define Keyboard_Protect                  Protect_handler
#define Keyboard_SetDate                  Invalidfn_handler
#define Keyboard_Refine                   Refine_handler
#define Keyboard_CloseObj                 Invalidfn_handler
PUBLIC  WORD fn( Keyboard_InitStream,     (Conode *));
PUBLIC  WORD fn( Keyboard_TidyStream,     (Conode *));
#define Keyboard_PrivateStream            Invalidfn_handler
#define Keyboard_Read                     Invalidfn_handler
#define Keyboard_Write                    Invalidfn_handler
PUBLIC  void fn( Keyboard_Close,          (Conode *));
#define Keyboard_GetSize                  Invalidfn_handler
#define Keyboard_SetSize                  Invalidfn_handler
#define Keyboard_Seek                     Invalidfn_handler
#define Keyboard_GetAttr                  Invalidfn_handler
#define Keyboard_SetAttr                  Invalidfn_handler
PUBLIC  void fn( Keyboard_EnableEvents,   (Conode *));
PUBLIC  void fn( Keyboard_Acknowledge,    (Conode *));
PUBLIC  void fn( Keyboard_NegAcknowledge, (Conode *));
#define Keyboard_Select                   Select_handler

PUBLIC  void fn( initialise_keyboard,     (void));
PUBLIC  void fn( tidy_keyboard,           (void));
PUBLIC  void fn( start_keyboard,          (void));
PUBLIC  void fn( stop_keyboard,           (void));
#endif

#if X_supported
PUBLIC  void fn( X_InitServer, (Conode *));
PUBLIC  void fn( X_TidyServer, (Conode *));
#define X_Private              Invalidfn_handler
PUBLIC  void fn( X_Testfun,    (bool *));
PUBLIC  void fn( X_Open,       (Conode *));
#define X_Locate               Dir_Locate
PUBLIC  void fn( X_Create,     (Conode *));
PUBLIC  void fn( X_Delete,     (Conode *));
#define X_ObjectInfo           Dir_ObjInfo
#define X_ServerInfo           Invalidfn_handler
#define X_Rename               Invalidfn_handler
#define X_Link                 Invalidfn_handler
#define X_Protect              Protect_handler
#define X_SetDate              Invalidfn_handler
#define X_Refine               Refine_handler
#define X_CloseObj             Invalidfn_handler

#define XDir_InitStream        Ignore
#define XDir_TidyStream        Ignore
#define XDir_PrivateStream     Invalidfn_handler
#define XDir_Read              Dir_Read
#define XDir_Write             Invalidfn_handler
#define XDir_Close             IOPROC_Close
#define XDir_GetSize           Dir_GetSize
#define XDir_SetSize           Invalidfn_handler
#define XDir_Seek              Invalidfn_handler
#define XDir_GetAttr           GetDefaultAttr
#define XDir_SetAttr           NullFn
#define XDir_EnableEvents      Invalidfn_handler
#define XDir_Acknowledge       IgnoreVoid
#define XDir_NegAcknowledge    IgnoreVoid
#define XDir_Select            Select_handler

PUBLIC  WORD fn( X_InitStream,   (Conode *));
PUBLIC  WORD fn( X_TidyStream,   (Conode *));
#define X_PrivateStream          Invalidfn_handler
PUBLIC  void fn( X_Read,         (Conode *));
PUBLIC  void fn( X_Write,        (Conode *));
PUBLIC  void fn( X_Close,        (Conode *));
#define X_GetSize                Device_GetSize
#define X_SetSize                Invalidfn_handler
#define X_Seek                   Invalidfn_handler
#define X_GetAttr                GetDefaultAttr
#define X_SetAttr                NullFn
#define X_EnableEvents           Invalidfn_handler
#define X_Acknowledge            IgnoreVoid
#define X_NegAcknowledge         IgnoreVoid
#define X_Select                 Select_handler

#endif

#if Network_supported
#define Network_InitServer              IgnoreVoid
#define Network_TidyServer              IgnoreVoid
#define Network_Private                 Invalidfn_handler
#define Network_Testfun                 Nullfn
#define Network_Locate                  Create_handler
PUBLIC  void fn( Network_Open,          (Conode *));
#define Network_Create                  Create_handler
#define Network_Delete                  Invalidfn_handler
#define Network_ObjectInfo              Device_ObjectInfo_handler
#define Network_ServerInfo              Invalidfn_handler
#define Network_Rename                  Invalidfn_handler
#define Network_Link                    Invalidfn_handler
#define Network_Protect                 Protect_handler
#define Network_SetDate                 Invalidfn_handler
#define Network_Refine                  Refine_handler
#define Network_CloseObj                Invalidfn_handler

#define Network_InitStream              Ignore
#define Network_TidyStream              Ignore
PUBLIC  void fn( Network_PrivateStream, (Conode *));
#define Network_Read                    Invalidfn_handler 
#define Network_Write                   Invalidfn_handler
PUBLIC  void fn( Network_Close,         (Conode *));
#define Network_GetSize                 Invalidfn_handler
#define Network_SetSize                 Invalidfn_handler
#define Network_Seek                    Invalidfn_handler
#define Network_GetAttr                 Invalidfn_handler
#define Network_SetAttr                 Invalidfn_handler
#define Network_EnableEvents            Invalidfn_handler
#define Network_Acknowledge             IgnoreVoid
#define Network_NegAcknowledge          IgnoreVoid
#define Network_Select                  Select_handler

#endif /* Network_supported */

#if internet_supported
PUBLIC  void fn( Internet_InitServer, (Conode *));
PUBLIC  void fn( Internet_TidyServer, (Conode *));
#define Internet_Private              Internet_PrivateStream
#define Internet_Testfun              NullFn
PUBLIC  void fn( Internet_Open,       (Conode *));
#define Internet_Create               Invalidfn_handler
#define Internet_Locate               Dir_Locate
PUBLIC  void fn( Internet_ObjectInfo, (Conode *));
#define Internet_ServerInfo           Invalidfn_handler
#define Internet_Delete               Invalidfn_handler
#define Internet_Rename               Invalidfn_handler
#define Internet_Link                 Invalidfn_handler
#define Internet_Protect              Protect_handler
#define Internet_SetDate              Invalidfn_handler
#define Internet_Refine               Refine_handler
#define Internet_CloseObj             Invalidfn_handler
#define Internet_Revoke               Invalidfn_handler

#define InternetDir_InitStream        Ignore
#define InternetDir_TidyStream        Ignore
#define InternetDir_PrivateStream     Invalidfn_handler
#define InternetDir_Read              Dir_Read
#define InternetDir_Write             Invalidfn_handler
#define InternetDir_Close             IOPROC_Close
#define InternetDir_GetSize           Dir_GetSize
#define InternetDir_SetSize           Invalidfn_handler
#define InternetDir_Seek              Invalidfn_handler
#define InternetDir_GetAttr           GetDefaultAttr
#define InternetDir_SetAttr           NullFn
#define InternetDir_EnableEvents      Invalidfn_handler
#define InternetDir_Acknowledge       IgnoreVoid
#define InternetDir_NegAcknowledge    IgnoreVoid
#define InternetDir_Select            Select_handler

PUBLIC  WORD fn( Internet_InitStream,   (Conode *));
#define Internet_TidyStream             Ignore
PUBLIC  void fn( Internet_PrivateStream,(Conode *));
PUBLIC  void fn( Internet_Read,         (Conode *));
PUBLIC  void fn( Internet_Write,        (Conode *));
PUBLIC  void fn( Internet_Close,        (Conode *));
PUBLIC  void fn( Internet_GetSize,      (Conode *));
PUBLIC  void fn( Internet_SetSize,      (Conode *));
#define Internet_Seek                   Invalidfn_handler
PUBLIC  void fn( Internet_GetInfo,      (Conode *));
PUBLIC  void fn( Internet_SetInfo,      (Conode *));
#define Internet_EnableEvents           Invalidfn_handler
#define Internet_Acknowledge            IgnoreVoid
#define Internet_NegAcknowledge         IgnoreVoid
PUBLIC  void fn( Internet_Select,       (Conode *));
#endif

/**
*** Explicit declarations of the memory allocation functions
***
*** For most of the hardware on which the server is expected to run malloc()
*** takes an unsigned integer as argument, which may limit it to 64K. The main
*** server sources never need buffers more than 64K, so that is fine.
***
*** If the system's memory allocation is unsatisfactory for one reason or
*** another, I have written my own. See st/stlocal.c and ibm/pclocal.c for
*** details.
**/

#if use_own_memory_management
PUBLIC void  fn(  initialise_memory, (void));
PUBLIC char *fn(  get_mem,           (uint));
PUBLIC void  fn(  free_mem,          (char *));
PUBLIC void  fn(  memory_map,        (void));       /* a debugging facility */
#define malloc(a)  get_mem(a)
#define free(a)    free_mem((char *) (a))

#else

#if !(PC || MAC)     /* The PC's header files declare malloc */
#if !(AMIGA)
PUBLIC char *fn( malloc,      (uint));
PUBLIC void fn( free,         (char *));
#else
PUBLIC char *fn( malloc,      (uint));
PUBLIC void fn( free,         (void *));
#endif
#endif

#endif /* use_own_memory_management */

/**
*** Here are some functions to deal with the multi-tasking support.
*** The only implementation I have so far is for the Sun
**/
#if multi_tasking
/**
*** These routines are called when the Server starts-up and leaves
*** server mode, and during restarts. 
**/
PUBLIC void fn( InitMultiwait,    (void));
PUBLIC void fn( RestartMultiwait, (void));
PUBLIC void fn( TidyMultiwait,    (void));
/**
*** This routine is called from inside the Server's main loop. It should
*** return when one of the I/O's the Server is currently waiting for is
*** possible, or after 1/2 a second to allow the Server to deal correctly
*** with timeouts.
**/
PUBLIC WORD fn( Multiwait, (void));
/**
*** This routine is called by the Server whenever it is waiting on some
*** form of I/O. The first argument is one of the constants defined in
*** structs.h, specifying the particular form of I/O. The second argument
*** is a pointer to an integer. Whenever the MultiWait is satisfied for the
*** event, that integer should be zapped to the value CoReady. Additional
*** arguments are available if required, e.g. to specify a particular
*** file descriptor.
**/
PUBLIC void fn( AddMultiwait, (WORD, WORD *,...));
/**
*** ClearMultiwait() is just the inverse of AddMultiwait()
**/
PUBLIC void fn( ClearMultiwait, (WORD, WORD *, ...));
#endif

/**
*** Bits that had been left out before.
**/
PUBLIC  void fn( initialise_devices,      (void));
PUBLIC  void fn( initialise_files,        (void));
PUBLIC  void fn( restart_devices,         (void));
PUBLIC  void fn( restore_devices,         (void));
PUBLIC  void fn( poll_the_devices,        (void));
PUBLIC  int  fn( read_char_from_keyboard, (WORD));
PUBLIC  int  fn( init_boot,               (void));
PUBLIC  void fn( boot_processor,          (int));
PUBLIC  void fn( tidy_boot,               (void));
PUBLIC  void fn( init_main_message,       (void));
PUBLIC  void fn( free_main_message,       (void));
PUBLIC  WORD fn( GetMsg,                  (MCB *));
PUBLIC  WORD fn( PutMsg,                  (MCB *));
#if debugger_incorporated
PUBLIC  void fn( init_debug,              (void));
PUBLIC  int  fn( debug,                   (void));
PUBLIC  void fn( tidy_debug,              (void));
#endif

#if floppies_available
PUBLIC  WORD fn( format_floppy, (char *, WORD, WORD, WORD, char *));
#endif

PUBLIC  int  fn( loadimage,               (void));
PUBLIC  void fn( resetlnk,                (void));
PUBLIC  void fn( xpreset,                 (void));
PUBLIC  void fn( xpanalyse,               (void));
PUBLIC  WORD fn( xpwrbyte,                (WORD));
PUBLIC  WORD fn( xpwrrdy,                 (void));
PUBLIC  WORD fn( xprdrdy,                 (void));
PUBLIC  WORD fn( xpwrword,                (WORD));
PUBLIC  WORD fn( xpwrint,                 (WORD));
PUBLIC  WORD fn( xpwrdata,                (BYTE *, WORD));
PUBLIC  WORD fn( xprddata,                (BYTE *, WORD));
PUBLIC  WORD fn( dbwrword,                (WORD, WORD));
PUBLIC  WORD fn( dbwrint,                 (WORD, WORD));
PUBLIC  WORD fn( xprdint,                 (void));
PUBLIC  WORD fn( xprdword,                (void));
PUBLIC  WORD fn( xprdbyte,                (void));
PUBLIC  WORD fn( dbrdword,                (WORD));
PUBLIC  WORD fn( dbrdint,                 (WORD));

#endif /* Daemon_Module */

/*------------------------------------------------------------------------
--                                                                      --
--          H E L I O S   I N P U T / O U T P U T   S E R V E R         --
--          ---------------------------------------------------         --
--                                                                      --
--             Copyright (C) 1987, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
--      server.h                                                        --
--                                                                      --
--      Author:  BLV 8/10/87                                            --
--                                                                      --
------------------------------------------------------------------------*/
/* RcsId: $Id: server.h,v 1.6 1991/09/13 14:45:51 martyn Exp $ */
/* Copyright (C) 1987, Perihelion Software Ltd.   			*/

#ifndef Daemon_Module

/**
***  The following lines declare all the shared variables in the system. The
***  space for these variables is allocated in module server.c, but all the
***  other modules need to have a declaration of them.
***
***  First come some pointers needed by the coroutine library. These are
***  explained in module server.c. Heliosnode is a pointer to a coroutine
***  list node used to maintain the linked lists in a sensible order.
***
***  mcb is the pointer to the server's message buffer, which is shared by all
***  devices.
***
***  CoCount is used to keep track of coroutine numbering. Each server and
***  stream coroutine has a unique identifier, obtained by incrementing
***  CoCount each time I create something. These identifiers are equivalent
***  to Helios ports, so all ports are unique.
***
***  Time_unit is used to convert host time in units of CLK_TCK per second
***  to Helios time in units of micro seconds. Startup_Time is a Unix time
***  stamp indicating when the system started up, and Now is the current
***  host time. Initial_stamp is a clock_t value corresponding to Startup_time.
***
***  IOname is used to store the result of the name conversion routine, which
***  is applied to all server requests.
***
***  Heliosdir is the name of the special Helios directory specified in
***  the host.con file. The system will try to use /Helios/lib, /Helios/bin
***  etc., and this drive Helios has to be mapped onto one of the local
***  directories identified by Heliosdir.
***
***  Maxdata imposes a limit on the size of the data vector in messsages
***  being passed between the Server and the transputer. It can be set in
***  the configuration file.
***
***  Err_buff is used to output() strings when the Server is running.
***
***  System_image holds a string specifying the system image to be booted
***  into the transputer.
***
***  Exit_jmpbuf is a C jump buffer set up by main() to allow the
***  initialisation routines to exit cleanly. 
***  
***  Special_Reboot is set when the user presses ctrl-shift-F10. Special_Exit
***  is set for ctrl-shift-F9, and Special_Status for ctrl-shift-F8.
***  DebugMode is toggled when the user presses ctrl-shift F7. These flags
***  are checked at regular intervals so that the user can switch systems
***  easily.
***
***  To keep coroutine stack sizes down I provide two miscellaneous buffers of
***  512 bytes each. These are used by e.g. the rename handlers to store file
***  names.
***
***  Next come the arrays for handling devices. All the routines are declared
***  in fundefs.h, and most of them are dummies of one sort or another.
***
***  Floppy_errno, RS232_errno, etc. are error flags, usually set by 
***  interrupt routines when a device error occurs e.g. attempting to
***  write to the floppy drive when there is no disk.
***
***  Device_count is used to keep track of the number of devices in the
***  server. This is used by the memory debugging routine.
***
***  Server_window is the main window, used for Server output in a multiple
***  windowing system and to store the console variables if no multiple
***  windows are available. Server_window_nopop controls the popping-up
***  of this window when pseudo-windows are used.
***
***  real_windows determines whether the windowing system in use involves
***  real multiple windows, e.g. Amiga, Sunview, X, or pseudo-windows,
***  e.g. PC, dumb terminal
***
***  Server_errno is an error variable that may be set by the local routines 
***  to give more accurate error messages. The main server sources always set
***  up a probable error code before any I/O operation, and the local
***  routines can change this error code after failure to reflect the real
***  error.
***
***  number_of_links specifies the number of link adapters which the server
***  has to monitor. The default is 1, to provide upward compatibility with
***  the older versions of the server where there could only ever be one
***  link adapter. current_link identifies the current link, to be used by
***  the link I/O routines. This avoids having to add a new argument to
***  all the link I/O routines to specify the link. link_table is a table of
***  link structures as described in structs.h.
***
***  Multi_nowait is used to indicate that there are coroutines running
***  which MUST be reactivated without a call to Multiwait(), which
***  could suspend the Server for up to half a second according to the
***  spec.
**/

#ifdef Server_Module
List        *WaitingCo, *PollingCo, *SelectCo;
Node        *Heliosnode;
MCB         *mcb;
word        CoCount=1L;             /* counter for coroutine id numbering */
word        time_unit = OneSec / CLK_TCK;
time_t      Startup_Time;
clock_t     Now, initial_stamp;
char        IOname[IOCDataMax];
char        *Heliosdir;
word        maxdata;
char        err_buff[256];
char        system_image[80];
jmp_buf     exit_jmpbuf;
WORD        Special_Reboot=FALSE, Special_Exit=FALSE, Special_Status=FALSE;
int         DebugMode = 0;
int 	    EnableThatLink = 0;       /* see module tload.c */
UBYTE       *bootstrap = NULL;        /* see module tload.c */
WORD        bootsize;
BYTE        misc_buffer1[512], misc_buffer2[512];
int         Device_count;
Window      Server_window;
int         Server_windows_nopop = 0;
#if multiple_windows
DirHeader   Window_List;
int         real_windows = 0;
#endif
WORD        Server_errno;
int         number_of_links = 1;
int         current_link    = 0;
PRIVATE     Trans_link root_link;
Trans_link  *link_table = &root_link;
int         Multi_nowait = 0;

device_declaration devices[] = 

{ { Type_Directory, DefaultServerName,
     { IOPROC_InitServer, IOPROC_TidyServer,  IOPROC_Private,
       IOPROC_Testfun,
       IOPROC_Open,       IOPROC_Create,      IOPROC_Locate,
       IOPROC_ObjectInfo, IOPROC_ServerInfo,  IOPROC_Delete,
       IOPROC_Rename,     IOPROC_Link,        IOPROC_Protect,
       IOPROC_SetDate,    IOPROC_Refine,      IOPROC_CloseObj
      }
  }
#if Romdisk_supported
 ,{ Type_Directory, "romdisk",
     { RomDisk_InitServer,  RomDisk_TidyServer,  RomDisk_Private,
       RomDisk_Testfun,
       RomDisk_Open,        RomDisk_Create,      RomDisk_Locate,
       RomDisk_ObjectInfo,  RomDisk_ServerInfo,  RomDisk_Delete,
       RomDisk_Rename,      RomDisk_Link,        RomDisk_Protect,
       RomDisk_SetDate,     RomDisk_Refine,      RomDisk_CloseObj
     }
  }
#endif

 ,{ Type_Directory, "helios",
      { Helios_InitServer,  Drive_TidyServer,   Drive_Private,
        Drive_Testfun,
        Drive_Open,         Drive_Create,       Drive_Locate,
        Drive_ObjectInfo,   Drive_ServerInfo,   Drive_Delete,    
        Drive_Rename,       Drive_Link,         Drive_Protect,
        Drive_SetDate,      Drive_Refine,       Drive_CloseObj
      }
  }

 ,{ Type_File, "logger",
      { Logger_InitServer,  Logger_TidyServer,   Logger_Private,
        Logger_Testfun,
        Logger_Open,        Logger_Create,      Logger_Locate,
        Logger_ObjectInfo,  Logger_ServerInfo,  Logger_Delete,    
        Logger_Rename,      Logger_Link,        Logger_Protect,
        Logger_SetDate,     Logger_Refine,      Logger_CloseObj
      }
  }

#if !(drives_are_special)
 ,{ Type_Directory, "files",
      { Drive_InitServer,   Drive_TidyServer,   Drive_Private,
        Drive_Testfun,
        Drive_Open,         Drive_Create,       Drive_Locate,  
        Drive_ObjectInfo,   Drive_ServerInfo,   Drive_Delete,  
        Drive_Rename,       Drive_Link,         Drive_Protect,
        Drive_SetDate,      Drive_Refine,       Drive_CloseObj
      }
  }
#endif

 ,{ Type_File, "console",
     { Console_InitServer,  Console_TidyServer,  Console_Private,
       Console_Testfun,
       Console_Open,        Console_Create,      Console_Locate,
       Console_ObjectInfo,  Console_ServerInfo,  Console_Delete,   
       Console_Rename,      Console_Link,        Console_Protect,
       Console_SetDate,     Console_Refine,      Console_CloseObj
     }
  }

#if multiple_windows
 ,{ Type_Directory, "window",
     { Window_InitServer,  Window_TidyServer,  Window_Private,
       (VoidFnPtr) Window_Testfun,
       Window_Open,        Window_Create,      Window_Locate,
       Window_ObjectInfo,  Window_ServerInfo,  Window_Delete,   
       Window_Rename,      Window_Link,        Window_Protect,
       Window_SetDate,     Window_Refine,      Window_CloseObj
     }
  }
#endif

#if Rawdisk_supported
 ,{ Type_Directory, "rawdisk",
     { RawDisk_InitServer,  RawDisk_TidyServer,  RawDisk_Private,
       RawDisk_Testfun,
       RawDisk_Open,        RawDisk_Create,      RawDisk_Locate,
       RawDisk_ObjectInfo,  RawDisk_ServerInfo,  RawDisk_Delete,
       RawDisk_Rename,      RawDisk_Link,        RawDisk_Protect,
       RawDisk_SetDate,     RawDisk_Refine,      RawDisk_CloseObj
     }
  }
#endif


 ,{ Type_Device, "clock", 
     { Clock_InitServer,  Clock_TidyServer,   Clock_Private,
       Clock_Testfun,
       Clock_Open,        Clock_Create,       Clock_Locate,
       Clock_ObjectInfo,  Clock_ServerInfo,   Clock_Delete,
       Clock_Rename,      Clock_Link,         Clock_Protect,
       Clock_SetDate,     Clock_Refine,       Clock_CloseObj
     }
  }

#if interaction_supported
 ,{ Type_File, machine_name, 
     { Host_InitServer,  Host_TidyServer,   Host_Private,
       Host_Testfun,
       Host_Open,        Host_Create,       Host_Locate,
       Host_ObjectInfo,  Host_ServerInfo,   Host_Delete,
       Host_Rename,      Host_Link,         Host_Protect,
       Host_SetDate,     Host_Refine,       Host_CloseObj
     }
  }
#endif

#if RS232_supported

 ,{ 
    Type_Directory, "rs232",
     { RS232_InitServer,  RS232_TidyServer,   RS232_Private,
       RS232_Testfun,
       RS232_Open,        RS232_Create,       RS232_Locate,
       RS232_ObjectInfo,  RS232_ServerInfo,   RS232_Delete,
       RS232_Rename,      RS232_Link,         RS232_Protect,
       RS232_SetDate,     RS232_Refine,       RS232_CloseObj
     }
  }

#endif

#if Centronics_supported

 ,{
    Type_Directory, "centronics",
     { Centronics_InitServer,  Centronics_TidyServer, Centronics_Private,
       Centronics_Testfun,
       Centronics_Open,        Centronics_Create,     Centronics_Locate,
       Centronics_ObjectInfo,  Centronics_ServerInfo, Centronics_Delete, 
       Centronics_Rename,      Centronics_Link,       Centronics_Protect,
       Centronics_SetDate,     Centronics_Refine,     Centronics_CloseObj
     }
  }

#endif

#if Printer_supported

 ,{ Type_Directory, "printers",
     { Printer_InitServer,     Printer_TidyServer,    Printer_Private,
       Printer_Testfun,
       Printer_Open,           Printer_Create,        Printer_Locate,
       Printer_ObjectInfo,     Printer_ServerInfo,    Printer_Delete,
       Printer_Rename,         Printer_Link,          Printer_Protect,
       Printer_SetDate,        Printer_Refine,        Printer_CloseObj
     }
  }

#endif

#if Midi_supported

 ,{ Type_Directory, "midi",
     { Midi_InitServer,     Midi_TidyServer,    Midi_Private,
       Midi_Testfun,
       Midi_Open,           Midi_Create,        Midi_Locate,
       Midi_ObjectInfo,     Midi_ServerInfo,    Midi_Delete,
       Midi_Rename,         Midi_Link,          Midi_Protect,
       Midi_SetDate,        Midi_Refine,        Midi_CloseObj
     }
  }

#endif

#if Ether_supported
  ,{ Type_File, "ether",
      { Ether_InitServer, Ether_TidyServer, Ether_Private,
        Ether_Testfun,
        Ether_Open, Ether_Create, Ether_Locate,
        Ether_ObjectInfo, Ether_ServerInfo, Ether_Delete,
        Ether_Rename, Ether_Link, Ether_Protect,
        Ether_SetDate, Ether_Refine, Ether_CloseObj
      }
  }

#endif

#if internet_supported

 ,{ Type_Directory, "internet",
     { Internet_InitServer,    Internet_TidyServer,     Internet_Private,
       Internet_Testfun,
       Internet_Open,          Internet_Create,         Internet_Locate,
       Internet_ObjectInfo,    Internet_ServerInfo,     Internet_Delete,
       Internet_Rename,        Internet_Link,           Internet_Protect,
       Internet_SetDate,       Internet_Refine,         Internet_CloseObj
     }
  }
#endif

#if mouse_supported

 ,{ Type_File, "mouse",
     { Mouse_InitServer,  Mouse_TidyServer,   Mouse_Private,
       Mouse_Testfun,
       Mouse_Open,        Mouse_Create,       Mouse_Locate,
       Mouse_ObjectInfo,  Mouse_ServerInfo,   Mouse_Delete,
       Mouse_Rename,      Mouse_Link,         Mouse_Protect,
       Mouse_SetDate,     Mouse_Refine,       Mouse_CloseObj
     }
  }

#endif

#if keyboard_supported

 ,{ Type_File, "keyboard",
     { Keyboard_InitServer,  Keyboard_TidyServer, Keyboard_Private,
       Keyboard_Testfun,
       Keyboard_Open,        Keyboard_Create,     Keyboard_Locate,
       Keyboard_ObjectInfo,  Keyboard_ServerInfo, Keyboard_Delete,
       Keyboard_Rename,      Keyboard_Link,       Keyboard_Protect,
       Keyboard_SetDate,     Keyboard_Refine,     Keyboard_CloseObj
     }
  }

#endif

#if gem_supported

 ,{ Type_File, "gem",  
     { Gem_InitServer,  Gem_TidyServer,   Gem_Private,
       Gem_Testfun,
       Gem_Open,        Gem_Create,       Gem_Locate,
       Gem_ObjectInfo,  Gem_ServerInfo,   Gem_Delete,
       Gem_Rename,      Gem_Link,         Gem_Protect,
       Gem_SetDate,     Gem_Refine,       Gem_CloseObj
     }
  }
#endif

#if X_supported

 ,{ Type_Directory, "x",
     { X_InitServer,    X_TidyServer,     X_Private,
       X_Testfun,
       X_Open,          X_Create,         X_Locate,
       X_ObjectInfo,    X_ServerInfo,     X_Delete,
       X_Rename,        X_Link,           X_Protect,
       X_SetDate,       X_Refine,         X_CloseObj
     }
  }
#endif

#if Network_supported

 ,{ Type_File, "NetworkController",
     { Network_InitServer,     Network_TidyServer,    Network_Private,
       Network_Testfun,
       Network_Open,           Network_Create,        Network_Locate,
       Network_ObjectInfo,     Network_ServerInfo,    Network_Delete,
       Network_Rename,         Network_Link,          Network_Protect,
       Network_SetDate,        Network_Refine,        Network_CloseObj
     }
  }
#endif
   
 ,{ 0L, NULL,  
     {  (VoidFnPtr) NULL, (VoidFnPtr) NULL, (VoidFnPtr) NULL, (VoidFnPtr) NULL,
	(VoidFnPtr) NULL, (VoidFnPtr) NULL, (VoidFnPtr) NULL, (VoidFnPtr) NULL,
	(VoidFnPtr) NULL, (VoidFnPtr) NULL, (VoidFnPtr) NULL, (VoidFnPtr) NULL,
	(VoidFnPtr) NULL, (VoidFnPtr) NULL, (VoidFnPtr) NULL, (VoidFnPtr) NULL
      }
  }

};

#if drives_are_special
VoidFnPtr Drive_Handlers[handler_max] =
      { Drive_InitServer,   Drive_TidyServer,   Drive_Private,
        Drive_Testfun,
        Drive_Open,         Drive_Create,       Drive_Locate,
        Drive_ObjectInfo,   Drive_ServerInfo,   Drive_Delete,
        Drive_Rename,       Drive_Link,         Drive_Protect,
        Drive_SetDate,      Drive_Refine,       Drive_CloseObj };
#endif

VoidFnPtr IOPROC_Handlers[Stream_max] =
      { (VoidFnPtr) IOPROC_InitStream, (VoidFnPtr) IOPROC_TidyStream,
        IOPROC_PrivateStream,
        IOPROC_Read,        IOPROC_Write,          IOPROC_GetSize,
        IOPROC_SetSize,     IOPROC_Close,          IOPROC_Seek,
        IOPROC_GetAttr,     IOPROC_SetAttr,        IOPROC_EnableEvents,
        IOPROC_Acknowledge, IOPROC_NegAcknowledge, IOPROC_Select };

VoidFnPtr Logger_Handlers[Stream_max] =
      { (VoidFnPtr) Logger_InitStream,  (VoidFnPtr) Logger_TidyStream,
        Logger_PrivateStream,
        Logger_Read,        Logger_Write,          Logger_GetSize,
        Logger_SetSize,     Logger_Close,          Logger_Seek,
        Logger_GetAttr,     Logger_SetAttr,        Logger_EnableEvents,
        Logger_Acknowledge, Logger_NegAcknowledge, Logger_Select };


VoidFnPtr Console_Handlers[Stream_max] =
      { (VoidFnPtr) Console_InitStream,  (VoidFnPtr) Console_TidyStream,
        Console_PrivateStream,
        Console_Read,        Console_Write,          Console_GetSize,
        Console_SetSize,     Console_Close,          Console_Seek,
        Console_GetAttr,     Console_SetAttr,        Console_EnableEvents,
        Console_Acknowledge, Console_NegAcknowledge, Console_Select };


#if multiple_windows
VoidFnPtr WindowDir_Handlers[Stream_max] =
      { (VoidFnPtr) WindowDir_InitStream, (VoidFnPtr) WindowDir_TidyStream,
        WindowDir_PrivateStream,
        WindowDir_Read,         WindowDir_Write,        WindowDir_GetSize,
        WindowDir_SetSize,      WindowDir_Close,        WindowDir_Seek,
        WindowDir_GetAttr,      WindowDir_SetAttr,      WindowDir_EnableEvents,
        WindowDir_Acknowledge,  WindowDir_NegAcknowledge, WindowDir_Select };
#endif

#if Rawdisk_supported

VoidFnPtr Rawdisk_Handlers[Stream_max] =
      { (VoidFnPtr) RawDisk_InitStream,  (VoidFnPtr) RawDisk_TidyStream,
        RawDisk_PrivateStream,
        RawDisk_Read,          RawDisk_Write,        RawDisk_GetSize,
        RawDisk_SetSize,       RawDisk_Close,        RawDisk_Seek,
        RawDisk_GetAttr,       RawDisk_SetAttr,      RawDisk_EnableEvents,
        RawDisk_Acknowledge,   RawDisk_NegAcknowledge, RawDisk_Select };
#endif
#if Romdisk_supported

VoidFnPtr Romdisk_Handlers[Stream_max] =
      { (VoidFnPtr) RomDisk_InitStream,  (VoidFnPtr) RomDisk_TidyStream,
        RomDisk_PrivateStream,
        RomDisk_Read,          RomDisk_Write,        RomDisk_GetSize,
        RomDisk_SetSize,       RomDisk_Close,        RomDisk_Seek,
        RomDisk_GetAttr,       RomDisk_SetAttr,      RomDisk_EnableEvents,
        RomDisk_Acknowledge,   RomDisk_NegAcknowledge, RomDisk_Select };
#endif

#if gem_supported
VoidFnPtr Gem_Handlers[Stream_max] =
      { (VoidFnPtr) Gem_InitStream,  (VoidFnPtr) Gem_TidyStream,
        Gem_PrivateStream,
        Gem_Read,        Gem_Write,          Gem_GetSize,
        Gem_SetSize,     Gem_Close,          Gem_Seek,
        Gem_GetAttr,     Gem_SetAttr,        Gem_EnableEvents,
        Gem_Acknowledge, Gem_NegAcknowledge, Gem_Select };
#endif

#if interaction_supported
VoidFnPtr Host_Handlers[Stream_max] =
      { (VoidFnPtr) Host_InitStream,  (VoidFnPtr) Host_TidyStream,
        Host_PrivateStream,
        Host_Read,        Host_Write,          Host_GetSize,
        Host_SetSize,     Host_Close,          Host_Seek,
        Host_GetAttr,     Host_SetAttr,        Host_EnableEvents,
        Host_Acknowledge, Host_NegAcknowledge, Host_Select };
#endif


#if Ports_used
VoidFnPtr PortDir_Handlers[Stream_max] =
      { (VoidFnPtr) PortDir_InitStream, (VoidFnPtr) PortDir_TidyStream,
        PortDir_PrivateStream,
        PortDir_Read,         PortDir_Write,          PortDir_GetSize,
        PortDir_SetSize,      PortDir_Close,          PortDir_Seek,
        PortDir_GetAttr,      PortDir_SetAttr,        PortDir_EnableEvents,
        PortDir_Acknowledge,  PortDir_NegAcknowledge, PortDir_Select };
#endif

#if RS232_supported
VoidFnPtr RS232_Handlers[Stream_max] =
      { (VoidFnPtr) RS232_InitStream, (VoidFnPtr) RS232_TidyStream,
        RS232_PrivateStream,
        RS232_Read,         RS232_Write,          RS232_GetSize,
        RS232_SetSize,      RS232_Close,          RS232_Seek,
        RS232_GetAttr,      RS232_SetAttr,        RS232_EnableEvents,
        RS232_Acknowledge,  RS232_NegAcknowledge, RS232_Select };
#endif

#if Centronics_supported
VoidFnPtr Centronics_Handlers[Stream_max] =
      { (VoidFnPtr) Centronics_InitStream, (VoidFnPtr) Centronics_TidyStream,
        Centronics_PrivateStream,
        Centronics_Read,        Centronics_Write,       Centronics_GetSize,
        Centronics_SetSize,     Centronics_Close,       Centronics_Seek,
        Centronics_GetAttr,     Centronics_SetAttr,     Centronics_EnableEvents,
        Centronics_Acknowledge, Centronics_NegAcknowledge, Centronics_Select };
#endif

#if Printer_supported
VoidFnPtr Printer_Handlers[Stream_max] =
      { (VoidFnPtr) Printer_InitStream, (VoidFnPtr) Printer_TidyStream,
        Printer_PrivateStream,
        Printer_Read,           Printer_Write,          Printer_GetSize,
        Printer_SetSize,        Printer_Close,          Printer_Seek,
        Printer_GetAttr,        Printer_SetAttr,        Printer_EnableEvents,
        Printer_Acknowledge,    Printer_NegAcknowledge, Printer_Select };
#endif

#if Midi_supported
VoidFnPtr Midi_Handlers[Stream_max] =
      { (VoidFnPtr) Midi_InitStream, (VoidFnPtr) Midi_TidyStream,
        Midi_PrivateStream,
        Midi_Read,           Midi_Write,          Midi_GetSize,
        Midi_SetSize,        Midi_Close,          Midi_Seek,
        Midi_GetAttr,        Midi_SetAttr,        Midi_EnableEvents,
        Midi_Acknowledge,    Midi_NegAcknowledge, Midi_Select };
#endif

#if Ether_supported
VoidFnPtr Ether_Handlers[Stream_max] =
      { (VoidFnPtr) Ether_InitStream, (VoidFnPtr) Ether_TidyStream,
        Ether_PrivateStream,
        Ether_Read,          Ether_Write,          Ether_GetSize,
        Ether_SetSize,       Ether_Close,          Ether_Seek,
        Ether_GetAttr,       Ether_SetAttr,        Ether_EnableEvents,
        Ether_Acknowledge,   Ether_NegAcknowledge, Ether_Select };
#endif

#if mouse_supported
VoidFnPtr Mouse_Handlers[Stream_max] =
      { (VoidFnPtr) Mouse_InitStream, (VoidFnPtr) Mouse_TidyStream,
        Mouse_PrivateStream,
        Mouse_Read,         Mouse_Write,          Mouse_GetSize,
        Mouse_SetSize,      Mouse_Close,          Mouse_Seek,
        Mouse_GetAttr,      Mouse_SetAttr,        Mouse_EnableEvents,
        Mouse_Acknowledge,  Mouse_NegAcknowledge, Mouse_Select };
#endif

#if keyboard_supported
VoidFnPtr Keyboard_Handlers[Stream_max] =
      { (VoidFnPtr) Keyboard_InitStream, (VoidFnPtr) Keyboard_TidyStream,
        Keyboard_PrivateStream,
        Keyboard_Read,        Keyboard_Write,        Keyboard_GetSize,
        Keyboard_SetSize,     Keyboard_Close,        Keyboard_Seek,
        Keyboard_GetAttr,     Keyboard_SetAttr,      Keyboard_EnableEvents,
        Keyboard_Acknowledge, Keyboard_NegAcknowledge, Keyboard_Select };
#endif

#if X_supported
VoidFnPtr XDir_Handlers[Stream_max] =
      { (VoidFnPtr) XDir_InitStream, (VoidFnPtr) XDir_TidyStream,
        XDir_PrivateStream,
        XDir_Read,            XDir_Write,            XDir_GetSize,
        XDir_SetSize,         XDir_Close,            XDir_Seek,
        XDir_GetAttr,         XDir_SetAttr,          XDir_EnableEvents,
        XDir_Acknowledge,     XDir_NegAcknowledge,   XDir_Select };

VoidFnPtr X_Handlers[Stream_max] =
      { (VoidFnPtr) X_InitStream, (VoidFnPtr) X_TidyStream,
        X_PrivateStream,
        X_Read,            X_Write,            X_GetSize,
        X_SetSize,         X_Close,            X_Seek,
        X_GetAttr,         X_SetAttr,          X_EnableEvents,
        X_Acknowledge,     X_NegAcknowledge,   X_Select };

#endif /* X_supported */

#if Network_supported
VoidFnPtr Network_Handlers[Stream_max] =
      { (VoidFnPtr) Network_InitStream, (VoidFnPtr) Network_TidyStream,
         Network_PrivateStream,
         Network_Read,        Network_Write,          Network_GetSize,
         Network_SetSize,     Network_Close,          Network_Seek,
         Network_GetAttr,     Network_SetAttr,        Network_EnableEvents,
         Network_Acknowledge, Network_NegAcknowledge, Network_Select };
#endif
                
VoidFnPtr File_Handlers[Stream_max] =
      { (VoidFnPtr) File_InitStream,  (VoidFnPtr) File_TidyStream,
        File_PrivateStream,
        File_Read,        File_Write,          File_GetSize,
        File_SetSize,     File_Close,          File_Seek,
        File_GetAttr,     File_SetAttr,        File_EnableEvents,
        File_Acknowledge, File_NegAcknowledge, File_Select };

VoidFnPtr Dir_Handlers[Stream_max] =
      { (VoidFnPtr) Dir_InitStream, (VoidFnPtr) Dir_TidyStream,
        Dir_PrivateStream,
        Dir_Read,         Dir_Write,          Dir_GetSize,
        Dir_SetSize,      Dir_Close,          Dir_Seek,
        Dir_GetAttr,      Dir_SetAttr,        Dir_EnableEvents,
        Dir_Acknowledge,  Dir_NegAcknowledge, Dir_Select };

#if internet_supported
VoidFnPtr InternetDir_Handlers[Stream_max] =
      { (VoidFnPtr) InternetDir_InitStream, (VoidFnPtr) InternetDir_TidyStream,
        InternetDir_PrivateStream,
        InternetDir_Read,        InternetDir_Write,         InternetDir_GetSize,
        InternetDir_SetSize,     InternetDir_Close,         InternetDir_Seek,
        InternetDir_GetAttr,     InternetDir_SetAttr,  InternetDir_EnableEvents,
        InternetDir_Acknowledge,InternetDir_NegAcknowledge,InternetDir_Select };

VoidFnPtr Internet_Handlers[Stream_max] =
      { (VoidFnPtr) Internet_InitStream, (VoidFnPtr) Internet_TidyStream,
        (VoidFnPtr) Internet_PrivateStream,
        Internet_Read,            Internet_Write,         Internet_GetSize,
        Internet_SetSize,         Internet_Close,         Internet_Seek,
        Internet_GetInfo,         Internet_SetInfo,       Internet_EnableEvents,
        Internet_Acknowledge,     Internet_NegAcknowledge,Internet_Select };

#endif /* Internet_supported */

int Server_Mode = Mode_Normal;

#else       /* declarations for the other modules */

extern int Server_Mode;
extern List         *WaitingCo, *PollingCo, *SelectCo;
extern MCB          *mcb;
extern Node         *Heliosnode;
extern word         CoCount;
extern word         time_unit;
extern time_t       Startup_Time;
extern clock_t      Now, initial_stamp;
extern char         IOname[];
extern char         *Heliosdir;
extern char         err_buff[];
extern word         maxdata;
extern char         system_image[];
extern jmp_buf      exit_jmpbuf;
extern word         Special_Reboot, Special_Exit, Special_Status;
extern int          DebugMode;
extern int          EnableThatLink;
extern char         *bootstrap;
extern word         bootsize;
extern BYTE         misc_buffer1[], misc_buffer2[];
extern int          Device_count;
extern Window       Server_window;
extern int          Server_windows_nopop;
#if multiple_windows
extern DirHeader    Window_List;
extern int          real_windows;
#endif
extern word         Server_errno;
extern int          number_of_links, current_link;
extern Trans_link   *link_table;
extern int          Multi_nowait;

extern VoidFnPtr Dir_Handlers[];
extern VoidFnPtr File_Handlers[];
extern VoidFnPtr IOPROC_Handlers[];
extern VoidFnPtr Console_Handlers[];
extern VoidFnPtr Logger_Handlers[];
#if multiple_windows
extern VoidFnPtr WindowDir_Handlers[];
#endif

#if Rawdisk_supported
extern VoidFnPtr Rawdisk_Handlers[];
#endif
#if Romdisk_supported
extern VoidFnPtr Romdisk_Handlers[];
#endif

#if gem_supported
extern VoidFnPtr Gem_Handlers[];
#endif
#if interaction_supported
extern VoidFnPtr Host_Handlers[];
#endif
#if Ports_used
extern VoidFnPtr PortDir_Handlers[];
#endif
#if RS232_supported
extern VoidFnPtr RS232_Handlers[];
#endif
#if Centronics_supported
extern VoidFnPtr Centronics_Handlers[];
#endif
#if Printer_supported
extern VoidFnPtr Printer_Handlers[];
#endif
#if Midi_supported
extern VoidFnPtr Midi_Handlers[];
#endif
#if Ether_supported
extern VoidFnPtr Ether_Handlers[];
#endif
#if mouse_supported
extern VoidFnPtr Mouse_Handlers[];
#endif
#if keyboard_supported
extern VoidFnPtr Keyboard_Handlers[];
#endif
#if X_supported
extern VoidFnPtr XDir_Handlers[];
extern VoidFnPtr X_Handlers[];
#endif
#if Network_supported
extern VoidFnPtr Network_Handlers[];
#endif
#if internet_supported
extern VoidFnPtr InternetDir_Handlers[];
extern VoidFnPtr Internet_Handlers[];
#endif

#endif  /* not Server_Module */

/**
*** Device errors, e.g. writing to a floppy drive that does not contain a
*** disk, have the annoying habit of invoking interrupts rather than
*** producing nice error codes. This gives some problems when I try to
*** handle them in a general way. Roughly, I expect the local routines to
*** set the following error variables to suitable values when a device error
*** occurs, by fair means or foul, and I take care of processing the error.
*** Because they have to be provided by the local routines, I prefer to keep
*** them in the local modules - mainly because of problems with segments on
*** the PC.
**/
#ifndef Local_Module

#if RS232_supported
extern int   RS232_errno;
#endif

#if Centronics_supported
extern int Centronics_errno;
#endif

#if floppies_available
extern int floppy_errno;
#endif

#if Printer_supported
extern int Printer_errno;
#endif

#if Midi_supported
extern int Midi_errno;
#endif

#endif  /* not Local_Module */

#endif /* not daemon module */
/*------------------------------------------------------------------------
--                                                                      --
--          H E L I O S   I N P U T / O U T P U T   S E R V E R         --
--          ---------------------------------------------------         --
--                                                                      --
--             Copyright (C) 1987, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
--      Debugopt.h                                                      --
--                                                                      --
--  Author:  BLV 10/12/87                                               --
--                                                                      --
------------------------------------------------------------------------*/
/* RcsId: $Id: debugopt.h,v 1.1 1990/10/16 16:32:59 alan Exp $ */
/* Copyright (C) 1987, Perihelion Software Ltd.   			*/

#ifndef Daemon_Module

/**
*** This header file is used to control the debugging options available in
*** the Server. There is a set of flags, debugflags, and a debugging option
*** is on when the corresponding bit is set in debugflags. The various flags
*** are declared. To keep track of all the debugging options I use a structure
*** debug_options, each option consisting of a character and the corresponding
*** flag, and I have an array of these options.
***
*** For example, when ctrl-shift M is pressed to toggle the message debugging
*** flag the system looks down options_list to find an entry with the letter
*** 'm'. It finds this, and xors debugflags with the corresponding flag
*** Message_Flag.
**/

typedef struct debug_options {
         int    flagchar;
         word   flag;
         char   *name;
} debug_options;

#define Message_Flag                 0x1L
#define Search_Flag                  0x2L
#define Open_Flag                    0x4L
#define Name_Flag                    0x8L
#define Read_Flag                   0x10L
#define Boot_Flag                   0x20L
#define Memory_Flag                 0x40L
#define Keyboard_Flag               0x80L
#define Init_Flag                  0x100L
#define Com_Flag                   0x200L
#define Write_Flag                 0x400L
#define Quit_Flag                  0x800L
#define Close_Flag                0x1000L
#define HardDisk_Flag             0x2000L
#define Log_Flag                  0x4000L
#define Graphics_Flag             0x8000L
#define Reconfigure_Flag         0x10000L
#define Timeout_Flag             0x20000L
#define OpenReply_Flag           0x40000L
#define FileIO_Flag              0x80000L
#define Delete_Flag             0x100000L
#define Directory_Flag          0x200000L
#define Nopop_Flag              0x400000L
#define ListAll_Flag            0x800000L
#define Error_Flag             0x1000000L
/**
*** All_Debug_Flags is a mask for all the debugging options except the
*** one-off ones : memory, log, reconfigure, nopop, listall.
*** It is used for -a etc.
**/
#define All_Debug_Flags       0x013EBFBFL
              
#define Log_to_screen           1
#define Log_to_file             2
#define Log_to_both             3

#ifdef Server_Module
WORD debugflags;
int  log_dest = Log_to_screen;
debug_options options_list[] = 
                               {
             /* 'a' == all */
             { 'b', Boot_Flag,        "boot"        },
             { 'c', Com_Flag,         "serial"      },
             { 'd', Delete_Flag,      "delete"      },
             { 'e', Error_Flag,       "errors"      },
             { 'f', FileIO_Flag,      "file I/O"    },
             { 'g', Graphics_Flag,    "graphics"    },
             { 'h', HardDisk_Flag,    "raw disk"    },
             { 'i', Init_Flag,        "init"        },
             { 'j', Directory_Flag,   "directory"   },
             { 'k', Keyboard_Flag,    "keyboard"    },
             { 'l', Log_Flag,         "logger"      },
             { 'm', Message_Flag,     "messages"    }, 
             { 'n', Name_Flag,        "names"       },
             { 'o', Open_Flag,        "open"        },
             { 'p', Close_Flag,       "close"       },
             { 'q', Quit_Flag,        "exit"        },
             { 'r', Read_Flag,        "read"        },
             { 's', Search_Flag,      "search"      },
             { 't', Timeout_Flag,     "timeouts"    },
             { 'u', Nopop_Flag,       "nopop"       },
             { 'v', OpenReply_Flag,   "open reply"  },
             { 'w', Write_Flag,       "write"       },
             { 'x', Memory_Flag,      "resources"   },
             { 'y', ListAll_Flag,     "list"        },
             { 'z', Reconfigure_Flag, "reconfigure" },
             { '\0', 0L              } };

#endif
#ifndef Server_Module
extern WORD debugflags;
extern int  log_dest;
extern debug_options options_list[];
#endif

/**
*** Where the bootstrap program is loaded, assumes T800 but not really
*** important
**/
#ifndef MemStart
#define MemStart  MinInt+0x70
#endif

/**
*** arguments to boot_transputer() in module tload.c
**/
#define debugboot       1
#define serverboot      2

/**
*** If the I/O Server is compiled with -DSMALL, most of the debugging
*** options disappear
**/
#ifdef SMALL
#define Debug(a, b)
#else
#define Debug(a,b) if (debugflags & a) ServerDebug b
#endif /* SMALL */

#endif  /* Daemon module */
