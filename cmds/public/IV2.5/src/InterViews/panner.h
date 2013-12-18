/*
 * Panner - an interactor for two-dimensional scrolling and zooming.
 */

#ifndef panner_h
#define panner_h

#include <InterViews/scene.h>

class Panner : public MonoScene {
public:
    Panner(Interactor*, int size = 0);
    Panner(const char*, Interactor*, int size = 0);
    Panner(Interactor*, int size, Painter*);
protected:
    int size;

    virtual void Reconfig();
private:
    Interactor* adjusters;
    Interactor* slider;

    void Init(Interactor*, int);
};

class Slider : public Interactor {
public:
    Slider(Interactor*);
    Slider(const char*, Interactor*);
    Slider(Interactor*, Painter*);
    ~Slider();

    virtual void Draw();
    virtual void Handle(Event&);
    virtual void Update();
    virtual void Reshape(Shape&);
    virtual void Resize();
protected:
    virtual void Reconfig();
    virtual void Redraw(Coord, Coord, Coord, Coord);
private:
    Interactor* interactor;
    Perspective* view;
    Perspective* shown;
    Coord left, bottom, right, top;
    Coord prevl, prevb, prevr, prevt;	// for smart update
    Coord llim, blim, rlim, tlim;	// sliding limits
    boolean constrained, syncScroll;
    int moveType;
    Coord origx, origy;

    void Init(Interactor*);
    Coord ViewX(Coord);
    Coord ViewY(Coord);
    Coord SliderX(Coord);
    Coord SliderY(Coord);
    void CalcLimits(Event&);		// calculate sliding limits
    void SizeKnob();			// calculate size of slider knob
    boolean Inside(Event&);		// true if inside slider knob
    void Constrain(Event&);		// constrain slider knob motion
    void Move(Coord dx, Coord dy);	// move view to reflect slider position
    void Slide(Event&);			// rubberband rect while mousing
    void Jump(Event&);			// for click outside knob
};

#endif
