/*
 * Raster - rasterized image
 */

#include <InterViews/raster.h>
#include <InterViews/color.h>
#include <bstring.h>

Raster::Raster (Color** data, int width, int height) {
    rep = new RasterRep(width, height);
    raster = new Color*[width * height];
    if (data != nil) {
	bcopy(data, (char*)raster, width*height*sizeof(Color*));
    } else {
	bzero((char*)raster, width*height*sizeof(Color*));
    }
    for (int x = 0; x < width; ++x) {
        for (int y = 0; y < height; ++y) {
            int index = Index(x, y);
            Color* c = raster[index];
            if (c == nil) {
                c = white;
                raster[index] = c;
            }
            c->Reference();
            rep->PutPixel(x, y, c->PixelValue());
        }
    }
}

Raster::Raster (Canvas* c, Coord x0, Coord y0, int width, int height) {
    rep = new RasterRep(c, x0, y0, width, height);
    raster = nil;
}

Raster::Raster (Raster* r) {
    rep = new RasterRep(r->rep);
    int width = Width();
    int height = Height();
    if (r->raster != nil) {
        raster = new Color*[width * height];
        bcopy((char*)r->raster, (char*)raster, width*height*sizeof(Color*));
        for (int x = 0; x < width; ++x) {
            for (int y = 0; y < height; ++y) {
                int index = Index(x, y);
                Color* c = raster[index];
                if (c == nil) {
                    c = white;
                    raster[index] = c;
                }
                c->Reference();
                rep->PutPixel(x, y, c->PixelValue());
            }
        }
    } else {
        raster = nil;
    }
}

Raster::~Raster () {
    if (LastRef()) {
        if (raster != nil) {
            int width = Width();
            int height = Height();
            for (int x = 0; x < width; ++x) {
                for (int y = 0; y < height; ++y) {
                    int index = Index(x, y);
                    delete raster[index];
                }
            }
            delete raster;
        }
	delete rep;
    }
}

Color* Raster::Peek (int x, int y) {
    if (Contains(x, y)) {
        if (raster == nil) {
            int width = Width();
            int height = Height();
            raster = new Color*[width * height];
            bzero((char*)raster, width*height*sizeof(Color*));
        }
        int index = Index(x, y);
        Color* c = raster[index];
        if (c == nil) {
            c = new Color(rep->GetPixel(x, y));
            raster[index] = c;
        }
        return c;
    } else {
        return nil;
    }
}

void Raster::Poke (Color* c, int x, int y) {
    if (Contains(x, y)) {
        if (raster == nil) {
            int width = Width();
            int height = Height();
            raster = new Color*[width * height];
            bzero((char*)raster, width*height*sizeof(Color*));
        }
        int index = Index(x, y);
        delete raster[index];
        raster[index] = c;
        c->Reference();
        rep->PutPixel(x, y, c->PixelValue());
    }
}
