/***
*stdio.h - definitions/declarations for standard I/O routines
*
*	Copyright (c) 1985-1992, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	This file defines the structures, values, macros, and functions
*	used by the level 2 I/O ("standard I/O") routines.
*	[ANSI/System V]
*
****/

#ifndef _INC_STDIO

#ifdef __cplusplus
extern "C" {
#endif

#if (_MSC_VER <= 600)
#define __cdecl     _cdecl
#define __far       _far
#define __near      _near
#endif

#ifndef _SIZE_T_DEFINED
typedef unsigned int size_t;
#define _SIZE_T_DEFINED
#endif

#ifndef _VA_LIST_DEFINED
typedef char *va_list;
#define _VA_LIST_DEFINED
#endif

/* buffered I/O macros */

#define BUFSIZ	512
#define _NFILE	20
#define EOF	(-1)

#ifndef _FILE_DEFINED
#pragma pack(2)
struct _iobuf {
	char *_ptr;
	int   _cnt;
	char *_base;
	char  _flag;
	char  _file;
	};
typedef struct _iobuf FILE;
#pragma pack()
#define _FILE_DEFINED
#endif


/* _P_tmpnam: Directory where temporary files may be created.
 * L_tmpnam size =  size of _P_tmpdir
 *	+ 1 (in case _P_tmpdir does not end in "\\")
 *	+ 6 (for the temp number string)
 *	+ 1 (for the null terminator)
 */

#define  _P_tmpdir "\\"
#define  L_tmpnam sizeof(_P_tmpdir)+8


/* fseek constants */

#define SEEK_CUR 1
#define SEEK_END 2
#define SEEK_SET 0


/* minimum guaranteed filename length, open file count, and unique
 * tmpnam filenames.
 */

#define FILENAME_MAX 63
#define FOPEN_MAX 18
#define TMP_MAX 32767
#define _SYS_OPEN 20


/* define NULL pointer value */

#ifndef NULL
#ifdef __cplusplus
#define NULL	0
#else
#define NULL	((void *)0)
#endif
#endif


/* declare _iob[] array */

#ifndef _STDIO_DEFINED
extern FILE __near __cdecl _iob[];
#endif


/* define file position type */

#ifndef _FPOS_T_DEFINED
typedef long fpos_t;
#define _FPOS_T_DEFINED
#endif


/* standard file pointers */

#ifndef _WINDLL
#define stdin  (&_iob[0])
#define stdout (&_iob[1])
#define stderr (&_iob[2])
#endif
#ifndef _WINDOWS
#define _stdaux (&_iob[3])
#define _stdprn (&_iob[4])
#endif


#define _IOREAD 	0x01
#define _IOWRT		0x02

#define _IOFBF		0x0
#define _IOLBF		0x40
#define _IONBF		0x04

#define _IOMYBUF	0x08
#define _IOEOF		0x10
#define _IOERR		0x20
#define _IOSTRG 	0x40
#define _IORW		0x80


#ifdef _WINDOWS
#ifndef _WINDLL
#ifndef _WINFO_DEFINED
/* interface version number */
#define _QWINVER	0

/* max number of windows */
#define _WFILE		20

/* values for windows screen buffer size */
#define _WINBUFINF	0
#define _WINBUFDEF	-1

/* size/move settings */
#define _WINSIZEMIN	1
#define _WINSIZEMAX	2
#define _WINSIZERESTORE 3
#define _WINSIZECHAR	4

/* size/move query types */
#define _WINMAXREQ	100
#define _WINCURRREQ	101

/* values for closing window */
#define _WINPERSIST	1
#define _WINNOPERSIST	0

/* pseudo file handle for frame window */
#define _WINFRAMEHAND	-1

/* menu items */
#define _WINSTATBAR	1
#define _WINTILE	2
#define _WINCASCADE	3
#define _WINARRANGE	4

/* quickwin exit options */
#define _WINEXITPROMPT		1
#define _WINEXITNOPERSIST	2
#define _WINEXITPERSIST 	3

/* open structure */
#pragma pack(2)
struct _wopeninfo {
	unsigned int _version;
	const char __far * _title;
	long _wbufsize;
	};
#pragma pack()

/* size/move structure */
struct _wsizeinfo {
	unsigned int _version;
	unsigned int _type;
	unsigned int _x;
	unsigned int _y;
	unsigned int _h;
	unsigned int _w;
	};
#define _WINFO_DEFINED
#endif
#endif
#endif

/* function prototypes */

#ifndef _STDIO_DEFINED
int __cdecl _filbuf(FILE *);
int __cdecl _flsbuf(int, FILE *);
FILE * __cdecl _fsopen(const char *,
	const char *, int);
void __cdecl clearerr(FILE *);
int __cdecl fclose(FILE *);
int __cdecl _fcloseall(void);
FILE * __cdecl _fdopen(int, const char *);
int __cdecl feof(FILE *);
int __cdecl ferror(FILE *);
int __cdecl fflush(FILE *);
int __cdecl fgetc(FILE *);
#ifndef _WINDLL
int __cdecl _fgetchar(void);
#endif
int __cdecl fgetpos(FILE *, fpos_t *);
char * __cdecl fgets(char *, int, FILE *);
int __cdecl _fileno(FILE *);
int __cdecl _flushall(void);
FILE * __cdecl fopen(const char *,
	const char *);
int __cdecl fprintf(FILE *, const char *, ...);
int __cdecl fputc(int, FILE *);
#ifndef _WINDLL
int __cdecl _fputchar(int);
#endif
int __cdecl fputs(const char *, FILE *);
size_t __cdecl fread(void *, size_t, size_t, FILE *);
FILE * __cdecl freopen(const char *,
	const char *, FILE *);
#ifndef _WINDLL
int __cdecl fscanf(FILE *, const char *, ...);
#endif
int __cdecl fsetpos(FILE *, const fpos_t *);
int __cdecl fseek(FILE *, long, int);
long __cdecl ftell(FILE *);
#ifdef _WINDOWS
#ifndef _WINDLL
FILE * __cdecl _fwopen(struct _wopeninfo *, struct _wsizeinfo *, const char *);
#endif
#endif
size_t __cdecl fwrite(const void *, size_t, size_t,
	FILE *);
int __cdecl getc(FILE *);
#ifndef _WINDLL
int __cdecl getchar(void);
char * __cdecl gets(char *);
#endif
int __cdecl _getw(FILE *);
#ifndef _WINDLL
void __cdecl perror(const char *);
#endif
int __cdecl printf(const char *, ...);
int __cdecl putc(int, FILE *);
#ifndef _WINDLL
int __cdecl putchar(int);
int __cdecl puts(const char *);
#endif
int __cdecl _putw(int, FILE *);
int __cdecl remove(const char *);
int __cdecl rename(const char *, const char *);
void __cdecl rewind(FILE *);
int __cdecl _rmtmp(void);
#ifndef _WINDLL
int __cdecl scanf(const char *, ...);
#endif
void __cdecl setbuf(FILE *, char *);
int __cdecl setvbuf(FILE *, char *, int, size_t);
int __cdecl _snprintf(char *, size_t, const char *, ...);
int __cdecl sprintf(char *, const char *, ...);
#ifndef _WINDLL
int __cdecl sscanf(const char *, const char *, ...);
#endif
char * __cdecl _tempnam(char *, char *);
FILE * __cdecl tmpfile(void);
char * __cdecl tmpnam(char *);
int __cdecl ungetc(int, FILE *);
int __cdecl _unlink(const char *);
int __cdecl vfprintf(FILE *, const char *, va_list);
int __cdecl vprintf(const char *, va_list);
int __cdecl _vsnprintf(char *, size_t, const char *, va_list);
int __cdecl vsprintf(char *, const char *, va_list);
#define _STDIO_DEFINED
#endif

/* macro definitions */

#define feof(_stream)	  ((_stream)->_flag & _IOEOF)
#define ferror(_stream)   ((_stream)->_flag & _IOERR)
#define _fileno(_stream)  ((int)(unsigned char)(_stream)->_file)
#define getc(_stream)	  (--(_stream)->_cnt >= 0 ? 0xff & *(_stream)->_ptr++ \
	: _filbuf(_stream))
#define putc(_c,_stream)  (--(_stream)->_cnt >= 0 \
	? 0xff & (*(_stream)->_ptr++ = (char)(_c)) :  _flsbuf((_c),(_stream)))
#ifndef _WINDLL
#define getchar()	  getc(stdin)
#define putchar(_c)	  putc((_c),stdout)
#endif


#ifndef __STDC__
/* Non-ANSI names for compatibility */

#define P_tmpdir  _P_tmpdir
#define SYS_OPEN  _SYS_OPEN

#ifndef _WINDOWS
#define stdaux	  _stdaux
#define stdprn	  _stdprn
#endif

int __cdecl fcloseall(void);
FILE * __cdecl fdopen(int, const char *);
#ifndef _WINDLL
int __cdecl fgetchar(void);
#endif
int __cdecl fileno(FILE *);
int __cdecl flushall(void);
#ifndef _WINDLL
int __cdecl fputchar(int);
#endif
int __cdecl getw(FILE *);
int __cdecl putw(int, FILE *);
int __cdecl rmtmp(void);
char * __cdecl tempnam(char *, char *);
int __cdecl unlink(const char *);

#endif	/* __STDC__ */

#ifdef __cplusplus
}
#endif

#define _INC_STDIO
#endif	/* _INC_STDIO */
/***
*errno.h - system wide error numbers (set by system calls)
*
*	Copyright (c) 1985-1992, Microsoft Corporation.  All rights reserved.
*
*Purpose:
*	This file defines the system-wide error numbers (set by
*	system calls).	Conforms to the XENIX standard.  Extended
*	for compatibility with Uniforum standard.
*	[ANSI/System V]
*
****/

#ifndef _INC_ERRNO

#ifdef __cplusplus
extern "C" {
#endif

#if (_MSC_VER <= 600)
#define __cdecl     _cdecl
#define __far       _far
#define __near      _near
#endif

/* declare reference to errno */

extern int __near __cdecl volatile errno;

/* Error Codes */

#define EZERO		0
#define EPERM		1
#define ENOENT		2
#define ESRCH		3
#define EINTR		4
#define EIO		5
#define ENXIO		6
#define E2BIG		7
#define ENOEXEC 	8
#define EBADF		9
#define ECHILD		10
#define EAGAIN		11
#define ENOMEM		12
#define EACCES		13
#define EFAULT		14
#define ENOTBLK 	15
#define EBUSY		16
#define EEXIST		17
#define EXDEV		18
#define ENODEV		19
#define ENOTDIR 	20
#define EISDIR		21
#define EINVAL		22
#define ENFILE		23
#define EMFILE		24
#define ENOTTY		25
#define ETXTBSY 	26
#define EFBIG		27
#define ENOSPC		28
#define ESPIPE		29
#define EROFS		30
#define EMLINK		31
#define EPIPE		32
#define EDOM		33
#define ERANGE		34
#define EUCLEAN 	35
#define EDEADLOCK	36

#ifdef __cplusplus
}
#endif

#define _INC_ERRNO
#endif	/* _INC_ERRNO */
/***
*time.h - definitions/declarations for time routines
*
*	Copyright (c) 1985-1992, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	This file contains the various declarations and definitions
*	for the time routines.
*	[ANSI/System V]
*
****/

#ifndef _INC_TIME

#ifdef __cplusplus
extern "C" {
#endif

#if (_MSC_VER <= 600)
#define __cdecl     _cdecl
#define __far       _far
#define __near      _near
#define __pascal    _pascal
#endif

/* implementation defined time types */

#ifndef _TIME_T_DEFINED
typedef unsigned long time_t;
#define _TIME_T_DEFINED
#endif

#ifndef _CLOCK_T_DEFINED
typedef long clock_t;
#define _CLOCK_T_DEFINED
#endif

#ifndef _SIZE_T_DEFINED
typedef unsigned int size_t;
#define _SIZE_T_DEFINED
#endif

/* structure for use with localtime(), gmtime(), etc. */

#ifndef _TM_DEFINED
struct tm {
	int tm_sec;	/* seconds after the minute - [0,59] */
	int tm_min;	/* minutes after the hour - [0,59] */
	int tm_hour;	/* hours since midnight - [0,23] */
	int tm_mday;	/* day of the month - [1,31] */
	int tm_mon;	/* months since January - [0,11] */
	int tm_year;	/* years since 1900 */
	int tm_wday;	/* days since Sunday - [0,6] */
	int tm_yday;	/* days since January 1 - [0,365] */
	int tm_isdst;	/* daylight savings time flag */
	};
#define _TM_DEFINED
#endif


/* define NULL pointer value */

#ifndef NULL
#ifdef __cplusplus
#define NULL	0
#else
#define NULL	((void *)0)
#endif
#endif


/* clock ticks macro - ANSI version */

#define CLOCKS_PER_SEC	1000


/* extern declarations for the global variables used by the ctime family of
 * routines.
 */

extern int __near __cdecl _daylight;	/* non-zero if daylight savings time is used */
extern long __near __cdecl _timezone;	/* difference in seconds between GMT and local time */
extern char * __near __cdecl _tzname[2];/* standard/daylight savings time zone names */


/* function prototypes */

double __cdecl difftime(time_t, time_t);

char * __cdecl asctime(const struct tm *);
char * __cdecl ctime(const time_t *);
#ifndef _WINDLL
clock_t __cdecl clock(void);
#endif
struct tm * __cdecl gmtime(const time_t *);
struct tm * __cdecl localtime(const time_t *);
time_t __cdecl mktime(struct tm *);
#ifndef _WINDLL
size_t __cdecl strftime(char *, size_t, const char *,
	const struct tm *);
#endif
char * __cdecl _strdate(char *);
char * __cdecl _strtime(char *);
time_t __cdecl time(time_t *);
void __cdecl _tzset(void);

#ifndef __STDC__
/* Non-ANSI names for compatibility */

#define CLK_TCK  CLOCKS_PER_SEC

extern int __near __cdecl daylight;
extern long __near __cdecl timezone;
extern char * __near __cdecl tzname[2];

void __cdecl tzset(void);

#endif	/* __STDC__ */

#ifdef __cplusplus
}
#endif

