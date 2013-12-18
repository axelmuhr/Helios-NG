/*------------------------------------------------------------------------
--                                                                      --
--               H E L I O S   V E C T O R   L I B R A R Y              --
--               -----------------------------------------              --
--                                                                      --
--             Copyright (C) 1992, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- vectest.c								--
--		Test program for the vector library.			--
--                                                                      --
--	Author:  BLV 14/10/92						--
--                                                                      --
------------------------------------------------------------------------*/
/* SccsId: %W%	%G% Copyright (C) 1987, Perihelion Software Ltd.	*/
/* $Id: vectest.c,v 1.6 1992/10/20 09:27:40 nickc Exp $ */

/*{{{  description */
/*
** This test program compares the behaviour of the assembler versions
** of the vector library with the equivalent C code. It checks that
** the results are identical and compares the time taken.
**
** There are two ways of running the program. By default the program
** just cycles through all the tests one by one. Alternatively it will
** display a menu allowing the tests to be performed one at a time.
**
** The various measurements are done with on-chip memory for the stack
** but not for the code. The latter would make a signicant difference
** with some processors, particularly transputers, but it would be
** complicated to do a fair test of in-line C code and the vector library
** if both used on-chip memory for code.
*/
/*}}}*/
/*{{{  header files and compile time options */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <posix.h>
#include <sys/wait.h>
#include <helios.h>
#include <queue.h>
#include <syslib.h>
#include <attrib.h>
#include <codes.h>
#include <memory.h>
#include <task.h>
#include <nonansi.h>
#include <vectlib.h>

#ifdef __TRAN
#include <asm.h>
#endif

#ifdef __C40
#pragma fast_FP
#endif

/*
** N.B. this stack is used for floating point printf() calls, please
** make sure there is enough. The default settings should have neither
** of the code modules in on-chip RAM, only the stack, and all the on-chip
** RAM should be used for the stack. This allows for a fair comparison
** between the vector library and the C code.
*/
#define FloatToOnChip	0
#define DoubleToOnChip	0
#define StackToOnChip	1
#define StackSize	3500

	/* This constant is used to allow for inaccuracies in the	*/
	/* floating point. It indicates the maximum allowed divergence	*/
	/* between vectlib routines and the C code. The value used has	*/
	/* been chosen by guesswork.					*/
#define EPSILON 0.0000001
/*}}}*/
/*{{{  version number and history */
#define VersionNumber	"1.00"
/*
** First version: BLV, 16.10.92
*/
/*}}}*/
/*{{{  statics including several big vectors, and #define's */
/*
** The test code keeps track of the following vectors:
**  Xf_Master, Yf_Master - These vectors are initialised with random numbers.
**   			   The working vectors are copied from these.
**  Xf_Asm, Yf_Asm	 - These vectors are passed to the assembler routines.
**  Xf_C, Yf_C		 - These vectors are processed by the C code.
** 
**  xd_Master ...	 - The same vectors but containing double precision
**			   numbers.
**
** Note that all of the vectors are actually processed by the routines.
** There are guards at the start and end of the vectors to detect overflow.
** The stride routines are always passed a stride of 2 so only half the
** numbers are processed.
**
** These vectors occupy 72K of data, which should not be a problem.
** However the matrix tests require additional static data, approximately
** another megabyte. Also the maxima and minima tests use a number of small
** extra vectors to check for special cases, e.g. first element or last
** element. Hence the test program will only work in processors
** with at least four megabytes.
*/
#define VectorStride	2
#define VectorSize	1024
#define VectorOffset	4	/* four guard entries on either side	*/

#define VectorProcessed (VectorSize - 2 * VectorOffset)
#define StrideProcessed (VectorProcessed / VectorStride)
		
		/* The strides are held in non-static variables so that	*/
		/* the compiler cannot optimise the stride calculations	*/
		/* for the in-line versions.				*/
int VectorXStride = VectorStride;
int VectorYStride = VectorStride;
#define STRIDES \
	int XS = VectorXStride;\
	int YS=VectorYStride

#define STRIDESX \
	int XS = VectorXStride

static float Xf_Master[ VectorSize ], Yf_Master[ VectorSize ];
static float Xf_Asm[    VectorSize ], Yf_Asm[    VectorSize ];
static float Xf_C[      VectorSize ], Yf_C[      VectorSize ];

static double Xd_Master[VectorSize], Yd_Master[VectorSize];
static double Xd_Asm[VectorSize], Yd_Asm[VectorSize];
static double Xd_C[VectorSize], Yd_C[VectorSize];

#define FLOATS \
	float *XA = &(Xf_Asm[VectorOffset]);\
	float *YA = &(Yf_Asm[VectorOffset]);\
	float *XC = &(Xf_C[VectorOffset]);\
	float *YC = &(Yf_C[VectorOffset])

#define FLOATSX \
	float *XA = &(Xf_Asm[VectorOffset]);\
	float *XC = &(Xf_C[VectorOffset])

#define DOUBLES \
	double *XA = &(Xd_Asm[VectorOffset]);\
	double *YA = &(Yd_Asm[VectorOffset]);\
	double *XC = &(Xd_C[VectorOffset]);\
	double *YC = &(Yd_C[VectorOffset])

#define DOUBLESX \
	double *XA = &(Xd_Asm[VectorOffset]);\
	double *XC = &(Xd_C[VectorOffset])

/*
** To detect corruption of the master vectors two checksums are calculated
** during vector initialisation and checked after every test function.
*/
static float	F_CheckSum;
static double 	D_CheckSum;

/*
** The program keeps track of the name of the current test to simply
** error reporting.
*/
static char *TestName = "";

/*
** The argument passed to float_checks() or double_checks() consists of a 
** number of flags, minimising the code in the actual test routines.
*/
#define Test_StridesUsed	0x01
#define Test_ReadOnly		0x02	/* First vector is not modified	*/

/*
** Screen sizes.
*/
static int	ScreenHeight;
static int	ScreenWidth;

/*
** Running in compact mode reduces the amount of output generated.
*/
static bool	CompactOutput	= FALSE;

/*
** All output should go to this file.
*/
static FILE	*Output		= stdout;
/*}}}*/
/*{{{  timing  */
/*
** The time taken for a vector operation of a 1024 elements is fairly
** small, so the most accurate timer available should be used. On the
** transputer this means ldtimer. There is no need to
** worry about the units since the timing code only takes ratios.
*/
#ifdef __TRAN
#define Timer()		ldtimer_()
#endif

#ifdef __C40
extern long		_ldtimer( long );

#define Timer()		_ldtimer(0)
#endif

/*
** Default to _cputime() if nothing better is available.
*/
#ifndef Timer
#define Timer()		_cputime()
#endif

/*
** These macros are used to perform standard timings.
*/
#define TIMERS	long t0, t1, t2	/* Declare three timers	*/
#define T0	t0 = Timer()
#define T1	t1 = Timer()
#define T2	t2 = Timer(); \
  show_times(t0, t1, t2)

static void show_times(long t0, long t1, long t2)
{
  double t1_minus_t0 = (double)(long)(t1 - t0);
  double t2_minus_t1 = (double)(long)(t2 - t1);


  if ((t1 == t0) || (t2 == t1))
   fprintf(Output, "Timer resolution too low.\n");
  elif (CompactOutput)
   fprintf(Output, "Test %s : %.1f%%\n", TestName,
	100.0 - (100.0 * t1_minus_t0) / t2_minus_t1);
  else
   fprintf(Output, "Veclib time is %3.1f%% of the C equivalent, saving %.1f%%, speedup %.1f times\n",
	 	(100.0 * t1_minus_t0) / t2_minus_t1,
	 	100.0 - (100.0 * t1_minus_t0) / t2_minus_t1,
	 	t2_minus_t1 / t1_minus_t0);
}
/*}}}*/
/*{{{  fill vectors with random data */
/*
** The test data I use has a fair distribution of numbers
** within the range -1000.0 -> 1000.0, all numbers having much the same
** magnitude. This seems like a reasonable test condition. Obviously
** other test data such as positive values only, very large numbers only,
** numbers between -1 and 1 only, etc. might be worth checking as well.
** However the floating point unit should cope with all of those conditions.
** The behaviour of the library is undefined when it comes to overflow,
** underflow, etc.
*/
#define RandMax	0x7FFFFFFF
static unsigned int	RandSeed;	/* Initialised using time()	*/

	/* return uint: 0 <= x <= 7FFFFFFF	*/
static unsigned int	Rand(void)
{ RandSeed = (RandSeed * 1103515245) + 12345;
  return((RandSeed >> 1) & 0x7FFFFFFF);
}

	/* return float: -1000.0 <= x < 1000.0	*/
static float FRand(void)
{ float result;
  result  = (float) Rand() / (float) RandMax;	/* 0 <= x < 1		*/
  result *= 2000.0F;				/* 0 <= x < 2000	*/
  result -= 1000.0F;				/* -1000 <= x < 1000	*/
  return(result);
}

	/* ditto for double			*/
static double DRand(void)
{ double result;
  result  = (double) Rand() / (double) RandMax;
  result *= 2000.0;
  result -= 1000.0;
  return(result);
}

	/* Fill in the master vectors.		*/
static void init_vectors(void)
{ int		i;
  float		f_checksum;
  double	d_checksum;

  RandSeed = (unsigned int) time(NULL);

  for (i = 0; i < VectorSize; i++)
   { Xf_Master[i]	= FRand();
     Yf_Master[i]	= FRand();
     Xd_Master[i]	= DRand();
     Yd_Master[i]	= DRand();
   }

	/* Calculate initial checksums.	*/
  f_checksum = 0.0F;	/* The C40 compiler has problems if the calculation	*/
  d_checksum = 0.0;	/* is done using the statics directly.			*/
  for (i = 0; i < VectorSize; i++)
   { f_checksum += Xf_Master[i];
     d_checksum += Xd_Master[i];
     f_checksum += Yf_Master[i];
     d_checksum += Yd_Master[i];
   }
  F_CheckSum = f_checksum;
  D_CheckSum = d_checksum;
}
/*}}}*/
/*{{{  screen manipulation */
/**
*** This contains the usual routines to initialise the screen and move
*** the cursor.
**/
static void	initialise_screen(void)
{ Attributes	attr;

  unless (isatty(0) && isatty(1))
   { fputs("vectest: not running interactively.\n", stderr);
     exit(EXIT_FAILURE);
   }

  if (GetAttributes(fdstream(0), &attr) < Err_Null)
   { fputs("vectest: failed to get keyboard details.\n", stderr);
     exit(EXIT_FAILURE);
   }

  AddAttribute(&attr, ConsoleRawInput);
  RemoveAttribute(&attr, ConsoleEcho);
  RemoveAttribute(&attr, ConsoleRawOutput);
  if (SetAttributes(fdstream(0), &attr) < Err_Null)
   { fputs("vectest: failed to initialise keyboard.\n", stderr);
     exit(EXIT_FAILURE);
   }
  ScreenHeight	= attr.Min;
  ScreenWidth	= attr.Time;

  setvbuf(stdin, NULL, _IONBF, 0);
}

static void move_to(int y, int x)
{ printf("\033[%d;%dH", y, x);
}

static void clear_screen()
{ putchar('\f');
}

