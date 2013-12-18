#ifndef shape_h
#define shape_h

#include <InterViews/defs.h>

/*
 * Constants for defining "infinite" stretchability or shrinkability.
 */

static const int hfil = 1000000;
static const int vfil = 1000000;

class Shape {
public:
    int width, height;		/* natural dimensions */
    int hstretch, vstretch;	/* stretchability */
    int hshrink, vshrink;	/* shrinkability */
    int aspect;			/* desired aspect ratio, 0 means don't care */
    int hunits, vunits;		/* allocate in multiples */

    Shape();
    ~Shape();

    void Square(int);
    void Rect(int w, int h);
    void Rigid(int hshr = 0, int hstr = 0, int vshr = 0, int vstr = 0);
    void SetUndefined();
    boolean Defined();
    boolean Undefined () { return !Defined(); }
};

#endif
