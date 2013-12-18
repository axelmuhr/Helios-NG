/*
 * Interface to Stencil, an object derived from Graphic.
 */

#ifndef stencil_h
#define stencil_h

#include <InterViews/Graphic/base.h>

class Bitmap;

class Stencil : public Graphic {
public:
    Stencil();
    Stencil(Bitmap* image, Bitmap* mask = nil, Graphic* = nil);
    ~Stencil();

    void GetOriginal(Bitmap*&, Bitmap*&);

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
private:
    boolean readBitmap(PFile*, Bitmap*&);
    boolean writeBitmap(PFile*, Bitmap*);
protected:
    Bitmap* image;
    Bitmap* mask;
};

#endif
