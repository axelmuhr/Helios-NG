
/* math.c: ANSI draft (X3J11 May 86) library code, section D.5 */
/* Copyright (C) A.C. Norman and A. Mycroft */

/* version 0.04b */
/* Nov 87: fix bug in ibm frexp(-ve arg).                                   */

#include "hostsys.h"
#include <limits.h>

/* This file contains code for most of the math routines from <math.h>      */

/* On the ARM some of these routines are implemented as floating point      */
/* opcodes and as such appear in startup.s                                  */

#ifndef NO_FLOATING_POINT

/* the macro HUGE_VAL must be defined before <math.h> is included since  */
/* <math.h> declares _huge_val extern if HUGE_VAL is not defined.        */

#define HUGE_VAL _huge_val

#include <math.h>                          /* for forward references */

#ifdef IBMFLOAT
const double _huge_val = 7.2370055773322621e+75;
#else
const double _huge_val = 1.79769313486231571e+308;
#endif

#ifdef IBMFLOAT

double frexp(d, lvn)
double d; int *lvn;
{
    fp_number d1;
    int n;
    if (d==0.0)
/* I worry a little about signed zeros here. I hope that -0.0 == 0.0     */
    {   *lvn = 0;
        return 0.0;
    }
    d1.d = d;
    n = 4*(d1.i.x - 0x40);        /* excess 64 exponent */
    d1.i.x = 0x40;
    d = d1.d;
    /* Note that the following code works for unnormalised numbers, but  */
    /* can then take 55 cycles to converge instead of usual 3 max.       */
    while ((d>=0 ? d : -d) < 0.5) d = d+d, n--;
    /* Now d is most definitely normalised.                              */
    *lvn = n;
    return d;
}


double ldexp(d, n)
double d; int n;
{
    fp_number d1;
    int nx;
    if (d==0.0) return 0.0;         /* special case                      */
    d1.d = d;
    nx = d1.i.x + (n & ~3)/4;
    n &= 3;
#ifndef DO_NOT_SUPPORT_UNNORMALIZED_NUMBERS
    /* The following code gets the msd/expt right for unnormalised nos.  */
    d1.i.x = 0x40;
    d1.d += 0.0;                    /* i.e. normalise                    */
    nx += d1.i.x - 0x40;
#endif
    {   int mhi = d1.i.mhi;
        int nx1 = (mhi & 0x00c00000)==0 ? 
                   ((mhi & 0x00200000)==0 ? n - 3 : n - 2) :
                   ((mhi & 0x00800000)==0 ? n - 1 : n);
        if (nx1 > 0) nx++, n -= 4;
/* That just dealt with the fact that in IBM format the exponent is for  */
/* base 16 and so scaling by a power of two can involve a real multiply. */
/* I now know what the true exponent (nx) in the result will be.         */
    }
    if (nx > 0x7f)                  /* Overflow (maybe do a raise() ?)   */
    {   d1.i.x = 0x7f;
        d1.i.mhi = 0xffffff;
        d1.i.mlo = 0xffffffff;
        return d1.d;
    }
    if (nx < 0)                     /* Deal with underflow/unnormalised  */
    {   if (nx <= -14) return 0.0;
        d1.i.x = 0;
        while (nx < 0) d1.d /= 16, nx++;     /* de-normalise gracefully  */
    }
    d1.i.x = nx;
    {   double d2;
        switch (n)
        {
case -3:d2 = 0.125; break;
case -2:d2 = 0.25;  break;
case -1:d2 = 0.5;   break;
default:   /* avoid dataflow whinge */
case 0: d2 = 1.0;   break;
case 1: d2 = 2.0;   break;
case 2: d2 = 4.0;   break;
case 3: d2 = 8.0;   break;
        }
        d1.d *= d2;
    }
    return d1.d;
}


#else       /* Here is the IEEE format stuff */

#ifndef DO_NOT_SUPPORT_UNNORMALIZED_NUMBERS

double frexp(d, lvn)
double d; int *lvn;
{
/* This version works even if d starts off as an unnormalized number in  */
/* the IEEE sense. But in that special case it will be mighty slow!      */
/* By that we mean at most 52 iterations for the smallest number.        */
    fp_number d1;
    int n;
    if (d==0.0)
    {   *lvn = 0;
        return 0.0;
    }
    d1.d = d;
    if ((n = d1.i.x - 0x3fe) == -0x3fe)
    {   int flag;
/* Here d1 has zero in its exponent field - this means that the mantissa */
/* is un-normalized. I have to shift it left (at least one bit) until a  */
/* suitable nonzero bit appears to go in the implicit-bit place in the   */
/* fractional result. For each bit shifted I need to adjust the final    */
/* exponent that will be returned.                                       */
/* I have already tested to see if d was zero so the fllowing loop MUST  */
/* terminate.                                                            */
        do
        {   flag = d1.i.mhi & 0x00080000;
            d1.i.mhi = (d1.i.mhi << 1) | (d1.i.mlo >> 31);
            d1.i.mlo = d1.i.mlo << 1;
            n--;
        } while (flag==0);
    }
    *lvn = n;
    d1.i.x = 0x3fe;
    return d1.d;
}

