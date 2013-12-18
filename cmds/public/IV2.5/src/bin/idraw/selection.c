// $Header: selection.c,v 1.14 89/05/18 16:55:54 vlis Exp $
// implements classes Selection and NPtSelection.

#include "ipaint.h"
#include "istring.h"
#include "listifont.h"
#include "mapipaint.h"
#include "selection.h"
#include "state.h"
#include <InterViews/rubcurve.h>
#include <InterViews/transformer.h>
#include <InterViews/Graphic/util.h>
#include <stream.h>

// Define the start of data token.

const char* startdata = "%I";

// Selection starts off with no handles.

Selection::Selection (Graphic* gs) : (gs) {
    handles = nil;
}

// Free storage allocated for the handles if any.

Selection::~Selection () {
    DeleteHandles();
}

// Copy returns a copy of the Selection.

Graphic* Selection::Copy () {
    return new Selection(this);
}

// HasChildren returns false so Idraw won't ungroup this Picture.

boolean Selection::HasChildren () {
    return false;
}

// GetPaddedBox returns the Selection's smallest box with enough
// padding added to include its handles.

void Selection::GetPaddedBox (BoxObj& box) {
    Picture::GetBox(box);
    const int HDPAD = HDSIZE/2 + 1; // how much to add to GetBox's size
    box.left -= HDPAD;
    box.bottom -= HDPAD;
    box.right += HDPAD;
    box.top += HDPAD;
}

// DrawHandles tells the handles to draw themselves unless they've
// already drawn themselves.

void Selection::DrawHandles (Painter* rasterxor, Canvas* canvas) {
    if (handles == nil) {
	CreateHandles();
    }
    handles->SetPainter(rasterxor);
    handles->SetCanvas(canvas);
    handles->Draw();
}

// EraseHandles tells the handles to erase themselves unless they've
// already erased themselves.

void Selection::EraseHandles (Painter* rasterxor, Canvas* canvas) {
    if (handles != nil) {
	handles->SetPainter(rasterxor);
	handles->SetCanvas(canvas);
	handles->Erase();
    }
}

// RedrawHandles knows for sure that no unerased handles remain on the
// screen, so it resets the handles to outline the Selection's
// possibly different shape and location before drawing the handles.

void Selection::RedrawHandles (Painter* rasterxor, Canvas* canvas) {
    DeleteHandles();
    DrawHandles(rasterxor, canvas);
}

// RedrawUnclippedHandles knows that some unerased handles probably
// remain on the screen so it can't reset the handles, but it can tell
// the handles to draw themselves whether or not they've already drawn
// themselves because the painter will clip the already drawn handles.

void Selection::RedrawUnclippedHandles (Painter* rasterxor, Canvas* canvas) {
    if (handles == nil) {
	CreateHandles();
    }
    handles->SetPainter(rasterxor);
    handles->SetCanvas(canvas);
    handles->Redraw();
}

// ResetHandles deletes the handles since the Selection may have moved
// out from under them.  Redrawing the handles will recreate them.

void Selection::ResetHandles () {
    DeleteHandles();
}

// ShapedBy returns false since the Selection does not contain any
// points which both determine its shape and fall within the given
// distance of the given point.

boolean Selection::ShapedBy (Coord, Coord, float) {
    return false;
}

// CreateShape creates and returns a Rubberband representing the
// Selection's shape for the user to reshape.

Rubberband* Selection::CreateShape (Coord, Coord) {
    return nil;
}

// GetReshapedCopy creates and returns a copy of the Selection
// incorporating the change made to its shape.

Selection* Selection::GetReshapedCopy () {
    return nil;
}

// Skip skips over tokens in the input stream until it reads a start
// of data token or reaches eof.

void Selection::Skip (istream& from) {
    while (from >> buf && strcmp(buf, startdata) != 0) {
	// skip Postscript code
    }
}

// ReadVersion reads the drawing's version number.  Knowing the
// drawing's version number allows us to invoke backward compatibility
// code if necessary or detect an incompatibility ahead of time.

static const int OLDESTVERSION  = 1;
static const int CURRENTVERSION = 5;

void Selection::ReadVersion (istream& from) {
    Skip(from);
    from >> buf;
    if (strcmp(buf, "Idraw") == 0) {
	from >> versionnumber;
    } else {
	versionnumber = OLDESTVERSION;
    }
    if (versionnumber > CURRENTVERSION) {
	fprintf(stderr, "warning: drawing version %d ", versionnumber);
	fprintf(stderr, "newer than idraw version %d\n", CURRENTVERSION);
    }
}    

