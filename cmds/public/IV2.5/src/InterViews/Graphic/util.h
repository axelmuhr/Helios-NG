/*
 * Various useful types and functions.
 */

#ifndef util_h
#define util_h

#include <InterViews/defs.h>
#include <bstring.h>
#include <math.h>

inline float fmax(float a, float b) { return (a >= b) ? a : b; }
inline float fmin(float a, float b) { return (a >= b) ? b : a; }

inline void exch (int& a, int& b) {
    int temp = a;
    a = b;
    b = temp;
}

overload square;
inline int square(int a) { return a *= a; }
inline float square(float a) { return a *= a; }

inline float degrees(float rad) { return rad * 180.0 / M_PI; }
inline float radians(float deg) { return deg * M_PI / 180.0; }

inline float Distance(Coord x0, Coord y0, Coord x1, Coord y1) {
    return sqrt(float(square(x0 - x1) + square(y0 - y1)));
}

inline void CopyArray (Coord* x, Coord* y, int n, Coord* newx, Coord* newy) {
    bcopy(x, newx, n * sizeof(Coord));
    bcopy(y, newy, n * sizeof(Coord));
}

inline void Midpoint (
    double x0, double y0, double x1, double y1, double& mx, double& my
) {
    mx = (x0 + x1) / 2.0;
    my = (y0 + y1) / 2.0;
}

inline void ThirdPoint (
    double x0, double y0, double x1, double y1, double& tx, double& ty
) {
    tx = (2*x0 + x1) / 3.0;
    ty = (2*y0 + y1) / 3.0;
}


#endif
