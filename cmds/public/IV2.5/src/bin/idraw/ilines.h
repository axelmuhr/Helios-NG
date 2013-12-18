// $Header: ilines.h,v 1.6 88/09/24 15:05:47 interran Exp $
// declares class IFillMultiLine.

#ifndef ilines_h
#define ilines_h

#include <InterViews/Graphic/polygons.h>

// An IFillMultiLine knows when NOT to draw itself.

class IFillMultiLine : public FillPolygon {
public:

    IFillMultiLine(Coord*, Coord*, int, Graphic* = nil);

protected:

    boolean contains(PointObj&, Graphic*);
    boolean intersects(BoxObj&, Graphic*);
    void draw(Canvas*, Graphic*);

};

#endif