#define _INC_TIME
#endif	/* _INC_TIME */
/***
*ctype.h - character conversion macros and ctype macros
*
*	Copyright (c) 1985-1992, Microsoft Corporation.  All rights reserved.
*
*Purpose:
*	Defines macros for character classification/conversion.
*	[ANSI/System V]
*
****/

#ifndef _INC_CTYPE

#ifdef __cplusplus
extern "C" {
#endif

#if (_MSC_VER <= 600)
#define __cdecl     _cdecl
#define __far       _far
#define __near      _near
#endif

/*
 * This declaration allows the user access to the ctype look-up
 * array _ctype defined in ctype.obj by simply including ctype.h
 */

extern unsigned char __near __cdecl _ctype[];

/* set bit masks for the possible character types */

#define _UPPER		0x1	/* upper case letter */
#define _LOWER		0x2	/* lower case letter */
#define _DIGIT		0x4	/* digit[0-9] */
#define _SPACE		0x8	/* tab, carriage return, newline, */
				/* vertical tab or form feed */
#define _PUNCT		0x10	/* punctuation character */
#define _CONTROL	0x20	/* control character */
#define _BLANK		0x40	/* space char */
#define _HEX		0x80	/* hexadecimal digit */

/* character classification function prototypes */

#ifndef _CTYPE_DEFINED
int __cdecl isalpha(int);
int __cdecl isupper(int);
int __cdecl islower(int);
int __cdecl isdigit(int);
int __cdecl isxdigit(int);
int __cdecl isspace(int);
int __cdecl ispunct(int);
int __cdecl isalnum(int);
int __cdecl isprint(int);
int __cdecl isgraph(int);
int __cdecl iscntrl(int);
int __cdecl toupper(int);
int __cdecl tolower(int);
int __cdecl _tolower(int);
int __cdecl _toupper(int);
int __cdecl __isascii(int);
int __cdecl __toascii(int);
int __cdecl __iscsymf(int);
int __cdecl __iscsym(int);
#define _CTYPE_DEFINED
#endif


/* the character classification macro definitions */

#define isalpha(_c)	( (_ctype+1)[_c] & (_UPPER|_LOWER) )
#define isupper(_c)	( (_ctype+1)[_c] & _UPPER )
#define islower(_c)	( (_ctype+1)[_c] & _LOWER )
#define isdigit(_c)	( (_ctype+1)[_c] & _DIGIT )
#define isxdigit(_c)	( (_ctype+1)[_c] & _HEX )
#define isspace(_c)	( (_ctype+1)[_c] & _SPACE )
#define ispunct(_c)	( (_ctype+1)[_c] & _PUNCT )
#define isalnum(_c)	( (_ctype+1)[_c] & (_UPPER|_LOWER|_DIGIT) )
#define isprint(_c)	( (_ctype+1)[_c] & (_BLANK|_PUNCT|_UPPER|_LOWER|_DIGIT) )
#define isgraph(_c)	( (_ctype+1)[_c] & (_PUNCT|_UPPER|_LOWER|_DIGIT) )
#define iscntrl(_c)	( (_ctype+1)[_c] & _CONTROL )
#ifndef __STDC__
#define toupper(_c)	( (islower(_c)) ? _toupper(_c) : (_c) )
#define tolower(_c)	( (isupper(_c)) ? _tolower(_c) : (_c) )
#endif
#define _tolower(_c)	( (_c)-'A'+'a' )
#define _toupper(_c)	( (_c)-'a'+'A' )
#define __isascii(_c)	( (unsigned)(_c) < 0x80 )
#define __toascii(_c)	( (_c) & 0x7f )

/* extended ctype macros */

#define __iscsymf(_c)	(isalpha(_c) || ((_c) == '_'))
#define __iscsym(_c)	(isalnum(_c) || ((_c) == '_'))

#ifndef __STDC__
/* Non-ANSI names for compatibility */
#ifndef _CTYPE_DEFINED
int __cdecl isascii(int);
int __cdecl toascii(int);
int __cdecl iscsymf(int);
int __cdecl iscsym(int);
#else
#define isascii __isascii
#define toascii __toascii
#define iscsymf __iscsymf
#define iscsym	__iscsym
#endif
#endif	/* __STDC__ */

#ifdef __cplusplus
}
#endif

#define _INC_CTYPE
#endif	/* _INC_CTYPE */
/***
*dos.h - definitions for MS-DOS interface routines
*
*	Copyright (c) 1985-1992, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	Defines the structs and unions used for the direct DOS interface
*	routines; includes macros to access the segment and offset
*	values of far pointers, so that they may be used by the routines; and
*	provides function prototypes for direct DOS interface functions.
*
****/

#ifndef _INC_DOS

#ifdef __cplusplus
extern "C" {
#endif

#if (_MSC_VER <= 600)
#define __cdecl     _cdecl
#define __far       _far
#define __interrupt _interrupt
#define __near      _near
#endif

#ifndef _REGS_DEFINED

/* word registers */

struct _WORDREGS {
	unsigned int ax;
	unsigned int bx;
	unsigned int cx;
	unsigned int dx;
	unsigned int si;
	unsigned int di;
	unsigned int cflag;
	};

/* byte registers */

struct _BYTEREGS {
	unsigned char al, ah;
	unsigned char bl, bh;
	unsigned char cl, ch;
	unsigned char dl, dh;
	};

/* general purpose registers union -
 *  overlays the corresponding word and byte registers.
 */

union _REGS {
	struct _WORDREGS x;
	struct _BYTEREGS h;
	};

/* segment registers */

struct _SREGS {
	unsigned int es;
	unsigned int cs;
	unsigned int ss;
	unsigned int ds;
	};

#ifndef __STDC__
/* Non-ANSI names for compatibility */

struct WORDREGS {
    unsigned int ax;
    unsigned int bx;
    unsigned int cx;
    unsigned int dx;
    unsigned int si;
    unsigned int di;
    unsigned int cflag;
    };

struct BYTEREGS {
    unsigned char al, ah;
    unsigned char bl, bh;
    unsigned char cl, ch;
    unsigned char dl, dh;
    };

union REGS {
    struct WORDREGS x;
    struct BYTEREGS h;
    };

struct SREGS {
    unsigned int es;
    unsigned int cs;
    unsigned int ss;
    unsigned int ds;
    };

#endif	/* __STDC__ */

#define _REGS_DEFINED
#endif /* _REGS_DEFINED */


/* dosexterror structure */

#ifndef _DOSERROR_DEFINED
#pragma pack(2)

struct _DOSERROR {
	int exterror;
	char errclass;
	char action;
	char locus;
	};

#if ((!defined(__STDC__)) && (!defined(__cplusplus)))
/* Non-ANSI name for compatibility */
struct DOSERROR {
	int exterror;
	char class;
	char action;
	char locus;
	};
#endif

#pragma pack()
#define _DOSERROR_DEFINED
#endif


/* _dos_findfirst structure */

#ifndef _FIND_T_DEFINED
#pragma pack(2)

struct _find_t {
	char reserved[21];
	char attrib;
	unsigned wr_time;
	unsigned wr_date;
	long size;
	char name[13];
	};

#ifndef __STDC__
/* Non-ANSI name for compatibility */
#define find_t _find_t
#endif

#pragma pack()
#define _FIND_T_DEFINED
#endif


/* _dos_getdate/_dossetdate and _dos_gettime/_dos_settime structures */

#ifndef _DATETIME_T_DEFINED
#pragma pack(2)

struct _dosdate_t {
	unsigned char day;		/* 1-31 */
	unsigned char month;		/* 1-12 */
	unsigned int year;		/* 1980-2099 */
	unsigned char dayofweek;	/* 0-6, 0=Sunday */
	};

struct _dostime_t {
	unsigned char hour;	/* 0-23 */
	unsigned char minute;	/* 0-59 */
	unsigned char second;	/* 0-59 */
	unsigned char hsecond;	/* 0-99 */
	};

#ifndef __STDC__
/* Non-ANSI names for compatibility */
#define dosdate_t _dosdate_t
#define dostime_t _dostime_t
#endif

#pragma pack()
#define _DATETIME_T_DEFINED
#endif


/* _dos_getdiskfree structure */

#ifndef _DISKFREE_T_DEFINED

struct _diskfree_t {
	unsigned total_clusters;
	unsigned avail_clusters;
	unsigned sectors_per_cluster;
	unsigned bytes_per_sector;
	};

#ifndef __STDC__
/* Non-ANSI name for compatibility */
#define diskfree_t _diskfree_t
#endif

#define _DISKFREE_T_DEFINED
#endif


/* manifest constants for _hardresume result parameter */

#define _HARDERR_IGNORE 	0	/* Ignore the error */
#define _HARDERR_RETRY		1	/* Retry the operation */
#define _HARDERR_ABORT		2	/* Abort program issuing Interrupt 23h */
#define _HARDERR_FAIL		3	/* Fail the system call in progress */
					/* _HARDERR_FAIL is not supported on DOS 2.x */

/* File attribute constants */

#define _A_NORMAL	0x00	/* Normal file - No read/write restrictions */
#define _A_RDONLY	0x01	/* Read only file */
#define _A_HIDDEN	0x02	/* Hidden file */
#define _A_SYSTEM	0x04	/* System file */
#define _A_VOLID	0x08	/* Volume ID file */
#define _A_SUBDIR	0x10	/* Subdirectory */
#define _A_ARCH 	0x20	/* Archive file */

/* macros to break C "far" pointers into their segment and offset components
 */
 
#define _FP_SEG(fp) (*((unsigned __far *)&(fp)+1))
#define _FP_OFF(fp) (*((unsigned __far *)&(fp)))


/* external variable declarations */

extern unsigned int __near __cdecl _osversion;


/* function prototypes */

int __cdecl _bdos(int, unsigned int, unsigned int);
#ifndef _WINDOWS
void __cdecl _chain_intr(void (__cdecl __interrupt __far *)());
#endif
void __cdecl _disable(void);
#ifndef _WINDOWS
unsigned __cdecl _dos_allocmem(unsigned, unsigned *);
#endif
unsigned __cdecl _dos_close(int);
unsigned __cdecl _dos_commit(int);
unsigned __cdecl _dos_creat(const char *, unsigned, int *);
unsigned __cdecl _dos_creatnew(const char *, unsigned, int *);
unsigned __cdecl _dos_findfirst(const char *, unsigned, struct _find_t *);
unsigned __cdecl _dos_findnext(struct _find_t *);
#ifndef _WINDOWS
unsigned __cdecl _dos_freemem(unsigned);
#endif
void __cdecl _dos_getdate(struct _dosdate_t *);
void __cdecl _dos_getdrive(unsigned *);
unsigned __cdecl _dos_getdiskfree(unsigned, struct _diskfree_t *);
unsigned __cdecl _dos_getfileattr(const char *, unsigned *);
unsigned __cdecl _dos_getftime(int, unsigned *, unsigned *);
void __cdecl _dos_gettime(struct _dostime_t *);
void (__cdecl __interrupt __far * __cdecl _dos_getvect(unsigned))();
#ifndef _WINDOWS
void __cdecl _dos_keep(unsigned, unsigned);
#endif
unsigned __cdecl _dos_open(const char *, unsigned, int *);
unsigned __cdecl _dos_read(int, void __far *, unsigned, unsigned *);
#ifndef _WINDOWS
unsigned __cdecl _dos_setblock(unsigned, unsigned, unsigned *);
#endif
unsigned __cdecl _dos_setdate(struct _dosdate_t *);
void __cdecl _dos_setdrive(unsigned, unsigned *);
unsigned __cdecl _dos_setfileattr(const char *, unsigned);
unsigned __cdecl _dos_setftime(int, unsigned, unsigned);
unsigned __cdecl _dos_settime(struct _dostime_t *);
#ifndef _WINDOWS
void __cdecl _dos_setvect(unsigned, void (__cdecl __interrupt __far *)());
#endif
unsigned __cdecl _dos_write(int, const void __far *, unsigned, unsigned *);
int __cdecl _dosexterr(struct _DOSERROR *);
void __cdecl _enable(void);
#ifndef _WINDOWS
void __cdecl _harderr(void (__far __cdecl *)());
void __cdecl _hardresume(int);
void __cdecl _hardretn(int);
#endif
int __cdecl _intdos(union _REGS *, union _REGS *);
int __cdecl _intdosx(union _REGS *, union _REGS *, struct _SREGS *);
int __cdecl _int86(int, union _REGS *, union _REGS *);
int __cdecl _int86x(int, union _REGS *, union _REGS *, struct _SREGS *);

void __cdecl _segread(struct _SREGS *);

#ifndef __STDC__
/* Non-ANSI names for compatibility */

#define FP_SEG	   _FP_SEG
#define FP_OFF	   _FP_OFF

int __cdecl bdos(int, unsigned int, unsigned int);
int __cdecl intdos(union REGS *, union REGS *);
int __cdecl intdosx(union REGS *, union REGS *, struct SREGS *);
int __cdecl int86(int, union REGS *, union REGS *);
int __cdecl int86x(int, union REGS *, union REGS *, struct SREGS *);
#ifndef __cplusplus
int __cdecl dosexterr(struct DOSERROR *);
#endif
void __cdecl segread(struct SREGS *);

#endif	/* __STDC__ */

#ifdef __cplusplus
}
#endif