#else   /* DO_NOT_SUPPORT_UNNORMALIZED_NUMBERS */

double frexp(d, lvn)
double d; int *lvn;
{
    fp_number d1;
    if (d==0.0)
    {   *lvn = 0;
        return 0.0;
    }
    d1.d = d;
    *lvn = d1.i.x - 0x3fe;
    d1.i.x = 0x3fe;
    return d1.d;
}

#endif   /* DO_NOT_SUPPORT_UNNORMALIZED_NUMBERS */

double ldexp(d, n)
double d; int n;
{
    fp_number d1;
    int nx;
    if (d==0.0) return 0.0;        /* special case                       */
    d1.d = d;
    nx = (int) d1.i.x + n;
    if (nx >= 0x7ff)
    {   nx = 0x7ff;                 /* overflow yields 'infinity'        */
        d1.i.mhi = 0;
        d1.i.mlo = 0;
    }
/* Maybe I should be prepared to generate un-normalized numbers here, or */
/* even deal with input d un-normalized and n positive yielding a proper */
/* result. All that seems like a lot of work and so I will not even try  */
/* in this version of the code!                                          */
    else if (nx <= 0) return 0.0;  /* deal with underflow                */
    d1.i.x = nx;
    return (d1.d);
}

#endif


#ifdef IBMFLOAT
#define _exp_arg_limit 174.673089501106208
#else
#define _exp_arg_limit 709.78271289338397
#endif

/* machine independent code - but beware the 1e-20's */

#define _pi_       3.14159265358979323846
#define _pi_3      1.04719755119659774615
#define _pi_2      1.57079632679489661923
#define _pi_4      0.78539816339744830962
#define _pi_6      0.52359877559829887038
#define _sqrt_half 0.70710678118654752440

#ifndef HOST_HAS_TRIG
/* The ARM has a load of trig functions etc available as opcodes, so     */
/* has no need for these software versions shown here.                   */

static double _sincos(double x, double y, int sign, int coscase)
{
    int n;
    double xn, f, g, r;
    if (y >= 1.0e9)     /* fail if argument is overlarge                 */
    {   errno = EDOM;
        return 0.0;
    }
    n = (int) ((y + _pi_2) / _pi_);
    xn = (double) n;
    if ((n & 1) != 0) sign = -sign;
    if (coscase) xn = xn - 0.5;
/* Observe uses of unary '+' to force evaluation order.                  */
#ifdef IBMFLOAT
    {   double x1 = (double)(int)x;
/* observe that the range check on y assures me that (int)x is OK.       */
        double x2 = x - x1;
        f = +(+(x1 - xn*3.1416015625) + x2) + xn*8.908910206761537356617e-6;
    }
#else
    f = +(x - xn*3.1416015625) + xn*8.908910206761537356617e-6;
#endif
/* I expect that the absolute value of f is less than pi/2 here          */
    if (fabs(f) >= 1.e-10)
#define _sincos_r1  -0.16666666666666665052
#define _sincos_r2   0.83333333333331650315e-2
#define _sincos_r3  -0.19841269841201840457e-3
#define _sincos_r4   0.27557319210152756119e-5
#define _sincos_r5  -0.25052106798274584544e-7
#define _sincos_r6   0.16058936490371589114e-9
#define _sincos_r7  -0.76429178068910467734e-12
#define _sincos_r8   0.27204790957888846175e-14
    {   g = f*f;
        r = ((((((((_sincos_r8) * g + _sincos_r7) * g + _sincos_r6) * g +
                    _sincos_r5) * g + _sincos_r4) * g + _sincos_r3) * g +
                    _sincos_r2) * g + _sincos_r1) * g;
        f += f*r;
    };
    if (sign < 0) return -f;
    else return f;
}

double sin(double x)
{
    if (x < 0.0) return _sincos(-x, -x, -1, 0);
    else return _sincos(x, x, 1, 0);
}

double cos(double x)
{
    if (x < 0.0) return _sincos(-x, _pi_2 - x, 1, 1);
    else return _sincos(x, _pi_2 + x, 1, 1);
}

#ifdef IBMFLOAT
#define _exp_negative_arg_limit -180.218266945585778
#else
/* NB: the following value is a proper limit provided denormalised       */
/* values are not being supported. It would need to be changed if they   */
/* were to start existing.                                               */
#define _exp_negative_arg_limit -708.39641853226408
#endif

