// $Header: mapipaint.c,v 1.10 89/04/06 14:52:19 interran Exp $
// implements MapIPaint subclasses.

#include "ipaint.h"
#include "istring.h"
#include "listibrush.h"
#include "listicolor.h"
#include "listifont.h"
#include "listipattern.h"
#include "mapipaint.h"
#include <InterViews/interactor.h>
#include <bstring.h>
#include <stream.h>

// Init creates the list's entries, initializes the number denoting
// the initial entry, and ensures both list and initial are valid.

void MapIPaint::Init (BaseList* list, Interactor* i, const char* menu) {
    DefineEntries(list, i, menu);
    DefineInitial(i, menu);
    if (list->Size() == 0) {
	fprintf(stderr, "No entries in %s menu, ", menu);
	fprintf(stderr, "creating a default entry\n");
	list->Append(CreateEntry(nil));
    }
    if (list->Index(initial-1) == nil) {
	fprintf(stderr, "No attribute at %s entry %d, ", menu, initial);
	fprintf(stderr, "setting initial attribute from first entry\n");
	initial = 1;
    }
}

// DefineEntries reads predefined or user-defined attributes and
// creates entries in the list from these attributes.  It retrieves
// each attribute using the concatenation of the menu's name and a
// number which increments from 1.  The first undefined or empty
// attribute terminates the menu's definition.

void MapIPaint::DefineEntries (BaseList* list, Interactor* i,
const char* menu) {
    char menuproperty[20];
    int num = 1;
    sprintf(menuproperty, "%s%d", menu, num);

    const char* attribute = i->GetAttribute(menuproperty);
    while (attribute != nil && strlen(attribute) > 0) {
	BaseNode* entry = CreateEntry(attribute);
	if (entry != nil) {
	    list->Append(entry);
	} else {
	    fprintf(stderr, "couldn't parse attribute for %s\n", menuproperty);
	}

	sprintf(menuproperty, "%s%d", menu, ++num);
	attribute = i->GetAttribute(menuproperty);
    }
}

// DefineInitial reads a predefined or user-defined attribute to
// define the number denoting the initial entry.  For example,
// "idraw.initialfont: 2" sets the initial font from the second entry.

void MapIPaint::DefineInitial (Interactor* i, const char* menu) {
    char property[20];
    sprintf(property, "initial%s", menu);

    const char* def = i->GetAttribute(property);
    char* definition = strdup(def); // some sscanfs write to their format...
    if (sscanf(definition, "%d", &initial) != 1) {
	initial = 2;		// default if we can't parse definition
	fprintf(stderr, "can't parse attribute for %s, ", property);
	fprintf(stderr, "value set to %d\n", initial);
    }
    delete definition;
}

// CreateEntry creates and returns an entry containing an attribute
// defined by the given string.  A subclass must implement it.

BaseNode* MapIPaint::CreateEntry (const char*) {
    return new BaseNode;
}

// Skew comments/code ratio to work around cpp bug
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

// MapIBrush creates its brushes from predefined or user-defined
// attributes.

MapIBrush::MapIBrush (Interactor* i, const char* menu) {
    ibrushlist = new IBrushList;
    Init(ibrushlist, i, menu);
}

// ~MapIBrush frees storage allocated for the brushes and the list.

MapIBrush::~MapIBrush () {
    for (IBrush* brush = First(); !AtEnd(); brush = Next()) {
	delete brush;
    }
    delete ibrushlist;
}

// Implement functions to give readonly access to the list.

int MapIBrush::Size () {
    return ibrushlist->Size();
}

boolean MapIBrush::AtEnd () {
    return ibrushlist->AtEnd();
}

IBrush* MapIBrush::First () {
    return ibrushlist->First()->GetBrush();
}

IBrush* MapIBrush::Last () {
    return ibrushlist->Last()->GetBrush();
}

IBrush* MapIBrush::Prev () {
    return ibrushlist->Prev()->GetBrush();
}

IBrush* MapIBrush::Next () {
    return ibrushlist->Next()->GetBrush();
}

