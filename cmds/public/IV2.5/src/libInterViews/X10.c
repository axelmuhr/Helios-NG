/*
 * X10-dependent code
 */

#include "itable.h"
#include <InterViews/bitmap.h>
#include <InterViews/brush.h>
#include <InterViews/canvas.h>
#include <InterViews/color.h>
#include <InterViews/cursor.h>
#include <InterViews/font.h>
#include <InterViews/interactor.h>
#include <InterViews/painter.h>
#include <InterViews/pattern.h>
#include <InterViews/raster.h>
#include <InterViews/reqerr.h>
#include <InterViews/rubrect.h>
#include <InterViews/scene.h>
#include <InterViews/sensor.h>
#include <InterViews/shape.h>
#include <InterViews/transformer.h>
#include <InterViews/world.h>
#include <InterViews/worldview.h>
#include <InterViews/X10/Xinput.h>
#include <InterViews/X10/Xoutput.h>
#include <InterViews/X10/Xutil.h>
#include <InterViews/X10/Xwindow.h>
#include <ctype.h>
#include <osfcn.h>
#include <stdio.h>
#include <string.h>
#include <bstring.h>

static WorldRep* current;
static int saved_argc;
static char** saved_argv;

InteractorTable* assocTable;

class WorldRep {
private:
    friend class World;
    friend class Painter;

    XDisplay* _display;
    XWindow _root;
    int _nplanes;
    int _planemask;
public:
    class XDisplay* display () { return _display; }
    XWindow root () { return _root; }
    int nplanes () { return _nplanes; }
    int planemask () { return _planemask; }
};

/*
 * class Bitmap
 */

static int BitmapDataSize (int w, int h) {
    return BitmapSize(w, h);
}

BitmapRep::BitmapRep(const char* filename) {
    map = nil;
    XReadBitmapFile(filename, &width, &height, &data, &x0, &y0);
    if (x0 == -1) {
        x0 = 0;
    }
    if (y0 == -1) {
        y0 = 0;
    } else {
        y0 = height-1 - y0;
    }
}

BitmapRep::BitmapRep (void* d, int w, int h, int x, int y) {
    width = w;
    height = h;
    x0 = x;
    y0 = y;
    int size = BitmapDataSize(width, height);
    map = nil;
    data = new char[size];
    if (d != nil) {
	bcopy(d, data, size);
    } else {
	bzero(data, size);
    }
}    

BitmapRep::BitmapRep (Font* f, int c) {
    char ch = c;
    width = f->Width(&ch, 1);
    height = f->Height();
    x0 = 0;
    y0 = f->Baseline();
    XFontInfo* i = (XFontInfo*)f->Info();
    map = XCharBitmap(i->id, c);
    int size = BitmapDataSize(width, height);
    map = nil;
    data = new char[size];
    bzero(data, size);
}    

BitmapRep::BitmapRep(BitmapRep* b, BitTx t) {
    switch (t) {
    case NoTx: case FlipH: case FlipV: case Rot180: case Inv:
        width = b->width; height = b->height; break;
    case Rot90: case Rot270:
        width = b->height; height = b->width; break;
    }
    int size = BitmapDataSize(width, height);
    data = new char[size];
    map = nil;
    for (int x = 0; x < width; ++x) {
	for (int y = 0; y < height; ++y) {
            boolean bit;
            switch(t) {
            case NoTx:   bit = b->GetBit(x, y); break;
            case FlipH:  bit = b->GetBit(width-x-1, y); break;
            case FlipV:  bit = b->GetBit(x, height-y-1); break;
            case Rot90:  bit = b->GetBit(height-y-1, x); break;
            case Rot180: bit = b->GetBit(width-x-1, height-y-1); break;
            case Rot270: bit = b->GetBit(y, width-x-1); break;
            case Inv:    bit = !b->GetBit(x, y); break;
            }
            PutBit(x, y, bit);
	}
    }
}

BitmapRep::BitmapRep (BitmapRep* b, Transformer* matrix) {
    Transformer t(matrix);

    Coord x1 = - b->x0;
    Coord y1 = - b->y0;
    Coord x2 = - b->x0;
    Coord y2 = b->height - b->y0;
    Coord x3 = b->width - b->x0;
    Coord y3 = b->height - b->y0;
    Coord x4 = b->width - b->x0;
    Coord y4 = - b->y0;

    t.Transform(x1, y1);
    t.Transform(x2, y2);
    t.Transform(x3, y3);
    t.Transform(x4, y4);

    Coord xmax = max(x1,max(x2,max(x3,x4))) - 1;
    Coord xmin = min(x1,min(x2,min(x3,x4)));
    Coord ymax = max(y1,max(y2,max(y3,y4))) - 1;
    Coord ymin = min(y1,min(y2,min(y3,y4)));

    width = xmax - xmin + 1;
    height = ymax - ymin + 1;
    if (width <= 0) width = 1;
    if (height <= 0) height = 1;
    x0 = -xmin;
    y0 = -ymin;
    int size = BitmapDataSize(width, height);
    data = new char[size];
    map = nil;
    t.Invert();

    for (Coord xx = xmin; xx <= xmax; ++xx) {
        for (Coord yy = ymin; yy <= ymax; ++yy) {
            float tx;
            float ty;
            t.Transform(xx+0.5, yy+0.5, tx, ty);
            int rx = round(tx-0.5);
            int ry = round(ty-0.5);
            if (
               rx >= -b->x0 && rx < -b->x0 + b->width
               && ry >= -b->y0 && ry < -b->y0 + b->height
            ) {
                PutBit(xx + x0, yy + y0, b->GetBit(rx + b->x0, ry + b->y0));
            } else {
                PutBit(xx + x0, yy + y0, false);
            }
        }
    }
}

BitmapRep::~BitmapRep () {
    Touch();
    delete data;
}

void BitmapRep::Touch () {
    if (map != nil) {
	XFreeBitmap((XBitmap)map);
	map = nil;
    }
}

inline static int BitmapIndex (int width, int height, int x, int y) {
    return (width+15)/16 * (height-y-1) + x/16;
}

inline static int BitmapBit (int x, int) {
    return 1 << (x%16);
}

void BitmapRep::PutBit (int x, int y, boolean bit) {
    if (bit) {
        *((short*)data + BitmapIndex(width, height, x, y)) |= BitmapBit(x, y);
    } else {
        *((short*)data + BitmapIndex(width, height, x, y)) &= ~BitmapBit(x, y);
    }
}

boolean BitmapRep::GetBit (int x, int y) {
    return (
        *((short*)data + BitmapIndex(width, height, x, y)) & BitmapBit(x, y)
    ) != 0;
}

void* BitmapRep::GetData () {
    return data;
}        

void* BitmapRep::GetMap () {
    if (map == nil) {
	map = XStoreBitmap(width, height, data);
    }
    return map;
}

/*
 * Class Raster
 */

static int RasterImageSize (int w, int h) {
    return w * h * sizeof(unsigned long);
}

static inline int RasterIndex(int width, int height, int x, int y) {
    return width * (height-y-1) + x;
}

RasterRep::RasterRep (int w, int h) {
    width = w;
    height = h;
    data = new char[RasterImageSize(width, height)];
}    

RasterRep::RasterRep (Canvas* c, int x0, int y0, int w, int h) {
    width = w;
    height = h;
    data = new char[RasterImageSize(width, height)];
    XPixmapGetZ(c->Id(), x0, c->Height()-(y0+h), w, h, data);
}

RasterRep::RasterRep (RasterRep* r) {
    width = r->width;
    height = r->height;
    if (r->data != nil) {
        for (int x = 0; x < width; ++x) {
            for (int y = 0; y < height; ++y) {
                PutPixel(x, y, r->GetPixel(x, y));
            }
        }
    } else {
        data = nil;
    }
}

void* RasterRep::GetData () {
    return data;
}

RasterRep::~RasterRep () {
    delete data;
}

void RasterRep::PutPixel (int x, int y, int pixel) {
    *((short*)data + RasterIndex(width, height, x, y)) = (short)pixel;
}

int RasterRep::GetPixel (int x, int y) {
    return int(*((short*)data + RasterIndex(width, height, x, y)));
}

/*
 * class Brush
 */

static boolean PatternBit (int i, int* pattern, int count) {
    boolean bit = true;
    int index = 0;
    while (i >= pattern[index]) {
        i -= pattern[index];
        bit = !bit;
        index = (index + 1) % count;
    }
    return bit;
}

BrushRep::BrushRep (int* pattern, int c, int) {
    if (c != 0) {
        count = 0;
        int pat = 0;
        const int w = 16;
        for (int i = 0; i < w; ++i) {
            pat = (pat << 1) & PatternBit(i, pattern, c);
        }
    } else {
        count = 0;
        info = nil;
    }
}

BrushRep::~BrushRep () {
    /* nothing to do */
}

/*
 * class Color
 */

ColorRep::ColorRep (ColorIntensity r, ColorIntensity g, ColorIntensity b) {
    XColor* c = new XColor;
    c->red = r;
    c->green = g;
    c->blue = b;
    if (XGetHardwareColor(c)) {
        info = (void*)c;
    } else {
        delete c;
        info = nil;
    }
}