static void waitfor_user()
{ int	x;

  move_to(ScreenHeight, 1);
  fputs("\033[KPress any key to continue.", stdout);
  fflush(stdout);
  x = getchar();
  move_to(ScreenHeight, 1);
  fputs("\033[K", stdout);
  fflush(stdout);
}
/*}}}*/
/*{{{  testing for overflows etc. */
/*
** This code tests the behaviour of the various vector library routines.
** There are two versions, one for single-precision and one for
** double-precision. Tests performed are as follows:
**  1) the guards at the start and end of the X vector are checked.
**  2) the Y vectors should be unchanged relative to the master.
**  3) the X vectors for the assembler and C version should be identical.
**  4) if the routine has not changed the X vector then this verified.
**  5) if the routine involves a stride then it checks that entries between
**     strides are unchanged.
**
** In addition the do_test() routine performs checksumming on the master
** copies of the vector.
*/
static void float_checks(int flags)
{ int	i;
  bool  small_errors = FALSE;

	/* Check the guard values at the two ends of the X vector.	*/
  for (i = 0; i < VectorOffset; i++)
   { if (Xf_Asm[i] != Xf_Master[i])
      fprintf(Output, "Asm: vector X, guard at %d corrupted.\n", i);
     if (Xf_C[i] != Xf_Master[i])
      fprintf(Output, "C: vector X, guard at %d corrupted.\n", i);
     if (Xf_Asm[VectorSize - (i+1)] != Xf_Master[VectorSize - (i+1)])
      fprintf(Output, "Asm: vector X, guard at %d corrupted.\n", VectorSize - (i+1));
     if (Xf_C[VectorSize - (i+1)] != Xf_Master[VectorSize - (i+1)])
      fprintf(Output, "C: vector X, guard at %d corrupted.\n", VectorSize - (i+1));
   }

  if (memcmp(Yf_Asm, Yf_Master, sizeof(Yf_Master)))
   fprintf(Output, "Asm: vector Y has been corrupted.\n");
  if (memcmp(Yf_C, Yf_Master, sizeof(Yf_Master)))
   fprintf(Output, "C: vector Y has been corrupted.\n");

  if (memcmp(Xf_Asm, Xf_C, sizeof(Xf_C)))
   for (i = 0; i < VectorSize; i++)
    { if (Xf_Asm[i] != Xf_C[i])
       { if (fabs((double)(float)(Xf_Asm[i] - Xf_C[i])) > fabs(EPSILON * Xf_Asm[i]))
          fprintf(Output, "Difference: asm X[%d] is %f, C X[%d] is %f\n", i, Xf_Asm[i], i, Xf_C[i]);
	 elif (!small_errors)
	  { small_errors = TRUE;
	    fprintf(Output, "Warning: small discrepancies have been detected.\n");
	  }
       }
    }
    

  if (flags & Test_ReadOnly)
   { if (memcmp(Xf_Asm, Xf_Master, sizeof(Xf_Master)))
      for (i = 0; i < VectorSize; i++)
       if (Xf_Asm[i] != Xf_Master[i])
        fprintf(Output, "Corruption: asm X[%d] is %f, master X[%d] is %f\n", i, Xf_Asm[i], i, Xf_Master[i]);
   }
	/* Avoid repeating the error messages.	*/
  elif (flags & Test_StridesUsed)
   for (i = 1; i < VectorSize; i += 2)
    if (Xf_Asm[i] != Xf_Master[i])
     fprintf(Output, "Stride problem: asm X[%d] is %f, master X[%d] is %f\n", i, Xf_Asm[i], i, Xf_Master[i]);
}

static void double_checks(int flags)
{ int	i;
  bool	small_errors = FALSE;

	/* Check the guard values at the two ends of the X vector.	*/
  for (i = 0; i < VectorOffset; i++)
   { if (Xd_Asm[i] != Xd_Master[i])
      fprintf(Output, "Asm: vector X, guard at %d corrupted.\n", i);
     if (Xd_C[i] != Xd_Master[i])
      fprintf(Output, "C: vector X, guard at %d corrupted.\n", i);
     if (Xd_Asm[VectorSize - (i+1)] != Xd_Master[VectorSize - (i+1)])
      fprintf(Output, "Asm: vector X, guard at %d corrupted.\n", VectorSize - (i+1));
     if (Xd_C[VectorSize - (i+1)] != Xd_Master[VectorSize - (i+1)])
      fprintf(Output, "C: vector X, guard at %d corrupted.\n", VectorSize - (i+1));
   }

  if (memcmp(Yd_Asm, Yd_Master, sizeof(Yd_Master)))
   fprintf(Output, "Asm: vector Y has been corrupted.\n");
  if (memcmp(Yd_C, Yd_Master, sizeof(Yd_Master)))
   fprintf(Output, "C: vector Y has been corrupted.\n");

  if (memcmp(Xd_Asm, Xd_C, sizeof(Xd_C)))
   for (i = 0; i < VectorSize; i++)
    { if (Xf_Asm[i] != Xf_C[i])
       { if (fabs((double)(float)(Xf_Asm[i] - Xf_C[i])) > fabs(EPSILON * Xf_Asm[i]))
          fprintf(Output, "Difference: asm X[%d] is %f, C X[%d] is %f\n", i, Xf_Asm[i], i, Xf_C[i]);
	 elif (!small_errors)
	  { small_errors = TRUE;
	    fprintf(Output, "Warning: small discrepencies have been detected.\n");
	  }
       }
    }

  if (flags & Test_ReadOnly)
   { if (memcmp(Xd_Asm, Xd_Master, sizeof(Xd_Master)))
      for (i = 0; i < VectorSize; i++)
       if (Xd_Asm[i] != Xd_Master[i])
        fprintf(Output, "Corruption: asm X[%d] is %f, master X[%d] is %f\n", i, Xd_Asm[i], i, Xd_Master[i]);
   }
	/* Avoid repeating the error messages.	*/
  elif (flags & Test_StridesUsed)
   for (i = 1; i < VectorSize; i += 2)
    if (Xd_Asm[i] != Xd_Master[i])
     fprintf(Output, "Stride problem: asm X[%d] is %f, master X[%d] is %f\n", i, Xd_Asm[i], i, Xd_Master[i]);
}

