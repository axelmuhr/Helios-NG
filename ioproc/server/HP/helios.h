/* @(#) $Revision: 70.11 $ */
#ifndef _STDIO_INCLUDED
#define _STDIO_INCLUDED

#ifndef _SYS_STDSYMS_INCLUDED
#  include <sys/stdsyms.h>
#endif /* _SYS_STDSYMS_INCLUDED */

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _INCLUDE__STDC__

#  define _NFILE	60
#  define BUFSIZ	1024
#  define _DBUFSIZ	8192

/* buffer size for multi-character output to unbuffered files */
#  define _SBFSIZ	8

   typedef struct {
	int		 __cnt;
	unsigned char	*__ptr;
	unsigned char	*__base;
	unsigned short	 __flag;
	unsigned char 	 __fileL;		/* low byte of file desc */
	unsigned char 	 __fileH;		/* high byte of file desc */
   } FILE;

   typedef struct {
	int		 __cnt;
	unsigned char	*__ptr;
	unsigned char	*__base;
	unsigned short	 __flag;
	unsigned char 	 __fileL;		/* low byte of file desc */
	unsigned char 	 __fileH;		/* high byte of file desc */
	unsigned char	*__bufendp;	/* end of buffer */
	unsigned char	 __smbuf[_SBFSIZ]; /* small buffer */
   } _FILEX;

/*
 * _IOLBF means that a file's output will be buffered line by line
 * In addition to being flags, _IONBF, _IOLBF and _IOFBF are possible
 * vales for "type" in setvbuf
 */
#  define _IOFBF		0000000
#  define _IOREAD		0000001
#  define _IOWRT		0000002
#  define _IONBF		0000004
#  define _IOMYBUF		0000010
#  define _IOEOF		0000020
#  define _IOERR		0000040
#  define _IOLBF		0000200
#  define _IORW			0000400
#  define _IOEXT		0001000
#  define _IODUMMY		0002000 /* dummy file for doscan, doprnt */

#  ifndef NULL
#    define	NULL	0
#  endif /* NULL */

#  ifndef _SIZE_T
#    define _SIZE_T
     typedef unsigned int size_t;
#  endif /* _SIZE_T */

   typedef long int fpos_t;

#  ifdef __hp9000s300
     typedef char *__va_list;
#  endif /* __hp9000s300 */

#  ifdef __hp9000s800
     typedef double *__va_list;
#  endif /* __hp9000s800 */


#  ifndef SEEK_SET
#    define SEEK_SET 0
#    define SEEK_CUR 1
#    define SEEK_END 2
#  endif /* SEEK_SET */

#  define _P_tmpdir "/usr/tmp/"

#  define L_tmpnam	(sizeof(_P_tmpdir) + 15)

#  ifndef TMP_MAX
#    define TMP_MAX 17576
#  endif /* TMP_MAX */

#  define FILENAME_MAX 	14
#  define FOPEN_MAX 	_NFILE

#  ifndef EOF
#    define	EOF	(-1)
#  endif /* EOF */
   
#  define	stdin	(&__iob[0])
#  define	stdout	(&__iob[1])
#  define	stderr	(&__iob[2])

   extern FILE __iob[];

#  if defined(__STDC__) || defined(__cplusplus)
     extern int remove(const char *);
     extern int rename(const char *, const char *);
     extern FILE *tmpfile(void);
     extern char *tmpnam(char *);
     extern int fclose(FILE *);
     extern int fflush(FILE *);
     extern FILE *fopen(const char *, const char *);
     extern FILE *freopen(const char *, const char *, FILE *);
     extern void setbuf(FILE *, char *);
     extern int setvbuf(FILE *, char *, int, size_t);
     extern int fprintf(FILE *, const char *, ...);
     extern int fscanf(FILE *, const char *,...);
     extern int printf(const char *,...);
     extern int scanf(const char *,...);
     extern int sprintf(char *, const char *,...);
     extern int sscanf(const char *, const char *,...);
     extern int vprintf(const char *, __va_list);
     extern int vfprintf(FILE *, const char *, __va_list);
     extern int vsprintf(char *, const char *, __va_list);
     extern int fgetc(FILE *);
     extern char *fgets(char *, int, FILE *);
     extern int fputc(int, FILE *);
     extern int fputs(const char *, FILE *);
     extern int getc(FILE *);
     extern int getchar(void);
     extern char *gets(char *);
     extern int putc(int, FILE *);
     extern int putchar(int);
     extern int puts(const char *);
     extern int ungetc(int, FILE *);
     extern int fgetpos(FILE *, fpos_t *);
     extern int fseek(FILE *, long int, int);
     extern int fsetpos(FILE *, const fpos_t *);
     extern long int ftell(FILE *);
     extern void rewind(FILE *);
     extern void clearerr(FILE *);
     extern int feof(FILE *);
     extern int ferror(FILE *);
     extern void perror(const char *);
#  else /* not __STDC__ || __cplusplus */
     extern int remove();
     extern int rename();
     extern FILE *tmpfile();
     extern char *tmpnam();
     extern int fclose();
     extern int fflush();
     extern FILE *fopen();
     extern FILE *freopen();
     extern void setbuf();
     extern int setvbuf();
     extern int fprintf();
     extern int fscanf();
     extern int printf();
     extern int scanf();
     extern int sprintf();
     extern int sscanf();
     extern int vprintf();
     extern int vfprintf();
     extern int vsprintf();
     extern int fgetc();
     extern char *fgets();
     extern int fputc();
     extern int fputs();
     extern int getc();
     extern int getchar();
     extern char *gets();
     extern int putc();
     extern int putchar();
     extern int puts();
     extern int ungetc();
     extern int fgetpos();
     extern int fseek();
     extern int fsetpos();
     extern long int ftell();
     extern void rewind();
     extern void clearerr();
     extern int feof();
     extern int ferror();
     extern void perror();
#  endif /* __STDC__ || __cplusplus */

#  ifdef _CLASSIC_ANSI_TYPES
     extern int fread();
     extern int fwrite();
#  else
#    if defined(__STDC__) || defined(__cplusplus)
       extern size_t fread(void *, size_t, size_t, FILE *);
       extern size_t fwrite(const void *, size_t, size_t, FILE *);
#    else /* not __STDC__ || __cplusplus */
       extern size_t fread();
       extern size_t fwrite();
#    endif /* else not __STDC__ || __cplusplus */
#  endif

/*
 * WARNING:  The following function declarations are provided solely
 *  for use in the macros found in this header file.  These are HP-UX
 *  specific routines used internally and direct use of these routines 
 *  is not supported.  These routines may be removed or significantly
 *  changed in future releases of HP-UX.
 */
#  if defined(__STDC__) || defined(__cplusplus)
     extern int __flsbuf(unsigned char, FILE *);
     extern int __filbuf(FILE *);
#  else /* not __STDC__ || __cplusplus */
     extern int __flsbuf();
     extern int __filbuf();
#  endif /* __STDC__ || __cplusplus */
  
#  ifndef __lint
#    define clearerr(__p)	((void) ((__p)->__flag &= ~(_IOERR | _IOEOF)))
#    define feof(__p)	((__p)->__flag & _IOEOF)
#    define ferror(__p)	((__p)->__flag & _IOERR)
#    define getc(__p)     (--(__p)->__cnt < 0 ? __filbuf(__p) : \
			(int) *(__p)->__ptr++)

#    define getchar()	getc(stdin)

#    define putc(__c, __p)	(--(__p)->__cnt < 0 ? \
			__flsbuf((unsigned char) (__c), (__p)) : \
			(int) (*(__p)->__ptr++ = (unsigned char) (__c)))

#    define putchar(__c)	putc((__c), stdout)
#  endif /* not __lint */
#endif /* _INCLUDE__STDC__ */

#ifdef _INCLUDE_POSIX_SOURCE
#  define L_ctermid	9
#  define L_cuserid	9

#  if defined(__STDC__) || defined(__cplusplus)
     extern char *ctermid(char *);
     extern int fileno(FILE *);
     extern FILE *fdopen(int, const char *);
#  else /* not __STDC */
     extern char *ctermid();
     extern int fileno();
     extern FILE *fdopen();
#  endif /* __STDC */
#endif /* _INCLUDE_POSIX_SOURCE */

#ifdef _INCLUDE_XOPEN_SOURCE
#  ifndef _AES_SOURCE

#    ifdef __hp9000s300
#      ifndef _VA_LIST
#        define _VA_LIST
         typedef char *va_list;
#      endif  /* _VA_LIST */
#    endif /* __hp9000s300 */

#    ifdef __hp9000s800
#      ifndef _VA_LIST
#        define _VA_LIST
         typedef double *va_list;
#      endif  /* _VA_LIST */
#    endif /* __hp9000s800 */

     extern char *optarg;
     extern int opterr;
     extern int optind;
     extern int optopt;

#    ifdef _PROTOTYPES
       extern int getopt(int, char * const [], const char *);
       extern char *cuserid(char *);
#    else
       extern int getopt();
       extern char *cuserid();
#    endif /* _PROTOTYPES */
#  endif /* not _AES_SOURCE */



#  define P_tmpdir 	_P_tmpdir

#  if defined(__STDC__) || defined(__cplusplus)
     extern int getw(FILE *);
     extern int putw(int, FILE *);
     extern int pclose(FILE *);
     extern FILE *popen(const char *, const char *);
     extern char *tempnam(const char *, const char *);
#  else /* not __STDC__ || __cplusplus */
     extern int getw();
     extern int putw();
     extern int pclose();
     extern FILE *popen();
     extern char *tempnam();
#  endif /* else not __STDC__ || __cplusplus */

#endif /* _INCLUDE_XOPEN_SOURCE */

#ifdef _INCLUDE_HPUX_SOURCE

#  define _iob __iob

#  if defined(__STDC__) || defined(__cplusplus)
     extern int nl_fprintf(FILE *, const char * ,...);
     extern int nl_fscanf(FILE *, const char * ,...);
     extern int nl_printf(const char * ,...);
     extern int nl_scanf(const char * ,...);
     extern int nl_sprintf(char *, const char * ,...);
     extern int nl_sscanf(const char *, const char * ,...);
     extern int vscanf(const char *, __va_list);
     extern int vfscanf(FILE *, const char *, __va_list);
     extern int vsscanf(char *, const char *, __va_list);

#  else /* not __STDC__ || __cplusplus */
     extern int vscanf();
     extern int vfscanf();
     extern int vsscanf();
#  endif /* __STDC__ || __cplusplus */

   extern unsigned char *__bufendtab[];

#  define _cnt		__cnt
#  define _ptr		__ptr
#  define _base		__base
#  define _flag		__flag

#  define _bufend(__p) \
    (*(((__p)->__flag & _IOEXT)  ? &(((_FILEX *)(__p))->__bufendp)	\
				: &(__bufendtab[(__p) - __iob])))

#  define _bufsiz(__p)	(_bufend(__p) - (__p)->__base)
#endif /* _INCLUDE_HPUX_SOURCE */

#ifdef __cplusplus
}
#endif

#endif /* _STDIO_INCLUDED */
/* $Header: types.h,v 1.33.61.8 92/06/17 19:28:51 rsh Exp $ */

#ifndef _SYS_TYPES_INCLUDED
#define _SYS_TYPES_INCLUDED

#ifdef _KERNEL_BUILD
#include "../h/stdsyms.h"
#else  /* ! _KERNEL_BUILD */
#include <sys/stdsyms.h>
#endif /* _KERNEL_BUILD */

#ifdef _INCLUDE_POSIX_SOURCE
#  ifndef _DEV_T
#    define _DEV_T
     typedef long dev_t;		/* For device numbers */
#  endif /* _DEV_T */

#  ifndef _INO_T
#    define _INO_T
     typedef unsigned long ino_t;	/* For file serial numbers */
#  endif /* _INO_T */

#  ifndef _MODE_T
#    define _MODE_T
     typedef unsigned short mode_t; 	/* For file types and modes */
#  endif /* _MODE_T */

#  ifndef _NLINK_T
#    define _NLINK_T
     typedef short nlink_t;		/* For link counts */
#  endif /* _NLINK_T */

#  ifndef _OFF_T
#    define _OFF_T
     typedef long off_t;		/* For file offsets and sizes */
#  endif /* _OFF_T */

#  ifndef _PID_T
#    define _PID_T
     typedef long pid_t;		/* For process and session IDs */
#  endif /* _PID_T */

#  ifndef _GID_T
#    define _GID_T
     typedef long gid_t;		/* For group IDs */
#  endif /* _GID_T */

#  ifndef _UID_T
#    define _UID_T
     typedef long uid_t;		/* For user IDs */
#  endif /* _UID_T */

#  ifndef _TIME_T
#    define _TIME_T
     typedef long time_t;		/* For times in seconds */
#  endif /* _TIME_T */

#  ifndef _SIZE_T
#    define _SIZE_T
     typedef unsigned int size_t;	/* Type returned by sizeof() */
#  endif /* _SIZE_T */

#  ifndef _SSIZE_T
#     define _SSIZE_T
      typedef int ssize_t;		/* Signed version of size_t */
#  endif /* _SSIZE_T */

#  ifndef _SITE_T
#    define _SITE_T
     typedef unsigned short __site_t;	/* see stat.h */
#  endif /* _SITE_T */

#  ifndef _CNODE_T
#    define _CNODE_T
     typedef unsigned short __cnode_t;	/* see stat.h */
#  endif /* _CNODE_T */
#endif /* _INCLUDE_POSIX_SOURCE */

#ifdef _INCLUDE_XOPEN_SOURCE
#  ifndef _CLOCK_T
#    define _CLOCK_T
     typedef unsigned long clock_t;	/* For clock ticks */
#  endif /* _CLOCK_T */

#  ifndef _KEY_T
#    define _KEY_T
      typedef long key_t;		/* For interprocess communication ID */
#  endif /* _KEY_T */

   typedef unsigned short __ushort;	/* Try to avoid using this */

   typedef long	__daddr_t;		/* For disk block addresses */
   typedef char *__caddr_t;		/* For character addresses */
   typedef long __swblk_t;

#endif /* _INCLUDE_XOPEN_SOURCE */


#ifdef _INCLUDE_AES_SOURCE
#  ifndef _CADDR_T
#    define _CADDR_T
     typedef __caddr_t		caddr_t;   /* also in ptrace.h */
#  endif /* _CADDR_T */
#endif /* _INCLUDE_AES_SOURCE */

#ifdef _INCLUDE_HPUX_SOURCE
   typedef unsigned char	u_char;	   /* Try to avoid using these */
   typedef unsigned short	u_short;   /* Try to avoid using these */
   typedef unsigned int		u_int;     /* Try to avoid using these */
   typedef unsigned long	u_long;    /* Try to avoid using these */
   typedef unsigned int		uint;	   /* Try to avoid using these */
   typedef unsigned short	ushort;	   /* Try to avoid using these */
   typedef unsigned char  ubit8;
   typedef unsigned short ubit16;
   typedef unsigned long  ubit32;
   typedef char           sbit8;
   typedef short          sbit16;
   typedef long           sbit32;

   typedef __swblk_t		swblk_t;
   typedef __daddr_t		daddr_t;
   typedef __site_t		site_t;
   typedef __cnode_t		cnode_t;

   typedef long			paddr_t;
   typedef short		cnt_t;
   typedef unsigned int		space_t;
   typedef unsigned int    	prot_t;
   typedef unsigned long        cdno_t;
   typedef unsigned short	use_t;

   typedef struct _physadr { int r[1]; } *physadr;
   typedef struct _quad { long val[2]; } quad;

   typedef char spu_t;

/*
 * Only include sysmacros.h when using kernel data structures.  If this
 * file is included for normal user applications, namespace pollution 
 * will occur.
 */
