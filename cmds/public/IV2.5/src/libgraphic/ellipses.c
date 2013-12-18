/*
 * Implementation of Ellipses and Circles, objects derived from Graphic.
 */

#include <InterViews/transformer.h>
#include <InterViews/Graphic/ellipses.h>
#include <InterViews/Graphic/grclasses.h>
#include <InterViews/Graphic/util.h>

boolean Ellipse::read (PFile* f) {
    return Graphic::read(f) && _patbr.Read(f) &&
	f->Read(x0) && f->Read(y0) && f->Read(r1) && f->Read(r2);
}

boolean Ellipse::write (PFile* f) {
    return Graphic::write(f) && _patbr.Write(f) &&
	f->Write(x0) && f->Write(y0) && f->Write(r1) && f->Write(r2);
}

void Ellipse::draw (Canvas *c, Graphic* gs) {
    if (gs->GetBrush()->Width() != NO_WIDTH) {
	update(gs);
	pEllipse(c, x0, y0, r1, r2);
    }
}

ClassId Ellipse::GetClassId () { 
    return ELLIPSE;
}

boolean Ellipse::IsA (ClassId id) {
    return ELLIPSE == id || Graphic::IsA(id);
}

Ellipse::Ellipse () {}

Ellipse::Ellipse (Coord x0, Coord y0, int r1, int r2, Graphic* gr) : (gr) {
    if (gr == nil) {
	SetBrush(nil);
    } else {
	SetBrush(gr->GetBrush());
    }
    this->x0 = x0;
    this->y0 = y0;
    this->r1 = r1;
    this->r2 = r2;
}    

Graphic* Ellipse::Copy () { 
    return new Ellipse(x0, y0, r1, r2, this);
}

Ellipse::~Ellipse () {
    uncacheExtent();
}

void Ellipse::GetOriginal (Coord& x0, Coord& y0, int& r1, int& r2) {
    x0 = this->x0;
    y0 = this->y0;
    r1 = this->r1;
    r2 = this->r2;
}

void Ellipse::getExtent (
    float& l, float& b, float& cx, float& cy, float& tol, Graphic* gs
) {
    float width, dummy, bx0, by0, bx1, by1;

    width = float(gs->GetBrush()->Width());
    tol = (width > 1) ? width/2 : 0;
    bx0 = float(x0 - r1);
    by0 = float(y0 - r2);
    bx1 = float(x0 + r1);
    by1 = float(y0 + r2);
    transformRect(bx0, by0, bx1, by1, l, b, dummy, dummy, gs);
    transform(float(x0), float(y0), cx, cy, gs);
}

boolean Ellipse::contains (PointObj& po, Graphic* gs) {
    PointObj pt (&po);
    
    invTransform(pt.x, pt.y, gs);
    return (
	square(r2)*square(pt.x - x0) + square(r1)*square(pt.y - y0) - 
	square(r1*r2) == 0
    );
}

static const float axis = 0.42;
static const float seen = 1.025;

static void EllipseCPs (
    Coord x0, Coord y0, int r1, int r2, Coord x[], Coord y[], Transformer* t
) {
    if (t == nil) {
        Coord px1, py1, px2, py2;

	px1 = round(float(r1)*axis); py1 = round(float(r2)*axis);
	px2 = round(float(r1)*seen); py2 = round(float(r2)*seen);
        x[0] = x0 + px1;    y[0] = y0 + py2;
        x[1] = x0 - px1;    y[1] = y[0];
        x[2] = x0 - px2;    y[2] = y0 + py1;
        x[3] = x[2];        y[3] = y0 - py1;
        x[4] = x[1];	    y[4] = y0 - py2;
        x[5] = x[0];	    y[5] = y[4];
        x[6] = x0 + px2;    y[6] = y[3];
        x[7] = x[6];	    y[7] = y[2];
    
    } else {
        float fx1, fy1, fx2, fy2, tx[8], ty[8], tmpx, tmpy;

        fx1 = float(r1)*axis; fy1 = float(r2)*axis;
        fx2 = float(r1)*seen; fy2 = float(r2)*seen;
        tx[0] = x0 + fx1;   ty[0] = y0 + fy2;
        tx[1] = x0 - fx1;   ty[1] = ty[0];
        tx[2] = x0 - fx2;   ty[2] = y0 + fy1;
        tx[3] = tx[2];      ty[3] = y0 - fy1;
        tx[4] = tx[1];	    ty[4] = y0 - fy2;
        tx[5] = tx[0];	    ty[5] = ty[4];
        tx[6] = x0 + fx2;   ty[6] = ty[3];
        tx[7] = tx[6];	    ty[7] = ty[2];

        for (int i = 0; i < 8; ++i) {
            t->Transform(tx[i], ty[i], tmpx, tmpy);
            x[i] = round(tmpx);
            y[i] = round(tmpy);
        }
    }
}

