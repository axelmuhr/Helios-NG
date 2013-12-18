/*
 * InterViews logo.
 */

#include <InterViews/shape.h>
#include <InterViews/painter.h>
#include <InterViews/paint.h>
#include <InterViews/bitmap.h>
#include <InterViews/sensor.h>
#include "logo.h"

Logo::Logo () : () {}

boolean Logo::Hit (Event& e) {
    Coord x = 0, y = 0;

    GetRelative(x, y, e.target);
    return e.x >= x && e.y >= y && e.x <= x+xmax && e.y <= y+ymax;
}

PolygonLogo::PolygonLogo (Coord size) {
    unit = float(size - 1) / 13.0;
    shape->width = size;
    shape->height = size;
    b = nil;
    w = nil;
}

void PolygonLogo::Reconfig () {
    delete b;
    delete w;
    b = new Painter(output);
    b->SetPattern(solid);
    w = new Painter(b);
#if sun && defined(X10)
    /* Sun X10 bug */
    w->SetColors(w->GetFgColor(), w->GetFgColor());
#else
    w->SetColors(w->GetBgColor(), w->GetBgColor());
#endif
    b->Scale(.01*unit, .01*unit);
    w->Scale(.01*unit, .01*unit);
}

PolygonLogo::~PolygonLogo () {
    delete b;
    delete w;
}

void PolygonLogo::Redraw (Coord, Coord, Coord, Coord) {
    // outer rects
    b->FillRect(canvas, 0, 0, 1300, 1300);
    w->FillRect(canvas, 100, 100, 1200, 1200);
    b->FillRect(canvas, 200, 200, 1100, 1100);

    // stems of in-pointing arrows
    w->FillRect(canvas, 200, 600, 370, 700);
    w->FillRect(canvas, 600, 200, 700, 370);
    w->FillRect(canvas, 930, 600, 1100, 700);
    w->FillRect(canvas, 600, 930, 700, 1100);
    
    // stems of out-pointing arrows
    rectx[0] = 314;	recty[0] = 386;
    rectx[1] = 386;	recty[1] = 314;
    rectx[2] = 986;	recty[2] = 914;
    rectx[3] = 914;	recty[3] = 986;
    w->FillPolygon(canvas, rectx, recty, 4);
    
    rectx[0] = 914;	recty[0] = 314;
    rectx[1] = 986;	recty[1] = 386;
    rectx[2] = 386;	recty[2] = 986;
    rectx[3] = 314;	recty[3] = 914;
    w->FillPolygon(canvas, rectx, recty, 4);
    
    // out-pointing arrows
    trix[0] = 300;  triy[0] = 300;
    trix[1] = 450;  triy[1] = 300;
    trix[2] = 300;  triy[2] = 450;
    w->FillPolygon(canvas, trix, triy, 3);
    
    trix[0] = 850;  triy[0] = 300;
    trix[1] = 1000;  triy[1] = 300;
    trix[2] = 1000;  triy[2] = 450;
    w->FillPolygon(canvas, trix, triy, 3);
    
    trix[0] = 1000;  triy[0] = 850;
    trix[1] = 1000;  triy[1] = 1000;
    trix[2] = 850;  triy[2] = 1000;
    w->FillPolygon(canvas, trix, triy, 3);
    
    trix[0] = 300;  triy[0] = 850;
    trix[1] = 450;  triy[1] = 1000;
    trix[2] = 300;  triy[2] = 1000;
    w->FillPolygon(canvas, trix, triy, 3);
    
    // in-pointing arrows
    trix[0] = 340;  triy[0] = 550;
    trix[1] = 440;  triy[1] = 650;
    trix[2] = 340;  triy[2] = 750;
    w->FillPolygon(canvas, trix, triy, 3);
    
    trix[0] = 550;  triy[0] = 340;
    trix[1] = 650;  triy[1] = 440;
    trix[2] = 750;  triy[2] = 340;
    w->FillPolygon(canvas, trix, triy, 3);
    
    trix[0] = 960;  triy[0] = 550;
    trix[1] = 960;  triy[1] = 750;
    trix[2] = 860;  triy[2] = 650;
    w->FillPolygon(canvas, trix, triy, 3);
    
    trix[0] = 550;  triy[0] = 960;
    trix[1] = 650;  triy[1] = 860;
    trix[2] = 750;  triy[2] = 960;
    w->FillPolygon(canvas, trix, triy, 3);
}

static const int framesize = 2;

BitmapLogo::BitmapLogo (Bitmap* bits, Coord size) {
    bitmap = bits;
    shape->width = size;
    shape->height = size;
    framer = nil;
}

void BitmapLogo::Reconfig () {
    delete framer;
    framer = new Painter(output);
    framer->SetPattern(new Pattern(0x5a5a));
}

BitmapLogo::~BitmapLogo () {
    delete framer;
    delete bitmap;
}

void BitmapLogo::Redraw (Coord x1, Coord y1, Coord x2, Coord y2) {
    output->ClearRect(canvas, x1, y1, x2, y2);
    output->Stencil(
	canvas, xmax/2 - bitmap->Width()/2, ymax/2 - bitmap->Height()/2,
	bitmap
    );
    framer->FillRect(canvas, 0, 0, framesize-1, ymax-framesize);
    framer->FillRect(canvas, 0, ymax-framesize+1, xmax-framesize, ymax);
    framer->FillRect(canvas, xmax-framesize+1, framesize, xmax, ymax);
    framer->FillRect(canvas, framesize, 0, xmax, framesize-1);
}
