/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
* User-visible pieces of the ANSI C standard I/O package.
*/
#ifndef _STDIO_H /* if not defined then stdio.h has not yet been included */
#define _STDIO_H

#ident	"@(#)head:stdio.h	2.34.1.2"

#ifndef _SIZE_T
#define _SIZE_T
typedef unsigned int 	size_t;
#endif 

typedef long	fpos_t;

#ifndef NULL
#define NULL            0
#endif 

#if defined(__STDC__)

#if #machine(pdp11)
#   define BUFSIZ		512
#   define _STDIO_REVERSE
#elif #machine(u370)
#   define BUFSIZ		4096
#   define _STDIO_REVERSE
#   define _STDIO_ALLOCATE
#else
#   define BUFSIZ		1024
#endif

#if #machine(i386)
#define _NFILE	60	/* initial number of streams */
#else
#define _NFILE	20	/* initial number of streams */
#endif

#else	/* !defined(__STDC__) */

#if pdp11 || u370

#if pdp11
#   define BUFSIZ		512
#   define _STDIO_REVERSE
#else 	/* u370 */
#   define BUFSIZ		4096
#   define _STDIO_REVERSE
#   define _STDIO_ALLOCATE
#endif

#else
#   define BUFSIZ		1024
#endif

#ifdef i386
#define _NFILE	60	/* initial number of streams */
#else
#define _NFILE	20	/* initial number of streams */
#endif

#endif	/* __STDC__ */

#define _SBFSIZ	8	/* compatibility with shared libs */

#define _IOFBF		0000	/* full buffered */
#define _IOLBF		0100	/* line buffered */
#define _IONBF		0004	/* not buffered */
#define _IOEOF		0020	/* EOF reached on read */
#define _IOERR		0040	/* I/O error from system */

#define _IOREAD		0001	/* currently reading */
#define _IOWRT		0002	/* currently writing */
#define _IORW		0200	/* opened for reading and writing */
#define _IOMYBUF	0010	/* stdio malloc()'d buffer */

#ifndef EOF
#   define EOF	(-1)
#endif

#define FOPEN_MAX	_NFILE
#define FILENAME_MAX    1024	/* max # of characters in a path name */

#define SEEK_SET	0
#define SEEK_CUR	1
#define SEEK_END	2
#define TMP_MAX		17576	/* 26 * 26 * 26 */

#if __STDC__ - 0 == 0 || defined(_POSIX_SOURCE) || defined(_XOPEN_SOURCE)
#define L_ctermid	9
#define L_cuserid	9
#endif

#if (__STDC__ - 0 == 0 && !defined(_POSIX_SOURCE)) || defined(_XOPEN_SOURCE)
#define P_tmpdir	"/var/tmp/"
#endif

#define L_tmpnam	25	/* (sizeof(P_tmpdir) + 15) */

#if defined(__STDC__)
#define stdin	(&__iob[0])
#define stdout	(&__iob[1])
#define stderr	(&__iob[2])
#else
#define stdin	(&_iob[0])
#define stdout	(&_iob[1])
#define stderr	(&_iob[2])
#endif

typedef struct	/* needs to be binary-compatible with old versions */
{
#ifdef _STDIO_REVERSE
	unsigned char	*_ptr;	/* next character from/to here in buffer */
	int		_cnt;	/* number of available characters in buffer */
#else
	int		_cnt;	/* number of available characters in buffer */
	unsigned char	*_ptr;	/* next character from/to here in buffer */
#endif
	unsigned char	*_base;	/* the buffer */
	unsigned char	_flag;	/* the state of the stream */
	unsigned char	_file;	/* UNIX System file descriptor */
} FILE;

#if defined(__STDC__)
extern FILE		__iob[_NFILE];
#else
extern FILE		_iob[_NFILE];
#endif
extern FILE		*_lastbuf;
extern unsigned char 	*_bufendtab[];
#ifndef _STDIO_ALLOCATE
extern unsigned char	 _sibuf[], _sobuf[];
#endif

#if defined(__STDC__)

extern int	remove(const char *);
extern int	rename(const char *, const char *);
extern FILE	*tmpfile(void);
extern char	*tmpnam(char *);
extern int	fclose(FILE *);
extern int	fflush(FILE *);
extern FILE	*fopen(const char *, const char *);
extern FILE	*freopen(const char *, const char *, FILE *);
extern void	setbuf(FILE *, char *);
extern int	setvbuf(FILE *, char *, int, size_t);
/* PRINTFLIKE2 */
extern int	fprintf(FILE *, const char *, ...);
/* SCANFLIKE2 */
extern int	fscanf(FILE *, const char *, ...);
/* PRINTFLIKE1 */
extern int	printf(const char *, ...);
/* SCANFLIKE1 */
extern int	scanf(const char *, ...);
/* PRINTFLIKE2 */
extern int	sprintf(char *, const char *, ...);
/* SCANFLIKE2 */
extern int	sscanf(const char *, const char *, ...);
extern int	vfprintf(FILE *, const char *, void *);
extern int	vprintf(const char *, void *);
extern int	vsprintf(char *, const char *, void *);
extern int	fgetc(FILE *);
extern char	*fgets(char *, int, FILE *); 
extern int	fputc(int, FILE *);
extern int	fputs(const char *, FILE *);
extern int	getc(FILE *);
extern int	getchar(void);
extern char	*gets(char *);
extern int	putc(int, FILE *);
extern int	putchar(int);
extern int	puts(const char *);
extern int	ungetc(int, FILE *);
extern size_t	fread(void *, size_t, size_t, FILE *);
	#pragma int_to_unsigned fread
extern size_t	fwrite(const void *, size_t, size_t, FILE *);
	#pragma int_to_unsigned fwrite
extern int	fgetpos(FILE *, fpos_t *);
extern int	fseek(FILE *, long, int);
extern int	fsetpos(FILE *, const fpos_t *);
extern long	ftell(FILE *);
extern void	rewind(FILE *);
extern void	clearerr(FILE *);
extern int	feof(FILE *);
extern int	ferror(FILE *);
extern void	perror(const char *);

extern int	__filbuf(FILE *);
extern int	__flsbuf(int, FILE *);

#if !#lint(on)
#define getc(p)		(--(p)->_cnt < 0 ? __filbuf(p) : (int)*(p)->_ptr++)
#define putc(x, p)	(--(p)->_cnt < 0 ? __flsbuf((x), (p)) \
				: (int)(*(p)->_ptr++ = (x)))
#define getchar()	getc(stdin)
#define putchar(x)	putc((x), stdout)
#define clearerr(p)	((void)((p)->_flag &= ~(_IOERR | _IOEOF)))
#define feof(p)		((p)->_flag & _IOEOF)
#define ferror(p)	((p)->_flag & _IOERR)
#endif

#if __STDC__ == 0 || defined(_POSIX_SOURCE) || defined(_XOPEN_SOURCE) /* non-ANSI standard compilation */

extern FILE    *fdopen(int, const char *);
extern FILE    *popen(const char *, const char *);
extern char    *ctermid(char *);
extern char    *cuserid(char *);
extern char    *tempnam(const char *, const char *);
extern int     getw(FILE *);
extern int     putw(int, FILE *);
extern int     pclose(FILE *);
extern int     system(const char *);
extern int	fileno(FILE *);

#if !#lint(on)
#define fileno(p)	(p)->_file
#endif

#endif	/* __STDC__ == 0 */

#else	/* !defined __STDC__ */
#define _bufend(p)      _bufendtab[(p)->_file]
#define _bufsiz(p)      (_bufend(p) - (p)->_base)

#ifndef lint
#define getc(p)         (--(p)->_cnt < 0 ? _filbuf(p) : (int) *(p)->_ptr++)
#define putc(x, p)      (--(p)->_cnt < 0 ? \
                        _flsbuf((unsigned char) (x), (p)) : \
                        (int) (*(p)->_ptr++ = (unsigned char) (x)))
#define getchar()       getc(stdin)
#define putchar(x)      putc((x), stdout)
#define clearerr(p)     ((void) ((p)->_flag &= ~(_IOERR | _IOEOF)))
#define feof(p)         ((p)->_flag & _IOEOF)
#define ferror(p)       ((p)->_flag & _IOERR)
#define fileno(p)       (p)->_file
#endif	/* lint */

extern FILE     *fopen(), *fdopen(), *freopen(), *popen(), *tmpfile();
extern long     ftell();
extern void     rewind(), setbuf();
extern char     *ctermid(), *cuserid(), *fgets(), *gets(), *tempnam(), *tmpnam();
extern int      fclose(), fflush(), fread(), fwrite(), fseek(), fgetc(),
                getw(), pclose(), printf(), fprintf(), sprintf(),
                vprintf(), vfprintf(), vsprintf(), fputc(), putw(),
                puts(), fputs(), scanf(), fscanf(), sscanf(),
                setvbuf(), system(), ungetc();

#endif	/* __STDC__ */

#endif  /* _STDIO_H */
/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _SYS_TYPES_H
#define _SYS_TYPES_H

#ident	"@(#)head.sys:sys/types.h	11.33.3.1"


/* POSIX Extensions */

typedef	unsigned char	uchar_t;
typedef	unsigned short	ushort_t;
typedef	unsigned int	uint_t;
typedef	unsigned long	ulong_t;


/* For BSD compatibility */
typedef char *		addr_t; /* ?<core address> type */

typedef	char *		caddr_t;	/* ?<core address> type */
typedef	long		daddr_t;	/* <disk address> type */
typedef	long		off_t;		/* ?<offset> type */
typedef	short		cnt_t;		/* ?<count> type */
typedef struct _label { int val[6]; } label_t;
typedef	ulong_t paddr_t;	/* <physical address> type */
typedef	uchar_t use_t;		/* use count for swap.  */
typedef	short		sysid_t;
typedef	short		index_t;
typedef	short		lock_t;		/* lock work for busy wait */
typedef enum boolean { B_FALSE, B_TRUE } boolean_t;
/*
 * New type from XENIX
 */
typedef char *      faddr_t;    /* same as caddr_t for 8086/386 */


typedef ulong_t k_sigset_t;	/* kernel signal set type */
typedef ulong_t k_fltset_t;	/* kernel fault set type */

/*
 * The following type is for various kinds of identifiers.  The
 * actual type must be the same for all since some system calls
 * (such as sigsend) take arguments that may be any of these
 * types.  The enumeration type idtype_t defined in sys/procset.h
 * is used to indicate what type of id is being specified.
 */

typedef long		id_t;		/* A process id,	*/
					/* process group id,	*/
					/* session id, 		*/
					/* scheduling class id,	*/
					/* user id, or group id.*/


/* Typedefs for dev_t components */

typedef ulong_t	major_t;	/* major part of device number */
typedef ulong_t	minor_t;	/* minor part of device number */


/*
 * For compatilbility reasons the following typedefs (prefixed o_) 
 * can't grow regardless of the EFT definition. Although,
 * applications should not explicitly use these typedefs
 * they may be included via a system header definition.
 * WARNING: These typedefs may be removed in a future
 * release. 
 *		ex. the definitions in s5inode.h remain small 
 *			to preserve compatibility in the S5
 *			file system type.
 */
typedef	ushort_t o_mode_t;		/* old file attribute type */
typedef short	o_dev_t;		/* old device type	*/
typedef	ushort_t o_uid_t;		/* old UID type		*/
typedef	o_uid_t	o_gid_t;		/* old GID type		*/
typedef	short	o_nlink_t;		/* old file link type	*/
typedef short	o_pid_t;		/* old process id type */
typedef ushort_t o_ino_t;		/* old inode type	*/


/* POSIX and XOPEN Declarations */

typedef	int		key_t;		/* IPC key type */
typedef	ulong_t	mode_t;			/* file attribute type	*/
typedef	long	uid_t;			/* UID type		*/
typedef	uid_t	gid_t;			/* GID type		*/
typedef	ulong_t nlink_t;		/* file link type	*/
typedef ulong_t	dev_t;		/* expanded device type */
typedef ulong_t	ino_t;		/* expanded inode type */
typedef long	pid_t;			/* process id type	*/

#ifndef _SIZE_T
#define _SIZE_T
typedef	uint_t	size_t;		/* len param for string funcs */
#endif
typedef ushort_t      sel_t;      /* Selector type */

#ifndef _TIME_T
#define _TIME_T
typedef	long		time_t;		/* time of day in seconds */
#endif	/* END _TIME_T */

#ifndef _CLOCK_T
#define _CLOCK_T
typedef	long		clock_t; /* relative time in a specified resolution */
#endif	/* ifndef _CLOCK_T */


#if (defined(_KERNEL) || !defined(_POSIX_SOURCE))

typedef	struct { int r[1]; } *	physadr;
typedef	unsigned char	unchar;
typedef	unsigned short	ushort;
typedef	unsigned int	uint;
typedef	unsigned long	ulong;




#if defined(_KERNEL)

#define SHRT_MIN        -32768          /* min value of a "short int" */
#define SHRT_MAX        32767           /* max value of a "short int" */
#define USHRT_MAX       65535		/* max value of an "unsigned short int" */
#define INT_MIN         (-2147483647-1)     /* min value of an "int" */
#define INT_MAX         2147483647      /* max value of an "int" */
#define UINT_MAX        4294967295	/* max value of an "unsigned int" */
#define LONG_MIN        (-2147483647-1)		/* min value of a "long int" */
#define LONG_MAX        2147483647      /* max value of a "long int" */
#define ULONG_MAX       4294967295 	/* max value of an "unsigned long int" */

#endif	/* defined(_KERNEL) */


#define	P_MYPID	((pid_t)0)

/*
 * The following is the value of type id_t to use to indicate the
 * caller's current id.  See procset.h for the type idtype_t
 * which defines which kind of id is being specified.
 */

#define	P_MYID	(-1)
#define NOPID (pid_t)(-1)

#ifndef NODEV
#define NODEV (dev_t)(-1)
#endif

/*
 * A host identifier is used to uniquely define a particular node
 * on an rfs network.  Its type is as follows.
 */

typedef	long	hostid_t;

/*
 * The following value of type hostid_t is used to indicate the
 * current host.  The actual hostid for each host is in the
 * kernel global variable rfs_hostid.
 */

#define	P_MYHOSTID	(-1)

typedef unsigned char	u_char;
typedef unsigned short	u_short;
typedef unsigned int	u_int;
typedef unsigned long	u_long;
typedef struct _quad { long val[2]; } quad;	/* used by UFS */


/*
 * Nested include for BSD/sockets source compatibility.
 * (The select macros used to be defined here).
 */
#include <sys/select.h>

#endif /* END (defined(_KERNEL) || !defined(_POSIX_SOURCE)) */

