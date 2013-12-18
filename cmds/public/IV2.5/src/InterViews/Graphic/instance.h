/*
 * Interface to Instance, a Graphic that refers to another Graphic.
 */

#ifndef instance_h
#define instance_h

#include <InterViews/Graphic/base.h>

class Instance : public FullGraphic {
public:
    Instance(Graphic* obj = nil, Graphic* gr = nil);

    void GetOriginal(Graphic*&);

    virtual Graphic* Copy();
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
protected:
    Graphic* getGraphic();

    virtual void getExtent(float&, float&, float&, float&, float&, Graphic*);
    virtual boolean contains(PointObj&, Graphic*);
    virtual boolean intersects(BoxObj&, Graphic*);
    virtual void draw(Canvas*, Graphic*);
    virtual void drawClipped(Canvas*, Coord, Coord, Coord, Coord, Graphic*);

    virtual boolean read(PFile*);
    virtual boolean write(PFile*);
protected:
    Ref refGr;
};

/*
 * inlines
 */

inline Graphic* Instance::getGraphic () { return (Graphic*) refGr(); }

#endif
