/*
 * Implementation of geometrical objects used by Graphic.
 */

#include <InterViews/Graphic/geomobjs.h>
#include <InterViews/Graphic/grclasses.h>
#include <InterViews/Graphic/pfile.h>
#include <InterViews/Graphic/util.h>
#include <bstring.h>
#include <math.h>

static const int NUMPOINTS = 200;	// must be > 1
static const double SMOOTHNESS = 1.0;
static int mlsize = 0;
static int mlcount = 0;
static Coord* mlx, *mly;

/*****************************************************************************/

PointObj::PointObj (Coord x, Coord y) { this->x = x; this->y = y; }
PointObj::PointObj (PointObj* p) { x = p->x; y = p->y; }

float PointObj::Distance (PointObj& p) {
    return sqrt(float(square(x - p.x) + square(y - p.y)));
}

boolean PointObj::read (PFile* f) {
    return Persistent::read(f) && f->Read(x) && f->Read(y);
}

boolean PointObj::write (PFile* f) {
    return Persistent::write(f) && f->Write(x) && f->Write(y);
}

ClassId PointObj::GetClassId () { return POINTOBJ; }

boolean PointObj::IsA (ClassId id) { 
    return POINTOBJ==id || Persistent::IsA(id);
}

/*****************************************************************************/

LineObj::LineObj (Coord x0, Coord y0, Coord x1, Coord y1) {
    p1.x = x0; p1.y = y0; p2.x = x1; p2.y = y1;
}

LineObj::LineObj (LineObj* l) {
    p1.x = l->p1.x; p1.y = l->p1.y; p2.x = l->p2.x; p2.y = l->p2.y;
}

boolean LineObj::read (PFile* f) {
    return Persistent::read(f) &&
	f->Read(p1.x) && f->Read(p1.y) && f->Read(p2.x) && f->Read(p2.y);
}

boolean LineObj::write (PFile* f) {
    return Persistent::write(f) && 
	f->Write(p1.x) && f->Write(p1.y) && f->Write(p2.x) && f->Write(p2.y);
}

ClassId LineObj::GetClassId () { return LINEOBJ; }
boolean LineObj::IsA (ClassId id) { return LINEOBJ==id || Persistent::IsA(id);}
LineObj::LineObj () {}

boolean LineObj::Contains (PointObj& p) {
    return
	(p.x >= min(p1.x, p2.x)) && (p.x <= max(p1.x, p2.x)) &&
	(p.y >= min(p1.y, p2.y)) && (p.y <= max(p1.y, p2.y)) &&
        ((p.y - p1.y)*(p2.x - p1.x) - (p2.y - p1.y)*(p.x - p1.x)) == 0;
}

inline int signum (int a) {
    if (a < 0) {
        return -1;
    } else if (a > 0) {
        return 1;
    } else {
        return 0;
    }
}

int LineObj::Same (PointObj& p1, PointObj& p2) {
    Coord dx, dx1, dx2;
    Coord dy, dy1, dy2;
    
    dx = this->p2.x - this->p1.x;
    dy = this->p2.y - this->p1.y;
    dx1 = p1.x - this->p1.x;
    dy1 = p1.y - this->p1.y;
    dx2 = p2.x - this->p2.x;
    dy2 = p2.y - this->p2.y;

    return signum(dx*dy1 - dy*dx1) * signum(dx*dy2 - dy*dx2);
}

boolean LineObj::Intersects (LineObj& l) {  // from Sedgewick, p. 313
    BoxObj b1 (p1.x, p1.y, p2.x, p2.y);
    BoxObj b2 (l.p1.x, l.p1.y, l.p2.x, l.p2.y);
    
    return b1.Intersects(b2) && Same(l.p1, l.p2) <= 0 && l.Same(p1, p2) <= 0;
}

/*****************************************************************************/

BoxObj::BoxObj (Coord x0, Coord y0, Coord x1, Coord y1) {
    left = min(x0, x1); bottom = min(y0, y1); 
    right = max(x0, x1); top = max(y0, y1);
}

BoxObj::BoxObj (BoxObj* b) {
    left = b->left; bottom = b->bottom; right = b->right; top = b->top;
}

boolean BoxObj::read (PFile* f) {
    return Persistent::read(f) &&
	f->Read(left) && f->Read(bottom) && f->Read(right) && f->Read(top);
}

