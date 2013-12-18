/*
 * Implementation of Label, an object derived from Graphic.
 */

#include <InterViews/transformer.h>
#include <InterViews/Graphic/grclasses.h>
#include <InterViews/Graphic/label.h>
#include <string.h>

void Label::draw (Canvas *c, Graphic* gs) {
    update(gs);
    pText(c, string, count, 0, 0);
}

ClassId Label::GetClassId () { return LABEL; }
boolean Label::IsA (ClassId id) { return LABEL == id || Graphic::IsA(id); }

Label::Label () {
    string = nil;
    count = 0;
}

Label::Label (const char* s, Graphic* gr) : (gr) {
    if (gr == nil) {
	SetFont(nil);
    } else {
	SetFont(gr->GetFont());
    }
    count = strlen(s);
    string = new char[count];
    strncpy(string, s, count);
}

Label::Label (const char* s, int n, Graphic* gr) : (gr) {
    if (gr == nil) {
	SetFont(nil);
    } else {
	SetFont(gr->GetFont());
    }
    count = n;
    string = new char[count];
    strncpy(string, s, count);
}

Graphic* Label::Copy () { return new Label(string, count, this); }

Label::~Label () {
    delete string;
}

const char* Label::GetOriginal () { return string; }
const char* Label::GetOriginal (int& n) { n = count; return string; }

void Label::GetOriginal (char*& s) {
    s = new char[count + 1];
    strncpy(s, string, count);
    s[count] = '\0';
}

void Label::GetOriginal (char*& s, int& n) {
    s = new char[count];
    n = count;
    strncpy(s, string, count);
}

boolean Label::read (PFile* f) {
    boolean ok;
    ok = Graphic::read(f) && font.Read(f) && f->Read(count);
    if (ok) {
	delete string;
	string = new char [count];
	ok = f->Read(string, count);
    }
    return ok;
}

boolean Label::write (PFile* f) {
    return 
	Graphic::write(f) && font.Write(f) &&
	f->Write(count) && f->Write(string, count);
}

void Label::getExtent (
    float& x0, float& y0, float& cx, float& cy, float& tol, Graphic* gs
) {
    PFont* f = gs->GetFont();
    float width = f->Width(string, count);
    float height = f->Height();

    if (gs->GetTransformer() == nil) {
	x0 = 0;
	y0 = 0;
	cx = width / 2;
	cy = height / 2;
    } else {
        transformRect(0, 0, width, height, x0, y0, cx, cy, gs);
	cx = (cx + x0)/2;
	cy = (cy + y0)/2;
    }
    tol = 0;
}

boolean Label::contains (PointObj& po, Graphic* gs) {
    PointObj pt (&po);
    PFont* f = gs->GetFont();

    invTransform(pt.x, pt.y, gs);
    BoxObj b (0, 0, f->Width(string, count), f->Height());
    return b.Contains(pt);
}

boolean Label::intersects (BoxObj& userb, Graphic* gs) {
    Transformer* t = gs->GetTransformer();
    PFont* f = gs->GetFont();
    Coord xmax = f->Width(string, count);
    Coord ymax = f->Height();
    Coord tx0, ty0, tx1, ty1;
    
    if (t != nil && t->Rotated()) {
	Coord x[4], tx[5];
	Coord y[4], ty[5];
    
	x[0] = x[3] = y[0] = y[1] = 0;
	x[2] = x[1] = xmax;
	y[2] = y[3] = ymax;
	transformList(x, y, 4, tx, ty, gs);
	tx[4] = tx[0];
	ty[4] = ty[0];
	FillPolygonObj fp (tx, ty, 5);
	return fp.Intersects(userb);
    
    } else if (t != nil) {
	t->Transform(0, 0, tx0, ty0);
	t->Transform(xmax, ymax, tx1, ty1);
	BoxObj b1 (tx0, ty0, tx1, ty1);
	return b1.Intersects(userb);

    } else {
	BoxObj b2 (0, 0, xmax, ymax);
	return b2.Intersects(userb);
    }
}

void Label::SetFont (PFont* f) {
    if (font != Ref(f)) {
	font = Ref(f);
	invalidateCaches();
    }
}

PFont* Label::GetFont () { return (PFont*) font(); }
