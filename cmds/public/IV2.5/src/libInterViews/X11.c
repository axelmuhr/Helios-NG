/*
 * X11-dependent code
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
#include <InterViews/sensor.h>
#include <InterViews/shape.h>
#include <InterViews/transformer.h>
#include <InterViews/world.h>
#include <InterViews/worldview.h>
#include <InterViews/X11/painterrep.h>
#include <InterViews/X11/worldrep.h>
#include <InterViews/X11/Xlib.h>
#include <InterViews/X11/Xutil.h>
#include <X11/Xatom.h>
#include <os/host.h>
#include <os/timing.h>
#include <bstring.h>
#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern void InitSensors(), InitCursors(Color*, Color*);

WorldRep* _world;
InteractorTable* assocTable;

static int saved_argc;
static char** saved_argv;

/*
 * class Bitmap
 */

static int BitmapImageSize (int w, int h) {
    return (w+7)/8 * h;
}

BitmapRep::BitmapRep (const char* filename) {
    XReadBitmapFile(
	_world->display(), _world->root(), filename, &width, &height,
	(Pixmap*)&map, &x0, &y0
    );
    if (x0 == -1) {
        x0 = 0;
    }
    if (y0 == -1) {
        y0 = 0;
    } else {
        y0 = height-1 - y0;
    }
    data = nil;
}

BitmapRep::BitmapRep (void* d, int w, int h, int x, int y) {
    width = w;
    height = h;
    x0 = x;
    y0 = y;
    int size = BitmapImageSize(width, height);
    char* newd = new char[size];
    if (d != nil) {
	bcopy(d, newd, size);
    } else {
	bzero(newd, size);
    }
    map = (void*)XCreateBitmapFromData(
	_world->display(), _world->root(), newd, w, h
    );
    data = nil;
}    

BitmapRep::BitmapRep (Font* f, int c) {
    XFontStruct* info = (XFontStruct*)f->Info();
    if (
        c >= info->min_char_or_byte2 && c <= info->max_char_or_byte2
        && info->per_char != nil
    ) {
        int i = c - info->min_char_or_byte2;
        width = info->per_char[i].rbearing - info->per_char[i].lbearing;
        height = info->per_char[i].ascent + info->per_char[i].descent;
        x0 = - info->per_char[i].lbearing;
        y0 = info->per_char[i].descent - 1;
    } else {
        width = info->max_bounds.rbearing - info->min_bounds.lbearing;
        height = info->max_bounds.ascent + info->max_bounds.descent;
        x0 = -info->min_bounds.lbearing;
        y0 = info->max_bounds.descent - 1;
    }
    if (width <= 0) width = 1;
    if (height <= 0) height = 1;
    map = (void*)XCreatePixmap(
        _world->display(), _world->root(), width, height, 1
    );
    GC gc = XCreateGC(_world->display(), (Pixmap)map, 0, nil);
    XSetFont(_world->display(), gc, info->fid);
    XSetForeground(_world->display(), gc, 0);
    XFillRectangle(_world->display(), (Pixmap)map, gc, 0, 0, width, height);
    XSetForeground(_world->display(), gc, 1);
    char ch = c;
    XDrawString(_world->display(), (Pixmap)map, gc, x0, height-1-y0, &ch, 1);
    XFreeGC(_world->display(), gc);
    data = nil;
}    

BitmapRep::BitmapRep (BitmapRep* b, BitTx t) {
    switch (t) {
    case NoTx: case FlipH: case FlipV: case Rot180: case Inv:
        width = b->width; height = b->height; break;
    case Rot90: case Rot270:
        width = b->height; height = b->width; break;
    }
    x0 = b->x0;
    y0 = b->y0;
    map = nil;
    int size = BitmapImageSize(width, height);
    char* d = new char[size];
    data = XCreateImage(
        _world->display(), _world->visual(),
        1, ZPixmap, 0, d, width, height, 8, 0
    );
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
            case Inv: bit = !b->GetBit(x, y); break;
            }
            PutBit(x, y, bit);
	}
    }
}

static void DrawSourceTransformedImage (
    XImage* s, int sx0, int sy0,
    XImage* m, int mx0, int my0,
    Drawable d, int height, int dx0, int dy0,
    boolean stencil, unsigned long fg, unsigned long bg,
    GC gc, Transformer* matrix,
    int xmin, int ymin, int xmax, int ymax
) {
    unsigned long lastdrawnpixel = fg;
    for (int xx = xmin; xx <= xmax; ++xx) {
        float lx, ly;
        float rx, ry;
        float tx, ty;
        matrix->Transform(float(xx), float(ymin), lx, ly);
        matrix->Transform(float(xx + 1), float(ymin), rx, ry);
        matrix->Transform(float(xx), float(ymax+1), tx, ty);
        float dx = (tx - lx) / float(ymax - ymin + 1);
        float dy = (ty - ly) / float(ymax - ymin + 1);
        int ilx = 0, ily = 0;
        int irx = 0, iry = 0;
        boolean lastmask = false, mask;
        unsigned long lastpixel = bg, pixel, source;
        for (int yy = ymin; yy <= ymax+1; ++yy) {
            mask = (
                yy <= ymax
                && (m == nil || XGetPixel(m, xx-mx0, m->height-1-(yy-my0)))
            );
            if (
                yy<sy0 || yy>=sy0+s->height || xx<sx0 || xx>=sx0+s->width
            ) {
                source = bg;
            } else {
                source = XGetPixel(s, xx-sx0, s->height-1-(yy-sy0));
            }
            if (stencil) {
                pixel = (source != 0) ? fg : bg;
            } else {
                pixel = source;
            }
            if (mask != lastmask || lastmask && pixel != lastpixel) {
                int iilx = round(lx), iily = round(ly);
                int iirx = round(rx), iiry = round(ry);
                if (lastmask) {
                    if (lastpixel != lastdrawnpixel) {
                        XSetForeground(_world->display(), gc, lastpixel);
                        lastdrawnpixel = lastpixel;
                    }
                    if (
                        (ilx==iilx || ily==iily) && (irx==ilx || iry==ily)
                    ) {
                        XFillRectangle(
                            _world->display(), d, gc,
                            min(ilx, iirx) - dx0,
                            height - (max(ily, iiry) - dy0),
                            abs(ilx - iirx), abs(ily - iiry)
                        );
                    } else {
                        XPoint v[4];
                        v[0].x = ilx-dx0; v[0].y = height - (ily-dy0);
                        v[1].x = iilx-dx0; v[1].y = height - (iily-dy0);
                        v[2].x = iirx-dx0; v[2].y = height - (iiry-dy0);
                        v[3].x = irx-dx0; v[3].y = height - (iry-dy0);
                        XFillPolygon(
                            _world->display(), d, gc,
                            v, 4, Convex, CoordModeOrigin
                        );
                    }
                }
                ilx = iilx; ily = iily;
                irx = iirx; iry = iiry;
                lastpixel = pixel;
                lastmask = mask;
            }
            lx += dx; ly += dy;
            rx += dx; ry += dy;
        }
    }
    XSetForeground(_world->display(), gc, fg);
}

static void DrawDestinationTransformedImage (
    XImage* s, int sx0, int sy0,
    XImage* m, int mx0, int my0,
    Drawable d, int height, int dx0, int dy0,
    boolean stencil, unsigned long fg, unsigned long bg,
    GC gc, Transformer* matrix,
    int xmin, int ymin, int xmax, int ymax
) {
    Transformer t(matrix);
    t.Invert();

    unsigned long lastdrawnpixel = fg;
    for (Coord xx = xmin; xx <= xmax; ++xx) {
        float fx, fy;
        float tx, ty;
        t.Transform(float(xx) + 0.5, float(ymin) + 0.5, fx, fy);
        t.Transform(float(xx) + 0.5, float(ymax) + 1.5, tx, ty);
        float dx = (tx - fx) / float(ymax - ymin + 1); 
        float dy = (ty - fy) / float(ymax - ymin + 1);
        Coord lasty = ymin;
        boolean lastmask = false, mask;
        unsigned long lastpixel = bg, pixel, source;
        for (Coord yy = ymin; yy <= ymax+1; ++yy) {
            int ix = round(fx - 0.5), iy = round(fy - 0.5);
            boolean insource = (
               ix >= sx0 && ix < sx0 + s->width
               && iy >= sy0 && iy < sy0 + s->height
            );
            boolean inmask = (
                m != nil && ix >= mx0 && ix < mx0 + m->width
                && iy >= my0 && iy < my0 + m->height
            );
            if (yy <= ymax) {
                if (m == nil) {
                    mask = insource;
                } else if (inmask) {
                    mask = XGetPixel(m, ix-mx0, m->height-1-(iy-my0)) != 0;
                } else {
                    mask = false;
                }
            } else {
                mask = false;
            }
            if (insource) {
                source = XGetPixel(s, ix-sx0, s->height-1-(iy-sy0));
            } else {
                source = bg;
            }
            if (stencil) {
                pixel = (source != 0) ? fg : bg;
            } else {
                pixel = source;
            }
            if (mask != lastmask || lastmask && pixel != lastpixel) {
                if (lastmask) {
                    if (lastpixel != lastdrawnpixel) {
                        XSetForeground(_world->display(), gc, lastpixel);
                        lastdrawnpixel = lastpixel;
                    }
                    XDrawLine(
                        _world->display(), d, gc,
                        xx - dx0, height - 1 - (lasty - dy0),
                        xx - dx0, height - 1 - (yy - 1 - dy0)
                    );
                }
                lastmask = mask;
                lastpixel = pixel;
                lasty = yy;
            }
            fx += dx;
            fy += dy;
        }
    }
    XSetForeground(_world->display(), gc, fg);
}

