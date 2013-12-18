// $Header: slellipses.h,v 1.6 88/09/24 15:08:06 interran Exp $
// declares classes EllipseSelection and CircleSelection.

#ifndef slellipses_h
#define slellipses_h

#include "selection.h"

// A EllipseSelection draws a ellipse with an outline and a filled
// interior.

class EllipseSelection : public Selection {
public:

    EllipseSelection(Coord, Coord, Coord, Coord, Graphic* = nil);
    EllipseSelection(istream&, State*);

    Graphic* Copy();
    void GetOriginal(Coord&, Coord&, int&, int&);

protected:

    void WriteData(ostream&);

};

// A CircleSelection draws a circle with an outline and a filled
// interior.

class CircleSelection : public Selection {
public:

    CircleSelection(Coord, Coord, int, Graphic* = nil);
    CircleSelection(istream&, State*);

    Graphic* Copy();
    void GetOriginal(Coord&, Coord&, int&);

protected:

    void WriteData(ostream&);

};

#endif