IBrush* MapIBrush::GetCur () {
    return ibrushlist->GetCur()->GetBrush();
}

IBrush* MapIBrush::Index (int index) {
    return ibrushlist->Index(index)->GetBrush();
}

boolean MapIBrush::Find (IBrush* brush) {
    return ibrushlist->Find(brush);
}

IBrush* MapIBrush::GetInitial () {
    // remember index = 0-base, initial = 1-base
    return ibrushlist->Index(initial-1)->GetBrush();
}

// FindOrAppend checks if the list already has a brush with the same
// attributes as the given attributes.  If so, it returns the brush
// already in the list; otherwise, it creates a new brush, appends it
// to the list, and returns it.

IBrush* MapIBrush::FindOrAppend (boolean none, int p, int w, int l, int r) {
    IBrush* brush = nil;
    if (none) {
	for (brush = First(); !AtEnd(); brush = Next()) {
	    if (brush->None()) {
		return brush;
	    }
	}
	brush = new IBrush;
    } else {
	for (brush = First(); !AtEnd(); brush = Next()) {
	    if (!brush->None() &&
		brush->GetLinePattern() == p &&
		brush->Width() == w &&
		brush->LeftArrow() == l &&
		brush->RightArrow() == r)
	    {
		return brush;
	    }
	}
	brush = new IBrush(p, w, l, r);
    }

    ibrushlist->Append(new IBrushNode(brush));
    return brush;
}

// CreateEntry returns an entry containing a brush created by parsing
// the given definition or an entry containing a default brush if no
// definition was given.  The definition usually contains two numbers
// and two booleans: a 16-bit hexadecimal number to define the brush's
// pattern, a decimal integer to define the brush's width in pixels,
// either 0 or 1 to determine whether lines start from an arrowhead,
// and either 0 or 1 to determine whether lines end in an arrowhead.
// One definition may contain the string "none" to define the
// nonexistent brush.  I found out sscanf barfs if you put 0x before
// the hexadecimal number so you can't do that.

BaseNode* MapIBrush::CreateEntry (const char* def) {
    if (def == nil) {
	def = "ffff 1 0 0";
    }
    char* definition = strdup(def); // some sscanfs write to their format...

    BaseNode* entry = nil;
    int p, w;
    boolean l, r;
    boolean none = (definition[0] == 'n' || definition[0] == 'N');

    if (none) {
	entry = new IBrushNode(new IBrush);
    } else if (sscanf(definition, "%x %d %d %d", &p, &w, &l, &r) == 4) {
	entry = new IBrushNode(new IBrush(p, w, l, r));
    }
    delete definition;

    return entry;
}

// MapIColor creates its colors from predefined or user-defined
// attributes.

MapIColor::MapIColor (Interactor* i, const char* menu) {
    icolorlist = new IColorList;
    Init(icolorlist, i, menu);
}

// ~MapIColor frees storage allocated for the colors and the list.

MapIColor::~MapIColor () {
    for (IColor* color = First(); !AtEnd(); color = Next()) {
	delete color;
    }
    delete icolorlist;
}

// Implement functions to give readonly access to the list.

int MapIColor::Size () {
    return icolorlist->Size();
}

boolean MapIColor::AtEnd () {
    return icolorlist->AtEnd();
}

IColor* MapIColor::First () {
    return icolorlist->First()->GetColor();
}

IColor* MapIColor::Last () {
    return icolorlist->Last()->GetColor();
}

IColor* MapIColor::Prev () {
    return icolorlist->Prev()->GetColor();
}

IColor* MapIColor::Next () {
    return icolorlist->Next()->GetColor();
}

IColor* MapIColor::GetCur () {
    return icolorlist->GetCur()->GetColor();
}

IColor* MapIColor::Index (int index) {
    return icolorlist->Index(index)->GetColor();
}

boolean MapIColor::Find (IColor* color) {
    return icolorlist->Find(color);
}

