
/* nonansi.sysdep.h                                                      */
/* Copyright (C) A.C. Norman and A. Mycroft */
/* version 0.01 */

#ifndef __sysdep_h
#define __sysdep_h

#ifndef __nonansi_norcrosys_h       /* BLV addition */
#include "nonansi/norcrosys.h"
#endif

#ifndef SOFTWARE_FLOATING_POINT
#  ifdef IBM
#     define IBMFLOAT
#  elif defined __C40
#     define C40FLT	1
#  else
#     define IEEE	/* IEEE floating point format assumed. */
#     ifdef __ARM
/* For the current ARM floating point system that Acorn use the first    */
/* word of a floating point value is the one containing the exponent.    */
#        undef OTHER_WORD_ORDER_FOR_FP_NUMBERS
#        define DO_NOT_SUPPORT_UNNORMALIZED_NUMBERS 1
#     elif defined(__TRAN)
#	define OTHER_WORD_ORDER_FOR_FP_NUMBERS 1
#       define DO_NOT_SUPPORT_UNNORMALIZED_NUMBERS 1
#     endif
#  endif
#endif


/* I/O stuff... */

extern FILEHANDLE _sys_open(const char *name, int openmode);

#ifdef RISCOS
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
#endif

#ifdef __HELIOS
#define NONHANDLE ((FILEHANDLE) NULL)
extern int _sys_istty_(FILEHANDLE fh);
extern int _sys_seek_( FILEHANDLE fh, int pos);
extern int _sys_flen_( FILEHANDLE fh);
extern int _sys_write_(FILEHANDLE fh, unsigned char *buff, int len, int mode);
extern int _sys_read_( FILEHANDLE fh, unsigned char *buff, int len, int mode);
extern int _sys_close_(FILEHANDLE fh);
#endif

#  ifndef ibm370
#   ifndef __ARM
#    ifndef __C40
#     ifndef __TRAN
#      error -dibm370 assumed
#      define ibm370 1
#     endif
#    endif
#   endif
# endif

#ifdef ibm370

#define NONHANDLE ((FILEHANDLE)0)
#define _sys_istty_(fh) (((DCB *)(fh))->DCBDEVT==DCBDVTRM)
#define _sys_seek_(fh, pos) _sysdie("Unimplemented fseek")
#define _sys_flen_(fh)      (_sysdie("Unimplemented filelen"), 0)

extern int _sys_write_(FILEHANDLE fh, unsigned char *buf, int len, int mode);
extern int _sys_read_(FILEHANDLE fh, unsigned char *buf, int len, int mode);
extern int _sys_close_(FILEHANDLE fh);

#endif

/* The following code is NOT PORTABLE but can stand as a prototype for   */
/* whatever makes sense on other machines.                               */

#ifdef IBMFLOAT
/* This version works with IBM 360 floating point.                       */

#ifdef BYTESEX_EVEN
typedef union {struct {int mhi:24, x:7, s:1; int mlo; } i;
               double d; } fp_number;
#else
typedef union {struct {int s:1, x:7, mhi:24; int mlo; } i;
               double d; } fp_number;
#endif

#elif defined C40FLT
typedef union
  {
    struct
      {
	signed int	pad : 24;	/* padding (in fact the top 24 bits of the mantissa) */
	signed int	x   :  8;	/* exponent */
	unsigned int	m   : 31;	/* mantissa */
	unsigned int	s   :  1;	/* sign bit */
      }
    i;
    
    double d;
  }
fp_number;
#else  /* !IBMFLOAT  and !C40FLT */

#ifndef OTHER_WORD_ORDER_FOR_FP_NUMBERS
/* This version works with the ARM floating point emulator - it may have */
/* to be reworked when or if floating point hardware is installed        */

#  ifdef BYTESEX_EVEN
typedef union {struct {int mhi:20, x:11, s:1; int mlo; } i;
               double d; } fp_number;
#  else
typedef union {struct {int s:1, x:11, mhi:20; int mlo; } i;
               double d; } fp_number;
#  endif

#else   /* OTHER_WORD_ORDER_FOR_FP_NUMBERS */
#  ifdef BYTESEX_EVEN
typedef union {struct {int mlo; int mhi:20, x:11, s:1; } i;
               double d; } fp_number;
#  else
typedef union {struct {int mlo; int s:1, x:11, mhi:20; } i;
               double d; } fp_number;
#  endif
#endif  /* OTHER_WORD_ORDER_FOR_FP_NUMBERS */

#endif

/* the object of the following macro is to adjust the floating point     */
/* variables concerned so that the more significant one can be squared   */
/* with NO LOSS OF PRECISION. It is only used when there is no danger    */
/* of over- or under-flow.                                               */

/* This code is NOT PORTABLE but can be modified for use elsewhere       */
/* It should, however, serve for IEEE and IBM FP formats.                */

#if defined __C40
#define _fp_normalize( high, low )                                      \
    {									\
      fp_number	temp;            /* access to representation      */    \
      double	temp1;           /* temporary value               */	\
									\
      temp.d    = high;          /* take original number          */	\
      temp.i.m &= 0xFFFF0000U;   /* make low part of mantissa 0   */	\
      temp1     = high - temp.d; /* the bit that was thrown away  */	\
      low      += temp1;         /* add into low-order result     */	\
      high      = temp.d;        /* and replace high-order result */	\
    }

#else /* everybody else */

#define _fp_normalize(high, low)                                          \
    {   fp_number temp;             /* access to representation     */    \
        double temp1;                                                     \
        temp.d     = high;          /* take original number         */    \
        temp.i.mlo = 0;             /* make low part of mantissa 0  */    \
        temp1      = high - temp.d; /* the bit that was thrown away */    \
        low       += temp1;         /* add into low-order result    */    \
        high       = temp.d;                                              \
    }
#endif /* C40FLT */

#endif


/* end of sysdep.h */