boolean Ellipse::intersects (BoxObj& userb, Graphic* gs) {
    Coord x[8], y[8];
    Transformer* t;
    MultiLineObj ml;
    BoxObj b;

    getBox(b, gs);
    if (b.Intersects(userb)) {
	t = gs->GetTransformer();
        EllipseCPs(x0, y0, r1, r2, x, y, t);
        ml.ClosedSplineToPolygon(x, y, 8);
	return ml.Intersects(userb);
    }
    return false;
}

void Ellipse::SetBrush (PBrush* brush) {
    if (br() != Ref(brush)) {
	br(Ref(brush));
	invalidateCaches();
    }
}

PBrush* Ellipse::GetBrush () { return (PBrush*) br()(); }
void Ellipse::SetPattern (PPattern* pattern) { pat(Ref(pattern)); }
PPattern* Ellipse::GetPattern () { return (PPattern*) pat()(); }

void Ellipse::pat (Ref) { }
Ref Ellipse::pat () { return nil; }
void Ellipse::br (Ref r) { _patbr = r; }
Ref Ellipse::br () { return _patbr; }

/*****************************************************************************/

void FillEllipse::draw (Canvas* c, Graphic* gs) {
    update(gs);
    pFillEllipse(c, x0, y0, r1, r2);
}

ClassId FillEllipse::GetClassId () { 
    return FILLELLIPSE;
}
boolean FillEllipse::IsA (ClassId id) {
    return FILLELLIPSE == id || Ellipse::IsA(id);
}

FillEllipse::FillEllipse () {}

FillEllipse::FillEllipse (
    Coord x0, Coord y0, int r1, int r2, Graphic* gr
) : (x0, y0, r1, r2, gr) {
    if (gr == nil) {
	SetPattern(nil);
    } else {
	SetPattern(gr->GetPattern());
    }
}

Graphic* FillEllipse::Copy () { 
    return new FillEllipse(x0, y0, r1, r2, this);
}

void FillEllipse::getExtent (
    float& l, float& b, float& cx, float& cy, float& tol, Graphic* gs
) {
    float bx0, by0, bx1, by1, dummy;

    tol = 0;
    bx0 = float(x0 - r1);
    by0 = float(y0 - r2);
    bx1 = float(x0 + r1);
    by1 = float(y0 + r2);
    transformRect(bx0, by0, bx1, by1, l, b, dummy, dummy, gs);
    transform(float(x0), float(y0), cx, cy, gs);
}

boolean FillEllipse::contains (PointObj& po, Graphic* gs) {
    PointObj pt (&po);
    invTransform(pt.x, pt.y, gs);
    return (
        square(r2)*square(pt.x - x0) + square(r1)*square(pt.y - y0) - 
	square(r1*r2)
    ) <= 0;
}

boolean FillEllipse::intersects (BoxObj& userb, Graphic* gs) {
    Coord x[8], y[8];
    Transformer* t;
    FillPolygonObj fp;
    BoxObj b;

    getBox(b, gs);
    if (b.Intersects(userb)) {
	t = gs->GetTransformer();
        EllipseCPs(x0, y0, r1, r2, x, y, t);
        fp.ClosedSplineToPolygon(x, y, 8);
	return fp.Intersects(userb);
    }
    return false;
}

void FillEllipse::pat (Ref r) { _patbr = r; }
Ref FillEllipse::pat () { return _patbr; }
void FillEllipse::br (Ref) { }
Ref FillEllipse::br () { return nil; }

void FillEllipse::SetBrush (PBrush*) { }

/****************************************************************************/

ClassId Circle::GetClassId () { return CIRCLE; }
boolean Circle::IsA (ClassId id) { return CIRCLE == id || Ellipse::IsA(id); }
Circle::Circle () {}

Circle::Circle (
    Coord x0, Coord y0, int radius, Graphic* gr
) : (x0, y0, radius, radius, gr) { }

Graphic* Circle::Copy () { return new Circle(x0, y0, r1, this); }

/*****************************************************************************/

ClassId FillCircle::GetClassId () { return FILLCIRCLE; }

boolean FillCircle::IsA (ClassId id) {
    return FILLCIRCLE==id || FillEllipse::IsA(id);
}

FillCircle::FillCircle () {}

FillCircle::FillCircle (
    Coord x0, Coord y0, int radius, Graphic* gr
) : (x0, y0, radius, radius, gr) { }

Graphic* FillCircle::Copy () { 
    return new FillCircle(x0, y0, r1, this);
}
