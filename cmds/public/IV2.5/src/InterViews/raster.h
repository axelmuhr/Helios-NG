/*
 * Raster - rasterized image
 */

#ifndef raster_h
#define raster_h

#include <InterViews/defs.h>
#include <InterViews/resource.h>

class Color;
class Canvas;

class Raster : public Resource {
public:
    Raster(Color** data, int width, int height);
    Raster(Canvas*, int x0, int y0, int width, int height);
    Raster(Raster*);
    ~Raster();

    int Width();
    int Height();
    boolean Contains(int x, int y);
    Index(int x, int y);

    Color* Peek(int x, int y);
    void Poke(Color*, int x, int y);
private:
    friend class Painter;

    Color** raster;
    class RasterRep* rep;
};

class RasterRep {
private:
    friend class Raster;
    friend class Painter;

    RasterRep(int w, int h);
    RasterRep(Canvas*, int x0, int y0, int w, int h);
    RasterRep(RasterRep*);
    ~RasterRep();

    int GetPixel(int x, int y);
    void PutPixel(int x, int y, int);
    void* GetData();

    int width;
    int height;
    void* data;
};

inline int Raster::Width () { return rep->width; }
inline int Raster::Height () { return rep->height; }
inline boolean Raster::Contains (int x, int y) {
    return x >= 0 && x < Width() && y >= 0 && y < Height();
}
inline int Raster::Index (int x, int y) { return y * Width() + x; }


#endif