ColorRep::ColorRep (
    int p, ColorIntensity& r, ColorIntensity& g, ColorIntensity& b
) {
    XColor* c = new XColor;
    c->pixel = p;
    XQueryColor(c);
    r = c->red;
    g = c->green;
    b = c->blue;
    info = (void*)c;
}

ColorRep::ColorRep (
    const char* name, ColorIntensity& r, ColorIntensity& g, ColorIntensity& b
) {
    XColor* c = new XColor;
    XColor exact;
    if (XGetColor(name, c, &exact)) {
        r = exact.red;
        g = exact.green;
        b = exact.blue;
        info = (void*)c;
    } else {
        delete c;
        info = nil;
    }
}

ColorRep::~ColorRep () {
    if (info != nil) {
        delete info;
    }
}

int ColorRep::GetPixel () {
    XColor* c = (XColor*)info;
    return c->pixel;
}

void ColorRep::GetIntensities (
    ColorIntensity& red, ColorIntensity& green, ColorIntensity& blue
) {
    XColor* c = (XColor*)info;
    red = c->red;
    green = c->green;
    blue = c->blue;
}

/*
 * class Cursor
 */

Cursor::Cursor (Bitmap* pat, Bitmap* mask, Color* fg, Color* bg) {
    id = XStoreCursor(
        pat->Map(), mask->Map(),
        -pat->Left(), pat->Height()-1 - (-pat->Bottom()),
        fg->PixelValue(), bg->PixelValue(), GXcopy
    );
}

Cursor::Cursor (Font* font, int pat, int mask, Color* fg, Color* bg) {
    Bitmap p(font, pat);
    Bitmap m(font, mask);
    id = XStoreCursor(
        p.Map(), m.Map(),
        -p.Left(), p.Height()-1 - (-p.Bottom()),
        fg->PixelValue(), bg->PixelValue(), GXcopy
    );
}

Cursor::Cursor (int, Color*, Color*) {
    id = nil;
    pat = nil;
}

void* Cursor::Id () {
    if (id == nil && pat != nil) {
	register int i;
	short pattern[cursorHeight], patmask[cursorHeight];
	register int* srcpat, * srcmask;
	register short* dstpat, * dstmask;

	srcpat = pat;
	dstpat = pattern;
	srcmask = mask;
	dstmask = patmask;
	for (i = 0; i < cursorHeight; i++) {
	    /* swap bits because X expects (backwards) VAX order */
	    register int j;
	    register unsigned s1, s2;

	    *dstpat = 0;
	    *dstmask = 0;
	    s1 = 1;
	    s2 = (1 << 15);
	    for (j = 0; j < 16; j++) {
		if ((s1 & *srcpat) != 0) {
		    *dstpat |= s2;
		}
		if ((s1 & *srcmask) != 0) {
		    *dstmask |= s2;
		}
		s1 <<= 1;
		s2 >>= 1;
	    }
	    ++srcpat;
	    ++dstpat;
	    ++srcmask;
	    ++dstmask;
	}
	id = XCreateCursor(
	    cursorWidth, cursorHeight, pattern, patmask,
	    x, cursorHeight - 1 - y,
	    foreground->PixelValue(), background->PixelValue(), GXcopy
	);
    }
    return id;
}

Cursor::~Cursor () {
    if (id != nil) {
	XFreeCursor(id);
    }
}

/*
 * class Font
 */

void Font::GetFontByName (const char* name) {
    rep->id = XGetFont(name);
    Init();
}

void Font::Init () {
    if (rep->id != nil) {
	register XFontInfo* i = new XFontInfo;

	XQueryFont(rep->id, i);
	rep->height = i->height;
	if (i->fixedwidth) {
	    i->widths = nil;
	} else {
	    i->widths = XFontWidths(rep->id);
	}
	rep->info = i;
    } else {
	rep->height = -1;
    }
}

Font::~Font () {
    if (LastRef()) {
	delete rep;
    }
}

FontRep::~FontRep () {
    if (LastRef() && id != nil) {
	register XFontInfo* i;

	XFreeFont(id);
	i = (XFontInfo*)info;
	if (i->widths != nil) {
	    delete i->widths;
	}
	delete i;
    }
}

int Font::Baseline () {
    register XFontInfo* i = (XFontInfo*)rep->info;
    return i->baseline - 1;
}

inline int CharWidth (register XFontInfo* info, char c) {
    register int i;

    i = c;
    if (i >= info->firstchar && i <= info->lastchar) {
#	if vax
	    if (current->nplanes() == 1) {
		/* X10 QVSS bug -- widths are 1 off */
		--i;
	    }
#	endif
	i = info->widths[i - info->firstchar];
    } else {
	i = 0;
    }
    return i;
}

int Font::Width (const char* s) {
    register XFontInfo* i = (XFontInfo*)rep->info;
    register char* p;
    register int w;

    if (i->fixedwidth) {
	return strlen(s) * i->width;
    } else {
	w = 0;
	for (p = (char*) s; *p != '\0'; p++) {
	    w += CharWidth(i, *p);
	}
	return w;
    }
}

int Font::Width (const char* s, int len) {
    register XFontInfo* i = (XFontInfo*)rep->info;
    register char* p;
    register int w, n;

    if (i->fixedwidth) {
	return len * i->width;
    } else {
	w = 0;
	for (p = (char*) s, n = len; *p != '\0' && n > 0; p++, n--) {
	    w += CharWidth(i, *p);
	}
	return w;
    }
}

int Font::Index (const char* s, int len, int offset, boolean between) {
    register XFontInfo* i = (XFontInfo*)rep->info;
    register int n, cw, coff;

    if (offset < 0 || *s == '\0' || len == 0) {
	return 0;
    }
    if (i->fixedwidth) {
	cw = i->width;
	n = offset / cw;
	coff = offset % cw;
    } else {
	register int w = 0;
	register const char* p;
	for (p = s, n = 0; *p != '\0' && n < len; ++p, ++n) {
	    cw = CharWidth(i, *p);
	    w += cw;
	    if (w > offset) {
		break;
	    }
	}
	coff = offset - w + cw;
    }
    if (between && coff > cw/2) {
	++n;
    }
    return min(n, len);
}

boolean Font::FixedWidth () {
    register XFontInfo* i = (XFontInfo*)rep->info;
    return i->fixedwidth;
}

/*
 * class Painter
 */

typedef XVertex XPoint;

static const XPointListSize = 200;
static XPoint xpoints[XPointListSize];

class PainterRep {
    friend class Painter;

    int raster;
    int fg, bg;
    boolean fillbg;
    XBitmap pat;
    XPixmap tile;
    XPixmap clearhi, clearlo, stipple;
    Canvas* curcanvas;

    PainterRep();
    XPoint* AllocPts (int n) {
	return (n <= XPointListSize) ? xpoints : new XPoint[n];
    }
    void FreePts (XPoint* v) {
	if (v != xpoints) {
	    delete v;
	}
    }
    void DoDraw(XWindow, void* pat, int bwidth, XPoint v[], int);
    void Update(Color* fg, Color* bg);
    void MakeNoBgPixmaps();
    void FreeNoBgPixmaps();
    void DrawTiled(void*, XPoint*, int);
};

PainterRep::PainterRep () {
    raster = GXcopy;
    fg = 0;
    bg = 0;
    fillbg = true;
    pat = nil;
    tile = XWhitePixmap;
    clearhi = nil;
    clearlo = nil;
    stipple = nil;
    curcanvas = nil;
}

inline void PainterRep::DoDraw (XWindow w, void* p, int b, XPoint v[], int n) {
    if (p == nil) {
	XDraw(w, v, n, b, b, fg, raster, current->planemask());
    } else if (fillbg) {
	XDrawPatterned(
	    w, v, n, b, b, fg, bg, (int)p, raster, current->planemask()
	);
    } else {
	XDrawDashed(
	    w, v, n, b, b, fg, (int)p, raster, current->planemask()
	);
    }
}

void PainterRep::Update (Color* f, Color* b) {
    if (raster == GXxor) {
	tile = XWhitePixmap;
	fg = 1;
	bg = 0;
	fillbg = true;
    } else {
	fg = f->PixelValue();
	bg = b->PixelValue();
	if (tile != XWhitePixmap) {
	    XFreePixmap(tile);
	}
	tile = XMakePixmap(pat, fg, bg);
	if (!fillbg && stipple == nil) {
	    MakeNoBgPixmaps();
	}
    }
}

void PainterRep::MakeNoBgPixmaps () {
    if (current->nplanes() == 1) {
	/* first turn off fg bits, leave bg as is */
	clearhi = XMakePixmap(pat, 0, 1);
    } else {
	int mask = 1 << current->nplanes() - 1;
	clearhi = XMakePixmap(pat, mask-1, mask);
	clearlo = XMakePixmap(pat, mask, mask-1);
    }
    /* set fg bits to foreground color */
    stipple = XMakePixmap(pat, fg, 0);
}

void PainterRep::FreeNoBgPixmaps () {
    if (stipple != nil) {
	XFreePixmap(stipple);
	XFreePixmap(clearhi);
	if (current->nplanes() > 1) {
	    XFreePixmap(clearlo);
	}
	stipple = nil;
    }
}

