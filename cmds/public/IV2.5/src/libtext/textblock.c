/*
 * TextBlock - an Interactor for Text
 */

#include <InterViews/scene.h>
#include <InterViews/Text/textblock.h>
#include <InterViews/Text/layout.h>
#include <InterViews/Text/textpainter.h>
#include <InterViews/sensor.h>
#include <InterViews/shape.h>
#include <InterViews/cursor.h>
#include <InterViews/paint.h>
#include <InterViews/perspective.h>
#include <memory.h>

static const int BIG = 1000000;
static const int STRETCH = 10;
static const float PAGEFRACT = 0.8;

TextBlock::TextBlock (int w, int h, Interactor* hand) {
    layout = nil;
    handler = hand;
    next = nil;
    start = 0;
    draw = false;
    shape->Rect(w, h);
    shape->Rigid(
	shape->width, shape->width * STRETCH, 
	shape->height, shape->height * STRETCH
    );
    perspective = new Perspective;
    perspective->Init(0, 0, 1, 1);
    perspective->curx = perspective->x0;
    perspective->cury = perspective->y0;
    perspective->curwidth = perspective->width;
    perspective->curheight = perspective->height;
    perspective->sx = 1;
    perspective->sy = 1;
    length = 0;
    left = top = 0;
}

TextBlock::~TextBlock () {
    if (layout != nil) {
	layout->Unchain(this);
    }
    delete perspective;
}

Coord TextBlock::XPix (Coord x) {
    return shape->hunits * (x - left);
}

Coord TextBlock::YPix (Coord y) {
    return ymax+1 - shape->vunits * (y + 1 - top);
}

Coord TextBlock::XChar (Coord x) {
    return x/shape->hunits + left;
}

Coord TextBlock::YChar (Coord y) {
    return (ymax-y)/shape->vunits + top;
}

int TextBlock::Cols () {
    return width;
}

int TextBlock::Rows () {
    return height;
}

void TextBlock::Initialize (Layout* l, Painter* p, Sensor* s) {
    delete output;
    output = p;
    if (output != nil) {
	output->Reference();
	shape->hunits = output->GetFont()->Width("n");
	shape->vunits = output->GetFont()->Height();
    }
    delete input;
    input = s;
    if (input != nil) {
	input->Reference();
	Listen(input);
    }
    layout = l;
}

void TextBlock::Flush (TextPainter* painter) {
    if (draw) {
	int post = max(0, bx + length - left - cols);
	int pre = max(0, left - bx);
	int l = length - pre - post;
	if (l > 0) {
	    painter->MoveTo(px, py);
	    painter->Text(canvas, buffer + pre, l);
	    px += shape->hunits * l;
	}
	bx += length;
	length = 0;
    }
}

void TextBlock::GoTo (Coord x, Coord y) {
    if (draw) {
	bx = x;
	by = y - start;
	px = XPix(max(x, left));
	py = YPix(y - start);
    }
}

void TextBlock::String (const char* s, int len) {
    if (draw && by >= top && by < top + rows) {
	int l = min(len, BUFFERSIZE-length);
	memcpy(buffer+length, s, l);
	length += l;
    }
}

void TextBlock::Space (int count) {
    if (draw && by >= top && by < top + rows) {
	int c = min(count, BUFFERSIZE-length);
	memset(buffer+length, ' ', c);
	length += c;
    }
}

void TextBlock::Caret () {
    if (draw && by >= top && by < top + rows) {
	if (bx >= left && bx <= left + cols) {
	    output->FillRect(canvas, px, py, px+1, py + shape->vunits-1);
	    px = px + 1;		    // hack!!!
	}
    }
}

void TextBlock::Overfull () {
    if (draw) {
	Coord x = XPix(left + cols);
	Coord y = YPix(top + rows);
	output->FillRect(canvas, x - 2, y, x - 1, y + shape->hunits-1);
	output->FillRect(canvas, x - shape->hunits, y, x - 3, y + 1);
    }
}

void TextBlock::EndLine () {
    if (draw && px < xmax) {
	output->ClearRect(canvas, px, py, xmax, py + shape->vunits - 1);
    }
}