double exp(double x)
{
    int n;
    double xn, g, z, gp, q, r;
    if (x > _exp_arg_limit)
    {   errno = ERANGE;
        return HUGE_VAL;
    }
    if (x < _exp_negative_arg_limit) return 0.0;
    if (fabs(x) < 1.e-20) return 1.0;
/* In C the cast (int)x truncates towards zero. Here I want to round.    */
    n = (int)((x >= 0 ? 0.5 : -0.5) + 1.44266950408889634074 * x);
    xn = (double)n;
#ifdef IBMFLOAT
    {   double x1 = (double)(int)x;
        double x2 = x - x1;
        g = +(+(x1 - xn * 0.693359375) + x2) - xn * (-2.1219444005469058277e-4);
    }
#else
    g = +(x - xn * 0.693359375) - xn * (-2.1219444005469058277e-4);
#endif
    z = g * g;
#define  _exp_p0  0.249999999999999993
#define  _exp_p1  0.694360001511792852e-2
#define  _exp_p2  0.165203300268279130e-4
#define  _exp_q0  0.500000000000000000
#define  _exp_q1  0.555538666969001188e-1
#define  _exp_q2  0.495862884905441294e-3
    gp = ((_exp_p2 * z + _exp_p1) * z + _exp_p0) * g;
    q = (_exp_q2 * z + _exp_q1) * z + _exp_q0;
    r = 0.5 + gp / (q - gp);
    return ldexp(r, n + 1);
}

double log(double x)
{
    double f, znum, zden, z, w, r, xn;
    int n;
    if (x <= 0.0)
    {   if (x==0.0) errno = ERANGE;
        else errno = EDOM;
        return -HUGE_VAL;
    }
    f = frexp(x, &n);
    if (f > _sqrt_half)
    {   znum = +(f - 0.5) - 0.5;
        zden = f * 0.5 + 0.5;
    }
    else
    {   n -= 1;
        znum = f - 0.5;
        zden = znum * 0.5 + 0.5;
    }
    z = znum / zden;
    w = z * z;
#define _log_a0 -0.64124943423745581147e2
#define _log_a1  0.16383943563021534222e2
#define _log_a2 -0.78956112887491257267e0
#define _log_b0 -0.76949932108494879777e3
#define _log_b1  0.31203222091924532844e3
#define _log_b2 -0.35667977739034646171e2
    r = w * ( ((_log_a2 * w + _log_a1) * w + _log_a0) /
              (((w + _log_b2) * w + _log_b1) * w + _log_b0) );
    r = z + z * r;
    xn = (double)n;
    return +(xn*(-2.121944400546905827679e-4) + r) + xn*(355.0/512.0);
}

double log10(double x)
{
    return log(x) * 0.43429448190325182765;  /* log10(e) */
}

double sqrt(double x)
{
    fp_number f;
    double y0;
    int n;
    if (x <= 0.0)
    {   if (x < 0.0) errno = EDOM;
        return 0.0;
    }
    f.d = x;
#ifdef IBMFLOAT
    n = f.i.x - 0x40;
    f.i.x = 0x40;
#else
    n = f.i.x - 0x3fe;
    f.i.x = 0x3fe;
#endif
    {   double fd = f.d;
#ifdef IBMFLOAT
        y0 = 0.223607 + 0.894427 * fd;
#else
        y0 = 0.41731 + 0.59016 * fd;
#endif
        y0 = 0.5 * (y0 + fd/y0);
        y0 = 0.5 * (y0 + fd/y0);
        y0 = 0.5 * (y0 + fd/y0);
#ifdef IBMFLOAT
#define __EPS 2.2204460492503131e-16
        y0 = y0 + +(0.5 * (fd/y0 - y0) + __EPS/32.0);
#endif
    }
    if (n & 1)
    {
#ifdef IBMFLOAT
        y0 = (y0 + __EPS/8.0) * 0.25;
#else
        y0 *= _sqrt_half;
#endif
        n += 1;
    }
    n /= 2;
    f.d = y0;
    f.i.x += n;
    return f.d;
}

