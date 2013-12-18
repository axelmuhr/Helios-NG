/*
 * Rubberbanding for curves.
 */

#ifndef rubcurve_h
#define rubcurve_h

#include <InterViews/rubband.h>

class RubberEllipse : public Rubberband {
public:
    RubberEllipse(
        Painter*, Canvas*, Coord cx, Coord cy, Coord rx, Coord ry,
	Coord offx = 0, Coord offy = 0
    );

    virtual void GetOriginal(Coord& cx, Coord& cy, Coord& rx, Coord& ry);
    virtual void GetCurrent(Coord& cx, Coord& cy, Coord& rx, Coord& ry);
    virtual void OriginalRadii(int& xr, int& yr);
    virtual void CurrentRadii(int& xr, int& yr);
    virtual void Draw();
protected:
    Coord centerx, radiusx;
    Coord centery, radiusy;
};

class RubberCircle : public RubberEllipse {
public:
    RubberCircle(
        Painter*, Canvas*, Coord cx, Coord cy, Coord rx, Coord ry,
	Coord offx = 0, Coord offy = 0
    );

    virtual void OriginalRadii(int& xr, int& yr);
    virtual void CurrentRadii(int& xr, int& yr);
    virtual void Draw();
};

class RubberPointList : public Rubberband {
public:
    RubberPointList(
        Painter*, Canvas*, Coord px[], Coord py[], int n,
	Coord offx = 0, Coord offy = 0
    );
    ~RubberPointList();
protected:
    void Copy(Coord*, Coord*, int, Coord*&, Coord*&);
protected:
    Coord *x;
    Coord *y;
    int count;
};    

class RubberVertex : public RubberPointList {
public:
    RubberVertex(
        Painter*, Canvas*, Coord px[], Coord py[], int n, int pt,
	Coord offx = 0, Coord offy = 0
    );

    virtual void GetOriginal(Coord*& px, Coord*& py, int& n, int& pt);
    virtual void GetCurrent(Coord*& px, Coord*& py, int& n, int& pt);
protected:
    void DrawSplineSection (Painter*, Canvas*, Coord x[], Coord y[]);
protected:
    int rubberPt;
};

class RubberHandles : public RubberVertex {
public:
    RubberHandles(
        Painter*, Canvas*, Coord x[], Coord y[], int n, int pt, int size,
	Coord offx = 0, Coord offy = 0
    );

    virtual void Track(Coord x, Coord y);
    virtual void Draw();
protected:
    int d;	/* half of handle size */
};

class RubberSpline : public RubberVertex {
public:
    RubberSpline(
        Painter*, Canvas*, Coord px[], Coord py[], int n, int pt,
	Coord offx = 0, Coord offy = 0
    );

    virtual void Draw();
};

class RubberClosedSpline : public RubberVertex {
public:
    RubberClosedSpline(
        Painter*, Canvas*, Coord px[], Coord py[], int n, int pt,
	Coord offx = 0, Coord offy = 0
    );

    virtual void Draw();
};

class SlidingPointList : public RubberPointList {
public:
    SlidingPointList (
        Painter*, Canvas*, Coord px[], Coord py[], int n,
	Coord rfx, Coord rfy, Coord offx = 0, Coord offy = 0
    );

    virtual void GetOriginal(Coord*& px, Coord*& py, int& n);
    virtual void GetCurrent(Coord*& px, Coord*& py, int& n);
    virtual void Draw();
    virtual void Track(Coord x, Coord y);
protected:
    Coord refx;
    Coord refy;
};

class SlidingLineList : public SlidingPointList {
public:
    SlidingLineList(
        Painter*, Canvas*, Coord x[], Coord y[], int n,
	Coord rfx, Coord rfy, Coord offx = 0, Coord offy = 0
    );

    virtual void Draw();
};    

#endif
