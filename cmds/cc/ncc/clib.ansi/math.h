#pragma force_top_level
#pragma include_only_once

/* math.h: ANSI 'C' (X3J11 Oct 88) library header, section 4.5 */
/* Copyright (C) Codemist Ltd. */
/* version 0.02 */

#ifndef __math_h
#define __math_h

#ifndef HUGE_VAL
#  define HUGE_VAL __huge_val
extern const double HUGE_VAL;
#endif

extern double acos(double /*x*/);
   /* computes the principal value of the arc cosine of x */
   /* a domain error occurs for arguments not in the range -1 to 1 */
   /* Returns: the arc cosine in the range 0 to Pi. */
extern double asin(double /*x*/);
   /* computes the principal value of the arc sine of x */
   /* a domain error occurs for arguments not in the range -1 to 1 */
   /* and -HUGE_VAL is returned. */
   /* Returns: the arc sine in the range -Pi/2 to Pi/2. */
extern double atan(double /*x*/);
   /* computes the principal value of the arc tangent of x */
   /* Returns: the arc tangent in the range -Pi/2 to Pi/2. */
extern double atan2(double /*x*/, double /*y*/);
   /* computes the principal value of the arc tangent of y/x, using the */
   /* signs of both arguments to determine the quadrant of the return value */
   /* a domain error occurs if both args are zero, and -HUGE_VAL returned. */
   /* Returns: the arc tangent of y/x, in the range -Pi to Pi. */

extern double cos(double /*x*/);
   /* computes the cosine of x (measured in radians). A large magnitude */
   /* argument may yield a result with little or no significance */
   /* Returns: the cosine value. */
extern double sin(double /*x*/);
   /* computes the sine of x (measured in radians). A large magnitude */
   /* argument may yield a result with little or no significance */
   /* Returns: the sine value. */
extern double tan(double /*x*/);
   /* computes the tangent of x (measured in radians). A large magnitude */
   /* argument may yield a result with little or no significance */
   /* Returns: the tangent value. */
   /*          if range error; returns HUGE_VAL. */

extern double cosh(double /*x*/);
   /* computes the hyperbolic cosine of x. A range error occurs if the */
   /* magnitude of x is too large. */
   /* Returns: the hyperbolic cosine value. */
   /*          if range error; returns HUGE_VAL. */
extern double sinh(double /*x*/);
   /* computes the hyperbolic sine of x. A range error occurs if the */
   /* magnitude of x is too large. */
   /* Returns: the hyperbolic sine value. */
   /*          if range error; returns -HUGE_VAL or HUGE_VAL depending */
   /*          on the sign of the argument */
extern double tanh(double /*x*/);
   /* computes the hyperbolic tangent of x. */
   /* Returns: the hyperbolic tangent value. */

extern double exp(double /*x*/);
   /* computes the exponential function of x. A range error occurs if the */
   /* magnitude of x is too large. */
   /* Returns: the exponential value. */
   /*          if underflow range error; 0 is returned. */
   /*          if overflow range error; HUGE_VAL is returned. */
extern double frexp(double /*value*/, int * /*exp*/);
   /* breaks a floating-point number into a normalised fraction and an */
   /* integral power of 2. It stores the integer in the int object pointed */
   /* to by exp. */
   /* Returns: the value x, such that x is a double with magnitude in the */
   /* interval 0.5 to 1.0 or zero, and value equals x times 2 raised to the */
   /* power *exp. If value is zero, both parts of the result are zero. */
extern double ldexp(double /*x*/, int /*exp*/);
   /* multiplies a floating-point number by an integral power of 2. */
   /* A range error may occur. */
   /* Returns: the value of x times 2 raised to the power of exp. */
   /*          if range error; HUGE_VAL is returned. */
extern double log(double /*x*/);
   /* computes the natural logarithm of x. A domain error occurs if the */
   /* argument is negative, and -HUGE_VAL is returned. A range error occurs */
   /* if the argument is zero. */
   /* Returns: the natural logarithm. */
   /*          if range error; -HUGE_VAL is returned. */
extern double log10(double /*x*/);
   /* computes the base-ten logarithm of x. A domain error occurs if the */
   /* argument is negative. A range error occurs if the argument is zero. */
   /* Returns: the base-ten logarithm. */
extern double modf(double /*value*/, double * /*iptr*/);
   /* breaks the argument value into integral and fraction parts, each of */
   /* which has the same sign as the argument. It stores the integral part */
   /* as a double in the object pointed to by iptr. */
   /* Returns: the signed fractional part of value. */

extern double pow(double /*x*/, double /*y*/);
   /* computes x raised to the power of y. A domain error occurs if x is */
   /* zero and y is less than or equal to zero, or if x is negative and y */
   /* is not an integer, and -HUGE_VAL returned. A range error may occur. */
   /* Returns: the value of x raised to the power of y. */
   /*          if underflow range error; 0 is returned. */
   /*          if overflow range error; HUGE_VAL is returned. */
extern double sqrt(double /*x*/);
   /* computes the non-negative square root of x. A domain error occurs */
   /* if the argument is negative, and -HUGE_VAL returned. */
   /* Returns: the value of the square root. */

extern double ceil(double /*x*/);
   /* computes the smallest integer not less than x. */
   /* Returns: the smallest integer not less than x, expressed as a double. */
extern double fabs(double /*x*/);
   /* computes the absolute value of the floating-point number x. */
   /* Returns: the absolute value of x. */
extern double floor(double /*d*/);
   /* computes the largest integer not greater than x. */
   /* Returns: the largest integer not greater than x, expressed as a double */
extern double fmod(double /*x*/, double /*y*/);
   /* computes the floating-point remainder of x/y. */
   /* Returns: the value x - i * y, for some integer i such that, if y is */
   /*          nonzero, the result has the same sign as x and magnitude */
   /*          less than the magnitude of y. If y is zero, a domain error */
   /*          occurs and -HUGE_VAL is returned. */

#endif

/* end of math.h */
