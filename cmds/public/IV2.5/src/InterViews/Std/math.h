/*
 * Interface to not-so-standard C math libraries.
 */

#ifndef math_h

#define abs math_h_abs
#define fabs math_h_fabs
#define atof math_h_atof
#define sin math_h_sin
#define cos math_h_cos
#define tan math_h_tan
#define asin math_h_asin
#define acos math_h_acos
#define atan math_h_atan
#define atan2 math_h_atan2
#define sinh math_h_sinh
#define cosh math_h_cosh
#define tanh math_h_tanh
#define frexp math_h_frexp
#define ldexp math_h_ldexp
#define modf math_h_modf
#define exp math_h_exp
#define log math_h_log
#define log10  math_h_log10 
#define pow math_h_pow
#define sqrt math_h_sqrt
#define floor math_h_floor
#define ceil math_h_ceil
#define trunc math_h_trunc
#define rint math_h_rint
#define fmod math_h_fmod
#define j0 math_h_j0
#define j1 math_h_j1
#define jn math_h_jn
#define y0 math_h_y0
#define y1 math_h_y1
#define yn math_h_yn
#define gamma math_h_gamma
#define hypot math_h_hypot
#define erf math_h_erf
#define erfc math_h_erfc
#define matherr math_h_matherr

/*
 * Sun <math.h> includes <floatingpoint.h>, which
 * typedef's single to float and declares strtod.
 *
 * I don't see a need to ifdef this for Sun as others may
 * follow what Sun does.
 */

#define single math_h_single
#define strtod math_h_strtod

#include "//usr/include/math.h"

#undef single
#undef strtod

#undef abs
#undef fabs
#undef atof
#undef sin
#undef cos
#undef tan
#undef asin
#undef acos
#undef atan
#undef atan2
#undef sinh
#undef cosh
#undef tanh
#undef frexp
#undef ldexp
#undef modf
#undef exp
#undef log
#undef log10  math_h_log10
#undef pow
#undef sqrt
#undef floor
#undef ceil
#undef trunc
#undef rint
#undef fmod
#undef j0
#undef j1
#undef jn
#undef y0
#undef y1
#undef yn
#undef gamma
#undef hypot
#undef erf
#undef erfc
#undef matherr

/* just in case standard header file didn't */
#ifndef math_h
#define math_h
#endif

#if defined(HUGE) && !defined(HUGE_VAL)
#define HUGE_VAL HUGE
#endif

#if !defined(MAXFLOAT)
#define MAXFLOAT ((float)1.701411733192644299e+38)
#endif

#if !defined(M_PI)
#define M_E 2.7182818284590452354
#define M_LOG2E	1.4426950408889634074
#define M_LOG10E 0.43429448190325182765
#define M_LN2 0.69314718055994530942
#define M_LN10 2.30258509299404568402
#define M_PI 3.14159265358979323846
#define M_PI_2 1.57079632679489661923
#define M_PI_4 0.78539816339744830962
#define M_1_PI 0.31830988618379067154
#define M_2_PI 0.63661977236758134308
#define M_2_SQRTPI 1.12837916709551257390
#define M_SQRT2	1.41421356237309504880
#define M_SQRT1_2 0.70710678118654752440
#endif

extern int abs(int);
extern double fabs(double);

extern double atof(const char*);

extern double sin(double);
extern double cos(double);
extern double tan(double);
extern double asin(double);
extern double acos(double);
extern double atan(double);
extern double atan2(double,double);
extern double sinh(double);
extern double cosh(double);
extern double tanh(double);

extern double frexp(double, int*);
extern double ldexp(double, int);
extern double modf(double, double*);

extern double exp(double);
extern double log(double);
extern double log10(double); 
extern double pow(double, double);
extern double sqrt(double);

extern double floor(double);
extern double ceil(double);
extern double trunc(double);
extern double rint(double);
extern double fmod(double, double);

extern double j0(double);
extern double j1(double);
extern double jn(double);
extern double y0(double);
extern double y1(double);
extern double yn(int, double);

extern double gamma(double);
extern double hypot(double, double);

extern double erf(double);
extern double erfc(double);

extern int matherr(struct exception*);

#endif
