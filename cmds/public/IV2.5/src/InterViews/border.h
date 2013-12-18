/*
 * A border is simply a infinitely stretchable line.
 */

#ifndef border_h
#define border_h

#include <InterViews/interactor.h>

class Border : public Interactor {
protected:
    int thickness;

    Border(int);
    Border(const char*, int);
    Border(Painter*, int);

    virtual void Redraw(Coord, Coord, Coord, Coord);
};

class HBorder : public Border {
public:
    HBorder(int thick = 1);
    HBorder(const char*, int thick = 1);
    HBorder(Painter* out, int thick = 1);
protected:
    virtual void Reconfig();
private:
    void Init();
};

class VBorder : public Border {
public:
    VBorder(int thick = 1);
    VBorder(const char*, int thick = 1);
    VBorder(Painter* out, int thick = 1);
protected:
    virtual void Reconfig();
private:
    void Init();
};

#endif
