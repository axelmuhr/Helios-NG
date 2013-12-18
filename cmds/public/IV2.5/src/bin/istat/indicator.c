/*
 * Indicator
 * $Header: indicator.c,v 1.6 89/05/05 11:58:24 linton Exp $
 */

#include "indicator.h"
#include "log.h"
#include <InterViews/font.h>
#include <InterViews/painter.h>
#include <InterViews/pattern.h>
#include <InterViews/shape.h>
#include <stdio.h>

float one = 1.0;

Indicator::Indicator (Log* l) {
    log = l;
    log->Attach(this);
    scale = 1.0;
    currentScale = 0;
    scaleCount = 1;
    scales = &one;
    scaleup = scaledown = nil;
    filter = nil;
}

void Indicator::FixScaling () {
    if (scaleup != nil && scaledown != nil) {
	while ((*scaleup)(this) && currentScale > 0) {
	    --currentScale;
	    scale = scales[currentScale];
	    Draw();
	}
	while ((*scaledown)(this) && currentScale < scaleCount-1) {
	    ++currentScale;
	    scale = scales[currentScale];
	    Draw();
	}
    }
}

void Indicator::Scaling (int count, int init, float* s) {
    scaleCount = count;
    currentScale = init;
    scales = s;
    Scale(scales[currentScale]);
}

Readout::Readout (Log* l, const char* s, const char* f) : (l) {
    sample = s;
    format = (char*)f;
    textx = texty = 0;
}

void Readout::Reconfig () {
    Font* f = output->GetFont();
    shape->Rect(f->Width(sample), f->Height());
}

void Readout::Redraw (Coord, Coord, Coord, Coord) {
    Trace* trace = log->GetTrace(1);
    char buffer[32];
    float data = (trace != nil) ? trace->Last() : 0;
    sprintf(buffer, format, Process(data));
    output->Text(canvas, buffer, 0, 0);
    Sync();
}

void Readout::RedrawList (int, Coord[], Coord[], Coord[], Coord[]) {
    Redraw(0, 0, xmax, ymax);
}

Bar::Bar (Log * l, int w) : (l) {
    width = w;
    patterns[1] = nil;
    patterns[2] = nil;
}

void Bar::Reconfig () {
    delete patterns[1];
    delete patterns[2];
    patterns[0] = output;
    patterns[1] = new Painter(output);
    patterns[1]->SetPattern(darkgray);
    patterns[2] = new Painter(output);
    patterns[2]->SetPattern(gray);
    shape->width = width;
    shape->height = int(0.5 * inches);
    shape->Rigid(shape->width/2, shape->width, shape->height, vfil);
}

Bar::~Bar () {
    delete patterns[1];
    delete patterns[2];
}

void Bar::Redraw (Coord, Coord, Coord, Coord) {
    FixScaling();
    output->Rect(canvas, 0, 0, xmax, ymax);
    Trace * trace;
    int height = 1;
    int i = 1;
    while ((trace = log->GetTrace(i)) != nil) {
	int h = int(Limit(Process(trace->Last())) * (ymax-1));
	patterns[i-1]->FillRect(canvas, 1, height, xmax-1, height+h);
	height += h;
	if (i >= 3) {
	    break;
	} else {
	    ++i;
	}
    }
    if (height < ymax-1) {
	output->ClearRect(canvas, 1, height, xmax-1, ymax-1);
    }
    Sync();
}

void Bar::RedrawList (int, Coord[], Coord[], Coord[], Coord[]) {
    Redraw(0, 0, xmax, ymax);
}

Pointer::Pointer (Log* l) : (l) {
    last1 = last2 = 0;
    thumb = 2;
    shape->width = (2*thumb + 5)*pixels;
    shape->height = int(0.5*inches);
    shape->Rigid(0, 0, shape->height, vfil);
}

void Pointer::Show1 (float val) {
    int x2 = xmax / 2;
    int t2 = 2 * thumb;
    int v = int(Limit(Process(val)) * (ymax-t2+1));
    output->ClearRect(canvas, x2-thumb-2, last1, x2+thumb+2, last1+t2);
    output->FillRect(canvas, x2-thumb-2, v, x2-3, v+t2);
    output->FillRect(canvas, x2+3, v, x2+thumb+2, v+t2);
    output->Line(canvas, x2-2, v, x2+2, v);
    last1 = v;
}