/*
 * These were added to allow non-ANSI compilers to compile the system.
 */

#ifdef __STDC__

#ifndef _VOID
#define _VOID	void
#endif

	/* End of ANSI section */

#else

#ifndef _VOID
#define _VOID	char
#endif

#ifndef const
#define const
#endif

#ifndef volatile
#define volatile
#endif

#endif /* of non-ANSI section */

#endif	/* _SYS_TYPES_H */
/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _JBLEN

#ident	"@(#)head:setjmp.h	1.9.2.9"

#if defined(__STDC__)

#if #machine(i386)
#define _SIGJBLEN 128	/* (sizeof(ucontext_t) / sizeof (int)) */
#else
#define _SIGJBLEN 64	/* (sizeof(ucontext_t) / sizeof (int)) */
#endif

#if #machine(pdp11)
#define _JBLEN  3
#else 
#if #machine(u370)
#define _JBLEN  4
#else
#if #machine(u3b)
#define _JBLEN  11
#else   
#define _JBLEN  10
#endif	/* #machine */
#endif	/* #machine */
#endif	/* #machine */

#else 

#if i386
#define _SIGJBLEN 128	/* (sizeof(ucontext_t) / sizeof (int)) */
#else
#define _SIGJBLEN 64	/* (sizeof(ucontext_t) / sizeof (int)) */
#endif

#if pdp11
#define _JBLEN  3
#else 
#if u370
#define _JBLEN  4
#else
#if u3b
#define _JBLEN  11
#else   
#define _JBLEN  10
#endif
#endif
#endif

#endif	/* __STDC__ */

typedef int jmp_buf[_JBLEN];

#if defined(__STDC__)
extern int setjmp(jmp_buf);
extern void longjmp(jmp_buf, int);

#if __STDC__ == 0 || defined(_POSIX_SOURCE) || defined(_XOPEN_SOURCE) /* non-ANSI standard compilation */
typedef int sigjmp_buf[_SIGJBLEN];

extern int sigsetjmp(sigjmp_buf, int);
extern void siglongjmp(sigjmp_buf, int);
#endif

#if __STDC__ != 0
#define setjmp(env)	setjmp(env)
#endif

#else
typedef int sigjmp_buf[_SIGJBLEN];

extern int setjmp();
extern void longjmp();
extern int sigsetjmp();
extern void siglongjmp();

#endif  /* __STDC__ */

#endif  /* _JBLEN */
/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _LIMITS_H
#define _LIMITS_H

#ident	"@(#)head:limits.h	1.34"

/* Sizes of integral types */
#define CHAR_BIT        8       	/* max # of bits in a "char" */
#define SCHAR_MIN	(-128)		/* min value of a "signed char" */
#define SCHAR_MAX	127		/* max value of a "signed char" */
#define UCHAR_MAX	255		/* max value of an "unsigned char" */

#define MB_LEN_MAX	5

#if defined(__STDC__)
#if #machine(i386) || #machine(sparc)
#define CHAR_MIN        SCHAR_MIN	/* min value of a "char" */
#define CHAR_MAX        SCHAR_MAX	/* max value of a "char" */
#else
#define CHAR_MIN        0               /* min value of a "char" */
#define CHAR_MAX        255             /* max value of a "char" */
#endif	/* i386 || sparc */
#else
#if i386 || sparc
#define CHAR_MIN        SCHAR_MIN	/* min value of a "char" */
#define CHAR_MAX        SCHAR_MAX	/* max value of a "char" */
#else
#define CHAR_MIN        0               /* min value of a "char" */
#define CHAR_MAX        255             /* max value of a "char" */
#endif	/* i386 || sparc */
#endif	/* __STDC__ */

#define SHRT_MIN        (-32768)        /* min value of a "short int" */
#define SHRT_MAX        32767           /* max value of a "short int" */
#define USHRT_MAX       65535		/* max value of an "unsigned short int" */
#define INT_MIN         (-2147483647-1)     /* min value of an "int" */
#define INT_MAX         2147483647      /* max value of an "int" */
#define UINT_MAX        4294967295	/* max value of an "unsigned int" */
#define LONG_MIN        (-2147483647-1)		/* min value of a "long int" */
#define LONG_MAX        2147483647      /* max value of a "long int" */
#define ULONG_MAX       4294967295 	/* max value of an "unsigned long int" */

#if __STDC__ - 0 == 0 || defined(_POSIX_SOURCE) || defined(_XOPEN_SOURCE)

#define	ARG_MAX		5120		/* max length of arguments to exec */
#define	LINK_MAX	1000		/* max # of links to a single file */

#ifndef MAX_CANON
#define MAX_CANON	256		/* max bytes in a line for canoical proccessing */
#endif

#ifndef MAX_INPUT
#define	MAX_INPUT	512		/* max size of a char input buffer */
#endif

#define NGROUPS_MAX	16		/* max number of groups for a user */


#ifndef PATH_MAX
#define	PATH_MAX	1024		/* max # of characters in a path name */
#endif

#if defined(__STDC__)
#if #machine(u3b15)
#define	PIPE_BUF	4096		/* max # bytes atomic in write to a pipe */
#else
#define	PIPE_BUF	5120		/* max # bytes atomic in write to a pipe */
#endif 	/* u3b15 */
#else	/* not __STDC__ */
#if u3b15
#define	PIPE_BUF	4096		/* max # bytes atomic in write to a pipe */
#else
#define	PIPE_BUF	5120		/* max # bytes atomic in write to a pipe */
#endif 	/* u3b15 */
#endif	/* __STDC__ */

#ifndef TMP_MAX
#define TMP_MAX		17576	/* 26 * 26 * 26 */
#endif

/* POSIX conformant definitions - An implementation may define
** other symbols which reflect the actual implementation. Alternate
** definitions may not be as restrictive as the POSIX definitions.
*/

#define _POSIX_ARG_MAX		4096
#define _POSIX_CHILD_MAX	   6
#define _POSIX_LINK_MAX		   8
#define _POSIX_MAX_CANON	 255
#define _POSIX_MAX_INPUT	 255
#define _POSIX_NAME_MAX		  14
#define _POSIX_NGROUPS_MAX	   0
#define _POSIX_OPEN_MAX		  16
#define _POSIX_PATH_MAX		 255
#define _POSIX_PIPE_BUF		 512

#endif

#if (__STDC__ - 0 == 0 && !defined(_POSIX_SOURCE)) || defined(_XOPEN_SOURCE)

#define	PASS_MAX	8		/* max # of characters in a password */

#define	NL_ARGMAX	9		/* max value of "digit" in calls to the
					 * NLS printf() and scanf() */
#define	NL_LANGMAX	14		/* max # of bytes in a LANG name */
#define	NL_MSGMAX	32767		/* max message number */
#define	NL_NMAX		1		/* max # of bytes in N-to-1 mapping characters */
#define	NL_SETMAX	255		/* max set number */
#define	NL_TEXTMAX	255		/* max set number */
#define	NZERO		20		/* default process priority */

#define	WORD_BIT	32		/* # of bits in a "word" or "int" */
#define	LONG_BIT	32		/* # of bits in a "long" */

#define	DBL_DIG		15		/* digits of precision of a "double" */
#define	DBL_MAX		1.7976931348623157E+308  /* max decimal value of a "double"*/
#define	DBL_MIN		2.2250738585072014E-308  /* min decimal value of a "double"*/
#define	FLT_DIG		6		/* digits of precision of a "float" */
#define	FLT_MAX		3.40282347E+38F /* max decimal value of a "float" */
#define	FLT_MIN		1.17549435E-38F /* min decimal value of a "float" */

#endif

#if __STDC__ - 0 == 0 && !defined(_POSIX_SOURCE) && !defined(_XOPEN_SOURCE)

#define	FCHR_MAX	1048576		/* max size of a file in bytes */
#define	PID_MAX		30000		/* max value for a process ID */
#define	CHILD_MAX	25		/* max # of processes per user id */
#define	NAME_MAX	14		/* max # of characters in a file name */

#ifndef OPEN_MAX
#if i386
#define	OPEN_MAX	60		/* max # of files a process can have open */
#else
#define	OPEN_MAX	20		/* max # of files a process can have open */
#endif
#endif

#if defined(__STDC__)
#if #machine(u3b15)
#define	PIPE_MAX	4096		/* max # bytes written to a pipe in a write */
#else
#define	PIPE_MAX	5120		/* max # bytes written to a pipe in a write */
#endif 	/* u3b15 */
#else	/* not __STDC__ */
#if u3b15
#define	PIPE_MAX	4096		/* max # bytes written to a pipe in a write */
#else
#define	PIPE_MAX	5120		/* max # bytes written to a pipe in a write */
#endif 	/* u3b15 */
#endif	/* __STDC__ */

#define	STD_BLK		1024		/* # bytes in a physical I/O block */
#define	UID_MAX		60002		/* max value for a user or group ID */
#define	USI_MAX		4294967295	/* max decimal value of an "unsigned" */
#define	SYSPID_MAX	1		/* max pid of system processes */

#if !defined(_STYPES)
#define SYS_NMLN	257	/* 4.0 size of utsname elements */
				/* also defined in sys/utsname.h */
#else
#define SYS_NMLN	9	/* old size of utsname elements */
#endif	/* _STYPES */

#ifndef CLK_TCK
#define CLK_TCK	_sysconf(3)	/* 3B2 clock ticks per second */
				/* 3 is _SC_CLK_TCK */
#endif

#define LOGNAME_MAX	8		/* max # of characters in a login name */

#endif

#endif	/* _LIMITS_H */
/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _SYS_TIME_H
#define _SYS_TIME_H

#ident	"@(#)head.sys:sys/time.h	1.16.3.1"

/*
 * Structure returned by gettimeofday(2) system call,
 * and used in other calls.
 */

#include <sys/types.h>

#if !defined(_POSIX_SOURCE) 
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
#define	DST_GB		7	/* Great Britain and Eire */
#define	DST_RUM		8	/* Rumania */
#define	DST_TUR		9	/* Turkey */
#define	DST_AUSTALT	10	/* Australian style with shift in 1986 */

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

/*
 * Time expressed in seconds and nanoseconds
 */
#endif /* !defined(_POSIX_SOURCE) */ 

typedef struct 	timestruc {
	time_t 		tv_sec;		/* seconds */
	long		tv_nsec;	/* and nanoseconds */
} timestruc_t;

#ifdef _KERNEL
/*
 * Bump a timestruc by a small number of nsec
 */

#define	BUMPTIME(t, nsec, flag) { \
	register timestruc_t	*tp = (t); \
\
	tp->tv_nsec += (nsec); \
	if (tp->tv_nsec >= 1000000000) { \
		tp->tv_nsec -= 1000000000; \
		tp->tv_sec++; \
		flag = 1; \
	} \
}

extern	timestruc_t	hrestime;
#endif

#if !defined(_KERNEL) && !defined(_POSIX_SOURCE)
#if defined(__STDC__)
int adjtime(struct timeval *, struct timeval *);
int getitimer(int, struct itimerval *);
int setitimer(int, struct itimerval *, struct itimerval *);
#endif /* __STDC__ */
#if !defined(_XOPEN_SOURCE)
#include <time.h>
#endif
#endif /* _KERNEL */


#endif	/* _SYS_TIME_H */
/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _ERRNO_H
#define _ERRNO_H

#ident	"@(#)head:errno.h	1.4.1.5"

/*
 * Error codes
 */

#include <sys/errno.h>

extern int errno;

#endif	/* _ERRNO_H */
/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef	_SYS_SOCKET_H
#define	_SYS_SOCKET_H

#ident	"@(#)head.sys:sys/socket.h	1.10.4.1"

/*
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * 		PROPRIETARY NOTICE (Combined)
 * 
 * This source code is unpublished proprietary information
 * constituting, or derived under license from AT&T's UNIX(r) System V.
 * In addition, portions of such source code were derived from Berkeley
 * 4.3 BSD under license from the Regents of the University of
 * California.
 * 
 * 
 * 
 * 		Copyright Notice 
 * 
 * Notice of copyright on this source code product does not indicate 
 * publication.
 * 
 * 	(c) 1986,1987,1988,1989  Sun Microsystems, Inc
 * 	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
 * 	          All rights reserved.
 *  
 */

#ifndef _KERNEL
#include <sys/netconfig.h>
#endif

/*
 * Definitions related to sockets: types, address families, options.
 */

#ifndef NC_TPI_CLTS
#define NC_TPI_CLTS	1		/* must agree with netconfig.h */
#define NC_TPI_COTS	2		/* must agree with netconfig.h */
#define NC_TPI_COTS_ORD	3		/* must agree with netconfig.h */
#define	NC_TPI_RAW	4		/* must agree with netconfig.h */
#endif /* !NC_TPI_CLTS */

/*
 * Types
 */
#define	SOCK_STREAM	NC_TPI_COTS	/* stream socket */
#define	SOCK_DGRAM	NC_TPI_CLTS	/* datagram socket */
#define	SOCK_RAW	NC_TPI_RAW	/* raw-protocol interface */
#define	SOCK_RDM	5		/* reliably-delivered message */
#define	SOCK_SEQPACKET	6		/* sequenced packet stream */

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
#define SO_ORDREL	0x0200		/* give use orderly release */
#define SO_IMASOCKET	0x0400		/* use socket semantics */

/*
 * N.B.: The following definition is present only for compatibility
 * with release 3.0.  It will disappear in later releases.
 */
#define	SO_DONTLINGER	(~SO_LINGER)	/* ~SO_LINGER */

/*
 * Additional options, not kept in so_options.
 */
