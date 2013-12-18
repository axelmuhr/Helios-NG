/*
 * Graphic base class implementation.
 */

#include <InterViews/canvas.h>
#include <InterViews/transformer.h>
#include <InterViews/Graphic/base.h>
#include <InterViews/Graphic/grclasses.h>
#include <InterViews/Graphic/hash.h>
#include <InterViews/Graphic/util.h>

Graphic& Graphic::operator = (Graphic& p) {
    SetColors(p.GetFgColor(), p.GetBgColor());
    FillBg(p.BgFilled());
    SetPattern(p.GetPattern());
    SetBrush(p.GetBrush());
    SetFont(p.GetFont());
    if (p.t == nil) {
	delete t;
	t = nil;
    } else {
	if (t == nil) {
	    t = new Transformer(p.t);
	} else {
	    *t = *p.t;
	}
    }
    invalidateCaches();
    return *this;
}

void Graphic::update (Graphic* gs) {
    Transformer* t;

    p = painters->Find(gs);
    t = p->GetTransformer();
    if (t == nil) {
	if (gs->t != nil) {
	    Transformer* newt = new Transformer(gs->t);
	    p->SetTransformer(newt);
	    delete newt;
	}
    } else {
	if (gs->t == nil) {
	    *t = *identity;
	} else {
	    *t = *gs->t;
	}
    }
}

void Graphic::getExtent (float&, float&, float&, float&, float&, Graphic*) { }
void Graphic::cachingOn () { caching = true; }
void Graphic::cachingOff () { caching = false; }

void Graphic::transform (Coord& x, Coord& y, Graphic* g) {
    Transformer* t = (g == nil) ? GetTransformer() : g->GetTransformer();

    if (t != nil) {
	t->Transform(x, y);
    }
}

void Graphic::transform (Coord x, Coord y, Coord& tx, Coord& ty, Graphic* g) {
    Transformer* t = (g == nil) ? GetTransformer() : g->GetTransformer();

    if (t != nil) {
	t->Transform(x, y, tx, ty);
    } else {
	tx = x;
	ty = y;
    }
}

void Graphic::transform (float x, float y, float& tx, float& ty, Graphic* g) {
    Transformer* t = (g == nil) ? GetTransformer() : g->GetTransformer();

    if (t != nil) {
	t->Transform(x, y, tx, ty);
    } else {
	tx = x;
	ty = y;
    }
}

void Graphic::transformList (
    Coord x[], Coord y[], int n, Coord tx[], Coord ty[], Graphic* g
) {
    Transformer* t = (g == nil) ? GetTransformer() : g->GetTransformer();

    if (t != nil) {
	t->TransformList(x, y, n, tx, ty);
    } else {
	CopyArray(x, y, n, tx, ty);
    }
}

void Graphic::transformRect (
    float x0, float y0, float x1, float y1,
    float& nx0, float& ny0, float& nx1, float& ny1, Graphic* g
) {
    Transformer* t = (g == nil) ? GetTransformer() : g->GetTransformer();
    float tx00, ty00, tx10, ty10, tx11, ty11, tx01, ty01;

    if (t != nil) {
	t->Transform(x0, y0, tx00, ty00);
	t->Transform(x1, y0, tx10, ty10);
	t->Transform(x1, y1, tx11, ty11);
	t->Transform(x0, y1, tx01, ty01);
	nx0 = fmin(tx00, fmin(tx01, fmin(tx10, tx11)));
	ny0 = fmin(ty00, fmin(ty01, fmin(ty10, ty11)));
	nx1 = fmax(tx00, fmax(tx01, fmax(tx10, tx11)));
	ny1 = fmax(ty00, fmax(ty01, fmax(ty10, ty11)));
    } else {
	nx0 = x0; ny0 = y0; nx1 = x1; ny1 = y1;
    }
}

void Graphic::invTransform (Coord& tx, Coord& ty, Graphic* g) {
    Transformer* t = (g == nil) ? GetTransformer() : g->GetTransformer();

    if (t != nil) {
	t->InvTransform(tx, ty);
    }
}

void Graphic::invTransform (
    Coord tx, Coord ty, Coord& x, Coord& y, Graphic* g
) {
    Transformer* t = (g == nil) ? GetTransformer() : g->GetTransformer();

    if (t != nil) {
	t->InvTransform(tx, ty, x, y);
    } else {
	x = tx;
	y = ty;
    }
}

