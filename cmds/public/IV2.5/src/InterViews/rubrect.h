/*
 * Rubberbanding for rectangles.
 */

#ifndef rubrect_h
#define rubrect_h

#include <InterViews/rubband.h>

class RubberRect : public Rubberband {
public:
    RubberRect(
        Painter*, Canvas*, Coord x0, Coord y0, Coord x1, Coord y1,
	Coord offx = 0, Coord offy = 0
    );

    virtual void GetOriginal(Coord& x0, Coord& y0, Coord& x1, Coord& y1);
    virtual void GetCurrent(Coord& x0, Coord& y0, Coord& x1, Coord& y1);
    virtual void Draw();
protected:
    Coord fixedx, fixedy;
    Coord movingx, movingy;
};

class RubberSquare : public RubberRect {
public:
    RubberSquare(
        Painter*, Canvas*, Coord x0, Coord y0, Coord x1, Coord y1,
	Coord offx = 0, Coord offy = 0
    );

    virtual void GetCurrent(Coord& x0, Coord& y0, Coord& x1, Coord& y1);
};

class SlidingRect : public RubberRect {
public:
    SlidingRect(
        Painter*, Canvas*, Coord x0, Coord y0, Coord x1, Coord y1, 
	Coord rfx, Coord rfy, Coord offx = 0, Coord offy = 0
    );

    virtual void GetCurrent(Coord& x0, Coord& y0, Coord& x1, Coord& y1);
protected:
    Coord refx;
    Coord refy;
};

class StretchingRect : public RubberRect {
public:
    StretchingRect (
        Painter*, Canvas*, Coord x0, Coord y0, Coord x1, Coord y1, Side s,
	Coord offx = 0, Coord offy = 0
    );

    virtual void GetCurrent(Coord& x0, Coord& y0, Coord& x1, Coord& y1);
    float CurrentStretching();
protected:
    Side side;
};

class ScalingRect : public RubberRect {
public:
    ScalingRect(
        Painter*, Canvas*, Coord x0, Coord y0, Coord x1, Coord y1,
	Coord cx, Coord cy, Coord offx = 0, Coord offy = 0
    );

    virtual void GetCurrent(Coord& x0, Coord& y0, Coord& x1, Coord& y1);
    float CurrentScaling();
protected:
    Coord centerx, centery;
    int width, height;
};

class RotatingRect : public Rubberband {
public:
    RotatingRect(
        Painter*, Canvas*, Coord x0, Coord y0, Coord x1, Coord y1, 
	Coord cx, Coord cy, Coord rfx, Coord rfy, 
	Coord offx = 0, Coord offy = 0
    );

    virtual void Draw();
    virtual void GetOriginal(Coord& x0, Coord& y0, Coord& x1, Coord& y1);
    virtual void GetCurrent(
        Coord& leftbotx, Coord& leftboty,
	Coord& rightbotx, Coord& rightboty,
	Coord& righttopx, Coord& righttopy,
	Coord& lefttopx, Coord& lefttopy
    );
    float CurrentAngle();
protected:
    void Transform (
	Coord& x, Coord& y,
	double a0, double a1, double b0, double b1, double c0, double c1
    );
protected:
    Coord left, right, centerx, refx;
    Coord bottom, top, centery, refy;
};

#endif