double _tancot(double x, int iflag)
{
    int n;
    double f, g, xnum, xden, y, xn;
    y = fabs(x);
    if (y >= 1.0e9)     /* fail if argument is overlarge                 */
    {   errno = EDOM;
        return 0.0;
    }
    n = (int) ((2.0 * y + _pi_2) / _pi_);
    if (x < 0) n = - n;
    xn = (double) n;
#ifdef IBMFLOAT
    {   double x1 = (double)(int)x;
        double x2 = x - x1;
        f = +(+(x1 - xn*1.57080078125) + x2) + xn*4.454455103380768678308e-6;
    }
#else
    f = +(x - xn*1.57080078125) + xn*4.454455103380768678308e-6;
#endif
    if (fabs(f) > 1.e-10)
    {   g = f * f;
#define _tan_p1 -0.13338350006421960681
#define _tan_p2  0.34248878235890589960e-2
#define _tan_p3 -0.17861707342254426711e-4
#define _tan_q0  1.00000000000000000000
#define _tan_q1 -0.46671683339755294240
#define _tan_q2  0.25663832289440112864e-1
#define _tan_q3 -0.31181531907010027307e-3
#define _tan_q4  0.49819433993786512270e-6
        xnum = ((_tan_p3*g + _tan_p2)*g + _tan_p1)*g*f + f;
        xden = (((_tan_q4*g + _tan_q3)*g + _tan_q2)*g + _tan_q1)*g + _tan_q0;
    }
    else
    {   xnum = f;
        xden = 1.0;
    }
/* It is probable that overflow can never occur here, since floating     */
/* point values fall about or more 1.e-16 apart near singularities       */
/* of the tangent function.                                              */
    if (iflag==0)
    {   if ((n & 1) == 0) return xnum / xden;
        else return - xden / xnum;
    }
    else
    {   if ((n & 1) == 0) return xden / xnum;
        else return - xnum / xden;
    }
}

double tan(double x)
{
    return _tancot(x, 0);
}

double _cot(double x)      /* Not specified by ANSI hence the funny name */
{
    if (fabs(x) < 1.0/HUGE_VAL)
    {   errno = ERANGE;
        if (x < 0.0) return -HUGE_VAL;
        else return HUGE_VAL;
    }
    return _tancot(x, 1);
}

double atan(double x)
{
    int n;
    double f;
    const static double a[4] = { 0.0, _pi_6, _pi_2, _pi_3 };
    f = fabs(x);
    if (f > 1.0)
    {   f = 1.0 / f;
        n = 2;
    }
    else n = 0;
#define _two_minus_root_three 0.26794919243112270647
#define _sqrt_three           1.73205080756887729353
#define _sqrt_three_minus_one 0.73205080756887729353
    if (f > _two_minus_root_three)
    {   f = (+(+(_sqrt_three_minus_one*f - 0.5) - 0.5) + f) / (_sqrt_three + f);
        n++;
    }
    if (fabs(f) > 1.e-10)
    {   double g = f * f;
        double r;
#define _atan_p0    -0.13688768894191926929e2
#define _atan_p1    -0.20505855195861651981e2
#define _atan_p2    -0.84946240351320683534e1
#define _atan_p3    -0.83758299368150059274
#define _atan_q0     0.41066306682575781263e2
#define _atan_q1     0.86157349597130242515e2
#define _atan_q2     0.59578436142597344465e2
#define _atan_q3     0.15024001160028576121e2
        r = ((((_atan_p3*g + _atan_p2)*g + _atan_p1)*g + _atan_p0)*g) /
             ((((g + _atan_q3)*g + _atan_q2)*g + _atan_q1)*g + _atan_q0);
        f = f + f * r;
    }
    if (n > 1) f = -f;
    f = f + a[n];
    if (x < 0) return -f;
    else return f;
}

double _asinacos(double x, int flag)
{
    int i;
    double y, g, r;
    const static double a[2] = {0.0, _pi_4 };
    const static double b[2] = {_pi_2, _pi_4 };
    y = fabs(x);
    if (y < 1.e-10) i = flag;
    else
    {   if (y > 0.5)
        {   i = 1 - flag;
            if (y > 1.0)
            {   errno = EDOM;
                return HUGE_VAL;
            }
            g = (+(0.5 - y) + 0.5) * 0.5;
            y = -2.0 * sqrt(g);
        }
        else
        {   i = flag;
            g = y * y;
        }
#define _asin_p1    -0.27368494524164255994e2
#define _asin_p2     0.57208227877891731407e2
#define _asin_p3    -0.39688862997504877339e2
#define _asin_p4     0.10152522233806463645e2
#define _asin_p5    -0.69674573447350646411e0
#define _asin_q0    -0.16421096714498560795e3
#define _asin_q1     0.41714430248260412556e3
#define _asin_q2    -0.38186303361750149284e3
#define _asin_q3     0.15095270841030604719e3
#define _asin_q4    -0.23823859153670238830e2
        r = (((((_asin_p5*g + _asin_p4)*g + _asin_p3)*g +
                              _asin_p2)*g + _asin_p1)*g)      /
             (((((g + _asin_q4)*g + _asin_q3)*g +
                      _asin_q2)*g + _asin_q1)*g + _asin_q0);
        y = y + y*r;
    }
    if (flag==0)
    {   y = +(a[i] + y) + a[i];
        if (x<0) y = -y;
    }
    else if (x < 0) y = +(b[i] + y) + b[i];
    else y = +(a[i] - y) + a[i];
    return y;
}

