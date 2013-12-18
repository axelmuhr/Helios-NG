/*
 * Implementation of RasterRect, an object derived from Graphic.
 */

#include <InterViews/raster.h>
#include <InterViews/transformer.h>
#include <InterViews/Graphic/grclasses.h>
#include <InterViews/Graphic/rasterrect.h>

/*****************************************************************************/

class ColorMaker {
public:
    ColorMaker();
    ~ColorMaker();
    Color* MakeColor(ColorIntensity r, ColorIntensity g, ColorIntensity b);
private:
    int size;
    Color** colors;
    void Grow(int size);
};

ColorMaker::ColorMaker () {
    size = 0;
    colors = nil;
    Grow(256);
}

ColorMaker::~ColorMaker () {
    for (int i = 0; i < size; ++i) delete colors[i];
    delete colors;
}

void ColorMaker::Grow (int s) {
    Color** newcolors = new Color* [s];
    for (int i = 0; i < s; ++i) {
        if (i < size) {
            newcolors[i] = colors[i];
        } else {
            newcolors[i] = nil;
        }
    }
    delete colors;
    colors = newcolors;
    size = s;
}

Color* ColorMaker::MakeColor (
    ColorIntensity r, ColorIntensity g, ColorIntensity b
) {
    int hash = (r + g + b) % size;
    for (int i = 0; i < 256; ++i) {
        int index = (i + hash) % size;
        if (colors[index] != nil) {
            ColorIntensity rr, gg, bb;
            colors[index]->Intensities(rr, gg, bb);
            if (rr == r && gg == g && bb == b) {
                return colors[index];
            } else {
                i = (i+1) % size;
            }
        } else {
            colors[index] = new Color(r, g, b);
            return colors[index];
        }
    }
    Grow(size*2);
    return MakeColor(r, g, b);
}

/*****************************************************************************/

void RasterRect::draw (Canvas *c, Graphic* gs) {
    update(gs);
    pRasterRect(c, 0, 0, raster);
}

ClassId RasterRect::GetClassId () { return RASTERRECT; }

boolean RasterRect::IsA (ClassId id) { 
    return RASTERRECT == id || Graphic::IsA(id); 
}

void RasterRect::Init () {
    if (colorMaker == nil) {
        colorMaker = new ColorMaker;
    }
}

RasterRect::RasterRect () { 
    Init();
    raster = nil;
}

RasterRect::RasterRect (Raster* r, Graphic* gr) : (gr) {
    Init();
    raster = r;
}

Graphic* RasterRect::Copy () {
    return new RasterRect(new Raster(raster), this);
}

RasterRect::~RasterRect () {
    delete raster;
}

Raster* RasterRect::GetOriginal () { return raster; }

boolean RasterRect::readRaster (PFile* f, Raster*& raster) {
    int w, h;
    ColorIntensity r, g, b;
    boolean ok = f->Read(w) && f->Read(h);

    if (ok) {
        raster = new Raster(nil, w, h);

        for (int i = 0; i < w; ++i) {
            for (int j = 0; j < h && ok; ++j) {
                ok = f->Read(r) && f->Read(g) && f->Read(b);
                raster->Poke(colorMaker->MakeColor(r, g, b), i, j);
            }
        }
    }
    return ok;
}
            
boolean RasterRect::writeRaster (PFile* f, Raster* raster) {
    int w = raster->Width();
    int h = raster->Height();
    ColorIntensity r, g, b;
    boolean ok = f->Write(w) && f->Write(h);

    if (ok) {
        for (int i = 0; i < w; ++i) {
            for (int j = 0; j < h && ok; ++j) {
                raster->Peek(i, j)->Intensities(r, g, b);
                ok = f->Write(r) && f->Write(g) && f->Write(b);
            }
        }
    }
    return ok;
}

boolean RasterRect::read (PFile* f) {
    return Graphic::read(f) && readRaster(f, raster);
}

boolean RasterRect::write (PFile* f) {
    return Graphic::write(f) && writeRaster(f, raster);
}

void RasterRect::getExtent (
    float& x0, float& y0, float& cx, float& cy, float& tol, Graphic* gs
) {
    if (gs->GetTransformer() == nil) {
	x0 = y0 = 0;
	cx = raster->Width() / 2;
	cy = raster->Height() / 2;
    } else {
	transformRect(0,0,raster->Width(),raster->Height(),x0,y0,cx,cy,gs);
	cx = (cx + x0)/2;
	cy = (cy + y0)/2;
    }
    tol = 0;
}

boolean RasterRect::contains (PointObj& po, Graphic* gs) {
    PointObj pt (&po);
    invTransform(pt.x, pt.y, gs);
    BoxObj b (0, 0, raster->Width(), raster->Height());
    return b.Contains(pt);
}

boolean RasterRect::intersects (BoxObj& userb, Graphic* gs) {
    Transformer* t = gs->GetTransformer();
    Coord xmax = raster->Width();
    Coord ymax = raster->Height();
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