void PainterRep::DrawTiled (void* id, XPoint* v, int n) {
    if (fillbg) {
	XDrawTiled(id, v, n, tile, raster, current->planemask());
    } else if (current->nplanes() == 1) {
	/* erase fg bits */
	XDrawTiled(id, v, n, clearhi, GXand, current->planemask());
	/* fill in fg bits */
	XDrawTiled(id, v, n, stipple, GXor, current->planemask());
    } else {
	/*
	 * First turn off fg bits, leave bg as is.  Then set fg
	 * bits to foreground color.
	 * To avoid using bogus colors, we'll erase the bits in
	 * two passes with two legal colors.
	 */
	int mask = 1 << current->nplanes() - 1;
	XDrawTiled(id, v, n, clearhi, GXand, mask);
	XDrawTiled(id, v, n, clearlo, GXand, mask-1);
	XDrawTiled(id, v, n, stipple, GXor, current->planemask());
    }
}

Painter::Painter () {
    rep = new PainterRep;
    Init();
}

Painter::Painter (Painter* copy) {
    rep = new PainterRep;
    Copy(copy);
    FillBg(copy->rep->fillbg);
}

Painter::~Painter () {
    if (LastRef()) {
	rep->FreeNoBgPixmaps();
	delete matrix;
	delete font;
	delete br;
	delete foreground;
	delete background;
	delete pattern;
	delete rep;
    }
}

void Painter::FillBg (boolean b) {
    if (rep->raster != GXcopy) {
	rep->fillbg = b;
	rep->raster = GXcopy;
	rep->Update(foreground, background);
    } else if (b != rep->fillbg) {
	rep->fillbg = b;
	if (!b && rep->stipple == nil) {
	    rep->MakeNoBgPixmaps();
	}
    }
}

boolean Painter::BgFilled () {
    return rep->fillbg;
}

void Painter::SetColors (Color* f, Color* b) {
    boolean changed = false;
    if (f != nil && foreground != f) {
        delete foreground;
	foreground = f;
        foreground->Reference();
	changed = true;
    }
    if (b != nil && background != b) {
        delete background;
	background = b;
        background->Reference();
	changed = true;
    }
    if (changed) {
	rep->FreeNoBgPixmaps();
	rep->Update(foreground, background);
    }
}

void Painter::SetPattern (Pattern* pat) {
    if (pattern != pat) {
        delete pattern;
	pattern = pat;
	if (pattern != nil) {
            pattern->Reference();
	    rep->pat = pattern->info;
	    rep->FreeNoBgPixmaps();
	    rep->Update(foreground, background);
	}
    }
}

void Painter::SetBrush (Brush* b) {
    if (br != b) {
        delete br;
        br = b;
        if (br != nil) {
            br->Reference();
        }
    }
}

void Painter::SetFont (Font* f) {
    if (font != f) {
        delete font;
        font = f;
        if (font != nil) {
            font->Reference();
        }
    }
}

void Painter::Clip (
    Canvas* c, Coord left, Coord bottom, Coord right, Coord top
) {
    c->Clip(left, bottom, right, top);
    rep->curcanvas = c;
}

void Painter::NoClip () {
    if (rep->curcanvas != nil) {
	rep->curcanvas->NoClip();
	rep->curcanvas = nil;
    }
}

void Painter::SetPlaneMask (int m) {
    current->_planemask = m;
}

void Painter::SetOverwrite (boolean) {
    /* always true in X10 */
}

void Painter::Map (Canvas* c, Coord x, Coord y, Coord& mx, Coord& my) {
    if (matrix == nil) {
	mx = x; my = y;
    } else {
	matrix->Transform(x, y, mx, my);
    }
    mx += xoff;
    my += yoff;
    c->Map(mx, my);
}

void Painter::MapList (
    Canvas* c, Coord x[], Coord y[], int n, Coord mx[], Coord my[]
) {
    register Coord* xp, * yp, * mxp, * myp;
    Coord* lim;

    xp = x; yp = y;
    mxp = mx; myp = my;
    lim = &x[n];
    if (matrix == nil) {
	for (; xp < lim; xp++, yp++, mxp++, myp++) {
	    *mxp = *xp + xoff;
	    *myp = *yp + yoff;
	    c->Map(*mxp, *myp);
	}
    } else {
	for (; xp < lim; xp++, yp++, mxp++, myp++) {
	    matrix->Transform(*xp, *yp, *mxp, *myp);
	    *mxp += xoff;
	    *myp += yoff;
	    c->Map(*mxp, *myp);
	}
    }
}

void Painter::MapList (
    Canvas* c, float x[], float y[], int n, Coord mx[], Coord my[]
) {
    register float* xp, * yp;
    register Coord* mxp, * myp;
    float tmpx, tmpy, * lim;

    xp = x; yp = y;
    mxp = mx; myp = my;
    lim = &x[n];
    if (matrix == nil) {
	for (; xp < lim; xp++, yp++, mxp++, myp++) {
	    *mxp = round(*xp + xoff);
	    *myp = round(*yp + yoff);
	    c->Map(*mxp, *myp);
	}
    } else {
	for (; xp < lim; xp++, yp++, mxp++, myp++) {
	    matrix->Transform(*xp, *yp, tmpx, tmpy);
	    *mxp = round(tmpx + xoff);
	    *myp = round(tmpy + yoff);
	    c->Map(*mxp, *myp);
	}
    }
}

void Painter::Begin_xor () {
    if (rep->raster != GXxor) {
	rep->raster = GXxor;
	rep->Update(foreground, background);
    }
}

void Painter::End_xor () {
    if (rep->raster != GXcopy) {
	rep->raster = GXcopy;
	rep->Update(foreground, background);
    }
}

void Painter::Stencil (Canvas* c, Coord x, Coord y, Bitmap* d, Bitmap* m) {
    if (matrix == nil) {
        Coord mx, my;
        Map(c, x, y + d->Height() - 1, mx, my);

        XBitmapBitsPut(
            c->Id(), mx, my, d->Width(), d->Height(),
            (short*)d->rep->data, rep->fg, rep->bg,
            (m != nil) ? (XBitmap)m->Map() : 0,
            rep->raster, current->planemask()
        );
    } else {
        Transformer t(matrix);

        Coord x1 = x + d->Left(),      y1 = y + d->Bottom();
        Coord x2 = x + d->Left(),      y2 = y + d->Top();
        Coord x3 = x + d->Right(),     y3 = y + d->Top();
        Coord x4 = x + d->Right(),     y4 = y + d->Bottom();

        t.Transform(x1, y1);
        t.Transform(x2, y2);
        t.Transform(x3, y3);
        t.Transform(x4, y4);

        Coord xmax = max(x1,max(x2,max(x3,x4)));
        Coord xmin = min(x1,min(x2,min(x3,x4)));
        Coord ymax = max(y1,max(y2,max(y3,y4)));
        Coord ymin = min(y1,min(y2,min(y3,y4)));

        t.Invert();

        for (Coord xx = xmin; xx <= xmax; ++xx) {
            for (Coord yy = ymin; yy <= ymax; ++yy) {
                float tx;
                float ty;
                t.Transform(xx+0.5, yy+0.5, tx, ty);
                int rx = round(tx-0.5) - x;
                int ry = round(ty-0.5) - y;
                if (
                    (m == nil && d->Contains(rx, ry))
                    || (m != nil && m->Peek(rx, ry))
                ) {
                    XLine(
                        c->Id(),
                        xx + xoff, c->height - 1 - (yy + yoff),
                        xx + xoff, c->height - 1 - (yy + yoff),
                        1, 1, d->Peek(rx, ry) ? rep->fg : rep->bg,
                        rep->raster, current->planemask()
                    );
                }
            }
        }
    }
}

void Painter::RasterRect (Canvas* c, Coord x, Coord y, Raster* r) {
    if (matrix == nil || !(matrix->Scaled() || matrix->Rotated())) {
        Coord mx, my;
        Map(c, x, y + r->Height() - 1, mx, my);

        XPixmapBitsPutZ(
            c->Id(), mx, my, r->Width(), r->Height(),
            (short*)r->rep->data, 0,
            rep->raster, current->planemask()
        );
    } else {
        Transformer t(matrix);

        Coord x1 = x, y1 = y;
        Coord x2 = x, y2 = y + r->Height()-1;
        Coord x3 = x + r->Width()-1, y3 = y + r->Height()-1;
        Coord x4 = x + r->Width()-1, y4 = y;

        t.Transform(x1, y1);
        t.Transform(x2, y2);
        t.Transform(x3, y3);
        t.Transform(x4, y4);

        Coord xmax = max(x1,max(x2,max(x3,x4)));
        Coord xmin = min(x1,min(x2,min(x3,x4)));
        Coord ymax = max(y1,max(y2,max(y3,y4)));
        Coord ymin = min(y1,min(y2,min(y3,y4)));

        t.Invert();

        for (Coord xx = xmin; xx <= xmax; ++xx) {
            for (Coord yy = ymin; yy <= ymax; ++yy) {
                float tx;
                float ty;
                t.Transform(xx+0.5, yy+0.5, tx, ty);
                int rx = int(tx-0.5) - x;
                int ry = int(ty-0.5) - y;
                Color* color = r->Peek(rx, ry);
                if (color != nil) {
                    XLine(
                        c->Id(),
                        xx + xoff, c->height - 1 - (yy + yoff),
                        xx + xoff, c->height - 1 - (yy + yoff),
                        1, 1, color->PixelValue(),
                        rep->raster, current->planemask()
                    );
                }
            }
        }
    }
}