#define	SO_SNDBUF	0x1001		/* send buffer size */
#define	SO_RCVBUF	0x1002		/* receive buffer size */
#define	SO_SNDLOWAT	0x1003		/* send low-water mark */
#define	SO_RCVLOWAT	0x1004		/* receive low-water mark */
#define	SO_SNDTIMEO	0x1005		/* send timeout */
#define	SO_RCVTIMEO	0x1006		/* receive timeout */
#define	SO_ERROR	0x1007		/* get error status and clear */
#define	SO_TYPE		0x1008		/* get socket type */
#define SO_PROTOTYPE	0x1009		/* get/set protocol type */

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
#define AF_UNSPEC       0		/* unspecified */
#define AF_UNIX         1		/* local to host (pipes, portals) */
#define AF_INET         2		/* internetwork: UDP, TCP, etc. */
#define AF_IMPLINK      3		/* arpanet imp addresses */
#define AF_PUP          4		/* pup protocols: e.g. BSP */
#define AF_CHAOS        5		/* mit CHAOS protocols */
#define AF_NS           6		/* XEROX NS protocols */
#define AF_NBS          7		/* nbs protocols */
#define AF_ECMA         8		/* european computer manufacturers */
#define AF_DATAKIT      9		/* datakit protocols */
#define AF_CCITT        10		/* CCITT protocols, X.25 etc */
#define AF_SNA          11		/* IBM SNA */
#define AF_DECnet       12		/* DECnet */
#define AF_DLI          13		/* Direct data link interface */
#define AF_LAT          14		/* LAT */
#define AF_HYLINK       15		/* NSC Hyperchannel */
#define AF_APPLETALK    16		/* Apple Talk */
#define AF_NIT          17		/* Network Interface Tap */
#define AF_802          18		/* IEEE 802.2, also ISO 8802 */
#define AF_OSI          19		/* umbrella for all families used
#define AF_X25          20		/* CCITT X.25 in particular */
#define AF_OSINET       21		/* AFI = 47, IDI = 4 */
#define AF_GOSIP        22		/* U.S. Government OSI */
#define	AF_MAX		22

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
#define	PF_DECnet	AF_DECnet
#define	PF_DLI		AF_DLI
#define	PF_LAT		AF_LAT
#define	PF_HYLINK	AF_HYLINK
#define	PF_APPLETALK	AF_APPLETALK
#define	PF_NIT		AF_NIT
#define	PF_802		AF_802
#define	PF_OSI		AF_OSI
#define	PF_X25		AF_X25
#define	PF_OSINET	AF_OSINET
#define	PF_GOSIP	AF_GOSIP

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
 * An option specification consists of an opthdr, followed by the value of
 * the option.  An options buffer contains one or more options.  The len
 * field of opthdr specifies the length of the option value in bytes.  This
 * length must be a multiple of sizeof(long) (use OPTLEN macro).
 */

struct opthdr {
	long            level;	/* protocol level affected */
	long            name;	/* option to modify */
	long            len;	/* length of option value */
};

#define OPTLEN(x) ((((x) + sizeof(long) - 1) / sizeof(long)) * sizeof(long))
#define OPTVAL(opt) ((char *)(opt + 1))

/*
 * the optdefault structure is used for internal tables of option default
 * values.
 */
struct optdefault {
	int             optname;/* the option */
	char           *val;	/* ptr to default value */
	int             len;	/* length of value */
};

/*
 * the opproc structure is used to build tables of options processing
 * functions for dooptions().
 */
struct opproc {
	int             level;	/* options level this function handles */
	int             (*func) ();	/* the function */
};

/*
 * This structure is used to encode pseudo system calls
 */
struct socksysreq {
	int             args[7];
};

/*
 * This structure is used for adding new protocols to the list supported by
 * sockets.
 */

struct socknewproto {
	int             family;	/* address family (AF_INET, etc.) */
	int             type;	/* protocol type (SOCK_STREAM, etc.) */
	int             proto;	/* per family proto number */
	dev_t           dev;	/* major/minor to use (must be a clone) */
	int             flags;	/* protosw flags */
};


/* defines for user/kernel interface */

#if (INTEL == 31) || (ATT == 31)
#define SOCKETSYS	88	/* MUST BE CHANGED DEPENDING ON OS/SYSENT.C!! */
#else
#define SOCKETSYS	83	/* MUST BE CHANGED DEPENDING ON OS/SYSENT.C!! */
#endif

#define  SO_ACCEPT	1
#define  SO_BIND	2
#define  SO_CONNECT	3
#define  SO_GETPEERNAME	4
#define  SO_GETSOCKNAME	5
#define  SO_GETSOCKOPT	6
#define  SO_LISTEN	7
#define  SO_RECV	8
#define  SO_RECVFROM	9
#define  SO_SEND	10
#define  SO_SENDTO	11
#define  SO_SETSOCKOPT	12
#define  SO_SHUTDOWN	13
#define  SO_SOCKET	14
#define  SO_SOCKPOLL	15
#define  SO_GETIPDOMAIN	16
#define  SO_SETIPDOMAIN	17
#define  SO_ADJTIME	18

#endif	/* _SYS_SOCKET_H */
/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _SYS_TIMES_H
#define _SYS_TIMES_H

#ident	"@(#)head.sys:sys/times.h	11.9.3.1"

#include <sys/types.h>

/*
 * Structure returned by times()
 */
struct tms {
	clock_t	tms_utime;		/* user time */
	clock_t	tms_stime;		/* system time */
	clock_t	tms_cutime;		/* user time, children */
	clock_t	tms_cstime;		/* system time, children */
};

#if !defined(_KERNEL)
#if defined(__STDC__)
clock_t times(struct tms *);
#else
clock_t times();
#endif
#endif

#endif	/* _SYS_TIMES_H */
/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _SIGNAL_H
#define _SIGNAL_H

#ident	"@(#)head:signal.h	1.5.3.4"

typedef int 	sig_atomic_t;

extern char *_sys_siglist[];
extern int _sys_nsig;

#include <sys/signal.h>

#if defined(__STDC__)

extern void (*signal(int, void (*)(int)))(int);
extern int raise(int);

#if __STDC__ == 0 || defined(_POSIX_SOURCE) || defined(_XOPEN_SOURCE)
#include <sys/types.h>
extern int kill(pid_t, int);
extern int sigaction(int, const struct sigaction *, struct sigaction *);
extern int sigaddset(sigset_t *, int);
extern int sigdelset(sigset_t *, int);
extern int sigemptyset(sigset_t *);
extern int sigfillset(sigset_t *);
extern int sigismember(const sigset_t *, int);
extern int sigpending(sigset_t *);
extern int sigprocmask(int, const sigset_t *, sigset_t *);
extern int sigsuspend(sigset_t *);
#endif

#if __STDC__ == 0 && !defined(_POSIX_SOURCE)
#include <sys/procset.h>
extern int gsignal(int);
extern void (*sigset(int, void (*)(int)))(int);
extern int sighold(int);
extern int sigrelse(int);
extern int sigignore(int);
extern int sigpause(int);
extern int (*ssignal(int, int (*)(int)))(int);
extern int sigaltstack(const stack_t *, stack_t *);
extern int sigsend(idtype_t, id_t, int);
extern int sigsendset(const procset_t *, int);
#endif

#else
extern	void(*signal())();
extern  void(*sigset())();

#endif 	/* __STDC__ */

#endif 	/* _SIGNAL_H */
/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)head:fcntl.h	1.6.1.7"

#include <sys/types.h>
#include <sys/fcntl.h>

#if defined(__STDC__)

extern int fcntl(int, int, ...);
extern int open(const char *, int, ...);
extern int creat(const char *, mode_t);

#else

extern int fcntl();
extern int open();
extern int creat();

#endif
/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _UNISTD_H
#define _UNISTD_H

#ident	"@(#)head:unistd.h	1.26"

/* Symbolic constants for the "access" routine: */
#define	R_OK	4	/* Test for Read permission */
#define	W_OK	2	/* Test for Write permission */
#define	X_OK	1	/* Test for eXecute permission */
#define	F_OK	0	/* Test for existence of File */

#if !defined(_POSIX_SOURCE) 
#define F_ULOCK	0	/* Unlock a previously locked region */
#define F_LOCK	1	/* Lock a region for exclusive use */
#define F_TLOCK	2	/* Test and lock a region for exclusive use */
#define F_TEST	3	/* Test a region for other processes locks */
#endif /* !defined(_POSIX_SOURCE) */ 


/* Symbolic constants for the "lseek" routine: */
#define	SEEK_SET	0	/* Set file pointer to "offset" */
#define	SEEK_CUR	1	/* Set file pointer to current plus "offset" */
#define	SEEK_END	2	/* Set file pointer to EOF plus "offset" */

#if !defined(_POSIX_SOURCE) 
/* Path names: */
#define	GF_PATH	"/etc/group"	/* Path name of the "group" file */
#define	PF_PATH	"/etc/passwd"	/* Path name of the "passwd" file */
#endif /* !defined(_POSIX_SOURCE) */ 

#include <sys/unistd.h>


/* compile-time symbolic constants,
** Support does not mean the feature is enabled.
** Use pathconf/sysconf to obtain actual configuration value.
** 
*/

#define _POSIX_JOB_CONTROL	1
#define _POSIX_SAVED_IDS	1

#ifndef _POSIX_VDISABLE
#define _POSIX_VDISABLE		0
#endif

#ifndef	NULL
#define NULL	0
#endif

#define	STDIN_FILENO	0
#define STDOUT_FILENO	1
#define STDERR_FILENO	2

/* Current version of POSIX */
#define _POSIX_VERSION		198808L

/* Current version of XOPEN */
#define _XOPEN_VERSION	3

#if defined(__STDC__)

#include <sys/types.h>

extern int access(const char *, int);
#if !defined(_POSIX_SOURCE) 
extern int acct(const char *);
#endif /* !defined(_POSIX_SOURCE) */ 
extern unsigned alarm(unsigned);
#if !defined(_POSIX_SOURCE) 
extern int brk(void *);
#endif /* !defined(_POSIX_SOURCE) */ 
extern int chdir(const char *);
extern int chown(const char *, uid_t, gid_t);
#if !defined(_POSIX_SOURCE) 
extern int chroot(const char *);
#endif /* !defined(_POSIX_SOURCE) */ 
extern int close(int);
extern char *ctermid(char *);
extern char *cuserid(char *);
extern int dup(int);
extern int dup2(int, int);
extern int execl(const char *, const char *, ...);
extern int execle(const char *, const char *, ...);
extern int execlp(const char *, const char *, ...);
extern int execv(const char *, char *const *);
extern int execve(const char *, char *const *, char *const *);
extern int execvp(const char *, char *const *);
extern void exit(int);
extern void _exit(int);
#if !defined(_POSIX_SOURCE)
extern int fattach(int, const char *);
extern int fchdir(int);
extern int fchown(int,uid_t, gid_t);
extern int fdetach(const char *);
#endif /* !defined(_POSIX_SOURCE) */ 
extern pid_t fork(void);
extern long fpathconf(int, int);
#if !defined(_POSIX_SOURCE)
extern int fsync(int);
extern int ftruncate(int, off_t);
#endif /* !defined(_POSIX_SOURCE) */ 
extern char *getcwd(char *, int);
extern gid_t getegid(void);
extern uid_t geteuid(void);
extern gid_t getgid(void);
extern int getgroups(int, gid_t *);
extern char *getlogin(void);
#if !defined(_POSIX_SOURCE)
extern pid_t getpgid(pid_t);
#endif /* !defined(_POSIX_SOURCE) */ 
extern pid_t getpid(void);
extern pid_t getppid(void);
extern pid_t getpgrp(void);
#if !defined(_POSIX_SOURCE)
char *gettxt(const char *, const char *);
#endif /* !defined(_POSIX_SOURCE) */ 
#if !defined(_POSIX_SOURCE)
extern pid_t getsid(pid_t);
#endif /* !defined(_POSIX_SOURCE) */ 
extern uid_t getuid(void);
#if !defined(_POSIX_SOURCE) 
extern int ioctl(int, int, ...);
#endif /* !defined(_POSIX_SOURCE) */ 
extern int isatty(int);
extern int link(const char *, const char *);
#if !defined(_POSIX_SOURCE) 
extern int lchown(const char *, uid_t, gid_t);
extern int lockf(int, int, long);
#endif /* !defined(_POSIX_SOURCE) */ 
extern off_t lseek(int, off_t, int);
#if !defined(_POSIX_SOURCE) 
extern int mincore(caddr_t, size_t, char *);
extern int nice(int);
#endif /* !defined(_POSIX_SOURCE) */ 
extern long pathconf(const char *, int);
extern int pause(void);
extern int pipe(int *);
#if !defined(_POSIX_SOURCE) 
extern void profil(unsigned short *, unsigned int, unsigned int, unsigned int);
extern int ptrace(int, pid_t, int, int);
#endif /* !defined(_POSIX_SOURCE) */ 
extern int read(int, void *, unsigned);
#if !defined(_POSIX_SOURCE) 
extern int readlink(const char *, void *, int);
#endif /* !defined(_POSIX_SOURCE) */ 
extern int rename(const char *, const char *);
extern int rmdir(const char *);
#if !defined(_POSIX_SOURCE) 
extern void *sbrk(int);
#endif /* !defined(_POSIX_SOURCE) */ 
extern int setgid(gid_t);
#if !defined(_POSIX_SOURCE) 
extern int setgroups(int, const gid_t *);
#endif /* !defined(_POSIX_SOURCE) */ 
extern int setpgid(pid_t, pid_t);
#if !defined(_POSIX_SOURCE) 
extern pid_t setpgrp(void);
#endif /* !defined(_POSIX_SOURCE) */ 
extern pid_t setsid(void);
extern int setuid(uid_t);
extern unsigned sleep(unsigned);
#if !defined(_POSIX_SOURCE) 
extern int stime(const time_t *);
extern int symlink(const char *, const char *);
extern void sync(void);
#endif /* !defined(_POSIX_SOURCE) */ 
extern long sysconf(int);
extern pid_t tcgetpgrp(int);
extern int tcsetpgrp(int, pid_t);
#if !defined(_POSIX_SOURCE) 
extern int truncate(const char *, off_t);
#endif /* !defined(_POSIX_SOURCE) */ 
extern char *ttyname(int);
extern int unlink(const char *);
#if !defined(_POSIX_SOURCE)
extern pid_t vfork(void);
#endif /* !defined(_POSIX_SOURCE) */ 
extern int write(int, const void *, unsigned);