IColor* MapIColor::GetInitial () {
    // remember index = 0-base, initial = 1-base
    return icolorlist->Index(initial-1)->GetColor();
}

// FindOrAppend checks if the list already has a color with the same
// name as the given name.  If so, it returns the color already in the
// list; otherwise, it creates a new color, appends it to the list,
// and returns it.

IColor* MapIColor::FindOrAppend (const char* name, int r, int g, int b) {
    IColor* color = nil;
    for (color = First(); !AtEnd(); color = Next()) {
	if (strcmp(color->GetName(), name) == 0) {
	    return color;
	}
    }

    color = new IColor(name);
    if (!color->Valid()) {
	delete color;
	color = new IColor(r, g, b, name);
	if (!color->Valid()) {
	    fprintf(stderr, "invalid color name %s ", name);
	    fprintf(stderr, "and intensities %d %d %d, ", r, g, b);
	    fprintf(stderr, "substituting black\n");
	    delete color;
	    color = new IColor(black, name);
	}
    }
    icolorlist->Append(new IColorNode(color));
    return color;
}

// CreateEntry returns an entry containing a color created by using
// the given name or an entry containing a default color if no name
// was given.

BaseNode* MapIColor::CreateEntry (const char* def) {
    if (def == nil) {
	def = "Black";
    }
    char* definition = strdup(def); // some sscanfs write to their format...

    IColor* color = nil;
    char name[100];
    int r = 0, g = 0, b = 0;

    if (sscanf(definition, "%s %d %d %d", name, &r, &g, &b) == 4) {
	color = new IColor(r, g, b, name);
	if (!color->Valid()) {
	    fprintf(stderr, "invalid intensities %d %d %d, ", r, g, b);
	    fprintf(stderr, "using color name %s\n", name);
	    delete color;
	    color = new IColor(name);
	    if (!color->Valid()) {
		fprintf(stderr, "invalid color name %s, ", name);
		fprintf(stderr, "substituting black\n");
		delete color;
		color = new IColor(black, name);
	    }
	}
    } else if (sscanf(definition, "%s", name) == 1) {
	color = new IColor(name);
	if (!color->Valid()) {
	    fprintf(stderr, "invalid color name %s, ", name);
	    fprintf(stderr, "substituting black\n");
	    delete color;
	    color = new IColor(black, name);
	}
    }
    delete definition;

    BaseNode* entry = nil;
    if (color != nil) {
	entry = new IColorNode(color);
    }
    return entry;
}

// MapIFont creates its fonts from predefined or user-defined
// attributes.

MapIFont::MapIFont (Interactor* i, const char* menu) {
    ifontlist = new IFontList;
    Init(ifontlist, i, menu);
}

// ~MapIFont frees storage allocated for the fonts and the list.

MapIFont::~MapIFont () {
    for (IFont* font = First(); !AtEnd(); font = Next()) {
	delete font;
    }
    delete ifontlist;
}

// Implement functions to give readonly access to the list.

int MapIFont::Size () {
    return ifontlist->Size();
}

boolean MapIFont::AtEnd () {
    return ifontlist->AtEnd();
}

IFont* MapIFont::First () {
    return ifontlist->First()->GetFont();
}

IFont* MapIFont::Last () {
    return ifontlist->Last()->GetFont();
}

IFont* MapIFont::Prev () {
    return ifontlist->Prev()->GetFont();
}

IFont* MapIFont::Next () {
    return ifontlist->Next()->GetFont();
}

IFont* MapIFont::GetCur () {
    return ifontlist->GetCur()->GetFont();
}

IFont* MapIFont::Index (int index) {
    return ifontlist->Index(index)->GetFont();
}

boolean MapIFont::Find (IFont* font) {
    return ifontlist->Find(font);
}

IFont* MapIFont::GetInitial () {
    // remember index = 0-base, initial = 1-base
    return ifontlist->Index(initial-1)->GetFont();
}

// FindOrAppend checks if the list already has a font with the same
// attributes as the given attributes (except for the name because it
// depends on the window system used).  If so, it returns the font
// already in the list; otherwise, it creates a new font, appends it
// to the list, and returns it.