#ifdef _KERNEL
#  ifdef __hp9000s300
#    ifdef _KERNEL_BUILD
#    include "../h/sysmacros.h"
#    else /* ! _KERNEL_BUILD */
#    include <sys/sysmacros.h>
#    endif /* _KERNEL_BUILD */
#  endif /* __hp9000s300 */
#endif /* _KERNEL */

#ifdef __hp9000s300
#    define NREGS_S 15 /* number of regs saved in save (sfc,dfc,a/d2-7,pc) */
#endif /* __hp9000s300 */

#  define MAXSUSE	65535	/* maximum share count on swap device */

#  ifdef __hp9000s300
     typedef struct label_t {
	   int	val[NREGS_S]; 
     } label_t;
#  endif /* __hp9000s300 */

#  ifdef __hp9000s800
     typedef short cpu_t;
     typedef struct label_t {
	int	lbl_rp;
       	int	lbl_sp;
       	int	lbl_s[17];
       	int	lbl_ss[1];
	double	lbl_sf[4];
     } label_t;
#  endif /* __hp9000s800 */

#  ifndef _KERNEL
   typedef char *dm_message;
#  endif /* not _KERNEL */

#  ifndef _AID_T
#     define _AID_T
      typedef long	aid_t;
#  endif /* _AID_T */


/* These probably should be moved to some other header */

#  define UID_NO_CHANGE  ((uid_t) -1)  /* for chown(2) and setresuid(2) */
#  define GID_NO_CHANGE  ((gid_t) -1)  /* for chown(2) and setresgid(2) */

   typedef pid_t		sid_t;	   /* For session IDs */

#  define PGID_NOT_SET   ((pid_t) -1)  /* for no pgrp */
#  define SID_NOT_SET	 ((sid_t) -1)  /* for no session */


/* Miscellaneous historical label_t values */


#  ifdef __vax
     typedef struct  label_t {
             int     val[14];
     } label_t;
#  endif


/* Types, macros, etc. for select() */

#  ifndef MAXFUPLIM
/*
 * MAXFUPLIM is the absolute limit of open files per process.  No process, 
 * even super-user processes may increase u.u_maxof beyond MAXFUPLIM.
 * MAXFUPLIM means maximum files upper limit.
 * Important Note:  This definition should actually go into h/param.h, but 
 * since it is needed by the select() macros which follow, it had to go here.
 * I did not put it in both files since h/param.h includes this file and that
 * would be error prone anyway.  
 */
#    define MAXFUPLIM       2048

/*
 * These macros are used for select().  select() uses bit masks of file
 * descriptors in longs.  These macros manipulate such bit fields (the
 * file sysrem macros uses chars).  FD_SETSIZE may be defined by the user,
 * but must be >= u.u_highestfd + 1.  Since we know the absolute limit on 
 * number of per process open files is 2048, we need to define FD_SETSIZE 
 * to be large enough to accomodate this many file descriptors.  Unless the 
 * user has this many files opened, he should redefine FD_SETSIZE to a 
 * smaller number.
 */
#    ifndef _KERNEL
#      ifndef FD_SETSIZE
#        define FD_SETSIZE MAXFUPLIM
#      endif
#    else /* not _KERNEL */
#      define FD_SETSIZE MAXFUPLIM
#    endif /* _KERNEL */

     typedef long fd_mask;

#    define NFDBITS (sizeof(fd_mask) * 8)     /* 8 bits per byte */

#    ifndef howmany
#      define howmany(x,y)  (((x)+((y)-1))/(y))
#    endif

     typedef struct fd_set {
       fd_mask fds_bits[howmany(FD_SETSIZE, NFDBITS)];
     } fd_set;

#    define FD_SET(n,p)  ((p)->fds_bits[(n)/NFDBITS] |= (1 << ((n) % NFDBITS)))
#    define FD_CLR(n,p) ((p)->fds_bits[(n)/NFDBITS] &= ~(1 << ((n) % NFDBITS)))
#    define FD_ISSET(n,p) ((p)->fds_bits[(n)/NFDBITS] & (1 << ((n) % NFDBITS)))

#    ifndef _KERNEL
#        define FD_ZERO(p)     memset((char *)(p), (char) 0, sizeof(*(p)))
#    endif /* not _KERNEL */
#   endif /* not MAXFUPLIM */

#endif /* _INCLUDE_HPUX_SOURCE */

#ifdef _UNSUPPORTED

	/* 
	 * NOTE: The following header file contains information specific
	 * to the internals of the HP-UX implementation. The contents of 
	 * this header file are subject to change without notice. Such
	 * changes may affect source code, object code, or binary
	 * compatibility between releases of HP-UX. Code which uses 
	 * the symbols contained within this header file is inherently
	 * non-portable (even between HP-UX implementations).
	*/
#ifdef _KERNEL_BUILD
#	include "../h/_types.h"
#else  /* ! _KERNEL_BUILD */
#	include <.unsupp/sys/_types.h>
#endif /* _KERNEL_BUILD */
#endif /* _UNSUPPORTED */

#endif /* _SYS_TYPES_INCLUDED */
/* @(#) $Revision: 70.12 $ */
#ifndef _CTYPE_INCLUDED
#define _CTYPE_INCLUDED

#ifndef _SYS_STDSYMS_INCLUDED
#  include <sys/stdsyms.h>
#endif /* _SYS_STDSYMS_INCLUDED */

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _INCLUDE__STDC__
#  if defined(__STDC__) || defined(__cplusplus)
     extern int isalnum(int);
     extern int isalpha(int);
     extern int iscntrl(int);
     extern int isdigit(int);
     extern int isgraph(int);
     extern int islower(int);
     extern int isprint(int);
     extern int ispunct(int);
     extern int isspace(int);
     extern int isupper(int);
     extern int isxdigit(int);
     extern int tolower(int);
     extern int toupper(int);
#  else /* not __STDC__ || __cplusplus */
     extern int isalnum();
     extern int isalpha();
     extern int iscntrl();
     extern int isdigit();
     extern int isgraph();
     extern int islower();
     extern int isprint();
     extern int ispunct();
     extern int isspace();
     extern int isupper();
     extern int isxdigit();
     extern int tolower();
     extern int toupper();
#  endif /* __STDC__ || __cplusplus */

#  define _U	01
#  define _L	02
#  define _N	04
#  define _S	010 
#  define _P	020
#  define _C	040
#  define _B	0100 
#  define _X	0200
/* Posix.2 */
#  define _A    01 /* Alpha */
#  define _G    02 /* Graph */
#  define _PR   04 /* Print */


#  ifndef __lint
     extern unsigned char 		*__ctype;
     extern unsigned char               *__ctype2;
     extern int                         __alnum;

     extern unsigned char		*__upshift;
     extern unsigned char		*__downshift;

#    define isalpha(__c)	(__ctype2[__c]&(_A)) /* changed from _U|_L */
#    define isupper(__c)	(__ctype[__c]&_U)
#    define islower(__c)	(__ctype[__c]&_L)
#    define isdigit(__c)	(__ctype[__c]&_N)
#    define isxdigit(__c)	(__ctype[__c]&_X)
#    define isalnum(__c)	(__alnum = __c,((__ctype2[__alnum]&_A)|(__ctype[__alnum]&_N))) /* was _U|_L|_N */
#    define isspace(__c)	(__ctype[__c]&_S)
#    define ispunct(__c)	(__ctype[__c]&_P)
#    define isprint(__c)	(__ctype2[__c]&(_PR)) /* was _P|_U|_L|_N|_B */
#    define isgraph(__c)	(__ctype2[__c]&(_G)) /* was _P|_U|_L|_N */
#    define iscntrl(__c)	(__ctype[__c]&_C)
#  endif /* __lint */

#endif /* _INCLUDE__STDC__ */


#ifdef _INCLUDE_XOPEN_SOURCE

#  ifdef _PROTOTYPES
     extern int isascii(int);
     extern int toascii(int);
#  else /* NOT _PROTOTYPES */
     extern int isascii();
     extern int toascii();
#  endif /* _PROTOTYPES */


#  ifndef __lint
#    define isascii(__c)		((unsigned) (__c)<=0177)
#    define toascii(__c)		((__c)&0177)
#    define _toupper(__c)		((__upshift)[__c]&0377)
#    define _tolower(__c)		((__downshift)[__c]&0377)
#  endif /* __lint */

#endif /* _INCLUDE_XOPEN_SOURCE */

#ifdef __cplusplus
}
#endif

#endif /* _CTYPE_INCLUDED */
/* @(#) $Revision: 66.5 $ */
#ifndef _SETJMP_INCLUDED /* allow multiple inclusions */
#define _SETJMP_INCLUDED

#ifndef _SYS_STDSYMS_INCLUDED
#  include <sys/stdsyms.h>
#endif /* _SYS_STDSYMS_INCLUDED */

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _INCLUDE__STDC__
#  ifdef __hp9000s300
#    define _JBLEN 100
     typedef int jmp_buf[_JBLEN];
#  endif /* __hp9000s300 */
#  ifdef __hp9000s800
#    define _JBLEN 50
     typedef double jmp_buf[_JBLEN/2];
#  endif /* __hp9000s800 */


#  if defined(__STDC__) || defined(__cplusplus)
     extern int setjmp(jmp_buf);
     extern void longjmp(jmp_buf, int);
#  else /* not __STDC__ || __cplusplus */
     extern int setjmp();
     extern void longjmp();
#  endif /* __STDC__ || __cplusplus */
#endif /* _INCLUDE__STDC__ */

#ifdef _INCLUDE_POSIX_SOURCE
#  ifdef __hp9000s800
     typedef double sigjmp_buf[_JBLEN/2];
#  endif /* __hp9000s800 */
#  ifdef __hp9000s300
     typedef int sigjmp_buf[_JBLEN];
#  endif /* __hp9000s300 */

#    if defined(__STDC__) || defined(__cplusplus)
       extern int sigsetjmp(sigjmp_buf, int);
       extern void siglongjmp(sigjmp_buf, int);
#    else /* not __STDC__ || __cplusplus */
       extern int sigsetjmp();
       extern void siglongjmp();
#    endif /* __STDC__ || __cplusplus*/
#endif /* _INCLUDE_POSIX_SOURCE */

#ifdef _INCLUDE_HPUX_SOURCE
#  if defined(__STDC__) || defined(__cplusplus)
     extern int _setjmp(jmp_buf);
     extern void _longjmp(jmp_buf, int);
#  else /* not __STDC__ || __cplusplus */
     extern int _setjmp();
     extern void _longjmp();
#  endif /* __STDC__ || __cplusplus */
#endif /* _INCLUDE_HPUX_SOURCE */

#ifdef __cplusplus
}
#endif

#endif /*_SETJMP_INCLUDED */
/* @(#) $Revision: 66.1 $ */
#ifndef _STDARG_INCLUDED
#define _STDARG_INCLUDED

#ifndef _SYS_STDSYMS_INCLUDED
#  include <sys/stdsyms.h>
#endif /* _SYS_STDSYMS_INCLUDED */

#include <varargs.h>

#endif /* _STDARG_INCLUDED */
/* $Header: time.h,v 1.20.61.5 92/04/19 19:38:40 smp Exp $ */

#ifndef _SYS_TIME_INCLUDED
#define _SYS_TIME_INCLUDED

/* time.h: Definitions for time handling functions */

#ifdef _KERNEL_BUILD
#include "../h/stdsyms.h"
#else  /* ! _KERNEL_BUILD */
#include <sys/stdsyms.h>
#endif /* _KERNEL_BUILD */


/* ANSI C time constants, types, and structures */

#ifdef _INCLUDE__STDC__
#  define CLOCKS_PER_SEC 1000000

#  ifndef NULL
#    define NULL 0
#  endif

#  ifndef _CLOCK_T
#    define _CLOCK_T
     typedef unsigned long clock_t;
#  endif /* _CLOCK_T */

#  ifndef _TIME_T
#    define _TIME_T
     typedef long time_t;
#  endif /* _TIME_T */

#  ifndef _SIZE_T
#    define _SIZE_T
     typedef unsigned int size_t;
#  endif /* _SIZE_T */

   /* Structure used with gmtime(), localtime(), mktime(), strftime(). */
   struct tm {
      int tm_sec;	/* second (0-61, allows for leap seconds) */
      int tm_min;	/* minute (0-59) */
      int tm_hour;	/* hour (0-23) */
      int tm_mday;	/* day of the month (1-31) */
      int tm_mon;	/* month (0-11) */
      int tm_year;	/* years since 1900 */
      int tm_wday;	/* day of the week (0-6) */
      int tm_yday;	/* day of the year (0-365) */
      int tm_isdst;	/* non-0 if daylight savings time is in effect */
   };
#endif /* _INCLUDE__STDC__ */


/* Additional types needed for HP-UX */

#ifdef _INCLUDE_HPUX_SOURCE
# ifndef _STRUCT_TIMEVAL
#  define _STRUCT_TIMEVAL
   /* Structure returned by gettimeofday(2) system call and others */
     struct timeval {
	  unsigned long	tv_sec;		/* seconds */
	  long		tv_usec;	/* and microseconds */
     };
# endif /* _STRUCT_TIMEVAL */

   /* Structure used to represent timezones for gettimeofday(2) and others */
   struct timezone {
	int	tz_minuteswest;	/* minutes west of Greenwich */
	int	tz_dsttime;	/* type of dst correction */
   };

   /* Structure defining a timer setting.  */
   struct	itimerval {
	struct	timeval it_interval;	/* timer interval */
	struct	timeval it_value;	/* current value */
   };
#endif /* _INCLUDE_HPUX_SOURCE */


/* Function prototypes and external variable declarations */

#ifndef _KERNEL
#ifdef __cplusplus
   extern "C" {
#endif /* __cplusplus */

#ifdef _INCLUDE__STDC__
#  ifdef _PROTOTYPES
     extern double difftime(time_t, time_t);
     extern time_t mktime(struct tm *);
     extern time_t time(time_t *);
     extern char *asctime(const struct tm *);
     extern char *ctime(const time_t *);
     extern struct tm *gmtime(const time_t *);
     extern struct tm *localtime(const time_t *);
     extern size_t strftime(char *, size_t, const char *, const struct tm *);
#  else /* not _PROTOTYPES */
     extern double difftime();
     extern time_t mktime();
     extern time_t time();
     extern char *asctime();
     extern char *ctime();
     extern struct tm *gmtime();
     extern struct tm *localtime();
     extern size_t strftime();
#  endif /* not _PROTOTYPES */

#  ifdef _CLASSIC_ANSI_TYPES
     extern long clock();
#  else /* not _CLASSIC_ANSI_TYPES */
#    ifdef _PROTOTYPES
       extern clock_t clock(void);
#    else /* not _PROTOTYPES */
       extern clock_t clock();
#    endif /* not _PROTOTYPES */
#  endif /* not _CLASSIC_ANSI_TYPES */
#endif /* _INCLUDE__STDC__ */

#ifdef _INCLUDE_POSIX_SOURCE
#  ifdef _PROTOTYPES
     extern void tzset(void);
#  else /* not _PROTOTYPES */
     extern void tzset();
#  endif /* not _PROTOTYPES */

   extern char *tzname[2];
#endif /* _INCLUDE_POSIX_SOURCE */


#ifdef _INCLUDE_XOPEN_SOURCE
   extern long timezone;
   extern int daylight;
#endif /* _INCLUDE_XOPEN_SOURCE */


#ifdef _INCLUDE_HPUX_SOURCE
#  ifdef _PROTOTYPES
     extern struct tm *getdate(const char *);
     extern char *nl_asctime(struct tm *, char *, int);
     extern char *nl_ctime(long *, char *, int);
     extern char *nl_ascxtime(struct tm *, char *);
     extern char *nl_cxtime(long *, char *);
     extern int getitimer(int, struct itimerval *);
     extern int setitimer(int, const struct itimerval *, struct itimerval *);
     extern int gettimeofday(struct timeval *, struct timezone *);
     extern int settimeofday(const struct timeval *, const struct timezone *);
     extern int select(size_t, int *, int *, int *, const struct timeval *);
     extern int stime(const time_t *);
     extern void profil(const void *, size_t, size_t, int);
#  else /* not _PROTOTYPES */
     extern struct tm *getdate();
     extern char *nl_asctime();
     extern char *nl_ctime();
     extern char *nl_ascxtime();
     extern char *nl_cxtime();
     extern int getitimer();
     extern int setitimer();
     extern int gettimeofday();
     extern int settimeofday();
     extern int select();
     extern int stime();
     extern void profil();
#  endif /* not _PROTOTYPES */
     extern int getdate_err;
#endif /* _INCLUDE_HPUX_SOURCE */

#ifdef __cplusplus
   }
