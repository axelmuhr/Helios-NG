// $Header: iellipses.c,v 1.6 88/09/24 15:05:37 interran Exp $
// implements classes IFillEllipse and IFillCircle.

#include "iellipses.h"
#include "ipaint.h"

// IFillEllipse passes its arguments to FillEllipse.

IFillEllipse::IFillEllipse (Coord x0, Coord y0, int rx, int ry, Graphic* gs)
: (x0, y0, rx, ry, gs) {
}

// contains returns true if the IFillEllipse contains the given point
// unless the pattern is the "none" pattern.

boolean IFillEllipse::contains (PointObj& po, Graphic* gs) {
    boolean contains = false;
    IPattern* pattern = (IPattern*) gs->GetPattern();
    if (!pattern->None()) {
	contains = FillEllipse::contains(po, gs);
    }
    return contains;
}

// intersects returns true if the IFillEllipse intersects the given
// box unless the pattern is the "none" pattern.

boolean IFillEllipse::intersects (BoxObj& userb, Graphic* gs) {
    boolean intersects = false;
    IPattern* pattern = (IPattern*) gs->GetPattern();
    if (!pattern->None()) {
	intersects = FillEllipse::intersects(userb, gs);
    }
    return intersects;
}

// draw draws the IFillEllipse unless the pattern is the "none" pattern.

void IFillEllipse::draw (Canvas* c, Graphic* gs) {
    IPattern* pattern = (IPattern*) gs->GetPattern();
    if (!pattern->None()) {
	FillEllipse::draw(c, gs);
    }
}

// IFillCircle passes its arguments to FillCircle.

IFillCircle::IFillCircle (Coord x0, Coord y0, int r, Graphic* gs)
: (x0, y0, r, gs) {
}

// contains returns true if the IFillCircle contains the given point
// unless the pattern is the "none" pattern.

boolean IFillCircle::contains (PointObj& po, Graphic* gs) {
    boolean contains = false;
    IPattern* pattern = (IPattern*) gs->GetPattern();
    if (!pattern->None()) {
	contains = FillCircle::contains(po, gs);
    }
    return contains;
}

// intersects returns true if the IFillCircle intersects the given box
// unless the pattern is the "none" pattern.

boolean IFillCircle::intersects (BoxObj& userb, Graphic* gs) {
    boolean intersects = false;
    IPattern* pattern = (IPattern*) gs->GetPattern();
    if (!pattern->None()) {
	intersects = FillCircle::intersects(userb, gs);
    }
    return intersects;
}

// draw draws the IFillCircle unless the pattern is the "none"
// pattern.

void IFillCircle::draw (Canvas* c, Graphic* gs) {
    IPattern* pattern = (IPattern*) gs->GetPattern();
    if (!pattern->None()) {
	FillCircle::draw(c, gs);
    }
}
