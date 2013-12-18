
/* hostsys.h: specify details of host system when compiling CLIB */
/* Copyright (C) A.C. Norman and A. Mycroft */
/* version 0.01a, amalgamates norcrosys+sysdep: */
/* soon rationalise to armsys.h, s370sys.h etc */
/* The tests of #ifdef ARM must go one day.    */

#ifndef __hostsys_h
#define __hostsys_h

#define __system_io 1           /* makes stdio.h declare more */


#undef MACHINE
#ifdef ARM
/* The following lines could be a #include "armsys.h" etc. */
#  define BYTESEX_EVEN 1
#  define MACHINE "ARM"
#endif
#ifdef ACW
#  define BYTESEX_EVEN 1
#  define MACHINE "ACW"
#endif
#ifdef ibm370
#  define BYTESEX_ODD 1
#  define MACHINE "370"
#endif
#ifdef AMIGA
#  define BYTESEX_ODD 1
#  define MACHINE "AMIGA"
/* #  define SHORT_ALIGNED_MOVES */
#endif
#if defined TRIPOS || defined NEWTRIPOS
#  define BYTESEX_ODD 1
#  define MACHINE "TRIPOS"
/* #  define SHORT_ALIGNED_MOVES */
#endif
#ifndef MACHINE
#  error -Dibm370 assumed
#  define ibm370 1
#  define BYTESEX_ODD 1
#  define MACHINE "370"
#endif

#define memclr(p,n) memset(p,0,n)

#ifdef ibm370
#  define MAXSTORE 0x00ffffff       /* used only by alloc.c */
#elif defined AMIGA
#  define MAXSTORE 0x00ffffff       /* used only by alloc.c */
#elif defined TRIPOS || defined NEWTRIPOS
#  define MAXSTORE 0x00ffffff       /* used only by alloc.c */
#else                               /* not right! */
#  define MAXSTORE 0x03ffffff       /* used only by alloc.c */
#  define HOST_LACKS_ALLOC 1
#endif

#ifdef AMIGA
#  define IEEE 1
/* #  define NO_FLOATING_POINT 1 */
#  define LINE_BUFFERED_TTYIO 1
#endif
#if defined TRIPOS || defined NEWTRIPOS
#  define IEEE 1
/* #  define NO_FLOATING_POINT 1 */
#  define LINE_BUFFERED_TTYIO 1
#endif

#ifdef TARGET_IS_68000
#  define SOFTWARE_FLOATING_POINT 1
#endif

#ifdef ARM                          /* fpe2 features stfp/ldfp ops */
#  define HOST_HAS_BCD_FLT 1
#  ifndef SOFTWARE_FLOATING_POINT
#    define HOST_HAS_TRIG 1         /* and ieee trig functions     */
#  endif
#endif

extern int _interrupts_off;
extern void _raise_stacked_interrupts(void);
extern void _postmortem(void);
extern void _mapstore(void);
extern void _write_profile(char *filename);
extern void _sysdie(const char *s);
extern void _initalloc(char *,char *), _initio(char *,char *,char *),
            _terminateio(void), _signal_init(void), _exit_init(void);
#ifndef size_t
#  define size_t unsigned int   /* see <stddef.h> */
#endif
extern void *_sys_alloc(size_t n);
extern void _sys_msg(const char *);

extern void _exit(int n);

#ifdef ARM
typedef int FILEHANDLE;
#define TTYFILENAME ":tt"
extern int _osbyte(int a, int x, int y, int c);
extern int _oswrch(int ch);
extern int _osbget(int fh);
extern int _osbput(int ch, int fh);
extern int _osgbpb(int op, int fh, void *base, int len, int extra);
extern int _osgbpb1(int op, int fh,
    struct _osgbpb1_control_block {void *base; int len; int extra;} *z);
extern int _osrdch(void);
extern int _osword(int op, int *data);
extern int _osfind(int op, char *name);
extern int _osfile(int op, const char *name, int loadaddr, int execaddr,
                                             int startaddr, int endaddr);
extern int _osfile1(int op, const char *name,
    struct _osfile1_control_block {int load; int exec; int start; int end;} *z);