#endif /* __cplusplus */
#endif /* not _KERNEL */


/*
 * CLK_TCK is needed by the kernel, and also in the POSIX namespace.
 */

#ifdef _INCLUDE_POSIX_SOURCE
#  ifndef CLK_TCK
#    ifdef __hp9000s300
#      define CLK_TCK 50
#    endif /* __hp9000s300 */
#    ifdef __hp9000s800
#      define CLK_TCK 100
#    endif /* __hp9000s800 */
#  endif /* CLK_TCK */
#endif


/* Additional HP-UX structures, macros, and constants */

#ifdef _INCLUDE_HPUX_SOURCE

   /* Kernel instrumentation time value */
    struct	ki_timeval {
	    long	tv_sec;		/* seconds */
	    long	tv_nunit;	/* and native units */
    };

   /* Kinds of daylight savings time */
#  define DST_NONE	0	/* not on dst */
#  define DST_USA	1	/* USA style dst */
#  define DST_AUST	2	/* Australian style dst */
#  define DST_WET	3	/* Western European dst */
#  define DST_MET	4	/* Middle European dst */
#  define DST_EET	5	/* Eastern European dst */

   /*
    * Operations on timevals.
    *
    * NB: timercmp does not work for >= or <=.
    */
#  define timerisset(tvp)		((tvp)->tv_sec || (tvp)->tv_usec)
#  define timercmp(tvp, uvp, cmp) \
          ((tvp)->tv_sec cmp (uvp)->tv_sec || \
           (tvp)->tv_sec == (uvp)->tv_sec && (tvp)->tv_usec cmp (uvp)->tv_usec)
#  define timerclear(tvp)		((tvp)->tv_sec = (tvp)->tv_usec = 0)

   /* Names of the interval timers */
#  define ITIMER_REAL		0
#  define ITIMER_VIRTUAL	1
#  define ITIMER_PROF		2

#endif /* _INCLUDE_HPUX_SOURCE */

#endif /* _SYS_TIME_INCLUDED */
/* @(#) $Revision: 66.2 $ */
#ifndef _ERRNO_INCLUDED /* allow multiple inclusions */
#define _ERRNO_INCLUDED

#ifndef _SYS_STDSYMS_INCLUDED
#  include <sys/stdsyms.h>
#endif /* _SYS_STDSYMS_INCLUDED */

extern int errno;
#include <sys/errno.h>

#  ifdef __cplusplus

extern "C" {
    extern void perror(const char*);
    extern int sys_nerr;
    extern char *sys_errlist[];
    extern char *strerror (int);
}

#  endif


#endif /* _ERRNO_INCLUDED */
/* $Header: socket.h,v 1.25.61.7 92/07/07 15:16:25 indnetwk Exp $ */

#ifndef _SYS_SOCKET_INCLUDED    /* allow multiple includes of this file */
#define _SYS_SOCKET_INCLUDED    /* without causing compilation errors */

/*
 * Copyright (c) 1982, 1985, 1986 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 *	@(#)socket.h	7.3 (Berkeley) 6/27/88
 */

/*
 * Definitions related to sockets: types, address families, options.
 */

#ifdef _KERNEL_BUILD
#include "../h/stdsyms.h"
#else  /* ! _KERNEL_BUILD */
#include <sys/stdsyms.h>
#endif /* _KERNEL_BUILD */

#ifdef _INCLUDE_HPUX_SOURCE

#ifdef _KERNEL_BUILD
#include "../h/types.h"
#include "../h/ioctl.h"
#else  /* ! _KERNEL_BUILD */
#include <sys/types.h>
#include <sys/ioctl.h>
#endif /* _KERNEL_BUILD */

/*
 * Types
 */

#ifndef _CADDR_T
#  define _CADDR_T
   typedef char *caddr_t;
#endif /* _CADDR_T */

/*
 * Types of sockets
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
#define	SO_SND_COPYAVOID	0x1009		/* avoid copy on send*/
#define	SO_RCV_COPYAVOID	0x100a		/* avoid copy on rcv */

/*
 * Structure used for manipulating linger option.
 *
 * if l_onoff == 0:
 *    close(2) returns immediately; any buffered data is sent later
 *    (default)
 * 
 * if l_onoff != 0:
 *    if l_linger == 0, close(2) returns after discarding any unsent data
 *    if l_linger != 0, close(2) does not return until buffered data is sent
 */

struct	linger {
	int	l_onoff;		/* 0 = do not wait to send data */
					/* non-0 = see l_linger         */
	int	l_linger;		/* 0 = discard unsent data      */
					/* non-0 = wait to send data    */
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
#define AF_OTS		17		/* Used for OSI in the ifnets */
#define	AF_NIT		18		/* NIT */

#define	AF_MAX		19

/*
 * Structure used by kernel to store most
 * addresses.
 */
struct sockaddr {
	unsigned short sa_family;	/* address family */
	char           sa_data[14];	/* up to 14 bytes of direct address */
};

/*
 * Structure used by kernel to pass protocol
 * information in raw sockets.
 */
struct sockproto {
	unsigned short	sp_family;	/* address family */
	unsigned short	sp_protocol;	/* protocol */
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
#define	SOMAXCONN	20

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

#ifdef _KERNEL
#define MSG_EOR         0x8             /* data completes record */
#define MSG_TRUNC       0x10            /* data discarded before delivery */
#define MSG_CTRUNC      0x20            /* control data lost before delivery */
#define MSG_WAITALL     0x40            /* wait for full request or error */

/* Following used within kernel */
#define MSG_MBUF        0x1000          /* data in kernel, skip uiomove */
#define MSG_NONBLOCK    0x4000          /* nonblocking request */
#define MSG_COMPAT      0x8000          /* 4.3-format sockaddr */
#endif  /* _KERNEL */

#define	MSG_MAXIOVLEN	16


/* BSDIPC system calls */

#ifndef _KERNEL
#ifdef __cplusplus
  extern "C" {
#endif /* __cplusplus */

#ifdef _PROTOTYPES
     extern int accept(int, void *, int *);
     extern int bind(int, const void *, int);
     extern int connect(int, const void *, int);
     extern int getpeername(int, void *, int *);
     extern int getsockname(int, void *, int *);
     extern int getsockopt(int, int, int, void *, int *);
     extern int listen(int, int);
     extern int rcmd(char **, unsigned short, const char *,
				     const char *, const char *, int *);
     extern int recv(int, void *, int, int);
     extern int recvfrom(int, void *, int, int, void *, int *);
     extern int recvmsg(int, struct msghdr msg[], int);
     extern int rexec(char **, int, const char *,
				     const char *, const char *, int *);
     extern int rresvport(int *);
     extern int ruserok(const char *, int, const char *, const char *);
     extern int send(int, const void *, int, int);
     extern int sendto(int, const void *, int, int, const void *, int);
     extern int sendmsg(int, const struct msghdr msg[], int);
     extern int setsockopt(int, int, int, const void *, int);
     extern int shutdown(int, int);
     extern int socket(int, int, int);
     extern int socketpair(int, int, int, int[2]);
#else /* not _PROTOTYPES */
     extern int accept();
     extern int bind();
     extern int connect();
     extern int getpeername();
     extern int getsockname();
     extern int getsockopt();
     extern int listen();
     extern int rcmd();		/* belongs here or elsewhere? */
     extern int recv();
     extern int recvfrom();
     extern int recvmsg();
     extern int rexec();	/* belongs here or elsewhere? */
     extern int rresvport();	/* belongs here or elsewhere? */
     extern int ruserok();	/* belongs here or elsewhere? */
     extern int send();
     extern int sendto();
     extern int sendmsg();
     extern int setsockopt();
     extern int shutdown();
     extern int socket();
     extern int socketpair();
#endif /* not _PROTOTYPES */

#  ifdef __cplusplus
     }
#  endif /* __cplusplus */
#endif /* not _KERNEL */

#endif /* _INCLUDE_HPUX_SOURCE */

#endif	/* not _SYS_SOCKET_INCLUDED */
/* $Header: times.h,v 1.10.61.3 92/04/19 19:38:37 smp Exp $ */

#ifndef _SYS_TIMES_INCLUDED
#define _SYS_TIMES_INCLUDED

#ifdef _KERNEL_BUILD
#include "../h/stdsyms.h"
#else  /* ! _KERNEL_BUILD */
#include <sys/stdsyms.h>
#endif /* _KERNEL_BUILD */

#ifdef _INCLUDE_POSIX_SOURCE

#  ifndef _CLOCK_T
#    define _CLOCK_T
     typedef unsigned long clock_t;
#  endif /* _CLOCK_T */

   /*
    * Structure returned by times()
    */
      struct tms {
	   clock_t tms_utime;		/* user time */
	   clock_t tms_stime;		/* system time */
	   clock_t tms_cutime;		/* user time, children */
	   clock_t tms_cstime;		/* system time, children */
      };

#  ifndef _KERNEL
#  ifdef __cplusplus
     extern "C" {
#  endif

#  ifdef _PROTOTYPES
         extern clock_t times(struct tms *);
#  else /* not _PROTOTYPES */
#    ifdef _CLASSIC_POSIX_TYPES
         extern long times();
#    else /* not _CLASSIC_POSIX_TYPES */
         extern clock_t times();
#    endif /* not _CLASSIC_POSIX_TYPES */
#  endif /* not _PROTOTYPES */

#  ifdef __cplusplus
     }
#  endif
#  endif /* _KERNEL */

#  endif /* _INCLUDE_POSIX_SOURCE */

#endif /* _SYS_TIMES_INCLUDED */
/* @(#) $Revision: 64.2 $ */
#ifndef _SIGNAL_INCLUDED /* allow multiple inclusions */
#define _SIGNAL_INCLUDED

#include <sys/signal.h>

#endif /* _SIGNAL_INCLUDED */
/* @(#) $Revision: 66.1 $ */
#ifndef _FCNTL_INCLUDED
#define _FCNTL_INCLUDED

#ifndef _SYS_STDSYMS_INCLUDED
#  include <sys/stdsyms.h>
#endif /* _SYS_STDSYMS_INCLUDED */

#include <sys/fcntl.h>

#endif /* _FCNTL_INCLUDED */
/* @(#) $Revision: 64.11 $ */
#ifndef _UNISTD_INCLUDED
#define _UNISTD_INCLUDED

/*
 * Unistd.h
 * symbolic constants and structures which are used
 * for support of the /usr/group standard.
 */

#include <sys/unistd.h>

#endif /* _UNISTD_INCLUDED */
/* $Header: stat.h,v 1.25.61.5 92/06/17 19:28:49 rsh Exp $ */

#ifndef _SYS_STAT_INCLUDED
#define _SYS_STAT_INCLUDED

/* stat.h: definitions for use with stat(), lstat(), and fstat() */

#ifdef _KERNEL_BUILD
#include "../h/stdsyms.h"
#else  /* ! _KERNEL_BUILD */
#include <sys/stdsyms.h>
#endif /* _KERNEL_BUILD */

#ifdef _INCLUDE_POSIX_SOURCE
#ifdef _KERNEL_BUILD
#  include "../h/types.h"
#else  /* ! _KERNEL_BUILD */
#  include <sys/types.h>
#endif /* _KERNEL_BUILD */

   /*
    * The stat structure filled in by stat(), lstat(), and fstat()
    */

