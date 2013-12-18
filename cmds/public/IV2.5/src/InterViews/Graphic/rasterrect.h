/*
 * Interface to RasterRect, an object derived from Graphic.
 */

#ifndef rasterrect_h
#define rasterrect_h

#include <InterViews/Graphic/base.h>

class Raster;

class RasterRect : public Graphic {
public:
    RasterRect();
    RasterRect(Raster*, Graphic* = nil);
    ~RasterRect();

    Raster* GetOriginal();

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
    void Init();
    boolean readRaster(PFile*, Raster*&);
    boolean writeRaster(PFile*, Raster*);
protected:
    Raster* raster;
private:
    static class ColorMaker* colorMaker;
};

#endif