void TextBlock::StartLine () {
    if (draw && px > 0) {
	output->ClearRect(canvas, 0, py, px-1, py + shape->vunits - 1);
    }
}

void TextBlock::EndBlock () {
    if (draw) {
	output->ClearRect(canvas, 0, 0, xmax, py - 1);
    }
}

boolean TextBlock::Contains (Coord y) {
    if (next == nil) {
	return y >= start;
    } else {
	return y >= start && y < start + height;
    }
}

boolean TextBlock::PastEnd (Coord y) {
    if (next == nil) {
	return false;
    } else {
	return y >= start + height;
    }
}

void TextBlock::Wait () { SetCursor(hourglass); }

void TextBlock::Done () { SetCursor(defaultCursor); }

void TextBlock::LastLine (Coord y) {
    if (next == nil) {
	register Perspective* p = perspective;
	int h = max(y - start + 1, p->curheight);
	if (h != p->height) {
	    p->cury += h - p->height;
	    p->height = h;
	    height = p->height;
	    perspective->Update();
	    top = (p->y0 + p->height) - (p->cury + p->curheight);
	}
    }
}

void TextBlock::Adjust (Perspective& np) {
    register Perspective* p = perspective;
    int i;
    boolean changed = false;

    i = round(p->curwidth * float(np.width)/float(np.curwidth));
    if (i != p->width) {
	changed = true;
	p->width = i;
    }
    i = p->x0 + round(p->width * float(np.curx-np.x0)/float(np.width));
    i = max(min(i, p->x0 + p->width - p->curwidth), p->x0);
    if (i != p->curx) {
	changed = true;
	p->curx = i;
    }
    i = round(p->curheight * float(np.height)/float(np.curheight));
    if (i != p->height) {
	changed = true;
	p->height = i;
    }
    i = p->y0 + round(p->height * float(np.cury-np.y0)/float(np.height));
    i = min(max(i, p->y0), p->y0 + p->height - p->curheight);
    if (i != p->cury) {
	changed = true;
	p->cury = i;
    }
    if (changed) {
	np = *p;
	perspective->Update();
	height = p->height;
	rows = p->curheight;
	top = (p->y0 + p->height) - (p->cury + p->curheight);
	width = p->width;
	cols = p->curwidth;
	left = p->curx - p->x0;
	Draw();
    }
}

void TextBlock::ComputeShape () {
    register Perspective* p = perspective;
    float scale;
    int visible;

    visible = ((xmax + 1) - ((xmax + 1) % shape->hunits))/shape->hunits;
    scale = float(visible)/float(p->curwidth);
    p->curwidth = visible;
    p->width = round(scale * p->width);
    p->curx = p->x0 + left;

    visible = ((ymax + 1) - ((ymax + 1) % shape->vunits))/shape->vunits;
    if (next != nil) {
	scale = float(visible)/float(p->curheight);
	p->curheight = visible;
	p->height = round(scale * p->height);
	p->cury = p->y0 + p->height - top - p->curheight;
    } else {
	p->curheight = visible;
	p->height = max(p->height, p->curheight);
	p->cury = p->y0 + p->height - top - p->curheight;
    }
    perspective->Update();
    height = p->height;
    rows = p->curheight;
    width = p->width;
    cols = p->curwidth;
    p->lx = round(PAGEFRACT * cols);
    p->ly = round(PAGEFRACT * rows);
}

void TextBlock::Resize () {
    draw = false;
    ComputeShape();
    if (layout != nil) {
	layout->Rechain();
    }
}

void TextBlock::Redraw (Coord xx1, Coord yy1, Coord xx2, Coord yy2) {
    draw = true;
    int y = YChar(yy1);
    if (y > height-1) {
	y = height-1;
    }
    if (layout != nil) {
	layout->Damage(XChar(xx1), YChar(yy2)+start, XChar(xx2), y+start);
	layout->Repair();
    } else {
	output->ClearRect(canvas, xx1, yy1, xx2, yy2);
    }
}

void TextBlock::Handle (Event& e) {
    if (handler != nil) {
	handler->Handle(e);
    }
}

void TextBlock::Reshape (Shape& s) {
    *shape = s;
    if (Parent() != nil) {
	Parent()->Change(this);
    }
}
