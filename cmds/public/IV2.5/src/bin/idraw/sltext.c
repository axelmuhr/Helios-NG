// $Header: sltext.c,v 1.9 89/04/17 00:31:44 linton Exp $
// implements class TextSelection.

#include "istring.h"
#include "liststring.h"
#include "sltext.h"
#include <InterViews/Graphic/label.h>
#include <bstring.h>
#include <stream.h>

// TextSelection copies the original text and appends Labels for each
// line of text to the TextSelection.

TextSelection::TextSelection (StringList* sl, Graphic* gs) : (gs) {
    height = 0;
    stringlist = new StringList;
    for (sl->First(); !sl->AtEnd(); sl->Next()) {
	const char* string = sl->GetCur()->GetString();
	stringlist->Append(new StringNode(string));
	Append(new Label(string));
    }
}

// TextSelection reads data to initialize its graphic state and create
// its lines of text.

TextSelection::TextSelection (istream& from, State* state) : (nil) {
    ReadTextGS(from, state);
    height = 0;
    stringlist = new StringList;
    if (versionnumber >= 3) {
	Skip(from);
	char lookahead = ' ';
	while (from >> lookahead && lookahead != ']') {
	    if (lookahead != '[') {
		from.putback(lookahead);
	    }
	    const char* string = ReadString(from);
	    stringlist->Append(new StringNode(string));
	    Append(new Label(string));
	}
    } else {
	while (from >> buf && strcmp(buf, startdata) == 0) {
	    char blank;
	    from.get(blank);
	    from.get(buf, BUFSIZE);
	    stringlist->Append(new StringNode(buf));
	    Append(new Label(buf));
	}
    }
}

// Free storage allocated for the text in the TextSelection.

TextSelection::~TextSelection () {
    delete stringlist;
}

// Copy returns a copy of the TextSelection.

Graphic* TextSelection::Copy () {
    return new TextSelection(stringlist, this);
}

// IsA returns true if the TextSelection is a TextSelection so Editor
// can identify TextSelections and edit them differently.

boolean TextSelection::IsA (ClassId id) {
    return id == TEXTSELECTION || Picture::IsA(id);
}

// GetOriginal returns a copy of the TextSelection's text in a newly
// allocated list.

StringList* TextSelection::GetOriginal () {
    StringList* sl = new StringList;
    for (stringlist->First(); !stringlist->AtEnd(); stringlist->Next()) {
	const char* string = stringlist->GetCur()->GetString();
	sl->Append(new StringNode(string));
    }
    return sl;
}

// ShapedBy returns true if the TextSelection intersects a box around
// the given point.

boolean TextSelection::ShapedBy (Coord px, Coord py, float maxdist) {
    int slop = round(maxdist / 2);
    BoxObj pickpoint(px - slop, py - slop, px + slop, py + slop);
    return (LastGraphicIntersecting(pickpoint) != nil);
}

// draw readjusts the spacing between lines of text for a new font if
// necessary and sets fillbg false and pattern solid to draw the text
// like the printer will.

void TextSelection::draw (Canvas* c, Graphic* gs) {
    ReadjustSpacing(gs->GetFont());
    boolean fillbg = gs->BgFilled();
    PPattern* pattern = gs->GetPattern();

    gs->SetPattern(psolid);
    gs->FillBg(false);
    Selection::draw(c, gs);
    gs->FillBg(fillbg);
    gs->SetPattern(pattern);
}

// drawClipped readjusts the spacing between lines of text for a new
// font if necessary and sets fillbg false and pattern solid to draw
// the text like the printer will.

void TextSelection::drawClipped (Canvas* c, Coord l, Coord b, Coord r, Coord t,
Graphic* gs) {
    ReadjustSpacing(gs->GetFont());
    boolean fillbg = gs->BgFilled();
    PPattern* pattern = gs->GetPattern();

    gs->SetPattern(psolid);
    gs->FillBg(false);
    Selection::drawClipped(c, l, b, r, t, gs);
    gs->FillBg(fillbg);
    gs->SetPattern(pattern);
}

// ReadjustSpacing recalculates the spacing between lines of text.

void TextSelection::ReadjustSpacing (PFont* font) {
    if (font != nil && height != font->Height()) {
	height = font->Height();
	int vertoffset = -height;
	for (First(); !AtEnd(); Next()) {
	    Graphic* label = GetCurrent();
	    label->SetTransformer(nil);
	    label->Translate(0, vertoffset);
	    vertoffset -= height;
	}
    }
}

// ReadString reads and returns a string written in Postscript syntax.

const char* TextSelection::ReadString (istream& from) {
    const int INITIALSIZE = 256;
    static int sizebuffer = 0;
    static char* buffer = nil;
    if (INITIALSIZE > sizebuffer) {
	sizebuffer = INITIALSIZE;
	buffer = new char[sizebuffer];
    }

    char c = ' ';
    while (from.get(c) && c != '(') {
	// skip characters before next string
    }

    int lenbuffer = 0;
    while (from.get(c) && c != ')') {
	if (c == '\\') {
	    from.get(c);
	}
	buffer[lenbuffer++] = c;

	if (lenbuffer == sizebuffer) {
	    char* oldbuffer = buffer;
	    sizebuffer += INITIALSIZE/2;
	    buffer = new char[sizebuffer];
	    bcopy(oldbuffer, buffer, lenbuffer * sizeof(char));
	    delete oldbuffer;
	}
    }
    buffer[lenbuffer] = '\0';

    return buffer;
}

// WriteData writes the TextSelection's data and Postscript code to
// draw it.

void TextSelection::WriteData (ostream& to) {
    to << "Begin " << startdata << " Text\n";
    WriteTextGS(to);
    to << startdata << "\n";
    to << "[\n";
    for (stringlist->First(); !stringlist->AtEnd(); stringlist->Next()) {
	const char* string = Filter(stringlist->GetCur()->GetString());
	to << "(" << string << ")\n";
    }
    to << "] Text\n";
    to << "End\n\n";
}

// Filter escapes embedded special characters that would cause syntax
// errors in a Postscript string.

const char* TextSelection::Filter (const char* string) {
    const int INITIALSIZE = 80;
    static int sizebuffer = 0;
    static char* buffer = nil;

    int newsize = max(2 * strlen(string) + 1, INITIALSIZE);
    if (newsize > sizebuffer) {
	delete buffer;
	sizebuffer = newsize;
	buffer = new char[sizebuffer];
    }

    for (char* p = buffer; *string != '\0'; p++, string++) {
	switch (*string) {
	case '(':
	case ')':
	case '\\':
	    *p++ = '\\';
	    // fall through
	default:
	    *p = *string;
	}
    }
    *p = '\0';

    return buffer;
}