boolean BoxObj::write (PFile* f) {
    return Persistent::write(f) &&
	f->Write(left) && f->Write(bottom) && f->Write(right) && f->Write(top);
}

ClassId BoxObj::GetClassId () { return BOXOBJ; }
boolean BoxObj::IsA (ClassId id) { return BOXOBJ==id || Persistent::IsA(id); }

boolean BoxObj::Contains (PointObj& p) {
    return (p.x >= left) && (p.x <= right) && (p.y >= bottom) && (p.y <= top);
}

boolean BoxObj::Intersects (BoxObj& b) {
    return (
        (left <= b.right) && (b.left <= right) && 
	(bottom <= b.top) && (b.bottom <= top) 
    ) || (
        (b.left <= right) && (left <= b.right) &&
        (b.bottom <= top) && (bottom <= b.top) 
    );
}

boolean BoxObj::Intersects (LineObj& l) {
    Coord x1 = min(l.p1.x, l.p2.x);
    Coord x2 = max(l.p1.x, l.p2.x);
    Coord y1 = min(l.p1.y, l.p2.y);
    Coord y2 = max(l.p1.y, l.p2.y);
    BoxObj lbox(x1, y1, x2, y2);
    boolean intersects = false;

    if (Intersects(lbox)) {
	intersects = Contains(l.p1) || Contains(l.p2);
        if (!intersects) {
            LineObj l0 (left, bottom, right, bottom);
            LineObj l1 (right, bottom, right, top);
            LineObj l2 (right, top, left, top);
            LineObj l3 (left, top, left, bottom);
            intersects =
	        l.Intersects(l0) || l.Intersects(l1) || 
	        l.Intersects(l2) || l.Intersects(l3);
	}
    }
    return intersects;
}

BoxObj BoxObj::operator- (BoxObj& b) {
    BoxObj i;

    if (Intersects(b)) {
        i.left = max(left, b.left);
	i.bottom = max(bottom, b.bottom);
	i.right = min(right, b.right);
	i.top = min(top, b.top);
    }
    return i;
}

BoxObj BoxObj::operator+ (BoxObj& b) {
    BoxObj m;
    
    m.left = min(left, b.left);
    m.bottom = min(bottom, b.bottom);
    m.right = max(right, b.right);
    m.top = max(top, b.top);
    return m;
}

boolean BoxObj::Within (BoxObj& b) {
    return (
        (left >= b.left) && (bottom >= b.bottom) && 
        (right <= b.right) && (top <= b.top) 
    );
}

/*****************************************************************************/

MultiLineObj::MultiLineObj (Coord* x, Coord* y, int n) {
    this->x = x; this->y = y; count = n;
}

void MultiLineObj::GrowBuf () {
    Coord* newx, *newy;
    int newsize;

    if (mlsize == 0) {
        mlsize = NUMPOINTS;
	mlx = new Coord[NUMPOINTS];
	mly = new Coord[NUMPOINTS];
    } else {
	newsize = mlsize * 2;
	newx = new Coord[newsize];
	newy = new Coord[newsize];
	bcopy(mlx, newx, newsize * sizeof(Coord));
	bcopy(mly, newy, newsize * sizeof(Coord));
	delete mlx;
	delete mly;
	mlx = newx;
	mly = newy;
	mlsize = newsize;
    }
}

boolean MultiLineObj::CanApproxWithLine (
    double x0, double y0, double x2, double y2, double x3, double y3
) {
    double triangleArea, sideSquared, dx, dy;
    
    triangleArea = x0*y2 - x2*y0 + x2*y3 - x3*y2 + x3*y0 - x0*y3;
    triangleArea *= triangleArea;	// actually 4 times the area
    dx = x3 - x0;
    dy = y3 - y0;
    sideSquared = dx*dx + dy*dy;
    return triangleArea <= SMOOTHNESS * sideSquared;
}

void MultiLineObj::AddLine (double x0, double y0, double x1, double y1) {
    if (mlcount >= mlsize) {
	GrowBuf();
    } 
    if (mlcount == 0) {
	mlx[mlcount] = round(x0);
	mly[mlcount] = round(y0);
	++mlcount;
    }
    mlx[mlcount] = round(x1);
    mly[mlcount] = round(y1);
    ++mlcount;
}

