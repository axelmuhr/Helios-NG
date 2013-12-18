/*------------------------------------------------------------------------
--                                                                      --
--               H E L I O S   V E C T O R   L I B R A R Y              --
--               -----------------------------------------              --
--                                                                      --
--             Copyright (C) 1992, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- vlibf.c								--
--	  Generic C versions of the float vector library routines.	--
-- Most if not all of this code should be compiled out and replaced	--
-- by assembler versions.						--
--                                                                      --
--	Author:  BLV 11/10/92						--
--                                                                      --
------------------------------------------------------------------------*/
/* SccsId: %W%	%G% Copyright (C) 1987, Perihelion Software Ltd.	*/
/* $Id: vlibf.c,v 1.1 1992/10/14 14:27:51 bart Exp $ */

#include <vectlib.h>

#define abs(a) ((a < 0) ? -a : a)

void VfAdd(int n, float *x, float *y)
{
  while (n--)
   *x++ += *y++;
}

void VfSub(int n, float *x, float *y)
{
  while (n--)
   *x++ -= *y++;
}
 
void VfMul(int n, float *x, float *y)
{
  while (n--)
   *x++ *= *y++;  
}

void VfDiv(int n, float *x, float *y)
{
  while (n--)
   *x++ /= *y++;
}

void VfsAdd(int n, float *x, int x_stride, float *y, int y_stride)
{
  while (n--)
   { *x += *y; x += x_stride; y += y_stride; }
}

void VfsSub(int n, float *x, int x_stride, float *y, int y_stride)
{
  while (n--)
   { *x -= *y; x += x_stride; y += y_stride; }
}

void VfsMul(int n, float *x, int x_stride, float *y, int y_stride)
{
  while (n--)
   { *x *= *y; x += x_stride; y += y_stride; }
}

void VfsDiv(int n, float *x, int x_stride, float *y, int y_stride)
{
  while (n--)
   { *x /= *y; x += x_stride; y += y_stride; }
}

void VfAddScalar(float value, int n, float *x)
{
  while (n--)
   *x++ += value;
}

void VfSubScalar(float value, int n, float *x)
{
  while (n--)
   *x++ -= value;
}

void VfMulScalar(float value, int n, float *x)
{
  while (n--)
   *x++ *= value;
}

void VfDivScalar(float value, int n, float *x)
{
  while (n--)
   *x++ /= value;
}

void VfRecScalar(float value, int n, float *x)
{
  while (n--)
   { *x = value / *x; x++; }
}

void VfsAddScalar(float value, int n, float *x, int stride)
{
  while (n--)
   { *x += value; x += stride; }
}

void VfsSubScalar(float value, int n, float *x, int stride)
{
  while (n--)
   { *x -= value; x += stride; }
}

void VfsMulScalar(float value, int n, float *x, int stride)
{
  while (n--)
   { *x *= value; x += stride; }
}

void VfsDivScalar(float value, int n, float *x, int stride)
{
  while (n--)
   { *x /= value; x += stride; }
}
 
void VfsRecScalar(float value, int n, float *x, int stride)
{
  while (n--)
   { *x = value / *x; x += stride; }
}

void VfMulAdd(float value, int n, float *x, float *y)
{
  while (n--)
   *x++ += (value * *y++);
}

void VfsMulAdd(float value, int n, float *x, int x_stride, float *y, int y_stride)
{
  while (n--)
   { *x += (value * *y); x += x_stride; y += y_stride; }
}

void VfCopy(int n, float *x, float *y)
{
  while (n--)
   *x++ = *y++;
}

void VfsCopy(int n, float *x, int x_stride, float *y, int y_stride)
{
  while (n--)
   { *x = *y; x += x_stride; y += y_stride; }
}

void VfFill(float value, int n, float *x)
{
  while (n--)
   *x++ = value;
}

void VfsFill(float value, int n, float *x, int stride)
{
  while (n--)
   { *x = value; x += stride; }
}

int  VfMax(int n, float *x)
{ int	index = 0;
  float max   = *x;
  int	i;

  for (i = 1; i < n; i++)
   if (x[i] > max)
    { index = i; max = x[i]; }

  return(index);
}

int  VfsMax(int n, float *x, int stride)
{ int	index = 0;
  float	max   = *x;
  int	i;

  for (i = 1, x += stride; i < n; i++, x += stride)
   if (*x > max)
    { index = i; max = *x; }

  return(index);
}

int  VfMin(int n, float *x)
{ int	index	= 0;
  float min	= *x;
  int	i;

  for (i = 1; i < n; i++)
   if (x[i] < min)
    { index = i; min = x[i]; }

  return(index);
}

int  VfsMin(int n, float *x, int stride)
{ int	index	= 0;
  float	min	= *x;
  int	i;

  for (i = 1, x += stride; i < n; i++, x += stride)
   if (*x < min)
    { index = i; min = x[i]; }

  return(index);
}

int  VfAmax(int n, float *x)
{ int	index	= 0;
  float	max	= abs(*x);
  int	i;

  for (i = 1; i < n; i++)
   if (abs(x[i]) > max)
    { index = i; max = abs(x[i]); }

  return(index);
}
 
int  VfsAmax(int n, float *x, int stride)
{ int	index	= 0;
  float	max	= abs(*x);
  int	i;

  for (i = 1, x += stride; i < n; i++, x += stride)
   if (abs(*x) > max)
    { index = i; max = abs(*x); }
  return(index);
}

int  VfAmin(int n, float *x)
{ int	index	= 0;
  float	min	= abs(*x);
  int	i;

  for (i = 1; i < n; i++)
   if (abs(x[i]) < min)
    { index = i; min = abs(x[i]); }

  return(index);
}

int  VfsAmin(int n, float *x, int stride)
{ int	index	= 0;
  float	min	= abs(*x);
  int	i;

  for (i = 1, x += stride; i < n; i++, x += stride)
   if (abs(*x) < min)
    { index = i; min = abs(*x); }

  return(index);
}

float  VfDot(int n, float *x, float *y)
{ float	result = 0.0;
  
  while (n--)
   result += (*x++ * *y++);

  return(result);
}

float  VfsDot(int n, float *x, int x_stride, float *y, int y_stride)
{ float	result = 0.0;

  while (n--)
   { result += (*x * *y); x += x_stride; y += y_stride; }

  return(result);
}

float VfSum(int n, float *x)
{ float	result = 0.0;

  while (n--)
   result += *x++;
 
  return(result);
}

float VfsSum(int n, float *x, int stride)
{ float	result = 0.0;

  while (n--)
   { result += *x; x += stride; }

  return(result);
}

float VfProd(int n, float *x)
{ float	result = 1.0;

  while (n--)
   result *= *x++;

  return(result);
}

float VfsProd(int n, float *x, int stride)
{ float	result = 1.0;

  while (n--)
   { result *= *x; x += stride; }
}