// ReadGridSpacing reads the grid spacing used by the drawing and
// stores the new grid spacing value.  It must correct the default
// grid spacing it gives to old drawings for an implementation botch
// in InterViews 2.4 that calculated point's value incorrectly using
// 72.07/inch instead of inch/72.27 (it was a botch in TWO ways).

void Selection::ReadGridSpacing (istream& from) {
    const int oldspacing = 8;
    const double oldpoints = 72.07/inches;
    double g = oldpoints * round(oldspacing * oldpoints);
    if (versionnumber >= 5) {
	g = oldspacing;
	from >> buf;
	if (strcmp(buf, "Grid") == 0) {
	    from >> g;
	}
    }
    SetGridSpacing(g);
}

// ReadGS reads data to initialize the graphic state for Selections
// which don't contain any text.

void Selection::ReadGS (istream& from, State* state) {
    ReadBrush(from, state);
    if (versionnumber >= 4) {
	ReadFgColor(from, state);
	ReadBgColor(from, state);
	SetFont(nil);
    } else if (versionnumber >= 2) {
	ReadFgColor(from, state);
	IColor* bg = state->GetMapIBgColor()->GetInitial();
	SetColors(GetFgColor(), bg);
	SetFont(nil);
    } else {
	IColor* fg = state->GetMapIFgColor()->GetInitial();
	IColor* bg = state->GetMapIBgColor()->GetInitial();
	SetColors(fg, bg);
	ReadFont(from, state);
    }
    ReadPattern(from, state);
    ReadTransformer(from);
}

// ReadPictGS reads data to initialize the graphic state for
// PictSelections which may contain some text.

void Selection::ReadPictGS (istream& from, State* state) {
    ReadBrush(from, state);
    if (versionnumber >= 4) {
	ReadFgColor(from, state);
	ReadBgColor(from, state);
    } else if (versionnumber >= 2) {
	ReadFgColor(from, state);
	SetColors(GetFgColor(), nil);
    } else {
	SetColors(nil, nil);
    }
    ReadFont(from, state);
    ReadPattern(from, state);
    ReadTransformer(from);
}

// ReadTextGS reads data to initialize the graphic state for
// TextSelections which don't need a brush or pattern.

void Selection::ReadTextGS (istream& from, State* state) {
    if (versionnumber >= 2) {
	SetBrush(nil);
	ReadFgColor(from, state);
	SetColors(GetFgColor(), nil);
    } else {
	ReadBrush(from, state);
	SetColors(state->GetMapIFgColor()->GetInitial(), nil);
    }
    ReadFont(from, state);
    if (versionnumber < 3) {
	ReadPattern(from, state);
	IPattern* pattern = (IPattern*) GetPattern();
	float graylevel = pattern->GetGrayLevel();
	const char* c = "Black";
	int r = 0, g = 0, b = 0;
	if (graylevel != 0 && graylevel != -1) {
	    if (graylevel == 1) {
		c = "White";
		r = g = b = 65535;
	    } else {
		c = "Gray";
		r = g = b = 49152;
	    }
	}
	SetColors(state->GetMapIFgColor()->FindOrAppend(c, r, g, b), nil);
    } else {
	SetPattern(nil);
    }
    ReadTransformer(from);
}

// ReadBrush reads data to set the Selection's brush.

void Selection::ReadBrush (istream& from, State* state) {
    Skip(from);
    from >> buf;
    if (buf[0] == 'b') {
	char lookahead = 'u';
	boolean undefined = false;
	boolean none = false;
	int p = 0;
	int w = 0;
	int l = false;
	int r = false;

	from >> lookahead;
	from.putback(lookahead);
	switch (lookahead) {
	case 'u':
	    undefined = true;
	    break;
	case 'n':
	    none = true;
	    break;
	default:
	    from >> p >> w >> l >> r;
	    break;
	}

	if (undefined || !from.good()) {
	    SetBrush(nil);
	} else {
	    MapIBrush* mb = state->GetMapIBrush();
	    IBrush* brush = mb->FindOrAppend(none, p, w, l, r);
	    SetBrush(brush);
	}
    }
}

// ReadFgColor reads data to set the Selection's foreground color.

void Selection::ReadFgColor (istream& from, State* state) {
    Skip(from);
    from >> buf;
    if (buf[0] == 'c' && (buf[1] == 'f' || versionnumber < 4)) {
	char lookahead = 'u';
	boolean undefined = false;
	char name[100];
	float fr = 0, fg = 0, fb = 0;

	from >> lookahead;
	from.putback(lookahead);
	if (lookahead == 'u') {
	    undefined = true;
	} else {
	    from >> name;
	    if (versionnumber >= 4) {
		from >> fr >> fg >> fb;
	    }
	}

	if (undefined || !from.good()) {
	    SetColors(nil, GetBgColor());
	} else {
	    int r = round(fr * 0xffff);
	    int g = round(fg * 0xffff);
	    int b = round(fb * 0xffff);
	    MapIColor* mfg = state->GetMapIFgColor();
	    IColor* fgcolor = mfg->FindOrAppend(name, r, g, b);
	    SetColors(fgcolor, GetBgColor());
	}
    }
}