#else
extern int access();
#if !defined(_POSIX_SOURCE) 
extern int acct();
#endif /* !defined(_POSIX_SOURCE) */ 
extern unsigned alarm();
#if !defined(_POSIX_SOURCE) 
extern int brk();
#endif /* !defined(_POSIX_SOURCE) */ 
extern int chdir();
extern int chown();
#if !defined(_POSIX_SOURCE) 
extern int chroot();
#endif /* !defined(_POSIX_SOURCE) */ 
extern int close();
extern char *ctermid();
extern char *cuserid();
extern int dup();
extern int dup2();
extern int execl();
extern int execle();
extern int execlp();
extern int execv();
extern int execve();
extern int execvp();
extern void exit();
extern void _exit();
#if !defined(_POSIX_SOURCE)
extern int fattach();
extern int fchdir();
extern int fchown();
extern int fdetach();
#endif /* !defined(_POSIX_SOURCE) */ 
extern int fork();
extern long fpathconf();
#if !defined(_POSIX_SOURCE)
extern int fsync();
extern int ftruncate();
#endif /* !defined(_POSIX_SOURCE) */ 
extern char *getcwd();
extern int getegid();
extern int geteuid();
extern int getgid();
extern int getgroups();
extern char *getlogin();
#if !defined(_POSIX_SOURCE)
extern int getpgid();
#endif /* !defined(_POSIX_SOURCE) */ 
extern int getpid();
extern int getppid();
extern int getpgrp();
#if !defined(_POSIX_SOURCE)
extern int getsid();
#endif /* !defined(_POSIX_SOURCE) */ 
#if !defined(_POSIX_SOURCE)
char *gettxt();
#endif /* !defined(_POSIX_SOURCE) */ 
extern int getuid();
#if !defined(_POSIX_SOURCE) 
extern int ioctl();
#endif /* !defined(_POSIX_SOURCE) */ 
extern int isatty();
#if !defined(_POSIX_SOURCE) 
extern int lchown();
#endif /* !defined(_POSIX_SOURCE) */ 
extern int link();
#if !defined(_POSIX_SOURCE) 
extern int lockf();
#endif /* !defined(_POSIX_SOURCE) */ 
extern long lseek();
#if !defined(_POSIX_SOURCE) 
extern int mincore();
extern int nice();
#endif /* !defined(_POSIX_SOURCE) */ 
extern long pathconf();
extern int pause();
extern int pipe();
#if !defined(_POSIX_SOURCE) 
extern void profil();
extern int ptrace();
#endif /* !defined(_POSIX_SOURCE) */ 
extern int read();
#if !defined(_POSIX_SOURCE) 
extern int readlink();
#endif /* !defined(_POSIX_SOURCE) */ 
extern int rmdir();
#if !defined(_POSIX_SOURCE) 
extern void *sbrk();
#endif /* !defined(_POSIX_SOURCE) */ 
extern int setgid();
#if !defined(_POSIX_SOURCE) 
extern int setgroups();
#endif /* !defined(_POSIX_SOURCE) */ 
extern int setpgid();
#if !defined(_POSIX_SOURCE) 
extern int setpgrp();
#endif /* !defined(_POSIX_SOURCE) */ 
extern int setsid();
extern int setuid();
extern unsigned sleep();
#if !defined(_POSIX_SOURCE) 
extern int stime();
extern int symlink();
extern void sync();
#endif /* !defined(_POSIX_SOURCE) */ 
extern long sysconf();
extern int tcgetpgrp();
extern int tcsetpgrp();
#if !defined(_POSIX_SOURCE)
extern int truncate();
#endif /* !defined(_POSIX_SOURCE) */ 
extern char *ttyname();
extern int unlink();
#if !defined(_POSIX_SOURCE)
extern int vfork();
#endif /* !defined(_POSIX_SOURCE) */ 
extern int write();

#endif

#endif /* _UNISTD_H */
/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _SYS_STAT_H
#define _SYS_STAT_H

#ident	"@(#)head.sys:sys/stat.h	11.29.3.1"

#include <sys/time.h>

#define _ST_FSTYPSZ 16		/* array size for file system type name */

/*
 * stat structure, used by stat(2) and fstat(2)
 */

#if defined(_KERNEL)

	/* SVID stat struct */
struct	stat {
	o_dev_t	st_dev;
	o_ino_t	st_ino;
	o_mode_t st_mode;
	o_nlink_t st_nlink;
	o_uid_t st_uid;
	o_gid_t st_gid;
	o_dev_t	st_rdev;
	off_t	st_size;
	time_t	st_atime;
	time_t	st_mtime;
	time_t	st_ctime;
};
	/* Expanded stat structure */ 

struct	xstat {
	dev_t	st_dev;
	long	st_pad1[3];	/* reserve for dev expansion, sysid definition */
	ino_t	st_ino;
	mode_t	st_mode;
	nlink_t st_nlink;
	uid_t 	st_uid;
	gid_t 	st_gid;
	dev_t	st_rdev;
	long	st_pad2[2];
	off_t	st_size;
	long	st_pad3;	/* reserve pad for future off_t expansion */
	timestruc_t st_atime;
	timestruc_t st_mtime;
	timestruc_t st_ctime;
	long	st_blksize;
	long	st_blocks;
	char	st_fstype[_ST_FSTYPSZ];
	long	st_pad4[8];	/* expansion area */
};

#else /* !defined(_KERNEL) */
#if !defined(_STYPES)	/* user level 4.0 stat struct */

/* maps to kernel struct xstat */
struct	stat {
	dev_t	st_dev;
	long	st_pad1[3];	/* reserved for network id */
	ino_t	st_ino;
	mode_t	st_mode;
	nlink_t st_nlink;
	uid_t 	st_uid;
	gid_t 	st_gid;
	dev_t	st_rdev;
	long	st_pad2[2];
	off_t	st_size;
	long	st_pad3;	/* future off_t expansion */
	timestruc_t st_atim;	
	timestruc_t st_mtim;	
	timestruc_t st_ctim;	
	long	st_blksize;
	long	st_blocks;
	char	st_fstype[_ST_FSTYPSZ];
	long	st_pad4[8];	/* expansion area */
};
#define st_atime	st_atim.tv_sec
#define st_mtime	st_mtim.tv_sec
#define st_ctime	st_ctim.tv_sec

#else	/*  SVID Issue 2 stat */

struct	stat {
	o_dev_t	st_dev;
	o_ino_t	st_ino;
	o_mode_t st_mode;
	o_nlink_t st_nlink;
	o_uid_t	st_uid;
	o_gid_t	st_gid;
	o_dev_t	st_rdev;
	off_t	st_size;
	time_t	st_atime;
	time_t	st_mtime;
	time_t	st_ctime;
};
#endif	/* end !defined(_STYPES) */
#endif	/* end defined(_KERNEL) */


/* MODE MASKS */

/* de facto standard definitions */

#if !defined(_POSIX_SOURCE) || defined(_XOPEN_SOURCE) 
#define	S_IFMT		0xF000	/* type of file */
#endif /*!defined(_POSIX_SOURCE) || defined(_XOPEN_SOURCE)*/ 
#if !defined(_POSIX_SOURCE) 
#define S_IAMB		0x1FF	/* access mode bits */
#endif /* !defined(_POSIX_SOURCE) */
#if !defined(_POSIX_SOURCE) || defined(_XOPEN_SOURCE) 
#define	S_IFIFO		0x1000	/* fifo */
#define	S_IFCHR		0x2000	/* character special */
#define	S_IFDIR		0x4000	/* directory */
#endif /*!defined(_POSIX_SOURCE) || defined(_XOPEN_SOURCE)*/ 
#if !defined(_POSIX_SOURCE) 
#define	S_IFNAM		0x5000  /* XENIX special named file */
#define		S_INSEM 0x1	/* XENIX semaphore subtype of IFNAM */
#define		S_INSHD 0x2	/* XENIX shared data subtype of IFNAM */
#endif /* !defined(_POSIX_SOURCE) */

#if !defined(_POSIX_SOURCE) || defined(_XOPEN_SOURCE) 
#define	S_IFBLK		0x6000	/* block special */
#define	S_IFREG		0x8000	/* regular */
#endif /*!defined(_POSIX_SOURCE) || defined(_XOPEN_SOURCE)*/ 
#if !defined(_POSIX_SOURCE) 
#define	S_IFLNK		0xA000	/* symbolic link */
#define	S_IFSOCK	0xC000	/* socket */
#endif /* !defined(_POSIX_SOURCE) */

#define	S_ISUID		0x800	/* set user id on execution */
#define	S_ISGID		0x400	/* set group id on execution */

#if !defined(_POSIX_SOURCE) || defined(_XOPEN_SOURCE) 
#define	S_ISVTX		0x200	/* save swapped text even after use */
#endif /* !defined(_POSIX_SOURCE) || defined(_XOPEN_SOURCE) */

#if !defined(_POSIX_SOURCE) 
#define	S_IREAD		00400	/* read permission, owner */
#define	S_IWRITE	00200	/* write permission, owner */
#define	S_IEXEC		00100	/* execute/search permission, owner */
#define	S_ENFMT		S_ISGID	/* record locking enforcement flag */
#endif /* !defined(_POSIX_SOURCE) */


/* the following macros are for POSIX conformance */

#define	S_IRWXU	00700		/* read, write, execute: owner */
#define	S_IRUSR	00400		/* read permission: owner */
#define	S_IWUSR	00200		/* write permission: owner */
#define	S_IXUSR	00100		/* execute permission: owner */
#define	S_IRWXG	00070		/* read, write, execute: group */
#define	S_IRGRP	00040		/* read permission: group */
#define	S_IWGRP	00020		/* write permission: group */
#define	S_IXGRP	00010		/* execute permission: group */
#define	S_IRWXO	00007		/* read, write, execute: other */
#define	S_IROTH	00004		/* read permission: other */
#define	S_IWOTH	00002		/* write permission: other */
#define	S_IXOTH	00001		/* execute permission: other */


#define S_ISFIFO(mode)	((mode&0xF000) == 0x1000)
#define S_ISCHR(mode)	((mode&0xF000) == 0x2000)
#define S_ISDIR(mode)	((mode&0xF000) == 0x4000)
#define S_ISBLK(mode)	((mode&0xF000) == 0x6000)
#define S_ISREG(mode)	((mode&0xF000) == 0x8000) 


/* a version number is included in the SVR4 stat and mknod interfaces. */


#define _R3_MKNOD_VER 1		/* SVR3.0 mknod */
#define _MKNOD_VER 2		/* current version of mknod */
#define _R3_STAT_VER 1		/* SVR3.0 stat */
#define _STAT_VER 2		/* current version of stat */

#if !defined(_KERNEL)
#if defined(__STDC__)

#if !defined(_STYPES)
static int fstat(int, struct stat *);
static int stat(const char *, struct stat *);
#if !defined(_POSIX_SOURCE) 
static int lstat(const char *, struct stat *);
static int mknod(const char *, mode_t, dev_t);
#endif /* !defined(_POSIX_SOURCE) */
#else
int fstat(int, struct stat *);
int stat(const char *, struct stat *);
#if !defined(_POSIX_SOURCE) 
int lstat(const char *, struct stat *);
int mknod(const char *, mode_t, dev_t);
#endif /* !defined(_POSIX_SOURCE) */
#endif

int _fxstat(const int, int, struct stat *);
int _xstat(const int, const char *, struct stat *);
#if !defined(_POSIX_SOURCE) 
int _lxstat(const int, const char *, struct stat *);
int _xmknod(const int, const char *, mode_t, dev_t);
#endif /* !defined(_POSIX_SOURCE) */
extern int chmod(const char *, mode_t);
extern int mkdir(const char *, mode_t);
extern int mkfifo(const char *, mode_t);
extern mode_t umask(mode_t);

#else	/* !__STDC__ */

#if !defined(_STYPES)
static int fstat(), stat();
#if !defined(_POSIX_SOURCE) 
static int mknod(), lstat();
#endif /* !defined(_POSIX_SOURCE) */
#else
int fstat(), stat();
#if !defined(_POSIX_SOURCE) 
int mknod(), lstat();
#endif /* !defined(_POSIX_SOURCE) */
#endif

int _fxstat(), _xstat();
#if !defined(_POSIX_SOURCE) 
int _xmknod(), _lxstat();
#endif /* !defined(_POSIX_SOURCE) */
extern int chmod();
extern int mkdir();
extern int mkfifo();
extern mode_t umask();

#endif /* defined(__STDC__) */
#endif /* !defined(_KERNEL) */

/*
 * NOTE: Application software should NOT program 
 * to the _xstat interface.
 */

#if !defined(_STYPES) && !defined(_KERNEL)
static int
stat(path, buf)
const char *path;
struct stat *buf;
{
int ret;
	ret = _xstat(_STAT_VER, path, buf);
	return ret; 
}

#if !defined(_POSIX_SOURCE) 
static int
lstat(path, buf)
const char *path;
struct stat *buf;
{
int ret;
	ret = _lxstat(_STAT_VER, path, buf);
	return ret;
}
#endif /* !defined(_POSIX_SOURCE) */

static int
fstat(fd, buf)
int fd;
struct stat *buf;
{
int ret;
	ret = _fxstat(_STAT_VER, fd, buf);
	return ret;
}

#if !defined(_POSIX_SOURCE) 
static int
mknod(path, mode, dev)
const char *path;
mode_t mode;
dev_t dev;
{
int ret;
	ret = _xmknod(_MKNOD_VER, path, mode, dev);
	return ret;
}
#endif /* !defined(_POSIX_SOURCE) */

#endif
/*			Function prototypes
 *			___________________
 *
 * fstat()/stat() used for NON-EFT case - functions defined in libc.
 * fxstat/xstat/lxstat are called indirectly from fstat/stat/lstat when EFT is 
 * enabled.
 */



#endif	/* _SYS_STAT_H */
/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _SYS_PARAM_H
#define _SYS_PARAM_H

#ident	"@(#)head.sys:sys/param.h	11.28.4.1"

#include <sys/types.h>
#include <sys/fs/s5param.h>
/*
 * Fundamental variables; don't change too often.
 */
#ifndef _POSIX_VDISABLE
#define _POSIX_VDISABLE 0 /* Disable special character functions */
#endif

#ifndef MAX_INPUT
#define MAX_INPUT 512     /* Maximum bytes stored in the input queue */
#endif

#ifndef MAX_CANON
#define MAX_CANON 256     /* Maximum bytes in a line for canoical processing */
#endif

#define UID_NOBODY  60001   /* user ID no body */
#define GID_NOBODY  UID_NOBODY

#define UID_NOACCESS    60002   /* user ID no access */


#define	MAXPID	30000		/* max process id */
#define	MAXUID	60002		/* max user id */
#define	MAXLINK	1000		/* max links */

#define	SSIZE	1		/* initial stack size (*4096 bytes) */
#define	SINCR	1		/* increment of stack (*4096 bytes) */
#define	USIZE	MINUSIZE	/* inital size of user block (*4096) */
#define	MINUSIZE  2		/* min size of user block (*4096 bytes) */
#define	MAXUSIZE 18		/* max size of user block (*4096 bytes) */

#define	CANBSIZ	256		/* max size of typewriter line	*/
#define	HZ	100		/* 100 ticks/second of the clock */
#define TICK    10000000	/* nanoseconds per tick */


#define NOFILE	20		/* this define is here for	*/
				/* compatibility purposes only	*/
				/* and will be removed in a	*/
				/* later release		*/

/*
 * The following macros are no longer supported in SVR 4.0
 * since there is no longer a limit on the number of files that
 * a process can open. However, for SVR3.2 source compatibility, 
 * you may uncomment  NOFILES_MIN and NOFILES_MAX.
 */

