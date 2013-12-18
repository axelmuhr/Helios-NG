/*
 * Interface to Points, Lines, and MultiLines, objects derived from Graphic.
 */

#ifndef lines_h
#define lines_h

#include <InterViews/Graphic/base.h>

class Point : public Graphic {
public:
    Point();
    Point(Coord x, Coord y, Graphic* gr = nil);

    void GetOriginal(Coord&, Coord&);
    virtual void SetBrush(PBrush*);
    virtual PBrush* GetBrush();

    virtual Graphic* Copy();
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
protected:
    virtual void getExtent(float&, float&, float&, float&, float&, Graphic*);
    virtual boolean contains(PointObj&, Graphic*);
    virtual boolean intersects(BoxObj&, Graphic*);
    virtual void draw(Canvas*, Graphic*);

    virtual boolean read(PFile*);
    virtual boolean write(PFile*);
protected:
    Coord x, y;
    Ref brush;
};

class Line : public Graphic {
public:
    Line();
    Line(Coord x0, Coord y0, Coord x1, Coord y1, Graphic* gr = nil);

    virtual void GetOriginal(Coord& x0, Coord& y0, Coord& x1, Coord& y1);
    virtual void SetBrush(PBrush*);
    virtual PBrush* GetBrush();

    virtual Graphic* Copy();
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
protected:
    virtual void getExtent(float&, float&, float&, float&, float&, Graphic*);
    virtual boolean contains(PointObj&, Graphic*);
    virtual boolean intersects(BoxObj&, Graphic*);
    virtual void draw(Canvas*, Graphic*);

    virtual boolean read(PFile*);
    virtual boolean write(PFile*);
protected:
    Coord x0, y0, x1, y1;
    Ref brush;
};

class MultiLine : public Graphic {
public:
    MultiLine();
    MultiLine(Coord* x, Coord* y, int count, Graphic* gr = nil);
    ~MultiLine();

    virtual void GetOriginal(Coord*& x, Coord*& y, int&);
    virtual void SetBrush(PBrush*);
    virtual PBrush* GetBrush();
    virtual void SetPattern(PPattern*);
    virtual PPattern* GetPattern();

    virtual boolean operator == (MultiLine&);
    virtual boolean operator != (MultiLine&);

    virtual Graphic* Copy();
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
protected:
    virtual boolean extentCached();
    void cacheExtent(float, float, float, float, float);
    virtual void uncacheExtent();
    void getCachedExtent(float&, float&, float&, float&, float&);
    virtual void getExtent(float&, float&, float&, float&, float&, Graphic*);
    virtual boolean contains(PointObj&, Graphic*);
    virtual boolean intersects(BoxObj&, Graphic*);
    void virtual draw(Canvas*, Graphic*);

    virtual void pat(Ref);
    virtual Ref pat();
    virtual void br(Ref);
    virtual Ref br();

    virtual boolean read(PFile*);
    virtual boolean write(PFile*);
protected:
    Ref _patbr;
    Coord* x, *y;
    int count;
    Extent* extent;
};

#endif