/*}}}*/
/*{{{  vector-vector tests */
/*{{{  VfAdd_test() */
static void VfAdd_test(void)
{ TIMERS;
  FLOATS;
  int	n = VectorProcessed;

  T0;
  VfAdd(n, XA, YA);
  T1;
  while (n--)
   *XC++ += *YC++;
  T2;
  float_checks(0);
}
/*}}}*/
/*{{{  VfSub_test() */
static void VfSub_test(void)
{ TIMERS;
  FLOATS;
  int	n = VectorProcessed;

  T0;
  VfSub(n, XA, YA);
  T1;
  while (n--)
   *XC++ -= *YC++;
  T2;
  float_checks(0);
}
/*}}}*/
/*{{{  VfMul_test() */
static void VfMul_test(void)
{ TIMERS;
  FLOATS;
  int n = VectorProcessed;

  T0;
  VfMul(n, XA, YA);
  T1;
  while (n--)
   *XC++ *= *YC++;
  T2;
  float_checks(0);
}
/*}}}*/
/*{{{  VfDiv_test() */
static void VfDiv_test(void)
{ TIMERS;
  FLOATS;
  int n = VectorProcessed;

  T0;
  VfDiv(n, XA, YA);
  T1;
  while (n--)
   *XC++ /= *YC++;
  T2;
  float_checks(0);
}
/*}}}*/
/*{{{  VfsAdd_test() */
static void VfsAdd_test(void)
{ TIMERS;
  FLOATS;
  STRIDES;
  int n = StrideProcessed;

  T0;
  VfsAdd(n, XA, XS, YA, YS);
  T1;
  while (n--)
   { *XC += *YC; XC += XS; YC += YS; }
  T2;
  float_checks(Test_StridesUsed);
}
/*}}}*/
/*{{{  VfsSub_test() */
static void VfsSub_test(void)
{ TIMERS;
  FLOATS;
  STRIDES;
  int n = StrideProcessed;

  T0;
  VfsSub(n, XA, XS, YA, YS);
  T1;
  while (n--)
   { *XC -= *YC; XC += XS; YC += YS; }
  T2;
  float_checks(Test_StridesUsed);
}
/*}}}*/
/*{{{  VfsMul_test() */
static void VfsMul_test(void)
{ TIMERS;
  FLOATS;
  STRIDES;
  int n = StrideProcessed;

  T0;
  VfsMul(n, XA, XS, YA, YS);
  T1;
  while (n--)
   { *XC *= *YC; XC += XS; YC += YS; }
  T2;
  float_checks(Test_StridesUsed);
}
/*}}}*/
/*{{{  VfsDiv_test() */
static void VfsDiv_test(void)
{ TIMERS;
  FLOATS;
  STRIDES;
  int	n = StrideProcessed;
  
  T0;
  VfsDiv(n, XA, XS, YA, YS);
  T1;
  while (n--)
   { *XC /= *YC; XC += XS; YC += YS; }
  T2;
  float_checks(Test_StridesUsed);
}
/*}}}*/
/*{{{  VdAdd_test() */
static void VdAdd_test(void)
{ TIMERS;
  DOUBLES;
  int	n = VectorProcessed;

  T0;
  VdAdd(n, XA, YA);
  T1;
  while (n--)
   *XC++ += *YC++;
  T2;
  double_checks(0);
}
/*}}}*/
/*{{{  VdSub_test() */
static void VdSub_test(void)
{ TIMERS;
  DOUBLES;
  int	n = VectorProcessed;

  T0;
  VdSub(n, XA, YA);
  T1;
  while (n--)
   *XC++ -= *YC++;
  T2;
  double_checks(0);
}
/*}}}*/
/*{{{  VdMul_test() */
static void VdMul_test(void)
{ TIMERS;
  DOUBLES;
  int	n = VectorProcessed;

  T0;
  VdMul(n, XA, YA);
  T1;
  while (n--)
   *XC++ *= *YC++;
  T2;
  double_checks(0);
}
/*}}}*/
/*{{{  VdDiv_test() */
static void VdDiv_test(void)
{ TIMERS;
  DOUBLES;
  int	n = VectorProcessed;

  T0;
  VdDiv(n, XA, YA);
  T1;
  while (n--)
   *XC++ /= *YC++;
  T2;
  double_checks(0);
}
/*}}}*/
/*{{{  VdsAdd_test() */
static void VdsAdd_test(void)
{ TIMERS;
  DOUBLES;
  STRIDES;
  int	n = StrideProcessed;

  T0;
  VdsAdd(n, XA, XS, YA, YS);
  T1;
  while (n--)
   { *XC += *YC; XC += XS; YC += YS; }
  T2;
  double_checks(Test_StridesUsed);
}
/*}}}*/
/*{{{  VdsSub_test() */
static void VdsSub_test(void)
{ TIMERS;
  DOUBLES;
  STRIDES;
  int	n = StrideProcessed;

  T0;
  VdsSub(n, XA, XS, YA, YS);
  T1;
  while (n--)
   { *XC -= *YC; XC += XS; YC += YS; }
  T2;
  double_checks(Test_StridesUsed);
}
/*}}}*/
/*{{{  VdsMul_test() */
static void VdsMul_test(void)
{ TIMERS;
  DOUBLES;
  STRIDES;
  int	n = StrideProcessed;

  T0;
  VdsMul(n, XA, XS, YA, YS);
  T1;
  while (n--)
   { *XC *= *YC; XC += XS; YC += YS; }
  T2;
  double_checks(Test_StridesUsed);
}
/*}}}*/
/*{{{  VdsDiv_test() */
static void VdsDiv_test(void)
{ TIMERS;
  DOUBLES;
  STRIDES;
  int	n = StrideProcessed;

  T0;
  VdsDiv(n, XA, XS, YA, YS);
  T1;
  while (n--)
   { *XC /= *YC; XC += XS; YC += YS; }
  T2;
  double_checks(Test_StridesUsed);
}
/*}}}*/
/*}}}*/
/*{{{  vector-scalar tests */
/*{{{  VfAddScalar_test() */
static void VfAddScalar_test(void)
{ TIMERS;
  FLOATSX;
  float	value	= 10.0F;
  int	n	= VectorProcessed;

  T0;
  VfAddScalar(value, n, XA);
  T1;
  while (n--)
   *XC++ += value;
  T2;
  float_checks(0);
}
/*}}}*/
/*{{{  VfSubScalar_test() */
static void VfSubScalar_test(void)
{ TIMERS;
  FLOATSX;
  float	value	= 3.141592F;
  int	n	= VectorProcessed;

  T0;
  VfSubScalar(value, n, XA);
  T1;
  while (n--)
   *XC++ -= value;
  T2;
  float_checks(0);
}
/*}}}*/
/*{{{  VfMulScalar_test() */
static void VfMulScalar_test(void)
{ TIMERS;
  FLOATSX;
  float	value	= 2.0F;
  int	n	= VectorProcessed;

  T0;
  VfMulScalar(value, n, XA);
  T1;
  while (n--)
   *XC++ *= value;
  T2;
  float_checks(0);
}
/*}}}*/
/*{{{  VfDivScalar_test() */
static void VfDivScalar_test(void)
{ TIMERS;
  FLOATSX;
  float	value	= 4.0F;
  int	n	= VectorProcessed;

  T0;
  VfDivScalar(value, n, XA);
  T1;
  while (n--)
   *XC++ /= value;
  T2;
  float_checks(0);
}
/*}}}*/
/*{{{  VfRecScalar_test() */
static void VfRecScalar_test(void)
{ TIMERS;
  FLOATSX;
  float	value	= 1.0F;
  int	n	= VectorProcessed;

  T0;
  VfRecScalar(value, n, XA);
  T1;
  while (n--)
   { *XC = value / *XC; XC++; }
  T2;
  float_checks(0);
}
/*}}}*/
/*{{{  VfsAddScalar_test() */
static void VfsAddScalar_test(void)
{ TIMERS;
  FLOATSX;
  STRIDESX;
  float	value	= 3.0F;
  int	n	= StrideProcessed;

  T0;
  VfsAddScalar(value, n, XA, XS);
  T1;
  while (n--)
   { *XC += value; XC += XS; }
  T2;
  float_checks(Test_StridesUsed);
}
/*}}}*/
/*{{{  VfsSubScalar_test() */
static void VfsSubScalar_test(void)
{ TIMERS;
  FLOATSX;
  STRIDESX;
  float	value	= 3.0F;
  int	n	= StrideProcessed;

  T0;
  VfsSubScalar(value, n, XA, XS);
  T1;
  while (n--)
   { *XC -= value; XC += XS; }
  T2;
  float_checks(Test_StridesUsed);
}
/*}}}*/
/*{{{  VfsMulScalar_test() */
static void VfsMulScalar_test(void)
{ TIMERS;
  FLOATSX;
  STRIDESX;
  float	value	= 3.0F;
  int	n	= StrideProcessed;

  T0;
  VfsMulScalar(value, n, XA, XS);
  T1;
  while (n--)
   { *XC *= value; XC += XS; }
  T2;
  float_checks(Test_StridesUsed);
}
/*}}}*/
/*{{{  VfsDivScalar_test() */
static void VfsDivScalar_test(void)
{ TIMERS;
  FLOATSX;
  STRIDESX;
  float	value	= 2.25F;
  int	n	= StrideProcessed;

  T0;
  VfsDivScalar(value, n, XA, XS);
  T1;
  while (n--)
   { *XC /= value; XC += XS; }
  T2;
  float_checks(Test_StridesUsed);
}
/*}}}*/
/*{{{  VfsRecScalar_test() */
static void VfsRecScalar_test(void)
{ TIMERS;
  FLOATSX;
  STRIDESX;
  float	value	= 4.0F;
  int	n	= StrideProcessed;

  T0;
  VfsRecScalar(value, n, XA, XS);
  T1;
  while (n--)
   { *XC = value / *XC; XC += XS; }
  T2;
  float_checks(Test_StridesUsed);
}
/*}}}*/
/*{{{  VdAddScalar_test() */
static void VdAddScalar_test(void)
{ TIMERS;
  DOUBLESX;
  double value	= -1234.5678;
  int	 n 	= VectorProcessed;

  T0;
  VdAddScalar(value, n, XA);
  T1;
  while (n--)
   *XC++ += value;
  T2;
  double_checks(0);
}
/*}}}*/
/*{{{  VdSubScalar_test() */
static void VdSubScalar_test(void)
{ TIMERS;
  DOUBLESX;
  double value	= -2.7182818;
  int	 n	= VectorProcessed;

  T0;
  VdSubScalar(value, n, XA);
  T1;
  while (n--)
   *XC++ -= value;
  T2;
  double_checks(0);
}
/*}}}*/
/*{{{  VdMulScalar_test() */
static void VdMulScalar_test(void)
{ TIMERS;
  DOUBLESX;
  double value	= -3.141592;
  int	 n	= VectorProcessed;

  T0;
  VdMulScalar(value, n, XA);
  T1;
  while (n--)
   *XC++ *= value;
  T2;
  double_checks(0);
}
/*}}}*/
/*{{{  VdDivScalar_test() */
static void VdDivScalar_test(void)
{ TIMERS;
  DOUBLESX;
  double value	= 0.00001;
  int	 n	= VectorProcessed;

  T0;
  VdDivScalar(value, n, XA);
  T1;
  while (n--)
   *XC++ /= value;
  T2;
  double_checks(0);
}
/*}}}*/
/*{{{  VdRecScalar_test() */
static void VdRecScalar_test(void)
{ TIMERS;
  DOUBLESX;
  double value	= -1.0;
  int	 n	= VectorProcessed;

  T0;
  VdRecScalar(value, n, XA);
  T1;
  while (n--)
   { *XC = value / *XC; XC++; }
  T2;
  double_checks(0);
}
/*}}}*/
/*{{{  VdsAddScalar_test() */
static void VdsAddScalar_test(void)
{ TIMERS;
  DOUBLESX;
  STRIDESX;
  double value	= 87654.0;
  int	n	= StrideProcessed;

  T0;
  VdsAddScalar(value, n, XA, XS);
  T1;
  while (n--)
   { *XC += value; XC += XS; }
  T2;
  double_checks(Test_StridesUsed);
}
/*}}}*/
/*{{{  VdsSubScalar_test() */
static void VdsSubScalar_test(void)
{ TIMERS;
  DOUBLESX;
  STRIDESX;
  double value	= 0.5;
  int	 n	= StrideProcessed;

  T0;
  VdsSubScalar(value, n, XA, XS);
  T1;
  while (n--)
   { *XC -= value; XC += XS; }
  T2;
  double_checks(Test_StridesUsed);
}
/*}}}*/
/*{{{  VdsMulScalar_test() */
static void VdsMulScalar_test(void)
{ TIMERS;
  DOUBLESX;
  STRIDESX;
  double value	= 0.0001;
  int	 n	= StrideProcessed;

  T0;
  VdsMulScalar(value, n, XA, XS);
  T1;
  while (n--)
   { *XC *= value; XC += XS; }
  T2;
  double_checks(Test_StridesUsed);
}
/*}}}*/
/*{{{  VdsDivScalar_test() */
static void VdsDivScalar_test(void)
{ TIMERS;
  DOUBLESX;
  STRIDESX;
  double value	= 10000.0;
  int	 n	= StrideProcessed;

  T0;
  VdsDivScalar(value, n, XA, XS);
  T1;
  while (n--)
   { *XC /= value; XC += XS; }
  T2;
  double_checks(Test_StridesUsed);
}
/*}}}*/
/*{{{  VdsRecScalar_test() */
static void VdsRecScalar_test(void)
{ TIMERS;
  DOUBLESX;
  STRIDESX;
  double value	= 100.0;
  int	 n	= StrideProcessed;

  T0;
  VdsRecScalar(value, n, XA, XS);
  T1;
  while (n--)
   { *XC = value / *XC; XC += XS; }
  T2;
  double_checks(Test_StridesUsed);
}
/*}}}*/
/*}}}*/
/*{{{  vector-scalar multiply tests */
/*{{{  VfMulAdd_test() */
static void VfMulAdd_test(void)
{ TIMERS;
  FLOATS;
  float 	value	= -4.0F;
  int		n	= VectorProcessed;

  T0;
  VfMulAdd(value, n, XA, YA);
  T1;
  while (n--)
   *XC++ += (value * *YC++);
  T2;

  float_checks(0);
}
/*}}}*/
/*{{{  VfsMulAdd_test() */
static void VfsMulAdd_test(void)
{ TIMERS;
  FLOATS;
  STRIDES;
  float 	value	= 32.0F;
  int		n	= StrideProcessed;

  T0;
  VfsMulAdd(value, n, XA, XS, YA, YS);
  T1;
  while (n--)
   { *XC += (value * *YC); XC += XS; YC += YS; }
  T2;

  float_checks(Test_StridesUsed);
}
/*}}}*/
/*{{{  VdMulAdd_test() */
static void VdMulAdd_test(void)
{ TIMERS;
  DOUBLES;
  double 	value	= 3.141592;
  int		n	= VectorProcessed;

  T0;
  VdMulAdd(value, n, XA, YA);
  T1;
  while (n--)
   *XC++ += (value * *YC++);
  T2;

  double_checks(0);
}
/*}}}*/
/*{{{  VdsMulAdd_test() */
static void VdsMulAdd_test(void)
{ TIMERS;
  DOUBLES;
  STRIDES;
  double 	value	= -2.0;
  int		n	= StrideProcessed;

  T0;
  VdsMulAdd(value, n, XA, XS, YA, YS);
  T1;
  while (n--)
   { *XC += (value * *YC); XC += XS; YC += YS; }
  T2;

  double_checks(Test_StridesUsed);
}
/*}}}*/
/*}}}*/
/*{{{  vector initialisation tests */
/*{{{  VfFill() */
static void VfFill_test(void)
{ TIMERS;
  FLOATSX;
  float	value	= 0.0F;
  int	n	= VectorProcessed;

  T0;
  VfFill(value, n, XA);
  T1;
  while (n--)
   *XC++ = value;
  T2;

  float_checks(0);
}
/*}}}*/
/*{{{  VfsFill() */
static void VfsFill_test(void)
{ TIMERS;
  FLOATSX;
  STRIDESX;
  float	value	= 1.0F;
  int	n	= StrideProcessed;

  T0;
  VfsFill(value, n, XA, XS);
  T1;
  while (n--)
   { *XC = value; XC += XS; }
  T2;

  float_checks(Test_StridesUsed);
}
/*}}}*/
/*{{{  VdFill() */
static void VdFill_test(void)
{ TIMERS;
  DOUBLESX;
  double	value	= 99.0;
  int		n	= VectorProcessed;

  T0;
  VdFill(value, n, XA);
  T1;
  while (n--)
   *XC++ = value;
  T2;

  double_checks(0);
}
/*}}}*/
/*{{{  VdsFill() */
static void VdsFill_test(void)
{ TIMERS;
  DOUBLESX;
  STRIDESX;
  double	value	= -1.0;
  int		n	= StrideProcessed;

  T0;
  VdsFill(value, n, XA, XS);
  T1;
  while (n--)
   { *XC = value; XC += XS; }
  T2;

  double_checks(Test_StridesUsed);
}
/*}}}*/
/*}}}*/
/*{{{  vector copy tests */
/*{{{  VfCopy_test() */
static void VfCopy_test(void)
{ TIMERS;
  FLOATS;
  int		n = VectorProcessed;

  T0;
  VfCopy(n, XA, YA);
  T1;
  while (n--)
   *XC++ = *YC++;
  T2;

  float_checks(0);
}
/*}}}*/
/*{{{  VfsCopy_test() */
static void VfsCopy_test(void)
{ TIMERS;
  FLOATS;
  STRIDES;
  int		n = StrideProcessed;

  T0;
  VfsCopy(n, XA, XS, YA, YS);
  T1;
  while (n--)
   { *XC = *YC; XC += XS; YC += YS; }
  T2;

  float_checks(Test_StridesUsed);
}
/*}}}*/
/*{{{  VdCopy_test() */
static void VdCopy_test(void)
{ TIMERS;
  DOUBLES;
  int		n = VectorProcessed;

  T0;
  VdCopy(n, XA, YA);
  T1;
  while (n--)
   *XC++ = *YC++;
  T2;

  double_checks(0);
}
/*}}}*/
/*{{{  VdsCopy_test() */
static void VdsCopy_test(void)
{ TIMERS;
  DOUBLES;
  STRIDES;
  int		n = StrideProcessed;

  T0;
  VdsCopy(n, XA, XS, YA, YS);
  T1;
  while (n--)
   { *XC = *YC; XC += XS; YC += YS; }
  T2;

  double_checks(Test_StridesUsed);
}
/*}}}*/
/*}}}*/
/*{{{  vector dot product tests */
/*{{{  VfDot_test() */
static void VfDot_test(void)
{ TIMERS;
  FLOATS;
  float result1, result2;
  int	n	= VectorProcessed;

  T0;
  result1 = VfDot(n, XA, YA);
  T1;
  result2 = 0.0F;
  while (n--)
   result2 += (*XC++ * *YC++);
  T2;

  float_checks(Test_ReadOnly);
  if (result1 != result2)
   fprintf(Output, "Error: results differ, veclib %f, C %f\n", result1, result2);
}
/*}}}*/
/*{{{  VfsDot_test() */
static void VfsDot_test(void)
{ TIMERS;
  FLOATS;
  STRIDES;
  float result1, result2;
  int	n	= StrideProcessed;

  T0;
  result1 = VfsDot(n, XA, XS, YA, YS);
  T1;
  result2 = 0.0F;
  while (n--)
   { result2 += (*XC * *YC); XC += XS; YC += YS; }
  T2;

  float_checks(Test_ReadOnly | Test_StridesUsed);
  if (result1 != result2)
   fprintf(Output, "Error: results differ, veclib %f, C %f\n", result1, result2);
}
/*}}}*/
/*{{{  VdDot_test() */
static void VdDot_test(void)
{ TIMERS;
  DOUBLES;
  double result1, result2;
  int	 n	= VectorProcessed;

  T0;
  result1 = VdDot(n, XA, YA);
  T1;
  result2 = 0.0;
  while (n--)
   result2 += (*XC++ * *YC++);
  T2;

  double_checks(Test_ReadOnly);
  if (result1 != result2)
   fprintf(Output, "Error: results differ, veclib %f, C %f\n", result1, result2);
}
/*}}}*/
/*{{{  VdsDot_test() */
static void VdsDot_test(void)
{ TIMERS;
  DOUBLES;
  STRIDES;
  double result1, result2;
  int	 n	= StrideProcessed;

  T0;
  result1 = VdsDot(n, XA, XS, YA, YS);
  T1;
  result2 = 0.0;
  while (n--)
   { result2 += (*XC * *YC); XC += XS; YC += YS; }
  T2;

  double_checks(Test_ReadOnly | Test_StridesUsed);
  if (result1 != result2)
   fprintf(Output, "Error: results differ, veclib %f, C %f\n", result1, result2);
}
/*}}}*/
/*}}}*/
/*{{{  vector sum and product tests */
/*{{{  VfProd_test() */
static void VfProd_test(void)
{ TIMERS;
  FLOATSX;
  float	 result1, result2;
  int	 n = VectorProcessed;

  T0;
  result1 = VfProd(n, XA);
  T1;
  result2 = 1.0F;
  while (n--)
   result2 *= *XC++;
  T2;

  float_checks(Test_ReadOnly);  
  if (result1 != result2)
   fprintf(Output, "Error: results differ, veclib %f, C %f\n", result1, result2);
}
/*}}}*/
/*{{{  VfsProd_test() */
static void VfsProd_test(void)
{ TIMERS;
  FLOATSX;
  STRIDESX;
  float	 result1, result2;
  int	 n = StrideProcessed;

  T0;
  result1 = VfsProd(n, XA, XS);
  T1;
  result2 = 1.0F;
  while (n--)
   { result2 *= *XC; XC += XS; }
  T2;

  float_checks(Test_ReadOnly | Test_StridesUsed);  
  if (result1 != result2)
   fprintf(Output, "Error: results differ, veclib %f, C %f\n", result1, result2);
}
/*}}}*/
/*{{{  VdProd_test() */
static void VdProd_test(void)
{ TIMERS;
  DOUBLESX;
  double result1, result2;
  int	 n = VectorProcessed;

  T0;
  result1 = VdProd(n, XA);
  T1;
  result2 = 1.0;
  while (n--)
   result2 *= *XC++;
  T2;

  double_checks(Test_ReadOnly);  
  if (result1 != result2)
   fprintf(Output, "Error: results differ, veclib %f, C %f\n", result1, result2);
}
/*}}}*/
/*{{{  VdsProd_test() */
static void VdsProd_test(void)
{ TIMERS;
  DOUBLESX;
  STRIDESX;
  double result1, result2;
  int	 n = VectorProcessed;

  T0;
  result1 = VdsProd(n, XA, XS);
  T1;
  result2 = 1.0;
  while (n--)
   { result2 *= *XC; XC += XS; }
  T2;

  double_checks(Test_ReadOnly | Test_StridesUsed);  
  if (result1 != result2)
   fprintf(Output, "Error: results differ, veclib %f, C %f\n", result1, result2);
}
/*}}}*/
/*{{{  VfSum_test() */
static void VfSum_test(void)
{ TIMERS;
  FLOATSX;
  float	 result1, result2;
  int	 n = VectorProcessed;

  T0;
  result1 = VfSum(n, XA);
  T1;
  result2 = 0.0F;
  while (n--)
   result2 += *XC++;
  T2;

  float_checks(Test_ReadOnly);  
  if (result1 != result2)
   fprintf(Output, "Error: results differ, veclib %f, C %f\n", result1, result2);
}
/*}}}*/
/*{{{  VfsSum_test() */
static void VfsSum_test(void)
{ TIMERS;
  FLOATSX;
  STRIDESX;
  float	 result1, result2;
  int	 n = VectorProcessed;

  T0;
  result1 = VfsSum(n, XA, XS);
  T1;
  result2 = 0.0F;
  while (n--)
   { result2 += *XC; XC += XS; }
  T2;

  float_checks(Test_ReadOnly | Test_StridesUsed);  
  if (result1 != result2)
   fprintf(Output, "Error: results differ, veclib %f, C %f\n", result1, result2);
}
/*}}}*/
/*{{{  VdSum_test() */
static void VdSum_test(void)
{ TIMERS;
  DOUBLESX;
  double result1, result2;
  int	 n = VectorProcessed;

  T0;
  result1 = VdSum(n, XA);
  T1;
  result2 = 0.0;
  while (n--)
   result2 += *XC++;
  T2;

  double_checks(Test_ReadOnly);  
  if (result1 != result2)
   fprintf(Output, "Error: results differ, veclib %f, C %f\n", result1, result2);
}
/*}}}*/
/*{{{  VdsSum_test() */
static void VdsSum_test(void)
{ TIMERS;
  DOUBLESX;
  STRIDESX;
  double result1, result2;
  int	 n = VectorProcessed;

  T0;
  result1 = VdsSum(n, XA, XS);
  T1;
  result2 = 0.0;
  while (n--)
   { result2 += *XC; XC += XS; }
  T2;

  double_checks(Test_ReadOnly | Test_StridesUsed);  
  if (result1 != result2)
   fprintf(Output, "Error: results differ, veclib %f, C %f\n", result1, result2);
}
/*}}}*/
/*}}}*/
/*{{{  vector maxima and minima tests */
#define abs(a) ((a < 0) ? -a : a)
/*{{{  standard float vectors for special case tests */
static float mmF1[] = {   0.0F,   1.0F,  2.0F, 3.0F,   4.0F,   5.0F };
static float mmF2[] = {   5.0F,   4.0F,  3.0F, 2.0F,   1.0F,   0.0F };
static float mmF3[] = {   1.0F,   0.0F,  2.0F, 3.0F,   4.0F,   5.0F };
static float mmF4[] = {   1.0F,   2.0F,  3.0F, 4.0F,   0.0F,   5.0F };
static float mmF5[] = {   0.0F,   5.0F,  1.0F, 0.0F,   3.0F,   4.0F };
static float mmF6[] = { -10.0F,   1.0F, -5.0F, 2.0F,   3.0F,   0.0F };
static float mmF7[] = {   0.0F,   1.0F,  2.0F, 3.0F,   4.0F, -10.0F };
static float mmF8[] = {   0.0F, -10.0F, -5.0F, 2.0F,   3.0F,   4.0F };
static float mmF9[] = {   0.0F,   1.0F,  2.0F, 3.0F, -10.0F,   4.0F };