#define _INC_DOS
#endif	/* _INC_DOS */
/***
*direct.h - function declarations for directory handling/creation
*
*	Copyright (c) 1985-1992, Microsoft Corporation.  All rights reserved.
*
*Purpose:
*	This include file contains the function declarations for the library
*	functions related to directory handling and creation.
*
****/

#ifndef _INC_DIRECT

#ifdef __cplusplus
extern "C" {
#endif

#if (_MSC_VER <= 600)
#define __cdecl     _cdecl
#define __far       _far
#endif

#ifndef _SIZE_T_DEFINED
typedef unsigned int size_t;
#define _SIZE_T_DEFINED
#endif

/* function prototypes */

int __cdecl _chdir(const char *);
int __cdecl _chdrive(int);
char * __cdecl _getcwd(char *, int);
char * __cdecl _getdcwd(int, char *, int);
int __cdecl _getdrive(void);
int __cdecl _mkdir(const char *);
int __cdecl _rmdir(const char *);

#ifndef __STDC__
/* Non-ANSI names for compatibility */
int __cdecl chdir(const char *);
char * __cdecl getcwd(char *, int);
int __cdecl mkdir(const char *);
int __cdecl rmdir(const char *);
#endif

#ifdef __cplusplus
}
#endif

#define _INC_DIRECT
#endif	/* _INC_DIRECT */
/***
*bios.h - declarations for bios interface functions and supporting definitions
*
*	Copyright (c) 1987-1992, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	This file declares the constants, structures, and functions
*	used for accessing and using various BIOS interfaces.
*
****/

#ifndef _INC_BIOS

#ifdef __cplusplus
extern "C" {
#endif

#if (_MSC_VER <= 600)
#define __cdecl     _cdecl
#define __far       _far
#endif


/* manifest constants for BIOS serial communications (RS-232) support */

/* serial port services */

#define _COM_INIT	0	/* init serial port */
#define _COM_SEND	1	/* send character */
#define _COM_RECEIVE	2	/* receive character */
#define _COM_STATUS	3	/* get serial port status */

/* serial port initializers.  One and only one constant from each of the
 * following four groups - character size, stop bit, parity, and baud rate -
 * must be specified in the initialization byte.
 */

/* character size initializers */

#define _COM_CHR7	2	/* 7 bits characters */
#define _COM_CHR8	3	/* 8 bits characters */

/* stop bit values - on or off */

#define _COM_STOP1	0	/* 1 stop bit */
#define _COM_STOP2	4	/* 2 stop bits */

/*  parity initializers */

#define _COM_NOPARITY	0	/* no parity */
#define _COM_ODDPARITY	8	/* odd parity */
#define _COM_EVENPARITY 24	/* even parity */

/*  baud rate initializers */

#define _COM_110	0	/* 110 baud */
#define _COM_150	32	/* 150 baud */
#define _COM_300	64	/* 300 baud */
#define _COM_600	96	/* 600 baud */
#define _COM_1200	128	/* 1200 baud */
#define _COM_2400	160	/* 2400 baud */
#define _COM_4800	192	/* 4800 baud */
#define _COM_9600	224	/* 9600 baud */


/* manifest constants for BIOS disk support */

/* disk services */

#define _DISK_RESET	0	/* reset disk controller */
#define _DISK_STATUS	1	/* get disk status */
#define _DISK_READ	2	/* read disk sectors */
#define _DISK_WRITE	3	/* write disk sectors */
#define _DISK_VERIFY	4	/* verify disk sectors */
#define _DISK_FORMAT	5	/* format disk track */

/* struct used to send/receive information to/from the BIOS disk services */

#ifndef _DISKINFO_T_DEFINED
#pragma pack(2)

struct _diskinfo_t {
    unsigned drive;
    unsigned head;
    unsigned track;
    unsigned sector;
    unsigned nsectors;
    void __far *buffer;
    };

#ifndef __STDC__
/* Non-ANSI name for compatibility */
#define diskinfo_t _diskinfo_t
#endif

#pragma pack()
#define _DISKINFO_T_DEFINED
#endif


/* manifest constants for BIOS keyboard support */

/* keyboard services */

#define _KEYBRD_READ		0	/* read next character from keyboard */
#define _KEYBRD_READY		1	/* check for keystroke */
#define _KEYBRD_SHIFTSTATUS	2	/* get current shift key status */

/* services for enhanced keyboards */

#define _NKEYBRD_READ		0x10	/* read next character from keyboard */
#define _NKEYBRD_READY		0x11	/* check for keystroke */
#define _NKEYBRD_SHIFTSTATUS	0x12	/* get current shift key status */


/* manifest constants for BIOS printer support */

/* printer services */

#define _PRINTER_WRITE	0	/* write character to printer */
#define _PRINTER_INIT	1	/* intialize printer */
#define _PRINTER_STATUS 2	/* get printer status */


/* manifest constants for BIOS time of day support */

/* time of day services */

#define _TIME_GETCLOCK	0	/* get current clock count */
#define _TIME_SETCLOCK	1	/* set current clock count */


#ifndef _REGS_DEFINED

/* word registers */

struct _WORDREGS {
    unsigned int ax;
    unsigned int bx;
    unsigned int cx;
    unsigned int dx;
    unsigned int si;
    unsigned int di;
    unsigned int cflag;
    };

/* byte registers */

struct _BYTEREGS {
    unsigned char al, ah;
    unsigned char bl, bh;
    unsigned char cl, ch;
    unsigned char dl, dh;
    };

/* general purpose registers union -
 *  overlays the corresponding word and byte registers.
 */

union _REGS {
    struct _WORDREGS x;
    struct _BYTEREGS h;
    };

/* segment registers */

struct _SREGS {
    unsigned int es;
    unsigned int cs;
    unsigned int ss;
    unsigned int ds;
    };

#ifndef __STDC__
/* Non-ANSI names for compatibility */

struct WORDREGS {
    unsigned int ax;
    unsigned int bx;
    unsigned int cx;
    unsigned int dx;
    unsigned int si;
    unsigned int di;
    unsigned int cflag;
    };

struct BYTEREGS {
    unsigned char al, ah;
    unsigned char bl, bh;
    unsigned char cl, ch;
    unsigned char dl, dh;
    };

union REGS {
    struct WORDREGS x;
    struct BYTEREGS h;
    };

struct SREGS {
    unsigned int es;
    unsigned int cs;
    unsigned int ss;
    unsigned int ds;
    };

#endif	/* __STDC__ */

#define _REGS_DEFINED
#endif /* _REGS_DEFINED */


/* function prototypes */

#ifndef _WINDOWS
unsigned __cdecl _bios_disk(unsigned, struct _diskinfo_t *);
#endif
unsigned __cdecl _bios_equiplist(void);
#ifndef _WINDOWS
unsigned __cdecl _bios_keybrd(unsigned);
#endif
unsigned __cdecl _bios_memsize(void);
#ifndef _WINDOWS
unsigned __cdecl _bios_printer(unsigned, unsigned, unsigned);
unsigned __cdecl _bios_serialcom(unsigned, unsigned, unsigned);
#endif
unsigned __cdecl _bios_timeofday(unsigned, long *);
int __cdecl _int86(int, union _REGS *, union _REGS *);
int __cdecl _int86x(int, union _REGS *, union _REGS *, struct _SREGS *);

#ifndef __STDC__
/* Non-ANSI names for compatibility */
int __cdecl int86(int, union REGS *, union REGS *);
int __cdecl int86x(int, union REGS *, union REGS *, struct SREGS *);
#endif


#ifdef __cplusplus
}
#endif

#define _INC_BIOS
#endif	/* _INC_BIOS */
/***
*string.h - declarations for string manipulation functions
*
*	Copyright (c) 1985-1992, Microsoft Corporation.  All rights reserved.
*
*Purpose:
*	This file contains the function declarations for the string
*	manipulation functions.
*	[ANSI/System V]
*
****/

#ifndef _INC_STRING

#ifdef __cplusplus
extern "C" {
#endif

#if (_MSC_VER <= 600)
#define __cdecl     _cdecl
#define __far       _far
#define __near      _near
#endif

#ifndef _SIZE_T_DEFINED
typedef unsigned int size_t;
#define _SIZE_T_DEFINED
#endif

#ifndef _WCHAR_T_DEFINED
typedef	unsigned short wchar_t;
#define _WCHAR_T_DEFINED
#endif

/* define NULL pointer value */

#ifndef NULL
#ifdef __cplusplus
#define NULL	0
#else
#define NULL	((void *)0)
#endif
#endif

/* function prototypes */

void * __cdecl _memccpy(void *, const void *,
	int, unsigned int);
void * __cdecl memchr(const void *, int, size_t);
int __cdecl memcmp(const void *, const void *,
	size_t);
int __cdecl _memicmp(const void *, const void *,
	unsigned int);
void * __cdecl memcpy(void *, const void *,
	size_t);
void * __cdecl memmove(void *, const void *,
	size_t);
void * __cdecl memset(void *, int, size_t);
void __cdecl _movedata(unsigned int, unsigned int, unsigned int,
	unsigned int, unsigned int);
char * __cdecl strcat(char *, const char *);
char * __cdecl strchr(const char *, int);
int __cdecl strcmp(const char *, const char *);
int __cdecl _strcmpi(const char *, const char *);
int __cdecl strcoll(const char *, const char *);
int __cdecl _stricmp(const char *, const char *);
char * __cdecl strcpy(char *, const char *);
size_t __cdecl strcspn(const char *, const char *);
char * __cdecl _strdup(const char *);
char * __cdecl _strerror(const char *);
char * __cdecl strerror(int);
size_t __cdecl strlen(const char *);
char * __cdecl _strlwr(char *);
char * __cdecl strncat(char *, const char *,
	size_t);
int __cdecl strncmp(const char *, const char *,
	size_t);
int __cdecl _strnicmp(const char *, const char *,
	size_t);
char * __cdecl strncpy(char *, const char *,
	size_t);
char * __cdecl _strnset(char *, int, size_t);
char * __cdecl strpbrk(const char *,
	const char *);
char * __cdecl strrchr(const char *, int);
char * __cdecl _strrev(char *);
char * __cdecl _strset(char *, int);
size_t __cdecl strspn(const char *, const char *);
char * __cdecl strstr(const char *,
	const char *);
char * __cdecl strtok(char *, const char *);
char * __cdecl _strupr(char *);
size_t __cdecl strxfrm (char *, const char *,
	size_t);


/* model independent function prototypes */

void __far * __far __cdecl _fmemccpy(void __far *, const void __far *,
	int, unsigned int);
void __far * __far __cdecl _fmemchr(const void __far *, int, size_t);
int __far __cdecl _fmemcmp(const void __far *, const void __far *,
	size_t);
void __far * __far __cdecl _fmemcpy(void __far *, const void __far *,
	size_t);
int __far __cdecl _fmemicmp(const void __far *, const void __far *,
	unsigned int);
void __far * __far __cdecl _fmemmove(void __far *, const void __far *,
	size_t);
void __far * __far __cdecl _fmemset(void __far *, int, size_t);
char __far * __far __cdecl _fstrcat(char __far *, const char __far *);
char __far * __far __cdecl _fstrchr(const char __far *, int);
int __far __cdecl _fstrcmp(const char __far *, const char __far *);
int __far __cdecl _fstricmp(const char __far *, const char __far *);
char __far * __far __cdecl _fstrcpy(char __far *, const char __far *);
size_t __far __cdecl _fstrcspn(const char __far *, const char __far *);
char __far * __far __cdecl _fstrdup(const char __far *);
char __near * __far __cdecl _nstrdup(const char __far *);
size_t __far __cdecl _fstrlen(const char __far *);
char __far * __far __cdecl _fstrlwr(char __far *);
char __far * __far __cdecl _fstrncat(char __far *, const char __far *,
	size_t);
int __far __cdecl _fstrncmp(const char __far *, const char __far *,
	size_t);
int __far __cdecl _fstrnicmp(const char __far *, const char __far *,
	size_t);
char __far * __far __cdecl _fstrncpy(char __far *, const char __far *,
	size_t);
char __far * __far __cdecl _fstrnset(char __far *, int, size_t);
char __far * __far __cdecl _fstrpbrk(const char __far *,
	const char __far *);
char __far * __far __cdecl _fstrrchr(const char __far *, int);
char __far * __far __cdecl _fstrrev(char __far *);
char __far * __far __cdecl _fstrset(char __far *, int);
size_t __far __cdecl _fstrspn(const char __far *, const char __far *);
char __far * __far __cdecl _fstrstr(const char __far *,
	const char __far *);
char __far * __far __cdecl _fstrtok(char __far *, const char __far *);
char __far * __far __cdecl _fstrupr(char __far *);


#ifndef __STDC__
/* Non-ANSI names for compatibility */
void * __cdecl memccpy(void *, const void *,
	int, unsigned int);
int __cdecl memicmp(const void *, const void *,
	unsigned int);
void __cdecl movedata(unsigned int, unsigned int, unsigned int,
	unsigned int, unsigned int);
int __cdecl strcmpi(const char *, const char *);
int __cdecl stricmp(const char *, const char *);
char * __cdecl strdup(const char *);
char * __cdecl strlwr(char *);
int __cdecl strnicmp(const char *, const char *,
	size_t);
char * __cdecl strnset(char *, int, size_t);
char * __cdecl strrev(char *);
char * __cdecl strset(char *, int);
char * __cdecl strupr(char *);
#endif

#ifdef __cplusplus
}
#endif