/* #define	NOFILES_MIN	 20	SVR3.2 Source Compatibility */
/* #define	NOFILES_MAX	100	SVR3.2 Source Compatibility */

/*
 * These define the maximum and minimum allowable values of the
 * configurable parameter NGROUPS_MAX.
 */
#define	NGROUPS_UMAX	32
#define	NGROUPS_UMIN	0

/*
 * The following defines apply to the kernel virtual address space.
 */

/*
 * The size of the kernel segment table in pages.  The starting address
 * comes from the vuifile.
 */
#define MAXKSEG		127	/*max no of pages per kseg */

/*
 * To avoid prefetch errors at the end of a region, it must
 * be padded with the following number of bytes.
 */

#define	PREFETCH	0

/*
 * Priorities.  Should not be altered too much.
 */

#define	PMASK	0177
#define	PCATCH	0400
#define	PNOSTOP	01000
#define	PSWP	0
#define	PINOD	10
#define PSNDD	PINOD
#define	PRIBIO	20
#define	PZERO	25
#define PMEM	0
#define	NZERO	20
#define	PPIPE	26
#define PVFS	27
#define	PWAIT	30
#define	PSLEP	39
#define	PUSER	60
#define	PIDLE	127

/*
 * Fundamental constants of the implementation--cannot be changed easily.
 */

#define	NBPW	sizeof(int)	/* number of bytes in an integer */
#define	NCPPT	1024		/* Number of clicks per page table */
#define	CPPTSHIFT	10	/* LOG2(NCPPT) if exact */
#define	NBPC	4096		/* Number of bytes per click */
#define	BPCSHIFT	12	/* LOG2(NBPC) if exact */
#define	NULL	0
#define	CMASK	0		/* default mask for file creation */
#define	CDLIMIT	(1L<<14)	/* default max write address */
#define NBPSCTR         512     /* Bytes per LOGICAL disk sector. */
#define	UBSIZE		512	/* unix block size.		*/
#define SCTRSHFT	9	/* Shift for BPSECT.		*/

#define	UMODE	3		/* current Xlevel == user */
#define	USERMODE(cs)	(((cs) & SEL_RPL) == UMODE)

#define	lobyte(X)	(((unsigned char *)&(X))[0])
#define	hibyte(X)	(((unsigned char *)&(X))[1])
#define	loword(X)	(((ushort *)&(X))[0])
#define	hiword(X)	(((ushort *)&(X))[1])

#define	MAXSUSE	255

/* REMOTE -- whether machine is primary, secondary, or regular */
#define SYSNAME 9		/* # chars in system name */
#define PREMOTE 39

/* XENIX compatibility */
#define	ktop(vaddr)	((paddr_t)svirtophys(vaddr))

/*
 * MAXPATHLEN defines the longest permissible path length,
 * including the terminating null, after expanding symbolic links.
 * MAXSYMLINKS defines the maximum number of symbolic links
 * that may be expanded in a path name. It should be set high
 * enough to allow all legitimate uses, but halt infinite loops
 * reasonably quickly.
 * MAXNAMELEN is the length (including the terminating null) of
 * the longest permissible file (component) name.
 */
#define	MAXPATHLEN	1024
#define	MAXSYMLINKS	20
#define	MAXNAMELEN	256

#ifndef NADDR
#define NADDR 13
#endif

/*
 * The following are defined to be the same as
 * defined in /usr/include/limits.h.  They are
 * needed for pipe and FIFO compatibility.
 */
#ifndef PIPE_BUF	/* max # bytes atomic in write to a pipe */
#ifdef u3b15
#define PIPE_BUF	4096
#else
#define PIPE_BUF	5120
#endif	/* u3b15 */
#endif	/* PIPE_BUF */

#ifndef PIPE_MAX	/* max # bytes written to a pipe in a write */
#ifdef u3b15
#define PIPE_MAX	4096
#else
#define PIPE_MAX	5120
#endif	/* u3b15 */
#endif	/* PIPE_MAX */

#define NBBY	8			/* number of bits per byte */

/*
 * File system parameters and macros.
 *
 * The file system is made out of blocks of at most MAXBSIZE units,
 * with smaller units (fragments) only in the last direct block.
 * MAXBSIZE primarily determines the size of buffers in the buffer
 * pool. It may be made larger without any effect on existing
 * file systems; however making it smaller make make some file
 * systems unmountable.
 *
 * Note that the blocked devices are assumed to have DEV_BSIZE
 * "sectors" and that fragments must be some multiple of this size.
 */
#define	MAXBSIZE	8192
#define	DEV_BSIZE	512
#define	DEV_BSHIFT	9		/* log2(DEV_BSIZE) */
#define	MAXFRAG 	8

#define	btodb(bytes)	 		/* calculates (bytes / DEV_BSIZE) */ \
	((unsigned)(bytes) >> DEV_BSHIFT)
#define	dbtob(db)			/* calculates (db * DEV_BSIZE) */ \
	((unsigned)(db) << DEV_BSHIFT)

/*
 * MMU_PAGES* describes the physical page size used by the mapping hardware.
 * PAGES* describes the logical page size used by the system.
 */

#define	MMU_PAGESIZE	0x1000		/* 4096 bytes */
#define	MMU_PAGESHIFT	12		/* log2(MMU_PAGESIZE) */
#define	MMU_PAGEOFFSET	(MMU_PAGESIZE-1)/* Mask of address bits in page */
#define	MMU_PAGEMASK	(~MMU_PAGEOFFSET)

#define	PAGESIZE	0x1000		/* All of the above, for logical */
#define	PAGESHIFT	12
#define	PAGEOFFSET	(PAGESIZE - 1)
#define	PAGEMASK	(~PAGEOFFSET)

#ifndef NODEV
#define NODEV	(dev_t)(-1)
#endif

/*
 * Some random macros for units conversion.
 */

/*
 * MMU pages to bytes, and back (with and without rounding)
 */
#define	mmu_ptob(x)	((x) << MMU_PAGESHIFT)
#define	mmu_btop(x)	(((unsigned)(x)) >> MMU_PAGESHIFT)
#define	mmu_btopr(x)	((((unsigned)(x) + MMU_PAGEOFFSET) >> MMU_PAGESHIFT))

/*
 * pages to bytes, and back (with and without rounding)
 */
#define	ptob(x)		((x) << PAGESHIFT)
#define	btop(x)		(((unsigned)(x)) >> PAGESHIFT)
#define	btopr(x)	((((unsigned)(x) + PAGEOFFSET) >> PAGESHIFT))

#define shm_alignment	ctob(1)		/* segment size */


#endif	/* _SYS_PARAM_H */
/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _SYS_FILE_H
#define _SYS_FILE_H

#ident	"@(#)head.sys:sys/file.h	11.28.5.4"
/*
 * One file structure is allocated for each open/creat/pipe call.
 * Main use is to hold the read/write pointer associated with
 * each open file.
 */
typedef struct file {
	struct file  *f_next;		/* pointer to next entry */
	struct file  *f_prev;		/* pointer to previous entry */
	ushort	f_flag;
	cnt_t	f_count;		/* reference count */
	struct vnode *f_vnode;		/* pointer to vnode structure */
	off_t	f_offset;		/* read/write character pointer */
	struct	cred *f_cred;		/* credentials of user who opened it */
	struct	aioreq *f_aiof;		/* aio file list forward link	*/
	struct	aioreq *f_aiob;		/* aio file list backward link	*/
	union {
		off_t f_off;
/* XENIX Support */
		struct	file *f_slnk;	/* XENIX semaphore queue */
/* End XENIX Support */
	} f_un;
} file_t;

#define f_offset	f_un.f_off	/* read/write character pointer */

/* flags */

#define	FOPEN		0xFFFFFFFF
#define	FREAD		0x01
#define	FWRITE		0x02
#define	FNDELAY		0x04
#define	FAPPEND		0x08
#define	FSYNC		0x10

/*
 * The new flag is added for raw disk async I/O feature.
 */
#define	FRAIOSIG	0x20	/* cause a signal for a completed RAIO request */ 

#define	FNONBLOCK	0x80

#define	FMASK		0xFF	/* should be disjoint from FASYNC */

/* open-only modes */

#define FCREAT      0x0100
#define FTRUNC      0x0200
#define FEXCL       0x0400
#define FNOCTTY     0x0800

/* Internal flag used by SVR4 async i/o feature */
#define FASYNC      0x1000

/* file descriptor flags */
#define FCLOSEXEC	001	/* close on exec */

/* miscellaneous defines */

#define NULLFP ((struct file *)0)

#ifndef L_SET
#define	L_SET	0	/* for lseek */
#endif /* L_SET */

/*
 * Count of number of entries in file list.
 */
extern unsigned int filecnt;

/*
 * Routines dealing with user per-open file flags and
 * user open files.  
 */

#if defined(__STDC__)
extern int getf(int, file_t **);
extern void closeall(int);
extern int closef(file_t *);
extern int ufalloc(int, int *);
extern int falloc(struct vnode *, int, file_t **, int *);
extern void finit(void);
extern void unfalloc(file_t *);
extern void setf(int, file_t *);
extern char getpof(int);
extern void setpof(int, char);
extern int filesearch(struct vnode *);
extern int fassign(struct vnode **, int, int*);

#else

extern int getf();
extern void closeall();
extern int closef();
extern int ufalloc();
extern int falloc();
extern void finit();
extern void unfalloc();
extern void setf();
extern char getpof();
extern void setpof();
extern int filesearch();
extern int fassign();

#endif	/* __STDC__ */

#endif	/* _SYS_FILE_H */
/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)head:memory.h	1.4.1.2"

/* from stddef.h */
#ifndef _SIZE_T
#define _SIZE_T
typedef unsigned        size_t;
#endif

#if defined(__STDC__)
extern void *memccpy(void *, const void *, int, size_t);
extern void *memchr(const void *, int, size_t);
extern void *memcpy(void *, const void *, size_t);
extern void *memset(void *, int, size_t);
extern int memcmp(const void *, const void *, size_t);

#else
extern char
	*memccpy(),
	*memchr(),
	*memcpy(),
	*memset();
extern int memcmp();
#endif
/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _SYS_VFS_H
#define _SYS_VFS_H

#ident	"@(#)head.sys:sys/vfs.h	1.23.4.1"

/*
 * Data associated with mounted file systems.
 */

/*
 * File system identifier. Should be unique (at least per machine).
 */
typedef struct {
	long val[2];			/* file system id type */
} fsid_t;

/*
 * File identifier.  Should be unique per filesystem on a single
 * machine.  This is typically called by a stateless file server
 * in order to generate "file handles".
 */
#define	MAXFIDSZ	16
#define	freefid(fidp) \
  kmem_free((caddr_t)(fidp), sizeof (struct fid) - MAXFIDSZ + (fidp)->fid_len)

typedef struct fid {
	u_short		fid_len;		/* length of data in bytes */
	char		fid_data[MAXFIDSZ];	/* data (variable length) */
} fid_t;

/*
 * Structure per mounted file system.  Each mounted file system has
 * an array of operations and an instance record.  The file systems
 * are kept on a singly linked list headed by "rootvfs" and terminated
 * by NULL.
 */
typedef struct vfs {
	struct vfs	*vfs_next;		/* next VFS in VFS list */
	struct vfsops	*vfs_op;		/* operations on VFS */
	struct vnode	*vfs_vnodecovered;	/* vnode mounted on */
	u_long		vfs_flag;		/* flags */
	u_long		vfs_bsize;		/* native block size */
	int		vfs_fstype;		/* file system type index */
	fsid_t		vfs_fsid;		/* file system id */
	caddr_t		vfs_data;		/* private data */
	dev_t		vfs_dev;		/* device of mounted VFS */
	u_long		vfs_bcount;		/* I/O count (accounting) */
	u_short		vfs_nsubmounts;		/* immediate sub-mount count */
} vfs_t;

/*
 * VFS flags.
 */
#define VFS_RDONLY	0x01		/* read-only vfs */
#define VFS_MLOCK	0x02		/* lock vfs so that subtree is stable */
#define VFS_MWAIT	0x04		/* someone is waiting for lock */
#define VFS_NOSUID	0x08		/* setuid disallowed */
#define VFS_REMOUNT	0x10		/* modify mount options only */
#define VFS_NOTRUNC	0x20		/* does not truncate long file names */
#define VFS_UNLINKABLE	0x40		/* unlink(2) can be applied to root */
#define VFS_BADBLOCK	0x80		/* disk based fs has bad block */

/*
 * Argument structure for mount(2).
 */
struct mounta {
	char	*spec;
	char	*dir;
	int	flags;
	char	*fstype;
	char	*dataptr;
	int	datalen;
};

/*
 * Operations supported on virtual file system.
 */
typedef struct vfsops {
	int	(*vfs_mount)();		/* mount file system */
	int	(*vfs_unmount)();	/* unmount file system */
	int	(*vfs_root)();		/* get root vnode */
	int	(*vfs_statvfs)();	/* get file system statistics */
	int	(*vfs_sync)();		/* flush fs buffers */
	int	(*vfs_vget)();		/* get vnode from fid */
	int	(*vfs_mountroot)();	/* mount the root filesystem */
	int	(*vfs_swapvp)();	/* return vnode for swap */
	int	(*vfs_filler[8])();	/* for future expansion */
} vfsops_t;

#define VFS_MOUNT(vfsp, mvp, uap, cr) \
	(*(vfsp)->vfs_op->vfs_mount)(vfsp, mvp, uap, cr)
#define VFS_UNMOUNT(vfsp, cr)	(*(vfsp)->vfs_op->vfs_unmount)(vfsp, cr)
#define VFS_ROOT(vfsp, vpp)	(*(vfsp)->vfs_op->vfs_root)(vfsp, vpp)
#define	VFS_STATVFS(vfsp, sp)	(*(vfsp)->vfs_op->vfs_statvfs)(vfsp, sp)
#define VFS_SYNC(vfsp, flag, cr) \
		(*(vfsp)->vfs_op->vfs_sync)(vfsp, flag, cr)
#define VFS_VGET(vfsp, vpp, fidp) \
		(*(vfsp)->vfs_op->vfs_vget)(vfsp, vpp, fidp)
#define VFS_MOUNTROOT(vfsp, init) \
		 (*(vfsp)->vfs_op->vfs_mountroot)(vfsp, init)
#define VFS_SWAPVP(vfsp, vpp, nm) \
		(*(vfsp)->vfs_op->vfs_swapvp)(vfsp, vpp, nm)

/*
 * Filesystem type switch table.
 */