void Pointer::Show2 (float val1, float val2) {
    int x2 = xmax / 2;
    int t2 = 2 * thumb;
    int v1 = int(Limit(Process(val1)) * (ymax-t2+1));
    int v2 = int(Limit(Process(val2)) * (ymax-t2+1));
    output->ClearRect(canvas, x2-thumb-2, last1, x2-1, last1+t2);
    output->FillRect(canvas, x2-thumb-2, v1, x2-3, v1+t2);
    output->Line(canvas, x2-2, v1+thumb, x2-1, v1+thumb);
    output->ClearRect(canvas, x2+1, last2, x2+thumb+2, last2+t2);
    output->FillRect(canvas, x2+3, v2, x2+thumb+2, v2+t2);
    output->Line(canvas, x2+1, v2+thumb, x2+2, v2+thumb);
    last1 = v1;
    last2 = v2;
}

void Pointer::Update () {
    FixScaling();
    Show();
}

void Pointer::Show () {
    Trace* t1 = log->GetTrace(1);
    Trace* t2 = log->GetTrace(2);
    if (t2 != nil) {
	Show2(t1->Last(), t2->Last());
    } else if (t1 != nil) {
	Show1(t1->Last());
    }
}

void Pointer::Redraw (Coord x1, Coord y1, Coord x2, Coord y2) {
    output->ClearRect(canvas, x1, y1, x2, y2);
    Show();
    output->Line(canvas, xmax/2, thumb, xmax/2, ymax-thumb);
}

static boolean GraphScaleDown (Indicator* i) {
    Trace* t = i->GetLog()->GetTrace(1);
    return t != nil && t->Last() > i->GetScale() && t->Min(4) > i->GetScale();
}

static boolean GraphScaleUp (Indicator* i) {
    Trace* t = i->GetLog()->GetTrace(1);
    return t != nil &&
	t->Last() < i->GetScale()/4.0 &&
	t->Max(20) < i->GetScale()/4.0;
}

Graph::Graph (Log* l, float j) : (l) {
    jump = j;
    drawn = 0;
    dots = nil;
    Scalers(&GraphScaleUp, &GraphScaleDown);
}

void Graph::Reconfig () {
    delete dots;
    dots = new Painter(output);
    dots->SetPattern(gray);
    shape->width = round(1.0*inches);
    shape->height = round(0.5*inches);
    shape->Rigid(shape->width, hfil, shape->height, vfil);
}

Graph::~Graph () {
    delete dots;
}

void Graph::Plot (int x, float val) {
    int h = int(Limit(Process(val)) * ymax);
    output->Line(canvas, x, 0, x, h);
}

void Graph::Dot (int x, float val) {
    int h = int(Limit(Process(val)) * ymax);
    dots->FillRect(canvas, x, h, x, h);
}

void Graph::Resize () {
    Trace* trace = log->GetTrace(1);
    if (trace != nil) {
	trace->History(xmax+1);
    }
}

void Graph::Update () {
    FixScaling();
    Trace * t1 = log->GetTrace(1);
    Trace * t2 = log->GetTrace(2);
    if (drawn < xmax) {
	++drawn;
	if (t1 != nil) {
	    Plot(drawn, t1->Last());
	}
	if (t2 != nil) {
	    Dot(drawn, t2->Last());
	}
	Sync();
    } else {
	drawn -= int(jump*xmax);
	Draw();
    }
}

void Graph::Redraw (Coord x1, Coord y1, Coord x2, Coord y2) {
    Trace* t1 = log->GetTrace(1);
    Trace* t2 = log->GetTrace(2);
    output->ClearRect(canvas, x1, y1, x2, y2);
    if (t1 != nil) {
	float* h1 = t1->History(drawn+1);
	for (int i = x1; i <= min(drawn,x2); ++i) {
	    Plot(i, h1[i]);
	}
    }
    if (t2 != nil) {
	float* h2 = t2->History(drawn+1);
	for (int i = x1; i <= min(drawn,x2); ++i) {
	    Dot(i, h2[i]);
	}
    }
    Sync();
}