   struct stat
   {
	dev_t	st_dev;
	ino_t	st_ino;
	mode_t	st_mode;
	nlink_t	st_nlink;
	unsigned short st_reserved1; /* old st_uid, replaced spare positions */
	unsigned short st_reserved2; /* old st_gid, replaced spare positions */
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
	unsigned int	st_pad:30;
	unsigned int	st_acl:1;   /* set if there are optional ACL entries */
	unsigned int    st_remote:1;	/* Set if file is remote */
	dev_t   st_netdev;  	/* ID of device containing */
				/* network special file */
	ino_t   st_netino;  	/* Inode number of network special file */
	__cnode_t	st_cnode;
	__cnode_t	st_rcnode;
	/* The site where the network device lives			*/
	__site_t	st_netsite;
	short	st_fstype;
	/* Real device number of device containing the inode for this file*/
	dev_t	st_realdev;
	/* Steal three spare for the device site number                   */
	unsigned short	st_basemode;
	unsigned short	st_spareshort;
#ifdef _CLASSIC_ID_TYPES
	unsigned short st_filler_uid;
	unsigned short st_uid;
#else
	uid_t	st_uid;
#endif
#ifdef _CLASSIC_ID_TYPES
	unsigned short st_filler_gid;
	unsigned short st_gid;
#else
	gid_t	st_gid;
#endif
#define _SPARE4_SIZE 3
	long    st_spare4[_SPARE4_SIZE];
   };

#endif /* _INCLUDE_POSIX_SOURCE */


/* Function prototypes */

#ifndef _KERNEL
#ifdef __cplusplus
   extern "C" {
#endif /* __cplusplus */

#ifdef _INCLUDE_POSIX_SOURCE
#  ifdef _PROTOTYPES
     extern int chmod(const char *, mode_t);
     extern int fstat(int, struct stat *);
     extern int mkdir(const char *, mode_t);
     extern int mkfifo(const char *, mode_t);
     extern int stat(const char *, struct stat *);
#  else /* not _PROTOTYPES */
     extern int chmod();
     extern int fstat();
     extern int mkdir();
     extern int mkfifo();
     extern int stat();
#  endif /* not _PROTOTYPES */

#  ifdef _CLASSIC_POSIX_TYPES
     extern int umask();
#  else
#    ifdef _PROTOTYPES
       extern mode_t umask(mode_t);
#    else /* not _PROTOTYPES */
       extern mode_t umask();
#    endif /* not _PROTOTYPES */
#  endif /* not _CLASSIC_POSIX_TYPES */
#endif /* _INCLUDE_POSIX_SOURCE */

#ifdef _INCLUDE_AES_SOURCE
#  ifdef _PROTOTYPES
      extern int fchmod(int, mode_t);
      extern int lstat(const char *, struct stat *);
#  else /* not _PROTOTYPES */
      extern int fchmod();
      extern int lstat();
#  endif /* not _PROTOTYPES */
#endif /* _INCLUDE_AES_SOURCE */

#ifdef _INCLUDE_HPUX_SOURCE
#  ifdef _PROTOTYPES
      extern int mknod(const char *, mode_t, dev_t);
#  else /* not _PROTOTYPES */
      extern int mknod();
#  endif /* not _PROTOTYPES */
#endif /* _INCLUDE_HPUX_SOURCE */

#ifdef __cplusplus
   }
#endif /* __cplusplus */
#endif /* not _KERNEL */


/* Symbols and macros for decoding the value of st_mode */

#ifdef _INCLUDE_POSIX_SOURCE
#  ifndef S_IRWXU		/* fcntl.h might have already done these */
#    define S_IRWXU 0000700	/* read, write, execute permission (owner) */
#    define S_IRUSR 0000400	/* read permission (owner) */
#    define S_IWUSR 0000200	/* write permission (owner) */
#    define S_IXUSR 0000100	/* execute permission (owner) */

#    define S_IRWXG 0000070	/* read, write, execute permission (group) */
#    define S_IRGRP 0000040	/* read permission (group) */
#    define S_IWGRP 0000020	/* write permission (group) */
#    define S_IXGRP 0000010	/* execute permission (group) */

#    define S_IRWXO 0000007	/* read, write, execute permission (other) */
#    define S_IROTH 0000004	/* read permission (other) */
#    define S_IWOTH 0000002	/* write permission (other) */
#    define S_IXOTH 0000001	/* execute permission (other) */
#  endif /* S_IRWXU */

#  define S_ISUID 0004000	/* set user ID on execution */
#  define S_ISGID 0002000	/* set group ID on execution */

#  define _S_IFMT   0170000	/* type of file */
#  define _S_IFREG  0100000	/* regular */
#  define _S_IFBLK  0060000	/* block special */
#  define _S_IFCHR  0020000	/* character special */
#  define _S_IFDIR  0040000	/* directory */
#  define _S_IFIFO  0010000	/* pipe or FIFO */

#  define S_ISDIR(_M)  ((_M & _S_IFMT)==_S_IFDIR) /* test for directory */
#  define S_ISCHR(_M)  ((_M & _S_IFMT)==_S_IFCHR) /* test for char special */
#  define S_ISBLK(_M)  ((_M & _S_IFMT)==_S_IFBLK) /* test for block special */
#  define S_ISREG(_M)  ((_M & _S_IFMT)==_S_IFREG) /* test for regular file */
#  define S_ISFIFO(_M) ((_M & _S_IFMT)==_S_IFIFO) /* test for pipe or FIFO */
#endif /* _INCLUDE_POSIX_SOURCE */

#ifdef _INCLUDE_XOPEN_SOURCE
#  define S_IFMT	_S_IFMT		/* type of file */
#  define S_IFBLK	_S_IFBLK	/* block special */
#  define S_IFCHR	_S_IFCHR	/* character special */
#  define S_IFDIR	_S_IFDIR	/* directory */
#  define S_IFIFO	_S_IFIFO	/* pipe or FIFO */
#  define S_IFREG	_S_IFREG	/* regular */
#  ifndef _XPG4
#    define S_ISVTX	0001000		/* save swapped text even after use */
#  endif /* not _XPG4 */
#endif /* _INCLUDE_XOPEN_SOURCE */

#ifdef _INCLUDE_AES_SOURCE
#  define S_IFLNK	0120000	/* symbolic link */
#  define S_ISLNK(_M) ((_M & S_IFMT)==S_IFLNK)   /* test for symbolic link */
#endif /* _INCLUDE_AES_SOURCE */

#ifdef _INCLUDE_HPUX_SOURCE
#  define S_CDF		0004000	/* hidden directory */
#  define S_ENFMT	0002000	/* enforced file locking (shared w/ S_ISGID) */
#  define S_IFNWK	0110000 /* network special */
#  define S_IFSOCK	0140000	/* socket */

#  ifndef S_ISVTX
#    define S_ISVTX	0001000	/* save swapped text even after use */
#  endif /* not S_ISVTX */

#  define S_ISNWK(_M) ((_M & S_IFMT)==S_IFNWK)  /* test for network special */
#  define S_ISSOCK(_M) ((_M & S_IFMT)==S_IFSOCK) /* test for socket */
#  define S_ISCDF(_M) (S_ISDIR(_M) && (_M & S_CDF)) /* test for hidden dir */


/* Some synonyms used historically in the kernel and elsewhere */

#  define S_IREAD  	S_IRUSR	/* read permission, owner */
#  define S_IWRITE 	S_IWUSR	/* write permission, owner */
#  define S_IEXEC  	S_IXUSR	/* execute/search permission, owner */

#  define st_rsite st_rcnode

#endif /* _INCLUDE_HPUX_SOURCE */

#ifdef _UNSUPPORTED

	/* 
	 * NOTE: The following header file contains information specific
	 * to the internals of the HP-UX implementation. The contents of 
	 * this header file are subject to change without notice. Such
	 * changes may affect source code, object code, or binary
	 * compatibility between releases of HP-UX. Code which uses 
	 * the symbols contained within this header file is inherently
	 * non-portable (even between HP-UX implementations).
	*/
#ifdef _KERNEL_BUILD
#	include "../h/_stat.h"
#else  /* ! _KERNEL_BUILD */
#	include <.unsupp/sys/_stat.h>
#endif /* _KERNEL_BUILD */
#endif /* _UNSUPPORTED */

#endif /* _SYS_STAT_INCLUDED */
/* @(#) $Revision: 1.65.61.16 $ */

#ifndef _SYS_PARAM_INCLUDED /* allows multiple inclusion */
#define _SYS_PARAM_INCLUDED

#ifndef _SYS_STDSYMS_INCLUDED
#ifdef _KERNEL_BUILD
#    include "../h/stdsyms.h"
#else  /* ! _KERNEL_BUILD */
#    include <sys/stdsyms.h>
#endif /* _KERNEL_BUILD */
#endif   /* _SYS_STDSYMS_INCLUDED  */

/*
 * Kernel Type Declarations
 */
#ifdef _KERNEL_BUILD
#ifndef LOCORE
#include "../h/types.h"
#endif /* not LOCORE */
#include "../machine/param.h"
#include "../h/time.h"
#ifdef __hp9000s800
#include "../machine/spl.h"
#endif /* __hp9000s800 */
#else /* ! _KERNEL_BUILD */
#include <sys/types.h>
#include <machine/param.h>
#include <sys/time.h>
#ifdef __hp9000s800
#include <machine/spl.h>
#endif /* __hp9000s800 */
#endif /* _KERNEL_BUILD */

/*
 * Machine-independent constants
 */

#ifdef _WSIO
/* defines for the front panel led lights */
#define led0            0x01
#define led1            0x02
#define led2            0x04
#define led3            0x08
#define KERN_OK_LED     0x10
#define DISK_DRV_LED    0x20
#define LAN_RCV_LED     0x40
#define LAN_XMIT_LED    0x80
#endif /* _WSIO */

#define	MSWAPX	15		/* pseudo mount table index for swapdev */

#define MAXCOMLEN 14		/* size of process name buffer for accounting */
#define	NOFILE	60		/* max open files per process */
#define MAX_LOCK_SIZE (long)(0x7fffffff) /* max lockable offset in file */

#define	MAXPID	30000		/* max process id */
#define	MAXUID	60000		/* max user id */
#define	MAXLINK	0x7fff		/* max # links to a file */
#ifndef MAX_CNODE		/* see also MAXCNODE in <cluster.h> */
#define MAX_CNODE 255           /* max cnode id */
#endif

#define MAXHOSTNAMELEN 64	/* max length of hostname */

#ifdef __hp9000s800
#if NBPG==4096
#define SSIZE	2		/* initial stack size/NBPG */
#define SINCR	2		/* increment of stack/NBPG */
#else /* NBPG == 4096 */
#define SSIZE	4		/* initial stack size/NBPG */
#define SINCR	4		/* increment of stack/NBPG */
#endif /* NBPG==4096 */

#define UAREA 0x7ffe6000                /* start of u area */
	/* KSTACKADDR is the base of the kernel stack, actually IN Uarea */
#define KSTACKADDR (UAREA+sizeof(struct user)-sizeof(double))  
#define TOPKSTACK (UAREA+UPAGES*NBPG)    /* top of kernel stack */

#define USRSTACK 0x7b033000   /* Start of user stack */
#define USRSTACKMAX UAREA     /* Top of user stack */

#define	CANBSIZ	256		/* max size of typewriter line	*/

#define	HZ	CLK_TCK		/* Ticks/second of the clock (in kernel land) */

#endif /* __hp9000s800 */

#ifdef __hp9000s300

/*
 * USRTEXT is the start of the user text/data space, while USRSTACK
 * is the top (end) of the user stack.  LOWPAGES and HIGHPAGES are
 * the number of pages from the beginning of the P0 region to the
 * beginning of the text and from the beginning of the P1 region to the
 * beginning of the stack respectively.
 */

#define	USRTEXT		0

#define UPAGES         1    /* pages for mapping struct user */
#define KSTACK_PAGES   4    /* pages of kernel stack (must be >= 2) */
#define FLOAT          5    /* pages of float card area */
#define DRAGON_PAGES  32
#define GAP1_PAGES    (216 - KSTACK_PAGES)
#define GAP2_PAGES    (3 - UPAGES)
#define HIGHPAGES  (DRAGON_PAGES+GAP1_PAGES+KSTACK_PAGES+UPAGES+GAP2_PAGES+FLOAT)
#define USRSTACK   (caddr_t) (-(HIGHPAGES*NBPG))
#define UAREA  (USRSTACK+((DRAGON_PAGES+GAP1_PAGES+KSTACK_PAGES)*NBPG))
#define FLOAT_AREA ((caddr_t) (-(FLOAT*NBPG)))
#define DRAGON_AREA     (USRSTACK)

#define KSTACK_RESERVE 3    /* # of reserve pages in kernel stack private pool */

#define	CANBSIZ	512		/* max size of typewriter line	*/

#define	HZ	CLK_TCK		/* Ticks/second of the clock (in kernel land) */

#endif /* __hp9000s300 */


/*
 * Maximum values (in seconds) for alarms, interval timers, and timeouts
 */
#define	MAX_ALARM	((unsigned long)(0x7fffffff / HZ))
#define	MAX_VTALARM	((unsigned long)0xffffffff)
#define	MAX_VT_ALARM	(MAX_VTALARM)	/* please use MAX_VTALARM instead */
#define	MAX_PROF	((unsigned long)0xffffffff)

#ifdef _KERNEL
/*
 * Note that the value of ARG_MAX, defined in <limits.h>, in shared source,
 * tracks this value as NCARGS-2.  If you change NCARGS, please also update
 * the value of ARG_MAX
 *
 * Also NOTE that NCARGS needs to be a multiple of NBPW.
 */
#endif /* _KERNEL */
#define	NCARGS	20480		/* # characters in exec arglist */

/*
 * priorities
 * should not be altered too much
 */

#define	PMASK	0xff
#define	PCATCH	0x100

#ifdef __hp9000s800
#define PRTBASE 0
#define PTIMESHARE 128
#endif /* __hp9000s800 */

/*
 * Priorities stronger (smaller number) than (or equal to) PZERO
 * are not signalable.
 */

#ifdef __hp9000s300
#ifdef	RTPRIO
#define PRTBASE 0
#define PTIMESHARE 128
#else	/* not RTPRIO */
#define PTIMESHARE  0
#endif	/* else not RTPRIO */
#endif /* __hp9000s300 */


#define	PSWP	(0+PTIMESHARE)
#define PMEM	(0+PTIMESHARE)
#define PRIRWLOCK (5+PTIMESHARE)
#define PRIBETA	(6+PTIMESHARE)
#define PRIALPHA (7+PTIMESHARE)
#define PRISYNC	(8+PTIMESHARE)
#define	PINOD	(10+PTIMESHARE)
#define	PRIBIO	(20+PTIMESHARE)
#define	PRIUBA	(24+PTIMESHARE)
#ifdef __hp9000s800
#define	PLLIO	(24+PTIMESHARE)
#endif /* __hp9000s800 */
#define	PZERO	(25+PTIMESHARE)
#define IPCPRI  (25+PTIMESHARE)

/*
 * Priorities weaker (bigger number) than PZERO are signalable.
 */

#define	PPIPE	(26+PTIMESHARE)
#define PVFS	(27+PTIMESHARE)
#define	PWAIT	(30+PTIMESHARE)
#define	PLOCK	(35+PTIMESHARE)
#define	PSLEP	(40+PTIMESHARE)
#define	PUSER	(50+PTIMESHARE)

#define PMAX_TIMESHARE	(127+PTIMESHARE)

/*
 * Special argument to changepri(), meaning "the appropriate non-real-time
 * priority".
 */
#define PNOT_REALTIME	(-1)

/*
 * Used in nice(2) calculations.
 */
#define NZERO 20

/*
 * fundamental constants of the implementation--
 * cannot be changed easily
 */

#define	NBPW	sizeof(int)	/* number of bytes in an integer */
#define NBTSPW	(NBBY*NBPW)	/* number of bits in an integer */

#define BSIZE   DEV_BSIZE
#define BSHIFT  DEV_BSHIFT

/*
 * NINDIR is the number of indirects in a file system block.
 */
#define	NINDIR(fs)	((fs)->fs_nindir)
/*
 * INOPB is the number of inodes in a secondary storage block.
 */
#define	INOPB(fs)	((fs)->fs_inopb)
#define INOPF(fs)       ((fs)->fs_inopb >> (fs)->fs_fragshift)
#ifndef NULL
#define	NULL	0
#endif
#define	CMASK	0		/* default mask for file creation */
#define	CDLIMIT	0x1FFFFFFF	/* default max write address */
#define	NODEV	(dev_t)(-1)
#define	SWDEF	(dev_t)(-2)
/* The root inode is the root of the file system.
 * Inode 0 can't be used for normal purposes and
 * historically bad blocks were linked to inode 1,
 * thus the root inode is 2. (inode 1 is no longer used for
 * this purpose, however numerous dump tapes make this
 * assumption, so we are stuck with it)
 * The lost+found directory is given the next available
 * inode when it is created by ``mkfs''.
 */
#define	ROOTINO		((ino_t)2)	/* i number of all roots */
#define LOSTFOUNDINO    (ROOTINO + 1)
#ifdef __hp9000s300
#define	SUPERBOFF	512	/* byte offset of the super block */
#define	DIRSIZ	14		/* max characters per directory */
#define	NICINOD	100		/* number of superblock inodes */
#endif /* __hp9000s300 */
#ifdef __hp9000s800
/*
 * Decode privilege level for HP-PA.
 */
#define PC_PRIV_MASK	3
#define PC_PRIV_KERN	0
#define PC_PRIV_USER	3

#define USERMODE(pc)	(((pc) & PC_PRIV_MASK) != PC_PRIV_KERN)

#define	lobyte(X)	(((unsigned char *)&X)[1])
#define	hibyte(X)	(((unsigned char *)&X)[0])
#define	loword(X)	(((ushort *)&X)[1])
#define	hiword(X)	(((ushort *)&X)[0])
#endif /* __hp9000s800 */

#ifdef __hp9000s300
#define USERMODE(ps)    (((ps) & PS_S) == 0)  /* check for user mode */
#define BASEPRI(ps)     (((ps) & PS_IPL) == 0)    /* check for int level 0 */
#endif /* __hp9000s300 */

#define	NGROUPS	20		/* max number groups */
#define	NOGROUP	((gid_t) -1)	/* marker for empty group set member */
/*
 * Signals.  Only include if defining _KERNEL.
 */
#ifdef _KERNEL
#ifdef _KERNEL_BUILD
#include "../h/signal.h"
#else /* ! _KERNEL_BUILD */
#include <sys/signal.h>
#endif /* _KERNEL_BUILD */
#endif /* _KERNEL */

#ifdef __hp9000s800
/* There are places where we would like to know if we have a possible
 * signal that we might have to handle.  If so, we have to grab the
 * kernel semaphore.  Note, this is a hint, and does not include the
 * call to issig...
 */
#define	ISSIG_MP(p) \
	(((p)->p_cursig) || \
	((p)->p_sig && ((p)->p_flag&STRC || \
	 ((p)->p_sig &~ (p)->p_sigmask))) )

#define	ISSIG(p) \
	((p)->p_sig && ((p)->p_flag&STRC || \
	 ((p)->p_sig &~ (p)->p_sigmask)) && issig())
#endif /* __hp9000s800 */

/*
 * Fundamental constants of the implementation.
 */
#ifndef NBBY
#define	NBBY	8		/* number of bits in a byte */
 				/* NOTE: this is also defined	*/
 				/* in fs.h (filsys.h).  So if	*/
 				/* NBBY gets changed, change it	*/
 				/* in fs.h (filsys.h) also	*/
#endif /* NBBY */

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
 * Block devices are read in BLKDEV_IOSIZE units. This number must
 * be a power of two and in the range of
 *	DEV_BSIZE <= BLKDEV_IOSIZE <= MAXBSIZE
 * This size has no effect upon the file system, but is usually set
 * to the block size of the root file system, so as to maximize the
 * speed of ``fsck''.
 */
#define	MAXBSIZE	65536
#ifndef	DEV_BSIZE
#define	DEV_BSIZE	1024
#define	DEV_BSHIFT	10      	/* log2(DEV_BSIZE) */
#define DEV_BMASK	(DEV_BSIZE-1)	/* For doing modulo functions */
#endif /* DEV_BSIZE */

#define BLKDEV_IOSIZE	2048
#define	BLKDEV_IOSHIFT	11
#define BLKDEV_IOMASK	(BLKDEV_IOSIZE-1)

#define	btodb(bytes)	 		/* calculates (bytes / DEV_BSIZE) */ \
	((unsigned)(bytes) >> DEV_BSHIFT)
#define	dbtob(db)			/* calculates (db * DEV_BSIZE) */ \
	((unsigned)(db) << DEV_BSHIFT)

#define btodbup(bytes)			/* same as btodb but round up */ \
	(((unsigned)(bytes) + (DEV_BSIZE-1)) >> DEV_BSHIFT)

/*
 * Map a ``block device block'' to a file system block.
 * This should be device dependent, and will be after we
 * add an entry to cdevsw for that purpose.  For now though
 * just use DEV_BSIZE.
 */
#define	bdbtofsb(bn)	((bn) / (BLKDEV_IOSIZE/DEV_BSIZE))

/*
 * MAXPATHLEN defines the longest permissable path length
 * after expanding symbolic links. It is used to allocate
 * a temporary buffer from the buffer pool in which to do the
 * name expansion, hence should be a power of two, and must
 * be less than or equal to MAXBSIZE.
 * MAXSYMLINKS defines the maximum number of symbolic links
 * that may be expanded in a path name. It should be set high
 * enough to allow all legitimate uses, but halt infinite loops
 * reasonably quickly.
 */
#define MAXPATHLEN	1024
#define MAXSYMLINKS	20

/*
 * bit map related macros
 */
#define	setbit(a,i)	((a)[(i)/NBBY] |= 1<<((i)%NBBY))
#define	clrbit(a,i)	((a)[(i)/NBBY] &= ~(1<<((i)%NBBY)))
#define	isset(a,i)	((a)[(i)/NBBY] & (1<<((i)%NBBY)))
#define	isclr(a,i)	(((a)[(i)/NBBY] & (1<<((i)%NBBY))) == 0)

/*
 * bit map related macros done on a word basis
 */
#define wsetbit(a,i)	((a)[(i)/NBTSPW] |= 1<<((i)%NBTSPW))
#define wclrbit(a,i)	((a)[(i)/NBTSPW] &= ~(1<<((i)%NBTSPW)))
#define wisset(a,i)	((a)[(i)/NBTSPW] & (1<<((i)%NBTSPW)))
#define wisclr(a,i)	(((a)[(i)/NBTSPW] & (1<<((i)%NBTSPW))) == 0)

/*
 * Macros for fast min/max.
 */
#define	MIN(a,b) (((a)<(b))?(a):(b))
#define	MAX(a,b) (((a)>(b))?(a):(b))

/*
 * Macros for counting and rounding.
 */
#define	howmany(x, y)	(((x)+((y)-1))/(y))
#define	roundup(x, y)	((((x)+((y)-1))/(y))*(y))

/*
 * Definitions for asychronous kernel preemption.
 * Kernel-specific functionality, so only try
 * to include kpreempt.h if we are defining _KERNEL.
 */
#ifdef _KERNEL_BUILD
#include "../h/kpreempt.h"
#endif /* _KERNEL_BUILD */



#ifdef _UNSUPPORTED

