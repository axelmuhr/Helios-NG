/*
 * Panner implementation.
 */

#include <InterViews/adjuster.h>
#include <InterViews/border.h>
#include <InterViews/box.h>
#include <InterViews/glue.h>
#include <InterViews/painter.h>
#include <InterViews/panner.h>
#include <InterViews/pattern.h>
#include <InterViews/perspective.h>
#include <InterViews/rubrect.h>
#include <InterViews/sensor.h>
#include <InterViews/shape.h>
#include <math.h>
#include <string.h>

Panner::Panner (Interactor* i, int size) {
    Init(i, size);
}

Panner::Panner (const char* name, Interactor* i, int size) {
    SetInstance(name);
    Init(i, size);
}

Panner::Panner (Interactor* i, int size, Painter* p) {
    output = p;
    output->Reference();
    Init(i, size);
}

/* 0.3 second delay for auto-repeat */
static int DELAY = 3;

void Panner::Init (Interactor* i, int n) {
    SetClassName("Panner");
    size = n;
    adjusters = new HBox(
        new HGlue,
	new VBox(
            new VGlue,
            new UpMover(i, DELAY),
	    new HBox(
                new HGlue,
		new LeftMover(i, DELAY),
		new HGlue,
		new RightMover(i, DELAY),
                new HGlue
	    ),
            new DownMover(i, DELAY),
            new VGlue
	),
        new HGlue,
	new VBox(
            new VGlue(2),
	    new Enlarger(i),
            new VGlue(4),
	    new Reducer(i),
            new VGlue(2)
	),
        new HGlue
    );
    slider = new Slider(i);
    Insert(
	new VBox(adjusters, new HBorder, slider)
    );
}

void Panner::Reconfig () {
    MonoScene::Reconfig();
    Shape a = *adjusters->GetShape();
    if (a.vstretch != 0 || a.vshrink != a.height / 3) {
        if (size != 0) {
            a.width = size;
            a.hshrink = a.hstretch = 0;
        }
        a.vstretch = 0;
        a.vshrink = a.height/3;
        adjusters->Reshape(a);
    }
    Shape* s = slider->GetShape();
    if (s->width != a.width) {
        slider->Reshape(a);
    }
}

static const int MIN_SLIDER_HT = 20;
typedef enum MoveType { MOVE_HORIZ, MOVE_VERT, MOVE_UNDEF };

Slider::Slider (Interactor* i) {
    Init(i);
}

Slider::Slider (const char* name, Interactor* i) {
    SetInstance(name);
    Init(i);
}

Slider::Slider (Interactor* i, Painter* p) : (nil, p) {
    Init(i);
}

void Slider::Init (Interactor* i) {
    SetClassName("Slider");
    interactor = i;
    view = i->GetPerspective();
    view->Attach(this);
    shown = new Perspective;
    constrained = false;
    moveType = MOVE_UNDEF;
    *shown = *view;
    shape->vstretch = shape->vshrink = 0;
    prevl = prevb = prevr = prevt = 0;
    input = new Sensor(updownEvents);
}

void Slider::Reconfig () {
    Painter* tmp = new Painter(output);
    delete output;
    output = tmp;

    const char* attrib = GetAttribute("syncScroll");
    syncScroll = attrib != nil &&
        (strcmp(attrib, "true") == 0 || strcmp(attrib, "on") == 0);
}

void Slider::Reshape (Shape& ns) {
    if (shown->width == 0) {
	*shape = ns;
    } else {
	shape->width = (canvas == nil) ? ns.width : xmax + 1;
	float aspect = float(shown->height) / float(shown->width);
	int h = round(aspect * float(shape->width));
	if (h != shape->height) {
	    shape->height = h;
	    Scene* p = Parent();
	    if (p != nil) {
		p->Change(this);
	    }
	}
    }
}

void Slider::Draw () {
    if (canvas != nil) {
	output->SetPattern(lightgray);
	output->FillRect(canvas, 0, 0, xmax, ymax);
	output->SetPattern(clear);
	output->FillRect(canvas, left, bottom, right, top);
	output->SetPattern(solid);
	output->Rect(canvas, left, bottom, right, top);
	output->Line(canvas, left+1, bottom-1, right+1, bottom-1);
	output->Line(canvas, right+1, bottom-1, right+1, top-1);

	prevl = left; prevb = bottom;
	prevr = right; prevt = top;
    }
}

void Slider::Redraw (Coord left, Coord bottom, Coord right, Coord top) {
    output->Clip(canvas, left, bottom, right, top);
    Draw();
    output->NoClip();
}

inline Coord Slider::ViewX (Coord x) {
    return round(float(x) * float(shown->width) / float(xmax));
}

inline Coord Slider::ViewY (Coord y) {
    return round(float(y) * float(shown->height) / float(ymax));
}

inline Coord Slider::SliderX (Coord x) {
    return round(float(x) * float(xmax) / float(shown->width));
}

inline Coord Slider::SliderY (Coord y) {
    return round(float(y) * float(ymax) / float(shown->height));
}

void Slider::Move (Coord dx, Coord dy) {
    shown->curx += dx;
    shown->cury += dy;
}

