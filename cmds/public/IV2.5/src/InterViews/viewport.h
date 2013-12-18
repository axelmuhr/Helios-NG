/*
 * A viewport contains another interactor.  Unlike a MonoScene or Frame,
 * a viewport always gives the interactor its desired shape.  However,
 * the interactor may not be entirely viewable through the viewport.
 * The viewport's perspective can be used to adjust what portion is visible.
 */

#ifndef viewport_h
#define viewport_h

#include <InterViews/scene.h>

class Viewport : public MonoScene {
public:
    Viewport(Interactor* = nil, Alignment = Center);
    Viewport(const char*, Interactor* = nil, Alignment = Center);
    Viewport(Sensor*, Painter*, Interactor* = nil, Alignment = Center);
    ~Viewport();

    virtual void Adjust(Perspective&);
    virtual void Resize();

    void AdjustTo(float px, float py, float zx, float zy);
    void AdjustBy(float dpx, float dpy, float dzx, float dzy);

    void ScrollTo(float px, float py);
    void ScrollXTo(float px);
    void ScrollYTo(float py);
    void ScrollBy(float dpx, float dpy);
    void ScrollXBy(float dpx);
    void ScrollYBy(float dpy);

    void ZoomTo(float zx, float zy);
    void ZoomXTo(float zx);
    void ZoomYTo(float zy);
    void ZoomBy(float dzx, float dzy);
    void ZoomXBy(float dzx);
    void ZoomYBy(float dzy);

    float XPos();
    float YPos();
    float XMag();
    float YMag();
protected:
    Alignment align;
    Painter* background;

    virtual void Reconfig();
    virtual void Redraw(Coord, Coord, Coord, Coord);
    virtual void DoMove(Interactor*, Coord& x, Coord& y);
private:
    int cwidth;
    int cheight;

    void Init(Interactor*, Alignment);
    void DoAdjust(float px, float py, float zx, float zy);
};

#endif
