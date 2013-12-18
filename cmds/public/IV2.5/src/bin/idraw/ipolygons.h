// $Header: ipolygons.h,v 1.6 88/09/24 15:06:01 interran Exp $
// declares classes IFillRect and IFillPolygon.

#ifndef ipolygons_h
#define ipolygons_h

#include <InterViews/Graphic/polygons.h>

// An IFillRect knows when NOT to draw itself.

class IFillRect : public FillRect {
public:

    IFillRect(Coord, Coord, Coord, Coord, Graphic* = nil);

protected:

    boolean contains(PointObj&, Graphic*);
    boolean intersects(BoxObj&, Graphic*);
    void draw(Canvas*, Graphic*);

};

// An IFillPolygon knows when NOT to draw itself.

class IFillPolygon : public FillPolygon {
public:

    IFillPolygon(Coord*, Coord*, int, Graphic* = nil);

protected:

    boolean contains(PointObj&, Graphic*);
    boolean intersects(BoxObj&, Graphic*);
    void draw(Canvas*, Graphic*);

};

#endif