void Graphic::invTransform (
    float tx, float ty, float& x, float& y, Graphic* g
) {
    Transformer* t = (g == nil) ? GetTransformer() : g->GetTransformer();

    if (t != nil) {
	t->InvTransform(tx, ty, x, y);
    } else {
	x = tx;
	y = ty;
    }
}

void Graphic::invTransformList(
    Coord tx[], Coord ty[], int n, Coord x[], Coord y[], Graphic* g
) {
    Transformer* t = (g == nil) ? GetTransformer() : g->GetTransformer();

    if (t != nil) {
	t->InvTransformList(tx, ty, n, x, y);
    } else {
	CopyArray(tx, ty, n, x, y);
    }
}

Graphic* Graphic::getRoot () {
    Graphic* cur, *parent = this;
    
    do {
	cur = parent;
	parent = cur->Parent();
    } while (parent != nil);
    return cur;
}

void Graphic::totalGS (Graphic& gs) {
    Graphic* parent = Parent();
    
    if (parent == nil) {
        concat(nil, this, &gs);
    } else {
        parent->totalGS(gs);
        concat(this, &gs, &gs);
    }
}

void Graphic::parentXform (Transformer& t) {
    Graphic* parent = Parent();

    if (parent == nil) {
        t = *identity;
    } else {
        parent->TotalTransformation(t);
    }
}

void Graphic::TotalTransformation (Transformer& total) {
    Graphic* parent = Parent();
    
    if (parent == nil) {
        concatTransformer(nil, t, &total);
    } else {
        parent->TotalTransformation(total);
        concatTransformer(t, &total, &total);
    }
}

void Graphic::setParent (Graphic* g, Graphic* parent) {
    if (!g->parent.Valid()) {	    // a graphic can have only one parent
	g->parent = Ref(parent);
    }
}

void Graphic::unsetParent (Graphic* g) {
    g->parent = nil;
    g->invalidateCaches();
}

boolean Graphic::read (PFile* f) {
    int test;
    float a[6];
    Ref dummy;		/* dummy origin ref for backward compatibility */
    boolean ok = 
	Persistent::read(f) && parent.Read(f) && dummy.Read(f) &&
	f->Read(fillBg) && fg.Read(f) && bg.Read(f) &&
	tag.Read(f) && f->Read(test);
    if (ok) {
	if (test != int(nil)) {
	    ok = f->Read(a, 6);
	    t = new Transformer(a[0], a[1], a[2], a[3], a[4], a[5]);
	}
    }
    return ok;
}

boolean Graphic::write (PFile* f) {
    float a[6];
    Ref dummy;		/* dummy origin ref for backward compatibility */
    boolean ok = 
	Persistent::write(f) && parent.Write(f) && dummy.Write(f) &&
	f->Write(fillBg) && fg.Write(f) && bg.Write(f) &&
	tag.Write(f) && f->Write(int(t));

    if (ok && t != nil) {
	t->GetEntries(a[0], a[1], a[2], a[3], a[4], a[5]);
	ok = f->Write(a, 6);
    }
    return ok;
}

void Graphic::concatGS (Graphic* a, Graphic* b, Graphic* dest) {
    int fill;
    PColor* fg, *bg;
    PFont* font;
    PBrush* br;
    PPattern* pat;
    
    if (a == nil) {
        *dest = *b;
        return;
    } else if (b == nil) {
        *dest = *a;
        return;
    }
    if ((fill = b->BgFilled()) == UNDEF) {
	fill = a->BgFilled();
    }
    dest->FillBg(fill);

    if ((fg = b->GetFgColor()) == nil) {
	fg = a->GetFgColor();
    }
    if ((bg = b->GetBgColor()) == nil) {
	bg = a->GetBgColor();
    }
    dest->SetColors(fg, bg);

    if ((pat = b->GetPattern()) == nil) {
	pat = a->GetPattern();
    }
    dest->SetPattern(pat);

    if ((font = b->GetFont()) == nil) {
	font = a->GetFont();
    }
    dest->SetFont(font);

    if ((br = b->GetBrush()) == nil) {
	br = a->GetBrush();
    }
    dest->SetBrush(br);
}

void Graphic::concatTransformer (
    Transformer* a, Transformer* b, Transformer* dest
) {
    if (a == nil) {
        *dest = (b == nil) ? *identity : *b;

    } else if (b == nil) {
        *dest = *a;
        
    } else {
        Transformer tmp(a);
        tmp.Postmultiply(b);
        *dest = tmp;
    }
}