	/* 
	 * NOTE: The following header file contains information specific
	 * to the internals of the HP-UX implementation. The contents of 
	 * this header file are subject to change without notice. Such
	 * changes may affect source code, object code, or binary
	 * compatibility between releases of HP-UX. Code which uses 
	 * the symbols contained within this header file is inherently
	 * non-portable (even between HP-UX implementations).
	*/
#ifdef _KERNEL_BUILD
#	include "../h/_param.h"
#else  /* ! _KERNEL_BUILD */
#	include <.unsupp/sys/_param.h>
#endif /* _KERNEL_BUILD */
#endif /* _UNSUPPORTED */

#endif /* _SYS_PARAM_INCLUDED */
/* $Header: file.h,v 1.32.61.5 92/04/19 20:35:36 smp Exp $ */

#ifndef _SYS_FILE_INCLUDED /* allows multiple inclusion */
#define _SYS_FILE_INCLUDED

#ifndef _SYS_STDSYMS_INCLUDED
#ifdef _KERNEL_BUILD
#    include "../h/stdsyms.h"
#else  /* ! _KERNEL_BUILD */
#    include <sys/stdsyms.h>
#endif /* _KERNEL_BUILD */
#endif   /* _SYS_STDSYMS_INCLUDED  */

#ifdef _KERNEL_BUILD
#include "../h/types.h"
#include "../h/fcntl.h"
#else  /* ! _KERNEL_BUILD */
#include <sys/types.h>
#include <fcntl.h>
#endif /* _KERNEL_BUILD */

#ifdef _INCLUDE_HPUX_SOURCE

/*
 * Descriptor table entry.
 * One for each kernel object.
 */
struct	file {
	int	f_flag;		/* see below */
	short	f_type;		/* descriptor type */
	short	f_count;	/* reference count */
	short	f_msgcount;	/* references from message queue */
#ifdef __cplusplus
	struct	fileops {
		int	(*fo_rw)(...);
		int	(*fo_ioctl)(...);
		int	(*fo_select)(...);
		int	(*fo_close)(...);
	} *f_ops;
#else /* not __cplusplus */
	struct	fileops {
		int	(*fo_rw)();
		int	(*fo_ioctl)();
		int	(*fo_select)();
		int	(*fo_close)();
	} *f_ops;
#endif /* not __cplusplus */
	caddr_t	f_data;		/* ptr to file specific struct (vnode/socket) */
	off_t	f_offset;
#ifdef _WSIO /* DIL */
	caddr_t	f_buf;
#endif /* _WSIO */
	struct	ucred *f_cred;	/* credentials of user who opened file */
};

#ifdef _KERNEL
extern	struct	file *file, *fileNFILE, *file_reserve;
extern	int	nfile;
extern  int     file_pad;
extern  struct  file *getf();
extern	struct	file **getfp();
extern	char	getp();
extern	char 	*getpp();
extern  struct  file *falloc();
#endif /* _KERNEL */

/* 
 * flags- also for fcntl call.
 */
#define	FOPEN		(-1)
#define	FREAD		000001		/* descriptor read/receive'able */
#define	FWRITE		000002		/* descriptor write/send'able */
/*
  FNDELAY and FAPPEND have equivalents in fcntl.h.  The numbers should 
  be the same!  HP replicates them because Bell replicates them.
 */
#define	FNDELAY		000004		/* no delay */
#define	FAPPEND		000010		/* append on each write */
#define	FMARK		000020		/* mark during gc() */
#define	FDEFER		000040		/* defer for next gc pass */
#define FNBLOCK	       0200000		/* POSIX */

/* bits to save after open */
/* FMASK defines the bits to save after open */
/* FCNTLCANT defines the bits that can't be modified with fcntl() */
/* FNOCTTY has equivalent in fcntl.h, O_NOCTTY. */
#define FNOCTTY		0400000 /* do not affiliate terminal device on open */
#define FMASK		(FREAD|FWRITE|FNDELAY|FNBLOCK|FAPPEND|FSYNCIO|FNOCTTY)
#define FCNTLCANT	(FREAD|FWRITE|FMARK|FDEFER|FNOCTTY)

/*
  FCREAT, FTRUNC and FEXCL have equivalents in fcntl.h.  The numbers must
  be the same!  HP replicates them because Bell replicates them.
 */
/* open only modes */
#define	FCREAT		00400		/* create if nonexistant */
#define	FTRUNC		01000		/* truncate to zero length */
#define	FEXCL		02000		/* error if already created */

/*
 * Access call.
 */
/*
  These have equivalents in unistd.h.  The numbers must
  be the same!  HP replicates them because Bell replicates them.
 */
#ifndef R_OK    
#define	F_OK		0	/* does file exist */
#define	X_OK		1	/* is it executable by caller */
#define	W_OK		2	/* writable by caller */
#define	R_OK		4	/* readable by caller */
#endif

/*
 * Lseek call.
 */
/*
  These have equivalents in unistd.h (SEEK_*).  The numbers must
  be the same!  HP replicates them because Bell replicates them.
 */
#define	L_SET		0	/* absolute offset */
#define	L_INCR		1	/* relative to current offset */
#define	L_XTND		2	/* relative to end of file */

#ifdef _KERNEL
/*
 * Convert a user supplied file descriptor into a pointer, with side effect
 * of returning if there is no file pointer for this file descriptor.
 */
#define GETF(fp, fd) { \
	if (((fp) = getf((fd))) == NULL) \
		return; \
}
/*
 * GETFP returns a pointer to location u.u_ofile.ofile[fd].  Should be
 * used when you need to modify u.u_ofile to make it point to something else.
 */
#define GETFP(fpp, fd) { \
	(fpp) = getfp((fd)); \
}
/*
 * GETP returns the contents of u.u_ofile.pofile[fd].  Should be used when
 * you need to examine u.u_ofile.pofile[fd].  
 */
#define GETP(c, fd) { \
	(c) = getp((fd)); \
}
/*
 * GETPP returns a pointer to location u.u_ofile.pofile[fd].  Should be used
 * when you need to modify u.u_ofile.pofile[fd].  
 */
#define GETPP(cp, fd) { \
	(cp) = getpp((fd)); \
}
#define FREEFP(fd, fp) { \
        if ((fp) != NULL) { \
                uffree(fd); \
                crfree((fp)->f_cred); \
		FPENTRYFREE(fp); \
                } \
}
/*
 * FPENTRYFREE frees an active (in-use) file table entry and decrements the 
 * number of active file table entries (for sar(1), etc. support).
 */
#define FPENTRYFREE(fp) { \
	(fp)->f_count = 0; \
	/* When the mpcntrs struct is added to mp.h -> UN-comment next line. \
	MPCNTRS.activefiles--; \
	*/ \
}

#endif /* _KERNEL */

#define	DTYPE_VNODE	1	/* file */
#define	DTYPE_SOCKET	2	/* Berkeley IPC Socket */
#define DTYPE_UNSP      5       /* user nsp control */
#define DTYPE_LLA	6	/* link-level lan access */

#endif /* _INCLUDE_HPUX_SOURCE */

#endif /* _SYS_FILE_INCLUDED */
/* @(#) $Revision: 66.2 $ */       
#ifndef _MEMORY_INCLUDED /* allows multiple inclusions */
#define _MEMORY_INCLUDED

#ifndef _SYS_STDSYMS_INCLUDED
#  include <sys/stdsyms.h>
#endif /* _SYS_STDSYMS_INCLUDED */

#include <string.h>

#endif /* _MEMORY_INCLUDED */
/* $Header: vfs.h,v 1.7.61.9 92/04/24 14:25:40 smp Exp $ */

#ifndef _SYS_VFS_INCLUDED
#define _SYS_VFS_INCLUDED

#ifdef _KERNEL_BUILD
#include "../h/stdsyms.h"
#else  /* ! _KERNEL_BUILD */
#include <sys/stdsyms.h>
#endif /* _KERNEL_BUILD */

#ifdef _INCLUDE_HPUX_SOURCE

#ifdef _KERNEL_BUILD
#include "../h/types.h"
#include "../h/param.h"
#else  /* ! _KERNEL_BUILD */
#include <sys/types.h>
#include <sys/param.h>
#endif /* not _KERNEL */


/*
 * file system statistics
 */
typedef long fsid_t[2];			/* file system ID type */


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
	u_short         vfs_exroot;             /* exported fs uid 0 mapping */
	short           vfs_exflags;            /* exported fs flags */
	caddr_t		vfs_data;		/* private data */
	int		vfs_icount;		/* ref count of processes */
						/* sleeping on the mnt inode */
	short		vfs_mtype;		/* Type of vfs */
	site_t		vfs_site;		/* Site holding vfs_lock */
	fsid_t		vfs_fsid;		/* file system ID for vfs_get */
	struct log_hdrT	*vfs_logp;		/* ptr to WA log */
	time_t		vfs_mnttime;		/* time mounted */
	char 		vfs_name[MAXPATHLEN];	/* file sys identifier */
};

/*
 * vfs flags.
 * VFS_MLOCK lock the vfs so that name lookup cannot proceed past the vfs.
 * This keeps the subtree stable during mounts and unmounts.
 */
#define VFS_RDONLY	0x01		/* read only vfs */
#define VFS_MLOCK	0x02		/* lock vfs so that subtree is stable */
#define VFS_MWAIT	0x04		/* someone is waiting for lock */
#define VFS_NOSUID      0x08            /* someone is waiting for lock */
#define VFS_EXPORTED    0x10            /* file system is exported (NFS) */
#define VFS_HARDENED    0x20            /* hardened filesystem */
#define VFS_QUOTA	0x40            /* filesystem with quotas */
#ifdef	__hp9000s800
#define VFS_MI_DEV	0x100		/* dev_t has mgr_index in it already */
#else	/* __hp9000s800 */
#define VFS_MI_DEV	0
#endif	/* __hp9000s9800 */
/*
 * exported vfs flags.
 */
#define EX_RDONLY       0x01            /* exported read only */
#define EX_RDMOSTLY     0x02            /* exported read mostly (NFS) */
#define EX_ASYNC        0x04            /* exported -async (NFS) */

/*
 * Operations supported on virtual file system.
 */
#ifdef __cplusplus
#  define __x  ...
   extern "C" {
#else
#  define __x
#endif

struct vfsops {
	int     (*vfs_mount)(__x);
	int	(*vfs_unmount)(__x);
	int	(*vfs_root)(__x);
	int	(*vfs_statfs)(__x);
	int	(*vfs_sync)(__x);
	int	(*vfs_vget)(__x);	/* get vnode from fid */
	int	(*vfs_getmount)(__x);	/* get mount info */
};

#ifdef __cplusplus
   }
#  undef __x
#endif


#define VFS_MOUNT(VFSP, PATH, DATA) \
				(*(VFSP)->vfs_op->vfs_mount)(VFSP, PATH, DATA)
#define VFS_UNMOUNT(VFSP)		(*(VFSP)->vfs_op->vfs_unmount)(VFSP)
#define VFS_ROOT(VFSP,VPP,NAME)	(*(VFSP)->vfs_op->vfs_root)(VFSP,VPP,NAME)
#define VFS_STATFS(VFSP, SBP)		(*(VFSP)->vfs_op->vfs_statfs)(VFSP,SBP)
#define VFS_SYNC(VFSP)			(*(VFSP)->vfs_op->vfs_sync)(VFSP)
/*
 * VFS_VGET -- given a pointer to a file system, and the file ID (FID),
 * lookup the vnode pointer.  Primarily used by the NFS server code to
 * translate file handles into vnodes.
 */