extern int _osargs(int op, int fh, int arg);
extern int _oscli(const char *s);
extern int _ttywrite(unsigned char *buf, unsigned int len, int flag);
extern int _ttyread(unsigned char *buff, int size, int flag);
extern double _ldfp(void *x);
extern void _stfp(double d, void *p);
#endif

#ifdef AMIGA
#include "exec/types.h"
#include "exec/memory.h"
#include "exec/libraries.h"
#include "exec/tasks.h"
#include "libraries/dos.h"

typedef long FILEHANDLE;

/* Exec routines */
#define nfindtask(name) FindTask(name)
#define nsignal(task, signals) Signal(task, signals)
#define nsetexcept( newsignals, signalmask) SetExcept( newsignals, signalmask)
#define nallocmem(size, type) AllocMem(size, type)
#define nfreemem(p, s) FreeMem(p, s)
#define nopenlibrary(libName, version) OpenLibrary(libName, version)
#define nsetsignal(newsignals, signalmask) SetSignal(newsignals, signalmask)

extern struct Task    *FindTask(const char *name);
extern int             Signal(struct Task *task, int signals);
extern int             SetExcept(int newsignals, int signalmask);
extern void           *AllocMem(int, int);
extern void            FreeMem(void *, int);
extern struct Library *OpenLibrary(const char *libName, int version);
extern int             SetSignal(int newsignals, int signalmask);

/* Dos routines */

#define nclose(file) Close(file)
#define ndeletefile(name) DeleteFile(name)
#define nrenamefile(name1, name2) Rename(name1, name2)
#define ninput() Input()
#define nisinteractive(file) IsInteractive(file)
#define nopen(name, mode) Open(name, mode)
#define noutput() Output()
#define nread(file, buffer, length) Read(file, buffer, length)
#define nseek(file, position, mode) Seek(file, position, mode)
#define nwrite(file, buffer, length) Write(file, buffer, length)
#define ndatestamp(v) DateStamp(v)

extern void        Close(FILEHANDLE file);
extern int         DeleteFile(const char *name);
extern int         Rename(const char *name1, const char *name2);
extern FILEHANDLE  Input(void);
extern int         IsInteractive( FILEHANDLE file);
extern FILEHANDLE  Open( const char *name, int mode);
extern FILEHANDLE  Output(void);
extern int         Read(FILEHANDLE file, void *buffer, int length);
extern int         Seek(FILEHANDLE file, int position, int mode);
extern int         Write(FILEHANDLE file, void *buffer, int length);
extern void       *DateStamp(void *v);

#endif

#ifdef NEWTRIPOS
#include "kfunctions.h"
#include "dosfunctions.h"

typedef FileHandle *FILEHANDLE;

/* Kernel Routines */
#define nallocmem(size, type) GetMem(size)
#define nfreemem(p, s) FreeMem((void *)p)

/* Dos routines */

#define nclose(file) Close(file)
#define ndeletefile(name) DeleteFile(name)
#define nrenamefile(name1, name2) Rename(name1, name2)
#define ninput() Input()
#define nisinteractive(file) IsInteractive(file)
#define nopen(name, mode) Open(name, mode)
#define noutput() Output()
#define nread(file, buffer, length) Read(file, (BYTE *)buffer, length)
#define nseek(file, position, mode) Seek(file, position, mode)
#define nwrite(file, buffer, length) Write(file, (BYTE *)buffer, length)
#define ndatestamp(v) DateStamp((DateVec *)v)

#endif

#ifdef TRIPOS
typedef long FILEHANDLE;

/* Tripos interface */
/* These globals MUST be kept in step with chdr */
#define g_initio 50
#define ug 150
#define g_callmain ug
#define  g_delete (ug+1)
#define  g_input (ug+2)
#define  g_isinteractive (ug+3)
#define  g_open (ug+4)
#define  g_output (ug+5)
#define  g_read (ug+6)
#define  g_seek (ug+7)
#define  g_write (ug+8)
#define  g_datstamp (ug+9)
#define  g_close (ug+10)
#define  g_alloc (ug+11)
#define  g_free (ug+12)
#define  g_rename (ug+17)

