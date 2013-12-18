// $Header: page.h,v 1.1 89/04/06 17:34:14 interran Exp $
// declares class Page.

#ifndef page_h
#define page_h

#include <InterViews/Graphic/picture.h>

// A Page is a picture that will group a grid, drawing, and page
// boundary BUT return only the page's dimensions as its extent.

class Page : public Picture {
public:

    Page(double, double, Graphic* = nil);

    void SetPortrait();
    void SetLandscape();
    void ToggleOrientation();

protected:

    void getExtent(float&, float&, float&, float&, float&, Graphic*);
    void drawClipped(Canvas*, Coord, Coord, Coord, Coord, Graphic*);

    int pagewidth;		// stores width of page
    int pageheight;		// stores height of page

};

#endif