IFont* MapIFont::FindOrAppend (const char* name, const char* pf,
const char* ps) {
    IFont* font = nil;
    for (font = First(); !AtEnd(); font = Next()) {
	if (strcmp(font->GetPrintFont(), pf) == 0
	    && strcmp(font->GetPrintSize(), ps) == 0)
	{
	    return font;
	}
    }

    font = new IFont(name, pf, ps);
    if (!font->Valid()) {
	fprintf(stderr, "invalid font name %s, ", name);
	fprintf(stderr, "substituting stdfont\n");
	delete font;
	font = new IFont("stdfont", pf, ps);
    }
    ifontlist->Append(new IFontNode(font));
    return font;
}

// CreateEntry returns an entry containing a font created by parsing
// the given definition or an entry containing a default font if no
// definition was given.  The definition must contain three strings
// separated by whitespace to define a font.  The first string defines
// the font's name, the second string the corresponding print font,
// and the third string the print size.

BaseNode* MapIFont::CreateEntry (const char* def) {
    if (def == nil) {
	def = "stdfont Courier 10";
    }
    char* definition = strdup(def); // some sscanfs write to their format...

    BaseNode* entry = nil;
    char name[100];
    char pf[100];
    char ps[10];

    if (sscanf(definition, "%s %s %s", name, pf, ps) == 3) {
	IFont* font = new IFont(name, pf, ps);
	if (!font->Valid()) {
	    fprintf(stderr, "invalid font name %s, ", name);
	    fprintf(stderr, "substituting stdfont\n");
	    delete font;
	    font = new IFont("stdfont", pf, ps);
	}
	entry = new IFontNode(font);
    }
    delete definition;

    return entry;
}

// MapIPattern creates its patterns from predefined or user-defined
// attributes.

MapIPattern::MapIPattern (Interactor* i, const char* menu) {
    ipatternlist = new IPatternList;
    Init(ipatternlist, i, menu);
}

// ~MapIPattern frees storage allocated for the patterns and the list.

MapIPattern::~MapIPattern () {
    for (IPattern* pattern = First(); !AtEnd(); pattern = Next()) {
	delete pattern;
    }
    delete ipatternlist;
}

// Implement functions to give readonly access to the list.

int MapIPattern::Size () {
    return ipatternlist->Size();
}

boolean MapIPattern::AtEnd () {
    return ipatternlist->AtEnd();
}

IPattern* MapIPattern::First () {
    return ipatternlist->First()->GetPattern();
}

IPattern* MapIPattern::Last () {
    return ipatternlist->Last()->GetPattern();
}

IPattern* MapIPattern::Prev () {
    return ipatternlist->Prev()->GetPattern();
}

IPattern* MapIPattern::Next () {
    return ipatternlist->Next()->GetPattern();
}

IPattern* MapIPattern::GetCur () {
    return ipatternlist->GetCur()->GetPattern();
}

IPattern* MapIPattern::Index (int index) {
    return ipatternlist->Index(index)->GetPattern();
}

boolean MapIPattern::Find (IPattern* pattern) {
    return ipatternlist->Find(pattern);
}

IPattern* MapIPattern::GetInitial () {
    // remember index = 0-base, initial = 1-base
    return ipatternlist->Index(initial-1)->GetPattern();
}

// Skew comments/code ratio to work around cpp bug
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

// FindOrAppend checks if the list already has a pattern with the same
// attributes as the given attributes.  If so, it returns the pattern
// already in the list; otherwise, it creates a new pattern, appends
// it to the list, and returns it.

