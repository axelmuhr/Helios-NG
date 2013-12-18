/*
 * Rubberbanding lines.
 */

#include <InterViews/rubline.h>
#include <InterViews/painter.h>
#include <math.h>

RubberLine::RubberLine (
    Painter* p, Canvas* c,
    Coord x0, Coord y0, Coord x1, Coord y1, Coord offx, Coord offy
) : (p, c, offx, offy) {
    fixedx = x0;
    fixedy = y0;
    movingx = x1;
    movingy = y1;
    trackx = x1;
    tracky = y1;
}

void RubberLine::GetOriginal (Coord& x0, Coord& y0, Coord& x1, Coord& y1) {
    x0 = fixedx;
    y0 = fixedy;
    x1 = movingx;
    y1 = movingy;
}

void RubberLine::GetCurrent (Coord& x0, Coord& y0, Coord& x1, Coord& y1) {
    x0 = fixedx;
    y0 = fixedy;
    x1 = trackx;
    y1 = tracky;
}

void RubberLine::Draw () {
    Coord x0, y0, x1, y1;

    if (!drawn) {
	GetCurrent(x0, y0, x1, y1);
	output->Line(canvas, x0+offx, y0+offy, x1+offx, y1+offy);
	drawn = true;
    }
}

RubberAxis::RubberAxis (
    Painter* p, Canvas* c, Coord x0, Coord y0, Coord x1, Coord y1,
    Coord offx, Coord offy
) : (p, c, x0, y0, x1, y1, offx, offy) {
    /* nothing else to do */
}

void RubberAxis::GetCurrent (Coord& x0, Coord& y0, Coord& x1, Coord& y1) {
    x0 = fixedx;
    y0 = fixedy;
    if (abs(fixedx - trackx) < abs(fixedy - tracky)) {
	x1 = fixedx;
	y1 = tracky;
    } else {
	x1 = trackx;
	y1 = fixedy;
    }
}

SlidingLine::SlidingLine (
    Painter* p, Canvas* c, Coord x0, Coord y0, Coord x1, Coord y1, 
    Coord rfx, Coord rfy, Coord offx, Coord offy
) : (p, c, x0, y0, x1, y1, offx, offy) {
    refx = rfx;
    refy = rfy;
    trackx = rfx;
    tracky = rfy;
}

void SlidingLine::GetCurrent (Coord& x0, Coord& y0, Coord& x1, Coord& y1) {
    Coord dx = trackx - refx;
    Coord dy = tracky - refy;

    x0 = fixedx + dx;
    y0 = fixedy + dy;
    x1 = movingx + dx;
    y1 = movingy + dy;
}

void RotatingLine::Transform (
    Coord& x, Coord& y,
    double a0, double a1, double b0, double b1, double c0, double c1
) {
    double tx, ty;

    tx = double(x);
    ty = double(y);
    x = round(a0*tx + b0*ty + c0);
    y = round(a1*tx + b1*ty + c1);
}

RotatingLine::RotatingLine (
    Painter* p, Canvas* c, Coord x0, Coord y0, Coord x1, Coord y1, 
    Coord cx, Coord cy, Coord rfx, Coord rfy, Coord offx, Coord offy
) : (p, c, x0, y0, x1, y1, offx, offy) {
    centerx = cx;
    centery = cy;
    refx = rfx;
    refy = rfy;
}    

void RotatingLine::GetCurrent (Coord& x0, Coord& y0, Coord& x1, Coord& y1) {
    double sin, cos, hprod, dx1, dy1, dx2, dy2;
    
    x0 = fixedx - centerx;
    y0 = fixedy - centery;
    x1 = movingx - centerx;
    y1 = movingy - centery;

    dx1 = double(refx - centerx);
    dy1 = double(refy - centery);
    dx2 = double(trackx - centerx);
    dy2 = double(tracky - centery);
    hprod = sqrt((dx1*dx1 + dy1*dy1) * (dx2*dx2 + dy2*dy2));
    if (hprod != 0.0) {
        cos = (dx1*dx2 + dy1*dy2) / hprod;
        sin = (dx1*dy2 - dx2*dy1) / hprod;
        Transform(x0, y0, cos, sin, -sin, cos, 0.0, 0.0);
        Transform(x1, y1, cos, sin, -sin, cos, 0.0, 0.0);
    }
    x0 += centerx;
    y0 += centery;
    x1 += centerx;
    y1 += centery;
}

float RotatingLine::OriginalAngle () {
    Coord x0, y0, x1, y1;

    GetOriginal(x0, y0, x1, y1);
    return Angle(x0, y0, x1, y1);
}

float RotatingLine::CurrentAngle () {
    Coord x0, y0, x1, y1;

    GetCurrent(x0, y0, x1, y1);
    return Angle(x0, y0, x1, y1);
}