// ReadBgColor reads data to set the Selection's background color.

void Selection::ReadBgColor (istream& from, State* state) {
    Skip(from);
    from >> buf;
    if (buf[0] == 'c' && buf[1] == 'b') {
	char lookahead = 'u';
	boolean undefined = false;
	char name[100];
	float fr = 0, fg = 0, fb = 0;

	from >> lookahead;
	from.putback(lookahead);
	if (lookahead == 'u') {
	    undefined = true;
	} else {
	    from >> name >> fr >> fg >> fb;
	}

	if (undefined || !from.good()) {
	    SetColors(GetFgColor(), nil);
	} else {
	    int r = round(fr * 0xffff);
	    int g = round(fg * 0xffff);
	    int b = round(fb * 0xffff);
	    MapIColor* mbg = state->GetMapIBgColor();
	    IColor* bgcolor = mbg->FindOrAppend(name, r, g, b);
	    SetColors(GetFgColor(), bgcolor);
	}
    }
}

// ReadFont reads data to set the Selection's font.

void Selection::ReadFont (istream& from, State* state) {
    Skip(from);
    from >> buf;
    if (buf[0] == 'f') {
	char lookahead = 'u';
	boolean undefined = false;
	char name[100];
	char printfont[100];
	char printsize[100];

	from >> lookahead;
	from.putback(lookahead);
	if (lookahead == 'u') {
	    undefined = true;
	} else {
	    from >> name;
	    from >> printfont;
	    from >> printsize;
	}

	if (undefined || !from.good()) {
	    SetFont(nil);
	} else {
	    MapIFont* mf = state->GetMapIFont();
	    char* pf = (versionnumber >= 3) ? &printfont[1] : printfont;
	    IFont* font = mf->FindOrAppend(name, pf, printsize);
	    SetFont(font);
	}
    }
}

// ReadPattern reads data to set the Selection's pattern.

void Selection::ReadPattern (istream& from, State* state) {
    Skip(from);
    from >> buf;
    if (buf[0] == 'p') {
	char lookahead = 'u';
	boolean undefined = false;
	boolean none = false;
	float graylevel = 0;
	int data[patternHeight];
	int size = 0;

	from >> lookahead;
	switch (lookahead) {
	case 'u':
	    undefined = true;
	    break;
	case 'n':
	    none = true;
	    break;
	case '<':
	    graylevel = -1;
	    break;
	default:
	    from.putback(lookahead);
	    break;
	}

	if (!undefined && !none && graylevel != -1) {
	    if (versionnumber >= 4) {
		from >> graylevel;
	    } else {
		from >> data[0];
		graylevel = CalcGrayLevel(data[0]);
	    }
	} else if (graylevel == -1) {
	    for (int i = 0; from >> buf && i < patternHeight; i++) {
		if (buf[0] == '>' || sscanf(buf, "%x", &data[i]) != 1) {
		    break;
		}
	    }
	    if (buf[0] == '>') {
		size = i;
	    } else {
		undefined = true;
	    }
	}

	if (undefined || !from.good()) {
	    SetPattern(nil);
	} else {
	    MapIPattern* mp = state->GetMapIPattern();
	    IPattern* pattern = mp->FindOrAppend(none, graylevel, data, size);
	    SetPattern(pattern);
	}
    }
}

// ReadTransformer reads data to set the Selection's transformation
// matrix.

void Selection::ReadTransformer (istream& from) {
    Skip(from);
    from >> buf;
    if (buf[0] == 't') {
	char uorbracket = 'u';
	boolean undefined = false;
	float a00, a01, a10, a11, a20, a21;

	from >> uorbracket;
	if (uorbracket == 'u') {
	    undefined = true;
	} else {
	    if (versionnumber < 3) {
		from.putback(uorbracket);
	    }
	    from >> a00 >> a01 >> a10 >> a11 >> a20 >> a21;
	}

	if (from.good()) {
	    Transformer* tnew = nil;
	    if (!undefined) {
		tnew = new Transformer(a00, a01, a10, a11, a20, a21);
		SetTransformer(tnew);
		delete tnew;
	    }
	}
    }
}

