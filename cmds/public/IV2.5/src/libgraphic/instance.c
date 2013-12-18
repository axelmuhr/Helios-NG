/*
 * Instance class implementation.  An Instance is a Graphic that is a
 * variation of another Graphic.
 */

#include <InterViews/canvas.h>
#include <InterViews/transformer.h>
#include <InterViews/Graphic/grclasses.h>
#include <InterViews/Graphic/instance.h>

boolean Instance::read (PFile* f) {
    return FullGraphic::read(f) && refGr.Read(f);
}

boolean Instance::write (PFile* f) {
    return FullGraphic::write(f) && refGr.Write(f);
}

ClassId Instance::GetClassId () { return INSTANCE; }

boolean Instance::IsA (ClassId id) {
    return INSTANCE == id || FullGraphic::IsA(id);
}

Instance::Instance (Graphic* obj, Graphic* gr) : (gr) { refGr = Ref(obj); }
Graphic* Instance::Copy () { return new Instance(getGraphic(), this); }
void Instance::GetOriginal (Graphic*& gr) { gr = getGraphic(); }

void Instance::draw (Canvas* c, Graphic* gs) {
    Graphic* gr = getGraphic();
    FullGraphic gstemp;
    Transformer ttemp;

    gstemp.SetTransformer(&ttemp);
    concatGraphic(gr, gr, gs, &gstemp);
    drawGraphic(gr, c, &gstemp);
    gstemp.SetTransformer(nil);
}

void Instance::drawClipped (
    Canvas* c, Coord l, Coord b, Coord r, Coord t, Graphic* gs
) {
    Graphic* gr = getGraphic();
    FullGraphic gstemp;
    Transformer ttemp;
    BoxObj box, clipBox(l, b, r, t);

    cachingOff();
    getBox(box, gs);
    if (clipBox.Intersects(box)) {
	gstemp.SetTransformer(&ttemp);
	concatGraphic(gr, gr, gs, &gstemp);
	drawClippedGraphic(gr, c, l, b, r, t, &gstemp);
	gstemp.SetTransformer(nil);
    }
    cachingOn();
}

void Instance::getExtent (
    float& x0, float& y0, float& cx, float& cy, float& tol, Graphic* gs
) {
    Graphic* gr = getGraphic();
    FullGraphic gstemp;
    Transformer ttemp;

    gstemp.SetTransformer(&ttemp);
    concatGraphic(gr, gr, gs, &gstemp);
    cachingOff();
    getExtentGraphic(gr, x0, y0, cx, cy, tol, &gstemp);
    cachingOn();
    gstemp.SetTransformer(nil);
}

boolean Instance::contains (PointObj& po, Graphic* gs) {
    Graphic* gr = getGraphic();
    FullGraphic gstemp;
    Transformer ttemp;
    boolean result;

    gstemp.SetTransformer(&ttemp);
    concatGraphic(gr, gr, gs, &gstemp);
    cachingOff();
    result = containsGraphic(gr, po, &gstemp);
    cachingOn();
    gstemp.SetTransformer(nil);
    return result;
}

boolean Instance::intersects (BoxObj& b, Graphic* gs) {
    Graphic* gr = getGraphic();
    FullGraphic gstemp;
    Transformer ttemp;
    boolean result;

    gstemp.SetTransformer(&ttemp);
    concatGraphic(gr, gr, gs, &gstemp);
    cachingOff();
    result = intersectsGraphic(gr, b, &gstemp);
    cachingOn();
    gstemp.SetTransformer(nil);
    return result;
}