static double mmD1[] = { 0.0, 1.0, 2.0, 3.0, 4.0, 5.0 };
static double mmD2[] = { 5.0, 4.0, 3.0, 2.0, 1.0, 0.0 };
static double mmD3[] = { 1.0, 0.0, 2.0, 3.0, 4.0, 5.0 };
static double mmD4[] = { 1.0, 2.0, 3.0, 4.0, 0.0, 5.0 };
static double mmD5[] = { 0.0, 5.0, 1.0, 0.0, 3.0, 4.0 };
static double mmD6[] = { -10.0, 1.0, -5.0, 2.0, 3.0, 0.0 };
static double mmD7[] = { 0.0, 1.0, 2.0, 3.0, 4.0, -10.0 };
static double mmD8[] = { 0.0, -10.0, -5.0, 2.0, 3.0, 4.0 };
static double mmD9[] = { 0.0, 1.0, 2.0, 3.0, -10.0, 4.0 };

static void fail(int n)
{ fprintf(Output, "Error: failed on special vector %d\n", n);
}
/*}}}*/
/*{{{  VfMax_test() */
static void VfMax_test(void)
{ TIMERS;
  FLOATSX;
  int	result1, result2;
  int	n = VectorProcessed;

  T0;
  result1 = VfMax(n, XA);
  T1;
  { int index	=	0;
    float max	=	*XC;
    int i;

    for (i = 1; i < n; i++)
     if (XC[i] > max)
      { index = i; max = XC[i]; }

    result2 = index;
  }
  T2;

  float_checks(Test_ReadOnly);
  if (result1 != result2)
   fprintf(Output, "Error: veclib returned %d (%f), C returned %d (%f)\n",
	result1, XA[result1], result2, XC[result2]);

  if (VfMax(6, mmF1) != 5) fail(1);
  if (VfMax(6, mmF2) != 0) fail(2);
  if (VfMax(6, mmF3) != 5) fail(3);
  if (VfMax(6, mmF4) != 5) fail(4);
  if (VfMax(6, mmF5) != 1) fail(5);
  if (VfMax(6, mmF6) != 4) fail(6);
  if (VfMax(6, mmF7) != 4) fail(7);
  if (VfMax(6, mmF8) != 5) fail(8);
  if (VfMax(6, mmF9) != 5) fail(9);
}
/*}}}*/
/*{{{  VfsMax_test() */
static void VfsMax_test(void)
{ TIMERS;
  FLOATSX;
  STRIDESX;
  int	result1, result2;
  int	n = StrideProcessed;

  T0;
  result1 = VfsMax(n, XA, XS);
  T1;
  { int index	=	0;
    float max	=	*XC;
    int i;

    for (i = 1, XC += XS; i < n; i++, XC += XS)
     if (*XC > max)
      { index = i; max = *XC; }

    result2 = index;
  }
  T2;

  float_checks(Test_ReadOnly | Test_StridesUsed);

  if (result1 != result2)
    fprintf(Output, "Error: veclib returned %d (%f), C returned %d (%f)\n",
	    result1, XA[result1], result2, XC[result2]);
  
  if (VfsMax(3, mmF1, 2) != 2) fail(1);
  if (VfsMax(3, mmF2, 2) != 0) fail(2);
  if (VfsMax(3, mmF3, 2) != 2) fail(3);
  if (VfsMax(3, mmF4, 2) != 1) fail(4);
  if (VfsMax(3, mmF5, 2) != 2) fail(5);
  if (VfsMax(3, mmF6, 2) != 2) fail(6);
  if (VfsMax(3, mmF7, 2) != 2) fail(7);
  if (VfsMax(3, mmF8, 2) != 2) fail(8);
  if (VfsMax(3, mmF9, 2) != 1) fail(9);
}
/*}}}*/
/*{{{  VdMax_test() */
static void VdMax_test(void)
{ TIMERS;
  DOUBLESX;
  int	result1, result2;
  int	n = VectorProcessed;

  T0;
  result1 = VdMax(n, XA);
  T1;
  { int  index	= 0;
    double max		= *XC;
    int  i;

    for (i = 1; i < n; i++)
     if (XC[i] > max)
      { index = i; max = XC[i]; }

    result2 = index;
  }
  T2;

  double_checks(Test_ReadOnly);
  if (result1 != result2)
   fprintf(Output, "Error: veclib returned %d (%f), C returned %d (%f)\n",
	result1, XA[result1], result2, XC[result2]);

  if (VdMax(6, mmD1) != 5) fail(1);
  if (VdMax(6, mmD2) != 0) fail(2);
  if (VdMax(6, mmD3) != 5) fail(3);
  if (VdMax(6, mmD4) != 5) fail(4);
  if (VdMax(6, mmD5) != 1) fail(5);
  if (VdMax(6, mmD6) != 4) fail(6);
  if (VdMax(6, mmD7) != 4) fail(7);
  if (VdMax(6, mmD8) != 5) fail(8);
  if (VdMax(6, mmD9) != 5) fail(9);
}
/*}}}*/
/*{{{  VdsMax_test() */
static void VdsMax_test(void)
{ TIMERS;
  DOUBLESX;
  STRIDESX;
  int	result1, result2;
  int	n = VectorProcessed;

  T0;
  result1 = VdsMax(n, XA, XS);
  T1;
  { int  index =	0;
    double max	 = 	*XC;
    int  i;

    for (i = 1, XC += XS; i < n; i++, XC += XS)
     if (*XC > max)
      { index = i; max = *XC; }

    result2 = index;
  }
  T2;

  double_checks(Test_ReadOnly | Test_StridesUsed);
  if (result1 != result2)
   fprintf(Output, "Error: veclib returned %d (%f), C returned %d (%f)\n",
	result1, XA[result1], result2, XC[result2]);

  if (VdsMax(3, mmD1, 2) != 2) fail(1);
  if (VdsMax(3, mmD2, 2) != 0) fail(2);
  if (VdsMax(3, mmD3, 2) != 2) fail(3);
  if (VdsMax(3, mmD4, 2) != 1) fail(4);
  if (VdsMax(3, mmD5, 2) != 2) fail(5);
  if (VdsMax(3, mmD6, 2) != 2) fail(6);
  if (VdsMax(3, mmD7, 2) != 2) fail(7);
  if (VdsMax(3, mmD8, 2) != 2) fail(8);
  if (VdsMax(3, mmD9, 2) != 1) fail(9);
}
/*}}}*/
/*{{{  VfAmax_test() */
static void VfAmax_test(void)
{ TIMERS;
  FLOATSX;
  int	result1, result2;
  int	n = VectorProcessed;

  T0;
  result1 = VfAmax(n, XA);
  T1;
  { int index	=	0;
    float max	=	abs(*XC);
    int i;

    for (i = 1; i < n; i++)
     if (abs(XC[i]) > max)
      { index = i; max = abs(XC[i]); }

    result2 = index;
  }
  T2;

  float_checks(Test_ReadOnly);
  if (result1 != result2)
   fprintf(Output, "Error: veclib returned %d (%f), C returned %d (%f)\n",
	result1, XA[result1], result2, XC[result2]);

  if (VfAmax(6, mmF1) != 5) fail(1);
  if (VfAmax(6, mmF2) != 0) fail(2);
  if (VfAmax(6, mmF3) != 5) fail(3);
  if (VfAmax(6, mmF4) != 5) fail(4);
  if (VfAmax(6, mmF5) != 1) fail(5);
  if (VfAmax(6, mmF6) != 0) fail(6);
  if (VfAmax(6, mmF7) != 5) fail(7);
  if (VfAmax(6, mmF8) != 1) fail(8);
  if (VfAmax(6, mmF9) != 4) fail(9);
}
/*}}}*/
/*{{{  VfsAmax_test() */
static void VfsAmax_test(void)
{ TIMERS;
  FLOATSX;
  STRIDESX;
  int	result1, result2;
  int	n = StrideProcessed;

  T0;
  result1 = VfsAmax(n, XA, XS);
  T1;
  { int index	=	0;
    float max	=	abs(*XC);
    int i;

    for (i = 1, XC += XS; i < n; i++, XC += XS)
     if (abs(*XC) > max)
      { index = i; max = abs(*XC); }

    result2 = index;
  }
  T2;

  float_checks(Test_ReadOnly | Test_StridesUsed);
  if (result1 != result2)
   fprintf(Output, "Error: veclib returned %d (%f), C returned %d (%f)\n",
	result1, XA[result1], result2, XC[result2]);

  if (VfsAmax(3, mmF1, 2) != 2) fail(1);
  if (VfsAmax(3, mmF2, 2) != 0) fail(2);
  if (VfsAmax(3, mmF3, 2) != 2) fail(3);
  if (VfsAmax(3, mmF4, 2) != 1) fail(4);
  if (VfsAmax(3, mmF5, 2) != 2) fail(5);
  if (VfsAmax(3, mmF6, 2) != 0) fail(6);
  if (VfsAmax(3, mmF7, 2) != 2) fail(7);
  if (VfsAmax(3, mmF8, 2) != 1) fail(8);
  if (VfsAmax(3, mmF9, 2) != 2) fail(9);
}
/*}}}*/
/*{{{  VdAmax_test() */
static void VdAmax_test(void)
{ TIMERS;
  DOUBLESX;
  int	result1, result2;
  int	n = VectorProcessed;

  T0;
  result1 = VdAmax(n, XA);
  T1;
  { int  index	= 0;
    double max		= abs(*XC);
    int  i;

    for (i = 1; i < n; i++)
     if (abs(XC[i]) > max)
      { index = i; max = abs(XC[i]); }

    result2 = index;
  }
  T2;

  double_checks(Test_ReadOnly);
  if (result1 != result2)
   fprintf(Output, "Error: veclib returned %d (%f), C returned %d (%f)\n",
	result1, XA[result1], result2, XC[result2]);

  if (VdAmax(6, mmD1) != 5) fail(1);
  if (VdAmax(6, mmD2) != 0) fail(2);
  if (VdAmax(6, mmD3) != 5) fail(3);
  if (VdAmax(6, mmD4) != 5) fail(4);
  if (VdAmax(6, mmD5) != 1) fail(5);
  if (VdAmax(6, mmD6) != 0) fail(6);
  if (VdAmax(6, mmD7) != 5) fail(7);
  if (VdAmax(6, mmD8) != 1) fail(8);
  if (VdAmax(6, mmD9) != 4) fail(9);
}
/*}}}*/
/*{{{  VdsAmax_test() */
static void VdsAmax_test(void)
{ TIMERS;
  DOUBLESX;
  STRIDESX;
  int	result1, result2;
  int	n = VectorProcessed;

  T0;
  result1 = VdsAmax(n, XA, XS);
  T1;
  { int  index  =	0;
    double max	  =	abs(*XC);
    int  i;

    for (i = 1, XC += XS; i < n; i++, XC += XS)
     if (abs(*XC) > max)
      { index = i; max = abs(*XC); }

    result2 = index;
  }
  T2;

  double_checks(Test_ReadOnly | Test_StridesUsed);
  if (result1 != result2)
   fprintf(Output, "Error: veclib returned %d (%f), C returned %d (%f)\n",
	result1, XA[result1], result2, XC[result2]);

  if (VdsAmax(3, mmD1, 2) != 2) fail(1);
  if (VdsAmax(3, mmD2, 2) != 0) fail(2);
  if (VdsAmax(3, mmD3, 2) != 2) fail(3);
  if (VdsAmax(3, mmD4, 2) != 1) fail(4);
  if (VdsAmax(3, mmD5, 2) != 2) fail(5);
  if (VdsAmax(3, mmD6, 2) != 0) fail(6);
  if (VdsAmax(3, mmD7, 2) != 2) fail(7);
  if (VdsAmax(3, mmD8, 2) != 1) fail(8);
  if (VdsAmax(3, mmD9, 2) != 2) fail(9);
}
/*}}}*/
/*{{{  VfMin_test() */
static void VfMin_test(void)
{ TIMERS;
  FLOATSX;
  int	result1, result2;
  int	n = VectorProcessed;

  T0;
  result1 = VfMin(n, XA);
  T1;
  { int index	=	0;
    float min	=	*XC;
    int i;

    for (i = 1; i < n; i++)
     if (XC[i] < min)
      { index = i; min = XC[i]; }

    result2 = index;
  }
  T2;

  float_checks(Test_ReadOnly);
  if (result1 != result2)
   fprintf(Output, "Error: veclib returned %d (%f), C returned %d (%f)\n",
	result1, XA[result1], result2, XC[result2]);

  if (VfMin(6, mmF1) != 0) fail(1);
  if (VfMin(6, mmF2) != 5) fail(2);
  if (VfMin(6, mmF3) != 1) fail(3);
  if (VfMin(6, mmF4) != 4) fail(4);
  if (VfMin(6, mmF5) != 0) fail(5);
  if (VfMin(6, mmF6) != 0) fail(6);
  if (VfMin(6, mmF7) != 5) fail(7);
  if (VfMin(6, mmF8) != 1) fail(8);
  if (VfMin(6, mmF9) != 4) fail(9);
}
/*}}}*/
/*{{{  VfsMin_test() */
static void VfsMin_test(void)
{ TIMERS;
  FLOATSX;
  STRIDESX;
  int	result1, result2;
  int	n = StrideProcessed;

  T0;
  result1 = VfsMin(n, XA, XS);
  T1;
  { int index	=	0;
    float min	=	*XC;
    int i;

    for (i = 1, XC += XS; i < n; i++, XC += XS)
     if (*XC < min)
      { index = i; min = *XC; }

    result2 = index;
  }
  T2;

  float_checks(Test_ReadOnly | Test_StridesUsed);
  if (result1 != result2)
   fprintf(Output, "Error: veclib returned %d (%f), C returned %d (%f)\n",
	result1, XA[result1], result2, XC[result2]);

  if (VfsMin(3, mmF1, 2) != 0) fail(1);
  if (VfsMin(3, mmF2, 2) != 2) fail(2);
  if (VfsMin(3, mmF3, 2) != 0) fail(3);
  if (VfsMin(3, mmF4, 2) != 2) fail(4);
  if (VfsMin(3, mmF5, 2) != 0) fail(5);
  if (VfsMin(3, mmF6, 2) != 0) fail(6);
  if (VfsMin(3, mmF7, 2) != 0) fail(7);
  if (VfsMin(3, mmF8, 2) != 1) fail(8);
  if (VfsMin(3, mmF9, 2) != 2) fail(9);
}
/*}}}*/
/*{{{  VdMin_test() */
static void VdMin_test(void)
{ TIMERS;
  DOUBLESX;
  int	result1, result2;
  int	n = VectorProcessed;

  T0;
  result1 = VdMin(n, XA);
  T1;
  { int  index	= 0;
    double min		= *XC;
    int  i;

    for (i = 1; i < n; i++)
     if (XC[i] < min)
      { index = i; min = XC[i]; }

    result2 = index;
  }
  T2;

  double_checks(Test_ReadOnly);
  if (result1 != result2)
   fprintf(Output, "Error: veclib returned %d (%f), C returned %d (%f)\n",
	result1, XA[result1], result2, XC[result2]);

  if (VdMin(6, mmD1) != 0) fail(1);
  if (VdMin(6, mmD2) != 5) fail(2);
  if (VdMin(6, mmD3) != 1) fail(3);
  if (VdMin(6, mmD4) != 4) fail(4);
  if (VdMin(6, mmD5) != 0) fail(5);
  if (VdMin(6, mmD6) != 0) fail(6);
  if (VdMin(6, mmD7) != 5) fail(7);
  if (VdMin(6, mmD8) != 1) fail(8);
  if (VdMin(6, mmD9) != 4) fail(9);
}
/*}}}*/
/*{{{  VdsMin_test() */
static void VdsMin_test(void)
{ TIMERS;
  DOUBLESX;
  STRIDESX;
  int	result1, result2;
  int	n = VectorProcessed;

  T0;
  result1 = VdsMin(n, XA, XS);
  T1;
  { int  index  =	0;
    double min	  =	*XC;
    int  i;

    for (i = 1, XC += XS; i < n; i++, XC += XS)
     if (*XC < min)
      { index = i; min = *XC; }

    result2 = index;
  }
  T2;

  double_checks(Test_ReadOnly | Test_StridesUsed);
  if (result1 != result2)
   fprintf(Output, "Error: veclib returned %d (%f), C returned %d (%f)\n",
	result1, XA[result1], result2, XC[result2]);

  if (VdsMin(3, mmD1, 2) != 0) fail(1);
  if (VdsMin(3, mmD2, 2) != 2) fail(2);
  if (VdsMin(3, mmD3, 2) != 0) fail(3);
  if (VdsMin(3, mmD4, 2) != 2) fail(4);
  if (VdsMin(3, mmD5, 2) != 0) fail(5);
  if (VdsMin(3, mmD6, 2) != 0) fail(6);
  if (VdsMin(3, mmD7, 2) != 0) fail(7);
  if (VdsMin(3, mmD8, 2) != 1) fail(8);
  if (VdsMin(3, mmD9, 2) != 2) fail(9);
}
/*}}}*/
/*{{{  VfAmin_test() */
static void VfAmin_test(void)
{ TIMERS;
  FLOATSX;
  int	result1, result2;
  int	n = VectorProcessed;

  T0;
  result1 = VfAmin(n, XA);
  T1;
  { int index	=	0;
    float min	=	abs(*XC);
    int i;

    for (i = 1; i < n; i++)
     if (abs(XC[i]) < min)
      { index = i; min = abs(XC[i]); }

    result2 = index;
  }
  T2;

  float_checks(Test_ReadOnly);
  if (result1 != result2)
   fprintf(Output, "Error: veclib returned %d (%f), C returned %d (%f)\n",
	result1, XA[result1], result2, XC[result2]);

  if (VfAmin(6, mmF1) != 0) fail(1);
  if (VfAmin(6, mmF2) != 5) fail(2);
  if (VfAmin(6, mmF3) != 1) fail(3);
  if (VfAmin(6, mmF4) != 4) fail(4);
  if (VfAmin(6, mmF5) != 0) fail(5);
  if (VfAmin(6, mmF6) != 5) fail(6);
  if (VfAmin(6, mmF7) != 0) fail(7);
  if (VfAmin(6, mmF8) != 0) fail(8);
  if (VfAmin(6, mmF9) != 0) fail(9);
}
/*}}}*/
/*{{{  VfsAmin_test() */
static void VfsAmin_test(void)
{ TIMERS;
  FLOATSX;
  STRIDESX;
  int	result1, result2;
  int	n = StrideProcessed;

  T0;
  result1 = VfsAmin(n, XA, XS);
  T1;
  { int index	=	0;
    float min	=	abs(*XC);
    int i;

    for (i = 1, XC += XS; i < n; i++, XC += XS)
     if (abs(*XC) < min)
      { index = i; min = abs(*XC); }

    result2 = index;
  }
  T2;

  float_checks(Test_ReadOnly | Test_StridesUsed);
  if (result1 != result2)
   fprintf(Output, "Error: veclib returned %d (%f), C returned %d (%f)\n",
	result1, XA[result1], result2, XC[result2]);

  if (VfsAmin(3, mmF1, 2) != 0) fail(1);
  if (VfsAmin(3, mmF2, 2) != 2) fail(2);
  if (VfsAmin(3, mmF3, 2) != 0) fail(3);
  if (VfsAmin(3, mmF4, 2) != 2) fail(4);
  if (VfsAmin(3, mmF5, 2) != 0) fail(5);
  if (VfsAmin(3, mmF6, 2) != 2) fail(6);
  if (VfsAmin(3, mmF7, 2) != 0) fail(7);
  if (VfsAmin(3, mmF8, 2) != 0) fail(8);
  if (VfsAmin(3, mmF9, 2) != 0) fail(9);
}
/*}}}*/
/*{{{  VdAmin_test() */
static void VdAmin_test(void)
{ TIMERS;
  DOUBLESX;
  int	result1, result2;
  int	n = VectorProcessed;

  T0;
  result1 = VdAmin(n, XA);
  T1;
  { int  index	= 0;
    double min		= abs(*XC);
    int  i;

    for (i = 1; i < n; i++)
     if (abs(XC[i]) < min)
      { index = i; min = abs(XC[i]); }

    result2 = index;
  }
  T2;

  double_checks(Test_ReadOnly);
  if (result1 != result2)
   fprintf(Output, "Error: veclib returned %d (%f), C returned %d (%f)\n",
	result1, XA[result1], result2, XC[result2]);

  if (VdAmin(6, mmD1) != 0) fail(1);
  if (VdAmin(6, mmD2) != 5) fail(2);
  if (VdAmin(6, mmD3) != 1) fail(3);
  if (VdAmin(6, mmD4) != 4) fail(4);
  if (VdAmin(6, mmD5) != 0) fail(5);
  if (VdAmin(6, mmD6) != 5) fail(6);
  if (VdAmin(6, mmD7) != 0) fail(7);
  if (VdAmin(6, mmD8) != 0) fail(8);
  if (VdAmin(6, mmD9) != 0) fail(9);
}
/*}}}*/
/*{{{  VdsAmin_test() */
static void VdsAmin_test(void)
{ TIMERS;
  DOUBLESX;
  STRIDESX;
  int	result1, result2;
  int	n = VectorProcessed;

  T0;
  result1 = VdsAmin(n, XA, XS);
  T1;
  { int  index  =	0;
    double min	  =	abs(*XC);
    int  i;

    for (i = 1, XC += XS; i < n; i++, XC += XS)
     if (abs(*XC) < min)
      { index = i; min = abs(*XC); }

    result2 = index;
  }
  T2;

  double_checks(Test_ReadOnly | Test_StridesUsed);
  if (result1 != result2)
   fprintf(Output, "Error: veclib returned %d (%f), C returned %d (%f)\n",
	result1, XA[result1], result2, XC[result2]);

  if (VdsAmin(3, mmD1, 2) != 0) fail(1);
  if (VdsAmin(3, mmD2, 2) != 2) fail(2);
  if (VdsAmin(3, mmD3, 2) != 0) fail(3);
  if (VdsAmin(3, mmD4, 2) != 2) fail(4);
  if (VdsAmin(3, mmD5, 2) != 0) fail(5);
  if (VdsAmin(3, mmD6, 2) != 2) fail(6);
  if (VdsAmin(3, mmD7, 2) != 0) fail(7);
  if (VdsAmin(3, mmD8, 2) != 0) fail(8);
  if (VdsAmin(3, mmD9, 2) != 0) fail(9);
}
/*}}}*/
/*}}}*/
/*{{{  miscellaneous tests */
/*{{{  big matrices */
#define N	128
static float	Af[N][N];
static float	Bf[N][N];
static float	Cf[N][N];
static float	Df[N][N];
static double	Ad[N][N];
static double	Bd[N][N];
static double	Cd[N][N];
static double	Dd[N][N];
static float	Scratchf[N];
static double	Scratchd[N];