// CalcGrayLevel calculates a 4x4 bitmap's gray level on the printer.
// Since the gray level ranges from 0 = solid to 1 = clear,
// CalcGrayLevel counts the number of 0 bits in the bitmap and divides
// the sum by the total number of bits in the bitmap.

float Selection::CalcGrayLevel (int seed) {
    const int numbits = 16;
    int numzeros = 0;
    for (int i = 0; i < numbits; i++) {
	numzeros += !((seed >> i) & 0x1);
    }
    return float(numzeros) / numbits;
}

// WriteData writes everything needed to draw or reconstruct the
// Selection.

void Selection::WriteData (ostream& to) {
    // define it in your subclass
}

// WriteVersion writes the drawing's version number.  Storing a
// version number with the drawing makes backward compatibility easier
// to support when the drawing format changes in the future.

void Selection::WriteVersion (ostream& to) {
    to << startdata << " Idraw " << CURRENTVERSION << " ";
}    

// WriteGridSpacing writes the drawing's grid spacing.  Storing the
// grid spacing with the drawing ensures graphics will remain aligned
// to the grid they were aligned to before.

void Selection::WriteGridSpacing (ostream& to) {
    to << "Grid " << gridspacing << "\n\n";
}    

// WriteGS writes the graphic state for Selections that don't contain
// any text.

void Selection::WriteGS (ostream& to) {
    WriteBrush(to);
    WriteFgColor(to);
    WriteBgColor(to);
    WritePattern(to);
    WriteTransformer(to);
}

// WritePictGS writes the graphic state for PictSelections which may
// contain some text.

void Selection::WritePictGS (ostream& to) {
    WriteBrush(to);
    WriteFgColor(to);
    WriteBgColor(to);
    WriteFont(to);
    WritePattern(to);
    WriteTransformer(to);
}

// WriteTextGS writes the graphic state for TextSelections which
// don't need a brush or pattern but do need a font.

void Selection::WriteTextGS (ostream& to) {
    WriteFgColor(to);
    WriteFont(to);
    WriteTransformer(to);
}

// WriteBrush writes the Selection's brush.

void Selection::WriteBrush (ostream& to) {
    IBrush* brush = (IBrush*) GetBrush();
    if (brush == nil) {
	to << startdata << " b u\n";
    } else if (brush->None()) {
	to << "none SetB " << startdata << " b n\n";
    } else {
	int p = brush->GetLinePattern();
	to << startdata << " b " << p << "\n";
	int w = brush->Width();
	boolean l = brush->LeftArrow();
	boolean r = brush->RightArrow();
	to << w << " " << l << " " << r << " ";
	const int* dashpat = brush->GetDashPattern();
	int dashpatsize = brush->GetDashPatternSize();
	int dashoffset = brush->GetDashOffset();
	if (dashpatsize <= 0) {
	    to << "[] " << dashoffset << " ";
	} else {
	    to << "[";
	    for (int i = 0; i < dashpatsize - 1; i++) {
		to << dashpat[i] << " ";
	    }
	    to << dashpat[i] << "] " << dashoffset << " ";
	}
	to << "SetB\n";
    }
}

// WriteFgColor writes the Selection's foreground color.

void Selection::WriteFgColor (ostream& to) {
    IColor* fgcolor = (IColor*) GetFgColor();
    if (fgcolor == nil) {
	to << startdata << " cfg u\n";
    } else {
	const char* name = fgcolor->GetName();
	to << startdata << " cfg " << name << "\n";
	if (strcmp(name, "white") == 0 || strcmp(name, "White") == 0) {
	    to << "1 1 1 SetCFg\n";
	} else {
	    int r, g, b;
	    fgcolor->Intensities(r, g, b);
	    float fr = float(r) / 0xffff;
	    float fg = float(g) / 0xffff;
	    float fb = float(b) / 0xffff; 
	    to << fr << " " << fg << " " << fb << " SetCFg\n";
	}
    }
}

// WriteBgColor writes the Selection's background color.

void Selection::WriteBgColor (ostream& to) {
    IColor* bgcolor = (IColor*) GetBgColor();
    if (bgcolor == nil) {
	to << startdata << " cbg u\n";
    } else {
	const char* name = bgcolor->GetName();
	to << startdata << " cbg " << name << "\n";
	if (strcmp(name, "white") == 0 || strcmp(name, "White") == 0) {
	    to << "1 1 1 SetCBg\n";
	} else {
	    int r, g, b;
	    bgcolor->Intensities(r, g, b);
	    float fr = float(r) / 0xffff;
	    float fg = float(g) / 0xffff;
	    float fb = float(b) / 0xffff; 
	    to << fr << " " << fg << " " << fb << " SetCBg\n";
	}
    }
}

