// $Header: rubbands.c,v 1.8 89/03/19 12:18:11 interran Exp $
// implements classes IStretchingRect, RubberMultiLine, and RubberPolygon.

#include "rubbands.h"
#include <InterViews/painter.h>

// abs returns the integer's magnitude.

inline int abs (int a) {
    return a > 0 ? a : -a;
}

// IStretchingRect starts with no defined side yet.

IStretchingRect::IStretchingRect (Painter* p, Canvas* c, Coord x0, Coord y0,
Coord x1, Coord y1, Coord offx, Coord offy)
: (p, c, x0, y0, x1, y1, RightSide, offx, offy) {
    firsttime = true;
    undefinedside = true;
    cx = (fixedx + movingx) / 2;
    cy = (fixedy + movingy) / 2;
}

// Track stores the point on the first call, decides which side
// becomes the stretching side on the next call or two, and draws the
// rectangle on all following calls.

void IStretchingRect::Track (Coord x, Coord y) {
    if (firsttime) {
	firsttime = false;
	origx = x;
	origy = y;
    } else if (undefinedside) {
	DefineSide(x, y);
    } else {
	StretchingRect::Track(x, y);
    }
}

// DefineSide picks the side to stretch after the motion of the
// tracking point becomes sufficiently unambiguous.

static const Coord THRESHOLD = 2;

void IStretchingRect::DefineSide (Coord x, Coord y) {
    Coord dx = abs(x - origx);
    Coord dy = abs(y - origy);
    Coord xydiff = abs(dx - dy);
    if (xydiff >= THRESHOLD) {
	undefinedside = false;
	if (dx > dy) {
	    if (x > cx) {
		side = RightSide;
	    } else {
		side = LeftSide;
	    }
	} else {
	    if (y > cy) {
		side = TopSide;
	    } else {
		side = BottomSide;
	    }
	}
    }
}

// CurrentSide returns the side that the user is dragging.

Alignment IStretchingRect::CurrentSide () {
    Alignment s = Left;
    switch (side) {
    case LeftSide:
	s = Left;
	break;
    case RightSide:
	s = Right;
	break;
    case TopSide:
	s = Top;
	break;
    case BottomSide:
	s = Bottom;
	break;
    }
    return s;
}

// RubberMultiLine passes its arguments to RubberVertex.

RubberMultiLine::RubberMultiLine (
    Painter* p, Canvas* c, Coord px[], Coord py[], int n, int pt,
    Coord offx, Coord offy
) : (p, c, px, py, n, pt, offx, offy) {
    /* nothing else to do */
}

// Draw draws only the one or two line segments attached to the rubber
// vertex.

void RubberMultiLine::Draw () {
    if (x == nil || y == nil) {
        return;
    }
    if (!drawn) {
	int before = rubberPt - 1;
	if (before >= 0) {
	    Coord x0 = x[before];
	    Coord y0 = y[before];
	    output->Line(canvas, x0+offx, y0+offy, trackx+offx, tracky+offy);
	}
	int after = rubberPt + 1;
	if (after <= count - 1) {
	    Coord x1 = x[after];
	    Coord y1 = y[after];
	    output->Line(canvas, trackx+offx, tracky+offy, x1+offx, y1+offy);
	}
	drawn = true;
    }
}

// RubberPolygon passes its arguments to RubberVertex.

RubberPolygon::RubberPolygon (
    Painter* p, Canvas* c, Coord px[], Coord py[], int n, int pt,
    Coord offx, Coord offy
) : (p, c, px, py, n, pt, offx, offy) {
    /* nothing else to do */
}

// Draw draws only the two line segments attached to the rubber
// vertex.

void RubberPolygon::Draw () {
    if (x == nil || y == nil) {
        return;
    }
    if (!drawn) {
	int before = (rubberPt > 0) ? rubberPt - 1 : count - 1;
	Coord x0 = x[before];
	Coord y0 = y[before];
	output->Line(canvas, x0+offx, y0+offy, trackx+offx, tracky+offy);
	if (count > 2) {
	    int after = (rubberPt < count - 1) ? rubberPt + 1 : 0;
	    Coord x1 = x[after];
	    Coord y1 = y[after];
	    output->Line(canvas, trackx+offx, tracky+offy, x1+offx, y1+offy);
	}
	drawn = true;
    }
}