void MultiLineObj::AddBezierArc (
     double x0, double y0, double x1, double y1,
     double x2, double y2, double x3, double y3
) {
    double midx01, midx12, midx23, midlsegx, midrsegx, cx,
    	   midy01, midy12, midy23, midlsegy, midrsegy, cy;
    
    Midpoint(x0, y0, x1, y1, midx01, midy01);
    Midpoint(x1, y1, x2, y2, midx12, midy12);
    Midpoint(x2, y2, x3, y3, midx23, midy23);
    Midpoint(midx01, midy01, midx12, midy12, midlsegx, midlsegy);
    Midpoint(midx12, midy12, midx23, midy23, midrsegx, midrsegy);
    Midpoint(midlsegx, midlsegy, midrsegx, midrsegy, cx, cy);    

    if (CanApproxWithLine(x0, y0, midlsegx, midlsegy, cx, cy)) {
        AddLine(x0, y0, cx, cy);
    } else if (
        (midx01 != x1) || (midy01 != y1) || (midlsegx != x2) ||
	(midlsegy != y2) || (cx != x3) || (cy != y3)
    ) {    
        AddBezierArc(x0, y0, midx01, midy01, midlsegx, midlsegy, cx, cy);
    }

    if (CanApproxWithLine(cx, cy, midx23, midy23, x3, y3)) {
        AddLine(cx, cy, x3, y3);
    } else if (
        (cx != x0) || (cy != y0) || (midrsegx != x1) || (midrsegy != y1) ||
	(midx23 != x2) || (midy23 != y2)
    ) {        
        AddBezierArc(cx, cy, midrsegx, midrsegy, midx23, midy23, x3, y3);
    }
}

void MultiLineObj::CalcSection (
    Coord cminus1x, Coord cminus1y, Coord cx, Coord cy,
    Coord cplus1x, Coord cplus1y, Coord cplus2x, Coord cplus2y
) {
    double p0x, p1x, p2x, p3x, tempx,
	   p0y, p1y, p2y, p3y, tempy;
    
    ThirdPoint(
        double(cx), double(cy), double(cplus1x), double(cplus1y), p1x, p1y
    );
    ThirdPoint(
        double(cplus1x), double(cplus1y), double(cx), double(cy), p2x, p2y
    );
    ThirdPoint(
        double(cx), double(cy), double(cminus1x), double(cminus1y),
	tempx, tempy
    );
    Midpoint(tempx, tempy, p1x, p1y, p0x, p0y);
    ThirdPoint(
        double(cplus1x), double(cplus1y), double(cplus2x), double(cplus2y),
	tempx, tempy
    );
    Midpoint(tempx, tempy, p2x, p2y, p3x, p3y);
    AddBezierArc(p0x, p0y, p1x, p1y, p2x, p2y, p3x, p3y);
}

void MultiLineObj::SplineToMultiLine (Coord* cpx, Coord* cpy, int cpcount) {
    register int cpi;
    mlcount = 0;

    CalcSection(
	cpx[0], cpy[0], cpx[0], cpy[0], cpx[0], cpy[0], cpx[1], cpy[1]
    );
    CalcSection(
	cpx[0], cpy[0], cpx[0], cpy[0], cpx[1], cpy[1], cpx[2], cpy[2]
    );

    for (cpi = 1; cpi < cpcount - 2; ++cpi) {
	CalcSection(
	    cpx[cpi - 1], cpy[cpi - 1], cpx[cpi], cpy[cpi],
	    cpx[cpi + 1], cpy[cpi + 1], cpx[cpi + 2], cpy[cpi + 2]
        );
    }

    CalcSection(
	cpx[cpi - 1], cpy[cpi - 1], cpx[cpi], cpy[cpi],
	cpx[cpi + 1], cpy[cpi + 1], cpx[cpi + 1], cpy[cpi + 1]
    );
    CalcSection(
	cpx[cpi], cpy[cpi], cpx[cpi + 1], cpy[cpi + 1],
	cpx[cpi + 1], cpy[cpi + 1], cpx[cpi + 1], cpy[cpi + 1]
    );
    x = mlx;
    y = mly;
    count = mlcount;
}

