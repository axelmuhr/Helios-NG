// $Header: sllines.c,v 1.9 89/05/18 16:55:49 vlis Exp $
// implements classes LineSelection and MultiLineSelection.

#include "ilines.h"
#include "rubbands.h"
#include "sllines.h"
#include <stream.h>

// LineSelection creates its line.

LineSelection::LineSelection (Coord x0, Coord y0, Coord x1, Coord y1,
Graphic* gs) : (gs) {
    myname = "Line";
    line = new Line(x0, y0, x1, y1);
}

// LineSelection reads data to initialize its graphic state and create
// its line.

LineSelection::LineSelection (istream& from, State* state) : (nil) {
    myname = "Line";
    line = nil;
    ReadGS(from, state);
    Skip(from);
    Coord x0, y0, x1, y1;
    from >> x0 >> y0 >> x1 >> y1;
    line = new Line(x0, y0, x1, y1);
}

// Copy returns a copy of the LineSelection.

Graphic* LineSelection::Copy () {
    Coord x0, y0, x1, y1;
    GetOriginal2(x0, y0, x1, y1);
    return new LineSelection(x0, y0, x1, y1, this);
}

// GetOriginal2 returns the two endpoints that were passed to the
// LineSelection's constructor.

void LineSelection::GetOriginal2 (Coord& x0, Coord& y0, Coord& x1, Coord& y1) {
    ((Line*) line)->GetOriginal(x0, y0, x1, y1);
}

// GetOriginal returns within two arrays the two endpoints that were
// passed to the LineSelection's constructor.

void LineSelection::GetOriginal (Coord*& x, Coord*& y, int& n) {
    x = new Coord[2];
    y = new Coord[2];
    n = 2;
    GetOriginal2(x[0], y[0], x[1], y[1]);
}

// WriteData writes the LineSelection's data and Postscript code to
// draw it.

void LineSelection::WriteData (ostream& to) {
    Coord x0, y0, x1, y1;
    GetOriginal2(x0, y0, x1, y1);
    to << "Begin " << startdata << " Line\n";
    WriteGS(to);
    to << startdata << "\n";
    to << x0 << " " << y0 << " " << x1 << " " << y1 << " Line\n";
    to << "End\n\n";
}

// CreateRubberVertex creates and returns the right kind of
// RubberVertex to represent the LineSelection's shape.

RubberVertex* LineSelection::CreateRubberVertex (Coord* x, Coord* y, int n,
int rubpt) {
    return new RubberMultiLine(nil, nil, x, y, n, rubpt);
}

// CreateReshapedCopy creates and returns a reshaped copy of itself
// using the passed points and its graphic state.

Selection* LineSelection::CreateReshapedCopy (Coord* x, Coord* y, int) {
    return new LineSelection(x[0], y[0], x[1], y[1], this);
}

// uncacheChildren uncaches the graphic's components' extents.

void LineSelection::uncacheChildren () {
    if (line != nil) {
	uncacheExtentGraphic(line);
    }
}

// getExtent returns the graphic's extent including a tolerance for
// the arrowheads.

void LineSelection::getExtent (float& l, float& b, float& cx, float& cy,
float& tol, Graphic* gs) {
    getExtentGraphic(line, l, b, cx, cy, tol, gs);
    tol = MergeArrowHeadTol(tol, gs);
}

// contains returns true if the graphic contains the point.

boolean LineSelection::contains (PointObj& po, Graphic* gs) {
    BoxObj b;
    getBox(b, gs);
    if (b.Contains(po)) {
	Coord x0, y0, x1, y1;
	GetOriginal2(x0, y0, x1, y1);

	if (containsGraphic(line, po, gs)) {
	    return true;
	} else if (LeftAcont(x0, y0, x1, y1, po, gs)) {
	    return true;
	} else if (RightAcont(x1, y1, x0, y0, po, gs)) {
	    return true;
	}
    }
    return false;
}

// intersects returns true if the graphic intersects the box.

boolean LineSelection::intersects (BoxObj& userb, Graphic* gs) {
    BoxObj b;
    getBox(b, gs);
    if (b.Intersects(userb)) {
	Coord x0, y0, x1, y1;
	GetOriginal2(x0, y0, x1, y1);

	if (intersectsGraphic(line, userb, gs)) {
	    return true;
	} else if (LeftAints(x0, y0, x1, y1, userb, gs)) {
	    return true;
	} else if (RightAints(x1, y1, x0, y0, userb, gs)) {
	    return true;
	}
    }
    return false;
}

// draw draws the graphic.

void LineSelection::draw (Canvas* c, Graphic* gs) {
    drawGraphic(line, c, gs);
    Coord x0, y0, x1, y1;
    GetOriginal2(x0, y0, x1, y1);
    drawLeftA(x0, y0, x1, y1, c, gs);
    drawRightA(x1, y1, x0, y0, c, gs);
}

// drawClipped draws the graphic if it intersects the clipping box.

void LineSelection::drawClipped (Canvas* c, Coord l, Coord b, Coord r,
Coord t, Graphic* gs) {
    BoxObj box;
    getBox(box, gs);

    BoxObj clipBox(l, b, r, t);
    if (clipBox.Intersects(box)) {
	draw(c, gs);
    }
}

// Skew comments/code ratio to work around cpp bug
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

// MultiLineSelection creates its components.

MultiLineSelection::MultiLineSelection (Coord* x, Coord* y, int n, Graphic* gs)
: (gs) {
    Init(x, y, n);
}

