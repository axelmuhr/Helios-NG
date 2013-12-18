/*------------------------------------------------------------------------
--                                                                      --
--               H E L I O S   V E C T O R   L I B R A R Y              --
--               -----------------------------------------              --
--                                                                      --
--             Copyright (C) 1992, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- vlibd.c								--
--	  Generic C versions of the double vector library routines.	--
-- Most if not all of this code should be compiled out and replaced	--
-- by assembler versions.						--
--                                                                      --
--	Author:  BLV 11/10/92						--
--                                                                      --
------------------------------------------------------------------------*/
/* SccsId: %W%	%G% Copyright (C) 1987, Perihelion Software Ltd.	*/
/* $Id: vlibd.c,v 1.1 1992/10/14 14:27:32 bart Exp $ */

#include <vectlib.h>

#define abs(a) ((a < 0) ? -a : a)

void VdAdd(int n, double *x, double *y)
{
  while (n--)
   *x++ += *y++;
}

void VdSub(int n, double *x, double *y)
{
  while (n--)
   *x++ -= *y++;
}
 
void VdMul(int n, double *x, double *y)
{
  while (n--)
   *x++ *= *y++;  
}

void VdDiv(int n, double *x, double *y)
{
  while (n--)
   *x++ /= *y++;
}

void VdsAdd(int n, double *x, int x_stride, double *y, int y_stride)
{
  while (n--)
   { *x += *y; x += x_stride; y += y_stride; }
}

void VdsSub(int n, double *x, int x_stride, double *y, int y_stride)
{
  while (n--)
   { *x -= *y; x += x_stride; y += y_stride; }
}

void VdsMul(int n, double *x, int x_stride, double *y, int y_stride)
{
  while (n--)
   { *x *= *y; x += x_stride; y += y_stride; }
}

void VdsDiv(int n, double *x, int x_stride, double *y, int y_stride)
{
  while (n--)
   { *x /= *y; x += x_stride; y += y_stride; }
}

void VdAddScalar(double value, int n, double *x)
{
  while (n--)
   *x++ += value;
}

void VdSubScalar(double value, int n, double *x)
{
  while (n--)
   *x++ -= value;
}

void VdMulScalar(double value, int n, double *x)
{
  while (n--)
   *x++ *= value;
}

void VdDivScalar(double value, int n, double *x)
{
  while (n--)
   *x++ /= value;
}

void VdRecScalar(double value, int n, double *x)
{
  while (n--)
   { *x = value / *x; x++; }
}

void VdsAddScalar(double value, int n, double *x, int stride)
{
  while (n--)
   { *x += value; x += stride; }
}

void VdsSubScalar(double value, int n, double *x, int stride)
{
  while (n--)
   { *x -= value; x += stride; }
}

void VdsMulScalar(double value, int n, double *x, int stride)
{
  while (n--)
   { *x *= value; x += stride; }
}

void VdsDivScalar(double value, int n, double *x, int stride)
{
  while (n--)
   { *x /= value; x += stride; }
}
 
void VdsRecScalar(double value, int n, double *x, int stride)
{
  while (n--)
   { *x = value / *x; x += stride; }
}

void VdMulAdd(double value, int n, double *x, double *y)
{
  while (n--)
   *x++ += (value * *y++);
}

void VdsMulAdd(double value, int n, double *x, int x_stride, double *y, int y_stride)
{
  while (n--)
   { *x += (value * *y); x += x_stride; y += y_stride; }
}

void VdCopy(int n, double *x, double *y)
{
  while (n--)
   *x++ = *y++;
}

void VdsCopy(int n, double *x, int x_stride, double *y, int y_stride)
{
  while (n--)
   { *x = *y; x += x_stride; y += y_stride; }
}

void VdFill(double value, int n, double *x)
{
  while (n--)
   *x++ = value;
}

void VdsFill(double value, int n, double *x, int stride)
{
  while (n--)
   { *x = value; x += stride; }
}

int  VdMax(int n, double *x)
{ int	index = 0;
  double max   = *x;
  int	i;

  for (i = 1; i < n; i++)
   if (x[i] > max)
    { index = i; max = x[i]; }

  return(index);
}

int  VdsMax(int n, double *x, int stride)
{ int	index = 0;
  double	max   = *x;
  int	i;

  for (i = 1, x += stride; i < n; i++, x += stride)
   if (*x > max)
    { index = i; max = *x; }

  return(index);
}

int  VdMin(int n, double *x)
{ int	index	= 0;
  double min	= *x;
  int	i;

  for (i = 1; i < n; i++)
   if (x[i] < min)
    { index = i; min = x[i]; }

  return(index);
}

int  VdsMin(int n, double *x, int stride)
{ int	index	= 0;
  double	min	= *x;
  int	i;

  for (i = 1, x += stride; i < n; i++, x += stride)
   if (*x < min)
    { index = i; min = x[i]; }

  return(index);
}

int  VdAmax(int n, double *x)
{ int	index	= 0;
  double	max	= abs(*x);
  int	i;

  for (i = 1; i < n; i++)
   if (abs(x[i]) > max)
    { index = i; max = abs(x[i]); }

  return(index);
}
 
int  VdsAmax(int n, double *x, int stride)
{ int	index	= 0;
  double	max	= abs(*x);
  int	i;

  for (i = 1, x += stride; i < n; i++, x += stride)
   if (abs(*x) > max)
    { index = i; max = abs(*x); }
  return(index);
}

int  VdAmin(int n, double *x)
{ int	index	= 0;
  double	min	= abs(*x);
  int	i;

  for (i = 1; i < n; i++)
   if (abs(x[i]) < min)
    { index = i; min = abs(x[i]); }

  return(index);
}

int  VdsAmin(int n, double *x, int stride)
{ int	index	= 0;
  double	min	= abs(*x);
  int	i;

  for (i = 1, x += stride; i < n; i++, x += stride)
   if (abs(*x) < min)
    { index = i; min = abs(*x); }

  return(index);
}

double  VdDot(int n, double *x, double *y)
{ double	result = 0.0;
  
  while (n--)
   result += (*x++ * *y++);

  return(result);
}

double  VdsDot(int n, double *x, int x_stride, double *y, int y_stride)
{ double	result = 0.0;

  while (n--)
   { result += (*x * *y); x += x_stride; y += y_stride; }

  return(result);
}

double VdSum(int n, double *x)
{ double	result = 0.0;

  while (n--)
   result += *x++;
 
  return(result);
}

double VdsSum(int n, double *x, int stride)
{ double	result = 0.0;

  while (n--)
   { result += *x; x += stride; }

  return(result);
}

double VdProd(int n, double *x)
{ double	result = 1.0;

  while (n--)
   result *= *x++;

  return(result);
}

double VdsProd(int n, double *x, int stride)
{ double	result = 1.0;

  while (n--)
   { result *= *x; x += stride; }
}


