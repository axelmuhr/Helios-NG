
/* hostsys.h: specify details of host system when compiling CLIB */
/* Copyright (C) Codemist Ltd, 1988             */
/* version 0.01a, amalgamates norcrosys+sysdep: */

#ifndef __hostsys_h
#define __hostsys_h

#undef MACHINE
#ifdef __clipper
#  define BYTESEX_EVEN 1
#  define MACHINE "CLIPPER"
#  define OTHER_WORD_ORDER_FOR_FP_NUMBERS
#endif

#ifdef __sparc
#  define BYTESEX_ODD 1
#  define MACHINE "SPARC"
#endif

#ifdef __titan
#  define BYTESEX_ODD 1
#  define MACHINE "TITAN"
#endif

#ifndef MACHINE
#  error -Dibm370 assumed
#  define ibm370 1
#  define BYTESEX_ODD 1
#  define MACHINE "370"
#endif

#define memclr(p,n) memset(p,0,n)

#  define MAXSTORE 0x03ffffff       /* used only by alloc.c */
#  define HOST_LACKS_ALLOC 1

extern int _interrupts_off;
extern void _raise_stacked_interrupts(void);
extern void _postmortem(void);
extern void _mapstore(void);
extern void _write_profile(char *filename);
extern void _sysdie(const char *s);
extern void _init_alloc(void), _initio(char*, char*, char*),
            _terminateio(void), _lib_shutdown(void), _signal_init(void),
            _exit_init(void);

extern int _signal_real_handler(int sig);

#ifndef __size_t
#define __size_t 1
typedef unsigned int size_t;  /* see <stddef.h> */
#endif
extern void *_sys_alloc(size_t n);
extern void _init_user_alloc(void);
extern void _terminate_user_alloc(void);
extern void _sys_msg(const char *);
#define _exit(n) _syscall1(SYS_exit,n)

typedef int FILEHANDLE;
#ifdef __sparc
extern int __sdiv10(int);
#define _kernel_sdiv10(x) __sdiv10(x)
#define _kernel_sdiv(x,y) ((y)/(x))
extern unsigned int __udiv10(unsigned int);
#define _kernel_udiv10(v) __udiv10(v)
#else
#define _kernel_sdiv10(x) ((x)/10)
#define _kernel_sdiv(x,y) ((y)/(x))
#define _kernel_udiv10(v) ((unsigned)((v))/10)
#endif

#define IEEE 1

/* I/O stuff... */

extern FILEHANDLE _sys_open(const char *name, int openmode);

#ifdef __clipper

/* definitions of required macros and functions */
#include "/users/jpff/ncc.include/syscall.h"    /* for indirect calls    */

extern int _syscall0(int);
extern int _syscall1(int, int);
extern int _syscall2(int, int, int);
extern int _syscall3(int, int, int, int);

#define NONHANDLE ((FILEHANDLE)(-1))
#define _TIOCGETD (0x40000000|(sizeof(int)<<16)|('t'<<8))
#ifndef _SGTTYB_
#define	_SGTTYB_
typedef struct sgttyb {
	char	sg_ispeed;		/* input speed */
	char	sg_ospeed;		/* output speed */
	char	sg_erase;		/* erase character */
	char	sg_kill;		/* kill character */
	short	sg_flags;		/* mode flags */
} sgttyb;
#endif
#define _sys_istty_(fh) (_syscall3(SYS_ioctl,(int)(fh),_TIOCGETD,(int)(&isabuf))==0?1:0)
#define _sys_seek_(fh, pos) _syscall3(SYS_lseek, (int)fh, (int)pos, 0)
#define _sys_ensure_(fh) _syscall3(SYS_write, 0, (int)"ensure\n", 7)
extern int _fh_length(FILEHANDLE);
#define _sys_flen_(fh) _fh_length(fh)
#define _ttywrite(buf, len, wh) _syscall3(SYS_write, 0, (int)(buf), len)
#define _sys_write_(fh, buf, len, mode) (_syscall3(SYS_write, (int)fh, (int)buf, (int)len)==len?0:-1)
extern int _sys_read_(FILEHANDLE fh, unsigned char *buf, int len, int mode);
#define _sys_close_(fh) _syscall1(SYS_close, (int)(fh))
#define _sys_tmpnam_(name, sig) sprintf(name, "$.tmp.x%.8x", sig)
#endif

#ifdef __sparc

