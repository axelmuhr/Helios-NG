/*
 * a rubber band for sizing windows
 */

#ifndef rubber_h
#define rubber_h

#include <InterViews/rubrect.h>

class SizingRect : public RubberRect {
protected:
    enum HMode { L, HC, R } hmode;
    enum VMode { B, VC, T } vmode;
    float fx;
    float fy;
public:
    SizingRect(
	Painter*, Canvas*, Coord x0, Coord y0, Coord x1, Coord y1,
	Coord ix, Coord iy, boolean snap = false, boolean constrain = true,
	Coord offx = 0, Coord offy = 0
    );
    void GetCurrent(Coord& x0, Coord& y0, Coord& x1, Coord& y1);
};

#endif