typedef int GLOBNUM;
int callglb( GLOBNUM n, int nargs, ...);

#define tr_initio() callglb(g_initio,0)

#define nallocmem(n, type ) callglb(g_alloc, 1, n)
#define nfreemem(p,l) callglb(g_free, 1, p)

#define nclose(file) callglb(g_close, 1, file)
#define ndeletefile(name) callglb(g_delete, 1, name)
#define nrenamefile(name1, name2) callglb(g_rename, 2, name1, name2)
#define ninput() ((FILEHANDLE)(callglb(g_input,0)))
#define nisinteractive(file) callglb(g_isinteractive,1,file)
#define nopen(name, mode) ((FILEHANDLE)(callglb(g_open,2,name,mode)))
#define noutput(void) ((FILEHANDLE)(callglb(g_output,0)))
#define nread(file, buffer, length) callglb(g_read, 3, file, buffer, length)
#define nseek(file, position, mode) callglb(g_seek, 3, file, position, mode)
#define nwrite(file, buffer, length) callglb(g_write, 3, file, buffer, length)
#define ndatestamp(v) callglb(g_datstamp, 1, v)

#define Act_FindInput 1005
#define Act_FindOutput 1006

#define MODE_OLDFILE Act_FindInput
#define MODE_NEWFILE Act_FindOutput
#define MODE_UPDATE  Act_FindInput

#define TICKS_PER_SECOND 50
#endif

#ifdef ibm370
#  if ('A' == 193)
#    define atoe(x) (x)       /* ebcdic already.                */
#    define etoa(x) (x)
#  else
#    define atoe(x) _atoe[x]  /* else translate text files etc. */
#    define etoa(x) _etoa[x]
#  endif
extern char _etoa[], _atoe[];
extern void _abend(int);
extern void *_svc_getmain(int);
extern void _svc_freemain(void *, int);
struct _svcwto { short len, mcsflags;
                 char  msg[80];
                 short desccode, routcde; };
extern void _svc_wto(const struct _svcwto *);
struct _svctime { int csecs; int yday/* 0-365 */; int year; };
extern void _svc_time(struct _svctime *);
extern void  _svc_stimer(int);
extern unsigned  _svc_ttimer(void);             /* units of 1/38400 sec  */
/* the following lines use "struct NIOPBASE" instead of "NIOPBASE" to    */
/* to reduce syntactic confusion if "niopbase.h" not included            */
typedef struct NIOPBASE *FILEHANDLE;
extern int _io_call(int fn, struct NIOPBASE *p, int arg), _io_r0;
extern struct _svcwto _io_emsg;  /* beware only 64 bytes thereof */
#endif


#ifndef SOFTWARE_FLOATING_POINT
#  ifdef ibm370
#     define IBMFLOAT 1
#  else
#     define IEEE 1
/* IEEE floating point format assumed.                                   */
#     ifdef ARM
/* For the current ARM floating point system that Acorn use the first    */
/* word of a floating point value is the one containing the exponent.    */
#        undef OTHER_WORD_ORDER_FOR_FP_NUMBERS
#        define DO_NOT_SUPPORT_UNNORMALIZED_NUMBERS 1
#     endif
#  endif
#endif

/* I/O stuff... */

extern FILEHANDLE _sys_open(const char *name, int openmode);

#ifdef ARM

#define TTYHANDLE 0          /* maybe it is now time to switch these ... */
#define NONHANDLE (-1)       /* ... two values                           */
#define _sys_istty_(fh) \
   ((fh) == TTYHANDLE)
#define _sys_seek_(fh, pos) \
   ((fh) == TTYHANDLE ? 0 : _osargs(1, fh, pos))
#define _sys_flen_(fh) \
   _osargs(2, fh, 0)
#define _sys_write_(fh, buf, len, mode) \
   ((fh) == TTYHANDLE ? _ttywrite(buf,len,mode) : _osgbpb(2,fh,buf,len,0))
#define _sys_read_(fh, buf, len, mode)  \
   ((fh) == TTYHANDLE ? _ttyread(buf,len,mode) : _osgbpb(4,fh,buf,len,0))
