#include <stdio.h>
#include <vectlib.h>
#include <nonansi.h>
#include <math.h>

#define VEC_LIB_SIZE	10 /* 100000 */	/* NB/ value must be > 1 */
#define INIT1		1.0
#define INIT2		3.0
#define GUARD		2
#define STRIDE1		3
#define STRIDE2		4

float	af[ VEC_LIB_SIZE + GUARD * 2 ];
float	bf[ VEC_LIB_SIZE + GUARD * 2 ];
float	cf[ VEC_LIB_SIZE + GUARD * 2 ][ STRIDE1 ];
float	df[ VEC_LIB_SIZE + GUARD * 2 ][ STRIDE2 ];

double	ad[ VEC_LIB_SIZE + GUARD * 2 ];
double	bd[ VEC_LIB_SIZE + GUARD * 2 ];
double	cd[ VEC_LIB_SIZE + GUARD * 2 ][ STRIDE1 ];
double	dd[ VEC_LIB_SIZE + GUARD * 2 ][ STRIDE2 ];



#define test_ends( array )	\
if (ok)	\
  {		\
    for (i = 0; i < GUARD; i++)	\
      { \
        int	j = VEC_LIB_SIZE + GUARD + i;	\
      \
	\
        if (a##array[ i ] != INIT1)	\
          {		\
	    printf( "CORRUPTION of a, start - %d = %f, should be %f\n", GUARD - i, a##array[ i ], INIT1 );\
            ok = 0; \
            break;  \
	  } \
	    \
	if (b##array[ i ] != INIT2)	\
	  {		\
	    printf( "CORRUPTION of b, start - %d = %f, should be %f\n", GUARD - i, b##array[ i ], INIT2 );\
	    ok = 0; \
	    break;  \
	  } \
	    \
	if (a##array[ j ] != INIT1)	\
	  {		\
	    printf( "CORRUPTION of a, end + %d = %f, should be %f\n", i + 1, a##array[ j ], INIT1 );	\
	    ok = 0; \
	    break;  \
	  } \
	    \
	if (b##array[ j ] != INIT2)	\
	  {		\
	    printf( "CORRUPTION of b, end + %d = %f, should be %f\n", i + 1, b##array[ j ], INIT2 );	\
	    ok = 0; \
	    break;  \
	  } \
      }   \
  }

#define test_stride_ends( array )	\
if (ok) \
  {	\
    for (j = STRIDE1; j--;) \
      { \
	for (i = 0; i < GUARD; i++)	\
	  { \
	    int		k = VEC_LIB_SIZE + GUARD + i;	\
	      \
	      \
	    if (c##array[ i ][ j ] != INIT1)	\
	      {	\
		printf( "CORRUPTION of %dth column of c, start - %d = %f, should be %f\n", \
		       j, GUARD - i, c##array[ i ][ j ], INIT1 );\
		ok = 0; \
		break;  \
	      } \
		\
	    if (c##array[ k ][ j ] != INIT1)	\
	      {	\
		printf( "CORRUPTION of c, end + %d = %f, should be %f\n", \
		       i + 1, c##array[ k ][ j ], INIT1 );	\
		ok = 0; \
		break;  \
	      } \
	  } \
      } \
	\
    for (j = STRIDE2; j--;) \
      { \
	for (i = 0; i < GUARD; i++)	\
	  { \
	    int		k = VEC_LIB_SIZE + GUARD + i;	\
	      \
	      \
	    if (d##array[ i ][ j ] != INIT2)	\
	      {	\
		printf( "CORRUPTION of d, start - %d = %f, should be %f\n", \
		       GUARD - i, d##array[ i ][ j ], INIT2 );\
		ok = 0; \
		break;  \
	      } \
	    \
	    if (d##array[ k ][ j ] != INIT2)	\
	      {	\
	        printf( "CORRUPTION of d, end + %d = %f, should be %f\n", \
		       i + 1, d##array[ k ][ j ], INIT2 );	\
		ok = 0; \
		break;  \
	      } \
	  } \
      } \
  } \
  

#define	do_void_test( func, result, array, arg1, arg2, arg3 )	\
{	\
  int	i;	\
  int	ok = 1;	\
  long  now; \
  \
  \
  for (i = VEC_LIB_SIZE + GUARD * 2; i--;)	\
    { \
      a##array[ i ] = (float) INIT1;	\
      b##array[ i ] = (float) INIT2;	\
    } \
  \
  printf( "%-12s: ", #func );	\
  fflush( stdout );    \
  \
  now = _cputime();	\
  \
  func ( arg1, arg2, arg3 ); 	\
  \
  now = _cputime() - now;	\
  \
  for (i = VEC_LIB_SIZE; i--;)	\
    { \
      if (a##array[ i + GUARD ] != result) \
	{     \
	  printf( "FAILURE: element %d is %f should be %f\n",	\
	       i, a##array[ i + GUARD ], result );   \
	  ok = 0; \
	  break; \
	} \
	  \
      if (b##array[ i + GUARD ] != INIT2)	\
	{     \
	  printf( "CORRUPTION: element %d should be %f but is %f\n",	\
		 i, INIT2, b##array[ i + GUARD ] ); \
	  ok = 0;   \
	  break;    \
	} \
    }	\
  \
  test_ends( array ) \
  \
  if (ok)	\
    printf( "Passed, Elapsed = %ld\n", now );	\
  \
} /* do_void_test */


#define	do_void_test1( func, result, array, arg1, arg2, arg3, arg4 )	\
{	\
  int	i;	\
  int	ok = 1;	\
  long  now; \
  \
  \
  for (i = VEC_LIB_SIZE + GUARD * 2; i--;)	\
    { \
      a##array[ i ] = (float) INIT1;	\
      b##array[ i ] = (float) INIT2;	\
    } \
  \
  printf( "%-12s: ", #func );	\
  fflush( stdout );    \
  \
  now = _cputime();	\
  \
  func ( arg1, arg2, arg3, arg4 ); 	\
  \
  now = _cputime() - now;	\
  \
  for (i = VEC_LIB_SIZE; i--;)	\
    { \
      if (a##array[ i + GUARD ] != result) \
	{     \
	  printf( "FAILURE: element %d is %f should be %f\n",	\
	       i, a##array[ i + GUARD ], result );	   \
	  ok = 0;   \
	  break;   \
	} \
	  \
      if (b##array[ i + GUARD ] != INIT2)	\
	{     \
	  printf( "CORRUPTION: element %d should be %f but is %f\n",	\
		 i, INIT2, b##array[ i + GUARD ] ); \
	  ok = 0;   \
	  break;    \
	} \
    }	\
  \
  test_ends( array ) \
  \
  if (ok)	\
    printf( "Passed, Elapsed = %ld\n", now );	\
  \
} /* do_void_test1 */


#define	do_res_test( func, result, array, type )	\
{	\
  int	i;	\
  type	res;	\
  int	ok = 1;	\
  long  now; \
  \
  \
  for (i = 0; i < GUARD; i++)	\
    { \
      a##array[ i ] = (float) INIT1;	\
      a##array[ i + VEC_LIB_SIZE + GUARD ] = (float) INIT1;	\
    } \
  \
  for (i = VEC_LIB_SIZE; i--;)	\
    { \
      a##array[ i + GUARD ] = (float)(1.0F - (float)i);	\
    } \
  \
  printf( "%-12s: ", #func ); \
  fflush( stdout );    \
  \
  now = _cputime();	\
  \
  res = func ( VEC_LIB_SIZE, a##array + GUARD ); 	\
  \
  now = _cputime() - now;	\
  \
  for (i = VEC_LIB_SIZE; i--;)	\
    { \
      if (a##array[ i + GUARD ] != 1.0 - (float)i)	\
	{     \
	  printf( "CORRUPTION: element %d of a is %f should be %f\n",	\
		 i, a##array[ i + GUARD ], 1.0 - (float)i );	   \
	  ok = 0;   \
	  break;   \
	} \
    }	\
  \
  test_ends( array )	\
      \
  if (ok && res != result)	\
    printf( "FAILURE: result of computation is wrong\n" ), ok = 0;	\
	\
  if (ok)	\
    printf( "Passed, Elapsed = %ld\n", now );	\
    \
} /* do_res_test */
  

#define	do_void_stride_test( func, result, array, arg1, arg2, arg3, arg4, arg5 )	\
{	\
  int	i;	\
  int	j;	\
  int	ok = 1;	\
  long  now; \
    \
    \
  for (i = VEC_LIB_SIZE + GUARD * 2; i--;)	\
    { \
      for (j = STRIDE1; j--;)	\
	{ \
	  c##array[ i ][ j ] = (float) INIT1;	\
	} \
	  \
      for (j = STRIDE2; j--;)	\
	{ \
	  d##array[ i ][ j ] = (float) INIT2;	\
	} \
    } \
      \
  printf( "%-12s: ", #func );	\
  fflush( stdout );    \
    \
  now = _cputime();	\
    \
  func ( arg1, arg2, arg3, arg4, arg5 ); 	\
    \
  now = _cputime() - now;	\
    \
  for (j = 1; j < STRIDE1; j++) \
    { \
      for (i = VEC_LIB_SIZE; i--;)	\
	{ \
	  if (c##array[ i + GUARD ][ j ] != INIT1)	\
	    {     \
	      printf( "CORRUPTION: element %d is %f should be %f\n",	\
		     i, c##array[ i + GUARD ][ j ], result );	   \
	      ok = 0;   \
	      break;   \
	    } \
	} \
    } \
      \
  for (j = 1; j < STRIDE2; j++) \
    { \
      for (i = VEC_LIB_SIZE; i--;)	\
	{ \
	  if (d##array[ i + GUARD ][ j ] != INIT2)	\
	    {     \
	      printf( "CORRUPTION: element %d should be %f but is %f\n",	\
		     i, INIT2, d##array[ i + GUARD ][ j ] ); \
	      ok = 0;   \
	      break;    \
	    } \
	} \
    } \
      \
  for (i = VEC_LIB_SIZE; i--;)	\
    { \
      if (c##array[ i + GUARD ][ 0 ] != result)	\
	{     \
	  printf( "FAILURE: element %d is %f should be %f\n",	\
		 i, c##array[ i + GUARD ][ 0 ], result );	   \
	  ok = 0; \
	  break;  \
	} \
	  \
      if (d##array[ i + GUARD ][ 0 ] != INIT2)	\
	{     \
	  printf( "CORRUPTION: element %d should be %f but is %f\n",	\
		 i, INIT2, d##array[ i + GUARD ][ 0 ] ); \
	  ok = 0;   \
	  break;    \
	} \
    }	\
      \
  test_stride_ends( array ) \
    \
  if (ok)	\
    printf( "Passed, Elapsed = %ld\n", now );	\
    \
} /* do_void_stride_test */
  

#define	do_void_stride_test1( func, result, array, arg1, arg2, arg3, arg4 )	\
{	\
  int	i;	\
  int	j;	\
  int	ok = 1;	\
  long  now; \
    \
    \
  for (i = VEC_LIB_SIZE + GUARD * 2; i--;)	\
    { \
      for (j = STRIDE1; j--;)	\
	{ \
	  c##array[ i ][ j ] = (float) INIT1;	\
	} \
	  \
      for (j = STRIDE2; j--;)	\
	{ \
	  d##array[ i ][ j ] = (float) INIT2;	\
	} \
    } \
      \
  printf( "%-12s: ", #func );	\
  fflush( stdout );    \
    \
  now = _cputime();	\
    \
  func ( arg1, arg2, arg3, arg4 ); \
    \
  now = _cputime() - now;	\
    \
  for (j = 1; j < STRIDE2; j++) \
    { \
      for (i = VEC_LIB_SIZE; i--;)	\
	{ \
	  if (d##array[ i + GUARD ][ j ] != INIT2)	\
	    {     \
	      printf( "CORRUPTION: element %d should be %f but is %f\n",	\
		     i, INIT2, d##array[ i + GUARD ][ j ] ); \
	      ok = 0;   \
	      break;    \
	    } \
	} \
    } \
      \
  for (j = 1; j < STRIDE1; j++) \
    { \
      for (i = VEC_LIB_SIZE; i--;)	\
	{ \
	  if (c##array[ i + GUARD ][ j ] != INIT1)	\
	    {     \
	      printf( "CORRUPTION: element %d is %f should be %f\n",	\
		     i, c##array[ i + GUARD ][ j ], result );	   \
	      ok = 0;   \
	      break;   \
	    } \
	} \
    } \
      \
  for (i = VEC_LIB_SIZE; i--;)	\
    { \
      if (c##array[ i + GUARD ][ 0 ] != result)	\
	{     \
	  printf( "FAILURE: element %d is %f should be %f\n",	\
		 i, c##array[ i + GUARD ][ 0 ], result );	   \
	  ok = 0; \
	  break; \
	} \
	  \
      if (d##array[ i + GUARD ][ 0 ] != INIT2)	\
	{     \
	  printf( "CORRUPTION: element %d should be %f but is %f\n",	\
		 i, INIT2, d##array[ i + GUARD ][ 0 ] ); \
	  ok = 0;   \
	  break;    \
	} \
    }	\
      \
  test_stride_ends( array ) \
    \
  if (ok)	\
    printf( "Passed, Elapsed = %ld\n", now );	\
    \
} /* do_void_stride_test1 */
  

#define	do_void_stride_test2( func, result, array, arg1, arg2, arg3, arg4, arg5, arg6 )	\
{	\
  int	i;	\
  int	j;	\
  int	ok = 1;	\
  long  now; \
    \
    \
  for (i = VEC_LIB_SIZE + GUARD * 2; i--;)	\
    { \
      for (j = STRIDE1; j--;)	\
	{ \
	  c##array[ i ][ j ] = (float) INIT1;	\
	} \
	  \
      for (j = STRIDE2; j--;)	\
	{ \
	  d##array[ i ][ j ] = (float) INIT2;	\
	} \
    } \
      \
  printf( "%-12s: ", #func );	\
  fflush( stdout );    \
    \
  now = _cputime();	\
    \
  func ( arg1, arg2, arg3, arg4, arg5, arg6 ); \
    \
  now = _cputime() - now;	\
    \
  for (j = 1; j < STRIDE2; j++) \
    { \
      for (i = VEC_LIB_SIZE; i--;)	\
	{ \
	  if (d##array[ i + GUARD ][ j ] != INIT2)	\
	    {     \
	      printf( "CORRUPTION: element %d should be %f but is %f\n",	\
		     i, INIT2, d##array[ i + GUARD ][ j ] ); \
	      ok = 0;   \
	      break;    \
	    } \
	} \
    } \
      \
  for (j = 1; j < STRIDE1; j++) \
    { \
      for (i = VEC_LIB_SIZE; i--;)	\
	{ \
	  if (c##array[ i + GUARD ][ j ] != INIT1)	\
	    {     \
	      printf( "CORRUPTION: element %d is %f should be %f\n",	\
		     i, c##array[ i + GUARD ][ j ], result );	   \
	      ok = 0;   \
	      break;   \
	    } \
	} \
    } \
      \
  for (i = VEC_LIB_SIZE; i--;)	\
    { \
      if (c##array[ i + GUARD ][ 0 ] != result)	\
	{     \
	  printf( "FAILURE: element %d is %f should be %f\n",	\
		 i, c##array[ i + GUARD ][ 0 ], result );	   \
	  ok = 0; \
	  break; \
	} \
	  \
      if (d##array[ i + GUARD ][ 0 ] != INIT2)	\
	{     \
	  printf( "CORRUPTION: element %d should be %f but is %f\n",	\
		 i, INIT2, d##array[ i + GUARD ][ 0 ] ); \
	  ok = 0;   \
	  break;    \
	} \
    }	\
      \
  test_stride_ends( array ) \
    \
  if (ok)	\
    printf( "Passed, Elapsed = %ld\n", now );	\
    \
} /* do_void_stride_test2 */
  

#define	do_res_stride_test( func, result, array, type )	\
{	\
  int	i;	\
  int	j;	\
  type	res;	\
  int	ok = 1;	\
  long  now; \
  \
  \
  for (i = 0; i < GUARD; i++)	\
    { \
      for (j = STRIDE1; j--;) \
	{ \
	  c##array[ i ][ j ] = (float) INIT1;	\
	  c##array[ i + VEC_LIB_SIZE + GUARD ][ j ] = (float) INIT1; \
	} \
    } \
  \
  for (i = VEC_LIB_SIZE; i--;)	\
    { \
      for (j = STRIDE1; j--;) \
	{ \
	  c##array[ i + GUARD ][ j ] = (float)(1.0F - (float)i); \
	} \
    } \
  \
  printf( "%-12s: ", #func ); \
  fflush( stdout );    \
  \
  now = _cputime();	\
  \
  res = func ( VEC_LIB_SIZE, c##array [ GUARD ], STRIDE1 ); \
  \
  now = _cputime() - now;	\
  \
  for (j = STRIDE1; j--;) \
    { \
      for (i = VEC_LIB_SIZE; i--;)	\
	{ \
          if (c##array[ i + GUARD ][ j ] != 1.0 - (float)i)	\
	    {     \
	      printf( "CORRUPTION: element %d, %d of a is %f should be %f\n",	\
		     i, j, c##array[ i + GUARD ][ j ], 1.0 - (float)i );	   \
	      ok = 0;   \
	      break;   \
	    } \
        } \
    } \
  \
  test_stride_ends( array ) \
      \
  if (ok && res != result) \
    printf( "FAILURE: result of computation is wrong\n" ), ok = 0; \
  \
  if (ok)	\
    printf( "Passed, Elapsed = %ld\n", now );	\
    \
} /* do_res_stride_test */
  

static void
void_tests( void )
{
  do_void_test( VfAdd,       INIT1 + INIT2, f, VEC_LIB_SIZE, af + GUARD, bf + GUARD );
  do_void_test( VfSub,       INIT1 - INIT2, f, VEC_LIB_SIZE, af + GUARD, bf + GUARD );
  do_void_test( VfMul,       INIT1 * INIT2, f, VEC_LIB_SIZE, af + GUARD, bf + GUARD );
  do_void_test( VfDiv,       (float)(INIT1 / INIT2), f, VEC_LIB_SIZE, af + GUARD, bf + GUARD );

  do_void_test( VdAdd,       INIT1 + INIT2, d, VEC_LIB_SIZE, ad + GUARD, bd + GUARD );
  do_void_test( VdSub,       INIT1 - INIT2, d, VEC_LIB_SIZE, ad + GUARD, bd + GUARD );
  do_void_test( VdMul,       INIT1 * INIT2, d, VEC_LIB_SIZE, ad + GUARD, bd + GUARD );
  do_void_test( VdDiv,       INIT1 / INIT2, d, VEC_LIB_SIZE, ad + GUARD, bd + GUARD );
  
  do_void_test( VfAddScalar, INIT1 + INIT2, f, INIT2, VEC_LIB_SIZE, af + GUARD );
  do_void_test( VfSubScalar, INIT1 - INIT2, f, INIT2, VEC_LIB_SIZE, af + GUARD );
  do_void_test( VfMulScalar, INIT1 * INIT2, f, INIT2, VEC_LIB_SIZE, af + GUARD );
  do_void_test( VfDivScalar, (float)(INIT1 / INIT2), f, INIT2, VEC_LIB_SIZE, af + GUARD );
  do_void_test( VfRecScalar, (float)(INIT2 / INIT1), f, INIT2, VEC_LIB_SIZE, af + GUARD );
  
  do_void_test( VdAddScalar, INIT1 + INIT2, d, INIT2, VEC_LIB_SIZE, ad + GUARD );
  do_void_test( VdSubScalar, INIT1 - INIT2, d, INIT2, VEC_LIB_SIZE, ad + GUARD );
  do_void_test( VdMulScalar, INIT1 * INIT2, d, INIT2, VEC_LIB_SIZE, ad + GUARD );
  do_void_test( VdDivScalar, INIT1 / INIT2, d, INIT2, VEC_LIB_SIZE, ad + GUARD );
  do_void_test( VdRecScalar, INIT2 / INIT1, d, INIT2, VEC_LIB_SIZE, ad + GUARD );

  do_void_test( VfCopy,      INIT2,         f, VEC_LIB_SIZE, af + GUARD, bf + GUARD );
  do_void_test( VdCopy,      INIT2,         d, VEC_LIB_SIZE, ad + GUARD, bd + GUARD );
  
  do_void_test( VfFill,      INIT2,         f, INIT2, VEC_LIB_SIZE, af + GUARD );
  do_void_test( VdFill,      INIT2,         d, INIT2, VEC_LIB_SIZE, ad + GUARD );

  do_void_test1( VfMulAdd,   INIT1 + INIT2 * INIT2, f, INIT2, VEC_LIB_SIZE, af + GUARD, bf + GUARD );
  do_void_test1( VdMulAdd,   INIT1 + INIT2 * INIT2, d, INIT2, VEC_LIB_SIZE, ad + GUARD, bd + GUARD );
  
  return;
  
} /* void_tests */


static void
res_tests( void )
{
  do_res_test( VfMax,  0,                f, unsigned long int );
  do_res_test( VdMax,  0,                d, unsigned long int );
  do_res_test( VfMin,  VEC_LIB_SIZE - 1, f, unsigned long int );
  do_res_test( VdMin,  VEC_LIB_SIZE - 1, d, unsigned long int );
  
  do_res_test( VfAmax, VEC_LIB_SIZE - 1, f, unsigned long int );
  do_res_test( VdAmax, VEC_LIB_SIZE - 1, d, unsigned long int );
  do_res_test( VfAmin, 1,                f, unsigned long int );
  do_res_test( VdAmin, 1,                d, unsigned long int );
  
  do_res_test( VfProd, 0.0,              f, float );
  do_res_test( VdProd, 0.0,              d, double );
  do_res_test( VfSum,  -( (VEC_LIB_SIZE - 1) * (VEC_LIB_SIZE - 2) / 2 - 1), f, float );
  do_res_test( VdSum,  -( (VEC_LIB_SIZE - 1) * (VEC_LIB_SIZE - 2) / 2 - 1), d, double );

  return;
  
} /* res_tests */  


static void
double_tests( void )
{
  float		rf;
  double	rd;

  
  do_void_test( rf = VfDot, INIT1, f, VEC_LIB_SIZE, af + GUARD, bf + GUARD );

  if (rf != INIT1 * INIT2 * VEC_LIB_SIZE)
    printf( "Computation error, result is %f, should be %f\n", rf, INIT1 * INIT2 * VEC_LIB_SIZE );
  
  do_void_test( rd = VdDot, INIT1, d, VEC_LIB_SIZE, ad + GUARD, bd + GUARD );

  if (rd != INIT1 * INIT2 * VEC_LIB_SIZE)
    printf( "Computation error, result is %f, should be %f\n", rd, INIT1 * INIT2 * VEC_LIB_SIZE );

  do_void_stride_test( rf = VfsDot, INIT1, f, VEC_LIB_SIZE, cf[ GUARD ], STRIDE1, df[ GUARD ], STRIDE2 );

  if (rf != INIT1 * INIT2 * VEC_LIB_SIZE)
    printf( "Computation error, result is %f, should be %f\n", rf, INIT1 * INIT2 * VEC_LIB_SIZE );
  
  do_void_stride_test( rd = VdsDot, INIT1, d, VEC_LIB_SIZE, cd[ GUARD ], STRIDE1, dd[ GUARD ], STRIDE2 );

  if (rd != INIT1 * INIT2 * VEC_LIB_SIZE)
    printf( "Computation error, result is %f, should be %f\n", rd, INIT1 * INIT2 * VEC_LIB_SIZE );

  return;
  
} /* double_tests */


static void
void_stride_tests( void )
{
  do_void_stride_test( VfsAdd, INIT1 + INIT2, f,
		      VEC_LIB_SIZE, cf[ GUARD ], STRIDE1, df[ GUARD ], STRIDE2 );
  do_void_stride_test( VfsSub, INIT1 - INIT2, f,
		      VEC_LIB_SIZE, cf[ GUARD ], STRIDE1, df[ GUARD ], STRIDE2 );
  do_void_stride_test( VfsMul, INIT1 * INIT2, f,
		      VEC_LIB_SIZE, cf[ GUARD ], STRIDE1, df[ GUARD ], STRIDE2 );
  do_void_stride_test( VfsDiv, (float)(INIT1 / INIT2), f,
		      VEC_LIB_SIZE, cf[ GUARD ], STRIDE1, df[ GUARD ], STRIDE2 );

  do_void_stride_test( VdsAdd, INIT1 + INIT2, d,
		      VEC_LIB_SIZE, cd[ GUARD ], STRIDE1, dd[ GUARD ], STRIDE2 );
  do_void_stride_test( VdsSub, INIT1 - INIT2, d,
		      VEC_LIB_SIZE, cd[ GUARD ], STRIDE1, dd[ GUARD ], STRIDE2 );
  do_void_stride_test( VdsMul, INIT1 * INIT2, d,
		      VEC_LIB_SIZE, cd[ GUARD ], STRIDE1, dd[ GUARD ], STRIDE2 );  
  do_void_stride_test( VdsDiv, INIT1 / INIT2, d,
		      VEC_LIB_SIZE, cd[ GUARD ], STRIDE1, dd[ GUARD ], STRIDE2 );
  
  do_void_stride_test1( VfsAddScalar, INIT1 + INIT2, f,
		       INIT2, VEC_LIB_SIZE, cf[ GUARD ], STRIDE1 );
  do_void_stride_test1( VfsSubScalar, INIT1 - INIT2, f,
		       INIT2, VEC_LIB_SIZE, cf[ GUARD ], STRIDE1 );
  do_void_stride_test1( VfsMulScalar, INIT1 * INIT2, f,
		       INIT2, VEC_LIB_SIZE, cf[ GUARD ], STRIDE1 );
  do_void_stride_test1( VfsDivScalar, (float)(INIT1 / INIT2), f,
		       INIT2, VEC_LIB_SIZE, cf[ GUARD ], STRIDE1 );
  do_void_stride_test1( VfsRecScalar, (float)(INIT2 / INIT1), f,
		       INIT2, VEC_LIB_SIZE, cf[ GUARD ], STRIDE1 );

  do_void_stride_test1( VdsAddScalar, INIT1 + INIT2, d,
		       INIT2, VEC_LIB_SIZE, cd[ GUARD ], STRIDE1 );
  do_void_stride_test1( VdsSubScalar, INIT1 - INIT2, d,
		       INIT2, VEC_LIB_SIZE, cd[ GUARD ], STRIDE1 );
  do_void_stride_test1( VdsMulScalar, INIT1 * INIT2, d,
		       INIT2, VEC_LIB_SIZE, cd[ GUARD ], STRIDE1 );
  do_void_stride_test1( VdsDivScalar, INIT1 / INIT2, d,
		       INIT2, VEC_LIB_SIZE, cd[ GUARD ], STRIDE1 );
  do_void_stride_test1( VdsRecScalar, INIT2 / INIT1, d,
		       INIT2, VEC_LIB_SIZE, cd[ GUARD ], STRIDE1 );

  do_void_stride_test(  VfsCopy,      INIT2,         f,
		      VEC_LIB_SIZE, cf[ GUARD ], STRIDE1, df[ GUARD ], STRIDE2 );
  do_void_stride_test(  VdsCopy,      INIT2,         d,
		      VEC_LIB_SIZE, cd[ GUARD ], STRIDE1, dd[ GUARD ], STRIDE2 );
  
  do_void_stride_test1( VfsFill,      INIT2,         f,
		      INIT2, VEC_LIB_SIZE, cf[ GUARD ], STRIDE1 );
  do_void_stride_test1( VdsFill,      INIT2,         d,
		      INIT2, VEC_LIB_SIZE, cd[ GUARD ], STRIDE1 );
  
  do_void_stride_test2( VfsMulAdd,    INIT1 + INIT2 * INIT2, f,
		       INIT2, VEC_LIB_SIZE, cf[ GUARD ], STRIDE1, df[ GUARD ], STRIDE2 );
  do_void_stride_test2( VdsMulAdd,    INIT1 + INIT2 * INIT2, d,
		       INIT2, VEC_LIB_SIZE, cd[ GUARD ], STRIDE1, dd[ GUARD ], STRIDE2 );
  
  return;

} /* void_stride_tests */


static void
res_stride_tests( void )
{
  do_res_stride_test( VfsMax,  0,                f, unsigned long int );
  do_res_stride_test( VdsMax,  0,                d, unsigned long int );
  do_res_stride_test( VfsMin,  VEC_LIB_SIZE - 1, f, unsigned long int );
  do_res_stride_test( VdsMin,  VEC_LIB_SIZE - 1, d, unsigned long int );
  
  do_res_stride_test( VfsAmax, VEC_LIB_SIZE - 1, f, unsigned long int );
  do_res_stride_test( VdsAmax, VEC_LIB_SIZE - 1, d, unsigned long int );
  do_res_stride_test( VfsAmin, 1,                f, unsigned long int );
  do_res_stride_test( VdsAmin, 1,                d, unsigned long int );
  
  do_res_stride_test( VfsSum,  -( (VEC_LIB_SIZE - 1) * (VEC_LIB_SIZE - 2) / 2 - 1), f, float );
  do_res_stride_test( VdsSum,  -( (VEC_LIB_SIZE - 1) * (VEC_LIB_SIZE - 2) / 2 - 1), d, double );
  do_res_stride_test( VfsProd, 0.0,              f, float );
  do_res_stride_test( VdsProd, 0.0,              d, double );
  
  return;
  
} /* res_stride_tests */  


int
main( void )
{
  printf( "vector library test starting\n" );

  void_tests();

  res_tests();

  double_tests();

  void_stride_tests();

  res_stride_tests();
  
  printf( "vector library test finished\n" );
  
  return 0;

} /* main */