// WriteFont writes the Selection's font.

void Selection::WriteFont (ostream& to) {
    IFont* font = (IFont*) GetFont();
    if (font == nil) {
	to << startdata << " f u\n";
    } else {
	const char* name = font->GetName();
	const char* pf = font->GetPrintFont();
	const char* ps = font->GetPrintSize();
	to << startdata << " f " << name << "\n";
	to << "/" << pf << " " << ps << " SetF\n";
    }
}

// WritePattern writes the Selection's pattern.

void Selection::WritePattern (ostream& to) {
    IPattern* pattern = (IPattern*) GetPattern();
    if (pattern == nil) {
	to << startdata << " p u\n";
    } else if (pattern->None()) {
	to << "none SetP " << startdata << " p n\n";
    } else if (pattern->GetSize() > 0) {
	const int* data = pattern->GetData();
	int size = pattern->GetSize();
	to << startdata << " p\n";
	to << "< ";
	if (size <= 8) {
	    for (int i = 0; i < 8; i++) {
		sprintf(buf, "%02x", data[i] & 0xff);
		to << buf << " ";
	    }
	} else {
	    for (int i = 0; i < patternHeight; i++) {
		sprintf(buf, "%0*x", patternWidth/4, data[i]);
		if (i != patternHeight - 2) {
		    to << buf << " ";
		} else {
		    to << buf << "\n  ";
		}
	    }
	}
	to << "> -1 SetP\n";
    } else {
	float graylevel = pattern->GetGrayLevel();
	to << startdata << " p\n";
	to << graylevel << " SetP\n";
    }
}

// WriteTransformer writes the Selection's transformation matrix.

void Selection::WriteTransformer (ostream& to) {
    Transformer* t = GetTransformer();
    if (t == nil || *t == *identity) {
	to << startdata << " t u\n";
    } else {
	float a00, a01, a10, a11, a20, a21;
	t->GetEntries(a00, a01, a10, a11, a20, a21);
	to << startdata << " t\n";
	to << "[ " << a00 << " " << a01 << " " << a10 << " ";
	to << a11 << " " << a20 << " " << a21 << " ] concat\n";
    }
}

// CreateHandles creates handles outlining the Selection's shape.

void Selection::CreateHandles () {
    const int N = 8;
    const int RUBPT = 0;
    Coord l, b, r, t, hx, hy, x[N], y[N];

    Picture::GetBox(l, b, r, t);
    hx = (r + l)/2;
    hy = (t + b)/2;
    x[0] = l;	y[0] = b;
    x[1] = hx;	y[1] = b;
    x[2] = r;	y[2] = b;
    x[3] = r;	y[3] = hy;
    x[4] = r;	y[4] = t;
    x[5] = hx;	y[5] = t;
    x[6] = l;	y[6] = t;
    x[7] = l;	y[7] = hy;

    handles = new RubberHandles(nil, nil, x, y, N, RUBPT, HDSIZE);
}

// DeleteHandles deletes the handles after setting their canvas to nil
// so they won't try to erase themselves.

void Selection::DeleteHandles () {
    if (handles != nil) {
	handles->SetCanvas(nil);
	delete handles;
	handles = nil;
    }
}

// NPtSelection passes its argument to Selection.

NPtSelection::NPtSelection (Graphic* gs) : (gs) {
    myname = "YouForgotToDefineMyName";
    rubbervertex = nil;
}

// GetOriginal returns the points that were passed to the
// the NPtSelection subclass's constructor.

void NPtSelection::GetOriginal (Coord*&, Coord*&, int&) {
    // implement it in your subclass
}

// ShapedBy returns true if any of the NPtSelection's points falls
// within the given distance of the given point.

boolean NPtSelection::ShapedBy (Coord px, Coord py, float maxdist) {
    Coord* x;
    Coord* y;
    int n;
    GetOriginal(x, y, n);
    TotalTransform(x, y, n);
    int closestpt = ClosestPoint(x, y, n, px, py);
    boolean shapedby = Distance(x[closestpt], y[closestpt], px, py) <= maxdist;
    delete x;
    delete y;
    return shapedby;
}

// CreateShape creates, stores, and returns a rubberband representing
// the NPtSelection's shape for the user to reshape.

Rubberband* NPtSelection::CreateShape (Coord px, Coord py) {
    Coord* x;
    Coord* y;
    int n;
    GetOriginal(x, y, n);
    TotalTransform(x, y, n);
    int rubpt = ClosestPoint(x, y, n, px, py);
    rubbervertex = CreateRubberVertex(x, y, n, rubpt);
    delete x;
    delete y;
    return rubbervertex;
}