static void DrawTransformedImage (
    XImage* s, int sx0, int sy0,
    XImage* m, int mx0, int my0,
    Drawable d, int height, int dx0, int dy0,
    boolean stencil, unsigned long fg, unsigned long bg,
    GC gc, Transformer* matrix
) {
    int x1 = (m != nil) ? mx0 : sx0;
    int y1 = (m != nil) ? my0 : sy0;
    int x2 = (m != nil) ? mx0 : sx0;
    int y2 = (m != nil) ? my0 + m->height : sy0 + s->height;
    int x3 = (m != nil) ? mx0 + m->width : sx0 + s->width;
    int y3 = (m != nil) ? my0 + m->height : sy0 + s->height;
    int x4 = (m != nil) ? mx0 + m->width : sx0 + s->width;
    int y4 = (m != nil) ? my0 : sy0;

    int sxmin = min(x1, min(x2, min(x3, x4)));
    int sxmax = max(x1, max(x2, max(x3, x4))) - 1;
    int symin = min(y1, min(y2, min(y3, y4)));
    int symax = max(y1, max(y2, max(y3, y4))) - 1;

    matrix->Transform(x1, y1);
    matrix->Transform(x2, y2);
    matrix->Transform(x3, y3);
    matrix->Transform(x4, y4);

    int dxmin = min(x1,min(x2,min(x3,x4)));
    int dxmax = max(x1,max(x2,max(x3,x4))) - 1;
    int dymin = min(y1,min(y2,min(y3,y4)));
    int dymax = max(y1,max(y2,max(y3,y4))) - 1;

    int swidth = sxmax - sxmin + 1;
    int sheight = symax - symin + 1;
    int dwidth = dxmax - dxmin + 1;
    int dheight = dymax - dymin + 1;

    boolean rect = (x1==x2 || y1==y2) && (x1==x4 || y1==y4);
    boolean alwaysdest = dwidth < 2 * swidth;
    boolean alwayssource = dwidth * dheight > 3 * swidth * sheight;
    boolean dest;

    switch (_world->tximages()) {
    case TxImagesDefault:
    case TxImagesAuto:
        dest = alwaysdest || (!alwayssource && !rect);
        break;
    case TxImagesDest:
        dest = true;
        break;
    case TxImagesSource:
        dest = false;
        break;
    }
    if (dest) {
        if (dheight > 0) {
            DrawDestinationTransformedImage(
                s, sx0, sy0, m, mx0, my0, d, height, dx0, dy0,
                stencil, fg, bg, gc, matrix,
                dxmin, dymin, dxmax, dymax
            );
        }
    } else {
        if (sheight > 0) {
            DrawSourceTransformedImage(
                s, sx0, sy0, m, mx0, my0, d, height, dx0, dy0,
                stencil, fg, bg, gc, matrix,
                sxmin, symin, sxmax, symax
            );
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
    map = (void*)XCreatePixmap(
        _world->display(), _world->root(), width, height, 1
    );
    GC gc = XCreateGC(_world->display(), (Pixmap)map, 0, nil);
    XSetForeground(_world->display(), gc, 0);
    XFillRectangle(_world->display(), (Pixmap)map, gc, 0, 0, width, height);
    XSetForeground(_world->display(), gc, 1);
    DrawTransformedImage(
        (XImage*)b->GetData(), -b->x0, -b->y0,
        (XImage*)b->GetData(), -b->x0, -b->y0,
        (Pixmap)map, height, -x0, -y0,
        true, 1, 0, gc, &t
    );
    XFreeGC(_world->display(), gc);
    data = nil;
}

BitmapRep::~BitmapRep () {
    Touch();
    if (data != nil) {
        XDestroyImage((XImage*)data);
        data = nil;
    }
}

void BitmapRep::Touch () {
    if (map != nil) {
	XFreePixmap(_world->display(), (Pixmap)map);
	map = nil;
    }
}

void BitmapRep::PutBit (int x, int y, boolean bit) {
    XImage* image = (XImage*)GetData();
    if (image != nil) {
        XPutPixel(image, x, height - y - 1, bit);
    }
}

boolean BitmapRep::GetBit (int x, int y) {
    XImage* image = (XImage*)GetData();
    if (image != nil) {
        return XGetPixel(image, x, height - y - 1) != 0;
    } else {
        return false;
    }
}

void* BitmapRep::GetData () {
    if (data == nil && map != nil) {
        data = XGetImage(
            _world->display(), (Pixmap)map, 0, 0, width, height, 0x01, ZPixmap
        );
    }
    return data;
}        

void* BitmapRep::GetMap () {
    if (map == nil && data != nil) {
        map = (void*)XCreatePixmap(
            _world->display(), _world->root(), width, height, 1
        );
	GC gc = XCreateGC(_world->display(), (Pixmap)map, 0, nil);
        XPutImage(
            _world->display(), (Pixmap)map, gc, (XImage*)data,
            0, 0, 0, 0, width, height
        );
	XFreeGC(_world->display(), gc);
    }
    return map;
}

/*
 * Class Raster
 */

static int RasterImageSize (int w, int h) {
    return w * h * sizeof(unsigned long);
}

RasterRep::RasterRep (int w, int h) {
    width = w;
    height = h;
    data = nil;
}

RasterRep::RasterRep (Canvas* c, int x0, int y0, int w, int h) {
    width = w;
    height = h;
    data = XGetImage(
        _world->display(), (Drawable)c->Id(),
        x0, c->Height()-1-y0, width, height, AllPlanes, ZPixmap
    );
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

RasterRep::~RasterRep () {
    if (data != nil) {
        XDestroyImage((XImage*)data);
    }
}

void* RasterRep::GetData () {
    if (data == nil) {
        data = XCreateImage(
            _world->display(), _world->visual(),
            DefaultDepth(_world->display(), _world->screen()),
            ZPixmap, 0, new char[RasterImageSize(width, height)],
            width, height, sizeof(unsigned long), 0
        );
    }
    return data;
}

void RasterRep::PutPixel (int x, int y, int pixel) {
    XImage* image = (XImage*)GetData();
    XPutPixel(image, x, height - y - 1, pixel);
}

int RasterRep::GetPixel (int x, int y) {
    XImage* image = (XImage*)GetData();
    return XGetPixel(image, x, height - y - 1);
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

BrushRep::BrushRep (int* pattern, int c, int width) {
    if (c != 0) {
        if (
            _world->dash() == DashAll
            || width == 0 && _world->dash() == DashThin
            || width == 0 && _world->dash() == DashDefault
        ) {
            count = c;
            info = new char[count];
            for (int i = 0; i < count; ++i) {
                *((char*)info + i) = char(pattern[i]);
            }
        } else {
            count = 0;
            const int width = 32;
            const int height = 32;
            boolean bits[width];
            for (int i = 0; i < width; ++i) {
                bits[i] = PatternBit(i, pattern, c);
            }
            info = (void*)XCreatePixmap(
                _world->display(), _world->root(), width, height, 1
            );
            GC gc = XCreateGC(_world->display(), (Pixmap)info, 0, nil);
            XSetForeground(_world->display(), gc, 0);
            XFillRectangle(
                _world->display(), (Pixmap)info, gc, 0, 0, width, height
            );
            XSetForeground(_world->display(), gc, 1);
            for (int x = 0; x < width; ++x) {
                for (int y = 0; y < height; ++y) {
                    if (bits[(x + y)%width]) {
                        XDrawPoint(
                            _world->display(), (Drawable)info, gc, x, y
                        );
                    }
                }
            }
            XFreeGC(_world->display(), gc);
        }
    } else {
        count = 0;
        info = nil;
    }
}

BrushRep::~BrushRep () {
    if (info != nil) {
        if (count != 0) {
            delete info;
        } else {
            XFreePixmap(_world->display(), (Pixmap)info);
        }
    }
}

/*
 * class Color
 */

ColorRep::ColorRep (ColorIntensity r, ColorIntensity g, ColorIntensity b) {
    XColor* c = new XColor;
    c->red = r;
    c->green = g;
    c->blue = b;
    if (XAllocColor(_world->display(), _world->cmap(), c)) {
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
    XQueryColor(_world->display(), _world->cmap(), c);
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
    if (
        XAllocNamedColor(
            _world->display(), _world->cmap(), name, c, &exact
        )
    ) {
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
    /* don't deallocate for now - needs fixing
    unsigned long p[1];

    p[0] = ((XColor*)info)->pixel;
    XFreeColors(_world->display(), _world->cmap(), p, 1, 0);
    */
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

/*
 * Create the pixmap for a cursor.  These are always 16x16, unlike
 * fill patterns, which are 32x32.
 */

static Pixmap MakeCursorPixmap (int* scanline) {
#if vax
    /*
     * Cursor bitmaps work fine on the VAX; I suspect because
     * XPutImage doesn't have to do (misbehaving) swapping on a VAX.
     * The #else code fills the cursor pixmap explicitly because
     * cursor bitmaps don't seem to work right on other machines.
     */
    char data[2*cursorHeight];
    char* p;
    register int i, j;
    register unsigned int s1, s2;
    register unsigned int src, dst;
    union {
	char c[sizeof(short)];
	short s;
    } u;

    p = data;
    for (i = 0; i < cursorHeight; i++) {
	dst = 0;
	src = scanline[i];
	s1 = 1;
	s2 = 1 << (cursorWidth - 1);
	for (j = 0; j < cursorWidth; j++) {
	    if ((s1 & src) != 0) {
		dst |= s2;
	    }
	    s1 <<= 1;
	    s2 >>= 1;
	}
	u.s = dst;
	*p++ = u.c[0];
	*p++ = u.c[1];
    }
    return XCreateBitmapFromData(
	_world->display(), _world->root(), data, cursorWidth, cursorHeight
    );
#else
    /*
     * As best as I can tell, cursors created with a bitmap
     * don't work right.  The problem must be in the X11R2 library,
     * because the cursor has a slight glitch when running either
     * local on the Sun or remote to a VAX X server.
     *
     * I don't have the stomach to try to track down the problem,
     * so the simple solution is to draw into an off-screen pixmap.
     */

    Pixmap dst;
    GC g;
    register int i, j;
    register unsigned s1, s2;

    dst = XCreatePixmap(
        _world->display(), _world->root(), cursorWidth, cursorHeight, 1
    );
    g = XCreateGC(_world->display(), dst, 0, nil);
    XSetForeground(_world->display(), g, 0);
    XSetFillStyle(_world->display(), g, FillSolid);
    XFillRectangle(
        _world->display(), dst, g, 0, 0, cursorWidth, cursorHeight
    );
    XSetForeground(_world->display(), g, 1);
    for (i = 0; i < cursorHeight; i++) {
	s1 = scanline[i];
	s2 = 1;
	for (j = 0; j < cursorWidth; j++) {
	    if ((s1 & s2) != 0) {
		XDrawPoint(_world->display(), dst, g, cursorWidth - 1 - j, i);
	    }
	    s2 <<= 1;
	}
    }
    XFreeGC(_world->display(), g);
    return dst;
#endif
}

/*
 * Convert a Color object to the X representation.
 */

static void MakeColor (Color* c, XColor& xc) {
    int r, g, b;

    xc.pixel = c->PixelValue();
    c->Intensities(r, g, b);
    xc.red = r;
    xc.green = g;
    xc.blue = b;
}

Cursor::Cursor (Bitmap* pat, Bitmap* mask, Color* fg, Color* bg) {
    XColor f, b;

    MakeColor(fg, f);
    MakeColor(bg, b);
    id = (void*)XCreatePixmapCursor(
        _world->display(), (Pixmap)pat->Map(), (Pixmap)mask->Map(),
        &f, &b, -pat->Left(), pat->Height()-1 - (-pat->Bottom())
    );
}

Cursor::Cursor (Font* font, int pat, int mask, Color* fg, Color* bg) {
    XColor f, b;

    MakeColor(fg, f);
    MakeColor(bg, b);
    XFontStruct* i = (XFontStruct*)font->Info();
    id = (void*)XCreateGlyphCursor(
        _world->display(), i->fid, i->fid, pat, mask, &f, &b
    );
}

Cursor::Cursor (int n, Color* fg, Color* bg) {
    XColor f, b;

    MakeColor(fg, f);
    MakeColor(bg, b);
    id = (void*)XCreateFontCursor(_world->display(), n);
    XRecolorCursor(_world->display(), (XCursor)id, &f, &b);
}

void* Cursor::Id () {
    if (id == nil && pat != nil && mask != nil) {
	Pixmap p, m;
	XColor f, b;

	p = MakeCursorPixmap(pat);
	m = MakeCursorPixmap(mask);
	MakeColor(foreground, f);
	MakeColor(background, b);
	id = (void*)XCreatePixmapCursor(
	    _world->display(), p, m, &f, &b, x, cursorHeight - 1 - y
	);
	XFreePixmap(_world->display(), p);
	XFreePixmap(_world->display(), m);
    }
    return id;
}

Cursor::~Cursor () {
    if (id != nil) {
	XFreeCursor(_world->display(), (XCursor)id);
    }
}

/*
 * class Font
 */

void Font::GetFontByName (const char* name) {
    rep->info = XLoadQueryFont(_world->display(), name);
    Init();
}

inline boolean IsFixedWidth (XFontStruct* i) {
    return i->min_bounds.width == i->max_bounds.width;
}

void Font::Init () {
    register XFontStruct* i = (XFontStruct*)rep->info;
    if (i != nil) {
	rep->id = (void*)i->fid;
	rep->height = i->ascent + i->descent;
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
    if (info != nil && LastRef()) {
	XFreeFont(_world->display(), (XFontStruct*)info);
    }
}

int Font::Baseline () {
    XFontStruct* i = (XFontStruct*)rep->info;
    return i->descent - 1;
}

int Font::Width (const char* s) {
    return XTextWidth((XFontStruct*)rep->info, s, strlen(s));
}

int Font::Width (const char* s, int len) {
    register const char* p, * q;

    q = &s[len];
    for (p = s; *p != '\0' && p < q; p++);
    return XTextWidth((XFontStruct*)rep->info, s, p - s);
}

int Font::Index (const char* s, int len, int offset, boolean between) {
    register XFontStruct* i = (XFontStruct*)rep->info;
    register const char* p;
    register int n, w;
    int coff, cw;

    if (offset < 0 || *s == '\0' || len == 0) {
	return 0;
    }
    if (IsFixedWidth(i)) {
	cw = i->min_bounds.width;
	n = offset / cw;
	coff = offset % cw;
    } else {
	w = 0;
	for (p = s, n = 0; *p != '\0' && n < len; ++p, ++n) {
	    cw = XTextWidth(i, p, 1);
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
    register XFontStruct* i = (XFontStruct*)rep->info;
    return IsFixedWidth(i);
}

PainterRep::PainterRep () {
    fillgc = XCreateGC(_world->display(), _world->root(), 0, nil);
    dashgc = XCreateGC(_world->display(), _world->root(), 0, nil);
    fillbg = true;
    overwrite = false;
    xor = false;
}

PainterRep::~PainterRep () {
    XFreeGC(_world->display(), fillgc);
    XFreeGC(_world->display(), dashgc);
}

void PainterRep::PrepareFill (void* info) {
    if (info == nil) {
        XSetFillStyle(_world->display(), fillgc, FillSolid);
    } else if (fillbg) {
        XSetStipple(_world->display(), fillgc, (Pixmap)info);
        XSetFillStyle(_world->display(), fillgc, FillOpaqueStippled);
    } else {
        XSetStipple(_world->display(), fillgc, (Pixmap)info);
        XSetFillStyle(_world->display(), fillgc, FillStippled);
    }
}

void PainterRep::PrepareDash (int width, void* info, int count) {
    if (count == 0) {
        XSetLineAttributes(
            _world->display(), dashgc, width,
            LineSolid, CapButt, JoinMiter
        );
        if (info == nil) {
            XSetFillStyle(
                _world->display(), dashgc, FillSolid
            );
        } else if (fillbg) {
            XSetFillStyle(
                _world->display(), dashgc, FillOpaqueStippled
            );
            XSetStipple(_world->display(), dashgc, (Pixmap)info);
        } else {
            XSetFillStyle(
                _world->display(), dashgc, FillStippled
            );
            XSetStipple(_world->display(), dashgc, (Pixmap)info);
        }
    } else {
        XSetFillStyle(_world->display(), dashgc, FillSolid);
        if (info == nil) {
            XSetLineAttributes(
                _world->display(), dashgc, width,
                LineSolid, CapButt, JoinMiter
            );
        } else if (fillbg) {
            XSetLineAttributes(
                _world->display(), dashgc, width,
                LineDoubleDash, CapButt, JoinMiter
            );
            XSetDashes(
                _world->display(), dashgc, 0, (char*)info, count
            );
        } else {
            XSetLineAttributes(
                _world->display(), dashgc, width,
                LineOnOffDash, CapButt, JoinMiter
            );
            XSetDashes(
                _world->display(), dashgc, 0, (char*)info, count
            );
        }
    }
}

/*
 * Short-hand for allocating a vector of X points.
 * The idea is to use a static array if possible; otherwise
 * allocate/deallocate off the heap.
 */

static const XPointListSize = 200;
static XPoint xpoints[XPointListSize];

inline XPoint* AllocPts (int n) {
    return (n <= XPointListSize) ? xpoints : new XPoint[n];
}

inline void FreePts (XPoint* v) {
    if (v != xpoints) {
	delete v;
    }
}

Painter::Painter () {
    rep = new PainterRep;
    Init();
}

Painter::Painter (Painter* copy) {
    rep = new PainterRep;
    rep->fillbg = copy->rep->fillbg;
    rep->overwrite = copy->rep->overwrite;
    rep->xor = copy->rep->xor;
    Copy(copy);
    if (rep->overwrite) {
	XSetSubwindowMode(_world->display(), rep->fillgc, IncludeInferiors);
	XSetSubwindowMode(_world->display(), rep->dashgc, IncludeInferiors);
    }
}

Painter::~Painter () {
    if (LastRef()) {
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
    if (rep->fillbg != b) {
        if (rep->xor) {
            End_xor();
        }
        rep->fillbg = b;
        if (pattern != nil) {
            rep->PrepareFill(pattern->info);
        }
        if (br != nil) {
            rep->PrepareDash(br->width, br->rep->info, br->rep->count);
        }
    }
}

boolean Painter::BgFilled () {
    return rep->fillbg;
}

void Painter::SetColors (Color* f, Color* b) {
    if (rep->xor) {
	End_xor();
    }
    if (f != nil && foreground != f) {
        delete foreground;
	foreground = f;
        foreground->Reference();
        XSetForeground(
            _world->display(), rep->fillgc, foreground->PixelValue()
        );
        XSetForeground(
            _world->display(), rep->dashgc, foreground->PixelValue()
        );
    }
    if (b != nil && background != b) {
        delete background;
	background = b;
        background->Reference();
	XSetBackground(
            _world->display(), rep->fillgc, background->PixelValue()
        );
	XSetBackground(
            _world->display(), rep->dashgc, background->PixelValue()
        );
    }
}

void Painter::SetPattern (Pattern* pat) {
    if (rep->xor) {
	End_xor();
    }
    if (pattern != pat) {
        delete pattern;
	pattern = pat;
        if (pattern != nil) {
            pattern->Reference();
            rep->PrepareFill(pattern->info);
        }
    }
}

void Painter::SetBrush (Brush* b) {
    if (rep->xor) {
	End_xor();
    }
    if (br != b) {
        delete br;
	br = b;
	if (br != nil) {
            br->Reference();
            rep->PrepareDash(br->width, br->rep->info, br->rep->count);
	}
    }
}

void Painter::SetFont (Font* f) {
    if (font != f) {
        delete font;
        font = f;
        if (font != nil) {
            font->Reference();
            XSetFont(_world->display(), rep->fillgc, (XFont)font->rep->id);
        }
    }
}

void Painter::Clip (
    Canvas* c, Coord left, Coord bottom, Coord right, Coord top
) {
    XRectangle r[1];
    Coord x, y;

    if (left > right) {
	x = right; r[0].width = left - right + 1;
    } else {
	x = left; r[0].width = right - left + 1;
    }
    if (bottom > top) {
	y = bottom; r[0].height = bottom - top + 1;
    } else {
	y = top; r[0].height = top - bottom + 1;
    }
    r[0].x = x;
    r[0].y = c->height - 1 - y;
    if (r[0].x == 0 && r[0].y == 0 &&
	r[0].width == c->width && r[0].height == c->height
    ) {
	/* clipping to entire canvas is equivalent to no clipping at all */
	XSetClipMask(_world->display(), rep->fillgc, None);
	XSetClipMask(_world->display(), rep->dashgc, None);
    } else {
	XSetClipRectangles(
            _world->display(), rep->fillgc, 0, 0, r, 1, Unsorted
        );
	XSetClipRectangles(
            _world->display(), rep->dashgc, 0, 0, r, 1, Unsorted
        );
    }
}

void Painter::NoClip () {
    XSetClipMask(_world->display(), rep->fillgc, None);
    XSetClipMask(_world->display(), rep->dashgc, None);
}

void Painter::SetOverwrite (boolean children) {
    if (rep->overwrite != children) {
	rep->overwrite = children;
	XSetSubwindowMode(
	    _world->display(), rep->fillgc,
            children ? IncludeInferiors : ClipByChildren
	);
	XSetSubwindowMode(
	    _world->display(), rep->dashgc,
            children ? IncludeInferiors : ClipByChildren
	);
    }
}

void Painter::SetPlaneMask (int m) {
    XSetPlaneMask(_world->display(), rep->fillgc, m);
    XSetPlaneMask(_world->display(), rep->dashgc, m);
}

void Painter::Map (Canvas* c, Coord x, Coord y, Coord& mx, Coord& my) {
    if (matrix == nil) {
	mx = x; my = y;
    } else {
	matrix->Transform(x, y, mx, my);
    }
    mx += xoff;
    my = c->height - 1 - (my + yoff);
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
	    *myp = c->height - 1 - (*yp + yoff);
	}
    } else {
	for (; xp < lim; xp++, yp++, mxp++, myp++) {
	    matrix->Transform(*xp, *yp, *mxp, *myp);
	    *mxp += xoff;
	    *myp = c->height - 1 - (*myp + yoff);
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
	    *myp = round(c->height - 1 - (*yp + yoff));
	}
    } else {
	for (; xp < lim; xp++, yp++, mxp++, myp++) {
	    matrix->Transform(*xp, *yp, tmpx, tmpy);
	    *mxp = round(tmpx + xoff);
	    *myp = round(c->height - 1 - (tmpy + yoff));
	}
    }
}

void Painter::Begin_xor () {
    if (!rep->xor) {
	rep->xor = true;
	XSetFunction(_world->display(), rep->fillgc, GXxor);
	XSetForeground(_world->display(), rep->fillgc, _world->xor());
	XSetFillStyle(_world->display(), rep->fillgc, FillSolid);
	XSetFunction(_world->display(), rep->dashgc, GXxor);
	XSetForeground(_world->display(), rep->dashgc, _world->xor());
	XSetFillStyle(_world->display(), rep->dashgc, FillSolid);
    }
}

void Painter::End_xor () {
    if (rep->xor) {
	rep->xor = false;
	XSetFunction(_world->display(), rep->fillgc, GXcopy);
	XSetForeground(
            _world->display(), rep->fillgc, foreground->PixelValue()
        );
        if (pattern != nil) {
            rep->PrepareFill(pattern->info);
        }
	XSetFunction(_world->display(), rep->dashgc, GXcopy);
	XSetForeground(
            _world->display(), rep->dashgc, foreground->PixelValue()
        );
        if (br != nil) {
            rep->PrepareDash(br->width, br->rep->info, br->rep->count);
        }
    }
}

static int TxKey (Transformer* t, int x, int y) {
    if (t == nil) {
        return 0;
    } else {
        Coord x1, y1, x2, y2, x3, y3;
        t->Transform(0, 0, x1, y1);
        t->Transform(0, y, x2, y2);
        t->Transform(x, 0, x3, y3);
        return (
              (char(x2 - x1) << 24)
            + (char(y2 - y1 - y) << 16)
            + (char(x3 - x1 - x) << 8)
            + (char(y3 - y1))
        );
    }
}

void Painter::Stencil (Canvas* c, Coord x, Coord y, Bitmap* d, Bitmap* m) {
    if (rep->xor) {
        End_xor();
    }
    int tx = TxKey(matrix, d->Width(), d->Height());
    if (tx == 0) {
        Coord dx, dy;
        Map(c, x + d->Left(), y + d->Top(), dx, dy);
        if (m == nil) {
            XCopyPlane(
                _world->display(), (Pixmap)d->Map(), (Drawable)c->Id(),
                rep->fillgc, 0, 0, d->Width(), d->Height(), dx, dy, 1
            );
        } else if (m == d) {
            XSetForeground(_world->display(), rep->fillgc, 0);
            XSetBackground(_world->display(), rep->fillgc, AllPlanes);
            XSetFunction(_world->display(), rep->fillgc, GXand);
            XCopyPlane(
                _world->display(), (Pixmap)d->Map(), (Drawable)c->Id(),
                rep->fillgc, 0, 0, d->Width(), d->Height(), dx, dy, 1
            );
            XSetForeground(
                _world->display(), rep->fillgc, foreground->PixelValue()
            );
            XSetBackground(_world->display(), rep->fillgc, 0);
            XSetFunction(_world->display(), rep->fillgc, GXxor);
            XCopyPlane(
                _world->display(), (Pixmap)d->Map(), (Drawable)c->Id(),
                rep->fillgc, 0, 0, d->Width(), d->Height(), dx, dy, 1
            );
            XSetBackground(
                _world->display(), rep->fillgc, background->PixelValue()
            );
            XSetFunction(_world->display(), rep->fillgc, GXcopy);
        } else {
            GC gc = XCreateGC(_world->display(), _world->root(), 0, nil);
            XSetForeground(_world->display(), gc, foreground->PixelValue());
            XSetBackground(_world->display(), gc, background->PixelValue());
            XSetGraphicsExposures(_world->display(), gc, False);
            Coord mx, my;
            Map(c, x + m->Left(), y + m->Top(), mx, my);
            XSetClipOrigin(_world->display(), gc, mx, my);
            XSetClipMask(_world->display(), gc, (Pixmap)m->Map());
            XCopyPlane(
                _world->display(), (Pixmap)d->Map(), (Drawable)c->Id(),
                gc, 0, 0, d->Width(), d->Height(), dx, dy, 1
            );
            XFreeGC(_world->display(), gc);
        }
    } else {
        if (m != nil) {
            DrawTransformedImage(
                (XImage*)d->rep->GetData(), x + d->Left(), y + d->Bottom(),
                (XImage*)m->rep->GetData(), x + m->Left(), y + m->Bottom(),
                (Drawable)c->Id(), c->height, -xoff, -yoff,
                true, foreground->PixelValue(), background->PixelValue(),
                rep->fillgc, matrix
            );
        } else {
            DrawTransformedImage(
                (XImage*)d->rep->GetData(), x + d->Left(), y + d->Bottom(),
                nil, 0, 0,
                (Drawable)c->Id(), c->height, -xoff, -yoff,
                true, foreground->PixelValue(), background->PixelValue(),
                rep->fillgc, matrix
            );
        }
    }
}

void Painter::RasterRect (Canvas* c, Coord x, Coord y, Raster* r) {
    int tx = TxKey(matrix, r->Width(), r->Height());
    if (tx == 0) {
        Coord mx, my;
        Map(c, x, y + r->Height() - 1, mx, my);
        XPutImage(
            _world->display(), (Drawable)c->id, rep->fillgc,
            (XImage*)r->rep->GetData(),
            0, 0, mx, my, r->Width(), r->Height()
        );
    } else {
        DrawTransformedImage(
            (XImage*)r->rep->GetData(), x, y,
            nil, 0, 0,
            (Drawable)c->Id(), c->Height(), -xoff, -yoff,
            false, 0, 0,
            rep->fillgc, matrix
        );
    }
}

#include "btable.h"
static BitmapTable* btable = nil;
static BitmapTable* txtable = nil;

static Bitmap* GetCharBitmap (
    Font* f, int c, int k, Transformer* t
) {
    
    if (btable == nil) {
        btable = new BitmapTable(256);
        txtable = new BitmapTable(1024);
    }
    Bitmap* basic;
    int fid = ((XFontStruct*)f->Info())->fid;
    if (!btable->Find(basic, fid, c)) {
        basic = new Bitmap(f, c);
        btable->Insert(fid, c, basic);
    }
    Bitmap* tx;
    int mapid = Pixmap(basic->Map());
    if (!txtable->Find(tx, mapid, k)) {
        tx = new Bitmap(basic);
        tx->Transform(t);
        txtable->Insert(mapid, k, tx);
    }
    return tx;
}

static void FlushCharBitmaps () {
    if (btable != nil) {
        /* not implemented, but should go something like this
        Bitmap* b;
        while (!btable->Empty()) {
            b = btable->RemoveEntry();
            delete b;
        }
        */
        delete btable;
        btable = nil;
    }
    if (txtable != nil) {
        /* not implemented, but should go something like this
        Bitmap* b;
        while (!txtable->Empty()) {
            b = txtable->RemoveEntry();
            delete b;
        }
        */
        delete txtable;
        txtable = nil;
    }
}

void Painter::Text (Canvas* c, const char* s, int len, Coord x, Coord y) {
    Coord x0, y0;
    Coord ybase = y + font->Baseline();
    Coord ytop = y + font->Height() - 1;
    int txstring = TxKey(matrix, font->Width(s, len), font->Height());

    if (style & Reversed) {
        SetColors(GetBgColor(), GetFgColor());
    }
    if (txstring == 0) {
        Map(c, x, ybase, x0, y0);
        if (rep->fillbg) {
            XDrawImageString(
                _world->display(), (Drawable)c->id,
                rep->fillgc, x0, y0, s, len
            );
        } else {
            XDrawString(
                _world->display(), (Drawable)c->id,
                rep->fillgc, x0, y0, s, len
            );
        }
        if (style & Boldface) {
            XDrawString(
                _world->display(), (Drawable)c->id,
                rep->fillgc, x0-1, y0, s, len
            );
        }
    } else {
        Coord curx = x;
        float fx0, fy0;
        Transformer notrans(matrix);
        notrans.Transform(0.0, 0.0, fx0, fy0);
        notrans.Translate(-fx0, -fy0);
        int txchar = TxKey(matrix, font->Width("M"), font->Height());
        Bitmap* bits;
        for (int i = 0; i < len; ++i) {
            Coord nextx = curx + font->Width(s+i, 1);
            if (rep->fillbg) {
                ClearRect(c, curx, y, nextx, ytop);
            }
            switch (_world->txfonts()) {
            case TxFontsOff:
                Map(c, curx, ybase, x0, y0);
                XDrawString(
                    _world->display(), (Drawable)c->id,
                    rep->fillgc, x0, y0, s+i, 1
                );
                if (style & Boldface) {
                    XDrawString(
                        _world->display(), (Drawable)c->id,
                        rep->fillgc, x0-1, y0, s+i, 1
                    );
                }
                break;
            case TxFontsOn:
                bits = new Bitmap(font, s[i]);
                Stencil(c, curx, ybase, bits, bits);
                if (style & Boldface) {
                    Stencil(c, curx-1, ybase, bits, bits);
                }
                delete bits;
                break;
            case TxFontsCache:
            case TxFontsDefault:
                bits = GetCharBitmap(font, s[i], txchar, &notrans);
                Transformer* oldmatrix = matrix;
                matrix = nil;
                oldmatrix->Transform(curx, ybase, x0, y0);
                Stencil(c, x0, y0, bits, bits);
                if (style & Boldface) {
                    oldmatrix->Transform(curx+1, ybase, x0, y0);
                    Stencil(c, x0, y0, bits, bits);
                }
                matrix = oldmatrix;
                break;
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
    XDrawPoint(_world->display(), (Drawable)c->id, rep->fillgc, mx, my);
}

void Painter::MultiPoint (Canvas* c, Coord x[], Coord y[], int n) {
    register XPoint* v;
    register int i;

    v = AllocPts(n);
    for (i = 0; i < n; i++) {
	Map(c, x[i], y[i], v[i].x, v[i].y);
    }
    XDrawPoints(
        _world->display(), (Drawable)c->id,
        rep->fillgc, v, n, CoordModeOrigin
    );
    FreePts(v);
}

void Painter::Line (Canvas* c, Coord x1, Coord y1, Coord x2, Coord y2) {
    Coord mx1, my1, mx2, my2;

    Map(c, x1, y1, mx1, my1);
    Map(c, x2, y2, mx2, my2);
    XDrawLine(
        _world->display(), (Drawable)c->id, rep->dashgc, mx1, my1, mx2, my2
    );
}

void Painter::Rect (Canvas* c, Coord x1, Coord y1, Coord x2, Coord y2) {
    if (matrix != nil && matrix->Rotated() && !matrix->Rotated90()) {
	Coord x[4], y[4];

	x[0] = x[3] = x1;
	x[1] = x[2] = x2;
	y[0] = y[1] = y1;
	y[2] = y[3] = y2;
	Polygon(c, x, y, 4);
    } else {
	Coord left, bottom, right, top, tmp;
	int w, h;

	Map(c, x1, y1, left, bottom);
	Map(c, x2, y2, right, top);
	if (left > right) {
	    tmp = left; left = right; right = tmp;
	}
	if (top > bottom) {
	    tmp = bottom; bottom = top; top = tmp;
	}
	w = right - left;
	h = bottom - top;
	XDrawRectangle(
            _world->display(), (Drawable)c->id, rep->dashgc, left, top, w, h
        );
    }
}

void Painter::FillRect (Canvas* c, Coord x1, Coord y1, Coord x2, Coord y2) {
    if (matrix != nil && matrix->Rotated() && !matrix->Rotated90()) {
	Coord x[4], y[4];

	x[0] = x[3] = x1;
	x[1] = x[2] = x2;
	y[0] = y[1] = y1;
	y[2] = y[3] = y2;
	FillPolygon(c, x, y, 4);
    } else {
	Coord left, bottom, right, top, tmp;
	int w, h;

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
	XFillRectangle(
            _world->display(), (Drawable)c->id, rep->fillgc, left, top, w, h
        );
    }
}

void Painter::ClearRect (Canvas* c, Coord x1, Coord y1, Coord x2, Coord y2) {
    XSetForeground(_world->display(), rep->fillgc, background->PixelValue());
    Pattern* curpat = pattern;
    SetPattern(solid);
    FillRect(c, x1, y1, x2, y2);
    XSetForeground(_world->display(), rep->fillgc, foreground->PixelValue());
    SetPattern(curpat);
}

void Painter::Circle (Canvas* c, Coord x, Coord y, int r) {
    if (matrix != nil && (matrix->Stretched() || matrix->Rotated())) {
	Ellipse(c, x, y, r, r);
    } else {
	Coord left, top, right, bottom;

	Map(c, x-r, y+r, left, top);
        Map(c, x+r, y-r, right, bottom);
	XDrawArc(
	    _world->display(), (Drawable)c->id, rep->dashgc,
            left, top, right-left, bottom-top, 0, 360*64
	);
    }
}

void Painter::FillCircle (Canvas* c, Coord x, Coord y, int r) {
    if (matrix != nil && (matrix->Stretched() || matrix->Rotated())) {
	FillEllipse(c, x, y, r, r);
    } else {
	Coord left, top, right, bottom;

	Map(c, x-r, y+r, left, top);
        Map(c, x+r, y-r, right, bottom);
	XFillArc(
	    _world->display(), (Drawable)c->id, rep->fillgc, 
            left, top, right-left, bottom-top, 0, 360*64
	);
    }
}

void Painter::MultiLine (Canvas* c, Coord x[], Coord y[], int n) {
    register XPoint* v;
    register int i;

    v = AllocPts(n);
    for (i = 0; i < n; i++) {
	Map(c, x[i], y[i], v[i].x, v[i].y);
    }
    XDrawLines(
        _world->display(), (Drawable)c->id,
        rep->dashgc, v, n, CoordModeOrigin
    );
    FreePts(v);
}

void Painter::MultiLineNoMap (Canvas* c, Coord x[], Coord y[], int n) {
    register XPoint* v;
    register int i;

    v = AllocPts(n);
    for (i = 0; i < n; i++) {
	v[i].x = x[i];
	v[i].y = y[i];
    }
    XDrawLines(
        _world->display(), (Drawable)c->id,
        rep->dashgc, v, n, CoordModeOrigin
    );
    FreePts(v);
}

void Painter::Polygon (Canvas* c, Coord x[], Coord y[], int n) {
    register XPoint* v;
    register int i;

    v = AllocPts(n+1);
    for (i = 0; i < n; i++) {
	Map(c, x[i], y[i], v[i].x, v[i].y);
    }
    if (x[i-1] != x[0] || y[i-1] != y[0]) {
	v[i] = v[0];
	++i;
    }
    XDrawLines(
        _world->display(), (Drawable)c->id,
        rep->dashgc, v, i, CoordModeOrigin
    );
    FreePts(v);
}

void Painter::FillPolygonNoMap (Canvas* c, Coord x[], Coord y[], int n) {
    register XPoint* v;
    register int i;

    v = AllocPts(n);
    for (i = 0; i < n; i++) {
	v[i].x = x[i];
	v[i].y = y[i];
    }
    XFillPolygon(
	_world->display(), (Drawable)c->id, rep->fillgc,
        v, n, Complex, CoordModeOrigin
    );
    FreePts(v);
}

void Painter::FillPolygon (Canvas* c, Coord x[], Coord y[], int n) {
    register XPoint* v;
    register int i;

    v = AllocPts(n+1);
    for (i = 0; i < n; i++) {
	Map(c, x[i], y[i], v[i].x, v[i].y);
    }
    XFillPolygon(
	_world->display(), (Drawable)c->id, rep->fillgc,
        v, n, Complex, CoordModeOrigin
    );
    FreePts(v);
}

void Painter::Copy (
    Canvas* src, Coord x1, Coord y1, Coord x2, Coord y2,
    Canvas* dst, Coord x0, Coord y0
) {
    Coord sx1, sy1, sx2, sy2, sx3, sy3, sx4, sy4, dx1, dy1;
    Transformer t(matrix);

    t.Transform(x1, y1, sx1, sy1);
    t.Transform(x1, y2, sx2, sy2);
    t.Transform(x2, y2, sx3, sy3);
    t.Transform(x2, y1, sx4, sy4);
    t.Transform(x0, y0, dx1, dy1);

    int minx = min(sx1, min(sx2, min(sx3, sx4)));
    int maxx = max(sx1, max(sx2, max(sx3, sx4)));
    int miny = min(sy1, min(sy2, min(sy3, sy4)));
    int maxy = max(sy1, max(sy2, max(sy3, sy4)));

    int w = maxx - minx + 1;
    int h = maxy - miny + 1;
    int sx = minx + xoff;
    int sy = src->height - 1 - (maxy + yoff);
    int dx = dx1 - (sx1 - minx) + xoff;
    int dy = dst->height - 1 - (dy1 - (sy1 - maxy) + yoff);

    if ((sx1 == sx2 || sy1 == sy2) && (sx1 == sx4 || sy1 == sy4)) {
        if (src->status == CanvasOffscreen) {
            XSetGraphicsExposures(_world->display(), rep->fillgc, False);
            XCopyArea(
                _world->display(), (Drawable)src->id, (Drawable)dst->id,
                rep->fillgc, sx, sy, w, h, dx, dy
            );
            XSetGraphicsExposures(_world->display(), rep->fillgc, True);
        } else {
            XCopyArea(
                _world->display(), (Drawable)src->id, (Drawable)dst->id,
                rep->fillgc, sx, sy, w, h, dx, dy
            );
            dst->WaitForCopy();
        }
    } else {
        GC copygc = XCreateGC(_world->display(), (Drawable)dst->id, 0, nil);
        Pixmap mask;
        mask = XCreatePixmap(_world->display(), _world->root(), w, h, 1);
        GC maskgc = XCreateGC(_world->display(), mask, 0, nil);
        XSetForeground(_world->display(), maskgc, 0);
        XFillRectangle(_world->display(), mask, maskgc, 0, 0, w, h);
        XSetForeground(_world->display(), maskgc, 1);
        XPoint v[4];
        v[0].x = sx1 - minx; v[0].y = maxy - sy1;
        v[1].x = sx2 - minx; v[1].y = maxy - sy2;
        v[2].x = sx3 - minx; v[2].y = maxy - sy3;
        v[3].x = sx4 - minx; v[3].y = maxy - sy4;
        XFillPolygon(
            _world->display(), mask, maskgc,
            v, 4, Convex, CoordModeOrigin
        );
        XFreeGC(_world->display(), maskgc);
        XSetClipOrigin(_world->display(), copygc, dx, dy);
        XSetClipMask(_world->display(), copygc, mask);
        if (src->status == CanvasOffscreen) {
            XSetGraphicsExposures(_world->display(), copygc, False);
            XCopyArea(
                _world->display(), (Drawable)src->id, (Drawable)dst->id,
                copygc, sx, sy, w, h, dx, dy
            );
            XSetGraphicsExposures(_world->display(), copygc, True);
        } else {
            XCopyArea(
                _world->display(), (Drawable)src->id, (Drawable)dst->id,
                copygc, sx, sy, w, h, dx, dy
            );
            dst->WaitForCopy();
        }
        XFreePixmap(_world->display(), mask);
        XFreeGC(_world->display(), copygc);
    }
}

void Painter::Read (
    Canvas* c, void* dst, Coord x1, Coord y1, Coord x2, Coord y2
) {
    /* unimplemented -- use Raster::Raster(Canvas*, ...) */
}

void Painter::Write (
    Canvas* c, const void* src, Coord x1, Coord y1, Coord x2, Coord y2
) {
    /* unimplemented -- use Painter::RasterRect(Canvas*, ...) */
}

/*
 * class Pattern
 */

/*
 * Create a Pixmap for stippling from the given array of data.
 * The assumption is that the data is 16x16 and should be expanded to 32x32.
 */

static Pixmap MakeStipple (int* p) {
    int data[32];
    register int i, j;
    register unsigned int s1, s2;
    register unsigned int src, dst;

    for (i = 0; i < patternHeight; i++) {
	dst = 0;
	src = p[i];
	s1 = 1;
	s2 = 1 << (patternWidth - 1);
	for (j = 0; j < patternWidth; j++) {
	    if ((s1 & src) != 0) {
		dst |= s2;
	    }
	    s1 <<= 1;
	    s2 >>= 1;
	}
	dst = (dst << 16) | dst;
	data[i] = dst;
	data[i+16] = dst;
    }
    return XCreateBitmapFromData(
        _world->display(), _world->root(), data, 32, 32
    );
}

Pattern::Pattern (Bitmap* b) {
    int width = b->Width();
    int height = b->Height();
    info = (void*)XCreatePixmap(
        _world->display(), _world->root(), width, height, 1
    );
    GC gc = XCreateGC(_world->display(), (Pixmap)info, 0, nil);
    XCopyArea(
        _world->display(), (Pixmap)b->Map(), (Pixmap)info, gc,
        0, 0, width, height, 0, 0
    );
    XFreeGC(_world->display(), gc);
}

Pattern::Pattern (int p[patternHeight]) {
    info = (void*)MakeStipple(p);
}

Pattern::Pattern (int dither) {
    if (dither == 0xffff) {
        info = nil;
    } else {
        register int i, seed;
        int r[16];

        seed = dither;
        for (i = 0; i < 4; i++) {
            r[i] = (seed & 0xf000) >> 12;
            r[i] |= r[i] << 4;
            r[i] |= r[i] << 8;
            seed <<= 4;
            r[i+4] = r[i];
            r[i+8] = r[i];
            r[i+12] = r[i];
        }
        info = (void*)MakeStipple(r);
    }
}

Pattern::~Pattern () {
    if (LastRef()) {
        if (info != nil) {
            XFreePixmap(_world->display(), (Pixmap)info);
        }
    }
}

/*
 * class Canvas
 */

class CanvasRep {
    friend class Canvas;

    /* nothing needed for X11 */
    int unused;
};

Canvas::Canvas (void* c) {
    id = c;
    width = 0;
    height = 0;
    status = CanvasUnmapped;
}

Canvas::Canvas (int w, int h) {
    id = (void*)XCreatePixmap(
	_world->display(), _world->root(),
        w, h, DefaultDepth(_world->display(), _world->screen())
    );
    width = w;
    height = h;
    status = CanvasOffscreen;
}

Canvas::~Canvas () {
    if (id != nil) {
	if (status == CanvasOffscreen) {
	    XFreePixmap(_world->display(), (Pixmap)id);
	} else {
	    XDestroyWindow(_world->display(), (Window)id);
	    assocTable->Remove(id);
	}
	id = nil;
    }
}

void Canvas::WaitForCopy () {
    XEvent xe;
    Interactor* i;

    for (;;) {
	XWindowEvent(_world->display(), (Window)id, ExposureMask, &xe);
	switch (xe.type) {
	    case NoExpose:
		return;
	    case Expose:
		if (assocTable->Find(i, (void*)xe.xexpose.window)) {
		    i->Redraw(
			xe.xexpose.x,
			height - xe.xexpose.y - xe.xexpose.height,
			xe.xexpose.x + xe.xexpose.width - 1,
			height - 1 - xe.xexpose.y
		    );
		}
		break;
	    case GraphicsExpose:
		if (assocTable->Find(i, (void*)xe.xgraphicsexpose.drawable)) {
		    i->Redraw(
			xe.xgraphicsexpose.x,
			height - xe.xgraphicsexpose.y -
			    xe.xgraphicsexpose.height,
			xe.xgraphicsexpose.x + xe.xgraphicsexpose.width - 1,
			height - 1 - xe.xgraphicsexpose.y
		    );
		}
		if (xe.xgraphicsexpose.count == 0) {
		    return;
		}
		break;
	}
    }
}

void Canvas::SetBackground (Color* c) {
    if (status != CanvasOffscreen) {
	XSetWindowBackground(_world->display(), (Window)id, c->PixelValue());
    }
}

void Canvas::Clip (Coord, Coord, Coord, Coord) {
    /* Canvas clipping unimplemented for X11. */
}

void Canvas::NoClip () {
    /* Canvas clipping unimplemented for X11. */
}

void Canvas::ClipOn () {
    /* Canvas clipping unimplemented for X11. */
}

void Canvas::ClipOff () {
    /* Canvas clipping unimplemented for X11. */
}

boolean Canvas::IsClipped () {
    return false;
}

void Canvas::Map (Coord& x, Coord& y) {
    /* nothing to do */
}

/*
 * class Interactor
 */

void Interactor::Listen (Sensor* s) {
    unsigned int m;

    cursensor = s;
    if (canvas == nil) {
	/* can't set input interest without window */
	return;
    }
    m = (s == nil) ? 0 : s->mask;
    if (parent != nil) {
	/* Exposure on everyone but root window */
	m |= ExposureMask;
	if (parent->parent == nil) {
	    /* StructureNotify on top-level windows only */
	    m |= StructureNotifyMask;
	}
    }
    XSelectInput(_world->display(), (Window)canvas->id, m);
}

int Interactor::Fileno () {
    return ConnectionNumber(_world->display());
}

/*
 * Read a single event from the event stream.  If it is an input event,
 * is associated with an interactor, and the interactor is interested in it,
 * then return true.  In all other cases, return false.
 */

boolean Interactor::GetEvent (Event& e, boolean remove) {
    XEvent xe;
    Window w;
    Interactor* i;
    WorldView* wv;

    XNextEvent(_world->display(), &xe);
    switch (xe.type) {
	case MapNotify:
	    if (assocTable->Find(i, (void*)xe.xmap.window)) {
		i->SendActivate();
	    }
	    return false;
	case UnmapNotify:
	    if (assocTable->Find(i, (void*)xe.xunmap.window)) {
		i->SendDeactivate();
	    }
	    return false;
	case Expose:
	    if (assocTable->Find(i, (void*)xe.xexpose.window)) {
		i->SendRedraw(
		    xe.xexpose.x, xe.xexpose.y,
		    xe.xexpose.width, xe.xexpose.height, xe.xexpose.count
		);
	    }
	    return false;
	case ConfigureNotify:
	    if (assocTable->Find(i, (void*)xe.xconfigure.window)) {
		i->SendResize(
		    xe.xconfigure.x, xe.xconfigure.y,
		    xe.xconfigure.width, xe.xconfigure.height
		);
	    }
	    return false;
	case MotionNotify:
	    w = xe.xmotion.window;
	    break;
	case KeyPress:
	    w = xe.xkey.window;
	    break;
	case ButtonPress:
	case ButtonRelease:
	    w = xe.xbutton.window;
	    break;
	case FocusIn:
	case FocusOut:
	    w = xe.xfocus.window;
	    break;
	case EnterNotify:
	case LeaveNotify:
	    w = xe.xcrossing.window;
	    break;
	case MapRequest:
	    wv = _worldview;
	    if (wv != nil &&
		wv->canvas->id == (void*)xe.xmaprequest.parent
	    ) {
		wv->InsertRemote((void*)xe.xmaprequest.window);
	    }
	    return false;
	case ConfigureRequest:
	    wv = _worldview;
	    if (wv != nil &&
		wv->canvas->id == (void*)xe.xconfigurerequest.parent
	    ) {
		wv->ChangeRemote(
		    (void*)xe.xconfigurerequest.window,
		    xe.xconfigurerequest.x, ymax - xe.xconfigurerequest.y,
		    xe.xconfigurerequest.width, xe.xconfigurerequest.height
		);
	    }
	    return false;
	default:
	    /* ignore */
	    return false;
    }
    /* only input events should get here */
    if (!assocTable->Find(i, (void*)w) ||
	i->cursensor == nil || !i->cursensor->Interesting(&xe, e)
    ) {
	return false;
    }
    e.target = i;

    /*
     * Can't do the QueryPointer before checking to make sure the window
     * is valid because a motion event might have been sent just before
     * we destroyed the target window.
     */
    if (xe.type == MotionNotify) {
	Window dummy;
	int x, y, wx, wy;
	unsigned int state;

	XQueryPointer(
            _world->display(), w, &dummy, &dummy, &x, &y, &wx, &wy, &state
        );
	e.x = wx;
	e.y = wy;
    }
    e.y = i->ymax - e.y;
    if (!remove) {
	XPutBackEvent(_world->display(), &xe);
    }
    return true;
}

/*
 * Check to see if any input events of interest are pending.
 * This routine will return true even if the event is for another interactor.
 */

boolean Interactor::Check () {
    Event e;

    while (XPending(_world->display())) {
	if (GetEvent(e, false)) {
	    return true;
	}
    }
    return false;
}

int Interactor::CheckQueue () {
    return QLength(_world->display());
}

void Interactor::SendRedraw (Coord x, Coord y, int w, int h, int count) {
    if (count == 0) {
	Coord top = ymax - y;
	Redraw(x, top - h + 1, x + w - 1, top);
    } else {
	register int i;
	XEvent xe;
	Coord buf[40];
	Coord* left, * bottom, * right, * top;

	i = count + 1;
	if (i <= 10) {
	    left = buf;
	} else {
	    left = new Coord[4*i];
	}
	bottom = &left[i];
	right = &bottom[i];
	top = &right[i];

	left[0] = x;
	top[0] = ymax - y;
	right[0] = x + w - 1;
	bottom[0] = top[0] - h + 1;
	for (i = 1; i <= count; i++) {
	    /*
	     * This should be XNextEvent(_world->display(), xe), but that
	     * didn't work.  I think that implies an X server bug,
	     * but I have to live with what I've got for now.
	     */
	    XWindowEvent(
                _world->display(), (Window)canvas->id, ExposureMask, &xe
            );
	    left[i] = xe.xexpose.x;
	    top[i] = ymax - xe.xexpose.y;
	    right[i] = left[i] + xe.xexpose.width - 1;
	    bottom[i] = top[i] - xe.xexpose.height + 1;
	}
	RedrawList(count+1, left, bottom, right, top);
	if (left != buf) {
	    delete left;
	}
    }
}

void Interactor::SendResize (Coord x, Coord y, int w, int h) {
    left = x;
    bottom = parent->ymax - y - h + 1;
    if (canvas->width != w || canvas->height != h) {
        canvas->width = w;
        canvas->height = h;
        xmax = w - 1;
        ymax = h - 1;
        Resize();
    }
}

void Interactor::SendActivate () {
    canvas->status = CanvasMapped;
    Activate();
}

void Interactor::SendDeactivate () {
    canvas->status = CanvasUnmapped;
    Deactivate();
}

void Interactor::Poll (Event& e) {
    Window root, child;
    int x, y, root_x, root_y;
    unsigned int state;
    register unsigned int s;
    Interactor* i;

    XQueryPointer(
	_world->display(), (Window)canvas->id, &root, &child,
	&root_x, &root_y, &x, &y, &state
    );
    e.x = x;
    e.y = ymax - y;
    if (assocTable->Find(i, (void*)root)) {
	e.w = (World*)i;
    } else {
	e.w = nil;
    }
    e.wx = root_x;
    e.wy = root_y;
    s = state;
    e.control = (s&ControlMask) != 0;
    e.meta = (s&Mod1Mask) != 0;
    e.shift = (s&ShiftMask) != 0;
    e.shiftlock = (s&LockMask) != 0;
    e.leftmouse = (s&Button1Mask) != 0;
    e.middlemouse = (s&Button2Mask) != 0;
    e.rightmouse = (s&Button3Mask) != 0;
}

void Interactor::Flush () {
    XFlush(_world->display());
}

void Interactor::Sync () {
    XSync(_world->display(), 0);
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
    t = this;
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
	Interactor* w;
	Window child;

	w = (rel == nil) ? t->parent : rel;
	XTranslateCoordinates(
	    _world->display(), (Window)canvas->id, (Window)w->canvas->id,
	    x, ymax - y, &rx, &ry, &child
	);
	x = rx;
	y = w->ymax - ry;
    }
}

void Interactor::DoSetCursor (Cursor* c) {
    if (c == nil) {
	XUndefineCursor(_world->display(), (Window)canvas->id);
    } else {
	XDefineCursor(_world->display(), (Window)canvas->id, (XCursor)c->Id());
    }
    Flush();
}

void Interactor::DoSetName (const char* s) {
    XStoreName(_world->display(), (Window)canvas->id, s);
}

static void DefineWindowGroupHint (
    Interactor* leader, Canvas* leadercanvas, XWMHints& wmhints
) {
    if (leader != nil && leadercanvas != nil) {
	wmhints.flags |= WindowGroupHint;
	wmhints.window_group = (Window)leadercanvas->Id();
    } else {
	wmhints.flags &= ~WindowGroupHint;
	wmhints.window_group = None;
    }
}

void Interactor::DoSetGroupLeader (Interactor* leader) {
    XWMHints* wmhints = XGetWMHints(_world->display(), (Window)canvas->id);
    Canvas* leadercanvas = leader ? leader->canvas : nil;
    DefineWindowGroupHint(leader, leadercanvas, *wmhints);
    XSetWMHints(_world->display(), (Window)canvas->id, wmhints);
    delete wmhints;
}

void Interactor::DoSetTransientFor (Interactor* owner) {
    if (owner != nil && owner->canvas != nil) {
	Window w = (Window)canvas->id;
	XSetTransientForHint(_world->display(), w, (Window)owner->canvas->id);
    }
    /* else clear the hint, but how? */
}

void Interactor::DoSetIconName (const char* name) {
    XSetIconName(_world->display(), (Window)canvas->id, name);
}

static void DefineIconPixmapHint (Bitmap* bitmap, XWMHints& wmhints) {
    if (bitmap != nil && bitmap->Map() != nil) {
	wmhints.flags |= IconPixmapHint;
	wmhints.icon_pixmap = (Pixmap)bitmap->Map();
    } else {
	wmhints.flags &= ~IconPixmapHint;
	wmhints.icon_pixmap = None;
    }
}

void Interactor::DoSetIconBitmap (Bitmap* bitmap) {
    XWMHints* wmhints = XGetWMHints(_world->display(), (Window)canvas->id);
    DefineIconPixmapHint(bitmap, *wmhints);
    XSetWMHints(_world->display(), (Window)canvas->id, wmhints);
    delete wmhints;
}

static void DefineIconMaskHint (Bitmap* mask, XWMHints& wmhints) {
    if (mask != nil && mask->Map() != nil) {
	wmhints.flags |= IconMaskHint;
	wmhints.icon_mask = (Pixmap)mask->Map();
    } else {
	wmhints.flags &= ~IconMaskHint;
	wmhints.icon_mask = None;
    }
}

void Interactor::DoSetIconMask (Bitmap* mask) {
    XWMHints* wmhints = XGetWMHints(_world->display(), (Window)canvas->id);
    DefineIconMaskHint(mask, *wmhints);
    XSetWMHints(_world->display(), (Window)canvas->id, wmhints);
    delete wmhints;
}

/*
 * iconcanvas is a reference to icon->canvas so we can read the
 * value InsertIcon sets into icon->canvas even though we can't
 * read icon->canvas directly (it's a protected member).
 */

static void PlaceIconInteractor (
    Interactor* i, Interactor* icon, Canvas*& iconcanvas
) {
    if (iconcanvas == nil) {
	const char* g = i->GetIconGeometry();
	if (g != nil) {
	    icon->SetGeometry(g);
	}
	i->GetWorld()->InsertIcon(icon);
    }
}

static void DefineIconWindowHint (
    Interactor* i, Interactor* icon, Canvas*& iconcanvas, XWMHints& wmhints
) {
    if (icon != nil) {
	PlaceIconInteractor(i, icon, iconcanvas);
	wmhints.flags |= IconWindowHint;
	wmhints.icon_window = (Window)iconcanvas->Id();
    } else {
	wmhints.flags &= ~IconWindowHint;
	wmhints.icon_window = None;
    }
}

void Interactor::DoSetIconInteractor (Interactor* icon) {
    XWMHints* wmhints = XGetWMHints(_world->display(), (Window)canvas->id);
    Canvas* dummycanvas = nil;
    Canvas*& iconcanvas = icon ? icon->canvas : dummycanvas;
    DefineIconWindowHint(this, icon, iconcanvas, *wmhints);
    XSetWMHints(_world->display(), (Window)canvas->id, wmhints);
    delete wmhints;
}

static void DefineIconPositionHint (
    Interactor* i, const char* g, XWMHints& wmhints
) {
    wmhints.flags &= ~IconPositionHint;
    if (g != nil) {
	Coord x = 0;
	Coord y = 0;
	int w = i->GetShape()->width;
	int h = i->GetShape()->height;
	Bitmap* b = i->GetIconBitmap();
	if (b != nil) {
	    w = b->Width();
	    h = b->Height();
	}
	Interactor* icon = i->GetIconInteractor();
	if (icon != nil) {
	    w = icon->GetShape()->width;
	    h = icon->GetShape()->height;
	}

	World* world = i->GetWorld();
	unsigned r = world->ParseGeometry(g, x, y, w, h);
	if ((r & GeomXNegative) != 0) {
	    x = world->Width() + x - w;
	}
	if ((r & GeomYNegative) != 0) {
	    y = world->Height() + y - h;
	}
	if ((r & (GeomXValue|GeomYValue)) != 0) {
	    wmhints.flags |= IconPositionHint;
	    wmhints.icon_x = x;
	    wmhints.icon_y = y;
	}
    }
}

void Interactor::DoSetIconGeometry (const char* g) {
    XWMHints* wmhints = XGetWMHints(_world->display(), (Window)canvas->id);
    DefineIconPositionHint(this, g, *wmhints);
    XSetWMHints(_world->display(), (Window)canvas->id, wmhints);
    delete wmhints;
}

void Interactor::Iconify () {
    static Atom XA_WM_CHANGE_STATE = None; /* remove when defined in Xatom.h */
    Bool propagate = False;
    long eventmask = (SubstructureRedirectMask|SubstructureNotifyMask);
    XEvent xe;

    if (XA_WM_CHANGE_STATE == None) {
	XA_WM_CHANGE_STATE = XInternAtom(
            _world->display(), "WM_CHANGE_STATE", False
        );
    }

    xe.type = ClientMessage;
    xe.xclient.type = ClientMessage;
    xe.xclient.display = _world->display();
    xe.xclient.window = (Window)canvas->id;
    xe.xclient.message_type = XA_WM_CHANGE_STATE;
    xe.xclient.format = 32;
    xe.xclient.data.l[0] = IconicState;

    XSendEvent(_world->display(), _world->root(), propagate, eventmask, &xe);
}

void Interactor::DeIconify () {
    if (canvas != nil) {
	XMapWindow(_world->display(), (Window)canvas->id);
	canvas->status = CanvasMapped;
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

static void DoXError (Display* errdisplay, XErrorEvent* e) {
    register ReqErr* r = errhandler;
    if (r != nil) {
	r->msgid = e->serial;
	r->code = e->error_code;
	r->request = e->request_code;
	r->detail = e->minor_code;
	r->id = (void*)e->resourceid;
	XGetErrorText(errdisplay, r->code, r->message, sizeof(r->message));
	r->Error();
    }
}

ReqErr* ReqErr::Install () {
    if (errhandler == nil) {
	XSetErrorHandler(DoXError);
    }
    ReqErr* r = errhandler;
    errhandler = this;
    return r;
}

/*
 * class Scene
 */

/*
 * Adjust window dimensions to avoid 0 width or height error.
 */

static void DefaultShape (int w, int h, int& nw, int& nh) {
    nw = (w == 0) ? round(2*inch) : w;
    nh = (h == 0) ? round(2*inch) : h;
}

/*
 * Place an interactor according to user preferences.
 */

void Scene::UserPlace (Interactor* i, int w, int h) {
    int width, height;
    DefaultShape(w, h, width, height);
    MakeWindow(i, 0, 0, width, height);
    SetWindowProperties(i, 0, 0, width, height, false);
    if (i->GetInteractorType() == IconInteractor) {
	Assign(i, 0, 0, width, height);
    } else {
	DoMap(i, width, height);
    }
}

/*
 * Place an interactor at a particular position.
 */

void Scene::Place (
    Interactor* i, Coord l, Coord b, Coord r, Coord t, boolean map
) {
    Coord newtop = ymax - t;
    int width, height;
    DefaultShape(r - l + 1, t - b + 1, width, height);
    MakeWindow(i, l, newtop, width, height);
    SetWindowProperties(i, l, newtop, width, height, true);
    Assign(i, l, b, width, height);
    if (map && i->GetInteractorType() != IconInteractor) {
	Map(i);
    }
}

/*
 * Pick the right set of window attributes for the interactor's window.
 * For ICCCM compatibility, only popup windows use override_redirect. As
 * a convenience, popup and transient windows automatically use save_under.
 */

static unsigned int PickWindowAttributes (
    Interactor* i, XSetWindowAttributes& a
) {
    register unsigned int m = 0;

    if (i->GetInteractorType() != InteriorInteractor) {
	m |= CWDontPropagate;
	a.do_not_propagate_mask = (
	    KeyPressMask | KeyReleaseMask |
	    ButtonPressMask | ButtonReleaseMask | PointerMotionMask
	);
    }

    Cursor* c = i->GetCursor();
    if (c != nil) {
	m |= CWCursor;
	a.cursor = (XCursor)c->Id();
    } else if (i->GetInteractorType() == InteriorInteractor) {
	m |= CWCursor;
	a.cursor = None;
    } else {
	m |= CWCursor;
	a.cursor = (XCursor)defaultCursor->Id();
    }

    switch (i->GetCanvasType()) {
	case CanvasSaveUnder:
	    m |= CWSaveUnder;
	    a.save_under = True;
	    break;
	case CanvasSaveContents:
	    m |= CWBackingStore;
	    a.backing_store = WhenMapped;
	    break;
	case CanvasSaveBoth:
	    m |= CWSaveUnder;
	    a.save_under = True;
	    m |= CWBackingStore;
	    a.backing_store = WhenMapped;
	    break;
    }

    switch (i->GetInteractorType()) {
	case InteriorInteractor:
	    m |= CWBackPixmap;
	    a.background_pixmap = ParentRelative;
	    m |= CWWinGravity;
	    a.win_gravity = UnmapGravity;
	    break;
	case PopupInteractor:
	    m |= CWOverrideRedirect;
	    a.override_redirect = True;
	    m |= CWSaveUnder;
	    a.save_under = True;
	    break;
	case TransientInteractor:
	    m |= CWSaveUnder;
	    a.save_under = True;
	    break;
    }    

    return m;
}

/*
 * Pick the right class for the Interactor's window.
 */

static int PickWindowClass (Interactor* i) {
    int wclass = InputOutput;
    if (i->GetCanvasType() == CanvasInputOnly) {
	wclass = InputOnly;
    }
    return wclass;
}

/*
 * Pick the right set of changes to the geometry of the interactor's window.
 */

static unsigned int PickWindowChanges (
    Canvas* canvas, Coord x, Coord y, int width, int height, XWindowChanges& c
) {
    register unsigned int m = 0;

    m |= CWX;
    c.x = x;
    m |= CWY;
    c.y = y;

    if (canvas->Width() != width) {
	m |= CWWidth;
	c.width = width;
    }
    if (canvas->Height() != height) {
	m |= CWHeight;
	c.height = height;
    }

    return m;
}

/*
 * Create a window for an interactor.  If a window already exists,
 * update its geometry, attributes, and event mask.
 */

void Scene::MakeWindow (
    Interactor* i, Coord x, Coord y, int width, int height
) {
    XSetWindowAttributes a;
    InteractorType t = i->GetInteractorType();
    if (parent == nil) {
	if (t == InteriorInteractor) {
	    /* backward compatibility for world->Insert(interactor) */
	    i->SetInteractorType(ToplevelInteractor);
	}
    } else if (t != InteriorInteractor) {
	/* parent != nil, should be interior now */
	i->SetInteractorType(InteriorInteractor);
    }
    if (i->canvas == nil) {
	unsigned int amask = PickWindowAttributes(i, a);
	Window w = XCreateWindow(
	    _world->display(), (Window)canvas->id, x, y, width, height, 0, 
	    /* CopyFromParent */ 0, PickWindowClass(i), CopyFromParent,
	    amask, &a
	);
	i->canvas = new Canvas((void*)w);
	assocTable->Insert((void*)w, i);
    } else {
	XWindowChanges c;
	unsigned int cmask = PickWindowChanges(
	    i->canvas, x, y, width, height, c
	);
	XConfigureWindow(_world->display(), (Window)i->canvas->id, cmask, &c);
    }
    i->Listen(i->cursensor == nil ? i->input : i->cursensor);
}

/*
 * ICCCM window managers will not read x, y, width, and height but they
 * will pay attention to who (user or pgm) specified the values.
 */

static int Squeeze (register int a, int lower, int upper) {
    if (a < lower) {
	return lower;
    }
    if (a > upper) {
	return upper;
    }
    return a;
}

static void DefineSizeHints (
    Interactor* i, Coord x, Coord y, int width, int height, boolean placed,
    XSizeHints& sizehints
) {
    sizehints.flags = 0;

    const int MINSIZE = 1;
    const int BIGSIZE = 32123;
    Shape* s = i->GetShape();

    if (placed) {
	sizehints.flags |= USPosition | USSize;
    } else {
	sizehints.flags |= PSize;
    }
    sizehints.x = x;
    sizehints.y = y;
    sizehints.width = width;
    sizehints.height = height;

    sizehints.flags |= PMinSize;
    sizehints.min_width = Squeeze(s->width - s->hshrink, MINSIZE, BIGSIZE);
    sizehints.min_height = Squeeze(s->height - s->vshrink, MINSIZE, BIGSIZE);

    sizehints.flags |= PMaxSize;
    sizehints.max_width = Squeeze(s->width + s->hstretch, MINSIZE, BIGSIZE);
    sizehints.max_height = Squeeze(s->height + s->vstretch, MINSIZE, BIGSIZE);

    sizehints.flags |= PResizeInc;
    sizehints.width_inc = min(s->hunits, 1);
    sizehints.height_inc = min(s->vunits, 1);

    if (s->aspect == 1) {
	sizehints.flags |= PAspect;
	sizehints.min_aspect.x = 1;
	sizehints.min_aspect.y = 1;
	sizehints.max_aspect.x = 1;
	sizehints.max_aspect.y = 1;
    }
}

static void DefineWMHints(
    Interactor* i, Interactor* icon, Canvas*& iconcanvas,
    Interactor* leader, Canvas* leadercanvas, XWMHints& wmhints
) {
    wmhints.flags = 0;

    wmhints.flags |= InputHint;
    wmhints.input = True;

    wmhints.flags |= StateHint;
    wmhints.initial_state = i->GetStartIconic() ? IconicState : NormalState;

    DefineIconPixmapHint(i->GetIconBitmap(), wmhints);
    DefineIconWindowHint(i, icon, iconcanvas, wmhints);

    const char* g = i->GetIconGeometry();
    if (g == nil && icon != nil) {
	g = icon->GetGeometry();
    }
    DefineIconPositionHint(i, g, wmhints);

    DefineIconMaskHint(i->GetIconMask(), wmhints);
    DefineWindowGroupHint(leader, leadercanvas, wmhints);
}

static void DefineClassHint (Interactor* i, XClassHint& classhint) {
    classhint.res_name = (char*)i->GetInstance();
    if (classhint.res_name == nil) {
	classhint.res_name = (char*)i->GetWorld()->GetInstance();
	if (classhint.res_name == nil) {
	    classhint.res_name = "unnamed";
	}
    }

    classhint.res_class = (char*)i->GetClassName();
    if (classhint.res_class == nil) {
	classhint.res_class = (char*)i->GetWorld()->GetClassName();
	if (classhint.res_class == nil) {
	    classhint.res_class = "Unnamed";
	}
    }
}

/*
 * Set toplevel windows' properties for ICCCM compatibility.
 */

void Scene::SetWindowProperties (
    Interactor* i, Coord x, Coord y, int width, int height, boolean placed
) {
    InteractorType itype = i->GetInteractorType();
    if (itype == InteriorInteractor || itype == PopupInteractor) {
	return;
    }

    /* WM_NORMAL_HINTS */

    XSizeHints sizehints;
    DefineSizeHints(i, x, y, width, height, placed, sizehints);

    /* WM_HINTS */

    Interactor* icon = i->GetIconInteractor();
    Canvas* dummycanvas = nil;
    Canvas*& iconcanvas = icon ? icon->canvas : dummycanvas;
    Interactor* leader = i->GetGroupLeader();
    Interactor* owner = i->GetTransientFor();
    if (leader == nil || leader->canvas == nil) {
	leader = owner;
    }
    Canvas* leadercanvas = leader ? leader->canvas : nil;
    XWMHints wmhints;
    DefineWMHints(i, icon, iconcanvas, leader, leadercanvas, wmhints);

    /* WM_CLASS */
    XClassHint classhint;
    DefineClassHint(i, classhint);

    /* WM_ICON_NAME */
    const char* icon_name = i->GetIconName();
    if (icon_name == nil) {
	icon_name = classhint.res_name;
    }

    /* WM_NAME */
    const char* title = i->GetName();
    if (title == nil) {
	title = classhint.res_name;
    }

    /* WM_PROTOCOLS */
    /* we do not participate in any protocols */

    /* WM_COLORMAP_WINDOWS */
    /* we do not manipulate colormaps */

    Window w = (Window)i->canvas->id;
    WorldRep* rep = i->GetWorld()->Rep();
    char* clienthost = rep->hostname();
    switch (itype) {
	case ApplicationInteractor:
	    XSetCommand(rep->display(), w, saved_argv, saved_argc);
	    /* fall through */
	case ToplevelInteractor:
	    XSetIconName(rep->display(), w, icon_name);
	    /* fall through */
	case TransientInteractor:
	case IconInteractor:
	    XStoreName(rep->display(), w, title);
	    XChangeProperty(
		rep->display(), w, XA_WM_CLIENT_MACHINE, XA_STRING, 8,
		PropModeReplace, (unsigned char*)clienthost, strlen(clienthost)
	    );
	    XSetNormalHints(rep->display(), w, &sizehints);
	    XSetWMHints(rep->display(), w, &wmhints);
	    XSetClassHint(rep->display(), w, &classhint);
	    if (owner != nil && owner->canvas != nil) {
		XSetTransientForHint(
		    rep->display(), w, (Window)owner->canvas->id
		);
	    }
	    break;
    }
}

/*
 * Map an interactor and wait for confirmation of its position.
 * The interactor is either being placed by the user or inserted
 * in the world (in which case a window manager might choose to allocate
 * it a different size and placement).
 */

struct EventInfo {
    Window w;
    Interactor* i;
};

static Bool MapOrRedraw (Display*, register XEvent* xe, char* w) {
    EventInfo* x = (EventInfo*)w;
    return (
	/* target window is mapped */
	(xe->type == MapNotify && xe->xmap.window == x->w) ||

	/* target window is configued */
	(xe->type == ConfigureNotify && xe->xconfigure.window == x->w) ||

	/* a window other than the target is exposed */
	(xe->type == Expose && xe->xexpose.window != x->w &&
	    assocTable->Find(x->i, (void*)xe->xexpose.window)
	)
    );
}

void Scene::DoMap (Interactor* i, int w, int h) {
    EventInfo info;
    XEvent xe;
    boolean assigned = false;
    Display* d = i->GetWorld()->Rep()->display();
    info.w = (Window)i->canvas->id;
    XMapRaised(d, info.w);
    for (;;) {
	XIfEvent(d, &xe, MapOrRedraw, (char*)&info);
	if (xe.type == MapNotify) {
	    if (!assigned) {
		Assign(i, 0, 0, w, h);
	    }
	    i->SendActivate();
	    break;
	} else if (xe.type == ConfigureNotify) {
	    i->SendResize(
		xe.xconfigure.x, xe.xconfigure.y,
		xe.xconfigure.width, xe.xconfigure.height
	    );
	    assigned = true;
	} else if (xe.type == Expose) {
	    info.i->SendRedraw(
		xe.xexpose.x, xe.xexpose.y,
		xe.xexpose.width, xe.xexpose.height, xe.xexpose.count
	    );
	}
    }
    i->canvas->status = CanvasMapped;
}

void Scene::Map (Interactor* i, boolean raised) {
    Window w = (Window)i->canvas->id;
    Display* d = i->GetWorld()->Rep()->display();
    if (raised) {
	XMapRaised(d, w);
    } else {
	XMapWindow(d, w);
    }
    i->canvas->status = CanvasMapped;
}

/*
 * We must send a synthetic UnMapNotify for ICCCM compatibility
 */

void Scene::Unmap (Interactor* i) {
    WorldRep* rep = GetWorld()->Rep();
    Window w = (Window)i->canvas->id;
    XUnmapWindow(rep->display(), w);
    i->canvas->status = CanvasUnmapped;
    if (parent == nil && i->GetInteractorType() != PopupInteractor) {
	Bool propagate = False;
	long eventmask = (SubstructureRedirectMask|SubstructureNotifyMask);
	XEvent xe;

	xe.type = UnmapNotify;
	xe.xunmap.type = UnmapNotify;
	xe.xunmap.display = rep->display();
	xe.xunmap.event = rep->root();
	xe.xunmap.window = w;
	xe.xunmap.from_configure = False;

	XSendEvent(rep->display(), rep->root(), propagate, eventmask, &xe);
    }
}

void Scene::Raise (Interactor* i) {
    DoRaise(i);
    XRaiseWindow(i->GetWorld()->Rep()->display(), (Window)i->canvas->id);
}

void Scene::Lower (Interactor* i) {
    DoLower(i);
    XLowerWindow(i->GetWorld()->Rep()->display(), (Window)i->canvas->id);
}

void Scene::Move (Interactor* i, Coord x, Coord y, Alignment a) {
    Coord ax = x, ay = y;
    DoAlign(i, a, ax, ay);
    DoMove(i, ax, ay);
    XMoveWindow(
	i->GetWorld()->Rep()->display(), (Window)i->canvas->id,
	ax, ymax - ay - i->ymax
    );
}

/*
 * class Sensor
 */

int motionmask = PointerMotionMask;
int keymask = KeyPressMask;
int entermask = EnterWindowMask;
int leavemask = LeaveWindowMask;
int focusmask = FocusChangeMask;
int substructmask = SubstructureRedirectMask;
int upmask = ButtonPressMask|ButtonReleaseMask|OwnerGrabButtonMask;
int downmask = ButtonPressMask|ButtonReleaseMask|OwnerGrabButtonMask;
int initmask = PointerMotionHintMask;

static Sensor* grabber;

boolean Sensor::Interesting (void* raw, Event& e) {
    boolean b;
    register XEvent* x = (XEvent*)raw;
    Interactor* i;

    b = false;
    switch (x->type) {
	case MotionNotify:
	    e.eventType = MotionEvent;
	    e.timestamp = x->xmotion.time;
	    e.x = x->xmotion.x;
	    e.y = x->xmotion.y;
	    if (assocTable->Find(i, (void*)x->xmotion.root)) {
		e.w = (World*)i;
	    } else {
		e.w = nil;
	    }
	    e.wx = x->xmotion.x_root;
	    e.wy = x->xmotion.y_root;
	    b = true;
	    break;
	case KeyPress:
	    e.eventType = KeyEvent;
	    e.GetButtonInfo(x);
	    b = ButtonIsSet(down, e.button);
	    break;
	case ButtonPress:
	    e.eventType = DownEvent;
	    e.GetButtonInfo(x);
	    b = ButtonIsSet(down, e.button);
	    if (b && ButtonIsSet(up, e.button)) {
		grabber = this;
	    } else {
		grabber = nil;
	    }
	    break;
	case ButtonRelease:
	    e.eventType = UpEvent;
	    e.GetButtonInfo(x);
	    b = ButtonIsSet(up, e.button) || (grabber != nil);
	    break;
	case FocusIn:
	    e.eventType = FocusInEvent;
	    b = true;
	    break;
	case FocusOut:
	    e.eventType = FocusOutEvent;
	    b = true;
	    break;
	case EnterNotify:
	    if (x->xcrossing.detail != NotifyInferior) {
		e.eventType = EnterEvent;
		e.timestamp = x->xcrossing.time;
		b = true;
	    }
	    break;
	case LeaveNotify:
	    if (x->xcrossing.detail != NotifyInferior) {
		e.eventType = LeaveEvent;
		e.timestamp = x->xcrossing.time;
		b = true;
	    }
	    break;
	default:
	    /* ignore */;
    }
    return b;
}

void Event::GetButtonInfo (void* xe) {
    register XKeyEvent* k = &(((XEvent*)xe)->xkey);
    Interactor* i;
    register int state;
    char buf[4096];

    timestamp = k->time;
    x = k->x;
    y = k->y;
    if (assocTable->Find(i, (void*)k->root)) {
        w = (World*)i;
    } else {
	w = nil;
    }
    wx = k->x_root;
    wy = k->y_root;
    state = k->state;
    shift = (state & ShiftMask) != 0;
    control = (state & ControlMask) != 0;
    meta = (state & Mod1Mask) != 0;
    shiftlock = (state & LockMask) != 0;
    leftmouse = (state & Button1Mask) != 0;
    middlemouse = (state & Button2Mask) != 0;
    rightmouse = (state & Button3Mask) != 0;
    button = k->keycode;
    if (k->type == KeyPress) {
	len = XLookupString(k, buf, sizeof(buf), nil, nil);
	if (len > 0) {
	    if (len < sizeof(int)) {
		keystring = keydata;
	    } else {
		keystring = new char[len+1];
	    }
	    strncpy(keystring, buf, len);
	    keystring[len] = '\0';
	}
    } else {
	button -= 1;
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

#if defined(hpux)
#include <sys/utsname.h>
#endif

char* WorldRep::gethostname () {
#if defined(hpux)
    struct utsname name;
    uname(&name);
    strncpy(_host, name.nodename, sizeof(_host));
#else
    ::gethostname(_host, sizeof(_host));
#endif
    return _host;
}

void World::Init (const char* device) {
    rep = new WorldRep;
    rep->_display = XOpenDisplay(device);
    if (rep->display() == nil) {
	fprintf(
	    stderr, "fatal error: can't open display '%s'\n",
	    XDisplayName(device)
	);
	exit(1);
    }
    rep->_screen = DefaultScreen(rep->display());
    rep->_root = RootWindow(rep->display(), rep->screen());
    rep->_visual = DefaultVisual(rep->display(), rep->screen());
    rep->_cmap = DefaultColormap(rep->display(), rep->screen());
    rep->_xor = 1;
    rep->_txfonts = TxFontsDefault;
    rep->_tximages = TxImagesDefault;
    rep->_dash = DashDefault;
    SetCurrent();
    if (assocTable == nil) {
	assocTable = new InteractorTable(2048);
    }
    assocTable->Insert((void*)rep->root(), this);
    canvas = new Canvas((void*)rep->root());
    canvas->width = DisplayWidth(rep->display(), rep->screen());
    canvas->height = DisplayHeight(rep->display(), rep->screen());
    canvas->status = CanvasMapped;
    xmax = canvas->width - 1;
    ymax = canvas->height - 1;
    double pixmm = (
	double(canvas->width) /
	double(DisplayWidthMM(rep->display(), rep->screen()))
    );
    cm = round(10*pixmm);
    inch = round(25.4*pixmm);
    inches = inch;
    point = inch/72.27;
    points = point;

    black = new Color("black");
    white = new Color("white");
    stdfont = new Font("fixed");
    stdpaint = new Painter();
}

static inline unsigned int MSB (unsigned int i) {
    return (i ^ (i>>1)) & i;
}

void World::FinishInit () {
    const char* xorpixel = GetAttribute("RubberbandPixelHint");
    char buffer[256];
    if (xorpixel != nil) {
        sprintf(buffer, "RubberbandPixel:%s", xorpixel);
        SetHint(buffer);
    } else if (rep->visual()->CC_class != DirectColor) {
        rep->_xor = black->PixelValue() ^ white->PixelValue();
    } else {
        rep->_xor = (
            MSB(rep->visual()->red_mask) |
            MSB(rep->visual()->green_mask) |
            MSB(rep->visual()->blue_mask)
        );
    }
    const char* txfonts = GetAttribute("TransformFontsHint");
    if (txfonts != nil) {
        sprintf(buffer, "TransformFonts:%s", txfonts);
        SetHint(buffer);
    }
    const char* tximages = GetAttribute("TransformImagesHint");
    if (tximages != nil) {
        sprintf(buffer, "TransformImages:%s", tximages);
        SetHint(buffer);
    }
    const char* dash = GetAttribute("DashHint");
    if (dash != nil) {
        sprintf(buffer, "Dash:%s", dash);
        SetHint(buffer);
    }
    SetCurrent();

    RootConfig();
    InitSensors();
    InitCursors(output->GetFgColor(), output->GetBgColor());
}

World::~World () {
    assocTable->Remove((void*)rep->root());
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
    return rep->display()->xdefaults;
}

void World::FinishInsert (Interactor*) {
    /* nothing else to do */
}

void World::DoChange (Interactor* i) {
    Canvas* c = i->canvas;
    if (c != nil && c->status == CanvasMapped) {
	Shape* s = i->GetShape();
	if (c->width != s->width || c->height != s->height) {
	    XResizeWindow(rep->display(), (Window)c->id, s->width, s->height);
	} else {
	    i->Resize();
	}
    }
}

void World::DoRemove (Interactor*) {
    XFlush(rep->display());
}

int World::Fileno () {
    return ConnectionNumber(rep->display());
}

void World::SetCurrent () {
    _world = rep;
}

void World::SetRoot (void* r) {
    rep->_root = (Window)r;
}

void World::SetScreen (int n) {
    rep->_screen = n;
}

int World::NPlanes () {
    return DisplayPlanes(rep->display(), rep->screen());
}

int World::NButtons () {
    return 3;
}

int World::PixelSize () {
    return BitmapPad(rep->display());
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

void World::SetHint (const char* hint) {
    char value[256];
    if (sscanf((char*)hint, "RubberbandPixel:%s", value) == 1) {
        sscanf(value, "%d", &rep->_xor);
    } else if (sscanf((char*)hint, "TransformFonts:%s", value) == 1) {
        if (strcmp(value, "off") == 0 || strcmp(value, "Off") == 0) {
            rep->_txfonts = TxFontsOff;
        } else if (strcmp(value, "on") == 0 || strcmp(value, "On") == 0) {
            rep->_txfonts = TxFontsOn;
        } else if (strcmp(value, "cache")==0 || strcmp(value, "Cache")==0) {
            rep->_txfonts = TxFontsCache;
        } else {
            rep->_txfonts = TxFontsDefault;
        }
        if (rep->_txfonts != TxFontsCache) {
            FlushCharBitmaps();
        }
    } else if (sscanf((char*)hint, "TransformImages:%s", value) == 1) {
        if (strcmp(value, "auto") == 0 || strcmp(value, "Auto") == 0) {
            rep->_tximages = TxImagesAuto;
        } else if (
            strcmp(value, "destination") == 0
            || strcmp(value, "Destination") == 0
        ) {
            rep->_tximages = TxImagesDest;
        } else if (strcmp(value, "source")==0 || strcmp(value, "Source")==0) {
            rep->_tximages = TxImagesSource;
        } else {
            rep->_tximages = TxImagesDefault;
        }
    } else if (sscanf((char*)hint, "Dash:%s", value) == 1) {
        if (strcmp(value, "none") == 0 || strcmp(value, "None") == 0) {
            rep->_dash = DashNone;
        } else if (strcmp(value, "thin") == 0 || strcmp(value, "Thin") == 0) {
            rep->_dash = DashThin;
        } else if (strcmp(value, "all") == 0 || strcmp(value, "All") == 0) {
            rep->_dash = DashAll;
        } else {
            rep->_dash = DashDefault;
        }
    }
}

void World::RingBell (int v) {
    if (v > 100) {
	XBell(rep->display(), 100);
    } else if (v >= 0) {
	XBell(rep->display(), v);
    }
}

void World::SetKeyClick (int v) {
    XKeyboardControl k;

    k.key_click_percent = v;
    XChangeKeyboardControl(rep->display(), KBKeyClickPercent, &k);
}

void World::SetAutoRepeat (boolean b) {
    if (b) {
	XAutoRepeatOn(rep->display());
    } else {
	XAutoRepeatOff(rep->display());
    }
}

void World::SetFeedback (int t, int s) {
    XChangePointerControl(rep->display(), True, True, s, 1, t);
}

/*
 * class WorldView
 */

void WorldView::Init (World* world) {
    WorldRep* rep = world->Rep();
    Window w = RootWindow(rep->display(), rep->screen());
    canvas = new Canvas((void*)w);
    canvas->width = DisplayWidth(rep->display(), rep->screen());
    canvas->height = DisplayHeight(rep->display(), rep->screen());
    canvas->status = CanvasMapped;
    xmax = canvas->width - 1;
    ymax = canvas->height - 1;
    output->SetOverwrite(true);
}

static const int bmask =
    ButtonPressMask|ButtonReleaseMask|OwnerGrabButtonMask|
    PointerMotionMask|PointerMotionHintMask;

void WorldView::GrabMouse (Cursor* c) {
    while (
	XGrabPointer(
	    world->Rep()->display(), (Window)canvas->id, True, bmask,
	    GrabModeAsync, GrabModeAsync, None, (XCursor)c->Id(),
	    CurrentTime
	) != GrabSuccess
    ) {
	sleep(1);
    }
}

void WorldView::UngrabMouse () {
    XUngrabPointer(world->Rep()->display(), CurrentTime);
}

boolean WorldView::GrabButton (unsigned b, unsigned m, Cursor* c) {
    XGrabButton(
	world->Rep()->display(), b, m, (Window)canvas->id, True, bmask,
	GrabModeAsync, GrabModeAsync, None, (XCursor)c->Id()
    );
    return true;
}

void WorldView::UngrabButton (unsigned b, unsigned m) {
    XUngrabButton(world->Rep()->display(), b, m, (Window)canvas->id);
}

void WorldView::Lock () {
/*
 * Bad idea to grab the server
 */
}

void WorldView::Unlock () {
/*
 * See Lock()
 */
}

void WorldView::ClearInput () {
    XSync(world->Rep()->display(), 1);
}

void WorldView::MoveMouse (Coord x, Coord y) {
    XWarpPointer(
	world->Rep()->display(), (Window)canvas->id, (Window)canvas->id,
	0, 0, xmax, ymax, x, ymax - y
    );
}

void WorldView::Map (RemoteInteractor i) {
    XMapWindow(world->Rep()->display(), (Window)i);
}

void WorldView::MapRaised (RemoteInteractor i) {
    XMapRaised(world->Rep()->display(), (Window)i);
}

void WorldView::Unmap (RemoteInteractor i) {
    XUnmapWindow(world->Rep()->display(), (Window)i);
}

RemoteInteractor WorldView::Find (Coord x, Coord y) {
    Window w;
    Coord rx, ry;

    XTranslateCoordinates(
	world->Rep()->display(), (Window)canvas->id, (Window)canvas->id,
	x, ymax - y, &rx, &ry, &w
    );
    return (void*)w;
}

void WorldView::Move (RemoteInteractor i, Coord left, Coord top) {
    XMoveWindow(world->Rep()->display(), (Window)i, left, ymax - top);
}

void WorldView::Change (
    RemoteInteractor i, Coord left, Coord top, int w, int h
) {
    XMoveResizeWindow(
	world->Rep()->display(), (Window)i, left, ymax - top, w, h
    );
}

void WorldView::Raise (RemoteInteractor i) {
    XRaiseWindow(world->Rep()->display(), (Window)i);
}

void WorldView::Lower (RemoteInteractor i) {
    XLowerWindow(world->Rep()->display(), (Window)i);
}

void WorldView::Focus (RemoteInteractor i) {
    if (i != curfocus) {
	curfocus = i;
	XSetInputFocus(
	    world->Rep()->display(), i == nil ? PointerRoot : (Window)i,
	    RevertToPointerRoot, CurrentTime
	);
    }
}

void WorldView::GetList (RemoteInteractor*& ilist, int& n) {
    Window parent;

    XQueryTree(
	world->Rep()->display(), (Window)canvas->id, &parent, &parent,
	(Window**)&ilist, &n
    );
}

void WorldView::GetInfo (
    RemoteInteractor i, Coord& x1, Coord& y1, Coord& x2, Coord& y2
) {
    Window root;
    int x, y;
    unsigned int w, h, bw, d;

    XGetGeometry(
	world->Rep()->display(), (Window)i, &root, &x, &y, &w, &h, &bw, &d
    );
    x1 = x;
    y2 = ymax - y;
    x2 = x + w + 2*bw - 1;
    y1 = y2 - h - 2*bw + 1;
}

boolean WorldView::GetHints (
    RemoteInteractor i, Coord& x, Coord& y, Shape& s
) {
    XSizeHints sizehints;

    sizehints.flags = 0;
    XGetSizeHints(
	world->Rep()->display(), (Window)i, &sizehints, XA_WM_NORMAL_HINTS
    );
    if ((sizehints.flags & USSize) != 0) {
	s.width = sizehints.width;
	s.height = sizehints.height;
	s.hstretch = sizehints.max_width - sizehints.width;
	s.hshrink = sizehints.width - sizehints.min_width;
	s.vstretch = sizehints.max_height - sizehints.height;
	s.vshrink = sizehints.height - sizehints.min_height;
	s.hunits = sizehints.width_inc;
	s.vunits = sizehints.height_inc;
    } else {
	s.width = 0;
	s.height = 0;
    }
    if ((sizehints.flags & USPosition) != 0) {
	x = sizehints.x;
	y = ymax - sizehints.y;
	return true;
    }
    return false;
}

void WorldView::SetHints (RemoteInteractor i, Coord x, Coord y, Shape& s) {
    XSizeHints sizehints;

    sizehints.flags = (USPosition | USSize);
    sizehints.x = x;
    sizehints.y = ymax - y;
    sizehints.width = s.width;
    sizehints.height = s.height;
    XSetSizeHints(
	world->Rep()->display(), (Window)i, &sizehints, XA_WM_NORMAL_HINTS
    );
}

RemoteInteractor WorldView::GetIcon (RemoteInteractor i) {
    XWMHints* h;
    RemoteInteractor r;

    h = XGetWMHints(world->Rep()->display(), (Window)i);
    if (h == nil || (h->flags&IconWindowHint) == 0) {
	r = nil;
    } else {
	r = (void*)h->icon_window;
    }
    delete h;
    return r;
}

/* obsolete - window mgrs should set WM_STATE on clients' toplevel windows */

void WorldView::AssignIcon (RemoteInteractor i, RemoteInteractor icon) {
    XWMHints h;

    h.flags = IconWindowHint;
    h.icon_window = (Window)i;
    XSetWMHints(world->Rep()->display(), (Window)icon, &h);
    h.icon_window = (Window)icon;
    XSetWMHints(world->Rep()->display(), (Window)i, &h);
}

void WorldView::UnassignIcon (RemoteInteractor i) {
    XWMHints h;

    h.flags = IconWindowHint;
    h.icon_window = None;
    XSetWMHints(world->Rep()->display(), (Window)i, &h);
}

RemoteInteractor WorldView::TransientOwner (RemoteInteractor i) {
    Window w;

    return
	XGetTransientForHint(world->Rep()->display(), (Window)i, &w) ?
	    (void*)w : nil;
}

char* WorldView::GetName (RemoteInteractor i) {
    char* name;

    XFetchName(world->Rep()->display(), (Window)i, &name);
    return name;
}