#define VFS_VGET(VFSP, VPP, FIDP) (*(VFSP)->vfs_op->vfs_vget)(VFSP, VPP, FIDP)
/*
 * VFS_GETMOUNT -- retrieve mount mounted on directory and other
 * file system information.  FSMNTDIR is a user address to which
 * the mounted on directory info is copied out.
 */
#define VFS_GETMOUNT(VFSP, FSMNTDIR, MNTDATA) \
	(*(VFSP)->vfs_op->vfs_getmount)(VFSP, FSMNTDIR, MNTDATA)


struct statfs {
	long f_type;			/* type of info, zero for now */
	long f_bsize;			/* fundamental file system block size */
	long f_blocks;			/* total blocks in file system */
	long f_bfree;			/* free block in fs */
	long f_bavail;			/* free blocks avail to non-superuser */
	long f_files;			/* total file nodes in file system */
	long f_ffree;			/* free file nodes in fs */
	fsid_t f_fsid;			/* file system ID */
	long f_magic;			/* file system magic number */
	long f_featurebits;		/* file system features */
	long f_spare[4];
	site_t f_cnode;			/* cluster node where mounted */
	short f_pad;
};

#ifdef _KERNEL
/*
 * public operations
 */
extern void	vfs_mountroot();	/* mount the root */
extern int	vfs_add();		/* add a new vfs to mounted vfs list */
extern void	vfs_remove();		/* remove a vfs from mounted vfs list */
extern int	vfs_lock();		/* lock a vfs */
extern void	vfs_unlock();		/* unlock a vfs */


#define VFS_INIT(VFSP, OP, DATA, MTYPE)	{ \
	(VFSP)->vfs_next = (struct vfs *)0; \
	(VFSP)->vfs_op = (OP); \
	(VFSP)->vfs_flag = 0; \
	(VFSP)->vfs_exflags = 0; \
	(VFSP)->vfs_icount = 0; \
	(VFSP)->vfs_data = (DATA); \
	(VFSP)->vfs_mtype = (MTYPE); \
	(VFSP)->vfs_logp = 0; \
}

/*
 * globals
 */
extern struct vfs *rootvfs;		/* ptr to root vfs structure */

#ifdef NDCLIENT
/*see changes below */
#endif /* NDCLIENT */

/* values below same as in mount.h */
#define MOUNT_UFS   0
#define MOUNT_NFS   1
#define MOUNT_CDFS  2		/*ISO-9660 and HSG file system*/
#define MOUNT_PC    3
#define MOUNT_DCFS  4		/* data compression fs */

#define       vfssw_assign(a1, a2) \
      { \
              vfssw[a1] =    &a2; \
      }

#endif /* _KERNEL */

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


/* Function prototypes */

#ifndef _KERNEL
#ifdef __cplusplus
  extern "C" {
#endif

#ifdef _PROTOTYPES
# ifdef __cplusplus
    extern struct vfs *getvfs(...);		/* return vfs given fsid */
# else /* not __cplusplus */
    extern struct vfs *getvfs();		/* return vfs given fsid */
# endif /* not __cplusplus */
    extern int statfs(const char *, struct statfs *);
    extern int fstatfs(int, struct statfs *);
    extern int statfsdev(const char *, struct statfs *);
    extern int fstatfsdev(int, struct statfs *);
#else /* not _PROTOTYPES */
    extern struct vfs *getvfs();		/* return vfs given fsid */
    extern int statfs();
    extern int fstatfs();
    extern int statfsdev();
    extern int fstatfsdev();
#endif /* not _PROTOTYPES */

#ifdef __cplusplus
  }
#endif /* not __cplusplus */
#endif /* not _KERNEL */

struct mount_data
{
	int	md_fstype;	/* type of fs (e.g. MOUNT_UFS, MOUNT_NFS */
	int	md_fsopts;	/* generic fs options: rw/ro, suid/nosuid */
	time_t	md_mnttime;	/* time of file system mount */
	site_t	md_msite;	/* cnode ID of mounting site */
	short	md_spare;	/* pad */
	dev_t	md_dev;		/* encoded major/minor dev */
	union {
		struct {
			dev_t	mdu_rdev;	/*  real major/minor dev */
			int	mdu_ufsopts;	/* ufs flags/options */
		} md_ufs;
		struct {
			int	mdn_nfsopts;	/* nfs mount options */
			int	mdn_retrans;	/* # retries */
			int	mdn_timeo;	/* timeout for retry */
			int	mdn_rsize;	/* read size */
			int	mdn_wsize;	/* write size */
			u_long	mdn_port;	/* IP port number */
			int	mdn_acregmin;	/* min secs cache file attr */
			int	mdn_acregmax;	/* max secs cache file attr */
			int	mdn_acdirmin;	/* min secs cache dir attr */
			int	mdn_acdirmax;	/* max secs cache dir attr */
		} md_nfs;
	} md_un;
};

#define md_rdev		md_un.md_ufs.mdu_rdev
#define md_ufsopts 	md_un.md_ufs.mdu_ufsopts
#define md_nfsopts 	md_un.md_nfs.mdn_nfsopts
#define md_retrans 	md_un.md_nfs.mdn_retrans
#define md_timeo	md_un.md_nfs.mdn_timeo
#define md_rsize	md_un.md_nfs.mdn_rsize
#define md_wsize	md_un.md_nfs.mdn_wsize
#define md_port		md_un.md_nfs.mdn_port
#define md_acregmin	md_un.md_nfs.mdn_acregmin
#define md_acregmax	md_un.md_nfs.mdn_acregmax
#define md_acdirmin	md_un.md_nfs.mdn_acdirmin
#define md_acdirmax	md_un.md_nfs.mdn_acdirmax

#endif /* _INCLUDE_HPUX_SOURCE */

#endif /* _SYS_VFS_INCLUDED */
/* $Header: in.h,v 1.20.61.3 92/03/02 07:38:54 indnetwk Exp $ */

#ifndef	_SYS_IN_INCLUDED
#define	_SYS_IN_INCLUDED
/*
 * Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 *	@(#)in.h	7.6 (Berkeley) 6/29/88 plus MULTICAST 1.1
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
#define	IPPROTO_IGMP		2		/* group mgmt protocol */
#define	IPPROTO_GGP		3		/* gateway^2 (deprecated) */
#define	IPPROTO_TCP		6		/* tcp */
#define	IPPROTO_EGP		8		/* exterior gateway protocol */
#define	IPPROTO_IGP		9		/* interior gateway protocol */
#define	IPPROTO_PUP		12		/* pup */
#define	IPPROTO_UDP		17		/* user datagram protocol */
#define	IPPROTO_IDP		22		/* xns idp */
#define	IPPROTO_OSPF		89		/* open SPF routing protocol */
#define	IPPROTO_PXP		241		/* Netipc PXP protocol */

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
#define	IN_CLASSA(i)		(((u_long)(i) & ((u_long)0x80000000)) == 0)
#define	IN_CLASSA_NET		((u_long)0xff000000)
#define	IN_CLASSA_NSHIFT	24
#define	IN_CLASSA_HOST		((u_long)0x00ffffff)
#define	IN_CLASSA_MAX		128

#define	IN_CLASSB(i)		(((u_long)(i) & ((u_long)0xc0000000)) == \
					((u_long)0x80000000))
#define	IN_CLASSB_NET		((u_long)0xffff0000)
#define	IN_CLASSB_NSHIFT	16
#define	IN_CLASSB_HOST		((u_long)0x0000ffff)
#define	IN_CLASSB_MAX		65536

#define	IN_CLASSC(i)		(((u_long)(i) & ((u_long)0xe0000000)) == \
				  ((u_long)0xc0000000))
#define	IN_CLASSC_NET		((u_long)0xffffff00)
#define	IN_CLASSC_NSHIFT	8
#define	IN_CLASSC_HOST		((u_long)0x000000ff)

#define	IN_CLASSD(i)		(((u_long)(i) & ((u_long)0xf0000000)) == \
				  ((u_long)0xe0000000))
#define	IN_CLASSD_NET		((u_long)0xf0000000)/* These aren't really    */
#define	IN_CLASSD_NSHIFT	28		    /* net and host fields,but*/
#define	IN_CLASSD_HOST		((u_long)0x0fffffff)/* routing needn't know.  */
#define	IN_MULTICAST(i)		IN_CLASSD(i)

#define	IN_EXPERIMENTAL(i)	(((u_long)(i) & ((u_long)0xe0000000)) == \
				  ((u_long)0xe0000000))
#define	IN_BADCLASS(i)		(((u_long)(i) & ((u_long)0xf0000000)) == \
				  ((u_long)0xf0000000))

#define	INADDR_ANY		((u_long)0x00000000)
#define	INADDR_BROADCAST	((u_long)0xffffffff)	/* must be masked */
#define	INADDR_LOOPBACK		((u_long)0x7f000001)

#define	INADDR_UNSPEC_GROUP	((u_long)0xe0000000)	/* 224.0.0.0   */
#define	INADDR_ALLHOSTS_GROUP	((u_long)0xe0000001)	/* 224.0.0.1   */
#define	INADDR_MAX_LOCAL_GROUP 	((u_long)0xe00000ff)	/* 224.0.0.255 */

#ifndef _KERNEL
#define	INADDR_NONE		((u_long)0xffffffff)	/* -1 return */
#endif

#define	IN_LOOPBACKNET		127			/* official! */

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
 * Macros for number representation conversion.
 */
#ifndef ntohl
#define	ntohl(x)	(x)
#define	ntohs(x)	(x)
#define	htonl(x)	(x)
#define	htons(x)	(x)
#endif

/*
 * Options for use with [gs]etsockopt at the IP level.
 */
#define	IP_OPTIONS		1	/* set/get IP per-packet options   */
#define	IP_MULTICAST_IF		2	/* set/get IP multicast interface  */
#define	IP_MULTICAST_TTL	3	/* set/get IP multicast timetolive */
#define	IP_MULTICAST_LOOP	4	/* set/get IP multicast loopback   */
#define	IP_ADD_MEMBERSHIP	5	/* add  an IP group membership     */
#define	IP_DROP_MEMBERSHIP	6	/* drop an IP group membership     */
#define	IP_TTL			16      /* set/get socket's ttl */

#define	IP_DEFAULT_MULTICAST_TTL   1	/* normally limit m'casts to 1 hop  */
#define	IP_DEFAULT_MULTICAST_LOOP  1	/* normally hear sends if a member  */
#define	IP_MAX_MEMBERSHIPS         20	/* per socket; must fit in one mbuf */

/*
 * Argument structure for IP_ADD_MEMBERSHIP and IP_DROP_MEMBERSHIP.
 */
struct ip_mreq {
	struct in_addr	imr_multiaddr;	/* IP multicast address of group */
	struct in_addr	imr_interface;	/* local IP address of interface */
};

#ifdef _KERNEL
extern	struct domain inetdomain;
extern	struct protosw inetsw[];
struct	in_addr in_makeaddr();
u_long	in_netof(), in_lnaof();

#define IN_MAXLINKHDR	32
#endif	/* _KERNEL */

#endif	/* _SYS_IN_INCLUDED */
/*
 * Copyright (c) 1980, 1983, 1988 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 *	@(#)netdb.h	5.10 (Berkeley) 6/27/88
 *	@(#)$Revision: 1.8.109.1 $
 */

/*
 * Structures returned by network
 * data base library.  All addresses
 * are supplied in host order, and
 * returned in network order (suitable
 * for use in system calls).
 */

#ifndef _NETDB_INCLUDED /* allow multiple inclusions */
#define _NETDB_INCLUDED

#include <sys/stdsyms.h>

#ifdef _INCLUDE_HPUX_SOURCE

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

#ifdef _PROTOTYPES
#  ifdef __cplusplus
     extern "C" {
#  endif
        extern struct hostent *gethostbyname(const char *);
        extern struct hostent *gethostbyaddr(const char *, int, int);
        extern struct hostent *gethostent(void);
        extern int sethostent(int);
        extern int endhostent(void);
        extern struct netent *getnetbyname(const char *);
        extern struct netent *getnetbyaddr(int, int);
        extern struct netent *getnetent(void);
        extern int setnetent(int);
        extern int endnetent(void);
        extern struct servent *getservbyname(const char *, const char *);
        extern struct servent *getservbyport(int, const char *);
        extern struct servent *getservent(void);
        extern int setservent(int);
        extern int endservent(void);
        extern struct protoent *getprotobyname(const char *);
        extern struct protoent *getprotobynumber(int);
        extern struct protoent *getprotoent(void);
        extern int setprotoent(int);
        extern int endprotoent(void);
        extern struct rpcent *getrpcent(void);
        extern struct rpcent *getrpcbyname(const char *);
        extern struct rpcent *getrpcbynumber(int);
        extern int setrpcent(int);
        extern int endrpcent(void);
#  ifdef __cplusplus
     }
#  endif
#else /* not _PROTOTYPES */
        extern struct hostent *gethostbyname();
        extern struct hostent *gethostbyaddr();
        extern struct hostent *gethostent();
        extern int sethostent();
        extern int endhostent();
        extern struct netent *getnetbyname();
        extern struct netent *getnetbyaddr();
        extern struct netent *getnetent();
        extern int setnetent();
        extern int endnetent();
        extern struct servent *getservbyname();
        extern struct servent *getservbyport();
        extern struct servent *getservent();
        extern int setservent();
        extern int endservent();
        extern struct protoent *getprotobyname();
        extern struct protoent *getprotobynumber();
        extern struct protoent *getprotoent();
        extern int setprotoent();
        extern int endprotoent();
        extern struct rpcent *getrpcent();
        extern struct rpcent *getrpcbyname();
        extern struct rpcent *getrpcbynumber();
        extern int setrpcent();
        extern int endrpcent();
#endif /* not _PROTOTYPES */

/*
 * Error return codes from gethostbyname() and gethostbyaddr()
 * (left in extern int h_errno).
 */

#define	HOST_NOT_FOUND	1 /* Authoritative Answer Host not found */
#define	TRY_AGAIN	2 /* Non-Authoritive Host not found, or SERVERFAIL */
#define	NO_RECOVERY	3 /* Non recoverable errors, FORMERR, REFUSED, NOTIMP */
#define	NO_DATA		4 /* Valid name, no data record of requested type */
#define	NO_ADDRESS	NO_DATA		/* no address, look for MX record */

#endif /* _INCLUDE_HPUX_SOURCE */

#endif /* _NETDB_INCLUDED */
/*
 * @(#)un.h: $Revision: 1.10.61.2 $ $Date: 91/06/19 13:48:01 $
 * $Locker: indnetwk $
 */

#ifndef	_SYS_UN_INCLUDED
#define	_SYS_UN_INCLUDED

/* 
 * Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 *	@(#)un.h	7.3 (Berkeley) 6/27/88
 */

/*
 * Definitions for UNIX IPC domain.
 */
struct	sockaddr_un {
	short	sun_family;		/* AF_UNIX */
	char	sun_path[92];		/* path name (gag) */
};

#ifdef _KERNEL
int	unp_discard();
#endif
#endif	/* _SYS_UN_INCLUDED */
/* @(#) $Revision: 66.1 $ */      
#ifndef _DIRENT_INCLUDED
#define _DIRENT_INCLUDED

#ifndef _SYS_STDSYMS_INCLUDED
#  include <sys/stdsyms.h>
#endif /* _SYS_STDSYMS_INCLUDED */

#include <sys/dirent.h>

#endif /*_DIRENT_INCLUDED*/
/* @(#) $Revision: 70.1 $ */
#ifndef _PWD_INCLUDED /* allow multiple inclusions */
#define _PWD_INCLUDED

