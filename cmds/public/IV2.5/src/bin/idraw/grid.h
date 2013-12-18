// $Header: grid.h,v 1.10 89/04/06 23:58:21 interran Exp $
// declares class Grid.

#ifndef grid_h
#define grid_h

#include <InterViews/Graphic/lines.h>

// A Grid draws a grid and can constrain points to lie on it.

static const int GRID_DEFAULTSPACING = 8;

class Grid : public MultiLine {	// because there's no MultiPoint to derive from
public:

    Grid(double, double, Graphic* = nil);

    void Constrain(Coord&, Coord&);
    void SetPortrait();
    void SetLandscape();
    void ToggleOrientation();

    boolean GetGridding();
    double GetSpacing();
    boolean GetVisibility();

    void SetGridding(boolean);
    void SetSpacing(double);
    void SetVisibility(boolean);

protected:

    void getExtent(float&, float&, float&, float&, float&, Graphic*);
    void draw(Canvas*, Graphic*);
    void drawClipped(Canvas*, Coord, Coord, Coord, Coord, Graphic*);
    int DefinePoints(Coord, Coord, Coord, Coord);

    boolean gridding;		// will constrain points to grid if true
    int spacing;		// stores spacing in units of pixels
    double pointsin;		// stores spacing in units of printer's points
    boolean visibility;		// will draw grid points if true
    int pagewidth;		// stores width of page
    int pageheight;		// stores height of page

};

// Define inline access functions to get and set members' values.

inline boolean Grid::GetGridding () {
    return gridding;
}

inline double Grid::GetSpacing () {
    return pointsin;
}

inline boolean Grid::GetVisibility () {
    return visibility;
}

inline void Grid::SetGridding (boolean g) {
    gridding = g;
}

inline void Grid::SetVisibility (boolean v) {
    visibility = v;
}

#endif
