/*
 * Rubberbanding primitives.
 */

#include <InterViews/rubband.h>
#include <InterViews/painter.h>
#include <math.h>

static float DEGS_PER_RAD = 180.0 / M_PI;

float Rubberband::Angle (Coord x0, Coord y0, Coord x1, Coord y1) {
    float dx, dy, angle;

    dx = float(x1 - x0);
    dy = float(y1 - y0);
    if (dx != 0) {
        angle = DEGS_PER_RAD * atan(dy/dx);
    } else if (dy < 0) {
        angle = -90;
    } else {
        angle = 90;
    }
    if (dx < 0) {
        angle += 180;
    }
    return angle;
}

float Rubberband::Distance (Coord x0, Coord y0, Coord x1, Coord y1) {
    float dx = x0 - x1;
    float dy = y0 - y1;

    return sqrt(dx*dx + dy*dy);
}

Rubberband::Rubberband (Painter* p, Canvas* c, Coord x, Coord y) {
    if (p == nil) {
	output = nil;
    } else {
	output = new Painter(p);
	output->Begin_xor();
    }
    canvas = c;
    drawn = false;
    offx = x;
    offy = y;
}

void Rubberband::Draw () {
    /* shouldn't be called */
}

void Rubberband::Redraw () {
    drawn = false;
    Draw();
}

void Rubberband::Erase () {
    if (drawn) {
        drawn = false;
        Draw();
        drawn = false;
    }
}

void Rubberband::Track (Coord x, Coord y) {
    if (x != trackx || y != tracky) {
        Erase();
	trackx = x;
	tracky = y;
	Draw();
    }
}

Rubberband::~Rubberband () {
    if (LastRef()) {
	delete output;
    }
}

void Rubberband::SetPainter (Painter* p) {
    if (p != output) {
	p->Reference();
	delete output;
	output = p;
	output->Begin_xor();
    }
}

void Rubberband::SetCanvas (Canvas* c) {
    canvas = c;
}