void Graphic::concat (Graphic* a, Graphic* b, Graphic* dest) {
    Transformer* ta, *tb, *td;

    ta = (a == nil) ? nil : a->GetTransformer();
    tb = (b == nil) ? nil : b->GetTransformer();
    td = dest->GetTransformer();

    if (td == nil) {
        td = new Transformer;
        dest->SetTransformer(td);
	delete td;
    }
    
    concatGS(a, b, dest);
    concatTransformer(ta, tb, td);
}

boolean Graphic::extentCached () { 
    return false; 
}

void Graphic::uncacheExtent () { }
void Graphic::uncacheChildren () { }

void Graphic::uncacheParents () {
    Graphic* p;
    for (p = Parent(); p != nil && p->extentCached(); p = p->Parent()) {
        p->uncacheExtent();
    }
}

void Graphic::invalidateCaches() {
    uncacheParents();
    uncacheExtent();
    uncacheChildren();
}

void Graphic::getBox (Coord& x0, Coord& y0, Coord& x1, Coord& y1, Graphic* gs) {
    float left, bottom, right, top;

    getBounds(left, bottom, right, top, gs);
    x0 = Coord(left - 1);
    y0 = Coord(bottom - 1);
    x1 = Coord(right + 1);
    y1 = Coord(top + 1);
}

void Graphic::GetExtent (Extent& e) {
    FullGraphic gs;
    
    totalGS(gs);
    getExtent(e.left, e.bottom, e.cx, e.cy, e.tol, &gs);
}

void Graphic::GetBounds (float& x0, float& y0, float& x1, float& y1) {
    FullGraphic gs;
    
    totalGS(gs);
    getBounds(x0, y0, x1, y1, &gs);
}

void Graphic::GetBox (Coord& x0, Coord& y0, Coord& x1, Coord& y1) {
    float left, bottom, right, top;

    GetBounds(left, bottom, right, top);
    x0 = Coord(left - 1);
    y0 = Coord(bottom - 1);
    x1 = Coord(right + 1);
    y1 = Coord(top + 1);
}

boolean Graphic::contains (PointObj& po, Graphic* gs) { 
    BoxObj b;

    getBox(b, gs);
    return b.Contains(po);
}

boolean Graphic::intersects (BoxObj& userb, Graphic* gs) { 
    BoxObj b;

    getBox(b, gs);
    return b.Intersects(userb);
}

void Graphic::draw (Canvas*, Graphic*) { }

void Graphic::erase (Canvas* c, Graphic* gs) {
    PColor* fg = gs->GetFgColor();
    PColor* bg = gs->GetBgColor();
    gs->SetColors(bg, bg);
    draw(c, gs);
    gs->SetColors(fg, bg);
}

void Graphic::drawClipped (
    Canvas* c, Coord left, Coord bottom, Coord right, Coord top, Graphic* gs
) {
    BoxObj thisBox;
    BoxObj clipBox(left, bottom, right, top);

    getBox(thisBox, gs);
    if (clipBox.Intersects(thisBox)) {
	draw(c, gs);
    }
}

void Graphic::eraseClipped (
    Canvas* c, Coord left, Coord bottom, Coord right, Coord top, Graphic* gs
) {
    BoxObj thisBox;
    BoxObj clipBox(left, bottom, right, top);

    getBox(thisBox, gs);
    if (clipBox.Intersects(thisBox)) {
	erase(c, gs);
    }
}

void Graphic::Draw (Canvas* c) {
    if (Parent() == nil) {
	draw(c, this);
    } else {
	FullGraphic gs;
	totalGS(gs);
	draw(c, &gs);
    }
}

void Graphic::Draw (Canvas* c, Coord l, Coord b, Coord r, Coord t) {
    if (Parent() == nil) {
	drawClipped(c, l, b, r, t, this);
    } else {
	FullGraphic gs;
	totalGS(gs);
	drawClipped(c, l, b, r, t, &gs);
    }
}

void Graphic::DrawClipped (Canvas* c, Coord l, Coord b, Coord r, Coord t) {
    painters->Clip(c, l, b, r, t);
    if (Parent() == nil) {
	drawClipped(c, l, b, r, t, this);
    } else {
	FullGraphic gs;
	totalGS(gs);
	drawClipped(c, l, b, r, t, &gs);
    }
    painters->NoClip();
}

void Graphic::Erase (Canvas* c) {
    if (Parent() == nil) {
	erase(c, this);
    } else {
	FullGraphic gs;
	totalGS(gs);
	erase(c, &gs);
    }
}

