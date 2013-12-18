/*
 * Implementation of squares view.
 */

#include "squares.h"
#include "view.h"
#include <InterViews/painter.h>
#include <InterViews/sensor.h>
#include <InterViews/shape.h>

SquaresView::SquaresView (Squares* s) {
    subject = s;
    subject->Attach(this);
    shape->width = 200;
    shape->height = 200;
    shape->hunits = 20;
    shape->vunits = 20;
}

SquaresView::~SquaresView () {
    subject->Detach(this);
}

void SquaresView::Redraw (Coord x1, Coord y1, Coord x2, Coord y2) {
    register SquareData* s;
    Coord x, y;
    int w, h;

    output->ClearRect(canvas, x1, y1, x2, y2);
    for (s = subject->Head(); s != nil; s = s->next) {
        w = round(xmax * s->size) / 2;
        h = round(ymax * s->size) / 2;
        x = round(xmax * s->cx);
        y = round(ymax * s->cy);
        if (x1 <= x + w && x2 >= x - w && y1 <= y + h && y2 >= y - h) {
            output->Rect(canvas, x - w, y - h, x + w, y + h);
        }
    }
}

void SquaresView::Update () {
    Draw();
}