typedef struct vfssw {
	char		*vsw_name;	/* type name string */
	int		(*vsw_init)();	/* init routine */
	struct vfsops	*vsw_vfsops;	/* filesystem operations vector */
	long		vsw_flag;	/* flags */
} vfssw_t;

/*
 * Public operations.
 */
extern void	vfs_mountroot();	/* mount the root */
extern void	vfs_add();		/* add a new vfs to mounted vfs list */
extern void	vfs_remove();		/* remove a vfs from mounted vfs list */
extern int	vfs_lock();		/* lock a vfs */
extern void	vfs_unlock();		/* unlock a vfs */
extern vfs_t 	*getvfs();		/* return vfs given fsid */
extern vfs_t 	*vfs_devsearch();	/* find vfs given device */
extern vfssw_t 	*vfs_getvfssw();	/* find vfssw ptr given fstype name */
extern u_long	vf_to_stf();		/* map VFS flags to statfs flags */
extern int	dounmount();		/* unmount a vfs */

#define VFS_INIT(vfsp, op, data)	{ \
	(vfsp)->vfs_next = (struct vfs *)0; \
	(vfsp)->vfs_op = (op); \
	(vfsp)->vfs_flag = 0; \
	(vfsp)->vfs_data = (data); \
	(vfsp)->vfs_nsubmounts = 0; \
}

/*
 * Globals.
 */
extern struct vfs *rootvfs;		/* ptr to root vfs structure */
extern struct vfssw vfssw[];		/* table of filesystem types */
extern char rootfstype[];		/* name of root fstype */
extern int nfstype;			/* # of elements in vfssw array */

/*
 * Reasons for calling the vfs_mountroot() operation.
 */

enum whymountroot { ROOT_INIT, ROOT_REMOUNT, ROOT_UNMOUNT };
typedef enum whymountroot whymountroot_t;

/*
 * VFS_SYNC flags.
 */
#define SYNC_ATTR	0x01		/* sync attributes only */
#define SYNC_CLOSE	0x02		/* close open file */

#endif	/* _SYS_VFS_H */
/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ifndef	_NETINET_IN_H
#define	_NETINET_IN_H

#ident	"@(#)kern-inet:in.h	1.3"

/*
 *  		PROPRIETARY NOTICE (Combined)
 *  
 *  This source code is unpublished proprietary information
 *  constituting, or derived under license from AT&T's Unix(r) System V.
 *  In addition, portions of such source code were derived from Berkeley
 *  4.3 BSD under license from the Regents of the University of
 *  California.
 *  
 *  
 *  
 *  		Copyright Notice 
 *  
 *  Notice of copyright on this source code product does not indicate 
 *  publication.
 *  
 *  	(c) 1986,1987,1988,1989  Sun Microsystems, Inc.
 *  	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
 *  	          All rights reserved.
 */

/*
 * System V STREAMS TCP - Release 3.0 
 *
 * Copyright 1987, 1988, 1989 Lachman Associates, Incorporated (LAI) 
 * All Rights Reserved. 
 *
 * The copyright above and this notice must be preserved in all copies of this
 * source code.  The copyright above does not evidence any actual or intended
 * publication of this source code. 
 *
 * This is unpublished proprietary trade secret source code of Lachman
 * Associates.  This source code may not be copied, disclosed, distributed,
 * demonstrated or licensed except as expressly authorized by Lachman
 * Associates. 
 *
 * System V STREAMS TCP was jointly developed by Lachman Associates and
 * Convergent Technologies. 
 */


/*
 * Constants and structures defined by the internet system,
 * Per RFC 790, September 1981.
 */
#include <sys/stream.h>
#include <sys/byteorder.h>	/* for network <--> host byteorder conversion */
				/* macros: ntohl, ntohs, htonl, htons	      */


#define IPM_ID		200	/* Module ID for IP stream */
#define ICMPM_ID	201	/* Module ID for ICMP stream */
#define TCPM_ID		202	/* Module ID for TCP stream */
#define UDPM_ID		203	/* Module ID for UDP stream */
#define ARPM_ID		204	/* Module ID for ARP stream */
#define APPM_ID		205	/* Module ID for ProcARP stream */
#define RIPM_ID         206     /* Module ID for RIP stream */

/*
 * Protocols
 */
#define	IPPROTO_IP		0		/* dummy for IP */
#define	IPPROTO_ICMP		1		/* control message protocol */
#define IPPROTO_IGMP		2		/* group control protocol */
#define	IPPROTO_GGP		3		/* gateway^2 (deprecated) */
#define	IPPROTO_TCP		6		/* tcp */
#define	IPPROTO_EGP		8		/* exterior gateway protocol */
#define	IPPROTO_PUP		12		/* pup */
#define	IPPROTO_UDP		17		/* user datagram protocol */
#define	IPPROTO_IDP		22		/* xns idp */
#define IPPROTO_HELLO		63		/* "hello" routing protocol */
#define	IPPROTO_ND		77		/* UNOFFICIAL net disk proto */

#define	IPPROTO_RAW		255		/* raw IP packet */
#define	IPPROTO_MAX		256

/*
 * Port/socket numbers: network standard functions
 */
#define	IPPORT_ECHO		7
#define	IPPORT_DISCARD		9
#define	IPPORT_SYSTAT		11
#define	IPPORT_DAYTIME		13
#define	IPPORT_NETSTAT		15
#define	IPPORT_FTP		21
#define	IPPORT_TELNET		23
#define	IPPORT_SMTP		25
#define	IPPORT_TIMESERVER	37
#define	IPPORT_NAMESERVER	42
#define	IPPORT_WHOIS		43
#define	IPPORT_MTP		57

/*
 * Port/socket numbers: host specific functions
 */
#define	IPPORT_TFTP		69
#define	IPPORT_RJE		77
#define	IPPORT_FINGER		79
#define	IPPORT_TTYLINK		87
#define	IPPORT_SUPDUP		95

/*
 * UNIX TCP sockets
 */
#define	IPPORT_EXECSERVER	512
#define	IPPORT_LOGINSERVER	513
#define	IPPORT_CMDSERVER	514
#define	IPPORT_EFSSERVER	520

/*
 * UNIX UDP sockets
 */
#define	IPPORT_BIFFUDP		512
#define	IPPORT_WHOSERVER	513
#define	IPPORT_ROUTESERVER	520	/* 520+1 also used */

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
 * Internet address
 *	This definition contains obsolete fields for compatibility
 *	with SunOS 3.x and 4.2bsd.  The presence of subnets renders
 *	divisions into fixed fields misleading at best.  New code
 *	should use only the s_addr field.
 */
struct in_addr {
	union {
		struct { u_char s_b1,s_b2,s_b3,s_b4; } S_un_b;
		struct { u_short s_w1,s_w2; } S_un_w;
		u_long S_addr;
	} S_un;
#define	s_addr	S_un.S_addr		/* should be used for all code */
#define	s_host	S_un.S_un_b.s_b2	/* OBSOLETE: host on imp */
#define	s_net	S_un.S_un_b.s_b1	/* OBSOLETE: network */
#define	s_imp	S_un.S_un_w.s_w2	/* OBSOLETE: imp */
#define	s_impno	S_un.S_un_b.s_b4	/* OBSOLETE: imp # */
#define	s_lh	S_un.S_un_b.s_b3	/* OBSOLETE: logical host */
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

#define	IN_CLASSC(i)		(((long)(i) & 0xe0000000) == 0xc0000000)
#define	IN_CLASSC_NET		0xffffff00
#define	IN_CLASSC_NSHIFT	8
#define	IN_CLASSC_HOST		0x000000ff

#define	IN_CLASSD(i)		(((long)(i) & 0xf0000000) == 0xe0000000)
#define	IN_MULTICAST(i)		IN_CLASSD(i)

#define	IN_EXPERIMENTAL(i)	(((long)(i) & 0xe0000000) == 0xe0000000)
#define	IN_BADCLASS(i)		(((long)(i) & 0xf0000000) == 0xf0000000)

#define	INADDR_ANY		(u_long)0x00000000
#define	INADDR_LOOPBACK		(u_long)0x7F000001
#define	INADDR_BROADCAST	(u_long)0xffffffff	/* must be masked */

#define	IN_LOOPBACKNET		127			/* official! */

/*
 * Define a macro to stuff the loopback address into an Internet address
 */
#define IN_SET_LOOPBACK_ADDR(a)	{(a)->sin_addr.s_addr  = htonl(INADDR_LOOPBACK); \
	(a)->sin_family = AF_INET;}

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
 * The transport providers allow any address length between
 * IN_MINADDRLEN and IN_MAXADDRLEN.  The minimum length corresponds to
 * a sockaddr_in without the sin_zero field.  The maximum length is
 * the size of the sockaddr_in structure.
 *
 * in_chkaddrlen returns true if the given length is valid.
 */

#define IN_MINADDRLEN	(sizeof(struct sockaddr_in) - 8)
#define IN_MAXADDRLEN	(sizeof(struct sockaddr_in))
#define in_chkaddrlen(x) ((x) >= IN_MINADDRLEN && (x) <= IN_MAXADDRLEN)

/*
 * Options for use with [gs]etsockopt at the IP level.
 */
#define	IP_OPTIONS	1		/* set/get IP per-packet options */

#ifdef	_KERNEL
extern	struct domain inetdomain;
extern	struct protosw inetsw[];
struct	in_addr in_makeaddr();
u_long	in_netof(), in_lnaof();

struct iocblk_in {
	struct iocblk   iocblk;
	queue_t        *ioc_transport_client;
	queue_t        *ioc_network_client;
	int             ioc_ifflags;
};

/*
 * Useful defines, should be in stream.h 
 */
#define msgbavail(bp) ((bp)->b_datap->db_lim - (bp)->b_wptr)
#define msgblen(bp)	((bp)->b_wptr - (bp)->b_rptr)
#endif	/* _KERNEL */


#endif	/* _NETINET_IN_H */
/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)/usr/src/head/netdb.h.sl 1.1 4.0 12/08/90 55731 AT&T-USL"

/*
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * 		PROPRIETARY NOTICE (Combined)
 * 
 * This source code is unpublished proprietary information
 * constituting, or derived under license from AT&T's UNIX(r) System V.
 * In addition, portions of such source code were derived from Berkeley
 * 4.3 BSD under license from the Regents of the University of
 * California.
 * 
 * 
 * 
 * 		Copyright Notice 
 * 
 * Notice of copyright on this source code product does not indicate 
 * publication.
 * 
 * 	(c) 1986,1987,1988.1989  Sun Microsystems, Inc
 * 	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
 * 	          All rights reserved.
 *  
 */

/*
 * Structures returned by network data base library.
 * All addresses are supplied in host order, and
 * returned in network order (suitable for use in system calls).
 */

#ifndef _NETDB_H
#define _NETDB_H

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

struct hostent	*gethostbyname(), *gethostbyaddr(), *gethostent();
struct netent	*getnetbyname(), *getnetbyaddr(), *getnetent();
struct servent	*getservbyname(), *getservbyport(), *getservent();
struct protoent	*getprotobyname(), *getprotobynumber(), *getprotoent();

/*
 * Error return codes from gethostbyname() and gethostbyaddr()
 * (when using the resolver)
 */

extern  int h_errno;	

#define	HOST_NOT_FOUND	1 /* Authoritive Answer Host not found */
#define	TRY_AGAIN	2 /* Non-Authoritive Host not found, or SERVERFAIL */
#define	NO_RECOVERY	3 /* Non recoverable errors, FORMERR, REFUSED, NOTIMP */
#define	NO_DATA		4 /* Valid name, no data record of requested type */
#define	NO_ADDRESS	NO_DATA		/* no address, look for MX record */


#define	MAXHOSTNAMELEN	256

#endif /*!_NETDB_H*/
/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef	_SYS_UN_H
#define	_SYS_UN_H

#ident	"@(#)/usr/src/uts/i386/sys/un.h.sl 1.1 4.0 12/08/90 20374 AT&T-USL"

/*
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * 		PROPRIETARY NOTICE (Combined)
 * 
 * This source code is unpublished proprietary information
 * constituting, or derived under license from AT&T's UNIX(r) System V.
 * In addition, portions of such source code were derived from Berkeley
 * 4.3 BSD under license from the Regents of the University of
 * California.
 * 
 * 
 * 
 * 		Copyright Notice 
 * 
 * Notice of copyright on this source code product does not indicate 
 * publication.
 * 
 * 	(c) 1986,1987,1988.1989  Sun Microsystems, Inc
 * 	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
 * 	          All rights reserved.
 *  
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

#endif /* _SYS_UN_H */
/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)head:dirent.h	1.6.1.5"

#ifndef _DIRENT_H
#define _DIRENT_H

#if !defined(_POSIX_SOURCE) 
#define MAXNAMLEN	512		/* maximum filename length */
#define DIRBUF		1048		/* buffer size for fs-indep. dirs */
#endif /* !defined(_POSIX_SOURCE) */ 

#ifndef _SYS_TYPES_H
#include <sys/types.h>
#endif

typedef struct
	{
	int		dd_fd;		/* file descriptor */
	int		dd_loc;		/* offset in block */
	int		dd_size;	/* amount of valid data */
	char		*dd_buf;	/* directory block */
	}	DIR;			/* stream data from opendir() */

#if defined(__STDC__)

extern DIR		*opendir( const char * );
extern struct dirent	*readdir( DIR * );
#if !defined(_POSIX_SOURCE) || defined(_XOPEN_SOURCE)
extern long		telldir( DIR * );
extern void		seekdir( DIR *, long );
#endif /* !defined(_POSIX_SOURCE) */ 
extern void		rewinddir( DIR * );
extern int		closedir( DIR * );

#else

extern DIR		*opendir();
extern struct dirent	*readdir();
#if !defined(_POSIX_SOURCE) || defined(_XOPEN_SOURCE)
extern long		telldir();
extern void		seekdir();
#endif /* !defined(_POSIX_SOURCE) */ 
extern void		rewinddir();
extern int		closedir();

#endif

#if !defined(_POSIX_SOURCE) || defined(_XOPEN_SOURCE)
#define rewinddir( dirp )	seekdir( dirp, 0L )
#endif

#ifndef _SYS_DIRENT_H
#include <sys/dirent.h>
#endif

#endif	/* _DIRENT_H */
/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ifndef _SYS_TERMIO_H
#define _SYS_TERMIO_H

#ident	"@(#)head.sys:sys/termio.h	11.12.3.1"

#include <sys/termios.h>


/* all the ioctl codes and flags are now in termios.h */

/*
 * Ioctl control packet
 */