void MultiLineObj::ClosedSplineToPolygon (Coord* cpx, Coord* cpy, int cpcount) {
    register int cpi;

    if (cpcount < 3) {
        x = cpx;
	y = cpy;
	count = cpcount;
    } else {
        mlcount = 0;
        CalcSection(
	    cpx[cpcount - 1], cpy[cpcount - 1], cpx[0], cpy[0], 
	    cpx[1], cpy[1], cpx[2], cpy[2]
        );

        for (cpi = 1; cpi < cpcount - 2; ++cpi) {
	    CalcSection(
	        cpx[cpi - 1], cpy[cpi - 1], cpx[cpi], cpy[cpi],
	        cpx[cpi + 1], cpy[cpi + 1], cpx[cpi + 2], cpy[cpi + 2]
            );
        }

        CalcSection(
	    cpx[cpi - 1], cpy[cpi - 1], cpx[cpi], cpy[cpi],
	    cpx[cpi + 1], cpy[cpi + 1], cpx[0], cpy[0]
        );
        CalcSection(
	    cpx[cpi], cpy[cpi], cpx[cpi + 1], cpy[cpi + 1],
	    cpx[0], cpy[0], cpx[1], cpy[1]
        );
        x = mlx;
        y = mly;
        count = mlcount;
    }
}

boolean MultiLineObj::read (PFile* f) {
    boolean ok = Persistent::read(f) && f->Read(count);
    if (ok) {
	delete x;
	delete y;
	x = new Coord [count];
	y = new Coord [count];
	ok = f->Read(x, count) && f->Read(y, count);
    }
    return ok;
}

boolean MultiLineObj::write (PFile* f) {
    return Persistent::write(f) &&
	f->Write(count) && f->Write(x, count) && f->Write(y, count);
}

ClassId MultiLineObj::GetClassId () { 
    return MULTILINEOBJ;
}

boolean MultiLineObj::IsA (ClassId id) { 
    return MULTILINEOBJ==id || Persistent::IsA(id);
}

void MultiLineObj::GetBox (BoxObj& b) {
    register int i;

    b.left = b.right = x[0];
    b.bottom = b.top = y[0];

    for (i = 1; i < count; ++i) {
	b.left = min(b.left, x[i]);
	b.bottom = min(b.bottom, y[i]);
	b.right = max(b.right, x[i]);
	b.top = max(b.top, y[i]);
    }
}


boolean MultiLineObj::Contains (PointObj& p) {
    register int i;
    BoxObj b;
    
    GetBox(b);
    if (b.Contains(p)) {
	for (i = 1; i < count; ++i) {
	    LineObj l (x[i-1], y[i-1], x[i], y[i]);
	    if (l.Contains(p)) {
	        return true;
	    }
	}
    }    
    return false;
}

boolean MultiLineObj::Intersects (LineObj& l) {
    register int i;
    BoxObj b;
    
    GetBox(b);
    if (b.Intersects(l)) {
	for (i = 1; i < count; ++i) {
	    if (l.Intersects(LineObj(x[i-1], y[i-1], x[i], y[i]))) {
	        return true;
	    }
	}
    }    
    return false;
}

boolean MultiLineObj::Intersects (BoxObj& userb) {
    register int i;
    BoxObj b;
    
    GetBox(b);
    if (b.Intersects(userb)) {
	for (i = 1; i < count; ++i) {
	    if (userb.Intersects(LineObj(x[i-1], y[i-1], x[i], y[i]))) {
	        return true;
	    }
	}
    }    
    return false;
}

boolean MultiLineObj::Within (BoxObj& userb) {
    BoxObj b;
    
    GetBox(b);
    return b.Within(userb);
}

/*****************************************************************************/

FillPolygonObj::FillPolygonObj(Coord* x, Coord* y, int n) : (x, y, n) {
    Normalize();
}

ClassId FillPolygonObj::GetClassId () { return FILLPOLYGONOBJ; }

boolean FillPolygonObj::IsA (ClassId id) { 
    return FILLPOLYGONOBJ == id || MultiLineObj::IsA(id);
}

FillPolygonObj::~FillPolygonObj () {
    delete normx;
    delete normy;
}

int FillPolygonObj::LowestLeft (Coord* x, Coord* y, int count) {
    register int i;
    int lowestLeft = 0;
    Coord lx = *x;
    Coord ly = *y;

    for (i = 1; i < count; ++i) {
        if (y[i] < ly || (y[i] == ly && x[i] < lx)) {
	    lowestLeft = i;
	    lx = x[i];
	    ly = y[i];
	}
    }
    return lowestLeft;
}

