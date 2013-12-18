// $Header: slellipses.c,v 1.6 88/09/24 15:08:00 interran Exp $
// implements classes EllipseSelection and CircleSelection.

#include "iellipses.h"
#include "slellipses.h"
#include <stream.h>

// EllipseSelection creates the ellipse's filled interior and outline.

EllipseSelection::EllipseSelection (Coord x0, Coord y0, int rx, int ry,
Graphic* gs) : (gs) {
    Append(new IFillEllipse(x0, y0, rx, ry));
    Append(new Ellipse(x0, y0, rx, ry));
}

// EllipseSelection reads data to initialize its graphic state and
// create its filled interior and outline.

EllipseSelection::EllipseSelection (istream& from, State* state) : (nil) {
    ReadGS(from, state);
    Skip(from);
    Coord x0, y0;
    int rx, ry;
    from >> x0 >> y0 >> rx >> ry;
    Append(new IFillEllipse(x0, y0, rx, ry));
    Append(new Ellipse(x0, y0, rx, ry));
}

// Copy returns a copy of the EllipseSelection.

Graphic* EllipseSelection::Copy () {
    Coord x0, y0;
    int rx, ry;
    GetOriginal(x0, y0, rx, ry);
    return new EllipseSelection(x0, y0, rx, ry, this);
}

// GetOriginal returns the center point and the x and y radii lengths
// that were passed to the EllipseSelection's constructor.

void EllipseSelection::GetOriginal (Coord& x0, Coord& y0, int& rx, int& ry) {
    ((Ellipse*) Last())->GetOriginal(x0, y0, rx, ry);
}

// WriteData writes the EllipseSelection's data and Postscript code to
// draw it.

void EllipseSelection::WriteData (ostream& to) {
    Coord x0, y0;
    int rx, ry;
    GetOriginal(x0, y0, rx, ry);
    to << "Begin " << startdata << " Elli\n";
    WriteGS(to);
    to << startdata << "\n";
    to << x0 << " " << y0 << " " << rx << " " << ry << " Elli\n";
    to << "End\n\n";
}

// CircleSelection creates the circle's filled interior and outline.

CircleSelection::CircleSelection (Coord x0, Coord y0, int r, Graphic* gs)
: (gs) {
    Append(new IFillCircle(x0, y0, r));
    Append(new Circle(x0, y0, r));
}

// CircleSelection reads data to initialize its graphic state and
// create its filled interior and outline.

CircleSelection::CircleSelection (istream& from, State* state) : (nil) {
    ReadGS(from, state);
    Skip(from);
    Coord x0, y0;
    int r;
    from >> x0 >> y0 >> r;
    Append(new IFillCircle(x0, y0, r));
    Append(new Circle(x0, y0, r));
}

// Copy returns a copy of the CircleSelection.

Graphic* CircleSelection::Copy () {
    Coord x0, y0;
    int r;
    GetOriginal(x0, y0, r);
    return new CircleSelection(x0, y0, r, this);
}

// GetOriginal returns the center point and radius length that were
// passed to the CircleSelection's constructor.

void CircleSelection::GetOriginal (Coord& x0, Coord& y0, int& r) {
    ((Circle*) Last())->GetOriginal(x0, y0, r, r);
}

// WriteData writes the CircleSelection's data and Postscript code to
// draw it.

void CircleSelection::WriteData (ostream& to) {
    Coord x0, y0;
    int r;
    GetOriginal(x0, y0, r);
    to << "Begin " << startdata << " Circ\n";
    WriteGS(to);
    to << startdata << "\n";
    to << x0 << " " << y0 << " " << r << " Circ\n";
    to << "End\n\n";
}
