// $Header: pageboundary.h,v 1.6 88/09/24 15:07:25 interran Exp $
// declares class PageBoundary.

#ifndef pageboundary_h
#define pageboundary_h

#include <InterViews/Graphic/polygons.h>

// A PageBoundary draws a boundary surrounding a page.

class PageBoundary : public FillRect {
public:

    PageBoundary(double, double, Graphic* = nil);

    void SetPortrait();
    void SetLandscape();
    void ToggleOrientation();

protected:

    void getExtent(float&, float&, float&, float&, float&, Graphic*);
    void draw(Canvas*, Graphic*);
    void drawClipped(Canvas*, Coord, Coord, Coord, Coord, Graphic*);

    int pagewidth;		// stores width of page
    int pageheight;		// stores height of page
    int border;			// stores thickness of boundary

};

#endif
