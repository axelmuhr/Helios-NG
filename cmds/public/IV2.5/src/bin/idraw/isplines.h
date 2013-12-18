// $Header: isplines.h,v 1.6 88/09/24 15:06:08 interran Exp $
// declares classes IFillBSpline and IFillClosedBSpline.

#ifndef isplines_h
#define isplines_h

#include <InterViews/Graphic/splines.h>

// An IFillBSpline knows when NOT to draw itself.

class IFillBSpline : public FillBSpline {
public:

    IFillBSpline(Coord*, Coord*, int, Graphic* = nil);

protected:

    boolean contains(PointObj&, Graphic*);
    boolean intersects(BoxObj&, Graphic*);
    void draw(Canvas*, Graphic*);

};

// An IFillClosedBSpline knows when NOT to draw itself.

class IFillClosedBSpline : public FillBSpline {
public:

    IFillClosedBSpline(Coord*, Coord*, int, Graphic* = nil);

protected:

    boolean contains(PointObj&, Graphic*);
    boolean intersects(BoxObj&, Graphic*);
    void draw(Canvas*, Graphic*);

};

#endif
