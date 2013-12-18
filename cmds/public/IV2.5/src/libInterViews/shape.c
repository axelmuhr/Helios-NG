/*
 * Shape constructors
 */

#include <InterViews/shape.h>

Shape::Shape () {
    SetUndefined();
    Rigid(hfil, hfil, vfil, vfil);
    aspect = 0;
    hunits = 1;
    vunits = 1;
}

Shape::~Shape () {
    /* nothing to do for now */
}

void Shape::Square (int side) {
    width = side; height = side;
    Rigid();
    aspect = 1;
}

void Shape::Rect (int w, int h) {
    width = w; height = h;
    Rigid();
}

void Shape::Rigid (int hshr, int hstr, int vshr, int vstr) {
    hshrink = hshr;
    hstretch = hstr;
    vshrink = vshr;
    vstretch = vstr;
}

void Shape::SetUndefined () {
    width = 0;
    height = 0;
}

boolean Shape::Defined () {
    return width != 0;
}
