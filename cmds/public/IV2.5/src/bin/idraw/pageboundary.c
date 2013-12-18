// $Header: pageboundary.c,v 1.7 89/05/18 16:55:45 vlis Exp $
// implements class PageBoundary.

#include "pageboundary.h"

// PageBoundary stores the page's dimensions.

PageBoundary::PageBoundary (double w, double h, Graphic* gs)
: (0, 0, 0, 0, gs) {
    pagewidth = round(w);
    pageheight = round(h);
    border = round(0.05*inch);
}

// SetPortrait redefines the PageBoundary to fit a portrait page.

void PageBoundary::SetPortrait () {
    int nw = min(pagewidth, pageheight);
    int nh = max(pagewidth, pageheight);
    pagewidth = nw;
    pageheight = nh;
    uncacheParents();
}

// SetLandscape redefines the PageBoundary to fit a landscape page.

void PageBoundary::SetLandscape () {
    int nw = max(pagewidth, pageheight);
    int nh = min(pagewidth, pageheight);
    pagewidth = nw;
    pageheight = nh;
    uncacheParents();
}

// ToggleOrientation redefines the PageBoundary to fit the page's
// other orientation.  N.B.: Rotating and translating a boundary
// rather than redefining its dimensions results in a poor looking
// boundary because the X10 Xqdss server doesn't display extremely
// thin polygons very well when they're rotated and translated.

void PageBoundary::ToggleOrientation () {
    int nw = pageheight;
    int nh = pagewidth;
    pagewidth = nw;
    pageheight = nh;
    uncacheParents();
}

// getExtent returns the PageBoundary's extent.

void PageBoundary::getExtent (float& l, float& b, float& cx, float& cy,
float& tol, Graphic* gs) {
    l = 0;
    b = 0;
    cx = pagewidth/2;
    cy = pageheight/2;
    transform(l, b, l, b, gs);
    transform(cx, cy, cx, cy, gs);
    tol = 0;
}

// draw draws the PageBoundary.

void PageBoundary::draw (Canvas* c, Graphic* gs) {
    update(gs);
    pFillRect(c, 0, 0, border, pageheight);
    pFillRect(c, 0, 0, pagewidth, border);
    pFillRect(c, pagewidth - border, 0, pagewidth, pageheight);
    pFillRect(c, 0, pageheight - border, pagewidth, pageheight);
}

void PageBoundary::drawClipped (
    Canvas* c, Coord l, Coord b, Coord r, Coord t, Graphic* gs
) {
    Coord x0, y0, x1, y1, x2, y2;
    BoxObj clipBox(l, b, r, t);
    
    transform(0, 0, x0, y0, gs);
    transform(border, pageheight, x1, y1, gs);
    transform(pagewidth, border, x2, y2, gs);

    BoxObj lbox(x0, y0, x1, y1);
    BoxObj bbox(x0, y0, x2, y2);
    
    transform(pagewidth - border, 0, x0, y0, gs);
    transform(0, pageheight - border, x1, y1, gs);
    transform(pagewidth, pageheight, x2, y2, gs);

    BoxObj rbox(x0, y0, x2, y2);
    BoxObj tbox(x1, y1, x2, y2);

    update(gs);
    if (lbox.Intersects(clipBox)) {
	pFillRect(c, 0, 0, border, pageheight);
    }
    if (bbox.Intersects(clipBox)) {
	pFillRect(c, 0, 0, pagewidth, border);
    }
    if (rbox.Intersects(clipBox)) {
	pFillRect(c, pagewidth - border, 0, pagewidth, pageheight);
    }
    if (tbox.Intersects(clipBox)) {
	pFillRect(c, 0, pageheight - border, pagewidth, pageheight);
    }
}