/* definitions of required macros and functions */
#include "sparccall.h"    /* for indirect calls    */

extern int _syscall0(int);
extern int _syscall1(int, int);
extern int _syscall2(int, int, int);
extern int _syscall3(int, int, int, int);

#define NONHANDLE ((FILEHANDLE)(-1))
extern int _sys_istty(int);
#define _sys_istty_(fh) (_sys_istty(fh)==0?1:0)
#define _sys_seek_(fh, pos) _syscall3(SYS_lseek, (int)fh, (int)pos, 0)
#define _sys_ensure_(fh) _syscall3(SYS_write, 0, (int)"ensure\n", 7)
extern int _fh_length(FILEHANDLE);
#define _sys_flen_(fh) _fh_length(fh)
#define _ttywrite(buf, len, wh) _syscall3(SYS_write, 0, (int)(buf), len)
#define _sys_write_(fh, buf, len, mode) (_syscall3(SYS_write, (int)fh, (int)buf, (int)len)==len?0:-1)
extern int _sys_read_(FILEHANDLE fh, unsigned char *buf, int len, int mode);
#define _sys_close_(fh) _syscall1(SYS_close, (int)(fh))
#define _sys_tmpnam_(name, sig) sprintf(name, "$.tmp.x%.8x", sig)
#endif

#ifdef __titan

/* definitions of required macros and functions */
#include "titancall.h"    /* for indirect calls    */

extern int _syscall0(int);
extern int _syscall1(int, int);
extern int _syscall2(int, int, int);
extern int _syscall3(int, int, int, int);

#define NONHANDLE ((FILEHANDLE)(-1))
extern int _sys_istty(int);
#define _sys_istty_(fh) (_sys_istty(fh)==0?1:0)
#define _sys_seek_(fh, pos) _syscall3(SYS_lseek, (int)fh, (int)pos, 0)
#define _sys_ensure_(fh) _syscall3(SYS_write, 0, (int)"ensure\n", 7)
extern int _fh_length(FILEHANDLE);
#define _sys_flen_(fh) _fh_length(fh)
#define _ttywrite(buf, len, wh) _syscall3(SYS_write, 0, (int)(buf), len)
#define _sys_write_(fh, buf, len, mode) (_syscall3(SYS_write, (int)fh, (int)buf, (int)len)==len?0:-1)
extern int _sys_read_(FILEHANDLE fh, unsigned char *buf, int len, int mode);
#define _sys_close_(fh) _syscall1(SYS_close, (int)(fh))
#define _sys_tmpnam_(name, sig) sprintf(name, "$.tmp.x%.8x", sig)
#endif

#ifdef __sparc
#  ifdef BYTESEX_EVEN
typedef union {struct {int mhi:20, x:11, s:1; unsigned mlo; } i;
               double d; } fp_number;
#  else
typedef union {struct {int s:1, x:11, mhi:20; unsigned mlo; } i;
               double d; } fp_number;
#  endif
#else
#  ifdef BYTESEX_EVEN
typedef union {struct {unsigned mlo; int mhi:20, x:11, s:1; } i;
               double d; } fp_number;
#  else
typedef union {struct {unsigned mlo; int s:1, x:11, mhi:20; } i;
               double d; } fp_number;
#  endif
#endif

/* the object of the following macro is to adjust the floating point     */
/* variables concerned so that the more significant one can be squared   */
/* with NO LOSS OF PRECISION. It is only used when there is no danger    */
/* of over- or under-flow.                                               */

/* This code is NOT PORTABLE but can be modified for use elsewhere       */
/* It should, however, serve for IEEE and IBM FP formats.                */

#define _fp_normalize(high, low)                                          \
    {   fp_number temp;        /* access to representation     */         \
        double temp1;                                                     \
        temp.d = high;         /* take original number         */         \
        temp.i.mlo = 0;        /* make low part of mantissa 0  */         \
        temp1 = high - temp.d; /* the bit that was thrown away */         \
        low += temp1;          /* add into low-order result    */         \
        high = temp.d;                                                    \
    }

/* The next line is not very nice, but since I want to declare a      */
/* function of type (FILE *) is seems to be needed. If you do not     */
/* want <stdio.h> included, tough luck!                               */
/* Note also the use of __system_io to alter the amount of detail     */
/* revealed by <stdio.h>.                                             */
#include <stdio.h>
extern FILE *_fopen_string_file(const char *data, int length);

#endif

/* end of hostsys.h */
