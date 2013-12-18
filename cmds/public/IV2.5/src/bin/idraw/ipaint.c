// $Header: ipaint.c,v 1.8 89/04/17 00:30:51 linton Exp $
// implements classes IBrush, IFont, and IPattern.

#include "ipaint.h"
#include "istring.h"

// IBrush creates the brush.  Calling IBrush with no arguments creates
// the "none" brush.

IBrush::IBrush () : () {
    leftarrow = false;
    rightarrow = false;
    CalcDashPat(0xffff);
}

IBrush::IBrush (int p, int w, boolean l, boolean r) : (p, w) {
    leftarrow = l;
    rightarrow = r;
    CalcDashPat(p);
}

// Width overrides PBrush::Width when PBrush's value is the predefined
// brush single because single cannot be trusted to return 1.  It
// might return 0 because the X implementation might support only a
// fast, device-dependent brush, not the standard single-width brush.

int IBrush::Width () {
    int width = PBrush::Width();
    if (value == single) {
	width = 1;
    }
    return width;
}

// CalcDashPat calculates and stores the Postscript dash pattern
// corresponding to the brush's line pattern.

void IBrush::CalcDashPat (int linepat) {
    linepat &= 0xffff;		// mask should always match patternWidth

    if (linepat == 0x0000) {	    // clear brush
	dashpatsize = 0;
	dashoffset = -1;
    } else if (linepat == 0xffff) { // solid brush
	dashpatsize = 0;
	dashoffset = 0;
    } else if (linepat == 0x5555) { // dotted brush, store 1 element not 16
	dashpat[0] = 1;
	dashpatsize = 1;
	dashoffset = 1;
    } else if (linepat == 0xaaaa) { // dotted brush, store 1 element not 16
	dashpat[0] = 1;
	dashpatsize = 1;
	dashoffset = 0;
    } else {
	int i = 0;
	while (!((linepat << i) & 0x8000)) {
	    ++i;
	}
	dashoffset = patternWidth - i + 1;

	int j = 0;
	boolean currentrun = true;
	int length = 0;
	for (int k = 0; k < patternWidth; k++) {
	    if ((((linepat << i) & 0x8000) != 0) == currentrun) {
		++length;
	    } else {
		dashpat[j++] = length;
		currentrun = !currentrun;
		length = 1;
	    }
	    i = (i == patternWidth) ? 0 : i + 1;
	}
	if (length > 0) {
	    dashpat[j] = length;
	}
	dashpatsize = j + 1;
    }

    const int Postscriptdashlimit = 11;
    if (dashpatsize > Postscriptdashlimit) {
	fprintf(stderr, "Brush dash pattern 0x%x exceeds maximum ", linepat);
	fprintf(stderr, "length of Postscript dash pattern with ");
	fprintf(stderr, "%d elements, truncated to ", dashpatsize);
	fprintf(stderr, "%d elements\n", Postscriptdashlimit);
	dashpatsize = Postscriptdashlimit;
    }
}

// IColor creates the named color and stores its name.

IColor::IColor (const char* n) : (n) {
    name = strdup(n);
}

// IColor creates the color using the given intensities and stores its
// "name".

IColor::IColor (int r, int g, int b, const char* n) : (r, g, b) {
    name = strdup(n);
}

// IColor stores the given color and its "name".

IColor::IColor (Color* color, const char* n) {
    value = color;
    value->Reference();
    name = strdup(n);
}

// Free storage allocated for the name.

IColor::~IColor () {
    delete name;
}

// IFont creates the named font and stores the print font and size.

IFont::IFont (const char* name, const char* pf, const char* ps)
: (FilterName(name)) {
    printfont = strdup(pf);
    printsize = strdup(ps);
    printfontandsize = new char[strlen(pf) + 1 + strlen(ps) + 1];
    strcpy(printfontandsize, pf);
    strcat(printfontandsize, " ");
    strcat(printfontandsize, ps);
}

// Free storage allocated for the print font and size.

IFont::~IFont () {
    delete printfont;
    delete printsize;
    delete printfontandsize;
}

// FilterName filters the name to ensure "stdfont" does not pass
// through to PFont without being converted to nil.

const char* IFont::FilterName (const char* name) {
    if (strcmp(name, "stdfont") == 0) {
	name = nil;
    }
    return name;
}

// IPattern creates the pattern.  Calling IPattern with no arguments
// creates the "none" pattern, calling IPattern with an integer
// creates a pattern from a 4x4 bitmap, calling IPattern with an array
// and the actual size creates a pattern from a 16x16 bitmap that may
// have originally been 8x8, and calling IPattern with an integer and
// a float creates a grayscale pattern from a 4x4 bitmap.

IPattern::IPattern () : () {
    graylevel = -1;
    size = 0;
}

IPattern::IPattern (int dither, float g) : (dither) {
    graylevel = g;
    size = 0;
}

IPattern::IPattern (int data[patternHeight], int s) : (data) {
    graylevel = -1;
    size = s;
}