// GetReshapedCopy creates and returns a copy of the NPtSelection
// incorporating the change made to its shape.

Selection* NPtSelection::GetReshapedCopy () {
    Coord* x;
    Coord* y;
    int n;
    int rubpt;
    rubbervertex->GetCurrent(x, y, n, rubpt);
    delete rubbervertex;

    InvTotalTransform(x, y, n);
    Selection* reshaped = CreateReshapedCopy(x, y, n);
    delete x;
    delete y;
    return reshaped;
}

// ReadPoints reads a set of points as efficiently as possible by
// using dynamic static buffers instead of mallocing on every call.

void NPtSelection::ReadPoints (istream& from, const Coord*& x, const Coord*& y,
int& n) {
    const int INITIALSIZE = 15;
    static int sizepoints = 0;
    static Coord* xcoords = nil;
    static Coord* ycoords = nil;

    Skip(from);
    from >> n;
    if (n > sizepoints) {
	delete xcoords;
	delete ycoords;
	sizepoints = max(n, INITIALSIZE);
	xcoords = new Coord[sizepoints];
	ycoords = new Coord[sizepoints];
    }

    for (int i = 0; i < n; i++) {
	if (versionnumber < 3) {
	    Skip(from);
	}
	from >> xcoords[i] >> ycoords[i];
    }

    x = xcoords;
    y = ycoords;
}

// WriteData writes the NPtSelection's data and Postscript code to
// draw it.

void NPtSelection::WriteData (ostream& to) {
    Coord* x;
    Coord* y;
    int n;
    GetOriginal(x, y, n);
    to << "Begin " << startdata << " " << myname << "\n";
    WriteGS(to);
    to << startdata << " " << n << "\n";
    for (int i = 0; i < n; i++) {
	to << x[i] << " " << y[i] << "\n";
    }
    to << n << " " << myname << "\n";
    to << "End\n\n";
}

// CreateRubberVertex creates and returns the right kind of
// RubberVertex to represent the NPtSelection's shape.

RubberVertex* NPtSelection::CreateRubberVertex (Coord*, Coord*, int, int) {
    // implement it in your subclass
    return nil;
}

// CreateReshapedCopy creates and returns a reshaped copy of itself
// using the passed points and its graphic state.

Selection* NPtSelection::CreateReshapedCopy (Coord*, Coord*, int) {
    // implement it in your subclass
    return nil;
}

// CreateHandles creates handles highlighting the NPtSelection's
// points.

void NPtSelection::CreateHandles () {
    const int RUBPT = 0;
    Coord* x;
    Coord* y;
    int n;

    GetOriginal(x, y, n);
    TotalTransform(x, y, n);
    handles = new RubberHandles(nil, nil, x, y, n, RUBPT, HDSIZE);
    delete x;
    delete y;
}

// TotalTransform transforms the given points from the Selection's
// coordinate system to the screen's coordinate system.

void NPtSelection::TotalTransform (Coord* x, Coord* y, int n) {
    Transformer total;

    TotalTransformation(total);
    for (int i = 0; i < n; i++) {
	total.Transform(x[i], y[i]);
    }
}

// InvTotalTransform transforms the given points from the screen's
// coordinate system to the NPtSelection's coordinate system.

void NPtSelection::InvTotalTransform (Coord* x, Coord* y, int n) {
    Transformer total;

    TotalTransformation(total);
    for (int i = 0; i < n; i++) {
	total.InvTransform(x[i], y[i]);
    }
}

// ClosestPoint returns the index within the arrays of the closest
// point to the given coordinates.

int NPtSelection::ClosestPoint (Coord* x, Coord* y, int n, Coord px,
Coord py) {
    int closestpt = 0;
    float mindist = Distance(x[0], y[0], px, py);
    for (int i = 1; i < n; i++) {
	float dist = Distance(x[i], y[i], px, py);
	if (dist < mindist) {
	    mindist = dist;
	    closestpt = i;
	}
    }
    return closestpt;
}

// LeftAcont checks whether the left arrowhead contains the point.

boolean NPtSelection::LeftAcont (Coord x0, Coord y0, Coord x1, Coord y1,
PointObj& po, Graphic* gs) {
    IBrush* brush = (IBrush*) gs->GetBrush();
    if (brush->LeftArrow()) {
	return ArrowHeadcont(x0, y0, x1, y1, po, gs);
    }
    return false;
}

// RightAcont checks whether the right arrowhead contains the point.

