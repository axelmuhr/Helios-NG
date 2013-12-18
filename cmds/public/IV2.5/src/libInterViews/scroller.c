/*
 * Scrolling implementation.
 */

#include <InterViews/cursor.h>
#include <InterViews/painter.h>
#include <InterViews/pattern.h>
#include <InterViews/perspective.h>
#include <InterViews/rubrect.h>
#include <InterViews/scroller.h>
#include <InterViews/sensor.h>
#include <InterViews/shape.h>
#include <string.h>

static const int inset = 1;	/* space between scroller canvas and bar */

inline int DefaultSize () { return round(0.20*inch); }

static Cursor* hcursor;
static Cursor* vcursor;

static CursorPattern vertPat = {
    0x0100, 0x0380, 0x07c0, 0x0fe0, 0x0280, 0x0280, 0x0280, 0x3ff8,
    0x0280, 0x0280, 0x0280, 0x0fe0, 0x07c0, 0x0380, 0x0100, 0x0
};

static CursorPattern vertMask = {
    0x0380, 0x07c0, 0x0fe0, 0x1ff0, 0x1ff0, 0x07c0, 0x3ff8, 0x3ff8,
    0x07c0, 0x07c0, 0x1ff0, 0x1ff0, 0x0fe0, 0x07c0, 0x0380, 0x0
};

static CursorPattern horizPat = {
    0x0, 0x0, 0x100, 0x100, 0x1110, 0x3118, 0x7ffc, 0xf11e,
    0x7ffc, 0x3118, 0x1110, 0x100, 0x100, 0x0, 0x0, 0x0
};

static CursorPattern horizMask = {
    0x0, 0x0, 0x300, 0x1310, 0x3b38, 0x7ffc, 0xfffe, 0xffff,
    0xfffe, 0x7ffc, 0x3b38, 0x1310, 0x300, 0x0, 0x0, 0x0
};

Scroller::Scroller (Interactor* i, int n) {
    interactor = i;
    size = n;
    Init();
}

Scroller::Scroller (const char* name, Interactor* i, int n) {
    SetInstance(name);
    interactor = i;
    size = n;
    Init();
}

Scroller::Scroller (Interactor* i, int n, Painter* out) : (nil, out) {
    interactor = i;
    size = n;
    Init();
}

void Scroller::Init () {
    view = interactor->GetPerspective();
    view->Attach(this);
    shown = new Perspective;
    shape->Rigid();
    input = new Sensor;
    input->Catch(DownEvent);
    input->Catch(UpEvent);
    input->Catch(MotionEvent);
}

Scroller::~Scroller () {
    view->Detach(this);
    delete shown;
}

void Scroller::MakeBackground () {
    Painter* bg = new Painter(output);
    delete output;
    output = bg;
    output->SetPattern(lightgray);
}

void Scroller::Resize () {
    *shown = *view;
}

inline void Scroller::Background (Coord x1, Coord y1, Coord x2, Coord y2) {
    output->FillRect(canvas, x1, y1, x2, y2);
}

HScroller::HScroller (Interactor* i, int n) : (i, n) {
    Init();
}

HScroller::HScroller (const char* name, Interactor* i, int n) : (name, i, n) {
    Init();
}

HScroller::HScroller (
    Interactor* i, int n, Sensor*, Painter* out
) : (i, n, out) {
    Init();
    Reconfig();
}

void HScroller::Init () {
    SetClassName("HScroller");
}

void HScroller::Reconfig () {
    if (size == 0) {
	shape->height = DefaultSize();
    } else {
	shape->height = size;
    }
    shape->width = round(1*inch);
    shape->hstretch = hfil;
    shape->hshrink = shape->width;
    if (hcursor == nil) {
	hcursor = new Cursor(
	    7, 8, horizPat, horizMask,
	    output->GetFgColor(), output->GetBgColor()
	);
    }
    SetCursor(hcursor);
    MakeBackground();

    const char* attrib = GetAttribute("syncScroll");
    syncScroll = attrib != nil &&
        (strcmp(attrib, "true") == 0 || strcmp(attrib, "on") == 0);
}

VScroller::VScroller (Interactor* i, int n) : (i, n) {
    Init();
}

VScroller::VScroller (const char* name, Interactor* i, int n) : (name, i, n) {
    Init();
}

VScroller::VScroller (
    Interactor* i, int n, Sensor*, Painter* out
) : (i, n, out) {
    Init();
    Reconfig();
}

void VScroller::Init () {
    SetClassName("VScroller");
}

void VScroller::Reconfig () {
    if (size == 0) {
	shape->width = DefaultSize();
    } else {
	shape->width = size;
    }
    shape->height = round(1*inch);
    shape->vstretch = vfil;
    shape->vshrink = shape->height;
    if (vcursor == nil) {
	vcursor = new Cursor(
	    7, 8, vertPat, vertMask,
	    output->GetFgColor(), output->GetBgColor()
	);
    }
    SetCursor(vcursor);
    MakeBackground();

    const char* attrib = GetAttribute("syncScroll");
    syncScroll = attrib != nil &&
        (strcmp(attrib, "true") == 0 || strcmp(attrib, "on") == 0);
}