// MultiLineSelection reads data to initialize its graphic state and
// create its components.

MultiLineSelection::MultiLineSelection (istream& from, State* state) : (nil) {
    multiline = nil;
    ReadGS(from, state);
    Coord* x;
    Coord* y;
    int n;
    ReadPoints(from, x, y, n);
    Init(x, y, n);
}

// Copy returns a copy of the MultiLineSelection.

Graphic* MultiLineSelection::Copy () {
    Coord* x;
    Coord* y;
    int n;
    GetOriginal(x, y, n);
    Graphic* copy = new MultiLineSelection(x, y, n, this);
    delete x;
    delete y;
    return copy;
}

// GetOriginal returns the vertices that were passed to the
// MultiLineSelection's constructor.

void MultiLineSelection::GetOriginal (Coord*& x, Coord*& y, int& n) {
    ((MultiLine*) multiline)->GetOriginal(x, y, n);
}

// Init creates the graphic's components and stores the arrowheads'
// endpoints.

void MultiLineSelection::Init (Coord* x, Coord* y, int n) {
    myname = "MLine";
    ifillmultiline = new IFillMultiLine(x, y, n);
    multiline = new MultiLine(x, y, n);
    lx0 = x[0];
    ly0 = y[0];
    lx1 = x[1];
    ly1 = y[1];
    rx0 = x[n-1];
    ry0 = y[n-1];
    rx1 = x[n-2];
    ry1 = y[n-2];
}

// CreateRubberVertex creates and returns the right kind of
// RubberVertex to represent the MultiLineSelection's shape.

RubberVertex* MultiLineSelection::CreateRubberVertex (Coord* x, Coord* y,
int n, int rubpt) {
    return new RubberMultiLine(nil, nil, x, y, n, rubpt);
}

// CreateReshapedCopy creates and returns a reshaped copy of itself
// using the passed points and its graphic state.

Selection* MultiLineSelection::CreateReshapedCopy (Coord* x, Coord* y, int n) {
    return new MultiLineSelection(x, y, n, this);
}

// uncacheChildren uncaches the graphic's components' extents.

void MultiLineSelection::uncacheChildren () {
    if (multiline != nil) {
	uncacheExtentGraphic(ifillmultiline);
	uncacheExtentGraphic(multiline);
    }
}

// getExtent returns the graphic's extent including a tolerance for
// the arrowheads.

void MultiLineSelection::getExtent (float& l, float& b, float& cx, float& cy,
float& tol, Graphic* gs) {
    Extent e;
    if (extentCached()) {
	getCachedExtent(e.left, e.bottom, e.cx, e.cy, e.tol);
    } else {
	FullGraphic gstmp;
	concatGSGraphic(ifillmultiline, this, gs, &gstmp);
	getExtentGraphic(
            ifillmultiline, e.left, e.bottom, e.cx, e.cy, e.tol, &gstmp
        );
	Extent te;
	concatGSGraphic(multiline, this, gs, &gstmp);
	getExtentGraphic(
            multiline, te.left, te.bottom, te.cx, te.cy, te.tol, &gstmp
        );
	e.Merge(te);
	cacheExtent(e.left, e.bottom, e.cx, e.cy, e.tol);
    }
    float right = 2*e.cx - e.left;
    float top = 2*e.cy - e.bottom;
    float dummy = 0;
    transformRect(e.left, e.bottom, right, top, l, b, dummy, dummy, gs);
    transform(e.cx, e.cy, cx, cy, gs);
    tol = MergeArrowHeadTol(e.tol, gs);
}

// contains returns true if the graphic contains the point.

boolean MultiLineSelection::contains (PointObj& po, Graphic* gs) {
    BoxObj b;
    getBox(b, gs);
    if (b.Contains(po)) {
	if (containsGraphic(ifillmultiline, po, gs)) {
	    return true;
	} else if (containsGraphic(multiline, po, gs)) {
	    return true;
	} else if (LeftAcont(lx0, ly0, lx1, ly1, po, gs)) {
	    return true;
	} else if (RightAcont(rx0, ry0, rx1, ry1, po, gs)) {
	    return true;
	}
    }
    return false;
}

// intersects returns true if the graphic intersects the box.

boolean MultiLineSelection::intersects (BoxObj& userb, Graphic* gs) {
    BoxObj b;
    getBox(b, gs);
    if (b.Intersects(userb)) {
	if (intersectsGraphic(ifillmultiline, userb, gs)) {
	    return true;
	} else if (intersectsGraphic(multiline, userb, gs)) {
	    return true;
	} else if (LeftAints(lx0, ly0, lx1, ly1, userb, gs)) {
	    return true;
	} else if (RightAints(rx0, ry0, rx1, ry1, userb, gs)) {
	    return true;
	}
    }
    return false;
}

// draw draws the graphic.

void MultiLineSelection::draw (Canvas* c, Graphic* gs) {
    drawGraphic(ifillmultiline, c, gs);
    drawGraphic(multiline, c, gs);
    drawLeftA(lx0, ly0, lx1, ly1, c, gs);
    drawRightA(rx0, ry0, rx1, ry1, c, gs);
}

// drawClipped draws the graphic if it intersects the clipping box.

void MultiLineSelection::drawClipped (Canvas* c, Coord l, Coord b, Coord r,
Coord t, Graphic* gs) {
    BoxObj box;
    getBox(box, gs);

    BoxObj clipBox(l, b, r, t);
    if (clipBox.Intersects(box)) {
	draw(c, gs);
    }
}