boolean NPtSelection::RightAcont (Coord x0, Coord y0, Coord x1, Coord y1,
PointObj& po, Graphic* gs) {
    IBrush* brush = (IBrush*) gs->GetBrush();
    if (brush->RightArrow()) {
	return ArrowHeadcont(x0, y0, x1, y1, po, gs);
    }
    return false;
}

// LeftAints checks whether the left arrowhead intersects the area.

boolean NPtSelection::LeftAints (Coord x0, Coord y0, Coord x1, Coord y1,
BoxObj& userb, Graphic* gs) {
    IBrush* brush = (IBrush*) gs->GetBrush();
    if (brush->LeftArrow()) {
	return ArrowHeadints(x0, y0, x1, y1, userb, gs);
    }
    return false;
}

// RightAints checks whether the right arrowhead intersects the area.

boolean NPtSelection::RightAints (Coord x0, Coord y0, Coord x1, Coord y1,
BoxObj& userb, Graphic* gs) {
    IBrush* brush = (IBrush*) gs->GetBrush();
    if (brush->RightArrow()) {
	return ArrowHeadints(x0, y0, x1, y1, userb, gs);
    }
    return false;
}

// drawLeftA draws the left arrowhead if it should be drawn.

void NPtSelection::drawLeftA (Coord x0, Coord y0, Coord x1, Coord y1,
Canvas* c, Graphic* gs) {
    IBrush* brush = (IBrush*) gs->GetBrush();
    if (brush->LeftArrow()) {
	drawArrowHead(x0, y0, x1, y1, c, gs);
    }
}

// drawRightA draws the right arrowhead if it should be drawn.

void NPtSelection::drawRightA (Coord x0, Coord y0, Coord x1, Coord y1,
Canvas* c, Graphic* gs) {
    IBrush* brush = (IBrush*) gs->GetBrush();
    if (brush->RightArrow()) {
	drawArrowHead(x0, y0, x1, y1, c, gs);
    }
}

// Define an arrow head with its tip at the origin and its tail on the
// negative x axis so we can rotate the tail about the origin to the
// right angle and translate the tip to the right point.

static const int ARROWN = 3;
static Coord arrowx[ARROWN] = {0, -ARROWHEIGHT,  -ARROWHEIGHT};
static Coord arrowy[ARROWN] = {0, ARROWWIDTH/2, -ARROWWIDTH/2};
static Coord arrowconvx[ARROWN + 1];
static Coord arrowconvy[ARROWN + 1];

// MergeArrowHeadTol returns a tolerance to use around the graphic's
// extent that includes the arrowhead's size.

float NPtSelection::MergeArrowHeadTol (float tol, Graphic* gs) {
    IBrush* brush = (IBrush*) gs->GetBrush();
    if (brush->LeftArrow() || brush->RightArrow()) {
	float magnif = 1;
	Transformer* view = getRoot()->GetTransformer();
	if (view != nil && !view->Rotated()) { // rot breaks magnif calc
	    float fpx0, fpy0, fpx1, fpy1;
	    view->Transform(0.0, 0.0, fpx0, fpy0);
	    view->Transform(1.0, 1.0, fpx1, fpy1);
	    magnif = fpy1 - fpy0;
	}
	float arrowtol = 0.5 * ARROWWIDTH * points * magnif;
	tol = fmax(arrowtol, tol);
    }
    return tol;
}

// ArrowHeadcont returns true if the arrowhead contains the point.
// The arrowhead may be filled or unfilled depending on the pattern.

boolean NPtSelection::ArrowHeadcont (Coord x0, Coord y0, Coord x1, Coord y1,
PointObj& po, Graphic* gs) {
    boolean contains = false;
    IPattern* pattern = (IPattern*) gs->GetPattern();

    SetCTM(x0, y0, x1, y1, gs, !pattern->None());
    PointObj pt(&po);
    invTransform(pt.x, pt.y, gs);

    if (pattern->None()) {
	MultiLineObj ml(arrowx, arrowy, ARROWN);
	LineObj l(arrowx[ARROWN-1], arrowy[ARROWN-1], arrowx[0], arrowy[0]);
	contains = ml.Contains(pt) || l.Contains(pt);
    } else {
	FillPolygonObj fp(arrowx, arrowy, ARROWN);
	contains = fp.Contains(pt);
    }

    RestoreCTM(gs);
    return contains;
}

// ArrowHeadints returns true if the arrowhead intersects the area.
// The arrowhead may be filled or unfilled depending on the pattern.