void HScroller::GetBarInfo (register Perspective* s, Coord& left, int& width) {
    Coord maxwidth = xmax + 1;
    if (s->width == 0) {
        scale = 0.0;
        left = -1;
        width = maxwidth + 2;
    } else {
	scale = double(maxwidth) / double(s->width);
        left = round(double(s->curx - s->x0) * scale);
        width = max(round(double(s->curwidth) * scale), 2);
    }
}

void VScroller::GetBarInfo (register Perspective* s, Coord& bot, int& height) {
    Coord maxheight = ymax + 1;
    if (s->height == 0) {
        scale = 0.0;
        bot = -1;
        height = maxheight + 2;
    } else {
	scale = double(maxheight) / double(s->height);
        bot = round(double(s->cury - s->y0) * scale);
        height = max(round(double(s->curheight) * scale), 2);
    }
}

inline void HScroller::Bar (Coord x, int width) {
    output->ClearRect(canvas, x, inset+1, x+width-1, ymax-inset-1);
}

inline void VScroller::Bar (Coord y, int height) {
    output->ClearRect(canvas, inset+1, y, xmax-inset-1, y+height-1);
}

inline void HScroller::Outline (Coord x, int width) {
    output->Rect(canvas, x, inset, x+width-1, ymax-inset);
}

inline void VScroller::Outline (Coord y, int height) {
    output->Rect(canvas, inset, y, xmax-inset, y+height-1);
}

inline void HScroller::Border (Coord x) {
    output->Line(canvas, x, inset, x, ymax-inset);
}

inline void VScroller::Border (Coord y) {
    output->Line(canvas, inset, y, xmax-inset, y);
}

inline void HScroller::Sides (Coord x1, Coord x2) {
    output->Line(canvas, x1, inset, x2, inset);
    output->Line(canvas, x1, ymax-inset, x2, ymax-inset);
}

inline void VScroller::Sides (Coord y1, Coord y2) {
    output->Line(canvas, inset, y1, inset, y2);
    output->Line(canvas, xmax-inset, y1, xmax-inset, y2);
}

void HScroller::Redraw (Coord x1, Coord y1, Coord x2, Coord y2) {
    Coord left;
    int width;

    Background(x1, y1, x2, y2);
    GetBarInfo(shown, left, width);
    Bar(left, width);
    Outline(left, width);
}

void VScroller::Redraw (Coord x1, Coord y1, Coord x2, Coord y2) {
    Coord bot;
    int height;

    Background(x1, y1, x2, y2);
    GetBarInfo(shown, bot, height);
    Bar(bot, height);
    Outline(bot, height);
}

void HScroller::Handle (Event& e) {
    if (e.eventType == DownEvent) {
        Perspective s;
        register Coord nx;
        Coord lower, upper;
        int dx;
        boolean syncing =
            (syncScroll && !e.control) || (!syncScroll && e.control);

	s = *view;
	nx = s.curx;
	dx = e.shift ? s.lx : s.sx;
	lower = s.x0;
	upper = lower + s.width - s.curwidth;
	switch (e.button) {
	    case LEFTMOUSE:
                if (nx > lower) {
                    nx = max(nx - dx, lower);
                }
		break;
	    case MIDDLEMOUSE:
		nx = Slide(e);
		break;
	    case RIGHTMOUSE:
                if (nx < upper) {
                    nx = min(nx + dx, upper);
                }
		break;
	}
	if (e.button != MIDDLEMOUSE || !syncing) {
	    s.curx = nx;
	    interactor->Adjust(s);
	}
    }
}

void VScroller::Handle (Event& e) {
    if (e.eventType == DownEvent) {
        Perspective s;
        register Coord ny;
        Coord lower, upper;
        int dy;
        boolean syncing =
            (syncScroll && !e.control) || (!syncScroll && e.control);

	s = *view;
	ny = s.cury;
	dy = e.shift ? s.ly : s.sy;
	lower = s.y0;
	upper = lower + s.height - s.curheight;
	switch (e.button) {
	    case LEFTMOUSE:
                if (ny > lower) {
                    ny = max(ny - dy, lower);
                }
		break;
	    case MIDDLEMOUSE:
		ny = Slide(e);
		break;
	    case RIGHTMOUSE:
                if (ny < upper) {
                    ny = min(ny + dy, upper);
                }
		break;
	}
	if (e.button != MIDDLEMOUSE || !syncing) {
	    s.cury = ny;
	    interactor->Adjust(s);
	}
    }
}

