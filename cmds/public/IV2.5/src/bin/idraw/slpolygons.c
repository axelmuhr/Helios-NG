// $Header: slpolygons.c,v 1.7 89/05/05 11:39:12 linton Exp $
// implements classes RectSelection and PolygonSelection.

#include "ipolygons.h"
#include "rubbands.h"
#include "slpolygons.h"
#include <stream.h>

// RectSelection creates the rectangle's filled interior and outline.

RectSelection::RectSelection (Coord l, Coord b, Coord r, Coord t, Graphic* gs)
: (gs) {
    myname = "Rect";
    Append(new IFillRect(l, b, r, t));
    Append(new Rect(l, b, r, t));
}

// RectSelection reads data to initialize its graphic state and create
// its filled interior and outline.

RectSelection::RectSelection (istream& from, State* state) : (nil) {
    myname = "Rect";
    ReadGS(from, state);
    Skip(from);
    Coord l, b, r, t;
    from >> l >> b >> r >> t;
    Append(new IFillRect(l, b, r, t));
    Append(new Rect(l, b, r, t));
}

// Copy returns a copy of the RectSelection.

Graphic* RectSelection::Copy () {
    Coord l, b, r, t;
    GetOriginal2(l, b, r, t);
    return new RectSelection(l, b, r, t, this);
}

// GetOriginal2 returns the two corners that were passed to the
// RectSelection's constructor.

void RectSelection::GetOriginal2 (Coord& l, Coord& b, Coord& r, Coord& t) {
    ((Rect*) Last())->GetOriginal(l, b, r, t);
}

// GetOriginal returns the two corners that were passed to the
// RectSelection's constructor plus the other two opposite corners.

void RectSelection::GetOriginal (Coord*& x, Coord*& y, int& n) {
    x = new Coord[4];
    y = new Coord[4];
    n = 4;
    GetOriginal2(x[0], y[0], x[2], y[2]);
    x[1] = x[0];
    y[1] = y[2];
    x[3] = x[2];
    y[3] = y[0];
}

// WriteData writes the RectSelection's data and Postscript code to
// draw it.

void RectSelection::WriteData (ostream& to) {
    Coord l, b, r, t;
    GetOriginal2(l, b, r, t);
    to << "Begin " << startdata << " Rect\n";
    WriteGS(to);
    to << startdata << "\n";
    to << l << " " << b << " " << r << " " << t << " Rect\n";
    to << "End\n\n";
}

// CreateRubberVertex creates and returns the right kind of
// RubberVertex to represent the RectSelection's shape.

RubberVertex* RectSelection::CreateRubberVertex (Coord* x, Coord* y,
int n, int rubpt) {
    return new RubberPolygon(nil, nil, x, y, n, rubpt);
}

// CreateReshapedCopy creates and returns a reshaped copy of itself
// using the passed points and its graphic state.  It returns a
// PolygonSelection because the points may not shape a rect any more.

Selection* RectSelection::CreateReshapedCopy (Coord* x, Coord* y, int n) {
    return new PolygonSelection(x, y, n, this);
}

// PolygonSelection creates the polygon's filled interior and outline.

PolygonSelection::PolygonSelection (Coord* x, Coord* y, int n, Graphic* gs)
: (gs) {
    myname = "Poly";
    Append(new IFillPolygon(x, y, n));
    Append(new Polygon(x, y, n));
}

// PolygonSelection reads data to initialize its graphic state and
// create its filled interior and outline.

PolygonSelection::PolygonSelection (istream& from, State* state) : (nil) {
    myname = "Poly";
    ReadGS(from, state);
    Coord* x;
    Coord* y;
    int n;
    ReadPoints(from, x, y, n);
    Append(new IFillPolygon(x, y, n));
    Append(new Polygon(x, y, n));
}

// Copy returns a copy of the PolygonSelection.

Graphic* PolygonSelection::Copy () {
    Coord* x;
    Coord* y;
    int n;
    GetOriginal(x, y, n);
    Graphic* copy = new PolygonSelection(x, y, n, this);
    delete x;
    delete y;
    return copy;
}

// GetOriginal returns the vertices that were passed to the
// PolygonSelection's constructor.

void PolygonSelection::GetOriginal (Coord*& x, Coord*& y, int& n) {
    ((Polygon*) Last())->GetOriginal(x, y, n);
}

// CreateRubberVertex creates and returns the right kind of
// RubberVertex to represent the PolygonSelection's shape.

RubberVertex* PolygonSelection::CreateRubberVertex (Coord* x, Coord* y,
int n, int rubpt) {
    return new RubberPolygon(nil, nil, x, y, n, rubpt);
}

// CreateReshapedCopy creates and returns a reshaped copy of itself
// using the passed points and its graphic state.

Selection* PolygonSelection::CreateReshapedCopy (Coord* x, Coord* y, int n) {
    return new PolygonSelection(x, y, n, this);
}