static void	init_float_matrix(float *m)
{ int	i;
  for (i = 0; i < (N * N); i++)
   m[i] = FRand();
}

static void	init_double_matrix(double *m)
{ int	i;
  for (i = 0; i < (N * N); i++)
   m[i] = DRand();
}
/*}}}*/
/*{{{  float matrix multiplication */
static void float_matrix_multiply(void)
{ TIMERS;
  int i, j, k;

  init_float_matrix(&(Af[0][0]));
  init_float_matrix(&(Bf[0][0]));

  fprintf(Output, "Performing a simple %dx%d float matrix multiply using veclib.\n", N, N);
  fprintf(Output, "Please note: this uses N cubed multiplications, not Strassen's method.\n");

  T0;
  for (i = 0; i < N; i++)
   for (j = 0; j < N; j++)
    Cf[i][j] = VfsDot(N, &(Af[i][0]), 1, &(Bf[0][j]), N);
  T1;
  for (i = 0; i < N; i++)
   for (j = 0; j < N; j++)
    { Df[i][j] = 0.0F;
      for (k = 0; k < N; k++)
       Df[i][j] += Af[i][k] * Bf[k][j];
    }
  T2;

  if (memcmp(Cf, Df, sizeof(Cf)))
   fprintf(Output, "Error: the resulting matrices differ.\n");
}
/*}}}*/
/*{{{  double matrix multiplication */
static void double_matrix_multiply(void)
{ TIMERS;
  int i, j, k;

  init_double_matrix(&(Ad[0][0]));
  init_double_matrix(&(Bd[0][0]));

  fprintf(Output, "Performing a simple %dx%d double matrix multiply using veclib.\n", N, N);
  fprintf(Output, "Please note: this uses N cubed multiplications, not Strassen's method.\n");

  T0;
  for (i = 0; i < N; i++)
   for (j = 0; j < N; j++)
    Cd[i][j] = VdsDot(N, &(Ad[i][0]), 1, &(Bd[0][j]), N);
  T1;
  for (i = 0; i < N; i++)
   for (j = 0; j < N; j++)
    { Dd[i][j] = 0.0;
      for (k = 0; k < N; k++)
       Dd[i][j] += Ad[i][k] * Bd[k][j];
    }
  T2;

  if (memcmp(Cf, Df, sizeof(Cf)))
   fprintf(Output, "Error: the resulting matrices differ.\n");
}
/*}}}*/
/*{{{  float Gaussian */
#define GAUSS_EPSILON	0.0005F
	/* This is my attempt at a Gaussian, not necessarily correct.	*/