#define _INC_STRING
#endif	/* _INC_STRING */
/***
*setjmp.h - definitions/declarations for setjmp/longjmp routines
*
*	Copyright (c) 1985-1992, Microsoft Corporation.  All rights reserved.
*
*Purpose:
*	This file defines the machine-dependent buffer used by
*	setjmp/longjmp to save and restore the program state, and
*	declarations for those routines.
*	[ANSI/System V]
*
****/

#ifndef _INC_SETJMP

#ifndef __cplusplus

#if (_MSC_VER <= 600)
#define __cdecl     _cdecl
#define __far       _far
#endif

/* define the buffer type for holding the state information */

#define _JBLEN	9  /* bp, di, si, sp, ret addr, ds */

#ifndef _JMP_BUF_DEFINED
typedef  int  jmp_buf[_JBLEN];
#define _JMP_BUF_DEFINED
#endif


/* function prototypes */

int  __cdecl setjmp(jmp_buf);
void __cdecl longjmp(jmp_buf, int);

#endif	/* _cplusplus */

#define _INC_SETJMP
#endif	/* _INC_SETJMP */
/***
*stdlib.h - declarations/definitions for commonly used library functions
*
*	Copyright (c) 1985-1992, Microsoft Corporation.  All rights reserved.
*
*Purpose:
*	This include file contains the function declarations for
*	commonly used library functions which either don't fit somewhere
*	else, or, like toupper/tolower, can't be declared in the normal
*	place for other reasons.
*	[ANSI]
*
****/

#ifndef _INC_STDLIB

#ifdef __cplusplus
extern "C" {
#endif

#if (_MSC_VER <= 600)
#define __cdecl     _cdecl
#define __far       _far
#define __near      _near
#define __pascal    _pascal
#endif

#ifndef _SIZE_T_DEFINED
typedef unsigned int size_t;
#define _SIZE_T_DEFINED
#endif

#ifndef _WCHAR_T_DEFINED
typedef unsigned short wchar_t;
#define _WCHAR_T_DEFINED
#endif

/* define NULL pointer value */

#ifndef NULL
#ifdef __cplusplus
#define NULL	0
#else
#define NULL	((void *)0)
#endif
#endif

/* exit() arg values */

#define EXIT_SUCCESS	0
#define EXIT_FAILURE	1

#ifndef _ONEXIT_T_DEFINED
typedef int (__cdecl * _onexit_t)();
typedef int (__far __cdecl * _fonexit_t)();
#ifndef __STDC__
/* Non-ANSI name for compatibility */
typedef int (__cdecl * onexit_t)();
#endif
#define _ONEXIT_T_DEFINED
#endif


/* data structure definitions for div and ldiv runtimes. */

#ifndef _DIV_T_DEFINED

typedef struct _div_t {
	int quot;
	int rem;
} div_t;

typedef struct _ldiv_t {
	long quot;
	long rem;
} ldiv_t;

#define _DIV_T_DEFINED
#endif

/* maximum value that can be returned by the rand function. */

#define RAND_MAX 0x7fff

extern unsigned short __mb_cur_max; /* mb-len for curr. locale */
#define MB_CUR_MAX __mb_cur_max


/* min and max macros */

#define __max(a,b)	(((a) > (b)) ? (a) : (b))
#define __min(a,b)	(((a) < (b)) ? (a) : (b))


/* sizes for buffers used by the _makepath() and _splitpath() functions.
 * note that the sizes include space for 0-terminator
 */

#define _MAX_PATH	260	/* max. length of full pathname */
#define _MAX_DRIVE	3	/* max. length of drive component */
#define _MAX_DIR	256	/* max. length of path component */
#define _MAX_FNAME	256	/* max. length of file name component */
#define _MAX_EXT	256	/* max. length of extension component */

/* external variable declarations */

extern int __near __cdecl volatile errno;	/* error value */
extern int __near __cdecl _doserrno;		/* OS system error value */

extern char * __near __cdecl _sys_errlist[];	/* perror error message table */
extern int __near __cdecl _sys_nerr;		/* # of entries in sys_errlist table */
extern char ** __near __cdecl _environ; 	/* pointer to environment table */
extern int __near __cdecl _fmode;		/* default file translation mode */
#ifndef _WINDOWS
extern int __near __cdecl _fileinfo;		/* open file info mode (for spawn) */
#endif

extern unsigned int __near __cdecl _psp;	/* Program Segment Prefix */

/* OS major/minor version numbers */

extern unsigned char __near __cdecl _osmajor;
extern unsigned char __near __cdecl _osminor;

/* OS mode */

#define _DOS_MODE	0	/* DOS */
#define _OS2_MODE	1	/* OS/2 */
#define _WIN_MODE	2	/* Windows */

extern unsigned char __near __cdecl _osmode;

/* CPU mode */

#define _REAL_MODE	0	/* real mode */
#define _PROT_MODE	1	/* protect mode */

extern unsigned char __near __cdecl _cpumode;

/* function prototypes */

double __cdecl atof(const char *);
double __cdecl strtod(const char *, char * *);
ldiv_t __cdecl ldiv(long, long);

void __cdecl abort(void);
int __cdecl abs(int);
int __cdecl atexit(void (__cdecl *)(void));
int __cdecl atoi(const char *);
long __cdecl atol(const char *);
long double __cdecl _atold(const char *);
void * __cdecl bsearch(const void *, const void *,
	size_t, size_t, int (__cdecl *)(const void *,
	const void *));
void * __cdecl calloc(size_t, size_t);
div_t __cdecl div(int, int);
char * __cdecl _ecvt(double, int, int *, int *);
#ifndef _WINDLL
void __cdecl exit(int);
void __cdecl _exit(int);
#endif
int __far __cdecl _fatexit(void (__cdecl __far *)(void));
char * __cdecl _fcvt(double, int, int *, int *);
_fonexit_t __far __cdecl _fonexit(_fonexit_t);
void __cdecl free(void *);
char * __cdecl _fullpath(char *, const char *,
	size_t);
char * __cdecl _gcvt(double, int, char *);
char * __cdecl getenv(const char *);
char * __cdecl _itoa(int, char *, int);
long __cdecl labs(long);
unsigned long __cdecl _lrotl(unsigned long, int);
unsigned long __cdecl _lrotr(unsigned long, int);
char * __cdecl _ltoa(long, char *, int);
void __cdecl _makepath(char *, const char *,
	const char *, const char *, const char *);
void * __cdecl malloc(size_t);
_onexit_t __cdecl _onexit(_onexit_t);
#ifndef _WINDLL
void __cdecl perror(const char *);
#endif
int __cdecl _putenv(const char *);
void __cdecl qsort(void *, size_t, size_t, int (__cdecl *)
	(const void *, const void *));
unsigned int __cdecl _rotl(unsigned int, int);
unsigned int __cdecl _rotr(unsigned int, int);
int __cdecl rand(void);
void * __cdecl realloc(void *, size_t);
void __cdecl _searchenv(const char *, const char *,
	char *);
void __cdecl _splitpath(const char *, char *,
	char *, char *, char *);
void __cdecl srand(unsigned int);
long __cdecl strtol(const char *, char * *,
	int);
long double __cdecl _strtold(const char *,
	char * *);
unsigned long __cdecl strtoul(const char *,
	char * *, int);
void __cdecl _swab(char *, char *, int);
#ifndef _WINDOWS
int __cdecl system(const char *);
#endif
char * __cdecl _ultoa(unsigned long, char *, int);

int __cdecl mblen(const char *, size_t);
int __cdecl mbtowc(wchar_t *, const char *, size_t);
int __cdecl wctomb(char *, wchar_t);
size_t __cdecl mbstowcs(wchar_t *, const char *, size_t);
size_t __cdecl wcstombs(char *, const wchar_t *, size_t);

/* model-independent function prototypes */

int __far __cdecl _fmblen(const char __far *, size_t);
int __far __cdecl _fmbtowc(wchar_t __far *, const char __far *,
	size_t);
int __far __cdecl _fwctomb(char __far *, wchar_t);
size_t __far __cdecl _fmbstowcs(wchar_t __far *, const char __far *,
	size_t);
size_t __far __cdecl _fwcstombs(char __far *, const wchar_t __far *,
	size_t);

#ifndef tolower 	/* tolower has been undefined - use function */
int __cdecl tolower(int);
#endif	/* tolower */

#ifndef toupper 	/* toupper has been undefined - use function */
int __cdecl toupper(int);
#endif	/* toupper */

#ifndef __STDC__
/* Non-ANSI names for compatibility */

#ifndef __cplusplus
#define max(a,b)	(((a) > (b)) ? (a) : (b))
#define min(a,b)	(((a) < (b)) ? (a) : (b))
#endif

extern char * __near __cdecl sys_errlist[];
extern int __near __cdecl sys_nerr;
extern char ** __near __cdecl environ;

#define DOS_MODE    _DOS_MODE
#define OS2_MODE    _OS2_MODE

char * __cdecl ecvt(double, int, int *, int *);
char * __cdecl fcvt(double, int, int *, int *);
char * __cdecl gcvt(double, int, char *);
char * __cdecl itoa(int, char *, int);
char * __cdecl ltoa(long, char *, int);
onexit_t __cdecl onexit(onexit_t);
int __cdecl putenv(const char *);
void __cdecl swab(char *, char *, int);
char * __cdecl ultoa(unsigned long, char *, int);

#endif	/* __STDC__ */

#ifdef __cplusplus
}
#endif

#define _INC_STDLIB
#endif	/* _INC_STDLIB */
/***
*io.h - declarations for low-level file handling and I/O functions
*
*	Copyright (c) 1985-1992, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	This file contains the function declarations for the low-level
*	file handling and I/O functions.
*
****/

#ifndef _INC_IO

