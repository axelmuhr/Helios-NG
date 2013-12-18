/*
 * Implementation of persistent paint objects used by Graphic.
 */

#include <InterViews/Graphic/grclasses.h>
#include <InterViews/Graphic/ppaint.h>
#include <bstring.h>
#include <string.h>

PColor* pblack;
PColor* pwhite;
PPattern* psolid;
PPattern* pclear;
PBrush* psingle;
PFont* pstdfont;

/*****************************************************************************/

boolean PColor::read (PFile* f) {
    int index, r, g, b;
    boolean ok = 
	Persistent::read(f) && 
	f->Read(index) && f->Read(r) && f->Read(g) && f->Read(b);
    
    if (ok) {
	value = new Color(r, g, b);
    }
    return ok;
}

boolean PColor::write (PFile* f) {
    int r, g, b;
    value->Intensities(r, g, b);
    return 
	Persistent::write(f) && f->Write(value->PixelValue()) &&
	f->Write(r) && f->Write(g) && f->Write(b);
}

ClassId PColor::GetClassId () { return PCOLOR; }
boolean PColor::IsA (ClassId id) { return PCOLOR == id || Persistent::IsA(id);}
PColor::PColor () { value = nil; }
PColor::PColor (int r, int g, int b) { value = new Color (r, g, b); }
PColor::PColor (const char* name) { value = new Color (name); }

PColor::~PColor () {
    delete value;
}

/*****************************************************************************/

boolean PPattern::read (PFile* f) {
    boolean ok = Persistent::read(f);

    if (ok) {
	ok = f->Read(data, patternHeight);
	if (ok) {
	    delete value;
	    value = new Pattern (data);
	}
    }
    return ok;
}

boolean PPattern::write (PFile* f) {
    return Persistent::write(f) && f->Write(data, patternHeight);
}

ClassId PPattern::GetClassId () { return PPATTERN; }

boolean PPattern::IsA (ClassId id) { 
    return PPATTERN == id || Persistent::IsA(id);
}

PPattern::PPattern () { value = nil; }

PPattern::PPattern (int p[patternHeight]) {
    bcopy(p, this->data, sizeof(int)*patternHeight);
    value = new Pattern (p);
}

PPattern::PPattern (int dither) {
    register int i;
    int r[4];

    for (i = 0; i < 4; i++) {
	r[i] = (dither & 0xf000) >> 12;
	r[i] |= r[i] << 4;
	r[i] |= r[i] << 8;
	dither <<= 4;
    }
    for (i = 0; i < patternHeight; i++) {
	data[i] = r[i%4];
    }
    value = new Pattern (data);
}

PPattern::~PPattern () {
    delete value;
}

/*****************************************************************************/

boolean PBrush::read (PFile* f) {
    int w;
    boolean ok = Persistent::read(f) && f->Read(w) && f->Read(p);
    
    if (ok) {
	if (w == 1 && p == 0xffff) { 
	    value = single;		// new brushes don't work on GPX; this
	    value->Reference();		// gives us at least the default
	} else if (w == NO_WIDTH) {
	    value = nil;
	} else {
	    value = new Brush(p, w);
	}
    }
    return ok;
}

boolean PBrush::write (PFile* f) {
    return Persistent::write(f) && f->Write(value->Width()) && f->Write(p);
}

ClassId PBrush::GetClassId  () { return PBRUSH; }
boolean PBrush::IsA (ClassId id) { return PBRUSH == id || Persistent::IsA(id);}
PBrush::PBrush () { value = nil; }

PBrush::PBrush (int p, int w) {
    this->p = p;
    if (w == 1  && p == 0xffff) { 
	value = single;		    // new brushes don't work on GPX; this
	value->Reference();	    // gives us at least the default
    } else if (w == NO_WIDTH) {
	value = nil;
    } else {
	value = new Brush(p, w);
    }
}

PBrush::~PBrush () {
    delete value;
}

/*****************************************************************************/

boolean PFont::read (PFile* f) {
    int count;
    boolean ok = Persistent::read(f) && f->Read(count);
    
    if (ok) {
	delete name;
	this->count = count;
	if (count == 0) {
	    name = nil;
	    delete value;
	    value = stdfont;
	} else {
	    name = new char [count];
	    ok = f->Read(name);
	    if (ok) {
		delete value;
		value = new Font (name, count);
	    }
	}
    }
    return ok;
}

boolean PFont::write (PFile* f) {
    boolean ok = Persistent::write(f) && f->Write(count);
    if (ok && count > 0) {
	ok = f->Write(name);
    }
    return ok;
}

ClassId PFont::GetClassId () { return PFONT; }
boolean PFont::IsA (ClassId id) { return PFONT == id || Persistent::IsA(id); }

PFont::PFont () {
    name = nil;
    value = nil;
}

PFont::PFont (const char* name) {
    if (name == nil) {
	count = 0;
	value = stdfont;
	this->name = nil;
    } else {
	count = strlen(name) + 1;
	this->name = new char [count];
	strcpy(this->name, name);
	value = new Font (this->name);
    }
}

PFont::PFont (const char* name, int count) {
    this->count = count + 1;
    this->name = new char [count + 1];
    strncpy(this->name, name, count);
    this->name[count] = '\0';
    value = new Font (this->name);
}

PFont::~PFont () {
    if (count > 0) {
	delete name;
	delete value;
    }
}

int PFont::Index (const char* s, int offset, boolean between) {
    return value->Index(s, offset, between);
}

int PFont::Index (const char* s, int len, int offset, boolean between) {
    return value->Index(s, len, offset, between);
}

/*****************************************************************************/

static int spat[] = {
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff
};

static int cpat[] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

void InitPPaint () {
    pblack = new PColor("black");
    pwhite = new PColor("white");
    pstdfont = new PFont(nil);
    psolid = new PPattern(spat);
    pclear = new PPattern(cpat);
    psingle = new PBrush(0xffff);
}