static void float_gaussian(void)
{ TIMERS;
  int	i, j, k;
  int	index;
  float max, temp;
  float scalar;

	/* veclib uses Af, C uses Bf					*/
  init_float_matrix(&(Af[0][0]));
  memcpy(&(Bf[0][0]), &(Af[0][0]), sizeof(Af));

  T0;
  for (i = 0; i < (N-1); i++)
   {		/* Find the pivot element for column i			*/
     index = i + VfsAmax(N - i, &(Af[i][i]), N);

		/* Interchange row index with row i			*/
     if (index != i)
      { VfCopy(N - i, Scratchf, &(Af[i][i]));
        VfCopy(N - i, &(Af[i][i]), &(Af[index][i]));
        VfCopy(N - i, &(Af[index][i]), Scratchf);
      }

     for (j = i + 1; j < N; j++)
      { scalar = -1 * (Af[j][i] / Af[i][i]);
        VfMulAdd(scalar, N - i, &(Af[j][i]), &(Af[i][i]));
      }
   }

  T1;

  for (i = 0; i < (N-1); i++)
   { 		/* Find the pivot element for this column		*/
     index = i; max = abs(Bf[index][i]);
     for (j = i+1; j < N; j++)
      if (abs(Bf[j][i]) > max)
       { index = j; max = abs(Bf[j][i]); }

		/* Interchange row index with row i			*/
     if (index != i)
      for (k = i; k < N; k++)
       { temp		= Bf[i][k];
 	 Bf[i][k]	= Bf[index][k];
	 Bf[index][k]	= temp;
       }

		/* Eliminate the remaining rows.			*/
     for (j = i+1; j < N; j++)
      { scalar = (Bf[j][i] / Bf[i][i]);
        for (k = i; k < N; k++)
         Bf[j][k] -= (scalar * Bf[i][k]);
      }
   }

  T2;

  if (memcmp(Af, Bf, sizeof(Af)))
   { fprintf(Output, "Error: vector library and C code differ.\n");
     for (i = 1; i < N; i++)
      for (j = 1; j < N; j++)
       if (Af[i][j] != Bf[i][j])
        fprintf(Output, "veclib[%d][%d] is %f, C[%d][%d] is %f\n",
		i, j, Af[i][j], i, j, Bf[i][j]);
   }

  for (i = 1; i < N; i++)
   for (j = 0; j < i; j++)
    if (abs(Af[i][j]) > GAUSS_EPSILON)
    fprintf(Output, "Warning: A[%d][%d] is %f, not zero\n", i, j, Af[i][j]);
}
#undef GAUSS_EPSILON
/*}}}*/
/*{{{  double Gaussian */
#define GAUSS_EPSILON	0.000001
static void double_gaussian(void)
{ TIMERS;
  int	 i, j, k;
  int	 index;
  double max, temp;
  double scalar;

	/* veclib uses Ad, C uses Bd					*/
  init_double_matrix(&(Ad[0][0]));
  memcpy(&(Bd[0][0]), &(Ad[0][0]), sizeof(Ad));

  T0;
  for (i = 0; i < (N-1); i++)
   {		/* Find the pivot element for column i			*/
     index = i + VdsAmax(N - i, &(Ad[i][i]), N);

		/* Interchange row index with row i			*/
     if (index != i)
      { VdCopy(N - i, Scratchd, &(Ad[i][i]));
        VdCopy(N - i, &(Ad[i][i]), &(Ad[index][i]));
        VdCopy(N - i, &(Ad[index][i]), Scratchd);
      }

     for (j = i + 1; j < N; j++)
      { scalar = -1 * (Ad[j][i] / Ad[i][i]);
        VdMulAdd(scalar, N - i, &(Ad[j][i]), &(Ad[i][i]));
      }
   }
  T1;
  for (i = 0; i < (N-1); i++)
   { 		/* Find the pivot element for this column		*/
     index = i; max = abs(Bd[index][i]);
     for (j = i+1; j < N; j++)
      if (abs(Bd[j][i]) > max)
       { index = j; max = abs(Bd[j][i]); }

		/* Interchange row index with row i			*/
     if (index != i)
      for (k = i; k < N; k++)
       { temp		= Bd[i][k];
 	 Bd[i][k]	= Bd[index][k];
	 Bd[index][k]	= temp;
       }

		/* Eliminate the remaining rows.			*/
     for (j = i+1; j < N; j++)
      { scalar = (Bd[j][i] / Bd[i][i]);
        for (k = i; k < N; k++)
         Bd[j][k] -= (scalar * Bd[i][k]);
      }
   }

  T2;

  if (memcmp(Ad, Bd, sizeof(Ad)))
   { fprintf(Output, "Error: vector library and C code differ.\n");
     for (i = 1; i < N; i++)
      for (j = 1; j < N; j++)
       if (Ad[i][j] != Bd[i][j])
        fprintf(Output, "veclib[%d][%d] is %f, C[%d][%d] is %f\n",
		i, j, Ad[i][j], i, j, Bd[i][j]);
   }

  for (i = 1; i < N; i++)
   for (j = 0; j < i; j++)
    if (abs(Ad[i][j]) > GAUSS_EPSILON)
     fprintf(Output, "Warning: A[%d][%d] is %f, not zero\n", i, j, Ad[i][j]);
}
#undef GAUSS_EPSILON
/*}}}*/
/*{{{  float error conditions */
static void float_errors(void)
{ FLOATS;

  fprintf(Output, "This routine checks the internal error testing of this program.\n");
  fprintf(Output, "It should report the following errors.\n");
  fprintf(Output, " 1) corruption of guards in X and Y vectors for C and assembler.\n");
  fprintf(Output, " 2) corruption of Y vectors for C and assembler.\n");
  fprintf(Output, " 3) significant difference between C and assembler results.\n");
  fprintf(Output, " 4) small discrepancies between C and assembler results.\n");
  fprintf(Output, " 5) corruption of element between strides.\n\n");

  memcpy(XA, YA, sizeof(YA));
  memcpy(XC, YC, sizeof(YC));
  XA[-2]			= 999;
  YA[VectorProcessed + 3]	= -999;
  XC[3]				= 1000;
  XC[VectorProcessed]		= -1000;
  YA[99]			= 4321;
  YC[23]			= 1234;
  XA[12]			= 12345;
  XA[24]			*= (long)(1.0 - (EPSILON / 2));
  XA[13]			= 54321;
  
  float_checks(Test_StridesUsed);
}
/*}}}*/
/*{{{  double error conditions */
static void double_errors(void)
{ DOUBLES;

  fprintf(Output, "This routine checks the internal error testing of this program.\n");
  fprintf(Output, "It should report the following errors.\n");
  fprintf(Output, " 1) corruption of guards in X and Y vectors for C and assembler.\n");
  fprintf(Output, " 2) corruption of Y vectors for C and assembler.\n");
  fprintf(Output, " 3) significant difference between C and assembler results.\n");
  fprintf(Output, " 4) small discrepancies between C and assembler results.\n");
  fprintf(Output, " 5) corruption of element between strides.\n\n");

  memcpy(XA, YA, sizeof(YA));
  memcpy(XC, YC, sizeof(YC));
  XA[-2]			= 999;
  YA[VectorProcessed + 3]	= -999;
  XC[3]				= 1000;
  XC[VectorProcessed]		= -1000;
  YA[99]			= 4321;
  YC[23]			= 1234;
  XA[12]			= 12345;
  XA[24]			*= (1.0 - (EPSILON / 2));
  XA[13]			= 54321;
  
  double_checks(Test_StridesUsed);
}
/*}}}*/
/*}}}*/
/*{{{  table of tests and the do_test() routine */
typedef struct TestEntry {
	bool		 Standard;	/* run normally.	*/
	char		*Name;		/* of test		*/
	VoidFnPtr	 Routine;	/* to perform the test.	*/
} TestEntry;