Coord HScroller::Slide (register Event& e) {
    Coord x1, y1, x2, y2;
    Coord oldx, minx, maxx;
    int width, w;
    Perspective s;

    s = *view;
    GetBarInfo(shown, oldx, width);
    if (e.x < oldx) {
        x1 = max(0, e.x - width/2);
    } else if (e.x > oldx + width) {
        x1 = min(e.x - width/2, xmax - width);
    } else {
        x1 = oldx;
    }
    x2 = x1 + width - 1;
    minx = min(oldx, 0);
    maxx = max(xmax + 1, oldx + width) - width;
    w = e.x - x1;

    boolean syncing = (syncScroll && !e.control) || (!syncScroll && e.control);
    SlidingRect r(output, canvas, x1+1, inset+1, x2-1, ymax-inset-1, e.x, 0);
    r.Draw();

    for (;;) {
        switch (e.eventType) {
        case UpEvent:
        case DownEvent:
        case MotionEvent:
            r.Track(max(minx + w, min(maxx + w, e.x)), 0);

            if (syncing) {
                r.Erase();
                r.GetCurrent(x1, y1, x2, y2);
                s.curx = shown->x0 + round(double(x1-1) / scale);
                interactor->Adjust(s);
            }
            break;
        }
        if (e.eventType == UpEvent) {
            break;
        }
	Read(e);
    }

    r.GetCurrent(x1, y1, x2, y2);
    r.Erase();
    return shown->x0 + round(double(x1-1) / scale);
}

Coord VScroller::Slide (register Event& e) {
    Coord x1, y1, x2, y2;
    Coord oldy, miny, maxy;
    int height, h;
    Perspective s;

    s = *view;
    GetBarInfo(shown, oldy, height);
    if (e.y < oldy) {
        y1 = max(0, e.y - height/2);
    } else if (e.y > oldy + height) {
        y1 = min(e.y - height/2, ymax - height);
    } else {
        y1 = oldy;
    }
    y2 = y1 + height - 1;
    miny = min(oldy, 0);
    maxy = max(ymax + 1, oldy + height) - height;
    h = e.y - y1;

    boolean syncing = (syncScroll && !e.control) || (!syncScroll && e.control);
    SlidingRect r(output, canvas, inset+1, y1+1, xmax-inset-1, y2-1, 0, e.y );
    r.Draw();

    for (;;) {
        switch (e.eventType) {
        case UpEvent:
        case DownEvent:
        case MotionEvent:
            r.Track(0, max(miny + h, min(maxy + h, e.y)));

            if (syncing) {
                r.Erase();
                r.GetCurrent(x1, y1, x2, y2);
                s.cury = shown->y0 + round(double(y1-1) / scale);
                interactor->Adjust(s);
            }
            break;
        }
        if (e.eventType == UpEvent) {
            break;
        }
	Read(e);
    }

    r.GetCurrent(x1, y1, x2, y2);
    r.Erase();
    return shown->y0 + round(double(y1-1) / scale);
}

void HScroller::Update () {
    Coord oldleft, oldright, newleft, newright;
    int oldwidth, newwidth;
    Perspective* p;

    if (canvas == nil) {
	return;
    }
    p = view;
    GetBarInfo(shown, oldleft, oldwidth);
    GetBarInfo(p, newleft, newwidth);
    if (oldleft != newleft || oldwidth != newwidth) {
	oldright = oldleft+oldwidth-1;
	newright = newleft+newwidth-1;
	if (oldright >= newleft && newright >= oldleft) {
	    if (oldright > newright) {
		Background(newright+1, inset, oldright, ymax-inset);
		Border(newright);
	    } else if (oldright < newright) {
		Bar(oldright, newright-oldright);
		Sides(oldright, newright);
		Border(newright);
	    }
	    if (oldleft > newleft) {
		Bar(newleft+1, oldleft-newleft);
		Sides(newleft, oldleft);
		Border(newleft);
	    } else if (oldleft < newleft) {
		Background(oldleft, inset, newleft-1, ymax-inset);
		Border(newleft);
	    }
	} else {
	    Background(oldleft, inset, oldright, ymax-inset);
	    Bar(newleft, newwidth);
	    Outline(newleft, newwidth);
	}
    }
    *shown = *p;
}

void VScroller::Update () {
    Coord oldbottom, oldtop, newbottom, newtop;
    int oldheight, newheight;
    Perspective* p;

    if (canvas == nil) {
	return;
    }
    p = view;
    GetBarInfo(shown, oldbottom, oldheight);
    GetBarInfo(p, newbottom, newheight);
    if (oldbottom != newbottom || oldheight != newheight) {
	oldtop = oldbottom+oldheight-1;
	newtop = newbottom+newheight-1;
	if (oldtop >= newbottom && newtop >= oldbottom) {
	    if (oldtop > newtop) {
		Background(inset, newtop+1, xmax-inset, oldtop);
		Border(newtop);
	    } else if (oldtop < newtop) {
		Bar(oldtop, newtop-oldtop);
		Sides(oldtop, newtop);
		Border(newtop);
	    }
	    if (oldbottom > newbottom) {
		Bar(newbottom+1, oldbottom-newbottom);
		Sides(newbottom, oldbottom);
		Border(newbottom);
	    } else if (oldbottom < newbottom) {
		Background(inset, oldbottom, xmax-inset, newbottom-1);
		Border(newbottom);
	    }
	} else {
	    Background(inset, oldbottom, xmax-inset, oldtop);
	    Bar(newbottom, newheight);
	    Outline(newbottom, newheight);
	}
    }
    *shown = *p;
}