boolean Slider::Inside (Event& e) {
    return e.x > left && e.x < right && e.y > bottom && e.y < top;
}

void Slider::CalcLimits (Event& e) {
    llim = e.x - max(0, left);
    blim = e.y - max(0, bottom);
    rlim = e.x + max(0, xmax - right);
    tlim = e.y + max(0, ymax - top);
    constrained = e.shift;
    moveType = MOVE_UNDEF;
    origx = e.x;
    origy = e.y;
}

static int CONSTRAIN_THRESH = 2;    
    // difference between x and y movement needed to decide which direction
    // is constrained

void Slider::Constrain (Event& e) {
    Coord dx, dy;

    if (constrained && moveType == MOVE_UNDEF) {
	dx = abs(e.x - origx);
	dy = abs(e.y - origy);
	if (abs(dx - dy) < CONSTRAIN_THRESH) {
	    e.x = origx;
	    e.y = origy;
	} else if (dx > dy) {
	    moveType = MOVE_HORIZ;
	} else {
	    moveType = MOVE_VERT;
	}
    }

    if (!constrained) {
	e.x = min(max(e.x, llim), rlim);
	e.y = min(max(e.y, blim), tlim);

    } else if (moveType == MOVE_HORIZ) {
	e.x = min(max(e.x, llim), rlim);
	e.y = origy;

    } else if (moveType == MOVE_VERT) {
	e.x = origx;
	e.y = min(max(e.y, blim), tlim);

    }
}

void Slider::Slide (Event& e) {
    Coord newleft, newbot, dummy;
    boolean control = e.control;

    Listen(allEvents);
    SlidingRect r(output, canvas, left, bottom, right, top, e.x, e.y);
    CalcLimits(e);
    do {
	switch (e.eventType) {
	    case MotionEvent:
		e.target->GetRelative(e.x, e.y, this);
		Constrain(e);
		r.Track(e.x, e.y);

                if ((syncScroll && !control) || (!syncScroll && control)) {
                    r.Erase();
                    r.GetCurrent(newleft, newbot, dummy, dummy);
                    Move(ViewX(newleft - left), ViewY(newbot - bottom));
                    interactor->Adjust(*shown);
                }

		break;
	    default:
		break;
	}
	Read(e);
    } while (e.eventType != UpEvent);

    r.GetCurrent(newleft, newbot, dummy, dummy);
    Move(ViewX(newleft - left), ViewY(newbot - bottom));
    Listen(input);
}

void Slider::Jump (Event& e) {
    register Perspective* s = shown;
    Coord dx, dy;
    
    if (e.button == RIGHTMOUSE) {
	dx = ViewX(e.x) - s->curx - s->curwidth/2;
	dy = ViewY(e.y) - s->cury - s->curheight/2;
    } else {
	if (e.button == LEFTMOUSE) {
	    dx = s->sx;
	    dy = s->sy;
	} else {
	    dx = s->lx;
	    dy = s->ly;
	}

	if (e.x < left) {
	    dx = -dx;
	} else if (e.x < right) {
	    dx = 0;
	}
	if (e.y < bottom) {
	    dy = -dy;
	} else if (e.y < top) {
	    dy = 0;
	}
    }
    dx = min(
	max(s->x0 - s->curx, dx), s->x0 + s->width - s->curx - s->curwidth
    );
    dy = min(
	max(s->y0 - s->cury, dy), s->y0 + s->height - s->cury - s->curheight
    );
    Move(dx, dy);
}	

void Slider::Handle (Event& e) {
    if (e.eventType == DownEvent) {
	if (Inside(e)) {
	    Slide(e);
	} else {
	    Jump(e);
	}
	interactor->Adjust(*shown);
    }
}
    
static const int MIN_SIZE = 2;

void Slider::SizeKnob () {
    register Perspective* s = shown;
    
    if (canvas != nil) {
	left = SliderX(s->curx - s->x0);
	bottom = SliderY(s->cury - s->y0);
	right = left + max(SliderX(s->curwidth), MIN_SIZE);
	top = bottom + max(SliderY(s->curheight), MIN_SIZE);
    }
}    

void Slider::Update () {
    register Perspective* p = shown;
    int h, oldwidth, oldheight;
    float aspect;
    Scene* s;
    Shape ns;

    oldwidth = p->width;
    oldheight = p->height;
    *p = *view;
    aspect = float(p->height) / float(p->width);

    SizeKnob();
    if (p->width != oldwidth || p->height != oldheight) {
	h = round(aspect * float(shape->width));
	if (h == shape->height) {
	    Draw();
	} else {
	    shape->height = h;
	    if ((s = Parent()) != nil) {
		s->Change(this);
	    }
	}
    } else if (
	prevl != left || prevb != bottom || prevr != right || prevt != top
    ) {
	Draw();
    }
}

void Slider::Resize () {
    int w = xmax + 1;
    if (shape->width != w) {
	Shape ns = *shape;
	ns.width = w;
	Reshape(ns);
    }
    SizeKnob();
}

Slider::~Slider () {
    view->Detach(this);
    delete shown;
}
