// $Header: page.c,v 1.2 89/05/18 16:56:14 vlis Exp $
// implements class Page.

#include "page.h"
#include <InterViews/transformer.h>

// Page stores the page's dimensions.

Page::Page (double w, double h, Graphic* gs) : (gs) {
    pagewidth = round(w);
    pageheight = round(h);
}

// SetPortrait redefines the dimensions to fit a portrait page.

void Page::SetPortrait () {
    int nw = min(pagewidth, pageheight);
    int nh = max(pagewidth, pageheight);
    pagewidth = nw;
    pageheight = nh;
    uncacheParents();
}

// SetLandscape redefines the dimensions to fit a landscape page.

void Page::SetLandscape () {
    int nw = max(pagewidth, pageheight);
    int nh = min(pagewidth, pageheight);
    pagewidth = nw;
    pageheight = nh;
    uncacheParents();
}

// ToggleOrientation redefines the dimensions to fit the page's
// other orientation.

void Page::ToggleOrientation () {
    int nw = pageheight;
    int nh = pagewidth;
    pagewidth = nw;
    pageheight = nh;
    uncacheParents();
}

// getExtent returns the Page's dimensions as its extent instead of
// computing the extent of its children (grid, drawing, and page
// boundary) so the Panner will always show the page's extent instead
// of the drawing's extent even when the drawing overlaps the page
// boundary.

void Page::getExtent (float& l, float& b, float& cx, float& cy,
float& tol, Graphic* gs) {
    l = 0;
    b = 0;
    cx = pagewidth/2;
    cy = pageheight/2;
    transform(l, b, l, b, gs);
    transform(cx, cy, cx, cy, gs);
    tol = 0;
}

// drawClipped should NOT test if the page's own extent intersects the
// clipping box; it should go right ahead and draw its children if
// THEIR extents intersect the clipping box.

void Page::drawClipped (
    Canvas* c, Coord l, Coord b, Coord r, Coord t, Graphic* gs
) {
    register RefList* i;
    Graphic* gr;
    FullGraphic gstemp;
    Transformer ttemp;
    
    gstemp.SetTransformer(&ttemp);
    for (i = refList->First(); i != refList->End(); i = i->Next()) {
	gr = getGraphic(i);
	concatGraphic(gr, gr, gs, &gstemp);
	drawClippedGraphic(gr, c, l, b, r, t, &gstemp);
    }
    gstemp.SetTransformer(nil); /* to avoid deleting ttemp explicitly */
}