void Painter::Text (Canvas* c, const char* s, int len, Coord x, Coord y) {
    Coord x0, y0;
    Coord ybase = y + font->Baseline();
    Coord ytop = y + font->Height() - 1;
    if (style & Reversed) {
        SetColors(GetBgColor(), GetFgColor());
    }
    if (matrix == nil || !(matrix->Scaled() || matrix->Rotated())) {
        Map(c, x, ytop, x0, y0);
        if (rep->fillbg) {
            XTextPad(
                c->Id(), x0, y0, s, len, font->rep->id, 0, 0,
                rep->fg, rep->bg, rep->raster, current->planemask()
            );
        } else {
            XTextMaskPad(
                c->Id(), x0, y0, s, len, font->rep->id, 0, 0,
                rep->fg, GXcopy, current->planemask()
            );
        }
        if (style & Boldface) {
            XTextMaskPad(
                c->Id(), x0-1, y0, s, len, font->rep->id, 0, 0,
                rep->fg, GXcopy, current->planemask()
            );
        }
    } else {
        Coord curx = x;
        for (int i = 0; i < len; ++i) {
            Coord nextx = curx + font->Width(s+i, 1);
            if (rep->fillbg) {
                ClearRect(c, curx, y, nextx, ytop);
            }
            Map(c, curx, ytop, x0, y0);
            XTextMaskPad(
                c->Id(), x0, y0, s+i, 1, font->rep->id, 0, 0,
                rep->fg, GXcopy, current->planemask()
            );
            if (style & Boldface) {
                XTextMaskPad(
                    c->Id(), x0-1, y0, s+i, 1, font->rep->id, 0, 0,
                    rep->fg, GXcopy, current->planemask()
                );
            }
            curx = nextx;
        }
    }
    if (style & Outlined) {
        /* unimplemented */
    }
    if (style & Underlined) {
        Line(c, x, ybase, x + font->Width(s, len) - 1, ybase);
    }
    if (style & Reversed) {
        SetColors(GetBgColor(), GetFgColor());
    }
}

void Painter::Point (Canvas* c, Coord x, Coord y) {
    Coord mx, my;

    Map(c, x, y, mx, my);
    XLine(
	c->Id(), mx, my, mx, my,
	br->width, br->width, rep->fg, rep->raster, current->planemask()
    );
}

void Painter::MultiPoint (Canvas* c, Coord x[], Coord y[], int n) {
    register XPoint* v;
    register int i;
    int nn;

    nn = 2*n;
    v = rep->AllocPts(nn);
    for (i = 0; i < nn; i += 2) {
	Map(c, x[i], y[i], v[i].x, v[i].y);
	v[i].flags = VertexDontDraw;
	v[i+1] = v[i];
	v[i+1].flags = 0;
    }
    rep->DoDraw(c->Id(), br->rep->info, br->width, v, nn);
    rep->FreePts(v);
}

/*
 * X behaves anomalously, at best, for single lines (in other words,
 * VertexDrawLastPoint doesn't seem to work right), so we simply add
 * the end point on twice.
 */

void Painter::Line (Canvas* c, Coord x1, Coord y1, Coord x2, Coord y2) {
    XPoint v[3];

    Map(c, x1, y1, v[0].x, v[0].y);
    v[0].flags = 0;
    Map(c, x2, y2, v[1].x, v[1].y);
    v[1].flags = 0;
    v[2] = v[1];
    v[2].flags = VertexDrawLastPoint;
    rep->DoDraw(c->Id(), br->rep->info, br->width, v, 3);
}

void Painter::Rect (Canvas* c, Coord x1, Coord y1, Coord x2, Coord y2) {
    XPoint v[5];

    Map(c, x1, y1, v[0].x, v[0].y);  v[0].flags = 0;
    Map(c, x2, y1, v[1].x, v[1].y);  v[1].flags = 0;
    Map(c, x2, y2, v[2].x, v[2].y);  v[2].flags = 0;
    Map(c, x1, y2, v[3].x, v[3].y);  v[3].flags = 0;
    Map(c, x1, y1, v[4].x, v[4].y);  v[4].flags = 0;
    rep->DoDraw(c->Id(), br->rep->info, br->width, v, 5);
}

void Painter::FillRect (Canvas* c, Coord x1, Coord y1, Coord x2, Coord y2) {
    Coord left, bottom, right, top, tmp, x[4], y[4];
    int w, h;

    if (matrix != nil && matrix->Rotated() && !matrix->Rotated90()) {
	x[0] = x[3] = x1;
	x[1] = x[2] = x2;
	y[0] = y[1] = y1;
	y[2] = y[3] = y2;
	FillPolygon(c, x, y, 4);
    } else {
	Map(c, x1, y1, left, bottom); 
	Map(c, x2, y2, right, top);
	if (left > right) {
	    tmp = left; left = right; right = tmp;
	}
	if (top > bottom) {
	    tmp = bottom; bottom = top; top = tmp;
	}
	w = right - left + 1;
	h = bottom - top + 1;
	if (rep->fillbg) {
	    XTileFill(
		c->Id(), left, top, w, h,
		rep->tile, 0, rep->raster, current->planemask()
	    );
	} else if (current->nplanes() == 1) {
	    /* erase fg bits */
	    XTileFill(
		c->Id(), left, top, w, h, rep->clearhi, 0, 
		GXand, current->planemask()
	    );
	    /* fill in fg bits */
	    XTileFill(
		c->Id(), left, top, w, h, rep->stipple, 0, 
		GXor, current->planemask()
	    );
	} else {
	    /*
	     * First turn off fg bits, leave bg as is.  Then set fg
	     * bits to foreground color.
	     * To avoid using bogus colors, we'll erase the bits in
	     * two passes with two legal colors.
	     */
	    int mask = 1 << current->nplanes() - 1;
	    XTileFill(c->Id(), left, top, w, h, rep->clearhi, 0, GXand, mask);
	    XTileFill(
		c->Id(), left, top, w, h, rep->clearlo, 0, GXand, mask-1
	    );
	    XTileFill(c->Id(), left, top, w, h, rep->stipple, 0, GXor, mask-1);
	}
    }
}

void Painter::ClearRect (Canvas* c, Coord x1, Coord y1, Coord x2, Coord y2) {
    Coord left, bottom, right, top, tmp;

    Map(c, x1, y1, left, bottom);
    Map(c, x2, y2, right, top);
    if (left > right) {
	tmp = left; left = right; right = tmp;
    }
    if (top > bottom) {
	tmp = bottom; bottom = top; top = tmp;
    }
    XPixFill(
	c->Id(), left, top, right - left + 1, bottom - top + 1,
	rep->bg, 0, rep->raster, current->planemask()
    );
}

void Painter::Circle (Canvas* c, Coord x, Coord y, int r) {
    XPoint v[5];

    if (matrix != nil && (matrix->Stretched() || matrix->Rotated())) {
	Ellipse(c, x, y, r, r);
    } else {
	Map(c, x-r, y, v[0].x, v[0].y);
	v[0].flags = VertexStartClosed+VertexCurved;
	Map(c, x, y+r, v[1].x, v[1].y);
	v[1].flags = VertexCurved;
	Map(c, x+r, y, v[2].x, v[2].y);
	v[2].flags = VertexCurved;
	Map(c, x, y-r, v[3].x, v[3].y);
	v[3].flags = VertexCurved;
	Map(c, x-r, y, v[4].x, v[4].y);
	v[4].flags = VertexEndClosed+VertexCurved;
	rep->DoDraw(c->Id(), br->rep->info, br->width, v, 5);
    }
}

void Painter::FillCircle (Canvas* c, Coord x, Coord y, int r) {
    XPoint v[5];

    if (matrix != nil && (matrix->Stretched() || matrix->Rotated())) {
	FillEllipse(c, x, y, r, r);
    } else {
	Map(c, x-r, y, v[0].x, v[0].y);
	v[0].flags = VertexStartClosed+VertexCurved;
	Map(c, x, y+r, v[1].x, v[1].y);
	v[1].flags = VertexCurved;
	Map(c, x+r, y, v[2].x, v[2].y);
	v[2].flags = VertexCurved;
	Map(c, x, y-r, v[3].x, v[3].y);
	v[3].flags = VertexCurved;
	Map(c, x-r, y, v[4].x, v[4].y);
	v[4].flags = VertexEndClosed+VertexCurved;
	rep->DrawTiled(c->Id(), v, 5);
    }
}

void Painter::MultiLine (Canvas* c, Coord x[], Coord y[], int n) {
    register XPoint* v;
    register int i;

    v = rep->AllocPts(n);
    for (i = 0; i < n; i++) {
	Map(c, x[i], y[i], v[i].x, v[i].y);
	v[i].flags = 0;
    }
    rep->DoDraw(c->Id(), br->rep->info, br->width, v, n);
    rep->FreePts(v);
}

void Painter::MultiLineNoMap (Canvas* c, Coord x[], Coord y[], int n) {
    register XPoint* v;
    register int i;

    v = rep->AllocPts(n);
    for (i = 0; i < n; i++) {
	v[i].x = x[i];
	v[i].y = y[i];
	v[i].flags = 0;
    }
    rep->DoDraw(c->Id(), br->rep->info, br->width, v, n);
    rep->FreePts(v);
}