#define _sys_close_(fh) \
   ((fh) == TTYHANDLE ? 0 : _osfind(0, (char *)fh))
#define _sys_tmpnam_(name, sig) sprintf(name, "$.tmp.x%.8x", sig)

#endif

#ifdef AMIGA
extern int _sys_seek_(FILEHANDLE,int);
extern int _sys_write_(FILEHANDLE,unsigned char *, int, int);
extern int _sys_istty_(FILEHANDLE);
extern int _sys_read_(FILEHANDLE,unsigned char *, int,int);
extern int _sys_close_(FILEHANDLE);
extern int _sys_flen_(FILEHANDLE);
#define _sys_tmpnam_(name, sig) sprintf(name, ":t/x%.8x", sig)
#define NONHANDLE ((FILEHANDLE)0)
#endif
#ifdef TRIPOS
extern int _sys_seek_(FILEHANDLE,int);
extern int _sys_write_(FILEHANDLE,unsigned char *, int, int);
extern int _sys_istty_(FILEHANDLE);
extern int _sys_read_(FILEHANDLE,unsigned char *, int,int);
extern int _sys_close_(FILEHANDLE);
extern int _sys_flen_(FILEHANDLE);
#define _sys_tmpnam_(name, sig) sprintf(name, ":t.x%.8x", sig)
#define NONHANDLE ((FILEHANDLE)0)
#endif
#ifdef NEWTRIPOS
extern int _sys_seek_(FILEHANDLE,int);
extern int _sys_write_(FILEHANDLE,unsigned char *, int, int);
extern int _sys_istty_(FILEHANDLE);
extern int _sys_read_(FILEHANDLE,unsigned char *, int,int);
extern int _sys_close_(FILEHANDLE);
extern int _sys_flen_(FILEHANDLE);
#define _sys_tmpnam_(name, sig) sprintf(name, ":t/x%.8x", sig)
#define NONHANDLE ((FILEHANDLE)0)
#endif

#ifdef ibm370

#define NONHANDLE ((FILEHANDLE)0)
#define _sys_istty_(fh) (((DCB *)(fh))->DCBDEVT==DCBDVTRM)
#define _sys_seek_(fh, pos) (_sysdie("Unimplemented fseek"), 0)
#define _sys_flen_(fh)      (_sysdie("Unimplemented filelen"), 0)

extern int _sys_write_(FILEHANDLE fh, unsigned char *buf, int len, int mode);
extern int _sys_read_(FILEHANDLE fh, unsigned char *buf, int len, int mode);
extern int _sys_close_(FILEHANDLE fh);
#define _sys_tmpnam_(name, sig) sprintf(name, "$.tmp.x%.8x", sig)

#endif

/* The following code is NOT PORTABLE but can stand as a prototype for   */
/* whatever makes sense on other machines.                               */

#ifdef IBMFLOAT
/* This version works with IBM 360 floating point.                       */

#  ifdef BYTESEX_EVEN
typedef union {struct {int mhi:24, x:7, s:1; unsigned mlo; } i;
               double d; } fp_number;
#  else
typedef union {struct {int s:1, x:7, mhi:24; unsigned mlo; } i;
               double d; } fp_number;
#  endif

#else
#  ifndef OTHER_WORD_ORDER_FOR_FP_NUMBERS
/* This version works with the ARM floating point emulator - it may have */
/* to be reworked when or if floating point hardware is installed        */

#    ifdef BYTESEX_EVEN
typedef union {struct {int mhi:20, x:11, s:1; unsigned mlo; } i;
               double d; } fp_number;
#    else /* 68000 case */
typedef union {struct {int s:1, x:11, mhi:20; unsigned mlo; } i;
               double d; } fp_number;
#    endif

#  else   /* OTHER_WORD_ORDER_FOR_FP_NUMBERS */
#    ifdef BYTESEX_EVEN
typedef union {struct {unsigned mlo; int mhi:20, x:11, s:1; } i;
               double d; } fp_number;
#    else
typedef union {struct {unsigned mlo; int s:1, x:11, mhi:20; } i;
               double d; } fp_number;
#    endif
#  endif  /* OTHER_WORD_ORDER_FOR_FP_NUMBERS */

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
