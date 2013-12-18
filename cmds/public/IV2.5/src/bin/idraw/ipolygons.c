// $Header: ipolygons.c,v 1.6 88/09/24 15:05:58 interran Exp $
// implements classes IFillRect and IFillPolygon.

#include "ipaint.h"
#include "ipolygons.h"

// IFillRect passes its arguments to FillRect.

IFillRect::IFillRect (Coord x0, Coord y0, Coord x1, Coord y1, Graphic* gs)
: (x0, y0, x1, y1, gs) {
}

// contains returns true if the IFillRect contains the given point
// unless the pattern is the "none" pattern.

boolean IFillRect::contains (PointObj& po, Graphic* gs) {
    boolean contains = false;
    IPattern* pattern = (IPattern*) gs->GetPattern();
    if (!pattern->None()) {
	contains = FillRect::contains(po, gs);
    }
    return contains;
}

// intersects returns true if the IFillRect intersects the given box
// unless the pattern is the "none" pattern.

boolean IFillRect::intersects (BoxObj& userb, Graphic* gs) {
    boolean intersects = false;
    IPattern* pattern = (IPattern*) gs->GetPattern();
    if (!pattern->None()) {
	intersects = FillRect::intersects(userb, gs);
    }
    return intersects;
}

// draw draws the IFillRect unless the pattern is the "none" pattern.

void IFillRect::draw (Canvas* c, Graphic* gs) {
    IPattern* pattern = (IPattern*) gs->GetPattern();
    if (!pattern->None()) {
	FillRect::draw(c, gs);
    }
}

// IFillPolygon passes its arguments to FillPolygon.

IFillPolygon::IFillPolygon (Coord* x, Coord* y, int n, Graphic* gs)
: (x, y, n, gs) {
}

// contains returns true if the IFillPolygon contains the given point
// unless the pattern is the "none" pattern.

boolean IFillPolygon::contains (PointObj& po, Graphic* gs) {
    boolean contains = false;
    IPattern* pattern = (IPattern*) gs->GetPattern();
    if (!pattern->None()) {
	contains = FillPolygon::contains(po, gs);
    }
    return contains;
}

// intersects returns true if the IFillPolygon intersects the given
// box unless the pattern is the "none" pattern.

boolean IFillPolygon::intersects (BoxObj& userb, Graphic* gs) {
    boolean intersects = false;
    IPattern* pattern = (IPattern*) gs->GetPattern();
    if (!pattern->None()) {
	intersects = FillPolygon::intersects(userb, gs);
    }
    return intersects;
}

// draw draws the IFillPolygon unless the pattern is the "none"
// pattern.

void IFillPolygon::draw (Canvas* c, Graphic* gs) {
    IPattern* pattern = (IPattern*) gs->GetPattern();
    if (!pattern->None()) {
	FillPolygon::draw(c, gs);
    }
}
