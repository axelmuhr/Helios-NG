/*
 * Adjuster - button-like interactors for incremental adjustment of an
 * interactor's perspective.
 */

#ifndef adjuster_h
#define adjuster_h

#include <InterViews/interactor.h>

static const int NO_AUTOREPEAT = -1;

class Bitmap;
class Shape;

class Adjuster : public Interactor {
public:
    Adjuster(Interactor*, int = NO_AUTOREPEAT);
    Adjuster(const char*, Interactor*, int = NO_AUTOREPEAT);
    Adjuster(Interactor*, int, Painter*);
    ~Adjuster();

    void Handle(Event&);
    virtual void Highlight();
    void Redraw(Coord, Coord, Coord, Coord);
    void Reshape(Shape&);
    virtual void UnHighlight();
protected:
    Interactor* view;
    Bitmap* plain;
    Bitmap* hit;
    Bitmap* mask;
    int delay;
    Perspective* shown;
    boolean highlighted;

    virtual void AdjustView(Event&);
    void AutoRepeat();
    void Flash();
    void HandlePress();
    void Invert();
    virtual void Reconfig();
    void TimerOn();
    void TimerOff();
private:
    void Init(Interactor*, int);
};

class Zoomer : public Adjuster {
public:
    Zoomer(Interactor*, float factor);
    Zoomer(const char*, Interactor*, float factor);
    Zoomer(Interactor*, float factor, Painter*);
protected:
    void AdjustView(Event&);
private:
    float factor;

    void Init(float);
};

class Enlarger : public Zoomer {
public:
    Enlarger(Interactor*);
    Enlarger(const char*, Interactor*);
    Enlarger(Interactor*, Painter*);
private:
    void Init();
};

class Reducer : public Zoomer {
public:
    Reducer(Interactor*);
    Reducer(const char*, Interactor*);
    Reducer(Interactor*, Painter*);
private:
    void Init();
};

class Mover : public Adjuster {
public:
    Mover(Interactor*, int delay, int moveType);
    Mover(const char*, Interactor*, int delay, int moveType);
    Mover(Interactor*, int delay, int moveType, Painter*);
protected:
    int moveType;
    void AdjustView(Event&);
private:
    void Init(int);
};

class LeftMover : public Mover {
public:
    LeftMover(Interactor*, int delay = NO_AUTOREPEAT);
    LeftMover(const char*, Interactor*, int delay = NO_AUTOREPEAT);
    LeftMover(Interactor*, int delay, Painter*);
private:
    void Init();
};

class RightMover : public Mover {
public:
    RightMover(Interactor*, int delay = NO_AUTOREPEAT);
    RightMover(const char*, Interactor*, int delay = NO_AUTOREPEAT);
    RightMover(Interactor*, int delay, Painter*);
private:
    void Init();
};

class UpMover : public Mover {
public:
    UpMover(Interactor*, int delay = NO_AUTOREPEAT);
    UpMover(const char*, Interactor*, int delay = NO_AUTOREPEAT);
    UpMover(Interactor*, int delay, Painter*);
private:
    void Init();
};

class DownMover : public Mover {
public:
    DownMover(Interactor*, int delay = NO_AUTOREPEAT);
    DownMover(const char*, Interactor*, int delay = NO_AUTOREPEAT);
    DownMover(Interactor*, int delay, Painter*);
private:
    void Init();
};

#endif
