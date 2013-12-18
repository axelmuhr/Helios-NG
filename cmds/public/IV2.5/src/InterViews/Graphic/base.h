/*
 * Interface to Graphic base class and FullGraphic, a subclass of Graphic
 * for which all graphics state is defined.
 */

#ifndef base_h
#define base_h

#include <InterViews/painter.h>
#include <InterViews/Graphic/geomobjs.h>
#include <InterViews/Graphic/ppaint.h>

static const int UNDEF = -1;

class Canvas;
class GraphicToPainter;

class Graphic : public Persistent {
public:
    Graphic(Graphic* gr = nil);
    virtual ~Graphic();

    virtual void Draw(Canvas*);
    virtual void Draw(Canvas*, Coord, Coord, Coord, Coord);
    virtual void DrawClipped(Canvas*, Coord, Coord, Coord, Coord);
    virtual void Erase(Canvas*);
    virtual void Erase(Canvas*, Coord, Coord, Coord, Coord);
    virtual void EraseClipped(Canvas*, Coord, Coord, Coord, Coord);

    virtual void FillBg(boolean);
    virtual int BgFilled();
    virtual void SetColors(PColor* f, PColor* b);
    virtual PColor* GetFgColor();
    virtual PColor* GetBgColor();
    virtual void SetPattern(PPattern*);
    virtual PPattern* GetPattern();
    virtual void SetBrush(PBrush*);
    virtual PBrush* GetBrush();
    virtual void SetFont(PFont*);
    virtual PFont* GetFont();

    void Translate(float dx, float dy);
    void Scale(float sx, float sy, float ctrx = 0.0, float ctry = 0.0);
    void Rotate(float angle, float ctrx = 0.0, float ctry = 0.0);
    void Align(Alignment, Graphic*, Alignment);
    void SetTransformer(Transformer*);
    Transformer* GetTransformer();
    void TotalTransformation(Transformer&);

    void GetBounds(float&, float&, float&, float&);
    void GetBox(Coord&, Coord&, Coord&, Coord&);
    void GetBox(BoxObj&);
    virtual void GetCenter(float&, float&);    
    virtual boolean Contains(PointObj&);
    virtual boolean Intersects(BoxObj&);

    void SetTag(Ref);
    Ref GetTag();

