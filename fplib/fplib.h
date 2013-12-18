/*> fplib.h <*/
/*---------------------------------------------------------------------------*/
/* This file specifies details of the host system when compiling "fplib".    */
/* It is derived from the file "hostsys.h" from the Acorn library "CLIB".    */
/*---------------------------------------------------------------------------*/

#ifndef __fplib_h
#define __fplib_h

#undef MACHINE

#ifdef __ARM
/* Describe the ARM */
#define BYTESEX_EVEN 1
#define MACHINE "ARM"

/* FPE2 features STFP/LDFP ops */
#define HOST_HAS_BCD_FLT 1

#define HOST_HAS_TRIG 1			/* and IEEE trig functions */

/* IEEE floating point format assumed.                                   */
/* For the current ARM floating point system that Acorn use the first    */
/* word of a floating point value is the one containing the exponent.    */

typedef union {struct {int mhi:20, x:11, s:1; unsigned mlo; } i;
               double d; } fp_number;

#define DO_NOT_SUPPORT_UNNORMALIZED_NUMBERS 1

#elif defined(__C40)

/* @@@ This SHOULD be checked */

#define BYTESEX_EVEN 	1		/* ie Little endian */
#define MACHINE 	"C40"

#define C40FLT		1		/* ie not IEEE */


/*
 * XXX - NC - 6/2/92
 *
 * Trust me.
 *
 */

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

#define DO_NOT_SUPPORT_UNNORMALIZED_NUMBERS 1

#else /* xputer */
/* Describe the transputer */

#define BYTESEX_EVEN 	1		/* ?? doesn't actually check for this */
#define MACHINE 	"TRANSPUTER"

/* FPE2 features STFP/LDFP ops */

#define HOST_HAS_BCD_FLT 1		/* ?? doesn't actually check for this */

#define IEEE 		1		/* THIS IS ASSUMED */
/* IEEE floating point format assumed.                                   */
/* For the transputer the second word is the one containing the exponent */

typedef union {struct { unsigned mlo; int mhi:20, x:11, s:1; } i;
               double d; } fp_number;


#define DO_NOT_SUPPORT_UNNORMALIZED_NUMBERS 1
#endif

/* The following code is NOT PORTABLE but can stand as a prototype for   */
/* whatever makes sense on other machines.                               */

/* This version works with the ARM floating point emulator - it may have */
/* to be reworked when or if floating point hardware is installed        */

/* the object of the following macro is to adjust the floating point     */
/* variables concerned so that the more significant one can be squared   */
/* with NO LOSS OF PRECISION. It is only used when there is no danger    */
/* of over- or under-flow.                                               */

/* This code is NOT PORTABLE but can be modified for use elsewhere       */
/* It should, however, serve for IEEE and IBM FP formats.                */

#if defined __C40

/*
 * XXX - NC - 6/2/92
 *
 * Oh ye ghods, what am I going to do ?
 *
 * I think that the code relies upon the fact that squaring a number
 * can never more than double the precision required.
 */

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
    {   fp_number temp;        /* access to representation     */         \
        double temp1;                                                     \
        temp.d = high;         /* take original number         */         \
        temp.i.mlo = 0;        /* make low part of mantissa 0  */         \
        temp1 = high - temp.d; /* the bit that was thrown away */         \
        low += temp1;          /* add into low-order result    */         \
        high = temp.d;                                                    \
    }

#endif /* __C40 */

#define memclr( s, n )	memset( s, 0, n )

#endif /* __fplib_h */

/*---------------------------------------------------------------------------*/
/*> EOF fplib.h <*/
