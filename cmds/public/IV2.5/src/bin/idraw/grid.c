// $Header: grid.c,v 1.12 89/05/18 16:55:19 vlis Exp $
// implements class Grid.

#include "grid.h"
#include <InterViews/canvas.h>
#include <InterViews/transformer.h>

// Grid starts out with gridding disabled, spacing set to the default
// value, visibility disabled, and enough storage for the grid points.

static Coord dummy;

Grid::Grid (double w, double h, Graphic* gs) : (&dummy, &dummy, 1, gs) {
    gridding = false;
    pointsin = 0.;
    spacing = 0;
    visibility = false;
    pagewidth = round(w);
    pageheight = round(h);

    SetSpacing(GRID_DEFAULTSPACING);
}

// Constrain replaces the given point with the closest grid point if
// gridding has been enabled.

void Grid::Constrain (Coord& x, Coord& y) {
    if (gridding) {
	Transformer parents;

	parentXform(parents);
	parents.InvTransform(x, y);
	x = spacing * round(float(x)/spacing);
	y = spacing * round(float(y)/spacing);
	parents.Transform(x, y);
    }
}

// SetPortrait redefines the grid points to fit a portrait page.

void Grid::SetPortrait () {
    int nw = min(pagewidth, pageheight);
    int nh = max(pagewidth, pageheight);
    pagewidth = nw;
    pageheight = nh;
    uncacheParents();
}

// SetLandscape redefines the grid points to fit a landscape page.

void Grid::SetLandscape () {
    int nw = max(pagewidth, pageheight);
    int nh = min(pagewidth, pageheight);
    pagewidth = nw;
    pageheight = nh;
    uncacheParents();
}

// ToggleOrientation redefines the grid points to fit the page's
// other orientation.

void Grid::ToggleOrientation () {
    int nw = pageheight;
    int nh = pagewidth;
    pagewidth = nw;
    pageheight = nh;
    uncacheParents();
}

// SetSpacing changes the spacing between the grid points, which also
// requires reallocating the number of points in the grid.  SetSpacing
// multiplies the argument by printer's points to get the true spacing.

void Grid::SetSpacing (double p) {
    if (pointsin != p) {
	pointsin = p;
	spacing = round(pointsin * points);
	delete x;
	delete y;
	count = (pagewidth/spacing + 1) * (pageheight/spacing + 1);
	x = new Coord[count];
	y = new Coord[count];
    }
}

// getExtent returns the grid's extent.

void Grid::getExtent (float& l, float& b, float& cx, float& cy, float& tol,
Graphic* gs) {
    l = 0;
    b = 0;
    cx = pagewidth/2;
    cy = pageheight/2;
    transform(l, b, l, b, gs);
    transform(cx, cy, cx, cy, gs);
    tol = 0;
}

// draw just calls drawClipped to cover the canvas with grid points.

void Grid::draw (Canvas* c, Graphic* gs) {
    Coord xmax = c->Width() + 1;
    Coord ymax = c->Height() + 1;
    drawClipped(c, 0, 0, xmax, ymax, gs);
}

// drawClipped grids the given area with points if visibility has been
// enabled.  It sents the points in chunks of MAXCHUNK points each
// because the MIT X11R3 Sun server will return a protocol error if
// idraw sends more than 16381 points at a time.

void Grid::drawClipped (Canvas* c, Coord l, Coord b, Coord r, Coord t,
Graphic* gs) {
    if (visibility && gs->GetBrush()->Width() != NO_WIDTH) {
	update(gs);
	int ntotal = DefinePoints(l, b, r, t);
	int nsent = 0;
	while (nsent < ntotal) {
	    const int MAXCHUNK = 2000;
	    int nchunk = min(MAXCHUNK, ntotal - nsent);
	    pMultiPoint(c, &x[nsent], &y[nsent], nchunk);
	    nsent += nchunk;
	}
    }
}

// DefinePoints defines just enough points to grid the given area.

int Grid::DefinePoints (Coord l, Coord b, Coord r, Coord t) {
    Transformer parents;
    parentXform(parents);
    parents.InvTransform(l, b);
    parents.InvTransform(r, t);
    l = spacing * round(float(l)/spacing);
    b = spacing * round(float(b)/spacing);
    r = spacing * round(float(r)/spacing);
    t = spacing * round(float(t)/spacing);
    Coord minix = max(0, l);
    Coord miniy = max(0, b);
    Coord maxix = min(r, pagewidth);
    Coord maxiy = min(t, pageheight);

    float x0, y0, x1, y1;
    parents.Transform(0., 0., x0, y0);
    parents.Transform(1., 1., x1, y1);
    float magnif = x1 - x0;	// square scaling assumed
    int tspacing = max(spacing, round(spacing/magnif));
    int n = 0;
    for (Coord ix = minix; ix <= maxix && n < count; ix += tspacing) {
	for (Coord iy = miniy; iy <= maxiy && n < count; iy += tspacing) {
	    x[n] = ix;
	    y[n] = iy;
	    ++n;
	}
    }
    return n;
}