    Graphic* Parent();
    virtual boolean HasChildren ();
    virtual Graphic& operator = (Graphic&);
    virtual Graphic* Copy();
    virtual Persistent* GetCluster();
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
protected:
/*
 * Member functions that declare a "Graphic* gs" parameter
 * take into account that graphic's graphic state information when
 * performing their function.  This is useful in hierarchical graphic objects
 * (such as Pictures) where higher-level graphics' graphics state influences
 * lower (and ultimately leaf) graphics'.
 */
    void update(Graphic* gs);		    /* updates painter w/gs' state */
    virtual void draw(Canvas*, Graphic* gs);
    virtual void drawClipped(Canvas*, Coord, Coord, Coord, Coord, Graphic* gs);
    virtual void erase(Canvas*, Graphic* gs);
    virtual void eraseClipped(Canvas*, Coord, Coord, Coord, Coord,Graphic* gs);
/*
 * Bounding box operations.
 */
    virtual void getExtent(float&, float&, float&, float&, float&,Graphic* gs);
        /* Returns lower-left and center coordinates, and a tolerance (tol)
	 * (in canvas coordinates) by which the final extent will be grown
	 * in each direction (i.e. l-=tol, b-=tol, r+=tol, t+=tol).
	 */
    void GetExtent(Extent& e);
    void getBounds(float&, float&, float&, float&, Graphic* gs);
    void getBox(Coord&, Coord&, Coord&, Coord&, Graphic* gs);
    void getBox(BoxObj&, Graphic* gs);
    virtual boolean contains(PointObj&, Graphic* gs);
    virtual boolean intersects(BoxObj&, Graphic* gs);
/*
 * Parent-related operations.
 */
    Graphic* getRoot();			    /* top level parent */
    void totalGS(Graphic& p);
    void parentXform(Transformer& t);	    /* parents' transformation */
    void setParent(Graphic*, Graphic* parent);
    void unsetParent(Graphic*);
/*
 * Bounding box caching operations.
 */
    void cachingOn();
    void cachingOff();
    virtual boolean extentCached();
    virtual void uncacheExtent();
    virtual void uncacheParents();
    virtual void uncacheChildren();
    virtual void invalidateCaches();
/*
 * Graphics state concatentation operations.
 */
    virtual void concatGS(Graphic* a, Graphic* b, Graphic* dest);
    virtual void concatTransformer(
        Transformer* a, Transformer* b, Transformer* dest
    );
    virtual void concat(Graphic* a, Graphic* b, Graphic* dest);
/*
 * Convenient transformations that check first if there's a transformer and
 * then perform the (inverse) transformation.  The functions use the
 * transformer of the supplied Graphic if there is one; otherwise this'
 * transformer is used.
 */
    void transform(Coord& x, Coord& y, Graphic* = nil);
    void transform(Coord x, Coord y, Coord& tx, Coord& ty, Graphic* = nil);
    void transform(float x, float y, float& tx, float& ty, Graphic* = nil);
    void transformList(
        Coord x[], Coord y[], int n, Coord tx[], Coord ty[], Graphic* = nil
    );
    void transformRect(
	float, float, float, float, 
        float&, float&, float&, float&, Graphic* = nil
    );
    void invTransform(Coord& tx, Coord& ty, Graphic* = nil);
    void invTransform(Coord tx, Coord ty, Coord& x, Coord& y, Graphic* = nil);
    void invTransform(float tx, float ty, float& x, float& y, Graphic* = nil);
    void invTransformList(
        Coord tx[], Coord ty[], int n, Coord x[], Coord y[], Graphic* = nil
    );
/*
 * Painter-equivalent rendering operations.  Graphic subclasses should use
 * to draw themselves instead of using a painter directly.
 */
    void pText(Canvas* c, char* s, int n, Coord x, Coord y);
    void pPoint(Canvas* c, Coord x, Coord y);
    void pMultiPoint(Canvas*, Coord x[], Coord y[], int);
    void pLine(Canvas*, Coord, Coord, Coord, Coord);
    void pRect(Canvas*, Coord, Coord, Coord, Coord);
    void pFillRect(Canvas*, Coord, Coord, Coord, Coord);
    void pRasterRect(Canvas*, Coord, Coord, Raster*);
    void pStencil(Canvas*, Coord, Coord, Bitmap*, Bitmap*);
    void pCircle(Canvas*, Coord, Coord, int);
    void pFillCircle(Canvas*, Coord, Coord, int);
    void pEllipse(Canvas*, Coord, Coord, int, int);
    void pFillEllipse(Canvas*, Coord, Coord, int, int);
    void pMultiLine(Canvas*, Coord x[], Coord y[], int);
    void pPolygon(Canvas*, Coord x[], Coord y[], int);
    void pFillPolygon(Canvas*, Coord x[], Coord y[], int);
    void pBSpline(Canvas*, Coord x[], Coord y[], int);
    void pClosedBSpline(Canvas*, Coord x[], Coord y[], int);
    void pFillBSpline(Canvas*, Coord x[], Coord y[], int);
/*
 * "Helper" functions that allow graphic subclasses to call
 * the protected member functions redefined by other graphic subclasses.
 */
    void drawGraphic(Graphic*, Canvas*, Graphic*);
    void drawClippedGraphic(Graphic*,Canvas*,Coord,Coord,Coord,Coord,Graphic*);
    void eraseGraphic(Graphic*, Canvas*, Graphic*);
    void eraseClippedGraphic(
        Graphic*, Canvas*, Coord, Coord, Coord, Coord, Graphic*
    );