struct termio {
	unsigned short	c_iflag;	/* input modes */
	unsigned short	c_oflag;	/* output modes */
	unsigned short	c_cflag;	/* control modes */
	unsigned short	c_lflag;	/* line discipline modes */
	char	c_line;		/* line discipline */
	unsigned char	c_cc[NCC];	/* control chars */
};

#define	IOCTYPE	0xff00


/*
 * structure of ioctl arg for LDGETT and LDSETT
 */
struct	termcb	{
	char	st_flgs;	/* term flags */
	char	st_termt;	/* term type */
	char	st_crow;	/* gtty only - current row */
	char	st_ccol;	/* gtty only - current col */
	char	st_vrow;	/* variable row */
	char	st_lrow;	/* last row */
};

#ifndef u3b15
#define	SSPEED	7	/* default speed: 300 baud */
#else
#define SSPEED	9	/* default speed: 1200 baud */
#endif

#ifdef u3b15
#define TTYTYPE (TIOC|8)
#endif
#define	TCDSET	(TIOC|32)

/*
 * Terminal types
 */
#define	TERM_NONE	0	/* tty */
#define	TERM_TEC	1	/* TEC Scope */
#define	TERM_V61	2	/* DEC VT61 */
#define	TERM_V10	3	/* DEC VT100 */
#define	TERM_TEX	4	/* Tektronix 4023 */
#define	TERM_D40	5	/* TTY Mod 40/1 */
#define	TERM_H45	6	/* Hewlitt-Packard 45 */
#define	TERM_D42	7	/* TTY Mod 40/2B */

/*
 * Terminal flags
 */
#define TM_NONE		0000	/* use default flags */
#define TM_SNL		0001	/* special newline flag */
#define TM_ANL		0002	/* auto newline on column 80 */
#define TM_LCF		0004	/* last col of last row special */
#define TM_CECHO	0010	/* echo terminal cursor control */
#define TM_CINVIS	0020	/* do not send esc seq to user */
#define TM_SET		0200	/* must be on to set/res flags */

/*
 * structure of ioctl arg for AIOCSETSS (defined is asy.h)
 */

struct	termss {
	char	ss_start; 	/* output start char */
	char 	ss_stop;	/* output stop char */
};


#endif	/* _SYS_TERMIO_H */
/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _PWD_H
#define _PWD_H

#ident	"@(#)head:pwd.h	1.3.1.9"

#include <sys/types.h>

struct passwd {
	char	*pw_name;
	char	*pw_passwd;
	uid_t	pw_uid;
	gid_t	pw_gid;
	char	*pw_age;
	char	*pw_comment;
	char	*pw_gecos;
	char	*pw_dir;
	char	*pw_shell;
};

#if !defined(_POSIX_SOURCE) 
struct comment {
	char	*c_dept;
	char	*c_name;
	char	*c_acct;
	char	*c_bin;
};
#endif /* !defined(_POSIX_SOURCE) */ 

#if defined(__STDC__)

#include <stdio.h>

#if !defined(_POSIX_SOURCE) 
extern struct passwd *getpwent(void);
#endif /* !defined(_POSIX_SOURCE) */ 
extern struct passwd *getpwuid(uid_t);
extern struct passwd *getpwnam(const char *);
#if !defined(_POSIX_SOURCE) 
extern void setpwent(void);
extern void endpwent(void);
extern struct passwd *fgetpwent(FILE *);
extern int putpwent(const struct passwd *, FILE *);
#endif /* !defined(_POSIX_SOURCE) */ 

#else
#if !defined(_POSIX_SOURCE) 
extern struct passwd *getpwent();
#endif /* !defined(_POSIX_SOURCE) */ 
extern struct passwd *getpwuid();
extern struct passwd *getpwnam();
#if !defined(_POSIX_SOURCE) 
extern void setpwent();
extern void endpwent();
extern struct passwd *fgetpwent();
extern int putpwent();
#endif /* !defined(_POSIX_SOURCE) */ 

#endif

#endif /* _PWD_H */
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

#ifdef CLK_TCK
#undef CLK_TCK
#endif

#if !(i486V4)
typedef long clock_t;
#endif

#define CLK_TCK       1	
#define clock() time(NULL)

#ifndef SEEK_SET
#define SEEK_SET	0
#endif

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

/* RcsId: $Id: defines.h,v 1.11 1992/06/19 10:35:15 bart Exp $ */
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

#ifdef TR5
#undef TR5
#define TR5 1
#define UNIX 1
#else
#define TR5 0
#endif

#ifdef i486V4
#undef i486V4
#define i486V4 1
#define UNIX 1
#else
#define i486V4 0
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

#ifdef MSWINDOWS
#undef MSWINDOWS
#define MSWINDOWS 1
#else
#define MSWINDOWS 0
#endif

	/* The I/O Server running under Helios in conjunction with	*/
	/* native networks, primarily a debugging tool.			*/
#ifdef HELIOS
#undef HELIOS
#define HELIOS 1
#else
#define HELIOS 0
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
/**
*** The PC I/O Server comes in two flavours, a DOS one and a Windows one.
*** The latter is controlled by an additional -DMSWINDOWS
**/
#define swapping_needed              0
#define Midi_supported               0
#define Ether_supported              1
#define Rawdisk_supported            1
#define use_own_memory_management    1
#ifdef HUNTROM
#define Romdisk_supported            1
#endif
#define X_supported                  0
#define RS232_supported              1
#define Network_supported            0
#define drives_are_special           1
#define floppies_available           1
#define files_cannot_be_opened_twice 0
#define multi_tasking                0
#define machine_name                 "pc"
#define multiple_windows             1
#define use_ANSI_emulator            1
#define ANSI_prototypes              1
#define Use_linkio                   1
#define internet_supported	     0
#ifdef SMALL
#define gem_supported                0
#define debugger_incorporated        0
#else
#define gem_supported                1
#define debugger_incorporated        1
#endif
#if MSWINDOWS
#undef gem_supported
#define gem_supported		     0
#define graphics		     1
#define print_graphics		     1
#define keyboard_supported           0
#define mouse_supported              0
#define Centronics_supported         0
#define Printer_supported            0
#define interaction_supported        0
#else
#define keyboard_supported           1
#define mouse_supported              1
#define Centronics_supported         1
#define Printer_supported            1
#define interaction_supported        1
#endif
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
#if (SUN3 || SUN4 || SM90 || TR5)
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

#if HELIOS
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
#define Network_supported            1
#define interrupts_use_clock         0
#define drives_are_special           0
#define floppies_available           0
#define use_own_memory_management    0
#define files_cannot_be_opened_twice 0
#define limit_on_files_open          20
#define multi_tasking                0
#define interaction_supported        0
#define machine_name                 "helios"
#define multiple_windows             1
#define use_ANSI_emulator            0
#define debugger_incorporated        1
#define internet_supported	     0
#define ANSI_prototypes              1
#define Use_linkio                   1
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
#ifndef Romdisk_supported
#define Romdisk_supported	     0
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

#ifndef graphics
#define graphics 0
#endif

#ifndef print_graphics
#define print_graphics 0
#endif

#ifndef Gem_Always_Available
#define Gem_Always_Available 0
#endif

#ifndef RS232_Always_Available
#define RS232_Always_Available 0
#endif

#ifndef Centronics_Always_Available
#define Centronics_Always_Available 0
#endif

#ifndef Centronics_readable
#define Centronics_readable 0
#endif

#ifndef Printer_Always_Available
#define Printer_Always_Available 0
#endif

#ifndef Midi_Always_Available
#define Midi_Always_Available 0
#endif

#ifndef Mouse_Always_Available
#define Mouse_Always_Available 0
#endif

#ifndef Keyboard_Always_Available
#define Keyboard_Always_Available 0
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
/* RcsId: $Id: barthdr,v 1.3 1992/06/19 10:35:15 bart Exp $ */
/* Copyright (C) 1987, Perihelion Software Ltd.         		*/

typedef long *ptr;
/* RTE : may not be required */
#if (ST || PC || AMIGA || ARMBSD || MEIKORTE || HELIOS)
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

#if (ST || ARMBSD || MEIKORTE || HELIOS)
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
#if (ST || HELIOS)
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

#if i486V4
#define bcopy(x, y, l) memcpy(y, x, l)
#define bzero(x, l) memset(x, 0, l)
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
/* RcsId: $Id: protocol.h,v 1.8 1992/09/14 13:51:16 bart Exp $ */
/* Copyright (C) 1989, Perihelion Software Ltd.        			*/

/*------------------------------------------------------------------------
--                                                                      --
-- helios.h                                                             --
--                                                                      --
------------------------------------------------------------------------*/

/* standard type definitions */

#if TR5 || i486V4
	/* 64 bit longs I assume */
typedef  int            WORD    ;       /* a machine word, 32 bits      */
typedef  unsigned int   UWORD   ;       /* a machine word, 32 bits      */
typedef  WORD           INT     ;       /* a synonym                    */
typedef  WORD           word    ;       /* another synonym              */
typedef  WORD           Offset  ;       /* and another                  */

#else

#if (!MSWINDOWS)
	/* MSWINDOWS already defines WORD and BYTE... */
typedef  long           WORD    ; 
#endif

typedef  unsigned long  UWORD   ;       /* a machine word, 32 bits      */
typedef  long           INT     ;       /* a synonym                    */
typedef  long           word    ;       /* another synonym              */
typedef  long           Offset  ;       /* and another                  */
#endif

typedef  short int      SHORT   ;       /* a 16 bit word                */
typedef  unsigned short USHORT  ;       /* an unsigned 16 bit value     */

#if (!MSWINDOWS)
typedef  char		BYTE;
#endif
typedef  char           byte    ;       /* a synonym                    */
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

#ifndef TRUE
#define TRUE            1l
#endif
#define true            1l
#ifndef FALSE
#define FALSE           0l
#endif
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
                            word  Input;
                            word  Output;
                            word  Control;
                            word  Local;
#if swapping_needed
                            short Time;
                            short Min;
#else
                            short Min;
                            short Time;
#endif
} Attributes;

typedef word Attribute;

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
#define MsgHdr_Flags_bytealign  0x03    /* used by C40 for non word    */
					/* aligned data */

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
    word        Type;       /* object type code */
    word        Flags;      /* flag word        */
    byte        Access[8];  /* a capability for it */
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
        word            Account;        /* accounting identifier        */
        word            Size;           /* object size in bytes         */
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

typedef struct { word type;        /* always Type_Directory */
                 word size;        /* size in bytes */
                 word used;        /* how many used */
                 word alloc;       /* unit of allocation in bytes */
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

#if (mouse_supported || gem_supported || MSWINDOWS)
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
                 word   Buttons;
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
typedef struct { word   Key;
                 word   What;
} Keyboard_Event;

#define Keys_KeyUp      1L
#define Keys_KeyDown    2L
#endif /* keyboard_supported */

typedef struct { word   junk1;
                 word   junk2;
} Break_Event;   /* this is for ctrl-C */

#if RS232_supported
typedef struct { word    junk1;
                 word    junk2;
} SerialBreak_Event;

typedef struct { word    junk1;
                 word    junk2;
} ModemRing_Event;
#endif

typedef struct IOevent { word Type;
                         word Counter;
                         word Stamp;
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
typedef struct { word port;
                 word *ownedby; /* to keep track of streams */
} event_handler;

/* end of ioevents.h */

/*------------------------------------------------------------------------
--                                                                      --
-- ioconfig.h                                                           --
--                                                                      --
------------------------------------------------------------------------*/
#define CONFIGSPACE	128

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
        word    LinkConf[1];    /* NLinks LinkConf structs      */
        char    namespace[CONFIGSPACE];	/* space for extra links and */
					/* MyName / ParentName */
} Config;

/* Config.Flags end up as Root.Flags */
#define Config_Flags_rootnode	   0x00000001	/* This is rootnode */
#define Config_Flags_special	   0x00000002	/* This is special nuc. */
#define Config_Flags_ROM	   0x00000004	/* This is ROMm'ed nuc. */
#define Config_Flags_xoffed	   0x00000100	/* Set if links xoffed	*/
#define Config_Flags_CacheOff	   0x00000200	/* Dont enable cache */

#define Link_Flags_parent	0x40	/* indicates the link which booted us */
#define Link_Flags_ioproc	0x20	/* indicates an io processor	*/
#define Link_Flags_debug	0x10	/* debugging link for IOdebug	*/
#define Link_Flags_report	0x08	/* report state changes		*/
#define Link_Flags_stopped	0x04	/* link traffic has been stopped*/
#define Link_Flags_HalfDuplex	0x80	/* use half duplex protocol     */

#define Link_Mode_Null		0	/* not connected to anything	*/
#define Link_Mode_Dumb		1	/* link is a dumb device	*/
#define Link_Mode_Intelligent	2	/* part of Helios network	*/

#define Link_State_Null		0	/* not connected to anything	*/
#define Link_State_Booting	1	/* booting remote processor	*/
#define Link_State_Dumb		2	/* dumb device			*/
#define Link_State_Running	3	/* live network link		*/
#define Link_State_Timedout	4	/* doing idle exchange		*/
#define Link_State_Crashed	5	/* remote processor has crashed	*/
#define Link_State_Dead		6	/* remote processor not running	*/

	/* These support "special" links, e.g. the seventh link on a	*/
	/* Hema DSP1. The values should be kept in step with kernel.h	*/
#define Link_Mode_Special	3
#define Link_State_DSP1		0x10
#define Link_State_Hydra	0x11

/* 
/* -- End of ioconfig.h */

/**
*** Link protocol bytes (words on C40)
**/
#define Proto_Write               0
#define Proto_Read                1
#define Proto_Msg                 2
#define Proto_Null                3
#define Proto_Term                4
#define Proto_Reconfigure         5
#define Proto_SecurityCheck       6
#define Proto_Reset               7
#define Proto_Go		  0x0A
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
#define Processor_C40	    5
/* and lots of others... */

#define B_Reset_Processor    0x0001
#define B_Send_Bootstrap     0x0002
#define B_Send_Image         0x0004
#define B_Send_Config        0x0008
#define B_Wait_Sync          0x0010
#define B_Check_Processor    0x0020
#define B_Send_Sync          0x0040
#define B_Send_IdRom         0x0080

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
/* RcsId: $Id: structs.h,v 1.5 1992/06/30 10:30:33 paul Exp $ */
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
**/ /* @@@ Bart, this is going to give us real grief on day! */
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
                 byte    data[1];
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
       word type;
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
                 word       size;
                 word       account;
} ObjNode;

#define DirEntryNode ObjNode

typedef struct { List list;
                 word entries;
} DirHeader;

/**
*** Communication Port_node's are slightly more complicated because I need
*** additional information.
**/

