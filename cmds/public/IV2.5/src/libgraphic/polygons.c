/*
 * Implementation of Rectangles and Polygons, objects derived from Graphic.
 */

#include <InterViews/transformer.h>
#include <InterViews/Graphic/grclasses.h>
#include <InterViews/Graphic/polygons.h>
#include <InterViews/Graphic/util.h>

boolean Rect::read (PFile* f) {
    return Graphic::read(f) && _patbr.Read(f) &&
	f->Read(x0) && f->Read(y0) && f->Read(x1) && f->Read(y1);
}

boolean Rect::write (PFile* f) {
    return Graphic::write(f) && _patbr.Write(f) &&
	f->Write(x0) && f->Write(y0) && f->Write(x1) && f->Write(y1);
}

void Rect::draw (Canvas *c, Graphic* gs) {
    if (gs->GetBrush()->Width() != NO_WIDTH) {
	update(gs);
	pRect(c, x0, y0, x1, y1);
    }
}

ClassId Rect::GetClassId () { 
    return RECT;
}

boolean Rect::IsA (ClassId id) { 
    return RECT == id || Graphic::IsA(id);
}

Rect::Rect () { }

Rect::Rect (
    Coord x0, Coord y0, Coord x1, Coord y1, Graphic* gr
) : (gr) {
    if (gr == nil) {
	SetBrush(nil);
    } else {
	SetBrush(gr->GetBrush());
    }
    this->x0 = min(x0, x1);
    this->y0 = min(y0, y1);
    this->x1 = max(x0, x1);
    this->y1 = max(y0, y1);
}

Graphic* Rect::Copy () { 
    return new Rect(x0, y0, x1, y1, this);
}

void Rect::GetOriginal (Coord& x0, Coord& y0, Coord& x1, Coord& y1) {
    x0 = this->x0;
    y0 = this->y0;
    x1 = this->x1;
    y1 = this->y1;
}

void Rect::getExtent (
    float& l, float& b, float& cx, float& cy, float& tol, Graphic* gs
) {
    float width, dummy;

    width = float(gs->GetBrush()->Width());
    tol = (width > 1) ? width/2 : 0;
    transformRect(x0, y0, x1, y1, l, b, dummy, dummy, gs);
    transform(float(x0+x1)/2, float(y0+y1)/2, cx, cy, gs);
}

boolean Rect::contains (PointObj& po, Graphic* gs) {
    PointObj pt (&po);
    invTransform(pt.x, pt.y, gs);
    return (
        ((pt.x == x0 || pt.x == x1) && y0 <= pt.y && pt.y <= y1) ||
	((pt.y == y0 || pt.y == y1) && x0 <= pt.x && pt.x <= x1)
    );
}

boolean Rect::intersects (BoxObj& userb, Graphic* gs) {
    Coord x[4], tx[5];
    Coord y[4], ty[5];
    
    x[0] = x[3] = this->x0;
    y[0] = y[1] = this->y0;
    x[2] = x[1] = this->x1;
    y[2] = y[3] = this->y1;
    transformList(x, y, 4, tx, ty, gs);
    tx[4] = tx[0];
    ty[4] = ty[0];
    MultiLineObj ml (tx, ty, 5);
    return ml.Intersects(userb) || ml.Within(userb);
}

void Rect::SetBrush (PBrush* brush) {
    if (br() != Ref(brush)) {
        br(Ref(brush));
	invalidateCaches();
    }
}

PBrush* Rect::GetBrush () { return (PBrush*) br()(); }
void Rect::SetPattern (PPattern* pattern) { pat(Ref(pattern)); }
PPattern* Rect::GetPattern () { return (PPattern*) pat()(); }

void Rect::pat (Ref) { }
Ref Rect::pat () { return nil; }
void Rect::br (Ref r) { _patbr = r; }
Ref Rect::br () { return _patbr; }

/*****************************************************************************/

void FillRect::draw (Canvas *c, Graphic* gs) {
    update(gs);
    pFillRect(c, x0, y0, x1, y1);
}

ClassId FillRect::GetClassId () { return FILLRECT; }
boolean FillRect::IsA (ClassId id) { return FILLRECT == id || Rect::IsA(id); }
FillRect::FillRect () { }

FillRect::FillRect (
    Coord x0, Coord y0, Coord x1, Coord y1, Graphic* gr
) : (x0, y0, x1, y1, gr) {
    if (gr == nil) {
	SetPattern(nil);
    } else {
	SetPattern(gr->GetPattern());
    }
}

Graphic* FillRect::Copy () { 
    return new FillRect(x0, y0, x1, y1, this);
}

void FillRect::getExtent (
    float& l, float& b, float& cx, float& cy, float& tol, Graphic* gs
){
    float dummy;

    transformRect(x0, y0, x1, y1, l, b, dummy, dummy, gs);
    transform(float(x0+x1)/2, float(y0+y1)/2, cx, cy, gs);
    tol = 0;
}

boolean FillRect::contains (PointObj& po, Graphic* gs) {
    PointObj pt (&po);
    invTransform(pt.x, pt.y, gs);
    BoxObj b (x0, y0, x1, y1);
    return b.Contains(pt);
}