#ifndef _SYS_STDSYMS_INCLUDED
#  include <sys/stdsyms.h>
#endif /* _SYS_STDSYMS_INCLUDED */

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _INCLUDE_POSIX_SOURCE

#  ifndef _GID_T
#    define _GID_T
     typedef long gid_t;
#  endif /* _GID_T */

#  ifndef _UID_T
#    define _UID_T
     typedef long uid_t;
#  endif /* _UID_T */

   struct passwd {
	char	*pw_name;
	char 	*pw_passwd;
#ifdef _CLASSIC_ID_TYPES
	int	pw_uid;
	int	pw_gid;
#else
	uid_t	pw_uid;
	gid_t	pw_gid;
#endif
	char 	*pw_age;
	char	*pw_comment;
	char	*pw_gecos;
	char	*pw_dir;
	char	*pw_shell;
	long	pw_audid;
	int	pw_audflg;
   };

#  if defined(__STDC__) || defined(__cplusplus)
     extern struct passwd *getpwuid(uid_t);
     extern struct passwd *getpwnam(const char *);
#  else /* __STDC__ || __cplusplus */
     extern struct passwd *getpwuid();
     extern struct passwd *getpwnam();
#  endif /* __STDC__ || __cplusplus */
#endif /* _INCLUDE_POSIX_SOURCE */

#ifdef _INCLUDE_HPUX_SOURCE
   struct s_passwd {
       char    *pw_name;
       char    *pw_passwd;
       char    *pw_age;
       long     pw_audid;
       int     pw_audflg;
   };


#  if defined(__STDC__) || defined(__cplusplus)
#    ifndef _STDIO_INCLUDED
#      include <stdio.h>
#    endif /* _STDIO_INCLUDED */

     extern void setpwent(void);
     extern void endpwent(void);
     extern struct passwd *getpwent(void);
     extern struct passwd *fgetpwent(FILE *);
     extern struct s_passwd *getspwent(void);
     extern struct s_passwd *getspwuid(int);
     extern struct s_passwd *getspwaid(int);
     extern struct s_passwd *getspwnam(char *);
     extern struct s_passwd *fgetspwent(FILE *);
#  else /* __STDC__ || __cplusplus */
     extern void setpwent();
     extern void endpwent();
     extern struct passwd *getpwent();
     extern struct passwd *fgetpwent();
     extern struct s_passwd *getspwent();
     extern struct s_passwd *getspwuid();
     extern struct s_passwd *getspwaid();
     extern struct s_passwd *getspwnam();
     extern struct s_passwd *fgetspwent();
#  endif /* __STDC__ || __cplusplus */

#  ifndef UID_NOBODY		/* Uid of NFS "nobody". */
#     define UID_NOBODY ((unsigned short) 0xfffe)
#  endif
#endif /* _INCLUDE_HPUX_SOURCE */

#ifdef __cplusplus
}
#endif

#endif /* _PWD_INCLUDED */
/* $Header: wait.h,v 1.17.61.4 92/04/19 19:38:14 smp Exp $ */

#ifndef _SYS_WAIT_INCLUDED
#define _SYS_WAIT_INCLUDED

/*
 * This file holds definitions relevent to the three system calls
 * wait, waitpid, and wait3.
 */

#ifdef _KERNEL_BUILD
#include "../h/stdsyms.h"
#else  /* ! _KERNEL_BUILD */
#include <sys/stdsyms.h>
#endif /* _KERNEL_BUILD */

#ifdef _INCLUDE_POSIX_SOURCE

#  ifndef _PID_T
#    define _PID_T
     typedef long pid_t;
#  endif /* _PID_T */

    /*
     * Option bits for the waitpid and wait3 calls.  WNOHANG causes the
     * wait to not hang if there are no stopped or terminated processes, rather
     * returning an error indication in this case (pid==0).  WUNTRACED
     * indicates that the caller should receive status about untraced children
     * which stop due to signals.  If children are stopped and a wait without
     * this option is done, it is as though they were still running... nothing
     * about them is returned.
     */
#  define WNOHANG		1	/* don't hang in wait */
#  define WUNTRACED	2	/* tell about stopped, untraced children */

#  define _WAITMASK	(WNOHANG | WUNTRACED)	/* all legal flags */

#  define WIFEXITED(_X)		(((int)(_X)&0377)==0)
#  define WIFSTOPPED(_X)	(((int)(_X)&0377)==0177)
#  define WIFSIGNALED(_X)	((((int)(_X)&0377)!=0)&&(((_X)&0377)!=0177))
#  define WEXITSTATUS(_X)	(((int)(_X)>>8)&0377)
#  define WTERMSIG(_X)		((int)(_X)&0177)
#  define WSTOPSIG(_X)		(((int)(_X)>>8)&0377)

#  ifndef _KERNEL
#  ifdef __cplusplus
    extern "C" {
#  endif /* __cplusplus */

#  ifdef _CLASSIC_POSIX_TYPES
     extern int wait();
     extern int waitpid();
#  else /* not _CLASSIC_POSIX_TYPES */
#    ifdef _PROTOTYPES
       extern pid_t wait(int *);
       extern pid_t waitpid(pid_t, int *, int);
#    else /* not _PROTOTYPES */
       extern pid_t wait();
       extern pid_t waitpid();
#    endif /* not _PROTOTYPES */
#  endif /* not _CLASSIC_POSIX_TYPES */

#  ifdef __cplusplus
    }
#  endif /* __cplusplus */
#  endif /* not _KERNEL */

#endif /* _INCLUDE_POSIX_SOURCE */


#if defined (_INCLUDE_HPUX_SOURCE) || defined (_BSD)

#  ifndef _KERNEL
#    ifdef __cplusplus
      extern "C" {
#    endif /* __cplusplus */
#    ifdef _CLASSIC_ID_TYPES
       extern int wait3();
#    else /* not _CLASSIC_ID_TYPES */
#      ifdef _PROTOTYPES
         extern pid_t wait3(int *, int, int *);
#      else /* not _PROTOTYPES */
         extern pid_t wait3();
#      endif /* not _PROTOTYPES */
#    endif /* not _CLASSIC_ID_TYPES */

#    ifdef __cplusplus
      }
#    endif /* __cplusplus */
#  endif /* not _KERNEL */

/*
 * Structure of the information in the first word returned by both
 * wait and wait3.  If w_stopval==WSTOPPED, then the second structure
 * describes the information returned, else the first.  See WUNTRACED below.
 */
   union wait	{
	int	w_status;		/* used in syscall */
	/*
	 * Terminated process status.
	 */
	struct {
#  ifdef vax
		unsigned short	w_Termsig:7;	/* termination signal */
		unsigned short	w_Coredump:1;	/* core dump indicator */
		unsigned short	w_Retcode:8;	/* exit code if w_termsig==0 */
#  else	/* not vax */
		unsigned short	w_pad;		/* pad to low order 16 bits */
		unsigned short	w_Retcode:8;	/* exit code if w_termsig==0 */
		unsigned short	w_Coredump:1;	/* core dump indicator */
		unsigned short	w_Termsig:7;	/* termination signal */
#  endif /* else not vax */
	} w_T;
	/*
	 * Stopped process status.  Returned
	 * only for traced children unless requested
	 * with the WUNTRACED option bit.
	 */
	struct {
#  ifdef vax
		unsigned short	w_Stopval:8;	/* == W_STOPPED if stopped */
		unsigned short	w_Stopsig:8;	/* signal that stopped us */
#  else	/* not vax */
		unsigned short	w_pad;		/* pad to low order 16 bits */
		unsigned short	w_Stopsig:8;	/* signal that stopped us */
		unsigned short	w_Stopval:8;	/* == W_STOPPED if stopped */
#  endif /* else not vax */
	} w_S;
   };
#  define w_termsig	w_T.w_Termsig
#  define w_coredump	w_T.w_Coredump
#  define w_retcode	w_T.w_Retcode
#  define w_stopval	w_S.w_Stopval
#  define w_stopsig	w_S.w_Stopsig

#  define WSTOPPED	0177	/* value of s.stopval if process is stopped */

#  define WCOREDUMP(_X)	((int)(_X)&0200)
#endif	/* _INCLUDE_HPUX_SOURCE || _BSD */

#ifdef _BSD
#  undef  WIFEXITED
#  define WIFEXITED(_X)		((_X).w_stopval != WSTOPPED && (_X).w_termsig == 0)
#  undef  WIFSTOPPED
#  define WIFSTOPPED(_X)	((_X).w_stopval == WSTOPPED)
#  undef  WIFSIGNALED
#  define WIFSIGNALED(_X)	((_X).w_stopval != WSTOPPED && (_X).w_termsig != 0)
#  undef  WEXITSTATUS
#  define WEXITSTATUS(_X) 	((_X).w_retcode)
#  undef  WTERMSIG
#  define WTERMSIG(_X)    	((_X).w_termsig)
#  undef  WCOREDUMP
#  define WCOREDUMP(_X)   	((_X).w_coredump)
#  undef  WSTOPSIG
#  define WSTOPSIG(_X)    	((_X).w_stopsig)
#endif  /* _BSD */

#endif	/* _SYS_WAIT_INCLUDED */
/* @(#) $Revision: 64.3 $ */
#ifndef _TERMIOS_INCLUDED
#define _TERMIOS_INCLUDED

#    include <sys/termio.h>

#endif /* _TERMIOS_INCLUDED */
/* @(#) $Revision: 70.8 $ */
#ifndef _LIMITS_INCLUDED
#define _LIMITS_INCLUDED

#ifndef _SYS_STDSYMS_INCLUDED
#  include <sys/stdsyms.h>
#endif /* _SYS_STDSYMS_INCLUDED */

#ifdef _INCLUDE__STDC__
#  define CHAR_BIT	8	    /* Number of bits in a char */
#  define CHAR_MAX	127	    /* Max integer value of a char */
#  define CHAR_MIN      (-128)      /* Min integer value of a char */
#  define MB_LEN_MAX	2	    /* Max bytes in a multibyte character */
#  define INT_MAX	2147483647  /* Max decimal value of an int */
#  define INT_MIN	(-2147483647 - 1)  /* Min decimal value of an int */
#  define LONG_MAX	2147483647L 	   /* Max decimal value of a long */
#  define LONG_MIN	(-2147483647L - 1) /* Min decimal value of a long */
#  define SCHAR_MAX     127        /* max value of a signed char */
#  define SCHAR_MIN     (-128)     /* Min value of a signed char */
#  define SHRT_MAX      32767      /* max decimal value of a short */
#  define SHRT_MIN      (-32768)   /* Min decimal value of a short */
#  define UCHAR_MAX     255        /* max value of unsigned char */
#  if defined(__STDC__) || defined(__cplusplus)
#    define UINT_MAX	4294967295U  /* max value of an unsigned integer */
#    define ULONG_MAX	4294967295UL /* max value of a unsigned long int */
#  else /* not __STDC__ || __cplusplus */
#    define UINT_MAX	4294967295  /* max value of an unsigned integer */
#    define ULONG_MAX	4294967295  /* max value of a unsigned long int */
#  endif /* else not __STDC__ || __cplusplus */
#  define USHRT_MAX     65535      /* max value of a unsigned short int */
#endif /* _INCLUDE__STDC__ */

#if defined(_INCLUDE_POSIX_SOURCE) || defined(_INCLUDE_POSIX2_SOURCE)
#  define _POSIX_ARG_MAX  	4096	/* The length of the arguments for 
					   one of the exec functions in bytes, 
					   including environment data. */
#  define _POSIX_CHILD_MAX	6	/* The number of simultaneous
					   processes per real user ID */
#  define _POSIX_LINK_MAX	8	/* The value of a files link count */
#  define _POSIX_MAX_CANON  	255     /* Max number of bytes in a terminal 
					   canonical input line */
#  define _POSIX_MAX_INPUT	255	/* The number of bytes for which space 
					   will be available in a terminal 
					   input queue */
#  define _POSIX_NAME_MAX	14	/* The number of bytes in a filename */
#  define _POSIX_NGROUPS_MAX	0	/* The number of simultaneous 
					   supplementary group IDs per
					   process. */
#  define _POSIX_OPEN_MAX	16	/* The number of files that one process
					   can have open at one time */
#  define _POSIX_PATH_MAX	255 	/* The number of bytes in a pathname */
#  define _POSIX_PIPE_BUF	512	/* The number of bytes that can be 
					   written atomically when writing to 
					   a pipe. */
#  define _POSIX_SSIZE_MAX	32767	/* The value that can be stored in
					   an object of type ssize_t */
#  define _POSIX_TZNAME_MAX	3	/* The maximum number of bytes
					   supported for the name of a time
					   zone (not of the TZ variable). */
#  define _POSIX_STREAM_MAX	8	/* The number of streams that one 
					   process can have open at one
					   time */

/*
 * The following limit is also available using the sysconf() function.
 * Use of sysconf() is advised over use of the constant value defined
 * here, since it should pose fewer portability and forward-compatability
 * problems.
 */
#  define NGROUPS_MAX		20	/* Maximum number of simultaneous
					   supplementary group IDs per 
					   process */

#  if !defined(_POSIX1_1988) && !defined(_XPG3)
#    define SSIZE_MAX		INT_MAX /* The maximum value that can be
					   stored in an object of type 
					   ssize_t */

/*
 * The following limit is also available using the sysconf() function.
 * Use of sysconf() is advised over use of the constant value defined
 * here, since it should pose fewer portability and forward-compatability
 * problems.
 */
#    define TZNAME_MAX		19	/* Maximum number of bytes
					   supported for the name of the 
					   time zone (not of the TZ 
					   variable). */
#  endif /* !defined(_POSIX1_1988) && !defined(_XPG3) */

#endif /* defined(_INCLUDE_POSIX_SOURCE) || defined(_INCLUDE_POSIX2_SOURCE) */

/* the following limits are from POSIX 1003.2 (draft 8) section 2.14 */
#if defined ( _INCLUDE_POSIX2_SOURCE ) || defined ( _XPG4 )

#  define BC_BASE_MAX		99	/* largest ibase and obase for bc */
#  define BC_DIM_MAX		2048	/* max no. of elems in bc array */
#  define BC_SCALE_MAX		99	/* maximum scale value for bc */
#  define BC_STRING_MAX		1000 	/* max length of string constant 
				  	   accepted by bc */
#  define COLL_WEIGHTS_MAX	2	/* max number of weights that can be 
					   assigned to an entry of the 
					   LC_COLLATE order keyword in the 
					   locale definition file   */
#  define EXPR_NEST_MAX		32	/* max no. of expressions nested 
					   within parentheses in expr */
#  define LINE_MAX		2048	/* Expected length in bytes of a 
					   utility's input line when input
					   is from text files */
#  define RE_DUP_MAX		255	/* Max no. of repeated occurrences
					   of an RE when using \{m,n\}
					   notation */

#  define _POSIX2_BC_BASE_MAX	99	/* The maximum ibase and obase values
				   	   allowed by the bc utility */
#  define _POSIX2_BC_DIM_MAX	2048	/* The maximum number of elements 
					   permitted in an array by the bc 
					   utility. */
#  define _POSIX2_BC_SCALE_MAX	99	/* The maximum scale value allowed 
					   by the bc utility. */
#  define _POSIX2_BC_STRING_MAX 1000	/* The maximum length of a string 
					   constant accepted by the bc 
					   utility. */
#  define _POSIX2_COLL_WEIGHTS_MAX 2	/* The maximum number of weights that 
					   can be assigned to an entry of the 
					   LC_COLLATE order keyword in the 
					   locale definition file. */
#  define _POSIX2_EXPR_NEST_MAX	32	/* The maximum number of expressions 
					   that can be nested within 
					   parentheses by the expr utility. */
