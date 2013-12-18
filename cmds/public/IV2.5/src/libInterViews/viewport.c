/*
 * A viewport contains another interactor whose position is determined
 * by the viewport's perspective.
 */

#include <InterViews/canvas.h>
#include <InterViews/painter.h>
#include <InterViews/pattern.h>
#include <InterViews/perspective.h>
#include <InterViews/shape.h>
#include <InterViews/viewport.h>

Viewport::Viewport (Interactor* i, Alignment a) {
    Init(i, a);
}

Viewport::Viewport (const char* name, Interactor* i, Alignment a) {
    SetInstance(name);
    Init(i, a);
}

Viewport::Viewport (
    Sensor* in, Painter* out, Interactor* i, Alignment a
) : (in, out) {
    Init(i, a);
}

void Viewport::Init (Interactor* i, Alignment a) {
    SetClassName("Viewport");
    align = a;
    shape->Rigid(hfil, hfil, vfil, vfil);
    perspective = new Perspective;
    Propagate(false);
    if (i != nil) {
	Insert(i);
    }
}

Viewport::~Viewport () {
    delete perspective;
    delete background;
}

void Viewport::Redraw (Coord x1, Coord y1, Coord x2, Coord y2) {
    background->FillRect(canvas, x1, y1, x2, y2);
}

void Viewport::Reconfig () {
    Shape* s = component->GetShape();
    cwidth = s->width;
    cheight = s->height;
    shape->width = cwidth;
    shape->height = cheight;
    perspective->Init(0, 0, cwidth, cheight);
    perspective->curx = 0;
    perspective->cury = 0;
    perspective->curwidth = cwidth;
    perspective->curheight = cheight;
    perspective->sx = s->hunits;
    perspective->sy = s->vunits;
    background = new Painter(output);
    background->SetPattern(lightgray);
}

void Viewport::DoMove (Interactor* i, Coord& x, Coord& y) {
    perspective->curx = perspective->x0 - x;
    perspective->cury = perspective->y0 - y;
    perspective->Update();
    MonoScene::DoMove(i, x, y);
}

static void AlignHelper (Alignment a, int& x, int& y, int width, int height) {
    switch (a) {
	case BottomLeft:
	case CenterLeft:
	case TopLeft:
	    break;
	case BottomCenter:
	case Center:
	case TopCenter:
	    x += width/2;
	    break;
	case BottomRight:
	case CenterRight:
	case TopRight:
	    x += width;
	    break;
    }
    switch (a) {
	case BottomLeft:
	case BottomCenter:
	case BottomRight:
	    break;
	case CenterLeft:
	case Center:
	case CenterRight:
	    y += height/2;
	    break;
	case TopLeft:
	case TopCenter:
	case TopRight:
	    y += height;
	    break;
    }
}

void Viewport::Resize () {
    canvas->SetBackground(output->GetBgColor());
    float px = XPos();
    float py = YPos();
    float zx = XMag();
    float zy = YMag();
    perspective->curwidth = xmax+1;
    perspective->curheight = ymax+1;
    perspective->lx = round(0.90 * perspective->curwidth);
    perspective->ly = round(0.90 * perspective->curheight);
    DoAdjust(px, py, zx, zy);
}

void Viewport::Adjust (Perspective& np) {
    int x = np.curx;
    int y = np.cury;
    AlignHelper(align, x, y, np.curwidth, np.curheight);

    float px = float(x - np.x0) / float(np.width);
    float py = float(y - np.y0) / float(np.height);
    float zx = (
        float(np.width) / float(cwidth) *
        float(perspective->curwidth) / float(np.curwidth)
    );
    float zy = (
        float(np.height) / float(cheight) *
        float(perspective->curheight) / float(np.curheight)
    );
    DoAdjust(px, py, zx, zy);
    np = *perspective;
}

void Viewport::DoAdjust (float px, float py, float zx, float zy) {
    register Perspective* p = perspective;
    register Shape* s = component->GetShape();
    cwidth = s->width;
    cheight = s->height;

    if (px < 0.0) px = 0.0;
    if (px > 1.0) px = 1.0;
    if (py < 0.0) py = 0.0;
    if (py > 1.0) py = 1.0;

    int w = round(cwidth * zx);
    int h = round(cheight * zy);
    int x = round(w * px);
    int y = round(h * py);

    AlignHelper(align, x, y, -p->curwidth, -p->curheight);
    Place(component, -x, -y, -x + (w - 1), -y + (h - 1));
    p->width = w;
    p->height = h;
    p->curx = p->x0 + x;
    p->cury = p->y0 + y;
    p->Update();
}

void Viewport::AdjustTo (float px, float py, float zx, float zy) {
    DoAdjust(px, py, zx, zy);
}

void Viewport::AdjustBy (float dpx, float dpy, float dzx, float dzy) {
    DoAdjust(XPos() + dpx, YPos() + dpy, XMag() * dzx, YMag() * dzy);
}

void Viewport::ScrollTo (float px, float py) {
    DoAdjust(px, py, XMag(), YMag());
}

void Viewport::ScrollXTo (float px) {
    DoAdjust(px, YPos(), XMag(), YMag());
}

void Viewport::ScrollYTo (float py) {
    DoAdjust(XPos(), py, XMag(), YMag());
}

void Viewport::ScrollBy (float dpx, float dpy) {
    DoAdjust(XPos() + dpx, YPos() + dpy, XMag(), YMag());
}

void Viewport::ScrollXBy (float dpx) {
    DoAdjust(XPos() + dpx, YPos(), XMag(), YMag());
}

void Viewport::ScrollYBy (float dpy) {
    DoAdjust(XPos(), YPos() + dpy, XMag(), YMag());
}

void Viewport::ZoomTo (float zx, float zy) {
    DoAdjust(XPos(), YPos(), zx, zy);
}

void Viewport::ZoomXTo (float zx) {
    DoAdjust(XPos(), YPos(), zx, YMag());
}

void Viewport::ZoomYTo (float zy) {
    DoAdjust(XPos(), YPos(), XMag(), zy);
}

void Viewport::ZoomBy (float dzx, float dzy) {
    DoAdjust(XPos(), YPos(), XMag() * dzx, YMag() * dzy);
}

void Viewport::ZoomXBy (float dzx) {
    DoAdjust(XPos(), YPos(), XMag() * dzx, YMag());
}

void Viewport::ZoomYBy (float dzy) {
    DoAdjust(XPos(), YPos(), XMag(), YMag() * dzy);
}

float Viewport::XPos () {
    int x = perspective->curx;
    int y = 0;
    AlignHelper(align, x, y, perspective->curwidth, perspective->curheight);
    return float(x - perspective->x0) / float(perspective->width);
}

float Viewport::YPos () {
    int x = 0;
    int y = perspective->cury;
    AlignHelper(align, x, y, perspective->curwidth, perspective->curheight);
    return float(y - perspective->y0) / float(perspective->height);
}

float Viewport::XMag () {
    return float(perspective->width) / float(cwidth);
}

float Viewport::YMag () {
    return float(perspective->height) / float(cheight);
}
