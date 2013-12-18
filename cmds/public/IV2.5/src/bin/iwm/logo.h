#ifndef logo_h
#define logo_h

#include <InterViews/interactor.h>

class Logo : public Interactor {
public:
    Logo();

    boolean Hit(Event&);
};

class PolygonLogo : public Logo {
public:
    PolygonLogo(Coord);
    ~PolygonLogo();
protected:
    virtual void Reconfig();
    virtual void Redraw(Coord, Coord, Coord, Coord);
private:
    float unit;
    Painter* b, * w;
    Coord rectx [4], recty [4];
    Coord trix [3], triy [4];
};

class Bitmap;

class BitmapLogo : public Logo {
public:
    BitmapLogo(Bitmap*, Coord);
    ~BitmapLogo();
protected:
    virtual void Reconfig();
    virtual void Redraw(Coord, Coord, Coord, Coord);
private:
    Painter* framer;
    Bitmap* bitmap;
};

#endif
