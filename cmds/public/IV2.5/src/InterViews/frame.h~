/*
 * A frame surrounds another interactor.
 */

#ifndef frame_h
#define frame_h

#include <InterViews/scene.h>

class Frame : public MonoScene {
public:
    Frame(Interactor* = nil, int width = 1);
    Frame(const char*, Interactor* = nil, int width = 1);
    Frame(Painter*, Interactor* = nil, int width = 1);

    virtual void Handle(Event&);
    virtual void HandleInput(Event&);
    virtual void Highlight(boolean);
protected:
    int left:8, bottom:8, right:8, top:8;
    
    Frame(Interactor*, int, int, int, int);
    Frame(const char*, Interactor*, int, int, int, int);
    Frame(Painter*, Interactor*, int, int, int, int);

    virtual void Reconfig();
    virtual void Resize();
    virtual void Redraw(Coord, Coord, Coord, Coord);
private:
    void Init(Interactor*, int, int, int, int);
};

class Banner;

class TitleFrame : public Frame {
public:
    TitleFrame(Banner*, Interactor*, int width = 1);
    TitleFrame(const char*, Banner*, Interactor*, int width = 1);
    TitleFrame(Painter*, Banner*, Interactor*, int width = 1);

    virtual void Highlight(boolean);
protected:
    Banner* banner;

    virtual Interactor* Wrap(Interactor*);
private:
    void Init(Banner*, Interactor*);
};

class BorderFrame : public Frame {
public:
    BorderFrame(Interactor* = nil, int width = 1);
    BorderFrame(const char*, Interactor* = nil, int width = 1);
    BorderFrame(Painter*, Interactor* = nil, int width = 1);
    ~BorderFrame();

    virtual void Highlight(boolean);
protected:
    virtual void Reconfig();
    virtual void Redraw(Coord, Coord, Coord, Coord);
private:
    boolean normal;
    Painter* grayout;

    void Init();
};

class ShadowFrame : public Frame {
public:
    ShadowFrame(Interactor* = nil, int h = 1, int v = 1);
    ShadowFrame(const char*, Interactor* = nil, int h = 1, int v = 1);
    ShadowFrame(Painter*, Interactor* = nil, int h = 1, int v = 1);
protected:
    virtual void Redraw(Coord, Coord, Coord, Coord);
private:
    void Init(Interactor*, int h, int v);
};

class MarginFrame : public Frame {
public:
    MarginFrame(Interactor* = nil, int margin = 0);
    MarginFrame(const char*, Interactor* = nil, int margin = 0);
    MarginFrame(Interactor*, int margin, int shrink, int stretch);
    MarginFrame(Interactor*, int hmargin, int vmargin);
    MarginFrame(Interactor*,
        int hmargin, int hshrink, int hstretch,
        int vmargin, int vshrink, int vstretch
    );
protected:
    virtual void Reconfig();
    virtual void Resize();
    virtual void Redraw(Coord, Coord, Coord, Coord);
protected:
    int hmargin, hshrink, hstretch;
    int vmargin, vshrink, vstretch;
private:
    void Init(int, int, int, int, int, int);
};
#endif