void Painter::Polygon (Canvas* c, Coord x[], Coord y[], int n) {
    register XPoint* v;
    register int i;

    v = rep->AllocPts(n+1);
    for (i = 0; i < n; i++) {
	Map(c, x[i], y[i], v[i].x, v[i].y);
	v[i].flags = 0;
    }
    if (x[i-1] != x[0] || y[i-1] != y[0]) {
	v[i] = v[0];
	++i;
    }
    v[0].flags |= VertexStartClosed;
    v[i-1].flags |= VertexEndClosed;
    rep->DoDraw(c->Id(), br->rep->info, br->width, v, i);
    rep->FreePts(v);
}

void Painter::FillPolygonNoMap (Canvas* c, Coord x[], Coord y[], int n) {
    register XPoint* v;
    register int i;

    v = rep->AllocPts(n);
    for (i = 0; i < n; i++) {
	v[i].x = x[i];
	v[i].y = y[i];
	v[i].flags = 0;
    }
    if (x[i-1] != x[0] || y[i-1] != y[0]) {
	v[i] = v[0];
	++i;
    }
    v[0].flags |= VertexStartClosed;
    v[i-1].flags |= VertexEndClosed;
    rep->DrawTiled(c->Id(), v, i);
    rep->FreePts(v);
}

void Painter::FillPolygon (Canvas* c, Coord x[], Coord y[], int n) {
    register XPoint* v;
    register int i;

    v = rep->AllocPts(n+1);
    for (i = 0; i < n; i++) {
	Map(c, x[i], y[i], v[i].x, v[i].y);
	v[i].flags = 0;
    }
    if (x[i-1] != x[0] || y[i-1] != y[0]) {
	v[i] = v[0];
	++i;
    }
    v[0].flags |= VertexStartClosed;
    v[i-1].flags |= VertexEndClosed;
    rep->DrawTiled(c->Id(), v, i);
    rep->FreePts(v);
}

void Painter::Copy (
    Canvas* src, Coord x1, Coord y1, Coord x2, Coord y2,
    Canvas* dst, Coord x0, Coord y0
) {
    register int w, h;
    Coord sx1, sy1, dx1, dy1;

    w = x2 - x1 + 1;
    h = y2 - y1 + 1;
    Map(src, x1, y2, sx1, sy1);
    Map(dst, x0, y0+h-1, dx1, dy1);
    if (src->status == CanvasOffscreen) {
	if (dst->status == CanvasOffscreen) {
	    /* offscreen-to-offscreen unimplemented */
	} else {
	    XPixmapPut(
		dst->id, sx1, sy1, dx1, dy1, w, h,
		src->id, rep->raster, current->planemask()
	    );
	}
    } else {
	if (dst->status == CanvasOffscreen) {
	    if (dst->id != nil) {
		XFreePixmap(dst->id);
	    }
	    dst->id = XPixmapSave(src->id, sx1, sy1, w, h);
	    dst->width = w;
	    dst->height = h;
	} else {
	    if (src->id == dst->id) {
		XCopyArea(
		    src->id, sx1, sy1, dx1, dy1, w, h,
		    rep->raster, current->planemask()
		);
		src->WaitForCopy();
	    } else {
		/* cross-window unimplemented */
	    }
	}
    }
}

void Painter::Read (
    Canvas* c, void* dst, Coord x1, Coord y1, Coord x2, Coord y2
) {
    XPixmapGetXY(c->id, x1, c->height-y2-1, x2-x1+1, y2-y1+1, dst);
}

void Painter::Write (
    Canvas* c, const void* src, Coord x1, Coord y1, Coord x2, Coord y2
) {
    XPixmapBitsPutXY(
	c->id, x1, c->height - y2 - 1, x2 - x1 + 1, y2 - y1 + 1,
	(short*)src, nil, rep->raster, current->planemask()
    );
}

/*
 * class Pattern
 */

Pattern::Pattern (int p[patternHeight]) {
    short bp[patternHeight];
    register int* src;
    register short* dst;
    register int i;

    src = p;
    dst = bp;
    for (i = 0; i < patternHeight; i++) {
	/*
	 * Swap bits in patterns for X, which expects them in VAX order.
	 */
	register int j;
	register unsigned s1, s2;

	*dst = 0;
	s1 = 1;
	s2 = 1 << (patternWidth-1);
	for (j = 0; j < patternWidth; j++) {
	    if ((s1 & *src) != 0) {
		*dst |= s2;
	    }
	    s1 <<= 1;
	    s2 >>= 1;
	}
	++src;
	++dst;
    }
    info = XStoreBitmap(patternWidth, patternHeight, bp);
}

Pattern::Pattern (int dither) {
    register int i, seed;
    int r[4];
    short p[patternHeight];

    register unsigned s1, s2;

    s1 = 1;
    s2 = 1 << (patternHeight-1);
    seed = 0;
    for (i = 0; i < patternWidth; i++) {
	if ((s1 & dither) != 0) {
	    seed |= s2;
	}
	s1 <<= 1;
	s2 >>= 1;
    }
    for (i = 0; i < 4; i++) {
	r[i] = (seed & 0xf000) >> 12;
	r[i] |= r[i] << 4;
	r[i] |= r[i] << 8;
	seed <<= 4;
    }
    for (i = 0; i < patternHeight; i++) {
	p[i] = r[i%4];
    }
    info = XStoreBitmap(patternWidth, patternHeight, p);
}

Pattern::Pattern (Bitmap* b) {
    info = b->Map();
    b->Reference();
}

Pattern::~Pattern () {
    if (LastRef()) {
	XFreeBitmap(info);
    }
}

/*
 * class Canvas
 */

/*
 * In X10, we implement clipping with a transparent window.
 * The window and (x,y) offset are stored in the associated CanvasRep.
 */

class CanvasRep {
    friend class Canvas;

    boolean active;
    Coord left, bottom;
    Canvas* unclipped;

    CanvasRep();
    ~CanvasRep();
};

CanvasRep::CanvasRep () {
    active = false;
    left = 0;
    bottom = 0;
    unclipped = nil;
}

CanvasRep::~CanvasRep () {
    delete unclipped;
}

Canvas::Canvas (void* c) {
    id = c;
    width = 0;
    height = 0;
    status = CanvasUnmapped;
    rep = nil;
}

Canvas::Canvas (int w, int h) {
    id = nil;
    width = w;
    height = h;
    status = CanvasOffscreen;
    rep = nil;
}

Canvas::~Canvas () {
    if (id != nil) {
	if (status == CanvasOffscreen) {
	    XFreePixmap(id);
	} else {
	    XDestroyWindow(id);
	    assocTable->Remove(id);
	}
	id = nil;
    }
    delete rep;
}

void Canvas::WaitForCopy () {
    XEvent xe;
    register XExposeEvent* r = (XExposeEvent*)&xe;
    Interactor* i;

    for (;;) {
	XWindowEvent(id, ExposeCopy|ExposeRegion, &xe);
	if (xe.type == ExposeCopy) {
	    break;
	}
	/* must be ExposeRegion */
	if (assocTable->Find(i, r->window)) {
	    i->Redraw(
		r->x, height - r->y - r->height,
		r->x + r->width - 1, height - 1 - r->y
	    );
	}
    }
}

void Canvas::SetBackground (Color* c) {
    XPixmap p;
    
    void* w = (rep != nil && rep->active) ? rep->unclipped->id : id;
    if (id != nil && status != CanvasOffscreen) {
	if (c->PixelValue() == white->PixelValue()) {
	    XChangeBackground(w, XWhitePixmap);
	} else if (c->PixelValue() == black->PixelValue()) {
	    XChangeBackground(w, XBlackPixmap);
	} else {
	    p = XMakeTile(c->PixelValue());
	    XChangeBackground(w, p);
	    XFreePixmap(p);
	}
    }
}

void Canvas::Clip (Coord x1, Coord y1, Coord x2, Coord y2) {
    Coord left, bottom, right, top;
    int w, h;

    if (x1 <= x2) {
	left = x1; right = x2;
    } else {
	left = x2; right = x1;
    }
    if (y1 <= y2) {
	bottom = y1; top = y2;
    } else {
	bottom = y2; top = y1;
    }
    w = right - left + 1;
    h = top - bottom + 1;
    if (rep != nil) {
	top = rep->unclipped->height - 1 - top;
	XConfigureWindow(id, x1, top, w, h);
    } else {
	top = height - 1 - top;
	rep = new CanvasRep;
	rep->unclipped = new Canvas(XCreateTransparency(id, x1, top, w, h));
	ClipOn();
	XSelectInput(id, EnterWindow|LeaveWindow);
	XMapWindow(id);
    }
    rep->left = x1;
    rep->bottom = y1;
    width = w;
    height = h;
}

void Canvas::NoClip () {
    ClipOff();
    delete rep;
    rep = nil;
}

/*
 * Temporarily turn clipping on/off.
 */

void Canvas::ClipOn () {
    if (rep != nil && !rep->active) {
	rep->active = true;
	register Canvas* c = rep->unclipped;
	void* tmp = id; id = c->id; c->id = tmp;
	int itmp = width; width = c->width; c->width = itmp;
	itmp = height; height = c->height; c->height = itmp;
    }
}

