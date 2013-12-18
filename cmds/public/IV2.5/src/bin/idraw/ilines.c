// $Header: ilines.c,v 1.6 88/09/24 15:05:43 interran Exp $
// implements class IFillMultiLine.

#include "ilines.h"
#include "ipaint.h"

// IFillMultiLine passes its arguments to FillPolygon.

IFillMultiLine::IFillMultiLine (Coord* x, Coord* y, int n, Graphic* gs)
: (x, y, n, gs) {
}

// contains returns true if the IFillMultiLine contains the given point
// unless the brush is an arrow or the pattern is the "none" pattern.

boolean IFillMultiLine::contains (PointObj& po, Graphic* gs) {
    boolean contains = false;
    IBrush* brush = (IBrush*) gs->GetBrush();
    IPattern* pattern = (IPattern*) gs->GetPattern();
    if (!brush->LeftArrow() && !brush->RightArrow() && !pattern->None()) {
	contains = FillPolygon::contains(po, gs);
    }
    return contains;
}

// intersects returns true if the IFillMultiLine intersects the given
// box unless the brush is an arrow or the pattern is the "none"
// pattern.

boolean IFillMultiLine::intersects (BoxObj& userb, Graphic* gs) {
    boolean intersects = false;
    IBrush* brush = (IBrush*) gs->GetBrush();
    IPattern* pattern = (IPattern*) gs->GetPattern();
    if (!brush->LeftArrow() && !brush->RightArrow() && !pattern->None()) {
	intersects = FillPolygon::intersects(userb, gs);
    }
    return intersects;
}

// draw draws the IFillMultiLine unless the brush is an arrow or the
// pattern is the "none" pattern (a IFillPolygon wouldn't have cared
// about the brush being an arrow).

void IFillMultiLine::draw (Canvas* c, Graphic* gs) {
    IBrush* brush = (IBrush*) gs->GetBrush();
    IPattern* pattern = (IPattern*) gs->GetPattern();
    if (!brush->LeftArrow() && !brush->RightArrow() && !pattern->None()) {
	FillPolygon::draw(c, gs);
    }
}