typedef struct { Attributes     attr;
                 Semaphore      lock;
                 word           id;
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
typedef struct Screen { byte          **map;
                        byte          *whole_screen;
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
			word	      handle;
                        int           head, tail;
                        int           XOFF;
#if multi_tasking
                        word          any_data;
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
        word    state;
        word    fildes;
        word    ready;
        word    flags;
        word    connection;
        byte    link_name[32];
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
	word		Type;		/* object type code 		*/
	word 		Flags;		/* flag word			*/
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
	word		Pos;		/* file position		*/
	word		Size;		/* size of transfer		*/
	word		Timeout;	/* transfer completion time	*/
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


/* C40 hardware configuration word flags */

#define HW_NucleusLocalS0	0	/* load nuc. onto Local bus strobe 0 */
					/* Above is the default */
#define HW_NucleusLocalS1	1	/* load nuc. onto Local bus strobe 1 */
#define HW_NucleusGlobalS0	2	/* load nuc. onto Global bus strobe 0 */
#define HW_NucleusGlobalS1	4	/* load nuc. onto Global bus strobe 1 */
#define HW_PseudoIDROM		8	/* send pseudo IDROM */
#define HW_ReplaceIDROM		16	/* send replacement IDROM */
#define HW_CacheOff		32	/* disable cache */


/* This structure defines the contents of the 'C40 TIM-40 IDROM. The IDROM   */
/* characterises the C40 system Helios is running on. If the board has	     */
/* no built-in IDROM a pseudo one is constructed and sent by the I/O Server. */
/* For more information see the TIM-40 specification.                        */

typedef struct IDROM {
	word	SIZE;		/* self inclusive size of this block */

	short	MAN_ID;		/* TIM-40 module manufacturers ID */
	byte	CPU_ID;		/* CPU type (00 = C40) */
	byte	CPU_CLK;	/* CPU cycle time (60ns = 59) */

	short	MODEL_NO;	/* TIM-40 module model number */
	byte	REV_LVL;	/* module revision level */
	byte	RESERVED;	/* currently unused (align to word boundary) */

	word	GBASE0;		/* address base of global bus strobe 0 */
	word	GBASE1;		/* address base of global bus strobe 1 */
	word	LBASE0;		/* address base of local bus strobe 0 */
	word	LBASE1;		/* address base of local bus strobe 1 */

				/* sizes are in words */
	word	GSIZE0;		/* size of memory on global bus strobe 0 */
	word	GSIZE1;		/* size of memory on global bus strobe 1 */
	word	LSIZE0;		/* size of memory on local bus strobe 0 */
	word	LSIZE1;		/* size of memory on local bus strobe a */

	word	FSIZE;		/* size of fast ram pool (inc. on-chip RAM) */

	/* Each of the following bytes contains two nibbles, one for */
	/* strobe 0 and one for strobe 1. The nibbles define how many cycles */
	/* it takes to read a word from that strobes associated memory. */
	byte	WAIT_G;		/* within page on global bus */
	byte	WAIT_L;		/* within page on local bus */
	byte	PWAIT_G;	/* outside page on global bus */
	byte	PWAIT_L;	/* outside page on local bus */

	word	TIMER0_PERIOD;	/* period time for 1ms interval on timer 0 */
	word	TIMER1_PERIOD;	/* period for DRAM refresh timer (optional) */
	short	TIMER0_CTRL;	/* contents set TCLK0 to access RAM not IDROM */
	short	TIMER1_CTRL;	/* sets up timer to refresh DRAM (optional) */

	word	GBCR;		/* global bus control register */
	word	LBCR;		/* local bus control register */

	word	AINIT_SIZE;	/* total size of auto-initialisation data */
} IDROM;

/* end of structs.h */
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
/* RcsId: $Id: fundefs.h,v 1.5 1992/06/19 10:35:15 bart Exp $ */
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
PUBLIC word fn(  Wander,      (List *, WordFnPtr, ...));
PUBLIC word fn(  TstList,     (List *));
PUBLIC word fn(  ListLength,  (List *));
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
PUBLIC word fn( swap, (word));
#else
#define swap(a) (a)
#endif

PUBLIC char *fn( get_config,      (char *));
PUBLIC word  fn(  get_int_config,  (char *));
PUBLIC word  fn(  mystrcmp,        (char *, char *));

#ifndef Daemon_Module

/**
*** A higher level coroutine library using the above linked lists.
**/
PUBLIC Conode *fn( NewCo,     (VoidFnPtr));
PUBLIC void   fn(  TidyColib, (void));
PUBLIC void   fn(  StartCo,   (Conode *));
PUBLIC void   fn(  Suspend,   (void));
PUBLIC void   fn(  Seppuku,   (void));
PUBLIC word   fn(  InitColib, (void));
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
PUBLIC  word fn( Ignore,                    (void));
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
PUBLIC word fn(  convert_name,    (void));
PUBLIC word fn(  flatten,         (char *));
PUBLIC word fn(  mystrcmp,        (char *, char *));
PUBLIC word fn(  get_unix_time,   (void));
PUBLIC void fn(  NewStream,       (word, word, word, VoidFnPtr *));
PUBLIC void fn(  AddAttribute,    (Attributes *, Attribute));
PUBLIC void fn(  RemoveAttribute, (Attributes *, Attribute));
PUBLIC void fn(  SetInputSpeed,   (Attributes *, word));
PUBLIC void fn(  SetOutputSpeed,  (Attributes *, word));
PUBLIC void fn(  InitAttributes,  (Attributes *));
PUBLIC void fn(  CopyAttributes,  (Attributes *, Attributes *));
PUBLIC word fn(  IsAnAttribute,   (Attributes *, Attribute));
PUBLIC word fn(  GetInputSpeed,   (Attributes *));
PUBLIC word fn(  GetOutputSpeed,  (Attributes *));
PUBLIC word fn(  Request_Stat,    (void));
PUBLIC void fn(  Request_Return,  (word, word, word));
PUBLIC void fn(  goto_sleep,      (word));
PUBLIC word fn(  FormOpenReply,   (word, word, word, word));
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

PUBLIC word fn( create_a_window, (char *));
PUBLIC void fn( window_size,     (word, word *, word *));
PUBLIC void fn( close_window,    (word));
PUBLIC void fn( send_to_window,  (char *, Window *));
#if use_ANSI_emulator

PUBLIC  void fn( ANSI_out, (char *, Window *));
#define window_output ANSI_out

#else /* use_ANSI_emulator */

#define window_output send_to_window

#endif /* use_ANSI_emulator     */

#define output(a) window_output(a, &Server_window)

#endif /* multiple_windows */

PUBLIC void fn( ServerDebug, (char *, ...));
PUBLIC void fn( outputch,    (int, Window *));
#if use_ANSI_emulator
PUBLIC word fn( Init_Ansi,   (Window *, word, word));
PUBLIC void fn( Tidy_Ansi,   (Screen *));
PUBLIC void fn( Resize_Ansi, (Window *, word, word, word));
#endif

#if MSWINDOWS
	/* Under windows, printf calls should be replaced by an	*/
	/* error box mechanism.					*/
#define printf err_out
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

PUBLIC  word fn( File_InitStream, (Conode *));
PUBLIC  word fn( File_TidyStream, (Conode *));
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

PUBLIC  word fn( Dir_InitStream,  (Conode *));
PUBLIC  word fn( Dir_TidyStream,  (Conode *));
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

PUBLIC  word fn( Console_InitStream,   (Conode *));
PUBLIC  word fn( Console_TidyStream,   (Conode *));
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
#if (UNIX || MAC || HELIOS)
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

#if (MSWINDOWS)
#if graphics
#include "windows\graph.def"
#endif
#if print_graphics
#include "windows\grfprn.def"
#endif
#include "windows\msevent.def"
#endif

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

PUBLIC  word fn( Gem_InitStream,     (Conode *));
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
#if (Ports_used || Rawdisk_supported)
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

PUBLIC  word fn( RS232_initlist,       (List *, ComsPort **));
PUBLIC  void fn( RS232_check_events,   (void));
PUBLIC  void fn( RS232_disable_events, (ComsPort *));
PUBLIC  void fn( RS232_enable_events,  (ComsPort *, word, word));
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

PUBLIC  word fn( Centronics_initlist,   (List *, ComsPort **));
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

PUBLIC  word fn( Printer_initlist,      (List *, ComsPort **));
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

PUBLIC  word fn( Midi_initlist,         (List *, ComsPort **));
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

PUBLIC  word fn( Mouse_InitStream,      (Conode *));
PUBLIC  word fn( Mouse_TidyStream,      (Conode *));
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
PUBLIC  word fn( Keyboard_InitStream,     (Conode *));
PUBLIC  word fn( Keyboard_TidyStream,     (Conode *));
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

PUBLIC  word fn( X_InitStream,   (Conode *));
PUBLIC  word fn( X_TidyStream,   (Conode *));
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
#define Internet_Testfun              Nullfn
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

PUBLIC  word fn( Internet_InitStream,   (Conode *));
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

#if !(PC || MAC || HELIOS)     /* The PC's header files declare malloc */
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
PUBLIC word fn( Multiwait, (void));
/**
*** This routine is called by the Server whenever it is waiting on some
*** form of I/O. The first argument is one of the constants defined in
*** structs.h, specifying the particular form of I/O. The second argument
*** is a pointer to an integer. Whenever the MultiWait is satisfied for the
*** event, that integer should be zapped to the value CoReady. Additional
*** arguments are available if required, e.g. to specify a particular
*** file descriptor.
**/
PUBLIC void fn( AddMultiwait, (word, word *,...));
/**
*** ClearMultiwait() is just the inverse of AddMultiwait()
**/
PUBLIC void fn( ClearMultiwait, (word, word *, ...));
#endif

/**
*** Bits that had been left out before.
**/
PUBLIC  void fn( initialise_devices,      (void));
PUBLIC  void fn( initialise_files,        (void));
PUBLIC  void fn( restart_devices,         (void));
PUBLIC  void fn( restore_devices,         (void));
PUBLIC  void fn( poll_the_devices,        (void));
PUBLIC  int  fn( read_char_from_keyboard, (word));
PUBLIC  int  fn( init_boot,               (void));
PUBLIC  void fn( boot_processor,          (int));
PUBLIC  void fn( tidy_boot,               (void));
PUBLIC  void fn( init_main_message,       (void));
PUBLIC  void fn( free_main_message,       (void));
PUBLIC  word fn( GetMsg,                  (MCB *));
PUBLIC  word fn( PutMsg,                  (MCB *));
#if debugger_incorporated
PUBLIC  void fn( init_debug,              (void));
PUBLIC  int  fn( debug,                   (void));
PUBLIC  void fn( tidy_debug,              (void));
#endif

#if floppies_available
PUBLIC  word fn( format_floppy, (char *, word, word, word, char *));
#endif

PUBLIC  int  fn( loadimage,               (void));
PUBLIC  void fn( resetlnk,                (void));
PUBLIC  void fn( xpreset,                 (void));
PUBLIC  void fn( xpanalyse,               (void));
PUBLIC  word fn( xpwrbyte,                (word));
PUBLIC  word fn( xpwrrdy,                 (void));
PUBLIC  word fn( xprdrdy,                 (void));
PUBLIC  word fn( xpwrword,                (word));
PUBLIC  word fn( xpwrint,                 (word));
PUBLIC  word fn( xpwrdata,                (byte *, word));
PUBLIC  word fn( xprddata,                (byte *, word));
PUBLIC  word fn( dbwrword,                (word, word));
PUBLIC  word fn( dbwrint,                 (word, word));
PUBLIC  word fn( xprdint,                 (void));
PUBLIC  word fn( xprdword,                (void));
PUBLIC  word fn( xprdbyte,                (void));
PUBLIC  word fn( dbrdword,                (word));
PUBLIC  word fn( dbrdint,                 (word));

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
/* RcsId: $Id: server.h,v 1.8 1992/06/30 10:29:47 paul Exp $ */
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
***
***  target_processor is used to indicate what type of processor the server
***  is communicating with.
**/

#ifdef Server_Module
word        target_processor;
List        *WaitingCo, *PollingCo, *SelectCo;
Node        *Heliosnode;
MCB         *mcb;
word        CoCount=1L;             /* counter for coroutine id numbering */
word        time_unit;
time_t      Startup_Time;
clock_t     Now, initial_stamp;
char        IOname[IOCDataMax];
char        *Heliosdir;
word        maxdata;
char        err_buff[256];
char        system_image[80];
jmp_buf     exit_jmpbuf;
word        Special_Reboot=false, Special_Exit=false, Special_Status=false;
int         DebugMode = 0;
int 	    EnableThatLink = 0;       /* see module tload.c */
UBYTE       *bootstrap = NULL;        /* see module tload.c */
word        bootsize;
byte        misc_buffer1[512], misc_buffer2[512];
int         Device_count;
Window      Server_window;
int         Server_windows_nopop = 0;
#if multiple_windows
DirHeader   Window_List;
int         real_windows = 0;
#endif
word        Server_errno;
int         number_of_links = 1;
int         current_link    = 0;
PRIVATE     Trans_link root_link;
Trans_link  *link_table = &root_link;
int         Multi_nowait = 0;
int         Default_BootLink = 0;
/* C40HalfDuplex flags use of a special protocol to stop HalfDuplex links */
/* blocking. Can be set to FALSE by host.con c40_disable_halfduplex */
word       C40HalfDuplex = TRUE;

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

#if (MSWINDOWS)
#if (graphics)
#include "windows\windev.inc"
#endif
#if print_graphics
#include "windows\grfprn.ser"
#endif
#include "windows\msedev.inc"
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


#if (Ports_used || Rawdisk_supported)
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

#if (MSWINDOWS)
#if (graphics)
#include "windows\winstrm.inc"
#endif
#if print_graphics
#include "windows\grfprn.str"
#endif
#include "windows\msestrm.inc"
#endif

int Server_Mode = Mode_Normal;

#else       /* declarations for the other modules */

extern word         target_processor;
extern int          Server_Mode;
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
extern byte         misc_buffer1[], misc_buffer2[];
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
extern int          Default_BootLink;
extern word         C40HalfDuplex;

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
#if (Ports_used || Rawdisk_supported)
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
#if (MSWINDOWS)
#if (graphics)
extern VoidFnPtr Graph_Handlers[];
#endif
#if print_graphics
extern VoidFnPtr GrfPrn_Handlers[];
#endif
extern VoidFnPtr MsEvent_Handlers[];
extern VoidFnPtr MsMouse_Handlers[];
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
/* RcsId: $Id: debugopt.h,v 1.2 1992/06/19 10:35:15 bart Exp $ */
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
word debugflags;
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
extern word debugflags;
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
