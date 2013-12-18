/*
 * Indicator : a view of a Log
 * $Header: indicator.h,v 1.3 89/04/17 21:09:31 linton Exp $
 */

#ifndef indicator_h
#define indicator_h

#include <InterViews/interactor.h>

class Indicator;
class Log;

typedef float (*FF)(float);
typedef boolean (*IPB)(Indicator*);

class Indicator : public Interactor {
public:
    Indicator(Log*);
    Log * GetLog();
    void Scale(float);
    float GetScale();
    void Scaling(int count, int init, float * scales);
    void Scalers(IPB up, IPB down);
    void Filter(FF);
    void Update () { Draw(); }
protected:
    Log* log;
    float scale;
    int currentScale;
    int scaleCount;
    float * scales;
    FF filter;
    IPB scaleup;
    IPB scaledown;

    float Limit(float f) { return (f>1.0) ? 1.0 : (f<0.0) ? 0.0 : f; }
    float Process(float f) { return filter!=nil?(*filter)(f)/scale:f/scale; }
    void FixScaling();
};

inline Log* Indicator::GetLog () { return log; }
inline void Indicator::Scale (float s) { scale = s; }
inline float Indicator::GetScale () { return scale; }
inline void Indicator::Scalers (IPB up, IPB down) {
    scaleup = up;
    scaledown = down;
}
inline void Indicator::Filter (FF f) { filter = f; }

class Readout : public Indicator {
public:
    Readout(Log*, const char* sample, const char* form);
    void Redraw(Coord, Coord, Coord, Coord);
    void RedrawList(int, Coord[], Coord[], Coord[], Coord[]);
private:
    const char* sample;
    char* format;
    int textx, texty;
    float multiplier;
    float shift;

    void Reconfig();
};

class Bar : public Indicator {
public:
    Bar(Log*, int width);
    ~Bar();
private:
    int width;
    Painter* patterns[3];

    virtual void Reconfig();
    virtual void Redraw(Coord, Coord, Coord, Coord);
    virtual void RedrawList(int, Coord[], Coord[], Coord[], Coord[]);
};

class Pointer : public Indicator {
public:
    Pointer(Log*);

    virtual void Update();
    virtual void Redraw(Coord, Coord, Coord, Coord);
private:
    int last1, last2;
    int thumb;

    void Show1(float val);
    void Show2(float val1, float val2);
    void Show();
};

class Graph : public Indicator {
public:
    Graph(Log*, float jump = 0.5);
    ~Graph();

    virtual void Update();
private:
    Painter* dots;
    float jump;
    int drawn;

    void Dot(int x, float val);
    void Plot(int x, float val);
    virtual void Reconfig();
    virtual void Redraw(Coord, Coord, Coord, Coord);
    virtual void Resize();
};

#endif
