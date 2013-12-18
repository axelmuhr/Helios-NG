// $Header: slsplines.h,v 1.6 88/09/24 15:08:41 interran Exp $
// declares classes BSplineSelection and ClosedBSplineSelection.

#ifndef slsplines_h
#define slsplines_h

#include "selection.h"

// A BSplineSelection draws an open B-spline with a filled interior.

class BSplineSelection : public NPtSelection {
public:

    BSplineSelection(Coord*, Coord*, int, Graphic* = nil);
    BSplineSelection(istream&, State*);

    Graphic* Copy();
    void GetOriginal(Coord*&, Coord*&, int&);

protected:

    void Init(Coord*, Coord*, int);
    RubberVertex* CreateRubberVertex(Coord*, Coord*, int, int);
    Selection* CreateReshapedCopy(Coord*, Coord*, int);

    void uncacheChildren();
    void getExtent(float&, float&, float&, float&, float&, Graphic*);
    boolean contains(PointObj&, Graphic*);
    boolean intersects(BoxObj&, Graphic*);
    void draw(Canvas*, Graphic*);
    void drawClipped(Canvas*, Coord, Coord, Coord, Coord, Graphic*);

    Graphic* ifillbspline;	// fills an open B-spline
    Graphic* bspline;		// draws an open B-spline
    Coord lx0, ly0, lx1, ly1;	// stores endpoints of left arrowhead
    Coord rx0, ry0, rx1, ry1;	// stores endpoints of right arrowhead

};

// A ClosedBSplineSelection draws a closed B-spline with a filled
// interior.

class ClosedBSplineSelection : public NPtSelection {
public:

    ClosedBSplineSelection(Coord*, Coord*, int, Graphic* = nil);
    ClosedBSplineSelection(istream&, State*);

    Graphic* Copy();
    void GetOriginal(Coord*&, Coord*&, int&);

protected:

    RubberVertex* CreateRubberVertex(Coord*, Coord*, int, int);
    Selection* CreateReshapedCopy(Coord*, Coord*, int);

};

#endif
