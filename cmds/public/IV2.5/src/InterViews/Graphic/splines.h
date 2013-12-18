/*
 * Interface to BSplines, objects derived from Graphic.
 */

#ifndef splines_h
#define splines_h

#include <InterViews/Graphic/lines.h>

class BSpline : public MultiLine {
public:
    BSpline();
    BSpline(Coord* x, Coord* y, int count, Graphic* gr =nil);

    virtual Graphic* Copy();
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
protected:
    virtual boolean contains(PointObj&, Graphic*);
    virtual boolean intersects(BoxObj&, Graphic*);
    virtual void draw(Canvas*, Graphic*);
};

class ClosedBSpline : public BSpline {
public:
    ClosedBSpline();
    ClosedBSpline(Coord* x, Coord* y, int count, Graphic* gr =nil);

    virtual Graphic* Copy();
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
protected:
    virtual boolean contains(PointObj&, Graphic*);
    virtual boolean intersects(BoxObj&, Graphic*);
    virtual void draw(Canvas*, Graphic*);
};

class FillBSpline : public ClosedBSpline {
public:
    FillBSpline();
    FillBSpline(Coord* x, Coord* y, int count, Graphic* gr =nil);

    virtual void SetBrush(PBrush*);

    virtual Graphic* Copy();
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
protected:
    virtual void getExtent(float&, float&, float&, float&, float&, Graphic*);
    virtual boolean contains(PointObj&, Graphic*);
    virtual boolean intersects(BoxObj&, Graphic*);
    virtual void draw(Canvas*, Graphic*);

    virtual void pat(Ref);
    virtual Ref pat();
    virtual void br(Ref);
    virtual Ref br();
};

#endif