    void getExtentGraphic(
        Graphic*, float&, float&, float&, float&, float&, Graphic* gs
    );
    boolean containsGraphic(Graphic*, PointObj&, Graphic* gs);
    boolean intersectsGraphic(Graphic*, BoxObj&, Graphic* gs);

    boolean extentCachedGraphic(Graphic*);
    void uncacheExtentGraphic(Graphic*);
    void uncacheParentsGraphic(Graphic*);
    void uncacheChildrenGraphic(Graphic*);
    void invalidateCachesGraphic(Graphic*);

    void concatGSGraphic(Graphic*, Graphic*, Graphic*, Graphic*);
    void concatTransformerGraphic(
        Graphic*,Transformer*, Transformer*, Transformer*
    );
    void concatGraphic(Graphic*, Graphic*, Graphic*, Graphic*);
/*
 * Persistent read/write operations.
 */
    virtual boolean read(PFile*);
    virtual boolean write(PFile*);
protected:
    static Transformer* identity;   /* identity matrix */
    static boolean caching;	    /* state of bounding box caching */
private:
    Ref parent;
    int fillBg;
    Ref fg;
    Ref bg;
    Ref tag;
    Transformer* t;
    static GraphicToPainter* painters;
    static Painter* p;
};

class FullGraphic : public Graphic {
public:
    FullGraphic(Graphic* = nil);

    virtual void SetPattern(PPattern*);
    virtual PPattern* GetPattern();
    virtual void SetBrush(PBrush*);
    virtual PBrush* GetBrush();
    virtual void SetFont(PFont*);
    virtual PFont* GetFont();

    virtual Graphic* Copy();
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
protected:
    virtual boolean read(PFile*);
    virtual boolean write(PFile*);
private:
    Ref pat;
    Ref brush;
    Ref font;
};

/*
 * inlines
 */

inline Transformer* Graphic::GetTransformer () { return t; }
inline Graphic* Graphic::Parent () { return (Graphic*) parent(); }
inline void Graphic::SetTag (Ref r) { tag = r; }
inline Ref Graphic::GetTag () { return tag; }

inline void Graphic::GetBox (BoxObj& b) { 
    GetBox(b.left, b.bottom, b.right, b.top);
}

inline void Graphic::getBox (BoxObj& b, Graphic* p) {
    getBox(b.left, b.bottom, b.right, b.top, p);
}

inline void Graphic::pText (Canvas* c, char* s, int n, Coord x, Coord y) {
    p->Text(c, s, n, x, y);
}

inline void Graphic::pPoint (Canvas* c, Coord x, Coord y) {
    p->Point(c, x, y);
}

inline void Graphic::pMultiPoint(Canvas* c, Coord x[], Coord y[], int n) {
    p->MultiPoint(c, x, y, n);
}

inline void Graphic::pLine(Canvas* c, Coord x1, Coord y1, Coord x2, Coord y2) {
    p->Line(c, x1, y1, x2, y2);
}

inline void Graphic::pRect(Canvas* c, Coord x1, Coord y1, Coord x2, Coord y2) {
    p->Rect(c, x1, y1, x2, y2);
}

inline void Graphic::pFillRect(Canvas* c,Coord x1,Coord y1, Coord x2,Coord y2){
    p->FillRect(c, x1, y1, x2, y2);
}

inline void Graphic::pRasterRect(Canvas* c, Coord x, Coord y, Raster* r){
    p->RasterRect(c, x, y, r);
}

inline void Graphic::pStencil(Canvas* c,Coord x,Coord y, Bitmap* i, Bitmap* m){
    p->Stencil(c, x, y, i, m);
}

inline void Graphic::pCircle(Canvas* c, Coord x, Coord y, int r) {
    p->Circle(c, x, y, r);
}