double asin(double x)
{
    return _asinacos(x, 0);
}

double acos(double x)
{
    return _asinacos(x, 1);
}

double pow(double x, double y)
{
    int sign = 0, m, p, i, mdash, pdash;
    double g, r, z, v, u1, u2;
/* The table a1[] contains properly rounded values for 2**(i/16), and    */
/* a2[] contains differences between the true values of 2**(i/16) and    */
/* the a1[] values for odd i.                                            */
#ifdef IBMFLOAT
    const static double a1[17] = {
/* It is painfully important that the following 17 floating point        */
/* numbers are read in to yield the quantities shown on the right.       */
        1.000000000000000000,    /* 41100000:00000000 */
        0.957603280698573644,    /* 40f5257d:152486cc */
        0.917004043204671229,    /* 40eac0c6:e7dd2439 */
        0.878126080186649740,    /* 40e0ccde:ec2a94e1 */
        0.840896415253714543,    /* 40d744fc:cad69d6b */
        0.805245165974627155,    /* 40ce248c:151f8481 */
        0.771105412703970413,    /* 40c5672a:115506db */
        0.738413072969749659,    /* 40bd08a3:9f580c37 */
        0.707106781186547531,    /* 40b504f3:33f9de65 */
        0.677127773468446367,    /* 40ad583e:ea42a14b */
        0.648419777325504834,    /* 40a5fed6:a9b15139 */
        0.620928906036742028,    /* 409ef532:6091a112 */
        0.594603557501360527,    /* 409837f0:518db8a9 */
        0.569394317378345823,    /* 4091c3d3:73ab11c3 */
        0.545253866332628830,    /* 408b95c1:e3ea8bd7 */
        0.522136891213706919,    /* 4085aac3:67cc487b */
        0.500000000000000000     /* 40800000:00000000 */
        };
    const static double a2[8] = {
        2.4114209503420287e-18,
        9.2291566937243078e-19,
        -1.5241915231122319e-18,
        -3.5421849765286817e-18,
        -3.1286215245415074e-18,
        -4.4654376565694489e-18,
        2.9305146686217562e-18,
        1.1260851040933474e-18
        };
#else /* IEEE format */
    const static double a1[17] = {
/* It is painfully important that the following 17 floating point        */
/* numbers are read in to yield the quantities shown on the right.       */
        1.0,                    /* 3ff00000:00000000 */
        0.9576032806985737,     /* 3feea4af:a2a490da */
        0.91700404320467121,    /* 3fed5818:dcfba487 */
        0.87812608018664973,    /* 3fec199b:dd85529c */
        0.8408964152537145,     /* 3feae89f:995ad3ad */
        0.80524516597462714,    /* 3fe9c491:82a3f090 */
        0.77110541270397037,    /* 3fe8ace5:422aa0db */
        0.73841307296974967,    /* 3fe7a114:73eb0187 */
        0.70710678118654757,    /* 3fe6a09e:667f3bcd */
        0.67712777346844632,    /* 3fe5ab07:dd485429 */
        0.64841977732550482,    /* 3fe4bfda:d5362a27 */
        0.620928906036742,      /* 3fe3dea6:4c123422 */
        0.59460355750136051,    /* 3fe306fe:0a31b715 */
        0.56939431737834578,    /* 3fe2387a:6e756238 */
        0.54525386633262884,    /* 3fe172b8:3c7d517b */
        0.52213689121370688,    /* 3fe0b558:6cf9890f */
        0.5                     /* 3fe00000:00000000 */
        };
    const static double a2[8] = {
        -5.3099730280915798e-17,
        1.4800703477186887e-17,
        1.2353596284702225e-17,
        -1.7419972784343138e-17,
        3.8504741898901863e-17,
        2.3290137959059465e-17,
        4.4563878092065126e-17,
        4.2759448527536718e-17
        };
#endif
    if (y == 1.0) return x;
    if (x <= 0.0)
    {   int ny;
        if (x==0.0)
        {   if (y <= 0.0)
            {   errno = EDOM;
                if (y==0.0) return 1.0;
                else return HUGE_VAL;
            }
            return 0.0;
        }
        if (y < (double)INT_MIN || y > (double)INT_MAX ||
            (double)(ny = (int)y) != y)
        {   errno = EDOM;
            return HUGE_VAL;
        }
/* Here y is an integer and x is negative.                               */
        x = -x;
        sign = (ny & 1);
    }
    if (y == 2.0 && x < 1.e20) return x*x;  /* special case.             */
    g = frexp(x, &m);
    p = 0;
    if (g <= a1[8]) p = 8;
    if (g <= a1[p+4]) p += 4;
    if (g <= a1[p+2]) p += 2;
    z = (+(g - a1[p+1]) - a2[p/2]) / (0.5*g + 0.5*a1[p+1]);
/* Expect abs(z) <= 0.044 here */
    v = z * z;
#define _pow_p1 0.83333333333333211405e-1
#define _pow_p2 0.12500000000503799174e-1
#define _pow_p3 0.22321421285924258967e-2
#define _pow_p4 0.43445775672163119635e-3
    r = (((_pow_p4*v + _pow_p3)*v + _pow_p2)*v + _pow_p1)*v*z;
#define _pow_k 0.44269504088896340736
    r = r + _pow_k * r;
    u2 = +(r + z * _pow_k) + z;
#define _reduce(v) ((double)(int)(16.0*(v))*0.0625)
    u1 = (double)(16*m-p-1) * 0.0625;
    {   double y1 = _reduce(y);
        double y2 = y - y1;
        double w = u2*y + u1*y2;
        double w1 = _reduce(w);
        double w2 = w - w1;
        int iw1;
        w = w1 + u1*y1;
        w1 = _reduce(w);
        w2 = w2 + +(w - w1);
        w = _reduce(w2);
        iw1 = (int)(16.0*(w1+w));
        w2 = w2 - w;
/* The following values have been determined experimentally, buth their  */
/* values are not very critical.                                         */
#ifdef IBMFLOAT
#  define _negative_pow_limit -4160
#else
#  define _negative_pow_limit -16352
#endif
        if (iw1 < _negative_pow_limit)
        {   errno = ERANGE;         /* Underflow certain                 */
            return 0.0;
        }
        if (w2 > 0.0)
        {   iw1 += 1;
            w2 -= 0.0625;
        }
        if (iw1 < 0) i = 0;
        else i = 1;
        mdash = iw1/16 + i;
        pdash = 16*mdash - iw1;
#define _pow_q1 0.69314718055994529629
#define _pow_q2 0.24022650695909537056
#define _pow_q3 0.55504108664085595326e-1
#define _pow_q4 0.96181290595172416964e-2
#define _pow_q5 0.13333541313585784703e-2
#define _pow_q6 0.15400290440989764601e-3
#define _pow_q7 0.14928852680595608186e-4
        z = ((((((_pow_q7*w2 + _pow_q6)*w2 + _pow_q5)*w2 +
                  _pow_q4)*w2 + _pow_q3)*w2 + _pow_q2)*w2 + _pow_q1)*w2;
        z = a1[pdash] + a1[pdash]*z;
        z = frexp(z, &m);
        mdash += m;
#ifdef IBMFLOAT
        if (mdash > 0x3f*4)
        {   errno = ERANGE;
            if (sign) r = -HUGE_VAL;
            else r = HUGE_VAL;
        }
        else if (mdash <= -0x41*4)
        {   errno = ERANGE;
            r = 0.0;
        }
#else
        if (mdash >= 0x7ff-0x3fe)
        {   errno = ERANGE;
            if (sign) r = -HUGE_VAL;
            else r = HUGE_VAL;
        }
        else if (mdash <= -0x3fe)
        {   errno = ERANGE;
            r = 0.0;
        }
#endif
        else
        {   r = ldexp(z, mdash);
            if (sign) r = -r;
        }
    }
    return r;
}