#  define _POSIX2_LINE_MAX	2048	/* Unless otherwise noted, the maximum 
					   length, in bytes, of a utility's 
					   input line (either standard input or 
					   another file), when the utility is 
					   described as processing text files.
					   The length includes room for the 
					   trailing newline. */
# define _POSIX2_RE_DUP_MAX	255	/* The maximum number of repeated 
					   occurrences of a regular expression 
					   permitted when using the interval 
					   notation \{m,n\}. */

#endif /* defined ( _INCLUDE_POSIX2_SOURCE ) || defined ( _XPG4 ) */


#ifdef _INCLUDE_XOPEN_SOURCE
#  if !defined (_AES_SOURCE) && !defined (_XPG4)
#     ifndef DBL_MIN
#       define DBL_MIN   2.2250738585072014e-308   /* Min decimal value of a 
						      double */
#     endif /* DBL_MIN */

#     ifndef FLT_MIN 
#       define FLT_MIN   1.17549435e-38	/* Min decimal value of a float */
#     endif /* FLT_MIN */
#  endif  /* !defined (_AES_SOURCE) && !defined (_XPG4) */

#  ifndef _AES_SOURCE
#     ifndef DBL_DIG
#       define DBL_DIG   15	/* Digits of precision of a double */  
#     endif /* DBL_DIG */

#     ifndef DBL_MAX
#       define DBL_MAX   1.7976931348623157e+308   /* Max decimal value of a 
						      double */
#     endif /* DBL_MAX */

#     ifndef FLT_DIG
#       define FLT_DIG   6		/* Digits of precision of a float */  
#     endif /* FLT_DIG */

#     ifndef FLT_MAX
#       define FLT_MAX   3.40282347e+38	/* Max decimal value of a float */
#     endif /* FLT_MAX */

#     ifndef TMP_MAX
#       define TMP_MAX     17576  /* min number of unique names generated by 
				     tmpnam() */
#     endif /* TMP_MAX */
#  endif /* _AES_SOURCE */


#  define LONG_BIT    32	/* Number of bits in a long */
#  define WORD_BIT    32	/* number of bits in a "word" (int) */
#  define NL_LANGMAX  (3*_POSIX_NAME_MAX+2)  /* Max number of bytes in a LANG name */
#  define NL_ARGMAX   9     	/* max value of "digit" in calls to the NLS
				   printf() and scanf() functions */

/* message catologue limits */
#  define NL_MSGMAX     65534 /* max message number */
#  define NL_NMAX       2	/* max number of bytes in N-to-1 mapping 
				   characters */
#  define NL_SETMAX     255   /* max set number */
#  define NL_TEXTMAX    8192  /* max number of bytes in a message string */
#  define NZERO		20	/* default process priority */
#  define PASS_MAX      8     /* max number of significant characters in a 
				   password (not including terminating null) */

#  define _SYS_NMLN	9	/* length of strings returned by
				   uname(OS) */

#  ifdef _XPG2
/*
 * The following limts are not actually invariant, but are configurable.
 * The correct values can be determined using the sysconf() function.
 * The default values are provided here because the constants are specified
 * by several publications including XPG (X/Open Portability Guide) Issue 2
 * and SVID (System V Interface Definition) Issue 2.
 */
#    define ARG_MAX		20478	/* Maximum length of arguments for the 
					   exec function in bytes, including 
					   environment data */
#    define CHILD_MAX	   	25	/* Maximum number of simultaneous
					   processes per real user ID */
#    define OPEN_MAX	   	60	/* Maximum number of files that one 
					   process can have open at any given 
					   time */

/*
 * The following limits are not actually invariant, but can vary by file
 * system or device.  the correct values can be determined using the 
 * pathconf() function.  The default values are provided here because the
 * constants are specified by several publications including XPG Issue 2
 * and SVID Issue 2.
 */
#    define LINK_MAX   	32767	/* Max number of links to a single file */
#    define MAX_CANON  	512     /* Max number of bytes in a terminal canonical 
			   	   input line */
#    define MAX_INPUT  	512 	/* Max number of bytes allowed in a terminal 
				   input queue */ 
#    define NAME_MAX   	14	/* Max number of characters in a filename
				   (not including terminating null) */
#    define PATH_MAX   	1023	/* max number of characters in a pathname (not 
				   including terminating null) */
#    define PIPE_BUF   	8192	/* max number bytes that is guaranteed
				   to be atomic when writing to a pipe */ 

/*
 * The following limits are not actually invariant, but are configurable.
 * The values are not generally useful for portable applications due to
 * the system-wide nature of these limits.  The default values are provided
 * here because the constants are specified by several publications
 * including XPG Issue 2 and SVID Issue 2.
 */
#    define LOCK_MAX	32	/* max number of entries in system lock table */
#    define PROC_MAX	84	/* max number of simultaneous processes on 
				   system */
#    define SYS_OPEN	120	/* max number of files open on system */

/* 
 * The following limits are obsolescent.  Some are not generally
 * useful for portable applications.  Some have been replaced by 
 * limits with different names defined by industry standards.  Some
 * have values that can vary.  The values (defaults for variable limits)
 * are provided here because the constants are specified by several 
 * publications including XPG Issue 2 and SVID Issue 2.
 */
#    define FCHR_MAX	INT_MAX		/*max file offset in bytes */
#    define MAX_CHAR	MAX_INPUT	/* max size of a character input 
					   buffer */
#    define SYS_NMLN	_SYS_NMLN	/* length of strings returned by
					   uname(OS) */
#    define SYSPID_MAX	4		/* max pid of system processes */
#    define USI_MAX	UINT_MAX	/* max decimal value of an unsigned 
					   int */
#  endif /* _XPG2 */

#endif /* _INCLUDE_XOPEN_SOURCE */


#ifdef _INCLUDE_AES_SOURCE
#  define ATEXIT_MAX	32	/* Max # of functions that can be called by
				   atexit() */
#endif /* _INCLUDE_AES_SOURCE */



#ifdef _INCLUDE_HPUX_SOURCE

#  include <sys/param.h>

#  define UCHAR_MIN   0       	/* Min value for unsigned char */

#  ifndef CLK_TCK
#    ifdef __hp9000s300
#      define CLK_TCK  50	/* Number of clock ticks per second */
#    endif /* __hp9000s300 */
#    ifdef __hp9000s800
#      define CLK_TCK  100	/* Number of clock ticks per second */
#    endif /* __hp9000s800 */
#  endif /* CLK_TCK */

/* message catologue limits */
#  define NL_SAFEFD	3	/* smallest available file no. */
#  define STD_BLK       512	/* number of bytes in a physical I/O block */

/* constants for nl_ascxtime(), et al. */
#  define NL_MAXDATE	200	/* maximum length of a time/date string */

/* constants for nl_init() */
#  define NLSDIR "/usr/lib/nls/"	/* root of language directories */
#  define MAX_INFO_MSGS	80	/* max number of nl_langinfo messages */
#  define MAX_DIGIT_ALT	35	/* max alt digit string(17 char) + NULL */
#  define MAX_PUNCT_ALT	67	/* max alt punct string(33 char) + NULL */
#  define MAX_ERA_FMTS	16	/* max number of era formats */

/* constants for nl_langinfo() and buildlang(1) */
#  define LEN_INFO_MSGS	80	/* max length of any one message */
#  define LEN_ERA_FMTS	40	/* max length of any one era format */

/* constants for the language configuration file: */
#  define NL_WHICHLANGS "/usr/lib/nls/config"	/* where its located */
#  define NL_SPECMAX	96
#  define MAXLNAMELEN 	14	/* max length of a language name */


/* constants for setlocale() and buildlang() */
#define _LC_ALL_SIZE		256	/* max size of LC_ALL data area */
#define _LC_COLLATE_SIZE       1024	/* max size of LC_COLLATE data area */

#define _LC_CTYPE_SIZE	       2456	/* max size of LC_CTYPE data area */

#define _LC_MONETARY_SIZE	128	/* max size of LC_MONETARY data area */
#define _LC_NUMERIC_SIZE	128	/* max size of LC_NUMERIC data area */
#define _LC_TIME_SIZE		912	/* max size of LC_TIME data area */
#define _LC_MESSAGES_SIZE	256     /* max size of LC_MESSAGES area */
/*
 * misc. constants
 */
#  define PID_MAX	MAXPID		/* max value for a process id */
#  define PIPE_MAX	INT_MAX		/* max number of bytes writable to a
					   pipe in one write */
#  define UID_MAX	MAXUID		/* smallest unattainable value for a 
					   user or group ID */

#endif /* _INCLUDE_HPUX_SOURCE */

#endif /* _LIMITS_INCLUDED */
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

#if !(i486V4 || SUN4 || RS6000 || HP9000)
#ifndef __clock_t
#define __clock_t 1
typedef long clock_t;
#endif
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
/*
** Localisation header file for the HP9000
*/

	/* HP9000 Unix has a logname routine */
#define logname my_logname

	/* These ioctl calls have the Posix names */
#define TCGETS   TCGETATTR
#define TCSETS   TCSETATTR
#define TCSETSW  TCSETATTRD

#define statfs(a, b, c, d) statfs(a, b)

#define seteuid(a) setresuid(-1, a, -1)

#define getdtablesize() 60

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

/* RcsId: $Id: defines.h,v 1.18 1993/08/12 14:24:15 bart Exp $ */
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

#ifdef SCOUNIX
#undef SCOUNIX
#define SCOUNIX 1
#define UNIX 1
#define USE_sendto
#define USE_recvfrom
#else
#define SCOUNIX 0
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

#ifdef RS6000
#undef RS6000
#define RS6000 1
#define UNIX 1
#else
#define RS6000 0
#endif

#ifdef HP9000
#undef HP9000
#define HP9000 1
#define UNIX 1
#else
#define HP9000 0
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

#ifdef ETC_DIR
#undef ETC_DIR
#define ETC_DIR 1
#else
#define ETC_DIR	0
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
#define print_graphics		     0
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
#if (SUN3 || SUN4 || SM90 || TR5 || RS6000 || HP9000)
#define swapping_needed              1
#else
#define swapping_needed              0
#endif

#if (SUN3 || SUN4 || RS6000 || HP9000 || SCOUNIX)
#define use_separate_windows         1
#else
#define use_separate_windows         0
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

#if (RS6000 || HP9000)
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
/* RcsId: $Id: protocol.h,v 1.9 1993/01/21 14:40:03 paul Exp $ */
/* Copyright (C) 1989, Perihelion Software Ltd.        			*/

/*------------------------------------------------------------------------
--                                                                      --
-- helios.h                                                             --
--                                                                      --
------------------------------------------------------------------------*/

/* standard type definitions */

#if TR5
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
#define Proto_ReSync		  0x7f
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
/* RcsId: $Id: barthdr,v 1.4 1992/10/09 12:20:43 martyn Exp $ */
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
--             Copyright (C) 1987, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
--      Debugopt.h                                                      --
--                                                                      --
--  Author:  BLV 10/12/87                                               --
--                                                                      --
------------------------------------------------------------------------*/
/* RcsId: $Id: debugopt.h,v 1.3 1993/08/12 14:24:15 bart Exp $ */
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
#define DDE_Flag               0x2000000L
/**
*** All_Debug_Flags is a mask for all the debugging options except the
*** one-off ones : memory, log, reconfigure, nopop, listall.
*** It is used for -a etc.
**/
#define All_Debug_Flags       0x033EBFBFL
              
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
/* RcsId: $Id: structs.h,v 1.6 1993/03/23 14:27:10 bart Exp $ */
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
        word	last_send;
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
*** word    : this link always transfers a word multiple rather than a
***           byte multiple.
**/
#define Link_flags_waiting        0x01
#define Link_flags_free           0x02
#define Link_flags_unused         0x04
#if (UNIX)
#define Link_flags_not_selectable 0x010000  
#define Link_flags_uninitialised  0x020000
#define Link_flags_firsttime      0x040000
#define Link_flags_datareadysent  0x080000
#define Link_flags_word           0x100000
#define Link_flags_messagemode    0x200000
#define Link_flags_halfduplex     0x400000
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
#define Debug_Exit	 0x1234560A

#endif /* UNIX */

#define Mode_Normal      1
#define Mode_Auxiliary   2
#define Mode_Subordinate 3
#define Mode_Remote      4
#define Mode_Daemon      5

#if (UNIX)
/*
** BLV - for Hydra to support C40 boards a significant amount of work has
** been needed. Previously Hydra had a full understanding of the protocols
** used between the root processor and the I/O Server, and knew when the
** processor was reset, booting, or running. This was fine for a single
** bootstrap mechanism but not for multiple bootstrap mechanisms - the
** code would get far too convoluted. Therefore I have replaced this
** approach with a system whereby Hydra can always accept some data from
** either the root processor or the I/O Server, and there is a
** higher level protocol.
**
** By default Hydra will check every link for data from the root processor.
** If any data is present then Hydra will inform the I/O Server via a suitable
** message, but will not actually read the data. The I/O Server will be woken up
** inside its call to select() and will request the appropriate amount of data.
**
** The I/O Server can send the following requests:
**   Reset
**   Analyse
**   Write block of n bytes, followed immediately by the data
**   Read block of n bytes. This request is likely to come after Hydra
**   has indicated that data is ready, but may not. In particular during
**   a bootstrap the I/O Server may perform a read without checking that the
**   root processor has data ready.
**
** Great care has to be taken to cope sensibly with errors and the fact that
** the I/O Server and Hydra can get out of sync, for example the I/O Server could
** send a Reset request at the same time that Hydra is sending a data ready
** message.
*/

typedef struct Hydra_Message {
	int	FnRc;
	union
	{
		int	Size;
		BYTE	Buf[4];
	} Extra;
} Hydra_Message;

#define Hydra_ResetRequest	0x01
#define Hydra_ResetAck		0x02
#define Hydra_AnalyseRequest	0x03
#define Hydra_AnalyseAck	0x04
#define Hydra_DataReadyByte	0x05
#define Hydra_DataReadyWord	0x06
#define Hydra_ReadRequest	0x07
#define Hydra_ReadAck		0x08
#define Hydra_WriteRequest	0x09
#define Hydra_WriteAck		0x0A
#define Hydra_Done		0x0B
#define Hydra_Broken		0x0C
#define Hydra_Nop		0x0D
#define Hydra_MessageMode	0x0E
#define Hydra_MessageAck	0x0F

	/* Special cases for exchanging bytes and words are worthwhile	*/
	/* because the data can fit into the Size field, and does not	*/
	/* require separate socket operations.				*/
#define Hydra_WriteByte		0x10
#define Hydra_WriteWord		0x11
#define Hydra_ReadByte		0x12
#define Hydra_ReadWord		0x13

#endif

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
/* RcsId: $Id: fundefs.h,v 1.10 1993/09/29 10:47:55 bart Exp $ */
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
#if (graphics)
#include "windows\graph.def"
#endif
#if (print_graphics)
#include "windows\grfprn.def"
#endif
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
#define Ether_InitServer                IgnoreVoid
#define Ether_TidyServer                IgnoreVoid
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
#define Ether_SetAttr                   Invalidfn_handler
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
PUBLIC void fn( ClearMultiwait, (word, ...));
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

#if (PC || SUN3 || SUN4 || ARMBSD)
/* VLSI PID support */
PUBLIC  void fn( vy86pid_set_baudrate,    (word));
PUBLIC  word fn( vy86pid_get_configbaud,  (void));
#endif

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
/* RcsId: $Id: server.h,v 1.11 1993/08/12 14:24:15 bart Exp $ */
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
/* blocking.								  */
word       C40HalfDuplex = FALSE;

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
#if (print_graphics)
#include "windows\grfprn.ser"
#endif
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
#if (print_graphics)
#include "windows\grfprn.str"
#endif
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
#if (print_graphics)
extern VoidFnPtr GrfPrn_Handlers[];
#endif
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