void Graphic::Erase (Canvas* c, Coord l, Coord b, Coord r, Coord t) {
    if (Parent() == nil) {
	eraseClipped(c, l, b, r, t, this);
    } else {
	FullGraphic gs;
	totalGS(gs);
	eraseClipped(c, l, b, r, t, &gs);
    }
}

void Graphic::EraseClipped (Canvas* c, Coord l, Coord b, Coord r, Coord t) {
    painters->Clip(c, l, b, r, t);
    if (Parent() == nil) {
	eraseClipped(c, l, b, r, t, this);
    } else {
	FullGraphic gs;
	totalGS(gs);
	eraseClipped(c, l, b, r, t, &gs);
    }
    painters->NoClip();
}

Persistent* Graphic::GetCluster () {
    if (parent.Valid()) {
	return Parent()->GetCluster();
    } else {
	return this;
    }
}

static const int PAINTERS_SIZE = 100;

ClassId Graphic::GetClassId () { return GRAPHIC; }

boolean Graphic::IsA (ClassId id) {
    return GRAPHIC == id || Persistent::IsA(id);
}

Graphic::Graphic (Graphic* gr) {
    parent = nil;
    tag = nil;
    t = nil;
    if (painters == nil) {
	painters = new GraphicToPainter(PAINTERS_SIZE);
	identity = new Transformer;
	cachingOn();
    }
    
    if (gr == nil) {
	FillBg((unsigned int) UNDEF);
	SetColors(nil, nil);
    } else {
	FillBg(gr->BgFilled());
	SetColors(gr->GetFgColor(), gr->GetBgColor());
	if (gr->t != nil) {
	    t = new Transformer(gr->t);
	}
    }
}

Graphic::~Graphic () { 
    if (t != nil) {
        delete t;
    }
 }

void Graphic::GetCenter (float& x, float& y) {
    FullGraphic gs;
    float l, b, tol;
    
    totalGS(gs);
    getExtent(l, b, x, y, tol, &gs);
}    

boolean Graphic::Contains (PointObj& p) {
    if (Parent() == nil) {
        return contains(p, this);
    } else {
	FullGraphic gs;
	totalGS(gs);
	return contains(p, &gs);
    }
}

boolean Graphic::Intersects (BoxObj& b) {
    if (Parent() == nil) {
        return intersects(b, this);
    } else {
	FullGraphic gs;
	totalGS(gs);
	return intersects(b, &gs);
    }
}

Graphic* Graphic::Copy () { return new Graphic(this); }

void Graphic::FillBg (boolean fbg) { fillBg = fbg; }
int Graphic::BgFilled () { return fillBg; }

void Graphic::SetColors (PColor* f, PColor* b) {
    fg = Ref(f);
    bg = Ref(b);
}

PColor* Graphic::GetFgColor () { return (PColor*) fg(); }
PColor* Graphic::GetBgColor () { return (PColor*) bg(); }

void Graphic::SetPattern (PPattern*) { }
PPattern* Graphic::GetPattern () { return nil; }

void Graphic::SetBrush (PBrush*) { }
PBrush* Graphic::GetBrush() { return nil; }

void Graphic::SetFont (PFont*) { }
PFont* Graphic::GetFont () { return nil; }

boolean Graphic::HasChildren () { return false; }

void Graphic::Translate (float dx, float dy) { 
    if (dx != 0 || dy != 0) {
	if (t == nil) {
	    t = new Transformer;
	}
	t->Translate(dx, dy);
	uncacheParents();
    }
}

void Graphic::Scale (float sx, float sy, float cx, float cy) {
    float ncx, ncy;

    if (sx != 1 || sy != 1) {
	if (t == nil) {
	    t = new Transformer;
	}
	Transformer parents;
	parentXform(parents);
	parents.InvTransform(cx, cy, ncx, ncy);
	
	if (ncx != 0 || ncy != 0) {
	    t->Translate(-ncx, -ncy);
	    t->Scale(sx, sy);
	    t->Translate(ncx, ncy);
	} else {
	    t->Scale(sx, sy);
	}
	uncacheParents();
    }
}

void Graphic::Rotate (float angle, float cx, float cy) {
    float mag = (angle < 0) ? -angle : angle;
    float ncx, ncy;

    if ((mag - int(mag)) != 0 || int(mag)%360 != 0) {
	if (t == nil) {
	    t = new Transformer;
	}
	Transformer parents;
	parentXform(parents);
	parents.InvTransform(cx, cy, ncx, ncy);
	
	if (ncx != 0 || ncy != 0) {
	    t->Translate(-ncx, -ncy);
	    t->Rotate(angle);
	    t->Translate(ncx, ncy);
	} else {
	    t->Rotate(angle);
	}
	uncacheParents();
    }
}