boolean NPtSelection::ArrowHeadints (Coord x0, Coord y0, Coord x1, Coord y1,
BoxObj& userb, Graphic* gs) {
    boolean intersects = false;
    IPattern* pattern = (IPattern*) gs->GetPattern();

    SetCTM(x0, y0, x1, y1, gs, !pattern->None());
    transformList(arrowx, arrowy, ARROWN, arrowconvx, arrowconvy, gs);

    if (pattern->None()) {
	arrowconvx[ARROWN] = arrowconvx[0];
	arrowconvy[ARROWN] = arrowconvy[0];
	MultiLineObj ml(arrowconvx, arrowconvy, ARROWN + 1);
	intersects = ml.Intersects(userb);
    } else {
	FillPolygonObj fp(arrowconvx, arrowconvy, ARROWN);
	intersects = fp.Intersects(userb);
    }

    RestoreCTM(gs);
    return intersects;
}

// drawArrowHead draws the arrowhead.

void NPtSelection::drawArrowHead (Coord x0, Coord y0, Coord x1, Coord y1,
Canvas* c, Graphic* gs) {
    IPattern* pattern = (IPattern*) gs->GetPattern();
    if (!pattern->None()) {
	SetCTM(x0, y0, x1, y1, gs, true);
	update(gs);
	pFillPolygon(c, arrowx, arrowy, ARROWN);
	RestoreCTM(gs);
    }

    IBrush* brush = (IBrush*) gs->GetBrush();
    if (!brush->None()) {
	SetCTM(x0, y0, x1, y1, gs, false);
	update(gs);
	pPolygon(c, arrowx, arrowy, ARROWN);
	RestoreCTM(gs);
    }
}

// SetCTM stores gs's former transformation matrix and overwrites it
// with a new one defined to scale the arrowhead to its proper size
// and align it with the line.  The matrix includes the graphic's
// topmost parent's scaling but no other parents' scaling so the
// arrowhead will change size when we zoom the view but stay the same
// size when we scale the line.  New argument patternfill special-
// cases filling arrowhead to include the outermost edge of the
// outline drawn by the brush so dashed brushes won't expose white
// space where there's no pattern fill.

static Transformer* origCTM;
static Transformer arrowCTM;

void NPtSelection::SetCTM (Coord x0, Coord y0, Coord x1, Coord y1,
Graphic* gs, boolean patternfill) {
    arrowCTM = *identity;

    if (patternfill) {
	IBrush* brush = (IBrush*) gs->GetBrush();
	float bw = brush->Width();
	float padtip = sqrt(ARROWHEIGHT*ARROWHEIGHT +
			    0.25*ARROWWIDTH*ARROWWIDTH) * bw / ARROWWIDTH;
	float padtail = bw / 2;
	float patternscale = (ARROWHEIGHT + padtip + padtail) / ARROWHEIGHT;
	arrowCTM.Scale(patternscale, patternscale);
	arrowCTM.Translate(padtip, 0.);
    }

    arrowCTM.Scale(point, point);
    Transformer* view = getRoot()->GetTransformer();
    if (view != nil && !view->Rotated()) { // rot breaks magnif calc
	float fpx0, fpy0, fpx1, fpy1;
	view->Transform(0.0, 0.0, fpx0, fpy0);
	view->Transform(1.0, 1.0, fpx1, fpy1);
	float arrowxmag = fpx1 - fpx0;
	float arrowymag = fpy1 - fpy0;
	if (arrowxmag == arrowymag) { // won't scale arrows in BrushView
	    arrowCTM.Scale(arrowxmag, arrowymag);
	}
    }

    float tipx, tipy, tailx, taily;
    transform(float(x0), float(y0), tipx, tipy, gs);
    transform(float(x1), float(y1), tailx, taily, gs);
    float angle = Slope(tipx - tailx, tipy - taily);
    arrowCTM.Rotate(angle);
    arrowCTM.Translate(tipx, tipy);

    origCTM = gs->GetTransformer();
    if (origCTM != nil) {
	origCTM->Reference();
    }
    gs->SetTransformer(&arrowCTM);
}

// RestoreCTM restores gs's original transformation matrix.

void NPtSelection::RestoreCTM (Graphic* gs) {
    gs->SetTransformer(origCTM);
    delete origCTM;
}

// sign returns 1 if the number's nonnegative, -1 if it's negative.

inline float sign (float num) {
    return (num >= 0.) ? 1. : -1.;
}

// Slope returns the number of degrees in the given slope.

float NPtSelection::Slope (float dx, float dy) {
    float angle = 0.;
    if (dx == 0.) {
        angle = sign(dy) * 90.;
    } else {
	angle = degrees(atan(dy/dx));
	if (dx < 0.) {
	    angle += sign(dy) * 180.;
	}
    }
    return angle;
}