IPattern* MapIPattern::FindOrAppend (boolean none, float graylevel,
int data[patternHeight], int size) {
    IPattern* pattern = nil;
    if (none) {
	for (pattern = First(); !AtEnd(); pattern = Next()) {
	    if (pattern->None()) {
		return pattern;
	    }
	}
	pattern = new IPattern;
    } else if (graylevel != -1) {
	for (pattern = First(); !AtEnd(); pattern = Next()) {
	    if (pattern->GetGrayLevel() == graylevel) {
		return pattern;
	    }
	}
	int shade = CalcBitmap(graylevel);
	pattern = new IPattern(shade, graylevel);
    } else {
	ExpandToFullSize(data, size);
	for (pattern = First(); !AtEnd(); pattern = Next()) {
	    if (pattern->GetSize() != 0) {
		const int* cmpdata = pattern->GetData();
		if (bcmp(data, cmpdata, patternHeight * sizeof(int)) == 0) {
		    return pattern;
		}
	    }
	}
	pattern = new IPattern(data, size);
    }

    ipatternlist->Append(new IPatternNode(pattern));
    return pattern;
}

// CreateEntry returns an entry containing a pattern created by
// parsing the given definition or an entry containing a default
// pattern if no definition was given.  The definition usually
// contains either one or patternHeight 16-bit hexadecimal numbers to
// define either a 4x4 pattern or a patternWidth x patternHeight
// pattern.  One definition may contain the string "none" to define
// the nonexistent pattern.  I found out sscanf barfs if you put 0x
// before the hexadecimal number so you can't do that.

BaseNode* MapIPattern::CreateEntry (const char* def) {
    if (def == nil) {
	def = "0.0";
    }
    char* definition = strdup(def); // some sscanfs write to their format...

    BaseNode* entry = nil;
    boolean none = (definition[0] == 'n' || definition[0] == 'N');

    if (none) {
	entry = new IPatternNode(new IPattern);
    } else {
	if (strchr(definition, '.') != nil) {
	    float graylevel;

	    if (sscanf(definition, "%f", &graylevel) == 1) {
		int shade = CalcBitmap(graylevel);
		entry = new IPatternNode(new IPattern(shade, graylevel));
	    }
	} else {
	    istream from(strlen(definition) + 1, definition);
	    char buffer[80];
	    int data[patternHeight];

	    for (int i = 0; from >> buffer && i < patternHeight; i++) {
		if (sscanf(buffer, "%x", &data[i]) != 1) {
		    break;
		}
	    }

	    if (i == 1 || i == 8 || i == patternHeight) {
		ExpandToFullSize(data, i);
		entry = new IPatternNode(new IPattern(data, i));
	    }
	}
    }
    delete definition;

    return entry;
}

// CalcBitmap calculates a 4x4 bitmap to represent a shade having the
// given gray level.

int MapIPattern::CalcBitmap (float graylevel) {
    static const int SHADES = 17;
    static int shades[SHADES] = {
	0xffff, 0xefff, 0xefbf, 0xafbf,
	0xafaf, 0xadaf, 0xada7, 0xa5a7,
	0xa5a5, 0x85a5, 0x8525, 0x0525,
	0x0505, 0x0405, 0x0401, 0x0001,
	0x0000
    };
    return shades[round(graylevel * (SHADES - 1))];
}

// ExpandToFullSize expands the bitmap from 4x4 or 8x8 to 16x16.

void MapIPattern::ExpandToFullSize (int data[patternHeight], int size) {
    if (size == 1) {
	int seed = data[0];
	for (int i = 0; i < 4; i++) {
	    data[i] = (seed & 0xf000) >> 12;
	    data[i] |= data[i] << 4;
	    data[i] |= data[i] << 8;
	    data[i+4] = data[i];
	    data[i+8] = data[i];
	    data[i+12] = data[i];
	    seed <<= 4;
	}
    } else if (size == 8) {
	for (int i = 0; i < 8; i++) {
	    data[i] &= 0xff;
	    data[i] |= data[i] << 8;
	    data[i+8] = data[i];
	}
    } else if (size == patternHeight) {
	const unsigned int patternWidthMask = ~(~0 << patternWidth);
	for (int i = 0; i < patternHeight; i++) {
	    data[i] &= patternWidthMask;
	}
    } else {
	fprintf(stderr, "invalid size passed to ExpandToFullSize\n");
    }
}
