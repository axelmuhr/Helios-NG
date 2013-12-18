/*
 * Interface to Rects and Polygons, objects derived from Graphic.
 */

#ifndef polygons_h
#define polygons_h

#include <InterViews/Graphic/lines.h>

class Rect : public Graphic {
public:
    Rect();
    Rect(Coord x0, Coord y0, Coord x1, Coord y1, Graphic* = nil);

    void GetOriginal(Coord&, Coord&, Coord&, Coord&);
    virtual void SetBrush(PBrush*);
    virtual PBrush* GetBrush();
    virtual void SetPattern(PPattern*);
    virtual PPattern* GetPattern();

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

    virtual Graphic* Copy();
    virtual boolean read(PFile*);
    virtual boolean write(PFile*);
protected:
    Coord x0, y0, x1, y1;
    Ref _patbr;
};

class FillRect : public Rect {
public:
    FillRect();
    FillRect(Coord x0, Coord y0, Coord x1, Coord y1, Graphic* = nil);

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

class Polygon : public MultiLine {
public:
    Polygon();
    Polygon(Coord* x, Coord* y, int count, Graphic* = nil);

    virtual Graphic* Copy();
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
protected:
    virtual boolean contains(PointObj&, Graphic*);
    virtual boolean intersects(BoxObj&, Graphic*);
    virtual void draw(Canvas*, Graphic*);
};

class FillPolygon : public Polygon {
public:
    FillPolygon();
    FillPolygon(Coord* x, Coord* y, int count, Graphic* = nil);

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