#ifdef __cplusplus
extern "C" {
#endif

#if (_MSC_VER <= 600)
#define __cdecl     _cdecl
#define __far       _far
#endif

#ifdef _WINDOWS
#ifndef _WINDLL
#ifndef _WINFO_DEFINED
/* interface version number */
#define _QWINVER	0

/* max number of windows */
#define _WFILE		20

/* values for windows screen buffer size */
#define _WINBUFINF	0
#define _WINBUFDEF	-1

/* size/move settings */
#define _WINSIZEMIN	1
#define _WINSIZEMAX	2
#define _WINSIZERESTORE 3
#define _WINSIZECHAR	4

/* size/move query types */
#define _WINMAXREQ	100
#define _WINCURRREQ	101

/* values for closing window */
#define _WINPERSIST	1
#define _WINNOPERSIST	0

/* pseudo file handle for frame window */
#define _WINFRAMEHAND	-1

/* menu items */
#define _WINSTATBAR	1
#define _WINTILE	2
#define _WINCASCADE	3
#define _WINARRANGE	4

/* quickwin exit options */
#define _WINEXITPROMPT		1
#define _WINEXITNOPERSIST	2
#define _WINEXITPERSIST 	3

/* open structure */
#pragma pack(2)
struct _wopeninfo {
	unsigned int _version;
	const char __far * _title;
	long _wbufsize;
	};
#pragma pack()

/* size/move structure */
struct _wsizeinfo {
	unsigned int _version;
	unsigned int _type;
	unsigned int _x;
	unsigned int _y;
	unsigned int _h;
	unsigned int _w;
	};

#define _WINFO_DEFINED
#endif
#endif
#endif

/* function prototypes */

int __cdecl _access(const char *, int);
int __cdecl _chmod(const char *, int);
int __cdecl _chsize(int, long);
int __cdecl _close(int);
int __cdecl _commit(int);
int __cdecl _creat(const char *, int);
int __cdecl _dup(int);
int __cdecl _dup2(int, int);
int __cdecl _eof(int);
long __cdecl _filelength(int);
int __cdecl _isatty(int);
int __cdecl _locking(int, int, long);
long __cdecl _lseek(int, long, int);
char * __cdecl _mktemp(char *);
int __cdecl _open(const char *, int, ...);
int __cdecl _read(int, void *, unsigned int);
int __cdecl remove(const char *);
int __cdecl rename(const char *, const char *);
int __cdecl _setmode(int, int);
int __cdecl _sopen(const char *, int, int, ...);
long __cdecl _tell(int);
int __cdecl _umask(int);
int __cdecl _unlink(const char *);
int __cdecl _write(int, const void *, unsigned int);
#ifdef _WINDOWS
#ifndef _WINDLL
int __cdecl _wabout(char *);
int __cdecl _wclose(int, int);
int __cdecl _wgetexit(void);
int __cdecl _wgetfocus(void);
long __cdecl _wgetscreenbuf(int);
int __cdecl _wgetsize(int, int, struct _wsizeinfo *);
int __cdecl _wmenuclick(int);
int __cdecl _wopen(struct _wopeninfo *, struct _wsizeinfo *, int);
int __cdecl _wsetexit(int);
int __cdecl _wsetfocus(int);
int __cdecl _wsetscreenbuf(int, long);
int __cdecl _wsetsize(int, struct _wsizeinfo *);
void __cdecl _wyield(void);
#endif
#endif

#ifndef __STDC__
/* Non-ANSI names for compatibility */
int __cdecl access(const char *, int);
int __cdecl chmod(const char *, int);
int __cdecl chsize(int, long);
int __cdecl close(int);
int __cdecl creat(const char *, int);
int __cdecl dup(int);
int __cdecl dup2(int, int);
int __cdecl eof(int);
long __cdecl filelength(int);
int __cdecl isatty(int);
int __cdecl locking(int, int, long);
long __cdecl lseek(int, long, int);
char * __cdecl mktemp(char *);
int __cdecl open(const char *, int, ...);
int __cdecl read(int, void *, unsigned int);
int __cdecl setmode(int, int);
int __cdecl sopen(const char *, int, int, ...);
long __cdecl tell(int);
int __cdecl umask(int);
int __cdecl unlink(const char *);
int __cdecl write(int, const void *, unsigned int);
#endif

#ifdef __cplusplus
}
#endif

#define _INC_IO
#endif	/* _INC_IO */
/***
*stdarg.h - defines ANSI-style macros for variable argument functions
*
*	Copyright (c) 1985-1992, Microsoft Corporation.  All rights reserved.
*
*Purpose:
*	This file defines ANSI-style macros for accessing arguments
*	of functions which take a variable number of arguments.
*	[ANSI]
*
****/

#ifndef _INC_STDARG

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _WINDLL
#define _FARARG_ __far
#else
#define _FARARG_
#endif

#if (_MSC_VER <= 600)
#define __far       _far
#endif

#ifndef _VA_LIST_DEFINED
typedef char _FARARG_ *va_list;
#define _VA_LIST_DEFINED
#endif

/*
 * define a macro to compute the size of a type, variable or expression,
 * rounded up to the nearest multiple of sizeof(int). This number is its
 * size as function argument (Intel architecture). Note that the macro
 * depends on sizeof(int) being a power of 2!
 */

#define _INTSIZEOF(n)	 ( (sizeof(n) + sizeof(int) - 1) & ~(sizeof(int) - 1) )

#define va_start(ap,v) ap = (va_list)&v + _INTSIZEOF(v)
#define va_arg(ap,t) ( *(t _FARARG_ *)((ap += _INTSIZEOF(t)) - _INTSIZEOF(t)) )
#define va_end(ap) ap = (va_list)0

#ifdef __cplusplus
}
#endif

#define _INC_STDARG
#endif	/* _INC_STDARG */
/***
*conio.h - console and port I/O declarations
*
*	Copyright (c) 1985-1992, Microsoft Corporation.  All rights reserved.
*
*Purpose:
*	This include file contains the function declarations for
*	the MS C V2.03 compatible console and port I/O routines.
*
****/

#ifndef _INC_CONIO

#ifdef __cplusplus
extern "C" {
#endif

#if (_MSC_VER <= 600)
#define __cdecl     _cdecl
#define __far       _far
#endif

/* function prototypes */

#ifndef _WINDOWS
char * __cdecl _cgets(char *);
int __cdecl _cprintf(const char *, ...);
int __cdecl _cputs(const char *);
int __cdecl _cscanf(const char *, ...);
int __cdecl _getch(void);
int __cdecl _getche(void);
#endif
int __cdecl _inp(unsigned);
unsigned __cdecl _inpw(unsigned);
#ifndef _WINDOWS
int __cdecl _kbhit(void);
#endif
int __cdecl _outp(unsigned, int);
unsigned __cdecl _outpw(unsigned, unsigned);
#ifndef _WINDOWS
int __cdecl _putch(int);
int __cdecl _ungetch(int);
#endif

#ifndef __STDC__
/* Non-ANSI names for compatibility */
#ifndef _WINDOWS
char * __cdecl cgets(char *);
int __cdecl cprintf(const char *, ...);
int __cdecl cputs(const char *);
int __cdecl cscanf(const char *, ...);
int __cdecl getch(void);
int __cdecl getche(void);
#endif
int __cdecl inp(unsigned);
unsigned __cdecl inpw(unsigned);
#ifndef _WINDOWS
int __cdecl kbhit(void);
#endif
int __cdecl outp(unsigned, int);
unsigned __cdecl outpw(unsigned, unsigned);
#ifndef _WINDOWS
int __cdecl putch(int);
int __cdecl ungetch(int);
#endif
#endif	/* __STDC__ */

#ifdef __cplusplus
}
#endif

#define _INC_CONIO
#endif	/* _INC_CONIO */
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

/* RcsId: $Id: defines.h,v 1.20 1994/06/29 13:42:25 tony Exp $ */
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

#ifdef SOLARIS
#undef SOLARIS
#define SOLARIS	1
#ifndef SUN4
#define SUN4	1
#define UNIX	1
#endif
#else
#define SOLARIS 0
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

#if (RS6000 || HP9000 || SOLARIS || __GNU__ > 0)
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
/* RcsId: $Id: barthdr,v 1.6 1994/07/06 10:44:59 mgun Exp $ */
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
#define fn(a,b) a b
#else
#define fn(a,b) a()
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
***
*** For C++ compilers, which have stricter type checking, I've also
*** placed the cast of the function to VoidFnPtr here as well.
**/
#if (ST || HELIOS)
#define func(x) (&x)
#else
#  ifdef cplusplus
#    define func(x) ((VoidFnPtr)(x))
#  else
#    define func(x) (x)
#  endif
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

#if ANSI_prototypes
int ToLower (int x)
#else
int ToLower(x)
int x;
#endif
{ return(isupper(x) ? tolower(x) : x);
}

#if ANSI_prototypes
int ToUpper (int x)
#else
int ToUpper(x)
int x;
#endif
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
  /* These have memcpy() and memset(), but not memmove() ... */
#if (!SOLARIS)
  /* ... which does */
#define memmove(a, b, c) bcopy(b,a,c)
#endif
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
/* RcsId: $Id: protocol.h,v 1.11 1994/06/29 13:42:25 tony Exp $ */
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
#define RS232_B56000         16

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
/* RcsId: $Id: structs.h,v 1.7 1994/06/29 13:42:25 tony Exp $ */
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

