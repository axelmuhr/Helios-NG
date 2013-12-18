/*
 * IEEE64.h - header file for the 64 bit IEEE floating point library
 *
 *   Copyright (c) 1992 Perihelion Software Ltd.
 *   All rights reserved.
 *
 * Author :	N Clifton
 * Version :	$Revision: 1.3 $
 * Date :	$Date: 1992/07/15 09:45:39 $
 * Id :		$Id: IEEE64.h,v 1.3 1992/07/15 09:45:39 nickc Exp $
 */

#if !defined _IEEE64_h && defined __C40
#define _IEEE64_h

/*
 * This header file describes the functions provided by the IEEE 64 bit
 * floating point library.  This library is callable from C.
 *
 * The library implements most, but not all of the IEEE standard.
 * The exceptions are as follows :-
 *
 * + Only double precision (64 bit) arithmetic is supported.
 *   Functions are provided to convert between signle precision
 *   (32 bit) and double precision IEEE numbers.
 *
 * + Negative zero is NOT supported.  In fact floating point zero
 *   is always represented as a zero value in both 32 bit words.
 *
 * + Denormalized numbers are NOT supported.
 *
 * + NaN is NOT supported.
 *
 * + Positive and negative infinities are NOT supported as a parameter
 *   to any of the functions.  The functions can return such numbers
 *   if their results over- or under- flow.
 *
 * + Only the "Round to Nearest" rounding mode is supported.
 *
 * + Exceptions and Traps are not supported.  Division by zero will
 *   (arbitarily) return zero.
 *
 * + The library does not implement the following functions:
 *
 *   - Remainder
 *   - Square Root
 *
 * + The compare function does not return an "unordered" result.
 *   As such it maps directly onto the C model of comparisons.
 */
 

 
/*
 * These structures represent the arguments that can be passed to
 * (and received from) the functions in the library
 */

typedef struct IEEE64
  {
       signed long int	high;	/* most  significant 32 bits of the 64 bit IEEE number */
     unsigned long int	low;	/* least significant 32 bits of the 64 bit IEEE number */
  }
IEEE64;

typedef struct IEEE32
  {
       signed long int	value;
  }
IEEE32;

/*
 * function prototypes
 */

#define FTSTD( a )	((a).high)

double		FSTOF(	IEEE32 a );		   /* converts from IEEE 32 bit   to TI   40 bit   format */
double		FDTOF(	IEEE64 a );		   /* converts from IEEE 64 bit   to TI   40 bit   format */
IEEE32		FFTOS(	double a );		   /* converts from TI   40 bit   to IEEE 32 bit   format */
IEEE64		FFTOD(	double a );		   /* converts from TI   40 bit   to IEEE 64 bit   format */
IEEE64		FSTOD(  IEEE32 val );		   /* converts from IEEE 64 bit   to IEEE 32 bit   format */
IEEE32		FDTOS(  IEEE64 val );		   /* converts from IEEE 32 bit   to IEEE 64 bit   format */
IEEE64		FUTOD(  unsigned long int val );   /* converts from unsigned long to IEEE 64 bit   format */
IEEE64		FITOD(    signed long int val );   /* converts from   signed long to IEEE 64 bit   format */
unsigned long	FDTOU(  IEEE64 a );		   /* converts from IEEE 64 bit   to unsigned long format */
signed long	FDTOI(  IEEE64 a );		   /* converts from IEEE 64 bit   to   signed long format */
signed int 	(FTSTD)(IEEE64 a );		   /* returns -1 iff a < 0, 0 iff a = 0, and 1  iff a > 0 */
signed int	FCMPD(  IEEE64 a, IEEE64 b );	   /* returns <0 iff a < b, 0 iff a = b, and >0 iff a > b */
IEEE64		FNEGD(  IEEE64 a );		   /* returns  -a      */
IEEE64		FSUBD(  IEEE64 a, IEEE64 b );	   /* computes "a - b" */
IEEE64		FADDD(  IEEE64 a, IEEE64 b );	   /* computes "a + b" */
IEEE64		FMULD(  IEEE64 a, IEEE64 b );	   /* computes "a * b" */
IEEE64		FDIVD(  IEEE64 a, IEEE64 b );	   /* computes "a / b" */

#ifdef UNACCESSIBLE_FROM_C

/* 
 * Note that functions that start with an underscore (eg _FADDD)
 * do not follow the normal Helios calling conventions.  Instead
 * they return their result in the first two argument registers
 * (R_A1 (high), R_A2 (low)).  This is more efficient, than the
 * normal Helios method (of allocating space for the result on the
 * stack and copying the result to this space).  They are provided
 * for assembly language programmers.
 */

IEEE64		_FUTOD( unsigned long int val );   /* converts from unsigned long */
IEEE64		_FITOD( signed long int val );	   /* converts from   signed long */
IEEE64		_FSUBD( IEEE64 a, IEEE64 b );	   /* computes "a - b" */
IEEE64		_FADDD( IEEE64 a, IEEE64 b );	   /* computes "a + b" */
IEEE64		_FMULD( IEEE64 a, IEEE64 b );	   /* computes "a * b" */
IEEE64		_FDIVD( IEEE64 a, IEEE64 b );	   /* computes "a / b" */
IEEE64		_FNEGD( IEEE64 a );		   /* computes "-a"    */
IEEE64		_FSTOD( IEEE32 a );		   /* converts to IEEE 32 bit format */

#endif /* UNACCESSIBLE_FROM_C */
 
#endif /* ! _IEEE64_h && __C40 */

/* do not put anything beyond this #endif */

/* @@ emacs customization */

/* Local Variables: */
/* mode: c */
/* End: */