inline void Graphic::pFillCircle(Canvas* c, Coord x, Coord y, int r) {
    p->FillCircle(c, x, y, r);
}

inline void Graphic::pEllipse(Canvas* c, Coord x, Coord y, int r1, int r2) {
    p->Ellipse(c, x, y, r1, r2);
}

inline void Graphic::pFillEllipse(Canvas* c, Coord x, Coord y, int r1, int r2){
    p->FillEllipse(c, x, y, r1, r2);
}

inline void Graphic::pMultiLine(Canvas* c, Coord x[], Coord y[], int n) {
    p->MultiLine(c, x, y, n);
}

inline void Graphic::pPolygon(Canvas* c, Coord x[], Coord y[], int n) {
    p->Polygon(c, x, y, n);
}

inline void Graphic::pFillPolygon(Canvas* c, Coord x[], Coord y[], int n) {
    p->FillPolygon(c, x, y, n);
}

inline void Graphic::pBSpline(Canvas* c, Coord x[], Coord y[], int n) {
    p->BSpline(c, x, y, n);
}

inline void Graphic::pClosedBSpline(Canvas* c, Coord x[], Coord y[], int n) {
    p->ClosedBSpline(c, x, y, n);
}

inline void Graphic::pFillBSpline(Canvas* c, Coord x[], Coord y[], int n) {
    p->FillBSpline(c, x, y, n);
}

inline void Graphic::getBounds (
    float& l, float& b, float& r, float& t, Graphic* gs
) {
    float tol;

    getExtent(l, b, r, t, tol, gs);
    r += r - l;
    t += t - b;
    l -= tol;
    b -= tol;
    r += tol;
    t += tol;
}

inline void Graphic::drawGraphic (Graphic* g, Canvas* c, Graphic* gs) {
     g->draw(c, gs);
}
inline void Graphic::eraseGraphic (Graphic* g, Canvas* c, Graphic* gs) {
    g->erase(c, gs);
}

inline void Graphic::drawClippedGraphic (
    Graphic* g, Canvas* c, Coord l, Coord b, Coord r, Coord t, Graphic* gs
) { g->drawClipped(c, l, b, r, t, gs); }

inline void Graphic::eraseClippedGraphic (
    Graphic* g, Canvas* c, Coord l, Coord b, Coord r, Coord t, Graphic* gs
) { g->eraseClipped(c, l, b, r, t, gs); }

inline void Graphic::getExtentGraphic (
    Graphic* g, float& l, float& b, float& r, float& t, float& tol, Graphic* gs
) { g->getExtent(l, b, r, t, tol, gs); }

inline boolean Graphic::containsGraphic (Graphic* g, PointObj& p, Graphic* gs){
    return g->contains(p, gs);
}

inline boolean Graphic::intersectsGraphic (Graphic* g, BoxObj& b, Graphic* gs){
    return g->intersects(b, gs);
}

inline boolean Graphic::extentCachedGraphic (Graphic* g) {
    return g->extentCached();
}

inline void Graphic::uncacheExtentGraphic (Graphic* g) { g->uncacheExtent(); }
inline void Graphic::uncacheParentsGraphic (Graphic* g) { g->uncacheParents();}

inline void Graphic::uncacheChildrenGraphic (Graphic* g) {
    g->uncacheChildren();
}

inline void Graphic::invalidateCachesGraphic (Graphic* g) {
    g->invalidateCaches();
}

inline void Graphic::concatGSGraphic (
    Graphic* g, Graphic* a, Graphic* b, Graphic* d
) {
    g->concatGS(a, b, d);
}

inline void Graphic::concatTransformerGraphic (
    Graphic* g, Transformer* a, Transformer* b, Transformer* dest
) {
    g->concatTransformer(a, b, dest);
}

inline void Graphic::concatGraphic (
    Graphic* g, Graphic* a, Graphic* b, Graphic* d
) {
    g->concat(a, b, d);
}

#endif
