/*
 * A frame surrounds another interactor, providing borders, title banners, etc.
 */

#include <InterViews/banner.h>
#include <InterViews/border.h>
#include <InterViews/box.h>
#include <InterViews/canvas.h>
#include <InterViews/frame.h>
#include <InterViews/painter.h>
#include <InterViews/pattern.h>
#include <InterViews/shape.h>
#include <InterViews/sensor.h>

Frame::Frame (Interactor* i, int w) {
    Init(i, w, w, w, w);
}

Frame::Frame (const char* name, Interactor* i, int w) {
    SetInstance(name);
    Init(i, w, w, w, w);
}

Frame::Frame (Interactor* i, int l, int b, int r, int t) {
    Init(i, l, b, r, t);
}

Frame::Frame (const char* name, Interactor* i, int l, int b, int r, int t) {
    SetInstance(name);
    Init(i, l, b, r, t);
}

Frame::Frame (
    Painter* p, Interactor* i, int l, int b, int r, int t
) : (nil, p) {
    Init(i, l, b, r, t);
}

Frame::Frame (Painter* p, Interactor* i, int w) : (nil, p) {
    Init(i, w, w, w, w);
}

void Frame::Init (Interactor* i, int l, int b, int r, int t) {
    SetClassName("Frame");
    left = l;
    bottom = b;
    right = r;
    top = t;
    if (i != nil) {
	Insert(i);
    }
    input = onoffEvents;
    input->Reference();
}

void Frame::Reconfig () {
    MonoScene::Reconfig();
    shape->width += left + right;
    shape->height += bottom + top;
}

void Frame::Resize () {
    canvas->SetBackground(output->GetBgColor());
    Place(component, left, bottom, xmax - right, ymax - top);
}

void Frame::Redraw (Coord x1, Coord y1, Coord x2, Coord y2) {
    register Coord r = xmax - right;
    register Coord t = ymax - top;

    if (x1 < left) {
        output->FillRect(canvas, 0, 0, left-1, t);
    }
    if (y1 < bottom) {
        output->FillRect(canvas, left, 0, xmax, bottom-1);
    }
    if (x2 > r) {
        output->FillRect(canvas, r+1, bottom, xmax, ymax);
    }
    if (y2 > t) {
        output->FillRect(canvas, 0, t+1, r, ymax);
    }
}

void Frame::Handle (Event& e) {
    if (e.eventType == OnEvent) {
        Highlight(true);
    } else if (e.eventType == OffEvent) {
        Highlight(false);
    } else {
	HandleInput(e);
    }
}

void Frame::HandleInput (Event& e) {
    component->Handle(e);
}

void Frame::Highlight (boolean) {
    /* default is to do nothing */
}

/*
 * A title frame is a frame around a box containing
 * a banner, border, and the component.
 */

TitleFrame::TitleFrame (Banner* b, Interactor* i, int w) : (nil, w) {
    Init(b, i);
}

TitleFrame::TitleFrame (
    const char* name, Banner* b, Interactor* i, int w
) : (name, nil, w) {
    Init(b, i);
}

TitleFrame::TitleFrame (
    Painter* out, Banner* b, Interactor* i, int width
) : (out, nil, width) {
    Init(b, i);
}

void TitleFrame::Init (Banner* b, Interactor* i) {
    SetClassName("TitleFrame");
    banner = b;
    if (i != nil) {
	Insert(i);
    }
}

Interactor* TitleFrame::Wrap (Interactor* i) {
    Scene* p = banner->Parent();
    if (p != nil) {
	p->Remove(banner);
    }
    return new VBox(banner, new HBorder, i);
}

void TitleFrame::Highlight (boolean b) {
    banner->highlight = b;
    banner->Draw();
}

/*
 * A border frame draws an outline using a solid pattern when
 * it contains the input focus and using a gray pattern otherwise.
 */
BorderFrame::BorderFrame (Interactor* i, int w) : (i, w) {
    Init();
}

BorderFrame::BorderFrame (
    const char* name, Interactor* i, int w
) : (name, i, w) {
    Init();
}

BorderFrame::BorderFrame (Painter* out, Interactor* i, int w) : (out, i, w) {
    Init();
}

void BorderFrame::Init () {
    SetClassName("BorderFrame");
    grayout = nil;
    normal = false;
}

void BorderFrame::Reconfig () {
    Frame::Reconfig();
    delete grayout;
    grayout = new Painter(output);
    grayout->SetPattern(gray);
}

BorderFrame::~BorderFrame () {
    delete grayout;
}

void BorderFrame::Highlight (boolean b) {
    normal = b;
    Redraw(0, 0, xmax, ymax);
}

void BorderFrame::Redraw (Coord x1, Coord y1, Coord x2, Coord y2) {
    if (normal) {
	Frame::Redraw(x1, y1, x2, y2);
    } else {
	Painter* tmp = output;
	output = grayout;
	Frame::Redraw(x1, y1, x2, y2);
	output = tmp;
    }
}

/*
 * A shadow frame is a frame with a drop shadow.
 */

ShadowFrame::ShadowFrame (Interactor* i, int h, int v) {
    Init(i, h, v);
}

