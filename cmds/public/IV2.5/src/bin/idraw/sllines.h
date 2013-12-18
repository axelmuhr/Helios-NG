// $Header: sllines.h,v 1.6 88/09/24 15:08:14 interran Exp $
// declares classes LineSelection and MultiLineSelection.

#ifndef sllines_h
#define sllines_h

#include "selection.h"

// A LineSelection draws a line.

class LineSelection : public NPtSelection {
public:

    LineSelection(Coord, Coord, Coord, Coord, Graphic* = nil);
    LineSelection(istream&, State*);

    Graphic* Copy();
    void GetOriginal2(Coord&, Coord&, Coord&, Coord&);
    void GetOriginal(Coord*&, Coord*&, int&);

protected:

    void WriteData(ostream&);
    RubberVertex* CreateRubberVertex(Coord*, Coord*, int, int);
    Selection* CreateReshapedCopy(Coord*, Coord*, int);

    void uncacheChildren();
    void getExtent(float&, float&, float&, float&, float&, Graphic*);
    boolean contains(PointObj&, Graphic*);
    boolean intersects(BoxObj&, Graphic*);
    void draw(Canvas*, Graphic*);
    void drawClipped(Canvas*, Coord, Coord, Coord, Coord, Graphic*);

    Graphic* line;		// draws the line

};

// A MultiLineSelection draws a set of connected lines with a filled
// interior.

class MultiLineSelection : public NPtSelection {
public:

    MultiLineSelection(Coord*, Coord*, int, Graphic* = nil);
    MultiLineSelection(istream&, State*);

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

    Graphic* ifillmultiline;	// fills a set of connected lines
    Graphic* multiline;		// draws a set of connected lines
    Coord lx0, ly0, lx1, ly1;	// stores endpoints of left arrowhead
    Coord rx0, ry0, rx1, ry1;	// stores endpoints of right arrowhead

};

#endif