#endif /* HOST_HAS_TRIG */

double atan2(double y, double x)
{
    if (x==0.0 && y==0.0)
    {   errno = EDOM;
        return 0.0;
    }
    if (fabs(x) < fabs(y))
    {   if (fabs(x / y)<1.0e-20)
        {   if (y<0.0) return - _pi_2;
            else return _pi_2;
        }
    }
    y = atan(y / x);
    if (x<0.0)
    {   if (y>0.0) y -= _pi_;
        else y += _pi_;
    }
    return y;
}

double fabs(double x)
{
    if (x<0.0) return -x;
    else return x;
}

double sinh(double x)
{
    int sign;
    double y;
    if (x<0.0) y = -x, sign = 1; else y = x, sign = 0;
    if (y>1.0)
    {
/* _sinh_lnv is REQUIRED to read in as a number with the lower part of   */
/* its floating point representation zero.                               */
#define    _sinh_lnv     0.69316101074218750000          /* ln(v)        */
#define    _sinh_vm2     0.24999308500451499336          /* 1/v^2        */
#define    _sinh_v2m1    0.13830277879601902638e-4       /* (v/2) - 1    */
        double w = y - _sinh_lnv, z, r;
        if (w>_exp_arg_limit)
        {   errno = ERANGE;
            if (sign) return -HUGE_VAL;
            else return HUGE_VAL;
        }
        z = exp(w);   /* should not overflow!                            */
        if (z < 1.0e10) z = z - _sinh_vm2/z;
        r = z + _sinh_v2m1 * z;
        if (sign) return -r;
        else return r;
    }
    else if (y<=1.0e-10) return x;
    else
    {
#define _sinh_p0    -0.35181283430177117881e6
#define _sinh_p1    -0.11563521196851768270e5
#define _sinh_p2    -0.16375798202630751372e3
#define _sinh_p3    -0.78966127417357099479e0
#define _sinh_q0    -0.21108770058106271242e7
#define _sinh_q1     0.36162723109421836460e5
#define _sinh_q2    -0.27773523119650701667e3
#define _sinh_q3     1.0
        double g = x*x;
        double r;
        /* Use a (minimax) rational approximation. See Cody & Waite.     */
        r = ((((_sinh_p3*g + _sinh_p2)*g + _sinh_p1)*g + _sinh_p0)*g) /
             (((g + _sinh_q2)*g + _sinh_q1)*g + _sinh_q0);
        return x + x*r;
    }
}

