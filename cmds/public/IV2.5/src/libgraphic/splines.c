/*
 * Implementation of BSplines and ClosedBSplines, objects derived from
 * Graphic.
 */

#include <InterViews/Graphic/grclasses.h>
#include <InterViews/Graphic/splines.h>
#include <InterViews/Graphic/util.h>

void BSpline::draw (Canvas *c, Graphic* gs) {
    if (gs->GetBrush()->Width() != NO_WIDTH) {
	update(gs);
	pBSpline(c, x, y, count);
    }
}

ClassId BSpline::GetClassId () { 
    return BSPLINE;
}

boolean BSpline::IsA (ClassId id) { 
    return BSPLINE == id || MultiLine::IsA(id);
}

BSpline::BSpline () {}

BSpline::BSpline (
    Coord* x, Coord* y, int count, Graphic* gr
) : (x, y, count, gr) { }

Graphic* BSpline::Copy () { 
    return new BSpline(x, y, count, this);
}

boolean BSpline::contains (PointObj& po, Graphic* gs) {
    PointObj pt (&po);
    BoxObj b;

    getBox(b, gs);
    if (b.Contains(pt)) {
	invTransform(pt.x, pt.y, gs);
	MultiLineObj ml;
	ml.SplineToMultiLine(x, y, count);
	return ml.Contains(pt);
    }
    return false;
}

boolean BSpline::intersects (BoxObj& userb, Graphic* gs) {
    Coord* convx, *convy;
    BoxObj b;
    boolean result = false;
    
    getBox(b, gs);
    if (b.Intersects(userb)) {
	convx = new Coord[count];
	convy = new Coord[count];
	transformList(x, y, count, convx, convy, gs);
	MultiLineObj ml;
	ml.SplineToMultiLine(convx, convy, count);
	result = ml.Intersects(userb);
	delete convx;
	delete convy;
    }
    return result;
}

void ClosedBSpline::draw (Canvas *c, Graphic* gs) {
    if (gs->GetBrush()->Width() != NO_WIDTH) {
	update(gs);
	pClosedBSpline(c, x, y, count);
    }
}

ClassId ClosedBSpline::GetClassId () { 
    return CLOSEDBSPLINE;
}

boolean ClosedBSpline::IsA (ClassId id) { 
    return CLOSEDBSPLINE == id || BSpline::IsA(id);
}

ClosedBSpline::ClosedBSpline () { }

ClosedBSpline::ClosedBSpline (
    Coord* x, Coord* y, int count, Graphic* gr
) : (x, y, count, gr) { }

Graphic* ClosedBSpline::Copy () { 
    return new ClosedBSpline(x, y, count, this);
}

boolean ClosedBSpline::contains (PointObj& po, Graphic* gs) {
    PointObj pt (&po);
    BoxObj b;

    getBox(b, gs);
    if (b.Contains(pt)) {
	invTransform(pt.x, pt.y, gs);
	MultiLineObj ml;
	ml.ClosedSplineToPolygon(x, y, count);
	return ml.Contains(pt);
    }
    return false;
}

boolean ClosedBSpline::intersects (BoxObj& userb, Graphic* gs) {
    Coord* convx, *convy;
    BoxObj b;
    boolean result = false;
    
    getBox(b, gs);
    if (b.Intersects(userb)) {
	convx = new Coord[count];
	convy = new Coord[count];
	transformList(x, y, count, convx, convy, gs);
	MultiLineObj ml;
	ml.ClosedSplineToPolygon(convx, convy, count);
	result = ml.Intersects(userb);
	delete convx;
	delete convy;
    }
    return result;
}

void FillBSpline::draw (Canvas *c, Graphic* gs) {
    update(gs);
    pFillBSpline(c, x, y, count);
}

ClassId FillBSpline::GetClassId () { 
    return FILLBSPLINE;
}

boolean FillBSpline::IsA (ClassId id) {
    return FILLBSPLINE==id ||ClosedBSpline::IsA(id);
}

FillBSpline::FillBSpline () { }

FillBSpline::FillBSpline (
    Coord* x, Coord* y, int count, Graphic* gr
) : (x, y, count, gr) {
    if (gr == nil) {
	SetPattern(nil);
    } else {
	SetPattern(gr->GetPattern());
    }
}

Graphic* FillBSpline::Copy () { 
    return new FillBSpline(x, y, count, this);
}

void FillBSpline::getExtent (
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

boolean FillBSpline::contains (PointObj& po, Graphic* gs) {
    PointObj pt (&po);
    BoxObj b;

    getBox(b, gs);
    if (b.Contains(pt)) {
	invTransform(pt.x, pt.y, gs);
	FillPolygonObj fp;
	fp.ClosedSplineToPolygon(x, y, count);
	return fp.Contains(pt);
    }
    return false;
}

boolean FillBSpline::intersects (BoxObj& userb, Graphic* gs) {
    Coord* convx, *convy;
    BoxObj b;
    boolean result = false;
    
    getBox(b, gs);
    if (b.Intersects(userb)) {
	convx = new Coord[count];
	convy = new Coord[count];
	transformList(x, y, count, convx, convy, gs);
	FillPolygonObj fp;
	fp.ClosedSplineToPolygon(convx, convy, count);
	result = fp.Intersects(userb);
	delete convx;
	delete convy;
    }
    return result;
}

void FillBSpline::pat (Ref r) { _patbr = r; }
Ref FillBSpline::pat () { return _patbr; }
void FillBSpline::br (Ref) { }
Ref FillBSpline::br () { return nil; }

void FillBSpline::SetBrush (PBrush*) { }