void Canvas::ClipOff () {
    if (rep != nil && rep->active) {
	rep->active = false;
	register Canvas* c = rep->unclipped;
	void* tmp = id; id = c->id; c->id = tmp;
	int itmp = width; width = c->width; c->width = itmp;
	itmp = height; height = c->height; c->height = itmp;
    }
}

boolean Canvas::IsClipped () {
    return rep != nil && rep->active;
}

void Canvas::Map (register Coord& x, register Coord& y) {
    if (rep != nil && rep->active) {
	x -= rep->left;
	y -= rep->bottom;
    }
    y = height - 1 - y;
}

/*
 * class Interactor
 */

int Interactor::Fileno () {
    return Xdpyno();
}

void Interactor::Listen (Sensor* s) {
    unsigned mask;

    cursensor = s;
    if (canvas != nil) {
	if (s == nil && Parent() != nil) {
	    XSelectInput(canvas->id, ExposeRegion|ExposeCopy);
	} else {
	    if (Parent() == nil) {
		if (s->mask & (ExposeRegion|ExposeCopy)) {
		    mask = s->mask & ~(ExposeRegion|ExposeCopy);
		} else {
		    mask = s->mask;
		}
	    } else {
		mask = s->mask;
	    }
	    XSelectInput(canvas->id, mask);
	}
    }
}

boolean Interactor::GetEvent (Event& e, boolean) {
    XEvent xe;
    register XExposeEvent* r = (XExposeEvent*)&xe;
    Interactor* i;

    XNextEvent(&xe);
    if (assocTable->Find(i, xe.window)) {
	if (i->cursensor != nil && i->cursensor->Interesting(&xe, e)) {
	    e.target = i;
	    e.y = i->ymax - e.y;
	    return true;
	} else if (xe.type == ExposeRegion) {
	    i->SendRedraw(r->x, r->y, r->width, r->height, 0);
	} else if (xe.type == ExposeWindow && i->parent->parent == nil) {
	    i->SendResize(0, 0, r->width, r->height);
	}
    }
    return false;
}

/*
 * Check to see if any input events of interest are pending.
 * This routine will return true even if the event is for another interactor.
 */

boolean Interactor::Check () {
    XEvent xe;
    Event e;
    register XExposeEvent* r = (XExposeEvent*) &xe;
    Interactor* i;

    while (XPending()) {
	XNextEvent(&xe);
	if (assocTable->Find(i, xe.window)) {
	    if (i->cursensor != nil && i->cursensor->Interesting(&xe, e)) {
		XPutBackEvent(&xe);
		return true;
	    } else if (xe.type == ExposeRegion) {
		i->SendRedraw(r->x, r->y, r->width, r->height, 0);
	    } else if (xe.type == ExposeWindow && i->parent->parent == nil) {
		i->SendResize(0, 0, r->width, r->height);
	    }
	}
    }
    return false;
}

int Interactor::CheckQueue () {
    return XQLength();
}

/*
 * Translate an ExposeRegion into the appropriate Redraw call.
 */

void Interactor::SendRedraw (Coord x, Coord iy, int w, int h, int) {
    register Coord y;

    y = ymax-iy;
    if (canvas->IsClipped()) {
	canvas->ClipOff();
	Redraw(x, y-h+1, x+w-1, y);
	canvas->ClipOn();
    } else {
	Redraw(x, y-h+1, x+w-1, y);
    }
}

/*
 * Translate an ExposeWindow event to the appropriate call.
 * ExposeWindow can imply either a full redraw
 * (as in the window moved) or a resize, so we check for a size change
 * explicitly.
 */

void Interactor::SendResize (Coord, Coord, int w, int h) {
    register Canvas* c;
    XWindowInfo info;

    c = canvas;
    if (canvas->IsClipped()) {
	canvas->ClipOff();
    }
    XQueryWindow(canvas->id, &info);
    left = info.x;
    bottom = parent->ymax - info.y - h + 1;
    if (w != c->width || h != c->height) {
	c->width = w;
	c->height = h;
	/*
	 * Since Canvas::Clip can't rely on Interactor::ymax to calculate
	 * top, it must instead use height - 1.  So we have to update
	 * height here (potentially twice if there's no clipping, but
	 * it's really not worth a conditional).
	 */
	canvas->height = h;
	xmax = w - 1;
	ymax = h - 1;
	XUnmapSubwindows(c->id);
	Resize();
    }
    Draw();
    if (canvas->IsClipped()) {
	canvas->ClipOn();
    }
}

void Interactor::Poll (Event& e) {
    int x, y;
    short state;
    register short s;
    XWindow dummy;
    register Interactor* i;
    int offx, offy;

    offx = 0;
    offy = 0;
    for (i = this; i->parent != nil; i = i->parent) {
	offx += i->left;
	offy += i->bottom;
    }
    XQueryMouseButtons(i->canvas->id, &x, &y, &dummy, &state);
    e.x = x - offx;
    e.y = (i->ymax - y) - offy;
    e.w = (World*)i;
    e.wx = x;
    e.wy = y;
    s = state;
    e.control = (s&ControlMask) != 0;
    e.meta = (s&MetaMask) != 0;
    e.shift = (s&ShiftMask) != 0;
    e.shiftlock = (s&ShiftLockMask) != 0;
    e.leftmouse = (s&LeftMask) != 0;
    e.middlemouse = (s&MiddleMask) != 0;
    e.rightmouse = (s&RightMask) != 0;
}

void Interactor::Flush () {
    XFlush();
}

void Interactor::Sync () {
    XSync(0);
}

void Interactor::GetRelative (Coord& x, Coord& y, Interactor* rel) {
    register Interactor* t, * r;
    Coord tx, ty, rx, ry;

    if (parent == nil) {
	if (rel == nil || rel->parent == nil) {
	    /* world relative to world -- nop */
	    return;
	}
	/* world relative to interactor is relative to interactor's l, b */
	rx = 0; ry = 0;
	rel->GetRelative(rx, ry);
	x = x - rx;
	y = y - ry;
	return;
    }
    tx = x; ty = y;
    for (t = this; t->parent->parent != nil; t = t->parent) {
	tx += t->left;
	ty += t->bottom;
    }
    if (rel == nil || rel->parent == nil) {
	r = nil;
    } else {
	rx = 0; ry = 0;
	for (r = rel; r->parent->parent != nil; r = r->parent) {
	    rx += r->left;
	    ry += r->bottom;
	}
    }
    if (r == t) {
	/* this and rel are within same top-level interactor */
	x = tx - rx;
	y = ty - ry;
    } else {
	XWindowInfo info;

	XQueryWindow(t->canvas->id, &info);
	x = tx + info.x;
	y = ty + t->parent->ymax - (info.y + info.height - 1);
	if (r != nil) {
	    XQueryWindow(r->canvas->id, &info);
	    x -= rx + info.x;
	    y -= ry + t->parent->ymax - (info.y + info.height - 1);
	}
    }
}

void Interactor::DoSetCursor (Cursor* c) {
    XDefineCursor(canvas->id, c->Id());
    Flush();
}

void Interactor::DoSetName (const char* s) {
    XStoreName(canvas->id, s);
}

void Interactor::DoSetGroupLeader (Interactor*) {
    /* if someone knows of X10 functionality for this, tell me */
}

void Interactor::DoSetTransientFor (Interactor*) {
    /* if someone knows of X10 functionality for this, tell me */
}

void Interactor::DoSetIconName (const char*) {
    /* if someone knows of X10 functionality for this, tell me */
}

void Interactor::DoSetIconBitmap (Bitmap*) {
    /* if someone knows of X10 functionality for this, tell me */
}

void Interactor::DoSetIconMask (Bitmap*) {
    /* if someone knows of X10 functionality for this, tell me */
}

static void PlaceIconInteractor (
    Interactor* i, Interactor* icon, Canvas* iconcanvas
) {
    if (iconcanvas == nil) {
	const char* g = i->GetIconGeometry();
	if (g != nil) {
	    icon->SetGeometry(g);
	}
	i->GetWorld()->InsertIcon(icon);
    }
}

void Interactor::DoSetIconInteractor (Interactor* icon) {
    if (icon != nil) {
	PlaceIconInteractor(this, icon, icon->canvas);
	XSetIconWindow(canvas->id, icon->canvas->id);
    } else {
	XClearIconWindow(canvas->id);
    }
}

void Interactor::DoSetIconGeometry (const char*) {
    /* if someone knows of X10 functionality for this, tell me */
}

static XWindow FindIcon (XWindow w) {
    XWindowInfo info;
    XQueryWindow(w, &info);
    return info.assoc_wind;
}

void Interactor::Iconify () {
    XWindow w = nil;
    Interactor* i = GetIconInteractor();
    if (i != nil && i->canvas != nil) {
	w = i->canvas->id;
	i->canvas->status = CanvasMapped;
    }
    if (w == nil && canvas != nil) {
	w = FindIcon(canvas->id);
    }
    if (w != nil) {
	XMapWindow(w);
	if (canvas != nil && canvas->id != nil) {
	    XUnmapWindow(canvas->id);
	    canvas->status = CanvasUnmapped;
	}
    }
}

