// $Header: iellipses.h,v 1.6 88/09/24 15:05:40 interran Exp $
// declares classes IFillEllipse and IFillCircle.

#ifndef iellipses_h
#define iellipses_h

#include <InterViews/Graphic/ellipses.h>

// An IFillEllipse knows when NOT to draw itself.

class IFillEllipse : public FillEllipse {
public:

    IFillEllipse(Coord, Coord, int, int, Graphic* = nil);

protected:

    boolean contains(PointObj&, Graphic*);
    boolean intersects(BoxObj&, Graphic*);
    void draw(Canvas*, Graphic*);

};

// An IFillCircle knows when NOT to draw itself.

class IFillCircle : public FillCircle {
public:

    IFillCircle(Coord, Coord, int, Graphic* = nil);

protected:

    boolean contains(PointObj&, Graphic*);
    boolean intersects(BoxObj&, Graphic*);
    void draw(Canvas*, Graphic*);

};

#endif
