/*
 * Common definitions for all of InterViews.
 */

#ifndef iv_defs_h
#define iv_defs_h

enum boolean { false, true };

enum Alignment {
    TopLeft, TopCenter, TopRight,
    CenterLeft, Center, CenterRight,
    BottomLeft, BottomCenter, BottomRight,
    Left, Right, Top, Bottom, HorizCenter, VertCenter
};

enum TextStyle {
    Plain = 0x0,
    Boldface = 0x1,
    Underlined = 0x2,
    Reversed = 0x4,
    Outlined = 0x8
};

typedef int Coord;

extern double inch, inches, cm, point, points;
static const int pixels = 1;

#define nil 0

inline int min (int a, int b) {
    return a < b ? a : b;
}

inline int max (int a, int b) {
    return a > b ? a : b;
}

inline int round (double x) {
    return x > 0 ? int(x+0.5) : -int(-x+0.5);
}

#endif