void Graphic::SetTransformer (Transformer* t) {
    if (t != this->t) {
	delete this->t;
	if (t != nil) {
	    t->Reference();
	}
	this->t = t;
	uncacheParents();
    }
}

void Graphic::Align (Alignment falign, Graphic* moved, Alignment malign) {
    float fx0, fy0, fx1, fy1, mx0, my0, mx1, my1, dx = 0, dy = 0;
    Transformer parents;

    GetBounds(fx0, fy0, fx1, fy1);
    moved->GetBounds(mx0, my0, mx1, my1);
    
    switch (falign) {
	case BottomLeft:
	case CenterLeft:
	case TopLeft:
	case Left:
	    dx = fx0;
	    break;
	case BottomCenter:
	case Center:
	case TopCenter:
	case HorizCenter:
	    dx = (fx0 + fx1 + 1)/2;
	    break;
	case BottomRight:
	case CenterRight:
	case TopRight:
	case Right:
	    dx = fx1 + 1;
	    break;
    }
    switch (falign) {
	case BottomLeft:
	case BottomCenter:
	case BottomRight:
	case Bottom:
	    dy = fy0;
	    break;
	case CenterLeft:
	case Center:
	case CenterRight:
	case VertCenter:
	    dy = (fy0 + fy1 + 1)/2;
	    break;
	case TopLeft:
	case TopCenter:
	case TopRight:
	case Top:
	    dy = fy1 + 1;
	    break;
    }
    
    switch (malign) {
	case BottomLeft:
	case CenterLeft:
	case TopLeft:
	case Left:
	    dx -= mx0;
	    break;	
	case BottomCenter:
	case Center:
	case TopCenter:
	case HorizCenter:
	    dx -= (mx0 + mx1 + 1)/2;
	    break;
	case BottomRight:
	case CenterRight:
	case TopRight:
	case Right:
	    dx -= (mx1 + 1);
	    break;
    }
    switch (malign) {
	case BottomLeft:
	case BottomCenter:
	case BottomRight:
	case Bottom:
	    dy -= my0;
	    break;
	case CenterLeft:
	case Center:
	case CenterRight:
	case VertCenter:
	    dy -= (my0 + my1 + 1)/2;
	    break;
	case TopLeft:
	case TopCenter:
	case TopRight:
	case Top:
	    dy -= (my1 + 1);
	    break;
    }
    if (dx != 0 || dy != 0) {
	parentXform(parents);
	parents.InvTransform(0.0, 0.0, fx0, fy0);
	parents.InvTransform(dx, dy, mx0, my0);
	moved->Translate(mx0-fx0, my0-fy0);
    }
}

/*****************************************************************************/

boolean FullGraphic::read (PFile* f) {
    return Graphic::read(f) && pat.Read(f) && brush.Read(f) && font.Read(f);
}

boolean FullGraphic::write (PFile* f) {
    return Graphic::write(f) && pat.Write(f) && brush.Write(f) &&font.Write(f);
}

Graphic* FullGraphic::Copy () { return new FullGraphic(this); }
ClassId FullGraphic::GetClassId () { return FULL_GRAPHIC; }

boolean FullGraphic::IsA (ClassId id) {
    return FULL_GRAPHIC == id || Graphic::IsA(id);
}

FullGraphic::FullGraphic (Graphic* gr) : (gr) {
    if (gr == nil) {
	SetPattern(nil);
	SetBrush(nil);
	SetFont(nil);
    } else {
	SetPattern(gr->GetPattern());
	SetBrush(gr->GetBrush());
	SetFont(gr->GetFont());
    }	
}

void FullGraphic::SetPattern (PPattern* p) { pat = Ref(p); }
PPattern* FullGraphic::GetPattern () { return (PPattern*) pat(); }

void FullGraphic::SetBrush (PBrush* brush) {
    if (this->brush != Ref(brush)) {
	this->brush = Ref(brush);
	invalidateCaches();
    }
}

PBrush* FullGraphic::GetBrush () { return (PBrush*) brush(); }

void FullGraphic::SetFont (PFont* font) {
    if (this->font != Ref(font)) {
	this->font = Ref(font);
	invalidateCaches();
    }
}

PFont* FullGraphic::GetFont () { return (PFont*) font(); }