ShadowFrame::ShadowFrame (
    const char* name, Interactor* i, int h, int v
) : (name) {
    Init(i, h, v);
}

ShadowFrame::ShadowFrame (Painter* p, Interactor* i, int h, int v) : (p) {
    Init(i, h, v);
}

void ShadowFrame::Init (Interactor* i, int h, int v) {
    if (h > 0) {
	bottom += h;
    } else {
	top += -h;
    }
    if (v > 0) {
	right += v;
    } else {
	left += -v;
    }
    if (i != nil) {
	Insert(i);
    }
}

void ShadowFrame::Redraw (Coord x1, Coord y1, Coord x2, Coord y2) {
    register Coord r = xmax - right;
    register Coord t = ymax - top;
    register Coord v = bottom + top - 2;
    register Coord h = left + right - 2;

    /* borders */
    if (x1 < left) {
        output->FillRect(canvas, left-1, bottom-1, left-1, t);
    }
    if (y1 < bottom) {
        output->FillRect(canvas, left, bottom-1, r+1, bottom-1);
    }
    if (x2 > r) {
        output->FillRect(canvas, r+1, bottom, r+1, t+1);
    }
    if (y2 > t) {
        output->FillRect(canvas, left-1, t+1, r, t+1);
    }

    /* shadows */
    if (left > 1 && x1 < left-1) {
        output->FillRect(canvas, 0, v, left-2, ymax-v);
    }
    if (bottom > 1 && y1 < bottom-1) {
        output->FillRect(canvas, h, 0, xmax-h, bottom-2);
    }
    if (right > 1 && x2 > r+1) {
        output->FillRect(canvas, r+2, v, xmax, ymax-v);
    }
    if (top > 1 && y2 > t+1) {
        output->FillRect(canvas, h, t+2, xmax-h, ymax);
    }

    /* corner */
    if (left > 1 && bottom > 1 && x1 < left-1 && y1 < bottom-1) {
        output->FillRect(canvas, 0, 0, h - 1, v - 1);
    } else if (left > 1 && top > 1 && x1 < left-1 && y2 > t+1) {
        output->FillRect(canvas, 0, ymax - v + 1, h - 1, ymax);
    } else if (right > 1 && bottom > 1 && x2 > r+1 && y1 < bottom-1) {
        output->FillRect(canvas, xmax - h + 1, 0, xmax, v - 1);
    } else if (right > 1 && top > 1 && x1 > r+1 && y2 > t+1) {
        output->FillRect(canvas, xmax - h + 1, ymax - v + 1, xmax, ymax);
    }
}
/*
 * A margin frame surrounds its component with horizontal and vertical
 * glue.
 */
MarginFrame::MarginFrame (Interactor* i, int margin) : (i, 0) {
    Init(margin, 0, 0, margin, 0, 0);
}
MarginFrame::MarginFrame (
    const char* name, Interactor* i, int margin
) : (name, i, 0) {
    Init(margin, 0, 0, margin, 0, 0);
}
MarginFrame::MarginFrame (
    Interactor* i, int margin, int shrink, int stretch
) : (i, 0) {
    Init(margin, shrink, stretch, margin, shrink, stretch);
}
MarginFrame::MarginFrame (Interactor* i, int hmargin, int vmargin) : (i, 0) {
    Init(hmargin, 0, 0, vmargin, 0, 0);
}
MarginFrame::MarginFrame (Interactor* i,
    int hmargin, int hshrink, int hstretch,
    int vmargin, int vshrink, int vstretch
) : (i, 0) {
    Init(hmargin, hshrink, hstretch, vmargin, vshrink, vstretch);
}
void MarginFrame::Init (int h, int hshr, int hstr, int v, int vshr, int vstr) {
    SetClassName("MarginFrame");
    hmargin = h * 2;
    hshrink = hshr * 2;
    hstretch = hstr * 2;
    vmargin = v * 2;
    vshrink = vshr * 2;
    vstretch = vstr * 2;
}
void MarginFrame::Reconfig () {
    Frame::Reconfig();
    shape->width += hmargin;
    shape->height += vmargin;
    shape->hshrink += hshrink;
    shape->hstretch += hstretch;
    shape->vshrink += vshrink;
    shape->vstretch += vstretch;
}
void MarginFrame::Resize () {
    canvas->SetBackground(output->GetBgColor());
    Coord hextra = (xmax+1) - shape->width;
    Coord h = hmargin;
    if (hextra > 0 && shape->hstretch != 0) {
        h += int(float(hstretch) / float(shape->hstretch) * float(hextra));
    } else if (hextra < 0 && shape->hshrink != 0) {
        h += int(float(hshrink) / float(shape->hshrink) * float(hextra));
    }
    Coord vextra = (ymax+1) - shape->height;
    Coord v = vmargin;
    if (vextra > 0 && shape->vstretch != 0) {
        v += int(float(vstretch) / float(shape->vstretch) * float(vextra));
    } else if (vextra < 0 && shape->vshrink != 0) {
        v += int(float(vshrink) / float(shape->vshrink) * float(vextra));
    }
    Place(component, h/2, v/2, xmax-h/2, ymax-v/2);
}
void MarginFrame::Redraw (Coord, Coord, Coord, Coord) { }