void Interactor::DeIconify () {
    if (canvas != nil && canvas->id != nil) {
	XMapWindow(canvas->id);
	canvas->status = CanvasMapped;
	XWindow w = nil;
	Interactor* i = GetIconInteractor();
	if (i != nil && i->canvas != nil) {
	    w = i->canvas->id;
	    i->canvas->status = CanvasUnmapped;
	}
	if (w == nil && canvas != nil) {
	    w = FindIcon(canvas->id);
	}
	if (w != nil) {
	    XUnmapWindow(w);
	}
    }
}

/*
 * class ReqErr
 */

static ReqErr* errhandler;

ReqErr::ReqErr () {
    /* no constructor code currently necessary */
}

ReqErr::~ReqErr () {
    if (errhandler == this) {
	errhandler = nil;
    }
}

void ReqErr::Error () {
    /* default is to do nothing */
}

static void DoXError (XDisplay*, register XErrorEvent* e) {
    register ReqErr* r = errhandler;
    if (r != nil) {
	r->msgid = e->serial;
	r->code = e->error_code;
	r->request = e->request_code;
	r->detail = e->func;
	r->id = e->window;
	strncpy(r->message, XErrDescrip(r->code), sizeof(r->message));
	r->Error();
    }
}

ReqErr* ReqErr::Install () {
    if (errhandler == nil) {
	XErrorHandler(DoXError);
    }
    ReqErr* r = errhandler;
    errhandler = this;
    return r;
}

/*
 * class Scene
 */

typedef struct ExposeList {
    XEvent* event;
    ExposeList* next;
};

void Scene::UserPlace (Interactor* i, int width, int height) {
    Event e;
    Coord x1, y1, x2, y2, tmp;
    RubberRect* r;
    const int mask = ButtonPressed|ButtonReleased|MouseMoved;
    register XEvent* xe;
    register ExposeList* list = nil;
    register ExposeList* el;

    if (i->GetInteractorType() == IconInteractor) {
	return;
    }
    if (i->GetStartIconic()) {
	/*
	 * we still need to create a window for the interactor even if we
	 * map the icon window instead of the toplevel window, so we ask
	 * the user to place the window before we iconify it.
	 */
	fprintf(
	    stderr, "place a window starting up iconic before we iconify it\n"
	);
    }

    do {
	Poll(e);
    } while (e.leftmouse || e.middlemouse || e.rightmouse);
    while (!XGrabMouse(canvas->id, upperleft->Id(), mask)) {
	sleep(1);
    }
    do {
	Poll(e);
    } while (!e.leftmouse && !e.middlemouse && !e.rightmouse);
    e.target = this;
    e.GetAbsolute(x1, y1);
    if (e.leftmouse && width != 0) {
	x2 = x1 + width - 1;
	y2 = y1 + height - 1;
	r = new SlidingRect(output, canvas, x1, y1, x2, y2, x1, y2);
	r->Track(x1, y1);
    } else {
	do {
	    Poll(e);
	    e.target = this;
	    e.GetAbsolute(x2, y2);
	    x2 = e.x; y2 = e.y;
	} while (x2 == x1 && y2 == y1 &&
	     (e.leftmouse || e.middlemouse || e.rightmouse)
	);
	r = new RubberRect(output, canvas, x1, y1, x2, y1);
	r->Track(x2, y1);
	XGrabMouse(canvas->id, lowerright->Id(), mask);
    }
    XGrabServer();
    while (e.leftmouse || e.middlemouse || e.rightmouse) {
	Poll(e);
	e.target = this;
	e.GetAbsolute(x1, y1);
	x1 = e.x; y1 = e.y;
	r->Track(x1, y1);
    }

    xe = new XEvent;
    while (XPending()) {
	XNextEvent(xe);
	if (xe->type == ExposeRegion || xe->type == ExposeWindow ||
	    xe->type == ExposeCopy
	) {
	    el = new ExposeList;
	    el->event = xe;
	    el->next = list;
	    list = el;
	    xe = new XEvent;
	}
    }
    while (list != nil) {
	XPutBackEvent(list->event);
	delete(list->event);
	list = list->next;
    }

    XUngrabServer();
    XUngrabMouse();
    r->GetCurrent(x1, y1, x2, y2);
    delete r;

    if (x1 > x2) {
	tmp = x1; x1 = x2; x2 = tmp;
    } else if (x1 == x2) {
	x2 = x1 + 10;
    }
    if (y1 > y2) {
	tmp = y1; y1 = y2; y2 = tmp;
    } else if (y1 == y2) {
	y2 = y1 + 10;
    }
    Place(i, x1, y1, x2, y2);
}

void Scene::Place (
    Interactor* i, Coord x1, Coord y1, Coord x2, Coord y2, boolean map
) {
    register XWindow id;
    register int w, h;
    Coord top;
    static boolean traversing = false;

    if (i->parent != nil && i->parent != this) {
	i->parent->Remove(i);
    }
    if (parent == nil) {
	traversing = true;
    }
    i->parent = this;
    top = ymax - y2;
    w = x2 - x1 + 1;
    h = y2 - y1 + 1;
    if (i->canvas == nil) {
	id = XCreateWindow(canvas->id, x1, top, w, h, 0, 0, 0);
	i->canvas = new Canvas(id);
	Assign(i, x1, y1, w, h);
	InitCanvas(i);
    } else {
	id = i->canvas->id;
	XUnmapSubwindows(id);
	XConfigureWindow(id, x1, top, w, h);
	Assign(i, x1, y1, w, h);
    }
    if (map && i->GetStartIconic()) {
	i->Iconify();
    } else if (map && i->GetInteractorType() != IconInteractor) {
	XMapWindow(id);
	i->canvas->status = CanvasMapped;
    }
    if (!traversing) {
	i->Redraw(0, 0, i->xmax, i->ymax);
    }
    if (parent == nil) {
	traversing = false;
    }
}

void Scene::Map (register Interactor* i, boolean) {
    XMapWindow(i->canvas->id);
    i->canvas->status = CanvasMapped;
    if (parent != nil) {
	i->Draw();
    }
}

void Scene::Unmap (register Interactor* i) {
    XUnmapWindow(i->canvas->id);
    i->canvas->status = CanvasUnmapped;
}

void Scene::InitCanvas (register Interactor* i) {
    register XWindow id = i->canvas->id;

    const char* s = i->GetName();
    if (s != nil) {
	i->DoSetName(s);
    }
    Interactor* ic = i->GetIconInteractor();
    if (ic != nil) {
	i->DoSetIconInteractor(ic);
    }
    if (i->cursensor == nil) {
	if (i->input == nil) {
	    XSelectInput(id, ExposeRegion|ExposeCopy);
	} else {
	    i->cursensor = i->input;
	    XSelectInput(id, i->cursensor->mask);
	}
    } else {
	XSelectInput(id, i->cursensor->mask);
    }
    Cursor* c = i->GetCursor();
    if (c != nil) {
	XDefineCursor(id, c->Id());
    } else if (parent == nil) {
	XDefineCursor(id, defaultCursor->Id());
    }
    assocTable->Insert(id, i);
}

void Scene::Raise (Interactor* i) {
    DoRaise(i);
    if (i->canvas != nil && i->canvas->status != CanvasOffscreen) {
	XRaiseWindow(i->canvas->id);
    }
}

void Scene::Lower (Interactor* i) {
    DoLower(i);
    if (i->canvas != nil && i->canvas->status != CanvasOffscreen) {
	XLowerWindow(i->canvas->id);
    }
}

void Scene::Move (Interactor* i, Coord x, Coord y, Alignment a) {
    Coord ax = x, ay = y;
    DoAlign(i, a, ax, ay);
    DoMove(i, ax, ay);
    XMoveWindow(i->canvas->id, ax, ymax - ay - i->ymax);
}

/*
 * class Sensor
 */

int motionmask = MouseMoved;
int keymask = KeyPressed;
int entermask = EnterWindow;
int leavemask = LeaveWindow;
int focusmask = FocusChange;
int substructmask = 0;
int upmask = ButtonPressed|ButtonReleased;
int downmask = ButtonPressed|ButtonReleased;
int initmask = ExposeRegion|ExposeCopy;

static XWindow focus;
static Sensor* grabber;

boolean Sensor::Interesting (void* raw, Event& e) {
    boolean b;
    XEvent* x = (XEvent*) raw;
    register XMouseEvent* m = (XMouseEvent*)x;
    XFocusChangeEvent* f = (XFocusChangeEvent*)x;

    b = false;
    switch (x->type) {
	case MouseMoved:
	    e.eventType = MotionEvent;
	    e.x = m->x;
	    e.y = m->y;
	    e.w = nil;
	    e.wx = (m->location >> 16) & 0xffff;
	    e.wy = m->location & 0xffff;
	    b = true;
	    break;
	case KeyPressed:
	    e.eventType = KeyEvent;
	    e.GetButtonInfo(x);
	    b = ButtonIsSet(down, e.button);
	    break;
	case ButtonPressed:
	    e.eventType = DownEvent;
	    e.GetButtonInfo(x);
	    b = ButtonIsSet(down, e.button);
	    if (b && ButtonIsSet(up, e.button)) {
		grabber = this;
	    } else {
		grabber = nil;
	    }
	    break;
	case ButtonReleased:
	    e.eventType = UpEvent;
	    e.GetButtonInfo(x);
	    b = ButtonIsSet(up, e.button) || (grabber != nil);
	    break;
	case FocusChange:
	    if (f->subwindow == nil) {
		if (f->detail == EnterWindow) {
		    focus = f->window;
		    e.eventType = OnEvent;
		} else {
		    focus = nil;
		    e.eventType = OffEvent;
		}
		b = true;
	    }
	    break;
	case EnterWindow:
	    if (f->subwindow == nil && f->detail != 1 && f->window != focus) {
		e.eventType = OnEvent;
		b = true;
	    }
	    break;
	case LeaveWindow:
	    if (f->subwindow == nil && f->detail != 1 && f->window != focus) {
		e.eventType = OffEvent;
		b = true;
	    }
	    break;
	default:
	    /* ignore */;
    }
    return b;
}

