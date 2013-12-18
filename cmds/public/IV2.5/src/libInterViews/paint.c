/*
 * Graphics implementation on top of X
 */

#include "ftable.h"
#include <InterViews/paint.h>
#include <InterViews/strtable.h>
#include <string.h>

Painter* stdpaint;

static void MakeSubString (char* tmp, const char* s, int n) {
    strncpy(tmp, s, n);
    tmp[n] = '\0';
}

/*
 * class Brush
 */

Brush* single;

Brush::Brush (int pat, int w) {
    int r[16];
    int count;
    width = w;
    unsigned int p = pat & 0xffff;
    
    if (p == 0xffff || p == 0) {
        count = 0;
    } else if (p == 0x5555 || p == 0xaaaa) {
        r[0] = 1;
        r[1] = 1;
        count = 2;
    } else if (p == 0xcccc || p == 0x3333) {
        r[0] = 2;
        r[1] = 2;
        count = 2;
    } else if (p == 0xf0f0 || p == 0x0f0f) {
        r[0] = 4;
        r[1] = 4;
        count = 2;
    } else if (p == 0xf000 || p == 0x000f) {
        r[0] = 4;
        r[1] = 12;
        count = 2;
    } else if (p == 0xff00 || p == 0x00ff) {
        r[0] = 8;
        r[1] = 8;
        count = 2;
    } else if (p == 0xfff0 || p == 0x0fff) {
        r[0] = 12;
        r[1] = 4;
        count = 2;
    } else {
        unsigned int m = 1<<15;
        int index = 0;
        while ((p & m) == 0) {
            m = m>>1;
        }
        while (m != 0) {
            int length;
            length = 0;
            while(m != 0 && (p & m) != 0) {
                ++length;
                m = m>>1;
            }
            if (length > 0) {
                r[index] = length;
                ++index;
            }
            length = 0;
            while (m != 0 && (p & m) == 0) {
                ++length;
                m = m>>1;
            }
            if (length > 0) {
                r[index] = length;
                ++index;
            }
        }
    }
    rep = new BrushRep(r, count, width);
}

Brush::Brush (int* p, int c, int w) {
    width = w;
    rep = new BrushRep(p, c, w);
}

Brush::~Brush () {
    if (LastRef()) {
        delete rep;
    }
}

/*
 * class Color
 */

Color* black;
Color* white;

Color::Color (ColorIntensity r, ColorIntensity g, ColorIntensity b) {
    red = r;
    green = g;
    blue = b;
    rep = new ColorRep(red, green, blue);
}

Color::Color (const char* name) {
    rep = new ColorRep(name, red, green, blue);
}

Color::Color (const char* name, int len) {
    char tmp[100];

    MakeSubString(tmp, name, len);
    rep = new ColorRep(tmp, red, green, blue);
}

Color::Color (int pixel) {
    rep = new ColorRep(pixel, red, green, blue);
}

boolean Color::Valid () {
    return rep->info != nil;
}

Color::~Color () {
    if (LastRef()) {
        delete rep;
    }
}

/*
 * class Font
 */

Font* stdfont;

static FontTable* fontTable;
static StringTable* fontnameTable;

Font::Font (const char* name) {
    if (!Lookup(name, strlen(name))) {
	GetFontByName(name);
    }
}

Font::Font (const char* name, int len) {
    if (!Lookup(name, len)) {
	if (name[len] == '\0') {
	    GetFontByName(name);
	} else {
	    char tmp[256];

	    MakeSubString(tmp, name, len);
	    GetFontByName(tmp);
	}
    }
}

boolean Font::Lookup (const char* name, int len) {
    StringId* s;

    if (fontTable == nil) {
	fontTable = new FontTable(32);
	fontnameTable = new StringTable(32);
    }
    s = fontnameTable->Id(name, len);
    if (fontTable->Find(rep, s)) {
	rep->Reference();
	return true;
    }
    rep = new FontRep;
    fontTable->Insert(s, rep);
    return false;
}

int Font::Height () {
    return rep->height;
}

int Font::Index (const char* s, int offset, boolean between) {
    return Index(s, strlen(s), offset, between);
}

boolean Font::Valid () {
    return rep->height != -1;
}

void* Font::Info () {
    return rep->info;
}

/*
 * class Pattern
 */

Pattern* solid;
Pattern* clear;
Pattern* lightgray;
Pattern* gray;
Pattern* darkgray;