#ifdef __cplusplus
typedef void	(*VoidNodeFnPtr)(Node *, ...);
typedef word	(*WordNodeFnPtr)(Node *, ...);
#else
typedef VoidFnPtr	VoidNodeFnPtr;
typedef WordFnPtr	WordNodeFnPtr;
#endif

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
typedef struct Conode_str { Node node;           /* conodes held in linked list  */
                 word id;             /* unique integer identifier    */
                 word type;           /* usually zero, may be suicide */
                                      /* or timedout or ready         */
                 word flags;
                 word timelimit;      /* when stream should die       */
                 ptr  cobase;         /* from CreateCo                */
#ifdef __cplusplus
		 void (**handlers)(struct Conode_str *);
#else
                 void (**handlers)(); /* request handlers             */
#endif
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

#ifdef __cplusplus

typedef void		(*VoidConFnPtr)(Conode *);
typedef void		(*VoidWordFnPtr)(word *);
typedef word		(*WordConFnPtr)(Conode *);

#else

typedef VoidFnPtr	VoidConFnPtr;
typedef VoidFnPtr	VoidWordFnPtr;
typedef WordFnPtr	WordConFnPtr;

#endif

typedef struct device_declaration {
       word type;
       char *name;
       VoidConFnPtr handlers[handler_max];
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

#ifdef __cplusplus
typedef WORD	(*WordUbyteFnPtr)(UBYTE *);
typedef WORD	(*WordIntFnPtr)(int);
typedef WORD	(*WordIntplusFnPtr)(int, BYTE *, ...);
#else
typedef WordFnPtr	WordUbyteFnPtr;
typedef WordFnPtr	WordIntFnPtr;
typedef WordFnPtr	WordIntplusFnPtr;
#endif

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
/* RcsId: $Id: fundefs.h,v 1.14 1994/06/29 13:42:25 tony Exp nickc $ */
/* Copyright (C) 1987, Perihelion Software Ltd.       			*/

#ifdef __cplusplus
#define iofree(x)	free ((void *)(x))
#else
#define iofree(x)	free (x)
#endif

/**
*** Linked list library, the same linked lists as under AmigaDos and Helios
**/
PUBLIC List *fn( MakeHeader,  (void));
PUBLIC void fn(  InitList,    (List *));
PUBLIC void fn(  WalkList,    (List *, VoidNodeFnPtr, ...));
PUBLIC void fn(  FreeList,    (List *));
PUBLIC Node *fn( AddHead,     (Node *, List *));
PUBLIC Node *fn( AddTail,     (Node *, List *));
PUBLIC Node *fn( listRemove,  (Node *));
PUBLIC Node *fn( NextNode,    (Node *));
PUBLIC word fn(  Wander,      (List *, WordNodeFnPtr, ...));
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

PUBLIC int fn (setsu, (int));
PUBLIC void fn (convert_errno, (void));

#if (UNIX && !MEIKORTE && !UNIX386 && !IUNIX386)
PUBLIC void fn (check_helios_name, (char *));
#endif

#if (UNIX)
PUBLIC void fn (unix_restore_devices, (void));
PUBLIC void fn (unix_initialise_devices, (void));
#endif

#ifndef Daemon_Module

/**
*** A higher level coroutine library using the above linked lists.
**/
PUBLIC Conode *fn( NewCo,     (VoidConFnPtr));
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

/*
 * PUBLIC  void fn( IgnoreVoid,                (void));
 */
#ifdef __cplusplus
PUBLIC  void fn (IgnoreVoid, 	(Conode *));
#else
PUBLIC  void fn (IgnoreVoid, (void));
#endif

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
PUBLIC void fn(  NewStream,       (word, word, word, VoidConFnPtr *));
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

#ifdef __cplusplus
extern "C"
{
#endif
PUBLIC word fn( create_a_window, (char *));
PUBLIC void fn( window_size,     (word, word *, word *));
PUBLIC void fn( close_window,    (word));
PUBLIC void fn( send_to_window,  (char *, Window *));
#ifdef __cplusplus
}
#endif
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
PUBLIC  void  fn(Graph_InitServer,   (Conode *));
PUBLIC  void  fn(Graph_TidyServer,   (Conode *));
#define Graph_Private                Forward_handler
PUBLIC  void  fn(Graph_Testfun,      (bool *));
PUBLIC  void  fn(Graph_Locate,       (Conode *));
PUBLIC  void  fn(Graph_Open,         (Conode *));
#define Graph_Create		     Graph_Locate
PUBLIC  void  fn(Graph_Delete,       (Conode *));
PUBLIC  void  fn(Graph_ObjectInfo,   (Conode *));
#define Graph_ServerInfo	     Invalidfn_handler
#define Graph_Rename		     Invalidfn_handler
#define Graph_Link		     Invalidfn_handler
#define Graph_Protect		     Protect_handler
#define Graph_SetDate		     Invalidfn_handler
#define Graph_Refine		     Refine_handler
#define Graph_CloseObj 	             Invalidfn_handler

PUBLIC  word fn(Graph_InitStream,    (Conode *));
PUBLIC  word fn(Graph_TidyStream,    (Conode *));
PUBLIC  void fn(Graph_PrivateStream, (Conode *));
PUBLIC  void fn(Graph_Read,          (Conode *));
PUBLIC  void fn(Graph_Write,         (Conode *));
PUBLIC  void fn(Graph_Close,         (Conode *));
#define Graph_GetSize                Device_GetSize
#define Graph_SetSize                Invalidfn_handler
#define Graph_Seek                   Invalidfn_handler
PUBLIC  void fn(Graph_GetAttr,       (Conode *));
PUBLIC  void fn(Graph_SetAttr,       (Conode *));
#define Graph_EnableEvents           Invalidfn_handler
#define Graph_Acknowledge            IgnoreVoid
#define Graph_NegAcknowledge         IgnoreVoid
#define Graph_Select                 Select_handler
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
// #define Ether_TidyServer                IgnoreVoid
// AxM: "Reactivated" this coroutine
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

#if !(SOLARIS || SUN3 || SUN4)

/* free/malloc declared in <stdlib.h> */

#if !(PC || MAC || HELIOS || RS6000)     /* The PC's header files declare malloc */
#if !(AMIGA)
PUBLIC char *fn( malloc,      (uint));
/* PUBLIC void fn( free,         (char *)); */	/* now included via <stdlib.h> */
#else
PUBLIC char *fn( malloc,      (uint));
PUBLIC void fn( free,         (void *));
#endif
#endif

#endif /* !SOLARIS */

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
/**
*** ClearMultiwait() is just the inverse of AddMultiwait()
**/

#ifdef __cplusplus
extern "C"
{
#endif

#if (RS6000 || HP9000 || SCOUNIX)

PUBLIC void fn (AddMultiwait, (WORD, WORD *, ...));
PUBLIC void fn( ClearMultiwait, (WORD, ...));

#else

PUBLIC void fn (AddMultiwait, (long, long *, long, long));
PUBLIC void fn (ClearMultiwait, (long, long, long));

#endif

#ifdef __cplusplus
}
#endif

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
PUBLIC  void fn( vy86pid_set_baudrate,    (word, bool));
PUBLIC  word fn( vy86pid_get_configbaud,  (void));
PUBLIC  void fn( vy86pid_setup_handshake, (void));
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
/* RcsId: $Id: server.h,v 1.13 1994/06/29 13:42:25 tony Exp $ */
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

#if SOLARIS
jmp_buf *   exit_jmpbuf;
#define	    longjmp_exit	longjmp (*exit_jmpbuf, 1)
#else
jmp_buf     exit_jmpbuf;
#define     longjmp_exit	longjmp (exit_jmpbuf, 1)
#endif

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

{ 
  { Type_Directory, DefaultServerName,
     { 
	IOPROC_InitServer, 
	IOPROC_TidyServer,  
	IOPROC_Private,
	(VoidConFnPtr)IOPROC_Testfun,
	IOPROC_Open,       
	IOPROC_Create,      
	IOPROC_Locate,
	IOPROC_ObjectInfo, 
	IOPROC_ServerInfo,  
	IOPROC_Delete,
       	IOPROC_Rename,     
	IOPROC_Link,        
	IOPROC_Protect,
       	IOPROC_SetDate,    
	IOPROC_Refine,      
	IOPROC_CloseObj
      }
  }

#if Romdisk_supported
 ,{ Type_Directory, "romdisk",
     { 
	RomDisk_InitServer,  
	RomDisk_TidyServer,  
	RomDisk_Private,
	(VoidConFnPtr)RomDisk_Testfun,
	RomDisk_Open,        
	RomDisk_Create,      
	RomDisk_Locate,
	RomDisk_ObjectInfo,  
	RomDisk_ServerInfo,  
	RomDisk_Delete,
	RomDisk_Rename,      
	RomDisk_Link,        
	RomDisk_Protect,
	RomDisk_SetDate,     
	RomDisk_Refine,      
	RomDisk_CloseObj
     }
  }
#endif

 ,{ Type_Directory, "helios",
      { 
	Helios_InitServer,  
	Drive_TidyServer,   
	Drive_Private,
	(VoidConFnPtr)Drive_Testfun,
	Drive_Open,         
	Drive_Create,       
	Drive_Locate,
	Drive_ObjectInfo,   
	Drive_ServerInfo,   
	Drive_Delete,    
	Drive_Rename,       
	Drive_Link,         
	Drive_Protect,
	Drive_SetDate,      
	Drive_Refine,       
	Drive_CloseObj
      }
  }

 ,{ Type_File, "logger",
      { 
	Logger_InitServer,  
	Logger_TidyServer,   
	Logger_Private,
	(VoidConFnPtr)Logger_Testfun,
	Logger_Open,        
	Logger_Create,      
	Logger_Locate,
	Logger_ObjectInfo,  
	Logger_ServerInfo,  
	Logger_Delete,    
	Logger_Rename,      
	Logger_Link,        
	Logger_Protect,
	Logger_SetDate,     
	Logger_Refine,      
	Logger_CloseObj
      }
  }

#if !(drives_are_special)
 ,{ Type_Directory, "files",
      { 
	Drive_InitServer,   
	Drive_TidyServer,   
	Drive_Private,
	(VoidConFnPtr)Drive_Testfun,
	Drive_Open,         
	Drive_Create,       
	Drive_Locate,  
	Drive_ObjectInfo,   
	Drive_ServerInfo,   
	Drive_Delete,  
	Drive_Rename,       
	Drive_Link,         
	Drive_Protect,
	Drive_SetDate,      
	Drive_Refine,       
	Drive_CloseObj
      }
  }
#endif

 ,{ Type_File, "console",
     { 
	Console_InitServer,  
	Console_TidyServer,  
	Console_Private,
	(VoidConFnPtr)Console_Testfun,
	Console_Open,        
	Console_Create,      
	Console_Locate,
	Console_ObjectInfo,  
	Console_ServerInfo,  
	Console_Delete,   
	Console_Rename,      
	Console_Link,        
	Console_Protect,
	Console_SetDate,     
	Console_Refine,      
	Console_CloseObj
     }
  }

#if multiple_windows
 ,{ Type_Directory, "window",
     { 
	Window_InitServer,  
	Window_TidyServer,  
	Window_Private,
       	(VoidConFnPtr)Window_Testfun,
	Window_Open,        
	Window_Create,      
	Window_Locate,
	Window_ObjectInfo,  
	Window_ServerInfo,  
	Window_Delete,   
	Window_Rename,      
	Window_Link,        
	Window_Protect,
	Window_SetDate,     
	Window_Refine,      
	Window_CloseObj
     }
  }
#endif

#if Rawdisk_supported
 ,{ Type_Directory, "rawdisk",
     { 
	RawDisk_InitServer,  
	RawDisk_TidyServer,  
	RawDisk_Private,
	(VoidConFnPtr)RawDisk_Testfun,
	RawDisk_Open,        
	RawDisk_Create,      
	RawDisk_Locate,
	RawDisk_ObjectInfo,  
	RawDisk_ServerInfo,  
	RawDisk_Delete,
	RawDisk_Rename,      
	RawDisk_Link,        
	RawDisk_Protect,
	RawDisk_SetDate,     
	RawDisk_Refine,      
	RawDisk_CloseObj
     }
  }
#endif


 ,{ Type_Device, "clock", 
     { 
	Clock_InitServer,  
	Clock_TidyServer,   
	Clock_Private,
	(VoidConFnPtr)Clock_Testfun,
	Clock_Open,        
	Clock_Create,       
	Clock_Locate,
	Clock_ObjectInfo,  
	Clock_ServerInfo,   
	Clock_Delete,
	Clock_Rename,      
	Clock_Link,         
	Clock_Protect,
	Clock_SetDate,     
	Clock_Refine,       
	Clock_CloseObj
     }
  }

#if interaction_supported
 ,{ Type_File, machine_name, 
     { 
	Host_InitServer,  
	Host_TidyServer,   
	Host_Private,
	(VoidConFnPtr)Host_Testfun,
	Host_Open,        
	Host_Create,       
	Host_Locate,
	Host_ObjectInfo,  
	Host_ServerInfo,   
	Host_Delete,
	Host_Rename,      
	Host_Link,         
	Host_Protect,
	Host_SetDate,     
	Host_Refine,       
	Host_CloseObj
     }
  }
#endif

#if RS232_supported

 ,{ 
    Type_Directory, "rs232",
     { 
	RS232_InitServer,  
	RS232_TidyServer,   
	RS232_Private,
	(VoidConFnPtr)RS232_Testfun,
	RS232_Open,        
	RS232_Create,       
	RS232_Locate,
	RS232_ObjectInfo,  
	RS232_ServerInfo,   
	RS232_Delete,
	RS232_Rename,      
	RS232_Link,         
	RS232_Protect,
	RS232_SetDate,     
	RS232_Refine,       
	RS232_CloseObj
     }
  }

#endif

#if Centronics_supported

 ,{
    Type_Directory, "centronics",
     { 
	Centronics_InitServer,  
	Centronics_TidyServer, 
	Centronics_Private,
	(VoidConFnPtr)Centronics_Testfun,
	Centronics_Open,        
	Centronics_Create,     
	Centronics_Locate,
	Centronics_ObjectInfo,  
	Centronics_ServerInfo, 
	Centronics_Delete, 
	Centronics_Rename,      
	Centronics_Link,       
	Centronics_Protect,
	Centronics_SetDate,     
	Centronics_Refine,     
	Centronics_CloseObj
     }
  }

#endif

#if Printer_supported

 ,{ Type_Directory, "printers",
     { 
	Printer_InitServer,     
	Printer_TidyServer,    
	Printer_Private,
	(VoidConFnPtr)Printer_Testfun,
	Printer_Open,           
	Printer_Create,        
	Printer_Locate,
	Printer_ObjectInfo,     
	Printer_ServerInfo,    
	Printer_Delete,
	Printer_Rename,         
	Printer_Link,          
	Printer_Protect,
	Printer_SetDate,        
	Printer_Refine,        
	Printer_CloseObj
     }
  }

#endif

#if Midi_supported

 ,{ Type_Directory, "midi",
     { 
	Midi_InitServer,     
	Midi_TidyServer,    
	Midi_Private,
	(VoidConFnPtr)Midi_Testfun,
	Midi_Open,           
	Midi_Create,        
	Midi_Locate,
	Midi_ObjectInfo,     
	Midi_ServerInfo,    
	Midi_Delete,
	Midi_Rename,         
	Midi_Link,          
	Midi_Protect,
	Midi_SetDate,        
	Midi_Refine,        
	Midi_CloseObj
     }
  }

#endif

#if Ether_supported
  ,{ Type_File, "ether",
      { 
	Ether_InitServer, 
	Ether_TidyServer, 
	Ether_Private,
	(VoidConFnPtr)Ether_Testfun,
	Ether_Open, 
	Ether_Create, 
	Ether_Locate,
	Ether_ObjectInfo, 
	Ether_ServerInfo, 
	Ether_Delete,
	Ether_Rename, 
	Ether_Link, 
	Ether_Protect,
	Ether_SetDate, 
	Ether_Refine, 
	Ether_CloseObj
      }
  }

#endif

#if internet_supported

 ,{ Type_Directory, "internet",
     { 
	Internet_InitServer,    
	Internet_TidyServer,     
	Internet_Private,
	(VoidConFnPtr)Internet_Testfun,
	Internet_Open,          
	Internet_Create,         
	Internet_Locate,
	Internet_ObjectInfo,    
	Internet_ServerInfo,     
	Internet_Delete,
	Internet_Rename,        
	Internet_Link,           
	Internet_Protect,
	Internet_SetDate,       
	Internet_Refine,         
	Internet_CloseObj
     }
  }
#endif

#if mouse_supported

 ,{ Type_File, "mouse",
     { 
	Mouse_InitServer,  
	Mouse_TidyServer,   
	Mouse_Private,
	(VoidConFnPtr)Mouse_Testfun,
	Mouse_Open,        
	Mouse_Create,       
	Mouse_Locate,
	Mouse_ObjectInfo,  
	Mouse_ServerInfo,   
	Mouse_Delete,
	Mouse_Rename,      
	Mouse_Link,         
	Mouse_Protect,
	Mouse_SetDate,     
	Mouse_Refine,       
	Mouse_CloseObj
     }
  }

#endif

#if keyboard_supported

 ,{ Type_File, "keyboard",
     { 
	Keyboard_InitServer,  
	Keyboard_TidyServer, 
	Keyboard_Private,
	(VoidConFnPtr)Keyboard_Testfun,
	Keyboard_Open,        
	Keyboard_Create,     
	Keyboard_Locate,
	Keyboard_ObjectInfo,  
	Keyboard_ServerInfo, 
	Keyboard_Delete,
	Keyboard_Rename,      
	Keyboard_Link,       
	Keyboard_Protect,
	Keyboard_SetDate,     
	Keyboard_Refine,     
	Keyboard_CloseObj
     }
  }

#endif

#if gem_supported

 ,{ Type_File, "gem",  
     { 
	Gem_InitServer,  
	Gem_TidyServer,   
	Gem_Private,
	(VoidConFnPtr)Gem_Testfun,
	Gem_Open,        
	Gem_Create,       
	Gem_Locate,
	Gem_ObjectInfo,  
	Gem_ServerInfo,   
	Gem_Delete,
	Gem_Rename,      
	Gem_Link,         
	Gem_Protect,
	Gem_SetDate,     
	Gem_Refine,       
	Gem_CloseObj
     }
  }
#endif

#if X_supported

 ,{ Type_Directory, "x",
     { 
	X_InitServer,    
	X_TidyServer,     
	X_Private,
	(VoidConFnPtr)X_Testfun,
	X_Open,          
	X_Create,         
	X_Locate,
	X_ObjectInfo,    
	X_ServerInfo,     
	X_Delete,
	X_Rename,        
	X_Link,           
	X_Protect,
	X_SetDate,       
	X_Refine,         
	X_CloseObj
     }
  }
#endif

#if Network_supported

 ,{ Type_File, "NetworkController",
     { 
	Network_InitServer,     
	Network_TidyServer,    
	Network_Private,
	(VoidConFnPtr)Network_Testfun,
	Network_Open,           
	Network_Create,        
	Network_Locate,
	Network_ObjectInfo,     
	Network_ServerInfo,    
	Network_Delete,
	Network_Rename,         
	Network_Link,          
	Network_Protect,
	Network_SetDate,        
	Network_Refine,        
	Network_CloseObj
     }
  }
#endif

#if (MSWINDOWS)
   ,{ Type_Directory, "graphics",
      {
	
	Graph_InitServer,   
	Graph_TidyServer,	
	Graph_Private,
	(VoidConFnPtr)Graph_Testfun,
	Graph_Open,	    
	Graph_Create,	
	Graph_Locate,
	Graph_ObjectInfo,   
	Graph_ServerInfo,	
	Graph_Delete,
	Graph_Rename,	    
	Graph_Link, 	
	Graph_Protect,
	Graph_SetDate,	    
	Graph_Refine,	
	Graph_CloseObj
      }
    }
#endif

 ,{ 0L, NULL,  
     {  (VoidConFnPtr) NULL, (VoidConFnPtr) NULL, (VoidConFnPtr) NULL, (VoidConFnPtr) NULL,
	(VoidConFnPtr) NULL, (VoidConFnPtr) NULL, (VoidConFnPtr) NULL, (VoidConFnPtr) NULL,
	(VoidConFnPtr) NULL, (VoidConFnPtr) NULL, (VoidConFnPtr) NULL, (VoidConFnPtr) NULL,
	(VoidConFnPtr) NULL, (VoidConFnPtr) NULL, (VoidConFnPtr) NULL, (VoidConFnPtr) NULL
      }
  }

};

#if drives_are_special
VoidConFnPtr Drive_Handlers[handler_max] =
      { 
	Drive_InitServer,   
	Drive_TidyServer,   
	Drive_Private,
	(VoidConFnPtr)Drive_Testfun,
	Drive_Open,         
	Drive_Create,       
	Drive_Locate,
	Drive_ObjectInfo,   
	Drive_ServerInfo,   
	Drive_Delete,
	Drive_Rename,       
	Drive_Link,         
	Drive_Protect,
	Drive_SetDate,      
	Drive_Refine,       
	Drive_CloseObj
      };
#endif

VoidConFnPtr IOPROC_Handlers[Stream_max] =
      { 
	(VoidConFnPtr)IOPROC_InitStream, 
	(VoidConFnPtr)IOPROC_TidyStream,
        IOPROC_PrivateStream,
        IOPROC_Read,        
	IOPROC_Write,          
	IOPROC_GetSize,
	IOPROC_SetSize,     
	IOPROC_Close,          
	IOPROC_Seek,
	IOPROC_GetAttr,     
	IOPROC_SetAttr,        
	IOPROC_EnableEvents,
	IOPROC_Acknowledge, 
	IOPROC_NegAcknowledge, 
	IOPROC_Select
      };

VoidConFnPtr Logger_Handlers[Stream_max] =
      { 
	(VoidConFnPtr)Logger_InitStream,  
	(VoidConFnPtr)Logger_TidyStream,
        Logger_PrivateStream,
        Logger_Read,        
	Logger_Write,          
	Logger_GetSize,
	Logger_SetSize,     
	Logger_Close,          
	Logger_Seek,
	Logger_GetAttr,     
	Logger_SetAttr,        
	Logger_EnableEvents,
	Logger_Acknowledge, 
	Logger_NegAcknowledge, 
	Logger_Select
      };


VoidConFnPtr Console_Handlers[Stream_max] =
      { 
	(VoidConFnPtr)Console_InitStream,  
	(VoidConFnPtr)Console_TidyStream,
        Console_PrivateStream,
	Console_Read,        
	Console_Write,          
	Console_GetSize,
	Console_SetSize,     
	Console_Close,          
	Console_Seek,
	Console_GetAttr,     
	Console_SetAttr,        
	Console_EnableEvents,
	Console_Acknowledge, 
	Console_NegAcknowledge, 
	Console_Select
      };

#if multiple_windows
VoidConFnPtr WindowDir_Handlers[Stream_max] =
      { 
	(VoidConFnPtr)WindowDir_InitStream, 
	(VoidConFnPtr)WindowDir_TidyStream,
	WindowDir_PrivateStream,
	WindowDir_Read,         
	WindowDir_Write,        
	WindowDir_GetSize,
	WindowDir_SetSize,      
	WindowDir_Close,        
	WindowDir_Seek,
	WindowDir_GetAttr,      
	WindowDir_SetAttr,      
	WindowDir_EnableEvents,
	WindowDir_Acknowledge,  
	WindowDir_NegAcknowledge, 
	WindowDir_Select
      };
#endif

#if Rawdisk_supported

VoidConFnPtr Rawdisk_Handlers[Stream_max] =
      { 
	(VoidConFnPtr)RawDisk_InitStream,  
	(VoidConFnPtr)RawDisk_TidyStream,
	RawDisk_PrivateStream,
	RawDisk_Read,          
	RawDisk_Write,        
	RawDisk_GetSize,
	RawDisk_SetSize,       
	RawDisk_Close,        
	RawDisk_Seek,
	RawDisk_GetAttr,       
	RawDisk_SetAttr,      
	RawDisk_EnableEvents,
	RawDisk_Acknowledge,   
	RawDisk_NegAcknowledge, 
	RawDisk_Select
      };
#endif
#if Romdisk_supported

VoidConFnPtr Romdisk_Handlers[Stream_max] =
      { 
	(VoidConFnPtr)RomDisk_InitStream,  
	(VoidConFnPtr)RomDisk_TidyStream,
	RomDisk_PrivateStream,
	RomDisk_Read,          
	RomDisk_Write,        
	RomDisk_GetSize,
	RomDisk_SetSize,       
	RomDisk_Close,        
	RomDisk_Seek,
	RomDisk_GetAttr,       
	RomDisk_SetAttr,      
	RomDisk_EnableEvents,
	RomDisk_Acknowledge,   
	RomDisk_NegAcknowledge, 
	RomDisk_Select
      };
#endif

#if gem_supported
VoidConFnPtr Gem_Handlers[Stream_max] =
      { 
	(VoidConFnPtr)Gem_InitStream,  
	(VoidConFnPtr)Gem_TidyStream,
	Gem_PrivateStream,
	Gem_Read,        
	Gem_Write,          
	Gem_GetSize,
	Gem_SetSize,     
	Gem_Close,          
	Gem_Seek,
	Gem_GetAttr,     
	Gem_SetAttr,        
	Gem_EnableEvents,
	Gem_Acknowledge, 
	Gem_NegAcknowledge, 
	Gem_Select
      };
#endif

#if interaction_supported
VoidConFnPtr Host_Handlers[Stream_max] =
      { 
	(VoidConFnPtr)Host_InitStream,  
	(VoidConFnPtr)Host_TidyStream,
	Host_PrivateStream,
	Host_Read,        
	Host_Write,          
	Host_GetSize,
	Host_SetSize,     
	Host_Close,          
	Host_Seek,
	Host_GetAttr,     
	Host_SetAttr,        
	Host_EnableEvents,
	Host_Acknowledge, 
	Host_NegAcknowledge, 
	Host_Select
      };
#endif


#if (Ports_used || Rawdisk_supported)
VoidConFnPtr PortDir_Handlers[Stream_max] =
      { 
	(VoidConFnPtr)PortDir_InitStream, 
	(VoidConFnPtr)PortDir_TidyStream,
	PortDir_PrivateStream,
	PortDir_Read,         
	PortDir_Write,          
	PortDir_GetSize,
	PortDir_SetSize,      
	PortDir_Close,          
	PortDir_Seek,
	PortDir_GetAttr,      
	PortDir_SetAttr,        
	PortDir_EnableEvents,
	PortDir_Acknowledge,  
	PortDir_NegAcknowledge, 
	PortDir_Select
      };
#endif

#if RS232_supported
VoidConFnPtr RS232_Handlers[Stream_max] =
      { 
	(VoidConFnPtr)RS232_InitStream, 
	(VoidConFnPtr)RS232_TidyStream,
	RS232_PrivateStream,
	RS232_Read,         
	RS232_Write,          
	RS232_GetSize,
	RS232_SetSize,      
	RS232_Close,          
	RS232_Seek,
	RS232_GetAttr,      
	RS232_SetAttr,        
	RS232_EnableEvents,
	RS232_Acknowledge,  
	RS232_NegAcknowledge, 
	RS232_Select
      };
#endif

#if Centronics_supported
VoidConFnPtr Centronics_Handlers[Stream_max] =
      { 
	(VoidConFnPtr)Centronics_InitStream, 
	(VoidConFnPtr)Centronics_TidyStream,
	Centronics_PrivateStream,
	Centronics_Read,        
	Centronics_Write,       
	Centronics_GetSize,
	Centronics_SetSize,     
	Centronics_Close,       
	Centronics_Seek,
	Centronics_GetAttr,     
	Centronics_SetAttr,     
	Centronics_EnableEvents,
	Centronics_Acknowledge, 
	Centronics_NegAcknowledge, 
	Centronics_Select
      };
#endif

#if Printer_supported
VoidConFnPtr Printer_Handlers[Stream_max] =
      { 
	(VoidConFnPtr)Printer_InitStream, 
	(VoidConFnPtr)Printer_TidyStream,
	Printer_PrivateStream,
	Printer_Read,           
	Printer_Write,          
	Printer_GetSize,
	Printer_SetSize,        
	Printer_Close,          
	Printer_Seek,
	Printer_GetAttr,        
	Printer_SetAttr,        
	Printer_EnableEvents,
	Printer_Acknowledge,    
	Printer_NegAcknowledge, 
	Printer_Select 
     };
#endif

#if Midi_supported
VoidConFnPtr Midi_Handlers[Stream_max] =
      { 
	(VoidConFnPtr)Midi_InitStream, 
	(VoidConFnPtr)Midi_TidyStream,
	Midi_PrivateStream,
	Midi_Read,           
	Midi_Write,          
	Midi_GetSize,
	Midi_SetSize,        
	Midi_Close,          
	Midi_Seek,
	Midi_GetAttr,        
	Midi_SetAttr,        
	Midi_EnableEvents,
	Midi_Acknowledge,    
	Midi_NegAcknowledge, 
	Midi_Select
      };
#endif

#if Ether_supported
VoidConFnPtr Ether_Handlers[Stream_max] =
      { 
	(VoidConFnPtr)Ether_InitStream, 
	(VoidConFnPtr)Ether_TidyStream,
	Ether_PrivateStream,
	Ether_Read,          
	Ether_Write,          
	Ether_GetSize,
	Ether_SetSize,       
	Ether_Close,          
	Ether_Seek,
	Ether_GetAttr,       
	Ether_SetAttr,        
	Ether_EnableEvents,
	Ether_Acknowledge,   
	Ether_NegAcknowledge, 
	Ether_Select
     };
#endif

#if mouse_supported
VoidConFnPtr Mouse_Handlers[Stream_max] =
      { 
	(VoidConFnPtr)Mouse_InitStream, 
	(VoidConFnPtr)Mouse_TidyStream,
	Mouse_PrivateStream,
	Mouse_Read,         
	Mouse_Write,          
	Mouse_GetSize,
	Mouse_SetSize,      
	Mouse_Close,          
	Mouse_Seek,
	Mouse_GetAttr,      
	Mouse_SetAttr,        
	Mouse_EnableEvents,
	Mouse_Acknowledge,  
	Mouse_NegAcknowledge, 
	Mouse_Select
      };
#endif

#if keyboard_supported
VoidConFnPtr Keyboard_Handlers[Stream_max] =
      { 
	(VoidConFnPtr)Keyboard_InitStream, 
	(VoidConFnPtr)Keyboard_TidyStream,
	Keyboard_PrivateStream,
	Keyboard_Read,        
	Keyboard_Write,        
	Keyboard_GetSize,
	Keyboard_SetSize,     
	Keyboard_Close,        
	Keyboard_Seek,
	Keyboard_GetAttr,     
	Keyboard_SetAttr,      
	Keyboard_EnableEvents,
	Keyboard_Acknowledge, 
	Keyboard_NegAcknowledge, 
	Keyboard_Select
      };
#endif

#if X_supported
VoidConFnPtr XDir_Handlers[Stream_max] =
      { 
	(VoidConFnPtr)XDir_InitStream, 
	(VoidConFnPtr)XDir_TidyStream,
	XDir_PrivateStream,
	XDir_Read,            
	XDir_Write,            
	XDir_GetSize,
	XDir_SetSize,         
	XDir_Close,            
	XDir_Seek,
	XDir_GetAttr,         
	XDir_SetAttr,          
	XDir_EnableEvents,
	XDir_Acknowledge,     
	XDir_NegAcknowledge,   
	XDir_Select
      };

VoidConFnPtr X_Handlers[Stream_max] =
      { (VoidConFnPtr)X_InitStream, 
	(VoidConFnPtr)X_TidyStream,
	X_PrivateStream,
	X_Read,            
	X_Write,            
	X_GetSize,
	X_SetSize,         
	X_Close,            
	X_Seek,
	X_GetAttr,         
	X_SetAttr,          
	X_EnableEvents,
	X_Acknowledge,     
	X_NegAcknowledge,   
	X_Select 
      };

#endif /* X_supported */

#if Network_supported
VoidConFnPtr Network_Handlers[Stream_max] =
      { 
	(VoidConFnPtr)Network_InitStream, 
	(VoidConFnPtr)Network_TidyStream,
	Network_PrivateStream,
	Network_Read,        
	Network_Write,          
	Network_GetSize,
	Network_SetSize,     
	Network_Close,          
	Network_Seek,
	Network_GetAttr,     
	Network_SetAttr,        
	Network_EnableEvents,
	Network_Acknowledge, 
	Network_NegAcknowledge, 
	Network_Select
      };
#endif
                
VoidConFnPtr File_Handlers[Stream_max] =
      { 
	(VoidConFnPtr)File_InitStream,  
	(VoidConFnPtr)File_TidyStream,
	File_PrivateStream,
	File_Read,        
	File_Write,          
	File_GetSize,
	File_SetSize,     
	File_Close,          
	File_Seek,
	File_GetAttr,     
	File_SetAttr,        
	File_EnableEvents,
	File_Acknowledge, 
	File_NegAcknowledge, 
	File_Select
      };

VoidConFnPtr Dir_Handlers[Stream_max] =
      { 
	(VoidConFnPtr)Dir_InitStream, 
	(VoidConFnPtr)Dir_TidyStream,
	Dir_PrivateStream,
	Dir_Read,         
	Dir_Write,          
	Dir_GetSize,
	Dir_SetSize,      
	Dir_Close,          
	Dir_Seek,
	Dir_GetAttr,      
	Dir_SetAttr,        
	Dir_EnableEvents,
	Dir_Acknowledge,  
	Dir_NegAcknowledge, 
	Dir_Select
      };

#if internet_supported
VoidConFnPtr InternetDir_Handlers[Stream_max] =
      { 
	(VoidConFnPtr)InternetDir_InitStream, 
	(VoidConFnPtr)InternetDir_TidyStream,
	InternetDir_PrivateStream,
	InternetDir_Read,        
	InternetDir_Write,         
	InternetDir_GetSize,
	InternetDir_SetSize,     
	InternetDir_Close,         
	InternetDir_Seek,
	InternetDir_GetAttr,     
	InternetDir_SetAttr,  
	InternetDir_EnableEvents,
	InternetDir_Acknowledge,
	InternetDir_NegAcknowledge,
	InternetDir_Select 
      };

VoidConFnPtr Internet_Handlers[Stream_max] =
      { 
	(VoidConFnPtr)Internet_InitStream,  
	(VoidConFnPtr)Internet_TidyStream,
	Internet_PrivateStream,
	Internet_Read,            
	Internet_Write,         
	Internet_GetSize,
	Internet_SetSize,         
	Internet_Close,         
	Internet_Seek,
	Internet_GetInfo,         
	Internet_SetInfo,       
	Internet_EnableEvents,
	Internet_Acknowledge,     
	Internet_NegAcknowledge,
	Internet_Select
      };

#endif /* Internet_supported */

#if (MSWINDOWS)
VoidConFnPtr   Graph_Handlers[Stream_max] =
      {
     	(VoidConFnPtr)Graph_InitStream,   
	(VoidConFnPtr)Graph_TidyStream,
	Graph_PrivateStream,
	Graph_Read,	     
	Graph_Write,	   
	Graph_GetSize,
	Graph_SetSize,	     
	Graph_Close,	   
	Graph_Seek,
	Graph_GetAttr,	     
	Graph_SetAttr,	   
	Graph_EnableEvents,
	Graph_Acknowledge,      
	Graph_NegAcknowledge, 
	Graph_Select
     };
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
#if SOLARIS
extern jmp_buf *      	exit_jmpbuf;
#define longjmp_exit	longjmp (*exit_jmpbuf, 1)
#else
extern jmp_buf      	exit_jmpbuf;
#define longjmp_exit	longjmp (exit_jmpbuf, 1)
#endif
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

extern VoidConFnPtr Dir_Handlers[];
extern VoidConFnPtr File_Handlers[];
extern VoidConFnPtr IOPROC_Handlers[];
extern VoidConFnPtr Console_Handlers[];
extern VoidConFnPtr Logger_Handlers[];
#if multiple_windows
extern VoidConFnPtr WindowDir_Handlers[];
#endif

#if Rawdisk_supported
extern VoidConFnPtr Rawdisk_Handlers[];
#endif
#if Romdisk_supported
extern VoidConFnPtr Romdisk_Handlers[];
#endif

#if gem_supported
extern VoidConFnPtr Gem_Handlers[];
#endif
#if interaction_supported
extern VoidConFnPtr Host_Handlers[];
#endif
#if (Ports_used || Rawdisk_supported)
extern VoidConFnPtr PortDir_Handlers[];
#endif
#if RS232_supported
extern VoidConFnPtr RS232_Handlers[];
#endif
#if Centronics_supported
extern VoidConFnPtr Centronics_Handlers[];
#endif
#if Printer_supported
extern VoidConFnPtr Printer_Handlers[];
#endif
#if Midi_supported
extern VoidConFnPtr Midi_Handlers[];
#endif
#if Ether_supported
extern VoidConFnPtr Ether_Handlers[];
#endif
#if mouse_supported
extern VoidConFnPtr Mouse_Handlers[];
#endif
#if keyboard_supported
extern VoidConFnPtr Keyboard_Handlers[];
#endif
#if X_supported
extern VoidConFnPtr XDir_Handlers[];
extern VoidConFnPtr X_Handlers[];
#endif
#if Network_supported
extern VoidConFnPtr Network_Handlers[];
#endif
#if internet_supported
extern VoidConFnPtr InternetDir_Handlers[];
extern VoidConFnPtr Internet_Handlers[];
#endif
#if (MSWINDOWS)
extern VoidConFnPtr Graph_Handlers[];
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
--               Copyright (C) 1987, Perihelion Software Ltd.           --
--                          All Rights Reserved.                        --
--                                                                      --
--  pclocal.h                                                           --
--                                                                      --
--  Author:  BLV 23/5/88                                                --
--                                                                      --
------------------------------------------------------------------------*/
/* RcsId: $Id# */
/* Copyright (C) 1987, Perihelion Software Ltd. 			*/

/**
*** Some manifests and declarations needed for the ST and PC filing systems.
***
*** Drive_len is the length of a drive identifier, e.g. C:, and under
*** MSdos and TOS drives have to be treated separately from subdirectories.
*** Hence I need a quick check to see whether a name refers to a drive or
*** to something else, and this check is to compare the length of the local
*** name local_name with drive_len.
***
*** The various search and type manifests are needed with Fsfirst() and
*** Fsnext() system calls. When searching a drive the search_VolLabel is
*** required as an arguments, whereas search_FileOrDir is used with a 
*** subdirectory or a file. The calls put some information about the file
*** or directory into a static searchbuffer, with the structure shown below.
*** One of the fields, attr, can take values of FileAttr_Dir or
*** FileAttr_File.
***
*** Unfortunately I do not know where the search calls put their information
*** normally, so I have to provide my own buffer and indicate this to the
*** system by a call to Setfdta() in module server.c .
***
**/
#define drive_len      2     /* the length of a drive identifier string, d: */

#define search_VolLabel    8
#define search_FileOrDir  16

#define FileAttr_Dir      16
#define FileAttr_File      0

#define OpenMode_ReadOnly   0L
#define OpenMode_WriteOnly  1L
#define OpenMode_ReadWrite  2L

typedef struct { char          junk[21];   /* MSdos private info */
                 byte          attr;       /* file attributes */
                 unsigned int  time;       /* time stamp */
                 unsigned int  date;       /* date stamp */
                 word          size;       /* length in bytes */
                 char          name[13];
} searchinfo;

#ifdef Files_Module
searchinfo searchbuffer;
#else
extern searchinfo searchbuffer;
#endif

PUBLIC word fn( get_drives,       (word *));
PUBLIC word fn( create_file,      (char *));
PUBLIC word fn( get_file_info,    (char *, ObjInfo *));
PUBLIC word fn( rename_object,    (char *, char *));
PUBLIC word fn( set_file_date,    (char *, word));
PUBLIC word fn( get_drive_info,   (char *, servinfo *));
PUBLIC int  fn( open_file,        (char *, word));
PUBLIC word fn( get_file_size,    (int, word));
PUBLIC word fn( write_to_file,    (int, word, byte *));
PUBLIC word fn( read_from_file,   (int, word, byte *));
PUBLIC word fn( search_directory, (char *, List *));
PUBLIC word fn( seek_in_file,     (int, word, int));
extern void check_helios_name(char *);

/**
*** a macro for clear bytes, memset being supported by Microsoft C
**/
#define clear_bytes(a, b) (void) memset((void *) a, 0, (size_t) b)

/**
*** Support for the RS232 lines. This is shared between the /rs232
*** device and the VY86PID link interface routines.
**/
#define MaxRS232     7
#if 1
#define overflow_max 256
#else
#define overflow_max 8192
#endif

typedef struct RS232 { 
	byte		*inbuf;
	byte		*outbuf;
	Attributes	*attr;
	Port		 breakint;
	Port		 modemint;
	int		 id;
	uint		 flags;
	uint		 incount;
	uint		 inmax;
	uint		 outcount;
	uint		 outmax;
	int		 overflow_count;
	int		 port_base;
	int		 current_mcr;
	int		 current_lcr;
	int		 LCR;	/* for restoring	*/
	int		 BAUDH;
	int		 BAUDL;
	int		 IER;
	int		 MCR;
	byte		 overflow_buf[overflow_max];
} RS232;

/**
*** Flags for the Com structure
**/
#define RS232_inXOFF           0x01
#define RS232_outXOFF          0x02
#define RS232_BreakDetected    0x04
#define RS232_UseHardware      0x08
#define RS232_ClearToWrite     0x10
#define RS232_NeedToXoff       0x20
#define RS232_UseInXoff        0x40
#define RS232_UseOutXoff       0x80
#define RS232_StripIn          0x100
#define RS232_IgnorePar        0x200
#define RS232_ParMark          0x400
#define RS232_ParCheck         0x800
#define RS232_NeedToXon        0x1000
#define RS232_TXEmpty          0x2000
#define RS232_CTS              0x4000
#define RS232_RingDetected     0x8000

#define XONByte                0x11
#define XOFFByte               0x13

             /* hardware handshaking support */
#define RTS                    0x02
#define DTR                    0x01
#define CTS                    0x10
#define Ring                   0x40

extern word RS232_send(ComsPort *, word, UBYTE *);
extern word RS232_pollwrite(ComsPort *);
extern word RS232_abortwrite(ComsPort *);
extern word RS232_receive(ComsPort *, word, UBYTE *);
extern word RS232_pollread(ComsPort *);
extern word RS232_abortread(ComsPort *);
extern void RS232_done(ComsPort *);
extern void RS232_configure(ComsPort *);
extern void RS232_error_handler(void);
extern void RS232_control_fifo(ComsPort *, bool);

extern RS232    *RS232_table;
extern ComsPort *RS232_coms;
#define Default_Port	MaxRS232
#define VY86PID_Port	MaxRS232 + 1

#if _MSC_VER <= 600
#define	_segread	segread
#define _REGS		REGS
#define _SREGS		SREGS
#define _int86x		int86x
#define _intdos		intdos
#define _mkdir		mkdir
#define _rmdir		rmdir
#define _lseek		lseek
#endif

#if _MSC_VER > 600
#define	fileno	_fileno
#endif