TestEntry Tests[] = {
	{ TRUE,		"VfAdd",		&VfAdd_test		},
	{ TRUE,		"VfSub",		&VfSub_test		},
	{ TRUE,		"VfMul",		&VfMul_test		},
	{ TRUE,		"VfDiv",		&VfDiv_test		},
	{ TRUE,		"VfsAdd",		&VfsAdd_test		},
	{ TRUE,		"VfsSub",		&VfsSub_test		},
	{ TRUE,		"VfsMul",		&VfsMul_test		},
	{ TRUE,		"VfsDiv",		&VfsDiv_test		},
	{ TRUE,		"VdAdd",		&VdAdd_test		},
	{ TRUE,		"VdSub",		&VdSub_test		},
	{ TRUE,		"VdMul",		&VdMul_test		},
	{ TRUE,		"VdDiv",		&VdDiv_test		},
	{ TRUE,		"VdsAdd",		&VdsAdd_test		},
	{ TRUE,		"VdsSub",		&VdsSub_test		},
	{ TRUE,		"VdsMul",		&VdsMul_test		},
	{ TRUE,		"VdsDiv",		&VdsDiv_test		},
	{ TRUE,		"VfAddScalar",	&VfAddScalar_test	},
	{ TRUE,		"VfSubScalar",	&VfSubScalar_test	},
	{ TRUE,		"VfMulScalar",	&VfMulScalar_test	},
	{ TRUE,		"VfDivScalar",	&VfDivScalar_test	},
	{ TRUE,		"VfRecScalar",	&VfRecScalar_test	},
	{ TRUE,		"VfsAddScalar",	&VfsAddScalar_test	},
	{ TRUE,		"VfsSubScalar",	&VfsSubScalar_test	},
	{ TRUE,		"VfsMulScalar",	&VfsMulScalar_test	},
	{ TRUE,		"VfsDivScalar",	&VfsDivScalar_test	},
	{ TRUE,		"VfsRecScalar",	&VfsRecScalar_test	},
	{ TRUE,		"VdAddScalar",	&VdAddScalar_test	},
	{ TRUE,		"VdSubScalar",	&VdSubScalar_test	},
	{ TRUE,		"VdMulScalar",	&VdMulScalar_test	},
	{ TRUE,		"VdDivScalar",	&VdDivScalar_test	},
	{ TRUE,		"VdRecScalar",	&VdRecScalar_test	},
	{ TRUE,		"VdsAddScalar",	&VdsAddScalar_test	},
	{ TRUE,		"VdsSubScalar",	&VdsSubScalar_test	},
	{ TRUE,		"VdsMulScalar",	&VdsMulScalar_test	},
	{ TRUE,		"VdsDivScalar",	&VdsDivScalar_test	},
	{ TRUE,		"VdsRecScalar",	&VdsRecScalar_test	},
	{ TRUE,		"VfMulAdd",		&VfMulAdd_test		},
	{ TRUE,		"VfsMulAdd",		&VfsMulAdd_test		},
	{ TRUE,		"VdMulAdd",		&VdMulAdd_test		},
	{ TRUE,		"VdsMulAdd",		&VdsMulAdd_test		},
	{ TRUE,		"VfCopy",		&VfCopy_test		},
	{ TRUE,		"VfsCopy",		&VfsCopy_test		},
	{ TRUE,		"VdCopy",		&VdCopy_test		},
	{ TRUE,		"VdsCopy",		&VdsCopy_test		},
	{ TRUE,		"VfFill",		&VfFill_test		},
	{ TRUE,		"VfsFill",		&VfsFill_test		},
	{ TRUE,		"VdFill",		&VdFill_test		},
	{ TRUE,		"VdsFill",		&VdsFill_test		},
	{ TRUE,		"VfMax",		&VfMax_test		},
	{ TRUE,		"VfMin",		&VfMin_test		},
	{ TRUE,		"VfAmax",		&VfAmax_test		},
	{ TRUE,		"VfAmin",		&VfAmin_test		},
	{ TRUE,		"VfsMax",		&VfsMax_test		},
	{ TRUE,		"VfsMin",		&VfsMin_test		},
	{ TRUE,		"VfsAmax",		&VfsAmax_test		},
	{ TRUE,		"VfsAmin",		&VfsAmin_test		},
	{ TRUE,		"VdMax",		&VdMax_test		},
	{ TRUE,		"VdMin",		&VdMin_test		},
	{ TRUE,		"VdAmax",		&VdAmax_test		},
	{ TRUE,		"VdAmin",		&VdAmin_test		},
	{ TRUE,		"VdsMax",		&VdsMax_test		},
	{ TRUE,		"VdsMin",		&VdsMin_test		},
	{ TRUE,		"VdsAmax",		&VdsAmax_test		},
	{ TRUE,		"VdsAmin",		&VdsAmin_test		},
	{ TRUE,		"VfDot",		&VfDot_test		},
	{ TRUE,		"VfsDot",		&VfsDot_test		},
	{ TRUE,		"VdDot",		&VdDot_test		},
	{ TRUE,		"VdsDot",		&VdsDot_test		},
	{ TRUE,		"VfSum",		&VfSum_test		},
	{ TRUE,		"VfsSum",		&VfsSum_test		},
	{ TRUE,		"VdSum",		&VdSum_test		},
	{ TRUE,		"VdsSum",		&VdsSum_test		},
	{ TRUE,		"VfProd",		&VfProd_test		},
	{ TRUE,		"VfsProd",		&VfsProd_test		},
	{ TRUE,		"VdProd",		&VdProd_test		},
	{ TRUE,		"VdsProd",		&VdsProd_test		},
	{ FALSE,	"float matrix multiply", &float_matrix_multiply	},
	{ FALSE,	"double matrix multiply", &double_matrix_multiply },
	{ FALSE,	"float Gaussian",	&float_gaussian		},
	{ FALSE,	"double Gaussian",	&double_gaussian	},
	{ FALSE,	".float errors",	&float_errors		},
	{ FALSE,	".double errors",	&double_errors		},
	{ FALSE,	NULL,			NULL			}
};
	
/*
** The do_test() routine performs the following.
**  1) if no on-chip memory has been allocated yet for the stack,
**     get some.
**  2) print out the name of the routine.
**  3) fill in the assembler and C vectors using the master vector.
**  4) call the appropriate function via Accelerate().
**  5) checksum the master vectors to detect corruption.
*/
static void do_test(int index)
{ float		f_check;
  double 	d_check;
  int		i;

	/* For processors with on-chip RAM, use Accelerate.	*/
#if StackToOnChip
  static	Carrier		*fastmem = NULL;

  if (fastmem == NULL)
   { fastmem = AllocFast(StackSize, &MyTask->MemPool);
    if (fastmem == NULL)
     { fprintf(stderr, "vectest: warning, cannot allocate on-chip memory for stack.\n");
       fastmem = (Carrier *) &fastmem;		/* avoid repeated messages	*/
     }
   }
#endif		/* processor has on-chip RAM			*/

  unless (CompactOutput)
   fprintf(Output, "Performing test %s\n", Tests[index].Name);
  TestName = Tests[index].Name;
  if (Tests[index].Name[1] == 'f')
   { memcpy(Xf_Asm, Xf_Master, sizeof(Xf_Master));
     memcpy(Yf_Asm, Yf_Master, sizeof(Yf_Master));
     memcpy(Xf_C,   Xf_Master, sizeof(Xf_Master));
     memcpy(Yf_C,   Yf_Master, sizeof(Yf_Master));
   }
  elif (Tests[index].Name[1] == 'd')
   { memcpy(Xd_Asm, Xd_Master, sizeof(Xd_Master));
     memcpy(Yd_Asm, Yd_Master, sizeof(Yd_Master));
     memcpy(Xd_C,   Xd_Master, sizeof(Yd_Master));
     memcpy(Yd_C,   Yd_Master, sizeof(Yd_Master));
   }

#if StackToOnChip
  if (fastmem != (Carrier *) &fastmem)
   Accelerate(fastmem, Tests[index].Routine, 0);
  else
#endif
   (*Tests[index].Routine)();

  for (i = 0, f_check = 0.0F, d_check = 0.0; i < VectorSize; i++)
   { f_check += Xf_Master[i];
     f_check += Yf_Master[i];
     d_check += Xd_Master[i];
     d_check += Yd_Master[i];
   }
  if (f_check != F_CheckSum)
   { fprintf(Output, "Corruption of master float vectors detected.\n");
     F_CheckSum = f_check;
   }
  if (d_check != D_CheckSum)
   { fprintf(Output, "Corruption of master double vectors detected.\n");
     D_CheckSum = d_check;
   }
}
/*}}}*/
/*{{{  menu */
/*{{{  statics */
/*
** These statics are used to manage the menu.
*/
static	int	First;		/* index of first function	*/
static	int	Current;	/* ditto for current		*/
static	int	NumberFuncs;	/* total number of tests	*/

static  char	*Normal	 = "\033[0m";	/* normal video		*/
static	char	*Inverse = "\033[7m";	/* inverse video	*/
/*}}}*/
/*{{{  first */
static void first_test(void)
{ Current = First = 0;
}
/*}}}*/
/*{{{  last */
static void last_test(void)
{ 
  Current = NumberFuncs - 1;
  while ((First + 30) < NumberFuncs)
   First += 30;
}
/*}}}*/
/*{{{  next */
static void next_page(void)
{ if ((First + 30) < NumberFuncs)
   { First += 30; Current = First; }
}
/*}}}*/
/*{{{  prev */
static void previous_page(void)
{ if (First > 0)
   { First -= 30; Current = First; }
}
/*}}}*/
/*{{{  up */
static void up_line(void)
{ if ((Current == First) || (Current == (First + 15)))
   { if ((Current + 14) < NumberFuncs)
      Current += 14;
     else
      Current = NumberFuncs - 1;
   }
  else
   Current--;
}
/*}}}*/
/*{{{  down */
static void down_line(void)
{ if ((Current == (First + 14)) || (Current == (First + 29)))
   Current -= 14;
  else
   { if (Current == (NumberFuncs - 1))
      { if (Current < (First + 15))
	 Current = First;
	else
	 Current = First + 15;
      }
     else
      Current++;
   }
}
/*}}}*/
/*{{{  left */
static void left(void)
{ 
  if (Current >= (First + 15))	/* in second column	*/
   Current -= 15;
  elif ((Current + 15) < NumberFuncs)
   Current += 15;
}
/*}}}*/
/*{{{  right */
static void right(void)
{
  if (Current >= (First + 15))	/* in first column	*/
   Current -= 15;
  elif ((Current + 15) < NumberFuncs)
   Current += 15;
}
/*}}}*/
/*{{{  CSI handling */
static bool cursor_key(void)
{ char	buf[16];

  if (Read(fdstream(0), buf, 1, OneSec) < 1)
   return(TRUE);	/* What's up Doc ?	*/

  if ((buf[0] >= 'A') && (buf[0] <= 'D'))	/* cursor key */
   { switch (buf[0])
      { case 'A'	: up_line(); break;
	case 'B'	: down_line(); break;
	case 'C'	: right(); break;
	case 'D'	: left(); break;
      }
     return(TRUE);	/* no redrawing needed	*/
   }

  if (buf[0] == 'H')	/* HOME	*/
   { first_test(); return(FALSE); }

  if ((buf[0] >= '2') && (buf[0] <= '4'))	/* potentially END, PgUp, PgDn */
   { if (Read(fdstream(0), &(buf[1]), 1, OneSec) < 1)
      return(TRUE);		/* What's up Doc ?	*/

     if (buf[1] == '~')		/* Sorry, function key	*/
      return(TRUE);

     if (buf[1] == 'z')		/* Bingo		*/
      { switch(buf[0])
	 { case '2'	: last_test(); break;		/* END	*/
	   case '3'	: previous_page(); break;	/* PgUp	*/
	   case '4'	: next_page(); break;		/* PgDn	*/
	 }
	return(FALSE);
      }
   }

	/* The start of a CSI has been detected but it is not a	*/
	/* recognised one. Discard the rest of it.		*/
  (void) Read(fdstream(0), buf, 16, OneSec);
  return(TRUE);
}
/*}}}*/
/*{{{  run current test */
static void run_test(void)
{ clear_screen();
  do_test(Current);
  waitfor_user();
}
/*}}}*/
/*{{{  shell escape */
static void shell_escape(void)
{ int	pid, rc;

  printf("\n"); fflush(stdout);

  if ((pid = vfork()) == 0)
   { execl("/helios/bin/shell", "shell", NULL);
     _exit(0);
   }
  else
   waitpid(pid, &rc, 0);
  initialise_screen();
}
/*}}}*/

