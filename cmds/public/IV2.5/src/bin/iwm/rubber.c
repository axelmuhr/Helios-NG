/*
 * a rubber band suitable for sizing windows
 */

#include "rubber.h"

static const int MinTarget = 3;
static const float TargetFraction = 0.333;

SizingRect::SizingRect(
    Painter* p, Canvas* c, Coord x0, Coord y0, Coord x1, Coord y1,
    Coord ix, Coord iy, boolean snap, boolean constrain, Coord offx, Coord offy
) : (p, c, x0, y0, x1, y1, offx, offy) {
    float f = constrain ? TargetFraction : 0.5;
    int w = x1 - x0 + 1;
    int h = y1 - y0 + 1;
    int wtarget = min(max(round(w*f), MinTarget), w/2 + 1);
    int htarget = min(max(round(h*f), MinTarget), h/2 + 1);
    if (ix < x0 + wtarget) {
	hmode = R;
	fx = float(w) / float(x1 - ix);
    } else if (ix > x1 - wtarget) {
	hmode = L;
	fx = float(w) / float(ix - x0);
    } else {
	hmode = HC;
	fx = 0.0;
    }
    if (iy < y0 + htarget) {
	vmode = T;
	fy = float(h) / float(y1 - iy);
    } else if (iy > y1 - htarget) {
	vmode = B;
	fy = float(h) / float(iy - y0);
    } else {
	vmode = VC;
	fy = 0.0;
    }
    if (snap) {
	fx = 1.0;
	fy = 1.0;
	Track(ix, iy);
    }
}

void SizingRect::GetCurrent(Coord& x0, Coord& y0, Coord& x1, Coord& y1) {
    x0 = fixedx;
    y0 = fixedy;
    x1 = movingx;
    y1 = movingy;
    switch(hmode) {
	case L: x1 = x0 + int(fx * (trackx - x0)); break;
	case HC: break;
	case R: x0 = x1 - int(fx * (x1 - trackx)); break;
    }
    switch(vmode) {
	case B: y1 = y0 + int(fy * (tracky - y0)); break;
	case VC: break;
	case T: y0 = y1 - int(fy * (y1 - tracky)); break;
    }
};
