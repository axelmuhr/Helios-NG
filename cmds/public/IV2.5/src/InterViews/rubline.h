/*
 * Rubberbanding for simple lines.
 */

#ifndef rubline_h
#define rubline_h

#include <InterViews/rubband.h>

class RubberLine : public Rubberband {
public:
    RubberLine(
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

class RubberAxis : public RubberLine {
public:
    RubberAxis(
        Painter*, Canvas*, Coord x0, Coord y0, Coord x1, Coord y1,
	Coord offx = 0, Coord offy = 0
    );

    virtual void GetCurrent(Coord& x0, Coord& y0, Coord& x1, Coord& y1);
};

class SlidingLine : public RubberLine {
public:
    SlidingLine(
	Painter*, Canvas*, Coord x0, Coord y0, Coord x1, Coord y1,
	Coord rfx, Coord rfy, Coord offx = 0, Coord offy = 0
    );

    virtual void GetCurrent(Coord& x0, Coord& y0, Coord& x1, Coord& y1);
protected:
    Coord refx;
    Coord refy;
};

class RotatingLine : public RubberLine {
public:
    RotatingLine(
	Painter*, Canvas*, Coord x0, Coord y0, Coord x1, Coord y1, 
	Coord cx, Coord cy, Coord rfx, Coord rfy, 
	Coord offx = 0, Coord offy = 0
    );

    virtual void GetCurrent(Coord& x0, Coord& y0, Coord& x1, Coord& y1);
    float OriginalAngle();
    float CurrentAngle();
protected:
    void Transform (
	Coord& x, Coord& y,
	double a0, double a1, double b0, double b1, double c0, double c1
    );
protected:
    Coord centerx, centery, refx, refy;
};

#endif