boolean FillRect::intersects (BoxObj& userb, Graphic* gs) {
    Transformer* t = gs->GetTransformer();
    Coord tx0, ty0, tx1, ty1;
    
    if (t != nil && t->Rotated()) {
	Coord x[4], tx[5];
	Coord y[4], ty[5];
    
	x[0] = x[3] = this->x0;
	y[0] = y[1] = this->y0;
	x[2] = x[1] = this->x1;
	y[2] = y[3] = this->y1;
	transformList(x, y, 4, tx, ty, gs);
	tx[4] = tx[0];
	ty[4] = ty[0];
	FillPolygonObj fp (tx, ty, 5);
	return fp.Intersects(userb);
    
    } else if (t != nil) {
	t->Transform(x0, y0, tx0, ty0);
	t->Transform(x1, y1, tx1, ty1);
	BoxObj b1 (tx0, ty0, tx1, ty1);
	return b1.Intersects(userb);

    } else {
	BoxObj b2 (x0, y0, x1, y1);
	return b2.Intersects(userb);
    }
}

void FillRect::pat (Ref r) { _patbr = r; }
Ref FillRect::pat () { return _patbr; }
void FillRect::br (Ref) { }
Ref FillRect::br () { return nil; }

void FillRect::SetBrush (PBrush*) { }

/*****************************************************************************/

void Polygon::draw (Canvas *c, Graphic* gs) {
    if (gs->GetBrush()->Width() != NO_WIDTH) {
	update(gs);
	pPolygon(c, x, y, count);
    }
}

ClassId Polygon::GetClassId () { 
    return POLYGON;
}

boolean Polygon::IsA (ClassId id) { 
    return POLYGON == id || MultiLine::IsA(id);
}

Polygon::Polygon () { }

Polygon::Polygon (
    Coord* x, Coord* y, int count, Graphic* gr
) : (x, y, count, gr) { }

Graphic* Polygon::Copy () { 
    return new Polygon(x, y, count, this);
}

boolean Polygon::contains (PointObj& po, Graphic* gs) {
    BoxObj b;
    PointObj pt (&po);
    
    getBox(b, gs);
    if (b.Contains(pt)) {
	MultiLineObj ml (x, y, count);
	LineObj l (x[count - 1], y[count - 1], *x, *y);
	invTransform(pt.x, pt.y, gs);
	return ml.Contains(pt) || l.Contains(pt);
    }
    return false;
}

boolean Polygon::intersects (BoxObj& userb, Graphic* gs) {
    Coord* convx, *convy;
    BoxObj b;
    boolean result = false;
    
    getBox(b, gs);
    if (b.Intersects(userb)) {
	convx = new Coord[count + 1];
	convy = new Coord[count + 1];
	transformList(x, y, count, convx, convy, gs);
	convx[count] = *convx;
	convy[count] = *convy;
	MultiLineObj ml (convx, convy, count + 1);
	result = ml.Intersects(userb);
	delete convx;
	delete convy;
    }
    return result;
}

/*****************************************************************************/

void FillPolygon::draw (Canvas *c, Graphic* gs) {
    update(gs);
    pFillPolygon(c, x, y, count);
}

ClassId FillPolygon::GetClassId () { 
    return FILLPOLYGON;
}

boolean FillPolygon::IsA (ClassId id) { 
    return FILLPOLYGON == id || Polygon::IsA(id);
}

FillPolygon::FillPolygon () { }

FillPolygon::FillPolygon (
    Coord* x, Coord* y, int count, Graphic* gr
) : (x, y, count, gr) {
    if (gr == nil) {
	SetPattern(nil);
    } else {
	SetPattern(gr->GetPattern());
    }
}

Graphic* FillPolygon::Copy () { 
    return new FillPolygon(x, y, count, this);
}

void FillPolygon::getExtent (
    float& l, float& b, float& cx, float& cy, float& tol, Graphic* gs
) {
    register int i;
    float bx0, by0, bx1, by1, tcx, tcy, dummy;
	
    if (extentCached()) {
	getCachedExtent(bx0, by0, tcx, tcy, tol);
	bx1 = 2*tcx - bx0;
	by1 = 2*tcy - by0;
    } else {
	bx0 = bx1 = x[0]; by0 = by1 = y[0];
	for (i = 1; i < count; ++i) {
	    bx0 = fmin(bx0, x[i]);
	    by0 = fmin(by0, y[i]);
	    bx1 = fmax(bx1, x[i]);
	    by1 = fmax(by1, y[i]);
	}
	tcx = (bx0 + bx1) / 2;
	tcy = (by0 + by1) / 2;
	tol = 0;
	cacheExtent(bx0, by0, tcx, tcy, tol);
    }
    transformRect(bx0, by0, bx1, by1, l, b, dummy, dummy, gs);
    transform(tcx, tcy, cx, cy, gs);
}

boolean FillPolygon::contains (PointObj& po, Graphic* gs) {
    BoxObj b;
    PointObj pt (&po);

    getBox(b, gs);
    if (b.Contains(pt)) {
	FillPolygonObj fp (x, y, count);
	invTransform(pt.x, pt.y, gs);
	return fp.Contains(pt);
    }
    return false;
}

boolean FillPolygon::intersects (BoxObj& userb, Graphic* gs) {
    Coord* convx, *convy;
    BoxObj b;
    boolean result = false;
    
    getBox(b, gs);
    if (b.Intersects(userb)) {
	convx = new Coord[count + 1];
	convy = new Coord[count + 1];
	transformList(x, y, count, convx, convy, gs);
	FillPolygonObj fp (convx, convy, count);
	result = fp.Intersects(userb);
	delete convx;
	delete convy;
    }
    return result;    
}

void FillPolygon::pat (Ref r) { _patbr = r; }
Ref FillPolygon::pat () { return _patbr; }
void FillPolygon::br (Ref) { }
Ref FillPolygon::br () { return nil; }

void FillPolygon::SetBrush (PBrush*) { }
