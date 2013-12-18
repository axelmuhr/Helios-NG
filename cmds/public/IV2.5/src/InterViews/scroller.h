/*
 * General scrolling interface.
 */

#ifndef scroller_h
#define scroller_h

#include <InterViews/interactor.h>

class Scroller : public Interactor {
protected:
    Interactor* interactor;
    int size;
    Perspective* view;
    Perspective* shown;
    double scale;
    Sensor* tracking;
    boolean syncScroll;

    Scroller(Interactor*, int);
    Scroller(const char*, Interactor*, int);
    Scroller(Interactor*, int, Painter* out);
    ~Scroller();

    void Background(Coord, Coord, Coord, Coord);
    void MakeBackground();
    virtual void Resize();
private:
    void Init();
};

class HScroller : public Scroller {
public:
    HScroller(Interactor*, int size = 0);
    HScroller(const char*, Interactor*, int size = 0);
    HScroller(Interactor*, int size, Sensor* in, Painter* out);

    virtual void Handle(Event&);
    virtual void Update();
private:
    void Bar(Coord, int);
    void Border(Coord);
    void GetBarInfo(Perspective*, Coord&, int&);
    void Init();
    void Outline(Coord, int);
    virtual void Reconfig();
    virtual void Redraw(Coord, Coord, Coord, Coord);
    void Sides(Coord, Coord);
    Coord Slide(Event&);
};

class VScroller : public Scroller {
public:
    VScroller(Interactor*, int size = 0);
    VScroller(const char*, Interactor*, int size = 0);
    VScroller(Interactor*, int size, Sensor* in, Painter* out);

    virtual void Handle(Event&);
    virtual void Update();
private:
    void Bar(Coord, int);
    void Border(Coord);
    void GetBarInfo(Perspective*, Coord&, int&);
    void Init();
    void Outline(Coord, int);
    virtual void Reconfig();
    virtual void Redraw(Coord, Coord, Coord, Coord);
    void Sides(Coord, Coord);
    Coord Slide(Event&);
};

#endif
