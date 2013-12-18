// $Header: slpolygons.h,v 1.6 88/09/24 15:08:33 interran Exp $
// declares classes RectSelection and PolygonSelection.

#ifndef slpolygons_h
#define slpolygons_h

#include "selection.h"

// A RectSelection draws a rectangle with an outline and a filled
// interior.

class RectSelection : public NPtSelection {
public:

    RectSelection(Coord, Coord, Coord, Coord, Graphic* = nil);
    RectSelection(istream&, State*);

    Graphic* Copy();
    void GetOriginal2(Coord&, Coord&, Coord&, Coord&);
    void GetOriginal(Coord*&, Coord*&, int&);

protected:

    void WriteData(ostream&);
    RubberVertex* CreateRubberVertex(Coord*, Coord*, int, int);
    Selection* CreateReshapedCopy(Coord*, Coord*, int);

};

// A PolygonSelection draws a polygon with an outline and a filled
// interior.

class PolygonSelection : public NPtSelection {
public:

    PolygonSelection(Coord*, Coord*, int, Graphic* = nil);
    PolygonSelection(istream&, State*);

    Graphic* Copy();
    void GetOriginal(Coord*&, Coord*&, int&);

protected:

    RubberVertex* CreateRubberVertex(Coord*, Coord*, int, int);
    Selection* CreateReshapedCopy(Coord*, Coord*, int);

};

#endif
