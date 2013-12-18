/*------------------------------------------------------------------------
--                                                                      --
--               H E L I O S   V E C T O R   L I B R A R Y              --
--               -----------------------------------------              --
--                                                                      --
--             Copyright (C) 1992, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- vectlib.h								--
--                                                                      --
--	Author:  BLV 11/10/92						--
--                                                                      --
------------------------------------------------------------------------*/
/* $Id: vectlib.h,v 1.5 1992/10/18 14:17:09 bart Exp $ */

extern void	VfAdd( int n, float * x, float * y );
extern void	VfSub( int n, float * x, float * y );
extern void	VfMul( int n, float * x, float * y );
extern void	VfDiv( int n, float * x, float * y );

extern void	VdAdd( int n, double * x, double * y );
extern void	VdSub( int n, double * x, double * y );
extern void	VdMul( int n, double * x, double * y );
extern void	VdDiv( int n, double * x, double * y );

extern void	VfsAdd( int n, float * x, int x_stride, float * y, int y_stride );
extern void	VfsSub( int n, float * x, int x_stride, float * y, int y_stride );
extern void	VfsMul( int n, float * x, int x_stride, float * y, int y_stride );
extern void	VfsDiv( int n, float * x, int x_stride, float * y, int y_stride );

extern void	VdsAdd( int n, double * x, int x_stride, double * y, int y_stride );
extern void	VdsSub( int n, double * x, int x_stride, double * y, int y_stride );
extern void	VdsMul( int n, double * x, int x_stride, double * y, int y_stride );
extern void	VdsDiv( int n, double * x, int x_stride, double * y, int y_stride );

extern void	VfAddScalar( float  value, int n, float * x );
extern void	VfSubScalar( float  value, int n, float * x );
extern void	VfMulScalar( float  value, int n, float * x );
extern void	VfDivScalar( float  value, int n, float * x );
extern void	VfRecScalar( float  value, int n, float * x );

extern void	VdAddScalar( double value, int n, double * x );
extern void	VdSubScalar( double value, int n, double * x );
extern void	VdMulScalar( double value, int n, double * x );
extern void	VdDivScalar( double value, int n, double * x );
extern void	VdRecScalar( double value, int n, double * x );

extern void	VfsAddScalar( float  value, int n, float * x, int stride );
extern void	VfsSubScalar( float  value, int n, float * x, int stride );
extern void	VfsMulScalar( float  value, int n, float * x, int stride );
extern void	VfsDivScalar( float  value, int n, float * x, int stride );
extern void	VfsRecScalar( float  value, int n, float * x, int stride );

extern void	VdsAddScalar( double value, int n, double * x, int stride );
extern void	VdsSubScalar( double value, int n, double * x, int stride );
extern void	VdsMulScalar( double value, int n, double * x, int stride );
extern void	VdsDivScalar( double value, int n, double * x, int stride );
extern void	VdsRecScalar( double value, int n, double * x, int stride );

extern void	VfMulAdd(  float  value, int n, float *  x, float * y );
extern void	VfsMulAdd( float  value, int n, float *  x, int x_stride, float *  y, int y_stride );
extern void	VdMulAdd(  double value, int n, double * x, double * y );
extern void	VdsMulAdd( double value, int n, double * x, int x_stride, double * y, int y_stride );

extern void	VfCopy(  int n, float *  x, float *  y );
extern void	VdCopy(  int n, double * x, double * y );
extern void	VfsCopy( int n, float *  x, int x_stride, float *  y, int y_stride );
extern void	VdsCopy( int n, double * x, int x_stride, double * y, int y_stride );

extern void	VfFill(  float  value, int n, float *  x );
extern void	VdFill(  double value, int n, double * x );
extern void	VfsFill( float  value, int n, float *  x, int stride );
extern void	VdsFill( double value, int n, double * x, int stride );

extern int	VfMax(   int n, float *  x );
extern int	VdMax(   int n, double * x );
extern int	VfsMax(  int n, float *  x, int stride );
extern int	VdsMax(  int n, double * x, int stride );

extern int	VfMin(   int n, float *  x );
extern int	VdMin(   int n, double * x );
extern int	VfsMin(  int n, float *  x, int stride );
extern int	VdsMin(  int n, double * x, int stride );

extern int	VfAmax(  int n, float *  x );
extern int	VdAmax(  int n, double * x );
extern int	VfsAmax( int n, float *  x, int stride );
extern int	VdsAmax( int n, double * x, int stride );

extern int	VfAmin(  int n, float *  x );
extern int	VdAmin(  int n, double * x );
extern int	VfsAmin( int n, float *  x, int stride );
extern int	VdsAmin( int n, double * x, int stride );

extern float	VfDot(   int n, float *  x, float *  y );
extern double	VdDot(   int n, double * x, double * y );
extern float	VfsDot(  int n, float *  x, int x_stride, float *  y, int y_stride );
extern double	VdsDot(  int n, double * x, int x_stride, double * y, int y_stride );

extern float	VfSum(   int n, float *  x );
extern double	VdSum(   int n, double * x );
extern float	VfsSum(  int n, float *  x, int stride );
extern double	VdsSum(  int n, double * x, int stride );

extern float	VfProd(  int n, float *  x );
extern double	VdProd(  int n, double * x );
extern float	VfsProd( int n, float *  x, int stride );
extern double	VdsProd( int n, double * x, int stride );

