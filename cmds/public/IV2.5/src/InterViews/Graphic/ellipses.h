/*
 * Interface to Ellipses and Circles, objects derived from Graphic.
 */

#ifndef ellipses_h
#define ellipses_h

#include <InterViews/Graphic/base.h>

class Ellipse : public Graphic {
public:
    Ellipse();
    Ellipse(Coord x0, Coord y0, int r1, int r2, Graphic* gr = nil);
    ~Ellipse();
    
    void GetOriginal(Coord&, Coord&, int&, int&);

    virtual void SetBrush(PBrush*);
    virtual PBrush* GetBrush();
    virtual void SetPattern(PPattern*);
    virtual PPattern* GetPattern();

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

    virtual boolean read(PFile*);
    virtual boolean write(PFile*);
protected:
    Ref _patbr;
    Coord x0, y0;
    int r1, r2;
};

class FillEllipse : public Ellipse {
public:
    FillEllipse();
    FillEllipse(Coord x0, Coord y0, int r1, int r2, Graphic* gr = nil);

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

class Circle : public Ellipse {
public:
    Circle();
    Circle(Coord x0, Coord y0, int radius, Graphic* gr = nil);

    virtual Graphic* Copy();
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
};

class FillCircle : public FillEllipse {
public:
    FillCircle();
    FillCircle(Coord x0, Coord y0, int radius, Graphic* gr = nil);

    virtual Graphic* Copy();
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
};

#endif