double cosh(double x)
{
    if (x<0.0) x = -x;
    if (x>1.0)
    {
        x = x - _sinh_lnv;
        if (x>_exp_arg_limit)
        {   errno = ERANGE;
            return HUGE_VAL;
        }
        x = exp(x);   /* the range check above ensures that this does    */
                      /* not overflow.                                   */
        if (x < 1.0e10) x = x + _sinh_vm2/x;
        /* This very final line might JUST overflow even though the call */
        /* to exp is safe and even though _exp_arg_limit is conservative */
        return x + _sinh_v2m1 * x;
    }
/* This second part is satisfactory, even though it is simple!           */
    x = exp(x);
    return 0.5*(x + 1/x);
}

double tanh(double x)
{
/* The first two exits avoid premature overflow as well as needless use  */
/* of the exp() function.                                                */
    int sign;
    if (x>27.0) return 1.0;         /* here exp(2x) dominates 1.0        */
    else if (x<-27.0) return -1.0;
    if (x<0.0) x = -x, sign = 1;
    else sign = 0;
    if (x>0.549306144334054846)     /* ln(3)/2 is crossover point        */
    {   x = exp(2.0*x);
        x = 1.0 - 2.0/(x + 1.0);
        if (sign) return -x;
        else return x;
    }
#define _tanh_p0    -0.16134119023996228053e4
#define _tanh_p1    -0.99225929672236083313e2
#define _tanh_p2    -0.96437492777225469787e0
#define _tanh_q0     0.48402357071988688686e4
#define _tanh_q1     0.22337720718962312926e4
#define _tanh_q2     0.11274474380534949335e3
#define _tanh_q3     1.0
    if (x>1.0e-10)
    {   double y = x*x;
        /* minimax rational approximation                                */
        y = (((_tanh_p2*y + _tanh_p1)*y + _tanh_p0)*y) /
             (((y + _tanh_q2)*y + _tanh_q1)*y + _tanh_q0);
        x = x + x*y;
    }
    if (sign) return -x;
    else return x;
}

double fmod(double x, double y)
{
/* floating point remainder of (x/y) for integral quotient. Remainder    */
/* has same sign as x.                                                   */
    double q, r;
    if (y==0.0 || x==0.0) return x;
    if (y < 0.0) y = -y;
    q = modf(x/y, &r);
    r = x - q * y;
/* The next few lines are an ultra-cautious scheme to ensure that the    */
/* result is less than fabs(y) in value and that it has the sign of x.   */
    if (x > 0.0)
    {   while (r >= y) r -= y;
        while (r < 0.0) r += y;
    }
    else
    {   while (r <= -y) r += y;
        while (r > 0.0) r -= y;
    }
    return r;
}

#ifdef IBMFLOAT

double floor(double d)
{
/* round x down to an integer towards minus infinity.                    */
    fp_number x;
    int exponent, mask, exact;
    if (d == 0.0) return 0.0;
    x.d = d;                            /* pun on union type             */
    if ((exponent = x.i.x - 0x40) < 0)
    {   if (x.i.s) return -1.0;
        else return 0.0;
    }
    else if (exponent >= 56/4) return x.d;
    if (exponent >= 24/4)
    {   mask = ((unsigned) 0xffffffff) >> (4*(exponent - 24));
        exact = x.i.mlo & mask;
        x.i.mlo &= ~mask;
    }
    else
    {   mask = 0xfffff >> (4*exponent);
        exact = (x.i.mhi & mask) | x.i.mlo;
        x.i.mhi &= ~mask;
        x.i.mlo = 0;
    }
    if (exact!=0 && x.i.s) return x.d - 1.0;
    else return x.d;
}