static void menu(void)
{ int	i;
  int	input;

  First = Current = 0;
  for (NumberFuncs = 0; Tests[NumberFuncs].Name != NULL; NumberFuncs++);

  initialise_screen();
  if ((ScreenHeight < 20) || (ScreenWidth < 60))
   { fprintf(stderr, "vectest: screen too small.\n");
     exit(EXIT_FAILURE);
   }

  forever
   { clear_screen();
     printf("\t\tVector Library Test Suite Version %s", VersionNumber);
     
     for (i = 0; i < 15; i++)
      { if ((First + i) == NumberFuncs) break;
	move_to(3 + i, 1);
	fputs(Tests[First+i].Name, stdout);
      }
     for (i = 15; i < 30; i++)
      { if ((First + i) == NumberFuncs) break;
	move_to(3 + i - 15, ScreenWidth / 2);
	fputs(Tests[First+i].Name, stdout);
      }
back:
     if (Current >= (First + 15))
      move_to(3 + (Current - First) - 15, ScreenWidth / 2);
     else
      move_to(3 + (Current - First), 1);
     printf("%s%s%s", Inverse, Tests[Current].Name, Normal);

     move_to(ScreenHeight - 1, 1);
     printf("%sF%sirst, %sL%sast, %sN%sext, %sP%srev, %sQ%suit, %s!%s shell\n",
		Inverse, Normal,Inverse, Normal, Inverse, Normal,
		Inverse, Normal,Inverse, Normal, Inverse, Normal);
     printf("Use cursor keys to select, return to run test.");
     fflush(stdout);

     input = getchar();
     if (Current >= (First + 15))
      move_to(3 + (Current - First) - 15, ScreenWidth / 2);
     else
      move_to(3 + (Current - First), 1);
     fputs(Tests[Current].Name, stdout);
     move_to(ScreenHeight, ScreenWidth);
     fflush(stdout);

     switch(input)
      { case 'q'	:
	case 'Q'	:
	case 0x03	:	/* ctrl-C	*/
	case 0x04	: 	/* ctrl-D	*/
	case 0x07	:	/* ctrl-G	*/
			  fputs("\n", stdout);
			  return;

	case 0x0C	: continue;	/* ctrl-L, redraw	*/

	case 'f'	:
	case 'F'	: first_test(); continue;

	case 'l'	:
	case 'L'	: last_test(); continue;

	case 0x16	:	/* ctrl-V	*/
	case 'n'	:
	case 'N'	: next_page(); continue;

	case 0x1A	:	/* ctrl-Z	*/
	case 'p'	:
	case 'P'	: previous_page(); continue;

	case 0x0E	: down_line();	/* ctrl-N	*/
			  break;

	case 0x10	: up_line();	/* ctrl-P	*/
			  break;

	case 0x09	:			/* tab		*/
	case 0x06	: right();		/* ctrl-F	*/
			  break;

	case 0x02	: left();		/* ctrl-B	*/
			  break;

	case 0x09B	:
	case 0xFFFFFF9B	: if (cursor_key())
				break;
			  else
				continue;

	case '!'	: shell_escape(); continue;

	case 0x0A	:
	case 0x0D	: run_test(); continue;

	default		: break;
      }

	/* By default no redrawing is done. The various cases in the	*/
	/* switch statement should use continue to perform a redraw.	*/
     goto back;
     input = input;
   }
}
/*}}}*/
/*{{{  calculate averages */
/*
** This code is used to determine average performance figures for the
** vector library routines. Unfortunately for the vector sizes used the
** results are very sensitive to interrupts/descheduling, leading to
** bogus results. Hence it is necessary to repeat the tests several
** times, eliminating the worst and best results, and averaging out the
** remainder.
*/
#define	NumberIterations	12
#define NumberToSkip		 3	/* on each end -> six results	*/
static	List	ResultsList;
typedef struct ResultEntry {
	Node		Node;
	char		Name[32];	/* of the test.			*/
	int		NumberResults;
	float		Results[NumberIterations];
} ResultEntry;

typedef struct SearchArg {	/* SearchList only passes one arg	*/
	char		*Name;
	float		 Value;
} SearchArg;

/*{{{  search_fn() */
	/* This routine checks whether the specified test is already	*/
	/* in the list of known tests. If so it inserts the result	*/
	/* value into the table at the appropriate position.		*/
static word search_fn(ResultEntry *node, SearchArg *arg)
{ int	i;
  if (!strcmp(node->Name, arg->Name))
   { for (i = node->NumberResults; i > 0; i--)
      if (arg->Value > node->Results[i-1])
       { node->Results[i] = arg->Value; break; }
      else
       node->Results[i] = node->Results[i-1];
     if (i == 0) node->Results[i] = arg->Value;
     node->NumberResults++;
     return(TRUE);
   }
  else
   return(FALSE);
}
/*}}}*/
/*{{{  output_fn() */
static void output_fn(ResultEntry *node)
{ int	i;
  float	total;

  if (node->NumberResults < (3 * NumberToSkip))
   { fprintf(Output, "Test %12s: only %d results, not sufficient.\n", 
		node->Name, node->NumberResults);
     return;
   }

  for (i = NumberToSkip, total = 0.0F; i < (node->NumberResults - NumberToSkip); i++)
   total += node->Results[i];

  total /= (float)(int)(node->NumberResults - (2 * NumberToSkip));

  fprintf(Output, "Test %12s: time taken %5.1f%%, saving %5.1f%%, speedup %4.1f times\n",
		node->Name, 100.0 - total, total, 100.0 / (100.0 - total));
}
/*}}}*/

static	void calculate_averages(char *logfile)
{ int		i, j;
  int		result;
  char		test_name[32];
  float		saving;
  SearchArg	arg;
  char *	fname = "vectest.log";
  char *	tmp;

  InitList(&ResultsList);
  CompactOutput = TRUE;

  if ((tmp = getenv( "TMPDIR" )) != NULL)
    {
      char * f = malloc( strlen( tmp ) + 1 + strlen( fname ) + 1 );

      
      strcpy( f, tmp );
      strcat( f, "/" );
      strcat( f, fname );
      fname = f;	 
    }
	 
  for (i = 0; i < NumberIterations; i++)
   {
     Output = fopen( fname, "w" );
     if (Output == NULL)
      { fprintf(stderr,"vectest: unable to open temporary logfile %s.\n", fname);
        exit(EXIT_FAILURE);
      }

     for (j = 0; Tests[j].Name != NULL; j++)
      if (Tests[j].Standard)
       do_test(j);

	/* Now read the results in and store them in a list.		*/
     fclose(Output);
     Output	= fopen(fname, "r");
     if (Output == NULL)
      { fprintf(stderr,"vectest: unable to reopen temporary logfile %s.\n", fname);
        exit(EXIT_FAILURE);
      }
     
     forever
      { result = fscanf(Output, "Test %s : %f%%\n", test_name, &saving);
        if (result == EOF) break;
        if (result != 2)
	  {
	    char buffer[ 1024 ];
	    

	    /* skip bad line */
	    
	    fgets( buffer, 1023, Output );

	    continue;
	  }	
	arg.Name 	= test_name;
	arg.Value	= saving;
        unless (SearchList(&ResultsList, (WordFnPtr) &search_fn, &arg))
         {	/* This test is not yet in the list.	*/
           ResultEntry *new_node	= New(ResultEntry);
	   if (new_node == NULL)
            { fputs("vectest: out of memory.\n", stderr);
	      exit(EXIT_FAILURE);
	    }
	   strcpy(new_node->Name, test_name);
	   new_node->NumberResults = 1;
	   new_node->Results[0]    = saving;
	   AddTail(&ResultsList, &(new_node->Node));
         }
      }		/* reading results file		*/
     fclose(Output);
   }		/* for the number of iterations	*/

  unlink( fname );	/* Remove temporary file	*/

  if (logfile != NULL)
   { Output = fopen(logfile, "w");
     if (Output == NULL)
      { fprintf(stderr, "vectest: cannot open output file %s\n", logfile);
        exit(EXIT_FAILURE);
      }
   }
  else
   Output = stdout;

  fprintf(Output, "\t\tVector library test suite Version %s\n\n", VersionNumber);
  fprintf(Output, "Results are average for %d iterations, discarding the %d best and worst results.\n",
		NumberIterations, NumberToSkip);
  fprintf(Output, "The three numbers output are as followed:\n");
  fprintf(Output, "  1) time taken by the vector library compared with equivalent C\n");
  fprintf(Output, "  2) saving achieved by using the vector library\n");
  fprintf(Output, "  3) speedup achieved by using the vector library\n\n");
  (void) WalkList(&ResultsList, (WordFnPtr) &output_fn);
}	
/*}}}*/
/*{{{  main */
/*
** The vectest program can be run with no arguments or with a single
** argument -m. In the first case it simply walks down the table of
** tests and executes them all. In the second case it call the menu
** function.
*/
/*{{{  usage() */
static void usage(void)
{ fputs("vectest: usage, vectest [-c] [-m] [-l logfile]\n", stderr);
  fputs("       : -c    compact output for automatic processing\n", stderr);
  fputs("       : -m    use interactive menu\n", stderr);
  fputs("       : -l    send output to specified logfile.\n", stderr);
  fputs("       : -a    run repeatedly and calculate averages.\n", stderr);
  exit(EXIT_FAILURE);
}
/*}}}*/

int main(int argc, char **argv)
{ int	i;
  bool	use_menu	= FALSE;
  bool  averages	= FALSE;
  char	*logfile	= NULL;

  init_vectors();

#if FloatToOnChip
  AccelerateCode(&VfAdd);
#endif
#if DoubleToOnChip
  AccelerateCode(&VdAdd);
#endif

  for (i = 1; i < argc; i++)
   if (argv[i][0] == '-')
    switch(argv[i][1])
     { case 'c'	: CompactOutput = TRUE; break;
       case 'm' : use_menu	= TRUE; break;
       case 'l' : if (argv[i][2] != '\0')
		   logfile = &(argv[i][2]);
		  elif (i == (argc - 1))
		   usage();
		  else
		   logfile = argv[++i];
		  break;
       case 'a' : averages = TRUE; break;

       default  : usage();
     }
    else
     usage();

  if (use_menu && (logfile != NULL))
   { fputs("vectest: cannot combine logging and menu.\n", stderr);
     exit(EXIT_FAILURE);
   }

  if (averages)
   { calculate_averages(logfile); return(EXIT_SUCCESS); }

  if (logfile != NULL)
   if ((Output = fopen(logfile, "w")) == NULL)
    { fprintf(stderr, "vectest: cannot open logfile %s\n", logfile);
      exit(EXIT_FAILURE);
    }

  if (use_menu)
   menu();
  else
   { fprintf(Output, "Vector library test suite : version %s\n", VersionNumber);
     for (i = 0; Tests[i].Name != NULL; i++)
      if (Tests[i].Standard)
       do_test(i);
   }
  return(EXIT_SUCCESS); 
}
/*}}}*/