void Event::GetButtonInfo (void* xe) {
    register XKeyOrButtonEvent* k = (XKeyOrButtonEvent*)xe;
    register int state;
    int n;

    timestamp = 10 * k->time;
    x = k->x;
    y = k->y;
    w = nil;
    wx = (k->location >> 16) & 0xffff;
    wy = k->location & 0xffff;
    state = k->detail;
    shift = (state & ShiftMask) != 0;
    control = (state & ControlMask) != 0;
    meta = (state & MetaMask) != 0;
    shiftlock = (state & ShiftLockMask) != 0;
    leftmouse = (state & LeftMask) != 0;
    middlemouse = (state & MiddleMask) != 0;
    rightmouse = (state & RightMask) != 0;
    button = state & 0xff;
    if (k->type == KeyPressed) {
	keystring = XLookupMapping(k, &n);
	len = n;
    } else {
	button = 2 - button;
	len = 0;
    }
    if (len == 0) {
	keystring = keydata;
	keydata[0] = '\0';
    }
}

/*
 * class World
 */

void World::Init (const char* device) {
    rep = new WorldRep;
    rep->_display = XOpenDisplay(device);
    if (rep->display() == nil) {
	fprintf(stderr, "fatal error: can't open display\n");
	exit(1);
    }
    rep->_root = XRootWindow;
    SetCurrent();
    if (assocTable == nil) {
	assocTable = new InteractorTable(4096);
    }
    canvas = new Canvas(rep->root());
    canvas->width = DisplayWidth();
    canvas->height = DisplayHeight();
    canvas->status = CanvasMapped;
    rep->_planemask = AllPlanes;
    rep->_nplanes = XDisplayPlanes();
    inch = 75.0; /* what X11 tells me for a VAXstation */
    inches = inch;
    cm = inch/2.54;
    point = 1;
    points = 1;
    assocTable->Insert(rep->root(), this);
    xmax = canvas->width - 1;
    ymax = canvas->height - 1;
    black = new Color("black");
    white = new Color("white");
    stdfont = new Font("6x13p");
    stdpaint = new Painter;
}

void World::FinishInit () {
    /* window system hints not implemented in X10 */
    extern void InitSensors(), InitCursors(Color*, Color*);

    RootConfig();
    InitSensors();
    InitCursors(output->GetFgColor(), output->GetBgColor());
}

World::~World () {
    assocTable->Remove(rep->root());
    XCloseDisplay(rep->display());
    delete rep;
}

void World::SaveCommandLine (int argc, char** argv) {
    saved_argc = argc;
    saved_argv = new char*[argc + 1];

    for (int i = 0 ; i < argc ; i++) {
	saved_argv[i] = argv[i];
    }
    saved_argv[i] = nil;
}

const char* World::UserDefaults () {
    return nil;
}

void World::FinishInsert (Interactor* i) {
    if (i->GetInteractorType() != IconInteractor) {
	XEvent xevent;
	XWindowEvent(i->canvas->id, ExposeWindow, &xevent);
	i->Draw();
    }
}

void World::DoChange (Interactor* i) {
    XEvent xevent;

    Coord x = 0;
    Coord y = 0;
    i->GetRelative(x, y, this);
    Shape* s = i->GetShape();
    Place(i, x, y, x + s->width - 1, y + s->height - 1);
    XWindowEvent(i->canvas->id, ExposeWindow, &xevent);
    i->Draw();
}

void World::DoRemove (Interactor*) {
    XFlush();
}

int World::Fileno () {
    return Xdpyno();
}

void World::SetCurrent () {
    XSetDisplay(rep->display());
    current = rep;
}

void World::SetRoot (void*) {}
void World::SetScreen (int) {}

int World::NPlanes () {
    return rep->nplanes();
}

int World::NButtons () {
    return 3;
}

int World::PixelSize () {
    return XYPixmapSize(32, 1, rep->nplanes()) >> 2;
}

const char* World::GetDefault (const char* name) {
    return GetAttribute(name);
}

const char* World::GetGlobalDefault (const char* name) {
    return GetAttribute(name);
}

unsigned World::ParseGeometry (
    const char* spec, Coord& x, Coord& y, int& w, int &h
) {
    return XParseGeometry(spec, &x, &y, &w, &h);
}

void World::RingBell (int v) {
    if (v > 7) {
	XFeep(7);
    } else if (v >= 0) {
	XFeep(v);
    }
}

void World::SetHint (const char*) {
    /* window system hints not implemented in X10*/
}

void World::SetKeyClick (int v) {
    XKeyClickControl(v);
}

void World::SetAutoRepeat (boolean b) {
    if (b) {
	XAutoRepeatOn();
    } else {
	XAutoRepeatOff();
    }
}

void World::SetFeedback (int thresh, int scale) {
    XMouseControl(scale, thresh);
}

/*
 * class WorldView
 */

void WorldView::Init (World*) {
    canvas = new Canvas(XRootWindow);
    canvas->width = DisplayWidth();
    canvas->height = DisplayHeight();
    canvas->status = CanvasMapped;
    xmax = canvas->width - 1;
    ymax = canvas->height - 1;
}

const int bmask = ButtonPressed|ButtonReleased;

void WorldView::GrabMouse (Cursor* c) {
    while (!XGrabMouse(world->canvas->id, c->Id(), bmask)) {
	sleep(1);
    }
}

void WorldView::UngrabMouse () {
    XUngrabMouse();
}

boolean WorldView::GrabButton (unsigned b, unsigned m, Cursor* c) {
    return XGrabButton(world->canvas->id, c->Id(), b | m, bmask);
}

void WorldView::UngrabButton (unsigned b, unsigned m) {
    XUngrabButton(b | m);
}

void WorldView::Lock () {
    XGrabServer();
}

void WorldView::Unlock () {
    XUngrabServer();
    Sync();
}

void WorldView::ClearInput () {
    XSync(1);
}

void WorldView::MoveMouse (Coord x, Coord y) {
    XWarpMouse(canvas->id, x, ymax - y);
}

void WorldView::Map (RemoteInteractor i) {
    XMapWindow(i);
}

void WorldView::MapRaised (RemoteInteractor i) {
    XMapWindow(i);
}

void WorldView::Unmap (RemoteInteractor i) {
    XUnmapWindow(i);
}

RemoteInteractor WorldView::Find (Coord x, Coord y) {
    RemoteInteractor i;
    Coord rx, ry;

    XInterpretLocator(world->canvas->id, &rx, &ry, &i, (x << 16) | (ymax - y));
    return i;
}

void WorldView::Move (RemoteInteractor i, Coord left, Coord top) {
    XMoveWindow(i, left, ymax - top);
}

void WorldView::Change (
    RemoteInteractor i, Coord left, Coord top, int w, int h
) {
    XConfigureWindow(i, left, ymax - top, w, h);
}

void WorldView::Raise (RemoteInteractor i) {
    XRaiseWindow(i);
}

void WorldView::Lower (RemoteInteractor i) {
    XLowerWindow(i);
}

void WorldView::Focus (RemoteInteractor i) {
    if (i != curfocus) {
	curfocus = i;
	XFocusKeyboard(i == nil ? world->canvas->id : i);
    }
}

void WorldView::GetList (RemoteInteractor*& ilist, int& n) {
    XWindow parent;

    XQueryTree(canvas->id, &parent, &n, &ilist);
}

void WorldView::GetInfo (
    RemoteInteractor i, Coord& x1, Coord& y1, Coord& x2, Coord& y2
) {
    XWindowInfo info;

    XQueryWindow(i, &info);
    x1 = info.x;
    y2 = ymax - info.y;
    x2 = info.x + info.width + 2*info.bdrwidth - 1;
    y1 = y2 - info.height - 2*info.bdrwidth + 1;
}

boolean WorldView::GetHints (
    RemoteInteractor, Coord&, Coord&, Shape& s
) {
    s.width = 0;
    s.height = 0;
    return false;
}

void WorldView::SetHints (RemoteInteractor i, Coord x, Coord y, Shape& s) {
    /* unimplemented on X10 */
}

RemoteInteractor WorldView::GetIcon (RemoteInteractor i) {
    XWindowInfo info;

    XQueryWindow(i, &info);
    return info.assoc_wind;
}

void WorldView::AssignIcon (RemoteInteractor i, RemoteInteractor icon) {
    XSetIconWindow(i, icon);
}

void WorldView::UnassignIcon (RemoteInteractor i) {
    XClearIconWindow(i);
}

RemoteInteractor WorldView::TransientOwner (RemoteInteractor) {
    return nil;
}

char* WorldView::GetName (RemoteInteractor i) {
    char* name;

    XFetchName(i, &name);
    return name;
}