double ceil(double d)
{
/* round x up to an integer towards plus infinity.                       */
    fp_number x;
    int exponent, mask, exact;
    if (d == 0.0) return 0.0;
    x.d = d;                            /* pun on union type             */
    if ((exponent = x.i.x - 0x40) < 0)
    {   if (x.i.s) return 0.0;
        else return 1.0;
    }
    else if (exponent >= 56/4) return x.d;
    if (exponent >= 24/4)
    {   mask = ((unsigned) 0xffffffff) >> (4*(exponent - 24));
        exact = x.i.mlo & mask;
        x.i.mlo &= ~mask;
    }
    else
    {   mask = 0xfffff >> (4*exponent);
        exact = (x.i.mhi & mask) | x.i.mlo;
        x.i.mhi &= ~mask;
        x.i.mlo = 0;
    }
    if (exact!=0 && x.i.s==0) return x.d + 1.0;
    else return x.d;
}

double modf(double value, double *iptr)
{
/* splits value into integral part & fraction (both same sign)           */
    fp_number x;
    int exponent, mask;
    if (value == 0.0)
    {   *iptr = 0.0;
        return 0.0;
    }
    x.d = value;
    if ((exponent = x.i.x - 0x40) < 0)
    {   *iptr = 0.0;
        return value;
    }
    else if (exponent >= 56/4)
    {   *iptr = value;
        return 0.0;
    }
    if (exponent >= 24/4)
    {   mask = ((unsigned) 0xffffffff) >> (4*(exponent - 24));
        x.i.mlo &= ~mask;
    }
    else
    {   mask = 0xffffff >> (4*exponent);
        x.i.mhi &= ~mask;
        x.i.mlo = 0;
    }
    *iptr = x.d;
    return value - x.d;
}

#else /* IBMFLOAT */

double floor(double d)
{
/* round x down to an integer towards minus infinity.                    */
    fp_number x;
    int exponent, mask, exact;
    if (d == 0.0) return 0.0;
    x.d = d;                            /* pun on union type             */
    if ((exponent = x.i.x - 0x3ff) < 0)
    {   if (x.i.s) return -1.0;
        else return 0.0;
    }
    else if (exponent >= 52) return x.d;
    if (exponent >= 20)
    {   mask = ((unsigned) 0xffffffff) >> (exponent - 20);
        exact = x.i.mlo & mask;
        x.i.mlo &= ~mask;
    }
    else
    {   mask = 0xfffff >> exponent;
        exact = (x.i.mhi & mask) | x.i.mlo;
        x.i.mhi &= ~mask;
        x.i.mlo = 0;
    }
    if (exact!=0 && x.i.s) return x.d - 1.0;
    else return x.d;
}

double ceil(double d)
{
/* round x up to an integer towards plus infinity.                       */
    fp_number x;
    int exponent, mask, exact;
    if (d == 0.0) return 0.0;
    x.d = d;                            /* pun on union type             */
    if ((exponent = x.i.x - 0x3ff) < 0)
    {   if (x.i.s) return 0.0;
        else return 1.0;
    }
    else if (exponent >= 52) return x.d;
    if (exponent >= 20)
    {   mask = ((unsigned) 0xffffffff) >> (exponent - 20);
        exact = x.i.mlo & mask;
        x.i.mlo &= ~mask;
    }
    else
    {   mask = 0xfffff >> exponent;
        exact = (x.i.mhi & mask) | x.i.mlo;
        x.i.mhi &= ~mask;
        x.i.mlo = 0;
    }
    if (exact!=0 && x.i.s==0) return x.d + 1.0;
    else return x.d;
}

double modf(double value, double *iptr)
{
/* splits value into integral part & fraction (both same sign)           */
    fp_number x;
    int exponent, mask;
    if (value == 0.0)
    {   *iptr = 0.0;
        return 0.0;
    }
    x.d = value;
    if ((exponent = x.i.x - 0x3ff) < 0)
    {   *iptr = 0.0;
        return value;
    }
    else if (exponent >= 52)
    {   *iptr = value;
        return 0.0;
    }
    if (exponent >= 20)
    {   mask = ((unsigned) 0xffffffff) >> (exponent - 20);
        x.i.mlo &= ~mask;
    }
    else
    {   mask = 0xfffff >> exponent;
        x.i.mhi &= ~mask;
        x.i.mlo = 0;
    }
    *iptr = x.d;
    return value - x.d;
}

#endif /* IBMFLOAT */
#endif /* NO_FLOATING_POINT */

/* end of math.c */
