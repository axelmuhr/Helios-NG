/*
 * Interface to Picture, a composite of one or more Graphics.
 */

#ifndef picture_h
#define picture_h

#include <InterViews/Graphic/base.h>

class Picture : public FullGraphic {
public:
    Picture(Graphic* gr = nil);
    virtual ~Picture();

    void Append(Graphic*, Graphic* =nil, Graphic* =nil, Graphic* =nil);
    void Prepend(Graphic*, Graphic* =nil, Graphic* =nil, Graphic* =nil);
    void InsertAfterCur(Graphic*, Graphic* =nil, Graphic* =nil, Graphic* =nil);
    void InsertBeforeCur(Graphic*, Graphic* =nil, Graphic* =nil,Graphic* =nil);
    void Remove(Graphic* p);
    void RemoveCur();
    void SetCurrent(Graphic* p);
    Graphic* GetCurrent();
    Graphic* First();
    Graphic* Last();
    Graphic* Next();
    Graphic* Prev();
    boolean IsEmpty () { return refList->IsEmpty(); }
    boolean AtEnd () { return cur == refList; }

    Graphic* FirstGraphicContaining(PointObj&);
    Graphic* LastGraphicContaining(PointObj&);
    int GraphicsContaining(PointObj&, Graphic**&);

    Graphic* FirstGraphicIntersecting(BoxObj&);
    Graphic* LastGraphicIntersecting(BoxObj&);
    int GraphicsIntersecting(BoxObj&, Graphic**&);

    Graphic* FirstGraphicWithin(BoxObj&);
    Graphic* LastGraphicWithin(BoxObj&);
    int GraphicsWithin(BoxObj&, Graphic**&);

    virtual boolean HasChildren();
    virtual void Propagate();

    virtual Graphic* Copy();
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
protected:
    Graphic* getGraphic(RefList*);

    virtual void draw(Canvas*, Graphic*);
    virtual void drawClipped(Canvas*, Coord, Coord, Coord, Coord, Graphic*);
    
    virtual void getExtent(float&, float&, float&, float&, float&, Graphic*);
    virtual boolean contains(PointObj&, Graphic*);
    virtual boolean intersects(BoxObj&, Graphic*);

    void getCachedExtent(float&, float&, float&, float&, float&);
    virtual boolean extentCached();
    virtual void cacheExtent(float, float, float, float, float);
    virtual void uncacheExtent();
    virtual void uncacheChildren();

    virtual boolean read(PFile*);
    virtual boolean write(PFile*);
    virtual boolean readObjects(PFile*);
    virtual boolean writeObjects(PFile*);
protected:
    RefList* refList, *cur;
private:
    Extent* extent;
};

/*
 * inlines
 */

inline Graphic* Picture::getGraphic (RefList* r) { return (Graphic*) (*r)(); }
inline Graphic* Picture::GetCurrent () { return getGraphic(cur); }

#endif