void FillPolygonObj::Normalize () {
    register int i, newcount = 1;
    int lowestLeft, limit = count;

    if (count == 0) {
	normx = normy = nil;
	normCount = 0;

    } else {
	if (*x == x[count - 1] && *y == y[count - 1]) {
	    --limit;
	}
	lowestLeft = LowestLeft(x, y, limit);
	normCount = limit + 2;
	normx = new Coord[normCount];
	normy = new Coord[normCount];
	for (i = lowestLeft; i < limit; ++i, ++newcount) {
	    normx[newcount] = x[i];
	    normy[newcount] = y[i];
	}
	for (i = 0; i < lowestLeft; ++i, ++newcount) {
	    normx[newcount] = x[i];
	    normy[newcount] = y[i];
	}
	normx[newcount] = normx[1];
	normy[newcount] = normy[1];
	--newcount;
	normx[0] = normx[newcount];
	normy[0] = normy[newcount];
    }
}

boolean FillPolygonObj::Contains (PointObj& p) {  // from Sedgewick, p. 317
    register int i, j = 0;
    int count = 0;
    LineObj lt, lp;
    BoxObj b;

    if (normCount == 0) {
	Normalize();
    }

    lt.p1 = lt.p2 = p;
    GetBox(b);
    lt.p2.x = b.right + 1;
    
    for (i = 1; i < normCount - 1; ++i) {
        lp.p1.x = lp.p2.x = normx[i];
        lp.p1.y = lp.p2.y = normy[i];

        if (!lp.Intersects(lt)) {
            lp.p2.x = normx[j];
	    lp.p2.y = normy[j];
	    j = i;
	    if (lp.Intersects(lt)) {
	        ++count;
	    }
	}
    }
    return count % 2 == 1;
}

boolean FillPolygonObj::Intersects (LineObj& l) {
    BoxObj b;
    boolean intersects = false;
    
    if (normCount == 0) {
	Normalize();
    }
    GetBox(b);
    if (b.Intersects(l)) {
        MultiLineObj ml (normx, normy, normCount - 1);
	intersects = ml.Intersects(l) || Contains(l.p1) || Contains(l.p2);
    }
    return intersects;
}

boolean FillPolygonObj::Intersects (BoxObj& ub) {
    BoxObj b;
    
    GetBox(b);
    if (!b.Intersects(ub)) {
	return false;
    }
    if (b.Within(ub)) {
	return true;
    }
    if (Intersects(LineObj(ub.left, ub.bottom, ub.right, ub.bottom))) {
	return true;
    }
    if (Intersects(LineObj(ub.right, ub.bottom, ub.right, ub.top))) {
	return true;
    }
    if (Intersects(LineObj(ub.right, ub.top, ub.left, ub.top))) {
	return true;
    }
    return Intersects(LineObj(ub.left, ub.top, ub.left, ub.bottom));
}

/*****************************************************************************/

Extent::Extent (float x0, float y0, float x1, float y1, float t) {
    left = x0; bottom = y0; cx = x1; cy = y1; tol = t;
}

Extent::Extent (Extent& e) {
    left = e.left; bottom = e.bottom; cx = e.cx; cy = e.cy; tol = e.tol;
}

boolean Extent::Within (Extent& e) {
    float l = left - tol, b = bottom - tol;
    float el = e.left - tol, eb = e.bottom - tol;

    return 
	l >= el && b >= eb && 2*cx - l <= 2*e.cx - el && 
	2*cy - b <= 2*e.cy - eb;
    }

void Extent::Merge (Extent& e) {
    float nl = fmin(left, e.left);
    float nb = fmin(bottom, e.bottom);

    if (Undefined()) {
	left = e.left; bottom = e.bottom; cx = e.cy; cy = e.cy;
    } else if (!e.Undefined()) {
	cx = (nl + fmax(2*cx - left, 2*e.cx - e.left)) / 2;
	cy = (nb + fmax(2*cy - bottom, 2*e.cy - e.bottom)) / 2;
	left = nl;
	bottom = nb;
    }
    tol = fmax(tol, e.tol);
}
